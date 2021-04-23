#include "simInternal.h"
#include "embeddedScriptContainer.h"
#include "tt.h"
#include "vVarious.h"
#include "threadPool.h"
#include "vFileFinder.h"
#include "simStrings.h"
#include "app.h"
#include "vDateTime.h"

CEmbeddedScriptContainer::CEmbeddedScriptContainer()
{
    insertDefaultScript(sim_scripttype_mainscript,false,false);
}

CEmbeddedScriptContainer::~CEmbeddedScriptContainer()
{ // beware, the current world could be nullptr
    removeAllScripts();
    for (size_t i=0;i<_callbackStructureToDestroyAtEndOfSimulation_new.size();i++)
        delete _callbackStructureToDestroyAtEndOfSimulation_new[i];
    for (size_t i=0;i<_callbackStructureToDestroyAtEndOfSimulation_old.size();i++)
        delete _callbackStructureToDestroyAtEndOfSimulation_old[i];
    broadcastDataContainer.removeAllObjects();
}

void CEmbeddedScriptContainer::simulationAboutToStart()
{
    broadcastDataContainer.simulationAboutToStart();
    for (size_t i=0;i<allScripts.size();i++)
        allScripts[i]->simulationAboutToStart();
}

void CEmbeddedScriptContainer::simulationEnded()
{
    for (size_t i=0;i<allScripts.size();i++)
        allScripts[i]->simulationEnded();

    broadcastDataContainer.simulationEnded();
    removeDestroyedScripts(sim_scripttype_mainscript);
    removeDestroyedScripts(sim_scripttype_childscript);
    for (size_t i=0;i<_callbackStructureToDestroyAtEndOfSimulation_new.size();i++)
        delete _callbackStructureToDestroyAtEndOfSimulation_new[i];
    _callbackStructureToDestroyAtEndOfSimulation_new.clear();
    for (size_t i=0;i<_callbackStructureToDestroyAtEndOfSimulation_old.size();i++)
        delete _callbackStructureToDestroyAtEndOfSimulation_old[i];
    _callbackStructureToDestroyAtEndOfSimulation_old.clear();
//  if (_initialValuesInitialized&&App::currentWorld->simulation->getResetSceneAtSimulationEnd())
//  {
//  }
}

void CEmbeddedScriptContainer::simulationAboutToEnd()
{
    for (size_t i=0;i<allScripts.size();i++)
        allScripts[i]->simulationAboutToEnd();
}

void CEmbeddedScriptContainer::addCallbackStructureObjectToDestroyAtTheEndOfSimulation_new(SScriptCallBack* object)
{
    _callbackStructureToDestroyAtEndOfSimulation_new.push_back(object);
}
void CEmbeddedScriptContainer::addCallbackStructureObjectToDestroyAtTheEndOfSimulation_old(SLuaCallBack* object)
{
    _callbackStructureToDestroyAtEndOfSimulation_old.push_back(object);
}

void CEmbeddedScriptContainer::resetScriptFlagCalledInThisSimulationStep()
{
    for (size_t i=0;i<allScripts.size();i++)
        allScripts[i]->resetCalledInThisSimulationStep();
}

int CEmbeddedScriptContainer::getCalledScriptsCountInThisSimulationStep(bool onlySimulationScripts)
{
    int cnt=0;
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (allScripts[i]->getCalledInThisSimulationStep())
        {
            if (onlySimulationScripts)
            {
                if (allScripts[i]->getScriptType()==sim_scripttype_mainscript)
                    cnt++;
                if (allScripts[i]->getScriptType()==sim_scripttype_childscript)
                {
                    if (!allScripts[i]->getThreadedExecution_oldThreads()) // ignore old threaded scripts
                        cnt++;
                }
            }
            else
                cnt++;
        }
    }
    return(cnt);
}

int CEmbeddedScriptContainer::removeDestroyedScripts(int scriptType)
{
    TRACE_INTERNAL;
    int retVal=0;
    for (int i=0;i<int(allScripts.size());i++)
    {
        if ( (allScripts[i]->getScriptType()==scriptType)&&allScripts[i]->getFlaggedForDestruction() )
        {
            if ( (!allScripts[i]->getThreadedExecution_oldThreads())||(!allScripts[i]->getThreadedExecutionIsUnderWay_oldThreads()) )
            {
                retVal++;
                CLuaScriptObject* it=allScripts[i];
                it->resetScript(); // should not be done in the destructor!
                allScripts.erase(allScripts.begin()+i);
                i--;
                CLuaScriptObject::destroy(it,true);
            }
        }
    }
    return(retVal);
}

bool CEmbeddedScriptContainer::isContactCallbackFunctionAvailable()
{
    for (size_t i=0;i<allScripts.size();i++)
    {
        CLuaScriptObject* it=allScripts[i];
        if (it->getContainsContactCallbackFunction())
            return(true);
    }
    return(false);
}

