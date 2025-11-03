#include <luaScriptFunctions.h>
#include <simInternal.h>
#include <tt.h>
#include <linMotionRoutines.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <vVarious.h>
#include <vDateTime.h>
#include <app.h>
#include <apiErrors.h>
#include <interfaceStack.h>
#include <fileOperations.h>
#include <utils.h>
#include <imgLoaderSaver.h>
#include <sceneObjectOperations.h>
#include <collisionRoutines.h>
#include <distanceRoutines.h>
#include <cbor.h>
#include <meshRoutines.h>
#include <vFileFinder.h>
#include <simFlavor.h>
#ifdef SIM_WITH_GUI
#include <QScreen>
#ifdef USES_QGLWIDGET
#include <QDesktopWidget>
#else
#include <QPainter>
#endif
#include <guiApp.h>
#endif

#define LUA_START(funcName)                                     \
    CApiErrors::getAndClearLastError();                \
    std::string functionName(funcName);                         \
    std::string errorString;                                    \
    std::string warningString;                                  \
    bool cSideErrorOrWarningReporting = true;

#define LUA_START_NO_CSIDE_ERROR(funcName)                      \
    CApiErrors::getAndClearLastError();                \
    std::string functionName(funcName);                         \
    std::string errorString;                                    \
    std::string warningString;                                  \
    bool cSideErrorOrWarningReporting = false;

#define LUA_END(p)                                                                                             \
    do                                                                                                         \
    {                                                                                                          \
        _reportWarningsIfNeeded(L, functionName.c_str(), warningString.c_str()); \
        CApiErrors::getAndClearLastError();                                                           \
        return (p);                                                                                            \
    } while (0)

typedef int (*_ccallback_t)(int);

void _reportWarningsIfNeeded(luaWrap_lua_State* L, const char* functionName, const char* warningString)
{
    if (strlen(warningString) > 0)
    {
        CScriptObject* it =
            App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
        if (it != nullptr)
        {
            int verb = sim_verbosity_scriptwarnings;
            int lineNumber = -1;
            lineNumber = luaWrap_getCurrentCodeLine(L);
            std::string msg;
            msg += std::to_string(lineNumber);
            msg += ": in '";
            msg += functionName;
            msg += "' ";
            msg += warningString;
            App::logScriptMsg(it, verb, msg.c_str());
        }
    }
}

void _raiseErrorIfNeeded(luaWrap_lua_State* L, const char* functionName, const char* errorString, bool cSideErrorOrWarningReporting)
{
    std::string errStr(errorString);
    if ((errStr.size() == 0) && cSideErrorOrWarningReporting)
        errStr = CApiErrors::getAndClearLastError();
    if (errStr.size() == 0)
        return;

    CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
    if (it == nullptr)
        return;
    int lineNumber = -1;
    lineNumber = luaWrap_getCurrentCodeLine(L);
    std::string msg;
    msg += std::to_string(lineNumber);
    msg += ": in ";
    msg += functionName;
    msg += ": ";
    msg += errStr;
    luaWrap_lua_pushtext(L, msg.c_str());

    luaWrap_lua_error(L); // does a long jump and never returns
}

#define LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED() \
    _raiseErrorIfNeeded(L, functionName.c_str(), errorString.c_str(), cSideErrorOrWarningReporting)

const SLuaCommands simLuaCommands[] = {
    {"ccallback0", _ccallback0},
    {"ccallback1", _ccallback1},
    {"ccallback2", _ccallback2},
    {"ccallback3", _ccallback3},
    {"loadPlugin", _loadPlugin},
    {"unloadPlugin", _unloadPlugin},
    {"registerCodeEditorInfos", _registerCodeEditorInfos},
    {"auxFunc", _auxFunc},
    {"setAutoYield", _setAutoYield},
    {"getAutoYield", _getAutoYield},
    {"getYieldAllowed", _getYieldAllowed},
    {"setYieldAllowed", _setYieldAllowed},
    {"registerScriptFuncHook", _registerScriptFuncHook},
    {"addLog", _addLog},
    {"quitSimulator", _quitSimulator},

    {"sim.handleExtCalls", _simHandleExtCalls},
    {"sim.getLastInfo", _simGetLastInfo},
    {"sim.isHandle", _simIsHandle},
    {"sim.handleDynamics", _simHandleDynamics},
    {"sim.handleProximitySensor", _simHandleProximitySensor},
    {"sim.readProximitySensor", _simReadProximitySensor},
    {"sim.resetProximitySensor", _simResetProximitySensor},
    {"sim.checkProximitySensor", _simCheckProximitySensor},
    {"sim.getObject", _simGetObject},
    {"sim.getObjectUid", _simGetObjectUid},
    {"sim.getObjectFromUid", _simGetObjectFromUid},
    {"sim.getScript", _simGetScript},
    {"sim.getObjectPosition", _simGetObjectPosition},
    {"sim.setObjectPosition", _simSetObjectPosition},
    {"sim.getJointPosition", _simGetJointPosition},
    {"sim.setJointPosition", _simSetJointPosition},
    {"sim.setJointTargetPosition", _simSetJointTargetPosition},
    {"sim.getJointTargetPosition", _simGetJointTargetPosition},
    {"sim.setJointTargetVelocity", _simSetJointTargetVelocity},
    {"sim.getJointTargetVelocity", _simGetJointTargetVelocity},
    {"sim.removeObjects", _simRemoveObjects},
    {"sim.removeModel", _simRemoveModel},
    {"sim.getSimulationTime", _simGetSimulationTime},
    {"sim.getSimulationState", _simGetSimulationState},
    {"sim.getSystemTime", _simGetSystemTime},
    {"sim.checkCollision", _simCheckCollision},
    {"sim.checkDistance", _simCheckDistance},
    {"sim.getSimulationTimeStep", _simGetSimulationTimeStep},
    {"sim.getSimulatorMessage", _simGetSimulatorMessage},
    {"sim.resetGraph", _simResetGraph},
    {"sim.handleGraph", _simHandleGraph},
    {"sim.getGraphCurve", _simGetGraphCurve},
    {"sim.getGraphInfo", _simGetGraphInfo},
    {"sim.addGraphStream", _simAddGraphStream},
    {"sim.destroyGraphCurve", _simDestroyGraphCurve},
    {"sim.setGraphStreamTransformation", _simSetGraphStreamTransformation},
    {"sim.duplicateGraphCurveToStatic", _simDuplicateGraphCurveToStatic},
    {"sim.addGraphCurve", _simAddGraphCurve},
    {"sim.setGraphStreamValue", _simSetGraphStreamValue},
    {"sim.refreshDialogs", _simRefreshDialogs},
    {"sim.stopSimulation", _simStopSimulation},
    {"sim.pauseSimulation", _simPauseSimulation},
    {"sim.startSimulation", _simStartSimulation},
    {"sim.getObjectPose", _simGetObjectPose},
    {"sim.setObjectPose", _simSetObjectPose},
    {"sim.getObjectChildPose", _simGetObjectChildPose},
    {"sim.setObjectChildPose", _simSetObjectChildPose},
    {"sim.getObjectChild", _simGetObjectChild},
    {"sim.getObjectParent", _simGetObjectParent},
    {"sim.setObjectParent", _simSetObjectParent},
    {"sim.getObjectType", _simGetObjectType},
    {"sim.getJointType", _simGetJointType},
    {"sim.getObjectAlias", _simGetObjectAlias},
    {"sim.setObjectAlias", _simSetObjectAlias},
    {"sim.getJointInterval", _simGetJointInterval},
    {"sim.setJointInterval", _simSetJointInterval},
    {"sim.loadScene", _simLoadScene},
    {"sim.closeScene", _simCloseScene},
    {"sim.saveScene", _simSaveScene},
    {"sim.loadModel", _simLoadModel},
    {"sim.saveModel", _simSaveModel},
    {"sim.getObjectSel", _simGetObjectSel},
    {"sim.setObjectSel", _simSetObjectSel},
    {"sim.setNavigationMode", _simSetNavigationMode},
    {"sim.getNavigationMode", _simGetNavigationMode},
    {"sim.setPage", _simSetPage},
    {"sim.getPage", _simGetPage},
    {"sim.copyPasteObjects", _simCopyPasteObjects},
    {"sim.scaleObjects", _simScaleObjects},
    {"sim.setAutoYieldDelay", _simSetAutoYieldDelay},
    {"sim.getAutoYieldDelay", _simGetAutoYieldDelay},
    {"sim.saveImage", _simSaveImage},
    {"sim.loadImage", _simLoadImage},
    {"sim.getScaledImage", _simGetScaledImage},
    {"sim.transformImage", _simTransformImage},
    {"sim.packInt32Table", _simPackInt32Table},
    {"sim.packUInt32Table", _simPackUInt32Table},
    {"sim.packFloatTable", _simPackFloatTable},
    {"sim.packDoubleTable", _simPackDoubleTable},
    {"sim.packUInt8Table", _simPackUInt8Table},
    {"sim.packUInt16Table", _simPackUInt16Table},
    {"sim.unpackInt32Table", _simUnpackInt32Table},
    {"sim.unpackUInt32Table", _simUnpackUInt32Table},
    {"sim.unpackFloatTable", _simUnpackFloatTable},
    {"sim.unpackDoubleTable", _simUnpackDoubleTable},
    {"sim.unpackUInt8Table", _simUnpackUInt8Table},
    {"sim.unpackUInt16Table", _simUnpackUInt16Table},
    {"sim.packTable", _simPackTable},
    {"sim.unpackTable", _simUnpackTable},
    {"sim.transformBuffer", _simTransformBuffer},
    {"sim.combineRgbImages", _simCombineRgbImages},
    {"sim.getVelocity", _simGetVelocity},
    {"sim.getObjectVelocity", _simGetObjectVelocity},
    {"sim.getJointVelocity", _simGetJointVelocity},
    {"sim.addForceAndTorque", _simAddForceAndTorque},
    {"sim.addForce", _simAddForce},
    {"sim.setExplicitHandling", _simSetExplicitHandling},
    {"sim.getExplicitHandling", _simGetExplicitHandling},
    {"sim.addDrawingObject", _simAddDrawingObject},
    {"sim.removeDrawingObject", _simRemoveDrawingObject},
    {"sim.addDrawingObjectItem", _simAddDrawingObjectItem},
    {"sim.addParticleObject", _simAddParticleObject},
    {"sim.removeParticleObject", _simRemoveParticleObject},
    {"sim.addParticleObjectItem", _simAddParticleObjectItem},
    {"sim.getObjectSizeFactor", _simGetObjectSizeFactor},
    {"sim.readForceSensor", _simReadForceSensor},
    {"sim.getLinkDummy", _simGetLinkDummy},
    {"sim.setLinkDummy", _simSetLinkDummy},
    {"sim.setObjectColor", _simSetObjectColor},
    {"sim.getObjectColor", _simGetObjectColor},
    {"sim.setShapeColor", _simSetShapeColor},
    {"sim.getShapeColor", _simGetShapeColor},
    {"sim.resetDynamicObject", _simResetDynamicObject},
    {"sim.setJointMode", _simSetJointMode},
    {"sim.getJointMode", _simGetJointMode},
    {"sim._serialOpen", _simSerialOpen},
    {"sim._serialClose", _simSerialClose},
    {"sim.serialSend", _simSerialSend},
    {"sim._serialRead", _simSerialRead},
    {"sim.serialCheck", _simSerialCheck},
    {"sim.getContactInfo", _simGetContactInfo},
    {"sim.auxiliaryConsoleOpen", _simAuxiliaryConsoleOpen},
    {"sim.auxiliaryConsoleClose", _simAuxiliaryConsoleClose},
    {"sim.auxiliaryConsolePrint", _simAuxiliaryConsolePrint},
    {"sim.auxiliaryConsoleShow", _simAuxiliaryConsoleShow},
    {"sim.importShape", _simImportShape},
    {"sim.importMesh", _simImportMesh},
    {"sim.exportMesh", _simExportMesh},
    {"sim.createShape", _simCreateShape},
    {"sim.getShapeMesh", _simGetShapeMesh},
    {"sim.getShapeBB", _simGetShapeBB},
    {"sim.createPrimitiveShape", _simCreatePrimitiveShape},
    {"sim.createHeightfieldShape", _simCreateHeightfieldShape},
    {"sim.createJoint", _simCreateJoint},
    {"sim.createDummy", _simCreateDummy},
    {"sim.createScript", _simCreateScript},
    {"sim.createProximitySensor", _simCreateProximitySensor},
    {"sim.createForceSensor", _simCreateForceSensor},
    {"sim.createVisionSensor", _simCreateVisionSensor},
    {"sim.floatingViewAdd", _simFloatingViewAdd},
    {"sim.floatingViewRemove", _simFloatingViewRemove},
    {"sim.adjustView", _simAdjustView},
    {"sim.cameraFitToView", _simCameraFitToView},
    {"sim.announceSceneContentChange", _simAnnounceSceneContentChange},
    {"sim.launchExecutable", _simLaunchExecutable},
    {"sim.getJointForce", _simGetJointForce},
    {"sim.getJointTargetForce", _simGetJointTargetForce},
    {"sim.setJointTargetForce", _simSetJointTargetForce},
    {"sim.getObjectQuaternion", _simGetObjectQuaternion},
    {"sim.setObjectQuaternion", _simSetObjectQuaternion},
    {"sim.groupShapes", _simGroupShapes},
    {"sim.ungroupShape", _simUngroupShape},
    {"sim.getThreadId", _simGetThreadId},
    {"sim.setShapeMaterial", _simSetShapeMaterial},
    {"sim.getTextureId", _simGetTextureId},
    {"sim.readTexture", _simReadTexture},
    {"sim.writeTexture", _simWriteTexture},
    {"sim.createTexture", _simCreateTexture},
    {"sim.getShapeGeomInfo", _simGetShapeGeomInfo},
    {"sim.getObjectsInTree", _simGetObjectsInTree},
    {"sim.getObjects", _simGetObjects},
    {"sim.scaleObject", _simScaleObject},
    {"sim.setShapeTexture", _simSetShapeTexture},
    {"sim.getShapeTextureId", _simGetShapeTextureId},
    {"sim.destroyCollection", _simDestroyCollection},
    {"sim.addItemToCollection", _simAddItemToCollection},
    {"sim.getCollectionObjects", _simGetCollectionObjects},
    {"sim.handleAddOnScripts", _simHandleAddOnScripts},
    {"sim.handleSandboxScript", _simHandleSandboxScript},
    {"sim.handleSimulationScripts", _simHandleSimulationScripts},
    {"sim.handleEmbeddedScripts", _simHandleEmbeddedScripts},
    {"sim.alignShapeBB", _simAlignShapeBB},
    {"sim.relocateShapeFrame", _simRelocateShapeFrame},
    {"sim.handleVisionSensor", _simHandleVisionSensor},
    {"sim.readVisionSensor", _simReadVisionSensor},
    {"sim.resetVisionSensor", _simResetVisionSensor},
    {"sim.getVisionSensorImg", _simGetVisionSensorImg},
    {"sim.setVisionSensorImg", _simSetVisionSensorImg},
    {"sim.getVisionSensorDepth", _simGetVisionSensorDepth},
    {"sim.checkVisionSensor", _simCheckVisionSensor},
    {"sim.checkVisionSensorEx", _simCheckVisionSensorEx},
    {"sim.ruckigPos", _simRuckigPos},
    {"sim.ruckigVel", _simRuckigVel},
    {"sim.ruckigStep", _simRuckigStep},
    {"sim.ruckigRemove", _simRuckigRemove},
    {"sim.getQuaternionFromMatrix", _simGetQuaternionFromMatrix},
    {"sim.callScriptFunction", _simCallScriptFunction},
    {"sim.getExtensionString", _simGetExtensionString},
    {"sim.computeMassAndInertia", _simComputeMassAndInertia},
    {"sim.createOctree", _simCreateOctree},
    {"sim.createPointCloud", _simCreatePointCloud},
    {"sim.setPointCloudOptions", _simSetPointCloudOptions},
    {"sim.getPointCloudOptions", _simGetPointCloudOptions},
    {"sim.insertVoxelsIntoOctree", _simInsertVoxelsIntoOctree},
    {"sim.removeVoxelsFromOctree", _simRemoveVoxelsFromOctree},
    {"sim.insertPointsIntoPointCloud", _simInsertPointsIntoPointCloud},
    {"sim.removePointsFromPointCloud", _simRemovePointsFromPointCloud},
    {"sim.intersectPointsWithPointCloud", _simIntersectPointsWithPointCloud},
    {"sim.getOctreeVoxels", _simGetOctreeVoxels},
    {"sim.getPointCloudPoints", _simGetPointCloudPoints},
    {"sim.insertObjectIntoOctree", _simInsertObjectIntoOctree},
    {"sim.subtractObjectFromOctree", _simSubtractObjectFromOctree},
    {"sim.insertObjectIntoPointCloud", _simInsertObjectIntoPointCloud},
    {"sim.subtractObjectFromPointCloud", _simSubtractObjectFromPointCloud},
    {"sim.checkOctreePointOccupancy", _simCheckOctreePointOccupancy},
    {"sim.handleSimulationStart", _simHandleSimulationStart},
    {"sim.handleSensingStart", _simHandleSensingStart},
    {"sim.auxFunc", _simAuxFunc},
    {"sim.setReferencedHandles", _simSetReferencedHandles},
    {"sim.getReferencedHandles", _simGetReferencedHandles},
    {"sim.getReferencedHandlesTags", _simGetReferencedHandlesTags},
    {"sim.getShapeViz", _simGetShapeViz},
    {"sim.executeScriptString", _simExecuteScriptString},
    {"sim.getApiFunc", _simGetApiFunc},
    {"sim.getApiInfo", _simGetApiInfo},
    {"sim.getPluginName", _simGetPluginName},
    {"sim.getPluginInfo", _simGetPluginInfo},
    {"sim.setPluginInfo", _simSetPluginInfo},
    {"sim.textEditorOpen", _simTextEditorOpen},
    {"sim.textEditorClose", _simTextEditorClose},
    {"sim.textEditorShow", _simTextEditorShow},
    {"sim.textEditorGetInfo", _simTextEditorGetInfo},
    {"sim.setJointDependency", _simSetJointDependency},
    {"sim.getJointDependency", _simGetJointDependency},
    {"sim.getStackTraceback", _simGetStackTraceback},
    {"sim.getShapeMass", _simGetShapeMass},
    {"sim.setShapeMass", _simSetShapeMass},
    {"sim.getShapeInertia", _simGetShapeInertia},
    {"sim.setShapeInertia", _simSetShapeInertia},
    {"sim.isDynamicallyEnabled", _simIsDynamicallyEnabled},
    {"sim.generateShapeFromPath", _simGenerateShapeFromPath},
    {"sim.getClosestPosOnPath", _simGetClosestPosOnPath},
    {"sim.initScript", _simInitScript},
    {"sim.moduleEntry", _simModuleEntry},
    {"sim.pushUserEvent", _simPushUserEvent},
    {"sim.createCollectionEx", _simCreateCollectionEx},
    {"sim.getGenesisEvents", _simGetGenesisEvents},
    {"sim.broadcastMsg", _simBroadcastMsg},
    {"sim.handleJointMotion", _simHandleJointMotion},
    {"sim.getVisionSensorRes", _simGetVisionSensorRes},
    {"sim.getObjectHierarchyOrder", _simGetObjectHierarchyOrder},
    {"sim.setObjectHierarchyOrder", _simSetObjectHierarchyOrder},
    {"sim._qhull", _sim_qhull},
    {"sim.systemSemaphore", _simSystemSemaphore},
    {"sim.setBoolProperty", _simSetBoolProperty},
    {"sim.getBoolProperty", _simGetBoolProperty},
    {"sim.setIntProperty", _simSetIntProperty},
    {"sim.getIntProperty", _simGetIntProperty},
    {"sim.setLongProperty", _simSetLongProperty},
    {"sim.getLongProperty", _simGetLongProperty},
    {"sim.setHandleProperty", _simSetHandleProperty},
    {"sim.getHandleProperty", _simGetHandleProperty},
    {"sim.setFloatProperty", _simSetFloatProperty},
    {"sim.getFloatProperty", _simGetFloatProperty},
    {"sim.setStringProperty", _simSetStringProperty},
    {"sim.getStringProperty", _simGetStringProperty},
    {"sim.setTableProperty", _simSetTableProperty},
    {"sim.getTableProperty", _simGetTableProperty},
    {"sim.setBufferProperty", _simSetBufferProperty},
    {"sim.getBufferProperty", _simGetBufferProperty},
    {"sim.setIntArray2Property", _simSetIntArray2Property},
    {"sim.getIntArray2Property", _simGetIntArray2Property},
    {"sim.setVector2Property", _simSetVector2Property},
    {"sim.getVector2Property", _simGetVector2Property},
    {"sim.setVector3Property", _simSetVector3Property},
    {"sim.getVector3Property", _simGetVector3Property},
    {"sim.setQuaternionProperty", _simSetQuaternionProperty},
    {"sim.getQuaternionProperty", _simGetQuaternionProperty},
    {"sim.setPoseProperty", _simSetPoseProperty},
    {"sim.getPoseProperty", _simGetPoseProperty},
    {"sim.setColorProperty", _simSetColorProperty},
    {"sim.getColorProperty", _simGetColorProperty},
    {"sim.setFloatArrayProperty", _simSetFloatArrayProperty},
    {"sim.getFloatArrayProperty", _simGetFloatArrayProperty},
    {"sim.setIntArrayProperty", _simSetIntArrayProperty},
    {"sim.getIntArrayProperty", _simGetIntArrayProperty},
    {"sim.setHandleArrayProperty", _simSetHandleArrayProperty},
    {"sim.getHandleArrayProperty", _simGetHandleArrayProperty},
    {"sim.removeProperty", _simRemoveProperty},
    {"sim.getPropertyName", _simGetPropertyName},
    {"sim.getPropertyInfo", _simGetPropertyInfo},
    {"sim.setEventFilters", _simSetEventFilters},

    {"sim.test", _simTest},

    // deprecated
    {"sim1.buildIdentityMatrix", _simBuildIdentityMatrix},
    {"sim1.buildMatrix", _simBuildMatrix},
    {"sim1.buildPose", _simBuildPose},
    {"sim1.getEulerAnglesFromMatrix", _simGetEulerAnglesFromMatrix},
    {"sim1.getMatrixInverse", _simGetMatrixInverse},
    {"sim1.getPoseInverse", _simGetPoseInverse},
    {"sim1.multiplyMatrices", _simMultiplyMatrices},
    {"sim1.multiplyPoses", _simMultiplyPoses},
    {"sim1.interpolateMatrices", _simInterpolateMatrices},
    {"sim1.interpolatePoses", _simInterpolatePoses},
    {"sim1.poseToMatrix", _simPoseToMatrix},
    {"sim1.matrixToPose", _simMatrixToPose},
    {"sim1.multiplyVector", _simMultiplyVector},
    {"sim1.getRotationAxis", _simGetRotationAxis},
    {"sim1.rotateAroundAxis", _simRotateAroundAxis},
    {"sim1.getObjectOrientation", _simGetObjectOrientation},
    {"sim1.setObjectOrientation", _simSetObjectOrientation},
    {"sim1.getObjectMatrix", _simGetObjectMatrix},
    {"sim1.setObjectMatrix", _simSetObjectMatrix},
    {"sim1.checkProximitySensorEx", _simCheckProximitySensorEx},
    {"sim1.checkProximitySensorEx2", _simCheckProximitySensorEx2},
    {"sim1.checkCollisionEx", _simCheckCollisionEx},
    {"sim1.buildMatrixQ", _simBuildMatrixQ},
    {"sim1.getRealTimeSimulation", _simGetRealTimeSimulation},
    {"sim1.setObjectProperty", _simSetObjectProperty},
    {"sim1.getObjectProperty", _simGetObjectProperty},
    {"sim1.setObjectSpecialProperty", _simSetObjectSpecialProperty},
    {"sim1.getObjectSpecialProperty", _simGetObjectSpecialProperty},
    {"sim1.setModelProperty", _simSetModelProperty},
    {"sim1.getModelProperty", _simGetModelProperty},
    {"sim1.persistentDataWrite", _simPersistentDataWrite},
    {"sim1.persistentDataRead", _simPersistentDataRead},
    {"sim1.getPersistentDataTags", _simGetPersistentDataTags},
    {"sim1.addScript", _simAddScript},
    {"sim1.associateScriptWithObject", _simAssociateScriptWithObject},
    {"sim1.removeScript", _simRemoveScript},
    {"sim1.getScriptInt32Param", _simGetScriptInt32Param},
    {"sim1.setScriptInt32Param", _simSetScriptInt32Param},
    {"sim1.getScriptStringParam", _simGetScriptStringParam},
    {"sim1.setScriptStringParam", _simSetScriptStringParam},
    {"sim1.getDecimatedMesh", _simGetDecimatedMesh},
    {"sim1.convexDecompose", _simConvexDecompose},
    {"sim1.loadModule", _simLoadModule},
    {"sim1.unloadModule", _simUnloadModule},
    {"sim1.isDeprecated", _simIsDeprecated},
    {"sim1.registerScriptFunction", _simRegisterScriptFunction},
    {"sim1.registerScriptVariable", _simRegisterScriptVariable},
    {"sim1.getObjectSelection", _simGetObjectSel},
    {"sim1.setObjectSelection", _simSetObjectSel},
    {"sim1.rmlPos", _simRuckigPos},
    {"sim1.rmlVel", _simRuckigVel},
    {"sim1.rmlStep", _simRuckigStep},
    {"sim1.rmlRemove", _simRuckigRemove},
    {"sim1.addStatusbarMessage", _simAddStatusbarMessage},
    {"sim1.getNameSuffix", _simGetNameSuffix},
    {"sim1.setNameSuffix", _simSetNameSuffix},
    {"sim1.resetMill", _simResetMill},
    {"sim1.resetMilling", _simResetMilling},
    {"sim1.handleMill", _simHandleMill},
    {"sim1.openTextEditor", _simOpenTextEditor},
    {"sim1.closeTextEditor", _simCloseTextEditor},
    {"sim1.getScriptSimulationParameter", _simGetScriptSimulationParameter},
    {"sim1.setScriptSimulationParameter", _simSetScriptSimulationParameter},
    {"sim1.setJointForce", _simSetJointMaxForce},
    {"sim1.handleMechanism", _simHandleMechanism},
    {"sim1.setPathTargetNominalVelocity", _simSetPathTargetNominalVelocity},
    {"sim1.getShapeMassAndInertia", _simGetShapeMassAndInertia},
    {"sim1.setShapeMassAndInertia", _simSetShapeMassAndInertia},
    {"sim1.checkIkGroup", _simCheckIkGroup},
    {"sim1.createIkGroup", _simCreateIkGroup},
    {"sim1.removeIkGroup", _simRemoveIkGroup},
    {"sim1.createIkElement", _simCreateIkElement},
    {"sim1.exportIk", _simExportIk},
    {"sim1.computeJacobian", _simComputeJacobian},
    {"sim1.getConfigForTipPose", _simGetConfigForTipPose},
    {"sim1.generateIkPath", _simGenerateIkPath},
    {"sim1.getIkGroupHandle", _simGetIkGroupHandle},
    {"sim1.getIkGroupMatrix", _simGetIkGroupMatrix},
    {"sim1.handleIkGroup", _simHandleIkGroup},
    {"sim1.setIkGroupProperties", _simSetIkGroupProperties},
    {"sim1.setIkElementProperties", _simSetIkElementProperties},
    {"sim1.setThreadIsFree", _simSetThreadIsFree},
    {"sim1._tubeRead", _simTubeRead},
    {"sim1.tubeOpen", _simTubeOpen},
    {"sim1.tubeClose", _simTubeClose},
    {"sim1.tubeWrite", _simTubeWrite},
    {"sim1.tubeStatus", _simTubeStatus},
    {"sim1.sendData", _simSendData},
    {"sim1.receiveData", _simReceiveData},
    {"sim1._moveToJointPos_1", _sim_moveToJointPos_1},
    {"sim1._moveToJointPos_2", _sim_moveToJointPos_2},
    {"sim1._moveToPos_1", _sim_moveToPos_1},
    {"sim1._moveToPos_2", _sim_moveToPos_2},
    {"sim1._moveToObj_1", _sim_moveToObj_1},
    {"sim1._moveToObj_2", _sim_moveToObj_2},
    {"sim1._followPath_1", _sim_followPath_1},
    {"sim1._followPath_2", _sim_followPath_2},
    {"sim1._del", _sim_del},
    {"sim1.getPathPosition", _simGetPathPosition},
    {"sim1.setPathPosition", _simSetPathPosition},
    {"sim1.getPathLength", _simGetPathLength},
    {"sim1.getDataOnPath", _simGetDataOnPath},
    {"sim1.getPositionOnPath", _simGetPositionOnPath},
    {"sim1.getOrientationOnPath", _simGetOrientationOnPath},
    {"sim1.getClosestPositionOnPath", _simGetClosestPositionOnPath},
    {"sim1._createPath", _sim_CreatePath},
    {"sim1.insertPathCtrlPoints", _simInsertPathCtrlPoints},
    {"sim1.cutPathCtrlPoints", _simCutPathCtrlPoints},
    {"sim1.getScriptExecutionCount", _simGetScriptExecutionCount},
    {"sim1.isScriptExecutionThreaded", _simIsScriptExecutionThreaded},
    {"sim1.isScriptRunningInThread", _simIsScriptRunningInThread},
    {"sim1.setThreadResumeLocation", _simSetThreadResumeLocation},
    {"sim1.resumeThreads", _simResumeThreads},
    {"sim1.launchThreadedChildScripts", _simLaunchThreadedChildScripts},
    {"sim1.getUserParameter", _simGetUserParameter},
    {"sim1.setUserParameter", _simSetUserParameter},
    {"sim1.setVisionSensorFilter", _simSetVisionSensorFilter},
    {"sim1.getVisionSensorFilter", _simGetVisionSensorFilter},
    {"sim1.getCollectionHandle", _simGetCollectionHandle},
    {"sim1.removeCollection", _simRemoveCollection},
    {"sim1.emptyCollection", _simEmptyCollection},
    {"sim1.getCollectionName", _simGetCollectionName},
    {"sim1.setCollectionName", _simSetCollectionName},
    {"sim1._createCollection", _sim_CreateCollection},
    {"sim1.addObjectToCollection", _simAddObjectToCollection},
    {"sim1.getCollisionHandle", _simGetCollisionHandle},
    {"sim1.getDistanceHandle", _simGetDistanceHandle},
    {"sim1.handleCollision", _simHandleCollision},
    {"sim1.readCollision", _simReadCollision},
    {"sim1.handleDistance", _simHandleDistance},
    {"sim1.readDistance", _simReadDistance},
    {"sim1.resetCollision", _simResetCollision},
    {"sim1.resetDistance", _simResetDistance},
    {"sim1.addBanner", _simAddBanner},
    {"sim1.removeBanner", _simRemoveBanner},
    {"sim1.addGhost", _simAddGhost},
    {"sim1.modifyGhost", _simModifyGhost},
    {"sim1.addPointCloud", _simAddPointCloud},
    {"sim1.modifyPointCloud", _simModifyPointCloud},
    {"sim1.setGraphUserData", _simSetGraphUserData},
    {"sim1.copyMatrix", _simCopyMatrix},
    {"sim1.getObjectInt32Parameter", _simGetObjectInt32Parameter},
    {"sim1.setObjectInt32Parameter", _simSetObjectInt32Param},
    {"sim1.getObjectFloatParameter", _simGetObjectFloatParameter},
    {"sim1.setObjectFloatParameter", _simSetObjectFloatParam},
    {"sim1.getObjectStringParameter", _simGetObjectStringParam},
    {"sim1.setObjectStringParameter", _simSetObjectStringParam},
    {"sim1.setBoolParameter", _simSetBoolParam},
    {"sim1.getBoolParameter", _simGetBoolParam},
    {"sim1.setInt32Parameter", _simSetInt32Param},
    {"sim1.getInt32Parameter", _simGetInt32Param},
    {"sim1.setFloatParameter", _simSetFloatParam},
    {"sim1.getFloatParameter", _simGetFloatParam},
    {"sim1.setStringParameter", _simSetStringParam},
    {"sim1.getStringParameter", _simGetStringParam},
    {"sim1.setArrayParameter", _simSetArrayParam},
    {"sim1.getArrayParameter", _simGetArrayParam},
    {"sim1.getEngineFloatParameter", _simGetEngineFloatParam},
    {"sim1.getEngineInt32Parameter", _simGetEngineInt32Param},
    {"sim1.getEngineBoolParameter", _simGetEngineBoolParam},
    {"sim1.setEngineFloatParameter", _simSetEngineFloatParam},
    {"sim1.setEngineInt32Parameter", _simSetEngineInt32Param},
    {"sim1.setEngineBoolParameter", _simSetEngineBoolParam},
    {"sim1.isHandleValid", _simIsHandleValid},
    {"sim1.setIntegerSignal", _simSetInt32Signal},
    {"sim1.getIntegerSignal", _simGetInt32Signal},
    {"sim1.clearIntegerSignal", _simClearInt32Signal},
    {"sim1.getObjectName", _simGetObjectName},
    {"sim1.setObjectName", _simSetObjectName},
    {"sim1.getScriptName", _simGetScriptName},
    {"sim1.setScriptVariable", _simSetScriptVariable},
    {"sim1.getObjectAssociatedWithScript", _simGetObjectAssociatedWithScript},
    {"sim1.getScriptAssociatedWithObject", _simGetScriptAssociatedWithObject},
    {"sim1.getCustomizationScriptAssociatedWithObject", _simGetCustomizationScriptAssociatedWithObject},
    {"sim1.getObjectConfiguration", _simGetObjectConfiguration},
    {"sim1.setObjectConfiguration", _simSetObjectConfiguration},
    {"sim1.getConfigurationTree", _simGetConfigurationTree},
    {"sim1.setConfigurationTree", _simSetConfigurationTree},
    {"sim1.setObjectSizeValues", _simSetObjectSizeValues},
    {"sim1.getObjectSizeValues", _simGetObjectSizeValues},
    {"sim1.openModule", _simOpenModule},
    {"sim1.closeModule", _simCloseModule},
    {"sim1.handleModule", _simHandleModule},
    {"sim1.getLastError", _simGetLastError},
    {"sim1._switchThread", _simSwitchThread},
    {"sim1.getSystemTimeInMs", _simGetSystemTimeInMs},
    {"sim1.fileDialog", _simFileDialog},
    {"sim1.msgBox", _simMsgBox},
    {"sim1.isObjectInSelection", _simIsObjectInSelection},
    {"sim1.addObjectToSelection", _simAddObjectToSelection},
    {"sim1.removeObjectFromSelection", _simRemoveObjectFromSelection},
    {"sim1.getObjectUniqueIdentifier", _simGetObjectUniqueIdentifier},
    {"sim1.breakForceSensor", _simBreakForceSensor},
    {"sim1.getJointMatrix", _simGetJointMatrix},
    {"sim1.setSphericalJointMatrix", _simSetSphericalJointMatrix},
    {"sim1.setScriptAttribute", _simSetScriptAttribute},
    {"sim1.getScriptAttribute", _simGetScriptAttribute},
    {"sim1.setScriptText", _simSetScriptText},
    {"sim1.getJointMaxForce", _simGetJointMaxForce},
    {"sim1.setJointMaxForce", _simSetJointMaxForce},
    {"sim1._getObjectHandle", _sim_getObjectHandle},
    {"sim1.removeObject", _simRemoveObject},
    {"sim1.getVisionSensorResolution", _simGetVisionSensorResolution},
    {"sim1.getVisionSensorImage", _simGetVisionSensorImage},
    {"sim1.setVisionSensorImage", _simSetVisionSensorImage},
    {"sim1.getVisionSensorCharImage", _simGetVisionSensorCharImage},
    {"sim1.setVisionSensorCharImage", _simSetVisionSensorCharImage},
    {"sim1.getVisionSensorDepthBuffer", _simGetVisionSensorDepthBuffer},
    {"sim1.createPureShape", _simCreatePureShape},
    {"sim1.getScriptHandle", _simGetScriptHandle},
    {"sim1.handleCustomizationScripts", _simHandleCustomizationScripts},
    {"sim1.setDoubleSignal", _simSetDoubleSignal},
    {"sim1.getDoubleSignal", _simGetDoubleSignal},
    {"sim1.clearDoubleSignal", _simClearDoubleSignal},
    {"sim1.reorientShapeBoundingBox", _simReorientShapeBoundingBox},
    {"sim1.createMeshShape", _simCreateMeshShape},
    {"sim1.invertMatrix", _simInvertMatrix},
    {"sim1.invertPose", _simInvertPose},
    {"sim1.setBoolParam", _simSetBoolParam},
    {"sim1.getBoolParam", _simGetBoolParam},
    {"sim1.setInt32Param", _simSetInt32Param},
    {"sim1.getInt32Param", _simGetInt32Param},
    {"sim1.setFloatParam", _simSetFloatParam},
    {"sim1.getFloatParam", _simGetFloatParam},
    {"sim1.setStringParam", _simSetStringParam},
    {"sim1.getStringParam", _simGetStringParam},
    {"sim1.setArrayParam", _simSetArrayParam},
    {"sim1.getArrayParam", _simGetArrayParam},
    {"sim1.setNamedStringParam", _simSetNamedStringParam},
    {"sim1.getNamedStringParam", _simGetNamedStringParam},
    {"sim1.setInt32Signal", _simSetInt32Signal},
    {"sim1.getInt32Signal", _simGetInt32Signal},
    {"sim1.clearInt32Signal", _simClearInt32Signal},
    {"sim1.setFloatSignal", _simSetFloatSignal},
    {"sim1.getFloatSignal", _simGetFloatSignal},
    {"sim1.clearFloatSignal", _simClearFloatSignal},
    {"sim1.setStringSignal", _simSetStringSignal},
    {"sim1.getStringSignal", _simGetStringSignal},
    {"sim1.clearStringSignal", _simClearStringSignal},
    {"sim1.getSignalName", _simGetSignalName},
    {"sim1.getLightParameters", _simGetLightParameters},
    {"sim1.setLightParameters", _simSetLightParameters},
    {"sim1.getObjectInt32Param", _simGetObjectInt32Param},
    {"sim1.setObjectInt32Param", _simSetObjectInt32Param},
    {"sim1.getObjectFloatParam", _simGetObjectFloatParam},
    {"sim1.setObjectFloatParam", _simSetObjectFloatParam},
    {"sim1.getObjectFloatArrayParam", _simGetObjectFloatArrayParam},
    {"sim1.setObjectFloatArrayParam", _simSetObjectFloatArrayParam},
    {"sim1.getObjectStringParam", _simGetObjectStringParam},
    {"sim1.setObjectStringParam", _simSetObjectStringParam},
    {"sim1.writeCustomStringData", _simWriteCustomStringData},
    {"sim1.readCustomStringData", _simReadCustomStringData},
    {"sim1.readCustomDataTags", _simReadCustomDataTags},
    {"sim1.getEngineFloatParam", _simGetEngineFloatParam},
    {"sim1.getEngineInt32Param", _simGetEngineInt32Param},
    {"sim1.getEngineBoolParam", _simGetEngineBoolParam},
    {"sim1.setEngineFloatParam", _simSetEngineFloatParam},
    {"sim1.setEngineInt32Param", _simSetEngineInt32Param},
    {"sim1.setEngineBoolParam", _simSetEngineBoolParam},
    {"", nullptr}};

const SLuaVariables simLuaVariables[] = {
    // Scene object types (main types):
    {"sim.sceneobject_shape", sim_sceneobject_shape},
    {"sim.sceneobject_joint", sim_sceneobject_joint},
    {"sim.sceneobject_graph", sim_sceneobject_graph},
    {"sim.sceneobject_camera", sim_sceneobject_camera},
    {"sim.sceneobject_dummy", sim_sceneobject_dummy},
    {"sim.sceneobject_proximitysensor", sim_sceneobject_proximitysensor},
    {"sim.sceneobject_path", sim_sceneobject_path},
    {"sim.sceneobject_renderingsensor", sim_sceneobject_visionsensor},
    {"sim.sceneobject_visionsensor", sim_sceneobject_visionsensor},
    {"sim.sceneobject_mill", sim_sceneobject_mill},
    {"sim.sceneobject_forcesensor", sim_sceneobject_forcesensor},
    {"sim.sceneobject_light", sim_sceneobject_light},
    {"sim.sceneobject_mirror", sim_sceneobject_mirror},
    {"sim.sceneobject_octree", sim_sceneobject_octree},
    {"sim.sceneobject_pointcloud", sim_sceneobject_pointcloud},
    {"sim.sceneobject_script", sim_sceneobject_script},
    // 3D object sub-types:
    {"sim.light_omnidirectional", sim_light_omnidirectional},
    {"sim.light_spot", sim_light_spot},
    {"sim.light_directional", sim_light_directional},
    {"sim.joint_revolute", sim_joint_revolute},
    {"sim.joint_prismatic", sim_joint_prismatic},
    {"sim.joint_spherical", sim_joint_spherical},
    {"sim.shape_simple", sim_shape_simple},
    {"sim.shape_compound", sim_shape_compound},
    {"sim.proximitysensor_pyramid", sim_proximitysensor_pyramid},
    {"sim.proximitysensor_cylinder", sim_proximitysensor_cylinder},
    {"sim.proximitysensor_disc", sim_proximitysensor_disc},
    {"sim.proximitysensor_cone", sim_proximitysensor_cone},
    {"sim.proximitysensor_ray", sim_proximitysensor_ray},
    // Other object types:
    {"sim.objecttype_sceneobject", sim_objecttype_sceneobject},
    {"sim.objecttype_collection", sim_objecttype_collection},
    {"sim.objecttype_script", sim_objecttype_script},
    {"sim.objecttype_texture", sim_objecttype_texture},
    {"sim.objecttype_mesh", sim_objecttype_mesh},
    {"sim.objecttype_interfacestack", sim_objecttype_interfacestack},

    // Simulation messages:
    {"sim.message_model_loaded", sim_message_model_loaded},
    {"sim.message_scene_loaded", sim_message_scene_loaded},
    {"sim.message_object_selection_changed", sim_message_object_selection_changed},
    {"sim.message_keypress", sim_message_keypress},
    // object properties. Combine with the | operator
    {"sim.objectproperty_collapsed", sim_objectproperty_collapsed},
    {"sim.objectproperty_selectable", sim_objectproperty_selectable},
    {"sim.objectproperty_selectmodelbaseinstead", sim_objectproperty_selectmodelbaseinstead},
    {"sim.objectproperty_dontshowasinsidemodel", sim_objectproperty_dontshowasinsidemodel},
    {"sim.objectproperty_selectinvisible", sim_objectproperty_selectinvisible},
    {"sim.objectproperty_depthinvisible", sim_objectproperty_depthinvisible},
    {"sim.objectproperty_cannotdelete", sim_objectproperty_cannotdelete},
    {"sim.objectproperty_cannotdeleteduringsim", sim_objectproperty_cannotdeleteduringsim},
    {"sim.objectproperty_ignoreviewfitting", sim_objectproperty_ignoreviewfitting},
    {"sim.objectproperty_hiddenforsimulation", sim_objectproperty_hiddenforsimulation},
    // Simulation status:
    {"sim.simulation_stopped", sim_simulation_stopped},
    {"sim.simulation_paused", sim_simulation_paused},
    {"sim.simulation_advancing", sim_simulation_advancing},
    {"sim.simulation_advancing_running", sim_simulation_advancing_running},
    {"sim.simulation_advancing_lastbeforestop", sim_simulation_advancing_lastbeforestop},
    // Texture mapping modes:
    {"sim.texturemap_plane", sim_texturemap_plane},
    {"sim.texturemap_cylinder", sim_texturemap_cylinder},
    {"sim.texturemap_sphere", sim_texturemap_sphere},
    {"sim.texturemap_cube", sim_texturemap_cube},
    // script types:
    {"sim.scripttype_main", sim_scripttype_main},
    {"sim.scripttype_simulation", sim_scripttype_simulation},
    {"sim.scripttype_addon", sim_scripttype_addon},
    {"sim.scripttype_customization", sim_scripttype_customization},
    {"sim.scripttype_sandbox", sim_scripttype_sandbox},
    {"sim.scripttype_passive", sim_scripttype_passive},
    // System callbacks
    {"sim.syscb_thread", sim_syscb_thread},
    {"sim.syscb_init", sim_syscb_init},
    {"sim.syscb_cleanup", sim_syscb_cleanup},
    {"sim.syscb_nonsimulation", sim_syscb_nonsimulation},
    {"sim.syscb_beforemainscript", sim_syscb_beforemainscript},
    {"sim.syscb_beforesimulation", sim_syscb_beforesimulation},
    {"sim.syscb_aftersimulation", sim_syscb_aftersimulation},
    {"sim.syscb_actuation", sim_syscb_actuation},
    {"sim.syscb_sensing", sim_syscb_sensing},
    {"sim.syscb_suspended", sim_syscb_suspended},
    {"sim.syscb_suspend", sim_syscb_suspend},
    {"sim.syscb_resume", sim_syscb_resume},
    {"sim.syscb_beforeinstanceswitch", sim_syscb_beforeinstanceswitch},
    {"sim.syscb_afterinstanceswitch", sim_syscb_afterinstanceswitch},
    {"sim.syscb_beforecopy", sim_syscb_beforecopy},
    {"sim.syscb_aftercopy", sim_syscb_aftercopy},
    {"sim.syscb_aos_suspend", sim_syscb_aos_suspend},
    {"sim.syscb_aos_resume", sim_syscb_aos_resume},
    {"sim.syscb_joint", sim_syscb_joint},
    {"sim.syscb_vision", sim_syscb_vision},
    {"sim.syscb_trigger", sim_syscb_trigger},
    {"sim.syscb_contact", sim_syscb_contact},
    {"sim.syscb_dyn", sim_syscb_dyn},
    {"sim.syscb_beforedelete", sim_syscb_beforedelete},
    {"sim.syscb_afterdelete", sim_syscb_afterdelete},
    {"sim.syscb_aftercreate", sim_syscb_aftercreate},
    //{"sim.syscb_threadmain",sim_syscb_threadmain},
    {"sim.syscb_userconfig", sim_syscb_userconfig},
    {"sim.syscb_moduleentry", sim_syscb_moduleentry},
    {"sim.syscb_selchange", sim_syscb_selchange},
    {"sim.syscb_data", sim_syscb_data},
    // script params:
    {"sim.scriptintparam_execorder", sim_scriptintparam_execorder},
    {"sim.scriptintparam_execcount", sim_scriptintparam_execcount},
    {"sim.scriptintparam_type", sim_scriptintparam_type},
    {"sim.scriptintparam_enabled", sim_scriptintparam_enabled},
    {"sim.scriptintparam_autorestartonerror", sim_scriptintparam_autorestartonerror},
    {"sim.scriptstringparam_description", sim_scriptstringparam_description},
    {"sim.scriptstringparam_name", sim_scriptstringparam_name},
    {"sim.scriptstringparam_text", sim_scriptstringparam_text},
    {"sim.scriptstringparam_nameext", sim_scriptstringparam_nameext},
    {"sim.scriptstringparam_lang", sim_scriptstringparam_lang},
    // script execution order:
    {"sim.scriptexecorder_first", sim_scriptexecorder_first},
    {"sim.scriptexecorder_normal", sim_scriptexecorder_normal},
    {"sim.scriptexecorder_last", sim_scriptexecorder_last},
    // language:
    {"sim.sim_lang_undefined", sim_lang_undefined},
    {"sim.sim_lang_lua", sim_lang_lua},
    {"sim.sim_lang_python", sim_lang_python},
    // special arguments for some functions:
    {"sim.handle_world", sim_handle_world},
    {"sim.handle_all", sim_handle_all},
    {"sim.handle_all_except_explicit", sim_handle_all_except_explicit},
    {"sim.handle_self", sim_handle_self},
    {"sim.handle_mainscript", sim_handle_mainscript},
    {"sim.handle_tree", sim_handle_tree},
    {"sim.handle_chain", sim_handle_chain},
    {"sim.handle_single", sim_handle_single},
    {"sim.handle_default", sim_handle_default},
    {"sim.handle_all_except_self", sim_handle_all_except_self},
    {"sim.handle_parent", sim_handle_parent},
    {"sim.handle_scene", sim_handle_scene},
    {"sim.handle_app", sim_handle_app},
    {"sim.handle_inverse", sim_handle_inverse},
    {"sim.handle_appstorage", sim_handle_appstorage},
    {"sim.handle_sceneobject", sim_handle_sceneobject},
    {"sim.handle_sandbox", sim_handle_sandbox},
    {"sim.handle_mesh", sim_handle_mesh},

    // special handle flags:
    {"sim.handleflag_assembly", sim_handleflag_assembly},
    {"sim.handleflag_camera", sim_handleflag_camera},
    {"sim.handleflag_togglevisibility", sim_handleflag_togglevisibility},
    {"sim.handleflag_extended", sim_handleflag_extended},
    {"sim.handleflag_greyscale", sim_handleflag_greyscale},
    {"sim.handleflag_depthbuffermeters", sim_handleflag_depthbuffermeters},
    {"sim.handleflag_depthbuffer", sim_handleflag_depthbuffer},
    {"sim.handleflag_keeporiginal", sim_handleflag_keeporiginal},
    {"sim.handleflag_codedstring", sim_handleflag_codedstring},
    {"sim.handleflag_wxyzquat", sim_handleflag_wxyzquat},
    {"sim.handleflag_reljointbaseframe", sim_handleflag_reljointbaseframe},
    {"sim.handleflag_addmultiple", sim_handleflag_addmultiple},
    {"sim.handleflag_abscoords", sim_handleflag_abscoords},
    {"sim.handleflag_model", sim_handleflag_model},
    {"sim.handleflag_rawvalue", sim_handleflag_rawvalue},
    {"sim.handleflag_axis", sim_handleflag_axis},
    {"sim.handleflag_resetforce", sim_handleflag_resetforce},
    {"sim.handleflag_resettorque", sim_handleflag_resettorque},
    {"sim.handleflag_resetforcetorque", sim_handleflag_resetforcetorque},
    {"sim.handleflag_silenterror", sim_handleflag_silenterror},
    // General object main properties:
    {"sim.objectspecialproperty_collidable", sim_objectspecialproperty_collidable},
    {"sim.objectspecialproperty_measurable", sim_objectspecialproperty_measurable},
    {"sim.objectspecialproperty_detectable", sim_objectspecialproperty_detectable},
    // Model override properties:
    {"sim.modelproperty_not_collidable", sim_modelproperty_not_collidable},
    {"sim.modelproperty_not_measurable", sim_modelproperty_not_measurable},
    {"sim.modelproperty_not_detectable", sim_modelproperty_not_detectable},
    {"sim.modelproperty_not_dynamic", sim_modelproperty_not_dynamic},
    {"sim.modelproperty_not_respondable", sim_modelproperty_not_respondable},
    {"sim.modelproperty_not_reset", sim_modelproperty_not_reset},
    {"sim.modelproperty_not_visible", sim_modelproperty_not_visible},
    {"sim.modelproperty_scripts_inactive", sim_modelproperty_scripts_inactive},
    {"sim.modelproperty_not_showasinsidemodel", sim_modelproperty_not_showasinsidemodel},
    {"sim.modelproperty_not_model", sim_modelproperty_not_model},

    // verbosity:
    {"sim.verbosity_useglobal", sim_verbosity_useglobal},
    {"sim.verbosity_none", sim_verbosity_none},
    {"sim.verbosity_errors", sim_verbosity_errors},
    {"sim.verbosity_warnings", sim_verbosity_warnings},
    {"sim.verbosity_loadinfos", sim_verbosity_loadinfos},
    {"sim.verbosity_questions", sim_verbosity_questions},
    {"sim.verbosity_scripterrors", sim_verbosity_scripterrors},
    {"sim.verbosity_scriptwarnings", sim_verbosity_scriptwarnings},
    {"sim.verbosity_scriptinfos", sim_verbosity_scriptinfos},
    {"sim.verbosity_msgs", sim_verbosity_msgs},
    {"sim.verbosity_infos", sim_verbosity_infos},
    {"sim.verbosity_debug", sim_verbosity_debug},
    {"sim.verbosity_trace", sim_verbosity_trace},
    {"sim.verbosity_tracelua", sim_verbosity_tracelua},
    {"sim.verbosity_traceall", sim_verbosity_traceall},
    {"sim.verbosity_default", sim_verbosity_default},
    {"sim.verbosity_undecorated", sim_verbosity_undecorated},
    {"sim.verbosity_onlyterminal", sim_verbosity_onlyterminal},
    {"sim.verbosity_once", sim_verbosity_once},

    // plugin info:
    {"sim.plugininfo_extversionstr", sim_plugininfo_extversionstr},
    {"sim.plugininfo_builddatestr", sim_plugininfo_builddatestr},
    {"sim.plugininfo_extversionint", sim_plugininfo_extversionint},
    {"sim.plugininfo_verbosity", sim_plugininfo_verbosity},
    {"sim.plugininfo_statusbarverbosity", sim_plugininfo_statusbarverbosity},

    // drawing objects
    {"sim.drawing_points", sim_drawing_points},
    {"sim.drawing_lines", sim_drawing_lines},
    {"sim.drawing_linestrip", sim_drawing_linestrip},
    {"sim.drawing_triangles", sim_drawing_triangles},
    {"sim.drawing_trianglepts", sim_drawing_trianglepts},
    {"sim.drawing_quadpts", sim_drawing_quadpts},
    {"sim.drawing_discpts", sim_drawing_discpts},
    {"sim.drawing_cubepts", sim_drawing_cubepts},
    {"sim.drawing_spherepts", sim_drawing_spherepts},

    {"sim.drawing_painttag", sim_drawing_painttag},
    {"sim.drawing_cyclic", sim_drawing_cyclic},
    {"sim.drawing_overlay", sim_drawing_overlay},
    {"sim.drawing_local", sim_drawing_local},

    // joint modes
    {"sim.jointmode_kinematic", sim_jointmode_kinematic},
    {"sim.jointmode_dependent", sim_jointmode_dependent},
    {"sim.jointmode_dynamic", sim_jointmode_dynamic},
    // joint dyn ctrl modes
    {"sim.jointdynctrl_free", sim_jointdynctrl_free},
    {"sim.jointdynctrl_force", sim_jointdynctrl_force},
    {"sim.jointdynctrl_velocity", sim_jointdynctrl_velocity},
    {"sim.jointdynctrl_position", sim_jointdynctrl_position},
    {"sim.jointdynctrl_spring", sim_jointdynctrl_spring},
    {"sim.jointdynctrl_callback", sim_jointdynctrl_callback},
    // physics engine
    {"sim.physics_bullet", sim_physics_bullet},
    {"sim.physics_ode", sim_physics_ode},
    {"sim.physics_vortex", sim_physics_vortex},
    {"sim.physics_newton", sim_physics_newton},
    {"sim.physics_mujoco", sim_physics_mujoco},
    {"sim.physics_drake", sim_physics_drake},
    // pure primitives type
    {"sim.primitiveshape_none", sim_primitiveshape_none},
    {"sim.primitiveshape_plane", sim_primitiveshape_plane},
    {"sim.primitiveshape_disc", sim_primitiveshape_disc},
    {"sim.primitiveshape_cuboid", sim_primitiveshape_cuboid},
    {"sim.primitiveshape_spheroid", sim_primitiveshape_spheroid},
    {"sim.primitiveshape_cylinder", sim_primitiveshape_cylinder},
    {"sim.primitiveshape_cone", sim_primitiveshape_cone},
    {"sim.primitiveshape_heightfield", sim_primitiveshape_heightfield},
    {"sim.primitiveshape_capsule", sim_primitiveshape_capsule},
    // property type
    {"sim.propertytype_bool", sim_propertytype_bool},
    {"sim.propertytype_int", sim_propertytype_int},
    {"sim.propertytype_float", sim_propertytype_float},
    {"sim.propertytype_string", sim_propertytype_string},
    {"sim.propertytype_buffer", sim_propertytype_buffer},
    {"sim.propertytype_intarray2", sim_propertytype_intarray2},
    {"sim.propertytype_long", sim_propertytype_long},
    {"sim.propertytype_vector2", sim_propertytype_vector2},
    {"sim.propertytype_vector3", sim_propertytype_vector3},
    {"sim.propertytype_quaternion", sim_propertytype_quaternion},
    {"sim.propertytype_pose", sim_propertytype_pose},
    {"sim.propertytype_matrix3x3", sim_propertytype_matrix3x3},
    {"sim.propertytype_matrix4x4", sim_propertytype_matrix4x4},
    {"sim.propertytype_color", sim_propertytype_color},
    {"sim.propertytype_floatarray", sim_propertytype_floatarray},
    {"sim.propertytype_intarray", sim_propertytype_intarray},
    {"sim.propertytype_table", sim_propertytype_table},
    {"sim.propertytype_null", sim_propertytype_null},
    {"sim.propertytype_matrix", sim_propertytype_matrix},
    {"sim.propertytype_array", sim_propertytype_array},
    {"sim.propertytype_map", sim_propertytype_map},
    {"sim.propertytype_handle", sim_propertytype_handle},
    {"sim.propertytype_handlearray", sim_propertytype_handlearray},
    // property info
    {"sim.propertyinfo_notwritable", sim_propertyinfo_notwritable},
    {"sim.propertyinfo_notreadable", sim_propertyinfo_notreadable},
    {"sim.propertyinfo_removable", sim_propertyinfo_removable},
    {"sim.propertyinfo_largedata", sim_propertyinfo_largedata},
    {"sim.propertyinfo_deprecated", sim_propertyinfo_deprecated},
    {"sim.propertyinfo_modelhashexclude", sim_propertyinfo_modelhashexclude},
    // dummy-dummy link types
    {"sim.dummytype_dynloopclosure", sim_dummytype_dynloopclosure},
    {"sim.dummytype_dyntendon", sim_dummytype_dyntendon},
    {"sim.dummytype_default", sim_dummytype_default},
    {"sim.dummytype_assembly", sim_dummytype_assembly},
    // color components
    {"sim.colorcomponent_ambient", sim_colorcomponent_ambient},
    {"sim.colorcomponent_ambient_diffuse", sim_colorcomponent_ambient_diffuse},
    {"sim.colorcomponent_diffuse", sim_colorcomponent_diffuse},
    {"sim.colorcomponent_specular", sim_colorcomponent_specular},
    {"sim.colorcomponent_emission", sim_colorcomponent_emission},
    {"sim.colorcomponent_transparency", sim_colorcomponent_transparency},
    {"sim.colorcomponent_auxiliary", sim_colorcomponent_auxiliary},
    // volume types
    {"sim.volume_ray", sim_volume_ray},
    {"sim.volume_randomizedray", sim_volume_randomizedray},
    {"sim.volume_pyramid", sim_volume_pyramid},
    {"sim.volume_cylinder", sim_volume_cylinder},
    {"sim.volume_disc", sim_volume_disc},
    {"sim.volume_cone", sim_volume_cone},

    // Vortex friction models
    {"sim.vortex_bodyfrictionmodel_box", sim_vortex_bodyfrictionmodel_box},
    {"sim.vortex_bodyfrictionmodel_scaledbox", sim_vortex_bodyfrictionmodel_scaledbox},
    {"sim.vortex_bodyfrictionmodel_proplow", sim_vortex_bodyfrictionmodel_proplow},
    {"sim.vortex_bodyfrictionmodel_prophigh", sim_vortex_bodyfrictionmodel_prophigh},
    {"sim.vortex_bodyfrictionmodel_scaledboxfast", sim_vortex_bodyfrictionmodel_scaledboxfast},
    {"sim.vortex_bodyfrictionmodel_neutral", sim_vortex_bodyfrictionmodel_neutral},
    {"sim.vortex_bodyfrictionmodel_none", sim_vortex_bodyfrictionmodel_none},
    // Bullet constraint solver types
    {"sim.bullet_constraintsolvertype_sequentialimpulse", sim_bullet_constraintsolvertype_sequentialimpulse},
    {"sim.bullet_constraintsolvertype_nncg", sim_bullet_constraintsolvertype_nncg},
    {"sim.bullet_constraintsolvertype_dantzig", sim_bullet_constraintsolvertype_dantzig},
    {"sim.bullet_constraintsolvertype_projectedgaussseidel", sim_bullet_constraintsolvertype_projectedgaussseidel},
    // buffer types:
    {"sim.buffer_uint8", sim_buffer_uint8},
    {"sim.buffer_int8", sim_buffer_int8},
    {"sim.buffer_uint16", sim_buffer_uint16},
    {"sim.buffer_int16", sim_buffer_int16},
    {"sim.buffer_uint32", sim_buffer_uint32},
    {"sim.buffer_int32", sim_buffer_int32},
    {"sim.buffer_float", sim_buffer_float},
    {"sim.buffer_double", sim_buffer_double},
    {"sim.buffer_uint8rgb", sim_buffer_uint8rgb},
    {"sim.buffer_uint8bgr", sim_buffer_uint8bgr},
    {"sim.buffer_uint8rgba", sim_buffer_uint8rgba},
    {"sim.buffer_uint8argb", sim_buffer_uint8argb},
    {"sim.buffer_base64", sim_buffer_base64},
    {"sim.buffer_split", sim_buffer_split},
    {"sim.buffer_clamp", sim_buffer_clamp},
    // Image combination:
    {"sim.imgcomb_vertical", sim_imgcomb_vertical},
    {"sim.imgcomb_horizontal", sim_imgcomb_horizontal},
    // Graph data stream transformations:
    {"sim.stream_transf_raw", sim_stream_transf_raw},
    {"sim.stream_transf_derivative", sim_stream_transf_derivative},
    {"sim.stream_transf_integral", sim_stream_transf_integral},
    {"sim.stream_transf_cumulative", sim_stream_transf_cumulative},

    // Ruckig:
    {"sim.ruckig_phasesync", sim_ruckig_phasesync},
    {"sim.ruckig_timesync", sim_ruckig_timesync},
    {"sim.ruckig_nosync", sim_ruckig_nosync},
    {"sim.ruckig_minvel", sim_ruckig_minvel},
    {"sim.ruckig_minaccel", sim_ruckig_minaccel},

    // deprecated!
    {"sim1.particle_points1", sim_particle_points1},
    {"sim1.particle_points2", sim_particle_points2},
    {"sim1.particle_points4", sim_particle_points4},
    {"sim1.particle_roughspheres", sim_particle_roughspheres},
    {"sim1.particle_spheres", sim_particle_spheres},
    {"sim1.particle_respondable1to4", sim_particle_respondable1to4},
    {"sim1.particle_respondable5to8", sim_particle_respondable5to8},
    {"sim1.particle_particlerespondable", sim_particle_particlerespondable},
    {"sim1.particle_ignoresgravity", sim_particle_ignoresgravity},
    {"sim1.particle_invisible", sim_particle_invisible},
    {"sim1.particle_painttag", sim_particle_painttag},
    {"sim1.particle_itemsizes", sim_particle_itemsizes},
    {"sim1.particle_itemdensities", sim_particle_itemdensities},
    {"sim1.particle_itemcolors", sim_particle_itemcolors},
    {"sim1.particle_cyclic", sim_particle_cyclic},
    {"sim1.particle_emissioncolor", sim_particle_emissioncolor},
    {"sim1.particle_water", sim_particle_water},
    {"sim1.handleflag_altname", sim_handleflag_altname},
    {"sim1.handle_main_script", sim_handle_mainscript},
    {"sim1.object_shape_type", sim_sceneobject_shape},
    {"sim1.object_joint_type", sim_sceneobject_joint},
    {"sim1.object_graph_type", sim_sceneobject_graph},
    {"sim1.object_camera_type", sim_sceneobject_camera},
    {"sim1.object_dummy_type", sim_sceneobject_dummy},
    {"sim1.object_proximitysensor_type", sim_sceneobject_proximitysensor},
    {"sim1.object_path_type", sim_sceneobject_path},
    {"sim1.object_renderingsensor_type", sim_sceneobject_visionsensor},
    {"sim1.object_visionsensor_type", sim_sceneobject_visionsensor},
    {"sim1.object_mill_type", sim_sceneobject_mill},
    {"sim1.object_forcesensor_type", sim_sceneobject_forcesensor},
    {"sim1.object_light_type", sim_sceneobject_light},
    {"sim1.object_mirror_type", sim_sceneobject_mirror},
    {"sim1.object_octree_type", sim_sceneobject_octree},
    {"sim1.object_pointcloud_type", sim_sceneobject_pointcloud},
    {"sim1.object_script_type", sim_sceneobject_script},
    {"sim1.light_omnidirectional_subtype", sim_light_omnidirectional},
    {"sim1.light_spot_subtype", sim_light_spot},
    {"sim1.light_directional_subtype", sim_light_directional},
    {"sim1.joint_revolute_subtype", sim_joint_revolute},
    {"sim1.joint_prismatic_subtype", sim_joint_prismatic},
    {"sim1.joint_spherical_subtype", sim_joint_spherical},
    {"sim1.shape_simpleshape_subtype", sim_shape_simple},
    {"sim1.shape_multishape_subtype", sim_shape_compound},
    {"sim1.proximitysensor_pyramid_subtype", sim_proximitysensor_pyramid},
    {"sim1.proximitysensor_cylinder_subtype", sim_proximitysensor_cylinder},
    {"sim1.proximitysensor_disc_subtype", sim_proximitysensor_disc},
    {"sim1.proximitysensor_cone_subtype", sim_proximitysensor_cone},
    {"sim1.proximitysensor_ray_subtype", sim_proximitysensor_ray},
    {"sim1.mill_pyramid_subtype", sim_mill_pyramid_subtype},
    {"sim1.mill_cylinder_subtype", sim_mill_cylinder_subtype},
    {"sim1.mill_disc_subtype", sim_mill_disc_subtype},
    {"sim1.mill_cone_subtype", sim_mill_cone_subtype},
    {"sim1.object_no_subtype", sim_object_no_subtype},
    {"sim1.appobj_object_type", sim_objecttype_sceneobject},
    {"sim1.appobj_simulation_type", sim_appobj_simulation_type},
    {"sim1.appobj_script_type", sim_appobj_script_type},
    {"sim1.appobj_texture_type", sim_objecttype_texture},
    {"sim1.scriptintparam_handle", sim_scriptintparam_handle},
    {"sim1.scriptintparam_objecthandle", sim_scriptintparam_objecthandle},
    {"sim1.scriptintparam_lang", sim_scriptintparam_lang},
    {"sim1.scripttype_mainscript", sim_scripttype_main},
    {"sim1.scripttype_childscript", sim_scripttype_simulation},
    {"sim1.scripttype_addonscript", sim_scripttype_addon},
    {"sim1.scripttype_customizationscript", sim_scripttype_customization},
    {"sim1.scripttype_sandboxscript", sim_scripttype_sandbox},
    {"sim1.drawing_itemcolors", sim_drawing_itemcolors},
    {"sim1.drawing_vertexcolors", sim_drawing_vertexcolors},
    {"sim1.drawing_itemsizes", sim_drawing_itemsizes},
    {"sim1.drawing_backfaceculling", sim_drawing_backfaceculling},
    {"sim1.drawing_wireframe", sim_drawing_wireframe},
    {"sim1.drawing_followparentvisibility", sim_drawing_followparentvisibility},
    {"sim1.drawing_50percenttransparency", sim_drawing_50percenttransparency},
    {"sim1.drawing_25percenttransparency", sim_drawing_25percenttransparency},
    {"sim1.drawing_12percenttransparency", sim_drawing_12percenttransparency},
    {"sim1.drawing_emissioncolor", sim_drawing_emissioncolor},
    {"sim1.drawing_facingcamera", sim_drawing_facingcamera},
    {"sim1.drawing_itemtransparency", sim_drawing_itemtransparency},
    {"sim1.drawing_persistent", sim_drawing_persistent},
    {"sim1.drawing_auxchannelcolor1", sim_drawing_auxchannelcolor1},
    {"sim1.drawing_auxchannelcolor2", sim_drawing_auxchannelcolor2},
    {"sim1.dummyintparam_link_type", sim_dummyintparam_dummytype},
    {"sim1.filtercomponent_originalimage", sim_filtercomponent_originalimage_deprecated},
    {"sim1.filtercomponent_originaldepth", sim_filtercomponent_originaldepth_deprecated},
    {"sim1.filtercomponent_uniformimage", sim_filtercomponent_uniformimage_deprecated},
    {"sim1.filtercomponent_tooutput", sim_filtercomponent_tooutput_deprecated},
    {"sim1.filtercomponent_tobuffer1", sim_filtercomponent_tobuffer1_deprecated},
    {"sim1.filtercomponent_tobuffer2", sim_filtercomponent_tobuffer2_deprecated},
    {"sim1.filtercomponent_frombuffer1", sim_filtercomponent_frombuffer1_deprecated},
    {"sim1.filtercomponent_frombuffer2", sim_filtercomponent_frombuffer2_deprecated},
    {"sim1.filtercomponent_swapbuffers", sim_filtercomponent_swapbuffers_deprecated},
    {"sim1.filtercomponent_addbuffer1", sim_filtercomponent_addbuffer1_deprecated},
    {"sim1.filtercomponent_subtractbuffer1", sim_filtercomponent_subtractbuffer1_deprecated},
    {"sim1.filtercomponent_multiplywithbuffer1", sim_filtercomponent_multiplywithbuffer1_deprecated},
    {"sim1.filtercomponent_horizontalflip", sim_filtercomponent_horizontalflip_deprecated},
    {"sim1.filtercomponent_verticalflip", sim_filtercomponent_verticalflip_deprecated},
    {"sim1.filtercomponent_rotate", sim_filtercomponent_rotate_deprecated},
    {"sim1.filtercomponent_shift", sim_filtercomponent_shift_deprecated},
    {"sim1.filtercomponent_resize", sim_filtercomponent_resize_deprecated},
    {"sim1.filtercomponent_3x3filter", sim_filtercomponent_3x3filter_deprecated},
    {"sim1.filtercomponent_5x5filter", sim_filtercomponent_5x5filter_deprecated},
    {"sim1.filtercomponent_sharpen", sim_filtercomponent_sharpen_deprecated},
    {"sim1.filtercomponent_edge", sim_filtercomponent_edge_deprecated},
    {"sim1.filtercomponent_rectangularcut", sim_filtercomponent_rectangularcut_deprecated},
    {"sim1.filtercomponent_circularcut", sim_filtercomponent_circularcut_deprecated},
    {"sim1.filtercomponent_normalize", sim_filtercomponent_normalize_deprecated},
    {"sim1.filtercomponent_intensityscale", sim_filtercomponent_intensityscale_deprecated},
    {"sim1.filtercomponent_keeporremovecolors", sim_filtercomponent_keeporremovecolors_deprecated},
    {"sim1.filtercomponent_scaleandoffsetcolors", sim_filtercomponent_scaleandoffsetcolors_deprecated},
    {"sim1.filtercomponent_binary", sim_filtercomponent_binary_deprecated},
    {"sim1.filtercomponent_swapwithbuffer1", sim_filtercomponent_swapwithbuffer1_deprecated},
    {"sim1.filtercomponent_addtobuffer1", sim_filtercomponent_addtobuffer1_deprecated},
    {"sim1.filtercomponent_subtractfrombuffer1", sim_filtercomponent_subtractfrombuffer1_deprecated},
    {"sim1.filtercomponent_correlationwithbuffer1", sim_filtercomponent_correlationwithbuffer1_deprecated},
    {"sim1.filtercomponent_colorsegmentation", sim_filtercomponent_colorsegmentation_deprecated},
    {"sim1.filtercomponent_blobextraction", sim_filtercomponent_blobextraction_deprecated},
    {"sim1.filtercomponent_imagetocoord", sim_filtercomponent_imagetocoord_deprecated},
    {"sim1.filtercomponent_pixelchange", sim_filtercomponent_pixelchange_deprecated},
    {"sim1.filtercomponent_velodyne", sim_filtercomponent_velodyne_deprecated},
    {"sim1.filtercomponent_todepthoutput", sim_filtercomponent_todepthoutput_deprecated},
    {"sim1.filtercomponent_customized", sim_filtercomponent_customized_deprecated},
    {"sim1.boolparam_scene_and_model_load_messages", sim_boolparam_scene_and_model_load_messages},
    {"sim1.handleflag_setmultiple", sim_handleflag_setmultiple},
    {"sim1.mainscriptcall_initialization", sim_syscb_init},
    {"sim1.mainscriptcall_cleanup", sim_syscb_cleanup},
    {"sim1.mainscriptcall_regular", sim_syscb_actuation},
    {"sim1.syscb_regular", sim_syscb_actuation},
    {"sim1.childscriptcall_initialization", sim_syscb_init},
    {"sim1.childscriptcall_cleanup", sim_syscb_cleanup},
    {"sim1.childscriptcall_actuation", sim_syscb_actuation},
    {"sim1.childscriptcall_sensing", sim_syscb_sensing},
    {"sim1.addonscriptcall_initialization", sim_syscb_init},
    {"sim1.addonscriptcall_suspend", sim_syscb_aos_suspend},
    {"sim1.addonscriptcall_restarting", sim_syscb_aos_resume},
    {"sim1.addonscriptcall_cleanup", sim_syscb_cleanup},
    {"sim1.customizationscriptcall_initialization", sim_syscb_init},
    {"sim1.customizationscriptcall_cleanup", sim_syscb_cleanup},
    {"sim1.customizationscriptcall_nonsimulation", sim_syscb_nonsimulation},
    {"sim1.customizationscriptcall_lastbeforesimulation", sim_syscb_beforesimulation},
    {"sim1.customizationscriptcall_firstaftersimulation", sim_syscb_aftersimulation},
    {"sim1.customizationscriptcall_simulationactuation", sim_syscb_actuation},
    {"sim1.customizationscriptcall_simulationsensing", sim_syscb_sensing},
    {"sim1.customizationscriptcall_simulationpause", sim_syscb_suspended},
    {"sim1.customizationscriptcall_simulationpausefirst", sim_syscb_suspend},
    {"sim1.customizationscriptcall_simulationpauselast", sim_syscb_resume},
    {"sim1.customizationscriptcall_lastbeforeinstanceswitch", sim_syscb_beforeinstanceswitch},
    {"sim1.customizationscriptcall_firstafterinstanceswitch", sim_syscb_afterinstanceswitch},
    {"sim1.customizationscriptcall_beforecopy", sim_syscb_beforecopy},
    {"sim1.customizationscriptcall_aftercopy", sim_syscb_aftercopy},
    {"sim1.boolparam_show_w_emitters", sim_boolparam_show_w_emitters},
    {"sim1.boolparam_show_w_receivers", sim_boolparam_show_w_receivers},
    {"sim1.syscb_aos_run", sim_syscb_aos_run_old},
    {"sim1.dummyintparam_follow_path", sim_dummyintparam_follow_path},
    {"sim1.dummyfloatparam_follow_path_offset", sim_dummyfloatparam_follow_path_offset},
    {"sim1.addonscriptcall_run", sim_syscb_aos_run_old},
    {"sim1.shapestringparam_color_name", sim_shapestringparam_color_name},
    {"sim1.jointfloatparam_screw_pitch", sim_jointfloatparam_screw_pitch},
    {"sim1.boolparam_force_show_wireless_emission", sim_boolparam_force_show_wireless_emission},
    {"sim1.boolparam_force_show_wireless_reception", sim_boolparam_force_show_wireless_reception},
    {"sim1.boolparam_collision_handling_enabled", sim_boolparam_collision_handling_enabled},
    {"sim1.boolparam_distance_handling_enabled", sim_boolparam_distance_handling_enabled},
    {"sim1.boolparam_proximity_sensor_handling_enabled", sim_boolparam_proximity_sensor_handling_enabled},
    {"sim1.boolparam_vision_sensor_handling_enabled", sim_boolparam_vision_sensor_handling_enabled},
    {"sim1.boolparam_rendering_sensor_handling_enabled", sim_boolparam_vision_sensor_handling_enabled},
    {"sim1.boolparam_mill_handling_enabled", sim_boolparam_mill_handling_enabled},
    {"sim1.modelproperty_not_renderable", sim_modelproperty_not_renderable},
    {"sim1.objectspecialproperty_detectable_ultrasonic", sim_objectspecialproperty_detectable_ultrasonic},
    {"sim1.objectspecialproperty_detectable_infrared", sim_objectspecialproperty_detectable_infrared},
    {"sim1.objectspecialproperty_detectable_laser", sim_objectspecialproperty_detectable_laser},
    {"sim1.objectspecialproperty_detectable_inductive", sim_objectspecialproperty_detectable_inductive},
    {"sim1.objectspecialproperty_detectable_capacitive", sim_objectspecialproperty_detectable_capacitive},
    {"sim1.objectspecialproperty_renderable", sim_objectspecialproperty_renderable},
    {"sim1.objectspecialproperty_detectable_all", sim_objectspecialproperty_detectable},
    {"sim1.objectspecialproperty_pathplanning_ignored", sim_objectspecialproperty_pathplanning_ignored},
    {"sim1.objectproperty_canupdatedna", sim_objectproperty_canupdatedna},
    {"sim1.objectproperty_hierarchyhiddenmodelchild", sim_objectproperty_hierarchyhiddenmodelchild},
    {"sim1.jointmode_ik", sim_jointmode_ik_deprecated},
    {"sim1.jointmode_ikdependent", sim_jointmode_reserved_previously_ikdependent},
    {"sim1.appobj_ik_type", sim_appobj_ik_type},
    {"sim1.ik_pseudo_inverse_method", sim_ik_pseudo_inverse_method},
    {"sim1.ik_damped_least_squares_method", sim_ik_damped_least_squares_method},
    {"sim1.ik_jacobian_transpose_method", sim_ik_jacobian_transpose_method},
    {"sim1.ik_undamped_pseudo_inverse_method", sim_ik_undamped_pseudo_inverse_method},
    {"sim1.ik_x_constraint", sim_ik_x_constraint},
    {"sim1.ik_y_constraint", sim_ik_y_constraint},
    {"sim1.ik_z_constraint", sim_ik_z_constraint},
    {"sim1.ik_alpha_beta_constraint", sim_ik_alpha_beta_constraint},
    {"sim1.ik_gamma_constraint", sim_ik_gamma_constraint},
    {"sim1.ikresult_not_performed", sim_ikresult_not_performed},
    {"sim1.ikresult_success", sim_ikresult_success},
    {"sim1.ikresult_fail", sim_ikresult_fail},
    {"sim1.dummy_linktype_gcs_loop_closure", sim_dummy_linktype_gcs_loop_closure},
    {"sim1.dummy_linktype_gcs_tip", sim_dummy_linktype_gcs_tip},
    {"sim1.dummy_linktype_gcs_target", sim_dummy_linktype_gcs_target},
    {"sim1.dummy_linktype_ik_tip_target", sim_dummy_linktype_ik_tip_target},
    {"sim1.dummy_linktype_dynamics_force_constraint", sim_dummy_linktype_dynamics_force_constraint},
    {"sim1.jointfloatparam_step_size", sim_jointfloatparam_step_size},
    {"sim1.jointfloatparam_ik_weight", sim_jointfloatparam_ik_weight},
    {"sim1.boolparam_ik_handling_enabled", sim_boolparam_ik_handling_enabled},
    {"sim1.boolparam_gcs_handling_enabled", sim_boolparam_gcs_handling_enabled},
    {"sim1.appobj_2delement_type", sim_appobj_ui_type}, // for backward compatibility
    {"sim1.appobj_ui_type", sim_appobj_ui_type},
    {"sim1.appobj_pathplanning_type", sim_appobj_pathplanning_type},
    {"sim1.navigation_camerafly", sim_navigation_camerafly_old},
    {"sim1.banner_left", sim_banner_left},
    {"sim1.banner_right", sim_banner_right},
    {"sim1.banner_nobackground", sim_banner_nobackground},
    {"sim1.banner_overlay", sim_banner_overlay},
    {"sim1.banner_followparentvisibility", sim_banner_followparentvisibility},
    {"sim1.banner_clickselectsparent", sim_banner_clickselectsparent},
    {"sim1.banner_clicktriggersevent", sim_banner_clicktriggersevent},
    {"sim1.banner_facingcamera", sim_banner_facingcamera},
    {"sim1.banner_fullyfacingcamera", sim_banner_fullyfacingcamera},
    {"sim1.banner_backfaceculling", sim_banner_backfaceculling},
    {"sim1.banner_keepsamesize", sim_banner_keepsamesize},
    {"sim1.banner_bitmapfont", sim_banner_bitmapfont},
    {"sim1.scriptdebug_none", sim_scriptdebug_none},
    {"sim1.scriptdebug_syscalls", sim_scriptdebug_syscalls},
    {"sim1.scriptdebug_vars_interval", sim_scriptdebug_vars_interval},
    {"sim1.scriptdebug_allcalls", sim_scriptdebug_allcalls},
    {"sim1.scriptdebug_vars", sim_scriptdebug_vars},
    {"sim1.scriptdebug_callsandvars", sim_scriptdebug_callsandvars},
    {"sim1.callbackid_rossubscriber", sim_callbackid_rossubscriber},
    {"sim1.callbackid_dynstep", sim_callbackid_dynstep},
    {"sim1.callbackid_userdefined", sim_callbackid_userdefined},
    {"sim1.buttonproperty_button", sim_buttonproperty_button},
    {"sim1.buttonproperty_label", sim_buttonproperty_label},
    {"sim1.buttonproperty_editbox", sim_buttonproperty_editbox},
    {"sim1.buttonproperty_slider", sim_buttonproperty_slider},
    {"sim1.buttonproperty_staydown", sim_buttonproperty_staydown},
    {"sim1.buttonproperty_enabled", sim_buttonproperty_enabled},
    {"sim1.buttonproperty_borderless", sim_buttonproperty_borderless},
    {"sim1.buttonproperty_horizontallycentered", sim_buttonproperty_horizontallycentered},
    {"sim1.buttonproperty_ignoremouse", sim_buttonproperty_ignoremouse},
    {"sim1.buttonproperty_isdown", sim_buttonproperty_isdown},
    {"sim1.buttonproperty_transparent", sim_buttonproperty_transparent},
    {"sim1.buttonproperty_nobackgroundcolor", sim_buttonproperty_nobackgroundcolor},
    {"sim1.buttonproperty_rollupaction", sim_buttonproperty_rollupaction},
    {"sim1.buttonproperty_closeaction", sim_buttonproperty_closeaction},
    {"sim1.buttonproperty_verticallycentered", sim_buttonproperty_verticallycentered},
    {"sim1.buttonproperty_downupevent", sim_buttonproperty_downupevent},
    {"sim1.pathproperty_show_line", sim_pathproperty_show_line},
    {"sim1.pathproperty_show_orientation", sim_pathproperty_show_orientation},
    {"sim1.pathproperty_closed_path", sim_pathproperty_closed_path},
    {"sim1.pathproperty_automatic_orientation", sim_pathproperty_automatic_orientation},
    {"sim1.pathproperty_flat_path", sim_pathproperty_flat_path},
    {"sim1.pathproperty_show_position", sim_pathproperty_show_position},
    {"sim1.pathproperty_keep_x_up", sim_pathproperty_keep_x_up},
    {"sim1.distcalcmethod_dl", sim_distcalcmethod_dl},
    {"sim1.distcalcmethod_dac", sim_distcalcmethod_dac},
    {"sim1.distcalcmethod_max_dl_dac", sim_distcalcmethod_max_dl_dac},
    {"sim1.distcalcmethod_dl_and_dac", sim_distcalcmethod_dl_and_dac},
    {"sim1.distcalcmethod_sqrt_dl2_and_dac2", sim_distcalcmethod_sqrt_dl2_and_dac2},
    {"sim1.distcalcmethod_dl_if_nonzero", sim_distcalcmethod_dl_if_nonzero},
    {"sim1.distcalcmethod_dac_if_nonzero", sim_distcalcmethod_dac_if_nonzero},
    {"sim1.appobj_collision_type", sim_appobj_collision_type},
    {"sim1.appobj_distance_type", sim_appobj_distance_type},
    {"sim1.appobj_collection_type", sim_objecttype_collection},
    {"sim1.message_ui_button_state_change", sim_message_ui_button_state_change},
    {"sim1.message_bannerclicked", sim_message_bannerclicked},
    {"sim1.message_prox_sensor_select_down", sim_message_prox_sensor_select_down},
    {"sim1.message_prox_sensor_select_up", sim_message_prox_sensor_select_up},
    {"sim1.message_pick_select_down", sim_message_pick_select_down},
    {"sim1.scripttype_addonfunction", sim_scripttype_addonfunction},
    {"sim1.customizationscriptattribute_activeduringsimulation", sim_customizationscriptattribute_activeduringsimulation},
    {"sim1.childscriptattribute_automaticcascadingcalls", sim_childscriptattribute_automaticcascadingcalls},
    {"sim1.customizationscriptattribute_cleanupbeforesave", sim_customizationscriptattribute_cleanupbeforesave},
    {"sim1.scriptattribute_debuglevel", sim_scriptattribute_debuglevel},
    {"sim1.script_no_error", sim_script_no_error},
    {"sim1.script_main_script_nonexistent", sim_script_main_script_nonexistent},
    {"sim1.script_main_not_called", sim_script_main_script_not_called},
    {"sim1.script_reentrance_error", sim_script_reentrance_error},
    {"sim1.script_lua_error", sim_script_lua_error},
    {"sim1.script_call_error", sim_script_call_error},
    {"sim1.api_error_report", sim_api_error_report},
    {"sim1.api_error_output", sim_api_error_output},
    {"sim1.api_warning_output", sim_api_warning_output},
    {"sim1.boolparam_rml2_available", sim_boolparam_rml2_available},
    {"sim1.boolparam_rml4_available", sim_boolparam_rml4_available},
    {"sim1.rml_phase_sync_if_possible", sim_ruckig_phasesync},
    {"sim1.rml_only_time_sync", sim_ruckig_timesync},
    {"sim1.rml_only_phase_sync", sim_ruckig_timesync},
    {"sim1.rml_no_sync", sim_ruckig_nosync},
    {"sim1.rml_keep_target_vel", 0},
    {"sim1.rml_recompute_trajectory", 0},
    {"sim1.rml_disable_extremum_motion_states_calc", 0},
    {"sim1.rml_keep_current_vel_if_fallback_strategy", 0},
    {"sim1.filedlg_type_load", sim_filedlg_type_load},
    {"sim1.filedlg_type_save", sim_filedlg_type_save},
    {"sim1.filedlg_type_load_multiple", sim_filedlg_type_load_multiple},
    {"sim1.filedlg_type_folder", sim_filedlg_type_folder},
    {"sim1.msgbox_type_info", sim_msgbox_type_info},
    {"sim1.msgbox_type_question", sim_msgbox_type_question},
    {"sim1.msgbox_type_warning", sim_msgbox_type_warning},
    {"sim1.msgbox_type_critical", sim_msgbox_type_critical},
    {"sim1.msgbox_buttons_ok", sim_msgbox_buttons_ok},
    {"sim1.msgbox_buttons_yesno", sim_msgbox_buttons_yesno},
    {"sim1.msgbox_buttons_yesnocancel", sim_msgbox_buttons_yesnocancel},
    {"sim1.msgbox_buttons_okcancel", sim_msgbox_buttons_okcancel},
    {"sim1.msgbox_return_cancel", sim_msgbox_return_cancel},
    {"sim1.msgbox_return_no", sim_msgbox_return_no},
    {"sim1.msgbox_return_yes", sim_msgbox_return_yes},
    {"sim1.msgbox_return_ok", sim_msgbox_return_ok},
    {"sim1.msgbox_return_error", sim_msgbox_return_error},
    {"sim1.dlgstyle_message", sim_dlgstyle_message},
    {"sim1.dlgstyle_input", sim_dlgstyle_input},
    {"sim1.dlgstyle_ok", sim_dlgstyle_ok},
    {"sim1.dlgstyle_ok_cancel", sim_dlgstyle_ok_cancel},
    {"sim1.dlgstyle_yes_no", sim_dlgstyle_yes_no},
    {"sim1.dlgstyle_dont_center", sim_dlgstyle_dont_center},
    {"sim1.dlgret_still_open", sim_dlgret_still_open},
    {"sim1.dlgret_ok", sim_dlgret_ok},
    {"sim1.dlgret_cancel", sim_dlgret_cancel},
    {"sim1.dlgret_yes", sim_dlgret_yes},
    {"sim1.dlgret_no", sim_dlgret_no},
    {"sim1.scriptattribute_executionorder", sim_scriptattribute_executionorder},
    {"sim1.scriptattribute_executioncount", sim_scriptattribute_executioncount},
    {"sim1.childscriptattribute_enabled", sim_childscriptattribute_enabled},
    {"sim1.scriptattribute_enabled", sim_scriptattribute_enabled},
    {"sim1.scriptattribute_scripttype", sim_scriptattribute_scripttype},
    {"sim1.scriptattribute_scripthandle", sim_scriptattribute_scripthandle},
    {"sim1.drawing_trianglepoints", sim_drawing_trianglepoints},
    {"sim1.drawing_quadpoints", sim_drawing_quadpoints},
    {"sim1.drawing_discpoints", sim_drawing_discpoints},
    {"sim1.drawing_cubepoints", sim_drawing_cubepoints},
    {"sim1.drawing_spherepoints", sim_drawing_spherepoints},
    {"sim1.pure_primitive_none", sim_primitiveshape_none},
    {"sim1.pure_primitive_plane", sim_primitiveshape_plane},
    {"sim1.pure_primitive_disc", sim_primitiveshape_disc},
    {"sim1.pure_primitive_cuboid", sim_primitiveshape_cuboid},
    {"sim1.pure_primitive_spheroid", sim_primitiveshape_spheroid},
    {"sim1.pure_primitive_cylinder", sim_primitiveshape_cylinder},
    {"sim1.pure_primitive_cone", sim_primitiveshape_cone},
    {"sim1.pure_primitive_heightfield", sim_primitiveshape_heightfield},
    {"sim1.jointmode_passive", sim_jointmode_kinematic},
    {"sim1.jointmode_force", sim_jointmode_dynamic},
    {"sim1.dummy_linktype_dynamics_loop_closure", sim_dummytype_dynloopclosure},
    {"sim1.dummylink_dynloopclosure", sim_dummytype_dynloopclosure},
    {"sim1.dummylink_dyntendon", sim_dummytype_dyntendon},
    {"sim1.bullet_global_stepsize", sim_bullet_global_stepsize},
    {"sim1.ode_global_stepsize", sim_ode_global_stepsize},
    {"sim1.vortex_global_stepsize", sim_vortex_global_stepsize},
    {"sim1.newton_global_stepsize", sim_newton_global_stepsize},
    {"sim1.jointfloatparam_pid_p", sim_jointfloatparam_pid_p},
    {"sim1.jointfloatparam_pid_i", sim_jointfloatparam_pid_i},
    {"sim1.jointfloatparam_pid_d", sim_jointfloatparam_pid_d},
    {"sim1.jointfloatparam_upper_limit", sim_jointfloatparam_upper_limit},
    {"sim1.intparam_simulation_warning_disabled_mask", sim_intparam_simulation_warning_disabled_mask},
    {"sim1.syscb_contactcallback", sim_syscb_contactcallback},
    {"sim1.syscb_dyncallback", sim_syscb_dyncallback},
    {"sim1.syscb_customcallback1", sim_syscb_customcallback1},
    {"sim1.syscb_customcallback2", sim_syscb_customcallback2},
    {"sim1.syscb_customcallback3", sim_syscb_customcallback3},
    {"sim1.syscb_customcallback4", sim_syscb_customcallback4},
    {"sim1.syscb_jointcallback", sim_syscb_jointcallback},
    {"sim1.navigation_cameratilt", sim_navigation_cameratilt},
    {"sim1.boolparam_hierarchy_visible", sim_boolparam_hierarchy_visible},
    {"sim1.boolparam_console_visible", sim_boolparam_console_visible},
    {"sim1.boolparam_dynamics_handling_enabled", sim_boolparam_dynamics_handling_enabled},
    {"sim1.boolparam_browser_visible", sim_boolparam_browser_visible},
    {"sim1.boolparam_shape_textures_are_visible", sim_boolparam_shape_textures_are_visible},
    {"sim1.boolparam_display_enabled", sim_boolparam_display_enabled},
    {"sim1.boolparam_infotext_visible", sim_boolparam_infotext_visible},
    {"sim1.boolparam_statustext_open", sim_boolparam_statustext_open},
    {"sim1.boolparam_fog_enabled", sim_boolparam_fog_enabled},
    {"sim1.boolparam_mirrors_enabled", sim_boolparam_mirrors_enabled},
    {"sim1.boolparam_aux_clip_planes_enabled", sim_boolparam_aux_clip_planes_enabled},
    {"sim1.boolparam_full_model_copy_from_api", sim_boolparam_reserved3},
    {"sim1.boolparam_realtime_simulation", sim_boolparam_realtime_simulation},
    {"sim1.boolparam_scene_closing", sim_boolparam_scene_closing},
    {"sim1.boolparam_use_glfinish_cmd", sim_boolparam_use_glfinish_cmd},
    {"sim1.boolparam_video_recording_triggered", sim_boolparam_video_recording_triggered},
    {"sim1.boolparam_fullscreen", sim_boolparam_fullscreen},
    {"sim1.boolparam_headless", sim_boolparam_headless},
    {"sim1.boolparam_cansave", sim_boolparam_cansave},
    {"sim1.boolparam_usingscriptobjects", sim_boolparam_usingscriptobjects},
    {"sim1.boolparam_hierarchy_toolbarbutton_enabled", sim_boolparam_hierarchy_toolbarbutton_enabled},
    {"sim1.boolparam_browser_toolbarbutton_enabled", sim_boolparam_browser_toolbarbutton_enabled},
    {"sim1.boolparam_objectshift_toolbarbutton_enabled", sim_boolparam_objectshift_toolbarbutton_enabled},
    {"sim1.boolparam_objectrotate_toolbarbutton_enabled", sim_boolparam_objectrotate_toolbarbutton_enabled},
    {"sim1.boolparam_force_calcstruct_all_visible", sim_boolparam_force_calcstruct_all_visible},
    {"sim1.boolparam_force_calcstruct_all", sim_boolparam_force_calcstruct_all},
    {"sim1.boolparam_exit_request", sim_boolparam_exit_request},
    {"sim1.boolparam_play_toolbarbutton_enabled", sim_boolparam_play_toolbarbutton_enabled},
    {"sim1.boolparam_pause_toolbarbutton_enabled", sim_boolparam_pause_toolbarbutton_enabled},
    {"sim1.boolparam_stop_toolbarbutton_enabled", sim_boolparam_stop_toolbarbutton_enabled},
    {"sim1.boolparam_waiting_for_trigger", sim_boolparam_waiting_for_trigger},
    {"sim1.boolparam_objproperties_toolbarbutton_enabled", sim_boolparam_objproperties_toolbarbutton_enabled},
    {"sim1.boolparam_calcmodules_toolbarbutton_enabled", sim_boolparam_calcmodules_toolbarbutton_enabled},
    {"sim1.boolparam_rosinterface_donotrunmainscript", sim_boolparam_rosinterface_donotrunmainscript},
    {"sim1.boolparam_rayvalid", sim_boolparam_rayvalid},
    {"sim1.boolparam_execunsafe", sim_boolparam_execunsafe},
    {"sim1.boolparam_execunsafeext", sim_boolparam_execunsafeext},
    {"sim1.intparam_error_report_mode", sim_intparam_error_report_mode},
    {"sim1.intparam_program_version", sim_intparam_program_version},
    {"sim1.intparam_program_full_version", sim_intparam_program_full_version},
    {"sim1.intparam_verbosity", sim_intparam_verbosity},
    {"sim1.intparam_statusbarverbosity", sim_intparam_statusbarverbosity},
    {"sim1.intparam_dlgverbosity", sim_intparam_dlgverbosity},
    {"sim1.intparam_exitcode", sim_intparam_exitcode},
    {"sim1.intparam_videoencoderindex", sim_intparam_videoencoder_index},
    {"sim1.intparam_compilation_version", sim_intparam_compilation_version},
    {"sim1.intparam_current_page", sim_intparam_current_page},
    {"sim1.intparam_flymode_camera_handle", sim_intparam_flymode_camera_handle},
    {"sim1.intparam_dynamic_step_divider", sim_intparam_dynamic_step_divider},
    {"sim1.intparam_dynamic_engine", sim_intparam_dynamic_engine},
    {"sim1.intparam_server_port_start", sim_intparam_server_port_start},
    {"sim1.intparam_server_port_range", sim_intparam_server_port_range},
    {"sim1.intparam_server_port_next", sim_intparam_server_port_next},
    {"sim1.intparam_visible_layers", sim_intparam_visible_layers},
    {"sim1.intparam_infotext_style", sim_intparam_infotext_style},
    {"sim1.intparam_settings", sim_intparam_settings},
    {"sim1.intparam_qt_version", sim_intparam_qt_version},
    {"sim1.intparam_platform", sim_intparam_platform},
    {"sim1.intparam_scene_unique_id", sim_intparam_scene_unique_id},
    {"sim1.intparam_edit_mode_type", sim_intparam_edit_mode_type},
    {"sim1.intparam_work_thread_count", sim_intparam_work_thread_count}, // deprecated
    {"sim1.intparam_mouse_x", sim_intparam_mouse_x},
    {"sim1.intparam_mouse_y", sim_intparam_mouse_y},
    {"sim1.intparam_core_count", sim_intparam_core_count},
    {"sim1.intparam_work_thread_calc_time_ms", sim_intparam_work_thread_calc_time_ms}, // deprecated
    {"sim1.intparam_idle_fps", sim_intparam_idle_fps},
    {"sim1.intparam_prox_sensor_select_down", sim_intparam_prox_sensor_select_down},
    {"sim1.intparam_prox_sensor_select_up", sim_intparam_prox_sensor_select_up},
    {"sim1.intparam_stop_request_counter", sim_intparam_stop_request_counter},
    {"sim1.intparam_program_revision", sim_intparam_program_revision},
    {"sim1.intparam_mouse_buttons", sim_intparam_mouse_buttons},
    {"sim1.intparam_dynamic_warning_disabled_mask", sim_intparam_dynamic_warning_disabled_mask},
    {"sim1.intparam_scene_index", sim_intparam_scene_index},
    {"sim1.intparam_motionplanning_seed", sim_intparam_motionplanning_seed},
    {"sim1.intparam_speedmodifier", sim_intparam_speedmodifier},
    {"sim1.intparam_dynamic_iteration_count", sim_intparam_dynamic_iteration_count},
    {"sim1.intparam_mouseclickcounterdown", sim_intparam_mouseclickcounterdown},
    {"sim1.intparam_mouseclickcounterup", sim_intparam_mouseclickcounterup},
    {"sim1.intparam_objectcreationcounter", sim_intparam_objectcreationcounter},
    {"sim1.intparam_objectdestructioncounter", sim_intparam_objectdestructioncounter},
    {"sim1.intparam_hierarchychangecounter", sim_intparam_hierarchychangecounter},
    {"sim1.intparam_notifydeprecated", sim_intparam_notifydeprecated},
    {"sim1.intparam_processid", sim_intparam_processid},
    {"sim1.intparam_processcnt", sim_intparam_processcnt},
    {"sim1.floatparam_rand", sim_floatparam_rand},
    {"sim1.floatparam_simulation_time_step", sim_floatparam_simulation_time_step},
    {"sim1.floatparam_stereo_distance", sim_floatparam_stereo_distance},
    {"sim1.floatparam_dynamic_step_size", sim_floatparam_dynamic_step_size},
    {"sim1.floatparam_mouse_wheel_zoom_factor", sim_floatparam_mouse_wheel_zoom_factor},
    {"sim1.floatparam_physicstimestep", sim_floatparam_physicstimestep},
    {"sim1.floatparam_maxtrisizeabs", sim_floatparam_maxtrisizeabs},
    {"sim1.floatparam_mintrisizerel", sim_floatparam_mintrisizerel},
    {"sim1.arrayparam_gravity", sim_arrayparam_gravity},
    {"sim1.arrayparam_fog", sim_arrayparam_fog},
    {"sim1.arrayparam_fog_color", sim_arrayparam_fog_color},
    {"sim1.arrayparam_background_color1", sim_arrayparam_background_color1},
    {"sim1.arrayparam_background_color2", sim_arrayparam_background_color2},
    {"sim1.arrayparam_ambient_light", sim_arrayparam_ambient_light},
    {"sim1.arrayparam_random_euler", sim_arrayparam_random_euler},
    {"sim1.arrayparam_rayorigin", sim_arrayparam_rayorigin},
    {"sim1.arrayparam_raydirection", sim_arrayparam_raydirection},
    {"sim1.stringparam_application_path", sim_stringparam_application_path},
    {"sim1.stringparam_video_filename", sim_stringparam_video_filename},
    {"sim1.stringparam_app_arg1", sim_stringparam_app_arg1},
    {"sim1.stringparam_app_arg2", sim_stringparam_app_arg2},
    {"sim1.stringparam_app_arg3", sim_stringparam_app_arg3},
    {"sim1.stringparam_app_arg4", sim_stringparam_app_arg4},
    {"sim1.stringparam_app_arg5", sim_stringparam_app_arg5},
    {"sim1.stringparam_app_arg6", sim_stringparam_app_arg6},
    {"sim1.stringparam_app_arg7", sim_stringparam_app_arg7},
    {"sim1.stringparam_app_arg8", sim_stringparam_app_arg8},
    {"sim1.stringparam_app_arg9", sim_stringparam_app_arg9},
    {"sim1.stringparam_scene_path_and_name", sim_stringparam_scene_path_and_name},
    {"sim1.stringparam_remoteapi_temp_file_dir", sim_stringparam_remoteapi_temp_file_dir},
    {"sim1.stringparam_scene_path", sim_stringparam_scene_path},
    {"sim1.stringparam_scene_name", sim_stringparam_scene_name},
    {"sim1.stringparam_scene_unique_id", sim_stringparam_scene_unique_id},
    {"sim1.stringparam_machine_id", sim_stringparam_machine_id},
    {"sim1.stringparam_machine_id_legacy", sim_stringparam_machine_id_legacy},
    {"sim1.stringparam_legacymachinetag", sim_stringparam_legacymachinetag},
    {"sim1.stringparam_verbosity", sim_stringparam_verbosity},
    {"sim1.stringparam_statusbarverbosity", sim_stringparam_statusbarverbosity},
    {"sim1.stringparam_dlgverbosity", sim_stringparam_dlgverbosity},
    {"sim1.stringparam_logfilter", sim_stringparam_consolelogfilter},
    {"sim1.stringparam_uniqueid", sim_stringparam_uniqueid},
    {"sim1.stringparam_tempdir", sim_stringparam_tempdir},
    {"sim1.stringparam_tempscenedir", sim_stringparam_tempscenedir},
    {"sim1.stringparam_datadir", sim_stringparam_datadir},
    {"sim1.stringparam_importexportdir", sim_stringparam_importexportdir},
    {"sim1.stringparam_addonpath", sim_stringparam_addonpath},
    {"sim1.stringparam_scenedefaultdir", sim_stringparam_scenedefaultdir},
    {"sim1.stringparam_modeldefaultdir", sim_stringparam_modeldefaultdir},
    {"sim1.stringparam_defaultpython", sim_stringparam_defaultpython},
    {"sim1.stringparam_additionalpythonpath", sim_stringparam_additionalpythonpath},
    {"sim1.stringparam_luadir", sim_stringparam_luadir},
    {"sim1.stringparam_pythondir", sim_stringparam_pythondir},
    {"sim1.stringparam_mujocodir", sim_stringparam_mujocodir},
    {"sim1.stringparam_usersettingsdir", sim_stringparam_usersettingsdir},
    {"sim1.stringparam_systemdir", sim_stringparam_systemdir},
    {"sim1.stringparam_resourcesdir", sim_stringparam_resourcesdir},
    {"sim1.stringparam_addondir", sim_stringparam_addondir},
    {"sim1.stringparam_sandboxlang", sim_stringparam_sandboxlang},
    {"sim1.objintparam_hierarchycolor", sim_objintparam_hierarchycolor},
    {"sim1.objintparam_visibility_layer", sim_objintparam_visibility_layer},
    {"sim1.objfloatparam_abs_x_velocity", sim_objfloatparam_abs_x_velocity},
    {"sim1.objfloatparam_abs_y_velocity", sim_objfloatparam_abs_y_velocity},
    {"sim1.objfloatparam_abs_z_velocity", sim_objfloatparam_abs_z_velocity},
    {"sim1.objfloatparam_abs_rot_velocity", sim_objfloatparam_abs_rot_velocity},
    {"sim1.objfloatparam_objbbox_min_x", sim_objfloatparam_objbbox_min_x},
    {"sim1.objfloatparam_objbbox_min_y", sim_objfloatparam_objbbox_min_y},
    {"sim1.objfloatparam_objbbox_min_z", sim_objfloatparam_objbbox_min_z},
    {"sim1.objfloatparam_objbbox_max_x", sim_objfloatparam_objbbox_max_x},
    {"sim1.objfloatparam_objbbox_max_y", sim_objfloatparam_objbbox_max_y},
    {"sim1.objfloatparam_objbbox_max_z", sim_objfloatparam_objbbox_max_z},
    {"sim1.objfloatparam_modelbbox_min_x", sim_objfloatparam_modelbbox_min_x},
    {"sim1.objfloatparam_modelbbox_min_y", sim_objfloatparam_modelbbox_min_y},
    {"sim1.objfloatparam_modelbbox_min_z", sim_objfloatparam_modelbbox_min_z},
    {"sim1.objfloatparam_modelbbox_max_x", sim_objfloatparam_modelbbox_max_x},
    {"sim1.objfloatparam_modelbbox_max_y", sim_objfloatparam_modelbbox_max_y},
    {"sim1.objfloatparam_modelbbox_max_z", sim_objfloatparam_modelbbox_max_z},
    {"sim1.objintparam_collection_self_collision_indicator", sim_objintparam_collection_self_collision_indicator},
    {"sim1.objfloatparam_transparency_offset", sim_objfloatparam_transparency_offset},
    {"sim1.objfloatparam_size_factor", sim_objfloatparam_size_factor},
    {"sim1.objintparam_child_role", sim_objintparam_child_role},
    {"sim1.objintparam_parent_role", sim_objintparam_parent_role},
    {"sim1.objintparam_manipulation_permissions", sim_objintparam_manipulation_permissions},
    {"sim1.objintparam_illumination_handle", sim_objintparam_illumination_handle},
    {"sim1.objstringparam_dna", sim_objstringparam_dna},
    {"sim1.objstringparam_unique_id", sim_objstringparam_unique_id},
    {"sim1.objintparam_visible", sim_objintparam_visible},
    {"sim1.objintparam_unique_id", sim_objintparam_unique_id},
    {"sim1.visionfloatparam_near_clipping", sim_visionfloatparam_near_clipping},
    {"sim1.visionfloatparam_far_clipping", sim_visionfloatparam_far_clipping},
    {"sim1.visionintparam_resolution_x", sim_visionintparam_resolution_x},
    {"sim1.visionintparam_resolution_y", sim_visionintparam_resolution_y},
    {"sim1.visionfloatparam_perspective_angle", sim_visionfloatparam_perspective_angle},
    {"sim1.visionfloatparam_ortho_size", sim_visionfloatparam_ortho_size},
    {"sim1.visionintparam_disabled_light_components", sim_visionintparam_disabled_light_components},
    {"sim1.visionintparam_rendering_attributes", sim_visionintparam_rendering_attributes},
    {"sim1.visionintparam_entity_to_render", sim_visionintparam_entity_to_render},
    {"sim1.visionintparam_windowed_size_x", sim_visionintparam_windowed_size_x},
    {"sim1.visionintparam_windowed_size_y", sim_visionintparam_windowed_size_y},
    {"sim1.visionintparam_windowed_pos_x", sim_visionintparam_windowed_pos_x},
    {"sim1.visionintparam_windowed_pos_y", sim_visionintparam_windowed_pos_y},
    {"sim1.visionintparam_pov_focal_blur", sim_visionintparam_pov_focal_blur},
    {"sim1.visionfloatparam_pov_blur_distance", sim_visionfloatparam_pov_blur_distance},
    {"sim1.visionfloatparam_pov_aperture", sim_visionfloatparam_pov_aperture},
    {"sim1.visionintparam_pov_blur_sampled", sim_visionintparam_pov_blur_sampled},
    {"sim1.visionintparam_render_mode", sim_visionintparam_render_mode},
    {"sim1.visionintparam_perspective_operation", sim_visionintparam_perspective_operation},
    {"sim1.visionfarrayparam_viewfrustum", sim_visionfarrayparam_viewfrustum},
    {"sim1.visionintparam_rgbignored", sim_visionintparam_rgbignored},
    {"sim1.visionintparam_depthignored", sim_visionintparam_depthignored},
    {"sim1.jointintparam_motor_enabled", sim_jointintparam_motor_enabled},
    {"sim1.jointintparam_ctrl_enabled", sim_jointintparam_ctrl_enabled},
    {"sim1.jointfloatparam_intrinsic_x", sim_jointfloatparam_intrinsic_x},
    {"sim1.jointfloatparam_intrinsic_y", sim_jointfloatparam_intrinsic_y},
    {"sim1.jointfloatparam_intrinsic_z", sim_jointfloatparam_intrinsic_z},
    {"sim1.jointfloatparam_intrinsic_qx", sim_jointfloatparam_intrinsic_qx},
    {"sim1.jointfloatparam_intrinsic_qy", sim_jointfloatparam_intrinsic_qy},
    {"sim1.jointfloatparam_intrinsic_qz", sim_jointfloatparam_intrinsic_qz},
    {"sim1.jointfloatparam_intrinsic_qw", sim_jointfloatparam_intrinsic_qw},
    {"sim1.jointfloatparam_velocity", sim_jointfloatparam_velocity},
    {"sim1.jointfloatparam_spherical_qx", sim_jointfloatparam_spherical_qx},
    {"sim1.jointfloatparam_spherical_qy", sim_jointfloatparam_spherical_qy},
    {"sim1.jointfloatparam_spherical_qz", sim_jointfloatparam_spherical_qz},
    {"sim1.jointfloatparam_spherical_qw", sim_jointfloatparam_spherical_qw},
    {"sim1.jointfloatparam_kc_k", sim_jointfloatparam_kc_k},
    {"sim1.jointfloatparam_kc_c", sim_jointfloatparam_kc_c},
    {"sim1.jointfloatparam_error_x", sim_jointfloatparam_error_x},
    {"sim1.jointfloatparam_error_y", sim_jointfloatparam_error_y},
    {"sim1.jointfloatparam_error_z", sim_jointfloatparam_error_z},
    {"sim1.jointfloatparam_error_a", sim_jointfloatparam_error_a},
    {"sim1.jointfloatparam_error_b", sim_jointfloatparam_error_b},
    {"sim1.jointfloatparam_error_g", sim_jointfloatparam_error_g},
    {"sim1.jointfloatparam_error_pos", sim_jointfloatparam_error_pos},
    {"sim1.jointfloatparam_error_angle", sim_jointfloatparam_error_angle},
    {"sim1.jointintparam_velocity_lock", sim_jointintparam_velocity_lock},
    {"sim1.jointintparam_vortex_dep_handle", sim_jointintparam_vortex_dep_handle},
    {"sim1.jointfloatparam_vortex_dep_multiplication", sim_jointfloatparam_vortex_dep_multiplication},
    {"sim1.jointfloatparam_vortex_dep_offset", sim_jointfloatparam_vortex_dep_offset},
    {"sim1.jointfloatparam_screwlead", sim_jointfloatparam_screwlead},
    {"sim1.jointfloatparam_maxvel", sim_jointfloatparam_maxvel},
    {"sim1.jointfloatparam_maxaccel", sim_jointfloatparam_maxaccel},
    {"sim1.jointfloatparam_maxjerk", sim_jointfloatparam_maxjerk},
    {"sim1.jointintparam_dynctrlmode", sim_jointintparam_dynctrlmode},
    {"sim1.jointintparam_dynvelctrltype", sim_jointintparam_dynvelctrltype},
    {"sim1.jointintparam_dynposctrltype", sim_jointintparam_dynposctrltype},
    {"sim1.shapefloatparam_init_velocity_x", sim_shapefloatparam_init_velocity_x},
    {"sim1.shapefloatparam_init_velocity_y", sim_shapefloatparam_init_velocity_y},
    {"sim1.shapefloatparam_init_velocity_z", sim_shapefloatparam_init_velocity_z},
    {"sim1.shapeintparam_static", sim_shapeintparam_static},
    {"sim1.shapeintparam_kinematic", sim_shapeintparam_kinematic},
    {"sim1.shapeintparam_respondable", sim_shapeintparam_respondable},
    {"sim1.shapefloatparam_mass", sim_shapefloatparam_mass},
    {"sim1.shapefloatparam_texture_x", sim_shapefloatparam_texture_x},
    {"sim1.shapefloatparam_texture_y", sim_shapefloatparam_texture_y},
    {"sim1.shapefloatparam_texture_z", sim_shapefloatparam_texture_z},
    {"sim1.shapefloatparam_texture_a", sim_shapefloatparam_texture_a},
    {"sim1.shapefloatparam_texture_b", sim_shapefloatparam_texture_b},
    {"sim1.shapefloatparam_texture_g", sim_shapefloatparam_texture_g},
    {"sim1.shapefloatparam_texture_scaling_x", sim_shapefloatparam_texture_scaling_x},
    {"sim1.shapefloatparam_texture_scaling_y", sim_shapefloatparam_texture_scaling_y},
    {"sim1.shapeintparam_culling", sim_shapeintparam_culling},
    {"sim1.shapeintparam_wireframe", sim_shapeintparam_wireframe},
    {"sim1.shapeintparam_compound", sim_shapeintparam_compound},
    {"sim1.shapeintparam_convex", sim_shapeintparam_convex},
    {"sim1.shapeintparam_convex_check", sim_shapeintparam_convex_check},
    {"sim1.shapeintparam_respondable_mask", sim_shapeintparam_respondable_mask},
    {"sim1.shapefloatparam_init_ang_velocity_x", sim_shapefloatparam_init_ang_velocity_x},
    {"sim1.shapefloatparam_init_ang_velocity_y", sim_shapefloatparam_init_ang_velocity_y},
    {"sim1.shapefloatparam_init_ang_velocity_z", sim_shapefloatparam_init_ang_velocity_z},
    {"sim1.shapefloatparam_init_velocity_a", sim_shapefloatparam_init_ang_velocity_x},
    {"sim1.shapefloatparam_init_velocity_b", sim_shapefloatparam_init_ang_velocity_y},
    {"sim1.shapefloatparam_init_velocity_g", sim_shapefloatparam_init_ang_velocity_z},
    {"sim1.shapestringparam_colorname", sim_shapestringparam_colorname},
    {"sim1.shapeintparam_edge_visibility", sim_shapeintparam_edge_visibility},
    {"sim1.shapefloatparam_shading_angle", sim_shapefloatparam_shading_angle},
    {"sim1.shapefloatparam_edge_angle", sim_shapefloatparam_edge_angle},
    {"sim1.shapeintparam_edge_borders_hidden", sim_shapeintparam_edge_borders_hidden},
    {"sim1.shapeintparam_component_cnt", sim_shapeintparam_component_cnt},
    {"sim1.shapeintparam_sleepmodestart", sim_shapeintparam_sleepmodestart},
    {"sim1.shapeintparam_respondablesuspendcnt", sim_shapeintparam_respondablesuspendcnt},
    {"sim1.proxintparam_ray_invisibility", sim_proxintparam_ray_invisibility},
    {"sim1.proxintparam_volume_type", sim_proxintparam_volume_type},
    {"sim1.proxintparam_entity_to_detect", sim_proxintparam_entity_to_detect},
    {"sim1.forcefloatparam_error_x", sim_forcefloatparam_error_x},
    {"sim1.forcefloatparam_error_y", sim_forcefloatparam_error_y},
    {"sim1.forcefloatparam_error_z", sim_forcefloatparam_error_z},
    {"sim1.forcefloatparam_error_a", sim_forcefloatparam_error_a},
    {"sim1.forcefloatparam_error_b", sim_forcefloatparam_error_b},
    {"sim1.forcefloatparam_error_g", sim_forcefloatparam_error_g},
    {"sim1.forcefloatparam_error_pos", sim_forcefloatparam_error_pos},
    {"sim1.forcefloatparam_error_angle", sim_forcefloatparam_error_angle},
    {"sim1.lightintparam_pov_casts_shadows", sim_lightintparam_pov_casts_shadows},
    {"sim1.lightfloatparam_spot_exponent", sim_lightfloatparam_spot_exponent},
    {"sim1.lightfloatparam_spot_cutoff", sim_lightfloatparam_spot_cutoff},
    {"sim1.lightfloatparam_const_attenuation", sim_lightfloatparam_const_attenuation},
    {"sim1.lightfloatparam_lin_attenuation", sim_lightfloatparam_lin_attenuation},
    {"sim1.lightfloatparam_quad_attenuation", sim_lightfloatparam_quad_attenuation},
    {"sim1.camerafloatparam_near_clipping", sim_camerafloatparam_near_clipping},
    {"sim1.camerafloatparam_far_clipping", sim_camerafloatparam_far_clipping},
    {"sim1.cameraintparam_disabled_light_components", sim_cameraintparam_disabled_light_components},
    {"sim1.camerafloatparam_perspective_angle", sim_camerafloatparam_perspective_angle},
    {"sim1.camerafloatparam_ortho_size", sim_camerafloatparam_ortho_size},
    {"sim1.cameraintparam_rendering_attributes", sim_cameraintparam_rendering_attributes},
    {"sim1.cameraintparam_pov_focal_blur", sim_cameraintparam_pov_focal_blur},
    {"sim1.camerafloatparam_pov_blur_distance", sim_camerafloatparam_pov_blur_distance},
    {"sim1.camerafloatparam_pov_aperture", sim_camerafloatparam_pov_aperture},
    {"sim1.cameraintparam_pov_blur_samples", sim_cameraintparam_pov_blur_samples},
    {"sim1.cameraintparam_perspective_operation", sim_cameraintparam_perspective_operation},
    {"sim1.cameraintparam_trackedobject", sim_cameraintparam_trackedobject},
    {"sim1.camerafarrayparam_viewfrustum", sim_camerafarrayparam_viewfrustum},
    {"sim1.dummyintparam_dummytype", sim_dummyintparam_dummytype},
    {"sim1.dummyfloatparam_size", sim_dummyfloatparam_size},
    {"sim1.dummystringparam_assemblytag", sim_dummystringparam_assemblytag},
    {"sim1.graphintparam_needs_refresh", sim_graphintparam_needs_refresh},
    {"sim1.millintparam_volume_type", sim_millintparam_volume_type},
    {"sim1.mirrorfloatparam_width", sim_mirrorfloatparam_width},
    {"sim1.mirrorfloatparam_height", sim_mirrorfloatparam_height},
    {"sim1.mirrorfloatparam_reflectance", sim_mirrorfloatparam_reflectance},
    {"sim1.mirrorintparam_enable", sim_mirrorintparam_enable},
    {"sim1.octreefloatparam_voxelsize", sim_octreefloatparam_voxelsize},
    {"sim1.bullet_global_internalscalingfactor", sim_bullet_global_internalscalingfactor},
    {"sim1.bullet_global_collisionmarginfactor", sim_bullet_global_collisionmarginfactor},
    {"sim1.bullet_global_constraintsolvingiterations", sim_bullet_global_constraintsolvingiterations},
    {"sim1.bullet_global_bitcoded", sim_bullet_global_bitcoded},
    {"sim1.bullet_global_constraintsolvertype", sim_bullet_global_constraintsolvertype},
    {"sim1.bullet_global_fullinternalscaling", sim_bullet_global_fullinternalscaling},
    {"sim1.bullet_global_computeinertias", sim_bullet_global_computeinertias},
    {"sim1.bullet_joint_stoperp", sim_bullet_joint_stoperp},
    {"sim1.bullet_joint_stopcfm", sim_bullet_joint_stopcfm},
    {"sim1.bullet_joint_normalcfm", sim_bullet_joint_normalcfm},
    {"sim1.bullet_joint_pospid1", sim_bullet_joint_pospid1},
    {"sim1.bullet_joint_pospid2", sim_bullet_joint_pospid2},
    {"sim1.bullet_joint_pospid3", sim_bullet_joint_pospid3},
    {"sim1.bullet_body_restitution", sim_bullet_body_restitution},
    {"sim1.bullet_body_oldfriction", sim_bullet_body_oldfriction},
    {"sim1.bullet_body_friction", sim_bullet_body_friction},
    {"sim1.bullet_body_lineardamping", sim_bullet_body_lineardamping},
    {"sim1.bullet_body_angulardamping", sim_bullet_body_angulardamping},
    {"sim1.bullet_body_nondefaultcollisionmargingfactor", sim_bullet_body_nondefaultcollisionmargingfactor},
    {"sim1.bullet_body_nondefaultcollisionmargingfactorconvex", sim_bullet_body_nondefaultcollisionmargingfactorconvex},
    {"sim1.bullet_body_bitcoded", sim_bullet_body_bitcoded},
    {"sim1.bullet_body_sticky", sim_bullet_body_sticky},
    {"sim1.bullet_body_usenondefaultcollisionmargin", sim_bullet_body_usenondefaultcollisionmargin},
    {"sim1.bullet_body_usenondefaultcollisionmarginconvex", sim_bullet_body_usenondefaultcollisionmarginconvex},
    {"sim1.bullet_body_autoshrinkconvex", sim_bullet_body_autoshrinkconvex},
    {"sim1.ode_global_internalscalingfactor", sim_ode_global_internalscalingfactor},
    {"sim1.ode_global_cfm", sim_ode_global_cfm},
    {"sim1.ode_global_erp", sim_ode_global_erp},
    {"sim1.ode_global_constraintsolvingiterations", sim_ode_global_constraintsolvingiterations},
    {"sim1.ode_global_bitcoded", sim_ode_global_bitcoded},
    {"sim1.ode_global_randomseed", sim_ode_global_randomseed},
    {"sim1.ode_global_fullinternalscaling", sim_ode_global_fullinternalscaling},
    {"sim1.ode_global_quickstep", sim_ode_global_quickstep},
    {"sim1.ode_global_computeinertias", sim_ode_global_computeinertias},
    {"sim1.ode_joint_stoperp", sim_ode_joint_stoperp},
    {"sim1.ode_joint_stopcfm", sim_ode_joint_stopcfm},
    {"sim1.ode_joint_bounce", sim_ode_joint_bounce},
    {"sim1.ode_joint_fudgefactor", sim_ode_joint_fudgefactor},
    {"sim1.ode_joint_normalcfm", sim_ode_joint_normalcfm},
    {"sim1.ode_joint_pospid1", sim_ode_joint_pospid1},
    {"sim1.ode_joint_pospid2", sim_ode_joint_pospid2},
    {"sim1.ode_joint_pospid3", sim_ode_joint_pospid3},
    {"sim1.ode_body_friction", sim_ode_body_friction},
    {"sim1.ode_body_softerp", sim_ode_body_softerp},
    {"sim1.ode_body_softcfm", sim_ode_body_softcfm},
    {"sim1.ode_body_lineardamping", sim_ode_body_lineardamping},
    {"sim1.ode_body_angulardamping", sim_ode_body_angulardamping},
    {"sim1.ode_body_maxcontacts", sim_ode_body_maxcontacts},
    {"sim1.vortex_global_internalscalingfactor", sim_vortex_global_internalscalingfactor},
    {"sim1.vortex_global_contacttolerance", sim_vortex_global_contacttolerance},
    {"sim1.vortex_global_constraintlinearcompliance", sim_vortex_global_constraintlinearcompliance},
    {"sim1.vortex_global_constraintlineardamping", sim_vortex_global_constraintlineardamping},
    {"sim1.vortex_global_constraintlinearkineticloss", sim_vortex_global_constraintlinearkineticloss},
    {"sim1.vortex_global_constraintangularcompliance", sim_vortex_global_constraintangularcompliance},
    {"sim1.vortex_global_constraintangulardamping", sim_vortex_global_constraintangulardamping},
    {"sim1.vortex_global_constraintangularkineticloss", sim_vortex_global_constraintangularkineticloss},
    {"sim1.vortex_global_bitcoded", sim_vortex_global_bitcoded},
    {"sim1.vortex_global_autosleep", sim_vortex_global_autosleep},
    {"sim1.vortex_global_multithreading", sim_vortex_global_multithreading},
    {"sim1.vortex_global_computeinertias", sim_vortex_global_computeinertias},
    {"sim1.vortex_joint_lowerlimitdamping", sim_vortex_joint_lowerlimitdamping},
    {"sim1.vortex_joint_upperlimitdamping", sim_vortex_joint_upperlimitdamping},
    {"sim1.vortex_joint_lowerlimitstiffness", sim_vortex_joint_lowerlimitstiffness},
    {"sim1.vortex_joint_upperlimitstiffness", sim_vortex_joint_upperlimitstiffness},
    {"sim1.vortex_joint_lowerlimitrestitution", sim_vortex_joint_lowerlimitrestitution},
    {"sim1.vortex_joint_upperlimitrestitution", sim_vortex_joint_upperlimitrestitution},
    {"sim1.vortex_joint_lowerlimitmaxforce", sim_vortex_joint_lowerlimitmaxforce},
    {"sim1.vortex_joint_upperlimitmaxforce", sim_vortex_joint_upperlimitmaxforce},
    {"sim1.vortex_joint_motorconstraintfrictioncoeff", sim_vortex_joint_motorconstraintfrictioncoeff},
    {"sim1.vortex_joint_motorconstraintfrictionmaxforce", sim_vortex_joint_motorconstraintfrictionmaxforce},
    {"sim1.vortex_joint_motorconstraintfrictionloss", sim_vortex_joint_motorconstraintfrictionloss},
    {"sim1.vortex_joint_p0loss", sim_vortex_joint_p0loss},
    {"sim1.vortex_joint_p0stiffness", sim_vortex_joint_p0stiffness},
    {"sim1.vortex_joint_p0damping", sim_vortex_joint_p0damping},
    {"sim1.vortex_joint_p0frictioncoeff", sim_vortex_joint_p0frictioncoeff},
    {"sim1.vortex_joint_p0frictionmaxforce", sim_vortex_joint_p0frictionmaxforce},
    {"sim1.vortex_joint_p0frictionloss", sim_vortex_joint_p0frictionloss},
    {"sim1.vortex_joint_p1loss", sim_vortex_joint_p1loss},
    {"sim1.vortex_joint_p1stiffness", sim_vortex_joint_p1stiffness},
    {"sim1.vortex_joint_p1damping", sim_vortex_joint_p1damping},
    {"sim1.vortex_joint_p1frictioncoeff", sim_vortex_joint_p1frictioncoeff},
    {"sim1.vortex_joint_p1frictionmaxforce", sim_vortex_joint_p1frictionmaxforce},
    {"sim1.vortex_joint_p1frictionloss", sim_vortex_joint_p1frictionloss},
    {"sim1.vortex_joint_p2loss", sim_vortex_joint_p2loss},
    {"sim1.vortex_joint_p2stiffness", sim_vortex_joint_p2stiffness},
    {"sim1.vortex_joint_p2damping", sim_vortex_joint_p2damping},
    {"sim1.vortex_joint_p2frictioncoeff", sim_vortex_joint_p2frictioncoeff},
    {"sim1.vortex_joint_p2frictionmaxforce", sim_vortex_joint_p2frictionmaxforce},
    {"sim1.vortex_joint_p2frictionloss", sim_vortex_joint_p2frictionloss},
    {"sim1.vortex_joint_a0loss", sim_vortex_joint_a0loss},
    {"sim1.vortex_joint_a0stiffness", sim_vortex_joint_a0stiffness},
    {"sim1.vortex_joint_a0damping", sim_vortex_joint_a0damping},
    {"sim1.vortex_joint_a0frictioncoeff", sim_vortex_joint_a0frictioncoeff},
    {"sim1.vortex_joint_a0frictionmaxforce", sim_vortex_joint_a0frictionmaxforce},
    {"sim1.vortex_joint_a0frictionloss", sim_vortex_joint_a0frictionloss},
    {"sim1.vortex_joint_a1loss", sim_vortex_joint_a1loss},
    {"sim1.vortex_joint_a1stiffness", sim_vortex_joint_a1stiffness},
    {"sim1.vortex_joint_a1damping", sim_vortex_joint_a1damping},
    {"sim1.vortex_joint_a1frictioncoeff", sim_vortex_joint_a1frictioncoeff},
    {"sim1.vortex_joint_a1frictionmaxforce", sim_vortex_joint_a1frictionmaxforce},
    {"sim1.vortex_joint_a1frictionloss", sim_vortex_joint_a1frictionloss},
    {"sim1.vortex_joint_a2loss", sim_vortex_joint_a2loss},
    {"sim1.vortex_joint_a2stiffness", sim_vortex_joint_a2stiffness},
    {"sim1.vortex_joint_a2damping", sim_vortex_joint_a2damping},
    {"sim1.vortex_joint_a2frictioncoeff", sim_vortex_joint_a2frictioncoeff},
    {"sim1.vortex_joint_a2frictionmaxforce", sim_vortex_joint_a2frictionmaxforce},
    {"sim1.vortex_joint_a2frictionloss", sim_vortex_joint_a2frictionloss},
    {"sim1.vortex_joint_dependencyfactor", sim_vortex_joint_dependencyfactor},
    {"sim1.vortex_joint_dependencyoffset", sim_vortex_joint_dependencyoffset},
    {"sim1.vortex_joint_bitcoded", sim_vortex_joint_bitcoded},
    {"sim1.vortex_joint_relaxationenabledbc", sim_vortex_joint_relaxationenabledbc},
    {"sim1.vortex_joint_frictionenabledbc", sim_vortex_joint_frictionenabledbc},
    {"sim1.vortex_joint_frictionproportionalbc", sim_vortex_joint_frictionproportionalbc},
    {"sim1.vortex_joint_objectid", sim_vortex_joint_objectid},
    {"sim1.vortex_joint_dependentobjectid", sim_vortex_joint_dependentobjectid},
    {"sim1.vortex_joint_motorfrictionenabled", sim_vortex_joint_motorfrictionenabled},
    {"sim1.vortex_joint_proportionalmotorfriction", sim_vortex_joint_proportionalmotorfriction},
    {"sim1.vortex_joint_pospid1", sim_vortex_joint_pospid1},
    {"sim1.vortex_joint_pospid2", sim_vortex_joint_pospid2},
    {"sim1.vortex_joint_pospid3", sim_vortex_joint_pospid3},
    {"sim1.vortex_body_primlinearaxisfriction", sim_vortex_body_primlinearaxisfriction},
    {"sim1.vortex_body_seclinearaxisfriction", sim_vortex_body_seclinearaxisfriction},
    {"sim1.vortex_body_primangularaxisfriction", sim_vortex_body_primangularaxisfriction},
    {"sim1.vortex_body_secangularaxisfriction", sim_vortex_body_secangularaxisfriction},
    {"sim1.vortex_body_normalangularaxisfriction", sim_vortex_body_normalangularaxisfriction},
    {"sim1.vortex_body_primlinearaxisstaticfrictionscale", sim_vortex_body_primlinearaxisstaticfrictionscale},
    {"sim1.vortex_body_seclinearaxisstaticfrictionscale", sim_vortex_body_seclinearaxisstaticfrictionscale},
    {"sim1.vortex_body_primangularaxisstaticfrictionscale", sim_vortex_body_primangularaxisstaticfrictionscale},
    {"sim1.vortex_body_secangularaxisstaticfrictionscale", sim_vortex_body_secangularaxisstaticfrictionscale},
    {"sim1.vortex_body_normalangularaxisstaticfrictionscale", sim_vortex_body_normalangularaxisstaticfrictionscale},
    {"sim1.vortex_body_compliance", sim_vortex_body_compliance},
    {"sim1.vortex_body_damping", sim_vortex_body_damping},
    {"sim1.vortex_body_restitution", sim_vortex_body_restitution},
    {"sim1.vortex_body_restitutionthreshold", sim_vortex_body_restitutionthreshold},
    {"sim1.vortex_body_adhesiveforce", sim_vortex_body_adhesiveforce},
    {"sim1.vortex_body_linearvelocitydamping", sim_vortex_body_linearvelocitydamping},
    {"sim1.vortex_body_angularvelocitydamping", sim_vortex_body_angularvelocitydamping},
    {"sim1.vortex_body_primlinearaxisslide", sim_vortex_body_primlinearaxisslide},
    {"sim1.vortex_body_seclinearaxisslide", sim_vortex_body_seclinearaxisslide},
    {"sim1.vortex_body_primangularaxisslide", sim_vortex_body_primangularaxisslide},
    {"sim1.vortex_body_secangularaxisslide", sim_vortex_body_secangularaxisslide},
    {"sim1.vortex_body_normalangularaxisslide", sim_vortex_body_normalangularaxisslide},
    {"sim1.vortex_body_primlinearaxisslip", sim_vortex_body_primlinearaxisslip},
    {"sim1.vortex_body_seclinearaxisslip", sim_vortex_body_seclinearaxisslip},
    {"sim1.vortex_body_primangularaxisslip", sim_vortex_body_primangularaxisslip},
    {"sim1.vortex_body_secangularaxisslip", sim_vortex_body_secangularaxisslip},
    {"sim1.vortex_body_normalangularaxisslip", sim_vortex_body_normalangularaxisslip},
    {"sim1.vortex_body_autosleeplinearspeedthreshold", sim_vortex_body_autosleeplinearspeedthreshold},
    {"sim1.vortex_body_autosleeplinearaccelthreshold", sim_vortex_body_autosleeplinearaccelthreshold},
    {"sim1.vortex_body_autosleepangularspeedthreshold", sim_vortex_body_autosleepangularspeedthreshold},
    {"sim1.vortex_body_autosleepangularaccelthreshold", sim_vortex_body_autosleepangularaccelthreshold},
    {"sim1.vortex_body_skinthickness", sim_vortex_body_skinthickness},
    {"sim1.vortex_body_autoangulardampingtensionratio", sim_vortex_body_autoangulardampingtensionratio},
    {"sim1.vortex_body_primaxisvectorx", sim_vortex_body_primaxisvectorx},
    {"sim1.vortex_body_primaxisvectory", sim_vortex_body_primaxisvectory},
    {"sim1.vortex_body_primaxisvectorz", sim_vortex_body_primaxisvectorz},
    {"sim1.vortex_body_primlinearaxisfrictionmodel", sim_vortex_body_primlinearaxisfrictionmodel},
    {"sim1.vortex_body_seclinearaxisfrictionmodel", sim_vortex_body_seclinearaxisfrictionmodel},
    {"sim1.vortex_body_primangulararaxisfrictionmodel", sim_vortex_body_primangulararaxisfrictionmodel},
    {"sim1.vortex_body_secmangulararaxisfrictionmodel", sim_vortex_body_secmangulararaxisfrictionmodel}, // wrong spelling
    {"sim1.vortex_body_secangularaxisfrictionmodel", sim_vortex_body_secangularaxisfrictionmodel},
    {"sim1.vortex_body_normalmangulararaxisfrictionmodel", sim_vortex_body_normalmangulararaxisfrictionmodel}, // wrong spelling
    {"sim1.vortex_body_normalangularaxisfrictionmodel", sim_vortex_body_normalangularaxisfrictionmodel},
    {"sim1.vortex_body_bitcoded", sim_vortex_body_bitcoded},
    {"sim1.vortex_body_autosleepsteplivethreshold", sim_vortex_body_autosleepsteplivethreshold},
    {"sim1.vortex_body_materialuniqueid", sim_vortex_body_materialuniqueid},
    {"sim1.vortex_body_pureshapesasconvex", sim_vortex_body_pureshapesasconvex},
    {"sim1.vortex_body_convexshapesasrandom", sim_vortex_body_convexshapesasrandom},
    {"sim1.vortex_body_randomshapesasterrain", sim_vortex_body_randomshapesasterrain},
    {"sim1.vortex_body_fastmoving", sim_vortex_body_fastmoving},
    {"sim1.vortex_body_autoslip", sim_vortex_body_autoslip},
    {"sim1.vortex_body_seclinaxissameasprimlinaxis", sim_vortex_body_seclinaxissameasprimlinaxis},
    {"sim1.vortex_body_secangaxissameasprimangaxis", sim_vortex_body_secangaxissameasprimangaxis},
    {"sim1.vortex_body_normangaxissameasprimangaxis", sim_vortex_body_normangaxissameasprimangaxis},
    {"sim1.vortex_body_autoangulardamping", sim_vortex_body_autoangulardamping},
    {"sim1.newton_global_contactmergetolerance", sim_newton_global_contactmergetolerance},
    {"sim1.newton_global_constraintsolvingiterations", sim_newton_global_constraintsolvingiterations},
    {"sim1.newton_global_bitcoded", sim_newton_global_bitcoded},
    {"sim1.newton_global_multithreading", sim_newton_global_multithreading},
    {"sim1.newton_global_exactsolver", sim_newton_global_exactsolver},
    {"sim1.newton_global_highjointaccuracy", sim_newton_global_highjointaccuracy},
    {"sim1.newton_global_computeinertias", sim_newton_global_computeinertias},
    {"sim1.newton_joint_dependencyfactor", sim_newton_joint_dependencyfactor},
    {"sim1.newton_joint_dependencyoffset", sim_newton_joint_dependencyoffset},
    {"sim1.newton_joint_objectid", sim_newton_joint_objectid},
    {"sim1.newton_joint_dependentobjectid", sim_newton_joint_dependentobjectid},
    {"sim1.newton_joint_pospid1", sim_newton_joint_pospid1},
    {"sim1.newton_joint_pospid2", sim_newton_joint_pospid2},
    {"sim1.newton_joint_pospid3", sim_newton_joint_pospid3},
    {"sim1.newton_body_staticfriction", sim_newton_body_staticfriction},
    {"sim1.newton_body_kineticfriction", sim_newton_body_kineticfriction},
    {"sim1.newton_body_restitution", sim_newton_body_restitution},
    {"sim1.newton_body_lineardrag", sim_newton_body_lineardrag},
    {"sim1.newton_body_angulardrag", sim_newton_body_angulardrag},
    {"sim1.newton_body_bitcoded", sim_newton_body_bitcoded},
    {"sim1.newton_body_fastmoving", sim_newton_body_fastmoving},
    {"sim1.mujoco_global_impratio", sim_mujoco_global_impratio},
    {"sim1.mujoco_global_wind1", sim_mujoco_global_wind1},
    {"sim1.mujoco_global_wind2", sim_mujoco_global_wind2},
    {"sim1.mujoco_global_wind3", sim_mujoco_global_wind3},
    {"sim1.mujoco_global_density", sim_mujoco_global_density},
    {"sim1.mujoco_global_viscosity", sim_mujoco_global_viscosity},
    {"sim1.mujoco_global_boundmass", sim_mujoco_global_boundmass},
    {"sim1.mujoco_global_boundinertia", sim_mujoco_global_boundinertia},
    {"sim1.mujoco_global_overridemargin", sim_mujoco_global_overridemargin},
    {"sim1.mujoco_global_overridesolref1", sim_mujoco_global_overridesolref1},
    {"sim1.mujoco_global_overridesolref2", sim_mujoco_global_overridesolref2},
    {"sim1.mujoco_global_overridesolimp1", sim_mujoco_global_overridesolimp1},
    {"sim1.mujoco_global_overridesolimp2", sim_mujoco_global_overridesolimp2},
    {"sim1.mujoco_global_overridesolimp3", sim_mujoco_global_overridesolimp3},
    {"sim1.mujoco_global_overridesolimp4", sim_mujoco_global_overridesolimp4},
    {"sim1.mujoco_global_overridesolimp5", sim_mujoco_global_overridesolimp5},
    {"sim1.mujoco_global_kinmass", sim_mujoco_global_kinmass},
    {"sim1.mujoco_global_kininertia", sim_mujoco_global_kininertia},
    {"sim1.mujoco_global_bitcoded", sim_mujoco_global_bitcoded},
    {"sim1.mujoco_global_iterations", sim_mujoco_global_iterations},
    {"sim1.mujoco_global_integrator", sim_mujoco_global_integrator},
    {"sim1.mujoco_global_solver", sim_mujoco_global_solver},
    {"sim1.mujoco_global_njmax", sim_mujoco_global_njmax},
    {"sim1.mujoco_global_nconmax", sim_mujoco_global_nconmax},
    {"sim1.mujoco_global_nstack", sim_mujoco_global_nstack},
    {"sim1.mujoco_global_rebuildtrigger", sim_mujoco_global_rebuildtrigger},
    {"sim1.mujoco_global_cone", sim_mujoco_global_cone},
    {"sim1.mujoco_global_overridekin", sim_mujoco_global_overridekin},
    {"sim1.mujoco_global_computeinertias", sim_mujoco_global_computeinertias},
    {"sim1.mujoco_global_multithreaded", sim_mujoco_global_multithreaded},
    {"sim1.mujoco_global_multiccd", sim_mujoco_global_multiccd},
    {"sim1.mujoco_global_balanceinertias", sim_mujoco_global_balanceinertias},
    {"sim1.mujoco_global_overridecontacts", sim_mujoco_global_overridecontacts},
    {"sim1.mujoco_joint_solreflimit1", sim_mujoco_joint_solreflimit1},
    {"sim1.mujoco_joint_solreflimit2", sim_mujoco_joint_solreflimit2},
    {"sim1.mujoco_joint_solimplimit1", sim_mujoco_joint_solimplimit1},
    {"sim1.mujoco_joint_solimplimit2", sim_mujoco_joint_solimplimit2},
    {"sim1.mujoco_joint_solimplimit3", sim_mujoco_joint_solimplimit3},
    {"sim1.mujoco_joint_solimplimit4", sim_mujoco_joint_solimplimit4},
    {"sim1.mujoco_joint_solimplimit5", sim_mujoco_joint_solimplimit5},
    {"sim1.mujoco_joint_frictionloss", sim_mujoco_joint_frictionloss},
    {"sim1.mujoco_joint_solreffriction1", sim_mujoco_joint_solreffriction1},
    {"sim1.mujoco_joint_solreffriction2", sim_mujoco_joint_solreffriction2},
    {"sim1.mujoco_joint_solimpfriction1", sim_mujoco_joint_solimpfriction1},
    {"sim1.mujoco_joint_solimpfriction2", sim_mujoco_joint_solimpfriction2},
    {"sim1.mujoco_joint_solimpfriction3", sim_mujoco_joint_solimpfriction3},
    {"sim1.mujoco_joint_solimpfriction4", sim_mujoco_joint_solimpfriction4},
    {"sim1.mujoco_joint_solimpfriction5", sim_mujoco_joint_solimpfriction5},
    {"sim1.mujoco_joint_stiffness", sim_mujoco_joint_stiffness},
    {"sim1.mujoco_joint_damping", sim_mujoco_joint_damping},
    {"sim1.mujoco_joint_springref", sim_mujoco_joint_springref},
    {"sim1.mujoco_joint_springdamper1", sim_mujoco_joint_springdamper1},
    {"sim1.mujoco_joint_springdamper2", sim_mujoco_joint_springdamper2},
    {"sim1.mujoco_joint_armature", sim_mujoco_joint_armature},
    {"sim1.mujoco_joint_margin", sim_mujoco_joint_margin},
    {"sim1.mujoco_joint_polycoef1", sim_mujoco_joint_polycoef1},
    {"sim1.mujoco_joint_polycoef2", sim_mujoco_joint_polycoef2},
    {"sim1.mujoco_joint_polycoef3", sim_mujoco_joint_polycoef3},
    {"sim1.mujoco_joint_polycoef4", sim_mujoco_joint_polycoef4},
    {"sim1.mujoco_joint_polycoef5", sim_mujoco_joint_polycoef5},
    {"sim1.mujoco_joint_dependentobjectid", sim_mujoco_joint_dependentobjectid},
    {"sim1.mujoco_joint_pospid1", sim_mujoco_joint_pospid1},
    {"sim1.mujoco_joint_pospid2", sim_mujoco_joint_pospid2},
    {"sim1.mujoco_joint_pospid3", sim_mujoco_joint_pospid3},
    {"sim1.mujoco_body_friction1", sim_mujoco_body_friction1},
    {"sim1.mujoco_body_friction2", sim_mujoco_body_friction2},
    {"sim1.mujoco_body_friction3", sim_mujoco_body_friction3},
    {"sim1.mujoco_body_solref1", sim_mujoco_body_solref1},
    {"sim1.mujoco_body_solref2", sim_mujoco_body_solref2},
    {"sim1.mujoco_body_solimp1", sim_mujoco_body_solimp1},
    {"sim1.mujoco_body_solimp2", sim_mujoco_body_solimp2},
    {"sim1.mujoco_body_solimp3", sim_mujoco_body_solimp3},
    {"sim1.mujoco_body_solimp4", sim_mujoco_body_solimp4},
    {"sim1.mujoco_body_solimp5", sim_mujoco_body_solimp5},
    {"sim1.mujoco_body_solmix", sim_mujoco_body_solmix},
    {"sim1.mujoco_body_margin", sim_mujoco_body_margin},
    {"sim1.mujoco_body_condim", sim_mujoco_body_condim},
    {"sim1.mujoco_body_priority", sim_mujoco_body_priority},
    {"sim1.mujoco_dummy_range1", sim_mujoco_dummy_range1},
    {"sim1.mujoco_dummy_range2", sim_mujoco_dummy_range2},
    {"sim1.mujoco_dummy_solreflimit1", sim_mujoco_dummy_solreflimit1},
    {"sim1.mujoco_dummy_solreflimit2", sim_mujoco_dummy_solreflimit2},
    {"sim1.mujoco_dummy_solimplimit1", sim_mujoco_dummy_solimplimit1},
    {"sim1.mujoco_dummy_solimplimit2", sim_mujoco_dummy_solimplimit2},
    {"sim1.mujoco_dummy_solimplimit3", sim_mujoco_dummy_solimplimit3},
    {"sim1.mujoco_dummy_solimplimit4", sim_mujoco_dummy_solimplimit4},
    {"sim1.mujoco_dummy_solimplimit5", sim_mujoco_dummy_solimplimit5},
    {"sim1.mujoco_dummy_margin", sim_mujoco_dummy_margin},
    {"sim1.mujoco_dummy_springlength", sim_mujoco_dummy_springlength},
    {"sim1.mujoco_dummy_stiffness", sim_mujoco_dummy_stiffness},
    {"sim1.mujoco_dummy_damping", sim_mujoco_dummy_damping},
    {"sim1.mujoco_dummy_bitcoded", sim_mujoco_dummy_bitcoded},
    {"sim1.mujoco_dummy_proxyjointid", sim_mujoco_dummy_proxyjointid},
    {"sim1.mujoco_dummy_limited", sim_mujoco_dummy_limited},
    {"sim1.dynmat_default", sim_dynmat_default},
    {"sim1.dynmat_highfriction", sim_dynmat_highfriction},
    {"sim1.dynmat_lowfriction", sim_dynmat_lowfriction},
    {"sim1.dynmat_nofriction", sim_dynmat_nofriction},
    {"sim1.dynmat_reststackgrasp", sim_dynmat_reststackgrasp},
    {"sim1.dynmat_foot", sim_dynmat_foot},
    {"sim1.dynmat_wheel", sim_dynmat_wheel},
    {"sim1.dynmat_gripper", sim_dynmat_gripper},
    {"sim1.dynmat_floor", sim_dynmat_floor},
    {"sim1.displayattribute_renderpass", sim_displayattribute_renderpass},
    {"sim1.displayattribute_depthpass", sim_displayattribute_depthpass},
    {"sim1.displayattribute_pickpass", sim_displayattribute_pickpass},
    {"sim1.displayattribute_selected", sim_displayattribute_selected},
    {"sim1.displayattribute_mainselection", sim_displayattribute_mainselection},
    {"sim1.displayattribute_forcewireframe", sim_displayattribute_forcewireframe},
    {"sim1.displayattribute_forbidwireframe", sim_displayattribute_forbidwireframe},
    {"sim1.displayattribute_forbidedges", sim_displayattribute_forbidedges},
    {"sim1.displayattribute_originalcolors", sim_displayattribute_originalcolors},
    {"sim1.displayattribute_ignorelayer", sim_displayattribute_ignorelayer},
    {"sim1.displayattribute_forvisionsensor", sim_displayattribute_forvisionsensor},
    {"sim1.displayattribute_colorcodedpickpass", sim_displayattribute_colorcodedpickpass},
    {"sim1.displayattribute_colorcoded", sim_displayattribute_colorcoded},
    {"sim1.displayattribute_trianglewireframe", sim_displayattribute_trianglewireframe},
    {"sim1.displayattribute_thickEdges", sim_displayattribute_thickEdges},
    {"sim1.displayattribute_dynamiccontentonly", sim_displayattribute_dynamiccontentonly},
    {"sim1.displayattribute_mirror", sim_displayattribute_mirror},
    {"sim1.displayattribute_useauxcomponent", sim_displayattribute_useauxcomponent},
    {"sim1.displayattribute_ignorerenderableflag", sim_displayattribute_ignorerenderableflag},
    {"sim1.displayattribute_noopenglcallbacks", sim_displayattribute_noopenglcallbacks},
    {"sim1.displayattribute_noghosts", sim_displayattribute_noghosts},
    {"sim1.displayattribute_nopointclouds", sim_displayattribute_nopointclouds},
    {"sim1.displayattribute_nodrawingobjects", sim_displayattribute_nodrawingobjects},
    {"sim1.displayattribute_noparticles", sim_displayattribute_noparticles},
    {"sim1.displayattribute_colorcodedtriangles", sim_displayattribute_colorcodedtriangles},
    {"sim1.navigation_passive", sim_navigation_passive},
    {"sim1.navigation_camerashift", sim_navigation_camerashift},
    {"sim1.navigation_camerarotate", sim_navigation_camerarotate},
    {"sim1.navigation_camerazoom", sim_navigation_camerazoom},
    {"sim1.navigation_cameraangle", sim_navigation_cameraangle},
    {"sim1.navigation_objectshift", sim_navigation_objectshift},
    {"sim1.navigation_objectrotate", sim_navigation_objectrotate},
    {"sim1.navigation_createpathpoint", sim_navigation_createpathpoint},
    {"sim1.navigation_clickselection", sim_navigation_clickselection},
    {"sim1.navigation_ctrlselection", sim_navigation_ctrlselection},
    {"sim1.navigation_shiftselection", sim_navigation_shiftselection},
    {"sim1.navigation_camerazoomwheel", sim_navigation_camerazoomwheel},
    {"sim1.navigation_camerarotaterightbutton", sim_navigation_camerarotaterightbutton},
    {"sim1.navigation_camerarotatemiddlebutton", sim_navigation_camerarotatemiddlebutton},
    {"", -1}};

void _registerTableFunction(luaWrap_lua_State* L, char const* const tableName, char const* const functionName,
                            luaWrap_lua_CFunction functionCallback)
{
    luaWrap_lua_rawgeti(L, luaWrapGet_LUA_REGISTRYINDEX(), luaWrapGet_LUA_RIDX_GLOBALS()); // table of globals
    luaWrap_lua_getfield(L, -1, tableName);
    if (!luaWrap_lua_isgeneraltable(L, -1))
    { // we first need to create the table
        luaWrap_lua_createtable(L, 0, 1);
        luaWrap_lua_setfield(L, -3, tableName);
        luaWrap_lua_pop(L, 1);
        luaWrap_lua_getfield(L, -1, tableName);
    }
    luaWrap_lua_pushtext(L, functionName);
    luaWrap_lua_pushcfunction(L, functionCallback);
    luaWrap_lua_settable(L, -3);
    luaWrap_lua_pop(L, 1);
    luaWrap_lua_pop(L, 1); // pop table of globals
}

void getFloatsFromTable(luaWrap_lua_State* L, int tablePos, size_t floatCount, float* arrayField)
{
    for (size_t i = 0; i < floatCount; i++)
    {
        luaWrap_lua_rawgeti(L, tablePos, int(i + 1));
        arrayField[i] = (float)luaWrap_lua_tonumber(L, -1);
        luaWrap_lua_pop(L, 1); // we pop one element from the stack;
    }
}

void getDoublesFromTable(luaWrap_lua_State* L, int tablePos, size_t doubleCount, double* arrayField)
{
    for (size_t i = 0; i < doubleCount; i++)
    {
        luaWrap_lua_rawgeti(L, tablePos, int(i + 1));
        arrayField[i] = luaWrap_lua_tonumber(L, -1);
        luaWrap_lua_pop(L, 1); // we pop one element from the stack;
    }
}

bool getIntsFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, int* arrayField)
{
    for (size_t i = 0; i < intCount; i++)
    {
        luaWrap_lua_rawgeti(L, tablePos, int(i + 1));
        if (!luaWrap_lua_isnumber(L, -1))
        {
            luaWrap_lua_pop(L, 1); // we pop one element from the stack;
            return (false);        // Not a number!!
        }
        arrayField[i] = luaToInt(L, -1);
        luaWrap_lua_pop(L, 1); // we pop one element from the stack;
    }
    return (true);
}

bool getLongsFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, long long int* arrayField)
{
    for (size_t i = 0; i < intCount; i++)
    {
        luaWrap_lua_rawgeti(L, tablePos, int(i + 1));
        if (!luaWrap_lua_isnumber(L, -1))
        {
            luaWrap_lua_pop(L, 1); // we pop one element from the stack;
            return (false);        // Not a number!!
        }
        arrayField[i] = luaWrap_lua_tointeger(L, -1);
        luaWrap_lua_pop(L, 1); // we pop one element from the stack;
    }
    return (true);
}

bool getUIntsFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, unsigned int* arrayField)
{
    for (size_t i = 0; i < intCount; i++)
    {
        luaWrap_lua_rawgeti(L, tablePos, int(i + 1));
        if (!luaWrap_lua_isnumber(L, -1))
        {
            luaWrap_lua_pop(L, 1); // we pop one element from the stack;
            return (false);        // Not a number!!
        }
        arrayField[i] = (unsigned int)luaToInt(L, -1);
        luaWrap_lua_pop(L, 1); // we pop one element from the stack;
    }
    return (true);
}

bool getUCharsFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, unsigned char* arrayField)
{
    for (size_t i = 0; i < intCount; i++)
    {
        luaWrap_lua_rawgeti(L, tablePos, int(i + 1));
        if (!luaWrap_lua_isnumber(L, -1))
        {
            luaWrap_lua_pop(L, 1); // we pop one element from the stack;
            return (false);        // Not a number!!
        }
        arrayField[i] = (unsigned char)luaToInt(L, -1);
        luaWrap_lua_pop(L, 1); // we pop one element from the stack;
    }
    return (true);
}

void getCharBoolsFromTable(luaWrap_lua_State* L, int tablePos, size_t boolCount, char* arrayField)
{
    for (size_t i = 0; i < boolCount; i++)
    {
        luaWrap_lua_rawgeti(L, tablePos, int(i + 1));
        arrayField[i] = (char)luaToBool(L, -1);
        luaWrap_lua_pop(L, 1); // we pop one element from the stack;
    }
}

void pushFloatTableOntoStack(luaWrap_lua_State* L, size_t floatCount, const float* arrayField)
{
    luaWrap_lua_newtable(L);
    int newTablePos = luaWrap_lua_gettop(L);
    for (size_t i = 0; i < floatCount; i++)
    {
        luaWrap_lua_pushnumber(L, arrayField[i]);
        luaWrap_lua_rawseti(L, newTablePos, int(i + 1));
    }
}

void pushDoubleTableOntoStack(luaWrap_lua_State* L, size_t doubleCount, const double* arrayField)
{
    luaWrap_lua_newtable(L);
    int newTablePos = luaWrap_lua_gettop(L);
    for (size_t i = 0; i < doubleCount; i++)
    {
        luaWrap_lua_pushnumber(L, arrayField[i]);
        luaWrap_lua_rawseti(L, newTablePos, int(i + 1));
    }
}

void pushIntTableOntoStack(luaWrap_lua_State* L, size_t intCount, const int* arrayField)
{
    luaWrap_lua_newtable(L);
    int newTablePos = luaWrap_lua_gettop(L);
    for (size_t i = 0; i < intCount; i++)
    {
        luaWrap_lua_pushinteger(L, arrayField[i]);
        luaWrap_lua_rawseti(L, newTablePos, int(i + 1));
    }
}

void pushLongTableOntoStack(luaWrap_lua_State* L, size_t intCount, const long long int* arrayField)
{
    luaWrap_lua_newtable(L);
    int newTablePos = luaWrap_lua_gettop(L);
    for (size_t i = 0; i < intCount; i++)
    {
        luaWrap_lua_pushinteger(L, arrayField[i]);
        luaWrap_lua_rawseti(L, newTablePos, int(i + 1));
    }
}

void pushUIntTableOntoStack(luaWrap_lua_State* L, size_t intCount, const unsigned int* arrayField)
{
    luaWrap_lua_newtable(L);
    int newTablePos = luaWrap_lua_gettop(L);
    for (size_t i = 0; i < intCount; i++)
    {
        luaWrap_lua_pushinteger(L, arrayField[i]);
        luaWrap_lua_rawseti(L, newTablePos, int(i + 1));
    }
}

void pushUCharTableOntoStack(luaWrap_lua_State* L, size_t intCount, const unsigned char* arrayField)
{
    luaWrap_lua_newtable(L);
    int newTablePos = luaWrap_lua_gettop(L);
    for (size_t i = 0; i < intCount; i++)
    {
        luaWrap_lua_pushinteger(L, arrayField[i]);
        luaWrap_lua_rawseti(L, newTablePos, int(i + 1));
    }
}

void pushStringTableOntoStack(luaWrap_lua_State* L, const std::vector<std::string>& stringTable)
{
    luaWrap_lua_newtable(L);
    int newTablePos = luaWrap_lua_gettop(L);
    for (size_t i = 0; i < stringTable.size(); i++)
    {
        luaWrap_lua_pushtext(L, stringTable[i].c_str());
        luaWrap_lua_rawseti(L, newTablePos, (int)i + 1);
    }
}

void pushBufferTableOntoStack(luaWrap_lua_State* L, const std::vector<std::string>& stringTable)
{
    luaWrap_lua_newtable(L);
    int newTablePos = luaWrap_lua_gettop(L);
    for (size_t i = 0; i < stringTable.size(); i++)
    {
        luaWrap_lua_pushbuffer(L, stringTable[i].c_str(), stringTable[i].size());
        luaWrap_lua_rawseti(L, newTablePos, (int)i + 1);
    }
}

int luaToInt(luaWrap_lua_State* L, int pos)
{
    return ((int)luaWrap_lua_tointeger(L, pos));
}

double luaToDouble(luaWrap_lua_State* L, int pos)
{
    return (luaWrap_lua_tonumber(L, pos));
}

bool luaToBool(luaWrap_lua_State* L, int pos)
{
    return (luaWrap_lua_toboolean(L, pos) != 0);
}

int checkOneGeneralInputArgument(luaWrap_lua_State* L, int index, int type, int cnt_orZeroIfNotTable, bool optional,
                                 bool nilInsteadOfTypeAndCountAllowed, std::string* errStr)
{ // return -1 means error, 0 means data is missing, 1 means data is nil, 2 means data is ok
    // if cnt_orZeroIfNotTable is -1, we are expecting a table, which could also be empty
    // 1. We check if there is something on the stack at that position:
    if (luaWrap_lua_gettop(L) < index)
    { // That data is missing:
        if (optional)
            return (0);
        if (errStr != nullptr)
            errStr->assign(SIM_ERROR_FUNCTION_REQUIRES_MORE_ARGUMENTS);
        return (-1);
    }
    // 2. We check if we have nil on the stack:
    if (luaWrap_lua_isnil(L, index))
    { // We have nil.
        // Did we expect a boolean? If yes, it is ok
        if ((type == lua_arg_bool) && (cnt_orZeroIfNotTable == 0))
            return (2);
        if ((type == lua_arg_nil) && (cnt_orZeroIfNotTable == 0))
            return (2);
        if (nilInsteadOfTypeAndCountAllowed)
            return (1);
    }
    // 3. we check if we expect a table:
    if (cnt_orZeroIfNotTable != 0) // was >=1 until 18/2/2016
    {
        // We check if we really have a table at that position:
        if (!luaWrap_lua_isnonbuffertable(L, index))
        {
            if (errStr != nullptr)
            {
                std::string msg("bad argument #");
                msg += std::to_string(index);
                msg += " (expecting a table).";
                errStr->assign(msg.c_str());
            }
            return (-1);
        }
        // we check the table size:
        if (int(luaWrap_lua_rawlen(L, index)) < cnt_orZeroIfNotTable)
        { // the size is not correct
            if (errStr != nullptr)
            {
                std::string msg("bad argument #");
                msg += std::to_string(index);
                msg += " (wrong table size).";
                errStr->assign(msg.c_str());
            }
            return (-1);
        }
        else
        { // we have the correct size
            // Now we need to check each element individually for the correct type:
            for (int i = 0; i < cnt_orZeroIfNotTable; i++)
            {
                luaWrap_lua_rawgeti(L, index, i + 1);
                if (!checkOneInputArgument(L, -1, type, nullptr))
                {
                    if (errStr != nullptr)
                    {
                        std::string msg("bad argument #");
                        msg += std::to_string(index);
                        msg += " (wrong table content).";
                        errStr->assign(msg.c_str());
                    }
                    return (-1);
                }
                luaWrap_lua_pop(L, 1); // we have to pop the value that was pushed with luaWrap_lua_rawgeti
            }
            // Everything went fine:
            return (2);
        }
    }
    else
    { // we expect a non-table type
        if (checkOneInputArgument(L, index, type, errStr))
            return (2);
        return (-1);
    }
}

bool checkOneInputArgument(luaWrap_lua_State* L, int index, int type, std::string* errStr)
{
    // 1. We check if there is something on the stack at that position:
    if (luaWrap_lua_gettop(L) < index)
    { // That data is missing:
        if (errStr != nullptr)
        {
            std::string msg("missing argument #");
            msg += std::to_string(index);
            errStr->assign(msg.c_str());
        }
        return (false);
    }
    if (type == lua_arg_number)
    {
        if (!luaWrap_lua_isnumber(L, index))
        {
            if (errStr != nullptr)
            {
                std::string msg("bad argument #");
                msg += std::to_string(index);
                msg += " (expecting a number).";
                errStr->assign(msg.c_str());
            }
            return (false); // error
        }
        return (true);
    }
    if (type == lua_arg_integer)
    {
        if (!luaWrap_lua_isinteger(L, index))
        {
            if (errStr != nullptr)
            {
                std::string msg("bad argument #");
                msg += std::to_string(index);
                msg += " (expecting an integer).";
                errStr->assign(msg.c_str());
            }
            return (false); // error
        }
        return (true);
    }
    if (type == lua_arg_bool)
    { // since anything can be a bool value, we don't generate any error!
        return (true);
    }
    if (type == lua_arg_nil)
    { // Here we expect a nil value:
        if (!luaWrap_lua_isnil(L, index))
        {
            if (errStr != nullptr)
            {
                std::string msg("bad argument #");
                msg += std::to_string(index);
                msg += " (expecting nil).";
                errStr->assign(msg.c_str());
            }
            return (false);
        }
        return (true);
    }
    if (type == lua_arg_string)
    { // lua_arg_string and lua_arg_buffer are the same!
        bool retVal = false;
        if (luaWrap_lua_isstring(L, index))
            retVal = true;
        else
        {
            retVal = luaWrap_lua_isbuffer(L, index);
            if ((!retVal) && (errStr != nullptr))
            {
                std::string msg("bad argument #");
                msg += std::to_string(index);
                msg += " (expecting a string/buffer).";
                errStr->assign(msg.c_str());
            }
        }
        return retVal;
    }
    if (type == lua_arg_table)
    {
        if (!luaWrap_lua_isnonbuffertable(L, index))
        {
            if (errStr != nullptr)
            {
                std::string msg("bad argument #");
                msg += std::to_string(index);
                msg += " (expecting a table).";
                errStr->assign(msg.c_str());
            }
            return (false); // error
        }
        return (true);
    }
    if (type == lua_arg_function)
    {
        if (!luaWrap_lua_isfunction(L, index))
        {
            if (errStr != nullptr)
            {
                std::string msg("bad argument #");
                msg += std::to_string(index);
                msg += " (expecting a function).";
                errStr->assign(msg.c_str());
            }
            return (false); // error
        }
        return (true);
    }
    if (type == lua_arg_userdata)
    {
        if (!luaWrap_lua_isuserdata(L, index))
        {
            if (errStr != nullptr)
            {
                std::string msg("bad argument #");
                msg += std::to_string(index);
                msg += " (expecting a user data).";
                errStr->assign(msg.c_str());
            }
            return (false); // error
        }
        return (true);
    }
    return (false);
}

int fetchBoolArg(luaWrap_lua_State* L, int index, bool defaultValue /*= false*/)
{ // make sure you have verified for correct args with checkInputArguments previously
    bool retVal = defaultValue;
    int argCnt = luaWrap_lua_gettop(L);
    if (argCnt >= index)
        retVal = luaWrap_lua_toboolean(L, index);
    return retVal;
}

long long int fetchLongArg(luaWrap_lua_State* L, int index, long long int defaultValue /*= -1*/)
{ // make sure you have verified for correct args with checkInputArguments previously
    long long int retVal = defaultValue;
    int argCnt = luaWrap_lua_gettop(L);
    if (argCnt >= index)
        retVal = luaWrap_lua_tointeger(L, index);
    return retVal;
}

int fetchIntArg(luaWrap_lua_State* L, int index, int defaultValue /*= -1*/)
{ // make sure you have verified for correct args with checkInputArguments previously
    return int(fetchLongArg(L, index, defaultValue));
}

double fetchDoubleArg(luaWrap_lua_State* L, int index, double defaultValue /*= 0.0*/)
{ // make sure you have verified for correct args with checkInputArguments previously
    double retVal = defaultValue;
    int argCnt = luaWrap_lua_gettop(L);
    if (argCnt >= index)
        retVal = luaWrap_lua_tonumber(L, index);
    return retVal;
}

std::string fetchTextArg(luaWrap_lua_State* L, int index, const char* txt /*= ""*/)
{ // make sure you have verified for correct args with checkInputArguments previously
    std::string retVal = txt;
    int argCnt = luaWrap_lua_gettop(L);
    if (argCnt >= index)
        retVal = luaWrap_lua_tostring(L, index);
    return retVal;
}

std::string fetchBufferArg(luaWrap_lua_State* L, int index)
{ // make sure you have verified for correct args with checkInputArguments previously
    std::string retVal;
    int argCnt = luaWrap_lua_gettop(L);
    if (argCnt >= index)
    {
        size_t s;
        const char* b = luaWrap_lua_tobuffer(L, index, &s);
        retVal.assign(b, b + s);
    }
    return retVal;
}

void fetchIntArrayArg(luaWrap_lua_State* L, int index, std::vector<int>& outArr, std::initializer_list<int> arr /*= {}*/)
{ // make sure you have verified for correct args with checkInputArguments previously
    std::vector<int> def;
    if (arr.size() != 0)
        for (int x : arr) def.push_back(x);
    fetchIntArrayArg(L, index, outArr, def);
}

void fetchIntArrayArg(luaWrap_lua_State* L, int index, std::vector<int>& outArr, std::vector<int>& arr)
{ // make sure you have verified for correct args with checkInputArguments previously
    outArr.clear();
    if (arr.size() != 0)
        for (int x : arr) outArr.push_back(x);
    int argCnt = luaWrap_lua_gettop(L);
    if ( (argCnt >= index) && (luaWrap_lua_isnonbuffertable(L, index)) )
    {
        int cnt = int(luaWrap_lua_rawlen(L, index));
        outArr.resize(cnt);
        getIntsFromTable(L, index, cnt, outArr.data());
    }
}

void fetchFloatArrayArg(luaWrap_lua_State* L, int index, std::vector<float>& outArr, std::initializer_list<float> arr /*= {}*/)
{ // make sure you have verified for correct args with checkInputArguments previously
    std::vector<float> def;
    if (arr.size() != 0)
        for (float x : arr) def.push_back(x);
    fetchFloatArrayArg(L, index, outArr, def);
}

void fetchFloatArrayArg(luaWrap_lua_State* L, int index, std::vector<float>& outArr, std::vector<float>& arr)
{ // make sure you have verified for correct args with checkInputArguments previously
    outArr.clear();
    if (arr.size() != 0)
        for (float x : arr) outArr.push_back(x);
    int argCnt = luaWrap_lua_gettop(L);
    if ( (argCnt >= index) && (luaWrap_lua_isnonbuffertable(L, index)) )
    {
        int cnt = int(luaWrap_lua_rawlen(L, index));
        outArr.resize(cnt);
        getFloatsFromTable(L, index, cnt, outArr.data());
    }
}

void fetchDoubleArrayArg(luaWrap_lua_State* L, int index, std::vector<double>& outArr, std::initializer_list<double> arr /*= {}*/)
{ // make sure you have verified for correct args with checkInputArguments previously
    std::vector<double> def;
    if (arr.size() != 0)
        for (double x : arr) def.push_back(x);
    fetchDoubleArrayArg(L, index, outArr, def);
}

void fetchDoubleArrayArg(luaWrap_lua_State* L, int index, std::vector<double>& outArr, std::vector<double>& arr)
{ // make sure you have verified for correct args with checkInputArguments previously
    outArr.clear();
    if (arr.size() != 0)
        for (double x : arr) outArr.push_back(x);
    int argCnt = luaWrap_lua_gettop(L);
    if ( (argCnt >= index) && (luaWrap_lua_isnonbuffertable(L, index)) )
    {
        int cnt = int(luaWrap_lua_rawlen(L, index));
        outArr.resize(cnt);
        getDoublesFromTable(L, index, cnt, outArr.data());
    }
}

bool isArgNilOrMissing(luaWrap_lua_State* L, int index)
{
    bool retVal = true;
    int argCnt = luaWrap_lua_gettop(L);
    if (argCnt >= index)
        retVal = luaWrap_lua_isnil(L, index);
    return retVal;
}

bool checkInputArguments(luaWrap_lua_State* L, std::string* errStr, int type1, int type1Cnt_zeroIfNotTable, int type2,
                         int type2Cnt_zeroIfNotTable, int type3, int type3Cnt_zeroIfNotTable, int type4,
                         int type4Cnt_zeroIfNotTable, int type5, int type5Cnt_zeroIfNotTable, int type6,
                         int type6Cnt_zeroIfNotTable, int type7, int type7Cnt_zeroIfNotTable, int type8,
                         int type8Cnt_zeroIfNotTable, int type9, int type9Cnt_zeroIfNotTable, int type10,
                         int type10Cnt_zeroIfNotTable, int type11, int type11Cnt_zeroIfNotTable)
{ // all arguments, except L and the functionName have default values
    // CAREFUL!!! if typeXCnt_zeroIfNotTable is 0, it means it is a simple value (not table)
    // if typeXCnt_zeroIfNotTable is >=1 then we are expecting a table with at least typeXCnt_zeroIfNotTable elements!!
    // if typeXCnt_zeroIfNotTable is =-1 then we are expecting a table that can have any number of elements
    bool retVal = true;
    int inArgs[11] = {type1, type2, type3, type4, type5, type6, type7, type8, type9, type10, type11};
    int inArgsCnt_zeroIfNotTable[11] = {type1Cnt_zeroIfNotTable, type2Cnt_zeroIfNotTable, type3Cnt_zeroIfNotTable, type4Cnt_zeroIfNotTable, type5Cnt_zeroIfNotTable, type6Cnt_zeroIfNotTable, type7Cnt_zeroIfNotTable, type8Cnt_zeroIfNotTable, type9Cnt_zeroIfNotTable, type10Cnt_zeroIfNotTable, type11Cnt_zeroIfNotTable};
    int argCnt = luaWrap_lua_gettop(L);
    for (int i = 0; i < 11; i++)
    {
        int arg = inArgs[i];
        bool optional = ((arg & lua_arg_optional) != 0);
        if (optional)
            arg -= lua_arg_optional;
        int argItemCnt = inArgsCnt_zeroIfNotTable[i];
        if (arg == lua_arg_empty)
            break;
        if (i >= argCnt)
        {
            if (!optional)
            { // not enough args
                if (errStr != nullptr)
                    errStr->assign(SIM_ERROR_FUNCTION_REQUIRES_MORE_ARGUMENTS);
                retVal = false;
            }
            break;
        }
        else
        {
            if (checkOneGeneralInputArgument(L, i + 1, arg, argItemCnt, optional, optional, errStr) < 1)
            {
                retVal = false;
                break;
            }
        }
    }
    return retVal;
}

bool doesEntityExist(std::string* errStr, int identifier)
{
    if (identifier > SIM_IDEND_SCENEOBJECT)
    {
        if (App::currentWorld->collections->getObjectFromHandle(identifier) == nullptr)
        {
            if (errStr != nullptr)
                errStr[0] = SIM_ERROR_ENTITY_INEXISTANT;
            return (false);
        }
        return (true);
    }
    else
    {
        if (App::currentWorld->sceneObjects->getObjectFromHandle(identifier) == nullptr)
        {
            if (errStr != nullptr)
                errStr[0] = SIM_ERROR_ENTITY_INEXISTANT;
            return (false);
        }
        return (true);
    }
}

int _genericFunctionHandler(luaWrap_lua_State* L, void (*callback)(struct SScriptCallBack* cb),
                            std::string& raiseErrorWithMsg, CScriptCustomFunction* func /*=nullptr*/)
{
    TRACE_LUA_API;

    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* itObj = App::worldContainer->getScriptObjectFromHandle(currentScriptID);

    CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
    CScriptObject::buildFromInterpreterStack_lua(L, stack, 1, 0); // all stack

    // Now we retrieve the object ID this script might be attached to:
    int linkedObject = -1;
    if (itObj->getScriptType() == sim_scripttype_simulation)
    {
        CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(
            itObj->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_simulation));
        if (obj != nullptr)
            linkedObject = obj->getObjectHandle();
    }
    if (itObj->getScriptType() == sim_scripttype_customization)
    {
        CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(
            itObj->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_customization));
        if (obj != nullptr)
            linkedObject = obj->getObjectHandle();
    }

    // We prepare the callback structure:
    SScriptCallBack* cb = new SScriptCallBack;
    cb->objectID = linkedObject;
    cb->scriptID = currentScriptID;
    cb->stackID = stack->getId();
    cb->waitUntilZero = 0;                             // old
    char raiseErrorMsg[258];                           // old
    raiseErrorMsg[0] = '\0';                           // old
    cb->raiseErrorWithMessage = (char*)&raiseErrorMsg; // old
    std::string source(luaWrap_getCurrentCodeSource(L));
    cb->source = (char*)source.c_str();
    cb->line = luaWrap_getCurrentCodeLine(L);

    // Now we can call the callback:
    CScriptObject::setInExternalCall(currentScriptID);
    if (callback != nullptr)
        callback(cb);
    else
        func->callBackFunction_new(cb); // call into old plugin
    CScriptObject::setInExternalCall(-1);

    // Now we have to build the returned data onto the stack:
    CScriptObject::buildOntoInterpreterStack_lua(L, stack, false);

    if (strlen(cb->raiseErrorWithMessage) != 0)
        raiseErrorWithMsg +=
            cb->raiseErrorWithMessage; // is this mechanism used?! We probably simply use simSetLastError..?

    // And we return the number of arguments:
    int outputArgCount = stack->getStackSize();
    delete cb;
    App::worldContainer->interfaceStackContainer->destroyStack(stack);
    return (outputArgCount);
}

int _simGenericFunctionHandler(luaWrap_lua_State* L)
{ // THIS FUNCTION SHOULD NOT LOCK THE API (AT LEAST NOT WHILE CALLING THE CALLBACK!!) SINCE IT IS NOT DIRECTLY
    // ACCESSING THE API!!!!
    TRACE_LUA_API;
    LUA_START("sim.genericFunctionHandler");

    luaWrap_lua_pushvalue(L, luaWrap_lua_upvalueindex(1));
    int id_old = -1;
    std::string funcN;
    if (luaWrap_lua_isinteger(L, -1))
        id_old = luaWrap_lua_tointeger(L, -1) - 1;
    else
        funcN = luaWrap_lua_tostring(L, -1);
    luaWrap_lua_pop(L, 1);

    int outputArgCount = 0;
    if (funcN.size() > 0)
    { // new plugin functions
        size_t p = funcN.find('@');
        std::string namespaceAndVer(funcN.begin(), funcN.begin() + p);
        std::string funcName(funcN.begin() + p + 1, funcN.end());
        CPlugin* plug = App::worldContainer->pluginContainer->getPluginFromName(namespaceAndVer.c_str());
        if (plug != nullptr)
        {
            CPluginCallbackContainer* cont = plug->getPluginCallbackContainer();
            SPluginCallback* pcb = cont->getCallbackFromName(funcName.c_str());
            App::logMsg(sim_verbosity_trace, (std::string("sim.genericFunctionHandler: ") + functionName).c_str());
            if (pcb != nullptr)
            {
                if (pcb->callback != nullptr)
                {
                    plug->pushCurrentPlugin();
                    outputArgCount = _genericFunctionHandler(L, pcb->callback, errorString);
                    plug->popCurrentPlugin();
                }
                else
                    errorString = std::string("sim.genericFunctionHandler: pcb->callback is NULL (") + functionName +
                                  ", funcN: " + funcN + ")";
            }
            else
                errorString =
                    std::string("sim.genericFunctionHandler: pcb is NULL (") + functionName + ", funcN: " + funcN + ")";
        }
        else
            errorString = "plugin not loaded.";
    }
    else
    { // old plugin functions
        for (size_t j = 0; j < App::worldContainer->scriptCustomFuncAndVarContainer->getCustomFunctionCount(); j++)
        { // we now search for the callback to call:
            CScriptCustomFunction* it =
                App::worldContainer->scriptCustomFuncAndVarContainer->getCustomFunctionFromIndex(j);
            if (it->getFunctionID() == id_old)
            { // we have the right one! Now we need to prepare the input and output argument arrays:
                functionName = it->getFunctionName();
                App::logMsg(sim_verbosity_trace, (std::string("sim.genericFunctionHandler: ") + functionName).c_str());
                if (it->getPluginName().size() != 0)
                {
                    functionName += "@simExt";
                    functionName += it->getPluginName();
                }
                else
                    functionName += "@plugin";

                if (it->getUsesStackToExchangeData())
                    outputArgCount = _genericFunctionHandler(L, nullptr, errorString, it);
                else
                    outputArgCount = _genericFunctionHandler_old(L, it);
                break;
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(outputArgCount);
}

int _ccallback0(luaWrap_lua_State* L)
{
    return (_ccallback(L, 0));
}
int _ccallback1(luaWrap_lua_State* L)
{
    return (_ccallback(L, 1));
}
int _ccallback2(luaWrap_lua_State* L)
{
    return (_ccallback(L, 2));
}
int _ccallback3(luaWrap_lua_State* L)
{
    return (_ccallback(L, 3));
}
int _ccallback(luaWrap_lua_State* L, size_t index)
{
    TRACE_LUA_API;
    LUA_START(std::string("ccallback") + std::to_string(index));

    if ((App::callbacks.size() > index) && (App::callbacks[index] != nullptr))
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
        CScriptObject::buildFromInterpreterStack_lua(L, stack, 1, 0);

        int res = ((_ccallback_t)App::callbacks[index])(stack->getId());
        if (res != 0)
        {
            CScriptObject::buildOntoInterpreterStack_lua(L, stack, false);
            int s = stack->getStackSize();
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
            LUA_END(s);
        }
        else
            errorString = SIM_ERROR_CSIDE_CALLBACK_SIGNALED_ERROR;
    }
    else
        errorString = SIM_ERROR_CSIDE_CALLBACK_NOT_REGISTERED;
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _loadPlugin(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("loadPlugin");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0))
    {
        CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
        std::string namespaceAndVersion(luaWrap_lua_tostring(L, 1));
        CPlugin* plug = App::worldContainer->pluginContainer->getPluginFromName(namespaceAndVersion.c_str());

        if ((plug != nullptr) && (plug->hasDependency(it->getScriptUid())))
        { // that script already loaded that plugin. If the script state has been reset, we do not enter here
            luaWrap_lua_getglobal(L, SIM_PLUGIN_NAMESPACES);
            luaWrap_lua_getfield(L, -1, namespaceAndVersion.c_str());
            LUA_END(1);
        }

        plug = App::worldContainer->pluginContainer->loadAndInitPlugin(namespaceAndVersion.c_str(), it->getScriptUid());
        if (plug != nullptr)
        { // success
            it->loadPluginFuncsAndVars(plug);

            luaWrap_lua_getglobal(L, SIM_PLUGIN_NAMESPACES);
            luaWrap_lua_getfield(L, -1, namespaceAndVersion.c_str());
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _unloadPlugin(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("unloadPlugin");

    errorString = SIM_ERROR_INVALID_PLUGIN;
    if (luaWrap_lua_isnonbuffertable(L, 1))
    {
        int options = 0;
        if (luaWrap_lua_isinteger(L, 2))
            options = luaWrap_lua_tointeger(L, 2);
        CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
        int pluginHandle = -1;
        luaWrap_lua_getfield(L, 1, "pluginHandle");
        if (luaWrap_lua_isinteger(L, -1))
            pluginHandle = luaWrap_lua_tointeger(L, -1);
        luaWrap_lua_pop(L, 1);

        if (pluginHandle >= 0)
        {
            CPlugin* plug = App::worldContainer->pluginContainer->getPluginFromHandle(pluginHandle);
            if ((plug != nullptr) && (plug->hasDependency(it->getScriptUid())))
            {
                errorString.clear();

                luaWrap_lua_getglobal(L, SIM_PLUGIN_NAMESPACES);
                luaWrap_lua_pushnil(L);
                luaWrap_lua_setfield(L, -2, plug->getName().c_str());

                App::worldContainer->pluginContainer->deinitAndUnloadPlugin(pluginHandle, it->getScriptUid(), (options & 1) != 0);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _registerCodeEditorInfos(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("registerCodeEditorInfos");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_string, 0))
        App::worldContainer->codeEditorInfos->setInfo(luaWrap_lua_tostring(L, 1), luaWrap_lua_tostring(L, 2),
                                                      &errorString);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _auxFunc(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("auxFunc");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0))
    {
        std::string cmd(luaWrap_lua_tostring(L, 1));
        if (cmd.compare("frexp") == 0)
        {
            if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 0))
            {
                int e;
                luaWrap_lua_pushnumber(L, frexp(luaWrap_lua_tonumber(L, 2), &e));
                luaWrap_lua_pushinteger(L, e);
                LUA_END(2);
            }
            LUA_END(0);
        }

        if (cmd.compare("rand") == 0)
        {
            int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
            CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
            if (it != nullptr)
            {
                luaWrap_lua_pushnumber(L, it->getRandomDouble());
                LUA_END(1);
            }
        }
        if (cmd.compare("randseed") == 0)
        {
            int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
            CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
            if (it != nullptr)
            {
                if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_integer, 0))
                {
                    unsigned int s = abs(luaToInt(L, 2));
                    it->setRandomSeed(s);
                }
            }
        }
        if (cmd.compare("usedmodule") == 0)
        {
            if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_string, 0))
            {
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
                it->addUsedModule(luaWrap_lua_tostring(L, 2));
            }
        }
        if (cmd.compare("stts") == 0)
        {
            if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_string, 0))
                CSimFlavor::getIntVal_str(3, luaWrap_lua_tostring(L, 2));
        }
        if (cmd.compare("useBuffers") == 0)
        {
            luaWrap_lua_pushboolean(L, App::userSettings->useBuffers);
            LUA_END(1);
        }
        //*
        if (cmd.compare("fetchframe") == 0)
        {
            if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_integer, 0))
            {
                std::vector<unsigned char> buff;
                int screenIndex = luaWrap_lua_tointeger(L, 2);
                int res[2] = {0, 0};
#ifdef SIM_WITH_GUI
                if (GuiApp::mainWindow != nullptr)
                {
                    if (screenIndex >= 0)
                    { // desktop
                        QList<QScreen*> screens = QGuiApplication::screens();
                        QPixmap pixmap(screens[screenIndex]->grabWindow(0));
                        QImage img(pixmap.toImage());
                        buff.resize(img.height() * img.width() * 3);
                        for (int i = 0; i < img.height(); i++)
                        {
                            for (int j = 0; j < img.width(); j++)
                            {
                                QRgb pix(img.pixel(j, i));
                                buff[3 * (j + i * img.width()) + 0] = qRed(pix);
                                buff[3 * (j + i * img.width()) + 1] = qGreen(pix);
                                buff[3 * (j + i * img.width()) + 2] = qBlue(pix);
                            }
                        }
                        res[0] = img.width();
                        res[1] = img.height();
                    }
                    else
                    { // openGL view only
                        glPixelStorei(GL_PACK_ALIGNMENT, 1);
                        int resX, resY;
                        GuiApp::mainWindow->getClientArea(resX, resY);
                        res[0] = resX;
                        res[1] = resY;
                        std::vector<unsigned char> tbuff(resX * resY * 3);
                        buff.resize(resX * resY * 3);
                        glReadPixels(0, 0, resX, resY, GL_RGB, GL_UNSIGNED_BYTE, tbuff.data());
                        for (int j = 0; j < resY; j++)
                        {
                            int yp = j * resX;
                            int yq = (resY - j - 1) * resX;
                            for (int i = 0; i < resX; i++)
                            {
                                buff[3 * (yp + i) + 0] = tbuff[3 * (yq + i) + 0];
                                buff[3 * (yp + i) + 1] = tbuff[3 * (yq + i) + 1];
                                buff[3 * (yp + i) + 2] = tbuff[3 * (yq + i) + 2];
                            }
                        }
                    }
                }
#endif
                luaWrap_lua_pushbuffer(L, (const char*)buff.data(), res[0] * res[1] * 3);
                pushIntTableOntoStack(L, 2, res);
                LUA_END(2);
            }
        }
        // */
        if (cmd.compare("getfiles") == 0)
        {
            if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_string, 0, lua_arg_string, 0,
                                    lua_arg_string, 0))
            {
                std::string path(luaWrap_lua_tostring(L, 2));
                std::string filter(luaWrap_lua_tostring(L, 3));
                std::string ext(luaWrap_lua_tostring(L, 4));
                VFileFinder finder;
                finder.searchFiles(path.c_str(), ext.c_str(), filter.c_str());
                int cnt = 0;
                SFileOrFolder* foundItem = finder.getFoundItem(cnt);
                std::vector<std::string> nstrs;
                std::vector<std::string> pstrs;
                while (foundItem != nullptr)
                {
                    pstrs.push_back(foundItem->path);
                    nstrs.push_back(foundItem->name);
                    cnt++;
                    foundItem = finder.getFoundItem(cnt);
                }
                pushStringTableOntoStack(L, nstrs);
                pushStringTableOntoStack(L, pstrs);
                LUA_END(2);
            }
        }
        if (cmd.compare("getKnownPlugin") == 0)
        {
            if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_integer, 0))
            {
                luaWrap_lua_pushtext(L, CSimFlavor::getStringVal_int(3, luaWrap_lua_tointeger(L, 2)).c_str());
                LUA_END(1);
            }
        }
        if (cmd.compare("headless") == 0)
        {
            bool retVal = true;
#ifdef SIM_WITH_GUI
            retVal = (GuiApp::mainWindow == nullptr);
#endif
            luaWrap_lua_pushboolean(L, retVal);
            LUA_END(1);
        }
        if (cmd.compare("simHandleJoint") == 0)
        {
            if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 0, lua_arg_number, 0))
                CALL_C_API(simHandleJoint, luaToInt(L, 2), luaToDouble(L, 3));
            LUA_END(0);
        }
        if (cmd.compare("simHandlePath") == 0)
        {
            if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 0, lua_arg_number, 0))
                CALL_C_API(simHandlePath, luaToInt(L, 2), luaToDouble(L, 3));
            LUA_END(0);
        }
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simHandleSimulationScripts(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.handleSimulationScripts");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int callType = luaToInt(L, 1);
        int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        CScriptObject* it = App::currentWorld->sceneObjects->getScriptObjectFromHandle(currentScriptID);
        if (it != nullptr)
        {
            if (it->getScriptType() == sim_scripttype_main)
            { // only the main script can call this function
                CInterfaceStack* inStack = App::worldContainer->interfaceStackContainer->createStack();
                CScriptObject::buildFromInterpreterStack_lua(L, inStack, 2, 0); // skip the first arg
                retVal = App::currentWorld->sceneObjects->callScripts_noMainScript(sim_scripttype_simulation, callType, inStack, nullptr);
                App::worldContainer->interfaceStackContainer->destroyStack(inStack);
            }
            /*
            silent error here, for backward compatibility!
            else
                errorString=SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT;
                */
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simHandleEmbeddedScripts(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.handleEmbeddedScripts");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int callType = luaToInt(L, 1);
        int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        CScriptObject* it = App::currentWorld->sceneObjects->getScriptObjectFromHandle(currentScriptID);
        if (it != nullptr)
        {
            if (it->getScriptType() == sim_scripttype_main)
            { // only the main script can call this function
                CInterfaceStack* inStack = App::worldContainer->interfaceStackContainer->createStack();
                CScriptObject::buildFromInterpreterStack_lua(L, inStack, 2, 0); // skip the first arg
                retVal = App::currentWorld->sceneObjects->callScripts_noMainScript(-1, callType, inStack, nullptr);
                App::worldContainer->interfaceStackContainer->destroyStack(inStack);
            }
            else
                errorString = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT;
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simHandleDynamics(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.handleDynamics");

    int retVal = -1; // means error
    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* itScrObj = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
    if ( (itScrObj->getScriptType() == sim_scripttype_main) || (itScrObj->getScriptType() == sim_scripttype_simulation) )
    {
        if (checkInputArguments(L, &errorString, lua_arg_number, 0))
            retVal = CALL_C_API(simHandleDynamics, luaToDouble(L, 1));
    }
    else
        errorString = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT_OR_CHILD_SCRIPT;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simHandleProximitySensor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.handleProximitySensor");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        double detPt[4];
        int detectedObjectID;
        double surfaceNormal[3];
        retVal = CALL_C_API(simHandleProximitySensor, luaToInt(L, 1), detPt, &detectedObjectID, surfaceNormal);
        if (retVal == 1)
        {
            luaWrap_lua_pushinteger(L, retVal);
            luaWrap_lua_pushnumber(L, detPt[3]);
            pushDoubleTableOntoStack(L, 3, detPt);
            luaWrap_lua_pushinteger(L, detectedObjectID);
            pushDoubleTableOntoStack(L, 3, surfaceNormal);
            LUA_END(5);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    double dummy[3] = {0.0, 0.0, 0.0};
    luaWrap_lua_pushnumber(L, 0.0);
    pushDoubleTableOntoStack(L, 3, dummy);
    luaWrap_lua_pushinteger(L, -1);
    pushDoubleTableOntoStack(L, 3, dummy);
    LUA_END(5);
}

int _sim_qhull(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim._qhull");

    if (checkInputArguments(L, &errorString, lua_arg_number, 9))
    {
        int vl = (int)luaWrap_lua_rawlen(L, 1);
        if (checkInputArguments(L, &errorString, lua_arg_number, vl))
        {
            double* vertices = new double[vl];
            getDoublesFromTable(L, 1, vl, vertices);
            double* vertOut;
            int vertOutL;
            int* indOut;
            int indOutL;
            if (CALL_C_API(simGetQHull, vertices, vl, &vertOut, &vertOutL, &indOut, &indOutL, 0, nullptr))
            {
                pushDoubleTableOntoStack(L, vertOutL, vertOut);
                pushIntTableOntoStack(L, indOutL, indOut);
                delete[] vertOut;
                delete[] indOut;
                LUA_END(2);
            }
            delete[] vertices;
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simReadProximitySensor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.readProximitySensor");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        double detPt[4];
        int detectedObjectID;
        double surfaceNormal[3];
        retVal = CALL_C_API(simReadProximitySensor, luaToInt(L, 1), detPt, &detectedObjectID, surfaceNormal);
        if (retVal == 1)
        {
            luaWrap_lua_pushinteger(L, retVal);
            luaWrap_lua_pushnumber(L, detPt[3]);
            pushDoubleTableOntoStack(L, 3, detPt);
            luaWrap_lua_pushinteger(L, detectedObjectID);
            pushDoubleTableOntoStack(L, 3, surfaceNormal);
            LUA_END(5);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    luaWrap_lua_pushnumber(L, 0.0);
    double ft[3] = {0.0, 0.0, 0.0};
    pushDoubleTableOntoStack(L, 3, ft);
    luaWrap_lua_pushinteger(L, -1);
    pushDoubleTableOntoStack(L, 3, ft);
    LUA_END(5);
}

int _simHandleVisionSensor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.handleVisionSensor");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        double* auxVals = nullptr;
        int* auxValsCount = nullptr;
        retVal = CALL_C_API(simHandleVisionSensor, luaToInt(L, 1), &auxVals, &auxValsCount);
        if (retVal != -1)
        {
            luaWrap_lua_pushinteger(L, retVal);
            int tableCount = 0;
            if (auxValsCount != nullptr)
            {
                tableCount = auxValsCount[0];
                int off = 0;
                for (int i = 0; i < tableCount; i++)
                {
                    pushDoubleTableOntoStack(L, auxValsCount[i + 1], auxVals + off);
                    off += auxValsCount[i + 1];
                }
                delete[] auxValsCount;
                delete[] auxVals;
            }
            for (int i = tableCount; i < 2; i++)
            {
                pushIntTableOntoStack(L, 0, nullptr); // return at least 2 aux packets, even empty
                tableCount++;
            }
            LUA_END(1 + tableCount);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simReadVisionSensor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.readVisionSensor");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        double* auxVals = nullptr;
        int* auxValsCount = nullptr;
        retVal = CALL_C_API(simReadVisionSensor, luaToInt(L, 1), &auxVals, &auxValsCount);
        if (retVal != -1)
        {
            luaWrap_lua_pushinteger(L, retVal);
            int tableCount = 0;
            if (auxValsCount != nullptr)
            {
                tableCount = auxValsCount[0];
                int off = 0;
                for (int i = 0; i < tableCount; i++)
                {
                    pushDoubleTableOntoStack(L, auxValsCount[i + 1], auxVals + off);
                    off += auxValsCount[i + 1];
                }
                delete[] auxValsCount;
                delete[] auxVals;
            }
            for (int i = tableCount; i < 2; i++)
            {
                pushIntTableOntoStack(L, 0, nullptr); // return at least 2 aux packets, even empty
                tableCount++;
            }
            LUA_END(1 + tableCount);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simResetProximitySensor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.resetProximitySensor");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simResetProximitySensor, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simResetVisionSensor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.resetVisionSensor");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simResetVisionSensor, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetVisionSensorImg(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getVisionSensorImg");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int sensorHandle = luaToInt(L, 1);
        int options = 0;
        double rgbaCutOff = 0.0;
        int pos[2] = {0, 0};
        int size[2] = {0, 0};
        int res;
        res = checkOneGeneralInputArgument(L, 2, lua_arg_integer, 0, true, false, &errorString); // options
        if ((res == 0) || (res == 2))
        {
            if (res == 2)
                options = luaToInt(L, 2);
            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString); // rgbaCutOff
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    rgbaCutOff = luaToDouble(L, 3);
                res = checkOneGeneralInputArgument(L, 4, lua_arg_integer, 2, true, false, &errorString); // pos
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        getIntsFromTable(L, 4, 2, pos);
                    res = checkOneGeneralInputArgument(L, 5, lua_arg_integer, 2, true, false, &errorString); // size
                    if ((res == 0) || (res == 2))
                    {
                        if (res == 2)
                            getIntsFromTable(L, 5, 2, size);
                        int resolution[2];
                        unsigned char* img = CALL_C_API(simGetVisionSensorImg, sensorHandle, options, rgbaCutOff, pos, size, resolution);
                        if (img != nullptr)
                        {
                            int s = 3;
                            if ((options & 1) != 0)
                                s = 1; // greyscale
                            if ((options & 2) != 0)
                                s++; //+ alpha channel
                            if (size[0] == 0)
                                size[0] = resolution[0];
                            if (size[1] == 0)
                                size[1] = resolution[1];
                            luaWrap_lua_pushbuffer(L, (const char*)img, s * size[0] * size[1]);
                            delete[]((char*)img);
                            pushIntTableOntoStack(L, 2, resolution);
                            LUA_END(2);
                        }
                    }
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetVisionSensorImg(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setVisionSensorImg");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int sensorHandle = luaToInt(L, 1);
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(sensorHandle);
        if (it != nullptr)
        { // Ok we have a valid object
            if (it->getObjectType() == sim_sceneobject_visionsensor)
            { // ok we have a valid vision sensor
                int resolution[2];
                ((CVisionSensor*)it)->getResolution(resolution);
                if (luaWrap_lua_isstring(L, 2))
                {
                    size_t l;
                    unsigned char* img = (unsigned char*)luaWrap_lua_tobuffer(L, 2, &l);

                    int options = 0;
                    int pos[2] = {0, 0};
                    int size[2] = {resolution[0], resolution[1]};
                    int res;
                    res = checkOneGeneralInputArgument(L, 3, lua_arg_integer, 0, true, false, &errorString); // options
                    if ((res == 0) || (res == 2))
                    {
                        if (res == 2)
                            options = luaToInt(L, 3);
                        res = checkOneGeneralInputArgument(L, 4, lua_arg_integer, 2, true, false, &errorString); // pos
                        if ((res == 0) || (res == 2))
                        {
                            if (res == 2)
                                getIntsFromTable(L, 4, 2, pos);
                            res = checkOneGeneralInputArgument(L, 5, lua_arg_integer, 2, true, false,
                                                               &errorString); // size
                            if ((res == 0) || (res == 2))
                            {
                                if (res == 2)
                                    getIntsFromTable(L, 5, 2, size);
                                int s = 3;
                                if ((options & 1) != 0)
                                    s = 1; // greyscale
                                if ((options & 2) != 0)
                                    s++; // + alpha channel
                                if (int(l) >= size[0] * size[1] * s)
                                    retVal = CALL_C_API(simSetVisionSensorImg, sensorHandle, img, options, pos, size);
                                else
                                    errorString = SIM_ERROR_INCORRECT_BUFFER_SIZE;
                            }
                        }
                    }
                }
                else
                    errorString = SIM_ERROR_ONE_ARGUMENT_TYPE_IS_WRONG;
            }
            else
                errorString = SIM_ERROR_OBJECT_NOT_VISION_SENSOR;
        }
        else
            errorString = SIM_ERROR_OBJECT_INEXISTANT;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetVisionSensorDepth(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getVisionSensorDepth");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int sensorHandle = luaToInt(L, 1);
        int options = 0;
        int pos[2] = {0, 0};
        int size[2] = {0, 0};
        int res;
        res = checkOneGeneralInputArgument(L, 2, lua_arg_integer, 0, true, false, &errorString); // options
        if ((res == 0) || (res == 2))
        {
            if (res == 2)
                options = luaToInt(L, 2);
            res = checkOneGeneralInputArgument(L, 3, lua_arg_integer, 2, true, false, &errorString); // pos
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    getIntsFromTable(L, 3, 2, pos);
                res = checkOneGeneralInputArgument(L, 4, lua_arg_integer, 2, true, false, &errorString); // size
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        getIntsFromTable(L, 4, 2, size);
                    int resolution[2];
                    float* depth = CALL_C_API(simGetVisionSensorDepth, sensorHandle, options, pos, size, resolution);
                    if (depth != nullptr)
                    {
                        if (size[0] == 0)
                            size[0] = resolution[0];
                        if (size[1] == 0)
                            size[1] = resolution[1];
                        luaWrap_lua_pushbuffer(L, (const char*)depth, size[0] * size[1] * sizeof(float));
                        delete[]((char*)depth);
                        pushIntTableOntoStack(L, 2, resolution);
                        LUA_END(2);
                    }
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simCheckProximitySensor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.checkProximitySensor");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        CProxSensor* it = App::currentWorld->sceneObjects->getProximitySensorFromHandle(handle);
        if (it != nullptr)
        {
            int res = checkOneGeneralInputArgument(L, 3, lua_arg_integer, 0, true, true, &errorString);
            if (res >= 0)
            {
                int options = 0;
                if (it->getFrontFaceDetection())
                    options = options | 1;
                if (it->getBackFaceDetection())
                    options = options | 2;
                if (!it->getExactMode())
                    options = options | 4;
                if (it->getAllowedNormal() > 0.0)
                    options = options | 8;
                if (res == 2)
                {
                    int opt = luaToInt(L, 3);
                    if (opt >= 0)
                        options = opt;
                }
                res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, true, true, &errorString);
                if (res >= 0)
                {
                    double threshhold = DBL_MAX;
                    if (res == 2)
                    {
                        double thr = luaToDouble(L, 4);
                        if (thr > 0.0)
                            threshhold = thr;
                    }
                    res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, true, true, &errorString);
                    if (res >= 0)
                    {
                        double maxNormal = it->getAllowedNormal();
                        if (res == 2)
                        {
                            double mn = luaToDouble(L, 5);
                            if (mn > 0.0)
                                maxNormal = mn;
                        }
                        double detPt[4];
                        double n[3];
                        int detectedObjectHandle;
                        retVal = CALL_C_API(simCheckProximitySensorEx, handle, luaToInt(L, 2), options, threshhold, maxNormal, detPt, &detectedObjectHandle, n);
                        if (retVal == 1)
                        {
                            luaWrap_lua_pushinteger(L, retVal);
                            luaWrap_lua_pushnumber(L, detPt[3]);
                            pushDoubleTableOntoStack(L, 3, detPt);
                            luaWrap_lua_pushinteger(L, detectedObjectHandle);
                            pushDoubleTableOntoStack(L, 3, n);
                            LUA_END(5);
                        }
                    }
                }
            }
        }
        else
            errorString = "invalid proximity sensor handle.";
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    double dummy[3] = {0.0, 0.0, 0.0};
    luaWrap_lua_pushnumber(L, 0.0);
    pushDoubleTableOntoStack(L, 3, dummy);
    luaWrap_lua_pushinteger(L, -1);
    pushDoubleTableOntoStack(L, 3, dummy);
    LUA_END(5);
}

int _simCheckVisionSensor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.checkVisionSensor");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        double* auxVals = nullptr;
        int* auxValsCount = nullptr;
        retVal = CALL_C_API(simCheckVisionSensor, luaToInt(L, 1), luaToInt(L, 2), &auxVals, &auxValsCount);
        if (retVal != -1)
        {
            luaWrap_lua_pushinteger(L, retVal);
            int tableCount = 0;
            if (auxValsCount != nullptr)
            {
                tableCount = auxValsCount[0];
                int off = 0;
                for (int i = 0; i < tableCount; i++)
                {
                    pushDoubleTableOntoStack(L, auxValsCount[i + 1], auxVals + off);
                    off += auxValsCount[i + 1];
                }
                delete[] auxValsCount;
                delete[] auxVals;
            }
            for (int i = tableCount; i < 2; i++)
            {
                pushIntTableOntoStack(L, 0, nullptr); // return at least 2 aux packets, even empty
                tableCount++;
            }
            LUA_END(1 + tableCount);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCheckVisionSensorEx(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.checkVisionSensorEx");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_bool, 0))
    {
        bool returnImage = luaToBool(L, 3);
        int arg1 = luaToInt(L, 1);
        int handleFlags = arg1 & 0xff00000;
        int sensHandle = arg1 & 0xfffff;
        int res[2];
        CALL_C_API(simGetVisionSensorRes, sensHandle, res);
        float* buffer = CALL_C_API(simCheckVisionSensorEx, luaToInt(L, 1), luaToInt(L, 2), returnImage);
        if (buffer != nullptr)
        {
            if ((handleFlags & sim_handleflag_codedstring) != 0)
            {
                if (returnImage)
                {
                    unsigned char* buff2 = new unsigned char[res[0] * res[1] * 3];
                    for (size_t i = 0; i < res[0] * res[1] * 3; i++)
                        buff2[i] = (unsigned char)(buffer[i] * 255.1);
                    luaWrap_lua_pushbuffer(L, (const char*)buff2, res[0] * res[1] * 3);
                    delete[] buff2;
                }
                else
                    luaWrap_lua_pushbuffer(L, (const char*)buffer, res[0] * res[1] * sizeof(float));
            }
            else
            {
                if (returnImage)
                    pushFloatTableOntoStack(L, res[0] * res[1] * 3, buffer);
                else
                    pushFloatTableOntoStack(L, res[0] * res[1], buffer);
            }
            CALL_C_API(simReleaseBuffer, buffer);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetObject(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObject");

    int retVal = -1; // means error

    if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_integer, 0, lua_arg_integer, 0, lua_arg_integer, 0))
    {
        std::string name = fetchTextArg(L, 1);
        int index = fetchIntArg(L, 2, -1);
        int proxyForSearch = fetchIntArg(L, 3, -1);
        int options = fetchIntArg(L, 4, 0);
        setCurrentScriptInfo_cSide(CScriptObject::getScriptHandleFromInterpreterState_lua(L), CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(L)); // for transmitting to the master function additional info (e.g.for autom. name adjustment, or for autom. object deletion when script ends)
        retVal = CALL_C_API(simGetObject, name.c_str(), index, proxyForSearch, options);
        setCurrentScriptInfo_cSide(-1, -1);
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectUid(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectUid");

    long long int retVal = -1; // means error

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0))
        retVal = CALL_C_API(simGetObjectUid, luaToInt(L, 1));
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectFromUid(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectFromUid");

    int retVal = -1; // means error

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_integer, 0))
    {
        long long int uid = fetchLongArg(L, 1);
        int options = fetchIntArg(L, 2, 0);
        retVal = CALL_C_API(simGetObjectFromUid, uid, options);
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetScript(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getScript");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0))
    {
        int scriptType = luaToInt(L, 1);
        int objectHandle = -1;
        int retVal = -1;
        if (scriptType == sim_handle_self)
            retVal = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        else
        {
            std::string scriptName;
            int res;
            // check for new arguments (scriptType, scriptName=''). Do not generate errors with arg2 & arg3:
            if ((luaWrap_lua_gettop(L) >= 2) && luaWrap_lua_stringtype(L, 2))
                scriptName = luaWrap_lua_tostring(L, 2);
            else
            { // check for old arguments (scriptType, objectHandle=-1, scriptName=''), for backw. comp.:
                if (checkInputArguments(L, nullptr, lua_arg_integer, 0, lua_arg_integer, 0))
                    objectHandle = luaToInt(L, 2);
                res = checkOneGeneralInputArgument(L, 3, lua_arg_string, 0, true, true, nullptr);
                if (res == 2)
                    scriptName = luaWrap_lua_tostring(L, 3);
            }
            if (((scriptName.size() > 0) || (objectHandle >= 0)) ||
                ((scriptType == sim_scripttype_main) || (scriptType == sim_scripttype_sandbox)))
            {
                setCurrentScriptInfo_cSide(CScriptObject::getScriptHandleFromInterpreterState_lua(L), CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(L)); // for transmitting to the master function additional info (e.g.for autom. name adjustment, or for autom. object deletion when script ends)
                retVal = CALL_C_API(simGetScriptHandleEx, scriptType, objectHandle, scriptName.c_str());
                setCurrentScriptInfo_cSide(-1, -1);
            }
        }
        if ((retVal != -1) || (objectHandle != -1))
        {
            luaWrap_lua_pushinteger(L, retVal);
            LUA_END(1);
        }
        else
            errorString = SIM_ERROR_SCRIPT_INEXISTANT;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetObjectPosition(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectPosition");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number | lua_arg_optional, 0))
    {
        double coord[3];
        int rel = sim_handle_world;
        if (luaWrap_lua_isinteger(L, 2))
            rel = luaToInt(L, 2);
        if (CALL_C_API(simGetObjectPosition, luaToInt(L, 1), rel, coord) == 1)
        {
            pushDoubleTableOntoStack(L, 3, coord);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetObjectPosition(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setObjectPosition");

    if (luaWrap_lua_isnonbuffertable(L, 2))
    {
        if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 3,
                                lua_arg_integer | lua_arg_optional, 0))
        {
            double coord[3];
            getDoublesFromTable(L, 2, 3, coord);
            int rel = sim_handle_world;
            if (luaWrap_lua_isinteger(L, 3))
                rel = luaToInt(L, 3);
            CALL_C_API(simSetObjectPosition, luaToInt(L, 1), rel, coord);
        }
    }
    else
    { // old
        if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 3))
        {
            double coord[3];
            getDoublesFromTable(L, 3, 3, coord);
            CALL_C_API(simSetObjectPosition, luaToInt(L, 1), luaToInt(L, 2), coord);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetJointPosition(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getJointPosition");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        double jointVal[1];
        if (CALL_C_API(simGetJointPosition, luaToInt(L, 1), jointVal) != -1)
        {
            luaWrap_lua_pushnumber(L, jointVal[0]);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetJointPosition(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setJointPosition");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simSetJointPosition, luaToInt(L, 1), luaToDouble(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetJointTargetPosition(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setJointTargetPosition");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_number, -1, true, false, &errorString);
        if (res >= 0)
        {
            int h = luaToInt(L, 1);
            if (res == 2)
            {
                CJoint* joint = App::currentWorld->sceneObjects->getJointFromHandle(h);
                if (joint != nullptr)
                {
                    if ((joint->getJointMode() == sim_jointmode_kinematic) ||
                        (joint->getJointMode() == sim_jointmode_dynamic))
                    {
                        double maxVelAccelJerk[3];
                        joint->getMaxVelAccelJerk(maxVelAccelJerk);
                        int cnt = 1; // only max. vel. with primitive, built-in controller
                        if ((joint->getJointMode() == sim_jointmode_kinematic) || (joint->getDynPosCtrlType() == 1))
                            cnt = 3; // Motion profile (max. vel, max. accel and max. jerk)
                        getDoublesFromTable(L, 3, std::min<size_t>(luaWrap_lua_rawlen(L, 3), cnt), maxVelAccelJerk);
                        joint->setMaxVelAccelJerk(maxVelAccelJerk);
                    }
                }
            }
            retVal = CALL_C_API(simSetJointTargetPosition, h, luaToDouble(L, 2));
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetJointTargetPosition(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getJointTargetPosition");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        double targetPos;
        if (CALL_C_API(simGetJointTargetPosition, luaToInt(L, 1), &targetPos) != -1)
        {
            luaWrap_lua_pushnumber(L, targetPos);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushnumber(L, 0.0);
    LUA_END(1);
}

int _simGetJointTargetForce(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getJointTargetForce");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        double jointF[1];
        if (CALL_C_API(simGetJointTargetForce, luaToInt(L, 1), jointF) > 0)
        {
            luaWrap_lua_pushnumber(L, jointF[0]);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetJointTargetForce(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setJointTargetForce");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        bool signedValue = true;
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_bool, 0, true, true, &errorString);
        if (res >= 0)
        {
            if (res == 2)
                signedValue = luaToBool(L, 3);
            retVal = CALL_C_API(simSetJointTargetForce, luaToInt(L, 1), luaToDouble(L, 2), signedValue);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetJointTargetVelocity(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setJointTargetVelocity");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_number, -1, true, false, &errorString);
        if (res >= 0)
        {
            int h = luaToInt(L, 1);
            if (res == 2)
            {
                CJoint* joint = App::currentWorld->sceneObjects->getJointFromHandle(h);
                if (joint != nullptr)
                {
                    if ((joint->getJointMode() == sim_jointmode_kinematic) ||
                        (joint->getJointMode() == sim_jointmode_dynamic))
                    {
                        if ((joint->getJointMode() == sim_jointmode_kinematic) || (joint->getDynPosCtrlType() == 1))
                        {
                            double maxVelAccelJerk[3];
                            joint->getMaxVelAccelJerk(maxVelAccelJerk);
                            getDoublesFromTable(L, 3, std::min<size_t>(luaWrap_lua_rawlen(L, 3), 2),
                                                maxVelAccelJerk + 1);
                            joint->setMaxVelAccelJerk(maxVelAccelJerk);
                        }
                    }
                    res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, true, false, &errorString);
                    if (res == 2)
                    {
                        double initVel = luaToDouble(L, 4);
                        joint->setKinematicMotionType(2, true, initVel);
                    }
                }
            }
            if (errorString.size() == 0)
                retVal = CALL_C_API(simSetJointTargetVelocity, h, luaToDouble(L, 2));
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetJointTargetVelocity(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getJointTargetVelocity");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        double targetVel;
        if (CALL_C_API(simGetJointTargetVelocity, luaToInt(L, 1), &targetVel) != -1)
        {
            luaWrap_lua_pushnumber(L, targetVel);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simRefreshDialogs(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.refreshDialogs");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simRefreshDialogs, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetSimulationTime(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getSimulationTime");

    double theTime = CALL_C_API(simGetSimulationTime, );
    if (theTime >= 0.0)
    {
        luaWrap_lua_pushnumber(L, theTime);
        LUA_END(1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, -1);
    LUA_END(1);
}

int _simGetSimulationState(luaWrap_lua_State* L)
{
    // In case we copy-paste a script during execution, the new script doesn't get the sim_simulation_starting message,
    // but that is ok!!! sim_simulation_starting is only for a simulation start. For a first run in a script, use some
    // random variable and check whether it is != from nil!! or use simGetScriptExecutionCount
    TRACE_LUA_API;
    LUA_START("sim.getSimulationState");

    int retVal = CALL_C_API(simGetSimulationState, );

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetSystemTime(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getSystemTime");

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED();
    luaWrap_lua_pushnumber(L, VDateTime::getTime());
    LUA_END(1);
}

int _simCheckCollision(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.checkCollision");
    int retVal = 0;
    int collidingIds[2] = {-1, -1};
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int entity1Handle = luaToInt(L, 1);
        int entity2Handle = luaToInt(L, 2);
        if (doesEntityExist(&errorString, entity1Handle))
        {
            if ((entity2Handle == sim_handle_all) || doesEntityExist(&errorString, entity2Handle))
            {
                if (entity2Handle == sim_handle_all)
                    entity2Handle = -1;

                if (App::currentWorld->mainSettings_old->collisionDetectionEnabled)
                {
                    if (CCollisionRoutine::doEntitiesCollide(entity1Handle, entity2Handle, nullptr, true, true, collidingIds))
                        retVal = 1;
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    pushIntTableOntoStack(L, 2, collidingIds);
    LUA_END(2);
}

int _simCheckDistance(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.checkDistance");
    int retVal = -1;
    double distanceData[7] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    int tb[2] = {-1, -1};
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int entity1Handle = luaToInt(L, 1);
        int entity2Handle = luaToInt(L, 2);
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, true, &errorString);
        if (res >= 0)
        {
            double threshold = -1.0;
            if (res == 2)
                threshold = luaToDouble(L, 3);
            if (doesEntityExist(&errorString, entity1Handle))
            {
                if ((entity2Handle == sim_handle_all) || doesEntityExist(&errorString, entity2Handle))
                {
                    if (entity2Handle == sim_handle_all)
                        entity2Handle = -1;
                    retVal = 0;
                    if (App::currentWorld->mainSettings_old->distanceCalculationEnabled)
                    {
                        int buffer[4];
                        App::currentWorld->cacheData->getCacheDataDist(entity1Handle, entity2Handle, buffer);
                        if (threshold <= 0.0)
                            threshold = DBL_MAX;
                        bool result = CDistanceRoutine::getDistanceBetweenEntitiesIfSmaller(entity1Handle, entity2Handle, threshold, distanceData, buffer, buffer + 2, true, true);
                        App::currentWorld->cacheData->setCacheDataDist(entity1Handle, entity2Handle, buffer);
                        if (result)
                        {
                            retVal = 1;
                            tb[0] = buffer[0];
                            tb[1] = buffer[2];
                        }
                    }
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    pushDoubleTableOntoStack(L, 7, distanceData);
    pushIntTableOntoStack(L, 2, tb);
    LUA_END(3);
}

int _simGetSimulationTimeStep(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getSimulationTimeStep");

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushnumber(L, CALL_C_API(simGetSimulationTimeStep, ));
    LUA_END(1);
}

int _simGetSimulatorMessage(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getSimulatorMessage");
    int auxVals[4] = {0, 0, 0, 0};
    double aux2Vals[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    int aux2Cnt = 0;
    CScriptObject* it =
        App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
    int commandID = it->extractCommandFromOutsideCommandQueue(auxVals, aux2Vals, aux2Cnt);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, commandID);
    pushIntTableOntoStack(L, 4, auxVals);
    pushDoubleTableOntoStack(L, aux2Cnt, aux2Vals);
    LUA_END(3);
}

int _simResetGraph(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.resetGraph");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simResetGraph, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simHandleGraph(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.handleGraph");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simHandleGraph, luaToInt(L, 1), luaToDouble(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAddGraphStream(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.addGraphStream");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_string, 0))
    {
        int graphHandle = luaToInt(L, 1);
        std::string streamName(luaWrap_lua_tostring(L, 2));
        std::string unitStr(luaWrap_lua_tostring(L, 3));
        if (streamName.size() != 0)
        {
            int options = 0;
            int res = checkOneGeneralInputArgument(L, 4, lua_arg_integer, 0, true, false, &errorString);
            if (res == 2)
                options = luaToInt(L, 4);
            if ((res == 0) || (res == 2))
            {
                float col[3] = {1.0, 0.0, 0.0};
                res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 3, true, false, &errorString);
                if (res == 2)
                    getFloatsFromTable(L, 5, 3, col);
                if ((res == 0) || (res == 2))
                {
                    double cyclicRange = piValue;
                    res = checkOneGeneralInputArgument(L, 6, lua_arg_number, 0, true, false, &errorString);
                    if (res == 2)
                        cyclicRange = luaToDouble(L, 6);
                    if ((res == 0) || (res == 2))
                    {
                        setCurrentScriptInfo_cSide(
                            CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                            CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                                L)); // for transmitting to the master function additional info (e.g.for autom. name
                                     // adjustment, or for autom. object deletion when script ends)
                        int retVal = CALL_C_API(simAddGraphStream, graphHandle, streamName.c_str(), unitStr.c_str(),
                                                                options, col, cyclicRange);
                        setCurrentScriptInfo_cSide(-1, -1);
                        luaWrap_lua_pushinteger(L, retVal);
                        LUA_END(1);
                    }
                }
            }
        }
        else
            errorString = SIM_ERROR_EMPTY_STRING_NOT_ALLOWED;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simDestroyGraphCurve(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.destroyGraphCurve");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_integer, 0))
        CALL_C_API(simDestroyGraphCurve, luaToInt(L, 1), luaToInt(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetGraphStreamTransformation(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setGraphStreamTransformation");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_integer, 0, lua_arg_integer, 0))
    {
        double mult = 1.0;
        int res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, true, false, &errorString);
        if (res == 2)
            mult = luaToDouble(L, 4);
        if ((res == 0) || (res == 2))
        {
            double off = 0.0;
            res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, true, false, &errorString);
            if (res == 2)
                off = luaToDouble(L, 5);
            if ((res == 0) || (res == 2))
            {
                int movAvgP = 1;
                res = checkOneGeneralInputArgument(L, 6, lua_arg_integer, 0, true, false, &errorString);
                if (res == 2)
                    movAvgP = luaToInt(L, 6);
                if ((res == 0) || (res == 2))
                    CALL_C_API(simSetGraphStreamTransformation, luaToInt(L, 1), luaToInt(L, 2), luaToInt(L, 3), mult, off,
                                                             movAvgP);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simDuplicateGraphCurveToStatic(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.duplicateGraphCurveToStatic");
    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_integer, 0))
    {
        std::string name;
        const char* str = nullptr;
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_string, 0, true, false, &errorString);
        if (res == 2)
        {
            name = luaWrap_lua_tostring(L, 3);
            if (name.length() > 0)
                str = &name[0];
        }
        if ((res == 0) || (res == 2))
        {
            setCurrentScriptInfo_cSide(
                CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                    L)); // for transmitting to the master function additional info (e.g.for autom. name adjustment, or
                         // for autom. object deletion when script ends)
            CALL_C_API(simDuplicateGraphCurveToStatic, luaToInt(L, 1), luaToInt(L, 2), str);
            setCurrentScriptInfo_cSide(-1, -1);
        }
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simAddGraphCurve(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.addGraphCurve");
    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_integer, 0, lua_arg_integer,
                            2, lua_arg_number, 2))
    {
        int graphHandle = luaToInt(L, 1);
        std::string curveName(luaWrap_lua_tostring(L, 2));
        int dim = luaToInt(L, 3);
        int res = checkOneGeneralInputArgument(L, 4, lua_arg_integer, dim, false, false, &errorString);
        if (res == 2)
        {
            int streamIds[3];
            getIntsFromTable(L, 4, dim, streamIds);
            int res = checkOneGeneralInputArgument(L, 5, lua_arg_number, dim, false, false, &errorString);
            if (res == 2)
            {
                double defaultVals[3];
                getDoublesFromTable(L, 5, dim, defaultVals);
                if (curveName.size() != 0)
                {
                    std::string unitStr;
                    char* _unitStr = nullptr;
                    res = checkOneGeneralInputArgument(L, 6, lua_arg_string, 0, true, false, &errorString);
                    if (res == 2)
                    {
                        unitStr = luaWrap_lua_tostring(L, 6);
                        if (unitStr.size() > 0)
                            _unitStr = &unitStr[0];
                    }
                    if ((res == 0) || (res == 2))
                    {
                        int options = 0;
                        int res = checkOneGeneralInputArgument(L, 7, lua_arg_integer, 0, true, false, &errorString);
                        if (res == 2)
                            options = luaToInt(L, 7);
                        if ((res == 0) || (res == 2))
                        {
                            float col[3] = {1.0, 1.0, 0.0};
                            res = checkOneGeneralInputArgument(L, 8, lua_arg_number, 3, true, false, &errorString);
                            if (res == 2)
                                getFloatsFromTable(L, 8, 3, col);
                            if ((res == 0) || (res == 2))
                            {
                                int curveWidth = 2;
                                res = checkOneGeneralInputArgument(L, 9, lua_arg_integer, 0, true, false, &errorString);
                                if (res == 2)
                                    curveWidth = luaToInt(L, 9);
                                if ((res == 0) || (res == 2))
                                {
                                    setCurrentScriptInfo_cSide(
                                        CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                                        CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                                            L)); // for transmitting to the master function additional info (e.g.for
                                                 // autom. name adjustment, or for autom. object deletion when script
                                                 // ends)
                                    int retVal =
                                        CALL_C_API(simAddGraphCurve, graphHandle, curveName.c_str(), dim, streamIds,
                                                                  defaultVals, _unitStr, options, col, curveWidth);
                                    setCurrentScriptInfo_cSide(-1, -1);
                                    luaWrap_lua_pushinteger(L, retVal);
                                    LUA_END(1);
                                }
                            }
                        }
                    }
                }
                else
                    errorString = SIM_ERROR_EMPTY_STRING_NOT_ALLOWED;
            }
        }
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetGraphStreamValue(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setGraphStreamValue");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_integer, 0, lua_arg_number, 0))
        CALL_C_API(simSetGraphStreamValue, luaToInt(L, 1), luaToInt(L, 2), luaToDouble(L, 3));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _addLog(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("addLog");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int v = luaToInt(L, 1);
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_string, 0, false, true, &errorString);
        if (res > 0)
        {
            if (res == 2)
            {
                std::string msg(luaWrap_lua_tostring(L, 2));
                CScriptObject* it =
                    App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
                App::logScriptMsg(it, v, msg.c_str());
            }
#ifdef SIM_WITH_GUI
            else
                GuiApp::clearStatusbar();
#endif
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _quitSimulator(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("quitSimulator");

    CALL_C_API(simQuitSimulator, false);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simStopSimulation(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.stopSimulation");

    int retVal = -1; // error
    retVal = CALL_C_API(simStopSimulation, );

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simPauseSimulation(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.pauseSimulation");

    int retVal = -1; // error
    retVal = CALL_C_API(simPauseSimulation, );

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simStartSimulation(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.startSimulation");

    int retVal = -1; // error
    retVal = CALL_C_API(simStartSimulation, );

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetObjectPose(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectPose");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_integer | lua_arg_optional, 0))
    {
        double arr[7];
        int rel = sim_handle_world;
        if (luaWrap_lua_isinteger(L, 2))
            rel = luaToInt(L, 2);
        if (CALL_C_API(simGetObjectPose, luaToInt(L, 1), rel, arr) == 1)
        {
            pushDoubleTableOntoStack(L, 7, arr); // Success
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetObjectPose(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setObjectPose");

    if (luaWrap_lua_isnonbuffertable(L, 2))
    {
        if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_number, 7,
                                lua_arg_integer | lua_arg_optional, 0))
        {
            double coord[7];
            getDoublesFromTable(L, 2, 7, coord);
            int rel = sim_handle_world;
            if (luaWrap_lua_isinteger(L, 3))
                rel = luaToInt(L, 3);
            CALL_C_API(simSetObjectPose, luaToInt(L, 1), rel, coord);
        }
    }
    else
    { // old
        if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 7))
        {
            double arr[7];
            getDoublesFromTable(L, 3, 7, arr);
            CALL_C_API(simSetObjectPose, luaToInt(L, 1), luaToInt(L, 2), arr);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetObjectChildPose(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectChildPose");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        double arr[7];
        if (CALL_C_API(simGetObjectChildPose, luaToInt(L, 1), arr) == 1)
        {
            pushDoubleTableOntoStack(L, 7, arr); // Success
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetObjectChildPose(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setObjectChildPose");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 7))
    {
        double arr[7];
        getDoublesFromTable(L, 2, 7, arr);
        CALL_C_API(simSetObjectChildPose, luaToInt(L, 1), arr);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetObjectParent(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectParent");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simGetObjectParent, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectChild(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectChild");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simGetObjectChild, luaToInt(L, 1), luaToInt(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectHierarchyOrder(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectHierarchyOrder");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int totalSiblings;
        int order = CALL_C_API(simGetObjectHierarchyOrder, luaToInt(L, 1), &totalSiblings);
        if (order != -1)
        {
            luaWrap_lua_pushinteger(L, order);
            luaWrap_lua_pushinteger(L, totalSiblings);
            LUA_END(2);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, -1);
    luaWrap_lua_pushinteger(L, 0);
    LUA_END(2);
}

int _simSetObjectHierarchyOrder(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setObjectHierarchyOrder");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
        CALL_C_API(simSetObjectHierarchyOrder, luaToInt(L, 1), luaToInt(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSystemSemaphore(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.systemSemaphore");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_bool, 0))
    {
        std::string key(luaWrap_lua_tostring(L, 1));
        bool acquire = luaToBool(L, 2);
        App::systemSemaphore(key.c_str(), acquire);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetBoolProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setBoolProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_bool, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        int pValue = luaToBool(L, 3);
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simSetBoolProperty, target, pName.c_str(), pValue) > 0)
        {
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                std::string nn(pName);
                if (target == sim_handle_app)
                    nn = "app." + nn;
                else if (target != sim_handle_scene)
                    nn = "obj." + nn;
                it->signalSet(nn.c_str(), target);
            }
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetBoolProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getBoolProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        int pValue;
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simGetBoolProperty, target, pName.c_str(), &pValue) > 0)
        {
            luaWrap_lua_pushboolean(L, pValue != 0);
            LUA_END(1);
        }
        if (noError)
        {
            luaWrap_lua_pushnil(L);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetIntProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setIntProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_integer, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        int pValue = luaWrap_lua_tointeger(L, 3);
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simSetIntProperty, target, pName.c_str(), pValue) > 0)
        {
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                std::string nn(pName);
                if (target == sim_handle_app)
                    nn = "app." + nn;
                else if (target != sim_handle_scene)
                    nn = "obj." + nn;
                it->signalSet(nn.c_str(), target);
            }
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetIntProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getIntProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        int pValue;
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simGetIntProperty, target, pName.c_str(), &pValue) > 0)
        {
            luaWrap_lua_pushinteger(L, pValue);
            LUA_END(1);
        }
        if (noError)
        {
            luaWrap_lua_pushnil(L);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetLongProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setLongProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_integer, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        long long int pValue = luaWrap_lua_tointeger(L, 3);
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simSetLongProperty, target, pName.c_str(), pValue) > 0)
        {
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                std::string nn(pName);
                if (target == sim_handle_app)
                    nn = "app." + nn;
                else if (target != sim_handle_scene)
                    nn = "obj." + nn;
                it->signalSet(nn.c_str(), target);
            }
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetLongProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getLongProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        long long int pValue;
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simGetLongProperty, target, pName.c_str(), &pValue) > 0)
        {
            luaWrap_lua_pushinteger(L, pValue);
            LUA_END(1);
        }
        if (noError)
        {
            luaWrap_lua_pushnil(L);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetHandleProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setHandleProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_integer, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        long long int pValue = luaWrap_lua_tointeger(L, 3);
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simSetHandleProperty, target, pName.c_str(), pValue) > 0)
        {
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                std::string nn(pName);
                if (target == sim_handle_app)
                    nn = "app." + nn;
                else if (target != sim_handle_scene)
                    nn = "obj." + nn;
                it->signalSet(nn.c_str(), target);
            }
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetHandleProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getHandleProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        long long int pValue;
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simGetHandleProperty, target, pName.c_str(), &pValue) > 0)
        {
            luaWrap_lua_pushinteger(L, pValue);
            LUA_END(1);
        }
        if (noError)
        {
            luaWrap_lua_pushnil(L);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetFloatProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setFloatProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_number, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        double pValue = luaToDouble(L, 3);
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simSetFloatProperty, target, pName.c_str(), pValue) > 0)
        {
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                std::string nn(pName);
                if (target == sim_handle_app)
                    nn = "app." + nn;
                else if (target != sim_handle_scene)
                    nn = "obj." + nn;
                it->signalSet(nn.c_str(), target);
            }
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetFloatProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getFloatProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        double pValue;
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simGetFloatProperty, target, pName.c_str(), &pValue) > 0)
        {
            luaWrap_lua_pushnumber(L, pValue);
            LUA_END(1);
        }
        if (noError)
        {
            luaWrap_lua_pushnil(L);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetStringProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setStringProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        std::string pValue(luaWrap_lua_tostring(L, 3));
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simSetStringProperty, target, pName.c_str(), pValue.c_str()) > 0)
        {
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                std::string nn(pName);
                if (target == sim_handle_app)
                    nn = "app." + nn;
                else if (target != sim_handle_scene)
                    nn = "obj." + nn;
                it->signalSet(nn.c_str(), target);
            }
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetStringProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getStringProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        char* pValue = nullptr;
        int res = CALL_C_API(simGetStringProperty, target, pName.c_str(), &pValue);
        if (res > 0)
        {
            luaWrap_lua_pushtext(L, pValue);
            delete[] pValue;
            LUA_END(1);
        }
        if (noError)
        {
            luaWrap_lua_pushnil(L);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetTableProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setTableProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_buffer, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        size_t pValueL;
        const char* pValue = ((char*)luaWrap_lua_tobuffer(L, 3, &pValueL));
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simSetTableProperty, target, pName.c_str(), pValue, int(pValueL)) > 0)
        {
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                std::string nn(pName);
                if (target == sim_handle_app)
                    nn = "app." + nn;
                else if (target != sim_handle_scene)
                    nn = "obj." + nn;
                it->signalSet(nn.c_str(), target);
            }
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetTableProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getTableProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        int pValueL;
        char* pValue = nullptr;
        int res = CALL_C_API(simGetTableProperty, target, pName.c_str(), &pValue, &pValueL);
        if (res > 0)
        {
            luaWrap_lua_pushbuffer(L, pValue, pValueL);
            delete[] pValue;
            LUA_END(1);
        }
        if (noError)
        {
            luaWrap_lua_pushnil(L);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetBufferProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setBufferProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_buffer, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        size_t pValueL;
        const char* pValue = ((char*)luaWrap_lua_tobuffer(L, 3, &pValueL));
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simSetBufferProperty, target, pName.c_str(), pValue, int(pValueL)) > 0)
        {
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                std::string nn(pName);
                if (target == sim_handle_app)
                    nn = "app." + nn;
                else if (target != sim_handle_scene)
                    nn = "obj." + nn;
                it->signalSet(nn.c_str(), target);
            }
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetBufferProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getBufferProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        int pValueL;
        char* pValue;
        int res = CALL_C_API(simGetBufferProperty, target, pName.c_str(), &pValue, &pValueL);
        if (res > 0)
        {
            luaWrap_lua_pushbuffer(L, pValue, size_t(pValueL));
            delete[] pValue;
            LUA_END(1);
        }
        if (noError)
        {
            luaWrap_lua_pushnil(L);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetIntArray2Property(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setIntArray2Property");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_integer, 2))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        int pValue[2];
        getIntsFromTable(L, 3, 2, pValue);
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simSetIntArray2Property, target, pName.c_str(), pValue) > 0)
        {
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                std::string nn(pName);
                if (target == sim_handle_app)
                    nn = "app." + nn;
                else if (target != sim_handle_scene)
                    nn = "obj." + nn;
                it->signalSet(nn.c_str(), target);
            }
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetIntArray2Property(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getIntArray2Property");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        int pValue[2];
        if (CALL_C_API(simGetIntArray2Property, target, pName.c_str(), pValue) > 0)
        {
            pushIntTableOntoStack(L, 2, pValue);
            LUA_END(1);
        }
        if (noError)
        {
            luaWrap_lua_pushnil(L);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetVector2Property(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setVector2Property");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_number, 2))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        double pValue[2];
        getDoublesFromTable(L, 3, 2, pValue);
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simSetVector2Property, target, pName.c_str(), pValue) > 0)
        {
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                std::string nn(pName);
                if (target == sim_handle_app)
                    nn = "app." + nn;
                else if (target != sim_handle_scene)
                    nn = "obj." + nn;
                it->signalSet(nn.c_str(), target);
            }
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetVector2Property(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getVector2Property");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        double pValue[2];
        if (CALL_C_API(simGetVector2Property, target, pName.c_str(), pValue) > 0)
        {
            pushDoubleTableOntoStack(L, 2, pValue);
            LUA_END(1);
        }
        if (noError)
        {
            luaWrap_lua_pushnil(L);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetVector3Property(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setVector3Property");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_number, 3))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        double pValue[3];
        getDoublesFromTable(L, 3, 3, pValue);
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simSetVector3Property, target, pName.c_str(), pValue) > 0)
        {
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                std::string nn(pName);
                if (target == sim_handle_app)
                    nn = "app." + nn;
                else if (target != sim_handle_scene)
                    nn = "obj." + nn;
                it->signalSet(nn.c_str(), target);
            }
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetVector3Property(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getVector3Property");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        double pValue[3];
        if (CALL_C_API(simGetVector3Property, target, pName.c_str(), pValue) > 0)
        {
            pushDoubleTableOntoStack(L, 3, pValue);
            LUA_END(1);
        }
        if (noError)
        {
            luaWrap_lua_pushnil(L);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetQuaternionProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setQuaternionProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_number, 4))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        double pValue[4];
        getDoublesFromTable(L, 3, 4, pValue);
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simSetQuaternionProperty, target, pName.c_str(), pValue) > 0)
        {
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                std::string nn(pName);
                if (target == sim_handle_app)
                    nn = "app." + nn;
                else if (target != sim_handle_scene)
                    nn = "obj." + nn;
                it->signalSet(nn.c_str(), target);
            }
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetQuaternionProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getQuaternionProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        double pValue[4];
        if (CALL_C_API(simGetQuaternionProperty, target, pName.c_str(), pValue) > 0)
        {
            pushDoubleTableOntoStack(L, 4, pValue);
            LUA_END(1);
        }
        if (noError)
        {
            luaWrap_lua_pushnil(L);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetPoseProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setPoseProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_number, 7))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        double pValue[7];
        getDoublesFromTable(L, 3, 7, pValue);
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simSetPoseProperty, target, pName.c_str(), pValue) > 0)
        {
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                std::string nn(pName);
                if (target == sim_handle_app)
                    nn = "app." + nn;
                else if (target != sim_handle_scene)
                    nn = "obj." + nn;
                it->signalSet(nn.c_str(), target);
            }
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetPoseProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getPoseProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        double pValue[7];
        if (CALL_C_API(simGetPoseProperty, target, pName.c_str(), pValue) > 0)
        {
            pushDoubleTableOntoStack(L, 7, pValue);
            LUA_END(1);
        }
        if (noError)
        {
            luaWrap_lua_pushnil(L);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetColorProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setColorProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_number, 3))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        float pValue[3];
        getFloatsFromTable(L, 3, 3, pValue);
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simSetColorProperty, target, pName.c_str(), pValue) > 0)
        {
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                std::string nn(pName);
                if (target == sim_handle_app)
                    nn = "app." + nn;
                else if (target != sim_handle_scene)
                    nn = "obj." + nn;
                it->signalSet(nn.c_str(), target);
            }
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetColorProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getColorProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        float pValue[3];
        if (CALL_C_API(simGetColorProperty, target, pName.c_str(), pValue) > 0)
        {
            pushFloatTableOntoStack(L, 3, pValue);
            LUA_END(1);
        }
        if (noError)
        {
            luaWrap_lua_pushnil(L);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetFloatArrayProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setFloatArrayProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_number, -1))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        int cnt = int(luaWrap_lua_rawlen(L, 3));
        std::vector<double> v;
        v.resize(cnt);
        getDoublesFromTable(L, 3, cnt, v.data());
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simSetFloatArrayProperty, target, pName.c_str(), v.data(), cnt) > 0)
        {
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                std::string nn(pName);
                if (target == sim_handle_app)
                    nn = "app." + nn;
                else if (target != sim_handle_scene)
                    nn = "obj." + nn;
                it->signalSet(nn.c_str(), target);
            }
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetFloatArrayProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getFloatArrayProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        int pValueL;
        double* pValue = nullptr;
        int res = CALL_C_API(simGetFloatArrayProperty, target, pName.c_str(), &pValue, &pValueL);
        if (res > 0)
        {
            pushDoubleTableOntoStack(L, pValueL, pValue);
            delete[] pValue;
            LUA_END(1);
        }
        if (noError)
        {
            luaWrap_lua_pushnil(L);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetIntArrayProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setIntArrayProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_integer, -1))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        int cnt = int(luaWrap_lua_rawlen(L, 3));
        std::vector<int> v;
        v.resize(cnt);
        getIntsFromTable(L, 3, cnt, v.data());
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simSetIntArrayProperty, target, pName.c_str(), v.data(), cnt) > 0)
        {
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                std::string nn(pName);
                if (target == sim_handle_app)
                    nn = "app." + nn;
                else if (target != sim_handle_scene)
                    nn = "obj." + nn;
                it->signalSet(nn.c_str(), target);
            }
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetIntArrayProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getIntArrayProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        int pValueL;
        int* pValue = nullptr;
        int res = CALL_C_API(simGetIntArrayProperty, target, pName.c_str(), &pValue, &pValueL);
        if (res > 0)
        {
            pushIntTableOntoStack(L, pValueL, pValue);
            delete[] pValue;
            LUA_END(1);
        }
        if (noError)
        {
            luaWrap_lua_pushnil(L);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetHandleArrayProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setHandleArrayProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0, lua_arg_integer, -1))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        int cnt = int(luaWrap_lua_rawlen(L, 3));
        std::vector<long long int> v;
        v.resize(cnt);
        getLongsFromTable(L, 3, cnt, v.data());
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (CALL_C_API(simSetHandleArrayProperty, target, pName.c_str(), v.data(), cnt) > 0)
        {
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                std::string nn(pName);
                if (target == sim_handle_app)
                    nn = "app." + nn;
                else if (target != sim_handle_scene)
                    nn = "obj." + nn;
                it->signalSet(nn.c_str(), target);
            }
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetHandleArrayProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getHandleArrayProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        int pValueL;
        long long int* pValue = nullptr;
        int res = CALL_C_API(simGetHandleArrayProperty, target, pName.c_str(), &pValue, &pValueL);
        if (res > 0)
        {
            pushLongTableOntoStack(L, pValueL, pValue);
            delete[] pValue;
            LUA_END(1);
        }
        if (noError)
        {
            luaWrap_lua_pushnil(L);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simRemoveProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.removeProperty");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        bool noError = false;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapBoolValue("noError", noError);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        CALL_C_API(simRemoveProperty, target, pName.c_str());
        if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
        {
            int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
            CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
            std::string nn(pName);
            if (target == sim_handle_app)
                nn = "app." + nn;
            else if (target != sim_handle_scene)
                nn = "obj." + nn;
            it->signalRemoved(nn.c_str());
        }
        if (noError)
            LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetPropertyName(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getPropertyName");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_integer, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        int index = luaWrap_lua_tointeger(L, 2);
        SPropertyOptions opt;
        std::string propertyPrefix;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapInt32Value("objectType", opt.objectType);
            stack->getStackMapStringValue("prefix", propertyPrefix);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        if (propertyPrefix.size() > 0)
            opt.prefix = propertyPrefix.c_str();
        char* pValue = CALL_C_API(simGetPropertyName, target, index, &opt);
        if (pValue != nullptr)
        {
            std::string w1(pValue);
            delete[] pValue;
            std::string w2;
            utils::extractCommaSeparatedWord(w1, w2);
            luaWrap_lua_pushtext(L, w2.c_str());
            luaWrap_lua_pushtext(L, w1.c_str());
            LUA_END(2);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetPropertyInfo(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getPropertyInfo");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        long long int target = luaWrap_lua_tointeger(L, 1);
        if (target == sim_handle_self)
            target = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        std::string pName(luaWrap_lua_tostring(L, 2));
        SPropertyOptions opt;
        if (luaWrap_lua_isnonbuffertable(L, 3))
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            stack->getStackMapInt32Value("objectType", opt.objectType);
            stack->getStackMapBoolValue("shortInfoTxt", opt.shortInfoTxt);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        SPropertyInfo infos;
        int res = CALL_C_API(simGetPropertyInfo, target, pName.c_str(), &infos, &opt);
        if (res > 0)
        {
            luaWrap_lua_pushinteger(L, infos.type);
            luaWrap_lua_pushinteger(L, infos.flags);
            if (infos.infoTxt == nullptr)
                luaWrap_lua_pushtext(L, "");
            else
            {
                luaWrap_lua_pushtext(L, infos.infoTxt);
                delete[] infos.infoTxt;
            }
            LUA_END(3);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetEventFilters(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setEventFilters");

    if (luaWrap_lua_isnonbuffertable(L, 1))
    {
        CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
        CScriptObject::buildFromInterpreterStack_lua(L, stack, 1, 1);
        std::vector<long long int> intKeys;
        stack->getStackMapKeys(nullptr, &intKeys);
        std::map<long long int, std::set<std::string>> filters;
        for (size_t i = 0; i < intKeys.size(); i++)
        {
            bool first = true;
            CInterfaceStackObject* obj = stack->getStackIntMapObject(intKeys[i]);
            if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_table))
            {
                CInterfaceStackTable* arr = ((CInterfaceStackTable*)obj);
                if (arr->isTableArray())
                {
                    std::vector<std::string> fields;
                    arr->getTextArray(fields);
                    if (fields.size() != 0)
                    {
                        for (size_t j = 0; j < fields.size(); j++)
                        {
                            if (fields[j].size() > 0)
                            {
                                if (first)
                                {
                                    filters[intKeys[i]] = std::set<std::string>();
                                    first = false;
                                }
                                filters[intKeys[i]].insert(fields[j]);
                            }
                        }
                    }
                    else
                        filters[intKeys[i]] = std::set<std::string>(); // empty table --> we want all events for that target object
                }
            }
        }
        App::worldContainer->interfaceStackContainer->destroyStack(stack);
        it->setEventFilters(filters);
    }
    else
        errorString = "bad argument #1 (expecting a table)";

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetObjectParent(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setObjectParent");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_integer, 0))
    {
        bool keepInPlace = true;
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_bool, 0, true, false, &errorString);
        if (res >= 0)
        {
            if (res == 2)
                keepInPlace = luaToBool(L, 3);
            retVal = CALL_C_API(simSetObjectParent, luaToInt(L, 1), luaToInt(L, 2), keepInPlace);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectType(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectType");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simGetObjectType, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetJointType(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getJointType");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simGetJointType, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simRemoveObjects(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.removeObjects");
    if (checkInputArguments(L, &errorString, lua_arg_number, -1))
    {
        std::vector<int> handles;
        int cnt = int(luaWrap_lua_rawlen(L, 1));
        if (cnt > 0)
        {
            handles.resize(cnt);
            getIntsFromTable(L, 1, cnt, &handles[0]);
            bool delayed = false;
            int res = checkOneGeneralInputArgument(L, 2, lua_arg_bool, 0, true, true, &errorString);
            if (res >= 0)
            {
                if (res == 2)
                    delayed = luaToBool(L, 2);
                if (delayed)
                    cnt = -cnt;
                //int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                //CScriptObject *it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                CALL_C_API(simRemoveObjects, &handles[0], cnt);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simRemoveModel(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.removeModel");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int objId = luaToInt(L, 1);
        bool delayed = false;
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_bool, 0, true, true, &errorString);
        if (res >= 0)
        {
            if (res == 2)
                delayed = luaToBool(L, 2);
            if (delayed)
                objId = -objId - 1;
            int retVal = CALL_C_API(simRemoveModel, objId);
            if (retVal >= 0)
            {
                luaWrap_lua_pushinteger(L, retVal);
                LUA_END(1);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetObjectAlias(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectAlias");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_integer | lua_arg_optional, 0))
    {
        int h = fetchIntArg(L, 1);
        int options = fetchIntArg(L, 2, -1);
        char* name = CALL_C_API(simGetObjectAlias, h, options);
        if (name != nullptr)
        {
            luaWrap_lua_pushtext(L, name);
            CALL_C_API(simReleaseBuffer, name);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetObjectAlias(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setObjectAlias");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_string, 0))
        retVal = CALL_C_API(simSetObjectAlias, luaToInt(L, 1), luaWrap_lua_tostring(L, 2), 0);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetJointInterval(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getJointInterval");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        bool cyclic;
        double interval[2];
        if (CALL_C_API(simGetJointInterval, luaToInt(L, 1), &cyclic, interval) == 1)
        {
            luaWrap_lua_pushboolean(L, cyclic != 0);
            pushDoubleTableOntoStack(L, 2, interval);
            LUA_END(2);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetJointInterval(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setJointInterval");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_bool, 0, lua_arg_number, 2))
    {
        double interval[2];
        getDoublesFromTable(L, 3, 2, interval);
        retVal = CALL_C_API(simSetJointInterval, luaToInt(L, 1), luaToBool(L, 2), interval);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simLoadScene(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.loadScene");

    int retVal = -1; // error
    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
    if ((script != nullptr) && ((script->getScriptType() == sim_scripttype_addonfunction) ||
                                (script->getScriptType() == sim_scripttype_addon) ||
                                (script->getScriptType() == sim_scripttype_sandbox)))
    {
        if (checkInputArguments(L, &errorString, lua_arg_string, 0))
        {
            size_t dataLength;
            const char* data = ((char*)luaWrap_lua_tobuffer(L, 1, &dataLength));
            if (dataLength < 1000)
                retVal = CALL_C_API(simLoadScene, luaWrap_lua_tostring(L, 1)); // loading from file
            else
            { // loading from buffer
                std::string nm(data + dataLength - 12, 12);
                std::vector<char> buffer(data, data + dataLength);
                if (nm == "@keepCurrent")
                {
                    buffer.erase(buffer.end() - 12, buffer.end());
                    CFileOperations::createNewScene(true);
                }
                std::string infoStr;
                if (CFileOperations::loadScene(nullptr, false, &buffer, &infoStr, &errorString))
                    retVal = 1;
                setLastInfo(infoStr.c_str());
            }
        }
    }
    else
        errorString = SIM_ERROR_MUST_BE_CALLED_FROM_ADDON_OR_SANDBOX_SCRIPT;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCloseScene(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.closeScene");

    int retVal = -1; // error
    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
    if ((script != nullptr) && ((script->getScriptType() == sim_scripttype_addonfunction) ||
                                (script->getScriptType() == sim_scripttype_addon) ||
                                (script->getScriptType() == sim_scripttype_sandbox)))
        retVal = CALL_C_API(simCloseScene, );
    else
        errorString = SIM_ERROR_MUST_BE_CALLED_FROM_ADDON_OR_SANDBOX_SCRIPT;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSaveScene(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.saveScene");

    int retVal = -1; // error
    if (luaWrap_lua_gettop(L) != 0)
    { // to file
        if (checkInputArguments(L, &errorString, lua_arg_string, 0))
            retVal = CALL_C_API(simSaveScene, luaWrap_lua_tostring(L, 1));
    }
    else
    { // to buffer
        if (!App::currentWorld->environment->getSceneLocked())
        {
            std::vector<char> buffer;
            std::string infoStr;
            if (CFileOperations::saveScene(nullptr, false, false, &buffer, &infoStr, &errorString))
            {
#ifdef SIM_WITH_GUI
                GuiApp::setRebuildHierarchyFlag(); // we might have saved under a different name, we need to reflect it
#endif
                setLastInfo(infoStr.c_str());
                luaWrap_lua_pushbuffer(L, &buffer[0], buffer.size());
                LUA_END(1);
            }
            setLastInfo(infoStr.c_str());
        }
        else
            errorString = SIM_ERROR_SCENE_LOCKED;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simLoadModel(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.loadModel");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, lua_arg_string, 0))
    {
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_bool, 0, true, true, &errorString);
        if (res >= 0)
        {
            bool onlyThumbnails = false;
            if (res == 2)
                onlyThumbnails = luaToBool(L, 2);
            size_t dataLength;
            const char* data = ((char*)luaWrap_lua_tobuffer(L, 1, &dataLength));
            if (dataLength < 1000)
            { // loading from file:
                std::string path(data, dataLength);
                size_t atCopyPos = path.find("@copy");
                bool forceAsCopy = (atCopyPos != std::string::npos);
                if (forceAsCopy)
                    path.erase(path.begin() + atCopyPos, path.end());

                std::string infoStr;
                if (CFileOperations::loadModel(path.c_str(), false, false, nullptr, onlyThumbnails, forceAsCopy,
                                               &infoStr, &errorString))
                {
                    setLastInfo(infoStr.c_str());
#ifdef SIM_WITH_GUI
                    GuiApp::setRebuildHierarchyFlag();
#endif
                    if (onlyThumbnails)
                    {
                        char* buff = new char[128 * 128 * 4];
                        bool opRes = App::currentWorld->environment->modelThumbnail_notSerializedHere
                                         .copyUncompressedImageToBuffer(buff);
                        if (opRes)
                        {
                            luaWrap_lua_pushbuffer(L, buff, 128 * 128 * 4);
                            delete[] buff;
                            LUA_END(1);
                        }
                        delete[] buff;
                        LUA_END(0);
                    }
                    else
                        retVal = App::currentWorld->sceneObjects->getLastSelectionHandle();
                }
                setLastInfo(infoStr.c_str());
            }
            else
            { // loading from buffer:
                std::vector<char> buffer(data, data + dataLength);
                std::string infoStr;
                if (CFileOperations::loadModel(nullptr, false, false, &buffer, onlyThumbnails, false, &infoStr,
                                               &errorString))
                {
                    setLastInfo(infoStr.c_str());
#ifdef SIM_WITH_GUI
                    GuiApp::setRebuildHierarchyFlag();
#endif
                    if (onlyThumbnails)
                    {
                        char* buff = new char[128 * 128 * 4];
                        bool opRes = App::currentWorld->environment->modelThumbnail_notSerializedHere
                                         .copyUncompressedImageToBuffer(buff);
                        if (opRes)
                        {
                            luaWrap_lua_pushbuffer(L, buff, 128 * 128 * 4);
                            delete[] buff;
                            LUA_END(1);
                        }
                        delete[] buff;
                        LUA_END(0);
                    }
                    else
                        retVal = App::currentWorld->sceneObjects->getLastSelectionHandle();
                }
                setLastInfo(infoStr.c_str());
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSaveModel(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.saveModel");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int model = luaToInt(L, 1);
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_string, 0, true, true, &errorString);
        if (res >= 0)
        {
            if (res == 2)
                retVal = CALL_C_API(simSaveModel, luaToInt(L, 1), luaWrap_lua_tostring(L, 2));
            else
            { // here we don't save to file, but to buffer:

                if (!App::currentWorld->environment->getSceneLocked())
                {
                    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(model);
                    if (it != nullptr)
                    {
                        if (it->getModelBase())
                        {
                            std::vector<int> initSelection(App::currentWorld->sceneObjects->getSelectedObjectHandlesPtr()[0]);
                            std::vector<char> buffer;
                            std::string infoStr;
                            if (CFileOperations::saveModel(model, nullptr, false, &buffer, &infoStr, &errorString))
                            {
                                App::currentWorld->sceneObjects->setSelectedObjectHandles(initSelection.data(), initSelection.size());
                                setLastInfo(infoStr.c_str());
                                luaWrap_lua_pushbuffer(L, &buffer[0], buffer.size());
                                LUA_END(1);
                            }
                        }
                        else
                            errorString = SIM_ERROR_OBJECT_NOT_MODEL_BASE;
                    }
                    else
                        errorString = SIM_ERROR_OBJECT_INEXISTANT;
                }
                else
                    errorString = SIM_ERROR_SCENE_LOCKED;
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectSel(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectSel");

    int selSize;
    int* sel = CALL_C_API(simGetObjectSel, &selSize);
    if (sel != nullptr)
    {
        pushIntTableOntoStack(L, selSize, sel);
        delete[] sel;
        LUA_END(1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetObjectSel(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setObjectSel");

    int argCnt = luaWrap_lua_gettop(L);
    if (argCnt >= 1)
    {
        if (luaWrap_lua_isnonbuffertable(L, 1))
        {
            int objCnt = (int)luaWrap_lua_rawlen(L, 1);
            if ((objCnt == 0) || checkInputArguments(L, &errorString, lua_arg_integer, objCnt))
            {
                std::vector<int> objectHandles;
                objectHandles.resize(objCnt, 0);
                getIntsFromTable(L, 1, objCnt, &objectHandles[0]);
                CALL_C_API(simSetObjectSel, &objectHandles[0], objCnt);
            }
        }
        else
            errorString = "bad argument #1 (expecting a table)";
    }
    else
        errorString = SIM_ERROR_FUNCTION_REQUIRES_MORE_ARGUMENTS;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simLaunchExecutable(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.launchExecutable");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, lua_arg_string, 0))
    {
        std::string file(luaWrap_lua_tostring(L, 1));
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_string, 0, true, false, &errorString);
        if ((res == 0) || (res == 2))
        {
            std::string args;
            if (res == 2)
                args = luaWrap_lua_tostring(L, 2);
            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString);
            if ((res == 0) || (res == 2))
            {
                int showStatus = 1;
                if (res == 2)
                    showStatus = luaToInt(L, 3);
                int sh = VVARIOUS_SHOWNORMAL;
                if (showStatus == 0)
                    sh = VVARIOUS_HIDE;
                CScriptObject* it =
                    App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
                std::string what(it->getDescriptiveName());
                what += " (via sim.launchExecutable)";
                if (1 == CALL_C_API(simCheckExecAuthorization, what.c_str(), (file + " " + args).c_str(), it->getScriptHandle()))
                {
                    if (VVarious::executeExternalApplication(file.c_str(), args.c_str(),
                                                             App::folders->getExecutablePath().c_str(),
                                                             sh)) // executable directory needed because otherwise the
                                                                  // shellExecute command might switch directories!
                        retVal = 1;
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetExtensionString(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getExtensionString");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int objHandle = luaToInt(L, 1);
        int index = luaToInt(L, 2);
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_string, 0, true, true, &errorString);
        if (res >= 0)
        {
            std::string key;
            if (res == 2)
                key = luaWrap_lua_tostring(L, 3);
            char* str = CALL_C_API(simGetExtensionString, objHandle, index, key.c_str());
            if (str != nullptr)
            {
                luaWrap_lua_pushtext(L, str);
                CALL_C_API(simReleaseBuffer, str);
                LUA_END(1);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simComputeMassAndInertia(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.computeMassAndInertia");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int shapeHandle = luaToInt(L, 1);
        double density = luaToDouble(L, 2);
        retVal = CALL_C_API(simComputeMassAndInertia, shapeHandle, density);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simTest(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.test");
    if (checkInputArguments(L, nullptr, lua_arg_string, 0))
    {
        std::string cmd = luaWrap_lua_tostring(L, 1);
        if (cmd.compare("mjcf") == 0)
        {
            std::string r = App::worldContainer->pluginContainer->dyn_generateMjcfFile();
            luaWrap_lua_pushtext(L, r.c_str());
            LUA_END(1);
        }
        if (cmd.compare("showSimpleDlg") == 0)
        {
#ifdef SIM_WITH_GUI
            GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, "Test dlg", "some text", VMESSAGEBOX_YES_NO_CANCEL, VMESSAGEBOX_REPLY_NO);
#endif
        }

        if (cmd.compare("sim.getGeodesicInfo") == 0)
        { // pt1,pt2,vertices,indices(can be {}),maxEdge,debugShape
            C3Vector pt1, pt2;
            getDoublesFromTable(L, 2, 3, pt1.data);
            getDoublesFromTable(L, 3, 3, pt2.data);
            int vl = 3 * ((int)luaWrap_lua_rawlen(L, 4) / 3);
            std::vector<double> vertices;
            vertices.resize(vl);
            getDoublesFromTable(L, 4, vl, vertices.data());
            int il = 3 * ((int)luaWrap_lua_rawlen(L, 5) / 3);
            std::vector<int> indices;
            indices.resize(il);
            getIntsFromTable(L, 5, il, indices.data());
            std::vector<double> path;
            int debugShape = -1;
            int* _debugShape = nullptr;
            if (luaToBool(L, 7))
                _debugShape = &debugShape;
            double dist = CMeshRoutines::getGeodesicDistanceOnConvexMesh(pt1, pt2, vertices, &indices, &path,
                                                                         luaToDouble(L, 6), _debugShape);
            luaWrap_lua_pushnumber(L, dist);
            pushDoubleTableOntoStack(L, path.size(), path.data());
            luaWrap_lua_pushinteger(L, debugShape);
            LUA_END(3);
        }
        if (cmd.compare("sim.recomputeInertia") == 0)
        {
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_shape); i++)
            {
                CShape* it = App::currentWorld->sceneObjects->getShapeFromIndex(i);
                if (!it->getStatic())
                {
                    CMeshWrapper* mesh = it->getMesh();
                    double mass = mesh->getMass();
                    C3Vector diagI;
                    mesh->getDiagonalInertiaInfo(diagI);
                    double wOld = cbrt(diagI(0) * diagI(1) * diagI(2));
                    if (it->computeMassAndInertia(1000.0))
                    {
                        mesh->setMass(mass);
                        C3Vector diagI2;
                        mesh->getDiagonalInertiaInfo(diagI2);
                        double wNew = cbrt(diagI2(0) * diagI2(1) * diagI2(2));
                        mesh->setInertia(mesh->getInertia() * (wOld / wNew));
                        mesh->getDiagonalInertiaInfo(diagI2);
                        wNew = cbrt(diagI2(0) * diagI2(1) * diagI2(2));
                    }
                }
            }
            LUA_END(0);
        }
        if (cmd.compare("getModelState") == 0)
        {
            int h = luaWrap_lua_tointeger(L, 2);
            int p = -1;
            if (checkInputArguments(L, nullptr, lua_arg_string, 0, lua_arg_number, 0, lua_arg_number, 0))
                p = luaWrap_lua_tointeger(L, 3);
            std::string state = App::currentWorld->sceneObjects->getModelState(h, p);
            luaWrap_lua_pushbuffer(L, state.c_str(), state.size());
            LUA_END(1);
        }
        if (cmd.compare("createStack") == 0)
        {
            int stack = CALL_C_API(simCreateStack);
            luaWrap_lua_pushinteger(L, stack);
            LUA_END(1);
        }
        if (cmd.compare("printStack") == 0)
        {
            int stackHandle = luaToInt(L, 2);
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
            std::string str;
            stack->printContent(-1, str);
            CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
            App::logScriptMsg(it, sim_verbosity_msgs, str.c_str());
            LUA_END(0);
        }
        if ((cmd.compare("sim.recomputeInertia1KeepMass") == 0) ||
            (cmd.compare("sim.recomputeInertia2KeepMass") == 0) || (cmd.compare("sim.recomputeInertia4KeepMass") == 0))
        {
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_shape); i++)
            {
                CShape* it = App::currentWorld->sceneObjects->getShapeFromIndex(i);
                if (!it->getStatic())
                {
                    CMeshWrapper* mesh = it->getMesh();
                    double mass = mesh->getMass();
                    if (it->computeMassAndInertia(1000.0))
                    {
                        mesh->setMass(mass); // keep mass
                        double f = 1.0;
                        if (cmd.compare("sim.recomputeInertia2KeepMass") == 0)
                            f = 2.0;
                        if (cmd.compare("sim.recomputeInertia4KeepMass") == 0)
                            f = 4.0;
                        mesh->setInertia(mesh->getInertia() * f);
                    }
                }
            }
            LUA_END(0);
        }
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simTextEditorOpen(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.textEditorOpen");
    int retVal = -1;

#ifdef SIM_WITH_GUI
    if (App::worldContainer->pluginContainer->isCodeEditorPluginAvailable() && (GuiApp::mainWindow != nullptr))
    {
        if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_string, 0))
        {
            const char* arg1 = luaWrap_lua_tostring(L, 1);
            const char* arg2 = luaWrap_lua_tostring(L, 2);
            retVal = GuiApp::mainWindow->codeEditorContainer->open(
                arg1, arg2, CScriptObject::getScriptHandleFromInterpreterState_lua(L));
        }
    }
    else
        errorString = "code Editor plugin was not found.";
#endif

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simTextEditorClose(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.textEditorClose");

#ifdef SIM_WITH_GUI
    if (App::worldContainer->pluginContainer->isCodeEditorPluginAvailable() && (GuiApp::mainWindow != nullptr))
    {
        if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        {
            int handle = luaToInt(L, 1);
            int posAndSize[4];
            std::string txt;
            if (GuiApp::mainWindow->codeEditorContainer->close(handle, posAndSize, &txt, nullptr))
            {
                luaWrap_lua_pushtext(L, txt.c_str());
                pushIntTableOntoStack(L, 2, posAndSize + 0);
                pushIntTableOntoStack(L, 2, posAndSize + 2);
                LUA_END(3);
            }
        }
    }
    else
        errorString = "code Editor plugin was not found.";
#endif

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simTextEditorShow(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.textEditorShow");
    int retVal = -1;

#ifdef SIM_WITH_GUI
    if (App::worldContainer->pluginContainer->isCodeEditorPluginAvailable() && (GuiApp::mainWindow != nullptr))
    {
        if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        {
            int handle = luaToInt(L, 1);
            bool showState = luaToBool(L, 2);
            retVal = GuiApp::mainWindow->codeEditorContainer->showOrHide(handle, showState);
            luaWrap_lua_pushinteger(L, retVal);
            LUA_END(1);
        }
    }
    else
        errorString = "code Editor plugin was not found.";
#endif

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simTextEditorGetInfo(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.textEditorGetInfo");

#ifdef SIM_WITH_GUI
    if (App::worldContainer->pluginContainer->isCodeEditorPluginAvailable() && (GuiApp::mainWindow != nullptr))
    {
        if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        {
            int handle = luaToInt(L, 1);
            int state = GuiApp::mainWindow->codeEditorContainer->getShowState(handle);
            if (state >= 0)
            {
                int posAndSize[4];
                std::string txt = GuiApp::mainWindow->codeEditorContainer->getText(handle, posAndSize);
                luaWrap_lua_pushtext(L, txt.c_str());
                pushIntTableOntoStack(L, 2, posAndSize + 0);
                pushIntTableOntoStack(L, 2, posAndSize + 2);
                luaWrap_lua_pushboolean(L, state != 0);
                LUA_END(4);
            }
            LUA_END(0);
        }
    }
    else
        errorString = "code Editor plugin was not found.";
#endif

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetJointDependency(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setJointDependency");
    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number,
                            0))
    {
        int jointHandle = luaToInt(L, 1);
        int masterJointHandle = luaToInt(L, 2);
        double off = luaToDouble(L, 3);
        double coeff = luaToDouble(L, 4);
        retVal = CALL_C_API(simSetJointDependency, jointHandle, masterJointHandle, off, coeff);
        if (retVal >= 0)
        {
            luaWrap_lua_pushinteger(L, retVal);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetJointDependency(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getJointDependency");
    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int jointHandle = luaToInt(L, 1);
        int masterJointHandle;
        double off;
        double coeff;
        retVal = CALL_C_API(simGetJointDependency, jointHandle, &masterJointHandle, &off, &coeff);
        if (retVal >= 0)
        {
            luaWrap_lua_pushinteger(L, masterJointHandle);
            luaWrap_lua_pushnumber(L, off);
            luaWrap_lua_pushnumber(L, coeff);
            LUA_END(3);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetStackTraceback(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getLastStackTraceback");
    std::string retVal;
    int scriptHandle = -1;
    if (luaWrap_lua_gettop(L) != 0)
    {
        if (checkInputArguments(L, &errorString, lua_arg_number, 0))
            scriptHandle = luaToInt(L, 1);
    }
    else
        scriptHandle = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(scriptHandle);
    if (it != nullptr)
        retVal = it->getAndClearLastStackTraceback();

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushtext(L, retVal.c_str());
    LUA_END(1);
}

int _simSetNavigationMode(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setNavigationMode");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simSetNavigationMode, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetNavigationMode(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getNavigationMode");

    int retVal = -1; // error
    retVal = CALL_C_API(simGetNavigationMode, );

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetPage(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setPage");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simSetPage, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetPage(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getPage");
    int retVal = -1; // error

    retVal = CALL_C_API(simGetPage, );

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCopyPasteObjects(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.copyPasteObjects");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 1, lua_arg_integer | lua_arg_optional,0))
    {
        std::vector<int> objectHandles;
        fetchIntArrayArg(L, 1, objectHandles);
        int options = fetchIntArg(L, 2, 0);
        if (CALL_C_API(simCopyPasteObjects, &objectHandles[0], objectHandles.size(), options) > 0)
        {
            pushIntTableOntoStack(L, objectHandles.size(), &objectHandles[0]);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simScaleObjects(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.scaleObjects");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, lua_arg_number, 1, lua_arg_number, 0, lua_arg_bool, 0))
    {
        int objCnt = (int)luaWrap_lua_rawlen(L, 1);
        if (checkInputArguments(L, &errorString, lua_arg_number, objCnt, lua_arg_number, 0, lua_arg_bool, 0))
        {
            std::vector<int> objectHandles;
            objectHandles.resize(objCnt, 0);
            getIntsFromTable(L, 1, objCnt, &objectHandles[0]);
            retVal = CALL_C_API(simScaleObjects, &objectHandles[0], objCnt, luaToDouble(L, 2), luaToBool(L, 3));
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetAutoYieldDelay(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setAutoYieldDelay");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int timeInMs = int(luaToDouble(L, 1) * 1000.0);
        int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
        if (it != nullptr)
            it->setDelayForAutoYielding(timeInMs);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetAutoYieldDelay(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getAutoYieldDelay");

    int timeInMs = 0;
    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
    if (it != nullptr)
        timeInMs = it->getDelayForAutoYielding();

    luaWrap_lua_pushinteger(L, timeInMs);
    LUA_END(1);
}

int _setAutoYield(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("setAutoYield");

    if (luaWrap_lua_gettop(L) > 0)
    {
        int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
        if (it != nullptr)
        {
            int retVal;
            if (luaWrap_lua_isnumber(L, 1))
            {
                int a = luaToInt(L, 1);
                retVal = it->changeAutoYieldingForbidLevel(a, true);
            }
            else
            {
                bool allow = luaToBool(L, 1);
                if (allow)
                    retVal = it->changeAutoYieldingForbidLevel(-1, false);
                else
                    retVal = it->changeAutoYieldingForbidLevel(1, false);
            }
            luaWrap_lua_pushinteger(L, retVal);
            LUA_END(1);
        }
    }
    else
        errorString = SIM_ERROR_FUNCTION_REQUIRES_MORE_ARGUMENTS;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _getAutoYield(luaWrap_lua_State* L)
{ // doesn't generate an error
    TRACE_LUA_API;
    LUA_START("getAutoYield");
    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
    bool retVal = false;
    int level = 0;
    if (it != nullptr)
    {
        level = it->getAutoYieldingForbidLevel();
        retVal = (level == 0);
    }

    luaWrap_lua_pushboolean(L, retVal);
    luaWrap_lua_pushinteger(L, level);
    LUA_END(2);
}

int _getYieldAllowed(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("getYieldAllowed");
    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
    bool canYield = true;
    if (it != nullptr)
        canYield = it->canManualYield();
    luaWrap_lua_pushboolean(L, canYield);
    LUA_END(1);
}

int _setYieldAllowed(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("setYieldAllowed");

    if (luaWrap_lua_gettop(L) > 0)
    {
        int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
        if (it != nullptr)
        {
            int retVal;
            if (luaWrap_lua_isnumber(L, 1))
            {
                int a = luaToInt(L, 1);
                retVal = it->changeOverallYieldingForbidLevel(a, true);
            }
            else
            {
                bool allow = luaToBool(L, 1);
                int a = 1;
                if (allow)
                    a = -1;
                retVal = it->changeOverallYieldingForbidLevel(a, false);
            }
            luaWrap_lua_pushinteger(L, retVal);
            LUA_END(1);
        }
    }
    else
        errorString = SIM_ERROR_FUNCTION_REQUIRES_MORE_ARGUMENTS;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _registerScriptFuncHook(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("registerScriptFuncHook");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_string, 0, lua_arg_bool, 0))
    {
        const char* systemFunc = luaWrap_lua_tostring(L, 1);
        const char* userFunc = luaWrap_lua_tostring(L, 2);
        bool execBefore = luaToBool(L, 3);
        retVal = CALL_C_API(simRegisterScriptFuncHook, CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                                                    systemFunc, userFunc, execBefore, 0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSaveImage(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.saveImage");

    std::string retBuffer;
    if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 2, lua_arg_number, 0, lua_arg_string, 0,
                            lua_arg_number, 0))
    {
        size_t dataLength;
        char* data = ((char*)luaWrap_lua_tobuffer(L, 1, &dataLength));
        std::string img(data, dataLength);
        int res[2];
        getIntsFromTable(L, 2, 2, res);
        int options = luaToInt(L, 3);
        std::string filename(luaWrap_lua_tostring(L, 4));
        int quality = luaToInt(L, 5);
        int channels = 3;
        if ((options & 3) == 1)
            channels = 4;
        if ((options & 3) == 2)
            channels = 1;
        if (int(dataLength) >= res[0] * res[1] * channels)
        {
            if ((res[0] > 0) && (res[1] > 0))
            {
                if (filename.size() > 0)
                {
                    bool err;
                    if (filename[0] != '.')
                        err = (1 != CALL_C_API(simSaveImage, (unsigned char*)&img[0], res, options, filename.c_str(), quality, nullptr));
                    else
                        err = !CImageLoaderSaver::save((unsigned char*)&img[0], res, options, filename.c_str(), quality, &retBuffer); // to memory
                    if (err)
                        errorString = SIM_ERROR_OPERATION_FAILED;
                }
                else
                    errorString = "bad argument #4 (expecting a valid filename)";
            }
            else
                errorString = SIM_ERROR_INVALID_RESOLUTION;
        }
        else
            errorString = SIM_ERROR_INCORRECT_BUFFER_SIZE;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushbuffer(L, &retBuffer[0], retBuffer.size());
    LUA_END(1);
}

int _simLoadImage(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.loadImage");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_string, 0))
    {
        int options = luaToInt(L, 1);
        size_t dataLength;
        char* data = ((char*)luaWrap_lua_tobuffer(L, 2, &dataLength));
        int resol[2];
        unsigned char* img = nullptr;
        if ((dataLength > 4) && (data[0] == '@') && (data[1] == 'm') && (data[2] == 'e') && (data[3] == 'm'))
        {
            int reserved[1] = {(int)dataLength};
            img = CALL_C_API(simLoadImage, resol, options, data + 4, reserved);
        }
        else
            img = CALL_C_API(simLoadImage, resol, options, data, nullptr);
        if (img != nullptr)
        {
            int s = resol[0] * resol[1] * 3;
            if (options & 1)
                s = resol[0] * resol[1] * 4;
            luaWrap_lua_pushbuffer(L, (const char*)img, s);
            delete[]((char*)img);
            pushIntTableOntoStack(L, 2, resol);
            LUA_END(2);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetScaledImage(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getScaledImage");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 2, lua_arg_number, 2, lua_arg_number,
                            0))
    {
        size_t dataLength;
        char* data = ((char*)luaWrap_lua_tobuffer(L, 1, &dataLength));
        std::string imgIn(data, dataLength);
        int resIn[2];
        getIntsFromTable(L, 2, 2, resIn);
        int resOut[2];
        getIntsFromTable(L, 3, 2, resOut);
        int options = luaToInt(L, 4);
        int channelsIn = 3;
        if (options & 1)
            channelsIn = 4;
        if (int(dataLength) >= resIn[0] * resIn[1] * channelsIn)
        {
            if ((resIn[0] > 0) && (resIn[1] > 0) && (resOut[0] > 0) && (resOut[1] > 0))
            {
                unsigned char* imgOut =
                    CALL_C_API(simGetScaledImage, (unsigned char*)&imgIn[0], resIn, resOut, options, nullptr);
                if (imgOut != nullptr)
                {
                    int s = resOut[0] * resOut[1] * 3;
                    if (options & 2)
                        s = resOut[0] * resOut[1] * 4;
                    luaWrap_lua_pushbuffer(L, (const char*)imgOut, s);
                    delete[]((char*)imgOut);
                    pushIntTableOntoStack(L, 2, resOut);
                    LUA_END(2);
                }
            }
            else
                errorString = SIM_ERROR_INVALID_RESOLUTION;
        }
        else
            errorString = SIM_ERROR_INCORRECT_BUFFER_SIZE;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simTransformImage(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.transformImage");
    int retVal = -1;

    if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 2, lua_arg_number, 0))
    {
        size_t dataLength;
        char* data = ((char*)luaWrap_lua_tobuffer(L, 1, &dataLength));
        int resol[2];
        getIntsFromTable(L, 2, 2, resol);
        int options = luaToInt(L, 3);
        int channels = 3;
        if (options & 1)
            channels = 4;
        if (int(dataLength) >= resol[0] * resol[1] * channels)
        {
            if ((resol[0] > 0) && (resol[1] > 0))
            {
                if (CALL_C_API(simTransformImage, (unsigned char*)data, resol, options, nullptr, nullptr, nullptr) == 1)
                {
                    luaWrap_lua_pushbuffer(L, (const char*)data, dataLength);
                    LUA_END(1);
                }
            }
            else
                errorString = SIM_ERROR_INVALID_RESOLUTION;
        }
        else
            errorString = SIM_ERROR_INCORRECT_BUFFER_SIZE;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simPackInt32Table(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.packInt32Table");

    if (luaWrap_lua_gettop(L) > 0)
    {
        if (luaWrap_lua_isnonbuffertable(L, 1))
        {
            int startIndex = 0;
            int count = 0;
            int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString);
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    startIndex = luaToInt(L, 2);

                res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString);
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        count = luaToInt(L, 3);

                    int tableSize = int(luaWrap_lua_rawlen(L, 1));

                    if (count == 0)
                        count = tableSize - startIndex;
                    if (count > tableSize - startIndex)
                        count = tableSize - startIndex;
                    if (count > 0)
                    {
                        char* data = new char[sizeof(int) * count];
                        for (int i = 0; i < count; i++)
                        {
                            luaWrap_lua_rawgeti(L, 1, i + 1 + startIndex);
                            int v = luaToInt(L, -1);
                            data[4 * i + 0] = ((char*)&v)[0];
                            data[4 * i + 1] = ((char*)&v)[1];
                            data[4 * i + 2] = ((char*)&v)[2];
                            data[4 * i + 3] = ((char*)&v)[3];
                            luaWrap_lua_pop(L, 1); // we have to pop the value that was pushed with luaWrap_lua_rawgeti
                        }
                        luaWrap_lua_pushbuffer(L, (const char*)data, count * sizeof(int));
                        delete[] data;
                        LUA_END(1);
                    }
                }
            }
        }
        else
            errorString = SIM_ERROR_ONE_ARGUMENT_TYPE_IS_WRONG;
    }
    else
        errorString = SIM_ERROR_FUNCTION_REQUIRES_MORE_ARGUMENTS;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simPackUInt32Table(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.packUInt32Table");

    if (luaWrap_lua_gettop(L) > 0)
    {
        if (luaWrap_lua_isnonbuffertable(L, 1))
        {
            int startIndex = 0;
            int count = 0;
            int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString);
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    startIndex = luaToInt(L, 2);

                res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString);
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        count = luaToInt(L, 3);

                    int tableSize = int(luaWrap_lua_rawlen(L, 1));

                    if (count == 0)
                        count = tableSize - startIndex;
                    if (count > tableSize - startIndex)
                        count = tableSize - startIndex;
                    if (count > 0)
                    {
                        char* data = new char[sizeof(unsigned int) * count];
                        for (int i = 0; i < count; i++)
                        {
                            luaWrap_lua_rawgeti(L, 1, i + 1 + startIndex);
                            unsigned int v = (unsigned int)luaToInt(L, -1);
                            data[4 * i + 0] = ((char*)&v)[0];
                            data[4 * i + 1] = ((char*)&v)[1];
                            data[4 * i + 2] = ((char*)&v)[2];
                            data[4 * i + 3] = ((char*)&v)[3];
                            luaWrap_lua_pop(L, 1); // we have to pop the value that was pushed with luaWrap_lua_rawgeti
                        }
                        luaWrap_lua_pushbuffer(L, (const char*)data, count * sizeof(unsigned int));
                        delete[] data;
                        LUA_END(1);
                    }
                }
            }
        }
        else
            errorString = SIM_ERROR_ONE_ARGUMENT_TYPE_IS_WRONG;
    }
    else
        errorString = SIM_ERROR_FUNCTION_REQUIRES_MORE_ARGUMENTS;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simPackFloatTable(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.packFloatTable");

    if (luaWrap_lua_gettop(L) > 0)
    {
        if (luaWrap_lua_isnonbuffertable(L, 1))
        {
            int startIndex = 0;
            int count = 0;
            int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString);
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    startIndex = luaToInt(L, 2);

                res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString);
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        count = luaToInt(L, 3);

                    int tableSize = int(luaWrap_lua_rawlen(L, 1));

                    if (count == 0)
                        count = tableSize - startIndex;
                    if (count > tableSize - startIndex)
                        count = tableSize - startIndex;
                    if (count > 0)
                    {
                        char* data = new char[sizeof(float) * count];
                        for (int i = 0; i < count; i++)
                        {
                            luaWrap_lua_rawgeti(L, 1, i + 1 + startIndex);
                            float v = luaToDouble(L, -1);
                            data[4 * i + 0] = ((char*)&v)[0];
                            data[4 * i + 1] = ((char*)&v)[1];
                            data[4 * i + 2] = ((char*)&v)[2];
                            data[4 * i + 3] = ((char*)&v)[3];
                            luaWrap_lua_pop(L, 1); // we have to pop the value that was pushed with luaWrap_lua_rawgeti
                        }
                        luaWrap_lua_pushbuffer(L, (const char*)data, count * sizeof(float));
                        delete[] data;
                        LUA_END(1);
                    }
                }
            }
        }
        else
            errorString = SIM_ERROR_ONE_ARGUMENT_TYPE_IS_WRONG;
    }
    else
        errorString = SIM_ERROR_FUNCTION_REQUIRES_MORE_ARGUMENTS;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simPackDoubleTable(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.packDoubleTable");

    if (luaWrap_lua_gettop(L) > 0)
    {
        if (luaWrap_lua_isnonbuffertable(L, 1))
        {
            int startIndex = 0;
            int count = 0;
            int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString);
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    startIndex = luaToInt(L, 2);

                res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString);
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        count = luaToInt(L, 3);

                    int tableSize = int(luaWrap_lua_rawlen(L, 1));

                    if (count == 0)
                        count = tableSize - startIndex;
                    if (count > tableSize - startIndex)
                        count = tableSize - startIndex;
                    if (count > 0)
                    {
                        char* data = new char[sizeof(double) * count];
                        for (int i = 0; i < count; i++)
                        {
                            luaWrap_lua_rawgeti(L, 1, i + 1 + startIndex);
                            double v = luaToDouble(L, -1);
                            data[sizeof(double) * i + 0] = ((char*)&v)[0];
                            data[sizeof(double) * i + 1] = ((char*)&v)[1];
                            data[sizeof(double) * i + 2] = ((char*)&v)[2];
                            data[sizeof(double) * i + 3] = ((char*)&v)[3];
                            data[sizeof(double) * i + 4] = ((char*)&v)[4];
                            data[sizeof(double) * i + 5] = ((char*)&v)[5];
                            data[sizeof(double) * i + 6] = ((char*)&v)[6];
                            data[sizeof(double) * i + 7] = ((char*)&v)[7];
                            luaWrap_lua_pop(L, 1); // we have to pop the value that was pushed with luaWrap_lua_rawgeti
                        }
                        luaWrap_lua_pushbuffer(L, (const char*)data, count * sizeof(double));
                        delete[] data;
                        LUA_END(1);
                    }
                }
            }
        }
        else
            errorString = SIM_ERROR_ONE_ARGUMENT_TYPE_IS_WRONG;
    }
    else
        errorString = SIM_ERROR_FUNCTION_REQUIRES_MORE_ARGUMENTS;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simPackUInt8Table(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.packUInt8Table");

    if (luaWrap_lua_gettop(L) > 0)
    {
        if (luaWrap_lua_isnonbuffertable(L, 1))
        {
            int startIndex = 0;
            int count = 0;
            int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString);
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    startIndex = luaToInt(L, 2);

                res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString);
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        count = luaToInt(L, 3);

                    int tableSize = int(luaWrap_lua_rawlen(L, 1));

                    if (count == 0)
                        count = tableSize - startIndex;
                    if (count > tableSize - startIndex)
                        count = tableSize - startIndex;
                    if (count > 0)
                    {
                        char* data = new char[sizeof(char) * count];
                        for (int i = 0; i < count; i++)
                        {
                            luaWrap_lua_rawgeti(L, 1, i + 1 + startIndex);
                            unsigned char v = (unsigned char)luaToInt(L, -1);
                            data[i] = ((char*)&v)[0];
                            luaWrap_lua_pop(L, 1); // we have to pop the value that was pushed with luaWrap_lua_rawgeti
                        }
                        luaWrap_lua_pushbuffer(L, (const char*)data, count * sizeof(char));
                        delete[] data;
                        LUA_END(1);
                    }
                }
            }
        }
        else
            errorString = SIM_ERROR_ONE_ARGUMENT_TYPE_IS_WRONG;
    }
    else
        errorString = SIM_ERROR_FUNCTION_REQUIRES_MORE_ARGUMENTS;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simPackUInt16Table(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.packUInt16Table");

    if (luaWrap_lua_gettop(L) > 0)
    {
        if (luaWrap_lua_isnonbuffertable(L, 1))
        {
            int startIndex = 0;
            int count = 0;
            int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString);
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    startIndex = luaToInt(L, 2);

                res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString);
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        count = luaToInt(L, 3);

                    int tableSize = int(luaWrap_lua_rawlen(L, 1));

                    if (count == 0)
                        count = tableSize - startIndex;
                    if (count > tableSize - startIndex)
                        count = tableSize - startIndex;
                    if (count > 0)
                    {
                        char* data = new char[sizeof(short) * count];
                        for (int i = 0; i < count; i++)
                        {
                            luaWrap_lua_rawgeti(L, 1, i + 1 + startIndex);
                            unsigned short v = (unsigned short)luaToInt(L, -1);
                            data[2 * i + 0] = ((char*)&v)[0];
                            data[2 * i + 1] = ((char*)&v)[1];
                            luaWrap_lua_pop(L, 1); // we have to pop the value that was pushed with luaWrap_lua_rawgeti
                        }
                        luaWrap_lua_pushbuffer(L, (const char*)data, count * sizeof(short));
                        delete[] data;
                        LUA_END(1);
                    }
                }
            }
        }
        else
            errorString = SIM_ERROR_ONE_ARGUMENT_TYPE_IS_WRONG;
    }
    else
        errorString = SIM_ERROR_FUNCTION_REQUIRES_MORE_ARGUMENTS;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simUnpackInt32Table(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.unpackInt32Table");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0))
    {
        int startIndex = 0;
        int count = 0;
        int additionalCharOffset = 0;
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString);
        if ((res == 0) || (res == 2))
        {
            if (res == 2)
                startIndex = luaToInt(L, 2);

            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString);
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    count = luaToInt(L, 3);

                res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, true, false, &errorString);
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        additionalCharOffset = luaToInt(L, 4);

                    size_t dataLength;
                    char* data = ((char*)luaWrap_lua_tobuffer(L, 1, &dataLength)) + additionalCharOffset;
                    dataLength = sizeof(int) * ((dataLength - additionalCharOffset) / sizeof(int));
                    int packetCount = int(dataLength / sizeof(int));
                    if (count == 0)
                        count = int(1999999999);

                    if (dataLength == 0)
                    { // since 20.03.2024: empty buffer results in an empty table
                        luaWrap_lua_newtable(L);
                        LUA_END(1);
                    }
                    else
                    {
                        if ((startIndex >= 0) && (startIndex < packetCount))
                        {
                            if (startIndex + count > packetCount)
                                count = packetCount - startIndex;

                            int* theInts = new int[count];
                            for (int i = 0; i < int(count); i++)
                            {
                                int v;
                                ((char*)&v)[0] = data[sizeof(int) * (i + startIndex) + 0];
                                ((char*)&v)[1] = data[sizeof(int) * (i + startIndex) + 1];
                                ((char*)&v)[2] = data[sizeof(int) * (i + startIndex) + 2];
                                ((char*)&v)[3] = data[sizeof(int) * (i + startIndex) + 3];
                                theInts[i] = v;
                            }
                            pushIntTableOntoStack(L, count, theInts);
                            delete[] theInts;
                            LUA_END(1);
                        }
                    }
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simUnpackUInt32Table(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.unpackUInt32Table");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0))
    {
        int startIndex = 0;
        int count = 0;
        int additionalCharOffset = 0;
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString);
        if ((res == 0) || (res == 2))
        {
            if (res == 2)
                startIndex = luaToInt(L, 2);

            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString);
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    count = luaToInt(L, 3);

                res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, true, false, &errorString);
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        additionalCharOffset = luaToInt(L, 4);

                    size_t dataLength;
                    char* data = ((char*)luaWrap_lua_tobuffer(L, 1, &dataLength)) + additionalCharOffset;
                    dataLength = sizeof(unsigned int) * ((dataLength - additionalCharOffset) / sizeof(unsigned int));
                    int packetCount = int(dataLength / sizeof(unsigned int));
                    if (count == 0)
                        count = int(1999999999);

                    if (dataLength == 0)
                    { // since 20.03.2024: empty buffer results in an empty table
                        luaWrap_lua_newtable(L);
                        LUA_END(1);
                    }
                    else
                    {
                        if ((startIndex >= 0) && (startIndex < packetCount))
                        {
                            if (startIndex + count > packetCount)
                                count = packetCount - startIndex;

                            unsigned int* theInts = new unsigned int[count];
                            for (int i = 0; i < int(count); i++)
                            {
                                unsigned int v;
                                ((char*)&v)[0] = data[sizeof(unsigned int) * (i + startIndex) + 0];
                                ((char*)&v)[1] = data[sizeof(unsigned int) * (i + startIndex) + 1];
                                ((char*)&v)[2] = data[sizeof(unsigned int) * (i + startIndex) + 2];
                                ((char*)&v)[3] = data[sizeof(unsigned int) * (i + startIndex) + 3];
                                theInts[i] = v;
                            }
                            pushUIntTableOntoStack(L, count, theInts);
                            delete[] theInts;
                            LUA_END(1);
                        }
                    }
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simUnpackFloatTable(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.unpackFloatTable");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0))
    {
        int startIndex = 0;
        int count = 0;
        int additionalCharOffset = 0;
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString);
        if ((res == 0) || (res == 2))
        {
            if (res == 2)
                startIndex = luaToInt(L, 2);

            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString);
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    count = luaToInt(L, 3);

                res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, true, false, &errorString);
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        additionalCharOffset = luaToInt(L, 4);

                    size_t dataLength;
                    char* data = ((char*)luaWrap_lua_tobuffer(L, 1, &dataLength)) + additionalCharOffset;
                    dataLength = sizeof(float) * ((dataLength - additionalCharOffset) / sizeof(float));
                    int packetCount = int(dataLength / sizeof(float));
                    if (count == 0)
                        count = int(1999999999);

                    if (dataLength == 0)
                    { // since 20.03.2024: empty buffer results in an empty table
                        luaWrap_lua_newtable(L);
                        LUA_END(1);
                    }
                    else
                    {
                        if ((startIndex >= 0) && (startIndex < packetCount))
                        {
                            if (startIndex + count > packetCount)
                                count = packetCount - startIndex;

                            float* theFloats = new float[count];
                            for (int i = 0; i < int(count); i++)
                            {
                                float v;
                                ((char*)&v)[0] = data[sizeof(float) * (i + startIndex) + 0];
                                ((char*)&v)[1] = data[sizeof(float) * (i + startIndex) + 1];
                                ((char*)&v)[2] = data[sizeof(float) * (i + startIndex) + 2];
                                ((char*)&v)[3] = data[sizeof(float) * (i + startIndex) + 3];
                                theFloats[i] = v;
                            }
                            pushFloatTableOntoStack(L, count, theFloats);
                            delete[] theFloats;
                            LUA_END(1);
                        }
                    }
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simUnpackDoubleTable(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.unpackDoubleTable");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0))
    {
        int startIndex = 0;
        int count = 0;
        int additionalCharOffset = 0;
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString);
        if ((res == 0) || (res == 2))
        {
            if (res == 2)
                startIndex = luaToInt(L, 2);

            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString);
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    count = luaToInt(L, 3);

                res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, true, false, &errorString);
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        additionalCharOffset = luaToInt(L, 4);

                    size_t dataLength;
                    char* data = ((char*)luaWrap_lua_tobuffer(L, 1, &dataLength)) + additionalCharOffset;
                    dataLength = sizeof(double) * ((dataLength - additionalCharOffset) / sizeof(double));
                    int packetCount = int(dataLength / sizeof(double));
                    if (count == 0)
                        count = int(1999999999);

                    if (dataLength == 0)
                    { // since 20.03.2024: empty buffer results in an empty table
                        luaWrap_lua_newtable(L);
                        LUA_END(1);
                    }
                    else
                    {
                        if ((startIndex >= 0) && (startIndex < packetCount))
                        {
                            if (startIndex + count > packetCount)
                                count = packetCount - startIndex;

                            double* theDoubles = new double[count];
                            for (int i = 0; i < int(count); i++)
                            {
                                double v;
                                ((char*)&v)[0] = data[sizeof(double) * (i + startIndex) + 0];
                                ((char*)&v)[1] = data[sizeof(double) * (i + startIndex) + 1];
                                ((char*)&v)[2] = data[sizeof(double) * (i + startIndex) + 2];
                                ((char*)&v)[3] = data[sizeof(double) * (i + startIndex) + 3];
                                ((char*)&v)[4] = data[sizeof(double) * (i + startIndex) + 4];
                                ((char*)&v)[5] = data[sizeof(double) * (i + startIndex) + 5];
                                ((char*)&v)[6] = data[sizeof(double) * (i + startIndex) + 6];
                                ((char*)&v)[7] = data[sizeof(double) * (i + startIndex) + 7];
                                theDoubles[i] = v;
                            }
                            pushDoubleTableOntoStack(L, count, theDoubles);
                            delete[] theDoubles;
                            LUA_END(1);
                        }
                    }
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simUnpackUInt8Table(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.unpackUInt8Table");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0))
    {
        int startIndex = 0;
        int count = 0;
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString);
        if ((res == 0) || (res == 2))
        {
            if (res == 2)
                startIndex = luaToInt(L, 2);

            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString);
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    count = luaToInt(L, 3);

                size_t dataLength;
                const char* data = (char*)luaWrap_lua_tobuffer(L, 1, &dataLength);
                int packetCount = (int)dataLength;
                if (count == 0)
                    count = int(1999999999);

                if (dataLength == 0)
                { // since 20.03.2024: empty buffer results in an empty table
                    luaWrap_lua_newtable(L);
                    LUA_END(1);
                }
                else
                {
                    if ((startIndex >= 0) && (startIndex < packetCount))
                    {
                        if (startIndex + count > packetCount)
                            count = packetCount - startIndex;

                        int* theBytes = new int[count];
                        for (int i = 0; i < count; i++)
                        {
                            unsigned char v;
                            ((char*)&v)[0] = data[i + startIndex];
                            theBytes[i] = v;
                        }
                        pushIntTableOntoStack(L, count, theBytes);
                        delete[] theBytes;
                        LUA_END(1);
                    }
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simUnpackUInt16Table(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.unpackUInt16Table");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0))
    {
        int startIndex = 0;
        int count = 0;
        int additionalCharOffset = 0;
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString);
        if ((res == 0) || (res == 2))
        {
            if (res == 2)
                startIndex = luaToInt(L, 2);

            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString);
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    count = luaToInt(L, 3);

                res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, true, false, &errorString);
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        additionalCharOffset = luaToInt(L, 4);

                    size_t dataLength;
                    char* data = ((char*)luaWrap_lua_tobuffer(L, 1, &dataLength)) + additionalCharOffset;
                    dataLength = 2 * ((dataLength - additionalCharOffset) / 2);
                    int packetCount = (int)dataLength / 2;
                    if (count == 0)
                        count = int(1999999999);

                    if (dataLength == 0)
                    { // since 20.03.2024: empty buffer results in an empty table
                        luaWrap_lua_newtable(L);
                        LUA_END(1);
                    }
                    else
                    {
                        if ((startIndex >= 0) && (startIndex < packetCount))
                        {
                            if (startIndex + count > packetCount)
                                count = packetCount - startIndex;

                            int* theWords = new int[count];
                            for (int i = 0; i < int(count); i++)
                            {
                                unsigned short v;
                                ((char*)&v)[0] = data[2 * (i + startIndex) + 0];
                                ((char*)&v)[1] = data[2 * (i + startIndex) + 1];
                                theWords[i] = v;
                            }
                            pushIntTableOntoStack(L, count, theWords);
                            delete[] theWords;
                            LUA_END(1);
                        }
                    }
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simTransformBuffer(luaWrap_lua_State* L)
{ // string inBuffer,number inFormat,number multiplier,number offset,number outFormat
    TRACE_LUA_API;
    LUA_START("sim.transformBuffer");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0,
                            lua_arg_number, 0))
    {
        size_t dataLength;
        int inFormat = luaToInt(L, 2);
        int outFormat = luaToInt(L, 5);
        bool clamp = (outFormat & sim_buffer_clamp) != 0;
        if (clamp)
            outFormat -= sim_buffer_clamp;
        bool something = false;
        if (inFormat == sim_buffer_float)
        {
            double mult = luaToDouble(L, 3);
            double off = luaToDouble(L, 4);
            bool noScalingNorOffset = ((mult == 1.0) && (off == 0.0));
            const float* data = (const float*)luaWrap_lua_tobuffer(L, 1, &dataLength);
            something = true;
            dataLength -= (dataLength % sizeof(float));
            dataLength /= sizeof(float);
            if (dataLength != 0)
            {
                if (outFormat == sim_buffer_uint8)
                {
                    unsigned char* dat = new unsigned char[dataLength];
                    if (clamp)
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                float v = data[i];
                                dat[i] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                float v = data[i] * mult + off;
                                dat[i] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                            }
                        }
                    }
                    else
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (unsigned char)(data[i]);
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (unsigned char)(data[i] * mult + off);
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength);
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_uint8rgb)
                {
                    unsigned char* dat = new unsigned char[dataLength * 3];
                    if (clamp)
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                float v = data[i];
                                dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                dat[3 * i + 1] = dat[3 * i + 0];
                                dat[3 * i + 2] = dat[3 * i + 0];
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                float v = data[i] * mult + off;
                                dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                dat[3 * i + 1] = dat[3 * i + 0];
                                dat[3 * i + 2] = dat[3 * i + 0];
                            }
                        }
                    }
                    else
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[3 * i + 0] = (unsigned char)data[i];
                                dat[3 * i + 1] = dat[3 * i + 0];
                                dat[3 * i + 2] = dat[3 * i + 0];
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[3 * i + 0] = (unsigned char)(data[i] * mult + off);
                                dat[3 * i + 1] = dat[3 * i + 0];
                                dat[3 * i + 2] = dat[3 * i + 0];
                            }
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength * 3);
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_int8)
                {
                    char* dat = new char[dataLength];
                    if (clamp)
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                float v = data[i];
                                dat[i] = (v < -128.499) ? (-128) : ((v > 127.499) ? (127) : ((char)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                float v = data[i] * mult + off;
                                dat[i] = (v < -128.499) ? (-128) : ((v > 127.499) ? (127) : ((char)v));
                            }
                        }
                    }
                    else
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (char)data[i];
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (char)(data[i] * mult + off);
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength);
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_uint16)
                {
                    uint16_t* dat = new uint16_t[dataLength];
                    if (clamp)
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                float v = data[i];
                                dat[i] = (v < 0.0) ? (0) : ((v > 65535.499) ? (65535) : ((uint16_t)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                float v = data[i] * mult + off;
                                dat[i] = (v < 0.0) ? (0) : ((v > 65535.499) ? (65535) : ((uint16_t)v));
                            }
                        }
                    }
                    else
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (uint16_t)data[i];
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (uint16_t)(data[i] * mult + off);
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength * sizeof(uint16_t));
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_int16)
                {
                    int16_t* dat = new int16_t[dataLength];
                    if (clamp)
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                float v = data[i];
                                dat[i] = (v < -32768.499) ? (-32768) : ((v > 32767.499) ? (32767) : ((int16_t)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                float v = data[i] * mult + off;
                                dat[i] = (v < -32768.499) ? (-32768) : ((v > 32767.499) ? (32767) : ((int16_t)v));
                            }
                        }
                    }
                    else
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (int16_t)data[i];
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (int16_t)(data[i] * mult + off);
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength * sizeof(int16_t));
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_uint32)
                {
                    uint32_t* dat = new uint32_t[dataLength];
                    if (clamp)
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                float v = data[i];
                                dat[i] = (v < 0.0) ? (0) : ((v > 4294967295.499) ? (4294967295) : ((uint32_t)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                float v = data[i] * mult + off;
                                dat[i] = (v < 0.0) ? (0) : ((v > 4294967295.499) ? (4294967295) : ((uint32_t)v));
                            }
                        }
                    }
                    else
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (uint32_t)data[i];
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (uint32_t)(data[i] * mult + off);
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength * sizeof(uint32_t));
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_int32)
                {
                    int32_t* dat = new int32_t[dataLength];
                    if (clamp)
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                float v = data[i];
                                dat[i] = (v < -2147483648.499) ? (-2147483647)
                                                               : ((v > 2147483647.499) ? (2147483647) : ((int32_t)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                float v = data[i] * mult + off;
                                dat[i] = (v < -2147483648.499) ? (-2147483647)
                                                               : ((v > 2147483647.499) ? (2147483647) : ((int32_t)v));
                            }
                        }
                    }
                    else
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (int32_t)data[i];
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (int32_t)(data[i] * mult + off);
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength * sizeof(int32_t));
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_float)
                {
                    float* dat = new float[dataLength];
                    if (noScalingNorOffset)
                    {
                        for (size_t i = 0; i < dataLength; i++)
                            dat[i] = data[i];
                    }
                    else
                    {
                        for (size_t i = 0; i < dataLength; i++)
                            dat[i] = data[i] * mult + off;
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength * sizeof(float));
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_double)
                {
                    double* dat = new double[dataLength];
                    if (noScalingNorOffset)
                    {
                        for (size_t i = 0; i < dataLength; i++)
                            dat[i] = (double)data[i];
                    }
                    else
                    {
                        for (size_t i = 0; i < dataLength; i++)
                            dat[i] = (double)(data[i] * mult + off);
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength * sizeof(double));
                    delete[] dat;
                    LUA_END(1);
                }
                errorString = SIM_ERROR_INVALID_FORMAT;
            }
            else
                errorString = SIM_ERROR_INVALID_DATA;
        }
        if (inFormat == sim_buffer_double)
        {
            double mult = luaToDouble(L, 3);
            double off = luaToDouble(L, 4);
            bool noScalingNorOffset = ((mult == 1.0) && (off == 0.0));
            const double* data = (const double*)luaWrap_lua_tobuffer(L, 1, &dataLength);
            something = true;
            dataLength -= (dataLength % sizeof(double));
            dataLength /= sizeof(double);
            if (dataLength != 0)
            {
                if (outFormat == sim_buffer_uint8)
                {
                    unsigned char* dat = new unsigned char[dataLength];
                    if (clamp)
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = data[i];
                                dat[i] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = data[i] * mult + off;
                                dat[i] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                            }
                        }
                    }
                    else
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (unsigned char)data[i];
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (unsigned char)(data[i] * mult + off);
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength);
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_uint8rgb)
                {
                    unsigned char* dat = new unsigned char[dataLength * 3];
                    if (clamp)
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = data[i];
                                dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                dat[3 * i + 1] = dat[3 * i + 0];
                                dat[3 * i + 2] = dat[3 * i + 0];
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = data[i] * mult + off;
                                dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                dat[3 * i + 1] = dat[3 * i + 0];
                                dat[3 * i + 2] = dat[3 * i + 0];
                            }
                        }
                    }
                    else
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[3 * i + 0] = (unsigned char)data[i];
                                dat[3 * i + 1] = dat[3 * i + 0];
                                dat[3 * i + 2] = dat[3 * i + 0];
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[3 * i + 0] = (unsigned char)(data[i] * mult + off);
                                dat[3 * i + 1] = dat[3 * i + 0];
                                dat[3 * i + 2] = dat[3 * i + 0];
                            }
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength * 3);
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_int8)
                {
                    char* dat = new char[dataLength];
                    if (clamp)
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = data[i];
                                dat[i] = (v < -128.499) ? (-128) : ((v > 127.499) ? (127) : ((char)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = data[i] * mult + off;
                                dat[i] = (v < -128.499) ? (-128) : ((v > 127.499) ? (127) : ((char)v));
                            }
                        }
                    }
                    else
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (char)data[i];
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (char)(data[i] * mult + off);
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength);
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_uint16)
                {
                    uint16_t* dat = new uint16_t[dataLength];
                    if (clamp)
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = data[i];
                                dat[i] = (v < 0.0) ? (0) : ((v > 65535.499) ? (65535) : ((uint16_t)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = data[i] * mult + off;
                                dat[i] = (v < 0.0) ? (0) : ((v > 65535.499) ? (65535) : ((uint16_t)v));
                            }
                        }
                    }
                    else
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (uint16_t)data[i];
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (uint16_t)(data[i] * mult + off);
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength * sizeof(uint16_t));
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_int16)
                {
                    int16_t* dat = new int16_t[dataLength];
                    if (clamp)
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = data[i];
                                dat[i] = (v < -32768.499) ? (-32768) : ((v > 32767.499) ? (32767) : ((int16_t)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = data[i] * mult + off;
                                dat[i] = (v < -32768.499) ? (-32768) : ((v > 32767.499) ? (32767) : ((int16_t)v));
                            }
                        }
                    }
                    else
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (int16_t)data[i];
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (int16_t)(data[i] * mult + off);
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength * sizeof(int16_t));
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_uint32)
                {
                    uint32_t* dat = new uint32_t[dataLength];
                    if (clamp)
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = data[i];
                                dat[i] = (v < 0.0) ? (0) : ((v > 4294967295.499) ? (4294967295) : ((uint32_t)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = data[i] * mult + off;
                                dat[i] = (v < 0.0) ? (0) : ((v > 4294967295.499) ? (4294967295) : ((uint32_t)v));
                            }
                        }
                    }
                    else
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (uint32_t)data[i];
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (uint32_t)(data[i] * mult + off);
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength * sizeof(uint32_t));
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_int32)
                {
                    int32_t* dat = new int32_t[dataLength];
                    if (clamp)
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = data[i];
                                dat[i] = (v < -2147483648.499) ? (-2147483647)
                                                               : ((v > 2147483647.499) ? (2147483647) : ((int32_t)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = data[i] * mult + off;
                                dat[i] = (v < -2147483648.499) ? (-2147483647)
                                                               : ((v > 2147483647.499) ? (2147483647) : ((int32_t)v));
                            }
                        }
                    }
                    else
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (int32_t)data[i];
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (int32_t)(data[i] * mult + off);
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength * sizeof(int32_t));
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_float)
                {
                    float* dat = new float[dataLength];
                    if (clamp)
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = data[i];
                                dat[i] = (v < -FLT_MAX) ? (-FLT_MAX) : ((v > FLT_MAX) ? (FLT_MAX) : ((float)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = data[i] * mult + off;
                                dat[i] = (v < -FLT_MAX) ? (-FLT_MAX) : ((v > FLT_MAX) ? (FLT_MAX) : ((float)v));
                            }
                        }
                    }
                    else
                    {
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (float)data[i];
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (float)(data[i] * mult + off);
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength * sizeof(float));
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_double)
                {
                    double* dat = new double[dataLength];
                    if (noScalingNorOffset)
                    {
                        for (size_t i = 0; i < dataLength; i++)
                            dat[i] = data[i];
                    }
                    else
                    {
                        for (size_t i = 0; i < dataLength; i++)
                            dat[i] = data[i] * mult + off;
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength * sizeof(double));
                    delete[] dat;
                    LUA_END(1);
                }
                errorString = SIM_ERROR_INVALID_FORMAT;
            }
            else
                errorString = SIM_ERROR_INVALID_DATA;
        }
        if ((inFormat == sim_buffer_uint8rgb) || (inFormat == sim_buffer_uint8bgr))
        {
            double mult = luaToDouble(L, 3);
            double off = luaToDouble(L, 4);
            bool noScalingNorOffset = ((mult == 1.0) && (off == 0.0));
            const unsigned char* data = (const unsigned char*)luaWrap_lua_tobuffer(L, 1, &dataLength);
            something = true;
            dataLength -= (dataLength % 3);
            dataLength /= 3;
            if (dataLength != 0)
            {
                if (((inFormat == sim_buffer_uint8rgb) && (outFormat == sim_buffer_uint8bgr)) ||
                    ((inFormat == sim_buffer_uint8bgr) && (outFormat == sim_buffer_uint8rgb)))
                {
                    unsigned char* dat = new unsigned char[3 * dataLength];
                    if (noScalingNorOffset)
                    {
                        for (size_t i = 0; i < dataLength; i++)
                        {
                            dat[3 * i + 0] = data[3 * i + 2];
                            dat[3 * i + 1] = data[3 * i + 1];
                            dat[3 * i + 2] = data[3 * i + 0];
                        }
                    }
                    else
                    {
                        if (clamp)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = double(data[3 * i + 2]) * mult + off;
                                dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                v = double(data[3 * i + 1]) * mult + off;
                                dat[3 * i + 1] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                v = double(data[3 * i + 0]) * mult + off;
                                dat[3 * i + 2] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[3 * i + 0] = (unsigned char)(double(data[3 * i + 2]) * mult + off);
                                dat[3 * i + 1] = (unsigned char)(double(data[3 * i + 1]) * mult + off);
                                dat[3 * i + 2] = (unsigned char)(double(data[3 * i + 0]) * mult + off);
                            }
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, 3 * dataLength);
                    delete[] dat;
                    LUA_END(1);
                }
                if (inFormat == outFormat)
                {
                    unsigned char* dat = new unsigned char[3 * dataLength];
                    if (noScalingNorOffset)
                    {
                        for (size_t i = 0; i < dataLength; i++)
                        {
                            dat[3 * i + 0] = data[3 * i + 0];
                            dat[3 * i + 1] = data[3 * i + 1];
                            dat[3 * i + 2] = data[3 * i + 2];
                        }
                    }
                    else
                    {
                        if (clamp)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = double(data[3 * i + 0]) * mult + off;
                                dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                v = double(data[3 * i + 1]) * mult + off;
                                dat[3 * i + 1] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                v = double(data[3 * i + 2]) * mult + off;
                                dat[3 * i + 2] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[3 * i + 0] = (unsigned char)(double(data[3 * i + 0]) * mult + off);
                                dat[3 * i + 1] = (unsigned char)(double(data[3 * i + 1]) * mult + off);
                                dat[3 * i + 2] = (unsigned char)(double(data[3 * i + 2]) * mult + off);
                            }
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, 3 * dataLength);
                    delete[] dat;
                    LUA_END(1);
                }
                errorString = SIM_ERROR_INVALID_FORMAT;
            }
            else
                errorString = SIM_ERROR_INVALID_DATA;
        }
        if (inFormat == sim_buffer_uint8rgba)
        {
            double mult = luaToDouble(L, 3);
            double off = luaToDouble(L, 4);
            bool noScalingNorOffset = ((mult == 1.0) && (off == 0.0));
            const unsigned char* data = (const unsigned char*)luaWrap_lua_tobuffer(L, 1, &dataLength);
            something = true;
            dataLength -= (dataLength % 4);
            dataLength /= 4;
            if (dataLength != 0)
            {
                if (outFormat == sim_buffer_uint8rgb)
                {
                    unsigned char* dat = new unsigned char[3 * dataLength];
                    if (noScalingNorOffset)
                    {
                        for (size_t i = 0; i < dataLength; i++)
                        {
                            dat[3 * i + 0] = data[4 * i + 0];
                            dat[3 * i + 1] = data[4 * i + 1];
                            dat[3 * i + 2] = data[4 * i + 2];
                        }
                    }
                    else
                    {
                        if (clamp)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = double(data[4 * i + 0]) * mult + off;
                                dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                v = double(data[4 * i + 1]) * mult + off;
                                dat[3 * i + 1] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                v = double(data[4 * i + 2]) * mult + off;
                                dat[3 * i + 2] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[3 * i + 0] = (unsigned char)(double(data[4 * i + 0]) * mult + off);
                                dat[3 * i + 1] = (unsigned char)(double(data[4 * i + 1]) * mult + off);
                                dat[3 * i + 2] = (unsigned char)(double(data[4 * i + 2]) * mult + off);
                            }
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, 3 * dataLength);
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == inFormat)
                {
                    unsigned char* dat = new unsigned char[4 * dataLength];
                    if (noScalingNorOffset)
                    {
                        for (size_t i = 0; i < dataLength; i++)
                        {
                            dat[4 * i + 0] = data[4 * i + 0];
                            dat[4 * i + 1] = data[4 * i + 1];
                            dat[4 * i + 2] = data[4 * i + 2];
                            dat[4 * i + 3] = data[4 * i + 3];
                        }
                    }
                    else
                    {
                        if (clamp)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = double(data[4 * i + 0]) * mult + off;
                                dat[4 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                v = double(data[4 * i + 1]) * mult + off;
                                dat[4 * i + 1] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                v = double(data[4 * i + 2]) * mult + off;
                                dat[4 * i + 2] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                v = double(data[4 * i + 3]) * mult + off;
                                dat[4 * i + 3] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[4 * i + 0] = (unsigned char)(double(data[4 * i + 0]) * mult + off);
                                dat[4 * i + 1] = (unsigned char)(double(data[4 * i + 1]) * mult + off);
                                dat[4 * i + 2] = (unsigned char)(double(data[4 * i + 2]) * mult + off);
                                dat[4 * i + 3] = (unsigned char)(double(data[4 * i + 3]) * mult + off);
                            }
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, 4 * dataLength);
                    delete[] dat;
                    LUA_END(1);
                }
                errorString = SIM_ERROR_INVALID_FORMAT;
            }
            else
                errorString = SIM_ERROR_INVALID_DATA;
        }
        if (inFormat == sim_buffer_uint8argb)
        {
            double mult = luaToDouble(L, 3);
            double off = luaToDouble(L, 4);
            bool noScalingNorOffset = ((mult == 1.0) && (off == 0.0));
            const unsigned char* data = (const unsigned char*)luaWrap_lua_tobuffer(L, 1, &dataLength);
            something = true;
            dataLength -= (dataLength % 4);
            dataLength /= 4;
            if (dataLength != 0)
            {
                if (outFormat == sim_buffer_uint8rgb)
                {
                    unsigned char* dat = new unsigned char[3 * dataLength];
                    if (noScalingNorOffset)
                    {
                        for (size_t i = 0; i < dataLength; i++)
                        {
                            dat[3 * i + 0] = data[4 * i + 1];
                            dat[3 * i + 1] = data[4 * i + 2];
                            dat[3 * i + 2] = data[4 * i + 3];
                        }
                    }
                    else
                    {
                        if (clamp)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = double(data[4 * i + 1]) * mult + off;
                                dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                v = double(data[4 * i + 2]) * mult + off;
                                dat[3 * i + 1] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                v = double(data[4 * i + 3]) * mult + off;
                                dat[3 * i + 2] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[3 * i + 0] = (unsigned char)(double(data[4 * i + 1]) * mult + off);
                                dat[3 * i + 1] = (unsigned char)(double(data[4 * i + 2]) * mult + off);
                                dat[3 * i + 2] = (unsigned char)(double(data[4 * i + 3]) * mult + off);
                            }
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, 3 * dataLength);
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == inFormat)
                {
                    unsigned char* dat = new unsigned char[4 * dataLength];
                    if (noScalingNorOffset)
                    {
                        for (size_t i = 0; i < dataLength; i++)
                        {
                            dat[4 * i + 0] = data[4 * i + 0];
                            dat[4 * i + 1] = data[4 * i + 1];
                            dat[4 * i + 2] = data[4 * i + 2];
                            dat[4 * i + 3] = data[4 * i + 3];
                        }
                    }
                    else
                    {
                        if (clamp)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = double(data[4 * i + 0]) * mult + off;
                                dat[4 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                v = double(data[4 * i + 1]) * mult + off;
                                dat[4 * i + 1] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                v = double(data[4 * i + 2]) * mult + off;
                                dat[4 * i + 2] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                v = double(data[4 * i + 3]) * mult + off;
                                dat[4 * i + 3] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[4 * i + 0] = (unsigned char)(double(data[4 * i + 0]) * mult + off);
                                dat[4 * i + 1] = (unsigned char)(double(data[4 * i + 1]) * mult + off);
                                dat[4 * i + 2] = (unsigned char)(double(data[4 * i + 2]) * mult + off);
                                dat[4 * i + 3] = (unsigned char)(double(data[4 * i + 3]) * mult + off);
                            }
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, 4 * dataLength);
                    delete[] dat;
                    LUA_END(1);
                }
                errorString = SIM_ERROR_INVALID_FORMAT;
            }
            else
                errorString = SIM_ERROR_INVALID_DATA;
        }
        if (inFormat == sim_buffer_uint8)
        {
            double mult = luaToDouble(L, 3);
            double off = luaToDouble(L, 4);
            bool noScalingNorOffset = ((mult == 1.0) && (off == 0.0));
            const unsigned char* data = (const unsigned char*)luaWrap_lua_tobuffer(L, 1, &dataLength);
            something = true;
            if (dataLength != 0)
            {
                if (outFormat == sim_buffer_split)
                {
                    size_t splitSize = size_t(luaToInt(L, 4));
                    if (splitSize > 0)
                    {
                        std::vector<std::string> stringTable;
                        size_t of = 0;
                        while (of < dataLength)
                        {
                            size_t of2 = of + splitSize;
                            if (of2 > dataLength)
                                of2 = dataLength;
                            std::string astr;
                            astr.assign(data + of, data + of2);
                            stringTable.push_back(astr);
                            of += splitSize;
                        }
                        pushBufferTableOntoStack(L, stringTable);
                        LUA_END(1);
                    }
                }
                if (outFormat == sim_buffer_base64)
                {
                    std::string inDat(data, data + dataLength);
                    std::string outDat(utils::encode64(inDat));
                    luaWrap_lua_pushbuffer(L, outDat.c_str(), outDat.length());
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_uint8)
                {
                    unsigned char* dat = new unsigned char[dataLength];
                    if (noScalingNorOffset)
                    {
                        for (size_t i = 0; i < dataLength; i++)
                            dat[i] = data[i];
                    }
                    else
                    {
                        if (clamp)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = double(data[i]) * mult + off;
                                dat[i] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (unsigned char)(double(data[i]) * mult + off);
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength);
                    delete[] dat;
                    LUA_END(1);
                }
                if (outFormat == sim_buffer_uint8rgb)
                {
                    unsigned char* dat = new unsigned char[3 * dataLength];
                    if (noScalingNorOffset)
                    {
                        for (size_t i = 0; i < dataLength; i++)
                        {
                            dat[3 * i + 0] = data[i];
                            dat[3 * i + 1] = data[i];
                            dat[3 * i + 2] = data[i];
                        }
                    }
                    else
                    {
                        if (clamp)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = double(data[i]) * mult + off;
                                dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                                dat[3 * i + 1] = dat[3 * i + 0];
                                dat[3 * i + 2] = dat[3 * i + 0];
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[3 * i + 0] = (unsigned char)(double(data[i]) * mult + off);
                                dat[3 * i + 1] = dat[3 * i + 0];
                                dat[3 * i + 2] = dat[3 * i + 0];
                            }
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, 3 * dataLength);
                    delete[] dat;
                    LUA_END(1);
                }
                errorString = SIM_ERROR_INVALID_FORMAT;
            }
            else
                errorString = SIM_ERROR_INVALID_DATA;
        }
        if ((inFormat == sim_buffer_uint8rgb) || (inFormat == sim_buffer_uint8bgr))
        {
            double mult = luaToDouble(L, 3);
            double off = luaToDouble(L, 4);
            bool noScalingNorOffset = ((mult == 1.0) && (off == 0.0));
            const unsigned char* data = (const unsigned char*)luaWrap_lua_tobuffer(L, 1, &dataLength);
            something = true;
            dataLength -= (dataLength % 3);
            dataLength /= 3;
            if (dataLength != 0)
            {
                if (outFormat == sim_buffer_uint8)
                {
                    unsigned char* dat = new unsigned char[dataLength];
                    if (noScalingNorOffset)
                    {
                        for (size_t i = 0; i < dataLength; i++)
                            dat[i] = (int(data[3 * i + 0]) + int(data[3 * i + 1]) + int(data[3 * i + 2])) / 3;
                    }
                    else
                    {
                        if (clamp)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = off + (double(data[3 * i + 0]) + double(data[3 * i + 1]) +
                                                  double(data[3 * i + 2])) *
                                                     mult / 3.0;
                                dat[i] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((unsigned char)v));
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                double v = off + (double(data[3 * i + 0]) + double(data[3 * i + 1]) +
                                                  double(data[3 * i + 2])) *
                                                     mult / 3.0;
                                dat[i] = (unsigned char)v;
                            }
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, dataLength);
                    delete[] dat;
                    LUA_END(1);
                }
                errorString = SIM_ERROR_INVALID_FORMAT;
            }
            else
                errorString = SIM_ERROR_INVALID_DATA;
        }
        if (inFormat == sim_buffer_base64)
        {
            const unsigned char* data = (const unsigned char*)luaWrap_lua_tobuffer(L, 1, &dataLength);
            something = true;
            if (dataLength != 0)
            {
                if (outFormat == sim_buffer_uint8)
                {
                    std::string inDat(data, data + dataLength);
                    std::string outDat(utils::decode64(inDat));
                    luaWrap_lua_pushbuffer(L, outDat.c_str(), outDat.length());
                    LUA_END(1);
                }
                errorString = SIM_ERROR_INVALID_FORMAT;
            }
            else
                errorString = SIM_ERROR_INVALID_DATA;
        }

        if (!something)
            errorString = SIM_ERROR_INVALID_FORMAT;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simCombineRgbImages(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.combineRgbImages");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 2, lua_arg_string, 0, lua_arg_number, 2,
                            lua_arg_number, 0))
    {
        size_t img1Length, img2Length;
        char* img1 = (char*)luaWrap_lua_tobuffer(L, 1, &img1Length);
        char* img2 = (char*)luaWrap_lua_tobuffer(L, 3, &img2Length);
        int res1[2];
        int res2[2];
        getIntsFromTable(L, 2, 2, res1);
        getIntsFromTable(L, 4, 2, res2);
        int op = luaToInt(L, 5);
        if ((img1Length >= size_t(res1[0] * res1[1] * 3)) && (img2Length >= size_t(res2[0] * res2[1] * 3)))
        {
            if (op == sim_imgcomb_vertical)
            {
                if (res1[0] == res2[0])
                {
                    unsigned char* dat = new unsigned char[(res1[0] * res1[1] + res2[0] * res2[1]) * 3];
                    size_t l = size_t(res1[0] * res1[1] * 3);
                    for (size_t i = 0; i < l; i++)
                        dat[i] = img1[i];
                    for (size_t i = 0; i < size_t(res2[0] * res2[1] * 3); i++)
                        dat[l + i] = img2[i];
                    luaWrap_lua_pushbuffer(L, (const char*)dat, (res1[0] * res1[1] + res2[0] * res2[1]) * 3);
                    delete[] dat;
                    LUA_END(1);
                }
                else
                    errorString = SIM_ERROR_INVALID_RESOLUTION;
            }
            if (op == sim_imgcomb_horizontal)
            {
                if (res1[1] == res2[1])
                {
                    unsigned char* dat = new unsigned char[(res1[0] * res1[1] + res2[0] * res2[1]) * 3];
                    for (size_t y = 0; y < size_t(res1[1]); y++)
                    {
                        int off1 = (int)y * res1[0] * 3;
                        int off2 = (int)y * res2[0] * 3;
                        int off3 = off1 + off2;
                        for (size_t i = 0; i < size_t(res1[0] * 3); i++)
                            dat[off3 + i] = img1[off1 + i];
                        off3 += res1[0] * 3;
                        for (size_t i = 0; i < size_t(res2[0] * 3); i++)
                            dat[off3 + i] = img2[off2 + i];
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)dat, (res1[0] * res1[1] + res2[0] * res2[1]) * 3);
                    delete[] dat;
                    LUA_END(1);
                }
                else
                    errorString = SIM_ERROR_INVALID_RESOLUTION;
            }
        }
        else
            errorString = SIM_ERROR_INVALID_DATA;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simAddDrawingObject(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.addDrawingObject");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number | lua_arg_optional, 0, lua_arg_number | lua_arg_optional, 0, lua_arg_number | lua_arg_optional, 0, lua_arg_number | lua_arg_optional, 0, lua_arg_number | lua_arg_optional, 3, lua_arg_number | lua_arg_optional, 3, lua_arg_number | lua_arg_optional, 3))
    {
        int objType = fetchIntArg(L, 1);
        double size = fetchDoubleArg(L, 2, 1.0);
        double duplicateTolerance = fetchDoubleArg(L, 3, 0.0);
        int parentID = fetchIntArg(L, 4, -1);
        int maxItemCount = fetchIntArg(L, 5, 1000);
        std::vector<float> ambient;
        fetchFloatArrayArg(L, 6, ambient, {0.0f, 0.0f, 0.0f});
        std::vector<float> specular;
        fetchFloatArrayArg(L, 6, specular, {0.25f, 0.25f, 0.25f});
        std::vector<float> emission;
        fetchFloatArrayArg(L, 6, emission, {0.0f, 0.0f, 0.0f});
        setCurrentScriptInfo_cSide(CScriptObject::getScriptHandleFromInterpreterState_lua(L), CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(L)); // for transmitting to the master function additional info (e.g.for autom. name adjustment, or for autom. object deletion when script ends)
        retVal = CALL_C_API(simAddDrawingObject, objType, size, duplicateTolerance, parentID, maxItemCount, ambient.data(), nullptr, specular.data(), emission.data());
        setCurrentScriptInfo_cSide(-1, -1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simRemoveDrawingObject(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.removeDrawingObject");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int objectHandle = luaToInt(L, 1);
        if (objectHandle == sim_handle_all)
        { // following condition added here on 2011/01/06 so as not to remove objects created from a c/c++ call or from
            // add-on:
            int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
            CScriptObject* itScrObj = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
            App::currentWorld->drawingCont->eraseAllObjects();
            retVal = 1;
        }
        else
            retVal = CALL_C_API(simRemoveDrawingObject, objectHandle);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAddDrawingObjectItem(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.addDrawingObjectItem");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int h = luaToInt(L, 1);
        int handleFlags = h & 0xff00000;
        h = h & 0xfffff;
        CDrawingObject* it = App::currentWorld->drawingCont->getObject(h);
        size_t d = 3;
        if (it != nullptr)
        {
            d = size_t(it->getExpectedFloatsPerItem());
            if ((handleFlags & (sim_handleflag_addmultiple | sim_handleflag_codedstring)) == (sim_handleflag_addmultiple | sim_handleflag_codedstring))
            { // data provided as coded string
                if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_buffer | lua_arg_optional, 0))
                {
                    if (!isArgNilOrMissing(L, 2))
                    {
                        std::string data = fetchBufferArg(L, 2);
                        size_t itemCnt = (data.size() / sizeof(float)) / d;
                        if (itemCnt > 0)
                        {
                            std::vector<double> vertices;
                            vertices.resize(itemCnt * d);
                            for (size_t i = 0; i < itemCnt * d; i++)
                                vertices[i] = double(((float*)data.data())[i]);
                            it->addItems(vertices.data(), itemCnt);
                            retVal = 1;
                        }
                        else
                            errorString = "bad argument #2 (invalid buffer size)";
                    }
                    else
                    { // old. back compatibility: clear the drawing object
                        if (it->addItem(nullptr))
                            retVal = 1;
                        else
                            retVal = 0;
                    }
                }
            }
            else
            { // data provided as table
                if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_table | lua_arg_optional, 0))
                {
                    if (!isArgNilOrMissing(L, 2))
                    {
                        std::vector<double> vertices;
                        fetchDoubleArrayArg(L, 2, vertices);
                        if (int(d) <= vertices.size())
                        { // append data from table
                            if ((((handleFlags & sim_handleflag_setmultiple) != 0) || ((handleFlags & sim_handleflag_addmultiple) != 0)))
                            { // append multiple
                                size_t itemCnt = vertices.size() / d;
                                if ((handleFlags & sim_handleflag_addmultiple) != 0)
                                    it->addItems(&vertices[0], itemCnt);
                                else
                                    it->setItems(&vertices[0], itemCnt); // old. Previously setmultiple, which is now deprecated
                                retVal = 1;
                            }
                            else
                            { // append single
                                if (it->addItem(vertices.data()))
                                    retVal = 1;
                                else
                                    retVal = 0;
                            }
                        }
                        else
                        { // empty table: we clear the drawing object
                            if (it->addItem(nullptr))
                                retVal = 1;
                            else
                                retVal = 0;
                        }
                    }
                    else
                    { // old. Back compatibility: clear the drawing object
                        if (it->addItem(nullptr))
                            retVal = 1;
                        else
                            retVal = 0;
                    }
                }
            }
        }
        else
            errorString = SIM_ERROR_OBJECT_INEXISTANT;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAddParticleObject(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.addParticleObject");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        // The 4th argument can be nil or a table. Check for that:
        if (luaWrap_lua_gettop(L) < 4)
            errorString = SIM_ERROR_FUNCTION_REQUIRES_MORE_ARGUMENTS;
        else
        {
            if ((!luaWrap_lua_isnil(L, 4)) && ((!luaWrap_lua_isnonbuffertable(L, 4)) || (int(luaWrap_lua_rawlen(L, 4)) < 3)))
                errorString = SIM_ERROR_ONE_ARGUMENT_TYPE_IS_WRONG;
            else
            {
                if ((checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, false, false, &errorString) == 2) &&
                    (checkOneGeneralInputArgument(L, 6, lua_arg_number, 0, false, false, &errorString) == 2))
                {
                    int objType = luaToInt(L, 1);
                    double size = luaToDouble(L, 2);
                    double massOverVolume = luaToDouble(L, 3);
                    int paramLen = int(luaWrap_lua_rawlen(L, 4));
                    paramLen = (paramLen - 1) / 2;
                    paramLen = paramLen * 2 + 1;
                    void* params = nullptr;
                    if (!luaWrap_lua_isnil(L, 4))
                    {
                        int intParams[30];
                        double floatParams[30];
                        getIntsFromTable(L, 4, paramLen, intParams);
                        getDoublesFromTable(L, 4, paramLen, floatParams);
                        params = new char[paramLen * sizeof(int)];
                        ((int*)params)[0] = std::min<int>(intParams[0], (paramLen - 1) / 2);
                        for (int i = 0; i < (paramLen - 1) / 2; i++)
                        {
                            ((int*)params)[1 + 2 * i] = intParams[1 + 2 * i + 0];
                            ((float*)params)[1 + 2 * i + 1] = (float)(floatParams[1 + 2 * i + 1]);
                        }
                    }
                    double lifeTime = luaToDouble(L, 5);
                    int maxItemCount = luaToInt(L, 6);
                    float* ambient = nullptr;
                    float* specular = nullptr;
                    float* emission = nullptr;
                    int res = checkOneGeneralInputArgument(L, 7, lua_arg_number, 3, true, true, &errorString);
                    int okToGo = (res != -1);
                    if (okToGo)
                    {
                        float ambientC[3];
                        float specularC[3];
                        float emissionC[3];
                        if (res > 0)
                        {
                            if (res == 2)
                            {
                                getFloatsFromTable(L, 7, 3, ambientC);
                                ambient = ambientC;
                            }
                            res = checkOneGeneralInputArgument(L, 8, lua_arg_number, 3, true, true, &errorString);
                            okToGo = (res != -1);
                            if (okToGo)
                            {
                                if (res > 0)
                                {
                                    res =
                                        checkOneGeneralInputArgument(L, 9, lua_arg_number, 3, true, true, &errorString);
                                    okToGo = (res != -1);
                                    if (okToGo)
                                    {
                                        if (res > 0)
                                        {
                                            if (res == 2)
                                            {
                                                getFloatsFromTable(L, 9, 3, specularC);
                                                specular = specularC;
                                            }
                                            res = checkOneGeneralInputArgument(L, 10, lua_arg_number, 3, true, true,
                                                                               &errorString);
                                            okToGo = (res != -1);
                                            if (okToGo)
                                            {
                                                if (res > 0)
                                                {
                                                    if (res == 2)
                                                    {
                                                        getFloatsFromTable(L, 10, 3, emissionC);
                                                        emission = emissionC;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        if (okToGo)
                        {
                            if (!App::currentWorld->simulation->isSimulationRunning())
                                errorString = SIM_ERROR_SIMULATION_NOT_RUNNING;
                            else
                                retVal = App::worldContainer->pluginContainer->dyn_addParticleObject(
                                    objType, size, massOverVolume, params, lifeTime, maxItemCount, ambient, nullptr,
                                    specular, emission);
                        }
                    }
                    delete[]((char*)params);
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simRemoveParticleObject(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.removeParticleObject");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        if (App::worldContainer->pluginContainer->dyn_removeParticleObject(luaToInt(L, 1)) != 0)
            retVal = 1;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAddParticleObjectItem(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.addParticleObjectItem");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int particleObjHandle = luaToInt(L, 1);
        int d = 6 + App::worldContainer->pluginContainer->dyn_getParticleObjectOtherFloatsPerItem(particleObjHandle);
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, d, true, true, &errorString);
        if (res == 2)
        {
            double vertex[20]; // we should have enough here!
            getDoublesFromTable(L, 2, d, vertex);

            if (App::worldContainer->pluginContainer->dyn_addParticleObjectItem(
                    particleObjHandle, vertex, App::currentWorld->simulation->getSimulationTime()) != 0)
                retVal = 1;
            else
                errorString = SIM_ERROR_OBJECT_INEXISTANT;
        }
        else
        {
            if (res >= 0)
            {
                if (App::worldContainer->pluginContainer->dyn_addParticleObjectItem(
                        particleObjHandle, nullptr, App::currentWorld->simulation->getSimulationTime()) != 0)
                    retVal = 1;
                else
                    errorString = SIM_ERROR_OBJECT_INEXISTANT;
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectSizeFactor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectSizeFactor");

    double retVal = -1.0; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simGetObjectSizeFactor, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushnumber(L, retVal);
    LUA_END(1);
}

int _simReadForceSensor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.readForceSensor");

    int retVal = -1;
    double force[3] = {0.0, 0.0, 0.0};
    double torque[3] = {0.0, 0.0, 0.0};
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simReadForceSensor, luaToInt(L, 1), force, torque);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    pushDoubleTableOntoStack(L, 3, force);
    pushDoubleTableOntoStack(L, 3, torque);
    LUA_END(3);
}

int _simGetVelocity(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getVelocity");

    double linVel[3] = {0.0, 0.0, 0.0};
    double angVel[3] = {0.0, 0.0, 0.0};
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        CALL_C_API(simGetVelocity, luaToInt(L, 1), linVel, angVel);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    pushDoubleTableOntoStack(L, 3, linVel);
    pushDoubleTableOntoStack(L, 3, angVel);
    LUA_END(2);
}

int _simGetObjectVelocity(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectVelocity");

    double linVel[3] = {0.0, 0.0, 0.0};
    double angVel[3] = {0.0, 0.0, 0.0};
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        CALL_C_API(simGetObjectVelocity, luaToInt(L, 1), linVel, angVel);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    pushDoubleTableOntoStack(L, 3, linVel);
    pushDoubleTableOntoStack(L, 3, angVel);
    LUA_END(2);
}

int _simGetJointVelocity(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getJointVelocity");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        double vel;
        int retVal = CALL_C_API(simGetJointVelocity, luaToInt(L, 1), &vel);
        if (retVal != -1)
        {
            luaWrap_lua_pushnumber(L, vel);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simAddForceAndTorque(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.addForceAndTorque");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        double f[3] = {0.0, 0.0, 0.0};
        double t[3] = {0.0, 0.0, 0.0};
        bool err = false;
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 3, true, true, &errorString);
        if (res == 2)
            getDoublesFromTable(L, 2, 3, f);
        err = err || (res < 0);
        if (!err)
        {
            int res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 3, true, true, &errorString);
            if (res == 2)
                getDoublesFromTable(L, 3, 3, t);
            err = err || (res < 0);
            if (!err)
                retVal = CALL_C_API(simAddForceAndTorque, luaToInt(L, 1), f, t);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAddForce(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.addForce");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 3, lua_arg_number, 3))
    {
        double r[3];
        double f[3];
        getDoublesFromTable(L, 2, 3, r);
        getDoublesFromTable(L, 3, 3, f);
        retVal = CALL_C_API(simAddForce, luaToInt(L, 1), r, f);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetExplicitHandling(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setExplicitHandling");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simSetExplicitHandling, luaToInt(L, 1), luaToInt(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetExplicitHandling(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getExplicitHandling");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simGetExplicitHandling, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetLinkDummy(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getLinkDummy");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simGetLinkDummy, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetLinkDummy(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setLinkDummy");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simSetLinkDummy, luaToInt(L, 1), luaToInt(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetShapeColor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setShapeColor");

    int retVal = -1; // means error
    int shapeHandle = -1;
    bool ok = false;
    bool correctColors = false;
    if (!checkInputArguments(L, nullptr, lua_arg_number, 0))
    { // this section is to guarantee backward compatibility: color values have changed in the release following 3.1.3.
        // So we need to adjust them
        if (checkInputArguments(L, &errorString, lua_arg_string, 0))
        {
            std::string txt(luaWrap_lua_tostring(L, 1));
            if (txt.compare(0, 20, "@backCompatibility1:") == 0)
            {
                txt.assign(txt.begin() + 20, txt.end());
                if (tt::getValidInt(txt.c_str(), shapeHandle)) // try to extract the original number
                {
                    correctColors = true;
                    ok = true;
                }
                else
                    checkInputArguments(L, &errorString, lua_arg_number, 0); // just generate an error
            }
            else
                checkInputArguments(L, &errorString, lua_arg_number, 0); // just generate an error
        }
    }
    else
    {
        ok = true;
        shapeHandle = luaToInt(L, 1);
    }

    if (ok)
    { // First arg ok
        std::string strTmp;
        char* str = nullptr;
        int colorComponent = 0;
        bool err = false;
        bool transformColor = false;
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_string, 0, false, true, &errorString);
        if (res == 2)
        {
            strTmp = luaWrap_lua_tostring(L, 2);
            str = (char*)strTmp.c_str();
            transformColor = ((strTmp.length() > 1) && (strTmp[0] == '@'));
        }
        err = err || (res < 1);
        if (!err)
        {
            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, false, false, &errorString);
            if (res == 2)
                colorComponent = luaToInt(L, 3);
            err = err || (res < 2);
            if (!err)
            {
                int floatsInTableExpected = 3;
                if (colorComponent == 4)
                    floatsInTableExpected = 1;
                res = checkOneGeneralInputArgument(L, 4, lua_arg_number, floatsInTableExpected, false, false,
                                                   &errorString);
                if (res == 2)
                {
                    if (strTmp.compare("@compound") == 0)
                    {
                        CShape* shape = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
                        if (shape != nullptr)
                            floatsInTableExpected *= shape->getComponentCount();
                    }
                    std::vector<float> rgbData;
                    rgbData.resize(floatsInTableExpected);
                    getFloatsFromTable(L, 4, floatsInTableExpected, &rgbData[0]);
                    if (correctColors && (colorComponent == 0) && (!transformColor))
                    {
                        rgbData[0] = (rgbData[0] + 0.25) / 0.85;
                        rgbData[1] = (rgbData[1] + 0.25) / 0.85;
                        rgbData[2] = (rgbData[2] + 0.25) / 0.85;
                        float mx = std::max<float>(std::max<float>(rgbData[0], rgbData[1]), rgbData[2]);
                        if (mx > 1.0)
                        {
                            rgbData[0] /= mx;
                            rgbData[1] /= mx;
                            rgbData[2] /= mx;
                        }
                    }
                    retVal = CALL_C_API(simSetShapeColor, shapeHandle, str, colorComponent, &rgbData[0]);
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetShapeColor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getShapeColor");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    { // First arg ok
        int shapeHandle = luaToInt(L, 1);
        std::string strTmp;
        char* str = nullptr;
        int colorComponent = 0;
        bool err = false;
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_string, 0, false, true, &errorString);
        if (res == 2)
        {
            strTmp = luaWrap_lua_tostring(L, 2);
            str = (char*)strTmp.c_str();
        }
        err = err || (res < 1);
        if (!err)
        {
            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, false, false, &errorString);
            if (res == 2)
                colorComponent = luaToInt(L, 3);
            err = err || (res < 2);
            if (!err)
            {
                int res = 1;
                if (strTmp.compare("@compound") == 0)
                {
                    CShape* shape = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
                    if (shape != nullptr)
                        res = shape->getComponentCount();
                }
                if (colorComponent != 4)
                    res *= 3;
                std::vector<float> rgbData;
                rgbData.resize(res);
                retVal = CALL_C_API(simGetShapeColor, shapeHandle, str, colorComponent, &rgbData[0]);
                if (retVal > 0)
                {
                    luaWrap_lua_pushinteger(L, retVal);
                    pushFloatTableOntoStack(L, res, &rgbData[0]);
                    LUA_END(2);
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    double dummy[3] = {0.0, 0.0, 0.0};
    pushDoubleTableOntoStack(L, 3, dummy);
    LUA_END(2);
}

int _simSetObjectColor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setObjectColor");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number,
                            3))
    {
        float col[3];
        getFloatsFromTable(L, 4, 3, col);
        int res = CALL_C_API(simSetObjectColor, luaToInt(L, 1), luaToInt(L, 2), luaToInt(L, 3), col);
        if (res >= 0)
        {
            luaWrap_lua_pushboolean(L, res != 0);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetObjectColor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectColor");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        float col[3];
        int res = CALL_C_API(simGetObjectColor, luaToInt(L, 1), luaToInt(L, 2), luaToInt(L, 3), col);
        if (res > 0)
        {
            pushFloatTableOntoStack(L, 3, col);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simResetDynamicObject(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.resetDynamicObject");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simResetDynamicObject, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetJointMode(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setJointMode");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int option_old = 0;
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, true, &errorString);
        if (res == 2)
            option_old = luaToInt(L, 3);
        retVal = CALL_C_API(simSetJointMode, luaToInt(L, 1), luaToInt(L, 2), luaToInt(L, 3));
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetJointMode(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getJointMode");

    int retVal = -1; // means error
    int options = 0;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simGetJointMode, luaToInt(L, 1), &options);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    luaWrap_lua_pushinteger(L, options);
    LUA_END(2);
}

int _simSerialOpen(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim._serialOpen");

    int retVal = -1; // means error
#ifdef SIM_WITH_GUI
    if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 0))
    {
        size_t dataLength;
        const char* portName = luaWrap_lua_tobuffer(L, 1, &dataLength);
        int baudrate = luaToInt(L, 2);
        retVal = App::worldContainer->serialPortContainer->serialPortOpen(true, portName, baudrate);
    }
#endif

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSerialClose(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim._serialClose");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        CALL_C_API(simSerialClose, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSerialSend(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.serialSend");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_string, 0))
    {
        size_t dataLength;
        char* data = (char*)luaWrap_lua_tobuffer(L, 2, &dataLength);
        retVal = CALL_C_API(simSerialSend, luaToInt(L, 1), data, (int)dataLength);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSerialRead(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim._serialRead");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        size_t maxLength = (size_t)luaToInt(L, 2);
        std::string data;
        data.resize(maxLength);
        int nb = CALL_C_API(simSerialRead, luaToInt(L, 1), (char*)data.c_str(), (int)maxLength);
        if (nb > 0)
        {
            luaWrap_lua_pushbuffer(L, data.c_str(), nb);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSerialCheck(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.serialCheck");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simSerialCheck, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetContactInfo(luaWrap_lua_State* L)
{ // problematic for remote API and in general: function returns different return value count
    TRACE_LUA_API;
    LUA_START("sim.getContactInfo");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        int collidingObjects[2];
        double contactInfo[9];
        int index = luaToInt(L, 3);
        if (index < sim_handleflag_extended)
            index += sim_handleflag_extended;
        int res = CALL_C_API(simGetContactInfo, luaToInt(L, 1), luaToInt(L, 2), index, collidingObjects, contactInfo);
        if (res == 1)
        {
            pushIntTableOntoStack(L, 2, collidingObjects);
            pushDoubleTableOntoStack(L, 3, contactInfo);
            pushDoubleTableOntoStack(L, 3, contactInfo + 3);
            pushDoubleTableOntoStack(L, 3, contactInfo + 6);
        }
        else
        {
            pushIntTableOntoStack(L, 0, nullptr);
            pushDoubleTableOntoStack(L, 0, nullptr);
            pushDoubleTableOntoStack(L, 0, nullptr);
            pushDoubleTableOntoStack(L, 0, nullptr);
        }
        LUA_END(4);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simAuxiliaryConsoleOpen(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.auxiliaryConsoleOpen");

    int retVal = -1; // Error
    if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number | lua_arg_optional, 0, lua_arg_number | lua_arg_optional, 0, lua_arg_integer | lua_arg_optional, 2, lua_arg_integer | lua_arg_optional, 2, lua_arg_number | lua_arg_optional, 3, lua_arg_number | lua_arg_optional, 3))
    {
        std::string title = fetchTextArg(L, 1, "");
        int maxLines = fetchIntArg(L, 2, 50);
        int mode = fetchIntArg(L, 3, 0);
        std::vector<int> pos;
        fetchIntArrayArg(L, 4, pos, {100, 100});
        std::vector<int> size;
        fetchIntArrayArg(L, 5, size, {640, 200});
        std::vector<float> textCol;
        fetchFloatArrayArg(L, 6, textCol, {0.0, 0.0, 0.0});
        std::vector<float> bgCol;
        fetchFloatArrayArg(L, 7, bgCol, {1.0, 1.0, 1.0});
        int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        CScriptObject* itScrObj = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
        if ((itScrObj->getScriptType() == sim_scripttype_main) || (itScrObj->getScriptType() == sim_scripttype_simulation))
            mode |= 1; // Add-ons and customization scripts do not have this restriction
        if ((itScrObj->getScriptType() != sim_scripttype_sandbox) && (itScrObj->getScriptType() != sim_scripttype_addon))
        { // Add-ons and sandbox scripts do not have this restriction
            mode |= 16;
            mode -= 16;
        }
        std::string name(luaWrap_lua_tostring(L, 1));
        setCurrentScriptInfo_cSide(CScriptObject::getScriptHandleFromInterpreterState_lua(L), -1); // for transmitting to the master function additional info (e.g.for autom. name adjustment, or for autom. object deletion when script ends)
        retVal = CALL_C_API(simAuxiliaryConsoleOpen, title.c_str(), maxLines, mode, pos.data(), size.data(), textCol.data(), bgCol.data());
        setCurrentScriptInfo_cSide(-1, -1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAuxiliaryConsoleClose(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.auxiliaryConsoleClose");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simAuxiliaryConsoleClose, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAuxiliaryConsoleShow(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.auxiliaryConsoleShow");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_bool, 0))
        retVal = CALL_C_API(simAuxiliaryConsoleShow, luaToInt(L, 1), luaToBool(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAuxiliaryConsolePrint(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.auxiliaryConsolePrint");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string | lua_arg_optional, 0))
    {
        int h = fetchIntArg(L, 1);
        std::string txt = fetchTextArg(L, 2, "");
        retVal = CALL_C_API(simAuxiliaryConsolePrint, h, txt.c_str());
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simImportShape(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.importShape");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_string, 0, lua_arg_number, 0, lua_arg_number, 0,
                            lua_arg_number, 0))
    {
        int fileType = luaToInt(L, 1);
        std::string pathAndFilename(luaWrap_lua_tostring(L, 2));
        int options = luaToInt(L, 3);
        double identicalVerticeTolerance = luaToDouble(L, 4);
        double scalingFactor = luaToDouble(L, 5);
        retVal = CALL_C_API(simImportShape, fileType, pathAndFilename.c_str(), options, identicalVerticeTolerance,
                                         scalingFactor);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simImportMesh(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.importMesh");

    int retValCnt = 1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_string, 0, lua_arg_number, 0, lua_arg_number, 0,
                            lua_arg_number, 0))
    {
        int fileType = luaToInt(L, 1);
        std::string pathAndFilename(luaWrap_lua_tostring(L, 2));
        int options = luaToInt(L, 3);
        double identicalVerticeTolerance = luaToDouble(L, 4);
        double scalingFactor = luaToDouble(L, 5);
        double** vertices;
        int* verticesSizes;
        int** indices;
        int* indicesSizes;
        int elementCount =
            CALL_C_API(simImportMesh, fileType, pathAndFilename.c_str(), options, identicalVerticeTolerance, scalingFactor,
                                   &vertices, &verticesSizes, &indices, &indicesSizes, nullptr, nullptr);
        if (elementCount > 0)
        {
            // Vertices:
            luaWrap_lua_newtable(L);
            int tablePos = luaWrap_lua_gettop(L);
            for (int i = 0; i < elementCount; i++)
            {
                luaWrap_lua_newtable(L);
                int smallTablePos = luaWrap_lua_gettop(L);
                for (int j = 0; j < verticesSizes[i]; j++)
                {
                    luaWrap_lua_pushnumber(L, vertices[i][j]);
                    luaWrap_lua_rawseti(L, smallTablePos, j + 1);
                }
                luaWrap_lua_rawseti(L, tablePos, i + 1);
            }
            // Indices:
            luaWrap_lua_newtable(L);
            tablePos = luaWrap_lua_gettop(L);
            for (int i = 0; i < elementCount; i++)
            {
                luaWrap_lua_newtable(L);
                int smallTablePos = luaWrap_lua_gettop(L);
                for (int j = 0; j < indicesSizes[i]; j++)
                {
                    luaWrap_lua_pushinteger(L, indices[i][j]);
                    luaWrap_lua_rawseti(L, smallTablePos, j + 1);
                }
                luaWrap_lua_rawseti(L, tablePos, i + 1);
            }
            retValCnt = 2;

            for (int i = 0; i < elementCount; i++)
            {
                delete[] indices[i];
                delete[] vertices[i];
            }
            delete[] indicesSizes;
            delete[] indices;
            delete[] verticesSizes;
            delete[] vertices;
            LUA_END(retValCnt);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simExportMesh(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.exportMesh");

    int retVal = -1; // indicates an error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_string, 0, lua_arg_number, 0, lua_arg_number,
                            0))
    {
        int fileType = luaToInt(L, 1);
        std::string pathAndFilename(luaWrap_lua_tostring(L, 2));
        int options = luaToInt(L, 3);
        double scalingFactor = luaToDouble(L, 4);
        int elementCount = 15487;
        if ((luaWrap_lua_gettop(L) >= 6) && luaWrap_lua_isnonbuffertable(L, 5) && luaWrap_lua_isnonbuffertable(L, 6))
        {
            int ve = (int)luaWrap_lua_rawlen(L, 5);
            int ie = (int)luaWrap_lua_rawlen(L, 6);
            elementCount = std::min<int>(ve, ie);
        }
        if ((checkOneGeneralInputArgument(L, 5, lua_arg_table, elementCount, false, false, &errorString) == 2) &&
            (checkOneGeneralInputArgument(L, 6, lua_arg_table, elementCount, false, false, &errorString) == 2))
        {
            double** vertices;
            int* verticesSizes;
            int** indices;
            int* indicesSizes;
            vertices = new double*[elementCount];
            verticesSizes = new int[elementCount];
            indices = new int*[elementCount];
            indicesSizes = new int[elementCount];
            // Following needed if we break before (because one table in a table is not correct (or not a table)):
            for (int i = 0; i < elementCount; i++)
            {
                vertices[i] = nullptr;
                verticesSizes[i] = 0;
                indices[i] = nullptr;
                indicesSizes[i] = 0;
            }

            bool error = false;
            for (int i = 0; i < elementCount; i++)
            {
                if (!error)
                {
                    luaWrap_lua_rawgeti(L, 5, i + 1);
                    if (luaWrap_lua_isnonbuffertable(L, -1))
                    {
                        int vl = (int)luaWrap_lua_rawlen(L, -1);
                        if (checkOneGeneralInputArgument(L, luaWrap_lua_gettop(L), lua_arg_number, vl, false, false,
                                                         &errorString) == 2)
                        {
                            verticesSizes[i] = vl;
                            vertices[i] = new double[vl];
                            getDoublesFromTable(L, luaWrap_lua_gettop(L), vl, vertices[i]);
                        }
                        else
                            error = true;
                    }
                    else
                        error = true;
                    luaWrap_lua_pop(L, 1); // we have to pop the value that was pushed with luaWrap_lua_rawgeti
                }
                if (!error)
                {
                    luaWrap_lua_rawgeti(L, 6, i + 1);
                    if (luaWrap_lua_isnonbuffertable(L, -1))
                    {
                        int vl = (int)luaWrap_lua_rawlen(L, -1);
                        if (checkOneGeneralInputArgument(L, luaWrap_lua_gettop(L), lua_arg_number, vl, false, false,
                                                         &errorString) == 2)
                        {
                            indicesSizes[i] = vl;
                            indices[i] = new int[vl];
                            getIntsFromTable(L, luaWrap_lua_gettop(L), vl, indices[i]);
                        }
                        else
                            error = true;
                    }
                    else
                        error = true;
                    luaWrap_lua_pop(L, 1); // we have to pop the value that was pushed with luaWrap_lua_rawgeti
                }
            }

            if (!error)
                retVal = CALL_C_API(simExportMesh, fileType, pathAndFilename.c_str(), options, scalingFactor, elementCount,
                                                (const double**)vertices, verticesSizes, (const int**)indices,
                                                indicesSizes, nullptr, nullptr);

            for (int i = 0; i < elementCount; i++)
            {
                delete[] vertices[i];
                delete[] indices[i];
            }
            delete[] indicesSizes;
            delete[] indices;
            delete[] verticesSizes;
            delete[] vertices;
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCreateShape(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.createShape");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_number, 0))
    {
        int options = fetchIntArg(L, 1);
        double shadingAngle = fetchDoubleArg(L, 2);

        int vl = 2;
        int il = 2;
        if ((luaWrap_lua_gettop(L) >= 4) && luaWrap_lua_isnonbuffertable(L, 3) && luaWrap_lua_isnonbuffertable(L, 4))
        {
            vl = (int)luaWrap_lua_rawlen(L, 3);
            il = (int)luaWrap_lua_rawlen(L, 4);
        }
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_number, vl, false, false, &errorString);
        if (res == 2)
        {
            res = checkOneGeneralInputArgument(L, 4, lua_arg_number, il, false, false, &errorString);
            if (res == 2)
            {
                std::vector<double> vertices;
                vertices.resize(vl);
                std::vector<int> indices;
                indices.resize(il);
                getDoublesFromTable(L, 3, vl, vertices.data());
                getIntsFromTable(L, 4, il, indices.data());

                std::vector<double> normals;
                normals.resize(il * 3);
                double* _normals = nullptr;
                std::vector<float> texCoords;
                texCoords.resize(il * 2);
                float* _texCoords = nullptr;
                int resol[2];
                unsigned char* img = nullptr;

                res = checkOneGeneralInputArgument(L, 5, lua_arg_number, il * 3, true, true, &errorString);
                if (res == 2)
                {
                    getDoublesFromTable(L, 5, il * 3, normals.data());
                    _normals = normals.data();
                }
                res = checkOneGeneralInputArgument(L, 6, lua_arg_number, il * 2, true, true, &errorString);
                if (res == 2)
                {
                    getFloatsFromTable(L, 6, il * 2, texCoords.data());
                    _texCoords = texCoords.data();
                }
                res = checkOneGeneralInputArgument(L, 7, lua_arg_string, 0, true, true, &errorString);
                size_t l;
                if (res == 2)
                    img = (unsigned char*)luaWrap_lua_tobuffer(L, 7, &l);
                res = checkOneGeneralInputArgument(L, 8, lua_arg_integer, 2, true, true, &errorString);
                if (res == 2)
                    getIntsFromTable(L, 8, 2, resol);
                retVal = CALL_C_API(simCreateShape, options, shadingAngle, vertices.data(), vl, indices.data(), il,
                                                 _normals, _texCoords, img, resol);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetShapeMesh(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getShapeMesh");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        double* vertices;
        int verticesSize;
        int* indices;
        int indicesSize;
        double* normals;
        int result =
            CALL_C_API(simGetShapeMesh, luaToInt(L, 1), &vertices, &verticesSize, &indices, &indicesSize, &normals);
        if (result > 0)
        {
            pushDoubleTableOntoStack(L, verticesSize, vertices);
            pushIntTableOntoStack(L, indicesSize, indices);
            pushDoubleTableOntoStack(L, indicesSize * 3, normals);
            delete[] vertices;
            delete[] indices;
            delete[] normals;
            LUA_END(3);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetShapeBB(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getShapeBB");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0))
    {
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(luaToInt(L, 1));
        if (it != nullptr)
        {
            if (it->getObjectType() == sim_sceneobject_shape)
            {
                CShape* shape = (CShape*)it;
                C3Vector hs;
                C7Vector ltr = shape->getBB(&hs);
                hs *= 2.0;
                double p[7];
                ltr.getData(p, true);
                pushDoubleTableOntoStack(L, 3, hs.data);
                pushDoubleTableOntoStack(L, 7, p);
                LUA_END(2);
            }
            else
                errorString = SIM_ERROR_OBJECT_NOT_SHAPE;
        }
        else
            errorString = SIM_ERROR_OBJECT_INEXISTANT;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simCreatePrimitiveShape(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.createPrimitiveShape");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_number, 3))
    {
        int primitiveType = luaToInt(L, 1);
        double sizes[3];
        getDoublesFromTable(L, 2, 3, sizes);
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_integer, 0, true, false, &errorString);
        if (res >= 0)
        {
            int options = 0;
            if (res == 2)
                options = luaToInt(L, 3);
            retVal = CALL_C_API(simCreatePrimitiveShape, primitiveType, sizes, options);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCreateHeightfieldShape(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.createHeightfieldShape");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0,
                            lua_arg_number, 0))
    {
        int options = luaToInt(L, 1);
        double shadingAngle = luaToDouble(L, 2);
        int xPointCount = luaToInt(L, 3);
        int yPointCount = luaToInt(L, 4);
        double xSize = luaToDouble(L, 5);
        int res =
            checkOneGeneralInputArgument(L, 6, lua_arg_number, xPointCount * yPointCount, false, false, &errorString);
        if (res == 2)
        {
            double* heights = new double[xPointCount * yPointCount];
            getDoublesFromTable(L, 6, xPointCount * yPointCount, heights);
            retVal =
                CALL_C_API(simCreateHeightfieldShape, options, shadingAngle, xPointCount, yPointCount, xSize, heights);
            delete[] heights;
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCreateJoint(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.createJoint");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_integer | lua_arg_optional, 0, lua_arg_integer | lua_arg_optional, 0, lua_arg_number | lua_arg_optional, 2))
    {
        int jointType = fetchIntArg(L, 1);
        int jointMode = fetchIntArg(L, 2, sim_jointmode_dynamic);
        int options = fetchIntArg(L, 3, 0);
        std::vector<double> sizes;
        fetchDoubleArrayArg(L, 4, sizes, {0.15, 0.02});
        retVal = CALL_C_API(simCreateJoint, jointType, jointMode, options, sizes.data(), nullptr, nullptr);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCreateDummy(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.createDummy");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number | lua_arg_optional, 0, lua_arg_number | lua_arg_optional, 3))
    {
        double size = fetchDoubleArg(L, 1, 0.005);
        std::vector<float> col;
        fetchFloatArrayArg(L, 2, col, {1.0f, 0.8f, 0.55f});
        float c[12] = {1.0f, 0.8f, 0.55f, 0.0f, 0.0f, 0.0f, 0.25f, 0.25f, 0.25f, 0.0f, 0.0f, 0.0f};
        c[0] = col[0];
        c[1] = col[1];
        c[2] = col[2];
        retVal = CALL_C_API(simCreateDummy, size, c);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCreateScript(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.createScript");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_string, 0))
    {
        int scriptType = luaToInt(L, 1);
        std::string txt = luaWrap_lua_tostring(L, 2);
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_integer, 0, true, true, &errorString);
        if (res >= 0)
        {
            int options = 0;
            if (res == 2)
                options = luaToInt(L, 3);

            res = checkOneGeneralInputArgument(L, 4, lua_arg_string, 0, true, true, &errorString);
            if (res >= 0)
            {
                std::string lang("lua");
                if (res == 2)
                    lang = luaWrap_lua_tostring(L, 4);
                retVal = CALL_C_API(simCreateScript, scriptType, txt.c_str(), options, lang.c_str());
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCreateProximitySensor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.createProximitySensor");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_integer | lua_arg_optional, 0, lua_arg_integer | lua_arg_optional, 0, lua_arg_integer | lua_arg_optional, 8, lua_arg_number | lua_arg_optional, 15))
    {
        int sensorType = fetchIntArg(L, 1);
        int subType = fetchIntArg(L, 2, 16);
        int options = fetchIntArg(L, 3, 0);
        std::vector<int> intParamsDef = {32, 32, 1, 16, 1, 1, 0, 0};
        std::vector<double> doubleParamsDef = {0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        if (sensorType == sim_proximitysensor_cylinder)
        {
            doubleParamsDef[0] = 0.1;
            doubleParamsDef[1] = 0.2;
            doubleParamsDef[7] = 0.1;
            doubleParamsDef[8] = 0.2;
            intParamsDef[0] = 32;
        }
        if (sensorType == sim_proximitysensor_disc)
        {
            doubleParamsDef[0] = 0.0;
            doubleParamsDef[1] = 0.2;
            doubleParamsDef[7] = 0.1;
            doubleParamsDef[9] = piValD2;
            doubleParamsDef[3] = 0.05;
            doubleParamsDef[5] = 0.1;
            intParamsDef[0] = 16;
            intParamsDef[1] = 32;
        }
        if (sensorType == sim_proximitysensor_pyramid)
        {
            doubleParamsDef[0] = 0.1;
            doubleParamsDef[1] = 0.2;
            doubleParamsDef[2] = 0.2;
            doubleParamsDef[3] = 0.1;
            doubleParamsDef[4] = 0.4;
            doubleParamsDef[5] = 0.2;
        }
        if (sensorType == sim_proximitysensor_ray)
        {
            doubleParamsDef[0] = 0.1;
            doubleParamsDef[1] = 0.4;
        }
        if (sensorType == sim_proximitysensor_cone)
        {
            doubleParamsDef[0] = 0.0;
            doubleParamsDef[7] = 0.1;
            doubleParamsDef[1] = 0.3 -  doubleParamsDef[7];
            doubleParamsDef[9] = piValD2;
            intParamsDef[0] = 32;
            intParamsDef[2] = 1;
            intParamsDef[3] = 16;
        }
        std::vector<int> intParams;
        fetchIntArrayArg(L, 4, intParams, intParamsDef);
        std::vector<double> doubleParams;
        fetchDoubleArrayArg(L, 5, doubleParams, doubleParamsDef);
        retVal = CALL_C_API(simCreateProximitySensor, sensorType, subType, options, intParams.data(), doubleParams.data(), nullptr);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCreateForceSensor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.createForceSensor");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_integer | lua_arg_optional, 0, lua_arg_integer | lua_arg_optional, 5, lua_arg_number | lua_arg_optional, 5))
    {
        int options = fetchIntArg(L, 1, 0);
        std::vector<int> intArgs;
        fetchIntArrayArg(L, 2, intArgs, {0, 1, 10, 0, 0});
        std::vector<double> doubleArgs;
        fetchDoubleArrayArg(L, 3, doubleArgs, {0.05, 100.0, 10.0, 0.0, 0.0});
        retVal = CALL_C_API(simCreateForceSensor, options, intArgs.data(), doubleArgs.data(), nullptr);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCreateVisionSensor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.createVisionSensor");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_integer | lua_arg_optional, 0, lua_arg_integer | lua_arg_optional, 4, lua_arg_number | lua_arg_optional, 11))
    {
        int options = fetchIntArg(L, 1, 0);
        std::vector<int> intParams;
        fetchIntArrayArg(L, 2, intParams, {256, 256, 0, 0});
        std::vector<double> doubleParams;
        std::vector<double> doubleParamsDef = {0.01, 10.0, 0.1, 0.01, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        if (options & 2)
            doubleParamsDef[2] = 60.0 * degToRad; // perspective. View angle
        fetchDoubleArrayArg(L, 2, doubleParams, doubleParamsDef);
        retVal = CALL_C_API(simCreateVisionSensor, options, intParams.data(), doubleParams.data(), nullptr);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simFloatingViewAdd(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.floatingViewAdd");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0,
                            lua_arg_number, 0))
        retVal = CALL_C_API(simFloatingViewAdd, luaToDouble(L, 1), luaToDouble(L, 2), luaToDouble(L, 3), luaToDouble(L, 4),
                                             luaToInt(L, 5));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simFloatingViewRemove(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.floatingViewRemove");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        retVal = CALL_C_API(simFloatingViewRemove, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAdjustView(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.adjustView");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        int res = checkOneGeneralInputArgument(L, 4, lua_arg_string, 0, true, true, &errorString);
        if (res >= 0)
        {
            char* txt = nullptr;
            if (res == 2)
                txt = (char*)luaWrap_lua_tostring(L, 4);
            if ((txt != nullptr) && (strlen(txt) > 0))
                retVal = CALL_C_API(simAdjustView, luaToInt(L, 1), luaToInt(L, 2), luaToInt(L, 3), txt);
            else
                retVal = CALL_C_API(simAdjustView, luaToInt(L, 1), luaToInt(L, 2), luaToInt(L, 3), nullptr);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCameraFitToView(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.cameraFitToView");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_integer | lua_arg_optional, -1, lua_arg_integer | lua_arg_optional, 0, lua_arg_number | lua_arg_optional, 0))
    {
        int viewIndex = fetchIntArg(L, 1);
        std::vector<int> handles;
        fetchIntArrayArg(L, 2, handles);
        int options = fetchIntArg(L, 3, 0);
        double scaling = fetchIntArg(L, 4, 1.0);
        retVal = CALL_C_API(simCameraFitToView, viewIndex, handles.size(), handles.data(), options, scaling);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAnnounceSceneContentChange(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.announceSceneContentChange");

    int retVal = CALL_C_API(simAnnounceSceneContentChange, );

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetJointForce(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getJointForce");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        double jointF;
        if (CALL_C_API(simGetJointForce, luaToInt(L, 1), &jointF) <= 0)
            jointF = 0.0;
        luaWrap_lua_pushnumber(L, jointF);
        LUA_END(1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simIsHandle(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.isHandle");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int objType = -1;
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString);
        if ((res == 0) || (res == 2))
        {
            if (res == 2)
                objType = luaToInt(L, 2);
            int res = CALL_C_API(simIsHandle, luaToInt(L, 1), objType);
            if (res >= 0)
            {
                luaWrap_lua_pushboolean(L, res != 0);
                LUA_END(1);
            }
        }
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simRuckigPos(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.ruckigPos");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        int dofs = luaToInt(L, 1);
        double timeStep = luaToDouble(L, 2);
        int flags = luaToInt(L, 3);
        int maxVelAccelJerkCnt = dofs * 3;
        if ((flags >= 0) && ((flags & sim_ruckig_minvel) != 0))
            maxVelAccelJerkCnt += dofs;
        if ((flags >= 0) && ((flags & sim_ruckig_minaccel) != 0))
            maxVelAccelJerkCnt += dofs;
        if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0,
                                lua_arg_number, dofs * 3, lua_arg_number, maxVelAccelJerkCnt, lua_arg_number, dofs,
                                lua_arg_number, dofs * 2))
        {
            std::vector<double> currentPosVelAccel;
            currentPosVelAccel.resize(dofs * 3);
            getDoublesFromTable(L, 4, dofs * 3, &currentPosVelAccel[0]);

            std::vector<double> maxVelAccelJerk;
            maxVelAccelJerk.resize(maxVelAccelJerkCnt);
            getDoublesFromTable(L, 5, maxVelAccelJerkCnt, &maxVelAccelJerk[0]);

            std::vector<double> minMaxVel;
            std::vector<double> minMaxAccel;
            std::vector<double> maxJerk;
            for (size_t i = 0; i < dofs; i++)
            {
                minMaxVel.push_back(maxVelAccelJerk[i]);
                minMaxAccel.push_back(maxVelAccelJerk[dofs + i]);
                maxJerk.push_back(maxVelAccelJerk[2 * dofs + i]);
            }
            size_t off = 3 * dofs;
            if ((flags >= 0) && ((flags & sim_ruckig_minvel) != 0))
            {
                for (size_t i = 0; i < dofs; i++)
                    minMaxVel.push_back(maxVelAccelJerk[off + i]);
                off += dofs;
            }
            if ((flags >= 0) && ((flags & sim_ruckig_minaccel) != 0))
            {
                for (size_t i = 0; i < dofs; i++)
                    minMaxAccel.push_back(maxVelAccelJerk[off + i]);
                off += dofs;
            }

            std::vector<char> selection;
            selection.resize(dofs);
            getCharBoolsFromTable(L, 6, dofs, &selection[0]);

            std::vector<double> targetPosVel;
            targetPosVel.resize(dofs * 2);
            getDoublesFromTable(L, 7, dofs * 2, &targetPosVel[0]);

            setCurrentScriptInfo_cSide(
                CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                    L)); // for transmitting to the master function additional info (e.g.for autom. name adjustment, or
                         // for autom. object deletion when script ends)
            int retVal = CALL_C_API(simRuckigPos, dofs, timeStep, flags, &currentPosVelAccel[0], &currentPosVelAccel[dofs],
                                               &currentPosVelAccel[dofs * 2], &minMaxVel[0], &minMaxAccel[0],
                                               &maxJerk[0], (bool*)selection.data(), &targetPosVel[0],
                                               &targetPosVel[dofs], nullptr, nullptr);
            setCurrentScriptInfo_cSide(-1, -1);

            if (retVal >= 0)
            {
                luaWrap_lua_pushinteger(L, retVal);
                LUA_END(1);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simRuckigVel(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.ruckigVel");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        int dofs = luaToInt(L, 1);
        double timeStep = luaToDouble(L, 2);
        int flags = luaToInt(L, 3);
        int maxAccelJerkCnt = dofs * 2;
        if ((flags >= 0) && ((flags & sim_ruckig_minaccel) != 0))
            maxAccelJerkCnt += dofs;
        if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0,
                                lua_arg_number, dofs * 3, lua_arg_number, maxAccelJerkCnt, lua_arg_number, dofs,
                                lua_arg_number, dofs))
        {
            std::vector<double> currentPosVelAccel;
            currentPosVelAccel.resize(dofs * 3);
            getDoublesFromTable(L, 4, dofs * 3, &currentPosVelAccel[0]);

            std::vector<double> maxAccelJerk;
            maxAccelJerk.resize(maxAccelJerkCnt);
            getDoublesFromTable(L, 5, maxAccelJerkCnt, &maxAccelJerk[0]);

            std::vector<double> minMaxAccel;
            std::vector<double> maxJerk;
            for (size_t i = 0; i < dofs; i++)
            {
                minMaxAccel.push_back(maxAccelJerk[i]);
                maxJerk.push_back(maxAccelJerk[dofs + i]);
            }
            size_t off = 2 * dofs;
            if ((flags >= 0) && ((flags & sim_ruckig_minaccel) != 0))
            {
                for (size_t i = 0; i < dofs; i++)
                    minMaxAccel.push_back(maxAccelJerk[off + i]);
                off += dofs;
            }

            std::vector<char> selection;
            selection.resize(dofs);
            getCharBoolsFromTable(L, 6, dofs, &selection[0]);

            std::vector<double> targetVel;
            targetVel.resize(dofs);
            getDoublesFromTable(L, 7, dofs, &targetVel[0]);

            setCurrentScriptInfo_cSide(
                CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                    L)); // for transmitting to the master function additional info (e.g.for autom. name adjustment, or
                         // for autom. object deletion when script ends)
            int retVal = CALL_C_API(simRuckigVel, dofs, timeStep, flags, &currentPosVelAccel[0], &currentPosVelAccel[dofs],
                                               &currentPosVelAccel[dofs * 2], &minMaxAccel[0], &maxJerk[0],
                                               (bool*)selection.data(), &targetVel[0], nullptr, nullptr);
            setCurrentScriptInfo_cSide(-1, -1);

            if (retVal >= 0)
            {
                luaWrap_lua_pushinteger(L, retVal);
                LUA_END(1);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simRuckigStep(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.ruckigStep");
    int retVal = -1;
    std::vector<double> newPosVelAccel;
    double syncTime = 0.0;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        double timeStep = luaToDouble(L, 2);
        int dofs = App::worldContainer->pluginContainer->ruckigPlugin_dofs(handle);
        if (dofs < 0)
            dofs = 1; // will be caught later down
        newPosVelAccel.resize(dofs * 3);
        retVal = CALL_C_API(simRuckigStep, handle, timeStep, &newPosVelAccel[0], &newPosVelAccel[dofs],
                                        &newPosVelAccel[dofs * 2], &syncTime, nullptr, nullptr);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    if (newPosVelAccel.size() > 0)
        pushDoubleTableOntoStack(L, newPosVelAccel.size(), &newPosVelAccel[0]);
    else
        pushIntTableOntoStack(L, 0, nullptr); // empty table
    luaWrap_lua_pushnumber(L, syncTime);
    LUA_END(3);
}

int _simRuckigRemove(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.ruckigRemove");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        int retVal = CALL_C_API(simRuckigRemove, handle);
        if (retVal >= 0)
        {
            luaWrap_lua_pushinteger(L, retVal);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetObjectQuaternion(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectQuaternion");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_integer | lua_arg_optional, 0))
    {
        double coord[4];
        int rel = sim_handle_world;
        if (luaWrap_lua_isinteger(L, 2))
            rel = luaToInt(L, 2);
        if (CALL_C_API(simGetObjectQuaternion, luaToInt(L, 1), rel, coord) == 1)
        {
            pushDoubleTableOntoStack(L, 4, coord);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetObjectQuaternion(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setObjectQuaternion");

    if (luaWrap_lua_isnonbuffertable(L, 2))
    {
        if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_number, 4,
                                lua_arg_integer | lua_arg_optional, 0))
        {
            double coord[4];
            getDoublesFromTable(L, 2, 4, coord);
            int rel = sim_handle_world;
            if (luaWrap_lua_isinteger(L, 3))
                rel = luaToInt(L, 3);
            CALL_C_API(simSetObjectQuaternion, luaToInt(L, 1), rel, coord);
        }
    }
    else
    { // old
        if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 4))
        {
            double coord[4];
            getDoublesFromTable(L, 3, 4, coord);
            CALL_C_API(simSetObjectQuaternion, luaToInt(L, 1), luaToInt(L, 2), coord);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetQuaternionFromMatrix(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getQuaternionFromMatrix");

    if (checkInputArguments(L, &errorString, lua_arg_number, 12))
    {
        double arr[12];
        double quaternion[4];
        getDoublesFromTable(L, 1, 12, arr);
        if (CALL_C_API(simGetQuaternionFromMatrix, arr, quaternion) == 1)
        {
            pushDoubleTableOntoStack(L, 4, quaternion);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simCallScriptFunction(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.callScriptFunction");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_string, 0))
    {
        CScriptObject* script = nullptr;
        std::string funcName;
        int scriptHandleOrType = luaToInt(L, 1);
        if (scriptHandleOrType == sim_handle_self)
            scriptHandleOrType = CScriptObject::getScriptHandleFromInterpreterState_lua(L);

        funcName = luaWrap_lua_tostring(L, 2);
        script = App::worldContainer->getScriptObjectFromHandle(scriptHandleOrType);

        if (script != nullptr)
        {
            if (script->hasInterpreterState())
            {
                CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
                CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 0); // skip the two first args

                if (VThread::isSimThread())
                { // For now we don't allow non-main threads to call non-threaded scripts!
                    int rr = script->callCustomScriptFunction(funcName.c_str(), stack);
                    if (rr == 1)
                    {
                        CScriptObject::buildOntoInterpreterStack_lua(L, stack, false);
                        int ss = stack->getStackSize();
                        App::worldContainer->interfaceStackContainer->destroyStack(stack);
                        LUA_END(ss);
                    }
                    else
                    {
                        if (rr == -1)
                            errorString = SIM_ERROR_ERROR_IN_SCRIPT_FUNCTION;
                        else // rr==0
                            errorString = SIM_ERROR_FAILED_CALLING_SCRIPT_FUNCTION;
                    }
                }
                else
                    errorString = SIM_ERROR_FAILED_CALLING_SCRIPT_FUNCTION;
                App::worldContainer->interfaceStackContainer->destroyStack(stack);
            }
            else
                errorString = SIM_ERROR_SCRIPT_NOT_INITIALIZED;
        }
        else
            errorString = SIM_ERROR_SCRIPT_INEXISTANT;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetShapeMass(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getShapeMass");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        double mass;
        int result = CALL_C_API(simGetShapeMass, handle, &mass);
        if (result != -1)
        {
            luaWrap_lua_pushnumber(L, mass);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetShapeMass(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setShapeMass");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        double mass = luaToDouble(L, 2);
        CALL_C_API(simSetShapeMass, handle, mass);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetShapeInertia(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getShapeInertia");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        double inertiaMatrix[9];
        double transformation[12];
        int result = CALL_C_API(simGetShapeInertia, handle, inertiaMatrix, transformation);
        if (result != -1)
        {
            pushDoubleTableOntoStack(L, 9, inertiaMatrix);
            pushDoubleTableOntoStack(L, 12, transformation);
            LUA_END(2);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetShapeInertia(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setShapeInertia");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 9, lua_arg_number, 12))
    {
        int handle = luaToInt(L, 1);
        double inertiaMatrix[9];
        getDoublesFromTable(L, 2, 9, inertiaMatrix);
        double transformation[12];
        getDoublesFromTable(L, 3, 12, transformation);
        CALL_C_API(simSetShapeInertia, handle, inertiaMatrix, transformation);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simIsDynamicallyEnabled(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.isDynamicallyEnabled");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        int res = CALL_C_API(simIsDynamicallyEnabled, handle);
        if (res >= 0)
        {
            luaWrap_lua_pushboolean(L, res != 0);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGenerateShapeFromPath(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.generateShapeFromPath");

    if (checkInputArguments(L, &errorString, lua_arg_number, 14, lua_arg_number, 4))
    {
        std::vector<double> ppath;
        std::vector<double> section;
        ppath.resize(luaWrap_lua_rawlen(L, 1));
        section.resize(luaWrap_lua_rawlen(L, 2));
        getDoublesFromTable(L, 1, luaWrap_lua_rawlen(L, 1), &ppath[0]);
        getDoublesFromTable(L, 2, luaWrap_lua_rawlen(L, 2), &section[0]);
        int options = 0;
        double* zvect = nullptr;
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_integer, 0, true, true, &errorString);
        if (res >= 0)
        {
            if (res == 2)
                options = luaToInt(L, 3);
            res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 3, true, true, &errorString);
            if (res >= 0)
            {
                double tmp[3];
                if (res == 2)
                {
                    getDoublesFromTable(L, 4, 3, tmp);
                    zvect = tmp;
                }
                int h = CALL_C_API(simGenerateShapeFromPath, &ppath[0], int(ppath.size()), &section[0],
                                                          int(section.size()), options, zvect, 0.0);
                if (h >= 0)
                {
                    luaWrap_lua_pushinteger(L, h);
                    LUA_END(1);
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetClosestPosOnPath(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getClosestPosOnPath");

    if (checkInputArguments(L, &errorString, lua_arg_number, 3, lua_arg_number, 1, lua_arg_number, 3))
    {
        size_t pathS = luaWrap_lua_rawlen(L, 1);
        size_t pathLS = luaWrap_lua_rawlen(L, 2);
        if (pathLS <= pathS / 3)
        {
            std::vector<double> ppath;
            std::vector<double> pathLengths;
            double absPt[3];
            ppath.resize(pathS);
            pathLengths.resize(pathLS);
            getDoublesFromTable(L, 1, pathS, &ppath[0]);
            getDoublesFromTable(L, 2, pathLS, &pathLengths[0]);
            getDoublesFromTable(L, 3, 3, absPt);
            double p = CALL_C_API(simGetClosestPosOnPath, &ppath[0], int(pathLS * 3), &pathLengths[0], absPt);
            luaWrap_lua_pushnumber(L, p);
            LUA_END(1);
        }
        else
            errorString = SIM_ERROR_ONE_TABLE_SIZE_IS_WRONG;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simInitScript(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.initScript");

    int scriptHandle = sim_handle_self;
    int res = checkOneGeneralInputArgument(L, 1, lua_arg_number, 0, true, false, &errorString);
    if (res == 2)
        scriptHandle = luaToInt(L, 1);
    if (scriptHandle == sim_handle_self)
        scriptHandle = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    if (scriptHandle == CScriptObject::getScriptHandleFromInterpreterState_lua(L))
        scriptHandle = -scriptHandle - 1;
    if ((res == 0) || (res == 2))
        CALL_C_API(simInitScript, scriptHandle); // executes asynchronously if script handle is negative

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simModuleEntry(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.moduleEntry");

    if (checkInputArguments(L, &errorString, lua_arg_integer, 0))
    {
        int itemHandle = luaToInt(L, 1);
        const char* label = nullptr;
        std::string _label;
        int state = -1;
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_string, 0, false, true, &errorString);
        if (res >= 1)
        {
            if (res == 2)
            {
                _label = luaWrap_lua_tostring(L, 2);
                label = _label.c_str();
            }
            res = checkOneGeneralInputArgument(L, 3, lua_arg_integer, 0, true, true, &errorString);
            if (res >= 0)
            {
                if (res == 2)
                    state = luaToInt(L, 3);
                setCurrentScriptInfo_cSide(
                    CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                    CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                        L)); // for transmitting to the master function additional info (e.g.for autom. name adjustment,
                             // or for autom. object deletion when script ends)
                int h = CALL_C_API(simModuleEntry, itemHandle, label, state);
                setCurrentScriptInfo_cSide(-1, -1);
                if (h >= 0)
                {
                    luaWrap_lua_pushinteger(L, h);
                    LUA_END(1);
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simPushUserEvent(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.pushUserEvent");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_integer, 0, lua_arg_integer, 0))
    {
        std::string eventStr(luaWrap_lua_tostring(L, 1));
        int handle = luaToInt(L, 2);
        long long int uid = luaWrap_lua_tointeger(L, 3);
        if (luaWrap_lua_isnonbuffertable(L, 4))
        {
            int options = 0; // bit0: mergeable
            int res = checkOneGeneralInputArgument(L, 5, lua_arg_integer, 0, true, true, &errorString);
            if (res >= 0)
            {
                if (res == 2)
                    options = luaToInt(L, 5);
                if (App::worldContainer->getEventsEnabled())
                {
                    CCbor* ev = App::worldContainer->createNakedEvent(eventStr.c_str(), handle, uid, (options & 1) != 0);
                    ev->appendText("data");
                    CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
                    CScriptObject::buildFromInterpreterStack_lua(L, stack, 4, 1); // skip the 3 first args
                    std::string buff = stack->getCborEncodedBuffer(0);
                    App::worldContainer->interfaceStackContainer->destroyStack(stack);
                    ev->appendRaw((unsigned char*)buff.data(), buff.size());
                    App::worldContainer->pushEvent();
                }
            }
        }
        else
            errorString.assign(SIM_ERROR_ONE_ARGUMENT_TYPE_IS_WRONG);
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simBroadcastMsg(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.broadcastMsg");

    if (luaWrap_lua_isnonbuffertable(L, 1))
    {
        int options = 0;
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_integer, 0, true, true, &errorString);
        if (res >= 0)
        {
            if (res == 2)
                options = luaToInt(L, 2);
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 1, 0);
            int scriptHandle = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
            stack->pushInt32OntoStack(scriptHandle, false);
            App::worldContainer->broadcastMsg(stack, scriptHandle, options);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
    }
    else
        errorString.assign(SIM_ERROR_ONE_ARGUMENT_TYPE_IS_WRONG);
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetVisionSensorRes(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getVisionSensorRes");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int resolution[2];
        if (CALL_C_API(simGetVisionSensorRes, luaToInt(L, 1), resolution) == 1)
        {
            pushIntTableOntoStack(L, 2, resolution);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simHandleJointMotion(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.handleJointMotion");

    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
    if (it != nullptr)
    {
        if (it->getScriptType() == sim_scripttype_main)
        {
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_joint); i++)
            {
                CJoint* it = App::currentWorld->sceneObjects->getJointFromIndex(i);
                if (it->getJointMode() == sim_jointmode_kinematic)
                    it->handleMotion();
            }
        }
        else
            errorString = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetGenesisEvents(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getGenesisEvents");

    // no args for now

    std::vector<unsigned char> genesisEvents;
    App::worldContainer->getGenesisEvents(&genesisEvents, nullptr);
    luaWrap_lua_pushbuffer(L, (char*)genesisEvents.data(), genesisEvents.size());
    LUA_END(1);

    //    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    //    LUA_END(0);
}

int _simGroupShapes(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.groupShapes");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, lua_arg_number, 1))
    {
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_bool, 0, true, true, &errorString);
        if (res >= 0)
        {
            bool mergeShapes = false;
            if (res == 2)
                mergeShapes = luaToBool(L, 2);
            int tableSize = int(luaWrap_lua_rawlen(L, 1));
            int* theTable = new int[tableSize];
            getIntsFromTable(L, 1, tableSize, theTable);
            if (mergeShapes)
                tableSize = -tableSize;
            retVal = CALL_C_API(simGroupShapes, theTable, tableSize);
            delete[] theTable;
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simUngroupShape(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.ungroupShape");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int shapeHandle = luaToInt(L, 1);
        int count;
        int* handles = CALL_C_API(simUngroupShape, shapeHandle, &count);
        if (handles != nullptr)
        {
            pushIntTableOntoStack(L, count, handles);
            delete[] handles;
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetShapeMaterial(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setShapeMaterial");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int shapeHandle = luaToInt(L, 1);
        int materialId = luaToInt(L, 2);
        retVal = CALL_C_API(simSetShapeMaterial, shapeHandle, materialId);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetTextureId(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getTextureId");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0))
    {
        std::string matName(luaWrap_lua_tostring(L, 1));
        int resolution[2];
        int retVal = CALL_C_API(simGetTextureId, matName.c_str(), resolution);
        if (retVal >= 0)
        {
            luaWrap_lua_pushinteger(L, retVal);
            pushIntTableOntoStack(L, 2, resolution);
            LUA_END(2);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simReadTexture(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.readTexture");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int textureId = luaToInt(L, 1);
        int options = luaToInt(L, 2);
        int posX = 0;
        int posY = 0;
        int sizeX = 0;
        int sizeY = 0;
        // Now check the optional arguments:
        int res;
        res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString);
        if ((res == 0) || (res == 2))
        {
            if (res == 2)
                posX = luaToInt(L, 3);
            res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, true, false, &errorString);
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    posY = luaToInt(L, 4);
                res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, true, false, &errorString);
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        sizeX = luaToInt(L, 5);
                    res = checkOneGeneralInputArgument(L, 6, lua_arg_number, 0, true, false, &errorString);
                    if ((res == 0) || (res == 2))
                    {
                        if (res == 2)
                            sizeY = luaToInt(L, 6);

                        CTextureObject* to = App::currentWorld->textureContainer->getObject(textureId);
                        if (to != nullptr)
                        {
                            int tSizeX, tSizeY;
                            to->getTextureSize(tSizeX, tSizeY);
                            if ((posX >= 0) && (posY >= 0) && (sizeX >= 0) && (sizeY >= 0) &&
                                (posX + sizeX <= tSizeX) && (posY + sizeY <= tSizeY))
                            {
                                if (sizeX == 0)
                                {
                                    posX = 0;
                                    sizeX = tSizeX;
                                }
                                if (sizeY == 0)
                                {
                                    posY = 0;
                                    sizeY = tSizeY;
                                }
                                unsigned char* textureData =
                                    CALL_C_API(simReadTexture, textureId, options, posX, posY, sizeX, sizeY);
                                if (textureData != nullptr)
                                { // here we return RGB data in a string
                                    luaWrap_lua_pushbuffer(L, (char*)textureData, sizeX * sizeY * 3);
                                    CALL_C_API(simReleaseBuffer, textureData);
                                    LUA_END(1);
                                }
                            }
                            else
                                errorString = "bad argument combination (arguments #3, #4, #5 and #6)";
                        }
                        else
                            errorString = SIM_ERROR_TEXTURE_INEXISTANT;
                    }
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simWriteTexture(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.writeTexture");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_integer, 0, lua_arg_string, 0))
    {
        int textureId = luaToInt(L, 1);
        int options = luaToInt(L, 2);
        size_t dataLength;
        char* data = (char*)luaWrap_lua_tobuffer(L, 3, &dataLength);
        if (dataLength >= 3)
        {
            int posX = 0;
            int posY = 0;
            int sizeX = 0;
            int sizeY = 0;
            double interpol = 0.0;
            // Now check the optional arguments:
            int res;
            res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, true, false, &errorString);
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    posX = luaToInt(L, 4);
                res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, true, false, &errorString);
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        posY = luaToInt(L, 5);
                    res = checkOneGeneralInputArgument(L, 6, lua_arg_number, 0, true, false, &errorString);
                    if ((res == 0) || (res == 2))
                    {
                        if (res == 2)
                            sizeX = luaToInt(L, 6);
                        res = checkOneGeneralInputArgument(L, 7, lua_arg_number, 0, true, false, &errorString);
                        if ((res == 0) || (res == 2))
                        {
                            if (res == 2)
                                sizeY = luaToInt(L, 7);
                            res = checkOneGeneralInputArgument(L, 8, lua_arg_number, 0, true, false, &errorString);
                            if ((res == 0) || (res == 2))
                            {
                                if (res == 2)
                                    interpol = luaToDouble(L, 8);
                                CTextureObject* to = App::currentWorld->textureContainer->getObject(textureId);
                                if (to != nullptr)
                                {
                                    int tSizeX, tSizeY;
                                    to->getTextureSize(tSizeX, tSizeY);
                                    if ((sizeX >= 0) && (sizeY >= 0))
                                    {
                                        if (sizeX == 0)
                                        {
                                            posX = 0;
                                            sizeX = tSizeX;
                                        }
                                        if (sizeY == 0)
                                        {
                                            posY = 0;
                                            sizeY = tSizeY;
                                        }
                                        if (int(dataLength) < sizeX * sizeY * 3)
                                        {
                                            std::vector<char> dat;
                                            dat.resize(sizeX * sizeY * 3);
                                            for (size_t i = 0; i < sizeX * sizeY; i++)
                                            {
                                                dat[3 * i + 0] = data[0];
                                                dat[3 * i + 1] = data[1];
                                                dat[3 * i + 2] = data[2];
                                            }
                                            retVal = CALL_C_API(simWriteTexture, textureId, options, dat.data(), posX,
                                                                              posY, sizeX, sizeY, interpol);
                                        }
                                        else
                                            retVal = CALL_C_API(simWriteTexture, textureId, options, data, posX, posY,
                                                                              sizeX, sizeY, interpol);
                                    }
                                    else
                                        errorString = "bad argument #6 and/or #7";
                                }
                                else
                                    errorString = SIM_ERROR_TEXTURE_INEXISTANT;
                            }
                        }
                    }
                }
            }
        }
        else
            errorString = SIM_ERROR_INVALID_BUFFER_SIZE;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCreateTexture(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.createTexture");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_integer | lua_arg_optional, 0, lua_arg_number | lua_arg_optional, 2, lua_arg_number | lua_arg_optional, 2, lua_arg_number | lua_arg_optional, 3, lua_arg_integer | lua_arg_optional, 0, lua_arg_integer | lua_arg_optional, 2))
    {
        std::string fileName(fetchTextArg(L, 1));
        int options = fetchIntArg(L, 2, 0);
        std::vector<double> planeSizes;
        fetchDoubleArrayArg(L, 3, planeSizes, {0.1, 0.1});
        std::vector<double> scalingUV;
        fetchDoubleArrayArg(L, 4, scalingUV, planeSizes);
        std::vector<double> xy_g;
        fetchDoubleArrayArg(L, 5, xy_g, {0.0, 0.0, 0.0});
        int fixedResolution = fetchIntArg(L, 6, 0);
        std::vector<int> resolution;
        fetchIntArrayArg(L, 7, resolution, {512, 512});
        int textureId;
        int shapeHandle = CALL_C_API(simCreateTexture, fileName.c_str(), options, planeSizes.data(), scalingUV.data(), xy_g.data(), fixedResolution, &textureId, resolution.data(), nullptr);
        if (shapeHandle >= 0)
        {
            luaWrap_lua_pushinteger(L, shapeHandle);
            luaWrap_lua_pushinteger(L, textureId);
            pushIntTableOntoStack(L, 2, resolution.data());
            LUA_END(3);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetShapeGeomInfo(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getShapeGeomInfo");

    int retVal = -1; // means error
    int intData[5] = {0, 0, 0, 0, 0};
    double floatData[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        retVal = CALL_C_API(simGetShapeGeomInfo, handle, intData, floatData, nullptr);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    luaWrap_lua_pushinteger(L, intData[0]);
    pushDoubleTableOntoStack(L, 4, floatData);
    LUA_END(3);
}

int _simGetObjects(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjects");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simGetObjects, luaToInt(L, 1), luaToInt(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectsInTree(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectsInTree");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        int objType = sim_handle_all;
        int options = 0;
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, true, &errorString);
        if (res >= 0)
        {
            if (res == 2)
                objType = luaToInt(L, 2);
            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, true, &errorString);
            if (res >= 0)
            {
                if (res == 2)
                    options = luaToInt(L, 3);
                int objCnt = 0;
                int* objHandles = CALL_C_API(simGetObjectsInTree, handle, objType, options, &objCnt);
                if (objHandles != nullptr)
                {
                    pushIntTableOntoStack(L, objCnt, objHandles);
                    CALL_C_API(simReleaseBuffer, (char*)objHandles);
                    LUA_END(1);
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simScaleObject(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.scaleObject");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number,
                            0))
    {
        int handle = luaToInt(L, 1);
        double x = luaToDouble(L, 2);
        double y = luaToDouble(L, 3);
        double z = luaToDouble(L, 4);
        int options = 0;
        int res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, true, true, &errorString);
        if (res >= 0)
        {
            if (res == 2)
                options = luaToInt(L, 5);
            retVal = CALL_C_API(simScaleObject, handle, x, y, z, options);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetShapeTexture(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setShapeTexture");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0,
                            lua_arg_number, 2))
    {
        int handle = luaToInt(L, 1);
        int textureId = luaToInt(L, 2);
        int mapMode = luaToInt(L, 3);
        int options = luaToInt(L, 4);
        double uvScaling[2];
        getDoublesFromTable(L, 5, 2, uvScaling);
        double* posP = nullptr;
        double* orP = nullptr;
        double _pos[3];
        double _or[3];
        int res = checkOneGeneralInputArgument(L, 6, lua_arg_number, 3, true, true, &errorString);
        if (res >= 0)
        {
            if (res == 2)
            {
                getDoublesFromTable(L, 6, 3, _pos);
                posP = _pos;
            }
            res = checkOneGeneralInputArgument(L, 7, lua_arg_number, 3, true, true, &errorString);
            if (res >= 0)
            {
                if (res == 2)
                {
                    getDoublesFromTable(L, 7, 3, _or);
                    orP = _or;
                }
                retVal = CALL_C_API(simSetShapeTexture, handle, textureId, mapMode, options, uvScaling, posP, orP);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetShapeTextureId(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getShapeTextureId");
    int retVal = -1;

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        retVal = CALL_C_API(simGetShapeTextureId, handle);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCreateCollectionEx(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.createCollectionEx");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int options = luaToInt(L, 1);
        setCurrentScriptInfo_cSide(CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                                   CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                                       L)); // for transmitting to the master function additional info (e.g.for autom.
                                            // name adjustment, or for autom. object deletion when script ends)
        int handle = CALL_C_API(simCreateCollectionEx, options);
        setCurrentScriptInfo_cSide(-1, -1);
        luaWrap_lua_pushinteger(L, handle);
        LUA_END(1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simAddItemToCollection(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.addItemToCollection");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number | lua_arg_optional, 0))
    {
        int collHandle = fetchIntArg(L, 1);
        int what = fetchIntArg(L, 2);
        int objHandle = fetchIntArg(L, 3);
        int options = fetchIntArg(L, 4, 0);
        CALL_C_API(simAddItemToCollection, collHandle, what, objHandle, options);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simDestroyCollection(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.destroyCollection");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        CALL_C_API(simDestroyCollection, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetCollectionObjects(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getCollectionObjects");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        int cnt;
        int* objHandles = CALL_C_API(simGetCollectionObjects, handle, &cnt);
        pushIntTableOntoStack(L, cnt, objHandles);
        delete[] objHandles;
        LUA_END(1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simHandleAddOnScripts(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.handleAddOnScripts");

    int retVal = -1;
    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* itScrObj = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
    if (itScrObj->getScriptType() == sim_scripttype_main)
    {
        if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        {
            int callType = luaToInt(L, 1);
            retVal = 0;
            int editMode = NO_EDIT_MODE;
#ifdef SIM_WITH_GUI
            editMode = GuiApp::getEditModeType();
#endif
            if (editMode == NO_EDIT_MODE)
                retVal = App::worldContainer->addOnScriptContainer->callScripts(callType, nullptr, nullptr);
        }
    }
    else
        errorString = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simHandleSandboxScript(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.handleSandboxScript");

    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* itScrObj = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
    if (itScrObj->getScriptType() == sim_scripttype_main)
    {
        if (checkInputArguments(L, &errorString, lua_arg_number, 0))
        {
            int callType = luaToInt(L, 1);
            int editMode = NO_EDIT_MODE;
#ifdef SIM_WITH_GUI
            editMode = GuiApp::getEditModeType();
#endif
            if ((editMode == NO_EDIT_MODE) && (App::worldContainer->sandboxScript != nullptr))
                App::worldContainer->sandboxScript->systemCallScript(callType, nullptr, nullptr);
        }
    }
    else
        errorString = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simAlignShapeBB(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.alignShapeBB");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_number, 7))
    {
        int shapeHandle = luaToInt(L, 1);
        double pose[7];
        getDoublesFromTable(L, 2, 7, pose);
        retVal = CALL_C_API(simAlignShapeBB, shapeHandle, pose);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushboolean(L, retVal == 1);
    LUA_END(1);
}

int _simRelocateShapeFrame(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.relocateShapeFrame");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, lua_arg_integer, 0, lua_arg_number, 7))
    {
        int shapeHandle = luaToInt(L, 1);
        double pose[7];
        getDoublesFromTable(L, 2, 7, pose);
        retVal = CALL_C_API(simRelocateShapeFrame, shapeHandle, pose);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushboolean(L, retVal == 1);
    LUA_END(1);
}

int _simCreateOctree(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.createOctree");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number | lua_arg_optional, 0, lua_arg_integer | lua_arg_optional, 0, lua_arg_number | lua_arg_optional, 0))
    {
        double voxelSize = fetchDoubleArg(L, 1, 0.025);
        int options = fetchIntArg(L, 2, 0);
        double pointSize = fetchDoubleArg(L, 3, 2.0);
        retVal = CALL_C_API(simCreateOctree, voxelSize, options, pointSize, nullptr);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCreatePointCloud(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.createPointCloud");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number | lua_arg_optional, 0, lua_arg_integer | lua_arg_optional, 0, lua_arg_integer | lua_arg_optional, 0, lua_arg_number | lua_arg_optional, 0))
    {
        double maxVoxelSize = fetchDoubleArg(L, 1, 0.02);
        int maxPtCntPerVoxel = fetchIntArg(L, 2, 20);
        int options = fetchIntArg(L, 3, 0);
        double pointSize = fetchDoubleArg(L, 4, 4.0);
        retVal = CALL_C_API(simCreatePointCloud, maxVoxelSize, maxPtCntPerVoxel, options, pointSize, nullptr);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetPointCloudOptions(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setPointCloudOptions");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0,
                            lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        double maxVoxelSize = luaToDouble(L, 2);
        int maxPtCntPerVoxel = luaToInt(L, 3);
        int options = luaToInt(L, 4);
        double pointSize = luaToDouble(L, 5);
        retVal = CALL_C_API(simSetPointCloudOptions, handle, maxVoxelSize, maxPtCntPerVoxel, options, pointSize, nullptr);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetPointCloudOptions(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getPointCloudOptions");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        double maxVoxelSize;
        int maxPtCntPerVoxel;
        int options;
        double pointSize;
        int retVal =
            CALL_C_API(simGetPointCloudOptions, handle, &maxVoxelSize, &maxPtCntPerVoxel, &options, &pointSize, nullptr);
        if (retVal >= 0)
        {
            luaWrap_lua_pushnumber(L, maxVoxelSize);
            luaWrap_lua_pushinteger(L, maxPtCntPerVoxel);
            luaWrap_lua_pushinteger(L, options);
            luaWrap_lua_pushnumber(L, pointSize);
            LUA_END(4);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simInsertVoxelsIntoOctree(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.insertVoxelsIntoOctree");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        int handleFlags = handle & 0xff00000;
        handle = handle & 0xfffff;
        int options = luaToInt(L, 2);
        unsigned char* cols = nullptr;
        std::vector<unsigned char> _cols;
        int v = 3;
        std::vector<double> pts;
        if ((handleFlags & sim_handleflag_codedstring) != 0)
        { // provided data is in buffers
            if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_string, 0))
            {
                size_t dataLength;
                const char* data = luaWrap_lua_tobuffer(L, 3, &dataLength);
                int ptCnt = int((dataLength / sizeof(float)) / 3);
                pts.resize(ptCnt * 3);
                if (options & 2)
                    v = ptCnt * 3;
                int res = checkOneGeneralInputArgument(L, 4, lua_arg_string, 0, true, true, &errorString);
                if (res >= 0)
                {
                    for (int i = 0; i < ptCnt * 3; i++)
                        pts[i] = double(((float*)data)[i]);
                    if (res == 2)
                    {
                        data = luaWrap_lua_tobuffer(L, 4, &dataLength);
                        _cols.resize(v);
                        for (int i = 0; i < std::min<int>(v, int(dataLength)); i++)
                            _cols[i] = ((unsigned char*)data)[i];
                        cols = &_cols[0];
                    }
                    res = checkOneGeneralInputArgument(L, 5, lua_arg_string, 0, true, true, &errorString);
                    if (res >= 0)
                    {
                        if (cols == nullptr)
                            retVal = CALL_C_API(simInsertVoxelsIntoOctree, handle, options, &pts[0], ptCnt, nullptr,
                                                                        nullptr, nullptr);
                        else
                        {
                            unsigned int* tags = nullptr;
                            std::vector<unsigned int> _tags;
                            if (res == 2)
                            {
                                data = luaWrap_lua_tobuffer(L, 5, &dataLength);
                                _tags.resize(v / 3);
                                for (int i = 0; i < std::min<int>(v / 3, int(dataLength / sizeof(int))); i++)
                                    _tags[i] = ((unsigned int*)data)[i];
                                tags = &_tags[0];
                            }
                            retVal = CALL_C_API(simInsertVoxelsIntoOctree, handle, options, &pts[0], ptCnt, cols, tags,
                                                                        nullptr);
                        }
                    }
                }
            }
        }
        else
        { // provided data is in tables
            if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 3))
            {
                int ptCnt = int(luaWrap_lua_rawlen(L, 3)) / 3;
                pts.resize(ptCnt * 3);
                if (options & 2)
                    v = ptCnt * 3;
                int res = checkOneGeneralInputArgument(L, 4, lua_arg_number, v, true, true, &errorString);
                if (res >= 0)
                {
                    getDoublesFromTable(L, 3, ptCnt * 3, &pts[0]);
                    if (res == 2)
                    {
                        _cols.resize(v);
                        getUCharsFromTable(L, 4, v, &_cols[0]);
                        cols = &_cols[0];
                    }
                    res = checkOneGeneralInputArgument(L, 5, lua_arg_number, v / 3, true, true, &errorString);
                    if (res >= 0)
                    {
                        if (cols == nullptr)
                            retVal = CALL_C_API(simInsertVoxelsIntoOctree, handle, options, &pts[0], ptCnt, nullptr,
                                                                        nullptr, nullptr);
                        else
                        {
                            unsigned int* tags = nullptr;
                            std::vector<unsigned int> _tags;
                            if (res == 2)
                            {
                                _tags.resize(v / 3);
                                getUIntsFromTable(L, 5, v / 3, &_tags[0]);
                                tags = &_tags[0];
                            }
                            retVal = CALL_C_API(simInsertVoxelsIntoOctree, handle, options, &pts[0], ptCnt, cols, tags,
                                                                        nullptr);
                        }
                    }
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simRemoveVoxelsFromOctree(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.removeVoxelsFromOctree");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        int options = luaToInt(L, 2);
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 3, false, true, &errorString);
        if (res >= 1)
        {
            if (res == 2)
            { // remove some voxels
                int ptCnt = int(luaWrap_lua_rawlen(L, 3)) / 3;
                std::vector<double> pts;
                pts.resize(ptCnt * 3);
                getDoublesFromTable(L, 3, ptCnt * 3, &pts[0]);
                retVal = CALL_C_API(simRemoveVoxelsFromOctree, handle, options, &pts[0], ptCnt, nullptr);
            }
            else
                retVal = CALL_C_API(simRemoveVoxelsFromOctree, handle, options, nullptr, 0, nullptr); // remove all voxels!
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simInsertPointsIntoPointCloud(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.insertPointsIntoPointCloud");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        int handleFlags = handle & 0xff00000;
        handle = handle & 0xfffff;
        int options = luaToInt(L, 2);
        float optionalValues[2];
        ((int*)optionalValues)[0] = 1; // duplicate tolerance bit
        optionalValues[1] = 0.0;       // duplicate tolerance
        unsigned char* cols = nullptr;
        std::vector<unsigned char> _cols;
        int v = 3;
        std::vector<double> pts;
        if ((handleFlags & sim_handleflag_codedstring) != 0)
        { // provided data is in buffers
            if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_string, 0))
            {
                size_t dataLength;
                const char* data = luaWrap_lua_tobuffer(L, 3, &dataLength);
                int ptCnt = int((dataLength / sizeof(float))) / 3;
                pts.resize(ptCnt * 3);
                if (options & 2)
                    v = ptCnt * 3;
                int res = checkOneGeneralInputArgument(L, 4, lua_arg_string, 0, true, true, &errorString);
                if (res >= 0)
                {
                    for (int i = 0; i < ptCnt * 3; i++)
                        pts[i] = double(((float*)data)[i]);
                    if (res == 2)
                    {
                        data = luaWrap_lua_tobuffer(L, 4, &dataLength);
                        _cols.resize(v);
                        for (int i = 0; i < std::min<int>(v, int(dataLength)); i++)
                            _cols[i] = ((unsigned char*)data)[i];
                        cols = &_cols[0];
                    }
                    res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, true, true, &errorString);
                    if (res >= 0)
                    {
                        if (res == 2)
                        {
                            optionalValues[1] = (float)luaToDouble(L, 5); // duplicate tolerance
                            retVal = CALL_C_API(simInsertPointsIntoPointCloud, handle, options, &pts[0], ptCnt, cols,
                                                                            optionalValues);
                        }
                        else
                            retVal =
                                CALL_C_API(simInsertPointsIntoPointCloud, handle, options, &pts[0], ptCnt, cols, nullptr);
                    }
                }
            }
        }
        else
        { // provided data is in tables
            if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 3))
            {
                int ptCnt = int(luaWrap_lua_rawlen(L, 3)) / 3;
                pts.resize(ptCnt * 3);
                if (options & 2)
                    v = ptCnt * 3;
                int res = checkOneGeneralInputArgument(L, 4, lua_arg_number, v, true, true, &errorString);
                if (res >= 0)
                {
                    getDoublesFromTable(L, 3, ptCnt * 3, &pts[0]);
                    if (res == 2)
                    {
                        _cols.resize(v);
                        getUCharsFromTable(L, 4, v, &_cols[0]);
                        cols = &_cols[0];
                    }
                    res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, true, true, &errorString);
                    if (res >= 0)
                    {
                        if (res == 2)
                        {
                            optionalValues[1] = (float)luaToDouble(L, 5); // duplicate tolerance
                            retVal = CALL_C_API(simInsertPointsIntoPointCloud, handle, options, &pts[0], ptCnt, cols,
                                                                            optionalValues);
                        }
                        else
                            retVal =
                                CALL_C_API(simInsertPointsIntoPointCloud, handle, options, &pts[0], ptCnt, cols, nullptr);
                    }
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simRemovePointsFromPointCloud(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.removePointsFromPointCloud");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        int options = luaToInt(L, 2);
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 3, false, true, &errorString);
        if (res >= 1)
        {
            int res2 = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, false, false, &errorString);
            if (res2 == 2)
            {
                double tolerance = luaToDouble(L, 4);
                if (res == 2)
                { // remove some points
                    int ptCnt = int(luaWrap_lua_rawlen(L, 3)) / 3;
                    std::vector<double> pts;
                    pts.resize(ptCnt * 3);
                    getDoublesFromTable(L, 3, ptCnt * 3, &pts[0]);
                    retVal =
                        CALL_C_API(simRemovePointsFromPointCloud, handle, options, &pts[0], ptCnt, tolerance, nullptr);
                }
                else
                    retVal = CALL_C_API(simRemovePointsFromPointCloud, handle, options, nullptr, 0, 0.0,
                                                                    nullptr); // remove all points
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simIntersectPointsWithPointCloud(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.intersectPointsWithPointCloud");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        int options = luaToInt(L, 2);
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 3, false, true, &errorString);
        if (res >= 1)
        {
            int res2 = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, false, false, &errorString);
            if (res2 == 2)
            {
                double tolerance = luaToDouble(L, 4);
                if (res == 2)
                { // intersect some points
                    int ptCnt = int(luaWrap_lua_rawlen(L, 3)) / 3;
                    std::vector<double> pts;
                    pts.resize(ptCnt * 3);
                    getDoublesFromTable(L, 3, ptCnt * 3, &pts[0]);
                    retVal =
                        CALL_C_API(simIntersectPointsWithPointCloud, handle, options, &pts[0], ptCnt, tolerance, nullptr);
                }
                else
                    retVal = CALL_C_API(simRemovePointsFromPointCloud, handle, options, nullptr, 0, 0.0,
                                                                    nullptr); // remove all points
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetOctreeVoxels(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getOctreeVoxels");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        int ptCnt = -1;
        const double* p = CALL_C_API(simGetOctreeVoxels, handle, &ptCnt, nullptr);
        if (ptCnt >= 0)
        {
            pushDoubleTableOntoStack(L, ptCnt * 3, p);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetPointCloudPoints(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getPointCloudPoints");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        int ptCnt = -1;
        const double* p = CALL_C_API(simGetPointCloudPoints, handle, &ptCnt, nullptr);
        if (ptCnt >= 0)
        {
            pushDoubleTableOntoStack(L, ptCnt * 3, p);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simInsertObjectIntoOctree(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.insertObjectIntoOctree");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        int handle1 = luaToInt(L, 1);
        int handle2 = luaToInt(L, 2);
        int options = luaToInt(L, 3);
        unsigned char col[3];
        unsigned char* c = nullptr;
        int tag = 0;
        int res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 3, true, true, &errorString);
        if (res >= 0)
        {
            if (res == 2)
            {
                getUCharsFromTable(L, 4, 3, col);
                c = col;
            }
            res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, true, true, &errorString);
            if (res == 2)
                tag = (unsigned int)luaToInt(L, 5);
            retVal = CALL_C_API(simInsertObjectIntoOctree, handle1, handle2, options, c, tag, nullptr);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSubtractObjectFromOctree(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.subtractObjectFromOctree");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        int handle1 = luaToInt(L, 1);
        int handle2 = luaToInt(L, 2);
        int options = luaToInt(L, 3);
        retVal = CALL_C_API(simSubtractObjectFromOctree, handle1, handle2, options, nullptr);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simInsertObjectIntoPointCloud(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.insertObjectIntoPointCloud");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number,
                            0))
    {
        int handle1 = luaToInt(L, 1);
        int handle2 = luaToInt(L, 2);
        int options = luaToInt(L, 3);
        double gridSize = luaToDouble(L, 4);
        float optionalValues[2];
        ((int*)optionalValues)[0] = 1; // duplicate tolerance bit
        optionalValues[1] = 0.0;       // duplicate tolerance
        unsigned char col[3];
        unsigned char* c = nullptr;
        int res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 3, true, true, &errorString);
        if (res >= 0)
        {
            if (res == 2)
            {
                getUCharsFromTable(L, 5, 3, col);
                c = col;
            }
            res = checkOneGeneralInputArgument(L, 6, lua_arg_number, 0, true, true, &errorString);
            if (res >= 0)
            {
                if (res == 2)
                {
                    optionalValues[1] = (float)luaToDouble(L, 6); // duplicate tolerance
                    retVal =
                        CALL_C_API(simInsertObjectIntoPointCloud, handle1, handle2, options, gridSize, c, optionalValues);
                }
                else
                    retVal = CALL_C_API(simInsertObjectIntoPointCloud, handle1, handle2, options, gridSize, c, nullptr);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSubtractObjectFromPointCloud(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.subtractObjectFromPointCloud");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number,
                            0))
    {
        int handle1 = luaToInt(L, 1);
        int handle2 = luaToInt(L, 2);
        int options = luaToInt(L, 3);
        double tolerance = luaToDouble(L, 4);
        retVal = CALL_C_API(simSubtractObjectFromPointCloud, handle1, handle2, options, tolerance, nullptr);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCheckOctreePointOccupancy(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.checkOctreePointOccupancy");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 3))
    {
        int handle = luaToInt(L, 1);
        int options = luaToInt(L, 2);
        int ptCnt = int(luaWrap_lua_rawlen(L, 3)) / 3;
        std::vector<double> points;
        points.resize(ptCnt * 3);
        getDoublesFromTable(L, 3, ptCnt * 3, &points[0]);
        unsigned int tag = 0;
        unsigned long long int location = 0;
        unsigned int locLow = location & 0xffffffff;
        unsigned int locHigh = (location >> 32) & 0xffffffff;
        retVal = CALL_C_API(simCheckOctreePointOccupancy, handle, options, &points[0], ptCnt, &tag, &location, nullptr);
        luaWrap_lua_pushinteger(L, retVal);
        luaWrap_lua_pushinteger(L, tag);
        luaWrap_lua_pushinteger(L, locLow);
        luaWrap_lua_pushinteger(L, locHigh);
        LUA_END(4);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simPackTable(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.packTable");

    if (luaWrap_lua_gettop(L) >= 1)
    {
        if (luaWrap_lua_isnonbuffertable(L, 1))
        {
            int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, true, &errorString);
            if (res >= 0)
            {
                int scheme = 0;
                if (res == 2)
                    scheme = luaToInt(L, 2);
                CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
                CScriptObject::buildFromInterpreterStack_lua(L, stack, 1, 1);
                std::string s;
                if (scheme == 0)
                    s = stack->getBufferFromTable();
                // (Since 10.07.2025 a Lua wrapper handles CBOR encoding)
                if (scheme == 1)
                    s = stack->getCborEncodedBuffer(1);
                if (scheme == 2)
                    s = stack->getCborEncodedBuffer(0); // doubles coded as double
                luaWrap_lua_pushbuffer(L, s.c_str(), s.length());
                App::worldContainer->interfaceStackContainer->destroyStack(stack);
                LUA_END(1);
            }
        }
        else
            errorString.assign(SIM_ERROR_ONE_ARGUMENT_TYPE_IS_WRONG);
    }
    else
        errorString.assign(SIM_ERROR_FUNCTION_REQUIRES_MORE_ARGUMENTS);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simUnpackTable(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.unpackTable");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0))
    {
        size_t l;
        const char* s = luaWrap_lua_tobuffer(L, 1, &l);
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
        if (stack->pushTableFromBuffer(s, (unsigned int)l))
        {
            CScriptObject::buildOntoInterpreterStack_lua(L, stack, true);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
            LUA_END(1);
        }
        App::worldContainer->interfaceStackContainer->destroyStack(stack);
        errorString.assign(SIM_ERROR_INVALID_DATA);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simHandleSimulationStart(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.handleSimulationStart");

    int retVal = -1;
    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* itScrObj = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
    if (itScrObj->getScriptType() == sim_scripttype_main)
    {
        // Following is for velocity measurement (initial):
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_joint); i++)
            App::currentWorld->sceneObjects->getJointFromIndex(i)->measureJointVelocity(0.0);
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(); i++)
            App::currentWorld->sceneObjects->getObjectFromIndex(i)->measureVelocity(0.0);
    }
    else
        errorString = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simHandleSensingStart(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.handleSensingStart");

    int retVal = -1;
    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* itScrObj = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
    if (itScrObj->getScriptType() == sim_scripttype_main)
    {
        // Following is for camera tracking!
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_camera); i++)
        {
            CCamera* it = App::currentWorld->sceneObjects->getCameraFromIndex(i);
            it->handleCameraTracking();
        }

        // Following is for velocity measurement:
        double dt = App::currentWorld->simulation->getTimeStep();
        double t = dt + App::currentWorld->simulation->getSimulationTime();
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_joint); i++)
            App::currentWorld->sceneObjects->getJointFromIndex(i)->measureJointVelocity(t);
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(); i++)
            App::currentWorld->sceneObjects->getObjectFromIndex(i)->measureVelocity(dt); // adapt that func!
    }
    else
        errorString = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAuxFunc(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.auxFunc");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0))
    {
        std::string cmd(luaWrap_lua_tostring(L, 1));
        if (cmd.compare("createMirror") == 0)
        {
            if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 2))
            {
                double s[2];
                getDoublesFromTable(L, 2, 2, s);
                CMirror* m = new CMirror();
                m->setMirrorWidth(s[0]);
                m->setMirrorHeight(s[1]);
                App::currentWorld->sceneObjects->addObjectToScene(m, false, true);
                int h = m->getObjectHandle();
                luaWrap_lua_pushinteger(L, h);
                LUA_END(1);
            }
            LUA_END(0);
        }
        if (cmd.compare("activateMainWindow") == 0)
        {
#ifdef SIM_WITH_GUI
            if (GuiApp::mainWindow != nullptr)
                GuiApp::mainWindow->activateMainWindow();
#endif
            LUA_END(0);
        }
        if (cmd.compare("sleep") == 0)
        {
            int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, false, false, &errorString);
            if (res == 2)
            {
                int tInMs = int(luaToDouble(L, 2) * 1000.0);
                VThread::sleep(tInMs);
            }
            LUA_END(0);
        }
        if (cmd.compare("curveToClipboard") == 0)
        {
            if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 0, lua_arg_number, 0,
                                    lua_arg_string, 0))
            {
                int graphHandle = luaToInt(L, 2);
                int curveType = luaToInt(L, 3);
                std::string curveName(luaWrap_lua_tostring(L, 4));
                CGraph* it = App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
                if (it != nullptr)
                {
                    it->curveToClipboard(curveType, curveName.c_str());
                    LUA_END(0);
                }
                else
                    errorString = SIM_ERROR_INVALID_HANDLE;
            }
        }
        if (cmd.compare("curveToStatic") == 0)
        {
            if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 0, lua_arg_number, 0,
                                    lua_arg_string, 0))
            {
                int graphHandle = luaToInt(L, 2);
                int curveType = luaToInt(L, 3);
                std::string curveName(luaWrap_lua_tostring(L, 4));
                CGraph* it = App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
                if (it != nullptr)
                {
                    it->curveToStatic(curveType, curveName.c_str());
                    LUA_END(0);
                }
                else
                    errorString = SIM_ERROR_INVALID_HANDLE;
            }
        }
        if (cmd.compare("removeStaticCurve") == 0)
        {
            if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 0, lua_arg_number, 0,
                                    lua_arg_string, 0))
            {
                int graphHandle = luaToInt(L, 2);
                int curveType = luaToInt(L, 3);
                std::string curveName(luaWrap_lua_tostring(L, 4));
                CGraph* it = App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
                if (it != nullptr)
                {
                    it->removeStaticCurve(curveType, curveName.c_str());
                    LUA_END(0);
                }
                else
                    errorString = SIM_ERROR_INVALID_HANDLE;
            }
        }
        if (cmd.compare("setAssemblyMatchValues") == 0)
        {
            if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 0, lua_arg_bool, 0,
                                    lua_arg_string, 0))
            {
                int objHandle = luaToInt(L, 2);
                bool childAttr = luaToBool(L, 3);
                std::string matchValues(luaWrap_lua_tostring(L, 4));
                CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objHandle);
                if (it != nullptr)
                {
                    it->setAssemblyMatchValues(childAttr, matchValues.c_str());
                    LUA_END(0);
                }
                else
                    errorString = SIM_ERROR_INVALID_HANDLE;
            }
        }
        if (cmd.compare("getAssemblyMatchValues") == 0)
        {
            if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 0, lua_arg_bool, 0))
            {
                int objHandle = luaToInt(L, 2);
                bool childAttr = luaToBool(L, 3);
                CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objHandle);
                if (it != nullptr)
                {
                    std::string str(it->getAssemblyMatchValues(childAttr));
                    luaWrap_lua_pushtext(L, str.c_str());
                    LUA_END(1);
                }
                else
                    errorString = SIM_ERROR_INVALID_HANDLE;
            }
        }
#ifdef SIM_WITH_GUI
        if (cmd.compare("drawImageLines") == 0)
        {
            if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_string, 0, lua_arg_number, 2,
                                    lua_arg_number, 4, lua_arg_number, 3, lua_arg_number, 0))
            {
                int res[2];
                getIntsFromTable(L, 3, 2, res);
                size_t imgLen;
                const char* img = luaWrap_lua_tobuffer(L, 2, &imgLen);
                if (imgLen == size_t(3 * res[0] * res[1]))
                {
                    std::vector<int> lines;
                    int vc = int(luaWrap_lua_rawlen(L, 4));
                    vc = 2 * (vc / 2);
                    lines.resize(vc);
                    getIntsFromTable(L, 4, vc, &lines[0]);
                    int col[3];
                    getIntsFromTable(L, 5, 3, col);
                    int lineWidth = luaToInt(L, 6);
                    QImage qimg((const unsigned char*)img, res[0], res[1], res[0] * 3, QImage::Format_RGB888);
                    QPixmap pix(QPixmap::fromImage(qimg));
                    QPainter paint(&pix);
                    QColor qcol(col[0], col[1], col[2], 255);
                    QPen pen(qcol);
                    pen.setWidth(lineWidth);
                    paint.setPen(pen);
                    for (size_t i = 0; i < (lines.size() / 2) - 1; i++)
                        paint.drawLine(lines[2 * i + 0], lines[2 * i + 1], lines[2 * i + 2], lines[2 * i + 3]);
                    qimg = pix.toImage();
                    std::vector<unsigned char> img2;
                    img2.resize(res[0] * res[1] * 3);
                    for (int y = 0; y < res[1]; y++)
                    {
                        for (int x = 0; x < res[0]; x++)
                        {
                            QRgb rgb = qimg.pixel(x, y);
                            img2[y * res[0] * 3 + 3 * x + 0] = qRed(rgb);
                            img2[y * res[0] * 3 + 3 * x + 1] = qGreen(rgb);
                            img2[y * res[0] * 3 + 3 * x + 2] = qBlue(rgb);
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)&img2[0], res[0] * res[1] * 3);
                    LUA_END(1);
                }
            }
        }
        if (cmd.compare("drawImageSquares") == 0)
        {
            if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_string, 0, lua_arg_number, 2,
                                    lua_arg_number, 2, lua_arg_number, 3, lua_arg_number, 0))
            {
                int res[2];
                getIntsFromTable(L, 3, 2, res);
                size_t imgLen;
                const char* img = luaWrap_lua_tobuffer(L, 2, &imgLen);
                if (imgLen == size_t(3 * res[0] * res[1]))
                {
                    std::vector<int> rects;
                    int vc = int(luaWrap_lua_rawlen(L, 4));
                    vc = 4 * (vc / 4);
                    rects.resize(vc);
                    getIntsFromTable(L, 4, vc, &rects[0]);
                    int col[3];
                    getIntsFromTable(L, 5, 3, col);
                    int lineWidth = luaToInt(L, 6);
                    QImage qimg((const unsigned char*)img, res[0], res[1], res[0] * 3, QImage::Format_RGB888);
                    QPixmap pix(QPixmap::fromImage(qimg));
                    QPainter paint(&pix);
                    QColor qcol(col[0], col[1], col[2], 255);
                    QPen pen(qcol);
                    pen.setWidth(lineWidth);
                    paint.setPen(pen);
                    for (size_t i = 0; i < rects.size() / 4; i++)
                        paint.drawRect(rects[4 * i + 0], rects[4 * i + 1], rects[4 * i + 2], rects[4 * i + 3]);
                    qimg = pix.toImage();
                    std::vector<unsigned char> img2;
                    img2.resize(res[0] * res[1] * 3);
                    for (int y = 0; y < res[1]; y++)
                    {
                        for (int x = 0; x < res[0]; x++)
                        {
                            QRgb rgb = qimg.pixel(x, y);
                            img2[y * res[0] * 3 + 3 * x + 0] = qRed(rgb);
                            img2[y * res[0] * 3 + 3 * x + 1] = qGreen(rgb);
                            img2[y * res[0] * 3 + 3 * x + 2] = qBlue(rgb);
                        }
                    }
                    luaWrap_lua_pushbuffer(L, (const char*)&img2[0], res[0] * res[1] * 3);
                    LUA_END(1);
                }
            }
        }
#endif
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetReferencedHandles(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setReferencedHandles");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int objHandle = luaToInt(L, 1);
        if (luaWrap_lua_isnonbuffertable(L, 2))
        {
            int cnt = (int)luaWrap_lua_rawlen(L, 2);
            if (cnt == 0)
                cnt = -1; // so that checkInputArguments doesn't fail with arg 2
            std::string tag("");
            if (checkInputArguments(L, nullptr, lua_arg_number, 0, lua_arg_integer, cnt, lua_arg_string, 0))
                tag = luaWrap_lua_tostring(L, 3);
            if (cnt > 0)
            {
                if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_integer, cnt))
                {
                    std::vector<int> handles;
                    handles.resize(cnt);
                    getIntsFromTable(L, 2, cnt, &handles[0]);
                    retVal = CALL_C_API(simSetReferencedHandles, objHandle, cnt, &handles[0], tag.c_str(), nullptr);
                }
            }
            else
                retVal = CALL_C_API(simSetReferencedHandles, objHandle, 0, nullptr, tag.c_str(), nullptr);
        }
        else
            errorString = "bad argument #2 (expecting a table)";
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetReferencedHandles(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getReferencedHandles");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        std::string tag("");
        if (checkInputArguments(L, nullptr, lua_arg_number, 0, lua_arg_string, 0))
            tag = luaWrap_lua_tostring(L, 2);
        int objHandle = luaToInt(L, 1);
        int* handles;
        int cnt = CALL_C_API(simGetReferencedHandles, objHandle, &handles, tag.c_str(), nullptr);
        if (cnt >= 0)
        {
            pushIntTableOntoStack(L, cnt, handles);
            delete[] handles;
            LUA_END(1);
        }
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetReferencedHandlesTags(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getReferencedHandlesTags");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int objHandle = luaToInt(L, 1);
        int handleFlags = objHandle & 0xff00000;
        objHandle = objHandle & 0xfffff;
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objHandle);
        if (it != nullptr)
        {
            std::vector<std::string> tags;
            if ((handleFlags & sim_handleflag_keeporiginal) == 0)
                it->getReferencedHandlesTags(tags);
            else
                it->getReferencedOriginalHandlesTags(tags);
            pushStringTableOntoStack(L, tags);
            LUA_END(1);
        }
        else
            errorString = SIM_ERROR_OBJECT_INEXISTANT;
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetGraphCurve(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getGraphCurve");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        int graphHandle = luaToInt(L, 1);
        int graphType = luaToInt(L, 2);
        int index = luaToInt(L, 3);
        CGraph* graph = App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
        if (graph != nullptr)
        {
            std::vector<double> xVals;
            std::vector<double> yVals;
            std::string label;
            int curveType;
            int curveId;
            long long int curveUid;
            int curveWidth;
            float col[3];
            double minMax[6];
            if (graph->getGraphCurveData(graphType, index, label, xVals, yVals, curveType, col, minMax, curveId, curveWidth, curveUid))
            {
                luaWrap_lua_pushtext(L, label.c_str());
                luaWrap_lua_pushinteger(L, curveType);
                pushFloatTableOntoStack(L, 3, col);
                if (xVals.size() > 0)
                    pushDoubleTableOntoStack(L, (int)xVals.size(), &xVals[0]);
                else
                    pushIntTableOntoStack(L, 0, nullptr); // empty table
                if (yVals.size() > 0)
                    pushDoubleTableOntoStack(L, (int)yVals.size(), &yVals[0]);
                else
                    pushIntTableOntoStack(L, 0, nullptr); // empty table
                pushDoubleTableOntoStack(L, 6, minMax);
                luaWrap_lua_pushinteger(L, curveId);
                luaWrap_lua_pushinteger(L, curveWidth);
                luaWrap_lua_pushinteger(L, curveUid);
                LUA_END(9);
            }
        }
        else
            errorString = SIM_ERROR_OBJECT_NOT_GRAPH;
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetGraphInfo(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getGraphInfo");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        int graphHandle = luaToInt(L, 1);
        CGraph* graph = App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
        if (graph != nullptr)
        {
            int bitCoded = 0;
            luaWrap_lua_pushinteger(L, bitCoded);
            pushFloatTableOntoStack(L, 3, graph->backgroundColor);
            pushFloatTableOntoStack(L, 3, graph->foregroundColor);
            LUA_END(3);
        }
        else
            errorString = SIM_ERROR_OBJECT_NOT_GRAPH;
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetShapeViz(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getShapeViz");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_number, 0))
    {
        int shapeHandle = luaToInt(L, 1);
        int index = luaToInt(L, 2);
        SShapeVizInfof info;
        int ret = CALL_C_API(simGetShapeVizf, shapeHandle + sim_handleflag_extended, index, &info);
        if (ret > 0)
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            stack->pushTableOntoStack();

            stack->insertKeyFloatArrayIntoStackTable("vertices", info.vertices, size_t(info.verticesSize));
            stack->insertKeyInt32ArrayIntoStackTable("indices", info.indices, size_t(info.indicesSize));
            stack->insertKeyFloatArrayIntoStackTable("normals", info.normals, size_t(info.indicesSize * 3));
            stack->insertKeyFloatArrayIntoStackTable("colors", info.colors, 9);
            stack->insertKeyFloatIntoStackTable("shadingAngle", info.shadingAngle);
            stack->insertKeyFloatIntoStackTable("transparency", info.transparency);
            stack->insertKeyInt32IntoStackTable("options", info.options);

            delete[] info.vertices;
            delete[] info.indices;
            delete[] info.normals;
            if (ret > 1)
            {
                stack->pushTextOntoStack("texture");
                stack->pushTableOntoStack();

                stack->insertKeyBufferIntoStackTable("texture", info.texture,
                                                     4 * info.textureRes[0] * info.textureRes[1]);
                stack->insertKeyInt32ArrayIntoStackTable("resolution", info.textureRes, 2);
                stack->insertKeyFloatArrayIntoStackTable("coordinates", info.textureCoords,
                                                         size_t(info.indicesSize * 2));
                stack->insertKeyInt32IntoStackTable("applyMode", info.textureApplyMode);
                stack->insertKeyInt32IntoStackTable("options", info.textureOptions);
                stack->insertKeyInt32IntoStackTable("id", info.textureId);

                stack->insertDataIntoStackTable();
                delete[] info.texture;
                delete[] info.textureCoords;
            }
            CScriptObject::buildOntoInterpreterStack_lua(L, stack, true);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
            LUA_END(1);
        }
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simExecuteScriptString(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.executeScriptString");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 0))
    {
        std::string stringToExecute(luaWrap_lua_tostring(L, 1));
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
        int scriptID = luaToInt(L, 2);
        if (scriptID == sim_handle_self)
            scriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        int retVal = CALL_C_API(simExecuteScriptString, scriptID, stringToExecute.c_str(), stack->getId());
        if (retVal >= 0)
        {
            luaWrap_lua_pushinteger(L, retVal);
            int s = 1;
            if (stack->getStackSize() > 0)
            {
                CScriptObject::buildOntoInterpreterStack_lua(L, stack, false); // true);
                s += stack->getStackSize();
            }
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
            if (s == 1)
            {
                pushIntTableOntoStack(L, 0, nullptr);
                s++;
            }
            LUA_END(s);
        }
        else
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetApiFunc(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getApiFunc");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_string, 0))
    {
        std::string apiWord(luaWrap_lua_tostring(L, 2));
        char* str = CALL_C_API(simGetApiFunc, luaToInt(L, 1), apiWord.c_str());
        std::vector<std::string> strTable;
        if (str != nullptr)
        {
            std::string sstr(str);
            CALL_C_API(simReleaseBuffer, str);
            size_t prevPos = 0;
            size_t spacePos = sstr.find(' ', prevPos);
            while (spacePos != std::string::npos)
            {
                strTable.push_back(std::string(sstr.begin() + prevPos, sstr.begin() + spacePos));
                prevPos = spacePos + 1;
                spacePos = sstr.find(' ', prevPos);
            }
            strTable.push_back(std::string(sstr.begin() + prevPos, sstr.end()));
        }
        pushStringTableOntoStack(L, strTable);
        LUA_END(1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetApiInfo(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getApiInfo");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0, lua_arg_string, 0))
    {
        std::string apiWord(luaWrap_lua_tostring(L, 2));
        char* str = CALL_C_API(simGetApiInfo, luaToInt(L, 1), apiWord.c_str());
        if (str != nullptr)
        {
            luaWrap_lua_pushtext(L, str);
            CALL_C_API(simReleaseBuffer, str);
            LUA_END(1);
        }
        LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetPluginName(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getPluginName");

    if (checkInputArguments(L, &errorString, lua_arg_number, 0))
    {
        unsigned char version;
        char* name = CALL_C_API(simGetPluginName, luaToInt(L, 1), &version);
        if (name != nullptr)
        {
            luaWrap_lua_pushtext(L, name);
            CALL_C_API(simReleaseBuffer, name);
            luaWrap_lua_pushinteger(L, (int)version);
            LUA_END(2);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetPluginInfo(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getPluginInfo");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 0))
    {
        char* stringInfo;
        int intInfo;
        std::string pluginName(luaWrap_lua_tostring(L, 1));
        int infoType = luaToInt(L, 2);
        int res = CALL_C_API(simGetPluginInfo, pluginName.c_str(), infoType, &stringInfo, &intInfo);
        if (res >= 0)
        {
            if ((infoType == sim_plugininfo_extversionstr) || (infoType == sim_plugininfo_builddatestr))
            {
                luaWrap_lua_pushtext(L, stringInfo);
                delete[] stringInfo;
                LUA_END(1);
            }
            if ((infoType == sim_plugininfo_extversionint) || (infoType == sim_plugininfo_verbosity) ||
                (infoType == sim_plugininfo_statusbarverbosity))
            {
                luaWrap_lua_pushinteger(L, intInfo);
                LUA_END(1);
            }
        }
        LUA_END(0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetPluginInfo(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setPluginInfo");

    if (checkInputArguments(L, &errorString, lua_arg_string, 0, lua_arg_number, 0))
    {
        std::string pluginName(luaWrap_lua_tostring(L, 1));
        int infoType = luaToInt(L, 2);
        if ((infoType == sim_plugininfo_verbosity) || (infoType == sim_plugininfo_statusbarverbosity))
        {
            int res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, false, false, &errorString);
            if (res == 2)
            {
                int verbosity = luaToInt(L, 3);
                CALL_C_API(simSetPluginInfo, pluginName.c_str(), infoType, nullptr, verbosity);
            }
            else
                errorString = SIM_ERROR_ONE_ARGUMENT_TYPE_IS_WRONG;
        }
        else
            errorString = SIM_ERROR_INVALID_PLUGIN_INFO_TYPE;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simHandleExtCalls(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.handleExtCalls");
    App::simThread->handleExtCalls();
    LUA_END(0);
}

int _simGetLastInfo(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getLastInfo");

    const char* info = CALL_C_API(simGetLastInfo, );
    std::string inf;
    if (info != nullptr)
    {
        inf = info;
        delete[] info;
    }
    luaWrap_lua_pushtext(L, inf.c_str());
    LUA_END(1);
}

#include <luaScriptFunctions-old.cpp>
