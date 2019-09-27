
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "luaScriptContainer.h"
#include "tt.h"
#include "vVarious.h"
#include "threadPool.h"
#include "vFileFinder.h"
#include "v_repStrings.h"
#include "app.h"
#include "vDateTime.h"
#include "funcDebug.h"

CLuaScriptContainer::CLuaScriptContainer()
{
    _inMainScriptNow=0;
    insertDefaultScript_mainAndChildScriptsOnly(sim_scripttype_mainscript,false);
}

CLuaScriptContainer::~CLuaScriptContainer()
{
    removeAllScripts();
    for (size_t i=0;i<_callbackStructureToDestroyAtEndOfSimulation_new.size();i++)
        delete _callbackStructureToDestroyAtEndOfSimulation_new[i];
    for (size_t i=0;i<_callbackStructureToDestroyAtEndOfSimulation_old.size();i++)
        delete _callbackStructureToDestroyAtEndOfSimulation_old[i];
    broadcastDataContainer.removeAllObjects();
}

void CLuaScriptContainer::simulationAboutToStart()
{
    broadcastDataContainer.simulationAboutToStart();
    for (size_t i=0;i<allScripts.size();i++)
        allScripts[i]->simulationAboutToStart();
}

void CLuaScriptContainer::simulationEnded()
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
//  if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
//  {
//  }
}

void CLuaScriptContainer::simulationAboutToEnd()
{
    for (size_t i=0;i<allScripts.size();i++)
        allScripts[i]->simulationAboutToEnd();
}

void CLuaScriptContainer::renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib)
{

}

void CLuaScriptContainer::addCallbackStructureObjectToDestroyAtTheEndOfSimulation_new(SScriptCallBack* object)
{
    _callbackStructureToDestroyAtEndOfSimulation_new.push_back(object);
}
void CLuaScriptContainer::addCallbackStructureObjectToDestroyAtTheEndOfSimulation_old(SLuaCallBack* object)
{
    _callbackStructureToDestroyAtEndOfSimulation_old.push_back(object);
}

void CLuaScriptContainer::resetScriptFlagCalledInThisSimulationStep()
{
    for (size_t i=0;i<allScripts.size();i++)
        allScripts[i]->setCalledInThisSimulationStep(false);
}

int CLuaScriptContainer::getCalledScriptsCountInThisSimulationStep(int scriptType)
{
    int cnt=0;
    for (size_t i=0;i<allScripts.size();i++)
    {
        bool threaded=(scriptType==(sim_scripttype_childscript|sim_scripttype_threaded));
        if (threaded)
            scriptType=sim_scripttype_childscript;
        if ( (allScripts[i]->getScriptType()==scriptType)&&(allScripts[i]->getThreadedExecution()==threaded)&&allScripts[i]->getCalledInThisSimulationStep() )
            cnt++;
    }
    return(cnt);
}

int CLuaScriptContainer::removeDestroyedScripts(int scriptType)
{
    FUNCTION_DEBUG;
    int retVal=0;
    for (int i=0;i<int(allScripts.size());i++)
    {
        if ( (allScripts[i]->getScriptType()==scriptType)&&allScripts[i]->getFlaggedForDestruction() )
        {
            if ( (!allScripts[i]->getThreadedExecution())||(!allScripts[i]->getThreadedExecutionIsUnderWay()) )
            {
                retVal++;
                CLuaScriptObject* it=allScripts[i];
                it->killLuaState(); // should not be done in the destructor!
                allScripts.erase(allScripts.begin()+i);
                i--;
                delete it;
            }
        }
    }
    return(retVal);
}

void CLuaScriptContainer::setInMainScriptNow(bool launched,int startTimeInMs)
{
    if (launched)
    {
        if (_inMainScriptNow==0)
            _mainScriptStartTimeInMs=startTimeInMs;
        _inMainScriptNow++;
    }
    else
        _inMainScriptNow--;
}

bool CLuaScriptContainer::getInMainScriptNow() const
{
    return(_inMainScriptNow>0);
}

int CLuaScriptContainer::getMainScriptExecTimeInMs() const
{
    return(VDateTime::getTimeDiffInMs(_mainScriptStartTimeInMs));
}

bool CLuaScriptContainer::isContactCallbackFunctionAvailable()
{
    for (size_t i=0;i<allScripts.size();i++)
    {
        CLuaScriptObject* it=allScripts[i];
        if (it->getContainsContactCallbackFunction())
            return(true);
    }
    return(false);
}

bool CLuaScriptContainer::isDynCallbackFunctionAvailable()
{
    for (size_t i=0;i<allScripts.size();i++)
    {
        CLuaScriptObject* it=allScripts[i];
        if (it->getContainsDynCallbackFunction())
            return(true);
    }
    return(false);
}