bool CEmbeddedScriptContainer::isDynCallbackFunctionAvailable()
{
    for (size_t i=0;i<allScripts.size();i++)
    {
        CLuaScriptObject* it=allScripts[i];
        if (it->getContainsDynCallbackFunction())
            return(true);
    }
    return(false);
}

void CEmbeddedScriptContainer::removeAllScripts()
{
    TRACE_INTERNAL;
    while (allScripts.size()>0)
    {
        CLuaScriptObject* it=allScripts[0];
        it->resetScript(); // should not be done in the destructor!
        allScripts.erase(allScripts.begin());
        CLuaScriptObject::destroy(it,true);
    }
}

void CEmbeddedScriptContainer::killAllSimulationLuaStates()
{
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (allScripts[i]->isSimulationScript())
            allScripts[i]->resetScript();
    }
}

void CEmbeddedScriptContainer::announceObjectWillBeErased(int scriptHandle)
{ // Never called from copy buffer!
    size_t i=0;
    while (i<allScripts.size())
    {
        if (allScripts[i]->announceSceneObjectWillBeErased(scriptHandle,false))
        {
            if (removeScript(allScripts[i]->getScriptHandle()))
                i=0; // ordering may have changed
            else
                i++;
        }
        else
            i++;
    }
}

bool CEmbeddedScriptContainer::removeScript_safe(int scriptHandle)
{ // removal may happen in a delayed fashion
    CLuaScriptObject* it=getScriptFromHandle(scriptHandle);
    if (it==nullptr)
        return(false);
    int res=it->flagScriptForRemoval();
    if (res==0)
        return(false);
    if (res==2)
        removeScript(scriptHandle);
    return(true);
}

bool CEmbeddedScriptContainer::removeScript(int scriptHandle)
{
    TRACE_INTERNAL;
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (allScripts[i]->getScriptHandle()==scriptHandle)
        {
            CLuaScriptObject* it=allScripts[i];
            it->resetScript(); // should not be done in the destructor!
            allScripts.erase(allScripts.begin()+i);
            CLuaScriptObject::destroy(it,true);
            App::worldContainer->setModificationFlag(16384);
            break;
        }
    }
    App::setFullDialogRefreshFlag();
    return(true);
}

CLuaScriptObject* CEmbeddedScriptContainer::getScriptFromHandle(int scriptHandle) const
{
    CLuaScriptObject* retVal=nullptr;
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (allScripts[i]->getScriptHandle()==scriptHandle)
        {
            retVal=allScripts[i];
            break;
        }
    }
    return(retVal);
}

CLuaScriptObject* CEmbeddedScriptContainer::getScriptFromObjectAttachedTo_child(int objectHandle) const
{ // used for child scripts
    if (objectHandle<0)
        return(nullptr); // 10/1/2016
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (allScripts[i]->getObjectHandleThatScriptIsAttachedTo_child()==objectHandle)
            return(allScripts[i]);
    }
    return(nullptr);
}

CLuaScriptObject* CEmbeddedScriptContainer::getScriptFromObjectAttachedTo_customization(int objectHandle) const
{ // used for customization scripts
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (allScripts[i]->getObjectHandleThatScriptIsAttachedTo_customization()==objectHandle)
            return(allScripts[i]);
    }
    return(nullptr);
}

int CEmbeddedScriptContainer::getScriptsFromObjectAttachedTo(int objectHandle,std::vector<CLuaScriptObject*>& scripts) const
{
    scripts.clear();
    CLuaScriptObject* it=getScriptFromObjectAttachedTo_child(objectHandle);
    if (it!=nullptr)
        scripts.push_back(it);
    it=getScriptFromObjectAttachedTo_customization(objectHandle);
    if (it!=nullptr)
        scripts.push_back(it);
    return(int(scripts.size()));
}

CLuaScriptObject* CEmbeddedScriptContainer::getMainScript() const
{
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (allScripts[i]->getScriptType()==sim_scripttype_mainscript)
            return(allScripts[i]);
    }
    return(nullptr);
}

int CEmbeddedScriptContainer::insertScript(CLuaScriptObject* script)
{
    // We make sure the id is unique:
    int newHandle=SIM_IDSTART_LUASCRIPT;
    while (getScriptFromHandle(newHandle)!=nullptr)
        newHandle++;
    script->setScriptHandle(newHandle);
    allScripts.push_back(script);
    App::worldContainer->setModificationFlag(8192);
    return(newHandle);
}

