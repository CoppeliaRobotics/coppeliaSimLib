#include "simInternal.h"
#include "embeddedScriptContainer.h"
#include "tt.h"
#include "vVarious.h"
#include "threadPool_old.h"
#include "vFileFinder.h"
#include "simStrings.h"
#include "app.h"
#include "vDateTime.h"

CEmbeddedScriptContainer::CEmbeddedScriptContainer()
{
    _nextScriptHandle=SIM_IDSTART_EMBEDDEDSCRIPT;
    _contactFuncCount=0;
    _dynFuncCount=0;
    _eventFuncCount=0;
    _jointFuncCount=0;
    insertDefaultScript(sim_scripttype_mainscript,false,true);
}

CEmbeddedScriptContainer::~CEmbeddedScriptContainer()
{ // beware, the current world could be nullptr
    removeAllScripts();
    for (size_t i=0;i<_callbackStructureToDestroyAtEndOfSimulation_new.size();i++)
        delete _callbackStructureToDestroyAtEndOfSimulation_new[i];
    for (size_t i=0;i<_callbackStructureToDestroyAtEndOfSimulation_old.size();i++)
        delete _callbackStructureToDestroyAtEndOfSimulation_old[i];
    broadcastDataContainer.eraseAllObjects();
}

int CEmbeddedScriptContainer::getContactFuncCount() const
{
    return(_contactFuncCount);
}

void CEmbeddedScriptContainer::setContactFuncCount(int cnt)
{
    _contactFuncCount=cnt;
}

int CEmbeddedScriptContainer::getDynFuncCount() const
{
    return(_dynFuncCount);
}

void CEmbeddedScriptContainer::setDynFuncCount(int cnt)
{
    _dynFuncCount=cnt;
}

int CEmbeddedScriptContainer::getEventFuncCount() const
{
    return(_eventFuncCount);
}

void CEmbeddedScriptContainer::setEventFuncCount(int cnt)
{
    _eventFuncCount=cnt;
}

int CEmbeddedScriptContainer::getJointFuncCount() const
{
    return(_jointFuncCount);
}

void CEmbeddedScriptContainer::setJointFuncCount(int cnt)
{
    _jointFuncCount=cnt;
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
                CScriptObject* it=allScripts[i];
                it->resetScript(); // should not be done in the destructor!
                allScripts.erase(allScripts.begin()+i);
                i--;
                CScriptObject::destroy(it,true);
            }
        }
    }
    return(retVal);
}

