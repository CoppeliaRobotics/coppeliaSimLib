#include "vrepMainHeader.h"
#include "v_rep.h"
#include "v_rep_internal.h"

VREP_DLLEXPORT simInt simRunSimulator(const simChar* applicationName,simInt options,simVoid(*initCallBack)(),simVoid(*loopCallBack)(),simVoid(*deinitCallBack)())
{
    return(simRunSimulator_internal(applicationName,options,initCallBack,loopCallBack,deinitCallBack,0,"",true));
}
VREP_DLLEXPORT simInt simRunSimulatorEx(const simChar* applicationName,simInt options,simVoid(*initCallBack)(),simVoid(*loopCallBack)(),simVoid(*deinitCallBack)(),simInt stopDelay,const simChar* sceneOrModelToLoad)
{
    return(simRunSimulator_internal(applicationName,options,initCallBack,loopCallBack,deinitCallBack,stopDelay,sceneOrModelToLoad,true));
}
VREP_DLLEXPORT simVoid* simGetMainWindow(simInt type)
{
    return(simGetMainWindow_internal(type));
}
VREP_DLLEXPORT simChar* simGetLastError()
{
    return(simGetLastError_internal());
}
VREP_DLLEXPORT simInt simSetBooleanParameter(simInt parameter,simBool boolState)
{
    return(simSetBoolParameter_internal(parameter,boolState));
}
VREP_DLLEXPORT simInt simGetBooleanParameter(simInt parameter)
{
    return(simGetBoolParameter_internal(parameter));
}
VREP_DLLEXPORT simInt simSetBoolParameter(simInt parameter,simBool boolState)
{
    return(simSetBoolParameter_internal(parameter,boolState));
}
VREP_DLLEXPORT simInt simGetBoolParameter(simInt parameter)
{
    return(simGetBoolParameter_internal(parameter));
}
VREP_DLLEXPORT simInt simSetIntegerParameter(simInt parameter,simInt intState)
{
    return(simSetInt32Parameter_internal(parameter,intState));
}
VREP_DLLEXPORT simInt simGetIntegerParameter(simInt parameter,simInt* intState)
{
    return(simGetInt32Parameter_internal(parameter,intState));
}
VREP_DLLEXPORT simInt simSetInt32Parameter(simInt parameter,simInt intState)
{
    return(simSetInt32Parameter_internal(parameter,intState));
}
VREP_DLLEXPORT simInt simGetInt32Parameter(simInt parameter,simInt* intState)
{
    return(simGetInt32Parameter_internal(parameter,intState));
}
VREP_DLLEXPORT simInt simGetUInt64Parameter(simInt parameter,simUInt64* intState)
{
    return(simGetUInt64Parameter_internal(parameter,intState));
}
VREP_DLLEXPORT simInt simSetFloatingParameter(simInt parameter,simFloat floatState)
{
    return(simSetFloatParameter_internal(parameter,floatState));
}
VREP_DLLEXPORT simInt simGetFloatingParameter(simInt parameter,simFloat* floatState)
{
    return(simGetFloatParameter_internal(parameter,floatState));
}
VREP_DLLEXPORT simInt simSetFloatParameter(simInt parameter,simFloat floatState)
{
    return(simSetFloatParameter_internal(parameter,floatState));
}
VREP_DLLEXPORT simInt simGetFloatParameter(simInt parameter,simFloat* floatState)
{
    return(simGetFloatParameter_internal(parameter,floatState));
}
VREP_DLLEXPORT simInt simSetStringParameter(simInt parameter,const simChar* str)
{
    return(simSetStringParameter_internal(parameter,str));
}
VREP_DLLEXPORT simChar* simGetStringParameter(simInt parameter)
{
    return(simGetStringParameter_internal(parameter));
}
VREP_DLLEXPORT simInt simGetObjectHandle(const simChar* objectName)
{
    return(simGetObjectHandle_internal(objectName));
}
VREP_DLLEXPORT simInt simRemoveObject(simInt objectHandle)
{
    return(simRemoveObject_internal(objectHandle));
}
VREP_DLLEXPORT simInt simRemoveModel(simInt objectHandle)
{
    return(simRemoveModel_internal(objectHandle));
}
VREP_DLLEXPORT simChar* simGetObjectName(simInt objectHandle)
{
    return(simGetObjectName_internal(objectHandle));
}
VREP_DLLEXPORT simInt simGetObjects(simInt index,simInt objectType)
{
    return(simGetObjects_internal(index,objectType));
}
VREP_DLLEXPORT simInt simSetObjectName(simInt objectHandle,const simChar* objectName)
{
    return(simSetObjectName_internal(objectHandle,objectName));
}
VREP_DLLEXPORT simInt simGetCollectionHandle(const simChar* collectionName)
{
    return(simGetCollectionHandle_internal(collectionName));
}
VREP_DLLEXPORT simInt simRemoveCollection(simInt collectionHandle)
{
    return(simRemoveCollection_internal(collectionHandle));
}
VREP_DLLEXPORT simInt simEmptyCollection(simInt collectionHandle)
{
    return(simEmptyCollection_internal(collectionHandle));
}
VREP_DLLEXPORT simChar* simGetCollectionName(simInt collectionHandle)
{
    return(simGetCollectionName_internal(collectionHandle));
}
VREP_DLLEXPORT simInt simSetCollectionName(simInt collectionHandle,const simChar* collectionName)
{
    return(simSetCollectionName_internal(collectionHandle,collectionName));
}
VREP_DLLEXPORT simInt simGetObjectMatrix(simInt objectHandle,simInt relativeToObjectHandle,simFloat* matrix)
{
    return(simGetObjectMatrix_internal(objectHandle,relativeToObjectHandle,matrix));
}
VREP_DLLEXPORT simInt simSetObjectMatrix(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* matrix)
{
    return(simSetObjectMatrix_internal(objectHandle,relativeToObjectHandle,matrix));
}
VREP_DLLEXPORT simInt simGetObjectPosition(simInt objectHandle,simInt relativeToObjectHandle,simFloat* position)
{
    return(simGetObjectPosition_internal(objectHandle,relativeToObjectHandle,position));
}
VREP_DLLEXPORT simInt simSetObjectPosition(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* position)
{
    return(simSetObjectPosition_internal(objectHandle,relativeToObjectHandle,position));
}
VREP_DLLEXPORT simInt simGetObjectOrientation(simInt objectHandle,simInt relativeToObjectHandle,simFloat* eulerAngles)
{
    return(simGetObjectOrientation_internal(objectHandle,relativeToObjectHandle,eulerAngles));
}
VREP_DLLEXPORT simInt simGetObjectQuaternion(simInt objectHandle,simInt relativeToObjectHandle,simFloat* quaternion)
{
    return(simGetObjectQuaternion_internal(objectHandle,relativeToObjectHandle,quaternion));
}
VREP_DLLEXPORT simInt simSetObjectQuaternion(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* quaternion)
{
    return(simSetObjectQuaternion_internal(objectHandle,relativeToObjectHandle,quaternion));
}
VREP_DLLEXPORT simInt simSetObjectOrientation(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* eulerAngles)
{
    return(simSetObjectOrientation_internal(objectHandle,relativeToObjectHandle,eulerAngles));
}
VREP_DLLEXPORT simInt simGetJointPosition(simInt objectHandle,simFloat* position)
{
    return(simGetJointPosition_internal(objectHandle,position));
}
VREP_DLLEXPORT simInt simSetJointPosition(simInt objectHandle,simFloat position)
{
    return(simSetJointPosition_internal(objectHandle,position));
}
VREP_DLLEXPORT simInt simSetJointTargetPosition(simInt objectHandle,simFloat targetPosition)
{
    return(simSetJointTargetPosition_internal(objectHandle,targetPosition));
}
VREP_DLLEXPORT simInt simGetJointTargetPosition(simInt objectHandle,simFloat* targetPosition)
{
    return(simGetJointTargetPosition_internal(objectHandle,targetPosition));
}
VREP_DLLEXPORT simInt simSetJointForce(simInt objectHandle,simFloat forceOrTorque)
{
    return(simSetJointForce_internal(objectHandle,forceOrTorque));
}
VREP_DLLEXPORT simInt simGetPathPosition(simInt objectHandle,simFloat* position)
{
    return(simGetPathPosition_internal(objectHandle,position));
}
VREP_DLLEXPORT simInt simSetPathPosition(simInt objectHandle,simFloat position)
{
    return(simSetPathPosition_internal(objectHandle,position));
}
VREP_DLLEXPORT simInt simGetPathLength(simInt objectHandle,simFloat* length)
{
    return(simGetPathLength_internal(objectHandle,length));
}
VREP_DLLEXPORT simInt simGetJointMatrix(simInt objectHandle,simFloat* matrix)
{
    return(simGetJointMatrix_internal(objectHandle,matrix));
}
VREP_DLLEXPORT simInt simSetSphericalJointMatrix(simInt objectHandle,const simFloat* matrix)
{
    return(simSetSphericalJointMatrix_internal(objectHandle,matrix));
}
VREP_DLLEXPORT simInt simGetJointInterval(simInt objectHandle,simBool* cyclic,simFloat* interval)
{
    return(simGetJointInterval_internal(objectHandle,cyclic,interval));
}
VREP_DLLEXPORT simInt simSetJointInterval(simInt objectHandle,simBool cyclic,const simFloat* interval)
{
    return(simSetJointInterval_internal(objectHandle,cyclic,interval));
}
VREP_DLLEXPORT simInt simGetObjectParent(simInt objectHandle)
{
    return(simGetObjectParent_internal(objectHandle));
}
VREP_DLLEXPORT simInt simGetObjectChild(simInt objectHandle,simInt index)
{
    return(simGetObjectChild_internal(objectHandle,index));
}
VREP_DLLEXPORT simInt simSetObjectParent(simInt objectHandle,simInt parentObjectHandle,simBool keepInPlace)
{
    return(simSetObjectParent_internal(objectHandle,parentObjectHandle,keepInPlace));
}
VREP_DLLEXPORT simInt simGetObjectType(simInt objectHandle)
{
    return(simGetObjectType_internal(objectHandle));
}
VREP_DLLEXPORT simInt simGetJointType(simInt objectHandle)
{
    return(simGetJointType_internal(objectHandle));
}
VREP_DLLEXPORT simInt simBuildIdentityMatrix(simFloat* matrix)
{
    return(simBuildIdentityMatrix_internal(matrix));
}
VREP_DLLEXPORT simInt simCopyMatrix(const simFloat* matrixIn,simFloat* matrixOut)
{
    return(simCopyMatrix_internal(matrixIn,matrixOut));
}
VREP_DLLEXPORT simInt simBuildMatrix(const simFloat* position,const simFloat* eulerAngles,simFloat* matrix)
{
    return(simBuildMatrix_internal(position,eulerAngles,matrix));
}
VREP_DLLEXPORT simInt simBuildMatrixQ(const simFloat* position,const simFloat* quaternion,simFloat* matrix)
{
    return(simBuildMatrixQ_internal(position,quaternion,matrix));
}
VREP_DLLEXPORT simInt simGetEulerAnglesFromMatrix(const simFloat* matrix,simFloat* eulerAngles)
{
    return(simGetEulerAnglesFromMatrix_internal(matrix,eulerAngles));
}
VREP_DLLEXPORT simInt simGetQuaternionFromMatrix(const simFloat* matrix,simFloat* quaternion)
{
    return(simGetQuaternionFromMatrix_internal(matrix,quaternion));
}
VREP_DLLEXPORT simInt simInvertMatrix(simFloat* matrix)
{
    return(simInvertMatrix_internal(matrix));
}
VREP_DLLEXPORT simInt simMultiplyMatrices(const simFloat* matrixIn1,const simFloat* matrixIn2,simFloat* matrixOut)
{
    return(simMultiplyMatrices_internal(matrixIn1,matrixIn2,matrixOut));
}
VREP_DLLEXPORT simInt simInterpolateMatrices(const simFloat* matrixIn1,const simFloat* matrixIn2,simFloat interpolFactor,simFloat* matrixOut)
{
    return(simInterpolateMatrices_internal(matrixIn1,matrixIn2,interpolFactor,matrixOut));
}
VREP_DLLEXPORT simInt simTransformVector(const simFloat* matrix,simFloat* vect)
{
    return(simTransformVector_internal(matrix,vect));
}
VREP_DLLEXPORT simInt simReservedCommand(simInt v,simInt w)
{
    return(simReservedCommand_internal(v,w));
}
VREP_DLLEXPORT simFloat simGetSimulationTime()
{
    return(simGetSimulationTime_internal());
}
VREP_DLLEXPORT simInt simGetSimulationState()
{
    return(simGetSimulationState_internal());
}
VREP_DLLEXPORT simFloat simGetSystemTime()
{
    return(simGetSystemTime_internal());
}
VREP_DLLEXPORT simInt simGetSystemTimeInMilliseconds()
{
    return(simGetSystemTimeInMilliseconds_internal());
}
VREP_DLLEXPORT simUInt simGetSystemTimeInMs(simInt previousTime)
{
    return(simGetSystemTimeInMs_internal(previousTime));
}
VREP_DLLEXPORT simInt simLoadScene(const simChar* filename)
{
    return(simLoadScene_internal(filename));
}
VREP_DLLEXPORT simInt simCloseScene()
{
    return(simCloseScene_internal());
}
VREP_DLLEXPORT simInt simSaveScene(const simChar* filename)
{
    return(simSaveScene_internal(filename));
}
VREP_DLLEXPORT simInt simLoadModel(const simChar* filename)
{
    return(simLoadModel_internal(filename));
}
VREP_DLLEXPORT simInt simSaveModel(int baseOfModelHandle,const simChar* filename)
{
    return(simSaveModel_internal(baseOfModelHandle,filename));
}
VREP_DLLEXPORT simInt simAddStatusbarMessage(const simChar* message)
{
    return(simAddStatusbarMessage_internal(message));
}
VREP_DLLEXPORT simChar* simGetSimulatorMessage(simInt* messageID,simInt* auxiliaryData,simInt* returnedDataSize)
{
    return(simGetSimulatorMessage_internal(messageID,auxiliaryData,returnedDataSize));
}
VREP_DLLEXPORT simInt simAddModuleMenuEntry(const simChar* entryLabel,simInt itemCount,simInt* itemHandles)
{
    return(simAddModuleMenuEntry_internal(entryLabel,itemCount,itemHandles));
}
VREP_DLLEXPORT simInt simSetModuleMenuItemState(simInt itemHandle,simInt state,const simChar* label)
{
    return(simSetModuleMenuItemState_internal(itemHandle,state,label));
}
VREP_DLLEXPORT simInt simDoesFileExist(const simChar* filename)
{
    return(simDoesFileExist_internal(filename));
}
VREP_DLLEXPORT simInt simIsObjectInSelection(simInt objectHandle)
{
    return(simIsObjectInSelection_internal(objectHandle));
}
VREP_DLLEXPORT simInt simAddObjectToSelection(simInt what,simInt objectHandle)
{
    return(simAddObjectToSelection_internal(what,objectHandle));
}
VREP_DLLEXPORT simInt simRemoveObjectFromSelection(simInt what,simInt objectHandle)
{
    return(simRemoveObjectFromSelection_internal(what,objectHandle));
}
VREP_DLLEXPORT simInt simGetObjectSelectionSize()
{
    return(simGetObjectSelectionSize_internal());
}
VREP_DLLEXPORT simInt simGetObjectLastSelection()
{
    return(simGetObjectLastSelection_internal());
}
VREP_DLLEXPORT simInt simGetObjectSelection(simInt* objectHandles)
{
    return(simGetObjectSelection_internal(objectHandles));
}
VREP_DLLEXPORT simInt simHandleCollision(simInt collisionObjectHandle)
{
    return(simHandleCollision_internal(collisionObjectHandle));
}
VREP_DLLEXPORT simInt simReadCollision(simInt collisionObjectHandle)
{
    return(simReadCollision_internal(collisionObjectHandle));
}
VREP_DLLEXPORT simInt simHandleDistance(simInt distanceObjectHandle,simFloat* smallestDistance)
{
    return(simHandleDistance_internal(distanceObjectHandle,smallestDistance));
}
VREP_DLLEXPORT simInt simReadDistance(simInt distanceObjectHandle,simFloat* smallestDistance)
{
    return(simReadDistance_internal(distanceObjectHandle,smallestDistance));
}
VREP_DLLEXPORT simInt simHandleProximitySensor(simInt sensorHandle,simFloat* detectedPoint,simInt* detectedObjectHandle,simFloat* normalVector)
{
    return(simHandleProximitySensor_internal(sensorHandle,detectedPoint,detectedObjectHandle,normalVector));
}
VREP_DLLEXPORT simInt simReadProximitySensor(simInt sensorHandle,simFloat* detectedPoint,simInt* detectedObjectHandle,simFloat* normalVector)
{
    return(simReadProximitySensor_internal(sensorHandle,detectedPoint,detectedObjectHandle,normalVector));
}
VREP_DLLEXPORT simInt simHandleMill(simInt millHandle,simFloat* removedSurfaceAndVolume)
{
    return(simHandleMill_internal(millHandle,removedSurfaceAndVolume));
}
VREP_DLLEXPORT simInt simHandleIkGroup(simInt ikGroupHandle)
{
    return(simHandleIkGroup_internal(ikGroupHandle));
}
VREP_DLLEXPORT simInt simCheckIkGroup(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simFloat* jointValues,const simInt* jointOptions)
{
    return(simCheckIkGroup_internal(ikGroupHandle,jointCnt,jointHandles,jointValues,jointOptions));
}
VREP_DLLEXPORT simInt simHandleDynamics(simFloat deltaTime)
{
    return(simHandleDynamics_internal(deltaTime));
}
VREP_DLLEXPORT simInt simGetScriptHandle(const simChar* scriptName)
{
    return(simGetScriptHandle_internal(scriptName));
}
VREP_DLLEXPORT simInt simSetScriptText(simInt scriptHandle,const simChar* scriptText)
{
    return(simSetScriptText_internal(scriptHandle,scriptText));
}
VREP_DLLEXPORT const simChar* simGetScriptText(simInt scriptHandle)
{
    return(simGetScriptText_internal(scriptHandle));
}
VREP_DLLEXPORT simInt simGetScriptProperty(simInt scriptHandle,simInt* scriptProperty,simInt* associatedObjectHandle)
{
    return(simGetScriptProperty_internal(scriptHandle,scriptProperty,associatedObjectHandle));
}
VREP_DLLEXPORT simInt simAssociateScriptWithObject(simInt scriptHandle,simInt associatedObjectHandle)
{
    return(simAssociateScriptWithObject_internal(scriptHandle,associatedObjectHandle));
}
VREP_DLLEXPORT simInt simGetScript(simInt index)
{
    return(simGetScript_internal(index));
}
VREP_DLLEXPORT simInt simGetScriptAssociatedWithObject(simInt objectHandle)
{
    return(simGetScriptAssociatedWithObject_internal(objectHandle));
}
VREP_DLLEXPORT simInt simGetCustomizationScriptAssociatedWithObject(simInt objectHandle)
{
    return(simGetCustomizationScriptAssociatedWithObject_internal(objectHandle));
}
VREP_DLLEXPORT simInt simGetObjectAssociatedWithScript(simInt scriptHandle)
{
    return(simGetObjectAssociatedWithScript_internal(scriptHandle));
}
VREP_DLLEXPORT simChar* simGetScriptName(simInt scriptHandle)
{
    return(simGetScriptName_internal(scriptHandle));
}
VREP_DLLEXPORT simInt simHandleMainScript()
{
    return(simHandleMainScript_internal());
}
VREP_DLLEXPORT simInt simResetScript(simInt scriptHandle)
{
    return(simResetScript_internal(scriptHandle));
}
VREP_DLLEXPORT simInt simAddScript(simInt scriptProperty)
{
    return(simAddScript_internal(scriptProperty));
}
VREP_DLLEXPORT simInt simRemoveScript(simInt scriptHandle)
{
    return(simRemoveScript_internal(scriptHandle));
}
VREP_DLLEXPORT simInt simRefreshDialogs(simInt refreshDegree)
{
    return(simRefreshDialogs_internal(refreshDegree));
}
VREP_DLLEXPORT simInt simGetCollisionHandle(const simChar* collisionObjectName)
{
    return(simGetCollisionHandle_internal(collisionObjectName));
}
VREP_DLLEXPORT simInt simGetDistanceHandle(const simChar* distanceObjectName)
{
    return(simGetDistanceHandle_internal(distanceObjectName));
}
VREP_DLLEXPORT simInt simGetIkGroupHandle(const simChar* ikGroupName)
{
    return(simGetIkGroupHandle_internal(ikGroupName));
}
VREP_DLLEXPORT simInt simResetCollision(simInt collisionObjectHandle)
{
    return(simResetCollision_internal(collisionObjectHandle));
}
VREP_DLLEXPORT simInt simResetDistance(simInt distanceObjectHandle)
{
    return(simResetDistance_internal(distanceObjectHandle));
}
VREP_DLLEXPORT simInt simResetProximitySensor(simInt sensorHandle)
{
    return(simResetProximitySensor_internal(sensorHandle));
}
VREP_DLLEXPORT simInt simResetMill(simInt millHandle)
{
    return(simResetMill_internal(millHandle));
}
VREP_DLLEXPORT simInt simCheckProximitySensor(simInt sensorHandle,simInt entityHandle,simFloat* detectedPoint)
{
    return(simCheckProximitySensor_internal(sensorHandle,entityHandle,detectedPoint));
}
VREP_DLLEXPORT simInt simCheckProximitySensorEx(simInt sensorHandle,simInt entityHandle,simInt detectionMode,simFloat detectionThreshold,simFloat maxAngle,simFloat* detectedPoint,simInt* detectedObjectHandle,simFloat* normalVector)
{
    return(simCheckProximitySensorEx_internal(sensorHandle,entityHandle,detectionMode,detectionThreshold,maxAngle,detectedPoint,detectedObjectHandle,normalVector));
}
VREP_DLLEXPORT simInt simCheckProximitySensorEx2(simInt sensorHandle,simFloat* vertexPointer,simInt itemType,simInt itemCount,simInt detectionMode,simFloat detectionThreshold,simFloat maxAngle,simFloat* detectedPoint,simFloat* normalVector)
{
    return(simCheckProximitySensorEx2_internal(sensorHandle,vertexPointer,itemType,itemCount,detectionMode,detectionThreshold,maxAngle,detectedPoint,normalVector));
}
VREP_DLLEXPORT simChar* simCreateBuffer(simInt size)
{
    return(simCreateBuffer_internal(size));
}
VREP_DLLEXPORT simInt simReleaseBuffer(const simChar* buffer)
{
    return(simReleaseBuffer_internal(buffer));
}
VREP_DLLEXPORT simInt simCheckCollision(simInt entity1Handle,simInt entity2Handle)
{
    return(simCheckCollision_internal(entity1Handle,entity2Handle));
}
VREP_DLLEXPORT simInt simCheckCollisionEx(simInt entity1Handle,simInt entity2Handle,simFloat** intersectionSegments)
{
    return(simCheckCollisionEx_internal(entity1Handle,entity2Handle,intersectionSegments));
}
VREP_DLLEXPORT simInt simCheckDistance(simInt entity1Handle,simInt entity2Handle,simFloat threshold,simFloat* distanceData)
{
    return(simCheckDistance_internal(entity1Handle,entity2Handle,threshold,distanceData));
}
VREP_DLLEXPORT simChar* simGetObjectConfiguration(simInt objectHandle)
{
    return(simGetObjectConfiguration_internal(objectHandle));
}
VREP_DLLEXPORT simInt simSetObjectConfiguration(const simChar* data)
{
    return(simSetObjectConfiguration_internal(data));
}
VREP_DLLEXPORT simChar* simGetConfigurationTree(simInt objectHandle)
{
    return(simGetConfigurationTree_internal(objectHandle));
}
VREP_DLLEXPORT simInt simSetConfigurationTree(const simChar* data)
{
    return(simSetConfigurationTree_internal(data));
}
VREP_DLLEXPORT simInt simSetSimulationTimeStep(simFloat timeStep)
{
    return(simSetSimulationTimeStep_internal(timeStep));
}
VREP_DLLEXPORT simFloat simGetSimulationTimeStep()
{
    return(simGetSimulationTimeStep_internal());
}
VREP_DLLEXPORT simInt simGetRealTimeSimulation()
{
    return(simGetRealTimeSimulation_internal());
}
VREP_DLLEXPORT simInt simIsRealTimeSimulationStepNeeded()
{
    return(simIsRealTimeSimulationStepNeeded_internal());
}
VREP_DLLEXPORT simInt simAdjustRealTimeTimer(simInt instanceIndex,simFloat deltaTime)
{
    return(simAdjustRealTimeTimer_internal(instanceIndex,deltaTime));
}
VREP_DLLEXPORT simInt simGetSimulationPassesPerRenderingPass()
{
    return(simGetSimulationPassesPerRenderingPass_internal());
}
VREP_DLLEXPORT simInt simAdvanceSimulationByOneStep()
{
    return(simAdvanceSimulationByOneStep_internal());
}
VREP_DLLEXPORT simInt simStartSimulation()
{
    return(simStartSimulation_internal());
}
VREP_DLLEXPORT simInt simStopSimulation()
{
    return(simStopSimulation_internal());
}
VREP_DLLEXPORT simInt simPauseSimulation()
{
    return(simPauseSimulation_internal());
}
VREP_DLLEXPORT simInt simLoadModule(const simChar* filenameAndPath,const simChar* pluginName)
{
    return(simLoadModule_internal(filenameAndPath,pluginName));
}
VREP_DLLEXPORT simInt simUnloadModule(simInt pluginhandle)
{
    return(simUnloadModule_internal(pluginhandle));
}
VREP_DLLEXPORT simVoid* simSendModuleMessage(simInt message,simInt* auxiliaryData,simVoid* customData,simInt* replyData)
{
    return(simSendModuleMessage_internal(message,auxiliaryData,customData,replyData));
}
VREP_DLLEXPORT simVoid* simBroadcastMessage(simInt* auxiliaryData,simVoid* customData,simInt* replyData)
{
    return(simBroadcastMessage_internal(auxiliaryData,customData,replyData));
}
VREP_DLLEXPORT simChar* simGetModuleName(simInt index,simUChar* moduleVersion)
{
    return(simGetModuleName_internal(index,moduleVersion));
}
VREP_DLLEXPORT simChar* simGetScriptSimulationParameter(simInt scriptHandle,const simChar* parameterName,simInt* parameterLength)
{
    return(simGetScriptSimulationParameter_internal(scriptHandle,parameterName,parameterLength));
}
VREP_DLLEXPORT simInt simSetScriptSimulationParameter(simInt scriptHandle,const simChar* parameterName,const simChar* parameterValue,simInt parameterLength)
{
    return(simSetScriptSimulationParameter_internal(scriptHandle,parameterName,parameterValue,parameterLength));
}
VREP_DLLEXPORT simInt simFloatingViewAdd(simFloat posX,simFloat posY,simFloat sizeX,simFloat sizeY,simInt options)
{
    return(simFloatingViewAdd_internal(posX,posY,sizeX,sizeY,options));
}
VREP_DLLEXPORT simInt simFloatingViewRemove(simInt floatingViewHandle)
{
    return(simFloatingViewRemove_internal(floatingViewHandle));
}
VREP_DLLEXPORT simInt simAdjustView(simInt viewHandleOrIndex,simInt associatedViewableObjectHandle,simInt options,const simChar* viewLabel)
{
    return(simAdjustView_internal(viewHandleOrIndex,associatedViewableObjectHandle,options,viewLabel));
}
VREP_DLLEXPORT simInt simSetLastError(const simChar* funcName,const simChar* errorMessage)
{
    return(simSetLastError_internal(funcName,errorMessage));
}
VREP_DLLEXPORT simInt simHandleGraph(simInt graphHandle,simFloat simulationTime)
{
    return(simHandleGraph_internal(graphHandle,simulationTime));
}
VREP_DLLEXPORT simInt simResetGraph(simInt graphHandle)
{
    return(simResetGraph_internal(graphHandle));
}
VREP_DLLEXPORT simInt simSetNavigationMode(simInt navigationMode)
{
    return(simSetNavigationMode_internal(navigationMode));
}
VREP_DLLEXPORT simInt simGetNavigationMode()
{
    return(simGetNavigationMode_internal());
}
VREP_DLLEXPORT simInt simSetPage(simInt index)
{
    return(simSetPage_internal(index));
}
VREP_DLLEXPORT simInt simGetPage()
{
    return(simGetPage_internal());
}
VREP_DLLEXPORT simInt simDisplayDialog(const simChar* titleText,const simChar* mainText,simInt dialogType,const simChar* initialText,const simFloat* titleColors,const simFloat* dialogColors,simInt* elementHandle)
{
    return(simDisplayDialog_internal(titleText,mainText,dialogType,initialText,titleColors,dialogColors,elementHandle));
}
VREP_DLLEXPORT simInt simGetDialogResult(simInt genericDialogHandle)
{
    return(simGetDialogResult_internal(genericDialogHandle));
}
VREP_DLLEXPORT simChar* simGetDialogInput(simInt genericDialogHandle)
{
    return(simGetDialogInput_internal(genericDialogHandle));
}
VREP_DLLEXPORT simInt simEndDialog(simInt genericDialogHandle)
{
    return(simEndDialog_internal(genericDialogHandle));
}
VREP_DLLEXPORT simInt simRegisterScriptCallbackFunction(const simChar* funcNameAtPluginName,const simChar* callTips,simVoid(*callBack)(struct SScriptCallBack* cb))
{
    return(simRegisterScriptCallbackFunction_internal(funcNameAtPluginName,callTips,callBack));
}
VREP_DLLEXPORT simInt simRegisterScriptVariable(const simChar* varNameAtPluginName,const simChar* varValue,simInt stackHandle)
{
    return(simRegisterScriptVariable_internal(varNameAtPluginName,varValue,stackHandle));
}
VREP_DLLEXPORT simInt simSetJointTargetVelocity(simInt objectHandle,simFloat targetVelocity)
{
    return(simSetJointTargetVelocity_internal(objectHandle,targetVelocity));
}
VREP_DLLEXPORT simInt simGetJointTargetVelocity(simInt objectHandle,simFloat* targetVelocity)
{
    return(simGetJointTargetVelocity_internal(objectHandle,targetVelocity));
}
VREP_DLLEXPORT simInt simSetPathTargetNominalVelocity(simInt objectHandle,simFloat targetNominalVelocity)
{
    return(simSetPathTargetNominalVelocity_internal(objectHandle,targetNominalVelocity));
}
VREP_DLLEXPORT simChar* simGetScriptRawBuffer(simInt scriptHandle,simInt bufferHandle)
{
    return(simGetScriptRawBuffer_internal(scriptHandle,bufferHandle));
}
VREP_DLLEXPORT simInt simSetScriptRawBuffer(simInt scriptHandle,const simChar* buffer,simInt bufferSize)
{
    return(simSetScriptRawBuffer_internal(scriptHandle,buffer,bufferSize));
}
VREP_DLLEXPORT simInt simReleaseScriptRawBuffer(simInt scriptHandle,simInt bufferHandle)
{
    return(simReleaseScriptRawBuffer_internal(scriptHandle,bufferHandle));
}
VREP_DLLEXPORT simInt simCopyPasteObjects(simInt* objectHandles,simInt objectCount,simInt options)
{
    return(simCopyPasteObjects_internal(objectHandles,objectCount,options));
}
VREP_DLLEXPORT simInt simScaleSelectedObjects(simFloat scalingFactor,simBool scalePositionsToo)
{
    return(simScaleSelectedObjects_internal(scalingFactor,scalePositionsToo));
}
VREP_DLLEXPORT simInt simScaleObjects(const simInt* objectHandles,simInt objectCount,simFloat scalingFactor,simBool scalePositionsToo)
{
    return(simScaleObjects_internal(objectHandles,objectCount,scalingFactor,scalePositionsToo));
}
VREP_DLLEXPORT simInt simDeleteSelectedObjects()
{
    return(simDeleteSelectedObjects_internal());
}
VREP_DLLEXPORT simInt simGetObjectUniqueIdentifier(simInt objectHandle,simInt* uniqueIdentifier)
{
    return(simGetObjectUniqueIdentifier_internal(objectHandle,uniqueIdentifier));
}
VREP_DLLEXPORT simInt simGetNameSuffix(const simChar* name)
{
    return(simGetNameSuffix_internal(name));
}
VREP_DLLEXPORT simInt simSendData(simInt targetID,simInt dataHeader,const simChar* dataName,const simChar* data,simInt dataLength,simInt antennaHandle,simFloat actionRadius,simFloat emissionAngle1,simFloat emissionAngle2,simFloat persistence)
{
    return(simSendData_internal(targetID,dataHeader,dataName,data,dataLength,antennaHandle,actionRadius,emissionAngle1,emissionAngle2,persistence));
}
VREP_DLLEXPORT simChar* simReceiveData(simInt dataHeader,const simChar* dataName,simInt antennaHandle,simInt index,simInt* dataLength,simInt* senderID,simInt* dataHeaderR,simChar** dataNameR)
{
    return(simReceiveData_internal(dataHeader,dataName,antennaHandle,index,dataLength,senderID,dataHeaderR,dataNameR));
}
VREP_DLLEXPORT simInt simSetGraphUserData(simInt graphHandle,const simChar* dataStreamName,simFloat data)
{
    return(simSetGraphUserData_internal(graphHandle,dataStreamName,data));
}
VREP_DLLEXPORT simInt simSetNameSuffix(simInt nameSuffixNumber)
{
    return(simSetNameSuffix_internal(nameSuffixNumber));
}
VREP_DLLEXPORT simInt simAddDrawingObject(simInt objectType,simFloat size,simFloat duplicateTolerance,simInt parentObjectHandle,simInt maxItemCount,const simFloat* ambient_diffuse,const simFloat* setToNULL,const simFloat* specular,const simFloat* emission)
{
    return(simAddDrawingObject_internal(objectType,size,duplicateTolerance,parentObjectHandle,maxItemCount,ambient_diffuse,setToNULL,specular,emission));
}
VREP_DLLEXPORT simInt simRemoveDrawingObject(simInt objectHandle)
{
    return(simRemoveDrawingObject_internal(objectHandle));
}
VREP_DLLEXPORT simInt simAddDrawingObjectItem(simInt objectHandle,const simFloat* itemData)
{
    return(simAddDrawingObjectItem_internal(objectHandle,itemData));
}
VREP_DLLEXPORT simInt simAddParticleObject(simInt objectType,simFloat size,simFloat density,const simVoid* params,simFloat lifeTime,simInt maxItemCount,const simFloat* ambient_diffuse,const simFloat* setToNULL,const simFloat* specular,const simFloat* emission)
{
    return(simAddParticleObject_internal(objectType,size,density,params,lifeTime,maxItemCount,ambient_diffuse,setToNULL,specular,emission));
}
VREP_DLLEXPORT simInt simRemoveParticleObject(simInt objectHandle)
{
    return(simRemoveParticleObject_internal(objectHandle));
}
VREP_DLLEXPORT simInt simAddParticleObjectItem(simInt objectHandle,const simFloat* itemData)
{
    return(simAddParticleObjectItem_internal(objectHandle,itemData));
}
VREP_DLLEXPORT simFloat simGetObjectSizeFactor(simInt objectHandle)
{
    return(simGetObjectSizeFactor_internal(objectHandle));
}
VREP_DLLEXPORT simInt simAnnounceSceneContentChange()
{
    return(simAnnounceSceneContentChange_internal());
}
VREP_DLLEXPORT simInt simResetMilling(simInt objectHandle)
{
    return(simResetMilling_internal(objectHandle));
}
VREP_DLLEXPORT simInt simApplyMilling(simInt objectHandle)
{
    return(simApplyMilling_internal(objectHandle));
}
VREP_DLLEXPORT simInt simSetIntegerSignal(const simChar* signalName,simInt signalValue)
{
    return(simSetIntegerSignal_internal(signalName,signalValue));
}
VREP_DLLEXPORT simInt simGetIntegerSignal(const simChar* signalName,simInt* signalValue)
{
    return(simGetIntegerSignal_internal(signalName,signalValue));
}
VREP_DLLEXPORT simInt simClearIntegerSignal(const simChar* signalName)
{
    return(simClearIntegerSignal_internal(signalName));
}
VREP_DLLEXPORT simInt simSetFloatSignal(const simChar* signalName,simFloat signalValue)
{
    return(simSetFloatSignal_internal(signalName,signalValue));
}
VREP_DLLEXPORT simInt simGetFloatSignal(const simChar* signalName,simFloat* signalValue)
{
    return(simGetFloatSignal_internal(signalName,signalValue));
}
VREP_DLLEXPORT simInt simClearFloatSignal(const simChar* signalName)
{
    return(simClearFloatSignal_internal(signalName));
}
VREP_DLLEXPORT simInt simSetDoubleSignal(const simChar* signalName,simDouble signalValue)
{
    return(simSetDoubleSignal_internal(signalName,signalValue));
}
VREP_DLLEXPORT simInt simGetDoubleSignal(const simChar* signalName,simDouble* signalValue)
{
    return(simGetDoubleSignal_internal(signalName,signalValue));
}
VREP_DLLEXPORT simInt simClearDoubleSignal(const simChar* signalName)
{
    return(simClearDoubleSignal_internal(signalName));
}
VREP_DLLEXPORT simInt simSetStringSignal(const simChar* signalName,const simChar* signalValue,simInt stringLength)
{
    return(simSetStringSignal_internal(signalName,signalValue,stringLength));
}
VREP_DLLEXPORT simChar* simGetStringSignal(const simChar* signalName,simInt* stringLength)
{
    return(simGetStringSignal_internal(signalName,stringLength));
}
VREP_DLLEXPORT simInt simClearStringSignal(const simChar* signalName)
{
    return(simClearStringSignal_internal(signalName));
}
VREP_DLLEXPORT simChar* simGetSignalName(simInt signalIndex,simInt signalType)
{
    return(simGetSignalName_internal(signalIndex,signalType));
}
VREP_DLLEXPORT simInt simSetObjectProperty(simInt objectHandle,simInt prop)
{
    return(simSetObjectProperty_internal(objectHandle,prop));
}
VREP_DLLEXPORT simInt simGetObjectProperty(simInt objectHandle)
{
    return(simGetObjectProperty_internal(objectHandle));
}
VREP_DLLEXPORT simInt simSetObjectSpecialProperty(simInt objectHandle,simInt prop)
{
    return(simSetObjectSpecialProperty_internal(objectHandle,prop));
}
VREP_DLLEXPORT simInt simGetObjectSpecialProperty(simInt objectHandle)
{
    return(simGetObjectSpecialProperty_internal(objectHandle));
}
VREP_DLLEXPORT simInt simGetPositionOnPath(simInt pathHandle,simFloat relativeDistance,simFloat* position)
{
    return(simGetPositionOnPath_internal(pathHandle,relativeDistance,position));
}
VREP_DLLEXPORT simInt simGetOrientationOnPath(simInt pathHandle,simFloat relativeDistance,simFloat* eulerAngles)
{
    return(simGetOrientationOnPath_internal(pathHandle,relativeDistance,eulerAngles));
}
VREP_DLLEXPORT simInt simGetDataOnPath(simInt pathHandle,simFloat relativeDistance,simInt dataType,simInt* intData,simFloat* floatData)
{
    return(simGetDataOnPath_internal(pathHandle,relativeDistance,dataType,intData,floatData));
}
VREP_DLLEXPORT simInt simGetClosestPositionOnPath(simInt pathHandle,simFloat* absolutePosition,simFloat* pathPosition)
{
    return(simGetClosestPositionOnPath_internal(pathHandle,absolutePosition,pathPosition));
}
VREP_DLLEXPORT simInt simReadForceSensor(simInt objectHandle,simFloat* forceVector,simFloat* torqueVector)
{
    return(simReadForceSensor_internal(objectHandle,forceVector,torqueVector));
}
VREP_DLLEXPORT simInt simBreakForceSensor(simInt objectHandle)
{
    return(simBreakForceSensor_internal(objectHandle));
}
VREP_DLLEXPORT simInt simGetShapeVertex(simInt shapeHandle,simInt groupElementIndex,simInt vertexIndex,simFloat* relativePosition)
{
    return(simGetShapeVertex_internal(shapeHandle,groupElementIndex,vertexIndex,relativePosition));
}
VREP_DLLEXPORT simInt simGetShapeTriangle(simInt shapeHandle,simInt groupElementIndex,simInt triangleIndex,simInt* vertexIndices,simFloat* triangleNormals)
{
    return(simGetShapeTriangle_internal(shapeHandle,groupElementIndex,triangleIndex,vertexIndices,triangleNormals));
}
VREP_DLLEXPORT simInt simSetLightParameters(simInt objectHandle,simInt state,const simFloat* setToNULL,const simFloat* diffusePart,const simFloat* specularPart)
{
    return(simSetLightParameters_internal(objectHandle,state,setToNULL,diffusePart,specularPart));
}
VREP_DLLEXPORT simInt simGetLightParameters(simInt objectHandle,simFloat* setToNULL,simFloat* diffusePart,simFloat* specularPart)
{
    return(simGetLightParameters_internal(objectHandle,setToNULL,diffusePart,specularPart));
}
VREP_DLLEXPORT simInt simGetVelocity(simInt shapeHandle,simFloat* linearVelocity,simFloat* angularVelocity)
{
    return(simGetVelocity_internal(shapeHandle,linearVelocity,angularVelocity));
}
VREP_DLLEXPORT simInt simGetObjectVelocity(simInt objectHandle,simFloat* linearVelocity,simFloat* angularVelocity)
{
    return(simGetObjectVelocity_internal(objectHandle,linearVelocity,angularVelocity));
}
VREP_DLLEXPORT simInt simAddForceAndTorque(simInt shapeHandle,const simFloat* force,const simFloat* torque)
{
    return(simAddForceAndTorque_internal(shapeHandle,force,torque));
}
VREP_DLLEXPORT simInt simAddForce(simInt shapeHandle,const simFloat* position,const simFloat* force)
{
    return(simAddForce_internal(shapeHandle,position,force));
}
VREP_DLLEXPORT simInt simSetExplicitHandling(simInt generalObjectHandle,int explicitFlags)
{
    return(simSetExplicitHandling_internal(generalObjectHandle,explicitFlags));
}
VREP_DLLEXPORT simInt simGetExplicitHandling(simInt generalObjectHandle)
{
    return(simGetExplicitHandling_internal(generalObjectHandle));
}
VREP_DLLEXPORT simInt simGetLinkDummy(simInt dummyHandle)
{
    return(simGetLinkDummy_internal(dummyHandle));
}
VREP_DLLEXPORT simInt simSetLinkDummy(simInt dummyHandle,simInt linkedDummyHandle)
{
    return(simSetLinkDummy_internal(dummyHandle,linkedDummyHandle));
}
VREP_DLLEXPORT simInt simSetModelProperty(simInt objectHandle,simInt modelProperty)
{
    return(simSetModelProperty_internal(objectHandle,modelProperty));
}
VREP_DLLEXPORT simInt simGetModelProperty(simInt objectHandle)
{
    return(simGetModelProperty_internal(objectHandle));
}
VREP_DLLEXPORT simInt simSetShapeColor(simInt shapeHandle,const simChar* colorName,simInt colorComponent,const simFloat* rgbData)
{
    return(simSetShapeColor_internal(shapeHandle,colorName,colorComponent,rgbData));
}
VREP_DLLEXPORT simInt simGetShapeColor(simInt shapeHandle,const simChar* colorName,simInt colorComponent,simFloat* rgbData)
{
    return(simGetShapeColor_internal(shapeHandle,colorName,colorComponent,rgbData));
}
VREP_DLLEXPORT simInt simResetDynamicObject(simInt objectHandle)
{
    return(simResetDynamicObject_internal(objectHandle));
}
VREP_DLLEXPORT simInt simSetJointMode(simInt jointHandle,simInt jointMode,simInt options)
{
    return(simSetJointMode_internal(jointHandle,jointMode,options));
}
VREP_DLLEXPORT simInt simGetJointMode(simInt jointHandle,simInt* options)
{
    return(simGetJointMode_internal(jointHandle,options));
}
VREP_DLLEXPORT simInt simSerialOpen(const simChar* portString,simInt baudRate,simVoid* reserved1,simVoid* reserved2)
{
    return(simSerialOpen_internal(portString,baudRate,reserved1,reserved2));
}
VREP_DLLEXPORT simInt simSerialClose(simInt portHandle)
{
    return(simSerialClose_internal(portHandle));
}
VREP_DLLEXPORT simInt simSerialSend(simInt portHandle,const simChar* data,simInt dataLength)
{
    return(simSerialSend_internal(portHandle,data,dataLength));
}
VREP_DLLEXPORT simInt simSerialRead(simInt portHandle,simChar* buffer,simInt dataLengthToRead)
{
    return(simSerialRead_internal(portHandle,buffer,dataLengthToRead));
}
VREP_DLLEXPORT simInt simSerialCheck(simInt portHandle)
{
    return(simSerialCheck_internal(portHandle));
}
VREP_DLLEXPORT simInt simGetContactInfo(simInt dynamicPass,simInt objectHandle,simInt index,simInt* objectHandles,simFloat* contactInfo)
{
    return(simGetContactInfo_internal(dynamicPass,objectHandle,index,objectHandles,contactInfo));
}
VREP_DLLEXPORT simInt simSetThreadIsFree(simBool freeMode)
{
    return(simSetThreadIsFree_internal(freeMode));
}
VREP_DLLEXPORT simInt simTubeOpen(simInt dataHeader,const simChar* dataName,simInt readBufferSize,simBool notUsedButKeepZero)
{
    return(simTubeOpen_internal(dataHeader,dataName,readBufferSize,notUsedButKeepZero));
}
VREP_DLLEXPORT simInt simTubeClose(simInt tubeHandle)
{
    return(simTubeClose_internal(tubeHandle));
}
VREP_DLLEXPORT simInt simTubeWrite(simInt tubeHandle,const simChar* data,simInt dataLength)
{
    return(simTubeWrite_internal(tubeHandle,data,dataLength));
}
VREP_DLLEXPORT simChar* simTubeRead(simInt tubeHandle,simInt* dataLength)
{
    return(simTubeRead_internal(tubeHandle,dataLength));
}
VREP_DLLEXPORT simInt simTubeStatus(simInt tubeHandle,simInt* readPacketsCount,simInt* writePacketsCount)
{
    return(simTubeStatus_internal(tubeHandle,readPacketsCount,writePacketsCount));
}
VREP_DLLEXPORT simInt simAuxiliaryConsoleOpen(const simChar* title,simInt maxLines,simInt mode,const simInt* position,const simInt* size,const simFloat* textColor,const simFloat* backgroundColor)
{
    return(simAuxiliaryConsoleOpen_internal(title,maxLines,mode,position,size,textColor,backgroundColor));
}
VREP_DLLEXPORT simInt simAuxiliaryConsoleClose(simInt consoleHandle)
{
    return(simAuxiliaryConsoleClose_internal(consoleHandle));
}
VREP_DLLEXPORT simInt simAuxiliaryConsoleShow(simInt consoleHandle,simBool showState)
{
    return(simAuxiliaryConsoleShow_internal(consoleHandle,showState));
}
VREP_DLLEXPORT simInt simAuxiliaryConsolePrint(simInt consoleHandle,const simChar* text)
{
    return(simAuxiliaryConsolePrint_internal(consoleHandle,text));
}
VREP_DLLEXPORT simInt simImportShape(simInt fileformat,const simChar* pathAndFilename,simInt options,simFloat identicalVerticeTolerance,simFloat scalingFactor)
{
    return(simImportShape_internal(fileformat,pathAndFilename,options,identicalVerticeTolerance,scalingFactor));
}
VREP_DLLEXPORT simInt simImportMesh(simInt fileformat,const simChar* pathAndFilename,simInt options,simFloat identicalVerticeTolerance,simFloat scalingFactor,simFloat*** vertices,simInt** verticesSizes,simInt*** indices,simInt** indicesSizes,simFloat*** reserved,simChar*** names)
{
    return(simImportMesh_internal(fileformat,pathAndFilename,options,identicalVerticeTolerance,scalingFactor,vertices,verticesSizes,indices,indicesSizes,reserved,names));
}
VREP_DLLEXPORT simInt simExportMesh(simInt fileformat,const simChar* pathAndFilename,simInt options,simFloat scalingFactor,simInt elementCount,const simFloat** vertices,const simInt* verticesSizes,const simInt** indices,const simInt* indicesSizes,simFloat** reserved,const simChar** names)
{
    return(simExportMesh_internal(fileformat,pathAndFilename,options,scalingFactor,elementCount,vertices,verticesSizes,indices,indicesSizes,reserved,names));
}
VREP_DLLEXPORT simInt simCreateMeshShape(simInt options,simFloat shadingAngle,const simFloat* vertices,simInt verticesSize,const simInt* indices,simInt indicesSize,simFloat* reserved)
{
    return(simCreateMeshShape_internal(options,shadingAngle,vertices,verticesSize,indices,indicesSize,reserved));
}
VREP_DLLEXPORT simInt simCreatePureShape(simInt primitiveType,simInt options,const simFloat* sizes,simFloat mass,const simInt* precision)
{
    return(simCreatePureShape_internal(primitiveType,options,sizes,mass,precision));
}
VREP_DLLEXPORT simInt simCreateHeightfieldShape(simInt options,simFloat shadingAngle,simInt xPointCount,simInt yPointCount,simFloat xSize,const simFloat* heights)
{
    return(simCreateHeightfieldShape_internal(options,shadingAngle,xPointCount,yPointCount,xSize,heights));
}
VREP_DLLEXPORT simInt simGetShapeMesh(simInt shapeHandle,simFloat** vertices,simInt* verticesSize,simInt** indices,simInt* indicesSize,simFloat** normals)
{
    return(simGetShapeMesh_internal(shapeHandle,vertices,verticesSize,indices,indicesSize,normals));
}
VREP_DLLEXPORT simInt simAddBanner(const simChar* label,simFloat size,simInt options,const simFloat* positionAndEulerAngles,simInt parentObjectHandle,const simFloat* labelColors,const simFloat* backgroundColors)
{
    return(simAddBanner_internal(label,size,options,positionAndEulerAngles,parentObjectHandle,labelColors,backgroundColors));
}
VREP_DLLEXPORT simInt simRemoveBanner(simInt bannerID)
{
    return(simRemoveBanner_internal(bannerID));
}
VREP_DLLEXPORT simInt simCreateJoint(simInt jointType,simInt jointMode,simInt options,const simFloat* sizes,const simFloat* colorA,const simFloat* colorB)
{
    return(simCreateJoint_internal(jointType,jointMode,options,sizes,colorA,colorB));
}
VREP_DLLEXPORT simInt simGetObjectIntParameter(simInt objectHandle,simInt parameterID,simInt* parameter)
{
    return(simGetObjectInt32Parameter_internal(objectHandle,parameterID,parameter));
}
VREP_DLLEXPORT simInt simSetObjectIntParameter(simInt objectHandle,simInt parameterID,simInt parameter)
{
    return(simSetObjectInt32Parameter_internal(objectHandle,parameterID,parameter));
}
VREP_DLLEXPORT simInt simGetObjectInt32Parameter(simInt objectHandle,simInt parameterID,simInt* parameter)
{
    return(simGetObjectInt32Parameter_internal(objectHandle,parameterID,parameter));
}
VREP_DLLEXPORT simInt simSetObjectInt32Parameter(simInt objectHandle,simInt parameterID,simInt parameter)
{
    return(simSetObjectInt32Parameter_internal(objectHandle,parameterID,parameter));
}
VREP_DLLEXPORT simInt simGetObjectFloatParameter(simInt objectHandle,simInt parameterID,simFloat* parameter)
{
    return(simGetObjectFloatParameter_internal(objectHandle,parameterID,parameter));
}
VREP_DLLEXPORT simInt simSetObjectFloatParameter(simInt objectHandle,simInt parameterID,simFloat parameter)
{
    return(simSetObjectFloatParameter_internal(objectHandle,parameterID,parameter));
}
VREP_DLLEXPORT simChar* simGetObjectStringParameter(simInt objectHandle,simInt parameterID,simInt* parameterLength)
{
    return(simGetObjectStringParameter_internal(objectHandle,parameterID,parameterLength));
}
VREP_DLLEXPORT simInt simSetObjectStringParameter(simInt objectHandle,simInt parameterID,simChar* parameter,simInt parameterLength)
{
    return(simSetObjectStringParameter_internal(objectHandle,parameterID,parameter,parameterLength));
}
VREP_DLLEXPORT simInt simSetSimulationPassesPerRenderingPass(simInt p)
{
    return(simSetSimulationPassesPerRenderingPass_internal(p));
}
VREP_DLLEXPORT simInt simGetRotationAxis(const simFloat* matrixStart,const simFloat* matrixGoal,simFloat* axis,simFloat* angle)
{
    return(simGetRotationAxis_internal(matrixStart,matrixGoal,axis,angle));
}
VREP_DLLEXPORT simInt simRotateAroundAxis(const simFloat* matrixIn,const simFloat* axis,const simFloat* axisPos,simFloat angle,simFloat* matrixOut)
{
    return(simRotateAroundAxis_internal(matrixIn,axis,axisPos,angle,matrixOut));
}
VREP_DLLEXPORT simInt simGetJointForce(simInt jointHandle,simFloat* forceOrTorque)
{
    return(simGetJointForce_internal(jointHandle,forceOrTorque));
}
VREP_DLLEXPORT simInt simSetArrayParameter(simInt parameter,const simVoid* arrayOfValues)
{
    return(simSetArrayParameter_internal(parameter,arrayOfValues));
}
VREP_DLLEXPORT simInt simGetArrayParameter(simInt parameter,simVoid* arrayOfValues)
{
    return(simGetArrayParameter_internal(parameter,arrayOfValues));
}
VREP_DLLEXPORT simInt simSetIkGroupProperties(simInt ikGroupHandle,simInt resolutionMethod,simInt maxIterations,simFloat damping,void* reserved)
{
    return(simSetIkGroupProperties_internal(ikGroupHandle,resolutionMethod,maxIterations,damping,reserved));
}
VREP_DLLEXPORT simInt simSetIkElementProperties(simInt ikGroupHandle,simInt tipDummyHandle,simInt constraints,const simFloat* precision,const simFloat* weight,void* reserved)
{
    return(simSetIkElementProperties_internal(ikGroupHandle,tipDummyHandle,constraints,precision,weight,reserved));
}
VREP_DLLEXPORT simInt simCameraFitToView(simInt viewHandleOrIndex,simInt objectCount,const simInt* objectHandles,simInt options,simFloat scaling)
{
    return(simCameraFitToView_internal(viewHandleOrIndex,objectCount,objectHandles,options,scaling));
}
VREP_DLLEXPORT simInt simPersistentDataWrite(const simChar* dataName,const simChar* dataValue,simInt dataLength,simInt options)
{
    return(simPersistentDataWrite_internal(dataName,dataValue,dataLength,options));
}
VREP_DLLEXPORT simChar* simPersistentDataRead(const simChar* dataName,simInt* dataLength)
{
    return(simPersistentDataRead_internal(dataName,dataLength));
}
VREP_DLLEXPORT simInt simIsHandleValid(simInt generalObjectHandle,simInt generalObjectType)
{
    return(simIsHandleValid_internal(generalObjectHandle,generalObjectType));
}
VREP_DLLEXPORT simInt simHandleVisionSensor(simInt visionSensorHandle,simFloat** auxValues,simInt** auxValuesCount)
{
    return(simHandleVisionSensor_internal(visionSensorHandle,auxValues,auxValuesCount));
}
VREP_DLLEXPORT simInt simReadVisionSensor(simInt visionSensorHandle,simFloat** auxValues,simInt** auxValuesCount)
{
    return(simReadVisionSensor_internal(visionSensorHandle,auxValues,auxValuesCount));
}
VREP_DLLEXPORT simInt simResetVisionSensor(simInt visionSensorHandle)
{
    return(simResetVisionSensor_internal(visionSensorHandle));
}
VREP_DLLEXPORT simInt simCheckVisionSensor(simInt visionSensorHandle,simInt entityHandle,simFloat** auxValues,simInt** auxValuesCount)
{
    return(simCheckVisionSensor_internal(visionSensorHandle,entityHandle,auxValues,auxValuesCount));
}
VREP_DLLEXPORT simFloat* simCheckVisionSensorEx(simInt visionSensorHandle,simInt entityHandle,simBool returnImage)
{
    return(simCheckVisionSensorEx_internal(visionSensorHandle,entityHandle,returnImage));
}
VREP_DLLEXPORT simInt simGetVisionSensorResolution(simInt visionSensorHandle,simInt* resolution)
{
    return(simGetVisionSensorResolution_internal(visionSensorHandle,resolution));
}
VREP_DLLEXPORT simFloat* simGetVisionSensorImage(simInt visionSensorHandle)
{
    return(simGetVisionSensorImage_internal(visionSensorHandle));
}
VREP_DLLEXPORT simUChar* simGetVisionSensorCharImage(simInt visionSensorHandle,simInt* resolutionX,simInt* resolutionY)
{
    return(simGetVisionSensorCharImage_internal(visionSensorHandle,resolutionX,resolutionY));
}
VREP_DLLEXPORT simInt simSetVisionSensorImage(simInt visionSensorHandle,const simFloat* image)
{
    return(simSetVisionSensorImage_internal(visionSensorHandle,image));
}
VREP_DLLEXPORT simInt simSetVisionSensorCharImage(simInt visionSensorHandle,const simUChar* image)
{
    return(simSetVisionSensorCharImage_internal(visionSensorHandle,image));
}
VREP_DLLEXPORT simFloat* simGetVisionSensorDepthBuffer(simInt visionSensorHandle)
{
    return(simGetVisionSensorDepthBuffer_internal(visionSensorHandle));
}
VREP_DLLEXPORT simInt simRMLPosition(simInt dofs,simDouble timeStep,simInt flags,const simDouble* currentPosVelAccel,const simDouble* maxVelAccelJerk,const simBool* selection,const simDouble* targetPosVel,simDouble* newPosVelAccel,simVoid* auxData)
{
    return(simRMLPosition_internal(dofs,timeStep,flags,currentPosVelAccel,maxVelAccelJerk,selection,targetPosVel,newPosVelAccel,auxData));
}
VREP_DLLEXPORT simInt simRMLVelocity(simInt dofs,simDouble timeStep,simInt flags,const simDouble* currentPosVelAccel,const simDouble* maxAccelJerk,const simBool* selection,const simDouble* targetVel,simDouble* newPosVelAccel,simVoid* auxData)
{
    return(simRMLVelocity_internal(dofs,timeStep,flags,currentPosVelAccel,maxAccelJerk,selection,targetVel,newPosVelAccel,auxData));
}
VREP_DLLEXPORT simInt simRMLPos(simInt dofs,simDouble smallestTimeStep,simInt flags,const simDouble* currentPosVelAccel,const simDouble* maxVelAccelJerk,const simBool* selection,const simDouble* targetPosVel,simVoid* auxData)
{
    return(simRMLPos_internal(dofs,smallestTimeStep,flags,currentPosVelAccel,maxVelAccelJerk,selection,targetPosVel,auxData));
}
VREP_DLLEXPORT simInt simRMLVel(simInt dofs,simDouble smallestTimeStep,simInt flags,const simDouble* currentPosVelAccel,const simDouble* maxAccelJerk,const simBool* selection,const simDouble* targetVel,simVoid* auxData)
{
    return(simRMLVel_internal(dofs,smallestTimeStep,flags,currentPosVelAccel,maxAccelJerk,selection,targetVel,auxData));
}
VREP_DLLEXPORT simInt simRMLStep(simInt handle,simDouble timeStep,simDouble* newPosVelAccel,simVoid* auxData,simVoid* reserved)
{
    return(simRMLStep_internal(handle,timeStep,newPosVelAccel,auxData,reserved));
}
VREP_DLLEXPORT simInt simRMLRemove(simInt handle)
{
    return(simRMLRemove_internal(handle));
}
VREP_DLLEXPORT simChar* simFileDialog(simInt mode,const simChar* title,const simChar* startPath,const simChar* initName,const simChar* extName,const simChar* ext)
{
    return(simFileDialog_internal(mode,title,startPath,initName,extName,ext));
}
VREP_DLLEXPORT simInt simMsgBox(simInt dlgType,simInt buttons,const simChar* title,const simChar* message)
{
    return(simMsgBox_internal(dlgType,buttons,title,message));
}
VREP_DLLEXPORT simInt simCreateDummy(simFloat size,const simFloat* color)
{
    return(simCreateDummy_internal(size,color));
}
VREP_DLLEXPORT simInt simSetShapeMassAndInertia(simInt shapeHandle,simFloat mass,const simFloat* inertiaMatrix,const simFloat* centerOfMass,const simFloat* transformation)
{
    return(simSetShapeMassAndInertia_internal(shapeHandle,mass,inertiaMatrix,centerOfMass,transformation));
}
VREP_DLLEXPORT simInt simGetShapeMassAndInertia(simInt shapeHandle,simFloat* mass,simFloat* inertiaMatrix,simFloat* centerOfMass,const simFloat* transformation)
{
    return(simGetShapeMassAndInertia_internal(shapeHandle,mass,inertiaMatrix,centerOfMass,transformation));
}
VREP_DLLEXPORT simInt simGroupShapes(const simInt* shapeHandles,simInt shapeCount)
{
    return(simGroupShapes_internal(shapeHandles,shapeCount));
}
VREP_DLLEXPORT simInt* simUngroupShape(simInt shapeHandle,simInt* shapeCount)
{
    return(simUngroupShape_internal(shapeHandle,shapeCount));
}
VREP_DLLEXPORT simInt simCreateProximitySensor(simInt sensorType,simInt subType,simInt options,const simInt* intParams,const simFloat* floatParams,const simFloat* color)
{
    return(simCreateProximitySensor_internal(sensorType,subType,options,intParams,floatParams,color));
}
VREP_DLLEXPORT simInt simCreateForceSensor(simInt options,const simInt* intParams,const simFloat* floatParams,const simFloat* color)
{
    return(simCreateForceSensor_internal(options,intParams,floatParams,color));
}
VREP_DLLEXPORT simInt simCreateVisionSensor(simInt options,const simInt* intParams,const simFloat* floatParams,const simFloat* color)
{
    return(simCreateVisionSensor_internal(options,intParams,floatParams,color));
}
VREP_DLLEXPORT simInt simConvexDecompose(simInt shapeHandle,simInt options,const simInt* intParams,const simFloat* floatParams)
{
    return(simConvexDecompose_internal(shapeHandle,options,intParams,floatParams));
}
VREP_DLLEXPORT simInt simCreatePath(simInt attributes,const simInt* intParams,const simFloat* floatParams,const simFloat* color)
{
    return(simCreatePath_internal(attributes,intParams,floatParams,color));
}
VREP_DLLEXPORT simInt simInsertPathCtrlPoints(simInt pathHandle,simInt options,simInt startIndex,simInt ptCnt,const simVoid* ptData)
{
    return(simInsertPathCtrlPoints_internal(pathHandle,options,startIndex,ptCnt,ptData));
}
VREP_DLLEXPORT simInt simCutPathCtrlPoints(simInt pathHandle,simInt startIndex,simInt ptCnt)
{
    return(simCutPathCtrlPoints_internal(pathHandle,startIndex,ptCnt));
}
VREP_DLLEXPORT simFloat* simGetIkGroupMatrix(simInt ikGroupHandle,simInt options,simInt* matrixSize)
{
    return(simGetIkGroupMatrix_internal(ikGroupHandle,options,matrixSize));
}
VREP_DLLEXPORT simInt simAddGhost(simInt ghostGroup,simInt objectHandle,simInt options,simFloat startTime,simFloat endTime,const simFloat* color)
{
    return(simAddGhost_internal(ghostGroup,objectHandle,options,startTime,endTime,color));
}
VREP_DLLEXPORT simInt simModifyGhost(simInt ghostGroup,simInt ghostId,simInt operation,simFloat floatValue,simInt options,simInt optionsMask,const simFloat* colorOrTransformation)
{
    return(simModifyGhost_internal(ghostGroup,ghostId,operation,floatValue,options,optionsMask,colorOrTransformation));
}
VREP_DLLEXPORT simVoid simQuitSimulator(simBool ignoredArgument)
{
    simQuitSimulator_internal(ignoredArgument);
}
VREP_DLLEXPORT simInt simGetThreadId()
{
    return(simGetThreadId_internal());
}
VREP_DLLEXPORT simInt simLockResources(simInt lockType,simInt reserved)
{
    return(simLockResources_internal(lockType,reserved));
}
VREP_DLLEXPORT simInt simUnlockResources(simInt lockHandle)
{
    return(simUnlockResources_internal(lockHandle));
}
VREP_DLLEXPORT simInt simEnableEventCallback(simInt eventCallbackType,const simChar* plugin,simInt reserved)
{
    return(simEnableEventCallback_internal(eventCallbackType,plugin,reserved));
}
VREP_DLLEXPORT simInt simSetShapeMaterial(simInt shapeHandle,simInt materialIdOrShapeHandle)
{
    return(simSetShapeMaterial_internal(shapeHandle,materialIdOrShapeHandle));
}
VREP_DLLEXPORT simInt simGetTextureId(const simChar* textureName,simInt* resolution)
{
    return(simGetTextureId_internal(textureName,resolution));
}
VREP_DLLEXPORT simChar* simReadTexture(simInt textureId,simInt options,simInt posX,simInt posY,simInt sizeX,simInt sizeY)
{
    return(simReadTexture_internal(textureId,options,posX,posY,sizeX,sizeY));
}
VREP_DLLEXPORT simInt simWriteTexture(simInt textureId,simInt options,const simChar* data,simInt posX,simInt posY,simInt sizeX,simInt sizeY,simFloat interpol)
{
    return(simWriteTexture_internal(textureId,options,data,posX,posY,sizeX,sizeY,interpol));
}
VREP_DLLEXPORT simInt simCreateTexture(const simChar* fileName,simInt options,const simFloat* planeSizes,const simFloat* scalingUV,const simFloat* xy_g,simInt fixedResolution,simInt* textureId,simInt* resolution,const simVoid* reserved)
{
    return(simCreateTexture_internal(fileName,options,planeSizes,scalingUV,xy_g,fixedResolution,textureId,resolution,reserved));
}
VREP_DLLEXPORT simInt simWriteCustomDataBlock(simInt objectHandle,const simChar* tagName,const simChar* data,simInt dataSize)
{
    return(simWriteCustomDataBlock_internal(objectHandle,tagName,data,dataSize));
}
VREP_DLLEXPORT simChar* simReadCustomDataBlock(simInt objectHandle,const simChar* tagName,simInt* dataSize)
{
    return(simReadCustomDataBlock_internal(objectHandle,tagName,dataSize));
}
VREP_DLLEXPORT simChar* simReadCustomDataBlockTags(simInt objectHandle,simInt* tagCount)
{
    return(simReadCustomDataBlockTags_internal(objectHandle,tagCount));
}
VREP_DLLEXPORT simInt simAddPointCloud(simInt pageMask,simInt layerMask,simInt objectHandle,simInt options,simFloat pointSize,simInt ptCnt,const simFloat* pointCoordinates,const simChar* defaultColors,const simChar* pointColors,const simFloat* pointNormals)
{
    return(simAddPointCloud_internal(pageMask,layerMask,objectHandle,options,pointSize,ptCnt,pointCoordinates,defaultColors,pointColors,pointNormals));
}
VREP_DLLEXPORT simInt simModifyPointCloud(simInt pointCloudHandle,simInt operation,const simInt* intParam,const simFloat* floatParam)
{
    return(simModifyPointCloud_internal(pointCloudHandle,operation,intParam,floatParam));
}
VREP_DLLEXPORT simInt simGetShapeGeomInfo(simInt shapeHandle,simInt* intData,simFloat* floatData,simVoid* reserved)
{
    return(simGetShapeGeomInfo_internal(shapeHandle,intData,floatData,reserved));
}
VREP_DLLEXPORT simInt* simGetObjectsInTree(simInt treeBaseHandle,simInt objectType,simInt options,simInt* objectCount)
{
    return(simGetObjectsInTree_internal(treeBaseHandle,objectType,options,objectCount));
}
VREP_DLLEXPORT simInt simSetObjectSizeValues(simInt objectHandle,const simFloat* sizeValues)
{
    return(simSetObjectSizeValues_internal(objectHandle,sizeValues));
}
VREP_DLLEXPORT simInt simGetObjectSizeValues(simInt objectHandle,simFloat* sizeValues)
{
    return(simGetObjectSizeValues_internal(objectHandle,sizeValues));
}
VREP_DLLEXPORT simInt simScaleObject(simInt objectHandle,simFloat xScale,simFloat yScale,simFloat zScale,simInt options)
{
    return(simScaleObject_internal(objectHandle,xScale,yScale,zScale,options));
}
VREP_DLLEXPORT simInt simSetShapeTexture(simInt shapeHandle,simInt textureId,simInt mappingMode,simInt options,const simFloat* uvScaling,const simFloat* position,const simFloat* orientation)
{
    return(simSetShapeTexture_internal(shapeHandle,textureId,mappingMode,options,uvScaling,position,orientation));
}
VREP_DLLEXPORT simInt simGetShapeTextureId(simInt shapeHandle)
{
    return(simGetShapeTextureId_internal(shapeHandle));
}
VREP_DLLEXPORT simInt* simGetCollectionObjects(simInt collectionHandle,simInt* objectCount)
{
    return(simGetCollectionObjects_internal(collectionHandle,objectCount));
}
VREP_DLLEXPORT simInt simSetScriptAttribute(simInt scriptHandle,simInt attributeID,simFloat floatVal,simInt intOrBoolVal)
{
    return(simSetScriptAttribute_internal(scriptHandle,attributeID,floatVal,intOrBoolVal));
}
VREP_DLLEXPORT simInt simGetScriptAttribute(simInt scriptHandle,simInt attributeID,simFloat* floatVal,simInt* intOrBoolVal)
{
    return(simGetScriptAttribute_internal(scriptHandle,attributeID,floatVal,intOrBoolVal));
}
VREP_DLLEXPORT simInt simReorientShapeBoundingBox(simInt shapeHandle,simInt relativeToHandle,simInt reservedSetToZero)
{
    return(simReorientShapeBoundingBox_internal(shapeHandle,relativeToHandle,reservedSetToZero));
}
VREP_DLLEXPORT simInt simSwitchThread()
{
    return(simSwitchThread_internal());
}

