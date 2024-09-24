#include <simInternal.h>
#include <embeddedScriptContainer.h>
#include <tt.h>
#include <vVarious.h>
#include <threadPool_old.h>
#include <vFileFinder.h>
#include <simStrings.h>
#include <app.h>
#include <vDateTime.h>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

CEmbeddedScriptContainer::CEmbeddedScriptContainer()
{
    _sysFuncAndHookCnt_event = 0;
    _sysFuncAndHookCnt_dyn = 0;
    _sysFuncAndHookCnt_contact = 0;
    _sysFuncAndHookCnt_joint = 0;
    insertDefaultScript(sim_scripttype_main, false, true);
}

CEmbeddedScriptContainer::~CEmbeddedScriptContainer()
{ // beware, the current world could be nullptr
    removeAllScripts();
    for (size_t i = 0; i < _callbackStructureToDestroyAtEndOfSimulation_new.size(); i++)
        delete _callbackStructureToDestroyAtEndOfSimulation_new[i];
    for (size_t i = 0; i < _callbackStructureToDestroyAtEndOfSimulation_old.size(); i++)
        delete _callbackStructureToDestroyAtEndOfSimulation_old[i];
    broadcastDataContainer.eraseAllObjects();
}

int CEmbeddedScriptContainer::getSysFuncAndHookCnt(int sysCall) const
{
    if (sysCall == sim_syscb_event)
        return (_sysFuncAndHookCnt_event);
    if (sysCall == sim_syscb_dyn)
        return (_sysFuncAndHookCnt_dyn);
    if (sysCall == sim_syscb_contact)
        return (_sysFuncAndHookCnt_contact);
    if (sysCall == sim_syscb_joint)
        return (_sysFuncAndHookCnt_joint);
    return (0);
}

void CEmbeddedScriptContainer::setSysFuncAndHookCnt(int sysCall, int cnt)
{
    if (sysCall == sim_syscb_event)
        _sysFuncAndHookCnt_event = cnt;
    if (sysCall == sim_syscb_dyn)
        _sysFuncAndHookCnt_dyn = cnt;
    if (sysCall == sim_syscb_contact)
        _sysFuncAndHookCnt_contact = cnt;
    if (sysCall == sim_syscb_joint)
        _sysFuncAndHookCnt_joint = cnt;
}

void CEmbeddedScriptContainer::simulationAboutToStart()
{
    broadcastDataContainer.simulationAboutToStart();
    for (size_t i = 0; i < allScripts.size(); i++)
        allScripts[i]->simulationAboutToStart();
}

void CEmbeddedScriptContainer::simulationEnded()
{
    for (size_t i = 0; i < allScripts.size(); i++)
        allScripts[i]->simulationEnded();

    broadcastDataContainer.simulationEnded();
    removeDestroyedScripts(sim_scripttype_main);
    removeDestroyedScripts(sim_scripttype_simulation);
    for (size_t i = 0; i < _callbackStructureToDestroyAtEndOfSimulation_new.size(); i++)
        delete _callbackStructureToDestroyAtEndOfSimulation_new[i];
    _callbackStructureToDestroyAtEndOfSimulation_new.clear();
    for (size_t i = 0; i < _callbackStructureToDestroyAtEndOfSimulation_old.size(); i++)
        delete _callbackStructureToDestroyAtEndOfSimulation_old[i];
    _callbackStructureToDestroyAtEndOfSimulation_old.clear();
    //  if (_initialValuesInitialized&&App::currentWorld->simulation->getResetSceneAtSimulationEnd())
    //  {
    //  }
}

void CEmbeddedScriptContainer::simulationAboutToEnd()
{
    CScriptObject* ms = getMainScript();
    if (ms != nullptr)
        ms->simulationAboutToEnd(); // calls cleanup in main script (then cleanup in simulation scripts), then destroys main script state
    for (size_t i = 0; i < allScripts.size(); i++)
    {
        if (ms != allScripts[i])
            allScripts[i]->simulationAboutToEnd(); // destroys simulation script states
    }
}

void CEmbeddedScriptContainer::addCallbackStructureObjectToDestroyAtTheEndOfSimulation_new(SScriptCallBack *object)
{
    _callbackStructureToDestroyAtEndOfSimulation_new.push_back(object);
}
void CEmbeddedScriptContainer::addCallbackStructureObjectToDestroyAtTheEndOfSimulation_old(SLuaCallBack *object)
{
    _callbackStructureToDestroyAtEndOfSimulation_old.push_back(object);
}

