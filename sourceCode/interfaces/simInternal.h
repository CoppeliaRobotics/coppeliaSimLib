#if !defined(simInternal_INCLUDED_)
#define simInternal_INCLUDED_

#include <simLib/simConst.h>
#include <simLib/simTypes.h>
#include <string>
#include <vector>
#include <apiErrors.h>
#include <type_traits>
#include <utility>

namespace CApiUtils {

template <typename Func>
decltype(auto) callCapiAndClearErrors(Func&& f) {
    using ReturnType = decltype(f());

    if constexpr (std::is_void_v<ReturnType>) {
        std::forward<Func>(f)();
        CApiErrors::getAndClearLastError();
    } else {
        auto result = std::forward<Func>(f)();
        CApiErrors::getAndClearLastError();
        return result;
    }
}

template <typename Func>
decltype(auto) callCapi(Func&& f) {
    using ReturnType = decltype(f());

    if constexpr (std::is_void_v<ReturnType>) {
        std::forward<Func>(f)();
    } else {
        return std::forward<Func>(f)();
    }
}
} // namespace CApiUtils

#define CALL_C_API_CLEAR_ERRORS(func, ...) CApiUtils::callCapiAndClearErrors([&]() { return func##_internal(__VA_ARGS__); })
#define CALL_C_API(func, ...) CApiUtils::callCapi([&]() { return func##_internal(__VA_ARGS__); })

void setCurrentScriptInfo_cSide(int scriptHandle, int scriptNameIndex);
int getCurrentScriptNameIndex_cSide();
std::string getIndexAdjustedObjectName(const char* nm);
void setLastInfo(const char* infoStr);

int simSetBoolProperty_internal(long long int target, const char* pName, int pState);
int simGetBoolProperty_internal(long long int target, const char* pName, int* pState);
int simSetIntProperty_internal(long long int target, const char* pName, int pState);
int simGetIntProperty_internal(long long int target, const char* pName, int* pState);
int simSetHandleProperty_internal(long long int target, const char* pName, long long int pState);
int simGetHandleProperty_internal(long long int target, const char* pName, long long int* pState);
int simSetLongProperty_internal(long long int target, const char* pName, long long int pState);
int simGetLongProperty_internal(long long int target, const char* pName, long long int* pState);
int simSetFloatProperty_internal(long long int target, const char* pName, double pState);
int simGetFloatProperty_internal(long long int target, const char* pName, double* pState);
int simSetStringProperty_internal(long long int target, const char* pName, const char* pState);
int simGetStringProperty_internal(long long int target, const char* pName, char** pState);
int simSetTableProperty_internal(long long int target, const char* pName, const char* buffer, int bufferL);
int simGetTableProperty_internal(long long int target, const char* pName, char** buffer, int* bufferL);
int simSetBufferProperty_internal(long long int target, const char* pName, const char* buffer, int bufferL);
int simGetBufferProperty_internal(long long int target, const char* pName, char** buffer, int* bufferL);
int simSetIntArray2Property_internal(long long int target, const char* pName, const int* pState);
int simGetIntArray2Property_internal(long long int target, const char* pName, int* pState);
int simSetVector2Property_internal(long long int target, const char* pName, const double* pState);
int simGetVector2Property_internal(long long int target, const char* pName, double* pState);
int simSetVector3Property_internal(long long int target, const char* pName, const double* pState);
int simGetVector3Property_internal(long long int target, const char* pName, double* pState);
int simSetQuaternionProperty_internal(long long int target, const char* pName, const double* pState);
int simGetQuaternionProperty_internal(long long int target, const char* pName, double* pState);
int simSetPoseProperty_internal(long long int target, const char* pName, const double* pState);
int simGetPoseProperty_internal(long long int target, const char* pName, double* pState);
int simSetColorProperty_internal(long long int target, const char* pName, const float* pState);
int simGetColorProperty_internal(long long int target, const char* pName, float* pState);
int simSetFloatArrayProperty_internal(long long int target, const char* pName, const double* v, int vL);
int simGetFloatArrayProperty_internal(long long int target, const char* pName, double** v, int* vL);
int simSetIntArrayProperty_internal(long long int target, const char* pName, const int* v, int vL);
int simGetIntArrayProperty_internal(long long int target, const char* pName, int** v, int* vL);
int simSetHandleArrayProperty_internal(long long int target, const char* pName, const long long int* v, int vL);
int simGetHandleArrayProperty_internal(long long int target, const char* pName, long long int** v, int* vL);
int simRemoveProperty_internal(long long int target, const char* pName);
char* simGetPropertyName_internal(long long int target, int index, SPropertyOptions* options);
int simGetPropertyInfo_internal(long long int target, const char* pName, SPropertyInfo* infos, SPropertyOptions* options);