int CEmbeddedScriptContainer::insertDefaultScript(int scriptType,bool threaded,bool oldThreadedScript)
{ 
    if (scriptType!=sim_scripttype_childscript)
        oldThreadedScript=false; // just to make sure
    int retVal=-1;
    std::string filenameAndPath(App::folders->getSystemPath()+"/");

    if (scriptType==sim_scripttype_mainscript)
    {
        CLuaScriptObject* defScript=new CLuaScriptObject(scriptType);
        retVal=insertScript(defScript);
        defScript->setScriptText(DEFAULT_MAINSCRIPT_CODE);
        filenameAndPath="";
    }
    if (scriptType==sim_scripttype_childscript)
    {
        if (oldThreadedScript)
            filenameAndPath+=DEFAULT_THREADEDCHILDSCRIPTOLD_NAME;
        else
        {
            if (threaded)
                filenameAndPath+=DEFAULT_THREADEDCHILDSCRIPT_NAME;
            else
                filenameAndPath+=DEFAULT_NONTHREADEDCHILDSCRIPT_NAME;
        }
    }
    if (scriptType==sim_scripttype_customizationscript)
    {
        if (threaded)
            filenameAndPath+=DEFAULT_THREADEDCUSTOMIZATIONSCRIPT_NAME;
        else
            filenameAndPath+=DEFAULT_NONTHREADEDCUSTOMIZATIONSCRIPT_NAME;
    }

    if (filenameAndPath.size()>0)
    {
        if (VFile::doesFileExist(filenameAndPath.c_str()))
        {
            try
            {
                VFile file(filenameAndPath.c_str(),VFile::READ|VFile::SHARE_DENY_NONE);
                VArchive archive(&file,VArchive::LOAD);
                unsigned int archiveLength=(unsigned int)file.getLength();
                char* defaultScript=new char[archiveLength+1];
                for (int i=0;i<int(archiveLength);i++)
                    archive >> defaultScript[i];
                defaultScript[archiveLength]=0;
                CLuaScriptObject* defScript=new CLuaScriptObject(scriptType);
                retVal=insertScript(defScript);
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
            catch(VFILE_EXCEPTION_TYPE e)
            {
                VFile::reportAndHandleFileExceptionError(e);
                char defaultMessage[]="Default script file could not be found!"; // do not use comments ("--"), we want to cause an execution error!
                CLuaScriptObject* defScript=new CLuaScriptObject(scriptType);
                retVal=insertScript(defScript);
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
            char defaultMessage[]="Default script file could not be found!"; // do not use comments ("--"), we want to cause an execution error!
            CLuaScriptObject* defScript=new CLuaScriptObject(scriptType);
            retVal=insertScript(defScript);
            defScript->setScriptText(defaultMessage);
            if (oldThreadedScript)
            {
                defScript->setThreadedExecution_oldThreads(true);
                defScript->setExecuteJustOnce_oldThreads(true);
            }
        }
    }
    App::setLightDialogRefreshFlag();
    return(retVal);
}

void CEmbeddedScriptContainer::callScripts(int callType,CInterfaceStack* inStack)
{
    TRACE_INTERNAL;
    if (!App::currentWorld->simulation->isSimulationStopped())
    {
        CLuaScriptObject* script=getMainScript();
        if (script!=nullptr)
        {
            script->systemCallMainScript(callType,inStack,nullptr);
            handleCascadedScriptExecution(sim_scripttype_childscript,callType,inStack,nullptr,nullptr);
        }
    }
    handleCascadedScriptExecution(sim_scripttype_customizationscript,callType,inStack,nullptr,nullptr);
}

void CEmbeddedScriptContainer::sceneOrModelAboutToBeSaved(int modelBase)
{
    CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(modelBase);
    if (obj!=nullptr)
    {
        std::vector<CSceneObject*> toExplore;
        toExplore.push_back(obj);
        while (toExplore.size()!=0)
        {
            obj=toExplore[toExplore.size()-1];
            toExplore.pop_back();
            CLuaScriptObject* it=getScriptFromObjectAttachedTo_customization(obj->getObjectHandle());
            if (it!=nullptr)
            {
                if (it->getCustomizationScriptCleanupBeforeSave_DEPRECATED())
                    it->resetScript();
            }
            for (size_t i=0;i<obj->getChildCount();i++)
                toExplore.push_back(obj->getChildFromIndex(i));
        }
    }
    else
    {
        for (size_t i=0;i<allScripts.size();i++)
        {
            CLuaScriptObject* it=allScripts[i];
            if (it->getScriptType()==sim_scripttype_customizationscript)
            {
                if (it->getCustomizationScriptCleanupBeforeSave_DEPRECATED())
                    it->resetScript();
            }
        }
    }
}

int CEmbeddedScriptContainer::_getScriptsToExecute(int scriptType,std::vector<CLuaScriptObject*>& scripts,std::vector<int>& uniqueIds) const
{
    std::vector<CSceneObject*> orderFirst;
    std::vector<CSceneObject*> orderNormal;
    std::vector<CSceneObject*> orderLast;
    std::vector<std::vector<CSceneObject*>* > toHandle;
    toHandle.push_back(&orderFirst);
    toHandle.push_back(&orderNormal);
    toHandle.push_back(&orderLast);
    if ( (scriptType!=sim_scripttype_customizationscript)||App::userSettings->runCustomizationScripts )
    {
        for (size_t i=0;i<App::currentWorld->sceneObjects->getOrphanCount();i++)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getOrphanFromIndex(i);
            toHandle[it->getScriptExecutionOrder(scriptType)]->push_back(it);
        }
        for (size_t i=0;i<toHandle.size();i++)
        {
            for (size_t j=0;j<toHandle[i]->size();j++)
                toHandle[i]->at(j)->getScriptsToExecute(scriptType,sim_scripttreetraversal_reverse,scripts,uniqueIds);
        }
    }
    return(int(scripts.size()));
}

bool CEmbeddedScriptContainer::doesScriptWithUniqueIdExist(int id) const
{
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (allScripts[i]->getScriptUniqueID()==id)
            return(true);
    }
    return(false);
}

