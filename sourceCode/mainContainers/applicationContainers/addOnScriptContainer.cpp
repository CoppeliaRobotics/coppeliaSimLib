#include "simInternal.h"
#include "tt.h"
#include "vVarious.h"
#include "threadPool_old.h"
#include "vFileFinder.h"
#include "simStrings.h"
#include "app.h"
#include "vDateTime.h"

CAddOnScriptContainer::CAddOnScriptContainer()
{
    _nextScriptHandle=SIM_IDSTART_ADDONSCRIPT;
    _contactFuncCount=0;
    _dynFuncCount=0;
    _eventFuncCount=0;
    _jointFuncCount=0;
    _insertAddOns();
    _prepareAddOnFunctionNames_old();
}

CAddOnScriptContainer::~CAddOnScriptContainer()
{
    removeAllAddOns(); // But add-ons should already have been removed at this stage
}

int CAddOnScriptContainer::getContactFuncCount() const
{
    return(_contactFuncCount);
}

void CAddOnScriptContainer::setContactFuncCount(int cnt)
{
    _contactFuncCount=cnt;
}

int CAddOnScriptContainer::getDynFuncCount() const
{
    return(_dynFuncCount);
}

void CAddOnScriptContainer::setDynFuncCount(int cnt)
{
    _dynFuncCount=cnt;
}

int CAddOnScriptContainer::getEventFuncCount() const
{
    return(_eventFuncCount);
}

void CAddOnScriptContainer::setEventFuncCount(int cnt)
{
    _eventFuncCount=cnt;
}

int CAddOnScriptContainer::getJointFuncCount() const
{
    return(_jointFuncCount);
}

void CAddOnScriptContainer::setJointFuncCount(int cnt)
{
    _jointFuncCount=cnt;
}

void CAddOnScriptContainer::simulationAboutToStart()
{
    for (size_t i=0;i<_addOns.size();i++)
        _addOns[i]->simulationAboutToStart();
}

void CAddOnScriptContainer::simulationEnded()
{
    for (size_t i=0;i<_addOns.size();i++)
        _addOns[i]->simulationEnded();
}

void CAddOnScriptContainer::simulationAboutToEnd()
{
    for (size_t i=0;i<_addOns.size();i++)
        _addOns[i]->simulationAboutToEnd();
}

CScriptObject* CAddOnScriptContainer::getAddOnFromID(int scriptID) const
{
    for (size_t i=0;i<_addOns.size();i++)
    {
        if (_addOns[i]->getScriptHandle()==scriptID)
            return(_addOns[i]);
    }
    return(nullptr);
}

CScriptObject* CAddOnScriptContainer::getAddOnFromName(const char* name) const
{
    for (size_t i=0;i<_addOns.size();i++)
    {
        if (_addOns[i]->getScriptName().compare(name)==0)
            return(_addOns[i]);
    }
    return(nullptr);
}

int CAddOnScriptContainer::_insertAddOn(CScriptObject* script)
{
    script->setScriptHandle(_nextScriptHandle++);
    _addOns.push_back(script);
    return(_nextScriptHandle-1);
}