void simRegCallback_internal(int index, void* callback);
void simRunGui_internal(int options);
int simInitialize_internal(const char* appDir, int options);
int simDeinitialize_internal();
int simPostExitRequest_internal();
int simGetExitRequest_internal();
int simLoop_internal(void (*callback)(), int options);
int simTest_internal(int mode, void* ptr1, void* ptr2, void* ptr3);
void* simGetMainWindow_internal(int type);
char* simGetLastError_internal();
char* simGetLastInfo_internal();
int simGetObject_internal(const char* objectPath, int index, int proxy, int options);
long long int simGetObjectUid_internal(int objectHandle);
int simGetObjectFromUid_internal(long long int uid, int options);
int simGetScriptHandleEx_internal(int scriptType, int objectHandle, const char* scriptName);
int simRemoveObjects_internal(const int* objectHandles, int count);
int simRemoveModel_internal(int objectHandle);
char* simGetObjectAlias_internal(int objectHandle, int options);
int simSetObjectAlias_internal(int objectHandle, const char* objectAlias, int options);
int simGetObjectMatrix_internal(int objectHandle, int relativeToObjectHandle, double* matrix);
int simSetObjectMatrix_internal(int objectHandle, int relativeToObjectHandle, const double* matrix);
int simGetObjectPose_internal(int objectHandle, int relativeToObjectHandle, double* pose);
int simSetObjectPose_internal(int objectHandle, int relativeToObjectHandle, const double* pose);
int simGetObjectPosition_internal(int objectHandle, int relativeToObjectHandle, double* position);
int simSetObjectPosition_internal(int objectHandle, int relativeToObjectHandle, const double* position);
int simGetObjectOrientation_internal(int objectHandle, int relativeToObjectHandle, double* eulerAngles);
int simGetObjectQuaternion_internal(int objectHandle, int relativeToObjectHandle, double* quaternion);
int simSetObjectQuaternion_internal(int objectHandle, int relativeToObjectHandle, const double* quaternion);
int simSetObjectOrientation_internal(int objectHandle, int relativeToObjectHandle, const double* eulerAngles);
int simGetJointPosition_internal(int objectHandle, double* position);
int simSetJointPosition_internal(int objectHandle, double position);
int simSetJointTargetPosition_internal(int objectHandle, double targetPosition);
int simGetJointTargetPosition_internal(int objectHandle, double* targetPosition);
int simGetObjectChildPose_internal(int objectHandle, double* pose);
int simSetObjectChildPose_internal(int objectHandle, const double* pose);
int simGetJointInterval_internal(int objectHandle, bool* cyclic, double* interval);
int simSetJointInterval_internal(int objectHandle, bool cyclic, const double* interval);
int simGetObjectParent_internal(int objectHandle);
int simGetObjectChild_internal(int objectHandle, int index);
int simSetObjectParent_internal(int objectHandle, int parentObjectHandle, bool keepInPlace);
int simGetObjectType_internal(int objectHandle);
int simGetJointType_internal(int objectHandle);
int simBuildIdentityMatrix_internal(double* matrix);
int simBuildMatrix_internal(const double* position, const double* eulerAngles, double* matrix);
int simBuildPose_internal(const double* position, const double* eulerAngles, double* pose);
int simGetEulerAnglesFromMatrix_internal(const double* matrix, double* eulerAngles);
int simInvertMatrix_internal(double* matrix);
int simMultiplyMatrices_internal(const double* matrixIn1, const double* matrixIn2, double* matrixOut);
int simMultiplyPoses_internal(const double* poseIn1, const double* poseIn2, double* poseOut);
int simInvertPose_internal(double* pose);
int simInterpolatePoses_internal(const double* poseIn1, const double* poseIn2, double interpolFactor, double* poseOut);
int simPoseToMatrix_internal(const double* poseIn, double* matrixOut);
int simMatrixToPose_internal(const double* matrixIn, double* poseOut);
int simInterpolateMatrices_internal(const double* matrixIn1, const double* matrixIn2, double interpolFactor,
                                    double* matrixOut);
int simTransformVector_internal(const double* matrix, double* vect);
int simReservedCommand_internal(int v, int w);
double simGetSimulationTime_internal();
int simGetSimulationState_internal();
double simGetSystemTime_internal();
int simLoadScene_internal(const char* filename);
int simCloseScene_internal();
int simSaveScene_internal(const char* filename);
int simLoadModel_internal(const char* filename);
int simSaveModel_internal(int baseOfModelHandle, const char* filename);
char* simGetSimulatorMessage_internal(int* messageID, int* auxiliaryData, int* returnedDataSize);
int simDoesFileExist_internal(const char* filename);
int* simGetObjectSel_internal(int* cnt);
int simSetObjectSel_internal(const int* handles, int cnt);
int simHandleProximitySensor_internal(int sensorHandle, double* detectedPoint, int* detectedObjectHandle,
                                      double* normalVector);
