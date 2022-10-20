#include "sim.h"
#include "simInternal.h"
#include "app.h"

SIM_DLLEXPORT simInt simRunSimulator(const simChar* applicationName,simInt options,simVoid(*initCallBack)(),simVoid(*loopCallBack)(),simVoid(*deinitCallBack)())
{
    return(simRunSimulator_internal(applicationName,options,initCallBack,loopCallBack,deinitCallBack,0,"",true));
}
SIM_DLLEXPORT simInt simRunSimulatorEx(const simChar* applicationName,simInt options,simVoid(*initCallBack)(),simVoid(*loopCallBack)(),simVoid(*deinitCallBack)(),simInt stopDelay,const simChar* sceneOrModelToLoad)
{
    return(simRunSimulator_internal(applicationName,options,initCallBack,loopCallBack,deinitCallBack,stopDelay,sceneOrModelToLoad,true));
}
SIM_DLLEXPORT simVoid* simGetMainWindow(simInt type)
{
    return(simGetMainWindow_internal(type));
}
SIM_DLLEXPORT simChar* simGetLastError()
{
    return(simGetLastError_internal());
}
SIM_DLLEXPORT simInt simSetBoolParam(simInt parameter,simBool boolState)
{
    return(simSetBoolParam_internal(parameter,boolState));
}
SIM_DLLEXPORT simInt simGetBoolParam(simInt parameter)
{
    return(simGetBoolParam_internal(parameter));
}
SIM_DLLEXPORT simInt simSetInt32Param(simInt parameter,simInt intState)
{
    return(simSetInt32Param_internal(parameter,intState));
}
SIM_DLLEXPORT simInt simGetInt32Param(simInt parameter,simInt* intState)
{
    return(simGetInt32Param_internal(parameter,intState));
}
SIM_DLLEXPORT simInt simGetUInt64Param(simInt parameter,simUInt64* intState)
{
    return(simGetUInt64Param_internal(parameter,intState));
}
SIM_DLLEXPORT simInt simSetStringParam(simInt parameter,const simChar* str)
{
    return(simSetStringParam_internal(parameter,str));
}
SIM_DLLEXPORT simChar* simGetStringParam(simInt parameter)
{
    return(simGetStringParam_internal(parameter));
}
SIM_DLLEXPORT simInt simSetArrayParam(simInt parameter,const simVoid* arrayOfValues)
{
    return(simSetArrayParam_internal(parameter,arrayOfValues));
}
SIM_DLLEXPORT simInt simGetArrayParam(simInt parameter,simVoid* arrayOfValues)
{
    return(simGetArrayParam_internal(parameter,arrayOfValues));
}
SIM_DLLEXPORT simInt simSetNamedStringParam(const simChar* paramName,const simChar* stringParam,simInt paramLength)
{
    return(simSetNamedStringParam_internal(paramName,stringParam,paramLength));
}
SIM_DLLEXPORT simChar* simGetNamedStringParam(const simChar* paramName,simInt* paramLength)
{
    return(simGetNamedStringParam_internal(paramName,paramLength));
}
SIM_DLLEXPORT simInt simGetObject(const simChar* objectAlias,simInt index,simInt proxy,simInt options)
{
    return(simGetObject_internal(objectAlias,index,proxy,options));
}
SIM_DLLEXPORT simInt64 simGetObjectUid(simInt objectHandle)
{
    return(simGetObjectUid_internal(objectHandle));
}
SIM_DLLEXPORT simInt simGetObjectFromUid(simInt64 uid,simInt options)
{
    return(simGetObjectFromUid_internal(uid,options));
}
SIM_DLLEXPORT simInt simGetScriptHandleEx(simInt scriptType,simInt objectHandle,const simChar* scriptName)
{
    return(simGetScriptHandleEx_internal(scriptType,objectHandle,scriptName));
}
SIM_DLLEXPORT simInt simRemoveObjects(const simInt* objectHandles,simInt count)
{
    return(simRemoveObjects_internal(objectHandles,count));
}
SIM_DLLEXPORT simInt simRemoveModel(simInt objectHandle)
{
    return(simRemoveModel_internal(objectHandle));
}
SIM_DLLEXPORT simChar* simGetObjectAlias(simInt objectHandle,simInt options)
{
    return(simGetObjectAlias_internal(objectHandle,options));
}
SIM_DLLEXPORT simInt simSetObjectAlias(simInt objectHandle,const simChar* objectAlias,simInt options)
{
    return(simSetObjectAlias_internal(objectHandle,objectAlias,options));
}
SIM_DLLEXPORT simInt simGetObjectParent(simInt objectHandle)
{
    return(simGetObjectParent_internal(objectHandle));
}
SIM_DLLEXPORT simInt simGetObjectChild(simInt objectHandle,simInt index)
{
    return(simGetObjectChild_internal(objectHandle,index));
}
SIM_DLLEXPORT simInt simSetObjectParent(simInt objectHandle,simInt parentObjectHandle,simBool keepInPlace)
{
    return(simSetObjectParent_internal(objectHandle,parentObjectHandle,keepInPlace));
}
SIM_DLLEXPORT simInt simGetObjectType(simInt objectHandle)
{
    return(simGetObjectType_internal(objectHandle));
}
SIM_DLLEXPORT simInt simGetJointType(simInt objectHandle)
{
    return(simGetJointType_internal(objectHandle));
}
SIM_DLLEXPORT simInt simReservedCommand(simInt v,simInt w)
{
    return(simReservedCommand_internal(v,w));
}
SIM_DLLEXPORT simInt simGetSimulationState()
{
    return(simGetSimulationState_internal());
}
SIM_DLLEXPORT simInt simLoadScene(const simChar* filename)
{
    return(simLoadScene_internal(filename));
}
SIM_DLLEXPORT simInt simCloseScene()
{
    return(simCloseScene_internal());
}
SIM_DLLEXPORT simInt simSaveScene(const simChar* filename)
{
    return(simSaveScene_internal(filename));
}
SIM_DLLEXPORT simInt simLoadModel(const simChar* filename)
{
    return(simLoadModel_internal(filename));
}
SIM_DLLEXPORT simInt simSaveModel(int baseOfModelHandle,const simChar* filename)
{
    return(simSaveModel_internal(baseOfModelHandle,filename));
}
SIM_DLLEXPORT simChar* simGetSimulatorMessage(simInt* messageID,simInt* auxiliaryData,simInt* returnedDataSize)
{
    return(simGetSimulatorMessage_internal(messageID,auxiliaryData,returnedDataSize));
}
SIM_DLLEXPORT simInt simDoesFileExist(const simChar* filename)
{
    return(simDoesFileExist_internal(filename));
}
SIM_DLLEXPORT simInt* simGetObjectSel(simInt* cnt)
{
    return(simGetObjectSel_internal(cnt));
}
SIM_DLLEXPORT simInt simSetObjectSel(const simInt* handles,simInt cnt)
{
    return(simSetObjectSel_internal(handles,cnt));
}
SIM_DLLEXPORT simInt simAssociateScriptWithObject(simInt scriptHandle,simInt associatedObjectHandle)
{
    return(simAssociateScriptWithObject_internal(scriptHandle,associatedObjectHandle));
}
SIM_DLLEXPORT simInt simHandleMainScript()
{
    return(simHandleMainScript_internal());
}
SIM_DLLEXPORT simInt simResetScript(simInt scriptHandle)
{
    return(simResetScript_internal(scriptHandle));
}
SIM_DLLEXPORT simInt simAddScript(simInt scriptProperty)
{
    return(simAddScript_internal(scriptProperty));
}
SIM_DLLEXPORT simInt simRemoveScript(simInt scriptHandle)
{
    return(simRemoveScript_internal(scriptHandle));
}
SIM_DLLEXPORT simInt simRefreshDialogs(simInt refreshDegree)
{
    return(simRefreshDialogs_internal(refreshDegree));
}
SIM_DLLEXPORT simInt simResetProximitySensor(simInt sensorHandle)
{
    return(simResetProximitySensor_internal(sensorHandle));
}
SIM_DLLEXPORT simChar* simCreateBuffer(simInt size)
{
    return(simCreateBuffer_internal(size));
}
SIM_DLLEXPORT simInt simReleaseBuffer(const simChar* buffer)
{
    return(simReleaseBuffer_internal(buffer));
}
SIM_DLLEXPORT simInt simCheckCollision(simInt entity1Handle,simInt entity2Handle)
{
    return(simCheckCollision_internal(entity1Handle,entity2Handle));
}
SIM_DLLEXPORT simInt simGetRealTimeSimulation()
{
    return(simGetRealTimeSimulation_internal());
}
SIM_DLLEXPORT simInt simIsRealTimeSimulationStepNeeded()
{
    return(simIsRealTimeSimulationStepNeeded_internal());
}
SIM_DLLEXPORT simInt simGetSimulationPassesPerRenderingPass()
{
    return(simGetSimulationPassesPerRenderingPass_internal());
}
SIM_DLLEXPORT simInt simAdvanceSimulationByOneStep()
{
    return(simAdvanceSimulationByOneStep_internal());
}
SIM_DLLEXPORT simInt simStartSimulation()
{
    return(simStartSimulation_internal());
}
SIM_DLLEXPORT simInt simStopSimulation()
{
    return(simStopSimulation_internal());
}
SIM_DLLEXPORT simInt simPauseSimulation()
{
    return(simPauseSimulation_internal());
}
SIM_DLLEXPORT simInt simLoadModule(const simChar* filenameAndPath,const simChar* pluginName)
{
    return(simLoadModule_internal(filenameAndPath,pluginName));
}
SIM_DLLEXPORT simInt simUnloadModule(simInt pluginhandle)
{
    return(simUnloadModule_internal(pluginhandle));
}
SIM_DLLEXPORT simChar* simGetModuleName(simInt index,simUChar* moduleVersion)
{
    return(simGetModuleName_internal(index,moduleVersion));
}
SIM_DLLEXPORT simInt simAdjustView(simInt viewHandleOrIndex,simInt associatedViewableObjectHandle,simInt options,const simChar* viewLabel)
{
    return(simAdjustView_internal(viewHandleOrIndex,associatedViewableObjectHandle,options,viewLabel));
}
SIM_DLLEXPORT simInt simSetLastError(const simChar* funcName,const simChar* errorMessage)
{
    return(simSetLastError_internal(funcName,errorMessage));
}
SIM_DLLEXPORT simInt simResetGraph(simInt graphHandle)
{
    return(simResetGraph_internal(graphHandle));
}
SIM_DLLEXPORT simInt simDestroyGraphCurve(simInt graphHandle,simInt curveId)
{
    return(simDestroyGraphCurve_internal(graphHandle,curveId));
}
SIM_DLLEXPORT simInt simDuplicateGraphCurveToStatic(simInt graphHandle,simInt curveId,const simChar* curveName)
{
    return(simDuplicateGraphCurveToStatic_internal(graphHandle,curveId,curveName));
}
SIM_DLLEXPORT simInt simSetNavigationMode(simInt navigationMode)
{
    return(simSetNavigationMode_internal(navigationMode));
}
SIM_DLLEXPORT simInt simGetNavigationMode()
{
    return(simGetNavigationMode_internal());
}
SIM_DLLEXPORT simInt simSetPage(simInt index)
{
    return(simSetPage_internal(index));
}
SIM_DLLEXPORT simInt simGetPage()
{
    return(simGetPage_internal());
}
SIM_DLLEXPORT simInt simRegisterScriptCallbackFunction(const simChar* funcNameAtPluginName,const simChar* callTips,simVoid(*callBack)(struct SScriptCallBack* cb))
{
    return(simRegisterScriptCallbackFunction_internal(funcNameAtPluginName,callTips,callBack));
}
SIM_DLLEXPORT simInt simRegisterScriptVariable(const simChar* varNameAtPluginName,const simChar* varValue,simInt stackHandle)
{
    return(simRegisterScriptVariable_internal(varNameAtPluginName,varValue,stackHandle));
}
SIM_DLLEXPORT simInt simRegisterScriptFuncHook(simInt scriptHandle,const simChar* funcToHook,const simChar* userFunction,simBool executeBefore,simInt options)
{
    return(simRegisterScriptFuncHook_internal(scriptHandle,funcToHook,userFunction,executeBefore,options));
}
SIM_DLLEXPORT simInt simCopyPasteObjects(simInt* objectHandles,simInt objectCount,simInt options)
{
    return(simCopyPasteObjects_internal(objectHandles,objectCount,options));
}
SIM_DLLEXPORT simChar* simReceiveData(simInt dataHeader,const simChar* dataName,simInt antennaHandle,simInt index,simInt* dataLength,simInt* senderID,simInt* dataHeaderR,simChar** dataNameR)
{
    return(simReceiveData_internal(dataHeader,dataName,antennaHandle,index,dataLength,senderID,dataHeaderR,dataNameR));
}
SIM_DLLEXPORT simInt simRemoveDrawingObject(simInt objectHandle)
{
    return(simRemoveDrawingObject_internal(objectHandle));
}
SIM_DLLEXPORT simInt simAnnounceSceneContentChange()
{
    return(simAnnounceSceneContentChange_internal());
}
SIM_DLLEXPORT simInt simSetInt32Signal(const simChar* signalName,simInt signalValue)
{
    return(simSetInt32Signal_internal(signalName,signalValue));
}
SIM_DLLEXPORT simInt simGetInt32Signal(const simChar* signalName,simInt* signalValue)
{
    return(simGetInt32Signal_internal(signalName,signalValue));
}
SIM_DLLEXPORT simInt simClearInt32Signal(const simChar* signalName)
{
    return(simClearInt32Signal_internal(signalName));
}
SIM_DLLEXPORT simInt simClearFloatSignal(const simChar* signalName)
{
    return(simClearFloatSignal_internal(signalName));
}
SIM_DLLEXPORT simInt simSetStringSignal(const simChar* signalName,const simChar* signalValue,simInt stringLength)
{
    return(simSetStringSignal_internal(signalName,signalValue,stringLength));
}
SIM_DLLEXPORT simChar* simGetStringSignal(const simChar* signalName,simInt* stringLength)
{
    return(simGetStringSignal_internal(signalName,stringLength));
}
SIM_DLLEXPORT simInt simClearStringSignal(const simChar* signalName)
{
    return(simClearStringSignal_internal(signalName));
}
SIM_DLLEXPORT simChar* simGetSignalName(simInt signalIndex,simInt signalType)
{
    return(simGetSignalName_internal(signalIndex,signalType));
}
SIM_DLLEXPORT simInt simSetObjectProperty(simInt objectHandle,simInt prop)
{
    return(simSetObjectProperty_internal(objectHandle,prop));
}
SIM_DLLEXPORT simInt simGetObjectProperty(simInt objectHandle)
{
    return(simGetObjectProperty_internal(objectHandle));
}
SIM_DLLEXPORT simInt simSetObjectSpecialProperty(simInt objectHandle,simInt prop)
{
    return(simSetObjectSpecialProperty_internal(objectHandle,prop));
}
SIM_DLLEXPORT simInt simGetObjectSpecialProperty(simInt objectHandle)
{
    return(simGetObjectSpecialProperty_internal(objectHandle));
}
SIM_DLLEXPORT simInt simSetExplicitHandling(simInt objectHandle,int explicitFlags)
{
    return(simSetExplicitHandling_internal(objectHandle,explicitFlags));
}
SIM_DLLEXPORT simInt simGetExplicitHandling(simInt objectHandle)
{
    return(simGetExplicitHandling_internal(objectHandle));
}
SIM_DLLEXPORT simInt simGetLinkDummy(simInt dummyHandle)
{
    return(simGetLinkDummy_internal(dummyHandle));
}
SIM_DLLEXPORT simInt simSetLinkDummy(simInt dummyHandle,simInt linkedDummyHandle)
{
    return(simSetLinkDummy_internal(dummyHandle,linkedDummyHandle));
}
SIM_DLLEXPORT simInt simSetModelProperty(simInt objectHandle,simInt modelProperty)
{
    return(simSetModelProperty_internal(objectHandle,modelProperty));
}
SIM_DLLEXPORT simInt simGetModelProperty(simInt objectHandle)
{
    return(simGetModelProperty_internal(objectHandle));
}
SIM_DLLEXPORT simInt simResetDynamicObject(simInt objectHandle)
{
    return(simResetDynamicObject_internal(objectHandle));
}
SIM_DLLEXPORT simInt simSetJointMode(simInt jointHandle,simInt jointMode,simInt options)
{
    return(simSetJointMode_internal(jointHandle,jointMode,options));
}
SIM_DLLEXPORT simInt simGetJointMode(simInt jointHandle,simInt* options)
{
    return(simGetJointMode_internal(jointHandle,options));
}
SIM_DLLEXPORT simInt simSerialOpen(const simChar* portString,simInt baudRate,simVoid* reserved1,simVoid* reserved2)
{
    return(simSerialOpen_internal(portString,baudRate,reserved1,reserved2));
}
SIM_DLLEXPORT simInt simSerialClose(simInt portHandle)
{
    return(simSerialClose_internal(portHandle));
}
SIM_DLLEXPORT simInt simSerialSend(simInt portHandle,const simChar* data,simInt dataLength)
{
    return(simSerialSend_internal(portHandle,data,dataLength));
}
SIM_DLLEXPORT simInt simSerialRead(simInt portHandle,simChar* buffer,simInt dataLengthToRead)
{
    return(simSerialRead_internal(portHandle,buffer,dataLengthToRead));
}
SIM_DLLEXPORT simInt simSerialCheck(simInt portHandle)
{
    return(simSerialCheck_internal(portHandle));
}
SIM_DLLEXPORT simInt simAuxiliaryConsoleClose(simInt consoleHandle)
{
    return(simAuxiliaryConsoleClose_internal(consoleHandle));
}
SIM_DLLEXPORT simInt simAuxiliaryConsoleShow(simInt consoleHandle,simBool showState)
{
    return(simAuxiliaryConsoleShow_internal(consoleHandle,showState));
}
SIM_DLLEXPORT simInt simAuxiliaryConsolePrint(simInt consoleHandle,const simChar* text)
{
    return(simAuxiliaryConsolePrint_internal(consoleHandle,text));
}
SIM_DLLEXPORT simInt simGetObjectInt32Param(simInt objectHandle,simInt ParamID,simInt* Param)
{
    return(simGetObjectInt32Param_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT simInt simSetObjectInt32Param(simInt objectHandle,simInt ParamID,simInt Param)
{
    return(simSetObjectInt32Param_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT simChar* simGetObjectStringParam(simInt objectHandle,simInt ParamID,simInt* ParamLength)
{
    return(simGetObjectStringParam_internal(objectHandle,ParamID,ParamLength));
}
SIM_DLLEXPORT simInt simSetObjectStringParam(simInt objectHandle,simInt ParamID,const simChar* Param,simInt ParamLength)
{
    return(simSetObjectStringParam_internal(objectHandle,ParamID,Param,ParamLength));
}
SIM_DLLEXPORT simInt simGetScriptInt32Param(simInt ScriptHandle,simInt ParamID,simInt* Param)
{
    return(simGetScriptInt32Param_internal(ScriptHandle,ParamID,Param));
}
SIM_DLLEXPORT simInt simSetScriptInt32Param(simInt ScriptHandle,simInt ParamID,simInt Param)
{
    return(simSetScriptInt32Param_internal(ScriptHandle,ParamID,Param));
}
SIM_DLLEXPORT simChar* simGetScriptStringParam(simInt ScriptHandle,simInt ParamID,simInt* ParamLength)
{
    return(simGetScriptStringParam_internal(ScriptHandle,ParamID,ParamLength));
}
SIM_DLLEXPORT simInt simSetScriptStringParam(simInt ScriptHandle,simInt ParamID,const simChar* Param,simInt ParamLength)
{
    return(simSetScriptStringParam_internal(ScriptHandle,ParamID,Param,ParamLength));
}
SIM_DLLEXPORT simInt simSetSimulationPassesPerRenderingPass(simInt p)
{
    return(simSetSimulationPassesPerRenderingPass_internal(p));
}
SIM_DLLEXPORT simInt simPersistentDataWrite(const simChar* dataName,const simChar* dataValue,simInt dataLength,simInt options)
{
    return(simPersistentDataWrite_internal(dataName,dataValue,dataLength,options));
}
SIM_DLLEXPORT simChar* simPersistentDataRead(const simChar* dataName,simInt* dataLength)
{
    return(simPersistentDataRead_internal(dataName,dataLength));
}
SIM_DLLEXPORT simInt simIsHandle(simInt generalObjectHandle,simInt generalObjectType)
{
    return(simIsHandle_internal(generalObjectHandle,generalObjectType));
}
SIM_DLLEXPORT simInt simResetVisionSensor(simInt visionSensorHandle)
{
    return(simResetVisionSensor_internal(visionSensorHandle));
}
SIM_DLLEXPORT simInt simSetVisionSensorImg(simInt sensorHandle,const simUChar* img,simInt options,const simInt* pos,const simInt* size)
{
    return(simSetVisionSensorImg_internal(sensorHandle,img,options,pos,size));
}
SIM_DLLEXPORT simInt simRuckigPos(simInt dofs,simDouble baseCycleTime,simInt flags,const simDouble* currentPos,const simDouble* currentVel,const simDouble* currentAccel,const simDouble* maxVel,const simDouble* maxAccel,const simDouble* maxJerk,const simBool* selection,const simDouble* targetPos,const simDouble* targetVel,simDouble* reserved1,simInt* reserved2)
{
    return(simRuckigPos_internal(dofs,baseCycleTime,flags,currentPos,currentVel,currentAccel,maxVel,maxAccel,maxJerk,selection,targetPos,targetVel,reserved1,reserved2));
}
SIM_DLLEXPORT simInt simRuckigVel(simInt dofs,simDouble baseCycleTime,simInt flags,const simDouble* currentPos,const simDouble* currentVel,const simDouble* currentAccel,const simDouble* maxAccel,const simDouble* maxJerk,const simBool* selection,const simDouble* targetVel,simDouble* reserved1,simInt* reserved2)
{
    return(simRuckigVel_internal(dofs,baseCycleTime,flags,currentPos,currentVel,currentAccel,maxAccel,maxJerk,selection,targetVel,reserved1,reserved2));
}
SIM_DLLEXPORT simInt simRuckigStep(simInt objHandle,simDouble cycleTime,simDouble* newPos,simDouble* newVel,simDouble* newAccel,simDouble* syncTime,simDouble* reserved1,simInt* reserved2)
{
    return(simRuckigStep_internal(objHandle,cycleTime,newPos,newVel,newAccel,syncTime,reserved1,reserved2));
}
SIM_DLLEXPORT simInt simRuckigRemove(simInt objHandle)
{
    return(simRuckigRemove_internal(objHandle));
}
SIM_DLLEXPORT simInt simGroupShapes(const simInt* shapeHandles,simInt shapeCount)
{
    return(simGroupShapes_internal(shapeHandles,shapeCount));
}
SIM_DLLEXPORT simInt* simUngroupShape(simInt shapeHandle,simInt* shapeCount)
{
    return(simUngroupShape_internal(shapeHandle,shapeCount));
}
SIM_DLLEXPORT simVoid simQuitSimulator(simBool ignoredArgument)
{
    simQuitSimulator_internal(ignoredArgument);
}
SIM_DLLEXPORT simInt simSetShapeMaterial(simInt shapeHandle,simInt materialIdOrShapeHandle)
{
    return(simSetShapeMaterial_internal(shapeHandle,materialIdOrShapeHandle));
}
SIM_DLLEXPORT simInt simGetTextureId(const simChar* textureName,simInt* resolution)
{
    return(simGetTextureId_internal(textureName,resolution));
}
SIM_DLLEXPORT simChar* simReadTexture(simInt textureId,simInt options,simInt posX,simInt posY,simInt sizeX,simInt sizeY)
{
    return(simReadTexture_internal(textureId,options,posX,posY,sizeX,sizeY));
}
SIM_DLLEXPORT simInt simWriteCustomDataBlock(simInt objectHandle,const simChar* tagName,const simChar* data,simInt dataSize)
{
    return(simWriteCustomDataBlock_internal(objectHandle,tagName,data,dataSize));
}
SIM_DLLEXPORT simChar* simReadCustomDataBlock(simInt objectHandle,const simChar* tagName,simInt* dataSize)
{
    return(simReadCustomDataBlock_internal(objectHandle,tagName,dataSize));
}
SIM_DLLEXPORT simChar* simReadCustomDataBlockTags(simInt objectHandle,simInt* tagCount)
{
    return(simReadCustomDataBlockTags_internal(objectHandle,tagCount));
}
SIM_DLLEXPORT simInt simGetObjects(simInt index,simInt objectType)
{
    return(simGetObjects_internal(index,objectType));
}
SIM_DLLEXPORT simInt* simGetObjectsInTree(simInt treeBaseHandle,simInt objectType,simInt options,simInt* objectCount)
{
    return(simGetObjectsInTree_internal(treeBaseHandle,objectType,options,objectCount));
}
SIM_DLLEXPORT simInt simGetShapeTextureId(simInt shapeHandle)
{
    return(simGetShapeTextureId_internal(shapeHandle));
}
SIM_DLLEXPORT simInt simCreateCollectionEx(simInt options)
{
    return(simCreateCollectionEx_internal(options));
}
SIM_DLLEXPORT simInt simAddItemToCollection(simInt collectionHandle,simInt what,simInt objectHandle,simInt options)
{
    return(simAddItemToCollection_internal(collectionHandle,what,objectHandle,options));
}
SIM_DLLEXPORT simInt simDestroyCollection(simInt collectionHandle)
{
    return(simDestroyCollection_internal(collectionHandle));
}
SIM_DLLEXPORT simInt* simGetCollectionObjects(simInt collectionHandle,simInt* objectCount)
{
    return(simGetCollectionObjects_internal(collectionHandle,objectCount));
}
SIM_DLLEXPORT simInt simReorientShapeBoundingBox(simInt shapeHandle,simInt relativeToHandle,simInt reservedSetToZero)
{
    return(simReorientShapeBoundingBox_internal(shapeHandle,relativeToHandle,reservedSetToZero));
}
SIM_DLLEXPORT simInt simSaveImage(const simUChar* image,const simInt* resolution,simInt options,const simChar* filename,simInt quality,simVoid* reserved)
{
    return(simSaveImage_internal(image,resolution,options,filename,quality,reserved));
}
SIM_DLLEXPORT simUChar* simLoadImage(simInt* resolution,simInt options,const simChar* filename,simVoid* reserved)
{
    return(simLoadImage_internal(resolution,options,filename,reserved));
}
SIM_DLLEXPORT simUChar* simGetScaledImage(const simUChar* imageIn,const simInt* resolutionIn,simInt* resolutionOut,simInt options,simVoid* reserved)
{
    return(simGetScaledImage_internal(imageIn,resolutionIn,resolutionOut,options,reserved));
}
SIM_DLLEXPORT simInt simCallScriptFunctionEx(simInt scriptHandleOrType,const simChar* functionNameAtScriptName,simInt stackId)
{
    return(simCallScriptFunctionEx_internal(scriptHandleOrType,functionNameAtScriptName,stackId));
}
SIM_DLLEXPORT simChar* simGetExtensionString(simInt objectHandle,simInt index,const char* key)
{
    return(simGetExtensionString_internal(objectHandle,index,key));
}
SIM_DLLEXPORT simInt simCreateStack()
{
    return(simCreateStack_internal());
}
SIM_DLLEXPORT simInt simReleaseStack(simInt stackHandle)
{
    return(simReleaseStack_internal(stackHandle));
}
SIM_DLLEXPORT simInt simCopyStack(simInt stackHandle)
{
    return(simCopyStack_internal(stackHandle));
}
SIM_DLLEXPORT simInt simPushNullOntoStack(simInt stackHandle)
{
    return(simPushNullOntoStack_internal(stackHandle));
}
SIM_DLLEXPORT simInt simPushBoolOntoStack(simInt stackHandle,simBool value)
{
    return(simPushBoolOntoStack_internal(stackHandle,value));
}
SIM_DLLEXPORT simInt simPushInt32OntoStack(simInt stackHandle,simInt value)
{
    return(simPushInt32OntoStack_internal(stackHandle,value));
}
SIM_DLLEXPORT simInt simPushInt64OntoStack(simInt stackHandle,simInt64 value)
{
    return(simPushInt64OntoStack_internal(stackHandle,value));
}
SIM_DLLEXPORT simInt simPushFloatOntoStack(simInt stackHandle,simFloat value)
{
    return(simPushFloatOntoStack_internal(stackHandle,value));
}
SIM_DLLEXPORT simInt simPushDoubleOntoStack(simInt stackHandle,simDouble value)
{
    return(simPushDoubleOntoStack_internal(stackHandle,value));
}
SIM_DLLEXPORT simInt simPushStringOntoStack(simInt stackHandle,const simChar* value,simInt stringSize)
{
    return(simPushStringOntoStack_internal(stackHandle,value,stringSize));
}
SIM_DLLEXPORT simInt simPushUInt8TableOntoStack(simInt stackHandle,const simUChar* values,simInt valueCnt)
{
    return(simPushUInt8TableOntoStack_internal(stackHandle,values,valueCnt));
}
SIM_DLLEXPORT simInt simPushInt32TableOntoStack(simInt stackHandle,const simInt* values,simInt valueCnt)
{
    return(simPushInt32TableOntoStack_internal(stackHandle,values,valueCnt));
}
SIM_DLLEXPORT simInt simPushInt64TableOntoStack(simInt stackHandle,const simInt64* values,simInt valueCnt)
{
    return(simPushInt64TableOntoStack_internal(stackHandle,values,valueCnt));
}
SIM_DLLEXPORT simInt simPushFloatTableOntoStack(simInt stackHandle,const simFloat* values,simInt valueCnt)
{
    return(simPushFloatTableOntoStack_internal(stackHandle,values,valueCnt));
}
SIM_DLLEXPORT simInt simPushDoubleTableOntoStack(simInt stackHandle,const simDouble* values,simInt valueCnt)
{
    return(simPushDoubleTableOntoStack_internal(stackHandle,values,valueCnt));
}
SIM_DLLEXPORT simInt simPushTableOntoStack(simInt stackHandle)
{
    return(simPushTableOntoStack_internal(stackHandle));
}
SIM_DLLEXPORT simInt simInsertDataIntoStackTable(simInt stackHandle)
{
    return(simInsertDataIntoStackTable_internal(stackHandle));
}
SIM_DLLEXPORT simInt simGetStackSize(simInt stackHandle)
{
    return(simGetStackSize_internal(stackHandle));
}
SIM_DLLEXPORT simInt simPopStackItem(simInt stackHandle,simInt count)
{
    return(simPopStackItem_internal(stackHandle,count));
}
SIM_DLLEXPORT simInt simMoveStackItemToTop(simInt stackHandle,simInt cIndex)
{
    return(simMoveStackItemToTop_internal(stackHandle,cIndex));
}
SIM_DLLEXPORT simInt simIsStackValueNull(simInt stackHandle)
{
    return(simIsStackValueNull_internal(stackHandle));
}
SIM_DLLEXPORT simInt simGetStackBoolValue(simInt stackHandle,simBool* boolValue)
{
    return(simGetStackBoolValue_internal(stackHandle,boolValue));
}
SIM_DLLEXPORT simInt simGetStackInt32Value(simInt stackHandle,simInt* numberValue)
{
    return(simGetStackInt32Value_internal(stackHandle,numberValue));
}
SIM_DLLEXPORT simInt simGetStackInt64Value(simInt stackHandle,simInt64* numberValue)
{
    return(simGetStackInt64Value_internal(stackHandle,numberValue));
}
SIM_DLLEXPORT simInt simGetStackFloatValue(simInt stackHandle,simFloat* numberValue)
{
    return(simGetStackFloatValue_internal(stackHandle,numberValue));
}
SIM_DLLEXPORT simInt simGetStackDoubleValue(simInt stackHandle,simDouble* numberValue)
{
    return(simGetStackDoubleValue_internal(stackHandle,numberValue));
}
SIM_DLLEXPORT simChar* simGetStackStringValue(simInt stackHandle,simInt* stringSize)
{
    return(simGetStackStringValue_internal(stackHandle,stringSize));
}
SIM_DLLEXPORT simInt simGetStackTableInfo(simInt stackHandle,simInt infoType)
{
    return(simGetStackTableInfo_internal(stackHandle,infoType));
}
SIM_DLLEXPORT simInt simGetStackUInt8Table(simInt stackHandle,simUChar* array,simInt count)
{
    return(simGetStackUInt8Table_internal(stackHandle,array,count));
}
SIM_DLLEXPORT simInt simGetStackInt32Table(simInt stackHandle,simInt* array,simInt count)
{
    return(simGetStackInt32Table_internal(stackHandle,array,count));
}
SIM_DLLEXPORT simInt simGetStackInt64Table(simInt stackHandle,simInt64* array,simInt count)
{
    return(simGetStackInt64Table_internal(stackHandle,array,count));
}
SIM_DLLEXPORT simInt simGetStackFloatTable(simInt stackHandle,simFloat* array,simInt count)
{
    return(simGetStackFloatTable_internal(stackHandle,array,count));
}
SIM_DLLEXPORT simInt simGetStackDoubleTable(simInt stackHandle,simDouble* array,simInt count)
{
    return(simGetStackDoubleTable_internal(stackHandle,array,count));
}
SIM_DLLEXPORT simInt simUnfoldStackTable(simInt stackHandle)
{
    return(simUnfoldStackTable_internal(stackHandle));
}
SIM_DLLEXPORT simInt simDebugStack(simInt stackHandle,simInt cIndex)
{
    return(simDebugStack_internal(stackHandle,cIndex));
}
SIM_DLLEXPORT simInt simGetEngineInt32Param(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{
    return(simGetEngineInt32Param_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT simBool simGetEngineBoolParam(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{
    return(simGetEngineBoolParam_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT simInt simSetEngineInt32Param(simInt paramId,simInt objectHandle,const simVoid* object,simInt val)
{
    return(simSetEngineInt32Param_internal(paramId,objectHandle,object,val));
}
SIM_DLLEXPORT simInt simSetEngineBoolParam(simInt paramId,simInt objectHandle,const simVoid* object,simBool val)
{
    return(simSetEngineBoolParam_internal(paramId,objectHandle,object,val));
}
SIM_DLLEXPORT simInt simInsertObjectIntoOctree(simInt octreeHandle,simInt objectHandle,simInt options,const simUChar* color,simUInt tag,simVoid* reserved)
{
    return(simInsertObjectIntoOctree_internal(octreeHandle,objectHandle,options,color,tag,reserved));
}
SIM_DLLEXPORT simInt simSubtractObjectFromOctree(simInt octreeHandle,simInt objectHandle,simInt options,simVoid* reserved)
{
    return(simSubtractObjectFromOctree_internal(octreeHandle,objectHandle,options,reserved));
}
SIM_DLLEXPORT simChar* simOpenTextEditor(const simChar* initText,const simChar* xml,simInt* various)
{
    return(simOpenTextEditor_internal(initText,xml,various));
}
SIM_DLLEXPORT simChar* simPackTable(simInt stackHandle,simInt* bufferSize)
{
    return(simPackTable_internal(stackHandle,bufferSize));
}
SIM_DLLEXPORT simInt simUnpackTable(simInt stackHandle,const simChar* buffer,simInt bufferSize)
{
    return(simUnpackTable_internal(stackHandle,buffer,bufferSize));
}
SIM_DLLEXPORT simInt simSetReferencedHandles(simInt objectHandle,simInt count,const simInt* referencedHandles,const simInt* reserved1,const simInt* reserved2)
{
    return(simSetReferencedHandles_internal(objectHandle,count,referencedHandles,reserved1,reserved2));
}
SIM_DLLEXPORT simInt simGetReferencedHandles(simInt objectHandle,simInt** referencedHandles,simInt** reserved1,simInt** reserved2)
{
    return(simGetReferencedHandles_internal(objectHandle,referencedHandles,reserved1,reserved2));
}
SIM_DLLEXPORT simInt simGetShapeViz(simInt shapeHandle,simInt index,struct SShapeVizInfo* info)
{
    return(simGetShapeViz_internal(shapeHandle,index,info));
}
SIM_DLLEXPORT simInt simExecuteScriptString(simInt scriptHandleOrType,const simChar* stringAtScriptName,simInt stackHandle)
{
    return(simExecuteScriptString_internal(scriptHandleOrType,stringAtScriptName,stackHandle));
}
SIM_DLLEXPORT simChar* simGetApiFunc(simInt scriptHandleOrType,const simChar* apiWord)
{
    return(simGetApiFunc_internal(scriptHandleOrType,apiWord));
}
SIM_DLLEXPORT simChar* simGetApiInfo(simInt scriptHandleOrType,const simChar* apiWord)
{
    return(simGetApiInfo_internal(scriptHandleOrType,apiWord));
}
SIM_DLLEXPORT simInt simSetModuleInfo(const simChar* moduleName,simInt infoType,const simChar* stringInfo,simInt intInfo)
{
    return(simSetModuleInfo_internal(moduleName,infoType,stringInfo,intInfo));
}
SIM_DLLEXPORT simInt simGetModuleInfo(const simChar* moduleName,simInt infoType,simChar** stringInfo,simInt* intInfo)
{
    return(simGetModuleInfo_internal(moduleName,infoType,stringInfo,intInfo));
}
SIM_DLLEXPORT simInt simIsDeprecated(const simChar* funcOrConst)
{
    return(simIsDeprecated_internal(funcOrConst));
}
SIM_DLLEXPORT simChar* simGetPersistentDataTags(simInt* tagCount)
{
    return(simGetPersistentDataTags_internal(tagCount));
}
SIM_DLLEXPORT simInt simEventNotification(const simChar* event)
{
    return(simEventNotification_internal(event));
}
SIM_DLLEXPORT simInt simAddLog(const simChar* pluginName,simInt verbosityLevel,const simChar* logMsg)
{
    return(simAddLog_internal(pluginName,verbosityLevel,logMsg));
}
SIM_DLLEXPORT simInt simIsDynamicallyEnabled(simInt objectHandle)
{
    return(simIsDynamicallyEnabled_internal(objectHandle));
}
SIM_DLLEXPORT simInt simInitScript(simInt scriptHandle)
{
    return(simInitScript_internal(scriptHandle));
}
SIM_DLLEXPORT simInt simModuleEntry(simInt handle,const simChar* label,simInt state)
{
    return(simModuleEntry_internal(handle,label,state));
}
SIM_DLLEXPORT simInt simCheckExecAuthorization(const simChar* what,const simChar* args)
{
    return(simCheckExecAuthorization_internal(what,args,-1));
}
SIM_DLLEXPORT simVoid _simSetDynamicSimulationIconCode(simVoid* object,simInt code)
{
    return(_simSetDynamicSimulationIconCode_internal(object,code));
}
SIM_DLLEXPORT simVoid _simSetDynamicObjectFlagForVisualization(simVoid* object,simInt flag)
{
    return(_simSetDynamicObjectFlagForVisualization_internal(object,flag));
}
SIM_DLLEXPORT simInt _simGetObjectListSize(simInt objType)
{
    return(_simGetObjectListSize_internal(objType));
}
SIM_DLLEXPORT const simVoid* _simGetObjectFromIndex(simInt objType,simInt index)
{
    return(_simGetObjectFromIndex_internal(objType,index));
}
SIM_DLLEXPORT simInt _simGetObjectID(const simVoid* object)
{
    return(_simGetObjectID_internal(object));
}
SIM_DLLEXPORT simInt _simGetObjectType(const simVoid* object)
{
    return(_simGetObjectType_internal(object));
}
SIM_DLLEXPORT const simVoid** _simGetObjectChildren(const simVoid* object,simInt* count)
{
    return(_simGetObjectChildren_internal(object,count));
}
SIM_DLLEXPORT const simVoid* _simGetGeomProxyFromShape(const simVoid* shape)
{
    return(_simGetGeomProxyFromShape_internal(shape));
}
SIM_DLLEXPORT const simVoid* _simGetParentObject(const simVoid* object)
{
    return(_simGetParentObject_internal(object));
}
SIM_DLLEXPORT const simVoid* _simGetObject(int objID)
{
    return(_simGetObject_internal(objID));
}
SIM_DLLEXPORT simBool _simIsShapeDynamicallyStatic(const simVoid* shape)
{
    return(_simIsShapeDynamicallyStatic_internal(shape));
}
SIM_DLLEXPORT simInt _simGetTreeDynamicProperty(const simVoid* object)
{
    return(_simGetTreeDynamicProperty_internal(object));
}
SIM_DLLEXPORT simInt _simGetDummyLinkType(const simVoid* dummy,simInt* linkedDummyID)
{
    return(_simGetDummyLinkType_internal(dummy,linkedDummyID));
}
SIM_DLLEXPORT simInt _simGetJointMode(const simVoid* joint)
{
    return(_simGetJointMode_internal(joint));
}
SIM_DLLEXPORT simBool _simIsJointInHybridOperation(const simVoid* joint)
{
    return(_simIsJointInHybridOperation_internal(joint));
}
SIM_DLLEXPORT simVoid _simDisableDynamicTreeForManipulation(const simVoid* object,simBool disableFlag)
{
    return(_simDisableDynamicTreeForManipulation_internal(object,disableFlag));
}
SIM_DLLEXPORT simBool _simIsShapeDynamicallyRespondable(const simVoid* shape)
{
    return(_simIsShapeDynamicallyRespondable_internal(shape));
}
SIM_DLLEXPORT simInt _simGetDynamicCollisionMask(const simVoid* shape)
{
    return(_simGetDynamicCollisionMask_internal(shape));
}
SIM_DLLEXPORT const simVoid* _simGetLastParentForLocalGlobalCollidable(const simVoid* shape)
{
    return(_simGetLastParentForLocalGlobalCollidable_internal(shape));
}
SIM_DLLEXPORT simBool _simGetStartSleeping(const simVoid* shape)
{
    return(_simGetStartSleeping_internal(shape));
}
SIM_DLLEXPORT simBool _simGetWasPutToSleepOnce(const simVoid* shape)
{
    return(_simGetWasPutToSleepOnce_internal(shape));
}
SIM_DLLEXPORT simBool _simGetDynamicsFullRefreshFlag(const simVoid* object)
{
    return(_simGetDynamicsFullRefreshFlag_internal(object));
}
SIM_DLLEXPORT simVoid _simSetDynamicsFullRefreshFlag(const simVoid* object,simBool flag)
{
    return(_simSetDynamicsFullRefreshFlag_internal(object,flag));
}
SIM_DLLEXPORT simVoid _simClearAdditionalForceAndTorque(const simVoid* shape)
{
    return(_simClearAdditionalForceAndTorque_internal(shape));
}
SIM_DLLEXPORT simInt _simGetJointType(const simVoid* joint)
{
    return(_simGetJointType_internal(joint));
}
SIM_DLLEXPORT const simVoid* _simGetGeomWrapFromGeomProxy(const simVoid* geomData)
{
    return(_simGetGeomWrapFromGeomProxy_internal(geomData));
}
SIM_DLLEXPORT simInt _simGetPurePrimitiveType(const simVoid* geomInfo)
{
    return(_simGetPurePrimitiveType_internal(geomInfo));
}
SIM_DLLEXPORT simBool _simIsGeomWrapGeometric(const simVoid* geomInfo)
{
    return(_simIsGeomWrapGeometric_internal(geomInfo));
}
SIM_DLLEXPORT simBool _simIsGeomWrapConvex(const simVoid* geomInfo)
{
    return(_simIsGeomWrapConvex_internal(geomInfo));
}
SIM_DLLEXPORT simInt _simGetGeometricCount(const simVoid* geomInfo)
{
    return(_simGetGeometricCount_internal(geomInfo));
}
SIM_DLLEXPORT simVoid _simGetAllGeometrics(const simVoid* geomInfo,simVoid** allGeometrics)
{
    return(_simGetAllGeometrics_internal(geomInfo,allGeometrics));
}
SIM_DLLEXPORT simVoid _simMakeDynamicAnnouncement(int announceType)
{
    return(_simMakeDynamicAnnouncement_internal(announceType));
}
SIM_DLLEXPORT simInt _simGetTimeDiffInMs(simInt previousTime)
{
    return(_simGetTimeDiffInMs_internal(previousTime));
}
SIM_DLLEXPORT simBool _simDoEntitiesCollide(simInt entity1ID,simInt entity2ID,simInt* cacheBuffer,simBool overrideCollidableFlagIfShape1,simBool overrideCollidableFlagIfShape2,simBool pathOrMotionPlanningRoutineCalling)
{
    return(_simDoEntitiesCollide_internal(entity1ID,entity2ID,cacheBuffer,overrideCollidableFlagIfShape1,overrideCollidableFlagIfShape2,pathOrMotionPlanningRoutineCalling));
}
SIM_DLLEXPORT simInt simFloatingViewRemove(simInt floatingViewHandle)
{
    return(simFloatingViewRemove_internal(floatingViewHandle));
}
SIM_DLLEXPORT const simVoid* _simGetIkGroupObject(int ikGroupID)
{
    return(_simGetIkGroupObject_internal(ikGroupID));
}
SIM_DLLEXPORT simInt _simMpHandleIkGroupObject(const simVoid* ikGroup)
{
    return(_simMpHandleIkGroupObject_internal(ikGroup));
}
// Following courtesy of Stephen James:
// Functions to allow an external application to have control of CoppeliaSim's thread loop
SIM_DLLEXPORT simInt simExtLaunchUIThread(const simChar* applicationName,simInt options,const simChar* sceneOrModelOrUiToLoad, const simChar* applicationDir_)
{
    return(simExtLaunchUIThread_internal(applicationName,options,sceneOrModelOrUiToLoad,applicationDir_));
}
SIM_DLLEXPORT simInt simExtPostExitRequest()
{
    return(simExtPostExitRequest_internal());
}
SIM_DLLEXPORT simInt simExtGetExitRequest()
{
    return(simExtGetExitRequest_internal());
}
SIM_DLLEXPORT simInt simExtStep(simBool stepIfRunning)
{
    return(simExtStep_internal(stepIfRunning));
}
SIM_DLLEXPORT simInt simExtCanInitSimThread()
{
    return(simExtCanInitSimThread_internal());
}
SIM_DLLEXPORT simInt simExtSimThreadInit()
{
    return(simExtSimThreadInit_internal());
}
SIM_DLLEXPORT simInt simExtSimThreadDestroy()
{
    return(simExtSimThreadDestroy_internal());
}

#ifdef switchToDouble
SIM_DLLEXPORT simInt simSetFloatParam_D(simInt parameter,simDouble floatState)
{
    return(simSetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT simInt simGetFloatParam_D(simInt parameter,simDouble* floatState)
{
    return(simGetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT simInt simGetObjectMatrix_D(simInt objectHandle,simInt relativeToObjectHandle,simDouble* matrix)
{
    return(simGetObjectMatrix_internal(objectHandle,relativeToObjectHandle,matrix));
}
SIM_DLLEXPORT simInt simSetObjectMatrix_D(simInt objectHandle,simInt relativeToObjectHandle,const simDouble* matrix)
{
    return(simSetObjectMatrix_internal(objectHandle,relativeToObjectHandle,matrix));
}
SIM_DLLEXPORT simInt simGetObjectPose_D(simInt objectHandle,simInt relativeToObjectHandle,simDouble* pose)
{
    return(simGetObjectPose_internal(objectHandle,relativeToObjectHandle,pose));
}
SIM_DLLEXPORT simInt simSetObjectPose_D(simInt objectHandle,simInt relativeToObjectHandle,const simDouble* pose)
{
    return(simSetObjectPose_internal(objectHandle,relativeToObjectHandle,pose));
}
SIM_DLLEXPORT simInt simGetObjectPosition_D(simInt objectHandle,simInt relativeToObjectHandle,simDouble* position)
{
    return(simGetObjectPosition_internal(objectHandle,relativeToObjectHandle,position));
}
SIM_DLLEXPORT simInt simSetObjectPosition_D(simInt objectHandle,simInt relativeToObjectHandle,const simDouble* position)
{
    return(simSetObjectPosition_internal(objectHandle,relativeToObjectHandle,position));
}
SIM_DLLEXPORT simInt simGetObjectOrientation_D(simInt objectHandle,simInt relativeToObjectHandle,simDouble* eulerAngles)
{
    return(simGetObjectOrientation_internal(objectHandle,relativeToObjectHandle,eulerAngles));
}
SIM_DLLEXPORT simInt simGetObjectQuaternion_D(simInt objectHandle,simInt relativeToObjectHandle,simDouble* quaternion)
{
    return(simGetObjectQuaternion_internal(objectHandle,relativeToObjectHandle,quaternion));
}
SIM_DLLEXPORT simInt simSetObjectQuaternion_D(simInt objectHandle,simInt relativeToObjectHandle,const simDouble* quaternion)
{
    return(simSetObjectQuaternion_internal(objectHandle,relativeToObjectHandle,quaternion));
}
SIM_DLLEXPORT simInt simSetObjectOrientation_D(simInt objectHandle,simInt relativeToObjectHandle,const simDouble* eulerAngles)
{
    return(simSetObjectOrientation_internal(objectHandle,relativeToObjectHandle,eulerAngles));
}
SIM_DLLEXPORT simInt simGetJointPosition_D(simInt objectHandle,simDouble* position)
{
    return(simGetJointPosition_internal(objectHandle,position));
}
SIM_DLLEXPORT simInt simSetJointPosition_D(simInt objectHandle,simDouble position)
{
    return(simSetJointPosition_internal(objectHandle,position));
}
SIM_DLLEXPORT simInt simSetJointTargetPosition_D(simInt objectHandle,simDouble targetPosition)
{
    return(simSetJointTargetPosition_internal(objectHandle,targetPosition));
}
SIM_DLLEXPORT simInt simGetJointTargetPosition_D(simInt objectHandle,simDouble* targetPosition)
{
    return(simGetJointTargetPosition_internal(objectHandle,targetPosition));
}
SIM_DLLEXPORT simInt simGetObjectChildPose_D(simInt objectHandle,simDouble* pose)
{
    return(simGetObjectChildPose_internal(objectHandle,pose));
}
SIM_DLLEXPORT simInt simSetObjectChildPose_D(simInt objectHandle,const simDouble* pose)
{
    return(simSetObjectChildPose_internal(objectHandle,pose));
}
SIM_DLLEXPORT simInt simGetJointInterval_D(simInt objectHandle,simBool* cyclic,simDouble* interval)
{
    return(simGetJointInterval_internal(objectHandle,cyclic,interval));
}
SIM_DLLEXPORT simInt simSetJointInterval_D(simInt objectHandle,simBool cyclic,const simDouble* interval)
{
    return(simSetJointInterval_internal(objectHandle,cyclic,interval));
}
SIM_DLLEXPORT simInt simBuildIdentityMatrix_D(simDouble* matrix)
{
    return(simBuildIdentityMatrix_internal(matrix));
}
SIM_DLLEXPORT simInt simBuildMatrix_D(const simDouble* position,const simDouble* eulerAngles,simDouble* matrix)
{
    return(simBuildMatrix_internal(position,eulerAngles,matrix));
}
SIM_DLLEXPORT simInt simBuildPose_D(const simDouble* position,const simDouble* eulerAngles,simDouble* pose)
{
    return(simBuildPose_internal(position,eulerAngles,pose));
}
SIM_DLLEXPORT simInt simGetEulerAnglesFromMatrix_D(const simDouble* matrix,simDouble* eulerAngles)
{
    return(simGetEulerAnglesFromMatrix_internal(matrix,eulerAngles));
}
SIM_DLLEXPORT simInt simInvertMatrix_D(simDouble* matrix)
{
    return(simInvertMatrix_internal(matrix));
}
SIM_DLLEXPORT simInt simMultiplyMatrices_D(const simDouble* matrixIn1,const simDouble* matrixIn2,simDouble* matrixOut)
{
    return(simMultiplyMatrices_internal(matrixIn1,matrixIn2,matrixOut));
}
SIM_DLLEXPORT simInt simMultiplyPoses_D(const simDouble* poseIn1,const simDouble* poseIn2,simDouble* poseOut)
{
    return(simMultiplyPoses_internal(poseIn1,poseIn2,poseOut));
}
SIM_DLLEXPORT simInt simInvertPose_D(simDouble* pose)
{
    return(simInvertPose_internal(pose));
}
SIM_DLLEXPORT simInt simInterpolatePoses_D(const simDouble* poseIn1,const simDouble* poseIn2,simDouble interpolFactor,simDouble* poseOut)
{
    return(simInterpolatePoses_internal(poseIn1,poseIn2,interpolFactor,poseOut));
}
SIM_DLLEXPORT simInt simPoseToMatrix_D(const simDouble* poseIn,simDouble* matrixOut)
{
    return(simPoseToMatrix_internal(poseIn,matrixOut));
}
SIM_DLLEXPORT simInt simMatrixToPose_D(const simDouble* matrixIn,simDouble* poseOut)
{
    return(simMatrixToPose_internal(matrixIn,poseOut));
}
SIM_DLLEXPORT simInt simInterpolateMatrices_D(const simDouble* matrixIn1,const simDouble* matrixIn2,simDouble interpolFactor,simDouble* matrixOut)
{
    return(simInterpolateMatrices_internal(matrixIn1,matrixIn2,interpolFactor,matrixOut));
}
SIM_DLLEXPORT simInt simTransformVector_D(const simDouble* matrix,simDouble* vect)
{
    return(simTransformVector_internal(matrix,vect));
}
SIM_DLLEXPORT simDouble simGetSimulationTime_D()
{
    return(simGetSimulationTime_internal());
}
SIM_DLLEXPORT simDouble simGetSystemTime_D()
{
    return(double(simGetSystemTime_internal()));
}
SIM_DLLEXPORT simInt simHandleProximitySensor_D(simInt sensorHandle,simDouble* detectedPoint,simInt* detectedObjectHandle,simDouble* normalVector)
{
    return(simHandleProximitySensor_internal(sensorHandle,detectedPoint,detectedObjectHandle,normalVector));
}
SIM_DLLEXPORT simInt simReadProximitySensor_D(simInt sensorHandle,simDouble* detectedPoint,simInt* detectedObjectHandle,simDouble* normalVector)
{
    return(simReadProximitySensor_internal(sensorHandle,detectedPoint,detectedObjectHandle,normalVector));
}
SIM_DLLEXPORT simInt simHandleDynamics_D(simDouble deltaTime)
{
    return(simHandleDynamics_internal(deltaTime));
}
SIM_DLLEXPORT simInt simCheckProximitySensor_D(simInt sensorHandle,simInt entityHandle,simDouble* detectedPoint)
{
    return(simCheckProximitySensor_internal(sensorHandle,entityHandle,detectedPoint));
}
SIM_DLLEXPORT simInt simCheckProximitySensorEx_D(simInt sensorHandle,simInt entityHandle,simInt detectionMode,simDouble detectionThreshold,simDouble maxAngle,simDouble* detectedPoint,simInt* detectedObjectHandle,simDouble* normalVector)
{
    return(simCheckProximitySensorEx_internal(sensorHandle,entityHandle,detectionMode,detectionThreshold,maxAngle,detectedPoint,detectedObjectHandle,normalVector));
}
SIM_DLLEXPORT simInt simCheckProximitySensorEx2_D(simInt sensorHandle,simDouble* vertexPointer,simInt itemType,simInt itemCount,simInt detectionMode,simDouble detectionThreshold,simDouble maxAngle,simDouble* detectedPoint,simDouble* normalVector)
{
    return(simCheckProximitySensorEx2_internal(sensorHandle,vertexPointer,itemType,itemCount,detectionMode,detectionThreshold,maxAngle,detectedPoint,normalVector));
}
SIM_DLLEXPORT simInt simCheckCollisionEx_D(simInt entity1Handle,simInt entity2Handle,simDouble** intersectionSegments)
{
    return(simCheckCollisionEx_internal(entity1Handle,entity2Handle,intersectionSegments));
}
SIM_DLLEXPORT simInt simCheckDistance_D(simInt entity1Handle,simInt entity2Handle,simDouble threshold,simDouble* distanceData)
{
    return(simCheckDistance_internal(entity1Handle,entity2Handle,threshold,distanceData));
}
SIM_DLLEXPORT simInt simSetSimulationTimeStep_D(simDouble timeStep)
{
    return(simSetSimulationTimeStep_internal(timeStep));
}
SIM_DLLEXPORT simDouble simGetSimulationTimeStep_D()
{
    return(simGetSimulationTimeStep_internal());
}
SIM_DLLEXPORT simInt simAdjustRealTimeTimer_D(simInt instanceIndex,simDouble deltaTime)
{
    return(simAdjustRealTimeTimer_internal(instanceIndex,deltaTime));
}
SIM_DLLEXPORT simInt simFloatingViewAdd_D(simDouble posX,simDouble posY,simDouble sizeX,simDouble sizeY,simInt options)
{
    return(simFloatingViewAdd_internal(posX,posY,sizeX,sizeY,options));
}
SIM_DLLEXPORT simInt simFloatingViewRemove_D(simInt floatingViewHandle)
{
    return(simFloatingViewRemove_internal(floatingViewHandle));
}
SIM_DLLEXPORT simInt simHandleGraph_D(simInt graphHandle,simDouble simulationTime)
{
    return(simHandleGraph_internal(graphHandle,simulationTime));
}
SIM_DLLEXPORT simInt simAddGraphStream_D(simInt graphHandle,const simChar* streamName,const simChar* unitStr,simInt options,const simDouble* color,simDouble cyclicRange)
{
    return(simAddGraphStream_internal(graphHandle,streamName,unitStr,options,color,cyclicRange));
}
SIM_DLLEXPORT simInt simSetGraphStreamTransformation_D(simInt graphHandle,simInt streamId,simInt trType,simDouble mult,simDouble off,simInt movingAvgPeriod)
{
    return(simSetGraphStreamTransformation_internal(graphHandle,streamId,trType,mult,off,movingAvgPeriod));
}
SIM_DLLEXPORT simInt simAddGraphCurve_D(simInt graphHandle,const simChar* curveName,simInt dim,const simInt* streamIds,const simDouble* defaultValues,const simChar* unitStr,simInt options,const simDouble* color,simInt curveWidth)
{
    return(simAddGraphCurve_internal(graphHandle,curveName,dim,streamIds,defaultValues,unitStr,options,color,curveWidth));
}
SIM_DLLEXPORT simInt simSetGraphStreamValue_D(simInt graphHandle,simInt streamId,simDouble value)
{
    return(simSetGraphStreamValue_internal(graphHandle,streamId,value));
}
SIM_DLLEXPORT simInt simSetJointTargetVelocity_D(simInt objectHandle,simDouble targetVelocity)
{
    return(simSetJointTargetVelocity_internal(objectHandle,targetVelocity));
}
SIM_DLLEXPORT simInt simGetJointTargetVelocity_D(simInt objectHandle,simDouble* targetVelocity)
{
    return(simGetJointTargetVelocity_internal(objectHandle,targetVelocity));
}
SIM_DLLEXPORT simInt simScaleObjects_D(const simInt* objectHandles,simInt objectCount,simDouble scalingFactor,simBool scalePositionsToo)
{
    return(simScaleObjects_internal(objectHandles,objectCount,scalingFactor,scalePositionsToo));
}
SIM_DLLEXPORT simInt simAddDrawingObject_D(simInt objectType,simDouble size,simDouble duplicateTolerance,simInt parentObjectHandle,simInt maxItemCount,const simDouble* color,const simDouble* setToNULL,const simDouble* setToNULL2,const simDouble* setToNULL3)
{
    return(simAddDrawingObject_internal(objectType,size,duplicateTolerance,parentObjectHandle,maxItemCount,color,setToNULL,setToNULL2,setToNULL3));
}
SIM_DLLEXPORT simInt simAddDrawingObjectItem_D(simInt objectHandle,const simDouble* itemData)
{
    return(simAddDrawingObjectItem_internal(objectHandle,itemData));
}
SIM_DLLEXPORT simDouble simGetObjectSizeFactor_D(simInt objectHandle)
{
    return(simGetObjectSizeFactor_internal(objectHandle));
}
SIM_DLLEXPORT simInt simSetFloatSignal_D(const simChar* signalName,simDouble signalValue)
{
    return(simSetFloatSignal_internal(signalName,signalValue));
}
SIM_DLLEXPORT simInt simGetFloatSignal_D(const simChar* signalName,simDouble* signalValue)
{
    return(simGetFloatSignal_internal(signalName,signalValue));
}
SIM_DLLEXPORT simInt simReadForceSensor_D(simInt objectHandle,simDouble* forceVector,simDouble* torqueVector)
{
    return(simReadForceSensor_internal(objectHandle,forceVector,torqueVector));
}
SIM_DLLEXPORT simInt simSetLightParameters_D(simInt objectHandle,simInt state,const simDouble* setToNULL,const simDouble* diffusePart,const simDouble* specularPart)
{
    return(simSetLightParameters_internal(objectHandle,state,setToNULL,diffusePart,specularPart));
}
SIM_DLLEXPORT simInt simGetLightParameters_D(simInt objectHandle,simDouble* setToNULL,simDouble* diffusePart,simDouble* specularPart)
{
    return(simGetLightParameters_internal(objectHandle,setToNULL,diffusePart,specularPart));
}
SIM_DLLEXPORT simInt simGetVelocity_D(simInt shapeHandle,simDouble* linearVelocity,simDouble* angularVelocity)
{
    return(simGetVelocity_internal(shapeHandle,linearVelocity,angularVelocity));
}
SIM_DLLEXPORT simInt simGetObjectVelocity_D(simInt objectHandle,simDouble* linearVelocity,simDouble* angularVelocity)
{
    return(simGetObjectVelocity_internal(objectHandle,linearVelocity,angularVelocity));
}
SIM_DLLEXPORT simInt simGetJointVelocity_D(simInt jointHandle,simDouble* velocity)
{
    return(simGetJointVelocity_internal(jointHandle,velocity));
}
SIM_DLLEXPORT simInt simAddForceAndTorque_D(simInt shapeHandle,const simDouble* force,const simDouble* torque)
{
    return(simAddForceAndTorque_internal(shapeHandle,force,torque));
}
SIM_DLLEXPORT simInt simAddForce_D(simInt shapeHandle,const simDouble* position,const simDouble* force)
{
    return(simAddForce_internal(shapeHandle,position,force));
}
SIM_DLLEXPORT simInt simSetObjectColor_D(simInt objectHandle,simInt index,simInt colorComponent,const simDouble* rgbData)
{
    return(simSetObjectColor_internal(objectHandle,index,colorComponent,rgbData));
}
SIM_DLLEXPORT simInt simGetObjectColor_D(simInt objectHandle,simInt index,simInt colorComponent,simDouble* rgbData)
{
    return(simGetObjectColor_internal(objectHandle,index,colorComponent,rgbData));
}
SIM_DLLEXPORT simInt simSetShapeColor_D(simInt shapeHandle,const simChar* colorName,simInt colorComponent,const simDouble* rgbData)
{
    return(simSetShapeColor_internal(shapeHandle,colorName,colorComponent,rgbData));
}
SIM_DLLEXPORT simInt simGetShapeColor_D(simInt shapeHandle,const simChar* colorName,simInt colorComponent,simDouble* rgbData)
{
    return(simGetShapeColor_internal(shapeHandle,colorName,colorComponent,rgbData));
}
SIM_DLLEXPORT simInt simGetContactInfo_D(simInt dynamicPass,simInt objectHandle,simInt index,simInt* objectHandles,simDouble* contactInfo)
{
    return(simGetContactInfo_internal(dynamicPass,objectHandle,index,objectHandles,contactInfo));
}
SIM_DLLEXPORT simInt simAuxiliaryConsoleOpen_D(const simChar* title,simInt maxLines,simInt mode,const simInt* position,const simInt* size,const simDouble* textColor,const simDouble* backgroundColor)
{
    return(simAuxiliaryConsoleOpen_internal(title,maxLines,mode,position,size,textColor,backgroundColor));
}
SIM_DLLEXPORT simInt simImportShape_D(simInt fileformat,const simChar* pathAndFilename,simInt options,simDouble identicalVerticeTolerance,simDouble scalingFactor)
{
    return(simImportShape_internal(fileformat,pathAndFilename,options,identicalVerticeTolerance,scalingFactor));
}
SIM_DLLEXPORT simInt simImportMesh_D(simInt fileformat,const simChar* pathAndFilename,simInt options,simDouble identicalVerticeTolerance,simDouble scalingFactor,simDouble*** vertices,simInt** verticesSizes,simInt*** indices,simInt** indicesSizes,simDouble*** reserved,simChar*** names)
{
    return(simImportMesh_internal(fileformat,pathAndFilename,options,identicalVerticeTolerance,scalingFactor,vertices,verticesSizes,indices,indicesSizes,reserved,names));
}
SIM_DLLEXPORT simInt simExportMesh_D(simInt fileformat,const simChar* pathAndFilename,simInt options,simDouble scalingFactor,simInt elementCount,const simDouble** vertices,const simInt* verticesSizes,const simInt** indices,const simInt* indicesSizes,simDouble** reserved,const simChar** names)
{
    return(simExportMesh_internal(fileformat,pathAndFilename,options,scalingFactor,elementCount,vertices,verticesSizes,indices,indicesSizes,reserved,names));
}
SIM_DLLEXPORT simInt simCreateMeshShape_D(simInt options,simDouble shadingAngle,const simDouble* vertices,simInt verticesSize,const simInt* indices,simInt indicesSize,simDouble* reserved)
{
    return(simCreateMeshShape_internal(options,shadingAngle,vertices,verticesSize,indices,indicesSize,reserved));
}
SIM_DLLEXPORT simInt simCreatePrimitiveShape_D(simInt primitiveType,const simDouble* sizes,simInt options)
{
    return(simCreatePrimitiveShape_internal(primitiveType,sizes,options));
}
SIM_DLLEXPORT simInt simCreateHeightfieldShape_D(simInt options,simDouble shadingAngle,simInt xPointCount,simInt yPointCount,simDouble xSize,const simDouble* heights)
{
    return(simCreateHeightfieldShape_internal(options,shadingAngle,xPointCount,yPointCount,xSize,heights));
}
SIM_DLLEXPORT simInt simGetShapeMesh_D(simInt shapeHandle,simDouble** vertices,simInt* verticesSize,simInt** indices,simInt* indicesSize,simDouble** normals)
{
    return(simGetShapeMesh_internal(shapeHandle,vertices,verticesSize,indices,indicesSize,normals));
}
SIM_DLLEXPORT simInt simCreateJoint_D(simInt jointType,simInt jointMode,simInt options,const simDouble* sizes,const simDouble* reservedA,const simDouble* reservedB)
{
    return(simCreateJoint_internal(jointType,jointMode,options,sizes,reservedA,reservedB));
}
SIM_DLLEXPORT simInt simGetObjectFloatParam_D(simInt objectHandle,simInt ParamID,simDouble* Param)
{
    return(simGetObjectFloatParam_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT simInt simSetObjectFloatParam_D(simInt objectHandle,simInt ParamID,simDouble Param)
{
    return(simSetObjectFloatParam_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT simDouble* simGetObjectFloatArrayParam_D(simInt objectHandle,simInt ParamID,simInt* size)
{
    return(simGetObjectFloatArrayParam_internal(objectHandle,ParamID,size));
}
SIM_DLLEXPORT simInt simSetObjectFloatArrayParam_D(simInt objectHandle,simInt ParamID,const simDouble* params,simInt size)
{
    return(simSetObjectFloatArrayParam_internal(objectHandle,ParamID,params,size));
}
SIM_DLLEXPORT simInt simGetRotationAxis_D(const simDouble* matrixStart,const simDouble* matrixGoal,simDouble* axis,simDouble* angle)
{
    return(simGetRotationAxis_internal(matrixStart,matrixGoal,axis,angle));
}
SIM_DLLEXPORT simInt simRotateAroundAxis_D(const simDouble* matrixIn,const simDouble* axis,const simDouble* axisPos,simDouble angle,simDouble* matrixOut)
{
    return(simRotateAroundAxis_internal(matrixIn,axis,axisPos,angle,matrixOut));
}
SIM_DLLEXPORT simInt simGetJointForce_D(simInt jointHandle,simDouble* forceOrTorque)
{
    return(simGetJointForce_internal(jointHandle,forceOrTorque));
}
SIM_DLLEXPORT simInt simGetJointTargetForce_D(simInt jointHandle,simDouble* forceOrTorque)
{
    return(simGetJointTargetForce_internal(jointHandle,forceOrTorque));
}
SIM_DLLEXPORT simInt simSetJointTargetForce_D(simInt objectHandle,simDouble forceOrTorque,simBool signedValue)
{
    return(simSetJointTargetForce_internal(objectHandle,forceOrTorque,signedValue));
}
SIM_DLLEXPORT simInt simCameraFitToView_D(simInt viewHandleOrIndex,simInt objectCount,const simInt* objectHandles,simInt options,simDouble scaling)
{
    return(simCameraFitToView_internal(viewHandleOrIndex,objectCount,objectHandles,options,scaling));
}
SIM_DLLEXPORT simInt simHandleVisionSensor_D(simInt visionSensorHandle,simDouble** auxValues,simInt** auxValuesCount)
{
    return(simHandleVisionSensor_internal(visionSensorHandle,auxValues,auxValuesCount));
}
SIM_DLLEXPORT simInt simReadVisionSensor_D(simInt visionSensorHandle,simDouble** auxValues,simInt** auxValuesCount)
{
    return(simReadVisionSensor_internal(visionSensorHandle,auxValues,auxValuesCount));
}
SIM_DLLEXPORT simInt simCheckVisionSensor_D(simInt visionSensorHandle,simInt entityHandle,simDouble** auxValues,simInt** auxValuesCount)
{
    return(simCheckVisionSensor_internal(visionSensorHandle,entityHandle,auxValues,auxValuesCount));
}
SIM_DLLEXPORT simDouble* simCheckVisionSensorEx_D(simInt visionSensorHandle,simInt entityHandle,simBool returnImage)
{
    return(simCheckVisionSensorEx_internal(visionSensorHandle,entityHandle,returnImage));
}
SIM_DLLEXPORT simUChar* simGetVisionSensorImg_D(simInt sensorHandle,simInt options,simDouble rgbaCutOff,const simInt* pos,const simInt* size,simInt* resolution)
{
    return(simGetVisionSensorImg_internal(sensorHandle,options,rgbaCutOff,pos,size,resolution));
}
SIM_DLLEXPORT simDouble* simGetVisionSensorDepth_D(simInt sensorHandle,simInt options,const simInt* pos,const simInt* size,simInt* resolution)
{
    return(simGetVisionSensorDepth_internal(sensorHandle,options,pos,size,resolution));
}
SIM_DLLEXPORT simInt simCreateDummy_D(simDouble size,const simDouble* reserved)
{
    return(simCreateDummy_internal(size,reserved));
}
SIM_DLLEXPORT simInt simCreateForceSensor_D(simInt options,const simInt* intParams,const simDouble* floatParams,const simDouble* reserved)
{
    return(simCreateForceSensor_internal(options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT simInt simCreateProximitySensor_D(simInt sensorType,simInt subType,simInt options,const simInt* intParams,const simDouble* floatParams,const simDouble* reserved)
{
    return(simCreateProximitySensor_internal(sensorType,subType,options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT simInt simCreateVisionSensor_D(simInt options,const simInt* intParams,const simDouble* floatParams,const simDouble* reserved)
{
    return(simCreateVisionSensor_internal(options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT simInt simConvexDecompose_D(simInt shapeHandle,simInt options,const simInt* intParams,const simDouble* floatParams)
{
    return(simConvexDecompose_internal(shapeHandle,options,intParams,floatParams));
}
SIM_DLLEXPORT simInt simCreateTexture_D(const simChar* fileName,simInt options,const simDouble* planeSizes,const simDouble* scalingUV,const simDouble* xy_g,simInt fixedResolution,simInt* textureId,simInt* resolution,const simVoid* reserved)
{
    return(simCreateTexture_internal(fileName,options,planeSizes,scalingUV,xy_g,fixedResolution,textureId,resolution,reserved));
}
SIM_DLLEXPORT simInt simWriteTexture_D(simInt textureId,simInt options,const simChar* data,simInt posX,simInt posY,simInt sizeX,simInt sizeY,simDouble interpol)
{
    return(simWriteTexture_internal(textureId,options,data,posX,posY,sizeX,sizeY,interpol));
}
SIM_DLLEXPORT simInt simGetShapeGeomInfo_D(simInt shapeHandle,simInt* intData,simDouble* floatData,simVoid* reserved)
{
    return(simGetShapeGeomInfo_internal(shapeHandle,intData,floatData,reserved));
}
SIM_DLLEXPORT simInt simScaleObject_D(simInt objectHandle,simDouble xScale,simDouble yScale,simDouble zScale,simInt options)
{
    return(simScaleObject_internal(objectHandle,xScale,yScale,zScale,options));
}
SIM_DLLEXPORT simInt simSetShapeTexture_D(simInt shapeHandle,simInt textureId,simInt mappingMode,simInt options,const simDouble* uvScaling,const simDouble* position,const simDouble* orientation)
{
    return(simSetShapeTexture_internal(shapeHandle,textureId,mappingMode,options,uvScaling,position,orientation));
}
SIM_DLLEXPORT simInt simTransformImage_D(simUChar* image,const simInt* resolution,simInt options,const simDouble* floatParams,const simInt* intParams,simVoid* reserved)
{
    return(simTransformImage_internal(image,resolution,options,floatParams,intParams,reserved));
}
SIM_DLLEXPORT simInt simGetQHull_D(const simDouble* inVertices,simInt inVerticesL,simDouble** verticesOut,simInt* verticesOutL,simInt** indicesOut,simInt* indicesOutL,simInt reserved1,const simDouble* reserved2)
{
    return(simGetQHull_internal(inVertices,inVerticesL,verticesOut,verticesOutL,indicesOut,indicesOutL,reserved1,reserved2));
}
SIM_DLLEXPORT simInt simGetDecimatedMesh_D(const simDouble* inVertices,simInt inVerticesL,const simInt* inIndices,simInt inIndicesL,simDouble** verticesOut,simInt* verticesOutL,simInt** indicesOut,simInt* indicesOutL,simDouble decimationPercent,simInt reserved1,const simDouble* reserved2)
{
    return(simGetDecimatedMesh_internal(inVertices,inVerticesL,inIndices,inIndicesL,verticesOut,verticesOutL,indicesOut,indicesOutL,decimationPercent,reserved1,reserved2));
}
SIM_DLLEXPORT simInt simComputeMassAndInertia_D(simInt shapeHandle,simDouble density)
{
    return(simComputeMassAndInertia_internal(shapeHandle,density));
}
SIM_DLLEXPORT simDouble simGetEngineFloatParam_D(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{
    return(simGetEngineFloatParam_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT simInt simSetEngineFloatParam_D(simInt paramId,simInt objectHandle,const simVoid* object,simDouble val)
{
    return(simSetEngineFloatParam_internal(paramId,objectHandle,object,val));
}
SIM_DLLEXPORT simInt simCreateOctree_D(simDouble voxelSize,simInt options,simDouble pointSize,simVoid* reserved)
{
    return(simCreateOctree_internal(voxelSize,options,pointSize,reserved));
}
SIM_DLLEXPORT simInt simCreatePointCloud_D(simDouble maxVoxelSize,simInt maxPtCntPerVoxel,simInt options,simDouble pointSize,simVoid* reserved)
{
    return(simCreatePointCloud_internal(maxVoxelSize,maxPtCntPerVoxel,options,pointSize,reserved));
}
SIM_DLLEXPORT simInt simSetPointCloudOptions_D(simInt pointCloudHandle,simDouble maxVoxelSize,simInt maxPtCntPerVoxel,simInt options,simDouble pointSize,simVoid* reserved)
{
    return(simSetPointCloudOptions_internal(pointCloudHandle,maxVoxelSize,maxPtCntPerVoxel,options,pointSize,reserved));
}
SIM_DLLEXPORT simInt simGetPointCloudOptions_D(simInt pointCloudHandle,simDouble* maxVoxelSize,simInt* maxPtCntPerVoxel,simInt* options,simDouble* pointSize,simVoid* reserved)
{
    return(simGetPointCloudOptions_internal(pointCloudHandle,maxVoxelSize,maxPtCntPerVoxel,options,pointSize,reserved));
}
SIM_DLLEXPORT simInt simInsertVoxelsIntoOctree_D(simInt octreeHandle,simInt options,const simDouble* pts,simInt ptCnt,const simUChar* color,const simUInt* tag,simVoid* reserved)
{
    return(simInsertVoxelsIntoOctree_internal(octreeHandle,options,pts,ptCnt,color,tag,reserved));
}
SIM_DLLEXPORT simInt simRemoveVoxelsFromOctree_D(simInt octreeHandle,simInt options,const simDouble* pts,simInt ptCnt,simVoid* reserved)
{
    return(simRemoveVoxelsFromOctree_internal(octreeHandle,options,pts,ptCnt,reserved));
}
SIM_DLLEXPORT simInt simInsertPointsIntoPointCloud_D(simInt pointCloudHandle,simInt options,const simDouble* pts,simInt ptCnt,const simUChar* color,simVoid* optionalValues)
{
    return(simInsertPointsIntoPointCloud_internal(pointCloudHandle,options,pts,ptCnt,color,optionalValues));
}
SIM_DLLEXPORT simInt simRemovePointsFromPointCloud_D(simInt pointCloudHandle,simInt options,const simDouble* pts,simInt ptCnt,simDouble tolerance,simVoid* reserved)
{
    return(simRemovePointsFromPointCloud_internal(pointCloudHandle,options,pts,ptCnt,tolerance,reserved));
}
SIM_DLLEXPORT simInt simIntersectPointsWithPointCloud_D(simInt pointCloudHandle,simInt options,const simDouble* pts,simInt ptCnt,simDouble tolerance,simVoid* reserved)
{
    return(simIntersectPointsWithPointCloud_internal(pointCloudHandle,options,pts,ptCnt,tolerance,reserved));
}
SIM_DLLEXPORT const simDouble* simGetOctreeVoxels_D(simInt octreeHandle,simInt* ptCnt,simVoid* reserved)
{
    return(simGetOctreeVoxels_internal(octreeHandle,ptCnt,reserved));
}
SIM_DLLEXPORT const simDouble* simGetPointCloudPoints_D(simInt pointCloudHandle,simInt* ptCnt,simVoid* reserved)
{
    return(simGetPointCloudPoints_internal(pointCloudHandle,ptCnt,reserved));
}
SIM_DLLEXPORT simInt simInsertObjectIntoPointCloud_D(simInt pointCloudHandle,simInt objectHandle,simInt options,simDouble gridSize,const simUChar* color,simVoid* optionalValues)
{
    return(simInsertObjectIntoPointCloud_internal(pointCloudHandle,objectHandle,options,gridSize,color,optionalValues));
}
SIM_DLLEXPORT simInt simSubtractObjectFromPointCloud_D(simInt pointCloudHandle,simInt objectHandle,simInt options,simDouble tolerance,simVoid* reserved)
{
    return(simSubtractObjectFromPointCloud_internal(pointCloudHandle,objectHandle,options,tolerance,reserved));
}
SIM_DLLEXPORT simInt simCheckOctreePointOccupancy_D(simInt octreeHandle,simInt options,const simDouble* points,simInt ptCnt,simUInt* tag,simUInt64* location,simVoid* reserved)
{
    return(simCheckOctreePointOccupancy_internal(octreeHandle,options,points,ptCnt,tag,location,reserved));
}
SIM_DLLEXPORT simInt simApplyTexture_D(simInt shapeHandle,const simDouble* textureCoordinates,simInt textCoordSize,const simUChar* texture,const simInt* textureResolution,simInt options)
{
    return(simApplyTexture_internal(shapeHandle,textureCoordinates,textCoordSize,texture,textureResolution,options));
}
SIM_DLLEXPORT simInt simSetJointDependency_D(simInt jointHandle,simInt masterJointHandle,simDouble offset,simDouble multCoeff)
{
    return(simSetJointDependency_internal(jointHandle,masterJointHandle,offset,multCoeff));
}
SIM_DLLEXPORT simInt simGetJointDependency_D(simInt jointHandle,simInt* masterJointHandle,simDouble* offset,simDouble* multCoeff)
{
    return(simGetJointDependency_internal(jointHandle,masterJointHandle,offset,multCoeff));
}
SIM_DLLEXPORT simInt simGetShapeMass_D(simInt shapeHandle,simDouble* mass)
{
    return(simGetShapeMass_internal(shapeHandle,mass));
}
SIM_DLLEXPORT simInt simSetShapeMass_D(simInt shapeHandle,simDouble mass)
{
    return(simSetShapeMass_internal(shapeHandle,mass));
}
SIM_DLLEXPORT simInt simGetShapeInertia_D(simInt shapeHandle,simDouble* inertiaMatrix,simDouble* transformationMatrix)
{
    return(simGetShapeInertia_internal(shapeHandle,inertiaMatrix,transformationMatrix));
}
SIM_DLLEXPORT simInt simSetShapeInertia_D(simInt shapeHandle,const simDouble* inertiaMatrix,const simDouble* transformationMatrix)
{
    return(simSetShapeInertia_internal(shapeHandle,inertiaMatrix,transformationMatrix));
}
SIM_DLLEXPORT simInt simGenerateShapeFromPath_D(const simDouble* path,simInt pathSize,const simDouble* section,simInt sectionSize,simInt options,const simDouble* upVector,simDouble reserved)
{
    return(simGenerateShapeFromPath_internal(path,pathSize,section,sectionSize,options,upVector,reserved));
}
SIM_DLLEXPORT simDouble simGetClosestPosOnPath_D(const simDouble* path,simInt pathSize,const simDouble* pathLengths,const simDouble* absPt)
{
    return(simGetClosestPosOnPath_internal(path,pathSize,pathLengths,absPt));
}
SIM_DLLEXPORT simInt simExtCallScriptFunction_D(simInt scriptHandleOrType, const simChar* functionNameAtScriptName,
                                               const simInt* inIntData, simInt inIntCnt,
                                               const simDouble* inFloatData, simInt inFloatCnt,
                                               const simChar** inStringData, simInt inStringCnt,
                                               const simChar* inBufferData, simInt inBufferCnt,
                                               simInt** outIntData, simInt* outIntCnt,
                                               simDouble** outFloatData, simInt* outFloatCnt,
                                               simChar*** outStringData, simInt* outStringCnt,
                                               simChar** outBufferData, simInt* outBufferSize)
{
    return(simExtCallScriptFunction_internal(scriptHandleOrType, functionNameAtScriptName,
                                             inIntData, inIntCnt, inFloatData, inFloatCnt,
                                             inStringData, inStringCnt, inBufferData, inBufferCnt,
                                             outIntData, outIntCnt, outFloatData, outFloatCnt,
                                             outStringData, outStringCnt, outBufferData, outBufferSize));
}
SIM_DLLEXPORT simVoid _simGetObjectLocalTransformation_D(const simVoid* object,simDouble* pos,simDouble* quat,simBool excludeFirstJointTransformation)
{
    return(_simGetObjectLocalTransformation_internal(object,pos,quat,excludeFirstJointTransformation));
}
SIM_DLLEXPORT simVoid _simSetObjectLocalTransformation_D(simVoid* object,const simDouble* pos,const simDouble* quat,simDouble simTime)
{
    return(_simSetObjectLocalTransformation_internal(object,pos,quat,simTime));
}
SIM_DLLEXPORT simVoid _simDynReportObjectCumulativeTransformation_D(simVoid* object,const simDouble* pos,const simDouble* quat,simDouble simTime)
{
    return(_simDynReportObjectCumulativeTransformation_internal(object,pos,quat,simTime));
}
SIM_DLLEXPORT simVoid _simSetObjectCumulativeTransformation_D(simVoid* object,const simDouble* pos,const simDouble* quat,simBool keepChildrenInPlace)
{
    return(_simSetObjectCumulativeTransformation_internal(object,pos,quat,keepChildrenInPlace));
}
SIM_DLLEXPORT simVoid _simGetObjectCumulativeTransformation_D(const simVoid* object,simDouble* pos,simDouble* quat,simBool excludeFirstJointTransformation)
{
    return(_simGetObjectCumulativeTransformation_internal(object,pos,quat,excludeFirstJointTransformation));
}
SIM_DLLEXPORT simVoid _simSetJointVelocity_D(const simVoid* joint,simDouble vel)
{
    return(_simSetJointVelocity_internal(joint,vel));
}
SIM_DLLEXPORT simVoid _simSetJointPosition_D(const simVoid* joint,simDouble pos)
{
    return(_simSetJointPosition_internal(joint,pos));
}
SIM_DLLEXPORT simDouble _simGetJointPosition_D(const simVoid* joint)
{
    return(_simGetJointPosition_internal(joint));
}
SIM_DLLEXPORT simVoid _simSetDynamicMotorPositionControlTargetPosition_D(const simVoid* joint,simDouble pos)
{
    return(_simSetDynamicMotorPositionControlTargetPosition_internal(joint,pos));
}
SIM_DLLEXPORT simVoid _simGetInitialDynamicVelocity_D(const simVoid* shape,simDouble* vel)
{
    return(_simGetInitialDynamicVelocity_internal(shape,vel));
}
SIM_DLLEXPORT simVoid _simSetInitialDynamicVelocity_D(simVoid* shape,const simDouble* vel)
{
    return(_simSetInitialDynamicVelocity_internal(shape,vel));
}
SIM_DLLEXPORT simVoid _simGetInitialDynamicAngVelocity_D(const simVoid* shape,simDouble* angularVel)
{
    return(_simGetInitialDynamicAngVelocity_internal(shape,angularVel));
}
SIM_DLLEXPORT simVoid _simSetInitialDynamicAngVelocity_D(simVoid* shape,const simDouble* angularVel)
{
    return(_simSetInitialDynamicAngVelocity_internal(shape,angularVel));
}
SIM_DLLEXPORT simVoid _simSetShapeDynamicVelocity_D(simVoid* shape,const simDouble* linear,const simDouble* angular,simDouble simTime)
{
    return(_simSetShapeDynamicVelocity_internal(shape,linear,angular,simTime));
}
SIM_DLLEXPORT simVoid _simGetAdditionalForceAndTorque_D(const simVoid* shape,simDouble* force,simDouble* torque)
{
    return(_simGetAdditionalForceAndTorque_internal(shape,force,torque));
}
SIM_DLLEXPORT simBool _simGetJointPositionInterval_D(const simVoid* joint,simDouble* minValue,simDouble* rangeValue)
{
    return(_simGetJointPositionInterval_internal(joint,minValue,rangeValue));
}
SIM_DLLEXPORT simDouble _simGetDynamicMotorTargetPosition_D(const simVoid* joint)
{
    return(_simGetDynamicMotorTargetPosition_internal(joint));
}
SIM_DLLEXPORT simDouble _simGetDynamicMotorTargetVelocity_D(const simVoid* joint)
{
    return(_simGetDynamicMotorTargetVelocity_internal(joint));
}
SIM_DLLEXPORT simDouble _simGetDynamicMotorMaxForce_D(const simVoid* joint)
{
    return(_simGetDynamicMotorMaxForce_internal(joint));
}
SIM_DLLEXPORT simDouble _simGetDynamicMotorUpperLimitVelocity_D(const simVoid* joint)
{
    return(_simGetDynamicMotorUpperLimitVelocity_internal(joint));
}
SIM_DLLEXPORT simVoid _simSetDynamicMotorReflectedPositionFromDynamicEngine_D(simVoid* joint,simDouble pos,simDouble simTime)
{
    return(_simSetDynamicMotorReflectedPositionFromDynamicEngine_internal(joint,pos,simTime));
}
SIM_DLLEXPORT simVoid _simSetJointSphericalTransformation_D(simVoid* joint,const simDouble* quat,simDouble simTime)
{
    return(_simSetJointSphericalTransformation_internal(joint,quat,simTime));
}
SIM_DLLEXPORT simVoid _simAddForceSensorCumulativeForcesAndTorques_D(simVoid* forceSensor,const simDouble* force,const simDouble* torque,int totalPassesCount,simDouble simTime)
{
    return(_simAddForceSensorCumulativeForcesAndTorques_internal(forceSensor,force,torque,totalPassesCount,simTime));
}
SIM_DLLEXPORT simVoid _simAddJointCumulativeForcesOrTorques_D(simVoid* joint,simDouble forceOrTorque,int totalPassesCount,simDouble simTime)
{
    return(_simAddJointCumulativeForcesOrTorques_internal(joint,forceOrTorque,totalPassesCount,simTime));
}
SIM_DLLEXPORT simDouble _simGetLocalInertiaInfo_D(const simVoid* object,simDouble* pos,simDouble* quat,simDouble* diagI)
{
    return(_simGetLocalInertiaInfo_internal(object,pos,quat,diagI));
}
SIM_DLLEXPORT simDouble _simGetMass_D(const simVoid* geomInfo)
{
    return(_simGetMass_internal(geomInfo));
}
SIM_DLLEXPORT simVoid _simGetPurePrimitiveSizes_D(const simVoid* geometric,simDouble* sizes)
{
    return(_simGetPurePrimitiveSizes_internal(geometric,sizes));
}
SIM_DLLEXPORT simVoid _simGetVerticesLocalFrame_D(const simVoid* geometric,simDouble* pos,simDouble* quat)
{
    return(_simGetVerticesLocalFrame_internal(geometric,pos,quat));
}
SIM_DLLEXPORT const simDouble* _simGetHeightfieldData_D(const simVoid* geometric,simInt* xCount,simInt* yCount,simDouble* minHeight,simDouble* maxHeight)
{
    return(_simGetHeightfieldData_internal(geometric,xCount,yCount,minHeight,maxHeight));
}
SIM_DLLEXPORT simVoid _simGetCumulativeMeshes_D(const simVoid* geomInfo,simDouble** vertices,simInt* verticesSize,simInt** indices,simInt* indicesSize)
{
    return(_simGetCumulativeMeshes_internal(geomInfo,vertices,verticesSize,indices,indicesSize));
}
SIM_DLLEXPORT simVoid _simGetGravity_D(simDouble* gravity)
{
    return(_simGetGravity_internal(gravity));
}
SIM_DLLEXPORT simBool _simGetDistanceBetweenEntitiesIfSmaller_D(simInt entity1ID,simInt entity2ID,simDouble* distance,simDouble* ray,simInt* cacheBuffer,simBool overrideMeasurableFlagIfNonCollection1,simBool overrideMeasurableFlagIfNonCollection2,simBool pathPlanningRoutineCalling)
{
    return(_simGetDistanceBetweenEntitiesIfSmaller_internal(entity1ID,entity2ID,distance,ray,cacheBuffer,overrideMeasurableFlagIfNonCollection1,overrideMeasurableFlagIfNonCollection2,pathPlanningRoutineCalling));
}
SIM_DLLEXPORT simInt _simHandleJointControl_D(const simVoid* joint,simInt auxV,const simInt* inputValuesInt,const simDouble* inputValuesFloat,simDouble* outputValues)
{
    return(_simHandleJointControl_internal(joint,auxV,inputValuesInt,inputValuesFloat,outputValues));
}
SIM_DLLEXPORT simInt _simHandleCustomContact_D(simInt objHandle1,simInt objHandle2,simInt engine,simInt* dataInt,simDouble* dataFloat)
{
    return(_simHandleCustomContact_internal(objHandle1,objHandle2,engine,dataInt,dataFloat));
}
SIM_DLLEXPORT simDouble _simGetPureHollowScaling_D(const simVoid* geometric)
{
    return(_simGetPureHollowScaling_internal(geometric));
}
SIM_DLLEXPORT simVoid _simDynCallback_D(const simInt* intData,const simDouble* floatData)
{
    _simDynCallback_internal(intData,floatData);
}
#endif

#include "sim-old.cpp"
