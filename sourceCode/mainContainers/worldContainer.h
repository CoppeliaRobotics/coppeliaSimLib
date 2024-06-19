#pragma once

#include <pluginContainer.h>
#include <scriptCustomFuncAndVarContainer.h>
#include <codeEditorInfos.h>
#include <persistentDataContainer.h>
#include <copyBuffer.h>
#include <addOnScriptContainer.h>
#include <interfaceStackContainer.h>
#include <simulatorMessageQueue.h>
#include <calculationInfo.h>
#include <moduleMenuItemContainer.h>
#include <world.h>
#include <customData.h>
#include <tuple>

#ifdef SIM_WITH_GUI
#include <globalGuiTextureContainer.h>
#include <serialPortContainer.h>
#endif

#define EVENTTYPE_OBJECTADDED "objectAdded"
#define EVENTTYPE_OBJECTCHANGED "objectChanged"
#define EVENTTYPE_OBJECTREMOVED "objectRemoved"

#define EVENTTYPE_ENVIRONMENTCHANGED "environmentChanged"

#define EVENTTYPE_DRAWINGOBJECTADDED "drawingObjectAdded"
#define EVENTTYPE_DRAWINGOBJECTCHANGED "drawingObjectChanged"
#define EVENTTYPE_DRAWINGOBJECTREMOVED "drawingObjectRemoved"

#define EVENTTYPE_APPSESSION "appSession"
#define EVENTTYPE_APPSETTINGSCHANGED "appSettingsChanged"
#define EVENTTYPE_SIMULATIONCHANGED "simulationChanged"
#define EVENTTYPE_GENESISBEGIN "genesisBegin"
#define EVENTTYPE_GENESISEND "genesisEnd"

class CWorldContainer
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
    void getAllSceneNames(std::vector<std::string> &l) const;

    CScriptObject *getScriptObjectFromHandle(int scriptHandle) const;
    CScriptObject *getScriptObjectFromUid(int uid) const;
    void announceObjectWillBeErased(const CSceneObject *object);
    void announceScriptWillBeErased(int scriptHandle, int scriptUid, bool simulationScript, bool sceneSwitchPersistentScript);
    void announceScriptStateWillBeErased(int scriptHandle, int scriptUid, bool simulationScript, bool sceneSwitchPersistentScript);

    void callScripts(int callType, CInterfaceStack *inStack, CInterfaceStack *outStack, CSceneObject *objectBranch = nullptr, int scriptToExclude = -1);
    void broadcastMsg(CInterfaceStack *inStack, int emittingScriptHandle, int options);

    int getSysFuncAndHookCnt(int sysCall) const;

    bool shouldTemporarilySuspendMainScript();
    void pushSceneObjectRemoveEvent(const CSceneObject *object);

    bool getEventsEnabled() const;
    CCbor *createNakedEvent(const char *event, int handle, long long int uid, bool mergeable);
    CCbor *createEvent(const char *event, long long int uid, const char *fieldName, bool mergeable);
    CCbor *createSceneObjectAddEvent(const CSceneObject *object);
    CCbor *createSceneObjectChangedEvent(const CSceneObject *object, bool isCommonObjectData, const char *fieldName,
                                         bool mergeable);
    CCbor *createSceneObjectChangedEvent(int sceneObjectHandle, bool isCommonObjectData, const char *fieldName,
                                         bool mergeable);
    void pushEvent();

    void pushGenesisEvents();
    void getGenesisEvents(std::vector<unsigned char> *genesisEvents, CInterfaceStack *stack);
    void dispatchEvents();

    void simulationAboutToStart();
    void simulationPaused();
    void simulationAboutToResume();
    void simulationAboutToStep();
    void simulationAboutToEnd();
    void simulationEnded(bool removeNewObjects);

    CCopyBuffer *copyBuffer;
    CSimulatorMessageQueue *simulatorMessageQueue;
    CCalculationInfo *calcInfo;
    CInterfaceStackContainer *interfaceStackContainer;
    CPluginContainer *pluginContainer;
    CCodeEditorInfos *codeEditorInfos;
    CScriptCustomFuncAndVarContainer *scriptCustomFuncAndVarContainer; // old

    CCustomData customAppData;
    CCustomData_old *customAppData_old;
    CPersistentDataContainer *persistentDataContainer_old;
    CAddOnScriptContainer *addOnScriptContainer;
    CScriptObject *sandboxScript;
    CModuleMenuItemContainer *moduleMenuItemContainer;
    CWorld *currentWorld;
#ifdef SIM_WITH_GUI
    CGlobalGuiTextureContainer *globalGuiTextureCont;
    CSerialPortContainer *serialPortContainer;
#endif

  private:
    CCbor *_createGeneralEvent(const char *event, int objectHandle, long long int uid, const char *objType,
                               const char *fieldName, bool mergeable, bool openDataField = true);
    bool _switchToWorld(int newWorldIndex);

    std::vector<CWorld *> _worlds;
    int _currentWorldIndex;
    std::string _sessionId;

    static long long int _eventSeq;
    VMutex _eventMutex; // just needed while we are still using the old GUI, since it will also generate events
    CCbor *_events;
    bool _eventsEnabled;

    std::vector<long long int> _uniqueIdsOfSelectionSinceLastTimeGetAndClearModificationFlagsWasCalled;
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
    void addMenu(VMenu *menu);
    int getInstanceIndexOfASceneNotYetSaved(bool doNotIncludeCurrentScene);
    void setInstanceIndexWithThumbnails(int index);
    bool processGuiCommand(int commandID);
#endif
};
