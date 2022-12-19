int simGetMaterialId_internal(const char* materialName)
{ // DEPRECATED since 29/10/2016.
    TRACE_C_API;
    // For backward compatibility (28/10/2016)
    // We now do not share materials anymore: each shape has its own material, so
    // the material of a shape is identified by the shape handle itself
    return(-1);
}

int simGetShapeMaterial_internal(int shapeHandle)
{ // DEPRECATED since 29/10/2016.
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1; // means error
        if (isShape(__func__,shapeHandle))
        { // since 28/10/2016 there is no more sharing of materials. So each shape has an individual material.
          // Here we return simply the shape handle, which can be used for simSetShapeMaterial!
            retVal=shapeHandle;
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simHandleVarious_internal()
{ // DEPRECATED since 29/10/2016. use simHandleSimulationStart and simHandleSensingStart instead!
    // HandleVarious should be the last function call before the sensing phase
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        // Following is for camera tracking!
        for (size_t i=0;i<App::currentWorld->sceneObjects->getCameraCount();i++)
        {
            CCamera*  it=App::currentWorld->sceneObjects->getCameraFromIndex(i);
            it->handleCameraTracking();
        }

        // Following is for velocity measurement:
        double dt=App::currentWorld->simulation->getTimeStep();
        double t=dt+App::currentWorld->simulation->getSimulationTime();
        for (size_t i=0;i<App::currentWorld->sceneObjects->getJointCount();i++)
            App::currentWorld->sceneObjects->getJointFromIndex(i)->measureJointVelocity(t);
        for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
            App::currentWorld->sceneObjects->getObjectFromIndex(i)->measureVelocity(dt);

        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetMpConfigForTipPose_internal(int motionPlanningObjectHandle,int options,double closeNodesDistance,int trialCount,const double* tipPose,int maxTimeInMs,double* outputJointPositions,const double* referenceConfigs,int referenceConfigCount,const double* jointWeights,const int* jointBehaviour,int correctionPasses)
{ // DEPRECATED since 21/1/2016   referenceConfigs can be nullptr, as well as jointWeights
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(-1);
}

int simResetPath_internal(int pathHandle)
{ // DEPRECATED
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (pathHandle!=sim_handle_all)&&(pathHandle!=sim_handle_all_except_explicit) )
        {
            if (!isPath(__func__,pathHandle))
            {
                return(-1);
            }
        }
        if (pathHandle>=0)
        { // Explicit handling
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(pathHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->resetPath();
        }
        else
        {
            for (size_t i=0;i<App::currentWorld->sceneObjects->getPathCount();i++)
            {
                CPath_old* p=App::currentWorld->sceneObjects->getPathFromIndex(i);
                if ( (pathHandle==sim_handle_all)||(!p->getExplicitHandling()) )
                    p->resetPath();
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simHandlePath_internal(int pathHandle,double deltaTime)
{ // DEPRECATED
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (pathHandle!=sim_handle_all)&&(pathHandle!=sim_handle_all_except_explicit) )
        {
            if (!isPath(__func__,pathHandle))
            {
                return(-1);
            }
        }
        if (pathHandle>=0)
        { // explicit handling
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(pathHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->handlePath(deltaTime);
        }
        else
        {
            for (size_t i=0;i<App::currentWorld->sceneObjects->getPathCount();i++)
            {
                CPath_old* p=App::currentWorld->sceneObjects->getPathFromIndex(i);
                if ( (pathHandle==sim_handle_all)||(!p->getExplicitHandling()) )
                    p->handlePath(deltaTime);
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simResetJoint_internal(int jointHandle)
{ // DEPRECATED
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (jointHandle!=sim_handle_all)&&(jointHandle!=sim_handle_all_except_explicit) )
        {
            if (!isJoint(__func__,jointHandle))
                return(-1);
        }
        if (jointHandle>=0)
        { // Explicit handling
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
            if (!it->getExplicitHandling_DEPRECATED())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->resetJoint_DEPRECATED();
        }
        else
        {
            for (size_t i=0;i<App::currentWorld->sceneObjects->getJointCount();i++)
            {
                CJoint* p=App::currentWorld->sceneObjects->getJointFromIndex(i);
                if ( (jointHandle==sim_handle_all)||(!p->getExplicitHandling_DEPRECATED()) )
                    p->resetJoint_DEPRECATED();
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simHandleJoint_internal(int jointHandle,double deltaTime)
{ // DEPRECATED
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (jointHandle!=sim_handle_all)&&(jointHandle!=sim_handle_all_except_explicit) )
        {
            if (!isJoint(__func__,jointHandle))
                return(-1);
        }
        if (jointHandle>=0)
        { // explicit handling
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
            if (!it->getExplicitHandling_DEPRECATED())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->handleJoint_DEPRECATED(deltaTime);
        }
        else
        {
            for (size_t i=0;i<App::currentWorld->sceneObjects->getJointCount();i++)
            {
                CJoint* p=App::currentWorld->sceneObjects->getJointFromIndex(i);
                if ( (jointHandle==sim_handle_all)||(!p->getExplicitHandling_DEPRECATED()) )
                    p->handleJoint_DEPRECATED(deltaTime);
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetPathPlanningHandle_internal(const char* pathPlanningObjectName)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;

    std::string pathPlanningObjectNameAdjusted=getIndexAdjustedObjectName(pathPlanningObjectName);
    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPathPlanningTask* it=App::currentWorld->pathPlanning->getObject(pathPlanningObjectNameAdjusted);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_PLANNING_OBJECT_INEXISTANT);
            return(-1);
        }
        int retVal=it->getObjectID();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetMotionPlanningHandle_internal(const char* motionPlanningObjectName)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(-1);
}

double* simFindMpPath_internal(int motionPlanningObjectHandle,const double* startConfig,const double* goalConfig,int options,double stepSize,int* outputConfigsCnt,int maxTimeInMs,double* reserved,const int* auxIntParams,const double* auxFloatParams)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(nullptr);
}

double* simSimplifyMpPath_internal(int motionPlanningObjectHandle,const double* pathBuffer,int configCnt,int options,double stepSize,int increment,int* outputConfigsCnt,int maxTimeInMs,double* reserved,const int* auxIntParams,const double* auxFloatParams)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(nullptr);
}

double* simFindIkPath_internal(int motionPlanningObjectHandle,const double* startConfig,const double* goalPose,int options,double stepSize,int* outputConfigsCnt,double* reserved,const int* auxIntParams,const double* auxFloatParams)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(nullptr);
}

double* simGetMpConfigTransition_internal(int motionPlanningObjectHandle,const double* startConfig,const double* goalConfig,int options,const int* select,double calcStepSize,double maxOutStepSize,int wayPointCnt,const double* wayPoints,int* outputConfigsCnt,const int* auxIntParams,const double* auxFloatParams)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(nullptr);
}

int simCreateMotionPlanning_internal(int jointCnt,const int* jointHandles,const int* jointRangeSubdivisions,const double* jointMetricWeights,int options,const int* intParams,const double* floatParams,const void* reserved)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(-1);
}

int simRemoveMotionPlanning_internal(int motionPlanningHandle)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(-1);
}

int simSearchPath_internal(int pathPlanningObjectHandle,double maximumSearchTime)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPathPlanningTask* it=App::currentWorld->pathPlanning->getObject(pathPlanningObjectHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_PLANNING_OBJECT_INEXISTANT);
            return(-1);
        }
        int retVal=0;
        if (it->performSearch(false,maximumSearchTime))
            retVal=1;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


int simInitializePathSearch_internal(int pathPlanningObjectHandle,double maximumSearchTime,double searchTimeStep)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPathPlanningTask* it=App::currentWorld->pathPlanning->getObject(pathPlanningObjectHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_PLANNING_OBJECT_INEXISTANT);
            return(-1);
        }
        if (App::currentWorld->pathPlanning->getTemporaryPathSearchObjectCount()>100)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_TOO_MANY_TEMP_OBJECTS);
            return(-1);
        }

        maximumSearchTime=tt::getLimitedFloat(0.01,36000.0,maximumSearchTime);
        searchTimeStep=tt::getLimitedFloat(0.001,std::min<double>(1.0,maximumSearchTime),searchTimeStep);
        CPathPlanningTask* oldIt=it;
        it=oldIt->copyYourself(); // we copy it because the original might be destroyed at any time
        it->setOriginalTask(oldIt);
        int retVal=-1; // error
        if (it->initiateSteppedSearch(false,maximumSearchTime,searchTimeStep))
            retVal=App::currentWorld->pathPlanning->addTemporaryPathSearchObject(it);
        else
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_PLANNING_OBJECT_NOT_CONSISTENT);
            delete it;
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simPerformPathSearchStep_internal(int temporaryPathSearchObject,bool abortSearch)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPathPlanningTask* it=App::currentWorld->pathPlanning->getTemporaryPathSearchObject(temporaryPathSearchObject);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_TEMP_PATH_SEARCH_OBJECT_INEXISTANT);
            return(-1);
        }
        if (abortSearch)
        {
            App::currentWorld->pathPlanning->removeTemporaryPathSearchObjectButDontDestroyIt(it);
            delete it;
            return(0);
        }
        int retVal=it->performSteppedSearch();
        if (retVal!=-2)
        {
            App::currentWorld->pathPlanning->removeTemporaryPathSearchObjectButDontDestroyIt(it);
            CPathPlanningTask* originalIt=it->getOriginalTask();
            int tree1Handle,tree2Handle;
            it->getAndDisconnectSearchTrees(tree1Handle,tree2Handle); // to keep trees visible!
            delete it;
            // Now we connect the trees only if the originalTask still exists:
            bool found=false;
            for (int ot=0;ot<int(App::currentWorld->pathPlanning->allObjects.size());ot++)
            {
                if (App::currentWorld->pathPlanning->allObjects[ot]==originalIt)
                {
                    found=true;
                    break;
                }
            }
            if (found)
                originalIt->connectExternalSearchTrees(tree1Handle,tree2Handle);
            else
            {
                App::currentWorld->drawingCont->removeObject(tree1Handle);
                App::currentWorld->drawingCont->removeObject(tree2Handle);
            }
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simLockInterface_internal(bool locked)
{ // DEPRECATED since release 3.1.0
    return(0);
}

int simCopyPasteSelectedObjects_internal()
{ // deprecated since 3.1.3
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
            return(-1);
        std::vector<int> sel;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        if (sel.size()>0)
        {
            if (fullModelCopyFromApi)
                CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
            App::worldContainer->copyBuffer->memorizeBuffer();
            App::worldContainer->copyBuffer->copyCurrentSelection(&sel,App::currentWorld->environment->getSceneLocked(),0);
            App::currentWorld->sceneObjects->deselectObjects();
            App::worldContainer->copyBuffer->pasteBuffer(App::currentWorld->environment->getSceneLocked(),3);
            App::worldContainer->copyBuffer->restoreBuffer();
            App::worldContainer->copyBuffer->clearMemorizedBuffer();
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simSerialPortOpen_internal(int portNumber,int baudRate,void* reserved1,void* reserved2)
{ // deprecated (10/04/2012)
    TRACE_C_API;
    if (!isSimulatorInitialized(__func__))
        return(-1);
#ifdef SIM_WITH_SERIAL
    if (App::worldContainer->serialPortContainer->serialPortOpen_old(false,portNumber,baudRate))
        return(1);
#endif
    return(-1);
}

int simSerialPortClose_internal(int portNumber)
{ // deprecated (10/04/2012)
    TRACE_C_API;
    if (!isSimulatorInitialized(__func__))
        return(-1);
#ifdef SIM_WITH_SERIAL
    if (App::worldContainer->serialPortContainer->serialPortClose_old(portNumber))
        return(1);
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PORT_NOT_OPEN);
#endif
    return(-1);
}

int simSerialPortSend_internal(int portNumber,const char* data,int dataLength)
{ // deprecated (10/04/2012)
    TRACE_C_API;
    if (!isSimulatorInitialized(__func__))
        return(-1);
    int retVal=-1;
#ifdef SIM_WITH_SERIAL
    retVal=App::worldContainer->serialPortContainer->serialPortSend_old(portNumber,data,dataLength);
    if (retVal==-1)
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PORT_NOT_OPEN);
#endif
    return(retVal);
}

int simSerialPortRead_internal(int portNumber,char* buffer,int dataLengthToRead)
{ // deprecated (10/04/2012)
    TRACE_C_API;
    if (!isSimulatorInitialized(__func__))
        return(-1);
    int retVal=-1;
#ifdef SIM_WITH_SERIAL
    retVal=App::worldContainer->serialPortContainer->serialPortReceive_old(portNumber,buffer,dataLengthToRead);
    if (retVal==-1)
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PORT_NOT_OPEN);
#endif
    return(retVal);
}

int simAppendScriptArrayEntry_internal(const char* reservedSetToNull,int scriptHandleOrType,const char* arrayNameAtScriptName,const char* keyName,const char* data,const int* what)
{ // deprecated (23/02/2016)
    TRACE_C_API;
    CScriptObject* script=nullptr;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::string arrayName;
        if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
        { // script is identified by its ID
            std::string arrNameAtScriptName(arrayNameAtScriptName);
            size_t p=arrNameAtScriptName.find('@');
            if (p!=std::string::npos)
                arrayName.assign(arrNameAtScriptName.begin(),arrNameAtScriptName.begin()+p);
            else
                arrayName=arrNameAtScriptName;
            script=App::worldContainer->getScriptFromHandle(scriptHandleOrType);
        }
        else
        {
            if (reservedSetToNull==nullptr)
            { // this can be the old or new way of doing it (the old way was active only 2 months, and not officially):
                std::string scriptName;
                std::string arrNameAtScriptName(arrayNameAtScriptName);
                size_t p=arrNameAtScriptName.find('@');
                if (p!=std::string::npos)
                {
                    scriptName.assign(arrNameAtScriptName.begin()+p+1,arrNameAtScriptName.end());
                    arrayName.assign(arrNameAtScriptName.begin(),arrNameAtScriptName.begin()+p);
                }
                else
                    arrayName=arrNameAtScriptName;

                if (scriptHandleOrType==sim_scripttype_mainscript) // new and old way (same coding)
                    script=App::currentWorld->embeddedScriptContainer->getMainScript();
                if (scriptHandleOrType==sim_scripttype_addonscript)
                {
                    if (scriptName.size()>0)
                        script=App::worldContainer->addOnScriptContainer->getAddOnFromName(scriptName.c_str());
                }
                if (scriptHandleOrType==sim_scripttype_sandboxscript)
                    script=App::worldContainer->sandboxScript;
                if (scriptHandleOrType==sim_scripttype_childscript)
                {
                    if (scriptName.size()>0)
                    { // new way
                        int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(scriptName.c_str());
                        script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objId);
                    }
                }
                if (scriptHandleOrType==sim_scripttype_customizationscript)
                { // new way only possible (6 was not available in the old way)
                    int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(scriptName.c_str());
                    script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objId);
                }
            }
            else
            { // this is the old way of doing it. Deprecated. Was only 2 months active, not officially
                arrayName=arrayNameAtScriptName;
                if (scriptHandleOrType==0) // main script
                    script=App::currentWorld->embeddedScriptContainer->getMainScript();
                if (scriptHandleOrType==3) // child script
                {
                    int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(reservedSetToNull);
                    script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objId);
                }
                if (scriptHandleOrType==5) // customization
                {
                    int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(reservedSetToNull);
                    script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objId);
                }
            }
        }

        if (script!=nullptr)
        {
            int retVal=script->appendTableEntry_DEPRECATED(arrayName.c_str(),keyName,data,what);
            if (retVal==-1)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OPERATION_FAILED);
            return(retVal);
        }

        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simClearScriptVariable_internal(const char* reservedSetToNull,int scriptHandleOrType,const char* variableNameAtScriptName)
{ // DEPRECATED (23/02/2016)
    TRACE_C_API;
    CScriptObject* script=nullptr;

    std::string variableName;
    if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
    { // script is identified by its ID
        std::string varNameAtScriptName(variableNameAtScriptName);
        size_t p=varNameAtScriptName.find('@');
        if (p!=std::string::npos)
            variableName.assign(varNameAtScriptName.begin(),varNameAtScriptName.begin()+p);
        else
            variableName=varNameAtScriptName;
        script=App::worldContainer->getScriptFromHandle(scriptHandleOrType);
    }
    else
    {
        if (reservedSetToNull==nullptr)
        { // this can be the old or new way of doing it (the old way was active only 2 months, and not officially):
            std::string scriptName;
            std::string varNameAtScriptName(variableNameAtScriptName);
            size_t p=varNameAtScriptName.find('@');
            if (p!=std::string::npos)
            {
                scriptName.assign(varNameAtScriptName.begin()+p+1,varNameAtScriptName.end());
                variableName.assign(varNameAtScriptName.begin(),varNameAtScriptName.begin()+p);
            }
            else
                variableName=varNameAtScriptName;

            if (scriptHandleOrType==sim_scripttype_mainscript) // new and old way (same coding)
                script=App::currentWorld->embeddedScriptContainer->getMainScript();
            if (scriptHandleOrType==sim_scripttype_addonscript)
            {
                if (scriptName.size()>0)
                    script=App::worldContainer->addOnScriptContainer->getAddOnFromName(scriptName.c_str());
            }
            if (scriptHandleOrType==sim_scripttype_sandboxscript)
                script=App::worldContainer->sandboxScript;
            if (scriptHandleOrType==sim_scripttype_childscript)
            {
                if (scriptName.size()>0)
                { // new way
                    int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(scriptName.c_str());
                    script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objId);
                }
            }
            if (scriptHandleOrType==sim_scripttype_customizationscript)
            { // new way only possible (6 was not available in the old way)
                int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(scriptName.c_str());
                script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objId);
            }
        }
        else
        { // this is the old way of doing it. Deprecated. Was only 2 months active, not officially
            variableName=variableNameAtScriptName;
            if (scriptHandleOrType==0) // main script
                script=App::currentWorld->embeddedScriptContainer->getMainScript();
            if (scriptHandleOrType==3) // child script
            {
                int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(reservedSetToNull);
                script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objId);
            }
            if (scriptHandleOrType==5) // customization
            {
                int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(reservedSetToNull);
                script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objId);
            }
        }
    }

    if (script!=nullptr)
    {
        int retVal=script->clearScriptVariable_DEPRECATED(variableName.c_str());
        if (retVal==-1)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OPERATION_FAILED);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);

    return(-1);
}

