#include <sim.h>
#include <simInternal.h>
#include <app.h>

#ifdef SIM_UNIFIED_HANDLES
SIM_DLLEXPORT UID simGetObject_L(const char* objectPath, int index, UID proxy, int options)
{
    return simGetObject_internal(objectPath, index, proxy, options);
}

SIM_DLLEXPORT long long int simGetObjectUid_L(UID objectHandle)
{
    return (simGetObjectUid_internal(objectHandle));
}

SIM_DLLEXPORT UID simGetObjectFromUid_L(long long int uid, int options)
{
    return (simGetObjectFromUid_internal(uid, options));
}

SIM_DLLEXPORT UID simGetScriptHandleEx_L(int scriptType, UID objectHandle, const char* scriptName)
{
    return (simGetScriptHandleEx_internal(scriptType, objectHandle, scriptName));
}

SIM_DLLEXPORT int simRemoveObjects_L(const UID* objectHandles, int count)
{
    return (simRemoveObjects_internal(objectHandles, count));
}

SIM_DLLEXPORT int simRemoveModel_L(UID objectHandle)
{
    return (simRemoveModel_internal(objectHandle));
}

SIM_DLLEXPORT char* simGetObjectAlias_L(UID objectHandle, int options)
{
    return (simGetObjectAlias_internal(objectHandle, options));
}

SIM_DLLEXPORT int simSetObjectAlias_L(UID objectHandle, const char* objectAlias, int options)
{
    return (simSetObjectAlias_internal(objectHandle, objectAlias, options));
}

SIM_DLLEXPORT UID simGetObjectParent_L(UID objectHandle)
{
    return (simGetObjectParent_internal(objectHandle));
}

SIM_DLLEXPORT UID simGetObjectChild_L(UID objectHandle, int index)
{
    return (simGetObjectChild_internal(objectHandle, index));
}

SIM_DLLEXPORT int simSetObjectParent_L(UID objectHandle, UID parentObjectHandle, bool keepInPlace)
{
    return (simSetObjectParent_internal(objectHandle, parentObjectHandle, keepInPlace));
}

SIM_DLLEXPORT int simGetObjectType_L(UID objectHandle)
{
    return (simGetObjectType_internal(objectHandle));
}

SIM_DLLEXPORT int simGetJointType_L(UID objectHandle)
{
    return (simGetJointType_internal(objectHandle));
}

SIM_DLLEXPORT UID simLoadModel_L(const char* filename)
{
    return (simLoadModel_internal(filename));
}

SIM_DLLEXPORT int simSaveModel_L(UID baseOfModelHandle, const char* filename)
{
    return (simSaveModel_internal(baseOfModelHandle, filename));
}

SIM_DLLEXPORT UID* simGetObjectSel_L(int* cnt)
{
    return (simGetObjectSel_internal(cnt));
}

SIM_DLLEXPORT int simSetObjectSel_L(const UID* handles, int cnt)
{
    return (simSetObjectSel_internal(handles, cnt));
}

SIM_DLLEXPORT int simAssociateScriptWithObject_L(UID scriptHandle, UID associatedObjectHandle)
{
    return (simAssociateScriptWithObject_internal(scriptHandle, associatedObjectHandle));
}

SIM_DLLEXPORT int simResetScript_L(UID scriptHandle)
{
    return (simResetScript_internal(scriptHandle));
}

SIM_DLLEXPORT UID simAddScript_L(int scriptProperty)
{
    return (simAddScript_internal(scriptProperty));
}

SIM_DLLEXPORT int simRemoveScript_L(UID scriptHandle)
{
    return (simRemoveScript_internal(scriptHandle));
}

SIM_DLLEXPORT int simResetProximitySensor_L(UID sensorHandle)
{
    return (simResetProximitySensor_internal(sensorHandle));
}

SIM_DLLEXPORT int simCheckCollision_L(UID entity1Handle, UID entity2Handle)
{
    return (simCheckCollision_internal(entity1Handle, entity2Handle));
}

SIM_DLLEXPORT int simAdjustView_L(int viewHandleOrIndex, UID associatedViewableObjectHandle, int options, const char* viewLabel)
{
    return (simAdjustView_internal(viewHandleOrIndex, associatedViewableObjectHandle, options, viewLabel));
}

SIM_DLLEXPORT int simResetGraph_L(UID graphHandle)
{
    return (simResetGraph_internal(graphHandle));
}

SIM_DLLEXPORT int simDestroyGraphCurve_L(UID graphHandle, UID curveId)
{
    return (simDestroyGraphCurve_internal(graphHandle, curveId));
}

SIM_DLLEXPORT UID simDuplicateGraphCurveToStatic_L(UID graphHandle, UID curveId, const char* curveName)
{
    return (simDuplicateGraphCurveToStatic_internal(graphHandle, curveId, curveName));
}

SIM_DLLEXPORT int simRegisterScriptVariable_L(const char* var, const char* val, UID stackHandle)
{
    return (simRegisterScriptVariable_internal(var, val, stackHandle));
}

SIM_DLLEXPORT int simRegisterScriptFuncHook_L(UID scriptHandle, const char* funcToHook, const char* userFunction, bool executeBefore, int options)
{
    return (simRegisterScriptFuncHook_internal(scriptHandle, funcToHook, userFunction, executeBefore, options));
}

SIM_DLLEXPORT int simCopyPasteObjects_L(UID* objectHandles, int objectCount, int options)
{
    return (simCopyPasteObjects_internal(objectHandles, objectCount, options));
}

SIM_DLLEXPORT char* simReceiveData_L(int dataHeader, const char* dataName, UID antennaHandle, int index, int* dataLength, UID* senderID, int* dataHeaderR, char** dataNameR)
{
    return (simReceiveData_internal(dataHeader, dataName, antennaHandle, index, dataLength, senderID, dataHeaderR, dataNameR));
}

SIM_DLLEXPORT int simRemoveDrawingObject_L(UID objectHandle)
{
    return (simRemoveDrawingObject_internal(objectHandle));
}

SIM_DLLEXPORT int simSetExplicitHandling_L(UID objectHandle, int explicitFlags)
{
    return (simSetExplicitHandling_internal(objectHandle, explicitFlags));
}

SIM_DLLEXPORT int simGetExplicitHandling_L(UID objectHandle)
{
    return (simGetExplicitHandling_internal(objectHandle));
}

SIM_DLLEXPORT UID simGetLinkDummy_L(UID dummyHandle)
{
    return (simGetLinkDummy_internal(dummyHandle));
}

SIM_DLLEXPORT int simSetLinkDummy_L(UID dummyHandle, UID linkedDummyHandle)
{
    return (simSetLinkDummy_internal(dummyHandle, linkedDummyHandle));
}

SIM_DLLEXPORT int simResetDynamicObject_L(UID objectHandle)
{
    return (simResetDynamicObject_internal(objectHandle));
}

SIM_DLLEXPORT int simSetJointMode_L(UID jointHandle, int jointMode, int options)
{
    return (simSetJointMode_internal(jointHandle, jointMode, options));
}

SIM_DLLEXPORT int simGetJointMode_L(UID jointHandle, int* options)
{
    return (simGetJointMode_internal(jointHandle, options));
}

SIM_DLLEXPORT int simIsHandle_L(UID generalObjectHandle, int generalObjectType)
{
    return (simIsHandle_internal(generalObjectHandle, generalObjectType));
}

SIM_DLLEXPORT int simResetVisionSensor_L(UID visionSensorHandle)
{
    return (simResetVisionSensor_internal(visionSensorHandle));
}

SIM_DLLEXPORT int simSetVisionSensorImg_L(UID sensorHandle, const unsigned char* img, int options, const int* pos, const int* size)
{
    return (simSetVisionSensorImg_internal(sensorHandle, img, options, pos, size));
}

SIM_DLLEXPORT UID simGroupShapes_L(const UID* shapeHandles, int shapeCount)
{
    return (simGroupShapes_internal(shapeHandles, shapeCount));
}

SIM_DLLEXPORT UID* simUngroupShape_L(UID shapeHandle, int* shapeCount)
{
    return (simUngroupShape_internal(shapeHandle, shapeCount));
}

SIM_DLLEXPORT int simSetShapeMaterial_L(UID shapeHandle, UID materialIdOrShapeHandle)
{
    return (simSetShapeMaterial_internal(shapeHandle, materialIdOrShapeHandle));
}

SIM_DLLEXPORT UID simGetTextureId_L(const char* textureName, int* resolution)
{
    return (simGetTextureId_internal(textureName, resolution));
}

SIM_DLLEXPORT unsigned char* simReadTexture_L(UID textureId, int options, int posX, int posY, int sizeX, int sizeY)
{
    return (simReadTexture_internal(textureId, options, posX, posY, sizeX, sizeY));
}

SIM_DLLEXPORT UID simGetObjects_L(int index, int objectType)
{
    return (simGetObjects_internal(index, objectType));
}

SIM_DLLEXPORT UID* simGetObjectsInTree_L(UID treeBaseHandle, int objectType, int options, int* objectCount)
{
    return (simGetObjectsInTree_internal(treeBaseHandle, objectType, options, objectCount));
}

SIM_DLLEXPORT UID simGetShapeTextureId_L(UID shapeHandle)
{
    return (simGetShapeTextureId_internal(shapeHandle));
}

SIM_DLLEXPORT UID simCreateCollectionEx_L(int options)
{
    return (simCreateCollectionEx_internal(options));
}

SIM_DLLEXPORT int simAddItemToCollection_L(UID collectionHandle, int what, UID objectHandle, int options)
{
    return (simAddItemToCollection_internal(collectionHandle, what, objectHandle, options));
}

SIM_DLLEXPORT int simDestroyCollection_L(UID collectionHandle)
{
    return (simDestroyCollection_internal(collectionHandle));
}

SIM_DLLEXPORT UID* simGetCollectionObjects_L(UID collectionHandle, int* objectCount)
{
    return (simGetCollectionObjects_internal(collectionHandle, objectCount));
}

SIM_DLLEXPORT int simAlignShapeBB_L(UID shapeHandle, const double* pose)
{
    return (simAlignShapeBB_internal(shapeHandle, pose));
}

SIM_DLLEXPORT int simRelocateShapeFrame_L(UID shapeHandle, const double* pose)
{
    return (simRelocateShapeFrame_internal(shapeHandle, pose));
}

SIM_DLLEXPORT int simCallScriptFunctionEx_L(UID scriptHandleOrType, const char* functionNameAtScriptName, UID stackId)
{
    return (simCallScriptFunctionEx_internal(scriptHandleOrType, functionNameAtScriptName, stackId));
}

SIM_DLLEXPORT char* simGetExtensionString_L(UID objectHandle, int index, const char* key)
{
    return (simGetExtensionString_internal(objectHandle, index, key));
}

SIM_DLLEXPORT UID simCreateStack_L()
{
    return (simCreateStack_internal());
}

SIM_DLLEXPORT int simReleaseStack_L(UID stackHandle)
{
    return (simReleaseStack_internal(stackHandle));
}

SIM_DLLEXPORT UID simCopyStack_L(UID stackHandle)
{
    return (simCopyStack_internal(stackHandle));
}

SIM_DLLEXPORT int simPushNullOntoStack_L(UID stackHandle)
{
    return (simPushNullOntoStack_internal(stackHandle));
}

SIM_DLLEXPORT int simPushBoolOntoStack_L(UID stackHandle, bool value)
{
    return (simPushBoolOntoStack_internal(stackHandle, value));
}

SIM_DLLEXPORT int simPushInt32OntoStack_L(UID stackHandle, int value)
{
    return (simPushInt32OntoStack_internal(stackHandle, value));
}

SIM_DLLEXPORT int simPushInt64OntoStack_L(UID stackHandle, long long int value)
{
    return (simPushInt64OntoStack_internal(stackHandle, value));
}

SIM_DLLEXPORT int simPushFloatOntoStack_L(UID stackHandle, float value)
{
    return (simPushFloatOntoStack_internal(stackHandle, value));
}

SIM_DLLEXPORT int simPushDoubleOntoStack_L(UID stackHandle, double value)
{
    return (simPushDoubleOntoStack_internal(stackHandle, value));
}

SIM_DLLEXPORT int simPushTextOntoStack_L(UID stackHandle, const char* value)
{
    return (simPushTextOntoStack_internal(stackHandle, value));
}

SIM_DLLEXPORT int simPushStringOntoStack_L(UID stackHandle, const char* value, int stringSize)
{
    return (simPushStringOntoStack_internal(stackHandle, value, stringSize));
}

SIM_DLLEXPORT int simPushBufferOntoStack_L(UID stackHandle, const char* value, int stringSize)
{
    return (simPushBufferOntoStack_internal(stackHandle, value, stringSize));
}

SIM_DLLEXPORT int simPushUInt8TableOntoStack_L(UID stackHandle, const unsigned char* values, int valueCnt)
{
    return (simPushUInt8TableOntoStack_internal(stackHandle, values, valueCnt));
}

SIM_DLLEXPORT int simPushInt32TableOntoStack_L(UID stackHandle, const int* values, int valueCnt)
{
    return (simPushInt32TableOntoStack_internal(stackHandle, values, valueCnt));
}

SIM_DLLEXPORT int simPushInt64TableOntoStack_L(UID stackHandle, const long long int* values, int valueCnt)
{
    return (simPushInt64TableOntoStack_internal(stackHandle, values, valueCnt));
}

SIM_DLLEXPORT int simPushFloatTableOntoStack_L(UID stackHandle, const float* values, int valueCnt)
{
    return (simPushFloatTableOntoStack_internal(stackHandle, values, valueCnt));
}

SIM_DLLEXPORT int simPushDoubleTableOntoStack_L(UID stackHandle, const double* values, int valueCnt)
{
    return (simPushDoubleTableOntoStack_internal(stackHandle, values, valueCnt));
}

SIM_DLLEXPORT int simPushTableOntoStack_L(UID stackHandle)
{
    return (simPushTableOntoStack_internal(stackHandle));
}

SIM_DLLEXPORT int simInsertDataIntoStackTable_L(UID stackHandle)
{
    return (simInsertDataIntoStackTable_internal(stackHandle));
}

SIM_DLLEXPORT int simGetStackSize_L(UID stackHandle)
{
    return (simGetStackSize_internal(stackHandle));
}

SIM_DLLEXPORT int simPopStackItem_L(UID stackHandle, int count)
{
    return (simPopStackItem_internal(stackHandle, count));
}

SIM_DLLEXPORT int simMoveStackItemToTop_L(UID stackHandle, int cIndex)
{
    return (simMoveStackItemToTop_internal(stackHandle, cIndex));
}

SIM_DLLEXPORT int simGetStackItemType_L(UID stackHandle, int cIndex)
{
    return (simGetStackItemType_internal(stackHandle, cIndex));
}

SIM_DLLEXPORT int simGetStackStringType_L(UID stackHandle, int cIndex)
{
    return (simGetStackStringType_internal(stackHandle, cIndex));
}

SIM_DLLEXPORT int simGetStackBoolValue_L(UID stackHandle, bool* boolValue)
{
    return (simGetStackBoolValue_internal(stackHandle, boolValue));
}

SIM_DLLEXPORT int simGetStackInt32Value_L(UID stackHandle, int* numberValue)
{
    return (simGetStackInt32Value_internal(stackHandle, numberValue));
}

SIM_DLLEXPORT int simGetStackInt64Value_L(UID stackHandle, long long int* numberValue)
{
    return (simGetStackInt64Value_internal(stackHandle, numberValue));
}

SIM_DLLEXPORT int simGetStackFloatValue_L(UID stackHandle, float* numberValue)
{
    return (simGetStackFloatValue_internal(stackHandle, numberValue));
}

SIM_DLLEXPORT int simGetStackDoubleValue_L(UID stackHandle, double* numberValue)
{
    return (simGetStackDoubleValue_internal(stackHandle, numberValue));
}

SIM_DLLEXPORT char* simGetStackStringValue_L(UID stackHandle, int* stringSize)
{
    return (simGetStackStringValue_internal(stackHandle, stringSize));
}

SIM_DLLEXPORT int simGetStackTableInfo_L(UID stackHandle, int infoType)
{
    return (simGetStackTableInfo_internal(stackHandle, infoType));
}

SIM_DLLEXPORT int simGetStackUInt8Table_L(UID stackHandle, unsigned char* array, int count)
{
    return (simGetStackUInt8Table_internal(stackHandle, array, count));
}

SIM_DLLEXPORT int simGetStackInt32Table_L(UID stackHandle, int* array, int count)
{
    return (simGetStackInt32Table_internal(stackHandle, array, count));
}

SIM_DLLEXPORT int simGetStackInt64Table_L(UID stackHandle, long long int* array, int count)
{
    return (simGetStackInt64Table_internal(stackHandle, array, count));
}

SIM_DLLEXPORT int simGetStackFloatTable_L(UID stackHandle, float* array, int count)
{
    return (simGetStackFloatTable_internal(stackHandle, array, count));
}

SIM_DLLEXPORT int simGetStackDoubleTable_L(UID stackHandle, double* array, int count)
{
    return (simGetStackDoubleTable_internal(stackHandle, array, count));
}

SIM_DLLEXPORT int simUnfoldStackTable_L(UID stackHandle)
{
    return (simUnfoldStackTable_internal(stackHandle));
}

SIM_DLLEXPORT int simDebugStack_L(UID stackHandle, int cIndex)
{
    return (simDebugStack_internal(stackHandle, cIndex));
}

SIM_DLLEXPORT int simInsertObjectIntoOctree_L(UID octreeHandle, UID objectHandle, int options, const unsigned char* color, unsigned int tag, void* reserved)
{
    return (simInsertObjectIntoOctree_internal(octreeHandle, objectHandle, options, color, tag, reserved));
}

SIM_DLLEXPORT int simSubtractObjectFromOctree_L(UID octreeHandle, UID objectHandle, int options, void* reserved)
{
    return (simSubtractObjectFromOctree_internal(octreeHandle, objectHandle, options, reserved));
}

SIM_DLLEXPORT char* simPackTable_L(UID stackHandle, int* bufferSize)
{
    return (simPackTable_internal(stackHandle, bufferSize));
}

