#pragma once

#include <string>

struct SProperty {
    const char* name;
    int type;
    int flags;
    const std::string shortInfoTxt;
    const std::string infoTxt;
};

struct SDynProperty
{
    const char* name;
    int type;
    int flags;
    int oldEnums[5];
    const std::string shortInfoTxt;
    const std::string infoTxt;
};

struct SEngMaterialProperty
{
    const char* name;
    int type;
    int flags;
    int oldEnums[5];
    const std::string shortInfoTxt;
    const std::string infoTxt;
};

struct SDummyProperty
{
    const char* name;
    int type;
    int flags;
    int oldEnums[5];
    const std::string shortInfoTxt;
    const std::string infoTxt;
};

struct SJointProperty
{
    const char* name;
    int type;
    int flags;
    int oldEnums[5];
    const std::string shortInfoTxt;
    const std::string infoTxt;
};

#define CUSTOMDATAPREFIX "customData."
#define CUSTOMDATAFLAGS (sim_propertyinfo_removable)

#define SIGNALPREFIX "signal."
#define SIGNALFLAGS (sim_propertyinfo_removable | sim_propertyinfo_modelhashexclude)

#define NAMEDPARAMPREFIX "namedParam."
#define NAMEDPARAMFLAGS (sim_propertyinfo_removable | sim_propertyinfo_modelhashexclude)

#define REFSPREFIX "refs."
#define REFSFLAGS (sim_propertyinfo_removable | sim_propertyinfo_modelhashexclude)

#define ORIGREFSPREFIX "origRefs."
#define ORIGREFSFLAGS (sim_propertyinfo_removable | sim_propertyinfo_modelhashexclude)

#define COLORPREFIX "color."
#define COLORPREFIX_CAP "Color."

#define proptypetag_bool "&bool&."
#define proptypetag_int "&int&."
#define proptypetag_float "&dbl&."
#define proptypetag_string "&str&."
#define proptypetag_buffer ""
#define proptypetag_intarray2 "&ivect2&."
#define proptypetag_long "&lng&."
#define proptypetag_vector3 "&vect3&."
#define proptypetag_quaternion "&quat&."
#define proptypetag_pose "&pose&."
#define proptypetag_color "&col&."
#define proptypetag_floatarray "&vect&."
#define proptypetag_intarray "&ivect&."
#define proptypetag_table "&tbl&."
#define proptypetag_matrix "&mtrxXX&."
#define proptypetag_array "&arr&."
#define proptypetag_map "&map&."
#define proptypetag_null "&nul&."
#define proptypetag_handle "&han&."
#define proptypetag_handlearray "&hanvect&."
#define proptypetag_stringarray "&strvect&."

#define OBJECT_PROPERTIES \
    FUNCX(propObject_objectType, "objectType", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, jsonStr({{"label", "Object type"}, {"description", ""}}), "") \
    FUNCX(propObject_handle, "handle", sim_propertytype_long, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, jsonStr({{"label", "Handle"}, {"description", ""}}), "") \
    FUNCX(propObject_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, jsonStr({{"label", "Object meta information"}, {"description", ""}}), "") \
    FUNCX(propObject_METHOD_getBoolProperty, "getBoolProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getBufferProperty, "getBufferProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getColorProperty, "getColorProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getFloatArrayProperty, "getFloatArrayProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getFloatProperty, "getFloatProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getStringArrayProperty, "getStringArrayProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getHandleArrayProperty, "getHandleArrayProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getHandleProperty, "getHandleProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getIntArray2Property, "getIntArray2Property", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getIntArrayProperty, "getIntArrayProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getIntProperty, "getIntProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getLongProperty, "getLongProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getPoseProperty, "getPoseProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getQuaternionProperty, "getQuaternionProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getStringProperty, "getStringProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getTableProperty, "getTableProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getVector2Property, "getVector2Property", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getVector3Property, "getVector3Property", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setBoolProperty, "setBoolProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setBufferProperty, "setBufferProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setColorProperty, "setColorProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setFloatArrayProperty, "setFloatArrayProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setFloatProperty, "setFloatProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setStringArrayProperty, "setStringArrayProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setHandleArrayProperty, "setHandleArrayProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setHandleProperty, "setHandleProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setIntArray2Property, "setIntArray2Property", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setIntArrayProperty, "setIntArrayProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setIntProperty, "setIntProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setLongProperty, "setLongProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setPoseProperty, "setPoseProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setQuaternionProperty, "setQuaternionProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setStringProperty, "setStringProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setTableProperty, "setTableProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setVector2Property, "setVector2Property", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setVector3Property, "setVector3Property", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setMatrixProperty, "setMatrixProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getMatrixProperty, "getMatrixProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setMethodProperty, "setMethodProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getMethodProperty, "getMethodProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_removeProperty, "removeProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getPropertyInfo, "getPropertyInfo", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getPropertyName, "getPropertyName", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getProperties, "getProperties", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setProperties, "setProperties", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setProperty, "setProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getPropertiesInfos, "getPropertiesInfos", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getProperty, "getProperty", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getPropertyTypeString, "getPropertyTypeString", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_isValid, "isValid", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "")

#define APP_PROPERTIES \
    FUNCX(propApp_sessionId, "sessionId", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Session ID"}, {"description", ""}}), "") \
    FUNCX(propApp_protocolVersion, "protocolVersion", sim_propertytype_int, 0,  jsonStr({{"label", "Protocol"}, {"description", "Protocol version"}}), "") \
    FUNCX(propApp_productVersion, "productVersion", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Product string"}, {"description", "Product version (string)"}}), "") \
    FUNCX(propApp_productVersionNb, "productVersionNb", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Product"}, {"description", "Product version (number)"}}), "") \
    FUNCX(propApp_platform, "platform", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Platform"}, {"description", "Platform (0: Windows, 1: macOS, 2: Linux)"}}), "") \
    FUNCX(propApp_flavor, "flavor", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Flavor"}, {"description", "Flavor (0: lite, 1: edu, 2: pro)"}}), "") \
    FUNCX(propApp_qtVersion, "qtVersion", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Qt"}, {"description", "Qt version"}}), "") \
    FUNCX(propApp_processId, "processId", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Process"}, {"description", "Process ID"}}), "") \
    FUNCX(propApp_processCnt, "processCnt", sim_propertytype_int, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Processes"}, {"description", "Overall processes"}}), "") \
    FUNCX(propApp_consoleVerbosity, "consoleVerbosity", sim_propertytype_int, 0,  jsonStr({{"label", "Console verbosity"}, {"description", ""}}), "") \
    FUNCX(propApp_statusbarVerbosity, "statusbarVerbosity", sim_propertytype_int, 0,  jsonStr({{"label", "Statusbar verbosity"}, {"description", ""}}), "") \
    FUNCX(propApp_dialogVerbosity, "dialogVerbosity", sim_propertytype_int, 0,  jsonStr({{"label", "Dialog verbosity"}, {"description", ""}}), "") \
    FUNCX(propApp_consoleVerbosityStr, "consoleVerbosityStr", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Console verbosity string"}, {"description", "Console verbosity string, only for client app"}}), "") \
    FUNCX(propApp_statusbarVerbosityStr, "statusbarVerbosityStr", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Statusbar verbosity string"}, {"description", "Statusbar verbosity string, only for client app"}}), "") \
    FUNCX(propApp_dialogVerbosityStr, "dialogVerbosityStr", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Dialog verbosity string"}, {"description", "Dialog verbosity string, only for client app"}}), "") \
    FUNCX(propApp_auxAddOn1, "auxAddOn1", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Aux. add-on 1"}, {"description", "Auxiliary add-on 1"}}), "") \
    FUNCX(propApp_auxAddOn2, "auxAddOn2", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Aux. add-on 2"}, {"description", "Auxiliary add-on 2"}}), "") \
    FUNCX(propApp_startupCode, "startupCode", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Start-up code"}, {"description", ""}}), "") \
    FUNCX(propApp_defaultTranslationStepSize, "defaultTranslationStepSize", sim_propertytype_float, 0,  jsonStr({{"label", "Translation step size"}, {"description", "Default translation step size"}}), "") \
    FUNCX(propApp_defaultRotationStepSize, "defaultRotationStepSize", sim_propertytype_float, 0,  jsonStr({{"label", "Rotation step size"}, {"description", "Default rotation step size"}}), "") \
    FUNCX(propApp_hierarchyEnabled, "hierarchyEnabled", sim_propertytype_bool, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Hierarchy enabled"}, {"description", ""}}), "") \
    FUNCX(propApp_browserEnabled, "browserEnabled", sim_propertytype_bool, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Browser enabled"}, {"description", ""}}), "") \
    FUNCX(propApp_displayEnabled, "displayEnabled", sim_propertytype_bool, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Display enabled"}, {"description", ""}}), "") \
    FUNCX(propApp_appDir, "appPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Application path"}, {"description", ""}}), "") \
    FUNCX(propApp_machineId, "machineId", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Machine ID"}, {"description", ""}}), "") \
    FUNCX(propApp_legacyMachineId, "legacyMachineId", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Legacy machine ID"}, {"description", ""}}), "") \
    FUNCX(propApp_tempDir, "tempPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Temporary path"}, {"description", ""}}), "") \
    FUNCX(propApp_sceneTempDir, "sceneTempPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Scene temporary path"}, {"description", ""}}), "") \
    FUNCX(propApp_settingsDir, "settingsPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Settings path"}, {"description", ""}}), "") \
    FUNCX(propApp_luaDir, "luaPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Lua path"}, {"description", ""}}), "") \
    FUNCX(propApp_pythonDir, "pythonPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Python path"}, {"description", ""}}), "") \
    FUNCX(propApp_mujocoDir, "mujocoPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "MuJoCo path"}, {"description", ""}}), "") \
    FUNCX(propApp_systemDir, "systemPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "System path"}, {"description", ""}}), "") \
    FUNCX(propApp_resourceDir, "resourcePath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Resource path"}, {"description", ""}}), "") \
    FUNCX(propApp_addOnDir, "addOnPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Add-on path"}, {"description", ""}}), "") \
    FUNCX(propApp_sceneDir, "scenePath", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Scene path"}, {"description", ""}}), "") \
    FUNCX(propApp_modelDir, "modelPath", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Model path"}, {"description", ""}}), "") \
    FUNCX(propApp_importExportDir, "importExportPath", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Import/export path"}, {"description", ""}}), "") \
    FUNCX(propApp_defaultPython, "defaultPython", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Default Python"}, {"description", "Default Python interpreter"}}), "") \
    FUNCX(propApp_sandboxLang, "sandboxLang", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Sandbox language"}, {"description", "Default sandbox language"}}), "") \
    FUNCX(propApp_headlessMode, "headlessMode", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Headless mode"}, {"description", "Headless mode (0: not headless, 1: GUI suppressed, 2: headless library)"}}), "") \
    FUNCX(propApp_canSave, "canSave", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Can save"}, {"description", "Whether save operation is allowed in given state"}}), "") \
    FUNCX(propApp_idleFps, "idleFps", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Loaded plugin names"}, {"description", ""}}), "") \
    FUNCX(propApp_pluginNames, "pluginNames", sim_propertytype_stringarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Plugins"}, {"description", "List of plugins"}}), "") \
    FUNCX(propApp_addOns, "addOns", sim_propertytype_handlearray, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Add-ons"}, {"description", "List of add-ons"}, {"handleType", "detachedScript"}}), "") \
    FUNCX(propApp_customObjects, "customObjects", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Custom objects"}, {"description", "List of app custom objects"}, {"handleType", "customObject"}}), "") \
    FUNCX(propApp_customClasses, "customClasses", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Custom classes"}, {"description", "List of app custom classes"}, {"handleType", "customObject"}}), "") \
    FUNCX(propApp_customSceneObjectClasses, "customSceneObjectClasses", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Custom scene object classes"}, {"description", "List of app custom scene object classes"}, {"handleType", "sceneObject"}}), "") \
    FUNCX(propApp_sandbox, "sandbox", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Sandbox"}, {"description", "Handle of the sandbox script"}, {"handleType", "detachedScript"}}), "") \
    FUNCX(propApp_appArgs, "appArgs", sim_propertytype_stringarray, sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App args"}, {"description", ""}}), "") \
    FUNCX(propApp_randomQuaternion, "randomQuaternion", sim_propertytype_quaternion, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Random quaternion"}, {"description", ""}}), "") \
    FUNCX(propApp_randomFloat, "randomFloat", sim_propertytype_float, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Random number"}, {"description", ""}}), "") \
    FUNCX(propApp_randomString, "randomString", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Random string"}, {"description", ""}}), "") \
    FUNCX(propApp_notifyDeprecated, "notifyDeprecated", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Notify deprecated"}, {"description", "Notify deprecated API (0: off, 1: light, 2: full)"}}), "") \
    FUNCX(propApp_execUnsafe, "execUnsafe", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Execute unsafe"}, {"description", ""}}), "") \
    FUNCX(propApp_execUnsafeExt, "execUnsafeExt", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Execute unsafe extended"}, {"description", "Execute unsafe for code triggered externally"}}), "") \
    FUNCX(propApp_dongleSerial, "dongleSerial", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propApp_machineSerialND, "machineSerialND", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propApp_machineSerial, "machineSerial", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propApp_dongleID, "dongleID", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propApp_machineIDX, "machineIDX", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propApp_machineID0, "machineID0", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propApp_machineID1, "machineID1", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propApp_machineID2, "machineID2", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propApp_machineID3, "machineID3", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propApp_pid, "pid", sim_propertytype_long, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "PID"}, {"description", ""}}), "") \
    FUNCX(propApp_systemTime, "systemTime", sim_propertytype_float, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "System time"}, {"description", ""}}), "") \
    FUNCX(propApp_METHOD_handleAddOnScripts, "handleAddOnScripts", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_handleSandboxScript, "handleSandboxScript", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_loadModelThumbnail, "loadModelThumbnail", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_loadModelThumbnailFromBuffer, "loadModelThumbnailFromBuffer", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_loadScene, "loadScene", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_loadSceneFromBuffer, "loadSceneFromBuffer", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_getObjects, "getObjects", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_createObject, "createObject", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_duplicateObjects, "duplicateObjects", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_loadImage, "loadImage", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_loadImageFromBuffer, "loadImageFromBuffer", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_saveImage, "saveImage", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_saveImageToBuffer, "saveImageToBuffer", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_transformImage, "transformImage", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_removeObjects, "removeObjects", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_logInfo, "logInfo", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_logWarn, "logWarn", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_logError, "logError", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_systemLock, "systemLock", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packTable, "packTable", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackTable, "unpackTable", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_pack, "pack", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpack, "unpack", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_serialize, "serialize", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_deserialize, "deserialize", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packDoubleTable, "packDoubleTable", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packFloatTable, "packFloatTable", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packInt64Table, "packInt64Table", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packInt32Table, "packInt32Table", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packUInt32Table, "packUInt32Table", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packInt16Table, "packInt16Table", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packUInt16Table, "packUInt16Table", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packInt8Table, "packInt8Table", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packUInt8Table, "packUInt8Table", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackDoubleTable, "unpackDoubleTable", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackFloatTable, "unpackFloatTable", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackInt64Table, "unpackInt64Table", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackInt32Table, "unpackInt32Table", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackUInt32Table, "unpackUInt32Table", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackInt16Table, "unpackInt16Table", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackUInt16Table, "unpackUInt16Table", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackInt8Table, "unpackInt8Table", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackUInt8Table, "unpackUInt8Table", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_fastIdleLoop, "fastIdleLoop", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_throttle, "throttle", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_scheduleExecution, "scheduleExecution", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_cancelScheduledExecution, "cancelScheduledExecution", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_openFile, "openFile", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_createCustomObjectClass, "createCustomObjectClass", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_quit, "quit", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propApp_DEPRECATED_appArg1, "appArg1", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 1"}, {"description", ""}}), "") \
    FUNCX(propApp_DEPRECATED_appArg2, "appArg2", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 2"}, {"description", ""}}), "") \
    FUNCX(propApp_DEPRECATED_appArg3, "appArg3", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 3"}, {"description", ""}}), "") \
    FUNCX(propApp_DEPRECATED_appArg4, "appArg4", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 4"}, {"description", ""}}), "") \
    FUNCX(propApp_DEPRECATED_appArg5, "appArg5", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 5"}, {"description", ""}}), "") \
    FUNCX(propApp_DEPRECATED_appArg6, "appArg6", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 6"}, {"description", ""}}), "") \
    FUNCX(propApp_DEPRECATED_appArg7, "appArg7", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 7"}, {"description", ""}}), "") \
    FUNCX(propApp_DEPRECATED_appArg8, "appArg8", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 8"}, {"description", ""}}), "") \
    FUNCX(propApp_DEPRECATED_appArg9, "appArg9", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 9"}, {"description", ""}}), "")

#define CUSTOMOBJECTCLASS_PROPERTIES \
    FUNCX(propCustomObjectClass_METHOD_makeOject, "makeObject", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propCustomObjectClass_METHOD_remove, "remove", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "")

#define CUSTOMOBJECT_PROPERTIES \
    FUNCX(propCustomObject_METHOD_remove, "remove", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "")

#define DETACHEDSCRIPT_PROPERTIES \
    FUNCX(propDetachedScript_scriptDisabled, "disabled", sim_propertytype_bool, 0,  jsonStr({{"label", "Disabled"}, {"description", "Distabled state"}}), "") \
    FUNCX(propDetachedScript_restartOnError, "restartOnError", sim_propertytype_bool, 0,  jsonStr({{"label", "Restart"}, {"description", "Restart on error"}}), "") \
    FUNCX(propDetachedScript_execPriority, "execPriority", sim_propertytype_int, 0,  jsonStr({{"label", "Execution priority"}, {"description", ""}}), "") \
    FUNCX(propDetachedScript_scriptType, "type", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Type"}, {"description", "Script type"}}), "") \
    FUNCX(propDetachedScript_executionDepth, "executionDepth", sim_propertytype_int, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Execution depth"}, {"description", ""}}), "") \
    FUNCX(propDetachedScript_scriptState, "state", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "State"}, {"description", "Script state"}}), "") \
    FUNCX(propDetachedScript_language, "language", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Language"}, {"description", ""}}), "") \
    FUNCX(propDetachedScript_code, "code", sim_propertytype_string, 0,  jsonStr({{"label", "Code"}, {"description", "Script content"}}), "") \
    FUNCX(propDetachedScript_scriptName, "scriptName", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Name"}, {"description", "Script name"}}), "") \
    FUNCX(propDetachedScript_addOnPath, "addOnPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Add-on path"}, {"description", "Path of add-on"}}), "") \
    FUNCX(propDetachedScript_addOnMenuPath, "addOnMenuPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Add-on menu path"}, {"description", "Menu path of add-on"}}), "") \
    FUNCX(propDetachedScript_METHOD_callFunction, "callFunction", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_getFunctions, "getFunctions", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_executeString, "executeString", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_getApiFunc, "getApiFunc", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_getApiInfo, "getApiInfo", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_getStackTraceback, "getStackTraceback", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_remove, "remove", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_registerFunctionHook, "registerFunctionHook", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_removeFunctionHook, "removeFunctionHook", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_lock, "lock", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_getObject, "getObject", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_setStepping, "setStepping", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_getStepping, "getStepping", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_yield, "yield", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_step, "step", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_wait, "wait", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_init, "init", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propDetachedScript_DEPRECATED_scriptType, "scriptType", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude,  jsonStr({{"", ""}, {"", ""}}), "") \
    FUNCX(propDetachedScript_DEPRECATED_scriptDisabled, "scriptDisabled", sim_propertytype_bool, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude,  jsonStr({{"", ""}, {"", ""}}), "") \
    FUNCX(propDetachedScript_DEPRECATED_scriptState, "scriptState", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"", ""}, {"", ""}}), "")

