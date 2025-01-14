#if !defined(sim_INCLUDED_)
#define sim_INCLUDED_

#include <simLib/simConst.h>
#include <simLib/simTypes.h>
#include <simLib/simExp.h>

SIM_DLLEXPORT int simSetBoolProperty(long long int target, const char* pName, int pState);
SIM_DLLEXPORT int simGetBoolProperty(long long int target, const char* pName, int* pState);
SIM_DLLEXPORT int simSetIntProperty(long long int target, const char* pName, int pState);
SIM_DLLEXPORT int simGetIntProperty(long long int target, const char* pName, int* pState);
SIM_DLLEXPORT int simSetLongProperty(long long int target, const char* pName, long long int pState);
SIM_DLLEXPORT int simGetLongProperty(long long int target, const char* pName, long long int* pState);
SIM_DLLEXPORT int simSetFloatProperty(long long int target, const char* pName, double pState);
SIM_DLLEXPORT int simGetFloatProperty(long long int target, const char* pName, double* pState);
SIM_DLLEXPORT int simSetStringProperty(long long int target, const char* pName, const char* pState);
SIM_DLLEXPORT char* simGetStringProperty(long long int target, const char* pName);
SIM_DLLEXPORT int simSetBufferProperty(long long int target, const char* pName, const char* buffer, int bufferL);
SIM_DLLEXPORT char* simGetBufferProperty(long long int target, const char* pName, int* bufferL);
SIM_DLLEXPORT int simSetIntArray2Property(long long int target, const char* pName, const int* pState);
SIM_DLLEXPORT int simGetIntArray2Property(long long int target, const char* pName, int* pState);
SIM_DLLEXPORT int simSetVector2Property(long long int target, const char* pName, const double* pState);
SIM_DLLEXPORT int simGetVector2Property(long long int target, const char* pName, double* pState);
SIM_DLLEXPORT int simSetVector3Property(long long int target, const char* pName, const double* pState);
SIM_DLLEXPORT int simGetVector3Property(long long int target, const char* pName, double* pState);
SIM_DLLEXPORT int simSetQuaternionProperty(long long int target, const char* pName, const double* pState);
SIM_DLLEXPORT int simGetQuaternionProperty(long long int target, const char* pName, double* pState);
SIM_DLLEXPORT int simSetPoseProperty(long long int target, const char* pName, const double* pState);
SIM_DLLEXPORT int simGetPoseProperty(long long int target, const char* pName, double* pState);
SIM_DLLEXPORT int simSetColorProperty(long long int target, const char* pName, const float* pState);
SIM_DLLEXPORT int simGetColorProperty(long long int target, const char* pName, float* pState);
SIM_DLLEXPORT int simSetFloatArrayProperty(long long int target, const char* pName, const double* v, int vL);
SIM_DLLEXPORT double* simGetFloatArrayProperty(long long int target, const char* pName, int* vL);
SIM_DLLEXPORT int simSetIntArrayProperty(long long int target, const char* pName, const int* v, int vL);
SIM_DLLEXPORT int* simGetIntArrayProperty(long long int target, const char* pName, int* vL);
SIM_DLLEXPORT int simRemoveProperty(long long int target, const char* pName);
SIM_DLLEXPORT char* simGetPropertyName(long long int target, int index, SPropertyOptions* options);
SIM_DLLEXPORT int simGetPropertyInfo(long long int target, const char*, SPropertyInfo* infos, SPropertyOptions* options);