int CAddOnScriptContainer::_insertAddOns()
{
    int addOnsCount=0;
    VFileFinder finder;
    finder.searchFilesWithExtension(App::folders->getAddOnPath().c_str(),ADDON_EXTENTION);
    int cnt=0;
    SFileOrFolder* foundItem=finder.getFoundItem(cnt);
    while (foundItem!=nullptr)
    {
        bool oldAddOn=true;
        std::string at;
        if (foundItem->name.find(ADDON_SCRIPT_PREFIX1_AUTOSTART)==0)
            at=ADDON_SCRIPT_PREFIX1_AUTOSTART;
        if (foundItem->name.find(ADDON_SCRIPT_PREFIX1_NOAUTOSTART)==0)
            at=ADDON_SCRIPT_PREFIX1_NOAUTOSTART;
        if (foundItem->name.find(ADDON_SCRIPT_PREFIX2_AUTOSTART)==0)
            at=ADDON_SCRIPT_PREFIX2_AUTOSTART;
        if (foundItem->name.find(ADDON_SCRIPT_PREFIX2_NOAUTOSTART)==0)
            at=ADDON_SCRIPT_PREFIX2_NOAUTOSTART;
        if ( (foundItem->name.find(ADDON_FUNCTION_PREFIX1)==0)||(foundItem->name.find(ADDON_FUNCTION_PREFIX2)==0) )
            at="X";
        if ( (at.size()==0)&&(foundItem->name.find(ADDON_PREFIX)==0) )
        {
            at=ADDON_PREFIX;
            oldAddOn=false;
        }
        if (at.size()>1)
        {
            std::string fp(App::folders->getAddOnPath()+"/");
            fp+=foundItem->name;
            CScriptObject* defScript=new CScriptObject(sim_scripttype_addonscript);
            if (defScript->setScriptTextFromFile(fp.c_str()))
            {
                defScript->setAddOnFilePath(fp.c_str());
                _insertAddOn(defScript);
                std::string nm(foundItem->name);
                nm.erase(nm.begin(),nm.begin()+at.size());
                nm.erase(nm.end()-strlen(ADDON_EXTENTION)-1,nm.end());
                defScript->setAddOnName(nm.c_str());
                if ( (at.compare(ADDON_SCRIPT_PREFIX1_NOAUTOSTART)==0)||(at.compare(ADDON_SCRIPT_PREFIX2_NOAUTOSTART)==0) )
                    defScript->setScriptState(CScriptObject::scriptState_ended);
                App::logMsg(sim_verbosity_infos,"add-on '%s' was loaded.",foundItem->name.c_str());
                addOnsCount++;
            }
            else
                App::logMsg(sim_verbosity_errors,"failed loading add-on '%s'.",foundItem->name.c_str());
        }
        cnt++;
        foundItem=finder.getFoundItem(cnt);
    }

    // Now the additional add-ons (this is enabled via command line options -a and -b:
    std::vector<std::string> additionalScripts;
    if (App::getAdditionalAddOnScript1().length()>0)
    {
        additionalScripts.push_back(App::getAdditionalAddOnScript1());
        App::setAdditionalAddOnScript1("");
    }
    if (App::getAdditionalAddOnScript2().length()>0)
    {
        additionalScripts.push_back(App::getAdditionalAddOnScript2());
        App::setAdditionalAddOnScript2("");
    }
    for (size_t addScr=0;addScr<additionalScripts.size();addScr++)
    {
        std::string fp(additionalScripts[addScr]);
        if (!VVarious::isAbsolutePath(fp.c_str()))
        {
            fp=App::folders->getAddOnPath()+"/";
            fp+=additionalScripts[addScr];
        }
        std::string fileName_noExtension(VVarious::splitPath_fileBase(fp.c_str()));
        std::string fileName_withExtension(VVarious::splitPath_fileBaseAndExtension(fp.c_str()));
        if (VFile::doesFileExist(fp.c_str()))
        {
            try
            {
                VFile file(fp.c_str(),VFile::READ|VFile::SHARE_DENY_NONE);
                VArchive archive(&file,VArchive::LOAD);
                unsigned int archiveLength=(unsigned int)file.getLength();
                char* script=new char[archiveLength+1];
                for (int i=0;i<int(archiveLength);i++)
                    archive >> script[i];
                script[archiveLength]=0;
                CScriptObject* defScript=new CScriptObject(sim_scripttype_addonscript);
                defScript->setAddOnFilePath(fp.c_str());
                _insertAddOn(defScript);
                std::string sc(script);
                defScript->setScriptText(sc.c_str());

                defScript->setAddOnName(fileName_noExtension.c_str());
                defScript->setThreadedExecution_oldThreads(false);

                delete[] script;
                archive.close();
                file.close();

                addOnsCount++;
                App::logMsg(sim_verbosity_infos,"add-on '%s' was loaded.",fileName_withExtension.c_str());
            }
            catch(VFILE_EXCEPTION_TYPE e)
            {
                App::logMsg(sim_verbosity_errors,"failed loading add-on '%s'.",fileName_withExtension.c_str());
            }
        }
        else
            App::logMsg(sim_verbosity_errors,"failed loading add-on '%s'.",fileName_withExtension.c_str());
    }

    return(addOnsCount);
}