void CLuaScriptContainer::removeAllScripts()
{
    FUNCTION_DEBUG;
    while (allScripts.size()>0)
    {
        CLuaScriptObject* it=allScripts[0];
        it->killLuaState(); // should not be done in the destructor!
        allScripts.erase(allScripts.begin());
        delete it;
    }
}

void CLuaScriptContainer::killAllSimulationLuaStates()
{
    for (size_t i=0;i<allScripts.size();i++)
    {
        if ( (allScripts[i]->getScriptType()==sim_scripttype_mainscript)||(allScripts[i]->getScriptType()==sim_scripttype_childscript) )
            allScripts[i]->killLuaState();
    }
}

void CLuaScriptContainer::announceObjectWillBeErased(int objectID)
{ // Never called from copy buffer!
    size_t i=0;
    while (i<allScripts.size())
    {
        if (allScripts[i]->announce3DObjectWillBeErased(objectID,false))
        {
            if (removeScript(allScripts[i]->getScriptID()))
                i=0; // ordering may have changed
            else
                i++;
        }
        else
            i++;
    }
}

bool CLuaScriptContainer::removeScript_safe(int scriptId)
{ // removal may happen in a delayed fashion
    CLuaScriptObject* it=getScriptFromID_noAddOnsNorSandbox(scriptId);
    if (it==nullptr)
        return(false);
    int res=it->flagScriptForRemoval();
    if (res==0)
        return(false);
    if (res==2)
        removeScript(scriptId);
    return(true);
}

bool CLuaScriptContainer::removeScript(int scriptID)
{
    FUNCTION_DEBUG;
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (allScripts[i]->getScriptID()==scriptID)
        {
            CLuaScriptObject* it=allScripts[i];
            it->killLuaState(); // should not be done in the destructor!
            allScripts.erase(allScripts.begin()+i);
            delete it;
            App::ct->setModificationFlag(16384);
            break;
        }
    }
    App::setFullDialogRefreshFlag();
    return(true);
}

CLuaScriptObject* CLuaScriptContainer::getScriptFromID_alsoAddOnsAndSandbox(int scriptID) const
{
    CLuaScriptObject* retVal=App::ct->addOnScriptContainer->getAddOnScriptFromID(scriptID);
    if (retVal==nullptr)
    {
        if (retVal==nullptr)
            retVal=getScriptFromID_noAddOnsNorSandbox(scriptID);
        if ( (retVal==nullptr)&&(App::ct->sandboxScript!=nullptr)&&(App::ct->sandboxScript->getScriptID()==scriptID) )
            retVal=App::ct->sandboxScript;

    }
    return(retVal);
}

CLuaScriptObject* CLuaScriptContainer::getScriptFromID_noAddOnsNorSandbox(int scriptID) const
{
    CLuaScriptObject* retVal=nullptr;
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (allScripts[i]->getScriptID()==scriptID)
        {
            retVal=allScripts[i];
            break;
        }
    }
    return(retVal);
}

CLuaScriptObject* CLuaScriptContainer::getScriptFromObjectAttachedTo_child(int threeDObjectID) const
{ // used for child scripts
    if (threeDObjectID<0)
        return(nullptr); // 10/1/2016
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (allScripts[i]->getObjectIDThatScriptIsAttachedTo_child()==threeDObjectID)
            return(allScripts[i]);
    }
    return(nullptr);
}

CLuaScriptObject* CLuaScriptContainer::getScriptFromObjectAttachedTo_customization(int threeDObjectID) const
{ // used for customization scripts
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (allScripts[i]->getObjectIDThatScriptIsAttachedTo_customization()==threeDObjectID)
            return(allScripts[i]);
    }
    return(nullptr);
}

int CLuaScriptContainer::getScriptsFromObjectAttachedTo(int threeDObjectID,std::vector<CLuaScriptObject*>& scripts) const
{
    scripts.clear();
    CLuaScriptObject* it=getScriptFromObjectAttachedTo_child(threeDObjectID);
    if (it!=nullptr)
        scripts.push_back(it);
    it=getScriptFromObjectAttachedTo_customization(threeDObjectID);
    if (it!=nullptr)
        scripts.push_back(it);
    return(int(scripts.size()));
}

CLuaScriptObject* CLuaScriptContainer::getMainScript() const
{
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (allScripts[i]->getScriptType()==sim_scripttype_mainscript)
            return(allScripts[i]);
    }
    return(nullptr);
}

int CLuaScriptContainer::insertScript(CLuaScriptObject* script)
{
    // We make sure the id is unique:
    int newID=SIM_IDSTART_LUASCRIPT;
    while (getScriptFromID_noAddOnsNorSandbox(newID)!=nullptr)
        newID++;
    script->setScriptID(newID);
    allScripts.push_back(script);
    App::ct->setModificationFlag(8192);
    return(newID);
}