SIM_DLLEXPORT int simUnpackTable_L(UID stackHandle, const char* buffer, int bufferSize)
{
    return (simUnpackTable_internal(stackHandle, buffer, bufferSize));
}

SIM_DLLEXPORT int simSetReferencedHandles_L(UID objectHandle, int count, const UID* referencedHandles, const char* tag, const int* reserved2)
{
    return (simSetReferencedHandles_internal(objectHandle, count, referencedHandles, tag, reserved2));
}

SIM_DLLEXPORT int simGetReferencedHandles_L(UID objectHandle, UID** referencedHandles, const char* tag, UID** reserved2)
{
    return (simGetReferencedHandles_internal(objectHandle, referencedHandles, tag, reserved2));
}

SIM_DLLEXPORT int simExecuteScriptString_L(UID scriptHandle, const char* stringToExecute, UID stackHandle)
{
    return (simExecuteScriptString_internal(scriptHandle, stringToExecute, stackHandle));
}

SIM_DLLEXPORT char* simGetApiFunc_L(UID scriptHandle, const char* apiWord)
{
    return (simGetApiFunc_internal(scriptHandle, apiWord));
}

SIM_DLLEXPORT char* simGetApiInfo_L(UID scriptHandle, const char* apiWord)
{
    return (simGetApiInfo_internal(scriptHandle, apiWord));
}

SIM_DLLEXPORT int simIsDynamicallyEnabled_L(UID objectHandle)
{
    return (simIsDynamicallyEnabled_internal(objectHandle));
}

SIM_DLLEXPORT int simInitScript_L(UID scriptHandle)
{
    return (simInitScript_internal(scriptHandle));
}

SIM_DLLEXPORT int simGetVisionSensorRes_L(UID sensorHandle, int* resolution)
{
    return (simGetVisionSensorRes_internal(sensorHandle, resolution));
}

SIM_DLLEXPORT UID simCreateShape_L(int options, double shadingAngle, const double* vertices, int verticesSize, const int* indices, int indicesSize, const double* normals, const float* textureCoords, const unsigned char* texture, const int* textureRes)
{
    return (simCreateShape_internal(options, shadingAngle, vertices, verticesSize, indices, indicesSize, normals, textureCoords, texture, textureRes));
}

SIM_DLLEXPORT UID _simGetObjectID_L(const void* object)
{
    return (_simGetObjectID_internal(object));
}

SIM_DLLEXPORT const void* _simGetObject_L(UID objID)
{
    return (_simGetObject_internal(objID));
}

SIM_DLLEXPORT int _simGetDummyLinkType_L(const void* dummy, UID* linkedDummyID)
{
    return (_simGetDummyLinkType_internal(dummy, linkedDummyID));
}

SIM_DLLEXPORT bool _simDoEntitiesCollide_L(UID entity1ID, UID entity2ID, UID* cacheBuffer, bool overrideCollidableFlagIfShape1, bool overrideCollidableFlagIfShape2, bool pathOrMotionPlanningRoutineCalling)
{
    return (_simDoEntitiesCollide_internal(entity1ID, entity2ID, cacheBuffer, overrideCollidableFlagIfShape1, overrideCollidableFlagIfShape2, pathOrMotionPlanningRoutineCalling));
}

SIM_DLLEXPORT float* simGetVisionSensorDepth_L(UID sensorHandle, int options, const int* pos, const int* size, int* resolution)
{
    return (simGetVisionSensorDepth_internal(sensorHandle, options, pos, size, resolution));
}

SIM_DLLEXPORT int _simSetVisionSensorDepth_L(UID sensorHandle, int options, const float* depth)
{
    return (_simSetVisionSensorDepth_internal(sensorHandle, options, depth));
}

SIM_DLLEXPORT float* simCheckVisionSensorEx_L(UID visionSensorHandle, UID entityHandle, bool returnImage)
{
    return (simCheckVisionSensorEx_internal(visionSensorHandle, entityHandle, returnImage));
}

SIM_DLLEXPORT UID simCreateScript_L(int scriptType, const char* scriptText, int options, const char* lang)
{
    return simCreateScript_internal(scriptType, scriptText, options, lang);
}

SIM_DLLEXPORT int simGetObjectHierarchyOrder_L(UID objectHandle, int* totalSiblings)
{
    return simGetObjectHierarchyOrder_internal(objectHandle, totalSiblings);
}

SIM_DLLEXPORT int simSetObjectHierarchyOrder_L(UID objectHandle, int order)
{
    return simSetObjectHierarchyOrder_internal(objectHandle, order);
}

SIM_DLLEXPORT int simGetShapeViz_D_L(UID shapeHandle, int index, struct SShapeVizInfo* info)
{
    return (simGetShapeViz_internal(shapeHandle, index, info));
}

SIM_DLLEXPORT int simGetObjectMatrix_D_L(UID objectHandle, UID relativeToObjectHandle, double* matrix)
{
    return (simGetObjectMatrix_internal(objectHandle, relativeToObjectHandle, matrix));
}

SIM_DLLEXPORT int simSetObjectMatrix_D_L(UID objectHandle, UID relativeToObjectHandle, const double* matrix)
{
    return (simSetObjectMatrix_internal(objectHandle, relativeToObjectHandle, matrix));
}

SIM_DLLEXPORT int simGetObjectPose_D_L(UID objectHandle, UID relativeToObjectHandle, double* pose)
{
    return (simGetObjectPose_internal(objectHandle, relativeToObjectHandle, pose));
}

SIM_DLLEXPORT int simSetObjectPose_D_L(UID objectHandle, UID relativeToObjectHandle, const double* pose)
{
    return (simSetObjectPose_internal(objectHandle, relativeToObjectHandle, pose));
}

SIM_DLLEXPORT int simGetObjectPosition_D_L(UID objectHandle, UID relativeToObjectHandle, double* position)
{
    return (simGetObjectPosition_internal(objectHandle, relativeToObjectHandle, position));
}

SIM_DLLEXPORT int simSetObjectPosition_D_L(UID objectHandle, UID relativeToObjectHandle, const double* position)
{
    return (simSetObjectPosition_internal(objectHandle, relativeToObjectHandle, position));
}

SIM_DLLEXPORT int simGetObjectOrientation_D_L(UID objectHandle, UID relativeToObjectHandle, double* eulerAngles)
{
    return (simGetObjectOrientation_internal(objectHandle, relativeToObjectHandle, eulerAngles));
}

SIM_DLLEXPORT int simGetObjectQuaternion_D_L(UID objectHandle, UID relativeToObjectHandle, double* quaternion)
{
    return (simGetObjectQuaternion_internal(objectHandle, relativeToObjectHandle, quaternion));
}

SIM_DLLEXPORT int simSetObjectQuaternion_D_L(UID objectHandle, UID relativeToObjectHandle, const double* quaternion)
{
    return (simSetObjectQuaternion_internal(objectHandle, relativeToObjectHandle, quaternion));
}

SIM_DLLEXPORT int simSetObjectOrientation_D_L(UID objectHandle, UID relativeToObjectHandle, const double* eulerAngles)
{
    return (simSetObjectOrientation_internal(objectHandle, relativeToObjectHandle, eulerAngles));
}

SIM_DLLEXPORT int simGetJointPosition_D_L(UID objectHandle, double* position)
{
    return (simGetJointPosition_internal(objectHandle, position));
}

SIM_DLLEXPORT int simSetJointPosition_D_L(UID objectHandle, double position)
{
    return (simSetJointPosition_internal(objectHandle, position));
}

SIM_DLLEXPORT int simSetJointTargetPosition_D_L(UID objectHandle, double targetPosition)
{
    return (simSetJointTargetPosition_internal(objectHandle, targetPosition));
}

SIM_DLLEXPORT int simGetJointTargetPosition_D_L(UID objectHandle, double* targetPosition)
{
    return (simGetJointTargetPosition_internal(objectHandle, targetPosition));
}

SIM_DLLEXPORT int simGetObjectChildPose_D_L(UID objectHandle, double* pose)
{
    return (simGetObjectChildPose_internal(objectHandle, pose));
}

SIM_DLLEXPORT int simSetObjectChildPose_D_L(UID objectHandle, const double* pose)
{
    return (simSetObjectChildPose_internal(objectHandle, pose));
}

SIM_DLLEXPORT int simGetJointInterval_D_L(UID objectHandle, bool* cyclic, double* interval)
{
    return (simGetJointInterval_internal(objectHandle, cyclic, interval));
}

SIM_DLLEXPORT int simSetJointInterval_D_L(UID objectHandle, bool cyclic, const double* interval)
{
    return (simSetJointInterval_internal(objectHandle, cyclic, interval));
}

SIM_DLLEXPORT int simHandleProximitySensor_D_L(UID sensorHandle, double* detectedPoint, UID* detectedObjectHandle, double* normalVector)
{
    return (simHandleProximitySensor_internal(sensorHandle, detectedPoint, detectedObjectHandle, normalVector));
}

SIM_DLLEXPORT int simReadProximitySensor_D_L(UID sensorHandle, double* detectedPoint, UID* detectedObjectHandle, double* normalVector)
{
    return (simReadProximitySensor_internal(sensorHandle, detectedPoint, detectedObjectHandle, normalVector));
}

SIM_DLLEXPORT int simCheckProximitySensor_D_L(UID sensorHandle, UID entityHandle, double* detectedPoint)
{
    return (simCheckProximitySensor_internal(sensorHandle, entityHandle, detectedPoint));
}

SIM_DLLEXPORT int simCheckProximitySensorEx_D_L(UID sensorHandle, UID entityHandle, int detectionMode, double detectionThreshold, double maxAngle, double* detectedPoint, UID* detectedObjectHandle, double* normalVector)
{
    return (simCheckProximitySensorEx_internal(sensorHandle, entityHandle, detectionMode, detectionThreshold, maxAngle, detectedPoint, detectedObjectHandle, normalVector));
}

SIM_DLLEXPORT int simCheckProximitySensorEx2_D_L(UID sensorHandle, double* vertexPointer, int itemType, int itemCount, int detectionMode, double detectionThreshold, double maxAngle, double* detectedPoint, double* normalVector)
{
    return (simCheckProximitySensorEx2_internal(sensorHandle, vertexPointer, itemType, itemCount, detectionMode, detectionThreshold, maxAngle, detectedPoint, normalVector));
}

SIM_DLLEXPORT int simCheckCollisionEx_D_L(UID entity1Handle, UID entity2Handle, double** intersectionSegments)
{
    return (simCheckCollisionEx_internal(entity1Handle, entity2Handle, intersectionSegments));
}

SIM_DLLEXPORT int simCheckDistance_D_L(UID entity1Handle, UID entity2Handle, double threshold, double* distanceData)
{
    return (simCheckDistance_internal(entity1Handle, entity2Handle, threshold, distanceData));
}

SIM_DLLEXPORT int simHandleGraph_D_L(UID graphHandle, double simulationTime)
{
    return (simHandleGraph_internal(graphHandle, simulationTime));
}

SIM_DLLEXPORT int simAddGraphStream_D_L(UID graphHandle, const char* streamName, const char* unitStr, int options, const float* color, double cyclicRange)
{
    return (simAddGraphStream_internal(graphHandle, streamName, unitStr, options, color, cyclicRange));
}

SIM_DLLEXPORT int simSetGraphStreamTransformation_D_L(UID graphHandle, int streamId, int trType, double mult, double off, int movingAvgPeriod)
{
    return (simSetGraphStreamTransformation_internal(graphHandle, streamId, trType, mult, off, movingAvgPeriod));
}

SIM_DLLEXPORT int simAddGraphCurve_D_L(UID graphHandle, const char* curveName, int dim, const int* streamIds, const double* defaultValues, const char* unitStr, int options, const float* color, int curveWidth)
{
    return (simAddGraphCurve_internal(graphHandle, curveName, dim, streamIds, defaultValues, unitStr, options, color, curveWidth));
}

SIM_DLLEXPORT int simSetGraphStreamValue_D_L(UID graphHandle, int streamId, double value)
{
    return (simSetGraphStreamValue_internal(graphHandle, streamId, value));
}

SIM_DLLEXPORT int simSetJointTargetVelocity_D_L(UID objectHandle, double targetVelocity)
{
    return (simSetJointTargetVelocity_internal(objectHandle, targetVelocity));
}

SIM_DLLEXPORT int simGetJointTargetVelocity_D_L(UID objectHandle, double* targetVelocity)
{
    return (simGetJointTargetVelocity_internal(objectHandle, targetVelocity));
}

SIM_DLLEXPORT int simScaleObjects_D_L(const UID* objectHandles, int objectCount, double scalingFactor, bool scalePositionsToo)
{
    return (simScaleObjects_internal(objectHandles, objectCount, scalingFactor, scalePositionsToo));
}

SIM_DLLEXPORT UID simAddDrawingObject_D_L(int objectType, double size, double duplicateTolerance, int parentObjectHandle, int maxItemCount, const float* color, const float* setToNULL, const float* setToNULL2, const float* setToNULL3)
{
    return (simAddDrawingObject_internal(objectType, size, duplicateTolerance, parentObjectHandle, maxItemCount, color, setToNULL, setToNULL2, setToNULL3));
}

SIM_DLLEXPORT int simAddDrawingObjectItem_D_L(UID objectHandle, const double* itemData)
{
    return (simAddDrawingObjectItem_internal(objectHandle, itemData));
}

SIM_DLLEXPORT double simGetObjectSizeFactor_D_L(UID objectHandle)
{
    return (simGetObjectSizeFactor_internal(objectHandle));
}

SIM_DLLEXPORT int simReadForceSensor_D_L(UID objectHandle, double* forceVector, double* torqueVector)
{
    return (simReadForceSensor_internal(objectHandle, forceVector, torqueVector));
}

SIM_DLLEXPORT int simGetVelocity_D_L(UID shapeHandle, double* linearVelocity, double* angularVelocity)
{
    return (simGetVelocity_internal(shapeHandle, linearVelocity, angularVelocity));
}

SIM_DLLEXPORT int simGetObjectVelocity_D_L(UID objectHandle, double* linearVelocity, double* angularVelocity)
{
    return (simGetObjectVelocity_internal(objectHandle, linearVelocity, angularVelocity));
}

SIM_DLLEXPORT int simGetJointVelocity_D_L(UID jointHandle, double* velocity)
{
    return (simGetJointVelocity_internal(jointHandle, velocity));
}

SIM_DLLEXPORT int simAddForceAndTorque_D_L(UID shapeHandle, const double* force, const double* torque)
{
    return (simAddForceAndTorque_internal(shapeHandle, force, torque));
}

SIM_DLLEXPORT int simAddForce_D_L(UID shapeHandle, const double* position, const double* force)
{
    return (simAddForce_internal(shapeHandle, position, force));
}

SIM_DLLEXPORT int simSetObjectColor_L(UID objectHandle, int index, int colorComponent, const float* rgbData)
{
    return (simSetObjectColor_internal(objectHandle, index, colorComponent, rgbData));
}

SIM_DLLEXPORT int simGetObjectColor_L(UID objectHandle, int index, int colorComponent, float* rgbData)
{
    return (simGetObjectColor_internal(objectHandle, index, colorComponent, rgbData));
}

SIM_DLLEXPORT int simSetShapeColor_L(UID shapeHandle, const char* colorName, int colorComponent, const float* rgbData)
{
    return (simSetShapeColor_internal(shapeHandle, colorName, colorComponent, rgbData));
}

SIM_DLLEXPORT int simGetShapeColor_L(UID shapeHandle, const char* colorName, int colorComponent, float* rgbData)
{
    return (simGetShapeColor_internal(shapeHandle, colorName, colorComponent, rgbData));
}

SIM_DLLEXPORT int simGetContactInfo_D_L(int dynamicPass, UID objectHandle, int index, UID* objectHandles, double* contactInfo)
{
    return (simGetContactInfo_internal(dynamicPass, objectHandle, index, objectHandles, contactInfo));
}

SIM_DLLEXPORT UID simImportShape_D_L(int fileformat, const char* pathAndFilename, int options, double identicalVerticeTolerance, double scalingFactor)
{
    return (simImportShape_internal(fileformat, pathAndFilename, options, identicalVerticeTolerance, scalingFactor));
}

SIM_DLLEXPORT UID simCreateMeshShape_D_L(int options, double shadingAngle, const double* vertices, int verticesSize, const int* indices, int indicesSize, double* reserved)
{
    return (simCreateMeshShape_internal(options, shadingAngle, vertices, verticesSize, indices, indicesSize, reserved));
}

SIM_DLLEXPORT UID simCreatePrimitiveShape_D_L(int primitiveType, const double* sizes, int options)
{
    return (simCreatePrimitiveShape_internal(primitiveType, sizes, options));
}

SIM_DLLEXPORT UID simCreateHeightfieldShape_D_L(int options, double shadingAngle, int xPointCount, int yPointCount, double xSize, const double* heights)
{
    return (simCreateHeightfieldShape_internal(options, shadingAngle, xPointCount, yPointCount, xSize, heights));
}

SIM_DLLEXPORT int simGetShapeMesh_D_L(int UID, double** vertices, int* verticesSize, int** indices, int* indicesSize, double** normals)
{
    return (simGetShapeMesh_internal(shapeHandle, vertices, verticesSize, indices, indicesSize, normals));
}

SIM_DLLEXPORT UID simCreateJoint_D_L(int jointType, int jointMode, int options, const double* sizes, const double* reservedA, const double* reservedB)
{
    return (simCreateJoint_internal(jointType, jointMode, options, sizes, reservedA, reservedB));
}

SIM_DLLEXPORT int simGetJointForce_D_L(UID jointHandle, double* forceOrTorque)
{
    return (simGetJointForce_internal(jointHandle, forceOrTorque));
}

SIM_DLLEXPORT int simGetJointTargetForce_D_L(UID jointHandle, double* forceOrTorque)
{
    return (simGetJointTargetForce_internal(jointHandle, forceOrTorque));
}

SIM_DLLEXPORT int simSetJointTargetForce_D_L(UID objectHandle, double forceOrTorque, bool signedValue)
{
    return (simSetJointTargetForce_internal(objectHandle, forceOrTorque, signedValue));
}

