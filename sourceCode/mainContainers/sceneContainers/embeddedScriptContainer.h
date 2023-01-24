#pragma once

#include "scriptObject.h"
#include "broadcastDataContainer.h"
#include "simInternal.h"

class CSceneObject;

class CEmbeddedScriptContainer
{
public:
    CEmbeddedScriptContainer();
    virtual ~CEmbeddedScriptContainer();
    void simulationAboutToStart();
    void simulationEnded();
    void simulationAboutToEnd();
    void removeAllScripts();
    void announceObjectWillBeErased(const CSceneObject* object);
    bool removeScript_safe(int scriptHandle);
    bool removeScript(int scriptHandle);
    int insertScript(CScriptObject* script);
    int getObjectHandleFromScriptHandle(int scriptHandle) const;
    CScriptObject* getScriptFromHandle(int scriptHandle) const;
    CScriptObject* getMainScript() const;
    CScriptObject* getScriptFromObjectAttachedTo(int scriptType,int objectHandle) const;

    int getScriptsFromObjectAttachedTo(int objectHandle,std::vector<CScriptObject*>& scripts) const;

    void killAllSimulationLuaStates();
    int insertDefaultScript(int scriptType,bool threaded,bool lua,bool oldThreadedScript=false);
    const std::vector<int>* getObjectIdsWhereDynCallbackFunctionsAvailable() const;

    int removeDestroyedScripts(int scriptType);
    void resetScriptFlagCalledInThisSimulationStep();
    int getCalledScriptsCountInThisSimulationStep(bool onlySimulationScripts);

    void addCallbackStructureObjectToDestroyAtTheEndOfSimulation_new(SScriptCallBack* object);
    void addCallbackStructureObjectToDestroyAtTheEndOfSimulation_old(SLuaCallBack* object);
    bool addCommandToOutsideCommandQueues(int commandID,int auxVal1,int auxVal2,int auxVal3,int auxVal4,const double aux2Vals[8],int aux2Count);

    int callChildAndEmbeddedScripts(int scriptType,int callTypeOrResumeLocation,CInterfaceStack* inStack,CInterfaceStack* outStack,CSceneObject* objectBranch=nullptr);
    bool shouldTemporarilySuspendMainScript();
    int getSysFuncAndHookCnt(int sysCall) const;
    void setSysFuncAndHookCnt(int sysCall,int cnt);

    void callScripts(int callType,CInterfaceStack* inStack,CInterfaceStack* outStack,CSceneObject* objectBranch=nullptr);
    void sceneOrModelAboutToBeSaved_old(int modelBase);
    int getEquivalentScriptExecPriority_old(int objectHandle) const;

    std::vector<CScriptObject*> allScripts;

    CBroadcastDataContainer broadcastDataContainer;

protected:
    size_t _getScriptsToExecute(std::vector<int>& scriptHandles,int scriptType) const;
    int _getScriptsToExecute_old(int scriptType,std::vector<CScriptObject*>& scripts,std::vector<int>& uniqueIds) const;

    int _sysFuncAndHookCnt_event;
    int _sysFuncAndHookCnt_dyn;
    int _sysFuncAndHookCnt_contact;
    int _sysFuncAndHookCnt_joint;
    std::vector<SScriptCallBack*> _callbackStructureToDestroyAtEndOfSimulation_new;
    std::vector<SLuaCallBack*> _callbackStructureToDestroyAtEndOfSimulation_old;
};
