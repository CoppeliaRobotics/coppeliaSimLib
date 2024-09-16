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

#define EVENTTYPE_DRAWINGOBJECTADDED "drawingObjectAdded"
#define EVENTTYPE_DRAWINGOBJECTCHANGED "drawingObjectChanged"
#define EVENTTYPE_DRAWINGOBJECTREMOVED "drawingObjectRemoved"

#define EVENTTYPE_APPSESSION "appSession"
#define EVENTTYPE_APPSETTINGSCHANGED "appSettingsChanged"
#define EVENTTYPE_GENESISBEGIN "genesisBegin"
#define EVENTTYPE_GENESISEND "genesisEnd"
#define EVENTTYPE_MSGDISPATCHTIME "msgDispatchTime"

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES \
    FUNCX(propApp_sessionId,                   "sessionId",                        sim_propertytype_string,    1) \
    FUNCX(propApp_protocolVersion,             "protocolVersion",                  sim_propertytype_int,       1) \
    FUNCX(propApp_productVersion,              "productVersion",                   sim_propertytype_string,    1) \
    FUNCX(propApp_productVersionNb,            "productVersionNb",                 sim_propertytype_int,    1) \
    FUNCX(propApp_platform,                    "platform",                         sim_propertytype_int,    1) \
    FUNCX(propApp_flavor,                      "flavor",                           sim_propertytype_int,    1) \
    FUNCX(propApp_qtVersion,                   "qtVersion",                        sim_propertytype_int,    1) \
    FUNCX(propApp_processId,                   "processId",                        sim_propertytype_int,    1) \
    FUNCX(propApp_processCnt,                  "processCnt",                       sim_propertytype_int,    1) \
    FUNCX(propApp_consoleVerbosity,            "consoleVerbosity",                 sim_propertytype_int,    0) \
    FUNCX(propApp_statusbarVerbosity,          "statusbarVerbosity",               sim_propertytype_int,    0) \
    FUNCX(propApp_defaultTranslationStepSize,  "defaultTranslationStepSize",       sim_propertytype_float,     0) \
    FUNCX(propApp_defaultRotationStepSize,     "defaultRotationStepSize",          sim_propertytype_float,     0) \
    FUNCX(propApp_hierarchyEnabled,            "hierarchyEnabled",                 sim_propertytype_bool,      0) \
    FUNCX(propApp_browserEnabled,              "browserEnabled",                   sim_propertytype_bool,      0) \
    FUNCX(propApp_displayEnabled,              "displayEnabled",                   sim_propertytype_bool,      0) \
    FUNCX(propApp_appDir,                      "appDir",                           sim_propertytype_string,    1) \
    FUNCX(propApp_machineId,                   "machineId",                        sim_propertytype_string,    1) \
    FUNCX(propApp_legacyMachineId,             "legacyMachineId",                  sim_propertytype_string,    1) \
    FUNCX(propApp_tempDir,                     "tempDir",                          sim_propertytype_string,    1) \
    FUNCX(propApp_sceneTempDir,                "sceneTempDir",                     sim_propertytype_string,    1) \
    FUNCX(propApp_settingsDir,                 "settingsDir",                      sim_propertytype_string,    1) \
    FUNCX(propApp_luaDir,                      "luaDir",                           sim_propertytype_string,    1) \
    FUNCX(propApp_pythonDir,                   "pythonDir",                        sim_propertytype_string,    1) \
    FUNCX(propApp_mujocoDir,                   "mujocoDir",                        sim_propertytype_string,    1) \
    FUNCX(propApp_systemDir,                   "systemDir",                        sim_propertytype_string,    1) \
    FUNCX(propApp_resourceDir,                 "resourceDir",                      sim_propertytype_string,    1) \
    FUNCX(propApp_addOnDir,                    "addOnDir",                         sim_propertytype_string,    1) \
    FUNCX(propApp_sceneDir,                    "sceneDir",                         sim_propertytype_string,    0) \
    FUNCX(propApp_modelDir,                    "modelDir",                         sim_propertytype_string,    0) \
    FUNCX(propApp_importExportDir,             "importExportDir",                  sim_propertytype_string,    0) \
    FUNCX(propApp_defaultPython,               "defaultPython",                    sim_propertytype_string,    1) \
    FUNCX(propApp_sandboxLang,                 "sandboxLang",                      sim_propertytype_string,    1) \

#define FUNCX(name, str, v1, v2) const SProperty name = {str, v1, v2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2) name,
const std::vector<SProperty> allProps_app = { DEFINE_PROPERTIES };
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

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
    CCbor *createSceneObjectChangedEvent(const CSceneObject *object, bool isCommonObjectData, const char *fieldName, bool mergeable);
    CCbor *createSceneObjectChangedEvent(int sceneObjectHandle, bool isCommonObjectData, const char *fieldName, bool mergeable);
    CCbor *createObjectChangedEvent(int objectHandle, const char *fieldName, bool mergeable);
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

    int setBoolProperty(const char* pName, bool pState);
    int getBoolProperty(const char* pName, bool& pState) const;
    int setIntProperty(const char* pName, int pState);
    int getIntProperty(const char* pName, int& pState) const;
    int setFloatProperty(const char* pName, double pState);
    int getFloatProperty(const char* pName, double& pState) const;
    int setStringProperty(const char* pName, const char* pState);
    int getStringProperty(const char* pName, std::string& pState) const;
    int setBufferProperty(const char* pName, const char* buffer, int bufferL);
    int getBufferProperty(const char* pName, std::string& pState) const;
    int setVector3Property(const char* pName, const C3Vector& pState);
    int getVector3Property(const char* pName, C3Vector& pState) const;
    int setQuaternionProperty(const char* pName, const C4Vector& pState);
    int getQuaternionProperty(const char* pName, C4Vector& pState) const;
    int setPoseProperty(const char* pName, const C7Vector& pState);
    int getPoseProperty(const char* pName, C7Vector& pState) const;
    int setMatrix3x3Property(const char* pName, const C3X3Matrix& pState);
    int getMatrix3x3Property(const char* pName, C3X3Matrix& pState) const;
    int setMatrix4x4Property(const char* pName, const C4X4Matrix& pState);
    int getMatrix4x4Property(const char* pName, C4X4Matrix& pState) const;
    int setColorProperty(const char* pName, const float* pState);
    int getColorProperty(const char* pName, float* pState) const;
    int setVectorProperty(const char* pName, const double* v, int vL);
    int getVectorProperty(const char* pName, std::vector<double>& pState) const;
    int setIntVectorProperty(const char* pName, const int* v, int vL);
    int getIntVectorProperty(const char* pName, std::vector<int>& pState) const;
    int removeProperty(const char* pName);
    static int getPropertyName(int& index, std::string& pName, CWorldContainer* targetObject);
    static int getPropertyInfo(const char* pName, int& info, CWorldContainer* targetObject);

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