void _simGetVortexParameters_internal(const void* object,int version,double* floatParams,int* intParams)
{ // if object is nullptr, we return general engine settings, if object is a shape, we return shape settings, otherwise joint settings
    // Version allows to adjust for future extensions.
    TRACE_C_API;
    std::vector<double> fparams;
    std::vector<int> iparams;
    int icnt=0;
    int fcnt=0;
    if (object==nullptr)
    {
        App::currentWorld->dynamicsContainer->getVortexFloatParams(fparams);
        App::currentWorld->dynamicsContainer->getVortexIntParams(iparams);
        if (version==0)
        {
            fcnt=10;
            icnt=1;
        }
        if (version==1)
        {
            fcnt=10;
            icnt=1;
        }
        if (version==2)
        {
            fcnt=10;
            icnt=1;
        }
        if (version>=3)
        { // when the dynamics plugin is the same version as CoppeliaSim, or newer!
            fcnt=10;
            icnt=1;
        }
    }
    else
    {
        CSceneObject* obj=(CSceneObject*)object;
        if (obj->getObjectType()==sim_object_shape_type)
        {
            CShape* shape=(CShape*)object;
            CDynMaterialObject* mat=shape->getDynMaterial();
            mat->getVortexFloatParams(fparams);
            mat->getVortexIntParams(iparams);
            if (version==0)
            {
                fcnt=32;
                icnt=8;
            }
            if (version==1)
            {
                fcnt=33;
                icnt=8;
            }
            if (version==2)
            {
                fcnt=36;
                icnt=8;
            }
            if (version>=3)
            { // when the dynamics plugin is the same version as CoppeliaSim, or newer!
                fcnt=36;
                icnt=8;
            }
        }
        if (obj->getObjectType()==sim_object_joint_type)
        {
            CJoint* joint=(CJoint*)object;
            joint->getVortexFloatParams(fparams);
            joint->getVortexIntParams(iparams);
            if (version==0)
            {
                fcnt=47;
                icnt=4;
            }
            if (version==1)
            {
                fcnt=47;
                icnt=4;
            }
            if (version==2)
            {
                fcnt=47;
                icnt=4;
            }
            if (version>=3)
            { // when the dynamics plugin is the same version as CoppeliaSim, or newer!
                fcnt=51;
                icnt=7;
            }
        }
    }
    for (int i=0;i<fcnt;i++)
        floatParams[i]=fparams[i];
    for (int i=0;i<icnt;i++)
        intParams[i]=iparams[i];
}

void _simGetNewtonParameters_internal(const void* object,int* version,double* floatParams,int* intParams)
{ // if object is nullptr, we return general engine settings, if object is a shape, we return shape settings, otherwise joint settings
    // Version allows to adjust for future extensions.
    TRACE_C_API;
    std::vector<double> fparams;
    std::vector<int> iparams;
    int icnt=0;
    int fcnt=0;
    if (object==nullptr)
    {
        App::currentWorld->dynamicsContainer->getNewtonFloatParams(fparams);
        App::currentWorld->dynamicsContainer->getNewtonIntParams(iparams);
        if (version[0]>=0)
        { // when the dynamics plugin is the same version as CoppeliaSim, or newer!
            fcnt=2;
            icnt=2;
        }
        version[0]=0;
    }
    else
    {
        CSceneObject* obj=(CSceneObject*)object;
        if (obj->getObjectType()==sim_object_shape_type)
        {
            CShape* shape=(CShape*)object;
            CDynMaterialObject* mat=shape->getDynMaterial();
            mat->getNewtonFloatParams(fparams);
            mat->getNewtonIntParams(iparams);
            if (version[0]>=0)
            { // when the dynamics plugin is the same version as CoppeliaSim, or newer!
                fcnt=5;
                icnt=1;
            }
            version[0]=0;
        }
        if (obj->getObjectType()==sim_object_joint_type)
        {
            CJoint* joint=(CJoint*)object;
            joint->getNewtonFloatParams(fparams);
            joint->getNewtonIntParams(iparams);
            if (version[0]>=0)
            { // when the dynamics plugin is the same version as CoppeliaSim, or newer!
                fcnt=2;
                icnt=2;
            }
            version[0]=0;
        }
    }
    for (int i=0;i<fcnt;i++)
        floatParams[i]=fparams[i];
    for (int i=0;i<icnt;i++)
        intParams[i]=iparams[i];
}

void _simGetJointOdeParameters_internal(const void* joint,double* stopERP,double* stopCFM,double* bounce,double* fudge,double* normalCFM)
{
    TRACE_C_API;
    stopERP[0]=((CJoint*)joint)->getEngineFloatParam(sim_ode_joint_stoperp,nullptr);
    stopCFM[0]=((CJoint*)joint)->getEngineFloatParam(sim_ode_joint_stopcfm,nullptr);
    bounce[0]=((CJoint*)joint)->getEngineFloatParam(sim_ode_joint_bounce,nullptr);
    fudge[0]=((CJoint*)joint)->getEngineFloatParam(sim_ode_joint_fudgefactor,nullptr);
    normalCFM[0]=((CJoint*)joint)->getEngineFloatParam(sim_ode_joint_normalcfm,nullptr);
}

void _simGetJointBulletParameters_internal(const void* joint,double* stopERP,double* stopCFM,double* normalCFM)
{
    TRACE_C_API;
    stopERP[0]=((CJoint*)joint)->getEngineFloatParam(sim_bullet_joint_stoperp,nullptr);
    stopCFM[0]=((CJoint*)joint)->getEngineFloatParam(sim_bullet_joint_stopcfm,nullptr);
    normalCFM[0]=((CJoint*)joint)->getEngineFloatParam(sim_bullet_joint_normalcfm,nullptr);
}

CShape* __getShapeFromGeomInfo(const void* geomInfo)
{
    for (size_t i=0;i<App::currentWorld->sceneObjects->getShapeCount();i++)
    {
        CShape* sh=App::currentWorld->sceneObjects->getShapeFromIndex(i);
        if (sh->getMeshWrapper()==(CMeshWrapper*)geomInfo)
            return(sh);
    }
    return(nullptr);
}

void _simGetOdeMaxContactFrictionCFMandERP_internal(const void* geomInfo,int* maxContacts,double* friction,double* cfm,double* erp)
{
    TRACE_C_API;
    CShape* shape=__getShapeFromGeomInfo(geomInfo);
    CDynMaterialObject* mat=shape->getDynMaterial();
    maxContacts[0]=mat->getEngineIntParam(sim_ode_body_maxcontacts,nullptr);
    friction[0]=mat->getEngineFloatParam(sim_ode_body_friction,nullptr);
    cfm[0]=mat->getEngineFloatParam(sim_ode_body_softcfm,nullptr);
    erp[0]=mat->getEngineFloatParam(sim_ode_body_softerp,nullptr);
}

bool _simGetBulletCollisionMargin_internal(const void* geomInfo,double* margin,int* otherProp)
{
    TRACE_C_API;
    CMeshWrapper* geomWrap=(CMeshWrapper*)geomInfo;
    CShape* shape=__getShapeFromGeomInfo(geomInfo);
    CDynMaterialObject* mat=shape->getDynMaterial();
    bool convexAndNotPure=(geomWrap->isConvex()&&(!geomWrap->isPure()));
    if (convexAndNotPure)
        margin[0]=mat->getEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactorconvex,nullptr);
    else
        margin[0]=mat->getEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactor,nullptr);

    if (otherProp!=nullptr)
    {
        otherProp[0]=0;
        if (mat->getEngineBoolParam(sim_bullet_body_autoshrinkconvex,nullptr))
            otherProp[0]|=1;
    }
    bool retVal=false;
    if (convexAndNotPure)
    {
        if (mat->getEngineBoolParam(sim_bullet_body_usenondefaultcollisionmarginconvex,nullptr))
            retVal=true;
    }
    else
    {
        if (mat->getEngineBoolParam(sim_bullet_body_usenondefaultcollisionmargin,nullptr))
            retVal=true;
    }
    return(retVal);
}

bool _simGetBulletStickyContact_internal(const void* geomInfo)
{
    TRACE_C_API;
    CShape* shape=__getShapeFromGeomInfo(geomInfo);
    CDynMaterialObject* mat=shape->getDynMaterial();
    return(mat->getEngineBoolParam(sim_bullet_body_sticky,nullptr));
}

double _simGetBulletRestitution_internal(const void* geomInfo)
{
    TRACE_C_API;
    CShape* shape=__getShapeFromGeomInfo(geomInfo);
    CDynMaterialObject* mat=shape->getDynMaterial();
    return(mat->getEngineFloatParam(sim_bullet_body_restitution,nullptr));
}

void _simGetDamping_internal(const void* geomInfo,double* linDamping,double* angDamping)
{
    TRACE_C_API;
    CShape* shape=__getShapeFromGeomInfo(geomInfo);
    CDynMaterialObject* mat=shape->getDynMaterial();

    int eng=App::currentWorld->dynamicsContainer->getDynamicEngineType(nullptr);
    if (eng==sim_physics_bullet)
    {
        linDamping[0]=mat->getEngineFloatParam(sim_bullet_body_lineardamping,nullptr);
        angDamping[0]=mat->getEngineFloatParam(sim_bullet_body_angulardamping,nullptr);
    }
    if (eng==sim_physics_ode)
    {
        linDamping[0]=mat->getEngineFloatParam(sim_ode_body_lineardamping,nullptr);
        angDamping[0]=mat->getEngineFloatParam(sim_ode_body_angulardamping,nullptr);
    }
}

double _simGetFriction_internal(const void* geomInfo)
{
    TRACE_C_API;
    CShape* shape=__getShapeFromGeomInfo(geomInfo);
    CDynMaterialObject* mat=shape->getDynMaterial();

    int eng=App::currentWorld->dynamicsContainer->getDynamicEngineType(nullptr);
    if (eng==sim_physics_bullet)
        return(mat->getEngineFloatParam(sim_bullet_body_oldfriction,nullptr));
    if (eng==sim_physics_ode)
        return(mat->getEngineFloatParam(sim_ode_body_friction,nullptr));
    return(0.0);
}