#define STACK_PROPERTIES \
    FUNCX(propStack_content, "content", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Content"}, {"description", ""}}), "")

#define COLLECTIONCONT_PROPERTIES \
    FUNCX(propCollectionCont_collections, "collections", sim_propertytype_handlearray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Collections"}, {"description", "Handles of all collections"}, {"handleType", "collection"}}), "")

#define COLLECTION_PROPERTIES \
    FUNCX(propCollection_objects, "objects", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Children handles"}, {"description", ""}, {"handleType", "sceneObject"}}), "") \
    FUNCX(propCollection_METHOD_addItem, "addItem", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propCollection_METHOD_removeItem, "removeItem", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propCollection_METHOD_checkCollision, "checkCollision", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propCollection_METHOD_remove, "remove", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propCollection_METHOD_changeColor, "changeColor", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propCollection_METHOD_restoreColor, "restoreColor", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propCollection_METHOD_checkDistance, "checkDistance", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "")

#define DRAWINGOBJECTCONT_PROPERTIES \
    FUNCX(propDrawingObjectCont_drawingObjects, "drawingObjects", sim_propertytype_handlearray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Drawing objects"}, {"description", "Handles of all drawing objects"}, {"handleType", "drawingObject"}}), "")

#define DRAWINGOBJECT_PROPERTIES \
    FUNCX(propDrawingObject_parent, "parent", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Parent handle"}, {"description", ""}, {"handleType", "sceneObject"}}), "") \
    FUNCX(propDrawingObject_METHOD_remove, "remove", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "")

#define COLOR_PROPERTIES \
    FUNCX(propColor_colDiffuse, "diffuse", sim_propertytype_color, 0,  jsonStr({{"label", "Diffuse color"}, {"description", ""}}), "") \
    FUNCX(propColor_colSpecular, "specular", sim_propertytype_color, 0,  jsonStr({{"label", "Specular color"}, {"description", ""}}), "") \
    FUNCX(propColor_colEmission, "emission", sim_propertytype_color, 0,  jsonStr({{"label", "Emission color"}, {"description", ""}}), "") \
    FUNCX(propColor_transparency, "transparency", sim_propertytype_float, 0,  jsonStr({{"label", "Transparency"}, {"description", ""}}), "")

#define CONVEXVOLUME_PROPERTIES \
    FUNCX(propConvexVolume_closeThreshold, "closeThreshold", sim_propertytype_float, 0,  jsonStr({{"label", "Close threshold"}, {"description", "Close threshold: if a detection occures below that threshold, it is not registered. 0.0 to disable"}}), "") \
    FUNCX(propConvexVolume_offset, "volume.offset", sim_propertytype_float, 0,  jsonStr({{"label", "Offset"}, {"description", "Offset of detection volume"}}), "") \
    FUNCX(propConvexVolume_range, "volume.range", sim_propertytype_float, 0,  jsonStr({{"label", "Range"}, {"description", "Range/depth of detection volume"}}), "") \
    FUNCX(propConvexVolume_xSize, "volume.xSize", sim_propertytype_floatarray, 0,  jsonStr({{"label", "X-sizes"}, {"description", "X-size (near and far) for pyramid-type volumes"}}), "") \
    FUNCX(propConvexVolume_ySize, "volume.ySize", sim_propertytype_floatarray, 0,  jsonStr({{"label", "Y-sizes"}, {"description", "Y-size (near and far) for pyramid-type volumes"}}), "") \
    FUNCX(propConvexVolume_radius, "volume.radius", sim_propertytype_floatarray, 0,  jsonStr({{"label", "Radius"}, {"description", "Radius for cylinder-, disk- and cone-type volumes"}}), "") \
    FUNCX(propConvexVolume_angle, "volume.angle", sim_propertytype_floatarray, 0,  jsonStr({{"label", "Angles"}, {"description", "Angle and inside gap for disk- and cone-type volumes"}}), "") \
    FUNCX(propConvexVolume_faces, "volume.faces", sim_propertytype_intarray, 0,  jsonStr({{"label", "Faces"}, {"description", "Number of faces (near and far) for cylinder-, disk- and pyramid-type volumes"}}), "") \
    FUNCX(propConvexVolume_subdivisions, "volume.subdivisions", sim_propertytype_intarray, 0,  jsonStr({{"label", "Subdivisions"}, {"description", "Number of subdivisions (near and far) for cone-type volumes"}}), "") \
    FUNCX(propConvexVolume_edges, "volume.edges", sim_propertytype_floatarray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Volume edges"}, {"description", "List of segments (defined by pairs of end-point coordinates) visualizing the volume"}}), "") \
    FUNCX(propConvexVolume_closeEdges, "volume.closeEdges", sim_propertytype_floatarray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Volume close edges"}, {"description", "List of segments (defined by pairs of end-point coordinates) visualizing the close threshold of the volume"}}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propConvexVolume_DEPRECATED_offset, "volume_offset", sim_propertytype_float, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_range, "volume_range", sim_propertytype_float, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_xSize, "volume_xSize", sim_propertytype_floatarray, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_ySize, "volume_ySize", sim_propertytype_floatarray, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_radius, "volume_radius", sim_propertytype_floatarray, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_angle, "volume_angle", sim_propertytype_floatarray, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_faces, "volume_faces", sim_propertytype_intarray, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_subdivisions, "volume_subdivisions", sim_propertytype_intarray, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_edges, "volume_edges", sim_propertytype_floatarray, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude | sim_propertyinfo_notwritable,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_closeEdges, "volume_closeEdges", sim_propertytype_floatarray, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude | sim_propertyinfo_notwritable,  "", "")

#define DYNCONT_PROPERTIES \
    FUNCX(propDynCont_dynamicsEnabled, "dynamicsEnabled", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dynamics enabled"}, {"description", ""}}), "") \
    FUNCX(propDynCont_showContactPoints, "showContactPoints", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Show contact points"}, {"description", ""}}), "") \
    FUNCX(propDynCont_dynamicsEngine, "dynamicsEngine", sim_propertytype_intarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dynamics engine"}, {"description", "Selected dynamics engine index and version"}}), "") \
    FUNCX(propDynCont_dynamicsStepSize, "dynamicsStepSize", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dynamics dt"}, {"description", "Dynamics step size"}}), "") \
    FUNCX(propDynCont_gravity, "gravity", sim_propertytype_vector3, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Gravity"}, {"description", ""}}), "") \
    FUNCX(propDynCont_engineProperties, "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Engine properties"}, {"description", "Engine properties as JSON text"}}), "") \
    FUNCX(propDynCont_bulletSolver, "bullet.solver", sim_propertytype_int, 0, sim_bullet_global_constraintsolvertype, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_bulletIterations, "bullet.iterations", sim_propertytype_int, 0, sim_bullet_global_constraintsolvingiterations, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_bulletComputeInertias, "bullet.computeInertias", sim_propertytype_bool, 0, sim_bullet_global_computeinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_bulletInternalScalingFull, "bullet.internalScalingFull", sim_propertytype_bool, 0, sim_bullet_global_fullinternalscaling, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_bulletInternalScalingScaling, "bullet.internalScalingValue", sim_propertytype_float, 0, sim_bullet_global_internalscalingfactor, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_bulletCollMarginScaling, "bullet.collisionMarginScaling", sim_propertytype_float, 0, sim_bullet_global_collisionmarginfactor, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_odeQuickStepEnabled, "ode.quickStepEnabled", sim_propertytype_bool, 0, sim_ode_global_quickstep, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_odeQuickStepIterations, "ode.quickStepIterations", sim_propertytype_int, 0, sim_ode_global_constraintsolvingiterations, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_odeComputeInertias, "ode.computeInertias", sim_propertytype_bool, 0, sim_ode_global_computeinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_odeInternalScalingFull, "ode.internalScalingFull", sim_propertytype_bool, 0, sim_ode_global_fullinternalscaling, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_odeInternalScalingScaling, "ode.internalScalingValue", sim_propertytype_float, 0, sim_ode_global_internalscalingfactor, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_odeGlobalErp, "ode.globalErp", sim_propertytype_float, 0, sim_ode_global_erp, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_odeGlobalCfm, "ode.globalCfm", sim_propertytype_float, 0, sim_ode_global_cfm, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexComputeInertias, "vortex.computeInertias", sim_propertytype_bool, 0, sim_vortex_global_computeinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexContactTolerance, "vortex.contactTolerance", sim_propertytype_float, 0, sim_vortex_global_contacttolerance, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexAutoSleep, "vortex.autoSleep", sim_propertytype_bool, 0, sim_vortex_global_autosleep, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexMultithreading, "vortex.multithreading", sim_propertytype_bool, 0, sim_vortex_global_multithreading, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexConstraintsLinearCompliance, "vortex.constraintsLinearCompliance", sim_propertytype_float, 0, sim_vortex_global_constraintlinearcompliance, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexConstraintsLinearDamping, "vortex.constraintsLinearDamping", sim_propertytype_float, 0, sim_vortex_global_constraintlineardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexConstraintsLinearKineticLoss, "vortex.constraintsLinearKineticLoss", sim_propertytype_float, 0, sim_vortex_global_constraintlinearkineticloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexConstraintsAngularCompliance, "vortex.constraintsAngularCompliance", sim_propertytype_float, 0, sim_vortex_global_constraintangularcompliance, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexConstraintsAngularDamping, "vortex.constraintsAngularDamping", sim_propertytype_float, 0, sim_vortex_global_constraintangulardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexConstraintsAngularKineticLoss, "vortex.constraintsAngularKineticLoss", sim_propertytype_float, 0, sim_vortex_global_constraintangularkineticloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_newtonIterations, "newton.iterations", sim_propertytype_int, 0, sim_newton_global_constraintsolvingiterations, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_newtonComputeInertias, "newton.computeInertias", sim_propertytype_bool, 0, sim_newton_global_computeinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_newtonMultithreading, "newton.multithreading", sim_propertytype_bool, 0, sim_newton_global_multithreading, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_newtonExactSolver, "newton.exactSolver", sim_propertytype_bool, 0, sim_newton_global_exactsolver, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_newtonHighJointAccuracy, "newton.highJointAccuracy", sim_propertytype_bool, 0, sim_newton_global_highjointaccuracy, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_newtonContactMergeTolerance, "newton.contactMergeTolerance", sim_propertytype_float, 0, sim_newton_global_contactmergetolerance, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoIntegrator, "mujoco.integrator", sim_propertytype_int, 0, sim_mujoco_global_integrator, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoSolver, "mujoco.solver", sim_propertytype_int, 0, sim_mujoco_global_solver, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoIterations, "mujoco.iterations", sim_propertytype_int, 0, sim_mujoco_global_iterations, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoRebuildTrigger, "mujoco.rebuildTrigger", sim_propertytype_int, 0, sim_mujoco_global_rebuildtrigger, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoComputeInertias, "mujoco.computeInertias", sim_propertytype_bool, 0, sim_mujoco_global_computeinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoMbMemory, "mujoco.mbmemory", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoNjMax, "mujoco.njmax", sim_propertytype_int, 0, sim_mujoco_global_njmax, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", "Deprecated. See mujoco.mbmemory instead."}}), "") \
    FUNCX(propDynCont_mujocoNconMax, "mujoco.nconmax", sim_propertytype_int, 0, sim_mujoco_global_nconmax, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", "Deprecated. See mujoco.mbmemory instead."}}), "") \
    FUNCX(propDynCont_mujocoNstack, "mujoco.nstack", sim_propertytype_int, 0, sim_mujoco_global_nstack, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", "Deprecated. See mujoco.mbmemory instead."}}), "") \
    FUNCX(propDynCont_mujocoCone, "mujoco.cone", sim_propertytype_int, 0, sim_mujoco_global_cone, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoKinematicBodiesOverrideFlags, "mujoco.kinematicBodiesOverrideFlags", sim_propertytype_int, 0, sim_mujoco_global_overridekin, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoKinematicBodiesMass, "mujoco.kinematicBodiesMass", sim_propertytype_float, 0, sim_mujoco_global_kinmass, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoKinematicBodiesInertia, "mujoco.kinematicBodiesInertia", sim_propertytype_float, 0, sim_mujoco_global_kininertia, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoBoundMass, "mujoco.boundMass", sim_propertytype_float, 0, sim_mujoco_global_boundmass, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoBoundInertia, "mujoco.boundInertia", sim_propertytype_float, 0, sim_mujoco_global_boundinertia, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoBalanceInertias, "mujoco.balanceInertias", sim_propertytype_bool, 0, sim_mujoco_global_balanceinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoMultithreaded, "mujoco.multithreaded", sim_propertytype_bool, 0, sim_mujoco_global_multithreaded, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoMulticcd, "mujoco.multiccd", sim_propertytype_bool, 0, sim_mujoco_global_multiccd, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoContactParamsOverride, "mujoco.contactParamsOverride", sim_propertytype_bool, 0, sim_mujoco_global_overridecontacts, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoContactParamsMargin, "mujoco.contactParamsMargin", sim_propertytype_float, 0, sim_mujoco_global_overridemargin, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoContactParamsSolref, "mujoco.contactParamsSolref", sim_propertytype_floatarray, 0, sim_mujoco_global_overridesolref1, sim_mujoco_global_overridesolref2, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoContactParamsSolimp, "mujoco.contactParamsSolimp", sim_propertytype_floatarray, 0, sim_mujoco_global_overridesolimp1, sim_mujoco_global_overridesolimp2, sim_mujoco_global_overridesolimp3, sim_mujoco_global_overridesolimp4, sim_mujoco_global_overridesolimp5,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoImpRatio, "mujoco.impratio", sim_propertytype_float, 0, sim_mujoco_global_impratio, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoWind, "mujoco.wind", sim_propertytype_vector3, 0, sim_mujoco_global_wind1, sim_mujoco_global_wind2, sim_mujoco_global_wind3, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoDensity, "mujoco.density", sim_propertytype_float, 0, sim_mujoco_global_density, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoViscosity, "mujoco.viscosity", sim_propertytype_float, 0, sim_mujoco_global_viscosity, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoJacobian, "mujoco.jacobian", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoTolerance, "mujoco.tolerance", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoLs_iterations, "mujoco.ls_iterations", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoLs_tolerance, "mujoco.ls_tolerance", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoNoslip_iterations, "mujoco.noslip_iterations", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoNoslip_tolerance, "mujoco.noslip_tolerance", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoCcd_iterations, "mujoco.ccd_iterations", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoCcd_tolerance, "mujoco.ccd_tolerance", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoSdf_iterations, "mujoco.sdf_iterations", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoSdf_initpoints, "mujoco.sdf_initpoints", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoEqualityEnable, "mujoco.equalityEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoFrictionlossEnable, "mujoco.frictionlossEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoLimitEnable, "mujoco.limitEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoContactEnable, "mujoco.contactEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoPassiveEnable, "mujoco.passiveEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoGravityEnable, "mujoco.gravityEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoWarmstartEnable, "mujoco.warmstartEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoActuationEnable, "mujoco.actuationEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoRefsafeEnable, "mujoco.refsafeEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoSensorEnable, "mujoco.sensorEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoMidphaseEnable, "mujoco.midphaseEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoEulerdampEnable, "mujoco.eulerdampEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoAutoresetEnable, "mujoco.autoresetEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoEnergyEnable, "mujoco.energyEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoInvdiscreteEnable, "mujoco.invdiscreteEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoNativeccdEnable, "mujoco.nativeccdEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoAlignfree, "mujoco.alignfree", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoKinematicWeldSolref, "mujoco.kinematicWeldSolref", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoKinematicWeldSolimp, "mujoco.kinematicWeldSolimp", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoKinematicWeldTorqueScale, "mujoco.kinematicWeldTorquescale", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "")