int simReadProximitySensor_internal(int sensorHandle, double* detectedPoint, int* detectedObjectHandle,
                                    double* normalVector);
int simHandleDynamics_internal(double deltaTime);
int simResetScript_internal(int scriptHandle);
int simRefreshDialogs_internal(int refreshDegree);
int simResetProximitySensor_internal(int sensorHandle);
int simCheckProximitySensor_internal(int sensorHandle, int entityHandle, double* detectedPoint);
int simCheckProximitySensorEx_internal(int sensorHandle, int entityHandle, int detectionMode, double detectionThreshold,
                                       double maxAngle, double* detectedPoint, int* detectedObjectHandle,
                                       double* normalVector);
int simCheckProximitySensorEx2_internal(int sensorHandle, double* vertexPointer, int itemType, int itemCount,
                                        int detectionMode, double detectionThreshold, double maxAngle,
                                        double* detectedPoint, double* normalVector);
void* simCreateBuffer_internal(int size);
int simReleaseBuffer_internal(const void* buffer);
int simCheckCollision_internal(int entity1Handle, int entity2Handle);
int simCheckCollisionEx_internal(int entity1Handle, int entity2Handle, double** intersectionSegments);
int simCheckDistance_internal(int entity1Handle, int entity2Handle, double threshold, double* distanceData);
int simSetSimulationTimeStep_internal(double timeStep);
double simGetSimulationTimeStep_internal();
int simGetRealTimeSimulation_internal();
int simStartSimulation_internal();
int simStopSimulation_internal();
int simPauseSimulation_internal();
char* simGetPluginName_internal(int index, unsigned char* setToNull);
int simSetPluginInfo_internal(const char* pluginName, int infoType, const char* stringInfo, int intInfo);
int simGetPluginInfo_internal(const char* pluginName, int infoType, char** stringInfo, int* intInfo);
int simFloatingViewAdd_internal(double posX, double posY, double sizeX, double sizeY, int options);
int simFloatingViewRemove_internal(int floatingViewHandle);
int simAdjustView_internal(int viewHandleOrIndex, int associatedViewableObjectHandle, int options,
                           const char* viewLabel);
int simSetLastError_internal(const char* setToNullptr, const char* errorMessage);
int simHandleGraph_internal(int graphHandle, double simulationTime);
int simResetGraph_internal(int graphHandle);
int simAddGraphStream_internal(int graphHandle, const char* streamName, const char* unitStr, int options,
                               const float* color, double cyclicRange);
int simDestroyGraphCurve_internal(int graphHandle, int curveId);
int simSetGraphStreamTransformation_internal(int graphHandle, int streamId, int trType, double mult, double off,
                                             int movingAvgPeriod);
int simDuplicateGraphCurveToStatic_internal(int graphHandle, int curveId, const char* curveName);
int simAddGraphCurve_internal(int graphHandle, const char* curveName, int dim, const int* streamIds,
                              const double* defaultValues, const char* unitStr, int options, const float* color,
                              int curveWidth);
int simSetGraphStreamValue_internal(int graphHandle, int streamId, double value);
int simSetNavigationMode_internal(int navigationMode);
int simGetNavigationMode_internal();
int simSetPage_internal(int index);
int simGetPage_internal();
int simRegisterScriptCallbackFunction_internal(const char* func, const char* reserved_setToNull,
                                               void (*callBack)(struct SScriptCallBack* cb));
int simRegisterScriptVariable_internal(const char* var, const char* val, int stackHandle);
int simRegisterScriptFuncHook_internal(int scriptHandle, const char* funcToHook, const char* userFunction,
                                       bool executeBefore, int options);
int simSetJointTargetVelocity_internal(int objectHandle, double targetVelocity);
int simGetJointTargetVelocity_internal(int objectHandle, double* targetVelocity);
int simCopyPasteObjects_internal(int* objectHandles, int objectCount, int options);
int simScaleObjects_internal(const int* objectHandles, int objectCount, double scalingFactor, bool scalePositionsToo);
int simAddDrawingObject_internal(int objectType, double size, double duplicateTolerance, int parentObjectHandle,
                                 int maxItemCount, const float* color, const float* setToNULL, const float* setToNULL2,
                                 const float* setToNULL3);