SIM_DLLEXPORT int simCameraFitToView_D_L(int viewHandleOrIndex, int objectCount, const UID* objectHandles, int options, double scaling)
{
    return (simCameraFitToView_internal(viewHandleOrIndex, objectCount, objectHandles, options, scaling));
}

SIM_DLLEXPORT int simHandleVisionSensor_D_L(UID visionSensorHandle, double** auxValues, int** auxValuesCount)
{
    return (simHandleVisionSensor_internal(visionSensorHandle, auxValues, auxValuesCount));
}

SIM_DLLEXPORT int simReadVisionSensor_D_L(UID visionSensorHandle, double** auxValues, int** auxValuesCount)
{
    return (simReadVisionSensor_internal(visionSensorHandle, auxValues, auxValuesCount));
}

SIM_DLLEXPORT int simCheckVisionSensor_D_L(UID visionSensorHandle, UID entityHandle, double** auxValues, int** auxValuesCount)
{
    return (simCheckVisionSensor_internal(visionSensorHandle, entityHandle, auxValues, auxValuesCount));
}

SIM_DLLEXPORT unsigned char* simGetVisionSensorImg_D_L(UID sensorHandle, int options, double rgbaCutOff, const int* pos, const int* size, int* resolution)
{
    return (simGetVisionSensorImg_internal(sensorHandle, options, rgbaCutOff, pos, size, resolution));
}

SIM_DLLEXPORT UID simCreateDummy_D_L(double size, const float* reserved)
{
    return (simCreateDummy_internal(size, reserved));
}

SIM_DLLEXPORT UID simCreateForceSensor_D_L(int options, const int* intParams, const double* floatParams, const double* reserved)
{
    return (simCreateForceSensor_internal(options, intParams, floatParams, reserved));
}

SIM_DLLEXPORT UID simCreateProximitySensor_D_L(int sensorType, int subType, int options, const int* intParams, const double* floatParams, const double* reserved)
{
    return (simCreateProximitySensor_internal(sensorType, subType, options, intParams, floatParams, reserved));
}

SIM_DLLEXPORT UID simCreateVisionSensor_D_L(int options, const int* intParams, const double* floatParams, const double* reserved)
{
    return (simCreateVisionSensor_internal(options, intParams, floatParams, reserved));
}

SIM_DLLEXPORT int simConvexDecompose_D_L(UID shapeHandle, int options, const int* intParams, const double* floatParams)
{
    return (simConvexDecompose_internal(shapeHandle, options, intParams, floatParams));
}

SIM_DLLEXPORT UID simCreateTexture_D_L(const char* fileName, int options, const double* planeSizes, const double* scalingUV, const double* xy_g, int fixedResolution, int* textureId, int* resolution, const void* reserved)
{
    return (simCreateTexture_internal(fileName, options, planeSizes, scalingUV, xy_g, fixedResolution, textureId, resolution, reserved));
}

SIM_DLLEXPORT int simWriteTexture_D_L(UID textureId, int options, const char* data, int posX, int posY, int sizeX, int sizeY, double interpol)
{
    return (simWriteTexture_internal(textureId, options, data, posX, posY, sizeX, sizeY, interpol));
}

SIM_DLLEXPORT int simGetShapeGeomInfo_D_L(UID shapeHandle, int* intData, double* floatData, void* reserved)
{
    return (simGetShapeGeomInfo_internal(shapeHandle, intData, floatData, reserved));
}

SIM_DLLEXPORT int simScaleObject_D_L(UID objectHandle, double xScale, double yScale, double zScale, int options)
{
    return (simScaleObject_internal(objectHandle, xScale, yScale, zScale, options));
}

SIM_DLLEXPORT int simSetShapeTexture_D_L(UID shapeHandle, UID textureId, int mappingMode, int options, const double* uvScaling, const double* position, const double* orientation)
{
    return (simSetShapeTexture_internal(shapeHandle, textureId, mappingMode, options, uvScaling, position, orientation));
}

SIM_DLLEXPORT int simComputeMassAndInertia_D_L(UID shapeHandle, double density)
{
    return (simComputeMassAndInertia_internal(shapeHandle, density));
}

SIM_DLLEXPORT UID simCreateOctree_D_L(double voxelSize, int options, double pointSize, void* reserved)
{
    return (simCreateOctree_internal(voxelSize, options, pointSize, reserved));
}

SIM_DLLEXPORT UID simCreatePointCloud_D_L(double maxVoxelSize, int maxPtCntPerVoxel, int options, double pointSize, void* reserved)
{
    return (simCreatePointCloud_internal(maxVoxelSize, maxPtCntPerVoxel, options, pointSize, reserved));
}

SIM_DLLEXPORT int simSetPointCloudOptions_D_L(UID pointCloudHandle, double maxVoxelSize, int maxPtCntPerVoxel, int options, double pointSize, void* reserved)
{
    return (simSetPointCloudOptions_internal(pointCloudHandle, maxVoxelSize, maxPtCntPerVoxel, options, pointSize, reserved));
}

SIM_DLLEXPORT int simGetPointCloudOptions_D_L(UID pointCloudHandle, double* maxVoxelSize, int* maxPtCntPerVoxel, int* options, double* pointSize, void* reserved)
{
    return (simGetPointCloudOptions_internal(pointCloudHandle, maxVoxelSize, maxPtCntPerVoxel, options, pointSize, reserved));
}

SIM_DLLEXPORT int simInsertVoxelsIntoOctree_D_L(UID octreeHandle, int options, const double* pts, int ptCnt, const unsigned char* color, const unsigned int* tag, void* reserved)
{
    return (simInsertVoxelsIntoOctree_internal(octreeHandle, options, pts, ptCnt, color, tag, reserved));
}

SIM_DLLEXPORT int simRemoveVoxelsFromOctree_D_L(UID octreeHandle, int options, const double* pts, int ptCnt, void* reserved)
{
    return (simRemoveVoxelsFromOctree_internal(octreeHandle, options, pts, ptCnt, reserved));
}

SIM_DLLEXPORT int simInsertPointsIntoPointCloud_D_L(UID pointCloudHandle, int options, const double* pts, int ptCnt, const unsigned char* color, void* optionalValues)
{
    return (simInsertPointsIntoPointCloud_internal(pointCloudHandle, options, pts, ptCnt, color, optionalValues));
}

SIM_DLLEXPORT int simRemovePointsFromPointCloud_D_L(UID pointCloudHandle, int options, const double* pts, int ptCnt, double tolerance, void* reserved)
{
    return (simRemovePointsFromPointCloud_internal(pointCloudHandle, options, pts, ptCnt, tolerance, reserved));
}

SIM_DLLEXPORT int simIntersectPointsWithPointCloud_D_L(int pointCloudHandle, int options, const double* pts, int ptCnt, double tolerance, void* reserved)
{
    return (simIntersectPointsWithPointCloud_internal(pointCloudHandle, options, pts, ptCnt, tolerance, reserved));
}

SIM_DLLEXPORT const double* simGetOctreeVoxels_D_L(UID octreeHandle, int* ptCnt, void* reserved)
{
    return (simGetOctreeVoxels_internal(octreeHandle, ptCnt, reserved));
}

SIM_DLLEXPORT const double* simGetPointCloudPoints_D_L(UID pointCloudHandle, int* ptCnt, void* reserved)
{
    return (simGetPointCloudPoints_internal(pointCloudHandle, ptCnt, reserved));
}

SIM_DLLEXPORT int simInsertObjectIntoPointCloud_D_L(UID pointCloudHandle, UID objectHandle, int options, double gridSize, const unsigned char* color, void* optionalValues)
{
    return (simInsertObjectIntoPointCloud_internal(pointCloudHandle, objectHandle, options, gridSize, color, optionalValues));
}

SIM_DLLEXPORT int simSubtractObjectFromPointCloud_D_L(UID pointCloudHandle, UID objectHandle, int options, double tolerance, void* reserved)
{
    return (simSubtractObjectFromPointCloud_internal(pointCloudHandle, objectHandle, options, tolerance, reserved));
}

SIM_DLLEXPORT int simCheckOctreePointOccupancy_D_L(UID octreeHandle, int options, const double* points, int ptCnt, unsigned int* tag, unsigned long long int* location, void* reserved)
{
    return (simCheckOctreePointOccupancy_internal(octreeHandle, options, points, ptCnt, tag, location, reserved));
}

SIM_DLLEXPORT int simApplyTexture_D_L(UID shapeHandle, const double* textureCoordinates, int textCoordSize, const unsigned char* texture, const int* textureResolution, int options)
{
    return (simApplyTexture_internal(shapeHandle, textureCoordinates, textCoordSize, texture, textureResolution, options));
}

SIM_DLLEXPORT int simSetJointDependency_D_L(UID jointHandle, UID masterJointHandle, double offset, double multCoeff)
{
    return (simSetJointDependency_internal(jointHandle, masterJointHandle, offset, multCoeff));
}

SIM_DLLEXPORT int simGetJointDependency_D_L(UID jointHandle, UID* masterJointHandle, double* offset, double* multCoeff)
{
    return (simGetJointDependency_internal(jointHandle, masterJointHandle, offset, multCoeff));
}

SIM_DLLEXPORT int simGetShapeMass_D_L(UID shapeHandle, double* mass)
{
    return (simGetShapeMass_internal(shapeHandle, mass));
}

SIM_DLLEXPORT int simSetShapeMass_D_L(UID shapeHandle, double mass)
{
    return (simSetShapeMass_internal(shapeHandle, mass));
}

SIM_DLLEXPORT int simGetShapeInertia_D_L(UID shapeHandle, double* inertiaMatrix, double* transformationMatrix)
{
    return (simGetShapeInertia_internal(shapeHandle, inertiaMatrix, transformationMatrix));
}

SIM_DLLEXPORT int simSetShapeInertia_D_L(UID shapeHandle, const double* inertiaMatrix, const double* transformationMatrix)
{
    return (simSetShapeInertia_internal(shapeHandle, inertiaMatrix, transformationMatrix));
}

SIM_DLLEXPORT UID simGenerateShapeFromPath_D_L(const double* path, int pathSize, const double* section, int sectionSize, int options, const double* upVector, double reserved)
{
    return (simGenerateShapeFromPath_internal(path, pathSize, section, sectionSize, options, upVector, reserved));
}

SIM_DLLEXPORT bool _simGetDistanceBetweenEntitiesIfSmaller_D_L(UID entity1ID, UID entity2ID, double* distance, double* ray, int* cacheBuffer, bool overrideMeasurableFlagIfNonCollection1, bool overrideMeasurableFlagIfNonCollection2,  bool pathPlanningRoutineCalling)
{
    return (_simGetDistanceBetweenEntitiesIfSmaller_internal(entity1ID, entity2ID, distance, ray, cacheBuffer, overrideMeasurableFlagIfNonCollection1, overrideMeasurableFlagIfNonCollection2, pathPlanningRoutineCalling));
}

SIM_DLLEXPORT int _simHandleCustomContact_D_L(UID objHandle1, UID objHandle2, int engine, int* dataInt, double* dataFloat)
{
    return (_simHandleCustomContact_internal(objHandle1, objHandle2, engine, dataInt, dataFloat));
}
#endif

SIM_DLLEXPORT int simSetBoolProperty(long long int target, const char* pName, int pState)
{
    return simSetBoolProperty_internal(target, pName, pState);
}

SIM_DLLEXPORT int simGetBoolProperty(long long int target, const char* pName, int* pState)
{
    return simGetBoolProperty_internal(target, pName, pState);
}

SIM_DLLEXPORT int simSetIntProperty(long long int target, const char* pName, int pState)
{
    return simSetIntProperty_internal(target, pName, pState);
}

SIM_DLLEXPORT int simGetIntProperty(long long int target, const char* pName, int* pState)
{
    return simGetIntProperty_internal(target, pName, pState);
}

SIM_DLLEXPORT int simSetLongProperty(long long int target, const char* pName, long long int pState)
{
    return simSetLongProperty_internal(target, pName, pState);
}

SIM_DLLEXPORT int simGetLongProperty(long long int target, const char* pName, long long int* pState)
{
    return simGetLongProperty_internal(target, pName, pState);
}

SIM_DLLEXPORT int simSetFloatProperty(long long int target, const char* pName, double pState)
{
    return simSetFloatProperty_internal(target, pName, pState);
}

SIM_DLLEXPORT int simGetFloatProperty(long long int target, const char* pName, double* pState)
{
    return simGetFloatProperty_internal(target, pName, pState);
}

SIM_DLLEXPORT int simSetStringProperty(long long int target, const char* pName, const char* pState)
{
    return simSetStringProperty_internal(target, pName, pState);
}

SIM_DLLEXPORT char* simGetStringProperty(long long int target, const char* pName)
{
    return simGetStringProperty_internal(target, pName);
}

SIM_DLLEXPORT int simSetTableProperty(long long int target, const char* pName, const char* buffer, int bufferL)
{
    return simSetTableProperty_internal(target, pName, buffer, bufferL);
}

SIM_DLLEXPORT char* simGetTableProperty(long long int target, const char* pName, int* bufferL)
{
    return simGetTableProperty_internal(target, pName, bufferL);
}

SIM_DLLEXPORT int simSetBufferProperty(long long int target, const char* pName, const char* buffer, int bufferL)
{
    return simSetBufferProperty_internal(target, pName, buffer, bufferL);
}

SIM_DLLEXPORT char* simGetBufferProperty(long long int target, const char* pName, int* bufferL)
{
    return simGetBufferProperty_internal(target, pName, bufferL);
}

SIM_DLLEXPORT int simSetIntArray2Property(long long int target, const char* pName, const int* pState)
{
    return simSetIntArray2Property_internal(target, pName, pState);
}

SIM_DLLEXPORT int simGetIntArray2Property(long long int target, const char* pName, int* pState)
{
    return simGetIntArray2Property_internal(target, pName, pState);
}

SIM_DLLEXPORT int simSetVector2Property(long long int target, const char* pName, const double* pState)
{
    return simSetVector2Property_internal(target, pName, pState);
}

SIM_DLLEXPORT int simGetVector2Property(long long int target, const char* pName, double* pState)
{
    return simGetVector2Property_internal(target, pName, pState);
}

SIM_DLLEXPORT int simSetVector3Property(long long int target, const char* pName, const double* pState)
{
    return simSetVector3Property_internal(target, pName, pState);
}

SIM_DLLEXPORT int simGetVector3Property(long long int target, const char* pName, double* pState)
{
    return simGetVector3Property_internal(target, pName, pState);
}

SIM_DLLEXPORT int simSetQuaternionProperty(long long int target, const char* pName, const double* pState)
{
    return simSetQuaternionProperty_internal(target, pName, pState);
}

SIM_DLLEXPORT int simGetQuaternionProperty(long long int target, const char* pName, double* pState)
{
    return simGetQuaternionProperty_internal(target, pName, pState);
}

SIM_DLLEXPORT int simSetPoseProperty(long long int target, const char* pName, const double* pState)
{
    return simSetPoseProperty_internal(target, pName, pState);
}

SIM_DLLEXPORT int simGetPoseProperty(long long int target, const char* pName, double* pState)
{
    return simGetPoseProperty_internal(target, pName, pState);
}

SIM_DLLEXPORT int simSetColorProperty(long long int target, const char* pName, const float* pState)
{
    return simSetColorProperty_internal(target, pName, pState);
}

SIM_DLLEXPORT int simGetColorProperty(long long int target, const char* pName, float* pState)
{
    return simGetColorProperty_internal(target, pName, pState);
}

SIM_DLLEXPORT int simSetFloatArrayProperty(long long int target, const char* pName, const double* v, int vL)
{
    return simSetFloatArrayProperty_internal(target, pName, v, vL);
}

SIM_DLLEXPORT double* simGetFloatArrayProperty(long long int target, const char* pName, int* vL)
{
    return simGetFloatArrayProperty_internal(target, pName, vL);
}

SIM_DLLEXPORT int simSetIntArrayProperty(long long int target, const char* pName, const int* v, int vL)
{
    return simSetIntArrayProperty_internal(target, pName, v, vL);
}

SIM_DLLEXPORT int* simGetIntArrayProperty(long long int target, const char* pName, int* vL)
{
    return simGetIntArrayProperty_internal(target, pName, vL);
}

SIM_DLLEXPORT int simRemoveProperty(long long int target, const char* pName)
{
    return simRemoveProperty_internal(target, pName);
}

SIM_DLLEXPORT char* simGetPropertyName(long long int target, int index, SPropertyOptions* options)
{
    return simGetPropertyName_internal(target, index, options);
}

SIM_DLLEXPORT int simGetPropertyInfo(long long int target, const char* pName, SPropertyInfo* infos, SPropertyOptions* options)
{
    return simGetPropertyInfo_internal(target, pName, infos, options);
}

SIM_DLLEXPORT void simRegCallback(int index, void* callback)
{
    simRegCallback_internal(index, callback);
}

SIM_DLLEXPORT void simRunGui(int options)
{
    simRunGui_internal(options);
}

SIM_DLLEXPORT int simPostExitRequest()
{
    return (simPostExitRequest_internal());
}

SIM_DLLEXPORT int simGetExitRequest()
{
    return (simGetExitRequest_internal());
}

SIM_DLLEXPORT int simLoop(void (*callback)(), int options)
{
    return (simLoop_internal(callback, options));
}

SIM_DLLEXPORT int simInitialize(const char* appDir, int options)
{
    return (simInitialize_internal(appDir, options));
}

SIM_DLLEXPORT int simDeinitialize()
{
    return (simDeinitialize_internal());
}


SIM_DLLEXPORT int simTest(int mode, void* ptr1, void* ptr2, void* ptr3)
{
    return (simTest_internal(mode, ptr1, ptr2, ptr3));
}

SIM_DLLEXPORT void* simGetMainWindow(int type)
{
    return (simGetMainWindow_internal(type));
}

SIM_DLLEXPORT char* simGetLastError()
{
    return (simGetLastError_internal());
}

SIM_DLLEXPORT char* simGetLastInfo()
{
    return (simGetLastInfo_internal());
}

SIM_DLLEXPORT int simGetObject(const char* objectPath, int index, int proxy, int options)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simGetObject_internal(objectPath, index, App::getNewHandleFromOldHandle(proxy), options));
}