VREP_DLLEXPORT simInt simCreateIkGroup(simInt options,const simInt* intParams,const simFloat* floatParams,const simVoid* reserved)
{
    return(simCreateIkGroup_internal(options,intParams,floatParams,reserved));
}
VREP_DLLEXPORT simInt simRemoveIkGroup(simInt ikGroupHandle)
{
    return(simRemoveIkGroup_internal(ikGroupHandle));
}
VREP_DLLEXPORT simInt simCreateIkElement(simInt ikGroupHandle,simInt options,const simInt* intParams,const simFloat* floatParams,const simVoid* reserved)
{
    return(simCreateIkElement_internal(ikGroupHandle,options,intParams,floatParams,reserved));
}
VREP_DLLEXPORT simInt simCreateCollection(const simChar* collectionName,simInt options)
{
    return(simCreateCollection_internal(collectionName,options));
}
VREP_DLLEXPORT simInt simAddObjectToCollection(simInt collectionHandle,simInt objectHandle,simInt what,simInt options)
{
    return(simAddObjectToCollection_internal(collectionHandle,objectHandle,what,options));
}
VREP_DLLEXPORT simInt simSaveImage(const simUChar* image,const simInt* resolution,simInt options,const simChar* filename,simInt quality,simVoid* reserved)
{
    return(simSaveImage_internal(image,resolution,options,filename,quality,reserved));
}
VREP_DLLEXPORT simUChar* simLoadImage(simInt* resolution,simInt options,const simChar* filename,simVoid* reserved)
{
    return(simLoadImage_internal(resolution,options,filename,reserved));
}
VREP_DLLEXPORT simUChar* simGetScaledImage(const simUChar* imageIn,const simInt* resolutionIn,simInt* resolutionOut,simInt options,simVoid* reserved)
{
    return(simGetScaledImage_internal(imageIn,resolutionIn,resolutionOut,options,reserved));
}
VREP_DLLEXPORT simInt simTransformImage(simUChar* image,const simInt* resolution,simInt options,const simFloat* floatParams,const simInt* intParams,simVoid* reserved)
{
    return(simTransformImage_internal(image,resolution,options,floatParams,intParams,reserved));
}


