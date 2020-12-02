#include "easyLock.h"
#include "simInternal.h"
#include "luaScriptObject.h"
#include "luaScriptFunctions.h"
#include "threadPool.h"
#include "tt.h"
#include <boost/lexical_cast.hpp>
#include "vDateTime.h"
#include "app.h"
#include "apiErrors.h"
#include "pluginContainer.h"
#include <boost/algorithm/string.hpp>
#include "ttUtil.h"

int CLuaScriptObject::_scriptUniqueCounter=-1;
bool CLuaScriptObject::emergencyStopButtonPressed=false;
int CLuaScriptObject::_nextIdForExternalScriptEditor=-1;
VMutex CLuaScriptObject::_globalMutex;
std::vector<CLuaScriptObject*> CLuaScriptObject::toBeCalledByThread;
std::map<std::string,std::string> CLuaScriptObject::_newApiMap;

const SNewApiMapping _simApiMapping[]=
{
    "sim.mainscriptcall_initialization","sim.syscb_init",
    "sim.mainscriptcall_cleanup","sim.syscb_cleanup",
    "sim.mainscriptcall_regular","sim.syscb_regular",
    "sim.childscriptcall_initialization","sim.syscb_init",
    "sim.childscriptcall_cleanup","sim.syscb_cleanup",
    "sim.childscriptcall_actuation","sim.syscb_actuation",
    "sim.childscriptcall_sensing","sim.syscb_sensing",
    "sim.customizationscriptcall_initialization","sim.syscb_init",
    "sim.customizationscriptcall_cleanup","sim.syscb_cleanup",
    "sim.customizationscriptcall_nonsimulation","sim.syscb_nonsimulation",
    "sim.customizationscriptcall_lastbeforesimulation","sim.syscb_beforesimulation",
    "sim.customizationscriptcall_firstaftersimulation","sim.syscb_aftersimulation",
    "sim.customizationscriptcall_simulationactuation","sim.syscb_actuation",
    "sim.customizationscriptcall_simulationsensing","sim.syscb_sensing",
    "sim.customizationscriptcall_simulationpause","sim.syscb_suspended",
    "sim.customizationscriptcall_simulationpausefirst","sim.syscb_suspend",
    "sim.customizationscriptcall_simulationpauselast","sim.syscb_resume",
    "sim.customizationscriptcall_lastbeforeinstanceswitch","sim.syscb_beforeinstanceswitch",
    "sim.customizationscriptcall_firstafterinstanceswitch","sim.syscb_afterinstanceswitch",
    "sim.customizationscriptcall_beforecopy","sim.syscb_beforecopy",
    "sim.customizationscriptcall_aftercopy","sim.syscb_aftercopy",
    "sim.customizationscriptcall_br","sim.syscb_br",

    "simGetScriptName","sim.getScriptName",
    "simGetObjectAssociatedWithScript","sim.getObjectAssociatedWithScript",
    "simGetScriptAssociatedWithObject","sim.getScriptAssociatedWithObject",
    "simGetCustomizationScriptAssociatedWithObject","sim.getCustomizationScriptAssociatedWithObject",
    "simGetScriptExecutionCount","sim.getScriptExecutionCount",
    "simIsScriptExecutionThreaded","sim.isScriptExecutionThreaded",
    "simIsScriptRunningInThread","sim.isScriptRunningInThread",
    "simOpenModule","sim.openModule",
    "simCloseModule","sim.closeModule",
    "simHandleModule","sim.handleModule",
    "simBoolOr32","sim.boolOr32",
    "simBoolAnd32","sim.boolAnd32",
    "simBoolXor32","sim.boolXor32",
    "simHandleDynamics","sim.handleDynamics",
    "simHandleIkGroup","sim.handleIkGroup",
    "simCheckIkGroup","sim.checkIkGroup",
    "simHandleCollision","sim.handleCollision",
    "simReadCollision","sim.readCollision",
    "simHandleDistance","sim.handleDistance",
    "simReadDistance","sim.readDistance",
    "simHandleProximitySensor","sim.handleProximitySensor",
    "simReadProximitySensor","sim.readProximitySensor",
    "simHandleMill","sim.handleMill",
    "simResetCollision","sim.resetCollision",
    "simResetDistance","sim.resetDistance",
    "simResetProximitySensor","sim.resetProximitySensor",
    "simResetMill","sim.resetMill",
    "simCheckProximitySensor","sim.checkProximitySensor",
    "simCheckProximitySensorEx","sim.checkProximitySensorEx",
    "simCheckProximitySensorEx2","sim.checkProximitySensorEx2",
    "simGetNameSuffix","sim.getNameSuffix",
    "simSetNameSuffix","sim.setNameSuffix",
    "simGetObjectHandle","sim.getObjectHandle",
    "simAddScript","sim.addScript",
    "simAssociateScriptWithObject","sim.associateScriptWithObject",
    "simSetScriptText","sim.setScriptText",
    "simGetScriptHandle","sim.getScriptHandle",
    "simGetCollectionHandle","sim.getCollectionHandle",
    "simRemoveCollection","sim.removeCollection",
    "simEmptyCollection","sim.emptyCollection",
    "simGetObjectPosition","sim.getObjectPosition",
    "simGetObjectOrientation","sim.getObjectOrientation",
    "simSetObjectPosition","sim.setObjectPosition",
    "simSetObjectOrientation","sim.setObjectOrientation",
    "simGetJointPosition","sim.getJointPosition",
    "simSetJointPosition","sim.setJointPosition",
    "simSetJointTargetPosition","sim.setJointTargetPosition",
    "simGetJointTargetPosition","sim.getJointTargetPosition",
    "simSetJointForce","sim.setJointForce",
    "simGetPathPosition","sim.getPathPosition",
    "simSetPathPosition","sim.setPathPosition",
    "simGetPathLength","sim.getPathLength",
    "simSetJointTargetVelocity","sim.setJointTargetVelocity",
    "simGetJointTargetVelocity","sim.getJointTargetVelocity",
    "simSetPathTargetNominalVelocity","sim.setPathTargetNominalVelocity",
    "simGetObjectName","sim.getObjectName",
    "simGetCollectionName","sim.getCollectionName",
    "simRemoveObject","sim.removeObject",
    "simRemoveModel","sim.removeModel",
    "simGetSimulationTime","sim.getSimulationTime",
    "simGetSimulationState","sim.getSimulationState",
    "simGetSystemTime","sim.getSystemTime",
    "simGetSystemTimeInMs","sim.getSystemTimeInMs",
    "simCheckCollision","sim.checkCollision",
    "simCheckCollisionEx","sim.checkCollisionEx",
    "simCheckDistance","sim.checkDistance",
    "simGetObjectConfiguration","sim.getObjectConfiguration",
    "simSetObjectConfiguration","sim.setObjectConfiguration",
    "simGetConfigurationTree","sim.getConfigurationTree",
    "simSetConfigurationTree","sim.setConfigurationTree",
    "simHandleMechanism","sim.handleMechanism",
    "simGetSimulationTimeStep","sim.getSimulationTimeStep",
    "simGetSimulatorMessage","sim.getSimulatorMessage",
    "simResetGraph","sim.resetGraph",
    "simHandleGraph","sim.handleGraph",
    "simAddStatusbarMessage","sim.addStatusbarMessage",
    "simGetLastError","sim.getLastError",
    "simGetObjects","sim.getObjects",
    "simRefreshDialogs","sim.refreshDialogs",
    "simGetModuleName","sim.getModuleName",
    "simGetIkGroupHandle","sim.getIkGroupHandle",
    "simRemoveScript","sim.removeScript",
    "simGetCollisionHandle","sim.getCollisionHandle",
    "simGetDistanceHandle","sim.getDistanceHandle",
    "simGetMechanismHandle","sim.getMechanismHandle",
    "simGetScriptSimulationParameter","sim.getScriptSimulationParameter",
    "simSetScriptSimulationParameter","sim.setScriptSimulationParameter",
    "simDisplayDialog","sim.displayDialog",
    "simGetDialogResult","sim.getDialogResult",
    "simGetDialogInput","sim.getDialogInput",
    "simEndDialog","sim.endDialog",
    "simStopSimulation","sim.stopSimulation",
    "simPauseSimulation","sim.pauseSimulation",
    "simStartSimulation","sim.startSimulation",
    "simGetObjectMatrix","sim.getObjectMatrix",
    "simSetObjectMatrix","sim.setObjectMatrix",
    "simGetJointMatrix","sim.getJointMatrix",
    "simSetSphericalJointMatrix","sim.setSphericalJointMatrix",
    "simBuildIdentityMatrix","sim.buildIdentityMatrix",
    "simCopyMatrix","sim.copyMatrix",
    "simBuildMatrix","sim.buildMatrix",
    "simGetEulerAnglesFromMatrix","sim.getEulerAnglesFromMatrix",
    "simInvertMatrix","sim.invertMatrix",
    "simMultiplyMatrices","sim.multiplyMatrices",
    "simInterpolateMatrices","sim.interpolateMatrices",
    "simMultiplyVector","sim.multiplyVector",
    "simGetObjectChild","sim.getObjectChild",
    "simGetObjectParent","sim.getObjectParent",
    "simSetObjectParent","sim.setObjectParent",
    "simGetObjectType","sim.getObjectType",
    "simGetJointType","sim.getJointType",
    "simSetBoolParameter","sim.setBoolParameter",
    "simGetBoolParameter","sim.getBoolParameter",
    "simSetInt32Parameter","sim.setInt32Parameter",
    "simGetInt32Parameter","sim.getInt32Parameter",
    "simSetFloatParameter","sim.setFloatParameter",
    "simGetFloatParameter","sim.getFloatParameter",
    "simSetStringParameter","sim.setStringParameter",
    "simGetStringParameter","sim.getStringParameter",
    "simSetArrayParameter","sim.setArrayParameter",
    "simGetArrayParameter","sim.getArrayParameter",
    "simSetObjectName","sim.setObjectName",
    "simSetCollectionName","sim.setCollectionName",
    "simGetJointInterval","sim.getJointInterval",
    "simSetJointInterval","sim.setJointInterval",
    "simLoadScene","sim.loadScene",
    "simSaveScene","sim.saveScene",
    "simLoadModel","sim.loadModel",
    "simSaveModel","sim.saveModel",
    "simIsObjectInSelection","sim.isObjectInSelection",
    "simAddObjectToSelection","sim.addObjectToSelection",
    "simRemoveObjectFromSelection","sim.removeObjectFromSelection",
    "simGetObjectSelection","sim.getObjectSelection",
    "simGetRealTimeSimulation","sim.getRealTimeSimulation",
    "simSetNavigationMode","sim.setNavigationMode",
    "simGetNavigationMode","sim.getNavigationMode",
    "simSetPage","sim.setPage",
    "simGetPage","sim.getPage",
    "simCopyPasteObjects","sim.copyPasteObjects",
    "simScaleObjects","sim.scaleObjects",
    "simGetObjectUniqueIdentifier","sim.getObjectUniqueIdentifier",
    "simSetThreadAutomaticSwitch","sim.setThreadAutomaticSwitch",
    "simGetThreadAutomaticSwitch","sim.getThreadAutomaticSwitch",
    "simSetThreadSwitchTiming","sim.setThreadSwitchTiming",
    "simSetThreadResumeLocation","sim.setThreadResumeLocation",
    "simResumeThreads","sim.resumeThreads",
    "simSwitchThread","sim.switchThread",
    "simCreateIkGroup","sim.createIkGroup",
    "simRemoveIkGroup","sim.removeIkGroup",
    "simCreateIkElement","sim.createIkElement",
    "simCreateCollection","sim.createCollection",
    "simAddObjectToCollection","sim.addObjectToCollection",
    "simSaveImage","sim.saveImage",
    "simLoadImage","sim.loadImage",
    "simGetScaledImage","sim.getScaledImage",
    "simTransformImage","sim.transformImage",
    "simGetQHull","sim.getQHull",
    "simGetDecimatedMesh","sim.getDecimatedMesh",
    "simExportIk","sim.exportIk",
    "simComputeJacobian","sim.computeJacobian",
    "simSendData","sim.sendData",
    "simReceiveData","sim.receiveData",
    "simPackInt32Table","sim.packInt32Table",
    "simPackUInt32Table","sim.packUInt32Table",
    "simPackFloatTable","sim.packFloatTable",
    "simPackDoubleTable","sim.packDoubleTable",
    "simPackUInt8Table","sim.packUInt8Table",
    "simPackUInt16Table","sim.packUInt16Table",
    "simUnpackInt32Table","sim.unpackInt32Table",
    "simUnpackUInt32Table","sim.unpackUInt32Table",
    "simUnpackFloatTable","sim.unpackFloatTable",
    "simUnpackDoubleTable","sim.unpackDoubleTable",
    "simUnpackUInt8Table","sim.unpackUInt8Table",
    "simUnpackUInt16Table","sim.unpackUInt16Table",
    "simPackTable","sim.packTable",
    "simUnpackTable","sim.unpackTable",
    "simTransformBuffer","sim.transformBuffer",
    "simCombineRgbImages","sim.combineRgbImages",
    "simGetVelocity","sim.getVelocity",
    "simGetObjectVelocity","sim.getObjectVelocity",
    "simAddForceAndTorque","sim.addForceAndTorque",
    "simAddForce","sim.addForce",
    "simSetExplicitHandling","sim.setExplicitHandling",
    "simGetExplicitHandling","sim.getExplicitHandling",
    "simSetGraphUserData","sim.setGraphUserData",
    "simAddDrawingObject","sim.addDrawingObject",
    "simRemoveDrawingObject","sim.removeDrawingObject",
    "simAddDrawingObjectItem","sim.addDrawingObjectItem",
    "simAddParticleObject","sim.addParticleObject",
    "simRemoveParticleObject","sim.removeParticleObject",
    "simAddParticleObjectItem","sim.addParticleObjectItem",
    "simGetObjectSizeFactor","sim.getObjectSizeFactor",
    "simSetIntegerSignal","sim.setIntegerSignal",
    "simGetIntegerSignal","sim.getIntegerSignal",
    "simClearIntegerSignal","sim.clearIntegerSignal",
    "simSetFloatSignal","sim.setFloatSignal",
    "simGetFloatSignal","sim.getFloatSignal",
    "simClearFloatSignal","sim.clearFloatSignal",
    "simSetStringSignal","sim.setStringSignal",
    "simGetStringSignal","sim.getStringSignal",
    "simClearStringSignal","sim.clearStringSignal",
    "simGetSignalName","sim.getSignalName",
    "simWaitForSignal","sim.waitForSignal",
    "simPersistentDataWrite","sim.persistentDataWrite",
    "simPersistentDataRead","sim.persistentDataRead",
    "simSetObjectProperty","sim.setObjectProperty",
    "simGetObjectProperty","sim.getObjectProperty",
    "simSetObjectSpecialProperty","sim.setObjectSpecialProperty",
    "simGetObjectSpecialProperty","sim.getObjectSpecialProperty",
    "simSetModelProperty","sim.setModelProperty",
    "simGetModelProperty","sim.getModelProperty",
    "simMoveToPosition","sim.moveToPosition",
    "simMoveToObject","sim.moveToObject",
    "simFollowPath","sim.followPath",
    "simMoveToJointPositions","sim.moveToJointPositions",
    "simWait","sim.wait",
    "simGetDataOnPath","sim.getDataOnPath",
    "simGetPositionOnPath","sim.getPositionOnPath",
    "simGetOrientationOnPath","sim.getOrientationOnPath",
    "simGetClosestPositionOnPath","sim.getClosestPositionOnPath",
    "simReadForceSensor","sim.readForceSensor",
    "simBreakForceSensor","sim.breakForceSensor",
    "simGetLightParameters","sim.getLightParameters",
    "simSetLightParameters","sim.setLightParameters",
    "simGetLinkDummy","sim.getLinkDummy",
    "simSetLinkDummy","sim.setLinkDummy",
    "simSetShapeColor","sim.setShapeColor",
    "simGetShapeColor","sim.getShapeColor",
    "simResetDynamicObject","sim.resetDynamicObject",
    "simSetJointMode","sim.setJointMode",
    "simGetJointMode","sim.getJointMode",
    "simSerialOpen","sim.serialOpen",
    "simSerialClose","sim.serialClose",
    "simSerialSend","sim.serialSend",
    "simSerialRead","sim.serialRead",
    "simSerialCheck","sim.serialCheck",
    "simGetContactInfo","sim.getContactInfo",
    "simSetThreadIsFree","sim.setThreadIsFree",
    "simTubeOpen","sim.tubeOpen",
    "simTubeClose","sim.tubeClose",
    "simTubeWrite","sim.tubeWrite",
    "simTubeRead","sim.tubeRead",
    "simTubeStatus","sim.tubeStatus",
    "simAuxiliaryConsoleOpen","sim.auxiliaryConsoleOpen",
    "simAuxiliaryConsoleClose","sim.auxiliaryConsoleClose",
    "simAuxiliaryConsolePrint","sim.auxiliaryConsolePrint",
    "simAuxiliaryConsoleShow","sim.auxiliaryConsoleShow",
    "simImportShape","sim.importShape",
    "simImportMesh","sim.importMesh",
    "simExportMesh","sim.exportMesh",
    "simCreateMeshShape","sim.createMeshShape",
    "simGetShapeMesh","sim.getShapeMesh",
    "simCreatePureShape","sim.createPureShape",
    "simCreateHeightfieldShape","sim.createHeightfieldShape",
    "simAddBanner","sim.addBanner",
    "simRemoveBanner","sim.removeBanner",
    "simCreateJoint","sim.createJoint",
    "simCreateDummy","sim.createDummy",
    "simCreateProximitySensor","sim.createProximitySensor",
    "simCreatePath","sim.createPath",
    "simCreateForceSensor","sim.createForceSensor",
    "simCreateVisionSensor","sim.createVisionSensor",
    "simInsertPathCtrlPoints","sim.insertPathCtrlPoints",
    "simCutPathCtrlPoints","sim.cutPathCtrlPoints",
    "simGetIkGroupMatrix","sim.getIkGroupMatrix",
    "simFloatingViewAdd","sim.floatingViewAdd",
    "simFloatingViewRemove","sim.floatingViewRemove",
    "simAdjustView","sim.adjustView",
    "simCameraFitToView","sim.cameraFitToView",
    "simAnnounceSceneContentChange","sim.announceSceneContentChange",
    "simGetObjectInt32Parameter","sim.getObjectInt32Parameter",
    "simSetObjectInt32Parameter","sim.setObjectInt32Parameter",
    "simGetObjectFloatParameter","sim.getObjectFloatParameter",
    "simSetObjectFloatParameter","sim.setObjectFloatParameter",
    "simGetObjectStringParameter","sim.getObjectStringParameter",
    "simSetObjectStringParameter","sim.setObjectStringParameter",
    "simGetRotationAxis","sim.getRotationAxis",
    "simRotateAroundAxis","sim.rotateAroundAxis",
    "simLaunchExecutable","sim.launchExecutable",
    "simGetJointForce","sim.getJointForce",
    "simSetIkGroupProperties","sim.setIkGroupProperties",
    "simSetIkElementProperties","sim.setIkElementProperties",
    "simIsHandleValid","sim.isHandleValid",
    "simGetObjectQuaternion","sim.getObjectQuaternion",
    "simSetObjectQuaternion","sim.setObjectQuaternion",
    "simSetShapeMassAndInertia","sim.setShapeMassAndInertia",
    "simGetShapeMassAndInertia","sim.getShapeMassAndInertia",
    "simGroupShapes","sim.groupShapes",
    "simUngroupShape","sim.ungroupShape",
    "simConvexDecompose","sim.convexDecompose",
    "simAddGhost","sim.addGhost",
    "simModifyGhost","sim.modifyGhost",
    "simQuitSimulator","sim.quitSimulator",
    "simGetThreadId","sim.getThreadId",
    "simSetShapeMaterial","sim.setShapeMaterial",
    "simGetTextureId","sim.getTextureId",
    "simReadTexture","sim.readTexture",
    "simWriteTexture","sim.writeTexture",
    "simCreateTexture","sim.createTexture",
    "simWriteCustomDataBlock","sim.writeCustomDataBlock",
    "simReadCustomDataBlock","sim.readCustomDataBlock",
    "simReadCustomDataBlockTags","sim.readCustomDataBlockTags",
    "simAddPointCloud","sim.addPointCloud",
    "simModifyPointCloud","sim.modifyPointCloud",
    "simGetShapeGeomInfo","sim.getShapeGeomInfo",
    "simGetObjectsInTree","sim.getObjectsInTree",
    "simSetObjectSizeValues","sim.setObjectSizeValues",
    "simGetObjectSizeValues","sim.getObjectSizeValues",
    "simScaleObject","sim.scaleObject",
    "simSetShapeTexture","sim.setShapeTexture",
    "simGetShapeTextureId","sim.getShapeTextureId",
    "simGetCollectionObjects","sim.getCollectionObjects",
    "simHandleCustomizationScripts","sim.handleCustomizationScripts",
    "simSetScriptAttribute","sim.setScriptAttribute",
    "simGetScriptAttribute","sim.getScriptAttribute",
    "simHandleChildScripts","sim.handleChildScripts",
    "simLaunchThreadedChildScripts","sim.launchThreadedChildScripts",
    "simReorientShapeBoundingBox","sim.reorientShapeBoundingBox",
    "simHandleVisionSensor","sim.handleVisionSensor",
    "simReadVisionSensor","sim.readVisionSensor",
    "simResetVisionSensor","sim.resetVisionSensor",
    "simGetVisionSensorResolution","sim.getVisionSensorResolution",
    "simGetVisionSensorImage","sim.getVisionSensorImage",
    "simSetVisionSensorImage","sim.setVisionSensorImage",
    "simGetVisionSensorCharImage","sim.getVisionSensorCharImage",
    "simSetVisionSensorCharImage","sim.setVisionSensorCharImage",
    "simGetVisionSensorDepthBuffer","sim.getVisionSensorDepthBuffer",
    "simCheckVisionSensor","sim.checkVisionSensor",
    "simCheckVisionSensorEx","sim.checkVisionSensorEx",
    "simBuildMatrixQ","sim.buildMatrixQ",
    "simGetQuaternionFromMatrix","sim.getQuaternionFromMatrix",
    "simFileDialog","sim.fileDialog",
    "simMsgBox","sim.msgBox",
    "simLoadModule","sim.loadModule",
    "simUnloadModule","sim.unloadModule",
    "simCallScriptFunction","sim.callScriptFunction",
    "simGetConfigForTipPose","sim.getConfigForTipPose",
    "simGenerateIkPath","sim.generateIkPath",
    "simGetExtensionString","sim.getExtensionString",
    "simComputeMassAndInertia","sim.computeMassAndInertia",
    "simSetScriptVariable","sim.setScriptVariable",
    "simGetEngineFloatParameter","sim.getEngineFloatParameter",
    "simGetEngineInt32Parameter","sim.getEngineInt32Parameter",
    "simGetEngineBoolParameter","sim.getEngineBoolParameter",
    "simSetEngineFloatParameter","sim.setEngineFloatParameter",
    "simSetEngineInt32Parameter","sim.setEngineInt32Parameter",
    "simSetEngineBoolParameter","sim.setEngineBoolParameter",
    "simCreateOctree","sim.createOctree",
    "simCreatePointCloud","sim.createPointCloud",
    "simSetPointCloudOptions","sim.setPointCloudOptions",
    "simGetPointCloudOptions","sim.getPointCloudOptions",
    "simInsertVoxelsIntoOctree","sim.insertVoxelsIntoOctree",
    "simRemoveVoxelsFromOctree","sim.removeVoxelsFromOctree",
    "simInsertPointsIntoPointCloud","sim.insertPointsIntoPointCloud",
    "simRemovePointsFromPointCloud","sim.removePointsFromPointCloud",
    "simIntersectPointsWithPointCloud","sim.intersectPointsWithPointCloud",
    "simGetOctreeVoxels","sim.getOctreeVoxels",
    "simGetPointCloudPoints","sim.getPointCloudPoints",
    "simInsertObjectIntoOctree","sim.insertObjectIntoOctree",
    "simSubtractObjectFromOctree","sim.subtractObjectFromOctree",
    "simInsertObjectIntoPointCloud","sim.insertObjectIntoPointCloud",
    "simSubtractObjectFromPointCloud","sim.subtractObjectFromPointCloud",
    "simCheckOctreePointOccupancy","sim.checkOctreePointOccupancy",
    "simOpenTextEditor","sim.openTextEditor",
    "simSetVisionSensorFilter","sim.setVisionSensorFilter",
    "simGetVisionSensorFilter","sim.getVisionSensorFilter",
    "simHandleSimulationStart","sim.handleSimulationStart",
    "simHandleSensingStart","sim.handleSensingStart",
    "simAuxFunc","sim.auxFunc",
    "simSetReferencedHandles","sim.setReferencedHandles",
    "simGetReferencedHandles","sim.getReferencedHandles",
    "simGetGraphCurve","sim.getGraphCurveData",
    "simTest","sim.test",
    "simRMLPos","sim.rmlPos",
    "simRMLVel","sim.rmlVel",
    "simRMLStep","sim.rmlStep",
    "simRMLRemove","sim.rmlRemove",
    "simRMLMoveToPosition","sim.rmlMoveToPosition",
    "simRMLMoveToJointPositions","sim.rmlMoveToJointPositions",
    "simBoolOr16","sim.boolOr32",
    "simBoolAnd16","sim.boolAnd32",
    "simBoolXor16","sim.boolXor32",
    "simPackInts","sim.packInt32Table",
    "simPackUInts","sim.packUInt32Table",
    "simPackFloats","sim.packFloatTable",
    "simPackDoubles","sim.packDoubleTable",
    "simPackBytes","sim.packUInt8Table",
    "simPackWords","sim.packUInt16Table",
    "simUnpackInts","sim.unpackInt32Table",
    "simUnpackUInts","sim.unpackUInt32Table",
    "simUnpackFloats","sim.unpackFloatTable",
    "simUnpackDoubles","sim.unpackDoubleTable",
    "simUnpackBytes","sim.unpackUInt8Table",
    "simUnpackWords","sim.unpackUInt16Table",
    "simSetBooleanParameter","sim.setBoolParameter",
    "simGetBooleanParameter","sim.getBoolParameter",
    "simSetIntegerParameter","sim.setInt32Parameter",
    "simGetIntegerParameter","sim.getInt32Parameter",
    "simSetFloatingParameter","sim.setFloatParameter",
    "simGetFloatingParameter","sim.getFloatParameter",
    "simGetObjectIntParameter","sim.getObjectInt32Parameter",
    "simSetObjectIntParameter","sim.setObjectInt32Parameter",
    "sim_object_shape_type","sim.object_shape_type",
    "sim_object_joint_type","sim.object_joint_type",
    "sim_object_graph_type","sim.object_graph_type",
    "sim_object_camera_type","sim.object_camera_type",
    "sim_object_dummy_type","sim.object_dummy_type",
    "sim_object_proximitysensor_type","sim.object_proximitysensor_type",
    "sim_object_path_type","sim.object_path_type",
    "sim_object_renderingsensor_type","sim.object_renderingsensor_type",
    "sim_object_visionsensor_type","sim.object_visionsensor_type",
    "sim_object_mill_type","sim.object_mill_type",
    "sim_object_forcesensor_type","sim.object_forcesensor_type",
    "sim_object_light_type","sim.object_light_type",
    "sim_object_mirror_type","sim.object_mirror_type",
    "sim_object_octree_type","sim.object_octree_type",
    "sim_object_pointcloud_type","sim.object_pointcloud_type",
    "sim_light_omnidirectional_subtype","sim.light_omnidirectional_subtype",
    "sim_light_spot_subtype","sim.light_spot_subtype",
    "sim_light_directional_subtype","sim.light_directional_subtype",
    "sim_joint_revolute_subtype","sim.joint_revolute_subtype",
    "sim_joint_prismatic_subtype","sim.joint_prismatic_subtype",
    "sim_joint_spherical_subtype","sim.joint_spherical_subtype",
    "sim_shape_simpleshape_subtype","sim.shape_simpleshape_subtype",
    "sim_shape_multishape_subtype","sim.shape_multishape_subtype",
    "sim_proximitysensor_pyramid_subtype","sim.proximitysensor_pyramid_subtype",
    "sim_proximitysensor_cylinder_subtype","sim.proximitysensor_cylinder_subtype",
    "sim_proximitysensor_disc_subtype","sim.proximitysensor_disc_subtype",
    "sim_proximitysensor_cone_subtype","sim.proximitysensor_cone_subtype",
    "sim_proximitysensor_ray_subtype","sim.proximitysensor_ray_subtype",
    "sim_mill_pyramid_subtype","sim.mill_pyramid_subtype",
    "sim_mill_cylinder_subtype","sim.mill_cylinder_subtype",
    "sim_mill_disc_subtype","sim.mill_disc_subtype",
    "sim_mill_cone_subtype","sim.mill_cone_subtype",
    "sim_object_no_subtype","sim.object_no_subtype",
    "sim_appobj_object_type","sim.appobj_object_type",
    "sim_appobj_collision_type","sim.appobj_collision_type",
    "sim_appobj_distance_type","sim.appobj_distance_type",
    "sim_appobj_simulation_type","sim.appobj_simulation_type",
    "sim_appobj_ik_type","sim.appobj_ik_type",
    "sim_appobj_constraintsolver_type","sim.appobj_constraintsolver_type",
    "sim_appobj_collection_type","sim.appobj_collection_type",
    "sim_appobj_2delement_type","sim.appobj_2delement_type",
    "sim_appobj_ui_type","sim.appobj_ui_type",
    "sim_appobj_script_type","sim.appobj_script_type",
    "sim_appobj_pathplanning_type","sim.appobj_pathplanning_type",
    "sim_appobj_texture_type","sim.appobj_texture_type",
    "sim_appobj_motionplanning_type","sim.appobj_motionplanning_type",
    "sim_ik_pseudo_inverse_method","sim.ik_pseudo_inverse_method",
    "sim_ik_damped_least_squares_method","sim.ik_damped_least_squares_method",
    "sim_ik_jacobian_transpose_method","sim.ik_jacobian_transpose_method",
    "sim_ik_x_constraint","sim.ik_x_constraint",
    "sim_ik_y_constraint","sim.ik_y_constraint",
    "sim_ik_z_constraint","sim.ik_z_constraint",
    "sim_ik_alpha_beta_constraint","sim.ik_alpha_beta_constraint",
    "sim_ik_gamma_constraint","sim.ik_gamma_constraint",
    "sim_ikresult_not_performed","sim.ikresult_not_performed",
    "sim_ikresult_success","sim.ikresult_success",
    "sim_ikresult_fail","sim.ikresult_fail",
    "sim_message_ui_button_state_change","sim.message_ui_button_state_change",
    "sim_message_model_loaded","sim.message_model_loaded",
    "sim_message_scene_loaded","sim.message_scene_loaded",
    "sim_message_object_selection_changed","sim.message_object_selection_changed",
    "sim_message_keypress","sim.message_keypress",
    "sim_message_bannerclicked","sim.message_bannerclicked",
    "sim_message_prox_sensor_select_down","sim.message_prox_sensor_select_down",
    "sim_message_prox_sensor_select_up","sim.message_prox_sensor_select_up",
    "sim_message_pick_select_down","sim.message_pick_select_down",
    "sim_buttonproperty_button","sim.buttonproperty_button",
    "sim_buttonproperty_label","sim.buttonproperty_label",
    "sim_buttonproperty_editbox","sim.buttonproperty_editbox",
    "sim_buttonproperty_slider","sim.buttonproperty_slider",
    "sim_buttonproperty_staydown","sim.buttonproperty_staydown",
    "sim_buttonproperty_enabled","sim.buttonproperty_enabled",
    "sim_buttonproperty_borderless","sim.buttonproperty_borderless",
    "sim_buttonproperty_horizontallycentered","sim.buttonproperty_horizontallycentered",
    "sim_buttonproperty_ignoremouse","sim.buttonproperty_ignoremouse",
    "sim_buttonproperty_isdown","sim.buttonproperty_isdown",
    "sim_buttonproperty_transparent","sim.buttonproperty_transparent",
    "sim_buttonproperty_nobackgroundcolor","sim.buttonproperty_nobackgroundcolor",
    "sim_buttonproperty_rollupaction","sim.buttonproperty_rollupaction",
    "sim_buttonproperty_closeaction","sim.buttonproperty_closeaction",
    "sim_buttonproperty_verticallycentered","sim.buttonproperty_verticallycentered",
    "sim_buttonproperty_downupevent","sim.buttonproperty_downupevent",
    "sim_objectproperty_collapsed","sim.objectproperty_collapsed",
    "sim_objectproperty_selectable","sim.objectproperty_selectable",
    "sim_objectproperty_selectmodelbaseinstead","sim.objectproperty_selectmodelbaseinstead",
    "sim_objectproperty_dontshowasinsidemodel","sim.objectproperty_dontshowasinsidemodel",
    "sim_objectproperty_canupdatedna","sim.objectproperty_canupdatedna",
    "sim_objectproperty_selectinvisible","sim.objectproperty_selectinvisible",
    "sim_objectproperty_depthinvisible","sim.objectproperty_depthinvisible",
    "sim_objectproperty_cannotdelete","sim.objectproperty_cannotdelete",
    "sim_simulation_stopped","sim.simulation_stopped",
    "sim_simulation_paused","sim.simulation_paused",
    "sim_simulation_advancing","sim.simulation_advancing",
    "sim_simulation_advancing_firstafterstop","sim.simulation_advancing_firstafterstop",
    "sim_simulation_advancing_running","sim.simulation_advancing_running",
    "sim_simulation_advancing_lastbeforepause","sim.simulation_advancing_lastbeforepause",
    "sim_simulation_advancing_firstafterpause","sim.simulation_advancing_firstafterpause",
    "sim_simulation_advancing_abouttostop","sim.simulation_advancing_abouttostop",
    "sim_simulation_advancing_lastbeforestop","sim.simulation_advancing_lastbeforestop",
    "sim_texturemap_plane","sim.texturemap_plane",
    "sim_texturemap_cylinder","sim.texturemap_cylinder",
    "sim_texturemap_sphere","sim.texturemap_sphere",
    "sim_texturemap_cube","sim.texturemap_cube",
    "sim_scripttype_mainscript","sim.scripttype_mainscript",
    "sim_scripttype_childscript","sim.scripttype_childscript",
    "sim_scripttype_addonscript","sim.scripttype_addonscript",
    "sim_scripttype_addonfunction","sim.scripttype_addonfunction",
    "sim_scripttype_customizationscript","sim.scripttype_customizationscript",
    "sim_scripttype_threaded","sim.scripttype_threaded",
    "sim_mainscriptcall_initialization","sim.syscb_init",
    "sim_mainscriptcall_cleanup","sim.syscb_cleanup",
    "sim_mainscriptcall_regular","sim.syscb_regular",
    "sim_childscriptcall_initialization","sim.syscb_init",
    "sim_childscriptcall_cleanup","sim.syscb_cleanup",
    "sim_childscriptcall_actuation","sim.syscb_actuation",
    "sim_childscriptcall_sensing","sim.syscb_sensing",
    "sim_childscriptcall_threaded","-1",
    "sim_customizationscriptcall_initialization","sim.syscb_init",
    "sim_customizationscriptcall_cleanup","sim.syscb_cleanup",
    "sim_customizationscriptcall_nonsimulation","sim.syscb_nonsimulation",
    "sim_customizationscriptcall_lastbeforesimulation","sim.syscb_beforesimulation",
    "sim_customizationscriptcall_firstaftersimulation","sim.syscb_aftersimulation",
    "sim_customizationscriptcall_simulationactuation","sim.syscb_actuation",
    "sim_customizationscriptcall_simulationsensing","sim.syscb_sensing",
    "sim_customizationscriptcall_simulationpause","sim.syscb_suspended",
    "sim_customizationscriptcall_simulationpausefirst","sim.syscb_suspend",
    "sim_customizationscriptcall_simulationpauselast","sim.syscb_resume",
    "sim_customizationscriptcall_lastbeforeinstanceswitch","sim.syscb_beforeinstanceswitch",
    "sim_customizationscriptcall_firstafterinstanceswitch","sim.syscb_afterinstanceswitch",
    "sim_addonscriptcall_initialization","sim.syscb_init",
    "sim_addonscriptcall_run","sim.syscb_run",
    "sim_addonscriptcall_suspend","sim.syscb_suspend",
    "sim_addonscriptcall_restarting","sim.syscb_resume",
    "sim_addonscriptcall_cleanup","sim.syscb_cleanup",
    "sim_customizationscriptattribute_activeduringsimulation","sim.customizationscriptattribute_activeduringsimulation",
    "sim_scriptattribute_executionorder","sim.scriptattribute_executionorder",
    "sim_scriptattribute_executioncount","sim.scriptattribute_executioncount",
    "sim_childscriptattribute_automaticcascadingcalls","sim.childscriptattribute_automaticcascadingcalls",
    "sim_childscriptattribute_enabled","sim.childscriptattribute_enabled",
    "sim_scriptattribute_enabled","sim.scriptattribute_enabled",
    "sim_customizationscriptattribute_cleanupbeforesave","sim.customizationscriptattribute_cleanupbeforesave",
    "sim_scriptexecorder_first","sim.scriptexecorder_first",
    "sim_scriptexecorder_normal","sim.scriptexecorder_normal",
    "sim_scriptexecorder_last","sim.scriptexecorder_last",
    "sim_scriptthreadresume_allnotyetresumed","sim.scriptthreadresume_allnotyetresumed",
    "sim_scriptthreadresume_default","sim.scriptthreadresume_default",
    "sim_scriptthreadresume_actuation_first","sim.scriptthreadresume_actuation_first",
    "sim_scriptthreadresume_actuation_last","sim.scriptthreadresume_actuation_last",
    "sim_scriptthreadresume_sensing_first","sim.scriptthreadresume_sensing_first",
    "sim_scriptthreadresume_sensing_last","sim.scriptthreadresume_sensing_last",
    "sim_scriptthreadresume_custom","sim.scriptthreadresume_custom",
    "sim_callbackid_rossubscriber","sim.callbackid_rossubscriber",
    "sim_callbackid_dynstep","sim.callbackid_dynstep",
    "sim_callbackid_userdefined","sim.callbackid_userdefined",
    "sim_script_no_error","sim.script_no_error",
    "sim_script_main_script_nonexistent","sim.script_main_script_nonexistent",
    "sim_script_main_not_called","sim.script_main_not_called",
    "sim_script_reentrance_error","sim.script_reentrance_error",
    "sim_script_lua_error","sim.script_lua_error",
    "sim_script_call_error","sim.script_call_error",
    "sim_api_error_report","sim.api_error_report",
    "sim_api_error_output","sim.api_error_output",
    "sim_api_warning_output","sim.api_warning_output",
    "sim_handle_all","sim.handle_all",
    "sim_handle_all_except_explicit","sim.handle_all_except_explicit",
    "sim_handle_self","sim.handle_self",
    "sim_handle_main_script","sim.handle_main_script",
    "sim_handle_tree","sim.handle_tree",
    "sim_handle_chain","sim.handle_chain",
    "sim_handle_single","sim.handle_single",
    "sim_handle_default","sim.handle_default",
    "sim_handle_all_except_self","sim.handle_all_except_self",
    "sim_handle_parent","sim.handle_parent",
    "sim_handle_scene","sim.handle_scene",
    "sim_handle_app","sim.handle_app",
    "sim_handleflag_assembly","sim.handleflag_assembly",
    "sim_handleflag_camera","sim.handleflag_camera",
    "sim_handleflag_togglevisibility","sim.handleflag_togglevisibility",
    "sim_handleflag_extended","sim.handleflag_extended",
    "sim_handleflag_greyscale","sim.handleflag_greyscale",
    "sim_handleflag_codedstring","sim.handleflag_codedstring",
    "sim_handleflag_wxyzquaternion","sim.handleflag_wxyzquaternion",
    "sim_handleflag_model","sim.handleflag_model",
    "sim_handleflag_rawvalue","sim.handleflag_rawvalue",
    "sim_objectspecialproperty_collidable","sim.objectspecialproperty_collidable",
    "sim_objectspecialproperty_measurable","sim.objectspecialproperty_measurable",
    "sim_objectspecialproperty_detectable_ultrasonic","sim.objectspecialproperty_detectable_ultrasonic",
    "sim_objectspecialproperty_detectable_infrared","sim.objectspecialproperty_detectable_infrared",
    "sim_objectspecialproperty_detectable_laser","sim.objectspecialproperty_detectable_laser",
    "sim_objectspecialproperty_detectable_inductive","sim.objectspecialproperty_detectable_inductive",
    "sim_objectspecialproperty_detectable_capacitive","sim.objectspecialproperty_detectable_capacitive",
    "sim_objectspecialproperty_renderable","sim.objectspecialproperty_renderable",
    "sim_objectspecialproperty_detectable_all","sim.objectspecialproperty_detectable_all",
    "sim_objectspecialproperty_pathplanning_ignored","sim.objectspecialproperty_pathplanning_ignored",
    "sim_modelproperty_not_collidable","sim.modelproperty_not_collidable",
    "sim_modelproperty_not_measurable","sim.modelproperty_not_measurable",
    "sim_modelproperty_not_renderable","sim.modelproperty_not_renderable",
    "sim_modelproperty_not_detectable","sim.modelproperty_not_detectable",
    "sim_modelproperty_not_dynamic","sim.modelproperty_not_dynamic",
    "sim_modelproperty_not_respondable","sim.modelproperty_not_respondable",
    "sim_modelproperty_not_reset","sim.modelproperty_not_reset",
    "sim_modelproperty_not_visible","sim.modelproperty_not_visible",
    "sim_modelproperty_scripts_inactive","sim.modelproperty_scripts_inactive",
    "sim_modelproperty_not_showasinsidemodel","sim.modelproperty_not_showasinsidemodel",
    "sim_modelproperty_not_model","sim.modelproperty_not_model",
    "sim_dlgstyle_message","sim.dlgstyle_message",
    "sim_dlgstyle_input","sim.dlgstyle_input",
    "sim_dlgstyle_ok","sim.dlgstyle_ok",
    "sim_dlgstyle_ok_cancel","sim.dlgstyle_ok_cancel",
    "sim_dlgstyle_yes_no","sim.dlgstyle_yes_no",
    "sim_dlgstyle_dont_center","sim.dlgstyle_dont_center",
    "sim_dlgret_still_open","sim.dlgret_still_open",
    "sim_dlgret_ok","sim.dlgret_ok",
    "sim_dlgret_cancel","sim.dlgret_cancel",
    "sim_dlgret_yes","sim.dlgret_yes",
    "sim_dlgret_no","sim.dlgret_no",
    "sim_pathproperty_show_line","sim.pathproperty_show_line",
    "sim_pathproperty_show_orientation","sim.pathproperty_show_orientation",
    "sim_pathproperty_closed_path","sim.pathproperty_closed_path",
    "sim_pathproperty_automatic_orientation","sim.pathproperty_automatic_orientation",
    "sim_pathproperty_flat_path","sim.pathproperty_flat_path",
    "sim_pathproperty_show_position","sim.pathproperty_show_position",
    "sim_pathproperty_keep_x_up","sim.pathproperty_keep_x_up",
    "sim_distcalcmethod_dl","sim.distcalcmethod_dl",
    "sim_distcalcmethod_dac","sim.distcalcmethod_dac",
    "sim_distcalcmethod_max_dl_dac","sim.distcalcmethod_max_dl_dac",
    "sim_distcalcmethod_dl_and_dac","sim.distcalcmethod_dl_and_dac",
    "sim_distcalcmethod_sqrt_dl2_and_dac2","sim.distcalcmethod_sqrt_dl2_and_dac2",
    "sim_distcalcmethod_dl_if_nonzero","sim.distcalcmethod_dl_if_nonzero",
    "sim_distcalcmethod_dac_if_nonzero","sim.distcalcmethod_dac_if_nonzero",
    "sim_boolparam_hierarchy_visible","sim.boolparam_hierarchy_visible",
    "sim_boolparam_console_visible","sim.boolparam_console_visible",
    "sim_boolparam_collision_handling_enabled","sim.boolparam_collision_handling_enabled",
    "sim_boolparam_distance_handling_enabled","sim.boolparam_distance_handling_enabled",
    "sim_boolparam_ik_handling_enabled","sim.boolparam_ik_handling_enabled",
    "sim_boolparam_gcs_handling_enabled","sim.boolparam_gcs_handling_enabled",
    "sim_boolparam_dynamics_handling_enabled","sim.boolparam_dynamics_handling_enabled",
    "sim_boolparam_proximity_sensor_handling_enabled","sim.boolparam_proximity_sensor_handling_enabled",
    "sim_boolparam_vision_sensor_handling_enabled","sim.boolparam_vision_sensor_handling_enabled",
    "sim_boolparam_rendering_sensor_handling_enabled","sim.boolparam_rendering_sensor_handling_enabled",
    "sim_boolparam_mill_handling_enabled","sim.boolparam_mill_handling_enabled",
    "sim_boolparam_browser_visible","sim.boolparam_browser_visible",
    "sim_boolparam_scene_and_model_load_messages","sim.boolparam_scene_and_model_load_messages",
    "sim_boolparam_shape_textures_are_visible","sim.boolparam_shape_textures_are_visible",
    "sim_boolparam_display_enabled","sim.boolparam_display_enabled",
    "sim_boolparam_infotext_visible","sim.boolparam_infotext_visible",
    "sim_boolparam_statustext_open","sim.boolparam_statustext_open",
    "sim_boolparam_fog_enabled","sim.boolparam_fog_enabled",
    "sim_boolparam_rml2_available","sim.boolparam_rml2_available",
    "sim_boolparam_rml4_available","sim.boolparam_rml4_available",
    "sim_boolparam_mirrors_enabled","sim.boolparam_mirrors_enabled",
    "sim_boolparam_aux_clip_planes_enabled","sim.boolparam_aux_clip_planes_enabled",
    "sim_boolparam_full_model_copy_from_api","sim.boolparam_full_model_copy_from_api",
    "sim_boolparam_realtime_simulation","sim.boolparam_realtime_simulation",
    "sim_boolparam_use_glfinish_cmd","sim.boolparam_use_glfinish_cmd",
    "sim_boolparam_force_show_wireless_emission","sim.boolparam_force_show_wireless_emission",
    "sim_boolparam_force_show_wireless_reception","sim.boolparam_force_show_wireless_reception",
    "sim_boolparam_video_recording_triggered","sim.boolparam_video_recording_triggered",
    "sim_boolparam_fullscreen","sim.boolparam_fullscreen",
    "sim_boolparam_headless","sim.boolparam_headless",
    "sim_boolparam_hierarchy_toolbarbutton_enabled","sim.boolparam_hierarchy_toolbarbutton_enabled",
    "sim_boolparam_browser_toolbarbutton_enabled","sim.boolparam_browser_toolbarbutton_enabled",
    "sim_boolparam_objectshift_toolbarbutton_enabled","sim.boolparam_objectshift_toolbarbutton_enabled",
    "sim_boolparam_objectrotate_toolbarbutton_enabled","sim.boolparam_objectrotate_toolbarbutton_enabled",
    "sim_boolparam_force_calcstruct_all_visible","sim.boolparam_force_calcstruct_all_visible",
    "sim_boolparam_force_calcstruct_all","sim.boolparam_force_calcstruct_all",
    "sim_boolparam_exit_request","sim.boolparam_exit_request",
    "sim_boolparam_play_toolbarbutton_enabled","sim.boolparam_play_toolbarbutton_enabled",
    "sim_boolparam_pause_toolbarbutton_enabled","sim.boolparam_pause_toolbarbutton_enabled",
    "sim_boolparam_stop_toolbarbutton_enabled","sim.boolparam_stop_toolbarbutton_enabled",
    "sim_boolparam_waiting_for_trigger","sim.boolparam_waiting_for_trigger",
    "sim_boolparam_objproperties_toolbarbutton_enabled","sim.boolparam_objproperties_toolbarbutton_enabled",
    "sim_boolparam_calcmodules_toolbarbutton_enabled","sim.boolparam_calcmodules_toolbarbutton_enabled",
    "sim_boolparam_rosinterface_donotrunmainscript","sim.boolparam_rosinterface_donotrunmainscript",
    "sim_intparam_error_report_mode","sim.intparam_error_report_mode",
    "sim_intparam_program_version","sim.intparam_program_version",
    "sim_intparam_compilation_version","sim.intparam_compilation_version",
    "sim_intparam_current_page","sim.intparam_current_page",
    "sim_intparam_flymode_camera_handle","sim.intparam_flymode_camera_handle",
    "sim_intparam_dynamic_step_divider","sim.intparam_dynamic_step_divider",
    "sim_intparam_dynamic_engine","sim.intparam_dynamic_engine",
    "sim_intparam_server_port_start","sim.intparam_server_port_start",
    "sim_intparam_server_port_range","sim.intparam_server_port_range",
    "sim_intparam_server_port_next","sim.intparam_server_port_next",
    "sim_intparam_visible_layers","sim.intparam_visible_layers",
    "sim_intparam_infotext_style","sim.intparam_infotext_style",
    "sim_intparam_settings","sim.intparam_settings",
    "sim_intparam_qt_version","sim.intparam_qt_version",
    "sim_intparam_event_flags_read","sim.intparam_event_flags_read",
    "sim_intparam_event_flags_read_clear","sim.intparam_event_flags_read_clear",
    "sim_intparam_platform","sim.intparam_platform",
    "sim_intparam_scene_unique_id","sim.intparam_scene_unique_id",
    "sim_intparam_edit_mode_type","sim.intparam_edit_mode_type",
    "sim_intparam_work_thread_count","sim.intparam_work_thread_count",
    "sim_intparam_mouse_x","sim.intparam_mouse_x",
    "sim_intparam_mouse_y","sim.intparam_mouse_y",
    "sim_intparam_core_count","sim.intparam_core_count",
    "sim_intparam_work_thread_calc_time_ms","sim.intparam_work_thread_calc_time_ms",
    "sim_intparam_idle_fps","sim.intparam_idle_fps",
    "sim_intparam_prox_sensor_select_down","sim.intparam_prox_sensor_select_down",
    "sim_intparam_prox_sensor_select_up","sim.intparam_prox_sensor_select_up",
    "sim_intparam_stop_request_counter","sim.intparam_stop_request_counter",
    "sim_intparam_program_revision","sim.intparam_program_revision",
    "sim_intparam_mouse_buttons","sim.intparam_mouse_buttons",
    "sim_intparam_dynamic_warning_disabled_mask","sim.intparam_dynamic_warning_disabled_mask",
    "sim_intparam_simulation_warning_disabled_mask","sim.intparam_simulation_warning_disabled_mask",
    "sim_intparam_scene_index","sim.intparam_scene_index",
    "sim_intparam_motionplanning_seed","sim.intparam_motionplanning_seed",
    "sim_intparam_speedmodifier","sim.intparam_speedmodifier",
    "sim_intparam_dynamic_iteration_count","sim.intparam_dynamic_iteration_count",
    "sim_floatparam_rand","sim.floatparam_rand",
    "sim_floatparam_simulation_time_step","sim.floatparam_simulation_time_step",
    "sim_floatparam_stereo_distance","sim.floatparam_stereo_distance",
    "sim_floatparam_dynamic_step_size","sim.floatparam_dynamic_step_size",
    "sim_floatparam_mouse_wheel_zoom_factor","sim.floatparam_mouse_wheel_zoom_factor",
    "sim_arrayparam_gravity","sim.arrayparam_gravity",
    "sim_arrayparam_fog","sim.arrayparam_fog",
    "sim_arrayparam_fog_color","sim.arrayparam_fog_color",
    "sim_arrayparam_background_color1","sim.arrayparam_background_color1",
    "sim_arrayparam_background_color2","sim.arrayparam_background_color2",
    "sim_arrayparam_ambient_light","sim.arrayparam_ambient_light",
    "sim_arrayparam_random_euler","sim.arrayparam_random_euler",
    "sim_stringparam_application_path","sim.stringparam_application_path",
    "sim_stringparam_video_filename","sim.stringparam_video_filename",
    "sim_stringparam_app_arg1","sim.stringparam_app_arg1",
    "sim_stringparam_app_arg2","sim.stringparam_app_arg2",
    "sim_stringparam_app_arg3","sim.stringparam_app_arg3",
    "sim_stringparam_app_arg4","sim.stringparam_app_arg4",
    "sim_stringparam_app_arg5","sim.stringparam_app_arg5",
    "sim_stringparam_app_arg6","sim.stringparam_app_arg6",
    "sim_stringparam_app_arg7","sim.stringparam_app_arg7",
    "sim_stringparam_app_arg8","sim.stringparam_app_arg8",
    "sim_stringparam_app_arg9","sim.stringparam_app_arg9",
    "sim_stringparam_scene_path_and_name","sim.stringparam_scene_path_and_name",
    "sim_stringparam_remoteapi_temp_file_dir","sim.stringparam_remoteapi_temp_file_dir",
    "sim_stringparam_scene_path","sim.stringparam_scene_path",
    "sim_stringparam_scene_name","sim.stringparam_scene_name",
    "sim_displayattribute_renderpass","sim.displayattribute_renderpass",
    "sim_displayattribute_depthpass","sim.displayattribute_depthpass",
    "sim_displayattribute_pickpass","sim.displayattribute_pickpass",
    "sim_displayattribute_selected","sim.displayattribute_selected",
    "sim_displayattribute_mainselection","sim.displayattribute_mainselection",
    "sim_displayattribute_forcewireframe","sim.displayattribute_forcewireframe",
    "sim_displayattribute_forbidwireframe","sim.displayattribute_forbidwireframe",
    "sim_displayattribute_forbidedges","sim.displayattribute_forbidedges",
    "sim_displayattribute_originalcolors","sim.displayattribute_originalcolors",
    "sim_displayattribute_ignorelayer","sim.displayattribute_ignorelayer",
    "sim_displayattribute_forvisionsensor","sim.displayattribute_forvisionsensor",
    "sim_displayattribute_colorcodedpickpass","sim.displayattribute_colorcodedpickpass",
    "sim_displayattribute_colorcoded","sim.displayattribute_colorcoded",
    "sim_displayattribute_trianglewireframe","sim.displayattribute_trianglewireframe",
    "sim_displayattribute_thickEdges","sim.displayattribute_thickedges",
    "sim_displayattribute_dynamiccontentonly","sim.displayattribute_dynamiccontentonly",
    "sim_displayattribute_mirror","sim.displayattribute_mirror",
    "sim_displayattribute_useauxcomponent","sim.displayattribute_useauxcomponent",
    "sim_displayattribute_ignorerenderableflag","sim.displayattribute_ignorerenderableflag",
    "sim_displayattribute_noopenglcallbacks","sim.displayattribute_noopenglcallbacks",
    "sim_displayattribute_noghosts","sim.displayattribute_noghosts",
    "sim_displayattribute_nopointclouds","sim.displayattribute_nopointclouds",
    "sim_displayattribute_nodrawingobjects","sim.displayattribute_nodrawingobjects",
    "sim_displayattribute_noparticles","sim.displayattribute_noparticles",
    "sim_displayattribute_colorcodedtriangles","sim.displayattribute_colorcodedtriangles",
    "sim_navigation_passive","sim.navigation_passive",
    "sim_navigation_camerashift","sim.navigation_camerashift",
    "sim_navigation_camerarotate","sim.navigation_camerarotate",
    "sim_navigation_camerazoom","sim.navigation_camerazoom",
    "sim_navigation_cameratilt","sim.navigation_cameratilt",
    "sim_navigation_cameraangle","sim.navigation_cameraangle",
    "sim_navigation_camerafly","sim.navigation_camerafly",
    "sim_navigation_objectshift","sim.navigation_objectshift",
    "sim_navigation_objectrotate","sim.navigation_objectrotate",
    "sim_navigation_createpathpoint","sim.navigation_createpathpoint",
    "sim_navigation_clickselection","sim.navigation_clickselection",
    "sim_navigation_ctrlselection","sim.navigation_ctrlselection",
    "sim_navigation_shiftselection","sim.navigation_shiftselection",
    "sim_navigation_camerazoomwheel","sim.navigation_camerazoomwheel",
    "sim_navigation_camerarotaterightbutton","sim.navigation_camerarotaterightbutton",
    "sim_navigation_camerarotatemiddlebutton","sim.navigation_camerarotatemiddlebutton",
    "sim_drawing_points","sim.drawing_points",
    "sim_drawing_lines","sim.drawing_lines",
    "sim_drawing_triangles","sim.drawing_triangles",
    "sim_drawing_trianglepoints","sim.drawing_trianglepoints",
    "sim_drawing_quadpoints","sim.drawing_quadpoints",
    "sim_drawing_discpoints","sim.drawing_discpoints",
    "sim_drawing_cubepoints","sim.drawing_cubepoints",
    "sim_drawing_spherepoints","sim.drawing_spherepoints",
    "sim_drawing_itemcolors","sim.drawing_itemcolors",
    "sim_drawing_vertexcolors","sim.drawing_vertexcolors",
    "sim_drawing_itemsizes","sim.drawing_itemsizes",
    "sim_drawing_backfaceculling","sim.drawing_backfaceculling",
    "sim_drawing_wireframe","sim.drawing_wireframe",
    "sim_drawing_painttag","sim.drawing_painttag",
    "sim_drawing_followparentvisibility","sim.drawing_followparentvisibility",
    "sim_drawing_cyclic","sim.drawing_cyclic",
    "sim_drawing_50percenttransparency","sim.drawing_50percenttransparency",
    "sim_drawing_25percenttransparency","sim.drawing_25percenttransparency",
    "sim_drawing_12percenttransparency","sim.drawing_12percenttransparency",
    "sim_drawing_emissioncolor","sim.drawing_emissioncolor",
    "sim_drawing_facingcamera","sim.drawing_facingcamera",
    "sim_drawing_overlay","sim.drawing_overlay",
    "sim_drawing_itemtransparency","sim.drawing_itemtransparency",
    "sim_drawing_persistent","sim.drawing_persistent",
    "sim_drawing_auxchannelcolor1","sim.drawing_auxchannelcolor1",
    "sim_drawing_auxchannelcolor2","sim.drawing_auxchannelcolor2",
    "sim_banner_left","sim.banner_left",
    "sim_banner_right","sim.banner_right",
    "sim_banner_nobackground","sim.banner_nobackground",
    "sim_banner_overlay","sim.banner_overlay",
    "sim_banner_followparentvisibility","sim.banner_followparentvisibility",
    "sim_banner_clickselectsparent","sim.banner_clickselectsparent",
    "sim_banner_clicktriggersevent","sim.banner_clicktriggersevent",
    "sim_banner_facingcamera","sim.banner_facingcamera",
    "sim_banner_fullyfacingcamera","sim.banner_fullyfacingcamera",
    "sim_banner_backfaceculling","sim.banner_backfaceculling",
    "sim_banner_keepsamesize","sim.banner_keepsamesize",
    "sim_banner_bitmapfont","sim.banner_bitmapfont",
    "sim_particle_points1","sim.particle_points1",
    "sim_particle_points2","sim.particle_points2",
    "sim_particle_points4","sim.particle_points4",
    "sim_particle_roughspheres","sim.particle_roughspheres",
    "sim_particle_spheres","sim.particle_spheres",
    "sim_particle_respondable1to4","sim.particle_respondable1to4",
    "sim_particle_respondable5to8","sim.particle_respondable5to8",
    "sim_particle_particlerespondable","sim.particle_particlerespondable",
    "sim_particle_ignoresgravity","sim.particle_ignoresgravity",
    "sim_particle_invisible","sim.particle_invisible",
    "sim_particle_painttag","sim.particle_painttag",
    "sim_particle_itemsizes","sim.particle_itemsizes",
    "sim_particle_itemdensities","sim.particle_itemdensities",
    "sim_particle_itemcolors","sim.particle_itemcolors",
    "sim_particle_cyclic","sim.particle_cyclic",
    "sim_particle_emissioncolor","sim.particle_emissioncolor",
    "sim_particle_water","sim.particle_water",
    "sim_jointmode_passive","sim.jointmode_passive",
    "sim_jointmode_ik","sim.jointmode_ik",
    "sim_jointmode_ikdependent","sim.jointmode_ikdependent",
    "sim_jointmode_dependent","sim.jointmode_dependent",
    "sim_jointmode_force","sim.jointmode_force",
    "sim_filedlg_type_load","sim.filedlg_type_load",
    "sim_filedlg_type_save","sim.filedlg_type_save",
    "sim_filedlg_type_load_multiple","sim.filedlg_type_load_multiple",
    "sim_filedlg_type_folder","sim.filedlg_type_folder",
    "sim_msgbox_type_info","sim.msgbox_type_info",
    "sim_msgbox_type_question","sim.msgbox_type_question",
    "sim_msgbox_type_warning","sim.msgbox_type_warning",
    "sim_msgbox_type_critical","sim.msgbox_type_critical",
    "sim_msgbox_buttons_ok","sim.msgbox_buttons_ok",
    "sim_msgbox_buttons_yesno","sim.msgbox_buttons_yesno",
    "sim_msgbox_buttons_yesnocancel","sim.msgbox_buttons_yesnocancel",
    "sim_msgbox_buttons_okcancel","sim.msgbox_buttons_okcancel",
    "sim_msgbox_return_cancel","sim.msgbox_return_cancel",
    "sim_msgbox_return_no","sim.msgbox_return_no",
    "sim_msgbox_return_yes","sim.msgbox_return_yes",
    "sim_msgbox_return_ok","sim.msgbox_return_ok",
    "sim_msgbox_return_error","sim.msgbox_return_error",
    "sim_physics_bullet","sim.physics_bullet",
    "sim_physics_ode","sim.physics_ode",
    "sim_physics_vortex","sim.physics_vortex",
    "sim_physics_newton","sim.physics_newton",
    "sim_pure_primitive_none","sim.pure_primitive_none",
    "sim_pure_primitive_plane","sim.pure_primitive_plane",
    "sim_pure_primitive_disc","sim.pure_primitive_disc",
    "sim_pure_primitive_cuboid","sim.pure_primitive_cuboid",
    "sim_pure_primitive_spheroid","sim.pure_primitive_spheroid",
    "sim_pure_primitive_cylinder","sim.pure_primitive_cylinder",
    "sim_pure_primitive_cone","sim.pure_primitive_cone",
    "sim_pure_primitive_heightfield","sim.pure_primitive_heightfield",
    "sim_dummy_linktype_dynamics_loop_closure","sim.dummy_linktype_dynamics_loop_closure",
    "sim_dummy_linktype_dynamics_force_constraint","sim.dummy_linktype_dynamics_force_constraint",
    "sim_dummy_linktype_gcs_loop_closure","sim.dummy_linktype_gcs_loop_closure",
    "sim_dummy_linktype_gcs_tip","sim.dummy_linktype_gcs_tip",
    "sim_dummy_linktype_gcs_target","sim.dummy_linktype_gcs_target",
    "sim_dummy_linktype_ik_tip_target","sim.dummy_linktype_ik_tip_target",
    "sim_colorcomponent_ambient","sim.colorcomponent_ambient",
    "sim_colorcomponent_ambient_diffuse","sim.colorcomponent_ambient_diffuse",
    "sim_colorcomponent_diffuse","sim.colorcomponent_diffuse",
    "sim_colorcomponent_specular","sim.colorcomponent_specular",
    "sim_colorcomponent_emission","sim.colorcomponent_emission",
    "sim_colorcomponent_transparency","sim.colorcomponent_transparency",
    "sim_colorcomponent_auxiliary","sim.colorcomponent_auxiliary",
    "sim_volume_ray","sim.volume_ray",
    "sim_volume_randomizedray","sim.volume_randomizedray",
    "sim_volume_pyramid","sim.volume_pyramid",
    "sim_volume_cylinder","sim.volume_cylinder",
    "sim_volume_disc","sim.volume_disc",
    "sim_volume_cone","sim.volume_cone",
    "sim_objintparam_visibility_layer","sim.objintparam_visibility_layer",
    "sim_objfloatparam_abs_x_velocity","sim.objfloatparam_abs_x_velocity",
    "sim_objfloatparam_abs_y_velocity","sim.objfloatparam_abs_y_velocity",
    "sim_objfloatparam_abs_z_velocity","sim.objfloatparam_abs_z_velocity",
    "sim_objfloatparam_abs_rot_velocity","sim.objfloatparam_abs_rot_velocity",
    "sim_objfloatparam_objbbox_min_x","sim.objfloatparam_objbbox_min_x",
    "sim_objfloatparam_objbbox_min_y","sim.objfloatparam_objbbox_min_y",
    "sim_objfloatparam_objbbox_min_z","sim.objfloatparam_objbbox_min_z",
    "sim_objfloatparam_objbbox_max_x","sim.objfloatparam_objbbox_max_x",
    "sim_objfloatparam_objbbox_max_y","sim.objfloatparam_objbbox_max_y",
    "sim_objfloatparam_objbbox_max_z","sim.objfloatparam_objbbox_max_z",
    "sim_objfloatparam_modelbbox_min_x","sim.objfloatparam_modelbbox_min_x",
    "sim_objfloatparam_modelbbox_min_y","sim.objfloatparam_modelbbox_min_y",
    "sim_objfloatparam_modelbbox_min_z","sim.objfloatparam_modelbbox_min_z",
    "sim_objfloatparam_modelbbox_max_x","sim.objfloatparam_modelbbox_max_x",
    "sim_objfloatparam_modelbbox_max_y","sim.objfloatparam_modelbbox_max_y",
    "sim_objfloatparam_modelbbox_max_z","sim.objfloatparam_modelbbox_max_z",
    "sim_objintparam_collection_self_collision_indicator","sim.objintparam_collection_self_collision_indicator",
    "sim_objfloatparam_transparency_offset","sim.objfloatparam_transparency_offset",
    "sim_objintparam_child_role","sim.objintparam_child_role",
    "sim_objintparam_parent_role","sim.objintparam_parent_role",
    "sim_objintparam_manipulation_permissions","sim.objintparam_manipulation_permissions",
    "sim_objintparam_illumination_handle","sim.objintparam_illumination_handle",
    "sim_objstringparam_dna","sim.objstringparam_dna",
    "sim_visionfloatparam_near_clipping","sim.visionfloatparam_near_clipping",
    "sim_visionfloatparam_far_clipping","sim.visionfloatparam_far_clipping",
    "sim_visionintparam_resolution_x","sim.visionintparam_resolution_x",
    "sim_visionintparam_resolution_y","sim.visionintparam_resolution_y",
    "sim_visionfloatparam_perspective_angle","sim.visionfloatparam_perspective_angle",
    "sim_visionfloatparam_ortho_size","sim.visionfloatparam_ortho_size",
    "sim_visionintparam_disabled_light_components","sim.visionintparam_disabled_light_components",
    "sim_visionintparam_rendering_attributes","sim.visionintparam_rendering_attributes",
    "sim_visionintparam_entity_to_render","sim.visionintparam_entity_to_render",
    "sim_visionintparam_windowed_size_x","sim.visionintparam_windowed_size_x",
    "sim_visionintparam_windowed_size_y","sim.visionintparam_windowed_size_y",
    "sim_visionintparam_windowed_pos_x","sim.visionintparam_windowed_pos_x",
    "sim_visionintparam_windowed_pos_y","sim.visionintparam_windowed_pos_y",
    "sim_visionintparam_pov_focal_blur","sim.visionintparam_pov_focal_blur",
    "sim_visionfloatparam_pov_blur_distance","sim.visionfloatparam_pov_blur_distance",
    "sim_visionfloatparam_pov_aperture","sim.visionfloatparam_pov_aperture",
    "sim_visionintparam_pov_blur_sampled","sim.visionintparam_pov_blur_sampled",
    "sim_visionintparam_render_mode","sim.visionintparam_render_mode",
    "sim_visionintparam_perspective_operation","sim.visionintparam_perspective_operation",
    "sim_jointintparam_motor_enabled","sim.jointintparam_motor_enabled",
    "sim_jointintparam_ctrl_enabled","sim.jointintparam_ctrl_enabled",
    "sim_jointfloatparam_pid_p","sim.jointfloatparam_pid_p",
    "sim_jointfloatparam_pid_i","sim.jointfloatparam_pid_i",
    "sim_jointfloatparam_pid_d","sim.jointfloatparam_pid_d",
    "sim_jointfloatparam_intrinsic_x","sim.jointfloatparam_intrinsic_x",
    "sim_jointfloatparam_intrinsic_y","sim.jointfloatparam_intrinsic_y",
    "sim_jointfloatparam_intrinsic_z","sim.jointfloatparam_intrinsic_z",
    "sim_jointfloatparam_intrinsic_qx","sim.jointfloatparam_intrinsic_qx",
    "sim_jointfloatparam_intrinsic_qy","sim.jointfloatparam_intrinsic_qy",
    "sim_jointfloatparam_intrinsic_qz","sim.jointfloatparam_intrinsic_qz",
    "sim_jointfloatparam_intrinsic_qw","sim.jointfloatparam_intrinsic_qw",
    "sim_jointfloatparam_velocity","sim.jointfloatparam_velocity",
    "sim_jointfloatparam_spherical_qx","sim.jointfloatparam_spherical_qx",
    "sim_jointfloatparam_spherical_qy","sim.jointfloatparam_spherical_qy",
    "sim_jointfloatparam_spherical_qz","sim.jointfloatparam_spherical_qz",
    "sim_jointfloatparam_spherical_qw","sim.jointfloatparam_spherical_qw",
    "sim_jointfloatparam_upper_limit","sim.jointfloatparam_upper_limit",
    "sim_jointfloatparam_kc_k","sim.jointfloatparam_kc_k",
    "sim_jointfloatparam_kc_c","sim.jointfloatparam_kc_c",
    "sim_jointfloatparam_ik_weight","sim.jointfloatparam_ik_weight",
    "sim_jointfloatparam_error_x","sim.jointfloatparam_error_x",
    "sim_jointfloatparam_error_y","sim.jointfloatparam_error_y",
    "sim_jointfloatparam_error_z","sim.jointfloatparam_error_z",
    "sim_jointfloatparam_error_a","sim.jointfloatparam_error_a",
    "sim_jointfloatparam_error_b","sim.jointfloatparam_error_b",
    "sim_jointfloatparam_error_g","sim.jointfloatparam_error_g",
    "sim_jointfloatparam_error_pos","sim.jointfloatparam_error_pos",
    "sim_jointfloatparam_error_angle","sim.jointfloatparam_error_angle",
    "sim_jointintparam_velocity_lock","sim.jointintparam_velocity_lock",
    "sim_jointintparam_vortex_dep_handle","sim.jointintparam_vortex_dep_handle",
    "sim_jointfloatparam_vortex_dep_multiplication","sim.jointfloatparam_vortex_dep_multiplication",
    "sim_jointfloatparam_vortex_dep_offset","sim.jointfloatparam_vortex_dep_offset",
    "sim_shapefloatparam_init_velocity_x","sim.shapefloatparam_init_velocity_x",
    "sim_shapefloatparam_init_velocity_y","sim.shapefloatparam_init_velocity_y",
    "sim_shapefloatparam_init_velocity_z","sim.shapefloatparam_init_velocity_z",
    "sim_shapeintparam_static","sim.shapeintparam_static",
    "sim_shapeintparam_respondable","sim.shapeintparam_respondable",
    "sim_shapefloatparam_mass","sim.shapefloatparam_mass",
    "sim_shapefloatparam_texture_x","sim.shapefloatparam_texture_x",
    "sim_shapefloatparam_texture_y","sim.shapefloatparam_texture_y",
    "sim_shapefloatparam_texture_z","sim.shapefloatparam_texture_z",
    "sim_shapefloatparam_texture_a","sim.shapefloatparam_texture_a",
    "sim_shapefloatparam_texture_b","sim.shapefloatparam_texture_b",
    "sim_shapefloatparam_texture_g","sim.shapefloatparam_texture_g",
    "sim_shapefloatparam_texture_scaling_x","sim.shapefloatparam_texture_scaling_x",
    "sim_shapefloatparam_texture_scaling_y","sim.shapefloatparam_texture_scaling_y",
    "sim_shapeintparam_culling","sim.shapeintparam_culling",
    "sim_shapeintparam_wireframe","sim.shapeintparam_wireframe",
    "sim_shapeintparam_compound","sim.shapeintparam_compound",
    "sim_shapeintparam_convex","sim.shapeintparam_convex",
    "sim_shapeintparam_convex_check","sim.shapeintparam_convex_check",
    "sim_shapeintparam_respondable_mask","sim.shapeintparam_respondable_mask",
    "sim_shapefloatparam_init_velocity_a","sim.shapefloatparam_init_ang_velocity_x",
    "sim_shapefloatparam_init_velocity_b","sim.shapefloatparam_init_ang_velocity_y",
    "sim_shapefloatparam_init_velocity_g","sim.shapefloatparam_init_ang_velocity_z",
    "sim_shapestringparam_color_name","sim.shapestringparam_color_name",
    "sim_shapeintparam_edge_visibility","sim.shapeintparam_edge_visibility",
    "sim_shapefloatparam_shading_angle","sim.shapefloatparam_shading_angle",
    "sim_shapefloatparam_edge_angle","sim.shapefloatparam_edge_angle",
    "sim_shapeintparam_edge_borders_hidden","sim.shapeintparam_edge_borders_hidden",
    "sim_proxintparam_ray_invisibility","sim.proxintparam_ray_invisibility",
    "sim_proxintparam_volume_type","sim.proxintparam_volume_type",
    "sim_proxintparam_entity_to_detect","sim.proxintparam_entity_to_detect",
    "sim_forcefloatparam_error_x","sim.forcefloatparam_error_x",
    "sim_forcefloatparam_error_y","sim.forcefloatparam_error_y",
    "sim_forcefloatparam_error_z","sim.forcefloatparam_error_z",
    "sim_forcefloatparam_error_a","sim.forcefloatparam_error_a",
    "sim_forcefloatparam_error_b","sim.forcefloatparam_error_b",
    "sim_forcefloatparam_error_g","sim.forcefloatparam_error_g",
    "sim_forcefloatparam_error_pos","sim.forcefloatparam_error_pos",
    "sim_forcefloatparam_error_angle","sim.forcefloatparam_error_angle",
    "sim_lightintparam_pov_casts_shadows","sim.lightintparam_pov_casts_shadows",
    "sim_cameraintparam_disabled_light_components","sim.cameraintparam_disabled_light_components",
    "sim_camerafloatparam_perspective_angle","sim.camerafloatparam_perspective_angle",
    "sim_camerafloatparam_ortho_size","sim.camerafloatparam_ortho_size",
    "sim_cameraintparam_rendering_attributes","sim.cameraintparam_rendering_attributes",
    "sim_cameraintparam_pov_focal_blur","sim.cameraintparam_pov_focal_blur",
    "sim_camerafloatparam_pov_blur_distance","sim.camerafloatparam_pov_blur_distance",
    "sim_camerafloatparam_pov_aperture","sim.camerafloatparam_pov_aperture",
    "sim_cameraintparam_pov_blur_samples","sim.cameraintparam_pov_blur_samples",
    "sim_dummyintparam_link_type","sim.dummyintparam_link_type",
    "sim_dummyintparam_follow_path","sim.dummyintparam_follow_path",
    "sim_dummyfloatparam_follow_path_offset","sim.dummyfloatparam_follow_path_offset",
    "sim_millintparam_volume_type","sim.millintparam_volume_type",
    "sim_mirrorfloatparam_width","sim.mirrorfloatparam_width",
    "sim_mirrorfloatparam_height","sim.mirrorfloatparam_height",
    "sim_mirrorfloatparam_reflectance","sim.mirrorfloatparam_reflectance",
    "sim_mirrorintparam_enable","sim.mirrorintparam_enable",
    "sim_bullet_global_stepsize","sim.bullet_global_stepsize",
    "sim_bullet_global_internalscalingfactor","sim.bullet_global_internalscalingfactor",
    "sim_bullet_global_collisionmarginfactor","sim.bullet_global_collisionmarginfactor",
    "sim_bullet_global_constraintsolvingiterations","sim.bullet_global_constraintsolvingiterations",
    "sim_bullet_global_bitcoded","sim.bullet_global_bitcoded",
    "sim_bullet_global_constraintsolvertype","sim.bullet_global_constraintsolvertype",
    "sim_bullet_global_fullinternalscaling","sim.bullet_global_fullinternalscaling",
    "sim_bullet_joint_stoperp","sim.bullet_joint_stoperp",
    "sim_bullet_joint_stopcfm","sim.bullet_joint_stopcfm",
    "sim_bullet_joint_normalcfm","sim.bullet_joint_normalcfm",
    "sim_bullet_body_restitution","sim.bullet_body_restitution",
    "sim_bullet_body_oldfriction","sim.bullet_body_oldfriction",
    "sim_bullet_body_friction","sim.bullet_body_friction",
    "sim_bullet_body_lineardamping","sim.bullet_body_lineardamping",
    "sim_bullet_body_angulardamping","sim.bullet_body_angulardamping",
    "sim_bullet_body_nondefaultcollisionmargingfactor","sim.bullet_body_nondefaultcollisionmargingfactor",
    "sim_bullet_body_nondefaultcollisionmargingfactorconvex","sim.bullet_body_nondefaultcollisionmargingfactorconvex",
    "sim_bullet_body_bitcoded","sim.bullet_body_bitcoded",
    "sim_bullet_body_sticky","sim.bullet_body_sticky",
    "sim_bullet_body_usenondefaultcollisionmargin","sim.bullet_body_usenondefaultcollisionmargin",
    "sim_bullet_body_usenondefaultcollisionmarginconvex","sim.bullet_body_usenondefaultcollisionmarginconvex",
    "sim_bullet_body_autoshrinkconvex","sim.bullet_body_autoshrinkconvex",
    "sim_ode_global_stepsize","sim.ode_global_stepsize",
    "sim_ode_global_internalscalingfactor","sim.ode_global_internalscalingfactor",
    "sim_ode_global_cfm","sim.ode_global_cfm",
    "sim_ode_global_erp","sim.ode_global_erp",
    "sim_ode_global_constraintsolvingiterations","sim.ode_global_constraintsolvingiterations",
    "sim_ode_global_bitcoded","sim.ode_global_bitcoded",
    "sim_ode_global_randomseed","sim.ode_global_randomseed",
    "sim_ode_global_fullinternalscaling","sim.ode_global_fullinternalscaling",
    "sim_ode_global_quickstep","sim.ode_global_quickstep",
    "sim_ode_joint_stoperp","sim.ode_joint_stoperp",
    "sim_ode_joint_stopcfm","sim.ode_joint_stopcfm",
    "sim_ode_joint_bounce","sim.ode_joint_bounce",
    "sim_ode_joint_fudgefactor","sim.ode_joint_fudgefactor",
    "sim_ode_joint_normalcfm","sim.ode_joint_normalcfm",
    "sim_ode_body_friction","sim.ode_body_friction",
    "sim_ode_body_softerp","sim.ode_body_softerp",
    "sim_ode_body_softcfm","sim.ode_body_softcfm",
    "sim_ode_body_lineardamping","sim.ode_body_lineardamping",
    "sim_ode_body_angulardamping","sim.ode_body_angulardamping",
    "sim_ode_body_maxcontacts","sim.ode_body_maxcontacts",
    "sim_vortex_global_stepsize","sim.vortex_global_stepsize",
    "sim_vortex_global_internalscalingfactor","sim.vortex_global_internalscalingfactor",
    "sim_vortex_global_contacttolerance","sim.vortex_global_contacttolerance",
    "sim_vortex_global_constraintlinearcompliance","sim.vortex_global_constraintlinearcompliance",
    "sim_vortex_global_constraintlineardamping","sim.vortex_global_constraintlineardamping",
    "sim_vortex_global_constraintlinearkineticloss","sim.vortex_global_constraintlinearkineticloss",
    "sim_vortex_global_constraintangularcompliance","sim.vortex_global_constraintangularcompliance",
    "sim_vortex_global_constraintangulardamping","sim.vortex_global_constraintangulardamping",
    "sim_vortex_global_constraintangularkineticloss","sim.vortex_global_constraintangularkineticloss",
    "sim_vortex_global_bitcoded","sim.vortex_global_bitcoded",
    "sim_vortex_global_autosleep","sim.vortex_global_autosleep",
    "sim_vortex_global_multithreading","sim.vortex_global_multithreading",
    "sim_vortex_joint_lowerlimitdamping","sim.vortex_joint_lowerlimitdamping",
    "sim_vortex_joint_upperlimitdamping","sim.vortex_joint_upperlimitdamping",
    "sim_vortex_joint_lowerlimitstiffness","sim.vortex_joint_lowerlimitstiffness",
    "sim_vortex_joint_upperlimitstiffness","sim.vortex_joint_upperlimitstiffness",
    "sim_vortex_joint_lowerlimitrestitution","sim.vortex_joint_lowerlimitrestitution",
    "sim_vortex_joint_upperlimitrestitution","sim.vortex_joint_upperlimitrestitution",
    "sim_vortex_joint_lowerlimitmaxforce","sim.vortex_joint_lowerlimitmaxforce",
    "sim_vortex_joint_upperlimitmaxforce","sim.vortex_joint_upperlimitmaxforce",
    "sim_vortex_joint_motorconstraintfrictioncoeff","sim.vortex_joint_motorconstraintfrictioncoeff",
    "sim_vortex_joint_motorconstraintfrictionmaxforce","sim.vortex_joint_motorconstraintfrictionmaxforce",
    "sim_vortex_joint_motorconstraintfrictionloss","sim.vortex_joint_motorconstraintfrictionloss",
    "sim_vortex_joint_p0loss","sim.vortex_joint_p0loss",
    "sim_vortex_joint_p0stiffness","sim.vortex_joint_p0stiffness",
    "sim_vortex_joint_p0damping","sim.vortex_joint_p0damping",
    "sim_vortex_joint_p0frictioncoeff","sim.vortex_joint_p0frictioncoeff",
    "sim_vortex_joint_p0frictionmaxforce","sim.vortex_joint_p0frictionmaxforce",
    "sim_vortex_joint_p0frictionloss","sim.vortex_joint_p0frictionloss",
    "sim_vortex_joint_p1loss","sim.vortex_joint_p1loss",
    "sim_vortex_joint_p1stiffness","sim.vortex_joint_p1stiffness",
    "sim_vortex_joint_p1damping","sim.vortex_joint_p1damping",
    "sim_vortex_joint_p1frictioncoeff","sim.vortex_joint_p1frictioncoeff",
    "sim_vortex_joint_p1frictionmaxforce","sim.vortex_joint_p1frictionmaxforce",
    "sim_vortex_joint_p1frictionloss","sim.vortex_joint_p1frictionloss",
    "sim_vortex_joint_p2loss","sim.vortex_joint_p2loss",
    "sim_vortex_joint_p2stiffness","sim.vortex_joint_p2stiffness",
    "sim_vortex_joint_p2damping","sim.vortex_joint_p2damping",
    "sim_vortex_joint_p2frictioncoeff","sim.vortex_joint_p2frictioncoeff",
    "sim_vortex_joint_p2frictionmaxforce","sim.vortex_joint_p2frictionmaxforce",
    "sim_vortex_joint_p2frictionloss","sim.vortex_joint_p2frictionloss",
    "sim_vortex_joint_a0loss","sim.vortex_joint_a0loss",
    "sim_vortex_joint_a0stiffness","sim.vortex_joint_a0stiffness",
    "sim_vortex_joint_a0damping","sim.vortex_joint_a0damping",
    "sim_vortex_joint_a0frictioncoeff","sim.vortex_joint_a0frictioncoeff",
    "sim_vortex_joint_a0frictionmaxforce","sim.vortex_joint_a0frictionmaxforce",
    "sim_vortex_joint_a0frictionloss","sim.vortex_joint_a0frictionloss",
    "sim_vortex_joint_a1loss","sim.vortex_joint_a1loss",
    "sim_vortex_joint_a1stiffness","sim.vortex_joint_a1stiffness",
    "sim_vortex_joint_a1damping","sim.vortex_joint_a1damping",
    "sim_vortex_joint_a1frictioncoeff","sim.vortex_joint_a1frictioncoeff",
    "sim_vortex_joint_a1frictionmaxforce","sim.vortex_joint_a1frictionmaxforce",
    "sim_vortex_joint_a1frictionloss","sim.vortex_joint_a1frictionloss",
    "sim_vortex_joint_a2loss","sim.vortex_joint_a2loss",
    "sim_vortex_joint_a2stiffness","sim.vortex_joint_a2stiffness",
    "sim_vortex_joint_a2damping","sim.vortex_joint_a2damping",
    "sim_vortex_joint_a2frictioncoeff","sim.vortex_joint_a2frictioncoeff",
    "sim_vortex_joint_a2frictionmaxforce","sim.vortex_joint_a2frictionmaxforce",
    "sim_vortex_joint_a2frictionloss","sim.vortex_joint_a2frictionloss",
    "sim_vortex_joint_dependencyfactor","sim.vortex_joint_dependencyfactor",
    "sim_vortex_joint_dependencyoffset","sim.vortex_joint_dependencyoffset",
    "sim_vortex_joint_bitcoded","sim.vortex_joint_bitcoded",
    "sim_vortex_joint_relaxationenabledbc","sim.vortex_joint_relaxationenabledbc",
    "sim_vortex_joint_frictionenabledbc","sim.vortex_joint_frictionenabledbc",
    "sim_vortex_joint_frictionproportionalbc","sim.vortex_joint_frictionproportionalbc",
    "sim_vortex_joint_objectid","sim.vortex_joint_objectid",
    "sim_vortex_joint_dependentobjectid","sim.vortex_joint_dependentobjectid",
    "sim_vortex_joint_motorfrictionenabled","sim.vortex_joint_motorfrictionenabled",
    "sim_vortex_joint_proportionalmotorfriction","sim.vortex_joint_proportionalmotorfriction",
    "sim_vortex_body_primlinearaxisfriction","sim.vortex_body_primlinearaxisfriction",
    "sim_vortex_body_seclinearaxisfriction","sim.vortex_body_seclinearaxisfriction",
    "sim_vortex_body_primangularaxisfriction","sim.vortex_body_primangularaxisfriction",
    "sim_vortex_body_secangularaxisfriction","sim.vortex_body_secangularaxisfriction",
    "sim_vortex_body_normalangularaxisfriction","sim.vortex_body_normalangularaxisfriction",
    "sim_vortex_body_primlinearaxisstaticfrictionscale","sim.vortex_body_primlinearaxisstaticfrictionscale",
    "sim_vortex_body_seclinearaxisstaticfrictionscale","sim.vortex_body_seclinearaxisstaticfrictionscale",
    "sim_vortex_body_primangularaxisstaticfrictionscale","sim.vortex_body_primangularaxisstaticfrictionscale",
    "sim_vortex_body_secangularaxisstaticfrictionscale","sim.vortex_body_secangularaxisstaticfrictionscale",
    "sim_vortex_body_normalangularaxisstaticfrictionscale","sim.vortex_body_normalangularaxisstaticfrictionscale",
    "sim_vortex_body_compliance","sim.vortex_body_compliance",
    "sim_vortex_body_damping","sim.vortex_body_damping",
    "sim_vortex_body_restitution","sim.vortex_body_restitution",
    "sim_vortex_body_restitutionthreshold","sim.vortex_body_restitutionthreshold",
    "sim_vortex_body_adhesiveforce","sim.vortex_body_adhesiveforce",
    "sim_vortex_body_linearvelocitydamping","sim.vortex_body_linearvelocitydamping",
    "sim_vortex_body_angularvelocitydamping","sim.vortex_body_angularvelocitydamping",
    "sim_vortex_body_primlinearaxisslide","sim.vortex_body_primlinearaxisslide",
    "sim_vortex_body_seclinearaxisslide","sim.vortex_body_seclinearaxisslide",
    "sim_vortex_body_primangularaxisslide","sim.vortex_body_primangularaxisslide",
    "sim_vortex_body_secangularaxisslide","sim.vortex_body_secangularaxisslide",
    "sim_vortex_body_normalangularaxisslide","sim.vortex_body_normalangularaxisslide",
    "sim_vortex_body_primlinearaxisslip","sim.vortex_body_primlinearaxisslip",
    "sim_vortex_body_seclinearaxisslip","sim.vortex_body_seclinearaxisslip",
    "sim_vortex_body_primangularaxisslip","sim.vortex_body_primangularaxisslip",
    "sim_vortex_body_secangularaxisslip","sim.vortex_body_secangularaxisslip",
    "sim_vortex_body_normalangularaxisslip","sim.vortex_body_normalangularaxisslip",
    "sim_vortex_body_autosleeplinearspeedthreshold","sim.vortex_body_autosleeplinearspeedthreshold",
    "sim_vortex_body_autosleeplinearaccelthreshold","sim.vortex_body_autosleeplinearaccelthreshold",
    "sim_vortex_body_autosleepangularspeedthreshold","sim.vortex_body_autosleepangularspeedthreshold",
    "sim_vortex_body_autosleepangularaccelthreshold","sim.vortex_body_autosleepangularaccelthreshold",
    "sim_vortex_body_skinthickness","sim.vortex_body_skinthickness",
    "sim_vortex_body_autoangulardampingtensionratio","sim.vortex_body_autoangulardampingtensionratio",
    "sim_vortex_body_primaxisvectorx","sim.vortex_body_primaxisvectorx",
    "sim_vortex_body_primaxisvectory","sim.vortex_body_primaxisvectory",
    "sim_vortex_body_primaxisvectorz","sim.vortex_body_primaxisvectorz",
    "sim_vortex_body_primlinearaxisfrictionmodel","sim.vortex_body_primlinearaxisfrictionmodel",
    "sim_vortex_body_seclinearaxisfrictionmodel","sim.vortex_body_seclinearaxisfrictionmodel",
    "sim_vortex_body_primangulararaxisfrictionmodel","sim.vortex_body_primangulararaxisfrictionmodel",
    "sim_vortex_body_secmangulararaxisfrictionmodel","sim.vortex_body_secmangulararaxisfrictionmodel",
    "sim_vortex_body_normalmangulararaxisfrictionmodel","sim.vortex_body_normalmangulararaxisfrictionmodel",
    "sim_vortex_body_bitcoded","sim.vortex_body_bitcoded",
    "sim_vortex_body_autosleepsteplivethreshold","sim.vortex_body_autosleepsteplivethreshold",
    "sim_vortex_body_materialuniqueid","sim.vortex_body_materialuniqueid",
    "sim_vortex_body_pureshapesasconvex","sim.vortex_body_pureshapesasconvex",
    "sim_vortex_body_convexshapesasrandom","sim.vortex_body_convexshapesasrandom",
    "sim_vortex_body_randomshapesasterrain","sim.vortex_body_randomshapesasterrain",
    "sim_vortex_body_fastmoving","sim.vortex_body_fastmoving",
    "sim_vortex_body_autoslip","sim.vortex_body_autoslip",
    "sim_vortex_body_seclinaxissameasprimlinaxis","sim.vortex_body_seclinaxissameasprimlinaxis",
    "sim_vortex_body_secangaxissameasprimangaxis","sim.vortex_body_secangaxissameasprimangaxis",
    "sim_vortex_body_normangaxissameasprimangaxis","sim.vortex_body_normangaxissameasprimangaxis",
    "sim_vortex_body_autoangulardamping","sim.vortex_body_autoangulardamping",
    "sim_newton_global_stepsize","sim.newton_global_stepsize",
    "sim_newton_global_contactmergetolerance","sim.newton_global_contactmergetolerance",
    "sim_newton_global_constraintsolvingiterations","sim.newton_global_constraintsolvingiterations",
    "sim_newton_global_bitcoded","sim.newton_global_bitcoded",
    "sim_newton_global_multithreading","sim.newton_global_multithreading",
    "sim_newton_global_exactsolver","sim.newton_global_exactsolver",
    "sim_newton_global_highjointaccuracy","sim.newton_global_highjointaccuracy",
    "sim_newton_joint_dependencyfactor","sim.newton_joint_dependencyfactor",
    "sim_newton_joint_dependencyoffset","sim.newton_joint_dependencyoffset",
    "sim_newton_joint_objectid","sim.newton_joint_objectid",
    "sim_newton_joint_dependentobjectid","sim.newton_joint_dependentobjectid",
    "sim_newton_body_staticfriction","sim.newton_body_staticfriction",
    "sim_newton_body_kineticfriction","sim.newton_body_kineticfriction",
    "sim_newton_body_restitution","sim.newton_body_restitution",
    "sim_newton_body_lineardrag","sim.newton_body_lineardrag",
    "sim_newton_body_angulardrag","sim.newton_body_angulardrag",
    "sim_newton_body_bitcoded","sim.newton_body_bitcoded",
    "sim_newton_body_fastmoving","sim.newton_body_fastmoving",
    "sim_vortex_bodyfrictionmodel_box","sim.vortex_bodyfrictionmodel_box",
    "sim_vortex_bodyfrictionmodel_scaledbox","sim.vortex_bodyfrictionmodel_scaledbox",
    "sim_vortex_bodyfrictionmodel_proplow","sim.vortex_bodyfrictionmodel_proplow",
    "sim_vortex_bodyfrictionmodel_prophigh","sim.vortex_bodyfrictionmodel_prophigh",
    "sim_vortex_bodyfrictionmodel_scaledboxfast","sim.vortex_bodyfrictionmodel_scaledboxfast",
    "sim_vortex_bodyfrictionmodel_neutral","sim.vortex_bodyfrictionmodel_neutral",
    "sim_vortex_bodyfrictionmodel_none","sim.vortex_bodyfrictionmodel_none",
    "sim_bullet_constraintsolvertype_sequentialimpulse","sim.bullet_constraintsolvertype_sequentialimpulse",
    "sim_bullet_constraintsolvertype_nncg","sim.bullet_constraintsolvertype_nncg",
    "sim_bullet_constraintsolvertype_dantzig","sim.bullet_constraintsolvertype_dantzig",
    "sim_bullet_constraintsolvertype_projectedgaussseidel","sim.bullet_constraintsolvertype_projectedgaussseidel",
    "sim_filtercomponent_originalimage","sim.filtercomponent_originalimage",
    "sim_filtercomponent_originaldepth","sim.filtercomponent_originaldepth",
    "sim_filtercomponent_uniformimage","sim.filtercomponent_uniformimage",
    "sim_filtercomponent_tooutput","sim.filtercomponent_tooutput",
    "sim_filtercomponent_tobuffer1","sim.filtercomponent_tobuffer1",
    "sim_filtercomponent_tobuffer2","sim.filtercomponent_tobuffer2",
    "sim_filtercomponent_frombuffer1","sim.filtercomponent_frombuffer1",
    "sim_filtercomponent_frombuffer2","sim.filtercomponent_frombuffer2",
    "sim_filtercomponent_swapbuffers","sim.filtercomponent_swapbuffers",
    "sim_filtercomponent_addbuffer1","sim.filtercomponent_addbuffer1",
    "sim_filtercomponent_subtractbuffer1","sim.filtercomponent_subtractbuffer1",
    "sim_filtercomponent_multiplywithbuffer1","sim.filtercomponent_multiplywithbuffer1",
    "sim_filtercomponent_horizontalflip","sim.filtercomponent_horizontalflip",
    "sim_filtercomponent_verticalflip","sim.filtercomponent_verticalflip",
    "sim_filtercomponent_rotate","sim.filtercomponent_rotate",
    "sim_filtercomponent_shift","sim.filtercomponent_shift",
    "sim_filtercomponent_resize","sim.filtercomponent_resize",
    "sim_filtercomponent_3x3filter","sim.filtercomponent_3x3filter",
    "sim_filtercomponent_5x5filter","sim.filtercomponent_5x5filter",
    "sim_filtercomponent_sharpen","sim.filtercomponent_sharpen",
    "sim_filtercomponent_edge","sim.filtercomponent_edge",
    "sim_filtercomponent_rectangularcut","sim.filtercomponent_rectangularcut",
    "sim_filtercomponent_circularcut","sim.filtercomponent_circularcut",
    "sim_filtercomponent_normalize","sim.filtercomponent_normalize",
    "sim_filtercomponent_intensityscale","sim.filtercomponent_intensityscale",
    "sim_filtercomponent_keeporremovecolors","sim.filtercomponent_keeporremovecolors",
    "sim_filtercomponent_scaleandoffsetcolors","sim.filtercomponent_scaleandoffsetcolors",
    "sim_filtercomponent_binary","sim.filtercomponent_binary",
    "sim_filtercomponent_swapwithbuffer1","sim.filtercomponent_swapwithbuffer1",
    "sim_filtercomponent_addtobuffer1","sim.filtercomponent_addtobuffer1",
    "sim_filtercomponent_subtractfrombuffer1","sim.filtercomponent_subtractfrombuffer1",
    "sim_filtercomponent_correlationwithbuffer1","sim.filtercomponent_correlationwithbuffer1",
    "sim_filtercomponent_colorsegmentation","sim.filtercomponent_colorsegmentation",
    "sim_filtercomponent_blobextraction","sim.filtercomponent_blobextraction",
    "sim_filtercomponent_imagetocoord","sim.filtercomponent_imagetocoord",
    "sim_filtercomponent_pixelchange","sim.filtercomponent_pixelchange",
    "sim_filtercomponent_velodyne","sim.filtercomponent_velodyne",
    "sim_filtercomponent_todepthoutput","sim.filtercomponent_todepthoutput",
    "sim_filtercomponent_customized","sim.filtercomponent_customized",
    "sim_buffer_uint8","sim.buffer_uint8",
    "sim_buffer_int8","sim.buffer_int8",
    "sim_buffer_uint16","sim.buffer_uint16",
    "sim_buffer_int16","sim.buffer_int16",
    "sim_buffer_uint32","sim.buffer_uint32",
    "sim_buffer_int32","sim.buffer_int32",
    "sim_buffer_float","sim.buffer_float",
    "sim_buffer_double","sim.buffer_double",
    "sim_buffer_uint8rgb","sim.buffer_uint8rgb",
    "sim_buffer_uint8bgr","sim.buffer_uint8bgr",
    "sim_imgcomb_vertical","sim.imgcomb_vertical",
    "sim_imgcomb_horizontal","sim.imgcomb_horizontal",
    "sim_dynmat_default","sim.dynmat_default",
    "sim_dynmat_highfriction","sim.dynmat_highfriction",
    "sim_dynmat_lowfriction","sim.dynmat_lowfriction",
    "sim_dynmat_nofriction","sim.dynmat_nofriction",
    "sim_dynmat_reststackgrasp","sim.dynmat_reststackgrasp",
    "sim_dynmat_foot","sim.dynmat_foot",
    "sim_dynmat_wheel","sim.dynmat_wheel",
    "sim_dynmat_gripper","sim.dynmat_gripper",
    "sim_dynmat_floor","sim.dynmat_floor",
    "simrml_phase_sync_if_possible","sim.rml_phase_sync_if_possible",
    "simrml_only_time_sync","sim.rml_only_time_sync",
    "simrml_only_phase_sync","sim.rml_only_phase_sync",
    "simrml_no_sync","sim.rml_no_sync",
    "simrml_disable_extremum_motion_states_calc","sim.rml_disable_extremum_motion_states_calc",
    "simrml_keep_target_vel","sim.rml_keep_target_vel",
    "simrml_recompute_trajectory","sim.rml_recompute_trajectory",
    "simrml_keep_current_vel_if_fallback_strategy","sim.rml_keep_current_vel_if_fallback_strategy",
    "","",
};

const SNewApiMapping _simBubbleApiMapping[]=
{
    "simExtBubble_create","simBubble.create",
    "simExtBubble_destroy","simBubble.destroy",
    "simExtBubble_start","simBubble.start",
    "simExtBubble_stop","simBubble.stop",
    "","",
};

const SNewApiMapping _simK3ApiMapping[]=
{
    "simExtK3_create","simK3.create",
    "simExtK3_destroy","simK3.destroy",
    "simExtK3_getInfrared","simK3.getInfrared",
    "simExtK3_getUltrasonic","simK3.getUltrasonic",
    "simExtK3_getLineSensor","simK3.getLineSensor",
    "simExtK3_getEncoder","simK3.getEncoder",
    "simExtK3_getGripperProxSensor","simK3.getGripperProxSensor",
    "simExtK3_setArmPosition","simK3.setArmPosition",
    "simExtK3_setGripperGap","simK3.setGripperGap",
    "simExtK3_setVelocity","simK3.setVelocity",
    "simExtK3_setEncoders","simK3.setEncoders",
    "","",
};

const SNewApiMapping _simMTBApiMapping[]=
{
    "simExtMtb_startServer","simMTB.startServer",
    "simExtMtb_stopServer","simMTB.stopServer",
    "simExtMtb_disconnectInput","simMTB.disconnectInput",
    "simExtMtb_connectInput","simMTB.connectInput",
    "simExtMtb_setInput","simMTB.setInput",
    "simExtMtb_getInput","simMTB.getInput",
    "simExtMtb_getOutput","simMTB.getOutput",
    "simExtMtb_getJoints","simMTB.getJoints",
    "simExtMtb_step","simMTB.step",
    "","",
};

const SNewApiMapping _simOpenMeshApiMapping[]=
{
    "simExtOpenMesh_getDecimated","simOpenMesh.getDecimated",
    "","",
};

const SNewApiMapping _simSkeletonApiMapping[]=
{
    "simExtPluginSkeleton_getData","simSkeleton.getData",
    "","",
};

const SNewApiMapping _simQHullApiMapping[]=
{
    "simExtQhull_compute","simQHull.compute",
    "","",
};

const SNewApiMapping _simRemoteApiApiMapping[]=
{
    "simExtRemoteApiStart","simRemoteApi.start",
    "simExtRemoteApiStop","simRemoteApi.stop",
    "simExtRemoteApiReset","simRemoteApi.reset",
    "simExtRemoteApiStatus","simRemoteApi.status",
    "","",
};

const SNewApiMapping _simRRS1ApiMapping[]=
{
    "simExtRRS1_startRcsServer","simRRS1.startRcsServer",
    "simExtRRS1_selectRcsServer","simRRS1.selectRcsServer",
    "simExtRRS1_stopRcsServer","simRRS1.stopRcsServer",
    "simExtRRS1_INITIALIZE","simRRS1.INITIALIZE",
    "simExtRRS1_RESET","simRRS1.RESET",
    "simExtRRS1_TERMINATE","simRRS1.TERMINATE",
    "simExtRRS1_GET_ROBOT_STAMP","simRRS1.GET_ROBOT_STAMP",
    "simExtRRS1_GET_HOME_JOINT_POSITION","simRRS1.GET_HOME_JOINT_POSITION",
    "simExtRRS1_GET_RCS_DATA","simRRS1.GET_RCS_DATA",
    "simExtRRS1_MODIFY_RCS_DATA","simRRS1.MODIFY_RCS_DATA",
    "simExtRRS1_SAVE_RCS_DATA","simRRS1.SAVE_RCS_DATA",
    "simExtRRS1_LOAD_RCS_DATA","simRRS1.LOAD_RCS_DATA",
    "simExtRRS1_GET_INVERSE_KINEMATIC","simRRS1.GET_INVERSE_KINEMATIC",
    "simExtRRS1_GET_FORWARD_KINEMATIC","simRRS1.GET_FORWARD_KINEMATIC",
    "simExtRRS1_MATRIX_TO_CONTROLLER_POSITION","simRRS1.MATRIX_TO_CONTROLLER_POSITION",
    "simExtRRS1_CONTROLLER_POSITION_TO_MATRIX","simRRS1.CONTROLLER_POSITION_TO_MATRIX",
    "simExtRRS1_GET_CELL_FRAME","simRRS1.GET_CELL_FRAME",
    "simExtRRS1_MODIFY_CELL_FRAME","simRRS1.MODIFY_CELL_FRAME",
    "simExtRRS1_SELECT_WORK_FRAMES","simRRS1.SELECT_WORK_FRAMES",
    "simExtRRS1_SET_INITIAL_POSITION","simRRS1.SET_INITIAL_POSITION",
    "simExtRRS1_SET_NEXT_TARGET","simRRS1.SET_NEXT_TARGET",
    "simExtRRS1_GET_NEXT_STEP","simRRS1.GET_NEXT_STEP",
    "simExtRRS1_SET_INTERPOLATION_TIME","simRRS1.SET_INTERPOLATION_TIME",
    "simExtRRS1_SELECT_MOTION_TYPE","simRRS1.SELECT_MOTION_TYPE",
    "simExtRRS1_SELECT_TARGET_TYPE","simRRS1.SELECT_TARGET_TYPE",
    "simExtRRS1_SELECT_TRAJECTORY_MODE","simRRS1.SELECT_TRAJECTORY_MODE",
    "simExtRRS1_SELECT_ORIENTATION_INTERPOLATION_MODE","simRRS1.SELECT_ORIENTATION_INTERPOLATION_MODE",
    "simExtRRS1_SELECT_DOMINANT_INTERPOLATION","simRRS1.SELECT_DOMINANT_INTERPOLATION",
    "simExtRRS1_SET_ADVANCE_MOTION","simRRS1.SET_ADVANCE_MOTION",
    "simExtRRS1_SET_MOTION_FILTER","simRRS1.SET_MOTION_FILTER",
    "simExtRRS1_SET_OVERRIDE_POSITION","simRRS1.SET_OVERRIDE_POSITION",
    "simExtRRS1_REVERSE_MOTION","simRRS1.REVERSE_MOTION",
    "simExtRRS1_SET_PAYLOAD_PARAMETER","simRRS1.SET_PAYLOAD_PARAMETER",
    "simExtRRS1_SELECT_TIME_COMPENSATION","simRRS1.SELECT_TIME_COMPENSATION",
    "simExtRRS1_SET_CONFIGURATION_CONTROL","simRRS1.SET_CONFIGURATION_CONTROL",
    "simExtRRS1_SET_JOINT_SPEEDS","simRRS1.SET_JOINT_SPEEDS",
    "simExtRRS1_SET_CARTESIAN_POSITION_SPEED","simRRS1.SET_CARTESIAN_POSITION_SPEED",
    "simExtRRS1_SET_CARTESIAN_ORIENTATION_SPEED","simRRS1.SET_CARTESIAN_ORIENTATION_SPEED",
    "simExtRRS1_SET_JOINT_ACCELERATIONS","simRRS1.SET_JOINT_ACCELERATIONS",
    "simExtRRS1_SET_CARTESIAN_POSITION_ACCELERATION","simRRS1.SET_CARTESIAN_POSITION_ACCELERATION",
    "simExtRRS1_SET_CARTESIAN_ORIENTATION_ACCELERATION","simRRS1.SET_CARTESIAN_ORIENTATION_ACCELERATION",
    "simExtRRS1_SET_JOINT_JERKS","simRRS1.SET_JOINT_JERKS",
    "simExtRRS1_SET_MOTION_TIME","simRRS1.SET_MOTION_TIME",
    "simExtRRS1_SET_OVERRIDE_SPEED","simRRS1.SET_OVERRIDE_SPEED",
    "simExtRRS1_SET_OVERRIDE_ACCELERATION","simRRS1.SET_OVERRIDE_ACCELERATION",
    "simExtRRS1_SELECT_FLYBY_MODE","simRRS1.SELECT_FLYBY_MODE",
    "simExtRRS1_SET_FLYBY_CRITERIA_PARAMETER","simRRS1.SET_FLYBY_CRITERIA_PARAMETER",
    "simExtRRS1_SELECT_FLYBY_CRITERIA","simRRS1.SELECT_FLYBY_CRITERIA",
    "simExtRRS1_CANCEL_FLYBY_CRITERIA","simRRS1.CANCEL_FLYBY_CRITERIA",
    "simExtRRS1_SELECT_POINT_ACCURACY","simRRS1.SELECT_POINT_ACCURACY",
    "simExtRRS1_SET_POINT_ACCURACY_PARAMETER","simRRS1.SET_POINT_ACCURACY_PARAMETER",
    "simExtRRS1_SET_REST_PARAMETER","simRRS1.SET_REST_PARAMETER",
    "simExtRRS1_GET_CURRENT_TARGETID","simRRS1.GET_CURRENT_TARGETID",
    "simExtRRS1_SELECT_TRACKING","simRRS1.SELECT_TRACKING",
    "simExtRRS1_SET_CONVEYOR_POSITION","simRRS1.SET_CONVEYOR_POSITION",
    "simExtRRS1_DEFINE_EVENT","simRRS1.DEFINE_EVENT",
    "simExtRRS1_CANCEL_EVENT","simRRS1.CANCEL_EVENT",
    "simExtRRS1_GET_EVENT","simRRS1.GET_EVENT",
    "simExtRRS1_STOP_MOTION","simRRS1.STOP_MOTION",
    "simExtRRS1_CONTINUE_MOTION","simRRS1.CONTINUE_MOTION",
    "simExtRRS1_CANCEL_MOTION","simRRS1.CANCEL_MOTION",
    "simExtRRS1_GET_MESSAGE","simRRS1.GET_MESSAGE",
    "simExtRRS1_SELECT_WEAVING_MODE","simRRS1.SELECT_WEAVING_MODE",
    "simExtRRS1_SELECT_WEAVING_GROUP","simRRS1.SELECT_WEAVING_GROUP",
    "simExtRRS1_SET_WEAVING_GROUP_PARAMETER","simRRS1.SET_WEAVING_GROUP_PARAMETER",
    "simExtRRS1_DEBUG","simRRS1.DEBUG",
    "simExtRRS1_EXTENDED_SERVICE","simRRS1.EXTENDED_SERVICE",
    "","",
};

const SNewApiMapping _simVisionApiMapping[]=
{
    "simExtVision_handleSpherical","simVision.handleSpherical",
    "simExtVision_handleAnaglyphStereo","simVision.handleAnaglyphStereo",
    "simExtVision_createVelodyneHDL64E","simVision.createVelodyneHDL64E",
    "simExtVision_destroyVelodyneHDL64E","simVision.destroyVelodyneHDL64E",
    "simExtVision_handleVelodyneHDL64E","simVision.handleVelodyneHDL64E",
    "simExtVision_createVelodyneVPL16","simVision.createVelodyneVPL16",
    "simExtVision_destroyVelodyneVPL16","simExtVision_destroyVelodyneVPL16",
    "simExtVision_handleVelodyneVPL16","simVision.handleVelodyneVPL16",
    "simExtVision_createVelodyne","simVision.createVelodyneHDL64E",
    "simExtVision_destroyVelodyne","simVision.destroyVelodyneHDL64E",
    "simExtVision_handleVelodyne","simVision.handleVelodyneHDL64E",
    "","",
};

const SNewApiMapping _simCamApiMapping[]=
{
    "simExtCamStart","simCam.start",
    "simExtCamEnd","simCam.stop",
    "simExtCamInfo","simCam.info",
    "simExtCamGrab","simCam.grab",
    "","",
};

const SNewApiMapping _simJoyApiMapping[]=
{
    "simExtJoyGetCount","simJoy.getCount",
    "simExtJoyGetData","simJoy.getData",
    "","",
};

const SNewApiMapping _simWiiApiMapping[]=
{
    "simExtWiiStart","simWii.start",
    "simExtWiiEnd","simWii.stop",
    "simExtWiiSet","simWii.set",
    "simExtWiiGet","simWii.get",
    "","",
};

const SNewApiMapping _simURDFApiMapping[]=
{
    "simExtImportUrdf","simURDF.import",
    "simExtImportUrdfFile","simURDF.importFile",
    "","",
};

const SNewApiMapping _simBWFApiMapping[]=
{
    "simExtBwf_query","simBWF.query",
    "","",
};

const SNewApiMapping _simUIApiMapping[]=
{
    "simExtCustomUI_create","simUI.create",
    "simExtCustomUI_destroy","simUI.destroy",
    "simExtCustomUI_getSliderValue","simUI.getSliderValue",
    "simExtCustomUI_setSliderValue","simUI.setSliderValue",
    "simExtCustomUI_getEditValue","simUI.getEditValue",
    "simExtCustomUI_setEditValue","simUI.setEditValue",
    "simExtCustomUI_getSpinboxValue","simUI.getSpinboxValue",
    "simExtCustomUI_setSpinboxValue","simUI.setSpinboxValue",
    "simExtCustomUI_getCheckboxValue","simUI.getCheckboxValue",
    "simExtCustomUI_setCheckboxValue","simUI.setCheckboxValue",
    "simExtCustomUI_getRadiobuttonValue","simUI.getRadiobuttonValue",
    "simExtCustomUI_setRadiobuttonValue","simUI.setRadiobuttonValue",
    "simExtCustomUI_getLabelText","simUI.getLabelText",
    "simExtCustomUI_setLabelText","simUI.setLabelText",
    "simExtCustomUI_insertComboboxItem","simUI.insertComboboxItem",
    "simExtCustomUI_removeComboboxItem","simUI.removeComboboxItem",
    "simExtCustomUI_getComboboxItemCount","simUI.getComboboxItemCount",
    "simExtCustomUI_getComboboxItemText","simUI.getComboboxItemText",
    "simExtCustomUI_getComboboxItems","simUI.getComboboxItems",
    "simExtCustomUI_setComboboxItems","simUI.setComboboxItems",
    "simExtCustomUI_setComboboxSelectedIndex","simUI.setComboboxSelectedIndex",
    "simExtCustomUI_hide","simUI.hide",
    "simExtCustomUI_show","simUI.show",
    "simExtCustomUI_isVisible","simUI.isVisible",
    "simExtCustomUI_getPosition","simUI.getPosition",
    "simExtCustomUI_setPosition","simUI.setPosition",
    "simExtCustomUI_getSize","simUI.getSize",
    "simExtCustomUI_setSize","simUI.setSize",
    "simExtCustomUI_getTitle","simUI.getTitle",
    "simExtCustomUI_setTitle","simUI.setTitle",
    "simExtCustomUI_setImageData","simUI.setImageData",
    "simExtCustomUI_setEnabled","simUI.setEnabled",
    "simExtCustomUI_getCurrentTab","simUI.getCurrentTab",
    "simExtCustomUI_setCurrentTab","simUI.setCurrentTab",
    "simExtCustomUI_getWidgetVisibility","simUI.getWidgetVisibility",
    "simExtCustomUI_setWidgetVisibility","simUI.setWidgetVisibility",
    "simExtCustomUI_getCurrentEditWidget","simUI.getCurrentEditWidget",
    "simExtCustomUI_setCurrentEditWidget","simUI.setCurrentEditWidget",
    "simExtCustomUI_replot","simUI.replot",
    "simExtCustomUI_addCurve","simUI.addCurve",
    "simExtCustomUI_addCurveTimePoints","simUI.addCurveTimePoints",
    "simExtCustomUI_addCurveXYPoints","simUI.addCurveXYPoints",
    "simExtCustomUI_clearCurve","simUI.clearCurve",
    "simExtCustomUI_removeCurve","simUI.removeCurve",
    "simExtCustomUI_setPlotRanges","simUI.setPlotRanges",
    "simExtCustomUI_setPlotXRange","simUI.setPlotXRange",
    "simExtCustomUI_setPlotYRange","simUI.setPlotYRange",
    "simExtCustomUI_growPlotRanges","simUI.growPlotRanges",
    "simExtCustomUI_growPlotXRange","simUI.growPlotXRange",
    "simExtCustomUI_growPlotYRange","simUI.growPlotYRange",
    "simExtCustomUI_setPlotLabels","simUI.setPlotLabels",
    "simExtCustomUI_setPlotXLabel","simUI.setPlotXLabel",
    "simExtCustomUI_setPlotYLabel","simUI.setPlotYLabel",
    "simExtCustomUI_rescaleAxes","simUI.rescaleAxes",
    "simExtCustomUI_rescaleAxesAll","simUI.rescaleAxesAll",
    "simExtCustomUI_setMouseOptions","simUI.setMouseOptions",
    "simExtCustomUI_setLegendVisibility","simUI.setLegendVisibility",
    "simExtCustomUI_getCurveData","simUI.getCurveData",
    "simExtCustomUI_clearTable","simUI.clearTable",
    "simExtCustomUI_setRowCount","simUI.setRowCount",
    "simExtCustomUI_setColumnCount","simUI.setColumnCount",
    "simExtCustomUI_setRowHeaderText","simUI.setRowHeaderText",
    "simExtCustomUI_setColumnHeaderText","simUI.setColumnHeaderText",
    "simExtCustomUI_setItem","simUI.setItem",
    "simExtCustomUI_getRowCount","simUI.getRowCount",
    "simExtCustomUI_getColumnCount","simUI.getColumnCount",
    "simExtCustomUI_getItem","simUI.getItem",
    "simExtCustomUI_setItemEditable","simUI.setItemEditable",
    "simExtCustomUI_saveState","simUI.saveState",
    "simExtCustomUI_restoreState","simUI.restoreState",
    "simExtCustomUI_setColumnWidth","simUI.setColumnWidth",
    "simExtCustomUI_setRowHeight","simUI.setRowHeight",
    "simExtCustomUI_setTableSelection","simUI.setTableSelection",
    "simExtCustomUI_setProgress","simUI.setProgress",
    "simExtCustomUI_clearTree","simUI.clearTree",
    "simExtCustomUI_addTreeItem","simUI.addTreeItem",
    "simExtCustomUI_updateTreeItemText","simUI.updateTreeItemText",
    "simExtCustomUI_updateTreeItemParent","simUI.updateTreeItemParent",
    "simExtCustomUI_removeTreeItem","simUI.removeTreeItem",
    "simExtCustomUI_setTreeSelection","simUI.setTreeSelection",
    "simExtCustomUI_expandAll","simUI.expandAll",
    "simExtCustomUI_collapseAll","simUI.collapseAll",
    "simExtCustomUI_expandToDepth","simUI.expandToDepth",
    "simExtCustomUI_addNode","simUI.addNode",
    "simExtCustomUI_removeNode","simUI.removeNode",
    "simExtCustomUI_setNodeValid","simUI.setNodeValid",
    "simExtCustomUI_isNodeValid","simUI.isNodeValid",
    "simExtCustomUI_setNodePos","simUI.setNodePos",
    "simExtCustomUI_getNodePos","simUI.getNodePos",
    "simExtCustomUI_setNodeText","simUI.setNodeText",
    "simExtCustomUI_getNodeText","simUI.getNodeText",
    "simExtCustomUI_setNodeInletCount","simUI.setNodeInletCount",
    "simExtCustomUI_getNodeInletCount","simUI.getNodeInletCount",
    "simExtCustomUI_setNodeOutletCount","simUI.setNodeOutletCount",
    "simExtCustomUI_getNodeOutletCount","simUI.getNodeOutletCount",
    "simExtCustomUI_addConnection","simUI.addConnection",
    "simExtCustomUI_removeConnection","simUI.removeConnection",
    "simExtCustomUI_setText","simUI.setText",
    "simExtCustomUI_setUrl","simUI.setUrl",
    "sim_customui_curve_type_time","simUI.curve_type.time",
    "sim_customui_curve_type_xy","simUI.curve_type.xy",
    "sim_customui_curve_style_scatter","simUI.curve_style.scatter",
    "sim_customui_curve_style_line","simUI.curve_style.line",
    "sim_customui_curve_style_line_and_scatter","simUI.curve_style.line_and_scatter",
    "sim_customui_curve_style_step_left","simUI.curve_style.step_left",
    "sim_customui_curve_style_step_center","simUI.curve_style.step_center",
    "sim_customui_curve_style_step_right","simUI.curve_style.step_right",
    "sim_customui_curve_style_impulse","simUI.curve_style.impulse",
    "sim_customui_curve_scatter_shape_none","simUI.curve_scatter_shape.none",
    "sim_customui_curve_scatter_shape_dot","simUI.curve_scatter_shape.dot",
    "sim_customui_curve_scatter_shape_cross","simUI.curve_scatter_shape.cross",
    "sim_customui_curve_scatter_shape_plus","simUI.curve_scatter_shape.plus",
    "sim_customui_curve_scatter_shape_circle","simUI.curve_scatter_shape.circle",
    "sim_customui_curve_scatter_shape_disc","simUI.curve_scatter_shape.disc",
    "sim_customui_curve_scatter_shape_square","simUI.curve_scatter_shape.square",
    "sim_customui_curve_scatter_shape_diamond","simUI.curve_scatter_shape.diamond",
    "sim_customui_curve_scatter_shape_star","simUI.curve_scatter_shape.star",
    "sim_customui_curve_scatter_shape_triangle","simUI.curve_scatter_shape.triangle",
    "sim_customui_curve_scatter_shape_triangle_inverted","simUI.curve_scatter_shape.triangle_inverted",
    "sim_customui_curve_scatter_shape_cross_square","simUI.curve_scatter_shape.cross_square",
    "sim_customui_curve_scatter_shape_plus_square","simUI.curve_scatter_shape.plus_square",
    "sim_customui_curve_scatter_shape_cross_circle","simUI.curve_scatter_shape.cross_circle",
    "sim_customui_curve_scatter_shape_plus_circle","simUI.curve_scatter_shape.plus_circle",
    "sim_customui_curve_scatter_shape_peace","simUI.curve_scatter_shape.peace",
    "sim_customui_line_style_solid","simUI.line_style.solid",
    "sim_customui_line_style_dotted","simUI.line_style.dotted",
    "sim_customui_line_style_dashed","simUI.line_style.dashed",
    "sim_customui_mouse_left_button_down","simUI.mouse.left_button_down",
    "sim_customui_mouse_left_button_up","simUI.mouse.left_button_up",
    "sim_customui_mouse_move","simUI.mouse.move",
    "","",
};

const SNewApiMapping _simROSApiMapping[]=
{
    "simExtRosInterface_subscribe","simROS.subscribe",
    "simExtRosInterface_shutdownSubscriber","simROS.shutdownSubscriber",
    "simExtRosInterface_subscriberTreatUInt8ArrayAsString","simROS.subscriberTreatUInt8ArrayAsString",
    "simExtRosInterface_advertise","simROS.advertise",
    "simExtRosInterface_shutdownPublisher","simROS.shutdownPublisher",
    "simExtRosInterface_publisherTreatUInt8ArrayAsString","simROS.publisherTreatUInt8ArrayAsString",
    "simExtRosInterface_publish","simROS.publish",
    "simExtRosInterface_serviceClient","simROS.serviceClient",
    "simExtRosInterface_shutdownServiceClient","simROS.shutdownServiceClient",
    "simExtRosInterface_serviceClientTreatUInt8ArrayAsString","simROS.serviceClientTreatUInt8ArrayAsString",
    "simExtRosInterface_call","simROS.call",
    "simExtRosInterface_advertiseService","simROS.advertiseService",
    "simExtRosInterface_shutdownServiceServer","simROS.shutdownServiceServer",
    "simExtRosInterface_serviceServerTreatUInt8ArrayAsString","simROS.serviceServerTreatUInt8ArrayAsString",
    "simExtRosInterface_sendTransform","simROS.sendTransform",
    "simExtRosInterface_sendTransforms","simROS.sendTransforms",
    "simExtRosInterface_imageTransportSubscribe","simROS.imageTransportSubscribe",
    "simExtRosInterface_imageTransportShutdownSubscriber","simROS.imageTransportShutdownSubscriber",
    "simExtRosInterface_imageTransportAdvertise","simROS.imageTransportAdvertise",
    "simExtRosInterface_imageTransportShutdownPublisher","simROS.imageTransportShutdownPublisher",
    "simExtRosInterface_imageTransportPublish","simROS.imageTransportPublish",
    "simExtRosInterface_getTime","simROS.getTime",
    "simExtRosInterface_getParamString","simROS.getParamString",
    "simExtRosInterface_getParamInt","simROS.getParamInt",
    "simExtRosInterface_getParamDouble","simROS.getParamDouble",
    "simExtRosInterface_getParamBool","simROS.getParamBool",
    "simExtRosInterface_setParamString","simROS.setParamString",
    "simExtRosInterface_setParamInt","simROS.setParamInt",
    "simExtRosInterface_setParamDouble","simROS.setParamDouble",
    "simExtRosInterface_setParamBool","simROS.setParamBool",
    "simExtRosInterface_hasParam","simROS.hasParam",
    "simExtRosInterface_deleteParam","simROS.deleteParam",
    "simExtRosInterface_searchParam","simROS.searchParam",
    "","",
};

const SNewApiMapping _simICPApiMapping[]=
{
    "simExtICP_match","simICP.match",
    "simExtICP_matchToShape","simICP.matchToShape",
    "","",
};

const SNewApiMapping _simOMPLApiMapping[]=
{
    "simExtOMPL_createStateSpace","simOMPL.createStateSpace",
    "simExtOMPL_destroyStateSpace","simOMPL.destroyStateSpace",
    "simExtOMPL_createTask","simOMPL.createTask",
    "simExtOMPL_destroyTask","simOMPL.destroyTask",
    "simExtOMPL_printTaskInfo","simOMPL.printTaskInfo",
    "simExtOMPL_setVerboseLevel","simOMPL.setVerboseLevel",
    "simExtOMPL_setStateValidityCheckingResolution","simOMPL.setStateValidityCheckingResolution",
    "simExtOMPL_setStateSpace","simOMPL.setStateSpace",
    "simExtOMPL_setAlgorithm","simOMPL.setAlgorithm",
    "simExtOMPL_setCollisionPairs","simOMPL.setCollisionPairs",
    "simExtOMPL_setStartState","simOMPL.setStartState",
    "simExtOMPL_setGoalState","simOMPL.setGoalState",
    "simExtOMPL_addGoalState","simOMPL.addGoalState",
    "simExtOMPL_setGoal","simOMPL.setGoal",
    "simExtOMPL_setup","simOMPL.setup",
    "simExtOMPL_solve","simOMPL.solve",
    "simExtOMPL_simplifyPath","simOMPL.simplifyPath",
    "simExtOMPL_interpolatePath","simOMPL.interpolatePath",
    "simExtOMPL_getPath","simOMPL.getPath",
    "simExtOMPL_compute","simOMPL.compute",
    "simExtOMPL_readState","simOMPL.readState",
    "simExtOMPL_writeState","simOMPL.writeState",
    "simExtOMPL_isStateValid","simOMPL.isStateValid",
    "simExtOMPL_setProjectionEvaluationCallback","simOMPL.setProjectionEvaluationCallback",
    "simExtOMPL_setStateValidationCallback","simOMPL.setStateValidationCallback",
    "simExtOMPL_setGoalCallback","simOMPL.setGoalCallback",
    "simExtOMPL_setValidStateSamplerCallback","simOMPL.setValidStateSamplerCallback",
    "sim_ompl_algorithm_BiTRRT","simOMPL.Algorithm.BiTRRT",
    "sim_ompl_algorithm_BITstar","simOMPL.Algorithm.BITstar",
    "sim_ompl_algorithm_BKPIECE1","simOMPL.Algorithm.BKPIECE1",
    "sim_ompl_algorithm_CForest","simOMPL.Algorithm.CForest",
    "sim_ompl_algorithm_EST","simOMPL.Algorithm.EST",
    "sim_ompl_algorithm_FMT","simOMPL.Algorithm.FMT",
    "sim_ompl_algorithm_KPIECE1","simOMPL.Algorithm.KPIECE1",
    "sim_ompl_algorithm_LazyPRM","simOMPL.Algorithm.LazyPRM",
    "sim_ompl_algorithm_LazyPRMstar","simOMPL.Algorithm.LazyPRMstar",
    "sim_ompl_algorithm_LazyRRT","simOMPL.Algorithm.LazyRRT",
    "sim_ompl_algorithm_LBKPIECE1","simOMPL.Algorithm.LBKPIECE1",
    "sim_ompl_algorithm_LBTRRT","simOMPL.Algorithm.LBTRRT",
    "sim_ompl_algorithm_PDST","simOMPL.Algorithm.PDST",
    "sim_ompl_algorithm_PRM","simOMPL.Algorithm.PRM",
    "sim_ompl_algorithm_PRMstar","simOMPL.Algorithm.PRMstar",
    "sim_ompl_algorithm_pRRT","simOMPL.Algorithm.pRRT",
    "sim_ompl_algorithm_pSBL","simOMPL.Algorithm.pSBL",
    "sim_ompl_algorithm_RRT","simOMPL.Algorithm.RRT",
    "sim_ompl_algorithm_RRTConnect","simOMPL.Algorithm.RRTConnect",
    "sim_ompl_algorithm_RRTstar","simOMPL.Algorithm.RRTstar",
    "sim_ompl_algorithm_SBL","simOMPL.Algorithm.SBL",
    "sim_ompl_algorithm_SPARS","simOMPL.Algorithm.SPARS",
    "sim_ompl_algorithm_SPARStwo","simOMPL.Algorithm.SPARStwo",
    "sim_ompl_algorithm_STRIDE","simOMPL.Algorithm.STRIDE",
    "sim_ompl_algorithm_TRRT","simOMPL.Algorithm.TRRT",
    "sim_ompl_statespacetype_position2d","simOMPL.StateSpaceType.position2d",
    "sim_ompl_statespacetype_pose2d","simOMPL.StateSpaceType.pose2d",
    "sim_ompl_statespacetype_position3d","simOMPL.StateSpaceType.position3d",
    "sim_ompl_statespacetype_pose3d","simOMPL.StateSpaceType.pose3d",
    "sim_ompl_statespacetype_joint_position","simOMPL.StateSpaceType.joint_position",
    "","",
};

const SNewApiMapping _simSDFApiMapping[]=
{
    "simExtSDF_import","simSDF.import",
    "simExtSDF_dump","simSDF.dump",
    "","",
};

const SNewApiMapping _simSurfRecApiMapping[]=
{
    "simExtSurfaceReconstruction_reconstruct","simSurfRec.reconstruct",
    "sim_surfacereconstruction_algorithm_scalespace","simSurfRec.Algorithm.scalespace",
    "sim_surfacereconstruction_algorithm_poisson","simSurfRec.Algorithm.poisson",
    "sim_surfacereconstruction_algorithm_advancingfront","simSurfRec.Algorithm.advancingfront",
    "","",
};

const SNewApiMapping _simxApiMapping[]=
{
    "simxStart","simx.start",
    "simxFinish","simx.finish",
    "simxAddStatusbarMessage","simx.addStatusbarMessage",
    "simxAuxiliaryConsoleClose","simx.auxiliaryConsoleClose",
    "simxAuxiliaryConsoleOpen","simx.auxiliaryConsoleOpen",
    "simxAuxiliaryConsolePrint","simx.auxiliaryConsolePrint",
    "simxAuxiliaryConsoleShow","simx.auxiliaryConsoleShow",
    "simxBreakForceSensor","simx.breakForceSensor",
    "simxClearFloatSignal","simx.clearFloatSignal",
    "simxClearIntegerSignal","simx.clearIntegerSignal",
    "simxClearStringSignal","simx.clearStringSignal",
    "simxGetObjectHandle","simx.getObjectHandle",
    "simxCloseScene","simx.closeScene",
    "simxCopyPasteObjects","simx.copyPasteObjects",
    "simxLoadScene","simx.loadScene",
    "simxLoadModel","simx.loadModel",
    "simxSetBooleanParameter","simx.setBooleanParameter",
    "simxStartSimulation","simx.startSimulation",
    "simxStopSimulation","simx.stopSimulation",
    "simxSetFloatingParameter","simx.setFloatingParameter",
    "simxSetIntegerParameter","simx.setIntegerParameter",
    "simxSetArrayParameter","simx.setArrayParameter",
    "simxGetBooleanParameter","simx.getBooleanParameter",
    "simxGetFloatingParameter","simx.getFloatingParameter",
    "simxGetIntegerParameter","simx.getIntegerParameter",
    "simxGetArrayParameter","simx.getArrayParameter",
    "simxGetStringParameter","simx.getStringParameter",
    "simxSetFloatSignal","simx.setFloatSignal",
    "simxSetIntegerSignal","simx.setIntegerSignal",
    "simxSetStringSignal","simx.setStringSignal",
    "simxGetFloatSignal","simx.getFloatSignal",
    "simxGetIntegerSignal","simx.getIntegerSignal",
    "simxGetStringSignal","simx.getStringSignal",
    "simxCreateDummy","simx.createDummy",
    "simxDisplayDialog","simx.displayDialog",
    "simxEndDialog","simx.endDialog",
    "simxGetDialogInput","simx.getDialogInput",
    "simxGetDialogResult","simx.getDialogResult",
    "simxEraseFile","simx.eraseFile",
    "simxGetCollisionHandle","simx.getCollisionHandle",
    "simxGetCollectionHandle","simx.getCollectionHandle",
    "simxGetConnectionId","simx.getConnectionId",
    "simxGetDistanceHandle","simx.getDistanceHandle",
    "simxGetInMessageInfo","simx.getInMessageInfo",
    "simxGetOutMessageInfo","simx.getOutMessageInfo",
    "simxGetJointForce","simx.getJointForce",
    "simxGetJointMatrix","simx.getJointMatrix",
    "simxGetJointPosition","simx.getJointPosition",
    "simxGetLastCmdTime","simx.getLastCmdTime",
    "simxGetLastErrors","simx.getLastErrors",
    "simxGetModelProperty","simx.getModelProperty",
    "simxGetObjectChild","simx.getObjectChild",
    "simxGetObjectFloatParameter","simx.getObjectFloatParameter",
    "simxGetObjectIntParameter","simx.getObjectIntParameter",
    "simxGetObjectGroupData","simx.getObjectGroupData",
    "simxGetObjectOrientation","simx.getObjectOrientation",
    "simxGetObjectPosition","simx.getObjectPosition",
    "simxGetObjectParent","simx.getObjectParent",
    "simxGetObjects","simx.getObjects",
    "simxGetObjectSelection","simx.getObjectSelection",
    "simxGetObjectVelocity","simx.getObjectVelocity",
    "simxGetPingTime","simx.getPingTime",
    "simxGetVisionSensorDepthBuffer","simx.getVisionSensorDepthBuffer",
    "simxGetVisionSensorImage","simx.getVisionSensorImage",
    "simxSetVisionSensorImage","simx.setVisionSensorImage",
    "simxPauseCommunication","simx.pauseCommunication",
    "simxPauseSimulation","simx.pauseSimulation",
    "simxQuery","simx.query",
    "simxReadCollision","simx.readCollision",
    "simxReadDistance","simx.readDistance",
    "simxReadForceSensor","simx.readForceSensor",
    "simxReadProximitySensor","simx.readProximitySensor",
    "simxReadStringStream","simx.readStringStream",
    "simxWriteStringStream","simx.writeStringStream",
    "simxReadVisionSensor","simx.readVisionSensor",
    "simxRemoveModel","simx.removeModel",
    "simxRemoveObject","simx.removeObject",
    "simxSetJointForce","simx.setJointForce",
    "simxSetJointPosition","simx.setJointPosition",
    "simxSetJointTargetPosition","simx.setJointTargetPosition",
    "simxSetJointTargetVelocity","simx.setJointTargetVelocity",
    "simxSetModelProperty","simx.setModelProperty",
    "simxSetObjectOrientation","simx.setObjectOrientation",
    "simxSetObjectPosition","simx.setObjectPosition",
    "simxSetObjectParent","simx.setObjectParent",
    "simxSetObjectSelection","simx.setObjectSelection",
    "simxSetSphericalJointMatrix","simx.setSphericalJointMatrix",
    "simxSynchronous","simx.synchronous",
    "simxSynchronousTrigger","simx.synchronousTrigger",
    "simxTransferFile","simx.transferFile",
    "simxSetObjectFloatParameter","simx.setObjectFloatParameter",
    "simxSetObjectIntParameter","simx.setObjectIntParameter",
    "simxCallScriptFunction","simx.callScriptFunction",
    "simxLoadUI","simx.loadUI",
    "simxGetUIButtonProperty","simx.getUIButtonProperty",
    "simxGetUIEventButton","simx.getUIEventButton",
    "simxGetUIHandle","simx.getUIHandle",
    "simxGetUISlider","simx.getUISlider",
    "simxRemoveUI","simx.removeUI",
    "simxSetUIButtonLabel","simx.setUIButtonLabel",
    "simxSetUIButtonProperty","simx.setUIButtonProperty",
    "simxSetUISlider","simx.setUISlider",
    "simx_return_ok","simx.return_ok",
    "simx_return_novalue_flag","simx.return_novalue_flag",
    "simx_return_timeout_flag","simx.return_timeout_flag",
    "simx_return_illegal_opmode_flag","simx.return_illegal_opmode_flag",
    "simx_return_remote_error_flag","simx.return_remote_error_flag",
    "simx_return_split_progress_flag","simx.return_split_progress_flag",
    "simx_return_local_error_flag","simx.return_local_error_flag",
    "simx_return_initialize_error_flag","simx.return_initialize_error_flag",
    "simx_opmode_oneshot","simx.opmode_oneshot",
    "simx_opmode_blocking","simx.opmode_blocking",
    "simx_opmode_oneshot_wait","simx.opmode_oneshot_wait",
    "simx_opmode_streaming","simx.opmode_streaming",
    "simx_opmode_oneshot_split","simx.opmode_oneshot_split",
    "simx_opmode_streaming_split","simx.opmode_streaming_split",
    "simx_opmode_discontinue","simx.opmode_discontinue",
    "simx_opmode_buffer","simx.opmode_buffer",
    "simx_opmode_remove","simx.opmode_remove",
    "simx_headeroffset_version","simx.headeroffset_version",
    "simx_headeroffset_message_id","simx.headeroffset_message_id",
    "simx_headeroffset_client_time","simx.headeroffset_client_time",
    "simx_headeroffset_server_time","simx.headeroffset_server_time",
    "simx_headeroffset_scene_id","simx.headeroffset_scene_id",
    "simx_headeroffset_server_state","simx.headeroffset_server_state",
    "simx_cmdheaderoffset_mem_size","simx.cmdheaderoffset_mem_size",
    "simx_cmdheaderoffset_full_mem_size","simx.cmdheaderoffset_full_mem_size",
    "simx_cmdheaderoffset_cmd","simx.cmdheaderoffset_cmd",
    "simx_cmdheaderoffset_sim_time","simx.cmdheaderoffset_sim_time",
    "simx_cmdheaderoffset_status","simx.cmdheaderoffset_status",
    "","",
};

CLuaScriptObject::CLuaScriptObject(int scriptTypeOrMinusOneForSerialization)
{
    _scriptHandle=SIM_IDSTART_LUASCRIPT;
    _scriptUniqueId=_scriptUniqueCounter++;
    _objectHandleAttachedTo=-1;

    _scriptText="";
    _scriptTextExec="";

    _numberOfPasses=0;
    _threadedExecution=false;
    _scriptIsDisabled=false;
    _executionState=execState_uninitialized;
    _executeJustOnce=false;
    _automaticCascadingCallsDisabled_OLD=false;
    _threadedExecutionUnderWay=false;
    _flaggedForDestruction=false;
    _mainScriptIsDefaultMainScript_old=false;
    _executionOrder=sim_scriptexecorder_normal;
    _debugLevel=0;
    _inDebug=false;
    _treeTraversalDirection=0; // reverse by default
    _custScriptDisabledDSim_compatibilityMode_DEPRECATED=false;
    _customizationScriptCleanupBeforeSave_DEPRECATED=false;
    _addOn_executionState=sim_syscb_init;

    _previousEditionWindowPosAndSize[0]=50;
    _previousEditionWindowPosAndSize[1]=50;
    _previousEditionWindowPosAndSize[2]=1000;
    _previousEditionWindowPosAndSize[3]=800;

    _warningAboutSimHandleChildScriptAlreadyIssued_oldCompatibility_7_8_2014=false;
    _warning_simRMLPosition_oldCompatibility_30_8_2014=false;
    _warning_simRMLVelocity_oldCompatibility_30_8_2014=false;
    _warning_simGetMpConfigForTipPose_oldCompatibility_21_1_2016=false;
    _warning_simFindIkPath_oldCompatibility_2_2_2016=false;
    _timeOfPcallStart=-1;

    _customObjectData=nullptr;
    _customObjectData_tempData=nullptr;

    _scriptParameters_backCompatibility=new CUserParameters();
    _outsideCommandQueue=new COutsideCommandQueueForScript();

    _scriptType=scriptTypeOrMinusOneForSerialization;
    _compatibilityModeOrFirstTimeCall_sysCallbacks=true;
    _containsJointCallbackFunction=false;
    _containsContactCallbackFunction=false;
    _containsDynCallbackFunction=false;
    _containsVisionCallbackFunction=false;
    _containsTriggerCallbackFunction=false;
    _containsUserConfigCallbackFunction=false;


    L=nullptr;
    _loadBufferResult=-1;
    _inExecutionNow=false;

    if (_nextIdForExternalScriptEditor==-1)
    { // new since 10/9/2014, otherwise there can be conflicts between simultaneously opened CoppeliaSim instances
        _nextIdForExternalScriptEditor=(VDateTime::getOSTimeInMs()&0xffff)*1000;
    }
    _filenameForExternalScriptEditor="embScript_"+tt::FNb(_nextIdForExternalScriptEditor++)+".lua";

    if (_scriptType==sim_scripttype_sandboxscript)
    {
        _scriptHandle=SIM_IDSTART_SANDBOXSCRIPT;
        L=initializeNewLuaState(_scriptHandle,getScriptNameIndexNumber(),_debugLevel);
        _calledInThisSimulationStep=false;
        _raiseErrors_backCompatibility=true;
        _randGen.seed(123456);
        _delayForAutoYielding=2;
        _forbidAutoYieldingLevel=0;
    }
}

CLuaScriptObject::~CLuaScriptObject()
{
    TRACE_INTERNAL;
    killLuaState(); // should already have been done outside of the destructor!
    delete _scriptParameters_backCompatibility;
    delete _outsideCommandQueue;
    delete _customObjectData;
    delete _customObjectData_tempData;
    if (App::userSettings->externalScriptEditor.length()>0)
    {
        // destroy file
        std::string fname=App::folders->getExtScriptEditorTempPath()+"/";
        fname.append(_filenameForExternalScriptEditor);
        if (VFile::doesFileExist(fname.c_str()))
            VFile::eraseFile(fname.c_str());
    }
    App::worldContainer->announceScriptWillBeErased(_scriptHandle,isSimulationScript(),isSceneSwitchPersistentScript());
}

std::string CLuaScriptObject::getFilenameForExternalScriptEditor() const
{
    std::string fname=App::folders->getExtScriptEditorTempPath()+"/";
    fname.append(_filenameForExternalScriptEditor);
    return(fname);
}

void CLuaScriptObject::fromFileToBuffer()
{
    if (App::userSettings->externalScriptEditor.size()>0)
    { // read file
        std::string fname=App::folders->getExtScriptEditorTempPath()+"/";
        fname.append(_filenameForExternalScriptEditor);

        VFile myFile(fname.c_str(),VFile::READ|VFile::SHARE_DENY_NONE,true);
        if (myFile.getFile()!=NULL)
        {
            VArchive arW(&myFile,VArchive::LOAD);
            unsigned int archiveLength=(unsigned int)myFile.getLength();

            _scriptText.resize(archiveLength,' ');
            for (unsigned int i=0;i<archiveLength;i++)
                arW >> _scriptText[i];
            arW.close();
            myFile.close();
        }
    }
}

bool CLuaScriptObject::shouldAutoYield()
{
    bool retVal=false;
    if ( (_forbidAutoYieldingLevel==0)&&(_forbidOverallYieldingLevel==0) )
        retVal=VDateTime::getTimeDiffInMs(_timeForNextAutoYielding)>0;
    if (retVal)
        _timeForNextAutoYielding=VDateTime::getTimeInMs()+_delayForAutoYielding;
    return(retVal);
}

bool CLuaScriptObject::canManualYield() const
{
    return(_forbidOverallYieldingLevel==0);
}

void CLuaScriptObject::setDelayForAutoYielding(int d)
{
    if (d<0)
        d=0;
    if (d>10000)
        d=10000;
    _delayForAutoYielding=d;
}

int CLuaScriptObject::changeAutoYieldingForbidLevel(int dx,bool absolute)
{
    int retVal=_forbidAutoYieldingLevel;
    if (absolute)
        _forbidAutoYieldingLevel=dx;
    else
    {
        _forbidAutoYieldingLevel+=dx;
        if (_forbidAutoYieldingLevel<0)
            _forbidAutoYieldingLevel=0;
    }
    return(retVal);
}

int CLuaScriptObject::getAutoYieldingForbidLevel() const
{
    return(_forbidAutoYieldingLevel);
}

int CLuaScriptObject::changeOverallYieldingForbidLevel(int dx,bool absolute)
{
    int retVal=_forbidOverallYieldingLevel;
    if (absolute)
        _forbidOverallYieldingLevel=dx;
    else
    {
        _forbidOverallYieldingLevel+=dx;
        if (_forbidOverallYieldingLevel<0)
            _forbidOverallYieldingLevel=0;
    }
    return(retVal);
}


void CLuaScriptObject::fromBufferToFile() const
{
    if (App::userSettings->externalScriptEditor.size()>0)
    { // write file
        if ( (App::currentWorld==nullptr)||(App::currentWorld->environment==nullptr)||(!App::currentWorld->environment->getSceneLocked()) )
        {
            std::string fname=App::folders->getExtScriptEditorTempPath()+"/";
            fname.append(_filenameForExternalScriptEditor);

            VFile myFile(fname.c_str(),VFile::CREATE_WRITE|VFile::SHARE_EXCLUSIVE,true);
            if (myFile.getFile()!=NULL)
            {
                VArchive arW(&myFile,VArchive::STORE);
                if (_scriptText.size()>0)
                {
                    for (size_t i=0;i<_scriptText.size();i++)
                        arW << _scriptText[i];
                }
                arW.close();
                myFile.close();
            }
        }
    }
}

std::string CLuaScriptObject::getSystemCallbackString(int calltype,bool callTips)
{
    if (calltype==sim_syscb_init)
    {
        std::string r("sysCall_init");
        if (callTips)
            r+="()\nCalled when the script is initialized.";
        return(r);
    }
    if (calltype==sim_syscb_cleanup)
    {
        std::string r("sysCall_cleanup");
        if (callTips)
            r+="()\nCalled when the script is destroyed.";
        return(r);
    }
    if (calltype==sim_syscb_nonsimulation)
    {
        std::string r("sysCall_nonSimulation");
        if (callTips)
            r+="()\nCalled when simulation is not running.";
        return(r);
    }
    if (calltype==sim_syscb_beforemainscript)
    {
        std::string r("sysCall_beforeMainScript");
        if (callTips)
            r+="()\nCalled just before the main script is called.\nCan be used to temporarily suppress calling the main script.";
        return(r);
    }

    if (calltype==sim_syscb_beforesimulation)
    {
        std::string r("sysCall_beforeSimulation");
        if (callTips)
            r+="()\nCalled just before simulation starts.";
        return(r);
    }
    if (calltype==sim_syscb_aftersimulation)
    {
        std::string r("sysCall_afterSimulation");
        if (callTips)
            r+="()\nCalled just after simulation ended.";
        return(r);
    }
    if (calltype==sim_syscb_actuation)
    {
        std::string r("sysCall_actuation");
        if (callTips)
            r+="()\nCalled in the actuation phase.";
        return(r);
    }
    if (calltype==sim_syscb_sensing)
    {
        std::string r("sysCall_sensing");
        if (callTips)
            r+="()\nCalled in the sensing phase.";
        return(r);
    }
    if (calltype==sim_syscb_suspended)
    {
        std::string r("sysCall_suspended");
        if (callTips)
            r+="()\nCalled when simulation is suspended.";
        return(r);
    }
    if (calltype==sim_syscb_suspend)
    {
        std::string r("sysCall_suspend");
        if (callTips)
            r+="()\nCalled just before simulation gets suspended.";
        return(r);
    }
    if (calltype==sim_syscb_resume)
    {
        std::string r("sysCall_resume");
        if (callTips)
            r+="()\nCalled just before simulation resumes.";
        return(r);
    }
    if (calltype==sim_syscb_beforeinstanceswitch)
    {
        std::string r("sysCall_beforeInstanceSwitch");
        if (callTips)
            r+="()\nCalled just before an instance switch.";
        return(r);
    }
    if (calltype==sim_syscb_afterinstanceswitch)
    {
        std::string r("sysCall_afterInstanceSwitch");
        if (callTips)
            r+="()\nCalled just after an instance switch.";
        return(r);
    }
    if (calltype==sim_syscb_beforecopy)
    {
        std::string r("sysCall_beforeCopy");
        if (callTips)
            r+="(inData)\nCalled just before objects are copied.";
        return(r);
    }
    if (calltype==sim_syscb_aftercopy)
    {
        std::string r("sysCall_afterCopy");
        if (callTips)
            r+="(inData)\nCalled just after objects were copied.";
        return(r);
    }
    if (calltype==sim_syscb_beforedelete)
    {
        std::string r("sysCall_beforeDelete");
        if (callTips)
            r+="(inData)\nCalled just before objects are deleted.";
        return(r);
    }
    if (calltype==sim_syscb_afterdelete)
    {
        std::string r("sysCall_afterDelete");
        if (callTips)
            r+="(inData)\nCalled just after objects were deleted.";
        return(r);
    }
    if (calltype==sim_syscb_aftercreate)
    {
        std::string r("sysCall_afterCreate");
        if (callTips)
            r+="()\nCalled just after objects were created.";
        return(r);
    }
    if (calltype==sim_syscb_aos_run_old)
    {
        std::string r("sysCall_addOnScriptRun");
        if (callTips)
            r+="()\nCalled when the add-on script is running.";
        return(r);
    }
    if (calltype==sim_syscb_aos_suspend)
    {
        std::string r("sysCall_addOnScriptSuspend");
        if (callTips)
            r+="()\nCalled just before the add-on script execution gets suspended.";
        return(r);
    }
    if (calltype==sim_syscb_aos_resume)
    {
        std::string r("sysCall_addOnScriptResume");
        if (callTips)
            r+="()\nCalled just before the add-on script execution resumes.";
        return(r);
    }

    if (calltype==sim_syscb_jointcallback)
    {
        std::string r("sysCall_jointCallback");
        if (callTips)
            r+="(inData)\nCalled after a dynamic simulation step.";
        return(r);
    }
    if (calltype==sim_syscb_vision)
    {
        std::string r("sysCall_vision");
        if (callTips)
            r+="(inData)\nCalled when a vision sensor requests image processing.";
        return(r);
    }
    if (calltype==sim_syscb_userconfig)
    {
        std::string r("sysCall_userConfig");
        if (callTips)
            r+="()\nCalled when the user double-clicks a user parameter icon.";
        return(r);
    }
    if (calltype==sim_syscb_trigger)
    {
        std::string r("sysCall_trigger");
        if (callTips)
            r+="(inData)\nCalled when the sensor is triggered.";
        return(r);
    }
    if (calltype==sim_syscb_contactcallback)
    {
        std::string r("sysCall_contactCallback");
        if (callTips)
            r+="(inData)\nCalled by the physics engine when two respondable shapes are contacting.";
        return(r);
    }
    if (calltype==sim_syscb_dyncallback)
    {
        std::string r("sysCall_dynCallback");
        if (callTips)
            r+="(inData)\nCalled by the physics engine twice per dynamic simulation pass.";
        return(r);
    }
    if ( (calltype>=sim_syscb_customcallback1)&&(calltype<=sim_syscb_customcallback4) )
    {
        std::string r("sysCall_customCallback");
        r+=boost::lexical_cast<std::string>(calltype-sim_syscb_customcallback1+1);
        if (callTips)
            r+="()\nCan be called by a customized main script.";
        return(r);
    }
    if (calltype==sim_syscb_threadmain)
    {
        std::string r("sysCall_threadmain");
        if (callTips)
            r+="()\nMain function of a threaded child script.";
        return(r);
    }
    if (calltype==sim_syscb_xr)
    {
        std::string r("sysCall_xr");
        if (callTips)
            r+="(...)\nCalled for XReality functionality.";
        return(r);
    }
    return("");
}

std::string CLuaScriptObject::getSystemCallbackExString(int calltype)
{
    if (calltype==sim_syscb_cleanup)
        return("sysCallEx_cleanup");
    if (calltype==sim_syscb_beforeinstanceswitch)
        return("sysCallEx_beforeInstanceSwitch");
    if (calltype==sim_syscb_afterinstanceswitch)
        return("sysCallEx_afterInstanceSwitch");
    if (calltype==sim_syscb_aos_suspend)
        return("sysCallEx_addOnScriptSuspend");
    if (calltype==sim_syscb_aos_resume)
        return("sysCallEx_addOnScriptResume");
    return("");
}

bool CLuaScriptObject::canCallSystemCallback(int scriptType,bool threadedOld,int callType)
{
    if (scriptType==-1)
        return(true);

    if ( threadedOld&&(scriptType==sim_scripttype_childscript) )
    { // for backward compatibility
        if (callType==sim_syscb_threadmain)
            return(true);
        if (callType==sim_syscb_cleanup)
            return(true);
        if (callType==sim_syscb_vision)
            return(true);
        if (callType==sim_syscb_trigger)
            return(true);
    }
    else
    { // For all script types (except for the old threaded scripts):
        if (callType==sim_syscb_init)
            return(true);
        if (callType==sim_syscb_cleanup)
            return(true);
        if (callType==sim_syscb_actuation)
            return(true);
        if (callType==sim_syscb_sensing)
            return(true);
        if (callType==sim_syscb_suspend)
            return(true);
        if (callType==sim_syscb_suspended)
            return(true);
        if (callType==sim_syscb_resume)
            return(true);
        if (callType==sim_syscb_beforecopy)
            return(true);
        if (callType==sim_syscb_aftercopy)
            return(true);
        if (callType==sim_syscb_beforedelete)
            return(true);
        if (callType==sim_syscb_afterdelete)
            return(true);
        if (callType==sim_syscb_aftercreate)
            return(true);
    }
    if ( (scriptType==sim_scripttype_sandboxscript)||(scriptType==sim_scripttype_addonscript)||(scriptType==sim_scripttype_customizationscript) )
    {
        if (callType==sim_syscb_nonsimulation)
            return(true);
        if (callType==sim_syscb_beforesimulation)
            return(true);
        if (callType==sim_syscb_aftersimulation)
            return(true);
        if (callType==sim_syscb_beforemainscript)
            return(true);
        if (callType==sim_syscb_beforeinstanceswitch)
            return(true);
        if (callType==sim_syscb_afterinstanceswitch)
            return(true);
    }
    if (scriptType!=sim_scripttype_childscript)
    {
        if (callType==sim_syscb_xr)
            return(true);
    }
    if (scriptType==sim_scripttype_addonscript)
    {
        if (callType==sim_syscb_aos_run_old) // for backward compatibility
            return(true);
        if (callType==sim_syscb_aos_suspend)
            return(true);
        if (callType==sim_syscb_aos_resume)
            return(true);
    }
    if ( (scriptType==sim_scripttype_customizationscript)||((!threadedOld)&&(scriptType==sim_scripttype_childscript)) )
    {
        if (callType==sim_syscb_jointcallback)
            return(true);
        if (callType==sim_syscb_vision)
            return(true);
        if (callType==sim_syscb_trigger)
            return(true);
        if (callType==sim_syscb_contactcallback)
            return(true);
        if (callType==sim_syscb_dyncallback)
            return(true);
        if (callType==sim_syscb_userconfig)
            return(true);
        if ( (callType>=sim_syscb_customcallback1)&&(callType<=sim_syscb_customcallback4) )
            return(true);
    }
    return(false);
}

std::vector<std::string> CLuaScriptObject::getAllSystemCallbackStrings(int scriptType,bool threaded,bool callTips)
{
    const int ct[]={
                 sim_syscb_init,
                 sim_syscb_cleanup,
                 sim_syscb_nonsimulation,
                 sim_syscb_beforemainscript,
                 sim_syscb_beforesimulation,
                 sim_syscb_aftersimulation,
                 sim_syscb_actuation,
                 sim_syscb_sensing,
                 sim_syscb_suspended,
                 sim_syscb_suspend,
                 sim_syscb_resume,
                 sim_syscb_beforeinstanceswitch,
                 sim_syscb_afterinstanceswitch,
                 sim_syscb_beforecopy,
                 sim_syscb_aftercopy,
                 sim_syscb_beforedelete,
                 sim_syscb_afterdelete,
                 sim_syscb_aftercreate,
                 sim_syscb_aos_run_old, // for backward compatibility
                 sim_syscb_aos_suspend,
                 sim_syscb_aos_resume,
                 sim_syscb_jointcallback,
                 sim_syscb_contactcallback,
                 sim_syscb_dyncallback,
                 sim_syscb_vision,
                 sim_syscb_trigger,
                 sim_syscb_customcallback1,
                 sim_syscb_customcallback2,
                 sim_syscb_customcallback3,
                 sim_syscb_customcallback4,
                 sim_syscb_threadmain,
                 sim_syscb_userconfig,
                 sim_syscb_xr,
                 -1
            };

    std::vector<std::string> retVal;
    size_t i=0;
    while (ct[i]!=-1)
    {
        if (scriptType!=-1)
        {
            if (canCallSystemCallback(scriptType,threaded,ct[i]))
                retVal.push_back(getSystemCallbackString(ct[i],callTips));
        }
        else
            retVal.push_back(getSystemCallbackString(ct[i],callTips));
        i++;
    }
    return(retVal);
}

bool CLuaScriptObject::getContainsJointCallbackFunction() const
{
    return(_containsJointCallbackFunction);
}

bool CLuaScriptObject::getContainsContactCallbackFunction() const
{
    return(_containsContactCallbackFunction);
}

bool CLuaScriptObject::getContainsDynCallbackFunction() const
{
    return(_containsDynCallbackFunction);
}

bool CLuaScriptObject::getContainsVisionCallbackFunction() const
{
    return(_containsVisionCallbackFunction);
}

bool CLuaScriptObject::getContainsTriggerCallbackFunction() const
{
    return(_containsTriggerCallbackFunction);
}

bool CLuaScriptObject::getContainsUserConfigCallbackFunction() const
{
    return(_containsUserConfigCallbackFunction);
}

std::string CLuaScriptObject::getAndClearLastStackTraceback()
{
    std::string retVal=_lastStackTraceback;
    _lastStackTraceback.clear();
    return(retVal);
}

void CLuaScriptObject::setLastError(const char* err)
{
    _lastError=err;
}

std::string CLuaScriptObject::getAndClearLastError()
{
    std::string retVal=_lastError;
    _lastError.clear();
    return(retVal);
}

double CLuaScriptObject::getRandomDouble()
{
    return(double(_randGen())/double(_randGen.max()));
}

void CLuaScriptObject::setRandomSeed(unsigned int s)
{
    _randGen.seed(s);
}

void CLuaScriptObject::setRaiseErrors_backCompatibility(bool raise)
{
    _raiseErrors_backCompatibility=raise;
}

bool CLuaScriptObject::getRaiseErrors_backCompatibility() const
{
    return(_raiseErrors_backCompatibility);
}


int CLuaScriptObject::getScriptExecutionTimeInMs() const
{
    if (_timeOfPcallStart<0) // happens sometimes when calling luaWrap_luaL_doString
        return(0);
    return(VDateTime::getTimeDiffInMs(_timeOfPcallStart));
}

void CLuaScriptObject::setObjectCustomData(int header,const char* data,int dataLength)
{
    if (_customObjectData==nullptr)
        _customObjectData=new CCustomData();
    _customObjectData->setData(header,data,dataLength);
}
int CLuaScriptObject::getObjectCustomDataLength(int header) const
{
    if (_customObjectData==nullptr)
        return(0);
    return(_customObjectData->getDataLength(header));
}
void CLuaScriptObject::getObjectCustomData(int header,char* data) const
{
    if (_customObjectData==nullptr)
        return;
    _customObjectData->getData(header,data);
}

bool CLuaScriptObject::getObjectCustomDataHeader(int index,int& header) const
{
    if (_customObjectData==nullptr)
        return(false);
    return(_customObjectData->getHeader(index,header));
}



void CLuaScriptObject::setObjectCustomData_tempData(int header,const char* data,int dataLength)
{
    if (_customObjectData_tempData==nullptr)
        _customObjectData_tempData=new CCustomData();
    _customObjectData_tempData->setData(header,data,dataLength);
}
int CLuaScriptObject::getObjectCustomDataLength_tempData(int header) const
{
    if (_customObjectData_tempData==nullptr)
        return(0);
    return(_customObjectData_tempData->getDataLength(header));
}
void CLuaScriptObject::getObjectCustomData_tempData(int header,char* data) const
{
    if (_customObjectData_tempData==nullptr)
        return;
    _customObjectData_tempData->getData(header,data);
}

bool CLuaScriptObject::getObjectCustomDataHeader_tempData(int index,int& header) const
{
    if (_customObjectData_tempData==nullptr)
        return(false);
    return(_customObjectData_tempData->getHeader(index,header));
}



void CLuaScriptObject::getPreviousEditionWindowPosAndSize(int posAndSize[4]) const
{
    for (int i=0;i<4;i++)
        posAndSize[i]=_previousEditionWindowPosAndSize[i];
}

void CLuaScriptObject::setPreviousEditionWindowPosAndSize(const int posAndSize[4])
{
    for (int i=0;i<4;i++)
        _previousEditionWindowPosAndSize[i]=posAndSize[i];
}

std::string CLuaScriptObject::getAddOnName() const
{
    return(_addOnName);
}

int CLuaScriptObject::getAddOnExecutionState() const
{
    return(_addOn_executionState);
}

void CLuaScriptObject::setExecuteJustOnce(bool justOnce)
{
    _executeJustOnce=justOnce;
}

bool CLuaScriptObject::getExecuteJustOnce() const
{
    return(_executeJustOnce);
}

void CLuaScriptObject::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    if ( (_scriptType==sim_scripttype_mainscript)||(_scriptType==sim_scripttype_childscript) )
    {
        _initialValuesInitialized=true;
//        if (_scriptParameters_backCompatibility!=nullptr)
//            _scriptParameters_backCompatibility->initializeInitialValues(simulationIsRunning);
        if (_outsideCommandQueue!=nullptr)
            _outsideCommandQueue->initializeInitialValues(simulationAlreadyRunning);
    }
}

void CLuaScriptObject::simulationAboutToStart()
{
    if ( (_scriptType==sim_scripttype_mainscript)||(_scriptType==sim_scripttype_childscript) )
    {
        killLuaState(); // should already be reset! (should have been done in simulationEnded routine)
        _numberOfPasses=0;
        _automaticCascadingCallsDisabled_OLD=false;
        initializeInitialValues(false);
        _warningAboutSimHandleChildScriptAlreadyIssued_oldCompatibility_7_8_2014=false;
        _warning_simRMLPosition_oldCompatibility_30_8_2014=false;
        _warning_simRMLVelocity_oldCompatibility_30_8_2014=false;
        _warning_simGetMpConfigForTipPose_oldCompatibility_21_1_2016=false;
        _warning_simFindIkPath_oldCompatibility_2_2_2016=false;
    }
}

void CLuaScriptObject::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if ( (_scriptType==sim_scripttype_mainscript)||(_scriptType==sim_scripttype_childscript) )
    {
        if (_outsideCommandQueue!=nullptr)
            _outsideCommandQueue->simulationEnded();
        _scriptTextExec.clear();
        if (_initialValuesInitialized&&App::currentWorld->simulation->getResetSceneAtSimulationEnd())
        {

        }
        _initialValuesInitialized=false;
    }
}

void CLuaScriptObject::simulationAboutToEnd()
{ // Added this on 7/8/2014.
    if ( (_scriptType==sim_scripttype_mainscript)||(_scriptType==sim_scripttype_childscript) )
        killLuaState(); // this has to happen while simulation is still running!!
}

void CLuaScriptObject::setCustScriptDisabledDSim_compatibilityMode_DEPRECATED(bool disabled)
{
    _custScriptDisabledDSim_compatibilityMode_DEPRECATED=disabled;
}

bool CLuaScriptObject::getCustScriptDisabledDSim_compatibilityMode_DEPRECATED() const
{
    return(_custScriptDisabledDSim_compatibilityMode_DEPRECATED);
}

void CLuaScriptObject::setCustomizationScriptCleanupBeforeSave_DEPRECATED(bool doIt)
{
    _customizationScriptCleanupBeforeSave_DEPRECATED=doIt;
}

bool CLuaScriptObject::getCustomizationScriptCleanupBeforeSave_DEPRECATED() const
{
    return(_customizationScriptCleanupBeforeSave_DEPRECATED);
}

bool CLuaScriptObject::hasCustomizationScripAnyChanceToGetExecuted(bool forCleanUpSection,bool whenSimulationRuns) const
{
    if (_scriptIsDisabled)
        return(false);
    // Following 2 since 23/1/2017
    if (forCleanUpSection)
        return(true);
    CSceneObject* obj=nullptr;
    if (_scriptType==sim_scripttype_customizationscript)
    {
        obj=App::currentWorld->sceneObjects->getObjectFromHandle(getObjectHandleThatScriptIsAttachedTo_customization());
        if (!App::userSettings->runCustomizationScripts)
            return(false);
        if (whenSimulationRuns&&_custScriptDisabledDSim_compatibilityMode_DEPRECATED&&_compatibilityModeOrFirstTimeCall_sysCallbacks)
            return(false);
    }
    if (obj==nullptr)
        return(false); // can happen when the object attached to the script is already destroyed.
    if (obj->getCumulativeModelProperty()&sim_modelproperty_scripts_inactive)
        return(false);
    return(true);
}

void CLuaScriptObject::setExecutionOrder(int order)
{
    _executionOrder=tt::getLimitedInt(sim_scriptexecorder_first,sim_scriptexecorder_last,order);
}

int CLuaScriptObject::getExecutionOrder() const
{
    return(_executionOrder);
}

void CLuaScriptObject::setDebugLevel(int l)
{
    _debugLevel=l;
}

int CLuaScriptObject::getDebugLevel() const
{
    return(_debugLevel);
}

void CLuaScriptObject::setTreeTraversalDirection(int dir)
{
    _treeTraversalDirection=tt::getLimitedInt(sim_scripttreetraversal_reverse,sim_scripttreetraversal_parent,dir);
}

int CLuaScriptObject::getTreeTraversalDirection() const
{
    return(_treeTraversalDirection);
}

void CLuaScriptObject::setScriptIsDisabled(bool isDisabled)
{
    _scriptIsDisabled=isDisabled;
}

bool CLuaScriptObject::getScriptIsDisabled() const
{
    return(_scriptIsDisabled);
}

bool CLuaScriptObject::getScriptEnabledAndNoErrorRaised() const
{
    return( (!_scriptIsDisabled)&&(_executionState!=execState_compilationError)&&(_executionState!=execState_runtimeError) );
}

int CLuaScriptObject::getScriptType() const
{
    return(_scriptType);
}

void CLuaScriptObject::flagForDestruction()
{
    _flaggedForDestruction=true;
}

bool CLuaScriptObject::getFlaggedForDestruction() const
{
    return(_flaggedForDestruction);
}

void CLuaScriptObject::setAddOnScriptAutoRun()
{
    _addOn_executionState=-1;
}

bool CLuaScriptObject::setScriptTextFromFile(const char* filename)
{
    _scriptText="";
    bool retVal=false;
    if (VFile::doesFileExist(filename))
    {
        try
        {
            VFile file(filename,VFile::READ|VFile::SHARE_DENY_NONE);
            VArchive archive(&file,VArchive::LOAD);
            unsigned int archiveLength=(unsigned int)file.getLength();
            char* script=new char[archiveLength+1];
            for (int i=0;i<int(archiveLength);i++)
                archive >> script[i];
            script[archiveLength]=0;
            setScriptText(script);
            delete[] script;
            archive.close();
            file.close();
            retVal=true;
        }
        catch(VFILE_EXCEPTION_TYPE e)
        {
        }
    }
    return(retVal);
}

void CLuaScriptObject::setScriptText(const char* scriptTxt)
{
    EASYLOCK(_localMutex);

    _scriptText="";
    if (scriptTxt!=nullptr)
        _scriptText=scriptTxt;
    fromBufferToFile();
}

const char* CLuaScriptObject::getScriptText()
{
    fromFileToBuffer();
    return(_scriptText.c_str());
}

int CLuaScriptObject::getScriptHandle() const
{
    return(_scriptHandle);
}

int CLuaScriptObject::getScriptUniqueID() const
{
    return(_scriptUniqueId);
}

void CLuaScriptObject::setScriptHandle(int newHandle)
{
    _scriptHandle=newHandle;
}

bool CLuaScriptObject::isSceneScript() const
{
    return ( (_scriptType==sim_scripttype_mainscript)||(_scriptType==sim_scripttype_childscript)||(_scriptType==sim_scripttype_customizationscript) );
}

std::string CLuaScriptObject::getDescriptiveName() const
{
    std::string retVal;
    if (_scriptType==sim_scripttype_mainscript)
        retVal+="Main script";
    if ( (_scriptType==sim_scripttype_childscript)||(_scriptType==sim_scripttype_customizationscript) )
    {
        if (_scriptType==sim_scripttype_childscript)
        {
            if (_threadedExecution)
                retVal+="Child script, threaded (deprecated, backward compatibility version)";
            else
                retVal+="Child script";
        }
        else
            retVal+="Customization script";
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_objectHandleAttachedTo);
        if (it==nullptr)
            retVal+=" (unassociated)";
        else
        {
            retVal+=" (";
            retVal+=it->getObjectName();
            retVal+=")";
        }
    }
    if (_scriptType==sim_scripttype_addonscript)
    {
        retVal+="Add-on script (";
        retVal+=_addOnName;
        retVal+=")";
    }
    if (_scriptType==sim_scripttype_addonfunction)
    {
        retVal+="Add-on function (";
        retVal+=_addOnName;
        retVal+=")";
    }
    if (_scriptType==sim_scripttype_sandboxscript)
        retVal+="Sandbox script";
    return(retVal);
}

std::string CLuaScriptObject::getShortDescriptiveName() const
{
    std::string retVal;
    if (_scriptType==sim_scripttype_mainscript)
        retVal+="mainScript";
    if ( (_scriptType==sim_scripttype_childscript)||(_scriptType==sim_scripttype_customizationscript) )
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_objectHandleAttachedTo);
        if (it==nullptr)
            retVal+="???";
        else
            retVal+=it->getObjectName();
        if (_scriptType==sim_scripttype_childscript)
            retVal+="@childScript";
        else
            retVal+="@customizationScript";
    }
    if (_scriptType==sim_scripttype_addonscript)
    {
        retVal+=_addOnName;
        retVal+="@addOnScript";
    }
    if (_scriptType==sim_scripttype_addonfunction)
    {
        retVal+=_addOnName;
        retVal+="@addOnFunction";
    }
    if (_scriptType==sim_scripttype_sandboxscript)
        retVal+="sandboxScript";
    return(retVal);
}

void CLuaScriptObject::setAddOnName(const char* name)
{
    _addOnName=name;
}

int CLuaScriptObject::getScriptNameIndexNumber() const
{
    int retVal=-1;
    if ( (_scriptType==sim_scripttype_childscript)||(_scriptType==sim_scripttype_customizationscript) )
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_objectHandleAttachedTo);
        if (it!=nullptr)
            retVal=tt::getNameSuffixNumber(it->getObjectName().c_str(),true);
    }
    return(retVal);
}

std::string CLuaScriptObject::getScriptPseudoName() const
{
    if ( (_scriptType==sim_scripttype_childscript)||(_scriptType==sim_scripttype_customizationscript) )
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_objectHandleAttachedTo);
        if (it!=nullptr)
            return(it->getObjectName());
    }
    if ( (_scriptType==sim_scripttype_addonscript)||(_scriptType==sim_scripttype_addonfunction) )
        return(_addOnName);
    return("");
}

void CLuaScriptObject::setThreadedExecution(bool threadedExec)
{
    if (threadedExec)
    {
        if (_scriptType==sim_scripttype_childscript)
            _threadedExecution=true;
    }
    else
        _threadedExecution=false;
}

bool CLuaScriptObject::getThreadedExecution() const
{
    return(_threadedExecution);
}

bool CLuaScriptObject::getThreadedExecutionIsUnderWay() const
{
    return(_threadedExecutionUnderWay);
}

void CLuaScriptObject::performSceneObjectLoadingMapping(const std::vector<int>* map)
{
    if (App::currentWorld->sceneObjects!=nullptr)
        _objectHandleAttachedTo=CWorld::getLoadingMapping(map,_objectHandleAttachedTo);
}

bool CLuaScriptObject::announceSceneObjectWillBeErased(int objectHandle,bool copyBuffer)
{ // script will be erased if attached to objectHandle (if threaded simulation is not running!)
    bool retVal=false;
    if (copyBuffer)
        retVal=(_objectHandleAttachedTo==objectHandle);
    else
    {
        bool closeCodeEditor=false;
        if (_objectHandleAttachedTo==objectHandle)
        {
            if (_scriptType==sim_scripttype_childscript)
            {
                closeCodeEditor=true;
                if (!App::currentWorld->simulation->isSimulationStopped()) // Removed the if(_threadedExecution()) thing on 2008/12/08
                { // threaded scripts cannot be directly erased, since the Lua state needs to be cleared in the thread that created it
                    _objectHandleAttachedTo=-1; // This is for a potential threaded simulation running
                    _flaggedForDestruction=true;
                    retVal=!_inExecutionNow; // from false to !_inExecutionNow on 8/9/2016
                }
                else
                    retVal=true;
            }
            if (_scriptType==sim_scripttype_customizationscript)
            {
                closeCodeEditor=true;
                _flaggedForDestruction=true;
                retVal=!_inExecutionNow; // from false to !_inExecutionNow on 26/8/2016 (i.e. no delayed destruction anymore. Important since the clean-up section of custom. scripts can contain code that refers to the attached object, etc.)
            }
        }
        if (closeCodeEditor)
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
                App::mainWindow->codeEditorContainer->closeFromScriptHandle(_scriptHandle,_previousEditionWindowPosAndSize,true);
#endif
        }
    }
    return(retVal);
}

int CLuaScriptObject::flagScriptForRemoval()
{ // retVal: 0--> cannot be removed, 1 --> will be removed in a delayed manner, 2--> can be removed now
#ifdef SIM_WITH_GUI
    if (App::mainWindow!=nullptr)
        App::mainWindow->codeEditorContainer->closeFromScriptHandle(_scriptHandle,_previousEditionWindowPosAndSize,true);
#endif

    if (App::currentWorld->simulation->isSimulationStopped())
    {
        if ( (_scriptType==sim_scripttype_mainscript)||(_scriptType==sim_scripttype_childscript) )
            return(2);
    }
    if (_scriptType==sim_scripttype_customizationscript)
    {
        _flaggedForDestruction=true;
        return(1);
    }
    return(0);
}

int CLuaScriptObject::getObjectHandleThatScriptIsAttachedTo_child() const
{
    if (_scriptType==sim_scripttype_childscript)
        return(_objectHandleAttachedTo);
    return(-1);
}

int CLuaScriptObject::getObjectHandleThatScriptIsAttachedTo_customization() const
{
    if (_scriptType==sim_scripttype_customizationscript)
        return(_objectHandleAttachedTo);
    return(-1);
}

int CLuaScriptObject::getObjectHandleThatScriptIsAttachedTo() const
{
    return(_objectHandleAttachedTo);
}

void CLuaScriptObject::setObjectHandleThatScriptIsAttachedTo(int newObjectHandle)
{
    if (newObjectHandle!=-1)
    {
        if ( (_scriptType==sim_scripttype_childscript)||(_scriptType==sim_scripttype_customizationscript) )
            _objectHandleAttachedTo=newObjectHandle;
    }
    else
        _objectHandleAttachedTo=-1;
}

int CLuaScriptObject::getNumberOfPasses() const
{
    return(_numberOfPasses);
}

void CLuaScriptObject::setNumberOfPasses(int p)
{
    _numberOfPasses=p;
}

void CLuaScriptObject::resetCalledInThisSimulationStep()
{
    _calledInThisSimulationStep=false;
}

bool CLuaScriptObject::getCalledInThisSimulationStep() const
{
    return(_calledInThisSimulationStep);
}

int CLuaScriptObject::runMainScript(int optionalCallType,const CInterfaceStack* inStack,CInterfaceStack* outStack,bool* functionPresent)
{
    TRACE_INTERNAL;
    if (functionPresent!=nullptr)
        functionPresent[0]=true; // we only return false if we know for sure the function is not there (if the script contains an error, we can't know for sure)

    int retVal=sim_script_no_error;
    if (!CThreadPool::getSimulationEmergencyStop())
    {
        if (!_scriptIsDisabled)
            retVal=_runMainScript(optionalCallType,inStack,outStack,functionPresent);
    }
    else
        CThreadPool::handleAllThreads_withResumeLocation(-1); // Added on 24/02/2011 (important, otherwise threads might never resume!!!)

    return(retVal);
}

int CLuaScriptObject::_runMainScript(int optionalCallType,const CInterfaceStack* inStack,CInterfaceStack* outStack,bool* functionPresent)
{
    TRACE_INTERNAL;
    if (functionPresent!=nullptr)
        functionPresent[0]=true; // we only return false if we know for sure the function is not there (if the script contains an error, we can't know for sure)
    int retVal=sim_script_no_error;

    if (optionalCallType==-1)
    {
        App::currentWorld->embeddedScriptContainer->resetScriptFlagCalledInThisSimulationStep();
        int startT=VDateTime::getTimeInMs();

        if (App::currentWorld->simulation->getSimulationState()==sim_simulation_advancing_firstafterstop)
            retVal|=_runMainScriptNow(sim_syscb_init,inStack,outStack,nullptr);

        if (_compatibilityModeOrFirstTimeCall_sysCallbacks)
            retVal|=_runMainScriptNow(sim_syscb_actuation,inStack,outStack,nullptr);
        else
        {
            retVal|=_runMainScriptNow(sim_syscb_actuation,inStack,outStack,nullptr);
            retVal|=_runMainScriptNow(sim_syscb_sensing,inStack,outStack,nullptr);
        }

        if (App::currentWorld->simulation->getSimulationState()==sim_simulation_advancing_lastbeforestop)
            retVal|=_runMainScriptNow(sim_syscb_cleanup,inStack,outStack,nullptr);

        App::worldContainer->calcInfo->setMainScriptExecutionTime(VDateTime::getTimeInMs()-startT);
        App::worldContainer->calcInfo->setSimulationScriptExecCount(App::currentWorld->embeddedScriptContainer->getCalledScriptsCountInThisSimulationStep(true));
    }
    else
    {
        if (!_compatibilityModeOrFirstTimeCall_sysCallbacks)
            retVal|=_runMainScriptNow(optionalCallType,inStack,outStack,functionPresent); // this only supported with the new calling method
    }
    return(retVal);
}

int CLuaScriptObject::_runMainScriptNow(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack,bool* functionPresent)
{
    if (functionPresent!=nullptr)
        functionPresent[0]=true; // we only return false if we know for sure the function is not there (if the script contains an error, we can't know for sure)
    App::currentWorld->embeddedScriptContainer->setInMainScriptNow(true,VDateTime::getTimeInMs());
    int retVal=_runScriptOrCallScriptFunction(callType,inStack,outStack);
    if ( (retVal==0)&&(functionPresent!=nullptr) )
        functionPresent[0]=false;
    App::currentWorld->embeddedScriptContainer->setInMainScriptNow(false,0);
    if (retVal<0)
        return(sim_script_lua_error);
    return(sim_script_no_error);
}

int CLuaScriptObject::runNonThreadedChildScript(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack)
{ // retval: -2: compil error, -1: runtimeError, 0: function not there, 1: ok
    TRACE_INTERNAL;

    int retVal=0;
    bool ok=true;
    if (L==nullptr)
    {
        if (callType>sim_syscb_sensing)
            ok=false; // specialized callbacks require the script to be initialized
        if (ok)
            retVal=_runNonThreadedChildScriptNow(sim_syscb_init,nullptr,nullptr);
    }

    if ( ok&&(callType!=sim_syscb_init) ) // sim_syscb_init was already executed just above!
        retVal=_runNonThreadedChildScriptNow(callType,inStack,outStack);

    return(retVal);
}

int CLuaScriptObject::_runNonThreadedChildScriptNow(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack)
{ // retval: -2: compil error, -1: runtimeError, 0: function not there, 1: ok
    int retVal=_runScriptOrCallScriptFunction(callType,inStack,outStack);
    return(retVal);
}

bool CLuaScriptObject::launchThreadedChildScript()
{
    TRACE_INTERNAL;
    if (_threadedExecutionUnderWay)
        return(false); // this script is being executed by another thread!

    if (_scriptTextExec.size()==0)
    {
        fromFileToBuffer();
        _scriptTextExec.assign(_scriptText.begin(),_scriptText.end());
    }

    if (_executeJustOnce&&(_numberOfPasses>0))
        return(false);

    if (CThreadPool::getSimulationStopRequested())// will also return true in case of emergency stop request
        return(false);

    _threadedExecutionUnderWay=true;
    _globalMutex.lock("CLuaScriptObject::launchThreadedChildScript()");
    toBeCalledByThread.push_back(this);
    _globalMutex.unlock();
    _threadedScript_associatedFiberOrThreadID=CThreadPool::createNewThread(_startAddressForThreadedScripts);
    _calledInThisSimulationStep=true;
    CThreadPool::switchToThread(_threadedScript_associatedFiberOrThreadID);
    return(true);
}

void CLuaScriptObject::_launchThreadedChildScriptNow()
{
    TRACE_INTERNAL;
    _executionState=execState_initialized;
    _timeForNextAutoYielding=VDateTime::getTimeInMs()+_delayForAutoYielding;
    _forbidOverallYieldingLevel=0;

    if (L==nullptr)
    {
        L=initializeNewLuaState(_scriptHandle,getScriptNameIndexNumber(),_debugLevel);
        _forbidAutoYieldingLevel=0;
        _calledInThisSimulationStep=false;
        _raiseErrors_backCompatibility=true;
        _randGen.seed(123456);
    }
    int oldTop=luaWrap_lua_gettop(L);   // We store lua's stack

    luaWrap_luaL_dostring(L,"sim_call_type=-1"); // for backward compatibility

    if (_luaLoadBuffer(L,_scriptTextExec.c_str(),_scriptTextExec.size(),getShortDescriptiveName().c_str()))
    {
        luaWrap_lua_getglobal(L,"debug");
        luaWrap_lua_getfield(L,-1,"traceback");
        luaWrap_lua_remove(L,-2);
        int argCnt=0;
        int errindex=-argCnt-2;
        luaWrap_lua_insert(L,errindex);
        if (_luaPCall(L,argCnt,luaWrapGet_LUA_MULTRET(),errindex,"scriptChunk")!=0)
        { // a runtime error occurred!
            // We have to exit the thread free mode if we are still in there (the instance should automatically be restored when this thread resumes):
            if (CThreadPool::isThreadInFreeMode())
                CThreadPool::setThreadFreeMode(false);

            std::string errMsg;
            if (luaWrap_lua_isstring(L,-1))
                errMsg=std::string(luaWrap_lua_tostring(L,-1));
            else
                errMsg="(error unknown)";
            luaWrap_lua_pop(L,1); // pop error from stack
            _announceErrorWasRaisedAndDisableScript(errMsg.c_str(),true);
        }
        else
        {
            luaWrap_lua_getglobal(L,getSystemCallbackString(sim_syscb_vision,false).c_str());
            _containsVisionCallbackFunction=luaWrap_lua_isfunction(L,-1);
            luaWrap_lua_getglobal(L,getSystemCallbackString(sim_syscb_trigger,false).c_str());
            _containsTriggerCallbackFunction=luaWrap_lua_isfunction(L,-1);

            int calls[2]={sim_syscb_threadmain,sim_syscb_cleanup};
            for (size_t callIndex=0;callIndex<2;callIndex++)
            {
                // Push the function name onto the stack (will be automatically popped from stack after _luaPCall):
                std::string funcName(getSystemCallbackString(calls[callIndex],false));
                luaWrap_lua_getglobal(L,funcName.c_str());
                if (luaWrap_lua_isfunction(L,-1))
                { // ok, the function exists!
                    // Push the arguments onto the stack (will be automatically popped from stack after _luaPCall):
                    luaWrap_lua_getglobal(L,"debug");
                    luaWrap_lua_getfield(L,-1,"traceback");
                    luaWrap_lua_remove(L,-2);
                    int argCnt=0;
                    int errindex=-argCnt-2;
                    luaWrap_lua_insert(L,errindex);
                    if (_luaPCall(L,argCnt,luaWrapGet_LUA_MULTRET(),errindex,funcName.c_str())!=0)
                    { // a runtime error occurred!
                        if (CThreadPool::isThreadInFreeMode())
                            CThreadPool::setThreadFreeMode(false);
                        std::string errMsg;
                        if (luaWrap_lua_isstring(L,-1))
                            errMsg=std::string(luaWrap_lua_tostring(L,-1));
                        else
                            errMsg="(error unknown)";
                        luaWrap_lua_pop(L,1); // pop error from stack
                        _announceErrorWasRaisedAndDisableScript(errMsg.c_str(),true);
                    }
                }
                else
                    luaWrap_lua_pop(L,1); // pop the function name
                _handleSimpleSysExCalls(calls[callIndex]);
            }
            if (CThreadPool::isThreadInFreeMode())
                CThreadPool::setThreadFreeMode(false);
        }
    }
    else
    { // A compilation error occurred!
        std::string errMsg;
        if (luaWrap_lua_isstring(L,-1))
            errMsg=std::string(luaWrap_lua_tostring(L,-1));
        else
            errMsg="(error unknown)";
        luaWrap_lua_pop(L,1); // pop error from stack
        _announceErrorWasRaisedAndDisableScript(errMsg.c_str(),false);
    }
    _numberOfPasses++;
    luaWrap_lua_settop(L,oldTop);       // We restore lua's stack
}

int CLuaScriptObject::resumeThreadedChildScriptIfLocationMatch(int resumeLocation)
{ // returns 0 (not resumed) or 1
    bool res=(CThreadPool::handleThread_ifHasResumeLocation(_threadedScript_associatedFiberOrThreadID,false,resumeLocation)==1);
    _calledInThisSimulationStep=_calledInThisSimulationStep||res;
    return(res);
}

int CLuaScriptObject::runCustomizationScript(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack)
{ // retval: -2: compil error, -1: runtimeError, 0: function not there or script not executed, 1: ok
    TRACE_INTERNAL;
    // Can we run the script? Was it temporarily disabled? Is it allowed to run during simulation? etc. is checked here:
    if (!hasCustomizationScripAnyChanceToGetExecuted(callType==sim_syscb_cleanup,App::currentWorld->simulation->isSimulationRunning()))
        return(0);

    int retVal=0;
    bool ok=true;
    if (L==nullptr)
    {   // The first time we call this script
        if (callType>sim_syscb_sensing)
            ok=false; // specialized callbacks require the script to be initialized
        else
        {
            // Generate the sim_syscb_init call:
            retVal=_runScriptOrCallScriptFunction(sim_syscb_init,inStack,outStack);

            // Above call might have disabled the script. Check again:
            if (!hasCustomizationScripAnyChanceToGetExecuted(false,App::currentWorld->simulation->isSimulationRunning()))
                return(0);

            if (App::currentWorld->simulation->isSimulationPaused())
            {   // Here we have copied an object with customization script into the scene while simulation was paused.
                // We need to generate the sim_syscb_suspend call from here (special case):
                retVal=_runScriptOrCallScriptFunction(sim_syscb_suspend,inStack,outStack);

                // Above call might have disabled the script. Check again:
                if (!hasCustomizationScripAnyChanceToGetExecuted(false,App::currentWorld->simulation->isSimulationRunning()))
                    return(0);
            }
        }
    }

    if ( ok&&(callType!=sim_syscb_init) ) // sim_syscb_init was already handled just here above
        retVal=_runScriptOrCallScriptFunction(callType,inStack,outStack);
    return(retVal);
}

void CLuaScriptObject::_handleSimpleSysExCalls(int callType)
{
    std::string cbEx(getSystemCallbackExString(callType));
    if (cbEx.size()>0)
    {
        std::string tmp("if "+cbEx+" then "+cbEx+"() end");
        luaWrap_luaL_dostring(L,tmp.c_str());
    }
}

int CLuaScriptObject::_runScriptOrCallScriptFunction(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack)
{ // retval: -2: compil error, -1: runtimeError, 0: function not there, 1: ok

//    if ( (_executionState==execState_compilationError)||((_executionState==execState_runtimeError)&&(callType!=sim_syscb_cleanup)) )
//        return(0); // if compilation previously failed, call nothing. If an error occured previously, allow only cleanup
    _timeForNextAutoYielding=VDateTime::getTimeInMs()+_delayForAutoYielding;
    _forbidOverallYieldingLevel=0;

    if (callType==sim_syscb_init)
        _executionState=execState_initialized;

    if ( (_executionState!=execState_initialized)&&((_executionState==execState_uninitialized)||(callType!=sim_syscb_cleanup)) )
        return(0); // if compilation previously failed, call nothing. If an error occured previously, allow only cleanup

    int retVal;
    if (_scriptTextExec.size()==0)
    {
        fromFileToBuffer();
        _scriptTextExec.assign(_scriptText.begin(),_scriptText.end());
    }
    if (L==nullptr)
    {
        L=initializeNewLuaState(_scriptHandle,getScriptNameIndexNumber(),_debugLevel);
        _calledInThisSimulationStep=false;
        _raiseErrors_backCompatibility=true;
        _randGen.seed(123456);
        _delayForAutoYielding=2;
        _forbidAutoYieldingLevel=0;
        if (_checkIfMixingOldAndNewCallMethods())
        {
            std::string msg(getShortDescriptiveName());
            msg+=": detected a possible attempt to mix the old and new calling methods. For example:";
            msg+="\n         with the old method: if sim_call_type==sim_childscriptcall_initialization then ... end";
            msg+="\n         with the new method: function sysCall_init() ... end";
            App::logMsg(sim_verbosity_warnings,msg.c_str());
        }
    }
    int oldTop=luaWrap_lua_gettop(L);   // We store lua's stack
    if (_compatibilityModeOrFirstTimeCall_sysCallbacks)
    {
        std::string tmp("sim_call_type="); // for backward compatibility
        tmp+=boost::lexical_cast<std::string>(callType);
        luaWrap_luaL_dostring(L,tmp.c_str());
        if (_luaLoadBuffer(L,_scriptTextExec.c_str(),_scriptTextExec.size(),getShortDescriptiveName().c_str()))
        {
            int inputArgs=0;
            if (inStack!=nullptr)
            { // for backward compatibility
                inputArgs=inStack->getStackSize();
                if (inputArgs!=0)
                    inStack->buildOntoLuaStack(L,false);
                //inStack->clear();
            }
            luaWrap_lua_getglobal(L,"debug");
            luaWrap_lua_getfield(L,-1,"traceback");
            luaWrap_lua_remove(L,-2);
            int argCnt=inputArgs;
            int errindex=-argCnt-2;
            luaWrap_lua_insert(L,errindex);
            if (_luaPCall(L,argCnt,luaWrapGet_LUA_MULTRET(),errindex,"scriptChunk")!=0)
            { // a runtime error occurred!
                std::string errMsg;
                if (luaWrap_lua_isstring(L,-1))
                    errMsg=std::string(luaWrap_lua_tostring(L,-1));
                else
                    errMsg="(error unknown)";
                luaWrap_lua_pop(L,1); // pop error from stack
                _announceErrorWasRaisedAndDisableScript(errMsg.c_str(),true);
                retVal=-1;
            }
            else
            {
                if (callType==sim_syscb_init)
                { // here we check if we can enable the new calling method:
                    std::string initCb=getSystemCallbackString(sim_syscb_init,false);
                    luaWrap_lua_getglobal(L,initCb.c_str());
                    _compatibilityModeOrFirstTimeCall_sysCallbacks=!(luaWrap_lua_isfunction(L,-1));
                    luaWrap_lua_pop(L,1);
                    if (!_compatibilityModeOrFirstTimeCall_sysCallbacks)
                        luaWrap_luaL_dostring(L,"sim_call_type=nil");
                }
                else
                { // for backward compatibility:
                    _handleSimpleSysExCalls(callType);
                    int currentTop=luaWrap_lua_gettop(L);
                    int numberOfArgs=currentTop-oldTop-1; // the first arg is linked to the debug mechanism
                    if (outStack!=nullptr)
                        outStack->buildFromLuaStack(L,oldTop+1+1,numberOfArgs); // the first arg is linked to the debug mechanism
                }
                retVal=1;
            }
            _numberOfPasses++;
        }
        else
        { // A compilation error occurred!
            std::string errMsg;
            if (luaWrap_lua_isstring(L,-1))
                errMsg=std::string(luaWrap_lua_tostring(L,-1));
            else
                errMsg="(error unknown)";
            luaWrap_lua_pop(L,1); // pop error from stack
            _announceErrorWasRaisedAndDisableScript(errMsg.c_str(),false);
            retVal=-2;
        }
    }
    if (!_compatibilityModeOrFirstTimeCall_sysCallbacks)
    {
        if (callType==sim_syscb_init)
        { // do this only once
            luaWrap_lua_getglobal(L,getSystemCallbackString(sim_syscb_jointcallback,false).c_str());
            _containsJointCallbackFunction=luaWrap_lua_isfunction(L,-1);
            luaWrap_lua_getglobal(L,getSystemCallbackString(sim_syscb_contactcallback,false).c_str());
            _containsContactCallbackFunction=luaWrap_lua_isfunction(L,-1);
            luaWrap_lua_getglobal(L,getSystemCallbackString(sim_syscb_dyncallback,false).c_str());
            _containsDynCallbackFunction=luaWrap_lua_isfunction(L,-1);
            luaWrap_lua_getglobal(L,getSystemCallbackString(sim_syscb_vision,false).c_str());
            _containsVisionCallbackFunction=luaWrap_lua_isfunction(L,-1);
            luaWrap_lua_getglobal(L,getSystemCallbackString(sim_syscb_trigger,false).c_str());
            _containsTriggerCallbackFunction=luaWrap_lua_isfunction(L,-1);
            luaWrap_lua_getglobal(L,getSystemCallbackString(sim_syscb_userconfig,false).c_str());
            _containsUserConfigCallbackFunction=luaWrap_lua_isfunction(L,-1);
            luaWrap_lua_pop(L,6);
        }
        // Push the function name onto the stack (will be automatically popped from stack after _luaPCall):
        std::string funcName(getSystemCallbackString(callType,false));
        luaWrap_lua_getglobal(L,funcName.c_str());
        if ( (callType==sim_syscb_xr)&&(!luaWrap_lua_isfunction(L,-1)) )
        {
            luaWrap_lua_pop(L,1); // pop the function name
            funcName="sysCall_br"; // for backward functionality
            luaWrap_lua_getglobal(L,funcName.c_str());
        }
        if (luaWrap_lua_isfunction(L,-1))
        { // ok, the function exists!
            // Push the arguments onto the stack (will be automatically popped from stack after _luaPCall):
            int inputArgs=0;
            if (inStack!=nullptr)
            {
                inputArgs=inStack->getStackSize();
                if (inputArgs!=0)
                    inStack->buildOntoLuaStack(L,false);
                //inStack->clear();
            }
            luaWrap_lua_getglobal(L,"debug");
            luaWrap_lua_getfield(L,-1,"traceback");
            luaWrap_lua_remove(L,-2);
            int argCnt=inputArgs;
            int errindex=-argCnt-2;
            luaWrap_lua_insert(L,errindex);
            if (_luaPCall(L,argCnt,luaWrapGet_LUA_MULTRET(),errindex,funcName.c_str())!=0)
            { // a runtime error occurred!
                std::string errMsg;
                if (luaWrap_lua_isstring(L,-1))
                    errMsg=std::string(luaWrap_lua_tostring(L,-1));
                else
                    errMsg="(error unknown)";
                luaWrap_lua_pop(L,1); // pop error from stack
                _announceErrorWasRaisedAndDisableScript(errMsg.c_str(),true);
                retVal=-1;
                _calledInThisSimulationStep=true;
            }
            else
            { // return values:
                int currentTop=luaWrap_lua_gettop(L);
                int numberOfArgs=currentTop-oldTop-1; // the first arg is linked to the debug mechanism
                if (outStack!=nullptr)
                    outStack->buildFromLuaStack(L,oldTop+1+1,numberOfArgs); // the first arg is linked to the debug mechanism
                _calledInThisSimulationStep=true;
                retVal=1;
            }
        }
        else
        {
            luaWrap_lua_pop(L,1); // pop the function name
            retVal=0;
        }
        _handleSimpleSysExCalls(callType);
    }
    luaWrap_lua_settop(L,oldTop);       // We restore lua's stack
    return(retVal);
}

int CLuaScriptObject::callSandboxScript(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack)
{
    if (L!=nullptr)
    {
        _executionState=execState_initialized; // initialized and no runtime error. Reset this constantly
        _runScriptOrCallScriptFunction(callType,inStack,outStack);
    }
    return(-1);
}

bool CLuaScriptObject::runSandboxScript_beforeMainScript()
{
    bool retVal=true;
    CInterfaceStack outStack;
    callSandboxScript(sim_syscb_beforemainscript,nullptr,&outStack);
    bool doNotRunMainScript;
    if (outStack.getStackMapBoolValue("doNotRunMainScript",doNotRunMainScript))
    {
        if (doNotRunMainScript)
            retVal=false;
    }
    return(retVal);
}

int CLuaScriptObject::runAddOn(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack)
{ // retval: -2: compil error, -1: runtimeError, 0: function not there or script not executed, 1: ok
    int retVal=0;
    if (_flaggedForDestruction)
        callType=sim_syscb_cleanup;
    if (L==nullptr)
    {
        if ( (callType==sim_syscb_init)||(_addOn_executionState==-1) ) // second arg. is for auto-run
        {
            if ( (_addOn_executionState!=-1)||(callType==sim_syscb_init)||(callType==sim_syscb_aos_run_old)||(callType==sim_syscb_nonsimulation)||(callType==sim_syscb_beforemainscript) )
                retVal=_runAddOn(sim_syscb_init,inStack,outStack);
        }
    }
    else
    { // ok, the add-on was already initialized. We can run it, suspend it, restart it, or kill it (and a few others):
        if ( (callType==sim_syscb_aos_run_old)||(callType==sim_syscb_nonsimulation)||(callType==sim_syscb_beforemainscript) )
        {
            if (_addOn_executionState!=sim_syscb_aos_suspend) // when suspended, we first need to unsuspend it
                retVal=_runAddOn(callType,inStack,outStack);
        }
        else if (callType==sim_syscb_aos_suspend)
        {
            if ( (_addOn_executionState==sim_syscb_aos_run_old)||(_addOn_executionState==sim_syscb_nonsimulation)||(_addOn_executionState==sim_syscb_beforemainscript) ) // only when running
                retVal=_runAddOn(callType,inStack,outStack);
        }
        else if (callType==sim_syscb_aos_resume)
        {
            if (_addOn_executionState==sim_syscb_aos_suspend) // only when suspended
                retVal=_runAddOn(callType,inStack,outStack);
        }
        else if (callType==sim_syscb_cleanup)
            killLuaState();
        else
        {
            if (_addOn_executionState!=sim_syscb_aos_suspend) // when suspended, we don't handle the other system calls
                retVal=_runAddOn(callType,inStack,outStack);
        }
    }
    return(retVal);
}

int CLuaScriptObject::_runAddOn(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack)
{
    CInterfaceStack outStackTmp;
    CInterfaceStack* outStackProxy;
    if (outStack!=nullptr)
        outStackProxy=outStack;
    else
        outStackProxy=&outStackTmp;

    int retVal;
    if (_compatibilityModeOrFirstTimeCall_sysCallbacks)
    { // for backward compatibility
        CInterfaceStack inStackLocal;
        inStackLocal.pushNumberOntoStack(double(callType));
        retVal=_runScriptOrCallScriptFunction(callType,&inStackLocal,outStackProxy);
    }
    else
        retVal=_runScriptOrCallScriptFunction(callType,inStack,outStackProxy);
    if (retVal>-2)
    {
        if ( (callType==sim_syscb_init)||(callType==sim_syscb_cleanup)||(callType==sim_syscb_aos_run_old)||(callType==sim_syscb_aos_suspend)||(callType==sim_syscb_aos_resume) )
            _addOn_executionState=callType;
    }
    if ( (outStackProxy->getStackSize()>0)&&(callType!=sim_syscb_cleanup) )
    {
        CInterfaceStack* outStackCopy=outStackProxy->copyYourself();
        outStackCopy->moveStackItemToTop(0);
        int r;
        if (outStackCopy->getStackIntValue(r))
        {
            if ( (r==sim_syscb_cleanup)||(r==sim_syscb_init) ) // second part is for backward compatibility
                killLuaState(); // we want to run the clean-up procedure
        }
        delete outStackCopy;
    }
    return(retVal);
}

VTHREAD_ID_TYPE CLuaScriptObject::getThreadedScriptThreadId() const
{
    return(_threadedScript_associatedFiberOrThreadID);
}

VTHREAD_RETURN_TYPE CLuaScriptObject::_startAddressForThreadedScripts(VTHREAD_ARGUMENT_TYPE lpData)
{
    TRACE_INTERNAL;
    _globalMutex.lock("CLuaScriptObject::_startAddressForThreadedScripts()");
    CLuaScriptObject* it=toBeCalledByThread[0];
    toBeCalledByThread.erase(toBeCalledByThread.begin());
    _globalMutex.unlock();

    it->_launchThreadedChildScriptNow();

    it->killLuaState(); // added on 2010/03/05 because of some crashes with threaded scripts (Lua said: resource destroyed by other thread than the one that created it!)
    it->_threadedExecutionUnderWay=false;

    return(VTHREAD_RETURN_VAL);
}

int CLuaScriptObject::callScriptFunction(const char* functionName,SLuaCallBack* pdata)
{ // DEPRECATED
    int retVal=-1; // means error

    if (!_prepareLuaStateAndCallScriptInitSectionIfNeeded())
        return(retVal);
    changeOverallYieldingForbidLevel(1,false);
    int oldTop=luaWrap_lua_gettop(L);   // We store lua's stack

    luaWrap_luaL_dostring(L,"sim_call_type=-1"); // for backward compatibility

    // Push the function name onto the stack (will be automatically popped from stack after _luaPCall):
    std::string func(functionName);
    size_t ppos=func.find('.');
    if (ppos==std::string::npos)
        luaWrap_lua_getglobal(L,func.c_str()); // in case we have a global function:
    else
    { // in case we have a function that is not global
        std::string globalVar(func.begin(),func.begin()+ppos);
        luaWrap_lua_getglobal(L,globalVar.c_str());
        func.assign(func.begin()+ppos+1,func.end());
        size_t ppos=func.find('.');
        while (ppos!=std::string::npos)
        {
            std::string var(func.begin(),func.begin()+ppos);
            luaWrap_lua_getfield(L,-1,var.c_str());
            luaWrap_lua_remove(L,-2);
            func.erase(func.begin(),func.begin()+ppos+1);
            ppos=func.find('.');
        }
        luaWrap_lua_getfield(L,-1,func.c_str());
        luaWrap_lua_remove(L,-2);
    }

    // Push the arguments onto the stack (will be automatically popped from stack after _luaPCall):
    int inputArgs=pdata->inputArgCount;
    if (inputArgs!=0)
    {
        int boolPt=0;
        int intPt=0;
        int floatPt=0;
        int doublePt=0;
        int stringPt=0;
        int stringBuffPt=0;
        for (int i=0;i<pdata->inputArgCount;i++)
            writeCustomFunctionDataOntoStack(L,pdata->inputArgTypeAndSize[2*i+0],pdata->inputArgTypeAndSize[2*i+1],pdata->inputBool,boolPt,pdata->inputInt,intPt,pdata->inputFloat,floatPt,pdata->inputDouble,doublePt,pdata->inputChar,stringPt,pdata->inputCharBuff,stringBuffPt);
    }

    luaWrap_lua_getglobal(L,"debug");
    luaWrap_lua_getfield(L,-1,"traceback");
    luaWrap_lua_remove(L,-2);
    int argCnt=inputArgs;
    int errindex=-argCnt-2;
    luaWrap_lua_insert(L,errindex);

    // Following line new since 7/3/2016:
    if (_luaPCall(L,argCnt,luaWrapGet_LUA_MULTRET(),errindex,functionName)!=0)
    { // a runtime error occurred!
        std::string errMsg;
        if (luaWrap_lua_isstring(L,-1))
            errMsg=std::string(luaWrap_lua_tostring(L,-1));
        else
            errMsg="(error unknown)";
        luaWrap_lua_pop(L,1); // pop error from stack
        _announceErrorWasRaisedAndDisableScript(errMsg.c_str(),true);

        // Following probably not needed:
        pdata->outputBool=new unsigned char[0];
        pdata->outputInt=new int[0];
        pdata->outputFloat=new float[0];
        pdata->outputDouble=new double[0];
        pdata->outputChar=new char[0];
        pdata->outputCharBuff=new char[0];
    }
    else
    { // return values:
        int currentTop=luaWrap_lua_gettop(L);

        // Following line new since 7/3/2016:
        int numberOfArgs=currentTop-oldTop-1; // the first arg is linked to the debug mechanism

        // We read all arguments from the stack
        std::vector<char> outBoolVector;
        std::vector<int> outIntVector;
        std::vector<float> outFloatVector;
        std::vector<double> outDoubleVector;
        std::vector<std::string> outStringVector;
        std::vector<std::string> outCharVector;
        std::vector<int> outInfoVector;
        for (int i=0;i<numberOfArgs;i++)
        {
            // Following line new since 7/3/2016:
            if (!readCustomFunctionDataFromStack(L,oldTop+i+1+1,pdata->outputArgTypeAndSize[i*2+0],outBoolVector,outIntVector,outFloatVector,outDoubleVector,outStringVector,outCharVector,outInfoVector))
                break;
        }

        // Now we prepare the output buffers:
        pdata->outputBool=new unsigned char[outBoolVector.size()];
        pdata->outputInt=new int[outIntVector.size()];
        pdata->outputFloat=new float[outFloatVector.size()];
        pdata->outputDouble=new double[outDoubleVector.size()];
        int charCnt=0;
        for (size_t k=0;k<outStringVector.size();k++)
            charCnt+=(int)outStringVector[k].length()+1; // terminal 0
        pdata->outputChar=new char[charCnt];

        int charBuffCnt=0;
        for (size_t k=0;k<outCharVector.size();k++)
            charBuffCnt+=(int)outCharVector[k].length();
        pdata->outputCharBuff=new char[charBuffCnt];

        pdata->outputArgCount=int(outInfoVector.size()/2);
        delete[] pdata->outputArgTypeAndSize;
        pdata->outputArgTypeAndSize=new int[outInfoVector.size()];
        // We fill the output buffers:
        for (int k=0;k<int(outBoolVector.size());k++)
            pdata->outputBool[k]=outBoolVector[k];
        for (int k=0;k<int(outIntVector.size());k++)
            pdata->outputInt[k]=outIntVector[k];
        for (int k=0;k<int(outFloatVector.size());k++)
            pdata->outputFloat[k]=outFloatVector[k];
        for (int k=0;k<int(outDoubleVector.size());k++)
            pdata->outputDouble[k]=outDoubleVector[k];
        charCnt=0;
        for (size_t k=0;k<outStringVector.size();k++)
        {
            for (size_t l=0;l<outStringVector[k].length();l++)
                pdata->outputChar[charCnt+l]=outStringVector[k][l];
            charCnt+=(int)outStringVector[k].length();
            // terminal 0:
            pdata->outputChar[charCnt]=0;
            charCnt++;
        }

        charBuffCnt=0;
        for (size_t k=0;k<outCharVector.size();k++)
        {
            for (size_t l=0;l<outCharVector[k].length();l++)
                pdata->outputCharBuff[charBuffCnt+l]=outCharVector[k][l];
            charBuffCnt+=(int)outCharVector[k].length();
        }

        for (int k=0;k<int(outInfoVector.size());k++)
            pdata->outputArgTypeAndSize[k]=outInfoVector[k];

        retVal=0;
    }
    luaWrap_lua_settop(L,oldTop);       // We restore lua's stack
    changeOverallYieldingForbidLevel(-1,false);
    return(retVal);
}

int CLuaScriptObject::callScriptFunctionEx(const char* functionName,CInterfaceStack* stack)
{ // retVal: -3: could not initialize script, -2: function does not exist, -1: error in function, 0:ok
    int retVal=-3;
    changeOverallYieldingForbidLevel(1,false);
    if ( (L!=nullptr) && (!_scriptIsDisabled) && (_executionState==execState_initialized) )
    {
        //if (!_prepareLuaStateAndCallScriptInitSectionIfNeeded())
        //    return(retVal);
        retVal=-2;

        int oldTop=luaWrap_lua_gettop(L);   // We store lua's stack

        luaWrap_luaL_dostring(L,"sim_call_type=-1"); // for backward compatibility

        // Push the function name onto the stack (will be automatically popped from stack after _luaPCall):
        std::string func(functionName);
        size_t ppos=func.find('.');
        bool notFunction=false;
        if (ppos==std::string::npos)
            luaWrap_lua_getglobal(L,func.c_str()); // in case we have a global function:
        else
        { // in case we have a function that is not global
            std::string globalVar(func.begin(),func.begin()+ppos);
            luaWrap_lua_getglobal(L,globalVar.c_str());
            if (luaWrap_lua_istable(L,-1))
            {
                func.assign(func.begin()+ppos+1,func.end());
                size_t ppos=func.find('.');
                while (ppos!=std::string::npos)
                {
                    std::string var(func.begin(),func.begin()+ppos);
                    luaWrap_lua_getfield(L,-1,var.c_str());
                    luaWrap_lua_remove(L,-2);
                    func.erase(func.begin(),func.begin()+ppos+1);
                    ppos=func.find('.');
                    if (!luaWrap_lua_istable(L,-1))
                    {
                        notFunction=true;
                        break;
                    }
                }
                if (!notFunction)
                {
                    luaWrap_lua_getfield(L,-1,func.c_str());
                    luaWrap_lua_remove(L,-2);
                }
            }
            else
                notFunction=true;
        }

        if ( (!notFunction)&&luaWrap_lua_isfunction(L,-1) )
        {
            retVal=-1;
            // Push the arguments onto the stack (will be automatically popped from stack after _luaPCall):
            int inputArgs=stack->getStackSize();

            if (inputArgs!=0)
                stack->buildOntoLuaStack(L,false);

            stack->clear();

            luaWrap_lua_getglobal(L,"debug");
            luaWrap_lua_getfield(L,-1,"traceback");
            luaWrap_lua_remove(L,-2);
            int argCnt=inputArgs;
            int errindex=-argCnt-2;
            luaWrap_lua_insert(L,errindex);

            if (_luaPCall(L,argCnt,luaWrapGet_LUA_MULTRET(),errindex,functionName)!=0)
            { // a runtime error occurred!
                std::string errMsg;
                if (luaWrap_lua_isstring(L,-1))
                    errMsg=std::string(luaWrap_lua_tostring(L,-1));
                else
                    errMsg="(error unknown)";
                luaWrap_lua_pop(L,1); // pop error from stack
                _announceErrorWasRaisedAndDisableScript(errMsg.c_str(),true);
            }
            else
            { // return values:
                int currentTop=luaWrap_lua_gettop(L);

                int numberOfArgs=currentTop-oldTop-1; // the first arg is linked to the debug mechanism
                stack->buildFromLuaStack(L,oldTop+1+1,numberOfArgs); // the first arg is linked to the debug mechanism
                retVal=0;
            }
        }

        luaWrap_lua_settop(L,oldTop);       // We restore lua's stack
    }
    changeOverallYieldingForbidLevel(-1,false);
    return(retVal);
}

int CLuaScriptObject::setScriptVariable(const char* variableName,CInterfaceStack* stack)
{
    int retVal=-1;
    if (!_prepareLuaStateAndCallScriptInitSectionIfNeeded())
        return(retVal);
    int oldTop=luaWrap_lua_gettop(L);   // We store lua's stack


    std::string var(variableName);
    size_t ppos=var.find('.');
    if (ppos==std::string::npos)
    { // in case we have a global variable
        if (stack!=nullptr)
            stack->buildOntoLuaStack(L,true);
        else
            luaWrap_lua_pushnil(L);
        luaWrap_lua_setglobal(L,variableName);
        retVal=0;
    }
    else
    { // in case we have a variable that is not global
        std::string globalVar(var.begin(),var.begin()+ppos);
        luaWrap_lua_getglobal(L,globalVar.c_str());
        if (luaWrap_lua_istable(L,-1))
        {
            var.assign(var.begin()+ppos+1,var.end());
            size_t ppos=var.find('.');
            bool badVar=false;
            while (ppos!=std::string::npos)
            {
                std::string vvar(var.begin(),var.begin()+ppos);
                luaWrap_lua_getfield(L,-1,vvar.c_str());
                luaWrap_lua_remove(L,-2);
                var.erase(var.begin(),var.begin()+ppos+1);
                ppos=var.find('.');
                if (!luaWrap_lua_istable(L,-1))
                {
                    badVar=true;
                    break;
                }
            }
            if (!badVar)
            {
                if (stack!=nullptr)
                    stack->buildOntoLuaStack(L,true);
                else
                    luaWrap_lua_pushnil(L);
                luaWrap_lua_setfield(L,-2,var.c_str());
                retVal=0;
            }
        }
    }

    luaWrap_lua_settop(L,oldTop);       // We restore lua's stack
    return(retVal);
}

int CLuaScriptObject::clearScriptVariable(const char* variableName)
{ // deprecated
    if (L==nullptr)
        return(-1);

    if (_threadedExecution)
    {
        if (!VThread::areThreadIDsSame(_threadedScript_associatedFiberOrThreadID,VThread::getCurrentThreadId()))
            return(-1); // only from the same thread when threaded!!
    }

    if (_scriptIsDisabled)
        return(-1);


    int oldTop=luaWrap_lua_gettop(L);   // We store lua's stack

    std::string var(variableName);
    size_t ppos=var.find('.');
    if (ppos==std::string::npos)
    { // in case we have a global variable
        luaWrap_lua_pushnil(L);
        luaWrap_lua_setglobal(L,variableName);
    }
    else
    { // in case we have a variable that is not global
        std::string globalVar(var.begin(),var.begin()+ppos);
        luaWrap_lua_getglobal(L,globalVar.c_str());
        var.assign(var.begin()+ppos+1,var.end());
        size_t ppos=var.find('.');
        while (ppos!=std::string::npos)
        {
            std::string vvar(var.begin(),var.begin()+ppos);
            luaWrap_lua_getfield(L,-1,vvar.c_str());
            luaWrap_lua_remove(L,-2);
            var.erase(var.begin(),var.begin()+ppos+1);
            ppos=var.find('.');
        }
        luaWrap_lua_pushnil(L);
        luaWrap_lua_setfield(L,-2,var.c_str());
    }

    luaWrap_lua_settop(L,oldTop);       // We restore lua's stack
    return(0);
}

int CLuaScriptObject::executeScriptString(const char* scriptString,CInterfaceStack* stack)
{ // retVal: -2: script not initialized, is disabled, or had previously an error, -1: string caused an error, 0: string didn't cause an error
    int retVal=-2;
    changeOverallYieldingForbidLevel(1,false);
    if ( (L!=nullptr) && (!_scriptIsDisabled) && (_executionState==execState_initialized) )
    {
        retVal=-1;
        int oldTop=luaWrap_lua_gettop(L);   // We store lua's stack

        bool firstCall=(_timeOfPcallStart==-1);
        if (firstCall)
            _timeOfPcallStart=VDateTime::getTimeInMs();


        std::string theString("return ");
        theString+=scriptString;
        int loadBufferRes=luaWrap_luaL_loadbuffer(L,theString.c_str(),theString.size(),scriptString);
        if (loadBufferRes!=0)
        {
            luaWrap_lua_settop(L,oldTop);       // We restore lua's stack
            loadBufferRes=luaWrap_luaL_loadbuffer(L,scriptString,strlen(scriptString),scriptString);
        }
        if (loadBufferRes==0)
        {
            int intTop=luaWrap_lua_gettop(L);
            if (luaWrap_lua_pcall(L,0,luaWrapGet_LUA_MULTRET(),0))
            { // a runtime error occurred!
                std::string errMsg;
                if (luaWrap_lua_isstring(L,-1))
                    errMsg=std::string(luaWrap_lua_tostring(L,-1));
                else
                    errMsg="(error unknown)";
                if (stack!=nullptr)
                {
                    stack->clear();
                    stack->pushStringOntoStack(errMsg.c_str(),0);
                }
            }
            else
            {
                int currentTop=luaWrap_lua_gettop(L);
                int numberOfArgs=currentTop-oldTop;
                if (stack!=nullptr)
                    stack->buildFromLuaStack(L,currentTop-numberOfArgs+1,numberOfArgs);
                retVal=0;
            }
        }
        else
        { // A compilation error occurred!
            std::string errMsg;
            if (luaWrap_lua_isstring(L,-1))
                errMsg=std::string(luaWrap_lua_tostring(L,-1));
            else
                errMsg="(error unknown)";
            if (stack!=nullptr)
            {
                stack->clear();
                stack->pushStringOntoStack(errMsg.c_str(),0);
            }
        }
/*
        std::string theString("return ");
        theString+=scriptString;
        if (0!=luaWrap_luaL_dostring(L,theString.c_str()))
        { // 'return theStringToExecute' failed to execute. Let's simply execute 'theStringToExecute'
            int intermediateTop=luaWrap_lua_gettop(L);
            if (0==luaWrap_luaL_dostring(L,scriptString))
                retVal=0; // ok, that worked without an error

            // Now build the return value or error onto the stack
            if (stack!=nullptr)
            {
                stack->clear();
                if (luaWrap_lua_gettop(L)>intermediateTop)
                    stack->buildFromLuaStack(L,intermediateTop+1,luaWrap_lua_gettop(L)-intermediateTop);
            }
        }
        else
        { // ok, let's see what we have in return:
            if (stack!=nullptr)
            {
                stack->clear();
                if (luaWrap_lua_gettop(L)>oldTop)
                    stack->buildFromLuaStack(L,oldTop+1,luaWrap_lua_gettop(L)-oldTop);
            }
            retVal=0;
        }
        //*/

        if (firstCall)
            _timeOfPcallStart=-1;
        luaWrap_lua_settop(L,oldTop);       // We restore lua's stack
    }
    changeOverallYieldingForbidLevel(-1,false);
    return(retVal);
}

bool CLuaScriptObject::hasLuaState() const
{
    return(L!=nullptr);
}

bool CLuaScriptObject::isSimulationScript() const
{
    return( (_scriptType==sim_scripttype_mainscript)||(_scriptType==sim_scripttype_childscript) );
}

bool CLuaScriptObject::isSceneSwitchPersistentScript() const
{
    return( (_scriptType==sim_scripttype_sandboxscript)||(_scriptType==sim_scripttype_addonscript) );
}

bool CLuaScriptObject::killLuaState()
{
    bool retVal=(L!=nullptr);
    if (L!=nullptr)
    {
        _executionState=execState_initialized; // in case we had a runtime error
        if (_scriptType==sim_scripttype_customizationscript)
        {
            if (hasCustomizationScripAnyChanceToGetExecuted(true,false)) // run the last pass even if we do not wish to execute the script during simulation
                _runScriptOrCallScriptFunction(sim_syscb_cleanup,nullptr,nullptr);
        }

        if (_scriptType==sim_scripttype_mainscript)
        {
            // Not needed for the main script (was already handled elsewhere!)
        }

        if (_scriptType==sim_scripttype_addonfunction)
        {
            // Not needed
        }

        if (_scriptType==sim_scripttype_sandboxscript)
        {
            // Not needed
        }
        if (_scriptType==sim_scripttype_addonscript)
        {
            _runAddOn(sim_syscb_cleanup,nullptr,nullptr);
        }
        if ((_scriptType==sim_scripttype_childscript)&&(!_threadedExecution) )
        {
            // Following few not elegant, but important otherwise we don't have the emergency stop button
            bool wasInMainScript=App::currentWorld->embeddedScriptContainer->getInMainScriptNow();
            if (!wasInMainScript)
                App::currentWorld->embeddedScriptContainer->setInMainScriptNow(true,VDateTime::getTimeInMs());
            _runNonThreadedChildScriptNow(sim_syscb_cleanup,nullptr,nullptr);
            if (!wasInMainScript)
                App::currentWorld->embeddedScriptContainer->setInMainScriptNow(false,0);
        }
        App::worldContainer->announceScriptStateWillBeErased(_scriptHandle,isSimulationScript(),isSceneSwitchPersistentScript());
        luaWrap_lua_close(L);
        L=nullptr;
    }
    _executionState=execState_uninitialized;
    _loadBufferResult=-1;
    _scriptTextExec.clear();
    if (!_threadedExecution)
        _numberOfPasses=0;
    _addOn_executionState=sim_syscb_init;
    _compatibilityModeOrFirstTimeCall_sysCallbacks=true;
    _containsJointCallbackFunction=false;
    _containsContactCallbackFunction=false;
    _containsDynCallbackFunction=false;
    _containsVisionCallbackFunction=false;
    _containsTriggerCallbackFunction=false;
    _containsUserConfigCallbackFunction=false;
    _flaggedForDestruction=false;
    return(retVal);
}

std::string CLuaScriptObject::getLuaSearchPath() const
{
    return(getAdditionalLuaSearchPath());
}

CLuaScriptObject* CLuaScriptObject::copyYourself()
{
    CLuaScriptObject* it=new CLuaScriptObject(_scriptType);
    it->_scriptHandle=_scriptHandle;
    it->_objectHandleAttachedTo=_objectHandleAttachedTo;
    it->_threadedExecution=_threadedExecution;
    it->_scriptIsDisabled=_scriptIsDisabled;
    it->_executionOrder=_executionOrder;
    it->_debugLevel=_debugLevel;
    it->_treeTraversalDirection=_treeTraversalDirection;
    it->setScriptText(getScriptText());

//    delete it->_scriptParameters_backCompatibility;
//    it->_scriptParameters_backCompatibility=_scriptParameters_backCompatibility->copyYourself();

    it->_executeJustOnce=_executeJustOnce;

    delete it->_customObjectData;
    it->_customObjectData=nullptr;
    if (_customObjectData!=nullptr)
        it->_customObjectData=_customObjectData->copyYourself();

    delete it->_customObjectData_tempData;
    it->_customObjectData_tempData=nullptr;
    if (_customObjectData_tempData!=nullptr)
        it->_customObjectData_tempData=_customObjectData_tempData->copyYourself();

    it->_initialValuesInitialized=_initialValuesInitialized;
    return(it);
}

CUserParameters* CLuaScriptObject::getScriptParametersObject_backCompatibility()
{
    return(_scriptParameters_backCompatibility);
}

bool CLuaScriptObject::addCommandToOutsideCommandQueue(int commandID,int auxVal1,int auxVal2,int auxVal3,int auxVal4,const float aux2Vals[8],int aux2Count)
{
    if (_outsideCommandQueue!=nullptr)
        return(_outsideCommandQueue->addCommand(commandID,auxVal1,auxVal2,auxVal3,auxVal4,aux2Vals,aux2Count));
    return(true);
}

int CLuaScriptObject::extractCommandFromOutsideCommandQueue(int auxVals[4],float aux2Vals[8],int& aux2Count)
{
    if (_outsideCommandQueue!=nullptr)
        return(_outsideCommandQueue->extractOneCommand(auxVals,aux2Vals,aux2Count));
    return(-1);
}

bool CLuaScriptObject::_prepareLuaStateAndCallScriptInitSectionIfNeeded()
{
    if ( (L==nullptr)&&_threadedExecution )
        return(false); // threaded scripts must be called from within, or already initialized!

    if (_threadedExecution)
    {
        if (!VThread::areThreadIDsSame(_threadedScript_associatedFiberOrThreadID,VThread::getCurrentThreadId()))
            return(false); // only from the same thread when threaded!!
    }

    if (_scriptIsDisabled)
        return(false);

    if (L!=nullptr)
        return(true);

    // this script was never run before!
    if (_scriptType==sim_scripttype_sandboxscript)
    {
        // not needed
    }
    if (_scriptType==sim_scripttype_customizationscript)
        runCustomizationScript(sim_syscb_init,nullptr,nullptr); // dummy call
    if (!App::currentWorld->simulation->isSimulationStopped())
    { // following only when simulation is running!
        if (_scriptType==sim_scripttype_childscript)
            runNonThreadedChildScript(sim_syscb_init,nullptr,nullptr); // dummy call
    }
    return(L!=nullptr);
}

bool CLuaScriptObject::_checkIfMixingOldAndNewCallMethods()
{
    return ( (_scriptText.find("sim_call_type")!=std::string::npos)&&(_scriptText.find("sysCall_")!=std::string::npos) );
}

void CLuaScriptObject::terminateScriptExecutionExternally(bool generateErrorMsg)
{
    if (generateErrorMsg)
    {
        std::string tmp("?: script execution was terminated externally.");
        _announceErrorWasRaisedAndDisableScript(tmp.c_str(),true);
    }
    if (getScriptType()==sim_scripttype_addonscript)
        flagForDestruction(); // stop it
    if (getDebugLevel()!=sim_scriptdebug_none)
        handleDebug("force_script_stop","C",true,true);
    luaWrap_lua_yield(L,0);
}

void CLuaScriptObject::_announceErrorWasRaisedAndDisableScript(const char* errMsg,bool runtimeError,bool debugRoutine/*=false*/)
{ // errMsg is in the form: xxxx:lineNb: msg
    std::string errM(errMsg);
    if ( (errM.find("attempt to yield across metamethod/C-call boundary")==std::string::npos)&&(errM.find("attempt to yield from outside a coroutine")==std::string::npos) )
    { // silent error when breaking out of a threaded child script at simulation end
        int verb=sim_verbosity_scripterrors;
        if (runtimeError)
            _executionState=execState_runtimeError;
        else
            _executionState=execState_compilationError;
        if ( (_scriptType==sim_scripttype_mainscript)||(_scriptType==sim_scripttype_childscript)||(_scriptType==sim_scripttype_customizationscript) )
            App::currentWorld->simulation->pauseOnErrorRequested();

        std::string name(getShortDescriptiveName());
        std::string msg(errM);
        if (debugRoutine)
        {
            size_t p=msg.find(": ");
            if (p!=std::string::npos)
                msg.insert(p+2,"[in debug routine] ");
        }

        App::logScriptMsg(name.c_str(),verb,msg.c_str());

        _lastStackTraceback=errM;
    }
}


void CLuaScriptObject::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Si2");
            ar << _scriptHandle << _objectHandleAttachedTo << _scriptType;
            ar.flush();

            // Keep following close to the beginning!
            ar.storeDataName("Va2");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_threadedExecution);
            SIM_SET_CLEAR_BIT(nothing,1,_scriptIsDisabled);
            // RESERVED
            SIM_SET_CLEAR_BIT(nothing,3,true); // used to be (!defaultMainScript). 16.11.2020
            SIM_SET_CLEAR_BIT(nothing,4,_executeJustOnce);
            // RESERVED!!
            SIM_SET_CLEAR_BIT(nothing,6,true); // this indicates we have the 'almost' new script execution engine (since V3.1.3)
            SIM_SET_CLEAR_BIT(nothing,7,true); // this indicates we have the new script execution engine (since V3.1.3)
            ar << nothing;
            ar.flush();

            // Keep following close to the beginning!
            ar.storeDataName("Va3"); // this is also used as a marked for the color correction introduced in V3.1.4 and later!
            nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,true); // needed for a code correction
            // reserved, was: SIM_SET_CLEAR_BIT(nothing,1,!_disableCustomizationScriptWithError);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Seo");
            ar << _executionOrder;
            ar.flush();

            ar.storeDataName("Ttd");
            ar << _treeTraversalDirection;
            ar.flush();

            ar.storeDataName("Dbl");
            ar << _debugLevel;
            ar.flush();

            std::string stt(_scriptText);

            // We store scripts in a light encoded way:
            ar.storeDataName("Ste");
            tt::lightEncodeBuffer(&stt[0],int(stt.length()));
            for (size_t i=0;i<stt.length();i++)
                ar << stt[i];
            ar.flush();

//            ar.storeDataName("Prm");
//            ar.setCountingMode();
//            _scriptParameters_backCompatibility->serialize(ar);
//            if (ar.setWritingMode())
//                _scriptParameters_backCompatibility->serialize(ar);

            // keep a while so that older versions can read this. 11.06.2019, V3.6.1 is current
            ar.storeDataName("Coc");
            ar << _objectHandleAttachedTo;
            ar.flush();

            if (_customObjectData!=nullptr)
            {
                ar.storeDataName("Cod");
                ar.setCountingMode();
                _customObjectData->serializeData(ar,nullptr,-1);
                if (ar.setWritingMode())
                    _customObjectData->serializeData(ar,nullptr,-1);
            }

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            bool backwardCompatibility_7_8_2014=false;
            bool backwardCompatibility_13_8_2014=false;
            bool executeInSensingPhase_oldCompatibility_7_8_2014=false;
            bool backwardCompatibilityCorrectionNeeded_13_10_2014=true;
            bool backwardCompatibilityCorrectionNeeded_8_11_2014=true;
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    bool justLoadedCustomScriptBuffer=false;
                    if (theName.compare("Si2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _scriptHandle >> _objectHandleAttachedTo >> _scriptType;
                    }

                    if (theName.compare("Ttd")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _treeTraversalDirection;
                    }
                    if (theName.compare("Dbl")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _debugLevel;
                    }

                    if (theName.compare("Seo")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _executionOrder;
                    }

                    if (theName.compare("Ste")==0)
                    { // The script is stored encoded!
                        noHit=false;
                        ar >> byteQuantity;

                        _scriptText.resize(byteQuantity,' ');
                        if (byteQuantity!=0)
                        {
                            for (int i=0;i<byteQuantity;i++)
                                ar >> _scriptText[i];
                            tt::lightDecodeBuffer(&_scriptText[0],byteQuantity);
                            while ( (_scriptText.size()>0)&&(_scriptText[_scriptText.size()-1]==0) )
                                _scriptText.erase(_scriptText.end()-1); // to fix a compatibility bug
                        }
                        justLoadedCustomScriptBuffer=true;
                    }
                    if (justLoadedCustomScriptBuffer&&(_scriptType==sim_scripttype_mainscript)&&_mainScriptIsDefaultMainScript_old) // old, keep for backward compatibility. 16.11.2020
                        _scriptText=DEFAULT_MAINSCRIPT_CODE;
                    if (theName=="Va2")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _threadedExecution=SIM_IS_BIT_SET(nothing,0);
                        _scriptIsDisabled=SIM_IS_BIT_SET(nothing,1);
                        _mainScriptIsDefaultMainScript_old=!SIM_IS_BIT_SET(nothing,3);
                        _executeJustOnce=SIM_IS_BIT_SET(nothing,4);
                        executeInSensingPhase_oldCompatibility_7_8_2014=SIM_IS_BIT_SET(nothing,5);
                        backwardCompatibility_7_8_2014=!SIM_IS_BIT_SET(nothing,6);
                        backwardCompatibility_13_8_2014=!SIM_IS_BIT_SET(nothing,7);
                        if (_threadedExecution)
                        {
                            if (App::userSettings->xrTest==123456789)
                                App::logMsg(sim_verbosity_errors,"Contains a threaded script...");
                        }
                    }
                    if (theName=="Va3")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        backwardCompatibilityCorrectionNeeded_8_11_2014=!SIM_IS_BIT_SET(nothing,0);
                        backwardCompatibilityCorrectionNeeded_13_10_2014=false;
                    }
                    if (theName.compare("Prm")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        delete _scriptParameters_backCompatibility;
                        _scriptParameters_backCompatibility=new CUserParameters();
                        _scriptParameters_backCompatibility->serialize(ar);
                    }

                    if (theName.compare("Coc")==0)
                    { // keep 3-4 years for backward compatibility (11.06.2019, V3.6.1 is current)
                        noHit=false;
                        ar >> byteQuantity;
                        int v;
                        ar >> v;
                        if (v>=0)
                            _objectHandleAttachedTo=v;
                    }
                    if (theName.compare("Cod")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        _customObjectData=new CCustomData();
                        _customObjectData->serializeData(ar,nullptr,-1);
                    }

                    if (noHit)
                        ar.loadUnknownData();
                }
            }

            _adjustScriptText1(this,backwardCompatibility_7_8_2014,executeInSensingPhase_oldCompatibility_7_8_2014);
            _adjustScriptText2(this,(!backwardCompatibility_7_8_2014)&&backwardCompatibility_13_8_2014);
            _adjustScriptText3(this,backwardCompatibilityCorrectionNeeded_13_10_2014);
            _adjustScriptText4(this,backwardCompatibilityCorrectionNeeded_8_11_2014);
            _adjustScriptText5(this,ar.getCoppeliaSimVersionThatWroteThisFile()<30300);
            _adjustScriptText6(this,ar.getCoppeliaSimVersionThatWroteThisFile()<30300);
            _adjustScriptText7(this,(ar.getCoppeliaSimVersionThatWroteThisFile()<=30302)&&(SIM_PROGRAM_VERSION_NB>30302));
            _adjustScriptText8(this,App::userSettings->changeScriptCodeForNewApiNotation);
            _adjustScriptText9(this);
            _adjustScriptText10(this,ar.getCoppeliaSimVersionThatWroteThisFile()<30401);
            _adjustScriptText11(this,ar.getCoppeliaSimVersionThatWroteThisFile()<40001);
            _adjustScriptText12(this,ar.getCoppeliaSimVersionThatWroteThisFile()<40100);
            _adjustScriptText13(this,ar.getCoppeliaSimVersionThatWroteThisFile()<40200);
            if ( _threadedExecution&&(App::userSettings->xrTest==123456789) ) //(!App::userSettings->makeOldThreadedScriptsAvailable) )
            {
                if (_convertThreadedScriptToCoroutine(this))
                {
                    _threadedExecution=false;
                    _executeJustOnce=false;
                }
            }
            fromBufferToFile();
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            if (exhaustiveXml)
            {
                ar.xmlAddNode_int("handle",_scriptHandle);
                ar.xmlAddNode_int("objectHandle",_objectHandleAttachedTo);

                ar.xmlAddNode_enum("type",_scriptType,sim_scripttype_mainscript,"mainScript",sim_scripttype_childscript,"childScript",sim_scripttype_customizationscript,"customizationScript");
            }

            ar.xmlPushNewNode("switches");
            if ( exhaustiveXml||(_scriptType==sim_scripttype_childscript) )
                ar.xmlAddNode_bool("threadedExecution",_threadedExecution);
            ar.xmlAddNode_bool("enabled",!_scriptIsDisabled);
            if ( exhaustiveXml||(_scriptType==sim_scripttype_childscript) )
                ar.xmlAddNode_bool("executeOnce",_executeJustOnce);
            ar.xmlPopNode();

            ar.xmlAddNode_int("executionOrder",_executionOrder);
            ar.xmlAddNode_int("treeTraversalDirection",_treeTraversalDirection);
            if (exhaustiveXml)
                ar.xmlAddNode_int("debugLevel",_debugLevel);

            std::string tmp(_scriptText.c_str());
            boost::replace_all(tmp,"\r\n","\n");
            ar.xmlAddNode_comment(" 'scriptText' tag: best to use it with a CDATA section for readability ",exhaustiveXml);
            ar.xmlAddNode_cdata("scriptText",tmp.c_str());

            if (exhaustiveXml)
            {
                if (_customObjectData!=nullptr)
                {
                    ar.xmlPushNewNode("customData");
                    _customObjectData->serializeData(ar,nullptr,_scriptHandle);
                    ar.xmlPopNode();
                }
            }
        }
        else
        {
            if (exhaustiveXml)
            {
                ar.xmlGetNode_int("handle",_scriptHandle);
                ar.xmlGetNode_int("objectHandle",_objectHandleAttachedTo);

                ar.xmlGetNode_enum("type",_scriptType,true,"mainScript",sim_scripttype_mainscript,"childScript",sim_scripttype_childscript,"customizationScript",sim_scripttype_customizationscript);
            }

            if (ar.xmlPushChildNode("switches",exhaustiveXml))
            {
                if ( exhaustiveXml||(_scriptType==sim_scripttype_childscript) )
                    ar.xmlGetNode_bool("threadedExecution",_threadedExecution,exhaustiveXml);
                if (ar.xmlGetNode_bool("enabled",_scriptIsDisabled,exhaustiveXml))
                    _scriptIsDisabled=!_scriptIsDisabled;
                if (exhaustiveXml)
                    ar.xmlGetNode_bool("isDefaultMainScript",_mainScriptIsDefaultMainScript_old);
                if ( exhaustiveXml||(_scriptType==sim_scripttype_childscript) )
                    ar.xmlGetNode_bool("executeOnce",_executeJustOnce,exhaustiveXml);
                ar.xmlPopNode();
            }

            ar.xmlGetNode_int("executionOrder",_executionOrder,exhaustiveXml);
            ar.xmlGetNode_int("treeTraversalDirection",_treeTraversalDirection,exhaustiveXml);
            if (exhaustiveXml)
                ar.xmlGetNode_int("debugLevel",_debugLevel);

            if (ar.xmlGetNode_cdata("scriptText",_scriptText,exhaustiveXml)&&(_scriptType==sim_scripttype_mainscript)&&_mainScriptIsDefaultMainScript_old) // for backward compatibility 16.11.2020
                _scriptText=DEFAULT_MAINSCRIPT_CODE;

            if (exhaustiveXml)
            {
                if (ar.xmlPushChildNode("customData",false))
                {
                    _customObjectData=new CCustomData();
                    _customObjectData->serializeData(ar,nullptr,-1);
                    ar.xmlPopNode();
                }
            }
            fromBufferToFile();
        }
    }
}


int CLuaScriptObject::appendTableEntry(const char* arrayName,const char* keyName,const char* data,const int what[2])
{ // DEPRECATED since 23/2/2016
    if ( (L==nullptr)&&_threadedExecution )
        return(-1); // threaded scripts must be called from within!

    if (_threadedExecution)
    {
        if (!VThread::areThreadIDsSame(_threadedScript_associatedFiberOrThreadID,VThread::getCurrentThreadId()))
            return(-1); // only from the same thread when threaded!!
    }

    if (_scriptIsDisabled)
        return(-1);

    if (L==nullptr)
    {  // this script was never run before!
        if (_scriptType==sim_scripttype_customizationscript)
            runCustomizationScript(sim_syscb_init,nullptr,nullptr); // dummy call
        if (!App::currentWorld->simulation->isSimulationStopped())
        { // following only when simulation is running!
            if (_scriptType==sim_scripttype_childscript)
                runNonThreadedChildScript(sim_syscb_init,nullptr,nullptr); // dummy call
        }
    }

    if (L==nullptr)
        return(-1);

    int oldTop=luaWrap_lua_gettop(L);   // We store lua's stack

    // First check if the table where we want to append a value exists. If not, or not a table, create it!
    luaWrap_lua_getglobal(L,arrayName);
    if (!luaWrap_lua_istable(L,-1))
    { // the table is inexistant
        luaWrap_lua_newtable(L);
        luaWrap_lua_setglobal(L,arrayName);
    }
    luaWrap_lua_pop(L,1);

    // The table where we want to append a value:
    luaWrap_lua_getglobal(L,arrayName);
    int theTablePos=luaWrap_lua_gettop(L);
    int theTableLength=int(luaWrap_lua_rawlen(L,theTablePos));

    // Do we want to simply insert the value, or do we want to insert a keyed value?
    if ( (keyName==nullptr)||(strlen(keyName)==0) )
    { // not keyed value:
    }
    else
    { // keyed value:
        luaWrap_lua_pushstring(L,keyName);
    }

    // Now push the value, which might itself be a table:
    int w=what[0];
    if ( (w&sim_script_arg_table)!=0 )
    { // we have a table
        w-=sim_script_arg_table;
        luaWrap_lua_newtable(L);
        int newTablePos=luaWrap_lua_gettop(L);
        int stringOff=0;
        for (int i=0;i<what[1];i++)
        {
            if (w==sim_script_arg_null)
                luaWrap_lua_pushnil(L);
            if (w==sim_script_arg_bool)
                luaWrap_lua_pushboolean(L,data[i]);
            if (w==sim_script_arg_int32)
                luaWrap_lua_pushinteger(L,((int*)data)[i]);
            if (w==sim_script_arg_float)
                luaWrap_lua_pushnumber(L,((float*)data)[i]);
            if (w==sim_script_arg_double)
                luaWrap_lua_pushnumber(L,((double*)data)[i]);
            if (w==sim_script_arg_string)
            {
                luaWrap_lua_pushstring(L,data+stringOff);
                stringOff+=int(strlen(data+stringOff))+1;
            }
            luaWrap_lua_rawseti(L,newTablePos,i+1);
        }
    }
    else
    { // we don't have a table
        if (w==sim_script_arg_null)
            luaWrap_lua_pushnil(L);
        if (w==sim_script_arg_bool)
            luaWrap_lua_pushboolean(L,data[0]);
        if (w==sim_script_arg_int32)
            luaWrap_lua_pushinteger(L,((int*)data)[0]);
        if (w==sim_script_arg_float)
            luaWrap_lua_pushnumber(L,((float*)data)[0]);
        if (w==sim_script_arg_double)
            luaWrap_lua_pushnumber(L,((double*)data)[0]);
        if (w==sim_script_arg_string)
            luaWrap_lua_pushstring(L,data);
        if (w==sim_script_arg_charbuff)
            luaWrap_lua_pushlstring(L,data,what[1]);
    }

    // Finally, insert the value in the table:
    if ( (keyName==nullptr)||(strlen(keyName)==0) )
    { // not keyed value:
        luaWrap_lua_rawseti(L,theTablePos,theTableLength+1);
    }
    else
    { // keyed value:
        luaWrap_lua_settable(L,-3);
    }

    luaWrap_lua_settop(L,oldTop);       // We restore lua's stack
    return(0);
}

void CLuaScriptObject::handleDebug(const char* funcName,const char* funcType,bool inCall,bool sysCall)
{
    if ( (_debugLevel!=sim_scriptdebug_none) && (!_inDebug) && (funcName!=nullptr) )
    {
        _inDebug=true;
        // Temp. disable the hook:
        luaWrap_lua_sethook(L,luaHookFunction,0,0);

        luaWrap_lua_getglobal(L,"_S");
        if (luaWrap_lua_istable(L,-1))
        {
            luaWrap_lua_getfield(L,-1,"debug");
            luaWrap_lua_remove(L,-2);
            if (luaWrap_lua_istable(L,-1))
            {
                luaWrap_lua_getfield(L,-1,"entryFunc");
                luaWrap_lua_remove(L,-2);
                if (luaWrap_lua_isfunction(L,-1))
                { // function name will be automatically popped after luaWrap_lua_pcall
                    luaWrap_lua_newtable(L);
                    luaWrap_lua_pushstring(L,getShortDescriptiveName().c_str());
                    luaWrap_lua_rawseti(L,-2,1);
                    luaWrap_lua_pushstring(L,funcName);
                    luaWrap_lua_rawseti(L,-2,2);
                    luaWrap_lua_pushstring(L,funcType);
                    luaWrap_lua_rawseti(L,-2,3);
                    luaWrap_lua_pushboolean(L,inCall);
                    luaWrap_lua_rawseti(L,-2,4);
                    luaWrap_lua_pushinteger(L,_debugLevel);
                    luaWrap_lua_rawseti(L,-2,5);
                    luaWrap_lua_pushboolean(L,sysCall);
                    luaWrap_lua_rawseti(L,-2,6);
                    luaWrap_lua_pushnumber(L,double(App::currentWorld->simulation->getSimulationTime_us())/1000000.0f);
                    luaWrap_lua_rawseti(L,-2,7);

                    if (luaWrap_lua_pcall(L,1,0,0)!=0)
                    { // an error occured in the lua debug code
                        std::string errMsg;
                        if (luaWrap_lua_isstring(L,-1))
                            errMsg+=std::string(luaWrap_lua_tostring(L,-1));
                        else
                            errMsg+="error unknown";
                        luaWrap_lua_pop(L,1); // pop error from stack
                        _announceErrorWasRaisedAndDisableScript(errMsg.c_str(),true,true);
                    }
                }
                else
                    luaWrap_lua_pop(L,1); // pop function name from stack
            }
            else
                luaWrap_lua_pop(L,1);
        }
        else
            luaWrap_lua_pop(L,1);

        // Re-enable the hook:
        int randComponent=rand()/(RAND_MAX/10);
        int hookMask=luaWrapGet_LUA_MASKCOUNT();
        if (_debugLevel>=sim_scriptdebug_allcalls)
            hookMask|=luaWrapGet_LUA_MASKCALL()|luaWrapGet_LUA_MASKRET();
        luaWrap_lua_sethook(L,luaHookFunction,hookMask,95+randComponent);

        _inDebug=false;
    }
}

int CLuaScriptObject::_luaPCall(luaWrap_lua_State* luaState,int nargs,int nresult,int errfunc,const char* funcName)
{ // do not forget, routine is often re-entered!
    bool firstCall=(_timeOfPcallStart==-1);
    if (firstCall)
        _timeOfPcallStart=VDateTime::getTimeInMs();
    _inExecutionNow=true;
    handleDebug(funcName,"Lua",true,true);
    int retVal=luaWrap_lua_pcall(luaState,nargs,nresult,errfunc);
    handleDebug(funcName,"Lua",false,true);
    _inExecutionNow=false;
    if (firstCall)
        _timeOfPcallStart=-1;
    return(retVal);
}

bool CLuaScriptObject::_luaLoadBuffer(luaWrap_lua_State* luaState,const char* buff,size_t sz,const char* name)
{
    // This is the slow version (loading and compiling the buffer over and over):
    // int loadBufferRes=luaWrap_luaL_loadbuffer(luaState,buff,sz,name);
    // return(loadBufferRes==0);

    // This is the faster version (loading and compiling the buffer once only):
    if (_loadBufferResult!=0)
    {
        _loadBufferResult=luaWrap_luaL_loadbuffer(luaState,buff,sz,name);
        if (_loadBufferResult==0)
            luaWrap_lua_setglobal(L,"sim_code_function_to_run");
    }
    if (_loadBufferResult==0)
        luaWrap_lua_getglobal(L,"sim_code_function_to_run");
    return(_loadBufferResult==0);
}


void CLuaScriptObject::setAutomaticCascadingCallsDisabled_OLD(bool disabled)
{
    _automaticCascadingCallsDisabled_OLD=disabled;
}

bool CLuaScriptObject::getAutomaticCascadingCallsDisabled_OLD() const
{
    return(_automaticCascadingCallsDisabled_OLD);
}

bool CLuaScriptObject::checkAndSetWarningAboutSimHandleChildScriptAlreadyIssued_oldCompatibility_7_8_2014()
{
    bool retVal=_warningAboutSimHandleChildScriptAlreadyIssued_oldCompatibility_7_8_2014;
    _warningAboutSimHandleChildScriptAlreadyIssued_oldCompatibility_7_8_2014=true;
    return(retVal);
}
bool CLuaScriptObject::checkAndSetWarning_simRMLPosition_oldCompatibility_30_8_2014()
{
    bool retVal=_warning_simRMLPosition_oldCompatibility_30_8_2014;
    _warning_simRMLPosition_oldCompatibility_30_8_2014=true;
    return(retVal);
}
bool CLuaScriptObject::checkAndSetWarning_simRMLVelocity_oldCompatibility_30_8_2014()
{
    bool retVal=_warning_simRMLVelocity_oldCompatibility_30_8_2014;
    _warning_simRMLVelocity_oldCompatibility_30_8_2014=true;
    return(retVal);
}
bool CLuaScriptObject::checkAndSetWarning_simGetMpConfigForTipPose_oldCompatibility_21_1_2016()
{
    bool retVal=_warning_simGetMpConfigForTipPose_oldCompatibility_21_1_2016;
    _warning_simGetMpConfigForTipPose_oldCompatibility_21_1_2016=true;
    return(retVal);
}
bool CLuaScriptObject::checkAndSetWarning_simFindIkPath_oldCompatibility_2_2_2016()
{
    bool retVal=_warning_simFindIkPath_oldCompatibility_2_2_2016;
    _warning_simFindIkPath_oldCompatibility_2_2_2016=true;
    return(retVal);
}

void CLuaScriptObject::_insertScriptText(CLuaScriptObject* scriptObject,bool toFront,const char* txt)
{
    std::string theScript(scriptObject->getScriptText());
    if (toFront)
        theScript=std::string(txt)+theScript;
    else
        theScript+=txt;
    scriptObject->setScriptText(theScript.c_str());

/*
    if (_scriptText_==nullptr)
        return;
    std::string theScript(_scriptText_);
    if (toFront)
        theScript=std::string(txt)+theScript;
    else
        theScript+=txt;
    delete[] _scriptText_;
    _scriptTextLength_=int(theScript.length())+1;
    _scriptText_=new char[_scriptTextLength_];
    for (int i=0;i<_scriptTextLength_-1;i++)
        _scriptText_[i]=theScript[i];
    _scriptText_[_scriptTextLength_-1]=0;
    */
}

std::string CLuaScriptObject::_replaceOldApi(const char* txt,bool forwardAdjustment)
{ // recursive
    size_t p=std::string(txt).find("sim");
    if (p!=std::string::npos)
    {
        std::string beforePart;
        std::string apiWord;
        std::string afterPart;
        _splitApiText(txt,p,beforePart,apiWord,afterPart);
        std::map<std::string,std::string>::iterator it=_newApiMap.find(apiWord);
        if (it!=_newApiMap.end())
        {
            apiWord=it->second;
            // Do a second stage replacement:
            std::map<std::string,std::string>::iterator it2=_newApiMap.find(apiWord);
            if (it2!=_newApiMap.end())
                apiWord=it2->second;
        }
        return(beforePart+apiWord+_replaceOldApi(afterPart.c_str(),forwardAdjustment));
    }
    return(std::string(txt));
}

void CLuaScriptObject::_performNewApiAdjustments(CLuaScriptObject* scriptObject,bool forwardAdjustment)
{
    std::vector<const SNewApiMapping*> all;
    if (_newApiMap.begin()==_newApiMap.end())
        all.push_back(_simApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simBubble"))
        all.push_back(_simBubbleApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simK3"))
        all.push_back(_simK3ApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simMTB"))
        all.push_back(_simMTBApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simOpenMesh"))
        all.push_back(_simOpenMeshApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simSkeleton"))
        all.push_back(_simSkeletonApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simQHull"))
        all.push_back(_simQHullApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simRemoteApi"))
        all.push_back(_simRemoteApiApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simRRS1"))
        all.push_back(_simRRS1ApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simVision"))
        all.push_back(_simVisionApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simCam"))
        all.push_back(_simCamApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simJoy"))
        all.push_back(_simJoyApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simWii"))
        all.push_back(_simWiiApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simURDF"))
        all.push_back(_simURDFApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simBWF"))
        all.push_back(_simBWFApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simUI"))
        all.push_back(_simUIApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simROS"))
        all.push_back(_simROSApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simICP"))
        all.push_back(_simICPApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simOMPL"))
        all.push_back(_simOMPLApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simSDF"))
        all.push_back(_simSDFApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simSurfRec"))
        all.push_back(_simSurfRecApiMapping);
    if (App::worldContainer->luaCustomFuncAndVarContainer->isVariableNamePresent("simx"))
        all.push_back(_simxApiMapping);

    for (size_t j=0;j<all.size();j++)
    {
        const SNewApiMapping* aMapping=all[j];
        for (int i=0;aMapping[i].oldApi!="";i++)
        {
            if (forwardAdjustment)
                _newApiMap[aMapping[i].oldApi]=aMapping[i].newApi;
            else
                _newApiMap[aMapping[i].newApi]=aMapping[i].oldApi;
        }
    }


    std::string theScript(scriptObject->getScriptText());
    theScript=_replaceOldApi(theScript.c_str(),forwardAdjustment);
    scriptObject->setScriptText(theScript.c_str());

/*
    if (_scriptText_!=nullptr)
    {

        std::string theScript(_scriptText_);
        theScript=_replaceOldApi(theScript,forwardAdjustment);

        delete[] _scriptText_;
        _scriptTextLength_=int(theScript.length())+1;
        _scriptText_=new char[_scriptTextLength_];
        for (int i=0;i<_scriptTextLength_-1;i++)
            _scriptText_[i]=theScript[i];
        _scriptText_[_scriptTextLength_-1]=0;
    }
            */
}

std::string CLuaScriptObject::extractScriptText(CLuaScriptObject* scriptObject,const char* startLine,const char* endLine,bool discardEndLine)
{
    std::string theScript(scriptObject->getScriptText());
    size_t startPos=theScript.find(startLine,0);
    if (startPos!=std::string::npos)
    {
        bool replacedSomething=false;
        size_t endPos=theScript.find(endLine,startPos);
        if (endPos==std::string::npos)
            return("");
        size_t endPos2=theScript.find("\n",endPos+1);
        if (endPos2==std::string::npos)
            endPos2=theScript.length();
        replacedSomething=true;
        std::string ret;
        if (discardEndLine)
        {
            ret.assign(theScript.begin()+startPos,theScript.begin()+endPos);
            theScript.replace(startPos,endPos2+1-startPos,"");
        }
        else
        {
            ret.assign(theScript.begin()+startPos,theScript.begin()+endPos2+2);
            theScript.replace(startPos,endPos2+1-startPos,"");
        }
        if (replacedSomething)
            scriptObject->setScriptText(theScript.c_str());
        return(ret);
    }
    return("");
}


bool CLuaScriptObject::_replaceScriptText(CLuaScriptObject* scriptObject,const char* oldTxt,const char* newTxt)
{
    std::string theScript(scriptObject->getScriptText());
    size_t startPos=theScript.find(oldTxt,0);
    bool replacedSomething=false;
    while (startPos!=std::string::npos)
    {
        theScript.replace(startPos,strlen(oldTxt),newTxt);
        startPos=theScript.find(oldTxt,startPos+1);
        replacedSomething=true;
    }
    if (replacedSomething)
        scriptObject->setScriptText(theScript.c_str());
    return(replacedSomething);
}

bool CLuaScriptObject::_replaceScriptTextKeepMiddleUnchanged(CLuaScriptObject* scriptObject,const char* oldTxtStart,const char* oldTxtEnd,const char* newTxtStart,const char* newTxtEnd)
{ // Will do following: oldTextStart*oldTextEnd --> nextTextStart*newTextEnd
    std::string theScript(scriptObject->getScriptText());
    size_t startPos=theScript.find(oldTxtStart,0);
    bool replacedSomething=false;
    while (startPos!=std::string::npos)
    {
        size_t startPos2=theScript.find(oldTxtEnd,startPos+strlen(oldTxtStart));
        if (startPos2!=std::string::npos)
        {
            // check if we have a line break in-between:
            bool lineBreak=false;
            for (size_t i=startPos;i<startPos2;i++)
            {
                if ( (theScript[i]==(unsigned char)13)||(theScript[i]==(unsigned char)10) )
                {
                    lineBreak=true;
                    break;
                }
            }
            if (!lineBreak)
            {
                theScript.replace(startPos2,strlen(oldTxtEnd),newTxtEnd);
                theScript.replace(startPos,strlen(oldTxtStart),newTxtStart);
                startPos=theScript.find(oldTxtStart,startPos2+strlen(newTxtEnd)+strlen(newTxtStart)-strlen(oldTxtStart));
                replacedSomething=true;
            }
            else
                startPos=theScript.find(oldTxtStart,startPos+1);
        }
        else
            startPos=theScript.find(oldTxtStart,startPos+1);
    }
    if (replacedSomething)
        scriptObject->setScriptText(theScript.c_str());
    return(replacedSomething);
}

bool CLuaScriptObject::_replaceScriptText(CLuaScriptObject* scriptObject,const char* oldTxt1,const char* oldTxt2,const char* oldTxt3,const char* newTxt)
{ // there can be spaces between the 3 words
    std::string theScript(scriptObject->getScriptText());
    size_t l1=strlen(oldTxt1);
    size_t l2=strlen(oldTxt2);
    size_t l3=strlen(oldTxt3);
    bool replacedSomething=false;
    size_t searchStart=0;
    while (searchStart<theScript.length())
    {
        size_t startPos1=theScript.find(oldTxt1,searchStart);
        if (startPos1!=std::string::npos)
        {
            searchStart=startPos1+1;
            size_t startPos2=theScript.find(oldTxt2,startPos1+l1);
            if (startPos2!=std::string::npos)
            {
                bool onlySpaces=true;
                size_t p=startPos1+l1;
                while (p<startPos2)
                {
                    if (theScript[p]!=' ')
                        onlySpaces=false;
                    p++;
                }
                if (onlySpaces)
                {
                    size_t startPos3=theScript.find(oldTxt3,startPos2+l2);
                    if (startPos3!=std::string::npos)
                    {
                        onlySpaces=true;
                        p=startPos2+l2;
                        while (p<startPos3)
                        {
                            if (theScript[p]!=' ')
                                onlySpaces=false;
                            p++;
                        }
                        if (onlySpaces)
                        { // ok!
                            theScript.replace(startPos1,startPos3-startPos1+l3,newTxt);
                            replacedSomething=true;
                        }
                    }
                }
            }
        }
        else
            searchStart=theScript.length();
    }
    if (replacedSomething)
        scriptObject->setScriptText(theScript.c_str());
    return(replacedSomething);
}

bool CLuaScriptObject::_containsScriptText(CLuaScriptObject* scriptObject,const char* txt)
{
    const std::string theScript(scriptObject->getScriptText());
    size_t startPos=theScript.find(txt);
    return(startPos!=std::string::npos);
}

int CLuaScriptObject::_countOccurences(const char* source,const char* word)
{
    int cnt=0;
    std::string ssource(source);
    size_t pos=ssource.find(word,0);
    while (pos!=std::string::npos)
    {
        cnt++;
        pos=ssource.find(word,pos+1);
    }
    return(cnt);
}

void CLuaScriptObject::_splitApiText(const char* txt,size_t pos,std::string& beforePart,std::string& apiWord,std::string& afterPart)
{
    size_t endPos;
    for (size_t i=pos;i<strlen(txt);i++)
    {
        char c=txt[i];
        if ( ((c>='0')&&(c<='9')) || ((c>='a')&&(c<='z')) || ((c>='A')&&(c<='Z')) || (c=='_') || (c=='.') )
            endPos=i+1;
        else
            break;
    }
    if (pos>0)
        beforePart.assign(txt,txt+pos);
    else
        beforePart.clear();
    apiWord.assign(txt+pos,txt+endPos);
    if (endPos<strlen(txt))
        afterPart=txt+endPos;
    else
        afterPart.clear();
}

void CLuaScriptObject::_adjustScriptText1(CLuaScriptObject* scriptObject,bool doIt,bool doIt2)
{
    if (!doIt)
        return;
    // here we have to adjust for the new script execution engine (since V3.1.3):
    if ( (scriptObject->getScriptType()==sim_scripttype_mainscript)&&(!scriptObject->_mainScriptIsDefaultMainScript_old) )
    {
        std::string txt;
        txt+=DEFAULT_MAINSCRIPT_CODE;
        txt+="\n";
        txt+=" \n";
        txt+=" \n";
        txt+="------------------------------------------------------------------------------ \n";
        txt+="-- Following main script automatically commented out by CoppeliaSim to guarantee \n";
        txt+="-- compatibility with CoppeliaSim 3.1.3 and later: \n";
        txt+=" \n";
        txt+="--[=[ \n";
        txt+=" \n";
        _insertScriptText(scriptObject,true,txt.c_str());
        txt="";
        txt+="\n";
        txt+=" \n";
        txt+=" \n";
        txt+="--]=] \n";
        txt+="------------------------------------------------------------------------------ \n";
        _insertScriptText(scriptObject,false,txt.c_str());
    }
    if (scriptObject->getScriptType()==sim_scripttype_childscript)
    {
        if (!scriptObject->getThreadedExecution())
        {

            _replaceScriptText(scriptObject,"\n","\n\t"); // "\r\n" is also handled

            std::string txt;
            if (doIt2)
            {
                // Add text to the beginning:
                txt+="------------------------------------------------------------------------------ \n";
                txt+="-- Following few lines automatically added by CoppeliaSim to guarantee compatibility \n";
                txt+="-- with CoppeliaSim 3.1.3 and later: \n";
                txt+="if (sim_call_type==sim.syscb_init) then \n";
                txt+=" \n";
                txt+="end \n";
                txt+="if (sim_call_type==sim.syscb_cleanup) then \n";
                txt+=" \n";
                txt+="end \n";
                txt+="if (sim_call_type==sim.syscb_sensing) then \n";
                txt+="  if not firstTimeHere93846738 then \n";
                txt+="      firstTimeHere93846738=0 \n";
                txt+="  end \n";
                txt+="  simSetScriptAttribute(sim_handle_self,sim_scriptattribute_executioncount,firstTimeHere93846738) \n";
                txt+="  firstTimeHere93846738=firstTimeHere93846738+1 \n";
                txt+=" \n";
                txt+="------------------------------------------------------------------------------ \n";
                txt+=" \n";
                txt+=" \n";
                _insertScriptText(scriptObject,true,txt.c_str());

                // Add text to the end:
                txt="\n";
                txt+=" \n";
                txt+=" \n";
                txt+="------------------------------------------------------------------------------ \n";
                txt+="-- Following few lines automatically added by CoppeliaSim to guarantee compatibility \n";
                txt+="-- with CoppeliaSim 3.1.3 and later: \n";
                txt+="end \n";
                txt+="------------------------------------------------------------------------------ \n";
                _insertScriptText(scriptObject,false,txt.c_str());

                // Because in old sensing child scripts, simHandleChildScript didn't anyway have an effect:
                _replaceScriptText(scriptObject,"simHandleChildScript(","-- commented by CoppeliaSim: s@imHandleChildScript(");
                _replaceScriptText(scriptObject,"s@imHandleChildScript","simHandleChildScript");
            }
            else
            { // actuation child script
                // Add text to the beginning:
                txt+="------------------------------------------------------------------------------ \n";
                txt+="-- Following few lines automatically added by CoppeliaSim to guarantee compatibility \n";
                txt+="-- with CoppeliaSim 3.1.3 and later: \n";
                txt+="if (sim_call_type==sim.syscb_init) then \n";
                txt+="  simSetScriptAttribute(sim_handle_self,sim_childscriptattribute_automaticcascadingcalls,false) \n";
                txt+="end \n";
                txt+="if (sim_call_type==sim.syscb_cleanup) then \n";
                txt+=" \n";
                txt+="end \n";
                txt+="if (sim_call_type==sim.syscb_sensing) then \n";
                txt+="  simHandleChildScripts(sim_call_type) \n";
                txt+="end \n";
                txt+="if (sim_call_type==sim.syscb_actuation) then \n";
                txt+="  if not firstTimeHere93846738 then \n";
                txt+="      firstTimeHere93846738=0 \n";
                txt+="  end \n";
                txt+="  simSetScriptAttribute(sim_handle_self,sim_scriptattribute_executioncount,firstTimeHere93846738) \n";
                txt+="  firstTimeHere93846738=firstTimeHere93846738+1 \n";
                txt+=" \n";
                txt+="------------------------------------------------------------------------------ \n";
                txt+=" \n";
                txt+=" \n";
                _insertScriptText(scriptObject,true,txt.c_str());

                // Add text to the end:
                txt="\n";
                txt+=" \n";
                txt+=" \n";
                txt+="------------------------------------------------------------------------------ \n";
                txt+="-- Following few lines automatically added by CoppeliaSim to guarantee compatibility \n";
                txt+="-- with CoppeliaSim 3.1.3 and later: \n";
                txt+="end \n";
                txt+="------------------------------------------------------------------------------ \n";
                _insertScriptText(scriptObject,false,txt.c_str());
                _replaceScriptText(scriptObject,"simHandleChildScript(","sim_handle_all_except_explicit",")","simHandleChildScripts(sim_call_type)");
                _replaceScriptText(scriptObject,"simHandleChildScript(","sim_handle_all",")","simHandleChildScripts(sim_call_type)");
                _replaceScriptText(scriptObject,"simHandleChildScript(","sim_handle_all_except_explicit",",","simHandleChildScripts(sim_call_type,");
                _replaceScriptText(scriptObject,"simHandleChildScript(","sim_handle_all",",","simHandleChildScripts(sim_call_type,");

                if (_containsScriptText(scriptObject,"simHandleChildScript("))
                { // output a warning
                    txt="Compatibility issue with @@REPLACE@@\n";
                    txt+="  Since CoppeliaSim 3.1.3, the function simHandleChildScript is not supported anymore.\n";
                    txt+="  It was replaced with simHandleChildScripts (i.e. with an additional 's'),\n";
                    txt+="  and operates slightly differently. CoppeliaSim has tried to automatically adjust\n";
                    txt+="  the script, but failed. Please correct this issue yourself by editing the script.";
                    CWorld::appendLoadOperationIssue(sim_verbosity_warnings,txt.c_str(),scriptObject->getScriptHandle());
                }
            }
        }
        else
        {
            if (_containsScriptText(scriptObject,"simHandleChildScript("))
            { // output a warning
                std::string txt="compatibility issue with @@REPLACE@@\n";
                txt+="  Since CoppeliaSim 3.1.3, the function simHandleChildScript is not supported anymore.\n";
                txt+="  It was replaced with simHandleChildScripts (i.e. with an additional 's'),\n";
                txt+="  and operates slightly differently. In addition to this, simhandleChildScripts\n";
                txt+="  cannot be called from threaded child scripts anymore. Please correct this issue\n";
                txt+="  yourself by editing the script.";
                CWorld::appendLoadOperationIssue(sim_verbosity_warnings,txt.c_str(),scriptObject->getScriptHandle());
            }
        }
    }
    if (scriptObject->getScriptType()==sim_scripttype_customizationscript)
    {
        _replaceScriptText(scriptObject,"sim_customizationscriptcall_firstaftersimulation","sim.syscb_aftersimulation");
        _replaceScriptText(scriptObject,"sim_customizationscriptcall_lastbeforesimulation","sim.syscb_beforesimulation");
        _replaceScriptText(scriptObject,"sim_customizationscriptcall_first","sim.syscb_init");
        _replaceScriptText(scriptObject,"sim_customizationscriptcall_last","sim.syscb_cleanup");
    }
}

void CLuaScriptObject::_adjustScriptText2(CLuaScriptObject* scriptObject,bool doIt)
{
    if (!doIt)
        return;
    if ( (scriptObject->getScriptType()==sim_scripttype_childscript)&&scriptObject->getThreadedExecution() )
    { // to correct for a forgotten thing. Happens only with files I modified between 11/8/2014 and 13/8/2014 (half of the demo scenes and models)
        _replaceScriptText(scriptObject,"pcall(threadFunction)","@@call(threadFunction)");
        _replaceScriptText(scriptObject,"@@call(threadFunction)","res,err=xpcall(threadFunction,function(err) return debug.traceback(err) end)\nif not res then\n\tsimAddStatusbarMessage('Lua runtime error: '..err)\nend");
    }
}

void CLuaScriptObject::_adjustScriptText3(CLuaScriptObject* scriptObject,bool doIt)
{
    if (!doIt)
        return;
    // 1. check if we haven't previously added the correction:
    if (!_containsScriptText(scriptObject,"@backCompatibility1:"))
    {
        bool modifiedSomething=_replaceScriptTextKeepMiddleUnchanged(scriptObject,"simSetShapeColor(",",","simSetShapeColor(colorCorrectionFunction(","),");

        if (modifiedSomething)
        {
            // Add text to the beginning:
            std::string txt;
            txt+="------------------------------------------------------------------------------ \n";
            txt+="-- Following few lines automatically added by CoppeliaSim to guarantee compatibility \n";
            txt+="-- with CoppeliaSim 3.1.3 and earlier: \n";
            txt+="colorCorrectionFunction=function(_aShapeHandle_) \n";
            txt+="  local version=simGetIntegerParameter(sim_intparam_program_version) \n";
            txt+="  local revision=simGetIntegerParameter(sim_intparam_program_revision) \n";
            txt+="  if (version==30103 and revision<3) or version<30103 then \n";
            txt+="      return _aShapeHandle_ \n";
            txt+="  end \n";
            txt+="  return '@backCompatibility1:'.._aShapeHandle_ \n";
            txt+="end \n";
            txt+="------------------------------------------------------------------------------ \n";
            txt+=" \n";
            txt+=" \n";
            _insertScriptText(scriptObject,true,txt.c_str());
        }
    }
}

void CLuaScriptObject::_adjustScriptText4(CLuaScriptObject* scriptObject,bool doIt)
{
    if (!doIt)
        return;
    _replaceScriptText(scriptObject,"res,err=pcall(threadFunction)","res,err=xpcall(threadFunction,function(err) return debug.traceback(err) end)");
}

void CLuaScriptObject::_adjustScriptText5(CLuaScriptObject* scriptObject,bool doIt)
{   // Following since 19/12/2015: not really needed, but better.
    if (!doIt)
        return;
    _replaceScriptText(scriptObject,"simGetBooleanParameter","simGetBoolParameter");
    _replaceScriptText(scriptObject,"simSetBooleanParameter","simSetBoolParameter");
    _replaceScriptText(scriptObject,"simGetIntegerParameter","simGetInt32Parameter");
    _replaceScriptText(scriptObject,"simSetIntegerParameter","simSetInt32Parameter");
    _replaceScriptText(scriptObject,"simGetFloatingParameter","simGetFloatParameter");
    _replaceScriptText(scriptObject,"simSetFloatingParameter","simSetFloatParameter");
    _replaceScriptText(scriptObject,"simGetObjectIntParameter","simGetObjectInt32Parameter");
    _replaceScriptText(scriptObject,"simSetObjectIntParameter","simSetObjectInt32Parameter");
}

void CLuaScriptObject::_adjustScriptText6(CLuaScriptObject* scriptObject,bool doIt)
{   // since 19/1/2016 we don't use tabs anymore in embedded scripts:
    if (!doIt)
        return;
    _replaceScriptText(scriptObject,"\t","    "); // tab to 4 spaces
}

void CLuaScriptObject::_adjustScriptText7(CLuaScriptObject* scriptObject,bool doIt)
{   // Following since 13/9/2016, but active only since V3.3.3 (or V3.4.0)
    if (!doIt)
        return;
    _replaceScriptText(scriptObject,"simPackInts","simPackInt32Table");
    _replaceScriptText(scriptObject,"simPackUInts","simPackUInt32Table");
    _replaceScriptText(scriptObject,"simPackFloats","simPackFloatTable");
    _replaceScriptText(scriptObject,"simPackDoubles","simPackDoubleTable");
    _replaceScriptText(scriptObject,"simPackBytes","simPackUInt8Table");
    _replaceScriptText(scriptObject,"simPackWords","simPackUInt16Table");
    _replaceScriptText(scriptObject,"simUnpackInts","simUnpackInt32Table");
    _replaceScriptText(scriptObject,"simUnpackUInts","simUnpackUInt32Table");
    _replaceScriptText(scriptObject,"simUnpackFloats","simUnpackFloatTable");
    _replaceScriptText(scriptObject,"simUnpackDoubles","simUnpackDoubleTable");
    _replaceScriptText(scriptObject,"simUnpackBytes","simUnpackUInt8Table");
    _replaceScriptText(scriptObject,"simUnpackWords","simUnpackUInt16Table");
}

void CLuaScriptObject::_adjustScriptText8(CLuaScriptObject* scriptObject,int adjust)
{   // Following since V3.4.1:
    if (adjust!=0)
        _performNewApiAdjustments(scriptObject,adjust>0);
}

void CLuaScriptObject::_adjustScriptText9(CLuaScriptObject* scriptObject)
{
    /*
    // ******************************************************
    // Some help to adjust for the new script calling conventions
    // Following script adjustments are not very reliable and should be manually checked!

    std::string errors="";
    if ( ((getScriptType()==sim_scripttype_childscript)&&(!getThreadedExecution()))||(getScriptType()==sim_scripttype_customizationscript) )
    {
        _replaceScriptText(scriptObject,"if (sim_call_type==sim.syscb_init) then","function sysCall_XXinit()");
        _replaceScriptText(scriptObject,"if (sim_call_type==sim.syscb_cleanup) then","function sysCall_cleanup()");
        _replaceScriptText(scriptObject,"if (sim_call_type==sim.syscb_actuation) then","function sysCall_actuation()");
        _replaceScriptText(scriptObject,"if (sim_call_type==sim.syscb_sensing) then","function sysCall_sensing()");
        if (_replaceScriptText(scriptObject,"sim.syscb_init","sim.syscb_init"))
            errors+="\n sim.syscb_init not converted";
        if (_replaceScriptText(scriptObject,"sim.syscb_cleanup","sim.syscb_cleanup"))
            errors+="\n sim.syscb_cleanup not converted";
        if (_replaceScriptText(scriptObject,"sim.syscb_actuation","sim.syscb_actuation"))
            errors+="\n sim.syscb_actuation not converted";
        if (_replaceScriptText(scriptObject,"sim.syscb_sensing","sim.syscb_sensing"))
            errors+="\n sim.syscb_sensing not converted";
        if (_replaceScriptText(scriptObject,"handleChildScripts","handleChildScripts"))
            errors+="\n ***** found 'handleChildScripts'";

        if (getScriptType()==sim_scripttype_customizationscript)
        {
            _replaceScriptText(scriptObject,"if (sim_call_type==sim.syscb_nonsimulation) then","function sysCall_nonSimulation()");
            _replaceScriptText(scriptObject,"if (sim_call_type==sim.syscb_beforesimulation) then","function sysCall_beforeSimulation()");
            _replaceScriptText(scriptObject,"if (sim_call_type==sim.syscb_suspended) then","function sysCall_suspended()");
            _replaceScriptText(scriptObject,"if (sim_call_type==sim.syscb_suspend) then","function sysCall_suspend()");
            _replaceScriptText(scriptObject,"if (sim_call_type==sim.syscb_resume) then","function sysCall_resume()");
            _replaceScriptText(scriptObject,"if (sim_call_type==sim.syscb_aftersimulation) then","function sysCall_afterSimulation()");
            _replaceScriptText(scriptObject,"if (sim_call_type==sim.syscb_beforeinstanceswitch) then","function sysCall_beforeInstanceSwitch()");
            _replaceScriptText(scriptObject,"if (sim_call_type==sim.syscb_afterinstanceswitch) then","function sysCall_afterInstanceSwitch()");
            _replaceScriptText(scriptObject,"if (sim_call_type==sim.syscb_beforecopy) then","function sysCall_beforeCopy()");
            _replaceScriptText(scriptObject,"if (sim_call_type==sim.syscb_aftercopy) then","function sysCall_afterCopy()");
            _replaceScriptText(scriptObject,"sim.setScriptAttribute(sim.handle_self,sim.customizationscriptattribute_activeduringsimulation,false)","");
            _replaceScriptText(scriptObject,"sim.setScriptAttribute(sim.handle_self,sim.customizationscriptattribute_activeduringsimulation,true)","");
            if (_replaceScriptText(scriptObject,"sim.syscb_nonsimulation","sim.syscb_nonsimulation"))
                errors+="\n sim.syscb_nonsimulation not converted";
            if (_replaceScriptText(scriptObject,"sim.syscb_beforesimulation","sim.syscb_beforesimulation"))
                errors+="\n sim.syscb_beforesimulation not converted";
            if (_replaceScriptText(scriptObject,"sim.syscb_suspended","sim.syscb_suspended"))
                errors+="\n sim.syscb_suspended not converted";
            if (_replaceScriptText(scriptObject,"sim.syscb_suspend","sim.syscb_suspend"))
                errors+="\n sim.syscb_suspend not converted";
            if (_replaceScriptText(scriptObject,"sim.syscb_resume","sim.syscb_resume"))
                errors+="\n sim.syscb_resume not converted";
            if (_replaceScriptText(scriptObject,"sim.syscb_aftersimulation","sim.syscb_aftersimulation"))
                errors+="\n sim.syscb_aftersimulation not converted";
            if (_replaceScriptText(scriptObject,"sim.syscb_beforeinstanceswitch","sim.syscb_beforeinstanceswitch"))
                errors+="\n sim.syscb_beforeinstanceswitch not converted";
            if (_replaceScriptText(scriptObject,"sim.syscb_afterinstanceswitch","sim.syscb_afterinstanceswitch"))
                errors+="\n sim.syscb_afterinstanceswitch not converted";
            if (_replaceScriptText(scriptObject,"sim.syscb_beforecopy","sim.syscb_beforecopy"))
                errors+="\n sim.syscb_beforecopy not converted";
            if (_replaceScriptText(scriptObject,"sim.syscb_aftercopy","sim.syscb_aftercopy"))
                errors+="\n sim.syscb_aftercopy not converted";
            if (_replaceScriptText(scriptObject,"sim.customizationscriptattribute_activeduringsimulation","sim.customizationscriptattribute_activeduringsimulation"))
                errors+="\n sim.customizationscriptattribute_activeduringsimulation not removed";

            if (_replaceScriptText(scriptObject,"require('utils')","require('utils')"))
                errors+="\n found 'require('utils')'";
        }
        if (!_replaceScriptText(scriptObject,"function sysCall_init()","function sysCall_init()"))
        {
            if (!_replaceScriptText(scriptObject,"sim.include","sim.include"))
            {
                if (_replaceScriptText(scriptObject,"function sysCall_XXinit()","function sysCall_init()"))
                {
                    std::string ohla=extractScriptText("function sysCall_init()","\nend",true);
                    if (ohla.size()==0)
                        errors+="\n problem with init section (1)";
                    else
                    {
                        int cnt=_countOccurences(ohla,"if ");
                        cnt+=_countOccurences(ohla,"while ");
                        cnt+=_countOccurences(ohla,"for ");
                        if (cnt!=_countOccurences(ohla,"end"))
                            errors+="\n possible error with additional 'end'";
                        ohla+="\nend\n";
                        _insertScriptText(scriptObject,true,ohla.c_str());
                    }
                }
                else
                    errors+="\n problem with init section (2)";
            }
        }
    }
    if (errors.size()>0)
    {
        printf("ERRORS: %s\n",errors.c_str());
        App::beep();
    }
    // ******************************************************
    // */
}

void CLuaScriptObject::_adjustScriptText10(CLuaScriptObject* scriptObject,bool doIt)
{   // some various small details:
    //   _replaceScriptTextKeepMiddleUnchanged(scriptObject,"sim.include('/",".lua')","require('/","')");
    //   _replaceScriptTextKeepMiddleUnchanged(scriptObject,"sim.include(\"/",".lua\")","require('/","')");
    //   _replaceScriptTextKeepMiddleUnchanged(scriptObject,"sim.include('\\",".lua')","require('\\","')");
    //   _replaceScriptTextKeepMiddleUnchanged(scriptObject,"sim.include(\"\\",".lua\")","require('\\","')");
    _replaceScriptText(scriptObject,"sim.include('/lua/graph_customization.lua')","graph=require('graph_customization')");
    _replaceScriptText(scriptObject,"require('/BlueWorkforce/","require('/bwf/");
    _replaceScriptText(scriptObject,"sim.include('/BlueWorkforce/","sim.include('/bwf/");
    if (!doIt)
        return;
    _replaceScriptText(scriptObject," onclose=\""," on-close=\"");
    _replaceScriptText(scriptObject," onchange=\""," on-change=\"");
    _replaceScriptText(scriptObject," onclick=\""," on-click=\"");
}

void CLuaScriptObject::_printContext(const char* str,size_t p)
{
    size_t off=60;
    if (p+off>=strlen(str))
        printf("** %s\n",str+p);
    else
    {
        std::string s;
        s.assign(str+p,str+p+off);
        printf("** %s\n",s.c_str());
    }
}

void CLuaScriptObject::_adjustScriptText11(CLuaScriptObject* scriptObject,bool doIt)
{ // A subtle bug was corrected in below function in CoppeliaSim4.0.1. Below to keep old code working as previously

    /*
    std::string s(scriptObject->getScriptText());

    size_t pf=0;
    size_t tp=s.find("print(",pf);
    while (tp!=std::string::npos)
    {
        _printContext(s.c_str(),tp);
        pf=tp+1;
        tp=s.find("print(",pf);
    }

    pf=0;
    tp=s.find("sim.addStatusbarMessage(",pf);
    while (tp!=std::string::npos)
    {
        _printContext(s.c_str(),tp);
        pf=tp+1;
        tp=s.find("sim.addStatusbarMessage(",pf);
    }

    pf=0;
    tp=s.find("simExt",pf);
    while (tp!=std::string::npos)
    {
        _printContext(s.c_str(),tp);
        pf=tp+1;
        tp=s.find("simExt",pf);
    }

    pf=0;
    tp=s.find("error_report_mode",pf);
    while (tp!=std::string::npos)
    {
        _printContext(s.c_str(),tp);
        pf=tp+1;
        tp=s.find("error_report_mode",pf);
    }
*/


    if (!doIt)
        return;

    std::string theScript;
    bool addFunc=false;

    theScript=(scriptObject->getScriptText());
    CTTUtil::regexReplace(theScript,"sim.getObjectOrientation\\(([^,]+),( *)-1( *)\\)","blabliblotemp($1,-1)");
    scriptObject->setScriptText(theScript.c_str());
    addFunc=_replaceScriptText(scriptObject,"sim.getObjectOrientation(","__getObjectOrientation__(");
    _replaceScriptText(scriptObject,"blabliblotemp","sim.getObjectOrientation");
    if (addFunc)
    {
        CWorld::appendLoadOperationIssue(sim_verbosity_warnings,"compatibility fix in script @@REPLACE@@:\n    replaced some occurrence of sim.getObjectOrientation with __getObjectOrientation__, to fix a possible bug in versions prior to CoppeliaSim V4.0.1.",scriptObject->getScriptHandle());
        std::string txt;
        txt+="function __getObjectOrientation__(a,b)\n";
        txt+="    -- compatibility routine, wrong results could be returned in some situations, in CoppeliaSim <4.0.1\n";
        txt+="    if b==sim.handle_parent then\n";
        txt+="        b=sim.getObjectParent(a)\n";
        txt+="    end\n";
        txt+="    if (b~=-1) and (sim.getObjectType(b)==sim.object_joint_type) and (sim.getInt32Parameter(sim.intparam_program_version)>=40001) then\n";
        txt+="        a=a+sim.handleflag_reljointbaseframe\n";
        txt+="    end\n";
        txt+="    return sim.getObjectOrientation(a,b)\n";
        txt+="end\n\n";
        _insertScriptText(scriptObject,true,txt.c_str());
    }

    theScript=scriptObject->getScriptText();
    CTTUtil::regexReplace(theScript,"sim.setObjectOrientation\\(([^,]+),( *)-1( *),","blabliblotemp($1,-1,");
    scriptObject->setScriptText(theScript.c_str());
    addFunc=_replaceScriptText(scriptObject,"sim.setObjectOrientation(","__setObjectOrientation__(");
    _replaceScriptText(scriptObject,"blabliblotemp","sim.setObjectOrientation");
    if (addFunc)
    {
        CWorld::appendLoadOperationIssue(sim_verbosity_warnings,"compatibility fix in script @@REPLACE@@:\n    replaced some occurrence of sim.setObjectOrientation with __setObjectOrientation__, to fix a possible bug in versions prior to CoppeliaSim V4.0.1.",scriptObject->getScriptHandle());
        std::string txt;
        txt+="function __setObjectOrientation__(a,b,c)\n";
        txt+="    -- compatibility routine, wrong results could be returned in some situations, in CoppeliaSim <4.0.1\n";
        txt+="    if b==sim.handle_parent then\n";
        txt+="        b=sim.getObjectParent(a)\n";
        txt+="    end\n";
        txt+="    if (b~=-1) and (sim.getObjectType(b)==sim.object_joint_type) and (sim.getInt32Parameter(sim.intparam_program_version)>=40001) then\n";
        txt+="        a=a+sim.handleflag_reljointbaseframe\n";
        txt+="    end\n";
        txt+="    return sim.setObjectOrientation(a,b,c)\n";
        txt+="end\n";
        _insertScriptText(scriptObject,true,txt.c_str());
    }

    theScript=scriptObject->getScriptText();
    CTTUtil::regexReplace(theScript,"sim.getObjectQuaternion\\(([^,]+),( *)-1( *)\\)","blabliblotemp($1,-1)");
    CTTUtil::regexReplace(theScript,"sim.getObjectQuaternion\\(([^,]+),( *)sim.handle_parent( *)\\)","blabliblotemp($1,sim.handle_parent)");
    scriptObject->setScriptText(theScript.c_str());
    addFunc=_replaceScriptText(scriptObject,"sim.getObjectQuaternion(","__getObjectQuaternion__(");
    _replaceScriptText(scriptObject,"blabliblotemp","sim.getObjectQuaternion");
    if (addFunc)
    {
        CWorld::appendLoadOperationIssue(sim_verbosity_warnings,"compatibility fix in script @@REPLACE@@:\n    replaced some occurrence of sim.getObjectQuaternion with __getObjectQuaternion__, to fix a possible bug in versions prior to CoppeliaSim V4.0.1.",scriptObject->getScriptHandle());
        std::string txt;
        txt+="function __getObjectQuaternion__(a,b)\n";
        txt+="    -- compatibility routine, wrong results could be returned in some situations, in CoppeliaSim <4.0.1\n";
        txt+="    if b==sim.handle_parent then\n";
        txt+="        b=sim.getObjectParent(a)\n";
        txt+="    end\n";
        txt+="    if (b~=-1) and (sim.getObjectType(b)==sim.object_joint_type) and (sim.getInt32Parameter(sim.intparam_program_version)>=40001) then\n";
        txt+="        a=a+sim.handleflag_reljointbaseframe\n";
        txt+="    end\n";
        txt+="    return sim.getObjectQuaternion(a,b)\n";
        txt+="end\n";
        _insertScriptText(scriptObject,true,txt.c_str());
    }

    theScript=scriptObject->getScriptText();
    CTTUtil::regexReplace(theScript,"sim.setObjectQuaternion\\(([^,]+),( *)-1( *),","blabliblotemp($1,-1,");
    CTTUtil::regexReplace(theScript,"sim.setObjectQuaternion\\(([^,]+),( *)sim.handle_parent( *),","blabliblotemp($1,sim.handle_parent,");
    scriptObject->setScriptText(theScript.c_str());
    addFunc=_replaceScriptText(scriptObject,"sim.setObjectQuaternion(","__setObjectQuaternion__(");
    _replaceScriptText(scriptObject,"blabliblotemp","sim.setObjectQuaternion");
    if (addFunc)
    {
        CWorld::appendLoadOperationIssue(sim_verbosity_warnings,"compatibility fix in script @@REPLACE@@:\n    replaced some occurrence of sim.setObjectQuaternion with __setObjectQuaternion__, to fix a possible bug in versions prior to CoppeliaSim V4.0.1.",scriptObject->getScriptHandle());
        std::string txt;
        txt+="function __setObjectQuaternion__(a,b,c)\n";
        txt+="    -- compatibility routine, wrong results could be returned in some situations, in CoppeliaSim <4.0.1\n";
        txt+="    if b==sim.handle_parent then\n";
        txt+="        b=sim.getObjectParent(a)\n";
        txt+="    end\n";
        txt+="    if (b~=-1) and (sim.getObjectType(b)==sim.object_joint_type) and (sim.getInt32Parameter(sim.intparam_program_version)>=40001) then\n";
        txt+="        a=a+sim.handleflag_reljointbaseframe\n";
        txt+="    end\n";
        txt+="    return sim.setObjectQuaternion(a,b,c)\n";
        txt+="end\n";
        _insertScriptText(scriptObject,true,txt.c_str());
    }

    theScript=scriptObject->getScriptText();
    CTTUtil::regexReplace(theScript,"sim.getObjectPosition\\(([^,]+),( *)-1( *)\\)","blabliblotemp($1,-1)");
    CTTUtil::regexReplace(theScript,"sim.getObjectPosition\\(([^,]+),( *)sim.handle_parent( *)\\)","blabliblotemp($1,sim.handle_parent)");
    scriptObject->setScriptText(theScript.c_str());
    addFunc=_replaceScriptText(scriptObject,"sim.getObjectPosition(","__getObjectPosition__(");
    _replaceScriptText(scriptObject,"blabliblotemp","sim.getObjectPosition");
    if (addFunc)
    {
        CWorld::appendLoadOperationIssue(sim_verbosity_warnings,"compatibility fix in script @@REPLACE@@:\n    replaced some occurrence of sim.getObjectPosition with __getObjectPosition__, to fix a possible bug in versions prior to CoppeliaSim V4.0.1.",scriptObject->getScriptHandle());
        std::string txt;
        txt+="function __getObjectPosition__(a,b)\n";
        txt+="    -- compatibility routine, wrong results could be returned in some situations, in CoppeliaSim <4.0.1\n";
        txt+="    if b==sim.handle_parent then\n";
        txt+="        b=sim.getObjectParent(a)\n";
        txt+="    end\n";
        txt+="    if (b~=-1) and (sim.getObjectType(b)==sim.object_joint_type) and (sim.getInt32Parameter(sim.intparam_program_version)>=40001) then\n";
        txt+="        a=a+sim.handleflag_reljointbaseframe\n";
        txt+="    end\n";
        txt+="    return sim.getObjectPosition(a,b)\n";
        txt+="end\n";
        _insertScriptText(scriptObject,true,txt.c_str());
    }

    theScript=scriptObject->getScriptText();
    CTTUtil::regexReplace(theScript,"sim.setObjectPosition\\(([^,]+),( *)-1( *),","blabliblotemp($1,-1,");
    CTTUtil::regexReplace(theScript,"sim.setObjectPosition\\(([^,]+),( *)sim.handle_parent( *),","blabliblotemp($1,sim.handle_parent,");
    scriptObject->setScriptText(theScript.c_str());
    addFunc=_replaceScriptText(scriptObject,"sim.setObjectPosition(","__setObjectPosition__(");
    _replaceScriptText(scriptObject,"blabliblotemp","sim.setObjectPosition");
    if (addFunc)
    {
        CWorld::appendLoadOperationIssue(sim_verbosity_warnings,"compatibility fix in script @@REPLACE@@:\n    replaced some occurrence of sim.setObjectPosition with __setObjectPosition__, to fix a possible bug in versions prior to CoppeliaSim V4.0.1.",scriptObject->getScriptHandle());
        std::string txt;
        txt+="function __setObjectPosition__(a,b,c)\n";
        txt+="    -- compatibility routine, wrong results could be returned in some situations, in CoppeliaSim <4.0.1\n";
        txt+="    if b==sim.handle_parent then\n";
        txt+="        b=sim.getObjectParent(a)\n";
        txt+="    end\n";
        txt+="    if (b~=-1) and (sim.getObjectType(b)==sim.object_joint_type) and (sim.getInt32Parameter(sim.intparam_program_version)>=40001) then\n";
        txt+="        a=a+sim.handleflag_reljointbaseframe\n";
        txt+="    end\n";
        txt+="    return sim.setObjectPosition(a,b,c)\n";
        txt+="end\n";
        _insertScriptText(scriptObject,true,txt.c_str());
    }
}

void CLuaScriptObject::_adjustScriptText12(CLuaScriptObject* scriptObject,bool doIt)
{   // ROS2 functions:
    if (!doIt)
        return;
    _replaceScriptText(scriptObject,"simROS2.serviceClientTreatUInt8ArrayAsString","simROS2.clientTreatUInt8ArrayAsString");
    _replaceScriptText(scriptObject,"simROS2.serviceServerTreatUInt8ArrayAsString","simROS2.serviceTreatUInt8ArrayAsString");
    _replaceScriptText(scriptObject,"simROS2.subscriberTreatUInt8ArrayAsString","simROS2.subscriptionTreatUInt8ArrayAsString");
    _replaceScriptText(scriptObject,"simROS2.imageTransportShutdownSubscriber","simROS2.imageTransportShutdownSubscription");
    _replaceScriptText(scriptObject,"simROS2.imageTransportSubscribe","simROS2.imageTransportCreateSubscription");
    _replaceScriptText(scriptObject,"simROS2.imageTransportAdvertise","simROS2.imageTransportCreatePublisher");
    _replaceScriptText(scriptObject,"simROS2.shutdownServiceServer","simROS2.shutdownService");
    _replaceScriptText(scriptObject,"simROS2.shutdownServiceClient","simROS2.shutdownClient");
    _replaceScriptText(scriptObject,"simROS2.shutdownSubscriber","simROS2.shutdownSubscription");
    _replaceScriptText(scriptObject,"simROS2.advertiseService","simROS2.createService");
    _replaceScriptText(scriptObject,"simROS2.serviceClient","simROS2.createClient");
    _replaceScriptText(scriptObject,"simROS2.subscribe","simROS2.createSubscription");
    _replaceScriptText(scriptObject,"simROS2.advertise","simROS2.createPublisher");
}

void CLuaScriptObject::_adjustScriptText13(CLuaScriptObject* scriptObject,bool doIt)
{   // for release 4.2.0:
    if (!doIt)
        return;
    if (_scriptType!=sim_scripttype_mainscript)
        _replaceScriptText(scriptObject,"sim.getSimulationState()~=sim.simulation_advancing_abouttostop","true");
    _replaceScriptText(scriptObject,"sim.getObjectAssociatedWithScript(sim.handle_self)","sim.getObjectHandle(sim.handle_self)");

    if ( (App::userSettings->xrTest==123456789)&&(_scriptType!=sim_scripttype_mainscript) )
    {
        const char txt1[]="function sysCall_actuation()\n\
    if coroutine.status(corout)~='dead' then\n\
        local ok,errorMsg=coroutine.resume(corout)\n\
        if errorMsg then\n\
            error(errorMsg)\n\
        end\n";
        const char txt2[]="function sysCall_actuation()\n\
    if coroutine.status(corout)~='dead' then\n\
        local ok,errorMsg=coroutine.resume(corout)\n\
        if errorMsg then\n\
            error(debug.traceback(corout,errorMsg),2)\n\
        end\n";
        _replaceScriptText(scriptObject,txt1,txt2);


        if (_containsScriptText(scriptObject," thread"))
            App::logMsg(sim_verbosity_errors,"Contains the word 'thread'");
        if (_containsScriptText(scriptObject," Thread"))
            App::logMsg(sim_verbosity_errors,"Contains the word 'Thread'");
        if (_containsScriptText(scriptObject,"sim.rmlMove"))
            App::logMsg(sim_verbosity_errors,"Contains sim.rmlMove...");
        if (_containsScriptText(scriptObject,"sim.getIk"))
            App::logMsg(sim_verbosity_errors,"Contains sim.getIk...");
        if (_containsScriptText(scriptObject,"sim.getScriptSimulationParameter"))
            App::logMsg(sim_verbosity_errors,"Contains sim.getScriptSimulationParameter...");
        if (_containsScriptText(scriptObject,"sim.setScriptSimulationParameter"))
            App::logMsg(sim_verbosity_errors,"Contains sim.setScriptSimulationParameter...");
        if (_containsScriptText(scriptObject,"sim.tube"))
            App::logMsg(sim_verbosity_errors,"Contains sim.tube...");
        if (_containsScriptText(scriptObject,"sim.addStatusbarMessage"))
            App::logMsg(sim_verbosity_errors,"Contains sim.addStatusbarMessage...");
        if (_containsScriptText(scriptObject,"sim.getNameSuffix"))
            App::logMsg(sim_verbosity_errors,"Contains sim.getNameSuffix...");
        if (_containsScriptText(scriptObject,"sim.setNameSuffix"))
            App::logMsg(sim_verbosity_errors,"Contains sim.setNameSuffix...");
        if (_containsScriptText(scriptObject,"sim.resetMill"))
            App::logMsg(sim_verbosity_errors,"Contains sim.resetMill...");
        if (_containsScriptText(scriptObject,"sim.handleMill"))
            App::logMsg(sim_verbosity_errors,"Contains sim.handleMill...");
        if (_containsScriptText(scriptObject,"sim.resetMilling"))
            App::logMsg(sim_verbosity_errors,"Contains sim.resetMilling...");
        if (_containsScriptText(scriptObject,"sim.openTextEditor"))
            App::logMsg(sim_verbosity_errors,"Contains sim.openTextEditor...");
        if (_containsScriptText(scriptObject,"sim.closeTextEditor"))
            App::logMsg(sim_verbosity_errors,"Contains sim.closeTextEditor...");
        if (_containsScriptText(scriptObject,"simGetMaterialId"))
            App::logMsg(sim_verbosity_errors,"Contains simGetMaterialId...");
        if (_containsScriptText(scriptObject,"simGetShapeMaterial"))
            App::logMsg(sim_verbosity_errors,"Contains simGetShapeMaterial...");
        if (_containsScriptText(scriptObject,"simHandleVarious"))
            App::logMsg(sim_verbosity_errors,"Contains simHandleVarious...");
        if (_containsScriptText(scriptObject,"simGetInstanceIndex"))
            App::logMsg(sim_verbosity_errors,"Contains simGetInstanceIndex...");
        if (_containsScriptText(scriptObject,"simGetVisibleInstanceIndex"))
            App::logMsg(sim_verbosity_errors,"Contains simGetVisibleInstanceIndex...");
        if (_containsScriptText(scriptObject,"simResetPath"))
            App::logMsg(sim_verbosity_errors,"Contains simResetPath...");
        if (_containsScriptText(scriptObject,"simHandlePath"))
            App::logMsg(sim_verbosity_errors,"Contains simHandlePath...");
        if (_containsScriptText(scriptObject,"simResetJoint"))
            App::logMsg(sim_verbosity_errors,"Contains simResetJoint...");
        if (_containsScriptText(scriptObject,"simHandleJoint"))
            App::logMsg(sim_verbosity_errors,"Contains simHandleJoint...");
        if (_containsScriptText(scriptObject,"simGetInvertedMatrix"))
            App::logMsg(sim_verbosity_errors,"Contains simGetInvertedMatrix...");
        if (_containsScriptText(scriptObject,"simAddSceneCustomData"))
            App::logMsg(sim_verbosity_errors,"Contains simAddSceneCustomData...");
        if (_containsScriptText(scriptObject,"simGetSceneCustomData"))
            App::logMsg(sim_verbosity_errors,"Contains simGetSceneCustomData...");
        if (_containsScriptText(scriptObject,"simAddObjectCustomData"))
            App::logMsg(sim_verbosity_errors,"Contains simAddObjectCustomData...");
        if (_containsScriptText(scriptObject,"simGetObjectCustomData"))
            App::logMsg(sim_verbosity_errors,"Contains simGetObjectCustomData...");
        if (_containsScriptText(scriptObject,"sim.setVisionSensorFilter"))
            App::logMsg(sim_verbosity_errors,"Contains sim.setVisionSensorFilter...");
        if (_containsScriptText(scriptObject,"sim.getVisionSensorFilter"))
            App::logMsg(sim_verbosity_errors,"Contains sim.getVisionSensorFilter...");
        if (_containsScriptText(scriptObject,"sim.handleMechanism"))
            App::logMsg(sim_verbosity_errors,"Contains sim.handleMechanism...");
        if (_containsScriptText(scriptObject,"sim.setPathTargetNominalVelocity"))
            App::logMsg(sim_verbosity_errors,"Contains sim.setPathTargetNominalVelocity...");
        if (_containsScriptText(scriptObject,"sim.setShapeMassAndInertia"))
            App::logMsg(sim_verbosity_errors,"Contains sim.setShapeMassAndInertia...");
        if (_containsScriptText(scriptObject,"sim.getShapeMassAndInertia"))
            App::logMsg(sim_verbosity_errors,"Contains sim.getShapeMassAndInertia...");
        if (_containsScriptText(scriptObject,"sim.checkIkGroup"))
            App::logMsg(sim_verbosity_errors,"Contains sim.checkIkGroup...");
        if (_containsScriptText(scriptObject,"sim.handleIkGroup"))
            App::logMsg(sim_verbosity_errors,"Contains sim.handleIkGroup...");
        if (_containsScriptText(scriptObject,"sim.createIkGroup"))
            App::logMsg(sim_verbosity_errors,"Contains sim.createIkGroup...");
        if (_containsScriptText(scriptObject,"sim.removeIkGroup"))
            App::logMsg(sim_verbosity_errors,"Contains sim.removeIkGroup...");
        if (_containsScriptText(scriptObject,"sim.createIkElement"))
            App::logMsg(sim_verbosity_errors,"Contains sim.createIkElement...");
        if (_containsScriptText(scriptObject,"sim.exportIk"))
            App::logMsg(sim_verbosity_errors,"Contains sim.exportIk...");
        if (_containsScriptText(scriptObject,"sim.computeJacobian"))
            App::logMsg(sim_verbosity_errors,"Contains sim.computeJacobian...");
        if (_containsScriptText(scriptObject,"sim.getConfigForTipPose"))
            App::logMsg(sim_verbosity_errors,"Contains sim.getConfigForTipPose...");
        if (_containsScriptText(scriptObject,"sim.generateIkPath"))
            App::logMsg(sim_verbosity_errors,"Contains sim.generateIkPath...");
        if (_containsScriptText(scriptObject,"sim.getIkGroupHandle"))
            App::logMsg(sim_verbosity_errors,"Contains sim.getIkGroupHandle...");
        if (_containsScriptText(scriptObject,"sim.getIkGroupMatrix"))
            App::logMsg(sim_verbosity_errors,"Contains sim.getIkGroupMatrix...");
        if (_containsScriptText(scriptObject,"sim.setIkGroupProperties"))
            App::logMsg(sim_verbosity_errors,"Contains sim.setIkGroupProperties...");
        if (_containsScriptText(scriptObject,"sim.setIkElementProperties"))
            App::logMsg(sim_verbosity_errors,"Contains sim.setIkElementProperties...");
        if (_containsScriptText(scriptObject,"sim.setThreadIsFree"))
            App::logMsg(sim_verbosity_errors,"Contains sim.setThreadIsFree...");
        if (_containsScriptText(scriptObject,"simSetUIPosition"))
            App::logMsg(sim_verbosity_errors,"Contains simSetUIPosition...");
        if (_containsScriptText(scriptObject,"simGetUIPosition"))
            App::logMsg(sim_verbosity_errors,"Contains simGetUIPosition...");
        if (_containsScriptText(scriptObject,"simGetUIHandle"))
            App::logMsg(sim_verbosity_errors,"Contains simGetUIHandle...");
        if (_containsScriptText(scriptObject,"simGetUIProperty"))
            App::logMsg(sim_verbosity_errors,"Contains simGetUIProperty...");
        if (_containsScriptText(scriptObject,"simSetUIProperty"))
            App::logMsg(sim_verbosity_errors,"Contains simSetUIProperty...");
        if (_containsScriptText(scriptObject,"simGetUIEventButton"))
            App::logMsg(sim_verbosity_errors,"Contains simGetUIEventButton...");
        if (_containsScriptText(scriptObject,"simGetUIButtonProperty"))
            App::logMsg(sim_verbosity_errors,"Contains simGetUIButtonProperty...");
        if (_containsScriptText(scriptObject,"simSetUIButtonProperty"))
            App::logMsg(sim_verbosity_errors,"Contains simSetUIButtonProperty...");
        if (_containsScriptText(scriptObject,"simGetUIButtonSize"))
            App::logMsg(sim_verbosity_errors,"Contains simGetUIButtonSize...");
        if (_containsScriptText(scriptObject,"simSetUIButtonLabel"))
            App::logMsg(sim_verbosity_errors,"Contains simSetUIButtonLabel...");
        if (_containsScriptText(scriptObject,"simGetUIButtonLabel"))
            App::logMsg(sim_verbosity_errors,"Contains simGetUIButtonLabel...");
        if (_containsScriptText(scriptObject,"simSetUISlider"))
            App::logMsg(sim_verbosity_errors,"Contains simSetUISlider...");
        if (_containsScriptText(scriptObject,"simGetUISlider"))
            App::logMsg(sim_verbosity_errors,"Contains simGetUISlider...");
        if (_containsScriptText(scriptObject,"simCreateUIButtonArray"))
            App::logMsg(sim_verbosity_errors,"Contains simCreateUIButtonArray...");
        if (_containsScriptText(scriptObject,"simSetUIButtonArrayColor"))
            App::logMsg(sim_verbosity_errors,"Contains simSetUIButtonArrayColor...");
        if (_containsScriptText(scriptObject,"simDeleteUIButtonArray"))
            App::logMsg(sim_verbosity_errors,"Contains simDeleteUIButtonArray...");
        if (_containsScriptText(scriptObject,"simCreateUI"))
            App::logMsg(sim_verbosity_errors,"Contains simCreateUI...");
        if (_containsScriptText(scriptObject,"simCreateUIButton"))
            App::logMsg(sim_verbosity_errors,"Contains simCreateUIButton...");
        if (_containsScriptText(scriptObject,"simLoadUI"))
            App::logMsg(sim_verbosity_errors,"Contains simLoadUI...");
        if (_containsScriptText(scriptObject,"simSaveUI"))
            App::logMsg(sim_verbosity_errors,"Contains simSaveUI...");
        if (_containsScriptText(scriptObject,"simRemoveUI"))
            App::logMsg(sim_verbosity_errors,"Contains simRemoveUI...");
        if (_containsScriptText(scriptObject,"simSetUIButtonColor"))
            App::logMsg(sim_verbosity_errors,"Contains simSetUIButtonColor...");
        if (_containsScriptText(scriptObject,"simHandleChildScript"))
            App::logMsg(sim_verbosity_errors,"Contains simHandleChildScript...");
        if (_containsScriptText(scriptObject,"simSearchPath"))
            App::logMsg(sim_verbosity_errors,"Contains simSearchPath...");
        if (_containsScriptText(scriptObject,"simInitializePathSearch"))
            App::logMsg(sim_verbosity_errors,"Contains simInitializePathSearch...");
        if (_containsScriptText(scriptObject,"simPerformPathSearchStep"))
            App::logMsg(sim_verbosity_errors,"Contains simPerformPathSearchStep...");
        if (_containsScriptText(scriptObject,"sim.sendData"))
            App::logMsg(sim_verbosity_errors,"Contains sim.sendData...");
        if (_containsScriptText(scriptObject,"sim.receiveData"))
            App::logMsg(sim_verbosity_errors,"Contains sim.receiveData...");
        if (_containsScriptText(scriptObject,"simSerialPortOpen"))
            App::logMsg(sim_verbosity_errors,"Contains simSerialPortOpen...");
        if (_containsScriptText(scriptObject,"simSerialPortClose"))
            App::logMsg(sim_verbosity_errors,"Contains simSerialPortClose...");
        if (_containsScriptText(scriptObject,"simSerialPortSend"))
            App::logMsg(sim_verbosity_errors,"Contains simSerialPortSend...");
        if (_containsScriptText(scriptObject,"simSerialPortRead"))
            App::logMsg(sim_verbosity_errors,"Contains simSerialPortRead...");
        if (_containsScriptText(scriptObject,"sim.rmlMoveToJointPositions"))
            App::logMsg(sim_verbosity_errors,"Contains sim.rmlMoveToJointPositions...");
        if (_containsScriptText(scriptObject,"simRMLMoveToJointPositions"))
            App::logMsg(sim_verbosity_errors,"Contains simRMLMoveToJointPositions...");
        if (_containsScriptText(scriptObject,"sim.rmlMoveToPosition"))
            App::logMsg(sim_verbosity_errors,"Contains sim.rmlMoveToPosition...");
        if (_containsScriptText(scriptObject,"simRMLMoveToPosition"))
            App::logMsg(sim_verbosity_errors,"Contains simRMLMoveToPosition...");

        if (_containsScriptText(scriptObject,"sim.getCollectionHandle"))
            App::logMsg(sim_verbosity_errors,"Contains sim.getCollectionHandle...");
        if (_containsScriptText(scriptObject,"sim.createCollection"))
            App::logMsg(sim_verbosity_errors,"Contains sim.createCollection...");
        if (_containsScriptText(scriptObject,"sim.addObjectToCollection"))
            App::logMsg(sim_verbosity_errors,"Contains sim.addObjectToCollection...");
        if (_containsScriptText(scriptObject,"sim.emptyCollection"))
            App::logMsg(sim_verbosity_errors,"Contains sim.emptyCollection...");
        if (_containsScriptText(scriptObject,"sim.removeCollection"))
            App::logMsg(sim_verbosity_errors,"Contains sim.removeCollection...");
        if (_containsScriptText(scriptObject,"sim.getCollectionName"))
            App::logMsg(sim_verbosity_errors,"Contains sim.getCollectionName...");
        if (_containsScriptText(scriptObject,"sim.setCollectionName"))
            App::logMsg(sim_verbosity_errors,"Contains sim.setCollectionName...");
        if (_containsScriptText(scriptObject,"sim.getCollisionHandle"))
            App::logMsg(sim_verbosity_errors,"Contains sim.getCollisionHandle...");
        if (_containsScriptText(scriptObject,"sim.handleCollision"))
            App::logMsg(sim_verbosity_errors,"Contains sim.handleCollision...");
        if (_containsScriptText(scriptObject,"sim.readCollision"))
            App::logMsg(sim_verbosity_errors,"Contains sim.readCollision...");
        if (_containsScriptText(scriptObject,"sim.resetCollision"))
            App::logMsg(sim_verbosity_errors,"Contains sim.resetCollision...");
        if (_containsScriptText(scriptObject,"sim.getDistanceHandle"))
            App::logMsg(sim_verbosity_errors,"Contains sim.getDistanceHandle...");
        if (_containsScriptText(scriptObject,"sim.handleDistance"))
            App::logMsg(sim_verbosity_errors,"Contains sim.handleDistance...");
        if (_containsScriptText(scriptObject,"sim.readDistance"))
            App::logMsg(sim_verbosity_errors,"Contains sim.readDistance...");
        if (_containsScriptText(scriptObject,"sim.resetDistance"))
            App::logMsg(sim_verbosity_errors,"Contains sim.resetDistance...");
        if (_containsScriptText(scriptObject,"sim.boolAnd32"))
            App::logMsg(sim_verbosity_errors,"Contains sim.boolAnd32...");
        if (_containsScriptText(scriptObject,"sim.boolOr32"))
            App::logMsg(sim_verbosity_errors,"Contains sim.boolOr32...");
        if (_containsScriptText(scriptObject,"sim.boolXor32"))
            App::logMsg(sim_verbosity_errors,"Contains sim.boolXor32...");
        if (_containsScriptText(scriptObject,"simBoolAnd32"))
            App::logMsg(sim_verbosity_errors,"Contains simBoolAnd32...");
        if (_containsScriptText(scriptObject,"simBoolOr32"))
            App::logMsg(sim_verbosity_errors,"Contains simBoolOr32...");
        if (_containsScriptText(scriptObject,"simBoolXor32"))
            App::logMsg(sim_verbosity_errors,"Contains simBoolXo32...");
        if (_containsScriptText(scriptObject,"simBoolAnd16"))
            App::logMsg(sim_verbosity_errors,"Contains simBoolAnd16...");
        if (_containsScriptText(scriptObject,"simBoolOr16"))
            App::logMsg(sim_verbosity_errors,"Contains simBoolOr16...");
        if (_containsScriptText(scriptObject,"simBoolXor16"))
            App::logMsg(sim_verbosity_errors,"Contains simBoolXo16...");

        if (_containsScriptText(scriptObject,"sim.getScriptExecutionCount"))
            App::logMsg(sim_verbosity_errors,"Contains sim.getScriptExecutionCount...");
        if (_containsScriptText(scriptObject,"sim.isScriptRunningInThread"))
            App::logMsg(sim_verbosity_errors,"Contains sim.isScriptRunningInThread...");
        if (_containsScriptText(scriptObject,"sim.isScriptExecutionThreaded"))
            App::logMsg(sim_verbosity_errors,"Contains sim.isScriptExecutionThreaded...");
        if (_containsScriptText(scriptObject,"sim.setThreadResumeLocation"))
            App::logMsg(sim_verbosity_errors,"Contains sim.setThreadResumeLocation...");
        if (_containsScriptText(scriptObject,"sim.resumeThreads"))
            App::logMsg(sim_verbosity_errors,"Contains sim.resumeThreads...");
        if (_containsScriptText(scriptObject,"sim.launchThreadedChildScripts"))
            App::logMsg(sim_verbosity_errors,"Contains sim.launchThreadedChildScripts...");
        if (_containsScriptText(scriptObject,"simGetScriptExecutionCount"))
            App::logMsg(sim_verbosity_errors,"Contains simGetScriptExecutionCount...");
        if (_containsScriptText(scriptObject,"simIsScriptExecutionThreaded"))
            App::logMsg(sim_verbosity_errors,"Contains simIsScriptExecutionThreaded...");
        if (_containsScriptText(scriptObject,"simIsScriptRunningInThread"))
            App::logMsg(sim_verbosity_errors,"Contains simIsScriptRunningInThread...");
        if (_containsScriptText(scriptObject,"simSetThreadResumeLocation"))
            App::logMsg(sim_verbosity_errors,"Contains simSetThreadResumeLocation...");
        if (_containsScriptText(scriptObject,"simResumeThreads"))
            App::logMsg(sim_verbosity_errors,"Contains simResumeThreads...");
        if (_containsScriptText(scriptObject,"simLaunchThreadedChildScripts"))
            App::logMsg(sim_verbosity_errors,"Contains simLaunchThreadedChildScripts...");
        if (_containsScriptText(scriptObject,"'utils'"))
            App::logMsg(sim_verbosity_errors,"Contains 'utils'...");


        //************************************************************
        // Scripts containing following should remain handled in threaded mode:
        if (_containsScriptText(scriptObject,"simMoveToPosition"))
            App::logMsg(sim_verbosity_errors,"Contains simMoveToPosition...");
        if (_containsScriptText(scriptObject,"sim.moveToPosition"))
            App::logMsg(sim_verbosity_errors,"Contains sim.moveToPosition...");
        if (_containsScriptText(scriptObject,"simMoveToObject"))
            App::logMsg(sim_verbosity_errors,"Contains simMoveToObject...");
        if (_containsScriptText(scriptObject,"sim.moveToObject"))
            App::logMsg(sim_verbosity_errors,"Contains sim.moveToObject...");
        if (_containsScriptText(scriptObject,"simFollowPath"))
            App::logMsg(sim_verbosity_errors,"Contains simFollowPath...");
        if (_containsScriptText(scriptObject,"sim.followPath"))
            App::logMsg(sim_verbosity_errors,"Contains sim.followPath...");
        if (_containsScriptText(scriptObject,"simMoveToJointPositions"))
            App::logMsg(sim_verbosity_errors,"Contains simMoveToJointPositions...");
        if (_containsScriptText(scriptObject,"sim.moveToJointPositions"))
            App::logMsg(sim_verbosity_errors,"Contains sim.moveToJointPositions...");
        //************************************************************
    }
}

bool CLuaScriptObject::_convertThreadedScriptToCoroutine(CLuaScriptObject* scriptObject)
{ // try to transform the threaded script into a non-threaded script with coroutines:
    bool retVal=false;
    if (_containsScriptText(scriptObject,"sysCall_threadmain"))
    {
        retVal=true;
        // Below functions are deprecated and won't work in a non-threaded script running a coroutine:
        if (_containsScriptText(scriptObject,"simMoveToPosition"))
            retVal=false;
        if (_containsScriptText(scriptObject,"sim.moveToPosition"))
            retVal=false;
        if (_containsScriptText(scriptObject,"simMoveToObject"))
            retVal=false;
        if (_containsScriptText(scriptObject,"sim.moveToObject"))
            retVal=false;
        if (_containsScriptText(scriptObject,"simFollowPath"))
            retVal=false;
        if (_containsScriptText(scriptObject,"sim.followPath"))
            retVal=false;
        if (_containsScriptText(scriptObject,"simMoveToJointPositions"))
            retVal=false;
        if (_containsScriptText(scriptObject,"sim.moveToJointPositions"))
            retVal=false;
        if (retVal)
        {
            _replaceScriptText(scriptObject,"sysCall_threadmain","coroutineMain");
            std::string txt="function sysCall_init()\n\
    corout=coroutine.create(coroutineMain)\n\
end\n\
\n\
function sysCall_actuation()\n\
    if coroutine.status(corout)~='dead' then\n\
        local ok,errorMsg=coroutine.resume(corout)\n\
        if errorMsg then\n\
            error(debug.traceback(corout,errorMsg),2)\n\
        end\n";
             if (!_executeJustOnce)
                txt+="    else\n        corout=coroutine.create(coroutineMain)\n";
             txt+="    end\nend\n\n";
            _insertScriptText(scriptObject,true,txt.c_str());
        }
    }
    return retVal;
}