SIM_DLLEXPORT long long int simGetObjectUid(int objectHandle)
{ // backw. compatibility version
    return simGetObjectUid_internal(App::getNewHandleFromOldHandle(objectHandle));
}

SIM_DLLEXPORT int simGetObjectFromUid(long long int uid, int options)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simGetObjectFromUid_internal(uid, options));
}

SIM_DLLEXPORT int simGetScriptHandleEx(int scriptType, int objectHandle, const char* scriptName)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simGetScriptHandleEx_internal(scriptType, App::getNewHandleFromOldHandle(objectHandle), scriptName));
}

SIM_DLLEXPORT int simRemoveObjects(const int* objectHandles, int count)
{ // backw. compatibility version
    std::vector<UID> v(count);
    for (size_t i = 0; i < count; i++)
        v[i] = App::getNewHandleFromOldHandle(objectHandles[i]);
    return simRemoveObjects_internal(v.data(), count);
}

SIM_DLLEXPORT int simRemoveModel(int objectHandle)
{ // backw. compatibility version
    return simRemoveModel_internal(App::getNewHandleFromOldHandle(objectHandle));
}

SIM_DLLEXPORT char* simGetObjectAlias(int objectHandle, int options)
{ // backw. compatibility version
    return (simGetObjectAlias_internal(App::getNewHandleFromOldHandle(objectHandle), options));
}

SIM_DLLEXPORT int simSetObjectAlias(int objectHandle, const char* objectAlias, int options)
{ // backw. compatibility version
    return (simSetObjectAlias_internal(App::getNewHandleFromOldHandle(objectHandle), objectAlias, options));
}

SIM_DLLEXPORT int simGetObjectParent(int objectHandle)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simGetObjectParent_internal(App::getNewHandleFromOldHandle(objectHandle)));
}

SIM_DLLEXPORT int simGetObjectChild(int objectHandle, int index)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simGetObjectChild_internal(App::getNewHandleFromOldHandle(objectHandle), index));
}

SIM_DLLEXPORT int simSetObjectParent(int objectHandle, int parentObjectHandle, bool keepInPlace)
{ // backw. compatibility version
    return (simSetObjectParent_internal(App::getNewHandleFromOldHandle(objectHandle), App::getNewHandleFromOldHandle(parentObjectHandle), keepInPlace));
}

SIM_DLLEXPORT int simGetObjectType(int objectHandle)
{ // backw. compatibility version
    return (simGetObjectType_internal(App::getNewHandleFromOldHandle(objectHandle)));
}

SIM_DLLEXPORT int simGetJointType(int objectHandle)
{ // backw. compatibility version
    return (simGetJointType_internal(App::getNewHandleFromOldHandle(objectHandle)));
}

SIM_DLLEXPORT int simReservedCommand(int v, int w)
{
    return (simReservedCommand_internal(v, w));
}

SIM_DLLEXPORT int simGetSimulationState()
{
    return (simGetSimulationState_internal());
}

SIM_DLLEXPORT int simLoadScene(const char* filename)
{
    return (simLoadScene_internal(filename));
}

SIM_DLLEXPORT int simCloseScene()
{
    return (simCloseScene_internal());
}

SIM_DLLEXPORT int simSaveScene(const char* filename)
{
    return (simSaveScene_internal(filename));
}

SIM_DLLEXPORT int simLoadModel(const char* filename)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simLoadModel_internal(filename));
}

SIM_DLLEXPORT int simSaveModel(int baseOfModelHandle, const char* filename)
{ // backw. compatibility version
    return (simSaveModel_internal(App::getNewHandleFromOldHandle(baseOfModelHandle), filename));
}

SIM_DLLEXPORT char* simGetSimulatorMessage(int* messageID, int* auxiliaryData, int* returnedDataSize)
{
    return (simGetSimulatorMessage_internal(messageID, auxiliaryData, returnedDataSize));
}

SIM_DLLEXPORT int simDoesFileExist(const char* filename)
{
    return (simDoesFileExist_internal(filename));
}

SIM_DLLEXPORT int* simGetObjectSel(int* cnt)
{ // backw. compatibility version
    UID* sel = simGetObjectSel_internal(cnt);
    int* retVal = new int[cnt[0]];
    for (size_t i = 0; i < cnt[0]; i++)
        retVal[i] = App::getOldHandleFromNewHandle(sel[i]);
    delete[] sel;
    return retVal;
}

SIM_DLLEXPORT int simSetObjectSel(const int* handles, int cnt)
{ // backw. compatibility version
    std::vector<UID> sel(cnt);
    for (size_t i = 0; i < cnt; i++)
        sel[i] = App::getNewHandleFromOldHandle(handles[i]);
    return simSetObjectSel_internal(sel.data(), cnt);
}

SIM_DLLEXPORT int simAssociateScriptWithObject(int scriptHandle, int associatedObjectHandle)
{ // backw. compatibility version
    return simAssociateScriptWithObject_internal(App::getNewHandleFromOldHandle(scriptHandle), App::getNewHandleFromOldHandle(associatedObjectHandle));
}

SIM_DLLEXPORT int simResetScript(int scriptHandle)
{ // backw. compatibility version
    return simResetScript_internal(App::getNewHandleFromOldHandle(scriptHandle));
}

SIM_DLLEXPORT int simAddScript(int scriptProperty)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simAddScript_internal(scriptProperty));
}

SIM_DLLEXPORT int simRemoveScript(int scriptHandle)
{ // backw. compatibility version
    return simRemoveScript_internal(App::getNewHandleFromOldHandle(scriptHandle));
}

SIM_DLLEXPORT int simRefreshDialogs(int refreshDegree)
{
    return (simRefreshDialogs_internal(refreshDegree));
}

SIM_DLLEXPORT int simResetProximitySensor(int sensorHandle)
{ // backw. compatibility version
    return simResetProximitySensor_internal(App::getNewHandleFromOldHandle(sensorHandle));
}

SIM_DLLEXPORT void* simCreateBuffer(int size)
{
    return (simCreateBuffer_internal(size));
}

SIM_DLLEXPORT int simReleaseBuffer(const void* buffer)
{
    return (simReleaseBuffer_internal(buffer));
}

SIM_DLLEXPORT int simCheckCollision(int entity1Handle, int entity2Handle)
{ // backw. compatibility version
    return simCheckCollision_internal(App::getNewHandleFromOldHandle(entity1Handle), App::getNewHandleFromOldHandle(entity2Handle));
}

SIM_DLLEXPORT int simGetRealTimeSimulation()
{
    return (simGetRealTimeSimulation_internal());
}

SIM_DLLEXPORT int simStartSimulation()
{
    return (simStartSimulation_internal());
}

SIM_DLLEXPORT int simStopSimulation()
{
    return (simStopSimulation_internal());
}

SIM_DLLEXPORT int simPauseSimulation()
{
    return (simPauseSimulation_internal());
}

SIM_DLLEXPORT int simAdjustView(int viewHandleOrIndex, int associatedViewableObjectHandle, int options, const char* viewLabel)
{ // backw. compatibility version
    return (simAdjustView_internal(viewHandleOrIndex, App::getNewHandleFromOldHandle(associatedViewableObjectHandle), options, viewLabel));
}

SIM_DLLEXPORT int simSetLastError(const char* setToNullptr, const char* errorMessage)
{
    return (simSetLastError_internal(setToNullptr, errorMessage));
}

SIM_DLLEXPORT int simResetGraph(int graphHandle)
{ // backw. compatibility version
    return simResetGraph_internal(App::getNewHandleFromOldHandle(graphHandle));
}

SIM_DLLEXPORT int simDestroyGraphCurve(int graphHandle, int curveId)
{ // backw. compatibility version
    return simDestroyGraphCurve_internal(App::getNewHandleFromOldHandle(graphHandle), curveId);
}

SIM_DLLEXPORT int simDuplicateGraphCurveToStatic(int graphHandle, int curveId, const char* curveName)
{ // backw. compatibility version
    return simDuplicateGraphCurveToStatic_internal(App::getNewHandleFromOldHandle(graphHandle), curveId, curveName);
}

SIM_DLLEXPORT int simSetNavigationMode(int navigationMode)
{
    return (simSetNavigationMode_internal(navigationMode));
}

SIM_DLLEXPORT int simGetNavigationMode()
{
    return (simGetNavigationMode_internal());
}

SIM_DLLEXPORT int simSetPage(int index)
{
    return (simSetPage_internal(index));
}

SIM_DLLEXPORT int simGetPage()
{
    return (simGetPage_internal());
}

SIM_DLLEXPORT int simRegisterScriptCallbackFunction(const char* func, const char* reserved_setToNull, void (*callBack)(struct SScriptCallBack* cb))
{
    return (simRegisterScriptCallbackFunction_internal(func, reserved_setToNull, callBack));
}

SIM_DLLEXPORT int simRegisterScriptVariable(const char* var, const char* val, int stackHandle)
{ // backw. compatibility version
    return simRegisterScriptVariable_internal(var, val, App::getNewHandleFromOldHandle(stackHandle));
}

SIM_DLLEXPORT int simRegisterScriptFuncHook(int scriptHandle, const char* funcToHook, const char* userFunction, bool executeBefore, int options)
{ // backw. compatibility version
    return simRegisterScriptFuncHook_internal(App::getNewHandleFromOldHandle(scriptHandle), funcToHook, userFunction, executeBefore, options);
}

SIM_DLLEXPORT int simCopyPasteObjects(int* objectHandles, int objectCount, int options)
{ // backw. compatibility version
    std::vector<UID> inOut(objectCount);
    for (size_t i = 0; i < objectCount; i++)
        inOut[i] = App::getNewHandleFromOldHandle(objectHandles[i]);
    int retVal = simCopyPasteObjects_internal(inOut.data(), objectCount, options);
    for (size_t i = 0; i < objectCount; i++)
        objectHandles[i] = App::getOldHandleFromNewHandle(inOut[i]);
    return retVal;
}

SIM_DLLEXPORT char* simReceiveData(int dataHeader, const char* dataName, int antennaHandle, int index, int* dataLength, int* senderID, int* dataHeaderR, char** dataNameR)
{ // backw. compatibility version
    UID _senderId;
    char* retVal = simReceiveData_internal(dataHeader, dataName, App::getNewHandleFromOldHandle(antennaHandle), index, dataLength, &_senderId, dataHeaderR, dataNameR);
    if (senderID != nullptr)
        senderID[0] = App::getOldHandleFromNewHandle(_senderId);
    return retVal;
}

SIM_DLLEXPORT int simRemoveDrawingObject(int objectHandle)
{ // backw. compatibility version
    return simRemoveDrawingObject_internal(App::getNewHandleFromOldHandle(objectHandle));
}

SIM_DLLEXPORT int simAnnounceSceneContentChange()
{
    return (simAnnounceSceneContentChange_internal());
}

SIM_DLLEXPORT int simSetExplicitHandling(int objectHandle, int explicitFlags)
{ // backw. compatibility version
    return simSetExplicitHandling_internal(App::getNewHandleFromOldHandle(objectHandle), explicitFlags);
}

SIM_DLLEXPORT int simGetExplicitHandling(int objectHandle)
{ // backw. compatibility version
    return simGetExplicitHandling_internal(App::getNewHandleFromOldHandle(objectHandle));
}

SIM_DLLEXPORT int simGetLinkDummy(int dummyHandle)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simGetLinkDummy_internal(App::getNewHandleFromOldHandle(dummyHandle)));
}

SIM_DLLEXPORT int simSetLinkDummy(int dummyHandle, int linkedDummyHandle)
{ // backw. compatibility version
    return simSetLinkDummy_internal(App::getNewHandleFromOldHandle(dummyHandle), App::getNewHandleFromOldHandle(linkedDummyHandle));
}

SIM_DLLEXPORT int simResetDynamicObject(int objectHandle)
{ // backw. compatibility version
    return simResetDynamicObject_internal(App::getNewHandleFromOldHandle(objectHandle));
}

SIM_DLLEXPORT int simSetJointMode(int jointHandle, int jointMode, int options)
{ // backw. compatibility version
    return simSetJointMode_internal(App::getNewHandleFromOldHandle(jointHandle), jointMode, options);
}

SIM_DLLEXPORT int simGetJointMode(int jointHandle, int* options)
{ // backw. compatibility version
    return simGetJointMode_internal(App::getNewHandleFromOldHandle(jointHandle), options);
}

SIM_DLLEXPORT int simSerialOpen(const char* portString, int baudRate, void* reserved1, void* reserved2)
{
    return (simSerialOpen_internal(portString, baudRate, reserved1, reserved2));
}

SIM_DLLEXPORT int simSerialClose(int portHandle)
{
    return (simSerialClose_internal(portHandle));
}

SIM_DLLEXPORT int simSerialSend(int portHandle, const char* data, int dataLength)
{
    return (simSerialSend_internal(portHandle, data, dataLength));
}

SIM_DLLEXPORT int simSerialRead(int portHandle, char* buffer, int dataLengthToRead)
{
    return (simSerialRead_internal(portHandle, buffer, dataLengthToRead));
}

SIM_DLLEXPORT int simSerialCheck(int portHandle)
{
    return (simSerialCheck_internal(portHandle));
}

SIM_DLLEXPORT int simAuxiliaryConsoleClose(int consoleHandle)
{
    return (simAuxiliaryConsoleClose_internal(consoleHandle));
}

SIM_DLLEXPORT int simAuxiliaryConsoleShow(int consoleHandle, bool showState)
{
    return (simAuxiliaryConsoleShow_internal(consoleHandle, showState));
}

SIM_DLLEXPORT int simAuxiliaryConsolePrint(int consoleHandle, const char* text)
{
    return (simAuxiliaryConsolePrint_internal(consoleHandle, text));
}

SIM_DLLEXPORT int simIsHandle(int generalObjectHandle, int generalObjectType)
{ // backw. compatibility version
    return simIsHandle_internal(App::getNewHandleFromOldHandle(generalObjectHandle), generalObjectType);
}

SIM_DLLEXPORT int simResetVisionSensor(int visionSensorHandle)
{ // backw. compatibility version
    return simResetVisionSensor_internal(App::getNewHandleFromOldHandle(visionSensorHandle));
}

SIM_DLLEXPORT int simSetVisionSensorImg(int sensorHandle, const unsigned char* img, int options, const int* pos, const int* size)
{ // backw. compatibility version
    return simSetVisionSensorImg_internal(App::getNewHandleFromOldHandle(sensorHandle), img, options, pos, size);
}

SIM_DLLEXPORT int simRuckigPos(int dofs, double baseCycleTime, int flags, const double* currentPos, const double* currentVel, const double* currentAccel, const double* maxVel, const double* maxAccel, const double* maxJerk, const bool* selection, const double* targetPos, const double* targetVel, double* reserved1, int* reserved2)
{
    return (simRuckigPos_internal(dofs, baseCycleTime, flags, currentPos, currentVel, currentAccel, maxVel, maxAccel, maxJerk, selection, targetPos, targetVel, reserved1, reserved2));
}

SIM_DLLEXPORT int simRuckigVel(int dofs, double baseCycleTime, int flags, const double* currentPos, const double* currentVel, const double* currentAccel, const double* maxAccel, const double* maxJerk, const bool* selection, const double* targetVel, double* reserved1, int* reserved2)
{
    return (simRuckigVel_internal(dofs, baseCycleTime, flags, currentPos, currentVel, currentAccel, maxAccel, maxJerk, selection, targetVel, reserved1, reserved2));
}

SIM_DLLEXPORT int simRuckigStep(int objHandle, double cycleTime, double* newPos, double* newVel, double* newAccel, double* syncTime, double* reserved1, int* reserved2)
{
    return (simRuckigStep_internal(objHandle, cycleTime, newPos, newVel, newAccel, syncTime, reserved1, reserved2));
}

SIM_DLLEXPORT int simRuckigRemove(int objHandle)
{
    return (simRuckigRemove_internal(objHandle));
}

SIM_DLLEXPORT int simGroupShapes(const int* shapeHandles, int shapeCount)
{ // backw. compatibility version
    std::vector<UID> sel(shapeCount);
    for (size_t i = 0; i < shapeCount; i++)
        sel[i] = App::getNewHandleFromOldHandle(shapeHandles[i]);
    return App::getOldHandleFromNewHandle(simGroupShapes_internal(sel.data(), shapeCount));
}

SIM_DLLEXPORT int* simUngroupShape(int shapeHandle, int* shapeCount)
{ // backw. compatibility version
    UID* v = simUngroupShape_internal(App::getNewHandleFromOldHandle(shapeHandle), shapeCount);
    int* retVal = new int[shapeCount[0]];
    for (size_t i = 0; i < shapeCount[0]; i++)
        retVal[i] = App::getOldHandleFromNewHandle(v[i]);
    delete[] v;
    return retVal;
}

SIM_DLLEXPORT void simQuitSimulator(bool ignoredArgument)
{
    simQuitSimulator_internal(ignoredArgument);
}

SIM_DLLEXPORT int simSetShapeMaterial(int shapeHandle, int materialIdOrShapeHandle)
{ // backw. compatibility version
    return simSetShapeMaterial_internal(App::getNewHandleFromOldHandle(shapeHandle), App::getNewHandleFromOldHandle(materialIdOrShapeHandle));
}

SIM_DLLEXPORT int simGetTextureId(const char* textureName, int* resolution)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simGetTextureId_internal(textureName, resolution));
}

SIM_DLLEXPORT unsigned char* simReadTexture(int textureId, int options, int posX, int posY, int sizeX, int sizeY)
{ // backw. compatibility version
    return (simReadTexture_internal(App::getNewHandleFromOldHandle(textureId), options, posX, posY, sizeX, sizeY));
}

SIM_DLLEXPORT int simGetObjects(int index, int objectType)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simGetObjects_internal(index, objectType));
}

SIM_DLLEXPORT int* simGetObjectsInTree(int treeBaseHandle, int objectType, int options, int* objectCount)
{ // backw. compatibility version
    UID* v = simGetObjectsInTree_internal(App::getNewHandleFromOldHandle(treeBaseHandle), objectType, options, objectCount);
    int* retVal = new int[objectCount[0]];
    for (size_t i = 0; i < objectCount[0]; i++)
        retVal[i] = App::getOldHandleFromNewHandle(v[i]);
    delete[] v;
    return retVal;
}