VREP_DLLEXPORT simInt simGetQHull(const simFloat* inVertices,simInt inVerticesL,simFloat** verticesOut,simInt* verticesOutL,simInt** indicesOut,simInt* indicesOutL,simInt reserved1,const simFloat* reserved2)
{
    return(simGetQHull_internal(inVertices,inVerticesL,verticesOut,verticesOutL,indicesOut,indicesOutL,reserved1,reserved2));
}
VREP_DLLEXPORT simInt simGetDecimatedMesh(const simFloat* inVertices,simInt inVerticesL,const simInt* inIndices,simInt inIndicesL,simFloat** verticesOut,simInt* verticesOutL,simInt** indicesOut,simInt* indicesOutL,simFloat decimationPercent,simInt reserved1,const simFloat* reserved2)
{
    return(simGetDecimatedMesh_internal(inVertices,inVerticesL,inIndices,inIndicesL,verticesOut,verticesOutL,indicesOut,indicesOutL,decimationPercent,reserved1,reserved2));
}
VREP_DLLEXPORT simInt simExportIk(const simChar* pathAndFilename,simInt reserved1,simVoid* reserved2)
{
    return(simExportIk_internal(pathAndFilename,reserved1,reserved2));
}
VREP_DLLEXPORT simInt simCallScriptFunctionEx(simInt scriptHandleOrType,const simChar* functionNameAtScriptName,simInt stackId)
{
    return(simCallScriptFunctionEx_internal(scriptHandleOrType,functionNameAtScriptName,stackId));
}
VREP_DLLEXPORT simInt simComputeJacobian(simInt ikGroupHandle,simInt options,simVoid* reserved)
{
    return(simComputeJacobian_internal(ikGroupHandle,options,reserved));
}
VREP_DLLEXPORT simInt simGetConfigForTipPose(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simFloat thresholdDist,simInt maxTimeInMs,simFloat* retConfig,const simFloat* metric,simInt collisionPairCnt,const simInt* collisionPairs,const simInt* jointOptions,const simFloat* lowLimits,const simFloat* ranges,simVoid* reserved)
{
    return(simGetConfigForTipPose_internal(ikGroupHandle,jointCnt,jointHandles,thresholdDist,maxTimeInMs,retConfig,metric,collisionPairCnt,collisionPairs,jointOptions,lowLimits,ranges,reserved));
}
VREP_DLLEXPORT simFloat* simGenerateIkPath(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simInt ptCnt,simInt collisionPairCnt,const simInt* collisionPairs,const simInt* jointOptions,simVoid* reserved)
{
    return(simGenerateIkPath_internal(ikGroupHandle,jointCnt,jointHandles,ptCnt,collisionPairCnt,collisionPairs,jointOptions,reserved));
}
VREP_DLLEXPORT simChar* simGetExtensionString(simInt objectHandle,simInt index,const char* key)
{
    return(simGetExtensionString_internal(objectHandle,index,key));
}
VREP_DLLEXPORT simInt simComputeMassAndInertia(simInt shapeHandle,simFloat density)
{
    return(simComputeMassAndInertia_internal(shapeHandle,density));
}
VREP_DLLEXPORT simInt simCreateStack()
{
    return(simCreateStack_internal());
}
VREP_DLLEXPORT simInt simReleaseStack(simInt stackHandle)
{
    return(simReleaseStack_internal(stackHandle));
}
VREP_DLLEXPORT simInt simCopyStack(simInt stackHandle)
{
    return(simCopyStack_internal(stackHandle));
}
VREP_DLLEXPORT simInt simPushNullOntoStack(simInt stackHandle)
{
    return(simPushNullOntoStack_internal(stackHandle));
}
VREP_DLLEXPORT simInt simPushBoolOntoStack(simInt stackHandle,simBool value)
{
    return(simPushBoolOntoStack_internal(stackHandle,value));
}
VREP_DLLEXPORT simInt simPushInt32OntoStack(simInt stackHandle,simInt value)
{
    return(simPushInt32OntoStack_internal(stackHandle,value));
}
VREP_DLLEXPORT simInt simPushFloatOntoStack(simInt stackHandle,simFloat value)
{
    return(simPushFloatOntoStack_internal(stackHandle,value));
}
VREP_DLLEXPORT simInt simPushDoubleOntoStack(simInt stackHandle,simDouble value)
{
    return(simPushDoubleOntoStack_internal(stackHandle,value));
}
VREP_DLLEXPORT simInt simPushStringOntoStack(simInt stackHandle,const simChar* value,simInt stringSize)
{
    return(simPushStringOntoStack_internal(stackHandle,value,stringSize));
}
VREP_DLLEXPORT simInt simPushUInt8TableOntoStack(simInt stackHandle,const simUChar* values,simInt valueCnt)
{
    return(simPushUInt8TableOntoStack_internal(stackHandle,values,valueCnt));
}
VREP_DLLEXPORT simInt simPushInt32TableOntoStack(simInt stackHandle,const simInt* values,simInt valueCnt)
{
    return(simPushInt32TableOntoStack_internal(stackHandle,values,valueCnt));
}
VREP_DLLEXPORT simInt simPushFloatTableOntoStack(simInt stackHandle,const simFloat* values,simInt valueCnt)
{
    return(simPushFloatTableOntoStack_internal(stackHandle,values,valueCnt));
}
VREP_DLLEXPORT simInt simPushDoubleTableOntoStack(simInt stackHandle,const simDouble* values,simInt valueCnt)
{
    return(simPushDoubleTableOntoStack_internal(stackHandle,values,valueCnt));
}
VREP_DLLEXPORT simInt simPushTableOntoStack(simInt stackHandle)
{
    return(simPushTableOntoStack_internal(stackHandle));
}
VREP_DLLEXPORT simInt simInsertDataIntoStackTable(simInt stackHandle)
{
    return(simInsertDataIntoStackTable_internal(stackHandle));
}
VREP_DLLEXPORT simInt simGetStackSize(simInt stackHandle)
{
    return(simGetStackSize_internal(stackHandle));
}
VREP_DLLEXPORT simInt simPopStackItem(simInt stackHandle,simInt count)
{
    return(simPopStackItem_internal(stackHandle,count));
}
VREP_DLLEXPORT simInt simMoveStackItemToTop(simInt stackHandle,simInt cIndex)
{
    return(simMoveStackItemToTop_internal(stackHandle,cIndex));
}
VREP_DLLEXPORT simInt simIsStackValueNull(simInt stackHandle)
{
    return(simIsStackValueNull_internal(stackHandle));
}
VREP_DLLEXPORT simInt simGetStackBoolValue(simInt stackHandle,simBool* boolValue)
{
    return(simGetStackBoolValue_internal(stackHandle,boolValue));
}
VREP_DLLEXPORT simInt simGetStackInt32Value(simInt stackHandle,simInt* numberValue)
{
    return(simGetStackInt32Value_internal(stackHandle,numberValue));
}
VREP_DLLEXPORT simInt simGetStackFloatValue(simInt stackHandle,simFloat* numberValue)
{
    return(simGetStackFloatValue_internal(stackHandle,numberValue));
}
VREP_DLLEXPORT simInt simGetStackDoubleValue(simInt stackHandle,simDouble* numberValue)
{
    return(simGetStackDoubleValue_internal(stackHandle,numberValue));
}
VREP_DLLEXPORT simChar* simGetStackStringValue(simInt stackHandle,simInt* stringSize)
{
    return(simGetStackStringValue_internal(stackHandle,stringSize));
}
VREP_DLLEXPORT simInt simGetStackTableInfo(simInt stackHandle,simInt infoType)
{
    return(simGetStackTableInfo_internal(stackHandle,infoType));
}
VREP_DLLEXPORT simInt simGetStackUInt8Table(simInt stackHandle,simUChar* array,simInt count)
{
    return(simGetStackUInt8Table_internal(stackHandle,array,count));
}
VREP_DLLEXPORT simInt simGetStackInt32Table(simInt stackHandle,simInt* array,simInt count)
{
    return(simGetStackInt32Table_internal(stackHandle,array,count));
}
VREP_DLLEXPORT simInt simGetStackFloatTable(simInt stackHandle,simFloat* array,simInt count)
{
    return(simGetStackFloatTable_internal(stackHandle,array,count));
}
VREP_DLLEXPORT simInt simGetStackDoubleTable(simInt stackHandle,simDouble* array,simInt count)
{
    return(simGetStackDoubleTable_internal(stackHandle,array,count));
}
VREP_DLLEXPORT simInt simUnfoldStackTable(simInt stackHandle)
{
    return(simUnfoldStackTable_internal(stackHandle));
}
VREP_DLLEXPORT simInt simDebugStack(simInt stackHandle,simInt cIndex)
{
    return(simDebugStack_internal(stackHandle,cIndex));
}
VREP_DLLEXPORT simInt simSetScriptVariable(simInt scriptHandleOrType,const simChar* variableNameAtScriptName,simInt stackHandle)
{
    return(simSetScriptVariable_internal(scriptHandleOrType,variableNameAtScriptName,stackHandle));
}