int simRemoveDrawingObject_internal(int objectHandle);
int simAddDrawingObjectItem_internal(int objectHandle, const double* itemData);
double simGetObjectSizeFactor_internal(int objectHandle);
int simAnnounceSceneContentChange_internal();
int simReadForceSensor_internal(int objectHandle, double* forceVector, double* torqueVector);
int simGetVelocity_internal(int shapeHandle, double* linearVelocity, double* angularVelocity);
int simGetObjectVelocity_internal(int objectHandle, double* linearVelocity, double* angularVelocity);
int simGetJointVelocity_internal(int jointHandle, double* velocity);
int simAddForceAndTorque_internal(int shapeHandle, const double* force, const double* torque);
int simAddForce_internal(int shapeHandle, const double* position, const double* force);
int simSetExplicitHandling_internal(int objectHandle, int explicitFlags);
int simGetExplicitHandling_internal(int objectHandle);
int simGetLinkDummy_internal(int dummyHandle);
int simSetLinkDummy_internal(int dummyHandle, int linkedDummyHandle);
int simSetObjectColor_internal(int objectHandle, int index, int colorComponent, const float* rgbData);
int simGetObjectColor_internal(int objectHandle, int index, int colorComponent, float* rgbData);
int simSetShapeColor_internal(int shapeHandle, const char* colorName, int colorComponent, const float* rgbData);
int simGetShapeColor_internal(int shapeHandle, const char* colorName, int colorComponent, float* rgbData);
int simResetDynamicObject_internal(int objectHandle);
int simSetJointMode_internal(int jointHandle, int jointMode, int options);
int simGetJointMode_internal(int jointHandle, int* options);
int simSerialOpen_internal(const char* portString, int baudRate, void* reserved1, void* reserved2);
int simSerialClose_internal(int portHandle);
int simSerialSend_internal(int portHandle, const char* data, int dataLength);
int simSerialRead_internal(int portHandle, char* buffer, int dataLengthToRead);
int simSerialCheck_internal(int portHandle);
int simGetContactInfo_internal(int dynamicPass, int objectHandle, int index, int* objectHandles, double* contactInfo);
int simAuxiliaryConsoleOpen_internal(const char* title, int maxLines, int mode, const int* position, const int* size,
                                     const float* textColor, const float* backgroundColor);
int simAuxiliaryConsoleClose_internal(int consoleHandle);
int simAuxiliaryConsoleShow_internal(int consoleHandle, bool showState);
int simAuxiliaryConsolePrint_internal(int consoleHandle, const char* text);
int simImportShape_internal(int fileformat, const char* pathAndFilename, int options, double identicalVerticeTolerance,
                            double scalingFactor);
int simImportMesh_internal(int fileformat, const char* pathAndFilename, int options, double identicalVerticeTolerance,
                           double scalingFactor, double*** vertices, int** verticesSizes, int*** indices,
                           int** indicesSizes, double*** reserved, char*** names);
int simExportMesh_internal(int fileformat, const char* pathAndFilename, int options, double scalingFactor,
                           int elementCount, const double** vertices, const int* verticesSizes, const int** indices,
                           const int* indicesSizes, double** reserved, const char** names);
int simCreateShape_internal(int options, double shadingAngle, const double* vertices, int verticesSize,
                            const int* indices, int indicesSize, const double* normals, const float* textureCoords,
                            const unsigned char* texture, const int* textureRes);
int simCreateMeshShape_internal(int options, double shadingAngle, const double* vertices, int verticesSize,
                                const int* indices, int indicesSize, double* reserved);
int simCreatePrimitiveShape_internal(int primitiveType, const double* sizes, int options);
int simCreateHeightfieldShape_internal(int options, double shadingAngle, int xPointCount, int yPointCount, double xSize,
                                       const double* heights);
int simGetShapeMesh_internal(int shapeHandle, double** vertices, int* verticesSize, int** indices, int* indicesSize,
                             double** normals);
int simCreateJoint_internal(int jointType, int jointMode, int options, const double* sizes, const double* reservedA,
                            const double* reservedB);
int simGetRotationAxis_internal(const double* matrixStart, const double* matrixGoal, double* axis, double* angle);
int simRotateAroundAxis_internal(const double* matrixIn, const double* axis, const double* axisPos, double angle,
                                 double* matrixOut);
int simGetJointForce_internal(int jointHandle, double* forceOrTorque);
int simGetJointTargetForce_internal(int jointHandle, double* forceOrTorque);
int simSetJointTargetForce_internal(int objectHandle, double forceOrTorque, bool signedValue);
int simCameraFitToView_internal(int viewHandleOrIndex, int objectCount, const int* objectHandles, int options,
                                double scaling);
int simIsHandle_internal(int generalObjectHandle, int generalObjectType);
int simHandleVisionSensor_internal(int visionSensorHandle, double** auxValues, int** auxValuesCount);
int simReadVisionSensor_internal(int visionSensorHandle, double** auxValues, int** auxValuesCount);
int simResetVisionSensor_internal(int visionSensorHandle);
int simCheckVisionSensor_internal(int visionSensorHandle, int entityHandle, double** auxValues, int** auxValuesCount);
float* simCheckVisionSensorEx_internal(int visionSensorHandle, int entityHandle, bool returnImage);
unsigned char* simGetVisionSensorImg_internal(int sensorHandle, int options, double rgbaCutOff, const int* pos,
                                              const int* size, int* resolution);
int simSetVisionSensorImg_internal(int sensorHandle, const unsigned char* img, int options, const int* pos,
                                   const int* size);