#define SCENE_PROPERTIES \
    FUNCX(propScene_mainScript, "mainScript", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Main script"}, {"description", "Handle of the main script"}, {"handleType", "detachedScript"}}), "") \
    FUNCX(propScene_finalSaveRequest, "finalSaveRequest", sim_propertytype_bool, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Final save"}, {"description", "Lock scene and models after next scene save operation"}}), "") \
    FUNCX(propScene_saveCalculationStructs, "saveCalculationStructs", sim_propertytype_bool, 0,  jsonStr({{"label", "Save calculation structures"}, {"description", "Save operation also saves existing calculation structures"}}), "") \
    FUNCX(propScene_visibilityLayers, "visibilityLayers", sim_propertytype_int, 0,  jsonStr({{"label", "Visibility layers"}, {"description", "Currently active visibility layers"}}), "") \
    FUNCX(propScene_sceneIsLocked, "locked", sim_propertytype_bool, sim_propertyinfo_notwritable,  jsonStr({{"label", "Scene is locked"}, {"description", ""}}), "") \
    FUNCX(propScene_sceneUid, "uid", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Scene UID"}, {"description", "Scene unique identifier"}}), "") \
    FUNCX(propScene_sceneUidString, "uidString", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Scene UID string"}, {"description", "Scene unique identifier string"}}), "") \
    FUNCX(propScene_scenePath, "path", sim_propertytype_string, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Scene path"}, {"description", ""}}), "") \
    FUNCX(propScene_acknowledgment, "acknowledgment", sim_propertytype_string, 0,  jsonStr({{"label", "Acknowledgment"}, {"description", "Scene acknowledgment"}}), "") \
    FUNCX(propScene_ambientLight, "ambientLight", sim_propertytype_color, 0,  jsonStr({{"label", "Ambient light"}, {"description", ""}}), "") \
    FUNCX(propScene_customObjects, "customObjects", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Custom objects"}, {"description", "List of scene custom objects"}, {"handleType", "customObject"}}), "") \
    FUNCX(propScene_METHOD_createObject, "createObject", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_removeObjects, "removeObjects", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_duplicateObjects, "duplicateObjects", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_handleSimulationScripts, "handleSimulationScripts", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_handleCustomizationScripts, "handleCustomizationScripts", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_getDescendants, "getDescendants", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_getObjects, "getObjects", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_loadModel, "loadModel", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_loadModelFromBuffer, "loadModelFromBuffer", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_save, "save", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_saveToBuffer, "saveToBuffer", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_getObject, "getObject", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_remove, "remove", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_announceChange, "announceChange", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_getObjectFromUid, "getObjectFromUid", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_startSimulation, "startSimulation", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_pauseSimulation, "pauseSimulation", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_stopSimulation, "stopSimulation", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_groupShapes, "groupShapes", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_mergeShapes, "mergeShapes", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propScene_DEPRECATED_sceneIsLocked, "sceneIsLocked", sim_propertytype_bool, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude | sim_propertyinfo_notwritable,  "", "") \
    FUNCX(propScene_DEPRECATED_sceneUid, "sceneUid", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  "", "") \
    FUNCX(propScene_DEPRECATED_sceneUidString, "sceneUidString", sim_propertytype_string,sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude | sim_propertyinfo_constant | sim_propertyinfo_notwritable,  "", "") \
    FUNCX(propScene_DEPRECATED_scenePath, "scenePath", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude,  "", "")

#define SIMULATION_PROPERTIES \
    FUNCX(propSimulation_removeNewObjectsAtEnd, "removeNewObjectsAtEnd", sim_propertytype_bool, 0,  jsonStr({{"label", "Remove new objects"}, {"description", "Remove new scene objects at simulation end"}}), "") \
    FUNCX(propSimulation_realtimeSimulation, "realtimeSimulation", sim_propertytype_bool, 0,  jsonStr({{"label", "Real-time simulation"}, {"description", ""}}), "") \
    FUNCX(propSimulation_pauseSimulationAtTime, "pauseSimulationAtTime", sim_propertytype_bool, 0,  jsonStr({{"label", "Pause simulation at time"}, {"description", "Pause simulation when simulation time exceeds a threshold"}}), "") \
    FUNCX(propSimulation_pauseSimulationAtError, "pauseSimulationAtError", sim_propertytype_bool, 0,  jsonStr({{"label", "Pause simulation on script error"}, {"description", ""}}), "") \
    FUNCX(propSimulation_simulationTime, "simulationTime", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Simulation time"}, {"description", ""}}), "") \
    FUNCX(propSimulation_timeStep, "timeStep", sim_propertytype_float, 0,  jsonStr({{"label", "Simulation dt"}, {"description", "Simulation time step"}}), "") \
    FUNCX(propSimulation_timeToPause, "timeToPause", sim_propertytype_float, 0,  jsonStr({{"label", "Simulation pause time"}, {"description", "Time at which simulation should pause"}}), "") \
    FUNCX(propSimulation_realtimeModifier, "realtimeModifier", sim_propertytype_float, 0,  jsonStr({{"label", "Real-time modifier"}, {"description", "Real-time multiplication factor"}}), "") \
    FUNCX(propSimulation_stepCount, "stepCount", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Simulation steps"}, {"description", "Counter of simulation steps"}}), "") \
    FUNCX(propSimulation_simulationState, "simulationState", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Simulation state"}, {"description", ""}}), "") \
    FUNCX(propSimulation_stepsPerRendering, "stepsPerRendering", sim_propertytype_int, 0,  jsonStr({{"label", "Steps per frame"}, {"description", "Simulation steps per frame"}}), "") \
    FUNCX(propSimulation_speedModifier, "speedModifier", sim_propertytype_int, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Speed modifier"}, {"description", ""}}), "")