int CAddOnScriptContainer::_prepareAddOnFunctionNames_old()
{
    int addOnsCount=0;
    VFileFinder finder;
    finder.searchFilesWithExtension(App::folders->getAddOnPath().c_str(),ADDON_EXTENTION);
    int cnt=0;
    SFileOrFolder* foundItem=finder.getFoundItem(cnt);
    while (foundItem!=nullptr)
    {
        std::string pref;
        if (foundItem->name.find(ADDON_FUNCTION_PREFIX1)==0)
            pref=ADDON_FUNCTION_PREFIX1;
        if (foundItem->name.find(ADDON_FUNCTION_PREFIX2)==0)
            pref=ADDON_FUNCTION_PREFIX2;
        if (pref.size()>0)
        {
            std::string nm(foundItem->name);
            nm.erase(nm.begin(),nm.begin()+pref.size());
            nm.erase(nm.end()-strlen(ADDON_EXTENTION)-1,nm.end());
            _allAddOnFunctionNames_old.push_back(nm);
            nm="Compatibility mode add-on functions\n"+nm;
            _allAddOnFunctionUiHandles_old.push_back(App::worldContainer->moduleMenuItemContainer->addMenuItem(nm.c_str(),-1));
            addOnsCount++;
        }
        cnt++;
        foundItem=finder.getFoundItem(cnt);
    }
    return(addOnsCount);
}

bool CAddOnScriptContainer::shouldTemporarilySuspendMainScript()
{ // return false: run main script, otherwise, do not run main script
    bool retVal=false;
    for (size_t i=0;i<_addOns.size();i++)
    {
        if (_addOns[i]->shouldTemporarilySuspendMainScript())
            retVal=true;
    }
    return(retVal);
}

int CAddOnScriptContainer::callScripts(int callType,CInterfaceStack* inStack,CInterfaceStack* outStack)
{
    int retVal=0;
    std::vector<CScriptObject*> scripts;
    std::vector<CScriptObject*> scripts_normal;
    std::vector<CScriptObject*> scripts_last;
    for (size_t i=0;i<_addOns.size();i++)
    {
        CScriptObject* it=_addOns[i];
        if (it->getScriptExecPriority()==sim_scriptexecorder_first)
            scripts.push_back(it);
        if (it->getScriptExecPriority()==sim_scriptexecorder_normal)
            scripts_normal.push_back(it);
        if (it->getScriptExecPriority()==sim_scriptexecorder_last)
            scripts_last.push_back(it);
    }
    scripts.insert(scripts.end(),scripts_normal.begin(),scripts_normal.end());
    scripts.insert(scripts.end(),scripts_last.begin(),scripts_last.end());
    bool interruptible=CScriptObject::isSystemCallbackInterruptible(callType);
    for (size_t i=0;i<scripts.size();i++)
    {
        CScriptObject* it=scripts[i];
        if ( it->hasFunction(callType)||it->getOldCallMode() )
        {
            if (it->systemCallScript(callType,inStack,outStack)==1)
                retVal++;
        }
        if (interruptible&&(outStack!=nullptr)&&(outStack->getStackSize()!=0))
            break;
    }
    return(retVal);
}

bool CAddOnScriptContainer::_removeAddOn(int scriptID)
{
    TRACE_INTERNAL;
    for (size_t i=0;i<_addOns.size();i++)
    {
        if (_addOns[i]->getScriptHandle()==scriptID)
        {
            CScriptObject* it=_addOns[i];
            it->resetScript(); // should not be done in the destructor!
            _addOns.erase(_addOns.begin()+i);
            CScriptObject::destroy(it,true);
            break;
        }
    }
    App::setFullDialogRefreshFlag();
    return(true);
}

