#pragma once

#include <simLib/simConst.h>

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
#define proptypetag_vector2 "&vect2&."
#define proptypetag_vector3 "&vect3&."
#define proptypetag_quaternion "&quat&."
#define proptypetag_pose "&pose&."
#define proptypetag_matrix3x3 "&mtrx33&."
#define proptypetag_matrix4x4 "&mtrx44&."
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

#define OBJECT_META_METHODS R"("getBoolProperty": "sim-2.getBoolProperty",
        "getBufferProperty": "sim-2.getBufferProperty",
        "getColorProperty": "sim-2.getColorProperty",
        "getExplicitHandling": "sim-2.getExplicitHandling",
        "getFloatArrayProperty": "sim-2.getFloatArrayProperty",
        "getFloatProperty": "sim-2.getFloatProperty",
        "getStringArrayProperty": "sim-2.getStringArrayProperty",
        "getHandleArrayProperty": "sim-2.getHandleArrayProperty",
        "getHandleProperty": "sim-2.getHandleProperty",
        "getIntArray2Property": "sim-2.getIntArray2Property",
        "getIntArrayProperty": "sim-2.getIntArrayProperty",
        "getIntProperty": "sim-2.getIntProperty",
        "getLongProperty": "sim-2.getLongProperty",
        "getPoseProperty": "sim-2.getPoseProperty",
        "getProperties": "sim-2.getProperties",
        "getPropertiesInfos": "sim-2.getPropertiesInfos",
        "getProperty": "sim-2.getProperty",
        "getPropertyInfo": "sim-2.getPropertyInfo",
        "getPropertyName": "sim-2.getPropertyName",
        "getPropertyTypeString": "sim-2.getPropertyTypeString",
        "getQuaternionProperty": "sim-2.getQuaternionProperty",
        "getStringProperty": "sim-2.getStringProperty",
        "getTableProperty": "sim-2.getTableProperty",
        "getVector2Property": "sim-2.getVector2Property",
        "getVector3Property": "sim-2.getVector3Property",
        "removeProperty": "sim-2.removeProperty",
        "setBoolProperty": "sim-2.setBoolProperty",
        "setBufferProperty": "sim-2.setBufferProperty",
        "setColorProperty": "sim-2.setColorProperty",
        "setFloatArrayProperty": "sim-2.setFloatArrayProperty",
        "setFloatProperty": "sim-2.setFloatProperty",
        "setStringArrayProperty": "sim-2.setStringArrayProperty",
        "setHandleArrayProperty": "sim-2.setHandleArrayProperty",
        "setHandleProperty": "sim-2.setHandleProperty",
        "setIntArray2Property": "sim-2.setIntArray2Property",
        "setIntArrayProperty": "sim-2.setIntArrayProperty",
        "setIntProperty": "sim-2.setIntProperty",
        "setLongProperty": "sim-2.setLongProperty",
        "setPoseProperty": "sim-2.setPoseProperty",
        "setProperties": "sim-2.setProperties",
        "setProperty": "sim-2.setProperty",
        "setQuaternionProperty": "sim-2.setQuaternionProperty",
        "setStringProperty": "sim-2.setStringProperty",
        "setTableProperty": "sim-2.setTableProperty",
        "setVector2Property": "sim-2.setVector2Property",
        "setVector3Property": "sim-2.setVector3Property")"

#define APP_META_METHODS R"("loadPlugin": "sim-2.loadPlugin",
        "unloadPlugin": "sim-2.unloadPlugin")"

#define DETACHEDSCRIPT_META_METHODS R"("callFunction": "sim-2.callScriptFunction",
        "executeScriptString": "sim-2.executeScriptString",
        "getApiFunc": "sim-2.getApiFunc",
        "getApiInfo": "sim-2.getApiInfo",
        "getStackTraceback": "sim-2.getStackTraceback",
        "init": "sim-2.initScript")"

#define COLLECTION_META_METHODS R"("addItem": "sim-2.addToCollection",
        "removeItem": "sim-2.removeFromCollection",
        "checkCollision": "sim-2.checkCollision",
        "checkDistance": "sim-2.checkDistance",
        "remove": "sim-2.removeCollection")"

#define DRAWINGOBJECT_META_METHODS R"("addItem": "sim-2.addDrawingObjectItem",
        "remove": "sim-2.removeDrawingObject")"

#define MESH_META_METHODS R"("__": "__")"

#define SCENE_META_METHODS R"("getAncestors": "sim-2.getObjectAncestors",
        "getDescendants": "sim-2.getObjectDescendants",
        "load": "sim-2.loadScene",
        "save": "sim-2.saveScene")"

#define SCENEOBJECT_META_METHODS R"("getAlias": "sim-2.getObjectAlias",
        "getPose": "sim-2.getObjectPose",
        "getPosition": "sim-2.getObjectPosition",
        "getQuaternion": "sim-2.getObjectQuaternion",
        "getVelocity": "sim-2.getObjectVelocity",
        "scaleObject": "sim-2.scaleObject",
        "setParent": "sim-2.setObjectParent",
        "setPose": "sim-2.setObjectPose",
        "setPosition": "sim-2.setObjectPosition",
        "setQuaternion": "sim-2.setObjectQuaternion",
        "visitTree": "sim-2.visitTree",
        "dynamicReset": "sim-2.resetDynamicObject",
        "getDescendants": "sim-2.getObjectDescendants",
        "getAncestors": "sim-2.getObjectAncestors",
        )" OBJECT_META_METHODS

#define SHAPE_META_METHODS R"("addForce": "sim-2.addForce",
        "addForceAndTorque": "sim-2.addForceAndTorque",
        "alignBB": "sim-2.alignShapeBB",
        "checkCollision": "sim-2.checkCollision",
        "checkDistance": "sim-2.checkDistance",
        "computeMassAndInertia": "sim-2.computeMassAndInertia",
        "getAppearance": "sim-2.getShapeAppearance",
        "relocateFrame": "sim-2.relocateShapeFrame",
        "setAppearance": "sim-2.setShapeAppearance",
        "setShapeBB": "sim-2.setShapeBB",
        "ungroup": "sim-2.ungroupShape",
        "getMeshData": "sim-2.getShapeMesh")"

#define CAMERA_META_METHODS R"("__": "__")"

#define FORCESENSOR_META_METHODS R"("__": "__")"

#define LIGHT_META_METHODS R"("__": "__")"

#define MIRROR_META_METHODS R"("__": "__")"

#define SCRIPT_META_METHODS R"("__": "__")"

#define DUMMY_META_METHODS R"("checkCollision": "sim-2.checkCollision",
        "checkDistance": "sim-2.checkDistance")"

#define GRAPH_META_METHODS R"("addCurve": "sim-2.addGraphCurve",
        "addStream": "sim-2.addGraphStream",
        "resetGraph": "sim-2.resetGraph")"

#define JOINT_META_METHODS R"("getForce": "sim-2.getJointForce",
        "resetDynamicObject": "sim-2.resetDynamicObject",
        "getVelocity": "sim-2.getJointVelocity")"

#define OCTREE_META_METHODS R"("checkCollision": "sim-2.checkCollision",
        "checkDistance": "sim-2.checkDistance",
        "checkPointOccupancy": "sim-2.checkOctreePointOccupancy",
        "insertObject": "sim-2.insertObjectIntoOctree",
        "insertVoxels": "sim-2.insertVoxelsIntoOctree",
        "removeVoxels": "sim-2.removeVoxelsFromOctree",
        "subtractObject": "sim-2.subtractObjectFromOctree")"

#define POINTCLOUD_META_METHODS R"("checkCollision": "sim-2.checkCollision",
        "checkDistance": "sim-2.checkDistance",
        "insertObject": "sim-2.insertObjectIntoPointCloud",
        "insertPoints": "sim-2.insertPointsIntoPointCloud",
        "intersectPoints": "sim-2.intersectPointsWithPointCloud",
        "removePoints": "sim-2.removePointsFromPointCloud",
        "subtractObject": "sim-2.subtractObjectFromPointCloud")"

#define PROXIMITYSENSOR_META_METHODS R"("handleSensor": "sim-2.handleProximitySensor",
        "resetSensor": "sim-2.resetProximitySensor",
        "checkSensor": "sim-2.checkProximitySensor")"

#define VISIONSENSOR_META_METHODS R"("handleSensor": "sim-2.handleVisionSensor",
        "resetSensor": "sim-2.resetVisionSensor",
        "checkSensor": "sim-2.checkVisionSensor",
        "getImage": "sim-2.getVisionSensorImg",
        "setImage": "sim-2.setVisionSensorImg")"


#define APP_PROPERTIES \
    FUNCX(propApp_objectType, "objectType", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Object type", "description": ""})===", "") \
    FUNCX(propApp_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Object meta information", "description": ""})===", "") \
    FUNCX(propApp_handle, "handle", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Handle", "description": "", "handleType": ""})===", "") \
    FUNCX(propApp_sessionId, "sessionId", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Session ID", "description": ""})===", "") \
    FUNCX(propApp_protocolVersion, "protocolVersion", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable, R"===({"label": "Protocol", "description": "Protocol version"})===", "") \
    FUNCX(propApp_productVersion, "productVersion", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable, R"===({"label": "Product string", "description": "Product version (string)"})===", "") \
    FUNCX(propApp_productVersionNb, "productVersionNb", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable, R"===({"label": "Product", "description": "Product version (number)"})===", "") \
    FUNCX(propApp_platform, "platform", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable, R"===({"label": "Platform", "description": "Platform (0: Windows, 1: macOS, 2: Linux)"})===", "") \
    FUNCX(propApp_flavor, "flavor", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable, R"===({"label": "Flavor", "description": "Flavor (0: lite, 1: edu, 2: pro)"})===", "") \
    FUNCX(propApp_qtVersion, "qtVersion", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable, R"===({"label": "Qt", "description": "Qt version"})===", "") \
    FUNCX(propApp_processId, "processId", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Process", "description": "Process ID"})===", "") \
    FUNCX(propApp_processCnt, "processCnt", sim_propertytype_int, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Processes", "description": "Overall processes"})===", "") \
    FUNCX(propApp_consoleVerbosity, "consoleVerbosity", sim_propertytype_int, 0, R"===({"label": "Console verbosity", "description": ""})===", "") \
    FUNCX(propApp_statusbarVerbosity, "statusbarVerbosity", sim_propertytype_int, 0, R"===({"label": "Statusbar verbosity", "description": ""})===", "") \
    FUNCX(propApp_dialogVerbosity, "dialogVerbosity", sim_propertytype_int, 0, R"===({"label": "Dialog verbosity", "description": ""})===", "") \
    FUNCX(propApp_consoleVerbosityStr, "consoleVerbosityStr", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude, R"===({"label": "Console verbosity string", "description": "Console verbosity string, only for client app"})===", "") \
    FUNCX(propApp_statusbarVerbosityStr, "statusbarVerbosityStr", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude, R"===({"label": "Statusbar verbosity string", "description": "Statusbar verbosity string, only for client app"})===", "") \
    FUNCX(propApp_dialogVerbosityStr, "dialogVerbosityStr", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude, R"===({"label": "Dialog verbosity string", "description": "Dialog verbosity string, only for client app"})===", "") \
    FUNCX(propApp_auxAddOn1, "auxAddOn1", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, R"===({"label": "Aux. add-on 1", "description": "Auxiliary add-on 1"})===", "") \
    FUNCX(propApp_auxAddOn2, "auxAddOn2", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, R"===({"label": "Aux. add-on 2", "description": "Auxiliary add-on 2"})===", "") \
    FUNCX(propApp_startupCode, "startupCode", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, R"===({"label": "Start-up code", "description": ""})===", "") \
    FUNCX(propApp_defaultTranslationStepSize, "defaultTranslationStepSize", sim_propertytype_float, 0, R"===({"label": "Translation step size", "description": "Default translation step size"})===", "") \
    FUNCX(propApp_defaultRotationStepSize, "defaultRotationStepSize", sim_propertytype_float, 0, R"===({"label": "Rotation step size", "description": "Default rotation step size"})===", "") \
    FUNCX(propApp_hierarchyEnabled, "hierarchyEnabled", sim_propertytype_bool, sim_propertyinfo_modelhashexclude, R"===({"label": "Hierarchy enabled", "description": ""})===", "") \
    FUNCX(propApp_browserEnabled, "browserEnabled", sim_propertytype_bool, sim_propertyinfo_modelhashexclude, R"===({"label": "Browser enabled", "description": ""})===", "") \
    FUNCX(propApp_displayEnabled, "displayEnabled", sim_propertytype_bool, sim_propertyinfo_modelhashexclude, R"===({"label": "Display enabled", "description": ""})===", "") \
    FUNCX(propApp_appDir, "appPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Application path", "description": ""})===", "") \
    FUNCX(propApp_machineId, "machineId", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Machine ID", "description": ""})===", "") \
    FUNCX(propApp_legacyMachineId, "legacyMachineId", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Legacy machine ID", "description": ""})===", "") \
    FUNCX(propApp_tempDir, "tempPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Temporary path", "description": ""})===", "") \
    FUNCX(propApp_sceneTempDir, "sceneTempPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Scene temporary path", "description": ""})===", "") \
    FUNCX(propApp_settingsDir, "settingsPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Settings path", "description": ""})===", "") \
    FUNCX(propApp_luaDir, "luaPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Lua path", "description": ""})===", "") \
    FUNCX(propApp_pythonDir, "pythonPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Python path", "description": ""})===", "") \
    FUNCX(propApp_mujocoDir, "mujocoPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "MuJoCo path", "description": ""})===", "") \
    FUNCX(propApp_systemDir, "systemPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "System path", "description": ""})===", "") \
    FUNCX(propApp_resourceDir, "resourcePath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Resource path", "description": ""})===", "") \
    FUNCX(propApp_addOnDir, "addOnPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Add-on path", "description": ""})===", "") \
    FUNCX(propApp_sceneDir, "scenePath", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, R"===({"label": "Scene path", "description": ""})===", "") \
    FUNCX(propApp_modelDir, "modelPath", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, R"===({"label": "Model path", "description": ""})===", "") \
    FUNCX(propApp_importExportDir, "importExportPath", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, R"===({"label": "Import/export path", "description": ""})===", "") \
    FUNCX(propApp_defaultPython, "defaultPython", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Default Python", "description": "Default Python interpreter"})===", "") \
    FUNCX(propApp_sandboxLang, "sandboxLang", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable, R"===({"label": "Sandbox language", "description": "Default sandbox language"})===", "") \
    FUNCX(propApp_headlessMode, "headlessMode", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Headless mode", "description": "Headless mode (0: not headless, 1: GUI suppressed, 2: headless library)"})===", "") \
    FUNCX(propApp_canSave, "canSave", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Can save", "description": "Whether save operation is allowed in given state"})===", "") \
    FUNCX(propApp_idleFps, "idleFps", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Loaded plugin names", "description": ""})===", "") \
    FUNCX(propApp_pluginNames, "pluginNames", sim_propertytype_stringarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Plugins", "description": "List of plugins"})===", "") \
    FUNCX(propApp_addOns, "addOns", sim_propertytype_handlearray, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Add-ons", "description": "List of add-ons", "handleType": "detachedScript"})===", "") \
    FUNCX(propApp_sandbox, "sandbox", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Sandbox", "description": "Handle of the sandbox script", "handleType": "detachedScript"})", "") \
    FUNCX(propApp_appArgs, "appArgs", sim_propertytype_stringarray, sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, R"===({"label": "App args", "description": ""})===", "") \
    FUNCX(propApp_appArg1, "appArg1", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, R"===({"label": "App arg. 1", "description": ""})===", "") \
    FUNCX(propApp_appArg2, "appArg2", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, R"===({"label": "App arg. 2", "description": ""})===", "") \
    FUNCX(propApp_appArg3, "appArg3", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, R"===({"label": "App arg. 3", "description": ""})===", "") \
    FUNCX(propApp_appArg4, "appArg4", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, R"===({"label": "App arg. 4", "description": ""})===", "") \
    FUNCX(propApp_appArg5, "appArg5", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, R"===({"label": "App arg. 5", "description": ""})===", "") \
    FUNCX(propApp_appArg6, "appArg6", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, R"===({"label": "App arg. 6", "description": ""})===", "") \
    FUNCX(propApp_appArg7, "appArg7", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, R"===({"label": "App arg. 7", "description": ""})===", "") \
    FUNCX(propApp_appArg8, "appArg8", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, R"===({"label": "App arg. 8", "description": ""})===", "") \
    FUNCX(propApp_appArg9, "appArg9", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, R"===({"label": "App arg. 9", "description": ""})===", "") \
    FUNCX(propApp_randomQuaternion, "randomQuaternion", sim_propertytype_quaternion, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Random quaternion", "description": ""})===", "") \
    FUNCX(propApp_randomFloat, "randomFloat", sim_propertytype_float, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Random number", "description": ""})===", "") \
    FUNCX(propApp_randomString, "randomString", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Random string", "description": ""})===", "") \
    FUNCX(propApp_notifyDeprecated, "notifyDeprecated", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Notify deprecated", "description": "Notify deprecated API (0: off, 1: light, 2: full)"})===", "") \
    FUNCX(propApp_execUnsafe, "execUnsafe", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Execute unsafe", "description": ""})===", "") \
    FUNCX(propApp_execUnsafeExt, "execUnsafeExt", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Execute unsafe extended", "description": "Execute unsafe for code triggered externally"})===", "") \
    FUNCX(propApp_dongleSerial, "dongleSerial", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propApp_machineSerialND, "machineSerialND", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propApp_machineSerial, "machineSerial", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propApp_dongleID, "dongleID", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propApp_machineIDX, "machineIDX", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propApp_machineID0, "machineID0", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propApp_machineID1, "machineID1", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propApp_machineID2, "machineID2", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propApp_machineID3, "machineID3", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propApp_pid, "pid", sim_propertytype_long, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "PID", "description": ""})===", "")