void CEmbeddedScriptContainer::resetScriptFlagCalledInThisSimulationStep()
{
    for (size_t i = 0; i < allScripts.size(); i++)
        allScripts[i]->resetCalledInThisSimulationStep();
}

int CEmbeddedScriptContainer::getCalledScriptsCountInThisSimulationStep(bool onlySimulationScripts)
{
    int cnt = 0;
    for (size_t i = 0; i < allScripts.size(); i++)
    {
        if (allScripts[i]->getCalledInThisSimulationStep())
        {
            if (onlySimulationScripts)
            {
                if (allScripts[i]->getScriptType() == sim_scripttype_main)
                    cnt++;
                if (allScripts[i]->getScriptType() == sim_scripttype_simulation)
                {
                    if (!allScripts[i]->getThreadedExecution_oldThreads()) // ignore old threaded scripts
                        cnt++;
                }
            }
            else
                cnt++;
        }
    }
    return (cnt);
}

int CEmbeddedScriptContainer::removeDestroyedScripts(int scriptType)
{
    TRACE_INTERNAL;
    int retVal = 0;
    for (int i = 0; i < int(allScripts.size()); i++)
    {
        if ((allScripts[i]->getScriptType() == scriptType) && allScripts[i]->getFlaggedForDestruction())
        {
            if ((!allScripts[i]->getThreadedExecution_oldThreads()) ||
                (!allScripts[i]->getThreadedExecutionIsUnderWay_oldThreads()))
            {
                retVal++;
                CScriptObject *it = allScripts[i];
                it->resetScript(); // should not be done in the destructor!
                allScripts.erase(allScripts.begin() + i);
                i--;
                CScriptObject::destroy(it, true);
            }
        }
    }
    return (retVal);
}

void CEmbeddedScriptContainer::removeAllScripts()
{
    TRACE_INTERNAL;
    while (allScripts.size() > 0)
    {
        CScriptObject *it = allScripts[0];
        it->resetScript(); // should not be done in the destructor!
        allScripts.erase(allScripts.begin());
        CScriptObject::destroy(it, true);
        App::worldContainer->setModificationFlag(16384);
    }
}

void CEmbeddedScriptContainer::killAllSimulationLuaStates()
{
    for (size_t i = 0; i < allScripts.size(); i++)
    {
        if (allScripts[i]->isSimulationOrMainScript())
            allScripts[i]->resetScript();
    }
}

void CEmbeddedScriptContainer::announceObjectWillBeErased(const CSceneObject *object)
{ // Never called from copy buffer!
    size_t i = 0;
    while (i < allScripts.size())
    {
        if (allScripts[i]->announceSceneObjectWillBeErased(object, false))
        {
            if (removeScript(allScripts[i]->getScriptHandle()))
                i = 0; // ordering may have changed
            else
                i++;
        }
        else
            i++;
    }
}

bool CEmbeddedScriptContainer::removeScript_safe(int scriptHandle)
{ // removal may happen in a delayed fashion
    CScriptObject *it = getScriptObjectFromHandle(scriptHandle);
    if (it == nullptr)
        return (false);
    int res = it->flagScriptForRemoval();
    if (res == 0)
        return (false);
    if (res == 2)
        removeScript(scriptHandle);
    return (true);
}

bool CEmbeddedScriptContainer::removeScript(int scriptHandle)
{
    TRACE_INTERNAL;
    for (size_t i = 0; i < allScripts.size(); i++)
    {
        if (allScripts[i]->getScriptHandle() == scriptHandle)
        {
            CScriptObject *it = allScripts[i];
            it->resetScript(); // should not be done in the destructor!
            allScripts.erase(allScripts.begin() + i);
            CScriptObject::destroy(it, true);
            App::worldContainer->setModificationFlag(16384);
            break;
        }
    }
#ifdef SIM_WITH_GUI
    GuiApp::setFullDialogRefreshFlag();
#endif
    return (true);
}

void CEmbeddedScriptContainer::setScriptsTemporarilySuspended(bool suspended)
{
    for (size_t i = 0; i < allScripts.size(); i++)
        allScripts[i]->setTemporarilySuspended(suspended);
}