float* simGetVisionSensorDepth_internal(int sensorHandle, int options, const int* pos, const int* size,
                                        int* resolution);
int _simSetVisionSensorDepth_internal(int sensorHandle, int options, const float* depth);
int simRuckigPos_internal(int dofs, double baseCycleTime, int flags, const double* currentPos, const double* currentVel,
                          const double* currentAccel, const double* maxVel, const double* maxAccel,
                          const double* maxJerk, const bool* selection, const double* targetPos,
                          const double* targetVel, double* reserved1, int* reserved2);
int simRuckigVel_internal(int dofs, double baseCycleTime, int flags, const double* currentPos, const double* currentVel,
                          const double* currentAccel, const double* maxAccel, const double* maxJerk,
                          const bool* selection, const double* targetVel, double* reserved1, int* reserved2);
int simRuckigStep_internal(int objHandle, double cycleTime, double* newPos, double* newVel, double* newAccel,
                           double* syncTime, double* reserved1, int* reserved2);
int simRuckigRemove_internal(int objHandle);
int simCreateDummy_internal(double size, const float* reserved);
int simCreateScript_internal(int scriptType, const char* scriptText, int options, const char* lang);
int simGroupShapes_internal(const int* shapeHandles, int shapeCount);
int* simUngroupShape_internal(int shapeHandle, int* shapeCount);
int simCreateProximitySensor_internal(int sensorType, int subType, int options, const int* intParams,
                                      const double* floatParams, const double* reserved);
int simCreateForceSensor_internal(int options, const int* intParams, const double* floatParams, const double* reserved);
int simCreateVisionSensor_internal(int options, const int* intParams, const double* floatParams,
                                   const double* reserved);
int simConvexDecompose_internal(int shapeHandle, int options, const int* intParams, const double* floatParams);
void simQuitSimulator_internal(bool ignoredArgument);
int simSetShapeMaterial_internal(int shapeHandle, int materialIdOrShapeHandle);
int simGetTextureId_internal(const char* textureName, int* resolution);
unsigned char* simReadTexture_internal(int textureId, int options, int posX, int posY, int sizeX, int sizeY);
int simWriteTexture_internal(int textureId, int options, const char* data, int posX, int posY, int sizeX, int sizeY,
                             double interpol);
int simCreateTexture_internal(const char* fileName, int options, const double* planeSizes, const double* scalingUV,
                              const double* xy_g, int fixedResolution, int* textureId, int* resolution,
                              const void* reserved);
int simGetShapeGeomInfo_internal(int shapeHandle, int* intData, double* floatData, void* reserved);
int simGetObjects_internal(int index, int objectType);
int* simGetObjectsInTree_internal(int treeBaseHandle, int objectType, int options, int* objectCount);
int simSetObjectSizeValues_internal(int objectHandle, const double* sizeValues);
int simGetObjectSizeValues_internal(int objectHandle, double* sizeValues);
int simScaleObject_internal(int objectHandle, double xScale, double yScale, double zScale, int options);
int simSetShapeTexture_internal(int shapeHandle, int textureId, int mappingMode, int options, const double* uvScaling,
                                const double* position, const double* orientation);
int simGetShapeTextureId_internal(int shapeHandle);
int simCreateCollectionEx_internal(int options);
int simAddItemToCollection_internal(int collectionHandle, int what, int objectHandle, int options);
int simDestroyCollection_internal(int collectionHandle);
int* simGetCollectionObjects_internal(int collectionHandle, int* objectCount);
int simAlignShapeBB_internal(int shapeHandle, const double* pose);
int simRelocateShapeFrame_internal(int shapeHandle, const double* pose);
int simSaveImage_internal(const unsigned char* image, const int* resolution, int options, const char* filename,
                          int quality, void* reserved);
unsigned char* simLoadImage_internal(int* resolution, int options, const char* filename, void* reserved);
unsigned char* simGetScaledImage_internal(const unsigned char* imageIn, const int* resolutionIn, int* resolutionOut,
                                          int options, void* reserved);
int simTransformImage_internal(unsigned char* image, const int* resolution, int options, const double* floatParams,
                               const int* intParams, void* reserved);
int simGetQHull_internal(const double* inVertices, int inVerticesL, double** verticesOut, int* verticesOutL,
                         int** indicesOut, int* indicesOutL, int reserved1, const double* reserved2);
int simGetDecimatedMesh_internal(const double* inVertices, int inVerticesL, const int* inIndices, int inIndicesL,
                                 double** verticesOut, int* verticesOutL, int** indicesOut, int* indicesOutL,
                                 double decimationPercent, int reserved1, const double* reserved2);
int simCallScriptFunctionEx_internal(int scriptHandleOrType, const char* functionNameAtScriptName, int stackId);
char* simGetExtensionString_internal(int objectHandle, int index, const char* key);
int simComputeMassAndInertia_internal(int shapeHandle, double density);