int CLuaScriptContainer::insertDefaultScript_mainAndChildScriptsOnly(int scriptType,bool threaded)
{ 
    if (scriptType!=sim_scripttype_childscript)
        threaded=false; // just to make sure
    int retVal=-1;
    std::string filenameAndPath(App::directories->systemDirectory+VREP_SLASH);

    if (scriptType==sim_scripttype_mainscript)
        filenameAndPath+=DEFAULT_MAINSCRIPT_NAME;
    if (scriptType==sim_scripttype_childscript)
    {
        if (threaded)
            filenameAndPath+=DEFAULT_THREADEDCHILDSCRIPT_NAME;
        else
            filenameAndPath+=DEFAULT_NONTHREADEDCHILDSCRIPT_NAME;
    }

    if (VFile::doesFileExist(filenameAndPath))
    {
        try
        {
            VFile file(filenameAndPath,VFile::READ|VFile::SHARE_DENY_NONE);
            VArchive archive(&file,VArchive::LOAD);
            unsigned int archiveLength=(unsigned int)file.getLength();
            char* defaultScript=new char[archiveLength+1];
            for (int i=0;i<int(archiveLength);i++)
                archive >> defaultScript[i];
            defaultScript[archiveLength]=0;
            CLuaScriptObject* defScript=new CLuaScriptObject(scriptType);
            retVal=insertScript(defScript);
            defScript->setScriptText(defaultScript);
            defScript->setThreadedExecution(threaded);
            if (threaded)
                defScript->setExecuteJustOnce(true);
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
            defScript->setThreadedExecution(threaded);
        }
    }
    else
    {
        char defaultMessage[]="Default script file could not be found!"; // do not use comments ("--"), we want to cause an execution error!
        CLuaScriptObject* defScript=new CLuaScriptObject(scriptType);
        retVal=insertScript(defScript);
        defScript->setScriptText(defaultMessage);
        defScript->setThreadedExecution(threaded);
    }
    App::setLightDialogRefreshFlag();
    return(retVal);
}

void CLuaScriptContainer::callChildMainCustomizationAddonSandboxScriptWithData(int callType,CInterfaceStack* inStack)
{
    FUNCTION_DEBUG;
    if (!App::ct->simulation->isSimulationStopped())
    {
        CLuaScriptObject* script=getMainScript();
        if (script!=nullptr)
        {
            script->runMainScript(callType,inStack,nullptr,nullptr);
            handleCascadedScriptExecution(sim_scripttype_childscript,callType,inStack,nullptr,nullptr);
        }
    }
    handleCascadedScriptExecution(sim_scripttype_customizationscript,callType,inStack,nullptr,nullptr);
    App::ct->addOnScriptContainer->handleAddOnScriptExecution(callType,inStack,nullptr);
    if (App::ct->sandboxScript!=nullptr)
        App::ct->sandboxScript->runSandboxScript(callType,inStack,nullptr);
}

void CLuaScriptContainer::sceneOrModelAboutToBeSaved(int modelBase)
{
    C3DObject* obj=App::ct->objCont->getObjectFromHandle(modelBase);
    if (obj!=nullptr)
    {
        std::vector<C3DObject*> toExplore;
        toExplore.push_back(obj);
        while (toExplore.size()!=0)
        {
            obj=toExplore[toExplore.size()-1];
            toExplore.pop_back();
            CLuaScriptObject* it=getScriptFromObjectAttachedTo_customization(obj->getObjectHandle());
            if (it!=nullptr)
            {
                if (it->getCustomizationScriptCleanupBeforeSave())
                    it->killLuaState();
            }
            for (size_t i=0;i<obj->childList.size();i++)
                toExplore.push_back(obj->childList[i]);
        }
    }
    else
    {
        for (size_t i=0;i<allScripts.size();i++)
        {
            CLuaScriptObject* it=allScripts[i];
            if (it->getScriptType()==sim_scripttype_customizationscript)
            {
                if (it->getCustomizationScriptCleanupBeforeSave())
                    it->killLuaState();
            }
        }
    }
}

