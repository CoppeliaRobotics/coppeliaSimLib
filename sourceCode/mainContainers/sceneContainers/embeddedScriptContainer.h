#pragma once

#include "luaScriptObject.h"
#include "broadcastDataContainer.h"
#include "simInternal.h"

class CEmbeddedScriptContainer
{
public:
    CEmbeddedScriptContainer();
    virtual ~CEmbeddedScriptContainer();
    void simulationAboutToStart();
    void simulationEnded();
    void simulationAboutToEnd();
    void removeAllScripts();
    void announceObjectWillBeErased(int scriptHandle);
    bool removeScript_safe(int scriptHandle);
    bool removeScript(int scriptHandle);
    int insertScript(CLuaScriptObject* script);
    CLuaScriptObject* getScriptFromHandle(int scriptHandle) const;
    CLuaScriptObject* getMainScript() const;
    CLuaScriptObject* getScriptFromObjectAttachedTo_child(int objectHandle) const;
    CLuaScriptObject* getScriptFromObjectAttachedTo_customization(int objectHandle) const;

    int getScriptsFromObjectAttachedTo(int objectHandle,std::vector<CLuaScriptObject*>& scripts) const;
    bool doesScriptWithUniqueIdExist(int id) const;

    void killAllSimulationLuaStates();
    int insertDefaultScript_mainAndChildScriptsOnly(int scriptType,bool threaded,bool oldThreadedScript);
    void setInMainScriptNow(bool launched,int startTimeInMs);
    bool getInMainScriptNow() const;
    int getMainScriptExecTimeInMs() const;
    const std::vector<int>* getObjectIdsWhereDynCallbackFunctionsAvailable() const;

    int removeDestroyedScripts(int scriptType);
    void resetScriptFlagCalledInThisSimulationStep();
    int getCalledScriptsCountInThisSimulationStep(bool onlySimulationScripts);

    void addCallbackStructureObjectToDestroyAtTheEndOfSimulation_new(SScriptCallBack* object);
    void addCallbackStructureObjectToDestroyAtTheEndOfSimulation_old(SLuaCallBack* object);
    bool addCommandToOutsideCommandQueues(int commandID,int auxVal1,int auxVal2,int auxVal3,int auxVal4,const float aux2Vals[8],int aux2Count);

    int handleCascadedScriptExecution(int scriptType,int callTypeOrResumeLocation,CInterfaceStack* inStack,CInterfaceStack* outStack,int* retInfo);
    bool isContactCallbackFunctionAvailable();
    bool isDynCallbackFunctionAvailable();

    void callScripts(int callType,CInterfaceStack* inStack);
    void sceneOrModelAboutToBeSaved(int modelBase);

    std::vector<CLuaScriptObject*> allScripts;

    CBroadcastDataContainer broadcastDataContainer;

protected:
    int _getScriptsToExecute(int scriptType,std::vector<CLuaScriptObject*>& scripts,std::vector<int>& uniqueIds) const;

    int _inMainScriptNow;
    int _mainScriptStartTimeInMs;

    std::vector<SScriptCallBack*> _callbackStructureToDestroyAtEndOfSimulation_new;
    std::vector<SLuaCallBack*> _callbackStructureToDestroyAtEndOfSimulation_old;
};