void CEmbeddedScriptContainer::extractScript(int scriptHandle)
{
    TRACE_INTERNAL;
    for (size_t i = 0; i < allScripts.size(); i++)
    {
        if (allScripts[i]->getScriptHandle() == scriptHandle)
        {
            allScripts.erase(allScripts.begin() + i);
            break;
        }
    }
}

CScriptObject *CEmbeddedScriptContainer::getScriptObjectFromHandle(int scriptHandle) const
{
    CScriptObject *retVal = nullptr;
    for (size_t i = 0; i < allScripts.size(); i++)
    {
        if (allScripts[i]->getScriptHandle() == scriptHandle)
        {
            retVal = allScripts[i];
            break;
        }
    }
    return (retVal);
}

CScriptObject *CEmbeddedScriptContainer::getScriptObjectFromUid(int uid) const
{
    CScriptObject *retVal = nullptr;
    for (size_t i = 0; i < allScripts.size(); i++)
    {
        if (allScripts[i]->getScriptUid() == uid)
        {
            retVal = allScripts[i];
            break;
        }
    }
    return (retVal);
}

int CEmbeddedScriptContainer::getObjectHandleFromScriptHandle(int scriptHandle) const
{
    CScriptObject *script = getScriptObjectFromHandle(scriptHandle);
    if (script != nullptr)
        return (script->getObjectHandleThatScriptIsAttachedTo(-1));
    return (-1);
}

CScriptObject *CEmbeddedScriptContainer::getScriptFromObjectAttachedTo(int scriptType, int objectHandle) const
{
    CScriptObject *retVal = nullptr;
    if (objectHandle >= 0)
    {
        for (size_t i = 0; i < allScripts.size(); i++)
        {
            if (allScripts[i]->getObjectHandleThatScriptIsAttachedTo(scriptType) == objectHandle)
            {
                retVal = allScripts[i];
                break;
            }
        }
    }
    return (retVal);
}

int CEmbeddedScriptContainer::getScriptsFromObjectAttachedTo(int objectHandle,
                                                             std::vector<CScriptObject *> &scripts) const
{
    scripts.clear();
    CScriptObject *it = getScriptFromObjectAttachedTo(sim_scripttype_simulation, objectHandle);
    if (it != nullptr)
        scripts.push_back(it);
    it = getScriptFromObjectAttachedTo(sim_scripttype_customization, objectHandle);
    if (it != nullptr)
        scripts.push_back(it);
    return (int(scripts.size()));
}

CScriptObject *CEmbeddedScriptContainer::getMainScript() const
{
    for (size_t i = 0; i < allScripts.size(); i++)
    {
        if (allScripts[i]->getScriptType() == sim_scripttype_main)
            return (allScripts[i]);
    }
    return (nullptr);
}

int CEmbeddedScriptContainer::insertScript(CScriptObject *script)
{
    allScripts.push_back(script);
    App::worldContainer->setModificationFlag(8192);
    return (script->getScriptHandle());
}

