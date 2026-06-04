#pragma once

#include <string>
#include <vector>
#include <map>

struct SDeprecatedProperty {
    std::vector<std::string> replacements;
    std::vector<int> types;
};
extern const std::map<std::string, SDeprecatedProperty> propDeprecationMapping;

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

#define CUSTOMDATAPREFIX "customData"
#define CUSTOMDATAPREFIXDOT CUSTOMDATAPREFIX "."
#define CUSTOMDATAFLAGS (sim_propertyinfo_removable)

#define SIGNALPREFIX "signal"
#define SIGNALPREFIXDOT SIGNALPREFIX "."
#define SIGNALFLAGS (sim_propertyinfo_removable | sim_propertyinfo_modelhashexclude)

#define NAMEDPARAMPREFIX "namedParam"
#define NAMEDPARAMPREFIXDOT NAMEDPARAMPREFIX "."
#define NAMEDPARAMFLAGS (sim_propertyinfo_removable | sim_propertyinfo_modelhashexclude)

#define REFSPREFIX "refs"
#define REFSPREFIXDOT REFSPREFIX "."
#define REFSFLAGS (sim_propertyinfo_removable | sim_propertyinfo_modelhashexclude)

#define ORIGREFSPREFIX "origRefs"
#define ORIGREFSPREFIXDOT ORIGREFSPREFIX "."
#define ORIGREFSFLAGS (sim_propertyinfo_removable | sim_propertyinfo_modelhashexclude)

#define COLORPREFIX "color"
#define COLORPREFIXDOT COLORPREFIX "."
#define COLORPREFIX_CAP "Color"
#define COLORPREFIXDOT_CAP COLORPREFIX_CAP "."
#define COLORPREFIXTAG "__--__"
#define COLORPREFIXTAGDOT COLORPREFIXTAG "."

#define PATHSPREFIX "paths"
#define PATHSPREFIXDOT PATHSPREFIX "."

#define VOLUMEPREFIX "volume"
#define VOLUMEPREFIXDOT VOLUMEPREFIX "."

#define DYNAMICSPREFIX "dynamics"
#define DYNAMICSPREFIXDOT DYNAMICSPREFIX "."

#define BULLETPREFIX "bullet"
#define BULLETPREFIXDOT BULLETPREFIX "."

#define ODEPREFIX "ode"
#define ODEPREFIXDOT ODEPREFIX "."

#define NEWTONPREFIX "newton"
#define NEWTONPREFIXDOT NEWTONPREFIX "."

#define VORTEXPREFIX "vortex"
#define VORTEXPREFIXDOT VORTEXPREFIX "."

#define MUJOCOPREFIX "mujoco"
#define MUJOCOPREFIXDOT MUJOCOPREFIX "."

#define SIMULATIONPREFIX "simulation"
#define SIMULATIONPREFIXDOT SIMULATIONPREFIX "."

#define TEXTUREPREFIX "texture"
#define TEXTUREPREFIXDOT TEXTUREPREFIX "."

#define MODELPREFIX "model"
#define MODELPREFIXDOT MODELPREFIX "."

#define MOVPREFIX "mov"
#define MOVPREFIXDOT MOVPREFIX "."

#define APPLYCOLORPREFIX "applyColor"
#define APPLYCOLORPREFIXDOT APPLYCOLORPREFIX "."

#define COMPOUNDCOLORSPREFIX "compoundColors"
#define COMPOUNDCOLORSPREFIXDOT COMPOUNDCOLORSPREFIX "."

#define POVRAYPREFIX "povray"
#define POVRAYPREFIXDOT POVRAYPREFIX "."

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
#define proptypetag_group "&grp&."

#define SIM_PROPERTYINFO_DEPRECATED (sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude)
#define SIM_PROPERTYINFO_METHOD (sim_propertyinfo_silent | sim_propertyinfo_constant | sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude)
#define SIM_PROPERTYINFO_GROUP (sim_propertyinfo_silent | sim_propertyinfo_constant | sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude)