int simCreateStack_internal();
int simReleaseStack_internal(int stackHandle);
int simCopyStack_internal(int stackHandle);
int simPushNullOntoStack_internal(int stackHandle);
int simPushBoolOntoStack_internal(int stackHandle, bool value);
int simPushInt32OntoStack_internal(int stackHandle, int value);
int simPushInt64OntoStack_internal(int stackHandle, long long int value);
int simPushFloatOntoStack_internal(int stackHandle, float value);
int simPushDoubleOntoStack_internal(int stackHandle, double value);
int simPushTextOntoStack_internal(int stackHandle, const char* value);
int simPushStringOntoStack_internal(int stackHandle, const char* value, int stringSize);
int simPushBufferOntoStack_internal(int stackHandle, const char* value, int stringSize);
int simPushMatrixOntoStack_internal(int stackHandle, const double* value, int rows, int cols);
int simPushUInt8TableOntoStack_internal(int stackHandle, const unsigned char* values, int valueCnt);
int simPushInt32TableOntoStack_internal(int stackHandle, const int* values, int valueCnt);
int simPushInt64TableOntoStack_internal(int stackHandle, const long long int* values, int valueCnt);
int simPushFloatTableOntoStack_internal(int stackHandle, const float* values, int valueCnt);
int simPushDoubleTableOntoStack_internal(int stackHandle, const double* values, int valueCnt);
int simPushTableOntoStack_internal(int stackHandle);
int simInsertDataIntoStackTable_internal(int stackHandle);
int simGetStackSize_internal(int stackHandle);
int simPopStackItem_internal(int stackHandle, int count);
int simMoveStackItemToTop_internal(int stackHandle, int cIndex);
int simGetStackItemType_internal(int stackHandle, int cIndex);
int simGetStackStringType_internal(int stackHandle, int cIndex);
int simGetStackBoolValue_internal(int stackHandle, bool* boolValue);
int simGetStackInt32Value_internal(int stackHandle, int* numberValue);
int simGetStackInt64Value_internal(int stackHandle, long long int* numberValue);
int simGetStackFloatValue_internal(int stackHandle, float* numberValue);
int simGetStackDoubleValue_internal(int stackHandle, double* numberValue);
char* simGetStackStringValue_internal(int stackHandle, int* stringSize);
double* simGetStackMatrix_internal(int stackHandle, int* rows, int* cols);
int simGetStackTableInfo_internal(int stackHandle, int infoType);
int simGetStackUInt8Table_internal(int stackHandle, unsigned char* array, int count);
int simGetStackInt32Table_internal(int stackHandle, int* array, int count);
int simGetStackInt64Table_internal(int stackHandle, long long int* array, int count);
int simGetStackFloatTable_internal(int stackHandle, float* array, int count);
int simGetStackDoubleTable_internal(int stackHandle, double* array, int count);
int simUnfoldStackTable_internal(int stackHandle);
int simDebugStack_internal(int stackHandle, int cIndex);
int simCreateOctree_internal(double voxelSize, int options, double pointSize, void* reserved);
int simCreatePointCloud_internal(double maxVoxelSize, int maxPtCntPerVoxel, int options, double pointSize,
                                 void* reserved);
int simSetPointCloudOptions_internal(int pointCloudHandle, double maxVoxelSize, int maxPtCntPerVoxel, int options,
                                     double pointSize, void* reserved);
int simGetPointCloudOptions_internal(int pointCloudHandle, double* maxVoxelSize, int* maxPtCntPerVoxel, int* options,
                                     double* pointSize, void* reserved);
int simInsertVoxelsIntoOctree_internal(int octreeHandle, int options, const double* pts, int ptCnt,
                                       const unsigned char* color, const unsigned int* tag, void* reserved);
int simRemoveVoxelsFromOctree_internal(int octreeHandle, int options, const double* pts, int ptCnt, void* reserved);
int simInsertPointsIntoPointCloud_internal(int pointCloudHandle, int options, const double* pts, int ptCnt,
                                           const unsigned char* color, void* optionalValues);
int simRemovePointsFromPointCloud_internal(int pointCloudHandle, int options, const double* pts, int ptCnt,
                                           double tolerance, void* reserved);
int simIntersectPointsWithPointCloud_internal(int pointCloudHandle, int options, const double* pts, int ptCnt,
                                              double tolerance, void* reserved);
const double* simGetOctreeVoxels_internal(int octreeHandle, int* ptCnt, void* reserved);
const double* simGetPointCloudPoints_internal(int pointCloudHandle, int* ptCnt, void* reserved);
int simInsertObjectIntoOctree_internal(int octreeHandle, int objectHandle, int options, const unsigned char* color,
                                       unsigned int tag, void* reserved);