SIM_DLLEXPORT int simTest(int mode, void* ptr1, void* ptr2, void* ptr3);
SIM_DLLEXPORT void* simGetMainWindow(int type);
SIM_DLLEXPORT char* simGetLastError();
SIM_DLLEXPORT char* simGetLastInfo();
SIM_DLLEXPORT int simGetObject(const char* objectPath, int index, int proxy, int options);
SIM_DLLEXPORT long long int simGetObjectUid(int objectHandle);
SIM_DLLEXPORT int simGetObjectFromUid(long long int uid, int options);
SIM_DLLEXPORT int simGetScriptHandleEx(int scriptType, int objectHandle, const char* scriptName);
SIM_DLLEXPORT int simRemoveObjects(const int* objectHandles, int count);
SIM_DLLEXPORT int simRemoveModel(int objectHandle);
SIM_DLLEXPORT char* simGetObjectAlias(int objectHandle, int options);
SIM_DLLEXPORT int simSetObjectAlias(int objectHandle, const char* objectAlias, int options);
SIM_DLLEXPORT int simGetObjectParent(int objectHandle);
SIM_DLLEXPORT int simGetObjectChild(int objectHandle, int index);
SIM_DLLEXPORT int simSetObjectParent(int objectHandle, int parentObjectHandle, bool keepInPlace);
SIM_DLLEXPORT int simGetObjectType(int objectHandle);
SIM_DLLEXPORT int simGetJointType(int objectHandle);
SIM_DLLEXPORT int simReservedCommand(int v, int w);
SIM_DLLEXPORT int simGetSimulationState();
SIM_DLLEXPORT int simLoadScene(const char* filename);
SIM_DLLEXPORT int simCloseScene();
SIM_DLLEXPORT int simSaveScene(const char* filename);
SIM_DLLEXPORT int simLoadModel(const char* filename);
SIM_DLLEXPORT int simSaveModel(int baseOfModelHandle, const char* filename);
SIM_DLLEXPORT char* simGetSimulatorMessage(int* messageID, int* auxiliaryData, int* returnedDataSize);
SIM_DLLEXPORT int simDoesFileExist(const char* filename);
SIM_DLLEXPORT int* simGetObjectSel(int* cnt);
SIM_DLLEXPORT int simSetObjectSel(const int* handles, int cnt);
SIM_DLLEXPORT int simAssociateScriptWithObject(int scriptHandle, int associatedObjectHandle);
SIM_DLLEXPORT int simResetScript(int scriptHandle);
SIM_DLLEXPORT int simAddScript(int scriptProperty);
SIM_DLLEXPORT int simRemoveScript(int scriptHandle);
SIM_DLLEXPORT int simRefreshDialogs(int refreshDegree);
SIM_DLLEXPORT int simResetProximitySensor(int sensorHandle);
SIM_DLLEXPORT void* simCreateBuffer(int size);
SIM_DLLEXPORT int simReleaseBuffer(const void* buffer);
SIM_DLLEXPORT int simCheckCollision(int entity1Handle, int entity2Handle);
SIM_DLLEXPORT int simGetRealTimeSimulation();
SIM_DLLEXPORT int simStartSimulation();
SIM_DLLEXPORT int simStopSimulation();
SIM_DLLEXPORT int simPauseSimulation();
SIM_DLLEXPORT int simAdjustView(int viewHandleOrIndex, int associatedViewableObjectHandle, int options, const char* viewLabel);
SIM_DLLEXPORT int simSetLastError(const char* setToNullptr, const char* errorMessage);
SIM_DLLEXPORT int simResetGraph(int graphHandle);
SIM_DLLEXPORT int simDestroyGraphCurve(int graphHandle, int curveId);
SIM_DLLEXPORT int simDuplicateGraphCurveToStatic(int graphHandle, int curveId, const char* curveName);
SIM_DLLEXPORT int simSetNavigationMode(int navigationMode);
SIM_DLLEXPORT int simGetNavigationMode();
SIM_DLLEXPORT int simSetPage(int index);
SIM_DLLEXPORT int simGetPage();
SIM_DLLEXPORT int simRegisterScriptCallbackFunction(const char* func, const char* reserved_setToNull, void (*callBack)(struct SScriptCallBack* cb));
SIM_DLLEXPORT int simRegisterScriptVariable(const char* var, const char* val, int stackHandle);
SIM_DLLEXPORT int simRegisterScriptFuncHook(int scriptHandle, const char* funcToHook, const char* userFunction, bool executeBefore, int options);
SIM_DLLEXPORT int simCopyPasteObjects(int* objectHandles, int objectCount, int options);
SIM_DLLEXPORT int simRemoveDrawingObject(int objectHandle);
SIM_DLLEXPORT int simAnnounceSceneContentChange();
SIM_DLLEXPORT int simSetExplicitHandling(int objectHandle, int explicitFlags);
SIM_DLLEXPORT int simGetExplicitHandling(int objectHandle);
SIM_DLLEXPORT int simGetLinkDummy(int dummyHandle);
SIM_DLLEXPORT int simSetLinkDummy(int dummyHandle, int linkedDummyHandle);
SIM_DLLEXPORT int simResetDynamicObject(int objectHandle);
SIM_DLLEXPORT int simSetJointMode(int jointHandle, int jointMode, int options);
SIM_DLLEXPORT int simGetJointMode(int jointHandle, int* options);
SIM_DLLEXPORT int simSerialOpen(const char* portString, int baudRate, void* reserved1, void* reserved2);
SIM_DLLEXPORT int simSerialClose(int portHandle);
SIM_DLLEXPORT int simSerialSend(int portHandle, const char* data, int dataLength);
SIM_DLLEXPORT int simSerialRead(int portHandle, char* buffer, int dataLengthToRead);
SIM_DLLEXPORT int simSerialCheck(int portHandle);
SIM_DLLEXPORT int simAuxiliaryConsoleClose(int consoleHandle);
SIM_DLLEXPORT int simAuxiliaryConsoleShow(int consoleHandle, bool showState);
SIM_DLLEXPORT int simAuxiliaryConsolePrint(int consoleHandle, const char* text);
SIM_DLLEXPORT int simIsHandle(int generalObjectHandle, int generalObjectType);
SIM_DLLEXPORT int simResetVisionSensor(int visionSensorHandle);
SIM_DLLEXPORT int simSetVisionSensorImg(int sensorHandle, const unsigned char* img, int options, const int* pos,
                                        const int* size);
SIM_DLLEXPORT int simRuckigPos(int dofs, double baseCycleTime, int flags, const double* currentPos,
                               const double* currentVel, const double* currentAccel, const double* maxVel,
                               const double* maxAccel, const double* maxJerk, const bool* selection,
                               const double* targetPos, const double* targetVel, double* reserved1, int* reserved2);
SIM_DLLEXPORT int simRuckigVel(int dofs, double baseCycleTime, int flags, const double* currentPos,
                               const double* currentVel, const double* currentAccel, const double* maxAccel,
                               const double* maxJerk, const bool* selection, const double* targetVel, double* reserved1,
                               int* reserved2);
SIM_DLLEXPORT int simRuckigStep(int objHandle, double cycleTime, double* newPos, double* newVel, double* newAccel,
                                double* syncTime, double* reserved1, int* reserved2);
SIM_DLLEXPORT int simRuckigRemove(int objHandle);
SIM_DLLEXPORT int simGroupShapes(const int* shapeHandles, int shapeCount);
SIM_DLLEXPORT int* simUngroupShape(int shapeHandle, int* shapeCount);
SIM_DLLEXPORT void simQuitSimulator(bool ignoredArgument);
SIM_DLLEXPORT int simSetShapeMaterial(int shapeHandle, int materialIdOrShapeHandle);
SIM_DLLEXPORT int simGetTextureId(const char* textureName, int* resolution);
SIM_DLLEXPORT unsigned char* simReadTexture(int textureId, int options, int posX, int posY, int sizeX, int sizeY);
SIM_DLLEXPORT int simGetObjects(int index, int objectType);
SIM_DLLEXPORT int* simGetObjectsInTree(int treeBaseHandle, int objectType, int options, int* objectCount);
SIM_DLLEXPORT int simGetShapeTextureId(int shapeHandle);
SIM_DLLEXPORT int simCreateCollectionEx(int options);
SIM_DLLEXPORT int simAddItemToCollection(int collectionHandle, int what, int objectHandle, int options);
SIM_DLLEXPORT int simDestroyCollection(int collectionHandle);
SIM_DLLEXPORT int* simGetCollectionObjects(int collectionHandle, int* objectCount);
SIM_DLLEXPORT int simAlignShapeBB(int shapeHandle, const double* pose);
SIM_DLLEXPORT int simRelocateShapeFrame(int shapeHandle, const double* pose);
SIM_DLLEXPORT int simSaveImage(const unsigned char* image, const int* resolution, int options, const char* filename,
                               int quality, void* reserved);