SIM_DLLEXPORT int simGetShapeTextureId(int shapeHandle)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simGetShapeTextureId_internal(App::getNewHandleFromOldHandle(shapeHandle)));
}

SIM_DLLEXPORT int simCreateCollectionEx(int options)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simCreateCollectionEx_internal(options));
}

SIM_DLLEXPORT int simAddItemToCollection(int collectionHandle, int what, int objectHandle, int options)
{ // backw. compatibility version
    return simAddItemToCollection_internal(App::getNewHandleFromOldHandle(collectionHandle), what, App::getNewHandleFromOldHandle(objectHandle), options);
}

SIM_DLLEXPORT int simDestroyCollection(int collectionHandle)
{ // backw. compatibility version
    return simDestroyCollection_internal(App::getNewHandleFromOldHandle(collectionHandle));
}

SIM_DLLEXPORT int* simGetCollectionObjects(int collectionHandle, int* objectCount)
{ // backw. compatibility version
    UID* v = simGetCollectionObjects_internal(App::getNewHandleFromOldHandle(collectionHandle), objectCount);
    int* retVal = new int[objectCount[0]];
    for (size_t i = 0; i < objectCount[0]; i++)
        retVal[i] = App::getOldHandleFromNewHandle(v[i]);
    delete[] v;
    return retVal;
}

SIM_DLLEXPORT int simAlignShapeBB(int shapeHandle, const double* pose)
{ // backw. compatibility version
    return simAlignShapeBB_internal(App::getNewHandleFromOldHandle(shapeHandle), pose);
}

SIM_DLLEXPORT int simRelocateShapeFrame(int shapeHandle, const double* pose)
{ // backw. compatibility version
    return simRelocateShapeFrame_internal(App::getNewHandleFromOldHandle(shapeHandle), pose);
}

SIM_DLLEXPORT int simSaveImage(const unsigned char* image, const int* resolution, int options, const char* filename, int quality, void* reserved)
{
    return (simSaveImage_internal(image, resolution, options, filename, quality, reserved));
}

SIM_DLLEXPORT unsigned char* simLoadImage(int* resolution, int options, const char* filename, void* reserved)
{
    return (simLoadImage_internal(resolution, options, filename, reserved));
}

SIM_DLLEXPORT unsigned char* simGetScaledImage(const unsigned char* imageIn, const int* resolutionIn, int* resolutionOut, int options, void* reserved)
{
    return (simGetScaledImage_internal(imageIn, resolutionIn, resolutionOut, options, reserved));
}

SIM_DLLEXPORT int simCallScriptFunctionEx(int scriptHandleOrType, const char* functionNameAtScriptName, int stackId)
{ // backw. compatibility version
    if (scriptHandleOrType > sim_scripttype_sandbox)
        scriptHandleOrType = App::getNewHandleFromOldHandle(scriptHandleOrType);
    return simCallScriptFunctionEx_internal(scriptHandleOrType, functionNameAtScriptName, App::getNewHandleFromOldHandle(stackId));
}

SIM_DLLEXPORT char* simGetExtensionString(int objectHandle, int index, const char* key)
{ // backw. compatibility version
    return simGetExtensionString_internal(App::getNewHandleFromOldHandle(objectHandle), index, key);
}

SIM_DLLEXPORT int simCreateStack()
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simCreateStack_internal());
}

SIM_DLLEXPORT int simReleaseStack(int stackHandle)
{ // backw. compatibility version
    return simReleaseStack_internal(App::getNewHandleFromOldHandle(stackHandle));
}

SIM_DLLEXPORT int simCopyStack(int stackHandle)
{ // backw. compatibility version
    return simCopyStack_internal(App::getNewHandleFromOldHandle(stackHandle));
}

SIM_DLLEXPORT int simPushNullOntoStack(int stackHandle)
{ // backw. compatibility version
    return simPushNullOntoStack_internal(App::getNewHandleFromOldHandle(stackHandle));
}

SIM_DLLEXPORT int simPushBoolOntoStack(int stackHandle, bool value)
{ // backw. compatibility version
    return simPushBoolOntoStack_internal(App::getNewHandleFromOldHandle(stackHandle), value);
}

SIM_DLLEXPORT int simPushInt32OntoStack(int stackHandle, int value)
{ // backw. compatibility version
    return simPushInt32OntoStack_internal(App::getNewHandleFromOldHandle(stackHandle), value);
}

SIM_DLLEXPORT int simPushInt64OntoStack(int stackHandle, long long int value)
{ // backw. compatibility version
    return simPushInt64OntoStack_internal(App::getNewHandleFromOldHandle(stackHandle), value);
}

SIM_DLLEXPORT int simPushFloatOntoStack(int stackHandle, float value)
{ // backw. compatibility version
    return simPushFloatOntoStack_internal(App::getNewHandleFromOldHandle(stackHandle), value);
}

SIM_DLLEXPORT int simPushDoubleOntoStack(int stackHandle, double value)
{ // backw. compatibility version
    return simPushDoubleOntoStack_internal(App::getNewHandleFromOldHandle(stackHandle), value);
}

SIM_DLLEXPORT int simPushTextOntoStack(int stackHandle, const char* value)
{ // backw. compatibility version
    return simPushTextOntoStack_internal(App::getNewHandleFromOldHandle(stackHandle), value);
}

SIM_DLLEXPORT int simPushStringOntoStack(int stackHandle, const char* value, int stringSize)
{ // backw. compatibility version
    return simPushStringOntoStack_internal(App::getNewHandleFromOldHandle(stackHandle), value, stringSize);
}

SIM_DLLEXPORT int simPushBufferOntoStack(int stackHandle, const char* value, int stringSize)
{ // backw. compatibility version
    return simPushBufferOntoStack_internal(App::getNewHandleFromOldHandle(stackHandle), value, stringSize);
}

SIM_DLLEXPORT int simPushUInt8TableOntoStack(int stackHandle, const unsigned char* values, int valueCnt)
{ // backw. compatibility version
    return simPushUInt8TableOntoStack_internal(App::getNewHandleFromOldHandle(stackHandle), values, valueCnt);
}

SIM_DLLEXPORT int simPushInt32TableOntoStack(int stackHandle, const int* values, int valueCnt)
{ // backw. compatibility version
    return simPushInt32TableOntoStack_internal(App::getNewHandleFromOldHandle(stackHandle), values, valueCnt);
}

SIM_DLLEXPORT int simPushInt64TableOntoStack(int stackHandle, const long long int* values, int valueCnt)
{ // backw. compatibility version
    return simPushInt64TableOntoStack_internal(App::getNewHandleFromOldHandle(stackHandle), values, valueCnt);
}

SIM_DLLEXPORT int simPushFloatTableOntoStack(int stackHandle, const float* values, int valueCnt)
{ // backw. compatibility version
    return simPushFloatTableOntoStack_internal(App::getNewHandleFromOldHandle(stackHandle), values, valueCnt);
}

SIM_DLLEXPORT int simPushDoubleTableOntoStack(int stackHandle, const double* values, int valueCnt)
{ // backw. compatibility version
    return simPushDoubleTableOntoStack_internal(App::getNewHandleFromOldHandle(stackHandle), values, valueCnt);
}

SIM_DLLEXPORT int simPushTableOntoStack(int stackHandle)
{ // backw. compatibility version
    return simPushTableOntoStack_internal(App::getNewHandleFromOldHandle(stackHandle));
}

SIM_DLLEXPORT int simInsertDataIntoStackTable(int stackHandle)
{ // backw. compatibility version
    return simInsertDataIntoStackTable_internal(App::getNewHandleFromOldHandle(stackHandle));
}

SIM_DLLEXPORT int simGetStackSize(int stackHandle)
{ // backw. compatibility version
    return simGetStackSize_internal(App::getNewHandleFromOldHandle(stackHandle));
}

SIM_DLLEXPORT int simPopStackItem(int stackHandle, int count)
{ // backw. compatibility version
    return simPopStackItem_internal(App::getNewHandleFromOldHandle(stackHandle), count);
}

SIM_DLLEXPORT int simMoveStackItemToTop(int stackHandle, int cIndex)
{ // backw. compatibility version
    return simMoveStackItemToTop_internal(App::getNewHandleFromOldHandle(stackHandle), cIndex);
}

SIM_DLLEXPORT int simGetStackItemType(int stackHandle, int cIndex)
{ // backw. compatibility version
    return simGetStackItemType_internal(App::getNewHandleFromOldHandle(stackHandle), cIndex);
}

SIM_DLLEXPORT int simGetStackStringType(int stackHandle, int cIndex)
{ // backw. compatibility version
    return simGetStackStringType_internal(App::getNewHandleFromOldHandle(stackHandle), cIndex);
}

SIM_DLLEXPORT int simGetStackBoolValue(int stackHandle, bool* boolValue)
{ // backw. compatibility version
    return simGetStackBoolValue_internal(App::getNewHandleFromOldHandle(stackHandle), boolValue);
}

SIM_DLLEXPORT int simGetStackInt32Value(int stackHandle, int* numberValue)
{ // backw. compatibility version
    return simGetStackInt32Value_internal(App::getNewHandleFromOldHandle(stackHandle), numberValue);
}

SIM_DLLEXPORT int simGetStackInt64Value(int stackHandle, long long int* numberValue)
{ // backw. compatibility version
    return simGetStackInt64Value_internal(App::getNewHandleFromOldHandle(stackHandle), numberValue);
}

SIM_DLLEXPORT int simGetStackFloatValue(int stackHandle, float* numberValue)
{ // backw. compatibility version
    return simGetStackFloatValue_internal(App::getNewHandleFromOldHandle(stackHandle), numberValue);
}

SIM_DLLEXPORT int simGetStackDoubleValue(int stackHandle, double* numberValue)
{ // backw. compatibility version
    return simGetStackDoubleValue_internal(App::getNewHandleFromOldHandle(stackHandle), numberValue);
}

SIM_DLLEXPORT char* simGetStackStringValue(int stackHandle, int* stringSize)
{ // backw. compatibility version
    return simGetStackStringValue_internal(App::getNewHandleFromOldHandle(stackHandle), stringSize);
}

SIM_DLLEXPORT int simGetStackTableInfo(int stackHandle, int infoType)
{ // backw. compatibility version
    return simGetStackTableInfo_internal(App::getNewHandleFromOldHandle(stackHandle), infoType);
}

SIM_DLLEXPORT int simGetStackUInt8Table(int stackHandle, unsigned char* array, int count)
{ // backw. compatibility version
    return simGetStackUInt8Table_internal(App::getNewHandleFromOldHandle(stackHandle), array, count);
}

SIM_DLLEXPORT int simGetStackInt32Table(int stackHandle, int* array, int count)
{ // backw. compatibility version
    return simGetStackInt32Table_internal(App::getNewHandleFromOldHandle(stackHandle), array, count);
}

SIM_DLLEXPORT int simGetStackInt64Table(int stackHandle, long long int* array, int count)
{ // backw. compatibility version
    return simGetStackInt64Table_internal(App::getNewHandleFromOldHandle(stackHandle), array, count);
}

SIM_DLLEXPORT int simGetStackFloatTable(int stackHandle, float* array, int count)
{ // backw. compatibility version
    return simGetStackFloatTable_internal(App::getNewHandleFromOldHandle(stackHandle), array, count);
}

SIM_DLLEXPORT int simGetStackDoubleTable(int stackHandle, double* array, int count)
{ // backw. compatibility version
    return simGetStackDoubleTable_internal(App::getNewHandleFromOldHandle(stackHandle), array, count);
}

SIM_DLLEXPORT int simUnfoldStackTable(int stackHandle)
{ // backw. compatibility version
    return simUnfoldStackTable_internal(App::getNewHandleFromOldHandle(stackHandle));
}

SIM_DLLEXPORT int simDebugStack(int stackHandle, int cIndex)
{ // backw. compatibility version
    return simDebugStack_internal(App::getNewHandleFromOldHandle(stackHandle), cIndex);
}

SIM_DLLEXPORT int simInsertObjectIntoOctree(int octreeHandle, int objectHandle, int options, const unsigned char* color, unsigned int tag, void* reserved)
{ // backw. compatibility version
    return simInsertObjectIntoOctree_internal(App::getNewHandleFromOldHandle(octreeHandle), App::getNewHandleFromOldHandle(objectHandle), options, color, tag, reserved);
}

SIM_DLLEXPORT int simSubtractObjectFromOctree(int octreeHandle, int objectHandle, int options, void* reserved)
{ // backw. compatibility version
    return simSubtractObjectFromOctree_internal(App::getNewHandleFromOldHandle(octreeHandle), App::getNewHandleFromOldHandle(objectHandle), options, reserved);
}

SIM_DLLEXPORT char* simOpenTextEditor(const char* initText, const char* xml, int* various)
{
    return (simOpenTextEditor_internal(initText, xml, various));
}

SIM_DLLEXPORT char* simPackTable(int stackHandle, int* bufferSize)
{ // backw. compatibility version
    return simPackTable_internal(App::getNewHandleFromOldHandle(stackHandle), bufferSize);
}

SIM_DLLEXPORT int simUnpackTable(int stackHandle, const char* buffer, int bufferSize)
{ // backw. compatibility version
    return simUnpackTable_internal(App::getNewHandleFromOldHandle(stackHandle), buffer, bufferSize);
}

SIM_DLLEXPORT int simSetReferencedHandles(int objectHandle, int count, const int* referencedHandles, const char* tag, const int* reserved2)
{ // backw. compatibility version
    std::vector<UID> v(count);
    for (size_t i = 0; i < count; i++)
        v[i] = App::getNewHandleFromOldHandle(referencedHandles[i]);
    return simSetReferencedHandles_internal(App::getNewHandleFromOldHandle(objectHandle), count, v.data(), tag, reserved2);
}

SIM_DLLEXPORT int simGetReferencedHandles(int objectHandle, int** referencedHandles, const char* tag, int** reserved2)
{ // backw. compatibility version
    UID* v;
    int retVal = simGetReferencedHandles_internal(App::getNewHandleFromOldHandle(objectHandle), &v, tag, reserved2);
    referencedHandles[0] = new int[retVal];
    for (size_t i = 0; i < retVal; i++)
        referencedHandles[0][i] = App::getOldHandleFromNewHandle(v[i]);
    return retVal;
}

SIM_DLLEXPORT int simExecuteScriptString(int scriptHandle, const char* stringToExecute, int stackHandle)
{ // backw. compatibility version
    return simExecuteScriptString_internal(App::getNewHandleFromOldHandle(scriptHandle), stringToExecute, App::getNewHandleFromOldHandle(stackHandle));
}

SIM_DLLEXPORT char* simGetApiFunc(int scriptHandle, const char* apiWord)
{ // backw. compatibility version
    return simGetApiFunc_internal(App::getNewHandleFromOldHandle(scriptHandle), apiWord);
}

SIM_DLLEXPORT char* simGetApiInfo(int scriptHandle, const char* apiWord)
{ // backw. compatibility version
    return simGetApiInfo_internal(App::getNewHandleFromOldHandle(scriptHandle), apiWord);
}

SIM_DLLEXPORT int simSetPluginInfo(const char* pluginName, int infoType, const char* stringInfo, int intInfo)
{
    return (simSetPluginInfo_internal(pluginName, infoType, stringInfo, intInfo));
}

SIM_DLLEXPORT int simGetPluginInfo(const char* pluginName, int infoType, char** stringInfo, int* intInfo)
{
    return (simGetPluginInfo_internal(pluginName, infoType, stringInfo, intInfo));
}

SIM_DLLEXPORT char* simGetPluginName(int index)
{
    return (simGetPluginName_internal(index, nullptr));
}

SIM_DLLEXPORT int simEventNotification(const char* event)
{
    return (simEventNotification_internal(event));
}

SIM_DLLEXPORT int simAddLog(const char* pluginName, int verbosityLevel, const char* logMsg)
{
    return (simAddLog_internal(pluginName, verbosityLevel, logMsg));
}

SIM_DLLEXPORT int simIsDynamicallyEnabled(int objectHandle)
{ // backw. compatibility version
    return simIsDynamicallyEnabled_internal(App::getNewHandleFromOldHandle(objectHandle));
}

SIM_DLLEXPORT int simInitScript(int scriptHandle)
{ // backw. compatibility version
    return simInitScript_internal(App::getNewHandleFromOldHandle(scriptHandle));
}

SIM_DLLEXPORT int simModuleEntry(int handle, const char* label, int state)
{
    return simModuleEntry_internal(handle, label, state);
}

SIM_DLLEXPORT int simCheckExecAuthorization(const char* what, const char* args)
{
    return (simCheckExecAuthorization_internal(what, args, -1));
}

SIM_DLLEXPORT int simGetVisionSensorRes(int sensorHandle, int* resolution)
{ // backw. compatibility version
    return simGetVisionSensorRes_internal(App::getNewHandleFromOldHandle(sensorHandle), resolution);
}

SIM_DLLEXPORT int simCreateShape(int options, double shadingAngle, const double* vertices, int verticesSize, const int* indices, int indicesSize, const double* normals, const float* textureCoords, const unsigned char* texture, const int* textureRes)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simCreateShape_internal(options, shadingAngle, vertices, verticesSize, indices, indicesSize, normals, textureCoords, texture, textureRes));
}

SIM_DLLEXPORT void _simSetDynamicSimulationIconCode(void* object, int code)
{
    return (_simSetDynamicSimulationIconCode_internal(object, code));
}

SIM_DLLEXPORT void _simSetDynamicObjectFlagForVisualization(void* object, int flag)
{
    return (_simSetDynamicObjectFlagForVisualization_internal(object, flag));
}

SIM_DLLEXPORT int _simGetObjectListSize(int objType)
{
    return (_simGetObjectListSize_internal(objType));
}

SIM_DLLEXPORT const void* _simGetObjectFromIndex(int objType, int index)
{
    return (_simGetObjectFromIndex_internal(objType, index));
}

SIM_DLLEXPORT int _simGetObjectID(const void* object)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(_simGetObjectID_internal(object));
}

