#pragma once

#include <script.h>
#include <broadcastDataContainer.h>
#include <simInternal.h>

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
    void announceSceneObjectWillBeErased(const CSceneObject* object);
    bool removeScript_safe(int scriptHandle);
    bool removeScript(int scriptHandle);
    void extractScript(int scriptHandle);
    int insertScript(CScript* script);
    int getObjectHandleFromScriptHandle(int scriptHandle) const;
    CScript* getScriptFromHandle(int scriptHandle) const;
    CScript* getScriptFromUid(int uid) const;
    CScript* getMainScript() const;
    CScript* getScriptFromObjectAttachedTo(int scriptType, int objectHandle) const;

    int getScriptsFromObjectAttachedTo(int objectHandle, std::vector<CScript*>& scripts) const;

    void pushMainScriptGenesisEvent() const;
    void killAllSimulationLuaStates();
    int insertDefaultScript(int scriptType, bool threaded, bool lua);
    const std::vector<int>* getObjectIdsWhereDynCallbackFunctionsAvailable() const;

    int removeDestroyedScripts(int scriptType);
    void resetScriptFlagCalledInThisSimulationStep();
    int getCalledScriptsCountInThisSimulationStep(bool onlySimulationScripts);
    void setScriptsTemporarilySuspended(bool suspended);

    bool addCommandToOutsideCommandQueues(int commandID, int auxVal1, int auxVal2, int auxVal3, int auxVal4,
                                          const double aux2Vals[8], int aux2Count);

    void getActiveLegacyScripts(std::vector<CScript*>& scripts, bool reverse = false) const;
    int callLegacyScripts(int scriptType, int callTypeOrResumeLocation, const CInterfaceStack* inStack, CInterfaceStack* outStack, CSceneObject* objectBranch = nullptr, int nakedScriptToExclude = -1);
    bool shouldTemporarilySuspendMainScript();
    int getSysFuncAndHookCnt(int sysCall) const;
    void setSysFuncAndHookCnt(int sysCall, int cnt);

    void sceneOrModelAboutToBeSaved_old(int modelBase);
    int getEquivalentScriptExecPriority_old(int objectHandle) const;

    std::vector<CScript*> allScripts;

    CBroadcastDataContainer broadcastDataContainer;

  protected:
    int _sysFuncAndHookCnt_event;
    int _sysFuncAndHookCnt_dyn;
    int _sysFuncAndHookCnt_contact;
    int _sysFuncAndHookCnt_joint;
};
