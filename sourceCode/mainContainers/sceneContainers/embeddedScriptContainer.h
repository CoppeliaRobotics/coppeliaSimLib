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
    CScriptObject* getScriptFromObjectAttachedTo_child(int objectHandle) const;
    CScriptObject* getScriptFromObjectAttachedTo_customization(int objectHandle) const;

    int getScriptsFromObjectAttachedTo(int objectHandle,std::vector<CScriptObject*>& scripts) const;
    bool doesScriptWithUniqueIdExist(int id) const;

    void killAllSimulationLuaStates();
    int insertDefaultScript(int scriptType,bool threaded,bool lua,bool oldThreadedScript=false);
    const std::vector<int>* getObjectIdsWhereDynCallbackFunctionsAvailable() const;

    int removeDestroyedScripts(int scriptType);
    void resetScriptFlagCalledInThisSimulationStep();
    int getCalledScriptsCountInThisSimulationStep(bool onlySimulationScripts);

    void addCallbackStructureObjectToDestroyAtTheEndOfSimulation_new(SScriptCallBack* object);
    void addCallbackStructureObjectToDestroyAtTheEndOfSimulation_old(SLuaCallBack* object);
    bool addCommandToOutsideCommandQueues(int commandID,int auxVal1,int auxVal2,int auxVal3,int auxVal4,const float aux2Vals[8],int aux2Count);

    int handleCascadedScriptExecution(int scriptType,int callTypeOrResumeLocation,CInterfaceStack* inStack,CInterfaceStack* outStack,int* retInfo);
    bool shouldTemporarilySuspendMainScript();
    bool isContactCallbackFunctionAvailable();
    bool isDynCallbackFunctionAvailable();

    void callScripts(int callType,CInterfaceStack* inStack);
    void sceneOrModelAboutToBeSaved_old(int modelBase);

    std::vector<CScriptObject*> allScripts;

    CBroadcastDataContainer broadcastDataContainer;

protected:
    int _getScriptsToExecute(int scriptType,std::vector<CScriptObject*>& scripts,std::vector<int>& uniqueIds) const;

    std::vector<SScriptCallBack*> _callbackStructureToDestroyAtEndOfSimulation_new;
    std::vector<SLuaCallBack*> _callbackStructureToDestroyAtEndOfSimulation_old;
};
