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
#include <tuple>

#ifdef SIM_WITH_GUI
    #include "globalGuiTextureContainer.h"
#endif

#ifdef SIM_WITH_SERIAL
    #include "serialPortContainer.h"
#endif


#define EVENTTYPE_OBJECTADDED "objectAdded"
#define EVENTTYPE_OBJECTCHANGED "objectChanged"
#define EVENTTYPE_OBJECTREMOVED "objectRemoved"
#define EVENTTYPE_SCENECHANGED "sceneChanged"

#define EVENTTYPE_DRAWINGOBJECTADDED "drawingObjectAdded"
#define EVENTTYPE_DRAWINGOBJECTCHANGED "drawingObjectChanged"
#define EVENTTYPE_DRAWINGOBJECTREMOVED "drawingObjectRemoved"

#define EVENTTYPE_SYSTEMCHANGED "systemChanged"

struct SEventInfo
{
    CInterfaceStackTable* eventTable;
    std::string event;
    std::string subEvent;
    std::string dataSubtype;
    int uid;
    bool mergeable;
};

struct SBufferedEvents
{
    CInterfaceStack* eventsStack;
    std::vector<SEventInfo> eventDescriptions;
};


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

    std::tuple<SEventInfo,CInterfaceStackTable*> prepareSystemEvent(const char* fieldName,bool mergeable);
    std::tuple<SEventInfo,CInterfaceStackTable*> prepareEvent(const char* event,int uid,const char* fieldName,bool mergeable);
    void pushSceneObjectRemoveEvent(const _CSceneObject_* object);
    std::tuple<SEventInfo,CInterfaceStackTable*> prepareSceneObjectAddEvent(const _CSceneObject_* object);
    std::tuple<SEventInfo,CInterfaceStackTable*> prepareSceneObjectChangedEvent(const _CSceneObject_* object,bool isCommonObjectData,const char* fieldName,bool mergeable);
    std::tuple<SEventInfo,CInterfaceStackTable*> prepareSceneObjectChangedEvent(int sceneObjectHandle,bool isCommonObjectData,const char* fieldName,bool mergeable);
    std::tuple<SEventInfo,CInterfaceStackTable*> _prepareGeneralEvent(const char* event,int objectHandle,int uid,const char* objType,const char* fieldName,bool mergeable);
    void _combineDuplicateEvents(SBufferedEvents* events);
    void _mergeEvents(SBufferedEvents* events);


    void pushEvent(SEventInfo& event);
    void sendEvents();
    bool getCborEvents() const;
    void setCborEvents(bool b);
    void setMergeEvents(bool b);
    bool getEnableEvents() const;
    void setEnableEvents(bool b);
    void pushReconstructAllEvents();
    void buildReconstructAllEventsOntoInterfaceStack(CInterfaceStack* stack);
    SBufferedEvents* swapBufferedEvents(SBufferedEvents* newBuffer);
    void pushReconstructSettingsEvents();

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

    static long long int _eventSeq;
    SBufferedEvents* _bufferedEvents;
    VMutex _eventMutex;
    bool _cborEvents;
    bool _mergeTheEvents;
    bool _enableEvents;

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