#define DETACHEDSCRIPT_PROPERTIES \
    FUNCX(propScriptObj_objectType, "objectType", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Object type", "description": ""})===", "") \
    FUNCX(propScriptObj_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Object meta information", "description": ""})===", "") \
    FUNCX(propScriptObj_handle, "handle", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Handle", "description": "", "handleType": ""})===", "") \
    FUNCX(propScriptObj_scriptDisabled, "scriptDisabled", sim_propertytype_bool, 0, R"===({"label": "Enabled", "description": "Script is enabled"})===", "") \
    FUNCX(propScriptObj_restartOnError, "restartOnError", sim_propertytype_bool, 0, R"===({"label": "Restart", "description": "Restart on error"})===", "") \
    FUNCX(propScriptObj_execPriority, "execPriority", sim_propertytype_int, 0, R"===({"label": "Execution priority", "description": ""})===", "") \
    FUNCX(propScriptObj_scriptType, "scriptType", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable, R"===({"label": "Type", "description": "Script type"})===", "") \
    FUNCX(propScriptObj_executionDepth, "executionDepth", sim_propertytype_int, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Execution depth", "description": ""})===", "") \
    FUNCX(propScriptObj_scriptState, "scriptState", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "State", "description": "Script state"})===", "") \
    FUNCX(propScriptObj_language, "language", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable, R"===({"label": "Language", "description": ""})===", "") \
    FUNCX(propScriptObj_code, "code", sim_propertytype_string, 0, R"===({"label": "Code", "description": "Script content"})===", "") \
    FUNCX(propScriptObj_scriptName, "scriptName", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Name", "description": "Script name"})===", "") \
    FUNCX(propScriptObj_addOnPath, "addOnPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Add-on path", "description": "Path of add-on"})===", "") \
    FUNCX(propScriptObj_addOnMenuPath, "addOnMenuPath", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Add-on menu path", "description": "Menu path of add-on"})===", "")

#define COLLECTIONCONT_PROPERTIES \
    FUNCX(propCollCont_collections, "collections", sim_propertytype_handlearray, sim_propertyinfo_notwritable, R"({"label": "Collections", "description": "Handles of all collections", "handleType": "collection"})", "")

#define COLLECTION_PROPERTIES \
    FUNCX(propCollection_objectType, "objectType", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Object type", "description": ""})", "") \
    FUNCX(propCollection_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Object meta information", "description": ""})", "") \
    FUNCX(propCollection_handle, "handle", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Handle", "description": "", "handleType": ""})", "") \
    FUNCX(propCollection_objects, "objects", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Children handles", "description": "", "handleType": "sceneObject"})", "")

#define DRAWINGOBJECTCONT_PROPERTIES \
    FUNCX(propDrawCont_drawingObjects, "drawingObjects", sim_propertytype_handlearray, sim_propertyinfo_notwritable, R"({"label": "Drawing objects", "description": "Handles of all drawing objects", "handleType": "drawingObject"})", "")

#define DRAWINGOBJECT_PROPERTIES \
    FUNCX(propDrawingObj_objectType, "objectType", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Object type", "description": ""})", "") \
    FUNCX(propDrawingObj_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Object meta information", "description": ""})", "") \
    FUNCX(propDrawingObj_handle, "handle", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Handle", "description": "", "handleType": ""})", "")

#define COLOR_PROPERTIES \
    FUNCX(propCol_colDiffuse, "diffuse", sim_propertytype_color, 0, R"({"label": "Diffuse color", "description": ""})", "") \
    FUNCX(propCol_colSpecular, "specular", sim_propertytype_color, 0, R"({"label": "Specular color", "description": ""})", "") \
    FUNCX(propCol_colEmission, "emission", sim_propertytype_color, 0, R"({"label": "Emission color", "description": ""})", "") \
    FUNCX(propCol_transparency, "transparency", sim_propertytype_float, 0, R"({"label": "Transparency", "description": ""})", "")

#define CONVEXVOLUME_PROPERTIES \
    FUNCX(propVolume_closeThreshold, "closeThreshold", sim_propertytype_float, 0, R"===({"label": "Close threshold", "description": "Close threshold: if a detection occures below that threshold, it is not registered. 0.0 to disable"})===", "") \
    FUNCX(propVolume_offset, "volume_offset", sim_propertytype_float, 0, R"===({"label": "Offset", "description": "Offset of detection volume"})===", "") \
    FUNCX(propVolume_range, "volume_range", sim_propertytype_float, 0, R"===({"label": "Range", "description": "Range/depth of detection volume"})===", "") \
    FUNCX(propVolume_xSize, "volume_xSize", sim_propertytype_floatarray, 0, R"===({"label": "X-sizes", "description": "X-size (near and far) for pyramid-type volumes"})===", "") \
    FUNCX(propVolume_ySize, "volume_ySize", sim_propertytype_floatarray, 0, R"===({"label": "Y-sizes", "description": "Y-size (near and far) for pyramid-type volumes"})===", "") \
    FUNCX(propVolume_radius, "volume_radius", sim_propertytype_floatarray, 0, R"===({"label": "Radius", "description": "Radius for cylinder-, disk- and cone-type volumes"})===", "") \
    FUNCX(propVolume_angle, "volume_angle", sim_propertytype_floatarray, 0, R"===({"label": "Angles", "description": "Angle and inside gap for disk- and cone-type volumes"})===", "") \
    FUNCX(propVolume_faces, "volume_faces", sim_propertytype_intarray, 0, R"===({"label": "Faces", "description": "Number of faces (near and far) for cylinder-, disk- and pyramid-type volumes"})===", "") \
    FUNCX(propVolume_subdivisions, "volume_subdivisions", sim_propertytype_intarray, 0, R"===({"label": "Subdivisions", "description": "Number of subdivisions (near and far) for cone-type volumes"})===", "") \
    FUNCX(propVolume_edges, "volume_edges", sim_propertytype_floatarray, sim_propertyinfo_notwritable, R"===({"label": "Volume edges", "description": "List of segments (defined by pairs of end-point coordinates) visualizing the volume"})===", "") \
    FUNCX(propVolume_closeEdges, "volume_closeEdges", sim_propertytype_floatarray, sim_propertyinfo_notwritable, R"===({"label": "Volume close edges", "description": "List of segments (defined by pairs of end-point coordinates) visualizing the close threshold of the volume"})===", "")

