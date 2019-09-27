
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "tt.h"
#include "vVarious.h"
#include "threadPool.h"
#include "vFileFinder.h"
#include "v_repStrings.h"
#include "app.h"
#include "vDateTime.h"
#include "funcDebug.h"

CAddOnScriptContainer::CAddOnScriptContainer()
{
    insertAddOnScripts();
    prepareAddOnFunctionNames();
}

CAddOnScriptContainer::~CAddOnScriptContainer()
{
    removeAllScripts(); // But add-ons should already have been removed at this stage
}

void CAddOnScriptContainer::simulationAboutToStart()
{
    for (size_t i=0;i<allAddOnScripts.size();i++)
        allAddOnScripts[i]->simulationAboutToStart();
}

void CAddOnScriptContainer::simulationEnded()
{
    for (size_t i=0;i<allAddOnScripts.size();i++)
        allAddOnScripts[i]->simulationEnded();
}

void CAddOnScriptContainer::simulationAboutToEnd()
{
    for (size_t i=0;i<allAddOnScripts.size();i++)
        allAddOnScripts[i]->simulationAboutToEnd();
}

void CAddOnScriptContainer::setAdditionalAddOnScript(std::string scriptNameWithExtension)
{
    _additionalAddOnScriptAllScenes=scriptNameWithExtension;
}

CLuaScriptObject* CAddOnScriptContainer::getAddOnScriptFromID(int scriptID) const
{
    for (size_t i=0;i<allAddOnScripts.size();i++)
    {
        if (allAddOnScripts[i]->getScriptID()==scriptID)
            return(allAddOnScripts[i]);
    }
    return(nullptr);
}

CLuaScriptObject* CAddOnScriptContainer::getAddOnScriptFromName(const char* name) const
{
    for (size_t i=0;i<allAddOnScripts.size();i++)
    {
        if (allAddOnScripts[i]->getAddOnName().compare(name)==0)
            return(allAddOnScripts[i]);
    }
    return(nullptr);
}

int CAddOnScriptContainer::insertScript(CLuaScriptObject* script)
{
    // We make sure the id is unique:
    int newID=SIM_IDSTART_ADDONSCRIPT;
    while (getAddOnScriptFromID(newID)!=nullptr)
        newID++;
    script->setScriptID(newID);
    allAddOnScripts.push_back(script);

    return(newID);
}

int CAddOnScriptContainer::insertAddOnScripts()
{
    int addOnsCount=0;
    VFileFinder finder;
    finder.searchFilesWithExtension(App::directories->executableDirectory,ADDON_EXTENTION);
    int cnt=0;
    SFileOrFolder* foundItem=finder.getFoundItem(cnt);
    while (foundItem!=nullptr)
    {
        if ( (foundItem->name.find(ADDON_SCRIPT_PREFIX_AUTOSTART)==0)||(foundItem->name.find(ADDON_SCRIPT_PREFIX_NOAUTOSTART)==0) )
        {
            std::string fp(App::directories->executableDirectory+VREP_SLASH);
            fp+=foundItem->name;
            CLuaScriptObject* defScript=new CLuaScriptObject(sim_scripttype_addonscript);
            if (defScript->setScriptTextFromFile(fp.c_str()))
            {
                insertScript(defScript);
                std::string nm(foundItem->name);
                nm.erase(nm.begin(),nm.begin()+strlen(ADDON_SCRIPT_PREFIX_AUTOSTART));
                nm.erase(nm.end()-strlen(ADDON_EXTENTION)-1,nm.end());
                defScript->setAddOnName(nm.c_str());
                if (foundItem->name.find(ADDON_SCRIPT_PREFIX_AUTOSTART)==0)
                    defScript->setAddOnScriptAutoRun();
                addOnsCount++;
                printf("Add-on script '%s' was loaded.\n",foundItem->name.c_str());
            }
            else
                printf("Failed loading add-on script '%s'.\n",foundItem->name.c_str());
        }
        cnt++;
        foundItem=finder.getFoundItem(cnt);
    }

    // Now the additional add-on scripts (this is enabled via command line options -a and -b:
    std::vector<std::string> additionalScripts;
    if (_additionalAddOnScriptAllScenes.length()!=0)
    {
        additionalScripts.push_back(_additionalAddOnScriptAllScenes);
        _additionalAddOnScriptAllScenes="";
    }
    if (_additionalAddOnScriptAllScenes.length()!=0)
        additionalScripts.push_back(_additionalAddOnScriptAllScenes);
    for (size_t addScr=0;addScr<additionalScripts.size();addScr++)
    {
        std::string fp(additionalScripts[addScr]);
        if (!VVarious::isAbsolutePath(fp))
        {
            fp=App::directories->executableDirectory+VREP_SLASH;
            fp+=additionalScripts[addScr];
        }
        std::string fileName_noExtension(VVarious::splitPath_fileBase(fp));
        std::string fileName_withExtension(VVarious::splitPath_fileBaseAndExtension(fp));
        if (VFile::doesFileExist(fp))
        {
            try
            {
                VFile file(fp,VFile::READ|VFile::SHARE_DENY_NONE);
                VArchive archive(&file,VArchive::LOAD);
                unsigned int archiveLength=(unsigned int)file.getLength();
                char* script=new char[archiveLength+1];
                for (int i=0;i<int(archiveLength);i++)
                    archive >> script[i];
                script[archiveLength]=0;
                CLuaScriptObject* defScript=new CLuaScriptObject(sim_scripttype_addonscript);
                insertScript(defScript);
                defScript->setScriptText(script);

                defScript->setAddOnName(fileName_noExtension.c_str());
                defScript->setThreadedExecution(false);
                defScript->setAddOnScriptAutoRun();

                delete[] script;
                archive.close();
                file.close();
                addOnsCount++;
                printf("Add-on script '%s' was loaded.\n",fileName_withExtension.c_str());
            }
            catch(VFILE_EXCEPTION_TYPE e)
            {
                printf("Failed loading add-on script '%s'.\n",fileName_withExtension.c_str());
            }
        }
    }

    return(addOnsCount);
}

