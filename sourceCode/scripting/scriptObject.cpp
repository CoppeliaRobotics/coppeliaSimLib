#include <scriptObject.h>
#include <tt.h>
#include <utils.h>
#include <vDateTime.h>
#include <app.h>
#include <boost/algorithm/string.hpp>
#include <interfaceStackNull.h>
#include <interfaceStackBool.h>
#include <interfaceStackNumber.h>
#include <interfaceStackInteger.h>
#include <interfaceStackString.h>
#include <interfaceStackTable.h>
#include <interfaceStackMatrix.h>
#include <simFlavor.h>
#include <luaScriptFunctions.h>
#include <luaWrapper.h>
#include <regex>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

#define BASE_SANDBOX_SCRIPT "sandboxScriptBase.lua"
#define INITIALLY_SUSPEND_LOADED_SCRIPTS true
int CScriptObject::_nextScriptHandle = SIM_IDSTART_LUASCRIPT;
std::vector<int> CScriptObject::_externalScriptCalls;
std::map<std::string, std::pair<int, int>> CScriptObject::_signalNameToScriptHandle;

CScriptObject::CScriptObject(int scriptType)
{ // scriptType to -1 for serialization
    _scriptUid = App::getFreshUniqueId(-1);
    _tempSuspended = false;
    _sceneObjectScript = false;
    _parentIsProxy = false;
    _sceneObjectHandle = -1;
    _scriptText = "";
    _scriptTextExec = "";
    _numberOfPasses = 0;
    _addOnUiMenuHandle = -1;
    _addOnExecPriority = sim_scriptexecorder_normal;
    for (size_t i = 0; i < 3; i++)
    {
        _sysFuncAndHookCnt_event[i] = 0;
        _sysFuncAndHookCnt_dyn[i] = 0;
        _sysFuncAndHookCnt_contact[i] = 0;
        _sysFuncAndHookCnt_joint[i] = 0;
    }

    // Old
    // ***********************************************************
    _automaticCascadingCallsDisabled_old = false;
    _mainScriptIsDefaultMainScript_old = false;
    _custScriptDisabledDSim_compatibilityMode_DEPRECATED = false;
    _customizationScriptCleanupBeforeSave_DEPRECATED = false;
    _scriptParameters_backCompatibility = new CUserParameters();
    _customObjectData_old = nullptr;
    _executionPriority_old = sim_scriptexecorder_normal;
    // ***********************************************************

    _scriptIsDisabled = false;
    _autoRestartOnError = false;
    _scriptState = scriptState_unloaded;
    _flaggedForDestruction = false;
    _executionDepth = 0;
    _autoStartAddOn = -1;
    _previousEditionWindowPosAndSize[0] = 50;
    _previousEditionWindowPosAndSize[1] = 50;
    _previousEditionWindowPosAndSize[2] = 1000;
    _previousEditionWindowPosAndSize[3] = 800;
    _outsideCommandQueue = new COutsideCommandQueueForScript();
    _scriptType = scriptType;
    _containedSystemCallbacks.resize(sim_syscb_endoflist, false);
    _timeOfScriptExecutionStart = -1;
    _interpreterState = nullptr;

    _loadBufferResult_lua = -1;
    setHandle();
}

CScriptObject::~CScriptObject()
{ // use destory further below to delete the object!
    TRACE_INTERNAL;
    _killInterpreterState(); // should already have been done outside of the destructor!
    delete _outsideCommandQueue;

    // Old:
    delete _scriptParameters_backCompatibility;
    delete _customObjectData_old;
}

int CScriptObject::setHandle()
{
    _scriptHandle = _nextScriptHandle++;
    if (_nextScriptHandle > SIM_IDEND_LUASCRIPT)
        _nextScriptHandle = SIM_IDSTART_LUASCRIPT;
    while ((App::currentWorld != nullptr) && (App::currentWorld->sceneObjects != nullptr) && (App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptObjectFromHandle(_scriptHandle) != nullptr))
    {
        _scriptHandle++;
        if (_scriptHandle > SIM_IDEND_LUASCRIPT)
            _scriptHandle = SIM_IDSTART_LUASCRIPT;
    }
    return _scriptHandle;
}

void CScriptObject::destroy(CScriptObject* obj, bool registeredObject, bool announceScriptDestruction /*= true*/)
{
    if (registeredObject)
    {
        if (App::userSettings->externalScriptEditor.length() > 0)
        {
            // destroy file
            std::string fname = App::folders->getTempDataPath() + "/";
            fname.append(obj->_filenameForExternalScriptEditor);
            if (VFile::doesFileExist(fname.c_str()))
                VFile::eraseFile(fname.c_str());
        }
        if (announceScriptDestruction)
            App::worldContainer->announceScriptWillBeErased(obj->getScriptHandle(), obj->getScriptUid(), obj->isSimulationOrMainScript(), obj->isSceneSwitchPersistentScript());
    }
    delete obj;
}

std::string CScriptObject::getFilenameForExternalScriptEditor()
{
    if (_filenameForExternalScriptEditor.size() == 0)
    {
        if (_scriptType == sim_scripttype_main)
            _filenameForExternalScriptEditor = "mainScript-";
        else
        {
            CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectHandle);
            if (obj != nullptr)
            {
                _filenameForExternalScriptEditor = obj->getObjectAlias();
                if (_scriptType == sim_scripttype_simulation)
                    _filenameForExternalScriptEditor += "-child-";
                else
                    _filenameForExternalScriptEditor += "-cust-";
            }
            else
                _filenameForExternalScriptEditor = "error-";
        }
        std::string tmp(App::currentWorld->environment->getSceneName());
        if (tmp.size() == 0)
            tmp = "newScene";
        _filenameForExternalScriptEditor += tmp + "-" + std::to_string(_scriptHandle);
        if (getLang() == "python")
            _filenameForExternalScriptEditor += ".py";
        else if (getLang() == "lua")
            _filenameForExternalScriptEditor += ".lua";
        else
            _filenameForExternalScriptEditor += ".txt";
        fromBufferToFile();
    }
    std::string fname = App::folders->getTempDataPath() + "/";
    fname.append(_filenameForExternalScriptEditor);
    return (fname);
}

void CScriptObject::fromFileToBuffer()
{
    if (App::userSettings->externalScriptEditor.size() > 0)
    { // read file
        std::string fname = App::folders->getTempDataPath() + "/";
        fname.append(_filenameForExternalScriptEditor);

        VFile myFile(fname.c_str(), VFile::READ | VFile::SHARE_DENY_NONE, true);
        if (myFile.getFile() != NULL)
        {
            VArchive arW(&myFile, VArchive::LOAD);
            unsigned int archiveLength = (unsigned int)myFile.getLength();
            std::string t;
            t.resize(archiveLength, ' ');
            for (unsigned int i = 0; i < archiveLength; i++)
                arW >> t[i];
            arW.close();
            myFile.close();
            setScriptText(t.c_str(), false);
        }
    }
}

bool CScriptObject::shouldAutoYield()
{
    bool retVal = false;
    if ((_forbidAutoYieldingLevel == 0) && (_forbidOverallYieldingLevel == 0))
        retVal = VDateTime::getTimeDiffInMs(_timeForNextAutoYielding) > 0;
    if (retVal)
        _timeForNextAutoYielding = int(VDateTime::getTimeInMs()) + _delayForAutoYielding;
    return (retVal);
}

bool CScriptObject::canManualYield() const
{
    return (_forbidOverallYieldingLevel == 0);
}

int CScriptObject::getDelayForAutoYielding() const
{
    return (_delayForAutoYielding);
}

void CScriptObject::setDelayForAutoYielding(int d)
{
    if (d < 0)
        d = 0;
    if (d > 10000)
        d = 10000;
    _delayForAutoYielding = d;
}

int CScriptObject::changeAutoYieldingForbidLevel(int dx, bool absolute)
{
    int retVal = _forbidAutoYieldingLevel;
    if (absolute)
        _forbidAutoYieldingLevel = dx;
    else
    {
        _forbidAutoYieldingLevel += dx;
        if (_forbidAutoYieldingLevel < 0)
            _forbidAutoYieldingLevel = 0;
    }
    return (retVal);
}

int CScriptObject::getAutoYieldingForbidLevel() const
{
    return (_forbidAutoYieldingLevel);
}

int CScriptObject::changeOverallYieldingForbidLevel(int dx, bool absolute)
{
    int retVal = _forbidOverallYieldingLevel;
    if (absolute)
        _forbidOverallYieldingLevel = dx;
    else
    {
        _forbidOverallYieldingLevel += dx;
        if (_forbidOverallYieldingLevel < 0)
            _forbidOverallYieldingLevel = 0;
    }
    return (retVal);
}

void CScriptObject::fromBufferToFile() const
{
    if (App::userSettings->externalScriptEditor.size() > 0)
    { // write file
        if ((App::currentWorld == nullptr) || (App::currentWorld->environment == nullptr) ||
            (!App::currentWorld->environment->getSceneLocked()))
        {
            std::string fname = App::folders->getTempDataPath() + "/";
            fname.append(_filenameForExternalScriptEditor);

            VFile myFile(fname.c_str(), VFile::CREATE_WRITE | VFile::SHARE_EXCLUSIVE, true);
            if (myFile.getFile() != NULL)
            {
                VArchive arW(&myFile, VArchive::STORE);
                if (_scriptText.size() > 0)
                {
                    for (size_t i = 0; i < _scriptText.size(); i++)
                        arW << _scriptText[i];
                }
                arW.close();
                myFile.close();
            }
        }
    }
}

int CScriptObject::getSystemCallbackFromString(const char* cb)
{
    if (std::string(cb) == "sysCall_info")
        return (sim_syscb_info);
    if (std::string(cb) == "sysCall_thread")
        return (sim_syscb_thread);
    if (std::string(cb) == "sysCall_init")
        return (sim_syscb_init);
    if (std::string(cb) == "sysCall_cleanup")
        return (sim_syscb_cleanup);
    if (std::string(cb) == "sysCall_nonSimulation")
        return (sim_syscb_nonsimulation);
    if (std::string(cb) == "sysCall_beforeMainScript")
        return (sim_syscb_beforemainscript);
    if (std::string(cb) == "sysCall_beforeSimulation")
        return (sim_syscb_beforesimulation);
    if (std::string(cb) == "sysCall_afterSimulation")
        return (sim_syscb_aftersimulation);
    if (std::string(cb) == "sysCall_actuation")
        return (sim_syscb_actuation);
    if (std::string(cb) == "sysCall_sensing")
        return (sim_syscb_sensing);
    if (std::string(cb) == "sysCall_suspended")
        return (sim_syscb_suspended);
    if (std::string(cb) == "sysCall_suspend")
        return (sim_syscb_suspend);
    if (std::string(cb) == "sysCall_resume")
        return (sim_syscb_resume);
    if (std::string(cb) == "sysCall_beforeInstanceSwitch")
        return (sim_syscb_beforeinstanceswitch);
    if (std::string(cb) == "sysCall_afterInstanceSwitch")
        return (sim_syscb_afterinstanceswitch);
    if (std::string(cb) == "sysCall_beforeCopy")
        return (sim_syscb_beforecopy);
    if (std::string(cb) == "sysCall_afterCopy")
        return (sim_syscb_aftercopy);
    if (std::string(cb) == "sysCall_beforeDelete")
        return (sim_syscb_beforedelete);
    if (std::string(cb) == "sysCall_afterDelete")
        return (sim_syscb_afterdelete);
    if (std::string(cb) == "sysCall_afterCreate")
        return (sim_syscb_aftercreate);
    if (std::string(cb) == "sysCall_addOnScriptSuspend")
        return (sim_syscb_aos_suspend);
    if (std::string(cb) == "sysCall_addOnScriptResume")
        return (sim_syscb_aos_resume);
    if (std::string(cb) == "sysCall_joint")
        return (sim_syscb_joint);
    if (std::string(cb) == "sysCall_vision")
        return (sim_syscb_vision);
    if (std::string(cb) == "sysCall_userConfig")
        return (sim_syscb_userconfig);
    if (std::string(cb) == "sysCall_moduleEntry")
        return (sim_syscb_moduleentry);
    if (std::string(cb) == "sysCall_trigger")
        return (sim_syscb_trigger);
    if (std::string(cb) == "sysCall_contact")
        return (sim_syscb_contact);
    if (std::string(cb) == "sysCall_dyn")
        return (sim_syscb_dyn);
    if (std::string(cb) == "sysCall_customCallback")
        return (sim_syscb_customcallback1);
    if (std::string(cb) == "sysCall_event")
        return (sim_syscb_event);
    if (std::string(cb) == "sysCall_ext")
        return (sim_syscb_ext);
    if (std::string(cb) == "sysCall_realTimeIdle")
        return (sim_syscb_realtimeidle);
    if (std::string(cb) == "sysCall_beforeSave")
        return (sim_syscb_beforesave);
    if (std::string(cb) == "sysCall_afterSave")
        return (sim_syscb_aftersave);
    if (std::string(cb) == "sysCall_afterLoad")
        return (sim_syscb_afterload);
    if (std::string(cb) == "sysCall_msg")
        return (sim_syscb_msg);
    if (std::string(cb) == "sysCall_selChange")
        return (sim_syscb_selchange);
    if (std::string(cb) == "sysCall_data")
        return (sim_syscb_data);

    // Old:
    if (std::string(cb) == "sysCall_addOnScriptRun")
        return (sim_syscb_aos_run_old);
    if (std::string(cb) == "sysCall_threadmain")
        return (sim_syscb_threadmain);
    if (std::string(cb) == "sysCall_jointCallback")
        return (sim_syscb_jointcallback);
    if (std::string(cb) == "sysCall_contactCallback")
        return (sim_syscb_contactcallback);
    if (std::string(cb) == "sysCall_dynCallback")
        return (sim_syscb_dyncallback);
    return (-1);
}

std::string CScriptObject::getSystemCallbackString(int calltype, int what)
{ // what: 0=function string, 1=string for code completion, if not deprecated, 2=string for code completion and calltip,
    // if not deprecated
    if (calltype == sim_syscb_info)
    {
        std::string r("sysCall_info");
        if (what == 2)
            r += " - Called even before the script is initialized.";
        return (r);
    }
    if (calltype == sim_syscb_thread)
    {
        std::string r("sysCall_thread");
        if (what == 2)
            r += " - Thread/coroutine entry point.";
        return (r);
    }
    if (calltype == sim_syscb_init)
    {
        std::string r("sysCall_init");
        if (what == 2)
            r += " - Called when the script is initialized.";
        return (r);
    }
    if (calltype == sim_syscb_cleanup)
    {
        std::string r("sysCall_cleanup");
        if (what == 2)
            r += " - Called when the script is destroyed.";
        return (r);
    }
    if (calltype == sim_syscb_nonsimulation)
    {
        std::string r("sysCall_nonSimulation");
        if (what == 2)
            r += " - Called when simulation is not running.";
        return (r);
    }
    if (calltype == sim_syscb_beforemainscript)
    {
        std::string r("sysCall_beforeMainScript");
        if (what == 2)
            r += " - Called just before the main script is called.";
        return (r);
    }

    if (calltype == sim_syscb_beforesimulation)
    {
        std::string r("sysCall_beforeSimulation");
        if (what == 2)
            r += " - Called just before simulation starts.";
        return (r);
    }
    if (calltype == sim_syscb_aftersimulation)
    {
        std::string r("sysCall_afterSimulation");
        if (what == 2)
            r += " - Called just after simulation ended.";
        return (r);
    }
    if (calltype == sim_syscb_actuation)
    {
        std::string r("sysCall_actuation");
        if (what == 2)
            r += " - Called in the actuation phase.";
        return (r);
    }
    if (calltype == sim_syscb_sensing)
    {
        std::string r("sysCall_sensing");
        if (what == 2)
            r += " - Called in the sensing phase.";
        return (r);
    }
    if (calltype == sim_syscb_suspended)
    {
        std::string r("sysCall_suspended");
        if (what == 2)
            r += " - Called when simulation is suspended.";
        return (r);
    }
    if (calltype == sim_syscb_suspend)
    {
        std::string r("sysCall_suspend");
        if (what == 2)
            r += " - Called just before simulation gets suspended.";
        return (r);
    }
    if (calltype == sim_syscb_resume)
    {
        std::string r("sysCall_resume");
        if (what == 2)
            r += " - Called just before simulation resumes.";
        return (r);
    }
    if (calltype == sim_syscb_beforeinstanceswitch)
    {
        std::string r("sysCall_beforeInstanceSwitch");
        if (what == 2)
            r += " - Called just before an instance switch.";
        return (r);
    }
    if (calltype == sim_syscb_afterinstanceswitch)
    {
        std::string r("sysCall_afterInstanceSwitch");
        if (what == 2)
            r += " - Called just after an instance switch.";
        return (r);
    }
    if (calltype == sim_syscb_beforecopy)
    {
        std::string r("sysCall_beforeCopy");
        if (what == 2)
            r += " - Called just before objects are copied.";
        return (r);
    }
    if (calltype == sim_syscb_aftercopy)
    {
        std::string r("sysCall_afterCopy");
        if (what == 2)
            r += " - Called just after objects were copied.";
        return (r);
    }
    if (calltype == sim_syscb_beforedelete)
    {
        std::string r("sysCall_beforeDelete");
        if (what == 2)
            r += " - Called just before objects are deleted.";
        return (r);
    }
    if (calltype == sim_syscb_afterdelete)
    {
        std::string r("sysCall_afterDelete");
        if (what == 2)
            r += " - Called just after objects were deleted.";
        return (r);
    }
    if (calltype == sim_syscb_aftercreate)
    {
        std::string r("sysCall_afterCreate");
        if (what == 2)
            r += " - Called just after objects were created.";
        return (r);
    }
    if (calltype == sim_syscb_aos_suspend)
    {
        std::string r("sysCall_addOnScriptSuspend");
        if (what == 2)
            r += " - Called just before the add-on script execution gets suspended.";
        return (r);
    }
    if (calltype == sim_syscb_aos_resume)
    {
        std::string r("sysCall_addOnScriptResume");
        if (what == 2)
            r += " - Called just before the add-on script execution resumes.";
        return (r);
    }
    if (calltype == sim_syscb_joint)
    {
        std::string r("sysCall_joint");
        if (what == 2)
            r += " - Called for motion handling of kinematic joints, or for custom control of dynamic joints.";
        return (r);
    }
    if (calltype == sim_syscb_vision)
    {
        std::string r("sysCall_vision");
        if (what == 2)
            r += " - Called when a vision sensor requests image processing.";
        return (r);
    }
    if (calltype == sim_syscb_userconfig)
    {
        std::string r("sysCall_userConfig");
        if (what == 2)
            r += " - Called when the user double-clicks a user parameter icon.";
        return (r);
    }
    if (calltype == sim_syscb_moduleentry)
    {
        std::string r("sysCall_moduleEntry");
        if (what == 2)
            r += " - Called when the user selects a module menu entry.";
        return (r);
    }
    if (calltype == sim_syscb_trigger)
    {
        std::string r("sysCall_trigger");
        if (what == 2)
            r += " - Called when the sensor is triggered.";
        return (r);
    }
    if (calltype == sim_syscb_contact)
    {
        std::string r("sysCall_contact");
        if (what == 2)
            r += " - Called by the physics engine when two respondable shapes are contacting.";
        return (r);
    }
    if (calltype == sim_syscb_dyn)
    {
        std::string r("sysCall_dyn");
        if (what == 2)
            r += " - Called by the physics engine twice per dynamic simulation pass.";
        return (r);
    }
    if ((calltype >= sim_syscb_customcallback1) && (calltype <= sim_syscb_customcallback4))
    {
        std::string r("sysCall_customCallback");
        r += std::to_string(calltype - sim_syscb_customcallback1 + 1);
        if (what == 2)
            r += " - Can be called by a customized main script.";
        return (r);
    }
    if (calltype == sim_syscb_event)
    {
        std::string r("sysCall_event");
        if (what == 2)
            r += " - Called asynchronously with buffered events.";
        return (r);
    }
    if (calltype == sim_syscb_ext)
    {
        std::string r("sysCall_ext");
        if (what == 2)
            r += " - Calls (and shadows) user callbacks.";
        return (r);
    }
    if (calltype == sim_syscb_realtimeidle)
    {
        std::string r("sysCall_realTimeIdle");
        if (what == 2)
            r += " - Called when in real-time mode and idle.";
        return (r);
    }
    if (calltype == sim_syscb_beforesave)
    {
        std::string r("sysCall_beforeSave");
        if (what == 2)
            r += " - Called just before the scene is saved.";
        return (r);
    }
    if (calltype == sim_syscb_aftersave)
    {
        std::string r("sysCall_afterSave");
        if (what == 2)
            r += " - Called just after the scene was saved.";
        return (r);
    }
    if (calltype == sim_syscb_afterload)
    {
        std::string r("sysCall_afterLoad");
        if (what == 2)
            r += " - Called just after the scene was loaded.";
        return (r);
    }
    if (calltype == sim_syscb_msg)
    {
        std::string r("sysCall_msg");
        if (what == 2)
            r += " - Called via sim.broadcastMsg.";
        return (r);
    }
    if (calltype == sim_syscb_selchange)
    {
        std::string r("sysCall_selChange");
        if (what == 2)
            r += " - Called when selection changes.";
        return (r);
    }
    if (calltype == sim_syscb_data)
    {
        std::string r("sysCall_data");
        if (what == 2)
            r += " - Called when a custom data block changed.";
        return (r);
    }

    // Old:
    if (calltype == sim_syscb_aos_run_old)
    {
        std::string r("sysCall_addOnScriptRun");
        if (what >= 1)
            return (""); // deprecated
        return (r);
    }
    if (calltype == sim_syscb_threadmain)
    {
        std::string r("sysCall_threadmain");
        if (what >= 1)
            return (""); // deprecated
        return (r);
    }
    if (calltype == sim_syscb_jointcallback)
    { // for backw. compat.
        std::string r("sysCall_jointCallback");
        if (what >= 1)
            return (""); // deprecated
        return (r);
    }
    if (calltype == sim_syscb_contactcallback)
    { // for backw. compat.
        std::string r("sysCall_contactCallback");
        if (what >= 1)
            return (""); // deprecated
        return (r);
    }
    if (calltype == sim_syscb_dyncallback)
    { // for backward compatibility
        std::string r("sysCall_dynCallback");
        if (what >= 1)
            return (""); // deprecated
        return (r);
    }

    return ("");
}

bool CScriptObject::wasModulePreviouslyUsed(const char* moduleName) const
{
    return (_previouslyUsedModules.find(moduleName) != _previouslyUsedModules.end());
}

void CScriptObject::addUsedModule(const char* module)
{
    _previouslyUsedModules.insert(module);
}

void CScriptObject::addModulesDetectedInCode()
{ // try to guess required modules. If such a similar module is already in _previouslyUsedModules, ignore it
    QRegularExpression re("\\brequire\\b\\s*\\(?\\s*['\"]([^'\"]*)['\"]\\s*\\)?");
    QRegularExpressionMatchIterator it = re.globalMatch(_scriptText.c_str());
    while (it.hasNext())
    {
        QRegularExpressionMatch m = it.next();
        std::string ns(m.captured(1).toStdString());
        std::string ns1(ns);
        size_t p = ns1.find("-");
        if (p != std::string::npos)
            ns1.assign(ns1.begin(), ns1.begin() + p);
        bool found = false;
        for (auto it2 = _previouslyUsedModules.begin(); it2 != _previouslyUsedModules.end(); it2++)
        {
            std::string ns2(*it2);
            p = ns2.find("-");
            if (p != std::string::npos)
                ns2.assign(ns2.begin(), ns2.begin() + p);
            if (ns1 == ns2)
            {
                found = true;
                break;
            }
        }
        if (!found)
            _previouslyUsedModules.insert(ns.c_str());
    }
}

void CScriptObject::getMatchingFunctions(const char* txt, std::set<std::string>& v, const CScriptObject* requestOrigin)
{
    App::worldContainer->scriptCustomFuncAndVarContainer->insertAllFunctionNamesThatStartSame(txt, v); // old plugins

    App::worldContainer->codeEditorInfos->insertWhatStartsSame(txt, v, 1, requestOrigin);
}

void CScriptObject::getMatchingConstants(const char* txt, std::set<std::string>& v, const CScriptObject* requestOrigin)
{
    App::worldContainer->scriptCustomFuncAndVarContainer->insertAllVariableNamesThatStartSame(txt, v); // old plugins

    App::worldContainer->codeEditorInfos->insertWhatStartsSame(txt, v, 2, requestOrigin);
}

std::string CScriptObject::getFunctionCalltip(const char* txt, const CScriptObject* requestOrigin)
{
    std::string retVal = App::worldContainer->codeEditorInfos->getFunctionCalltip(txt, requestOrigin);

    if (retVal.size() == 0)
    { // Check old plugin functions' calltips:
        for (size_t j = 0; j < App::worldContainer->scriptCustomFuncAndVarContainer->getCustomFunctionCount(); j++)
        {
            CScriptCustomFunction* it =
                App::worldContainer->scriptCustomFuncAndVarContainer->getCustomFunctionFromIndex(j);
            std::string n = it->getFunctionName();
            if (n.compare(txt) == 0)
            {
                retVal = it->getCallTips();
                break;
            }
        }
    }

    return (retVal);
}

bool CScriptObject::isSystemCallbackInReverseOrder(int callType)
{
    bool retVal = ((callType == sim_syscb_contactcallback) || (callType == sim_syscb_contact));
    return (retVal);
}

bool CScriptObject::isSystemCallbackInterruptible(int callType)
{
    bool retVal =
        ((callType == sim_syscb_contactcallback) || (callType == sim_syscb_contact) || (callType == sim_syscb_joint));
    return (retVal);
}

bool CScriptObject::canCallSystemCallback(int scriptType, bool threadedOld, int callType)
{
    if (scriptType == -1)
        return (true);

    if (threadedOld && (scriptType == sim_scripttype_simulation))
    { // for backward compatibility
        if (callType == sim_syscb_threadmain)
            return (true);
        if (callType == sim_syscb_cleanup)
            return (true);
        if (callType == sim_syscb_vision)
            return (true);
        if (callType == sim_syscb_trigger)
            return (true);
    }
    else
    { // For all script types (except for the old threaded scripts):
        if (callType == sim_syscb_info)
            return (true);
        if (callType == sim_syscb_init)
            return (true);
        if (callType == sim_syscb_cleanup)
            return (true);
        if (callType == sim_syscb_actuation)
            return (true);
        if (callType == sim_syscb_sensing)
            return (true);
        if (callType == sim_syscb_suspend)
            return (true);
        if (callType == sim_syscb_suspended)
            return (true);
        if (callType == sim_syscb_resume)
            return (true);
        if (callType == sim_syscb_beforecopy)
            return (true);
        if (callType == sim_syscb_aftercopy)
            return (true);
        if (callType == sim_syscb_beforedelete)
            return (true);
        if (callType == sim_syscb_afterdelete)
            return (true);
        if (callType == sim_syscb_aftercreate)
            return (true);
        if (callType == sim_syscb_moduleentry)
            return (true);
        if (callType == sim_syscb_event)
            return (true);
        if (callType == sim_syscb_ext)
            return (true);
        if (callType == sim_syscb_beforemainscript)
            return (true);
        if (callType == sim_syscb_realtimeidle)
            return (true);
        if (callType == sim_syscb_beforesave)
            return (true);
        if (callType == sim_syscb_aftersave)
            return (true);
        if (callType == sim_syscb_afterload)
            return (true);
        if (callType == sim_syscb_msg)
            return (true);
        if (callType == sim_syscb_selchange)
            return (true);
    }
    if ((scriptType == sim_scripttype_sandbox) || (scriptType == sim_scripttype_addon) ||
        (scriptType == sim_scripttype_customization))
    {
        if (callType == sim_syscb_nonsimulation)
            return (true);
        if (callType == sim_syscb_beforesimulation)
            return (true);
        if (callType == sim_syscb_aftersimulation)
            return (true);
        //       if (callType==sim_syscb_beforemainscript)
        //           return(true);
        if (callType == sim_syscb_beforeinstanceswitch)
            return (true);
        if (callType == sim_syscb_afterinstanceswitch)
            return (true);
    }
    if (scriptType == sim_scripttype_addon)
    {
        if (callType == sim_syscb_aos_run_old) // for backward compatibility
            return (true);
        if (callType == sim_syscb_aos_suspend)
            return (true);
        if (callType == sim_syscb_aos_resume)
            return (true);
    }
    if ((scriptType == sim_scripttype_customization) ||
        ((!threadedOld) && (scriptType == sim_scripttype_simulation)))
    {
        if (callType == sim_syscb_jointcallback) // backw. compat.
            return (true);
        if (callType == sim_syscb_joint)
            return (true);
        if (callType == sim_syscb_vision)
            return (true);
        if (callType == sim_syscb_trigger)
            return (true);
        if (callType == sim_syscb_contactcallback) // backw. compat.
            return (true);
        if (callType == sim_syscb_contact)
            return (true);
        if (callType == sim_syscb_dyncallback) // backw. compat.
            return (true);
        if (callType == sim_syscb_dyn)
            return (true);
        if (callType == sim_syscb_userconfig)
            return (true);
        if (callType == sim_syscb_data)
            return (true);
        if ((callType >= sim_syscb_customcallback1) && (callType <= sim_syscb_customcallback4))
            return (true);
    }
    return (false);
}

std::vector<int> CScriptObject::getAllSystemCallbacks(int scriptType, bool threadedOld)
{
    const int ct[] = {sim_syscb_info, sim_syscb_init, sim_syscb_cleanup, sim_syscb_nonsimulation,
                      sim_syscb_beforemainscript, sim_syscb_beforesimulation, sim_syscb_aftersimulation,
                      sim_syscb_actuation, sim_syscb_sensing, sim_syscb_suspended, sim_syscb_suspend, sim_syscb_resume,
                      sim_syscb_beforeinstanceswitch, sim_syscb_afterinstanceswitch, sim_syscb_beforecopy,
                      sim_syscb_aftercopy, sim_syscb_beforedelete, sim_syscb_afterdelete, sim_syscb_aftercreate,
                      // sim_syscb_aos_run_old, // for backward compatibility
                      sim_syscb_aos_suspend, sim_syscb_aos_resume,
                      sim_syscb_jointcallback, // backw. compat.
                      sim_syscb_joint,
                      sim_syscb_contactcallback, // backw. comp.
                      sim_syscb_contact,
                      sim_syscb_dyncallback, // backw. comp.
                      sim_syscb_dyn, sim_syscb_vision, sim_syscb_trigger, sim_syscb_customcallback1,
                      sim_syscb_customcallback2, sim_syscb_customcallback3, sim_syscb_customcallback4,
                      // sim_syscb_threadmain, // for backward compatibility
                      sim_syscb_userconfig, sim_syscb_moduleentry, sim_syscb_event, sim_syscb_ext,
                      sim_syscb_realtimeidle, sim_syscb_beforesave, sim_syscb_aftersave, sim_syscb_afterload, sim_syscb_msg,
                      sim_syscb_selchange, sim_syscb_data, -1};

    std::vector<int> retVal;
    size_t i = 0;
    while (ct[i] != -1)
    {
        if (scriptType != -1)
        {
            if (canCallSystemCallback(scriptType, threadedOld, ct[i]))
                retVal.push_back(ct[i]);
        }
        else
            retVal.push_back(ct[i]);
        i++;
    }
    return (retVal);
}

void CScriptObject::setInExternalCall(int scriptHandle)
{
    if (scriptHandle < 0)
        _externalScriptCalls.pop_back();
    else
        _externalScriptCalls.push_back(scriptHandle);
}

int CScriptObject::getInExternalCall()
{
    if (_externalScriptCalls.size() > 0)
        return (_externalScriptCalls[_externalScriptCalls.size() - 1]);
    return (-1);
}

std::vector<std::string> CScriptObject::getAllSystemCallbackStrings(int scriptType, int what)
{ // what: 0=function string, 1=string for code completion, if not deprecated, 2=string for code completion and calltip,
    // if not deprecated
    std::vector<int> ct = getAllSystemCallbacks(scriptType, false);
    std::vector<std::string> retVal;
    for (size_t i = 0; i < ct.size(); i++)
    {
        std::string str(getSystemCallbackString(ct[i], what));
        if (str.size() > 0)
            retVal.push_back(str);
    }
    return (retVal);
}

bool CScriptObject::hasSystemFunction(int callType, bool returnTrueIfNotInitialized /*=true*/) const
{ // when the script is not initialized, we need to return true
    if (returnTrueIfNotInitialized && (_scriptState != scriptState_initialized))
        return (true);
    return (_containedSystemCallbacks[callType]);
}

bool CScriptObject::hasSystemFunctionOrHook(int callType) const
{
    std::string tmp(getSystemCallbackString(callType, 0));
    return (hasSystemFunction(callType) || hasFunctionHook(tmp.c_str()));
}

void CScriptObject::setTemporarilySuspended(bool s)
{
    _tempSuspended = s;
}

std::string CScriptObject::getAndClearLastStackTraceback()
{
    std::string retVal = _lastStackTraceback;
    _lastStackTraceback.clear();
    return (retVal);
}

double CScriptObject::getRandomDouble()
{
    return (double(_randGen()) / double(_randGen.max()));
}

void CScriptObject::setRandomSeed(unsigned int s)
{
    _randGen.seed(s);
}

int CScriptObject::getScriptExecutionTimeInMs() const
{
    if (_timeOfScriptExecutionStart < 0) // happens sometimes when calling luaWrap_luaL_doString
        return (0);
    return (VDateTime::getTimeDiffInMs(_timeOfScriptExecutionStart));
}

void CScriptObject::resetScriptExecutionTime()
{
    _timeOfScriptExecutionStart = int(VDateTime::getTimeInMs());
}

void CScriptObject::getPreviousEditionWindowPosAndSize(int posAndSize[4]) const
{
    for (int i = 0; i < 4; i++)
        posAndSize[i] = _previousEditionWindowPosAndSize[i];
}

void CScriptObject::setPreviousEditionWindowPosAndSize(const int posAndSize[4])
{
    for (int i = 0; i < 4; i++)
        _previousEditionWindowPosAndSize[i] = posAndSize[i];
}

std::string CScriptObject::getScriptName() const
{
    if (_scriptType == sim_scripttype_main)
        return ("mainScript");
    if (_scriptType == sim_scripttype_sandbox)
        return ("sandboxScript");
    if (_scriptType == sim_scripttype_addon)
        return (_addOnMenuName);
    if ((_scriptType == sim_scripttype_simulation) || (_scriptType == sim_scripttype_customization) || (_scriptType == sim_scripttype_passive))
    {
        CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectHandle);
        if (obj != nullptr)
            return (obj->getObjectAlias());
        return ("error");
    }

    return ("deprecatedScript");
}

int CScriptObject::getAddOnUiMenuHandle() const
{
    return (_addOnUiMenuHandle);
}

void CScriptObject::setAddOnPath(const char* p)
{
    _addOnPath = p;
}

std::string CScriptObject::getAddOnPath() const
{
    return _addOnPath;
}

int CScriptObject::getScriptState() const
{
    return _scriptState;
}

void CScriptObject::addSpecializedObjectEventData(CCbor* ev)
{
    if (_scriptHandle >= SIM_IDSTART_LUASCRIPT)
        ev->appendKeyText(propObject_objectType.name, "detachedScript");
    ev->appendKeyBool(propScriptObj_scriptDisabled.name, _scriptIsDisabled);
    ev->appendKeyBool(propScriptObj_restartOnError.name, _autoRestartOnError);
    ev->appendKeyInt(propScriptObj_execPriority.name, getScriptExecPriority());
    ev->appendKeyInt(propScriptObj_scriptType.name, _scriptType);
    ev->appendKeyInt(propScriptObj_scriptState.name, _scriptState);
    ev->appendKeyText(propScriptObj_language.name, _lang.c_str());
    ev->appendKeyText(propScriptObj_code.name, _scriptText.c_str());
    ev->appendKeyText(propScriptObj_scriptName.name, getScriptName().c_str());
    ev->appendKeyText(propScriptObj_addOnPath.name, _addOnPath.c_str());
    ev->appendKeyText(propScriptObj_addOnMenuPath.name, _addOnMenuPath.c_str());
}

void CScriptObject::setScriptState(int state)
{
    bool diff = (_scriptState != state);
    if (diff)
    {
        _scriptState = state;
        if (isNotInCopyBuffer() && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propScriptObj_scriptState.name;
            CCbor* ev;
            if (_scriptHandle <= SIM_IDEND_SCENEOBJECT)
                ev = App::worldContainer->createSceneObjectChangedEvent(_scriptHandle, false, cmd, true); // scene object type scripts (new)
            else
                ev = App::worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED, _scriptHandle, _scriptUid, cmd, true); // main, sandbox, add-ons, and old-type scripts
            ev->appendKeyInt(cmd, _scriptState);
            App::worldContainer->pushEvent();
        }
    }
}

void CScriptObject::setScriptExecPriority(int priority)
{
    bool diff = false;
    if (_sceneObjectHandle != -1)
    { // i.e. all new script objects, and all old associated scripts
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectHandle);
        if (it != nullptr)
        {
            int p = it->getScriptExecPriority();
            diff = (p != priority);
            if (diff)
                it->setScriptExecPriority_raw(priority);
        }
    }
    if (_scriptType == sim_scripttype_addon)
    {
        diff = (_addOnExecPriority != priority);
        _addOnExecPriority = priority;
    }
    if (diff)
    {
        if (isNotInCopyBuffer() && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propScriptObj_execPriority.name;
            CCbor* ev;
            if (_scriptHandle <= SIM_IDEND_SCENEOBJECT)
                ev = App::worldContainer->createSceneObjectChangedEvent(_scriptHandle, false, cmd, true); // scene object type scripts (new)
            else
                ev = App::worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED, _scriptHandle, _scriptUid, cmd, true); // main, sandbox, add-ons, and old-type scripts
            ev->appendKeyInt(cmd, priority);
            App::worldContainer->pushEvent();
        }
    }
}

int CScriptObject::getScriptExecPriority() const
{
    int retVal = sim_scriptexecorder_normal;
    if (_sceneObjectHandle != -1)
    { // i.e. all new script objects, and all old associated scripts
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectHandle);
        if (it != nullptr)
            retVal = it->getScriptExecPriority();
    }
    if (_scriptType == sim_scripttype_addon)
        retVal = _addOnExecPriority;
    return retVal;
}

void CScriptObject::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    if (isSimulationOrMainScript())
    {
        _scriptObjectInitialValuesInitialized = true;
        if (_outsideCommandQueue != nullptr)
            _outsideCommandQueue->initializeInitialValues(simulationAlreadyRunning);
    }
}

void CScriptObject::simulationAboutToStart()
{
    if (isSimulationOrMainScript())
    {
        resetScript(); // should already be reset! (should have been done in simulationEnded routine)
        setNumberOfPasses(0);
        _automaticCascadingCallsDisabled_old = false;
        initializeInitialValues(false);
    }
}

void CScriptObject::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
    // ended). For thoses situations there is the initializeInitialValues routine!
    if (isSimulationOrMainScript())
    {
        if (_outsideCommandQueue != nullptr)
            _outsideCommandQueue->simulationEnded();
        _scriptTextExec.clear();
        if (_scriptObjectInitialValuesInitialized)
        {
        }
        _scriptObjectInitialValuesInitialized = false;
    }
}

void CScriptObject::simulationAboutToEnd()
{
    if (isSimulationOrMainScript())
        resetScript(); // this has to happen while simulation is still running!!
}

void CScriptObject::setScriptIsDisabled(bool isDisabled)
{
    bool diff = (_scriptIsDisabled != isDisabled);
    if (diff)
    {
        _scriptIsDisabled = isDisabled;
        if (isNotInCopyBuffer() && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propScriptObj_scriptDisabled.name;
            CCbor* ev;
            if (_scriptHandle <= SIM_IDEND_SCENEOBJECT)
                ev = App::worldContainer->createSceneObjectChangedEvent(_scriptHandle, false, cmd, true); // scene object type scripts (new)
            else
                ev = App::worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED, _scriptHandle, _scriptUid, cmd, true); // main, sandbox, add-ons, and old-type scripts
            ev->appendKeyBool(cmd, _scriptIsDisabled);
            App::worldContainer->pushEvent();
        }
    }
}

bool CScriptObject::getScriptIsDisabled() const
{
    return _scriptIsDisabled;
}

bool CScriptObject::getScriptHasError() const
{
    return (_scriptState & scriptState_error);
}

void CScriptObject::setParentIsProxy(bool isProxy)
{
    _parentIsProxy = isProxy;
}

bool CScriptObject::getParentIsProxy() const
{
    return _parentIsProxy;
}

bool CScriptObject::isNotInCopyBuffer() const
{ // corresponds to the getIsInScene function with scene objects
    bool retVal = ((_scriptType == sim_scripttype_sandbox) || (_scriptType == sim_scripttype_addon));
    if (!retVal)
    {
        if (App::currentWorld->sceneObjects != nullptr)
        {
            CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectHandle);
            if (it != nullptr)
                retVal = it->getIsInScene();
        }
    }
    return retVal;
}

void CScriptObject::setAutoRestartOnError(bool restart)
{
    bool diff = (_autoRestartOnError != restart);
    if (diff)
    {
        _autoRestartOnError = restart;
        if (isNotInCopyBuffer() && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propScriptObj_restartOnError.name;
            CCbor* ev;
            if (_scriptHandle <= SIM_IDEND_SCENEOBJECT)
                ev = App::worldContainer->createSceneObjectChangedEvent(_scriptHandle, false, cmd, true); // scene object type scripts (new)
            else
                ev = App::worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED, _scriptHandle, _scriptUid, cmd, true); // main, sandbox, add-ons, and old-type scripts
            ev->appendKeyBool(cmd, _autoRestartOnError);
            App::worldContainer->pushEvent();
        }
    }
}

bool CScriptObject::getScriptDisabledAndNoErrorRaised() const
{
    bool disabled = _scriptIsDisabled;
    if (_scriptType == sim_scripttype_customization)
    {
        if (!App::userSettings->runCustomizationScripts)
            disabled = true;
    }
    return (disabled && ((_scriptState & scriptState_error) == 0));
}

int CScriptObject::getScriptType() const
{
    return (_scriptType);
}

void CScriptObject::flagForDestruction()
{
    _flaggedForDestruction = true;
}

bool CScriptObject::getFlaggedForDestruction() const
{
    return (_flaggedForDestruction);
}

bool CScriptObject::setScriptTextFromFile(const char* filename)
{
    std::string t;
    bool retVal = false;
    if (VFile::doesFileExist(filename))
    {
        try
        {
            VFile file(filename, VFile::READ | VFile::SHARE_DENY_NONE);
            VArchive archive(&file, VArchive::LOAD);
            unsigned int archiveLength = (unsigned int)file.getLength();
            t.resize(archiveLength, ' ');
            for (int i = 0; i < int(archiveLength); i++)
                archive >> t[i];
            archive.close();
            file.close();
            retVal = true;
        }
        catch (VFILE_EXCEPTION_TYPE)
        {
        }
    }
    setScriptText(t.c_str());
    return (retVal);
}

void CScriptObject::setScriptText(const char* scriptTxt, bool toFileIfApplicable /*= true*/)
{
    bool diff = false;
    if (scriptTxt == nullptr)
        diff = (_scriptText != "");
    else
        diff = (_scriptText != scriptTxt);
    if (diff)
    {
        _scriptText = "";
        if (scriptTxt != nullptr)
            _scriptText = scriptTxt;
        if (isNotInCopyBuffer() && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propScriptObj_code.name;
            CCbor* ev;
            if (_scriptHandle <= SIM_IDEND_SCENEOBJECT)
                ev = App::worldContainer->createSceneObjectChangedEvent(_scriptHandle, false, cmd, true); // scene object type scripts (new)
            else
                ev = App::worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED, _scriptHandle, _scriptUid, cmd, true); // main, sandbox, add-ons, and old-type scripts
            ev->appendKeyText(cmd, scriptTxt);
            App::worldContainer->pushEvent();
        }
        if (toFileIfApplicable)
            fromBufferToFile();
    }
}

const char* CScriptObject::getScriptText()
{
    fromFileToBuffer();
    return (_scriptText.c_str());
}

int CScriptObject::getScriptHandle() const
{
    return (_scriptHandle);
}

long long int CScriptObject::getScriptUid() const
{
    return _scriptUid;
}

size_t CScriptObject::getSimpleHash() const
{
    std::hash<std::string> hasher;
    return (hasher(_scriptText));
}

bool CScriptObject::isSimulatonCustomizationOrMainScript() const
{
    return (isSimulationOrMainScript() || (_scriptType == sim_scripttype_customization));
}

std::string CScriptObject::getDescriptiveName() const
{
    std::string retVal;
    if (_scriptType == sim_scripttype_main)
        retVal += "Main script";
    if ((_scriptType == sim_scripttype_simulation) || (_scriptType == sim_scripttype_customization) || (_scriptType == sim_scripttype_passive))
    {
        if (_scriptType == sim_scripttype_simulation)
            retVal += "Simulation script";
        else if (_scriptType == sim_scripttype_customization)
            retVal += "Customization script";
        else
            retVal += "Passive script";
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectHandle);
        if (it == nullptr)
            retVal += " (unassociated)";
        else
        {
            retVal += " \"";
            retVal += it->getObjectAlias_printPath();
            retVal += "\"";
        }
    }
    if (_scriptType == sim_scripttype_addon)
    {
        retVal += "Add-on script \"";
        retVal += _addOnMenuPath;
        retVal += "\"";
    }
    if (_scriptType == sim_scripttype_addonfunction)
    {
        retVal += "Add-on function \"";
        retVal += _addOnMenuPath;
        retVal += "\"";
    }
    if (_scriptType == sim_scripttype_sandbox)
        retVal += "Sandbox script";
    return (retVal);
}

std::string CScriptObject::getShortDescriptiveName() const
{
    std::string retVal;
    if (_scriptType == sim_scripttype_main)
        retVal += "mainScript";
    if ((_scriptType == sim_scripttype_simulation) || (_scriptType == sim_scripttype_customization))
    {
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectHandle);
        if (it == nullptr)
            retVal += "???";
        else
            retVal += it->getObjectAlias_printPath();
        if (!_sceneObjectScript)
        { // old scripts
            if (_scriptType == sim_scripttype_simulation)
                retVal += "@simulationScript";
            else if (_scriptType == sim_scripttype_customization)
                retVal += "@customizationScript";
            else
                retVal += "@passiveScript";
        }
    }
    if (_scriptType == sim_scripttype_addon)
    {
        retVal += _addOnMenuPath;
        retVal += "@addOnScript";
    }
    if (_scriptType == sim_scripttype_addonfunction)
    {
        retVal += _addOnMenuPath;
        retVal += "@addOnFunction";
    }
    if (_scriptType == sim_scripttype_sandbox)
        retVal += "sandboxScript";
    return (retVal);
}

void CScriptObject::setDisplayAddOnName(const char* name)
{
    _addOnMenuName = name;           // e.g. "Animation capture"
    _addOnMenuPath = _addOnMenuName; // e.g. "Tools >> Blabla" (if sysCall_info returns menu = 'Tools\nBlabla')
}

void CScriptObject::performScriptLoadingMapping(const std::map<int, int>* map, int opType)
{
    if (opType == 3)
        _scriptHandle = CWorld::getLoadingMapping(map, _scriptHandle); // model save
}

void CScriptObject::performSceneObjectLoadingMapping(const std::map<int, int>* map)
{
    if (App::currentWorld->sceneObjects != nullptr)
        _sceneObjectHandle = CWorld::getLoadingMapping(map, _sceneObjectHandle);
}

bool CScriptObject::announceSceneObjectWillBeErased(const CSceneObject* object, bool copyBuffer)
{ // script will be erased if attached to object (if threaded simulation is not running!)
    bool retVal = false;
    if (copyBuffer)
        retVal = (_sceneObjectHandle == object->getObjectHandle());
    else
    {
        bool closeCodeEditor = false;
        if (_sceneObjectHandle == object->getObjectHandle())
        {
            closeCodeEditor = true;
            _flaggedForDestruction = true;
            retVal = (_executionDepth == 0);
        }
        if (closeCodeEditor)
        {
#ifdef SIM_WITH_GUI
            if (GuiApp::mainWindow != nullptr)
                GuiApp::mainWindow->codeEditorContainer->closeFromScriptUid(_scriptUid, _previousEditionWindowPosAndSize, true);
#endif
        }
    }
    return (retVal);
}

int CScriptObject::flagScriptForRemoval()
{ // for old scripts. retVal: 0--> cannot be removed, 1 --> will be removed in a delayed manner, 2--> can be removed now
#ifdef SIM_WITH_GUI
    if (GuiApp::mainWindow != nullptr)
        GuiApp::mainWindow->codeEditorContainer->closeFromScriptUid(_scriptUid, _previousEditionWindowPosAndSize, true);
#endif

    if (App::currentWorld->simulation->isSimulationStopped())
    {
        if (isSimulationOrMainScript())
            return (2);
    }
    if (_scriptType == sim_scripttype_customization)
    {
        _flaggedForDestruction = true;
        return (1);
    }
    return (0);
}

int CScriptObject::getObjectHandleThatScriptIsAttachedTo(int scriptTypeToConsider) const
{
    int retVal = -1;
    if ((scriptTypeToConsider == -1) || (_scriptType == scriptTypeToConsider))
        retVal = _sceneObjectHandle;
    return (retVal);
}

void CScriptObject::setObjectHandleThatScriptIsAttachedTo(int newObjectHandle)
{
    if (newObjectHandle != -1)
    {
        if ((_scriptType == sim_scripttype_simulation) || (_scriptType == sim_scripttype_customization))
            _sceneObjectHandle = newObjectHandle;
    }
    else
        _sceneObjectHandle = -1;
}

int CScriptObject::getNumberOfPasses() const
{
    return _numberOfPasses;
}

void CScriptObject::setNumberOfPasses(int p)
{
    bool diff = (_numberOfPasses != p);
    if (diff)
        _numberOfPasses = p;
}

void CScriptObject::resetCalledInThisSimulationStep()
{
    _calledInThisSimulationStep = false;
}

bool CScriptObject::getCalledInThisSimulationStep() const
{
    return (_calledInThisSimulationStep);
}

int CScriptObject::systemCallMainScript(int optionalCallType, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{ // retval: -2: compil error, -1: runtimeError, 0: function not there or script not executed, 1: ok
    TRACE_INTERNAL;

    int retVal = -1;
    if (optionalCallType == -1)
    {
        App::currentWorld->sceneObjects->resetScriptFlagCalledInThisSimulationStep();
        int startT = int(VDateTime::getTimeInMs());

        if (_scriptState < scriptState_initialized)
            retVal = systemCallScript(sim_syscb_init, inStack, outStack);

        retVal = systemCallScript(sim_syscb_actuation, inStack, outStack);
        App::worldContainer->dispatchEvents(); // make sure that remote worlds reflect CoppeliaSim's state before sensing
        retVal = systemCallScript(sim_syscb_sensing, inStack, outStack);

        if (App::currentWorld->simulation->getSimulationState() == sim_simulation_advancing_lastbeforestop)
            retVal = systemCallScript(sim_syscb_cleanup, inStack, outStack);

        App::worldContainer->calcInfo->setMainScriptExecutionTime(int(VDateTime::getTimeInMs()) - startT);
        App::worldContainer->calcInfo->setSimulationScriptExecCount(App::currentWorld->sceneObjects->getCalledScriptsCountInThisSimulationStep(true));
    }
    else
        retVal = systemCallScript(optionalCallType, inStack, outStack);
    return (retVal);
}

int CScriptObject::systemCallScript(int callType, const CInterfaceStack* inStack, CInterfaceStack* outStack,
                                    bool addOnManuallyStarted /*=false*/)
{ // retval: -2: compil error, -1: runtimeError, 0: function not there or script not executed, 1: ok
    TRACE_INTERNAL;
    if (_tempSuspended)
        return 0;
    if ((_scriptType == sim_scripttype_addon) && (_scriptState == scriptState_unloaded) &&
        (callType != sim_syscb_info))
    {
        _handleInfoCallback();
        if ((!addOnManuallyStarted) && (_autoStartAddOn != 1))
            return (0);
    }

    if ((callType == sim_syscb_init) && (_scriptType == sim_scripttype_addon))
        CSimFlavor::getIntVal_str(3, _addOnMenuName.c_str());

    int retVal = 0;

    bool scriptDisabled = _scriptIsDisabled;
    if ((_scriptType == sim_scripttype_customization) && (!App::userSettings->runCustomizationScripts))
        scriptDisabled = true;
    if ((!scriptDisabled) || (((_scriptState & 7) == scriptState_initialized) && (callType == sim_syscb_cleanup)))
    { // Only cleanup call allowed when script is not enabled (e.g. when the used disabled it temporarily)
        if ((_scriptState == scriptState_unloaded) && (callType <= sim_syscb_sensing))
        { // First execute the script chunk if not yet done
            if (_loadCode())
            {
                if (callType == sim_syscb_info)
                    retVal = _callSystemScriptFunction(sim_syscb_info, inStack, outStack);
                else
                {
                    if (_scriptType != sim_scripttype_addon)
                        retVal = _callSystemScriptFunction(sim_syscb_info, nullptr,
                                                           nullptr); // implicit call, not for add-ons
                }
            }
            else
                retVal = -2;
        }
        if ((_scriptState == scriptState_uninitialized) && (callType != sim_syscb_info) &&
            (callType <= sim_syscb_sensing))
        { // Second execute the init section if not yet done
            if (callType == sim_syscb_init)
                retVal = _callSystemScriptFunction(sim_syscb_init, inStack, outStack);
            else
                retVal = _callSystemScriptFunction(sim_syscb_init, nullptr, nullptr); // implicit call
        }
        if (((_scriptState & 7) == scriptState_initialized) && (callType != sim_syscb_info) && (callType != sim_syscb_init))
        { // Execute the script call
            if (((_scriptState & scriptState_error) == 0) || (callType == sim_syscb_cleanup))
            {
                if ((callType != sim_syscb_event) || hasSystemFunctionOrHook(sim_syscb_event))
                {
                    retVal = _callSystemScriptFunction(callType, inStack, outStack);
                    if (_scriptType == sim_scripttype_sandbox)
                        setScriptState(_scriptState & 7); // remove a possible error flag
                }
                else
                    retVal = 0;
            }
        }
    }

    if (_addOnUiMenuHandle != -1)
    {
        CModuleMenuItem* m = App::worldContainer->moduleMenuItemContainer->getItemFromHandle(_addOnUiMenuHandle);
        if (m != nullptr)
        {
            std::string txt(_addOnMenuPath);
            while (true)
            {
                size_t p = txt.find(" >> ");
                if (p == std::string::npos)
                    break;
                txt.erase(0, p + 4);
            }

            if (_scriptState == CScriptObject::scriptState_initialized)
                txt += " (running)";
            if ((_scriptState & CScriptObject::scriptState_error) != 0)
                txt += " (error)";
            if ((_scriptState & CScriptObject::scriptState_suspended) != 0)
                txt += " (suspended)";
            m->setLabel(txt.c_str());
        }
    }

    return (retVal);
}

bool CScriptObject::shouldTemporarilySuspendMainScript()
{
    bool retVal = false;
    if (_scriptType == sim_scripttype_sandbox)
        setScriptState(_scriptState & 7); // remove a possible error flag
    CInterfaceStack* outStack = App::worldContainer->interfaceStackContainer->createStack();
    _callSystemScriptFunction(sim_syscb_beforemainscript, nullptr, outStack);
    bool doNotRunMainScript;
    if (outStack->getStackMapBoolValue("doNotRunMainScript", doNotRunMainScript))
    {
        if (doNotRunMainScript)
            retVal = true;
    }
    App::worldContainer->interfaceStackContainer->destroyStack(outStack);
    return (retVal);
}

void CScriptObject::_handleInfoCallback()
{
    if (_autoStartAddOn == -1)
    { // do this only once!
        CInterfaceStack* outStack = App::worldContainer->interfaceStackContainer->createStack();
        systemCallScript(sim_syscb_info, nullptr, outStack);
        resetScript();
        bool boolVal = true;
        outStack->getStackMapBoolValue("autoStart", boolVal);
        if (boolVal)
            _autoStartAddOn = 1;
        else
            _autoStartAddOn = 0;
        std::string menuEntry(_addOnMenuName);
        outStack->getStackMapStringValue("menu", menuEntry);
        if (menuEntry.size() > 0)
        { // might contain also path info, e.g. "Exporters/nURDF exporter"
            _addOnMenuPath = menuEntry;
            size_t r = _addOnMenuPath.find("\n");
            while (r != std::string::npos)
            {
                _addOnMenuPath.replace(r, 1, " >> ");
                r = _addOnMenuPath.find("\n");
            }
            _addOnUiMenuHandle = App::worldContainer->moduleMenuItemContainer->addMenuItem(menuEntry.c_str(), -1);
        }
        boolVal = true;
        outStack->getStackMapBoolValue("menuEnabled", boolVal);
        if ((_addOnUiMenuHandle != -1) && (!boolVal))
            App::worldContainer->moduleMenuItemContainer->getItemFromHandle(_addOnUiMenuHandle)->setState(0);

        App::worldContainer->interfaceStackContainer->destroyStack(outStack);
    }
}

int CScriptObject::___loadCode(const char* code, const char* functionsToFind, std::vector<bool>& functionsFound,
                               std::string* errorMsg)
{ // retVal: -1=compil error, 0=runtime error, 1=no error
    int retVal = -1;

    std::string _code(code);
    if (_lang == "python")
    {
        int lineCnt = 0;
        std::string l;
        std::string tmpCode(code);
        std::string t("wrapper='pythonWrapperV2'\n"); // default wrapper
        bool stayIn = true;
        while (stayIn && utils::extractLine(tmpCode, l))
        {
            lineCnt++;
            utils::removeSpacesAtBeginningAndEnd(l);
            if (l.size() != 0)
            {
                if (l[0] == '#')
                {
                    l.erase(l.begin());
                    utils::removeSpacesAtBeginningAndEnd(l);
                    if (l.find("luaExec ") == 0)
                    {
                        l.erase(l.begin(), l.begin() + 8);
                        utils::removeSpacesAtBeginningAndEnd(l);
                        t = t + l;
                    }
                    t = t + "\n";
                }
                else if (l.compare(0, 3, "'''") == 0)
                {
                    l.erase(l.begin(), l.begin() + 3);
                    utils::removeSpacesAtBeginningAndEnd(l);
                    bool isLua = ((l.find("luaExec ") == 0) || (l.compare("luaExec") == 0));
                    bool stayIn2 = true;
                    if (isLua)
                    {
                        l.erase(l.begin(), l.begin() + 7);
                        size_t p = l.find("'''");
                        if (p != std::string::npos)
                        {
                            stayIn2 = false;
                            l.erase(l.begin() + p, l.end());
                        }
                        t = t + l;
                    }
                    t = t + "\n";
                    while (stayIn2 && utils::extractLine(tmpCode, l))
                    {
                        lineCnt++;
                        if (!isLua)
                        {
                            utils::removeSpacesAtBeginningAndEnd(l);
                            isLua = ((l.find("luaExec ") == 0) || (l.compare("luaExec") == 0));
                            if (isLua)
                                l.erase(l.begin(), l.begin() + 7);
                        }
                        size_t p = l.find("'''");
                        if (p != std::string::npos)
                        {
                            stayIn2 = false;
                            l.erase(l.begin() + p, l.end());
                        }
                        if (isLua)
                            t = t + l;
                        t = t + "\n";
                    }
                }
                else
                {
                    if (l.find("include ") == 0)
                    {
                        l.erase(l.begin(), l.begin() + 8);
                        utils::removeSpacesAtBeginningAndEnd(l);
                        if (l.size() > 0)
                            t = t + "pythonFile='" + l + ".py'\n";
                    }
                    else
                        lineCnt--;
                    stayIn = false;
                }
            }
            else
                t = t + "\n";
        }
        for (int i = 0; i < lineCnt; i++)
            utils::extractLine(_code, l);
        for (int i = 0; i < lineCnt; i++)
            _code = "#\n" + _code;
        // printf("luaExec:\n%s\n",t.c_str());
        // printf("code:\n%s\n",_code.c_str());
        _code = t + "\nrequire(wrapper) pythonUserCode=[=[" + _code +
                "]=] if pythonFile and #pythonFile>1 then loadExternalFile(pythonFile) end";
    }
    luaWrap_lua_State* L = (luaWrap_lua_State*)_interpreterState;
    int oldTop = luaWrap_lua_gettop(L);
    std::string tmp("sim_call_type="); // for backward compatibility
    tmp += std::to_string(sim_syscb_init);
    _execSimpleString_safe_lua(L, tmp.c_str());
    if (_loadBuffer_lua(_code.c_str(), _code.size(), getShortDescriptiveName().c_str()))
    {
        if (_executionDepth == 0)
            _timeOfScriptExecutionStart = int(VDateTime::getTimeInMs());
        setExecutionDepth(_executionDepth + 1);
        if (_callScriptFunction(-1, "", nullptr, nullptr, errorMsg) == -1)
            retVal = 0; // a runtime error occurred!
        else
        { // here we check if we can enable the new calling method:
            retVal = 1;

            // base.lua already checked for usage of "sim.syscb_init" and similar consts

            // We check if any of the system callback function is present to rule out compatibility mode:
            bool foundCallbackMech = false;
            std::vector<int> sysCb = getAllSystemCallbacks(-1, false);
            for (size_t i = 0; i < sysCb.size(); i++)
            {
                std::string str = getSystemCallbackString(sysCb[i], 0);
                luaWrap_lua_getglobal(L, str.c_str());
                foundCallbackMech = luaWrap_lua_isfunction(L, -1);
                luaWrap_lua_pop(L, 1);
                if (foundCallbackMech)
                    break;
            }

            _execSimpleString_safe_lua(L, "sim_call_type=nil");
            size_t off = 0;
            size_t l = strlen(functionsToFind + off);
            size_t cnt = 0;
            while (l != 0)
            {
                luaWrap_lua_getglobal(L, functionsToFind + off);
                functionsFound[cnt++] = (luaWrap_lua_isfunction(L, -1) || hasFunctionHook(functionsToFind + off));
                luaWrap_lua_pop(L, 1);
                off += l + 1;
                l = strlen(functionsToFind + off);
            }
        }
        setExecutionDepth(_executionDepth - 1);
        if (_executionDepth == 0)
            _timeOfScriptExecutionStart = -1;
    }
    else
    { // A compilation error occurred!
        retVal = -1;
        if (errorMsg != nullptr)
        {
            if (luaWrap_lua_isstring(L, -1))
                errorMsg[0] = std::string(luaWrap_lua_tostring(L, -1));
            else
                errorMsg[0] = "(error unknown)";
        }
        luaWrap_lua_pop(L, 1); // pop error from stack
    }
    luaWrap_lua_settop(L, oldTop); // We restore lua's stack

    return (retVal);
}

bool CScriptObject::_loadCode()
{
    if (_scriptState == scriptState_unloaded)
    {
        fromFileToBuffer();
        _scriptTextExec.assign(_scriptText.begin(), _scriptText.end());
        std::string intStateErr;
        if (_initInterpreterState(&intStateErr))
        {
            std::string functions;
            for (size_t i = 0; i < sim_syscb_endoflist; i++)
                functions += getSystemCallbackString(int(i), 0) + '\0';
            functions += '\0';
            std::string errMsg;
            int r = ___loadCode(_scriptTextExec.c_str(), functions.c_str(), _containedSystemCallbacks, &errMsg);
            if (r >= 0)
            {
                if (r == 0)
                { // a runtime error occurred!
                    setScriptState(_scriptState | scriptState_error);
                    _announceErrorWasRaisedAndPossiblyPauseSimulation(errMsg.c_str(), true);
                }
                else
                { // success
#ifdef SIM_WITH_GUI
                    GuiApp::setRefreshHierarchyViewFlag();
#endif
                    setScriptState(scriptState_uninitialized);
                    // Following because below funcs are speed-sensitive:
                    if (hasSystemFunction(sim_syscb_event, false))
                        setFuncAndHookCnt(sim_syscb_event, 0, 1);
                    if ((hasSystemFunction(sim_syscb_dyn, false)) ||
                        (hasSystemFunction(sim_syscb_dyncallback, false)))
                        setFuncAndHookCnt(sim_syscb_dyn, 0, 1);
                    if ((hasSystemFunction(sim_syscb_contact, false)) ||
                        (hasSystemFunction(sim_syscb_contactcallback, false)))
                        setFuncAndHookCnt(sim_syscb_contact, 0, 1);
                    if ((hasSystemFunction(sim_syscb_joint, false)) ||
                        (hasSystemFunction(sim_syscb_jointcallback, false)))
                        setFuncAndHookCnt(sim_syscb_joint, 0, 1);
                }
                setNumberOfPasses(_numberOfPasses + 1);
            }
            else
            { // A compilation/load error occurred!
                setScriptState(_scriptState | scriptState_error);
                _announceErrorWasRaisedAndPossiblyPauseSimulation(errMsg.c_str(), false);
            }
        }
        else
        { // The interpreter state could not be created!
            setScriptState(_scriptState | scriptState_error);
            _announceErrorWasRaisedAndPossiblyPauseSimulation(intStateErr.c_str(), false);
        }
    }

    if ((_scriptState & scriptState_error) != 0)
        _killInterpreterState();
    return ((_scriptState == scriptState_uninitialized) || (_scriptState == scriptState_initialized));
}

int CScriptObject::_callSystemScriptFunction(int callType, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{ // retval: -1: runtimeError, 0: function not there or not called, 1: ok
    if (callType == sim_syscb_info)
    {
        if (_scriptState != scriptState_uninitialized)
            return (0);
    }
    else if (callType == sim_syscb_init)
    {
#ifdef SIM_WITH_GUI
        GuiApp::setRefreshHierarchyViewFlag();
#endif
        if (_scriptState != scriptState_uninitialized)
            return (0);
        setScriptState(scriptState_initialized);
    }
    else
    {
        if ((_scriptState & 7) != scriptState_initialized)
            return (0);
        if (callType == sim_syscb_cleanup)
        {
            int ss = (_scriptState & scriptState_error); // keep the error flag
            ss |= scriptState_ended;                     // set the ended state
            setScriptState(ss);
        }
        else
        {
            if ((_scriptState & scriptState_error) != 0)
                return (0);
            if (callType == sim_syscb_aos_resume)
                setScriptState((_scriptState | scriptState_suspended) - scriptState_suspended);
            if ((_scriptState & scriptState_suspended) != 0)
                return (0);
            if (callType == sim_syscb_aos_suspend)
                setScriptState(_scriptState | scriptState_suspended);
        }
    }

    if (_executionDepth == 0)
    { // remember: a script func. can call another script func indirectly via the system, even system callback can do
        // that!
        _timeForNextAutoYielding = int(VDateTime::getTimeInMs()) + _delayForAutoYielding;
        _forbidOverallYieldingLevel = 0;
    }
    else
        changeOverallYieldingForbidLevel(1, false);

    CInterfaceStack* _outStack = App::worldContainer->interfaceStackContainer->createStack();
    if (outStack == nullptr)
        outStack = _outStack;

    // ---------------------------------
    if (_scriptType == sim_scripttype_main)
    { // corresponding calls for plugins:
        int data[4] = {0, int(App::currentWorld->simulation->getSimulationTime() * 1000.0), 0, 0};
        if (callType == sim_syscb_init)
            App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(
                sim_message_eventcallback_simulationinit, data);
        if (callType == sim_syscb_actuation)
            App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(
                sim_message_eventcallback_simulationactuation, data);
        if (callType == sim_syscb_sensing)
            App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(
                sim_message_eventcallback_simulationsensing, data);
        if (callType == sim_syscb_cleanup)
            App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(
                sim_message_eventcallback_simulationcleanup, data);
    }

    // Following to make sure we get updates on the presence of sim_syscb_userconfig:
    luaWrap_lua_State* L = (luaWrap_lua_State*)_interpreterState;
    std::string tmp(getSystemCallbackString(sim_syscb_userconfig, 0));
    luaWrap_lua_getglobal(L, tmp.c_str());
    _containedSystemCallbacks[sim_syscb_userconfig] = luaWrap_lua_isfunction(L, -1);
    luaWrap_lua_pop(L, 1);

    std::string errMsg;
    if (_executionDepth == 0)
        _timeOfScriptExecutionStart = int(VDateTime::getTimeInMs());
    setExecutionDepth(_executionDepth + 1);
    int retVal = _callScriptFunction(callType, getSystemCallbackString(callType, 0).c_str(), inStack, outStack, &errMsg);
    setExecutionDepth(_executionDepth - 1);
    if (_executionDepth == 0)
        _timeOfScriptExecutionStart = -1;
    if (retVal != 0)
    {
        if (retVal == -1)
        { // a runtime error occurred!
            setScriptState(_scriptState | scriptState_error);
            _announceErrorWasRaisedAndPossiblyPauseSimulation(errMsg.c_str(), true);
            retVal = -1;
        }
        _calledInThisSimulationStep = true;
    }
    if (_scriptType == sim_scripttype_main)
    { // corresponding calls for plugins:
        int data[4] = {1, int(App::currentWorld->simulation->getSimulationTime() * 1000.0), 0, 0};
        if (callType == sim_syscb_init)
            App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(
                sim_message_eventcallback_simulationinit, data);
        if (callType == sim_syscb_actuation)
            App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(
                sim_message_eventcallback_simulationactuation, data);
        if (callType == sim_syscb_sensing)
            App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(
                sim_message_eventcallback_simulationsensing, data);
        if (callType == sim_syscb_cleanup)
            App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(
                sim_message_eventcallback_simulationcleanup, data);
    }
    // ---------------------------------

    if (_executionDepth != 0)
        changeOverallYieldingForbidLevel(-1, false);

    if (_executionDepth == 0)
    { // a system script func. could call a custom script function which could call a system script function, etc. Let
        // the calls unwind before doing anything heavy!
        if ((_scriptState & scriptState_error) != 0)
        { // We got an error
            if (callType == sim_syscb_info)
            {
                setScriptState(scriptState_ended | scriptState_error);
                _killInterpreterState();
            }
            else
            {
                if ((_scriptState & 7) != scriptState_ended)
                    _killInterpreterState();
            }
        }
        else
        {
            if (((_scriptState & 7) != scriptState_ended) && (callType != sim_syscb_info))
            {
                std::string cmd;
                if (outStack->getStackMapStringValue("cmd", cmd))
                {
                    if (cmd.compare("restart") == 0)
                        resetScript();
                    if (cmd.compare("cleanup") == 0)
                        _killInterpreterState();
                }
                else
                { // Following for backward compatibility with older add-ons: they could return 1 (sim_syscb_cleanup) to
                    // request cleanup
                    long long int theValue;
                    if ((_scriptType == sim_scripttype_addon) && (outStack->getStackStrictInt64Value(theValue)) &&
                        (theValue == 1))
                        _killInterpreterState();
                }
            }
        }
    }
    App::worldContainer->interfaceStackContainer->destroyStack(_outStack);
    return (retVal);
}

int CScriptObject::_callScriptFunction(int sysCallType, const char* functionName, const CInterfaceStack* inStack, CInterfaceStack* outStack, std::string* errorMsg)
{ // retVal: -1=error during execution, 0=func does not exist, 1=execution ok
    // This will also execute function hooks
    int retVal = 1;
    std::string err;
    for (size_t i = 0; i < _functionHooks_before.size() / 2; i++)
    {
        if (_functionHooks_before[2 * i + 0].compare(functionName) == 0)
        {
            int r = _callScriptFunc(_functionHooks_before[2 * i + 1].c_str(), inStack, nullptr, &err);
            if (r < 0)
            {
                retVal = r;
                if (errorMsg != nullptr)
                {
                    errorMsg[0] += err;
                    errorMsg[0] += "\n\n";
                }
            }
        }
    }
    int r = _callScriptFunc(functionName, inStack, outStack, &err);
    if (r <= retVal)
    {
        retVal = r;
        if (r < 0)
        {
            if (errorMsg != nullptr)
            {
                errorMsg[0] += err;
                errorMsg[0] += "\n\n";
            }
        }
    }
    for (size_t i = 0; i < _functionHooks_after.size() / 2; i++)
    {
        if (_functionHooks_after[2 * i + 0].compare(functionName) == 0)
        {
            int r = _callScriptFunc(_functionHooks_after[2 * i + 1].c_str(), inStack, nullptr, &err);
            if (r < 0)
            {
                retVal = r;
                if (errorMsg != nullptr)
                {
                    errorMsg[0] += err;
                    errorMsg[0] += "\n\n";
                }
            }
        }
    }

    if ((retVal == 1) && ((sysCallType == sim_syscb_init) || (sysCallType == sim_syscb_cleanup) ||
                          (sysCallType == sim_syscb_aos_suspend) || (sysCallType == sim_syscb_aos_resume) ||
                          (sysCallType == sim_syscb_userconfig)))
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();

        stack->pushTableOntoStack();

        stack->pushTextOntoStack("id"); // key or index
        stack->pushTextOntoStack("systemCall");
        stack->insertDataIntoStackTable();

        stack->pushTextOntoStack("data");
        stack->pushTableOntoStack();

        stack->pushTextOntoStack("callType"); // key or index
        stack->pushInt32OntoStack(sysCallType);
        stack->insertDataIntoStackTable();

        stack->pushTextOntoStack("script"); // key or index
        stack->pushInt32OntoStack(_scriptHandle);
        stack->insertDataIntoStackTable();

        stack->insertDataIntoStackTable();
        App::worldContainer->broadcastMsg(stack, _scriptHandle, 0);
        App::worldContainer->interfaceStackContainer->destroyStack(stack);
    }

    if ((errorMsg != nullptr) && (errorMsg[0].size() > 1) && boost::algorithm::ends_with(errorMsg->c_str(), "\n\n"))
    {
        errorMsg[0].pop_back();
        errorMsg[0].pop_back();
    }
    return (retVal);
}

int CScriptObject::_callScriptFunc(const char* functionName, const CInterfaceStack* inStack, CInterfaceStack* outStack,
                                   std::string* errorMsg)
{ // retVal: -1=error during execution, 0=func does not exist, 1=execution ok
    int retVal = 0;
    std::string func(functionName);

    luaWrap_lua_State* L = (luaWrap_lua_State*)_interpreterState;
    bool funcDoesNotExist = false;
    int oldTop = luaWrap_lua_gettop(L); // We store lua's stack
    // Push the function name onto the stack (will be automatically popped from stack after luaWrap_lua_pcall):
    if (func.size() > 0)
    {
        size_t ppos = func.find('.');
        if (ppos == std::string::npos)
            luaWrap_lua_getglobal(L, func.c_str()); // in case we have a global function:
        else
        { // in case we have a function that is not global
            std::string globalVar(func.begin(), func.begin() + ppos);
            luaWrap_lua_getglobal(L, globalVar.c_str());
            if (luaWrap_lua_isnonbuffertable(L, -1))
            {
                func.assign(func.begin() + ppos + 1, func.end());
                size_t ppos = func.find('.');
                while (ppos != std::string::npos)
                {
                    std::string var(func.begin(), func.begin() + ppos);
                    luaWrap_lua_getfield(L, -1, var.c_str());
                    luaWrap_lua_remove(L, -2);
                    func.erase(func.begin(), func.begin() + ppos + 1);
                    ppos = func.find('.');
                    if (!luaWrap_lua_isnonbuffertable(L, -1))
                    {
                        funcDoesNotExist = true;
                        break;
                    }
                }
                if (!funcDoesNotExist)
                {
                    luaWrap_lua_getfield(L, -1, func.c_str());
                    luaWrap_lua_remove(L, -2);
                }
            }
            else
                funcDoesNotExist = true;
        }
    }
    if ((func.size() == 0) || ((!funcDoesNotExist) && luaWrap_lua_isfunction(L, -1)))
    { // ok, the function exists, or we call the script chunk
        // Push the arguments onto the stack (will be automatically popped from stack after luaWrap_lua_pcall):
        int inputArgs = 0;
        if (inStack != nullptr)
        {
            inputArgs = inStack->getStackSize();
            if (inputArgs != 0)
                buildOntoInterpreterStack_lua(L, inStack, false);
        }
        luaWrap_lua_getglobal(L, "debug");
        luaWrap_lua_getfield(L, -1, "traceback");
        luaWrap_lua_remove(L, -2);
        int argCnt = inputArgs;
        int errindex = -argCnt - 2;
        luaWrap_lua_insert(L, errindex);
        if (luaWrap_lua_pcall((luaWrap_lua_State*)_interpreterState, argCnt, luaWrapGet_LUA_MULTRET(), errindex) != 0)
        { // a runtime error occurred!
            retVal = -1;
            if (errorMsg != nullptr)
            {
                if (luaWrap_lua_isstring(L, -1))
                {
                    errorMsg[0] = std::string(luaWrap_lua_tostring(L, -1));
                    size_t p = errorMsg[0].find("__]]__");
                    if (p != std::string::npos)
                        errorMsg[0] = errorMsg[0].substr(0, p);
                    p = errorMsg[0].find("__[[__");
                    if (p != std::string::npos)
                        errorMsg[0] = errorMsg[0].substr(p + 6);
                }
                else
                    errorMsg[0] = "(error unknown)";
            }
            luaWrap_lua_pop(L, 1); // pop error from stack
        }
        else
        { // execution went fine:
            retVal = 1;
            int currentTop = luaWrap_lua_gettop(L);
            int numberOfArgs = currentTop - oldTop - 1; // the first arg is linked to the debug mechanism
            if (outStack != nullptr)
                buildFromInterpreterStack_lua(L, outStack, oldTop + 1 + 1, numberOfArgs); // the first arg is linked to the debug mechanism
        }
    }
    luaWrap_lua_settop(L, oldTop); // We restore lua's stack

    return (retVal);
}

int CScriptObject::callCustomScriptFunction(const char* functionName, CInterfaceStack* inOutStack)
{ // retval: -1: runtimeError, 0: function not there or not executed, 1: ok
    int retVal = 0;
    if (_scriptState == scriptState_initialized)
    {
        changeOverallYieldingForbidLevel(1, false); // never yield from such a call
        CInterfaceStack* outStack = App::worldContainer->interfaceStackContainer->createStack();
        // -------------------------------------
        std::string errMsg;
        if (_executionDepth == 0)
            _timeOfScriptExecutionStart = int(VDateTime::getTimeInMs());
        setExecutionDepth(_executionDepth + 1);

        luaWrap_lua_State* L = (luaWrap_lua_State*)_interpreterState;
        luaWrap_lua_getglobal(L, "sysCall_ext");
        bool extFunc = luaWrap_lua_isfunction(L, -1);
        luaWrap_lua_pop(L, 1);
        if (extFunc || hasFunctionHook("sysCall_ext"))
        { // if sysCall_ext is present, the original func won't be called. Otherwise yes, independently of any such
            // hooks
            CInterfaceStack* inStack = nullptr;
            if (inOutStack)
                inStack = App::worldContainer->interfaceStackContainer->createStackCopy(inOutStack);
            else
                inStack = App::worldContainer->interfaceStackContainer->createStack();
            inStack->pushTextOntoStack(functionName, true);
            retVal = _callScriptFunction(-1, "sysCall_ext", inStack, outStack, &errMsg);
            App::worldContainer->interfaceStackContainer->destroyStack(inStack);
        }
        if (!extFunc)
            retVal = _callScriptFunction(-1, functionName, inOutStack, outStack, &errMsg);

        setExecutionDepth(_executionDepth - 1);
        if (_executionDepth == 0)
            _timeOfScriptExecutionStart = -1;
        if (retVal != 0)
        {
            if (retVal == -1)
            { // a runtime error occurred!
                setScriptState(_scriptState | scriptState_error);
                _announceErrorWasRaisedAndPossiblyPauseSimulation(errMsg.c_str(), true);
                retVal = -1;
            }
            _calledInThisSimulationStep = true;
        }
        // -------------------------------------

        if (inOutStack != nullptr)
            inOutStack->clear();
        if (_scriptType == sim_scripttype_sandbox)
            setScriptState(_scriptState & 7); // remove a possible error flag
        if ((retVal == -1) && (_scriptType != sim_scripttype_sandbox))
        {
            if (_executionDepth == 0)
                _killInterpreterState();
        }
        else
        {
            if (inOutStack != nullptr)
                inOutStack->copyFrom(outStack);
        }
        App::worldContainer->interfaceStackContainer->destroyStack(outStack);
        changeOverallYieldingForbidLevel(-1, false);
    }
    return (retVal);
}

int CScriptObject::executeScriptString(const char* scriptString, CInterfaceStack* outStack)
{ // retVal: -2: script not initialized, is disabled, or had previously an error, -1: string caused an error, 0: string
    // didn't cause an error
    int retVal = -2;
    changeOverallYieldingForbidLevel(1, false);
    if (_scriptState == scriptState_initialized)
    {
        if (_executionDepth == 0)
            _timeOfScriptExecutionStart = int(VDateTime::getTimeInMs());
        setExecutionDepth(_executionDepth + 1);
        if (_execScriptString(scriptString, outStack))
            retVal = 0; // success
        else
            retVal = -1;
        setExecutionDepth(_executionDepth - 1);
        if (_executionDepth == 0)
            _timeOfScriptExecutionStart = -1;
    }
    changeOverallYieldingForbidLevel(-1, false);
    return (retVal);
}

bool CScriptObject::_execScriptString(const char* scriptString, CInterfaceStack* outStack)
{ // retVal: success, otherwise error
    bool retVal = false;

    luaWrap_lua_State* L = (luaWrap_lua_State*)_interpreterState;
    int oldTop = luaWrap_lua_gettop(L); // We store lua's stack
    std::string theString("return ");
    theString += scriptString;
    int loadBufferRes = luaWrap_luaL_loadbuffer(L, theString.c_str(), theString.size(), scriptString);
    if (loadBufferRes != 0)
    {
        luaWrap_lua_settop(L, oldTop); // We restore lua's stack
        loadBufferRes = luaWrap_luaL_loadbuffer(L, scriptString, strlen(scriptString), scriptString);
    }
    if (loadBufferRes == 0)
    {
        luaWrap_lua_getglobal(L, "debug");
        luaWrap_lua_getfield(L, -1, "traceback");
        luaWrap_lua_remove(L, -2);
        int errindex = -2;
        luaWrap_lua_insert(L, errindex);
        if (luaWrap_lua_pcall(L, 0, luaWrapGet_LUA_MULTRET(), errindex) != 0)
        { // a runtime error occurred!
            std::string errMsg;
            if (luaWrap_lua_isstring(L, -1))
                errMsg = std::string(luaWrap_lua_tostring(L, -1));
            else
                errMsg = "(error unknown)";
            if (outStack != nullptr)
            {
                outStack->clear();
                outStack->pushTextOntoStack(errMsg.c_str());
            }
        }
        else
        {
            retVal = true;
            int currentTop = luaWrap_lua_gettop(L);
            int numberOfArgs = currentTop - oldTop - 1; // the first arg is linked to the debug mechanism
            if (outStack != nullptr)
                buildFromInterpreterStack_lua(L, outStack, oldTop + 1 + 1,
                                              numberOfArgs); // the first arg is linked to the debug mechanism
        }
    }
    else
    { // A compilation error occurred!
        std::string errMsg;
        if (luaWrap_lua_isstring(L, -1))
            errMsg = std::string(luaWrap_lua_tostring(L, -1));
        else
            errMsg = "(error unknown)";
        if (outStack != nullptr)
        {
            outStack->clear();
            outStack->pushTextOntoStack(errMsg.c_str());
        }
    }
    luaWrap_lua_settop(L, oldTop); // We restore lua's stack

    return (retVal);
}

bool CScriptObject::hasInterpreterState() const
{
    return (_interpreterState != nullptr);
}

bool CScriptObject::isSimulationOrMainScript() const
{
    return ((_scriptType == sim_scripttype_main) || (_scriptType == sim_scripttype_simulation));
}

bool CScriptObject::isSceneSwitchPersistentScript() const
{
    return ((_scriptType == sim_scripttype_sandbox) || (_scriptType == sim_scripttype_addon));
}

void CScriptObject::setIsSceneObjectScript(bool s)
{
    _sceneObjectScript = s;
}

bool CScriptObject::resetScript()
{
    bool retVal = _killInterpreterState();
    fromFileToBuffer();
    setScriptState(scriptState_unloaded);
    return retVal;
}

void CScriptObject::initScript()
{ // add-on scripts won't reload, just reinitialize
    resetScript();
    if ((_scriptType == sim_scripttype_simulation) || (_scriptType == sim_scripttype_customization) || (_scriptType == sim_scripttype_sandbox))
    {
        if (_scriptType == sim_scripttype_sandbox)
        {
            App::logMsg(sim_verbosity_loadinfos | sim_verbosity_onlyterminal, "initializing the sandbox script...");
            _initInterpreterState(nullptr);
            if (App::userSettings->preferredSandboxLang == "bareLua")
            {
                _lang = "lua";
                if (setScriptTextFromFile((App::folders->getLuaPath() + "/" + BASE_SANDBOX_SCRIPT).c_str()))
                {
                    if (systemCallScript(sim_syscb_init, nullptr, nullptr) >= 0) // init could be missing, but using an init-hook!
                        App::logMsg(sim_verbosity_loadinfos | sim_verbosity_onlyterminal, "'bareLua' sandbox script initialized.");
                }
                else
                {
                    _scriptIsDisabled = true;
                    App::logMsg(sim_verbosity_loadinfos | sim_verbosity_onlyterminal, (std::string(BASE_SANDBOX_SCRIPT) + " was not found.").c_str());
                }
            }
            else
            {
                _lang = "python";
                if (setScriptTextFromFile((App::folders->getPythonPath() + "/sandboxScript.py").c_str()))
                {
                    if (systemCallScript(sim_syscb_init, nullptr, nullptr) >= 0) // init could be missing, but using an init-hook!
                        App::logMsg(sim_verbosity_loadinfos | sim_verbosity_onlyterminal, "sandbox script initialized.");
                    else
                    { // we revert to bareLua
                        _lang = "lua";
                        if (setScriptTextFromFile((App::folders->getLuaPath() + "/" + BASE_SANDBOX_SCRIPT).c_str()))
                        {
                            resetScript();
                            if (systemCallScript(sim_syscb_init, nullptr, nullptr) >= 0) // init could be missing, but using an init-hook!
                                App::logMsg(sim_verbosity_loadinfos | sim_verbosity_onlyterminal, "'bareLua' sandbox script initialized (Python sandbox failed).");
                        }
                        else
                        {
                            _scriptIsDisabled = true;
                            App::logMsg(sim_verbosity_loadinfos | sim_verbosity_onlyterminal, (std::string(BASE_SANDBOX_SCRIPT) + " was not found (Python sandbox failed).").c_str());
                        }
                    }
                }
                else
                    App::logMsg(sim_verbosity_loadinfos | sim_verbosity_onlyterminal, "sandboxScript.py was not found.");
            }
        }
        else
            systemCallScript(sim_syscb_init, nullptr, nullptr);
    }
}

bool CScriptObject::getIsUpToDate()
{
    bool retVal = true;
    if (hasInterpreterState() && (_scriptTextExec.compare(getScriptText()) != 0))
        retVal = false;
    return (retVal);
}

bool CScriptObject::_killInterpreterState()
{
    bool retVal = (_scriptState != scriptState_unloaded);
    if (_interpreterState != nullptr)
    {
        if ((_scriptState & 7) == scriptState_initialized)
        {
            if (isSimulatonCustomizationOrMainScript() || (_scriptType == sim_scripttype_addon))
                _callSystemScriptFunction(sim_syscb_cleanup, nullptr, nullptr);
            // if (_scriptType==sim_scripttype_addonfunction) // Not needed
            // if (_scriptType==sim_scripttype_sandbox) // Not needed
        }
        App::worldContainer->announceScriptStateWillBeErased(_scriptHandle, _scriptUid, isSimulationOrMainScript(), isSceneSwitchPersistentScript());
        luaWrap_lua_close((luaWrap_lua_State*)_interpreterState);
        _interpreterState = nullptr;
    }

    int ss = (_scriptState & scriptState_error); // keep the error flag
    ss |= scriptState_ended;                     // set the ended state
    setScriptState(ss);
    _scriptTextExec.clear();
    setExecutionDepth(0);

    for (size_t i = 0; i < 3; i++)
    {
        setFuncAndHookCnt(sim_syscb_event, i, 0);
        setFuncAndHookCnt(sim_syscb_dyn, i, 0);
        setFuncAndHookCnt(sim_syscb_contact, i, 0);
        setFuncAndHookCnt(sim_syscb_joint, i, 0);
    }

    _containedSystemCallbacks.clear();
    _containedSystemCallbacks.resize(sim_syscb_endoflist, false);
    _flaggedForDestruction = false;
    _functionHooks_before.clear();
    _functionHooks_after.clear();

    _loadBufferResult_lua = -1;
    setNumberOfPasses(0);
#ifdef SIM_WITH_GUI
    GuiApp::setRefreshHierarchyViewFlag();
#endif
    if ((_scriptState & scriptState_error) && _autoRestartOnError)
    {
        setAutoRestartOnError(false);
        setScriptState(scriptState_unloaded);
    }

    std::vector<std::pair<std::string, int>> toRem;
    for (const auto& entry : _signalNameToScriptHandle)
    {
        if (entry.second.first == _scriptHandle)
            toRem.push_back(std::make_pair(entry.first, entry.second.second));
    }
    for (size_t i = 0; i < toRem.size(); i++)
    {
        std::string nn(toRem[i].first);
        signalRemoved(nn.c_str());
        utils::replaceSubstringStart(nn, "app.", "");
        utils::replaceSubstringStart(nn, "obj.", "");
        CALL_C_API_CLEAR_ERRORS(simRemoveProperty, toRem[i].second, nn.c_str());
    }
    _eventFilters.clear();

    return (retVal);
}

CScriptObject* CScriptObject::copyYourself()
{
    CScriptObject* it = new CScriptObject(_scriptType);

    it->_scriptType = _scriptType;
    // it->_scriptHandle=_scriptHandle;
    it->_tempSuspended = true;
    it->_sceneObjectHandle = _sceneObjectHandle;
    it->_scriptIsDisabled = _scriptIsDisabled;
    it->_parentIsProxy = _parentIsProxy;
    it->setScriptText(getScriptText());
    it->_lang = _lang;
    it->_scriptObjectInitialValuesInitialized = _scriptObjectInitialValuesInitialized;
    it->_addOnExecPriority = _addOnExecPriority;

    it->_executionPriority_old = _executionPriority_old;
    delete it->_customObjectData_old;
    it->_customObjectData_old = nullptr;
    if (_customObjectData_old != nullptr)
        it->_customObjectData_old = _customObjectData_old->copyYourself();
    return (it);
}

bool CScriptObject::addCommandToOutsideCommandQueue(int commandID, int auxVal1, int auxVal2, int auxVal3, int auxVal4,
                                                    const double aux2Vals[8], int aux2Count)
{
    if (_outsideCommandQueue != nullptr)
        return (_outsideCommandQueue->addCommand(commandID, auxVal1, auxVal2, auxVal3, auxVal4, aux2Vals, aux2Count));
    return (true);
}

void CScriptObject::setEventFilters(const std::map<long long int, std::set<std::string>>& filters)
{
    _eventFilters = filters;
}

bool CScriptObject::prepareFilteredEventsBuffer(const std::vector<unsigned char>& input, const std::vector<SEventInf>& inf, std::vector<unsigned char>& output) const
{
    bool retVal = false;
    if (_eventFilters.size() > 0)
    {
        long long int mainScriptHandle = -1;
        CScriptObject* mainScript = App::currentWorld->sceneObjects->embeddedScriptContainer->getMainScript();
        if (mainScript != nullptr)
            mainScriptHandle = mainScript->getScriptHandle();

        output.push_back(input[0]); // "array open" (holding all events)
        size_t p = 1;
        for (size_t ev = 0; ev < inf.size(); ev++)
        {
            long long int t = inf[ev].target;
            long long int altT = t;
            if ((t >= 0) && (t <= SIM_IDEND_SCENEOBJECT))
                altT = sim_handle_sceneobject;
            else if ((t >= SIM_IDSTART_LUASCRIPT) && (t <= SIM_IDEND_LUASCRIPT))
            {
                if (t == App::worldContainer->sandboxScript->getScriptHandle())
                    altT = sim_handle_sandbox;
                else if ((mainScriptHandle != -1) && (t == mainScriptHandle))
                    altT = sim_handle_mainscript;
            }
            else if (t >= SIM_UIDSTART)
                altT = sim_handle_mesh;
            auto s_event = _eventFilters.find(t);
            if ((s_event == _eventFilters.end()) && (t != altT))
                s_event = _eventFilters.find(altT);
            if (s_event != _eventFilters.end())
            { // we keep that event... maybe (if not empty)
                if (inf[ev].fieldPositions.size() == 0)
                    output.insert(output.end(), input.begin() + inf[ev].pos, input.begin() + inf[ev].pos + inf[ev].size); // empty data field
                else
                {
                    bool headerThere = false;
                    for (size_t i = 0; i < inf[ev].fieldNames.size(); i++)
                    {
                        if ((s_event->second.find(inf[ev].fieldNames[i]) != s_event->second.end()) || (s_event->second.size() == 0))
                        {
                            if (!headerThere)
                            {
                                headerThere = true;
                                output.insert(output.end(), input.begin() + inf[ev].pos, input.begin() + inf[ev].fieldPositions[0]); // up to the first data field
                            }
                            output.insert(output.end(), input.begin() + inf[ev].fieldPositions[i], input.begin() + inf[ev].fieldPositions[i] + inf[ev].fieldSizes[i]);
                        }
                    }
                    if (headerThere)
                        output.insert(output.end(), input.begin() + inf[ev].fieldPositions[inf[ev].fieldPositions.size() - 1] + inf[ev].fieldSizes[inf[ev].fieldSizes.size() - 1], input.begin() + inf[ev].pos + inf[ev].size); // to the end of that event
                }
            }
        }
        output.push_back(input[input.size() - 1]); // "array close" (holding all events)
        retVal = true;
    }
    return retVal;
}

int CScriptObject::extractCommandFromOutsideCommandQueue(int auxVals[4], double aux2Vals[8], int& aux2Count)
{
    if (_outsideCommandQueue != nullptr)
        return (_outsideCommandQueue->extractOneCommand(auxVals, aux2Vals, aux2Count));
    return (-1);
}

void CScriptObject::terminateScriptExecutionExternally(bool generateErrorMsg)
{
    if (generateErrorMsg)
    {
        std::string tmp("?: script execution was terminated externally.");
        _announceErrorWasRaisedAndPossiblyPauseSimulation(tmp.c_str(), true);
    }

    luaWrap_lua_yield((luaWrap_lua_State*)_interpreterState, 0);
}

void CScriptObject::_announceErrorWasRaisedAndPossiblyPauseSimulation(const char* errMsg, bool runtimeError)
{ // errMsg is in the form: xxxx:lineNb: msg
    std::string errM(errMsg);
    if ((errM.find("attempt to yield across metamethod/C-call boundary") == std::string::npos) &&
        (errM.find("attempt to yield from outside a coroutine") == std::string::npos))
    { // silent error when breaking out of a threaded simulation script at simulation end
        if ((_scriptType == sim_scripttype_main) || (_scriptType == sim_scripttype_simulation) || (_scriptType == sim_scripttype_customization))
            App::currentWorld->simulation->pauseOnErrorRequested();
        App::logScriptMsg(this, sim_verbosity_scripterrors, errM.c_str());
        _lastStackTraceback = errM;
    }
#ifdef SIM_WITH_GUI
    GuiApp::setRefreshHierarchyViewFlag();
#endif
}

int CScriptObject::getScriptHandleFromInterpreterState_lua(void* LL)
{
    luaWrap_lua_State* L = (luaWrap_lua_State*)LL;
    int retVal = -1;
    luaWrap_lua_getglobal(L, SIM_SCRIPT_HANDLE);
    if (luaWrap_lua_isnumber(L, -1))
        retVal = luaWrap_lua_tointeger(L, -1);
    luaWrap_lua_pop(L, 1);
    return (retVal);
}

void CScriptObject::_setScriptHandleToInterpreterState_lua(void* LL, int h)
{
    luaWrap_lua_State* L = (luaWrap_lua_State*)LL;
    std::string tmp(SIM_SCRIPT_HANDLE);
    tmp += "=";
    tmp += std::to_string(h);
    luaWrap_luaL_dostring(L, tmp.c_str());
}

void CScriptObject::setScriptNameIndexToInterpreterState_lua_old(void* LL, int index)
{
    luaWrap_lua_State* L = (luaWrap_lua_State*)LL;
    std::string tmp(SIM_SCRIPT_NAME_INDEX_OLD);
    tmp += "=";
    tmp += std::to_string(index);
    luaWrap_luaL_dostring(L, tmp.c_str());
}

int CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(void* LL)
{
    luaWrap_lua_State* L = (luaWrap_lua_State*)LL;
    int retVal = -1;
    luaWrap_lua_getglobal(L, SIM_SCRIPT_NAME_INDEX_OLD);
    if (luaWrap_lua_isnumber(L, -1))
        retVal = luaWrap_lua_tointeger(L, -1);
    luaWrap_lua_pop(L, 1);
    return (retVal);
}

std::string CScriptObject::getSearchPath_lua()
{
    std::string retVal;

    // in first position, so we can override things:
    retVal += App::folders->getUserSettingsPath() + "/lua/?.lua;";
    retVal += App::folders->getUserSettingsPath() + "/lua/?/init.lua;";

    // main:
    retVal += App::folders->getInterpretersRootPath() + "/?.lua;";
    retVal += App::folders->getInterpretersRootPath() + "/?/init.lua;";

    // present by default, but also needed for the code editor:
    retVal += App::folders->getInterpretersRootPath() + "/lua/?.lua;";
    retVal += App::folders->getInterpretersRootPath() + "/lua/?/init.lua;";

    // backw. compatibility:
    retVal += App::folders->getInterpretersRootPath() + "/lua/models/deprecated/?.lua;";

    // backw. compatibility:
    retVal += App::folders->getInterpretersRootPath() + "/lua/deprecated/?.lua;";

    // backw. compatibility:
    retVal += App::folders->getInterpretersRootPath() + "/bwf/?.lua;";

    retVal += App::folders->getInterpretersRootPath() + "/luarocks/share/lua/5.4/?.lua";

    if (App::currentWorld->environment->getScenePathAndName().compare("") != 0)
    {
        retVal += ";";
        retVal += App::currentWorld->environment->getScenePath() + "/?.lua";
        retVal += ";";
        retVal += App::currentWorld->environment->getScenePath() + "/?/init.lua";
    }
    if (App::userSettings->additionalLuaPath.length() > 0)
    {
        retVal += ";";
        retVal += App::userSettings->additionalLuaPath + "/?.lua";
        retVal += ";";
        retVal += App::userSettings->additionalLuaPath + "/?/init.lua";
    }
    return (retVal);
}

std::string CScriptObject::getSearchCPath_lua()
{
    std::string retVal(App::folders->getInterpretersRootPath());
#ifdef WIN_SIM
    retVal += "/luarocks/lib/lua/5.4/?.dll";
#else
    retVal += "/luarocks/lib/lua/5.4/?.so";
#endif
    return (retVal);
}

std::string CScriptObject::getSearchPath_python()
{
    std::string retVal;
    retVal += App::folders->getInterpretersRootPath();
    retVal += "/?.py;";
    retVal += App::folders->getInterpretersRootPath();
    retVal += "/python/?.py;";
    if (App::currentWorld->environment->getScenePathAndName().compare("") != 0)
    {
        retVal += ";";
        retVal += App::currentWorld->environment->getScenePath();
        retVal += "/?.py";
    }
    if (App::userSettings->additionalPythonPath.length() > 0)
    {
        retVal += ";";
        retVal += App::userSettings->additionalPythonPath;
        retVal += "/?.py";
    }
    return (retVal);
}

std::string CScriptObject::_removeLangTagInCode()
{
    std::string retVal;
    std::string l;
    std::string tmpCode(_scriptText);
    while (utils::extractLine(tmpCode, l))
    {
        utils::removeSpacesAtBeginningAndEnd(l);
        if ((l.size() > 0) && (l[0] == '#'))
        {
            l.erase(l.begin());
            utils::removeSpacesAtBeginningAndEnd(l);
            std::string w;
            if ((utils::extractSpaceSeparatedWord(l, w) && (w == "python")))
            {
                retVal = w;
                while (true)
                {
                    _scriptText = tmpCode;
                    if (!utils::extractLine(tmpCode, l))
                        break;
                    utils::removeSpacesAtBeginningAndEnd(l);
                    if (l.size() != 0)
                        break;
                }
                break;
            }
        }
        else if ((l.size() > 1) && (l[0] == '-') && (l[1] == '-'))
        {
            l.erase(l.begin());
            l.erase(l.begin());
            utils::removeSpacesAtBeginningAndEnd(l);
            std::string w;
            if ((utils::extractSpaceSeparatedWord(l, w) && (w == "lua")))
            {
                retVal = w;
                while (true)
                {
                    _scriptText = tmpCode;
                    if (!utils::extractLine(tmpCode, l))
                        break;
                    utils::removeSpacesAtBeginningAndEnd(l);
                    if (l.size() != 0)
                        break;
                }
                break;
            }
        }
        else if (l.size() > 0)
            break;
    }
    return retVal;
}

std::string CScriptObject::getLang() const
{
    return _lang;
}

void CScriptObject::setLang(const char* lang)
{
    if (lang != nullptr)
    {
        _lang = lang;
        if ((_lang != "lua") && (_lang != "python"))
            _scriptIsDisabled = true;
    }
    else
    {
        if (_scriptType != sim_scripttype_passive)
            _lang = "lua";
    }
}

void CScriptObject::setExecutionDepth(int d)
{
    bool diff = (_executionDepth != d);
    if (diff)
        _executionDepth = d;
}

int CScriptObject::getExecutionDepth() const
{
    return _executionDepth;
}

bool CScriptObject::_initInterpreterState(std::string* errorMsg)
{
    _previouslyUsedModules.clear();
    _calledInThisSimulationStep = false;
    _randGen.seed(123456);
    _delayForAutoYielding = 2;
    _forbidAutoYieldingLevel = 0;
    _timeForNextAutoYielding = int(VDateTime::getTimeInMs()) + _delayForAutoYielding;
    _forbidOverallYieldingLevel = 0;

    luaWrap_lua_State* L = luaWrap_luaL_newstate();
    _interpreterState = L;
    luaWrap_luaL_openlibs(L);
    _execSimpleString_safe_lua(L, "os.setlocale'C'");

    _setScriptHandleToInterpreterState_lua(L, _scriptHandle);
    _execSimpleString_safe_lua(L, (std::string(SIM_PLUGIN_NAMESPACES) + "={}").c_str());
    setScriptNameIndexToInterpreterState_lua_old(L, _getScriptNameIndexNumber_old());

    // --------------------------------------------
    // prepend some paths to the Lua path variable:
    luaWrap_lua_getglobal(L, "package");
    luaWrap_lua_getfield(L, -1, "path");
    std::string cur_path = getSearchPath_lua() + ";" + luaWrap_lua_tostring(L, -1);
    boost::replace_all(cur_path, "\\", "/");
    luaWrap_lua_pop(L, 1);
    luaWrap_lua_pushtext(L, cur_path.c_str());
    luaWrap_lua_setfield(L, -2, "path");
    luaWrap_lua_pop(L, 1);
    // --------------------------------------------

    // --------------------------------------------
    // prepend some paths to the Lua cpath variable:
    luaWrap_lua_getglobal(L, "package");
    luaWrap_lua_getfield(L, -1, "cpath");
    cur_path = getSearchCPath_lua() + ";" + luaWrap_lua_tostring(L, -1);
    boost::replace_all(cur_path, "\\", "/");
    luaWrap_lua_pop(L, 1);
    luaWrap_lua_pushtext(L, cur_path.c_str());
    luaWrap_lua_setfield(L, -2, "cpath");
    luaWrap_lua_pop(L, 1);
    // --------------------------------------------

    _execSimpleString_safe_lua(L, "sim={} sim1={}");
    registerNewFunctions_lua();
    _registerNewVariables_lua();
    _execSimpleString_safe_lua(L, "_S={}; _S.internalApi={sim=sim, sim1=sim1}; sim=nil sim1=nil");

    std::string dummyStr;
    if (App::getAppNamedParam("devmode", dummyStr))
        _execSimpleString_safe_lua(L, "_DEVMODE=true");
    if (0 != _execSimpleString_safe_lua(L, "require('base')"))
    {
        if (errorMsg != nullptr)
            errorMsg[0] = luaWrap_lua_tostring(L, -1);
        _killInterpreterState();
        _initFunctionHookCount = 0;
    }
    else
    {
        // Following 3 for the old plugins:
        registerPluginVariables(true); // for now we do not react to a failed require("file"), for backward compatibility's sake. We report
                   // a warning, and only to the console and for the sandbox script
        registerPluginFunctions();
        registerPluginVariables(false);

        luaWrap_lua_sethook(L, _hookFunction_lua, luaWrapGet_LUA_MASKCOUNT(), 100); // This instruction gets also called in luaHookFunction!!!!
        _initFunctionHookCount = int(_functionHooks_before.size() + _functionHooks_after.size());
    }

    return (_interpreterState != nullptr);
}

void CScriptObject::_hookFunction_lua(void* LL, void* arr)
{
    TRACE_INTERNAL;
    luaWrap_lua_State* L = (luaWrap_lua_State*)LL;
    CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(getScriptHandleFromInterpreterState_lua(L));
    if (it == nullptr)
        return;

    luaWrap_lua_Debug* ar = (luaWrap_lua_Debug*)arr;

    if (ar->event != luaWrapGet_LUA_HOOKCALL())
    {
        // Following 3 instructions are important: it can happen that the user locks/unlocks automatic thread switch in
        // a loop, and that the hook function by malchance only gets called when the thread switches are not allowed
        // (due to the loop timing and hook call timing overlap) --> this thread doesn't switch and stays in a lua loop
        // forever. To avoid this we add some random component to the hook timing:
        int randComponent = rand() / (RAND_MAX / 10);
        int hookMask = luaWrapGet_LUA_MASKCOUNT();
        luaWrap_lua_sethook(L, _hookFunction_lua, hookMask, 95 + randComponent);
        // Also remember: the hook gets also called when calling luaWrap_luaL_doString from c++ and similar!!

#ifdef SIM_WITH_GUI
        if (App::userSettings->getAbortScriptExecutionTiming() != 0)
        {
            if (it->getScriptExecutionTimeInMs() > (App::userSettings->getAbortScriptExecutionTiming() * 1000))
            {
                if (App::currentWorld->simulation->showAndHandleEmergencyStopButton(
                        true, it->getShortDescriptiveName().c_str()))
                    it->terminateScriptExecutionExternally(true);
            }
            else
                App::currentWorld->simulation->showAndHandleEmergencyStopButton(false, "");
        }
#endif
        //        luaWrap_luaL_dostring(L,"return coroutine.isyieldable()");
        luaWrap_luaL_dostring(L, "return coroutine.running()"); // sec. ret. val. is false --> can yield
        if (luaWrap_lua_toboolean(L, -1) == 0)
        {
            if (it->shouldAutoYield())
            {
                luaWrap_lua_pop(L, 1);
                return luaWrap_lua_yield(L, 0); // does a long jump and never returns
            }
        }
        luaWrap_lua_pop(L, 1);
    }
}

void CScriptObject::buildFromInterpreterStack_lua(void* LL, CInterfaceStack* stack, int fromPos, int cnt)
{ // fromPos: 1-n, cnt==0 --> all
    // !! LL is not the same for a script when in normal or inside a coroutine !!
    luaWrap_lua_State* L = (luaWrap_lua_State*)LL;
    stack->clear();
    int numberOfArguments = luaWrap_lua_gettop(L);
    if (fromPos > 1)
        numberOfArguments -= fromPos - 1;
    if (cnt > 0)
        numberOfArguments = std::min<int>(numberOfArguments, cnt);
    for (int i = fromPos; i < fromPos + numberOfArguments; i++)
    {
        std::map<void*, bool> visitedTables;
        CInterfaceStackObject* obj = _generateObjectFromInterpreterStack_lua(L, i, visitedTables);
        stack->pushObjectOntoStack(obj);
    }
}

void CScriptObject::buildOntoInterpreterStack_lua(void* LL, const CInterfaceStack* stack, bool takeOnlyTop)
{ // !! LL is not the same for a script when in normal or inside a coroutine !!
    luaWrap_lua_State* L = (luaWrap_lua_State*)LL;
    if (takeOnlyTop)
    {
        if (stack->getStackSize() > 0)
            _pushOntoInterpreterStack_lua(L, stack->getStackObjectFromIndex(stack->getStackSize() - 1));
    }
    else
    {
        for (size_t i = 0; i < int(stack->getStackSize()); i++)
        {
            CInterfaceStackObject* obj = stack->getStackObjectFromIndex(i);
            _pushOntoInterpreterStack_lua(L, obj);
        }
    }
}

void CScriptObject::registerNewFunctions_lua()
{
    luaWrap_lua_State* L = (luaWrap_lua_State*)_interpreterState;
    // API functions (base (e.g. "loadPlugin"), regular (e.g. "sim.getObject") and deprecated (e.g. "sim1.handleIkGroup")):
    for (int i = 0; simLuaCommands[i].name != ""; i++)
    {
        std::string name(simLuaCommands[i].name);
        size_t p = name.find('.');
        if (p != std::string::npos)
        { // sim, sim1, sim2 namespaces
            std::string prefix(name.begin(), name.begin() + p);
            name.erase(name.begin(), name.begin() + p + 1);
            _registerTableFunction(L, prefix.c_str(), name.c_str(), simLuaCommands[i].func);
        }
        else
            luaWrap_lua_register(L, simLuaCommands[i].name.c_str(), simLuaCommands[i].func); // global namespace
    }
}

bool CScriptObject::hasFunctionHook(const char* sysFunc) const
{
    for (size_t i = 0; i < _functionHooks_before.size() / 2; i++)
    {
        if (_functionHooks_before[2 * i + 0].compare(sysFunc) == 0)
            return (true);
    }
    for (size_t i = 0; i < _functionHooks_after.size() / 2; i++)
    {
        if (_functionHooks_after[2 * i + 0].compare(sysFunc) == 0)
            return (true);
    }
    return (false);
}

int CScriptObject::getFuncAndHookCnt(int sysCall, size_t what) const
{ // Only for time critical functions/hooks (event, dyn, contact, joint). what: 0=func, 1=hook before, 2=hook after
    if (sysCall == sim_syscb_event)
        return (_sysFuncAndHookCnt_event[what]);
    if (sysCall == sim_syscb_dyn)
        return (_sysFuncAndHookCnt_dyn[what]);
    if (sysCall == sim_syscb_contact)
        return (_sysFuncAndHookCnt_contact[what]);
    if (sysCall == sim_syscb_joint)
        return (_sysFuncAndHookCnt_joint[what]);
    return (0);
}

void CScriptObject::setFuncAndHookCnt(int sysCall, size_t what, int cnt)
{ // Only for time critical functions/hooks (event, dyn, contact, joint). what: 0=func, 1=hook before, 2=hook after
    if ((sysCall == sim_syscb_event) || (sysCall == -1))
    {
        int dx = cnt - _sysFuncAndHookCnt_event[what];
        _sysFuncAndHookCnt_event[what] = cnt;
        if (_scriptType == sim_scripttype_addon)
            App::worldContainer->addOnScriptContainer->setSysFuncAndHookCnt(
                sim_syscb_event, App::worldContainer->addOnScriptContainer->getSysFuncAndHookCnt(sim_syscb_event) + dx);
        else if (_scriptType != sim_scripttype_sandbox)
        {
            if (_scriptHandle < SIM_IDSTART_LUASCRIPT)
                App::currentWorld->sceneObjects->setSysFuncAndHookCnt(sim_syscb_event, App::currentWorld->sceneObjects->getSysFuncAndHookCnt(sim_syscb_event) + dx);
            else
                App::currentWorld->sceneObjects->embeddedScriptContainer->setSysFuncAndHookCnt(sim_syscb_event, App::currentWorld->sceneObjects->embeddedScriptContainer->getSysFuncAndHookCnt(sim_syscb_event) + dx);
        }
    }
    if ((sysCall == sim_syscb_dyn) || (sysCall == -1))
    {
        int dx = cnt - _sysFuncAndHookCnt_dyn[what];
        _sysFuncAndHookCnt_dyn[what] = cnt;
        if (_scriptType == sim_scripttype_addon)
            App::worldContainer->addOnScriptContainer->setSysFuncAndHookCnt(
                sim_syscb_dyn, App::worldContainer->addOnScriptContainer->getSysFuncAndHookCnt(sim_syscb_dyn) + dx);
        else if (_scriptType != sim_scripttype_sandbox)
        {
            if (_scriptHandle < SIM_IDSTART_LUASCRIPT)
                App::currentWorld->sceneObjects->setSysFuncAndHookCnt(sim_syscb_dyn, App::currentWorld->sceneObjects->getSysFuncAndHookCnt(sim_syscb_dyn) + dx);
            else
                App::currentWorld->sceneObjects->embeddedScriptContainer->setSysFuncAndHookCnt(sim_syscb_dyn, App::currentWorld->sceneObjects->embeddedScriptContainer->getSysFuncAndHookCnt(sim_syscb_dyn) + dx);
        }
    }
    if ((sysCall == sim_syscb_contact) || (sysCall == -1))
    {
        int dx = cnt - _sysFuncAndHookCnt_contact[what];
        _sysFuncAndHookCnt_contact[what] = cnt;
        if (_scriptType == sim_scripttype_addon)
            App::worldContainer->addOnScriptContainer->setSysFuncAndHookCnt(
                sim_syscb_contact,
                App::worldContainer->addOnScriptContainer->getSysFuncAndHookCnt(sim_syscb_contact) + dx);
        else if (_scriptType != sim_scripttype_sandbox)
        {
            if (_scriptHandle < SIM_IDSTART_LUASCRIPT)
                App::currentWorld->sceneObjects->setSysFuncAndHookCnt(sim_syscb_contact, App::currentWorld->sceneObjects->getSysFuncAndHookCnt(sim_syscb_contact) + dx);
            else
                App::currentWorld->sceneObjects->embeddedScriptContainer->setSysFuncAndHookCnt(sim_syscb_contact, App::currentWorld->sceneObjects->embeddedScriptContainer->getSysFuncAndHookCnt(sim_syscb_contact) + dx);
        }
    }
    if ((sysCall == sim_syscb_joint) || (sysCall == -1))
    {
        int dx = cnt - _sysFuncAndHookCnt_joint[what];
        _sysFuncAndHookCnt_joint[what] = cnt;
        if (_scriptType == sim_scripttype_addon)
            App::worldContainer->addOnScriptContainer->setSysFuncAndHookCnt(
                sim_syscb_joint, App::worldContainer->addOnScriptContainer->getSysFuncAndHookCnt(sim_syscb_joint) + dx);
        else if (_scriptType != sim_scripttype_sandbox)
        {
            if (_scriptHandle < SIM_IDSTART_LUASCRIPT)
                App::currentWorld->sceneObjects->setSysFuncAndHookCnt(sim_syscb_joint, App::currentWorld->sceneObjects->getSysFuncAndHookCnt(sim_syscb_joint) + dx);
            else
                App::currentWorld->sceneObjects->embeddedScriptContainer->setSysFuncAndHookCnt(sim_syscb_joint, App::currentWorld->sceneObjects->embeddedScriptContainer->getSysFuncAndHookCnt(sim_syscb_joint) + dx);
        }
    }
}

int CScriptObject::registerFunctionHook(const char* sysFunc, const char* userFunc, bool before)
{
    int sysFuncNb = getSystemCallbackFromString(sysFunc);
    if (strlen(sysFunc) == 0)
    {
        if (before)
        {
            _functionHooks_before.clear();
            setFuncAndHookCnt(-1, 1, 0);
        }
        else
        {
            _functionHooks_after.clear();
            setFuncAndHookCnt(-1, 2, 0);
        }
        return (0); // successful unregister
    }
    else
    {
        std::vector<std::string>* l = &_functionHooks_after;
        if (before)
            l = &_functionHooks_before;
        bool userFuncEmpty = strlen(userFunc) == 0;
        size_t i = 0;
        while (i < l->size() / 2)
        {
            if (l->at(2 * i + 0).compare(sysFunc) == 0)
            {
                if (userFuncEmpty)
                    l->erase(l->begin() + 2 * i, l->begin() + 2 * i + 2); // remove all hooks for that system function
                else
                {
                    if (l->at(2 * i + 1).compare(userFunc) == 0)
                    { // function already registered. Unregister it:
                        l->erase(l->begin() + 2 * i, l->begin() + 2 * i + 2);
                        if (sysFuncNb >= 0)
                        {
                            if (before)
                                setFuncAndHookCnt(sysFuncNb, 1, getFuncAndHookCnt(sysFuncNb, 1) - 1);
                            else
                                setFuncAndHookCnt(sysFuncNb, 2, getFuncAndHookCnt(sysFuncNb, 2) - 1);
                        }
                        return (0); // successful unregister
                    }
                    i++;
                }
            }
            else
                i++;
        }
        if (userFuncEmpty)
        {
            if (sysFuncNb >= 0)
            {
                if (before)
                    setFuncAndHookCnt(sysFuncNb, 1, 0);
                else
                    setFuncAndHookCnt(sysFuncNb, 2, 0);
            }
            return (0); // successful unregister all hooks for that system function
        }
        // We need to register that function:
        if (before)
        {
            l->insert(l->begin(), userFunc);
            l->insert(l->begin(), sysFunc);
        }
        else
        {
            l->push_back(sysFunc);
            l->push_back(userFunc);
        }
        if (sysFuncNb >= 0)
        {
            if (before)
                setFuncAndHookCnt(sysFuncNb, 1, getFuncAndHookCnt(sysFuncNb, 1) + 1);
            else
                setFuncAndHookCnt(sysFuncNb, 2, getFuncAndHookCnt(sysFuncNb, 2) + 1);
        }
        return (1); // successful register
    }
}

bool CScriptObject::replaceScriptText(const char* oldTxt, const char* newTxt)
{
    std::string theScript(getScriptText());
    size_t startPos = theScript.find(oldTxt, 0);
    bool replacedSomething = false;
    while (startPos != std::string::npos)
    {
        theScript.replace(startPos, strlen(oldTxt), newTxt);
        startPos = theScript.find(oldTxt, startPos + 1);
        replacedSomething = true;
    }
    if (replacedSomething)
        setScriptText(theScript.c_str());
    return replacedSomething;
}

void CScriptObject::printInterpreterStack() const
{
    luaWrap_lua_State* L = (luaWrap_lua_State*)_interpreterState;
    int cnt = luaWrap_lua_gettop(L);
    printf("***Lua stack***\n");
    for (int i = 1; i <= cnt; i++)
    {
        printf("%i: type: %s: ", i, lua_typename((lua_State*)L, lua_type((lua_State*)L, i)));
        if (luaWrap_lua_isnil(L, i))
            printf("nil\n");
        if (luaWrap_lua_isboolean(L, i))
            printf("bool\n");
        if (luaWrap_lua_isinteger(L, i))
            printf("int: %lli\n", luaWrap_lua_tointeger(L, i));
        if (luaWrap_lua_isnumber(L, i))
            printf("number: %f\n", luaWrap_lua_tonumber(L, i));
        if (luaWrap_lua_isstring(L, i))
            printf("string: %s\n", luaWrap_lua_tostring(L, i));
        if (luaWrap_lua_isnonbuffertable(L, i))
            printf("table\n");
        if (luaWrap_lua_isfunction(L, i))
            printf("function\n");
    }
    printf("***********\n");
}

void CScriptObject::loadPluginFuncsAndVars(CPlugin* plug)
{
    static std::set<std::string> failedLuaExecs;
    luaWrap_lua_State* L = (luaWrap_lua_State*)_interpreterState;

    _execSimpleString_safe_lua(L, "__tmploadPluginFuncsAndVars1 = {}");

    // Now load the callbacks into Lua state:
    CPluginCallbackContainer* cbCont = plug->getPluginCallbackContainer();
    size_t index = 0;
    while (true)
    {
        SPluginCallback* dat = cbCont->getCallbackFromIndex(index++);
        if (dat != nullptr)
        {
            std::string idName(plug->getName() + "@" + dat->funcName);
            luaWrap_lua_rawgeti(L, luaWrapGet_LUA_REGISTRYINDEX(), luaWrapGet_LUA_RIDX_GLOBALS()); // table of globals
            luaWrap_lua_pushtext(L, idName.c_str());
            luaWrap_lua_pushcclosure(L, _simGenericFunctionHandler, 1);
            luaWrap_lua_setfield(L, -2, "__tmploadPluginFuncsAndVars2");
            luaWrap_lua_pop(L, 1); // pop table of globals
            std::string tmp("__tmploadPluginFuncsAndVars1." + dat->funcName +
                            "=__tmploadPluginFuncsAndVars2 __tmploadPluginFuncsAndVars2 = nil");
            _execSimpleString_safe_lua(L, tmp.c_str());
        }
        else
            break;
    }

    // Now load the variables into Lua state:
    CPluginVariableContainer* varCont = plug->getPluginVariableContainer();
    index = 0;
    while (true)
    {
        SPluginVariable* dat = varCont->getVariableFromIndex(index++);
        if (dat != nullptr)
        {
            // Following just a simple way to make sure cascaded namespaces exist
            std::string varName(dat->varName);
            std::string totVar("__tmploadPluginFuncsAndVars1.");
            while (varName.find(".") != std::string::npos)
            {
                std::string wordd(utils::extractWord(varName, "."));
                totVar += wordd;
                wordd = "if ( " + totVar + " == nil) then boom = boom + 1 end";
                if (0 != _execSimpleString_safe_lua(L, wordd.c_str()))
                {
                    wordd = totVar + " = {}";
                    _execSimpleString_safe_lua(L, wordd.c_str());
                }
                totVar += ".";
            }

            std::string variableName("__tmploadPluginFuncsAndVars1." + dat->varName);
            if (dat->stackHandle <= 0)
            { // simple variable
                std::string tmp(variableName + " = " + dat->varValue);
                if ((0 != _execSimpleString_safe_lua(L, tmp.c_str())) && (failedLuaExecs.find(tmp) == failedLuaExecs.end()))
                { // warning only once
                    failedLuaExecs.insert(tmp);
                    tmp = "failed executing '" + tmp + "' (plugin '" + plug->getName() + "')";
                    App::logScriptMsg(this, sim_verbosity_scriptwarnings, tmp.c_str());
                }
            }
            else
            { // stack variable
                CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(dat->stackHandle);
                buildOntoInterpreterStack_lua(L, stack, true);
                luaWrap_lua_setglobal(L, variableName.c_str());
            }
        }
        else
            break;
    }

    // Place the plugin handle in there, e.g. simBB.pluginHandle=pluginHandle:
    std::string tmp("__tmploadPluginFuncsAndVars1.pluginHandle=");
    tmp += std::to_string(plug->getHandle());
    _execSimpleString_safe_lua(L, tmp.c_str());

    // Set API to SIM_PLUGIN_NAMESPACES table, e.g. sim_plugin_namespaces[pluginName]=funcsAndVars:
    tmp = SIM_PLUGIN_NAMESPACES;
    tmp += "[\"";
    tmp += plug->getName() + "\"]=__tmploadPluginFuncsAndVars1 __tmploadPluginFuncsAndVars1=nil";
    _execSimpleString_safe_lua(L, tmp.c_str());
}

void CScriptObject::registerPluginFunctions()
{
    for (size_t i = 0; i < App::worldContainer->scriptCustomFuncAndVarContainer->getCustomFunctionCount(); i++)
    {
        CScriptCustomFunction* customFunc =
            App::worldContainer->scriptCustomFuncAndVarContainer->getCustomFunctionFromIndex(i);
        if (customFunc->hasCallback())
        {
            std::string functionName(customFunc->getFunctionName());
            int functionID = customFunc->getFunctionID();

            size_t p = functionName.find(".");
            luaWrap_lua_State* L = (luaWrap_lua_State*)_interpreterState;
            if (p != std::string::npos)
            { // this is the new notation, e.g. simUI.create()
                std::string prefix(functionName.begin(), functionName.begin() + p);
                luaWrap_lua_rawgeti(L, luaWrapGet_LUA_REGISTRYINDEX(),
                                    luaWrapGet_LUA_RIDX_GLOBALS()); // table of globals
                luaWrap_lua_pushinteger(L, functionID + 1);
                //                luaWrap_lua_pushtext(L,functionName.c_str());
                luaWrap_lua_pushcclosure(L, _simGenericFunctionHandler, 1);
                luaWrap_lua_setfield(L, -2, "__iuafkjsdgoi158zLK");
                luaWrap_lua_pop(L, 1); // pop table of globals
                std::string tmp("if not ");
                tmp += prefix;
                tmp += " then ";
                tmp += prefix;
                tmp += "={} end ";
                tmp += functionName;
                tmp += "=__iuafkjsdgoi158zLK __iuafkjsdgoi158zLK=nil";
                _execSimpleString_safe_lua(L, tmp.c_str());
            }
            else
            { // Old
                luaWrap_lua_rawgeti(L, luaWrapGet_LUA_REGISTRYINDEX(),
                                    luaWrapGet_LUA_RIDX_GLOBALS()); // table of globals
                luaWrap_lua_pushinteger(L, functionID + 1);
                luaWrap_lua_pushcclosure(L, _simGenericFunctionHandler, 1);
                luaWrap_lua_setfield(L, -2, functionName.c_str());
                luaWrap_lua_pop(L, 1); // pop table of globals
            }
        }
    }
}

void CScriptObject::_registerNewVariables_lua()
{
    luaWrap_lua_State* L = (luaWrap_lua_State*)_interpreterState;
    // Constants (regular (e.g. "sim.sceneobject_shape") and deprecated (e.g. "sim1.particle_points1")):
    for (size_t i = 0; simLuaVariables[i].name != ""; i++)
    {
        std::string tmp(simLuaVariables[i].name.c_str());
        tmp += "=" + std::to_string(simLuaVariables[i].val);
        _execSimpleString_safe_lua(L, tmp.c_str());
    }
}

bool CScriptObject::registerPluginVariables(bool onlyRequireStatements)
{
    for (size_t i = 0; i < App::worldContainer->scriptCustomFuncAndVarContainer->getCustomVariableCount(); i++)
    {
        CScriptCustomVariable* customVar =
            App::worldContainer->scriptCustomFuncAndVarContainer->getCustomVariableFromIndex(i);
        std::string variableName(customVar->getVariableName());
        std::string variableValue(customVar->getVariableValue());
        int variableStackId = customVar->getVariableStackId();

        luaWrap_lua_State* L = (luaWrap_lua_State*)_interpreterState;
        if (variableStackId == 0)
        { // simple variable
            bool doIt = false;
            if (onlyRequireStatements && (variableValue.find("require") == 0))
                doIt = true;
            if ((!onlyRequireStatements) && (variableValue.find("require") != 0))
                doIt = true;
            if (doIt)
            {
                std::string tmp(variableName);
                tmp += "=" + variableValue;
                if ((0 != _execSimpleString_safe_lua(L, tmp.c_str())) && onlyRequireStatements &&
                    (_scriptType == sim_scripttype_sandbox))
                { // warning only with sandbox scripts
                    if ((variableName.find("simCHAI3D") == std::string::npos) &&
                        (variableName.find("simJoy") == std::string::npos))
                    { // ignore 2 files (old plugins)
                        tmp = "failed executing '" + tmp;
                        tmp += "'";
                        App::logScriptMsg(this, sim_verbosity_scriptwarnings, tmp.c_str());
                    }
                }
            }
        }
        else
        { // stack variable
            if (variableStackId != 0)
            {
                if (!onlyRequireStatements)
                {
                    CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(variableStackId);
                    buildOntoInterpreterStack_lua(L, stack, true);
                    luaWrap_lua_setglobal(L, variableName.c_str());
                }
            }
        }
    }
    return (true);
}

void CScriptObject::serialize(CSer& ar)
{
    _removeLangTagInCode();
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("Si2");
            ar << int(0) << _sceneObjectHandle << _scriptType;
            ar.flush();

            // Keep following close to the beginning!
            ar.storeDataName("Va2");
            unsigned char nothing = 0;
            SIM_SET_CLEAR_BIT(nothing, 0, false); // _threadedExecution_oldThreads 13.06.2025
            SIM_SET_CLEAR_BIT(nothing, 1, _scriptIsDisabled);
            // RESERVED
            SIM_SET_CLEAR_BIT(nothing, 3, true); // used to be (!defaultMainScript). 16.11.2020
            SIM_SET_CLEAR_BIT(nothing, 4, false); // used to be _executeJustOnce_oldThreads. 13.06.2025
            // RESERVED!!
            SIM_SET_CLEAR_BIT(nothing, 6, true); // this indicates we have the 'almost' new script execution engine (since V3.1.3)
            SIM_SET_CLEAR_BIT(nothing, 7, true); // this indicates we have the new script execution engine (since V3.1.3)
            ar << nothing;
            ar.flush();

            // Keep following close to the beginning!
            ar.storeDataName("Va3"); // this is also used as a marked for the color correction introduced in V3.1.4 and later!
            nothing = 0;
            SIM_SET_CLEAR_BIT(nothing, 0, true); // needed for a code correction
            // reserved, was: SIM_SET_CLEAR_BIT(nothing,1,!_disableCustomizationScriptWithError);
            SIM_SET_CLEAR_BIT(nothing, 2, _parentIsProxy);
            ar << nothing;
            ar.flush();

            // Keep for backward compatibility (19.09.2022)
            ar.storeDataName("Seo");
            ar << _executionPriority_old;
            ar.flush();

            // ar.storeDataName("Ttd");  reserved

            // We store the script in a light encoded way:
            std::string stt(_scriptText);
            // For backward comp:
            // -----------------
            if (_lang == "lua")
                stt = "--lua\n" + stt;
            else if (_lang == "python")
                stt = "#python\n" + stt;
            // -----------------
            ar.storeDataName("Ste");
            tt::lightEncodeBuffer(&stt[0], int(stt.length()));
            for (size_t i = 0; i < stt.length(); i++)
                ar << stt[i];
            ar.flush();

            ar.storeDataName("Pum");
            ar << int(_previouslyUsedModules.size());
            for (auto it = _previouslyUsedModules.begin(); it != _previouslyUsedModules.end(); it++)
                ar << *it;
            ar.flush();

            // keep a while so that older versions can read this. 11.06.2019, V3.6.1 is current
            ar.storeDataName("Coc");
            ar << _sceneObjectHandle;
            ar.flush();

            if (_customObjectData_old != nullptr)
            {
                ar.storeDataName("Cod");
                ar.setCountingMode();
                _customObjectData_old->serializeData(ar, nullptr, -1);
                if (ar.setWritingMode())
                    _customObjectData_old->serializeData(ar, nullptr, -1);
            }

            ar.storeDataName("Lne");
            ar << _lang;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            _tempSuspended = INITIALLY_SUSPEND_LOADED_SCRIPTS;
            int byteQuantity;
            std::string theName = "";
            bool backwardCompatibility_7_8_2014 = false;
            bool backwardCompatibility_13_8_2014 = false;
            bool executeInSensingPhase_oldCompatibility_7_8_2014 = false;
            bool backwardCompatibilityCorrectionNeeded_13_10_2014 = true;
            bool backwardCompatibilityCorrectionNeeded_8_11_2014 = true;
            bool usingOldThreads_oldThread = false;
            bool execJustOnce_oldThread = false;
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    bool justLoadedCustomScriptBuffer = false;
                    if (theName.compare("Si2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        int dummy;
                        ar >> dummy >> _sceneObjectHandle >> _scriptType;
                    }

                    if (theName.compare("Seo") == 0)
                    { // For backward compatibility (19.09.2022)
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _executionPriority_old;
                    }

                    if (theName.compare("Ste") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;

                        _scriptText.resize(byteQuantity, ' ');
                        if (byteQuantity != 0)
                        {
                            for (int i = 0; i < byteQuantity; i++)
                                ar >> _scriptText[i];
                            tt::lightDecodeBuffer(&_scriptText[0], byteQuantity);
                            while ((_scriptText.size() > 0) && (_scriptText[_scriptText.size() - 1] == 0))
                                _scriptText.erase(_scriptText.end() - 1); // to fix a compatibility bug
                        }
                        justLoadedCustomScriptBuffer = true;

                        std::string tg(_removeLangTagInCode());
                        if (tg == "python")
                            _lang = tg;
                        else
                            _lang = "lua";
                    }

                    if (theName.compare("Pum") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt;
                        std::string val;
                        ar >> cnt;
                        for (int i = 0; i < cnt; i++)
                        {
                            ar >> val;
                            _previouslyUsedModules.insert(val);
                        }
                    }

                    if (justLoadedCustomScriptBuffer && (_scriptType == sim_scripttype_main) &&
                        _mainScriptIsDefaultMainScript_old) // old, keep for backward compatibility. 16.11.2020
                        _scriptText = DEFAULT_MAINSCRIPT_CODE;
                    if (theName == "Va2")
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        usingOldThreads_oldThread = SIM_IS_BIT_SET(nothing, 0);
                        _scriptIsDisabled = SIM_IS_BIT_SET(nothing, 1);
                        _mainScriptIsDefaultMainScript_old = !SIM_IS_BIT_SET(nothing, 3);
                        execJustOnce_oldThread = SIM_IS_BIT_SET(nothing, 4);
                        executeInSensingPhase_oldCompatibility_7_8_2014 = SIM_IS_BIT_SET(nothing, 5);
                        backwardCompatibility_7_8_2014 = !SIM_IS_BIT_SET(nothing, 6);
                        backwardCompatibility_13_8_2014 = !SIM_IS_BIT_SET(nothing, 7);
                    }
                    if (theName == "Va3")
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        backwardCompatibilityCorrectionNeeded_8_11_2014 = !SIM_IS_BIT_SET(nothing, 0);
                        backwardCompatibilityCorrectionNeeded_13_10_2014 = false;
                        _parentIsProxy = SIM_IS_BIT_SET(nothing, 2);
                    }
                    if (theName.compare("Prm") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        delete _scriptParameters_backCompatibility;
                        _scriptParameters_backCompatibility = new CUserParameters();
                        _scriptParameters_backCompatibility->serialize(ar);
                    }

                    if (theName.compare("Coc") == 0)
                    { // keep 3-4 years for backward compatibility (11.06.2019, V3.6.1 is current)
                        noHit = false;
                        ar >> byteQuantity;
                        int v;
                        ar >> v;
                        if (v >= 0)
                            _sceneObjectHandle = v;
                    }
                    if (theName.compare("Cod") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        _customObjectData_old = new CCustomData_old();
                        _customObjectData_old->serializeData(ar, nullptr, -1);
                    }

                    if (theName.compare("Lne") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _lang;
                    }

                    if (noHit)
                        ar.loadUnknownData();
                }
            }

            _adjustScriptText1_old(this, backwardCompatibility_7_8_2014, executeInSensingPhase_oldCompatibility_7_8_2014);
            _adjustScriptText2_old(this, (!backwardCompatibility_7_8_2014) && backwardCompatibility_13_8_2014);
            _adjustScriptText3_old(this, backwardCompatibilityCorrectionNeeded_13_10_2014);
            _adjustScriptText4_old(this, backwardCompatibilityCorrectionNeeded_8_11_2014);
            _adjustScriptText5_old(this, ar.getCoppeliaSimVersionThatWroteThisFile() < 30300);
            _adjustScriptText6_old(this, ar.getCoppeliaSimVersionThatWroteThisFile() < 30300);
            _adjustScriptText7_old(this, (ar.getCoppeliaSimVersionThatWroteThisFile() <= 30302) && (SIM_PROGRAM_VERSION_NB > 30302));
            _performNewApiAdjustments_old(this, true);
            _adjustScriptText10_old(this, ar.getCoppeliaSimVersionThatWroteThisFile() < 30401);
            _adjustScriptText11_old(this, ar.getCoppeliaSimVersionThatWroteThisFile() < 40001);
            _adjustScriptText12_old(this, ar.getCoppeliaSimVersionThatWroteThisFile() < 40100);
            _adjustScriptText13_old(this, ar.getCoppeliaSimVersionThatWroteThisFile() < 40200);
            _adjustScriptText14_old(this, ar.getCoppeliaSimVersionThatWroteThisFile() < 40201);
            _adjustScriptText15_old(this, ar.getCoppeliaSimVersionThatWroteThisFile() <= 40300);
            _adjustScriptText16_old(this, ar.getCoppeliaSimVersionThatWroteThisFile() <= 40500);
            _adjustScriptText17_old(this, ar.getCoppeliaSimVersionThatWroteThisFile() <= 40501);

            if (CSimFlavor::getBoolVal(18))
                _detectDeprecated_old(this);

            if (usingOldThreads_oldThread && (ar.getCoppeliaSimVersionThatWroteThisFile() < 40200))
            {
                if (!_convertThreadedScriptToCoroutine_old(this, execJustOnce_oldThread))
                    App::logMsg(sim_verbosity_errors, "Attempting to instanciate a threaded script: threaded scripts are not supported anymore as of CoppeliaSim V4.10.0 rev2 and later. Convert those scripts. This scene/model will not run properly.");
            }

            if (getLang() == "python")
            {
                if (ar.getSerializationVersionThatWroteThisFile() < 25)
                { // make sure to use the old Python wrapper, with old Python scripts, for backward compatibility:
                    std::smatch match;
                    std::regex regEx(" *# *python.*");
                    std::string newTxt(
                        "#python\n#luaExec wrapper='deprecated.pythonWrapper' -- using the old wrapper for backw. compat.");
                    newTxt += "\n# To switch to the new wrapper, simply remove above line, and add sim=require('sim')";
                    newTxt += "\n# as the first instruction in sysCall_init() or sysCall_thread()";
                    if (std::regex_search(_scriptText, match, regEx))
                        _scriptText = std::string(match.prefix()) + newTxt + std::string(match.suffix());
                }
            }

            fromBufferToFile();
        }
    }
    else
    {
        bool exhaustiveXml = ((ar.getFileType() != CSer::filetype_csim_xml_simplescene_file) &&
                              (ar.getFileType() != CSer::filetype_csim_xml_simplemodel_file));
        if (ar.isStoring())
        {
            if (exhaustiveXml)
            {
                ar.xmlAddNode_int("handle", _scriptHandle);
                ar.xmlAddNode_int("objectHandle", _sceneObjectHandle);

                ar.xmlAddNode_enum("type", _scriptType, sim_scripttype_main, "mainScript",
                                   sim_scripttype_simulation, "childScript", sim_scripttype_customization,
                                   "customizationScript");
            }

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("enabled", !_scriptIsDisabled);
            ar.xmlAddNode_bool("parentIsProxy", _parentIsProxy);
            ar.xmlPopNode();

            ar.xmlAddNode_comment(" 'executionOrder' tag: for backward compatibility only ", exhaustiveXml);
            ar.xmlAddNode_int("executionOrder", _executionPriority_old); // for backward compatibility 19.09.2022

            std::string tmp(_scriptText.c_str());
            // For backw. comp.:
            // -----------------
            if (_lang == "lua")
                tmp = "--lua\n" + tmp;
            else if (_lang == "python")
                tmp = "#python\n" + tmp;
            // -----------------
            boost::replace_all(tmp, "\r\n", "\n");
            ar.xmlAddNode_comment(" 'scriptText' tag: best to use it with a CDATA section for readability ",
                                  exhaustiveXml);
            ar.xmlAddNode_cdata("scriptText", tmp.c_str());

            ar.xmlAddNode_string("lang", _lang.c_str());

            if (exhaustiveXml)
            {
                if (_customObjectData_old != nullptr)
                {
                    ar.xmlPushNewNode("customData");
                    _customObjectData_old->serializeData(ar, nullptr, _scriptHandle);
                    ar.xmlPopNode();
                }
            }
        }
        else
        {
            _tempSuspended = INITIALLY_SUSPEND_LOADED_SCRIPTS;
            int previousScriptHandle = -1;
            if (exhaustiveXml)
            {
                ar.xmlGetNode_int("handle", previousScriptHandle);
                ar.xmlGetNode_int("objectHandle", _sceneObjectHandle);

                ar.xmlGetNode_enum("type", _scriptType, true, "mainScript", sim_scripttype_main, "childScript",
                                   sim_scripttype_simulation, "customizationScript",
                                   sim_scripttype_customization);
            }

            if (ar.xmlPushChildNode("switches", exhaustiveXml))
            {
                if (ar.xmlGetNode_bool("enabled", _scriptIsDisabled, exhaustiveXml))
                    _scriptIsDisabled = !_scriptIsDisabled;
                ar.xmlGetNode_bool("parentIsProxy", _parentIsProxy, exhaustiveXml);
                if (exhaustiveXml)
                    ar.xmlGetNode_bool("isDefaultMainScript", _mainScriptIsDefaultMainScript_old, false);
                ar.xmlPopNode();
            }

            ar.xmlGetNode_int("executionOrder", _executionPriority_old, exhaustiveXml); // for backward compatibility 19.09.2022

            if (ar.xmlGetNode_cdata("scriptText", _scriptText, exhaustiveXml) &&
                (_scriptType == sim_scripttype_main) &&
                _mainScriptIsDefaultMainScript_old) // for backward compatibility 16.11.2020
                _scriptText = DEFAULT_MAINSCRIPT_CODE;

            std::string tg(_removeLangTagInCode());
            if (tg == "python")
                _lang = tg;
            else
                _lang = "lua";

            ar.xmlGetNode_string("lang", _lang);

            if (exhaustiveXml)
            {
                if (ar.xmlPushChildNode("customData", false))
                {
                    _customObjectData_old = new CCustomData_old();
                    _customObjectData_old->serializeData(ar, nullptr, -1);
                    ar.xmlPopNode();
                }
            }
            fromBufferToFile();
        }
    }
}

int CScriptObject::_execSimpleString_safe_lua(void* LL, const char* string)
{
    int t1 = _forbidAutoYieldingLevel;
    int t2 = _forbidOverallYieldingLevel;
    int retVal = luaWrap_luaL_dostring((luaWrap_lua_State*)LL, string);
    _forbidAutoYieldingLevel = t1;
    _forbidOverallYieldingLevel = t2;
    return (retVal);
}

bool CScriptObject::_loadBuffer_lua(const char* buff, size_t sz, const char* name)
{
    // This is the slow version (loading and compiling the buffer over and over):
    // int loadBufferRes=luaWrap_luaL_loadbuffer(luaState,buff,sz,name);
    // return(loadBufferRes==0);

    // This is the faster version (loading and compiling the buffer once only):
    luaWrap_lua_State* L = (luaWrap_lua_State*)_interpreterState;
    if (_loadBufferResult_lua != 0)
    {
        _loadBufferResult_lua = luaWrap_luaL_loadbuffer(L, buff, sz, name);
        if (_loadBufferResult_lua == 0)
            luaWrap_lua_setglobal(L, "sim_code_function_to_run");
    }
    if (_loadBufferResult_lua == 0)
        luaWrap_lua_getglobal(L, "sim_code_function_to_run");
    return (_loadBufferResult_lua == 0);
}

void CScriptObject::_printContext(const char* str, size_t p)
{
    size_t off = 60;
    if (p + off >= strlen(str))
        printf("** %s\n", str + p);
    else
    {
        std::string s;
        s.assign(str + p, str + p + off);
        printf("** %s\n", s.c_str());
    }
}

int CScriptObject::_countInterpreterStackTableEntries_lua(void* LL, int index)
{ // !! LL is not the same for a script when in normal or inside a coroutine !!
    luaWrap_lua_State* L = (luaWrap_lua_State*)LL;
    int cnt = 0;
    luaWrap_lua_pushvalue(L, index); // copy of the table to the top
    luaWrap_lua_pushnil(L);          // nil on top
    while (luaWrap_lua_next(L, -2))  // pops a value, then pushes a key-value pair (if table is not empty)
    {                                // stack now contains at -1 the value, at -2 the key, at -3 the table
        luaWrap_lua_pop(L, 1);       // pop 1 value (the value)
        // stack now contains at -1 the key, at -2 the table
        cnt++;
    }
    luaWrap_lua_pop(L, 1);
    // Stack is now restored to what it was at function entry
    return (cnt);
}

CInterfaceStackTable* CScriptObject::_generateTableMapFromInterpreterStack_lua(void* LL, int index,
                                                                               std::map<void*, bool>& visitedTables)
{ // there must be a table at the given index.
    // !! LL is not the same for a script when in normal or inside a coroutine !!
    luaWrap_lua_State* L = (luaWrap_lua_State*)LL;
    CInterfaceStackTable* table = new CInterfaceStackTable();
    luaWrap_lua_pushvalue(L, index); // copy of the table to the top
    luaWrap_lua_pushnil(L);          // nil on top
    while (luaWrap_lua_next(L, -2))  // pops a value, then pushes a key-value pair (if table is not empty)
    {                                // stack now contains at -1 the value, at -2 the key, at -3 the table
        // copy the key:
        luaWrap_lua_pushvalue(L, -2);
        // stack now contains at -1 the key, at -2 the value, at -3 the key, and at -4 the table
        int t = luaWrap_lua_stype(L, -1);
        if (t == sim_stackitem_double)
        { // the key is a number
            double key = luaWrap_lua_tonumber(L, -1);
            CInterfaceStackObject* obj = _generateObjectFromInterpreterStack_lua(L, -2, visitedTables);
            table->appendMapObject_object(key, obj);
        }
        else if (t == sim_stackitem_integer)
        { // the key is an integer
            long long int key = luaWrap_lua_tointeger(L, -1);
            CInterfaceStackObject* obj = _generateObjectFromInterpreterStack_lua(L, -2, visitedTables);
            table->appendMapObject_object(key, obj);
        }
        else if (t == sim_stackitem_bool)
        { // the key is a bool
            bool key = luaWrap_lua_toboolean(L, -1) != 0;
            CInterfaceStackObject* obj = _generateObjectFromInterpreterStack_lua(L, -2, visitedTables);
            table->appendMapObject_object(key, obj);
        }
        else if (t == sim_stackitem_string)
        { // the key is a string
            const char* txtStr = luaWrap_lua_tostring(L, -1);
            CInterfaceStackObject* obj = _generateObjectFromInterpreterStack_lua(L, -2, visitedTables);
            table->appendMapObject_object(txtStr, obj);
        }
        else
        { // the key is something weird, e.g. a table, a thread, etc. Convert this to a string:
            void* p = (void*)luaWrap_lua_topointer(L, -1);
            char num[21];
            snprintf(num, 20, "%p", p);
            std::string str;
            if (t == sim_stackitem_table)
                str = "<TABLE ";
            else if (t == sim_stackitem_userdat)
                str = "<USERDATA ";
            else if (t == sim_stackitem_func)
                str = "<FUNCTION ";
            else if (t == sim_stackitem_thread)
                str = "<THREAD ";
            else if (t == sim_stackitem_lightuserdat)
                str = "<LIGHTUSERDATA ";
            else
                str = "<UNKNOWNTYPE ";
            str += num;
            str += ">";
            CInterfaceStackObject* obj = _generateObjectFromInterpreterStack_lua(L, -2, visitedTables);
            table->appendMapObject_object(str.c_str(), obj);
        }
        luaWrap_lua_pop(L, 2); // pop 2 values (key+value)
        // stack now contains at -1 the key, at -2 the table
    }
    luaWrap_lua_pop(L, 1);
    // Stack is now restored to what it was at function entry
    return (table);
}

CInterfaceStackTable* CScriptObject::_generateTableArrayFromInterpreterStack_lua(void* LL, int index,
                                                                                 std::map<void*, bool>& visitedTables)
{ // there must be a table at the given index.
    // !! LL is not the same for a script when in normal or inside a coroutine !!
    luaWrap_lua_State* L = (luaWrap_lua_State*)LL;
    CInterfaceStackTable* table = new CInterfaceStackTable();
    int arraySize = int(luaWrap_lua_rawlen(L, index));
    for (int i = 0; i < arraySize; i++)
    {
        // Push the element i+1 of the table to the top of Lua's stack:
        luaWrap_lua_rawgeti(L, index, i + 1);
        CInterfaceStackObject* obj = _generateObjectFromInterpreterStack_lua(L, -1, visitedTables);
        luaWrap_lua_pop(L, 1); // we pop one element from the stack;
        table->appendArrayObject(obj);
    }
    return (table);
}

CInterfaceStackObject* CScriptObject::_generateObjectFromInterpreterStack_lua(void* LL, int index,
                                                                              std::map<void*, bool>& visitedTables)
{ // generates just one object at the given index
    // !! LL is not the same for a script when in normal or inside a coroutine !!
    luaWrap_lua_State* L = (luaWrap_lua_State*)LL;
    int t = luaWrap_lua_stype(L, index);
    if (t == sim_stackitem_null)
        return (new CInterfaceStackNull());
    else if (t == sim_stackitem_bool)
        return (new CInterfaceStackBool(luaWrap_lua_toboolean(L, index) != 0));
    else if (t == sim_stackitem_double)
        return (new CInterfaceStackNumber(luaWrap_lua_tonumber(L, index)));
    else if (t == sim_stackitem_integer)
        return (new CInterfaceStackInteger(luaWrap_lua_tointeger(L, index)));
    else if (t == sim_stackitem_string)
    {
        size_t l;
        const char* c = luaWrap_lua_tobuffer(L, index, &l);
        //        if (CCbor::isText(c, l))
        //            return (new CInterfaceStackString(c)); // text
        return (new CInterfaceStackString(c, l, false));
    }
    else if (t == sim_stackitem_table)
    { // this part is more tricky:
        size_t rows, cols;
        std::vector<double> matrix;
        if (luaWrap_lua_isbuffer(L, index))
        { // this is a buffer
            size_t l;
            const char* c = luaWrap_lua_tobuffer(L, index, &l);
            return (new CInterfaceStackString(c, l, true));
        }
        else if (luaWrap_lua_isMatrix(L, index, &rows, &cols, &matrix))
        { // this is a matrix
            return new CInterfaceStackMatrix(matrix.data(), rows, cols);
        }
        else
        { // Following to avoid getting trapped in circular references:
            void* p = (void*)luaWrap_lua_topointer(L, index);
            std::map<void*, bool>::iterator it = visitedTables.find(p);
            CInterfaceStackTable* table = nullptr;
            if (it != visitedTables.end())
            { // we have a circular reference!
                table = new CInterfaceStackTable();
                table->setCircularRef();
            }
            else
            {
                visitedTables[p] = true;
                int tableValueCnt = _countInterpreterStackTableEntries_lua(L, index);
                int arraySize = int(luaWrap_lua_rawlen(L, index));
                if (tableValueCnt == arraySize)
                { // we have an array (or keys that go from "1" to arraySize):
                    table = _generateTableArrayFromInterpreterStack_lua(L, index, visitedTables);
                }
                else
                { // we have a more complex table, a map, where the keys are specific:
                    table = _generateTableMapFromInterpreterStack_lua(L, index, visitedTables);
                }
                it = visitedTables.find(p);
                visitedTables.erase(it);
            }
            return (table);
        }
    }
    else
    { // following types translate to strings (i.e. can't be handled outside of the Lua state)
        void* p = (void*)luaWrap_lua_topointer(L, index);
        char num[21];
        snprintf(num, 20, "%p", p);
        std::string str;
        if (t == sim_stackitem_userdat)
            str = "<USERDATA ";
        else if (t == sim_stackitem_func)
            str = "<FUNCTION ";
        else if (t == sim_stackitem_thread)
            str = "<THREAD ";
        else if (t == sim_stackitem_lightuserdat)
            str = "<LIGHTUSERDATA ";
        else
            str = "<UNKNOWNTYPE ";
        str += num;
        str += ">";
        return (new CInterfaceStackString(str.c_str()));
    }
}

void CScriptObject::_pushOntoInterpreterStack_lua(void* LL, CInterfaceStackObject* obj)
{ // !! LL is not the same for a script when in normal or inside a coroutine !!
    luaWrap_lua_State* L = (luaWrap_lua_State*)LL;
    int t = obj->getObjectType();
    if (t == sim_stackitem_null)
        luaWrap_lua_pushnil(L);
    else if (t == sim_stackitem_bool)
        luaWrap_lua_pushboolean(L, ((CInterfaceStackBool*)obj)->getValue());
    else if (t == sim_stackitem_double)
        luaWrap_lua_pushnumber(L, ((CInterfaceStackNumber*)obj)->getValue());
    else if (t == sim_stackitem_integer)
        luaWrap_lua_pushinteger(L, ((CInterfaceStackInteger*)obj)->getValue());
    else if (t == sim_stackitem_string)
    {
        size_t l;
        const char* str = ((CInterfaceStackString*)obj)->getValue(&l);
        if (((CInterfaceStackString*)obj)->isBuffer())
            luaWrap_lua_pushbuffer(L, str, l);
        else
        {
            if (((CInterfaceStackString*)obj)->isText())
                luaWrap_lua_pushtext(L, str);
            else
                luaWrap_lua_pushbinarystring(L, str, l);
        }
    }
    else if (t == sim_stackitem_matrix)
    {
        size_t r, c;
        const double* matr = ((CInterfaceStackMatrix*)obj)->getValue(r, c);
        luaWrap_lua_pushmatrix(L, matr, r, c);
    }
    else if (t == sim_stackitem_table)
    {
        luaWrap_lua_newtable(L);
        CInterfaceStackTable* table = (CInterfaceStackTable*)obj;
        if (table->isTableArray())
        { // array-type table
            for (size_t i = 0; i < table->getArraySize(); i++)
            {
                CInterfaceStackObject* tobj = table->getArrayItemAtIndex(i);
                _pushOntoInterpreterStack_lua(L, tobj);
                luaWrap_lua_rawseti(L, -2, int(i) + 1);
            }
        }
        else
        { // map-type table
            for (size_t i = 0; i < table->getMapEntryCount(); i++)
            {
                std::string stringKey;
                double numberKey;
                long long int integerKey;
                bool boolKey;
                int keyType = -1;
                CInterfaceStackObject* tobj = table->getMapItemAtIndex(i, stringKey, numberKey, integerKey, boolKey, keyType);
                if (keyType == sim_stackitem_string)
                    luaWrap_lua_pushbinarystring(L, stringKey.c_str(), stringKey.size());
                else if (keyType == sim_stackitem_double)
                    luaWrap_lua_pushnumber(L, numberKey);
                else if (keyType == sim_stackitem_integer)
                    luaWrap_lua_pushinteger(L, integerKey);
                else if (keyType == sim_stackitem_bool)
                    luaWrap_lua_pushboolean(L, boolKey);
                else
                { // we have an invalid key
                    std::string s(utils::generateUniqueAlphaNumericString());
                    s += " (Invalid key)";
                    luaWrap_lua_pushtext(L, s.c_str());
                }
                _pushOntoInterpreterStack_lua(L, tobj);
                luaWrap_lua_settable(L, -3);
            }
        }
    }
}

void CScriptObject::signalSet(const char* sigName, long long int target /*= sim_handle_scene*/)
{ // sigName is xx.signal.name (no type info), with xx. 'app.', '' or 'ojb.' (for app, scene or object)
    _signalNameToScriptHandle[sigName] = std::make_pair(_scriptHandle, target);
}

void CScriptObject::signalRemoved(const char* sigName)
{ // sigName is xx.signal.name (no type info), with xx. 'app.', '' or 'ojb.' (for app, scene or object)
    _signalNameToScriptHandle.erase(sigName);
}

int CScriptObject::setBoolProperty(const char* pName, bool pState)
{
    int retVal = -1;

    if (strcmp(propScriptObj_scriptDisabled.name, pName) == 0)
    {
        retVal = 1;
        setScriptIsDisabled(pState);
    }
    else if (strcmp(propScriptObj_restartOnError.name, pName) == 0)
    {
        retVal = 1;
        setAutoRestartOnError(pState);
    }

    return retVal;
}

int CScriptObject::getBoolProperty(const char* pName, bool& pState) const
{
    int retVal = -1;

    if (strcmp(propScriptObj_scriptDisabled.name, pName) == 0)
    {
        retVal = 1;
        pState = _scriptIsDisabled;
    }
    else if (strcmp(propScriptObj_restartOnError.name, pName) == 0)
    {
        retVal = 1;
        pState = _autoRestartOnError;
    }

    return retVal;
}

int CScriptObject::setIntProperty(const char* pName, int pState)
{
    int retVal = -1;

    if (strcmp(propScriptObj_execPriority.name, pName) == 0)
    {
        retVal = 1;
        setScriptExecPriority(pState);
    }

    return retVal;
}

int CScriptObject::getIntProperty(const char* pName, int& pState) const
{
    int retVal = -1;

    if (strcmp(propScriptObj_execPriority.name, pName) == 0)
    {
        retVal = 1;
        pState = getScriptExecPriority();
    }
    else if (strcmp(propScriptObj_scriptType.name, pName) == 0)
    {
        retVal = 1;
        pState = _scriptType;
    }
    else if (strcmp(propScriptObj_executionDepth.name, pName) == 0)
    {
        retVal = 1;
        pState = _executionDepth;
    }
    else if (strcmp(propScriptObj_scriptState.name, pName) == 0)
    {
        retVal = 1;
        pState = _scriptState;
    }

    return retVal;
}

int CScriptObject::setLongProperty(const char* pName, long long int pState)
{
    int retVal = -1;

    return retVal;
}

int CScriptObject::getLongProperty(const char* pName, long long int& pState) const
{
    int retVal = -1;

    return retVal;
}

int CScriptObject::setStringProperty(const char* pName, const char* pState)
{
    int retVal = -1;

    if (strcmp(propScriptObj_code.name, pName) == 0)
    {
        retVal = 1;
        setScriptText(pState);
    }

    return retVal;
}

int CScriptObject::getStringProperty(const char* pName, std::string& pState) const
{
    int retVal = -1;

    if (strcmp(propScriptObj_code.name, pName) == 0)
    {
        retVal = 1;
#ifdef SIM_WITH_GUI
        if (GuiApp::mainWindow != nullptr)
            GuiApp::mainWindow->codeEditorContainer->saveOrCopyOperationAboutToHappen();
#endif
        pState = _scriptText;
    }
    else if (strcmp(propScriptObj_language.name, pName) == 0)
    {
        retVal = 1;
        pState = _lang;
    }
    else if (strcmp(propScriptObj_scriptName.name, pName) == 0)
    {
        retVal = 1;
        pState = getScriptName();
    }
    else if (strcmp(propScriptObj_addOnPath.name, pName) == 0)
    {
        retVal = 1;
        pState = _addOnPath;
    }
    else if (strcmp(propScriptObj_addOnMenuPath.name, pName) == 0)
    {
        retVal = 1;
        pState = _addOnMenuPath;
    }

    return retVal;
}

int CScriptObject::getPropertyName(int& index, std::string& pName, std::string* appartenance) const
{
    int retVal = CScriptObject::getPropertyName_static(index, pName, appartenance);
    return retVal;
}

int CScriptObject::getPropertyName_static(int& index, std::string& pName, std::string* appartenance)
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_scriptObject.size(); i++)
    {
        if ((pName.size() == 0) || utils::startsWith(allProps_scriptObject[i].name, pName.c_str()))
        {
            if ((allProps_scriptObject[i].flags & sim_propertyinfo_deprecated) == 0)
            {
                index--;
                if (index == -1)
                {
                    pName = allProps_scriptObject[i].name;
                    retVal = 1;
                    if (appartenance != nullptr)
                        appartenance[0] += ".detachedScript";
                    break;
                }
            }
        }
    }
    return retVal;
}

int CScriptObject::getPropertyInfo(const char* pName, int& info, std::string& infoTxt, bool detachedScript) const
{
    int retVal = CScriptObject::getPropertyInfo_static(pName, info, infoTxt, detachedScript);
    if (retVal != -1)
    {
        if (strcmp(propScriptObj_code.name, pName) == 0)
        {
            if (_scriptText.size() > LARGE_PROPERTY_SIZE)
                info = info | 0x100;
        }
    }
    return retVal;
}

int CScriptObject::getPropertyInfo_static(const char* ppName, int& info, std::string& infoTxt, bool detachedScript)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "detachedScript."));
    const char* pName = ppName;
    if (detachedScript)
        pName = _pName.c_str();
    int retVal = -1;
    for (size_t i = 0; i < allProps_scriptObject.size(); i++)
    {
        if (strcmp(allProps_scriptObject[i].name, pName) == 0)
        {
            retVal = allProps_scriptObject[i].type;
            info = allProps_scriptObject[i].flags;
            if ((infoTxt == "") && (strcmp(allProps_scriptObject[i].infoTxt, "") != 0))
                infoTxt = allProps_scriptObject[i].infoTxt;
            else
                infoTxt = allProps_scriptObject[i].shortInfoTxt;
            break;
        }
    }
    return retVal;
}

// OLD
// **************************************************************
// **************************************************************

void CScriptObject::setExecutionPriority_old(int order)
{
    _executionPriority_old = tt::getLimitedInt(sim_scriptexecorder_first, sim_scriptexecorder_last, order);
}
int CScriptObject::getExecutionPriority_old() const
{
    return (_executionPriority_old);
}
void CScriptObject::setObjectCustomData_old(int header, const char* data, int dataLength)
{
    if (_customObjectData_old == nullptr)
        _customObjectData_old = new CCustomData_old();
    _customObjectData_old->setData(header, data, dataLength);
}
int CScriptObject::getObjectCustomDataLength_old(int header) const
{
    if (_customObjectData_old == nullptr)
        return (0);
    return (_customObjectData_old->getDataLength(header));
}
void CScriptObject::getObjectCustomData_old(int header, char* data) const
{
    if (_customObjectData_old == nullptr)
        return;
    _customObjectData_old->getData(header, data);
}
void CScriptObject::setCustScriptDisabledDSim_compatibilityMode_DEPRECATED(bool disabled)
{
    _custScriptDisabledDSim_compatibilityMode_DEPRECATED = disabled;
}
bool CScriptObject::getCustScriptDisabledDSim_compatibilityMode_DEPRECATED() const
{
    return (_custScriptDisabledDSim_compatibilityMode_DEPRECATED);
}
void CScriptObject::setCustomizationScriptCleanupBeforeSave_DEPRECATED(bool doIt)
{
    _customizationScriptCleanupBeforeSave_DEPRECATED = doIt;
}
bool CScriptObject::getCustomizationScriptCleanupBeforeSave_DEPRECATED() const
{
    return (_customizationScriptCleanupBeforeSave_DEPRECATED);
}
int CScriptObject::_getScriptNameIndexNumber_old() const
{
    int retVal = -1;
    if ((_scriptType == sim_scripttype_simulation) || (_scriptType == sim_scripttype_customization))
    {
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectHandle);
        if (it != nullptr)
            retVal = tt::getNameSuffixNumber(it->getObjectName_old().c_str(), true);
    }
    return (retVal);
}
std::string CScriptObject::getScriptPseudoName_old() const
{
    if ((_scriptType == sim_scripttype_simulation) || (_scriptType == sim_scripttype_customization))
    {
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectHandle);
        if (it != nullptr)
            return (it->getObjectName_old());
    }
    if ((_scriptType == sim_scripttype_addon) || (_scriptType == sim_scripttype_addonfunction))
        return (_addOnMenuName);
    return ("");
}

int CScriptObject::callScriptFunction_DEPRECATED(const char* functionName, SLuaCallBack* pdata)
{                    // DEPRECATED
    int retVal = -1; // means error

    if (_scriptState != scriptState_initialized)
        return (retVal);

    changeOverallYieldingForbidLevel(1, false);
    luaWrap_lua_State* L = (luaWrap_lua_State*)_interpreterState;
    int oldTop = luaWrap_lua_gettop(L); // We store lua's stack

    _execSimpleString_safe_lua(L, "sim_call_type=-1"); // for backward compatibility

    // Push the function name onto the stack (will be automatically popped from stack after luaWrap_lua_pcall):
    std::string func(functionName);
    size_t ppos = func.find('.');
    if (ppos == std::string::npos)
        luaWrap_lua_getglobal(L, func.c_str()); // in case we have a global function:
    else
    { // in case we have a function that is not global
        std::string globalVar(func.begin(), func.begin() + ppos);
        luaWrap_lua_getglobal(L, globalVar.c_str());
        func.assign(func.begin() + ppos + 1, func.end());
        size_t ppos = func.find('.');
        while (ppos != std::string::npos)
        {
            std::string var(func.begin(), func.begin() + ppos);
            luaWrap_lua_getfield(L, -1, var.c_str());
            luaWrap_lua_remove(L, -2);
            func.erase(func.begin(), func.begin() + ppos + 1);
            ppos = func.find('.');
        }
        luaWrap_lua_getfield(L, -1, func.c_str());
        luaWrap_lua_remove(L, -2);
    }

    // Push the arguments onto the stack (will be automatically popped from stack after luaWrap_lua_pcall):
    int inputArgs = pdata->inputArgCount;
    if (inputArgs != 0)
    {
        int boolPt = 0;
        int intPt = 0;
        int floatPt = 0;
        int doublePt = 0;
        int stringPt = 0;
        int stringBuffPt = 0;
        for (int i = 0; i < pdata->inputArgCount; i++)
            writeCustomFunctionDataOntoStack_old(
                L, pdata->inputArgTypeAndSize[2 * i + 0], pdata->inputArgTypeAndSize[2 * i + 1], pdata->inputBool,
                boolPt, pdata->inputInt, intPt, pdata->inputFloat, floatPt, pdata->inputDouble, doublePt,
                pdata->inputChar, stringPt, pdata->inputCharBuff, stringBuffPt);
    }

    luaWrap_lua_getglobal(L, "debug");
    luaWrap_lua_getfield(L, -1, "traceback");
    luaWrap_lua_remove(L, -2);
    int argCnt = inputArgs;
    int errindex = -argCnt - 2;
    luaWrap_lua_insert(L, errindex);

    if (_executionDepth == 0)
        _timeOfScriptExecutionStart = int(VDateTime::getTimeInMs());
    setExecutionDepth(_executionDepth + 1);
    if (luaWrap_lua_pcall((luaWrap_lua_State*)_interpreterState, argCnt, luaWrapGet_LUA_MULTRET(), errindex) != 0)
    { // a runtime error occurred!
        setExecutionDepth(_executionDepth - 1);
        if (_executionDepth == 0)
            _timeOfScriptExecutionStart = -1;
        std::string errMsg;
        if (luaWrap_lua_isstring(L, -1))
            errMsg = std::string(luaWrap_lua_tostring(L, -1));
        else
            errMsg = "(error unknown)";
        luaWrap_lua_pop(L, 1); // pop error from stack
        _announceErrorWasRaisedAndPossiblyPauseSimulation(errMsg.c_str(), true);

        // Following probably not needed:
        pdata->outputBool = new bool[0];
        pdata->outputInt = new int[0];
        pdata->outputFloat = new float[0];
        pdata->outputDouble = new double[0];
        pdata->outputChar = new char[0];
        pdata->outputCharBuff = new char[0];
    }
    else
    { // return values:
        setExecutionDepth(_executionDepth - 1);
        if (_executionDepth == 0)
            _timeOfScriptExecutionStart = -1;
        int currentTop = luaWrap_lua_gettop(L);

        // Following line new since 7/3/2016:
        int numberOfArgs = currentTop - oldTop - 1; // the first arg is linked to the debug mechanism

        // We read all arguments from the stack
        std::vector<char> outBoolVector;
        std::vector<int> outIntVector;
        std::vector<float> outFloatVector;
        std::vector<double> outDoubleVector;
        std::vector<std::string> outStringVector;
        std::vector<std::string> outCharVector;
        std::vector<int> outInfoVector;
        for (int i = 0; i < numberOfArgs; i++)
        {
            // Following line new since 7/3/2016:
            if (!readCustomFunctionDataFromStack_old(L, oldTop + i + 1 + 1, pdata->outputArgTypeAndSize[i * 2 + 0],
                                                     outBoolVector, outIntVector, outFloatVector, outDoubleVector,
                                                     outStringVector, outCharVector, outInfoVector))
                break;
        }

        // Now we prepare the output buffers:
        pdata->outputBool = new bool[outBoolVector.size()];
        pdata->outputInt = new int[outIntVector.size()];
        pdata->outputFloat = new float[outFloatVector.size()];
        pdata->outputDouble = new double[outDoubleVector.size()];
        int charCnt = 0;
        for (size_t k = 0; k < outStringVector.size(); k++)
            charCnt += (int)outStringVector[k].length() + 1; // terminal 0
        pdata->outputChar = new char[charCnt];

        int charBuffCnt = 0;
        for (size_t k = 0; k < outCharVector.size(); k++)
            charBuffCnt += (int)outCharVector[k].length();
        pdata->outputCharBuff = new char[charBuffCnt];

        pdata->outputArgCount = int(outInfoVector.size() / 2);
        delete[] pdata->outputArgTypeAndSize;
        pdata->outputArgTypeAndSize = new int[outInfoVector.size()];
        // We fill the output buffers:
        for (int k = 0; k < int(outBoolVector.size()); k++)
            pdata->outputBool[k] = outBoolVector[k];
        for (int k = 0; k < int(outIntVector.size()); k++)
            pdata->outputInt[k] = outIntVector[k];
        for (int k = 0; k < int(outFloatVector.size()); k++)
            pdata->outputFloat[k] = outFloatVector[k];
        for (int k = 0; k < int(outDoubleVector.size()); k++)
            pdata->outputDouble[k] = outDoubleVector[k];
        charCnt = 0;
        for (size_t k = 0; k < outStringVector.size(); k++)
        {
            for (size_t l = 0; l < outStringVector[k].length(); l++)
                pdata->outputChar[charCnt + l] = outStringVector[k][l];
            charCnt += (int)outStringVector[k].length();
            // terminal 0:
            pdata->outputChar[charCnt] = 0;
            charCnt++;
        }

        charBuffCnt = 0;
        for (size_t k = 0; k < outCharVector.size(); k++)
        {
            for (size_t l = 0; l < outCharVector[k].length(); l++)
                pdata->outputCharBuff[charBuffCnt + l] = outCharVector[k][l];
            charBuffCnt += (int)outCharVector[k].length();
        }

        for (int k = 0; k < int(outInfoVector.size()); k++)
            pdata->outputArgTypeAndSize[k] = outInfoVector[k];

        retVal = 0;
    }
    luaWrap_lua_settop(L, oldTop); // We restore lua's stack
    changeOverallYieldingForbidLevel(-1, false);
    return (retVal);
}
int CScriptObject::setScriptVariable_old(const char* variableName, CInterfaceStack* stack)
{
    int retVal = -1;
    if (_scriptState != scriptState_initialized)
        return (retVal);
    luaWrap_lua_State* L = (luaWrap_lua_State*)_interpreterState;
    int oldTop = luaWrap_lua_gettop(L); // We store lua's stack

    std::string var(variableName);
    size_t ppos = var.find('.');
    if (ppos == std::string::npos)
    { // in case we have a global variable
        if (stack != nullptr)
            buildOntoInterpreterStack_lua(L, stack, true);
        else
            luaWrap_lua_pushnil(L);
        luaWrap_lua_setglobal(L, variableName);
        retVal = 0;
    }
    else
    { // in case we have a variable that is not global
        std::string globalVar(var.begin(), var.begin() + ppos);
        luaWrap_lua_getglobal(L, globalVar.c_str());
        if (luaWrap_lua_isnonbuffertable(L, -1))
        {
            var.assign(var.begin() + ppos + 1, var.end());
            size_t ppos = var.find('.');
            bool badVar = false;
            while (ppos != std::string::npos)
            {
                std::string vvar(var.begin(), var.begin() + ppos);
                luaWrap_lua_getfield(L, -1, vvar.c_str());
                luaWrap_lua_remove(L, -2);
                var.erase(var.begin(), var.begin() + ppos + 1);
                ppos = var.find('.');
                if (!luaWrap_lua_isnonbuffertable(L, -1))
                {
                    badVar = true;
                    break;
                }
            }
            if (!badVar)
            {
                if (stack != nullptr)
                    buildOntoInterpreterStack_lua(L, stack, true);
                else
                    luaWrap_lua_pushnil(L);
                luaWrap_lua_setfield(L, -2, var.c_str());
                retVal = 0;
            }
        }
    }

    luaWrap_lua_settop(L, oldTop); // We restore lua's stack
    return (retVal);
}
int CScriptObject::clearScriptVariable_DEPRECATED(const char* variableName)
{ // deprecated
    luaWrap_lua_State* L = (luaWrap_lua_State*)_interpreterState;
    if (L == nullptr)
        return (-1);

    if (_scriptIsDisabled)
        return (-1);

    int oldTop = luaWrap_lua_gettop(L); // We store lua's stack

    std::string var(variableName);
    size_t ppos = var.find('.');
    if (ppos == std::string::npos)
    { // in case we have a global variable
        luaWrap_lua_pushnil(L);
        luaWrap_lua_setglobal(L, variableName);
    }
    else
    { // in case we have a variable that is not global
        std::string globalVar(var.begin(), var.begin() + ppos);
        luaWrap_lua_getglobal(L, globalVar.c_str());
        var.assign(var.begin() + ppos + 1, var.end());
        size_t ppos = var.find('.');
        while (ppos != std::string::npos)
        {
            std::string vvar(var.begin(), var.begin() + ppos);
            luaWrap_lua_getfield(L, -1, vvar.c_str());
            luaWrap_lua_remove(L, -2);
            var.erase(var.begin(), var.begin() + ppos + 1);
            ppos = var.find('.');
        }
        luaWrap_lua_pushnil(L);
        luaWrap_lua_setfield(L, -2, var.c_str());
    }

    luaWrap_lua_settop(L, oldTop); // We restore lua's stack
    return (0);
}
CUserParameters* CScriptObject::getScriptParametersObject_backCompatibility()
{
    return (_scriptParameters_backCompatibility);
}

int CScriptObject::appendTableEntry_DEPRECATED(const char* arrayName, const char* keyName, const char* data,
                                               const int what[2])
{ // DEPRECATED since 23/2/2016
    luaWrap_lua_State* L = (luaWrap_lua_State*)_interpreterState;

    if ((_scriptIsDisabled) || (L == nullptr))
        return (-1);

    int oldTop = luaWrap_lua_gettop(L); // We store lua's stack

    // First check if the table where we want to append a value exists. If not, or not a table, create it!
    luaWrap_lua_getglobal(L, arrayName);
    if (!luaWrap_lua_isnonbuffertable(L, -1))
    { // the table is inexistant
        luaWrap_lua_newtable(L);
        luaWrap_lua_setglobal(L, arrayName);
    }
    luaWrap_lua_pop(L, 1);

    // The table where we want to append a value:
    luaWrap_lua_getglobal(L, arrayName);
    int theTablePos = luaWrap_lua_gettop(L);
    int theTableLength = int(luaWrap_lua_rawlen(L, theTablePos));

    // Do we want to simply insert the value, or do we want to insert a keyed value?
    if ((keyName == nullptr) || (strlen(keyName) == 0))
    { // not keyed value:
    }
    else
    { // keyed value:
        luaWrap_lua_pushtext(L, keyName);
    }

    // Now push the value, which might itself be a table:
    int w = what[0];
    if ((w & sim_script_arg_table) != 0)
    { // we have a table
        w -= sim_script_arg_table;
        luaWrap_lua_newtable(L);
        int newTablePos = luaWrap_lua_gettop(L);
        int stringOff = 0;
        for (int i = 0; i < what[1]; i++)
        {
            if (w == sim_script_arg_null)
                luaWrap_lua_pushnil(L);
            if (w == sim_script_arg_bool)
                luaWrap_lua_pushboolean(L, data[i]);
            if (w == sim_script_arg_int32)
                luaWrap_lua_pushinteger(L, ((int*)data)[i]);
            if (w == sim_script_arg_float)
                luaWrap_lua_pushnumber(L, ((double*)data)[i]);
            if (w == sim_script_arg_double)
                luaWrap_lua_pushnumber(L, ((double*)data)[i]);
            if (w == sim_script_arg_string)
            {
                luaWrap_lua_pushtext(L, data + stringOff);
                stringOff += int(strlen(data + stringOff)) + 1;
            }
            luaWrap_lua_rawseti(L, newTablePos, i + 1);
        }
    }
    else
    { // we don't have a table
        if (w == sim_script_arg_null)
            luaWrap_lua_pushnil(L);
        if (w == sim_script_arg_bool)
            luaWrap_lua_pushboolean(L, data[0]);
        if (w == sim_script_arg_int32)
            luaWrap_lua_pushinteger(L, ((int*)data)[0]);
        if (w == sim_script_arg_float)
            luaWrap_lua_pushnumber(L, ((double*)data)[0]);
        if (w == sim_script_arg_double)
            luaWrap_lua_pushnumber(L, ((double*)data)[0]);
        if (w == sim_script_arg_string)
            luaWrap_lua_pushtext(L, data);
        if (w == sim_script_arg_charbuff)
            luaWrap_lua_pushbinarystring(L, data, what[1]); // push binary string for backw. comp.
    }

    // Finally, insert the value in the table:
    if ((keyName == nullptr) || (strlen(keyName) == 0))
    { // not keyed value:
        luaWrap_lua_rawseti(L, theTablePos, theTableLength + 1);
    }
    else
    { // keyed value:
        luaWrap_lua_settable(L, -3);
    }

    luaWrap_lua_settop(L, oldTop); // We restore lua's stack
    return (0);
}
void CScriptObject::setAutomaticCascadingCallsDisabled_old(bool disabled)
{
    _automaticCascadingCallsDisabled_old = disabled;
}
bool CScriptObject::getAutomaticCascadingCallsDisabled_old() const
{
    return (_automaticCascadingCallsDisabled_old);
}
void CScriptObject::_insertScriptText_old(CScriptObject* scriptObject, bool toFront, const char* txt)
{
    std::string theScript(scriptObject->getScriptText());
    if (toFront)
        theScript = std::string(txt) + theScript;
    else
        theScript += txt;
    scriptObject->setScriptText(theScript.c_str());
}

bool CScriptObject::_replaceScriptText_old(CScriptObject* scriptObject, const char* oldTxt, const char* newTxt)
{
    return scriptObject->replaceScriptText(oldTxt, newTxt);
}

bool CScriptObject::_replaceScriptTextKeepMiddleUnchanged_old(CScriptObject* scriptObject, const char* oldTxtStart,
                                                              const char* oldTxtEnd, const char* newTxtStart,
                                                              const char* newTxtEnd)
{ // Will do following: oldTextStart*oldTextEnd --> nextTextStart*newTextEnd
    std::string theScript(scriptObject->getScriptText());
    size_t startPos = theScript.find(oldTxtStart, 0);
    bool replacedSomething = false;
    while (startPos != std::string::npos)
    {
        size_t startPos2 = theScript.find(oldTxtEnd, startPos + strlen(oldTxtStart));
        if (startPos2 != std::string::npos)
        {
            // check if we have a line break in-between:
            bool lineBreak = false;
            for (size_t i = startPos; i < startPos2; i++)
            {
                if ((theScript[i] == (unsigned char)13) || (theScript[i] == (unsigned char)10))
                {
                    lineBreak = true;
                    break;
                }
            }
            if (!lineBreak)
            {
                theScript.replace(startPos2, strlen(oldTxtEnd), newTxtEnd);
                theScript.replace(startPos, strlen(oldTxtStart), newTxtStart);
                startPos = theScript.find(oldTxtStart,
                                          startPos2 + strlen(newTxtEnd) + strlen(newTxtStart) - strlen(oldTxtStart));
                replacedSomething = true;
            }
            else
                startPos = theScript.find(oldTxtStart, startPos + 1);
        }
        else
            startPos = theScript.find(oldTxtStart, startPos + 1);
    }
    if (replacedSomething)
        scriptObject->setScriptText(theScript.c_str());
    return (replacedSomething);
}
bool CScriptObject::_replaceScriptText_old(CScriptObject* scriptObject, const char* oldTxt1, const char* oldTxt2,
                                           const char* oldTxt3, const char* newTxt)
{ // there can be spaces between the 3 words
    std::string theScript(scriptObject->getScriptText());
    size_t l1 = strlen(oldTxt1);
    size_t l2 = strlen(oldTxt2);
    size_t l3 = strlen(oldTxt3);
    bool replacedSomething = false;
    size_t searchStart = 0;
    while (searchStart < theScript.length())
    {
        size_t startPos1 = theScript.find(oldTxt1, searchStart);
        if (startPos1 != std::string::npos)
        {
            searchStart = startPos1 + 1;
            size_t startPos2 = theScript.find(oldTxt2, startPos1 + l1);
            if (startPos2 != std::string::npos)
            {
                bool onlySpaces = true;
                size_t p = startPos1 + l1;
                while (p < startPos2)
                {
                    if (theScript[p] != ' ')
                        onlySpaces = false;
                    p++;
                }
                if (onlySpaces)
                {
                    size_t startPos3 = theScript.find(oldTxt3, startPos2 + l2);
                    if (startPos3 != std::string::npos)
                    {
                        onlySpaces = true;
                        p = startPos2 + l2;
                        while (p < startPos3)
                        {
                            if (theScript[p] != ' ')
                                onlySpaces = false;
                            p++;
                        }
                        if (onlySpaces)
                        { // ok!
                            theScript.replace(startPos1, startPos3 - startPos1 + l3, newTxt);
                            replacedSomething = true;
                        }
                    }
                }
            }
        }
        else
            searchStart = theScript.length();
    }
    if (replacedSomething)
        scriptObject->setScriptText(theScript.c_str());
    return (replacedSomething);
}
bool CScriptObject::_containsScriptText_old(CScriptObject* scriptObject, const char* txt)
{
    const std::string theScript(scriptObject->getScriptText());
    size_t startPos = theScript.find(txt);
    /*
    if (startPos != std::string::npos)
    {
        size_t newlinePos = theScript.rfind('\n', startPos - 1);
        if (newlinePos == std::string::npos)
            newlinePos = 0;
        size_t newlinePos2 = theScript.find('\n', startPos );
        if (newlinePos2 == std::string::npos)
            newlinePos2 = theScript.size();
        std::string str(theScript.begin() + newlinePos + 1, theScript.begin() + newlinePos2);
        printf("** %s**\n", str.c_str());
        return false;
    }
    //*/
    return (startPos != std::string::npos);
}
void CScriptObject::_splitApiText_old(const char* txt, size_t pos, std::string& beforePart, std::string& apiWord,
                                      std::string& afterPart)
{
    size_t endPos;
    for (size_t i = pos; i < strlen(txt); i++)
    {
        char c = txt[i];
        if (((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_') ||
            (c == '.'))
            endPos = i + 1;
        else
            break;
    }
    if (pos > 0)
        beforePart.assign(txt, txt + pos);
    else
        beforePart.clear();
    apiWord.assign(txt + pos, txt + endPos);
    if (endPos < strlen(txt))
        afterPart = txt + endPos;
    else
        afterPart.clear();
}
void CScriptObject::_adjustScriptText1_old(CScriptObject* scriptObject, bool doIt, bool doIt2)
{
    if (!doIt)
        return;
    // here we have to adjust for the new script execution engine (since V3.1.3):
    if ((scriptObject->getScriptType() == sim_scripttype_main) &&
        (!scriptObject->_mainScriptIsDefaultMainScript_old))
    {
        std::string txt;
        txt += DEFAULT_MAINSCRIPT_CODE;
        txt += "\n";
        txt += " \n";
        txt += " \n";
        txt += "------------------------------------------------------------------------------ \n";
        txt += "-- Following main script automatically commented out by CoppeliaSim to guarantee \n";
        txt += "-- compatibility with CoppeliaSim 3.1.3 and later: \n";
        txt += " \n";
        txt += "--[=[ \n";
        txt += " \n";
        _insertScriptText_old(scriptObject, true, txt.c_str());
        txt = "";
        txt += "\n";
        txt += " \n";
        txt += " \n";
        txt += "--]=] \n";
        txt += "------------------------------------------------------------------------------ \n";
        _insertScriptText_old(scriptObject, false, txt.c_str());
    }
    if (scriptObject->getScriptType() == sim_scripttype_simulation)
    {
        _replaceScriptText_old(scriptObject, "\n", "\n\t"); // "\r\n" is also handled

        std::string txt;
        if (doIt2)
        {
            // Add text to the beginning:
            txt += "------------------------------------------------------------------------------ \n";
            txt += "-- Following few lines automatically added by CoppeliaSim to guarantee compatibility \n";
            txt += "-- with CoppeliaSim 3.1.3 and later: \n";
            txt += "if (sim_call_type==sim.syscb_init) then \n";
            txt += " \n";
            txt += "end \n";
            txt += "if (sim_call_type==sim.syscb_cleanup) then \n";
            txt += " \n";
            txt += "end \n";
            txt += "if (sim_call_type==sim.syscb_sensing) then \n";
            txt += "  if not firstTimeHere93846738 then \n";
            txt += "      firstTimeHere93846738=0 \n";
            txt += "  end \n";
            txt +=
                "  simSetScriptAttribute(sim_handle_self,sim_scriptattribute_executioncount,firstTimeHere93846738) "
                "\n";
            txt += "  firstTimeHere93846738=firstTimeHere93846738+1 \n";
            txt += " \n";
            txt += "------------------------------------------------------------------------------ \n";
            txt += " \n";
            txt += " \n";
            _insertScriptText_old(scriptObject, true, txt.c_str());

            // Add text to the end:
            txt = "\n";
            txt += " \n";
            txt += " \n";
            txt += "------------------------------------------------------------------------------ \n";
            txt += "-- Following few lines automatically added by CoppeliaSim to guarantee compatibility \n";
            txt += "-- with CoppeliaSim 3.1.3 and later: \n";
            txt += "end \n";
            txt += "------------------------------------------------------------------------------ \n";
            _insertScriptText_old(scriptObject, false, txt.c_str());

            // Because in old sensing simulation scripts, simHandleChildScript didn't anyway have an effect:
            _replaceScriptText_old(scriptObject, "simHandleChildScript(",
                                   "-- commented by CoppeliaSim: s@imHandleChildScript(");
            _replaceScriptText_old(scriptObject, "s@imHandleChildScript", "simHandleChildScript");
        }
        else
        { // actuation simulation script
            // Add text to the beginning:
            txt += "------------------------------------------------------------------------------ \n";
            txt += "-- Following few lines automatically added by CoppeliaSim to guarantee compatibility \n";
            txt += "-- with CoppeliaSim 3.1.3 and later: \n";
            txt += "if (sim_call_type==sim.syscb_init) then \n";
            txt +=
                "  simSetScriptAttribute(sim_handle_self,sim_childscriptattribute_automaticcascadingcalls,false) "
                "\n";
            txt += "end \n";
            txt += "if (sim_call_type==sim.syscb_cleanup) then \n";
            txt += " \n";
            txt += "end \n";
            txt += "if (sim_call_type==sim.syscb_sensing) then \n";
            txt += "  simHandleChildScripts(sim_call_type) \n";
            txt += "end \n";
            txt += "if (sim_call_type==sim.syscb_actuation) then \n";
            txt += "  if not firstTimeHere93846738 then \n";
            txt += "      firstTimeHere93846738=0 \n";
            txt += "  end \n";
            txt +=
                "  simSetScriptAttribute(sim_handle_self,sim_scriptattribute_executioncount,firstTimeHere93846738) "
                "\n";
            txt += "  firstTimeHere93846738=firstTimeHere93846738+1 \n";
            txt += " \n";
            txt += "------------------------------------------------------------------------------ \n";
            txt += " \n";
            txt += " \n";
            _insertScriptText_old(scriptObject, true, txt.c_str());

            // Add text to the end:
            txt = "\n";
            txt += " \n";
            txt += " \n";
            txt += "------------------------------------------------------------------------------ \n";
            txt += "-- Following few lines automatically added by CoppeliaSim to guarantee compatibility \n";
            txt += "-- with CoppeliaSim 3.1.3 and later: \n";
            txt += "end \n";
            txt += "------------------------------------------------------------------------------ \n";
            _insertScriptText_old(scriptObject, false, txt.c_str());
            _replaceScriptText_old(scriptObject, "simHandleChildScript(", "sim_handle_all_except_explicit", ")",
                                   "simHandleChildScripts(sim_call_type)");
            _replaceScriptText_old(scriptObject, "simHandleChildScript(", "sim_handle_all", ")",
                                   "simHandleChildScripts(sim_call_type)");
            _replaceScriptText_old(scriptObject, "simHandleChildScript(", "sim_handle_all_except_explicit", ",",
                                   "simHandleChildScripts(sim_call_type,");
            _replaceScriptText_old(scriptObject, "simHandleChildScript(", "sim_handle_all", ",",
                                   "simHandleChildScripts(sim_call_type,");

            if (_containsScriptText_old(scriptObject, "simHandleChildScript("))
            { // output a warning
                txt = "Compatibility issue with @@REPLACE@@\n";
                txt += "  Since CoppeliaSim 3.1.3, the function simHandleChildScript is not supported anymore.\n";
                txt += "  It was replaced with simHandleChildScripts (i.e. with an additional 's'),\n";
                txt += "  and operates slightly differently. CoppeliaSim has tried to automatically adjust\n";
                txt += "  the script, but failed. Please correct this issue yourself by editing the script.";
                CWorld::appendLoadOperationIssue(sim_verbosity_warnings, txt.c_str(),
                                                 scriptObject->getScriptHandle());
            }
        }
    }
    if (scriptObject->getScriptType() == sim_scripttype_customization)
    {
        _replaceScriptText_old(scriptObject, "sim_customizationscriptcall_firstaftersimulation",
                               "sim.syscb_aftersimulation");
        _replaceScriptText_old(scriptObject, "sim_customizationscriptcall_lastbeforesimulation",
                               "sim.syscb_beforesimulation");
        _replaceScriptText_old(scriptObject, "sim_customizationscriptcall_first", "sim.syscb_init");
        _replaceScriptText_old(scriptObject, "sim_customizationscriptcall_last", "sim.syscb_cleanup");
    }
}
void CScriptObject::_adjustScriptText2_old(CScriptObject* scriptObject, bool doIt)
{
}
void CScriptObject::_adjustScriptText3_old(CScriptObject* scriptObject, bool doIt)
{
    if (!doIt)
        return;
    // 1. check if we haven't previously added the correction:
    if (!_containsScriptText_old(scriptObject, "@backCompatibility1:"))
    {
        bool modifiedSomething = _replaceScriptTextKeepMiddleUnchanged_old(
            scriptObject, "simSetShapeColor(", ",", "simSetShapeColor(colorCorrectionFunction(", "),");

        if (modifiedSomething)
        {
            // Add text to the beginning:
            std::string txt;
            txt += "------------------------------------------------------------------------------ \n";
            txt += "-- Following few lines automatically added by CoppeliaSim to guarantee compatibility \n";
            txt += "-- with CoppeliaSim 3.1.3 and earlier: \n";
            txt += "colorCorrectionFunction=function(_aShapeHandle_) \n";
            txt += "  local version=simGetIntegerParameter(sim_intparam_program_version) \n";
            txt += "  local revision=simGetIntegerParameter(sim_intparam_program_revision) \n";
            txt += "  if (version==30103 and revision<3) or version<30103 then \n";
            txt += "      return _aShapeHandle_ \n";
            txt += "  end \n";
            txt += "  return '@backCompatibility1:'.._aShapeHandle_ \n";
            txt += "end \n";
            txt += "------------------------------------------------------------------------------ \n";
            txt += " \n";
            txt += " \n";
            _insertScriptText_old(scriptObject, true, txt.c_str());
        }
    }
}
void CScriptObject::_adjustScriptText4_old(CScriptObject* scriptObject, bool doIt)
{
    if (!doIt)
        return;
    _replaceScriptText_old(scriptObject, "res,err=pcall(threadFunction)",
                           "res,err=xpcall(threadFunction,function(err) return debug.traceback(err) end)");
}
void CScriptObject::_adjustScriptText5_old(CScriptObject* scriptObject, bool doIt)
{ // Following since 19/12/2015: not really needed, but better.
    if (!doIt)
        return;
    _replaceScriptText_old(scriptObject, "simGetBooleanParameter", "simGetBoolParameter");
    _replaceScriptText_old(scriptObject, "simSetBooleanParameter", "simSetBoolParameter");
    _replaceScriptText_old(scriptObject, "simGetIntegerParameter", "simGetInt32Parameter");
    _replaceScriptText_old(scriptObject, "simSetIntegerParameter", "simSetInt32Parameter");
    _replaceScriptText_old(scriptObject, "simGetFloatingParameter", "simGetFloatParameter");
    _replaceScriptText_old(scriptObject, "simSetFloatingParameter", "simSetFloatParameter");
    _replaceScriptText_old(scriptObject, "simGetObjectIntParameter", "simGetObjectInt32Parameter");
    _replaceScriptText_old(scriptObject, "simSetObjectIntParameter", "simSetObjectInt32Parameter");
}
void CScriptObject::_adjustScriptText6_old(CScriptObject* scriptObject, bool doIt)
{ // since 19/1/2016 we don't use tabs anymore in embedded scripts:
    if (!doIt)
        return;
    _replaceScriptText_old(scriptObject, "\t", "    "); // tab to 4 spaces
}
void CScriptObject::_adjustScriptText7_old(CScriptObject* scriptObject, bool doIt)
{ // Following since 13/9/2016, but active only since V3.3.3 (or V3.4.0)
    if (!doIt)
        return;
    _replaceScriptText_old(scriptObject, "simPackInts", "simPackInt32Table");
    _replaceScriptText_old(scriptObject, "simPackUInts", "simPackUInt32Table");
    _replaceScriptText_old(scriptObject, "simPackFloats", "simPackFloatTable");
    _replaceScriptText_old(scriptObject, "simPackDoubles", "simPackDoubleTable");
    _replaceScriptText_old(scriptObject, "simPackBytes", "simPackUInt8Table");
    _replaceScriptText_old(scriptObject, "simPackWords", "simPackUInt16Table");
    _replaceScriptText_old(scriptObject, "simUnpackInts", "simUnpackInt32Table");
    _replaceScriptText_old(scriptObject, "simUnpackUInts", "simUnpackUInt32Table");
    _replaceScriptText_old(scriptObject, "simUnpackFloats", "simUnpackFloatTable");
    _replaceScriptText_old(scriptObject, "simUnpackDoubles", "simUnpackDoubleTable");
    _replaceScriptText_old(scriptObject, "simUnpackBytes", "simUnpackUInt8Table");
    _replaceScriptText_old(scriptObject, "simUnpackWords", "simUnpackUInt16Table");
}
void CScriptObject::_adjustScriptText10_old(CScriptObject* scriptObject, bool doIt)
{ // some various small details:
    //   _replaceScriptTextKeepMiddleUnchanged(scriptObject,"sim.include('/",".lua')","require('/","')");
    //   _replaceScriptTextKeepMiddleUnchanged(scriptObject,"sim.include(\"/",".lua\")","require('/","')");
    //   _replaceScriptTextKeepMiddleUnchanged(scriptObject,"sim.include('\\",".lua')","require('\\","')");
    //   _replaceScriptTextKeepMiddleUnchanged(scriptObject,"sim.include(\"\\",".lua\")","require('\\","')");
    _replaceScriptText_old(scriptObject, "sim.include('/lua/graph_customization.lua')",
                           "graph=require('graph_customization')");
    _replaceScriptText_old(scriptObject, "require('/BlueWorkforce/", "require('/bwf/");
    _replaceScriptText_old(scriptObject, "sim.include('/BlueWorkforce/", "sim.include('/bwf/");
    if (!doIt)
        return;
    _replaceScriptText_old(scriptObject, " onclose=\"", " on-close=\"");
    _replaceScriptText_old(scriptObject, " onchange=\"", " on-change=\"");
    _replaceScriptText_old(scriptObject, " onclick=\"", " on-click=\"");
}
void CScriptObject::_adjustScriptText11_old(CScriptObject* scriptObject, bool doIt)
{ // A subtle bug was corrected in below function in CoppeliaSim4.0.1. Below to keep old code working as previously
    if (!doIt)
        return;

    std::string theScript;
    bool addFunc = false;

    theScript = (scriptObject->getScriptText());
    utils::regexReplace(theScript, "sim.getObjectOrientation\\(([^,]+),( *)-1( *)\\)", "blabliblotemp($1,-1)");
    scriptObject->setScriptText(theScript.c_str());
    addFunc = _replaceScriptText_old(scriptObject, "sim.getObjectOrientation(", "__getObjectOrientation__(");
    _replaceScriptText_old(scriptObject, "blabliblotemp", "sim.getObjectOrientation");
    if (addFunc)
    {
        //        CWorld::appendLoadOperationIssue(sim_verbosity_warnings,"compatibility fix in script @@REPLACE@@:\n
        //        replaced some occurrence of sim.getObjectOrientation with __getObjectOrientation__, to fix a possible
        //        bug in versions prior to CoppeliaSim V4.0.1.",scriptObject->getScriptHandle());
        std::string txt;
        txt += "function __getObjectOrientation__(a,b)\n";
        txt +=
            "    -- compatibility routine, wrong results could be returned in some situations, in CoppeliaSim <4.0.1\n";
        txt += "    if b==sim.handle_parent then\n";
        txt += "        b=sim.getObjectParent(a)\n";
        txt += "    end\n";
        txt += "    if (b~=-1) and (sim.getObjectType(b)==sim.object_joint_type) and "
               "(sim.getInt32Parameter(sim.intparam_program_version)>=40001) then\n";
        txt += "        a=a+sim.handleflag_reljointbaseframe\n";
        txt += "    end\n";
        txt += "    return sim.getObjectOrientation(a,b)\n";
        txt += "end\n\n";
        _insertScriptText_old(scriptObject, true, txt.c_str());
    }

    theScript = scriptObject->getScriptText();
    utils::regexReplace(theScript, "sim.setObjectOrientation\\(([^,]+),( *)-1( *),", "blabliblotemp($1,-1,");
    scriptObject->setScriptText(theScript.c_str());
    addFunc = _replaceScriptText_old(scriptObject, "sim.setObjectOrientation(", "__setObjectOrientation__(");
    _replaceScriptText_old(scriptObject, "blabliblotemp", "sim.setObjectOrientation");
    if (addFunc)
    {
        //        CWorld::appendLoadOperationIssue(sim_verbosity_warnings,"compatibility fix in script @@REPLACE@@:\n
        //        replaced some occurrence of sim.setObjectOrientation with __setObjectOrientation__, to fix a possible
        //        bug in versions prior to CoppeliaSim V4.0.1.",scriptObject->getScriptHandle());
        std::string txt;
        txt += "function __setObjectOrientation__(a,b,c)\n";
        txt +=
            "    -- compatibility routine, wrong results could be returned in some situations, in CoppeliaSim <4.0.1\n";
        txt += "    if b==sim.handle_parent then\n";
        txt += "        b=sim.getObjectParent(a)\n";
        txt += "    end\n";
        txt += "    if (b~=-1) and (sim.getObjectType(b)==sim.object_joint_type) and "
               "(sim.getInt32Parameter(sim.intparam_program_version)>=40001) then\n";
        txt += "        a=a+sim.handleflag_reljointbaseframe\n";
        txt += "    end\n";
        txt += "    return sim.setObjectOrientation(a,b,c)\n";
        txt += "end\n";
        _insertScriptText_old(scriptObject, true, txt.c_str());
    }

    theScript = scriptObject->getScriptText();
    utils::regexReplace(theScript, "sim.getObjectQuaternion\\(([^,]+),( *)-1( *)\\)", "blabliblotemp($1,-1)");
    utils::regexReplace(theScript, "sim.getObjectQuaternion\\(([^,]+),( *)sim.handle_parent( *)\\)",
                        "blabliblotemp($1,sim.handle_parent)");
    scriptObject->setScriptText(theScript.c_str());
    addFunc = _replaceScriptText_old(scriptObject, "sim.getObjectQuaternion(", "__getObjectQuaternion__(");
    _replaceScriptText_old(scriptObject, "blabliblotemp", "sim.getObjectQuaternion");
    if (addFunc)
    {
        //        CWorld::appendLoadOperationIssue(sim_verbosity_warnings,"compatibility fix in script @@REPLACE@@:\n
        //        replaced some occurrence of sim.getObjectQuaternion with __getObjectQuaternion__, to fix a possible
        //        bug in versions prior to CoppeliaSim V4.0.1.",scriptObject->getScriptHandle());
        std::string txt;
        txt += "function __getObjectQuaternion__(a,b)\n";
        txt +=
            "    -- compatibility routine, wrong results could be returned in some situations, in CoppeliaSim <4.0.1\n";
        txt += "    if b==sim.handle_parent then\n";
        txt += "        b=sim.getObjectParent(a)\n";
        txt += "    end\n";
        txt += "    if (b~=-1) and (sim.getObjectType(b)==sim.object_joint_type) and "
               "(sim.getInt32Parameter(sim.intparam_program_version)>=40001) then\n";
        txt += "        a=a+sim.handleflag_reljointbaseframe\n";
        txt += "    end\n";
        txt += "    return sim.getObjectQuaternion(a,b)\n";
        txt += "end\n";
        _insertScriptText_old(scriptObject, true, txt.c_str());
    }

    theScript = scriptObject->getScriptText();
    utils::regexReplace(theScript, "sim.setObjectQuaternion\\(([^,]+),( *)-1( *),", "blabliblotemp($1,-1,");
    utils::regexReplace(theScript, "sim.setObjectQuaternion\\(([^,]+),( *)sim.handle_parent( *),",
                        "blabliblotemp($1,sim.handle_parent,");
    scriptObject->setScriptText(theScript.c_str());
    addFunc = _replaceScriptText_old(scriptObject, "sim.setObjectQuaternion(", "__setObjectQuaternion__(");
    _replaceScriptText_old(scriptObject, "blabliblotemp", "sim.setObjectQuaternion");
    if (addFunc)
    {
        //        CWorld::appendLoadOperationIssue(sim_verbosity_warnings,"compatibility fix in script @@REPLACE@@:\n
        //        replaced some occurrence of sim.setObjectQuaternion with __setObjectQuaternion__, to fix a possible
        //        bug in versions prior to CoppeliaSim V4.0.1.",scriptObject->getScriptHandle());
        std::string txt;
        txt += "function __setObjectQuaternion__(a,b,c)\n";
        txt +=
            "    -- compatibility routine, wrong results could be returned in some situations, in CoppeliaSim <4.0.1\n";
        txt += "    if b==sim.handle_parent then\n";
        txt += "        b=sim.getObjectParent(a)\n";
        txt += "    end\n";
        txt += "    if (b~=-1) and (sim.getObjectType(b)==sim.object_joint_type) and "
               "(sim.getInt32Parameter(sim.intparam_program_version)>=40001) then\n";
        txt += "        a=a+sim.handleflag_reljointbaseframe\n";
        txt += "    end\n";
        txt += "    return sim.setObjectQuaternion(a,b,c)\n";
        txt += "end\n";
        _insertScriptText_old(scriptObject, true, txt.c_str());
    }

    theScript = scriptObject->getScriptText();
    utils::regexReplace(theScript, "sim.getObjectPosition\\(([^,]+),( *)-1( *)\\)", "blabliblotemp($1,-1)");
    utils::regexReplace(theScript, "sim.getObjectPosition\\(([^,]+),( *)sim.handle_parent( *)\\)",
                        "blabliblotemp($1,sim.handle_parent)");
    scriptObject->setScriptText(theScript.c_str());
    addFunc = _replaceScriptText_old(scriptObject, "sim.getObjectPosition(", "__getObjectPosition__(");
    _replaceScriptText_old(scriptObject, "blabliblotemp", "sim.getObjectPosition");
    if (addFunc)
    {
        //        CWorld::appendLoadOperationIssue(sim_verbosity_warnings,"compatibility fix in script @@REPLACE@@:\n
        //        replaced some occurrence of sim.getObjectPosition with __getObjectPosition__, to fix a possible bug in
        //        versions prior to CoppeliaSim V4.0.1.",scriptObject->getScriptHandle());
        std::string txt;
        txt += "function __getObjectPosition__(a,b)\n";
        txt +=
            "    -- compatibility routine, wrong results could be returned in some situations, in CoppeliaSim <4.0.1\n";
        txt += "    if b==sim.handle_parent then\n";
        txt += "        b=sim.getObjectParent(a)\n";
        txt += "    end\n";
        txt += "    if (b~=-1) and (sim.getObjectType(b)==sim.object_joint_type) and "
               "(sim.getInt32Parameter(sim.intparam_program_version)>=40001) then\n";
        txt += "        a=a+sim.handleflag_reljointbaseframe\n";
        txt += "    end\n";
        txt += "    return sim.getObjectPosition(a,b)\n";
        txt += "end\n";
        _insertScriptText_old(scriptObject, true, txt.c_str());
    }

    theScript = scriptObject->getScriptText();
    utils::regexReplace(theScript, "sim.setObjectPosition\\(([^,]+),( *)-1( *),", "blabliblotemp($1,-1,");
    utils::regexReplace(theScript, "sim.setObjectPosition\\(([^,]+),( *)sim.handle_parent( *),",
                        "blabliblotemp($1,sim.handle_parent,");
    scriptObject->setScriptText(theScript.c_str());
    addFunc = _replaceScriptText_old(scriptObject, "sim.setObjectPosition(", "__setObjectPosition__(");
    _replaceScriptText_old(scriptObject, "blabliblotemp", "sim.setObjectPosition");
    if (addFunc)
    {
        //        CWorld::appendLoadOperationIssue(sim_verbosity_warnings,"compatibility fix in script @@REPLACE@@:\n
        //        replaced some occurrence of sim.setObjectPosition with __setObjectPosition__, to fix a possible bug in
        //        versions prior to CoppeliaSim V4.0.1.",scriptObject->getScriptHandle());
        std::string txt;
        txt += "function __setObjectPosition__(a,b,c)\n";
        txt +=
            "    -- compatibility routine, wrong results could be returned in some situations, in CoppeliaSim <4.0.1\n";
        txt += "    if b==sim.handle_parent then\n";
        txt += "        b=sim.getObjectParent(a)\n";
        txt += "    end\n";
        txt += "    if (b~=-1) and (sim.getObjectType(b)==sim.object_joint_type) and "
               "(sim.getInt32Parameter(sim.intparam_program_version)>=40001) then\n";
        txt += "        a=a+sim.handleflag_reljointbaseframe\n";
        txt += "    end\n";
        txt += "    return sim.setObjectPosition(a,b,c)\n";
        txt += "end\n";
        _insertScriptText_old(scriptObject, true, txt.c_str());
    }
}
void CScriptObject::_adjustScriptText12_old(CScriptObject* scriptObject, bool doIt)
{ // ROS2 functions:
    if (!doIt)
        return;
    _replaceScriptText_old(scriptObject, "simROS2.serviceClientTreatUInt8ArrayAsString",
                           "simROS2.clientTreatUInt8ArrayAsString");
    _replaceScriptText_old(scriptObject, "simROS2.serviceServerTreatUInt8ArrayAsString",
                           "simROS2.serviceTreatUInt8ArrayAsString");
    _replaceScriptText_old(scriptObject, "simROS2.subscriberTreatUInt8ArrayAsString",
                           "simROS2.subscriptionTreatUInt8ArrayAsString");
    _replaceScriptText_old(scriptObject, "simROS2.imageTransportShutdownSubscriber",
                           "simROS2.imageTransportShutdownSubscription");
    _replaceScriptText_old(scriptObject, "simROS2.imageTransportSubscribe", "simROS2.imageTransportCreateSubscription");
    _replaceScriptText_old(scriptObject, "simROS2.imageTransportAdvertise", "simROS2.imageTransportCreatePublisher");
    _replaceScriptText_old(scriptObject, "simROS2.shutdownServiceServer", "simROS2.shutdownService");
    _replaceScriptText_old(scriptObject, "simROS2.shutdownServiceClient", "simROS2.shutdownClient");
    _replaceScriptText_old(scriptObject, "simROS2.shutdownSubscriber", "simROS2.shutdownSubscription");
    _replaceScriptText_old(scriptObject, "simROS2.advertiseService", "simROS2.createService");
    _replaceScriptText_old(scriptObject, "simROS2.serviceClient", "simROS2.createClient");
    _replaceScriptText_old(scriptObject, "simROS2.subscribe", "simROS2.createSubscription");
    _replaceScriptText_old(scriptObject, "simROS2.advertise", "simROS2.createPublisher");
}
void CScriptObject::_adjustScriptText13_old(CScriptObject* scriptObject, bool doIt)
{ // for release 4.2.0:
    if (!doIt)
        return;
    if (_scriptType != sim_scripttype_main)
        _replaceScriptText_old(scriptObject, "sim.getSimulationState()~=sim.simulation_advancing_abouttostop", "true");
    _replaceScriptText_old(scriptObject, "sim.getObjectAssociatedWithScript(sim.handle_self)",
                           "sim.getObjectHandle(sim.handle_self)");

    if (CSimFlavor::getBoolVal(18) && (_scriptType != sim_scripttype_main))
    {
        const char txt1[] = "function sysCall_actuation()\n\
    if coroutine.status(corout)~='dead' then\n\
        local ok,errorMsg=coroutine.resume(corout)\n\
        if errorMsg then\n\
            error(errorMsg)\n\
        end\n";
        const char txt2[] = "function sysCall_actuation()\n\
    if coroutine.status(corout)~='dead' then\n\
        local ok,errorMsg=coroutine.resume(corout)\n\
        if errorMsg then\n\
            error(debug.traceback(corout,errorMsg),2)\n\
        end\n";
        _replaceScriptText_old(scriptObject, txt1, txt2);
    }
}
bool CScriptObject::_convertThreadedScriptToCoroutine_old(CScriptObject* scriptObject, bool execJustOnce)
{ // try to transform the threaded script into a non-threaded script with coroutines:
    bool retVal = false;
    if (_containsScriptText_old(scriptObject, "sysCall_threadmain"))
    {
        retVal = true;
        _replaceScriptText_old(scriptObject, "sysCall_threadmain", "coroutineMain");
        std::string txt = "function sysCall_init()\n\
    corout=coroutine.create(coroutineMain)\n\
end\n\
\n\
function sysCall_actuation()\n\
    if coroutine.status(corout)~='dead' then\n\
        local ok,errorMsg=coroutine.resume(corout)\n\
        if errorMsg then\n\
            error(debug.traceback(corout,errorMsg),2)\n\
        end\n";
        if (!execJustOnce)
            txt += "    else\n        corout=coroutine.create(coroutineMain)\n";
        txt += "    end\nend\n\n";
        _insertScriptText_old(scriptObject, true, txt.c_str());
    }
    return retVal;
}
void CScriptObject::_adjustScriptText14_old(CScriptObject* scriptObject, bool doIt)
{ // for release 4.2.1:
    if (!doIt)
        return;

    _replaceScriptText_old(scriptObject, "sim.setObjectInt32Parameter", "sim.setObjectInt32Param");
    _replaceScriptText_old(scriptObject, "sim.setObjectFloatParameter", "sim.setObjectFloatParam");
    _replaceScriptText_old(scriptObject, "sim.getObjectStringParameter", "sim.getObjectStringParam");
    _replaceScriptText_old(scriptObject, "sim.setObjectStringParameter", "sim.setObjectStringParam");

    _replaceScriptText_old(scriptObject, "sim.setBoolParameter", "sim.setBoolParam");
    _replaceScriptText_old(scriptObject, "sim.getBoolParameter", "sim.getBoolParam");
    _replaceScriptText_old(scriptObject, "sim.setInt32Parameter", "sim.setInt32Param");
    _replaceScriptText_old(scriptObject, "sim.getInt32Parameter", "sim.getInt32Param");
    _replaceScriptText_old(scriptObject, "sim.setFloatParameter", "sim.setFloatParam");
    _replaceScriptText_old(scriptObject, "sim.getFloatParameter", "sim.getFloatParam");
    _replaceScriptText_old(scriptObject, "sim.setStringParameter", "sim.setStringParam");
    _replaceScriptText_old(scriptObject, "sim.getStringParameter", "sim.getStringParam");
    _replaceScriptText_old(scriptObject, "sim.setArrayParameter", "sim.setArrayParam");
    _replaceScriptText_old(scriptObject, "sim.getArrayParameter", "sim.getArrayParam");

    _replaceScriptText_old(scriptObject, "sim.getEngineBoolParameter", "sim.getEngineBoolParam_old");
    _replaceScriptText_old(scriptObject, "sim.getEngineInt32Parameter", "sim.getEngineInt32Param");
    _replaceScriptText_old(scriptObject, "sim.getEngineFloatParameter", "sim.getEngineFloatParam_old");
    _replaceScriptText_old(scriptObject, "sim.setEngineBoolParameter", "sim.setEngineBoolParam_old");
    _replaceScriptText_old(scriptObject, "sim.setEngineInt32Parameter", "sim.setEngineInt32Param");
    _replaceScriptText_old(scriptObject, "sim.setEngineFloatParameter", "sim.setEngineFloatParam_old");

    _replaceScriptText_old(scriptObject, "sim.setIntegerSignal", "sim.setInt32Signal");
    _replaceScriptText_old(scriptObject, "sim.getIntegerSignal", "sim.getInt32Signal");
    _replaceScriptText_old(scriptObject, "sim.clearIntegerSignal", "sim.clearInt32Signal");
}

void CScriptObject::_adjustScriptText15_old(CScriptObject* scriptObject, bool doIt)
{ // for release 4.3.0 and earlier:
    if (!doIt)
        return;

    _replaceScriptText_old(scriptObject, "sim.rmlPos", "sim.ruckigPos");
    _replaceScriptText_old(scriptObject, "sim.rmlVel", "sim.ruckigVel");
    _replaceScriptText_old(scriptObject, "sim.rmlStep", "sim.ruckigStep");
    _replaceScriptText_old(scriptObject, "sim.rmlRemove", "sim.ruckigRemove");

    _replaceScriptText_old(scriptObject, "sim.rml_phase_sync_if_possible", "sim.ruckig_phasesync");
    _replaceScriptText_old(scriptObject, "sim.rml_only_time_sync", "sim.ruckig_timesync");
    _replaceScriptText_old(scriptObject, "sim.rml_only_phase_sync", "sim.ruckig_phasesync");
    _replaceScriptText_old(scriptObject, "sim.rml_no_sync", "sim.ruckig_nosync");
}

void CScriptObject::_adjustScriptText16_old(CScriptObject* scriptObject, bool doIt)
{ // for release 4.4.0 and earlier:
    if (!doIt)
        return;

    _replaceScriptText_old(scriptObject, "sim.getObjectSelection", "sim.getObjectSel");
    _replaceScriptText_old(scriptObject, "sim.setObjectSelection", "sim.setObjectSel");
    _replaceScriptText_old(scriptObject, "simIK.getJointIkWeight", "simIK.getJointWeight");
    _replaceScriptText_old(scriptObject, "simIK.setJointIkWeight", "simIK.setJointWeight");
    _replaceScriptText_old(scriptObject, "simIK.getIkGroupHandle", "simIK.getGroupHandle");
    _replaceScriptText_old(scriptObject, "simIK.doesIkGroupExist", "simIK.doesGroupExist");
    _replaceScriptText_old(scriptObject, "simIK.createIkGroup", "simIK.createGroup");
    _replaceScriptText_old(scriptObject, "simIK.getIkGroupFlags", "simIK.getGroupFlags");
    _replaceScriptText_old(scriptObject, "simIK.setIkGroupFlags", "simIK.setGroupFlags");
    _replaceScriptText_old(scriptObject, "simIK.getIkGroupCalculation", "simIK.getGroupCalculation");
    _replaceScriptText_old(scriptObject, "simIK.setIkGroupCalculation", "simIK.setGroupCalculation");
    _replaceScriptText_old(scriptObject, "simIK.getIkGroupJointLimitHits", "simIK.getGroupJointLimitHits");
    _replaceScriptText_old(scriptObject, "simIK.addIkElement", "simIK.addElement");
    _replaceScriptText_old(scriptObject, "simIK.getIkElementFlags", "simIK.getElementFlags");
    _replaceScriptText_old(scriptObject, "simIK.setIkElementFlags", "simIK.setElementFlags");
    _replaceScriptText_old(scriptObject, "simIK.getIkElementBase", "simIK.getElementBase");
    _replaceScriptText_old(scriptObject, "simIK.setIkElementBase", "simIK.setElementBase");
    _replaceScriptText_old(scriptObject, "simIK.getIkElementConstraints", "simIK.getElementConstraints");
    _replaceScriptText_old(scriptObject, "simIK.setIkElementConstraints", "simIK.setElementConstraints");
    _replaceScriptText_old(scriptObject, "simIK.getIkElementPrecision", "simIK.getElementPrecision");
    _replaceScriptText_old(scriptObject, "simIK.setIkElementPrecision", "simIK.setElementPrecision");
    _replaceScriptText_old(scriptObject, "simIK.getIkElementWeights", "simIK.getElementWeights");
    _replaceScriptText_old(scriptObject, "simIK.setIkElementWeights", "simIK.setElementWeights");
    _replaceScriptText_old(scriptObject, "simIK.handleIkGroup", "simIK.handleGroup");
    _replaceScriptText_old(scriptObject, "simIK.addIkElementFromScene", "simIK.addElementFromScene");
}

void CScriptObject::_adjustScriptText17_old(CScriptObject* scriptObject, bool doIt)
{ // for release 4.5.1 and earlier:
    if (!doIt)
        return;

    // _replaceScriptText_old(scriptObject,"sim.switchThread","sim.step");
}

void CScriptObject::_detectDeprecated_old(CScriptObject* scriptObject)
{
    /*
    if (_containsScriptText_old(scriptObject, "sim.getStringSignal"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getStringSignal...");
    if (_containsScriptText_old(scriptObject, "sim.readCustomDataBlock"))
        App::logMsg(sim_verbosity_errors, "Contains sim.readCustomDataBlock...");
    if (_containsScriptText_old(scriptObject, "sim.getNamedStringParam"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getNamedStringParam...");
    if (_containsScriptText_old(scriptObject, "sim.getScriptStringParam"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getScriptStringParam...");
    if (_containsScriptText_old(scriptObject, "sim.readCustomDataBlockTags"))
        App::logMsg(sim_verbosity_errors, "Contains sim.readCustomDataBlockTags...");
*/

    /* Explicit requires:
    std::string tmp;
    _scriptText.insert(0,"\n");
    if (_containsScriptText_old(scriptObject,"simROS2"))
        tmp.insert(0,"simROS2=require'simROS2'\n");
    if (_containsScriptText_old(scriptObject,"simROS"))
        tmp.insert(0,"simROS=require'simROS'\n");
    if (_containsScriptText_old(scriptObject,"simICP"))
        tmp.insert(0,"simICP=require'simICP'\n");
    if (_containsScriptText_old(scriptObject,"simIGL"))
        tmp.insert(0,"simIGL=require'simIGL'\n");
    if (_containsScriptText_old(scriptObject,"simEigen"))
        tmp.insert(0,"simEigen=require'simEigen'\n");
    if (_containsScriptText_old(scriptObject,"simIM"))
        tmp.insert(0,"simIM=require'simIM'\n");
    if (_containsScriptText_old(scriptObject,"simZMQ"))
        tmp.insert(0,"simZMQ=require'simZMQ'\n");
    if (_containsScriptText_old(scriptObject,"simWS"))
        tmp.insert(0,"simWS=require'simWS'\n");
    if (_containsScriptText_old(scriptObject,"simVision"))
        tmp.insert(0,"simVision=require'simVision'\n");
    if (_containsScriptText_old(scriptObject,"simURDF"))
        tmp.insert(0,"simURDF=require'simURDF'\n");
    if (_containsScriptText_old(scriptObject,"simSurfRec"))
        tmp.insert(0,"simSurfRec=require'simSurfRec'\n");
    if (_containsScriptText_old(scriptObject,"simSubprocess"))
        tmp.insert(0,"simSubprocess=require'simSubprocess'\n");
    if (_containsScriptText_old(scriptObject,"simSDF"))
        tmp.insert(0,"simSDF=require'simSDF'\n");
    if (_containsScriptText_old(scriptObject,"simRRS1"))
        tmp.insert(0,"simRRS1=require'simRRS1'\n");
    if (_containsScriptText_old(scriptObject,"simQHull"))
        tmp.insert(0,"simQHull=require'simQHull'\n");
    if (_containsScriptText_old(scriptObject,"simOpenMesh"))
        tmp.insert(0,"simOpenMesh=require'simOpenMesh'\n");
    if (_containsScriptText_old(scriptObject,"simOMPL"))
        tmp.insert(0,"simOMPL=require'simOMPL'\n");
    if (_containsScriptText_old(scriptObject,"simMTB"))
        tmp.insert(0,"simMTB=require'simMTB'\n");
    if (_containsScriptText_old(scriptObject,"simCHAI3D"))
        tmp.insert(0,"simCHAI3D=require'simCHAI3D'\n");
    if (_containsScriptText_old(scriptObject,"simBubble"))
        tmp.insert(0,"simBubble=require'simBubble'\n");
    if (_containsScriptText_old(scriptObject,"simGeom"))
        tmp.insert(0,"simGeom=require'simGeom'\n");
    if (_containsScriptText_old(scriptObject,"simAssimp"))
        tmp.insert(0,"simAssimp=require'simAssimp'\n");
    if (_containsScriptText_old(scriptObject,"simMujoco"))
        tmp.insert(0,"simMujoco=require'simMujoco'\n");
    if (_containsScriptText_old(scriptObject,"simUI"))
        tmp.insert(0,"simUI=require'simUI'\n");
    if (_containsScriptText_old(scriptObject,"simIK"))
        tmp.insert(0,"simIK=require'simIK'\n");
    tmp.insert(0,"sim=require'sim'\n");
    printf("********************\n%s\n*********************\n",tmp.c_str());
    _scriptText.insert(0,tmp);
     // */

    /*
    std::smatch match;
    std::regex regEx("sim.getObjectMatrix\\((.+),( *-1 *)\\)");
    while (std::regex_search(_scriptText,match,regEx))
    {
        std::string nt(std::string("sim.getObjectMatrix(")+match.str(1)+",sim.handle_world)");
        _scriptText=std::string(match.prefix())+nt+std::string(match.suffix());
    }
    */
    // if (getLanguage()==sim_lang_lua)
    //    _scriptText.insert(0,"--lua\n\n");

    //    _replaceScriptText_old(scriptObject, "sim.readCustomDataBlock", "sim.readCustomBufferData");
    //    _replaceScriptText_old(scriptObject, "sim.writeCustomDataBlock", "sim.writeCustomBufferData");

    std::smatch match;
    std::regex regEx;
    /*
    if (_containsScriptText_old(scriptObject, "org.conman.cbor"))
    {
        _replaceScriptText_old(scriptObject, "org.conman.cbor", "simCBOR");
        App::logMsg(sim_verbosity_errors, "Contains org.conman.cbor...");
    }

    regEx = "(^|[^A-Za-z0-9_\\.\\-])Matrix\\(";
    bool oldMatrices = false;
    while (std::regex_search(_scriptText, match, regEx))
    {
        oldMatrices = true;
        std::string nt(match.str(1) + std::string("simEigen.Matrix("));
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "(^|[^A-Za-z0-9_\\.\\-])Matrix\\{";
    while (std::regex_search(_scriptText, match, regEx))
    {
        oldMatrices = true;
        std::string nt(match.str(1) + std::string("simEigen.Matrix{"));
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    regEx = "(^|[^A-Za-z0-9_\\.\\-])Vector3\\(";
    while (std::regex_search(_scriptText, match, regEx))
    {
        oldMatrices = true;
        std::string nt(match.str(1) + std::string("simEigen.Vector("));
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "(^|[^A-Za-z0-9_\\.\\-])Vector3\\{";
    while (std::regex_search(_scriptText, match, regEx))
    {
        oldMatrices = true;
        std::string nt(match.str(1) + std::string("simEigen.Vector{"));
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    regEx = "(^|[^A-Za-z0-9_\\.\\-])Vector\\(";
    while (std::regex_search(_scriptText, match, regEx))
    {
        oldMatrices = true;
        std::string nt(match.str(1) + std::string("simEigen.Vector("));
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "(^|[^A-Za-z0-9_\\.\\-])Vector\\{";
    while (std::regex_search(_scriptText, match, regEx))
    {
        oldMatrices = true;
        std::string nt(match.str(1) + std::string("simEigen.Vector{"));
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    if (oldMatrices)
    {
        _scriptText = "local simEigen = require('simEigen')\n" + _scriptText;
        App::logMsg(sim_verbosity_errors, "Replaced old Matrices with simEigen Matrices...");
    }
//*/
    regEx = "sim.setInt32Signal\\('([^,]+),";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setIntProperty(sim.handle_scene, 'signal.") + match.str(1) + ",");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.setFloatSignal\\('([^,]+),";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setFloatProperty(sim.handle_scene, 'signal.") + match.str(1) + ",");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.setStringSignal\\('([^,]+),";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setStringProperty(sim.handle_scene, 'signal.") + match.str(1) + ",");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    regEx = "sim.getInt32Signal\\('([^']+)'";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.getIntProperty(sim.handle_scene, 'signal.") + match.str(1) + "', {noError = true}");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.getFloatSignal\\('([^']+)'";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.getFloatProperty(sim.handle_scene, 'signal.") + match.str(1) + "', {noError = true}");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.getStringSignal\\('([^']+)'";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.getStringProperty(sim.handle_scene, 'signal.") + match.str(1) + "', {noError = true}");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.clearInt32Signal\\('([^']+)'";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.removeProperty(sim.handle_scene, 'signal.") + match.str(1) + "', {noError = true}");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.clearFloatSignal\\('([^']+)'";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.removeProperty(sim.handle_scene, 'signal.") + match.str(1) + "', {noError = true}");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.clearStringSignal\\('([^']+)'";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.removeProperty(sim.handle_scene, 'signal.") + match.str(1) + "', {noError = true}");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    regEx = "sim.setInt32Signal\\(\"([^,]+),";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setIntProperty(sim.handle_scene, \"signal.") + match.str(1) + ",");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.setFloatSignal\\(\"([^,]+),";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setFloatProperty(sim.handle_scene, \"signal.") + match.str(1) + ",");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.setStringSignal\\(\"([^,]+),";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setStringProperty(sim.handle_scene, \"signal.") + match.str(1) + ",");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    regEx = "sim.getInt32Signal\\(\"([^\"]+)\"";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.getIntProperty(sim.handle_scene, \"signal.") + match.str(1) + "\", {noError = true}");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.getFloatSignal\\(\"([^\"]+)\"";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.getFloatProperty(sim.handle_scene, \"signal.") + match.str(1) + "\", {noError = true}");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.getStringSignal\\(\"([^\"]+)\"";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.getStringProperty(sim.handle_scene, \"signal.") + match.str(1) + "\", {noError = true}");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.clearInt32Signal\\(\"([^\"]+)\"";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.removeProperty(sim.handle_scene, \"signal.") + match.str(1) + "\", {noError = true}");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.clearFloatSignal\\(\"([^\"]+)\"";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.removeProperty(sim.handle_scene, \"signal.") + match.str(1) + "\", {noError = true}");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.clearStringSignal\\(\"([^\"]+)\"";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.removeProperty(sim.handle_scene, \"signal.") + match.str(1) + "\", {noError = true}");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    regEx = "sim.writeCustomBufferData\\(([^,]+),'([^,]+),";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setBufferProperty(" + match.str(1) + ", 'customData.") + match.str(2) + ",");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    regEx = "sim.writeCustomBufferData\\(([^,]+), '([^,]+),";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setBufferProperty(" + match.str(1) + ", 'customData.") + match.str(2) + ",");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    regEx = "sim.writeCustomBufferData\\(([^,]+),\"([^,]+),";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setBufferProperty(" + match.str(1) + ", \"customData.") + match.str(2) + ",");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    regEx = "sim.writeCustomBufferData\\(([^,]+), \"([^,]+),";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setBufferProperty(" + match.str(1) + ", \"customData.") + match.str(2) + ",");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    regEx = "sim.readCustomBufferData\\(([^,]+),'([^']+)'";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.getBufferProperty(" + match.str(1) + ", 'customData.") + match.str(2) + "', {noError = true}");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    regEx = "sim.readCustomBufferData\\(([^,]+), '([^']+)'";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.getBufferProperty(" + match.str(1) + ", 'customData.") + match.str(2) + "', {noError = true}");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    regEx = "sim.readCustomBufferData\\(([^,]+),\"([^\"]+)\"";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.getBufferProperty(" + match.str(1) + ", \"customData.") + match.str(2) + "\", {noError = true}");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    regEx = "sim.readCustomBufferData\\(([^,]+), \"([^\"]+)\"";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.getBufferProperty(" + match.str(1) + ", \"customData.") + match.str(2) + "\", {noError = true}");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    regEx = "sim.writeCustomTableData\\(([^,]+),'([^,]+),([^\\)]+)\\)";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setBufferProperty(" + match.str(1) + ", 'customData.") + match.str(2) + ", sim.packTable(" + match.str(3) + "))");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    regEx = "sim.writeCustomTableData\\(([^,]+), '([^,]+),([^\\)]+)\\)";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setBufferProperty(" + match.str(1) + ", 'customData.") + match.str(2) + ", sim.packTable(" + match.str(3) + "))");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    regEx = "sim.writeCustomTableData\\(([^,]+),\"([^,]+),([^\\)]+)\\)";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setBufferProperty(" + match.str(1) + ", \"customData.") + match.str(2) + ", sim.packTable(" + match.str(3) + "))");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    regEx = "sim.writeCustomTableData\\(([^,]+), \"([^,]+),([^\\)]+)\\)";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setBufferProperty(" + match.str(1) + ", \"customData.") + match.str(2) + ", sim.packTable(" + match.str(3) + "))");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    std::string trueV = "true";
    std::string falseV = "false";
    if (scriptObject->getLang() == "python")
    {
        trueV = "True";
        falseV = "False";
    }

    regEx = "sim.setObjectInt32Param\\(([^,]+),\\s*sim.shapeintparam_respondable\\s*,\\s*1\\s*\\)";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setBoolProperty(") + match.str(1) + ", 'respondable', " + trueV + ")");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.setObjectInt32Param\\(([^,]+),\\s*sim.shapeintparam_respondable\\s*,\\s*0\\s*\\)";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setBoolProperty(") + match.str(1) + ", 'respondable', " + falseV + ")");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.setObjectInt32Param\\(([^,]+),\\s*sim.objintparam_visibility_layer";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setIntProperty(") + match.str(1) + ", 'layer'");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.getObjectInt32Param\\(([^,]+),\\s*sim.objintparam_visibility_layer";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.getIntProperty(") + match.str(1) + ", 'layer'");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.setObjectInt32Param\\(([^,]+),\\s*sim.shapeintparam_static\\s*,\\s*1\\s*\\)";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setBoolProperty(") + match.str(1) + ", 'dynamic', " + falseV + ")");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.setObjectInt32Param\\(([^,]+),\\s*sim.shapeintparam_static\\s*,\\s*0\\s*\\)";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt(std::string("sim.setBoolProperty(") + match.str(1) + ", 'dynamic', " + trueV + ")");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }
    regEx = "sim.getFloatParam\\(\\s*sim.floatparam_rand";
    while (std::regex_search(_scriptText, match, regEx))
    {
        std::string nt("sim.getFloatProperty(sim.handle_app, 'randomFloat'");
        _scriptText = std::string(match.prefix()) + nt + std::string(match.suffix());
    }

    if (_containsScriptText_old(scriptObject, "Int32Signal("))
        App::logMsg(sim_verbosity_errors, "Contains Int32Signal(...");
    if (_containsScriptText_old(scriptObject, "FloatSignal("))
        App::logMsg(sim_verbosity_errors, "Contains FloatSignal(...");
    if (_containsScriptText_old(scriptObject, "StringSignal("))
        App::logMsg(sim_verbosity_errors, "Contains StringSignal(...");
    if (_containsScriptText_old(scriptObject, "sim.getSignalName("))
        App::logMsg(sim_verbosity_errors, "Contains sim.getSignalName(...");
    if (_containsScriptText_old(scriptObject, "BoolParam("))
        App::logMsg(sim_verbosity_errors, "Contains BoolParam(...");
    if (_containsScriptText_old(scriptObject, "Int32Param("))
        App::logMsg(sim_verbosity_errors, "Contains Int32Param(...");
    if (_containsScriptText_old(scriptObject, "FloatParam("))
        App::logMsg(sim_verbosity_errors, "Contains FloatParam(...");
    if (_containsScriptText_old(scriptObject, "StringParam("))
        App::logMsg(sim_verbosity_errors, "Contains StringParam(...");
    if (_containsScriptText_old(scriptObject, "ArrayParam("))
        App::logMsg(sim_verbosity_errors, "Contains ArrayParam(...");
    if (_containsScriptText_old(scriptObject, "ObjectProperty("))
        App::logMsg(sim_verbosity_errors, "Contains ObjectProperty(...");
    if (_containsScriptText_old(scriptObject, "ObjectSpecialProperty("))
        App::logMsg(sim_verbosity_errors, "Contains ObjectSpecialProperty(...");
    if (_containsScriptText_old(scriptObject, "ModelProperty("))
        App::logMsg(sim_verbosity_errors, "Contains ModelProperty(...");
    if (_containsScriptText_old(scriptObject, "sim.readCustomString"))
        App::logMsg(sim_verbosity_errors, "Contains sim.readCustomString...");
    if (_containsScriptText_old(scriptObject, "sim.writeCustomString"))
        App::logMsg(sim_verbosity_errors, "Contains sim.writeCustomString...");
    if (_containsScriptText_old(scriptObject, "sim.readCustomBuffer"))
        App::logMsg(sim_verbosity_errors, "Contains sim.readCustomBuffer...");
    if (_containsScriptText_old(scriptObject, "sim.writeCustomBuffer"))
        App::logMsg(sim_verbosity_errors, "Contains sim.writeCustomBuffer...");
    if (_containsScriptText_old(scriptObject, "sim.readCustomTable"))
        App::logMsg(sim_verbosity_errors, "Contains sim.readCustomTable...");
    if (_containsScriptText_old(scriptObject, "sim.writeCustomTable"))
        App::logMsg(sim_verbosity_errors, "Contains sim.writeCustomTable...");

    if (_containsScriptText_old(scriptObject, "LightParameters"))
        App::logMsg(sim_verbosity_errors, "Contains LightParameters...");

    _replaceScriptText_old(scriptObject, "sim.light_omnidirectional_subtype", "sim.light_omnidirectional");
    _replaceScriptText_old(scriptObject, "sim.light_spot_subtype", "sim.light_spot");
    _replaceScriptText_old(scriptObject, "sim.light_directional_subtype", "sim.light_directional");
    _replaceScriptText_old(scriptObject, "sim.joint_revolute_subtype", "sim.joint_revolute");
    _replaceScriptText_old(scriptObject, "sim.joint_prismatic_subtype", "sim.joint_prismatic");
    _replaceScriptText_old(scriptObject, "sim.joint_spherical_subtype", "sim.joint_spherical");
    _replaceScriptText_old(scriptObject, "sim.shape_simpleshape_subtype", "sim.shape_simple");
    _replaceScriptText_old(scriptObject, "sim.shape_multishape_subtype", "sim.shape_compound");
    _replaceScriptText_old(scriptObject, "sim.proximitysensor_pyramid_subtype", "sim.proximitysensor_pyramid");
    _replaceScriptText_old(scriptObject, "sim.proximitysensor_cylinder_subtype", "sim.proximitysensor_cylinder");
    _replaceScriptText_old(scriptObject, "sim.proximitysensor_disc_subtype", "sim.proximitysensor_disc");
    _replaceScriptText_old(scriptObject, "sim.proximitysensor_cone_subtype", "sim.proximitysensor_cone");
    _replaceScriptText_old(scriptObject, "sim.proximitysensor_ray_subtype", "sim.proximitysensor_ray");
    _replaceScriptText_old(scriptObject, "sim.object_shape_type", "sim.sceneobject_shape");
    _replaceScriptText_old(scriptObject, "sim.object_joint_type", "sim.sceneobject_joint");
    _replaceScriptText_old(scriptObject, "sim.object_graph_type", "sim.sceneobject_graph");
    _replaceScriptText_old(scriptObject, "sim.object_camera_type", "sim.sceneobject_camera");
    _replaceScriptText_old(scriptObject, "sim.object_dummy_type", "sim.sceneobject_dummy");
    _replaceScriptText_old(scriptObject, "sim.object_proximitysensor_type", "sim.sceneobject_proximitysensor");
    _replaceScriptText_old(scriptObject, "sim.object_path_type", "sim.sceneobject_path");
    _replaceScriptText_old(scriptObject, "sim.object_visionsensor_type", "sim.sceneobject_visionsensor");
    _replaceScriptText_old(scriptObject, "sim.object_mill_type", "sim.sceneobject_mill");
    _replaceScriptText_old(scriptObject, "sim.object_forcesensor_type", "sim.sceneobject_forcesensor");
    _replaceScriptText_old(scriptObject, "sim.object_light_type", "sim.sceneobject_light");
    _replaceScriptText_old(scriptObject, "sim.object_mirror_type", "sim.sceneobject_mirror");
    _replaceScriptText_old(scriptObject, "sim.object_octree_type", "sim.sceneobject_octree");
    _replaceScriptText_old(scriptObject, "sim.object_pointcloud_type", "sim.sceneobject_pointcloud");
    _replaceScriptText_old(scriptObject, "sim.object_script_type", "sim.sceneobject_script");
    _replaceScriptText_old(scriptObject, "sim.appobj_object_type", "sim.objecttype_sceneobject");
    _replaceScriptText_old(scriptObject, "sim.appobj_collection_type", "sim.objecttype_collection");
    _replaceScriptText_old(scriptObject, "sim.appobj_texture_type", "sim.objecttype_texture");

    _replaceScriptText_old(scriptObject, "sim.scripttype_mainscript", "sim.scripttype_main");
    _replaceScriptText_old(scriptObject, "sim.scripttype_childscript", "sim.scripttype_simulation");
    _replaceScriptText_old(scriptObject, "sim.scripttype_addonscript", "sim.scripttype_addon");
    _replaceScriptText_old(scriptObject, "sim.scripttype_customizationscript", "sim.scripttype_customization");
    _replaceScriptText_old(scriptObject, "sim.scripttype_sandboxscript", "sim.scripttype_sandbox");

    if (_containsScriptText_old(scriptObject, "sim.addScript"))
        App::logMsg(sim_verbosity_errors, "Contains sim.addScript...");
    if (_containsScriptText_old(scriptObject, "sim.associateScriptWithObject"))
        App::logMsg(sim_verbosity_errors, "Contains sim.associateScriptWithObject...");
    if (_containsScriptText_old(scriptObject, "sim.removeScript"))
        App::logMsg(sim_verbosity_errors, "Contains sim.removeScript...");
    if (_containsScriptText_old(scriptObject, "sim.getScriptStringParam"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getScriptStringParam...");
    if (_containsScriptText_old(scriptObject, "sim.setScriptStringParam"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setScriptStringParam...");
    if (_containsScriptText_old(scriptObject, "sim.getScriptInt32Param"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getScriptInt32Param...");
    if (_containsScriptText_old(scriptObject, "sim.setScriptInt32Param"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setScriptInt32Param...");
    if (_containsScriptText_old(scriptObject, "sim.scriptintparam_lang"))
        App::logMsg(sim_verbosity_errors, "Contains sim.scriptintparam_lang...");
    if (_containsScriptText_old(scriptObject, "sim.scriptintparam_handle"))
        App::logMsg(sim_verbosity_errors, "Contains sim.scriptintparam_handle...");
    if (_containsScriptText_old(scriptObject, "sim.scriptintparam_objecthandle"))
        App::logMsg(sim_verbosity_errors, "Contains sim.scriptintparam_objecthandle...");

    if (_containsScriptText_old(scriptObject, "sim.convexDecompose"))
        App::logMsg(sim_verbosity_errors, "Contains sim.convexDecompose...");

    if (_containsScriptText_old(scriptObject, "sim.getQHull"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getQHull...");

    if (_containsScriptText_old(scriptObject, "sim.getDecimatedMesh"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getDecimatedMesh...");

    if (_containsScriptText_old(scriptObject, "sim.removeObject("))
        App::logMsg(sim_verbosity_errors, "Contains sim.removeObject...");

    if (_containsScriptText_old(scriptObject, "sim.readCustomDataBlock"))
        App::logMsg(sim_verbosity_errors, "Contains sim.readCustomDataBlock...");
    if (_containsScriptText_old(scriptObject, "sim.writeCustomDataBlock"))
        App::logMsg(sim_verbosity_errors, "Contains sim.writeCustomDataBlock...");
    if (_containsScriptText_old(scriptObject, "sim.readCustomDataBlockTags"))
        App::logMsg(sim_verbosity_errors, "Contains sim.readCustomDataBlockTags...");

    if (_containsScriptText_old(scriptObject, "sim.dummyintparam_link_type"))
        App::logMsg(sim_verbosity_errors, "Contains sim.dummyintparam_link_type...");

    if (_containsScriptText_old(scriptObject, "sim.dummylink_dynloopclosure"))
        App::logMsg(sim_verbosity_errors, "Contains sim.dummylink_dynloopclosure...");
    if (_containsScriptText_old(scriptObject, "sim.dummylink_dyntendon"))
        App::logMsg(sim_verbosity_errors, "Contains sim.dummylink_dyntendon...");

    if (_containsScriptText_old(scriptObject, "sim.getThreadExitRequest"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getThreadExitRequest...");
    if (_containsScriptText_old(scriptObject, "coroutine.create"))
        App::logMsg(sim_verbosity_errors, "Contains coroutine.create...");

    if (_containsScriptText_old(scriptObject, "sim.switchThread"))
        App::logMsg(sim_verbosity_errors, "Contains sim.switchThread...");
    if (_containsScriptText_old(scriptObject, "sim.setThreadSwitchAllowed"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setThreadSwitchAllowed...");
    if (_containsScriptText_old(scriptObject, "sim.getThreadSwitchAllowed"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getThreadSwitchAllowed...");
    if (_containsScriptText_old(scriptObject, "sim.setThreadAutomaticSwitch"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setThreadAutomaticSwitch...");
    if (_containsScriptText_old(scriptObject, "sim.getThreadAutomaticSwitch"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getThreadAutomaticSwitch...");
    if (_containsScriptText_old(scriptObject, "sim.setThreadSwitchTiming"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setThreadSwitchTiming...");
    if (_containsScriptText_old(scriptObject, "sim.getThreadSwitchTiming"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getThreadSwitchTiming...");

    if (_containsScriptText_old(scriptObject, "sim.shapestringparam_color_name"))
        App::logMsg(sim_verbosity_errors, "Contains sim.shapestringparam_color_name...");
    if (_containsScriptText_old(scriptObject, "sim.invertMatrix"))
        App::logMsg(sim_verbosity_errors, "Contains sim.invertMatrix...");
    if (_containsScriptText_old(scriptObject, "sim.invertPose"))
        App::logMsg(sim_verbosity_errors, "Contains sim.invertPose...");
    if (_containsScriptText_old(scriptObject, "simIK.syncToIkWorld"))
        App::logMsg(sim_verbosity_errors, "Contains simIK.syncToIkWorld...");
    if (_containsScriptText_old(scriptObject, "simIK.syncFromIkWorld"))
        App::logMsg(sim_verbosity_errors, "Contains simIK.syncFromIkWorld...");

    if (_containsScriptText_old(scriptObject, "sim.reorientShapeBoundingBox"))
        App::logMsg(sim_verbosity_errors, "Contains sim.reorientShapeBoundingBox...");
    if (_containsScriptText_old(scriptObject, "sim.createMeshShape"))
        App::logMsg(sim_verbosity_errors, "Contains sim.createMeshShape...");
    if (_containsScriptText_old(scriptObject, "sim.getObjectSelection"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getObjectSelection...");
    if (_containsScriptText_old(scriptObject, "sim.setObjectSelection"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setObjectSelection...");
    if (_containsScriptText_old(scriptObject, "simIK.getLinkedDummy"))
        App::logMsg(sim_verbosity_errors, "Contains simIK.getLinkedDummy...");
    if (_containsScriptText_old(scriptObject, "simIK.setLinkedDummy"))
        App::logMsg(sim_verbosity_errors, "Contains simIK.setLinkedDummy...");

    if (_containsScriptText_old(scriptObject, "simIK.result_not_performed"))
        App::logMsg(sim_verbosity_errors, "Contains simIK.result_not_performed...");
    if (_containsScriptText_old(scriptObject, "simIK.result_fail"))
        App::logMsg(sim_verbosity_errors, "Contains simIK.result_fail...");

    if (_containsScriptText_old(scriptObject, "simIK.applySceneToIkEnvironment"))
        App::logMsg(sim_verbosity_errors, "Contains simIK.applySceneToIkEnvironment...");

    if (_containsScriptText_old(scriptObject, "simIK.applyIkEnvironmentToScene"))
        App::logMsg(sim_verbosity_errors, "Contains simIK.applyIkEnvironmentToScene...");

    if (_containsScriptText_old(scriptObject, "sim.getDoubleSignal"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getDoubleSignal...");
    if (_containsScriptText_old(scriptObject, "sim.setDoubleSignal"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setDoubleSignal...");
    if (_containsScriptText_old(scriptObject, "sim.clearDoubleSignal"))
        App::logMsg(sim_verbosity_errors, "Contains sim.clearDoubleSignal...");

    if (_containsScriptText_old(scriptObject, "sysCall_jointCallback"))
        App::logMsg(sim_verbosity_errors, "Contains sysCall_jointCallback...");
    if (_containsScriptText_old(scriptObject, "sysCall_contactCallback"))
        App::logMsg(sim_verbosity_errors, "Contains sysCall_contactCallback...");
    if (_containsScriptText_old(scriptObject, "sysCall_dynCallback"))
        App::logMsg(sim_verbosity_errors, "Contains sysCall_dynCallback...");

    if (_containsScriptText_old(scriptObject, "sim.jointfloatparam_upper_limit"))
        App::logMsg(sim_verbosity_errors, "Contains sim.jointfloatparam_upper_limit...");

    if (_containsScriptText_old(scriptObject, "sim.dummy_linktype"))
        App::logMsg(sim_verbosity_errors, "Contains sim.dummy_linktype...");

    if (_containsScriptText_old(scriptObject, "sim.jointmode_passive"))
        App::logMsg(sim_verbosity_errors, "Contains sim.jointmode_passive...");
    if (_containsScriptText_old(scriptObject, "sim.jointmode_force"))
        App::logMsg(sim_verbosity_errors, "Contains sim.jointmode_force...");

    if (_containsScriptText_old(scriptObject, "sim.createPureShape"))
        App::logMsg(sim_verbosity_errors, "Contains sim.createPureShape...");

    if (_containsScriptText_old(scriptObject, "sim.getVisionSensorResolution"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getVisionSensorResolution...");
    if (_containsScriptText_old(scriptObject, "sim.getVisionSensorImage"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getVisionSensorImage...");
    if (_containsScriptText_old(scriptObject, "sim.getVisionSensorCharImage"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getVisionSensorCharImage...");
    if (_containsScriptText_old(scriptObject, "sim.setVisionSensorImage"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setVisionSensorImage...");
    if (_containsScriptText_old(scriptObject, "sim.setVisionSensorCharImage"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setVisionSensorCharImage...");
    if (_containsScriptText_old(scriptObject, "sim.getVisionSensorDepthBuffer"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getVisionSensorDepthBuffer...");

    if (_containsScriptText_old(scriptObject, "sim.getSystemTimeInMs"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getSystemTimeInMs...");

    if (_containsScriptText_old(scriptObject, "sim.drawing_trianglepoints"))
        App::logMsg(sim_verbosity_errors, "Contains sim.drawing_trianglepoints...");
    if (_containsScriptText_old(scriptObject, "sim.drawing_quadpoints"))
        App::logMsg(sim_verbosity_errors, "Contains sim.drawing_quadpoints...");
    if (_containsScriptText_old(scriptObject, "sim.drawing_discpoints"))
        App::logMsg(sim_verbosity_errors, "Contains sim.drawing_discpoints...");
    if (_containsScriptText_old(scriptObject, "sim.drawing_cubepoints"))
        App::logMsg(sim_verbosity_errors, "Contains sim.drawing_cubepoints...");
    if (_containsScriptText_old(scriptObject, "sim.drawing_spherepoints"))
        App::logMsg(sim_verbosity_errors, "Contains sim.drawing_spherepoints...");
    if (_containsScriptText_old(scriptObject, "sim.setJointMaxForce"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setJointMaxForce...");
    if (_containsScriptText_old(scriptObject, "sim.getJointMaxForce"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getJointMaxForce...");
    if (_containsScriptText_old(scriptObject, "sim.setScriptAttribute"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setScriptAttribute...");
    if (_containsScriptText_old(scriptObject, "sim.getScriptAttribute"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getScriptAttribute...");
    if (_containsScriptText_old(scriptObject, "sim.getObjectHandle"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getObjectHandle...");
    if (_containsScriptText_old(scriptObject, "simIK.getConfigForTipPose"))
        App::logMsg(sim_verbosity_errors, "Contains simIK.getConfigForTipPose...");
    if (_containsScriptText_old(scriptObject, "sim.getJointMatrix"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getJointMatrix...");
    if (_containsScriptText_old(scriptObject, "sim.setSphericalJointMatrix"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setSphericalJointMatrix...");
    if (_containsScriptText_old(scriptObject, "sim.getObjectUniqueIdentifier"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getObjectUniqueIdentifier...");
    if (_containsScriptText_old(scriptObject, "sim.isObjectInSelection"))
        App::logMsg(sim_verbosity_errors, "Contains sim.isObjectInSelection...");
    if (_containsScriptText_old(scriptObject, "sim.addObjectToSelection"))
        App::logMsg(sim_verbosity_errors, "Contains sim.addObjectToSelection...");
    if (_containsScriptText_old(scriptObject, "sim.removeObjectFromSelection"))
        App::logMsg(sim_verbosity_errors, "Contains sim.removeObjectFromSelection...");
    if (_containsScriptText_old(scriptObject, "sim.getObjectLastSelection"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getObjectLastSelection...");
    if (_containsScriptText_old(scriptObject, "sim.deleteSelectedObjects"))
        App::logMsg(sim_verbosity_errors, "Contains sim.deleteSelectedObjects...");
    if (_containsScriptText_old(scriptObject, "sim.scaleSelectedObjects"))
        App::logMsg(sim_verbosity_errors, "Contains sim.scaleSelectedObjects...");
    if (_containsScriptText_old(scriptObject, "sim.copyPasteSelectedObjects"))
        App::logMsg(sim_verbosity_errors, "Contains sim.copyPasteSelectedObjects...");
    if (_containsScriptText_old(scriptObject, "sim.breakForceSensor"))
        App::logMsg(sim_verbosity_errors, "Contains sim.breakForceSensor...");

    if (_containsScriptText_old(scriptObject, "sim.fileDlg"))
        App::logMsg(sim_verbosity_errors, "Contains sim.fileDlg...");
    if (_containsScriptText_old(scriptObject, "sim.msgBox"))
        App::logMsg(sim_verbosity_errors, "Contains sim.msgBox...");
    if (_containsScriptText_old(scriptObject, "sim.displayDialog"))
        App::logMsg(sim_verbosity_errors, "Contains sim.displayDialog...");

    if (_containsScriptText_old(scriptObject, "Reflexxes"))
        App::logMsg(sim_verbosity_errors, "Contains Reflexxes...");
    if (_containsScriptText_old(scriptObject, "reflexxes"))
        App::logMsg(sim_verbosity_errors, "Contains reflexxes...");
    if (_containsScriptText_old(scriptObject, "sim.rml"))
        App::logMsg(sim_verbosity_errors, "Contains sim.rml*...");
    if (_containsScriptText_old(scriptObject, "sim.getObjectHandle(sim.handle_self)"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getObjectHandle(sim.handle_self)...");
    if (_containsScriptText_old(scriptObject, "sim.getObjectName"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getObjectName...");
    if (_containsScriptText_old(scriptObject, "sim.setObjectName"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setObjectName...");
    if (_containsScriptText_old(scriptObject, "sim.getScriptName"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getScriptName...");
    if (_containsScriptText_old(scriptObject, "sim.setScriptVariable"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setScriptVariable...");
    if (_containsScriptText_old(scriptObject, "sim.setSimilarName"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setSimilarName...");
    if (_containsScriptText_old(scriptObject, "sim.getObjectAssociatedWithScript"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getObjectAssociatedWithScript...");
    if (_containsScriptText_old(scriptObject, "sim.getScriptAssociatedWithObject"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getScriptAssociatedWithObject...");
    if (_containsScriptText_old(scriptObject, "sim.getCustomizationScriptAssociatedWithObject"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getCustomizationScriptAssociatedWithObject...");
    if (_containsScriptText_old(scriptObject, "sim.getObjectSizeValues"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getObjectSizeValues...");
    if (_containsScriptText_old(scriptObject, "sim.setObjectSizeValues"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setObjectSizeValues...");
    if (_containsScriptText_old(scriptObject, "sim.getObjectConfiguration"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getObjectConfiguration...");
    if (_containsScriptText_old(scriptObject, "sim.setObjectConfiguration"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setObjectConfiguration...");
    if (_containsScriptText_old(scriptObject, "sim.getConfigurationTree"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getConfigurationTree...");
    if (_containsScriptText_old(scriptObject, "sim.setConfigurationTree"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setConfigurationTree...");

    if (_containsScriptText_old(scriptObject, "__initFunctions"))
        App::logMsg(sim_verbosity_errors, "Contains __initFunctions...");

    if (_containsScriptText_old(scriptObject, "sim.getObjectInt32Parameter"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getObjectInt32Parameter...");
    if (_containsScriptText_old(scriptObject, "sim.getObjectIntParameter"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getObjectIntParameter...");
    if (_containsScriptText_old(scriptObject, "sim.getObjectFloatParameter"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getObjectFloatParameter...");
    if (_containsScriptText_old(scriptObject, "sim.isHandleValid"))
        App::logMsg(sim_verbosity_errors, "Contains sim.isHandleValid...");
    if (_containsScriptText_old(scriptObject, "sim.addPointCloud"))
        App::logMsg(sim_verbosity_errors, "Contains sim.addPointCloud...");

    if (_containsScriptText_old(scriptObject, "sysCall_vision") &&
        (scriptObject->_scriptType == sim_scripttype_customization))
        App::logMsg(sim_verbosity_errors, "Contains a vision callback in a customization script");
    if (_containsScriptText_old(scriptObject, "sysCall_trigger") &&
        (scriptObject->_scriptType == sim_scripttype_customization))
        App::logMsg(sim_verbosity_errors, "Contains a trigger callback in a customization script");

    if (_containsScriptText_old(scriptObject, "sim.rmlMove"))
        App::logMsg(sim_verbosity_errors, "Contains sim.rmlMove...");
    if (_containsScriptText_old(scriptObject, "sim.include"))
        App::logMsg(sim_verbosity_errors, "Contains sim.include...");
    if (_containsScriptText_old(scriptObject, "sim.getIk"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getIk...");
    if (_containsScriptText_old(scriptObject, "sim.getScriptSimulationParameter"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getScriptSimulationParameter...");
    if (_containsScriptText_old(scriptObject, "sim.setScriptSimulationParameter"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setScriptSimulationParameter...");
    if (_containsScriptText_old(scriptObject, "sim.tube"))
        App::logMsg(sim_verbosity_errors, "Contains sim.tube...");
    if (_containsScriptText_old(scriptObject, "sim.addStatusbarMessage"))
        App::logMsg(sim_verbosity_errors, "Contains sim.addStatusbarMessage...");
    if (_containsScriptText_old(scriptObject, "sim.getNameSuffix"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getNameSuffix...");
    if (_containsScriptText_old(scriptObject, "sim.setNameSuffix"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setNameSuffix...");
    if (_containsScriptText_old(scriptObject, "sim.resetMill"))
        App::logMsg(sim_verbosity_errors, "Contains sim.resetMill...");
    if (_containsScriptText_old(scriptObject, "sim.handleMill"))
        App::logMsg(sim_verbosity_errors, "Contains sim.handleMill...");
    if (_containsScriptText_old(scriptObject, "sim.resetMilling"))
        App::logMsg(sim_verbosity_errors, "Contains sim.resetMilling...");
    if (_containsScriptText_old(scriptObject, "sim.openTextEditor"))
        App::logMsg(sim_verbosity_errors, "Contains sim.openTextEditor...");
    if (_containsScriptText_old(scriptObject, "sim.closeTextEditor"))
        App::logMsg(sim_verbosity_errors, "Contains sim.closeTextEditor...");
    if (_containsScriptText_old(scriptObject, "simGetMaterialId"))
        App::logMsg(sim_verbosity_errors, "Contains simGetMaterialId...");
    if (_containsScriptText_old(scriptObject, "simGetShapeMaterial"))
        App::logMsg(sim_verbosity_errors, "Contains simGetShapeMaterial...");
    if (_containsScriptText_old(scriptObject, "simHandleVarious"))
        App::logMsg(sim_verbosity_errors, "Contains simHandleVarious...");
    if (_containsScriptText_old(scriptObject, "simGetInstanceIndex"))
        App::logMsg(sim_verbosity_errors, "Contains simGetInstanceIndex...");
    if (_containsScriptText_old(scriptObject, "simGetVisibleInstanceIndex"))
        App::logMsg(sim_verbosity_errors, "Contains simGetVisibleInstanceIndex...");
    if (_containsScriptText_old(scriptObject, "simResetPath"))
        App::logMsg(sim_verbosity_errors, "Contains simResetPath...");
    if (_containsScriptText_old(scriptObject, "simHandlePath"))
        App::logMsg(sim_verbosity_errors, "Contains simHandlePath...");
    if (_containsScriptText_old(scriptObject, "simResetJoint"))
        App::logMsg(sim_verbosity_errors, "Contains simResetJoint...");
    if (_containsScriptText_old(scriptObject, "simHandleJoint"))
        App::logMsg(sim_verbosity_errors, "Contains simHandleJoint...");
    if (_containsScriptText_old(scriptObject, "simGetInvertedMatrix"))
        App::logMsg(sim_verbosity_errors, "Contains simGetInvertedMatrix...");
    if (_containsScriptText_old(scriptObject, "simAddSceneCustomData"))
        App::logMsg(sim_verbosity_errors, "Contains simAddSceneCustomData...");
    if (_containsScriptText_old(scriptObject, "simGetSceneCustomData"))
        App::logMsg(sim_verbosity_errors, "Contains simGetSceneCustomData...");
    if (_containsScriptText_old(scriptObject, "simAddObjectCustomData"))
        App::logMsg(sim_verbosity_errors, "Contains simAddObjectCustomData...");
    if (_containsScriptText_old(scriptObject, "simGetObjectCustomData"))
        App::logMsg(sim_verbosity_errors, "Contains simGetObjectCustomData...");
    if (_containsScriptText_old(scriptObject, "sim.setVisionSensorFilter"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setVisionSensorFilter...");
    if (_containsScriptText_old(scriptObject, "sim.getVisionSensorFilter"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getVisionSensorFilter...");
    if (_containsScriptText_old(scriptObject, "sim.handleMechanism"))
        App::logMsg(sim_verbosity_errors, "Contains sim.handleMechanism...");
    if (_containsScriptText_old(scriptObject, "sim.setPathTargetNominalVelocity"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setPathTargetNominalVelocity...");
    if (_containsScriptText_old(scriptObject, "sim.setShapeMassAndInertia"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setShapeMassAndInertia...");
    if (_containsScriptText_old(scriptObject, "sim.getShapeMassAndInertia"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getShapeMassAndInertia...");
    if (_containsScriptText_old(scriptObject, "sim.checkIkGroup"))
        App::logMsg(sim_verbosity_errors, "Contains sim.checkIkGroup...");
    if (_containsScriptText_old(scriptObject, "sim.handleIkGroup"))
        App::logMsg(sim_verbosity_errors, "Contains sim.handleIkGroup...");
    if (_containsScriptText_old(scriptObject, "sim.createIkGroup"))
        App::logMsg(sim_verbosity_errors, "Contains sim.createIkGroup...");
    if (_containsScriptText_old(scriptObject, "sim.removeIkGroup"))
        App::logMsg(sim_verbosity_errors, "Contains sim.removeIkGroup...");
    if (_containsScriptText_old(scriptObject, "sim.createIkElement"))
        App::logMsg(sim_verbosity_errors, "Contains sim.createIkElement...");
    if (_containsScriptText_old(scriptObject, "sim.exportIk"))
        App::logMsg(sim_verbosity_errors, "Contains sim.exportIk...");
    if (_containsScriptText_old(scriptObject, "sim.computeJacobian"))
        App::logMsg(sim_verbosity_errors, "Contains sim.computeJacobian...");
    if (_containsScriptText_old(scriptObject, "sim.getConfigForTipPose"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getConfigForTipPose...");
    if (_containsScriptText_old(scriptObject, "sim.generateIkPath"))
        App::logMsg(sim_verbosity_errors, "Contains sim.generateIkPath...");
    if (_containsScriptText_old(scriptObject, "sim.getIkGroupHandle"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getIkGroupHandle...");
    if (_containsScriptText_old(scriptObject, "sim.getIkGroupMatrix"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getIkGroupMatrix...");
    if (_containsScriptText_old(scriptObject, "sim.setIkGroupProperties"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setIkGroupProperties...");
    if (_containsScriptText_old(scriptObject, "sim.setIkElementProperties"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setIkElementProperties...");
    if (_containsScriptText_old(scriptObject, "sim.setThreadIsFree"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setThreadIsFree...");
    if (_containsScriptText_old(scriptObject, "simSetUIPosition"))
        App::logMsg(sim_verbosity_errors, "Contains simSetUIPosition...");
    if (_containsScriptText_old(scriptObject, "simGetUIPosition"))
        App::logMsg(sim_verbosity_errors, "Contains simGetUIPosition...");
    if (_containsScriptText_old(scriptObject, "simGetUIHandle"))
        App::logMsg(sim_verbosity_errors, "Contains simGetUIHandle...");
    if (_containsScriptText_old(scriptObject, "simGetUIProperty"))
        App::logMsg(sim_verbosity_errors, "Contains simGetUIProperty...");
    if (_containsScriptText_old(scriptObject, "simSetUIProperty"))
        App::logMsg(sim_verbosity_errors, "Contains simSetUIProperty...");
    if (_containsScriptText_old(scriptObject, "simGetUIEventButton"))
        App::logMsg(sim_verbosity_errors, "Contains simGetUIEventButton...");
    if (_containsScriptText_old(scriptObject, "simGetUIButtonProperty"))
        App::logMsg(sim_verbosity_errors, "Contains simGetUIButtonProperty...");
    if (_containsScriptText_old(scriptObject, "simSetUIButtonProperty"))
        App::logMsg(sim_verbosity_errors, "Contains simSetUIButtonProperty...");
    if (_containsScriptText_old(scriptObject, "simGetUIButtonSize"))
        App::logMsg(sim_verbosity_errors, "Contains simGetUIButtonSize...");
    if (_containsScriptText_old(scriptObject, "simSetUIButtonLabel"))
        App::logMsg(sim_verbosity_errors, "Contains simSetUIButtonLabel...");
    if (_containsScriptText_old(scriptObject, "simGetUIButtonLabel"))
        App::logMsg(sim_verbosity_errors, "Contains simGetUIButtonLabel...");
    if (_containsScriptText_old(scriptObject, "simSetUISlider"))
        App::logMsg(sim_verbosity_errors, "Contains simSetUISlider...");
    if (_containsScriptText_old(scriptObject, "simGetUISlider"))
        App::logMsg(sim_verbosity_errors, "Contains simGetUISlider...");
    if (_containsScriptText_old(scriptObject, "simCreateUIButtonArray"))
        App::logMsg(sim_verbosity_errors, "Contains simCreateUIButtonArray...");
    if (_containsScriptText_old(scriptObject, "simSetUIButtonArrayColor"))
        App::logMsg(sim_verbosity_errors, "Contains simSetUIButtonArrayColor...");
    if (_containsScriptText_old(scriptObject, "simDeleteUIButtonArray"))
        App::logMsg(sim_verbosity_errors, "Contains simDeleteUIButtonArray...");
    if (_containsScriptText_old(scriptObject, "simCreateUI"))
        App::logMsg(sim_verbosity_errors, "Contains simCreateUI...");
    if (_containsScriptText_old(scriptObject, "simCreateUIButton"))
        App::logMsg(sim_verbosity_errors, "Contains simCreateUIButton...");
    if (_containsScriptText_old(scriptObject, "simLoadUI"))
        App::logMsg(sim_verbosity_errors, "Contains simLoadUI...");
    if (_containsScriptText_old(scriptObject, "simSaveUI"))
        App::logMsg(sim_verbosity_errors, "Contains simSaveUI...");
    if (_containsScriptText_old(scriptObject, "simRemoveUI"))
        App::logMsg(sim_verbosity_errors, "Contains simRemoveUI...");
    if (_containsScriptText_old(scriptObject, "simSetUIButtonColor"))
        App::logMsg(sim_verbosity_errors, "Contains simSetUIButtonColor...");
    if (_containsScriptText_old(scriptObject, "simHandleChildScript"))
        App::logMsg(sim_verbosity_errors, "Contains simHandleChildScript...");
    if (_containsScriptText_old(scriptObject, "simSearchPath"))
        App::logMsg(sim_verbosity_errors, "Contains simSearchPath...");
    if (_containsScriptText_old(scriptObject, "simInitializePathSearch"))
        App::logMsg(sim_verbosity_errors, "Contains simInitializePathSearch...");
    if (_containsScriptText_old(scriptObject, "simPerformPathSearchStep"))
        App::logMsg(sim_verbosity_errors, "Contains simPerformPathSearchStep...");
    if (_containsScriptText_old(scriptObject, "sim.sendData"))
        App::logMsg(sim_verbosity_errors, "Contains sim.sendData...");
    if (_containsScriptText_old(scriptObject, "sim.receiveData"))
        App::logMsg(sim_verbosity_errors, "Contains sim.receiveData...");
    if (_containsScriptText_old(scriptObject, "simSerialPortOpen"))
        App::logMsg(sim_verbosity_errors, "Contains simSerialPortOpen...");
    if (_containsScriptText_old(scriptObject, "simSerialPortClose"))
        App::logMsg(sim_verbosity_errors, "Contains simSerialPortClose...");
    if (_containsScriptText_old(scriptObject, "simSerialPortSend"))
        App::logMsg(sim_verbosity_errors, "Contains simSerialPortSend...");
    if (_containsScriptText_old(scriptObject, "simSerialPortRead"))
        App::logMsg(sim_verbosity_errors, "Contains simSerialPortRead...");
    if (_containsScriptText_old(scriptObject, "sim.rmlMoveToJointPositions"))
        App::logMsg(sim_verbosity_errors, "Contains sim.rmlMoveToJointPositions...");
    if (_containsScriptText_old(scriptObject, "simRMLMoveToJointPositions"))
        App::logMsg(sim_verbosity_errors, "Contains simRMLMoveToJointPositions...");
    if (_containsScriptText_old(scriptObject, "sim.rmlMoveToPosition"))
        App::logMsg(sim_verbosity_errors, "Contains sim.rmlMoveToPosition...");
    if (_containsScriptText_old(scriptObject, "simRMLMoveToPosition"))
        App::logMsg(sim_verbosity_errors, "Contains simRMLMoveToPosition...");

    if (_containsScriptText_old(scriptObject, "sim.getCollectionHandle"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getCollectionHandle...");
    if (_containsScriptText_old(scriptObject, "sim.addCollection"))
        App::logMsg(sim_verbosity_errors, "Contains sim.addCollection...");
    if (_containsScriptText_old(scriptObject, "sim.addObjectToCollection"))
        App::logMsg(sim_verbosity_errors, "Contains sim.addObjectToCollection...");
    if (_containsScriptText_old(scriptObject, "sim.emptyCollection"))
        App::logMsg(sim_verbosity_errors, "Contains sim.emptyCollection...");
    if (_containsScriptText_old(scriptObject, "sim.removeCollection"))
        App::logMsg(sim_verbosity_errors, "Contains sim.removeCollection...");
    if (_containsScriptText_old(scriptObject, "sim.getCollectionName"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getCollectionName...");
    if (_containsScriptText_old(scriptObject, "sim.setCollectionName"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setCollectionName...");
    if (_containsScriptText_old(scriptObject, "sim.getCollisionHandle"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getCollisionHandle...");
    if (_containsScriptText_old(scriptObject, "sim.handleCollision"))
        App::logMsg(sim_verbosity_errors, "Contains sim.handleCollision...");
    if (_containsScriptText_old(scriptObject, "sim.readCollision"))
        App::logMsg(sim_verbosity_errors, "Contains sim.readCollision...");
    if (_containsScriptText_old(scriptObject, "sim.resetCollision"))
        App::logMsg(sim_verbosity_errors, "Contains sim.resetCollision...");
    if (_containsScriptText_old(scriptObject, "sim.getDistanceHandle"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getDistanceHandle...");
    if (_containsScriptText_old(scriptObject, "sim.handleDistance"))
        App::logMsg(sim_verbosity_errors, "Contains sim.handleDistance...");
    if (_containsScriptText_old(scriptObject, "sim.readDistance"))
        App::logMsg(sim_verbosity_errors, "Contains sim.readDistance...");
    if (_containsScriptText_old(scriptObject, "sim.resetDistance"))
        App::logMsg(sim_verbosity_errors, "Contains sim.resetDistance...");
    if (_containsScriptText_old(scriptObject, "sim.boolAnd32"))
        App::logMsg(sim_verbosity_errors, "Contains sim.boolAnd32...");
    if (_containsScriptText_old(scriptObject, "sim.boolOr32"))
        App::logMsg(sim_verbosity_errors, "Contains sim.boolOr32...");
    if (_containsScriptText_old(scriptObject, "sim.boolXor32"))
        App::logMsg(sim_verbosity_errors, "Contains sim.boolXor32...");
    if (_containsScriptText_old(scriptObject, "simBoolAnd32"))
        App::logMsg(sim_verbosity_errors, "Contains simBoolAnd32...");
    if (_containsScriptText_old(scriptObject, "simBoolOr32"))
        App::logMsg(sim_verbosity_errors, "Contains simBoolOr32...");
    if (_containsScriptText_old(scriptObject, "simBoolXor32"))
        App::logMsg(sim_verbosity_errors, "Contains simBoolXo32...");
    if (_containsScriptText_old(scriptObject, "simBoolAnd16"))
        App::logMsg(sim_verbosity_errors, "Contains simBoolAnd16...");
    if (_containsScriptText_old(scriptObject, "simBoolOr16"))
        App::logMsg(sim_verbosity_errors, "Contains simBoolOr16...");
    if (_containsScriptText_old(scriptObject, "simBoolXor16"))
        App::logMsg(sim_verbosity_errors, "Contains simBoolXo16...");

    if (_containsScriptText_old(scriptObject, "sim.getScriptExecutionCount"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getScriptExecutionCount...");
    if (_containsScriptText_old(scriptObject, "sim.isScriptRunningInThread"))
        App::logMsg(sim_verbosity_errors, "Contains sim.isScriptRunningInThread...");
    if (_containsScriptText_old(scriptObject, "sim.isScriptExecutionThreaded"))
        App::logMsg(sim_verbosity_errors, "Contains sim.isScriptExecutionThreaded...");
    if (_containsScriptText_old(scriptObject, "sim.setThreadResumeLocation"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setThreadResumeLocation...");
    if (_containsScriptText_old(scriptObject, "sim.resumeThreads"))
        App::logMsg(sim_verbosity_errors, "Contains sim.resumeThreads...");
    if (_containsScriptText_old(scriptObject, "sim.launchThreadedChildScripts"))
        App::logMsg(sim_verbosity_errors, "Contains sim.launchThreadedChildScripts...");
    if (_containsScriptText_old(scriptObject, "simGetScriptExecutionCount"))
        App::logMsg(sim_verbosity_errors, "Contains simGetScriptExecutionCount...");
    if (_containsScriptText_old(scriptObject, "simIsScriptExecutionThreaded"))
        App::logMsg(sim_verbosity_errors, "Contains simIsScriptExecutionThreaded...");
    if (_containsScriptText_old(scriptObject, "simIsScriptRunningInThread"))
        App::logMsg(sim_verbosity_errors, "Contains simIsScriptRunningInThread...");
    if (_containsScriptText_old(scriptObject, "simSetThreadResumeLocation"))
        App::logMsg(sim_verbosity_errors, "Contains simSetThreadResumeLocation...");
    if (_containsScriptText_old(scriptObject, "sim.setJointForce"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setJointForce...");
    if (_containsScriptText_old(scriptObject, "simResumeThreads"))
        App::logMsg(sim_verbosity_errors, "Contains simResumeThreads...");
    if (_containsScriptText_old(scriptObject, "simLaunchThreadedChildScripts"))
        App::logMsg(sim_verbosity_errors, "Contains simLaunchThreadedChildScripts...");
    if (_containsScriptText_old(scriptObject, "sim.copyMatrix"))
        App::logMsg(sim_verbosity_errors, "Contains sim.copyMatrix...");
    if (_containsScriptText_old(scriptObject, "sim.getPathPosition"))
        App::logMsg(sim_verbosity_errors, "Contains sim.getPathPosition...");
    if (_containsScriptText_old(scriptObject, "sim.setPathPosition"))
        App::logMsg(sim_verbosity_errors, "Contains sim.setPathPosition...");

    if (_containsScriptText_old(scriptObject, "'utils'"))
        App::logMsg(sim_verbosity_errors, "Contains 'utils'...");

    //************************************************************
    // Scripts containing following should remain handled in threaded mode:
    if (_containsScriptText_old(scriptObject, "simMoveToPosition"))
        App::logMsg(sim_verbosity_errors, "Contains simMoveToPosition...");
    if (_containsScriptText_old(scriptObject, "sim.moveToPosition"))
        App::logMsg(sim_verbosity_errors, "Contains sim.moveToPosition...");
    if (_containsScriptText_old(scriptObject, "simMoveToObject"))
        App::logMsg(sim_verbosity_errors, "Contains simMoveToObject...");
    if (_containsScriptText_old(scriptObject, "sim.moveToObject"))
        App::logMsg(sim_verbosity_errors, "Contains sim.moveToObject...");
    if (_containsScriptText_old(scriptObject, "simFollowPath"))
        App::logMsg(sim_verbosity_errors, "Contains simFollowPath...");
    if (_containsScriptText_old(scriptObject, "sim.followPath"))
        App::logMsg(sim_verbosity_errors, "Contains sim.followPath...");
    if (_containsScriptText_old(scriptObject, "simMoveToJointPositions"))
        App::logMsg(sim_verbosity_errors, "Contains simMoveToJointPositions...");
    if (_containsScriptText_old(scriptObject, "sim.moveToJointPositions"))
        App::logMsg(sim_verbosity_errors, "Contains sim.moveToJointPositions...");
    //************************************************************
}
// **************************************************************
// **************************************************************

// Old
// **************************************************************
// **************************************************************
std::string CScriptObject::_replaceOldApi(const char* txt, bool forwardAdjustment)
{ // recursive
    size_t p = std::string(txt).find("sim");
    if (p != std::string::npos)
    {
        std::string beforePart;
        std::string apiWord;
        std::string afterPart;
        _splitApiText_old(txt, p, beforePart, apiWord, afterPart);
        std::map<std::string, std::string>::iterator it = _newApiMap_old.find(apiWord);
        if (it != _newApiMap_old.end())
        {
            apiWord = it->second;
            // Do a second stage replacement:
            std::map<std::string, std::string>::iterator it2 = _newApiMap_old.find(apiWord);
            if (it2 != _newApiMap_old.end())
                apiWord = it2->second;
        }
        return (beforePart + apiWord + _replaceOldApi(afterPart.c_str(), forwardAdjustment));
    }
    return (std::string(txt));
}
void CScriptObject::_performNewApiAdjustments_old(CScriptObject* scriptObject, bool forwardAdjustment)
{
    std::vector<const SNewApiMapping*> all;
    if (_newApiMap_old.begin() == _newApiMap_old.end())
        all.push_back(_simApiMapping);
    all.push_back(_simBubbleApiMapping);
    all.push_back(_simK3ApiMapping);
    all.push_back(_simMTBApiMapping);
    all.push_back(_simOpenMeshApiMapping);
    all.push_back(_simSkeletonApiMapping);
    all.push_back(_simQHullApiMapping);
    all.push_back(_simRemoteApiApiMapping);
    all.push_back(_simRRS1ApiMapping);
    all.push_back(_simVisionApiMapping);
    all.push_back(_simCamApiMapping);
    all.push_back(_simJoyApiMapping);
    all.push_back(_simWiiApiMapping);
    all.push_back(_simURDFApiMapping);
    all.push_back(_simBWFApiMapping);
    all.push_back(_simUIApiMapping);
    all.push_back(_simROSApiMapping);
    all.push_back(_simICPApiMapping);
    all.push_back(_simOMPLApiMapping);
    all.push_back(_simSDFApiMapping);
    all.push_back(_simSurfRecApiMapping);
    all.push_back(_simxApiMapping);

    for (size_t j = 0; j < all.size(); j++)
    {
        const SNewApiMapping* aMapping = all[j];
        for (int i = 0; aMapping[i].oldApi != ""; i++)
        {
            if (forwardAdjustment)
                _newApiMap_old[aMapping[i].oldApi] = aMapping[i].newApi;
            else
                _newApiMap_old[aMapping[i].newApi] = aMapping[i].oldApi;
        }
    }

    std::string theScript(scriptObject->getScriptText());
    theScript = _replaceOldApi(theScript.c_str(), forwardAdjustment);
    scriptObject->setScriptText(theScript.c_str());
}

std::map<std::string, std::string> CScriptObject::_newApiMap_old;
const SNewApiMapping _simApiMapping[] = {
    "sim.mainscriptcall_initialization",
    "sim.syscb_init",
    "sim.mainscriptcall_cleanup",
    "sim.syscb_cleanup",
    "sim.mainscriptcall_regular",
    "sim.syscb_regular",
    "sim.childscriptcall_initialization",
    "sim.syscb_init",
    "sim.childscriptcall_cleanup",
    "sim.syscb_cleanup",
    "sim.childscriptcall_actuation",
    "sim.syscb_actuation",
    "sim.childscriptcall_sensing",
    "sim.syscb_sensing",
    "sim.customizationscriptcall_initialization",
    "sim.syscb_init",
    "sim.customizationscriptcall_cleanup",
    "sim.syscb_cleanup",
    "sim.customizationscriptcall_nonsimulation",
    "sim.syscb_nonsimulation",
    "sim.customizationscriptcall_lastbeforesimulation",
    "sim.syscb_beforesimulation",
    "sim.customizationscriptcall_firstaftersimulation",
    "sim.syscb_aftersimulation",
    "sim.customizationscriptcall_simulationactuation",
    "sim.syscb_actuation",
    "sim.customizationscriptcall_simulationsensing",
    "sim.syscb_sensing",
    "sim.customizationscriptcall_simulationpause",
    "sim.syscb_suspended",
    "sim.customizationscriptcall_simulationpausefirst",
    "sim.syscb_suspend",
    "sim.customizationscriptcall_simulationpauselast",
    "sim.syscb_resume",
    "sim.customizationscriptcall_lastbeforeinstanceswitch",
    "sim.syscb_beforeinstanceswitch",
    "sim.customizationscriptcall_firstafterinstanceswitch",
    "sim.syscb_afterinstanceswitch",
    "sim.customizationscriptcall_beforecopy",
    "sim.syscb_beforecopy",
    "sim.customizationscriptcall_aftercopy",
    "sim.syscb_aftercopy",
    "sim.customizationscriptcall_br",
    "sim.syscb_br",
    "simGetScriptName",
    "sim.getScriptName",
    "simGetObjectAssociatedWithScript",
    "sim.getObjectAssociatedWithScript",
    "simGetScriptAssociatedWithObject",
    "sim.getScriptAssociatedWithObject",
    "simGetCustomizationScriptAssociatedWithObject",
    "sim.getCustomizationScriptAssociatedWithObject",
    "simGetScriptExecutionCount",
    "sim.getScriptExecutionCount",
    "simIsScriptExecutionThreaded",
    "sim.isScriptExecutionThreaded",
    "simIsScriptRunningInThread",
    "sim.isScriptRunningInThread",
    "simOpenModule",
    "sim.openModule",
    "simCloseModule",
    "sim.closeModule",
    "simHandleModule",
    "sim.handleModule",
    "simBoolOr32",
    "sim.boolOr32",
    "simBoolAnd32",
    "sim.boolAnd32",
    "simBoolXor32",
    "sim.boolXor32",
    "simHandleDynamics",
    "sim.handleDynamics",
    "simHandleIkGroup",
    "sim.handleIkGroup",
    "simCheckIkGroup",
    "sim.checkIkGroup",
    "simHandleCollision",
    "sim.handleCollision",
    "simReadCollision",
    "sim.readCollision",
    "simHandleDistance",
    "sim.handleDistance",
    "simReadDistance",
    "sim.readDistance",
    "simHandleProximitySensor",
    "sim.handleProximitySensor",
    "simReadProximitySensor",
    "sim.readProximitySensor",
    "simHandleMill",
    "sim.handleMill",
    "simResetCollision",
    "sim.resetCollision",
    "simResetDistance",
    "sim.resetDistance",
    "simResetProximitySensor",
    "sim.resetProximitySensor",
    "simResetMill",
    "sim.resetMill",
    "simCheckProximitySensor",
    "sim.checkProximitySensor",
    "simCheckProximitySensorEx",
    "sim.checkProximitySensorEx",
    "simCheckProximitySensorEx2",
    "sim.checkProximitySensorEx2",
    "simGetNameSuffix",
    "sim.getNameSuffix",
    "simSetNameSuffix",
    "sim.setNameSuffix",
    "simGetObjectHandle",
    "sim.getObjectHandle",
    "simAddScript",
    "sim.addScript",
    "simAssociateScriptWithObject",
    "sim.associateScriptWithObject",
    "simSetScriptText",
    "sim.setScriptText",
    "simGetScriptHandle",
    "sim.getScriptHandle",
    "simGetCollectionHandle",
    "sim.getCollectionHandle",
    "simRemoveCollection",
    "sim.removeCollection",
    "simEmptyCollection",
    "sim.emptyCollection",
    "simGetObjectPosition",
    "sim.getObjectPosition",
    "simGetObjectOrientation",
    "sim.getObjectOrientation",
    "simSetObjectPosition",
    "sim.setObjectPosition",
    "simSetObjectOrientation",
    "sim.setObjectOrientation",
    "simGetJointPosition",
    "sim.getJointPosition",
    "simSetJointPosition",
    "sim.setJointPosition",
    "simSetJointTargetPosition",
    "sim.setJointTargetPosition",
    "simGetJointTargetPosition",
    "sim.getJointTargetPosition",
    "simSetJointForce",
    "sim.setJointForce",
    "simGetPathPosition",
    "sim.getPathPosition",
    "simSetPathPosition",
    "sim.setPathPosition",
    "simGetPathLength",
    "sim.getPathLength",
    "simSetJointTargetVelocity",
    "sim.setJointTargetVelocity",
    "simGetJointTargetVelocity",
    "sim.getJointTargetVelocity",
    "simSetPathTargetNominalVelocity",
    "sim.setPathTargetNominalVelocity",
    "simGetObjectName",
    "sim.getObjectName",
    "simGetCollectionName",
    "sim.getCollectionName",
    "simRemoveObject",
    "sim.removeObject",
    "simRemoveModel",
    "sim.removeModel",
    "simGetSimulationTime",
    "sim.getSimulationTime",
    "simGetSimulationState",
    "sim.getSimulationState",
    "simGetSystemTime",
    "sim.getSystemTime",
    "simGetSystemTimeInMs",
    "sim.getSystemTimeInMs",
    "simCheckCollision",
    "sim.checkCollision",
    "simCheckCollisionEx",
    "sim.checkCollisionEx",
    "simCheckDistance",
    "sim.checkDistance",
    "simGetObjectConfiguration",
    "sim.getObjectConfiguration",
    "simSetObjectConfiguration",
    "sim.setObjectConfiguration",
    "simGetConfigurationTree",
    "sim.getConfigurationTree",
    "simSetConfigurationTree",
    "sim.setConfigurationTree",
    "simHandleMechanism",
    "sim.handleMechanism",
    "simGetSimulationTimeStep",
    "sim.getSimulationTimeStep",
    "simGetSimulatorMessage",
    "sim.getSimulatorMessage",
    "simResetGraph",
    "sim.resetGraph",
    "simHandleGraph",
    "sim.handleGraph",
    "simAddStatusbarMessage",
    "sim.addStatusbarMessage",
    "simGetLastError",
    "sim.getLastError",
    "simGetObjects",
    "sim.getObjects",
    "simRefreshDialogs",
    "sim.refreshDialogs",
    "simGetModuleName",
    "sim.getModuleName",
    "simGetIkGroupHandle",
    "sim.getIkGroupHandle",
    "simRemoveScript",
    "sim.removeScript",
    "simGetCollisionHandle",
    "sim.getCollisionHandle",
    "simGetDistanceHandle",
    "sim.getDistanceHandle",
    "simGetMechanismHandle",
    "sim.getMechanismHandle",
    "simGetScriptSimulationParameter",
    "sim.getScriptSimulationParameter",
    "simSetScriptSimulationParameter",
    "sim.setScriptSimulationParameter",
    "simDisplayDialog",
    "sim.displayDialog",
    "simGetDialogResult",
    "sim.getDialogResult",
    "simGetDialogInput",
    "sim.getDialogInput",
    "simEndDialog",
    "sim.endDialog",
    "simStopSimulation",
    "sim.stopSimulation",
    "simPauseSimulation",
    "sim.pauseSimulation",
    "simStartSimulation",
    "sim.startSimulation",
    "simGetObjectMatrix",
    "sim.getObjectMatrix",
    "simSetObjectMatrix",
    "sim.setObjectMatrix",
    "simGetJointMatrix",
    "sim.getJointMatrix",
    "simSetSphericalJointMatrix",
    "sim.setSphericalJointMatrix",
    "simBuildIdentityMatrix",
    "sim.buildIdentityMatrix",
    "simCopyMatrix",
    "sim.copyMatrix",
    "simBuildMatrix",
    "sim.buildMatrix",
    "simGetEulerAnglesFromMatrix",
    "sim.getEulerAnglesFromMatrix",
    "simInvertMatrix",
    "sim.invertMatrix",
    "simMultiplyMatrices",
    "sim.multiplyMatrices",
    "simInterpolateMatrices",
    "sim.interpolateMatrices",
    "simMultiplyVector",
    "sim.multiplyVector",
    "simGetObjectChild",
    "sim.getObjectChild",
    "simGetObjectParent",
    "sim.getObjectParent",
    "simSetObjectParent",
    "sim.setObjectParent",
    "simGetObjectType",
    "sim.getObjectType",
    "simGetJointType",
    "sim.getJointType",
    "simSetBoolParameter",
    "sim.setBoolParameter",
    "simGetBoolParameter",
    "sim.getBoolParameter",
    "simSetInt32Parameter",
    "sim.setInt32Parameter",
    "simGetInt32Parameter",
    "sim.getInt32Parameter",
    "simSetFloatParameter",
    "sim.setFloatParameter",
    "simGetFloatParameter",
    "sim.getFloatParameter",
    "simSetStringParameter",
    "sim.setStringParameter",
    "simGetStringParameter",
    "sim.getStringParameter",
    "simSetArrayParameter",
    "sim.setArrayParameter",
    "simGetArrayParameter",
    "sim.getArrayParameter",
    "simSetObjectName",
    "sim.setObjectName",
    "simSetCollectionName",
    "sim.setCollectionName",
    "simGetJointInterval",
    "sim.getJointInterval",
    "simSetJointInterval",
    "sim.setJointInterval",
    "simLoadScene",
    "sim.loadScene",
    "simSaveScene",
    "sim.saveScene",
    "simLoadModel",
    "sim.loadModel",
    "simSaveModel",
    "sim.saveModel",
    "simIsObjectInSelection",
    "sim.isObjectInSelection",
    "simAddObjectToSelection",
    "sim.addObjectToSelection",
    "simRemoveObjectFromSelection",
    "sim.removeObjectFromSelection",
    "simGetObjectSelection",
    "sim.getObjectSelection",
    "simGetRealTimeSimulation",
    "sim.getIsRealTimeSimulation",
    "simSetNavigationMode",
    "sim.setNavigationMode",
    "simGetNavigationMode",
    "sim.getNavigationMode",
    "simSetPage",
    "sim.setPage",
    "simGetPage",
    "sim.getPage",
    "simCopyPasteObjects",
    "sim.copyPasteObjects",
    "simScaleObjects",
    "sim.scaleObjects",
    "simGetObjectUniqueIdentifier",
    "sim.getObjectUniqueIdentifier",
    "simSetThreadAutomaticSwitch",
    "sim.setThreadAutomaticSwitch",
    "simGetThreadAutomaticSwitch",
    "sim.getThreadAutomaticSwitch",
    "simSetThreadSwitchTiming",
    "sim.setThreadSwitchTiming",
    "simSetThreadResumeLocation",
    "sim.setThreadResumeLocation",
    "simResumeThreads",
    "sim.resumeThreads",
    "simSwitchThread",
    "sim.switchThread",
    "simCreateIkGroup",
    "sim.createIkGroup",
    "simRemoveIkGroup",
    "sim.removeIkGroup",
    "simCreateIkElement",
    "sim.createIkElement",
    "simCreateCollection",
    "sim.createCollection",
    "simAddObjectToCollection",
    "sim.addObjectToCollection",
    "simSaveImage",
    "sim.saveImage",
    "simLoadImage",
    "sim.loadImage",
    "simGetScaledImage",
    "sim.getScaledImage",
    "simTransformImage",
    "sim.transformImage",
    "simGetQHull",
    "sim.getQHull",
    "simGetDecimatedMesh",
    "sim.getDecimatedMesh",
    "simExportIk",
    "sim.exportIk",
    "simComputeJacobian",
    "sim.computeJacobian",
    "simSendData",
    "sim.sendData",
    "simReceiveData",
    "sim.receiveData",
    "simPackInt32Table",
    "sim.packInt32Table",
    "simPackUInt32Table",
    "sim.packUInt32Table",
    "simPackFloatTable",
    "sim.packFloatTable",
    "simPackDoubleTable",
    "sim.packDoubleTable",
    "simPackUInt8Table",
    "sim.packUInt8Table",
    "simPackUInt16Table",
    "sim.packUInt16Table",
    "simUnpackInt32Table",
    "sim.unpackInt32Table",
    "simUnpackUInt32Table",
    "sim.unpackUInt32Table",
    "simUnpackFloatTable",
    "sim.unpackFloatTable",
    "simUnpackDoubleTable",
    "sim.unpackDoubleTable",
    "simUnpackUInt8Table",
    "sim.unpackUInt8Table",
    "simUnpackUInt16Table",
    "sim.unpackUInt16Table",
    "simPackTable",
    "sim.packTable",
    "simUnpackTable",
    "sim.unpackTable",
    "simTransformBuffer",
    "sim.transformBuffer",
    "simCombineRgbImages",
    "sim.combineRgbImages",
    "simGetVelocity",
    "sim.getVelocity",
    "simGetObjectVelocity",
    "sim.getObjectVelocity",
    "simAddForceAndTorque",
    "sim.addForceAndTorque",
    "simAddForce",
    "sim.addForce",
    "simSetExplicitHandling",
    "sim.setExplicitHandling",
    "simGetExplicitHandling",
    "sim.getExplicitHandling",
    "simSetGraphUserData",
    "sim.setGraphUserData",
    "simAddDrawingObject",
    "sim.addDrawingObject",
    "simRemoveDrawingObject",
    "sim.removeDrawingObject",
    "simAddDrawingObjectItem",
    "sim.addDrawingObjectItem",
    "simAddParticleObject",
    "sim.addParticleObject",
    "simRemoveParticleObject",
    "sim.removeParticleObject",
    "simAddParticleObjectItem",
    "sim.addParticleObjectItem",
    "simGetObjectSizeFactor",
    "sim.getObjectSizeFactor",
    "simSetIntegerSignal",
    "sim.setIntegerSignal",
    "simGetIntegerSignal",
    "sim.getIntegerSignal",
    "simClearIntegerSignal",
    "sim.clearIntegerSignal",
    "simSetFloatSignal",
    "sim.setFloatSignal",
    "simGetFloatSignal",
    "sim.getFloatSignal",
    "simClearFloatSignal",
    "sim.clearFloatSignal",
    "simSetStringSignal",
    "sim.setStringSignal",
    "simGetStringSignal",
    "sim.getStringSignal",
    "simClearStringSignal",
    "sim.clearStringSignal",
    "simGetSignalName",
    "sim.getSignalName",
    "simWaitForSignal",
    "sim.waitForSignal",
    "simPersistentDataWrite",
    "sim.persistentDataWrite",
    "simPersistentDataRead",
    "sim.persistentDataRead",
    "simSetObjectProperty",
    "sim.setObjectProperty",
    "simGetObjectProperty",
    "sim.getObjectProperty",
    "simSetObjectSpecialProperty",
    "sim.setObjectSpecialProperty",
    "simGetObjectSpecialProperty",
    "sim.getObjectSpecialProperty",
    "simSetModelProperty",
    "sim.setModelProperty",
    "simGetModelProperty",
    "sim.getModelProperty",
    "simMoveToPosition",
    "sim.moveToPosition",
    "simMoveToObject",
    "sim.moveToObject",
    "simFollowPath",
    "sim.followPath",
    "simMoveToJointPositions",
    "sim.moveToJointPositions",
    "simWait",
    "sim.wait",
    "simGetDataOnPath",
    "sim.getDataOnPath",
    "simGetPositionOnPath",
    "sim.getPositionOnPath",
    "simGetOrientationOnPath",
    "sim.getOrientationOnPath",
    "simGetClosestPositionOnPath",
    "sim.getClosestPositionOnPath",
    "simReadForceSensor",
    "sim.readForceSensor",
    "simBreakForceSensor",
    "sim.breakForceSensor",
    "simGetLightParameters",
    "sim.getLightParameters",
    "simSetLightParameters",
    "sim.setLightParameters",
    "simGetLinkDummy",
    "sim.getLinkDummy",
    "simSetLinkDummy",
    "sim.setLinkDummy",
    "simSetShapeColor",
    "sim.setShapeColor",
    "simGetShapeColor",
    "sim.getShapeColor",
    "simResetDynamicObject",
    "sim.resetDynamicObject",
    "simSetJointMode",
    "sim.setJointMode",
    "simGetJointMode",
    "sim.getJointMode",
    "simSerialOpen",
    "sim.serialOpen",
    "simSerialClose",
    "sim.serialClose",
    "simSerialSend",
    "sim.serialSend",
    "simSerialRead",
    "sim.serialRead",
    "simSerialCheck",
    "sim.serialCheck",
    "simGetContactInfo",
    "sim.getContactInfo",
    "simSetThreadIsFree",
    "sim.setThreadIsFree",
    "simTubeOpen",
    "sim.tubeOpen",
    "simTubeClose",
    "sim.tubeClose",
    "simTubeWrite",
    "sim.tubeWrite",
    "simTubeRead",
    "sim.tubeRead",
    "simTubeStatus",
    "sim.tubeStatus",
    "simAuxiliaryConsoleOpen",
    "sim.auxiliaryConsoleOpen",
    "simAuxiliaryConsoleClose",
    "sim.auxiliaryConsoleClose",
    "simAuxiliaryConsolePrint",
    "sim.auxiliaryConsolePrint",
    "simAuxiliaryConsoleShow",
    "sim.auxiliaryConsoleShow",
    "simImportShape",
    "sim.importShape",
    "simImportMesh",
    "sim.importMesh",
    "simExportMesh",
    "sim.exportMesh",
    "simCreateMeshShape",
    "sim.createMeshShape",
    "simGetShapeMesh",
    "sim.getShapeMesh",
    "simCreatePureShape",
    "sim.createPureShape",
    "simCreateHeightfieldShape",
    "sim.createHeightfieldShape",
    "simAddBanner",
    "sim.addBanner",
    "simRemoveBanner",
    "sim.removeBanner",
    "simCreateJoint",
    "sim.createJoint",
    "simCreateDummy",
    "sim.createDummy",
    "simCreateProximitySensor",
    "sim.createProximitySensor",
    "simCreatePath",
    "sim.createPath",
    "simCreateForceSensor",
    "sim.createForceSensor",
    "simCreateVisionSensor",
    "sim.createVisionSensor",
    "simInsertPathCtrlPoints",
    "sim.insertPathCtrlPoints",
    "simCutPathCtrlPoints",
    "sim.cutPathCtrlPoints",
    "simGetIkGroupMatrix",
    "sim.getIkGroupMatrix",
    "simFloatingViewAdd",
    "sim.floatingViewAdd",
    "simFloatingViewRemove",
    "sim.floatingViewRemove",
    "simAdjustView",
    "sim.adjustView",
    "simCameraFitToView",
    "sim.cameraFitToView",
    "simAnnounceSceneContentChange",
    "sim.announceSceneContentChange",
    "simGetObjectInt32Parameter",
    "sim.getObjectInt32Parameter",
    "simSetObjectInt32Parameter",
    "sim.setObjectInt32Parameter",
    "simGetObjectFloatParameter",
    "sim.getObjectFloatParameter",
    "simSetObjectFloatParameter",
    "sim.setObjectFloatParameter",
    "simGetObjectStringParameter",
    "sim.getObjectStringParameter",
    "simSetObjectStringParameter",
    "sim.setObjectStringParameter",
    "simGetRotationAxis",
    "sim.getRotationAxis",
    "simRotateAroundAxis",
    "sim.rotateAroundAxis",
    "simLaunchExecutable",
    "sim.launchExecutable",
    "simGetJointForce",
    "sim.getJointForce",
    "simSetIkGroupProperties",
    "sim.setIkGroupProperties",
    "simSetIkElementProperties",
    "sim.setIkElementProperties",
    "simIsHandleValid",
    "sim.isHandleValid",
    "simGetObjectQuaternion",
    "sim.getObjectQuaternion",
    "simSetObjectQuaternion",
    "sim.setObjectQuaternion",
    "simSetShapeMassAndInertia",
    "sim.setShapeMassAndInertia",
    "simGetShapeMassAndInertia",
    "sim.getShapeMassAndInertia",
    "simGroupShapes",
    "sim.groupShapes",
    "simUngroupShape",
    "sim.ungroupShape",
    "simAddGhost",
    "sim.addGhost",
    "simModifyGhost",
    "sim.modifyGhost",
    "simQuitSimulator",
    "sim.quitSimulator",
    "simGetThreadId",
    "sim.getThreadId",
    "simSetShapeMaterial",
    "sim.setShapeMaterial",
    "simGetTextureId",
    "sim.getTextureId",
    "simReadTexture",
    "sim.readTexture",
    "simWriteTexture",
    "sim.writeTexture",
    "simCreateTexture",
    "sim.createTexture",
    "simWriteCustomDataBlock",
    "sim.writeCustomDataBlock",
    "simReadCustomDataBlock",
    "sim.readCustomDataBlock",
    "simReadCustomDataBlockTags",
    "sim.readCustomDataBlockTags",
    "simAddPointCloud",
    "sim.addPointCloud",
    "simModifyPointCloud",
    "sim.modifyPointCloud",
    "simGetShapeGeomInfo",
    "sim.getShapeGeomInfo",
    "simGetObjectsInTree",
    "sim.getObjectsInTree",
    "simSetObjectSizeValues",
    "sim.setObjectSizeValues",
    "simGetObjectSizeValues",
    "sim.getObjectSizeValues",
    "simScaleObject",
    "sim.scaleObject",
    "simSetShapeTexture",
    "sim.setShapeTexture",
    "simGetShapeTextureId",
    "sim.getShapeTextureId",
    "simGetCollectionObjects",
    "sim.getCollectionObjects",
    "simHandleCustomizationScripts",
    "sim.handleCustomizationScripts",
    "simSetScriptAttribute",
    "sim.setScriptAttribute",
    "simGetScriptAttribute",
    "sim.getScriptAttribute",
    "simHandleChildScripts",
    "sim.handleSimulationScripts",
    "simLaunchThreadedChildScripts",
    "sim.launchThreadedChildScripts",
    "simReorientShapeBoundingBox",
    "sim.reorientShapeBoundingBox",
    "simHandleVisionSensor",
    "sim.handleVisionSensor",
    "simReadVisionSensor",
    "sim.readVisionSensor",
    "simResetVisionSensor",
    "sim.resetVisionSensor",
    "simGetVisionSensorResolution",
    "sim.getVisionSensorResolution",
    "simGetVisionSensorImage",
    "sim.getVisionSensorImage",
    "simSetVisionSensorImage",
    "sim.setVisionSensorImage",
    "simGetVisionSensorCharImage",
    "sim.getVisionSensorCharImage",
    "simSetVisionSensorCharImage",
    "sim.setVisionSensorCharImage",
    "simGetVisionSensorDepthBuffer",
    "sim.getVisionSensorDepthBuffer",
    "simCheckVisionSensor",
    "sim.checkVisionSensor",
    "simCheckVisionSensorEx",
    "sim.checkVisionSensorEx",
    "simBuildMatrixQ",
    "sim.buildMatrixQ",
    "simGetQuaternionFromMatrix",
    "sim.getQuaternionFromMatrix",
    "simFileDialog",
    "sim.fileDialog",
    "simMsgBox",
    "sim.msgBox",
    "simLoadModule",
    "sim.loadModule",
    "simUnloadModule",
    "sim.unloadModule",
    "simCallScriptFunction",
    "sim.callScriptFunction",
    "simGetConfigForTipPose",
    "sim.getConfigForTipPose",
    "simGenerateIkPath",
    "sim.generateIkPath",
    "simGetExtensionString",
    "sim.getExtensionString",
    "simComputeMassAndInertia",
    "sim.computeMassAndInertia",
    "simSetScriptVariable",
    "sim.setScriptVariable",
    "simGetEngineFloatParameter",
    "sim.getEngineFloatParameter",
    "simGetEngineInt32Parameter",
    "sim.getEngineInt32Parameter",
    "simGetEngineBoolParameter",
    "sim.getEngineBoolParameter",
    "simSetEngineFloatParameter",
    "sim.setEngineFloatParameter",
    "simSetEngineInt32Parameter",
    "sim.setEngineInt32Parameter",
    "simSetEngineBoolParameter",
    "sim.setEngineBoolParameter",
    "simCreateOctree",
    "sim.createOctree",
    "simCreatePointCloud",
    "sim.createPointCloud",
    "simSetPointCloudOptions",
    "sim.setPointCloudOptions",
    "simGetPointCloudOptions",
    "sim.getPointCloudOptions",
    "simInsertVoxelsIntoOctree",
    "sim.insertVoxelsIntoOctree",
    "simRemoveVoxelsFromOctree",
    "sim.removeVoxelsFromOctree",
    "simInsertPointsIntoPointCloud",
    "sim.insertPointsIntoPointCloud",
    "simRemovePointsFromPointCloud",
    "sim.removePointsFromPointCloud",
    "simIntersectPointsWithPointCloud",
    "sim.intersectPointsWithPointCloud",
    "simGetOctreeVoxels",
    "sim.getOctreeVoxels",
    "simGetPointCloudPoints",
    "sim.getPointCloudPoints",
    "simInsertObjectIntoOctree",
    "sim.insertObjectIntoOctree",
    "simSubtractObjectFromOctree",
    "sim.subtractObjectFromOctree",
    "simInsertObjectIntoPointCloud",
    "sim.insertObjectIntoPointCloud",
    "simSubtractObjectFromPointCloud",
    "sim.subtractObjectFromPointCloud",
    "simCheckOctreePointOccupancy",
    "sim.checkOctreePointOccupancy",
    "simOpenTextEditor",
    "sim.openTextEditor",
    "simSetVisionSensorFilter",
    "sim.setVisionSensorFilter",
    "simGetVisionSensorFilter",
    "sim.getVisionSensorFilter",
    "simHandleSimulationStart",
    "sim.handleSimulationStart",
    "simHandleSensingStart",
    "sim.handleSensingStart",
    "simAuxFunc",
    "sim.auxFunc",
    "simSetReferencedHandles",
    "sim.setReferencedHandles",
    "simGetReferencedHandles",
    "sim.getReferencedHandles",
    "simGetGraphCurve",
    "sim.getGraphCurveData",
    "simTest",
    "sim.test",
    "simRMLPos",
    "sim.rmlPos",
    "simRMLVel",
    "sim.rmlVel",
    "simRMLStep",
    "sim.rmlStep",
    "simRMLRemove",
    "sim.rmlRemove",
    "simRMLMoveToPosition",
    "sim.rmlMoveToPosition",
    "simRMLMoveToJointPositions",
    "sim.rmlMoveToJointPositions",
    "simBoolOr16",
    "sim.boolOr32",
    "simBoolAnd16",
    "sim.boolAnd32",
    "simBoolXor16",
    "sim.boolXor32",
    "simPackInts",
    "sim.packInt32Table",
    "simPackUInts",
    "sim.packUInt32Table",
    "simPackFloats",
    "sim.packFloatTable",
    "simPackDoubles",
    "sim.packDoubleTable",
    "simPackBytes",
    "sim.packUInt8Table",
    "simPackWords",
    "sim.packUInt16Table",
    "simUnpackInts",
    "sim.unpackInt32Table",
    "simUnpackUInts",
    "sim.unpackUInt32Table",
    "simUnpackFloats",
    "sim.unpackFloatTable",
    "simUnpackDoubles",
    "sim.unpackDoubleTable",
    "simUnpackBytes",
    "sim.unpackUInt8Table",
    "simUnpackWords",
    "sim.unpackUInt16Table",
    "simSetBooleanParameter",
    "sim.setBoolParameter",
    "simGetBooleanParameter",
    "sim.getBoolParameter",
    "simSetIntegerParameter",
    "sim.setInt32Parameter",
    "simGetIntegerParameter",
    "sim.getInt32Parameter",
    "simSetFloatingParameter",
    "sim.setFloatParameter",
    "simGetFloatingParameter",
    "sim.getFloatParameter",
    "simGetObjectIntParameter",
    "sim.getObjectInt32Parameter",
    "simSetObjectIntParameter",
    "sim.setObjectInt32Parameter",
    "sim_object_shape_type",
    "sim.sceneobject_shape",
    "sim_object_joint_type",
    "sim.sceneobject_joint",
    "sim_object_graph_type",
    "sim.sceneobject_graph",
    "sim_object_camera_type",
    "sim.sceneobject_camera",
    "sim_object_dummy_type",
    "sim.sceneobject_dummy",
    "sim_object_proximitysensor_type",
    "sim.sceneobject_proximitysensor",
    "sim_object_path_type",
    "sim.sceneobject_path",
    "sim_object_renderingsensor_type",
    "sim.sceneobject_renderingsensor",
    "sim_object_visionsensor_type",
    "sim.sceneobject_visionsensor",
    "sim_object_mill_type",
    "sim.sceneobject_mill",
    "sim_object_forcesensor_type",
    "sim.sceneobject_forcesensor",
    "sim_object_light_type",
    "sim.sceneobject_light",
    "sim_object_mirror_type",
    "sim.sceneobject_mirror",
    "sim_object_octree_type",
    "sim.sceneobject_octree",
    "sim_object_pointcloud_type",
    "sim.sceneobject_pointcloud",
    "sim_light_omnidirectional_subtype",
    "sim.light_omnidirectional",
    "sim_light_spot_subtype",
    "sim.light_spot",
    "sim_light_directional_subtype",
    "sim.light_directional",
    "sim_joint_revolute_subtype",
    "sim.joint_revolute",
    "sim_joint_prismatic_subtype",
    "sim.joint_prismatic",
    "sim_joint_spherical_subtype",
    "sim.joint_spherical",
    "sim_shape_simpleshape_subtype",
    "sim.shape_simple",
    "sim_shape_multishape_subtype",
    "sim.shape_compound",
    "sim_proximitysensor_pyramid_subtype",
    "sim.proximitysensor_pyramid",
    "sim_proximitysensor_cylinder_subtype",
    "sim.proximitysensor_cylinder",
    "sim_proximitysensor_disc_subtype",
    "sim.proximitysensor_disc",
    "sim_proximitysensor_cone_subtype",
    "sim.proximitysensor_cone",
    "sim_proximitysensor_ray_subtype",
    "sim.proximitysensor_ray",
    "sim_mill_pyramid_subtype",
    "sim.mill_pyramid_subtype",
    "sim_mill_cylinder_subtype",
    "sim.mill_cylinder_subtype",
    "sim_mill_disc_subtype",
    "sim.mill_disc_subtype",
    "sim_mill_cone_subtype",
    "sim.mill_cone_subtype",
    "sim_object_no_subtype",
    "sim.object_no_subtype",
    "sim_appobj_object_type",
    "sim.objecttype_sceneobject",
    "sim_appobj_collision_type",
    "sim.appobj_collision_type",
    "sim_appobj_distance_type",
    "sim.appobj_distance_type",
    "sim_appobj_simulation_type",
    "sim.appobj_simulation_type",
    "sim_appobj_ik_type",
    "sim.appobj_ik_type",
    "sim_appobj_constraintsolver_type",
    "sim.appobj_constraintsolver_type",
    "sim_appobj_collection_type",
    "sim.appobj_collection_type",
    "sim_appobj_2delement_type",
    "sim.appobj_2delement_type",
    "sim_appobj_ui_type",
    "sim.appobj_ui_type",
    "sim_appobj_script_type",
    "sim.appobj_script_type",
    "sim_appobj_pathplanning_type",
    "sim.appobj_pathplanning_type",
    "sim_appobj_texture_type",
    "sim.objecttype_texture",
    "sim_appobj_motionplanning_type",
    "sim.appobj_motionplanning_type",
    "sim_ik_pseudo_inverse_method",
    "sim.ik_pseudo_inverse_method",
    "sim_ik_damped_least_squares_method",
    "sim.ik_damped_least_squares_method",
    "sim_ik_jacobian_transpose_method",
    "sim.ik_jacobian_transpose_method",
    "sim_ik_x_constraint",
    "sim.ik_x_constraint",
    "sim_ik_y_constraint",
    "sim.ik_y_constraint",
    "sim_ik_z_constraint",
    "sim.ik_z_constraint",
    "sim_ik_alpha_beta_constraint",
    "sim.ik_alpha_beta_constraint",
    "sim_ik_gamma_constraint",
    "sim.ik_gamma_constraint",
    "sim_ikresult_not_performed",
    "sim.ikresult_not_performed",
    "sim_ikresult_success",
    "sim.ikresult_success",
    "sim_ikresult_fail",
    "sim.ikresult_fail",
    "sim_message_ui_button_state_change",
    "sim.message_ui_button_state_change",
    "sim_message_model_loaded",
    "sim.message_model_loaded",
    "sim_message_scene_loaded",
    "sim.message_scene_loaded",
    "sim_message_object_selection_changed",
    "sim.message_object_selection_changed",
    "sim_message_keypress",
    "sim.message_keypress",
    "sim_message_bannerclicked",
    "sim.message_bannerclicked",
    "sim_message_prox_sensor_select_down",
    "sim.message_prox_sensor_select_down",
    "sim_message_prox_sensor_select_up",
    "sim.message_prox_sensor_select_up",
    "sim_message_pick_select_down",
    "sim.message_pick_select_down",
    "sim_buttonproperty_button",
    "sim.buttonproperty_button",
    "sim_buttonproperty_label",
    "sim.buttonproperty_label",
    "sim_buttonproperty_editbox",
    "sim.buttonproperty_editbox",
    "sim_buttonproperty_slider",
    "sim.buttonproperty_slider",
    "sim_buttonproperty_staydown",
    "sim.buttonproperty_staydown",
    "sim_buttonproperty_enabled",
    "sim.buttonproperty_enabled",
    "sim_buttonproperty_borderless",
    "sim.buttonproperty_borderless",
    "sim_buttonproperty_horizontallycentered",
    "sim.buttonproperty_horizontallycentered",
    "sim_buttonproperty_ignoremouse",
    "sim.buttonproperty_ignoremouse",
    "sim_buttonproperty_isdown",
    "sim.buttonproperty_isdown",
    "sim_buttonproperty_transparent",
    "sim.buttonproperty_transparent",
    "sim_buttonproperty_nobackgroundcolor",
    "sim.buttonproperty_nobackgroundcolor",
    "sim_buttonproperty_rollupaction",
    "sim.buttonproperty_rollupaction",
    "sim_buttonproperty_closeaction",
    "sim.buttonproperty_closeaction",
    "sim_buttonproperty_verticallycentered",
    "sim.buttonproperty_verticallycentered",
    "sim_buttonproperty_downupevent",
    "sim.buttonproperty_downupevent",
    "sim_objectproperty_collapsed",
    "sim.objectproperty_collapsed",
    "sim_objectproperty_selectable",
    "sim.objectproperty_selectable",
    "sim_objectproperty_selectmodelbaseinstead",
    "sim.objectproperty_selectmodelbaseinstead",
    "sim_objectproperty_dontshowasinsidemodel",
    "sim.objectproperty_dontshowasinsidemodel",
    "sim_objectproperty_canupdatedna",
    "sim.objectproperty_canupdatedna",
    "sim_objectproperty_selectinvisible",
    "sim.objectproperty_selectinvisible",
    "sim_objectproperty_depthinvisible",
    "sim.objectproperty_depthinvisible",
    "sim_objectproperty_cannotdelete",
    "sim.objectproperty_cannotdelete",
    "sim_simulation_stopped",
    "sim.simulation_stopped",
    "sim_simulation_paused",
    "sim.simulation_paused",
    "sim_simulation_advancing",
    "sim.simulation_advancing",
    "sim_simulation_advancing_running",
    "sim.simulation_advancing_running",
    "sim_simulation_advancing_lastbeforestop",
    "sim.simulation_advancing_lastbeforestop",
    "sim_texturemap_plane",
    "sim.texturemap_plane",
    "sim_texturemap_cylinder",
    "sim.texturemap_cylinder",
    "sim_texturemap_sphere",
    "sim.texturemap_sphere",
    "sim_texturemap_cube",
    "sim.texturemap_cube",
    "sim_scripttype_mainscript",
    "sim.scripttype_main",
    "sim_scripttype_childscript",
    "sim.scripttype_simulation",
    "sim_scripttype_addonscript",
    "sim.scripttype_addon",
    "sim_scripttype_addonfunction",
    "sim.scripttype_addonfunction",
    "sim_scripttype_customizationscript",
    "sim.scripttype_customization",
    "sim_scripttype_threaded",
    "sim.scripttype_threaded",
    "sim_mainscriptcall_initialization",
    "sim.syscb_init",
    "sim_mainscriptcall_cleanup",
    "sim.syscb_cleanup",
    "sim_mainscriptcall_regular",
    "sim.syscb_regular",
    "sim_childscriptcall_initialization",
    "sim.syscb_init",
    "sim_childscriptcall_cleanup",
    "sim.syscb_cleanup",
    "sim_childscriptcall_actuation",
    "sim.syscb_actuation",
    "sim_childscriptcall_sensing",
    "sim.syscb_sensing",
    "sim_childscriptcall_threaded",
    "-1",
    "sim_customizationscriptcall_initialization",
    "sim.syscb_init",
    "sim_customizationscriptcall_cleanup",
    "sim.syscb_cleanup",
    "sim_customizationscriptcall_nonsimulation",
    "sim.syscb_nonsimulation",
    "sim_customizationscriptcall_lastbeforesimulation",
    "sim.syscb_beforesimulation",
    "sim_customizationscriptcall_firstaftersimulation",
    "sim.syscb_aftersimulation",
    "sim_customizationscriptcall_simulationactuation",
    "sim.syscb_actuation",
    "sim_customizationscriptcall_simulationsensing",
    "sim.syscb_sensing",
    "sim_customizationscriptcall_simulationpause",
    "sim.syscb_suspended",
    "sim_customizationscriptcall_simulationpausefirst",
    "sim.syscb_suspend",
    "sim_customizationscriptcall_simulationpauselast",
    "sim.syscb_resume",
    "sim_customizationscriptcall_lastbeforeinstanceswitch",
    "sim.syscb_beforeinstanceswitch",
    "sim_customizationscriptcall_firstafterinstanceswitch",
    "sim.syscb_afterinstanceswitch",
    "sim_addonscriptcall_initialization",
    "sim.syscb_init",
    "sim_addonscriptcall_run",
    "sim.syscb_run",
    "sim_addonscriptcall_suspend",
    "sim.syscb_suspend",
    "sim_addonscriptcall_restarting",
    "sim.syscb_resume",
    "sim_addonscriptcall_cleanup",
    "sim.syscb_cleanup",
    "sim_customizationscriptattribute_activeduringsimulation",
    "sim.customizationscriptattribute_activeduringsimulation",
    "sim_scriptattribute_executionorder",
    "sim.scriptattribute_executionorder",
    "sim_scriptattribute_executioncount",
    "sim.scriptattribute_executioncount",
    "sim_childscriptattribute_automaticcascadingcalls",
    "sim.childscriptattribute_automaticcascadingcalls",
    "sim_childscriptattribute_enabled",
    "sim.childscriptattribute_enabled",
    "sim_scriptattribute_enabled",
    "sim.scriptattribute_enabled",
    "sim_customizationscriptattribute_cleanupbeforesave",
    "sim.customizationscriptattribute_cleanupbeforesave",
    "sim_scriptexecorder_first",
    "sim.scriptexecorder_first",
    "sim_scriptexecorder_normal",
    "sim.scriptexecorder_normal",
    "sim_scriptexecorder_last",
    "sim.scriptexecorder_last",
    "sim_scriptthreadresume_allnotyetresumed",
    "sim.scriptthreadresume_allnotyetresumed",
    "sim_scriptthreadresume_default",
    "sim.scriptthreadresume_default",
    "sim_scriptthreadresume_actuation_first",
    "sim.scriptthreadresume_actuation_first",
    "sim_scriptthreadresume_actuation_last",
    "sim.scriptthreadresume_actuation_last",
    "sim_scriptthreadresume_sensing_first",
    "sim.scriptthreadresume_sensing_first",
    "sim_scriptthreadresume_sensing_last",
    "sim.scriptthreadresume_sensing_last",
    "sim_scriptthreadresume_custom",
    "sim.scriptthreadresume_custom",
    "sim_callbackid_rossubscriber",
    "sim.callbackid_rossubscriber",
    "sim_callbackid_dynstep",
    "sim.callbackid_dynstep",
    "sim_callbackid_userdefined",
    "sim.callbackid_userdefined",
    "sim_script_no_error",
    "sim.script_no_error",
    "sim_script_main_script_nonexistent",
    "sim.script_main_script_nonexistent",
    "sim_script_main_not_called",
    "sim.script_main_not_called",
    "sim_script_reentrance_error",
    "sim.script_reentrance_error",
    "sim_script_lua_error",
    "sim.script_lua_error",
    "sim_script_call_error",
    "sim.script_call_error",
    "sim_api_error_report",
    "sim.api_error_report",
    "sim_api_error_output",
    "sim.api_error_output",
    "sim_api_warning_output",
    "sim.api_warning_output",
    "sim_handle_all",
    "sim.handle_all",
    "sim_handle_all_except_explicit",
    "sim.handle_all_except_explicit",
    "sim_handle_self",
    "sim.handle_self",
    "sim_handle_mainscript",
    "sim.handle_mainscript",
    "sim_handle_tree",
    "sim.handle_tree",
    "sim_handle_chain",
    "sim.handle_chain",
    "sim_handle_single",
    "sim.handle_single",
    "sim_handle_default",
    "sim.handle_default",
    "sim_handle_all_except_self",
    "sim.handle_all_except_self",
    "sim_handle_parent",
    "sim.handle_parent",
    "sim_handle_scene",
    "sim.handle_scene",
    "sim_handle_app",
    "sim.handle_app",
    "sim_handleflag_assembly",
    "sim.handleflag_assembly",
    "sim_handleflag_camera",
    "sim.handleflag_camera",
    "sim_handleflag_togglevisibility",
    "sim.handleflag_togglevisibility",
    "sim_handleflag_extended",
    "sim.handleflag_extended",
    "sim_handleflag_greyscale",
    "sim.handleflag_greyscale",
    "sim_handleflag_codedstring",
    "sim.handleflag_codedstring",
    "sim_handleflag_model",
    "sim.handleflag_model",
    "sim_handleflag_rawvalue",
    "sim.handleflag_rawvalue",
    "sim_objectspecialproperty_collidable",
    "sim.objectspecialproperty_collidable",
    "sim_objectspecialproperty_measurable",
    "sim.objectspecialproperty_measurable",
    "sim_objectspecialproperty_detectable_ultrasonic",
    "sim.objectspecialproperty_detectable_ultrasonic",
    "sim_objectspecialproperty_detectable_infrared",
    "sim.objectspecialproperty_detectable_infrared",
    "sim_objectspecialproperty_detectable_laser",
    "sim.objectspecialproperty_detectable_laser",
    "sim_objectspecialproperty_detectable_inductive",
    "sim.objectspecialproperty_detectable_inductive",
    "sim_objectspecialproperty_detectable_capacitive",
    "sim.objectspecialproperty_detectable_capacitive",
    "sim_objectspecialproperty_renderable",
    "sim.objectspecialproperty_renderable",
    "sim_objectspecialproperty_detectable_all",
    "sim.objectspecialproperty_detectable",
    "sim_objectspecialproperty_pathplanning_ignored",
    "sim.objectspecialproperty_pathplanning_ignored",
    "sim_modelproperty_not_collidable",
    "sim.modelproperty_not_collidable",
    "sim_modelproperty_not_measurable",
    "sim.modelproperty_not_measurable",
    "sim_modelproperty_not_renderable",
    "sim.modelproperty_not_renderable",
    "sim_modelproperty_not_detectable",
    "sim.modelproperty_not_detectable",
    "sim_modelproperty_not_dynamic",
    "sim.modelproperty_not_dynamic",
    "sim_modelproperty_not_respondable",
    "sim.modelproperty_not_respondable",
    "sim_modelproperty_not_reset",
    "sim.modelproperty_not_reset",
    "sim_modelproperty_not_visible",
    "sim.modelproperty_not_visible",
    "sim_modelproperty_scripts_inactive",
    "sim.modelproperty_scripts_inactive",
    "sim_modelproperty_not_showasinsidemodel",
    "sim.modelproperty_not_showasinsidemodel",
    "sim_modelproperty_not_model",
    "sim.modelproperty_not_model",
    "sim_dlgstyle_message",
    "sim.dlgstyle_message",
    "sim_dlgstyle_input",
    "sim.dlgstyle_input",
    "sim_dlgstyle_ok",
    "sim.dlgstyle_ok",
    "sim_dlgstyle_ok_cancel",
    "sim.dlgstyle_ok_cancel",
    "sim_dlgstyle_yes_no",
    "sim.dlgstyle_yes_no",
    "sim_dlgstyle_dont_center",
    "sim.dlgstyle_dont_center",
    "sim_dlgret_still_open",
    "sim.dlgret_still_open",
    "sim_dlgret_ok",
    "sim.dlgret_ok",
    "sim_dlgret_cancel",
    "sim.dlgret_cancel",
    "sim_dlgret_yes",
    "sim.dlgret_yes",
    "sim_dlgret_no",
    "sim.dlgret_no",
    "sim_pathproperty_show_line",
    "sim.pathproperty_show_line",
    "sim_pathproperty_show_orientation",
    "sim.pathproperty_show_orientation",
    "sim_pathproperty_closed_path",
    "sim.pathproperty_closed_path",
    "sim_pathproperty_automatic_orientation",
    "sim.pathproperty_automatic_orientation",
    "sim_pathproperty_flat_path",
    "sim.pathproperty_flat_path",
    "sim_pathproperty_show_position",
    "sim.pathproperty_show_position",
    "sim_pathproperty_keep_x_up",
    "sim.pathproperty_keep_x_up",
    "sim_distcalcmethod_dl",
    "sim.distcalcmethod_dl",
    "sim_distcalcmethod_dac",
    "sim.distcalcmethod_dac",
    "sim_distcalcmethod_max_dl_dac",
    "sim.distcalcmethod_max_dl_dac",
    "sim_distcalcmethod_dl_and_dac",
    "sim.distcalcmethod_dl_and_dac",
    "sim_distcalcmethod_sqrt_dl2_and_dac2",
    "sim.distcalcmethod_sqrt_dl2_and_dac2",
    "sim_distcalcmethod_dl_if_nonzero",
    "sim.distcalcmethod_dl_if_nonzero",
    "sim_distcalcmethod_dac_if_nonzero",
    "sim.distcalcmethod_dac_if_nonzero",
    "sim_boolparam_hierarchy_visible",
    "sim.boolparam_hierarchy_visible",
    "sim_boolparam_console_visible",
    "sim.boolparam_console_visible",
    "sim_boolparam_collision_handling_enabled",
    "sim.boolparam_collision_handling_enabled",
    "sim_boolparam_distance_handling_enabled",
    "sim.boolparam_distance_handling_enabled",
    "sim_boolparam_ik_handling_enabled",
    "sim.boolparam_ik_handling_enabled",
    "sim_boolparam_gcs_handling_enabled",
    "sim.boolparam_gcs_handling_enabled",
    "sim_boolparam_dynamics_handling_enabled",
    "sim.boolparam_dynamics_handling_enabled",
    "sim_boolparam_proximity_sensor_handling_enabled",
    "sim.boolparam_proximity_sensor_handling_enabled",
    "sim_boolparam_vision_sensor_handling_enabled",
    "sim.boolparam_vision_sensor_handling_enabled",
    "sim_boolparam_rendering_sensor_handling_enabled",
    "sim.boolparam_rendering_sensor_handling_enabled",
    "sim_boolparam_mill_handling_enabled",
    "sim.boolparam_mill_handling_enabled",
    "sim_boolparam_browser_visible",
    "sim.boolparam_browser_visible",
    "sim_boolparam_scene_and_model_load_messages",
    "sim.boolparam_scene_and_model_load_messages",
    "sim_boolparam_shape_textures_are_visible",
    "sim.boolparam_shape_textures_are_visible",
    "sim_boolparam_display_enabled",
    "sim.boolparam_display_enabled",
    "sim_boolparam_infotext_visible",
    "sim.boolparam_infotext_visible",
    "sim_boolparam_statustext_open",
    "sim.boolparam_statustext_open",
    "sim_boolparam_fog_enabled",
    "sim.boolparam_fog_enabled",
    "sim_boolparam_rml2_available",
    "sim.boolparam_rml2_available",
    "sim_boolparam_rml4_available",
    "sim.boolparam_rml4_available",
    "sim_boolparam_mirrors_enabled",
    "sim.boolparam_mirrors_enabled",
    "sim_boolparam_aux_clip_planes_enabled",
    "sim.boolparam_aux_clip_planes_enabled",
    "sim_boolparam_full_model_copy_from_api",
    "sim.boolparam_full_model_copy_from_api",
    "sim_boolparam_realtime_simulation",
    "sim.boolparam_realtime_simulation",
    "sim_boolparam_use_glfinish_cmd",
    "sim.boolparam_use_glfinish_cmd",
    "sim_boolparam_force_show_wireless_emission",
    "sim.boolparam_force_show_wireless_emission",
    "sim_boolparam_force_show_wireless_reception",
    "sim.boolparam_force_show_wireless_reception",
    "sim_boolparam_video_recording_triggered",
    "sim.boolparam_video_recording_triggered",
    "sim_boolparam_fullscreen",
    "sim.boolparam_fullscreen",
    "sim_boolparam_headless",
    "sim.boolparam_headless",
    "sim_boolparam_hierarchy_toolbarbutton_enabled",
    "sim.boolparam_hierarchy_toolbarbutton_enabled",
    "sim_boolparam_browser_toolbarbutton_enabled",
    "sim.boolparam_browser_toolbarbutton_enabled",
    "sim_boolparam_objectshift_toolbarbutton_enabled",
    "sim.boolparam_objectshift_toolbarbutton_enabled",
    "sim_boolparam_objectrotate_toolbarbutton_enabled",
    "sim.boolparam_objectrotate_toolbarbutton_enabled",
    "sim_boolparam_force_calcstruct_all_visible",
    "sim.boolparam_force_calcstruct_all_visible",
    "sim_boolparam_force_calcstruct_all",
    "sim.boolparam_force_calcstruct_all",
    "sim_boolparam_exit_request",
    "sim.boolparam_exit_request",
    "sim_boolparam_play_toolbarbutton_enabled",
    "sim.boolparam_play_toolbarbutton_enabled",
    "sim_boolparam_pause_toolbarbutton_enabled",
    "sim.boolparam_pause_toolbarbutton_enabled",
    "sim_boolparam_stop_toolbarbutton_enabled",
    "sim.boolparam_stop_toolbarbutton_enabled",
    "sim_boolparam_waiting_for_trigger",
    "sim.boolparam_waiting_for_trigger",
    "sim_boolparam_objproperties_toolbarbutton_enabled",
    "sim.boolparam_objproperties_toolbarbutton_enabled",
    "sim_boolparam_calcmodules_toolbarbutton_enabled",
    "sim.boolparam_calcmodules_toolbarbutton_enabled",
    "sim_boolparam_rosinterface_donotrunmainscript",
    "sim.boolparam_rosinterface_donotrunmainscript",
    "sim_intparam_error_report_mode",
    "sim.intparam_error_report_mode",
    "sim_intparam_program_version",
    "sim.intparam_program_version",
    "sim_intparam_compilation_version",
    "sim.intparam_compilation_version",
    "sim_intparam_current_page",
    "sim.intparam_current_page",
    "sim_intparam_flymode_camera_handle",
    "sim.intparam_flymode_camera_handle",
    "sim_intparam_dynamic_step_divider",
    "sim.intparam_dynamic_step_divider",
    "sim_intparam_dynamic_engine",
    "sim.intparam_dynamic_engine",
    "sim_intparam_server_port_start",
    "sim.intparam_server_port_start",
    "sim_intparam_server_port_range",
    "sim.intparam_server_port_range",
    "sim_intparam_server_port_next",
    "sim.intparam_server_port_next",
    "sim_intparam_visible_layers",
    "sim.intparam_visible_layers",
    "sim_intparam_infotext_style",
    "sim.intparam_infotext_style",
    "sim_intparam_settings",
    "sim.intparam_settings",
    "sim_intparam_qt_version",
    "sim.intparam_qt_version",
    "sim_intparam_event_flags_read",
    "sim.intparam_event_flags_read",
    "sim_intparam_event_flags_read_clear",
    "sim.intparam_event_flags_read_clear",
    "sim_intparam_platform",
    "sim.intparam_platform",
    "sim_intparam_scene_unique_id",
    "sim.intparam_scene_unique_id",
    "sim_intparam_edit_mode_type",
    "sim.intparam_edit_mode_type",
    "sim_intparam_work_thread_count",
    "sim.intparam_work_thread_count",
    "sim_intparam_mouse_x",
    "sim.intparam_mouse_x",
    "sim_intparam_mouse_y",
    "sim.intparam_mouse_y",
    "sim_intparam_core_count",
    "sim.intparam_core_count",
    "sim_intparam_work_thread_calc_time_ms",
    "sim.intparam_work_thread_calc_time_ms",
    "sim_intparam_idle_fps",
    "sim.intparam_idle_fps",
    "sim_intparam_prox_sensor_select_down",
    "sim.intparam_prox_sensor_select_down",
    "sim_intparam_prox_sensor_select_up",
    "sim.intparam_prox_sensor_select_up",
    "sim_intparam_stop_request_counter",
    "sim.intparam_stop_request_counter",
    "sim_intparam_program_revision",
    "sim.intparam_program_revision",
    "sim_intparam_mouse_buttons",
    "sim.intparam_mouse_buttons",
    "sim_intparam_dynamic_warning_disabled_mask",
    "sim.intparam_dynamic_warning_disabled_mask",
    "sim_intparam_simulation_warning_disabled_mask",
    "sim.intparam_simulation_warning_disabled_mask",
    "sim_intparam_scene_index",
    "sim.intparam_scene_index",
    "sim_intparam_motionplanning_seed",
    "sim.intparam_motionplanning_seed",
    "sim_intparam_speedmodifier",
    "sim.intparam_speedmodifier",
    "sim_intparam_dynamic_iteration_count",
    "sim.intparam_dynamic_iteration_count",
    "sim_floatparam_rand",
    "sim.floatparam_rand",
    "sim_floatparam_simulation_time_step",
    "sim.floatparam_simulation_time_step",
    "sim_floatparam_stereo_distance",
    "sim.floatparam_stereo_distance",
    "sim_floatparam_dynamic_step_size",
    "sim.floatparam_dynamic_step_size",
    "sim_floatparam_mouse_wheel_zoom_factor",
    "sim.floatparam_mouse_wheel_zoom_factor",
    "sim_arrayparam_gravity",
    "sim.arrayparam_gravity",
    "sim_arrayparam_fog",
    "sim.arrayparam_fog",
    "sim_arrayparam_fog_color",
    "sim.arrayparam_fog_color",
    "sim_arrayparam_background_color1",
    "sim.arrayparam_background_color1",
    "sim_arrayparam_background_color2",
    "sim.arrayparam_background_color2",
    "sim_arrayparam_ambient_light",
    "sim.arrayparam_ambient_light",
    "sim_arrayparam_random_euler",
    "sim.arrayparam_random_euler",
    "sim_stringparam_application_path",
    "sim.stringparam_application_path",
    "sim_stringparam_video_filename",
    "sim.stringparam_video_filename",
    "sim_stringparam_app_arg1",
    "sim.stringparam_app_arg1",
    "sim_stringparam_app_arg2",
    "sim.stringparam_app_arg2",
    "sim_stringparam_app_arg3",
    "sim.stringparam_app_arg3",
    "sim_stringparam_app_arg4",
    "sim.stringparam_app_arg4",
    "sim_stringparam_app_arg5",
    "sim.stringparam_app_arg5",
    "sim_stringparam_app_arg6",
    "sim.stringparam_app_arg6",
    "sim_stringparam_app_arg7",
    "sim.stringparam_app_arg7",
    "sim_stringparam_app_arg8",
    "sim.stringparam_app_arg8",
    "sim_stringparam_app_arg9",
    "sim.stringparam_app_arg9",
    "sim_stringparam_scene_path_and_name",
    "sim.stringparam_scene_path_and_name",
    "sim_stringparam_remoteapi_temp_file_dir",
    "sim.stringparam_remoteapi_temp_file_dir",
    "sim_stringparam_scene_path",
    "sim.stringparam_scene_path",
    "sim_stringparam_scene_name",
    "sim.stringparam_scene_name",
    "sim_displayattribute_renderpass",
    "sim.displayattribute_renderpass",
    "sim_displayattribute_depthpass",
    "sim.displayattribute_depthpass",
    "sim_displayattribute_pickpass",
    "sim.displayattribute_pickpass",
    "sim_displayattribute_selected",
    "sim.displayattribute_selected",
    "sim_displayattribute_mainselection",
    "sim.displayattribute_mainselection",
    "sim_displayattribute_forcewireframe",
    "sim.displayattribute_forcewireframe",
    "sim_displayattribute_forbidwireframe",
    "sim.displayattribute_forbidwireframe",
    "sim_displayattribute_forbidedges",
    "sim.displayattribute_forbidedges",
    "sim_displayattribute_originalcolors",
    "sim.displayattribute_originalcolors",
    "sim_displayattribute_ignorelayer",
    "sim.displayattribute_ignorelayer",
    "sim_displayattribute_forvisionsensor",
    "sim.displayattribute_forvisionsensor",
    "sim_displayattribute_colorcodedpickpass",
    "sim.displayattribute_colorcodedpickpass",
    "sim_displayattribute_colorcoded",
    "sim.displayattribute_colorcoded",
    "sim_displayattribute_trianglewireframe",
    "sim.displayattribute_trianglewireframe",
    "sim_displayattribute_thickEdges",
    "sim.displayattribute_thickedges",
    "sim_displayattribute_dynamiccontentonly",
    "sim.displayattribute_dynamiccontentonly",
    "sim_displayattribute_mirror",
    "sim.displayattribute_mirror",
    "sim_displayattribute_useauxcomponent",
    "sim.displayattribute_useauxcomponent",
    "sim_displayattribute_ignorerenderableflag",
    "sim.displayattribute_ignorerenderableflag",
    "sim_displayattribute_noopenglcallbacks",
    "sim.displayattribute_noopenglcallbacks",
    "sim_displayattribute_noghosts",
    "sim.displayattribute_noghosts",
    "sim_displayattribute_nopointclouds",
    "sim.displayattribute_nopointclouds",
    "sim_displayattribute_nodrawingobjects",
    "sim.displayattribute_nodrawingobjects",
    "sim_displayattribute_noparticles",
    "sim.displayattribute_noparticles",
    "sim_displayattribute_colorcodedtriangles",
    "sim.displayattribute_colorcodedtriangles",
    "sim_navigation_passive",
    "sim.navigation_passive",
    "sim_navigation_camerashift",
    "sim.navigation_camerashift",
    "sim_navigation_camerarotate",
    "sim.navigation_camerarotate",
    "sim_navigation_camerazoom",
    "sim.navigation_camerazoom",
    "sim_navigation_cameratilt",
    "sim.navigation_cameratilt",
    "sim_navigation_cameraangle",
    "sim.navigation_cameraangle",
    "sim_navigation_camerafly",
    "sim.navigation_camerafly",
    "sim_navigation_objectshift",
    "sim.navigation_objectshift",
    "sim_navigation_objectrotate",
    "sim.navigation_objectrotate",
    "sim_navigation_createpathpoint",
    "sim.navigation_createpathpoint",
    "sim_navigation_clickselection",
    "sim.navigation_clickselection",
    "sim_navigation_ctrlselection",
    "sim.navigation_ctrlselection",
    "sim_navigation_shiftselection",
    "sim.navigation_shiftselection",
    "sim_navigation_camerazoomwheel",
    "sim.navigation_camerazoomwheel",
    "sim_navigation_camerarotaterightbutton",
    "sim.navigation_camerarotaterightbutton",
    "sim_navigation_camerarotatemiddlebutton",
    "sim.navigation_camerarotatemiddlebutton",
    "sim_drawing_points",
    "sim.drawing_points",
    "sim_drawing_lines",
    "sim.drawing_lines",
    "sim_drawing_triangles",
    "sim.drawing_triangles",
    "sim_drawing_trianglepoints",
    "sim.drawing_trianglepoints",
    "sim_drawing_quadpoints",
    "sim.drawing_quadpoints",
    "sim_drawing_discpoints",
    "sim.drawing_discpoints",
    "sim_drawing_cubepoints",
    "sim.drawing_cubepoints",
    "sim_drawing_spherepoints",
    "sim.drawing_spherepoints",
    "sim_drawing_itemcolors",
    "sim.drawing_itemcolors",
    "sim_drawing_vertexcolors",
    "sim.drawing_vertexcolors",
    "sim_drawing_itemsizes",
    "sim.drawing_itemsizes",
    "sim_drawing_backfaceculling",
    "sim.drawing_backfaceculling",
    "sim_drawing_wireframe",
    "sim.drawing_wireframe",
    "sim_drawing_painttag",
    "sim.drawing_painttag",
    "sim_drawing_followparentvisibility",
    "sim.drawing_followparentvisibility",
    "sim_drawing_cyclic",
    "sim.drawing_cyclic",
    "sim_drawing_50percenttransparency",
    "sim.drawing_50percenttransparency",
    "sim_drawing_25percenttransparency",
    "sim.drawing_25percenttransparency",
    "sim_drawing_12percenttransparency",
    "sim.drawing_12percenttransparency",
    "sim_drawing_emissioncolor",
    "sim.drawing_emissioncolor",
    "sim_drawing_facingcamera",
    "sim.drawing_facingcamera",
    "sim_drawing_overlay",
    "sim.drawing_overlay",
    "sim_drawing_itemtransparency",
    "sim.drawing_itemtransparency",
    "sim_drawing_persistent",
    "sim.drawing_persistent",
    "sim_drawing_auxchannelcolor1",
    "sim.drawing_auxchannelcolor1",
    "sim_drawing_auxchannelcolor2",
    "sim.drawing_auxchannelcolor2",
    "sim_banner_left",
    "sim.banner_left",
    "sim_banner_right",
    "sim.banner_right",
    "sim_banner_nobackground",
    "sim.banner_nobackground",
    "sim_banner_overlay",
    "sim.banner_overlay",
    "sim_banner_followparentvisibility",
    "sim.banner_followparentvisibility",
    "sim_banner_clickselectsparent",
    "sim.banner_clickselectsparent",
    "sim_banner_clicktriggersevent",
    "sim.banner_clicktriggersevent",
    "sim_banner_facingcamera",
    "sim.banner_facingcamera",
    "sim_banner_fullyfacingcamera",
    "sim.banner_fullyfacingcamera",
    "sim_banner_backfaceculling",
    "sim.banner_backfaceculling",
    "sim_banner_keepsamesize",
    "sim.banner_keepsamesize",
    "sim_banner_bitmapfont",
    "sim.banner_bitmapfont",
    "sim_particle_points1",
    "sim.particle_points1",
    "sim_particle_points2",
    "sim.particle_points2",
    "sim_particle_points4",
    "sim.particle_points4",
    "sim_particle_roughspheres",
    "sim.particle_roughspheres",
    "sim_particle_spheres",
    "sim.particle_spheres",
    "sim_particle_respondable1to4",
    "sim.particle_respondable1to4",
    "sim_particle_respondable5to8",
    "sim.particle_respondable5to8",
    "sim_particle_particlerespondable",
    "sim.particle_particlerespondable",
    "sim_particle_ignoresgravity",
    "sim.particle_ignoresgravity",
    "sim_particle_invisible",
    "sim.particle_invisible",
    "sim_particle_painttag",
    "sim.particle_painttag",
    "sim_particle_itemsizes",
    "sim.particle_itemsizes",
    "sim_particle_itemdensities",
    "sim.particle_itemdensities",
    "sim_particle_itemcolors",
    "sim.particle_itemcolors",
    "sim_particle_cyclic",
    "sim.particle_cyclic",
    "sim_particle_emissioncolor",
    "sim.particle_emissioncolor",
    "sim_particle_water",
    "sim.particle_water",
    "sim_jointmode_passive",
    "sim.jointmode_kinematic",
    "sim_jointmode_ik",
    "sim.jointmode_ik",
    "sim_jointmode_ikdependent",
    "sim.jointmode_ikdependent",
    "sim_jointmode_dependent",
    "sim.jointmode_dependent",
    "sim_jointmode_force",
    "sim.jointmode_dynamic",
    "sim_filedlg_type_load",
    "sim.filedlg_type_load",
    "sim_filedlg_type_save",
    "sim.filedlg_type_save",
    "sim_filedlg_type_load_multiple",
    "sim.filedlg_type_load_multiple",
    "sim_filedlg_type_folder",
    "sim.filedlg_type_folder",
    "sim_msgbox_type_info",
    "sim.msgbox_type_info",
    "sim_msgbox_type_question",
    "sim.msgbox_type_question",
    "sim_msgbox_type_warning",
    "sim.msgbox_type_warning",
    "sim_msgbox_type_critical",
    "sim.msgbox_type_critical",
    "sim_msgbox_buttons_ok",
    "sim.msgbox_buttons_ok",
    "sim_msgbox_buttons_yesno",
    "sim.msgbox_buttons_yesno",
    "sim_msgbox_buttons_yesnocancel",
    "sim.msgbox_buttons_yesnocancel",
    "sim_msgbox_buttons_okcancel",
    "sim.msgbox_buttons_okcancel",
    "sim_msgbox_return_cancel",
    "sim.msgbox_return_cancel",
    "sim_msgbox_return_no",
    "sim.msgbox_return_no",
    "sim_msgbox_return_yes",
    "sim.msgbox_return_yes",
    "sim_msgbox_return_ok",
    "sim.msgbox_return_ok",
    "sim_msgbox_return_error",
    "sim.msgbox_return_error",
    "sim_physics_bullet",
    "sim.physics_bullet",
    "sim_physics_ode",
    "sim.physics_ode",
    "sim_physics_vortex",
    "sim.physics_vortex",
    "sim_physics_newton",
    "sim.physics_newton",
    "sim_pure_primitive_none",
    "sim.primitiveshape_none",
    "sim_pure_primitive_plane",
    "sim.primitiveshape_plane",
    "sim_pure_primitive_disc",
    "sim.primitiveshape_disc",
    "sim_pure_primitive_cuboid",
    "sim.primitiveshape_cuboid",
    "sim_pure_primitive_spheroid",
    "sim.primitiveshape_spheroid",
    "sim_pure_primitive_cylinder",
    "sim.primitiveshape_cylinder",
    "sim_pure_primitive_cone",
    "sim.primitiveshape_cone",
    "sim_pure_primitive_heightfield",
    "sim.primitiveshape_heightfield",
    "sim_dummy_linktype_dynamics_loop_closure",
    "sim.dummytype_dynloopclosure",
    "sim_dummy_linktype_dynamics_force_constraint",
    "sim.dummy_linktype_dynamics_force_constraint",
    "sim_dummy_linktype_gcs_loop_closure",
    "sim.dummy_linktype_gcs_loop_closure",
    "sim_dummy_linktype_gcs_tip",
    "sim.dummy_linktype_gcs_tip",
    "sim_dummy_linktype_gcs_target",
    "sim.dummy_linktype_gcs_target",
    "sim_dummy_linktype_ik_tip_target",
    "sim.dummy_linktype_ik_tip_target",
    "sim_colorcomponent_ambient",
    "sim.colorcomponent_ambient",
    "sim_colorcomponent_ambient_diffuse",
    "sim.colorcomponent_ambient_diffuse",
    "sim_colorcomponent_diffuse",
    "sim.colorcomponent_diffuse",
    "sim_colorcomponent_specular",
    "sim.colorcomponent_specular",
    "sim_colorcomponent_emission",
    "sim.colorcomponent_emission",
    "sim_colorcomponent_transparency",
    "sim.colorcomponent_transparency",
    "sim_colorcomponent_auxiliary",
    "sim.colorcomponent_auxiliary",
    "sim_volume_ray",
    "sim.volume_ray",
    "sim_volume_randomizedray",
    "sim.volume_randomizedray",
    "sim_volume_pyramid",
    "sim.volume_pyramid",
    "sim_volume_cylinder",
    "sim.volume_cylinder",
    "sim_volume_disc",
    "sim.volume_disc",
    "sim_volume_cone",
    "sim.volume_cone",
    "sim_objintparam_visibility_layer",
    "sim.objintparam_visibility_layer",
    "sim_objfloatparam_abs_x_velocity",
    "sim.objfloatparam_abs_x_velocity",
    "sim_objfloatparam_abs_y_velocity",
    "sim.objfloatparam_abs_y_velocity",
    "sim_objfloatparam_abs_z_velocity",
    "sim.objfloatparam_abs_z_velocity",
    "sim_objfloatparam_abs_rot_velocity",
    "sim.objfloatparam_abs_rot_velocity",
    "sim_objfloatparam_objbbox_min_x",
    "sim.objfloatparam_objbbox_min_x",
    "sim_objfloatparam_objbbox_min_y",
    "sim.objfloatparam_objbbox_min_y",
    "sim_objfloatparam_objbbox_min_z",
    "sim.objfloatparam_objbbox_min_z",
    "sim_objfloatparam_objbbox_max_x",
    "sim.objfloatparam_objbbox_max_x",
    "sim_objfloatparam_objbbox_max_y",
    "sim.objfloatparam_objbbox_max_y",
    "sim_objfloatparam_objbbox_max_z",
    "sim.objfloatparam_objbbox_max_z",
    "sim_objfloatparam_modelbbox_min_x",
    "sim.objfloatparam_modelbbox_min_x",
    "sim_objfloatparam_modelbbox_min_y",
    "sim.objfloatparam_modelbbox_min_y",
    "sim_objfloatparam_modelbbox_min_z",
    "sim.objfloatparam_modelbbox_min_z",
    "sim_objfloatparam_modelbbox_max_x",
    "sim.objfloatparam_modelbbox_max_x",
    "sim_objfloatparam_modelbbox_max_y",
    "sim.objfloatparam_modelbbox_max_y",
    "sim_objfloatparam_modelbbox_max_z",
    "sim.objfloatparam_modelbbox_max_z",
    "sim_objintparam_collection_self_collision_indicator",
    "sim.objintparam_collection_self_collision_indicator",
    "sim_objfloatparam_transparency_offset",
    "sim.objfloatparam_transparency_offset",
    "sim_objintparam_child_role",
    "sim.objintparam_child_role",
    "sim_objintparam_parent_role",
    "sim.objintparam_parent_role",
    "sim_objintparam_manipulation_permissions",
    "sim.objintparam_manipulation_permissions",
    "sim_objintparam_illumination_handle",
    "sim.objintparam_illumination_handle",
    "sim_objstringparam_dna",
    "sim.objstringparam_dna",
    "sim_visionfloatparam_near_clipping",
    "sim.visionfloatparam_near_clipping",
    "sim_visionfloatparam_far_clipping",
    "sim.visionfloatparam_far_clipping",
    "sim_visionintparam_resolution_x",
    "sim.visionintparam_resolution_x",
    "sim_visionintparam_resolution_y",
    "sim.visionintparam_resolution_y",
    "sim_visionfloatparam_perspective_angle",
    "sim.visionfloatparam_perspective_angle",
    "sim_visionfloatparam_ortho_size",
    "sim.visionfloatparam_ortho_size",
    "sim_visionintparam_disabled_light_components",
    "sim.visionintparam_disabled_light_components",
    "sim_visionintparam_rendering_attributes",
    "sim.visionintparam_rendering_attributes",
    "sim_visionintparam_entity_to_render",
    "sim.visionintparam_entity_to_render",
    "sim_visionintparam_windowed_size_x",
    "sim.visionintparam_windowed_size_x",
    "sim_visionintparam_windowed_size_y",
    "sim.visionintparam_windowed_size_y",
    "sim_visionintparam_windowed_pos_x",
    "sim.visionintparam_windowed_pos_x",
    "sim_visionintparam_windowed_pos_y",
    "sim.visionintparam_windowed_pos_y",
    "sim_visionintparam_pov_focal_blur",
    "sim.visionintparam_pov_focal_blur",
    "sim_visionfloatparam_pov_blur_distance",
    "sim.visionfloatparam_pov_blur_distance",
    "sim_visionfloatparam_pov_aperture",
    "sim.visionfloatparam_pov_aperture",
    "sim_visionintparam_pov_blur_sampled",
    "sim.visionintparam_pov_blur_sampled",
    "sim_visionintparam_render_mode",
    "sim.visionintparam_render_mode",
    "sim_visionintparam_perspective_operation",
    "sim.visionintparam_perspective_operation",
    "sim_jointintparam_motor_enabled",
    "sim.jointintparam_motor_enabled",
    "sim_jointintparam_ctrl_enabled",
    "sim.jointintparam_ctrl_enabled",
    "sim_jointfloatparam_pid_p",
    "sim.jointfloatparam_pid_p",
    "sim_jointfloatparam_pid_i",
    "sim.jointfloatparam_pid_i",
    "sim_jointfloatparam_pid_d",
    "sim.jointfloatparam_pid_d",
    "sim_jointfloatparam_intrinsic_x",
    "sim.jointfloatparam_intrinsic_x",
    "sim_jointfloatparam_intrinsic_y",
    "sim.jointfloatparam_intrinsic_y",
    "sim_jointfloatparam_intrinsic_z",
    "sim.jointfloatparam_intrinsic_z",
    "sim_jointfloatparam_intrinsic_qx",
    "sim.jointfloatparam_intrinsic_qx",
    "sim_jointfloatparam_intrinsic_qy",
    "sim.jointfloatparam_intrinsic_qy",
    "sim_jointfloatparam_intrinsic_qz",
    "sim.jointfloatparam_intrinsic_qz",
    "sim_jointfloatparam_intrinsic_qw",
    "sim.jointfloatparam_intrinsic_qw",
    "sim_jointfloatparam_velocity",
    "sim.jointfloatparam_velocity",
    "sim_jointfloatparam_spherical_qx",
    "sim.jointfloatparam_spherical_qx",
    "sim_jointfloatparam_spherical_qy",
    "sim.jointfloatparam_spherical_qy",
    "sim_jointfloatparam_spherical_qz",
    "sim.jointfloatparam_spherical_qz",
    "sim_jointfloatparam_spherical_qw",
    "sim.jointfloatparam_spherical_qw",
    "sim_jointfloatparam_upper_limit",
    "sim.jointfloatparam_upper_limit",
    "sim_jointfloatparam_kc_k",
    "sim.jointfloatparam_kc_k",
    "sim_jointfloatparam_kc_c",
    "sim.jointfloatparam_kc_c",
    "sim_jointfloatparam_ik_weight",
    "sim.jointfloatparam_ik_weight",
    "sim_jointfloatparam_error_x",
    "sim.jointfloatparam_error_x",
    "sim_jointfloatparam_error_y",
    "sim.jointfloatparam_error_y",
    "sim_jointfloatparam_error_z",
    "sim.jointfloatparam_error_z",
    "sim_jointfloatparam_error_a",
    "sim.jointfloatparam_error_a",
    "sim_jointfloatparam_error_b",
    "sim.jointfloatparam_error_b",
    "sim_jointfloatparam_error_g",
    "sim.jointfloatparam_error_g",
    "sim_jointfloatparam_error_pos",
    "sim.jointfloatparam_error_pos",
    "sim_jointfloatparam_error_angle",
    "sim.jointfloatparam_error_angle",
    "sim_jointintparam_velocity_lock",
    "sim.jointintparam_velocity_lock",
    "sim_jointintparam_vortex_dep_handle",
    "sim.jointintparam_vortex_dep_handle",
    "sim_jointfloatparam_vortex_dep_multiplication",
    "sim.jointfloatparam_vortex_dep_multiplication",
    "sim_jointfloatparam_vortex_dep_offset",
    "sim.jointfloatparam_vortex_dep_offset",
    "sim_shapefloatparam_init_velocity_x",
    "sim.shapefloatparam_init_velocity_x",
    "sim_shapefloatparam_init_velocity_y",
    "sim.shapefloatparam_init_velocity_y",
    "sim_shapefloatparam_init_velocity_z",
    "sim.shapefloatparam_init_velocity_z",
    "sim_shapeintparam_static",
    "sim.shapeintparam_static",
    "sim_shapeintparam_respondable",
    "sim.shapeintparam_respondable",
    "sim_shapefloatparam_mass",
    "sim.shapefloatparam_mass",
    "sim_shapefloatparam_texture_x",
    "sim.shapefloatparam_texture_x",
    "sim_shapefloatparam_texture_y",
    "sim.shapefloatparam_texture_y",
    "sim_shapefloatparam_texture_z",
    "sim.shapefloatparam_texture_z",
    "sim_shapefloatparam_texture_a",
    "sim.shapefloatparam_texture_a",
    "sim_shapefloatparam_texture_b",
    "sim.shapefloatparam_texture_b",
    "sim_shapefloatparam_texture_g",
    "sim.shapefloatparam_texture_g",
    "sim_shapefloatparam_texture_scaling_x",
    "sim.shapefloatparam_texture_scaling_x",
    "sim_shapefloatparam_texture_scaling_y",
    "sim.shapefloatparam_texture_scaling_y",
    "sim_shapeintparam_culling",
    "sim.shapeintparam_culling",
    "sim_shapeintparam_wireframe",
    "sim.shapeintparam_wireframe",
    "sim_shapeintparam_compound",
    "sim.shapeintparam_compound",
    "sim_shapeintparam_convex",
    "sim.shapeintparam_convex",
    "sim_shapeintparam_convex_check",
    "sim.shapeintparam_convex_check",
    "sim_shapeintparam_respondable_mask",
    "sim.shapeintparam_respondable_mask",
    "sim_shapefloatparam_init_velocity_a",
    "sim.shapefloatparam_init_ang_velocity_x",
    "sim_shapefloatparam_init_velocity_b",
    "sim.shapefloatparam_init_ang_velocity_y",
    "sim_shapefloatparam_init_velocity_g",
    "sim.shapefloatparam_init_ang_velocity_z",
    "sim_shapestringparam_color_name",
    "sim.shapestringparam_color_name",
    "sim_shapeintparam_edge_visibility",
    "sim.shapeintparam_edge_visibility",
    "sim_shapefloatparam_shading_angle",
    "sim.shapefloatparam_shading_angle",
    "sim_shapefloatparam_edge_angle",
    "sim.shapefloatparam_edge_angle",
    "sim_shapeintparam_edge_borders_hidden",
    "sim.shapeintparam_edge_borders_hidden",
    "sim_proxintparam_ray_invisibility",
    "sim.proxintparam_ray_invisibility",
    "sim_proxintparam_volume_type",
    "sim.proxintparam_volume_type",
    "sim_proxintparam_entity_to_detect",
    "sim.proxintparam_entity_to_detect",
    "sim_forcefloatparam_error_x",
    "sim.forcefloatparam_error_x",
    "sim_forcefloatparam_error_y",
    "sim.forcefloatparam_error_y",
    "sim_forcefloatparam_error_z",
    "sim.forcefloatparam_error_z",
    "sim_forcefloatparam_error_a",
    "sim.forcefloatparam_error_a",
    "sim_forcefloatparam_error_b",
    "sim.forcefloatparam_error_b",
    "sim_forcefloatparam_error_g",
    "sim.forcefloatparam_error_g",
    "sim_forcefloatparam_error_pos",
    "sim.forcefloatparam_error_pos",
    "sim_forcefloatparam_error_angle",
    "sim.forcefloatparam_error_angle",
    "sim_lightintparam_pov_casts_shadows",
    "sim.lightintparam_pov_casts_shadows",
    "sim_cameraintparam_disabled_light_components",
    "sim.cameraintparam_disabled_light_components",
    "sim_camerafloatparam_perspective_angle",
    "sim.camerafloatparam_perspective_angle",
    "sim_camerafloatparam_ortho_size",
    "sim.camerafloatparam_ortho_size",
    "sim_cameraintparam_rendering_attributes",
    "sim.cameraintparam_rendering_attributes",
    "sim_cameraintparam_pov_focal_blur",
    "sim.cameraintparam_pov_focal_blur",
    "sim_camerafloatparam_pov_blur_distance",
    "sim.camerafloatparam_pov_blur_distance",
    "sim_camerafloatparam_pov_aperture",
    "sim.camerafloatparam_pov_aperture",
    "sim_cameraintparam_pov_blur_samples",
    "sim.cameraintparam_pov_blur_samples",
    "sim_dummyintparam_link_type",
    "sim.dummyintparam_dummytype",
    "sim_dummyintparam_follow_path",
    "sim.dummyintparam_follow_path",
    "sim_dummyfloatparam_follow_path_offset",
    "sim.dummyfloatparam_follow_path_offset",
    "sim_millintparam_volume_type",
    "sim.millintparam_volume_type",
    "sim_mirrorfloatparam_width",
    "sim.mirrorfloatparam_width",
    "sim_mirrorfloatparam_height",
    "sim.mirrorfloatparam_height",
    "sim_mirrorfloatparam_reflectance",
    "sim.mirrorfloatparam_reflectance",
    "sim_mirrorintparam_enable",
    "sim.mirrorintparam_enable",
    "sim_bullet_global_stepsize",
    "sim.bullet_global_stepsize",
    "sim_bullet_global_internalscalingfactor",
    "sim.bullet_global_internalscalingfactor",
    "sim_bullet_global_collisionmarginfactor",
    "sim.bullet_global_collisionmarginfactor",
    "sim_bullet_global_constraintsolvingiterations",
    "sim.bullet_global_constraintsolvingiterations",
    "sim_bullet_global_bitcoded",
    "sim.bullet_global_bitcoded",
    "sim_bullet_global_constraintsolvertype",
    "sim.bullet_global_constraintsolvertype",
    "sim_bullet_global_fullinternalscaling",
    "sim.bullet_global_fullinternalscaling",
    "sim_bullet_joint_stoperp",
    "sim.bullet_joint_stoperp",
    "sim_bullet_joint_stopcfm",
    "sim.bullet_joint_stopcfm",
    "sim_bullet_joint_normalcfm",
    "sim.bullet_joint_normalcfm",
    "sim_bullet_body_restitution",
    "sim.bullet_body_restitution",
    "sim_bullet_body_oldfriction",
    "sim.bullet_body_oldfriction",
    "sim_bullet_body_friction",
    "sim.bullet_body_friction",
    "sim_bullet_body_lineardamping",
    "sim.bullet_body_lineardamping",
    "sim_bullet_body_angulardamping",
    "sim.bullet_body_angulardamping",
    "sim_bullet_body_nondefaultcollisionmargingfactor",
    "sim.bullet_body_nondefaultcollisionmargingfactor",
    "sim_bullet_body_nondefaultcollisionmargingfactorconvex",
    "sim.bullet_body_nondefaultcollisionmargingfactorconvex",
    "sim_bullet_body_bitcoded",
    "sim.bullet_body_bitcoded",
    "sim_bullet_body_sticky",
    "sim.bullet_body_sticky",
    "sim_bullet_body_usenondefaultcollisionmargin",
    "sim.bullet_body_usenondefaultcollisionmargin",
    "sim_bullet_body_usenondefaultcollisionmarginconvex",
    "sim.bullet_body_usenondefaultcollisionmarginconvex",
    "sim_bullet_body_autoshrinkconvex",
    "sim.bullet_body_autoshrinkconvex",
    "sim_ode_global_stepsize",
    "sim.ode_global_stepsize",
    "sim_ode_global_internalscalingfactor",
    "sim.ode_global_internalscalingfactor",
    "sim_ode_global_cfm",
    "sim.ode_global_cfm",
    "sim_ode_global_erp",
    "sim.ode_global_erp",
    "sim_ode_global_constraintsolvingiterations",
    "sim.ode_global_constraintsolvingiterations",
    "sim_ode_global_bitcoded",
    "sim.ode_global_bitcoded",
    "sim_ode_global_randomseed",
    "sim.ode_global_randomseed",
    "sim_ode_global_fullinternalscaling",
    "sim.ode_global_fullinternalscaling",
    "sim_ode_global_quickstep",
    "sim.ode_global_quickstep",
    "sim_ode_joint_stoperp",
    "sim.ode_joint_stoperp",
    "sim_ode_joint_stopcfm",
    "sim.ode_joint_stopcfm",
    "sim_ode_joint_bounce",
    "sim.ode_joint_bounce",
    "sim_ode_joint_fudgefactor",
    "sim.ode_joint_fudgefactor",
    "sim_ode_joint_normalcfm",
    "sim.ode_joint_normalcfm",
    "sim_ode_body_friction",
    "sim.ode_body_friction",
    "sim_ode_body_softerp",
    "sim.ode_body_softerp",
    "sim_ode_body_softcfm",
    "sim.ode_body_softcfm",
    "sim_ode_body_lineardamping",
    "sim.ode_body_lineardamping",
    "sim_ode_body_angulardamping",
    "sim.ode_body_angulardamping",
    "sim_ode_body_maxcontacts",
    "sim.ode_body_maxcontacts",
    "sim_vortex_global_stepsize",
    "sim.vortex_global_stepsize",
    "sim_vortex_global_internalscalingfactor",
    "sim.vortex_global_internalscalingfactor",
    "sim_vortex_global_contacttolerance",
    "sim.vortex_global_contacttolerance",
    "sim_vortex_global_constraintlinearcompliance",
    "sim.vortex_global_constraintlinearcompliance",
    "sim_vortex_global_constraintlineardamping",
    "sim.vortex_global_constraintlineardamping",
    "sim_vortex_global_constraintlinearkineticloss",
    "sim.vortex_global_constraintlinearkineticloss",
    "sim_vortex_global_constraintangularcompliance",
    "sim.vortex_global_constraintangularcompliance",
    "sim_vortex_global_constraintangulardamping",
    "sim.vortex_global_constraintangulardamping",
    "sim_vortex_global_constraintangularkineticloss",
    "sim.vortex_global_constraintangularkineticloss",
    "sim_vortex_global_bitcoded",
    "sim.vortex_global_bitcoded",
    "sim_vortex_global_autosleep",
    "sim.vortex_global_autosleep",
    "sim_vortex_global_multithreading",
    "sim.vortex_global_multithreading",
    "sim_vortex_joint_lowerlimitdamping",
    "sim.vortex_joint_lowerlimitdamping",
    "sim_vortex_joint_upperlimitdamping",
    "sim.vortex_joint_upperlimitdamping",
    "sim_vortex_joint_lowerlimitstiffness",
    "sim.vortex_joint_lowerlimitstiffness",
    "sim_vortex_joint_upperlimitstiffness",
    "sim.vortex_joint_upperlimitstiffness",
    "sim_vortex_joint_lowerlimitrestitution",
    "sim.vortex_joint_lowerlimitrestitution",
    "sim_vortex_joint_upperlimitrestitution",
    "sim.vortex_joint_upperlimitrestitution",
    "sim_vortex_joint_lowerlimitmaxforce",
    "sim.vortex_joint_lowerlimitmaxforce",
    "sim_vortex_joint_upperlimitmaxforce",
    "sim.vortex_joint_upperlimitmaxforce",
    "sim_vortex_joint_motorconstraintfrictioncoeff",
    "sim.vortex_joint_motorconstraintfrictioncoeff",
    "sim_vortex_joint_motorconstraintfrictionmaxforce",
    "sim.vortex_joint_motorconstraintfrictionmaxforce",
    "sim_vortex_joint_motorconstraintfrictionloss",
    "sim.vortex_joint_motorconstraintfrictionloss",
    "sim_vortex_joint_p0loss",
    "sim.vortex_joint_p0loss",
    "sim_vortex_joint_p0stiffness",
    "sim.vortex_joint_p0stiffness",
    "sim_vortex_joint_p0damping",
    "sim.vortex_joint_p0damping",
    "sim_vortex_joint_p0frictioncoeff",
    "sim.vortex_joint_p0frictioncoeff",
    "sim_vortex_joint_p0frictionmaxforce",
    "sim.vortex_joint_p0frictionmaxforce",
    "sim_vortex_joint_p0frictionloss",
    "sim.vortex_joint_p0frictionloss",
    "sim_vortex_joint_p1loss",
    "sim.vortex_joint_p1loss",
    "sim_vortex_joint_p1stiffness",
    "sim.vortex_joint_p1stiffness",
    "sim_vortex_joint_p1damping",
    "sim.vortex_joint_p1damping",
    "sim_vortex_joint_p1frictioncoeff",
    "sim.vortex_joint_p1frictioncoeff",
    "sim_vortex_joint_p1frictionmaxforce",
    "sim.vortex_joint_p1frictionmaxforce",
    "sim_vortex_joint_p1frictionloss",
    "sim.vortex_joint_p1frictionloss",
    "sim_vortex_joint_p2loss",
    "sim.vortex_joint_p2loss",
    "sim_vortex_joint_p2stiffness",
    "sim.vortex_joint_p2stiffness",
    "sim_vortex_joint_p2damping",
    "sim.vortex_joint_p2damping",
    "sim_vortex_joint_p2frictioncoeff",
    "sim.vortex_joint_p2frictioncoeff",
    "sim_vortex_joint_p2frictionmaxforce",
    "sim.vortex_joint_p2frictionmaxforce",
    "sim_vortex_joint_p2frictionloss",
    "sim.vortex_joint_p2frictionloss",
    "sim_vortex_joint_a0loss",
    "sim.vortex_joint_a0loss",
    "sim_vortex_joint_a0stiffness",
    "sim.vortex_joint_a0stiffness",
    "sim_vortex_joint_a0damping",
    "sim.vortex_joint_a0damping",
    "sim_vortex_joint_a0frictioncoeff",
    "sim.vortex_joint_a0frictioncoeff",
    "sim_vortex_joint_a0frictionmaxforce",
    "sim.vortex_joint_a0frictionmaxforce",
    "sim_vortex_joint_a0frictionloss",
    "sim.vortex_joint_a0frictionloss",
    "sim_vortex_joint_a1loss",
    "sim.vortex_joint_a1loss",
    "sim_vortex_joint_a1stiffness",
    "sim.vortex_joint_a1stiffness",
    "sim_vortex_joint_a1damping",
    "sim.vortex_joint_a1damping",
    "sim_vortex_joint_a1frictioncoeff",
    "sim.vortex_joint_a1frictioncoeff",
    "sim_vortex_joint_a1frictionmaxforce",
    "sim.vortex_joint_a1frictionmaxforce",
    "sim_vortex_joint_a1frictionloss",
    "sim.vortex_joint_a1frictionloss",
    "sim_vortex_joint_a2loss",
    "sim.vortex_joint_a2loss",
    "sim_vortex_joint_a2stiffness",
    "sim.vortex_joint_a2stiffness",
    "sim_vortex_joint_a2damping",
    "sim.vortex_joint_a2damping",
    "sim_vortex_joint_a2frictioncoeff",
    "sim.vortex_joint_a2frictioncoeff",
    "sim_vortex_joint_a2frictionmaxforce",
    "sim.vortex_joint_a2frictionmaxforce",
    "sim_vortex_joint_a2frictionloss",
    "sim.vortex_joint_a2frictionloss",
    "sim_vortex_joint_dependencyfactor",
    "sim.vortex_joint_dependencyfactor",
    "sim_vortex_joint_dependencyoffset",
    "sim.vortex_joint_dependencyoffset",
    "sim_vortex_joint_bitcoded",
    "sim.vortex_joint_bitcoded",
    "sim_vortex_joint_relaxationenabledbc",
    "sim.vortex_joint_relaxationenabledbc",
    "sim_vortex_joint_frictionenabledbc",
    "sim.vortex_joint_frictionenabledbc",
    "sim_vortex_joint_frictionproportionalbc",
    "sim.vortex_joint_frictionproportionalbc",
    "sim_vortex_joint_objectid",
    "sim.vortex_joint_objectid",
    "sim_vortex_joint_dependentobjectid",
    "sim.vortex_joint_dependentobjectid",
    "sim_vortex_joint_motorfrictionenabled",
    "sim.vortex_joint_motorfrictionenabled",
    "sim_vortex_joint_proportionalmotorfriction",
    "sim.vortex_joint_proportionalmotorfriction",
    "sim_vortex_body_primlinearaxisfriction",
    "sim.vortex_body_primlinearaxisfriction",
    "sim_vortex_body_seclinearaxisfriction",
    "sim.vortex_body_seclinearaxisfriction",
    "sim_vortex_body_primangularaxisfriction",
    "sim.vortex_body_primangularaxisfriction",
    "sim_vortex_body_secangularaxisfriction",
    "sim.vortex_body_secangularaxisfriction",
    "sim_vortex_body_normalangularaxisfriction",
    "sim.vortex_body_normalangularaxisfriction",
    "sim_vortex_body_primlinearaxisstaticfrictionscale",
    "sim.vortex_body_primlinearaxisstaticfrictionscale",
    "sim_vortex_body_seclinearaxisstaticfrictionscale",
    "sim.vortex_body_seclinearaxisstaticfrictionscale",
    "sim_vortex_body_primangularaxisstaticfrictionscale",
    "sim.vortex_body_primangularaxisstaticfrictionscale",
    "sim_vortex_body_secangularaxisstaticfrictionscale",
    "sim.vortex_body_secangularaxisstaticfrictionscale",
    "sim_vortex_body_normalangularaxisstaticfrictionscale",
    "sim.vortex_body_normalangularaxisstaticfrictionscale",
    "sim_vortex_body_compliance",
    "sim.vortex_body_compliance",
    "sim_vortex_body_damping",
    "sim.vortex_body_damping",
    "sim_vortex_body_restitution",
    "sim.vortex_body_restitution",
    "sim_vortex_body_restitutionthreshold",
    "sim.vortex_body_restitutionthreshold",
    "sim_vortex_body_adhesiveforce",
    "sim.vortex_body_adhesiveforce",
    "sim_vortex_body_linearvelocitydamping",
    "sim.vortex_body_linearvelocitydamping",
    "sim_vortex_body_angularvelocitydamping",
    "sim.vortex_body_angularvelocitydamping",
    "sim_vortex_body_primlinearaxisslide",
    "sim.vortex_body_primlinearaxisslide",
    "sim_vortex_body_seclinearaxisslide",
    "sim.vortex_body_seclinearaxisslide",
    "sim_vortex_body_primangularaxisslide",
    "sim.vortex_body_primangularaxisslide",
    "sim_vortex_body_secangularaxisslide",
    "sim.vortex_body_secangularaxisslide",
    "sim_vortex_body_normalangularaxisslide",
    "sim.vortex_body_normalangularaxisslide",
    "sim_vortex_body_primlinearaxisslip",
    "sim.vortex_body_primlinearaxisslip",
    "sim_vortex_body_seclinearaxisslip",
    "sim.vortex_body_seclinearaxisslip",
    "sim_vortex_body_primangularaxisslip",
    "sim.vortex_body_primangularaxisslip",
    "sim_vortex_body_secangularaxisslip",
    "sim.vortex_body_secangularaxisslip",
    "sim_vortex_body_normalangularaxisslip",
    "sim.vortex_body_normalangularaxisslip",
    "sim_vortex_body_autosleeplinearspeedthreshold",
    "sim.vortex_body_autosleeplinearspeedthreshold",
    "sim_vortex_body_autosleeplinearaccelthreshold",
    "sim.vortex_body_autosleeplinearaccelthreshold",
    "sim_vortex_body_autosleepangularspeedthreshold",
    "sim.vortex_body_autosleepangularspeedthreshold",
    "sim_vortex_body_autosleepangularaccelthreshold",
    "sim.vortex_body_autosleepangularaccelthreshold",
    "sim_vortex_body_skinthickness",
    "sim.vortex_body_skinthickness",
    "sim_vortex_body_autoangulardampingtensionratio",
    "sim.vortex_body_autoangulardampingtensionratio",
    "sim_vortex_body_primaxisvectorx",
    "sim.vortex_body_primaxisvectorx",
    "sim_vortex_body_primaxisvectory",
    "sim.vortex_body_primaxisvectory",
    "sim_vortex_body_primaxisvectorz",
    "sim.vortex_body_primaxisvectorz",
    "sim_vortex_body_primlinearaxisfrictionmodel",
    "sim.vortex_body_primlinearaxisfrictionmodel",
    "sim_vortex_body_seclinearaxisfrictionmodel",
    "sim.vortex_body_seclinearaxisfrictionmodel",
    "sim_vortex_body_primangulararaxisfrictionmodel",
    "sim.vortex_body_primangulararaxisfrictionmodel",
    "sim_vortex_body_secmangulararaxisfrictionmodel",
    "sim.vortex_body_secmangulararaxisfrictionmodel",
    "sim_vortex_body_normalmangulararaxisfrictionmodel",
    "sim.vortex_body_normalmangulararaxisfrictionmodel",
    "sim_vortex_body_bitcoded",
    "sim.vortex_body_bitcoded",
    "sim_vortex_body_autosleepsteplivethreshold",
    "sim.vortex_body_autosleepsteplivethreshold",
    "sim_vortex_body_materialuniqueid",
    "sim.vortex_body_materialuniqueid",
    "sim_vortex_body_pureshapesasconvex",
    "sim.vortex_body_pureshapesasconvex",
    "sim_vortex_body_convexshapesasrandom",
    "sim.vortex_body_convexshapesasrandom",
    "sim_vortex_body_randomshapesasterrain",
    "sim.vortex_body_randomshapesasterrain",
    "sim_vortex_body_fastmoving",
    "sim.vortex_body_fastmoving",
    "sim_vortex_body_autoslip",
    "sim.vortex_body_autoslip",
    "sim_vortex_body_seclinaxissameasprimlinaxis",
    "sim.vortex_body_seclinaxissameasprimlinaxis",
    "sim_vortex_body_secangaxissameasprimangaxis",
    "sim.vortex_body_secangaxissameasprimangaxis",
    "sim_vortex_body_normangaxissameasprimangaxis",
    "sim.vortex_body_normangaxissameasprimangaxis",
    "sim_vortex_body_autoangulardamping",
    "sim.vortex_body_autoangulardamping",
    "sim_newton_global_stepsize",
    "sim.newton_global_stepsize",
    "sim_newton_global_contactmergetolerance",
    "sim.newton_global_contactmergetolerance",
    "sim_newton_global_constraintsolvingiterations",
    "sim.newton_global_constraintsolvingiterations",
    "sim_newton_global_bitcoded",
    "sim.newton_global_bitcoded",
    "sim_newton_global_multithreading",
    "sim.newton_global_multithreading",
    "sim_newton_global_exactsolver",
    "sim.newton_global_exactsolver",
    "sim_newton_global_highjointaccuracy",
    "sim.newton_global_highjointaccuracy",
    "sim_newton_joint_dependencyfactor",
    "sim.newton_joint_dependencyfactor",
    "sim_newton_joint_dependencyoffset",
    "sim.newton_joint_dependencyoffset",
    "sim_newton_joint_objectid",
    "sim.newton_joint_objectid",
    "sim_newton_joint_dependentobjectid",
    "sim.newton_joint_dependentobjectid",
    "sim_newton_body_staticfriction",
    "sim.newton_body_staticfriction",
    "sim_newton_body_kineticfriction",
    "sim.newton_body_kineticfriction",
    "sim_newton_body_restitution",
    "sim.newton_body_restitution",
    "sim_newton_body_lineardrag",
    "sim.newton_body_lineardrag",
    "sim_newton_body_angulardrag",
    "sim.newton_body_angulardrag",
    "sim_newton_body_bitcoded",
    "sim.newton_body_bitcoded",
    "sim_newton_body_fastmoving",
    "sim.newton_body_fastmoving",
    "sim_vortex_bodyfrictionmodel_box",
    "sim.vortex_bodyfrictionmodel_box",
    "sim_vortex_bodyfrictionmodel_scaledbox",
    "sim.vortex_bodyfrictionmodel_scaledbox",
    "sim_vortex_bodyfrictionmodel_proplow",
    "sim.vortex_bodyfrictionmodel_proplow",
    "sim_vortex_bodyfrictionmodel_prophigh",
    "sim.vortex_bodyfrictionmodel_prophigh",
    "sim_vortex_bodyfrictionmodel_scaledboxfast",
    "sim.vortex_bodyfrictionmodel_scaledboxfast",
    "sim_vortex_bodyfrictionmodel_neutral",
    "sim.vortex_bodyfrictionmodel_neutral",
    "sim_vortex_bodyfrictionmodel_none",
    "sim.vortex_bodyfrictionmodel_none",
    "sim_bullet_constraintsolvertype_sequentialimpulse",
    "sim.bullet_constraintsolvertype_sequentialimpulse",
    "sim_bullet_constraintsolvertype_nncg",
    "sim.bullet_constraintsolvertype_nncg",
    "sim_bullet_constraintsolvertype_dantzig",
    "sim.bullet_constraintsolvertype_dantzig",
    "sim_bullet_constraintsolvertype_projectedgaussseidel",
    "sim.bullet_constraintsolvertype_projectedgaussseidel",
    "sim_filtercomponent_originalimage",
    "sim.filtercomponent_originalimage",
    "sim_filtercomponent_originaldepth",
    "sim.filtercomponent_originaldepth",
    "sim_filtercomponent_uniformimage",
    "sim.filtercomponent_uniformimage",
    "sim_filtercomponent_tooutput",
    "sim.filtercomponent_tooutput",
    "sim_filtercomponent_tobuffer1",
    "sim.filtercomponent_tobuffer1",
    "sim_filtercomponent_tobuffer2",
    "sim.filtercomponent_tobuffer2",
    "sim_filtercomponent_frombuffer1",
    "sim.filtercomponent_frombuffer1",
    "sim_filtercomponent_frombuffer2",
    "sim.filtercomponent_frombuffer2",
    "sim_filtercomponent_swapbuffers",
    "sim.filtercomponent_swapbuffers",
    "sim_filtercomponent_addbuffer1",
    "sim.filtercomponent_addbuffer1",
    "sim_filtercomponent_subtractbuffer1",
    "sim.filtercomponent_subtractbuffer1",
    "sim_filtercomponent_multiplywithbuffer1",
    "sim.filtercomponent_multiplywithbuffer1",
    "sim_filtercomponent_horizontalflip",
    "sim.filtercomponent_horizontalflip",
    "sim_filtercomponent_verticalflip",
    "sim.filtercomponent_verticalflip",
    "sim_filtercomponent_rotate",
    "sim.filtercomponent_rotate",
    "sim_filtercomponent_shift",
    "sim.filtercomponent_shift",
    "sim_filtercomponent_resize",
    "sim.filtercomponent_resize",
    "sim_filtercomponent_3x3filter",
    "sim.filtercomponent_3x3filter",
    "sim_filtercomponent_5x5filter",
    "sim.filtercomponent_5x5filter",
    "sim_filtercomponent_sharpen",
    "sim.filtercomponent_sharpen",
    "sim_filtercomponent_edge",
    "sim.filtercomponent_edge",
    "sim_filtercomponent_rectangularcut",
    "sim.filtercomponent_rectangularcut",
    "sim_filtercomponent_circularcut",
    "sim.filtercomponent_circularcut",
    "sim_filtercomponent_normalize",
    "sim.filtercomponent_normalize",
    "sim_filtercomponent_intensityscale",
    "sim.filtercomponent_intensityscale",
    "sim_filtercomponent_keeporremovecolors",
    "sim.filtercomponent_keeporremovecolors",
    "sim_filtercomponent_scaleandoffsetcolors",
    "sim.filtercomponent_scaleandoffsetcolors",
    "sim_filtercomponent_binary",
    "sim.filtercomponent_binary",
    "sim_filtercomponent_swapwithbuffer1",
    "sim.filtercomponent_swapwithbuffer1",
    "sim_filtercomponent_addtobuffer1",
    "sim.filtercomponent_addtobuffer1",
    "sim_filtercomponent_subtractfrombuffer1",
    "sim.filtercomponent_subtractfrombuffer1",
    "sim_filtercomponent_correlationwithbuffer1",
    "sim.filtercomponent_correlationwithbuffer1",
    "sim_filtercomponent_colorsegmentation",
    "sim.filtercomponent_colorsegmentation",
    "sim_filtercomponent_blobextraction",
    "sim.filtercomponent_blobextraction",
    "sim_filtercomponent_imagetocoord",
    "sim.filtercomponent_imagetocoord",
    "sim_filtercomponent_pixelchange",
    "sim.filtercomponent_pixelchange",
    "sim_filtercomponent_velodyne",
    "sim.filtercomponent_velodyne",
    "sim_filtercomponent_todepthoutput",
    "sim.filtercomponent_todepthoutput",
    "sim_filtercomponent_customized",
    "sim.filtercomponent_customized",
    "sim_buffer_uint8",
    "sim.buffer_uint8",
    "sim_buffer_int8",
    "sim.buffer_int8",
    "sim_buffer_uint16",
    "sim.buffer_uint16",
    "sim_buffer_int16",
    "sim.buffer_int16",
    "sim_buffer_uint32",
    "sim.buffer_uint32",
    "sim_buffer_int32",
    "sim.buffer_int32",
    "sim_buffer_float",
    "sim.buffer_float",
    "sim_buffer_double",
    "sim.buffer_double",
    "sim_buffer_uint8rgb",
    "sim.buffer_uint8rgb",
    "sim_buffer_uint8bgr",
    "sim.buffer_uint8bgr",
    "sim_imgcomb_vertical",
    "sim.imgcomb_vertical",
    "sim_imgcomb_horizontal",
    "sim.imgcomb_horizontal",
    "sim_dynmat_default",
    "sim.dynmat_default",
    "sim_dynmat_highfriction",
    "sim.dynmat_highfriction",
    "sim_dynmat_lowfriction",
    "sim.dynmat_lowfriction",
    "sim_dynmat_nofriction",
    "sim.dynmat_nofriction",
    "sim_dynmat_reststackgrasp",
    "sim.dynmat_reststackgrasp",
    "sim_dynmat_foot",
    "sim.dynmat_foot",
    "sim_dynmat_wheel",
    "sim.dynmat_wheel",
    "sim_dynmat_gripper",
    "sim.dynmat_gripper",
    "sim_dynmat_floor",
    "sim.dynmat_floor",
    "simrml_phase_sync_if_possible",
    "sim.rml_phase_sync_if_possible",
    "simrml_only_time_sync",
    "sim.rml_only_time_sync",
    "simrml_only_phase_sync",
    "sim.rml_only_phase_sync",
    "simrml_no_sync",
    "sim.rml_no_sync",
    "simrml_disable_extremum_motion_states_calc",
    "sim.rml_disable_extremum_motion_states_calc",
    "simrml_keep_target_vel",
    "sim.rml_keep_target_vel",
    "simrml_recompute_trajectory",
    "sim.rml_recompute_trajectory",
    "simrml_keep_current_vel_if_fallback_strategy",
    "sim.rml_keep_current_vel_if_fallback_strategy",
    "",
    "",
};
const SNewApiMapping _simBubbleApiMapping[] = {
    "simExtBubble_create",
    "simBubble.create",
    "simExtBubble_destroy",
    "simBubble.destroy",
    "simExtBubble_start",
    "simBubble.start",
    "simExtBubble_stop",
    "simBubble.stop",
    "",
    "",
};
const SNewApiMapping _simK3ApiMapping[] = {
    "simExtK3_create",
    "simK3.create",
    "simExtK3_destroy",
    "simK3.destroy",
    "simExtK3_getInfrared",
    "simK3.getInfrared",
    "simExtK3_getUltrasonic",
    "simK3.getUltrasonic",
    "simExtK3_getLineSensor",
    "simK3.getLineSensor",
    "simExtK3_getEncoder",
    "simK3.getEncoder",
    "simExtK3_getGripperProxSensor",
    "simK3.getGripperProxSensor",
    "simExtK3_setArmPosition",
    "simK3.setArmPosition",
    "simExtK3_setGripperGap",
    "simK3.setGripperGap",
    "simExtK3_setVelocity",
    "simK3.setVelocity",
    "simExtK3_setEncoders",
    "simK3.setEncoders",
    "",
    "",
};
const SNewApiMapping _simMTBApiMapping[] = {
    "simExtMtb_startServer",
    "simMTB.startServer",
    "simExtMtb_stopServer",
    "simMTB.stopServer",
    "simExtMtb_disconnectInput",
    "simMTB.disconnectInput",
    "simExtMtb_connectInput",
    "simMTB.connectInput",
    "simExtMtb_setInput",
    "simMTB.setInput",
    "simExtMtb_getInput",
    "simMTB.getInput",
    "simExtMtb_getOutput",
    "simMTB.getOutput",
    "simExtMtb_getJoints",
    "simMTB.getJoints",
    "simExtMtb_step",
    "simMTB.step",
    "",
    "",
};
const SNewApiMapping _simOpenMeshApiMapping[] = {
    "simExtOpenMesh_getDecimated",
    "simOpenMesh.getDecimated",
    "",
    "",
};
const SNewApiMapping _simSkeletonApiMapping[] = {
    "simExtPluginSkeleton_getData",
    "simSkeleton.getData",
    "",
    "",
};
const SNewApiMapping _simQHullApiMapping[] = {
    "simExtQhull_compute",
    "simQHull.compute",
    "",
    "",
};
const SNewApiMapping _simRemoteApiApiMapping[] = {
    "simExtRemoteApiStart",
    "simRemoteApi.start",
    "simExtRemoteApiStop",
    "simRemoteApi.stop",
    "simExtRemoteApiReset",
    "simRemoteApi.reset",
    "simExtRemoteApiStatus",
    "simRemoteApi.status",
    "",
    "",
};
const SNewApiMapping _simRRS1ApiMapping[] = {
    "simExtRRS1_startRcsServer",
    "simRRS1.startRcsServer",
    "simExtRRS1_selectRcsServer",
    "simRRS1.selectRcsServer",
    "simExtRRS1_stopRcsServer",
    "simRRS1.stopRcsServer",
    "simExtRRS1_INITIALIZE",
    "simRRS1.INITIALIZE",
    "simExtRRS1_RESET",
    "simRRS1.RESET",
    "simExtRRS1_TERMINATE",
    "simRRS1.TERMINATE",
    "simExtRRS1_GET_ROBOT_STAMP",
    "simRRS1.GET_ROBOT_STAMP",
    "simExtRRS1_GET_HOME_JOINT_POSITION",
    "simRRS1.GET_HOME_JOINT_POSITION",
    "simExtRRS1_GET_RCS_DATA",
    "simRRS1.GET_RCS_DATA",
    "simExtRRS1_MODIFY_RCS_DATA",
    "simRRS1.MODIFY_RCS_DATA",
    "simExtRRS1_SAVE_RCS_DATA",
    "simRRS1.SAVE_RCS_DATA",
    "simExtRRS1_LOAD_RCS_DATA",
    "simRRS1.LOAD_RCS_DATA",
    "simExtRRS1_GET_INVERSE_KINEMATIC",
    "simRRS1.GET_INVERSE_KINEMATIC",
    "simExtRRS1_GET_FORWARD_KINEMATIC",
    "simRRS1.GET_FORWARD_KINEMATIC",
    "simExtRRS1_MATRIX_TO_CONTROLLER_POSITION",
    "simRRS1.MATRIX_TO_CONTROLLER_POSITION",
    "simExtRRS1_CONTROLLER_POSITION_TO_MATRIX",
    "simRRS1.CONTROLLER_POSITION_TO_MATRIX",
    "simExtRRS1_GET_CELL_FRAME",
    "simRRS1.GET_CELL_FRAME",
    "simExtRRS1_MODIFY_CELL_FRAME",
    "simRRS1.MODIFY_CELL_FRAME",
    "simExtRRS1_SELECT_WORK_FRAMES",
    "simRRS1.SELECT_WORK_FRAMES",
    "simExtRRS1_SET_INITIAL_POSITION",
    "simRRS1.SET_INITIAL_POSITION",
    "simExtRRS1_SET_NEXT_TARGET",
    "simRRS1.SET_NEXT_TARGET",
    "simExtRRS1_GET_NEXT_STEP",
    "simRRS1.GET_NEXT_STEP",
    "simExtRRS1_SET_INTERPOLATION_TIME",
    "simRRS1.SET_INTERPOLATION_TIME",
    "simExtRRS1_SELECT_MOTION_TYPE",
    "simRRS1.SELECT_MOTION_TYPE",
    "simExtRRS1_SELECT_TARGET_TYPE",
    "simRRS1.SELECT_TARGET_TYPE",
    "simExtRRS1_SELECT_TRAJECTORY_MODE",
    "simRRS1.SELECT_TRAJECTORY_MODE",
    "simExtRRS1_SELECT_ORIENTATION_INTERPOLATION_MODE",
    "simRRS1.SELECT_ORIENTATION_INTERPOLATION_MODE",
    "simExtRRS1_SELECT_DOMINANT_INTERPOLATION",
    "simRRS1.SELECT_DOMINANT_INTERPOLATION",
    "simExtRRS1_SET_ADVANCE_MOTION",
    "simRRS1.SET_ADVANCE_MOTION",
    "simExtRRS1_SET_MOTION_FILTER",
    "simRRS1.SET_MOTION_FILTER",
    "simExtRRS1_SET_OVERRIDE_POSITION",
    "simRRS1.SET_OVERRIDE_POSITION",
    "simExtRRS1_REVERSE_MOTION",
    "simRRS1.REVERSE_MOTION",
    "simExtRRS1_SET_PAYLOAD_PARAMETER",
    "simRRS1.SET_PAYLOAD_PARAMETER",
    "simExtRRS1_SELECT_TIME_COMPENSATION",
    "simRRS1.SELECT_TIME_COMPENSATION",
    "simExtRRS1_SET_CONFIGURATION_CONTROL",
    "simRRS1.SET_CONFIGURATION_CONTROL",
    "simExtRRS1_SET_JOINT_SPEEDS",
    "simRRS1.SET_JOINT_SPEEDS",
    "simExtRRS1_SET_CARTESIAN_POSITION_SPEED",
    "simRRS1.SET_CARTESIAN_POSITION_SPEED",
    "simExtRRS1_SET_CARTESIAN_ORIENTATION_SPEED",
    "simRRS1.SET_CARTESIAN_ORIENTATION_SPEED",
    "simExtRRS1_SET_JOINT_ACCELERATIONS",
    "simRRS1.SET_JOINT_ACCELERATIONS",
    "simExtRRS1_SET_CARTESIAN_POSITION_ACCELERATION",
    "simRRS1.SET_CARTESIAN_POSITION_ACCELERATION",
    "simExtRRS1_SET_CARTESIAN_ORIENTATION_ACCELERATION",
    "simRRS1.SET_CARTESIAN_ORIENTATION_ACCELERATION",
    "simExtRRS1_SET_JOINT_JERKS",
    "simRRS1.SET_JOINT_JERKS",
    "simExtRRS1_SET_MOTION_TIME",
    "simRRS1.SET_MOTION_TIME",
    "simExtRRS1_SET_OVERRIDE_SPEED",
    "simRRS1.SET_OVERRIDE_SPEED",
    "simExtRRS1_SET_OVERRIDE_ACCELERATION",
    "simRRS1.SET_OVERRIDE_ACCELERATION",
    "simExtRRS1_SELECT_FLYBY_MODE",
    "simRRS1.SELECT_FLYBY_MODE",
    "simExtRRS1_SET_FLYBY_CRITERIA_PARAMETER",
    "simRRS1.SET_FLYBY_CRITERIA_PARAMETER",
    "simExtRRS1_SELECT_FLYBY_CRITERIA",
    "simRRS1.SELECT_FLYBY_CRITERIA",
    "simExtRRS1_CANCEL_FLYBY_CRITERIA",
    "simRRS1.CANCEL_FLYBY_CRITERIA",
    "simExtRRS1_SELECT_POINT_ACCURACY",
    "simRRS1.SELECT_POINT_ACCURACY",
    "simExtRRS1_SET_POINT_ACCURACY_PARAMETER",
    "simRRS1.SET_POINT_ACCURACY_PARAMETER",
    "simExtRRS1_SET_REST_PARAMETER",
    "simRRS1.SET_REST_PARAMETER",
    "simExtRRS1_GET_CURRENT_TARGETID",
    "simRRS1.GET_CURRENT_TARGETID",
    "simExtRRS1_SELECT_TRACKING",
    "simRRS1.SELECT_TRACKING",
    "simExtRRS1_SET_CONVEYOR_POSITION",
    "simRRS1.SET_CONVEYOR_POSITION",
    "simExtRRS1_DEFINE_EVENT",
    "simRRS1.DEFINE_EVENT",
    "simExtRRS1_CANCEL_EVENT",
    "simRRS1.CANCEL_EVENT",
    "simExtRRS1_GET_EVENT",
    "simRRS1.GET_EVENT",
    "simExtRRS1_STOP_MOTION",
    "simRRS1.STOP_MOTION",
    "simExtRRS1_CONTINUE_MOTION",
    "simRRS1.CONTINUE_MOTION",
    "simExtRRS1_CANCEL_MOTION",
    "simRRS1.CANCEL_MOTION",
    "simExtRRS1_GET_MESSAGE",
    "simRRS1.GET_MESSAGE",
    "simExtRRS1_SELECT_WEAVING_MODE",
    "simRRS1.SELECT_WEAVING_MODE",
    "simExtRRS1_SELECT_WEAVING_GROUP",
    "simRRS1.SELECT_WEAVING_GROUP",
    "simExtRRS1_SET_WEAVING_GROUP_PARAMETER",
    "simRRS1.SET_WEAVING_GROUP_PARAMETER",
    "simExtRRS1_DEBUG",
    "simRRS1.DEBUG",
    "simExtRRS1_EXTENDED_SERVICE",
    "simRRS1.EXTENDED_SERVICE",
    "",
    "",
};
const SNewApiMapping _simVisionApiMapping[] = {
    "simExtVision_handleSpherical",
    "simVision.handleSpherical",
    "simExtVision_handleAnaglyphStereo",
    "simVision.handleAnaglyphStereo",
    "simExtVision_createVelodyneHDL64E",
    "simVision.createVelodyneHDL64E",
    "simExtVision_destroyVelodyneHDL64E",
    "simVision.destroyVelodyneHDL64E",
    "simExtVision_handleVelodyneHDL64E",
    "simVision.handleVelodyneHDL64E",
    "simExtVision_createVelodyneVPL16",
    "simVision.createVelodyneVPL16",
    "simExtVision_destroyVelodyneVPL16",
    "simExtVision_destroyVelodyneVPL16",
    "simExtVision_handleVelodyneVPL16",
    "simVision.handleVelodyneVPL16",
    "simExtVision_createVelodyne",
    "simVision.createVelodyneHDL64E",
    "simExtVision_destroyVelodyne",
    "simVision.destroyVelodyneHDL64E",
    "simExtVision_handleVelodyne",
    "simVision.handleVelodyneHDL64E",
    "",
    "",
};
const SNewApiMapping _simCamApiMapping[] = {
    "simExtCamStart",
    "simCam.start",
    "simExtCamEnd",
    "simCam.stop",
    "simExtCamInfo",
    "simCam.info",
    "simExtCamGrab",
    "simCam.grab",
    "",
    "",
};
const SNewApiMapping _simJoyApiMapping[] = {
    "simExtJoyGetCount",
    "simJoy.getCount",
    "simExtJoyGetData",
    "simJoy.getData",
    "",
    "",
};
const SNewApiMapping _simWiiApiMapping[] = {
    "simExtWiiStart",
    "simWii.start",
    "simExtWiiEnd",
    "simWii.stop",
    "simExtWiiSet",
    "simWii.set",
    "simExtWiiGet",
    "simWii.get",
    "",
    "",
};
const SNewApiMapping _simURDFApiMapping[] = {
    "simExtImportUrdf",
    "simURDF.import",
    "simExtImportUrdfFile",
    "simURDF.importFile",
    "",
    "",
};
const SNewApiMapping _simBWFApiMapping[] = {
    "simExtBwf_query",
    "simBWF.query",
    "",
    "",
};
const SNewApiMapping _simUIApiMapping[] = {
    "simExtCustomUI_create",
    "simUI.create",
    "simExtCustomUI_destroy",
    "simUI.destroy",
    "simExtCustomUI_getSliderValue",
    "simUI.getSliderValue",
    "simExtCustomUI_setSliderValue",
    "simUI.setSliderValue",
    "simExtCustomUI_getEditValue",
    "simUI.getEditValue",
    "simExtCustomUI_setEditValue",
    "simUI.setEditValue",
    "simExtCustomUI_getSpinboxValue",
    "simUI.getSpinboxValue",
    "simExtCustomUI_setSpinboxValue",
    "simUI.setSpinboxValue",
    "simExtCustomUI_getCheckboxValue",
    "simUI.getCheckboxValue",
    "simExtCustomUI_setCheckboxValue",
    "simUI.setCheckboxValue",
    "simExtCustomUI_getRadiobuttonValue",
    "simUI.getRadiobuttonValue",
    "simExtCustomUI_setRadiobuttonValue",
    "simUI.setRadiobuttonValue",
    "simExtCustomUI_getLabelText",
    "simUI.getLabelText",
    "simExtCustomUI_setLabelText",
    "simUI.setLabelText",
    "simExtCustomUI_insertComboboxItem",
    "simUI.insertComboboxItem",
    "simExtCustomUI_removeComboboxItem",
    "simUI.removeComboboxItem",
    "simExtCustomUI_getComboboxItemCount",
    "simUI.getComboboxItemCount",
    "simExtCustomUI_getComboboxItemText",
    "simUI.getComboboxItemText",
    "simExtCustomUI_getComboboxItems",
    "simUI.getComboboxItems",
    "simExtCustomUI_setComboboxItems",
    "simUI.setComboboxItems",
    "simExtCustomUI_setComboboxSelectedIndex",
    "simUI.setComboboxSelectedIndex",
    "simExtCustomUI_hide",
    "simUI.hide",
    "simExtCustomUI_show",
    "simUI.show",
    "simExtCustomUI_isVisible",
    "simUI.isVisible",
    "simExtCustomUI_getPosition",
    "simUI.getPosition",
    "simExtCustomUI_setPosition",
    "simUI.setPosition",
    "simExtCustomUI_getSize",
    "simUI.getSize",
    "simExtCustomUI_setSize",
    "simUI.setSize",
    "simExtCustomUI_getTitle",
    "simUI.getTitle",
    "simExtCustomUI_setTitle",
    "simUI.setTitle",
    "simExtCustomUI_setImageData",
    "simUI.setImageData",
    "simExtCustomUI_setEnabled",
    "simUI.setEnabled",
    "simExtCustomUI_getCurrentTab",
    "simUI.getCurrentTab",
    "simExtCustomUI_setCurrentTab",
    "simUI.setCurrentTab",
    "simExtCustomUI_getWidgetVisibility",
    "simUI.getWidgetVisibility",
    "simExtCustomUI_setWidgetVisibility",
    "simUI.setWidgetVisibility",
    "simExtCustomUI_getCurrentEditWidget",
    "simUI.getCurrentEditWidget",
    "simExtCustomUI_setCurrentEditWidget",
    "simUI.setCurrentEditWidget",
    "simExtCustomUI_replot",
    "simUI.replot",
    "simExtCustomUI_addCurve",
    "simUI.addCurve",
    "simExtCustomUI_addCurveTimePoints",
    "simUI.addCurveTimePoints",
    "simExtCustomUI_addCurveXYPoints",
    "simUI.addCurveXYPoints",
    "simExtCustomUI_clearCurve",
    "simUI.clearCurve",
    "simExtCustomUI_removeCurve",
    "simUI.removeCurve",
    "simExtCustomUI_setPlotRanges",
    "simUI.setPlotRanges",
    "simExtCustomUI_setPlotXRange",
    "simUI.setPlotXRange",
    "simExtCustomUI_setPlotYRange",
    "simUI.setPlotYRange",
    "simExtCustomUI_growPlotRanges",
    "simUI.growPlotRanges",
    "simExtCustomUI_growPlotXRange",
    "simUI.growPlotXRange",
    "simExtCustomUI_growPlotYRange",
    "simUI.growPlotYRange",
    "simExtCustomUI_setPlotLabels",
    "simUI.setPlotLabels",
    "simExtCustomUI_setPlotXLabel",
    "simUI.setPlotXLabel",
    "simExtCustomUI_setPlotYLabel",
    "simUI.setPlotYLabel",
    "simExtCustomUI_rescaleAxes",
    "simUI.rescaleAxes",
    "simExtCustomUI_rescaleAxesAll",
    "simUI.rescaleAxesAll",
    "simExtCustomUI_setMouseOptions",
    "simUI.setMouseOptions",
    "simExtCustomUI_setLegendVisibility",
    "simUI.setLegendVisibility",
    "simExtCustomUI_getCurveData",
    "simUI.getCurveData",
    "simExtCustomUI_clearTable",
    "simUI.clearTable",
    "simExtCustomUI_setRowCount",
    "simUI.setRowCount",
    "simExtCustomUI_setColumnCount",
    "simUI.setColumnCount",
    "simExtCustomUI_setRowHeaderText",
    "simUI.setRowHeaderText",
    "simExtCustomUI_setColumnHeaderText",
    "simUI.setColumnHeaderText",
    "simExtCustomUI_setItem",
    "simUI.setItem",
    "simExtCustomUI_getRowCount",
    "simUI.getRowCount",
    "simExtCustomUI_getColumnCount",
    "simUI.getColumnCount",
    "simExtCustomUI_getItem",
    "simUI.getItem",
    "simExtCustomUI_setItemEditable",
    "simUI.setItemEditable",
    "simExtCustomUI_saveState",
    "simUI.saveState",
    "simExtCustomUI_restoreState",
    "simUI.restoreState",
    "simExtCustomUI_setColumnWidth",
    "simUI.setColumnWidth",
    "simExtCustomUI_setRowHeight",
    "simUI.setRowHeight",
    "simExtCustomUI_setTableSelection",
    "simUI.setTableSelection",
    "simExtCustomUI_setProgress",
    "simUI.setProgress",
    "simExtCustomUI_clearTree",
    "simUI.clearTree",
    "simExtCustomUI_addTreeItem",
    "simUI.addTreeItem",
    "simExtCustomUI_updateTreeItemText",
    "simUI.updateTreeItemText",
    "simExtCustomUI_updateTreeItemParent",
    "simUI.updateTreeItemParent",
    "simExtCustomUI_removeTreeItem",
    "simUI.removeTreeItem",
    "simExtCustomUI_setTreeSelection",
    "simUI.setTreeSelection",
    "simExtCustomUI_expandAll",
    "simUI.expandAll",
    "simExtCustomUI_collapseAll",
    "simUI.collapseAll",
    "simExtCustomUI_expandToDepth",
    "simUI.expandToDepth",
    "simExtCustomUI_addNode",
    "simUI.addNode",
    "simExtCustomUI_removeNode",
    "simUI.removeNode",
    "simExtCustomUI_setNodeValid",
    "simUI.setNodeValid",
    "simExtCustomUI_isNodeValid",
    "simUI.isNodeValid",
    "simExtCustomUI_setNodePos",
    "simUI.setNodePos",
    "simExtCustomUI_getNodePos",
    "simUI.getNodePos",
    "simExtCustomUI_setNodeText",
    "simUI.setNodeText",
    "simExtCustomUI_getNodeText",
    "simUI.getNodeText",
    "simExtCustomUI_setNodeInletCount",
    "simUI.setNodeInletCount",
    "simExtCustomUI_getNodeInletCount",
    "simUI.getNodeInletCount",
    "simExtCustomUI_setNodeOutletCount",
    "simUI.setNodeOutletCount",
    "simExtCustomUI_getNodeOutletCount",
    "simUI.getNodeOutletCount",
    "simExtCustomUI_addConnection",
    "simUI.addConnection",
    "simExtCustomUI_removeConnection",
    "simUI.removeConnection",
    "simExtCustomUI_setText",
    "simUI.setText",
    "simExtCustomUI_setUrl",
    "simUI.setUrl",
    "sim_customui_curve_type_time",
    "simUI.curve_type.time",
    "sim_customui_curve_type_xy",
    "simUI.curve_type.xy",
    "sim_customui_curve_style_scatter",
    "simUI.curve_style.scatter",
    "sim_customui_curve_style_line",
    "simUI.curve_style.line",
    "sim_customui_curve_style_line_and_scatter",
    "simUI.curve_style.line_and_scatter",
    "sim_customui_curve_style_step_left",
    "simUI.curve_style.step_left",
    "sim_customui_curve_style_step_center",
    "simUI.curve_style.step_center",
    "sim_customui_curve_style_step_right",
    "simUI.curve_style.step_right",
    "sim_customui_curve_style_impulse",
    "simUI.curve_style.impulse",
    "sim_customui_curve_scatter_shape_none",
    "simUI.curve_scatter_shape.none",
    "sim_customui_curve_scatter_shape_dot",
    "simUI.curve_scatter_shape.dot",
    "sim_customui_curve_scatter_shape_cross",
    "simUI.curve_scatter_shape.cross",
    "sim_customui_curve_scatter_shape_plus",
    "simUI.curve_scatter_shape.plus",
    "sim_customui_curve_scatter_shape_circle",
    "simUI.curve_scatter_shape.circle",
    "sim_customui_curve_scatter_shape_disc",
    "simUI.curve_scatter_shape.disc",
    "sim_customui_curve_scatter_shape_square",
    "simUI.curve_scatter_shape.square",
    "sim_customui_curve_scatter_shape_diamond",
    "simUI.curve_scatter_shape.diamond",
    "sim_customui_curve_scatter_shape_star",
    "simUI.curve_scatter_shape.star",
    "sim_customui_curve_scatter_shape_triangle",
    "simUI.curve_scatter_shape.triangle",
    "sim_customui_curve_scatter_shape_triangle_inverted",
    "simUI.curve_scatter_shape.triangle_inverted",
    "sim_customui_curve_scatter_shape_cross_square",
    "simUI.curve_scatter_shape.cross_square",
    "sim_customui_curve_scatter_shape_plus_square",
    "simUI.curve_scatter_shape.plus_square",
    "sim_customui_curve_scatter_shape_cross_circle",
    "simUI.curve_scatter_shape.cross_circle",
    "sim_customui_curve_scatter_shape_plus_circle",
    "simUI.curve_scatter_shape.plus_circle",
    "sim_customui_curve_scatter_shape_peace",
    "simUI.curve_scatter_shape.peace",
    "sim_customui_line_style_solid",
    "simUI.line_style.solid",
    "sim_customui_line_style_dotted",
    "simUI.line_style.dotted",
    "sim_customui_line_style_dashed",
    "simUI.line_style.dashed",
    "sim_customui_mouse_left_button_down",
    "simUI.mouse.left_button_down",
    "sim_customui_mouse_left_button_up",
    "simUI.mouse.left_button_up",
    "sim_customui_mouse_move",
    "simUI.mouse.move",
    "",
    "",
};
const SNewApiMapping _simROSApiMapping[] = {
    "simExtRosInterface_subscribe",
    "simROS.subscribe",
    "simExtRosInterface_shutdownSubscriber",
    "simROS.shutdownSubscriber",
    "simExtRosInterface_subscriberTreatUInt8ArrayAsString",
    "simROS.subscriberTreatUInt8ArrayAsString",
    "simExtRosInterface_advertise",
    "simROS.advertise",
    "simExtRosInterface_shutdownPublisher",
    "simROS.shutdownPublisher",
    "simExtRosInterface_publisherTreatUInt8ArrayAsString",
    "simROS.publisherTreatUInt8ArrayAsString",
    "simExtRosInterface_publish",
    "simROS.publish",
    "simExtRosInterface_serviceClient",
    "simROS.serviceClient",
    "simExtRosInterface_shutdownServiceClient",
    "simROS.shutdownServiceClient",
    "simExtRosInterface_serviceClientTreatUInt8ArrayAsString",
    "simROS.serviceClientTreatUInt8ArrayAsString",
    "simExtRosInterface_call",
    "simROS.call",
    "simExtRosInterface_advertiseService",
    "simROS.advertiseService",
    "simExtRosInterface_shutdownServiceServer",
    "simROS.shutdownServiceServer",
    "simExtRosInterface_serviceServerTreatUInt8ArrayAsString",
    "simROS.serviceServerTreatUInt8ArrayAsString",
    "simExtRosInterface_sendTransform",
    "simROS.sendTransform",
    "simExtRosInterface_sendTransforms",
    "simROS.sendTransforms",
    "simExtRosInterface_imageTransportSubscribe",
    "simROS.imageTransportSubscribe",
    "simExtRosInterface_imageTransportShutdownSubscriber",
    "simROS.imageTransportShutdownSubscriber",
    "simExtRosInterface_imageTransportAdvertise",
    "simROS.imageTransportAdvertise",
    "simExtRosInterface_imageTransportShutdownPublisher",
    "simROS.imageTransportShutdownPublisher",
    "simExtRosInterface_imageTransportPublish",
    "simROS.imageTransportPublish",
    "simExtRosInterface_getTime",
    "simROS.getTime",
    "simExtRosInterface_getParamString",
    "simROS.getParamString",
    "simExtRosInterface_getParamInt",
    "simROS.getParamInt",
    "simExtRosInterface_getParamDouble",
    "simROS.getParamDouble",
    "simExtRosInterface_getParamBool",
    "simROS.getParamBool",
    "simExtRosInterface_setParamString",
    "simROS.setParamString",
    "simExtRosInterface_setParamInt",
    "simROS.setParamInt",
    "simExtRosInterface_setParamDouble",
    "simROS.setParamDouble",
    "simExtRosInterface_setParamBool",
    "simROS.setParamBool",
    "simExtRosInterface_hasParam",
    "simROS.hasParam",
    "simExtRosInterface_deleteParam",
    "simROS.deleteParam",
    "simExtRosInterface_searchParam",
    "simROS.searchParam",
    "",
    "",
};
const SNewApiMapping _simICPApiMapping[] = {
    "simExtICP_match",
    "simICP.match",
    "simExtICP_matchToShape",
    "simICP.matchToShape",
    "",
    "",
};
const SNewApiMapping _simOMPLApiMapping[] = {
    "simExtOMPL_createStateSpace",
    "simOMPL.createStateSpace",
    "simExtOMPL_destroyStateSpace",
    "simOMPL.destroyStateSpace",
    "simExtOMPL_createTask",
    "simOMPL.createTask",
    "simExtOMPL_destroyTask",
    "simOMPL.destroyTask",
    "simExtOMPL_printTaskInfo",
    "simOMPL.printTaskInfo",
    "simExtOMPL_setVerboseLevel",
    "simOMPL.setVerboseLevel",
    "simExtOMPL_setStateValidityCheckingResolution",
    "simOMPL.setStateValidityCheckingResolution",
    "simExtOMPL_setStateSpace",
    "simOMPL.setStateSpace",
    "simExtOMPL_setAlgorithm",
    "simOMPL.setAlgorithm",
    "simExtOMPL_setCollisionPairs",
    "simOMPL.setCollisionPairs",
    "simExtOMPL_setStartState",
    "simOMPL.setStartState",
    "simExtOMPL_setGoalState",
    "simOMPL.setGoalState",
    "simExtOMPL_addGoalState",
    "simOMPL.addGoalState",
    "simExtOMPL_setGoal",
    "simOMPL.setGoal",
    "simExtOMPL_setup",
    "simOMPL.setup",
    "simExtOMPL_solve",
    "simOMPL.solve",
    "simExtOMPL_simplifyPath",
    "simOMPL.simplifyPath",
    "simExtOMPL_interpolatePath",
    "simOMPL.interpolatePath",
    "simExtOMPL_getPath",
    "simOMPL.getPath",
    "simExtOMPL_compute",
    "simOMPL.compute",
    "simExtOMPL_readState",
    "simOMPL.readState",
    "simExtOMPL_writeState",
    "simOMPL.writeState",
    "simExtOMPL_isStateValid",
    "simOMPL.isStateValid",
    "simExtOMPL_setProjectionEvaluationCallback",
    "simOMPL.setProjectionEvaluationCallback",
    "simExtOMPL_setStateValidationCallback",
    "simOMPL.setStateValidationCallback",
    "simExtOMPL_setGoalCallback",
    "simOMPL.setGoalCallback",
    "simExtOMPL_setValidStateSamplerCallback",
    "simOMPL.setValidStateSamplerCallback",
    "sim_ompl_algorithm_BiTRRT",
    "simOMPL.Algorithm.BiTRRT",
    "sim_ompl_algorithm_BITstar",
    "simOMPL.Algorithm.BITstar",
    "sim_ompl_algorithm_BKPIECE1",
    "simOMPL.Algorithm.BKPIECE1",
    "sim_ompl_algorithm_CForest",
    "simOMPL.Algorithm.CForest",
    "sim_ompl_algorithm_EST",
    "simOMPL.Algorithm.EST",
    "sim_ompl_algorithm_FMT",
    "simOMPL.Algorithm.FMT",
    "sim_ompl_algorithm_KPIECE1",
    "simOMPL.Algorithm.KPIECE1",
    "sim_ompl_algorithm_LazyPRM",
    "simOMPL.Algorithm.LazyPRM",
    "sim_ompl_algorithm_LazyPRMstar",
    "simOMPL.Algorithm.LazyPRMstar",
    "sim_ompl_algorithm_LazyRRT",
    "simOMPL.Algorithm.LazyRRT",
    "sim_ompl_algorithm_LBKPIECE1",
    "simOMPL.Algorithm.LBKPIECE1",
    "sim_ompl_algorithm_LBTRRT",
    "simOMPL.Algorithm.LBTRRT",
    "sim_ompl_algorithm_PDST",
    "simOMPL.Algorithm.PDST",
    "sim_ompl_algorithm_PRM",
    "simOMPL.Algorithm.PRM",
    "sim_ompl_algorithm_PRMstar",
    "simOMPL.Algorithm.PRMstar",
    "sim_ompl_algorithm_pRRT",
    "simOMPL.Algorithm.pRRT",
    "sim_ompl_algorithm_pSBL",
    "simOMPL.Algorithm.pSBL",
    "sim_ompl_algorithm_RRT",
    "simOMPL.Algorithm.RRT",
    "sim_ompl_algorithm_RRTConnect",
    "simOMPL.Algorithm.RRTConnect",
    "sim_ompl_algorithm_RRTstar",
    "simOMPL.Algorithm.RRTstar",
    "sim_ompl_algorithm_SBL",
    "simOMPL.Algorithm.SBL",
    "sim_ompl_algorithm_SPARS",
    "simOMPL.Algorithm.SPARS",
    "sim_ompl_algorithm_SPARStwo",
    "simOMPL.Algorithm.SPARStwo",
    "sim_ompl_algorithm_STRIDE",
    "simOMPL.Algorithm.STRIDE",
    "sim_ompl_algorithm_TRRT",
    "simOMPL.Algorithm.TRRT",
    "sim_ompl_statespacetype_position2d",
    "simOMPL.StateSpaceType.position2d",
    "sim_ompl_statespacetype_pose2d",
    "simOMPL.StateSpaceType.pose2d",
    "sim_ompl_statespacetype_position3d",
    "simOMPL.StateSpaceType.position3d",
    "sim_ompl_statespacetype_pose3d",
    "simOMPL.StateSpaceType.pose3d",
    "sim_ompl_statespacetype_joint_position",
    "simOMPL.StateSpaceType.joint_position",
    "",
    "",
};
const SNewApiMapping _simSDFApiMapping[] = {
    "simExtSDF_import",
    "simSDF.import",
    "simExtSDF_dump",
    "simSDF.dump",
    "",
    "",
};
const SNewApiMapping _simSurfRecApiMapping[] = {
    "simExtSurfaceReconstruction_reconstruct",
    "simSurfRec.reconstruct",
    "sim_surfacereconstruction_algorithm_scalespace",
    "simSurfRec.Algorithm.scalespace",
    "sim_surfacereconstruction_algorithm_poisson",
    "simSurfRec.Algorithm.poisson",
    "sim_surfacereconstruction_algorithm_advancingfront",
    "simSurfRec.Algorithm.advancingfront",
    "",
    "",
};
const SNewApiMapping _simxApiMapping[] = {
    "simxStart",
    "simx.start",
    "simxFinish",
    "simx.finish",
    "simxAddStatusbarMessage",
    "simx.addStatusbarMessage",
    "simxAuxiliaryConsoleClose",
    "simx.auxiliaryConsoleClose",
    "simxAuxiliaryConsoleOpen",
    "simx.auxiliaryConsoleOpen",
    "simxAuxiliaryConsolePrint",
    "simx.auxiliaryConsolePrint",
    "simxAuxiliaryConsoleShow",
    "simx.auxiliaryConsoleShow",
    "simxBreakForceSensor",
    "simx.breakForceSensor",
    "simxClearFloatSignal",
    "simx.clearFloatSignal",
    "simxClearIntegerSignal",
    "simx.clearIntegerSignal",
    "simxClearStringSignal",
    "simx.clearStringSignal",
    "simxGetObjectHandle",
    "simx.getObjectHandle",
    "simxCloseScene",
    "simx.closeScene",
    "simxCopyPasteObjects",
    "simx.copyPasteObjects",
    "simxLoadScene",
    "simx.loadScene",
    "simxLoadModel",
    "simx.loadModel",
    "simxSetBooleanParameter",
    "simx.setBooleanParameter",
    "simxStartSimulation",
    "simx.startSimulation",
    "simxStopSimulation",
    "simx.stopSimulation",
    "simxSetFloatingParameter",
    "simx.setFloatingParameter",
    "simxSetIntegerParameter",
    "simx.setIntegerParameter",
    "simxSetArrayParameter",
    "simx.setArrayParameter",
    "simxGetBooleanParameter",
    "simx.getBooleanParameter",
    "simxGetFloatingParameter",
    "simx.getFloatingParameter",
    "simxGetIntegerParameter",
    "simx.getIntegerParameter",
    "simxGetArrayParameter",
    "simx.getArrayParameter",
    "simxGetStringParameter",
    "simx.getStringParameter",
    "simxSetFloatSignal",
    "simx.setFloatSignal",
    "simxSetIntegerSignal",
    "simx.setIntegerSignal",
    "simxSetStringSignal",
    "simx.setStringSignal",
    "simxGetFloatSignal",
    "simx.getFloatSignal",
    "simxGetIntegerSignal",
    "simx.getIntegerSignal",
    "simxGetStringSignal",
    "simx.getStringSignal",
    "simxCreateDummy",
    "simx.createDummy",
    "simxDisplayDialog",
    "simx.displayDialog",
    "simxEndDialog",
    "simx.endDialog",
    "simxGetDialogInput",
    "simx.getDialogInput",
    "simxGetDialogResult",
    "simx.getDialogResult",
    "simxEraseFile",
    "simx.eraseFile",
    "simxGetCollisionHandle",
    "simx.getCollisionHandle",
    "simxGetCollectionHandle",
    "simx.getCollectionHandle",
    "simxGetConnectionId",
    "simx.getConnectionId",
    "simxGetDistanceHandle",
    "simx.getDistanceHandle",
    "simxGetInMessageInfo",
    "simx.getInMessageInfo",
    "simxGetOutMessageInfo",
    "simx.getOutMessageInfo",
    "simxGetJointForce",
    "simx.getJointForce",
    "simxGetJointMatrix",
    "simx.getJointMatrix",
    "simxGetJointPosition",
    "simx.getJointPosition",
    "simxGetLastCmdTime",
    "simx.getLastCmdTime",
    "simxGetLastErrors",
    "simx.getLastErrors",
    "simxGetModelProperty",
    "simx.getModelProperty",
    "simxGetObjectChild",
    "simx.getObjectChild",
    "simxGetObjectFloatParameter",
    "simx.getObjectFloatParameter",
    "simxGetObjectIntParameter",
    "simx.getObjectIntParameter",
    "simxGetObjectGroupData",
    "simx.getObjectGroupData",
    "simxGetObjectOrientation",
    "simx.getObjectOrientation",
    "simxGetObjectPosition",
    "simx.getObjectPosition",
    "simxGetObjectParent",
    "simx.getObjectParent",
    "simxGetObjects",
    "simx.getObjects",
    "simxGetObjectSelection",
    "simx.getObjectSelection",
    "simxGetObjectVelocity",
    "simx.getObjectVelocity",
    "simxGetPingTime",
    "simx.getPingTime",
    "simxGetVisionSensorDepthBuffer",
    "simx.getVisionSensorDepthBuffer",
    "simxGetVisionSensorImage",
    "simx.getVisionSensorImage",
    "simxSetVisionSensorImage",
    "simx.setVisionSensorImage",
    "simxPauseCommunication",
    "simx.pauseCommunication",
    "simxPauseSimulation",
    "simx.pauseSimulation",
    "simxQuery",
    "simx.query",
    "simxReadCollision",
    "simx.readCollision",
    "simxReadDistance",
    "simx.readDistance",
    "simxReadForceSensor",
    "simx.readForceSensor",
    "simxReadProximitySensor",
    "simx.readProximitySensor",
    "simxReadStringStream",
    "simx.readStringStream",
    "simxWriteStringStream",
    "simx.writeStringStream",
    "simxReadVisionSensor",
    "simx.readVisionSensor",
    "simxRemoveModel",
    "simx.removeModel",
    "simxRemoveObject",
    "simx.removeObject",
    "simxSetJointForce",
    "simx.setJointForce",
    "simxSetJointPosition",
    "simx.setJointPosition",
    "simxSetJointTargetPosition",
    "simx.setJointTargetPosition",
    "simxSetJointTargetVelocity",
    "simx.setJointTargetVelocity",
    "simxSetModelProperty",
    "simx.setModelProperty",
    "simxSetObjectOrientation",
    "simx.setObjectOrientation",
    "simxSetObjectPosition",
    "simx.setObjectPosition",
    "simxSetObjectParent",
    "simx.setObjectParent",
    "simxSetObjectSelection",
    "simx.setObjectSelection",
    "simxSetSphericalJointMatrix",
    "simx.setSphericalJointMatrix",
    "simxSynchronous",
    "simx.synchronous",
    "simxSynchronousTrigger",
    "simx.synchronousTrigger",
    "simxTransferFile",
    "simx.transferFile",
    "simxSetObjectFloatParameter",
    "simx.setObjectFloatParameter",
    "simxSetObjectIntParameter",
    "simx.setObjectIntParameter",
    "simxCallScriptFunction",
    "simx.callScriptFunction",
    "simxLoadUI",
    "simx.loadUI",
    "simxGetUIButtonProperty",
    "simx.getUIButtonProperty",
    "simxGetUIEventButton",
    "simx.getUIEventButton",
    "simxGetUIHandle",
    "simx.getUIHandle",
    "simxGetUISlider",
    "simx.getUISlider",
    "simxRemoveUI",
    "simx.removeUI",
    "simxSetUIButtonLabel",
    "simx.setUIButtonLabel",
    "simxSetUIButtonProperty",
    "simx.setUIButtonProperty",
    "simxSetUISlider",
    "simx.setUISlider",
    "simx_return_ok",
    "simx.return_ok",
    "simx_return_novalue_flag",
    "simx.return_novalue_flag",
    "simx_return_timeout_flag",
    "simx.return_timeout_flag",
    "simx_return_illegal_opmode_flag",
    "simx.return_illegal_opmode_flag",
    "simx_return_remote_error_flag",
    "simx.return_remote_error_flag",
    "simx_return_split_progress_flag",
    "simx.return_split_progress_flag",
    "simx_return_local_error_flag",
    "simx.return_local_error_flag",
    "simx_return_initialize_error_flag",
    "simx.return_initialize_error_flag",
    "simx_opmode_oneshot",
    "simx.opmode_oneshot",
    "simx_opmode_blocking",
    "simx.opmode_blocking",
    "simx_opmode_oneshot_wait",
    "simx.opmode_oneshot_wait",
    "simx_opmode_streaming",
    "simx.opmode_streaming",
    "simx_opmode_oneshot_split",
    "simx.opmode_oneshot_split",
    "simx_opmode_streaming_split",
    "simx.opmode_streaming_split",
    "simx_opmode_discontinue",
    "simx.opmode_discontinue",
    "simx_opmode_buffer",
    "simx.opmode_buffer",
    "simx_opmode_remove",
    "simx.opmode_remove",
    "simx_headeroffset_version",
    "simx.headeroffset_version",
    "simx_headeroffset_message_id",
    "simx.headeroffset_message_id",
    "simx_headeroffset_client_time",
    "simx.headeroffset_client_time",
    "simx_headeroffset_server_time",
    "simx.headeroffset_server_time",
    "simx_headeroffset_scene_id",
    "simx.headeroffset_scene_id",
    "simx_headeroffset_server_state",
    "simx.headeroffset_server_state",
    "simx_cmdheaderoffset_mem_size",
    "simx.cmdheaderoffset_mem_size",
    "simx_cmdheaderoffset_full_mem_size",
    "simx.cmdheaderoffset_full_mem_size",
    "simx_cmdheaderoffset_cmd",
    "simx.cmdheaderoffset_cmd",
    "simx_cmdheaderoffset_sim_time",
    "simx.cmdheaderoffset_sim_time",
    "simx_cmdheaderoffset_status",
    "simx.cmdheaderoffset_status",
    "",
    "",
};
// **************************************************************
// **************************************************************