void CEmbeddedScriptContainer::removeAllScripts()
{
    TRACE_INTERNAL;
    while (allScripts.size()>0)
    {
        CScriptObject* it=allScripts[0];
        it->resetScript(); // should not be done in the destructor!
        allScripts.erase(allScripts.begin());
        CScriptObject::destroy(it,true);
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

void CEmbeddedScriptContainer::announceObjectWillBeErased(const CSceneObject* object)
{ // Never called from copy buffer!
    size_t i=0;
    while (i<allScripts.size())
    {
        if (allScripts[i]->announceSceneObjectWillBeErased(object,false))
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
    CScriptObject* it=getScriptFromHandle(scriptHandle);
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
            CScriptObject* it=allScripts[i];
            it->resetScript(); // should not be done in the destructor!
            allScripts.erase(allScripts.begin()+i);
            CScriptObject::destroy(it,true);
            App::worldContainer->setModificationFlag(16384);
            break;
        }
    }
    App::setFullDialogRefreshFlag();
    return(true);
}

CScriptObject* CEmbeddedScriptContainer::getScriptFromHandle(int scriptHandle) const
{
    CScriptObject* retVal=nullptr;
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

int CEmbeddedScriptContainer::getObjectHandleFromScriptHandle(int scriptHandle) const
{
    CScriptObject* script=getScriptFromHandle(scriptHandle);
    if (script!=nullptr)
        return(script->getObjectHandleThatScriptIsAttachedTo(-1));
    return(-1);
}

CScriptObject* CEmbeddedScriptContainer::getScriptFromObjectAttachedTo(int scriptType,int objectHandle) const
{
    CScriptObject* retVal=nullptr;
    if (objectHandle>=0)
    {
        for (size_t i=0;i<allScripts.size();i++)
        {
            if (allScripts[i]->getObjectHandleThatScriptIsAttachedTo(scriptType)==objectHandle)
            {
                retVal=allScripts[i];
                break;
            }
        }
    }
    return(retVal);
}

int CEmbeddedScriptContainer::getScriptsFromObjectAttachedTo(int objectHandle,std::vector<CScriptObject*>& scripts) const
{
    scripts.clear();
    CScriptObject* it=getScriptFromObjectAttachedTo(sim_scripttype_childscript,objectHandle);
    if (it!=nullptr)
        scripts.push_back(it);
    it=getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objectHandle);
    if (it!=nullptr)
        scripts.push_back(it);
    return(int(scripts.size()));
}

CScriptObject* CEmbeddedScriptContainer::getMainScript() const
{
    for (size_t i=0;i<allScripts.size();i++)
    {
        if (allScripts[i]->getScriptType()==sim_scripttype_mainscript)
            return(allScripts[i]);
    }
    return(nullptr);
}

int CEmbeddedScriptContainer::insertScript(CScriptObject* script)
{
    script->setScriptHandle(_nextScriptHandle++);
    allScripts.push_back(script);
    App::worldContainer->setModificationFlag(8192);
    return(_nextScriptHandle-1);
}

int CEmbeddedScriptContainer::insertDefaultScript(int scriptType,bool threaded,bool lua,bool oldThreadedScript/*=false*/)
{ 
    if (scriptType!=sim_scripttype_childscript)
        oldThreadedScript=false; // just to make sure
    int retVal=-1;
    std::string filenameAndPath(App::folders->getSystemPath()+"/");

    if (scriptType==sim_scripttype_mainscript)
    {
        CScriptObject* defScript=new CScriptObject(scriptType);
        retVal=insertScript(defScript);
        defScript->setScriptText(DEFAULT_MAINSCRIPT_CODE);
        filenameAndPath="";
    }
    if (scriptType==sim_scripttype_childscript)
    {
        if (oldThreadedScript)
            filenameAndPath+=DEFAULT_THREADEDCHILDSCRIPTOLD;
        else
        {
            if (threaded)
                filenameAndPath+=DEFAULT_THREADEDCHILDSCRIPT;
            else
                filenameAndPath+=DEFAULT_NONTHREADEDCHILDSCRIPT;
        }
    }
    if (scriptType==sim_scripttype_customizationscript)
    {
        if (threaded)
            filenameAndPath+=DEFAULT_THREADEDCUSTOMIZATIONSCRIPT;
        else
            filenameAndPath+=DEFAULT_NONTHREADEDCUSTOMIZATIONSCRIPT;
    }

    if (filenameAndPath.size()>0)
    {
        if (lua)
            filenameAndPath+=".lua";
        else
            filenameAndPath+=".py";
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
                CScriptObject* defScript=new CScriptObject(scriptType);
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
                CScriptObject* defScript=new CScriptObject(scriptType);
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
            CScriptObject* defScript=new CScriptObject(scriptType);
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

void CEmbeddedScriptContainer::callScripts(int callType,CInterfaceStack* inStack,CInterfaceStack* outStack,CSceneObject* objectBranch/*=nullptr*/)
{ // with objectBranch!=nullptr, will return the branch starting at objectBranch up to the main script
    TRACE_INTERNAL;
    if (!App::userSettings->enableOldScriptTraversal)
    {
        bool doNotInterrupt=!CScriptObject::isSystemCallbackInterruptible(callType);
        if (CScriptObject::isSystemCallbackInReverseOrder(callType))
        { // reverse order
            if (!App::currentWorld->simulation->isSimulationStopped())
            {
                CScriptObject* script=getMainScript();
                if ( (script!=nullptr)&&(script->hasFunction(callType)) )
                    script->systemCallMainScript(callType,inStack,outStack);
            }
            if ( doNotInterrupt||(outStack==nullptr)||(outStack->getStackSize()==0) )
                callChildAndEmbeddedScripts(-1,callType,inStack,outStack,objectBranch);
        }
        else
        { // regular. From unimportant, to important
            callChildAndEmbeddedScripts(-1,callType,inStack,outStack,objectBranch);
            if ( doNotInterrupt||(outStack==nullptr)||(outStack->getStackSize()==0) )
            {
                if (!App::currentWorld->simulation->isSimulationStopped())
                {
                    CScriptObject* script=getMainScript();
                    if ( (script!=nullptr)&&(script->hasFunction(callType)) )
                        script->systemCallMainScript(callType,inStack,outStack);
                }
            }
        }
    }
    else
    { // for backward compatibility:
        if (!App::currentWorld->simulation->isSimulationStopped())
        {
            CScriptObject* script=getMainScript();
            if (script!=nullptr)
            {
                script->systemCallMainScript(callType,inStack,outStack);
                callChildAndEmbeddedScripts(sim_scripttype_childscript,callType,inStack,outStack,nullptr);
            }
        }
        callChildAndEmbeddedScripts(sim_scripttype_customizationscript,callType,inStack,outStack,nullptr);
    }
}

int CEmbeddedScriptContainer::getEquivalentScriptExecPriority_old(int objectHandle) const
{ // for backward compatibility
    int retVal=-1; // no script attached
    CScriptObject* it=getScriptFromObjectAttachedTo(sim_scripttype_childscript,objectHandle);
    if (it!=nullptr)
        retVal=it->getExecutionPriority_old();
    it=getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objectHandle);
    if (it!=nullptr)
        retVal=it->getExecutionPriority_old();
    return(retVal);
}

void CEmbeddedScriptContainer::sceneOrModelAboutToBeSaved_old(int modelBase)
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
            CScriptObject* it=getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,obj->getObjectHandle());
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
            CScriptObject* it=allScripts[i];
            if (it->getScriptType()==sim_scripttype_customizationscript)
            {
                if (it->getCustomizationScriptCleanupBeforeSave_DEPRECATED())
                    it->resetScript();
            }
        }
    }
}