SIM_DLLEXPORT unsigned char* simLoadImage(int* resolution, int options, const char* filename, void* reserved);
SIM_DLLEXPORT unsigned char* simGetScaledImage(const unsigned char* imageIn, const int* resolutionIn,
                                               int* resolutionOut, int options, void* reserved);
SIM_DLLEXPORT int simCallScriptFunctionEx(int scriptHandleOrType, const char* functionNameAtScriptName, int stackId);
SIM_DLLEXPORT char* simGetExtensionString(int objectHandle, int index, const char* key);
SIM_DLLEXPORT int simCreateStack();
SIM_DLLEXPORT int simReleaseStack(int stackHandle);
SIM_DLLEXPORT int simCopyStack(int stackHandle);
SIM_DLLEXPORT int simPushNullOntoStack(int stackHandle);
SIM_DLLEXPORT int simPushBoolOntoStack(int stackHandle, bool value);
SIM_DLLEXPORT int simPushInt32OntoStack(int stackHandle, int value);
SIM_DLLEXPORT int simPushInt64OntoStack(int stackHandle, long long int value);
SIM_DLLEXPORT int simPushFloatOntoStack(int stackHandle, float value);
SIM_DLLEXPORT int simPushDoubleOntoStack(int stackHandle, double value);
SIM_DLLEXPORT int simPushTextOntoStack(int stackHandle, const char* value);
SIM_DLLEXPORT int simPushStringOntoStack(int stackHandle, const char* value, int stringSize);
SIM_DLLEXPORT int simPushBufferOntoStack(int stackHandle, const char* value, int stringSize);
SIM_DLLEXPORT int simPushUInt8TableOntoStack(int stackHandle, const unsigned char* values, int valueCnt);
SIM_DLLEXPORT int simPushInt32TableOntoStack(int stackHandle, const int* values, int valueCnt);
SIM_DLLEXPORT int simPushInt64TableOntoStack(int stackHandle, const long long int* values, int valueCnt);
SIM_DLLEXPORT int simPushFloatTableOntoStack(int stackHandle, const float* values, int valueCnt);
SIM_DLLEXPORT int simPushDoubleTableOntoStack(int stackHandle, const double* values, int valueCnt);
SIM_DLLEXPORT int simPushTableOntoStack(int stackHandle);
SIM_DLLEXPORT int simInsertDataIntoStackTable(int stackHandle);
SIM_DLLEXPORT int simGetStackSize(int stackHandle);
SIM_DLLEXPORT int simPopStackItem(int stackHandle, int count);
SIM_DLLEXPORT int simMoveStackItemToTop(int stackHandle, int cIndex);
SIM_DLLEXPORT int simGetStackItemType(int stackHandle, int cIndex);
SIM_DLLEXPORT int simGetStackStringType(int stackHandle, int cIndex);
SIM_DLLEXPORT int simGetStackBoolValue(int stackHandle, bool* boolValue);
SIM_DLLEXPORT int simGetStackInt32Value(int stackHandle, int* numberValue);
SIM_DLLEXPORT int simGetStackInt64Value(int stackHandle, long long int* numberValue);
SIM_DLLEXPORT int simGetStackFloatValue(int stackHandle, float* numberValue);
SIM_DLLEXPORT int simGetStackDoubleValue(int stackHandle, double* numberValue);
SIM_DLLEXPORT char* simGetStackStringValue(int stackHandle, int* stringSize);
SIM_DLLEXPORT int simGetStackTableInfo(int stackHandle, int infoType);
SIM_DLLEXPORT int simGetStackUInt8Table(int stackHandle, unsigned char* array, int count);
SIM_DLLEXPORT int simGetStackInt32Table(int stackHandle, int* array, int count);
SIM_DLLEXPORT int simGetStackInt64Table(int stackHandle, long long int* array, int count);
SIM_DLLEXPORT int simGetStackFloatTable(int stackHandle, float* array, int count);
SIM_DLLEXPORT int simGetStackDoubleTable(int stackHandle, double* array, int count);
SIM_DLLEXPORT int simUnfoldStackTable(int stackHandle);
SIM_DLLEXPORT int simDebugStack(int stackHandle, int cIndex);
SIM_DLLEXPORT float* simGetVisionSensorDepth(int sensorHandle, int options, const int* pos, const int* size,
                                             int* resolution);
SIM_DLLEXPORT int _simSetVisionSensorDepth(int sensorHandle, int options, const float* depth);
SIM_DLLEXPORT float* simCheckVisionSensorEx(int visionSensorHandle, int entityHandle, bool returnImage);
SIM_DLLEXPORT int simInsertObjectIntoOctree(int octreeHandle, int objectHandle, int options, const unsigned char* color,
                                            unsigned int tag, void* reserved);
