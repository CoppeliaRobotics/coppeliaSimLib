
#pragma once

#include "mainCont.h"
#include "luaScriptObject.h"
#include "broadcastDataContainer.h"
#include "v_rep_internal.h"

class CLuaScriptContainer : public CMainCont 
{
public:
    CLuaScriptContainer();
    virtual ~CLuaScriptContainer();
    void simulationAboutToStart();
    void simulationEnded();
    void simulationAboutToEnd();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);
    void removeAllScripts();
    void announceObjectWillBeErased(int objectID);
    bool removeScript_safe(int scriptId);
    bool removeScript(int scriptID);
    int insertScript(CLuaScriptObject* script);
    CLuaScriptObject* getScriptFromID_alsoAddOnsAndSandbox(int scriptID) const;
    CLuaScriptObject* getScriptFromID_noAddOnsNorSandbox(int scriptID) const;
    CLuaScriptObject* getMainScript() const;
    CLuaScriptObject* getScriptFromObjectAttachedTo_child(int threeDObjectID) const;
    CLuaScriptObject* getScriptFromObjectAttachedTo_customization(int threeDObjectID) const;

    int getScriptsFromObjectAttachedTo(int threeDObjectID,std::vector<CLuaScriptObject*>& scripts) const;
    bool doesScriptWithUniqueIdExist(int id) const;

    void killAllSimulationLuaStates();
    int insertDefaultScript_mainAndChildScriptsOnly(int scriptType,bool threaded);
    void setInMainScriptNow(bool launched,int startTimeInMs);
    bool getInMainScriptNow() const;
    int getMainScriptExecTimeInMs() const;
    const std::vector<int>* getObjectIdsWhereDynCallbackFunctionsAvailable() const;

    int removeDestroyedScripts(int scriptType);
    void resetScriptFlagCalledInThisSimulationStep();
    int getCalledScriptsCountInThisSimulationStep(int scriptType);

    void addCallbackStructureObjectToDestroyAtTheEndOfSimulation_new(SScriptCallBack* object);
    void addCallbackStructureObjectToDestroyAtTheEndOfSimulation_old(SLuaCallBack* object);
    bool addCommandToOutsideCommandQueues(int commandID,int auxVal1,int auxVal2,int auxVal3,int auxVal4,const float aux2Vals[8],int aux2Count);

    int handleCascadedScriptExecution(int scriptType,int callTypeOrResumeLocation,CInterfaceStack* inStack,CInterfaceStack* outStack,int* retInfo);
    bool isContactCallbackFunctionAvailable();
    bool isDynCallbackFunctionAvailable();

    void callChildMainCustomizationAddonSandboxScriptWithData(int callType,CInterfaceStack* inStack);
    void sceneOrModelAboutToBeSaved(int modelBase);

    std::vector<CLuaScriptObject*> allScripts;

    int getScriptSimulationParameter_mainAndChildScriptsOnly(int scriptHandle,const char* parameterName,std::string& parameterValue) const;
    int setScriptSimulationParameter_mainAndChildScriptsOnly(int scriptHandle,const char* parameterName,const char* parameterValue,int parameterValueLength);

    CBroadcastDataContainer broadcastDataContainer;


protected:
    int _getScriptsToExecute(int scriptType,std::vector<CLuaScriptObject*>& scripts,std::vector<int>& uniqueIds) const;

    int _inMainScriptNow;
    int _mainScriptStartTimeInMs;

    std::vector<SScriptCallBack*> _callbackStructureToDestroyAtEndOfSimulation_new;
    std::vector<SLuaCallBack*> _callbackStructureToDestroyAtEndOfSimulation_old;
};
