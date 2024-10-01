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

#define EVENTTYPE_GENESISBEGIN "genesisBegin"
#define EVENTTYPE_GENESISEND "genesisEnd"
#define EVENTTYPE_MSGDISPATCHTIME "msgDispatchTime"

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES \
    FUNCX(propApp_sessionId,                   "sessionId",                        sim_propertytype_string,    sim_propertyinfo_notwritable, "Session ID", "") \
    FUNCX(propApp_protocolVersion,             "protocolVersion",                  sim_propertytype_int,       sim_propertyinfo_notwritable, "Protocol", "Protocol version") \
    FUNCX(propApp_productVersion,              "productVersion",                   sim_propertytype_string,    sim_propertyinfo_notwritable, "Product string", "Product version (string)") \
    FUNCX(propApp_productVersionNb,            "productVersionNb",                 sim_propertytype_int,    sim_propertyinfo_notwritable, "Product", "Product version (number)") \
    FUNCX(propApp_platform,                    "platform",                         sim_propertytype_int,    sim_propertyinfo_notwritable, "Platform", "OS") \
    FUNCX(propApp_flavor,                      "flavor",                           sim_propertytype_int,    sim_propertyinfo_notwritable, "Flavor", "") \
    FUNCX(propApp_qtVersion,                   "qtVersion",                        sim_propertytype_int,    sim_propertyinfo_notwritable, "Qt", "Qt version") \
    FUNCX(propApp_processId,                   "processId",                        sim_propertytype_int,    sim_propertyinfo_notwritable, "Process", "Process ID") \
    FUNCX(propApp_processCnt,                  "processCnt",                       sim_propertytype_int,    sim_propertyinfo_notwritable, "Processes", "Overall processes") \
    FUNCX(propApp_consoleVerbosity,            "consoleVerbosity",                 sim_propertytype_int,    0, "Console verbosity", "") \
    FUNCX(propApp_statusbarVerbosity,          "statusbarVerbosity",               sim_propertytype_int,    0, "Statusbar verbosity", "") \
    FUNCX(propApp_defaultTranslationStepSize,  "defaultTranslationStepSize",       sim_propertytype_float,     0, "Translation step size", "Default translation step size") \
    FUNCX(propApp_defaultRotationStepSize,     "defaultRotationStepSize",          sim_propertytype_float,     0, "Rotation step size", "Default rotation step size") \
    FUNCX(propApp_hierarchyEnabled,            "hierarchyEnabled",                 sim_propertytype_bool,      0, "Hierarchy enabled", "") \
    FUNCX(propApp_browserEnabled,              "browserEnabled",                   sim_propertytype_bool,      0, "Browser enabled", "") \
    FUNCX(propApp_displayEnabled,              "displayEnabled",                   sim_propertytype_bool,      0, "Display enabled", "") \
    FUNCX(propApp_appDir,                      "appPath",                          sim_propertytype_string,    sim_propertyinfo_notwritable, "Application path", "") \
    FUNCX(propApp_machineId,                   "machineId",                        sim_propertytype_string,    sim_propertyinfo_notwritable, "Machine ID", "") \
    FUNCX(propApp_legacyMachineId,             "legacyMachineId",                  sim_propertytype_string,    sim_propertyinfo_notwritable, "Legacy machine ID", "") \
    FUNCX(propApp_tempDir,                     "tempPath",                         sim_propertytype_string,    sim_propertyinfo_notwritable, "Temporary path", "") \
    FUNCX(propApp_sceneTempDir,                "sceneTempPath",                    sim_propertytype_string,    sim_propertyinfo_notwritable, "Scene temporary path", "") \
    FUNCX(propApp_settingsDir,                 "settingsPath",                     sim_propertytype_string,    sim_propertyinfo_notwritable, "Settings path", "") \
    FUNCX(propApp_luaDir,                      "luaPath",                          sim_propertytype_string,    sim_propertyinfo_notwritable, "Lua path", "") \
    FUNCX(propApp_pythonDir,                   "pythonPath",                       sim_propertytype_string,    sim_propertyinfo_notwritable, "Python path", "") \
    FUNCX(propApp_mujocoDir,                   "mujocoPath",                       sim_propertytype_string,    sim_propertyinfo_notwritable, "MuJoCo path", "") \
    FUNCX(propApp_systemDir,                   "systemPath",                       sim_propertytype_string,    sim_propertyinfo_notwritable, "System path", "") \
    FUNCX(propApp_resourceDir,                 "resourcePath",                     sim_propertytype_string,    sim_propertyinfo_notwritable, "Resource path", "") \
    FUNCX(propApp_addOnDir,                    "addOnPath",                        sim_propertytype_string,    sim_propertyinfo_notwritable, "Add-on path", "") \
    FUNCX(propApp_sceneDir,                    "scenePath",                        sim_propertytype_string,    0, "Scene path", "") \
    FUNCX(propApp_modelDir,                    "modelPath",                        sim_propertytype_string,    0, "Model path", "") \
    FUNCX(propApp_importExportDir,             "importExportPath",                 sim_propertytype_string,    0, "Import/export path", "") \
    FUNCX(propApp_defaultPython,               "defaultPython",                    sim_propertytype_string,    sim_propertyinfo_notwritable, "Default Python", "Default Python interpreter") \
    FUNCX(propApp_sandboxLang,                 "sandboxLang",                      sim_propertytype_string,    sim_propertyinfo_notwritable, "Sandbox language", "Default sandbox language") \