SIM_DLLEXPORT int simSubtractObjectFromOctree(int octreeHandle, int objectHandle, int options, void* reserved);
SIM_DLLEXPORT char* simOpenTextEditor(const char* initText, const char* xml, int* various);
SIM_DLLEXPORT char* simPackTable(int stackHandle, int* bufferSize);
SIM_DLLEXPORT int simUnpackTable(int stackHandle, const char* buffer, int bufferSize);
SIM_DLLEXPORT int simSetReferencedHandles(int objectHandle, int count, const int* referencedHandles,
                                          const char* tag, const int* reserved2);
SIM_DLLEXPORT int simGetReferencedHandles(int objectHandle, int** referencedHandles, const char* tag, int** reserved2);
SIM_DLLEXPORT int simExecuteScriptString(int scriptHandle, const char* stringToExecute, int stackHandle);
SIM_DLLEXPORT char* simGetApiFunc(int scriptHandle, const char* apiWord);
SIM_DLLEXPORT char* simGetApiInfo(int scriptHandleOrType, const char* apiWord);
SIM_DLLEXPORT char* simGetPluginName(int index);
SIM_DLLEXPORT int simSetPluginInfo(const char* pluginName, int infoType, const char* stringInfo, int intInfo);
SIM_DLLEXPORT int simGetPluginInfo(const char* pluginName, int infoType, char** stringInfo, int* intInfo);
SIM_DLLEXPORT int simEventNotification(const char* event);
SIM_DLLEXPORT int simAddLog(const char* pluginName, int verbosityLevel, const char* logMsg);
SIM_DLLEXPORT int simIsDynamicallyEnabled(int objectHandle);
SIM_DLLEXPORT int simInitScript(int scriptHandle);
SIM_DLLEXPORT int simModuleEntry(int handle, const char* label, int state);
SIM_DLLEXPORT int simCheckExecAuthorization(const char* what, const char* args);
SIM_DLLEXPORT int simGetVisionSensorRes(int sensorHandle, int* resolution);
SIM_DLLEXPORT int simCreateShape(int options, double shadingAngle, const double* vertices, int verticesSize,
                                 const int* indices, int indicesSize, const double* normals, const float* textureCoords,
                                 const unsigned char* texture, const int* textureRes);

// non documented functions, mainly for the dynamics plugins:
SIM_DLLEXPORT void simRegCallback(int index, void* callback);
SIM_DLLEXPORT void simRunGui(int options);
SIM_DLLEXPORT int simInitialize(const char* appDir, int options);
SIM_DLLEXPORT int simDeinitialize();
SIM_DLLEXPORT int simPostExitRequest();
SIM_DLLEXPORT int simGetExitRequest();
SIM_DLLEXPORT int simLoop(void (*callback)(), int options);
SIM_DLLEXPORT void _simSetDynamicSimulationIconCode(void* object, int code);
SIM_DLLEXPORT void _simSetDynamicObjectFlagForVisualization(void* object, int flag);
SIM_DLLEXPORT int _simGetObjectListSize(int objType);
SIM_DLLEXPORT const void* _simGetObjectFromIndex(int objType, int index);
SIM_DLLEXPORT int _simGetObjectID(const void* object);
SIM_DLLEXPORT int _simGetObjectType(const void* object);
SIM_DLLEXPORT const void** _simGetObjectChildren(const void* object, int* count);
SIM_DLLEXPORT const void* _simGetParentObject(const void* object);
SIM_DLLEXPORT const void* _simGetObject(int objID);
SIM_DLLEXPORT bool _simIsShapeDynamicallyStatic(const void* shape);
SIM_DLLEXPORT int _simGetTreeDynamicProperty(const void* object);
SIM_DLLEXPORT int _simGetDummyLinkType(const void* dummy, int* linkedDummyID);
SIM_DLLEXPORT int _simGetJointMode(const void* joint);
SIM_DLLEXPORT bool _simIsJointInHybridOperation(const void* joint);
SIM_DLLEXPORT void _simDisableDynamicTreeForManipulation(const void* object, bool disableFlag);
SIM_DLLEXPORT bool _simIsShapeDynamicallyRespondable(const void* shape);
SIM_DLLEXPORT int _simGetDynamicCollisionMask(const void* shape);
SIM_DLLEXPORT const void* _simGetLastParentForLocalGlobalCollidable(const void* shape);
SIM_DLLEXPORT bool _simGetStartSleeping(const void* shape);
SIM_DLLEXPORT bool _simGetWasPutToSleepOnce(const void* shape);
SIM_DLLEXPORT bool _simGetDynamicsFullRefreshFlag(const void* object);
SIM_DLLEXPORT void _simSetDynamicsFullRefreshFlag(const void* object, bool flag);
SIM_DLLEXPORT void _simClearAdditionalForceAndTorque(const void* shape);
SIM_DLLEXPORT int _simGetJointType(const void* joint);
SIM_DLLEXPORT const void* _simGetGeomWrapFromGeomProxy(const void* geomData);
SIM_DLLEXPORT int _simGetPurePrimitiveType(const void* geomInfo);
SIM_DLLEXPORT bool _simIsGeomWrapGeometric(const void* geomInfo);
SIM_DLLEXPORT bool _simIsGeomWrapConvex(const void* geomInfo);
SIM_DLLEXPORT int _simGetGeometricCount(const void* geomInfo);
SIM_DLLEXPORT void _simGetAllGeometrics(const void* geomInfo, void** allGeometrics);
SIM_DLLEXPORT void _simMakeDynamicAnnouncement(int announceType);
SIM_DLLEXPORT int _simGetTimeDiffInMs(int previousTime);
SIM_DLLEXPORT bool _simDoEntitiesCollide(int entity1ID, int entity2ID, int* cacheBuffer,
                                         bool overrideCollidableFlagIfShape1, bool overrideCollidableFlagIfShape2,
                                         bool pathOrMotionPlanningRoutineCalling);