#define DYNCONT_PROPERTIES \
    FUNCX(propDyn_dynamicsEnabled, "dynamicsEnabled", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "Dynamics enabled", "description": ""})", "") \
    FUNCX(propDyn_showContactPoints, "showContactPoints", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "Show contact points", "description": ""})", "") \
    FUNCX(propDyn_dynamicsEngine, "dynamicsEngine", sim_propertytype_intarray, 0, -1, -1, -1, -1, -1, R"({"label": "Dynamics engine", "description": "Selected dynamics engine index and version"})", "") \
    FUNCX(propDyn_dynamicsStepSize, "dynamicsStepSize", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"({"label": "Dynamics dt", "description": "Dynamics step size"})", "") \
    FUNCX(propDyn_gravity, "gravity", sim_propertytype_vector3, 0, -1, -1, -1, -1, -1, R"({"label": "Gravity", "description": ""})", "") \
    FUNCX(propDyn_engineProperties, "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"({"label": "Engine properties", "description": "Engine properties as JSON text"})", "") \
    FUNCX(propDyn_bulletSolver, "bullet.solver", sim_propertytype_int, 0, sim_bullet_global_constraintsolvertype, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_bulletIterations, "bullet.iterations", sim_propertytype_int, 0, sim_bullet_global_constraintsolvingiterations, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_bulletComputeInertias, "bullet.computeInertias", sim_propertytype_bool, 0, sim_bullet_global_computeinertias, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_bulletInternalScalingFull, "bullet.internalScalingFull", sim_propertytype_bool, 0, sim_bullet_global_fullinternalscaling, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_bulletInternalScalingScaling, "bullet.internalScalingValue", sim_propertytype_float, 0, sim_bullet_global_internalscalingfactor, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_bulletCollMarginScaling, "bullet.collisionMarginScaling", sim_propertytype_float, 0, sim_bullet_global_collisionmarginfactor, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_odeQuickStepEnabled, "ode.quickStepEnabled", sim_propertytype_bool, 0, sim_ode_global_quickstep, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_odeQuickStepIterations, "ode.quickStepIterations", sim_propertytype_int, 0, sim_ode_global_constraintsolvingiterations, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_odeComputeInertias, "ode.computeInertias", sim_propertytype_bool, 0, sim_ode_global_computeinertias, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_odeInternalScalingFull, "ode.internalScalingFull", sim_propertytype_bool, 0, sim_ode_global_fullinternalscaling, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_odeInternalScalingScaling, "ode.internalScalingValue", sim_propertytype_float, 0, sim_ode_global_internalscalingfactor, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_odeGlobalErp, "ode.globalErp", sim_propertytype_float, 0, sim_ode_global_erp, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_odeGlobalCfm, "ode.globalCfm", sim_propertytype_float, 0, sim_ode_global_cfm, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_vortexComputeInertias, "vortex.computeInertias", sim_propertytype_bool, 0, sim_vortex_global_computeinertias, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_vortexContactTolerance, "vortex.contactTolerance", sim_propertytype_float, 0, sim_vortex_global_contacttolerance, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_vortexAutoSleep, "vortex.autoSleep", sim_propertytype_bool, 0, sim_vortex_global_autosleep, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_vortexMultithreading, "vortex.multithreading", sim_propertytype_bool, 0, sim_vortex_global_multithreading, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_vortexConstraintsLinearCompliance, "vortex.constraintsLinearCompliance", sim_propertytype_float, 0, sim_vortex_global_constraintlinearcompliance, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_vortexConstraintsLinearDamping, "vortex.constraintsLinearDamping", sim_propertytype_float, 0, sim_vortex_global_constraintlineardamping, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_vortexConstraintsLinearKineticLoss, "vortex.constraintsLinearKineticLoss", sim_propertytype_float, 0, sim_vortex_global_constraintlinearkineticloss, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_vortexConstraintsAngularCompliance, "vortex.constraintsAngularCompliance", sim_propertytype_float, 0, sim_vortex_global_constraintangularcompliance, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_vortexConstraintsAngularDamping, "vortex.constraintsAngularDamping", sim_propertytype_float, 0, sim_vortex_global_constraintangulardamping, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_vortexConstraintsAngularKineticLoss, "vortex.constraintsAngularKineticLoss", sim_propertytype_float, 0, sim_vortex_global_constraintangularkineticloss, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_newtonIterations, "newton.iterations", sim_propertytype_int, 0, sim_newton_global_constraintsolvingiterations, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_newtonComputeInertias, "newton.computeInertias", sim_propertytype_bool, 0, sim_newton_global_computeinertias, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_newtonMultithreading, "newton.multithreading", sim_propertytype_bool, 0, sim_newton_global_multithreading, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_newtonExactSolver, "newton.exactSolver", sim_propertytype_bool, 0, sim_newton_global_exactsolver, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_newtonHighJointAccuracy, "newton.highJointAccuracy", sim_propertytype_bool, 0, sim_newton_global_highjointaccuracy, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_newtonContactMergeTolerance, "newton.contactMergeTolerance", sim_propertytype_float, 0, sim_newton_global_contactmergetolerance, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoIntegrator, "mujoco.integrator", sim_propertytype_int, 0, sim_mujoco_global_integrator, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoSolver, "mujoco.solver", sim_propertytype_int, 0, sim_mujoco_global_solver, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoIterations, "mujoco.iterations", sim_propertytype_int, 0, sim_mujoco_global_iterations, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoRebuildTrigger, "mujoco.rebuildTrigger", sim_propertytype_int, 0, sim_mujoco_global_rebuildtrigger, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoComputeInertias, "mujoco.computeInertias", sim_propertytype_bool, 0, sim_mujoco_global_computeinertias, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoMbMemory, "mujoco.mbmemory", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoNjMax, "mujoco.njmax", sim_propertytype_int, 0, sim_mujoco_global_njmax, -1, -1, -1, -1, R"({"label": "", "description": "Deprecated. See mujoco.mbmemory instead."})", "") \
    FUNCX(propDyn_mujocoNconMax, "mujoco.nconmax", sim_propertytype_int, 0, sim_mujoco_global_nconmax, -1, -1, -1, -1, R"({"label": "", "description": "Deprecated. See mujoco.mbmemory instead."})", "") \
    FUNCX(propDyn_mujocoNstack, "mujoco.nstack", sim_propertytype_int, 0, sim_mujoco_global_nstack, -1, -1, -1, -1, R"({"label": "", "description": "Deprecated. See mujoco.mbmemory instead."})", "") \
    FUNCX(propDyn_mujocoCone, "mujoco.cone", sim_propertytype_int, 0, sim_mujoco_global_cone, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoKinematicBodiesOverrideFlags, "mujoco.kinematicBodiesOverrideFlags", sim_propertytype_int, 0, sim_mujoco_global_overridekin, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoKinematicBodiesMass, "mujoco.kinematicBodiesMass", sim_propertytype_float, 0, sim_mujoco_global_kinmass, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoKinematicBodiesInertia, "mujoco.kinematicBodiesInertia", sim_propertytype_float, 0, sim_mujoco_global_kininertia, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoBoundMass, "mujoco.boundMass", sim_propertytype_float, 0, sim_mujoco_global_boundmass, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoBoundInertia, "mujoco.boundInertia", sim_propertytype_float, 0, sim_mujoco_global_boundinertia, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoBalanceInertias, "mujoco.balanceInertias", sim_propertytype_bool, 0, sim_mujoco_global_balanceinertias, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoMultithreaded, "mujoco.multithreaded", sim_propertytype_bool, 0, sim_mujoco_global_multithreaded, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoMulticcd, "mujoco.multiccd", sim_propertytype_bool, 0, sim_mujoco_global_multiccd, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoContactParamsOverride, "mujoco.contactParamsOverride", sim_propertytype_bool, 0, sim_mujoco_global_overridecontacts, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoContactParamsMargin, "mujoco.contactParamsMargin", sim_propertytype_float, 0, sim_mujoco_global_overridemargin, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoContactParamsSolref, "mujoco.contactParamsSolref", sim_propertytype_floatarray, 0, sim_mujoco_global_overridesolref1, sim_mujoco_global_overridesolref2, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoContactParamsSolimp, "mujoco.contactParamsSolimp", sim_propertytype_floatarray, 0, sim_mujoco_global_overridesolimp1, sim_mujoco_global_overridesolimp2, sim_mujoco_global_overridesolimp3, sim_mujoco_global_overridesolimp4, sim_mujoco_global_overridesolimp5, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoImpRatio, "mujoco.impratio", sim_propertytype_float, 0, sim_mujoco_global_impratio, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoWind, "mujoco.wind", sim_propertytype_vector3, 0, sim_mujoco_global_wind1, sim_mujoco_global_wind2, sim_mujoco_global_wind3, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoDensity, "mujoco.density", sim_propertytype_float, 0, sim_mujoco_global_density, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoViscosity, "mujoco.viscosity", sim_propertytype_float, 0, sim_mujoco_global_viscosity, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoJacobian, "mujoco.jacobian", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoTolerance, "mujoco.tolerance", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoLs_iterations, "mujoco.ls_iterations", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoLs_tolerance, "mujoco.ls_tolerance", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoNoslip_iterations, "mujoco.noslip_iterations", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoNoslip_tolerance, "mujoco.noslip_tolerance", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoCcd_iterations, "mujoco.ccd_iterations", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoCcd_tolerance, "mujoco.ccd_tolerance", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoSdf_iterations, "mujoco.sdf_iterations", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoSdf_initpoints, "mujoco.sdf_initpoints", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoEqualityEnable, "mujoco.equalityEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoFrictionlossEnable, "mujoco.frictionlossEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoLimitEnable, "mujoco.limitEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoContactEnable, "mujoco.contactEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoPassiveEnable, "mujoco.passiveEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoGravityEnable, "mujoco.gravityEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoWarmstartEnable, "mujoco.warmstartEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoActuationEnable, "mujoco.actuationEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoRefsafeEnable, "mujoco.refsafeEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoSensorEnable, "mujoco.sensorEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoMidphaseEnable, "mujoco.midphaseEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoEulerdampEnable, "mujoco.eulerdampEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoAutoresetEnable, "mujoco.autoresetEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoEnergyEnable, "mujoco.energyEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoInvdiscreteEnable, "mujoco.invdiscreteEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoNativeccdEnable, "mujoco.nativeccdEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoAlignfree, "mujoco.alignfree", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoKinematicWeldSolref, "mujoco.kinematicWeldSolref", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoKinematicWeldSolimp, "mujoco.kinematicWeldSolimp", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoKinematicWeldTorqueScale, "mujoco.kinematicWeldTorquescale", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")

#define ENVIRONMENT_PROPERTIES \
    FUNCX(propScene_objectType, "objectType", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Object type", "description": ""})", "") \
    FUNCX(propScene_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Object meta information", "description": ""})", "") \
    FUNCX(propScene_handle, "handle", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Handle", "description": "", "handleType": ""})", "") \
    FUNCX(propScene_mainScript, "mainScript", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Main script", "description": "Handle of the main script", "handleType": "detachedScript"})", "") \
    FUNCX(propScene_finalSaveRequest, "finalSaveRequest", sim_propertytype_bool, sim_propertyinfo_modelhashexclude, R"({"label": "Final save", "description": "Lock scene and models after next scene save operation"})", "") \
    FUNCX(propScene_sceneIsLocked, "sceneIsLocked", sim_propertytype_bool, sim_propertyinfo_notwritable, R"({"label": "Scene is locked", "description": ""})", "") \
    FUNCX(propScene_saveCalculationStructs, "saveCalculationStructs", sim_propertytype_bool, 0, R"({"label": "Save calculation structures", "description": "Save operation also saves existing calculation structures"})", "") \
    FUNCX(propScene_sceneUid, "sceneUid", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Scene UID", "description": "Scene unique identifier"})", "") \
    FUNCX(propScene_visibilityLayers, "visibilityLayers", sim_propertytype_int, 0, R"({"label": "Visibility layers", "description": "Currently active visibility layers"})", "") \
    FUNCX(propScene_scenePath, "scenePath", sim_propertytype_string, sim_propertyinfo_modelhashexclude, R"({"label": "Scene path", "description": ""})", "") \
    FUNCX(propScene_acknowledgment, "acknowledgment", sim_propertytype_string, 0, R"({"label": "Acknowledgment", "description": "Scene acknowledgment"})", "") \
    FUNCX(propScene_sceneUidString, "sceneUidString", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable, R"({"label": "Scene UID string", "description": "Scene unique identifier string"})", "") \
    FUNCX(propScene_ambientLight, "ambientLight", sim_propertytype_color, 0, R"({"label": "Ambient light", "description": ""})", "")

#define SIMULATION_PROPERTIES \
    FUNCX(propSim_removeNewObjectsAtEnd, "removeNewObjectsAtEnd", sim_propertytype_bool, 0, R"({"label": "Remove new objects", "description": "Remove new scene objects at simulation end"})", "") \
    FUNCX(propSim_realtimeSimulation, "realtimeSimulation", sim_propertytype_bool, 0, R"({"label": "Real-time simulation", "description": ""})", "") \
    FUNCX(propSim_pauseSimulationAtTime, "pauseSimulationAtTime", sim_propertytype_bool, 0, R"({"label": "Pause simulation at time", "description": "Pause simulation when simulation time exceeds a threshold"})", "") \
    FUNCX(propSim_pauseSimulationAtError, "pauseSimulationAtError", sim_propertytype_bool, 0, R"({"label": "Pause simulation on script error", "description": ""})", "") \
    FUNCX(propSim_simulationTime, "simulationTime", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Simulation time", "description": ""})", "") \
    FUNCX(propSim_timeStep, "timeStep", sim_propertytype_float, 0, R"({"label": "Simulation dt", "description": "Simulation time step"})", "") \
    FUNCX(propSim_timeToPause, "timeToPause", sim_propertytype_float, 0, R"({"label": "Simulation pause time", "description": "Time at which simulation should pause"})", "") \
    FUNCX(propSim_realtimeModifier, "realtimeModifier", sim_propertytype_float, 0, R"({"label": "Real-time modifier", "description": "Real-time multiplication factor"})", "") \
    FUNCX(propSim_stepCount, "stepCount", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Simulation steps", "description": "Counter of simulation steps"})", "") \
    FUNCX(propSim_simulationState, "simulationState", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Simulation state", "description": ""})", "") \
    FUNCX(propSim_stepsPerRendering, "stepsPerRendering", sim_propertytype_int, 0, R"({"label": "Steps per frame", "description": "Simulation steps per frame"})", "") \
    FUNCX(propSim_speedModifier, "speedModifier", sim_propertytype_int, sim_propertyinfo_modelhashexclude, R"({"label": "Speed modifier", "description": ""})", "")