size_t CEmbeddedScriptContainer::_getScriptsToExecute(std::vector<int>& scriptHandles,int scriptType) const
{ // returns all non-disabled scripts, from leaf to root. With scriptType==-1, returns child and customization scripts
    std::vector<CSceneObject*> objects;
    std::vector<CSceneObject*> objectsNormalPriority;
    std::vector<CSceneObject*> objectsLastPriority;
    for (size_t i=0;i<App::currentWorld->sceneObjects->getOrphanCount();i++)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getOrphanFromIndex(i);
        if (it->getScriptExecPriority()==sim_scriptexecorder_first)
            objects.push_back(it);
        if (it->getScriptExecPriority()==sim_scriptexecorder_normal)
            objectsNormalPriority.push_back(it);
        if (it->getScriptExecPriority()==sim_scriptexecorder_last)
            objectsLastPriority.push_back(it);
    }
    objects.insert(objects.end(),objectsNormalPriority.begin(),objectsNormalPriority.end());
    objects.insert(objects.end(),objectsLastPriority.begin(),objectsLastPriority.end());
    for (size_t i=0;i<objects.size();i++)
        objects[i]->getScriptsToExecute(scriptHandles,scriptType);
    return(scriptHandles.size());
}

int CEmbeddedScriptContainer::_getScriptsToExecute_old(int scriptType,std::vector<CScriptObject*>& scripts,std::vector<int>& uniqueIds) const
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
            toHandle[it->getScriptExecutionOrder_old(scriptType)]->push_back(it);
        }
        for (size_t i=0;i<toHandle.size();i++)
        {
            for (size_t j=0;j<toHandle[i]->size();j++)
                toHandle[i]->at(j)->getScriptsToExecute_old(scriptType,sim_scripttreetraversal_reverse,scripts,uniqueIds);
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
    std::vector<int> scriptHandles;
    _getScriptsToExecute(scriptHandles,-1);
    for (size_t i=0;i<scriptHandles.size();i++)
    {
        CScriptObject* it=getScriptFromHandle(scriptHandles[i]);
        if (it!=nullptr)
        { // could have been erased in the mean time!
            if (it->shouldTemporarilySuspendMainScript())
                retVal=true;
        }
    }
    return(retVal);
}