#define MESHWRAPPER_PROPERTIES \
    FUNCX(propMeshWrapper_mass, "mass", sim_propertytype_float, 0,  jsonStr({{"label", "Mass"}, {"description", ""}}), "") \
    FUNCX(propMeshWrapper_com, "centerOfMass", sim_propertytype_vector3, 0,  jsonStr({{"label", "Center of mass"}, {"description", "Center of mass, relative to the shape's reference frame"}}), "") \
    FUNCX(propMeshWrapper_inertiaMatrix, "inertiaMatrix", sim_propertytype_matrix, 0,  jsonStr({{"label", "Inertia matrix"}, {"description", "Inertia matrix, relative to the shape's reference frame"}}), "") \
    FUNCX(propMeshWrapper_pmi, "principalMomentOfInertia", sim_propertytype_floatarray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Principal moment of inertia"}, {"description", "Principal moment of inertia, relative to pmiQuaternion"}}), "") \
    FUNCX(propMeshWrapper_pmiQuaternion, "pmiQuaternion", sim_propertytype_quaternion, sim_propertyinfo_notwritable,  jsonStr({{"label", "Quaternion of the principal moment of inertia"}, {"description", "Quaternion of the principal moment of inertia, relative to the shape's reference frame"}}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propMeshWrapper_DEPRECATED_inertia, "inertia", sim_propertytype_floatarray, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Inertia tensor"}, {"description", "Inertia tensor, relative to the shape's reference frame"}}), "")

#define MESH_PROPERTIES \
    FUNCX(propMesh_textureResolution, "textureResolution", sim_propertytype_intarray2, sim_propertyinfo_notwritable,  jsonStr({{"label", "Texture resolution"}, {"description", ""}}), "") \
    FUNCX(propMesh_textureCoordinates, "textureCoordinates", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Texture coordinates"}, {"description", ""}}), "") \
    FUNCX(propMesh_textureApplyMode, "textureApplyMode", sim_propertytype_int, 0,  jsonStr({{"label", "Texture apply mode"}, {"description", ""}}), "") \
    FUNCX(propMesh_textureRepeatU, "textureRepeatU", sim_propertytype_bool, 0,  jsonStr({{"label", "Texture repeat U"}, {"description", ""}}), "") \
    FUNCX(propMesh_textureRepeatV, "textureRepeatV", sim_propertytype_bool, 0,  jsonStr({{"label", "Texture repeat V"}, {"description", ""}}), "") \
    FUNCX(propMesh_textureInterpolate, "textureInterpolate", sim_propertytype_bool, 0,  jsonStr({{"label", "Interpolate texture"}, {"description", ""}}), "") \
    FUNCX(propMesh_texture, "rawTexture", sim_propertytype_buffer, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Texture"}, {"description", ""}}), "") \
    FUNCX(propMesh_textureID, "textureID", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Texture ID"}, {"description", ""}}), "") \
    FUNCX(propMesh_vertices, "vertices", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Vertices"}, {"description", ""}}), "") \
    FUNCX(propMesh_indices, "indices", sim_propertytype_intarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Indices"}, {"description", "Indices (3 values per triangle)"}}), "") \
    FUNCX(propMesh_normals, "normals", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Normals"}, {"description", "Normals (3*3 values per triangle)"}}), "") \
    FUNCX(propMesh_shadingAngle, "shadingAngle", sim_propertytype_float, 0,  jsonStr({{"label", "Shading angle"}, {"description", ""}}), "") \
    FUNCX(propMesh_showEdges, "showEdges", sim_propertytype_bool, 0,  jsonStr({{"label", "Visible edges"}, {"description", ""}}), "") \
    FUNCX(propMesh_culling, "culling", sim_propertytype_bool, 0,  jsonStr({{"label", "Backface culling"}, {"description", ""}}), "") \
    FUNCX(propMesh_wireframe, "wireframe", sim_propertytype_bool, 0,  jsonStr({{"label", "Wireframe"}, {"description", ""}}), "") \
    FUNCX(propMesh_shapeUid, "shapeUid", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Shape UID"}, {"description", "Unique identifier of parent shape"}}), "") \
    FUNCX(propMesh_shape, "shape", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Shape handle"}, {"description", ""}, {"handleType", "shape"}}), "") \
    FUNCX(propMesh_primitiveType, "primitiveType", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Primitive type"}, {"description", ""}}), "") \
    FUNCX(propMesh_convex, "convex", sim_propertytype_bool, sim_propertyinfo_notwritable,  jsonStr({{"label", "Convex"}, {"description", "Whether mesh is convex or not"}}), "") \
    FUNCX(propMesh_colorName, "colorName", sim_propertytype_string, 0,  jsonStr({{"label", "Color name"}, {"description", ""}}), "")

#define VIEWABLEBASE_PROPERTIES \
    FUNCX(propViewableBase_viewAngle, "viewAngle", sim_propertytype_float, 0,  jsonStr({{"label", "View angle"}, {"description", "View angle (in perspective projection mode)"}}), "") \
    FUNCX(propViewableBase_viewSize, "viewSize", sim_propertytype_float, 0,  jsonStr({{"label", "View size"}, {"description", "View size (in orthogonal projection mode)"}}), "") \
    FUNCX(propViewableBase_clippingPlanes, "clippingPlanes", sim_propertytype_floatarray, 0,  jsonStr({{"label", "Clipping planes"}, {"description", "Near and far clipping planes"}}), "") \
    FUNCX(propViewableBase_perspective, "perspective", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Perspective"}, {"description", "Perspective projection mode, otherwise orthogonal projection mode"}}), "") \
    FUNCX(propViewableBase_showFrustum, "showFrustum", sim_propertytype_bool, 0,  jsonStr({{"label", "Show view frustum"}, {"description", ""}}), "") \
    FUNCX(propViewableBase_frustumCornerNear, "frustumCornerNear", sim_propertytype_vector3, sim_propertyinfo_notwritable,  jsonStr({{"label", "Near corner of View frustum"}, {"description", ""}}), "") \
    FUNCX(propViewableBase_frustumCornerFar, "frustumCornerFar", sim_propertytype_vector3, sim_propertyinfo_notwritable,  jsonStr({{"label", "Far corner of view frustum"}, {"description", ""}}), "") \
    FUNCX(propViewableBase_resolution, "resolution", sim_propertytype_intarray2, 0,  jsonStr({{"label", "Resolution"}, {"description", "Resolution (relevant only with vision sensors)"}}), "")

#define SCENEOBJECTCONT_PROPERTIES \
    FUNCX(propSceneObjectCont_objects, "objects", sim_propertytype_handlearray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Objects"}, {"description", "Handles of all scene objects"}, {"handleType", "sceneObject"}}), "") \
    FUNCX(propSceneObjectCont_orphans, "orphans", sim_propertytype_handlearray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Orphan objects"}, {"description", "Handles of all orphan scene objects"}, {"handleType", "sceneObject"}}), "") \
    FUNCX(propSceneObjectCont_selection, "selection", sim_propertytype_handlearray, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Selected objects"}, {"description", "Handles of selected scene objects"}, {"handleType", "sceneObject"}}), "") \
    FUNCX(propSceneObjectCont_objectCreationCounter, "objectCreationCounter", sim_propertytype_int, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Object creation counter"}, {"description", "Counter of created scene objects"}}), "") \
    FUNCX(propSceneObjectCont_objectDestructionCounter, "objectDestructionCounter", sim_propertytype_int, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Object destruction counter"}, {"description", "Counter of destroyed scene objects"}}), "") \
    FUNCX(propSceneObjectCont_hierarchyChangeCounter, "hierarchyChangeCounter", sim_propertytype_int, sim_propertyinfo_silent | sim_propertyinfo_notwritable,  jsonStr({{"label", "Hierarchy change counter"}, {"description", "Counter of scene hierarchy changes"}}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propSceneObjectCont_DEPRECATED_objectHandles, "objectHandles", sim_propertytype_intarray, sim_propertyinfo_deprecated | sim_propertyinfo_notwritable,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propSceneObjectCont_DEPRECATED_orphanHandles, "orphanHandles", sim_propertytype_intarray, sim_propertyinfo_deprecated | sim_propertyinfo_notwritable,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propSceneObjectCont_DEPRECATED_selectionHandles, "selectionHandles", sim_propertytype_intarray, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "")

#define DYNMATERIAL_PROPERTIES \
    FUNCX(propMaterial_engineProperties, "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Engine properties"}, {"description", "Engine properties as JSON text"}}), "") \
    FUNCX(propMaterial_bulletRestitution, "bullet.restitution", sim_propertytype_float, 0, sim_bullet_body_restitution, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletFriction0, "bullet.frictionOld", sim_propertytype_float, 0, sim_bullet_body_oldfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletFriction, "bullet.friction", sim_propertytype_float, 0, sim_bullet_body_friction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletLinearDamping, "bullet.linearDamping", sim_propertytype_float, 0, sim_bullet_body_lineardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletAngularDamping, "bullet.angularDamping", sim_propertytype_float, 0, sim_bullet_body_angulardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletNonDefaultCollisionMarginFactor, "bullet.customCollisionMarginValue", sim_propertytype_float, 0, sim_bullet_body_nondefaultcollisionmargingfactor, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletNonDefaultCollisionMarginFactorConvex, "bullet.customCollisionMarginConvexValue", sim_propertytype_float, 0, sim_bullet_body_nondefaultcollisionmargingfactorconvex, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletSticky, "bullet.stickyContact", sim_propertytype_bool, 0, sim_bullet_body_sticky, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletNonDefaultCollisionMargin, "bullet.customCollisionMarginEnabled", sim_propertytype_bool, 0, sim_bullet_body_usenondefaultcollisionmargin, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletNonDefaultCollisionMarginConvex, "bullet.customCollisionMarginConvexEnabled", sim_propertytype_bool, 0, sim_bullet_body_usenondefaultcollisionmarginconvex, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletAutoShrinkConvex, "bullet.autoShrinkConvexMeshes", sim_propertytype_bool, 0, sim_bullet_body_autoshrinkconvex, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_odeFriction, "ode.friction", sim_propertytype_float, 0, sim_ode_body_friction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_odeSoftErp, "ode.softErp", sim_propertytype_float, 0, sim_ode_body_softerp, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_odeSoftCfm, "ode.softCfm", sim_propertytype_float, 0, sim_ode_body_softcfm, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_odeLinearDamping, "ode.linearDamping", sim_propertytype_float, 0, sim_ode_body_lineardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_odeAngularDamping, "ode.angularDamping", sim_propertytype_float, 0, sim_ode_body_angulardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_odeMaxContacts, "ode.maxContacts", sim_propertytype_int, 0, sim_ode_body_maxcontacts, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryLinearAxisFriction, "vortex.linearPrimaryAxisFrictionValue", sim_propertytype_float, 0, sim_vortex_body_primlinearaxisfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisFriction, "vortex.linearSecondaryAxisFrictionValue", sim_propertytype_float, 0, sim_vortex_body_seclinearaxisfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryAngularAxisFriction, "vortex.angularPrimaryAxisFrictionValue", sim_propertytype_float, 0, sim_vortex_body_primangularaxisfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisFriction, "vortex.angularSecondaryAxisFrictionValue", sim_propertytype_float, 0, sim_vortex_body_secangularaxisfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexNormalAngularAxisFriction, "vortex.angularNormalAxisFrictionValue", sim_propertytype_float, 0, sim_vortex_body_normalangularaxisfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryLinearAxisStaticFrictionScale, "vortex.linearPrimaryAxisStaticFrictionScale", sim_propertytype_float, 0, sim_vortex_body_primlinearaxisstaticfrictionscale, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisStaticFrictionScale, "vortex.linearSecondaryAxisStaticFrictionScale", sim_propertytype_float, 0, sim_vortex_body_seclinearaxisstaticfrictionscale, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryAngularAxisStaticFrictionScale, "vortex.angularPrimaryAxisStaticFrictionScale", sim_propertytype_float, 0, sim_vortex_body_primangularaxisstaticfrictionscale, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisStaticFrictionScale, "vortex.angularSecondaryAxisStaticFrictionScale", sim_propertytype_float, 0, sim_vortex_body_secangularaxisstaticfrictionscale, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexNormalAngularAxisStaticFrictionScale, "vortex.angularNormalAxisStaticFrictionScale", sim_propertytype_float, 0, sim_vortex_body_normalangularaxisstaticfrictionscale, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexCompliance, "vortex.compliance", sim_propertytype_float, 0, sim_vortex_body_compliance, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexDamping, "vortex.damping", sim_propertytype_float, 0, sim_vortex_body_damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexRestitution, "vortex.restitution", sim_propertytype_float, 0, sim_vortex_body_restitution, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexRestitutionThreshold, "vortex.restitutionThreshold", sim_propertytype_float, 0, sim_vortex_body_restitutionthreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAdhesiveForce, "vortex.adhesiveForce", sim_propertytype_float, 0, sim_vortex_body_adhesiveforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexLinearVelocityDamping, "vortex.linearVelDamping", sim_propertytype_float, 0, sim_vortex_body_linearvelocitydamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAngularVelocityDamping, "vortex.angularVelDamping", sim_propertytype_float, 0, sim_vortex_body_angularvelocitydamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryLinearAxisSlide, "vortex.linearPrimaryAxisSlide", sim_propertytype_float, 0, sim_vortex_body_primlinearaxisslide, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisSlide, "vortex.linearSecondaryAxisSlide", sim_propertytype_float, 0, sim_vortex_body_seclinearaxisslide, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryAngularAxisSlide, "vortex.angularPrimaryAxisSlide", sim_propertytype_float, 0, sim_vortex_body_primangularaxisslide, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisSlide, "vortex.angularSecondaryAxisSlide", sim_propertytype_float, 0, sim_vortex_body_secangularaxisslide, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexNormalAngularAxisSlide, "vortex.angularNormalAxisSlide", sim_propertytype_float, 0, sim_vortex_body_normalangularaxisslide, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryLinearAxisSlip, "vortex.linearPrimaryAxisSlip", sim_propertytype_float, 0, sim_vortex_body_primlinearaxisslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisSlip, "vortex.linearSecondaryAxisSlip", sim_propertytype_float, 0, sim_vortex_body_seclinearaxisslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryAngularAxisSlip, "vortex.angularPrimaryAxisSlip", sim_propertytype_float, 0, sim_vortex_body_primangularaxisslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisSlip, "vortex.angularSecondaryAxisSlip", sim_propertytype_float, 0, sim_vortex_body_secangularaxisslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexNormalAngularAxisSlip, "vortex.angularNormalAxisSlip", sim_propertytype_float, 0, sim_vortex_body_normalangularaxisslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAutoSleepLinearSpeedThreshold, "vortex.autoSleepThresholdLinearSpeed", sim_propertytype_float, 0, sim_vortex_body_autosleeplinearspeedthreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAutoSleepLinearAccelerationThreshold, "vortex.autoSleepThresholdLinearAccel", sim_propertytype_float, 0, sim_vortex_body_autosleeplinearaccelthreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAutoSleepAngularSpeedThreshold, "vortex.autoSleepThresholdAngularSpeed", sim_propertytype_float, 0, sim_vortex_body_autosleepangularspeedthreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAutoSleepAngularAccelerationThreshold, "vortex.autoSleepThresholdAngularAccel", sim_propertytype_float, 0, sim_vortex_body_autosleepangularaccelthreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSkinThickness, "vortex.skinThickness", sim_propertytype_float, 0, sim_vortex_body_skinthickness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAutoAngularDampingTensionRatio, "vortex.autoAngularDampingTensionRatio", sim_propertytype_float, 0, sim_vortex_body_autoangulardampingtensionratio, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryAxisVector, "vortex.linearPrimaryValue", sim_propertytype_vector3, 0, sim_vortex_body_primaxisvectorx, sim_vortex_body_primaxisvectory, sim_vortex_body_primaxisvectorz, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryLinearAxisFrictionModel, "vortex.linearPrimaryAxisFrictionModel", sim_propertytype_int, 0, sim_vortex_body_primlinearaxisfrictionmodel, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisFrictionModel, "vortex.linearSecondaryAxisFrictionModel", sim_propertytype_int, 0, sim_vortex_body_seclinearaxisfrictionmodel, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryAngularAxisFrictionModel, "vortex.angularPrimaryAxisFrictionModel", sim_propertytype_int, 0, sim_vortex_body_primangulararaxisfrictionmodel, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisFrictionModel, "vortex.angularSecondaryAxisFrictionModel", sim_propertytype_int, 0, sim_vortex_body_secangularaxisfrictionmodel, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexNormalAngularAxisFrictionModel, "vortex.angularNormalAxisFrictionModel", sim_propertytype_int, 0, sim_vortex_body_normalangularaxisfrictionmodel, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAutoSleepStepLiveThreshold, "vortex.autoSleepThresholdSteps", sim_propertytype_int, 0, sim_vortex_body_autosleepsteplivethreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexMaterialUniqueId, "vortex.materialUniqueId", sim_propertytype_int, sim_propertyinfo_modelhashexclude, sim_vortex_body_materialuniqueid, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimitiveShapesAsConvex, "vortex.primitiveAsConvex", sim_propertytype_bool, 0, sim_vortex_body_pureshapesasconvex, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexConvexShapesAsRandom, "vortex.convexAsRandom", sim_propertytype_bool, 0, sim_vortex_body_convexshapesasrandom, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexRandomShapesAsTerrain, "vortex.randomAsTerrain", sim_propertytype_bool, 0, sim_vortex_body_randomshapesasterrain, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexFastMoving, "vortex.fastMoving", sim_propertytype_bool, 0, sim_vortex_body_fastmoving, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAutoSlip, "vortex.autoSlip", sim_propertytype_bool, 0, sim_vortex_body_autoslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisSameAsPrimaryLinearAxis, "vortex.linearSecondaryAxisFollowPrimaryAxis", sim_propertytype_bool, 0, sim_vortex_body_seclinaxissameasprimlinaxis, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisSameAsPrimaryAngularAxis, "vortex.angularSecondaryAxisFollowPrimaryAxis", sim_propertytype_bool, 0, sim_vortex_body_secangaxissameasprimangaxis, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexNormalAngularAxisSameAsPrimaryAngularAxis, "vortex.angularNormalAxisFollowPrimaryAxis", sim_propertytype_bool, 0, sim_vortex_body_normangaxissameasprimangaxis, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAutoAngularDamping, "vortex.autoAngularDampingEnabled", sim_propertytype_bool, 0, sim_vortex_body_autoangulardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_newtonStaticFriction, "newton.staticFriction", sim_propertytype_float, 0, sim_newton_body_staticfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_newtonKineticFriction, "newton.kineticFriction", sim_propertytype_float, 0, sim_newton_body_kineticfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_newtonRestitution, "newton.restitution", sim_propertytype_float, 0, sim_newton_body_restitution, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_newtonLinearDrag, "newton.linearDrag", sim_propertytype_float, 0, sim_newton_body_lineardrag, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_newtonAngularDrag, "newton.angularDrag", sim_propertytype_float, 0, sim_newton_body_angulardrag, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_newtonFastMoving, "newton.fastMoving", sim_propertytype_bool, 0, sim_newton_body_fastmoving, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoFriction, "mujoco.friction", sim_propertytype_floatarray, 0, sim_mujoco_body_friction1, sim_mujoco_body_friction2, sim_mujoco_body_friction3, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoSolref, "mujoco.solref", sim_propertytype_floatarray, 0, sim_mujoco_body_solref1, sim_mujoco_body_solref2, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoSolimp, "mujoco.solimp", sim_propertytype_floatarray, 0, sim_mujoco_body_solimp1, sim_mujoco_body_solimp2, sim_mujoco_body_solimp3, sim_mujoco_body_solimp4, sim_mujoco_body_solimp5,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoSolmix, "mujoco.solmix", sim_propertytype_float, 0, sim_mujoco_body_solmix, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoMargin, "mujoco.margin", sim_propertytype_float, 0, sim_mujoco_body_margin, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoGap, "mujoco.gap", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoCondim, "mujoco.condim", sim_propertytype_int, 0, sim_mujoco_body_condim, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoPriority, "mujoco.priority", sim_propertytype_int, 0, sim_mujoco_body_priority, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoAdhesion, "mujoco.adhesion", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoAdhesionGain, "mujoco.adhesiongain", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoAdhesionForcelimited, "mujoco.adhesionforcelimited", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoAdhesionCtrlrange, "mujoco.adhesionctrlrange", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoAdhesionForcerange, "mujoco.adhesionforcerange", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoAdhesionCtrl, "mujoco.adhesionctrl", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoGravcomp, "mujoco.gravcomp", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "")