void CAddOnScriptContainer::removeAllAddOns()
{
    while (_addOns.size()>0)
    {
        CScriptObject* it=_addOns[0];
        it->resetScript(); // should not be done in the destructor!
        _addOns.erase(_addOns.begin());
        CScriptObject::destroy(it,true);
    }
}

bool CAddOnScriptContainer::processCommand(int commandID)
{ // Return value is true if the command belonged to hierarchy menu and was executed
    if ( (commandID>=UI_MODULE_MENU_CMDS_START)&&(commandID<=UI_MODULE_MENU_CMDS_END) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            CScriptObject* it=nullptr;
            for (size_t i=0;i<_addOns.size();i++)
            {
                if (_addOns[i]->getAddOnUiMenuHandle()==commandID)
                {
                    it=_addOns[i];
                    break;
                }
            }
            if (it!=nullptr)
            {
                int st=it->getScriptState();
                int sysCall=-1;
                if ( ((st&CScriptObject::scriptState_error)!=0)||((st&7)!=CScriptObject::scriptState_initialized) )
                {
                    sysCall=sim_syscb_init;
                    it->resetScript();
                }
                if (st==(CScriptObject::scriptState_initialized|CScriptObject::scriptState_suspended))
                    sysCall=sim_syscb_aos_resume;
                if (st==CScriptObject::scriptState_initialized)
                    sysCall=sim_syscb_aos_suspend;
                if (sysCall!=-1)
                    it->systemCallScript(sysCall,nullptr,nullptr,true);
                return(true);
            }
            else
            { // OLD add-on functions:
                int index=-1;
                for (size_t i=0;i<_allAddOnFunctionUiHandles_old.size();i++)
                {
                    if (_allAddOnFunctionUiHandles_old[i]==commandID)
                    {
                        index=i;
                        break;
                    }
                }
                if (index!=-1)
                {   // execute the add-on function here!!
                    std::string fp1(App::folders->getAddOnPath()+"/");
                    fp1+=ADDON_FUNCTION_PREFIX1;
                    fp1+=_allAddOnFunctionNames_old[index];
                    fp1+=".";
                    fp1+=ADDON_EXTENTION;
                    std::string fp2(App::folders->getAddOnPath()+"/");
                    fp2+=ADDON_FUNCTION_PREFIX2;
                    fp2+=_allAddOnFunctionNames_old[index];
                    fp2+=".";
                    fp2+=ADDON_EXTENTION;
                    std::string fp;
                    if (VFile::doesFileExist(fp1.c_str()))
                        fp=fp1;
                    else
                    {
                        if (VFile::doesFileExist(fp2.c_str()))
                            fp=fp2;
                    }
                    if (fp.size()>0)
                    {
                        try
                        {
                            VFile file(fp.c_str(),VFile::READ|VFile::SHARE_DENY_NONE);
                            VArchive archive(&file,VArchive::LOAD);
                            unsigned int archiveLength=(unsigned int)file.getLength();
                            char* script=new char[archiveLength+1];
                            for (int i=0;i<int(archiveLength);i++)
                                archive >> script[i];
                            script[archiveLength]=0;
                            CScriptObject* defScript=new CScriptObject(sim_scripttype_addonfunction);
                            int scriptID=_insertAddOn(defScript);
                            defScript->setScriptText(script);
                            defScript->setAddOnName(_allAddOnFunctionNames_old[index].c_str());
                            defScript->setThreadedExecution_oldThreads(false);
                            defScript->systemCallScript(sim_syscb_init,nullptr,nullptr);
                            delete[] script;
                            archive.close();
                            file.close();
                            _removeAddOn(scriptID);
                        }
                        catch(VFILE_EXCEPTION_TYPE e)
                        {
                        }
                    }
                    return(true);
                }
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
    }
    return(false);
}