#define OBJECT_PROPERTIES \
    FUNCX(propObject_objectType, "objectType", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, jsonStr({{"label", "Object type"}, {"description", ""}}), "") \
    FUNCX(propObject_handle, "handle", sim_propertytype_long, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, jsonStr({{"label", "Handle"}, {"description", ""}}), "") \
    FUNCX(propObject_metaInfoSuperClass, "metaInfo.superClass", sim_propertytype_stringarray, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, jsonStr({{"label", "object super class"}, {"description", ""}}), "") \
    FUNCX(propObject_metaInfoNameSpaces, "metaInfo.namespaces", sim_propertytype_stringarray, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, jsonStr({{"label", "object name spaces"}, {"description", ""}}), "") \
    FUNCX(propObject_metaInfoIsClass, "metaInfo.isClass", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, jsonStr({{"label", "class flag"}, {"description", ""}}), "") \
    FUNCX(propObject_metaInfoIsSceneObject, "metaInfo.isSceneObject", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, jsonStr({{"label", "sceneObject flag"}, {"description", ""}}), "") \
    FUNCX(propObject_GROUP_metaInfo, "metaInfo", sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    FUNCX(propObject_METHOD_getBoolProperty, "getBoolProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getBufferProperty, "getBufferProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getColorProperty, "getColorProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getFloatArrayProperty, "getFloatArrayProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getFloatProperty, "getFloatProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getStringArrayProperty, "getStringArrayProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getHandleArrayProperty, "getHandleArrayProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getHandleProperty, "getHandleProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getIntArray2Property, "getIntArray2Property", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getIntArrayProperty, "getIntArrayProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getIntProperty, "getIntProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getLongProperty, "getLongProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getPoseProperty, "getPoseProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getQuaternionProperty, "getQuaternionProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getStringProperty, "getStringProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getTableProperty, "getTableProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getVector3Property, "getVector3Property", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setBoolProperty, "setBoolProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setBufferProperty, "setBufferProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setColorProperty, "setColorProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setFloatArrayProperty, "setFloatArrayProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setFloatProperty, "setFloatProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setStringArrayProperty, "setStringArrayProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setHandleArrayProperty, "setHandleArrayProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setHandleProperty, "setHandleProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setIntArray2Property, "setIntArray2Property", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setIntArrayProperty, "setIntArrayProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setIntProperty, "setIntProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setLongProperty, "setLongProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setPoseProperty, "setPoseProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setQuaternionProperty, "setQuaternionProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setStringProperty, "setStringProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setTableProperty, "setTableProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setVector3Property, "setVector3Property", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setMatrixProperty, "setMatrixProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getMatrixProperty, "getMatrixProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setMethodProperty, "setMethodProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getMethodProperty, "getMethodProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_removeProperty, "removeProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getPropertyInfo, "getPropertyInfo", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getPropertyInfos, "getPropertyInfos", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getPropertyName, "getPropertyName", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getProperties, "getProperties", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setProperties, "setProperties", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_setProperty, "setProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getPropertiesInfos, "getPropertiesInfos", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getProperty, "getProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_getPropertyTypeString, "getPropertyTypeString", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propObject_METHOD_isValid, "isValid", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "")

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
    FUNCX(propApp_machineId, "machineId", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Machine ID"}, {"description", ""}}), "") \
    FUNCX(propApp_legacyMachineId, "legacyMachineId", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Legacy machine ID"}, {"description", ""}}), "") \
    FUNCX(propApp_appDir, PATHSPREFIXDOT "app", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Application path"}, {"description", ""}}), "") \
    FUNCX(propApp_tempDir, PATHSPREFIXDOT "temp", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Temporary path"}, {"description", ""}}), "") \
    FUNCX(propApp_sceneTempDir, PATHSPREFIXDOT "sceneTemp", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Scene temporary path"}, {"description", ""}}), "") \
    FUNCX(propApp_settingsDir, PATHSPREFIXDOT "settings", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Settings path"}, {"description", ""}}), "") \
    FUNCX(propApp_luaDir, PATHSPREFIXDOT "lua", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Lua path"}, {"description", ""}}), "") \
    FUNCX(propApp_pythonDir, PATHSPREFIXDOT "python", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Python path"}, {"description", ""}}), "") \
    FUNCX(propApp_mujocoDir, PATHSPREFIXDOT "mujoco", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "MuJoCo path"}, {"description", ""}}), "") \
    FUNCX(propApp_systemDir, PATHSPREFIXDOT "system", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "System path"}, {"description", ""}}), "") \
    FUNCX(propApp_resourceDir, PATHSPREFIXDOT "resources", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Resource path"}, {"description", ""}}), "") \
    FUNCX(propApp_addOnDir, PATHSPREFIXDOT "addOns", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Add-on path"}, {"description", ""}}), "") \
    FUNCX(propApp_sceneDir, PATHSPREFIXDOT "scenes", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Scene path"}, {"description", ""}}), "") \
    FUNCX(propApp_modelDir, PATHSPREFIXDOT "models", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Model path"}, {"description", ""}}), "") \
    FUNCX(propApp_importExportDir, PATHSPREFIXDOT "importExport", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Import/export path"}, {"description", ""}}), "") \
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
    FUNCX(propApp_appArgs, "args", sim_propertytype_stringarray, sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App args"}, {"description", ""}}), "") \
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
    FUNCX(propApp_GROUP_namedParam, NAMEDPARAMPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    FUNCX(propApp_GROUP_customData, CUSTOMDATAPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    FUNCX(propApp_GROUP_signal, SIGNALPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    FUNCX(propApp_GROUP_paths, PATHSPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    FUNCX(propApp_METHOD_handleAddOnScripts, "handleAddOnScripts", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_handleSandboxScript, "handleSandboxScript", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_loadModelThumbnail, "loadModelThumbnail", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_loadModelThumbnailFromBuffer, "loadModelThumbnailFromBuffer", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_loadScene, "loadScene", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_loadSceneFromBuffer, "loadSceneFromBuffer", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_getObjects, "getObjects", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_createObject, "createObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_duplicateObjects, "duplicateObjects", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_loadImage, "loadImage", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_loadImageFromBuffer, "loadImageFromBuffer", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_saveImage, "saveImage", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_saveImageToBuffer, "saveImageToBuffer", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_transformImage, "transformImage", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_removeObjects, "removeObjects", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_logInfo, "logInfo", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_logWarn, "logWarn", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_logError, "logError", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_systemLock, "systemLock", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
/*    FUNCX(propApp_METHOD_packTable, "packTable", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") */ \
/*    FUNCX(propApp_METHOD_unpackTable, "unpackTable", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") */ \
    FUNCX(propApp_METHOD_pack, "pack", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpack, "unpack", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_serialize, "serialize", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_deserialize, "deserialize", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packDoubleArray, "packDoubleArray", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packFloatArray, "packFloatArray", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packInt64Array, "packInt64Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packInt32Array, "packInt32Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packUInt32Array, "packUInt32Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packInt16Array, "packInt16Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packUInt16Array, "packUInt16Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packInt8Array, "packInt8Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_packUInt8Array, "packUInt8Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackDoubleArray, "unpackDoubleArray", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackFloatArray, "unpackFloatArray", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackInt64Array, "unpackInt64Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackInt32Array, "unpackInt32Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackUInt32Array, "unpackUInt32Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackInt16Array, "unpackInt16Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackUInt16Array, "unpackUInt16Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackInt8Array, "unpackInt8Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_unpackUInt8Array, "unpackUInt8Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_fastIdleLoop, "fastIdleLoop", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_throttle, "throttle", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_scheduleExecution, "scheduleExecution", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_cancelScheduledExecution, "cancelScheduledExecution", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_openFile, "openFile", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_createCustomObjectClass, "createCustomObjectClass", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_quit, "quit", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_pushEvent, "pushEvent", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_getGenesisEvents, "getGenesisEvents", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_getPluginInfo, "getPluginInfo", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propApp_METHOD_setPluginInfo, "setPluginInfo", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propApp_DEPRECATED_appDir, "appPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propApp_DEPRECATED_tempDir, "tempPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propApp_DEPRECATED_sceneTempDir, "sceneTempPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propApp_DEPRECATED_settingsDir, "settingsPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propApp_DEPRECATED_luaDir, "luaPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propApp_DEPRECATED_pythonDir, "pythonPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propApp_DEPRECATED_mujocoDir, "mujocoPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propApp_DEPRECATED_systemDir, "systemPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propApp_DEPRECATED_resourceDir, "resourcePath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propApp_DEPRECATED_addOnDir, "addOnPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propApp_DEPRECATED_sceneDir, "scenePath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent, "", "") \
    FUNCX(propApp_DEPRECATED_modelDir, "modelPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent, "", "") \
    FUNCX(propApp_DEPRECATED_importExportDir, "importExportPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent, "", "") \
    FUNCX(propApp_DEPRECATED_appArg1, "appArg1", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 1"}, {"description", ""}}), "") \
    FUNCX(propApp_DEPRECATED_appArg2, "appArg2", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 2"}, {"description", ""}}), "") \
    FUNCX(propApp_DEPRECATED_appArg3, "appArg3", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 3"}, {"description", ""}}), "") \
    FUNCX(propApp_DEPRECATED_appArg4, "appArg4", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 4"}, {"description", ""}}), "") \
    FUNCX(propApp_DEPRECATED_appArg5, "appArg5", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 5"}, {"description", ""}}), "") \
    FUNCX(propApp_DEPRECATED_appArg6, "appArg6", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 6"}, {"description", ""}}), "") \
    FUNCX(propApp_DEPRECATED_appArg7, "appArg7", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 7"}, {"description", ""}}), "") \
    FUNCX(propApp_DEPRECATED_appArg8, "appArg8", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 8"}, {"description", ""}}), "") \
    FUNCX(propApp_DEPRECATED_appArg9, "appArg9", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "App arg. 9"}, {"description", ""}}), "")

#define CUSTOMOBJECTCLASS_PROPERTIES \
    FUNCX(propCustomObjectClass_METHOD_makeOject, "makeObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propCustomObjectClass_METHOD_remove, "remove", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "")

#define CUSTOMOBJECT_PROPERTIES \
    FUNCX(propCustomObject_METHOD_remove, "remove", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "")

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
    FUNCX(propDetachedScript_addOnMenuPath, "addOnMenuPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Add-on menu path"}, {"description", "Menu path of add-on"}}), "") \
    FUNCX(propDetachedScript_autoYieldDelay, "autoYieldDelay", sim_propertytype_float, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Automatic yield delay"}, {"description", ""}}), "") \
    FUNCX(propDetachedScript_METHOD_callFunction, "callFunction", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_getFunctions, "getFunctions", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_executeString, "executeString", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
/*    FUNCX(propDetachedScript_METHOD_getApiFunc, "getApiFunc", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") */ \
/*    FUNCX(propDetachedScript_METHOD_getApiInfo, "getApiInfo", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") */ \
    FUNCX(propDetachedScript_METHOD_getStackTraceback, "getStackTraceback", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_remove, "remove", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_registerFunctionHook, "registerFunctionHook", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_removeFunctionHook, "removeFunctionHook", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_lock, "lock", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_getObject, "getObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_setStepping, "setStepping", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_getStepping, "getStepping", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_yield, "yield", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_step, "step", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_wait, "wait", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_init, "init", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_setEventFilters, "setEventFilters", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propDetachedScript_METHOD_broadcast, "broadcast", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propDetachedScript_DEPRECATED_scriptType, "scriptType", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"", ""}, {"", ""}}), "") \
    FUNCX(propDetachedScript_DEPRECATED_scriptDisabled, "scriptDisabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  jsonStr({{"", ""}, {"", ""}}), "") \
    FUNCX(propDetachedScript_DEPRECATED_scriptState, "scriptState", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,  jsonStr({{"", ""}, {"", ""}}), "")

#define STACK_PROPERTIES \
    FUNCX(propStack_content, "content", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Content"}, {"description", ""}}), "")

#define COLLECTIONCONT_PROPERTIES \
    FUNCX(propCollectionCont_collections, "collections", sim_propertytype_handlearray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Collections"}, {"description", "Handles of all collections"}, {"handleType", "collection"}}), "")

#define COLLECTION_PROPERTIES \
    FUNCX(propCollection_objects, "objects", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Children handles"}, {"description", ""}, {"handleType", "sceneObject"}}), "") \
    FUNCX(propCollection_METHOD_addItem, "addItem", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propCollection_METHOD_removeItem, "removeItem", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propCollection_METHOD_checkCollision, "checkCollision", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propCollection_METHOD_remove, "remove", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propCollection_METHOD_changeColor, "changeColor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propCollection_METHOD_restoreColor, "restoreColor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propCollection_METHOD_checkDistance, "checkDistance", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "")

#define DRAWINGOBJECTCONT_PROPERTIES \
    FUNCX(propDrawingObjectCont_drawingObjects, "drawingObjects", sim_propertytype_handlearray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Drawing objects"}, {"description", "Handles of all drawing objects"}, {"handleType", "drawingObject"}}), "")

#define DRAWINGOBJECT_PROPERTIES \
    FUNCX(propDrawingObject_parent, "parent", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Parent handle"}, {"description", ""}, {"handleType", "sceneObject"}}), "") \
    FUNCX(propDrawingObject_METHOD_remove, "remove", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "")

#define COLOR_PROPERTIES /* do not change order of following 5 properties!! */ \
    FUNCX(propColor_colDiffuse, "diffuse", sim_propertytype_color, 0,  jsonStr({{"label", "Diffuse color"}, {"description", ""}}), "") \
    FUNCX(propColor_colSpecular, "specular", sim_propertytype_color, 0,  jsonStr({{"label", "Specular color"}, {"description", ""}}), "") \
    FUNCX(propColor_colEmission, "emission", sim_propertytype_color, 0,  jsonStr({{"label", "Emission color"}, {"description", ""}}), "") \
    FUNCX(propColor_transparency, "transparency", sim_propertytype_float, 0,  jsonStr({{"label", "Transparency"}, {"description", ""}}), "") \
    FUNCX(propColor_GROUP_prefix, COLORPREFIXTAG, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "")

#define CONVEXVOLUME_PROPERTIES \
    FUNCX(propConvexVolume_closeThreshold, "closeThreshold", sim_propertytype_float, 0,  jsonStr({{"label", "Close threshold"}, {"description", "Close threshold: if a detection occures below that threshold, it is not registered. 0.0 to disable"}}), "") \
    FUNCX(propConvexVolume_offset, VOLUMEPREFIXDOT "offset", sim_propertytype_float, 0,  jsonStr({{"label", "Offset"}, {"description", "Offset of detection volume"}}), "") \
    FUNCX(propConvexVolume_range, VOLUMEPREFIXDOT "range", sim_propertytype_float, 0,  jsonStr({{"label", "Range"}, {"description", "Range/depth of detection volume"}}), "") \
    FUNCX(propConvexVolume_xSize, VOLUMEPREFIXDOT "xSize", sim_propertytype_floatarray, 0,  jsonStr({{"label", "X-sizes"}, {"description", "X-size (near and far) for pyramid-type volumes"}}), "") \
    FUNCX(propConvexVolume_ySize, VOLUMEPREFIXDOT "ySize", sim_propertytype_floatarray, 0,  jsonStr({{"label", "Y-sizes"}, {"description", "Y-size (near and far) for pyramid-type volumes"}}), "") \
    FUNCX(propConvexVolume_radius, VOLUMEPREFIXDOT "radius", sim_propertytype_floatarray, 0,  jsonStr({{"label", "Radius"}, {"description", "Radius for cylinder-, disk- and cone-type volumes"}}), "") \
    FUNCX(propConvexVolume_angle, VOLUMEPREFIXDOT "angle", sim_propertytype_floatarray, 0,  jsonStr({{"label", "Angles"}, {"description", "Angle and inside gap for disk- and cone-type volumes"}}), "") \
    FUNCX(propConvexVolume_faces, VOLUMEPREFIXDOT "faces", sim_propertytype_intarray, 0,  jsonStr({{"label", "Faces"}, {"description", "Number of faces (near and far) for cylinder-, disk- and pyramid-type volumes"}}), "") \
    FUNCX(propConvexVolume_subdivisions, VOLUMEPREFIXDOT "subdivisions", sim_propertytype_intarray, 0,  jsonStr({{"label", "Subdivisions"}, {"description", "Number of subdivisions (near and far) for cone-type volumes"}}), "") \
    FUNCX(propConvexVolume_edges, VOLUMEPREFIXDOT "edges", sim_propertytype_floatarray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Volume edges"}, {"description", "List of segments (defined by pairs of end-point coordinates) visualizing the volume"}}), "") \
    FUNCX(propConvexVolume_closeEdges, VOLUMEPREFIXDOT "closeEdges", sim_propertytype_floatarray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Volume close edges"}, {"description", "List of segments (defined by pairs of end-point coordinates) visualizing the close threshold of the volume"}}), "") \
    FUNCX(propConvexVolume_GROUP_volume, VOLUMEPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    /* Following for backward compatibility: */ \
    FUNCX(propConvexVolume_DEPRECATED_offset, "volume_offset", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_range, "volume_range", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_xSize, "volume_xSize", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_ySize, "volume_ySize", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_radius, "volume_radius", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_angle, "volume_angle", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_faces, "volume_faces", sim_propertytype_intarray, SIM_PROPERTYINFO_DEPRECATED,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_subdivisions, "volume_subdivisions", sim_propertytype_intarray, SIM_PROPERTYINFO_DEPRECATED,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_edges, "volume_edges", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,  "", "") \
    FUNCX(propConvexVolume_DEPRECATED_closeEdges, "volume_closeEdges", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,  "", "")

#define DYNCONT_PROPERTIES \
    FUNCX(propDynCont_dynamicsEnabled, DYNAMICSPREFIXDOT "enabled", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dynamics enabled"}, {"description", ""}}), "") \
    FUNCX(propDynCont_showContactPoints, DYNAMICSPREFIXDOT "showContactPoints", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Show contact points"}, {"description", ""}}), "") \
    FUNCX(propDynCont_dynamicsEngine, DYNAMICSPREFIXDOT "engine", sim_propertytype_intarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dynamics engine"}, {"description", "Selected dynamics engine index and version"}}), "") \
    FUNCX(propDynCont_dynamicsStepSize, DYNAMICSPREFIXDOT "stepSize", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dynamics dt"}, {"description", "Dynamics step size"}}), "") \
    FUNCX(propDynCont_gravity, DYNAMICSPREFIXDOT "gravity", sim_propertytype_vector3, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Gravity"}, {"description", ""}}), "") \
    FUNCX(propDynCont_engineProperties, DYNAMICSPREFIXDOT "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Engine properties"}, {"description", "Engine properties as JSON text"}}), "") \
    FUNCX(propDynCont_bulletSolver, DYNAMICSPREFIXDOT BULLETPREFIXDOT "solver", sim_propertytype_int, 0, sim_bullet_global_constraintsolvertype, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_bulletIterations, DYNAMICSPREFIXDOT BULLETPREFIXDOT "iterations", sim_propertytype_int, 0, sim_bullet_global_constraintsolvingiterations, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_bulletComputeInertias, DYNAMICSPREFIXDOT BULLETPREFIXDOT "computeInertias", sim_propertytype_bool, 0, sim_bullet_global_computeinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_bulletInternalScalingFull, DYNAMICSPREFIXDOT BULLETPREFIXDOT "internalScalingFull", sim_propertytype_bool, 0, sim_bullet_global_fullinternalscaling, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_bulletInternalScalingScaling, DYNAMICSPREFIXDOT BULLETPREFIXDOT "internalScalingValue", sim_propertytype_float, 0, sim_bullet_global_internalscalingfactor, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_bulletCollMarginScaling, DYNAMICSPREFIXDOT BULLETPREFIXDOT "collisionMarginScaling", sim_propertytype_float, 0, sim_bullet_global_collisionmarginfactor, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_odeQuickStepEnabled, DYNAMICSPREFIXDOT ODEPREFIXDOT "quickStepEnabled", sim_propertytype_bool, 0, sim_ode_global_quickstep, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_odeQuickStepIterations, DYNAMICSPREFIXDOT ODEPREFIXDOT "quickStepIterations", sim_propertytype_int, 0, sim_ode_global_constraintsolvingiterations, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_odeComputeInertias, DYNAMICSPREFIXDOT ODEPREFIXDOT "computeInertias", sim_propertytype_bool, 0, sim_ode_global_computeinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_odeInternalScalingFull, DYNAMICSPREFIXDOT ODEPREFIXDOT "internalScalingFull", sim_propertytype_bool, 0, sim_ode_global_fullinternalscaling, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_odeInternalScalingScaling, DYNAMICSPREFIXDOT ODEPREFIXDOT "internalScalingValue", sim_propertytype_float, 0, sim_ode_global_internalscalingfactor, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_odeGlobalErp, DYNAMICSPREFIXDOT ODEPREFIXDOT "globalErp", sim_propertytype_float, 0, sim_ode_global_erp, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_odeGlobalCfm, DYNAMICSPREFIXDOT ODEPREFIXDOT "globalCfm", sim_propertytype_float, 0, sim_ode_global_cfm, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexComputeInertias, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "computeInertias", sim_propertytype_bool, 0, sim_vortex_global_computeinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexContactTolerance, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "contactTolerance", sim_propertytype_float, 0, sim_vortex_global_contacttolerance, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexAutoSleep, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoSleep", sim_propertytype_bool, 0, sim_vortex_global_autosleep, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexMultithreading, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "multithreading", sim_propertytype_bool, 0, sim_vortex_global_multithreading, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexConstraintsLinearCompliance, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "constraintsLinearCompliance", sim_propertytype_float, 0, sim_vortex_global_constraintlinearcompliance, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexConstraintsLinearDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "constraintsLinearDamping", sim_propertytype_float, 0, sim_vortex_global_constraintlineardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexConstraintsLinearKineticLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "constraintsLinearKineticLoss", sim_propertytype_float, 0, sim_vortex_global_constraintlinearkineticloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexConstraintsAngularCompliance, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "constraintsAngularCompliance", sim_propertytype_float, 0, sim_vortex_global_constraintangularcompliance, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexConstraintsAngularDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "constraintsAngularDamping", sim_propertytype_float, 0, sim_vortex_global_constraintangulardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_vortexConstraintsAngularKineticLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "constraintsAngularKineticLoss", sim_propertytype_float, 0, sim_vortex_global_constraintangularkineticloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_newtonIterations, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "iterations", sim_propertytype_int, 0, sim_newton_global_constraintsolvingiterations, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_newtonComputeInertias, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "computeInertias", sim_propertytype_bool, 0, sim_newton_global_computeinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_newtonMultithreading, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "multithreading", sim_propertytype_bool, 0, sim_newton_global_multithreading, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_newtonExactSolver, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "exactSolver", sim_propertytype_bool, 0, sim_newton_global_exactsolver, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_newtonHighJointAccuracy, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "highJointAccuracy", sim_propertytype_bool, 0, sim_newton_global_highjointaccuracy, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_newtonContactMergeTolerance, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "contactMergeTolerance", sim_propertytype_float, 0, sim_newton_global_contactmergetolerance, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoIntegrator, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "integrator", sim_propertytype_int, 0, sim_mujoco_global_integrator, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoSolver, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "solver", sim_propertytype_int, 0, sim_mujoco_global_solver, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoIterations, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "iterations", sim_propertytype_int, 0, sim_mujoco_global_iterations, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoRebuildTrigger, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "rebuildTrigger", sim_propertytype_int, 0, sim_mujoco_global_rebuildtrigger, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoComputeInertias, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "computeInertias", sim_propertytype_bool, 0, sim_mujoco_global_computeinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoMbMemory, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "mbmemory", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoNjMax, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "njmax", sim_propertytype_int, 0, sim_mujoco_global_njmax, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", "Deprecated. See mujoco.mbmemory instead."}}), "") \
    FUNCX(propDynCont_mujocoNconMax, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "nconmax", sim_propertytype_int, 0, sim_mujoco_global_nconmax, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", "Deprecated. See mujoco.mbmemory instead."}}), "") \
    FUNCX(propDynCont_mujocoNstack, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "nstack", sim_propertytype_int, 0, sim_mujoco_global_nstack, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", "Deprecated. See mujoco.mbmemory instead."}}), "") \
    FUNCX(propDynCont_mujocoCone, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "cone", sim_propertytype_int, 0, sim_mujoco_global_cone, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoKinematicBodiesOverrideFlags, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "kinematicBodiesOverrideFlags", sim_propertytype_int, 0, sim_mujoco_global_overridekin, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoKinematicBodiesMass, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "kinematicBodiesMass", sim_propertytype_float, 0, sim_mujoco_global_kinmass, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoKinematicBodiesInertia, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "kinematicBodiesInertia", sim_propertytype_float, 0, sim_mujoco_global_kininertia, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoBoundMass, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "boundMass", sim_propertytype_float, 0, sim_mujoco_global_boundmass, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoBoundInertia, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "boundInertia", sim_propertytype_float, 0, sim_mujoco_global_boundinertia, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoBalanceInertias, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "balanceInertias", sim_propertytype_bool, 0, sim_mujoco_global_balanceinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoMultithreaded, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "multithreaded", sim_propertytype_bool, 0, sim_mujoco_global_multithreaded, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoMulticcd, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "multiccd", sim_propertytype_bool, 0, sim_mujoco_global_multiccd, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoContactParamsOverride, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "contactParamsOverride", sim_propertytype_bool, 0, sim_mujoco_global_overridecontacts, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoContactParamsMargin, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "contactParamsMargin", sim_propertytype_float, 0, sim_mujoco_global_overridemargin, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoContactParamsSolref, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "contactParamsSolref", sim_propertytype_floatarray, 0, sim_mujoco_global_overridesolref1, sim_mujoco_global_overridesolref2, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoContactParamsSolimp, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "contactParamsSolimp", sim_propertytype_floatarray, 0, sim_mujoco_global_overridesolimp1, sim_mujoco_global_overridesolimp2, sim_mujoco_global_overridesolimp3, sim_mujoco_global_overridesolimp4, sim_mujoco_global_overridesolimp5,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoImpRatio, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "impratio", sim_propertytype_float, 0, sim_mujoco_global_impratio, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoWind, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "wind", sim_propertytype_vector3, 0, sim_mujoco_global_wind1, sim_mujoco_global_wind2, sim_mujoco_global_wind3, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoDensity, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "density", sim_propertytype_float, 0, sim_mujoco_global_density, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoViscosity, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "viscosity", sim_propertytype_float, 0, sim_mujoco_global_viscosity, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoJacobian, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "jacobian", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoTolerance, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "tolerance", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoLs_iterations, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "ls_iterations", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoLs_tolerance, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "ls_tolerance", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoNoslip_iterations, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "noslip_iterations", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoNoslip_tolerance, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "noslip_tolerance", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoCcd_iterations, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "ccd_iterations", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoCcd_tolerance, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "ccd_tolerance", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoSdf_iterations, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "sdf_iterations", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoSdf_initpoints, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "sdf_initpoints", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoEqualityEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "equalityEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoFrictionlossEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "frictionlossEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoLimitEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "limitEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoContactEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "contactEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoPassiveEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "passiveEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoGravityEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "gravityEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoWarmstartEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "warmstartEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoActuationEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "actuationEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoRefsafeEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "refsafeEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoSensorEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "sensorEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoMidphaseEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "midphaseEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoEulerdampEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "eulerdampEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoAutoresetEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "autoresetEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoEnergyEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "energyEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoInvdiscreteEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "invdiscreteEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoNativeccdEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "nativeccdEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoAlignfree, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "alignfree", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoKinematicWeldSolref, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "kinematicWeldSolref", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoKinematicWeldSolimp, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "kinematicWeldSolimp", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_mujocoKinematicWeldTorqueScale, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "kinematicWeldTorquescale", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_GROUP_dynamics, DYNAMICSPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propDynCont_GROUP_dynamicsBullet, DYNAMICSPREFIXDOT BULLETPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propDynCont_GROUP_dynamicsOde, DYNAMICSPREFIXDOT ODEPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propDynCont_GROUP_dynamicsNewton, DYNAMICSPREFIXDOT NEWTONPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propDynCont_GROUP_dynamicsVortex, DYNAMICSPREFIXDOT VORTEXPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propDynCont_GROUP_dynamicsMujoco, DYNAMICSPREFIXDOT MUJOCOPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    /* Following for backward compatibility: */ \
    FUNCX(propDynCont_DEPRECATED_dynamicsEnabled, "dynamicsEnabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dynamics enabled"}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_showContactPoints, "showContactPoints", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", "Show contact points"}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_dynamicsEngine, "dynamicsEngine", sim_propertytype_intarray, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dynamics engine"}, {"description", "Selected dynamics engine index and version"}}), "") \
    FUNCX(propDynCont_DEPRECATED_dynamicsStepSize, "dynamicsStepSize", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dynamics dt"}, {"description", "Dynamics step size"}}), "") \
    FUNCX(propDynCont_DEPRECATED_gravity, "gravity", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", "Gravity"}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_engineProperties, "engineProperties", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", "Engine properties"}, {"description", "Engine properties as JSON text"}}), "") \
    FUNCX(propDynCont_DEPRECATED_bulletSolver, BULLETPREFIXDOT "solver", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_global_constraintsolvertype, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_bulletIterations, BULLETPREFIXDOT "iterations", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_global_constraintsolvingiterations, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_bulletComputeInertias, BULLETPREFIXDOT "computeInertias", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_global_computeinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_bulletInternalScalingFull, BULLETPREFIXDOT "internalScalingFull", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_global_fullinternalscaling, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_bulletInternalScalingScaling, BULLETPREFIXDOT "internalScalingValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_global_internalscalingfactor, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_bulletCollMarginScaling, BULLETPREFIXDOT "collisionMarginScaling", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_global_collisionmarginfactor, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_odeQuickStepEnabled, ODEPREFIXDOT "quickStepEnabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_ode_global_quickstep, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_odeQuickStepIterations, ODEPREFIXDOT "quickStepIterations", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_ode_global_constraintsolvingiterations, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_odeComputeInertias, ODEPREFIXDOT "computeInertias", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_ode_global_computeinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_odeInternalScalingFull, ODEPREFIXDOT "internalScalingFull", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_ode_global_fullinternalscaling, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_odeInternalScalingScaling, ODEPREFIXDOT "internalScalingValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_ode_global_internalscalingfactor, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_odeGlobalErp, ODEPREFIXDOT "globalErp", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_ode_global_erp, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_odeGlobalCfm, ODEPREFIXDOT "globalCfm", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_ode_global_cfm, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_vortexComputeInertias, VORTEXPREFIXDOT "computeInertias", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_global_computeinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_vortexContactTolerance, VORTEXPREFIXDOT "contactTolerance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_global_contacttolerance, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_vortexAutoSleep, VORTEXPREFIXDOT "autoSleep", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_global_autosleep, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_vortexMultithreading, VORTEXPREFIXDOT "multithreading", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_global_multithreading, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_vortexConstraintsLinearCompliance, VORTEXPREFIXDOT "constraintsLinearCompliance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_global_constraintlinearcompliance, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_vortexConstraintsLinearDamping, VORTEXPREFIXDOT "constraintsLinearDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_global_constraintlineardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_vortexConstraintsLinearKineticLoss, VORTEXPREFIXDOT "constraintsLinearKineticLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_global_constraintlinearkineticloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_vortexConstraintsAngularCompliance, VORTEXPREFIXDOT "constraintsAngularCompliance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_global_constraintangularcompliance, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_vortexConstraintsAngularDamping, VORTEXPREFIXDOT "constraintsAngularDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_global_constraintangulardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_vortexConstraintsAngularKineticLoss, VORTEXPREFIXDOT "constraintsAngularKineticLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_global_constraintangularkineticloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_newtonIterations, NEWTONPREFIXDOT "iterations", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_newton_global_constraintsolvingiterations, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_newtonComputeInertias, NEWTONPREFIXDOT "computeInertias", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_newton_global_computeinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_newtonMultithreading, NEWTONPREFIXDOT "multithreading", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_newton_global_multithreading, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_newtonExactSolver, NEWTONPREFIXDOT "exactSolver", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_newton_global_exactsolver, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_newtonHighJointAccuracy, NEWTONPREFIXDOT "highJointAccuracy", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_newton_global_highjointaccuracy, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_newtonContactMergeTolerance, NEWTONPREFIXDOT "contactMergeTolerance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_newton_global_contactmergetolerance, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoIntegrator, MUJOCOPREFIXDOT "integrator", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_integrator, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoSolver, MUJOCOPREFIXDOT "solver", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_solver, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoIterations, MUJOCOPREFIXDOT "iterations", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_iterations, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoRebuildTrigger, MUJOCOPREFIXDOT "rebuildTrigger", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_rebuildtrigger, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoComputeInertias, MUJOCOPREFIXDOT "computeInertias", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_computeinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoMbMemory, MUJOCOPREFIXDOT "mbmemory", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoNjMax, MUJOCOPREFIXDOT "njmax", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_njmax, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", "Deprecated. See mujoco.mbmemory instead."}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoNconMax, MUJOCOPREFIXDOT "nconmax", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_nconmax, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", "Deprecated. See mujoco.mbmemory instead."}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoNstack, MUJOCOPREFIXDOT "nstack", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_nstack, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", "Deprecated. See mujoco.mbmemory instead."}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoCone, MUJOCOPREFIXDOT "cone", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_cone, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoKinematicBodiesOverrideFlags, MUJOCOPREFIXDOT "kinematicBodiesOverrideFlags", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_overridekin, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoKinematicBodiesMass, MUJOCOPREFIXDOT "kinematicBodiesMass", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_kinmass, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoKinematicBodiesInertia, MUJOCOPREFIXDOT "kinematicBodiesInertia", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_kininertia, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoBoundMass, MUJOCOPREFIXDOT "boundMass", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_boundmass, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoBoundInertia, MUJOCOPREFIXDOT "boundInertia", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_boundinertia, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoBalanceInertias, MUJOCOPREFIXDOT "balanceInertias", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_balanceinertias, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoMultithreaded, MUJOCOPREFIXDOT "multithreaded", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_multithreaded, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoMulticcd, MUJOCOPREFIXDOT "multiccd", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_multiccd, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoContactParamsOverride, MUJOCOPREFIXDOT "contactParamsOverride", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_overridecontacts, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoContactParamsMargin, MUJOCOPREFIXDOT "contactParamsMargin", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_overridemargin, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoContactParamsSolref, MUJOCOPREFIXDOT "contactParamsSolref", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_overridesolref1, sim_mujoco_global_overridesolref2, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoContactParamsSolimp, MUJOCOPREFIXDOT "contactParamsSolimp", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_overridesolimp1, sim_mujoco_global_overridesolimp2, sim_mujoco_global_overridesolimp3, sim_mujoco_global_overridesolimp4, sim_mujoco_global_overridesolimp5,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoImpRatio, MUJOCOPREFIXDOT "impratio", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_impratio, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoWind, MUJOCOPREFIXDOT "wind", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_wind1, sim_mujoco_global_wind2, sim_mujoco_global_wind3, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoDensity, MUJOCOPREFIXDOT "density", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_density, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoViscosity, MUJOCOPREFIXDOT "viscosity", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_global_viscosity, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoJacobian, MUJOCOPREFIXDOT "jacobian", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoTolerance, MUJOCOPREFIXDOT "tolerance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoLs_iterations, MUJOCOPREFIXDOT "ls_iterations", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoLs_tolerance, MUJOCOPREFIXDOT "ls_tolerance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoNoslip_iterations, MUJOCOPREFIXDOT "noslip_iterations", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoNoslip_tolerance, MUJOCOPREFIXDOT "noslip_tolerance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoCcd_iterations, MUJOCOPREFIXDOT "ccd_iterations", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoCcd_tolerance, MUJOCOPREFIXDOT "ccd_tolerance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoSdf_iterations, MUJOCOPREFIXDOT "sdf_iterations", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoSdf_initpoints, MUJOCOPREFIXDOT "sdf_initpoints", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoEqualityEnable, MUJOCOPREFIXDOT "equalityEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoFrictionlossEnable, MUJOCOPREFIXDOT "frictionlossEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoLimitEnable, MUJOCOPREFIXDOT "limitEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoContactEnable, MUJOCOPREFIXDOT "contactEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoPassiveEnable, MUJOCOPREFIXDOT "passiveEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoGravityEnable, MUJOCOPREFIXDOT "gravityEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoWarmstartEnable, MUJOCOPREFIXDOT "warmstartEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoActuationEnable, MUJOCOPREFIXDOT "actuationEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoRefsafeEnable, MUJOCOPREFIXDOT "refsafeEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoSensorEnable, MUJOCOPREFIXDOT "sensorEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoMidphaseEnable, MUJOCOPREFIXDOT "midphaseEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoEulerdampEnable, MUJOCOPREFIXDOT "eulerdampEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoAutoresetEnable, MUJOCOPREFIXDOT "autoresetEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoEnergyEnable, MUJOCOPREFIXDOT "energyEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoInvdiscreteEnable, MUJOCOPREFIXDOT "invdiscreteEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoNativeccdEnable, MUJOCOPREFIXDOT "nativeccdEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoAlignfree, MUJOCOPREFIXDOT "alignfree", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoKinematicWeldSolref, MUJOCOPREFIXDOT "kinematicWeldSolref", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoKinematicWeldSolimp, MUJOCOPREFIXDOT "kinematicWeldSolimp", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDynCont_DEPRECATED_mujocoKinematicWeldTorqueScale, MUJOCOPREFIXDOT "kinematicWeldTorquescale", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "")

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
    FUNCX(propScene_GROUP_customData, CUSTOMDATAPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    FUNCX(propScene_GROUP_signal, SIGNALPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    FUNCX(propScene_METHOD_createObject, "createObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_removeObjects, "removeObjects", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_duplicateObjects, "duplicateObjects", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_handleSimulationScripts, "handleSimulationScripts", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_handleCustomizationScripts, "handleCustomizationScripts", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_getDescendants, "getDescendants", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_getObjects, "getObjects", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_loadModel, "loadModel", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_loadModelFromBuffer, "loadModelFromBuffer", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_save, "save", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_saveToBuffer, "saveToBuffer", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_getObject, "getObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_remove, "remove", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_announceChange, "announceChange", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_getObjectFromUid, "getObjectFromUid", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_startSimulation, SIMULATIONPREFIXDOT "start", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_pauseSimulation, SIMULATIONPREFIXDOT "pause", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_stopSimulation, SIMULATIONPREFIXDOT "stop", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_groupShapes, "groupShapes", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_mergeShapes, "mergeShapes", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_getContacts, "getContacts", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propScene_METHOD_stepDynamics, DYNAMICSPREFIXDOT "step", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propScene_DEPRECATED_sceneIsLocked, "sceneIsLocked", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,  "", "") \
    FUNCX(propScene_DEPRECATED_sceneUid, "sceneUid", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,  "", "") \
    FUNCX(propScene_DEPRECATED_sceneUidString, "sceneUidString", sim_propertytype_string,SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,  "", "") \
    FUNCX(propScene_DEPRECATED_scenePath, "scenePath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED,  "", "")

#define SIMULATION_PROPERTIES \
    FUNCX(propSimulation_removeNewObjectsAtEnd, SIMULATIONPREFIXDOT "removeNewObjects", sim_propertytype_bool, 0,  jsonStr({{"label", "Remove new objects"}, {"description", "Remove new scene objects at simulation end"}}), "") \
    FUNCX(propSimulation_realtimeSimulation, SIMULATIONPREFIXDOT "realtime", sim_propertytype_bool, 0,  jsonStr({{"label", "Real-time simulation"}, {"description", ""}}), "") \
    FUNCX(propSimulation_pauseSimulationAtTime, SIMULATIONPREFIXDOT "pauseAtTime", sim_propertytype_bool, 0,  jsonStr({{"label", "Pause simulation at time"}, {"description", "Pause simulation when simulation time exceeds a threshold"}}), "") \
    FUNCX(propSimulation_pauseSimulationAtError, SIMULATIONPREFIXDOT "pauseAtError", sim_propertytype_bool, 0,  jsonStr({{"label", "Pause simulation on script error"}, {"description", ""}}), "") \
    FUNCX(propSimulation_simulationTime, SIMULATIONPREFIXDOT "time", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Simulation time"}, {"description", ""}}), "") \
    FUNCX(propSimulation_timeStep, SIMULATIONPREFIXDOT "timeStep", sim_propertytype_float, 0,  jsonStr({{"label", "Simulation dt"}, {"description", "Simulation time step"}}), "") \
    FUNCX(propSimulation_timeToPause, SIMULATIONPREFIXDOT "timeToPause", sim_propertytype_float, 0,  jsonStr({{"label", "Simulation pause time"}, {"description", "Time at which simulation should pause"}}), "") \
    FUNCX(propSimulation_realtimeModifier, SIMULATIONPREFIXDOT "realtimeModifier", sim_propertytype_float, 0,  jsonStr({{"label", "Real-time modifier"}, {"description", "Real-time multiplication factor"}}), "") \
    FUNCX(propSimulation_stepCount, SIMULATIONPREFIXDOT "stepCount", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Simulation steps"}, {"description", "Counter of simulation steps"}}), "") \
    FUNCX(propSimulation_simulationState, SIMULATIONPREFIXDOT "state", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Simulation state"}, {"description", ""}}), "") \
    FUNCX(propSimulation_stepsPerRendering, SIMULATIONPREFIXDOT "stepsPerRendering", sim_propertytype_int, 0,  jsonStr({{"label", "Steps per frame"}, {"description", "Simulation steps per frame"}}), "") \
    FUNCX(propSimulation_speedModifier, SIMULATIONPREFIXDOT "speedModifier", sim_propertytype_int, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Speed modifier"}, {"description", ""}}), "") \
    FUNCX(propSimulation_GROUP_simulation, SIMULATIONPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    /* Following for backward compatibility: */ \
    FUNCX(propSimulation_DEPRECATED_removeNewObjectsAtEnd, "removeNewObjectsAtEnd", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSimulation_DEPRECATED_realtimeSimulation, "realtimeSimulation", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSimulation_DEPRECATED_pauseSimulationAtTime, "pauseSimulationAtTime", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSimulation_DEPRECATED_pauseSimulationAtError, "pauseSimulationAtError", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSimulation_DEPRECATED_simulationTime, "simulationTime", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propSimulation_DEPRECATED_timeStep, "timeStep", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSimulation_DEPRECATED_timeToPause, "timeToPause", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSimulation_DEPRECATED_realtimeModifier, "realtimeModifier", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSimulation_DEPRECATED_stepCount, "stepCount", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propSimulation_DEPRECATED_simulationState, "simulationState", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propSimulation_DEPRECATED_stepsPerRendering, "stepsPerRendering", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSimulation_DEPRECATED_speedModifier, "speedModifier", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, "", "")


#define MESHWRAPPER_PROPERTIES \
    FUNCX(propMeshWrapper_mass, "mass", sim_propertytype_float, 0,  jsonStr({{"label", "Mass"}, {"description", ""}}), "") \
    FUNCX(propMeshWrapper_com, "centerOfMass", sim_propertytype_vector3, 0,  jsonStr({{"label", "Center of mass"}, {"description", "Center of mass, relative to the shape's reference frame"}}), "") \
    FUNCX(propMeshWrapper_inertiaMatrix, "inertiaMatrix", sim_propertytype_matrix, 0,  jsonStr({{"label", "Inertia matrix"}, {"description", "Inertia matrix, relative to the shape's reference frame"}}), "") \
    FUNCX(propMeshWrapper_pmi, "pmi", sim_propertytype_vector3, sim_propertyinfo_notwritable,  jsonStr({{"label", "Principal moment of inertia"}, {"description", "Principal moment of inertia, relative to pmiQuaternion"}}), "") \
    FUNCX(propMeshWrapper_pmiQuaternion, "pmiQuaternion", sim_propertytype_quaternion, sim_propertyinfo_notwritable,  jsonStr({{"label", "Quaternion of the principal moment of inertia"}, {"description", "Quaternion of the principal moment of inertia, relative to the shape's reference frame"}}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propMeshWrapper_DEPRECATED_pmi, "principalMomentOfInertia", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "","") \
    FUNCX(propMeshWrapper_DEPRECATED_inertia, "inertia", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, "", "")

#define MESH_PROPERTIES \
    FUNCX(propMesh_textureResolution, TEXTUREPREFIXDOT "resolution", sim_propertytype_intarray2, sim_propertyinfo_notwritable,  jsonStr({{"label", "Texture resolution"}, {"description", ""}}), "") \
    FUNCX(propMesh_textureCoordinates, TEXTUREPREFIXDOT "coordinates", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Texture coordinates"}, {"description", ""}}), "") \
    FUNCX(propMesh_textureApplyMode, TEXTUREPREFIXDOT "applyMode", sim_propertytype_int, 0,  jsonStr({{"label", "Texture apply mode"}, {"description", ""}}), "") \
    FUNCX(propMesh_textureRepeatU, TEXTUREPREFIXDOT "repeatU", sim_propertytype_bool, 0,  jsonStr({{"label", "Texture repeat U"}, {"description", ""}}), "") \
    FUNCX(propMesh_textureRepeatV, TEXTUREPREFIXDOT "repeatV", sim_propertytype_bool, 0,  jsonStr({{"label", "Texture repeat V"}, {"description", ""}}), "") \
    FUNCX(propMesh_textureInterpolate, TEXTUREPREFIXDOT "interpolate", sim_propertytype_bool, 0,  jsonStr({{"label", "Interpolate texture"}, {"description", ""}}), "") \
    FUNCX(propMesh_texture, TEXTUREPREFIXDOT "data", sim_propertytype_buffer, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Texture"}, {"description", ""}}), "") \
    FUNCX(propMesh_textureID, TEXTUREPREFIXDOT "id", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Texture ID"}, {"description", ""}}), "") \
    FUNCX(propMesh_vertices, "vertices", sim_propertytype_matrix, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Vertices"}, {"description", ""}}), "") \
    FUNCX(propMesh_indices, "indices", sim_propertytype_intarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Indices"}, {"description", "Indices (3 values per triangle)"}}), "") \
    FUNCX(propMesh_normals, "normals", sim_propertytype_matrix, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Normals"}, {"description", "Normals (3*3 values per triangle)"}}), "") \
    FUNCX(propMesh_shadingAngle, "shadingAngle", sim_propertytype_float, 0,  jsonStr({{"label", "Shading angle"}, {"description", ""}}), "") \
    FUNCX(propMesh_showEdges, "showEdges", sim_propertytype_bool, 0,  jsonStr({{"label", "Visible edges"}, {"description", ""}}), "") \
    FUNCX(propMesh_culling, "culling", sim_propertytype_bool, 0,  jsonStr({{"label", "Backface culling"}, {"description", ""}}), "") \
    FUNCX(propMesh_wireframe, "wireframe", sim_propertytype_bool, 0,  jsonStr({{"label", "Wireframe"}, {"description", ""}}), "") \
    FUNCX(propMesh_shapeUid, "shapeUid", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Shape UID"}, {"description", "Unique identifier of parent shape"}}), "") \
    FUNCX(propMesh_shape, "shape", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Shape handle"}, {"description", ""}, {"handleType", "shape"}}), "") \
    FUNCX(propMesh_primitiveType, "primitiveType", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Primitive type"}, {"description", ""}}), "") \
    FUNCX(propMesh_convex, "convex", sim_propertytype_bool, sim_propertyinfo_notwritable,  jsonStr({{"label", "Convex"}, {"description", "Whether mesh is convex or not"}}), "") \
    FUNCX(propMesh_colorName, "colorName", sim_propertytype_string, 0,  jsonStr({{"label", "Color name"}, {"description", ""}}), "") \
    FUNCX(propScene_METHOD_textureSetData, TEXTUREPREFIXDOT "setData", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propMesh_GROUP_texture, TEXTUREPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    /* Following for backward compatibility: */ \
    FUNCX(propMesh_DEPRECATED_textureResolution, "textureResolution", sim_propertytype_intarray2, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propMesh_DEPRECATED_textureCoordinates, "textureCoordinates", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propMesh_DEPRECATED_textureApplyMode, "textureApplyMode", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propMesh_DEPRECATED_textureRepeatU, "textureRepeatU", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propMesh_DEPRECATED_textureRepeatV, "textureRepeatV", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propMesh_DEPRECATED_textureInterpolate, "textureInterpolate", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propMesh_DEPRECATED_texture, "rawTexture", sim_propertytype_buffer, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propMesh_DEPRECATED_textureID, "textureID", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "", "")

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
    FUNCX(propSceneObjectCont_DEPRECATED_objectHandles, "objectHandles", sim_propertytype_intarray, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propSceneObjectCont_DEPRECATED_orphanHandles, "orphanHandles", sim_propertytype_intarray, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propSceneObjectCont_DEPRECATED_selectionHandles, "selectionHandles", sim_propertytype_intarray, SIM_PROPERTYINFO_DEPRECATED, "", "")

#define DYNMATERIAL_PROPERTIES \
    FUNCX(propMaterial_engineProperties, DYNAMICSPREFIXDOT "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Engine properties"}, {"description", "Engine properties as JSON text"}}), "") \
    FUNCX(propMaterial_bulletRestitution, DYNAMICSPREFIXDOT BULLETPREFIXDOT "restitution", sim_propertytype_float, 0, sim_bullet_body_restitution, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletFriction0, DYNAMICSPREFIXDOT BULLETPREFIXDOT "frictionOld", sim_propertytype_float, 0, sim_bullet_body_oldfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletFriction, DYNAMICSPREFIXDOT BULLETPREFIXDOT "friction", sim_propertytype_float, 0, sim_bullet_body_friction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletLinearDamping, DYNAMICSPREFIXDOT BULLETPREFIXDOT "linearDamping", sim_propertytype_float, 0, sim_bullet_body_lineardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletAngularDamping, DYNAMICSPREFIXDOT BULLETPREFIXDOT "angularDamping", sim_propertytype_float, 0, sim_bullet_body_angulardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletNonDefaultCollisionMarginFactor, DYNAMICSPREFIXDOT BULLETPREFIXDOT "customCollisionMarginValue", sim_propertytype_float, 0, sim_bullet_body_nondefaultcollisionmargingfactor, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletNonDefaultCollisionMarginFactorConvex, DYNAMICSPREFIXDOT BULLETPREFIXDOT "customCollisionMarginConvexValue", sim_propertytype_float, 0, sim_bullet_body_nondefaultcollisionmargingfactorconvex, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletSticky, DYNAMICSPREFIXDOT BULLETPREFIXDOT "stickyContact", sim_propertytype_bool, 0, sim_bullet_body_sticky, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletNonDefaultCollisionMargin, DYNAMICSPREFIXDOT BULLETPREFIXDOT "customCollisionMarginEnabled", sim_propertytype_bool, 0, sim_bullet_body_usenondefaultcollisionmargin, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletNonDefaultCollisionMarginConvex, DYNAMICSPREFIXDOT BULLETPREFIXDOT "customCollisionMarginConvexEnabled", sim_propertytype_bool, 0, sim_bullet_body_usenondefaultcollisionmarginconvex, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_bulletAutoShrinkConvex, DYNAMICSPREFIXDOT BULLETPREFIXDOT "autoShrinkConvexMeshes", sim_propertytype_bool, 0, sim_bullet_body_autoshrinkconvex, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_odeFriction, DYNAMICSPREFIXDOT ODEPREFIXDOT "friction", sim_propertytype_float, 0, sim_ode_body_friction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_odeSoftErp, DYNAMICSPREFIXDOT ODEPREFIXDOT "softErp", sim_propertytype_float, 0, sim_ode_body_softerp, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_odeSoftCfm, DYNAMICSPREFIXDOT ODEPREFIXDOT "softCfm", sim_propertytype_float, 0, sim_ode_body_softcfm, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_odeLinearDamping, DYNAMICSPREFIXDOT ODEPREFIXDOT "linearDamping", sim_propertytype_float, 0, sim_ode_body_lineardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_odeAngularDamping, DYNAMICSPREFIXDOT ODEPREFIXDOT "angularDamping", sim_propertytype_float, 0, sim_ode_body_angulardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_odeMaxContacts, DYNAMICSPREFIXDOT ODEPREFIXDOT "maxContacts", sim_propertytype_int, 0, sim_ode_body_maxcontacts, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryLinearAxisFriction, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearPrimaryAxisFrictionValue", sim_propertytype_float, 0, sim_vortex_body_primlinearaxisfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisFriction, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearSecondaryAxisFrictionValue", sim_propertytype_float, 0, sim_vortex_body_seclinearaxisfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryAngularAxisFriction, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularPrimaryAxisFrictionValue", sim_propertytype_float, 0, sim_vortex_body_primangularaxisfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisFriction, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularSecondaryAxisFrictionValue", sim_propertytype_float, 0, sim_vortex_body_secangularaxisfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexNormalAngularAxisFriction, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularNormalAxisFrictionValue", sim_propertytype_float, 0, sim_vortex_body_normalangularaxisfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryLinearAxisStaticFrictionScale, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearPrimaryAxisStaticFrictionScale", sim_propertytype_float, 0, sim_vortex_body_primlinearaxisstaticfrictionscale, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisStaticFrictionScale, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearSecondaryAxisStaticFrictionScale", sim_propertytype_float, 0, sim_vortex_body_seclinearaxisstaticfrictionscale, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryAngularAxisStaticFrictionScale, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularPrimaryAxisStaticFrictionScale", sim_propertytype_float, 0, sim_vortex_body_primangularaxisstaticfrictionscale, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisStaticFrictionScale, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularSecondaryAxisStaticFrictionScale", sim_propertytype_float, 0, sim_vortex_body_secangularaxisstaticfrictionscale, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexNormalAngularAxisStaticFrictionScale, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularNormalAxisStaticFrictionScale", sim_propertytype_float, 0, sim_vortex_body_normalangularaxisstaticfrictionscale, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexCompliance, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "compliance", sim_propertytype_float, 0, sim_vortex_body_compliance, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "damping", sim_propertytype_float, 0, sim_vortex_body_damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexRestitution, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "restitution", sim_propertytype_float, 0, sim_vortex_body_restitution, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexRestitutionThreshold, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "restitutionThreshold", sim_propertytype_float, 0, sim_vortex_body_restitutionthreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAdhesiveForce, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "adhesiveForce", sim_propertytype_float, 0, sim_vortex_body_adhesiveforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexLinearVelocityDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearVelDamping", sim_propertytype_float, 0, sim_vortex_body_linearvelocitydamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAngularVelocityDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularVelDamping", sim_propertytype_float, 0, sim_vortex_body_angularvelocitydamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryLinearAxisSlide, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearPrimaryAxisSlide", sim_propertytype_float, 0, sim_vortex_body_primlinearaxisslide, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisSlide, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearSecondaryAxisSlide", sim_propertytype_float, 0, sim_vortex_body_seclinearaxisslide, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryAngularAxisSlide, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularPrimaryAxisSlide", sim_propertytype_float, 0, sim_vortex_body_primangularaxisslide, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisSlide, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularSecondaryAxisSlide", sim_propertytype_float, 0, sim_vortex_body_secangularaxisslide, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexNormalAngularAxisSlide, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularNormalAxisSlide", sim_propertytype_float, 0, sim_vortex_body_normalangularaxisslide, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryLinearAxisSlip, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearPrimaryAxisSlip", sim_propertytype_float, 0, sim_vortex_body_primlinearaxisslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisSlip, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearSecondaryAxisSlip", sim_propertytype_float, 0, sim_vortex_body_seclinearaxisslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryAngularAxisSlip, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularPrimaryAxisSlip", sim_propertytype_float, 0, sim_vortex_body_primangularaxisslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisSlip, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularSecondaryAxisSlip", sim_propertytype_float, 0, sim_vortex_body_secangularaxisslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexNormalAngularAxisSlip, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularNormalAxisSlip", sim_propertytype_float, 0, sim_vortex_body_normalangularaxisslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAutoSleepLinearSpeedThreshold, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoSleepThresholdLinearSpeed", sim_propertytype_float, 0, sim_vortex_body_autosleeplinearspeedthreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAutoSleepLinearAccelerationThreshold, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoSleepThresholdLinearAccel", sim_propertytype_float, 0, sim_vortex_body_autosleeplinearaccelthreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAutoSleepAngularSpeedThreshold, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoSleepThresholdAngularSpeed", sim_propertytype_float, 0, sim_vortex_body_autosleepangularspeedthreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAutoSleepAngularAccelerationThreshold, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoSleepThresholdAngularAccel", sim_propertytype_float, 0, sim_vortex_body_autosleepangularaccelthreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSkinThickness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "skinThickness", sim_propertytype_float, 0, sim_vortex_body_skinthickness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAutoAngularDampingTensionRatio, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoAngularDampingTensionRatio", sim_propertytype_float, 0, sim_vortex_body_autoangulardampingtensionratio, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryAxisVector, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearPrimaryValue", sim_propertytype_vector3, 0, sim_vortex_body_primaxisvectorx, sim_vortex_body_primaxisvectory, sim_vortex_body_primaxisvectorz, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryLinearAxisFrictionModel, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearPrimaryAxisFrictionModel", sim_propertytype_int, 0, sim_vortex_body_primlinearaxisfrictionmodel, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisFrictionModel, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearSecondaryAxisFrictionModel", sim_propertytype_int, 0, sim_vortex_body_seclinearaxisfrictionmodel, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimaryAngularAxisFrictionModel, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularPrimaryAxisFrictionModel", sim_propertytype_int, 0, sim_vortex_body_primangulararaxisfrictionmodel, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisFrictionModel, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularSecondaryAxisFrictionModel", sim_propertytype_int, 0, sim_vortex_body_secangularaxisfrictionmodel, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexNormalAngularAxisFrictionModel, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularNormalAxisFrictionModel", sim_propertytype_int, 0, sim_vortex_body_normalangularaxisfrictionmodel, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAutoSleepStepLiveThreshold, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoSleepThresholdSteps", sim_propertytype_int, 0, sim_vortex_body_autosleepsteplivethreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexMaterialUniqueId, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "materialUniqueId", sim_propertytype_int, sim_propertyinfo_modelhashexclude, sim_vortex_body_materialuniqueid, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexPrimitiveShapesAsConvex, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "primitiveAsConvex", sim_propertytype_bool, 0, sim_vortex_body_pureshapesasconvex, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexConvexShapesAsRandom, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "convexAsRandom", sim_propertytype_bool, 0, sim_vortex_body_convexshapesasrandom, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexRandomShapesAsTerrain, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "randomAsTerrain", sim_propertytype_bool, 0, sim_vortex_body_randomshapesasterrain, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexFastMoving, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "fastMoving", sim_propertytype_bool, 0, sim_vortex_body_fastmoving, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAutoSlip, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoSlip", sim_propertytype_bool, 0, sim_vortex_body_autoslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisSameAsPrimaryLinearAxis, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearSecondaryAxisFollowPrimaryAxis", sim_propertytype_bool, 0, sim_vortex_body_seclinaxissameasprimlinaxis, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisSameAsPrimaryAngularAxis, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularSecondaryAxisFollowPrimaryAxis", sim_propertytype_bool, 0, sim_vortex_body_secangaxissameasprimangaxis, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexNormalAngularAxisSameAsPrimaryAngularAxis, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularNormalAxisFollowPrimaryAxis", sim_propertytype_bool, 0, sim_vortex_body_normangaxissameasprimangaxis, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_vortexAutoAngularDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoAngularDampingEnabled", sim_propertytype_bool, 0, sim_vortex_body_autoangulardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_newtonStaticFriction, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "staticFriction", sim_propertytype_float, 0, sim_newton_body_staticfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_newtonKineticFriction, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "kineticFriction", sim_propertytype_float, 0, sim_newton_body_kineticfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_newtonRestitution, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "restitution", sim_propertytype_float, 0, sim_newton_body_restitution, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_newtonLinearDrag, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "linearDrag", sim_propertytype_float, 0, sim_newton_body_lineardrag, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_newtonAngularDrag, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "angularDrag", sim_propertytype_float, 0, sim_newton_body_angulardrag, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_newtonFastMoving, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "fastMoving", sim_propertytype_bool, 0, sim_newton_body_fastmoving, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoFriction, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "friction", sim_propertytype_floatarray, 0, sim_mujoco_body_friction1, sim_mujoco_body_friction2, sim_mujoco_body_friction3, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoSolref, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "solref", sim_propertytype_floatarray, 0, sim_mujoco_body_solref1, sim_mujoco_body_solref2, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoSolimp, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "solimp", sim_propertytype_floatarray, 0, sim_mujoco_body_solimp1, sim_mujoco_body_solimp2, sim_mujoco_body_solimp3, sim_mujoco_body_solimp4, sim_mujoco_body_solimp5,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoSolmix, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "solmix", sim_propertytype_float, 0, sim_mujoco_body_solmix, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoMargin, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "margin", sim_propertytype_float, 0, sim_mujoco_body_margin, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoGap, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "gap", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoCondim, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "condim", sim_propertytype_int, 0, sim_mujoco_body_condim, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoPriority, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "priority", sim_propertytype_int, 0, sim_mujoco_body_priority, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoAdhesion, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "adhesion", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoAdhesionGain, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "adhesiongain", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoAdhesionForcelimited, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "adhesionforcelimited", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoAdhesionCtrlrange, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "adhesionctrlrange", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoAdhesionForcerange, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "adhesionforcerange", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoAdhesionCtrl, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "adhesionctrl", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_mujocoGravcomp, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "gravcomp", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_GROUP_dynamics, DYNAMICSPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propMaterial_GROUP_dynamicsBullet, DYNAMICSPREFIXDOT BULLETPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propMaterial_GROUP_dynamicsOde, DYNAMICSPREFIXDOT ODEPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propMaterial_GROUP_dynamicsNewton, DYNAMICSPREFIXDOT NEWTONPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propMaterial_GROUP_dynamicsVortex, DYNAMICSPREFIXDOT VORTEXPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propMaterial_GROUP_dynamicsMujoco, DYNAMICSPREFIXDOT MUJOCOPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    /* Following for backward compatibility: */ \
    FUNCX(propMaterial_DEPRECATED_engineProperties, "engineProperties", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", "Engine properties"}, {"description", "Engine properties as JSON text"}}), "") \
    FUNCX(propMaterial_DEPRECATED_bulletRestitution, BULLETPREFIXDOT "restitution", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_body_restitution, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_bulletFriction0, BULLETPREFIXDOT "frictionOld", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_body_oldfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_bulletFriction, BULLETPREFIXDOT "friction", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_body_friction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_bulletLinearDamping, BULLETPREFIXDOT "linearDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_body_lineardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_bulletAngularDamping, BULLETPREFIXDOT "angularDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_body_angulardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_bulletNonDefaultCollisionMarginFactor, BULLETPREFIXDOT "customCollisionMarginValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_body_nondefaultcollisionmargingfactor, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_bulletNonDefaultCollisionMarginFactorConvex, BULLETPREFIXDOT "customCollisionMarginConvexValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_body_nondefaultcollisionmargingfactorconvex, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_bulletSticky, BULLETPREFIXDOT "stickyContact", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_body_sticky, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_bulletNonDefaultCollisionMargin, BULLETPREFIXDOT "customCollisionMarginEnabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_body_usenondefaultcollisionmargin, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_bulletNonDefaultCollisionMarginConvex, BULLETPREFIXDOT "customCollisionMarginConvexEnabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_body_usenondefaultcollisionmarginconvex, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_bulletAutoShrinkConvex, BULLETPREFIXDOT "autoShrinkConvexMeshes", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_body_autoshrinkconvex, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_odeFriction, ODEPREFIXDOT "friction", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_ode_body_friction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_odeSoftErp, ODEPREFIXDOT "softErp", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_ode_body_softerp, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_odeSoftCfm, ODEPREFIXDOT "softCfm", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_ode_body_softcfm, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_odeLinearDamping, ODEPREFIXDOT "linearDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_ode_body_lineardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_odeAngularDamping, ODEPREFIXDOT "angularDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_ode_body_angulardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_odeMaxContacts, ODEPREFIXDOT "maxContacts", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_ode_body_maxcontacts, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexPrimaryLinearAxisFriction, VORTEXPREFIXDOT "linearPrimaryAxisFrictionValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_primlinearaxisfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexSecondaryLinearAxisFriction, VORTEXPREFIXDOT "linearSecondaryAxisFrictionValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_seclinearaxisfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexPrimaryAngularAxisFriction, VORTEXPREFIXDOT "angularPrimaryAxisFrictionValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_primangularaxisfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexSecondaryAngularAxisFriction, VORTEXPREFIXDOT "angularSecondaryAxisFrictionValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_secangularaxisfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexNormalAngularAxisFriction, VORTEXPREFIXDOT "angularNormalAxisFrictionValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_normalangularaxisfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexPrimaryLinearAxisStaticFrictionScale, VORTEXPREFIXDOT "linearPrimaryAxisStaticFrictionScale", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_primlinearaxisstaticfrictionscale, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexSecondaryLinearAxisStaticFrictionScale, VORTEXPREFIXDOT "linearSecondaryAxisStaticFrictionScale", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_seclinearaxisstaticfrictionscale, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexPrimaryAngularAxisStaticFrictionScale, VORTEXPREFIXDOT "angularPrimaryAxisStaticFrictionScale", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_primangularaxisstaticfrictionscale, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexSecondaryAngularAxisStaticFrictionScale, VORTEXPREFIXDOT "angularSecondaryAxisStaticFrictionScale", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_secangularaxisstaticfrictionscale, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexNormalAngularAxisStaticFrictionScale, VORTEXPREFIXDOT "angularNormalAxisStaticFrictionScale", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_normalangularaxisstaticfrictionscale, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexCompliance, VORTEXPREFIXDOT "compliance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_compliance, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexDamping, VORTEXPREFIXDOT "damping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexRestitution, VORTEXPREFIXDOT "restitution", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_restitution, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexRestitutionThreshold, VORTEXPREFIXDOT "restitutionThreshold", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_restitutionthreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexAdhesiveForce, VORTEXPREFIXDOT "adhesiveForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_adhesiveforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexLinearVelocityDamping, VORTEXPREFIXDOT "linearVelDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_linearvelocitydamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexAngularVelocityDamping, VORTEXPREFIXDOT "angularVelDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_angularvelocitydamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexPrimaryLinearAxisSlide, VORTEXPREFIXDOT "linearPrimaryAxisSlide", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_primlinearaxisslide, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexSecondaryLinearAxisSlide, VORTEXPREFIXDOT "linearSecondaryAxisSlide", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_seclinearaxisslide, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexPrimaryAngularAxisSlide, VORTEXPREFIXDOT "angularPrimaryAxisSlide", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_primangularaxisslide, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexSecondaryAngularAxisSlide, VORTEXPREFIXDOT "angularSecondaryAxisSlide", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_secangularaxisslide, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexNormalAngularAxisSlide, VORTEXPREFIXDOT "angularNormalAxisSlide", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_normalangularaxisslide, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexPrimaryLinearAxisSlip, VORTEXPREFIXDOT "linearPrimaryAxisSlip", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_primlinearaxisslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexSecondaryLinearAxisSlip, VORTEXPREFIXDOT "linearSecondaryAxisSlip", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_seclinearaxisslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexPrimaryAngularAxisSlip, VORTEXPREFIXDOT "angularPrimaryAxisSlip", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_primangularaxisslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexSecondaryAngularAxisSlip, VORTEXPREFIXDOT "angularSecondaryAxisSlip", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_secangularaxisslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexNormalAngularAxisSlip, VORTEXPREFIXDOT "angularNormalAxisSlip", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_normalangularaxisslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexAutoSleepLinearSpeedThreshold, VORTEXPREFIXDOT "autoSleepThresholdLinearSpeed", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_autosleeplinearspeedthreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexAutoSleepLinearAccelerationThreshold, VORTEXPREFIXDOT "autoSleepThresholdLinearAccel", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_autosleeplinearaccelthreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexAutoSleepAngularSpeedThreshold, VORTEXPREFIXDOT "autoSleepThresholdAngularSpeed", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_autosleepangularspeedthreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexAutoSleepAngularAccelerationThreshold, VORTEXPREFIXDOT "autoSleepThresholdAngularAccel", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_autosleepangularaccelthreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexSkinThickness, VORTEXPREFIXDOT "skinThickness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_skinthickness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexAutoAngularDampingTensionRatio, VORTEXPREFIXDOT "autoAngularDampingTensionRatio", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_autoangulardampingtensionratio, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexPrimaryAxisVector, VORTEXPREFIXDOT "linearPrimaryValue", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_primaxisvectorx, sim_vortex_body_primaxisvectory, sim_vortex_body_primaxisvectorz, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexPrimaryLinearAxisFrictionModel, VORTEXPREFIXDOT "linearPrimaryAxisFrictionModel", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_primlinearaxisfrictionmodel, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexSecondaryLinearAxisFrictionModel, VORTEXPREFIXDOT "linearSecondaryAxisFrictionModel", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_seclinearaxisfrictionmodel, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexPrimaryAngularAxisFrictionModel, VORTEXPREFIXDOT "angularPrimaryAxisFrictionModel", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_primangulararaxisfrictionmodel, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexSecondaryAngularAxisFrictionModel, VORTEXPREFIXDOT "angularSecondaryAxisFrictionModel", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_secangularaxisfrictionmodel, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexNormalAngularAxisFrictionModel, VORTEXPREFIXDOT "angularNormalAxisFrictionModel", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_normalangularaxisfrictionmodel, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexAutoSleepStepLiveThreshold, VORTEXPREFIXDOT "autoSleepThresholdSteps", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_autosleepsteplivethreshold, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexMaterialUniqueId, VORTEXPREFIXDOT "materialUniqueId", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_materialuniqueid, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexPrimitiveShapesAsConvex, VORTEXPREFIXDOT "primitiveAsConvex", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_pureshapesasconvex, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexConvexShapesAsRandom, VORTEXPREFIXDOT "convexAsRandom", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_convexshapesasrandom, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexRandomShapesAsTerrain, VORTEXPREFIXDOT "randomAsTerrain", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_randomshapesasterrain, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexFastMoving, VORTEXPREFIXDOT "fastMoving", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_fastmoving, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexAutoSlip, VORTEXPREFIXDOT "autoSlip", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_autoslip, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexSecondaryLinearAxisSameAsPrimaryLinearAxis, VORTEXPREFIXDOT "linearSecondaryAxisFollowPrimaryAxis", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_seclinaxissameasprimlinaxis, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexSecondaryAngularAxisSameAsPrimaryAngularAxis, VORTEXPREFIXDOT "angularSecondaryAxisFollowPrimaryAxis", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_secangaxissameasprimangaxis, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexNormalAngularAxisSameAsPrimaryAngularAxis, VORTEXPREFIXDOT "angularNormalAxisFollowPrimaryAxis", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_normangaxissameasprimangaxis, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_vortexAutoAngularDamping, VORTEXPREFIXDOT "autoAngularDampingEnabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_body_autoangulardamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_newtonStaticFriction, NEWTONPREFIXDOT "staticFriction", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_newton_body_staticfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_newtonKineticFriction, NEWTONPREFIXDOT "kineticFriction", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_newton_body_kineticfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_newtonRestitution, NEWTONPREFIXDOT "restitution", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_newton_body_restitution, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_newtonLinearDrag, NEWTONPREFIXDOT "linearDrag", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_newton_body_lineardrag, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_newtonAngularDrag, NEWTONPREFIXDOT "angularDrag", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_newton_body_angulardrag, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_newtonFastMoving, NEWTONPREFIXDOT "fastMoving", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_newton_body_fastmoving, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_mujocoFriction, MUJOCOPREFIXDOT "friction", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_body_friction1, sim_mujoco_body_friction2, sim_mujoco_body_friction3, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_mujocoSolref, MUJOCOPREFIXDOT "solref", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_body_solref1, sim_mujoco_body_solref2, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_mujocoSolimp, MUJOCOPREFIXDOT "solimp", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_body_solimp1, sim_mujoco_body_solimp2, sim_mujoco_body_solimp3, sim_mujoco_body_solimp4, sim_mujoco_body_solimp5,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_mujocoSolmix, MUJOCOPREFIXDOT "solmix", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_body_solmix, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_mujocoMargin, MUJOCOPREFIXDOT "margin", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_body_margin, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_mujocoGap, MUJOCOPREFIXDOT "gap", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_mujocoCondim, MUJOCOPREFIXDOT "condim", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_body_condim, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_mujocoPriority, MUJOCOPREFIXDOT "priority", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_body_priority, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_mujocoAdhesion, MUJOCOPREFIXDOT "adhesion", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_mujocoAdhesionGain, MUJOCOPREFIXDOT "adhesiongain", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_mujocoAdhesionForcelimited, MUJOCOPREFIXDOT "adhesionforcelimited", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_mujocoAdhesionCtrlrange, MUJOCOPREFIXDOT "adhesionctrlrange", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_mujocoAdhesionForcerange, MUJOCOPREFIXDOT "adhesionforcerange", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_mujocoAdhesionCtrl, MUJOCOPREFIXDOT "adhesionctrl", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propMaterial_DEPRECATED_mujocoGravcomp, MUJOCOPREFIXDOT "gravcomp", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "")