#define CUSTOMSCENEOBJECTCLASS_PROPERTIES \
    FUNCX(propCustomSceneObjectClass_METHOD_makeOject, "makeObject", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propCustomSceneObjectClass_METHOD_remove, "remove", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "")

#define SCENEOBJECT_PROPERTIES \
    FUNCX(propSceneObject_modelInvisible, "modelInvisible", sim_propertytype_bool, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Inherited model invisibility"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_modelBase, "modelBase", sim_propertytype_bool, 0,  jsonStr({{"label", "Model base"}, {"description", "Model base flag, indicates the scene object is the base of a model"}}), "") \
    FUNCX(propSceneObject_layer, "layer", sim_propertytype_int, 0,  jsonStr({{"label", "Visibility layer"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_childOrder, "childOrder", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Child order"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_parentUid, "parentUid", sim_propertytype_long, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Parent UID"}, {"description", "Parent scene object unique identifier"}}), "") \
    FUNCX(propSceneObject_objectUid, "objectUid", sim_propertytype_long, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Object UID"}, {"description", "Scene object unique identifier"}}), "") \
    FUNCX(propSceneObject_parent, "parent", sim_propertytype_handle, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Parent"}, {"description", "Parent scene object handle"}, {"handleType", "sceneObject"}}), "") \
    FUNCX(propSceneObject_selected, "selected", sim_propertytype_bool, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Selected"}, {"description", "Selection state"}}), "") \
    FUNCX(propSceneObject_hierarchyColor, "hierarchyColor", sim_propertytype_int, 0,  jsonStr({{"label", "Hierarchy color"}, {"description", "Hierarchy color index"}}), "") \
    FUNCX(propSceneObject_collectionSelfCollInd, "collectionSelfCollisionIndicator", sim_propertytype_int, 0,  jsonStr({{"label", "Collection self collision indicator"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_collidable, "collidable", sim_propertytype_bool, 0,  jsonStr({{"label", "Collidable"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_measurable, "measurable", sim_propertytype_bool, 0,  jsonStr({{"label", "Measurable"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_detectable, "detectable", sim_propertytype_bool, 0,  jsonStr({{"label", "Detectable"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_modelAcknowledgment, "modelAcknowledgment", sim_propertytype_string, 0,  jsonStr({{"label", "Acknowledgment"}, {"description", "Model acknowledgment"}}), "") \
    FUNCX(propSceneObject_dna, "dna", sim_propertytype_buffer, sim_propertyinfo_notwritable,  jsonStr({{"label", "DNA"}, {"description", "Scene object DNA"}}), "") \
    FUNCX(propSceneObject_persistentUid, "persistentUid", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Persistent UID"}, {"description", "Scene object persistent unique identifier"}}), "") \
    FUNCX(propSceneObject_calcLinearVelocity, "calcLinearVelocity", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Linear velocity"}, {"description", "Calculated scene object linear velocity vector"}}), "") \
    FUNCX(propSceneObject_calcRotationAxis, "calcRotationAxis", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Rotation axis"}, {"description", "Calculated scene object rotation axis"}}), "") \
    FUNCX(propSceneObject_calcRotationVelocity, "calcRotationVelocity", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Rotation velocity"}, {"description", "Calculated scene object rotation velocity"}}), "") \
    FUNCX(propSceneObject_dynamicIcon, "dynamicIcon", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Dynamic icon"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_dynamicFlag, "dynamicFlag", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Dynamic flag"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_objectProperty, "objectPropertyFlags", /*redund.*/ sim_propertytype_int, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Object flags"}, {"description", "Scene object flags, redundant"}}), "") \
    FUNCX(propSceneObject_ignoreViewFitting, "ignoreViewFitting", sim_propertytype_bool, 0,  jsonStr({{"label", "Ignore view fitting"}, {"description", "View fitting is ignored for this scene object"}}), "") \
    FUNCX(propSceneObject_collapsed, "collapsed", sim_propertytype_bool, 0,  jsonStr({{"label", "Collapsed hierarchy"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_selectable, "selectable", sim_propertytype_bool, 0,  jsonStr({{"label", "Selectable"}, {"description", "Scene object is selectable"}}), "") \
    FUNCX(propSceneObject_selectModel, "selectModel", sim_propertytype_bool, 0,  jsonStr({{"label", "Select model instead"}, {"description", "Selecting the scene object instead selects the parent model"}}), "") \
    FUNCX(propSceneObject_hideFromModelBB, "hideFromModelBB", sim_propertytype_bool, 0,  jsonStr({{"label", "Hide from model bounding box"}, {"description", "Scene object is not part of a parent model's bounding box"}}), "") \
    FUNCX(propSceneObject_selectInvisible, "selectInvisible", sim_propertytype_bool, 0,  jsonStr({{"label", "Select invisible"}, {"description", "Scene object is invisible during a selection operation"}}), "") \
    FUNCX(propSceneObject_depthInvisible, "depthInvisible", sim_propertytype_bool, 0,  jsonStr({{"label", "Depth pass invisible"}, {"description", "Scene object is invisible for the depth buffer during a click operation"}}), "") \
    FUNCX(propSceneObject_cannotDelete, "cannotDelete", sim_propertytype_bool, 0,  jsonStr({{"label", "Cannot delete"}, {"description", "Scene object cannot be deleted while simulation is not running"}}), "") \
    FUNCX(propSceneObject_cannotDeleteSim, "cannotDeleteInSim", sim_propertytype_bool, 0,  jsonStr({{"label", "Cannot delete during simulation"}, {"description", "Scene object cannot be deleted while simulation is running"}}), "") \
    FUNCX(propSceneObject_modelProperty, "model.propertyFlags", /*redund.*/ sim_propertytype_int, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Model flags"}, {"description", "Model flags, redundant"}}), "") \
    FUNCX(propSceneObject_modelNotCollidable, "model.notCollidable", sim_propertytype_bool, 0,  jsonStr({{"label", "Model not collidable"}, {"description", "Model is not collidable"}}), "") \
    FUNCX(propSceneObject_modelNotMeasurable, "model.notMeasurable", sim_propertytype_bool, 0,  jsonStr({{"label", "Model not measurable"}, {"description", "Model is not measurable"}}), "") \
    FUNCX(propSceneObject_modelNotDetectable, "model.notDetectable", sim_propertytype_bool, 0,  jsonStr({{"label", "Model not detectable"}, {"description", "Model is not detectable"}}), "") \
    FUNCX(propSceneObject_modelNotDynamic, "model.notDynamic", sim_propertytype_bool, 0,  jsonStr({{"label", "Model not dynamic"}, {"description", "Model is not dynamic, i.e. model is static"}}), "") \
    FUNCX(propSceneObject_modelNotRespondable, "model.notRespondable", sim_propertytype_bool, 0,  jsonStr({{"label", "Model not respondable"}, {"description", "Model is not respondable"}}), "") \
    FUNCX(propSceneObject_modelNotVisible, "model.notVisible", sim_propertytype_bool, 0,  jsonStr({{"label", "Model not visible"}, {"description", "Model is not visible"}}), "") \
    FUNCX(propSceneObject_modelScriptsNotActive, "model.scriptsNotActive", sim_propertytype_bool, 0,  jsonStr({{"label", "Model scripts inactive"}, {"description", "Model scripts are not active"}}), "") \
    FUNCX(propSceneObject_modelNotInParentBB, "model.notInParentBB", sim_propertytype_bool, 0,  jsonStr({{"label", "Model invisible to other model's bounding boxes"}, {"description", "Model is invisible to other model's bounding boxes"}}), "") \
    FUNCX(propSceneObject_modelBBSize, "model.bbSize", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Model bounding boxe size"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_modelBBPos, "model.bbPos", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Model bounding boxe position"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_pose, "pose", sim_propertytype_pose, 0,  jsonStr({{"label", "Pose"}, {"description", "Scene object local pose"}}), "") \
    FUNCX(propSceneObject_position, "position", sim_propertytype_vector3, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Position"}, {"description", "Scene object local position"}}), "") \
    FUNCX(propSceneObject_quaternion, "quaternion", sim_propertytype_quaternion, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Quaternion"}, {"description", "Scene object local quaternion"}}), "") \
    FUNCX(propSceneObject_eulerAngles, "eulerAngles", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Euler angles"}, {"description", "Scene object local Euler angles"}}), "") \
    FUNCX(propSceneObject_absPose, "absPose", sim_propertytype_pose, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Pose"}, {"description", "Scene object absolute pose"}}), "") \
    FUNCX(propSceneObject_absPosition, "absPosition", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Position"}, {"description", "Scene object absolute position"}}), "") \
    FUNCX(propSceneObject_absQuaternion, "absQuaternion", sim_propertytype_quaternion, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Quaternion"}, {"description", "Scene object absolute quaternion"}}), "") \
    FUNCX(propSceneObject_absEulerAngles, "absEulerAngles", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Euler angles"}, {"description", "Scene object absolute Euler angles"}}), "") \
    FUNCX(propSceneObject_name, "name", sim_propertytype_string, 0,  jsonStr({{"label", "Name"}, {"description", "Scene object name"}}), "") \
    FUNCX(propSceneObject_bbPose, "bbPose", sim_propertytype_pose, sim_propertyinfo_notwritable,  jsonStr({{"label", "Bounding box pose"}, {"description", "Bounding box local pose"}}), "") \
    FUNCX(propSceneObject_size, "size", sim_propertytype_vector3, 0,  jsonStr({{"label", "Bounding box size"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_movementOptions, "mov.optionsFlags", sim_propertytype_int, 0,  jsonStr({{"label", "Movement flags"}, {"description", "Scene object movement flags"}}), "") \
    FUNCX(propSceneObject_movementStepSize, "mov.stepSize", sim_propertytype_floatarray, 0,  jsonStr({{"label", "Movement step sizes"}, {"description", "Linear and angular step sizes"}}), "") \
    FUNCX(propSceneObject_movementRelativity, "mov.relativity", sim_propertytype_intarray, 0,  jsonStr({{"label", "Movement relativity"}, {"description", "Movement relativity, linear and angular"}}), "") \
    FUNCX(propSceneObject_movementPreferredAxes, "mov.preferredAxesFlags", sim_propertytype_int, 0,  jsonStr({{"label", "Preferred axes"}, {"description", "Preferred axes flags (redundant)"}}), "") \
    FUNCX(propSceneObject_movTranslNoSim, "mov.translNoSim", sim_propertytype_bool, 0,  jsonStr({{"label", "Translation enabled"}, {"description", "Translation enabled when simulation is not running"}}), "") \
    FUNCX(propSceneObject_movTranslInSim, "mov.translInSim", sim_propertytype_bool, 0,  jsonStr({{"label", "Translation enabled during simulation"}, {"description", "Translation enabled when simulation is running"}}), "") \
    FUNCX(propSceneObject_movRotNoSim, "mov.rotNoSim", sim_propertytype_bool, 0,  jsonStr({{"label", "Rotation enabled"}, {"description", "Rotation enabled when simulation is not running"}}), "") \
    FUNCX(propSceneObject_movRotInSim, "mov.rotInSim", sim_propertytype_bool, 0,  jsonStr({{"label", "Rotation enabled during simulation"}, {"description", "Rotation enabled when simulation is running"}}), "") \
    FUNCX(propSceneObject_movAltTransl, "mov.altTransl", sim_propertytype_bool, 0,  jsonStr({{"label", "Alternate translation axes enabled"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_movAltRot, "mov.altRot", sim_propertytype_bool, 0,  jsonStr({{"label", "Alternate rotation axes enabled"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_movPrefTranslX, "mov.prefTranslX", sim_propertytype_bool, 0,  jsonStr({{"label", "Preferred X-translation"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_movPrefTranslY, "mov.prefTranslY", sim_propertytype_bool, 0,  jsonStr({{"label", "Preferred Y-translation"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_movPrefTranslZ, "mov.prefTranslZ", sim_propertytype_bool, 0,  jsonStr({{"label", "Preferred Z-translation"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_movPrefRotX, "mov.prefRotX", sim_propertytype_bool, 0,  jsonStr({{"label", "Preferred X-axis rotation"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_movPrefRotY, "mov.prefRotY", sim_propertytype_bool, 0,  jsonStr({{"label", "Preferred Y-axis rotation"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_movPrefRotZ, "mov.prefRotZ", sim_propertytype_bool, 0,  jsonStr({{"label", "Preferred Z-axis rotation"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_visible, "visible", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Visible"}, {"description", "Whether the scene object is currently visible"}}), "") \
    FUNCX(propSceneObject_children, "children", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Children handles"}, {"description", ""}, {"handleType", "sceneObject"}}), "") \
    FUNCX(propSceneObject_modelHash, "modelHash", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Model hash"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_METHOD_getAncestors, "getAncestors", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_getDescendants, "getDescendants", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_removeModel, "removeModel", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_remove, "remove", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_saveModel, "saveModel", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_saveModelToBuffer, "saveModelToBuffer", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_checkCollision, "checkCollision", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_checkDistance, "checkDistance", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_getName, "getName", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_getPose, "getPose", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_getPosition, "getPosition", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_getQuaternion, "getQuaternion", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_setPose, "setPose", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_setPosition, "setPosition", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_setQuaternion, "setQuaternion", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_setParent, "setParent", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_scale, "scale", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_scaleTree, "scaleTree", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_dynamicReset, "dynamicReset", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_visitTree, "visitTree", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_changeColor, "changeColor", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_restoreColor, "restoreColor", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_makeClass, "makeClass", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_makeObject, "makeObject", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_getObject, "getObject", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propSceneObject_DEPRECATED_parentHandle, "parentHandle", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propSceneObject_DEPRECATED_modelProperty, "modelPropertyFlags", /*redund.*/ sim_propertytype_int, sim_propertyinfo_modelhashexclude | sim_propertyinfo_deprecated,  jsonStr({{"label", "Model flags"}, {"description", "Model flags, redundant"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_modelNotCollidable, "modelNotCollidable", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Model not collidable"}, {"description", "Model is not collidable"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_modelNotMeasurable, "modelNotMeasurable", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Model not measurable"}, {"description", "Model is not measurable"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_modelNotDetectable, "modelNotDetectable", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Model not detectable"}, {"description", "Model is not detectable"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_modelNotDynamic, "modelNotDynamic", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Model not dynamic"}, {"description", "Model is not dynamic, i.e. model is static"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_modelNotRespondable, "modelNotRespondable", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Model not respondable"}, {"description", "Model is not respondable"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_modelNotVisible, "modelNotVisible", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Model not visible"}, {"description", "Model is not visible"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_modelScriptsNotActive, "modelScriptsNotActive", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Model scripts inactive"}, {"description", "Model scripts are not active"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_modelNotInParentBB, "modelNotInParentBB", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Model invisible to other model's bounding boxes"}, {"description", "Model is invisible to other model's bounding boxes"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_alias, "alias", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Name"}, {"description", "Scene object name"}, {"deprecated-by", "name"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_deprecatedName, "deprecatedName", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Deprecated name"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_DEPRECATED_bbHsize, "bbHSize", sim_propertytype_vector3, sim_propertyinfo_deprecated | sim_propertyinfo_notwritable,  jsonStr({{"label", "Bounding box half sizes"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_DEPRECATED_movementOptions, "movementOptionsFlags", sim_propertytype_int, sim_propertyinfo_deprecated,  jsonStr({{"label", "Movement flags"}, {"description", "Scene object movement flags"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_movementStepSize, "movementStepSize", sim_propertytype_floatarray, sim_propertyinfo_deprecated,  jsonStr({{"label", "Movement step sizes"}, {"description", "Linear and angular step sizes"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_movementRelativity, "movementRelativity", sim_propertytype_intarray, sim_propertyinfo_deprecated,  jsonStr({{"label", "Movement relativity"}, {"description", "Movement relativity, linear and angular"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_movementPreferredAxes, "movementPreferredAxesFlags", sim_propertytype_int, sim_propertyinfo_deprecated,  jsonStr({{"label", "Preferred axes"}, {"description", "Preferred axes flags (redundant)"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_movTranslNoSim, "movTranslNoSim", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Translation enabled"}, {"description", "Translation enabled when simulation is not running"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_movTranslInSim, "movTranslInSim", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Translation enabled during simulation"}, {"description", "Translation enabled when simulation is running"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_movRotNoSim, "movRotNoSim", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Rotation enabled"}, {"description", "Rotation enabled when simulation is not running"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_movRotInSim, "movRotInSim", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Rotation enabled during simulation"}, {"description", "Rotation enabled when simulation is running"}}), "") \
    FUNCX(propSceneObject_DEPRECATED_movAltTransl, "movAltTransl", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Alternate translation axes enabled"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_DEPRECATED_movAltRot, "movAltRot", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Alternate rotation axes enabled"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_DEPRECATED_movPrefTranslX, "movPrefTranslX", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Preferred X-translation"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_DEPRECATED_movPrefTranslY, "movPrefTranslY", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Preferred Y-translation"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_DEPRECATED_movPrefTranslZ, "movPrefTranslZ", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Preferred Z-translation"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_DEPRECATED_movPrefRotX, "movPrefRotX", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Preferred X-axis rotation"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_DEPRECATED_movPrefRotY, "movPrefRotY", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Preferred Y-axis rotation"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_DEPRECATED_movPrefRotZ, "movPrefRotZ", sim_propertytype_bool, sim_propertyinfo_deprecated,  jsonStr({{"label", "Preferred Z-axis rotation"}, {"description", ""}}), "")

#define SHAPE_PROPERTIES \
    FUNCX(propShape_meshes, "meshes", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Meshes"}, {"description", "Mesh handles"}, {"handleType", "mesh"}}), "") \
    FUNCX(propShape_applyCulling, "applyCulling", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Apply culling"}, {"description", "Enables/disables culling for all contained meshes"}}), "") \
    FUNCX(propShape_applyShadingAngle, "applyShadingAngle", sim_propertytype_float, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Apply shading"}, {"description", "Applies a shading angle to all contained meshes"}}), "") \
    FUNCX(propShape_applyShowEdges, "applyShowEdges", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Apply edges"}, {"description", "Enables/disables edges for all contained meshes"}}), "") \
    FUNCX(propShape_flipFaces, "flipFaces", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Flip faces"}, {"description", "Flips faces of all contained meshes"}}), "") \
    FUNCX(propShape_applyColorDiffuse, "applyColor.diffuse", sim_propertytype_color, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Apply diffuse color"}, {"description", "Applies the diffuse color component to all contained meshes"}}), "") \
    FUNCX(propShape_applyColorSpecular, "applyColor.specular", sim_propertytype_color, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Apply specular color"}, {"description", "Applies the specular color component to all contained meshes"}}), "") \
    FUNCX(propShape_applyColorEmission, "applyColor.emission", sim_propertytype_color, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Apply emission color"}, {"description", "Applies the emission color component to all contained meshes"}}), "") \
    FUNCX(propShape_applyColorTransparency, "applyColor.transparency", sim_propertytype_float, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Apply transparency"}, {"description", "Applies transparency to all contained meshes"}}), "") \
    FUNCX(propShape_compoundColorDiffuse, "compoundColors.diffuse", sim_propertytype_floatarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Diffuse color data"}, {"description", "Diffuse color from all compound elements"}}), "") \
    FUNCX(propShape_compoundColorSpecular, "compoundColors.specular", sim_propertytype_floatarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Specular color data"}, {"description", "Specular color from all compound elements"}}), "") \
    FUNCX(propShape_compoundColorEmission, "compoundColors.emission", sim_propertytype_floatarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Emission color data"}, {"description", "Emission color from all compound elements"}}), "") \
    FUNCX(propShape_compoundColorTransparency, "compoundColors.transparency", sim_propertytype_floatarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Transparency data"}, {"description", "Transparency value from all compound elements"}}), "") \
    FUNCX(propShape_compoundEdges, "compoundEdges", sim_propertytype_intarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Edge visibility data"}, {"description", "Edge visibility from all compound elements"}}), "") \
    FUNCX(propShape_compoundWireframe, "compoundWireframe", sim_propertytype_intarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Wireframe data"}, {"description", "Wireframe state from all compound elements"}}), "") \
    FUNCX(propShape_compoundCullings, "compoundCullings", sim_propertytype_intarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Culling data"}, {"description", "Backface culling state from all compound elements"}}), "") \
    FUNCX(propShape_compoundShadingAngles, "compoundShadingAngles", sim_propertytype_floatarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Shading angle data"}, {"description", "Shading angle state from all compound elements"}}), "") \
    FUNCX(propShape_respondableMask, "respondableMask", sim_propertytype_int, 0,  jsonStr({{"label", "Respondable mask"}, {"description", ""}}), "") \
    FUNCX(propShape_startInDynSleepMode, "startInDynSleepMode", sim_propertytype_bool, 0,  jsonStr({{"label", "Start in sleep mode"}, {"description", ""}}), "") \
    FUNCX(propShape_dynamic, "dynamic", sim_propertytype_bool, 0,  jsonStr({{"label", "Dynamic"}, {"description", "Shape is dynamic, i.e. not static"}}), "") \
    FUNCX(propShape_kinematic, "kinematic", sim_propertytype_bool, 0,  jsonStr({{"label", "Kinematic"}, {"description", "Special flag mainly used for MuJoCo static shapes that move and need to transmit a friction"}}), "") \
    FUNCX(propShape_respondable, "respondable", sim_propertytype_bool, 0,  jsonStr({{"label", "Respondable"}, {"description", "Shape will transmit a collision force"}}), "") \
    FUNCX(propShape_setToDynamicWithParent, "setToDynamicWithParent", sim_propertytype_bool, 0,  jsonStr({{"label", "Set to dynamic if gets parent"}, {"description", "Shape will be made dynamic if it receives a parent"}}), "") \
    FUNCX(propShape_initLinearVelocity, "initLinearVelocity", sim_propertytype_vector3, 0,  jsonStr({{"label", "Initial linear velocity"}, {"description", ""}}), "") \
    FUNCX(propShape_initAngularVelocity, "initAngularVelocity", sim_propertytype_vector3, 0,  jsonStr({{"label", "Initial rotational velocity"}, {"description", ""}}), "") \
    FUNCX(propShape_dynLinearVelocity, "dynLinearVelocity", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Linear velocity"}, {"description", "Linear velocity, as transmitted by the physics engine"}}), "") \
    FUNCX(propShape_dynAngularVelocity, "dynAngularVelocity", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Rotational velocity"}, {"description", "Rotational velocity, as transmitted by the physics engine"}}), "") \
    FUNCX(propShape_convex, "convex", sim_propertytype_bool, sim_propertyinfo_notwritable,  jsonStr({{"label", "Convex"}, {"description", "Whether the shape's components are all convex or not"}}), "") \
    FUNCX(propShape_primitive, "primitive", sim_propertytype_bool, sim_propertyinfo_notwritable,  jsonStr({{"label", "Primitive"}, {"description", "Whether the shape's components are all primitives"}}), "") \
    FUNCX(propShape_compound, "compound", sim_propertytype_bool, sim_propertyinfo_notwritable,  jsonStr({{"label", "Compound"}, {"description", "Whether the shape is a compound"}}), "") \
    FUNCX(propShape_METHOD_addForce, "addForce", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_addTorque, "addTorque", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_relocateFrame, "relocateFrame", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_alignBoundingBox, "alignBoundingBox", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_getInertia, "getInertia", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_setInertia, "setInertia", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_computeInertia, "computeInertia", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_setAppearance, "setAppearance", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_getAppearance, "getAppearance", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_ungroup, "ungroup", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_divide, "divide", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "")

#define CAMERA_PROPERTIES \
    FUNCX(propCamera_size, "cameraSize", sim_propertytype_float, 0,  jsonStr({{"label", "Size"}, {"description", "Camera size"}}), "") \
    FUNCX(propCamera_parentAsManipProxy, "parentAsManipulationProxy", sim_propertytype_bool, 0,  jsonStr({{"label", "Parent as proxy"}, {"description", "Use parent as manipulation proxy"}}), "") \
    FUNCX(propCamera_translationEnabled, "translationEnabled", sim_propertytype_bool, 0,  jsonStr({{"label", "Translation enabled"}, {"description", ""}}), "") \
    FUNCX(propCamera_rotationEnabled, "rotationEnabled", sim_propertytype_bool, 0,  jsonStr({{"label", "Rotation enabled"}, {"description", ""}}), "") \
    FUNCX(propCamera_trackedObjectHandle, "trackedObjectHandle", sim_propertytype_int, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Tracked object"}, {"description", "Tracked scene object handle"}}), "")

#define DUMMY_PROPERTIES \
    FUNCX(propDummy_size, "dummySize", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Size"}, {"description", "Dummy size"}}), "") \
    FUNCX(propDummy_linkedDummy, "linkedDummy", sim_propertytype_handle, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Linked dummy"}, {"description", "Handle of the linked dummy"}, {"handleType", "dummy"}}), "") \
    FUNCX(propDummy_dummyType, "dummyType", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Type"}, {"description", "Dummy type"}}), "") \
    FUNCX(propDummy_assemblyTag, "assemblyTag", sim_propertytype_string, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Assembly tag"}, {"description", ""}}), "") \
    FUNCX(propDummy_engineProperties, "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Engine properties"}, {"description", "Engine properties as JSON text"}}), "") \
    FUNCX(propDummy_mujocoLimitsEnabled, "mujoco.limitsEnabled", sim_propertytype_bool, 0, sim_mujoco_dummy_limited, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoLimitsRange, "mujoco.limitsRange", sim_propertytype_floatarray, 0, sim_mujoco_dummy_range1, sim_mujoco_dummy_range2, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoLimitsSolref, "mujoco.limitsSolref", sim_propertytype_floatarray, 0, sim_mujoco_dummy_solreflimit1, sim_mujoco_dummy_solreflimit2, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoLimitsSolimp, "mujoco.limitsSolimp", sim_propertytype_floatarray, 0, sim_mujoco_dummy_solimplimit1, sim_mujoco_dummy_solimplimit2, sim_mujoco_dummy_solimplimit3, sim_mujoco_dummy_solimplimit4, sim_mujoco_dummy_solimplimit5,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoMargin, "mujoco.margin", sim_propertytype_float, 0, sim_mujoco_dummy_margin, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoSpringStiffness, "mujoco.springStiffness", sim_propertytype_float, 0, sim_mujoco_dummy_stiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoSpringDamping, "mujoco.springDamping", sim_propertytype_float, 0, sim_mujoco_dummy_damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoSpringLength, "mujoco.springLength", sim_propertytype_float, 0, sim_mujoco_dummy_springlength, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoJointProxyHandle, "mujoco.jointProxyHandle", sim_propertytype_int, sim_propertyinfo_modelhashexclude, sim_mujoco_dummy_proxyjointid, -1, -1, -1, -1,  jsonStr({{"label", "Joint proxy"}, {"description", "Handle of the joint proxy, MuJoCo only"}}), "") \
    FUNCX(propDummy_mujocoOverlapConstrSolref, "mujoco.overlapConstrSolref", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoOverlapConstrSolimp, "mujoco.overlapConstrSolimp", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoOverlapConstrTorqueScale, "mujoco.overlapConstrTorquescale", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propDummy_DEPRECATED_linkedDummyHandle, "linkedDummyHandle", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "")