int CEmbeddedScriptContainer::callChildAndEmbeddedScripts(int scriptType,int callTypeOrResumeLocation,CInterfaceStack* inStack,CInterfaceStack* outStack,CSceneObject* objectBranch/*=nullptr*/)
{ // ignores the main script. See mainly callScripts instead
    int cnt=0;
    if (!App::userSettings->enableOldScriptTraversal)
    {
        std::vector<int> scriptHandles;
        if (objectBranch==nullptr)
            _getScriptsToExecute(scriptHandles,scriptType);
        else
            objectBranch->getScriptsToExecute_branch(scriptHandles,scriptType);
        if (CScriptObject::isSystemCallbackInReverseOrder(callTypeOrResumeLocation))
            std::reverse(scriptHandles.begin(),scriptHandles.end());
        bool canInterrupt=CScriptObject::isSystemCallbackInterruptible(callTypeOrResumeLocation);
        for (size_t i=0;i<scriptHandles.size();i++)
        {
            CScriptObject* script=getScriptFromHandle(scriptHandles[i]);
            if (script!=nullptr)
            { // the script could have been erased in the mean time
                if (script->getThreadedExecution_oldThreads())
                { // is an old, threaded script
                    if (callTypeOrResumeLocation==sim_scriptthreadresume_launch)
                    {
                        if (script->launchThreadedChildScript_oldThreads())
                            cnt++;
                    }
                    else
                        cnt+=script->resumeThreadedChildScriptIfLocationMatch_oldThreads(callTypeOrResumeLocation);
                }
                else if (script->hasFunction(callTypeOrResumeLocation))
                { // has the function
                    if (script->systemCallScript(callTypeOrResumeLocation,inStack,outStack)==1)
                    {
                        cnt++;
                        if ( canInterrupt&&(outStack!=nullptr)&&(outStack->getStackSize()!=0) )
                            break;
                    }
                }
                else
                { // has not the function. Check if we need to support old callbacks:
                    int compatCall=-1;
                    if (callTypeOrResumeLocation==sim_syscb_dyn)
                        compatCall=sim_syscb_dyncallback;
                    if (callTypeOrResumeLocation==sim_syscb_contact)
                        compatCall=sim_syscb_contactcallback;
                    if ( (compatCall!=-1)&&(script->hasFunction(compatCall)) )
                    {
                        if (script->systemCallScript(compatCall,inStack,outStack)==1)
                        {
                            cnt++;
                            if ( canInterrupt&&(outStack!=nullptr)&&(outStack->getStackSize()!=0) )
                                break;
                        }
                    }

                }
            }
        }
    }
    else
    { // old routine
        std::vector<CScriptObject*> scripts;
        std::vector<int> uniqueIds;
        _getScriptsToExecute_old(scriptType,scripts,uniqueIds);
        for (size_t i=0;i<scripts.size();i++)
        {
            if (doesScriptWithUniqueIdExist(uniqueIds[i]))
            { // the script could have been erased in the mean time
                CScriptObject* script=scripts[i];
                if (!script->getScriptIsDisabled())
                {
                    if (scriptType==sim_scripttype_customizationscript)
                    {
                        bool doIt=true;
                        if ( (callTypeOrResumeLocation==sim_syscb_dyncallback)&&(!script->hasFunction(sim_syscb_dyncallback)) )
                            doIt=false;
                        if ( (callTypeOrResumeLocation==sim_syscb_contactcallback)&&(!script->hasFunction(sim_syscb_contactcallback)) )
                            doIt=false;
                        if (doIt)
                        {
                            if (script->systemCallScript(callTypeOrResumeLocation,inStack,outStack)==1)
                            {
                                cnt++;
                                if (callTypeOrResumeLocation==sim_syscb_contactcallback)
                                    break;
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
                            if ( (callTypeOrResumeLocation==sim_syscb_dyncallback)&&(!script->hasFunction(sim_syscb_dyncallback)) )
                                doIt=false;
                            if ( (callTypeOrResumeLocation==sim_syscb_contactcallback)&&(!script->hasFunction(sim_syscb_contactcallback)) )
                                doIt=false;
                            if (doIt)
                            {
                                if (script->systemCallScript(callTypeOrResumeLocation,inStack,outStack)==1)
                                {
                                    cnt++;
                                    if (callTypeOrResumeLocation==sim_syscb_contactcallback)
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