int simAddSceneCustomData_internal(int header,const char* data,int dataLength)
{ // deprecated
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::string hh("_oldSceneCustomData_");
        hh+=std::to_string(header);
        hh+="_";
        App::currentWorld->customSceneData.setData(hh.c_str(),data,dataLength);
        // ---------------------- Old -----------------------------
        App::currentWorld->customSceneData_old->setData(header,data,dataLength);
        // ---------------------- Old -----------------------------
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simGetSceneCustomDataLength_internal(int header)
{ // deprecated
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::string hh("_oldSceneCustomData_");
        hh+=std::to_string(header);
        hh+="_";
        std::string data=App::currentWorld->customSceneData.getData(hh.c_str());
        int retVal=int(data.size());
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetSceneCustomData_internal(int header,char* data)
{ // deprecated
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::string hh("_oldSceneCustomData_");
        hh+=std::to_string(header);
        hh+="_";
        std::string dat=App::currentWorld->customSceneData.getData(hh.c_str());
        for (size_t i=0;i<dat.size();i++)
            data[i]=dat[i];
        App::currentWorld->customSceneData_old->getData(header,data);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}



int simAddObjectCustomData_internal(int objectHandle,int header,const char* data,int dataLength)
{ // deprecated
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        std::string hh("_oldObjectCustomData_");
        hh+=std::to_string(header);
        hh+="_";
        it->writeCustomDataBlock(false,hh.c_str(),data,dataLength);
        // ---------------------- Old -----------------------------
        it->setObjectCustomData_old(header,data,dataLength);
        // ---------------------- Old -----------------------------
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simGetObjectCustomDataLength_internal(int objectHandle,int header)
{ // deprecated
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        std::string hh("_oldObjectCustomData_");
        hh+=std::to_string(header);
        hh+="_";
        std::string data=it->readCustomDataBlock(false,hh.c_str());
        int retVal=int(data.size());
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetObjectCustomData_internal(int objectHandle,int header,char* data)
{ // deprecated
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        std::string hh("_oldObjectCustomData_");
        hh+=std::to_string(header);
        hh+="_";
        std::string dat=it->readCustomDataBlock(false,hh.c_str());
        for (size_t i=0;i<dat.size();i++)
            data[i]=dat[i];
        App::currentWorld->customSceneData_old->getData(header,data);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simLoadUI_internal(const char* filename,int maxCount,int* uiHandles)
{
    TRACE_C_API;
    return(-1);
}

int simCreateUI_internal(const char* elementName,int menuAttributes,const int* clientSize,const int* cellSize,int* buttonHandles)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int s[2]={clientSize[0],clientSize[1]};
        if (menuAttributes!=0)
            s[1]++;
        int b=0;
        for (int i=0;i<8;i++)
        {
            if (menuAttributes&(1<<i))
                b++;
        }
        if (b>s[0])
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_DIMENSIONS);
            return(-1);
        }
        CButtonBlock* it=new CButtonBlock(s[0],s[1],cellSize[0],cellSize[1],-1);
        it->setBlockName(elementName);
        if ((menuAttributes&sim_ui_menu_systemblock)!=0)
            it->setAttributes(it->getAttributes()|sim_ui_property_systemblock);
        App::currentWorld->buttonBlockContainer->insertBlock(it,false);
        int retVal=it->getBlockID();
        int retHandlesP=0;
        float white[3]={1.0,1.0,1.0};
        if (menuAttributes&sim_ui_menu_title)
        { // We have a title bar:
            int p[2]={0,0};
            int s2[2]={s[0]-b+1,1};
            float blue[3]={0.36f,0.35f,0.87f};
            buttonHandles[retHandlesP]=simCreateUIButton_internal(retVal,p,s2,sim_buttonproperty_label|sim_buttonproperty_enabled|sim_buttonproperty_verticallycentered);
            simSetUIButtonColor_internal(retVal,buttonHandles[retHandlesP],blue,blue,white);
            retHandlesP++;
        }
        if (menuAttributes&sim_ui_menu_minimize)
        { // We have a minimize button:
            int p[2]={s[0]-b,0};
            if (menuAttributes&sim_ui_menu_title)
                p[0]++;
            int s2[2]={1,1};
            float blue[3]={0.18f,0.16f,0.84f};
            buttonHandles[retHandlesP]=simCreateUIButton_internal(retVal,p,s2,sim_buttonproperty_button|sim_buttonproperty_enabled|sim_buttonproperty_staydown|sim_buttonproperty_horizontallycentered|sim_buttonproperty_verticallycentered|sim_buttonproperty_isdown|sim_buttonproperty_rollupaction);
            simSetUIButtonColor_internal(retVal,buttonHandles[retHandlesP],blue,blue,white);
            simSetUIButtonLabel_internal(retVal,buttonHandles[retHandlesP],"&&fg999&&Square","&&fg999&&Minimize");
            it->setRollupMin(VPoint(0,0));
            it->setRollupMax(VPoint(s[0],0));
            retHandlesP++;
        }
        if (menuAttributes&sim_ui_menu_close)
        { // We have a close button:
            int p[2]={s[0]-1,0};
            int s2[2]={1,1};
            float red[3]={0.84f,0.16f,0.17f};
            buttonHandles[retHandlesP]=simCreateUIButton_internal(retVal,p,s2,sim_buttonproperty_button|sim_buttonproperty_enabled|sim_buttonproperty_horizontallycentered|sim_buttonproperty_verticallycentered|sim_buttonproperty_closeaction);
            simSetUIButtonColor_internal(retVal,buttonHandles[retHandlesP],red,red,white);
            simSetUIButtonLabel_internal(retVal,buttonHandles[retHandlesP],"&&fg999&&Check","");
            retHandlesP++;
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simCreateUIButton_internal(int elementHandle,const int* position,const int* size,int buttonProperty)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=new CSoftButton("",position[0],position[1],size[0],size[1]);
        if (!it->insertButton(but))
        {
            delete but;
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_WRONG_POS_SIZE_PARAMS);
            return(-1);
        }
        int retVal=but->buttonID;
        if (simSetUIButtonProperty_internal(elementHandle,retVal,buttonProperty)==-1)
            retVal=-1; // should not happen!
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simGetUIHandle_internal(const char* elementName)
{
    TRACE_C_API;

    std::string elementNameAdjusted=getIndexAdjustedObjectName(elementName);
    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithName(elementNameAdjusted);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_UI_INEXISTANT);
            return(-1);
        }
        int retVal=it->getBlockID();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetUIProperty_internal(int elementHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        int retVal=it->getAttributes();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetUIEventButton_internal(int elementHandle,int* auxiliaryValues)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
            return(-1);
        int retVal=-1;
#ifdef SIM_WITH_GUI
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        retVal=it->getLastEventButtonID(auxiliaryValues);
#endif
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetUIProperty_internal(int elementHandle,int elementProperty)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        // Following few new since 4/2/2013 (to bring newly made visible UI to the front)
        int attrib=it->getAttributes();
        it->setAttributes(elementProperty);
        int attribNew=it->getAttributes();
        if ( ((attrib&sim_ui_property_visible)==0)&&((attribNew&sim_ui_property_visible)!=0) )
            App::currentWorld->buttonBlockContainer->sendBlockToFront(it->getBlockID());
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetUIButtonSize_internal(int elementHandle,int buttonHandle,int* size)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        size[0]=but->getLength();
        size[1]=but->getHeight();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


int simGetUIButtonProperty_internal(int elementHandle,int buttonHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        int retVal=but->getAttributes();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetUIButtonProperty_internal(int elementHandle,int buttonHandle,int buttonProperty)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        but->setAttributes(buttonProperty);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetUIButtonLabel_internal(int elementHandle,int buttonHandle,const char* upStateLabel,const char* downStateLabel)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        if (upStateLabel!=nullptr)
            but->label=std::string(upStateLabel);
        if (downStateLabel!=nullptr)
            but->downLabel=std::string(downStateLabel);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

char* simGetUIButtonLabel_internal(int elementHandle,int buttonHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(nullptr);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        char* retVal=new char[but->label.length()+1];
        for (unsigned int i=0;i<but->label.length();i++)
            retVal[i]=but->label[i];
        retVal[but->label.length()]=0;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simSetUISlider_internal(int elementHandle,int buttonHandle,int position)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        if (but->getButtonType()!=sim_buttonproperty_slider)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_BUTTON_NOT_SLIDER);
            return(-1);
        }
        but->setSliderPos((double(position)/500.0)-1.0);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetUISlider_internal(int elementHandle,int buttonHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        if (but->getButtonType()!=sim_buttonproperty_slider)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_BUTTON_NOT_SLIDER);
            return(-1);
        }
        int retVal=int((but->getSliderPos()+1.0)*500.0);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetUIButtonColor_internal(int elementHandle,int buttonHandle,const float* upStateColor,const float* downStateColor,const float* labelColor)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        for (int i=0;i<3;i++)
        {
            if (upStateColor!=nullptr)
                but->backgroundColor[i]=upStateColor[i];
            if (downStateColor!=nullptr)
                but->downBackgroundColor[i]=downStateColor[i];
            if (labelColor!=nullptr)
                but->textColor[i]=labelColor[i];
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simRemoveUI_internal(int elementHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (elementHandle==sim_handle_all)
        {
            App::currentWorld->buttonBlockContainer->removeAllBlocks(false);
            return(1);
        }
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_UI_INEXISTANT);
            return(-1);
        }
        App::currentWorld->buttonBlockContainer->removeBlockFromID(elementHandle);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simCreateUIButtonArray_internal(int elementHandle,int buttonHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        but->enableArray(true);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simSetUIButtonArrayColor_internal(int elementHandle,int buttonHandle,const int* position,const float* color)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        if (!but->setArrayColor(position[0],position[1],color))
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OPERATION_FAILED);
            return(-1);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simDeleteUIButtonArray_internal(int elementHandle,int buttonHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        but->enableArray(false);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simSetUIButtonTexture_internal(int elementHandle,int buttonHandle,const int* size,const char* textureData)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        CTextureProperty* tp=but->getTextureProperty();
        if (tp!=nullptr)
        { // We already have a texture. Is it the same size/type? or do we wanna remove the texture anyway?
            int tob=tp->getTextureObjectID();
            bool remove=true;
            if ((tob>=SIM_IDSTART_TEXTURE)&&(tob<=SIM_IDSTART_TEXTURE)&&(size!=nullptr))
            { // we have the correct type (i.e. non-vision sensor)
                CTextureObject* to=App::currentWorld->textureContainer->getObject(tob);
                if (to!=nullptr)
                {
                    int sizeX,sizeY;
                    to->getTextureSize(sizeX,sizeY);
                    if ( (size[0]==sizeX)&&(size[1]==sizeY) )
                    { // we just need to actualize the texture content:
                        to->setImage(false,false,true,(unsigned char*)textureData);
                        remove=false;
                    }
                }
            }
            if (remove)
            {
                App::currentWorld->textureContainer->announceGeneralObjectWillBeErased(elementHandle,but->getUniqueID());
                delete tp;
                tp=nullptr;
                but->setTextureProperty(nullptr);
            }
        }
        if ((tp==nullptr)&&(size!=nullptr))
        { // add an existing texture
            CTextureObject* textureObj=new CTextureObject(size[0],size[1]);
            textureObj->setImage(false,false,true,(unsigned char*)textureData); // keep false,true
            textureObj->setObjectName("textureSetThroughAPI");
            textureObj->addDependentObject(it->getBlockID(),but->getUniqueID()); // Unique ID starts exceptionnally at 1
            int textureID=App::currentWorld->textureContainer->addObject(textureObj,false); // might erase the textureObj and return a similar object already present!!
            tp=new CTextureProperty(textureID);
            but->setTextureProperty(tp);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simSaveUI_internal(int count,const int* uiHandles,const char* filename)
{
    TRACE_C_API;
    return(-1);
}

int simGetUIPosition_internal(int elementHandle,int* position)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
            return(-1);

        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        VPoint p;
        it->getBlockPositionAbsolute(p);
        position[0]=p.x;
        position[1]=p.y;
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetUIPosition_internal(int elementHandle,const int* position)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        it->setDesiredBlockPosition(position[0],position[1]);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simHandleGeneralCallbackScript_internal(int callbackId,int callbackTag,void* additionalData)
{ // Deprecated since release 3.4.1
    TRACE_C_API;
    return(-1);
}

int simRegisterCustomLuaFunction_internal(const char* funcName,const char* callTips,const int* inputArgumentTypes,void(*callBack)(struct SLuaCallBack* p))
{ // deprecated
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        bool retVal=1;
        if (App::worldContainer->scriptCustomFuncAndVarContainer->removeCustomFunction(funcName))
            retVal=0;// that function already existed. We remove it and replace it!
        std::vector<int> inputV;
        if (inputArgumentTypes!=nullptr)
        {
            for (int i=0;i<inputArgumentTypes[0];i++)
                inputV.push_back(inputArgumentTypes[i+1]);
        }
        CScriptCustomFunction* newFunction=new CScriptCustomFunction(funcName,callTips,inputV,callBack);
        if (!App::worldContainer->scriptCustomFuncAndVarContainer->insertCustomFunction(newFunction))
        {
            delete newFunction;
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_CUSTOM_LUA_FUNC_COULD_NOT_BE_REGISTERED);
            return(-1);
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simRegisterContactCallback_internal(int(*callBack)(int,int,int,int*,double*))
{ // deprecated. Disabled on 18.05.2022
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(-1);
}

int simRegisterJointCtrlCallback_internal(int(*callBack)(int,int,int,const int*,const double*,double*))
{ // deprecated. Disabled on 18.05.2022
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(-1);
}

int simGetMechanismHandle_internal(const char* mechanismName)
{ // deprecated
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(-1);
}

int simHandleMechanism_internal(int mechanismHandle)
{ // deprecated
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(-1);
}

int simHandleCustomizationScripts_internal(int callType)
{ // deprecated
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;
        if (App::getEditModeType()==NO_EDIT_MODE)
        {
            retVal=App::currentWorld->embeddedScriptContainer->callChildAndEmbeddedScripts(sim_scripttype_customizationscript,callType,nullptr,nullptr);
            App::currentWorld->embeddedScriptContainer->removeDestroyedScripts(sim_scripttype_customizationscript);
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simCallScriptFunction_internal(int scriptHandleOrType,const char* functionNameAtScriptName,SLuaCallBack* data,const char* reservedSetToNull)
{ // DEPRECATED
    TRACE_C_API;
    CScriptObject* script=nullptr;

    std::string funcName;
    if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
    { // script is identified by its ID
        std::string funcNameAtScriptName(functionNameAtScriptName);
        size_t p=funcNameAtScriptName.find('@');
        if (p!=std::string::npos)
            funcName.assign(funcNameAtScriptName.begin(),funcNameAtScriptName.begin()+p);
        else
            funcName=funcNameAtScriptName;
        script=App::worldContainer->getScriptFromHandle(scriptHandleOrType);
    }
    else
    { // script is identified by a script type and sometimes also a script name
        if (reservedSetToNull==nullptr)
        {
            std::string scriptName;
            std::string funcNameAtScriptName(functionNameAtScriptName);
            size_t p=funcNameAtScriptName.find('@');
            if (p!=std::string::npos)
            {
                scriptName.assign(funcNameAtScriptName.begin()+p+1,funcNameAtScriptName.end());
                funcName.assign(funcNameAtScriptName.begin(),funcNameAtScriptName.begin()+p);
            }
            else
                funcName=funcNameAtScriptName;
            if (scriptHandleOrType==sim_scripttype_mainscript)
                script=App::currentWorld->embeddedScriptContainer->getMainScript();
            if ( (scriptHandleOrType==sim_scripttype_childscript)||(scriptHandleOrType==(sim_scripttype_childscript|sim_scripttype_threaded_old))||(scriptHandleOrType==sim_scripttype_customizationscript) )
            {
                int objId=-1;
                CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,scriptName.c_str(),0,nullptr);
                if (obj!=nullptr)
                    objId=obj->getObjectHandle();
                else
                    objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(scriptName.c_str());
                if (scriptHandleOrType==sim_scripttype_customizationscript)
                    script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objId);
                else
                    script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objId);
            }
            if (scriptHandleOrType==sim_scripttype_addonscript)
                script=App::worldContainer->addOnScriptContainer->getAddOnFromName(scriptName.c_str());
        }
        else
        { // this is the old way of doing it. Deprecated. Was only 2 months active, not officially
            funcName=functionNameAtScriptName;
            if (scriptHandleOrType==0) // main script
                script=App::currentWorld->embeddedScriptContainer->getMainScript();
            if (scriptHandleOrType==3) // child script
            {
                int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(reservedSetToNull);
                script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objId);
            }
            if (scriptHandleOrType==5) // customization
            {
                int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(reservedSetToNull);
                script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objId);
            }
        }
    }

    if (script!=nullptr)
    {
        int retVal=-1; // error
        if (script->getThreadedExecutionIsUnderWay_oldThreads())
        { // very special handling here!
            if (VThread::areThreadIDsSame(script->getThreadedScriptThreadId_old(),VThread::getCurrentThreadId()))
                retVal=script->callScriptFunction_DEPRECATED(funcName.c_str(),data);
            else
            { // we have to execute that function via another thread!
                void* d[4];
                int callType=0;
                d[0]=&callType;
                d[1]=script;
                d[2]=(void*)funcName.c_str();
                d[3]=data;

                retVal=CThreadPool_old::callRoutineViaSpecificThread(script->getThreadedScriptThreadId_old(),d);
            }
        }
        else
        {
            if (VThread::isCurrentThreadTheMainSimulationThread())
            { // For now we don't allow non-main threads to call non-threaded scripts!
                retVal=script->callScriptFunction_DEPRECATED(funcName.c_str(),data);
            }
        }
        if (retVal==-1)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_FAILED_CALLING_SCRIPT_FUNCTION);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);

    return(-1);
}

int simSetVisionSensorFilter_internal(int visionSensorHandle,int filterIndex,int options,const int* pSizes,const unsigned char* bytes,const int* ints,const double* floats,const unsigned char* custom)
{ // DEPRECATED
    TRACE_C_API;

    return(-1);
}

int simGetVisionSensorFilter_internal(int visionSensorHandle,int filterIndex,int* options,int* pSizes,unsigned char** bytes,int** ints,double** floats,unsigned char** custom)
{ // DEPRECATED
    TRACE_C_API;

    return(-1);
}

char* simGetScriptSimulationParameter_internal(int scriptHandle,const char* parameterName,int* parameterLength)
{ // DEPRECATED
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromHandle(scriptHandle);
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(scriptHandle);
        if ( (it!=nullptr)||(obj!=nullptr) )
        {
            if (obj==nullptr)
                obj=App::currentWorld->sceneObjects->getObjectFromHandle(it->getObjectHandleThatScriptIsAttachedTo(-1));
            if (obj!=nullptr)
            {
                CUserParameters* uso=obj->getUserScriptParameterObject();
                if (uso!=nullptr)
                {
                    std::string parameterValue;
                    if (uso->getParameterValue(parameterName,parameterValue))
                    {
                        char* retVal=new char[parameterValue.length()+1];
                        for (size_t i=0;i<parameterValue.length();i++)
                            retVal[i]=parameterValue[i];
                        retVal[parameterValue.length()]=0;
                        parameterLength[0]=(int)parameterValue.length();
                        return(retVal);
                    }
                }
            }
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(nullptr);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simSetScriptSimulationParameter_internal(int scriptHandle,const char* parameterName,const char* parameterValue,int parameterLength)
{ // DEPRECATED
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromHandle(scriptHandle);
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(scriptHandle);
        int retVal=-1;
        if ( (it!=nullptr)||(obj!=nullptr) )
        {
            if (obj==nullptr)
                obj=App::currentWorld->sceneObjects->getObjectFromHandle(it->getObjectHandleThatScriptIsAttachedTo(-1));
            if (obj!=nullptr)
            {
                retVal=0;
                CUserParameters* uso=obj->getUserScriptParameterObject();
                bool s=false;
                if (uso==nullptr)
                {
                    uso=new CUserParameters();
                    s=true;
                }
                uso->setParameterValue(parameterName,parameterValue,size_t(parameterLength));
                if (s)
                    obj->setUserScriptParameterObject(uso);
            }
        }
        if (retVal==-1)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetNameSuffix_internal(const char* name)
{ // DEPRECATED in 2020
    TRACE_C_API;

    int suffixNumber;
    if (name!=nullptr)
        suffixNumber=tt::getNameSuffixNumber(name,true);
    else
        suffixNumber=getCurrentScriptNameIndex_cSide();
    return(suffixNumber);
}

int simSetNameSuffix_internal(int nameSuffixNumber)
{ // DEPRECATED in 2020
    TRACE_C_API;

    if (nameSuffixNumber<-1)
        nameSuffixNumber=-1;
    setCurrentScriptInfo_cSide(_currentScriptHandle,nameSuffixNumber);
    return(1);
}

int simAddStatusbarMessage_internal(const char* message)
{ // DEPRECATED in 2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (message!=nullptr)
        {
//            App::addStatusbarMessage(message,false);
            int v=sim_verbosity_msgs;
            if (std::string(message).compare(0,18,"Lua runtime error:")==0) // probably not used at all.
                v=sim_verbosity_errors;
            App::logScriptMsg(nullptr,v|sim_verbosity_undecorated,message);
        }
        else
            App::clearStatusbar();

        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

char* simGetScriptRawBuffer_internal(int scriptHandle,int bufferHandle)
{ // DEPRECATED in 2020
    return(nullptr);
}

int simSetScriptRawBuffer_internal(int scriptHandle,const char* buffer,int bufferSize)
{ // DEPRECATED in 2020
    return(-1);
}

int simReleaseScriptRawBuffer_internal(int scriptHandle,int bufferHandle)
{ // DEPRECATED in 2020
    return(-1);
}

int simSetShapeMassAndInertia_internal(int shapeHandle,double mass,const double* inertiaMatrix,const double* centerOfMass,const double* transformation)
{ // DEPRECATED in 2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__,shapeHandle))
            return(-1);
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);

        if (mass<0.0000001)
            mass=0.0000001;
        C3X3Matrix m;
        m.setData(inertiaMatrix);
        m.axis[0](1)=m.axis[1](0);
        m.axis[0](2)=m.axis[2](0);
        m.axis[1](2)=m.axis[2](1);
        m/=mass; // in CoppeliaSim we work with the "massless inertia"
        it->getMeshWrapper()->setMass(mass);
        C3Vector com(centerOfMass);
        C4X4Matrix tr;
        if (transformation==nullptr)
            tr.setIdentity();
        else
            tr.setData(transformation);

        C4Vector rot;
        C3Vector pmoment;
        CMeshWrapper::findPrincipalMomentOfInertia(m,rot,pmoment);
        if (pmoment(0)<0.0000001)
            pmoment(0)=0.0000001;
        if (pmoment(1)<0.0000001)
            pmoment(1)=0.0000001;
        if (pmoment(2)<0.0000001)
            pmoment(0)=0.0000001;
        it->getMeshWrapper()->setPrincipalMomentsOfInertia(pmoment);
        it->getMeshWrapper()->setLocalInertiaFrame(it->getFullCumulativeTransformation().getInverse()*tr.getTransformation()*C7Vector(rot,com));
        it->setDynamicsResetFlag(true,false);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetShapeMassAndInertia_internal(int shapeHandle,double* mass,double* inertiaMatrix,double* centerOfMass,const double* transformation)
{ // DEPRECATED in 2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__,shapeHandle))
            return(-1);
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);

        mass[0]=it->getMeshWrapper()->getMass();
        //double mmm=it->geomInfo->getMass();

        C7Vector tr(it->getFullCumulativeTransformation()*it->getMeshWrapper()->getLocalInertiaFrame());
        C4X4Matrix ref;
        if (transformation==nullptr)
            ref.setIdentity();
        else
            ref.setData(transformation);
        C3X3Matrix m(CMeshWrapper::getNewTensor(it->getMeshWrapper()->getPrincipalMomentsOfInertia(),ref.getTransformation().getInverse()*tr));
        m*=mass[0]; // in CoppeliaSim we work with the "massless inertia"
        m.getData(inertiaMatrix);
        (ref.getTransformation().getInverse()*tr).X.getData(centerOfMass);

        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simCheckIkGroup_internal(int ikGroupHandle,int jointCnt,const int* jointHandles,double* jointValues,const int* jointOptions)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesIKGroupExist(__func__,ikGroupHandle))
            return(-1);
        int retVal=-1;
        CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        int r=it->checkIkGroup(jointCnt,jointHandles,jointValues,jointOptions);
        if (r==-1)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
        if (r==-2)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLES);
        if (r>=sim_ikresult_not_performed)
            retVal=r;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simCreateIkGroup_internal(int options,const int* intParams,const double* floatParams,const void* reserved)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CIkGroup_old* ikGroup=new CIkGroup_old();
        ikGroup->setObjectName("IK_Group",false);
        App::currentWorld->ikGroups->addIkGroup(ikGroup,false);
        ikGroup->setEnabled((options&1)==0);
        ikGroup->setRestoreIfPositionNotReached((options&4)!=0);
        ikGroup->setRestoreIfOrientationNotReached((options&8)!=0);
        ikGroup->setIgnoreMaxStepSizes((options&16)==0);
        ikGroup->setExplicitHandling((options&32)!=0);
        if (intParams!=nullptr)
        {
            ikGroup->setCalculationMethod(intParams[0]);
            ikGroup->setMaxIterations(intParams[1]);
        }
        if (floatParams!=nullptr)
            ikGroup->setDampingFactor(floatParams[0]);
        return(ikGroup->getObjectHandle());
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simRemoveIkGroup_internal(int ikGroupHandle)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(-1);
        }
        App::currentWorld->ikGroups->removeIkGroup(it->getObjectHandle());
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simCreateIkElement_internal(int ikGroupHandle,int options,const int* intParams,const double* floatParams,const void* reserved)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(-1);
        }
        int tip=intParams[0];
        int base=intParams[1];
        int constrBase=intParams[2];
        int constraints=intParams[3];
        if (!isDummy(__func__,tip))
            return(-1);
        if (App::currentWorld->sceneObjects->getObjectFromHandle(base)==nullptr)
            base=-1;
        if (App::currentWorld->sceneObjects->getObjectFromHandle(constrBase)==nullptr)
            constrBase=-1;
        CIkElement_old* ikEl=new CIkElement_old(tip);
        ikEl->setEnabled((options&1)==0);
        ikEl->setBase(base);
        ikEl->setAlternativeBaseForConstraints(constrBase);
        ikEl->setConstraints(constraints);
        if (floatParams!=nullptr)
        {
            ikEl->setMinLinearPrecision(floatParams[0]);
            ikEl->setMinAngularPrecision(floatParams[1]);
            ikEl->setPositionWeight(floatParams[2]);
            ikEl->setOrientationWeight(floatParams[3]);
        }
        it->addIkElement(ikEl);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simExportIk_internal(const char* pathAndFilename,int reserved1,void* reserved2)
{ // deprecated on 29.09.2020
    CApiErrors::setLastWarningOrError(__func__,"Not supported anymore. Use CoppeliaSim V4.2.0 or earlier.");
    return(-1);
}

int simComputeJacobian_internal(int ikGroupHandle,int options,void* reserved)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesIKGroupExist(__func__,ikGroupHandle))
            return(-1);
        int returnValue=-1;
        CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        if (it->computeOnlyJacobian(options))
            returnValue=0;
        return(returnValue);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetConfigForTipPose_internal(int ikGroupHandle,int jointCnt,const int* jointHandles,double thresholdDist,int maxTimeInMs,double* retConfig,const double* metric,int collisionPairCnt,const int* collisionPairs,const int* jointOptions,const double* lowLimits,const double* ranges,void* reserved)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesIKGroupExist(__func__,ikGroupHandle))
            return(-1);

        CIkGroup_old* ikGroup=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        std::string err;
        int retVal=ikGroup->getConfigForTipPose(jointCnt,jointHandles,thresholdDist,maxTimeInMs,retConfig,metric,collisionPairCnt,collisionPairs,jointOptions,lowLimits,ranges,err);
        if (retVal<0)
            CApiErrors::setLastWarningOrError(__func__,err.c_str());
        return(retVal);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