int simSubtractObjectFromOctree_internal(int octreeHandle, int objectHandle, int options, void* reserved);
int simInsertObjectIntoPointCloud_internal(int pointCloudHandle, int objectHandle, int options, double gridSize,
                                           const unsigned char* color, void* optionalValues);
int simSubtractObjectFromPointCloud_internal(int pointCloudHandle, int objectHandle, int options, double tolerance,
                                             void* reserved);
int simCheckOctreePointOccupancy_internal(int octreeHandle, int options, const double* points, int ptCnt,
                                          unsigned int* tag, unsigned long long int* location, void* reserved);
char* simOpenTextEditor_internal(const char* initText, const char* xml, int* various);
char* simPackTable_internal(int stackHandle, int* bufferSize);
int simUnpackTable_internal(int stackHandle, const char* buffer, int bufferSize);
int simSetReferencedHandles_internal(int objectHandle, int count, const int* referencedHandles, const char* tag,
                                     const int* reserved2);
int simGetReferencedHandles_internal(int objectHandle, int** referencedHandles, const char* tag, int** reserved2);
int simGetShapeViz_internal(int shapeHandle, int index, struct SShapeVizInfo* info);
int simGetShapeVizf_internal(int shapeHandle, int index, struct SShapeVizInfof* info);
int simExecuteScriptString_internal(int scriptHandle, const char* stringToExecute, int stackHandle);
char* simGetApiFunc_internal(int scriptHandle, const char* apiWord);
char* simGetApiInfo_internal(int scriptHandle, const char* apiWord);
int simEventNotification_internal(const char* event);
int simApplyTexture_internal(int shapeHandle, const double* textureCoordinates, int textCoordSize,
                             const unsigned char* texture, const int* textureResolution, int options);
int simSetJointDependency_internal(int jointHandle, int masterJointHandle, double offset, double multCoeff);
int simGetJointDependency_internal(int jointHandle, int* masterJointHandle, double* offset, double* multCoeff);
int simAddLog_internal(const char* pluginName, int verbosityLevel, const char* logMsg);
int simGetShapeMass_internal(int shapeHandle, double* mass);
int simSetShapeMass_internal(int shapeHandle, double mass);
int simGetShapeInertia_internal(int shapeHandle, double* inertiaMatrix, double* transformationMatrix);
int simSetShapeInertia_internal(int shapeHandle, const double* inertiaMatrix, const double* transformationMatrix);
int simIsDynamicallyEnabled_internal(int objectHandle);
int simGenerateShapeFromPath_internal(const double* path, int pathSize, const double* section, int sectionSize,
                                      int options, const double* upVector, double reserved);
double simGetClosestPosOnPath_internal(const double* path, int pathSize, const double* pathLengths,
                                       const double* absPt);
int simInitScript_internal(int scriptHandle);
int simModuleEntry_internal(int handle, const char* label, int state);
int simCheckExecAuthorization_internal(const char* what, const char* args, int scriptHandle);

void _simSetDynamicSimulationIconCode_internal(void* object, int code);
void _simSetDynamicObjectFlagForVisualization_internal(void* object, int flag);
int _simGetObjectListSize_internal(int objType);
const void* _simGetObjectFromIndex_internal(int objType, int index);
int _simGetObjectID_internal(const void* object);
int _simGetObjectType_internal(const void* object);
const void** _simGetObjectChildren_internal(const void* object, int* count);
const void* _simGetParentObject_internal(const void* object);
const void* _simGetObject_internal(int objID);
void _simGetObjectLocalTransformation_internal(const void* object, double* pos, double* quat,
                                               bool excludeFirstJointTransformation);
void _simSetObjectLocalTransformation_internal(void* object, const double* pos, const double* quat, double simTime);
void _simDynReportObjectCumulativeTransformation_internal(void* object, const double* pos, const double* quat,
                                                          double simTime);
void _simSetObjectCumulativeTransformation_internal(void* object, const double* pos, const double* quat,
                                                    bool keepChildrenInPlace);
void _simGetObjectCumulativeTransformation_internal(const void* object, double* pos, double* quat,
                                                    bool excludeFirstJointTransformation);