SIM_DLLEXPORT int _simGetObjectType(const void* object)
{
    return (_simGetObjectType_internal(object));
}

SIM_DLLEXPORT const void** _simGetObjectChildren(const void* object, int* count)
{
    return (_simGetObjectChildren_internal(object, count));
}

SIM_DLLEXPORT const void* _simGetParentObject(const void* object)
{
    return (_simGetParentObject_internal(object));
}

SIM_DLLEXPORT const void* _simGetObject(int objID)
{ // backw. compatibility version
    return _simGetObject_internal(App::getNewHandleFromOldHandle(objID));
}

SIM_DLLEXPORT bool _simIsShapeDynamicallyStatic(const void* shape)
{
    return (_simIsShapeDynamicallyStatic_internal(shape));
}

SIM_DLLEXPORT int _simGetTreeDynamicProperty(const void* object)
{
    return (_simGetTreeDynamicProperty_internal(object));
}

SIM_DLLEXPORT int _simGetDummyLinkType(const void* dummy, int* linkedDummyID)
{ // backw. compatibility version
    UID ld;
    int retVal = _simGetDummyLinkType_internal(dummy, &ld);
    if (linkedDummyID != nullptr)
        linkedDummyID[0] = App::getOldHandleFromNewHandle(ld);
    return retVal;
}

SIM_DLLEXPORT int _simGetJointMode(const void* joint)
{
    return (_simGetJointMode_internal(joint));
}

SIM_DLLEXPORT bool _simIsJointInHybridOperation(const void* joint)
{
    return (_simIsJointInHybridOperation_internal(joint));
}

SIM_DLLEXPORT void _simDisableDynamicTreeForManipulation(const void* object, bool disableFlag)
{
    return (_simDisableDynamicTreeForManipulation_internal(object, disableFlag));
}

SIM_DLLEXPORT bool _simIsShapeDynamicallyRespondable(const void* shape)
{
    return (_simIsShapeDynamicallyRespondable_internal(shape));
}

SIM_DLLEXPORT int _simGetDynamicCollisionMask(const void* shape)
{
    return (_simGetDynamicCollisionMask_internal(shape));
}

SIM_DLLEXPORT const void* _simGetLastParentForLocalGlobalCollidable(const void* shape)
{
    return (_simGetLastParentForLocalGlobalCollidable_internal(shape));
}

SIM_DLLEXPORT bool _simGetStartSleeping(const void* shape)
{
    return (_simGetStartSleeping_internal(shape));
}

SIM_DLLEXPORT bool _simGetWasPutToSleepOnce(const void* shape)
{
    return (_simGetWasPutToSleepOnce_internal(shape));
}

SIM_DLLEXPORT bool _simGetDynamicsFullRefreshFlag(const void* object)
{
    return (_simGetDynamicsFullRefreshFlag_internal(object));
}

SIM_DLLEXPORT void _simSetDynamicsFullRefreshFlag(const void* object, bool flag)
{
    return (_simSetDynamicsFullRefreshFlag_internal(object, flag));
}

SIM_DLLEXPORT void _simClearAdditionalForceAndTorque(const void* shape)
{
    return (_simClearAdditionalForceAndTorque_internal(shape));
}

SIM_DLLEXPORT int _simGetJointType(const void* joint)
{
    return (_simGetJointType_internal(joint));
}

SIM_DLLEXPORT const void* _simGetGeomWrapFromGeomProxy(const void* geomData)
{
    return (_simGetGeomWrapFromGeomProxy_internal(geomData));
}

SIM_DLLEXPORT int _simGetPurePrimitiveType(const void* geomInfo)
{
    return (_simGetPurePrimitiveType_internal(geomInfo));
}

SIM_DLLEXPORT bool _simIsGeomWrapGeometric(const void* geomInfo)
{
    return (_simIsGeomWrapGeometric_internal(geomInfo));
}

SIM_DLLEXPORT bool _simIsGeomWrapConvex(const void* geomInfo)
{
    return (_simIsGeomWrapConvex_internal(geomInfo));
}

SIM_DLLEXPORT int _simGetGeometricCount(const void* geomInfo)
{
    return (_simGetGeometricCount_internal(geomInfo));
}

SIM_DLLEXPORT void _simGetAllGeometrics(const void* geomInfo, void** allGeometrics)
{
    return (_simGetAllGeometrics_internal(geomInfo, allGeometrics));
}

SIM_DLLEXPORT void _simMakeDynamicAnnouncement(int announceType)
{
    return (_simMakeDynamicAnnouncement_internal(announceType));
}

SIM_DLLEXPORT int _simGetTimeDiffInMs(int previousTime)
{
    return (_simGetTimeDiffInMs_internal(previousTime));
}

SIM_DLLEXPORT bool _simDoEntitiesCollide(int entity1ID, int entity2ID, int* cacheBuffer, bool overrideCollidableFlagIfShape1, bool overrideCollidableFlagIfShape2, bool pathOrMotionPlanningRoutineCalling)
{ // backw. compatibility version
    UID* __cacheBuffer = nullptr;
    UID _cacheBuffer[4];
    if (cacheBuffer != nullptr)
    {
        _cacheBuffer[0] = App::getNewHandleFromOldHandle(cacheBuffer[0]);
        _cacheBuffer[1] = cacheBuffer[1];
        _cacheBuffer[2] = App::getNewHandleFromOldHandle(cacheBuffer[2]);
        _cacheBuffer[3] = cacheBuffer[3];
        __cacheBuffer = _cacheBuffer;
    }
    bool retVal = _simDoEntitiesCollide_internal(App::getNewHandleFromOldHandle(entity1ID), App::getNewHandleFromOldHandle(entity2ID), __cacheBuffer, overrideCollidableFlagIfShape1, overrideCollidableFlagIfShape2, pathOrMotionPlanningRoutineCalling);
    if (cacheBuffer != nullptr)
    {
        cacheBuffer[0] = App::getOldHandleFromNewHandle(_cacheBuffer[0]);
        cacheBuffer[1] = _cacheBuffer[1];
        cacheBuffer[2] = App::getOldHandleFromNewHandle(_cacheBuffer[2]);
        cacheBuffer[3] = _cacheBuffer[3];
        __cacheBuffer = _cacheBuffer;
    }
    return retVal;
}

SIM_DLLEXPORT int simFloatingViewRemove(int floatingViewHandle)
{
    return (simFloatingViewRemove_internal(floatingViewHandle));
}

SIM_DLLEXPORT const void* _simGetIkGroupObject(int ikGroupID)
{
    return (_simGetIkGroupObject_internal(ikGroupID));
}

SIM_DLLEXPORT int _simMpHandleIkGroupObject(const void* ikGroup)
{
    return (_simMpHandleIkGroupObject_internal(ikGroup));
}

SIM_DLLEXPORT float* simGetVisionSensorDepth(int sensorHandle, int options, const int* pos, const int* size, int* resolution)
{ // backw. compatibility version
    return simGetVisionSensorDepth_internal(App::getNewHandleFromOldHandle(sensorHandle), options, pos, size, resolution);
}

SIM_DLLEXPORT int _simSetVisionSensorDepth(int sensorHandle, int options, const float* depth)
{ // backw. compatibility version
    return _simSetVisionSensorDepth_internal(App::getNewHandleFromOldHandle(sensorHandle), options, depth);
}

SIM_DLLEXPORT float* simCheckVisionSensorEx(int visionSensorHandle, int entityHandle, bool returnImage)
{ // backw. compatibility version
    return simCheckVisionSensorEx_internal(App::getNewHandleFromOldHandle(visionSensorHandle), App::getNewHandleFromOldHandle(entityHandle), returnImage);
}

SIM_DLLEXPORT int simCreateScript(int scriptType, const char* scriptText, int options, const char* lang)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simCreateScript_internal(scriptType, scriptText, options, lang));
}

SIM_DLLEXPORT int simGetObjectHierarchyOrder(int objectHandle, int* totalSiblings)
{ // backw. compatibility version
    return simGetObjectHierarchyOrder_internal(App::getNewHandleFromOldHandle(objectHandle), totalSiblings);
}

SIM_DLLEXPORT int simSetObjectHierarchyOrder(int objectHandle, int order)
{ // backw. compatibility version
    return simSetObjectHierarchyOrder_internal(App::getNewHandleFromOldHandle(objectHandle), order);
}

SIM_DLLEXPORT int simGetShapeViz_D(int shapeHandle, int index, struct SShapeVizInfo* info)
{ // backw. compatibility version
    return simGetShapeViz_internal(App::getNewHandleFromOldHandle(shapeHandle), index, info);
}

SIM_DLLEXPORT int simGetObjectMatrix_D(int objectHandle, int relativeToObjectHandle, double* matrix)
{ // backw. compatibility version
    return simGetObjectMatrix_internal(App::getNewHandleFromOldHandle(objectHandle), App::getNewHandleFromOldHandle(relativeToObjectHandle), matrix);
}

SIM_DLLEXPORT int simSetObjectMatrix_D(int objectHandle, int relativeToObjectHandle, const double* matrix)
{ // backw. compatibility version
    return simSetObjectMatrix_internal(App::getNewHandleFromOldHandle(objectHandle), App::getNewHandleFromOldHandle(relativeToObjectHandle), matrix);
}

SIM_DLLEXPORT int simGetObjectPose_D(int objectHandle, int relativeToObjectHandle, double* pose)
{ // backw. compatibility version
    return simGetObjectPose_internal(App::getNewHandleFromOldHandle(objectHandle), App::getNewHandleFromOldHandle(relativeToObjectHandle), pose);
}

SIM_DLLEXPORT int simSetObjectPose_D(int objectHandle, int relativeToObjectHandle, const double* pose)
{ // backw. compatibility version
    return simSetObjectPose_internal(App::getNewHandleFromOldHandle(objectHandle), App::getNewHandleFromOldHandle(relativeToObjectHandle), pose);
}

SIM_DLLEXPORT int simGetObjectPosition_D(int objectHandle, int relativeToObjectHandle, double* position)
{ // backw. compatibility version
    return simGetObjectPosition_internal(App::getNewHandleFromOldHandle(objectHandle), App::getNewHandleFromOldHandle(relativeToObjectHandle), position);
}

SIM_DLLEXPORT int simSetObjectPosition_D(int objectHandle, int relativeToObjectHandle, const double* position)
{ // backw. compatibility version
    return simSetObjectPosition_internal(App::getNewHandleFromOldHandle(objectHandle), App::getNewHandleFromOldHandle(relativeToObjectHandle), position);
}

SIM_DLLEXPORT int simGetObjectOrientation_D(int objectHandle, int relativeToObjectHandle, double* eulerAngles)
{ // backw. compatibility version
    return simGetObjectOrientation_internal(App::getNewHandleFromOldHandle(objectHandle), App::getNewHandleFromOldHandle(relativeToObjectHandle), eulerAngles);
}

SIM_DLLEXPORT int simGetObjectQuaternion_D(int objectHandle, int relativeToObjectHandle, double* quaternion)
{ // backw. compatibility version
    return simGetObjectQuaternion_internal(App::getNewHandleFromOldHandle(objectHandle), App::getNewHandleFromOldHandle(relativeToObjectHandle), quaternion);
}

SIM_DLLEXPORT int simSetObjectQuaternion_D(int objectHandle, int relativeToObjectHandle, const double* quaternion)
{ // backw. compatibility version
    return simSetObjectQuaternion_internal(App::getNewHandleFromOldHandle(objectHandle), App::getNewHandleFromOldHandle(relativeToObjectHandle), quaternion);
}

SIM_DLLEXPORT int simSetObjectOrientation_D(int objectHandle, int relativeToObjectHandle, const double* eulerAngles)
{ // backw. compatibility version
    return simSetObjectOrientation_internal(App::getNewHandleFromOldHandle(objectHandle), App::getNewHandleFromOldHandle(relativeToObjectHandle), eulerAngles);
}

SIM_DLLEXPORT int simGetJointPosition_D(int objectHandle, double* position)
{ // backw. compatibility version
    return simGetJointPosition_internal(App::getNewHandleFromOldHandle(objectHandle), position);
}

SIM_DLLEXPORT int simSetJointPosition_D(int objectHandle, double position)
{ // backw. compatibility version
    return simSetJointPosition_internal(App::getNewHandleFromOldHandle(objectHandle), position);
}

SIM_DLLEXPORT int simSetJointTargetPosition_D(int objectHandle, double targetPosition)
{ // backw. compatibility version
    return simSetJointTargetPosition_internal(App::getNewHandleFromOldHandle(objectHandle), targetPosition);
}

SIM_DLLEXPORT int simGetJointTargetPosition_D(int objectHandle, double* targetPosition)
{ // backw. compatibility version
    return simGetJointTargetPosition_internal(App::getNewHandleFromOldHandle(objectHandle), targetPosition);
}

SIM_DLLEXPORT int simGetObjectChildPose_D(int objectHandle, double* pose)
{ // backw. compatibility version
    return simGetObjectChildPose_internal(App::getNewHandleFromOldHandle(objectHandle), pose);
}

SIM_DLLEXPORT int simSetObjectChildPose_D(int objectHandle, const double* pose)
{ // backw. compatibility version
    return simSetObjectChildPose_internal(App::getNewHandleFromOldHandle(objectHandle), pose);
}

SIM_DLLEXPORT int simGetJointInterval_D(int objectHandle, bool* cyclic, double* interval)
{ // backw. compatibility version
    return simGetJointInterval_internal(App::getNewHandleFromOldHandle(objectHandle), cyclic, interval);
}

SIM_DLLEXPORT int simSetJointInterval_D(int objectHandle, bool cyclic, const double* interval)
{ // backw. compatibility version
    return simSetJointInterval_internal(App::getNewHandleFromOldHandle(objectHandle), cyclic, interval);
}

SIM_DLLEXPORT int simBuildIdentityMatrix_D(double* matrix)
{
    return (simBuildIdentityMatrix_internal(matrix));
}

SIM_DLLEXPORT int simBuildMatrix_D(const double* position, const double* eulerAngles, double* matrix)
{
    return (simBuildMatrix_internal(position, eulerAngles, matrix));
}

SIM_DLLEXPORT int simBuildPose_D(const double* position, const double* eulerAngles, double* pose)
{
    return (simBuildPose_internal(position, eulerAngles, pose));
}

SIM_DLLEXPORT int simGetEulerAnglesFromMatrix_D(const double* matrix, double* eulerAngles)
{
    return (simGetEulerAnglesFromMatrix_internal(matrix, eulerAngles));
}

SIM_DLLEXPORT int simInvertMatrix_D(double* matrix)
{
    return (simInvertMatrix_internal(matrix));
}

SIM_DLLEXPORT int simMultiplyMatrices_D(const double* matrixIn1, const double* matrixIn2, double* matrixOut)
{
    return (simMultiplyMatrices_internal(matrixIn1, matrixIn2, matrixOut));
}

SIM_DLLEXPORT int simMultiplyPoses_D(const double* poseIn1, const double* poseIn2, double* poseOut)
{
    return (simMultiplyPoses_internal(poseIn1, poseIn2, poseOut));
}

SIM_DLLEXPORT int simInvertPose_D(double* pose)
{
    return (simInvertPose_internal(pose));
}

SIM_DLLEXPORT int simInterpolatePoses_D(const double* poseIn1, const double* poseIn2, double interpolFactor, double* poseOut)
{
    return (simInterpolatePoses_internal(poseIn1, poseIn2, interpolFactor, poseOut));
}

SIM_DLLEXPORT int simPoseToMatrix_D(const double* poseIn, double* matrixOut)
{
    return (simPoseToMatrix_internal(poseIn, matrixOut));
}

SIM_DLLEXPORT int simMatrixToPose_D(const double* matrixIn, double* poseOut)
{
    return (simMatrixToPose_internal(matrixIn, poseOut));
}

SIM_DLLEXPORT int simInterpolateMatrices_D(const double* matrixIn1, const double* matrixIn2, double interpolFactor, double* matrixOut)
{
    return (simInterpolateMatrices_internal(matrixIn1, matrixIn2, interpolFactor, matrixOut));
}

SIM_DLLEXPORT int simTransformVector_D(const double* matrix, double* vect)
{
    return (simTransformVector_internal(matrix, vect));
}

SIM_DLLEXPORT double simGetSimulationTime_D()
{
    return (simGetSimulationTime_internal());
}

SIM_DLLEXPORT double simGetSystemTime_D()
{
    return (double(simGetSystemTime_internal()));
}

SIM_DLLEXPORT int simHandleProximitySensor_D(int sensorHandle, double* detectedPoint, int* detectedObjectHandle, double* normalVector)
{ // backw. compatibility version
    UID _detectedObjectHandle;
    int retVal = simHandleProximitySensor_internal(App::getNewHandleFromOldHandle(sensorHandle), detectedPoint, &_detectedObjectHandle, normalVector);
    if (detectedObjectHandle != nullptr)
        detectedObjectHandle[0] = App::getOldHandleFromNewHandle(_detectedObjectHandle);
    return retVal;
}

SIM_DLLEXPORT int simReadProximitySensor_D(int sensorHandle, double* detectedPoint, int* detectedObjectHandle, double* normalVector)
{ // backw. compatibility version
    UID _detectedObjectHandle;
    int retVal = simReadProximitySensor_internal(App::getNewHandleFromOldHandle(sensorHandle), detectedPoint, &_detectedObjectHandle, normalVector);
    if (detectedObjectHandle != nullptr)
        detectedObjectHandle[0] = App::getOldHandleFromNewHandle(_detectedObjectHandle);
    return retVal;
}

SIM_DLLEXPORT int simHandleDynamics_D(double deltaTime)
{
    return (simHandleDynamics_internal(deltaTime));
}

SIM_DLLEXPORT int simCheckProximitySensor_D(int sensorHandle, int entityHandle, double* detectedPoint)
{ // backw. compatibility version
    return simCheckProximitySensor_internal(App::getNewHandleFromOldHandle(sensorHandle), App::getNewHandleFromOldHandle(entityHandle), detectedPoint);
}