#define FORCESENSOR_PROPERTIES \
    FUNCX(propForceSensor_size, "sensorSize", sim_propertytype_float, 0,  jsonStr({{"label", "Size"}, {"description", "Sensor size"}}), "") \
    FUNCX(propForceSensor_forceThresholdEnabled, "forceThresholdEnabled", sim_propertytype_bool, 0,  jsonStr({{"label", "Force threshold enabled"}, {"description", ""}}), "") \
    FUNCX(propForceSensor_torqueThresholdEnabled, "torqueThresholdEnabled", sim_propertytype_bool, 0,  jsonStr({{"label", "Torque threshold enabled"}, {"description", ""}}), "") \
    FUNCX(propForceSensor_filterType, "filterType", sim_propertytype_int, 0,  jsonStr({{"label", "Filter"}, {"description", "Filter type"}}), "") \
    FUNCX(propForceSensor_filterSampleSize, "filterSampleSize", sim_propertytype_int, 0,  jsonStr({{"label", "Filter sample"}, {"description", "Filter sample size"}}), "") \
    FUNCX(propForceSensor_consecutiveViolationsToTrigger, "consecutiveViolationsToTrigger", sim_propertytype_int, 0,  jsonStr({{"label", "Consecutive violations to trigger"}, {"description", ""}}), "") \
    FUNCX(propForceSensor_forceThreshold, "forceThreshold", sim_propertytype_float, 0,  jsonStr({{"label", "Force threshold"}, {"description", ""}}), "") \
    FUNCX(propForceSensor_torqueThreshold, "torqueThreshold", sim_propertytype_float, 0,  jsonStr({{"label", "Torque threshold"}, {"description", ""}}), "") \
    FUNCX(propForceSensor_sensorForce, "sensorForce", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Force"}, {"description", "Measured force vector"}}), "") \
    FUNCX(propForceSensor_sensorTorque, "sensorTorque", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Torque"}, {"description", "Measured torque vector"}}), "") \
    FUNCX(propForceSensor_filteredSensorForce, "filteredSensorForce", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Filtered force"}, {"description", "Filtered force vector"}}), "") \
    FUNCX(propForceSensor_filteredSensorTorque, "filteredSensorTorque", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Filtered torque"}, {"description", "Filtered torque vector"}}), "") \
    FUNCX(propForceSensor_intrinsicError, "intrinsicError", sim_propertytype_pose, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Intrinsic error"}, {"description", "Intrinsic error, generated by some physics engines"}}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propForceSensor_DEPRECATED_sensorAverageForce, "filterSensorForce", sim_propertytype_vector3, sim_propertyinfo_deprecated | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propForceSensor_DEPRECATED_sensorAverageTorque, "filterSensorTorque", sim_propertytype_vector3, sim_propertyinfo_deprecated | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \

#define GRAPH_PROPERTIES \
    FUNCX(propGraph_size, "graphSize", sim_propertytype_float, 0,  jsonStr({{"label", "Size"}, {"description", "Size of the 3D graph representation"}}), "") \
    FUNCX(propGraph_bufferSize, "bufferSize", sim_propertytype_int, 0,  jsonStr({{"label", "Buffer size"}, {"description", ""}}), "") \
    FUNCX(propGraph_cyclic, "cyclic", sim_propertytype_bool, 0,  jsonStr({{"label", "Cyclic"}, {"description", "Buffer is cyclic"}}), "") \
    FUNCX(propGraph_backgroundColor, "backgroundColor", sim_propertytype_color, 0,  jsonStr({{"label", "Background color"}, {"description", ""}}), "") \
    FUNCX(propGraph_foregroundColor, "foregroundColor", sim_propertytype_color, 0,  jsonStr({{"label", "Foreground color"}, {"description", ""}}), "") \
    FUNCX(propGraph_METHOD_addCurve, "addCurve", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propGraph_METHOD_addSignal, "addSignal", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propGraph_METHOD_setSignalPoint, "setSignalPoint", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propGraph_METHOD_removeTrace, "removeTrace", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propGraph_METHOD_snapshotTrace, "snapshotTrace", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propGraph_METHOD_step, "step", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propGraph_METHOD_resetGraph, "resetGraph", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "")

#define JOINT_PROPERTIES \
    FUNCX(propJoint_length, "jointLength", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Size"}, {"description", "Joint size"}}), "") \
    FUNCX(propJoint_diameter, "jointDiameter", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Size"}, {"description", "Joint size"}}), "") \
    FUNCX(propJoint_position, "jointPosition", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Position"}, {"description", "Joint linear/angular displacement"}}), "") \
    FUNCX(propJoint_quaternion, "jointQuaternion", sim_propertytype_quaternion, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Spherical joint quaternion"}, {"description", ""}}), "") \
    FUNCX(propJoint_screwLead, "screwLead", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Screw lead"}, {"description", ""}}), "") \
    FUNCX(propJoint_intrinsicError, "intrinsicError", sim_propertytype_pose, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Intrinsic error"}, {"description", "Intrinsic error, generated by some physics engines"}}), "") \
    FUNCX(propJoint_intrinsicPose, "intrinsicPose", sim_propertytype_pose, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Intrinsic pose"}, {"description", "Intrinsic pose (includes a possible intrinsic error)"}}), "") \
    FUNCX(propJoint_calcVelocity, "calcVelocity", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Velocity"}, {"description", "Calculated joint linear or angular velocity"}}), "") \
    FUNCX(propJoint_jointType, "jointType", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable, -1, -1, -1, -1, -1,  jsonStr({{"label", "Type"}, {"description", "Joint type"}}), "") \
    FUNCX(propJoint_cyclic, "cyclic", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Cyclic"}, {"description", "Cyclic revolute joint, has no limits"}}), "") \
    FUNCX(propJoint_enforceLimits, "enforceLimits", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Enforce limits"}, {"description", "Enforce limits strictly"}}), "") \
    FUNCX(propJoint_interval, "interval", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Interval"}, {"description", "Joint limits (lower and upper bounds)"}}), "") \
    FUNCX(propJoint_targetPos, "targetPos", sim_propertytype_float, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Target position"}, {"description", "Position to reach by controller"}}), "") \
    FUNCX(propJoint_targetVel, "targetVel", sim_propertytype_float, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Target velocity"}, {"description", "Velocity to reach by controller"}}), "") \
    FUNCX(propJoint_targetForce, "targetForce", sim_propertytype_float, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Target force"}, {"description", "Maximum force to exert"}}), "") \
    FUNCX(propJoint_jointForce, "jointForce", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Force"}, {"description", "Force applied"}}), "") \
    FUNCX(propJoint_averageJointForce, "averageJointForce", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Average force"}, {"description", "Force applied on average (in a simulation step)"}}), "") \
    FUNCX(propJoint_jointMode, "jointMode", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Mode"}, {"description", "Joint mode"}}), "") \
    FUNCX(propJoint_dynCtrlMode, "dynCtrlMode", sim_propertytype_int, 0, -1, -1, -1, -1, -1, jsonStr({{"label", "Control mode" }, {"description", "Joint control mode, when in dynamic mode"}, {"enum", "jointDynCtrlMode"}}), "") \
    FUNCX(propJoint_dependencyMaster, "dependencyMaster", sim_propertytype_handle, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dependency master"}, {"description", "Handle of master joint (in a dependency relationship)"}, {"handleType", "joint"}}), "") \
    FUNCX(propJoint_dependencyParams, "dependencyParams", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dependency parameters"}, {"description", "Dependency parameters (offset and coefficient)"}}), "") \
    FUNCX(propJoint_maxVelAccelJerk, "maxVelAccelJerk", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Maximum velocity, acceleration and jerk"}, {"description", ""}}), "") \
    FUNCX(propJoint_springDamperParams, "springDamperParams", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Spring damper parameters"}, {"description", ""}}), "") \
    FUNCX(propJoint_dynVelMode, "dynVelMode", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dynamic velocity mode"}, {"description", "Dynamic velocity mode (0: default, 1: Ruckig)"}}), "") \
    FUNCX(propJoint_dynPosMode, "dynPosMode", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dynamic position mode"}, {"description", "Dynamic position mode (0: default, 1: Ruckig)"}}), "") \
    FUNCX(propJoint_engineProperties, "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Engine properties"}, {"description", "Engine properties as JSON text"}}), "") \
    FUNCX(propJoint_bulletStopErp, "bullet.stopErp", sim_propertytype_float, 0, sim_bullet_joint_stoperp, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_bulletStopCfm, "bullet.stopCfm", sim_propertytype_float, 0, sim_bullet_joint_stopcfm, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_bulletNormalCfm, "bullet.normalCfm", sim_propertytype_float, 0, sim_bullet_joint_normalcfm, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_bulletPosPid, "bullet.posPid", sim_propertytype_floatarray, 0, sim_bullet_joint_pospid1, sim_bullet_joint_pospid2, sim_bullet_joint_pospid3, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_odeStopErp, "ode.stopErp", sim_propertytype_float, 0, sim_ode_joint_stoperp, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_odeStopCfm, "ode.stopCfm", sim_propertytype_float, 0, sim_ode_joint_stopcfm, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_odeNormalCfm, "ode.normalCfm", sim_propertytype_float, 0, sim_ode_joint_normalcfm, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_odeBounce, "ode.bounce", sim_propertytype_float, 0, sim_ode_joint_bounce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_odeFudgeFactor, "ode.fudge", sim_propertytype_float, 0, sim_ode_joint_fudgefactor, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_odePosPid, "ode.posPid", sim_propertytype_floatarray, 0, sim_ode_joint_pospid1, sim_ode_joint_pospid2, sim_ode_joint_pospid3, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexLowerLimitDamping, "vortex.axisLimitsLowerDamping", sim_propertytype_float, 0, sim_vortex_joint_lowerlimitdamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexUpperLimitDamping, "vortex.axisLimitsUpperDamping", sim_propertytype_float, 0, sim_vortex_joint_upperlimitdamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexLowerLimitStiffness, "vortex.axisLimitsLowerStiffness", sim_propertytype_float, 0, sim_vortex_joint_lowerlimitstiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexUpperLimitStiffness, "vortex.axisLimitsUpperStiffness", sim_propertytype_float, 0, sim_vortex_joint_upperlimitstiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexLowerLimitRestitution, "vortex.axisLimitsLowerRestitution", sim_propertytype_float, 0, sim_vortex_joint_lowerlimitrestitution, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexUpperLimitRestitution, "vortex.axisLimitsUpperRestitution", sim_propertytype_float, 0, sim_vortex_joint_upperlimitrestitution, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexLowerLimitMaxForce, "vortex.axisLimitsLowerMaxForce", sim_propertytype_float, 0, sim_vortex_joint_lowerlimitmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexUpperLimitMaxForce, "vortex.axisLimitsUpperMaxForce", sim_propertytype_float, 0, sim_vortex_joint_upperlimitmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexAxisFrictionEnabled, "vortex.axisFrictionEnabled", sim_propertytype_bool, 0, sim_vortex_joint_motorfrictionenabled, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexAxisFrictionProportional, "vortex.axisFrictionProportional", sim_propertytype_bool, 0, sim_vortex_joint_proportionalmotorfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexAxisFrictionCoeff, "vortex.axisFrictionValue", sim_propertytype_float, 0, sim_vortex_joint_motorconstraintfrictioncoeff, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexAxisFrictionMaxForce, "vortex.axisFrictionMaxForce", sim_propertytype_float, 0, sim_vortex_joint_motorconstraintfrictionmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexAxisFrictionLoss, "vortex.axisFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_motorconstraintfrictionloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexRelaxationEnabledBits, "vortex.relaxationEnabledBits", sim_propertytype_int, 0, sim_vortex_joint_relaxationenabledbc, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexFrictionEnabledBits, "vortex.frictionEnabledBits", sim_propertytype_int, 0, sim_vortex_joint_frictionenabledbc, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexFrictionProportionalBits, "vortex.frictionProportionalBits", sim_propertytype_int, 0, sim_vortex_joint_frictionproportionalbc, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisPosRelaxationStiffness, "vortex.xAxisPosRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_p0stiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisPosRelaxationDamping, "vortex.xAxisPosRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_p0damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisPosRelaxationLoss, "vortex.xAxisPosRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_p0loss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisPosFrictionCoeff, "vortex.xAxisPosFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_p0frictioncoeff, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisPosFrictionMaxForce, "vortex.xAxisPosFrictionMaxForce", sim_propertytype_float, 0, sim_vortex_joint_p0frictionmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisPosFrictionLoss, "vortex.xAxisPosFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_p0frictionloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisPosRelaxationStiffness, "vortex.yAxisPosRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_p1stiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisPosRelaxationDamping, "vortex.yAxisPosRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_p1damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisPosRelaxationLoss, "vortex.yAxisPosRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_p1loss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisPosFrictionCoeff, "vortex.yAxisPosFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_p1frictioncoeff, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisPosFrictionMaxForce, "vortex.yAxisPosFrictionMaxForce", sim_propertytype_float, 0, sim_vortex_joint_p1frictionmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisPosFrictionLoss, "vortex.yAxisPosFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_p1frictionloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisPosRelaxationStiffness, "vortex.zAxisPosRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_p2stiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisPosRelaxationDamping, "vortex.zAxisPosRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_p2damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisPosRelaxationLoss, "vortex.zAxisPosRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_p2loss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisPosFrictionCoeff, "vortex.zAxisPosFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_p2frictioncoeff, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisPosFrictionMaxForce, "vortex.zAxisPosFrictionMaxForce", sim_propertytype_float, 0, sim_vortex_joint_p2frictionmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisPosFrictionLoss, "vortex.zAxisPosFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_p2frictionloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisOrientRelaxStiffness, "vortex.xAxisOrientRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_a0stiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisOrientRelaxDamping, "vortex.xAxisOrientRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_a0damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisOrientRelaxLoss, "vortex.xAxisOrientRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_a0loss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisOrientFrictionCoeff, "vortex.xAxisOrientFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_a0frictioncoeff, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisOrientFrictionMaxTorque, "vortex.xAxisOrientFrictionMaxTorque", sim_propertytype_float, 0, sim_vortex_joint_a0frictionmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisOrientFrictionLoss, "vortex.xAxisOrientFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_a0frictionloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisOrientRelaxStiffness, "vortex.yAxisOrientRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_a1stiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisOrientRelaxDamping, "vortex.yAxisOrientRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_a1damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisOrientRelaxLoss, "vortex.yAxisOrientRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_a1loss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisOrientFrictionCoeff, "vortex.yAxisOrientFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_a1frictioncoeff, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisOrientFrictionMaxTorque, "vortex.yAxisOrientFrictionMaxTorque", sim_propertytype_float, 0, sim_vortex_joint_a1frictionmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisOrientFrictionLoss, "vortex.yAxisOrientFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_a1frictionloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisOrientRelaxStiffness, "vortex.zAxisOrientRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_a2stiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisOrientRelaxDamping, "vortex.zAxisOrientRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_a2damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisOrientRelaxLoss, "vortex.zAxisOrientRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_a2loss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisOrientFrictionCoeff, "vortex.zAxisOrientFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_a2frictioncoeff, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisOrientFrictionMaxTorque, "vortex.zAxisOrientFrictionMaxTorque", sim_propertytype_float, 0, sim_vortex_joint_a2frictionmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisOrientFrictionLoss, "vortex.zAxisOrientFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_a2frictionloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexPosPid, "vortex.posPid", sim_propertytype_floatarray, 0, sim_vortex_joint_pospid1, sim_vortex_joint_pospid2, sim_vortex_joint_pospid3, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_newtonPosPid, "newton.posPid", sim_propertytype_floatarray, 0, sim_newton_joint_pospid1, sim_newton_joint_pospid2, sim_newton_joint_pospid3, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoArmature, "mujoco.armature", sim_propertytype_float, 0, sim_mujoco_joint_armature, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoMargin, "mujoco.margin", sim_propertytype_float, 0, sim_mujoco_joint_margin, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoFrictionLoss, "mujoco.frictionLoss", sim_propertytype_float, 0, sim_mujoco_joint_frictionloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoSpringStiffness, "mujoco.springStiffness", sim_propertytype_float, 0, sim_mujoco_joint_stiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoSpringDamping, "mujoco.springDamping", sim_propertytype_float, 0, sim_mujoco_joint_damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoSpringRef, "mujoco.springRef", sim_propertytype_float, 0, sim_mujoco_joint_springref, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoPosPid, "mujoco.posPid", sim_propertytype_floatarray, 0, sim_mujoco_joint_pospid1, sim_mujoco_joint_pospid2, sim_mujoco_joint_pospid3, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoLimitsSolRef, "mujoco.limitsSolref", sim_propertytype_floatarray, 0, sim_mujoco_joint_solreflimit1, sim_mujoco_joint_solreflimit2, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoLimitsSolImp, "mujoco.limitsSolimp", sim_propertytype_floatarray, 0, sim_mujoco_joint_solimplimit1, sim_mujoco_joint_solimplimit2, sim_mujoco_joint_solimplimit3, sim_mujoco_joint_solimplimit4, sim_mujoco_joint_solimplimit5,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoFrictionSolRef, "mujoco.frictionSolref", sim_propertytype_floatarray, 0, sim_mujoco_joint_solreffriction1, sim_mujoco_joint_solreffriction2, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoFrictionSolImp, "mujoco.frictionSolimp", sim_propertytype_floatarray, 0, sim_mujoco_joint_solimpfriction1, sim_mujoco_joint_solimpfriction2, sim_mujoco_joint_solimpfriction3, sim_mujoco_joint_solimpfriction4, sim_mujoco_joint_solimpfriction5,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoSpringDamper, "mujoco.springSpringDamper", sim_propertytype_floatarray, 0, sim_mujoco_joint_springdamper1, sim_mujoco_joint_springdamper2, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoDependencyPolyCoef, "mujoco.dependencyPolyCoeff", sim_propertytype_floatarray, 0, sim_mujoco_joint_polycoef1, sim_mujoco_joint_polycoef2, sim_mujoco_joint_polycoef3, sim_mujoco_joint_polycoef4, sim_mujoco_joint_polycoef5,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propJoint_DEPRECATED_dependencyMaster, "dependencyMasterHandle", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dependency master"}, {"description", "Handle of master joint (in a dependency relationship)"}}), "")

#define LIGHT_PROPERTIES \
    FUNCX(propLight_size, "lightSize", sim_propertytype_float, 0,  jsonStr({{"label", "Size"}, {"description", "Light size"}}), "") \
    FUNCX(propLight_enabled, "enabled", sim_propertytype_bool, 0,  jsonStr({{"label", "Enabled"}, {"description", ""}}), "") \
    FUNCX(propLight_lightType, "lightType", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Type"}, {"description", "Light type"}}), "") \
    FUNCX(propLight_spotExponent, "spotExponent", sim_propertytype_int, 0,  jsonStr({{"label", "Spot exponent"}, {"description", ""}}), "") \
    FUNCX(propLight_spotCutoffAngle, "spotCutoffAngle", sim_propertytype_float, 0,  jsonStr({{"label", "Cut off angle"}, {"description", "Spot cut off angle"}}), "") \
    FUNCX(propLight_attenuationFactors, "attenuationFactors", sim_propertytype_floatarray, 0,  jsonStr({{"label", "Attenuation factor"}, {"description", ""}}), "") \
    FUNCX(propLight_povCastShadows, "povray.castShadows", sim_propertytype_bool, sim_propertyinfo_silent,  jsonStr({{"label", "POV-Ray: cast shadows"}, {"description", "Light casts shadows (with the POV-Ray renderer plugin)"}}), "")

#define MIRROR_PROPERTIES \
    FUNCX(propMirror_fake, "", sim_propertytype_float, sim_propertyinfo_constant | sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_notreadable,  jsonStr({{"label", ""}, {"description", ""}}), "")

#define OCTREE_PROPERTIES \
    FUNCX(propOctree_voxelSize, "voxelSize", sim_propertytype_float, 0,  jsonStr({{"label", "Voxel size"}, {"description", ""}}), "") \
    FUNCX(propOctree_randomColors, "randomColors", sim_propertytype_bool, 0,  jsonStr({{"label", "Random voxel colors"}, {"description", ""}}), "") \
    FUNCX(propOctree_showPoints, "showPoints", sim_propertytype_bool, 0,  jsonStr({{"label", "Show points instead of voxels"}, {"description", ""}}), "") \
    FUNCX(propOctree_points, "points", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Voxels"}, {"description", "Voxel positions"}}), "") \
    FUNCX(propOctree_colors, "colors", sim_propertytype_buffer, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Voxel Colors"}, {"description", ""}}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propOctree_DEPRECATED_voxels, "voxels", sim_propertytype_floatarray, sim_propertyinfo_deprecated | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Voxels"}, {"description", "Voxel positions"}}), "")

#define POINTCLOUD_PROPERTIES \
    FUNCX(propPointCloud_ocTreeStruct, "ocTreeStruct", sim_propertytype_bool, 0,  jsonStr({{"label", "OC-Tree structure Enabled"}, {"description", "Use an oc-tree structure"}}), "") \
    FUNCX(propPointCloud_randomColors, "randomColors", sim_propertytype_bool, 0,  jsonStr({{"label", "Random point colors"}, {"description", ""}}), "") \
    FUNCX(propPointCloud_pointSize, "pointSize", sim_propertytype_int, 0,  jsonStr({{"label", "Point size"}, {"description", ""}}), "") \
    FUNCX(propPointCloud_maxPtsInCell, "maxPointsInCell", sim_propertytype_int, 0,  jsonStr({{"label", "Max. points in cell"}, {"description", "Maximum number of points in an oc-tree cell/voxel"}}), "") \
    FUNCX(propPointCloud_cellSize, "cellSize", sim_propertytype_float, 0,  jsonStr({{"label", "Cell size"}, {"description", "Size of the oc-tree cell/voxel"}}), "") \
    FUNCX(propPointCloud_pointDisplayFraction, "pointDisplayFraction", sim_propertytype_float, 0,  jsonStr({{"label", "Display fraction"}, {"description", "Fraction of points to be displayed in an oc-tree cell/voxel"}}), "") \
    FUNCX(propPointCloud_points, "points", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Points"}, {"description", "Point positions"}}), "") \
    FUNCX(propPointCloud_colors, "colors", sim_propertytype_buffer, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Colors"}, {"description", "Point colors"}}), "")