int CAddOnScriptContainer::prepareAddOnFunctionNames()
{
    int addOnsCount=0;
    VFileFinder finder;
    finder.searchFilesWithExtension(App::directories->executableDirectory,ADDON_EXTENTION);
    int cnt=0;
    SFileOrFolder* foundItem=finder.getFoundItem(cnt);
    while (foundItem!=nullptr)
    {
        if (foundItem->name.find(ADDON_FUNCTION_PREFIX)==0)
        {
            std::string nm(foundItem->name);
            nm.erase(nm.begin(),nm.begin()+strlen(ADDON_FUNCTION_PREFIX));
            nm.erase(nm.end()-strlen(ADDON_EXTENTION)-1,nm.end());
            allAddOnFunctionNames.push_back(nm);
            addOnsCount++;
        }
        cnt++;
        foundItem=finder.getFoundItem(cnt);
    }
    return(addOnsCount);
}

bool CAddOnScriptContainer::handleAddOnScriptExecution_beforeMainScript()
{ // return true: run main script, otherwise, do not run main script
    bool retVal=true;

    for (size_t i=0;i<allAddOnScripts.size();i++)
    {
        CLuaScriptObject* it=allAddOnScripts[i];
        if (it->getScriptType()==sim_scripttype_addonscript)
        {
            CInterfaceStack outStack;
            it->runAddOn(sim_syscb_beforemainscript,nullptr,&outStack);
            bool doNotRunMainScript;
            if (outStack.getStackMapBoolValue("doNotRunMainScript",doNotRunMainScript))
            {
                if (doNotRunMainScript)
                    retVal=false;
            }
        }
    }
    return(retVal);
}

int CAddOnScriptContainer::handleAddOnScriptExecution(int callType,CInterfaceStack* inStack,CInterfaceStack* outStack)
{
    int retVal=0;
    for (size_t i=0;i<allAddOnScripts.size();i++)
    {
        CLuaScriptObject* it=allAddOnScripts[i];
        if (it->getScriptType()==sim_scripttype_addonscript)
        {
            if (it->runAddOn(callType,inStack,outStack)==1)
                retVal++;
        }
    }
    return(retVal);
}

bool CAddOnScriptContainer::removeScript(int scriptID)
{
    FUNCTION_DEBUG;
    for (size_t i=0;i<allAddOnScripts.size();i++)
    {
        if (allAddOnScripts[i]->getScriptID()==scriptID)
        {
            CLuaScriptObject* it=allAddOnScripts[i];
            it->killLuaState(); // should not be done in the destructor!
            allAddOnScripts.erase(allAddOnScripts.begin()+i);
            delete it;
            break;
        }
    }
    App::setFullDialogRefreshFlag();
    return(true);
}

void CAddOnScriptContainer::removeAllScripts()
{
    while (allAddOnScripts.size()>0)
    {
        CLuaScriptObject* it=allAddOnScripts[0];
        it->killLuaState(); // should not be done in the destructor!
        allAddOnScripts.erase(allAddOnScripts.begin());
        delete it;
    }
}

