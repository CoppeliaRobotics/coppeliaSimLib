#include "simInternal.h"
#include "tt.h"
#include "vVarious.h"
#include "threadPool.h"
#include "vFileFinder.h"
#include "simStrings.h"
#include "app.h"
#include "vDateTime.h"

CAddOnScriptContainer::CAddOnScriptContainer()
{
    _insertAddOns();
    _prepareAddOnFunctionNames_old();
}

CAddOnScriptContainer::~CAddOnScriptContainer()
{
    removeAllAddOns(); // But add-ons should already have been removed at this stage
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

CLuaScriptObject* CAddOnScriptContainer::getAddOnFromID(int scriptID) const
{
    for (size_t i=0;i<_addOns.size();i++)
    {
        if (_addOns[i]->getScriptHandle()==scriptID)
            return(_addOns[i]);
    }
    return(nullptr);
}

CLuaScriptObject* CAddOnScriptContainer::getAddOnFromName(const char* name) const
{
    for (size_t i=0;i<_addOns.size();i++)
    {
        if (_addOns[i]->getAddOnName().compare(name)==0)
            return(_addOns[i]);
    }
    return(nullptr);
}

int CAddOnScriptContainer::_insertAddOn(CLuaScriptObject* script)
{
    // We make sure the id is unique:
    int newHandle=SIM_IDSTART_ADDONSCRIPT;
    while (getAddOnFromID(newHandle)!=nullptr)
        newHandle++;
    script->setScriptHandle(newHandle);
    _addOns.push_back(script);

    return(newHandle);
}

int CAddOnScriptContainer::_insertAddOns()
{
    int addOnsCount=0;
    VFileFinder finder;
    finder.searchFilesWithExtension(App::folders->getExecutablePath().c_str(),ADDON_EXTENTION);
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
            std::string fp(App::folders->getExecutablePath()+"/");
            fp+=foundItem->name;
            CLuaScriptObject* defScript=new CLuaScriptObject(sim_scripttype_addonscript);
            if (defScript->setScriptTextFromFile(fp.c_str()))
            {
                _insertAddOn(defScript);
                std::string nm(foundItem->name);
                nm.erase(nm.begin(),nm.begin()+at.size());
                nm.erase(nm.end()-strlen(ADDON_EXTENTION)-1,nm.end());
                defScript->setAddOnName(nm.c_str());
                if ( (at.compare(ADDON_SCRIPT_PREFIX1_NOAUTOSTART)==0)||(at.compare(ADDON_SCRIPT_PREFIX2_NOAUTOSTART)==0) )
                    defScript->setScriptState(CLuaScriptObject::scriptState_ended);
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
            fp=App::folders->getExecutablePath()+"/";
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
                CLuaScriptObject* defScript=new CLuaScriptObject(sim_scripttype_addonscript);
                _insertAddOn(defScript);
                defScript->setScriptText(script);

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
    finder.searchFilesWithExtension(App::folders->getExecutablePath().c_str(),ADDON_EXTENTION);
    int cnt=0;
    SFileOrFolder* foundItem=finder.getFoundItem(cnt);
    while (foundItem!=nullptr)
    {
        if (foundItem->name.find(ADDON_FUNCTION_PREFIX1)==0)
        {
            std::string nm(foundItem->name);
            nm.erase(nm.begin(),nm.begin()+strlen(ADDON_FUNCTION_PREFIX1));
            nm.erase(nm.end()-strlen(ADDON_EXTENTION)-1,nm.end());
            _allAddOnFunctionNames_old.push_back(nm);
            addOnsCount++;
        }
        else
        {
            if (foundItem->name.find(ADDON_FUNCTION_PREFIX2)==0)
            {
                std::string nm(foundItem->name);
                nm.erase(nm.begin(),nm.begin()+strlen(ADDON_FUNCTION_PREFIX2));
                nm.erase(nm.end()-strlen(ADDON_EXTENTION)-1,nm.end());
                _allAddOnFunctionNames_old.push_back(nm);
                addOnsCount++;
            }
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
    for (size_t i=0;i<_addOns.size();i++)
    {
        CLuaScriptObject* it=_addOns[i];
        if (it->systemCallScript(callType,inStack,outStack)==1)
            retVal++;
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
            CLuaScriptObject* it=_addOns[i];
            it->resetScript(); // should not be done in the destructor!
            _addOns.erase(_addOns.begin()+i);
            delete it;
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
        CLuaScriptObject* it=_addOns[0];
        it->resetScript(); // should not be done in the destructor!
        _addOns.erase(_addOns.begin());
        delete it;
    }
}

bool CAddOnScriptContainer::processCommand(int commandID)
{ // Return value is true if the command belonged to hierarchy menu and was executed
    if ( (commandID>=SCRIPT_CONT_COMMANDS_ADDON_SCRIPT_MENU_ITEM_START_SCCMD)&&(commandID<=SCRIPT_CONT_COMMANDS_ADDON_SCRIPT_MENU_ITEM_END_SCCMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            int index=commandID-SCRIPT_CONT_COMMANDS_ADDON_SCRIPT_MENU_ITEM_START_SCCMD;
            int cnt=0;
            CLuaScriptObject* it=nullptr;
            for (size_t i=0;i<_addOns.size();i++)
            {
                it=_addOns[i];
                if (it->getScriptType()==sim_scripttype_addonscript)
                {
                    if (index==cnt)
                        break;
                    it=nullptr;
                    cnt++;
                }
            }
            if (it!=nullptr)
            {
                int st=it->getScriptState();
                int sysCall=-1;
                if ( ((st&CLuaScriptObject::scriptState_error)!=0)||((st&7)!=CLuaScriptObject::scriptState_initialized) )
                {
                    sysCall=sim_syscb_init;
                    it->resetScript();
                }
                if (st==(CLuaScriptObject::scriptState_initialized|CLuaScriptObject::scriptState_suspended))
                    sysCall=sim_syscb_aos_resume;
                if (st==CLuaScriptObject::scriptState_initialized)
                    sysCall=sim_syscb_aos_suspend;
                if (sysCall!=-1)
                    it->systemCallScript(sysCall,nullptr,nullptr,true);
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

// OLD (add-on functions):
    if ( (commandID>=SCRIPT_CONT_COMMANDS_ADDON_FUNCTION_MENU_ITEM_START_SCCMD)&&(commandID<=SCRIPT_CONT_COMMANDS_ADDON_FUNCTION_MENU_ITEM_END_SCCMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            int index=commandID-SCRIPT_CONT_COMMANDS_ADDON_FUNCTION_MENU_ITEM_START_SCCMD;
            if (index<int(_allAddOnFunctionNames_old.size()))
            {
//                std::string txt("Starting add-on function");
//                txt+=" ";
//                txt+=_allAddOnFunctionNames_old[index];
//                App::logMsg(sim_verbosity_msgs,txt.c_str());

                // execute the add-on function here!!
                std::string fp1(App::folders->getExecutablePath()+"/");
                fp1+=ADDON_FUNCTION_PREFIX1;
                fp1+=_allAddOnFunctionNames_old[index];
                fp1+=".";
                fp1+=ADDON_EXTENTION;
                std::string fp2(App::folders->getExecutablePath()+"/");
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
                        CLuaScriptObject* defScript=new CLuaScriptObject(sim_scripttype_addonfunction);
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
    //                  printf("CoppeliaSim error: failed loading add-on script '%s'.\n",foundItem->name.c_str());
    //                  VFile::reportAndHandleFileExceptionError(e);
                    }
                }
               // App::logMsg(sim_verbosity_msgs,"Ended add-on function");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    return(false);
}

#ifdef SIM_WITH_GUI
void CAddOnScriptContainer::addMenu(VMenu* menu)
{
    int id=SCRIPT_CONT_COMMANDS_ADDON_SCRIPT_MENU_ITEM_START_SCCMD;
    for (size_t i=0;i<_addOns.size();i++)
    {
        CLuaScriptObject* it=_addOns[i];
        if (it->getScriptType()==sim_scripttype_addonscript)
        {
            int st=it->getScriptState();
            std::string txt;
            txt=it->getAddOnName();
            if (st==CLuaScriptObject::scriptState_initialized)
                txt+=" (running)";
            if ((st&CLuaScriptObject::scriptState_error)!=0)
                txt+=" (error)";
            if ((st&CLuaScriptObject::scriptState_suspended)!=0)
                txt+=" (suspended)";
            menu->appendMenuItem(true,false,id,txt.c_str());
            id++;
        }
    }

    // OLD (add-on functions):
    id=SCRIPT_CONT_COMMANDS_ADDON_FUNCTION_MENU_ITEM_START_SCCMD;
    for (size_t i=0;i<_allAddOnFunctionNames_old.size();i++)
    {
        menu->appendMenuItem(true,false,id,_allAddOnFunctionNames_old[i].c_str());
        id++;
    }
}
#endif
