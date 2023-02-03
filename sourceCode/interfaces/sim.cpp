#include <sim.h>
#include <simInternal.h>
#include <app.h>

SIM_DLLEXPORT int simRunSimulator(const char* applicationName,int options,void(*initCallBack)(),void(*loopCallBack)(),void(*deinitCallBack)())
{
    return(simRunSimulator_internal(applicationName,options,initCallBack,loopCallBack,deinitCallBack,0,"",true));
}
SIM_DLLEXPORT int simRunSimulatorEx(const char* applicationName,int options,void(*initCallBack)(),void(*loopCallBack)(),void(*deinitCallBack)(),int stopDelay,const char* sceneOrModelToLoad)
{
    return(simRunSimulator_internal(applicationName,options,initCallBack,loopCallBack,deinitCallBack,stopDelay,sceneOrModelToLoad,true));
}
SIM_DLLEXPORT void* simGetMainWindow(int type)
{
    return(simGetMainWindow_internal(type));
}
SIM_DLLEXPORT char* simGetLastError()
{
    return(simGetLastError_internal());
}
SIM_DLLEXPORT int simSetBoolParam(int parameter,bool boolState)
{
    return(simSetBoolParam_internal(parameter,boolState));
}
SIM_DLLEXPORT int simGetBoolParam(int parameter)
{
    return(simGetBoolParam_internal(parameter));
}
SIM_DLLEXPORT int simSetInt32Param(int parameter,int intState)
{
    return(simSetInt32Param_internal(parameter,intState));
}
SIM_DLLEXPORT int simGetInt32Param(int parameter,int* intState)
{
    return(simGetInt32Param_internal(parameter,intState));
}
SIM_DLLEXPORT int simGetUInt64Param(int parameter,unsigned long long int* intState)
{
    return(simGetUInt64Param_internal(parameter,intState));
}
SIM_DLLEXPORT int simSetStringParam(int parameter,const char* str)
{
    return(simSetStringParam_internal(parameter,str));
}
SIM_DLLEXPORT char* simGetStringParam(int parameter)
{
    return(simGetStringParam_internal(parameter));
}
SIM_DLLEXPORT int simSetNamedStringParam(const char* paramName,const char* stringParam,int paramLength)
{
    return(simSetNamedStringParam_internal(paramName,stringParam,paramLength));
}
SIM_DLLEXPORT char* simGetNamedStringParam(const char* paramName,int* paramLength)
{
    return(simGetNamedStringParam_internal(paramName,paramLength));
}
SIM_DLLEXPORT int simGetObject(const char* objectPath,int index,int proxy,int options)
{
    return(simGetObject_internal(objectPath,index,proxy,options));
}
SIM_DLLEXPORT long long int simGetObjectUid(int objectHandle)
{
    return(simGetObjectUid_internal(objectHandle));
}
SIM_DLLEXPORT int simGetObjectFromUid(long long int uid,int options)
{
    return(simGetObjectFromUid_internal(uid,options));
}
SIM_DLLEXPORT int simGetScriptHandleEx(int scriptType,int objectHandle,const char* scriptName)
{
    return(simGetScriptHandleEx_internal(scriptType,objectHandle,scriptName));
}
SIM_DLLEXPORT int simRemoveObjects(const int* objectHandles,int count)
{
    return(simRemoveObjects_internal(objectHandles,count));
}
SIM_DLLEXPORT int simRemoveModel(int objectHandle)
{
    return(simRemoveModel_internal(objectHandle));
}
SIM_DLLEXPORT char* simGetObjectAlias(int objectHandle,int options)
{
    return(simGetObjectAlias_internal(objectHandle,options));
}
SIM_DLLEXPORT int simSetObjectAlias(int objectHandle,const char* objectAlias,int options)
{
    return(simSetObjectAlias_internal(objectHandle,objectAlias,options));
}
SIM_DLLEXPORT int simGetObjectParent(int objectHandle)
{
    return(simGetObjectParent_internal(objectHandle));
}
SIM_DLLEXPORT int simGetObjectChild(int objectHandle,int index)
{
    return(simGetObjectChild_internal(objectHandle,index));
}
SIM_DLLEXPORT int simSetObjectParent(int objectHandle,int parentObjectHandle,bool keepInPlace)
{
    return(simSetObjectParent_internal(objectHandle,parentObjectHandle,keepInPlace));
}
SIM_DLLEXPORT int simGetObjectType(int objectHandle)
{
    return(simGetObjectType_internal(objectHandle));
}
SIM_DLLEXPORT int simGetJointType(int objectHandle)
{
    return(simGetJointType_internal(objectHandle));
}
SIM_DLLEXPORT int simReservedCommand(int v,int w)
{
    return(simReservedCommand_internal(v,w));
}
SIM_DLLEXPORT int simGetSimulationState()
{
    return(simGetSimulationState_internal());
}
SIM_DLLEXPORT int simLoadScene(const char* filename)
{
    return(simLoadScene_internal(filename));
}
SIM_DLLEXPORT int simCloseScene()
{
    return(simCloseScene_internal());
}
SIM_DLLEXPORT int simSaveScene(const char* filename)
{
    return(simSaveScene_internal(filename));
}
SIM_DLLEXPORT int simLoadModel(const char* filename)
{
    return(simLoadModel_internal(filename));
}
SIM_DLLEXPORT int simSaveModel(int baseOfModelHandle,const char* filename)
{
    return(simSaveModel_internal(baseOfModelHandle,filename));
}
SIM_DLLEXPORT char* simGetSimulatorMessage(int* messageID,int* auxiliaryData,int* returnedDataSize)
{
    return(simGetSimulatorMessage_internal(messageID,auxiliaryData,returnedDataSize));
}
SIM_DLLEXPORT int simDoesFileExist(const char* filename)
{
    return(simDoesFileExist_internal(filename));
}
SIM_DLLEXPORT int* simGetObjectSel(int* cnt)
{
    return(simGetObjectSel_internal(cnt));
}
SIM_DLLEXPORT int simSetObjectSel(const int* handles,int cnt)
{
    return(simSetObjectSel_internal(handles,cnt));
}
SIM_DLLEXPORT int simAssociateScriptWithObject(int scriptHandle,int associatedObjectHandle)
{
    return(simAssociateScriptWithObject_internal(scriptHandle,associatedObjectHandle));
}
SIM_DLLEXPORT int simHandleMainScript()
{
    return(simHandleMainScript_internal());
}
SIM_DLLEXPORT int simResetScript(int scriptHandle)
{
    return(simResetScript_internal(scriptHandle));
}
SIM_DLLEXPORT int simAddScript(int scriptProperty)
{
    return(simAddScript_internal(scriptProperty));
}
SIM_DLLEXPORT int simRemoveScript(int scriptHandle)
{
    return(simRemoveScript_internal(scriptHandle));
}
SIM_DLLEXPORT int simRefreshDialogs(int refreshDegree)
{
    return(simRefreshDialogs_internal(refreshDegree));
}
SIM_DLLEXPORT int simResetProximitySensor(int sensorHandle)
{
    return(simResetProximitySensor_internal(sensorHandle));
}
SIM_DLLEXPORT void* simCreateBuffer(int size)
{
    return(simCreateBuffer_internal(size));
}
SIM_DLLEXPORT int simReleaseBuffer(const void* buffer)
{
    return(simReleaseBuffer_internal(buffer));
}
SIM_DLLEXPORT int simCheckCollision(int entity1Handle,int entity2Handle)
{
    return(simCheckCollision_internal(entity1Handle,entity2Handle));
}
SIM_DLLEXPORT int simGetRealTimeSimulation()
{
    return(simGetRealTimeSimulation_internal());
}
SIM_DLLEXPORT int simIsRealTimeSimulationStepNeeded()
{
    return(simIsRealTimeSimulationStepNeeded_internal());
}
SIM_DLLEXPORT int simGetSimulationPassesPerRenderingPass()
{
    return(simGetSimulationPassesPerRenderingPass_internal());
}
SIM_DLLEXPORT int simAdvanceSimulationByOneStep()
{
    return(simAdvanceSimulationByOneStep_internal());
}
SIM_DLLEXPORT int simStartSimulation()
{
    return(simStartSimulation_internal());
}
SIM_DLLEXPORT int simStopSimulation()
{
    return(simStopSimulation_internal());
}
SIM_DLLEXPORT int simPauseSimulation()
{
    return(simPauseSimulation_internal());
}
SIM_DLLEXPORT int simLoadModule(const char* filenameAndPath,const char* pluginName)
{
    return(simLoadModule_internal(filenameAndPath,pluginName));
}
SIM_DLLEXPORT int simUnloadModule(int pluginhandle)
{
    return(simUnloadModule_internal(pluginhandle));
}
SIM_DLLEXPORT char* simGetModuleName(int index,unsigned char* moduleVersion)
{
    return(simGetModuleName_internal(index,moduleVersion));
}
SIM_DLLEXPORT int simAdjustView(int viewHandleOrIndex,int associatedViewableObjectHandle,int options,const char* viewLabel)
{
    return(simAdjustView_internal(viewHandleOrIndex,associatedViewableObjectHandle,options,viewLabel));
}
SIM_DLLEXPORT int simSetLastError(const char* funcName,const char* errorMessage)
{
    return(simSetLastError_internal(funcName,errorMessage));
}
SIM_DLLEXPORT int simResetGraph(int graphHandle)
{
    return(simResetGraph_internal(graphHandle));
}
SIM_DLLEXPORT int simDestroyGraphCurve(int graphHandle,int curveId)
{
    return(simDestroyGraphCurve_internal(graphHandle,curveId));
}
SIM_DLLEXPORT int simDuplicateGraphCurveToStatic(int graphHandle,int curveId,const char* curveName)
{
    return(simDuplicateGraphCurveToStatic_internal(graphHandle,curveId,curveName));
}
SIM_DLLEXPORT int simSetNavigationMode(int navigationMode)
{
    return(simSetNavigationMode_internal(navigationMode));
}
SIM_DLLEXPORT int simGetNavigationMode()
{
    return(simGetNavigationMode_internal());
}
SIM_DLLEXPORT int simSetPage(int index)
{
    return(simSetPage_internal(index));
}
SIM_DLLEXPORT int simGetPage()
{
    return(simGetPage_internal());
}
SIM_DLLEXPORT int simRegisterScriptCallbackFunction(const char* funcNameAtPluginName,const char* callTips,void(*callBack)(struct SScriptCallBack* cb))
{
    return(simRegisterScriptCallbackFunction_internal(funcNameAtPluginName,callTips,callBack));
}
SIM_DLLEXPORT int simRegisterScriptVariable(const char* varNameAtPluginName,const char* varValue,int stackHandle)
{
    return(simRegisterScriptVariable_internal(varNameAtPluginName,varValue,stackHandle));
}
SIM_DLLEXPORT int simRegisterScriptFuncHook(int scriptHandle,const char* funcToHook,const char* userFunction,bool executeBefore,int options)
{
    return(simRegisterScriptFuncHook_internal(scriptHandle,funcToHook,userFunction,executeBefore,options));
}
SIM_DLLEXPORT int simCopyPasteObjects(int* objectHandles,int objectCount,int options)
{
    return(simCopyPasteObjects_internal(objectHandles,objectCount,options));
}
SIM_DLLEXPORT char* simReceiveData(int dataHeader,const char* dataName,int antennaHandle,int index,int* dataLength,int* senderID,int* dataHeaderR,char** dataNameR)
{
    return(simReceiveData_internal(dataHeader,dataName,antennaHandle,index,dataLength,senderID,dataHeaderR,dataNameR));
}
SIM_DLLEXPORT int simRemoveDrawingObject(int objectHandle)
{
    return(simRemoveDrawingObject_internal(objectHandle));
}
SIM_DLLEXPORT int simAnnounceSceneContentChange()
{
    return(simAnnounceSceneContentChange_internal());
}
SIM_DLLEXPORT int simSetInt32Signal(const char* signalName,int signalValue)
{
    return(simSetInt32Signal_internal(signalName,signalValue));
}
SIM_DLLEXPORT int simGetInt32Signal(const char* signalName,int* signalValue)
{
    return(simGetInt32Signal_internal(signalName,signalValue));
}
SIM_DLLEXPORT int simClearInt32Signal(const char* signalName)
{
    return(simClearInt32Signal_internal(signalName));
}
SIM_DLLEXPORT int simClearFloatSignal(const char* signalName)
{
    return(simClearFloatSignal_internal(signalName));
}
SIM_DLLEXPORT int simSetStringSignal(const char* signalName,const char* signalValue,int stringLength)
{
    return(simSetStringSignal_internal(signalName,signalValue,stringLength));
}
SIM_DLLEXPORT char* simGetStringSignal(const char* signalName,int* stringLength)
{
    return(simGetStringSignal_internal(signalName,stringLength));
}
SIM_DLLEXPORT int simClearStringSignal(const char* signalName)
{
    return(simClearStringSignal_internal(signalName));
}
SIM_DLLEXPORT char* simGetSignalName(int signalIndex,int signalType)
{
    return(simGetSignalName_internal(signalIndex,signalType));
}
SIM_DLLEXPORT int simSetObjectProperty(int objectHandle,int prop)
{
    return(simSetObjectProperty_internal(objectHandle,prop));
}
SIM_DLLEXPORT int simGetObjectProperty(int objectHandle)
{
    return(simGetObjectProperty_internal(objectHandle));
}
SIM_DLLEXPORT int simSetObjectSpecialProperty(int objectHandle,int prop)
{
    return(simSetObjectSpecialProperty_internal(objectHandle,prop));
}
SIM_DLLEXPORT int simGetObjectSpecialProperty(int objectHandle)
{
    return(simGetObjectSpecialProperty_internal(objectHandle));
}
SIM_DLLEXPORT int simSetExplicitHandling(int objectHandle,int explicitFlags)
{
    return(simSetExplicitHandling_internal(objectHandle,explicitFlags));
}
SIM_DLLEXPORT int simGetExplicitHandling(int objectHandle)
{
    return(simGetExplicitHandling_internal(objectHandle));
}
SIM_DLLEXPORT int simGetLinkDummy(int dummyHandle)
{
    return(simGetLinkDummy_internal(dummyHandle));
}
SIM_DLLEXPORT int simSetLinkDummy(int dummyHandle,int linkedDummyHandle)
{
    return(simSetLinkDummy_internal(dummyHandle,linkedDummyHandle));
}
SIM_DLLEXPORT int simSetModelProperty(int objectHandle,int modelProperty)
{
    return(simSetModelProperty_internal(objectHandle,modelProperty));
}
SIM_DLLEXPORT int simGetModelProperty(int objectHandle)
{
    return(simGetModelProperty_internal(objectHandle));
}
SIM_DLLEXPORT int simResetDynamicObject(int objectHandle)
{
    return(simResetDynamicObject_internal(objectHandle));
}
SIM_DLLEXPORT int simSetJointMode(int jointHandle,int jointMode,int options)
{
    return(simSetJointMode_internal(jointHandle,jointMode,options));
}
SIM_DLLEXPORT int simGetJointMode(int jointHandle,int* options)
{
    return(simGetJointMode_internal(jointHandle,options));
}
SIM_DLLEXPORT int simSerialOpen(const char* portString,int baudRate,void* reserved1,void* reserved2)
{
    return(simSerialOpen_internal(portString,baudRate,reserved1,reserved2));
}
SIM_DLLEXPORT int simSerialClose(int portHandle)
{
    return(simSerialClose_internal(portHandle));
}
SIM_DLLEXPORT int simSerialSend(int portHandle,const char* data,int dataLength)
{
    return(simSerialSend_internal(portHandle,data,dataLength));
}
SIM_DLLEXPORT int simSerialRead(int portHandle,char* buffer,int dataLengthToRead)
{
    return(simSerialRead_internal(portHandle,buffer,dataLengthToRead));
}
SIM_DLLEXPORT int simSerialCheck(int portHandle)
{
    return(simSerialCheck_internal(portHandle));
}
SIM_DLLEXPORT int simAuxiliaryConsoleClose(int consoleHandle)
{
    return(simAuxiliaryConsoleClose_internal(consoleHandle));
}
SIM_DLLEXPORT int simAuxiliaryConsoleShow(int consoleHandle,bool showState)
{
    return(simAuxiliaryConsoleShow_internal(consoleHandle,showState));
}
SIM_DLLEXPORT int simAuxiliaryConsolePrint(int consoleHandle,const char* text)
{
    return(simAuxiliaryConsolePrint_internal(consoleHandle,text));
}
SIM_DLLEXPORT int simGetObjectInt32Param(int objectHandle,int ParamID,int* Param)
{
    return(simGetObjectInt32Param_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT int simSetObjectInt32Param(int objectHandle,int ParamID,int Param)
{
    return(simSetObjectInt32Param_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT char* simGetObjectStringParam(int objectHandle,int ParamID,int* ParamLength)
{
    return(simGetObjectStringParam_internal(objectHandle,ParamID,ParamLength));
}
SIM_DLLEXPORT int simSetObjectStringParam(int objectHandle,int ParamID,const char* Param,int ParamLength)
{
    return(simSetObjectStringParam_internal(objectHandle,ParamID,Param,ParamLength));
}
SIM_DLLEXPORT int simGetScriptInt32Param(int ScriptHandle,int ParamID,int* Param)
{
    return(simGetScriptInt32Param_internal(ScriptHandle,ParamID,Param));
}
SIM_DLLEXPORT int simSetScriptInt32Param(int ScriptHandle,int ParamID,int Param)
{
    return(simSetScriptInt32Param_internal(ScriptHandle,ParamID,Param));
}
SIM_DLLEXPORT char* simGetScriptStringParam(int ScriptHandle,int ParamID,int* ParamLength)
{
    return(simGetScriptStringParam_internal(ScriptHandle,ParamID,ParamLength));
}
SIM_DLLEXPORT int simSetScriptStringParam(int ScriptHandle,int ParamID,const char* Param,int ParamLength)
{
    return(simSetScriptStringParam_internal(ScriptHandle,ParamID,Param,ParamLength));
}
SIM_DLLEXPORT int simSetSimulationPassesPerRenderingPass(int p)
{
    return(simSetSimulationPassesPerRenderingPass_internal(p));
}
SIM_DLLEXPORT int simPersistentDataWrite(const char* dataName,const char* dataValue,int dataLength,int options)
{
    return(simPersistentDataWrite_internal(dataName,dataValue,dataLength,options));
}
SIM_DLLEXPORT char* simPersistentDataRead(const char* dataName,int* dataLength)
{
    return(simPersistentDataRead_internal(dataName,dataLength));
}
SIM_DLLEXPORT int simIsHandle(int generalObjectHandle,int generalObjectType)
{
    return(simIsHandle_internal(generalObjectHandle,generalObjectType));
}
SIM_DLLEXPORT int simResetVisionSensor(int visionSensorHandle)
{
    return(simResetVisionSensor_internal(visionSensorHandle));
}
SIM_DLLEXPORT int simSetVisionSensorImg(int sensorHandle,const unsigned char* img,int options,const int* pos,const int* size)
{
    return(simSetVisionSensorImg_internal(sensorHandle,img,options,pos,size));
}
SIM_DLLEXPORT int simRuckigPos(int dofs,double baseCycleTime,int flags,const double* currentPos,const double* currentVel,const double* currentAccel,const double* maxVel,const double* maxAccel,const double* maxJerk,const bool* selection,const double* targetPos,const double* targetVel,double* reserved1,int* reserved2)
{
    return(simRuckigPos_internal(dofs,baseCycleTime,flags,currentPos,currentVel,currentAccel,maxVel,maxAccel,maxJerk,selection,targetPos,targetVel,reserved1,reserved2));
}
SIM_DLLEXPORT int simRuckigVel(int dofs,double baseCycleTime,int flags,const double* currentPos,const double* currentVel,const double* currentAccel,const double* maxAccel,const double* maxJerk,const bool* selection,const double* targetVel,double* reserved1,int* reserved2)
{
    return(simRuckigVel_internal(dofs,baseCycleTime,flags,currentPos,currentVel,currentAccel,maxAccel,maxJerk,selection,targetVel,reserved1,reserved2));
}
SIM_DLLEXPORT int simRuckigStep(int objHandle,double cycleTime,double* newPos,double* newVel,double* newAccel,double* syncTime,double* reserved1,int* reserved2)
{
    return(simRuckigStep_internal(objHandle,cycleTime,newPos,newVel,newAccel,syncTime,reserved1,reserved2));
}
SIM_DLLEXPORT int simRuckigRemove(int objHandle)
{
    return(simRuckigRemove_internal(objHandle));
}
SIM_DLLEXPORT int simGroupShapes(const int* shapeHandles,int shapeCount)
{
    return(simGroupShapes_internal(shapeHandles,shapeCount));
}
SIM_DLLEXPORT int* simUngroupShape(int shapeHandle,int* shapeCount)
{
    return(simUngroupShape_internal(shapeHandle,shapeCount));
}
SIM_DLLEXPORT void simQuitSimulator(bool ignoredArgument)
{
    simQuitSimulator_internal(ignoredArgument);
}
SIM_DLLEXPORT int simSetShapeMaterial(int shapeHandle,int materialIdOrShapeHandle)
{
    return(simSetShapeMaterial_internal(shapeHandle,materialIdOrShapeHandle));
}
SIM_DLLEXPORT int simGetTextureId(const char* textureName,int* resolution)
{
    return(simGetTextureId_internal(textureName,resolution));
}
SIM_DLLEXPORT unsigned char* simReadTexture(int textureId,int options,int posX,int posY,int sizeX,int sizeY)
{
    return(simReadTexture_internal(textureId,options,posX,posY,sizeX,sizeY));
}
SIM_DLLEXPORT int simWriteCustomDataBlock(int objectHandle,const char* tagName,const char* data,int dataSize)
{
    return(simWriteCustomDataBlock_internal(objectHandle,tagName,data,dataSize));
}
SIM_DLLEXPORT char* simReadCustomDataBlock(int objectHandle,const char* tagName,int* dataSize)
{
    return(simReadCustomDataBlock_internal(objectHandle,tagName,dataSize));
}
SIM_DLLEXPORT char* simReadCustomDataBlockTags(int objectHandle,int* tagCount)
{
    return(simReadCustomDataBlockTags_internal(objectHandle,tagCount));
}
SIM_DLLEXPORT int simGetObjects(int index,int objectType)
{
    return(simGetObjects_internal(index,objectType));
}
SIM_DLLEXPORT int* simGetObjectsInTree(int treeBaseHandle,int objectType,int options,int* objectCount)
{
    return(simGetObjectsInTree_internal(treeBaseHandle,objectType,options,objectCount));
}
SIM_DLLEXPORT int simGetShapeTextureId(int shapeHandle)
{
    return(simGetShapeTextureId_internal(shapeHandle));
}
SIM_DLLEXPORT int simCreateCollectionEx(int options)
{
    return(simCreateCollectionEx_internal(options));
}
SIM_DLLEXPORT int simAddItemToCollection(int collectionHandle,int what,int objectHandle,int options)
{
    return(simAddItemToCollection_internal(collectionHandle,what,objectHandle,options));
}
SIM_DLLEXPORT int simDestroyCollection(int collectionHandle)
{
    return(simDestroyCollection_internal(collectionHandle));
}
SIM_DLLEXPORT int* simGetCollectionObjects(int collectionHandle,int* objectCount)
{
    return(simGetCollectionObjects_internal(collectionHandle,objectCount));
}
SIM_DLLEXPORT int simReorientShapeBoundingBox(int shapeHandle,int relativeToHandle,int reservedSetToZero)
{
    return(simReorientShapeBoundingBox_internal(shapeHandle,relativeToHandle,reservedSetToZero));
}
SIM_DLLEXPORT int simSaveImage(const unsigned char* image,const int* resolution,int options,const char* filename,int quality,void* reserved)
{
    return(simSaveImage_internal(image,resolution,options,filename,quality,reserved));
}
SIM_DLLEXPORT unsigned char* simLoadImage(int* resolution,int options,const char* filename,void* reserved)
{
    return(simLoadImage_internal(resolution,options,filename,reserved));
}
SIM_DLLEXPORT unsigned char* simGetScaledImage(const unsigned char* imageIn,const int* resolutionIn,int* resolutionOut,int options,void* reserved)
{
    return(simGetScaledImage_internal(imageIn,resolutionIn,resolutionOut,options,reserved));
}
SIM_DLLEXPORT int simCallScriptFunctionEx(int scriptHandleOrType,const char* functionNameAtScriptName,int stackId)
{
    return(simCallScriptFunctionEx_internal(scriptHandleOrType,functionNameAtScriptName,stackId));
}
SIM_DLLEXPORT char* simGetExtensionString(int objectHandle,int index,const char* key)
{
    return(simGetExtensionString_internal(objectHandle,index,key));
}
SIM_DLLEXPORT int simCreateStack()
{
    return(simCreateStack_internal());
}
SIM_DLLEXPORT int simReleaseStack(int stackHandle)
{
    return(simReleaseStack_internal(stackHandle));
}
SIM_DLLEXPORT int simCopyStack(int stackHandle)
{
    return(simCopyStack_internal(stackHandle));
}
SIM_DLLEXPORT int simPushNullOntoStack(int stackHandle)
{
    return(simPushNullOntoStack_internal(stackHandle));
}
SIM_DLLEXPORT int simPushBoolOntoStack(int stackHandle,bool value)
{
    return(simPushBoolOntoStack_internal(stackHandle,value));
}
SIM_DLLEXPORT int simPushInt32OntoStack(int stackHandle,int value)
{
    return(simPushInt32OntoStack_internal(stackHandle,value));
}
SIM_DLLEXPORT int simPushInt64OntoStack(int stackHandle,long long int value)
{
    return(simPushInt64OntoStack_internal(stackHandle,value));
}
SIM_DLLEXPORT int simPushFloatOntoStack(int stackHandle,float value)
{
    return(simPushFloatOntoStack_internal(stackHandle,value));
}
SIM_DLLEXPORT int simPushDoubleOntoStack(int stackHandle,double value)
{
    return(simPushDoubleOntoStack_internal(stackHandle,value));
}
SIM_DLLEXPORT int simPushStringOntoStack(int stackHandle,const char* value,int stringSize)
{
    return(simPushStringOntoStack_internal(stackHandle,value,stringSize));
}
SIM_DLLEXPORT int simPushUInt8TableOntoStack(int stackHandle,const unsigned char* values,int valueCnt)
{
    return(simPushUInt8TableOntoStack_internal(stackHandle,values,valueCnt));
}
SIM_DLLEXPORT int simPushInt32TableOntoStack(int stackHandle,const int* values,int valueCnt)
{
    return(simPushInt32TableOntoStack_internal(stackHandle,values,valueCnt));
}
SIM_DLLEXPORT int simPushInt64TableOntoStack(int stackHandle,const long long int* values,int valueCnt)
{
    return(simPushInt64TableOntoStack_internal(stackHandle,values,valueCnt));
}
SIM_DLLEXPORT int simPushFloatTableOntoStack(int stackHandle,const float* values,int valueCnt)
{
    return(simPushFloatTableOntoStack_internal(stackHandle,values,valueCnt));
}
SIM_DLLEXPORT int simPushDoubleTableOntoStack(int stackHandle,const double* values,int valueCnt)
{
    return(simPushDoubleTableOntoStack_internal(stackHandle,values,valueCnt));
}
SIM_DLLEXPORT int simPushTableOntoStack(int stackHandle)
{
    return(simPushTableOntoStack_internal(stackHandle));
}
SIM_DLLEXPORT int simInsertDataIntoStackTable(int stackHandle)
{
    return(simInsertDataIntoStackTable_internal(stackHandle));
}
SIM_DLLEXPORT int simGetStackSize(int stackHandle)
{
    return(simGetStackSize_internal(stackHandle));
}
SIM_DLLEXPORT int simPopStackItem(int stackHandle,int count)
{
    return(simPopStackItem_internal(stackHandle,count));
}
SIM_DLLEXPORT int simMoveStackItemToTop(int stackHandle,int cIndex)
{
    return(simMoveStackItemToTop_internal(stackHandle,cIndex));
}
SIM_DLLEXPORT int simIsStackValueNull(int stackHandle)
{
    return(simIsStackValueNull_internal(stackHandle));
}
SIM_DLLEXPORT int simGetStackBoolValue(int stackHandle,bool* boolValue)
{
    return(simGetStackBoolValue_internal(stackHandle,boolValue));
}
SIM_DLLEXPORT int simGetStackInt32Value(int stackHandle,int* numberValue)
{
    return(simGetStackInt32Value_internal(stackHandle,numberValue));
}
SIM_DLLEXPORT int simGetStackInt64Value(int stackHandle,long long int* numberValue)
{
    return(simGetStackInt64Value_internal(stackHandle,numberValue));
}
SIM_DLLEXPORT int simGetStackFloatValue(int stackHandle,float* numberValue)
{
    return(simGetStackFloatValue_internal(stackHandle,numberValue));
}
SIM_DLLEXPORT int simGetStackDoubleValue(int stackHandle,double* numberValue)
{
    return(simGetStackDoubleValue_internal(stackHandle,numberValue));
}
SIM_DLLEXPORT char* simGetStackStringValue(int stackHandle,int* stringSize)
{
    return(simGetStackStringValue_internal(stackHandle,stringSize));
}
SIM_DLLEXPORT int simGetStackTableInfo(int stackHandle,int infoType)
{
    return(simGetStackTableInfo_internal(stackHandle,infoType));
}
SIM_DLLEXPORT int simGetStackUInt8Table(int stackHandle,unsigned char* array,int count)
{
    return(simGetStackUInt8Table_internal(stackHandle,array,count));
}
SIM_DLLEXPORT int simGetStackInt32Table(int stackHandle,int* array,int count)
{
    return(simGetStackInt32Table_internal(stackHandle,array,count));
}
SIM_DLLEXPORT int simGetStackInt64Table(int stackHandle,long long int* array,int count)
{
    return(simGetStackInt64Table_internal(stackHandle,array,count));
}
SIM_DLLEXPORT int simGetStackFloatTable(int stackHandle,float* array,int count)
{
    return(simGetStackFloatTable_internal(stackHandle,array,count));
}
SIM_DLLEXPORT int simGetStackDoubleTable(int stackHandle,double* array,int count)
{
    return(simGetStackDoubleTable_internal(stackHandle,array,count));
}
SIM_DLLEXPORT int simUnfoldStackTable(int stackHandle)
{
    return(simUnfoldStackTable_internal(stackHandle));
}
SIM_DLLEXPORT int simDebugStack(int stackHandle,int cIndex)
{
    return(simDebugStack_internal(stackHandle,cIndex));
}
SIM_DLLEXPORT int simGetEngineInt32Param(int paramId,int objectHandle,const void* object,bool* ok)
{
    return(simGetEngineInt32Param_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT bool simGetEngineBoolParam(int paramId,int objectHandle,const void* object,bool* ok)
{
    return(simGetEngineBoolParam_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT int simSetEngineInt32Param(int paramId,int objectHandle,const void* object,int val)
{
    return(simSetEngineInt32Param_internal(paramId,objectHandle,object,val));
}
SIM_DLLEXPORT int simSetEngineBoolParam(int paramId,int objectHandle,const void* object,bool val)
{
    return(simSetEngineBoolParam_internal(paramId,objectHandle,object,val));
}
SIM_DLLEXPORT int simInsertObjectIntoOctree(int octreeHandle,int objectHandle,int options,const unsigned char* color,unsigned int tag,void* reserved)
{
    return(simInsertObjectIntoOctree_internal(octreeHandle,objectHandle,options,color,tag,reserved));
}
SIM_DLLEXPORT int simSubtractObjectFromOctree(int octreeHandle,int objectHandle,int options,void* reserved)
{
    return(simSubtractObjectFromOctree_internal(octreeHandle,objectHandle,options,reserved));
}
SIM_DLLEXPORT char* simOpenTextEditor(const char* initText,const char* xml,int* various)
{
    return(simOpenTextEditor_internal(initText,xml,various));
}
SIM_DLLEXPORT char* simPackTable(int stackHandle,int* bufferSize)
{
    return(simPackTable_internal(stackHandle,bufferSize));
}
SIM_DLLEXPORT int simUnpackTable(int stackHandle,const char* buffer,int bufferSize)
{
    return(simUnpackTable_internal(stackHandle,buffer,bufferSize));
}
SIM_DLLEXPORT int simSetReferencedHandles(int objectHandle,int count,const int* referencedHandles,const int* reserved1,const int* reserved2)
{
    return(simSetReferencedHandles_internal(objectHandle,count,referencedHandles,reserved1,reserved2));
}
SIM_DLLEXPORT int simGetReferencedHandles(int objectHandle,int** referencedHandles,int** reserved1,int** reserved2)
{
    return(simGetReferencedHandles_internal(objectHandle,referencedHandles,reserved1,reserved2));
}
SIM_DLLEXPORT int simExecuteScriptString(int scriptHandleOrType,const char* stringAtScriptName,int stackHandle)
{
    return(simExecuteScriptString_internal(scriptHandleOrType,stringAtScriptName,stackHandle));
}
SIM_DLLEXPORT char* simGetApiFunc(int scriptHandleOrType,const char* apiWord)
{
    return(simGetApiFunc_internal(scriptHandleOrType,apiWord));
}
SIM_DLLEXPORT char* simGetApiInfo(int scriptHandleOrType,const char* apiWord)
{
    return(simGetApiInfo_internal(scriptHandleOrType,apiWord));
}
SIM_DLLEXPORT int simSetModuleInfo(const char* moduleName,int infoType,const char* stringInfo,int intInfo)
{
    return(simSetModuleInfo_internal(moduleName,infoType,stringInfo,intInfo));
}
SIM_DLLEXPORT int simGetModuleInfo(const char* moduleName,int infoType,char** stringInfo,int* intInfo)
{
    return(simGetModuleInfo_internal(moduleName,infoType,stringInfo,intInfo));
}
SIM_DLLEXPORT int simIsDeprecated(const char* funcOrConst)
{
    return(simIsDeprecated_internal(funcOrConst));
}
SIM_DLLEXPORT char* simGetPersistentDataTags(int* tagCount)
{
    return(simGetPersistentDataTags_internal(tagCount));
}
SIM_DLLEXPORT int simEventNotification(const char* event)
{
    return(simEventNotification_internal(event));
}
SIM_DLLEXPORT int simAddLog(const char* pluginName,int verbosityLevel,const char* logMsg)
{
    return(simAddLog_internal(pluginName,verbosityLevel,logMsg));
}
SIM_DLLEXPORT int simIsDynamicallyEnabled(int objectHandle)
{
    return(simIsDynamicallyEnabled_internal(objectHandle));
}
SIM_DLLEXPORT int simInitScript(int scriptHandle)
{
    return(simInitScript_internal(scriptHandle));
}
SIM_DLLEXPORT int simModuleEntry(int handle,const char* label,int state)
{
    return(simModuleEntry_internal(handle,label,state));
}
SIM_DLLEXPORT int simCheckExecAuthorization(const char* what,const char* args)
{
    return(simCheckExecAuthorization_internal(what,args,-1));
}
SIM_DLLEXPORT int simGetVisionSensorRes(int sensorHandle,int* resolution)
{
    return(simGetVisionSensorRes_internal(sensorHandle,resolution));
}
SIM_DLLEXPORT void _simSetDynamicSimulationIconCode(void* object,int code)
{
    return(_simSetDynamicSimulationIconCode_internal(object,code));
}
SIM_DLLEXPORT void _simSetDynamicObjectFlagForVisualization(void* object,int flag)
{
    return(_simSetDynamicObjectFlagForVisualization_internal(object,flag));
}
SIM_DLLEXPORT int _simGetObjectListSize(int objType)
{
    return(_simGetObjectListSize_internal(objType));
}
SIM_DLLEXPORT const void* _simGetObjectFromIndex(int objType,int index)
{
    return(_simGetObjectFromIndex_internal(objType,index));
}
SIM_DLLEXPORT int _simGetObjectID(const void* object)
{
    return(_simGetObjectID_internal(object));
}
SIM_DLLEXPORT int _simGetObjectType(const void* object)
{
    return(_simGetObjectType_internal(object));
}
SIM_DLLEXPORT const void** _simGetObjectChildren(const void* object,int* count)
{
    return(_simGetObjectChildren_internal(object,count));
}
SIM_DLLEXPORT const void* _simGetGeomProxyFromShape(const void* shape)
{
    return(_simGetGeomProxyFromShape_internal(shape));
}
SIM_DLLEXPORT const void* _simGetParentObject(const void* object)
{
    return(_simGetParentObject_internal(object));
}
SIM_DLLEXPORT const void* _simGetObject(int objID)
{
    return(_simGetObject_internal(objID));
}
SIM_DLLEXPORT bool _simIsShapeDynamicallyStatic(const void* shape)
{
    return(_simIsShapeDynamicallyStatic_internal(shape));
}
SIM_DLLEXPORT int _simGetTreeDynamicProperty(const void* object)
{
    return(_simGetTreeDynamicProperty_internal(object));
}
SIM_DLLEXPORT int _simGetDummyLinkType(const void* dummy,int* linkedDummyID)
{
    return(_simGetDummyLinkType_internal(dummy,linkedDummyID));
}
SIM_DLLEXPORT int _simGetJointMode(const void* joint)
{
    return(_simGetJointMode_internal(joint));
}
SIM_DLLEXPORT bool _simIsJointInHybridOperation(const void* joint)
{
    return(_simIsJointInHybridOperation_internal(joint));
}
SIM_DLLEXPORT void _simDisableDynamicTreeForManipulation(const void* object,bool disableFlag)
{
    return(_simDisableDynamicTreeForManipulation_internal(object,disableFlag));
}
SIM_DLLEXPORT bool _simIsShapeDynamicallyRespondable(const void* shape)
{
    return(_simIsShapeDynamicallyRespondable_internal(shape));
}
SIM_DLLEXPORT int _simGetDynamicCollisionMask(const void* shape)
{
    return(_simGetDynamicCollisionMask_internal(shape));
}
SIM_DLLEXPORT const void* _simGetLastParentForLocalGlobalCollidable(const void* shape)
{
    return(_simGetLastParentForLocalGlobalCollidable_internal(shape));
}
SIM_DLLEXPORT bool _simGetStartSleeping(const void* shape)
{
    return(_simGetStartSleeping_internal(shape));
}
SIM_DLLEXPORT bool _simGetWasPutToSleepOnce(const void* shape)
{
    return(_simGetWasPutToSleepOnce_internal(shape));
}
SIM_DLLEXPORT bool _simGetDynamicsFullRefreshFlag(const void* object)
{
    return(_simGetDynamicsFullRefreshFlag_internal(object));
}
SIM_DLLEXPORT void _simSetDynamicsFullRefreshFlag(const void* object,bool flag)
{
    return(_simSetDynamicsFullRefreshFlag_internal(object,flag));
}
SIM_DLLEXPORT void _simClearAdditionalForceAndTorque(const void* shape)
{
    return(_simClearAdditionalForceAndTorque_internal(shape));
}
SIM_DLLEXPORT int _simGetJointType(const void* joint)
{
    return(_simGetJointType_internal(joint));
}
SIM_DLLEXPORT const void* _simGetGeomWrapFromGeomProxy(const void* geomData)
{
    return(_simGetGeomWrapFromGeomProxy_internal(geomData));
}
SIM_DLLEXPORT int _simGetPurePrimitiveType(const void* geomInfo)
{
    return(_simGetPurePrimitiveType_internal(geomInfo));
}
SIM_DLLEXPORT bool _simIsGeomWrapGeometric(const void* geomInfo)
{
    return(_simIsGeomWrapGeometric_internal(geomInfo));
}
SIM_DLLEXPORT bool _simIsGeomWrapConvex(const void* geomInfo)
{
    return(_simIsGeomWrapConvex_internal(geomInfo));
}
SIM_DLLEXPORT int _simGetGeometricCount(const void* geomInfo)
{
    return(_simGetGeometricCount_internal(geomInfo));
}
SIM_DLLEXPORT void _simGetAllGeometrics(const void* geomInfo,void** allGeometrics)
{
    return(_simGetAllGeometrics_internal(geomInfo,allGeometrics));
}
SIM_DLLEXPORT void _simMakeDynamicAnnouncement(int announceType)
{
    return(_simMakeDynamicAnnouncement_internal(announceType));
}
SIM_DLLEXPORT int _simGetTimeDiffInMs(int previousTime)
{
    return(_simGetTimeDiffInMs_internal(previousTime));
}
SIM_DLLEXPORT bool _simDoEntitiesCollide(int entity1ID,int entity2ID,int* cacheBuffer,bool overrideCollidableFlagIfShape1,bool overrideCollidableFlagIfShape2,bool pathOrMotionPlanningRoutineCalling)
{
    return(_simDoEntitiesCollide_internal(entity1ID,entity2ID,cacheBuffer,overrideCollidableFlagIfShape1,overrideCollidableFlagIfShape2,pathOrMotionPlanningRoutineCalling));
}
SIM_DLLEXPORT int simFloatingViewRemove(int floatingViewHandle)
{
    return(simFloatingViewRemove_internal(floatingViewHandle));
}
SIM_DLLEXPORT const void* _simGetIkGroupObject(int ikGroupID)
{
    return(_simGetIkGroupObject_internal(ikGroupID));
}
SIM_DLLEXPORT int _simMpHandleIkGroupObject(const void* ikGroup)
{
    return(_simMpHandleIkGroupObject_internal(ikGroup));
}
// Following courtesy of Stephen James:
// Functions to allow an external application to have control of CoppeliaSim's thread loop
SIM_DLLEXPORT int simExtLaunchUIThread(const char* applicationName,int options,const char* sceneOrModelOrUiToLoad, const char* applicationDir_)
{
    return(simExtLaunchUIThread_internal(applicationName,options,sceneOrModelOrUiToLoad,applicationDir_));
}
SIM_DLLEXPORT int simExtPostExitRequest()
{
    return(simExtPostExitRequest_internal());
}
SIM_DLLEXPORT int simExtGetExitRequest()
{
    return(simExtGetExitRequest_internal());
}
SIM_DLLEXPORT int simExtStep(bool stepIfRunning)
{
    return(simExtStep_internal(stepIfRunning));
}
SIM_DLLEXPORT int simExtCanInitSimThread()
{
    return(simExtCanInitSimThread_internal());
}
SIM_DLLEXPORT int simExtSimThreadInit()
{
    return(simExtSimThreadInit_internal());
}
SIM_DLLEXPORT int simExtSimThreadDestroy()
{
    return(simExtSimThreadDestroy_internal());
}
SIM_DLLEXPORT float* simGetVisionSensorDepth(int sensorHandle,int options,const int* pos,const int* size,int* resolution)
{
    return(simGetVisionSensorDepth_internal(sensorHandle,options,pos,size,resolution));
}
SIM_DLLEXPORT int _simSetVisionSensorDepth(int sensorHandle,int options,const float* depth)
{
    return(_simSetVisionSensorDepth_internal(sensorHandle,options,depth));
}
SIM_DLLEXPORT float* simCheckVisionSensorEx(int visionSensorHandle,int entityHandle,bool returnImage)
{
    return(simCheckVisionSensorEx_internal(visionSensorHandle,entityHandle,returnImage));
}

SIM_DLLEXPORT int simSetArrayParam_D(int parameter,const double* arrayOfValues)
{
    return(simSetArrayParam_internal(parameter,arrayOfValues));
}
SIM_DLLEXPORT int simGetArrayParam_D(int parameter,double* arrayOfValues)
{
    return(simGetArrayParam_internal(parameter,arrayOfValues));
}
SIM_DLLEXPORT int simGetShapeViz_D(int shapeHandle,int index,struct SShapeVizInfo* info)
{
    return(simGetShapeViz_internal(shapeHandle,index,info));
}
SIM_DLLEXPORT int simSetFloatParam_D(int parameter,double floatState)
{
    return(simSetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT int simGetFloatParam_D(int parameter,double* floatState)
{
    return(simGetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT int simGetObjectMatrix_D(int objectHandle,int relativeToObjectHandle,double* matrix)
{
    return(simGetObjectMatrix_internal(objectHandle,relativeToObjectHandle,matrix));
}
SIM_DLLEXPORT int simSetObjectMatrix_D(int objectHandle,int relativeToObjectHandle,const double* matrix)
{
    return(simSetObjectMatrix_internal(objectHandle,relativeToObjectHandle,matrix));
}
SIM_DLLEXPORT int simGetObjectPose_D(int objectHandle,int relativeToObjectHandle,double* pose)
{
    return(simGetObjectPose_internal(objectHandle,relativeToObjectHandle,pose));
}
SIM_DLLEXPORT int simSetObjectPose_D(int objectHandle,int relativeToObjectHandle,const double* pose)
{
    return(simSetObjectPose_internal(objectHandle,relativeToObjectHandle,pose));
}
SIM_DLLEXPORT int simGetObjectPosition_D(int objectHandle,int relativeToObjectHandle,double* position)
{
    return(simGetObjectPosition_internal(objectHandle,relativeToObjectHandle,position));
}
SIM_DLLEXPORT int simSetObjectPosition_D(int objectHandle,int relativeToObjectHandle,const double* position)
{
    return(simSetObjectPosition_internal(objectHandle,relativeToObjectHandle,position));
}
SIM_DLLEXPORT int simGetObjectOrientation_D(int objectHandle,int relativeToObjectHandle,double* eulerAngles)
{
    return(simGetObjectOrientation_internal(objectHandle,relativeToObjectHandle,eulerAngles));
}
SIM_DLLEXPORT int simGetObjectQuaternion_D(int objectHandle,int relativeToObjectHandle,double* quaternion)
{
    return(simGetObjectQuaternion_internal(objectHandle,relativeToObjectHandle,quaternion));
}
SIM_DLLEXPORT int simSetObjectQuaternion_D(int objectHandle,int relativeToObjectHandle,const double* quaternion)
{
    return(simSetObjectQuaternion_internal(objectHandle,relativeToObjectHandle,quaternion));
}
SIM_DLLEXPORT int simSetObjectOrientation_D(int objectHandle,int relativeToObjectHandle,const double* eulerAngles)
{
    return(simSetObjectOrientation_internal(objectHandle,relativeToObjectHandle,eulerAngles));
}
SIM_DLLEXPORT int simGetJointPosition_D(int objectHandle,double* position)
{
    return(simGetJointPosition_internal(objectHandle,position));
}
SIM_DLLEXPORT int simSetJointPosition_D(int objectHandle,double position)
{
    return(simSetJointPosition_internal(objectHandle,position));
}
SIM_DLLEXPORT int simSetJointTargetPosition_D(int objectHandle,double targetPosition)
{
    return(simSetJointTargetPosition_internal(objectHandle,targetPosition));
}
SIM_DLLEXPORT int simGetJointTargetPosition_D(int objectHandle,double* targetPosition)
{
    return(simGetJointTargetPosition_internal(objectHandle,targetPosition));
}
SIM_DLLEXPORT int simGetObjectChildPose_D(int objectHandle,double* pose)
{
    return(simGetObjectChildPose_internal(objectHandle,pose));
}
SIM_DLLEXPORT int simSetObjectChildPose_D(int objectHandle,const double* pose)
{
    return(simSetObjectChildPose_internal(objectHandle,pose));
}
SIM_DLLEXPORT int simGetJointInterval_D(int objectHandle,bool* cyclic,double* interval)
{
    return(simGetJointInterval_internal(objectHandle,cyclic,interval));
}
SIM_DLLEXPORT int simSetJointInterval_D(int objectHandle,bool cyclic,const double* interval)
{
    return(simSetJointInterval_internal(objectHandle,cyclic,interval));
}
SIM_DLLEXPORT int simBuildIdentityMatrix_D(double* matrix)
{
    return(simBuildIdentityMatrix_internal(matrix));
}
SIM_DLLEXPORT int simBuildMatrix_D(const double* position,const double* eulerAngles,double* matrix)
{
    return(simBuildMatrix_internal(position,eulerAngles,matrix));
}
SIM_DLLEXPORT int simBuildPose_D(const double* position,const double* eulerAngles,double* pose)
{
    return(simBuildPose_internal(position,eulerAngles,pose));
}
SIM_DLLEXPORT int simGetEulerAnglesFromMatrix_D(const double* matrix,double* eulerAngles)
{
    return(simGetEulerAnglesFromMatrix_internal(matrix,eulerAngles));
}
SIM_DLLEXPORT int simInvertMatrix_D(double* matrix)
{
    return(simInvertMatrix_internal(matrix));
}
SIM_DLLEXPORT int simMultiplyMatrices_D(const double* matrixIn1,const double* matrixIn2,double* matrixOut)
{
    return(simMultiplyMatrices_internal(matrixIn1,matrixIn2,matrixOut));
}
SIM_DLLEXPORT int simMultiplyPoses_D(const double* poseIn1,const double* poseIn2,double* poseOut)
{
    return(simMultiplyPoses_internal(poseIn1,poseIn2,poseOut));
}
SIM_DLLEXPORT int simInvertPose_D(double* pose)
{
    return(simInvertPose_internal(pose));
}
SIM_DLLEXPORT int simInterpolatePoses_D(const double* poseIn1,const double* poseIn2,double interpolFactor,double* poseOut)
{
    return(simInterpolatePoses_internal(poseIn1,poseIn2,interpolFactor,poseOut));
}
SIM_DLLEXPORT int simPoseToMatrix_D(const double* poseIn,double* matrixOut)
{
    return(simPoseToMatrix_internal(poseIn,matrixOut));
}
SIM_DLLEXPORT int simMatrixToPose_D(const double* matrixIn,double* poseOut)
{
    return(simMatrixToPose_internal(matrixIn,poseOut));
}
SIM_DLLEXPORT int simInterpolateMatrices_D(const double* matrixIn1,const double* matrixIn2,double interpolFactor,double* matrixOut)
{
    return(simInterpolateMatrices_internal(matrixIn1,matrixIn2,interpolFactor,matrixOut));
}
SIM_DLLEXPORT int simTransformVector_D(const double* matrix,double* vect)
{
    return(simTransformVector_internal(matrix,vect));
}
SIM_DLLEXPORT double simGetSimulationTime_D()
{
    return(simGetSimulationTime_internal());
}
SIM_DLLEXPORT double simGetSystemTime_D()
{
    return(double(simGetSystemTime_internal()));
}
SIM_DLLEXPORT int simHandleProximitySensor_D(int sensorHandle,double* detectedPoint,int* detectedObjectHandle,double* normalVector)
{
    return(simHandleProximitySensor_internal(sensorHandle,detectedPoint,detectedObjectHandle,normalVector));
}
SIM_DLLEXPORT int simReadProximitySensor_D(int sensorHandle,double* detectedPoint,int* detectedObjectHandle,double* normalVector)
{
    return(simReadProximitySensor_internal(sensorHandle,detectedPoint,detectedObjectHandle,normalVector));
}
SIM_DLLEXPORT int simHandleDynamics_D(double deltaTime)
{
    return(simHandleDynamics_internal(deltaTime));
}
SIM_DLLEXPORT int simCheckProximitySensor_D(int sensorHandle,int entityHandle,double* detectedPoint)
{
    return(simCheckProximitySensor_internal(sensorHandle,entityHandle,detectedPoint));
}
SIM_DLLEXPORT int simCheckProximitySensorEx_D(int sensorHandle,int entityHandle,int detectionMode,double detectionThreshold,double maxAngle,double* detectedPoint,int* detectedObjectHandle,double* normalVector)
{
    return(simCheckProximitySensorEx_internal(sensorHandle,entityHandle,detectionMode,detectionThreshold,maxAngle,detectedPoint,detectedObjectHandle,normalVector));
}
SIM_DLLEXPORT int simCheckProximitySensorEx2_D(int sensorHandle,double* vertexPointer,int itemType,int itemCount,int detectionMode,double detectionThreshold,double maxAngle,double* detectedPoint,double* normalVector)
{
    return(simCheckProximitySensorEx2_internal(sensorHandle,vertexPointer,itemType,itemCount,detectionMode,detectionThreshold,maxAngle,detectedPoint,normalVector));
}
SIM_DLLEXPORT int simCheckCollisionEx_D(int entity1Handle,int entity2Handle,double** intersectionSegments)
{
    return(simCheckCollisionEx_internal(entity1Handle,entity2Handle,intersectionSegments));
}
SIM_DLLEXPORT int simCheckDistance_D(int entity1Handle,int entity2Handle,double threshold,double* distanceData)
{
    return(simCheckDistance_internal(entity1Handle,entity2Handle,threshold,distanceData));
}
SIM_DLLEXPORT int simSetSimulationTimeStep_D(double timeStep)
{
    return(simSetSimulationTimeStep_internal(timeStep));
}
SIM_DLLEXPORT double simGetSimulationTimeStep_D()
{
    return(simGetSimulationTimeStep_internal());
}
SIM_DLLEXPORT int simAdjustRealTimeTimer_D(int instanceIndex,double deltaTime)
{
    return(simAdjustRealTimeTimer_internal(instanceIndex,deltaTime));
}
SIM_DLLEXPORT int simFloatingViewAdd_D(double posX,double posY,double sizeX,double sizeY,int options)
{
    return(simFloatingViewAdd_internal(posX,posY,sizeX,sizeY,options));
}
SIM_DLLEXPORT int simFloatingViewRemove_D(int floatingViewHandle)
{
    return(simFloatingViewRemove_internal(floatingViewHandle));
}
SIM_DLLEXPORT int simHandleGraph_D(int graphHandle,double simulationTime)
{
    return(simHandleGraph_internal(graphHandle,simulationTime));
}
SIM_DLLEXPORT int simAddGraphStream_D(int graphHandle,const char* streamName,const char* unitStr,int options,const float* color,double cyclicRange)
{
    return(simAddGraphStream_internal(graphHandle,streamName,unitStr,options,color,cyclicRange));
}
SIM_DLLEXPORT int simSetGraphStreamTransformation_D(int graphHandle,int streamId,int trType,double mult,double off,int movingAvgPeriod)
{
    return(simSetGraphStreamTransformation_internal(graphHandle,streamId,trType,mult,off,movingAvgPeriod));
}
SIM_DLLEXPORT int simAddGraphCurve_D(int graphHandle,const char* curveName,int dim,const int* streamIds,const double* defaultValues,const char* unitStr,int options,const float* color,int curveWidth)
{
    return(simAddGraphCurve_internal(graphHandle,curveName,dim,streamIds,defaultValues,unitStr,options,color,curveWidth));
}
SIM_DLLEXPORT int simSetGraphStreamValue_D(int graphHandle,int streamId,double value)
{
    return(simSetGraphStreamValue_internal(graphHandle,streamId,value));
}
SIM_DLLEXPORT int simSetJointTargetVelocity_D(int objectHandle,double targetVelocity)
{
    return(simSetJointTargetVelocity_internal(objectHandle,targetVelocity));
}
SIM_DLLEXPORT int simGetJointTargetVelocity_D(int objectHandle,double* targetVelocity)
{
    return(simGetJointTargetVelocity_internal(objectHandle,targetVelocity));
}
SIM_DLLEXPORT int simScaleObjects_D(const int* objectHandles,int objectCount,double scalingFactor,bool scalePositionsToo)
{
    return(simScaleObjects_internal(objectHandles,objectCount,scalingFactor,scalePositionsToo));
}
SIM_DLLEXPORT int simAddDrawingObject_D(int objectType,double size,double duplicateTolerance,int parentObjectHandle,int maxItemCount,const float* color,const float* setToNULL,const float* setToNULL2,const float* setToNULL3)
{
    return(simAddDrawingObject_internal(objectType,size,duplicateTolerance,parentObjectHandle,maxItemCount,color,setToNULL,setToNULL2,setToNULL3));
}
SIM_DLLEXPORT int simAddDrawingObjectItem_D(int objectHandle,const double* itemData)
{
    return(simAddDrawingObjectItem_internal(objectHandle,itemData));
}
SIM_DLLEXPORT double simGetObjectSizeFactor_D(int objectHandle)
{
    return(simGetObjectSizeFactor_internal(objectHandle));
}
SIM_DLLEXPORT int simSetFloatSignal_D(const char* signalName,double signalValue)
{
    return(simSetFloatSignal_internal(signalName,signalValue));
}
SIM_DLLEXPORT int simGetFloatSignal_D(const char* signalName,double* signalValue)
{
    return(simGetFloatSignal_internal(signalName,signalValue));
}
SIM_DLLEXPORT int simReadForceSensor_D(int objectHandle,double* forceVector,double* torqueVector)
{
    return(simReadForceSensor_internal(objectHandle,forceVector,torqueVector));
}
SIM_DLLEXPORT int simSetLightParameters(int objectHandle,int state,const float* setToNULL,const float* diffusePart,const float* specularPart)
{
    return(simSetLightParameters_internal(objectHandle,state,setToNULL,diffusePart,specularPart));
}
SIM_DLLEXPORT int simGetLightParameters_D(int objectHandle,double* setToNULL,double* diffusePart,double* specularPart)
{
    return(simGetLightParameters_internal(objectHandle,setToNULL,diffusePart,specularPart));
}
SIM_DLLEXPORT int simGetVelocity_D(int shapeHandle,double* linearVelocity,double* angularVelocity)
{
    return(simGetVelocity_internal(shapeHandle,linearVelocity,angularVelocity));
}
SIM_DLLEXPORT int simGetObjectVelocity_D(int objectHandle,double* linearVelocity,double* angularVelocity)
{
    return(simGetObjectVelocity_internal(objectHandle,linearVelocity,angularVelocity));
}
SIM_DLLEXPORT int simGetJointVelocity_D(int jointHandle,double* velocity)
{
    return(simGetJointVelocity_internal(jointHandle,velocity));
}
SIM_DLLEXPORT int simAddForceAndTorque_D(int shapeHandle,const double* force,const double* torque)
{
    return(simAddForceAndTorque_internal(shapeHandle,force,torque));
}
SIM_DLLEXPORT int simAddForce_D(int shapeHandle,const double* position,const double* force)
{
    return(simAddForce_internal(shapeHandle,position,force));
}
SIM_DLLEXPORT int simSetObjectColor(int objectHandle,int index,int colorComponent,const float* rgbData)
{
    return(simSetObjectColor_internal(objectHandle,index,colorComponent,rgbData));
}
SIM_DLLEXPORT int simGetObjectColor(int objectHandle,int index,int colorComponent,float* rgbData)
{
    return(simGetObjectColor_internal(objectHandle,index,colorComponent,rgbData));
}
SIM_DLLEXPORT int simSetShapeColor(int shapeHandle,const char* colorName,int colorComponent,const float* rgbData)
{
    return(simSetShapeColor_internal(shapeHandle,colorName,colorComponent,rgbData));
}
SIM_DLLEXPORT int simGetShapeColor(int shapeHandle,const char* colorName,int colorComponent,float* rgbData)
{
    return(simGetShapeColor_internal(shapeHandle,colorName,colorComponent,rgbData));
}
SIM_DLLEXPORT int simGetContactInfo_D(int dynamicPass,int objectHandle,int index,int* objectHandles,double* contactInfo)
{
    return(simGetContactInfo_internal(dynamicPass,objectHandle,index,objectHandles,contactInfo));
}
SIM_DLLEXPORT int simAuxiliaryConsoleOpen(const char* title,int maxLines,int mode,const int* position,const int* size,const float* textColor,const float* backgroundColor)
{
    return(simAuxiliaryConsoleOpen_internal(title,maxLines,mode,position,size,textColor,backgroundColor));
}
SIM_DLLEXPORT int simImportShape_D(int fileformat,const char* pathAndFilename,int options,double identicalVerticeTolerance,double scalingFactor)
{
    return(simImportShape_internal(fileformat,pathAndFilename,options,identicalVerticeTolerance,scalingFactor));
}
SIM_DLLEXPORT int simImportMesh_D(int fileformat,const char* pathAndFilename,int options,double identicalVerticeTolerance,double scalingFactor,double*** vertices,int** verticesSizes,int*** indices,int** indicesSizes,double*** reserved,char*** names)
{
    return(simImportMesh_internal(fileformat,pathAndFilename,options,identicalVerticeTolerance,scalingFactor,vertices,verticesSizes,indices,indicesSizes,reserved,names));
}
SIM_DLLEXPORT int simExportMesh_D(int fileformat,const char* pathAndFilename,int options,double scalingFactor,int elementCount,const double** vertices,const int* verticesSizes,const int** indices,const int* indicesSizes,double** reserved,const char** names)
{
    return(simExportMesh_internal(fileformat,pathAndFilename,options,scalingFactor,elementCount,vertices,verticesSizes,indices,indicesSizes,reserved,names));
}
SIM_DLLEXPORT int simCreateMeshShape_D(int options,double shadingAngle,const double* vertices,int verticesSize,const int* indices,int indicesSize,double* reserved)
{
    return(simCreateMeshShape_internal(options,shadingAngle,vertices,verticesSize,indices,indicesSize,reserved));
}
SIM_DLLEXPORT int simCreatePrimitiveShape_D(int primitiveType,const double* sizes,int options)
{
    return(simCreatePrimitiveShape_internal(primitiveType,sizes,options));
}
SIM_DLLEXPORT int simCreateHeightfieldShape_D(int options,double shadingAngle,int xPointCount,int yPointCount,double xSize,const double* heights)
{
    return(simCreateHeightfieldShape_internal(options,shadingAngle,xPointCount,yPointCount,xSize,heights));
}
SIM_DLLEXPORT int simGetShapeMesh_D(int shapeHandle,double** vertices,int* verticesSize,int** indices,int* indicesSize,double** normals)
{
    return(simGetShapeMesh_internal(shapeHandle,vertices,verticesSize,indices,indicesSize,normals));
}
SIM_DLLEXPORT int simCreateJoint_D(int jointType,int jointMode,int options,const double* sizes,const double* reservedA,const double* reservedB)
{
    return(simCreateJoint_internal(jointType,jointMode,options,sizes,reservedA,reservedB));
}
SIM_DLLEXPORT int simGetObjectFloatParam_D(int objectHandle,int ParamID,double* Param)
{
    return(simGetObjectFloatParam_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT int simSetObjectFloatParam_D(int objectHandle,int ParamID,double Param)
{
    return(simSetObjectFloatParam_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT double* simGetObjectFloatArrayParam_D(int objectHandle,int ParamID,int* size)
{
    return(simGetObjectFloatArrayParam_internal(objectHandle,ParamID,size));
}
SIM_DLLEXPORT int simSetObjectFloatArrayParam_D(int objectHandle,int ParamID,const double* params,int size)
{
    return(simSetObjectFloatArrayParam_internal(objectHandle,ParamID,params,size));
}
SIM_DLLEXPORT int simGetRotationAxis_D(const double* matrixStart,const double* matrixGoal,double* axis,double* angle)
{
    return(simGetRotationAxis_internal(matrixStart,matrixGoal,axis,angle));
}
SIM_DLLEXPORT int simRotateAroundAxis_D(const double* matrixIn,const double* axis,const double* axisPos,double angle,double* matrixOut)
{
    return(simRotateAroundAxis_internal(matrixIn,axis,axisPos,angle,matrixOut));
}
SIM_DLLEXPORT int simGetJointForce_D(int jointHandle,double* forceOrTorque)
{
    return(simGetJointForce_internal(jointHandle,forceOrTorque));
}
SIM_DLLEXPORT int simGetJointTargetForce_D(int jointHandle,double* forceOrTorque)
{
    return(simGetJointTargetForce_internal(jointHandle,forceOrTorque));
}
SIM_DLLEXPORT int simSetJointTargetForce_D(int objectHandle,double forceOrTorque,bool signedValue)
{
    return(simSetJointTargetForce_internal(objectHandle,forceOrTorque,signedValue));
}
SIM_DLLEXPORT int simCameraFitToView_D(int viewHandleOrIndex,int objectCount,const int* objectHandles,int options,double scaling)
{
    return(simCameraFitToView_internal(viewHandleOrIndex,objectCount,objectHandles,options,scaling));
}
SIM_DLLEXPORT int simHandleVisionSensor_D(int visionSensorHandle,double** auxValues,int** auxValuesCount)
{
    return(simHandleVisionSensor_internal(visionSensorHandle,auxValues,auxValuesCount));
}
SIM_DLLEXPORT int simReadVisionSensor_D(int visionSensorHandle,double** auxValues,int** auxValuesCount)
{
    return(simReadVisionSensor_internal(visionSensorHandle,auxValues,auxValuesCount));
}
SIM_DLLEXPORT int simCheckVisionSensor_D(int visionSensorHandle,int entityHandle,double** auxValues,int** auxValuesCount)
{
    return(simCheckVisionSensor_internal(visionSensorHandle,entityHandle,auxValues,auxValuesCount));
}
SIM_DLLEXPORT unsigned char* simGetVisionSensorImg_D(int sensorHandle,int options,double rgbaCutOff,const int* pos,const int* size,int* resolution)
{
    return(simGetVisionSensorImg_internal(sensorHandle,options,rgbaCutOff,pos,size,resolution));
}
SIM_DLLEXPORT int simCreateDummy_D(double size,const float* reserved)
{
    return(simCreateDummy_internal(size,reserved));
}
SIM_DLLEXPORT int simCreateForceSensor_D(int options,const int* intParams,const double* floatParams,const double* reserved)
{
    return(simCreateForceSensor_internal(options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT int simCreateProximitySensor_D(int sensorType,int subType,int options,const int* intParams,const double* floatParams,const double* reserved)
{
    return(simCreateProximitySensor_internal(sensorType,subType,options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT int simCreateVisionSensor_D(int options,const int* intParams,const double* floatParams,const double* reserved)
{
    return(simCreateVisionSensor_internal(options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT int simConvexDecompose_D(int shapeHandle,int options,const int* intParams,const double* floatParams)
{
    return(simConvexDecompose_internal(shapeHandle,options,intParams,floatParams));
}
SIM_DLLEXPORT int simCreateTexture_D(const char* fileName,int options,const double* planeSizes,const double* scalingUV,const double* xy_g,int fixedResolution,int* textureId,int* resolution,const void* reserved)
{
    return(simCreateTexture_internal(fileName,options,planeSizes,scalingUV,xy_g,fixedResolution,textureId,resolution,reserved));
}
SIM_DLLEXPORT int simWriteTexture_D(int textureId,int options,const char* data,int posX,int posY,int sizeX,int sizeY,double interpol)
{
    return(simWriteTexture_internal(textureId,options,data,posX,posY,sizeX,sizeY,interpol));
}
SIM_DLLEXPORT int simGetShapeGeomInfo_D(int shapeHandle,int* intData,double* floatData,void* reserved)
{
    return(simGetShapeGeomInfo_internal(shapeHandle,intData,floatData,reserved));
}
SIM_DLLEXPORT int simScaleObject_D(int objectHandle,double xScale,double yScale,double zScale,int options)
{
    return(simScaleObject_internal(objectHandle,xScale,yScale,zScale,options));
}
SIM_DLLEXPORT int simSetShapeTexture_D(int shapeHandle,int textureId,int mappingMode,int options,const double* uvScaling,const double* position,const double* orientation)
{
    return(simSetShapeTexture_internal(shapeHandle,textureId,mappingMode,options,uvScaling,position,orientation));
}
SIM_DLLEXPORT int simTransformImage_D(unsigned char* image,const int* resolution,int options,const double* floatParams,const int* intParams,void* reserved)
{
    return(simTransformImage_internal(image,resolution,options,floatParams,intParams,reserved));
}
SIM_DLLEXPORT int simGetQHull_D(const double* inVertices,int inVerticesL,double** verticesOut,int* verticesOutL,int** indicesOut,int* indicesOutL,int reserved1,const double* reserved2)
{
    return(simGetQHull_internal(inVertices,inVerticesL,verticesOut,verticesOutL,indicesOut,indicesOutL,reserved1,reserved2));
}
SIM_DLLEXPORT int simGetDecimatedMesh_D(const double* inVertices,int inVerticesL,const int* inIndices,int inIndicesL,double** verticesOut,int* verticesOutL,int** indicesOut,int* indicesOutL,double decimationPercent,int reserved1,const double* reserved2)
{
    return(simGetDecimatedMesh_internal(inVertices,inVerticesL,inIndices,inIndicesL,verticesOut,verticesOutL,indicesOut,indicesOutL,decimationPercent,reserved1,reserved2));
}
SIM_DLLEXPORT int simComputeMassAndInertia_D(int shapeHandle,double density)
{
    return(simComputeMassAndInertia_internal(shapeHandle,density));
}
SIM_DLLEXPORT double simGetEngineFloatParam_D(int paramId,int objectHandle,const void* object,bool* ok)
{
    return(simGetEngineFloatParam_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT int simSetEngineFloatParam_D(int paramId,int objectHandle,const void* object,double val)
{
    return(simSetEngineFloatParam_internal(paramId,objectHandle,object,val));
}
SIM_DLLEXPORT int simCreateOctree_D(double voxelSize,int options,double pointSize,void* reserved)
{
    return(simCreateOctree_internal(voxelSize,options,pointSize,reserved));
}
SIM_DLLEXPORT int simCreatePointCloud_D(double maxVoxelSize,int maxPtCntPerVoxel,int options,double pointSize,void* reserved)
{
    return(simCreatePointCloud_internal(maxVoxelSize,maxPtCntPerVoxel,options,pointSize,reserved));
}
SIM_DLLEXPORT int simSetPointCloudOptions_D(int pointCloudHandle,double maxVoxelSize,int maxPtCntPerVoxel,int options,double pointSize,void* reserved)
{
    return(simSetPointCloudOptions_internal(pointCloudHandle,maxVoxelSize,maxPtCntPerVoxel,options,pointSize,reserved));
}
SIM_DLLEXPORT int simGetPointCloudOptions_D(int pointCloudHandle,double* maxVoxelSize,int* maxPtCntPerVoxel,int* options,double* pointSize,void* reserved)
{
    return(simGetPointCloudOptions_internal(pointCloudHandle,maxVoxelSize,maxPtCntPerVoxel,options,pointSize,reserved));
}
SIM_DLLEXPORT int simInsertVoxelsIntoOctree_D(int octreeHandle,int options,const double* pts,int ptCnt,const unsigned char* color,const unsigned int* tag,void* reserved)
{
    return(simInsertVoxelsIntoOctree_internal(octreeHandle,options,pts,ptCnt,color,tag,reserved));
}
SIM_DLLEXPORT int simRemoveVoxelsFromOctree_D(int octreeHandle,int options,const double* pts,int ptCnt,void* reserved)
{
    return(simRemoveVoxelsFromOctree_internal(octreeHandle,options,pts,ptCnt,reserved));
}
SIM_DLLEXPORT int simInsertPointsIntoPointCloud_D(int pointCloudHandle,int options,const double* pts,int ptCnt,const unsigned char* color,void* optionalValues)
{
    return(simInsertPointsIntoPointCloud_internal(pointCloudHandle,options,pts,ptCnt,color,optionalValues));
}
SIM_DLLEXPORT int simRemovePointsFromPointCloud_D(int pointCloudHandle,int options,const double* pts,int ptCnt,double tolerance,void* reserved)
{
    return(simRemovePointsFromPointCloud_internal(pointCloudHandle,options,pts,ptCnt,tolerance,reserved));
}
SIM_DLLEXPORT int simIntersectPointsWithPointCloud_D(int pointCloudHandle,int options,const double* pts,int ptCnt,double tolerance,void* reserved)
{
    return(simIntersectPointsWithPointCloud_internal(pointCloudHandle,options,pts,ptCnt,tolerance,reserved));
}
SIM_DLLEXPORT const double* simGetOctreeVoxels_D(int octreeHandle,int* ptCnt,void* reserved)
{
    return(simGetOctreeVoxels_internal(octreeHandle,ptCnt,reserved));
}
SIM_DLLEXPORT const double* simGetPointCloudPoints_D(int pointCloudHandle,int* ptCnt,void* reserved)
{
    return(simGetPointCloudPoints_internal(pointCloudHandle,ptCnt,reserved));
}
SIM_DLLEXPORT int simInsertObjectIntoPointCloud_D(int pointCloudHandle,int objectHandle,int options,double gridSize,const unsigned char* color,void* optionalValues)
{
    return(simInsertObjectIntoPointCloud_internal(pointCloudHandle,objectHandle,options,gridSize,color,optionalValues));
}
SIM_DLLEXPORT int simSubtractObjectFromPointCloud_D(int pointCloudHandle,int objectHandle,int options,double tolerance,void* reserved)
{
    return(simSubtractObjectFromPointCloud_internal(pointCloudHandle,objectHandle,options,tolerance,reserved));
}
SIM_DLLEXPORT int simCheckOctreePointOccupancy_D(int octreeHandle,int options,const double* points,int ptCnt,unsigned int* tag,unsigned long long int* location,void* reserved)
{
    return(simCheckOctreePointOccupancy_internal(octreeHandle,options,points,ptCnt,tag,location,reserved));
}
SIM_DLLEXPORT int simApplyTexture_D(int shapeHandle,const double* textureCoordinates,int textCoordSize,const unsigned char* texture,const int* textureResolution,int options)
{
    return(simApplyTexture_internal(shapeHandle,textureCoordinates,textCoordSize,texture,textureResolution,options));
}
SIM_DLLEXPORT int simSetJointDependency_D(int jointHandle,int masterJointHandle,double offset,double multCoeff)
{
    return(simSetJointDependency_internal(jointHandle,masterJointHandle,offset,multCoeff));
}
SIM_DLLEXPORT int simGetJointDependency_D(int jointHandle,int* masterJointHandle,double* offset,double* multCoeff)
{
    return(simGetJointDependency_internal(jointHandle,masterJointHandle,offset,multCoeff));
}
SIM_DLLEXPORT int simGetShapeMass_D(int shapeHandle,double* mass)
{
    return(simGetShapeMass_internal(shapeHandle,mass));
}
SIM_DLLEXPORT int simSetShapeMass_D(int shapeHandle,double mass)
{
    return(simSetShapeMass_internal(shapeHandle,mass));
}
SIM_DLLEXPORT int simGetShapeInertia_D(int shapeHandle,double* inertiaMatrix,double* transformationMatrix)
{
    return(simGetShapeInertia_internal(shapeHandle,inertiaMatrix,transformationMatrix));
}
SIM_DLLEXPORT int simSetShapeInertia_D(int shapeHandle,const double* inertiaMatrix,const double* transformationMatrix)
{
    return(simSetShapeInertia_internal(shapeHandle,inertiaMatrix,transformationMatrix));
}
SIM_DLLEXPORT int simGenerateShapeFromPath_D(const double* path,int pathSize,const double* section,int sectionSize,int options,const double* upVector,double reserved)
{
    return(simGenerateShapeFromPath_internal(path,pathSize,section,sectionSize,options,upVector,reserved));
}
SIM_DLLEXPORT double simGetClosestPosOnPath_D(const double* path,int pathSize,const double* pathLengths,const double* absPt)
{
    return(simGetClosestPosOnPath_internal(path,pathSize,pathLengths,absPt));
}
SIM_DLLEXPORT int simExtCallScriptFunction_D(int scriptHandleOrType, const char* functionNameAtScriptName,
                                               const int* inIntData, int inIntCnt,
                                               const double* inFloatData, int inFloatCnt,
                                               const char** inStringData, int inStringCnt,
                                               const char* inBufferData, int inBufferCnt,
                                               int** outIntData, int* outIntCnt,
                                               double** outFloatData, int* outFloatCnt,
                                               char*** outStringData, int* outStringCnt,
                                               char** outBufferData, int* outBufferSize)
{
    return(simExtCallScriptFunction_internal(scriptHandleOrType, functionNameAtScriptName,
                                             inIntData, inIntCnt, inFloatData, inFloatCnt,
                                             inStringData, inStringCnt, inBufferData, inBufferCnt,
                                             outIntData, outIntCnt, outFloatData, outFloatCnt,
                                             outStringData, outStringCnt, outBufferData, outBufferSize));
}
SIM_DLLEXPORT void _simGetObjectLocalTransformation_D(const void* object,double* pos,double* quat,bool excludeFirstJointTransformation)
{
    return(_simGetObjectLocalTransformation_internal(object,pos,quat,excludeFirstJointTransformation));
}
SIM_DLLEXPORT void _simSetObjectLocalTransformation_D(void* object,const double* pos,const double* quat,double simTime)
{
    return(_simSetObjectLocalTransformation_internal(object,pos,quat,simTime));
}
SIM_DLLEXPORT void _simDynReportObjectCumulativeTransformation_D(void* object,const double* pos,const double* quat,double simTime)
{
    return(_simDynReportObjectCumulativeTransformation_internal(object,pos,quat,simTime));
}
SIM_DLLEXPORT void _simSetObjectCumulativeTransformation_D(void* object,const double* pos,const double* quat,bool keepChildrenInPlace)
{
    return(_simSetObjectCumulativeTransformation_internal(object,pos,quat,keepChildrenInPlace));
}
SIM_DLLEXPORT void _simGetObjectCumulativeTransformation_D(const void* object,double* pos,double* quat,bool excludeFirstJointTransformation)
{
    return(_simGetObjectCumulativeTransformation_internal(object,pos,quat,excludeFirstJointTransformation));
}
SIM_DLLEXPORT void _simSetJointVelocity_D(const void* joint,double vel)
{
    return(_simSetJointVelocity_internal(joint,vel));
}
SIM_DLLEXPORT void _simSetJointPosition_D(const void* joint,double pos)
{
    return(_simSetJointPosition_internal(joint,pos));
}
SIM_DLLEXPORT double _simGetJointPosition_D(const void* joint)
{
    return(_simGetJointPosition_internal(joint));
}
SIM_DLLEXPORT void _simSetDynamicMotorPositionControlTargetPosition_D(const void* joint,double pos)
{
    return(_simSetDynamicMotorPositionControlTargetPosition_internal(joint,pos));
}
SIM_DLLEXPORT void _simGetInitialDynamicVelocity_D(const void* shape,double* vel)
{
    return(_simGetInitialDynamicVelocity_internal(shape,vel));
}
SIM_DLLEXPORT void _simSetInitialDynamicVelocity_D(void* shape,const double* vel)
{
    return(_simSetInitialDynamicVelocity_internal(shape,vel));
}
SIM_DLLEXPORT void _simGetInitialDynamicAngVelocity_D(const void* shape,double* angularVel)
{
    return(_simGetInitialDynamicAngVelocity_internal(shape,angularVel));
}
SIM_DLLEXPORT void _simSetInitialDynamicAngVelocity_D(void* shape,const double* angularVel)
{
    return(_simSetInitialDynamicAngVelocity_internal(shape,angularVel));
}
SIM_DLLEXPORT void _simSetShapeDynamicVelocity_D(void* shape,const double* linear,const double* angular,double simTime)
{
    return(_simSetShapeDynamicVelocity_internal(shape,linear,angular,simTime));
}
SIM_DLLEXPORT void _simGetAdditionalForceAndTorque_D(const void* shape,double* force,double* torque)
{
    return(_simGetAdditionalForceAndTorque_internal(shape,force,torque));
}
SIM_DLLEXPORT bool _simGetJointPositionInterval_D(const void* joint,double* minValue,double* rangeValue)
{
    return(_simGetJointPositionInterval_internal(joint,minValue,rangeValue));
}
SIM_DLLEXPORT double _simGetDynamicMotorTargetPosition_D(const void* joint)
{
    return(_simGetDynamicMotorTargetPosition_internal(joint));
}
SIM_DLLEXPORT double _simGetDynamicMotorTargetVelocity_D(const void* joint)
{
    return(_simGetDynamicMotorTargetVelocity_internal(joint));
}
SIM_DLLEXPORT double _simGetDynamicMotorMaxForce_D(const void* joint)
{
    return(_simGetDynamicMotorMaxForce_internal(joint));
}
SIM_DLLEXPORT double _simGetDynamicMotorUpperLimitVelocity_D(const void* joint)
{
    return(_simGetDynamicMotorUpperLimitVelocity_internal(joint));
}
SIM_DLLEXPORT void _simSetDynamicMotorReflectedPositionFromDynamicEngine_D(void* joint,double pos,double simTime)
{
    return(_simSetDynamicMotorReflectedPositionFromDynamicEngine_internal(joint,pos,simTime));
}
SIM_DLLEXPORT void _simSetJointSphericalTransformation_D(void* joint,const double* quat,double simTime)
{
    return(_simSetJointSphericalTransformation_internal(joint,quat,simTime));
}
SIM_DLLEXPORT void _simAddForceSensorCumulativeForcesAndTorques_D(void* forceSensor,const double* force,const double* torque,int totalPassesCount,double simTime)
{
    return(_simAddForceSensorCumulativeForcesAndTorques_internal(forceSensor,force,torque,totalPassesCount,simTime));
}
SIM_DLLEXPORT void _simAddJointCumulativeForcesOrTorques_D(void* joint,double forceOrTorque,int totalPassesCount,double simTime)
{
    return(_simAddJointCumulativeForcesOrTorques_internal(joint,forceOrTorque,totalPassesCount,simTime));
}
SIM_DLLEXPORT double _simGetLocalInertiaInfo_D(const void* object,double* pos,double* quat,double* diagI)
{
    return(_simGetLocalInertiaInfo_internal(object,pos,quat,diagI));
}
SIM_DLLEXPORT double _simGetMass_D(const void* geomInfo)
{
    return(_simGetMass_internal(geomInfo));
}
SIM_DLLEXPORT void _simGetPurePrimitiveSizes_D(const void* geometric,double* sizes)
{
    return(_simGetPurePrimitiveSizes_internal(geometric,sizes));
}
SIM_DLLEXPORT void _simGetVerticesLocalFrame_D(const void* geometric,double* pos,double* quat)
{
    return(_simGetVerticesLocalFrame_internal(geometric,pos,quat));
}
SIM_DLLEXPORT const double* _simGetHeightfieldData_D(const void* geometric,int* xCount,int* yCount,double* minHeight,double* maxHeight)
{
    return(_simGetHeightfieldData_internal(geometric,xCount,yCount,minHeight,maxHeight));
}
SIM_DLLEXPORT void _simGetCumulativeMeshes_D(const void* geomInfo,double** vertices,int* verticesSize,int** indices,int* indicesSize)
{
    return(_simGetCumulativeMeshes_internal(geomInfo,vertices,verticesSize,indices,indicesSize));
}
SIM_DLLEXPORT void _simGetGravity_D(double* gravity)
{
    return(_simGetGravity_internal(gravity));
}
SIM_DLLEXPORT bool _simGetDistanceBetweenEntitiesIfSmaller_D(int entity1ID,int entity2ID,double* distance,double* ray,int* cacheBuffer,bool overrideMeasurableFlagIfNonCollection1,bool overrideMeasurableFlagIfNonCollection2,bool pathPlanningRoutineCalling)
{
    return(_simGetDistanceBetweenEntitiesIfSmaller_internal(entity1ID,entity2ID,distance,ray,cacheBuffer,overrideMeasurableFlagIfNonCollection1,overrideMeasurableFlagIfNonCollection2,pathPlanningRoutineCalling));
}
SIM_DLLEXPORT int _simHandleJointControl_D(const void* joint,int auxV,const int* inputValuesInt,const double* inputValuesFloat,double* outputValues)
{
    return(_simHandleJointControl_internal(joint,auxV,inputValuesInt,inputValuesFloat,outputValues));
}
SIM_DLLEXPORT int _simHandleCustomContact_D(int objHandle1,int objHandle2,int engine,int* dataInt,double* dataFloat)
{
    return(_simHandleCustomContact_internal(objHandle1,objHandle2,engine,dataInt,dataFloat));
}
SIM_DLLEXPORT double _simGetPureHollowScaling_D(const void* geometric)
{
    return(_simGetPureHollowScaling_internal(geometric));
}
SIM_DLLEXPORT void _simDynCallback_D(const int* intData,const double* floatData)
{
    _simDynCallback_internal(intData,floatData);
}

#include <sim-old.cpp>