bool CAddOnScriptContainer::processCommand(int commandID)
{ // Return value is true if the command belonged to hierarchy menu and was executed
    if ( (commandID>=SCRIPT_CONT_COMMANDS_ADDON_FUNCTION_MENU_ITEM_START_SCCMD)&&(commandID<=SCRIPT_CONT_COMMANDS_ADDON_FUNCTION_MENU_ITEM_END_SCCMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            int index=commandID-SCRIPT_CONT_COMMANDS_ADDON_FUNCTION_MENU_ITEM_START_SCCMD;
            if (index<int(allAddOnFunctionNames.size()))
            {
                std::string txt(IDSNS_STARTING_ADDON_FUNCTION);
                txt+=" ";
                txt+=allAddOnFunctionNames[index];
                App::addStatusbarMessage(txt.c_str());

                // execute the add-on function here!!
                std::string fp(App::directories->executableDirectory+VREP_SLASH);
                fp+=ADDON_FUNCTION_PREFIX;
                fp+=allAddOnFunctionNames[index];
                fp+=".";
                fp+=ADDON_EXTENTION;
                if (VFile::doesFileExist(fp))
                {
                    try
                    {
                        VFile file(fp,VFile::READ|VFile::SHARE_DENY_NONE);
                        VArchive archive(&file,VArchive::LOAD);
                        unsigned int archiveLength=(unsigned int)file.getLength();
                        char* script=new char[archiveLength+1];
                        for (int i=0;i<int(archiveLength);i++)
                            archive >> script[i];
                        script[archiveLength]=0;
                        CLuaScriptObject* defScript=new CLuaScriptObject(sim_scripttype_addonfunction);
                        int scriptID=insertScript(defScript);
                        defScript->setScriptText(script);
                        defScript->setAddOnName(allAddOnFunctionNames[index].c_str());
                        defScript->setThreadedExecution(false);
                        defScript->runAddOn(sim_syscb_init,nullptr,nullptr);
                        delete[] script;
                        archive.close();
                        file.close();

                        removeScript(scriptID);
                    }
                    catch(VFILE_EXCEPTION_TYPE e)
                    {
    //                  printf("Failed loading add-on script '%s'.\n",foundItem->name.c_str());
    //                  VFile::reportAndHandleFileExceptionError(e);
                    }
                }
                App::addStatusbarMessage(IDSNS_ENDED_ADDON_FUNCTION);
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

    if ( (commandID>=SCRIPT_CONT_COMMANDS_ADDON_SCRIPT_MENU_ITEM_START_SCCMD)&&(commandID<=SCRIPT_CONT_COMMANDS_ADDON_SCRIPT_MENU_ITEM_END_SCCMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            int index=commandID-SCRIPT_CONT_COMMANDS_ADDON_SCRIPT_MENU_ITEM_START_SCCMD;
            int cnt=0;
            CLuaScriptObject* it=nullptr;
            for (size_t i=0;i<allAddOnScripts.size();i++)
            {
                it=allAddOnScripts[i];
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
                std::string txt;
                int st=it->getAddOnExecutionState();
                if (st==sim_syscb_aos_suspend)
                {
                    it->runAddOn(sim_syscb_aos_resume,nullptr,nullptr);
                    txt=IDSNS_RESUMED_ADDON_SCRIPT;
                }
                if (st==sim_syscb_aos_run)
                {
                    it->runAddOn(sim_syscb_aos_suspend,nullptr,nullptr);
                    txt=IDSNS_PAUSED_ADDON_SCRIPT;
                }
                if (st==sim_syscb_init)
                {
                    it->runAddOn(sim_syscb_init,nullptr,nullptr);
                    txt=IDSNS_STARTED_ADDON_SCRIPT;
                }
                txt+=" ";
                txt+=it->getAddOnName();
                App::addStatusbarMessage(txt.c_str());
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
    // First add-on functions:
    int id=SCRIPT_CONT_COMMANDS_ADDON_FUNCTION_MENU_ITEM_START_SCCMD;
    for (size_t i=0;i<allAddOnFunctionNames.size();i++)
    {
        menu->appendMenuItem(true,false,id,allAddOnFunctionNames[i].c_str());
        id++;
    }

    menu->appendMenuSeparator();

    // Now add-on scripts:
    id=SCRIPT_CONT_COMMANDS_ADDON_SCRIPT_MENU_ITEM_START_SCCMD;
    for (size_t i=0;i<allAddOnScripts.size();i++)
    {
        CLuaScriptObject* it=allAddOnScripts[i];
        if (it->getScriptType()==sim_scripttype_addonscript)
        {
            int st=it->getAddOnExecutionState();
            std::string txt;
            txt=it->getAddOnName();
            if (st==sim_syscb_aos_suspend)
                txt+=" (suspended)";
            if (st==sim_syscb_aos_run)
                txt+=" (running)";
            if (st==sim_syscb_init)
                txt+=" (stopped)";
            menu->appendMenuItem(true,st==sim_syscb_aos_run,id,txt.c_str(),true);
            id++;
        }
    }
}
#endif