int CLuaScriptContainer::_getScriptsToExecute(int scriptType,std::vector<CLuaScriptObject*>& scripts,std::vector<int>& uniqueIds) const
{
    std::vector<C3DObject*> orderFirst;
    std::vector<C3DObject*> orderNormal;
    std::vector<C3DObject*> orderLast;
    std::vector<std::vector<C3DObject*>* > toHandle;
    toHandle.push_back(&orderFirst);
    toHandle.push_back(&orderNormal);
    toHandle.push_back(&orderLast);
    for (size_t i=0;i<App::ct->objCont->orphanList.size();i++)
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->orphanList[i]);
        toHandle[it->getScriptExecutionOrder(scriptType)]->push_back(it);
    }
    for (size_t i=0;i<toHandle.size();i++)
    {
        for (size_t j=0;j<toHandle[i]->size();j++)
            toHandle[i]->at(j)->getScriptsToExecute(scriptType,sim_scripttreetraversal_reverse,scripts,uniqueIds);
    }
    return(int(scripts.size()));
}

bool CLuaScriptContainer::doesScriptWithUniqueIdExist(int id) const
{
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (allScripts[i]->getScriptUniqueID()==id)
            return(true);
    }
    return(false);
}


int CLuaScriptContainer::handleCascadedScriptExecution(int scriptType,int callTypeOrResumeLocation,CInterfaceStack* inStack,CInterfaceStack* outStack,int* retInfo)
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
                        if (script->runCustomizationScript(callTypeOrResumeLocation,inStack,outStack)==1)
                        {
                            cnt++;
                            if (callTypeOrResumeLocation==sim_syscb_contactcallback)
                            {
                                if (retInfo!=nullptr)
                                    retInfo[0]=1;
                                break;
                            }
                            if (callTypeOrResumeLocation==sim_syscb_beforemainscript)
                            {
                                bool doNotRunMainScript;
                                if ((outStack!=nullptr)&&outStack->getStackMapBoolValue("doNotRunMainScript",doNotRunMainScript))
                                {
                                    if (doNotRunMainScript)
                                    {
                                        if (retInfo!=nullptr)
                                            retInfo[0]=1;
                                    }
                                }
                            }
                        }
                    }
                }
                else if ((scriptType&sim_scripttype_childscript)!=0)
                {
                    if (script->getThreadedExecution())
                    {
                        if (callTypeOrResumeLocation==sim_scriptthreadresume_launch)
                        {
                            if (script->launchThreadedChildScript())
                                cnt++;
                        }
                        else
                            cnt+=script->resumeThreadedChildScriptIfLocationMatch(callTypeOrResumeLocation);
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
                            if (script->runNonThreadedChildScript(callTypeOrResumeLocation,inStack,outStack)==1)
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

int CLuaScriptContainer::getScriptSimulationParameter_mainAndChildScriptsOnly(int scriptHandle,const char* parameterName,std::string& parameterValue) const
{
    int retVal=0;
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (!allScripts[i]->getFlaggedForDestruction())
        {
            if ( (scriptHandle==allScripts[i]->getScriptID())||
                (scriptHandle==sim_handle_all)||
                ( (scriptHandle==sim_handle_main_script)&&(allScripts[i]->getScriptType()==sim_scripttype_mainscript) ) )
            {
                if ((allScripts[i]->getScriptType()==sim_scripttype_mainscript)||(allScripts[i]->getScriptType()==sim_scripttype_childscript))
                { // We only access main script and child script1
                    bool res=allScripts[i]->getScriptParametersObject()->getParameterValue(parameterName,parameterValue);
                    if (res)
                        retVal++;
                }
            }
        }
    }
    return(retVal);
}

int CLuaScriptContainer::setScriptSimulationParameter_mainAndChildScriptsOnly(int scriptHandle,const char* parameterName,const char* parameterValue,int parameterValueLength)
{
    int retVal=0;
    for (int i=0;i<int(allScripts.size());i++)
    {
        if (!allScripts[i]->getFlaggedForDestruction())
        {
            if ( (scriptHandle==allScripts[i]->getScriptID())||
                (scriptHandle==sim_handle_all)||
                ( (scriptHandle==sim_handle_main_script)&&(allScripts[i]->getScriptType()==sim_scripttype_mainscript) ) )
            {
                if ((allScripts[i]->getScriptType()==sim_scripttype_mainscript)||(allScripts[i]->getScriptType()==sim_scripttype_childscript))
                { // We only access main script and child script1
                    bool res=allScripts[i]->getScriptParametersObject()->setParameterValue(parameterName,parameterValue,parameterValueLength);
                    if (res)
                        retVal++;
                }
            }
        }
    }
    return(retVal);
}

bool CLuaScriptContainer::addCommandToOutsideCommandQueues(int commandID,int auxVal1,int auxVal2,int auxVal3,int auxVal4,const float aux2Vals[8],int aux2Count)
{
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (!allScripts[i]->getFlaggedForDestruction())
            allScripts[i]->addCommandToOutsideCommandQueue(commandID,auxVal1,auxVal2,auxVal3,auxVal4,aux2Vals,aux2Count);
    }
    return(true);
}