#define CUSTOMSCENEOBJECTCLASS_PROPERTIES \
    FUNCX(propCustomSceneObjectClass_METHOD_makeOject, "makeObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propCustomSceneObjectClass_METHOD_remove, "remove", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "")

#define SCENEOBJECT_PROPERTIES \
    FUNCX(propSceneObject_modelInvisible, "modelInvisible", sim_propertytype_bool, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Inherited model invisibility"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_modelBase, "modelBase", sim_propertytype_bool, 0,  jsonStr({{"label", "Model base"}, {"description", "Model base flag, indicates the scene object is the base of a model"}}), "") \
    FUNCX(propSceneObject_layer, "layer", sim_propertytype_int, 0,  jsonStr({{"label", "Visibility layer"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_tree, "tree", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Object tree"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_childOrder, "childOrder", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Child order"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_parentUid, "parentUid", sim_propertytype_long, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Parent UID"}, {"description", "Parent scene object unique identifier"}}), "") \
    FUNCX(propSceneObject_uid, "uid", sim_propertytype_long, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Object UID"}, {"description", "Scene object unique identifier"}}), "") \
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
    FUNCX(propSceneObject_modelProperty, MODELPREFIXDOT "propertyFlags", /*redund.*/ sim_propertytype_int, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Model flags"}, {"description", "Model flags, redundant"}}), "") \
    FUNCX(propSceneObject_modelNotCollidable, MODELPREFIXDOT "notCollidable", sim_propertytype_bool, 0,  jsonStr({{"label", "Model not collidable"}, {"description", "Model is not collidable"}}), "") \
    FUNCX(propSceneObject_modelNotMeasurable, MODELPREFIXDOT "notMeasurable", sim_propertytype_bool, 0,  jsonStr({{"label", "Model not measurable"}, {"description", "Model is not measurable"}}), "") \
    FUNCX(propSceneObject_modelNotDetectable, MODELPREFIXDOT "notDetectable", sim_propertytype_bool, 0,  jsonStr({{"label", "Model not detectable"}, {"description", "Model is not detectable"}}), "") \
    FUNCX(propSceneObject_modelNotDynamic, MODELPREFIXDOT "notDynamic", sim_propertytype_bool, 0,  jsonStr({{"label", "Model not dynamic"}, {"description", "Model is not dynamic, i.e. model is static"}}), "") \
    FUNCX(propSceneObject_modelNotRespondable, MODELPREFIXDOT "notRespondable", sim_propertytype_bool, 0,  jsonStr({{"label", "Model not respondable"}, {"description", "Model is not respondable"}}), "") \
    FUNCX(propSceneObject_modelNotVisible, MODELPREFIXDOT "notVisible", sim_propertytype_bool, 0,  jsonStr({{"label", "Model not visible"}, {"description", "Model is not visible"}}), "") \
    FUNCX(propSceneObject_modelScriptsNotActive, MODELPREFIXDOT "scriptsNotActive", sim_propertytype_bool, 0,  jsonStr({{"label", "Model scripts inactive"}, {"description", "Model scripts are not active"}}), "") \
    FUNCX(propSceneObject_modelNotInParentBB, MODELPREFIXDOT "notInParentBB", sim_propertytype_bool, 0,  jsonStr({{"label", "Model invisible to other model's bounding boxes"}, {"description", "Model is invisible to other model's bounding boxes"}}), "") \
    FUNCX(propSceneObject_modelBBSize, MODELPREFIXDOT "bbSize", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Model bounding boxe size"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_modelBBPos, MODELPREFIXDOT "bbPos", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Model bounding boxe position"}, {"description", ""}}), "") \
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
    FUNCX(propSceneObject_movementOptions, MOVPREFIXDOT "optionsFlags", sim_propertytype_int, 0,  jsonStr({{"label", "Movement flags"}, {"description", "Scene object movement flags"}}), "") \
    FUNCX(propSceneObject_movementStepSize, MOVPREFIXDOT "stepSize", sim_propertytype_floatarray, 0,  jsonStr({{"label", "Movement step sizes"}, {"description", "Linear and angular step sizes"}}), "") \
    FUNCX(propSceneObject_movementRelativity, MOVPREFIXDOT "relativity", sim_propertytype_intarray, 0,  jsonStr({{"label", "Movement relativity"}, {"description", "Movement relativity, linear and angular"}}), "") \
    FUNCX(propSceneObject_movementPreferredAxes, MOVPREFIXDOT "preferredAxesFlags", sim_propertytype_int, 0,  jsonStr({{"label", "Preferred axes"}, {"description", "Preferred axes flags (redundant)"}}), "") \
    FUNCX(propSceneObject_movTranslNoSim, MOVPREFIXDOT "translNoSim", sim_propertytype_bool, 0,  jsonStr({{"label", "Translation enabled"}, {"description", "Translation enabled when simulation is not running"}}), "") \
    FUNCX(propSceneObject_movTranslInSim, MOVPREFIXDOT "translInSim", sim_propertytype_bool, 0,  jsonStr({{"label", "Translation enabled during simulation"}, {"description", "Translation enabled when simulation is running"}}), "") \
    FUNCX(propSceneObject_movRotNoSim, MOVPREFIXDOT "rotNoSim", sim_propertytype_bool, 0,  jsonStr({{"label", "Rotation enabled"}, {"description", "Rotation enabled when simulation is not running"}}), "") \
    FUNCX(propSceneObject_movRotInSim, MOVPREFIXDOT "rotInSim", sim_propertytype_bool, 0,  jsonStr({{"label", "Rotation enabled during simulation"}, {"description", "Rotation enabled when simulation is running"}}), "") \
    FUNCX(propSceneObject_movAltTransl, MOVPREFIXDOT "altTransl", sim_propertytype_bool, 0,  jsonStr({{"label", "Alternate translation axes enabled"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_movAltRot, MOVPREFIXDOT "altRot", sim_propertytype_bool, 0,  jsonStr({{"label", "Alternate rotation axes enabled"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_movPrefTranslX, MOVPREFIXDOT "prefTranslX", sim_propertytype_bool, 0,  jsonStr({{"label", "Preferred X-translation"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_movPrefTranslY, MOVPREFIXDOT "prefTranslY", sim_propertytype_bool, 0,  jsonStr({{"label", "Preferred Y-translation"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_movPrefTranslZ, MOVPREFIXDOT "prefTranslZ", sim_propertytype_bool, 0,  jsonStr({{"label", "Preferred Z-translation"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_movPrefRotX, MOVPREFIXDOT "prefRotX", sim_propertytype_bool, 0,  jsonStr({{"label", "Preferred X-axis rotation"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_movPrefRotY, MOVPREFIXDOT "prefRotY", sim_propertytype_bool, 0,  jsonStr({{"label", "Preferred Y-axis rotation"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_movPrefRotZ, MOVPREFIXDOT "prefRotZ", sim_propertytype_bool, 0,  jsonStr({{"label", "Preferred Z-axis rotation"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_visible, "visible", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Visible"}, {"description", "Whether the scene object is currently visible"}}), "") \
    FUNCX(propSceneObject_children, "children", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Children handles"}, {"description", ""}, {"handleType", "sceneObject"}}), "") \
    FUNCX(propSceneObject_modelHash, "modelHash", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Model hash"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_dynamicallyEnabled, "dynamicallyEnabled", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Dynamically enabled"}, {"description", ""}}), "") \
    FUNCX(propSceneObject_GROUP_customData, CUSTOMDATAPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    FUNCX(propSceneObject_GROUP_signal, SIGNALPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    FUNCX(propSceneObject_GROUP_refs, REFSPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    FUNCX(propSceneObject_GROUP_origRefs, ORIGREFSPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    FUNCX(propSceneObject_GROUP_model, MODELPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    FUNCX(propSceneObject_GROUP_mov, MOVPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    FUNCX(propSceneObject_METHOD_getAncestors, "getAncestors", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_getDescendants, "getDescendants", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_removeModel, "removeModel", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_remove, "remove", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_saveModel, "saveModel", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_saveModelToBuffer, "saveModelToBuffer", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_checkCollision, "checkCollision", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_checkDistance, "checkDistance", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_getName, "getName", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_getPose, "getPose", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_getPosition, "getPosition", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_getQuaternion, "getQuaternion", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_setPose, "setPose", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_setPosition, "setPosition", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_setQuaternion, "setQuaternion", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_setParent, "setParent", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_scale, "scale", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_scaleTree, "scaleTree", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_dynamicReset, "dynamicReset", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_visitTree, "visitTree", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_changeColor, "changeColor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_restoreColor, "restoreColor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_makeClass, "makeClass", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_makeObject, "makeObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propSceneObject_METHOD_getObject, "getObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propSceneObject_DEPRECATED_parentHandle, "parentHandle", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_modelProperty, "modelPropertyFlags", /*redund.*/ sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_modelNotCollidable, "modelNotCollidable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_modelNotMeasurable, "modelNotMeasurable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_modelNotDetectable, "modelNotDetectable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_modelNotDynamic, "modelNotDynamic", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_modelNotRespondable, "modelNotRespondable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_modelNotVisible, "modelNotVisible", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_modelScriptsNotActive, "modelScriptsNotActive", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_modelNotInParentBB, "modelNotInParentBB", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_alias, "alias", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_deprecatedName, "deprecatedName", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_bbHsize, "bbHSize", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propSceneObject_DEPRECATED_objectUid, "objectUid", sim_propertytype_long, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propSceneObject_DEPRECATED_movementOptions, "movementOptionsFlags", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_movementStepSize, "movementStepSize", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_movementRelativity, "movementRelativity", sim_propertytype_intarray, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_movementPreferredAxes, "movementPreferredAxesFlags", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_movTranslNoSim, "movTranslNoSim", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_movTranslInSim, "movTranslInSim", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_movRotNoSim, "movRotNoSim", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_movRotInSim, "movRotInSim", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_movAltTransl, "movAltTransl", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_movAltRot, "movAltRot", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_movPrefTranslX, "movPrefTranslX", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_movPrefTranslY, "movPrefTranslY", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_movPrefTranslZ, "movPrefTranslZ", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_movPrefRotX, "movPrefRotX", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_movPrefRotY, "movPrefRotY", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propSceneObject_DEPRECATED_movPrefRotZ, "movPrefRotZ", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, "", "")

#define SHAPE_PROPERTIES \
    FUNCX(propShape_meshes, "meshes", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Meshes"}, {"description", "Mesh handles"}, {"handleType", "mesh"}}), "") \
    FUNCX(propShape_applyCulling, "applyCulling", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Apply culling"}, {"description", "Enables/disables culling for all contained meshes"}}), "") \
    FUNCX(propShape_applyShadingAngle, "applyShadingAngle", sim_propertytype_float, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Apply shading"}, {"description", "Applies a shading angle to all contained meshes"}}), "") \
    FUNCX(propShape_applyShowEdges, "applyShowEdges", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Apply edges"}, {"description", "Enables/disables edges for all contained meshes"}}), "") \
    FUNCX(propShape_flipFaces, "flipFaces", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Flip faces"}, {"description", "Flips faces of all contained meshes"}}), "") \
    FUNCX(propShape_applyColorDiffuse, APPLYCOLORPREFIXDOT "diffuse", sim_propertytype_color, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Apply diffuse color"}, {"description", "Applies the diffuse color component to all contained meshes"}}), "") \
    FUNCX(propShape_applyColorSpecular, APPLYCOLORPREFIXDOT "specular", sim_propertytype_color, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Apply specular color"}, {"description", "Applies the specular color component to all contained meshes"}}), "") \
    FUNCX(propShape_applyColorEmission, APPLYCOLORPREFIXDOT "emission", sim_propertytype_color, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Apply emission color"}, {"description", "Applies the emission color component to all contained meshes"}}), "") \
    FUNCX(propShape_applyColorTransparency, APPLYCOLORPREFIXDOT "transparency", sim_propertytype_float, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Apply transparency"}, {"description", "Applies transparency to all contained meshes"}}), "") \
    FUNCX(propShape_compoundColorDiffuse, COMPOUNDCOLORSPREFIXDOT "diffuse", sim_propertytype_floatarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Diffuse color data"}, {"description", "Diffuse color from all compound elements"}}), "") \
    FUNCX(propShape_compoundColorSpecular, COMPOUNDCOLORSPREFIXDOT "specular", sim_propertytype_floatarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Specular color data"}, {"description", "Specular color from all compound elements"}}), "") \
    FUNCX(propShape_compoundColorEmission, COMPOUNDCOLORSPREFIXDOT "emission", sim_propertytype_floatarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Emission color data"}, {"description", "Emission color from all compound elements"}}), "") \
    FUNCX(propShape_compoundColorTransparency, COMPOUNDCOLORSPREFIXDOT "transparency", sim_propertytype_floatarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Transparency data"}, {"description", "Transparency value from all compound elements"}}), "") \
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
    FUNCX(propShape_GROUP_applyColor, APPLYCOLORPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    FUNCX(propShape_GROUP_compoundColors, COMPOUNDCOLORSPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    FUNCX(propShape_METHOD_addForce, "addForce", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_addTorque, "addTorque", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_relocateFrame, "relocateFrame", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_alignBoundingBox, "alignBoundingBox", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_getInertia, "getInertia", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_setInertia, "setInertia", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_computeInertia, "computeInertia", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_setAppearance, "setAppearance", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_getAppearance, "getAppearance", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_getContacts, "getContacts", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_ungroup, "ungroup", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propShape_METHOD_divide, "divide", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "")

#define CAMERA_PROPERTIES \
    FUNCX(propCamera_size, "cameraSize", sim_propertytype_float, 0,  jsonStr({{"label", "Size"}, {"description", "Camera size"}}), "") \
    FUNCX(propCamera_parentAsManipProxy, "parentAsManipulationProxy", sim_propertytype_bool, 0,  jsonStr({{"label", "Parent as proxy"}, {"description", "Use parent as manipulation proxy"}}), "") \
    FUNCX(propCamera_translationEnabled, "translationEnabled", sim_propertytype_bool, 0,  jsonStr({{"label", "Translation enabled"}, {"description", ""}}), "") \
    FUNCX(propCamera_rotationEnabled, "rotationEnabled", sim_propertytype_bool, 0,  jsonStr({{"label", "Rotation enabled"}, {"description", ""}}), "") \
    FUNCX(propCamera_trackedObject, "trackedObject", sim_propertytype_handle, sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Tracked object"}, {"description", "Tracked scene object"}, {"handleType", "sceneObject"}}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propCamera_DEPRECATED_trackedObject, "trackedObjectHandle", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, "", "") \

#define DUMMY_PROPERTIES \
    FUNCX(propDummy_linkedDummy, "linkedDummy", sim_propertytype_handle, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Linked dummy"}, {"description", "Handle of the linked dummy"}, {"handleType", "dummy"}}), "") \
    FUNCX(propDummy_assemblyTag, "assemblyTag", sim_propertytype_string, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Assembly tag"}, {"description", ""}}), "") \
    FUNCX(propDummy_size, "dummySize", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Size"}, {"description", "Dummy size"}}), "") \
    FUNCX(propDummy_dummyType, "type", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Type"}, {"description", "Dummy type"}}), "") \
    FUNCX(propDummy_engineProperties, DYNAMICSPREFIXDOT "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Engine properties"}, {"description", "Engine properties as JSON text"}}), "") \
    FUNCX(propDummy_mujocoLimitsEnabled, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "limitsEnabled", sim_propertytype_bool, 0, sim_mujoco_dummy_limited, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoLimitsRange, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "limitsRange", sim_propertytype_floatarray, 0, sim_mujoco_dummy_range1, sim_mujoco_dummy_range2, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoLimitsSolref, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "limitsSolref", sim_propertytype_floatarray, 0, sim_mujoco_dummy_solreflimit1, sim_mujoco_dummy_solreflimit2, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoLimitsSolimp, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "limitsSolimp", sim_propertytype_floatarray, 0, sim_mujoco_dummy_solimplimit1, sim_mujoco_dummy_solimplimit2, sim_mujoco_dummy_solimplimit3, sim_mujoco_dummy_solimplimit4, sim_mujoco_dummy_solimplimit5,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoMargin, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "margin", sim_propertytype_float, 0, sim_mujoco_dummy_margin, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoSpringStiffness, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "springStiffness", sim_propertytype_float, 0, sim_mujoco_dummy_stiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoSpringDamping, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "springDamping", sim_propertytype_float, 0, sim_mujoco_dummy_damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoSpringLength, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "springLength", sim_propertytype_float, 0, sim_mujoco_dummy_springlength, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoJointProxyHandle, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "jointProxyHandle", sim_propertytype_int, sim_propertyinfo_modelhashexclude, sim_mujoco_dummy_proxyjointid, -1, -1, -1, -1,  jsonStr({{"label", "Joint proxy"}, {"description", "Handle of the joint proxy, MuJoCo only"}}), "") \
    FUNCX(propDummy_mujocoOverlapConstrSolref, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "overlapConstrSolref", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoOverlapConstrSolimp, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "overlapConstrSolimp", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_mujocoOverlapConstrTorqueScale, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "overlapConstrTorquescale", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propDummy_GROUP_dynamics, DYNAMICSPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propDummy_GROUP_dynamicsMujoco, DYNAMICSPREFIXDOT MUJOCOPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    /* Following for backward compatibility: */ \
    FUNCX(propDummy_DEPRECATED_dummyType, "dummyType", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propDummy_DEPRECATED_engineProperties, "engineProperties", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propDummy_DEPRECATED_mujocoLimitsEnabled, MUJOCOPREFIXDOT "limitsEnabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_dummy_limited, -1, -1, -1, -1, "", "") \
    FUNCX(propDummy_DEPRECATED_mujocoLimitsRange, MUJOCOPREFIXDOT "limitsRange", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_dummy_range1, sim_mujoco_dummy_range2, -1, -1, -1, "", "") \
    FUNCX(propDummy_DEPRECATED_mujocoLimitsSolref, MUJOCOPREFIXDOT "limitsSolref", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_dummy_solreflimit1, sim_mujoco_dummy_solreflimit2, -1, -1, -1, "", "") \
    FUNCX(propDummy_DEPRECATED_mujocoLimitsSolimp, MUJOCOPREFIXDOT "limitsSolimp", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_dummy_solimplimit1, sim_mujoco_dummy_solimplimit2, sim_mujoco_dummy_solimplimit3, sim_mujoco_dummy_solimplimit4, sim_mujoco_dummy_solimplimit5, "", "") \
    FUNCX(propDummy_DEPRECATED_mujocoMargin, MUJOCOPREFIXDOT "margin", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_dummy_margin, -1, -1, -1, -1, "", "") \
    FUNCX(propDummy_DEPRECATED_mujocoSpringStiffness, MUJOCOPREFIXDOT "springStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_dummy_stiffness, -1, -1, -1, -1, "", "") \
    FUNCX(propDummy_DEPRECATED_mujocoSpringDamping, MUJOCOPREFIXDOT "springDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_dummy_damping, -1, -1, -1, -1, "", "") \
    FUNCX(propDummy_DEPRECATED_mujocoSpringLength, MUJOCOPREFIXDOT "springLength", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_dummy_springlength, -1, -1, -1, -1, "", "") \
    FUNCX(propDummy_DEPRECATED_mujocoJointProxyHandle, MUJOCOPREFIXDOT "jointProxyHandle", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_dummy_proxyjointid, -1, -1, -1, -1, "", "") \
    FUNCX(propDummy_DEPRECATED_mujocoOverlapConstrSolref, MUJOCOPREFIXDOT "overlapConstrSolref", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propDummy_DEPRECATED_mujocoOverlapConstrSolimp, MUJOCOPREFIXDOT "overlapConstrSolimp", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propDummy_DEPRECATED_mujocoOverlapConstrTorqueScale, MUJOCOPREFIXDOT "overlapConstrTorquescale", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propDummy_DEPRECATED_linkedDummyHandle, "linkedDummyHandle", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1, "", "")

#define FORCESENSOR_PROPERTIES \
    FUNCX(propForceSensor_size, "sensorSize", sim_propertytype_float, 0,  jsonStr({{"label", "Size"}, {"description", "Sensor size"}}), "") \
    FUNCX(propForceSensor_forceThresholdEnabled, "forceThresholdEnabled", sim_propertytype_bool, 0,  jsonStr({{"label", "Force threshold enabled"}, {"description", ""}}), "") \
    FUNCX(propForceSensor_torqueThresholdEnabled, "torqueThresholdEnabled", sim_propertytype_bool, 0,  jsonStr({{"label", "Torque threshold enabled"}, {"description", ""}}), "") \
    FUNCX(propForceSensor_filterType, "filterType", sim_propertytype_int, 0,  jsonStr({{"label", "Filter"}, {"description", "Filter type"}}), "") \
    FUNCX(propForceSensor_filterSampleSize, "filterSampleSize", sim_propertytype_int, 0,  jsonStr({{"label", "Filter sample"}, {"description", "Filter sample size"}}), "") \
    FUNCX(propForceSensor_consecutiveViolationsToTrigger, "consecutiveViolationsToTrigger", sim_propertytype_int, 0,  jsonStr({{"label", "Consecutive violations to trigger"}, {"description", ""}}), "") \
    FUNCX(propForceSensor_forceThreshold, "forceThreshold", sim_propertytype_float, 0,  jsonStr({{"label", "Force threshold"}, {"description", ""}}), "") \
    FUNCX(propForceSensor_torqueThreshold, "torqueThreshold", sim_propertytype_float, 0,  jsonStr({{"label", "Torque threshold"}, {"description", ""}}), "") \
    FUNCX(propForceSensor_sensorForce, "force", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Force"}, {"description", "Measured force vector"}}), "") \
    FUNCX(propForceSensor_sensorTorque, "torque", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Torque"}, {"description", "Measured torque vector"}}), "") \
    FUNCX(propForceSensor_filteredSensorForce, "filteredForce", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Filtered force"}, {"description", "Filtered force vector"}}), "") \
    FUNCX(propForceSensor_filteredSensorTorque, "filteredTorque", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Filtered torque"}, {"description", "Filtered torque vector"}}), "") \
    FUNCX(propForceSensor_intrinsicError, "intrinsicError", sim_propertytype_pose, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Intrinsic error"}, {"description", "Intrinsic error, generated by some physics engines"}}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propForceSensor_DEPRECATED_sensorForce, "sensorForce", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propForceSensor_DEPRECATED_sensorTorque, "sensorTorque", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propForceSensor_DEPRECATED_sensorAverageForce, "filterSensorForce", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propForceSensor_DEPRECATED_sensorAverageTorque, "filterSensorTorque", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "", "") \

#define GRAPH_PROPERTIES \
    FUNCX(propGraph_size, "graphSize", sim_propertytype_float, 0,  jsonStr({{"label", "Size"}, {"description", "Size of the 3D graph representation"}}), "") \
    FUNCX(propGraph_bufferSize, "bufferSize", sim_propertytype_int, 0,  jsonStr({{"label", "Buffer size"}, {"description", ""}}), "") \
    FUNCX(propGraph_cyclic, "cyclic", sim_propertytype_bool, 0,  jsonStr({{"label", "Cyclic"}, {"description", "Buffer is cyclic"}}), "") \
    FUNCX(propGraph_backgroundColor, "backgroundColor", sim_propertytype_color, 0,  jsonStr({{"label", "Background color"}, {"description", ""}}), "") \
    FUNCX(propGraph_foregroundColor, "foregroundColor", sim_propertytype_color, 0,  jsonStr({{"label", "Foreground color"}, {"description", ""}}), "") \
    FUNCX(propGraph_METHOD_addCurve, "addCurve", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propGraph_METHOD_addSignal, "addSignal", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propGraph_METHOD_setSignalPoint, "setSignalPoint", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propGraph_METHOD_removeTrace, "removeTrace", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propGraph_METHOD_snapshotTrace, "snapshotTrace", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propGraph_METHOD_step, "step", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propGraph_METHOD_resetGraph, "resetGraph", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "")

#define JOINT_PROPERTIES \
    FUNCX(propJoint_position, "jointPosition", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Position"}, {"description", "Joint linear/angular displacement"}}), "") \
    FUNCX(propJoint_quaternion, "jointQuaternion", sim_propertytype_quaternion, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Spherical joint quaternion"}, {"description", ""}}), "") \
    FUNCX(propJoint_screwLead, "screwLead", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Screw lead"}, {"description", ""}}), "") \
    FUNCX(propJoint_intrinsicError, "intrinsicError", sim_propertytype_pose, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Intrinsic error"}, {"description", "Intrinsic error, generated by some physics engines"}}), "") \
    FUNCX(propJoint_intrinsicPose, "intrinsicPose", sim_propertytype_pose, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Intrinsic pose"}, {"description", "Intrinsic pose (includes a possible intrinsic error)"}}), "") \
    FUNCX(propJoint_calcVelocity, "calcVelocity", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Velocity"}, {"description", "Calculated joint linear or angular velocity"}}), "") \
    FUNCX(propJoint_cyclic, "cyclic", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Cyclic"}, {"description", "Cyclic revolute joint, has no limits"}}), "") \
    FUNCX(propJoint_enforceLimits, "enforceLimits", sim_propertytype_bool, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Enforce limits"}, {"description", "Enforce limits strictly"}}), "") \
    FUNCX(propJoint_interval, "interval", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Interval"}, {"description", "Joint limits (lower and upper bounds)"}}), "") \
    FUNCX(propJoint_targetPos, "targetPosition", sim_propertytype_float, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Target position"}, {"description", "Position to reach by controller"}}), "") \
    FUNCX(propJoint_targetVel, "targetVelocity", sim_propertytype_float, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Target velocity"}, {"description", "Velocity to reach by controller"}}), "") \
    FUNCX(propJoint_targetForce, "targetForce", sim_propertytype_float, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Target force"}, {"description", "Maximum force to exert"}}), "") \
    FUNCX(propJoint_length, "length", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Size"}, {"description", "Joint size"}}), "") \
    FUNCX(propJoint_diameter, "diameter", sim_propertytype_float, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Size"}, {"description", "Joint size"}}), "") \
    FUNCX(propJoint_jointType, "type", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable, -1, -1, -1, -1, -1,  jsonStr({{"label", "Type"}, {"description", "Joint type"}}), "") \
    FUNCX(propJoint_jointForce, "force", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Force"}, {"description", "Force applied"}}), "") \
    FUNCX(propJoint_averageJointForce, "averageForce", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Average force"}, {"description", "Force applied on average (in a simulation step)"}}), "") \
    FUNCX(propJoint_jointMode, "mode", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Mode"}, {"description", "Joint mode"}}), "") \
    FUNCX(propJoint_dependencyMaster, "dependencyMaster", sim_propertytype_handle, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dependency master"}, {"description", "Handle of master joint (in a dependency relationship)"}, {"handleType", "joint"}}), "") \
    FUNCX(propJoint_dependencyParams, "dependencyParams", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dependency parameters"}, {"description", "Dependency parameters (offset and coefficient)"}}), "") \
    FUNCX(propJoint_maxVelAccelJerk, "maxVelAccelJerk", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Maximum velocity, acceleration and jerk"}, {"description", ""}}), "") \
    FUNCX(propJoint_dynCtrlMode, DYNAMICSPREFIXDOT "ctrlMode", sim_propertytype_int, 0, -1, -1, -1, -1, -1, jsonStr({{"label", "Control mode" }, {"description", "Joint control mode, when in dynamic mode"}, {"enum", "jointDynCtrlMode"}}), "") \
    FUNCX(propJoint_springDamperParams, DYNAMICSPREFIXDOT "springDamperParams", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Spring damper parameters"}, {"description", ""}}), "") \
    FUNCX(propJoint_dynVelMode, DYNAMICSPREFIXDOT "velMode", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dynamic velocity mode"}, {"description", "Dynamic velocity mode (0: default, 1: Ruckig)"}}), "") \
    FUNCX(propJoint_dynPosMode, DYNAMICSPREFIXDOT "posMode", sim_propertytype_int, 0, -1, -1, -1, -1, -1,  jsonStr({{"label", "Dynamic position mode"}, {"description", "Dynamic position mode (0: default, 1: Ruckig)"}}), "") \
    FUNCX(propJoint_engineProperties, DYNAMICSPREFIXDOT "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1,  jsonStr({{"label", "Engine properties"}, {"description", "Engine properties as JSON text"}}), "") \
    FUNCX(propJoint_bulletStopErp, DYNAMICSPREFIXDOT BULLETPREFIXDOT "stopErp", sim_propertytype_float, 0, sim_bullet_joint_stoperp, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_bulletStopCfm, DYNAMICSPREFIXDOT BULLETPREFIXDOT "stopCfm", sim_propertytype_float, 0, sim_bullet_joint_stopcfm, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_bulletNormalCfm, DYNAMICSPREFIXDOT BULLETPREFIXDOT "normalCfm", sim_propertytype_float, 0, sim_bullet_joint_normalcfm, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_bulletPosPid, DYNAMICSPREFIXDOT BULLETPREFIXDOT "posPid", sim_propertytype_floatarray, 0, sim_bullet_joint_pospid1, sim_bullet_joint_pospid2, sim_bullet_joint_pospid3, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_odeStopErp, DYNAMICSPREFIXDOT ODEPREFIXDOT "stopErp", sim_propertytype_float, 0, sim_ode_joint_stoperp, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_odeStopCfm, DYNAMICSPREFIXDOT ODEPREFIXDOT "stopCfm", sim_propertytype_float, 0, sim_ode_joint_stopcfm, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_odeNormalCfm, DYNAMICSPREFIXDOT ODEPREFIXDOT "normalCfm", sim_propertytype_float, 0, sim_ode_joint_normalcfm, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_odeBounce, DYNAMICSPREFIXDOT ODEPREFIXDOT "bounce", sim_propertytype_float, 0, sim_ode_joint_bounce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_odeFudgeFactor, DYNAMICSPREFIXDOT ODEPREFIXDOT "fudge", sim_propertytype_float, 0, sim_ode_joint_fudgefactor, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_odePosPid, DYNAMICSPREFIXDOT ODEPREFIXDOT "posPid", sim_propertytype_floatarray, 0, sim_ode_joint_pospid1, sim_ode_joint_pospid2, sim_ode_joint_pospid3, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexLowerLimitDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisLimitsLowerDamping", sim_propertytype_float, 0, sim_vortex_joint_lowerlimitdamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexUpperLimitDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisLimitsUpperDamping", sim_propertytype_float, 0, sim_vortex_joint_upperlimitdamping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexLowerLimitStiffness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisLimitsLowerStiffness", sim_propertytype_float, 0, sim_vortex_joint_lowerlimitstiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexUpperLimitStiffness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisLimitsUpperStiffness", sim_propertytype_float, 0, sim_vortex_joint_upperlimitstiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexLowerLimitRestitution, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisLimitsLowerRestitution", sim_propertytype_float, 0, sim_vortex_joint_lowerlimitrestitution, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexUpperLimitRestitution, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisLimitsUpperRestitution", sim_propertytype_float, 0, sim_vortex_joint_upperlimitrestitution, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexLowerLimitMaxForce, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisLimitsLowerMaxForce", sim_propertytype_float, 0, sim_vortex_joint_lowerlimitmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexUpperLimitMaxForce, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisLimitsUpperMaxForce", sim_propertytype_float, 0, sim_vortex_joint_upperlimitmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexAxisFrictionEnabled, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisFrictionEnabled", sim_propertytype_bool, 0, sim_vortex_joint_motorfrictionenabled, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexAxisFrictionProportional, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisFrictionProportional", sim_propertytype_bool, 0, sim_vortex_joint_proportionalmotorfriction, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexAxisFrictionCoeff, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisFrictionValue", sim_propertytype_float, 0, sim_vortex_joint_motorconstraintfrictioncoeff, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexAxisFrictionMaxForce, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisFrictionMaxForce", sim_propertytype_float, 0, sim_vortex_joint_motorconstraintfrictionmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexAxisFrictionLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_motorconstraintfrictionloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexRelaxationEnabledBits, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "relaxationEnabledBits", sim_propertytype_int, 0, sim_vortex_joint_relaxationenabledbc, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexFrictionEnabledBits, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "frictionEnabledBits", sim_propertytype_int, 0, sim_vortex_joint_frictionenabledbc, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexFrictionProportionalBits, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "frictionProportionalBits", sim_propertytype_int, 0, sim_vortex_joint_frictionproportionalbc, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisPosRelaxationStiffness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisPosRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_p0stiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisPosRelaxationDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisPosRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_p0damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisPosRelaxationLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisPosRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_p0loss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisPosFrictionCoeff, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisPosFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_p0frictioncoeff, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisPosFrictionMaxForce, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisPosFrictionMaxForce", sim_propertytype_float, 0, sim_vortex_joint_p0frictionmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisPosFrictionLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisPosFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_p0frictionloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisPosRelaxationStiffness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisPosRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_p1stiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisPosRelaxationDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisPosRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_p1damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisPosRelaxationLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisPosRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_p1loss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisPosFrictionCoeff, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisPosFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_p1frictioncoeff, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisPosFrictionMaxForce, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisPosFrictionMaxForce", sim_propertytype_float, 0, sim_vortex_joint_p1frictionmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisPosFrictionLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisPosFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_p1frictionloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisPosRelaxationStiffness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisPosRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_p2stiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisPosRelaxationDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisPosRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_p2damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisPosRelaxationLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisPosRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_p2loss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisPosFrictionCoeff, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisPosFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_p2frictioncoeff, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisPosFrictionMaxForce, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisPosFrictionMaxForce", sim_propertytype_float, 0, sim_vortex_joint_p2frictionmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisPosFrictionLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisPosFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_p2frictionloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisOrientRelaxStiffness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisOrientRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_a0stiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisOrientRelaxDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisOrientRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_a0damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisOrientRelaxLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisOrientRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_a0loss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisOrientFrictionCoeff, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisOrientFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_a0frictioncoeff, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisOrientFrictionMaxTorque, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisOrientFrictionMaxTorque", sim_propertytype_float, 0, sim_vortex_joint_a0frictionmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexXAxisOrientFrictionLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisOrientFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_a0frictionloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisOrientRelaxStiffness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisOrientRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_a1stiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisOrientRelaxDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisOrientRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_a1damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisOrientRelaxLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisOrientRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_a1loss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisOrientFrictionCoeff, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisOrientFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_a1frictioncoeff, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisOrientFrictionMaxTorque, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisOrientFrictionMaxTorque", sim_propertytype_float, 0, sim_vortex_joint_a1frictionmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexYAxisOrientFrictionLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisOrientFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_a1frictionloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisOrientRelaxStiffness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisOrientRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_a2stiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisOrientRelaxDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisOrientRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_a2damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisOrientRelaxLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisOrientRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_a2loss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisOrientFrictionCoeff, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisOrientFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_a2frictioncoeff, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisOrientFrictionMaxTorque, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisOrientFrictionMaxTorque", sim_propertytype_float, 0, sim_vortex_joint_a2frictionmaxforce, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexZAxisOrientFrictionLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisOrientFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_a2frictionloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_vortexPosPid, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "posPid", sim_propertytype_floatarray, 0, sim_vortex_joint_pospid1, sim_vortex_joint_pospid2, sim_vortex_joint_pospid3, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_newtonPosPid, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "posPid", sim_propertytype_floatarray, 0, sim_newton_joint_pospid1, sim_newton_joint_pospid2, sim_newton_joint_pospid3, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoArmature, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "armature", sim_propertytype_float, 0, sim_mujoco_joint_armature, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoMargin, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "margin", sim_propertytype_float, 0, sim_mujoco_joint_margin, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoFrictionLoss, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "frictionLoss", sim_propertytype_float, 0, sim_mujoco_joint_frictionloss, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoSpringStiffness, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "springStiffness", sim_propertytype_float, 0, sim_mujoco_joint_stiffness, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoSpringDamping, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "springDamping", sim_propertytype_float, 0, sim_mujoco_joint_damping, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoSpringRef, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "springRef", sim_propertytype_float, 0, sim_mujoco_joint_springref, -1, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoPosPid, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "posPid", sim_propertytype_floatarray, 0, sim_mujoco_joint_pospid1, sim_mujoco_joint_pospid2, sim_mujoco_joint_pospid3, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoLimitsSolRef, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "limitsSolref", sim_propertytype_floatarray, 0, sim_mujoco_joint_solreflimit1, sim_mujoco_joint_solreflimit2, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoLimitsSolImp, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "limitsSolimp", sim_propertytype_floatarray, 0, sim_mujoco_joint_solimplimit1, sim_mujoco_joint_solimplimit2, sim_mujoco_joint_solimplimit3, sim_mujoco_joint_solimplimit4, sim_mujoco_joint_solimplimit5,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoFrictionSolRef, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "frictionSolref", sim_propertytype_floatarray, 0, sim_mujoco_joint_solreffriction1, sim_mujoco_joint_solreffriction2, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoFrictionSolImp, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "frictionSolimp", sim_propertytype_floatarray, 0, sim_mujoco_joint_solimpfriction1, sim_mujoco_joint_solimpfriction2, sim_mujoco_joint_solimpfriction3, sim_mujoco_joint_solimpfriction4, sim_mujoco_joint_solimpfriction5,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoSpringDamper, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "springSpringDamper", sim_propertytype_floatarray, 0, sim_mujoco_joint_springdamper1, sim_mujoco_joint_springdamper2, -1, -1, -1,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_mujocoDependencyPolyCoef, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "dependencyPolyCoeff", sim_propertytype_floatarray, 0, sim_mujoco_joint_polycoef1, sim_mujoco_joint_polycoef2, sim_mujoco_joint_polycoef3, sim_mujoco_joint_polycoef4, sim_mujoco_joint_polycoef5,  jsonStr({{"label", ""}, {"description", ""}}), "") \
    FUNCX(propJoint_GROUP_dynamics, DYNAMICSPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propJoint_GROUP_dynamicsBullet, DYNAMICSPREFIXDOT BULLETPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propJoint_GROUP_dynamicsOde, DYNAMICSPREFIXDOT ODEPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propJoint_GROUP_dynamicsNewton, DYNAMICSPREFIXDOT NEWTONPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propJoint_GROUP_dynamicsVortex, DYNAMICSPREFIXDOT VORTEXPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propJoint_GROUP_dynamicsMujoco, DYNAMICSPREFIXDOT MUJOCOPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propJoint_METHOD_setTargetPosition, "setTargetPosition", sim_propertytype_method, SIM_PROPERTYINFO_METHOD, -1, -1, -1, -1, -1, jsonStr({}), "") \
    FUNCX(propJoint_METHOD_setTargetVelocity, "setTargetVelocity", sim_propertytype_method, SIM_PROPERTYINFO_METHOD, -1, -1, -1, -1, -1, jsonStr({}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propJoint_DEPRECATED_targetPos, "targetPos", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propJoint_DEPRECATED_targetVel, "targetVel", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propJoint_DEPRECATED_length, "jointLength", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propJoint_DEPRECATED_diameter, "jointDiameter", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propJoint_DEPRECATED_jointType, "jointType", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propJoint_DEPRECATED_jointForce, "jointForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propJoint_DEPRECATED_averageJointForce, "averageJointForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propJoint_DEPRECATED_jointMode, "jointMode", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propJoint_DEPRECATED_dynCtrlMode, "dynCtrlMode", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1, "", "") \
    FUNCX(propJoint_DEPRECATED_springDamperParams, "springDamperParams", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_dynVelMode, "dynVelMode", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_dynPosMode, "dynPosMode", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_engineProperties, "engineProperties", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_bulletStopErp, BULLETPREFIXDOT "stopErp", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_joint_stoperp, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_bulletStopCfm, BULLETPREFIXDOT "stopCfm", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_joint_stopcfm, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_bulletNormalCfm, BULLETPREFIXDOT "normalCfm", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_joint_normalcfm, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_bulletPosPid, BULLETPREFIXDOT "posPid", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_bullet_joint_pospid1, sim_bullet_joint_pospid2, sim_bullet_joint_pospid3, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_odeStopErp, ODEPREFIXDOT "stopErp", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_ode_joint_stoperp, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_odeStopCfm, ODEPREFIXDOT "stopCfm", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_ode_joint_stopcfm, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_odeNormalCfm, ODEPREFIXDOT "normalCfm", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_ode_joint_normalcfm, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_odeBounce, ODEPREFIXDOT "bounce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_ode_joint_bounce, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_odeFudgeFactor, ODEPREFIXDOT "fudge", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_ode_joint_fudgefactor, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_odePosPid, ODEPREFIXDOT "posPid", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_ode_joint_pospid1, sim_ode_joint_pospid2, sim_ode_joint_pospid3, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexLowerLimitDamping, VORTEXPREFIXDOT "axisLimitsLowerDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_lowerlimitdamping, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexUpperLimitDamping, VORTEXPREFIXDOT "axisLimitsUpperDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_upperlimitdamping, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexLowerLimitStiffness, VORTEXPREFIXDOT "axisLimitsLowerStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_lowerlimitstiffness, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexUpperLimitStiffness, VORTEXPREFIXDOT "axisLimitsUpperStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_upperlimitstiffness, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexLowerLimitRestitution, VORTEXPREFIXDOT "axisLimitsLowerRestitution", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_lowerlimitrestitution, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexUpperLimitRestitution, VORTEXPREFIXDOT "axisLimitsUpperRestitution", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_upperlimitrestitution, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexLowerLimitMaxForce, VORTEXPREFIXDOT "axisLimitsLowerMaxForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_lowerlimitmaxforce, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexUpperLimitMaxForce, VORTEXPREFIXDOT "axisLimitsUpperMaxForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_upperlimitmaxforce, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexAxisFrictionEnabled, VORTEXPREFIXDOT "axisFrictionEnabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_motorfrictionenabled, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexAxisFrictionProportional, VORTEXPREFIXDOT "axisFrictionProportional", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_proportionalmotorfriction, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexAxisFrictionCoeff, VORTEXPREFIXDOT "axisFrictionValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_motorconstraintfrictioncoeff, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexAxisFrictionMaxForce, VORTEXPREFIXDOT "axisFrictionMaxForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_motorconstraintfrictionmaxforce, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexAxisFrictionLoss, VORTEXPREFIXDOT "axisFrictionLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_motorconstraintfrictionloss, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexRelaxationEnabledBits, VORTEXPREFIXDOT "relaxationEnabledBits", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_relaxationenabledbc, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexFrictionEnabledBits, VORTEXPREFIXDOT "frictionEnabledBits", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_frictionenabledbc, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexFrictionProportionalBits, VORTEXPREFIXDOT "frictionProportionalBits", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_frictionproportionalbc, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexXAxisPosRelaxationStiffness, VORTEXPREFIXDOT "xAxisPosRelaxationStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p0stiffness, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexXAxisPosRelaxationDamping, VORTEXPREFIXDOT "xAxisPosRelaxationDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p0damping, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexXAxisPosRelaxationLoss, VORTEXPREFIXDOT "xAxisPosRelaxationLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p0loss, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexXAxisPosFrictionCoeff, VORTEXPREFIXDOT "xAxisPosFrictionCoeff", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p0frictioncoeff, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexXAxisPosFrictionMaxForce, VORTEXPREFIXDOT "xAxisPosFrictionMaxForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p0frictionmaxforce, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexXAxisPosFrictionLoss, VORTEXPREFIXDOT "xAxisPosFrictionLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p0frictionloss, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexYAxisPosRelaxationStiffness, VORTEXPREFIXDOT "yAxisPosRelaxationStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p1stiffness, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexYAxisPosRelaxationDamping, VORTEXPREFIXDOT "yAxisPosRelaxationDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p1damping, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexYAxisPosRelaxationLoss, VORTEXPREFIXDOT "yAxisPosRelaxationLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p1loss, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexYAxisPosFrictionCoeff, VORTEXPREFIXDOT "yAxisPosFrictionCoeff", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p1frictioncoeff, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexYAxisPosFrictionMaxForce, VORTEXPREFIXDOT "yAxisPosFrictionMaxForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p1frictionmaxforce, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexYAxisPosFrictionLoss, VORTEXPREFIXDOT "yAxisPosFrictionLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p1frictionloss, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexZAxisPosRelaxationStiffness, VORTEXPREFIXDOT "zAxisPosRelaxationStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p2stiffness, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexZAxisPosRelaxationDamping, VORTEXPREFIXDOT "zAxisPosRelaxationDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p2damping, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexZAxisPosRelaxationLoss, VORTEXPREFIXDOT "zAxisPosRelaxationLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p2loss, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexZAxisPosFrictionCoeff, VORTEXPREFIXDOT "zAxisPosFrictionCoeff", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p2frictioncoeff, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexZAxisPosFrictionMaxForce, VORTEXPREFIXDOT "zAxisPosFrictionMaxForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p2frictionmaxforce, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexZAxisPosFrictionLoss, VORTEXPREFIXDOT "zAxisPosFrictionLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_p2frictionloss, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexXAxisOrientRelaxStiffness, VORTEXPREFIXDOT "xAxisOrientRelaxationStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a0stiffness, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexXAxisOrientRelaxDamping, VORTEXPREFIXDOT "xAxisOrientRelaxationDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a0damping, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexXAxisOrientRelaxLoss, VORTEXPREFIXDOT "xAxisOrientRelaxationLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a0loss, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexXAxisOrientFrictionCoeff, VORTEXPREFIXDOT "xAxisOrientFrictionCoeff", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a0frictioncoeff, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexXAxisOrientFrictionMaxTorque, VORTEXPREFIXDOT "xAxisOrientFrictionMaxTorque", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a0frictionmaxforce, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexXAxisOrientFrictionLoss, VORTEXPREFIXDOT "xAxisOrientFrictionLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a0frictionloss, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexYAxisOrientRelaxStiffness, VORTEXPREFIXDOT "yAxisOrientRelaxationStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a1stiffness, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexYAxisOrientRelaxDamping, VORTEXPREFIXDOT "yAxisOrientRelaxationDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a1damping, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexYAxisOrientRelaxLoss, VORTEXPREFIXDOT "yAxisOrientRelaxationLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a1loss, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexYAxisOrientFrictionCoeff, VORTEXPREFIXDOT "yAxisOrientFrictionCoeff", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a1frictioncoeff, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexYAxisOrientFrictionMaxTorque, VORTEXPREFIXDOT "yAxisOrientFrictionMaxTorque", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a1frictionmaxforce, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexYAxisOrientFrictionLoss, VORTEXPREFIXDOT "yAxisOrientFrictionLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a1frictionloss, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexZAxisOrientRelaxStiffness, VORTEXPREFIXDOT "zAxisOrientRelaxationStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a2stiffness, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexZAxisOrientRelaxDamping, VORTEXPREFIXDOT "zAxisOrientRelaxationDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a2damping, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexZAxisOrientRelaxLoss, VORTEXPREFIXDOT "zAxisOrientRelaxationLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a2loss, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexZAxisOrientFrictionCoeff, VORTEXPREFIXDOT "zAxisOrientFrictionCoeff", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a2frictioncoeff, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexZAxisOrientFrictionMaxTorque, VORTEXPREFIXDOT "zAxisOrientFrictionMaxTorque", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a2frictionmaxforce, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexZAxisOrientFrictionLoss, VORTEXPREFIXDOT "zAxisOrientFrictionLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_a2frictionloss, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_vortexPosPid, VORTEXPREFIXDOT "posPid", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_vortex_joint_pospid1, sim_vortex_joint_pospid2, sim_vortex_joint_pospid3, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_newtonPosPid, NEWTONPREFIXDOT "posPid", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_newton_joint_pospid1, sim_newton_joint_pospid2, sim_newton_joint_pospid3, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_mujocoArmature, MUJOCOPREFIXDOT "armature", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_joint_armature, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_mujocoMargin, MUJOCOPREFIXDOT "margin", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_joint_margin, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_mujocoFrictionLoss, MUJOCOPREFIXDOT "frictionLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_joint_frictionloss, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_mujocoSpringStiffness, MUJOCOPREFIXDOT "springStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_joint_stiffness, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_mujocoSpringDamping, MUJOCOPREFIXDOT "springDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_joint_damping, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_mujocoSpringRef, MUJOCOPREFIXDOT "springRef", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_joint_springref, -1, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_mujocoPosPid, MUJOCOPREFIXDOT "posPid", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_joint_pospid1, sim_mujoco_joint_pospid2, sim_mujoco_joint_pospid3, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_mujocoLimitsSolRef, MUJOCOPREFIXDOT "limitsSolref", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_joint_solreflimit1, sim_mujoco_joint_solreflimit2, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_mujocoLimitsSolImp, MUJOCOPREFIXDOT "limitsSolimp", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_joint_solimplimit1, sim_mujoco_joint_solimplimit2, sim_mujoco_joint_solimplimit3, sim_mujoco_joint_solimplimit4, sim_mujoco_joint_solimplimit5,  "", "") \
    FUNCX(propJoint_DEPRECATED_mujocoFrictionSolRef, MUJOCOPREFIXDOT "frictionSolref", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_joint_solreffriction1, sim_mujoco_joint_solreffriction2, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_mujocoFrictionSolImp, MUJOCOPREFIXDOT "frictionSolimp", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_joint_solimpfriction1, sim_mujoco_joint_solimpfriction2, sim_mujoco_joint_solimpfriction3, sim_mujoco_joint_solimpfriction4, sim_mujoco_joint_solimpfriction5,  "", "") \
    FUNCX(propJoint_DEPRECATED_mujocoSpringDamper, MUJOCOPREFIXDOT "springSpringDamper", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_joint_springdamper1, sim_mujoco_joint_springdamper2, -1, -1, -1,  "", "") \
    FUNCX(propJoint_DEPRECATED_mujocoDependencyPolyCoef, MUJOCOPREFIXDOT "dependencyPolyCoeff", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, sim_mujoco_joint_polycoef1, sim_mujoco_joint_polycoef2, sim_mujoco_joint_polycoef3, sim_mujoco_joint_polycoef4, sim_mujoco_joint_polycoef5,  "", "") \
    FUNCX(propJoint_DEPRECATED_dependencyMaster, "dependencyMasterHandle", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, -1, -1, -1, -1, -1, "", "")