bool _simIsShapeDynamicallyStatic_internal(const void* shape);
int _simGetTreeDynamicProperty_internal(const void* object);
int _simGetDummyLinkType_internal(const void* dummy, int* linkedDummyID);
int _simGetJointMode_internal(const void* joint);
bool _simIsJointInHybridOperation_internal(const void* joint);
void _simDisableDynamicTreeForManipulation_internal(const void* object, bool disableFlag);
bool _simIsShapeDynamicallyRespondable_internal(const void* shape);
int _simGetDynamicCollisionMask_internal(const void* shape);
const void* _simGetLastParentForLocalGlobalCollidable_internal(const void* shape);
void _simSetJointVelocity_internal(const void* joint, double vel);
void _simSetJointPosition_internal(const void* joint, double pos);
double _simGetJointPosition_internal(const void* joint);
void _simSetDynamicMotorPositionControlTargetPosition_internal(const void* joint, double pos);
void _simGetInitialDynamicVelocity_internal(const void* shape, double* vel);
void _simSetInitialDynamicVelocity_internal(void* shape, const double* vel);
void _simGetInitialDynamicAngVelocity_internal(const void* shape, double* angularVel);
void _simSetInitialDynamicAngVelocity_internal(void* shape, const double* angularVel);
bool _simGetStartSleeping_internal(const void* shape);
bool _simGetWasPutToSleepOnce_internal(const void* shape);
bool _simGetDynamicsFullRefreshFlag_internal(const void* object);
void _simSetDynamicsFullRefreshFlag_internal(const void* object, bool flag);
void _simSetShapeDynamicVelocity_internal(void* shape, const double* linear, const double* angular, double simTime);
void _simGetAdditionalForceAndTorque_internal(const void* shape, double* force, double* torque);
void _simClearAdditionalForceAndTorque_internal(const void* shape);
bool _simGetJointPositionInterval_internal(const void* joint, double* minValue, double* rangeValue);
int _simGetJointType_internal(const void* joint);
double _simGetDynamicMotorTargetPosition_internal(const void* joint);
double _simGetDynamicMotorTargetVelocity_internal(const void* joint);
double _simGetDynamicMotorMaxForce_internal(const void* joint);
double _simGetDynamicMotorUpperLimitVelocity_internal(const void* joint);
void _simSetDynamicMotorReflectedPositionFromDynamicEngine_internal(void* joint, double pos, double simTime);
void _simSetJointSphericalTransformation_internal(void* joint, const double* quat, double simTime);
void _simAddForceSensorCumulativeForcesAndTorques_internal(void* forceSensor, const double* force, const double* torque,
                                                           int totalPassesCount, double simTime);
void _simAddJointCumulativeForcesOrTorques_internal(void* joint, double forceOrTorque, int totalPassesCount,
                                                    double simTime);
const void* _simGetGeomWrapFromGeomProxy_internal(const void* geomData);
double _simGetMass_internal(const void* geomInfo);
double _simGetLocalInertiaInfo_internal(const void* object, double* pos, double* quat, double* diagI);
int _simGetPurePrimitiveType_internal(const void* geomInfo);
bool _simIsGeomWrapGeometric_internal(const void* geomInfo);
bool _simIsGeomWrapConvex_internal(const void* geomInfo);
int _simGetGeometricCount_internal(const void* geomInfo);
void _simGetAllGeometrics_internal(const void* geomInfo, void** allGeometrics);
void _simGetPurePrimitiveSizes_internal(const void* geometric, double* sizes);
void _simMakeDynamicAnnouncement_internal(int announceType);
void _simGetVerticesLocalFrame_internal(const void* shape, const void* geometric, double* pos, double* quat);
const double* _simGetHeightfieldData_internal(const void* geometric, int* xCount, int* yCount, double* minHeight,
                                              double* maxHeight);
void _simGetCumulativeMeshes_internal(const void* shape, const void* geomInfo, double** vertices, int* verticesSize,
                                      int** indices, int* indicesSize);
void _simGetGravity_internal(double* gravity);
int _simGetTimeDiffInMs_internal(int previousTime);
bool _simDoEntitiesCollide_internal(int entity1ID, int entity2ID, int* cacheBuffer, bool overrideCollidableFlagIfShape1,
                                    bool overrideCollidableFlagIfShape2, bool pathOrMotionPlanningRoutineCalling);
bool _simGetDistanceBetweenEntitiesIfSmaller_internal(int entity1ID, int entity2ID, double* distance, double* ray,
                                                      int* cacheBuffer, bool overrideMeasurableFlagIfNonCollection1,
                                                      bool overrideMeasurableFlagIfNonCollection2,
                                                      bool pathPlanningRoutineCalling);
int _simHandleJointControl_internal(const void* joint, int auxV, const int* inputValuesInt,
                                    const double* inputValuesFloat, double* outputValues);
int _simGetJointDynCtrlMode_internal(const void* joint);
int _simHandleCustomContact_internal(int objHandle1, int objHandle2, int engine, int* dataInt, double* dataFloat);
const void* _simGetIkGroupObject_internal(int ikGroupID);
int _simMpHandleIkGroupObject_internal(const void* ikGroup);
double _simGetPureHollowScaling_internal(const void* geometric);
void _simDynCallback_internal(const int* intData, const double* floatData);

int simGetVisionSensorRes_internal(int visionSensorHandle, int* resolution);
int simGetObjectHierarchyOrder_internal(int objectHandle, int* totalSiblings);
int simSetObjectHierarchyOrder_internal(int objectHandle, int order);

#include <simInternal-old.h>

#endif // !defined(simInternal_INCLUDED_)