SIM_DLLEXPORT const void* _simGetIkGroupObject(int ikGroupID);
SIM_DLLEXPORT int _simMpHandleIkGroupObject(const void* ikGroup);
SIM_DLLEXPORT int _simGetJointDynCtrlMode(const void* joint);
SIM_DLLEXPORT int simFloatingViewRemove(int floatingViewHandle);
SIM_DLLEXPORT int simCreateScript(int scriptType, const char* scriptText, int options, const char* lang);
SIM_DLLEXPORT int simGetObjectHierarchyOrder(int objectHandle, int* totalSiblings);
SIM_DLLEXPORT int simSetObjectHierarchyOrder(int objectHandle, int order);

SIM_DLLEXPORT int simGetShapeViz_D(int shapeHandle, int index, struct SShapeVizInfo* info);
SIM_DLLEXPORT int simGetObjectMatrix_D(int objectHandle, int relativeToObjectHandle, double* matrix);
SIM_DLLEXPORT int simSetObjectMatrix_D(int objectHandle, int relativeToObjectHandle, const double* matrix);
SIM_DLLEXPORT int simGetObjectPose_D(int objectHandle, int relativeToObjectHandle, double* pose);
SIM_DLLEXPORT int simSetObjectPose_D(int objectHandle, int relativeToObjectHandle, const double* pose);
SIM_DLLEXPORT int simGetObjectPosition_D(int objectHandle, int relativeToObjectHandle, double* position);
SIM_DLLEXPORT int simSetObjectPosition_D(int objectHandle, int relativeToObjectHandle, const double* position);
SIM_DLLEXPORT int simGetObjectOrientation_D(int objectHandle, int relativeToObjectHandle, double* eulerAngles);
SIM_DLLEXPORT int simGetObjectQuaternion_D(int objectHandle, int relativeToObjectHandle, double* quaternion);
SIM_DLLEXPORT int simSetObjectQuaternion_D(int objectHandle, int relativeToObjectHandle, const double* quaternion);
SIM_DLLEXPORT int simSetObjectOrientation_D(int objectHandle, int relativeToObjectHandle, const double* eulerAngles);
SIM_DLLEXPORT int simGetJointPosition_D(int objectHandle, double* position);
SIM_DLLEXPORT int simSetJointPosition_D(int objectHandle, double position);
SIM_DLLEXPORT int simSetJointTargetPosition_D(int objectHandle, double targetPosition);
SIM_DLLEXPORT int simGetJointTargetPosition_D(int objectHandle, double* targetPosition);
SIM_DLLEXPORT int simGetObjectChildPose_D(int objectHandle, double* pose);
SIM_DLLEXPORT int simSetObjectChildPose_D(int objectHandle, const double* pose);
SIM_DLLEXPORT int simGetJointInterval_D(int objectHandle, bool* cyclic, double* interval);
SIM_DLLEXPORT int simSetJointInterval_D(int objectHandle, bool cyclic, const double* interval);
SIM_DLLEXPORT int simBuildIdentityMatrix_D(double* matrix);
SIM_DLLEXPORT int simBuildMatrix_D(const double* position, const double* eulerAngles, double* matrix);
SIM_DLLEXPORT int simBuildPose_D(const double* position, const double* eulerAngles, double* pose);
SIM_DLLEXPORT int simGetEulerAnglesFromMatrix_D(const double* matrix, double* eulerAngles);
SIM_DLLEXPORT int simInvertMatrix_D(double* matrix);
SIM_DLLEXPORT int simMultiplyMatrices_D(const double* matrixIn1, const double* matrixIn2, double* matrixOut);
SIM_DLLEXPORT int simMultiplyPoses_D(const double* poseIn1, const double* poseIn2, double* poseOut);
SIM_DLLEXPORT int simInvertPose_D(double* pose);
SIM_DLLEXPORT int simInterpolatePoses_D(const double* poseIn1, const double* poseIn2, double interpolFactor,
                                        double* poseOut);
SIM_DLLEXPORT int simPoseToMatrix_D(const double* poseIn, double* matrixOut);
SIM_DLLEXPORT int simMatrixToPose_D(const double* matrixIn, double* poseOut);
SIM_DLLEXPORT int simInterpolateMatrices_D(const double* matrixIn1, const double* matrixIn2, double interpolFactor,
                                           double* matrixOut);
SIM_DLLEXPORT int simTransformVector_D(const double* matrix, double* vect);
SIM_DLLEXPORT double simGetSimulationTime_D();
SIM_DLLEXPORT double simGetSystemTime_D();
SIM_DLLEXPORT int simHandleProximitySensor_D(int sensorHandle, double* detectedPoint, int* detectedObjectHandle,
                                             double* normalVector);
SIM_DLLEXPORT int simReadProximitySensor_D(int sensorHandle, double* detectedPoint, int* detectedObjectHandle,
                                           double* normalVector);
SIM_DLLEXPORT int simHandleDynamics_D(double deltaTime);
SIM_DLLEXPORT int simCheckProximitySensor_D(int sensorHandle, int entityHandle, double* detectedPoint);
SIM_DLLEXPORT int simCheckProximitySensorEx_D(int sensorHandle, int entityHandle, int detectionMode,
                                              double detectionThreshold, double maxAngle, double* detectedPoint,
                                              int* detectedObjectHandle, double* normalVector);
SIM_DLLEXPORT int simCheckProximitySensorEx2_D(int sensorHandle, double* vertexPointer, int itemType, int itemCount,
                                               int detectionMode, double detectionThreshold, double maxAngle,
                                               double* detectedPoint, double* normalVector);