#define MESHWRAPPER_PROPERTIES \
    FUNCX(propMeshWrap_mass, "mass", sim_propertytype_float, 0, R"===({"label": "Mass", "description": ""})===", "") \
    FUNCX(propMeshWrap_com, "centerOfMass", sim_propertytype_vector3, 0, R"===({"label": "Center of mass", "description": "Center of mass, relative to the shape's reference frame"})===", "") \
    FUNCX(propMeshWrap_inertia, "inertia", sim_propertytype_floatarray, 0, R"===({"label": "Inertia tensor", "description": "Inertia tensor, relative to the shape's reference frame"})===", "") \
    FUNCX(propMeshWrap_pmi, "principalMomentOfInertia", sim_propertytype_floatarray, sim_propertyinfo_notwritable, R"===({"label": "Principal moment of inertia", "description": "Principal moment of inertia, relative to pmiQuaternion"})===", "") \
    FUNCX(propMeshWrap_pmiQuaternion, "pmiQuaternion", sim_propertytype_quaternion, sim_propertyinfo_notwritable, R"===({"label": "Quaternion of the principal moment of inertia", "description": "Quaternion of the principal moment of inertia, relative to the shape's reference frame"})===", "")

#define MESH_PROPERTIES \
    FUNCX(propMesh_textureResolution, "textureResolution", sim_propertytype_intarray2, sim_propertyinfo_notwritable, R"===({"label": "Texture resolution", "description": ""})===", "") \
    FUNCX(propMesh_textureCoordinates, "textureCoordinates", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Texture coordinates", "description": ""})===", "") \
    FUNCX(propMesh_textureApplyMode, "textureApplyMode", sim_propertytype_int, 0, R"===({"label": "Texture apply mode", "description": ""})===", "") \
    FUNCX(propMesh_textureRepeatU, "textureRepeatU", sim_propertytype_bool, 0, R"===({"label": "Texture repeat U", "description": ""})===", "") \
    FUNCX(propMesh_textureRepeatV, "textureRepeatV", sim_propertytype_bool, 0, R"===({"label": "Texture repeat V", "description": ""})===", "") \
    FUNCX(propMesh_textureInterpolate, "textureInterpolate", sim_propertytype_bool, 0, R"===({"label": "Interpolate texture", "description": ""})===", "") \
    FUNCX(propMesh_texture, "rawTexture", sim_propertytype_buffer, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Texture", "description": ""})===", "") \
    FUNCX(propMesh_textureID, "textureID", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Texture ID", "description": ""})===", "") \
    FUNCX(propMesh_vertices, "vertices", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Vertices", "description": ""})===", "") \
    FUNCX(propMesh_indices, "indices", sim_propertytype_intarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Indices", "description": "Indices (3 values per triangle)"})===", "") \
    FUNCX(propMesh_normals, "normals", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Normals", "description": "Normals (3*3 values per triangle)"})===", "") \
    FUNCX(propMesh_shadingAngle, "shadingAngle", sim_propertytype_float, 0, R"===({"label": "Shading angle", "description": ""})===", "") \
    FUNCX(propMesh_showEdges, "showEdges", sim_propertytype_bool, 0, R"===({"label": "Visible edges", "description": ""})===", "") \
    FUNCX(propMesh_culling, "culling", sim_propertytype_bool, 0, R"===({"label": "Backface culling", "description": ""})===", "") \
    FUNCX(propMesh_objectType, "objectType", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Object type", "description": ""})===", "") \
    FUNCX(propMesh_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Object meta information", "description": ""})===", "") \
    FUNCX(propMesh_handle, "handle", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Handle", "description": "", "handleType": ""})===", "") \
    FUNCX(propMesh_shapeUid, "shapeUid", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Shape UID", "description": "Unique identifier of parent shape"})===", "") \
    FUNCX(propMesh_shape, "shape", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Shape handle", "description": "", "handleType": "shape"})===", "") \
    FUNCX(propMesh_primitiveType, "primitiveType", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Primitive type", "description": ""})===", "") \
    FUNCX(propMesh_convex, "convex", sim_propertytype_bool, sim_propertyinfo_notwritable, R"===({"label": "Convex", "description": "Whether mesh is convex or not"})===", "") \
    FUNCX(propMesh_colorName, "colorName", sim_propertytype_string, 0, R"===({"label": "Color name", "description": ""})===", "")

#define VIEWABLEBASE_PROPERTIES \
    FUNCX(propViewable_viewAngle, "viewAngle", sim_propertytype_float, 0, R"===({"label": "View angle", "description": "View angle (in perspective projection mode)"})===", "") \
    FUNCX(propViewable_viewSize, "viewSize", sim_propertytype_float, 0, R"===({"label": "View size", "description": "View size (in orthogonal projection mode)"})===", "") \
    FUNCX(propViewable_clippingPlanes, "clippingPlanes", sim_propertytype_floatarray, 0, R"===({"label": "Clipping planes", "description": "Near and far clipping planes"})===", "") \
    FUNCX(propViewable_perspective, "perspective", sim_propertytype_bool, sim_propertyinfo_constant | sim_propertyinfo_notwritable, R"===({"label": "Perspective", "description": "Perspective projection mode, otherwise orthogonal projection mode"})===", "") \
    FUNCX(propViewable_showFrustum, "showFrustum", sim_propertytype_bool, 0, R"===({"label": "Show view frustum", "description": ""})===", "") \
    FUNCX(propViewable_frustumCornerNear, "frustumCornerNear", sim_propertytype_vector3, sim_propertyinfo_notwritable, R"===({"label": "Near corner of View frustum", "description": ""})===", "") \
    FUNCX(propViewable_frustumCornerFar, "frustumCornerFar", sim_propertytype_vector3, sim_propertyinfo_notwritable, R"===({"label": "Far corner of view frustum", "description": ""})===", "") \
    FUNCX(propViewable_resolution, "resolution", sim_propertytype_intarray2, 0, R"===({"label": "Resolution", "description": "Resolution (relevant only with vision sensors)"})===", "")

#define SCENEOBJECTCONT_PROPERTIES \
    FUNCX(propObjCont_objectHandles, "objectHandles", sim_propertytype_intarray, sim_propertyinfo_deprecated | sim_propertyinfo_notwritable, R"({"label": "", "description": ""})", "") \
    FUNCX(propObjCont_orphanHandles, "orphanHandles", sim_propertytype_intarray, sim_propertyinfo_deprecated | sim_propertyinfo_notwritable, R"({"label": "", "description": ""})", "") \
    FUNCX(propObjCont_selectionHandles, "selectionHandles", sim_propertytype_intarray, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude, R"({"label": "", "description": ""})", "") \
    FUNCX(propObjCont_objects, "objects", sim_propertytype_handlearray, sim_propertyinfo_notwritable, R"({"label": "Objects", "description": "Handles of all scene objects", "handleType": "sceneObject"})", "") \
    FUNCX(propObjCont_orphans, "orphans", sim_propertytype_handlearray, sim_propertyinfo_notwritable, R"({"label": "Orphan objects", "description": "Handles of all orphan scene objects", "handleType": "sceneObject"})", "") \
    FUNCX(propObjCont_selection, "selection", sim_propertytype_handlearray, sim_propertyinfo_modelhashexclude, R"({"label": "Selected objects", "description": "Handles of selected scene objects", "handleType": "sceneObject"})", "") \
    FUNCX(propObjCont_objectCreationCounter, "objectCreationCounter", sim_propertytype_int, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Object creation counter", "description": "Counter of created scene objects"})", "") \
    FUNCX(propObjCont_objectDestructionCounter, "objectDestructionCounter", sim_propertytype_int, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Object destruction counter", "description": "Counter of destroyed scene objects"})", "") \
    FUNCX(propObjCont_hierarchyChangeCounter, "hierarchyChangeCounter", sim_propertytype_int, sim_propertyinfo_silent | sim_propertyinfo_notwritable, R"({"label": "Hierarchy change counter", "description": "Counter of scene hierarchy changes"})", "")

#define DYNMATERIAL_PROPERTIES \
    FUNCX(propMaterial_engineProperties, "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"===({"label": "Engine properties", "description": "Engine properties as JSON text"})===", "") \
    FUNCX(propMaterial_bulletRestitution, "bullet.restitution", sim_propertytype_float, 0, sim_bullet_body_restitution, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_bulletFriction0, "bullet.frictionOld", sim_propertytype_float, 0, sim_bullet_body_oldfriction, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_bulletFriction, "bullet.friction", sim_propertytype_float, 0, sim_bullet_body_friction, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_bulletLinearDamping, "bullet.linearDamping", sim_propertytype_float, 0, sim_bullet_body_lineardamping, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_bulletAngularDamping, "bullet.angularDamping", sim_propertytype_float, 0, sim_bullet_body_angulardamping, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_bulletNonDefaultCollisionMarginFactor, "bullet.customCollisionMarginValue", sim_propertytype_float, 0, sim_bullet_body_nondefaultcollisionmargingfactor, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_bulletNonDefaultCollisionMarginFactorConvex, "bullet.customCollisionMarginConvexValue", sim_propertytype_float, 0, sim_bullet_body_nondefaultcollisionmargingfactorconvex, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_bulletSticky, "bullet.stickyContact", sim_propertytype_bool, 0, sim_bullet_body_sticky, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_bulletNonDefaultCollisionMargin, "bullet.customCollisionMarginEnabled", sim_propertytype_bool, 0, sim_bullet_body_usenondefaultcollisionmargin, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_bulletNonDefaultCollisionMarginConvex, "bullet.customCollisionMarginConvexEnabled", sim_propertytype_bool, 0, sim_bullet_body_usenondefaultcollisionmarginconvex, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_bulletAutoShrinkConvex, "bullet.autoShrinkConvexMeshes", sim_propertytype_bool, 0, sim_bullet_body_autoshrinkconvex, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_odeFriction, "ode.friction", sim_propertytype_float, 0, sim_ode_body_friction, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_odeSoftErp, "ode.softErp", sim_propertytype_float, 0, sim_ode_body_softerp, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_odeSoftCfm, "ode.softCfm", sim_propertytype_float, 0, sim_ode_body_softcfm, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_odeLinearDamping, "ode.linearDamping", sim_propertytype_float, 0, sim_ode_body_lineardamping, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_odeAngularDamping, "ode.angularDamping", sim_propertytype_float, 0, sim_ode_body_angulardamping, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_odeMaxContacts, "ode.maxContacts", sim_propertytype_int, 0, sim_ode_body_maxcontacts, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexPrimaryLinearAxisFriction, "vortex.linearPrimaryAxisFrictionValue", sim_propertytype_float, 0, sim_vortex_body_primlinearaxisfriction, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisFriction, "vortex.linearSecondaryAxisFrictionValue", sim_propertytype_float, 0, sim_vortex_body_seclinearaxisfriction, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexPrimaryAngularAxisFriction, "vortex.angularPrimaryAxisFrictionValue", sim_propertytype_float, 0, sim_vortex_body_primangularaxisfriction, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisFriction, "vortex.angularSecondaryAxisFrictionValue", sim_propertytype_float, 0, sim_vortex_body_secangularaxisfriction, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexNormalAngularAxisFriction, "vortex.angularNormalAxisFrictionValue", sim_propertytype_float, 0, sim_vortex_body_normalangularaxisfriction, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexPrimaryLinearAxisStaticFrictionScale, "vortex.linearPrimaryAxisStaticFrictionScale", sim_propertytype_float, 0, sim_vortex_body_primlinearaxisstaticfrictionscale, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisStaticFrictionScale, "vortex.linearSecondaryAxisStaticFrictionScale", sim_propertytype_float, 0, sim_vortex_body_seclinearaxisstaticfrictionscale, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexPrimaryAngularAxisStaticFrictionScale, "vortex.angularPrimaryAxisStaticFrictionScale", sim_propertytype_float, 0, sim_vortex_body_primangularaxisstaticfrictionscale, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisStaticFrictionScale, "vortex.angularSecondaryAxisStaticFrictionScale", sim_propertytype_float, 0, sim_vortex_body_secangularaxisstaticfrictionscale, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexNormalAngularAxisStaticFrictionScale, "vortex.angularNormalAxisStaticFrictionScale", sim_propertytype_float, 0, sim_vortex_body_normalangularaxisstaticfrictionscale, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexCompliance, "vortex.compliance", sim_propertytype_float, 0, sim_vortex_body_compliance, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexDamping, "vortex.damping", sim_propertytype_float, 0, sim_vortex_body_damping, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexRestitution, "vortex.restitution", sim_propertytype_float, 0, sim_vortex_body_restitution, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexRestitutionThreshold, "vortex.restitutionThreshold", sim_propertytype_float, 0, sim_vortex_body_restitutionthreshold, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexAdhesiveForce, "vortex.adhesiveForce", sim_propertytype_float, 0, sim_vortex_body_adhesiveforce, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexLinearVelocityDamping, "vortex.linearVelDamping", sim_propertytype_float, 0, sim_vortex_body_linearvelocitydamping, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexAngularVelocityDamping, "vortex.angularVelDamping", sim_propertytype_float, 0, sim_vortex_body_angularvelocitydamping, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexPrimaryLinearAxisSlide, "vortex.linearPrimaryAxisSlide", sim_propertytype_float, 0, sim_vortex_body_primlinearaxisslide, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisSlide, "vortex.linearSecondaryAxisSlide", sim_propertytype_float, 0, sim_vortex_body_seclinearaxisslide, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexPrimaryAngularAxisSlide, "vortex.angularPrimaryAxisSlide", sim_propertytype_float, 0, sim_vortex_body_primangularaxisslide, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisSlide, "vortex.angularSecondaryAxisSlide", sim_propertytype_float, 0, sim_vortex_body_secangularaxisslide, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexNormalAngularAxisSlide, "vortex.angularNormalAxisSlide", sim_propertytype_float, 0, sim_vortex_body_normalangularaxisslide, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexPrimaryLinearAxisSlip, "vortex.linearPrimaryAxisSlip", sim_propertytype_float, 0, sim_vortex_body_primlinearaxisslip, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisSlip, "vortex.linearSecondaryAxisSlip", sim_propertytype_float, 0, sim_vortex_body_seclinearaxisslip, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexPrimaryAngularAxisSlip, "vortex.angularPrimaryAxisSlip", sim_propertytype_float, 0, sim_vortex_body_primangularaxisslip, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisSlip, "vortex.angularSecondaryAxisSlip", sim_propertytype_float, 0, sim_vortex_body_secangularaxisslip, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexNormalAngularAxisSlip, "vortex.angularNormalAxisSlip", sim_propertytype_float, 0, sim_vortex_body_normalangularaxisslip, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexAutoSleepLinearSpeedThreshold, "vortex.autoSleepThresholdLinearSpeed", sim_propertytype_float, 0, sim_vortex_body_autosleeplinearspeedthreshold, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexAutoSleepLinearAccelerationThreshold, "vortex.autoSleepThresholdLinearAccel", sim_propertytype_float, 0, sim_vortex_body_autosleeplinearaccelthreshold, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexAutoSleepAngularSpeedThreshold, "vortex.autoSleepThresholdAngularSpeed", sim_propertytype_float, 0, sim_vortex_body_autosleepangularspeedthreshold, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexAutoSleepAngularAccelerationThreshold, "vortex.autoSleepThresholdAngularAccel", sim_propertytype_float, 0, sim_vortex_body_autosleepangularaccelthreshold, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexSkinThickness, "vortex.skinThickness", sim_propertytype_float, 0, sim_vortex_body_skinthickness, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexAutoAngularDampingTensionRatio, "vortex.autoAngularDampingTensionRatio", sim_propertytype_float, 0, sim_vortex_body_autoangulardampingtensionratio, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexPrimaryAxisVector, "vortex.linearPrimaryValue", sim_propertytype_vector3, 0, sim_vortex_body_primaxisvectorx, sim_vortex_body_primaxisvectory, sim_vortex_body_primaxisvectorz, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexPrimaryLinearAxisFrictionModel, "vortex.linearPrimaryAxisFrictionModel", sim_propertytype_int, 0, sim_vortex_body_primlinearaxisfrictionmodel, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisFrictionModel, "vortex.linearSecondaryAxisFrictionModel", sim_propertytype_int, 0, sim_vortex_body_seclinearaxisfrictionmodel, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexPrimaryAngularAxisFrictionModel, "vortex.angularPrimaryAxisFrictionModel", sim_propertytype_int, 0, sim_vortex_body_primangulararaxisfrictionmodel, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisFrictionModel, "vortex.angularSecondaryAxisFrictionModel", sim_propertytype_int, 0, sim_vortex_body_secangularaxisfrictionmodel, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexNormalAngularAxisFrictionModel, "vortex.angularNormalAxisFrictionModel", sim_propertytype_int, 0, sim_vortex_body_normalangularaxisfrictionmodel, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexAutoSleepStepLiveThreshold, "vortex.autoSleepThresholdSteps", sim_propertytype_int, 0, sim_vortex_body_autosleepsteplivethreshold, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexMaterialUniqueId, "vortex.materialUniqueId", sim_propertytype_int, sim_propertyinfo_modelhashexclude, sim_vortex_body_materialuniqueid, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexPrimitiveShapesAsConvex, "vortex.primitiveAsConvex", sim_propertytype_bool, 0, sim_vortex_body_pureshapesasconvex, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexConvexShapesAsRandom, "vortex.convexAsRandom", sim_propertytype_bool, 0, sim_vortex_body_convexshapesasrandom, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexRandomShapesAsTerrain, "vortex.randomAsTerrain", sim_propertytype_bool, 0, sim_vortex_body_randomshapesasterrain, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexFastMoving, "vortex.fastMoving", sim_propertytype_bool, 0, sim_vortex_body_fastmoving, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexAutoSlip, "vortex.autoSlip", sim_propertytype_bool, 0, sim_vortex_body_autoslip, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexSecondaryLinearAxisSameAsPrimaryLinearAxis, "vortex.linearSecondaryAxisFollowPrimaryAxis", sim_propertytype_bool, 0, sim_vortex_body_seclinaxissameasprimlinaxis, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexSecondaryAngularAxisSameAsPrimaryAngularAxis, "vortex.angularSecondaryAxisFollowPrimaryAxis", sim_propertytype_bool, 0, sim_vortex_body_secangaxissameasprimangaxis, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexNormalAngularAxisSameAsPrimaryAngularAxis, "vortex.angularNormalAxisFollowPrimaryAxis", sim_propertytype_bool, 0, sim_vortex_body_normangaxissameasprimangaxis, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_vortexAutoAngularDamping, "vortex.autoAngularDampingEnabled", sim_propertytype_bool, 0, sim_vortex_body_autoangulardamping, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_newtonStaticFriction, "newton.staticFriction", sim_propertytype_float, 0, sim_newton_body_staticfriction, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_newtonKineticFriction, "newton.kineticFriction", sim_propertytype_float, 0, sim_newton_body_kineticfriction, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_newtonRestitution, "newton.restitution", sim_propertytype_float, 0, sim_newton_body_restitution, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_newtonLinearDrag, "newton.linearDrag", sim_propertytype_float, 0, sim_newton_body_lineardrag, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_newtonAngularDrag, "newton.angularDrag", sim_propertytype_float, 0, sim_newton_body_angulardrag, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_newtonFastMoving, "newton.fastMoving", sim_propertytype_bool, 0, sim_newton_body_fastmoving, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_mujocoFriction, "mujoco.friction", sim_propertytype_floatarray, 0, sim_mujoco_body_friction1, sim_mujoco_body_friction2, sim_mujoco_body_friction3, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_mujocoSolref, "mujoco.solref", sim_propertytype_floatarray, 0, sim_mujoco_body_solref1, sim_mujoco_body_solref2, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_mujocoSolimp, "mujoco.solimp", sim_propertytype_floatarray, 0, sim_mujoco_body_solimp1, sim_mujoco_body_solimp2, sim_mujoco_body_solimp3, sim_mujoco_body_solimp4, sim_mujoco_body_solimp5, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_mujocoSolmix, "mujoco.solmix", sim_propertytype_float, 0, sim_mujoco_body_solmix, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_mujocoMargin, "mujoco.margin", sim_propertytype_float, 0, sim_mujoco_body_margin, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_mujocoGap, "mujoco.gap", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_mujocoCondim, "mujoco.condim", sim_propertytype_int, 0, sim_mujoco_body_condim, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_mujocoPriority, "mujoco.priority", sim_propertytype_int, 0, sim_mujoco_body_priority, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_mujocoAdhesion, "mujoco.adhesion", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_mujocoAdhesionGain, "mujoco.adhesiongain", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_mujocoAdhesionForcelimited, "mujoco.adhesionforcelimited", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_mujocoAdhesionCtrlrange, "mujoco.adhesionctrlrange", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_mujocoAdhesionForcerange, "mujoco.adhesionforcerange", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_mujocoAdhesionCtrl, "mujoco.adhesionctrl", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propMaterial_mujocoGravcomp, "mujoco.gravcomp", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "")

#define SCENEOBJECT_PROPERTIES_1 \
    FUNCX(propObject_modelInvisible, "modelInvisible", sim_propertytype_bool, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Inherited model invisibility", "description": ""})===", "") \
    FUNCX(propObject_modelBase, "modelBase", sim_propertytype_bool, 0, R"===({"label": "Model base", "description": "Model base flag, indicates the scene object is the base of a model"})===", "") \
    FUNCX(propObject_layer, "layer", sim_propertytype_int, 0, R"===({"label": "Visibility layer", "description": ""})===", "") \
    FUNCX(propObject_childOrder, "childOrder", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Child order", "description": ""})===", "") \
    FUNCX(propObject_parentUid, "parentUid", sim_propertytype_long, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Parent UID", "description": "Parent scene object unique identifier"})===", "") \
    FUNCX(propObject_objectUid, "objectUid", sim_propertytype_long, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Object UID", "description": "Scene object unique identifier"})===", "") \
    FUNCX(propObject_parentHandle, "parentHandle", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propObject_parent, "parent", sim_propertytype_handle, sim_propertyinfo_modelhashexclude, R"===({"label": "Parent", "description": "Parent scene object handle", "handleType": "sceneObject"})===", "") \
    FUNCX(propObject_selected, "selected", sim_propertytype_bool, sim_propertyinfo_modelhashexclude, R"===({"label": "Selected", "description": "Selection state"})===", "") \
    FUNCX(propObject_hierarchyColor, "hierarchyColor", sim_propertytype_int, 0, R"===({"label": "Hierarchy color", "description": "Hierarchy color index"})===", "") \
    FUNCX(propObject_collectionSelfCollInd, "collectionSelfCollisionIndicator", sim_propertytype_int, 0, R"===({"label": "Collection self collision indicator", "description": ""})===", "") \
    FUNCX(propObject_collidable, "collidable", sim_propertytype_bool, 0, R"===({"label": "Collidable", "description": ""})===", "") \
    FUNCX(propObject_measurable, "measurable", sim_propertytype_bool, 0, R"===({"label": "Measurable", "description": ""})===", "") \
    FUNCX(propObject_detectable, "detectable", sim_propertytype_bool, 0, R"===({"label": "Detectable", "description": ""})===", "") \
    FUNCX(propObject_modelAcknowledgment, "modelAcknowledgment", sim_propertytype_string, 0, R"===({"label": "Acknowledgment", "description": "Model acknowledgment"})===", "") \
    FUNCX(propObject_dna, "dna", sim_propertytype_buffer, sim_propertyinfo_notwritable, R"===({"label": "DNA", "description": "Scene object DNA"})===", "") \
    FUNCX(propObject_persistentUid, "persistentUid", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Persistent UID", "description": "Scene object persistent unique identifier"})===", "") \
    FUNCX(propObject_calcLinearVelocity, "calcLinearVelocity", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Linear velocity", "description": "Calculated scene object linear velocity vector"})===", "") \
    FUNCX(propObject_calcRotationAxis, "calcRotationAxis", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Rotation axis", "description": "Calculated scene object rotation axis"})===", "") \
    FUNCX(propObject_calcRotationVelocity, "calcRotationVelocity", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Rotation velocity", "description": "Calculated scene object rotation velocity"})===", "") \
    FUNCX(propObject_dynamicIcon, "dynamicIcon", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Dynamic icon", "description": ""})===", "") \
    FUNCX(propObject_dynamicFlag, "dynamicFlag", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Dynamic flag", "description": ""})===", "") \
    FUNCX(propObject_objectProperty, "objectPropertyFlags", /*redund.*/ sim_propertytype_int, sim_propertyinfo_modelhashexclude, R"===({"label": "Object flags", "description": "Scene object flags, redundant"})===", "") \
    FUNCX(propObject_ignoreViewFitting, "ignoreViewFitting", sim_propertytype_bool, 0, R"===({"label": "Ignore view fitting", "description": "View fitting is ignored for this scene object"})===", "") \
    FUNCX(propObject_collapsed, "collapsed", sim_propertytype_bool, 0, R"===({"label": "Collapsed hierarchy", "description": ""})===", "") \
    FUNCX(propObject_selectable, "selectable", sim_propertytype_bool, 0, R"===({"label": "Selectable", "description": "Scene object is selectable"})===", "") \
    FUNCX(propObject_selectModel, "selectModel", sim_propertytype_bool, 0, R"===({"label": "Select model instead", "description": "Selecting the scene object instead selects the parent model"})===", "") \
    FUNCX(propObject_hideFromModelBB, "hideFromModelBB", sim_propertytype_bool, 0, R"===({"label": "Hide from model bounding box", "description": "Scene object is not part of a parent model's bounding box"})===", "") \
    FUNCX(propObject_selectInvisible, "selectInvisible", sim_propertytype_bool, 0, R"===({"label": "Select invisible", "description": "Scene object is invisible during a selection operation"})===", "") \
    FUNCX(propObject_depthInvisible, "depthInvisible", sim_propertytype_bool, 0, R"===({"label": "Depth pass invisible", "description": "Scene object is invisible for the depth buffer during a click operation"})===", "") \
    FUNCX(propObject_cannotDelete, "cannotDelete", sim_propertytype_bool, 0, R"===({"label": "Cannot delete", "description": "Scene object cannot be deleted while simulation is not running"})===", "") \
    FUNCX(propObject_cannotDeleteSim, "cannotDeleteInSim", sim_propertytype_bool, 0, R"===({"label": "Cannot delete during simulation", "description": "Scene object cannot be deleted while simulation is running"})===", "")

#define SCENEOBJECT_PROPERTIES_2 \
    FUNCX(propObject_modelPropertyDEPRECATED, "modelPropertyFlags", /*redund.*/ sim_propertytype_int, sim_propertyinfo_modelhashexclude | sim_propertyinfo_deprecated, R"===({"label": "Model flags", "description": "Model flags, redundant"})===", "") \
    FUNCX(propObject_modelNotCollidableDEPRECATED, "modelNotCollidable", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Model not collidable", "description": "Model is not collidable"})===", "") \
    FUNCX(propObject_modelNotMeasurableDEPRECATED, "modelNotMeasurable", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Model not measurable", "description": "Model is not measurable"})===", "") \
    FUNCX(propObject_modelNotDetectableDEPRECATED, "modelNotDetectable", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Model not detectable", "description": "Model is not detectable"})===", "") \
    FUNCX(propObject_modelNotDynamicDEPRECATED, "modelNotDynamic", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Model not dynamic", "description": "Model is not dynamic, i.e. model is static"})===", "") \
    FUNCX(propObject_modelNotRespondableDEPRECATED, "modelNotRespondable", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Model not respondable", "description": "Model is not respondable"})===", "") \
    FUNCX(propObject_modelNotVisibleDEPRECATED, "modelNotVisible", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Model not visible", "description": "Model is not visible"})===", "") \
    FUNCX(propObject_modelScriptsNotActiveDEPRECATED, "modelScriptsNotActive", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Model scripts inactive", "description": "Model scripts are not active"})===", "") \
    FUNCX(propObject_modelNotInParentBBDEPRECATED, "modelNotInParentBB", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Model invisible to other model's bounding boxes", "description": "Model is invisible to other model's bounding boxes"})===", "") \
    FUNCX(propObject_modelProperty, "model.propertyFlags", /*redund.*/ sim_propertytype_int, sim_propertyinfo_modelhashexclude, R"===({"label": "Model flags", "description": "Model flags, redundant"})===", "") \
    FUNCX(propObject_modelNotCollidable, "model.notCollidable", sim_propertytype_bool, 0, R"===({"label": "Model not collidable", "description": "Model is not collidable"})===", "") \
    FUNCX(propObject_modelNotMeasurable, "model.notMeasurable", sim_propertytype_bool, 0, R"===({"label": "Model not measurable", "description": "Model is not measurable"})===", "") \
    FUNCX(propObject_modelNotDetectable, "model.notDetectable", sim_propertytype_bool, 0, R"===({"label": "Model not detectable", "description": "Model is not detectable"})===", "") \
    FUNCX(propObject_modelNotDynamic, "model.notDynamic", sim_propertytype_bool, 0, R"===({"label": "Model not dynamic", "description": "Model is not dynamic, i.e. model is static"})===", "") \
    FUNCX(propObject_modelNotRespondable, "model.notRespondable", sim_propertytype_bool, 0, R"===({"label": "Model not respondable", "description": "Model is not respondable"})===", "") \
    FUNCX(propObject_modelNotVisible, "model.notVisible", sim_propertytype_bool, 0, R"===({"label": "Model not visible", "description": "Model is not visible"})===", "") \
    FUNCX(propObject_modelScriptsNotActive, "model.scriptsNotActive", sim_propertytype_bool, 0, R"===({"label": "Model scripts inactive", "description": "Model scripts are not active"})===", "") \
    FUNCX(propObject_modelNotInParentBB, "model.notInParentBB", sim_propertytype_bool, 0, R"===({"label": "Model invisible to other model's bounding boxes", "description": "Model is invisible to other model's bounding boxes"})===", "") \
    FUNCX(propObject_modelBBSize, "model.bbSize", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Model bounding boxe size", "description": ""})===", "") \
    FUNCX(propObject_modelBBPos, "model.bbPos", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Model bounding boxe position", "description": ""})===", "") \
    FUNCX(propObject_pose, "pose", sim_propertytype_pose, 0, R"===({"label": "Pose", "description": "Scene object local pose"})===", "") \
    FUNCX(propObject_position, "position", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, R"===({"label": "Position", "description": "Scene object local position"})===", "") \
    FUNCX(propObject_quaternion, "quaternion", sim_propertytype_quaternion, sim_propertyinfo_modelhashexclude, R"===({"label": "Quaternion", "description": "Scene object local quaternion"})===", "") \
    FUNCX(propObject_eulerAngles, "eulerAngles", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, R"===({"label": "Euler angles", "description": "Scene object local Euler angles"})===", "") \
    FUNCX(propObject_absPose, "absPose", sim_propertytype_pose, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, R"===({"label": "Pose", "description": "Scene object absolute pose"})===", "") \
    FUNCX(propObject_absPosition, "absPosition", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, R"===({"label": "Position", "description": "Scene object absolute position"})===", "") \
    FUNCX(propObject_absQuaternion, "absQuaternion", sim_propertytype_quaternion, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, R"===({"label": "Quaternion", "description": "Scene object absolute quaternion"})===", "") \
    FUNCX(propObject_absEulerAngles, "absEulerAngles", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, R"===({"label": "Euler angles", "description": "Scene object absolute Euler angles"})===", "") \
    FUNCX(propObject_alias, "alias", sim_propertytype_string, 0, R"===({"label": "Alias", "description": "Scene object alias"})===", "") \
    FUNCX(propObject_deprecatedName, "deprecatedName", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude, R"===({"label": "Deprecated name", "description": ""})===", "") \
    FUNCX(propObject_bbPose, "bbPose", sim_propertytype_pose, sim_propertyinfo_notwritable, R"===({"label": "Bounding box pose", "description": "Bounding box local pose"})===", "") \
    FUNCX(propObject_bbHsize, "bbHSize", sim_propertytype_vector3, sim_propertyinfo_notwritable, R"===({"label": "Bounding box half sizes", "description": ""})===", "") \
    FUNCX(propObject_movementOptionsDEPRECATED, "movementOptionsFlags", sim_propertytype_int, sim_propertyinfo_deprecated, R"===({"label": "Movement flags", "description": "Scene object movement flags"})===", "") \
    FUNCX(propObject_movementStepSizeDEPRECATED, "movementStepSize", sim_propertytype_floatarray, sim_propertyinfo_deprecated, R"===({"label": "Movement step sizes", "description": "Linear and angular step sizes"})===", "") \
    FUNCX(propObject_movementRelativityDEPRECATED, "movementRelativity", sim_propertytype_intarray, sim_propertyinfo_deprecated, R"===({"label": "Movement relativity", "description": "Movement relativity, linear and angular"})===", "") \
    FUNCX(propObject_movementPreferredAxesDEPRECATED, "movementPreferredAxesFlags", sim_propertytype_int, sim_propertyinfo_deprecated, R"===({"label": "Preferred axes", "description": "Preferred axes flags (redundant)"})===", "") \
    FUNCX(propObject_movementOptions, "mov.optionsFlags", sim_propertytype_int, 0, R"===({"label": "Movement flags", "description": "Scene object movement flags"})===", "") \
    FUNCX(propObject_movementStepSize, "mov.stepSize", sim_propertytype_floatarray, 0, R"===({"label": "Movement step sizes", "description": "Linear and angular step sizes"})===", "") \
    FUNCX(propObject_movementRelativity, "mov.relativity", sim_propertytype_intarray, 0, R"===({"label": "Movement relativity", "description": "Movement relativity, linear and angular"})===", "") \
    FUNCX(propObject_movementPreferredAxes, "mov.preferredAxesFlags", sim_propertytype_int, 0, R"===({"label": "Preferred axes", "description": "Preferred axes flags (redundant)"})===", "") \
    FUNCX(propObject_movTranslNoSimDEPRECATED, "movTranslNoSim", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Translation enabled", "description": "Translation enabled when simulation is not running"})===", "") \
    FUNCX(propObject_movTranslInSimDEPRECATED, "movTranslInSim", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Translation enabled during simulation", "description": "Translation enabled when simulation is running"})===", "") \
    FUNCX(propObject_movRotNoSimDEPRECATED, "movRotNoSim", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Rotation enabled", "description": "Rotation enabled when simulation is not running"})===", "") \
    FUNCX(propObject_movRotInSimDEPRECATED, "movRotInSim", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Rotation enabled during simulation", "description": "Rotation enabled when simulation is running"})===", "") \
    FUNCX(propObject_movAltTranslDEPRECATED, "movAltTransl", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Alternate translation axes enabled", "description": ""})===", "") \
    FUNCX(propObject_movAltRotDEPRECATED, "movAltRot", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Alternate rotation axes enabled", "description": ""})===", "") \
    FUNCX(propObject_movPrefTranslXDEPRECATED, "movPrefTranslX", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Preferred X-translation", "description": ""})===", "") \
    FUNCX(propObject_movPrefTranslYDEPRECATED, "movPrefTranslY", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Preferred Y-translation", "description": ""})===", "") \
    FUNCX(propObject_movPrefTranslZDEPRECATED, "movPrefTranslZ", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Preferred Z-translation", "description": ""})===", "") \
    FUNCX(propObject_movPrefRotXDEPRECATED, "movPrefRotX", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Preferred X-axis rotation", "description": ""})===", "") \
    FUNCX(propObject_movPrefRotYDEPRECATED, "movPrefRotY", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Preferred Y-axis rotation", "description": ""})===", "") \
    FUNCX(propObject_movPrefRotZDEPRECATED, "movPrefRotZ", sim_propertytype_bool, sim_propertyinfo_deprecated, R"===({"label": "Preferred Z-axis rotation", "description": ""})===", "") \
    FUNCX(propObject_movTranslNoSim, "mov.translNoSim", sim_propertytype_bool, 0, R"===({"label": "Translation enabled", "description": "Translation enabled when simulation is not running"})===", "") \
    FUNCX(propObject_movTranslInSim, "mov.translInSim", sim_propertytype_bool, 0, R"===({"label": "Translation enabled during simulation", "description": "Translation enabled when simulation is running"})===", "") \
    FUNCX(propObject_movRotNoSim, "mov.rotNoSim", sim_propertytype_bool, 0, R"===({"label": "Rotation enabled", "description": "Rotation enabled when simulation is not running"})===", "") \
    FUNCX(propObject_movRotInSim, "mov.rotInSim", sim_propertytype_bool, 0, R"===({"label": "Rotation enabled during simulation", "description": "Rotation enabled when simulation is running"})===", "") \
    FUNCX(propObject_movAltTransl, "mov.altTransl", sim_propertytype_bool, 0, R"===({"label": "Alternate translation axes enabled", "description": ""})===", "") \
    FUNCX(propObject_movAltRot, "mov.altRot", sim_propertytype_bool, 0, R"===({"label": "Alternate rotation axes enabled", "description": ""})===", "") \
    FUNCX(propObject_movPrefTranslX, "mov.prefTranslX", sim_propertytype_bool, 0, R"===({"label": "Preferred X-translation", "description": ""})===", "") \
    FUNCX(propObject_movPrefTranslY, "mov.prefTranslY", sim_propertytype_bool, 0, R"===({"label": "Preferred Y-translation", "description": ""})===", "") \
    FUNCX(propObject_movPrefTranslZ, "mov.prefTranslZ", sim_propertytype_bool, 0, R"===({"label": "Preferred Z-translation", "description": ""})===", "") \
    FUNCX(propObject_movPrefRotX, "mov.prefRotX", sim_propertytype_bool, 0, R"===({"label": "Preferred X-axis rotation", "description": ""})===", "") \
    FUNCX(propObject_movPrefRotY, "mov.prefRotY", sim_propertytype_bool, 0, R"===({"label": "Preferred Y-axis rotation", "description": ""})===", "") \
    FUNCX(propObject_movPrefRotZ, "mov.prefRotZ", sim_propertytype_bool, 0, R"===({"label": "Preferred Z-axis rotation", "description": ""})===", "") \
    FUNCX(propObject_objectType, "objectType", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Object type", "description": "Scene object type"})===", "") \
    FUNCX(propObject_handle, "handle", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Handle", "description": "", "handleType": "sceneObject"})===", "") \
    FUNCX(propObject_visible, "visible", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Visible", "description": "Whether the scene object is currently visible"})===", "") \
    FUNCX(propObject_children, "children", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Children handles", "description": "", "handleType": "sceneObject"})===", "") \
    FUNCX(propObject_modelHash, "modelHash", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Model hash", "description": ""})===", "")

#define SCENEOBJECT_PROPERTIES \
    SCENEOBJECT_PROPERTIES_1 \
    SCENEOBJECT_PROPERTIES_2

#define SHAPE_PROPERTIES \
    FUNCX(propShape_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Object meta information", "description": ""})===", "") \
    FUNCX(propShape_meshes, "meshes", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Meshes", "description": "Mesh handles", "handleType": "mesh"})===", "") \
    FUNCX(propShape_applyCulling, "applyCulling", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude, R"===({"label": "Apply culling", "description": "Enables/disables culling for all contained meshes"})===", "") \
    FUNCX(propShape_applyShadingAngle, "applyShadingAngle", sim_propertytype_float, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude, R"===({"label": "Apply shading", "description": "Applies a shading angle to all contained meshes"})===", "") \
    FUNCX(propShape_applyShowEdges, "applyShowEdges", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude, R"===({"label": "Apply edges", "description": "Enables/disables edges for all contained meshes"})===", "") \
    FUNCX(propShape_flipFaces, "flipFaces", sim_propertytype_bool, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude, R"===({"label": "Flip faces", "description": "Flips faces of all contained meshes"})===", "") \
    FUNCX(propShape_applyColorDiffuse, "applyColor.diffuse", sim_propertytype_color, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude, R"===({"label": "Apply diffuse color", "description": "Applies the diffuse color component to all contained meshes"})===", "") \
    FUNCX(propShape_applyColorSpecular, "applyColor.specular", sim_propertytype_color, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude, R"===({"label": "Apply specular color", "description": "Applies the specular color component to all contained meshes"})===", "") \
    FUNCX(propShape_applyColorEmission, "applyColor.emission", sim_propertytype_color, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude, R"===({"label": "Apply emission color", "description": "Applies the emission color component to all contained meshes"})===", "") \
    FUNCX(propShape_applyColorTransparency, "applyColor.transparency", sim_propertytype_float, sim_propertyinfo_silent | sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude, R"===({"label": "Apply transparency", "description": "Applies transparency to all contained meshes"})===", "") \
    FUNCX(propShape_respondableMask, "respondableMask", sim_propertytype_int, 0, R"===({"label": "Respondable mask", "description": ""})===", "") \
    FUNCX(propShape_startInDynSleepMode, "startInDynSleepMode", sim_propertytype_bool, 0, R"===({"label": "Start in sleep mode", "description": ""})===", "") \
    FUNCX(propShape_dynamic, "dynamic", sim_propertytype_bool, 0, R"===({"label": "Dynamic", "description": "Shape is dynamic, i.e. not static"})===", "") \
    FUNCX(propShape_kinematic, "kinematic", sim_propertytype_bool, 0, R"===({"label": "Kinematic", "description": "Special flag mainly used for MuJoCo static shapes that move and need to transmit a friction"})===", "") \
    FUNCX(propShape_respondable, "respondable", sim_propertytype_bool, 0, R"===({"label": "Respondable", "description": "Shape will transmit a collision force"})===", "") \
    FUNCX(propShape_setToDynamicWithParent, "setToDynamicWithParent", sim_propertytype_bool, 0, R"===({"label": "Set to dynamic if gets parent", "description": "Shape will be made dynamic if it receives a parent"})===", "") \
    FUNCX(propShape_initLinearVelocity, "initLinearVelocity", sim_propertytype_vector3, 0, R"===({"label": "Initial linear velocity", "description": ""})===", "") \
    FUNCX(propShape_initAngularVelocity, "initAngularVelocity", sim_propertytype_vector3, 0, R"===({"label": "Initial rotational velocity", "description": ""})===", "") \
    FUNCX(propShape_dynLinearVelocity, "dynLinearVelocity", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Linear velocity", "description": "Linear velocity, as transmitted by the physics engine"})===", "") \
    FUNCX(propShape_dynAngularVelocity, "dynAngularVelocity", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Rotational velocity", "description": "Rotational velocity, as transmitted by the physics engine"})===", "") \
    FUNCX(propShape_convex, "convex", sim_propertytype_bool, sim_propertyinfo_notwritable, R"===({"label": "Convex", "description": "Whether the shape's components are all convex or not"})===", "") \
    FUNCX(propShape_primitive, "primitive", sim_propertytype_bool, sim_propertyinfo_notwritable, R"===({"label": "Primitive", "description": "Whether the shape's components are all primitives"})===", "") \
    FUNCX(propShape_compound, "compound", sim_propertytype_bool, sim_propertyinfo_notwritable, R"===({"label": "Compound", "description": "Whether the shape is a compound"})===", "")

#define CAMERA_PROPERTIES \
    FUNCX(propCamera_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Object meta information", "description": ""})", "") \
    FUNCX(propCamera_size, "cameraSize", sim_propertytype_float, 0, R"({"label": "Size", "description": "Camera size"})", "") \
    FUNCX(propCamera_parentAsManipProxy, "parentAsManipulationProxy", sim_propertytype_bool, 0, R"({"label": "Parent as proxy", "description": "Use parent as manipulation proxy"})", "") \
    FUNCX(propCamera_translationEnabled, "translationEnabled", sim_propertytype_bool, 0, R"({"label": "Translation enabled", "description": ""})", "") \
    FUNCX(propCamera_rotationEnabled, "rotationEnabled", sim_propertytype_bool, 0, R"({"label": "Rotation enabled", "description": ""})", "") \
    FUNCX(propCamera_trackedObjectHandle, "trackedObjectHandle", sim_propertytype_int, sim_propertyinfo_modelhashexclude, R"({"label": "Tracked object", "description": "Tracked scene object handle"})", "")

#define DUMMY_PROPERTIES \
    FUNCX(propDummy_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"({"label": "Object meta information", "description": ""})", "") \
    FUNCX(propDummy_size, "dummySize", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"({"label": "Size", "description": "Dummy size"})", "") \
    FUNCX(propDummy_linkedDummyHandle, "linkedDummyHandle", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDummy_linkedDummy, "linkedDummy", sim_propertytype_handle, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"({"label": "Linked dummy", "description": "Handle of the linked dummy", "handleType": "dummy"})", "") \
    FUNCX(propDummy_dummyType, "dummyType", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"({"label": "Type", "description": "Dummy type"})", "") \
    FUNCX(propDummy_assemblyTag, "assemblyTag", sim_propertytype_string, 0, -1, -1, -1, -1, -1, R"({"label": "Assembly tag", "description": ""})", "") \
    FUNCX(propDummy_engineProperties, "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"({"label": "Engine properties", "description": "Engine properties as JSON text"})", "") \
    FUNCX(propDummy_mujocoLimitsEnabled, "mujoco.limitsEnabled", sim_propertytype_bool, 0, sim_mujoco_dummy_limited, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDummy_mujocoLimitsRange, "mujoco.limitsRange", sim_propertytype_floatarray, 0, sim_mujoco_dummy_range1, sim_mujoco_dummy_range2, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDummy_mujocoLimitsSolref, "mujoco.limitsSolref", sim_propertytype_floatarray, 0, sim_mujoco_dummy_solreflimit1, sim_mujoco_dummy_solreflimit2, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDummy_mujocoLimitsSolimp, "mujoco.limitsSolimp", sim_propertytype_floatarray, 0, sim_mujoco_dummy_solimplimit1, sim_mujoco_dummy_solimplimit2, sim_mujoco_dummy_solimplimit3, sim_mujoco_dummy_solimplimit4, sim_mujoco_dummy_solimplimit5, R"({"label": "", "description": ""})", "") \
    FUNCX(propDummy_mujocoMargin, "mujoco.margin", sim_propertytype_float, 0, sim_mujoco_dummy_margin, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDummy_mujocoSpringStiffness, "mujoco.springStiffness", sim_propertytype_float, 0, sim_mujoco_dummy_stiffness, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDummy_mujocoSpringDamping, "mujoco.springDamping", sim_propertytype_float, 0, sim_mujoco_dummy_damping, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDummy_mujocoSpringLength, "mujoco.springLength", sim_propertytype_float, 0, sim_mujoco_dummy_springlength, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDummy_mujocoJointProxyHandle, "mujoco.jointProxyHandle", sim_propertytype_int, sim_propertyinfo_modelhashexclude, sim_mujoco_dummy_proxyjointid, -1, -1, -1, -1, R"({"label": "Joint proxy", "description": "Handle of the joint proxy, MuJoCo only"})", "") \
    FUNCX(propDummy_mujocoOverlapConstrSolref, "mujoco.overlapConstrSolref", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDummy_mujocoOverlapConstrSolimp, "mujoco.overlapConstrSolimp", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "") \
    FUNCX(propDummy_mujocoOverlapConstrTorqueScale, "mujoco.overlapConstrTorquescale", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")

#define FORCESENSOR_PROPERTIES \
    FUNCX(propFSensor_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Object meta information", "description": ""})", "") \
    FUNCX(propFSensor_size, "sensorSize", sim_propertytype_float, 0, R"({"label": "Size", "description": "Sensor size"})", "") \
    FUNCX(propFSensor_forceThresholdEnabled, "forceThresholdEnabled", sim_propertytype_bool, 0, R"({"label": "Force threshold enabled", "description": ""})", "") \
    FUNCX(propFSensor_torqueThresholdEnabled, "torqueThresholdEnabled", sim_propertytype_bool, 0, R"({"label": "Torque threshold enabled", "description": ""})", "") \
    FUNCX(propFSensor_filterType, "filterType", sim_propertytype_int, 0, R"({"label": "Filter", "description": "Filter type"})", "") \
    FUNCX(propFSensor_filterSampleSize, "filterSampleSize", sim_propertytype_int, 0, R"({"label": "Filter sample", "description": "Filter sample size"})", "") \
    FUNCX(propFSensor_consecutiveViolationsToTrigger, "consecutiveViolationsToTrigger", sim_propertytype_int, 0, R"({"label": "Consecutive violations to trigger", "description": ""})", "") \
    FUNCX(propFSensor_forceThreshold, "forceThreshold", sim_propertytype_float, 0, R"({"label": "Force threshold", "description": ""})", "") \
    FUNCX(propFSensor_torqueThreshold, "torqueThreshold", sim_propertytype_float, 0, R"({"label": "Torque threshold", "description": ""})", "") \
    FUNCX(propFSensor_sensorForce, "sensorForce", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Force", "description": "Measured force vector"})", "") \
    FUNCX(propFSensor_sensorTorque, "sensorTorque", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Torque", "description": "Measured torque vector"})", "") \
    FUNCX(propFSensor_sensorAverageForce, "filterSensorForce", sim_propertytype_vector3, sim_propertyinfo_deprecated | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "", "description": ""})", "") \
    FUNCX(propFSensor_sensorAverageTorque, "filterSensorTorque", sim_propertytype_vector3, sim_propertyinfo_deprecated | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "", "description": ""})", "") \
    FUNCX(propFSensor_filteredSensorForce, "filteredSensorForce", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Filtered force", "description": "Filtered force vector"})", "") \
    FUNCX(propFSensor_filteredSensorTorque, "filteredSensorTorque", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Filtered torque", "description": "Filtered torque vector"})", "") \
    FUNCX(propFSensor_intrinsicError, "intrinsicError", sim_propertytype_pose, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Intrinsic error", "description": "Intrinsic error, generated by some physics engines"})", "")

#define GRAPH_PROPERTIES \
    FUNCX(propGraph_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Object meta information", "description": ""})", "") \
    FUNCX(propGraph_size, "graphSize", sim_propertytype_float, 0, R"({"label": "Size", "description": "Size of the 3D graph representation"})", "") \
    FUNCX(propGraph_bufferSize, "bufferSize", sim_propertytype_int, 0, R"({"label": "Buffer size", "description": ""})", "") \
    FUNCX(propGraph_cyclic, "cyclic", sim_propertytype_bool, 0, R"({"label": "Cyclic", "description": "Buffer is cyclic"})", "") \
    FUNCX(propGraph_backgroundColor, "backgroundColor", sim_propertytype_color, 0, R"({"label": "Background color", "description": ""})", "") \
    FUNCX(propGraph_foregroundColor, "foregroundColor", sim_propertytype_color, 0, R"({"label": "Foreground color", "description": ""})", "")

#define JOINT_PROPERTIES \
    FUNCX(propJoint_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"===({"label": "Object meta information", "description": ""})===", "") \
    FUNCX(propJoint_length, "jointLength", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"===({"label": "Size", "description": "Joint size"})===", "") \
    FUNCX(propJoint_diameter, "jointDiameter", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"===({"label": "Size", "description": "Joint size"})===", "") \
    FUNCX(propJoint_position, "jointPosition", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"===({"label": "Position", "description": "Joint linear/angular displacement"})===", "") \
    FUNCX(propJoint_quaternion, "jointQuaternion", sim_propertytype_quaternion, 0, -1, -1, -1, -1, -1, R"===({"label": "Spherical joint quaternion", "description": ""})===", "") \
    FUNCX(propJoint_screwLead, "screwLead", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"===({"label": "Screw lead", "description": ""})===", "") \
    FUNCX(propJoint_intrinsicError, "intrinsicError", sim_propertytype_pose, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"===({"label": "Intrinsic error", "description": "Intrinsic error, generated by some physics engines"})===", "") \
    FUNCX(propJoint_intrinsicPose, "intrinsicPose", sim_propertytype_pose, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"===({"label": "Intrinsic pose", "description": "Intrinsic pose (includes a possible intrinsic error)"})===", "") \
    FUNCX(propJoint_calcVelocity, "calcVelocity", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"===({"label": "Velocity", "description": "Calculated joint linear or angular velocity"})===", "") \
    FUNCX(propJoint_jointType, "jointType", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable, -1, -1, -1, -1, -1, R"===({"label": "Type", "description": "Joint type"})===", "") \
    FUNCX(propJoint_cyclic, "cyclic", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"===({"label": "Cyclic", "description": "Cyclic revolute joint, has no limits"})===", "") \
    FUNCX(propJoint_enforceLimits, "enforceLimits", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"===({"label": "Enforce limits", "description": "Enforce limits strictly"})===", "") \
    FUNCX(propJoint_interval, "interval", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1, R"===({"label": "Interval", "description": "Joint limits (lower and upper bounds)"})===", "") \
    FUNCX(propJoint_targetPos, "targetPos", sim_propertytype_float, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"===({"label": "Target position", "description": "Position to reach by controller"})===", "") \
    FUNCX(propJoint_targetVel, "targetVel", sim_propertytype_float, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"===({"label": "Target velocity", "description": "Velocity to reach by controller"})===", "") \
    FUNCX(propJoint_targetForce, "targetForce", sim_propertytype_float, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"===({"label": "Target force", "description": "Maximum force to exert"})===", "") \
    FUNCX(propJoint_jointForce, "jointForce", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"===({"label": "Force", "description": "Force applied"})===", "") \
    FUNCX(propJoint_averageJointForce, "averageJointForce", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"===({"label": "Average force", "description": "Force applied on average (in a simulation step)"})===", "") \
    FUNCX(propJoint_jointMode, "jointMode", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"===({"label": "Mode", "description": "Joint mode"})===", "") \
    FUNCX(propJoint_dynCtrlMode, "dynCtrlMode", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"===({"label": "Control mode", "description": "Joint control mode, when in dynamic mode"})===", "") \
    FUNCX(propJoint_dependencyMaster, "dependencyMasterHandle", sim_propertytype_int, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"===({"label": "Dependency master", "description": "Handle of master joint (in a dependency relationship)"})===", "") \
    FUNCX(propJoint_dependencyParams, "dependencyParams", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1, R"===({"label": "Dependency parameters", "description": "Dependency parameters (offset and coefficient)"})===", "") \
    FUNCX(propJoint_maxVelAccelJerk, "maxVelAccelJerk", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1, R"===({"label": "Maximum velocity, acceleration and jerk", "description": ""})===", "") \
    FUNCX(propJoint_springDamperParams, "springDamperParams", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1, R"===({"label": "Spring damper parameters", "description": ""})===", "") \
    FUNCX(propJoint_dynVelMode, "dynVelMode", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"===({"label": "Dynamic velocity mode", "description": "Dynamic velocity mode (0: default, 1: Ruckig)"})===", "") \
    FUNCX(propJoint_dynPosMode, "dynPosMode", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"===({"label": "Dynamic position mode", "description": "Dynamic position mode (0: default, 1: Ruckig)"})===", "") \
    FUNCX(propJoint_engineProperties, "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"===({"label": "Engine properties", "description": "Engine properties as JSON text"})===", "") \
    FUNCX(propJoint_bulletStopErp, "bullet.stopErp", sim_propertytype_float, 0, sim_bullet_joint_stoperp, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_bulletStopCfm, "bullet.stopCfm", sim_propertytype_float, 0, sim_bullet_joint_stopcfm, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_bulletNormalCfm, "bullet.normalCfm", sim_propertytype_float, 0, sim_bullet_joint_normalcfm, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_bulletPosPid, "bullet.posPid", sim_propertytype_floatarray, 0, sim_bullet_joint_pospid1, sim_bullet_joint_pospid2, sim_bullet_joint_pospid3, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_odeStopErp, "ode.stopErp", sim_propertytype_float, 0, sim_ode_joint_stoperp, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_odeStopCfm, "ode.stopCfm", sim_propertytype_float, 0, sim_ode_joint_stopcfm, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_odeNormalCfm, "ode.normalCfm", sim_propertytype_float, 0, sim_ode_joint_normalcfm, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_odeBounce, "ode.bounce", sim_propertytype_float, 0, sim_ode_joint_bounce, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_odeFudgeFactor, "ode.fudge", sim_propertytype_float, 0, sim_ode_joint_fudgefactor, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_odePosPid, "ode.posPid", sim_propertytype_floatarray, 0, sim_ode_joint_pospid1, sim_ode_joint_pospid2, sim_ode_joint_pospid3, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexLowerLimitDamping, "vortex.axisLimitsLowerDamping", sim_propertytype_float, 0, sim_vortex_joint_lowerlimitdamping, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexUpperLimitDamping, "vortex.axisLimitsUpperDamping", sim_propertytype_float, 0, sim_vortex_joint_upperlimitdamping, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexLowerLimitStiffness, "vortex.axisLimitsLowerStiffness", sim_propertytype_float, 0, sim_vortex_joint_lowerlimitstiffness, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexUpperLimitStiffness, "vortex.axisLimitsUpperStiffness", sim_propertytype_float, 0, sim_vortex_joint_upperlimitstiffness, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexLowerLimitRestitution, "vortex.axisLimitsLowerRestitution", sim_propertytype_float, 0, sim_vortex_joint_lowerlimitrestitution, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexUpperLimitRestitution, "vortex.axisLimitsUpperRestitution", sim_propertytype_float, 0, sim_vortex_joint_upperlimitrestitution, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexLowerLimitMaxForce, "vortex.axisLimitsLowerMaxForce", sim_propertytype_float, 0, sim_vortex_joint_lowerlimitmaxforce, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexUpperLimitMaxForce, "vortex.axisLimitsUpperMaxForce", sim_propertytype_float, 0, sim_vortex_joint_upperlimitmaxforce, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexAxisFrictionEnabled, "vortex.axisFrictionEnabled", sim_propertytype_bool, 0, sim_vortex_joint_motorfrictionenabled, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexAxisFrictionProportional, "vortex.axisFrictionProportional", sim_propertytype_bool, 0, sim_vortex_joint_proportionalmotorfriction, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexAxisFrictionCoeff, "vortex.axisFrictionValue", sim_propertytype_float, 0, sim_vortex_joint_motorconstraintfrictioncoeff, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexAxisFrictionMaxForce, "vortex.axisFrictionMaxForce", sim_propertytype_float, 0, sim_vortex_joint_motorconstraintfrictionmaxforce, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexAxisFrictionLoss, "vortex.axisFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_motorconstraintfrictionloss, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexRelaxationEnabledBits, "vortex.relaxationEnabledBits", sim_propertytype_int, 0, sim_vortex_joint_relaxationenabledbc, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexFrictionEnabledBits, "vortex.frictionEnabledBits", sim_propertytype_int, 0, sim_vortex_joint_frictionenabledbc, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexFrictionProportionalBits, "vortex.frictionProportionalBits", sim_propertytype_int, 0, sim_vortex_joint_frictionproportionalbc, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexXAxisPosRelaxationStiffness, "vortex.xAxisPosRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_p0stiffness, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexXAxisPosRelaxationDamping, "vortex.xAxisPosRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_p0damping, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexXAxisPosRelaxationLoss, "vortex.xAxisPosRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_p0loss, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexXAxisPosFrictionCoeff, "vortex.xAxisPosFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_p0frictioncoeff, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexXAxisPosFrictionMaxForce, "vortex.xAxisPosFrictionMaxForce", sim_propertytype_float, 0, sim_vortex_joint_p0frictionmaxforce, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexXAxisPosFrictionLoss, "vortex.xAxisPosFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_p0frictionloss, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexYAxisPosRelaxationStiffness, "vortex.yAxisPosRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_p1stiffness, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexYAxisPosRelaxationDamping, "vortex.yAxisPosRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_p1damping, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexYAxisPosRelaxationLoss, "vortex.yAxisPosRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_p1loss, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexYAxisPosFrictionCoeff, "vortex.yAxisPosFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_p1frictioncoeff, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexYAxisPosFrictionMaxForce, "vortex.yAxisPosFrictionMaxForce", sim_propertytype_float, 0, sim_vortex_joint_p1frictionmaxforce, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexYAxisPosFrictionLoss, "vortex.yAxisPosFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_p1frictionloss, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexZAxisPosRelaxationStiffness, "vortex.zAxisPosRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_p2stiffness, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexZAxisPosRelaxationDamping, "vortex.zAxisPosRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_p2damping, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexZAxisPosRelaxationLoss, "vortex.zAxisPosRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_p2loss, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexZAxisPosFrictionCoeff, "vortex.zAxisPosFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_p2frictioncoeff, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexZAxisPosFrictionMaxForce, "vortex.zAxisPosFrictionMaxForce", sim_propertytype_float, 0, sim_vortex_joint_p2frictionmaxforce, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexZAxisPosFrictionLoss, "vortex.zAxisPosFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_p2frictionloss, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexXAxisOrientRelaxStiffness, "vortex.xAxisOrientRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_a0stiffness, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexXAxisOrientRelaxDamping, "vortex.xAxisOrientRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_a0damping, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexXAxisOrientRelaxLoss, "vortex.xAxisOrientRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_a0loss, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexXAxisOrientFrictionCoeff, "vortex.xAxisOrientFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_a0frictioncoeff, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexXAxisOrientFrictionMaxTorque, "vortex.xAxisOrientFrictionMaxTorque", sim_propertytype_float, 0, sim_vortex_joint_a0frictionmaxforce, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexXAxisOrientFrictionLoss, "vortex.xAxisOrientFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_a0frictionloss, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexYAxisOrientRelaxStiffness, "vortex.yAxisOrientRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_a1stiffness, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexYAxisOrientRelaxDamping, "vortex.yAxisOrientRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_a1damping, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexYAxisOrientRelaxLoss, "vortex.yAxisOrientRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_a1loss, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexYAxisOrientFrictionCoeff, "vortex.yAxisOrientFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_a1frictioncoeff, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexYAxisOrientFrictionMaxTorque, "vortex.yAxisOrientFrictionMaxTorque", sim_propertytype_float, 0, sim_vortex_joint_a1frictionmaxforce, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexYAxisOrientFrictionLoss, "vortex.yAxisOrientFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_a1frictionloss, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexZAxisOrientRelaxStiffness, "vortex.zAxisOrientRelaxationStiffness", sim_propertytype_float, 0, sim_vortex_joint_a2stiffness, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexZAxisOrientRelaxDamping, "vortex.zAxisOrientRelaxationDamping", sim_propertytype_float, 0, sim_vortex_joint_a2damping, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexZAxisOrientRelaxLoss, "vortex.zAxisOrientRelaxationLoss", sim_propertytype_float, 0, sim_vortex_joint_a2loss, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexZAxisOrientFrictionCoeff, "vortex.zAxisOrientFrictionCoeff", sim_propertytype_float, 0, sim_vortex_joint_a2frictioncoeff, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexZAxisOrientFrictionMaxTorque, "vortex.zAxisOrientFrictionMaxTorque", sim_propertytype_float, 0, sim_vortex_joint_a2frictionmaxforce, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexZAxisOrientFrictionLoss, "vortex.zAxisOrientFrictionLoss", sim_propertytype_float, 0, sim_vortex_joint_a2frictionloss, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_vortexPosPid, "vortex.posPid", sim_propertytype_floatarray, 0, sim_vortex_joint_pospid1, sim_vortex_joint_pospid2, sim_vortex_joint_pospid3, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_newtonPosPid, "newton.posPid", sim_propertytype_floatarray, 0, sim_newton_joint_pospid1, sim_newton_joint_pospid2, sim_newton_joint_pospid3, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_mujocoArmature, "mujoco.armature", sim_propertytype_float, 0, sim_mujoco_joint_armature, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_mujocoMargin, "mujoco.margin", sim_propertytype_float, 0, sim_mujoco_joint_margin, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_mujocoFrictionLoss, "mujoco.frictionLoss", sim_propertytype_float, 0, sim_mujoco_joint_frictionloss, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_mujocoSpringStiffness, "mujoco.springStiffness", sim_propertytype_float, 0, sim_mujoco_joint_stiffness, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_mujocoSpringDamping, "mujoco.springDamping", sim_propertytype_float, 0, sim_mujoco_joint_damping, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_mujocoSpringRef, "mujoco.springRef", sim_propertytype_float, 0, sim_mujoco_joint_springref, -1, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_mujocoPosPid, "mujoco.posPid", sim_propertytype_floatarray, 0, sim_mujoco_joint_pospid1, sim_mujoco_joint_pospid2, sim_mujoco_joint_pospid3, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_mujocoLimitsSolRef, "mujoco.limitsSolref", sim_propertytype_floatarray, 0, sim_mujoco_joint_solreflimit1, sim_mujoco_joint_solreflimit2, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_mujocoLimitsSolImp, "mujoco.limitsSolimp", sim_propertytype_floatarray, 0, sim_mujoco_joint_solimplimit1, sim_mujoco_joint_solimplimit2, sim_mujoco_joint_solimplimit3, sim_mujoco_joint_solimplimit4, sim_mujoco_joint_solimplimit5, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_mujocoFrictionSolRef, "mujoco.frictionSolref", sim_propertytype_floatarray, 0, sim_mujoco_joint_solreffriction1, sim_mujoco_joint_solreffriction2, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_mujocoFrictionSolImp, "mujoco.frictionSolimp", sim_propertytype_floatarray, 0, sim_mujoco_joint_solimpfriction1, sim_mujoco_joint_solimpfriction2, sim_mujoco_joint_solimpfriction3, sim_mujoco_joint_solimpfriction4, sim_mujoco_joint_solimpfriction5, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_mujocoSpringDamper, "mujoco.springSpringDamper", sim_propertytype_floatarray, 0, sim_mujoco_joint_springdamper1, sim_mujoco_joint_springdamper2, -1, -1, -1, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propJoint_mujocoDependencyPolyCoef, "mujoco.dependencyPolyCoeff", sim_propertytype_floatarray, 0, sim_mujoco_joint_polycoef1, sim_mujoco_joint_polycoef2, sim_mujoco_joint_polycoef3, sim_mujoco_joint_polycoef4, sim_mujoco_joint_polycoef5, R"===({"label": "", "description": ""})===", "")

#define LIGHT_PROPERTIES \
    FUNCX(propLight_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Object meta information", "description": ""})===", "") \
    FUNCX(propLight_size, "lightSize", sim_propertytype_float, 0, R"===({"label": "Size", "description": "Light size"})===", "") \
    FUNCX(propLight_enabled, "enabled", sim_propertytype_bool, 0, R"===({"label": "Enabled", "description": ""})===", "") \
    FUNCX(propLight_lightType, "lightType", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable, R"===({"label": "Type", "description": "Light type"})===", "") \
    FUNCX(propLight_spotExponent, "spotExponent", sim_propertytype_int, 0, R"===({"label": "Spot exponent", "description": ""})===", "") \
    FUNCX(propLight_spotCutoffAngle, "spotCutoffAngle", sim_propertytype_float, 0, R"===({"label": "Cut off angle", "description": "Spot cut off angle"})===", "") \
    FUNCX(propLight_attenuationFactors, "attenuationFactors", sim_propertytype_floatarray, 0, R"===({"label": "Attenuation factor", "description": ""})===", "") \
    FUNCX(propLight_povCastShadows, "povray.castShadows", sim_propertytype_bool, sim_propertyinfo_silent, R"===({"label": "POV-Ray: cast shadows", "description": "Light casts shadows (with the POV-Ray renderer plugin)"})===", "")

#define MIRROR_PROPERTIES \
    FUNCX(propMirror_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Object meta information", "description": ""})===", "") \

#define OCTREE_PROPERTIES \
    FUNCX(propOctree_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Object meta information", "description": ""})===", "") \
    FUNCX(propOctree_voxelSize, "voxelSize", sim_propertytype_float, 0, R"===({"label": "Voxel size", "description": ""})===", "") \
    FUNCX(propOctree_randomColors, "randomColors", sim_propertytype_bool, 0, R"===({"label": "Random voxel colors", "description": ""})===", "") \
    FUNCX(propOctree_showPoints, "showPoints", sim_propertytype_bool, 0, R"===({"label": "Show points instead of voxels", "description": ""})===", "") \
    FUNCX(propOctree_voxels, "voxels", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Voxels", "description": "Voxel positions"})===", "") \
    FUNCX(propOctree_colors, "colors", sim_propertytype_buffer, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Voxel Colors", "description": ""})===", "")

#define POINTCLOUD_PROPERTIES \
    FUNCX(propPointCloud_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Object meta information", "description": ""})===", "") \
    FUNCX(propPointCloud_ocTreeStruct, "ocTreeStruct", sim_propertytype_bool, 0, R"===({"label": "OC-Tree structure Enabled", "description": "Use an oc-tree structure"})===", "") \
    FUNCX(propPointCloud_randomColors, "randomColors", sim_propertytype_bool, 0, R"===({"label": "Random point colors", "description": ""})===", "") \
    FUNCX(propPointCloud_pointSize, "pointSize", sim_propertytype_int, 0, R"===({"label": "Point size", "description": ""})===", "") \
    FUNCX(propPointCloud_maxPtsInCell, "maxPointsInCell", sim_propertytype_int, 0, R"===({"label": "Max. points in cell", "description": "Maximum number of points in an oc-tree cell/voxel"})===", "") \
    FUNCX(propPointCloud_cellSize, "cellSize", sim_propertytype_float, 0, R"===({"label": "Cell size", "description": "Size of the oc-tree cell/voxel"})===", "") \
    FUNCX(propPointCloud_pointDisplayFraction, "pointDisplayFraction", sim_propertytype_float, 0, R"===({"label": "Display fraction", "description": "Fraction of points to be displayed in an oc-tree cell/voxel"})===", "") \
    FUNCX(propPointCloud_points, "points", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Points", "description": "Point positions"})===", "") \
    FUNCX(propPointCloud_colors, "colors", sim_propertytype_buffer, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Colors", "description": "Point colors"})===", "")

#define PROXIMITYSENSOR_PROPERTIES \
    FUNCX(propProximitySensor_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Object meta information", "description": ""})===", "") \
    FUNCX(propProximitySensor_size, "sensorPointSize", sim_propertytype_float, 0, R"===({"label": "Sensor point size", "description": ""})===", "") \
    FUNCX(propProximitySensor_frontFaceDetection, "frontFaceDetection", sim_propertytype_bool, 0, R"===({"label": "Front face detection", "description": ""})===", "") \
    FUNCX(propProximitySensor_backFaceDetection, "backFaceDetection", sim_propertytype_bool, 0, R"===({"label": "Back face detection", "description": ""})===", "") \
    FUNCX(propProximitySensor_exactMode, "exactMode", sim_propertytype_bool, 0, R"===({"label": "Exact mode", "description": ""})===", "") \
    FUNCX(propProximitySensor_explicitHandling, "explicitHandling", sim_propertytype_bool, 0, R"===({"label": "Explicit handling", "description": ""})===", "") \
    FUNCX(propProximitySensor_showVolume, "showVolume", sim_propertytype_bool, 0, R"===({"label": "Show volume", "description": ""})===", "") \
    FUNCX(propProximitySensor_randomizedDetection, "randomizedDetection", sim_propertytype_bool, sim_propertyinfo_notwritable, R"===({"label": "Randomized detection", "description": ""})===", "") \
    FUNCX(propProximitySensor_sensorType, "sensorType", sim_propertytype_int, sim_propertyinfo_constant | sim_propertyinfo_notwritable, R"===({"label": "Type", "description": "Sensor type"})===", "") \
    FUNCX(propProximitySensor_detectedObjectHandle, "detectedObjectHandle", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Detected object", "description": "Detected scene object handle"})===", "") \
    FUNCX(propProximitySensor_angleThreshold, "angleThreshold", sim_propertytype_float, 0, R"===({"label": "Angle threshold", "description": "Angle threshold, 0.0 to disable"})===", "") \
    FUNCX(propProximitySensor_detectedPoint, "detectedPoint", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Detected point", "description": ""})===", "") \
    FUNCX(propProximitySensor_detectedNormal, "detectedNormal", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Detected normal", "description": "Detected normal vector"})===", "")

#define VISIONSENSOR_PROPERTIES \
    FUNCX(propVisionSensor_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Object meta information", "description": ""})===", "") \
    FUNCX(propVisionSensor_size, "sensorSize", sim_propertytype_float, 0, R"===({"label": "Size", "description": "Sensor size"})===", "") \
    FUNCX(propVisionSensor_backgroundCol, "backgroundColor", sim_propertytype_color, 0, R"===({"label": "Background color", "description": ""})===", "") \
    FUNCX(propVisionSensor_renderMode, "renderMode", sim_propertytype_int, 0, R"===({"label": "Render mode", "description": ""})===", "") \
    FUNCX(propVisionSensor_backgroundSameAsEnv, "backgroundColorFromEnvironment", sim_propertytype_bool, 0, R"===({"label": "Background color from environment", "description": ""})===", "") \
    FUNCX(propVisionSensor_explicitHandling, "explicitHandling", sim_propertytype_bool, 0, R"===({"label": "Explicit handling", "description": ""})===", "") \
    FUNCX(propVisionSensor_useExtImage, "useExtImage", sim_propertytype_bool, 0, R"===({"label": "External input", "description": ""})===", "") \
    FUNCX(propVisionSensor_ignoreRgbInfo, "ignoreImageInfo", sim_propertytype_bool, 0, R"===({"label": "Ignore RGB buffer", "description": "Ignore RGB buffer (faster)"})===", "") \
    FUNCX(propVisionSensor_ignoreDepthInfo, "ignoreDepthInfo", sim_propertytype_bool, 0, R"===({"label": "Ignore Depth buffer", "description": "Ignore Depth buffer (faster)"})===", "") \
    FUNCX(propVisionSensor_omitPacket1, "omitPacket1", sim_propertytype_bool, 0, R"===({"label": "Packet 1 is blank", "description": "Omit packet 1 (faster)"})===", "") \
    FUNCX(propVisionSensor_emitImageChangedEvent, "emitImageChangedEvent", sim_propertytype_bool, 0, R"===({"label": "Emit image change event", "description": ""})===", "") \
    FUNCX(propVisionSensor_emitDepthChangedEvent, "emitDepthChangedEvent", sim_propertytype_bool, 0, R"===({"label": "Emit depth change event", "description": ""})===", "") \
    FUNCX(propVisionSensor_imageBuffer, "imageBuffer", sim_propertytype_buffer, sim_propertyinfo_modelhashexclude, R"===({"label": "RGB buffer", "description": ""})===", "") \
    FUNCX(propVisionSensor_depthBuffer, "depthBuffer", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Depth buffer", "description": ""})===", "") \
    FUNCX(propVisionSensor_packedDepthBuffer, "packedDepthBuffer", sim_propertytype_buffer, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Packed depth buffer", "description": ""})===", "") \
    FUNCX(propVisionSensor_triggerState, "triggerState", sim_propertytype_bool, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Trigger state", "description": ""})===", "") \
    FUNCX(propVisionSensor_packet1, "packet1", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Data packet 1", "description": ""})===", "") \
    FUNCX(propVisionSensor_packet2, "packet2", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Data packet 2", "description": ""})===", "") \
    FUNCX(propVisionSensor_povFocalBlur, "povray.focalBlur", sim_propertytype_bool, sim_propertyinfo_silent, R"===({"label": "POV-Ray: focal blur", "description": "Focal blur (with the POV-Ray renderer plugin)"})===", "") \
    FUNCX(propVisionSensor_povBlurSamples, "povray.blurSamples", sim_propertytype_int, sim_propertyinfo_silent, R"===({"label": "POV-Ray: blur samples", "description": "Focal blur samples (with the POV-Ray renderer plugin)"})===", "") \
    FUNCX(propVisionSensor_povBlurDistance, "povray.blurDistance", sim_propertytype_float, sim_propertyinfo_silent, R"===({"label": "POV-Ray: blur distance", "description": "Focal blur distance (with the POV-Ray renderer plugin)"})===", "") \
    FUNCX(propVisionSensor_povAperture, "povray.aperture", sim_propertytype_float, sim_propertyinfo_silent, R"===({"label": "POV-Ray: aperture", "description": "Aperture (with the POV-Ray renderer plugin)"})===", "")

#define SCRIPT_PROPERTIES \
    FUNCX(propScript_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Object meta information", "description": ""})===", "") \
    FUNCX(propScript_size, "scriptSize", sim_propertytype_float, 0, R"===({"label": "Size", "description": "Size of the script object"})===", "") \
    FUNCX(propScript_resetAfterSimError, "resetAfterSimError", sim_propertytype_bool, 0, R"===({"label": "Reset after simulation error", "description": ""})===", "") \
    FUNCX(propScript_detachedScript, "detachedScript", sim_propertytype_handle, sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Detached script handle", "description": "", "handleType": "detachedScript"})===", "") \
    /* Following for backward compatibility: */ \
    FUNCX(propScript_scriptDisabled, "scriptDisabled", sim_propertytype_bool, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propScript_restartOnError, "restartOnError", sim_propertytype_bool, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propScript_execPriority, "execPriority", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propScript_scriptType, "scriptType", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude | sim_propertyinfo_notwritable, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propScript_executionDepth, "executionDepth", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propScript_scriptState, "scriptState", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propScript_language, "language", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propScript_code, "code", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propScript_scriptName, "scriptName", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propScript_addOnPath, "addOnPath", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propScript_addOnMenuPath, "addOnMenuPath", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_constant | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "")