SIM_DLLEXPORT int simCheckProximitySensorEx_D(int sensorHandle, int entityHandle, int detectionMode, double detectionThreshold, double maxAngle, double* detectedPoint, int* detectedObjectHandle, double* normalVector)
{ // backw. compatibility version
    UID _detectedObjectHandle;
    int retVal = simCheckProximitySensorEx_internal(App::getNewHandleFromOldHandle(sensorHandle), App::getNewHandleFromOldHandle(entityHandle), detectionMode, detectionThreshold, maxAngle, detectedPoint, &_detectedObjectHandle, normalVector);
    if (detectedObjectHandle != nullptr)
        detectedObjectHandle[0] = App::getOldHandleFromNewHandle(_detectedObjectHandle);
    return retVal;
}

SIM_DLLEXPORT int simCheckProximitySensorEx2_D(int sensorHandle, double* vertexPointer, int itemType, int itemCount, int detectionMode, double detectionThreshold, double maxAngle, double* detectedPoint, double* normalVector)
{ // backw. compatibility version
    return simCheckProximitySensorEx2_internal(App::getNewHandleFromOldHandle(sensorHandle), vertexPointer, itemType, itemCount, detectionMode, detectionThreshold, maxAngle, detectedPoint, normalVector);
}

SIM_DLLEXPORT int simCheckCollisionEx_D(int entity1Handle, int entity2Handle, double** intersectionSegments)
{ // backw. compatibility version
    return simCheckCollisionEx_internal(App::getNewHandleFromOldHandle(entity1Handle), App::getNewHandleFromOldHandle(entity2Handle), intersectionSegments);
}

SIM_DLLEXPORT int simCheckDistance_D(int entity1Handle, int entity2Handle, double threshold, double* distanceData)
{ // backw. compatibility version
    return simCheckDistance_internal(App::getNewHandleFromOldHandle(entity1Handle), App::getNewHandleFromOldHandle(entity2Handle), threshold, distanceData);
}

SIM_DLLEXPORT int simSetSimulationTimeStep_D(double timeStep)
{
    return (simSetSimulationTimeStep_internal(timeStep));
}

SIM_DLLEXPORT double simGetSimulationTimeStep_D()
{
    return (simGetSimulationTimeStep_internal());
}

SIM_DLLEXPORT int simFloatingViewAdd_D(double posX, double posY, double sizeX, double sizeY, int options)
{
    return (simFloatingViewAdd_internal(posX, posY, sizeX, sizeY, options));
}

SIM_DLLEXPORT int simFloatingViewRemove_D(int floatingViewHandle)
{
    return (simFloatingViewRemove_internal(floatingViewHandle));
}

SIM_DLLEXPORT int simHandleGraph_D(int graphHandle, double simulationTime)
{ // backw. compatibility version
    return simHandleGraph_internal(App::getNewHandleFromOldHandle(graphHandle), simulationTime);
}

SIM_DLLEXPORT int simAddGraphStream_D(int graphHandle, const char* streamName, const char* unitStr, int options, const float* color, double cyclicRange)
{ // backw. compatibility version
    return simAddGraphStream_internal(App::getNewHandleFromOldHandle(graphHandle), streamName, unitStr, options, color, cyclicRange);
}

SIM_DLLEXPORT int simSetGraphStreamTransformation_D(int graphHandle, int streamId, int trType, double mult, double off, int movingAvgPeriod)
{ // backw. compatibility version
    return simSetGraphStreamTransformation_internal(App::getNewHandleFromOldHandle(graphHandle), streamId, trType, mult, off, movingAvgPeriod);
}

SIM_DLLEXPORT int simAddGraphCurve_D(int graphHandle, const char* curveName, int dim, const int* streamIds, const double* defaultValues, const char* unitStr, int options, const float* color, int curveWidth)
{ // backw. compatibility version
    return simAddGraphCurve_internal(App::getNewHandleFromOldHandle(graphHandle), curveName, dim, streamIds, defaultValues, unitStr, options, color, curveWidth);
}

SIM_DLLEXPORT int simSetGraphStreamValue_D(int graphHandle, int streamId, double value)
{ // backw. compatibility version
    return simSetGraphStreamValue_internal(App::getNewHandleFromOldHandle(graphHandle), streamId, value);
}

SIM_DLLEXPORT int simSetJointTargetVelocity_D(int objectHandle, double targetVelocity)
{ // backw. compatibility version
    return simSetJointTargetVelocity_internal(App::getNewHandleFromOldHandle(objectHandle), targetVelocity);
}

SIM_DLLEXPORT int simGetJointTargetVelocity_D(int objectHandle, double* targetVelocity)
{ // backw. compatibility version
    return simGetJointTargetVelocity_internal(App::getNewHandleFromOldHandle(objectHandle), targetVelocity);
}

SIM_DLLEXPORT int simScaleObjects_D(const int* objectHandles, int objectCount, double scalingFactor, bool scalePositionsToo)
{ // backw. compatibility version
    std::vector<UID> v(objectCount);
    for (size_t i = 0; i < objectCount; i++)
        v[i] = App::getNewHandleFromOldHandle(objectHandles[i]);
    return simScaleObjects_internal(v.data(), objectCount, scalingFactor, scalePositionsToo);
}

SIM_DLLEXPORT int simAddDrawingObject_D(int objectType, double size, double duplicateTolerance, int parentObjectHandle, int maxItemCount, const float* color, const float* setToNULL, const float* setToNULL2, const float* setToNULL3)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simAddDrawingObject_internal(objectType, size, duplicateTolerance, App::getNewHandleFromOldHandle(parentObjectHandle), maxItemCount, color, setToNULL, setToNULL2, setToNULL3));
}

SIM_DLLEXPORT int simAddDrawingObjectItem_D(int objectHandle, const double* itemData)
{ // backw. compatibility version
    return simAddDrawingObjectItem_internal(App::getNewHandleFromOldHandle(objectHandle), itemData);
}

SIM_DLLEXPORT double simGetObjectSizeFactor_D(int objectHandle)
{ // backw. compatibility version
    return simGetObjectSizeFactor_internal(App::getNewHandleFromOldHandle(objectHandle));
}

SIM_DLLEXPORT int simReadForceSensor_D(int objectHandle, double* forceVector, double* torqueVector)
{ // backw. compatibility version
    return simReadForceSensor_internal(App::getNewHandleFromOldHandle(objectHandle), forceVector, torqueVector);
}

SIM_DLLEXPORT int simGetVelocity_D(int shapeHandle, double* linearVelocity, double* angularVelocity)
{ // backw. compatibility version
    return simGetVelocity_internal(App::getNewHandleFromOldHandle(shapeHandle), linearVelocity, angularVelocity);
}

SIM_DLLEXPORT int simGetObjectVelocity_D(int objectHandle, double* linearVelocity, double* angularVelocity)
{ // backw. compatibility version
    return simGetObjectVelocity_internal(App::getNewHandleFromOldHandle(objectHandle), linearVelocity, angularVelocity);
}

SIM_DLLEXPORT int simGetJointVelocity_D(int jointHandle, double* velocity)
{ // backw. compatibility version
    return simGetJointVelocity_internal(App::getNewHandleFromOldHandle(jointHandle), velocity);
}

SIM_DLLEXPORT int simAddForceAndTorque_D(int shapeHandle, const double* force, const double* torque)
{ // backw. compatibility version
    return simAddForceAndTorque_internal(App::getNewHandleFromOldHandle(shapeHandle), force, torque);
}

SIM_DLLEXPORT int simAddForce_D(int shapeHandle, const double* position, const double* force)
{ // backw. compatibility version
    return simAddForce_internal(App::getNewHandleFromOldHandle(shapeHandle), position, force);
}

SIM_DLLEXPORT int simSetObjectColor(int objectHandle, int index, int colorComponent, const float* rgbData)
{ // backw. compatibility version
    return simSetObjectColor_internal(App::getNewHandleFromOldHandle(objectHandle), index, colorComponent, rgbData);
}

SIM_DLLEXPORT int simGetObjectColor(int objectHandle, int index, int colorComponent, float* rgbData)
{ // backw. compatibility version
    return simGetObjectColor_internal(App::getNewHandleFromOldHandle(objectHandle), index, colorComponent, rgbData);
}

SIM_DLLEXPORT int simSetShapeColor(int shapeHandle, const char* colorName, int colorComponent, const float* rgbData)
{ // backw. compatibility version
    return simSetShapeColor_internal(App::getNewHandleFromOldHandle(shapeHandle), colorName, colorComponent, rgbData);
}

SIM_DLLEXPORT int simGetShapeColor(int shapeHandle, const char* colorName, int colorComponent, float* rgbData)
{ // backw. compatibility version
    return simGetShapeColor_internal(App::getNewHandleFromOldHandle(shapeHandle), colorName, colorComponent, rgbData);
}

SIM_DLLEXPORT int simGetContactInfo_D(int dynamicPass, int objectHandle, int index, int* objectHandles, double* contactInfo)
{ // backw. compatibility version
    UID _objectHandles[2];
    int retVal = simGetContactInfo_internal(dynamicPass, App::getNewHandleFromOldHandle(objectHandle), index, _objectHandles, contactInfo);
    if (objectHandles != nullptr)
    {
        objectHandles[0] = App::getOldHandleFromNewHandle(_objectHandles[0]);
        objectHandles[1] = App::getOldHandleFromNewHandle(_objectHandles[1]);
    }
    return retVal;
}

SIM_DLLEXPORT int simAuxiliaryConsoleOpen(const char* title, int maxLines, int mode, const int* position, const int* size, const float* textColor, const float* backgroundColor)
{
    return (simAuxiliaryConsoleOpen_internal(title, maxLines, mode, position, size, textColor, backgroundColor));
}

SIM_DLLEXPORT int simImportShape_D(int fileformat, const char* pathAndFilename, int options, double identicalVerticeTolerance, double scalingFactor)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simImportShape_internal(fileformat, pathAndFilename, options, identicalVerticeTolerance, scalingFactor));
}

SIM_DLLEXPORT int simImportMesh_D(int fileformat, const char* pathAndFilename, int options, double identicalVerticeTolerance, double scalingFactor, double*** vertices, int** verticesSizes, int*** indices, int** indicesSizes, double*** reserved, char*** names)
{
    return (simImportMesh_internal(fileformat, pathAndFilename, options, identicalVerticeTolerance, scalingFactor, vertices, verticesSizes, indices, indicesSizes, reserved, names));
}

SIM_DLLEXPORT int simExportMesh_D(int fileformat, const char* pathAndFilename, int options, double scalingFactor, int elementCount, const double** vertices, const int* verticesSizes, const int** indices, const int* indicesSizes, double** reserved, const char** names)
{
    return (simExportMesh_internal(fileformat, pathAndFilename, options, scalingFactor, elementCount, vertices, verticesSizes, indices, indicesSizes, reserved, names));
}

SIM_DLLEXPORT int simCreateMeshShape_D(int options, double shadingAngle, const double* vertices, int verticesSize, const int* indices, int indicesSize, double* reserved)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simCreateMeshShape_internal(options, shadingAngle, vertices, verticesSize, indices, indicesSize, reserved));
}

SIM_DLLEXPORT int simCreatePrimitiveShape_D(int primitiveType, const double* sizes, int options)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simCreatePrimitiveShape_internal(primitiveType, sizes, options));
}

SIM_DLLEXPORT int simCreateHeightfieldShape_D(int options, double shadingAngle, int xPointCount, int yPointCount, double xSize, const double* heights)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simCreateHeightfieldShape_internal(options, shadingAngle, xPointCount, yPointCount, xSize, heights));
}

SIM_DLLEXPORT int simGetShapeMesh_D(int shapeHandle, double** vertices, int* verticesSize, int** indices, int* indicesSize, double** normals)
{ // backw. compatibility version
    return simGetShapeMesh_internal(App::getNewHandleFromOldHandle(shapeHandle), vertices, verticesSize, indices, indicesSize, normals);
}

SIM_DLLEXPORT int simCreateJoint_D(int jointType, int jointMode, int options, const double* sizes, const double* reservedA, const double* reservedB)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simCreateJoint_internal(jointType, jointMode, options, sizes, reservedA, reservedB));
}

SIM_DLLEXPORT int simGetRotationAxis_D(const double* matrixStart, const double* matrixGoal, double* axis, double* angle)
{
    return (simGetRotationAxis_internal(matrixStart, matrixGoal, axis, angle));
}

SIM_DLLEXPORT int simRotateAroundAxis_D(const double* matrixIn, const double* axis, const double* axisPos, double angle, double* matrixOut)
{
    return (simRotateAroundAxis_internal(matrixIn, axis, axisPos, angle, matrixOut));
}

SIM_DLLEXPORT int simGetJointForce_D(int jointHandle, double* forceOrTorque)
{ // backw. compatibility version
    return simGetJointForce_internal(App::getNewHandleFromOldHandle(jointHandle), forceOrTorque);
}

SIM_DLLEXPORT int simGetJointTargetForce_D(int jointHandle, double* forceOrTorque)
{ // backw. compatibility version
    return simGetJointTargetForce_internal(App::getNewHandleFromOldHandle(jointHandle), forceOrTorque);
}

SIM_DLLEXPORT int simSetJointTargetForce_D(int objectHandle, double forceOrTorque, bool signedValue)
{ // backw. compatibility version
    return simSetJointTargetForce_internal(App::getNewHandleFromOldHandle(objectHandle), forceOrTorque, signedValue);
}

SIM_DLLEXPORT int simCameraFitToView_D(int viewHandleOrIndex, int objectCount, const int* objectHandles, int options, double scaling)
{ // backw. compatibility version
    std::vector<UID> v(objectCount);
    for (size_t i = 0; i < objectCount; i++)
        v[i] = App::getNewHandleFromOldHandle(objectHandles[i]);
    return simCameraFitToView_internal(viewHandleOrIndex, objectCount, v.data(), options, scaling);
}

SIM_DLLEXPORT int simHandleVisionSensor_D(int visionSensorHandle, double** auxValues, int** auxValuesCount)
{ // backw. compatibility version
    return simHandleVisionSensor_internal(App::getNewHandleFromOldHandle(visionSensorHandle), auxValues, auxValuesCount);
}

SIM_DLLEXPORT int simReadVisionSensor_D(int visionSensorHandle, double** auxValues, int** auxValuesCount)
{ // backw. compatibility version
    return simReadVisionSensor_internal(App::getNewHandleFromOldHandle(visionSensorHandle), auxValues, auxValuesCount);
}

SIM_DLLEXPORT int simCheckVisionSensor_D(int visionSensorHandle, int entityHandle, double** auxValues, int** auxValuesCount)
{ // backw. compatibility version
    return simCheckVisionSensor_internal(App::getNewHandleFromOldHandle(visionSensorHandle), App::getNewHandleFromOldHandle(entityHandle), auxValues, auxValuesCount);
}

SIM_DLLEXPORT unsigned char* simGetVisionSensorImg_D(int sensorHandle, int options, double rgbaCutOff, const int* pos, const int* size, int* resolution)
{ // backw. compatibility version
    return simGetVisionSensorImg_internal(App::getNewHandleFromOldHandle(sensorHandle), options, rgbaCutOff, pos, size, resolution);
}

SIM_DLLEXPORT int simCreateDummy_D(double size, const float* reserved)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simCreateDummy_internal(size, reserved));
}

SIM_DLLEXPORT int simCreateForceSensor_D(int options, const int* intParams, const double* floatParams, const double* reserved)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simCreateForceSensor_internal(options, intParams, floatParams, reserved));
}

SIM_DLLEXPORT int simCreateProximitySensor_D(int sensorType, int subType, int options, const int* intParams, const double* floatParams, const double* reserved)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simCreateProximitySensor_internal(sensorType, subType, options, intParams, floatParams, reserved));
}

SIM_DLLEXPORT int simCreateVisionSensor_D(int options, const int* intParams, const double* floatParams, const double* reserved)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simCreateVisionSensor_internal(options, intParams, floatParams, reserved));
}

SIM_DLLEXPORT int simConvexDecompose_D(int shapeHandle, int options, const int* intParams, const double* floatParams)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simConvexDecompose_internal(App::getNewHandleFromOldHandle(shapeHandle), options, intParams, floatParams));
}

SIM_DLLEXPORT int simCreateTexture_D(const char* fileName, int options, const double* planeSizes, const double* scalingUV, const double* xy_g, int fixedResolution, int* textureId, int* resolution, const void* reserved)
{ // backw. compatibility version
    UID _textureId;
    int retVal = App::getOldHandleFromNewHandle(simCreateTexture_internal(fileName, options, planeSizes, scalingUV, xy_g, fixedResolution, &_textureId, resolution, reserved));
    if (textureId != nullptr)
        textureId[0] = App::getOldHandleFromNewHandle(_textureId);
    return retVal;
}

SIM_DLLEXPORT int simWriteTexture_D(int textureId, int options, const char* data, int posX, int posY, int sizeX, int sizeY, double interpol)
{ // backw. compatibility version
    return simWriteTexture_internal(App::getNewHandleFromOldHandle(textureId), options, data, posX, posY, sizeX, sizeY, interpol);
}

SIM_DLLEXPORT int simGetShapeGeomInfo_D(int shapeHandle, int* intData, double* floatData, void* reserved)
{ // backw. compatibility version
    return simGetShapeGeomInfo_internal(App::getNewHandleFromOldHandle(shapeHandle), intData, floatData, reserved);
}

SIM_DLLEXPORT int simScaleObject_D(int objectHandle, double xScale, double yScale, double zScale, int options)
{ // backw. compatibility version
    return simScaleObject_internal(App::getNewHandleFromOldHandle(objectHandle), xScale, yScale, zScale, options);
}

SIM_DLLEXPORT int simSetShapeTexture_D(int shapeHandle, int textureId, int mappingMode, int options, const double* uvScaling, const double* position, const double* orientation)
{ // backw. compatibility version
    return simSetShapeTexture_internal(App::getNewHandleFromOldHandle(shapeHandle), App::getNewHandleFromOldHandle(textureId), mappingMode, options, uvScaling, position, orientation);
}

SIM_DLLEXPORT int simTransformImage_D(unsigned char* image, const int* resolution, int options, const double* floatParams, const int* intParams, void* reserved)
{
    return (simTransformImage_internal(image, resolution, options, floatParams, intParams, reserved));
}