SIM_DLLEXPORT int simCheckCollisionEx_D(int entity1Handle, int entity2Handle, double** intersectionSegments);
SIM_DLLEXPORT int simCheckDistance_D(int entity1Handle, int entity2Handle, double threshold, double* distanceData);
SIM_DLLEXPORT int simSetSimulationTimeStep_D(double timeStep);
SIM_DLLEXPORT double simGetSimulationTimeStep_D();
SIM_DLLEXPORT int simFloatingViewAdd_D(double posX, double posY, double sizeX, double sizeY, int options);
SIM_DLLEXPORT int simHandleGraph_D(int graphHandle, double simulationTime);
SIM_DLLEXPORT int simSetGraphStreamTransformation_D(int graphHandle, int streamId, int trType, double mult, double off,
                                                    int movingAvgPeriod);
SIM_DLLEXPORT int simAddGraphStream_D(int graphHandle, const char* streamName, const char* unitStr, int options,
                                      const float* color, double cyclicRange);
SIM_DLLEXPORT int simAddGraphCurve_D(int graphHandle, const char* curveName, int dim, const int* streamIds,
                                     const double* defaultValues, const char* unitStr, int options, const float* color,
                                     int curveWidth);
SIM_DLLEXPORT int simSetGraphStreamValue_D(int graphHandle, int streamId, double value);
SIM_DLLEXPORT int simSetJointTargetVelocity_D(int objectHandle, double targetVelocity);
SIM_DLLEXPORT int simGetJointTargetVelocity_D(int objectHandle, double* targetVelocity);
SIM_DLLEXPORT int simScaleObjects_D(const int* objectHandles, int objectCount, double scalingFactor,
                                    bool scalePositionsToo);
SIM_DLLEXPORT int simAddDrawingObject_D(int objectType, double size, double duplicateTolerance, int parentObjectHandle,
                                        int maxItemCount, const float* color, const float* setToNULL,
                                        const float* setToNULL2, const float* setToNULL3);
SIM_DLLEXPORT int simAddDrawingObjectItem_D(int objectHandle, const double* itemData);
SIM_DLLEXPORT double simGetObjectSizeFactor_D(int objectHandle);
SIM_DLLEXPORT int simReadForceSensor_D(int objectHandle, double* forceVector, double* torqueVector);
SIM_DLLEXPORT int simGetVelocity_D(int shapeHandle, double* linearVelocity, double* angularVelocity);
SIM_DLLEXPORT int simGetObjectVelocity_D(int objectHandle, double* linearVelocity, double* angularVelocity);
SIM_DLLEXPORT int simGetJointVelocity_D(int jointHandle, double* velocity);
SIM_DLLEXPORT int simAddForceAndTorque_D(int shapeHandle, const double* force, const double* torque);
SIM_DLLEXPORT int simAddForce_D(int shapeHandle, const double* position, const double* force);
SIM_DLLEXPORT int simSetObjectColor(int objectHandle, int index, int colorComponent, const float* rgbData);
SIM_DLLEXPORT int simGetObjectColor(int objectHandle, int index, int colorComponent, float* rgbData);
SIM_DLLEXPORT int simSetShapeColor(int shapeHandle, const char* colorName, int colorComponent, const float* rgbData);
SIM_DLLEXPORT int simGetShapeColor(int shapeHandle, const char* colorName, int colorComponent, float* rgbData);
SIM_DLLEXPORT int simGetContactInfo_D(int dynamicPass, int objectHandle, int index, int* objectHandles,
                                      double* contactInfo);
SIM_DLLEXPORT int simImportShape_D(int fileformat, const char* pathAndFilename, int options,
                                   double identicalVerticeTolerance, double scalingFactor);
SIM_DLLEXPORT int simImportMesh_D(int fileformat, const char* pathAndFilename, int options,
                                  double identicalVerticeTolerance, double scalingFactor, double*** vertices,
                                  int** verticesSizes, int*** indices, int** indicesSizes, double*** reserved,
                                  char*** names);
SIM_DLLEXPORT int simExportMesh_D(int fileformat, const char* pathAndFilename, int options, double scalingFactor,
                                  int elementCount, const double** vertices, const int* verticesSizes,
                                  const int** indices, const int* indicesSizes, double** reserved, const char** names);
SIM_DLLEXPORT int simCreateMeshShape_D(int options, double shadingAngle, const double* vertices, int verticesSize,
                                       const int* indices, int indicesSize, double* reserved);
SIM_DLLEXPORT int simCreatePrimitiveShape_D(int primitiveType, const double* sizes, int options);
SIM_DLLEXPORT int simCreateHeightfieldShape_D(int options, double shadingAngle, int xPointCount, int yPointCount,
                                              double xSize, const double* heights);
SIM_DLLEXPORT int simGetShapeMesh_D(int shapeHandle, double** vertices, int* verticesSize, int** indices,
                                    int* indicesSize, double** normals);
SIM_DLLEXPORT int simCreateJoint_D(int jointType, int jointMode, int options, const double* sizes,
                                   const double* reservedA, const double* reservedB);
SIM_DLLEXPORT int simAuxiliaryConsoleOpen(const char* title, int maxLines, int mode, const int* position,
                                          const int* size, const float* textColor, const float* backgroundColor);
SIM_DLLEXPORT int simGetRotationAxis_D(const double* matrixStart, const double* matrixGoal, double* axis,
                                       double* angle);
SIM_DLLEXPORT int simRotateAroundAxis_D(const double* matrixIn, const double* axis, const double* axisPos, double angle,
                                        double* matrixOut);
SIM_DLLEXPORT int simGetJointForce_D(int jointHandle, double* forceOrTorque);
SIM_DLLEXPORT int simGetJointTargetForce_D(int jointHandle, double* forceOrTorque);
SIM_DLLEXPORT int simSetJointTargetForce_D(int objectHandle, double forceOrTorque, bool signedValue);
SIM_DLLEXPORT int simCameraFitToView_D(int viewHandleOrIndex, int objectCount, const int* objectHandles, int options,
                                       double scaling);
