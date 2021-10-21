#pragma once

#include "scriptCustomFuncAndVarContainer.h"
#include "persistentDataContainer.h"
#include "copyBuffer.h"
#include "addOnScriptContainer.h"
#include "interfaceStackContainer.h"
#include "simulatorMessageQueue.h"
#include "calculationInfo.h"
#include "moduleMenuItemContainer.h"
#include "world.h"
#include "_worldContainer_.h"

#ifdef SIM_WITH_GUI
    #include "globalGuiTextureContainer.h"
#endif

#ifdef SIM_WITH_SERIAL
    #include "serialPortContainer.h"
#endif

class CWorldContainer : public _CWorldContainer_
{
public:
    CWorldContainer();
    virtual ~CWorldContainer();

    void initialize();
    void deinitialize();

    void setModificationFlag(int bitMask);
    int getModificationFlags(bool clearTheFlagsAfter);

    int createNewWorld();
    int destroyCurrentWorld();
    int getWorldCount() const;
    int getCurrentWorldIndex() const;
    bool switchToWorld(int worldIndex);
    bool isWorldSwitchingLocked() const;
    void getAllSceneNames(std::vector<std::string>& l) const;
    CScriptObject* getScriptFromHandle(int scriptHandle) const;
    void callScripts(int callType,CInterfaceStack* inStack);
    CInterfaceStackTable* createFreshEvent(const char* event,const char* change,int uid,bool mergeable=true);
    void pushEvent();
    void sendEvents();
    void setCborEvents();

    void simulationAboutToStart();
    void simulationPaused();
    void simulationAboutToResume();
    void simulationAboutToStep();
    void simulationAboutToEnd();
    void simulationEnded(bool removeNewObjects);

    void announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript);
    void announceScriptStateWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript);

    CCopyBuffer* copyBuffer;
    CPersistentDataContainer* persistentDataContainer;
    CSimulatorMessageQueue* simulatorMessageQueue;
    CCalculationInfo* calcInfo;
    CInterfaceStackContainer* interfaceStackContainer;
    CScriptCustomFuncAndVarContainer* scriptCustomFuncAndVarContainer;
    CCustomData* customAppData;
    CAddOnScriptContainer* addOnScriptContainer;
    CScriptObject* sandboxScript;
    CModuleMenuItemContainer* moduleMenuItemContainer;
#ifdef SIM_WITH_GUI
    CGlobalGuiTextureContainer* globalGuiTextureCont;
#endif
#ifdef SIM_WITH_SERIAL
    CSerialPortContainer* serialPortContainer;
#endif

private:
    bool _switchToWorld(int newWorldIndex);

    std::vector<CWorld*> _worlds;
    int _currentWorldIndex;
    CInterfaceStackTable* _event;
    CInterfaceStack* _bufferedEvents;
    std::vector<std::string> _bufferedEventsSummary;
    std::string _lastEventN;
    std::string _lastEventNN;
    bool _cborEvents;

    std::vector<int> _uniqueIdsOfSelectionSinceLastTimeGetAndClearModificationFlagsWasCalled;
    int _modificationFlags;
    // +bit 0: object(s) erased
    // +bit 1: object(s) created
    // +bit 2: model loaded
    // +bit 3: scene loaded
    // +bit 4: undo called
    // +bit 5: redo called
    // +bit 6: instance switched
    // +bit 7: edit mode active         --> is set when getAndClearModificationFlags is called
    // +bit 8: object(s) scaled
    // +bit 9: selection state changed  --> is set when getAndClearModificationFlags is called
    // +bit 10: key pressed
    // +bit 11: simulation started
    // +bit 12: simulation ended

#ifdef SIM_WITH_GUI
public:
    void keyPress(int key);
    void addMenu(VMenu* menu);
    int getInstanceIndexOfASceneNotYetSaved(bool doNotIncludeCurrentScene);
    void setInstanceIndexWithThumbnails(int index);
    bool processGuiCommand(int commandID);
#endif
};