SIM_DLLEXPORT int simGetQHull_D(const double* inVertices, int inVerticesL, double** verticesOut, int* verticesOutL, int** indicesOut, int* indicesOutL, int reserved1, const double* reserved2)
{
    return (simGetQHull_internal(inVertices, inVerticesL, verticesOut, verticesOutL, indicesOut, indicesOutL, reserved1, reserved2));
}

SIM_DLLEXPORT int simGetDecimatedMesh_D(const double* inVertices, int inVerticesL, const int* inIndices, int inIndicesL, double** verticesOut, int* verticesOutL, int** indicesOut, int* indicesOutL, double decimationPercent, int reserved1, const double* reserved2)
{
    return (simGetDecimatedMesh_internal(inVertices, inVerticesL, inIndices, inIndicesL, verticesOut, verticesOutL, indicesOut, indicesOutL, decimationPercent, reserved1, reserved2));
}

SIM_DLLEXPORT int simComputeMassAndInertia_D(int shapeHandle, double density)
{ // backw. compatibility version
    return simComputeMassAndInertia_internal(App::getNewHandleFromOldHandle(shapeHandle), density);
}

SIM_DLLEXPORT int simCreateOctree_D(double voxelSize, int options, double pointSize, void* reserved)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simCreateOctree_internal(voxelSize, options, pointSize, reserved));
}

SIM_DLLEXPORT int simCreatePointCloud_D(double maxVoxelSize, int maxPtCntPerVoxel, int options, double pointSize, void* reserved)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simCreatePointCloud_internal(maxVoxelSize, maxPtCntPerVoxel, options, pointSize, reserved));
}

SIM_DLLEXPORT int simSetPointCloudOptions_D(int pointCloudHandle, double maxVoxelSize, int maxPtCntPerVoxel, int options, double pointSize, void* reserved)
{ // backw. compatibility version
    return simSetPointCloudOptions_internal(App::getNewHandleFromOldHandle(pointCloudHandle), maxVoxelSize, maxPtCntPerVoxel, options, pointSize, reserved);
}

SIM_DLLEXPORT int simGetPointCloudOptions_D(int pointCloudHandle, double* maxVoxelSize, int* maxPtCntPerVoxel, int* options, double* pointSize, void* reserved)
{ // backw. compatibility version
    return simGetPointCloudOptions_internal(App::getNewHandleFromOldHandle(pointCloudHandle), maxVoxelSize, maxPtCntPerVoxel, options, pointSize, reserved);
}

SIM_DLLEXPORT int simInsertVoxelsIntoOctree_D(int octreeHandle, int options, const double* pts, int ptCnt, const unsigned char* color, const unsigned int* tag, void* reserved)
{ // backw. compatibility version
    return simInsertVoxelsIntoOctree_internal(App::getNewHandleFromOldHandle(octreeHandle), options, pts, ptCnt, color, tag, reserved);
}

SIM_DLLEXPORT int simRemoveVoxelsFromOctree_D(int octreeHandle, int options, const double* pts, int ptCnt, void* reserved)
{ // backw. compatibility version
    return simRemoveVoxelsFromOctree_internal(App::getNewHandleFromOldHandle(octreeHandle), options, pts, ptCnt, reserved);
}

SIM_DLLEXPORT int simInsertPointsIntoPointCloud_D(int pointCloudHandle, int options, const double* pts, int ptCnt, const unsigned char* color, void* optionalValues)
{ // backw. compatibility version
    return simInsertPointsIntoPointCloud_internal(App::getNewHandleFromOldHandle(pointCloudHandle), options, pts, ptCnt, color, optionalValues);
}

SIM_DLLEXPORT int simRemovePointsFromPointCloud_D(int pointCloudHandle, int options, const double* pts, int ptCnt, double tolerance, void* reserved)
{ // backw. compatibility version
    return simRemovePointsFromPointCloud_internal(App::getNewHandleFromOldHandle(pointCloudHandle), options, pts, ptCnt, tolerance, reserved);
}

SIM_DLLEXPORT int simIntersectPointsWithPointCloud_D(int pointCloudHandle, int options, const double* pts, int ptCnt, double tolerance, void* reserved)
{ // backw. compatibility version
    return simIntersectPointsWithPointCloud_internal(App::getNewHandleFromOldHandle(pointCloudHandle), options, pts, ptCnt, tolerance, reserved);
}

SIM_DLLEXPORT const double* simGetOctreeVoxels_D(int octreeHandle, int* ptCnt, void* reserved)
{ // backw. compatibility version
    return simGetOctreeVoxels_internal(App::getNewHandleFromOldHandle(octreeHandle), ptCnt, reserved);
}

SIM_DLLEXPORT const double* simGetPointCloudPoints_D(int pointCloudHandle, int* ptCnt, void* reserved)
{ // backw. compatibility version
    return simGetPointCloudPoints_internal(App::getNewHandleFromOldHandle(pointCloudHandle), ptCnt, reserved);
}

SIM_DLLEXPORT int simInsertObjectIntoPointCloud_D(int pointCloudHandle, int objectHandle, int options, double gridSize, const unsigned char* color, void* optionalValues)
{ // backw. compatibility version
    return simInsertObjectIntoPointCloud_internal(App::getNewHandleFromOldHandle(pointCloudHandle), App::getNewHandleFromOldHandle(objectHandle), options, gridSize, color, optionalValues);
}

SIM_DLLEXPORT int simSubtractObjectFromPointCloud_D(int pointCloudHandle, int objectHandle, int options, double tolerance, void* reserved)
{ // backw. compatibility version
    return simSubtractObjectFromPointCloud_internal(App::getNewHandleFromOldHandle(pointCloudHandle), App::getNewHandleFromOldHandle(objectHandle), options, tolerance, reserved);
}

SIM_DLLEXPORT int simCheckOctreePointOccupancy_D(int octreeHandle, int options, const double* points, int ptCnt, unsigned int* tag, unsigned long long int* location, void* reserved)
{ // backw. compatibility version
    return simCheckOctreePointOccupancy_internal(App::getNewHandleFromOldHandle(octreeHandle), options, points, ptCnt, tag, location, reserved);
}

SIM_DLLEXPORT int simApplyTexture_D(int shapeHandle, const double* textureCoordinates, int textCoordSize, const unsigned char* texture, const int* textureResolution, int options)
{ // backw. compatibility version
    return simApplyTexture_internal(App::getNewHandleFromOldHandle(shapeHandle), textureCoordinates, textCoordSize, texture, textureResolution, options);
}

SIM_DLLEXPORT int simSetJointDependency_D(int jointHandle, int masterJointHandle, double offset, double multCoeff)
{ // backw. compatibility version
    return simSetJointDependency_internal(App::getNewHandleFromOldHandle(jointHandle), App::getNewHandleFromOldHandle(masterJointHandle), offset, multCoeff);
}

SIM_DLLEXPORT int simGetJointDependency_D(int jointHandle, int* masterJointHandle, double* offset, double* multCoeff)
{ // backw. compatibility version
    UID _masterJointHandle;
    int retVal = simGetJointDependency_internal(App::getNewHandleFromOldHandle(jointHandle), &_masterJointHandle, offset, multCoeff);
    if (masterJointHandle != nullptr)
        masterJointHandle[0] = App::getOldHandleFromNewHandle(_masterJointHandle);
    return retVal;
}

SIM_DLLEXPORT int simGetShapeMass_D(int shapeHandle, double* mass)
{ // backw. compatibility version
    return simGetShapeMass_internal(App::getNewHandleFromOldHandle(shapeHandle), mass);
}

SIM_DLLEXPORT int simSetShapeMass_D(int shapeHandle, double mass)
{ // backw. compatibility version
    return simSetShapeMass_internal(App::getNewHandleFromOldHandle(shapeHandle), mass);
}

SIM_DLLEXPORT int simGetShapeInertia_D(int shapeHandle, double* inertiaMatrix, double* transformationMatrix)
{ // backw. compatibility version
    return simGetShapeInertia_internal(App::getNewHandleFromOldHandle(shapeHandle), inertiaMatrix, transformationMatrix);
}

SIM_DLLEXPORT int simSetShapeInertia_D(int shapeHandle, const double* inertiaMatrix, const double* transformationMatrix)
{ // backw. compatibility version
    return simSetShapeInertia_internal(App::getNewHandleFromOldHandle(shapeHandle), inertiaMatrix, transformationMatrix);
}

SIM_DLLEXPORT int simGenerateShapeFromPath_D(const double* path, int pathSize, const double* section, int sectionSize, int options, const double* upVector, double reserved)
{ // backw. compatibility version
    return App::getOldHandleFromNewHandle(simGenerateShapeFromPath_internal(path, pathSize, section, sectionSize, options, upVector, reserved));
}

SIM_DLLEXPORT double simGetClosestPosOnPath_D(const double* path, int pathSize, const double* pathLengths, const double* absPt)
{
    return (simGetClosestPosOnPath_internal(path, pathSize, pathLengths, absPt));
}

SIM_DLLEXPORT void _simGetObjectLocalTransformation_D(const void* object, double* pos, double* quat, bool excludeFirstJointTransformation)
{
    return (_simGetObjectLocalTransformation_internal(object, pos, quat, excludeFirstJointTransformation));
}

SIM_DLLEXPORT void _simSetObjectLocalTransformation_D(void* object, const double* pos, const double* quat, double simTime)
{
    return (_simSetObjectLocalTransformation_internal(object, pos, quat, simTime));
}

SIM_DLLEXPORT void _simDynReportObjectCumulativeTransformation_D(void* object, const double* pos, const double* quat, double simTime)
{
    return (_simDynReportObjectCumulativeTransformation_internal(object, pos, quat, simTime));
}

SIM_DLLEXPORT void _simSetObjectCumulativeTransformation_D(void* object, const double* pos, const double* quat, bool keepChildrenInPlace)
{
    return (_simSetObjectCumulativeTransformation_internal(object, pos, quat, keepChildrenInPlace));
}

SIM_DLLEXPORT void _simGetObjectCumulativeTransformation_D(const void* object, double* pos, double* quat, bool excludeFirstJointTransformation)
{
    return (_simGetObjectCumulativeTransformation_internal(object, pos, quat, excludeFirstJointTransformation));
}

SIM_DLLEXPORT void _simSetJointVelocity_D(const void* joint, double vel)
{
    return (_simSetJointVelocity_internal(joint, vel));
}

SIM_DLLEXPORT void _simSetJointPosition_D(const void* joint, double pos)
{
    return (_simSetJointPosition_internal(joint, pos));
}

SIM_DLLEXPORT double _simGetJointPosition_D(const void* joint)
{
    return (_simGetJointPosition_internal(joint));
}

SIM_DLLEXPORT void _simSetDynamicMotorPositionControlTargetPosition_D(const void* joint, double pos)
{
    return (_simSetDynamicMotorPositionControlTargetPosition_internal(joint, pos));
}

SIM_DLLEXPORT void _simGetInitialDynamicVelocity_D(const void* shape, double* vel)
{
    return (_simGetInitialDynamicVelocity_internal(shape, vel));
}

SIM_DLLEXPORT void _simSetInitialDynamicVelocity_D(void* shape, const double* vel)
{
    return (_simSetInitialDynamicVelocity_internal(shape, vel));
}

SIM_DLLEXPORT void _simGetInitialDynamicAngVelocity_D(const void* shape, double* angularVel)
{
    return (_simGetInitialDynamicAngVelocity_internal(shape, angularVel));
}

SIM_DLLEXPORT void _simSetInitialDynamicAngVelocity_D(void* shape, const double* angularVel)
{
    return (_simSetInitialDynamicAngVelocity_internal(shape, angularVel));
}

SIM_DLLEXPORT void _simSetShapeDynamicVelocity_D(void* shape, const double* linear, const double* angular, double simTime)
{
    return (_simSetShapeDynamicVelocity_internal(shape, linear, angular, simTime));
}

SIM_DLLEXPORT void _simGetAdditionalForceAndTorque_D(const void* shape, double* force, double* torque)
{
    return (_simGetAdditionalForceAndTorque_internal(shape, force, torque));
}

SIM_DLLEXPORT bool _simGetJointPositionInterval_D(const void* joint, double* minValue, double* rangeValue)
{
    return (_simGetJointPositionInterval_internal(joint, minValue, rangeValue));
}

SIM_DLLEXPORT double _simGetDynamicMotorTargetPosition_D(const void* joint)
{
    return (_simGetDynamicMotorTargetPosition_internal(joint));
}

SIM_DLLEXPORT double _simGetDynamicMotorTargetVelocity_D(const void* joint)
{
    return (_simGetDynamicMotorTargetVelocity_internal(joint));
}

SIM_DLLEXPORT double _simGetDynamicMotorMaxForce_D(const void* joint)
{
    return (_simGetDynamicMotorMaxForce_internal(joint));
}

SIM_DLLEXPORT double _simGetDynamicMotorUpperLimitVelocity_D(const void* joint)
{
    return (_simGetDynamicMotorUpperLimitVelocity_internal(joint));
}

SIM_DLLEXPORT void _simSetDynamicMotorReflectedPositionFromDynamicEngine_D(void* joint, double pos, double simTime)
{
    return (_simSetDynamicMotorReflectedPositionFromDynamicEngine_internal(joint, pos, simTime));
}

SIM_DLLEXPORT void _simSetJointSphericalTransformation_D(void* joint, const double* quat, double simTime)
{
    return (_simSetJointSphericalTransformation_internal(joint, quat, simTime));
}

SIM_DLLEXPORT void _simAddForceSensorCumulativeForcesAndTorques_D(void* forceSensor, const double* force, const double* torque, int totalPassesCount, double simTime)
{
    return (_simAddForceSensorCumulativeForcesAndTorques_internal(forceSensor, force, torque, totalPassesCount, simTime));
}

SIM_DLLEXPORT void _simAddJointCumulativeForcesOrTorques_D(void* joint, double forceOrTorque, int totalPassesCount,double simTime)
{
    return (_simAddJointCumulativeForcesOrTorques_internal(joint, forceOrTorque, totalPassesCount, simTime));
}

SIM_DLLEXPORT double _simGetLocalInertiaInfo_D(const void* object, double* pos, double* quat, double* diagI)
{
    return (_simGetLocalInertiaInfo_internal(object, pos, quat, diagI));
}

SIM_DLLEXPORT double _simGetMass_D(const void* geomInfo)
{
    return (_simGetMass_internal(geomInfo));
}

SIM_DLLEXPORT void _simGetPurePrimitiveSizes_D(const void* geometric, double* sizes)
{
    return (_simGetPurePrimitiveSizes_internal(geometric, sizes));
}

SIM_DLLEXPORT void _simGetVerticesLocalFrame_D(const void* shape, const void* geometric, double* pos, double* quat)
{
    return (_simGetVerticesLocalFrame_internal(shape, geometric, pos, quat));
}

SIM_DLLEXPORT const double* _simGetHeightfieldData_D(const void* geometric, int* xCount, int* yCount, double* minHeight, double* maxHeight)
{
    return (_simGetHeightfieldData_internal(geometric, xCount, yCount, minHeight, maxHeight));
}

SIM_DLLEXPORT void _simGetCumulativeMeshes_D(const void* shape, const void* geomInfo, double** vertices, int* verticesSize, int** indices, int* indicesSize)
{
    return (_simGetCumulativeMeshes_internal(shape, geomInfo, vertices, verticesSize, indices, indicesSize));
}

SIM_DLLEXPORT void _simGetGravity_D(double* gravity)
{
    return (_simGetGravity_internal(gravity));
}

SIM_DLLEXPORT bool _simGetDistanceBetweenEntitiesIfSmaller_D(int entity1ID, int entity2ID, double* distance, double* ray, int* cacheBuffer, bool overrideMeasurableFlagIfNonCollection1, bool overrideMeasurableFlagIfNonCollection2,  bool pathPlanningRoutineCalling)
{ // backw. compatibility version
    UID* __cacheBuffer = nullptr;
    UID _cacheBuffer[4];
    if (cacheBuffer != nullptr)
    {
        _cacheBuffer[0] = App::getNewHandleFromOldHandle(cacheBuffer[0]);
        _cacheBuffer[1] = cacheBuffer[1];
        _cacheBuffer[2] = App::getNewHandleFromOldHandle(cacheBuffer[2]);
        _cacheBuffer[3] = cacheBuffer[3];
        __cacheBuffer = _cacheBuffer;
    }

    bool retVal = _simGetDistanceBetweenEntitiesIfSmaller_internal(App::getNewHandleFromOldHandle(entity1ID), App::getNewHandleFromOldHandle(entity2ID), distance, ray, __cacheBuffer, overrideMeasurableFlagIfNonCollection1, overrideMeasurableFlagIfNonCollection2, pathPlanningRoutineCalling);
    if (cacheBuffer != nullptr)
    {
        cacheBuffer[0] = App::getOldHandleFromNewHandle(_cacheBuffer[0]);
        cacheBuffer[1] = _cacheBuffer[1];
        cacheBuffer[2] = App::getOldHandleFromNewHandle(_cacheBuffer[2]);
        cacheBuffer[3] = _cacheBuffer[3];
        __cacheBuffer = _cacheBuffer;
    }
    return retVal;
}

SIM_DLLEXPORT int _simHandleJointControl_D(const void* joint, int auxV, const int* inputValuesInt, const double* inputValuesFloat, double* outputValues)
{
    return (_simHandleJointControl_internal(joint, auxV, inputValuesInt, inputValuesFloat, outputValues));
}

SIM_DLLEXPORT int _simHandleCustomContact_D(int objHandle1, int objHandle2, int engine, int* dataInt, double* dataFloat)
{ // backw. compatibility version
    return _simHandleCustomContact_internal(App::getNewHandleFromOldHandle(objHandle1), App::getNewHandleFromOldHandle(objHandle2), engine, dataInt, dataFloat);
}

SIM_DLLEXPORT double _simGetPureHollowScaling_D(const void* geometric)
{
    return (_simGetPureHollowScaling_internal(geometric));
}

SIM_DLLEXPORT void _simDynCallback_D(const int* intData, const double* floatData)
{
    _simDynCallback_internal(intData, floatData);
}

#include <sim-old.cpp>