SIM_DLLEXPORT int simHandleVisionSensor_D(int visionSensorHandle, double** auxValues, int** auxValuesCount);
SIM_DLLEXPORT int simReadVisionSensor_D(int visionSensorHandle, double** auxValues, int** auxValuesCount);
SIM_DLLEXPORT int simCheckVisionSensor_D(int visionSensorHandle, int entityHandle, double** auxValues,
                                         int** auxValuesCount);
SIM_DLLEXPORT unsigned char* simGetVisionSensorImg_D(int sensorHandle, int options, double rgbaCutOff, const int* pos,
                                                     const int* size, int* resolution);
SIM_DLLEXPORT int simCreateDummy_D(double size, const float* reserved);
SIM_DLLEXPORT int simCreateProximitySensor_D(int sensorType, int subType, int options, const int* intParams,
                                             const double* floatParams, const double* reserved);
SIM_DLLEXPORT int simCreateForceSensor_D(int options, const int* intParams, const double* floatParams,
                                         const double* reserved);
SIM_DLLEXPORT int simCreateVisionSensor_D(int options, const int* intParams, const double* floatParams,
                                          const double* reserved);
SIM_DLLEXPORT int simConvexDecompose_D(int shapeHandle, int options, const int* intParams, const double* floatParams);
SIM_DLLEXPORT int simWriteTexture_D(int textureId, int options, const char* data, int posX, int posY, int sizeX,
                                    int sizeY, double interpol);
SIM_DLLEXPORT int simCreateTexture_D(const char* fileName, int options, const double* planeSizes,
                                     const double* scalingUV, const double* xy_g, int fixedResolution, int* textureId,
                                     int* resolution, const void* reserved);
SIM_DLLEXPORT int simGetShapeGeomInfo_D(int shapeHandle, int* intData, double* floatData, void* reserved);
SIM_DLLEXPORT int simScaleObject_D(int objectHandle, double xScale, double yScale, double zScale, int options);
SIM_DLLEXPORT int simSetShapeTexture_D(int shapeHandle, int textureId, int mappingMode, int options,
                                       const double* uvScaling, const double* position, const double* orientation);
SIM_DLLEXPORT int simTransformImage_D(unsigned char* image, const int* resolution, int options,
                                      const double* floatParams, const int* intParams, void* reserved);
SIM_DLLEXPORT int simGetQHull_D(const double* inVertices, int inVerticesL, double** verticesOut, int* verticesOutL,
                                int** indicesOut, int* indicesOutL, int reserved1, const double* reserved2);
SIM_DLLEXPORT int simGetDecimatedMesh_D(const double* inVertices, int inVerticesL, const int* inIndices, int inIndicesL,
                                        double** verticesOut, int* verticesOutL, int** indicesOut, int* indicesOutL,
                                        double decimationPercent, int reserved1, const double* reserved2);
SIM_DLLEXPORT int simComputeMassAndInertia_D(int shapeHandle, double density);
SIM_DLLEXPORT int simCreateOctree_D(double voxelSize, int options, double pointSize, void* reserved);
SIM_DLLEXPORT int simCreatePointCloud_D(double maxVoxelSize, int maxPtCntPerVoxel, int options, double pointSize,
                                        void* reserved);
SIM_DLLEXPORT int simSetPointCloudOptions_D(int pointCloudHandle, double maxVoxelSize, int maxPtCntPerVoxel,
                                            int options, double pointSize, void* reserved);
SIM_DLLEXPORT int simGetPointCloudOptions_D(int pointCloudHandle, double* maxVoxelSize, int* maxPtCntPerVoxel,
                                            int* options, double* pointSize, void* reserved);
SIM_DLLEXPORT int simInsertVoxelsIntoOctree_D(int octreeHandle, int options, const double* pts, int ptCnt,
                                              const unsigned char* color, const unsigned int* tag, void* reserved);
SIM_DLLEXPORT int simRemoveVoxelsFromOctree_D(int octreeHandle, int options, const double* pts, int ptCnt,
                                              void* reserved);
SIM_DLLEXPORT int simInsertPointsIntoPointCloud_D(int pointCloudHandle, int options, const double* pts, int ptCnt,
                                                  const unsigned char* color, void* optionalValues);
SIM_DLLEXPORT int simRemovePointsFromPointCloud_D(int pointCloudHandle, int options, const double* pts, int ptCnt,
                                                  double tolerance, void* reserved);
SIM_DLLEXPORT int simIntersectPointsWithPointCloud_D(int pointCloudHandle, int options, const double* pts, int ptCnt,
                                                     double tolerance, void* reserved);
SIM_DLLEXPORT const double* simGetOctreeVoxels_D(int octreeHandle, int* ptCnt, void* reserved);
SIM_DLLEXPORT const double* simGetPointCloudPoints_D(int pointCloudHandle, int* ptCnt, void* reserved);
SIM_DLLEXPORT int simInsertObjectIntoPointCloud_D(int pointCloudHandle, int objectHandle, int options, double gridSize,
                                                  const unsigned char* color, void* optionalValues);
SIM_DLLEXPORT int simSubtractObjectFromPointCloud_D(int pointCloudHandle, int objectHandle, int options,
                                                    double tolerance, void* reserved);
SIM_DLLEXPORT int simCheckOctreePointOccupancy_D(int octreeHandle, int options, const double* points, int ptCnt,
                                                 unsigned int* tag, unsigned long long int* location, void* reserved);
SIM_DLLEXPORT int simApplyTexture_D(int shapeHandle, const double* textureCoordinates, int textCoordSize,
                                    const unsigned char* texture, const int* textureResolution, int options);