int CEmbeddedScriptContainer::insertDefaultScript(int scriptType, bool threaded, bool lua,
                                                  bool oldThreadedScript /*=false*/)
{
    if (scriptType != sim_scripttype_simulation)
        oldThreadedScript = false; // just to make sure
    int retVal = -1;
    std::string filenameAndPath(App::folders->getSystemPath() + "/");

    if (scriptType == sim_scripttype_main)
    {
        CScriptObject *defScript = new CScriptObject(scriptType);
        defScript->setLang("lua");
        retVal = insertScript(defScript);
        defScript->setScriptText(DEFAULT_MAINSCRIPT_CODE);
        filenameAndPath = "";
    }
    if (scriptType == sim_scripttype_simulation)
    {
        if (oldThreadedScript)
            filenameAndPath += DEFAULT_THREADEDCHILDSCRIPTOLD;
        else
        {
            if (threaded)
                filenameAndPath += DEFAULT_THREADEDCHILDSCRIPT;
            else
                filenameAndPath += DEFAULT_NONTHREADEDCHILDSCRIPT;
        }
    }
    if (scriptType == sim_scripttype_customization)
    {
        if (threaded)
            filenameAndPath += DEFAULT_THREADEDCUSTOMIZATIONSCRIPT;
        else
            filenameAndPath += DEFAULT_NONTHREADEDCUSTOMIZATIONSCRIPT;
    }

    if (filenameAndPath.size() > 0)
    {
        std::string lang;
        if (lua)
        {
            lang = "lua";
            filenameAndPath += ".lua";
        }
        else
        {
            lang = "python";
            filenameAndPath += ".py";
        }
        if (VFile::doesFileExist(filenameAndPath.c_str()))
        {
            try
            {
                VFile file(filenameAndPath.c_str(), VFile::READ | VFile::SHARE_DENY_NONE);
                VArchive archive(&file, VArchive::LOAD);
                unsigned int archiveLength = (unsigned int)file.getLength();
                char *defaultScript = new char[archiveLength + 1];
                for (int i = 0; i < int(archiveLength); i++)
                    archive >> defaultScript[i];
                defaultScript[archiveLength] = 0;
                CScriptObject *defScript = new CScriptObject(scriptType);
                defScript->setLang(lang.c_str());
                retVal = insertScript(defScript);
                defScript->setScriptText(defaultScript);
                if (oldThreadedScript)
                {
                    defScript->setThreadedExecution_oldThreads(true);
                    defScript->setExecuteJustOnce_oldThreads(true);
                }
                delete[] defaultScript;
                archive.close();
                file.close();
            }
            catch (VFILE_EXCEPTION_TYPE e)
            {
                VFile::reportAndHandleFileExceptionError(e);
                char defaultMessage[] = "Default script file could not be found!"; // do not use comments ("--"), we
                                                                                   // want to cause an execution error!
                CScriptObject *defScript = new CScriptObject(scriptType);
                defScript->setLang(lang.c_str());
                retVal = insertScript(defScript);
                defScript->setScriptText(defaultMessage);
                if (oldThreadedScript)
                {
                    defScript->setThreadedExecution_oldThreads(true);
                    defScript->setExecuteJustOnce_oldThreads(true);
                }
            }
        }
        else
        {
            char defaultMessage[] = "Default script file could not be found!"; // do not use comments ("--"), we want to
                                                                               // cause an execution error!
            CScriptObject *defScript = new CScriptObject(scriptType);
            retVal = insertScript(defScript);
            defScript->setScriptText(defaultMessage);
            if (oldThreadedScript)
            {
                defScript->setThreadedExecution_oldThreads(true);
                defScript->setExecuteJustOnce_oldThreads(true);
            }
        }
    }
#ifdef SIM_WITH_GUI
    GuiApp::setLightDialogRefreshFlag();
#endif
    return (retVal);
}

int CEmbeddedScriptContainer::getEquivalentScriptExecPriority_old(int objectHandle) const
{                    // for backward compatibility
    int retVal = -1; // no script attached
    CScriptObject *it = getScriptFromObjectAttachedTo(sim_scripttype_simulation, objectHandle);
    if (it != nullptr)
        retVal = it->getExecutionPriority_old();
    it = getScriptFromObjectAttachedTo(sim_scripttype_customization, objectHandle);
    if (it != nullptr)
        retVal = it->getExecutionPriority_old();
    return (retVal);
}

void CEmbeddedScriptContainer::sceneOrModelAboutToBeSaved_old(int modelBase)
{
    CSceneObject *obj = App::currentWorld->sceneObjects->getObjectFromHandle(modelBase);
    if (obj != nullptr)
    {
        std::vector<CSceneObject *> toExplore;
        toExplore.push_back(obj);
        while (toExplore.size() != 0)
        {
            obj = toExplore[toExplore.size() - 1];
            toExplore.pop_back();
            CScriptObject *it =
                getScriptFromObjectAttachedTo(sim_scripttype_customization, obj->getObjectHandle());
            if (it != nullptr)
            {
                if (it->getCustomizationScriptCleanupBeforeSave_DEPRECATED())
                    it->resetScript();
            }
            for (size_t i = 0; i < obj->getChildCount(); i++)
                toExplore.push_back(obj->getChildFromIndex(i));
        }
    }
    else
    {
        for (size_t i = 0; i < allScripts.size(); i++)
        {
            CScriptObject *it = allScripts[i];
            if (it->getScriptType() == sim_scripttype_customization)
            {
                if (it->getCustomizationScriptCleanupBeforeSave_DEPRECATED())
                    it->resetScript();
            }
        }
    }
}