#define PROXIMITYSENSOR_PROPERTIES \
    FUNCX(propProximitySensor_size, "sensorPointSize", sim_propertytype_float, 0,  jsonStr({{"label", "Sensor point size"}, {"description", ""}}), "") \
    FUNCX(propProximitySensor_frontFaceDetection, "frontFaceDetection", sim_propertytype_bool, 0,  jsonStr({{"label", "Front face detection"}, {"description", ""}}), "") \
    FUNCX(propProximitySensor_backFaceDetection, "backFaceDetection", sim_propertytype_bool, 0,  jsonStr({{"label", "Back face detection"}, {"description", ""}}), "") \
    FUNCX(propProximitySensor_exactMode, "exactMode", sim_propertytype_bool, 0,  jsonStr({{"label", "Exact mode"}, {"description", ""}}), "") \
    FUNCX(propProximitySensor_explicitHandling, "explicitHandling", sim_propertytype_bool, 0,  jsonStr({{"label", "Explicit handling"}, {"description", ""}}), "") \
    FUNCX(propProximitySensor_showVolume, "showVolume", sim_propertytype_bool, 0,  jsonStr({{"label", "Show volume"}, {"description", ""}}), "") \
    FUNCX(propProximitySensor_randomizedDetection, "randomizedDetection", sim_propertytype_bool, sim_propertyinfo_notwritable,  jsonStr({{"label", "Randomized detection"}, {"description", ""}}), "") \
    FUNCX(propProximitySensor_sensorType, "sensorType", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Type"}, {"description", "Sensor type"}}), "") \
    FUNCX(propProximitySensor_detectedObjectHandle, "detectedObjectHandle", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Detected object"}, {"description", "Detected scene object handle"}}), "") \
    FUNCX(propProximitySensor_angleThreshold, "angleThreshold", sim_propertytype_float, 0,  jsonStr({{"label", "Angle threshold"}, {"description", "Angle threshold, 0.0 to disable"}}), "") \
    FUNCX(propProximitySensor_detectedPoint, "detectedPoint", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Detected point"}, {"description", ""}}), "") \
    FUNCX(propProximitySensor_detectedNormal, "detectedNormal", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Detected normal"}, {"description", "Detected normal vector"}}), "") \
    FUNCX(propProximitySensor_METHOD_handleSensor, "handleSensor", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propProximitySensor_METHOD_resetSensor, "resetSensor", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propProximitySensor_METHOD_checkSensor, "checkSensor", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "")