double* simGenerateIkPath_internal(int ikGroupHandle,int jointCnt,const int* jointHandles,int ptCnt,int collisionPairCnt,const int* collisionPairs,const int* jointOptions,void* reserved)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesIKGroupExist(__func__,ikGroupHandle))
            return(nullptr);
        std::vector<CJoint*> joints;
        CIkGroup_old* ikGroup=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        bool err=false;
        for (int i=0;i<jointCnt;i++)
        {
            CJoint* aJoint=App::currentWorld->sceneObjects->getJointFromHandle(jointHandles[i]);
            if (aJoint==nullptr)
                err=true;
            else
                joints.push_back(aJoint);
        }
        if (err)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLES);
        std::vector<CDummy*> tips;
        std::vector<CDummy*> targets;
        std::vector<C7Vector> startTrs;
        std::vector<C7Vector> goalTrs;
        if (!err)
        {
            if (ikGroup->getIkElementCount()>0)
            {
                for (size_t i=0;i<ikGroup->getIkElementCount();i++)
                {
                    CIkElement_old* ikElement=ikGroup->getIkElementFromIndex(i);
                    CDummy* tip=App::currentWorld->sceneObjects->getDummyFromHandle(ikElement->getTipHandle());
                    CDummy* target=App::currentWorld->sceneObjects->getDummyFromHandle(ikElement->getTargetHandle());
                    if ((tip==nullptr)||(target==nullptr))
                        err=true;
                    tips.push_back(tip);
                    targets.push_back(target);
                    startTrs.push_back(tip->getFullCumulativeTransformation());
                    goalTrs.push_back(target->getFullCumulativeTransformation());
                }
            }
            else
            {
                err=true;
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_IK_ELEMENT_INEXISTANT);
            }
        }
        if (!err)
        {
            if (ptCnt<2)
            {
                err=true;
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENT);
            }
        }
        if ( (!err)&&(collisionPairCnt>0)&&(collisionPairs!=nullptr) )
        {
            for (int i=0;i<collisionPairCnt;i++)
            {
                if (collisionPairs[2*i+0]!=-1)
                {
                    if (!doesCollectionExist(__func__,collisionPairs[2*i+0]))
                        err=true;
                    else
                    {
                        if (collisionPairs[2*i+1]!=sim_handle_all)
                        {
                            if (!doesCollectionExist(__func__,collisionPairs[2*i+1]))
                                err=true;
                        }
                    }
                }
            }
        }
        if (!err)
        {
            // Save joint positions/modes (all of them, just in case)
            std::vector<CJoint*> sceneJoints;
            std::vector<double> initSceneJointValues;
            std::vector<int> initSceneJointModes;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getJointCount();i++)
            {
                CJoint* aj=App::currentWorld->sceneObjects->getJointFromIndex(i);
                sceneJoints.push_back(aj);
                initSceneJointValues.push_back(aj->getPosition());
                initSceneJointModes.push_back(aj->getJointMode());
            }

            ikGroup->setAllInvolvedJointsToNewJointMode(sim_jointmode_kinematic);

            bool ikGroupWasActive=ikGroup->getEnabled();
            if (!ikGroupWasActive)
                ikGroup->setEnabled(true);

            // It can happen that some IK elements get deactivated when the user provided wrong handles, so save the activation state:
            std::vector<bool> enabledElements;
            for (size_t i=0;i<ikGroup->getIkElementCount();i++)
            {
                CIkElement_old* ikElement=ikGroup->getIkElementFromIndex(i);
                enabledElements.push_back(ikElement->getEnabled());
            }

            // Set the correct mode for the joints involved:
            for (int i=0;i<jointCnt;i++)
            {
                if ( (jointOptions==nullptr)||((jointOptions[i]&1)==0) )
                    joints[i]->setJointMode_noDynMotorTargetPosCorrection(sim_jointmode_ik_deprecated);
                else
                    joints[i]->setJointMode_noDynMotorTargetPosCorrection(sim_jointmode_dependent);
            }

            // do the calculation:
            double t=0.0;
            double dt=1.0/(ptCnt-1);
            bool failed=false;
            std::vector<double> thePath;
            for (int iterCnt=0;iterCnt<ptCnt;iterCnt++)
            {
                for (size_t el=0;el<ikGroup->getIkElementCount();el++)
                { // set all targets to an interpolated pose
                    C7Vector tr;
                    tr.buildInterpolation(startTrs[el],goalTrs[el],t);
                    targets[el]->setAbsoluteTransformation(tr);
                }

                // Try to perform IK:
                if (sim_ikresult_success==ikGroup->computeGroupIk(true))
                {
                    bool colliding=false;
                    if ( (collisionPairCnt>0)&&(collisionPairs!=nullptr) )
                    { // we need to check if this state collides:
                        for (int i=0;i<collisionPairCnt;i++)
                        {
                            if (collisionPairs[2*i+0]>=0)
                            {
                                int env=collisionPairs[2*i+1];
                                if (env==sim_handle_all)
                                    env=-1;
                                if (CCollisionRoutine::doEntitiesCollide(collisionPairs[2*i+0],env,nullptr,false,false,nullptr))
                                {
                                    colliding=true;
                                    break;
                                }
                            }
                        }
                    }
                    if (!colliding)
                    { // we save this path point
                        for (int i=0;i<jointCnt;i++)
                            thePath.push_back(joints[i]->getPosition());
                    }
                    else
                        failed=true;
                }
                else
                    failed=true;
                if (failed)
                    break;
                t+=dt;
            }

            if (!ikGroupWasActive)
                ikGroup->setEnabled(false);

            // Restore the IK element activation state:
            for (size_t i=0;i<ikGroup->getIkElementCount();i++)
            {
                CIkElement_old* ikElement=ikGroup->getIkElementFromIndex(i);
                ikElement->setEnabled(enabledElements[i]);
            }

            // Restore joint positions/modes:
            for (size_t i=0;i<sceneJoints.size();i++)
            {
                if (sceneJoints[i]->getPosition()!=initSceneJointValues[i])
                    sceneJoints[i]->setPosition(initSceneJointValues[i]);
                if (sceneJoints[i]->getJointMode()!=initSceneJointModes[i])
                    sceneJoints[i]->setJointMode_noDynMotorTargetPosCorrection(initSceneJointModes[i]);
            }

            // Restore target dummies:
            for (size_t el=0;el<ikGroup->getIkElementCount();el++)
                targets[el]->setAbsoluteTransformation(goalTrs[el]);

            if (!failed)
            {
                double* retVal=new double[jointCnt*ptCnt];
                for (int i=0;i<jointCnt*ptCnt;i++)
                    retVal[i]=thePath[i];
                return(retVal);
            }
            return(nullptr);
        }
        return(nullptr);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simGetIkGroupHandle_internal(const char* ikGroupName)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    size_t silentErrorPos=std::string(ikGroupName).find("@silentError");
    std::string nm(ikGroupName);
    if (silentErrorPos!=std::string::npos)
        nm.erase(nm.begin()+silentErrorPos,nm.end());

    std::string ikGroupNameAdjusted=getIndexAdjustedObjectName(nm.c_str());
    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromName(ikGroupNameAdjusted.c_str());
        if (it==nullptr)
        {
            if (silentErrorPos==std::string::npos)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(-1);
        }
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

double* simGetIkGroupMatrix_internal(int ikGroupHandle,int options,int* matrixSize)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(nullptr);
        }
        double* retData=nullptr;
        if (options==0)
            retData=(double*)it->getLastJacobianData(matrixSize);
        if (options==1)
            retData=it->getLastManipulabilityValue(matrixSize);
        return(retData);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simHandleIkGroup_internal(int ikGroupHandle)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (ikGroupHandle!=sim_handle_all)&&(ikGroupHandle!=sim_handle_all_except_explicit) )
        {
            if (!doesIKGroupExist(__func__,ikGroupHandle))
                return(-1);
        }
        int returnValue=0;
        if (ikGroupHandle<0)
            returnValue=App::currentWorld->ikGroups->computeAllIkGroups(ikGroupHandle==sim_handle_all_except_explicit);
        else
        { // explicit handling
            CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            returnValue=it->computeGroupIk(false);
        }
        return(returnValue);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetIkGroupProperties_internal(int ikGroupHandle,int resolutionMethod,int maxIterations,double damping,void* reserved)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(-1);
        }
        it->setCalculationMethod(resolutionMethod);
        it->setMaxIterations(maxIterations);
        it->setDampingFactor(damping);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetIkElementProperties_internal(int ikGroupHandle,int tipDummyHandle,int constraints,const double* precision,const double* weight,void* reserved)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(-1);
        }
        if (!isDummy(__func__,tipDummyHandle))
            return(-1);
        CIkElement_old* el=it->getIkElementFromTipHandle(tipDummyHandle);
        if (el==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_IK_ELEMENT_INEXISTANT);
            return(-1);
        }
        el->setConstraints(constraints);
        if (precision!=nullptr)
        {
            el->setMinLinearPrecision(precision[0]);
            el->setMinAngularPrecision(precision[1]);
        }
        if (weight!=nullptr)
        {
            el->setPositionWeight(weight[0]);
            el->setOrientationWeight(weight[1]);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetThreadIsFree_internal(bool freeMode)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (VThread::isCurrentThreadTheMainSimulationThread())
    {

        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_CANNOT_BE_CALLED_FROM_MAIN_THREAD);
        return(-1);
    }
    if (CThreadPool_old::setThreadFreeMode(freeMode!=0))
        return(1);
    return(0);
}

int simTubeOpen_internal(int dataHeader,const char* dataName,int readBufferSize,bool notUsedButKeepFalse)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal;
        retVal=App::currentWorld->commTubeContainer->openTube(dataHeader,dataName,false,readBufferSize);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simTubeClose_internal(int tubeHandle)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;
        if (tubeHandle&1)
        { // not used anymore
        }
        else
        {
            if (App::currentWorld->commTubeContainer->closeTube(tubeHandle))
                retVal=1;
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simTubeWrite_internal(int tubeHandle,const char* data,int dataLength)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;
        if (tubeHandle&1)
        { // not used anymore
        }
        else
        {
            if (App::currentWorld->commTubeContainer->writeToTube_copyBuffer(tubeHandle,data,dataLength))
                retVal=1;
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

char* simTubeRead_internal(int tubeHandle,int* dataLength)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        char* retVal;
        retVal=App::currentWorld->commTubeContainer->readFromTube_bufferNotCopied(tubeHandle,dataLength[0]);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simTubeStatus_internal(int tubeHandle,int* readPacketsCount,int* writePacketsCount)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int readP=0;
        int writeP=0;
        int retVal;
        retVal=App::currentWorld->commTubeContainer->getTubeStatus(tubeHandle,readP,writeP);
        if (readPacketsCount!=nullptr)
            readPacketsCount[0]=readP;
        if (writePacketsCount!=nullptr)
            writePacketsCount[0]=writeP;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSendData_internal(int targetID,int dataHeader,const char* dataName,const char* data,int dataLength,int antennaHandle,double actionRadius,double emissionAngle1,double emissionAngle2,double persistence)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (App::currentWorld->simulation->getSimulationState()==sim_simulation_stopped)
    {
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (targetID!=0)&&(targetID!=sim_handle_all) )
        {
            CScriptObject* it=App::worldContainer->getScriptFromHandle(targetID);
            if (it==nullptr)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_TARGET_HANDLE);
                return(-1);
            }
        }
        if (dataHeader<0)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_DATA_HEADER);
            return(-1);
        }
        if (strlen(dataName)<1)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_DATA_NAME);
            return(-1);
        }
        if (dataLength<1)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_DATA);
            return(-1);
        }
        if (antennaHandle!=sim_handle_default)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(antennaHandle);
            if (it==nullptr)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ANTENNA_HANDLE);
                return(-1);
            }
        }
        actionRadius=tt::getLimitedFloat(0.0,FLOAT_MAX,actionRadius);
        emissionAngle1=tt::getLimitedFloat(0.0,piValue,emissionAngle1);
        emissionAngle2=tt::getLimitedFloat(0.0,piValT2,emissionAngle2);
        persistence=tt::getLimitedFloat(0.0,99999999999999.9,persistence);
        if (persistence==0.0)
            persistence=App::currentWorld->simulation->getTimeStep()*1.5;
        std::string datN(dataName);
        App::currentWorld->embeddedScriptContainer->broadcastDataContainer.broadcastData(0,targetID,dataHeader,datN,
                        App::currentWorld->simulation->getSimulationTime()+persistence,actionRadius,antennaHandle,
                        emissionAngle1,emissionAngle2,data,dataLength);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

char* simReceiveData_internal(int dataHeader,const char* dataName,int antennaHandle,int index,int* dataLength,int* senderID,int* dataHeaderR,char** dataNameR)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (App::currentWorld->simulation->getSimulationState()==sim_simulation_stopped)
    {
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (dataHeader<0)
            dataHeader=-1;
        if (dataName!=nullptr)
        {
            if (strlen(dataName)<1)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_DATA_NAME);
                return(nullptr);
            }
        }
        if (antennaHandle!=sim_handle_default)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(antennaHandle);
            if (it==nullptr)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ANTENNA_HANDLE);
                return(nullptr);
            }
        }
        if (index<0)
            index=-1;
        std::string datNm;
        if (dataName!=nullptr)
            datNm=dataName;
        int theIndex=index;
        int theSenderID;
        int theDataHeader;
        std::string theDataName;
        char* data0=App::currentWorld->embeddedScriptContainer->broadcastDataContainer.receiveData(0,App::currentWorld->simulation->getSimulationTime(),
                dataHeader,datNm,antennaHandle,dataLength[0],theIndex,theSenderID,theDataHeader,theDataName);
        char* retData=nullptr;
        if (data0!=nullptr)
        {
            retData=new char[dataLength[0]];
            for (int i=0;i<dataLength[0];i++)
                retData[i]=data0[i];
            if (senderID!=nullptr)
                senderID[0]=theSenderID;
            if (dataHeaderR!=nullptr)
                dataHeaderR[0]=theDataHeader;
            if (dataNameR!=nullptr)
            {
                dataNameR[0]=new char[theDataName.length()+1];
                for (int i=0;i<int(theDataName.length());i++)
                    dataNameR[0][i]=theDataName[i];
                dataNameR[0][theDataName.length()]=0; // terminal zero
            }
        }
        return(retData);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simGetDataOnPath_internal(int pathHandle,double relativeDistance,int dataType,int* intData,double* floatData)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,pathHandle))
            return(-1);
        if (!isPath(__func__,pathHandle))
            return(-1);
        CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(pathHandle);
        double auxChannels[4];
        int auxFlags;
        if (dataType==0)
        {
            if (relativeDistance>-0.5)
            { // regular use of the function
                if (it->pathContainer->getAuxDataOnBezierCurveAtNormalizedVirtualDistance(relativeDistance,auxFlags,auxChannels))
                {
                    intData[0]=auxFlags;
                    for (int i=0;i<4;i++)
                        floatData[i]=auxChannels[i];
                    return(1);
                }
                else
                {
                    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_EMPTY);
                    return(-1);
                }
            }
            else
            { // We are working with indices pointing on ctrl points. index=-(relativeDistance+1.0)
                CSimplePathPoint_old* ctrlPt=it->pathContainer->getSimplePathPoint(int(-relativeDistance-0.5));
                if (ctrlPt!=nullptr)
                {
                    intData[0]=ctrlPt->getAuxFlags();
                    ctrlPt->getAuxChannels(floatData);
                    return(1);
                }
                else
                {
                    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_CTRL_PT);
                    return(-1);
                }
            }
        }
        else
            return(-1);

    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