#define LIGHT_PROPERTIES \
    FUNCX(propLight_size, "lightSize", sim_propertytype_float, 0,  jsonStr({{"label", "Size"}, {"description", "Light size"}}), "") \
    FUNCX(propLight_enabled, "enabled", sim_propertytype_bool, 0,  jsonStr({{"label", "Enabled"}, {"description", ""}}), "") \
    FUNCX(propLight_lightType, "type", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Type"}, {"description", "Light type"}}), "") \
    FUNCX(propLight_spotExponent, "spotExponent", sim_propertytype_int, 0,  jsonStr({{"label", "Spot exponent"}, {"description", ""}}), "") \
    FUNCX(propLight_spotCutoffAngle, "spotCutoffAngle", sim_propertytype_float, 0,  jsonStr({{"label", "Cut off angle"}, {"description", "Spot cut off angle"}}), "") \
    FUNCX(propLight_attenuationFactors, "attenuationFactors", sim_propertytype_floatarray, 0,  jsonStr({{"label", "Attenuation factor"}, {"description", ""}}), "") \
    FUNCX(propLight_povCastShadows, POVRAYPREFIXDOT "castShadows", sim_propertytype_bool, sim_propertyinfo_silent,  jsonStr({{"label", "POV-Ray: cast shadows"}, {"description", "Light casts shadows (with the POV-Ray renderer plugin)"}}), "") \
    FUNCX(propLight_GROUP_povray, POVRAYPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    /* Following for backward compatibility: */ \
    FUNCX(propLight_DEPRECATED_lightType, "lightType", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, "", "")

#define MIRROR_PROPERTIES \
    FUNCX(propMirror_fake, "", sim_propertytype_float, sim_propertyinfo_constant | sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_notreadable,  jsonStr({{"label", ""}, {"description", ""}}), "")

#define OCTREE_PROPERTIES \
    FUNCX(propOctree_voxelSize, "voxelSize", sim_propertytype_float, 0,  jsonStr({{"label", "Voxel size"}, {"description", ""}}), "") \
    FUNCX(propOctree_randomColors, "randomColors", sim_propertytype_bool, 0,  jsonStr({{"label", "Random voxel colors"}, {"description", ""}}), "") \
    FUNCX(propOctree_showPoints, "showPoints", sim_propertytype_bool, 0,  jsonStr({{"label", "Show points instead of voxels"}, {"description", ""}}), "") \
    FUNCX(propOctree_points, "points", sim_propertytype_matrix, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Voxels"}, {"description", "Voxel positions"}}), "") \
    FUNCX(propOctree_packedPoints, "packedPoints", sim_propertytype_buffer, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Packed voxels"}, {"description", "Voxel positions, as packed floats"}}), "") \
    FUNCX(propOctree_colors, "colors", sim_propertytype_buffer, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Voxel Colors"}, {"description", ""}}), "") \
    FUNCX(propOctree_METHOD_addFromObject, "addFromObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propOctree_METHOD_subtractFromObject, "subtractFromObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propOctree_METHOD_clear, "clear", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propOctree_METHOD_addVoxels, "addVoxels", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propOctree_METHOD_addPackedVoxels, "addPackedVoxels", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propOctree_METHOD_subtractVoxels, "subtractVoxels", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propOctree_METHOD_subtractPackedVoxels, "subtractPackedVoxels", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propOctree_METHOD_checkPoints, "checkPoints", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propOctree_METHOD_checkPackedPoints, "checkPackedPoints", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propOctree_DEPRECATED_voxels, "voxels", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "", "")

#define POINTCLOUD_PROPERTIES \
    FUNCX(propPointCloud_ocTreeStruct, "octreeStruct", sim_propertytype_bool, 0,  jsonStr({{"label", "OC-Tree structure Enabled"}, {"description", "Use an oc-tree structure"}}), "") \
    FUNCX(propPointCloud_randomColors, "randomColors", sim_propertytype_bool, 0,  jsonStr({{"label", "Random point colors"}, {"description", ""}}), "") \
    FUNCX(propPointCloud_pointSize, "pointSize", sim_propertytype_int, 0,  jsonStr({{"label", "Point size"}, {"description", ""}}), "") \
    FUNCX(propPointCloud_maxPtsInCell, "maxPointsInCell", sim_propertytype_int, 0,  jsonStr({{"label", "Max. points in cell"}, {"description", "Maximum number of points in an oc-tree cell/voxel"}}), "") \
    FUNCX(propPointCloud_cellSize, "cellSize", sim_propertytype_float, 0,  jsonStr({{"label", "Cell size"}, {"description", "Size of the oc-tree cell/voxel"}}), "") \
    FUNCX(propPointCloud_pointDisplayFraction, "pointDisplayFraction", sim_propertytype_float, 0,  jsonStr({{"label", "Display fraction"}, {"description", "Fraction of points to be displayed in an oc-tree cell/voxel"}}), "") \
    FUNCX(propPointCloud_points, "points", sim_propertytype_matrix, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Points"}, {"description", "Point positions"}}), "") \
    FUNCX(propPointCloud_packedPoints, "packedPoints", sim_propertytype_buffer, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Packed points"}, {"description", "Point positions, as packed floats"}}), "") \
    FUNCX(propPointCloud_colors, "colors", sim_propertytype_buffer, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Colors"}, {"description", "Point colors"}}), "") \
    FUNCX(propPointCloud_METHOD_addFromObject, "addFromObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propPointCloud_METHOD_subtractFromObject, "subtractFromObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propPointCloud_METHOD_clear, "clear", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propPointCloud_METHOD_addPoints, "addPoints", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propPointCloud_METHOD_addPackedPoints, "addPackedPoints", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propPointCloud_METHOD_subtractPoints, "subtractPoints", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propPointCloud_METHOD_subtractPackedPoints, "subtractPackedPoints", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propPointCloud_METHOD_intersectPoints, "intersectPoints", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propPointCloud_METHOD_intersectPackedPoints, "intersectPackedPoints", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "")

#define PROXIMITYSENSOR_PROPERTIES \
    FUNCX(propProximitySensor_size, "pointSize", sim_propertytype_float, 0,  jsonStr({{"label", "Sensor point size"}, {"description", ""}}), "") \
    FUNCX(propProximitySensor_frontFaceDetection, "frontFaceDetection", sim_propertytype_bool, 0,  jsonStr({{"label", "Front face detection"}, {"description", ""}}), "") \
    FUNCX(propProximitySensor_backFaceDetection, "backFaceDetection", sim_propertytype_bool, 0,  jsonStr({{"label", "Back face detection"}, {"description", ""}}), "") \
    FUNCX(propProximitySensor_exactMode, "exactMode", sim_propertytype_bool, 0,  jsonStr({{"label", "Exact mode"}, {"description", ""}}), "") \
    FUNCX(propProximitySensor_explicitHandling, "explicitHandling", sim_propertytype_bool, 0,  jsonStr({{"label", "Explicit handling"}, {"description", ""}}), "") \
    FUNCX(propProximitySensor_showVolume, "showVolume", sim_propertytype_bool, 0,  jsonStr({{"label", "Show volume"}, {"description", ""}}), "") \
    FUNCX(propProximitySensor_randomizedDetection, "randomizedDetection", sim_propertytype_bool, sim_propertyinfo_notwritable,  jsonStr({{"label", "Randomized detection"}, {"description", ""}}), "") \
    FUNCX(propProximitySensor_sensorType, "type", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Type"}, {"description", "Sensor type"}}), "") \
    FUNCX(propProximitySensor_detectedObject, "detectedObject", sim_propertytype_handle, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Detected object"}, {"description", "Detected scene object"}, {"handleType", "sceneObject"}}), "") \
    FUNCX(propProximitySensor_angleThreshold, "angleThreshold", sim_propertytype_float, 0,  jsonStr({{"label", "Angle threshold"}, {"description", "Angle threshold, 0.0 to disable"}}), "") \
    FUNCX(propProximitySensor_detectedPoint, "detectedPoint", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Detected point"}, {"description", ""}}), "") \
    FUNCX(propProximitySensor_detectedNormal, "detectedNormal", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Detected normal"}, {"description", "Detected normal vector"}}), "") \
    FUNCX(propProximitySensor_METHOD_handleSensor, "handleSensor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propProximitySensor_METHOD_resetSensor, "resetSensor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propProximitySensor_METHOD_checkSensor, "checkSensor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propProximitySensor_DEPRECATED_size, "sensorPointSize", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, "", "") \
    FUNCX(propProximitySensor_DEPRECATED_sensorType, "sensorType", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propProximitySensor_DEPRECATED_detectedObjectHandle, "detectedObjectHandle", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "", "")

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
    FUNCX(propVisionSensor_depthBuffer, "depth", sim_propertytype_matrix, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Depth buffer"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_packedDepthBuffer, "packedDepthBuffer", sim_propertytype_buffer, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Packed depth buffer"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_triggerState, "triggerState", sim_propertytype_bool, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Trigger state"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_packet1, "packet1", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Data packet 1"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_packet2, "packet2", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Data packet 2"}, {"description", ""}}), "") \
    FUNCX(propVisionSensor_povFocalBlur, POVRAYPREFIXDOT "focalBlur", sim_propertytype_bool, sim_propertyinfo_silent,  jsonStr({{"label", "POV-Ray: focal blur"}, {"description", "Focal blur (with the POV-Ray renderer plugin)"}}), "") \
    FUNCX(propVisionSensor_povBlurSamples, POVRAYPREFIXDOT "blurSamples", sim_propertytype_int, sim_propertyinfo_silent,  jsonStr({{"label", "POV-Ray: blur samples"}, {"description", "Focal blur samples (with the POV-Ray renderer plugin)"}}), "") \
    FUNCX(propVisionSensor_povBlurDistance, POVRAYPREFIXDOT "blurDistance", sim_propertytype_float, sim_propertyinfo_silent,  jsonStr({{"label", "POV-Ray: blur distance"}, {"description", "Focal blur distance (with the POV-Ray renderer plugin)"}}), "") \
    FUNCX(propVisionSensor_povAperture, POVRAYPREFIXDOT "aperture", sim_propertytype_float, sim_propertyinfo_silent,  jsonStr({{"label", "POV-Ray: aperture"}, {"description", "Aperture (with the POV-Ray renderer plugin)"}}), "") \
    FUNCX(propVisionSensor_GROUP_povray, POVRAYPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, "", "") \
    FUNCX(propVisionSensor_METHOD_handleSensor, "handleSensor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propVisionSensor_METHOD_resetSensor, "resetSensor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propVisionSensor_METHOD_checkSensor, "checkSensor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propVisionSensor_METHOD_getDepth, "getDepth", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propVisionSensor_METHOD_getImage, "getImage", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propVisionSensor_METHOD_setImage, "setImage", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propVisionSensor_DEPRECATED_depthBuffer, "depthBuffer", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, "", "")

#define SCRIPT_PROPERTIES \
    FUNCX(propScript_size, "scriptSize", sim_propertytype_float, 0,  jsonStr({{"label", "Size"}, {"description", "Size of the object"}}), "") \
    FUNCX(propScript_resetAfterSimError, "resetAfterSimError", sim_propertytype_bool, 0,  jsonStr({{"label", "Reset after simulation error"}, {"description", ""}}), "") \
    FUNCX(propScript_detachedScript, "detachedScript", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Detached script handle"}, {"description", ""}, {"handleType", "detachedScript"}}), "") \
    /* Following for backward compatibility: */ \
    FUNCX(propScript_DEPRECATED_scriptDisabled, "scriptDisabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent, "", "") \
    FUNCX(propScript_DEPRECATED_restartOnError, "restartOnError", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent, "", "") \
    FUNCX(propScript_DEPRECATED_execPriority, "execPriority", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent, "", "") \
    FUNCX(propScript_DEPRECATED_scriptType, "scriptType", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propScript_DEPRECATED_executionDepth, "executionDepth", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propScript_DEPRECATED_scriptState, "scriptState", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propScript_DEPRECATED_language, "language", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propScript_DEPRECATED_code, "code", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent, "", "") \
    FUNCX(propScript_DEPRECATED_scriptName, "scriptName", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propScript_DEPRECATED_addOnPath, "addOnPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, "", "") \
    FUNCX(propScript_DEPRECATED_addOnMenuPath, "addOnMenuPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, "", "")

#define MARKER_PROPERTIES \
    FUNCX(propMarker_itemType, "itemType", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Type"}, {"description", "Item type"}}), "") \
    FUNCX(propMarker_cyclic, "cyclic", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Cyclic"}, {"description", "Item buffer is cyclic"}}), "") \
    FUNCX(propMarker_local, "local", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Local"}, {"description", "Coordinates are local to the marker's reference frame"}}), "") \
    FUNCX(propMarker_overlay, "overlay", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  jsonStr({{"label", "Overlay"}, {"description", "Items are are displayed overlaid"}}), "") \
    FUNCX(propMarker_points, "points", sim_propertytype_matrix, sim_propertyinfo_notwritable,  jsonStr({{"label", "Points"}, {"description", ""}}), "") \
    FUNCX(propMarker_quaternions, "quaternions", sim_propertytype_matrix, sim_propertyinfo_notwritable,  jsonStr({{"label", "Quaternions"}, {"description", ""}}), "") \
    FUNCX(propMarker_sizes, "sizes", sim_propertytype_matrix, sim_propertyinfo_notwritable,  jsonStr({{"label", "Sizes"}, {"description", ""}}), "") \
    FUNCX(propMarker_colors, "colors", sim_propertytype_buffer, sim_propertyinfo_notwritable,  jsonStr({{"label", "Colors"}, {"description", ""}}), "") \
    FUNCX(propMarker_packedPoints, "packedPoints", sim_propertytype_buffer, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Packed points"}, {"description", ""}}), "") \
    FUNCX(propMarker_packedQuaternions, "packedQuaternions", sim_propertytype_buffer, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Packed quaternions"}, {"description", ""}}), "") \
    FUNCX(propMarker_packedSizes, "packedSizes", sim_propertytype_buffer, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  jsonStr({{"label", "Packed sizes"}, {"description", ""}}), "") \
    FUNCX(propMarker_vertices, "vertices", sim_propertytype_matrix, sim_propertyinfo_notwritable,  jsonStr({{"label", "Vertices"}, {"description", "Vertices for custom marker"}}), "") \
    FUNCX(propMarker_indices, "indices", sim_propertytype_intarray, sim_propertyinfo_notwritable,  jsonStr({{"label", "Indices"}, {"description", "Indices for custom marker"}}), "") \
    FUNCX(propMarker_normals, "normals", sim_propertytype_matrix, sim_propertyinfo_notwritable,  jsonStr({{"label", "Normals"}, {"description", "Normals for custom marker"}}), "") \
    FUNCX(propMarker_METHOD_addItems, "addItems", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propMarker_METHOD_clearItems, "clearItems", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "") \
    FUNCX(propMarker_METHOD_removeItems, "removeItems", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  jsonStr({}), "")

#define CUSTOMSCENEOBJECT_PROPERTIES \
    FUNCX(propCustomSceneObject_size, "objectSize", sim_propertytype_float, 0,  jsonStr({{"label", "Size"}, {"description", "Custom scene object size"}}), "")