bool CEmbeddedScriptContainer::shouldTemporarilySuspendMainScript()
{
    bool retVal=false;
    std::vector<CLuaScriptObject*> scripts;
    std::vector<int> uniqueIds;
    _getScriptsToExecute(sim_scripttype_customizationscript,scripts,uniqueIds);
    for (size_t i=0;i<scripts.size();i++)
    {
        if (doesScriptWithUniqueIdExist(uniqueIds[i]))
        { // the script could have been erased in the mean time
            if (scripts[i]->shouldTemporarilySuspendMainScript())
                retVal=true;
        }
    }
    return(retVal);
}

int CEmbeddedScriptContainer::handleCascadedScriptExecution(int scriptType,int callTypeOrResumeLocation,CInterfaceStack* inStack,CInterfaceStack* outStack,int* retInfo)
{
    int cnt=0;
    if (retInfo!=nullptr)
        retInfo[0]=0;
    std::vector<CLuaScriptObject*> scripts;
    std::vector<int> uniqueIds;
    _getScriptsToExecute(scriptType,scripts,uniqueIds);
    for (size_t i=0;i<scripts.size();i++)
    {
        if (doesScriptWithUniqueIdExist(uniqueIds[i]))
        { // the script could have been erased in the mean time
            CLuaScriptObject* script=scripts[i];
            if (!script->getScriptIsDisabled())
            {
                if (scriptType==sim_scripttype_customizationscript)
                {
                    bool doIt=true;
                    if ( (callTypeOrResumeLocation==sim_syscb_dyncallback)&&(!script->getContainsDynCallbackFunction()) )
                        doIt=false;
                    if ( (callTypeOrResumeLocation==sim_syscb_contactcallback)&&(!script->getContainsContactCallbackFunction()) )
                        doIt=false;
                    if (doIt)
                    {
                        if (script->systemCallScript(callTypeOrResumeLocation,inStack,outStack)==1)
                        {
                            cnt++;
                            if (callTypeOrResumeLocation==sim_syscb_contactcallback)
                            {
                                if (retInfo!=nullptr)
                                    retInfo[0]=1;
                                break;
                            }
                        }
                    }
                }
                else if ((scriptType&sim_scripttype_childscript)!=0)
                {
                    if (script->getThreadedExecution_oldThreads())
                    {
                        if (callTypeOrResumeLocation==sim_scriptthreadresume_launch)
                        {
                            if (script->launchThreadedChildScript_oldThreads())
                                cnt++;
                        }
                        else
                            cnt+=script->resumeThreadedChildScriptIfLocationMatch_oldThreads(callTypeOrResumeLocation);
                    }
                    else
                    {
                        bool doIt=true;
                        if ( (callTypeOrResumeLocation==sim_syscb_dyncallback)&&(!script->getContainsDynCallbackFunction()) )
                            doIt=false;
                        if ( (callTypeOrResumeLocation==sim_syscb_contactcallback)&&(!script->getContainsContactCallbackFunction()) )
                            doIt=false;
                        if (doIt)
                        {
                            if (script->systemCallScript(callTypeOrResumeLocation,inStack,outStack)==1)
                            {
                                cnt++;
                                if (callTypeOrResumeLocation==sim_syscb_contactcallback)
                                {
                                    if (retInfo!=nullptr)
                                        retInfo[0]=1;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return(cnt);
}

bool CEmbeddedScriptContainer::addCommandToOutsideCommandQueues(int commandID,int auxVal1,int auxVal2,int auxVal3,int auxVal4,const float aux2Vals[8],int aux2Count)
{
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (!allScripts[i]->getFlaggedForDestruction())
            allScripts[i]->addCommandToOutsideCommandQueue(commandID,auxVal1,auxVal2,auxVal3,auxVal4,aux2Vals,aux2Count);
    }
    return(true);
}