int simGetPositionOnPath_internal(int pathHandle,double relativeDistance,double* position)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,pathHandle))
            return(-1);
        if (!isPath(__func__,pathHandle))
            return(-1);
        CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(pathHandle);
        C7Vector tr;

        if (relativeDistance>-0.5)
        { // regular use of the function
            if (it->pathContainer->getTransformationOnBezierCurveAtNormalizedVirtualDistance(relativeDistance,tr))
            {
                tr=it->getCumulativeTransformation()*tr;
                tr.X.getData(position);
                return(1);
            }
            else
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_EMPTY);
                return(-1);
            }
        }
        else
        { // We are working with indices pointing on ctrl points. index=-(relativeDistance+1.0)
            CSimplePathPoint_old* ctrlPt=it->pathContainer->getSimplePathPoint(int(-relativeDistance-0.5));
            if (ctrlPt!=nullptr)
            {
                tr=ctrlPt->getTransformation();
                tr=it->getCumulativeTransformation()*tr;
                tr.X.getData(position);
                return(1);
            }
            else
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_CTRL_PT);
                return(-1);
            }
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetOrientationOnPath_internal(int pathHandle,double relativeDistance,double* eulerAngles)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,pathHandle))
            return(-1);
        if (!isPath(__func__,pathHandle))
            return(-1);
        CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(pathHandle);
        C7Vector tr;
        if (relativeDistance>-0.5)
        { // regular use of the function
            if (it->pathContainer->getTransformationOnBezierCurveAtNormalizedVirtualDistance(relativeDistance,tr))
            {
                tr=it->getCumulativeTransformation()*tr;
                C3Vector(tr.Q.getEulerAngles()).getData(eulerAngles);
                return(1);
            }
            else
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_EMPTY);
                return(-1);
            }
        }
        else
        { // We are working with indices pointing on ctrl points. index=-(relativeDistance+1.0)
            CSimplePathPoint_old* ctrlPt=it->pathContainer->getSimplePathPoint(int(-relativeDistance-0.5));
            if (ctrlPt!=nullptr)
            {
                tr=ctrlPt->getTransformation();
                tr=it->getCumulativeTransformation()*tr;
                C3Vector(tr.Q.getEulerAngles()).getData(eulerAngles);
                return(1);
            }
            else
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_CTRL_PT);
                return(-1);
            }
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetClosestPositionOnPath_internal(int pathHandle,double* absolutePosition,double* pathPosition)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,pathHandle))
            return(-1);
        if (!isPath(__func__,pathHandle))
            return(-1);
        CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(pathHandle);
        C3Vector p(absolutePosition);
        if (it->pathContainer->getPositionOnPathClosestTo(p,*pathPosition))
        {
            double pl=it->pathContainer->getBezierVirtualPathLength();
            if (pl!=0.0)
                *pathPosition/=pl;
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_EMPTY);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetPathPosition_internal(int objectHandle,double* position)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isPath(__func__,objectHandle))
            return(-1);
        CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(objectHandle);
        position[0]=double(it->pathContainer->getPosition());
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetPathPosition_internal(int objectHandle,double position)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isPath(__func__,objectHandle))
            return(-1);
        CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(objectHandle);
        it->pathContainer->setPosition(position);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetPathLength_internal(int objectHandle,double* length)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isPath(__func__,objectHandle))
            return(-1);
        CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(objectHandle);
        length[0]=it->pathContainer->getBezierVirtualPathLength();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simCreatePath_internal(int attributes,const int* intParams,const double* floatParams,const float* color)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CPath_old* newObject=new CPath_old();
        App::currentWorld->sceneObjects->addObjectToScene(newObject,false,true);
        if (attributes!=-1)
            newObject->pathContainer->setAttributes(attributes);

        if (intParams!=nullptr)
        {
            newObject->pathContainer->setLineSize(intParams[0]);
            newObject->pathContainer->setPathLengthCalculationMethod(intParams[1]);
        }

        if (floatParams!=nullptr)
        {
            newObject->pathContainer->setSquareSize(floatParams[0]);
            newObject->pathContainer->setAngleVarToDistanceCoeff(floatParams[1]);
            newObject->pathContainer->setOnSpotDistanceToDistanceCoeff(floatParams[2]);
        }

        int retVal=newObject->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simInsertPathCtrlPoints_internal(int pathHandle,int options,int startIndex,int ptCnt,const void* ptData)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CPath_old* path=App::currentWorld->sceneObjects->getPathFromHandle(pathHandle);
        if (path==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_INEXISTANT);
            return(-1);
        }

        path->pathContainer->enableActualization(false);
        int fiCnt=11;
        if (options&2)
            fiCnt=16;

        for (int i=0;i<ptCnt;i++)
        {
            CSimplePathPoint_old* pt=new CSimplePathPoint_old();
            C7Vector tr(C4Vector(((double*)ptData)[fiCnt*i+3],((double*)ptData)[fiCnt*i+4],((double*)ptData)[fiCnt*i+5]),C3Vector(((double*)ptData)+fiCnt*i+0));
            pt->setTransformation(tr,path->pathContainer->getAttributes());
            pt->setMaxRelAbsVelocity(((double*)ptData)[fiCnt*i+6]);
            pt->setOnSpotDistance(((double*)ptData)[fiCnt*i+7]);
            pt->setBezierPointCount(((int*)ptData)[fiCnt*i+8]);
            pt->setBezierFactors(((double*)ptData)[fiCnt*i+9],((double*)ptData)[fiCnt*i+10]);
            if (options&2)
            {
                pt->setAuxFlags(((int*)ptData)[fiCnt*i+11]);
                pt->setAuxChannels(((double*)ptData)+fiCnt*i+12);
            }
            path->pathContainer->insertSimplePathPoint(pt,startIndex+i);
        }
        if (options&1)
            path->pathContainer->setAttributes(path->pathContainer->getAttributes()|sim_pathproperty_closed_path);
        else
            path->pathContainer->setAttributes((path->pathContainer->getAttributes()|sim_pathproperty_closed_path)-sim_pathproperty_closed_path);
        path->pathContainer->enableActualization(true);
        path->pathContainer->actualizePath();
        App::setFullDialogRefreshFlag();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simCutPathCtrlPoints_internal(int pathHandle,int startIndex,int ptCnt)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CPath_old* path=App::currentWorld->sceneObjects->getPathFromHandle(pathHandle);
        if (path==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_INEXISTANT);
            return(-1);
        }
        if ((startIndex<0)||(ptCnt<0))
            path->pathContainer->removeAllSimplePathPoints();
        else
        {
            path->pathContainer->enableActualization(false);
            for (int i=0;i<ptCnt;i++)
                path->pathContainer->removeSimplePathPoint(startIndex);
            path->pathContainer->enableActualization(true);
            path->pathContainer->actualizePath();
        }
        App::setFullDialogRefreshFlag();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simGetThreadId_internal()
{ // deprecated on 01.10.2020
    TRACE_C_API;

    int retVal=VThread::getThreadId_apiQueried();
    return(retVal); // 0=GUI thread, 1=main sim thread, 2-n=aux. sim threads
}

int simSwitchThread_internal()
{ // deprecated on 01.10.2020
    TRACE_C_API;
    if (CThreadPool_old::getThreadAutomaticSwitch())
    { // Important: when a script forbids thread switching, we don't want that a plugin switches anyways
        if (CThreadPool_old::switchBackToPreviousThread())
            return(1);
    }
    return(0);
}

int simLockResources_internal(int lockType,int reserved)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

#ifdef SIM_WITH_QT
    CSimAndUiThreadSync* obj=new CSimAndUiThreadSync(__func__);
    bool res=false;
    int retVal=-1; // fail
    if (lockType==sim_lock_ui_wants_to_read)
        res=obj->uiThread_tryToLockForUiEventRead(5);
    if (lockType==sim_lock_ui_wants_to_write)
        res=obj->uiThread_tryToLockForUiEventWrite(800);
    if (lockType==sim_lock_nonui_wants_to_write)
    {
        obj->simThread_lockForSimThreadWrite();
        res=true;
    }
    if (res)
    {
        EASYLOCK(_lockForExtLockList);
        retVal=obj->getObjectHandle();
        _extLockList.push_back(obj);
    }
    return(retVal);
#else
    return(0);
#endif
}

int simUnlockResources_internal(int lockHandle)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

#ifdef SIM_WITH_QT
    int retVal=0;
    { // scope parenthesis are important here!
        EASYLOCK(_lockForExtLockList);
        for (int i=0;i<int(_extLockList.size());i++)
        {
            if (_extLockList[i]->getObjectHandle()==lockHandle)
            {
                delete _extLockList[i];
                _extLockList.erase(_extLockList.begin()+i);
                retVal=1;
                break;
            }
        }
    }
    return(retVal);
#else
    return(1);
#endif
}

char* simGetUserParameter_internal(int objectHandle,const char* parameterName,int* parameterLength)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(nullptr);
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        CUserParameters* uso=obj->getUserScriptParameterObject();
        if (uso!=nullptr)
        {
            std::string parameterValue;
            if (uso->getParameterValue(parameterName,parameterValue))
            {
                char* retVal=new char[parameterValue.length()+1];
                for (size_t i=0;i<parameterValue.length();i++)
                    retVal[i]=parameterValue[i];
                retVal[parameterValue.length()]=0;
                parameterLength[0]=(int)parameterValue.length();
                return(retVal);
            }
        }
        return(nullptr);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simSetUserParameter_internal(int objectHandle,const char* parameterName,const char* parameterValue,int parameterLength)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        CUserParameters* uso=obj->getUserScriptParameterObject();
        bool s=false;
        if (uso==nullptr)
        {
            uso=new CUserParameters();
            s=true;
        }
        if (std::string(parameterName).compare("@enable")==0)
            uso->addParameterValue("exampleParameter","string","Hello World!",strlen("Hello World!"));
        else
            uso->setParameterValue(parameterName,parameterValue,size_t(parameterLength));
        if (s)
            obj->setUserScriptParameterObject(uso);
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetPathTargetNominalVelocity_internal(int objectHandle,double targetNominalVelocity)
{ // deprecated probably around 2015 or earlier
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isPath(__func__,objectHandle))
            return(-1);
        CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(objectHandle);
        it->pathContainer->setTargetNominalVelocity(targetNominalVelocity);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetCollectionHandle_internal(const char* collectionName)
{ // deprecated on 17.11.2020
    TRACE_C_API;

    size_t silentErrorPos=std::string(collectionName).find("@silentError");
    std::string nm(collectionName);
    if (silentErrorPos!=std::string::npos)
        nm.erase(nm.begin()+silentErrorPos,nm.end());

    std::string collectionNameAdjusted=getIndexAdjustedObjectName(nm.c_str());
    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CCollection* it=App::currentWorld->collections->getObjectFromName(collectionNameAdjusted.c_str());
        if (it==nullptr)
        {
            if (silentErrorPos==std::string::npos)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COLLECTION_INEXISTANT);
            return(-1);
        }
        int retVal=it->getCollectionHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simRemoveCollection_internal(int collectionHandle)
{ // deprecated on 17.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::vector<int> memSel;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            memSel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));

        if (collectionHandle==sim_handle_all)
        {
            App::currentWorld->sceneObjects->deselectObjects();
            for (size_t i=0;i<App::currentWorld->collections->getObjectCount();i++)
                App::currentWorld->collections->addCollectionToSelection(App::currentWorld->collections->getObjectFromIndex(i)->getCollectionHandle());
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::currentWorld->sceneObjects->deselectObjects();
            App::currentWorld->sceneObjects->eraseObjects(sel,true);
            App::currentWorld->collections->removeAllCollections();
            // Restore previous' selection state:
            for (size_t i=0;i<memSel.size();i++)
                App::currentWorld->sceneObjects->addObjectToSelection(memSel[i]);
            return(1);
        }
        else
        {
            CCollection* it=App::currentWorld->collections->getObjectFromHandle(collectionHandle);
            if (it==nullptr)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COLLECTION_INEXISTANT);
                return(-1);
            }
            App::currentWorld->sceneObjects->deselectObjects();
            App::currentWorld->collections->addCollectionToSelection(it->getCollectionHandle());
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::currentWorld->sceneObjects->deselectObjects();
            App::currentWorld->sceneObjects->eraseObjects(sel,true);
            App::currentWorld->collections->removeCollection(collectionHandle);
            // Restore previous' selection state:
            for (size_t i=0;i<memSel.size();i++)
                App::currentWorld->sceneObjects->addObjectToSelection(memSel[i]);
            return(1);
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simEmptyCollection_internal(int collectionHandle)
{ // deprecated on 17.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (collectionHandle==sim_handle_all)
        {
            for (size_t i=0;i<App::currentWorld->collections->getObjectCount();i++)
                App::currentWorld->collections->getObjectFromIndex(i)->emptyCollection();
            return(1);
        }
        CCollection* it=App::currentWorld->collections->getObjectFromHandle(collectionHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COLLECTION_INEXISTANT);
            return(-1);
        }
        it->emptyCollection();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

char* simGetCollectionName_internal(int collectionHandle)
{ // deprecated on 17.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesCollectionExist(__func__,collectionHandle))
            return(nullptr);
        CCollection* it=App::currentWorld->collections->getObjectFromHandle(collectionHandle);
        char* retVal=new char[it->getCollectionName().length()+1];
        for (unsigned int i=0;i<it->getCollectionName().length();i++)
            retVal[i]=it->getCollectionName()[i];
        retVal[it->getCollectionName().length()]=0;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simSetCollectionName_internal(int collectionHandle,const char* collectionName)
{ // deprecated on 17.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesCollectionExist(__func__,collectionHandle))
            return(-1);
        CCollection* it=App::currentWorld->collections->getObjectFromHandle(collectionHandle);
        std::string originalText(collectionName);
        if (originalText.length()>127)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
            return(-1);
        }
        std::string text(collectionName);
        tt::removeIllegalCharacters(text,true);
        if (originalText!=text)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
            return(-1);
        }
        if (it->getCollectionName().compare(text)==0)
            return(1);
        if (App::currentWorld->collections->getObjectFromName(text.c_str())!=nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
            return(-1);
        }
        it->setCollectionName(originalText.c_str(),true);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);

}

