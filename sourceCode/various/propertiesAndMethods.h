#pragma once

#include <string>
#include <vector>
#include <map>
#include <simLib/simConst.h>
#include <QVariantMap>
#include <QJsonObject>
#include <QJsonDocument>

struct PropertyInfo {
    QVariantMap map;
    std::string json;
    PropertyInfo(std::initializer_list<QPair<QString, QVariant>> init)
    {
        for(const auto& [k, v] : init)
            map.insert(k, v);
        json = QJsonDocument(QJsonObject::fromVariantMap(map)).toJson(QJsonDocument::Compact).toStdString();
    }
};

struct SProperty {
    const char* name;
    int type;
    int flags;
    const PropertyInfo info;
    int oldEnums[5];
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
#define proptypetag_enum "&enm&."
#define proptypetag_group "&grp&."

#define SIM_PROPERTYINFO_DEPRECATED (sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude)
#define SIM_PROPERTYINFO_METHOD (sim_propertyinfo_silent | sim_propertyinfo_constant | sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude)
#define SIM_PROPERTYINFO_GROUP (sim_propertyinfo_silent | sim_propertyinfo_constant | sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude)

#define OBJECT_PROPERTIES \
    FUNCX(objectType, "objectType", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, PropertyInfo({{"label", "Object type"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(handle, "handle", sim_propertytype_long, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, PropertyInfo({{"label", "Handle"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(metaInfoSuperClass, "metaInfo.superClass", sim_propertytype_stringarray, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, PropertyInfo({{"label", "object super class"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(metaInfoNameSpaces, "metaInfo.namespaces", sim_propertytype_stringarray, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, PropertyInfo({{"label", "object name spaces"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(metaInfoIsClass, "metaInfo.isClass", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, PropertyInfo({{"label", "class flag"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(metaInfoIsSceneObject, "metaInfo.isSceneObject", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, PropertyInfo({{"label", "sceneObject flag"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_metaInfo, "metaInfo", sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getBoolProperty, "getBoolProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getBufferProperty, "getBufferProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getColorProperty, "getColorProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getFloatArrayProperty, "getFloatArrayProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getFloatProperty, "getFloatProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getStringArrayProperty, "getStringArrayProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getHandleArrayProperty, "getHandleArrayProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getHandleProperty, "getHandleProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getIntArray2Property, "getIntArray2Property", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getIntArrayProperty, "getIntArrayProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getIntProperty, "getIntProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getEnumProperty, "getEnumProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getLongProperty, "getLongProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getPoseProperty, "getPoseProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getQuaternionProperty, "getQuaternionProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getStringProperty, "getStringProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getStrEnumProperty, "getStrEnumProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getTableProperty, "getTableProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getVector3Property, "getVector3Property", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setBoolProperty, "setBoolProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setBufferProperty, "setBufferProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setColorProperty, "setColorProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setFloatArrayProperty, "setFloatArrayProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setFloatProperty, "setFloatProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setStringArrayProperty, "setStringArrayProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setHandleArrayProperty, "setHandleArrayProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setHandleProperty, "setHandleProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setIntArray2Property, "setIntArray2Property", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setIntArrayProperty, "setIntArrayProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setIntProperty, "setIntProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setEnumProperty, "setEnumProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setLongProperty, "setLongProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setPoseProperty, "setPoseProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setQuaternionProperty, "setQuaternionProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setStringProperty, "setStringProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setStrEnumProperty, "setStrEnumProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setTableProperty, "setTableProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setVector3Property, "setVector3Property", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setMatrixProperty, "setMatrixProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getMatrixProperty, "getMatrixProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setMethodProperty, "setMethodProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getMethodProperty, "getMethodProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_removeProperty, "removeProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getPropertyInfo, "getPropertyInfo", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getPropertyInfos, "getPropertyInfos", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getPropertyName, "getPropertyName", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getProperties, "getProperties", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setProperties, "setProperties", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setProperty, "setProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getPropertiesInfos, "getPropertiesInfos", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getProperty, "getProperty", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getPropertyTypeString, "getPropertyTypeString", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_isValid, "isValid", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \

#define APP_PROPERTIES \
    FUNCX(sessionId, "sessionId", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Session ID"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(protocolVersion, "protocolVersion", sim_propertytype_int, 0,  PropertyInfo({{"label", "Protocol"}, {"description", "Protocol version"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(productVersion, "productVersion", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Product string"}, {"description", "Product version (string)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(productVersionNb, "productVersionNb", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Product"}, {"description", "Product version (number)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(platform, "platform", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Platform"}, {"description", "Platform (0: Windows, 1: macOS, 2: Linux)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(flavor, "flavor", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Flavor"}, {"description", "Flavor (0: lite, 1: edu, 2: pro)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(qtVersion, "qtVersion", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Qt"}, {"description", "Qt version"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(processId, "processId", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Process"}, {"description", "Process ID"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(processCnt, "processCnt", sim_propertytype_int, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Processes"}, {"description", "Overall processes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(consoleVerbosity, "consoleVerbosity", sim_propertytype_int, 0,  PropertyInfo({{"label", "Console verbosity"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(statusbarVerbosity, "statusbarVerbosity", sim_propertytype_int, 0,  PropertyInfo({{"label", "Statusbar verbosity"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(dialogVerbosity, "dialogVerbosity", sim_propertytype_int, 0,  PropertyInfo({{"label", "Dialog verbosity"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(consoleVerbosityStr, "consoleVerbosityStr", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Console verbosity string"}, {"description", "Console verbosity string, only for client app"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(statusbarVerbosityStr, "statusbarVerbosityStr", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Statusbar verbosity string"}, {"description", "Statusbar verbosity string, only for client app"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(dialogVerbosityStr, "dialogVerbosityStr", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Dialog verbosity string"}, {"description", "Dialog verbosity string, only for client app"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(auxAddOn1, "auxAddOn1", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Aux. add-on 1"}, {"description", "Auxiliary add-on 1"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(auxAddOn2, "auxAddOn2", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Aux. add-on 2"}, {"description", "Auxiliary add-on 2"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(startupCode, "startupCode", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Start-up code"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(defaultTranslationStepSize, "defaultTranslationStepSize", sim_propertytype_float, 0,  PropertyInfo({{"label", "Translation step size"}, {"description", "Default translation step size"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(defaultRotationStepSize, "defaultRotationStepSize", sim_propertytype_float, 0,  PropertyInfo({{"label", "Rotation step size"}, {"description", "Default rotation step size"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(hierarchyEnabled, "hierarchyEnabled", sim_propertytype_bool, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Hierarchy enabled"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(browserEnabled, "browserEnabled", sim_propertytype_bool, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Browser enabled"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(displayEnabled, "displayEnabled", sim_propertytype_bool, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Display enabled"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(machineId, "machineId", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Machine ID"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(legacyMachineId, "legacyMachineId", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Legacy machine ID"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(appDir, PATHSPREFIXDOT "app", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Application path"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(tempDir, PATHSPREFIXDOT "temp", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Temporary path"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(sceneTempDir, PATHSPREFIXDOT "sceneTemp", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Scene temporary path"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(settingsDir, PATHSPREFIXDOT "settings", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Settings path"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(luaDir, PATHSPREFIXDOT "lua", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Lua path"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(pythonDir, PATHSPREFIXDOT "python", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Python path"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(mujocoDir, PATHSPREFIXDOT "mujoco", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "MuJoCo path"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(systemDir, PATHSPREFIXDOT "system", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "System path"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(resourceDir, PATHSPREFIXDOT "resources", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Resource path"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(addOnDir, PATHSPREFIXDOT "addOns", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Add-on path"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(sceneDir, PATHSPREFIXDOT "scenes", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Scene path"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(modelDir, PATHSPREFIXDOT "models", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Model path"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(importExportDir, PATHSPREFIXDOT "importExport", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Import/export path"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(defaultPython, "defaultPython", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Default Python"}, {"description", "Default Python interpreter"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(sandboxLang, "sandboxLang", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Sandbox language"}, {"description", "Default sandbox language"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(headlessMode, "headlessMode", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Headless mode"}, {"description", "Headless mode (0: not headless, 1: GUI suppressed, 2: headless library)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(canSave, "canSave", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Can save"}, {"description", "Whether save operation is allowed in given state"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(idleFps, "idleFps", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Loaded plugin names"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(pluginNames, "pluginNames", sim_propertytype_stringarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Plugins"}, {"description", "List of plugins"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(addOns, "addOns", sim_propertytype_handlearray, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Add-ons"}, {"description", "List of add-ons"}, {"handleType", "detachedScript"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(customObjects, "customObjects", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Custom objects"}, {"description", "List of app custom objects"}, {"handleType", "customObject"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(customClasses, "customClasses", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Custom classes"}, {"description", "List of app custom classes"}, {"handleType", "customObject"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(customSceneObjectClasses, "customSceneObjectClasses", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Custom scene object classes"}, {"description", "List of app custom scene object classes"}, {"handleType", "sceneObject"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(sandbox, "sandbox", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Sandbox"}, {"description", "Handle of the sandbox script"}, {"handleType", "detachedScript"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(appArgs, "args", sim_propertytype_stringarray, sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "App args"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(randomQuaternion, "randomQuaternion", sim_propertytype_quaternion, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Random quaternion"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(randomFloat, "randomFloat", sim_propertytype_float, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Random number"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(randomString, "randomString", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Random string"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(notifyDeprecated, "notifyDeprecated", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Notify deprecated"}, {"description", "Notify deprecated API (0: off, 1: light, 2: full)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(execUnsafe, "execUnsafe", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Execute unsafe"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(execUnsafeExt, "execUnsafeExt", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Execute unsafe extended"}, {"description", "Execute unsafe for code triggered externally"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(dongleSerial, "dongleSerial", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(machineSerialND, "machineSerialND", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(machineSerial, "machineSerial", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(dongleID, "dongleId", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(machineIDX, "machineIdX", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(machineID0, "machineId0", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(machineID1, "machineId1", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(machineID2, "machineId2", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(machineID3, "machineId3", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(pid, "pid", sim_propertytype_long, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "PID"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(systemTime, "systemTime", sim_propertytype_float, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "System time"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_namedParam, NAMEDPARAMPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_customData, CUSTOMDATAPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_signal, SIGNALPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_paths, PATHSPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_handleAddOnScripts, "handleAddOnScripts", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_handleSandboxScript, "handleSandboxScript", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_loadModelThumbnail, "loadModelThumbnail", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_loadModelThumbnailFromBuffer, "loadModelThumbnailFromBuffer", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_loadScene, "loadScene", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_loadSceneFromBuffer, "loadSceneFromBuffer", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getObjects, "getObjects", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_createObject, "createObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_duplicateObjects, "duplicateObjects", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_loadImage, "loadImage", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_loadImageFromBuffer, "loadImageFromBuffer", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_saveImage, "saveImage", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_saveImageToBuffer, "saveImageToBuffer", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_transformImage, "transformImage", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_removeObjects, "removeObjects", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_logInfo, "logInfo", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_logWarn, "logWarn", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_logError, "logError", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_systemLock, "systemLock", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_pack, "pack", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_unpack, "unpack", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_serialize, "serialize", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_deserialize, "deserialize", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_packDoubleArray, "packDoubleArray", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_packFloatArray, "packFloatArray", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_packInt64Array, "packInt64Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_packInt32Array, "packInt32Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_packUInt32Array, "packUInt32Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_packInt16Array, "packInt16Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_packUInt16Array, "packUInt16Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_packInt8Array, "packInt8Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_packUInt8Array, "packUInt8Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_unpackDoubleArray, "unpackDoubleArray", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_unpackFloatArray, "unpackFloatArray", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_unpackInt64Array, "unpackInt64Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_unpackInt32Array, "unpackInt32Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_unpackUInt32Array, "unpackUInt32Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_unpackInt16Array, "unpackInt16Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_unpackUInt16Array, "unpackUInt16Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_unpackInt8Array, "unpackInt8Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_unpackUInt8Array, "unpackUInt8Array", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_fastIdleLoop, "fastIdleLoop", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_throttle, "throttle", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_scheduleExecution, "scheduleExecution", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_cancelScheduledExecution, "cancelScheduledExecution", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_openFile, "openFile", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_createCustomObjectClass, "createCustomObjectClass", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_quit, "quit", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_pushEvent, "pushEvent", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getGenesisEvents, "getGenesisEvents", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getPluginInfo, "getPluginInfo", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setPluginInfo, "setPluginInfo", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATED_dongleID, "dongleID", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_machineIDX, "machineIDX", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_machineID0, "machineID0", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_machineID1, "machineID1", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_machineID2, "machineID2", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_machineID3, "machineID3", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_appDir, "appPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_tempDir, "tempPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_sceneTempDir, "sceneTempPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_settingsDir, "settingsPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_luaDir, "luaPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_pythonDir, "pythonPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_mujocoDir, "mujocoPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_systemDir, "systemPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_resourceDir, "resourcePath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_addOnDir, "addOnPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_sceneDir, "scenePath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_modelDir, "modelPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_importExportDir, "importExportPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_appArg1, "appArg1", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "App arg. 1"}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_appArg2, "appArg2", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "App arg. 2"}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_appArg3, "appArg3", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "App arg. 3"}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_appArg4, "appArg4", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "App arg. 4"}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_appArg5, "appArg5", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "App arg. 5"}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_appArg6, "appArg6", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "App arg. 6"}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_appArg7, "appArg7", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "App arg. 7"}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_appArg8, "appArg8", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "App arg. 8"}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_appArg9, "appArg9", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "App arg. 9"}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \

#define CUSTOMOBJECTCLASS_PROPERTIES \
    FUNCX(METHOD_makeOject, "makeObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_remove, "remove", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \

#define CUSTOMOBJECT_PROPERTIES \
    FUNCX(METHOD_remove, "remove", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \

#define DETACHEDSCRIPT_PROPERTIES \
    FUNCX(scriptDisabled, "disabled", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Disabled"}, {"description", "Distabled state"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(restartOnError, "restartOnError", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Restart"}, {"description", "Restart on error"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(execPriority, "execPriority", sim_propertytype_int, 0,  PropertyInfo({{"label", "Execution priority"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(scriptType, "type", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Type"}, {"description", "Script type"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(executionDepth, "executionDepth", sim_propertytype_int, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Execution depth"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(scriptState, "state", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "State"}, {"description", "Script state"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(language, "language", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Language"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(code, "code", sim_propertytype_string, 0,  PropertyInfo({{"label", "Code"}, {"description", "Script content"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(scriptName, "scriptName", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Name"}, {"description", "Script name"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(addOnPath, "addOnPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Add-on path"}, {"description", "Path of add-on"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(addOnMenuPath, "addOnMenuPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Add-on menu path"}, {"description", "Menu path of add-on"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(autoYieldDelay, "autoYieldDelay", sim_propertytype_float, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Automatic yield delay"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_callFunction, "callFunction", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getFunctions, "getFunctions", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_executeString, "executeString", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
/*    FUNCX(METHOD_getApiFunc, "getApiFunc", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) */ \
/*    FUNCX(METHOD_getApiInfo, "getApiInfo", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) */ \
    FUNCX(METHOD_getStackTraceback, "getStackTraceback", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_remove, "remove", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_registerFunctionHook, "registerFunctionHook", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_removeFunctionHook, "removeFunctionHook", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_lock, "lock", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getObject, "getObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setStepping, "setStepping", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getStepping, "getStepping", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_yield, "yield", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_step, "step", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_wait, "wait", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_init, "init", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setEventFilters, "setEventFilters", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_broadcast, "broadcast", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATED_scriptType, "scriptType", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"", ""}, {"", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_scriptDisabled, "scriptDisabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"", ""}, {"", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_scriptState, "scriptState", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,  PropertyInfo({{"", ""}, {"", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \

#define STACK_PROPERTIES \
    FUNCX(content, "content", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Content"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \

#define COLLECTIONCONT_PROPERTIES \
    FUNCX(collections, "collections", sim_propertytype_handlearray, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Collections"}, {"description", "Handles of all collections"}, {"handleType", "collection"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \

#define COLLECTION_PROPERTIES \
    FUNCX(objects, "objects", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Children handles"}, {"description", ""}, {"handleType", "sceneObject"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_addItem, "addItem", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_removeItem, "removeItem", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_checkCollision, "checkCollision", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_remove, "remove", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_changeColor, "changeColor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_restoreColor, "restoreColor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_checkDistance, "checkDistance", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \

#define DRAWINGOBJECTCONT_PROPERTIES \
    FUNCX(drawingObjects, "drawingObjects", sim_propertytype_handlearray, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Drawing objects"}, {"description", "Handles of all drawing objects"}, {"handleType", "drawingObject"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \

#define DRAWINGOBJECT_PROPERTIES \
    FUNCX(parent, "parent", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Parent handle"}, {"description", ""}, {"handleType", "sceneObject"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_remove, "remove", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \

#define COLOR_PROPERTIES /* do not change order of following 5 properties!! */ \
    FUNCX(colDiffuse, "diffuse", sim_propertytype_color, 0,  PropertyInfo({{"label", "Diffuse color"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(colSpecular, "specular", sim_propertytype_color, 0,  PropertyInfo({{"label", "Specular color"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(colEmission, "emission", sim_propertytype_color, 0,  PropertyInfo({{"label", "Emission color"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(transparency, "transparency", sim_propertytype_float, 0,  PropertyInfo({{"label", "Transparency"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_prefix, COLORPREFIXTAG, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \

#define CONVEXVOLUME_PROPERTIES \
    FUNCX(closeThreshold, "closeThreshold", sim_propertytype_float, 0,  PropertyInfo({{"label", "Close threshold"}, {"description", "Close threshold: if a detection occures below that threshold, it is not registered. 0.0 to disable"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(offset, VOLUMEPREFIXDOT "offset", sim_propertytype_float, 0,  PropertyInfo({{"label", "Offset"}, {"description", "Offset of detection volume"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(range, VOLUMEPREFIXDOT "range", sim_propertytype_float, 0,  PropertyInfo({{"label", "Range"}, {"description", "Range/depth of detection volume"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(xSize, VOLUMEPREFIXDOT "xSize", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", "X-sizes"}, {"description", "X-size (near and far) for pyramid-type volumes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(ySize, VOLUMEPREFIXDOT "ySize", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", "Y-sizes"}, {"description", "Y-size (near and far) for pyramid-type volumes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(radius, VOLUMEPREFIXDOT "radius", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", "Radius"}, {"description", "Radius for cylinder-, disk- and cone-type volumes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(angle, VOLUMEPREFIXDOT "angle", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", "Angles"}, {"description", "Angle and inside gap for disk- and cone-type volumes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(faces, VOLUMEPREFIXDOT "faces", sim_propertytype_intarray, 0,  PropertyInfo({{"label", "Faces"}, {"description", "Number of faces (near and far) for cylinder-, disk- and pyramid-type volumes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(subdivisions, VOLUMEPREFIXDOT "subdivisions", sim_propertytype_intarray, 0,  PropertyInfo({{"label", "Subdivisions"}, {"description", "Number of subdivisions (near and far) for cone-type volumes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(edges, VOLUMEPREFIXDOT "edges", sim_propertytype_floatarray, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Volume edges"}, {"description", "List of segments (defined by pairs of end-point coordinates) visualizing the volume"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(closeEdges, VOLUMEPREFIXDOT "closeEdges", sim_propertytype_floatarray, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Volume close edges"}, {"description", "List of segments (defined by pairs of end-point coordinates) visualizing the close threshold of the volume"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_volume, VOLUMEPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATED_offset, "volume_offset", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_range, "volume_range", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_xSize, "volume_xSize", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_ySize, "volume_ySize", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_radius, "volume_radius", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_angle, "volume_angle", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_faces, "volume_faces", sim_propertytype_intarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_subdivisions, "volume_subdivisions", sim_propertytype_intarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_edges, "volume_edges", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_closeEdges, "volume_closeEdges", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \

#define DYNCONT_PROPERTIES \
    FUNCX(dynamicsEnabled, DYNAMICSPREFIXDOT "enabled", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Dynamics enabled"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1)\
    FUNCX(showContactPoints, DYNAMICSPREFIXDOT "showContactPoints", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Show contact points"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(dynamicsEngine, DYNAMICSPREFIXDOT "engine", sim_propertytype_intarray, 0,  PropertyInfo({{"label", "Dynamics engine"}, {"description", "Selected dynamics engine index and version"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(dynamicsStepSize, DYNAMICSPREFIXDOT "stepSize", sim_propertytype_float, 0,  PropertyInfo({{"label", "Dynamics dt"}, {"description", "Dynamics step size"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(gravity, DYNAMICSPREFIXDOT "gravity", sim_propertytype_vector3, 0,  PropertyInfo({{"label", "Gravity"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(engineProperties, DYNAMICSPREFIXDOT "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Engine properties"}, {"description", "Engine properties as JSON text"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(bulletSolver, DYNAMICSPREFIXDOT BULLETPREFIXDOT "solver", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_global_constraintsolvertype, -1, -1, -1, -1) \
    FUNCX(bulletIterations, DYNAMICSPREFIXDOT BULLETPREFIXDOT "iterations", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_global_constraintsolvingiterations, -1, -1, -1, -1) \
    FUNCX(bulletComputeInertias, DYNAMICSPREFIXDOT BULLETPREFIXDOT "computeInertias", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_global_computeinertias, -1, -1, -1, -1) \
    FUNCX(bulletInternalScalingFull, DYNAMICSPREFIXDOT BULLETPREFIXDOT "internalScalingFull", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_global_fullinternalscaling, -1, -1, -1, -1) \
    FUNCX(bulletInternalScalingScaling, DYNAMICSPREFIXDOT BULLETPREFIXDOT "internalScalingValue", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_global_internalscalingfactor, -1, -1, -1, -1) \
    FUNCX(bulletCollMarginScaling, DYNAMICSPREFIXDOT BULLETPREFIXDOT "collisionMarginScaling", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_global_collisionmarginfactor, -1, -1, -1, -1) \
    FUNCX(odeQuickStepEnabled, DYNAMICSPREFIXDOT ODEPREFIXDOT "quickStepEnabled", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_global_quickstep, -1, -1, -1, -1) \
    FUNCX(odeQuickStepIterations, DYNAMICSPREFIXDOT ODEPREFIXDOT "quickStepIterations", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_global_constraintsolvingiterations, -1, -1, -1, -1) \
    FUNCX(odeComputeInertias, DYNAMICSPREFIXDOT ODEPREFIXDOT "computeInertias", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_global_computeinertias, -1, -1, -1, -1) \
    FUNCX(odeInternalScalingFull, DYNAMICSPREFIXDOT ODEPREFIXDOT "internalScalingFull", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_global_fullinternalscaling, -1, -1, -1, -1) \
    FUNCX(odeInternalScalingScaling, DYNAMICSPREFIXDOT ODEPREFIXDOT "internalScalingValue", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_global_internalscalingfactor, -1, -1, -1, -1) \
    FUNCX(odeGlobalErp, DYNAMICSPREFIXDOT ODEPREFIXDOT "globalErp", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_global_erp, -1, -1, -1, -1) \
    FUNCX(odeGlobalCfm, DYNAMICSPREFIXDOT ODEPREFIXDOT "globalCfm", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_global_cfm, -1, -1, -1, -1) \
    FUNCX(vortexComputeInertias, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "computeInertias", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_global_computeinertias, -1, -1, -1, -1) \
    FUNCX(vortexContactTolerance, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "contactTolerance", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_global_contacttolerance, -1, -1, -1, -1) \
    FUNCX(vortexAutoSleep, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoSleep", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_global_autosleep, -1, -1, -1, -1) \
    FUNCX(vortexMultithreading, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "multithreading", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_global_multithreading, -1, -1, -1, -1) \
    FUNCX(vortexConstraintsLinearCompliance, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "constraintsLinearCompliance", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_global_constraintlinearcompliance, -1, -1, -1, -1) \
    FUNCX(vortexConstraintsLinearDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "constraintsLinearDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_global_constraintlineardamping, -1, -1, -1, -1) \
    FUNCX(vortexConstraintsLinearKineticLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "constraintsLinearKineticLoss", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_global_constraintlinearkineticloss, -1, -1, -1, -1) \
    FUNCX(vortexConstraintsAngularCompliance, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "constraintsAngularCompliance", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_global_constraintangularcompliance, -1, -1, -1, -1) \
    FUNCX(vortexConstraintsAngularDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "constraintsAngularDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_global_constraintangulardamping, -1, -1, -1, -1) \
    FUNCX(vortexConstraintsAngularKineticLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "constraintsAngularKineticLoss", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_global_constraintangularkineticloss, -1, -1, -1, -1) \
    FUNCX(newtonIterations, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "iterations", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_newton_global_constraintsolvingiterations, -1, -1, -1, -1) \
    FUNCX(newtonComputeInertias, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "computeInertias", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_newton_global_computeinertias, -1, -1, -1, -1) \
    FUNCX(newtonMultithreading, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "multithreading", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_newton_global_multithreading, -1, -1, -1, -1) \
    FUNCX(newtonExactSolver, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "exactSolver", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_newton_global_exactsolver, -1, -1, -1, -1) \
    FUNCX(newtonHighJointAccuracy, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "highJointAccuracy", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_newton_global_highjointaccuracy, -1, -1, -1, -1) \
    FUNCX(newtonContactMergeTolerance, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "contactMergeTolerance", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_newton_global_contactmergetolerance, -1, -1, -1, -1) \
    FUNCX(mujocoIntegrator, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "integrator", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_integrator, -1, -1, -1, -1) \
    FUNCX(mujocoSolver, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "solver", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_solver, -1, -1, -1, -1) \
    FUNCX(mujocoIterations, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "iterations", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_iterations, -1, -1, -1, -1) \
    FUNCX(mujocoRebuildTrigger, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "rebuildTrigger", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_rebuildtrigger, -1, -1, -1, -1) \
    FUNCX(mujocoComputeInertias, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "computeInertias", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_computeinertias, -1, -1, -1, -1) \
    FUNCX(mujocoMbMemory, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "mbmemory", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoNjMax, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "njmax", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", "Deprecated. See mujoco.mbmemory instead."}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_njmax, -1, -1, -1, -1) \
    FUNCX(mujocoNconMax, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "nconmax", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", "Deprecated. See mujoco.mbmemory instead."}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_nconmax, -1, -1, -1, -1) \
    FUNCX(mujocoNstack, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "nstack", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", "Deprecated. See mujoco.mbmemory instead."}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_nstack, -1, -1, -1, -1) \
    FUNCX(mujocoCone, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "cone", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_cone, -1, -1, -1, -1) \
    FUNCX(mujocoKinematicBodiesOverrideFlags, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "kinematicBodiesOverrideFlags", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_overridekin, -1, -1, -1, -1) \
    FUNCX(mujocoKinematicBodiesMass, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "kinematicBodiesMass", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_kinmass, -1, -1, -1, -1) \
    FUNCX(mujocoKinematicBodiesInertia, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "kinematicBodiesInertia", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_kininertia, -1, -1, -1, -1) \
    FUNCX(mujocoBoundMass, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "boundMass", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_boundmass, -1, -1, -1, -1) \
    FUNCX(mujocoBoundInertia, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "boundInertia", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_boundinertia, -1, -1, -1, -1) \
    FUNCX(mujocoBalanceInertias, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "balanceInertias", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_balanceinertias, -1, -1, -1, -1) \
    FUNCX(mujocoMultithreaded, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "multithreaded", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_multithreaded, -1, -1, -1, -1) \
    FUNCX(mujocoMulticcd, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "multiccd", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_multiccd, -1, -1, -1, -1) \
    FUNCX(mujocoContactParamsOverride, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "contactParamsOverride", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_overridecontacts, -1, -1, -1, -1) \
    FUNCX(mujocoContactParamsMargin, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "contactParamsMargin", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_overridemargin, -1, -1, -1, -1) \
    FUNCX(mujocoContactParamsSolref, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "contactParamsSolref", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_overridesolref1, sim_mujoco_global_overridesolref2, -1, -1, -1) \
    FUNCX(mujocoContactParamsSolimp, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "contactParamsSolimp", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_overridesolimp1, sim_mujoco_global_overridesolimp2, sim_mujoco_global_overridesolimp3, sim_mujoco_global_overridesolimp4, sim_mujoco_global_overridesolimp5) \
    FUNCX(mujocoImpRatio, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "impratio", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_impratio, -1, -1, -1, -1) \
    FUNCX(mujocoWind, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "wind", sim_propertytype_vector3, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_wind1, sim_mujoco_global_wind2, sim_mujoco_global_wind3, -1, -1) \
    FUNCX(mujocoDensity, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "density", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_density, -1, -1, -1, -1) \
    FUNCX(mujocoViscosity, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "viscosity", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_global_viscosity, -1, -1, -1, -1) \
    FUNCX(mujocoJacobian, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "jacobian", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoTolerance, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "tolerance", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoLs_iterations, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "ls_iterations", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoLs_tolerance, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "ls_tolerance", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoNoslip_iterations, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "noslip_iterations", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoNoslip_tolerance, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "noslip_tolerance", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoCcd_iterations, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "ccd_iterations", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoCcd_tolerance, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "ccd_tolerance", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoSdf_iterations, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "sdf_iterations", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoSdf_initpoints, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "sdf_initpoints", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoEqualityEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "equalityEnable", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoFrictionlossEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "frictionlossEnable", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoLimitEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "limitEnable", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoContactEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "contactEnable", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoPassiveEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "passiveEnable", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoGravityEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "gravityEnable", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoWarmstartEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "warmstartEnable", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoActuationEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "actuationEnable", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoRefsafeEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "refsafeEnable", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoSensorEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "sensorEnable", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoMidphaseEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "midphaseEnable", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoEulerdampEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "eulerdampEnable", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoAutoresetEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "autoresetEnable", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoEnergyEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "energyEnable", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoInvdiscreteEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "invdiscreteEnable", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoNativeccdEnable, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "nativeccdEnable", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoAlignfree, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "alignfree", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoKinematicWeldSolref, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "kinematicWeldSolref", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoKinematicWeldSolimp, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "kinematicWeldSolimp", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoKinematicWeldTorqueScale, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "kinematicWeldTorquescale", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamics, DYNAMICSPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamicsBullet, DYNAMICSPREFIXDOT BULLETPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamicsOde, DYNAMICSPREFIXDOT ODEPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamicsNewton, DYNAMICSPREFIXDOT NEWTONPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamicsVortex, DYNAMICSPREFIXDOT VORTEXPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamicsMujoco, DYNAMICSPREFIXDOT MUJOCOPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATEDGROUP_dynamicsBullet, BULLETPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP | SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATEDGROUP_dynamicsOde, ODEPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP | SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATEDGROUP_dynamicsNewton, NEWTONPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP | SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATEDGROUP_dynamicsVortex, VORTEXPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP | SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATEDGROUP_dynamicsMujoco, MUJOCOPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP | SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_dynamicsEnabled, "dynamicsEnabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", "Dynamics enabled"}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_showContactPoints, "showContactPoints", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", "Show contact points"}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_dynamicsEngine, "dynamicsEngine", sim_propertytype_intarray, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", "Dynamics engine"}, {"description", "Selected dynamics engine index and version"}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_dynamicsStepSize, "dynamicsStepSize", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", "Dynamics dt"}, {"description", "Dynamics step size"}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_gravity, "gravity", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", "Gravity"}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_engineProperties, "engineProperties", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", "Engine properties"}, {"description", "Engine properties as JSON text"}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletSolver, BULLETPREFIXDOT "solver", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_global_constraintsolvertype, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletIterations, BULLETPREFIXDOT "iterations", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_global_constraintsolvingiterations, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletComputeInertias, BULLETPREFIXDOT "computeInertias", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_global_computeinertias, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletInternalScalingFull, BULLETPREFIXDOT "internalScalingFull", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_global_fullinternalscaling, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletInternalScalingScaling, BULLETPREFIXDOT "internalScalingValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_global_internalscalingfactor, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletCollMarginScaling, BULLETPREFIXDOT "collisionMarginScaling", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_global_collisionmarginfactor, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odeQuickStepEnabled, ODEPREFIXDOT "quickStepEnabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_global_quickstep, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odeQuickStepIterations, ODEPREFIXDOT "quickStepIterations", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_global_constraintsolvingiterations, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odeComputeInertias, ODEPREFIXDOT "computeInertias", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_global_computeinertias, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odeInternalScalingFull, ODEPREFIXDOT "internalScalingFull", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_global_fullinternalscaling, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odeInternalScalingScaling, ODEPREFIXDOT "internalScalingValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_global_internalscalingfactor, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odeGlobalErp, ODEPREFIXDOT "globalErp", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_global_erp, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odeGlobalCfm, ODEPREFIXDOT "globalCfm", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_global_cfm, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexComputeInertias, VORTEXPREFIXDOT "computeInertias", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_global_computeinertias, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexContactTolerance, VORTEXPREFIXDOT "contactTolerance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_global_contacttolerance, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexAutoSleep, VORTEXPREFIXDOT "autoSleep", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_global_autosleep, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexMultithreading, VORTEXPREFIXDOT "multithreading", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_global_multithreading, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexConstraintsLinearCompliance, VORTEXPREFIXDOT "constraintsLinearCompliance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_global_constraintlinearcompliance, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexConstraintsLinearDamping, VORTEXPREFIXDOT "constraintsLinearDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_global_constraintlineardamping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexConstraintsLinearKineticLoss, VORTEXPREFIXDOT "constraintsLinearKineticLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_global_constraintlinearkineticloss, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexConstraintsAngularCompliance, VORTEXPREFIXDOT "constraintsAngularCompliance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_global_constraintangularcompliance, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexConstraintsAngularDamping, VORTEXPREFIXDOT "constraintsAngularDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_global_constraintangulardamping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexConstraintsAngularKineticLoss, VORTEXPREFIXDOT "constraintsAngularKineticLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_global_constraintangularkineticloss, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_newtonIterations, NEWTONPREFIXDOT "iterations", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_newton_global_constraintsolvingiterations, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_newtonComputeInertias, NEWTONPREFIXDOT "computeInertias", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_newton_global_computeinertias, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_newtonMultithreading, NEWTONPREFIXDOT "multithreading", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_newton_global_multithreading, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_newtonExactSolver, NEWTONPREFIXDOT "exactSolver", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_newton_global_exactsolver, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_newtonHighJointAccuracy, NEWTONPREFIXDOT "highJointAccuracy", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_newton_global_highjointaccuracy, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_newtonContactMergeTolerance, NEWTONPREFIXDOT "contactMergeTolerance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_newton_global_contactmergetolerance, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoIntegrator, MUJOCOPREFIXDOT "integrator", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_integrator, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoSolver, MUJOCOPREFIXDOT "solver", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_solver, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoIterations, MUJOCOPREFIXDOT "iterations", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_iterations, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoRebuildTrigger, MUJOCOPREFIXDOT "rebuildTrigger", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_rebuildtrigger, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoComputeInertias, MUJOCOPREFIXDOT "computeInertias", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_computeinertias, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoMbMemory, MUJOCOPREFIXDOT "mbmemory", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoNjMax, MUJOCOPREFIXDOT "njmax", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", "Deprecated. See mujoco.mbmemory instead."}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_njmax, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoNconMax, MUJOCOPREFIXDOT "nconmax", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", "Deprecated. See mujoco.mbmemory instead."}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_nconmax, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoNstack, MUJOCOPREFIXDOT "nstack", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", "Deprecated. See mujoco.mbmemory instead."}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_nstack, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoCone, MUJOCOPREFIXDOT "cone", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_cone, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoKinematicBodiesOverrideFlags, MUJOCOPREFIXDOT "kinematicBodiesOverrideFlags", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_overridekin, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoKinematicBodiesMass, MUJOCOPREFIXDOT "kinematicBodiesMass", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_kinmass, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoKinematicBodiesInertia, MUJOCOPREFIXDOT "kinematicBodiesInertia", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_kininertia, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoBoundMass, MUJOCOPREFIXDOT "boundMass", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_boundmass, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoBoundInertia, MUJOCOPREFIXDOT "boundInertia", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_boundinertia, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoBalanceInertias, MUJOCOPREFIXDOT "balanceInertias", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_balanceinertias, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoMultithreaded, MUJOCOPREFIXDOT "multithreaded", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_multithreaded, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoMulticcd, MUJOCOPREFIXDOT "multiccd", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_multiccd, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoContactParamsOverride, MUJOCOPREFIXDOT "contactParamsOverride", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_overridecontacts, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoContactParamsMargin, MUJOCOPREFIXDOT "contactParamsMargin", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_overridemargin, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoContactParamsSolref, MUJOCOPREFIXDOT "contactParamsSolref", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_overridesolref1, sim_mujoco_global_overridesolref2, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoContactParamsSolimp, MUJOCOPREFIXDOT "contactParamsSolimp", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_overridesolimp1, sim_mujoco_global_overridesolimp2, sim_mujoco_global_overridesolimp3, sim_mujoco_global_overridesolimp4, sim_mujoco_global_overridesolimp5) \
    FUNCX(DEPRECATED_mujocoImpRatio, MUJOCOPREFIXDOT "impratio", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_impratio, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoWind, MUJOCOPREFIXDOT "wind", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_wind1, sim_mujoco_global_wind2, sim_mujoco_global_wind3, -1, -1) \
    FUNCX(DEPRECATED_mujocoDensity, MUJOCOPREFIXDOT "density", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_density, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoViscosity, MUJOCOPREFIXDOT "viscosity", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_global_viscosity, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoJacobian, MUJOCOPREFIXDOT "jacobian", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoTolerance, MUJOCOPREFIXDOT "tolerance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoLs_iterations, MUJOCOPREFIXDOT "ls_iterations", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoLs_tolerance, MUJOCOPREFIXDOT "ls_tolerance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoNoslip_iterations, MUJOCOPREFIXDOT "noslip_iterations", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoNoslip_tolerance, MUJOCOPREFIXDOT "noslip_tolerance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoCcd_iterations, MUJOCOPREFIXDOT "ccd_iterations", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoCcd_tolerance, MUJOCOPREFIXDOT "ccd_tolerance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoSdf_iterations, MUJOCOPREFIXDOT "sdf_iterations", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoSdf_initpoints, MUJOCOPREFIXDOT "sdf_initpoints", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoEqualityEnable, MUJOCOPREFIXDOT "equalityEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoFrictionlossEnable, MUJOCOPREFIXDOT "frictionlossEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoLimitEnable, MUJOCOPREFIXDOT "limitEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoContactEnable, MUJOCOPREFIXDOT "contactEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoPassiveEnable, MUJOCOPREFIXDOT "passiveEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoGravityEnable, MUJOCOPREFIXDOT "gravityEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoWarmstartEnable, MUJOCOPREFIXDOT "warmstartEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoActuationEnable, MUJOCOPREFIXDOT "actuationEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoRefsafeEnable, MUJOCOPREFIXDOT "refsafeEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoSensorEnable, MUJOCOPREFIXDOT "sensorEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoMidphaseEnable, MUJOCOPREFIXDOT "midphaseEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoEulerdampEnable, MUJOCOPREFIXDOT "eulerdampEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoAutoresetEnable, MUJOCOPREFIXDOT "autoresetEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoEnergyEnable, MUJOCOPREFIXDOT "energyEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoInvdiscreteEnable, MUJOCOPREFIXDOT "invdiscreteEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoNativeccdEnable, MUJOCOPREFIXDOT "nativeccdEnable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoAlignfree, MUJOCOPREFIXDOT "alignfree", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoKinematicWeldSolref, MUJOCOPREFIXDOT "kinematicWeldSolref", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoKinematicWeldSolimp, MUJOCOPREFIXDOT "kinematicWeldSolimp", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoKinematicWeldTorqueScale, MUJOCOPREFIXDOT "kinematicWeldTorquescale", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \

#define SCENE_PROPERTIES \
    FUNCX(mainScript, "mainScript", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Main script"}, {"description", "Handle of the main script"}, {"handleType", "detachedScript"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(finalSaveRequest, "finalSaveRequest", sim_propertytype_bool, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Final save"}, {"description", "Lock scene and models after next scene save operation"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(saveCalculationStructs, "saveCalculationStructs", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Save calculation structures"}, {"description", "Save operation also saves existing calculation structures"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(visibilityLayers, "visibilityLayers", sim_propertytype_int, 0,  PropertyInfo({{"label", "Visibility layers"}, {"description", "Currently active visibility layers"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(sceneIsLocked, "locked", sim_propertytype_bool, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Scene is locked"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(sceneUid, "uid", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Scene UID"}, {"description", "Scene unique identifier"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(sceneUidString, "uidString", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Scene UID string"}, {"description", "Scene unique identifier string"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(scenePath, "path", sim_propertytype_string, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Scene path"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(acknowledgment, "acknowledgment", sim_propertytype_string, 0,  PropertyInfo({{"label", "Acknowledgment"}, {"description", "Scene acknowledgment"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(ambientLight, "ambientLight", sim_propertytype_color, 0,  PropertyInfo({{"label", "Ambient light"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(customObjects, "customObjects", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Custom objects"}, {"description", "List of scene custom objects"}, {"handleType", "customObject"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_customData, CUSTOMDATAPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_signal, SIGNALPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_createObject, "createObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_removeObjects, "removeObjects", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_duplicateObjects, "duplicateObjects", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_handleSimulationScripts, "handleSimulationScripts", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_handleCustomizationScripts, "handleCustomizationScripts", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getDescendants, "getDescendants", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getObjects, "getObjects", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_loadModel, "loadModel", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_loadModelFromBuffer, "loadModelFromBuffer", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_save, "save", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_saveToBuffer, "saveToBuffer", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getObject, "getObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_remove, "remove", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_announceChange, "announceChange", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getObjectFromUid, "getObjectFromUid", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_startSimulation, SIMULATIONPREFIXDOT "start", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_pauseSimulation, SIMULATIONPREFIXDOT "pause", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_stopSimulation, SIMULATIONPREFIXDOT "stop", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_groupShapes, "groupShapes", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_mergeShapes, "mergeShapes", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getContacts, "getContacts", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_stepDynamics, DYNAMICSPREFIXDOT "step", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATED_sceneIsLocked, "sceneIsLocked", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_sceneUid, "sceneUid", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_sceneUidString, "sceneUidString", sim_propertytype_string,SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_scenePath, "scenePath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \

#define SIMULATION_PROPERTIES \
    FUNCX(removeNewObjectsAtEnd, SIMULATIONPREFIXDOT "removeNewObjects", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Remove new objects"}, {"description", "Remove new scene objects at simulation end"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(realtimeSimulation, SIMULATIONPREFIXDOT "realtime", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Real-time simulation"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(pauseSimulationAtTime, SIMULATIONPREFIXDOT "pauseAtTime", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Pause simulation at time"}, {"description", "Pause simulation when simulation time exceeds a threshold"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(pauseSimulationAtError, SIMULATIONPREFIXDOT "pauseAtError", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Pause simulation on script error"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(simulationTime, SIMULATIONPREFIXDOT "time", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Simulation time"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(timeStep, SIMULATIONPREFIXDOT "timeStep", sim_propertytype_float, 0,  PropertyInfo({{"label", "Simulation dt"}, {"description", "Simulation time step"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(timeToPause, SIMULATIONPREFIXDOT "timeToPause", sim_propertytype_float, 0,  PropertyInfo({{"label", "Simulation pause time"}, {"description", "Time at which simulation should pause"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(realtimeModifier, SIMULATIONPREFIXDOT "realtimeModifier", sim_propertytype_float, 0,  PropertyInfo({{"label", "Real-time modifier"}, {"description", "Real-time multiplication factor"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(stepCount, SIMULATIONPREFIXDOT "stepCount", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Simulation steps"}, {"description", "Counter of simulation steps"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(simulationState, SIMULATIONPREFIXDOT "state", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Simulation state"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(stepsPerRendering, SIMULATIONPREFIXDOT "stepsPerRendering", sim_propertytype_int, 0,  PropertyInfo({{"label", "Steps per frame"}, {"description", "Simulation steps per frame"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(speedModifier, SIMULATIONPREFIXDOT "speedModifier", sim_propertytype_int, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Speed modifier"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_simulation, SIMULATIONPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATED_removeNewObjectsAtEnd, "removeNewObjectsAtEnd", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_realtimeSimulation, "realtimeSimulation", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_pauseSimulationAtTime, "pauseSimulationAtTime", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_pauseSimulationAtError, "pauseSimulationAtError", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_simulationTime, "simulationTime", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_timeStep, "timeStep", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_timeToPause, "timeToPause", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_realtimeModifier, "realtimeModifier", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_stepCount, "stepCount", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_simulationState, "simulationState", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_stepsPerRendering, "stepsPerRendering", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_speedModifier, "speedModifier", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \

#define MESHWRAPPER_PROPERTIES \
    FUNCX(mass, "mass", sim_propertytype_float, 0,  PropertyInfo({{"label", "Mass"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(com, "centerOfMass", sim_propertytype_vector3, 0,  PropertyInfo({{"label", "Center of mass"}, {"description", "Center of mass, relative to the shape's reference frame"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(inertiaMatrix, "inertiaMatrix", sim_propertytype_matrix, 0,  PropertyInfo({{"label", "Inertia matrix"}, {"description", "Inertia matrix, relative to the shape's reference frame"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(pmi, "pmi", sim_propertytype_vector3, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Principal moment of inertia"}, {"description", "Principal moment of inertia, relative to pmiQuaternion"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(pmiQuaternion, "pmiQuaternion", sim_propertytype_quaternion, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Quaternion of the principal moment of inertia"}, {"description", "Quaternion of the principal moment of inertia, relative to the shape's reference frame"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATED_pmi, "principalMomentOfInertia", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_inertia, "inertia", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \

#define MESH_PROPERTIES \
    FUNCX(textureResolution, TEXTUREPREFIXDOT "resolution", sim_propertytype_intarray2, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Texture resolution"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(textureCoordinates, TEXTUREPREFIXDOT "coordinates", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Texture coordinates"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(textureApplyMode, TEXTUREPREFIXDOT "applyMode", sim_propertytype_int, 0,  PropertyInfo({{"label", "Texture apply mode"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(textureRepeatU, TEXTUREPREFIXDOT "repeatU", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Texture repeat U"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(textureRepeatV, TEXTUREPREFIXDOT "repeatV", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Texture repeat V"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(textureInterpolate, TEXTUREPREFIXDOT "interpolate", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Interpolate texture"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(texture, TEXTUREPREFIXDOT "data", sim_propertytype_buffer, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Texture"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(textureID, TEXTUREPREFIXDOT "id", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Texture ID"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(vertices, "vertices", sim_propertytype_matrix, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Vertices"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(indices, "indices", sim_propertytype_intarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Indices"}, {"description", "Indices (3 values per triangle)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(normals, "normals", sim_propertytype_matrix, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Normals"}, {"description", "Normals (3*3 values per triangle)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(shadingAngle, "shadingAngle", sim_propertytype_float, 0,  PropertyInfo({{"label", "Shading angle"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(showEdges, "showEdges", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Visible edges"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(culling, "culling", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Backface culling"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(wireframe, "wireframe", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Wireframe"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(shapeUid, "shapeUid", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Shape UID"}, {"description", "Unique identifier of parent shape"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(shape, "shape", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Shape handle"}, {"description", ""}, {"handleType", "shape"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(primitiveType, "primitiveType", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Primitive type"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(convex, "convex", sim_propertytype_bool, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Convex"}, {"description", "Whether mesh is convex or not"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(colorName, "colorName", sim_propertytype_string, 0,  PropertyInfo({{"label", "Color name"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_textureSetData, TEXTUREPREFIXDOT "setData", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_texture, TEXTUREPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATED_textureResolution, "textureResolution", sim_propertytype_intarray2, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_textureCoordinates, "textureCoordinates", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_textureApplyMode, "textureApplyMode", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_textureRepeatU, "textureRepeatU", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_textureRepeatV, "textureRepeatV", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_textureInterpolate, "textureInterpolate", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_texture, "rawTexture", sim_propertytype_buffer, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_textureID, "textureID", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \

#define VIEWABLEBASE_PROPERTIES \
    FUNCX(viewAngle, "viewAngle", sim_propertytype_float, 0,  PropertyInfo({{"label", "View angle"}, {"description", "View angle (in perspective projection mode)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(viewSize, "viewSize", sim_propertytype_float, 0,  PropertyInfo({{"label", "View size"}, {"description", "View size (in orthogonal projection mode)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(clippingPlanes, "clippingPlanes", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", "Clipping planes"}, {"description", "Near and far clipping planes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(perspective, "perspective", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Perspective"}, {"description", "Perspective projection mode, otherwise orthogonal projection mode"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(showFrustum, "showFrustum", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Show view frustum"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(frustumCornerNear, "frustumCornerNear", sim_propertytype_vector3, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Near corner of View frustum"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(frustumCornerFar, "frustumCornerFar", sim_propertytype_vector3, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Far corner of view frustum"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(resolution, "resolution", sim_propertytype_intarray2, 0,  PropertyInfo({{"label", "Resolution"}, {"description", "Resolution (relevant only with vision sensors)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \

#define SCENEOBJECTCONT_PROPERTIES \
    FUNCX(objects, "objects", sim_propertytype_handlearray, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Objects"}, {"description", "Handles of all scene objects"}, {"handleType", "sceneObject"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(orphans, "orphans", sim_propertytype_handlearray, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Orphan objects"}, {"description", "Handles of all orphan scene objects"}, {"handleType", "sceneObject"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(selection, "selection", sim_propertytype_handlearray, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Selected objects"}, {"description", "Handles of selected scene objects"}, {"handleType", "sceneObject"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(objectCreationCounter, "objectCreationCounter", sim_propertytype_int, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Object creation counter"}, {"description", "Counter of created scene objects"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(objectDestructionCounter, "objectDestructionCounter", sim_propertytype_int, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Object destruction counter"}, {"description", "Counter of destroyed scene objects"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(hierarchyChangeCounter, "hierarchyChangeCounter", sim_propertytype_int, sim_propertyinfo_silent | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Hierarchy change counter"}, {"description", "Counter of scene hierarchy changes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATED_objectHandles, "objectHandles", sim_propertytype_intarray, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_orphanHandles, "orphanHandles", sim_propertytype_intarray, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_selectionHandles, "selectionHandles", sim_propertytype_intarray, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \

#define DYNMATERIAL_PROPERTIES \
    FUNCX(engineProperties, DYNAMICSPREFIXDOT "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Engine properties"}, {"description", "Engine properties as JSON text"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(bulletRestitution, DYNAMICSPREFIXDOT BULLETPREFIXDOT "restitution", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_body_restitution, -1, -1, -1, -1) \
    FUNCX(bulletFriction0, DYNAMICSPREFIXDOT BULLETPREFIXDOT "frictionOld", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_body_oldfriction, -1, -1, -1, -1) \
    FUNCX(bulletFriction, DYNAMICSPREFIXDOT BULLETPREFIXDOT "friction", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_body_friction, -1, -1, -1, -1) \
    FUNCX(bulletLinearDamping, DYNAMICSPREFIXDOT BULLETPREFIXDOT "linearDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_body_lineardamping, -1, -1, -1, -1) \
    FUNCX(bulletAngularDamping, DYNAMICSPREFIXDOT BULLETPREFIXDOT "angularDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_body_angulardamping, -1, -1, -1, -1) \
    FUNCX(bulletNonDefaultCollisionMarginFactor, DYNAMICSPREFIXDOT BULLETPREFIXDOT "customCollisionMarginValue", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_body_nondefaultcollisionmargingfactor, -1, -1, -1, -1) \
    FUNCX(bulletNonDefaultCollisionMarginFactorConvex, DYNAMICSPREFIXDOT BULLETPREFIXDOT "customCollisionMarginConvexValue", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_body_nondefaultcollisionmargingfactorconvex, -1, -1, -1, -1) \
    FUNCX(bulletSticky, DYNAMICSPREFIXDOT BULLETPREFIXDOT "stickyContact", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_body_sticky, -1, -1, -1, -1) \
    FUNCX(bulletNonDefaultCollisionMargin, DYNAMICSPREFIXDOT BULLETPREFIXDOT "customCollisionMarginEnabled", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_body_usenondefaultcollisionmargin, -1, -1, -1, -1) \
    FUNCX(bulletNonDefaultCollisionMarginConvex, DYNAMICSPREFIXDOT BULLETPREFIXDOT "customCollisionMarginConvexEnabled", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_body_usenondefaultcollisionmarginconvex, -1, -1, -1, -1) \
    FUNCX(bulletAutoShrinkConvex, DYNAMICSPREFIXDOT BULLETPREFIXDOT "autoShrinkConvexMeshes", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_body_autoshrinkconvex, -1, -1, -1, -1) \
    FUNCX(odeFriction, DYNAMICSPREFIXDOT ODEPREFIXDOT "friction", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_body_friction, -1, -1, -1, -1) \
    FUNCX(odeSoftErp, DYNAMICSPREFIXDOT ODEPREFIXDOT "softErp", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_body_softerp, -1, -1, -1, -1) \
    FUNCX(odeSoftCfm, DYNAMICSPREFIXDOT ODEPREFIXDOT "softCfm", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_body_softcfm, -1, -1, -1, -1) \
    FUNCX(odeLinearDamping, DYNAMICSPREFIXDOT ODEPREFIXDOT "linearDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_body_lineardamping, -1, -1, -1, -1) \
    FUNCX(odeAngularDamping, DYNAMICSPREFIXDOT ODEPREFIXDOT "angularDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_body_angulardamping, -1, -1, -1, -1) \
    FUNCX(odeMaxContacts, DYNAMICSPREFIXDOT ODEPREFIXDOT "maxContacts", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_body_maxcontacts, -1, -1, -1, -1) \
    FUNCX(vortexPrimaryLinearAxisFriction, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearPrimaryAxisFrictionValue", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_primlinearaxisfriction, -1, -1, -1, -1) \
    FUNCX(vortexSecondaryLinearAxisFriction, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearSecondaryAxisFrictionValue", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_seclinearaxisfriction, -1, -1, -1, -1) \
    FUNCX(vortexPrimaryAngularAxisFriction, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularPrimaryAxisFrictionValue", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_primangularaxisfriction, -1, -1, -1, -1) \
    FUNCX(vortexSecondaryAngularAxisFriction, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularSecondaryAxisFrictionValue", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_secangularaxisfriction, -1, -1, -1, -1) \
    FUNCX(vortexNormalAngularAxisFriction, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularNormalAxisFrictionValue", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_normalangularaxisfriction, -1, -1, -1, -1) \
    FUNCX(vortexPrimaryLinearAxisStaticFrictionScale, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearPrimaryAxisStaticFrictionScale", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_primlinearaxisstaticfrictionscale, -1, -1, -1, -1) \
    FUNCX(vortexSecondaryLinearAxisStaticFrictionScale, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearSecondaryAxisStaticFrictionScale", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_seclinearaxisstaticfrictionscale, -1, -1, -1, -1) \
    FUNCX(vortexPrimaryAngularAxisStaticFrictionScale, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularPrimaryAxisStaticFrictionScale", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_primangularaxisstaticfrictionscale, -1, -1, -1, -1) \
    FUNCX(vortexSecondaryAngularAxisStaticFrictionScale, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularSecondaryAxisStaticFrictionScale", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_secangularaxisstaticfrictionscale, -1, -1, -1, -1) \
    FUNCX(vortexNormalAngularAxisStaticFrictionScale, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularNormalAxisStaticFrictionScale", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_normalangularaxisstaticfrictionscale, -1, -1, -1, -1) \
    FUNCX(vortexCompliance, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "compliance", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_compliance, -1, -1, -1, -1) \
    FUNCX(vortexDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "damping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_damping, -1, -1, -1, -1) \
    FUNCX(vortexRestitution, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "restitution", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_restitution, -1, -1, -1, -1) \
    FUNCX(vortexRestitutionThreshold, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "restitutionThreshold", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_restitutionthreshold, -1, -1, -1, -1) \
    FUNCX(vortexAdhesiveForce, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "adhesiveForce", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_adhesiveforce, -1, -1, -1, -1) \
    FUNCX(vortexLinearVelocityDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearVelDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_linearvelocitydamping, -1, -1, -1, -1) \
    FUNCX(vortexAngularVelocityDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularVelDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_angularvelocitydamping, -1, -1, -1, -1) \
    FUNCX(vortexPrimaryLinearAxisSlide, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearPrimaryAxisSlide", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_primlinearaxisslide, -1, -1, -1, -1) \
    FUNCX(vortexSecondaryLinearAxisSlide, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearSecondaryAxisSlide", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_seclinearaxisslide, -1, -1, -1, -1) \
    FUNCX(vortexPrimaryAngularAxisSlide, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularPrimaryAxisSlide", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_primangularaxisslide, -1, -1, -1, -1) \
    FUNCX(vortexSecondaryAngularAxisSlide, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularSecondaryAxisSlide", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_secangularaxisslide, -1, -1, -1, -1) \
    FUNCX(vortexNormalAngularAxisSlide, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularNormalAxisSlide", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_normalangularaxisslide, -1, -1, -1, -1) \
    FUNCX(vortexPrimaryLinearAxisSlip, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearPrimaryAxisSlip", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_primlinearaxisslip, -1, -1, -1, -1) \
    FUNCX(vortexSecondaryLinearAxisSlip, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearSecondaryAxisSlip", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_seclinearaxisslip, -1, -1, -1, -1) \
    FUNCX(vortexPrimaryAngularAxisSlip, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularPrimaryAxisSlip", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_primangularaxisslip, -1, -1, -1, -1) \
    FUNCX(vortexSecondaryAngularAxisSlip, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularSecondaryAxisSlip", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_secangularaxisslip, -1, -1, -1, -1) \
    FUNCX(vortexNormalAngularAxisSlip, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularNormalAxisSlip", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_normalangularaxisslip, -1, -1, -1, -1) \
    FUNCX(vortexAutoSleepLinearSpeedThreshold, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoSleepThresholdLinearSpeed", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_autosleeplinearspeedthreshold, -1, -1, -1, -1) \
    FUNCX(vortexAutoSleepLinearAccelerationThreshold, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoSleepThresholdLinearAccel", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_autosleeplinearaccelthreshold, -1, -1, -1, -1) \
    FUNCX(vortexAutoSleepAngularSpeedThreshold, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoSleepThresholdAngularSpeed", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_autosleepangularspeedthreshold, -1, -1, -1, -1) \
    FUNCX(vortexAutoSleepAngularAccelerationThreshold, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoSleepThresholdAngularAccel", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_autosleepangularaccelthreshold, -1, -1, -1, -1) \
    FUNCX(vortexSkinThickness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "skinThickness", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_skinthickness, -1, -1, -1, -1) \
    FUNCX(vortexAutoAngularDampingTensionRatio, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoAngularDampingTensionRatio", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_autoangulardampingtensionratio, -1, -1, -1, -1) \
    FUNCX(vortexPrimaryAxisVector, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearPrimaryValue", sim_propertytype_vector3, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_primaxisvectorx, sim_vortex_body_primaxisvectory, sim_vortex_body_primaxisvectorz, -1, -1) \
    FUNCX(vortexPrimaryLinearAxisFrictionModel, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearPrimaryAxisFrictionModel", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_primlinearaxisfrictionmodel, -1, -1, -1, -1) \
    FUNCX(vortexSecondaryLinearAxisFrictionModel, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearSecondaryAxisFrictionModel", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_seclinearaxisfrictionmodel, -1, -1, -1, -1) \
    FUNCX(vortexPrimaryAngularAxisFrictionModel, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularPrimaryAxisFrictionModel", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_primangulararaxisfrictionmodel, -1, -1, -1, -1) \
    FUNCX(vortexSecondaryAngularAxisFrictionModel, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularSecondaryAxisFrictionModel", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_secangularaxisfrictionmodel, -1, -1, -1, -1) \
    FUNCX(vortexNormalAngularAxisFrictionModel, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularNormalAxisFrictionModel", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_normalangularaxisfrictionmodel, -1, -1, -1, -1) \
    FUNCX(vortexAutoSleepStepLiveThreshold, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoSleepThresholdSteps", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_autosleepsteplivethreshold, -1, -1, -1, -1) \
    FUNCX(vortexMaterialUniqueId, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "materialUniqueId", sim_propertytype_int, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_materialuniqueid, -1, -1, -1, -1) \
    FUNCX(vortexPrimitiveShapesAsConvex, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "primitiveAsConvex", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_pureshapesasconvex, -1, -1, -1, -1) \
    FUNCX(vortexConvexShapesAsRandom, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "convexAsRandom", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_convexshapesasrandom, -1, -1, -1, -1) \
    FUNCX(vortexRandomShapesAsTerrain, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "randomAsTerrain", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_randomshapesasterrain, -1, -1, -1, -1) \
    FUNCX(vortexFastMoving, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "fastMoving", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_fastmoving, -1, -1, -1, -1) \
    FUNCX(vortexAutoSlip, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoSlip", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_autoslip, -1, -1, -1, -1) \
    FUNCX(vortexSecondaryLinearAxisSameAsPrimaryLinearAxis, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "linearSecondaryAxisFollowPrimaryAxis", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_seclinaxissameasprimlinaxis, -1, -1, -1, -1) \
    FUNCX(vortexSecondaryAngularAxisSameAsPrimaryAngularAxis, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularSecondaryAxisFollowPrimaryAxis", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_secangaxissameasprimangaxis, -1, -1, -1, -1) \
    FUNCX(vortexNormalAngularAxisSameAsPrimaryAngularAxis, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "angularNormalAxisFollowPrimaryAxis", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_normangaxissameasprimangaxis, -1, -1, -1, -1) \
    FUNCX(vortexAutoAngularDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "autoAngularDampingEnabled", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_body_autoangulardamping, -1, -1, -1, -1) \
    FUNCX(newtonStaticFriction, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "staticFriction", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_newton_body_staticfriction, -1, -1, -1, -1) \
    FUNCX(newtonKineticFriction, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "kineticFriction", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_newton_body_kineticfriction, -1, -1, -1, -1) \
    FUNCX(newtonRestitution, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "restitution", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_newton_body_restitution, -1, -1, -1, -1) \
    FUNCX(newtonLinearDrag, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "linearDrag", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_newton_body_lineardrag, -1, -1, -1, -1) \
    FUNCX(newtonAngularDrag, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "angularDrag", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_newton_body_angulardrag, -1, -1, -1, -1) \
    FUNCX(newtonFastMoving, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "fastMoving", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_newton_body_fastmoving, -1, -1, -1, -1) \
    FUNCX(mujocoFriction, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "friction", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_body_friction1, sim_mujoco_body_friction2, sim_mujoco_body_friction3, -1, -1) \
    FUNCX(mujocoSolref, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "solref", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_body_solref1, sim_mujoco_body_solref2, -1, -1, -1) \
    FUNCX(mujocoSolimp, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "solimp", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_body_solimp1, sim_mujoco_body_solimp2, sim_mujoco_body_solimp3, sim_mujoco_body_solimp4, sim_mujoco_body_solimp5) \
    FUNCX(mujocoSolmix, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "solmix", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_body_solmix, -1, -1, -1, -1) \
    FUNCX(mujocoMargin, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "margin", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_body_margin, -1, -1, -1, -1) \
    FUNCX(mujocoGap, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "gap", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoCondim, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "condim", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_body_condim, -1, -1, -1, -1) \
    FUNCX(mujocoPriority, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "priority", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_body_priority, -1, -1, -1, -1) \
    FUNCX(mujocoAdhesion, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "adhesion", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoAdhesionGain, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "adhesiongain", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoAdhesionForcelimited, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "adhesionforcelimited", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoAdhesionCtrlrange, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "adhesionctrlrange", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoAdhesionForcerange, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "adhesionforcerange", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoAdhesionCtrl, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "adhesionctrl", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoGravcomp, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "gravcomp", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamics, DYNAMICSPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamicsBullet, DYNAMICSPREFIXDOT BULLETPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamicsOde, DYNAMICSPREFIXDOT ODEPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamicsNewton, DYNAMICSPREFIXDOT NEWTONPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamicsVortex, DYNAMICSPREFIXDOT VORTEXPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamicsMujoco, DYNAMICSPREFIXDOT MUJOCOPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATEDGROUP_dynamicsBullet, BULLETPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP | SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATEDGROUP_dynamicsOde, ODEPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP | SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATEDGROUP_dynamicsNewton, NEWTONPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP | SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATEDGROUP_dynamicsVortex, VORTEXPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP | SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATEDGROUP_dynamicsMujoco, MUJOCOPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP | SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_engineProperties, "engineProperties", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", "Engine properties"}, {"description", "Engine properties as JSON text"}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletRestitution, BULLETPREFIXDOT "restitution", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_body_restitution, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletFriction0, BULLETPREFIXDOT "frictionOld", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_body_oldfriction, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletFriction, BULLETPREFIXDOT "friction", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_body_friction, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletLinearDamping, BULLETPREFIXDOT "linearDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_body_lineardamping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletAngularDamping, BULLETPREFIXDOT "angularDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_body_angulardamping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletNonDefaultCollisionMarginFactor, BULLETPREFIXDOT "customCollisionMarginValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_body_nondefaultcollisionmargingfactor, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletNonDefaultCollisionMarginFactorConvex, BULLETPREFIXDOT "customCollisionMarginConvexValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_body_nondefaultcollisionmargingfactorconvex, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletSticky, BULLETPREFIXDOT "stickyContact", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_body_sticky, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletNonDefaultCollisionMargin, BULLETPREFIXDOT "customCollisionMarginEnabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_body_usenondefaultcollisionmargin, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletNonDefaultCollisionMarginConvex, BULLETPREFIXDOT "customCollisionMarginConvexEnabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_body_usenondefaultcollisionmarginconvex, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletAutoShrinkConvex, BULLETPREFIXDOT "autoShrinkConvexMeshes", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_body_autoshrinkconvex, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odeFriction, ODEPREFIXDOT "friction", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_body_friction, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odeSoftErp, ODEPREFIXDOT "softErp", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_body_softerp, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odeSoftCfm, ODEPREFIXDOT "softCfm", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_body_softcfm, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odeLinearDamping, ODEPREFIXDOT "linearDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_body_lineardamping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odeAngularDamping, ODEPREFIXDOT "angularDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_body_angulardamping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odeMaxContacts, ODEPREFIXDOT "maxContacts", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_body_maxcontacts, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexPrimaryLinearAxisFriction, VORTEXPREFIXDOT "linearPrimaryAxisFrictionValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_primlinearaxisfriction, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexSecondaryLinearAxisFriction, VORTEXPREFIXDOT "linearSecondaryAxisFrictionValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_seclinearaxisfriction, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexPrimaryAngularAxisFriction, VORTEXPREFIXDOT "angularPrimaryAxisFrictionValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_primangularaxisfriction, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexSecondaryAngularAxisFriction, VORTEXPREFIXDOT "angularSecondaryAxisFrictionValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_secangularaxisfriction, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexNormalAngularAxisFriction, VORTEXPREFIXDOT "angularNormalAxisFrictionValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_normalangularaxisfriction, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexPrimaryLinearAxisStaticFrictionScale, VORTEXPREFIXDOT "linearPrimaryAxisStaticFrictionScale", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_primlinearaxisstaticfrictionscale, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexSecondaryLinearAxisStaticFrictionScale, VORTEXPREFIXDOT "linearSecondaryAxisStaticFrictionScale", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_seclinearaxisstaticfrictionscale, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexPrimaryAngularAxisStaticFrictionScale, VORTEXPREFIXDOT "angularPrimaryAxisStaticFrictionScale", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_primangularaxisstaticfrictionscale, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexSecondaryAngularAxisStaticFrictionScale, VORTEXPREFIXDOT "angularSecondaryAxisStaticFrictionScale", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_secangularaxisstaticfrictionscale, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexNormalAngularAxisStaticFrictionScale, VORTEXPREFIXDOT "angularNormalAxisStaticFrictionScale", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_normalangularaxisstaticfrictionscale, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexCompliance, VORTEXPREFIXDOT "compliance", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_compliance, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexDamping, VORTEXPREFIXDOT "damping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_damping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexRestitution, VORTEXPREFIXDOT "restitution", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_restitution, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexRestitutionThreshold, VORTEXPREFIXDOT "restitutionThreshold", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_restitutionthreshold, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexAdhesiveForce, VORTEXPREFIXDOT "adhesiveForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_adhesiveforce, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexLinearVelocityDamping, VORTEXPREFIXDOT "linearVelDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_linearvelocitydamping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexAngularVelocityDamping, VORTEXPREFIXDOT "angularVelDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_angularvelocitydamping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexPrimaryLinearAxisSlide, VORTEXPREFIXDOT "linearPrimaryAxisSlide", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_primlinearaxisslide, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexSecondaryLinearAxisSlide, VORTEXPREFIXDOT "linearSecondaryAxisSlide", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_seclinearaxisslide, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexPrimaryAngularAxisSlide, VORTEXPREFIXDOT "angularPrimaryAxisSlide", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_primangularaxisslide, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexSecondaryAngularAxisSlide, VORTEXPREFIXDOT "angularSecondaryAxisSlide", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_secangularaxisslide, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexNormalAngularAxisSlide, VORTEXPREFIXDOT "angularNormalAxisSlide", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_normalangularaxisslide, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexPrimaryLinearAxisSlip, VORTEXPREFIXDOT "linearPrimaryAxisSlip", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_primlinearaxisslip, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexSecondaryLinearAxisSlip, VORTEXPREFIXDOT "linearSecondaryAxisSlip", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_seclinearaxisslip, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexPrimaryAngularAxisSlip, VORTEXPREFIXDOT "angularPrimaryAxisSlip", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_primangularaxisslip, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexSecondaryAngularAxisSlip, VORTEXPREFIXDOT "angularSecondaryAxisSlip", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_secangularaxisslip, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexNormalAngularAxisSlip, VORTEXPREFIXDOT "angularNormalAxisSlip", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_normalangularaxisslip, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexAutoSleepLinearSpeedThreshold, VORTEXPREFIXDOT "autoSleepThresholdLinearSpeed", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_autosleeplinearspeedthreshold, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexAutoSleepLinearAccelerationThreshold, VORTEXPREFIXDOT "autoSleepThresholdLinearAccel", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_autosleeplinearaccelthreshold, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexAutoSleepAngularSpeedThreshold, VORTEXPREFIXDOT "autoSleepThresholdAngularSpeed", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_autosleepangularspeedthreshold, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexAutoSleepAngularAccelerationThreshold, VORTEXPREFIXDOT "autoSleepThresholdAngularAccel", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_autosleepangularaccelthreshold, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexSkinThickness, VORTEXPREFIXDOT "skinThickness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_skinthickness, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexAutoAngularDampingTensionRatio, VORTEXPREFIXDOT "autoAngularDampingTensionRatio", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_autoangulardampingtensionratio, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexPrimaryAxisVector, VORTEXPREFIXDOT "linearPrimaryValue", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_primaxisvectorx, sim_vortex_body_primaxisvectory, sim_vortex_body_primaxisvectorz, -1, -1) \
    FUNCX(DEPRECATED_vortexPrimaryLinearAxisFrictionModel, VORTEXPREFIXDOT "linearPrimaryAxisFrictionModel", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_primlinearaxisfrictionmodel, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexSecondaryLinearAxisFrictionModel, VORTEXPREFIXDOT "linearSecondaryAxisFrictionModel", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_seclinearaxisfrictionmodel, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexPrimaryAngularAxisFrictionModel, VORTEXPREFIXDOT "angularPrimaryAxisFrictionModel", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_primangulararaxisfrictionmodel, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexSecondaryAngularAxisFrictionModel, VORTEXPREFIXDOT "angularSecondaryAxisFrictionModel", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_secangularaxisfrictionmodel, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexNormalAngularAxisFrictionModel, VORTEXPREFIXDOT "angularNormalAxisFrictionModel", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_normalangularaxisfrictionmodel, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexAutoSleepStepLiveThreshold, VORTEXPREFIXDOT "autoSleepThresholdSteps", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_autosleepsteplivethreshold, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexMaterialUniqueId, VORTEXPREFIXDOT "materialUniqueId", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_materialuniqueid, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexPrimitiveShapesAsConvex, VORTEXPREFIXDOT "primitiveAsConvex", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_pureshapesasconvex, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexConvexShapesAsRandom, VORTEXPREFIXDOT "convexAsRandom", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_convexshapesasrandom, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexRandomShapesAsTerrain, VORTEXPREFIXDOT "randomAsTerrain", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_randomshapesasterrain, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexFastMoving, VORTEXPREFIXDOT "fastMoving", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_fastmoving, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexAutoSlip, VORTEXPREFIXDOT "autoSlip", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_autoslip, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexSecondaryLinearAxisSameAsPrimaryLinearAxis, VORTEXPREFIXDOT "linearSecondaryAxisFollowPrimaryAxis", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_seclinaxissameasprimlinaxis, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexSecondaryAngularAxisSameAsPrimaryAngularAxis, VORTEXPREFIXDOT "angularSecondaryAxisFollowPrimaryAxis", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_secangaxissameasprimangaxis, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexNormalAngularAxisSameAsPrimaryAngularAxis, VORTEXPREFIXDOT "angularNormalAxisFollowPrimaryAxis", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_normangaxissameasprimangaxis, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexAutoAngularDamping, VORTEXPREFIXDOT "autoAngularDampingEnabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_body_autoangulardamping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_newtonStaticFriction, NEWTONPREFIXDOT "staticFriction", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_newton_body_staticfriction, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_newtonKineticFriction, NEWTONPREFIXDOT "kineticFriction", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_newton_body_kineticfriction, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_newtonRestitution, NEWTONPREFIXDOT "restitution", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_newton_body_restitution, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_newtonLinearDrag, NEWTONPREFIXDOT "linearDrag", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_newton_body_lineardrag, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_newtonAngularDrag, NEWTONPREFIXDOT "angularDrag", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_newton_body_angulardrag, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_newtonFastMoving, NEWTONPREFIXDOT "fastMoving", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_newton_body_fastmoving, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoFriction, MUJOCOPREFIXDOT "friction", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_body_friction1, sim_mujoco_body_friction2, sim_mujoco_body_friction3, -1, -1) \
    FUNCX(DEPRECATED_mujocoSolref, MUJOCOPREFIXDOT "solref", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_body_solref1, sim_mujoco_body_solref2, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoSolimp, MUJOCOPREFIXDOT "solimp", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_body_solimp1, sim_mujoco_body_solimp2, sim_mujoco_body_solimp3, sim_mujoco_body_solimp4, sim_mujoco_body_solimp5) \
    FUNCX(DEPRECATED_mujocoSolmix, MUJOCOPREFIXDOT "solmix", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_body_solmix, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoMargin, MUJOCOPREFIXDOT "margin", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_body_margin, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoGap, MUJOCOPREFIXDOT "gap", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoCondim, MUJOCOPREFIXDOT "condim", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_body_condim, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoPriority, MUJOCOPREFIXDOT "priority", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_body_priority, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoAdhesion, MUJOCOPREFIXDOT "adhesion", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoAdhesionGain, MUJOCOPREFIXDOT "adhesiongain", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoAdhesionForcelimited, MUJOCOPREFIXDOT "adhesionforcelimited", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoAdhesionCtrlrange, MUJOCOPREFIXDOT "adhesionctrlrange", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoAdhesionForcerange, MUJOCOPREFIXDOT "adhesionforcerange", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoAdhesionCtrl, MUJOCOPREFIXDOT "adhesionctrl", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoGravcomp, MUJOCOPREFIXDOT "gravcomp", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \

#define CUSTOMSCENEOBJECTCLASS_PROPERTIES \
    FUNCX(METHOD_makeOject, "makeObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_remove, "remove", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \

#define SCENEOBJECT_PROPERTIES \
    FUNCX(modelInvisible, "modelInvisible", sim_propertytype_bool, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Inherited model invisibility"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(modelBase, "modelBase", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Model base"}, {"description", "Model base flag, indicates the scene object is the base of a model"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(layer, "layer", sim_propertytype_int, 0,  PropertyInfo({{"label", "Visibility layer"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(tree, "tree", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Object tree"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(childOrder, "childOrder", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Child order"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(parentUid, "parentUid", sim_propertytype_long, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Parent UID"}, {"description", "Parent scene object unique identifier"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(uid, "uid", sim_propertytype_long, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Object UID"}, {"description", "Scene object unique identifier"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(parent, "parent", sim_propertytype_handle, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Parent"}, {"description", "Parent scene object handle"}, {"handleType", "sceneObject"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(selected, "selected", sim_propertytype_bool, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Selected"}, {"description", "Selection state"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(hierarchyColor, "hierarchyColor", sim_propertytype_int, 0,  PropertyInfo({{"label", "Hierarchy color"}, {"description", "Hierarchy color index"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(collectionSelfCollInd, "collectionSelfCollisionIndicator", sim_propertytype_int, 0,  PropertyInfo({{"label", "Collection self collision indicator"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(collidable, "collidable", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Collidable"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(measurable, "measurable", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Measurable"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(detectable, "detectable", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Detectable"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(modelAcknowledgment, "modelAcknowledgment", sim_propertytype_string, 0,  PropertyInfo({{"label", "Acknowledgment"}, {"description", "Model acknowledgment"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(dna, "dna", sim_propertytype_buffer, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "DNA"}, {"description", "Scene object DNA"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(persistentUid, "persistentUid", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Persistent UID"}, {"description", "Scene object persistent unique identifier"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(calcLinearVelocity, "calcLinearVelocity", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Linear velocity"}, {"description", "Calculated scene object linear velocity vector"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(calcRotationAxis, "calcRotationAxis", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Rotation axis"}, {"description", "Calculated scene object rotation axis"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(calcRotationVelocity, "calcRotationVelocity", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Rotation velocity"}, {"description", "Calculated scene object rotation velocity"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(dynamicIcon, "dynamicIcon", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Dynamic icon"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(dynamicFlag, "dynamicFlag", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Dynamic flag"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(objectProperty, "objectPropertyFlags", /*redund.*/ sim_propertytype_int, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Object flags"}, {"description", "Scene object flags, redundant"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(ignoreViewFitting, "ignoreViewFitting", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Ignore view fitting"}, {"description", "View fitting is ignored for this scene object"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(collapsed, "collapsed", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Collapsed hierarchy"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(selectable, "selectable", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Selectable"}, {"description", "Scene object is selectable"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(selectModel, "selectModel", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Select model instead"}, {"description", "Selecting the scene object instead selects the parent model"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(hideFromModelBB, "hideFromModelBB", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Hide from model bounding box"}, {"description", "Scene object is not part of a parent model's bounding box"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(selectInvisible, "selectInvisible", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Select invisible"}, {"description", "Scene object is invisible during a selection operation"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(depthInvisible, "depthInvisible", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Depth pass invisible"}, {"description", "Scene object is invisible for the depth buffer during a click operation"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(cannotDelete, "cannotDelete", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Cannot delete"}, {"description", "Scene object cannot be deleted while simulation is not running"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(cannotDeleteSim, "cannotDeleteInSim", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Cannot delete during simulation"}, {"description", "Scene object cannot be deleted while simulation is running"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(modelProperty, MODELPREFIXDOT "propertyFlags", /*redund.*/ sim_propertytype_int, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Model flags"}, {"description", "Model flags, redundant"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(modelNotCollidable, MODELPREFIXDOT "notCollidable", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Model not collidable"}, {"description", "Model is not collidable"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(modelNotMeasurable, MODELPREFIXDOT "notMeasurable", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Model not measurable"}, {"description", "Model is not measurable"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(modelNotDetectable, MODELPREFIXDOT "notDetectable", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Model not detectable"}, {"description", "Model is not detectable"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(modelNotDynamic, MODELPREFIXDOT "notDynamic", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Model not dynamic"}, {"description", "Model is not dynamic, i.e. model is static"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(modelNotRespondable, MODELPREFIXDOT "notRespondable", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Model not respondable"}, {"description", "Model is not respondable"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(modelNotVisible, MODELPREFIXDOT "notVisible", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Model not visible"}, {"description", "Model is not visible"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(modelScriptsNotActive, MODELPREFIXDOT "scriptsNotActive", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Model scripts inactive"}, {"description", "Model scripts are not active"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(modelNotInParentBB, MODELPREFIXDOT "notInParentBB", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Model invisible to other model's bounding boxes"}, {"description", "Model is invisible to other model's bounding boxes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(modelBBSize, MODELPREFIXDOT "bbSize", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Model bounding box size"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(modelBBPos, MODELPREFIXDOT "bbPos", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Model bounding box position"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(pose, "relative.pose", sim_propertytype_pose, 0,  PropertyInfo({{"label", "Pose"}, {"description", "Scene object local pose"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(position, "relative.position", sim_propertytype_vector3, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Position"}, {"description", "Scene object local position"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(quaternion, "relative.quaternion", sim_propertytype_quaternion, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Quaternion"}, {"description", "Scene object local quaternion"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(eulerAngles, "relative.eulerAngles", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Euler angles"}, {"description", "Scene object local Euler angles"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(absPose, "absolute.pose", sim_propertytype_pose, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Pose"}, {"description", "Scene object absolute pose"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(absPosition, "absolute.position", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Position"}, {"description", "Scene object absolute position"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(absQuaternion, "absolute.quaternion", sim_propertytype_quaternion, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Quaternion"}, {"description", "Scene object absolute quaternion"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(absEulerAngles, "absolute.eulerAngles", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Euler angles"}, {"description", "Scene object absolute Euler angles"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(name, "name", sim_propertytype_string, 0,  PropertyInfo({{"label", "Name"}, {"description", "Scene object name"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(bbPose, "bbPose", sim_propertytype_pose, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Bounding box pose"}, {"description", "Bounding box local pose"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(size, "dimension", sim_propertytype_vector3, 0,  PropertyInfo({{"label", "Bounding box size"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(movementOptions, MOVPREFIXDOT "optionsFlags", sim_propertytype_int, 0,  PropertyInfo({{"label", "Movement flags"}, {"description", "Scene object movement flags"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(movementStepSize, MOVPREFIXDOT "stepSize", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", "Movement step sizes"}, {"description", "Linear and angular step sizes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(movementRelativity, MOVPREFIXDOT "relativity", sim_propertytype_intarray, 0,  PropertyInfo({{"label", "Movement relativity"}, {"description", "Movement relativity, linear and angular"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(movementPreferredAxes, MOVPREFIXDOT "preferredAxesFlags", sim_propertytype_int, 0,  PropertyInfo({{"label", "Preferred axes"}, {"description", "Preferred axes flags (redundant)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(movTranslNoSim, MOVPREFIXDOT "translNoSim", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Translation enabled"}, {"description", "Translation enabled when simulation is not running"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(movTranslInSim, MOVPREFIXDOT "translInSim", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Translation enabled during simulation"}, {"description", "Translation enabled when simulation is running"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(movRotNoSim, MOVPREFIXDOT "rotNoSim", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Rotation enabled"}, {"description", "Rotation enabled when simulation is not running"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(movRotInSim, MOVPREFIXDOT "rotInSim", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Rotation enabled during simulation"}, {"description", "Rotation enabled when simulation is running"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(movAltTransl, MOVPREFIXDOT "altTransl", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Alternate translation axes enabled"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(movAltRot, MOVPREFIXDOT "altRot", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Alternate rotation axes enabled"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(movPrefTranslX, MOVPREFIXDOT "prefTranslX", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Preferred X-translation"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(movPrefTranslY, MOVPREFIXDOT "prefTranslY", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Preferred Y-translation"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(movPrefTranslZ, MOVPREFIXDOT "prefTranslZ", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Preferred Z-translation"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(movPrefRotX, MOVPREFIXDOT "prefRotX", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Preferred X-axis rotation"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(movPrefRotY, MOVPREFIXDOT "prefRotY", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Preferred Y-axis rotation"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(movPrefRotZ, MOVPREFIXDOT "prefRotZ", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Preferred Z-axis rotation"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(visible, "visible", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Visible"}, {"description", "Whether the scene object is currently visible"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(children, "children", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Children handles"}, {"description", ""}, {"handleType", "sceneObject"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(modelHash, "modelHash", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Model hash"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(dynamicallyEnabled, "dynamicallyEnabled", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Dynamically enabled"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_customData, CUSTOMDATAPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_signal, SIGNALPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_refs, REFSPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_origRefs, ORIGREFSPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_model, MODELPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_mov, MOVPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_absolute, "absolute", sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_relative, "relative", sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getAncestors, "getAncestors", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getDescendants, "getDescendants", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_removeModel, "removeModel", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_remove, "remove", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_saveModel, "saveModel", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_saveModelToBuffer, "saveModelToBuffer", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_checkCollision, "checkCollision", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_checkDistance, "checkDistance", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getName, "getName", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getPose, "getPose", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getPosition, "getPosition", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getQuaternion, "getQuaternion", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setPose, "setPose", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setPosition, "setPosition", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setQuaternion, "setQuaternion", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setParent, "setParent", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_scale, "scale", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_scaleTree, "scaleTree", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_dynamicReset, "dynamicReset", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_visitTree, "visitTree", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_changeColor, "changeColor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_restoreColor, "restoreColor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_makeClass, "makeClass", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_makeObject, "makeObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getObject, "getObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATED_pose, "pose", sim_propertytype_pose, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
/*    FUNCX(DEPRECATED_position, "position", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 0}, {"endSupport", 1}})) \
    FUNCX(DEPRECATED_quaternion, "quaternion", sim_propertytype_quaternion, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 0}, {"endSupport", 1}})) */ \
    FUNCX(DEPRECATED_eulerAngles, "eulerAngles", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_parentHandle, "parentHandle", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_modelProperty, "modelPropertyFlags", /*redund.*/ sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_modelNotCollidable, "modelNotCollidable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_modelNotMeasurable, "modelNotMeasurable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_modelNotDetectable, "modelNotDetectable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_modelNotDynamic, "modelNotDynamic", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_modelNotRespondable, "modelNotRespondable", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_modelNotVisible, "modelNotVisible", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_modelScriptsNotActive, "modelScriptsNotActive", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_modelNotInParentBB, "modelNotInParentBB", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_alias, "alias", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_deprecatedName, "deprecatedName", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_bbHsize, "bbHSize", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_objectUid, "objectUid", sim_propertytype_long, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_movementOptions, "movementOptionsFlags", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_movementStepSize, "movementStepSize", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_movementRelativity, "movementRelativity", sim_propertytype_intarray, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_movementPreferredAxes, "movementPreferredAxesFlags", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_movTranslNoSim, "movTranslNoSim", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_movTranslInSim, "movTranslInSim", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_movRotNoSim, "movRotNoSim", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_movRotInSim, "movRotInSim", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_movAltTransl, "movAltTransl", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_movAltRot, "movAltRot", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_movPrefTranslX, "movPrefTranslX", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_movPrefTranslY, "movPrefTranslY", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_movPrefTranslZ, "movPrefTranslZ", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_movPrefRotX, "movPrefRotX", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_movPrefRotY, "movPrefRotY", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_movPrefRotZ, "movPrefRotZ", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \

#define SHAPE_PROPERTIES \
    FUNCX(meshes, "meshes", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Meshes"}, {"description", "Mesh handles"}, {"handleType", "mesh"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(applyCulling, "applyCulling", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Apply culling"}, {"description", "Enables/disables culling for all contained meshes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(applyShadingAngle, "applyShadingAngle", sim_propertytype_float, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Apply shading"}, {"description", "Applies a shading angle to all contained meshes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(applyShowEdges, "applyShowEdges", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Apply edges"}, {"description", "Enables/disables edges for all contained meshes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(flipFaces, "flipFaces", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Flip faces"}, {"description", "Flips faces of all contained meshes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(applyColorDiffuse, APPLYCOLORPREFIXDOT "diffuse", sim_propertytype_color, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Apply diffuse color"}, {"description", "Applies the diffuse color component to all contained meshes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(applyColorSpecular, APPLYCOLORPREFIXDOT "specular", sim_propertytype_color, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Apply specular color"}, {"description", "Applies the specular color component to all contained meshes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(applyColorEmission, APPLYCOLORPREFIXDOT "emission", sim_propertytype_color, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Apply emission color"}, {"description", "Applies the emission color component to all contained meshes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(applyColorTransparency, APPLYCOLORPREFIXDOT "transparency", sim_propertytype_float, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Apply transparency"}, {"description", "Applies transparency to all contained meshes"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(compoundColorDiffuse, COMPOUNDCOLORSPREFIXDOT "diffuse", sim_propertytype_floatarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Diffuse color data"}, {"description", "Diffuse color from all compound elements"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(compoundColorSpecular, COMPOUNDCOLORSPREFIXDOT "specular", sim_propertytype_floatarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Specular color data"}, {"description", "Specular color from all compound elements"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(compoundColorEmission, COMPOUNDCOLORSPREFIXDOT "emission", sim_propertytype_floatarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Emission color data"}, {"description", "Emission color from all compound elements"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(compoundColorTransparency, COMPOUNDCOLORSPREFIXDOT "transparency", sim_propertytype_floatarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Transparency data"}, {"description", "Transparency value from all compound elements"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(compoundEdges, "compoundEdges", sim_propertytype_intarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Edge visibility data"}, {"description", "Edge visibility from all compound elements"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(compoundWireframe, "compoundWireframe", sim_propertytype_intarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Wireframe data"}, {"description", "Wireframe state from all compound elements"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(compoundCullings, "compoundCullings", sim_propertytype_intarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Culling data"}, {"description", "Backface culling state from all compound elements"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(compoundShadingAngles, "compoundShadingAngles", sim_propertytype_floatarray, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Shading angle data"}, {"description", "Shading angle state from all compound elements"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(respondableMask, "respondableMask", sim_propertytype_int, 0,  PropertyInfo({{"label", "Respondable mask"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(startInDynSleepMode, "startInDynSleepMode", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Start in sleep mode"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(dynamic, "dynamic", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Dynamic"}, {"description", "Shape is dynamic, i.e. not static"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(kinematic, "kinematic", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Kinematic"}, {"description", "Special flag mainly used for MuJoCo static shapes that move and need to transmit a friction"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(respondable, "respondable", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Respondable"}, {"description", "Shape will transmit a collision force"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(setToDynamicWithParent, "setToDynamicWithParent", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Set to dynamic if gets parent"}, {"description", "Shape will be made dynamic if it receives a parent"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(initLinearVelocity, "initLinearVelocity", sim_propertytype_vector3, 0,  PropertyInfo({{"label", "Initial linear velocity"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(initAngularVelocity, "initAngularVelocity", sim_propertytype_vector3, 0,  PropertyInfo({{"label", "Initial rotational velocity"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(dynLinearVelocity, "dynLinearVelocity", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Linear velocity"}, {"description", "Linear velocity, as transmitted by the physics engine"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(dynAngularVelocity, "dynAngularVelocity", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Rotational velocity"}, {"description", "Rotational velocity, as transmitted by the physics engine"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(convex, "convex", sim_propertytype_bool, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Convex"}, {"description", "Whether the shape's components are all convex or not"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(primitive, "primitive", sim_propertytype_bool, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Primitive"}, {"description", "Whether the shape's components are all primitives"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(compound, "compound", sim_propertytype_bool, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Compound"}, {"description", "Whether the shape is a compound"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_applyColor, APPLYCOLORPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_compoundColors, COMPOUNDCOLORSPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_addForce, "addForce", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_addTorque, "addTorque", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_relocateFrame, "relocateFrame", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_alignBoundingBox, "alignBoundingBox", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getInertia, "getInertia", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setInertia, "setInertia", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_computeInertia, "computeInertia", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setAppearance, "setAppearance", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getAppearance, "getAppearance", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getContacts, "getContacts", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_ungroup, "ungroup", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_divide, "divide", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \

#define CAMERA_PROPERTIES \
    FUNCX(size, "size", sim_propertytype_float, 0,  PropertyInfo({{"label", "Size"}, {"description", "Camera size"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(parentAsManipProxy, "parentAsManipulationProxy", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Parent as proxy"}, {"description", "Use parent as manipulation proxy"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(translationEnabled, "translationEnabled", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Translation enabled"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(rotationEnabled, "rotationEnabled", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Rotation enabled"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(trackedObject, "trackedObject", sim_propertytype_handle, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Tracked object"}, {"description", "Tracked scene object"}, {"handleType", "sceneObject"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATED_size, "cameraSize", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_trackedObject, "trackedObjectHandle", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \

#define DUMMY_PROPERTIES \
    FUNCX(linkedDummy, "linkedDummy", sim_propertytype_handle, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Linked dummy"}, {"description", "Handle of the linked dummy"}, {"handleType", "dummy"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(assemblyTag, "assemblyTag", sim_propertytype_string, 0,  PropertyInfo({{"label", "Assembly tag"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(size, "size", sim_propertytype_float, 0,  PropertyInfo({{"label", "Size"}, {"description", "Dummy size"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(dummyType, "type", sim_propertytype_int, 0,  PropertyInfo({{"label", "Type"}, {"description", "Dummy type"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(engineProperties, DYNAMICSPREFIXDOT "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Engine properties"}, {"description", "Engine properties as JSON text"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoLimitsEnabled, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "limitsEnabled", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_dummy_limited, -1, -1, -1, -1) \
    FUNCX(mujocoLimitsRange, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "limitsRange", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_dummy_range1, sim_mujoco_dummy_range2, -1, -1, -1) \
    FUNCX(mujocoLimitsSolref, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "limitsSolref", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_dummy_solreflimit1, sim_mujoco_dummy_solreflimit2, -1, -1, -1) \
    FUNCX(mujocoLimitsSolimp, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "limitsSolimp", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_dummy_solimplimit1, sim_mujoco_dummy_solimplimit2, sim_mujoco_dummy_solimplimit3, sim_mujoco_dummy_solimplimit4, sim_mujoco_dummy_solimplimit5) \
    FUNCX(mujocoMargin, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "margin", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_dummy_margin, -1, -1, -1, -1) \
    FUNCX(mujocoSpringStiffness, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "springStiffness", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_dummy_stiffness, -1, -1, -1, -1) \
    FUNCX(mujocoSpringDamping, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "springDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_dummy_damping, -1, -1, -1, -1) \
    FUNCX(mujocoSpringLength, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "springLength", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_dummy_springlength, -1, -1, -1, -1) \
    FUNCX(mujocoJointProxyHandle, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "jointProxyHandle", sim_propertytype_int, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Joint proxy"}, {"description", "Handle of the joint proxy, MuJoCo only"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_dummy_proxyjointid, -1, -1, -1, -1) \
    FUNCX(mujocoOverlapConstrSolref, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "overlapConstrSolref", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoOverlapConstrSolimp, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "overlapConstrSolimp", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(mujocoOverlapConstrTorqueScale, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "overlapConstrTorquescale", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamics, DYNAMICSPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamicsMujoco, DYNAMICSPREFIXDOT MUJOCOPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATEDGROUP_dynamicsMujoco, MUJOCOPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP | SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_size, "dummySize", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_dummyType, "dummyType", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_engineProperties, "engineProperties", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoLimitsEnabled, MUJOCOPREFIXDOT "limitsEnabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_dummy_limited, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoLimitsRange, MUJOCOPREFIXDOT "limitsRange", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_dummy_range1, sim_mujoco_dummy_range2, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoLimitsSolref, MUJOCOPREFIXDOT "limitsSolref", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_dummy_solreflimit1, sim_mujoco_dummy_solreflimit2, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoLimitsSolimp, MUJOCOPREFIXDOT "limitsSolimp", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_dummy_solimplimit1, sim_mujoco_dummy_solimplimit2, sim_mujoco_dummy_solimplimit3, sim_mujoco_dummy_solimplimit4, sim_mujoco_dummy_solimplimit5) \
    FUNCX(DEPRECATED_mujocoMargin, MUJOCOPREFIXDOT "margin", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_dummy_margin, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoSpringStiffness, MUJOCOPREFIXDOT "springStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_dummy_stiffness, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoSpringDamping, MUJOCOPREFIXDOT "springDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_dummy_damping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoSpringLength, MUJOCOPREFIXDOT "springLength", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_dummy_springlength, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoJointProxyHandle, MUJOCOPREFIXDOT "jointProxyHandle", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_dummy_proxyjointid, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoOverlapConstrSolref, MUJOCOPREFIXDOT "overlapConstrSolref", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoOverlapConstrSolimp, MUJOCOPREFIXDOT "overlapConstrSolimp", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoOverlapConstrTorqueScale, MUJOCOPREFIXDOT "overlapConstrTorquescale", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_linkedDummyHandle, "linkedDummyHandle", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \

#define FORCESENSOR_PROPERTIES \
    FUNCX(size, "size", sim_propertytype_float, 0,  PropertyInfo({{"label", "Size"}, {"description", "Sensor size"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(forceThresholdEnabled, "forceThresholdEnabled", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Force threshold enabled"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(torqueThresholdEnabled, "torqueThresholdEnabled", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Torque threshold enabled"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(filterType, "filterType", sim_propertytype_int, 0,  PropertyInfo({{"label", "Filter"}, {"description", "Filter type"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(filterSampleSize, "filterSampleSize", sim_propertytype_int, 0,  PropertyInfo({{"label", "Filter sample"}, {"description", "Filter sample size"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(consecutiveViolationsToTrigger, "consecutiveViolationsToTrigger", sim_propertytype_int, 0,  PropertyInfo({{"label", "Consecutive violations to trigger"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(forceThreshold, "forceThreshold", sim_propertytype_float, 0,  PropertyInfo({{"label", "Force threshold"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(torqueThreshold, "torqueThreshold", sim_propertytype_float, 0,  PropertyInfo({{"label", "Torque threshold"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(sensorForce, "force", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Force"}, {"description", "Measured force vector"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(sensorTorque, "torque", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Torque"}, {"description", "Measured torque vector"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(filteredSensorForce, "filteredForce", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Filtered force"}, {"description", "Filtered force vector"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(filteredSensorTorque, "filteredTorque", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Filtered torque"}, {"description", "Filtered torque vector"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(intrinsicError, "intrinsicError", sim_propertytype_pose, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Intrinsic error"}, {"description", "Intrinsic error, generated by some physics engines"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATED_size, "sensorSize", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_sensorForce, "sensorForce", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_sensorTorque, "sensorTorque", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_sensorAverageForce, "filterSensorForce", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_sensorAverageTorque, "filterSensorTorque", sim_propertytype_vector3, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \

#define GRAPH_PROPERTIES \
    FUNCX(size, "size", sim_propertytype_float, 0,  PropertyInfo({{"label", "Size"}, {"description", "Size of the 3D graph representation"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(bufferSize, "bufferSize", sim_propertytype_int, 0,  PropertyInfo({{"label", "Buffer size"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(cyclic, "cyclic", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Cyclic"}, {"description", "Buffer is cyclic"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(backgroundColor, "backgroundColor", sim_propertytype_color, 0,  PropertyInfo({{"label", "Background color"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(foregroundColor, "foregroundColor", sim_propertytype_color, 0,  PropertyInfo({{"label", "Foreground color"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_addCurve, "addCurve", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_addSignal, "addSignal", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setSignalPoint, "setSignalPoint", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_removeTrace, "removeTrace", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_snapshotTrace, "snapshotTrace", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_step, "step", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_resetGraph, "resetGraph", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATED_size, "graphSize", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \

#define JOINT_PROPERTIES \
    FUNCX(position, "position", sim_propertytype_float, 0,  PropertyInfo({{"label", "Position"}, {"description", "Joint linear/angular displacement"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(quaternion, "quaternion", sim_propertytype_quaternion, 0,  PropertyInfo({{"label", "Spherical joint quaternion"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(screwLead, "screwLead", sim_propertytype_float, 0,  PropertyInfo({{"label", "Screw lead"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(intrinsicError, "intrinsicError", sim_propertytype_pose, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Intrinsic error"}, {"description", "Intrinsic error, generated by some physics engines"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(intrinsicPose, "intrinsicPose", sim_propertytype_pose, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Intrinsic pose"}, {"description", "Intrinsic pose (includes a possible intrinsic error)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(calcVelocity, "calcVelocity", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Velocity"}, {"description", "Calculated joint linear or angular velocity"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(cyclic, "cyclic", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Cyclic"}, {"description", "Cyclic revolute joint, has no limits"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(enforceLimits, "enforceLimits", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Enforce limits"}, {"description", "Enforce limits strictly"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(interval, "interval", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", "Interval"}, {"description", "Joint limits (lower and upper bounds)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(targetPos, "targetPosition", sim_propertytype_float, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Target position"}, {"description", "Position to reach by controller"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(targetVel, "targetVelocity", sim_propertytype_float, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Target velocity"}, {"description", "Velocity to reach by controller"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(targetForce, "targetForce", sim_propertytype_float, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Target force"}, {"description", "Maximum force to exert"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(length, "length", sim_propertytype_float, 0,  PropertyInfo({{"label", "Length"}, {"description", "Joint length"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(diameter, "diameter", sim_propertytype_float, 0,  PropertyInfo({{"label", "Diamter"}, {"description", "Joint diameter"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(jointType, "type", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Type"}, {"description", "Joint type"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(jointForce, "force", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Force"}, {"description", "Force applied"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(averageJointForce, "averageForce", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Average force"}, {"description", "Force applied on average (in a simulation step)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(jointMode, "mode", sim_propertytype_int, 0,  PropertyInfo({{"label", "Mode"}, {"description", "Joint mode"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(dependencyMaster, "dependencyMaster", sim_propertytype_handle, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Dependency master"}, {"description", "Handle of master joint (in a dependency relationship)"}, {"handleType", "joint"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(dependencyParams, "dependencyParams", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", "Dependency parameters"}, {"description", "Dependency parameters (offset and coefficient)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(maxVelAccelJerk, "maxVelAccelJerk", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", "Maximum velocity, acceleration and jerk"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(dynCtrlMode, DYNAMICSPREFIXDOT "ctrlMode", sim_propertytype_enum, 0, PropertyInfo({{"label", "Control mode" }, {"description", "Joint control mode, when in dynamic mode"}, {"enum", "jointDynCtrlMode"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(springDamperParams, DYNAMICSPREFIXDOT "springDamperParams", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", "Spring damper parameters"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(dynVelMode, DYNAMICSPREFIXDOT "velMode", sim_propertytype_int, 0,  PropertyInfo({{"label", "Dynamic velocity mode"}, {"description", "Dynamic velocity mode (0: default, 1: Ruckig)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(dynPosMode, DYNAMICSPREFIXDOT "posMode", sim_propertytype_int, 0,  PropertyInfo({{"label", "Dynamic position mode"}, {"description", "Dynamic position mode (0: default, 1: Ruckig)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(engineProperties, DYNAMICSPREFIXDOT "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Engine properties"}, {"description", "Engine properties as JSON text"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(bulletStopErp, DYNAMICSPREFIXDOT BULLETPREFIXDOT "stopErp", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_joint_stoperp, -1, -1, -1, -1) \
    FUNCX(bulletStopCfm, DYNAMICSPREFIXDOT BULLETPREFIXDOT "stopCfm", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_joint_stopcfm, -1, -1, -1, -1) \
    FUNCX(bulletNormalCfm, DYNAMICSPREFIXDOT BULLETPREFIXDOT "normalCfm", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_joint_normalcfm, -1, -1, -1, -1) \
    FUNCX(bulletPosPid, DYNAMICSPREFIXDOT BULLETPREFIXDOT "posPid", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_bullet_joint_pospid1, sim_bullet_joint_pospid2, sim_bullet_joint_pospid3, -1, -1) \
    FUNCX(odeStopErp, DYNAMICSPREFIXDOT ODEPREFIXDOT "stopErp", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_joint_stoperp, -1, -1, -1, -1) \
    FUNCX(odeStopCfm, DYNAMICSPREFIXDOT ODEPREFIXDOT "stopCfm", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_joint_stopcfm, -1, -1, -1, -1) \
    FUNCX(odeNormalCfm, DYNAMICSPREFIXDOT ODEPREFIXDOT "normalCfm", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_joint_normalcfm, -1, -1, -1, -1) \
    FUNCX(odeBounce, DYNAMICSPREFIXDOT ODEPREFIXDOT "bounce", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_joint_bounce, -1, -1, -1, -1) \
    FUNCX(odeFudgeFactor, DYNAMICSPREFIXDOT ODEPREFIXDOT "fudge", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_joint_fudgefactor, -1, -1, -1, -1) \
    FUNCX(odePosPid, DYNAMICSPREFIXDOT ODEPREFIXDOT "posPid", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_ode_joint_pospid1, sim_ode_joint_pospid2, sim_ode_joint_pospid3, -1, -1) \
    FUNCX(vortexLowerLimitDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisLimitsLowerDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_lowerlimitdamping, -1, -1, -1, -1) \
    FUNCX(vortexUpperLimitDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisLimitsUpperDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_upperlimitdamping, -1, -1, -1, -1) \
    FUNCX(vortexLowerLimitStiffness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisLimitsLowerStiffness", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_lowerlimitstiffness, -1, -1, -1, -1) \
    FUNCX(vortexUpperLimitStiffness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisLimitsUpperStiffness", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_upperlimitstiffness, -1, -1, -1, -1) \
    FUNCX(vortexLowerLimitRestitution, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisLimitsLowerRestitution", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_lowerlimitrestitution, -1, -1, -1, -1) \
    FUNCX(vortexUpperLimitRestitution, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisLimitsUpperRestitution", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_upperlimitrestitution, -1, -1, -1, -1) \
    FUNCX(vortexLowerLimitMaxForce, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisLimitsLowerMaxForce", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_lowerlimitmaxforce, -1, -1, -1, -1) \
    FUNCX(vortexUpperLimitMaxForce, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisLimitsUpperMaxForce", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_upperlimitmaxforce, -1, -1, -1, -1) \
    FUNCX(vortexAxisFrictionEnabled, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisFrictionEnabled", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_motorfrictionenabled, -1, -1, -1, -1) \
    FUNCX(vortexAxisFrictionProportional, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisFrictionProportional", sim_propertytype_bool, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_proportionalmotorfriction, -1, -1, -1, -1) \
    FUNCX(vortexAxisFrictionCoeff, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisFrictionValue", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_motorconstraintfrictioncoeff, -1, -1, -1, -1) \
    FUNCX(vortexAxisFrictionMaxForce, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisFrictionMaxForce", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_motorconstraintfrictionmaxforce, -1, -1, -1, -1) \
    FUNCX(vortexAxisFrictionLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "axisFrictionLoss", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_motorconstraintfrictionloss, -1, -1, -1, -1) \
    FUNCX(vortexRelaxationEnabledBits, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "relaxationEnabledBits", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_relaxationenabledbc, -1, -1, -1, -1) \
    FUNCX(vortexFrictionEnabledBits, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "frictionEnabledBits", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_frictionenabledbc, -1, -1, -1, -1) \
    FUNCX(vortexFrictionProportionalBits, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "frictionProportionalBits", sim_propertytype_int, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_frictionproportionalbc, -1, -1, -1, -1) \
    FUNCX(vortexXAxisPosRelaxationStiffness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisPosRelaxationStiffness", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p0stiffness, -1, -1, -1, -1) \
    FUNCX(vortexXAxisPosRelaxationDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisPosRelaxationDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p0damping, -1, -1, -1, -1) \
    FUNCX(vortexXAxisPosRelaxationLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisPosRelaxationLoss", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p0loss, -1, -1, -1, -1) \
    FUNCX(vortexXAxisPosFrictionCoeff, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisPosFrictionCoeff", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p0frictioncoeff, -1, -1, -1, -1) \
    FUNCX(vortexXAxisPosFrictionMaxForce, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisPosFrictionMaxForce", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p0frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(vortexXAxisPosFrictionLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisPosFrictionLoss", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p0frictionloss, -1, -1, -1, -1) \
    FUNCX(vortexYAxisPosRelaxationStiffness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisPosRelaxationStiffness", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p1stiffness, -1, -1, -1, -1) \
    FUNCX(vortexYAxisPosRelaxationDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisPosRelaxationDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p1damping, -1, -1, -1, -1) \
    FUNCX(vortexYAxisPosRelaxationLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisPosRelaxationLoss", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p1loss, -1, -1, -1, -1) \
    FUNCX(vortexYAxisPosFrictionCoeff, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisPosFrictionCoeff", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p1frictioncoeff, -1, -1, -1, -1) \
    FUNCX(vortexYAxisPosFrictionMaxForce, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisPosFrictionMaxForce", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p1frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(vortexYAxisPosFrictionLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisPosFrictionLoss", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p1frictionloss, -1, -1, -1, -1) \
    FUNCX(vortexZAxisPosRelaxationStiffness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisPosRelaxationStiffness", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p2stiffness, -1, -1, -1, -1) \
    FUNCX(vortexZAxisPosRelaxationDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisPosRelaxationDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p2damping, -1, -1, -1, -1) \
    FUNCX(vortexZAxisPosRelaxationLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisPosRelaxationLoss", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p2loss, -1, -1, -1, -1) \
    FUNCX(vortexZAxisPosFrictionCoeff, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisPosFrictionCoeff", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p2frictioncoeff, -1, -1, -1, -1) \
    FUNCX(vortexZAxisPosFrictionMaxForce, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisPosFrictionMaxForce", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p2frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(vortexZAxisPosFrictionLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisPosFrictionLoss", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_p2frictionloss, -1, -1, -1, -1) \
    FUNCX(vortexXAxisOrientRelaxStiffness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisOrientRelaxationStiffness", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a0stiffness, -1, -1, -1, -1) \
    FUNCX(vortexXAxisOrientRelaxDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisOrientRelaxationDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a0damping, -1, -1, -1, -1) \
    FUNCX(vortexXAxisOrientRelaxLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisOrientRelaxationLoss", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a0loss, -1, -1, -1, -1) \
    FUNCX(vortexXAxisOrientFrictionCoeff, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisOrientFrictionCoeff", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a0frictioncoeff, -1, -1, -1, -1) \
    FUNCX(vortexXAxisOrientFrictionMaxTorque, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisOrientFrictionMaxTorque", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a0frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(vortexXAxisOrientFrictionLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "xAxisOrientFrictionLoss", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a0frictionloss, -1, -1, -1, -1) \
    FUNCX(vortexYAxisOrientRelaxStiffness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisOrientRelaxationStiffness", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a1stiffness, -1, -1, -1, -1) \
    FUNCX(vortexYAxisOrientRelaxDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisOrientRelaxationDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a1damping, -1, -1, -1, -1) \
    FUNCX(vortexYAxisOrientRelaxLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisOrientRelaxationLoss", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a1loss, -1, -1, -1, -1) \
    FUNCX(vortexYAxisOrientFrictionCoeff, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisOrientFrictionCoeff", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a1frictioncoeff, -1, -1, -1, -1) \
    FUNCX(vortexYAxisOrientFrictionMaxTorque, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisOrientFrictionMaxTorque", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a1frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(vortexYAxisOrientFrictionLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "yAxisOrientFrictionLoss", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a1frictionloss, -1, -1, -1, -1) \
    FUNCX(vortexZAxisOrientRelaxStiffness, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisOrientRelaxationStiffness", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a2stiffness, -1, -1, -1, -1) \
    FUNCX(vortexZAxisOrientRelaxDamping, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisOrientRelaxationDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a2damping, -1, -1, -1, -1) \
    FUNCX(vortexZAxisOrientRelaxLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisOrientRelaxationLoss", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a2loss, -1, -1, -1, -1) \
    FUNCX(vortexZAxisOrientFrictionCoeff, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisOrientFrictionCoeff", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a2frictioncoeff, -1, -1, -1, -1) \
    FUNCX(vortexZAxisOrientFrictionMaxTorque, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisOrientFrictionMaxTorque", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a2frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(vortexZAxisOrientFrictionLoss, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "zAxisOrientFrictionLoss", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_a2frictionloss, -1, -1, -1, -1) \
    FUNCX(vortexPosPid, DYNAMICSPREFIXDOT VORTEXPREFIXDOT "posPid", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_vortex_joint_pospid1, sim_vortex_joint_pospid2, sim_vortex_joint_pospid3, -1, -1) \
    FUNCX(newtonPosPid, DYNAMICSPREFIXDOT NEWTONPREFIXDOT "posPid", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_newton_joint_pospid1, sim_newton_joint_pospid2, sim_newton_joint_pospid3, -1, -1) \
    FUNCX(mujocoArmature, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "armature", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_joint_armature, -1, -1, -1, -1) \
    FUNCX(mujocoMargin, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "margin", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_joint_margin, -1, -1, -1, -1) \
    FUNCX(mujocoFrictionLoss, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "frictionLoss", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_joint_frictionloss, -1, -1, -1, -1) \
    FUNCX(mujocoSpringStiffness, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "springStiffness", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_joint_stiffness, -1, -1, -1, -1) \
    FUNCX(mujocoSpringDamping, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "springDamping", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_joint_damping, -1, -1, -1, -1) \
    FUNCX(mujocoSpringRef, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "springRef", sim_propertytype_float, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_joint_springref, -1, -1, -1, -1) \
    FUNCX(mujocoPosPid, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "posPid", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_joint_pospid1, sim_mujoco_joint_pospid2, sim_mujoco_joint_pospid3, -1, -1) \
    FUNCX(mujocoLimitsSolRef, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "limitsSolref", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_joint_solreflimit1, sim_mujoco_joint_solreflimit2, -1, -1, -1) \
    FUNCX(mujocoLimitsSolImp, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "limitsSolimp", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_joint_solimplimit1, sim_mujoco_joint_solimplimit2, sim_mujoco_joint_solimplimit3, sim_mujoco_joint_solimplimit4, sim_mujoco_joint_solimplimit5) \
    FUNCX(mujocoFrictionSolRef, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "frictionSolref", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_joint_solreffriction1, sim_mujoco_joint_solreffriction2, -1, -1, -1) \
    FUNCX(mujocoFrictionSolImp, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "frictionSolimp", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_joint_solimpfriction1, sim_mujoco_joint_solimpfriction2, sim_mujoco_joint_solimpfriction3, sim_mujoco_joint_solimpfriction4, sim_mujoco_joint_solimpfriction5) \
    FUNCX(mujocoSpringDamper, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "springSpringDamper", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_joint_springdamper1, sim_mujoco_joint_springdamper2, -1, -1, -1) \
    FUNCX(mujocoDependencyPolyCoef, DYNAMICSPREFIXDOT MUJOCOPREFIXDOT "dependencyPolyCoeff", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", ""}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  sim_mujoco_joint_polycoef1, sim_mujoco_joint_polycoef2, sim_mujoco_joint_polycoef3, sim_mujoco_joint_polycoef4, sim_mujoco_joint_polycoef5) \
    FUNCX(GROUP_dynamics, DYNAMICSPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamicsBullet, DYNAMICSPREFIXDOT BULLETPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamicsOde, DYNAMICSPREFIXDOT ODEPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamicsNewton, DYNAMICSPREFIXDOT NEWTONPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamicsVortex, DYNAMICSPREFIXDOT VORTEXPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(GROUP_dynamicsMujoco, DYNAMICSPREFIXDOT MUJOCOPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(METHOD_setTargetPosition, "setTargetPosition", sim_propertytype_method, SIM_PROPERTYINFO_METHOD, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    FUNCX(METHOD_setTargetVelocity, "setTargetVelocity", sim_propertytype_method, SIM_PROPERTYINFO_METHOD, PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}}),  -1, -1, -1, -1, -1) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATEDGROUP_dynamicsBullet, BULLETPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP | SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATEDGROUP_dynamicsOde, ODEPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP | SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATEDGROUP_dynamicsNewton, NEWTONPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP | SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATEDGROUP_dynamicsVortex, VORTEXPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP | SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATEDGROUP_dynamicsMujoco, MUJOCOPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP | SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_position, "jointPosition", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_quaternion, "jointQuaternion", sim_propertytype_quaternion, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_targetPos, "targetPos", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_targetVel, "targetVel", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_length, "jointLength", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_diameter, "jointDiameter", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_jointType, "jointType", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_jointForce, "jointForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_averageJointForce, "averageJointForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_jointMode, "jointMode", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_dynCtrlMode, "dynCtrlMode", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_springDamperParams, "springDamperParams", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_dynVelMode, "dynVelMode", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_dynPosMode, "dynPosMode", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_engineProperties, "engineProperties", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletStopErp, BULLETPREFIXDOT "stopErp", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_joint_stoperp, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletStopCfm, BULLETPREFIXDOT "stopCfm", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_joint_stopcfm, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletNormalCfm, BULLETPREFIXDOT "normalCfm", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_joint_normalcfm, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_bulletPosPid, BULLETPREFIXDOT "posPid", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_bullet_joint_pospid1, sim_bullet_joint_pospid2, sim_bullet_joint_pospid3, -1, -1) \
    FUNCX(DEPRECATED_odeStopErp, ODEPREFIXDOT "stopErp", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_joint_stoperp, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odeStopCfm, ODEPREFIXDOT "stopCfm", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_joint_stopcfm, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odeNormalCfm, ODEPREFIXDOT "normalCfm", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_joint_normalcfm, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odeBounce, ODEPREFIXDOT "bounce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_joint_bounce, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odeFudgeFactor, ODEPREFIXDOT "fudge", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_joint_fudgefactor, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_odePosPid, ODEPREFIXDOT "posPid", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_ode_joint_pospid1, sim_ode_joint_pospid2, sim_ode_joint_pospid3, -1, -1) \
    FUNCX(DEPRECATED_vortexLowerLimitDamping, VORTEXPREFIXDOT "axisLimitsLowerDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_lowerlimitdamping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexUpperLimitDamping, VORTEXPREFIXDOT "axisLimitsUpperDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_upperlimitdamping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexLowerLimitStiffness, VORTEXPREFIXDOT "axisLimitsLowerStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_lowerlimitstiffness, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexUpperLimitStiffness, VORTEXPREFIXDOT "axisLimitsUpperStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_upperlimitstiffness, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexLowerLimitRestitution, VORTEXPREFIXDOT "axisLimitsLowerRestitution", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_lowerlimitrestitution, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexUpperLimitRestitution, VORTEXPREFIXDOT "axisLimitsUpperRestitution", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_upperlimitrestitution, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexLowerLimitMaxForce, VORTEXPREFIXDOT "axisLimitsLowerMaxForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_lowerlimitmaxforce, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexUpperLimitMaxForce, VORTEXPREFIXDOT "axisLimitsUpperMaxForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_upperlimitmaxforce, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexAxisFrictionEnabled, VORTEXPREFIXDOT "axisFrictionEnabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_motorfrictionenabled, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexAxisFrictionProportional, VORTEXPREFIXDOT "axisFrictionProportional", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_proportionalmotorfriction, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexAxisFrictionCoeff, VORTEXPREFIXDOT "axisFrictionValue", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_motorconstraintfrictioncoeff, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexAxisFrictionMaxForce, VORTEXPREFIXDOT "axisFrictionMaxForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_motorconstraintfrictionmaxforce, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexAxisFrictionLoss, VORTEXPREFIXDOT "axisFrictionLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_motorconstraintfrictionloss, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexRelaxationEnabledBits, VORTEXPREFIXDOT "relaxationEnabledBits", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_relaxationenabledbc, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexFrictionEnabledBits, VORTEXPREFIXDOT "frictionEnabledBits", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_frictionenabledbc, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexFrictionProportionalBits, VORTEXPREFIXDOT "frictionProportionalBits", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_frictionproportionalbc, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexXAxisPosRelaxationStiffness, VORTEXPREFIXDOT "xAxisPosRelaxationStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p0stiffness, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexXAxisPosRelaxationDamping, VORTEXPREFIXDOT "xAxisPosRelaxationDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p0damping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexXAxisPosRelaxationLoss, VORTEXPREFIXDOT "xAxisPosRelaxationLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p0loss, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexXAxisPosFrictionCoeff, VORTEXPREFIXDOT "xAxisPosFrictionCoeff", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p0frictioncoeff, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexXAxisPosFrictionMaxForce, VORTEXPREFIXDOT "xAxisPosFrictionMaxForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p0frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexXAxisPosFrictionLoss, VORTEXPREFIXDOT "xAxisPosFrictionLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p0frictionloss, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexYAxisPosRelaxationStiffness, VORTEXPREFIXDOT "yAxisPosRelaxationStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p1stiffness, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexYAxisPosRelaxationDamping, VORTEXPREFIXDOT "yAxisPosRelaxationDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p1damping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexYAxisPosRelaxationLoss, VORTEXPREFIXDOT "yAxisPosRelaxationLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p1loss, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexYAxisPosFrictionCoeff, VORTEXPREFIXDOT "yAxisPosFrictionCoeff", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p1frictioncoeff, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexYAxisPosFrictionMaxForce, VORTEXPREFIXDOT "yAxisPosFrictionMaxForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p1frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexYAxisPosFrictionLoss, VORTEXPREFIXDOT "yAxisPosFrictionLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p1frictionloss, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexZAxisPosRelaxationStiffness, VORTEXPREFIXDOT "zAxisPosRelaxationStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p2stiffness, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexZAxisPosRelaxationDamping, VORTEXPREFIXDOT "zAxisPosRelaxationDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p2damping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexZAxisPosRelaxationLoss, VORTEXPREFIXDOT "zAxisPosRelaxationLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p2loss, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexZAxisPosFrictionCoeff, VORTEXPREFIXDOT "zAxisPosFrictionCoeff", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p2frictioncoeff, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexZAxisPosFrictionMaxForce, VORTEXPREFIXDOT "zAxisPosFrictionMaxForce", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p2frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexZAxisPosFrictionLoss, VORTEXPREFIXDOT "zAxisPosFrictionLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_p2frictionloss, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexXAxisOrientRelaxStiffness, VORTEXPREFIXDOT "xAxisOrientRelaxationStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a0stiffness, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexXAxisOrientRelaxDamping, VORTEXPREFIXDOT "xAxisOrientRelaxationDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a0damping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexXAxisOrientRelaxLoss, VORTEXPREFIXDOT "xAxisOrientRelaxationLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a0loss, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexXAxisOrientFrictionCoeff, VORTEXPREFIXDOT "xAxisOrientFrictionCoeff", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a0frictioncoeff, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexXAxisOrientFrictionMaxTorque, VORTEXPREFIXDOT "xAxisOrientFrictionMaxTorque", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a0frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexXAxisOrientFrictionLoss, VORTEXPREFIXDOT "xAxisOrientFrictionLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a0frictionloss, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexYAxisOrientRelaxStiffness, VORTEXPREFIXDOT "yAxisOrientRelaxationStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a1stiffness, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexYAxisOrientRelaxDamping, VORTEXPREFIXDOT "yAxisOrientRelaxationDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a1damping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexYAxisOrientRelaxLoss, VORTEXPREFIXDOT "yAxisOrientRelaxationLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a1loss, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexYAxisOrientFrictionCoeff, VORTEXPREFIXDOT "yAxisOrientFrictionCoeff", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a1frictioncoeff, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexYAxisOrientFrictionMaxTorque, VORTEXPREFIXDOT "yAxisOrientFrictionMaxTorque", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a1frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexYAxisOrientFrictionLoss, VORTEXPREFIXDOT "yAxisOrientFrictionLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a1frictionloss, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexZAxisOrientRelaxStiffness, VORTEXPREFIXDOT "zAxisOrientRelaxationStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a2stiffness, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexZAxisOrientRelaxDamping, VORTEXPREFIXDOT "zAxisOrientRelaxationDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a2damping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexZAxisOrientRelaxLoss, VORTEXPREFIXDOT "zAxisOrientRelaxationLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a2loss, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexZAxisOrientFrictionCoeff, VORTEXPREFIXDOT "zAxisOrientFrictionCoeff", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a2frictioncoeff, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexZAxisOrientFrictionMaxTorque, VORTEXPREFIXDOT "zAxisOrientFrictionMaxTorque", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a2frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexZAxisOrientFrictionLoss, VORTEXPREFIXDOT "zAxisOrientFrictionLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_a2frictionloss, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_vortexPosPid, VORTEXPREFIXDOT "posPid", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_vortex_joint_pospid1, sim_vortex_joint_pospid2, sim_vortex_joint_pospid3, -1, -1) \
    FUNCX(DEPRECATED_newtonPosPid, NEWTONPREFIXDOT "posPid", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_newton_joint_pospid1, sim_newton_joint_pospid2, sim_newton_joint_pospid3, -1, -1) \
    FUNCX(DEPRECATED_mujocoArmature, MUJOCOPREFIXDOT "armature", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_joint_armature, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoMargin, MUJOCOPREFIXDOT "margin", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_joint_margin, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoFrictionLoss, MUJOCOPREFIXDOT "frictionLoss", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_joint_frictionloss, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoSpringStiffness, MUJOCOPREFIXDOT "springStiffness", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_joint_stiffness, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoSpringDamping, MUJOCOPREFIXDOT "springDamping", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_joint_damping, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoSpringRef, MUJOCOPREFIXDOT "springRef", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_joint_springref, -1, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoPosPid, MUJOCOPREFIXDOT "posPid", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_joint_pospid1, sim_mujoco_joint_pospid2, sim_mujoco_joint_pospid3, -1, -1) \
    FUNCX(DEPRECATED_mujocoLimitsSolRef, MUJOCOPREFIXDOT "limitsSolref", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_joint_solreflimit1, sim_mujoco_joint_solreflimit2, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoLimitsSolImp, MUJOCOPREFIXDOT "limitsSolimp", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_joint_solimplimit1, sim_mujoco_joint_solimplimit2, sim_mujoco_joint_solimplimit3, sim_mujoco_joint_solimplimit4, sim_mujoco_joint_solimplimit5) \
    FUNCX(DEPRECATED_mujocoFrictionSolRef, MUJOCOPREFIXDOT "frictionSolref", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_joint_solreffriction1, sim_mujoco_joint_solreffriction2, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoFrictionSolImp, MUJOCOPREFIXDOT "frictionSolimp", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_joint_solimpfriction1, sim_mujoco_joint_solimpfriction2, sim_mujoco_joint_solimpfriction3, sim_mujoco_joint_solimpfriction4, sim_mujoco_joint_solimpfriction5) \
    FUNCX(DEPRECATED_mujocoSpringDamper, MUJOCOPREFIXDOT "springSpringDamper", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_joint_springdamper1, sim_mujoco_joint_springdamper2, -1, -1, -1) \
    FUNCX(DEPRECATED_mujocoDependencyPolyCoef, MUJOCOPREFIXDOT "dependencyPolyCoeff", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  sim_mujoco_joint_polycoef1, sim_mujoco_joint_polycoef2, sim_mujoco_joint_polycoef3, sim_mujoco_joint_polycoef4, sim_mujoco_joint_polycoef5) \
    FUNCX(DEPRECATED_dependencyMaster, "dependencyMasterHandle", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED, PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}}),  -1, -1, -1, -1, -1) \

#define LIGHT_PROPERTIES \
    FUNCX(size, "size", sim_propertytype_float, 0,  PropertyInfo({{"label", "Size"}, {"description", "Light size"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(enabled, "enabled", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Enabled"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(lightType, "type", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Type"}, {"description", "Light type"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(spotExponent, "spotExponent", sim_propertytype_int, 0,  PropertyInfo({{"label", "Spot exponent"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(spotCutoffAngle, "spotCutoffAngle", sim_propertytype_float, 0,  PropertyInfo({{"label", "Cut off angle"}, {"description", "Spot cut off angle"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(attenuationFactors, "attenuationFactors", sim_propertytype_floatarray, 0,  PropertyInfo({{"label", "Attenuation factor"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(povCastShadows, POVRAYPREFIXDOT "castShadows", sim_propertytype_bool, sim_propertyinfo_silent,  PropertyInfo({{"label", "POV-Ray: cast shadows"}, {"description", "Light casts shadows (with the POV-Ray renderer plugin)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_povray, POVRAYPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATED_lightType, "lightType", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_size, "lightSize", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \

#define OCTREE_PROPERTIES \
    FUNCX(voxelSize, "voxelSize", sim_propertytype_float, 0,  PropertyInfo({{"label", "Voxel size"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(randomColors, "randomColors", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Random voxel colors"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(showPoints, "showPoints", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Show points instead of voxels"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(points, "points", sim_propertytype_matrix, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Voxels"}, {"description", "Voxel positions"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(packedPoints, "packedPoints", sim_propertytype_buffer, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Packed voxels"}, {"description", "Voxel positions, as packed floats"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(colors, "colors", sim_propertytype_buffer, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Voxel Colors"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_addFromObject, "addFromObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_subtractFromObject, "subtractFromObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_clear, "clear", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_addVoxels, "addVoxels", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_addPackedVoxels, "addPackedVoxels", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_subtractVoxels, "subtractVoxels", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_subtractPackedVoxels, "subtractPackedVoxels", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_checkPoints, "checkPoints", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_checkPackedPoints, "checkPackedPoints", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATED_voxels, "voxels", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \

#define POINTCLOUD_PROPERTIES \
    FUNCX(ocTreeStruct, "octreeStruct", sim_propertytype_bool, 0,  PropertyInfo({{"label", "OC-Tree structure Enabled"}, {"description", "Use an oc-tree structure"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(randomColors, "randomColors", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Random point colors"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(pointSize, "pointSize", sim_propertytype_int, 0,  PropertyInfo({{"label", "Point size"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(maxPtsInCell, "maxPointsInCell", sim_propertytype_int, 0,  PropertyInfo({{"label", "Max. points in cell"}, {"description", "Maximum number of points in an oc-tree cell/voxel"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(cellSize, "cellSize", sim_propertytype_float, 0,  PropertyInfo({{"label", "Cell size"}, {"description", "Size of the oc-tree cell/voxel"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(pointDisplayFraction, "pointDisplayFraction", sim_propertytype_float, 0,  PropertyInfo({{"label", "Display fraction"}, {"description", "Fraction of points to be displayed in an oc-tree cell/voxel"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(points, "points", sim_propertytype_matrix, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Points"}, {"description", "Point positions"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(packedPoints, "packedPoints", sim_propertytype_buffer, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Packed points"}, {"description", "Point positions, as packed floats"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(colors, "colors", sim_propertytype_buffer, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Colors"}, {"description", "Point colors"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_addFromObject, "addFromObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_subtractFromObject, "subtractFromObject", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_clear, "clear", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_addPoints, "addPoints", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_addPackedPoints, "addPackedPoints", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_subtractPoints, "subtractPoints", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_subtractPackedPoints, "subtractPackedPoints", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_intersectPoints, "intersectPoints", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_intersectPackedPoints, "intersectPackedPoints", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \

#define PROXIMITYSENSOR_PROPERTIES \
    FUNCX(size, "pointSize", sim_propertytype_float, 0,  PropertyInfo({{"label", "Sensor point size"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(frontFaceDetection, "frontFaceDetection", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Front face detection"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(backFaceDetection, "backFaceDetection", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Back face detection"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(exactMode, "exactMode", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Exact mode"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(explicitHandling, "explicitHandling", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Explicit handling"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(showVolume, "showVolume", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Show volume"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(randomizedDetection, "randomizedDetection", sim_propertytype_bool, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Randomized detection"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(sensorType, "type", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Type"}, {"description", "Sensor type"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(detectedObject, "detectedObject", sim_propertytype_handle, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Detected object"}, {"description", "Detected scene object"}, {"handleType", "sceneObject"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(angleThreshold, "angleThreshold", sim_propertytype_float, 0,  PropertyInfo({{"label", "Angle threshold"}, {"description", "Angle threshold, 0.0 to disable"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(detectedPoint, "detectedPoint", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Detected point"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(detectedNormal, "detectedNormal", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Detected normal"}, {"description", "Detected normal vector"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_handleSensor, "handleSensor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_resetSensor, "resetSensor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_checkSensor, "checkSensor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATED_size, "sensorPointSize", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_sensorType, "sensorType", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_detectedObjectHandle, "detectedObjectHandle", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \

#define VISIONSENSOR_PROPERTIES \
    FUNCX(size, "size", sim_propertytype_float, 0,  PropertyInfo({{"label", "Size"}, {"description", "Sensor size"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(backgroundCol, "backgroundColor", sim_propertytype_color, 0,  PropertyInfo({{"label", "Background color"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(renderMode, "renderMode", sim_propertytype_int, 0,  PropertyInfo({{"label", "Render mode"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(backgroundSameAsEnv, "backgroundColorFromEnvironment", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Background color from environment"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(explicitHandling, "explicitHandling", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Explicit handling"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(useExtImage, "useExtImage", sim_propertytype_bool, 0,  PropertyInfo({{"label", "External input"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(ignoreRgbInfo, "ignoreImageInfo", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Ignore RGB buffer"}, {"description", "Ignore RGB buffer (faster)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(ignoreDepthInfo, "ignoreDepthInfo", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Ignore Depth buffer"}, {"description", "Ignore Depth buffer (faster)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(omitPacket1, "omitPacket1", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Packet 1 is blank"}, {"description", "Omit packet 1 (faster)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(emitImageChangedEvent, "emitImageChangedEvent", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Emit image change event"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(emitDepthChangedEvent, "emitDepthChangedEvent", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Emit depth change event"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(imageBuffer, "imageBuffer", sim_propertytype_buffer, sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "RGB buffer"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(depthBuffer, "depth", sim_propertytype_matrix, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Depth buffer"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(packedDepthBuffer, "packedDepthBuffer", sim_propertytype_buffer, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Packed depth buffer"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(triggerState, "triggerState", sim_propertytype_bool, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Trigger state"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(packet1, "packet1", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Data packet 1"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(packet2, "packet2", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Data packet 2"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(povFocalBlur, POVRAYPREFIXDOT "focalBlur", sim_propertytype_bool, sim_propertyinfo_silent,  PropertyInfo({{"label", "POV-Ray: focal blur"}, {"description", "Focal blur (with the POV-Ray renderer plugin)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(povBlurSamples, POVRAYPREFIXDOT "blurSamples", sim_propertytype_int, sim_propertyinfo_silent,  PropertyInfo({{"label", "POV-Ray: blur samples"}, {"description", "Focal blur samples (with the POV-Ray renderer plugin)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(povBlurDistance, POVRAYPREFIXDOT "blurDistance", sim_propertytype_float, sim_propertyinfo_silent,  PropertyInfo({{"label", "POV-Ray: blur distance"}, {"description", "Focal blur distance (with the POV-Ray renderer plugin)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(povAperture, POVRAYPREFIXDOT "aperture", sim_propertytype_float, sim_propertyinfo_silent,  PropertyInfo({{"label", "POV-Ray: aperture"}, {"description", "Aperture (with the POV-Ray renderer plugin)"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(GROUP_povray, POVRAYPREFIX, sim_propertytype_group, SIM_PROPERTYINFO_GROUP,PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_handleSensor, "handleSensor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_resetSensor, "resetSensor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_checkSensor, "checkSensor", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getDepth, "getDepth", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_getImage, "getImage", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_setImage, "setImage", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATED_size, "sensorSize", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_depthBuffer, "depthBuffer", sim_propertytype_floatarray, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \

#define SCRIPT_PROPERTIES \
    FUNCX(size, "size", sim_propertytype_float, 0,  PropertyInfo({{"label", "Size"}, {"description", "Size of the object"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(resetAfterSimError, "resetAfterSimError", sim_propertytype_bool, 0,  PropertyInfo({{"label", "Reset after simulation error"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(detachedScript, "detachedScript", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Detached script handle"}, {"description", ""}, {"handleType", "detachedScript"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    /* Following for backward compatibility: */ \
    FUNCX(DEPRECATED_size, "scriptSize", sim_propertytype_float, SIM_PROPERTYINFO_DEPRECATED,  PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_scriptDisabled, "scriptDisabled", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_restartOnError, "restartOnError", sim_propertytype_bool, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_execPriority, "execPriority", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_scriptType, "scriptType", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_executionDepth, "executionDepth", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_scriptState, "scriptState", sim_propertytype_int, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_language, "language", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_code, "code", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_scriptName, "scriptName", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_silent | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_addOnPath, "addOnPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \
    FUNCX(DEPRECATED_addOnMenuPath, "addOnMenuPath", sim_propertytype_string, SIM_PROPERTYINFO_DEPRECATED | sim_propertyinfo_constant | sim_propertyinfo_notwritable,PropertyInfo({{"startSupport", 1}, {"startDeprecated", 2}, {"endSupport", 2}})) \

#define MARKER_PROPERTIES \
    FUNCX(itemType, "itemType", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Type"}, {"description", "Item type"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(cyclic, "cyclic", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Cyclic"}, {"description", "Item buffer is cyclic"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(local, "local", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Local"}, {"description", "Coordinates are local to the marker's reference frame"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(overlay, "overlay", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Overlay"}, {"description", "Items are are displayed overlaid"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(points, "points", sim_propertytype_matrix, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Points"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(quaternions, "quaternions", sim_propertytype_matrix, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Quaternions"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(sizes, "sizes", sim_propertytype_matrix, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Sizes"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(colors, "colors", sim_propertytype_buffer, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Colors"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(packedPoints, "packedPoints", sim_propertytype_buffer, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Packed points"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(packedQuaternions, "packedQuaternions", sim_propertytype_buffer, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Packed quaternions"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(packedSizes, "packedSizes", sim_propertytype_buffer, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude,  PropertyInfo({{"label", "Packed sizes"}, {"description", ""}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(vertices, "vertices", sim_propertytype_matrix, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Vertices"}, {"description", "Vertices for custom marker"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(indices, "indices", sim_propertytype_intarray, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Indices"}, {"description", "Indices for custom marker"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(normals, "normals", sim_propertytype_matrix, sim_propertyinfo_notwritable,  PropertyInfo({{"label", "Normals"}, {"description", "Normals for custom marker"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_addItems, "addItems", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_clearItems, "clearItems", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \
    FUNCX(METHOD_removeItems, "removeItems", sim_propertytype_method, SIM_PROPERTYINFO_METHOD,  PropertyInfo({{"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \

#define CUSTOMSCENEOBJECT_PROPERTIES \
    FUNCX(size, "size", sim_propertytype_float, 0,  PropertyInfo({{"label", "Size"}, {"description", "Custom scene object size"}, {"startSupport", 2}, {"startDeprecated", 0}, {"endSupport", 0}})) \

// ----------------------------------------------------------------------------------------------
enum class PropObject {
    #define FUNCX(name, str, v1, v2, t1) name,
    OBJECT_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_obj;
const SProperty& prop(PropObject p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropApp {
    #define FUNCX(name, str, v1, v2, t1) name,
    APP_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_app;
const SProperty& prop(PropApp p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropCustomObjectClass {
    #define FUNCX(name, str, v1, v2, t1) name,
    CUSTOMOBJECTCLASS_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_customObjectClass;
const SProperty& prop(PropCustomObjectClass p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropCustomObject {
    #define FUNCX(name, str, v1, v2, t1) name,
    CUSTOMOBJECT_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_customObject;
const SProperty& prop(PropCustomObject p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropDetachedScript {
    #define FUNCX(name, str, v1, v2, t1) name,
    DETACHEDSCRIPT_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_detachedScript;
const SProperty& prop(PropDetachedScript p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropStack {
    #define FUNCX(name, str, v1, v2, t1) name,
    STACK_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_stack;
const SProperty& prop(PropStack p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropCollectionCont {
    #define FUNCX(name, str, v1, v2, t1) name,
    COLLECTIONCONT_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_collCont;
const SProperty& prop(PropCollectionCont p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropCollection {
    #define FUNCX(name, str, v1, v2, t1) name,
    COLLECTION_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_collection;
const SProperty& prop(PropCollection p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropDrawingObjectCont {
    #define FUNCX(name, str, v1, v2, t1) name,
    DRAWINGOBJECTCONT_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_drawCont;
const SProperty& prop(PropDrawingObjectCont p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropDrawingObject {
    #define FUNCX(name, str, v1, v2, t1) name,
    DRAWINGOBJECT_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_drawingObj;
const SProperty& prop(PropDrawingObject p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropColor {
    #define FUNCX(name, str, v1, v2, t1) name,
    COLOR_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_col;
const SProperty& prop(PropColor p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropConvexVolume {
    #define FUNCX(name, str, v1, v2, t1) name,
    CONVEXVOLUME_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_volume;
const SProperty& prop(PropConvexVolume p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropDynCont {
    #define FUNCX(name, str, v1, v2, t1, w0, w1, w2, w3, w4) name,
    DYNCONT_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_dyn;
const SProperty& prop(PropDynCont p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropScene {
    #define FUNCX(name, str, v1, v2, t1) name,
    SCENE_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_scene;
const SProperty& prop(PropScene p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropSimulation {
    #define FUNCX(name, str, v1, v2, t1) name,
    SIMULATION_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_sim;
const SProperty& prop(PropSimulation p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropMeshWrapper {
    #define FUNCX(name, str, v1, v2, t1) name,
    MESHWRAPPER_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_meshWrap;
const SProperty& prop(PropMeshWrapper p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropMesh {
    #define FUNCX(name, str, v1, v2, t1) name,
    MESH_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_mesh;
const SProperty& prop(PropMesh p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropViewableBase {
    #define FUNCX(name, str, v1, v2, t1) name,
    VIEWABLEBASE_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_viewable;
const SProperty& prop(PropViewableBase p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropSceneObjectCont {
    #define FUNCX(name, str, v1, v2, t1) name,
    SCENEOBJECTCONT_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_objCont;
const SProperty& prop(PropSceneObjectCont p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropMaterial {
    #define FUNCX(name, str, v1, v2, t1, w0, w1, w2, w3, w4) name,
    DYNMATERIAL_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_material;
const SProperty& prop(PropMaterial p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropCustomSceneObjectClass {
    #define FUNCX(name, str, v1, v2, t1) name,
    CUSTOMSCENEOBJECTCLASS_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_customSceneObjectClass;
const SProperty& prop(PropCustomSceneObjectClass p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropSceneObject {
    #define FUNCX(name, str, v1, v2, t1) name,
    SCENEOBJECT_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_sceneObject;
const SProperty& prop(PropSceneObject p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropShape {
    #define FUNCX(name, str, v1, v2, t1) name,
    SHAPE_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_shape;
const SProperty& prop(PropShape p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropCamera {
    #define FUNCX(name, str, v1, v2, t1) name,
    CAMERA_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_camera;
const SProperty& prop(PropCamera p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropDummy {
    #define FUNCX(name, str, v1, v2, t1, w0, w1, w2, w3, w4) name,
    DUMMY_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_dummy;
const SProperty& prop(PropDummy p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropForceSensor {
    #define FUNCX(name, str, v1, v2, t1) name,
    FORCESENSOR_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_forceSensor;
const SProperty& prop(PropForceSensor p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropGraph {
    #define FUNCX(name, str, v1, v2, t1) name,
    GRAPH_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_graph;
const SProperty& prop(PropGraph p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropJoint {
    #define FUNCX(name, str, v1, v2, t1, w0, w1, w2, w3, w4) name,
    JOINT_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_joint;
const SProperty& prop(PropJoint p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropLight {
    #define FUNCX(name, str, v1, v2, t1) name,
    LIGHT_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_light;
const SProperty& prop(PropLight p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropOctree {
    #define FUNCX(name, str, v1, v2, t1) name,
    OCTREE_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_ocTree;
const SProperty& prop(PropOctree p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropPointCloud {
    #define FUNCX(name, str, v1, v2, t1) name,
    POINTCLOUD_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_pointCloud;
const SProperty& prop(PropPointCloud p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropProximitySensor {
    #define FUNCX(name, str, v1, v2, t1) name,
    PROXIMITYSENSOR_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_proximitySensor;
const SProperty& prop(PropProximitySensor p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropVisionSensor {
    #define FUNCX(name, str, v1, v2, t1) name,
    VISIONSENSOR_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_visionSensor;
const SProperty& prop(PropVisionSensor p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropScript {
    #define FUNCX(name, str, v1, v2, t1) name,
    SCRIPT_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_script;
const SProperty& prop(PropScript p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropMarker {
    #define FUNCX(name, str, v1, v2, t1) name,
    MARKER_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_marker;
const SProperty& prop(PropMarker p);
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
enum class PropCustomSceneObject {
    #define FUNCX(name, str, v1, v2, t1) name,
    CUSTOMSCENEOBJECT_PROPERTIES
    #undef FUNCX
};
extern const std::vector<SProperty> allProps_customSceneObject;
const SProperty& prop(PropCustomSceneObject p);
// ----------------------------------------------------------------------------------------------

#define DEPRECATION_NO_REPLACE "__noReplace__"

struct SDeprecatedProp {
    std::string replacement;
    std::string type;
    bool deprecated;
};
extern const std::map<std::string, std::vector<SDeprecatedProp>> propDeprecationMappings[2];