bool CEmbeddedScriptContainer::shouldTemporarilySuspendMainScript()
{
    bool retVal = false;
    std::vector<int> scriptHandles;
    App::currentWorld->sceneObjects->getScriptsToExecute(scriptHandles, -1, true, false);
    for (size_t i = 0; i < scriptHandles.size(); i++)
    {
        CScriptObject *it = getScriptObjectFromHandle(scriptHandles[i]);
        if (it != nullptr)
        { // could have been erased in the mean time!
            if (it->shouldTemporarilySuspendMainScript())
                retVal = true;
        }
    }
    return (retVal);
}

int CEmbeddedScriptContainer::callScripts_noMainScript(int scriptType, int callTypeOrResumeLocation,
                                                          CInterfaceStack *inStack, CInterfaceStack *outStack,
                                                          CSceneObject *objectBranch /*=nullptr*/,
                                                          int scriptToExclude /*=-1*/)
{ // ignores the main script
    int cnt = 0;
    std::vector<int> scriptHandles;

    if (objectBranch == nullptr)
        App::currentWorld->sceneObjects->getScriptsToExecute(scriptHandles, scriptType, true, CScriptObject::isSystemCallbackInReverseOrder(callTypeOrResumeLocation));
    else
        objectBranch->getScriptsInChain(scriptHandles, scriptType, true);

    bool canInterrupt = CScriptObject::isSystemCallbackInterruptible(callTypeOrResumeLocation);
    for (size_t i = 0; i < scriptHandles.size(); i++)
    {
        CScriptObject *script = getScriptObjectFromHandle(scriptHandles[i]);
        if ((script != nullptr) && (script->getScriptHandle() != scriptToExclude))
        { // the script could have been erased in the mean time
            if (script->getThreadedExecution_oldThreads())
            { // is an old, threaded script
                if (callTypeOrResumeLocation == sim_scriptthreadresume_launch)
                {
                    if (script->launchThreadedChildScript_oldThreads())
                        cnt++;
                }
                else
                    cnt += script->resumeThreadedChildScriptIfLocationMatch_oldThreads(callTypeOrResumeLocation);
            }
            else if (script->hasSystemFunctionOrHook(callTypeOrResumeLocation) || script->getOldCallMode())
            { // has the function
                if (script->systemCallScript(callTypeOrResumeLocation, inStack, outStack) == 1)
                {
                    cnt++;
                    if (canInterrupt && (outStack != nullptr) && (outStack->getStackSize() != 0))
                        break;
                }
            }
            else
            { // has not the function. Check if we need to support old callbacks:
                int compatCall = -1;
                if (callTypeOrResumeLocation == sim_syscb_dyn)
                    compatCall = sim_syscb_dyncallback;
                if (callTypeOrResumeLocation == sim_syscb_contact)
                    compatCall = sim_syscb_contactcallback;
                if ((compatCall != -1) && script->hasSystemFunctionOrHook(compatCall))
                {
                    if (script->systemCallScript(compatCall, inStack, outStack) == 1)
                    {
                        cnt++;
                        if (canInterrupt && (outStack != nullptr) && (outStack->getStackSize() != 0))
                            break;
                    }
                }
            }
        }
    }
    return (cnt);
}

bool CEmbeddedScriptContainer::addCommandToOutsideCommandQueues(int commandID, int auxVal1, int auxVal2, int auxVal3,
                                                                int auxVal4, const double aux2Vals[8], int aux2Count)
{
    for (size_t i = 0; i < allScripts.size(); i++)
    {
        if (!allScripts[i]->getFlaggedForDestruction())
            allScripts[i]->addCommandToOutsideCommandQueue(commandID, auxVal1, auxVal2, auxVal3, auxVal4, aux2Vals,
                                                           aux2Count);
    }
    return (true);
}

void CEmbeddedScriptContainer::pushObjectGenesisEvents() const
{
    for (size_t i = 0; i < allScripts.size(); i++)
    {
        CScriptObject* it = allScripts[i];
        if (!it->getFlaggedForDestruction())
        {
            CCbor *ev = App::worldContainer->createEvent(EVENTTYPE_OBJECTADDED, it->getScriptHandle(), it->getScriptUid(), nullptr, false);
            it->addSpecializedObjectEventData(ev);
            App::worldContainer->pushEvent();
        }
    }
}