int simCreateCollection_internal(const char* collectionName,int options)
{ // deprecated on 17.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::string originalText;
        if (collectionName!=nullptr)
            originalText=collectionName;
        if (originalText.length()!=0)
        {
            if ( (originalText.length()<=0)||(originalText.length()>127) )
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
                return(-1);
            }
            std::string text(collectionName);
            tt::removeIllegalCharacters(text,true);
            if (originalText!=text)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
                return(-1);
            }
            if (App::currentWorld->collections->getObjectFromName(text.c_str())!=nullptr)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
                return(-1);
            }
        }
        else
            originalText="collection"; // default name

        CCollection* it=new CCollection(-2);
        it->setCollectionName(originalText.c_str(),false);
        App::currentWorld->collections->addCollection(it,false);
        it->setOverridesObjectMainProperties((options&1)!=0);
        return(it->getCollectionHandle());
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simAddObjectToCollection_internal(int collectionHandle,int objectHandle,int what,int options)
{ // deprecated on 17.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesCollectionExist(__func__,collectionHandle))
        {
            return(-1);
        }
        CCollection* it=App::currentWorld->collections->getObjectFromHandle(collectionHandle);
        if (what!=sim_handle_all)
        {
            if (!doesObjectExist(__func__,objectHandle))
            {
                return(-1);
            }
        }
        CCollectionElement* el=nullptr;
        if (what==sim_handle_all)
        {
            el=new CCollectionElement(-1,sim_collectionelement_all,true);
        }
        if (what==sim_handle_single)
        {
            el=new CCollectionElement(objectHandle,sim_collectionelement_loose,(options&1)==0);
        }
        if (what==sim_handle_tree)
        {
            int what=sim_collectionelement_frombaseincluded;
            if ((options&2)!=0)
                what=sim_collectionelement_frombaseexcluded;
            el=new CCollectionElement(objectHandle,what,(options&1)==0);
        }
        if (what==sim_handle_chain)
        {
            int what=sim_collectionelement_fromtipincluded;
            if ((options&2)!=0)
                what=sim_collectionelement_fromtipexcluded;
            el=new CCollectionElement(objectHandle,what,(options&1)==0);
        }
        if (el==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENT);
            return(-1);
        }
        it->addCollectionElement(el);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simHandleCollision_internal(int collisionObjectHandle)
{ // deprecated on 20.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (collisionObjectHandle!=sim_handle_all)&&(collisionObjectHandle!=sim_handle_all_except_explicit) )
        {
            if (!doesCollisionObjectExist(__func__,collisionObjectHandle))
            {
                return(-1);
            }
        }
        int colCnt=0;
        if (collisionObjectHandle<0)
            colCnt=App::currentWorld->collisions->handleAllCollisions(collisionObjectHandle==sim_handle_all_except_explicit); // implicit handling
        else
        { // explicit handling
            CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(collisionObjectHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            if (it->handleCollision())
                colCnt++;
        }
        return(colCnt);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simReadCollision_internal(int collisionObjectHandle)
{ // deprecated on 20.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesCollisionObjectExist(__func__,collisionObjectHandle))
        {
            return(-1);
        }
        CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(collisionObjectHandle);
        int retVal=it->readCollision(nullptr);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simHandleDistance_internal(int distanceObjectHandle,double* smallestDistance)
{ // deprecated on 20.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (distanceObjectHandle!=sim_handle_all)&&(distanceObjectHandle!=sim_handle_all_except_explicit) )
        {
            if (!doesDistanceObjectExist(__func__,distanceObjectHandle))
                return(-1);
        }
        double d;
        if (distanceObjectHandle<0)
            d=App::currentWorld->distances->handleAllDistances(distanceObjectHandle==sim_handle_all_except_explicit); // implicit handling
        else
        { // explicit handling
            CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(distanceObjectHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            d=it->handleDistance();
        }
        if (d>=0.0)
        {
            if (smallestDistance!=nullptr)
                smallestDistance[0]=d;
            return(1);
        }
        else
            return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simReadDistance_internal(int distanceObjectHandle,double* smallestDistance)
{ // deprecated on 20.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesDistanceObjectExist(__func__,distanceObjectHandle))
            return(-1);
        double d;
        CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(distanceObjectHandle);
        d=it->readDistance();
        if (d>=0.0)
        {
            smallestDistance[0]=d;
            return(1);
        }
        smallestDistance[0]=FLOAT_MAX; // new for V3.3.2 rev2
        return(0); // from -1 to 0 for V3.3.2 rev2
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetCollisionHandle_internal(const char* collisionObjectName)
{ // deprecated on 20.11.2020
    TRACE_C_API;

    size_t silentErrorPos=std::string(collisionObjectName).find("@silentError");
    std::string nm(collisionObjectName);
    if (silentErrorPos!=std::string::npos)
        nm.erase(nm.begin()+silentErrorPos,nm.end());

    std::string collisionObjectNameAdjusted=getIndexAdjustedObjectName(nm.c_str());
    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromName(collisionObjectNameAdjusted.c_str());
        if (it==nullptr)
        {
            if (silentErrorPos==std::string::npos)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COLLISION_INEXISTANT);
            return(-1);
        }
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetDistanceHandle_internal(const char* distanceObjectName)
{ // deprecated on 20.11.2020
    TRACE_C_API;

    size_t silentErrorPos=std::string(distanceObjectName).find("@silentError");
    std::string nm(distanceObjectName);
    if (silentErrorPos!=std::string::npos)
        nm.erase(nm.begin()+silentErrorPos,nm.end());

    std::string distanceObjectNameAdjusted=getIndexAdjustedObjectName(nm.c_str());
    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CDistanceObject_old* it=App::currentWorld->distances->getObjectFromName(distanceObjectNameAdjusted.c_str());
        if (it==nullptr)
        {
            if (silentErrorPos==std::string::npos)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_DISTANCE_INEXISTANT);
            return(-1);
        }
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simResetCollision_internal(int collisionObjectHandle)
{ // deprecated on 20.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (collisionObjectHandle!=sim_handle_all)&&(collisionObjectHandle!=sim_handle_all_except_explicit) )
        {
            if (!doesCollisionObjectExist(__func__,collisionObjectHandle))
                return(-1);
        }
        if (collisionObjectHandle<0)
            App::currentWorld->collisions->resetAllCollisions(collisionObjectHandle==sim_handle_all_except_explicit);
        else
        { // Explicit handling
            CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(collisionObjectHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->clearCollisionResult();
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simResetDistance_internal(int distanceObjectHandle)
{ // deprecated on 20.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (distanceObjectHandle!=sim_handle_all)&&(distanceObjectHandle!=sim_handle_all_except_explicit) )
        {
            if (!doesDistanceObjectExist(__func__,distanceObjectHandle))
                return(-1);
        }
        if (distanceObjectHandle<0)
            App::currentWorld->distances->resetAllDistances(distanceObjectHandle==sim_handle_all_except_explicit);
        else
        { // Explicit handling
            CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(distanceObjectHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->clearDistanceResult();
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simAddBanner_internal(const char* label,double size,int options,const double* positionAndEulerAngles,int parentObjectHandle,const float* labelColors,const float* backgroundColors)
{ // deprecated on 23.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal=-1; // means error

        CBannerObject* it=new CBannerObject(label,options,parentObjectHandle,positionAndEulerAngles,labelColors,backgroundColors,size);
        retVal=App::currentWorld->bannerCont->addObject(it);

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simRemoveBanner_internal(int bannerID)
{ // deprecated on 23.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (bannerID==sim_handle_all)
            App::currentWorld->bannerCont->eraseAllObjects(false);
        else
        {
            int handleFlags=0;
            if (bannerID>=0)
            {
                handleFlags=bannerID&0xff00000;
                bannerID=bannerID&0xfffff;
            }
            CBannerObject* it=App::currentWorld->bannerCont->getObject(bannerID);
            if (it==nullptr)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_INEXISTANT);
                return(-1);
            }
            if (handleFlags&sim_handleflag_togglevisibility)
            {
                if (it->toggleVisibility())
                    return(1);
                return(0);
            }
            else
                App::currentWorld->bannerCont->removeObject(bannerID);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simAddGhost_internal(int ghostGroup,int objectHandle,int options,double startTime,double endTime,const float* color)
{ // deprecated on 23.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        int retVal=App::currentWorld->ghostObjectCont->addGhost(ghostGroup,objectHandle,options,startTime,endTime,color);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simModifyGhost_internal(int ghostGroup,int ghostId,int operation,double floatValue,int options,int optionsMask,const double* colorOrTransformation)
{ // deprecated on 23.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=App::currentWorld->ghostObjectCont->modifyGhost(ghostGroup,ghostId,operation,floatValue,options,optionsMask,colorOrTransformation);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetGraphUserData_internal(int graphHandle,const char* streamName,double data)
{ // deprecated on 23.11.2020
    TRACE_C_API;

    if (!App::currentWorld->simulation->isSimulationRunning())
    {
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isGraph(__func__,graphHandle))
            return(-1);
        CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
        CGraphData_old* stream=it->getGraphData(streamName);
        if (stream==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_DATA_STREAM);
            return(-1);
        }
        if (stream->getDataType()!=GRAPH_NOOBJECT_USER_DEFINED)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_DATA_STREAM_NOT_USER_DEFINED);
            return(-1);
        }
        stream->setUserData(data);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simAddPointCloud_internal(int pageMask,int layerMask,int objectHandle,int options,double pointSize,int ptCnt,const double* pointCoordinates,const char* defaultColors,const char* pointColors,const double* pointNormals)
{ // deprecated on 23.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1;
        CPtCloud_old* ptCloud=new CPtCloud_old(pageMask,layerMask,objectHandle,options,pointSize,ptCnt,pointCoordinates,(unsigned char*)pointColors,pointNormals,(unsigned char*)defaultColors);
        retVal=App::currentWorld->pointCloudCont->addObject(ptCloud);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simModifyPointCloud_internal(int pointCloudHandle,int operation,const int* intParam,const double* floatParam)
{ // deprecated on 23.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (operation==0)
        {
            if (App::currentWorld->pointCloudCont->removeObject(pointCloudHandle))
                return(1);
        }
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simCopyMatrix_internal(const double* matrixIn,double* matrixOut)
{ // deprecated on 23.11.2020
    TRACE_C_API;

    for (int i=0;i<12;i++)
        matrixOut[i]=matrixIn[i];
    return(1);
}

int simAddModuleMenuEntry_internal(const char* entryLabel,int itemCount,int* itemHandles)
{ // deprecated on 04.05.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (itemCount==1)
            itemHandles[0]=App::worldContainer->moduleMenuItemContainer->addMenuItem(entryLabel,-1);
        else
        {
            std::string s(entryLabel);
            s+="\n";
            for (int i=0;i<itemCount;i++)
                itemHandles[i]=App::worldContainer->moduleMenuItemContainer->addMenuItem(s.c_str(),-1);
        }
        return(1);
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OPERATION_FAILED);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetModuleMenuItemState_internal(int itemHandle,int state,const char* label)
{ // deprecated on 04.05.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CModuleMenuItem* it=App::worldContainer->moduleMenuItemContainer->getItemFromHandle(itemHandle);
        if (it!=nullptr)
        {
            if (state!=-1)
                it->setState(state);
            if (label!=nullptr)
                it->setLabel(label);
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ITEM_HANDLE);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

char* simGetObjectName_internal(int objectHandle)
{ // deprecated on 08.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handle=objectHandle;
        int handleFlags=0;
        if (objectHandle>=0)
        {
            handleFlags=objectHandle&0x0ff00000;
            handle=objectHandle&0x000fffff;
        }
        if (!doesObjectExist(__func__,handle))
            return(nullptr);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(handle);
        std::string nm;
        if ((handleFlags&sim_handleflag_altname)!=0)
            nm=it->getObjectAltName_old();
        else
            nm=it->getObjectName_old();
        char* retVal=new char[nm.length()+1];
        for (size_t i=0;i<nm.length();i++)
            retVal[i]=nm[i];
        retVal[nm.length()]=0;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simSetObjectName_internal(int objectHandle,const char* objectName)
{ // deprecated on 08.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int handle=objectHandle;
        int handleFlags=0;
        if (objectHandle>=0)
        {
            handleFlags=objectHandle&0x0ff00000;
            handle=objectHandle&0x000fffff;
        }
        if (!doesObjectExist(__func__,handle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(handle);
        std::string originalText(objectName);
        if (originalText.length()>127)
        {
            if ((handleFlags&sim_handleflag_silenterror)==0)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_OBJECT_NAME);
            return(-1);
        }
        if (originalText.length()<1)
        {
            if ((handleFlags&sim_handleflag_silenterror)==0)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_OBJECT_NAME);
            return(-1);
        }
        std::string text(objectName);
        if ((handleFlags&sim_handleflag_altname)!=0)
            tt::removeAltNameIllegalCharacters(text);
        else
            tt::removeIllegalCharacters(text,true);
        if (originalText!=text)
        {
            if ((handleFlags&sim_handleflag_silenterror)==0)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_OBJECT_NAME);
            return(-1);
        }
        std::string oldName;
        if ((handleFlags&sim_handleflag_altname)!=0)
            oldName=it->getObjectAltName_old();
        else
            oldName=it->getObjectName_old();

        if (oldName.compare(text)==0)
            return(1);
        bool err;
        if ((handleFlags&sim_handleflag_altname)!=0)
            err=(App::currentWorld->sceneObjects->getObjectFromAltName_old(text.c_str())!=nullptr);
        else
            err=(App::currentWorld->sceneObjects->getObjectFromName_old(text.c_str())!=nullptr);
        if (err)
        {
            if ((handleFlags&sim_handleflag_silenterror)==0)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_OBJECT_NAME);
            return(-1);
        }
        if ((handleFlags&sim_handleflag_altname)!=0)
            App::currentWorld->sceneObjects->setObjectAltName_old(it,text.c_str(),true);
        else
        {
            App::currentWorld->sceneObjects->setObjectName_old(it,text.c_str(),true);
            App::setFullDialogRefreshFlag();
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

char* simGetScriptName_internal(int scriptHandle)
{ // deprecated on 08.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(nullptr);
        }
        std::string name(it->getScriptPseudoName_old());
        char* retVal=new char[name.length()+1];
        for (int i=0;i<int(name.length());i++)
            retVal[i]=name[i];
        retVal[name.length()]=0;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simGetScriptHandle_internal(const char* targetAtScriptName)
{ // deprecated on 16.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    std::string scriptName(targetAtScriptName);
    std::string targetName;

    std::string targetAtScriptNm(targetAtScriptName);
    size_t p=targetAtScriptNm.find('@');
    if (p!=std::string::npos)
    {
        scriptName.assign(targetAtScriptNm.begin()+p+1,targetAtScriptNm.end());
        targetName.assign(targetAtScriptNm.begin(),targetAtScriptNm.begin()+p);
    }

    bool useAlias=( (scriptName.size()>0)&&((scriptName[0]=='/')||(scriptName[0]=='.')||(scriptName[0]==':')) );
    if (!useAlias)
        scriptName=getIndexAdjustedObjectName(scriptName.c_str());

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=nullptr;
        if ((targetName.length()==0)||(targetName.compare("child")==0)||(targetName.compare("main")==0))
        {
            if (scriptName.length()==0)
                it=App::currentWorld->embeddedScriptContainer->getMainScript();
            else
            {
                CSceneObject* obj=nullptr;
                if (useAlias)
                    obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,scriptName.c_str(),0,nullptr);
                else
                    obj=App::currentWorld->sceneObjects->getObjectFromName_old(scriptName.c_str());
                if (obj!=nullptr)
                    it=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,obj->getObjectHandle());
            }
        }
        if (targetName.compare("customization")==0)
        {
            CSceneObject* obj=nullptr;
            if (useAlias)
                obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,scriptName.c_str(),0,nullptr);
            else
                obj=App::currentWorld->sceneObjects->getObjectFromName_old(scriptName.c_str());
            if (obj!=nullptr)
                it=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,obj->getObjectHandle());
        }
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        if (it->getFlaggedForDestruction())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_WAS_DESTROYED);
            return(-1);
        }
        int retVal=it->getScriptHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetScriptVariable_internal(int scriptHandleOrType,const char* variableNameAtScriptName,int stackHandle)
{ // deprecated on 16.06.2021
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CScriptObject* script=nullptr;
        std::string variableName;
        if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
        { // script is identified by its ID
            std::string varNameAtScriptName(variableNameAtScriptName);
            size_t p=varNameAtScriptName.find('@');
            if (p!=std::string::npos)
                variableName.assign(varNameAtScriptName.begin(),varNameAtScriptName.begin()+p);
            else
                variableName=varNameAtScriptName;
            script=App::worldContainer->getScriptFromHandle(scriptHandleOrType);
        }
        else
        {
            std::string scriptName;
            std::string varNameAtScriptName(variableNameAtScriptName);
            size_t p=varNameAtScriptName.find('@');
            if (p!=std::string::npos)
            {
                scriptName.assign(varNameAtScriptName.begin()+p+1,varNameAtScriptName.end());
                variableName.assign(varNameAtScriptName.begin(),varNameAtScriptName.begin()+p);
            }
            else
                variableName=varNameAtScriptName;

            if (scriptHandleOrType==sim_scripttype_mainscript)
                script=App::currentWorld->embeddedScriptContainer->getMainScript();
            if (scriptHandleOrType==sim_scripttype_addonscript)
            {
                if (scriptName.size()>0)
                    script=App::worldContainer->addOnScriptContainer->getAddOnFromName(scriptName.c_str());
            }
            if (scriptHandleOrType==sim_scripttype_sandboxscript)
                script=App::worldContainer->sandboxScript;
            if ( (scriptHandleOrType==sim_scripttype_childscript)||(scriptHandleOrType==sim_scripttype_customizationscript) )
            {
                if (scriptName.size()>0)
                {
                    int objId=-1;
                    CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,scriptName.c_str(),0,nullptr);
                    if (obj!=nullptr)
                        objId=obj->getObjectHandle();
                    else
                        objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(scriptName.c_str());
                    if (scriptHandleOrType==sim_scripttype_customizationscript)
                        script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objId);
                    else
                        script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objId);
                }
            }
        }

        if (script!=nullptr)
        {
            bool doAClear=(stackHandle==0);
            CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
            if ( (stack==nullptr)&&(!doAClear) )
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
                return(-1);
            }
            if ( (stack!=nullptr)&&(stack->getStackSize()==0) )
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
                return(-1);
            }
            int retVal=-1; // error
            if (script->getThreadedExecutionIsUnderWay_oldThreads())
            { // very special handling here!
                if (VThread::areThreadIDsSame(script->getThreadedScriptThreadId_old(),VThread::getCurrentThreadId()))
                    retVal=script->setScriptVariable_old(variableName.c_str(),stack);
                else
                { // we have to execute that function via another thread!
                    void* d[4];
                    int callType=2;
                    d[0]=&callType;
                    d[1]=script;
                    d[2]=(void*)variableName.c_str();
                    d[3]=stack;

                    retVal=CThreadPool_old::callRoutineViaSpecificThread(script->getThreadedScriptThreadId_old(),d);
                }
            }
            else
            {
                if (VThread::isCurrentThreadTheMainSimulationThread())
                { // For now we don't allow non-main threads to call non-threaded scripts!
                    retVal=script->setScriptVariable_old(variableName.c_str(),stack);
                }
            }

            if (retVal==-1)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OPERATION_FAILED);
            return(retVal);
        }

        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetScript_internal(int index)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (index<0)||(index>=int(App::currentWorld->embeddedScriptContainer->allScripts.size())) )
            return(-1);
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->allScripts[index];
        int retVal=it->getScriptHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetScriptAssociatedWithObject_internal(int objectHandle)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objectHandle);
        if (it==nullptr)
            return(-1);
        int retVal=it->getScriptHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetCustomizationScriptAssociatedWithObject_internal(int objectHandle)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objectHandle);
        if (it==nullptr)
            return(-1);
        int retVal=it->getScriptHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetObjectAssociatedWithScript_internal(int scriptHandle)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        int retVal=-1;
        if (it->getScriptType()==sim_scripttype_childscript)
                retVal=it->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_childscript);
        if (it->getScriptType()==sim_scripttype_customizationscript)
                retVal=it->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_customizationscript);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

char* simGetObjectConfiguration_internal(int objectHandle)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(nullptr);

        std::vector<char> data;
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (it!=nullptr)
        {
            CMemorizedConf_old temp(it);
            temp.serializeToMemory(data);
            char* retBuffer=new char[data.size()+sizeof(int)];
            ((int*)retBuffer)[0]=int(data.size());
            for (size_t i=0;i<data.size();i++)
                retBuffer[sizeof(int)+i]=data[i];
            return(retBuffer);
        }
        return(nullptr);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simSetObjectConfiguration_internal(const char* data)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (data!=nullptr)
        {
            std::vector<char> arr;
            int l=((int*)data)[0];
            for (int i=0;i<l;i++)
                arr.push_back(data[i+sizeof(int)]);
            CMemorizedConf_old temp;
            temp.serializeFromMemory(arr);
            temp.restore();
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

char* simGetConfigurationTree_internal(int objectHandle)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (objectHandle!=sim_handle_all)
        {
            if (!doesObjectExist(__func__,objectHandle))
                return(nullptr);
        }
        if (objectHandle==sim_handle_all)
            objectHandle=-1;

        std::vector<char> data;
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        std::vector<CSceneObject*> sel;
        if (it==nullptr)
        { // We memorize everything:
            for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectFromIndex(i));
        }
        else
        { // We memorize just the object and all its children:
            it->getAllObjectsRecursive(&sel,true,true);
        }
        for (size_t i=0;i<sel.size();i++)
        {
            CMemorizedConf_old temp(sel[i]);
            temp.serializeToMemory(data);
        }
        char* retBuffer=new char[data.size()+sizeof(int)];
        ((int*)retBuffer)[0]=int(data.size());
        for (size_t i=0;i<data.size();i++)
            retBuffer[sizeof(int)+i]=data[i];
        return(retBuffer);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simSetConfigurationTree_internal(const char* data)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (data!=nullptr)
        {
            int l=((int*)data)[0];
            std::vector<char> arr;
            for (int i=0;i<l;i++)
                arr.push_back(data[i+sizeof(int)]);
            std::vector<CMemorizedConf_old*> allConfs;
            std::vector<int> parentCount;
            std::vector<int> index;
            while (arr.size()!=0)
            {
                CMemorizedConf_old* temp=new CMemorizedConf_old();
                temp->serializeFromMemory(arr);
                parentCount.push_back(temp->getParentCount());
                index.push_back((int)index.size());
                allConfs.push_back(temp);
            }
            tt::orderAscending(parentCount,index);
            for (size_t i=0;i<index.size();i++)
            {
                allConfs[index[i]]->restore();
                delete allConfs[index[i]];
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simEnableEventCallback_internal(int eventCallbackType,const char* plugin,int reserved)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (CPluginContainer::enableOrDisableSpecificEventCallback(eventCallbackType,plugin))
            return(1);
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simSetObjectSizeValues_internal(int objectHandle,const double* sizeValues)
{ // deprecated on 28.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (doesObjectExist(__func__,objectHandle))
        {
            CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            obj->setSizeValues(sizeValues);
            return(1);
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetObjectSizeValues_internal(int objectHandle,double* sizeValues)
{ // deprecated on 28.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (doesObjectExist(__func__,objectHandle))
        {
            CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            obj->getSizeValues(sizeValues);
            return(1);
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

char* simFileDialog_internal(int mode,const char* title,const char* startPath,const char* initName,const char* extName,const char* ext)
{ // deprecated on 07.09.2021
    TRACE_C_API;
#ifdef SIM_WITH_GUI
    if (!isSimulatorInitialized(__func__))
        return(nullptr);
    char* retVal=nullptr;

    std::string nameAndPath;
    bool native=1;
    #ifndef WIN_SIM // native dialogs have a bug on MacOS/Linux versions: the initial directory is not set. Because of that, we don't use native dialogs
        native=0;
    #endif
    CPluginContainer::customUi_fileDialog(mode,title,startPath,initName,extName,ext,native,nameAndPath);

/*
    std::string stPath(startPath);
    if (stPath.length()==0)
        stPath=App::directories->executableDirectory;
    nameAndPath=App::uiThread->getOpenOrSaveFileName_api(mode,title,stPath.c_str(),initName,extName,ext);
    */
    if (nameAndPath.length()!=0)
    {
        retVal=new char[nameAndPath.length()+1];
        for (size_t i=0;i<nameAndPath.length();i++)
            retVal[i]=nameAndPath[i];
        retVal[nameAndPath.length()]=0; // terminal 0
    }
    return(retVal);
#else
    return(nullptr);
#endif
}

int simMsgBox_internal(int dlgType,int buttons,const char* title,const char* message)
{ // deprecated on 07.09.2021
    TRACE_C_API;
    if (!isSimulatorInitialized(__func__))
        return(-1);
    int retVal=sim_msgbox_return_ok;
#ifdef SIM_WITH_GUI
    retVal=CPluginContainer::customUi_msgBox(dlgType,buttons,title,message,sim_msgbox_return_ok);
#endif
    return(retVal);
}

int simDisplayDialog_internal(const char* titleText,const char* mainText,int dialogType,const char* initialText,const float* titleColors,const float* dialogColors,int* elementHandle)
{ // deprecated on 07.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
#ifdef SIM_WITH_GUI
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->createStack();
        stack->pushStringOntoStack(titleText,0);
        stack->pushStringOntoStack(mainText,0);
        stack->pushInt32OntoStack(dialogType);
        stack->pushBoolOntoStack(false);
        if (initialText!=nullptr)
            stack->pushStringOntoStack(initialText,0);
        simCallScriptFunctionEx_internal(sim_scripttype_sandboxscript,"sim.displayDialog",stack->getId());
        int retVal;
        stack->getStackInt32Value(retVal);
        App::worldContainer->interfaceStackContainer->destroyStack(stack);
        return(retVal);
#else
        return(1);
#endif
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simGetDialogResult_internal(int genericDialogHandle)
{ // deprecated on 07.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->createStack();;
        stack->pushInt32OntoStack(genericDialogHandle);
        simCallScriptFunctionEx_internal(sim_scripttype_sandboxscript,"sim.getDialogResult",stack->getId());
        int retVal;
        stack->getStackInt32Value(retVal);
        App::worldContainer->interfaceStackContainer->destroyStack(stack);
        return(retVal);
#else
        return(sim_dlgret_cancel);
#endif
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

char* simGetDialogInput_internal(int genericDialogHandle)
{ // deprecated on 07.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::string tmp;
#ifdef SIM_WITH_GUI
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->createStack();
        stack->pushInt32OntoStack(genericDialogHandle);
        simCallScriptFunctionEx_internal(sim_scripttype_sandboxscript,"sim.getDialogInput",stack->getId());
        bool r=stack->getStackStringValue(tmp);
        App::worldContainer->interfaceStackContainer->destroyStack(stack);
        if (!r)
            return(nullptr);
#else
        return(nullptr);
#endif
        char* retVal=new char[tmp.length()+1];
        retVal[tmp.length()]=0;
        for (int i=0;i<int(tmp.length());i++)
            retVal[i]=tmp[i];
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simEndDialog_internal(int genericDialogHandle)
{ // deprecated on 07.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
#ifdef SIM_WITH_GUI
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->createStack();
        stack->pushInt32OntoStack(genericDialogHandle);
        simCallScriptFunctionEx_internal(sim_scripttype_sandboxscript,"sim.endDialog",stack->getId());
        App::worldContainer->interfaceStackContainer->destroyStack(stack);
#endif
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simIsObjectInSelection_internal(int objectHandle)
{ // deprecated on 24.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;
        if (App::currentWorld->sceneObjects->isObjectInSelection(objectHandle))
            retVal|=1;
        CSceneObject* lastSel=App::currentWorld->sceneObjects->getLastSelectionObject();
        if (lastSel!=nullptr)
        {
            if (lastSel->getObjectHandle()==objectHandle)
                retVal|=2;
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simAddObjectToSelection_internal(int what,int objectHandle)
{ // deprecated on 24.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
            return(-1);
        if (what==sim_handle_all)
            App::currentWorld->sceneObjects->selectAllObjects();
        else
        {
            if (!doesObjectExist(__func__,objectHandle))
                return(-1);
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            if (what==sim_handle_single)
                App::currentWorld->sceneObjects->addObjectToSelection(objectHandle);
            else
            {
                if ((what==sim_handle_tree)||(what==sim_handle_chain))
                {
                    std::vector<CSceneObject*> allObjects;
                    if (what==sim_handle_tree)
                        it->getAllObjectsRecursive(&allObjects,true,true);
                    if (what==sim_handle_chain)
                        it->getChain(allObjects,true,true);
                    for (int i=0;i<int(allObjects.size());i++)
                        App::currentWorld->sceneObjects->addObjectToSelection(allObjects[i]->getObjectHandle());
                }
                else
                {
                    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENT);
                    return(-1);
                }
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);

}

int simRemoveObjectFromSelection_internal(int what,int objectHandle)
{ // deprecated on 24.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
            return(-1);
        if (what==sim_handle_all)
            App::currentWorld->sceneObjects->deselectObjects();
        else
        {
            if (!doesObjectExist(__func__,objectHandle))
            {
                return(-1);
            }
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            if (what==sim_handle_single)
                App::currentWorld->sceneObjects->removeObjectFromSelection(objectHandle);
            else
            {
                if ((what==sim_handle_tree)||(what==sim_handle_chain))
                {
                    std::vector<CSceneObject*> allObjects;
                    if (what==sim_handle_tree)
                        it->getAllObjectsRecursive(&allObjects,true,true);
                    if (what==sim_handle_chain)
                        it->getChain(allObjects,true,true);
                    for (int i=0;i<int(allObjects.size());i++)
                        App::currentWorld->sceneObjects->removeObjectFromSelection(allObjects[i]->getObjectHandle());
                }
                else
                {
                    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENT);
                    return(-1);
                }
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);

}

int simGetObjectSelectionSize_internal()
{ // deprecated on 24.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=int(App::currentWorld->sceneObjects->getSelectionCount());
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetObjectLastSelection_internal()
{ // deprecated on 24.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1;
        CSceneObject* it=App::currentWorld->sceneObjects->getLastSelectionObject();
        if (it!=nullptr)
            retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


int simGetObjectSelection_internal(int* objectHandles)
{ // deprecated on 24.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            objectHandles[i]=App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i);
        return(int(App::currentWorld->sceneObjects->getSelectionCount()));
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simScaleSelectedObjects_internal(double scalingFactor,bool scalePositionsToo)
{ // deprecated on 24.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
            return(-1);
        std::vector<int> sel;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        CSceneObjectOperations::scaleObjects(sel,scalingFactor,scalePositionsToo!=0);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simDeleteSelectedObjects_internal()
{ // deprecated on 24.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
            return(-1);
        std::vector<int> sel;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
        App::currentWorld->sceneObjects->deselectObjects();
        App::currentWorld->sceneObjects->eraseObjects(sel,true);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simGetObjectUniqueIdentifier_internal(int objectHandle,int* uniqueIdentifier)
{ // deprecated on 08.10.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (objectHandle!=sim_handle_all)&&(!doesObjectExist(__func__,objectHandle)) )
            return(-1);
        if (objectHandle!=sim_handle_all)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            uniqueIdentifier[0]=int(it->getObjectUid());
        }
        else
        { // for backward compatibility
            int p=0;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
            {
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromIndex(i);
                if ( (it->getObjectHandle()==objectHandle)||(objectHandle==sim_handle_all) )
                    uniqueIdentifier[p++]=int(it->getObjectUid());
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

void _simSetDynamicJointLocalTransformationPart2_internal(void* joint,const double* pos,const double* quat)
{ // deprecated on 08.11.2021
}

void _simSetDynamicForceSensorLocalTransformationPart2_internal(void* forceSensor,const double* pos,const double* quat)
{ // deprecated on 08.11.2021
}

void _simSetDynamicJointLocalTransformationPart2IsValid_internal(void* joint,bool valid)
{ // deprecated on 08.11.2021
}

void _simSetDynamicForceSensorLocalTransformationPart2IsValid_internal(void* forceSensor,bool valid)
{ // deprecated on 08.11.2021
}

int simBreakForceSensor_internal(int objectHandle)
{ // deprecated on 08.11.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isForceSensor(__func__,objectHandle))
            return(-1);
        if (App::currentWorld->simulation->isSimulationStopped())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
            return(-1);
        }
        CForceSensor* it=App::currentWorld->sceneObjects->getForceSensorFromHandle(objectHandle);
        CSceneObject* child=it->getChildFromIndex(0);
        if (child!=nullptr)
            App::currentWorld->sceneObjects->setObjectParent(child,nullptr,true);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

bool _simIsForceSensorBroken_internal(const void* forceSensor)
{ // deprecated on 08.11.2021
    TRACE_C_API;
    CSceneObject* child=((CForceSensor*)forceSensor)->getChildFromIndex(0);
    return(child==nullptr);
}

void _simGetDynamicForceSensorLocalTransformationPart2_internal(const void* forceSensor,double* pos,double* quat)
{ // deprecated on 08.11.2021
    TRACE_C_API;
    C7Vector tr;
    tr.setIdentity();
    tr.X.getData(pos);
    tr.Q.getData(quat);
}

int simGetJointMatrix_internal(int objectHandle,double* matrix)
{ // deprecated on 09.11.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        C7Vector trFull(it->getFullLocalTransformation());
        C7Vector trPart1(it->getLocalTransformation());
        C7Vector tr(trPart1.getInverse()*trFull);
        tr.getMatrix().getData(matrix);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetSphericalJointMatrix_internal(int objectHandle,const double* matrix)
{ // deprecated on 09.11.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        if (it->getJointType()!=sim_joint_spherical_subtype)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_JOINT_NOT_SPHERICAL);
            return(-1);
        }
        C4X4Matrix m;
        m.setData(matrix);
        it->setSphericalTransformation(C4Vector(m.M.getQuaternion()));
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetObjectHandleEx_internal(const char* objectAlias,int index,int proxy,int options)
{ // deprecated on 03.12.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CSceneObject* it=nullptr;
        std::string nm(objectAlias);
        size_t silentErrorPos=std::string(objectAlias).find("@silentError"); // Old, for backcompatibility
        std::string additionalMessage_backCompatibility;
        if ( (nm.size()>0)&&((nm[0]=='.')||(nm[0]==':')||(nm[0]=='/')) )
        {
            int objHandle=App::currentWorld->embeddedScriptContainer->getObjectHandleFromScriptHandle(_currentScriptHandle);
            CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(objHandle);
            CSceneObject* prox=App::currentWorld->sceneObjects->getObjectFromHandle(proxy);
            it=App::currentWorld->sceneObjects->getObjectFromPath(obj,nm.c_str(),index,prox);
        }
        else
        { // Old, for backcompatibility:
            size_t altPos=std::string(objectAlias).find("@alt");
            size_t firstAtPos=std::string(objectAlias).find("@");
            if (firstAtPos!=std::string::npos)
                nm.erase(nm.begin()+firstAtPos,nm.end());
            if (altPos==std::string::npos)
            { // handle retrieval via regular name
                nm=getIndexAdjustedObjectName(nm.c_str());
                it=App::currentWorld->sceneObjects->getObjectFromName_old(nm.c_str());
                if (it==nullptr)
                {
                    additionalMessage_backCompatibility+="\n\nSince CoppeliaSim V4.3.0, objects should be retrieved via a path and alias, e.g. \"./path/to/alias\", \":/path/to/alias\", \"/path/to/alias\", etc.";
                    additionalMessage_backCompatibility+="\nYou however tried to access an object in a way that doesn't follow the new notation, i.e. \"";
                    additionalMessage_backCompatibility+=objectAlias;
                    additionalMessage_backCompatibility+="\" wasn't found.";
                    additionalMessage_backCompatibility+="\nNote also that object aliases are distinct from object names, which are deprecated and not displayed anymore. Additionally, sim.getObject is preferred over sim.getObjectHandle.";
                    additionalMessage_backCompatibility+="\nMake sure to read the following page for additional details: https://www.coppeliarobotics.com/helpFiles/en/accessingSceneObjects.htm";
                }
            }
            else
                it=App::currentWorld->sceneObjects->getObjectFromAltName_old(nm.c_str()); // handle retrieval via alt name
        }

        if (it==nullptr)
        {
            if ( (silentErrorPos==std::string::npos)&&((options&1)==0) )
            {
                additionalMessage_backCompatibility=SIM_ERROR_OBJECT_INEXISTANT_OR_ILL_FORMATTED_PATH+additionalMessage_backCompatibility;
                CApiErrors::setLastWarningOrError(__func__,additionalMessage_backCompatibility.c_str());
            }
            return(-1);
        }
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetScriptAttribute_internal(int scriptHandle,int attributeID,double floatVal,int intOrBoolVal)
{ // deprecated on 05.01.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        int retVal=-1;
        if ( (attributeID==sim_customizationscriptattribute_activeduringsimulation)&&(it->getScriptType()==sim_scripttype_customizationscript) )
        {
            it->setCustScriptDisabledDSim_compatibilityMode_DEPRECATED(intOrBoolVal==0);
            retVal=1;
        }
        if ( (attributeID==sim_customizationscriptattribute_cleanupbeforesave)&&(it->getScriptType()==sim_scripttype_customizationscript) )
        {
            it->setCustomizationScriptCleanupBeforeSave_DEPRECATED(intOrBoolVal!=0);
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_executionorder)
        {
            it->setExecutionPriority_old(intOrBoolVal);
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_executioncount)
        {
            it->setNumberOfPasses(intOrBoolVal);
            retVal=1;
        }
        if ( (attributeID==sim_childscriptattribute_automaticcascadingcalls)&&(it->getScriptType()==sim_scripttype_childscript)&&(!it->getThreadedExecution_oldThreads()) )
        {
            it->setAutomaticCascadingCallsDisabled_old(intOrBoolVal==0);
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_enabled)//&&(it->getScriptType()==sim_scripttype_childscript) )
        {
            it->setScriptIsDisabled(intOrBoolVal==0);
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_debuglevel)
            retVal=1; // deprecated. Doesn't do anything


        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetScriptAttribute_internal(int scriptHandle,int attributeID,double* floatVal,int* intOrBoolVal)
{ // deprecated on 05.01.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        int retVal=-1;
        if ( (attributeID==sim_customizationscriptattribute_activeduringsimulation)&&(it->getScriptType()==sim_scripttype_customizationscript) )
        {
            if (it->getCustScriptDisabledDSim_compatibilityMode_DEPRECATED())
                intOrBoolVal[0]=0;
            else
                intOrBoolVal[0]=1;
            retVal=1;
        }
        if ( (attributeID==sim_customizationscriptattribute_cleanupbeforesave)&&(it->getScriptType()==sim_scripttype_customizationscript) )
        {
            if (it->getCustomizationScriptCleanupBeforeSave_DEPRECATED())
                intOrBoolVal[0]=1;
            else
                intOrBoolVal[0]=0;
            retVal=1;
        }
        if ( (attributeID==sim_childscriptattribute_automaticcascadingcalls)&&(it->getScriptType()==sim_scripttype_childscript)&&(!it->getThreadedExecution_oldThreads()) )
        {
            if (it->getAutomaticCascadingCallsDisabled_old())
                intOrBoolVal[0]=0;
            else
                intOrBoolVal[0]=1;
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_executionorder)
        {
            intOrBoolVal[0]=it->getExecutionPriority_old();
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_executioncount)
        {
            intOrBoolVal[0]=it->getNumberOfPasses();
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_debuglevel)
        {
            intOrBoolVal[0]=sim_scriptdebug_none; // deprecated, doesn't work anymore
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_scripttype)
        {
            intOrBoolVal[0]=it->getScriptType();
            if (it->getThreadedExecution_oldThreads())
                intOrBoolVal[0]|=sim_scripttype_threaded_old;
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_scripthandle)
        {
            intOrBoolVal[0]=it->getScriptHandle();
            retVal=1;
        }

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetScriptText_internal(int scriptHandle,const char* scriptText)
{ // deprecated on 04.02.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }

#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
            App::mainWindow->codeEditorContainer->closeFromScriptHandle(scriptHandle,nullptr,true);
#endif
        it->setScriptText(scriptText);
        if ( (it->getScriptType()!=sim_scripttype_childscript)||(!it->getThreadedExecution_oldThreads())||App::currentWorld->simulation->isSimulationStopped() )
            it->resetScript();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

const char* simGetScriptText_internal(int scriptHandle)
{ // deprecated on 04.02.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (App::currentWorld->environment->getSceneLocked())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCENE_LOCKED);
            return(nullptr);
        }
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(nullptr);
        }
        const char* retVal=nullptr;

#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
            App::mainWindow->codeEditorContainer->closeFromScriptHandle(scriptHandle,nullptr,false);
        else
#endif
            retVal=it->getScriptText();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simGetScriptProperty_internal(int scriptHandle,int* scriptProperty,int* associatedObjectHandle)
{ // deprecated on 04.02.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        scriptProperty[0]=it->getScriptType();
        associatedObjectHandle[0]=it->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_childscript);
        if (it->getThreadedExecution_oldThreads())
            scriptProperty[0]|=sim_scripttype_threaded_old;
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetJointMaxForce_internal(int jointHandle,double* forceOrTorque)
{ // deprecated on 24.02.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,jointHandle))
            return(-1);
        if (!isJoint(__func__,jointHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
        forceOrTorque[0]=it->getTargetForce(false);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetJointMaxForce_internal(int objectHandle,double forceOrTorque)
{ // deprecated on 24.02.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        it->setTargetForce(forceOrTorque,false);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

void _simSetGeomProxyDynamicsFullRefreshFlag_internal(void* geomData,bool flag)
{ // deprecated on 03.03.2022. Has no effect
}

bool _simGetGeomProxyDynamicsFullRefreshFlag_internal(const void* geomData)
{ // deprecated on 03.03.2022. Has no effect
    return(0);
}

int simRemoveObject_internal(int objectHandle)
{ // deprecated on 07.03.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (objectHandle==sim_handle_all)
        {
            App::currentWorld->sceneObjects->eraseAllObjects(true);
            return(1);
        }
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_INEXISTANT);
            return(-1);
        }

        // Memorize the selection:
        std::vector<int> initSel;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            initSel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));

        App::currentWorld->sceneObjects->eraseObject(it,true);

        // Restore the initial selection:
        App::currentWorld->sceneObjects->deselectObjects();
        for (size_t i=0;i<initSel.size();i++)
            App::currentWorld->sceneObjects->addObjectToSelection(initSel[i]);

        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

unsigned int simGetSystemTimeInMs_internal(int previousTime)
{ // deprecated on 01.04.2022
    TRACE_C_API;
    unsigned int retVal;
    if (previousTime<-1)
        retVal=VDateTime::getOSTimeInMs();
    else
    {
        if (previousTime==-1)
            retVal=(unsigned int)VDateTime::getTimeInMs();
        else
            retVal=(unsigned int)VDateTime::getTimeDiffInMs(previousTime);
    }
    return(retVal);
}

float* simGetVisionSensorImage_internal(int sensorHandle)
{ // deprecated on 11.04.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=sensorHandle&0xff00000;
        sensorHandle=sensorHandle&0xfffff;
        if (!doesObjectExist(__func__,sensorHandle))
            return(nullptr);
        if (!isVisionSensor(__func__,sensorHandle))
            return(nullptr);
        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        int res[2];
        it->getResolution(res);
        int valPerPixel=3;
        if ((handleFlags&sim_handleflag_greyscale)!=0)
            valPerPixel=1;
        float* buff=new float[res[0]*res[1]*valPerPixel];
        unsigned char* imgBuff=it->getRgbBufferPointer();
        if ((handleFlags&sim_handleflag_greyscale)!=0)
        {
            for (int i=0;i<res[0]*res[1];i++)
            {
                float v=float(imgBuff[3*i+0])/255.0;
                v+=float(imgBuff[3*i+1])/255.0;
                v+=float(imgBuff[3*i+2])/255.0;
                buff[i]=v/3.0;
            }
        }
        else
        {
            for (int i=0;i<res[0]*res[1]*3;i++)
                buff[i]=float(imgBuff[i])/255.0;
        }
        return(buff);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simSetVisionSensorImage_internal(int sensorHandle,const float* image)
{ // deprecated on 11.04.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int handleFlags=sensorHandle&0xff00000;
        int objectHandle=sensorHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isVisionSensor(__func__,objectHandle))
            return(-1);
        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(objectHandle);
        int retVal=0;
        if (handleFlags&sim_handleflag_depthbuffer)
            it->setDepthBuffer(image);
        else
        {
            if (it->setExternalImage_old(image,(handleFlags&sim_handleflag_greyscale)!=0,(handleFlags&sim_handleflag_rawvalue)!=0))
                retVal=1;
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

unsigned char* simGetVisionSensorCharImage_internal(int sensorHandle,int* resolutionX,int* resolutionY)
{ // deprecated on 11.04.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=sensorHandle&0xff00000;
        sensorHandle=sensorHandle&0xfffff;
        if (!doesObjectExist(__func__,sensorHandle))
            return(nullptr);
        if (!isVisionSensor(__func__,sensorHandle))
            return(nullptr);
        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        int res[2];
        it->getResolution(res);
        if (resolutionX!=nullptr)
            resolutionX[0]=res[0];
        if (resolutionY!=nullptr)
            resolutionY[0]=res[1];

        int valPerPixel=3;
        if ((handleFlags&sim_handleflag_greyscale)!=0)
            valPerPixel=1;
        unsigned char* buff=new unsigned char[res[0]*res[1]*valPerPixel];
        unsigned char* imgBuff=it->getRgbBufferPointer();
        if ((handleFlags&sim_handleflag_greyscale)!=0)
        {
            int n=res[0]*res[1];
            for (int i=0;i<n;i++)
            {
                unsigned int v=imgBuff[3*i+0];
                v+=imgBuff[3*i+1];
                v+=imgBuff[3*i+2];
                buff[i]=(unsigned char)(v/3);
            }
        }
        else
        {
            int n=res[0]*res[1]*3;
            for (int i=0;i<n;i++)
                buff[i]=imgBuff[i];
        }
        return(buff);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simSetVisionSensorCharImage_internal(int sensorHandle,const unsigned char* image)
{ // deprecated on 11.04.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int handleFlags=sensorHandle&0xff00000;
        int objectHandle=sensorHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isVisionSensor(__func__,objectHandle))
            return(-1);
        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(objectHandle);
        int retVal=0;
        if (it->setExternalCharImage_old(image,(handleFlags&sim_handleflag_greyscale)!=0,(handleFlags&sim_handleflag_rawvalue)!=0))
            retVal=1;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

float* simGetVisionSensorDepthBuffer_internal(int sensorHandle)
{ // deprecated on 11.04.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=sensorHandle&0xff00000;
        sensorHandle=sensorHandle&0xfffff;
        if (!doesObjectExist(__func__,sensorHandle))
            return(nullptr);
        if (!isVisionSensor(__func__,sensorHandle))
            return(nullptr);
        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        int res[2];
        it->getResolution(res);
        float* buff=new float[res[0]*res[1]];
        float* depthBuff=it->getDepthBufferPointer();
        if ((handleFlags&sim_handleflag_depthbuffermeters)!=0)
        { // Here we need to convert values to distances in meters:
            float n=(float)it->getNearClippingPlane();
            float f=(float)it->getFarClippingPlane();
            float fmn=f-n;
            for (int i=0;i<res[0]*res[1];i++)
                buff[i]=n+fmn*depthBuff[i];
        }
        else
        { // values are: 0=on the close clipping plane, 1=on the far clipping plane
            for (int i=0;i<res[0]*res[1];i++)
                buff[i]=depthBuff[i];
        }
        return(buff);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simCreatePureShape_internal(int primitiveType,int options,const double* sizes,double mass,const int* precision)
{ // deprecated on 27.04.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int pType=0;
        C3Vector s(tt::getLimitedFloat(0.00001,100000.0,sizes[0]),tt::getLimitedFloat(0.00001,100000.0,sizes[1]),tt::getLimitedFloat(0.00001,100000.0,sizes[2]));
        int sides=32;
        if (precision!=nullptr)
        {
            if (pType==2)
                sides=tt::getLimitedInt(3,50,precision[1]); // sphere
            else
                sides=tt::getLimitedInt(0,50,precision[1]);
        }

        if (primitiveType==0) // cuboid
            pType=sim_primitiveshape_cuboid;
        if (primitiveType==1) // sphere
        {
            pType=sim_primitiveshape_spheroid;
            s(1)=s(0);
            s(2)=s(0);
        }
        if (primitiveType==2) // cylinder
        {
            pType=sim_primitiveshape_cylinder;
            s(1)=s(0);
        }
        if (primitiveType==3) // cone
        {
            pType=sim_primitiveshape_cone;
            s(1)=s(0);
        }
        if (pType==0)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_TYPE);
            return(-1);
        }
        int op=0;
        if ((options&1)!=0)
            op|=1;
        if ((options&4)==0)
            op|=2;
        if ((options&32)!=0)
            op|=4;
        CShape* shape=CAddOperations::addPrimitiveShape(pType,s,op,nullptr,0,sides,0,(options&16)==0,true);
        shape->setLocalTransformation(C7Vector::identityTransformation);
        shape->setVisibleEdges((options&2)!=0);
        shape->setRespondable((options&8)!=0);
        shape->getMeshWrapper()->setMass(tt::getLimitedFloat(0.000001,10000.0,mass));
        return(shape->getObjectHandle());
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

void* simBroadcastMessage_internal(int* auxiliaryData,void* customData,int* replyData)
{ // deprecated on 04.05.2022
    TRACE_C_API;

    if (replyData!=nullptr)
    {
        replyData[0]=-1;
        replyData[1]=-1;
        replyData[2]=-1;
        replyData[3]=-1;
    }
    void* retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_broadcast,auxiliaryData,customData,replyData);
    return(retVal);
}

void* simSendModuleMessage_internal(int message,int* auxiliaryData,void* customData,int* replyData)
{ // deprecated on 04.05.2022
    TRACE_C_API;

    if (replyData!=nullptr)
    {
        replyData[0]=-1;
        replyData[1]=-1;
        replyData[2]=-1;
        replyData[3]=-1;
    }
    void* retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(message,auxiliaryData,customData,replyData);
    return(retVal);
}

int simBuildMatrixQ_internal(const double* position,const double* quaternion,double* matrix)
{ // deprecated on 09.08.2022
    TRACE_C_API;

    C4X4Matrix m;
    C4Vector q(quaternion[3],quaternion[0],quaternion[1],quaternion[2]);
    m.M=q.getMatrix();
    m.X.setData(position);
    m.getData(matrix);
    return(1);
}

int simGetQuaternionFromMatrix_internal(const double* matrix,double* quaternion)
{ // deprecated on 09.08.2022
    TRACE_C_API;

    C4X4Matrix m;
    m.setData(matrix);
    C4Vector q(m.M.getQuaternion());
    quaternion[0]=q(1);
    quaternion[1]=q(2);
    quaternion[2]=q(3);
    quaternion[3]=q(0);
    return(1);
}

void _simGetLocalInertiaFrame_internal(const void* geomInfo,double* pos,double* quat)
{ // deprecated on 19.08.2022
    TRACE_C_API;
    C7Vector tr(((CMeshWrapper*)geomInfo)->getLocalInertiaFrame());
    tr.Q.getData(quat);
    tr.X.getData(pos);
}

void _simGetPrincipalMomentOfInertia_internal(const void* geomInfo,double* inertia)
{ // deprecated on 19.08.2022
    TRACE_C_API;
    ((CMeshWrapper*)geomInfo)->getPrincipalMomentsOfInertia().getData(inertia);
}

int simSetDoubleSignalOld_internal(const char* signalName,double signalValue)
{ // deprecated on 13.10.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        App::currentWorld->signalContainer->setDoubleSignal_old(signalName,signalValue,_currentScriptHandle);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetDoubleSignalOld_internal(const char* signalName,double* signalValue)
{ // deprecated on 13.10.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;

        if (App::currentWorld->signalContainer->getDoubleSignal_old(signalName,signalValue[0]))
            retVal=1;

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simClearDoubleSignalOld_internal(const char* signalName)
{ // deprecated on 13.10.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal;

        if (signalName==nullptr)
            retVal=App::currentWorld->signalContainer->clearAllDoubleSignals_old();
        else
            retVal=App::currentWorld->signalContainer->clearDoubleSignal_old(signalName);

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetShapeVertex_internal(int shapeHandle,int groupElementIndex,int vertexIndex,double* relativePosition)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,shapeHandle))
        {
            return(-1);
        }
        if (!isShape(__func__,shapeHandle))
        {
            return(-1);
        }
        if (App::currentWorld->environment->getSceneLocked())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCENE_LOCKED);
            return(-1);
        }
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        CMesh* cc=it->getMeshWrapper()->getShapeComponentAtIndex(groupElementIndex);
        if (cc==nullptr)
        {
            return(0);
        }
        std::vector<double> wvert;
        cc->getCumulativeMeshes(wvert,nullptr,nullptr);
        if ( (vertexIndex<0)||(vertexIndex>=int(wvert.size())/3) )
        {
            return(0);
        }
        relativePosition[0]=wvert[3*vertexIndex+0];
        relativePosition[1]=wvert[3*vertexIndex+1];
        relativePosition[2]=wvert[3*vertexIndex+2];
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetShapeTriangle_internal(int shapeHandle,int groupElementIndex,int triangleIndex,int* vertexIndices,double* triangleNormals)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,shapeHandle))
        {
            return(-1);
        }
        if (!isShape(__func__,shapeHandle))
        {
            return(-1);
        }
        if (App::currentWorld->environment->getSceneLocked())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCENE_LOCKED);
            return(-1);
        }
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        CMesh* cc=it->getMeshWrapper()->getShapeComponentAtIndex(groupElementIndex);
        if (cc==nullptr)
        {
            return(0);
        }
        std::vector<double> wvert;
        std::vector<int> wind;
        std::vector<double> wnorm;
        cc->getCumulativeMeshes(wvert,&wind,&wnorm);
        if ( (triangleIndex<0)||(triangleIndex>=int(wind.size())/3) )
        {
            return(0);
        }
        if (vertexIndices!=nullptr)
        {
            vertexIndices[0]=wind[3*triangleIndex+0];
            vertexIndices[1]=wind[3*triangleIndex+1];
            vertexIndices[2]=wind[3*triangleIndex+2];
        }
        if (triangleNormals!=nullptr)
        {
            triangleNormals[0]=wnorm[9*triangleIndex+0];
            triangleNormals[1]=wnorm[9*triangleIndex+1];
            triangleNormals[2]=wnorm[9*triangleIndex+2];
            triangleNormals[3]=wnorm[9*triangleIndex+3];
            triangleNormals[4]=wnorm[9*triangleIndex+4];
            triangleNormals[5]=wnorm[9*triangleIndex+5];
            triangleNormals[6]=wnorm[9*triangleIndex+6];
            triangleNormals[7]=wnorm[9*triangleIndex+7];
            triangleNormals[8]=wnorm[9*triangleIndex+8];
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