VREP_DLLEXPORT simFloat simGetEngineFloatParameter(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{
    return(simGetEngineFloatParameter_internal(paramId,objectHandle,object,ok));
}
VREP_DLLEXPORT simInt simGetEngineInt32Parameter(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{
    return(simGetEngineInt32Parameter_internal(paramId,objectHandle,object,ok));
}
VREP_DLLEXPORT simBool simGetEngineBoolParameter(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{
    return(simGetEngineBoolParameter_internal(paramId,objectHandle,object,ok));
}
VREP_DLLEXPORT simInt simSetEngineFloatParameter(simInt paramId,simInt objectHandle,const simVoid* object,simFloat val)
{
    return(simSetEngineFloatParameter_internal(paramId,objectHandle,object,val));
}
VREP_DLLEXPORT simInt simSetEngineInt32Parameter(simInt paramId,simInt objectHandle,const simVoid* object,simInt val)
{
    return(simSetEngineInt32Parameter_internal(paramId,objectHandle,object,val));
}
VREP_DLLEXPORT simInt simSetEngineBoolParameter(simInt paramId,simInt objectHandle,const simVoid* object,simBool val)
{
    return(simSetEngineBoolParameter_internal(paramId,objectHandle,object,val));
}
VREP_DLLEXPORT simInt simCreateOctree(simFloat voxelSize,simInt options,simFloat pointSize,simVoid* reserved)
{
    return(simCreateOctree_internal(voxelSize,options,pointSize,reserved));
}
VREP_DLLEXPORT simInt simCreatePointCloud(simFloat maxVoxelSize,simInt maxPtCntPerVoxel,simInt options,simFloat pointSize,simVoid* reserved)
{
    return(simCreatePointCloud_internal(maxVoxelSize,maxPtCntPerVoxel,options,pointSize,reserved));
}
VREP_DLLEXPORT simInt simSetPointCloudOptions(simInt pointCloudHandle,simFloat maxVoxelSize,simInt maxPtCntPerVoxel,simInt options,simFloat pointSize,simVoid* reserved)
{
    return(simSetPointCloudOptions_internal(pointCloudHandle,maxVoxelSize,maxPtCntPerVoxel,options,pointSize,reserved));
}
VREP_DLLEXPORT simInt simGetPointCloudOptions(simInt pointCloudHandle,simFloat* maxVoxelSize,simInt* maxPtCntPerVoxel,simInt* options,simFloat* pointSize,simVoid* reserved)
{
    return(simGetPointCloudOptions_internal(pointCloudHandle,maxVoxelSize,maxPtCntPerVoxel,options,pointSize,reserved));
}
VREP_DLLEXPORT simInt simInsertVoxelsIntoOctree(simInt octreeHandle,simInt options,const simFloat* pts,simInt ptCnt,const simUChar* color,const simUInt* tag,simVoid* reserved)
{
    return(simInsertVoxelsIntoOctree_internal(octreeHandle,options,pts,ptCnt,color,tag,reserved));
}
VREP_DLLEXPORT simInt simRemoveVoxelsFromOctree(simInt octreeHandle,simInt options,const simFloat* pts,simInt ptCnt,simVoid* reserved)
{
    return(simRemoveVoxelsFromOctree_internal(octreeHandle,options,pts,ptCnt,reserved));
}
VREP_DLLEXPORT simInt simInsertPointsIntoPointCloud(simInt pointCloudHandle,simInt options,const simFloat* pts,simInt ptCnt,const simUChar* color,simVoid* optionalValues)
{
    return(simInsertPointsIntoPointCloud_internal(pointCloudHandle,options,pts,ptCnt,color,optionalValues));
}
VREP_DLLEXPORT simInt simRemovePointsFromPointCloud(simInt pointCloudHandle,simInt options,const simFloat* pts,simInt ptCnt,simFloat tolerance,simVoid* reserved)
{
    return(simRemovePointsFromPointCloud_internal(pointCloudHandle,options,pts,ptCnt,tolerance,reserved));
}
VREP_DLLEXPORT simInt simIntersectPointsWithPointCloud(simInt pointCloudHandle,simInt options,const simFloat* pts,simInt ptCnt,simFloat tolerance,simVoid* reserved)
{
    return(simIntersectPointsWithPointCloud_internal(pointCloudHandle,options,pts,ptCnt,tolerance,reserved));
}
VREP_DLLEXPORT const float* simGetOctreeVoxels(simInt octreeHandle,simInt* ptCnt,simVoid* reserved)
{
    return(simGetOctreeVoxels_internal(octreeHandle,ptCnt,reserved));
}
VREP_DLLEXPORT const float* simGetPointCloudPoints(simInt pointCloudHandle,simInt* ptCnt,simVoid* reserved)
{
    return(simGetPointCloudPoints_internal(pointCloudHandle,ptCnt,reserved));
}
VREP_DLLEXPORT simInt simInsertObjectIntoOctree(simInt octreeHandle,simInt objectHandle,simInt options,const simUChar* color,simUInt tag,simVoid* reserved)
{
    return(simInsertObjectIntoOctree_internal(octreeHandle,objectHandle,options,color,tag,reserved));
}
VREP_DLLEXPORT simInt simSubtractObjectFromOctree(simInt octreeHandle,simInt objectHandle,simInt options,simVoid* reserved)
{
    return(simSubtractObjectFromOctree_internal(octreeHandle,objectHandle,options,reserved));
}
VREP_DLLEXPORT simInt simInsertObjectIntoPointCloud(simInt pointCloudHandle,simInt objectHandle,simInt options,simFloat gridSize,const simUChar* color,simVoid* optionalValues)
{
    return(simInsertObjectIntoPointCloud_internal(pointCloudHandle,objectHandle,options,gridSize,color,optionalValues));
}
VREP_DLLEXPORT simInt simSubtractObjectFromPointCloud(simInt pointCloudHandle,simInt objectHandle,simInt options,simFloat tolerance,simVoid* reserved)
{
    return(simSubtractObjectFromPointCloud_internal(pointCloudHandle,objectHandle,options,tolerance,reserved));
}
VREP_DLLEXPORT simInt simCheckOctreePointOccupancy(simInt octreeHandle,simInt options,const simFloat* points,simInt ptCnt,simUInt* tag,simUInt64* location,simVoid* reserved)
{
    return(simCheckOctreePointOccupancy_internal(octreeHandle,options,points,ptCnt,tag,location,reserved));
}
VREP_DLLEXPORT simChar* simOpenTextEditor(const simChar* initText,const simChar* xml,simInt* various)
{
    return(simOpenTextEditor_internal(initText,xml,various));
}
VREP_DLLEXPORT simChar* simPackTable(simInt stackHandle,simInt* bufferSize)
{
    return(simPackTable_internal(stackHandle,bufferSize));
}
VREP_DLLEXPORT simInt simUnpackTable(simInt stackHandle,const simChar* buffer,simInt bufferSize)
{
    return(simUnpackTable_internal(stackHandle,buffer,bufferSize));
}
VREP_DLLEXPORT simInt simSetVisionSensorFilter(simInt visionSensorHandle,simInt filterIndex,simInt options,const simInt* pSizes,const simUChar* bytes,const simInt* ints,const simFloat* floats,const simUChar* custom)
{
    return(simSetVisionSensorFilter_internal(visionSensorHandle,filterIndex,options,pSizes,bytes,ints,floats,custom));
}
VREP_DLLEXPORT simInt simGetVisionSensorFilter(simInt visionSensorHandle,simInt filterIndex,simInt* options,simInt* pSizes,simUChar** bytes,simInt** ints,simFloat** floats,simUChar** custom)
{
    return(simGetVisionSensorFilter_internal(visionSensorHandle,filterIndex,options,pSizes,bytes,ints,floats,custom));
}
VREP_DLLEXPORT simInt simSetReferencedHandles(simInt objectHandle,simInt count,const simInt* referencedHandles,const simInt* reserved1,const simInt* reserved2)
{
    return(simSetReferencedHandles_internal(objectHandle,count,referencedHandles,reserved1,reserved2));
}
VREP_DLLEXPORT simInt simGetReferencedHandles(simInt objectHandle,simInt** referencedHandles,simInt** reserved1,simInt** reserved2)
{
    return(simGetReferencedHandles_internal(objectHandle,referencedHandles,reserved1,reserved2));
}
VREP_DLLEXPORT simInt simGetShapeViz(simInt shapeHandle,simInt index,struct SShapeVizInfo* info)
{
    return(simGetShapeViz_internal(shapeHandle,index,info));
}
VREP_DLLEXPORT simInt simExecuteScriptString(simInt scriptHandleOrType,const simChar* stringAtScriptName,simInt stackHandle)
{
    return(simExecuteScriptString_internal(scriptHandleOrType,stringAtScriptName,stackHandle));
}
VREP_DLLEXPORT simChar* simGetApiFunc(simInt scriptHandleOrType,const simChar* apiWord)
{
    return(simGetApiFunc_internal(scriptHandleOrType,apiWord));
}
VREP_DLLEXPORT simChar* simGetApiInfo(simInt scriptHandleOrType,const simChar* apiWord)
{
    return(simGetApiInfo_internal(scriptHandleOrType,apiWord));
}
VREP_DLLEXPORT simInt simSetModuleInfo(const simChar* moduleName,simInt infoType,const simChar* stringInfo,simInt intInfo)
{
    return(simSetModuleInfo_internal(moduleName,infoType,stringInfo,intInfo));
}
VREP_DLLEXPORT simInt simGetModuleInfo(const simChar* moduleName,simInt infoType,simChar** stringInfo,simInt* intInfo)
{
    return(simGetModuleInfo_internal(moduleName,infoType,stringInfo,intInfo));
}
VREP_DLLEXPORT simInt simIsDeprecated(const simChar* funcOrConst)
{
    return(simIsDeprecated_internal(funcOrConst));
}
VREP_DLLEXPORT simChar* simGetPersistentDataTags(simInt* tagCount)
{
    return(simGetPersistentDataTags_internal(tagCount));
}
VREP_DLLEXPORT simInt simEventNotification(const simChar* event)
{
    return(simEventNotification_internal(event));
}
VREP_DLLEXPORT simInt simApplyTexture(simInt shapeHandle,const simFloat* textureCoordinates,simInt textCoordSize,const simUChar* texture,const simInt* textureResolution,simInt options)
{
    return(simApplyTexture_internal(shapeHandle,textureCoordinates,textCoordSize,texture,textureResolution,options));
}
VREP_DLLEXPORT simInt simSetJointDependency(simInt jointHandle,simInt masterJointHandle,simFloat offset,simFloat coeff)
{
    return(simSetJointDependency_internal(jointHandle,masterJointHandle,offset,coeff));
}
VREP_DLLEXPORT simInt simSetStringNamedParam(const simChar* paramName,const simChar* stringParam,simInt paramLength)
{
    return(simSetStringNamedParam_internal(paramName,stringParam,paramLength));
}
VREP_DLLEXPORT simChar* simGetStringNamedParam(const simChar* paramName,simInt* paramLength)
{
    return(simGetStringNamedParam_internal(paramName,paramLength));
}


VREP_DLLEXPORT simInt _simGetContactCallbackCount()
{
    return(_simGetContactCallbackCount_internal());
}
VREP_DLLEXPORT const void* _simGetContactCallback(simInt index)
{
    return(_simGetContactCallback_internal(index));
}
VREP_DLLEXPORT simVoid _simSetDynamicSimulationIconCode(simVoid* object,simInt code)
{
    return(_simSetDynamicSimulationIconCode_internal(object,code));
}
VREP_DLLEXPORT simVoid _simSetDynamicObjectFlagForVisualization(simVoid* object,simInt flag)
{
    return(_simSetDynamicObjectFlagForVisualization_internal(object,flag));
}
VREP_DLLEXPORT simInt _simGetObjectListSize(simInt objType)
{
    return(_simGetObjectListSize_internal(objType));
}
VREP_DLLEXPORT const simVoid* _simGetObjectFromIndex(simInt objType,simInt index)
{
    return(_simGetObjectFromIndex_internal(objType,index));
}
VREP_DLLEXPORT simInt _simGetObjectID(const simVoid* object)
{
    return(_simGetObjectID_internal(object));
}
VREP_DLLEXPORT simInt _simGetObjectType(const simVoid* object)
{
    return(_simGetObjectType_internal(object));
}
VREP_DLLEXPORT const simVoid** _simGetObjectChildren(const simVoid* object,simInt* count)
{
    return(_simGetObjectChildren_internal(object,count));
}
VREP_DLLEXPORT const simVoid* _simGetGeomProxyFromShape(const simVoid* shape)
{
    return(_simGetGeomProxyFromShape_internal(shape));
}
VREP_DLLEXPORT const simVoid* _simGetParentObject(const simVoid* object)
{
    return(_simGetParentObject_internal(object));
}
VREP_DLLEXPORT const simVoid* _simGetObject(int objID)
{
    return(_simGetObject_internal(objID));
}
VREP_DLLEXPORT simVoid _simGetObjectLocalTransformation(const simVoid* object,simFloat* pos,simFloat* quat,simBool excludeFirstJointTransformation)
{
    return(_simGetObjectLocalTransformation_internal(object,pos,quat,excludeFirstJointTransformation));
}
VREP_DLLEXPORT simVoid _simSetObjectLocalTransformation(simVoid* object,const simFloat* pos,const simFloat* quat)
{
    return(_simSetObjectLocalTransformation_internal(object,pos,quat));
}
VREP_DLLEXPORT simVoid _simSetObjectCumulativeTransformation(simVoid* object,const simFloat* pos,const simFloat* quat,simBool keepChildrenInPlace)
{
    return(_simSetObjectCumulativeTransformation_internal(object,pos,quat,keepChildrenInPlace));
}
VREP_DLLEXPORT simVoid _simGetObjectCumulativeTransformation(const simVoid* object,simFloat* pos,simFloat* quat,simBool excludeFirstJointTransformation)
{
    return(_simGetObjectCumulativeTransformation_internal(object,pos,quat,excludeFirstJointTransformation));
}
VREP_DLLEXPORT simBool _simIsShapeDynamicallyStatic(const simVoid* shape)
{
    return(_simIsShapeDynamicallyStatic_internal(shape));
}
VREP_DLLEXPORT simInt _simGetTreeDynamicProperty(const simVoid* object)
{
    return(_simGetTreeDynamicProperty_internal(object));
}
VREP_DLLEXPORT simInt _simGetDummyLinkType(const simVoid* dummy,simInt* linkedDummyID)
{
    return(_simGetDummyLinkType_internal(dummy,linkedDummyID));
}
VREP_DLLEXPORT simInt _simGetJointMode(const simVoid* joint)
{
    return(_simGetJointMode_internal(joint));
}
VREP_DLLEXPORT simBool _simIsJointInHybridOperation(const simVoid* joint)
{
    return(_simIsJointInHybridOperation_internal(joint));
}
VREP_DLLEXPORT simVoid _simDisableDynamicTreeForManipulation(const simVoid* object,simBool disableFlag)
{
    return(_simDisableDynamicTreeForManipulation_internal(object,disableFlag));
}
VREP_DLLEXPORT simBool _simIsShapeDynamicallyRespondable(const simVoid* shape)
{
    return(_simIsShapeDynamicallyRespondable_internal(shape));
}
VREP_DLLEXPORT simInt _simGetDynamicCollisionMask(const simVoid* shape)
{
    return(_simGetDynamicCollisionMask_internal(shape));
}
VREP_DLLEXPORT const simVoid* _simGetLastParentForLocalGlobalCollidable(const simVoid* shape)
{
    return(_simGetLastParentForLocalGlobalCollidable_internal(shape));
}
VREP_DLLEXPORT simVoid _simSetShapeIsStaticAndNotRespondableButDynamicTag(const simVoid* shape,simBool tag)
{
    return(_simSetShapeIsStaticAndNotRespondableButDynamicTag_internal(shape,tag));
}
VREP_DLLEXPORT simBool _simGetShapeIsStaticAndNotRespondableButDynamicTag(const simVoid* shape)
{
    return(_simGetShapeIsStaticAndNotRespondableButDynamicTag_internal(shape));
}
VREP_DLLEXPORT simVoid _simSetJointPosition(const simVoid* joint,simFloat pos)
{
    return(_simSetJointPosition_internal(joint,pos));
}
VREP_DLLEXPORT simFloat _simGetJointPosition(const simVoid* joint)
{
    return(_simGetJointPosition_internal(joint));
}
VREP_DLLEXPORT simVoid _simSetDynamicMotorPositionControlTargetPosition(const simVoid* joint,simFloat pos)
{
    return(_simSetDynamicMotorPositionControlTargetPosition_internal(joint,pos));
}
VREP_DLLEXPORT simVoid _simGetInitialDynamicVelocity(const simVoid* shape,simFloat* vel)
{
    return(_simGetInitialDynamicVelocity_internal(shape,vel));
}
VREP_DLLEXPORT simVoid _simSetInitialDynamicVelocity(simVoid* shape,const simFloat* vel)
{
    return(_simSetInitialDynamicVelocity_internal(shape,vel));
}
VREP_DLLEXPORT simVoid _simGetInitialDynamicAngVelocity(const simVoid* shape,simFloat* angularVel)
{
    return(_simGetInitialDynamicAngVelocity_internal(shape,angularVel));
}
VREP_DLLEXPORT simVoid _simSetInitialDynamicAngVelocity(simVoid* shape,const simFloat* angularVel)
{
    return(_simSetInitialDynamicAngVelocity_internal(shape,angularVel));
}
VREP_DLLEXPORT simBool _simGetStartSleeping(const simVoid* shape)
{
    return(_simGetStartSleeping_internal(shape));
}
VREP_DLLEXPORT simBool _simGetWasPutToSleepOnce(const simVoid* shape)
{
    return(_simGetWasPutToSleepOnce_internal(shape));
}
VREP_DLLEXPORT simBool _simGetDynamicsFullRefreshFlag(const simVoid* object)
{
    return(_simGetDynamicsFullRefreshFlag_internal(object));
}
VREP_DLLEXPORT simVoid _simSetDynamicsFullRefreshFlag(const simVoid* object,simBool flag)
{
    return(_simSetDynamicsFullRefreshFlag_internal(object,flag));
}
VREP_DLLEXPORT simVoid _simSetGeomProxyDynamicsFullRefreshFlag(simVoid* geomData,simBool flag)
{
    return(_simSetGeomProxyDynamicsFullRefreshFlag_internal(geomData,flag));
}
VREP_DLLEXPORT simBool _simGetGeomProxyDynamicsFullRefreshFlag(const simVoid* geomData)
{
    return(_simGetGeomProxyDynamicsFullRefreshFlag_internal(geomData));
}
VREP_DLLEXPORT simBool _simGetParentFollowsDynamic(const simVoid* shape)
{
    return(_simGetParentFollowsDynamic_internal(shape));
}
VREP_DLLEXPORT simVoid _simSetShapeDynamicVelocity(simVoid* shape,const simFloat* linear,const simFloat* angular)
{
    return(_simSetShapeDynamicVelocity_internal(shape,linear,angular));
}
VREP_DLLEXPORT simVoid _simGetAdditionalForceAndTorque(const simVoid* shape,simFloat* force,simFloat* torque)
{
    return(_simGetAdditionalForceAndTorque_internal(shape,force,torque));
}
VREP_DLLEXPORT simVoid _simClearAdditionalForceAndTorque(const simVoid* shape)
{
    return(_simClearAdditionalForceAndTorque_internal(shape));
}
VREP_DLLEXPORT simBool _simGetJointPositionInterval(const simVoid* joint,simFloat* minValue,simFloat* rangeValue)
{
    return(_simGetJointPositionInterval_internal(joint,minValue,rangeValue));
}
VREP_DLLEXPORT simInt _simGetJointType(const simVoid* joint)
{
    return(_simGetJointType_internal(joint));
}
VREP_DLLEXPORT simBool _simIsForceSensorBroken(const simVoid* forceSensor)
{
    return(_simIsForceSensorBroken_internal(forceSensor));
}
VREP_DLLEXPORT simVoid _simGetDynamicForceSensorLocalTransformationPart2(const simVoid* forceSensor,simFloat* pos,simFloat* quat)
{
    return(_simGetDynamicForceSensorLocalTransformationPart2_internal(forceSensor,pos,quat));
}
VREP_DLLEXPORT simBool _simIsDynamicMotorEnabled(const simVoid* joint)
{
    return(_simIsDynamicMotorEnabled_internal(joint));
}
VREP_DLLEXPORT simBool _simIsDynamicMotorPositionCtrlEnabled(const simVoid* joint)
{
    return(_simIsDynamicMotorPositionCtrlEnabled_internal(joint));
}
VREP_DLLEXPORT simBool _simIsDynamicMotorTorqueModulationEnabled(const simVoid* joint)
{
    return(_simIsDynamicMotorTorqueModulationEnabled_internal(joint));
}
VREP_DLLEXPORT simVoid _simGetMotorPid(const simVoid* joint,simFloat* pParam,simFloat* iParam,simFloat* dParam)
{
    return(_simGetMotorPid_internal(joint,pParam,iParam,dParam));
}
VREP_DLLEXPORT simFloat _simGetDynamicMotorTargetPosition(const simVoid* joint)
{
    return(_simGetDynamicMotorTargetPosition_internal(joint));
}
VREP_DLLEXPORT simFloat _simGetDynamicMotorTargetVelocity(const simVoid* joint)
{
    return(_simGetDynamicMotorTargetVelocity_internal(joint));
}
VREP_DLLEXPORT simFloat _simGetDynamicMotorMaxForce(const simVoid* joint)
{
    return(_simGetDynamicMotorMaxForce_internal(joint));
}
VREP_DLLEXPORT simFloat _simGetDynamicMotorUpperLimitVelocity(const simVoid* joint)
{
    return(_simGetDynamicMotorUpperLimitVelocity_internal(joint));
}
VREP_DLLEXPORT simVoid _simSetDynamicMotorReflectedPositionFromDynamicEngine(simVoid* joint,simFloat pos)
{
    return(_simSetDynamicMotorReflectedPositionFromDynamicEngine_internal(joint,pos));
}
VREP_DLLEXPORT simVoid _simSetJointSphericalTransformation(simVoid* joint,const simFloat* quat)
{
    return(_simSetJointSphericalTransformation_internal(joint,quat));
}
VREP_DLLEXPORT simVoid _simAddForceSensorCumulativeForcesAndTorques(simVoid* forceSensor,const simFloat* force,const simFloat* torque,int totalPassesCount)
{
    return(_simAddForceSensorCumulativeForcesAndTorques_internal(forceSensor,force,torque,totalPassesCount));
}
VREP_DLLEXPORT simVoid _simAddJointCumulativeForcesOrTorques(simVoid* joint,simFloat forceOrTorque,int totalPassesCount)
{
    return(_simAddJointCumulativeForcesOrTorques_internal(joint,forceOrTorque,totalPassesCount));
}
VREP_DLLEXPORT simVoid _simSetDynamicJointLocalTransformationPart2(simVoid* joint,const simFloat* pos,const simFloat* quat)
{
    return(_simSetDynamicJointLocalTransformationPart2_internal(joint,pos,quat));
}
VREP_DLLEXPORT simVoid _simSetDynamicForceSensorLocalTransformationPart2(simVoid* forceSensor,const simFloat* pos,const simFloat* quat)
{
    return(_simSetDynamicForceSensorLocalTransformationPart2_internal(forceSensor,pos,quat));
}
VREP_DLLEXPORT simVoid _simSetDynamicJointLocalTransformationPart2IsValid(simVoid* joint,simBool valid)
{
    return(_simSetDynamicJointLocalTransformationPart2IsValid_internal(joint,valid));
}
VREP_DLLEXPORT simVoid _simSetDynamicForceSensorLocalTransformationPart2IsValid(simVoid* forceSensor,simBool valid)
{
    return(_simSetDynamicForceSensorLocalTransformationPart2IsValid_internal(forceSensor,valid));
}
VREP_DLLEXPORT const simVoid* _simGetGeomWrapFromGeomProxy(const simVoid* geomData)
{
    return(_simGetGeomWrapFromGeomProxy_internal(geomData));
}
VREP_DLLEXPORT simVoid _simGetLocalInertiaFrame(const simVoid* geomInfo,simFloat* pos,simFloat* quat)
{
    return(_simGetLocalInertiaFrame_internal(geomInfo,pos,quat));
}
VREP_DLLEXPORT simInt _simGetPurePrimitiveType(const simVoid* geomInfo)
{
    return(_simGetPurePrimitiveType_internal(geomInfo));
}
VREP_DLLEXPORT simBool _simIsGeomWrapGeometric(const simVoid* geomInfo)
{
    return(_simIsGeomWrapGeometric_internal(geomInfo));
}
VREP_DLLEXPORT simBool _simIsGeomWrapConvex(const simVoid* geomInfo)
{
    return(_simIsGeomWrapConvex_internal(geomInfo));
}
VREP_DLLEXPORT simInt _simGetGeometricCount(const simVoid* geomInfo)
{
    return(_simGetGeometricCount_internal(geomInfo));
}
VREP_DLLEXPORT simVoid _simGetAllGeometrics(const simVoid* geomInfo,simVoid** allGeometrics)
{
    return(_simGetAllGeometrics_internal(geomInfo,allGeometrics));
}
VREP_DLLEXPORT simVoid _simGetPurePrimitiveSizes(const simVoid* geometric,simFloat* sizes)
{
    return(_simGetPurePrimitiveSizes_internal(geometric,sizes));
}
VREP_DLLEXPORT simVoid _simMakeDynamicAnnouncement(int announceType)
{
    return(_simMakeDynamicAnnouncement_internal(announceType));
}
VREP_DLLEXPORT simVoid _simGetVerticesLocalFrame(const simVoid* geometric,simFloat* pos,simFloat* quat)
{
    return(_simGetVerticesLocalFrame_internal(geometric,pos,quat));
}
VREP_DLLEXPORT const simFloat* _simGetHeightfieldData(const simVoid* geometric,simInt* xCount,simInt* yCount,simFloat* minHeight,simFloat* maxHeight)
{
    return(_simGetHeightfieldData_internal(geometric,xCount,yCount,minHeight,maxHeight));
}
VREP_DLLEXPORT simVoid _simGetCumulativeMeshes(const simVoid* geomInfo,simFloat** vertices,simInt* verticesSize,simInt** indices,simInt* indicesSize)
{
    return(_simGetCumulativeMeshes_internal(geomInfo,vertices,verticesSize,indices,indicesSize));
}
VREP_DLLEXPORT simFloat _simGetMass(const simVoid* geomInfo)
{
    return(_simGetMass_internal(geomInfo));
}
VREP_DLLEXPORT simVoid _simGetPrincipalMomentOfInertia(const simVoid* geomInfo,simFloat* inertia)
{
    return(_simGetPrincipalMomentOfInertia_internal(geomInfo,inertia));
}
VREP_DLLEXPORT simVoid _simGetGravity(simFloat* gravity)
{
    return(_simGetGravity_internal(gravity));
}
VREP_DLLEXPORT simInt _simGetTimeDiffInMs(simInt previousTime)
{
    return(_simGetTimeDiffInMs_internal(previousTime));
}
VREP_DLLEXPORT simBool _simDoEntitiesCollide(simInt entity1ID,simInt entity2ID,simInt* cacheBuffer,simBool overrideCollidableFlagIfShape1,simBool overrideCollidableFlagIfShape2,simBool pathOrMotionPlanningRoutineCalling)
{
    return(_simDoEntitiesCollide_internal(entity1ID,entity2ID,cacheBuffer,overrideCollidableFlagIfShape1,overrideCollidableFlagIfShape2,pathOrMotionPlanningRoutineCalling));
}
VREP_DLLEXPORT simBool _simGetDistanceBetweenEntitiesIfSmaller(simInt entity1ID,simInt entity2ID,simFloat* distance,simFloat* ray,simInt* cacheBuffer,simBool overrideMeasurableFlagIfNonCollection1,simBool overrideMeasurableFlagIfNonCollection2,simBool pathPlanningRoutineCalling)
{
    return(_simGetDistanceBetweenEntitiesIfSmaller_internal(entity1ID,entity2ID,distance,ray,cacheBuffer,overrideMeasurableFlagIfNonCollection1,overrideMeasurableFlagIfNonCollection2,pathPlanningRoutineCalling));
}
VREP_DLLEXPORT simInt _simHandleJointControl(const simVoid* joint,simInt auxV,const simInt* inputValuesInt,const simFloat* inputValuesFloat,simFloat* outputValues)
{
    return(_simHandleJointControl_internal(joint,auxV,inputValuesInt,inputValuesFloat,outputValues));
}
VREP_DLLEXPORT simInt _simHandleCustomContact(simInt objHandle1,simInt objHandle2,simInt engine,simInt* dataInt,simFloat* dataFloat)
{
    return(_simHandleCustomContact_internal(objHandle1,objHandle2,engine,dataInt,dataFloat));
}
VREP_DLLEXPORT const simVoid* _simGetIkGroupObject(int ikGroupID)
{
    return(_simGetIkGroupObject_internal(ikGroupID));
}
VREP_DLLEXPORT simInt _simMpHandleIkGroupObject(const simVoid* ikGroup)
{
    return(_simMpHandleIkGroupObject_internal(ikGroup));
}
VREP_DLLEXPORT simFloat _simGetPureHollowScaling(const simVoid* geometric)
{
    return(_simGetPureHollowScaling_internal(geometric));
}
VREP_DLLEXPORT simVoid _simDynCallback(const simInt* intData,const simFloat* floatData)
{
    _simDynCallback_internal(intData,floatData);
}

// Following courtesy of Stephen James:
// Functions to allow an external application to have control of vrep's thread loop
VREP_DLLEXPORT simInt simExtLaunchUIThread(const simChar* applicationName,simInt options,const simChar* sceneOrModelOrUiToLoad, const simChar* applicationDir_)
{
    return(simExtLaunchUIThread_internal(applicationName,options,sceneOrModelOrUiToLoad,applicationDir_));
}
VREP_DLLEXPORT simInt simExtPostExitRequest()
{
    return(simExtPostExitRequest_internal());
}
VREP_DLLEXPORT simInt simExtGetExitRequest()
{
    return(simExtGetExitRequest_internal());
}
VREP_DLLEXPORT simInt simExtStep(simBool stepIfRunning)
{
    return(simExtStep_internal(stepIfRunning));
}
VREP_DLLEXPORT simInt simExtCanInitSimThread()
{
    return(simExtCanInitSimThread_internal());
}
VREP_DLLEXPORT simInt simExtSimThreadInit()
{
    return(simExtSimThreadInit_internal());
}
VREP_DLLEXPORT simInt simExtSimThreadDestroy()
{
    return(simExtSimThreadDestroy_internal());
}
VREP_DLLEXPORT simInt simExtCallScriptFunction(simInt scriptHandleOrType, const simChar* functionNameAtScriptName,
                                               const simInt* inIntData, simInt inIntCnt,
                                               const simFloat* inFloatData, simInt inFloatCnt,
                                               const simChar** inStringData, simInt inStringCnt,
                                               const simChar* inBufferData, simInt inBufferCnt,
                                               simInt** outIntData, simInt* outIntCnt,
                                               simFloat** outFloatData, simInt* outFloatCnt,
                                               simChar*** outStringData, simInt* outStringCnt,
                                               simChar** outBufferData, simInt* outBufferSize)
{
    return(simExtCallScriptFunction_internal(scriptHandleOrType, functionNameAtScriptName,
                                             inIntData, inIntCnt, inFloatData, inFloatCnt,
                                             inStringData, inStringCnt, inBufferData, inBufferCnt,
                                             outIntData, outIntCnt, outFloatData, outFloatCnt,
                                             outStringData, outStringCnt, outBufferData, outBufferSize));
}


// Deprecated begin
VREP_DLLEXPORT simInt simGetMaterialId(const simChar* materialName)
{
    return(simGetMaterialId_internal(materialName));
}
VREP_DLLEXPORT simInt simGetShapeMaterial(simInt shapeHandle)
{
    return(simGetShapeMaterial_internal(shapeHandle));
}
VREP_DLLEXPORT simInt simHandleVarious()
{
    return(simHandleVarious_internal());
}
VREP_DLLEXPORT simInt simSerialPortOpen(simInt portNumber,simInt baudRate,simVoid* reserved1,simVoid* reserved2)
{
    return(simSerialPortOpen_internal(portNumber,baudRate,reserved1,reserved2));
}
VREP_DLLEXPORT simInt simSerialPortClose(simInt portNumber)
{
    return(simSerialPortClose_internal(portNumber));
}
VREP_DLLEXPORT simInt simSerialPortSend(simInt portNumber,const simChar* data,simInt dataLength)
{
    return(simSerialPortSend_internal(portNumber,data,dataLength));
}
VREP_DLLEXPORT simInt simSerialPortRead(simInt portNumber,simChar* buffer,simInt dataLengthToRead)
{
    return(simSerialPortRead_internal(portNumber,buffer,dataLengthToRead));
}
VREP_DLLEXPORT simInt simJointGetForce(simInt jointHandle,simFloat* forceOrTorque)
{
    return(simGetJointForce_internal(jointHandle,forceOrTorque));
}
VREP_DLLEXPORT simInt simGetPathPlanningHandle(const simChar* pathPlanningObjectName)
{
    return(simGetPathPlanningHandle_internal(pathPlanningObjectName));
}
VREP_DLLEXPORT simInt simGetMotionPlanningHandle(const simChar* motionPlanningObjectName)
{
    return(simGetMotionPlanningHandle_internal(motionPlanningObjectName));
}
VREP_DLLEXPORT simInt simGetMpConfigForTipPose(simInt motionPlanningObjectHandle,simInt options,simFloat closeNodesDistance,simInt trialCount,const simFloat* tipPose,simInt maxTimeInMs,simFloat* outputJointPositions,const simFloat* referenceConfigs,simInt referenceConfigCount,const simFloat* jointWeights,const simInt* jointBehaviour,simInt correctionPasses)
{
    return(simGetMpConfigForTipPose_internal(motionPlanningObjectHandle,options,closeNodesDistance,trialCount,tipPose,maxTimeInMs,outputJointPositions,referenceConfigs,referenceConfigCount,jointWeights,jointBehaviour,correctionPasses));
}
VREP_DLLEXPORT simFloat* simFindMpPath(simInt motionPlanningObjectHandle,const simFloat* startConfig,const simFloat* goalConfig,simInt options,simFloat stepSize,simInt* outputConfigsCnt,simInt maxTimeInMs,simFloat* reserved,const simInt* auxIntParams,const simFloat* auxFloatParams)
{
    return(simFindMpPath_internal(motionPlanningObjectHandle,startConfig,goalConfig,options,stepSize,outputConfigsCnt,maxTimeInMs,reserved,auxIntParams,auxFloatParams));
}
VREP_DLLEXPORT simFloat* simSimplifyMpPath(simInt motionPlanningObjectHandle,const simFloat* pathBuffer,simInt configCnt,simInt options,simFloat stepSize,simInt increment,simInt* outputConfigsCnt,simInt maxTimeInMs,simFloat* reserved,const simInt* auxIntParams,const simFloat* auxFloatParams)
{
    return(simSimplifyMpPath_internal(motionPlanningObjectHandle,pathBuffer,configCnt,options,stepSize,increment,outputConfigsCnt,maxTimeInMs,reserved,auxIntParams,auxFloatParams));
}
VREP_DLLEXPORT simFloat* simFindIkPath(simInt motionPlanningObjectHandle,const simFloat* startConfig,const simFloat* goalPose,simInt options,simFloat stepSize,simInt* outputConfigsCnt,simFloat* reserved,const simInt* auxIntParams,const simFloat* auxFloatParams)
{
    return(simFindIkPath_internal(motionPlanningObjectHandle,startConfig,goalPose,options,stepSize,outputConfigsCnt,reserved,auxIntParams,auxFloatParams));
}
VREP_DLLEXPORT simFloat* simGetMpConfigTransition(simInt motionPlanningObjectHandle,const simFloat* startConfig,const simFloat* goalConfig,simInt options,const simInt* select,simFloat calcStepSize,simFloat maxOutStepSize,simInt wayPointCnt,const simFloat* wayPoints,simInt* outputConfigsCnt,const simInt* auxIntParams,const simFloat* auxFloatParams)
{
    return(simGetMpConfigTransition_internal(motionPlanningObjectHandle,startConfig,goalConfig,options,select,calcStepSize,maxOutStepSize,wayPointCnt,wayPoints,outputConfigsCnt,auxIntParams,auxFloatParams));
}
VREP_DLLEXPORT simInt simCreateMotionPlanning(simInt jointCnt,const simInt* jointHandles,const simInt* jointRangeSubdivisions,const simFloat* jointMetricWeights,simInt options,const simInt* intParams,const simFloat* floatParams,const simVoid* reserved)
{
    return(simCreateMotionPlanning_internal(jointCnt,jointHandles,jointRangeSubdivisions,jointMetricWeights,options,intParams,floatParams,reserved));
}
VREP_DLLEXPORT simInt simRemoveMotionPlanning(simInt motionPlanningHandle)
{
    return(simRemoveMotionPlanning_internal(motionPlanningHandle));
}
VREP_DLLEXPORT simInt simSearchPath(simInt pathPlanningObjectHandle,simFloat maximumSearchTime)
{
    return(simSearchPath_internal(pathPlanningObjectHandle,maximumSearchTime));
}
VREP_DLLEXPORT simInt simInitializePathSearch(simInt pathPlanningObjectHandle,simFloat maximumSearchTime,simFloat searchTimeStep)
{
    return(simInitializePathSearch_internal(pathPlanningObjectHandle,maximumSearchTime,searchTimeStep));
}
VREP_DLLEXPORT simInt simPerformPathSearchStep(simInt temporaryPathSearchObject,simBool abortSearch)
{
    return(simPerformPathSearchStep_internal(temporaryPathSearchObject,abortSearch));
}
VREP_DLLEXPORT simInt simLockInterface(simBool locked)
{
    return(simLockInterface_internal(locked));
}
VREP_DLLEXPORT simInt simCopyPasteSelectedObjects()
{
    return(simCopyPasteSelectedObjects_internal());
}
VREP_DLLEXPORT simInt simResetPath(simInt pathHandle)
{
    return(simResetPath_internal(pathHandle));
}
VREP_DLLEXPORT simInt simHandlePath(simInt pathHandle,simFloat deltaTime)
{
    return(simHandlePath_internal(pathHandle,deltaTime));
}
VREP_DLLEXPORT simInt simResetJoint(simInt jointHandle)
{
    return(simResetJoint_internal(jointHandle));
}
VREP_DLLEXPORT simInt simHandleJoint(simInt jointHandle,simFloat deltaTime)
{
    return(simHandleJoint_internal(jointHandle,deltaTime));
}
VREP_DLLEXPORT simInt simAppendScriptArrayEntry(const simChar* reservedSetToNull,simInt scriptHandleOrType,const simChar* arrayNameAtScriptName,const simChar* keyName,const simChar* data,const simInt* what)
{
    return(simAppendScriptArrayEntry_internal(reservedSetToNull,scriptHandleOrType,arrayNameAtScriptName,keyName,data,what));
}
VREP_DLLEXPORT simInt simClearScriptVariable(const simChar* reservedSetToNull,simInt scriptHandleOrType,const simChar* variableNameAtScriptName)
{
    return(simClearScriptVariable_internal(reservedSetToNull,scriptHandleOrType,variableNameAtScriptName));
}
VREP_DLLEXPORT simVoid _simGetJointOdeParameters(const simVoid* joint,simFloat* stopERP,simFloat* stopCFM,simFloat* bounce,simFloat* fudge,simFloat* normalCFM)
{
    return(_simGetJointOdeParameters_internal(joint,stopERP,stopCFM,bounce,fudge,normalCFM));
}
VREP_DLLEXPORT simVoid _simGetJointBulletParameters(const simVoid* joint,simFloat* stopERP,simFloat* stopCFM,simFloat* normalCFM)
{
    return(_simGetJointBulletParameters_internal(joint,stopERP,stopCFM,normalCFM));
}
VREP_DLLEXPORT simVoid _simGetOdeMaxContactFrictionCFMandERP(const simVoid* geomInfo,simInt* maxContacts,simFloat* friction,simFloat* cfm,simFloat* erp)
{
    return(_simGetOdeMaxContactFrictionCFMandERP_internal(geomInfo,maxContacts,friction,cfm,erp));
}
VREP_DLLEXPORT simBool _simGetBulletCollisionMargin(const simVoid* geomInfo,simFloat* margin,simInt* otherProp)
{
    return(_simGetBulletCollisionMargin_internal(geomInfo,margin,otherProp));
}
VREP_DLLEXPORT simBool _simGetBulletStickyContact(const simVoid* geomInfo)
{
    return(_simGetBulletStickyContact_internal(geomInfo));
}
VREP_DLLEXPORT simFloat _simGetBulletRestitution(const simVoid* geomInfo)
{
    return(_simGetBulletRestitution_internal(geomInfo));
}
VREP_DLLEXPORT simVoid _simGetVortexParameters(const simVoid* object,simInt version,simFloat* floatParams,simInt* intParams)
{
    _simGetVortexParameters_internal(object,version,floatParams,intParams);
}
VREP_DLLEXPORT simVoid _simGetNewtonParameters(const simVoid* object,simInt* version,simFloat* floatParams,simInt* intParams)
{
    _simGetNewtonParameters_internal(object,version,floatParams,intParams);
}
VREP_DLLEXPORT simVoid _simGetDamping(const simVoid* geomInfo,simFloat* linDamping,simFloat* angDamping)
{
    return(_simGetDamping_internal(geomInfo,linDamping,angDamping));
}
VREP_DLLEXPORT simFloat _simGetFriction(const simVoid* geomInfo)
{
    return(_simGetFriction_internal(geomInfo));
}
VREP_DLLEXPORT simInt simAddSceneCustomData(simInt header,const simChar* data,simInt dataLength)
{
    return(simAddSceneCustomData_internal(header,data,dataLength));
}
VREP_DLLEXPORT simInt simGetSceneCustomDataLength(simInt header)
{
    return(simGetSceneCustomDataLength_internal(header));
}
VREP_DLLEXPORT simInt simGetSceneCustomData(simInt header,simChar* data)
{
    return(simGetSceneCustomData_internal(header,data));
}
VREP_DLLEXPORT simInt simAddObjectCustomData(simInt objectHandle,simInt header,const simChar* data,simInt dataLength)
{
    return(simAddObjectCustomData_internal(objectHandle,header,data,dataLength));
}
VREP_DLLEXPORT simInt simGetObjectCustomDataLength(simInt objectHandle,simInt header)
{
    return(simGetObjectCustomDataLength_internal(objectHandle,header));
}
VREP_DLLEXPORT simInt simGetObjectCustomData(simInt objectHandle,simInt header,simChar* data)
{
    return(simGetObjectCustomData_internal(objectHandle,header,data));
}
VREP_DLLEXPORT simInt simCreateUI(const simChar* uiName,simInt menuAttributes,const simInt* clientSize,const simInt* cellSize,simInt* buttonHandles)
{
    return(simCreateUI_internal(uiName,menuAttributes,clientSize,cellSize,buttonHandles));
}
VREP_DLLEXPORT simInt simCreateUIButton(simInt uiHandle,const simInt* position,const simInt* size,simInt buttonProperty)
{
    return(simCreateUIButton_internal(uiHandle,position,size,buttonProperty));
}
VREP_DLLEXPORT simInt simGetUIHandle(const simChar* uiName)
{
    return(simGetUIHandle_internal(uiName));
}
VREP_DLLEXPORT simInt simGetUIProperty(simInt uiHandle)
{
    return(simGetUIProperty_internal(uiHandle));
}
VREP_DLLEXPORT simInt simGetUIEventButton(simInt uiHandle,simInt* auxiliaryValues)
{
    return(simGetUIEventButton_internal(uiHandle,auxiliaryValues));
}
VREP_DLLEXPORT simInt simSetUIProperty(simInt uiHandle,simInt elementProperty)
{
    return(simSetUIProperty_internal(uiHandle,elementProperty));
}
VREP_DLLEXPORT simInt simGetUIButtonProperty(simInt uiHandle,simInt buttonHandle)
{
    return(simGetUIButtonProperty_internal(uiHandle,buttonHandle));
}
VREP_DLLEXPORT simInt simSetUIButtonProperty(simInt uiHandle,simInt buttonHandle,simInt buttonProperty)
{
    return(simSetUIButtonProperty_internal(uiHandle,buttonHandle,buttonProperty));
}
VREP_DLLEXPORT simInt simGetUIButtonSize(simInt uiHandle,simInt buttonHandle,simInt* size)
{
    return(simGetUIButtonSize_internal(uiHandle,buttonHandle,size));
}
VREP_DLLEXPORT simInt simSetUIButtonLabel(simInt uiHandle,simInt buttonHandle,const simChar* upStateLabel,const simChar* downStateLabel)
{
    return(simSetUIButtonLabel_internal(uiHandle,buttonHandle,upStateLabel,downStateLabel));
}
VREP_DLLEXPORT simChar* simGetUIButtonLabel(simInt uiHandle,simInt buttonHandle)
{
    return(simGetUIButtonLabel_internal(uiHandle,buttonHandle));
}
VREP_DLLEXPORT simInt simSetUISlider(simInt uiHandle,simInt buttonHandle,simInt position)
{
    return(simSetUISlider_internal(uiHandle,buttonHandle,position));
}
VREP_DLLEXPORT simInt simGetUISlider(simInt uiHandle,simInt buttonHandle)
{
    return(simGetUISlider_internal(uiHandle,buttonHandle));
}
VREP_DLLEXPORT simInt simSetUIButtonColor(simInt uiHandle,simInt buttonHandle,const simFloat* upStateColor,const simFloat* downStateColor,const simFloat* labelColor)
{
    return(simSetUIButtonColor_internal(uiHandle,buttonHandle,upStateColor,downStateColor,labelColor));
}
VREP_DLLEXPORT simInt simSetUIButtonTexture(simInt uiHandle,simInt buttonHandle,const simInt* size,const simChar* textureData)
{
    return(simSetUIButtonTexture_internal(uiHandle,buttonHandle,size,textureData));
}
VREP_DLLEXPORT simInt simCreateUIButtonArray(simInt uiHandle,simInt buttonHandle)
{
    return(simCreateUIButtonArray_internal(uiHandle,buttonHandle));
}
VREP_DLLEXPORT simInt simSetUIButtonArrayColor(simInt uiHandle,simInt buttonHandle,const simInt* position,const simFloat* color)
{
    return(simSetUIButtonArrayColor_internal(uiHandle,buttonHandle,position,color));
}
VREP_DLLEXPORT simInt simDeleteUIButtonArray(simInt uiHandle,simInt buttonHandle)
{
    return(simDeleteUIButtonArray_internal(uiHandle,buttonHandle));
}
VREP_DLLEXPORT simInt simRemoveUI(simInt uiHandle)
{
    return(simRemoveUI_internal(uiHandle));
}
VREP_DLLEXPORT simInt simSetUIPosition(simInt uiHandle,const simInt* position)
{
    return(simSetUIPosition_internal(uiHandle,position));
}
VREP_DLLEXPORT simInt simGetUIPosition(simInt uiHandle,simInt* position)
{
    return(simGetUIPosition_internal(uiHandle,position));
}
VREP_DLLEXPORT simInt simLoadUI(const simChar* filename,int maxCount,int* uiHandles)
{
    return(simLoadUI_internal(filename,maxCount,uiHandles));
}
VREP_DLLEXPORT simInt simSaveUI(int count,const int* uiHandles,const simChar* filename)
{
    return(simSaveUI_internal(count,uiHandles,filename));
}
VREP_DLLEXPORT simInt simHandleGeneralCallbackScript(simInt callbackId,simInt callbackTag,simVoid* additionalData)
{
    return(simHandleGeneralCallbackScript_internal(callbackId,callbackTag,additionalData));
}
VREP_DLLEXPORT simInt simRegisterCustomLuaFunction(const simChar* funcName,const simChar* callTips,const simInt* inputArgumentTypes,simVoid(*callBack)(struct SLuaCallBack* p))
{
    return(simRegisterCustomLuaFunction_internal(funcName,callTips,inputArgumentTypes,callBack));
}
VREP_DLLEXPORT simInt simRegisterCustomLuaVariable(const simChar* varName,const simChar* varValue)
{
    return(simRegisterScriptVariable_internal(varName,varValue,0));
}
VREP_DLLEXPORT simInt simRegisterContactCallback(simInt(*callBack)(simInt,simInt,simInt,simInt*,simFloat*))
{
    return(simRegisterContactCallback_internal(callBack));
}
VREP_DLLEXPORT simInt simRegisterJointCtrlCallback(simInt(*callBack)(simInt,simInt,simInt,const simInt*,const simFloat*,simFloat*))
{
    return(simRegisterJointCtrlCallback_internal(callBack));
}
VREP_DLLEXPORT simInt simGetMechanismHandle(const simChar* mechanismName)
{
    return(simGetMechanismHandle_internal(mechanismName));
}
VREP_DLLEXPORT simInt simHandleMechanism(simInt mechanismHandle)
{
    return(simHandleMechanism_internal(mechanismHandle));
}
VREP_DLLEXPORT simInt simHandleCustomizationScripts(simInt callType)
{
    return(simHandleCustomizationScripts_internal(callType));
}
VREP_DLLEXPORT simInt simCallScriptFunction(simInt scriptHandleOrType,const simChar* functionNameAtScriptName,SLuaCallBack* data,const simChar* reservedSetToNull)
{
    return(simCallScriptFunction_internal(scriptHandleOrType,functionNameAtScriptName,data,reservedSetToNull));
}
VREP_DLLEXPORT simInt _simGetJointCallbackCallOrder(const simVoid* )
{
    return(0);
}
// Deprecated end