#define VISIONSENSOR_PROPERTIES \
    FUNCX(propVisionSensor_size, "sensorSize", sim_propertytype_float, 0,  jsonStr({{"label", "Size"}, {"description", "Sensor size"}}), "") \
    FUNCX(propVisionSensor_backgroundCol, "backgroundColor", sim_propertytype_color, 0,  jsonStr({{"label", "Background color"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_renderMode, "renderMode", sim_propertytype_int, 0,  jsonStr({{"label", "Render mode"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_backgroundSameAsEnv, "backgroundColorFromEnvironment", sim_propertytype_bool, 0,  jsonStr({{"label", "Background color from environment"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_explicitHandling, "explicitHandling", sim_propertytype_bool, 0,  jsonStr({{"label", "Explicit handling"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_useExtImage, "useExtImage", sim_propertytype_bool, 0,  jsonStr({{"label", "External input"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_ignoreRgbInfo, "ignoreImageInfo", sim_propertytype_bool, 0,  jsonStr({{"label", "Ignore RGB buffer"}, {"description", "Ignore RGB buffer (faster)"}}), "") \
    FUNCX(propVisionSensor_ignoreDepthInfo, "ignoreDepthInfo", sim_propertytype_bool, 0,  jsonStr({{"label", "Ignore Depth buffer"}, {"description", "Ignore Depth buffer (faster)"}}), "") \
    FUNCX(propVisionSensor_omitPacket1, "omitPacket1", sim_propertytype_bool, 0,  jsonStr({{"label", "Packet 1 is blank"}, {"description", "Omit packet 1 (faster)"}}), "") \
    FUNCX(propVisionSensor_emitImageChangedEvent, "emitImageChangedEvent", sim_propertytype_bool, 0,  jsonStr({{"label", "Emit image change event"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_emitDepthChangedEvent, "emitDepthChangedEvent", sim_propertytype_bool, 0,  jsonStr({{"label", "Emit depth change event"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_imageBuffer, "imageBuffer", sim_propertytype_buffer, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "RGB buffer"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_depthBuffer, "depthBuffer", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Depth buffer"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_packedDepthBuffer, "packedDepthBuffer", sim_propertytype_buffer, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Packed depth buffer"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_triggerState, "triggerState", sim_propertytype_bool, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Trigger state"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_packet1, "packet1", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Data packet 1"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_packet2, "packet2", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Data packet 2"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_povFocalBlur, "povray.focalBlur", sim_propertytype_bool, sim_propertyinfo_silent,  jsonStr({{"label", "POV-Ray: focal blur"}, {"description", "Focal blur (with the POV-Ray renderer plugin)"}}), "") \
    FUNCX(propVisionSensor_povBlurSamples, "povray.blurSamples", sim_propertytype_int, sim_propertyinfo_silent,  jsonStr({{"label", "POV-Ray: blur samples"}, {"description", "Focal blur samples (with the POV-Ray renderer plugin)"}}), "") \
    FUNCX(propVisionSensor_povBlurDistance, "povray.blurDistance", sim_propertytype_float, sim_propertyinfo_silent,  jsonStr({{"label", "POV-Ray: blur distance"}, {"description", "Focal blur distance (with the POV-Ray renderer plugin)"}}), "") \
    FUNCX(propVisionSensor_povAperture, "povray.aperture", sim_propertytype_float, sim_propertyinfo_silent,  jsonStr({{"label", "POV-Ray: aperture"}, {"description", "Aperture (with the POV-Ray renderer plugin)"}}), "") \
    FUNCX(propVisionSensor_METHOD_handleSensor, "handleSensor", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propVisionSensor_METHOD_resetSensor, "resetSensor", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propVisionSensor_METHOD_checkSensor, "checkSensor", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propVisionSensor_METHOD_getDepth, "getDepth", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propVisionSensor_METHOD_getImage, "getImage", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propVisionSensor_METHOD_setImage, "setImage", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "")

#define SCRIPT_PROPERTIES \
    FUNCX(propScript_size, "scriptSize", sim_propertytype_float, 0,  jsonStr({{"label", "Size"}, {"description", "Size of the object"}}), "") \
    FUNCX(propScript_resetAfterSimError, "resetAfterSimError", sim_propertytype_bool, 0,  jsonStr({{"label", "Reset after simulation error"}, {"description", ""}}), "") \
    FUNCX(propScript_detachedScript, "detachedScript", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Detached script handle"}, {"description", ""}, {"handleType", "detachedScript"}}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propScript_DEPRECATED_scriptDisabled, "scriptDisabled", sim_propertytype_bool, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propScript_DEPRECATED_restartOnError, "restartOnError", sim_propertytype_bool, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propScript_DEPRECATED_execPriority, "execPriority", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propScript_DEPRECATED_scriptType, "scriptType", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude | sim_propertyinfo_notwritable,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propScript_DEPRECATED_executionDepth, "executionDepth", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propScript_DEPRECATED_scriptState, "scriptState", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propScript_DEPRECATED_language, "language", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propScript_DEPRECATED_code, "code", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propScript_DEPRECATED_scriptName, "scriptName", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propScript_DEPRECATED_addOnPath, "addOnPath", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propScript_DEPRECATED_addOnMenuPath, "addOnMenuPath", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", ""}, {"description", ""}}), "")

#define MARKER_PROPERTIES \
    FUNCX(propMarker_itemType, "itemType", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Type"}, {"description", "Item type"}}), "") \
    FUNCX(propMarker_cyclic, "cyclic", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Cyclic"}, {"description", "Item buffer is cyclic"}}), "") \
    FUNCX(propMarker_local, "local", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Local"}, {"description", "Coordinates are local to the marker's reference frame"}}), "") \
    FUNCX(propMarker_overlay, "overlay", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Overlay"}, {"description", "Items are are displayed overlaid"}}), "") \
    FUNCX(propMarker_points, "points", sim_propertytype_floatarray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Points"}, {"description", ""}}), "") \
    FUNCX(propMarker_quaternions, "quaternions", sim_propertytype_floatarray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Quaternions"}, {"description", ""}}), "") \
    FUNCX(propMarker_colors, "colors", sim_propertytype_buffer, sim_propertyinfo_notwritable,  jsonStr({{"label", "Colors"}, {"description", ""}}), "") \
    FUNCX(propMarker_sizes, "sizes", sim_propertytype_floatarray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Sizes"}, {"description", ""}}), "") \
    FUNCX(propMarker_vertices, "vertices", sim_propertytype_floatarray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Vertices"}, {"description", "Vertices for custom marker"}}), "") \
    FUNCX(propMarker_indices, "indices", sim_propertytype_intarray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Indices"}, {"description", "Indices for custom marker"}}), "") \
    FUNCX(propMarker_normals, "normals", sim_propertytype_floatarray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Normals"}, {"description", "Normals for custom marker"}}), "") \
    FUNCX(propMarker_METHOD_addItems, "addItems", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propMarker_METHOD_clearItems, "clearItems", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "") \
    FUNCX(propMarker_METHOD_removeItems, "removeItems", sim_propertytype_method, sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({}), "")

#define CUSTOMSCENEOBJECT_PROPERTIES \
    FUNCX(propCustomSceneObject_size, "objectSize", sim_propertytype_float, 0,  jsonStr({{"label", "Size"}, {"description", "Custom scene object size"}}), "")