#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
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
    void announceScriptWillBeErased(int scriptHandle, long long int scriptUid, bool simulationScript, bool sceneSwitchPersistentScript);
    void announceScriptStateWillBeErased(int scriptHandle, long long int scriptUid, bool simulationScript, bool sceneSwitchPersistentScript);

    void callScripts(int callType, CInterfaceStack *inStack, CInterfaceStack *outStack, CSceneObject *objectBranch = nullptr, int scriptToExclude = -1);
    void broadcastMsg(CInterfaceStack *inStack, int emittingScriptHandle, int options);

    int getSysFuncAndHookCnt(int sysCall) const;

    bool shouldTemporarilySuspendMainScript();
    void pushSceneObjectRemoveEvent(const CSceneObject *object);

    bool getEventsEnabled() const;
    CCbor *createNakedEvent(const char *event, long long int handle, long long int uid, bool mergeable); // has no 'data' field
    CCbor *createEvent(const char *event, long long int handle, long long int uid, const char *fieldName, bool mergeable);
    CCbor *createSceneObjectAddEvent(const CSceneObject *object);
    CCbor *createSceneObjectChangedEvent(const CSceneObject *object, bool isCommonObjectData, const char *fieldName, bool mergeable);
    CCbor *createSceneObjectChangedEvent(long long int sceneObjectHandle, bool isCommonObjectData, const char *fieldName, bool mergeable);
    CCbor *createObjectChangedEvent(long long int objectHandle, const char *fieldName, bool mergeable);
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
    int setIntArray2Property(const char* pName, const int* pState);
    int getIntArray2Property(const char* pName, int* pState) const;
    int setVector2Property(const char* pName, const double* pState);
    int getVector2Property(const char* pName, double* pState) const;
    int setVector3Property(const char* pName, const C3Vector& pState);
    int getVector3Property(const char* pName, C3Vector& pState) const;
    int setQuaternionProperty(const char* pName, const C4Vector& pState);
    int getQuaternionProperty(const char* pName, C4Vector& pState) const;
    int setPoseProperty(const char* pName, const C7Vector& pState);
    int getPoseProperty(const char* pName, C7Vector& pState) const;
    int setColorProperty(const char* pName, const float* pState);
    int getColorProperty(const char* pName, float* pState) const;
    int setFloatArrayProperty(const char* pName, const double* v, int vL);
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const;
    int setIntArrayProperty(const char* pName, const int* v, int vL);
    int getIntArrayProperty(const char* pName, std::vector<int>& pState) const;
    int removeProperty(const char* pName);
    static int getPropertyName(int& index, std::string& pName, CWorldContainer* targetObject);
    static int getPropertyInfo(const char* pName, int& info, std::string& infoTxt, CWorldContainer* targetObject);

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
    CCbor *_createGeneralEvent(const char *event, long long int objectHandle, long long int uid, const char *objType,
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