SIM_DLLEXPORT int simSetJointDependency_D(int jointHandle, int masterJointHandle, double offset, double multCoeff);
SIM_DLLEXPORT int simGetJointDependency_D(int jointHandle, int* masterJointHandle, double* offset, double* multCoeff);
SIM_DLLEXPORT int simGetShapeMass_D(int shapeHandle, double* mass);
SIM_DLLEXPORT int simSetShapeMass_D(int shapeHandle, double mass);
SIM_DLLEXPORT int simGetShapeInertia_D(int shapeHandle, double* inertiaMatrix, double* transformationMatrix);
SIM_DLLEXPORT int simSetShapeInertia_D(int shapeHandle, const double* inertiaMatrix,
                                       const double* transformationMatrix);
SIM_DLLEXPORT int simGenerateShapeFromPath_D(const double* path, int pathSize, const double* section, int sectionSize,
                                             int options, const double* upVector, double reserved);
SIM_DLLEXPORT double simGetClosestPosOnPath_D(const double* path, int pathSize, const double* pathLengths,
                                              const double* absPt);
SIM_DLLEXPORT void _simGetObjectLocalTransformation_D(const void* object, double* pos, double* quat,
                                                      bool excludeFirstJointTransformation);
SIM_DLLEXPORT void _simSetObjectLocalTransformation_D(void* object, const double* pos, const double* quat,
                                                      double simTime);
SIM_DLLEXPORT void _simDynReportObjectCumulativeTransformation_D(void* object, const double* pos, const double* quat,
                                                                 double simTime);
SIM_DLLEXPORT void _simSetObjectCumulativeTransformation_D(void* object, const double* pos, const double* quat,
                                                           bool keepChildrenInPlace);
SIM_DLLEXPORT void _simGetObjectCumulativeTransformation_D(const void* object, double* pos, double* quat,
                                                           bool excludeFirstJointTransformation);
SIM_DLLEXPORT void _simSetJointVelocity_D(const void* joint, double vel);
SIM_DLLEXPORT void _simSetJointPosition_D(const void* joint, double pos);
SIM_DLLEXPORT double _simGetJointPosition_D(const void* joint);
SIM_DLLEXPORT void _simSetDynamicMotorPositionControlTargetPosition_D(const void* joint, double pos);
SIM_DLLEXPORT void _simGetInitialDynamicVelocity_D(const void* shape, double* vel);
SIM_DLLEXPORT void _simSetInitialDynamicVelocity_D(void* shape, const double* vel);
SIM_DLLEXPORT void _simGetInitialDynamicAngVelocity_D(const void* shape, double* angularVel);
SIM_DLLEXPORT void _simSetInitialDynamicAngVelocity_D(void* shape, const double* angularVel);
SIM_DLLEXPORT void _simSetShapeDynamicVelocity_D(void* shape, const double* linear, const double* angular,
                                                 double simTime);
SIM_DLLEXPORT void _simGetAdditionalForceAndTorque_D(const void* shape, double* force, double* torque);
SIM_DLLEXPORT bool _simGetJointPositionInterval_D(const void* joint, double* minValue, double* rangeValue);
SIM_DLLEXPORT double _simGetDynamicMotorTargetPosition_D(const void* joint);
SIM_DLLEXPORT double _simGetDynamicMotorTargetVelocity_D(const void* joint);
SIM_DLLEXPORT double _simGetDynamicMotorMaxForce_D(const void* joint);
SIM_DLLEXPORT double _simGetDynamicMotorUpperLimitVelocity_D(const void* joint);
SIM_DLLEXPORT void _simSetDynamicMotorReflectedPositionFromDynamicEngine_D(void* joint, double pos, double simTime);
SIM_DLLEXPORT void _simSetJointSphericalTransformation_D(void* joint, const double* quat, double simTime);
SIM_DLLEXPORT void _simAddForceSensorCumulativeForcesAndTorques_D(void* forceSensor, const double* force,
                                                                  const double* torque, int totalPassesCount,
                                                                  double simTime);
SIM_DLLEXPORT void _simAddJointCumulativeForcesOrTorques_D(void* joint, double forceOrTorque, int totalPassesCount,
                                                           double simTime);
SIM_DLLEXPORT double _simGetMass_D(const void* geomInfo);
SIM_DLLEXPORT double _simGetLocalInertiaInfo_D(const void* object, double* pos, double* quat, double* diagI);
SIM_DLLEXPORT void _simGetPurePrimitiveSizes_D(const void* geometric, double* sizes);
SIM_DLLEXPORT void _simGetVerticesLocalFrame_D(const void* shape, const void* geometric, double* pos, double* quat);
SIM_DLLEXPORT const double* _simGetHeightfieldData_D(const void* geometric, int* xCount, int* yCount, double* minHeight,
                                                     double* maxHeight);
SIM_DLLEXPORT void _simGetCumulativeMeshes_D(const void* shape, const void* geomInfo, double** vertices,
                                             int* verticesSize, int** indices, int* indicesSize);
SIM_DLLEXPORT void _simGetGravity_D(double* gravity);
SIM_DLLEXPORT bool _simGetDistanceBetweenEntitiesIfSmaller_D(int entity1ID, int entity2ID, double* distance,
                                                             double* ray, int* cacheBuffer,
                                                             bool overrideMeasurableFlagIfNonCollection1,
                                                             bool overrideMeasurableFlagIfNonCollection2,
                                                             bool pathPlanningRoutineCalling);
SIM_DLLEXPORT int _simHandleJointControl_D(const void* joint, int auxV, const int* inputValuesInt,
                                           const double* inputValuesFloat, double* outputValues);
SIM_DLLEXPORT int _simHandleCustomContact_D(int objHandle1, int objHandle2, int engine, int* dataInt,
                                            double* dataFloat);
SIM_DLLEXPORT double _simGetPureHollowScaling_D(const void* geometric);
SIM_DLLEXPORT void _simDynCallback_D(const int* intData, const double* floatData);

#include <sim-old.h>

#endif // !defined(sim_INCLUDED_)
