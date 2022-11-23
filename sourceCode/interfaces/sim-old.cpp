#ifdef switchToDouble
// single precision, non-deprecated:
SIM_DLLEXPORT int simSetFloatParam(int parameter,floatFloat floatState)
{
    return(simSetFloatParam_internal(parameter,(double)floatState));
}
SIM_DLLEXPORT int simGetFloatParam(int parameter,floatFloat* floatState)
{
    double v;
    int retVal=simGetFloatParam_internal(parameter,&v);
    floatState[0]=(floatFloat)v;
    return(retVal);
}
SIM_DLLEXPORT int simGetObjectMatrix(int objectHandle,int relativeToObjectHandle,floatFloat* matrix)
{
    double m[12];
    int retVal=simGetObjectMatrix_internal(objectHandle,relativeToObjectHandle,m);
    for (size_t i=0;i<12;i++)
        matrix[i]=(floatFloat)m[i];
    return(retVal);
}
SIM_DLLEXPORT int simSetObjectMatrix(int objectHandle,int relativeToObjectHandle,const floatFloat* matrix)
{
    double m[12];
    for (size_t i=0;i<12;i++)
        m[i]=(double)matrix[i];
    return(simSetObjectMatrix_internal(objectHandle,relativeToObjectHandle,m));
}
SIM_DLLEXPORT int simGetObjectPose(int objectHandle,int relativeToObjectHandle,floatFloat* pose)
{
    double p[7];
    int retVal=simGetObjectPose_internal(objectHandle,relativeToObjectHandle,p);
    for (size_t i=0;i<7;i++)
        pose[i]=(floatFloat)p[i];
    return(retVal);
}
SIM_DLLEXPORT int simSetObjectPose(int objectHandle,int relativeToObjectHandle,const floatFloat* pose)
{
    double p[7];
    for (size_t i=0;i<7;i++)
        p[i]=(double)pose[i];
    return(simSetObjectPose_internal(objectHandle,relativeToObjectHandle,p));
}
SIM_DLLEXPORT int simGetObjectPosition(int objectHandle,int relativeToObjectHandle,floatFloat* position)
{
    double p[3];
    int retVal=simGetObjectPosition_internal(objectHandle,relativeToObjectHandle,p);
    for (size_t i=0;i<3;i++)
        position[i]=(floatFloat)p[i];
    return(retVal);
}
SIM_DLLEXPORT int simSetObjectPosition(int objectHandle,int relativeToObjectHandle,const floatFloat* position)
{
    double p[3];
    for (size_t i=0;i<3;i++)
        p[i]=(double)position[i];
    return(simSetObjectPosition_internal(objectHandle,relativeToObjectHandle,p));
}
SIM_DLLEXPORT int simGetObjectOrientation(int objectHandle,int relativeToObjectHandle,floatFloat* eulerAngles)
{
    double e[3];
    int retVal=simGetObjectOrientation_internal(objectHandle,relativeToObjectHandle,e);
    for (size_t i=0;i<3;i++)
        eulerAngles[i]=(floatFloat)e[i];
    return(retVal);
}
SIM_DLLEXPORT int simGetObjectQuaternion(int objectHandle,int relativeToObjectHandle,floatFloat* quaternion)
{
    double q[4];
    int retVal=simGetObjectQuaternion_internal(objectHandle,relativeToObjectHandle,q);
    for (size_t i=0;i<4;i++)
        quaternion[i]=(floatFloat)q[i];
    return(retVal);
}
SIM_DLLEXPORT int simSetObjectQuaternion(int objectHandle,int relativeToObjectHandle,const floatFloat* quaternion)
{
    double q[4];
    for (size_t i=0;i<4;i++)
        q[i]=(double)quaternion[i];
    return(simSetObjectQuaternion_internal(objectHandle,relativeToObjectHandle,q));
}
SIM_DLLEXPORT int simSetObjectOrientation(int objectHandle,int relativeToObjectHandle,const floatFloat* eulerAngles)
{
    double e[3];
    for (size_t i=0;i<3;i++)
        e[i]=(double)eulerAngles[i];
    return(simSetObjectOrientation_internal(objectHandle,relativeToObjectHandle,e));
}
SIM_DLLEXPORT int simGetJointPosition(int objectHandle,floatFloat* position)
{
    double p[3];
    int retVal=simGetJointPosition_internal(objectHandle,p);
    for (size_t i=0;i<3;i++)
        position[i]=(floatFloat)p[i];
    return(retVal);
}
SIM_DLLEXPORT int simSetJointPosition(int objectHandle,floatFloat position)
{
    return(simSetJointPosition_internal(objectHandle,(double)position));
}
SIM_DLLEXPORT int simSetJointTargetPosition(int objectHandle,floatFloat targetPosition)
{
    return(simSetJointTargetPosition_internal(objectHandle,(double)targetPosition));
}
SIM_DLLEXPORT int simGetJointTargetPosition(int objectHandle,floatFloat* targetPosition)
{
    double t;
    int retVal=simGetJointTargetPosition_internal(objectHandle,&t);
    targetPosition[0]=(floatFloat)t;
    return(retVal);
}
SIM_DLLEXPORT int simGetObjectChildPose(int objectHandle,floatFloat* pose)
{
    double p[7];
    int retVal=simGetObjectChildPose_internal(objectHandle,p);
    for (size_t i=0;i<7;i++)
        pose[i]=(floatFloat)p[i];
    return(retVal);
}
SIM_DLLEXPORT int simSetObjectChildPose(int objectHandle,const floatFloat* pose)
{
    double p[7];
    for (size_t i=0;i<7;i++)
        p[i]=(double)pose[i];
    return(simSetObjectChildPose_internal(objectHandle,p));
}
SIM_DLLEXPORT int simGetJointInterval(int objectHandle,bool* cyclic,floatFloat* interval)
{
    double interv[2];
    int retVal=simGetJointInterval_internal(objectHandle,cyclic,interv);
    for (size_t i=0;i<2;i++)
        interval[i]=(floatFloat)interv[i];
    return(retVal);
}
SIM_DLLEXPORT int simSetJointInterval(int objectHandle,bool cyclic,const floatFloat* interval)
{
    double interv[2];
    for (size_t i=0;i<2;i++)
        interv[i]=(double)interval[i];
    return(simSetJointInterval_internal(objectHandle,cyclic,interv));
}
SIM_DLLEXPORT int simBuildIdentityMatrix(floatFloat* matrix)
{
    double m[12];
    int retVal=simBuildIdentityMatrix_internal(m);
    for (size_t i=0;i<12;i++)
        matrix[i]=(floatFloat)m[i];
    return(retVal);
}
SIM_DLLEXPORT int simBuildMatrix(const floatFloat* position,const floatFloat* eulerAngles,floatFloat* matrix)
{
    double p[3];
    double e[3];
    double m[12];
    for (size_t i=0;i<2;i++)
    {
        p[i]=(double)position[i];
        e[i]=(double)eulerAngles[i];
    }
    int retVal=simBuildMatrix_internal(p,e,m);
    for (size_t i=0;i<12;i++)
        matrix[i]=(floatFloat)m[i];
    return(retVal);
}
SIM_DLLEXPORT int simBuildPose(const floatFloat* position,const floatFloat* eulerAngles,floatFloat* pose)
{
    double p[3];
    double e[3];
    double p[7];
    for (size_t i=0;i<2;i++)
    {
        p[i]=(double)position[i];
        e[i]=(double)eulerAngles[i];
    }
    int retVal=simBuildPose_internal(p,e,p);
    for (size_t i=0;i<7;i++)
        pose[i]=(floatFloat)p[i];
    return(retVal);
}
SIM_DLLEXPORT int simGetEulerAnglesFromMatrix(const floatFloat* matrix,floatFloat* eulerAngles)
{
    double m[12];
    for (size_t i=0;i<12;i++)
        m[i]=(double)matrix[i];
    double e[3];
    int retVal=simGetEulerAnglesFromMatrix_internal(m,e);
    for (size_t i=0;i<3;i++)
        eulerAngles[i]=(floatFloat)e[i];
    return(retVal);
}
SIM_DLLEXPORT int simInvertMatrix(floatFloat* matrix)
{
    double m[12];
    for (size_t i=0;i<12;i++)
        m[i]=(double)matrix[i];
    int retVal=simInvertMatrix_internal(m);
    for (size_t i=0;i<12;i++)
        matrix[i]=(floatFloat)m[i];
    return(retVal);
}
SIM_DLLEXPORT int simMultiplyMatrices(const floatFloat* matrixIn1,const floatFloat* matrixIn2,floatFloat* matrixOut)
{
    double m1[12];
    double m2[12];
    for (size_t i=0;i<12;i++)
    {
        m1[i]=(double)matrixIn1[i];
        m2[i]=(double)matrixIn2[i];
    }
    double m[12];
    int retVal=simMultiplyMatrices_internal(m1,m2,m);
    for (size_t i=0;i<12;i++)
        matrixOut[i]=(floatFloat)m[i];
    return(retVal);
}
SIM_DLLEXPORT int simMultiplyPoses(const floatFloat* poseIn1,const floatFloat* poseIn2,floatFloat* poseOut)
{
    double p1[7];
    double p2[7];
    for (size_t i=0;i<7;i++)
    {
        p1[i]=(double)poseIn1[i];
        p2[i]=(double)poseIn2[i];
    }
    double p[7];
    int retVal=simMultiplyPoses_internal(p1,p2,p);
    for (size_t i=0;i<7;i++)
        poseOut[i]=(floatFloat)p[i];
    return(retVal);
}
SIM_DLLEXPORT int simInvertPose(floatFloat* pose)
{
    double p[7];
    for (size_t i=0;i<7;i++)
        p[i]=(double)pose[i];
    int retVal=simInvertPose_internal(p);
    for (size_t i=0;i<7;i++)
        pose[i]=(floatFloat)p[i];
    return(retVal);
}
SIM_DLLEXPORT int simInterpolatePoses(const floatFloat* poseIn1,const floatFloat* poseIn2,floatFloat interpolFactor,floatFloat* poseOut)
{
    double p1[7];
    double p2[7];
    for (size_t i=0;i<7;i++)
    {
        p1[i]=(double)poseIn1[i];
        p2[i]=(double)poseIn2[i];
    }
    double p[7];
    int retVal=simInterpolatePoses_internal(p1,p2,(double)interpolFactor,p);
    for (size_t i=0;i<7;i++)
        poseOut[i]=(floatFloat)p[i];
    return(retVal);
}
SIM_DLLEXPORT int simPoseToMatrix(const floatFloat* poseIn,floatFloat* matrixOut)
{
    double p[7];
    for (size_t i=0;i<7;i++)
        p[i]=(double)poseIn[i];
    double m[12];
    int retVal=simPoseToMatrix_internal(p,m);
    for (size_t i=0;i<12;i++)
        matrixOut[i]=(floatFloat)m[i];
    return(retVal);
}
SIM_DLLEXPORT int simMatrixToPose(const floatFloat* matrixIn,floatFloat* poseOut)
{
    double m[12];
    for (size_t i=0;i<12;i++)
        m[i]=(double)matrixIn[i];
    double p[7];
    int retVal=simMatrixToPose_internal(m,p);
    for (size_t i=0;i<7;i++)
        poseOut[i]=(floatFloat)p[i];
    return(retVal);
}
SIM_DLLEXPORT int simInterpolateMatrices(const floatFloat* matrixIn1,const floatFloat* matrixIn2,floatFloat interpolFactor,floatFloat* matrixOut)
{
    double m1[12];
    double m2[12];
    for (size_t i=0;i<12;i++)
    {
        m1[i]=(double)matrixIn1[i];
        m2[i]=(double)matrixIn2[i];
    }
    double m[12];
    int retVal=simInterpolateMatrices_internal(m1,m2,(double)interpolFactor,m);
    for (size_t i=0;i<12;i++)
        matrixOut[i]=(floatFloat)m[i];
    return(retVal);
}
SIM_DLLEXPORT int simTransformVector(const floatFloat* matrix,floatFloat* vect)
{
    double m[12];
    for (size_t i=0;i<12;i++)
        m[i]=(double)matrix[i];
    double v[3];
    for (size_t i=0;i<3;i++)
        v[i]=(double)vect[i];
    int retVal=simTransformVector_internal(m,v);
    for (size_t i=0;i<3;i++)
        vect[i]=(floatFloat)v[i];
    return(retVal);
}
SIM_DLLEXPORT floatFloat simGetSimulationTime()
{
    return((floatFloat)simGetSimulationTime_internal());
}
SIM_DLLEXPORT floatFloat simGetSystemTime()
{
    return(floatFloat(simGetSystemTime_internal()));
}
SIM_DLLEXPORT int simHandleProximitySensor(int sensorHandle,floatFloat* detectedPoint,int* detectedObjectHandle,floatFloat* normalVector)
{
    double pt[3];
    double n[3];
    int retVal=simHandleProximitySensor_internal(sensorHandle,pt,detectedObjectHandle,n);
    if (retVal>0)
    {
        for (size_t i=0;i<3;i++)
        {
            if (detectedPoint!=nullptr)
                detectedPoint[i]=(floatFloat)pt[i];
            if (normalVector!=nullptr)
                normalVector[i]=(floatFloat)n[i];
        }
    }
    return(retVal);
}
SIM_DLLEXPORT int simReadProximitySensor(int sensorHandle,floatFloat* detectedPoint,int* detectedObjectHandle,floatFloat* normalVector)
{
    double pt[3];
    double n[3];
    int retVal=simReadProximitySensor_internal(sensorHandle,pt,detectedObjectHandle,n);
    if (retVal>0)
    {
        for (size_t i=0;i<3;i++)
        {
            if (detectedPoint!=nullptr)
                detectedPoint[i]=(floatFloat)pt[i];
            if (normalVector!=nullptr)
                normalVector[i]=(floatFloat)n[i];
        }
    }
    return(retVal);
}
SIM_DLLEXPORT int simHandleDynamics(floatFloat deltaTime)
{
    return(simHandleDynamics_internal((double)deltaTime));
}
SIM_DLLEXPORT int simCheckProximitySensor(int sensorHandle,int entityHandle,floatFloat* detectedPoint)
{
    double pt[3];
    int retVal=simCheckProximitySensor_internal(sensorHandle,entityHandle,pt);
    if ( (retVal>0)&&(detectedPoint!=nullptr) )
    {
        for (size_t i=0;i<3;i++)
            detectedPoint[i]=(floatFloat)pt[i];
    }
    return(retVal);
}
SIM_DLLEXPORT int simCheckProximitySensorEx(int sensorHandle,int entityHandle,int detectionMode,floatFloat detectionThreshold,floatFloat maxAngle,floatFloat* detectedPoint,int* detectedObjectHandle,floatFloat* normalVector)
{
    double pt[3];
    double n[3];
    int retVal=simCheckProximitySensorEx_internal(sensorHandle,entityHandle,detectionMode,(double)detectionThreshold,(double)maxAngle,pt,detectedObjectHandle,n);
    if (retVal>0)
    {
        for (size_t i=0;i<3;i++)
        {
            if (detectedPoint!=nullptr)
                detectedPoint[i]=(floatFloat)pt[i];
            if (normalVector!=nullptr)
                normalVector[i]=(floatFloat)n[i];
        }
    }
    return(retVal);
}
SIM_DLLEXPORT int simCheckProximitySensorEx2(int sensorHandle,floatFloat* vertexPointer,int itemType,int itemCount,int detectionMode,floatFloat detectionThreshold,floatFloat maxAngle,floatFloat* detectedPoint,floatFloat* normalVector)
{
    int c=3;
    if (itemType==1)
        c*=2;
    if (itemType==2)
        c*=3;
    double v[9];
    for (size_t i=0;i<c;i++)
        v[i]=(double)vertexPointer[i];
    double pt[3];
    double n[3];
    int retVal=simCheckProximitySensorEx2_internal(sensorHandle,vertexPointer,itemType,itemCount,detectionMode,(double)detectionThreshold,(double)maxAngle,pt,n);
    if (retVal>0)
    {
        for (size_t i=0;i<3;i++)
        {
            if (detectedPoint!=nullptr)
                detectedPoint[i]=(floatFloat)pt[i];
            if (normalVector!=nullptr)
                normalVector[i]=(floatFloat)n[i];
        }
    }
    return(retVal);
}
SIM_DLLEXPORT int simCheckCollisionEx(int entity1Handle,int entity2Handle,floatFloat** intersectionSegments)
{
    double* s;
    int retVal=simCheckCollisionEx_internal(entity1Handle,entity2Handle,&s);
    if (retVal>0)
    {
        intersectionSegments[0]=(floatFloat*)simCreateBuffer_internal(sizeof(floatFloat)*retVal*6);
        for (int i=0;i<retVal*6;i++)
            intersectionSegments[0][i]=(floatFloat)s[i];
        simReleaseBuffer_internal((char*)s);
    }
    return(retVal);
}
SIM_DLLEXPORT int simCheckDistance(int entity1Handle,int entity2Handle,floatFloat threshold,floatFloat* distanceData)
{
    double d[7];
    int retVal=simCheckDistance_internal(entity1Handle,entity2Handle,(double)threshold,d);
    if (retVal>0)
    {
        for (size_t i=0;i<7;i++)
            distanceData[i]=(floatFloat)d[i];
    }
    return(retVal);
}
SIM_DLLEXPORT int simSetSimulationTimeStep(floatFloat timeStep)
{
    return(simSetSimulationTimeStep_internal((double)timeStep));
}
SIM_DLLEXPORT floatFloat simGetSimulationTimeStep()
{
    return((double)simGetSimulationTimeStep_internal());
}
SIM_DLLEXPORT int simAdjustRealTimeTimer(int instanceIndex,floatFloat deltaTime)
{
    return(simAdjustRealTimeTimer_internal(instanceIndex,(double)deltaTime));
}
SIM_DLLEXPORT int simFloatingViewAdd(floatFloat posX,floatFloat posY,floatFloat sizeX,floatFloat sizeY,int options)
{
    return(simFloatingViewAdd_internal((double)posX,(double)posY,(double)sizeX,(double)sizeY,options));
}
SIM_DLLEXPORT int simHandleGraph(int graphHandle,floatFloat simulationTime)
{
    return(simHandleGraph_internal(graphHandle,(double)simulationTime));
}
SIM_DLLEXPORT int simAddGraphStream(int graphHandle,const char* streamName,const char* unitStr,int options,const floatFloat* color,floatFloat cyclicRange)
{
    double c[3];
    double* c_=nullptr;
    if (color!=nullptr)
    {
        c_=c;
        for (size_t i=0;i<3;i++)
            c[i]=(double)color[i];
    }
    return(simAddGraphStream_internal(graphHandle,streamName,unitStr,options,c_,(double)cyclicRange));
}
SIM_DLLEXPORT int simSetGraphStreamTransformation(int graphHandle,int streamId,int trType,floatFloat mult,floatFloat off,int movingAvgPeriod)
{
    return(simSetGraphStreamTransformation_internal(graphHandle,streamId,trType,(double)mult,(double)off,movingAvgPeriod));
}
SIM_DLLEXPORT int simAddGraphCurve(int graphHandle,const char* curveName,int dim,const int* streamIds,const floatFloat* defaultValues,const char* unitStr,int options,const floatFloat* color,int curveWidth)
{
    double defV[3];
    for (int i=0;i<dim;i++)
        defV[i]=(double)defaultValues[i];
    double c[3];
    double* c_=nullptr;
    if (color!=nullptr)
    {
        c_=c;
        for (size_t i=0;i<3;i++)
            c[i]=(double)color[i];
    }
    return(simAddGraphCurve_internal(graphHandle,curveName,dim,streamIds,defV,unitStr,options,c_,curveWidth));
}
SIM_DLLEXPORT int simSetGraphStreamValue(int graphHandle,int streamId,floatFloat value)
{
    return(simSetGraphStreamValue_internal(graphHandle,streamId,(double)value));
}
SIM_DLLEXPORT int simSetJointTargetVelocity(int objectHandle,floatFloat targetVelocity)
{
    return(simSetJointTargetVelocity_internal(objectHandle,(double)targetVelocity));
}
SIM_DLLEXPORT int simGetJointTargetVelocity(int objectHandle,floatFloat* targetVelocity)
{
    return(simGetJointTargetVelocity_internal(objectHandle,(double)targetVelocity));
}
SIM_DLLEXPORT int simScaleObjects(const int* objectHandles,int objectCount,floatFloat scalingFactor,bool scalePositionsToo)
{
    return(simScaleObjects_internal(objectHandles,objectCount,(double)scalingFactor,scalePositionsToo));
}
SIM_DLLEXPORT int simAddDrawingObject(int objectType,floatFloat size,floatFloat duplicateTolerance,int parentObjectHandle,int maxItemCount,const floatFloat* color,const floatFloat* setToNULL,const floatFloat* setToNULL2,const floatFloat* setToNULL3)
{
    double* c1_=nullptr;
    double* c2_=nullptr;
    double* c3_=nullptr;
    double* c4_=nullptr;
    double c1[3];
    double c2[3];
    double c3[3];
    double c4[3];
    for (size_t i=0;i<3;i++)
    {
        if (color!=nullptr)
        {
            c1[i]=color[i];
            c1_=c1;
        }
        if (setToNULL!=nullptr)
        {
            c2[i]=setToNULL[i];
            c2_=c2;
        }
        if (setToNULL2!=nullptr)
        {
            c3[i]=setToNULL2[i];
            c3_=c3;
        }
        if (setToNULL3!=nullptr)
        {
            c4[i]=setToNULL3[i];
            c4_=c4;
        }
    }
    return(simAddDrawingObject_internal(objectType,(double)size,(double)duplicateTolerance,parentObjectHandle,maxItemCount,c1_,c2_,c3_,c4_));
}
SIM_DLLEXPORT int simAddDrawingObjectItem(int objectHandle,const floatFloat* itemData)
{
    objectHandle=objectHandle&0xfffff;
    CDrawingObject* it=App::currentWorld->drawingCont->getObject(objectHandle);
    int retVal=-1;
    if (it!=nullptr)
    {
        double* d_=nullptr;
        std::vector<double> d;
        if (itemData!=nullptr)
        {
            int s=it->getExpectedFloatsPerItem();
            d.resize(s);
            for (int i=0;i<s;i++)
                d[i]=(double)itemData[i];
            d_=&d[0];
        }
        retVal=simAddDrawingObjectItem_internal(objectHandle,d_));
    }
    return(retVal);
}
SIM_DLLEXPORT floatFloat simGetObjectSizeFactor(int objectHandle)
{
    return((floatFloat)simGetObjectSizeFactor_internal(objectHandle));
}
SIM_DLLEXPORT int simSetFloatSignal(const char* signalName,floatFloat signalValue)
{
    return(simSetFloatSignal_internal(signalName,(double)signalValue));
}
SIM_DLLEXPORT int simGetFloatSignal(const char* signalName,floatFloat* signalValue)
{
    double v;
    int retVal=simGetFloatSignal_internal(signalName,&v);
    signalValue[0]=(floatFloat)v;
    return(retVal);
}
SIM_DLLEXPORT int simReadForceSensor(int objectHandle,floatFloat* forceVector,floatFloat* torqueVector)
{
    double f[3];
    double t[3];
    int retVal=simReadForceSensor_internal(objectHandle,f,t);
    if ((retVal&1)!=0)
    {
        for (size_t i=0;i<3;i++)
        {
            if (forceVector!=nullptr)
                forceVector[i]=(floatFloat)f[i];
            if (torqueVector!=nullptr)
                torqueVector[i]=(floatFloat)t[i];
        }
    }
    return(retVal);
}
SIM_DLLEXPORT int simSetLightParameters(int objectHandle,int state,const floatFloat* setToNULL,const floatFloat* diffusePart,const floatFloat* specularPart)
{
    double b[3];
    double b_=nullptr;
    double c[3];
    double c_=nullptr;
    for (size_t i=0;i<3;i++)
    {
        if (diffusePart!=nullptr)
        {
            b[i]=(double)diffusePart[i];
            b_=b;
        }
        if (specularPart!=nullptr)
        {
            c[i]=(double)specularPart[i];
            c_=c;
        }
    }
    return(simSetLightParameters_internal(objectHandle,state,nullptr,b_,c_));
}
SIM_DLLEXPORT int simGetLightParameters(int objectHandle,floatFloat* setToNULL,floatFloat* diffusePart,floatFloat* specularPart)
{
    double b[3];
    double c[3];
    int retVal=simGetLightParameters_internal(objectHandle,nullptr,b,c);
    if (retVal!=-1)
    {
        for (size_t i=0;i<3;i++)
        {
            if (diffusePart!=nullptr)
                diffusePart[i]=(floatFloat)b[i];
            if (specularPart!=nullptr)
                specularPart[i]=(floatFloat)c[i];
        }
    }
    return(retVal);
}
SIM_DLLEXPORT int simGetVelocity(int shapeHandle,floatFloat* linearVelocity,floatFloat* angularVelocity)
{
    double lv[3];
    double av[3];
    int retVal=simGetVelocity_internal(shapeHandle,lv,av);
    if (retVal!=-1)
    {
        for (size_t i=0;i<3;i++)
        {
            if (linearVelocity!=nullptr)
                linearVelocity[i]=(floatFloat)lv[i];
            if (angularVelocity!=nullptr)
                angularVelocity[i]=(floatFloat)av[i];
        }
    }
    return(retVal);
}
SIM_DLLEXPORT int simGetObjectVelocity(int objectHandle,floatFloat* linearVelocity,floatFloat* angularVelocity)
{
    double lv[3];
    double av[3];
    int retVal=simGetObjectVelocity_internal(objectHandle,lv,av);
    if (retVal!=-1)
    {
        for (size_t i=0;i<3;i++)
        {
            if (linearVelocity!=nullptr)
                linearVelocity[i]=(floatFloat)lv[i];
            if (angularVelocity!=nullptr)
                angularVelocity[i]=(floatFloat)av[i];
        }
    }
    return(retVal);
}
SIM_DLLEXPORT int simGetJointVelocity(int jointHandle,floatFloat* velocity)
{
    double v;
    int retVal=simGetJointVelocity_internal(jointHandle,&v);
    velocity[0]=(floatFloat)v;
    return(retVal);
}
SIM_DLLEXPORT int simAddForceAndTorque(int shapeHandle,const floatFloat* force,const floatFloat* torque)
{
    double f[3];
    double* f_=nullptr;
    double t[3];
    double* t_=nullptr;
    for (size_t i=0;i<3;i++)
    {
        if (force!=nullptr)
        {
            f[i]=(double)force[i];
            f_=f;
        }
        if (torque!=nullptr)
        {
            t[i]=(double)torque[i];
            t_=t;
        }
    }
    return(simAddForceAndTorque_internal(shapeHandle,f_,t_));
}
SIM_DLLEXPORT int simAddForce(int shapeHandle,const floatFloat* position,const floatFloat* force)
{
    double p[3];
    double f[3];
    for (size_t i=0;i<3;i++)
    {
        p[i]=(double)position[i];
        f[i]=(double)force[i];
    }
    return(simAddForce_internal(shapeHandle,p,f));
}
SIM_DLLEXPORT int simSetObjectColor(int objectHandle,int index,int colorComponent,const floatFloat* rgbData)
{
    double c[3];
    for (size_t i=0;i<3;i++)
        c[i]=(double)rgbData[i];
    return(simSetObjectColor_internal(objectHandle,index,colorComponent,c));
}
SIM_DLLEXPORT int simGetObjectColor(int objectHandle,int index,int colorComponent,floatFloat* rgbData)
{
    double c[3];
    int retVal=simGetObjectColor_internal(objectHandle,index,colorComponent,c);
    if (retVal>0)
    {
        for (size_t i=0;i<3;i++)
            rgbData[i]=(floatFloat)c[i];
    }
    return(retVal);
}
SIM_DLLEXPORT int simSetShapeColor(int shapeHandle,const char* colorName,int colorComponent,const floatFloat* rgbData)
{
    double c[3];
    size_t cnt=3;
    if (colorComponent==sim_colorcomponent_transparency)
        cnt=1;
    for (size_t i=0;i<cnt;i++)
        c[i]=(double)rgbData[i];
    return(simSetShapeColor_internal(shapeHandle,colorName,colorComponent,c));
}
SIM_DLLEXPORT int simGetShapeColor(int shapeHandle,const char* colorName,int colorComponent,floatFloat* rgbData)
{
    double c[3];
    int retVal=simGetShapeColor_internal(shapeHandle,colorName,colorComponent,c);
    if (retVal>0)
    {
        size_t cnt=3;
        if (colorComponent==sim_colorcomponent_transparency)
            cnt=1;
        for (size_t i=0;i<cnt;i++)
            rgbData[i]=(floatFloat)c[i];
    }
    return(retVal);
}
SIM_DLLEXPORT int simGetContactInfo(int dynamicPass,int objectHandle,int index,int* objectHandles,floatFloat* contactInfo)
{
    double inf[9];
    int retVal=simGetContactInfo_internal(dynamicPass,objectHandle,index,objectHandles,inf);
    if (retVal>0)
    {
        size_t cnt=6;
        if ((index&sim_handleflag_extended)!=0)
            cnt=9;
        for (size_t i=0;i<cnt;i++)
            contactInfo[i]=(floatFloat)inf[i];
    }
    return(retVal);
}
SIM_DLLEXPORT int simAuxiliaryConsoleOpen(const char* title,int maxLines,int mode,const int* position,const int* size,const floatFloat* textColor,const floatFloat* backgroundColor)
{
    double t[3];
    double t_=nullptr;
    double b[3];
    double v_=nullptr;
    for (size_t i=0;i<3;i++)
    {
        if (textColor!=nullptr)
        {
            t[i]=(double)textColor[i];
            t_=t;
        }
        if (backgroundColor!=nullptr)
        {
            b[i]=(double)backgroundColor[i];
            b_=b;
        }
    }
    return(simAuxiliaryConsoleOpen_internal(title,maxLines,mode,position,size,t_,b_));
}
SIM_DLLEXPORT int simImportShape(int fileformat,const char* pathAndFilename,int options,floatFloat identicalVerticeTolerance,floatFloat scalingFactor)
{
    return(simImportShape_internal(fileformat,pathAndFilename,options,(double)identicalVerticeTolerance,(double)scalingFactor));
}
SIM_DLLEXPORT int simImportMesh(int fileformat,const char* pathAndFilename,int options,floatFloat identicalVerticeTolerance,floatFloat scalingFactor,floatFloat*** vertices,int** verticesSizes,int*** indices,int** indicesSizes,floatFloat*** reserved,char*** names)
{
    double** v;
    int retVal=simImportMesh_internal(fileformat,pathAndFilename,options,(double)identicalVerticeTolerance,(double)scalingFactor,&v,verticesSizes,indices,indicesSizes,nullptr,names);
    if (retVal>0)
    {
        vertices[0]=(floatFloat**)simCreateBuffer_internal(retVal*sizeof(floatFloat*));
        for (int j=0;j<retVal;j++)
        {
            int s=verticesSizes[j]
            vertices[0][j]=(floatFloat*)simCreateBuffer_internal(s*sizeof(floatFloat));
            for (int i=0;i<s;i++)
                vertices[0][j][i]=(floatFloat)v[j][i];
            simReleaseBuffer_internal((char*)v[j]);
        }
        simReleaseBuffer_internal((char*)v);
    }
    return(retVal);
}
SIM_DLLEXPORT int simExportMesh(int fileformat,const char* pathAndFilename,int options,floatFloat scalingFactor,int elementCount,const floatFloat** vertices,const int* verticesSizes,const int** indices,const int* indicesSizes,floatFloat** reserved,const char** names)
{
    double** v=new double*[elementCount];
    for (int j=0;j<elementCount;j++)
    {
        int s=verticesSizes[j];
        v[j]=new double[s];
        for (int i=0;i<s;i++)
            v[j][i]=(double)vertices[j][i];
    }
    int retVal=simExportMesh_internal(fileformat,pathAndFilename,options,(double)scalingFactor,elementCount,v,verticesSizes,indices,indicesSizes,nullptr,names);
    for (int j=0;j<elementCount;j++)
        delete[] v[j];
    delete[] v;
    return(retVal);
}
SIM_DLLEXPORT int simCreateMeshShape(int options,floatFloat shadingAngle,const floatFloat* vertices,int verticesSize,const int* indices,int indicesSize,floatFloat* reserved)
{
    std::vector<double> v;
    for (int i=0;i<verticesSize;i++)
        v.push_back((double)vertices[i]);
    return(simCreateMeshShape_internal(options,(double)shadingAngle,&v[0],verticesSize,indices,indicesSize,nullptr));
}
SIM_DLLEXPORT int simCreatePrimitiveShape(int primitiveType,const floatFloat* sizes,int options)
{
    double s[3];
    for (size_t i=0;i<3;i++)
        s[i]=(double)sizes[i];
    return(simCreatePrimitiveShape_internal(primitiveType,s,options));
}
SIM_DLLEXPORT int simCreateHeightfieldShape(int options,floatFloat shadingAngle,int xPointCount,int yPointCount,floatFloat xSize,const floatFloat* heights)
{
    std::vector<double> h;
    h.resize(xPointCount*yPointCount);
    for (int i=0;i<xPointCount*yPointCount;i++)
        h[i]=(double)heights[i];
    return(simCreateHeightfieldShape_internal(options,(double)shadingAngle,xPointCount,yPointCount,(double)xSize,&h[0]));
}
SIM_DLLEXPORT int simGetShapeMesh(int shapeHandle,floatFloat** vertices,int* verticesSize,int** indices,int* indicesSize,floatFloat** normals)
{
    double* vert;
    double* n;
    int retVal=simGetShapeMesh_internal(shapeHandle,&vert,verticesSize,indices,indicesSize,&n);
    if (retVal!=-1)
    {
        vertices[0]=(floatFloat*)simCreateBuffer_internal(verticesSize[0]*sizeof(floatFloat*));
        normals[0]=(floatFloat*)simCreateBuffer_internal(verticesSize[0]*sizeof(floatFloat*)*3);
        for (int i=0;i<verticesSize[0];i++)
            vertices[0][i]=(floatFloat)vert[i];
        for (int i=0;i<verticesSize[0]*3;i++)
            normals[0][i]=(floatFloat)n[i];
        simReleaseBuffer_internal((char*)vert);
        simReleaseBuffer_internal((char*)n);
    }
    return(retVal);
}
SIM_DLLEXPORT int simCreateJoint(int jointType,int jointMode,int options,const floatFloat* sizes,const floatFloat* reservedA,const floatFloat* reservedB)
{
    double s[2];
    double* s_=nullptr;
    if (sizes!=nullptr)
    {
        s[0]=(double)sizes[0];
        s[1]=(double)sizes[1];
        s_=s;
    }
    return(simCreateJoint_internal(jointType,jointMode,options,s_,nullptr,nullptr));
}
SIM_DLLEXPORT int simGetObjectFloatParam(int objectHandle,int ParamID,floatFloat* Param)
{
    double p;
    int retVal=simGetObjectFloatParam_internal(objectHandle,ParamID,&p);
    if (retVal>0)
        Param[0]=(floatFloat)p;
    return(retVal);
}
SIM_DLLEXPORT int simSetObjectFloatParam(int objectHandle,int ParamID,floatFloat Param)
{
    return(simSetObjectFloatParam_internal(objectHandle,ParamID,(double)Param));
}
SIM_DLLEXPORT floatFloat* simGetObjectFloatArrayParam(int objectHandle,int ParamID,int* size)
{
    floatFloat* retVal=nullptr;
    double* p=simGetObjectFloatArrayParam_internal(objectHandle,ParamID,size);
    if (p!=nullptr)
    {
        retVal=(floatFloat*)simCreateBuffer_internal(size[0]*sizeof(floatFloat));
        for (int i=0;i<size[0];i++)
            retVal[i]=(floatFloat)p[i];
        simReleaseBuffer_internal((char*)p);
    }
    return(retVal);
}
SIM_DLLEXPORT int simSetObjectFloatArrayParam(int objectHandle,int ParamID,const floatFloat* params,int size)
{
    std::vector<double> p;
    p.resize(size);
    for (int i=0;i<size;i++)
        p[i]=(double)params[i];
    return(simSetObjectFloatArrayParam_internal(objectHandle,ParamID,&p[0],size));
}
SIM_DLLEXPORT int simGetRotationAxis(const floatFloat* matrixStart,const floatFloat* matrixGoal,floatFloat* axis,floatFloat* angle)
{
    double m1[12];
    double m2[12];
    double a[3];
    double ang;
    for (size_t i=0;i<12;i++)
    {
        m1[i]=(double)matrixStart[i];
        m2[i]=(double)matrixGoal[i];
    }
    int retVal=simGetRotationAxis_internal(m1,m2,a,&ang);
    if (retVal!=-1)
    {
        for (size_t i=0;i<3;i++)
            axis[i]=(floatFloat)a[i];
        angle[0]=(floatFloat)ang;
    }
    return(retVal);
}
SIM_DLLEXPORT int simRotateAroundAxis(const floatFloat* matrixIn,const floatFloat* axis,const floatFloat* axisPos,floatFloat angle,floatFloat* matrixOut)
{
    double mIn[12];
    for (size_t i=0;i<12;i++)
        mIn[i]=(double)matrixIn[i];
    double a[3];
    double p[3];
    for (size_t i=0;i<3;i++)
    {
        a[i]=(double)axis[i];
        p[i]=(double)axisPos[i];
    }
    double mOut[12];
    int retVal=simRotateAroundAxis_internal(mIn,a,p,(double)angle,mOut);
    if (retVal!=-1)
    {
        for (size_t i=0;i<12;i++)
            matrixOut[i]=(floatFloat)mOut[i];
    }
    return(retVal);
}
SIM_DLLEXPORT int simGetJointForce(int jointHandle,floatFloat* forceOrTorque)
{
    double f;
    int retVal=simGetJointForce_internal(jointHandle,&f);
    if (retVal>0)
        forceOrTorque[0]=(floatFloat)f;
    return(retVal);
}
SIM_DLLEXPORT int simGetJointTargetForce(int jointHandle,floatFloat* forceOrTorque)
{
    double f;
    int retVal=simGetJointTargetForce_internal(jointHandle,&f);
    if (retVal>0)
        forceOrTorque[0]=(floatFloat)f;
    return(retVal);
}
SIM_DLLEXPORT int simSetJointTargetForce(int objectHandle,floatFloat forceOrTorque,bool signedValue)
{
    return(simSetJointTargetForce_internal(objectHandle,(double)forceOrTorque,signedValue));
}
SIM_DLLEXPORT int simCameraFitToView(int viewHandleOrIndex,int objectCount,const int* objectHandles,int options,floatFloat scaling)
{
    return(simCameraFitToView_internal(viewHandleOrIndex,objectCount,objectHandles,options,(double)scaling));
}
SIM_DLLEXPORT int simHandleVisionSensor(int visionSensorHandle,floatFloat** auxValues,int** auxValuesCount)
{
    double* av;
    int retVal=simHandleVisionSensor_internal(visionSensorHandle,&av,auxValuesCount);
    if (retVal!=-1)
    {
        int c=0;
        for (int i=0;i<auxValuesCount[0][0];i++)
            c+=auxValuesCount[0][i+1];
        auxValues[0]=simCreateBuffer_internal(c*sizeof(floatFloat));
        for (int i=0;i<c;i++)
            auxValues[0][i]=(floatFloat)av[i];
        simReleaseBuffer_internal((char*)av);
    }
    return(retVal);
}
SIM_DLLEXPORT int simReadVisionSensor(int visionSensorHandle,floatFloat** auxValues,int** auxValuesCount)
{
    double* av;
    int retVal=simReadVisionSensor_internal(visionSensorHandle,&av,auxValuesCount);
    if (retVal!=-1)
    {
        int c=0;
        for (int i=0;i<auxValuesCount[0][0];i++)
            c+=auxValuesCount[0][i+1];
        auxValues[0]=simCreateBuffer_internal(c*sizeof(floatFloat));
        for (int i=0;i<c;i++)
            auxValues[0][i]=(floatFloat)av[i];
        simReleaseBuffer_internal((char*)av);
    }
    return(retVal);
}
SIM_DLLEXPORT int simCheckVisionSensor(int visionSensorHandle,int entityHandle,floatFloat** auxValues,int** auxValuesCount)
{
    double* av;
    int retVal=simCheckVisionSensor_internal(visionSensorHandle,entityHandle,&av,auxValuesCount);
    if (retVal!=-1)
    {
        int c=0;
        for (int i=0;i<auxValuesCount[0][0];i++)
            c+=auxValuesCount[0][i+1];
        auxValues[0]=simCreateBuffer_internal(c*sizeof(floatFloat));
        for (int i=0;i<c;i++)
            auxValues[0][i]=(floatFloat)av[i];
        simReleaseBuffer_internal((char*)av);
    }
    return(retVal);
}
SIM_DLLEXPORT floatFloat* simCheckVisionSensorEx(int visionSensorHandle,int entityHandle,bool returnImage)
{
    floatFloat retVal=nullptr;
    CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(visionSensorHandle);
    if (it!=nullptr)
    {
        int r[2];
        it->getResolution(r);
        double* dta=simCheckVisionSensorEx_internal(visionSensorHandle,entityHandle,returnImage);
        if (dta!=nullptr)
        {
            int s=1;
            if (returnImage)
                s=3;
            retVal=simCreateBuffer_internal(r[0]*r[1]*s*sizeof(floatFloat));
            for (int i=0;i<r[0]*r[1]*s;i++)
                retVal[i]=(floatFloat)dta[i];
            simReleaseBuffer_internal((char*)dta);
        }
    }
    return(retVal);
}
SIM_DLLEXPORT unsigned char* simGetVisionSensorImg(int sensorHandle,int options,floatFloat rgbaCutOff,const int* pos,const int* size,int* resolution)
{
    return(simGetVisionSensorImg_internal(sensorHandle,options,(double)rgbaCutOff,pos,size,resolution));
}
SIM_DLLEXPORT floatFloat* simGetVisionSensorDepth(int sensorHandle,int options,const int* pos,const int* size,int* resolution)
{
    floatFloat retVal=nullptr;
    double* dta=simGetVisionSensorDepth_internal(sensorHandle,options,pos,size,resolution);
    if (dta!=nullptr)
    {
        retVal=simCreateBuffer_internal(resolution[0]*resolution[1]*sizeof(floatFloat));
        for (int i=0;i<resolution[0]*resolution[1];i++)
            retVal[i]=(floatFloat)dta[i];
        simReleaseBuffer_internal((char*)dta);
    }
    return(retVal);
}
SIM_DLLEXPORT int simCreateDummy(floatFloat size,const floatFloat* reserved)
{
    return(simCreateDummy_internal((double)size,nullptr));
}
SIM_DLLEXPORT int simCreateForceSensor(int options,const int* intParams,const floatFloat* floatParams,const floatFloat* reserved)
{
    double v[5];
    for (size_t i=0;i<5;i++)
        v[i]=(double)floatParams[i];
    return(simCreateForceSensor_internal(options,intParams,v,nullptr));
}
SIM_DLLEXPORT int simCreateProximitySensor(int sensorType,int subType,int options,const int* intParams,const floatFloat* floatParams,const floatFloat* reserved)
{
    double v[15];
    for (size_t i=0;i<15;i++)
        v[i]=(double)floatParams[i];
    return(simCreateProximitySensor_internal(sensorType,subType,options,intParams,v,nullptr));
}
SIM_DLLEXPORT int simCreateVisionSensor(int options,const int* intParams,const floatFloat* floatParams,const floatFloat* reserved)
{
    double v[11];
    for (size_t i=0;i<11;i++)
        v[i]=(double)floatParams[i];
    return(simCreateVisionSensor_internal(options,intParams,v,nullptr));
}
SIM_DLLEXPORT int simConvexDecompose(int shapeHandle,int options,const int* intParams,const floatFloat* floatParams)
{
    double v[10];
    for (size_t i=0;i<10;i++)
        v[i]=(double)floatParams[i];
    return(simConvexDecompose_internal(shapeHandle,options,intParams,v));
}
SIM_DLLEXPORT int simCreateTexture(const char* fileName,int options,const floatFloat* planeSizes,const floatFloat* scalingUV,const floatFloat* xy_g,int fixedResolution,int* textureId,int* resolution,const void* reserved)
{
    double ps[2];
    double* ps_=nullptr;
    if (planeSizes!=nullptr)
    {
        ps[0]=(double)planeSizes[0];
        ps[1]=(double)planeSizes[1];
        ps_=ps;
    }
    double ss[2];
    double* ss_=nullptr;
    if (scalingUV!=nullptr)
    {
        ss[0]=(double)scalingUV[0];
        ss[1]=(double)scalingUV[1];
        ss_=ss;
    }
    double s3[3];
    double* s3_=nullptr;
    if (xy_g!=nullptr)
    {
        s3[0]=(double)xy_g[0];
        s3[1]=(double)xy_g[1];
        s3[2]=(double)xy_g[2];
        s3_=s3;
    }
    return(simCreateTexture_internal(fileName,options,ps_,ss_,s3_,fixedResolution,textureId,resolution,reserved));
}
SIM_DLLEXPORT int simWriteTexture(int textureId,int options,const char* data,int posX,int posY,int sizeX,int sizeY,floatFloat interpol)
{
    return(simWriteTexture_internal(textureId,options,data,posX,posY,sizeX,sizeY,(double)interpol));
}
SIM_DLLEXPORT int simGetShapeGeomInfo(int shapeHandle,int* intData,floatFloat* floatData,void* reserved)
{
    double v[5];
    int retVal=simGetShapeGeomInfo_internal(shapeHandle,intData,v,reserved);
    if (retVal!=nullptr)
    {
        for (size_t i=0;i<5;i++)
            floatData[i]=(floatFloat)v[i];
    }
    return(retVal);
}
SIM_DLLEXPORT int simScaleObject(int objectHandle,floatFloat xScale,floatFloat yScale,floatFloat zScale,int options)
{
    return(simScaleObject_internal(objectHandle,(double)xScale,(double)yScale,(double)zScale,options));
}
SIM_DLLEXPORT int simSetShapeTexture(int shapeHandle,int textureId,int mappingMode,int options,const floatFloat* uvScaling,const floatFloat* position,const floatFloat* orientation)
{
    double uv[2];
    for (size_t i=0;i<2;i++)
        uv[i]=(double)uvScaling[i];
    double p[3];
    double* p=nullptr;
    if (position!=nullptr)
    {
        for (size_t i=0;i<3;i++)
            p[i]=(double)position[i];
        p_=p;
    }
    double o[3];
    double* o=nullptr;
    if (orientation!=nullptr)
    {
        for (size_t i=0;i<3;i++)
            o[i]=(double)orientation[i];
        o_=o;
    }
    return(simSetShapeTexture_internal(shapeHandle,textureId,mappingMode,options,uvScaling,p_,o_));
}
SIM_DLLEXPORT int simTransformImage(unsigned char* image,const int* resolution,int options,const floatFloat* floatParams,const int* intParams,void* reserved)
{
    return(simTransformImage_internal(image,resolution,options,nullptr,nullptr,nullptr));
}
SIM_DLLEXPORT int simGetQHull(const floatFloat* inVertices,int inVerticesL,floatFloat** verticesOut,int* verticesOutL,int** indicesOut,int* indicesOutL,int reserved1,const floatFloat* reserved2)
{
    std::vector<double> v;
    v.resize(inVerticesL);
    for (int i=0;i<inVerticesL;i++)
        v[i]=(double)inVertices[i];
    double* vo;
    int retVal=simGetQHull_internal(&v[0],inVerticesL,&v0,verticesOutL,indicesOut,indicesOutL,reserved1,nullptr);
    if (retVal>0)
    {
        verticesOut[0]=simCreateBuffer_internal(verticesOutL[0]*sizeof(floatFloat));
        for (int i=0;i<verticesOutL[0];i++)
            verticesOut[0][i]=(floatFloat)v0[i];
        simReleaseBuffer_internal((char*)vo);
    }
    return(retVal);
}
SIM_DLLEXPORT int simGetDecimatedMesh(const floatFloat* inVertices,int inVerticesL,const int* inIndices,int inIndicesL,floatFloat** verticesOut,int* verticesOutL,int** indicesOut,int* indicesOutL,floatFloat decimationPercent,int reserved1,const floatFloat* reserved2)
{
    std::vector<double> v;
    v.resize(inVerticesL);
    for (int i=0;i<inVerticesL;i++)
        v[i]=(double)inVertices[i];
    double* vo;
    int retVal=simGetDecimatedMesh_internal(&v[0],inVerticesL,inIndices,inIndicesL,&vo,verticesOutL,indicesOut,indicesOutL,(double)decimationPercent,reserved1,nullptr);
    if (retVal>0)
    {
        verticesOut[0]=simCreateBuffer_internal(verticesOutL[0]*sizeof(floatFloat));
        for (int i=0;i<verticesOutL[0];i++)
            verticesOut[0][i]=(floatFloat)v0[i];
        simReleaseBuffer_internal((char*)vo);
    }
    return(retVal);
}
SIM_DLLEXPORT int simComputeMassAndInertia(int shapeHandle,floatFloat density)
{
    return(simComputeMassAndInertia_internal(shapeHandle,(double)density));
}
SIM_DLLEXPORT floatFloat simGetEngineFloatParam(int paramId,int objectHandle,const void* object,bool* ok)
{
    return((floatFloat)simGetEngineFloatParam_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT int simSetEngineFloatParam(int paramId,int objectHandle,const void* object,floatFloat val)
{
    return(simSetEngineFloatParam_internal(paramId,objectHandle,object,(double)val));
}
SIM_DLLEXPORT int simCreateOctree(floatFloat voxelSize,int options,floatFloat pointSize,void* reserved)
{
    return(simCreateOctree_internal((double)voxelSize,options,(double)pointSize,reserved));
}
SIM_DLLEXPORT int simCreatePointCloud(floatFloat maxVoxelSize,int maxPtCntPerVoxel,int options,floatFloat pointSize,void* reserved)
{
    return(simCreatePointCloud_internal((double)maxVoxelSize,maxPtCntPerVoxel,options,(double)pointSize,reserved));
}
SIM_DLLEXPORT int simSetPointCloudOptions(int pointCloudHandle,floatFloat maxVoxelSize,int maxPtCntPerVoxel,int options,floatFloat pointSize,void* reserved)
{
    return(simSetPointCloudOptions_internal(pointCloudHandle,(double)maxVoxelSize,maxPtCntPerVoxel,options,(double)pointSize,reserved));
}
SIM_DLLEXPORT int simGetPointCloudOptions(int pointCloudHandle,floatFloat* maxVoxelSize,int* maxPtCntPerVoxel,int* options,floatFloat* pointSize,void* reserved)
{
    double mvs;
    double ps;
    int retVal=simGetPointCloudOptions_internal(pointCloudHandle,&mvs,maxPtCntPerVoxel,options,&ps,reserved);
    if (retVal>0)
    {
        maxVoxelSize[0]=(floatFloat)msv;
        pointSize[0]=(floatFloat)ps;
    }
    return(retVal);
}
SIM_DLLEXPORT int simInsertVoxelsIntoOctree(int octreeHandle,int options,const floatFloat* pts,int ptCnt,const unsigned char* color,const unsigned int* tag,void* reserved)
{
    std::vector<double> p;
    p.resize(ptCnt*3);
    for (int i=0;i<ptCnt*3;i++)
        p[i]=(double)pts[i];
    return(simInsertVoxelsIntoOctree_internal(octreeHandle,options,&p[0],ptCnt,color,tag,reserved));
}
SIM_DLLEXPORT int simRemoveVoxelsFromOctree(int octreeHandle,int options,const floatFloat* pts,int ptCnt,void* reserved)
{
    std::vector<double> p;
    p.resize(ptCnt*3);
    for (int i=0;i<ptCnt*3;i++)
        p[i]=(double)pts[i];
    return(simRemoveVoxelsFromOctree_internal(octreeHandle,options,&p[0],ptCnt,reserved));
}
SIM_DLLEXPORT int simInsertPointsIntoPointCloud(int pointCloudHandle,int options,const floatFloat* pts,int ptCnt,const unsigned char* color,void* optionalValues)
{
    std::vector<double> p;
    p.resize(ptCnt*3);
    for (int i=0;i<ptCnt*3;i++)
        p[i]=(double)pts[i];
    return(simInsertPointsIntoPointCloud_internal(pointCloudHandle,options,&p[0],ptCnt,color,optionalValues));
}
SIM_DLLEXPORT int simRemovePointsFromPointCloud(int pointCloudHandle,int options,const floatFloat* pts,int ptCnt,floatFloat tolerance,void* reserved)
{
    std::vector<double> p;
    p.resize(ptCnt*3);
    for (int i=0;i<ptCnt*3;i++)
        p[i]=(double)pts[i];
    return(simRemovePointsFromPointCloud_internal(pointCloudHandle,options,&p[0],ptCnt,(double)tolerance,reserved));
}
SIM_DLLEXPORT int simIntersectPointsWithPointCloud(int pointCloudHandle,int options,const floatFloat* pts,int ptCnt,floatFloat tolerance,void* reserved)
{
    std::vector<double> p;
    p.resize(ptCnt*3);
    for (int i=0;i<ptCnt*3;i++)
        p[i]=(double)pts[i];
    return(simIntersectPointsWithPointCloud_internal(pointCloudHandle,options,&p[0],ptCnt,(double)tolerance,reserved));
}
SIM_DLLEXPORT const floatFloat* simGetOctreeVoxels(int octreeHandle,int* ptCnt,void* reserved)
{
    return(nullptr); // We drop this function in case of single-point precision
}
SIM_DLLEXPORT const floatFloat* simGetPointCloudPoints(int pointCloudHandle,int* ptCnt,void* reserved)
{
    return(nullptr); // We drop this function in case of single-point precision
}
SIM_DLLEXPORT int simInsertObjectIntoPointCloud(int pointCloudHandle,int objectHandle,int options,floatFloat gridSize,const unsigned char* color,void* optionalValues)
{
    return(simInsertObjectIntoPointCloud_internal(pointCloudHandle,objectHandle,options,(double)gridSize,color,optionalValues));
}
SIM_DLLEXPORT int simSubtractObjectFromPointCloud(int pointCloudHandle,int objectHandle,int options,floatFloat tolerance,void* reserved)
{
    return(simSubtractObjectFromPointCloud_internal(pointCloudHandle,objectHandle,options,(double)tolerance,reserved));
}
SIM_DLLEXPORT int simCheckOctreePointOccupancy(int octreeHandle,int options,const floatFloat* points,int ptCnt,unsigned int* tag,unsigned long long int* location,void* reserved)
{
    std::vector<double> p;
    p.resize(ptCnt*3);
    for (int i=0;i<ptCnt*3;i++)
        p[i]=(double)points[i];
    return(simCheckOctreePointOccupancy_internal(octreeHandle,options,&p[0],ptCnt,tag,location,reserved));
}
SIM_DLLEXPORT int simApplyTexture(int shapeHandle,const floatFloat* textureCoordinates,int textCoordSize,const unsigned char* texture,const int* textureResolution,int options)
{
    std::vector<double> tc;
    tc.resize(textCoordSize);
    for (i=0;i<textCoordSize;i++)
        tc[i]=(double)textureCoordinates[i];
    return(simApplyTexture_internal(shapeHandle,&tc[0],textCoordSize,texture,textureResolution,options));
}
SIM_DLLEXPORT int simSetJointDependency(int jointHandle,int masterJointHandle,floatFloat offset,floatFloat multCoeff)
{
    return(simSetJointDependency_internal(jointHandle,masterJointHandle,(double)offset,(double)multCoeff));
}
SIM_DLLEXPORT int simGetJointDependency(int jointHandle,int* masterJointHandle,floatFloat* offset,floatFloat* multCoeff)
{
    double o;
    double m;
    int retVal=simGetJointDependency_internal(jointHandle,masterJointHandle,&o,&m);
    if (retVal!=-1)
    {
        offset[0]=(floatFloat)o;
        multCoeff[0]=(floatFloat)m;
    }
    return(retVal);
}
SIM_DLLEXPORT int simGetShapeMass(int shapeHandle,floatFloat* mass)
{
    double m;
    int retVal=simGetShapeMass_internal(shapeHandle,&m);
    if (retVal!=-1)
        mass[0]=(floatFloat)m;
    return(retVal);
}
SIM_DLLEXPORT int simSetShapeMass(int shapeHandle,floatFloat mass)
{
    return(simSetShapeMass_internal(shapeHandle,(double)mass));
}
SIM_DLLEXPORT int simGetShapeInertia(int shapeHandle,floatFloat* inertiaMatrix,floatFloat* transformationMatrix)
{
    double ine[9];
    double matr[12];
    int retVal=simGetShapeInertia_internal(shapeHandle,ine,matr);
    if (retVal!=-1)
    {
        for (size_t i=0;i<9;i++)
            inertiaMatrix[i]=(floatFloat)ine[i];
        for (size_t i=0;i<12;i++)
            transformationMatrix[i]=(floatFloat)matr[i];
    }
    return(retVal);
}
SIM_DLLEXPORT int simSetShapeInertia(int shapeHandle,const floatFloat* inertiaMatrix,const floatFloat* transformationMatrix)
{
    double ine[9];
    double matr[12];
    for (size_t i=0;i<9;i++)
        ine[i]=(double)inertiaMatrix[i];
    for (size_t i=0;i<12;i++)
        matr[i]=(double)transformationMatrix[i];
    return(simSetShapeInertia_internal(shapeHandle,ine,matr));
}
SIM_DLLEXPORT int simGenerateShapeFromPath(const floatFloat* path,int pathSize,const floatFloat* section,int sectionSize,int options,const floatFloat* upVector,floatFloat reserved)
{
    std::vector<double> p;
    p.resize(pathSize);
    for (int i=0;i<pathSize;i++)
        p[i]=(double)path[i];
    std::vector<double> s;
    s.resize(sectionSize);
    for (int i=0;i<sectionSize;i++)
        s[i]=(double)section[i];
    double upv[3];
    double* upv_=nullptr;
    if (upVector!=nullptr)
    {
        for (size_t i=0;i<3;i++)
            upv[i]=(double)upVector[i];
        upv_=upv;
    }
    return(simGenerateShapeFromPath_internal(&p[0],pathSize,&s[0],sectionSize,options,upv_,reserved));
}
SIM_DLLEXPORT floatFloat simGetClosestPosOnPath(const floatFloat* path,int pathSize,const floatFloat* pathLengths,const floatFloat* absPt)
{
    std::vector<double> p;
    p.resize(pathSize);
    for (int i=0;i<pathSize;i++)
        p[i]=(double)path[i];
    std::vector<double> pl;
    pl.resize(pathSize/3);
    for (int i=0;i<pathSize/3;i++)
        pl[i]=(double)pathLength[i];
    double pt[3];
    for (size_t i=0;i<3;i++)
        pt[i]=(double)absPt[i];
    return((floatFloat)simGetClosestPosOnPath_internal(&p[0],pathSize,&pl[0],&pt[0]));
}
SIM_DLLEXPORT int simExtCallScriptFunction(int scriptHandleOrType, const char* functionNameAtScriptName,
                                               const int* inIntData, int inIntCnt,
                                               const floatFloat* inFloatData, int inFloatCnt,
                                               const char** inStringData, int inStringCnt,
                                               const char* inBufferData, int inBufferCnt,
                                               int** outIntData, int* outIntCnt,
                                               floatFloat** outFloatData, int* outFloatCnt,
                                               char*** outStringData, int* outStringCnt,
                                               char** outBufferData, int* outBufferSize)
{
    std::vector<double> inFloatD;
    double* inFloatD_=nullptr;
    if ( (inFloatData!=nullptr)&&(inFloatCnt>0) )
    {
        for (int i=0;i<inFloatCnt;i++)
            inFloatD[i]=(double)inFloatData[i];
        inFloatD_=&inFloatD[0];
    }
    double* outFloatD;
    int retVal=simExtCallScriptFunction_internal(scriptHandleOrType, functionNameAtScriptName,
                                                 inIntData, inIntCnt, inFloatD_, inFloatCnt,
                                                 inStringData, inStringCnt, inBufferData, inBufferCnt,
                                                 outIntData, outIntCnt, &outFloatD, outFloatCnt,
                                                 outStringData, outStringCnt, outBufferData, outBufferSize);
    if (retVal!=-1)
    {
        outFloatData[0]=simCreateBuffer_internal(outFloatCnt[0]*sizeof(floatFloat));
        for (int i=0;i<outFloatCnt[0];i++)
            outFloatData[0][i]=(floatFloat)outFloatD[i];
        simReleaseBuffer_internal((char*)outFloatD);
    }
    return(retVal);
}

SIM_DLLEXPORT void _simGetObjectLocalTransformation(const void* object,floatFloat* pos,floatFloat* quat,bool excludeFirstJointTransformation)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simSetObjectLocalTransformation(void* object,const floatFloat* pos,const floatFloat* quat,floatFloat simTime)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simDynReportObjectCumulativeTransformation(void* object,const floatFloat* pos,const floatFloat* quat,floatFloat simTime)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simSetObjectCumulativeTransformation(void* object,const floatFloat* pos,const floatFloat* quat,bool keepChildrenInPlace)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simGetObjectCumulativeTransformation(const void* object,floatFloat* pos,floatFloat* quat,bool excludeFirstJointTransformation)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simSetJointVelocity(const void* joint,floatFloat vel)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simSetJointPosition(const void* joint,floatFloat pos)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT floatFloat _simGetJointPosition(const void* joint)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT void _simSetDynamicMotorPositionControlTargetPosition(const void* joint,floatFloat pos)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simGetInitialDynamicVelocity(const void* shape,floatFloat* vel)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simSetInitialDynamicVelocity(void* shape,const floatFloat* vel)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simGetInitialDynamicAngVelocity(const void* shape,floatFloat* angularVel)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simSetInitialDynamicAngVelocity(void* shape,const floatFloat* angularVel)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simSetShapeDynamicVelocity(void* shape,const floatFloat* linear,const floatFloat* angular,floatFloat simTime)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simGetAdditionalForceAndTorque(const void* shape,floatFloat* force,floatFloat* torque)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT bool _simGetJointPositionInterval(const void* joint,floatFloat* minValue,floatFloat* rangeValue)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
    return(0);
}
SIM_DLLEXPORT floatFloat _simGetDynamicMotorTargetPosition(const void* joint)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT floatFloat _simGetDynamicMotorTargetVelocity(const void* joint)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT floatFloat _simGetDynamicMotorMaxForce(const void* joint)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT floatFloat _simGetDynamicMotorUpperLimitVelocity(const void* joint)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT void _simSetDynamicMotorReflectedPositionFromDynamicEngine(void* joint,floatFloat pos,floatFloat simTime)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simSetJointSphericalTransformation(void* joint,const floatFloat* quat,floatFloat simTime)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simAddForceSensorCumulativeForcesAndTorques(void* forceSensor,const floatFloat* force,const floatFloat* torque,int totalPassesCount,floatFloat simTime)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simAddJointCumulativeForcesOrTorques(void* joint,floatFloat forceOrTorque,int totalPassesCount,floatFloat simTime)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT floatFloat _simGetLocalInertiaInfo(const void* object,floatFloat* pos,floatFloat* quat,floatFloat* diagI)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT floatFloat _simGetMass(const void* geomInfo)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT void _simGetPurePrimitiveSizes(const void* geometric,floatFloat* sizes)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simGetVerticesLocalFrame(const void* geometric,floatFloat* pos,floatFloat* quat)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT const floatFloat* _simGetHeightfieldData(const void* geometric,int* xCount,int* yCount,floatFloat* minHeight,floatFloat* maxHeight)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
    return(nullptr);
}
SIM_DLLEXPORT void _simGetCumulativeMeshes(const void* geomInfo,floatFloat** vertices,int* verticesSize,int** indices,int* indicesSize)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simGetGravity(floatFloat* gravity)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT bool _simGetDistanceBetweenEntitiesIfSmaller(int entity1ID,int entity2ID,floatFloat* distance,floatFloat* ray,int* cacheBuffer,bool overrideMeasurableFlagIfNonCollection1,bool overrideMeasurableFlagIfNonCollection2,bool pathPlanningRoutineCalling)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
    return(0);
}
SIM_DLLEXPORT int _simHandleJointControl(const void* joint,int auxV,const int* inputValuesInt,const floatFloat* inputValuesFloat,floatFloat* outputValues)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
    return(0);
}
SIM_DLLEXPORT int _simHandleCustomContact(int objHandle1,int objHandle2,int engine,int* dataInt,floatFloat* dataFloat)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
    return(0);
}
SIM_DLLEXPORT floatFloat _simGetPureHollowScaling(const void* geometric)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
    return(0);
}
SIM_DLLEXPORT void _simDynCallback(const int* intData,const floatFloat* floatData)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
#else
SIM_DLLEXPORT int simSetFloatParam(int parameter,floatFloat floatState)
{
    return(simSetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT int simGetFloatParam(int parameter,floatFloat* floatState)
{
    return(simGetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT int simGetObjectMatrix(int objectHandle,int relativeToObjectHandle,floatFloat* matrix)
{
    return(simGetObjectMatrix_internal(objectHandle,relativeToObjectHandle,matrix));
}
SIM_DLLEXPORT int simSetObjectMatrix(int objectHandle,int relativeToObjectHandle,const floatFloat* matrix)
{
    return(simSetObjectMatrix_internal(objectHandle,relativeToObjectHandle,matrix));
}
SIM_DLLEXPORT int simGetObjectPose(int objectHandle,int relativeToObjectHandle,floatFloat* pose)
{
    return(simGetObjectPose_internal(objectHandle,relativeToObjectHandle,pose));
}
SIM_DLLEXPORT int simSetObjectPose(int objectHandle,int relativeToObjectHandle,const floatFloat* pose)
{
    return(simSetObjectPose_internal(objectHandle,relativeToObjectHandle,pose));
}
SIM_DLLEXPORT int simGetObjectPosition(int objectHandle,int relativeToObjectHandle,floatFloat* position)
{
    return(simGetObjectPosition_internal(objectHandle,relativeToObjectHandle,position));
}
SIM_DLLEXPORT int simSetObjectPosition(int objectHandle,int relativeToObjectHandle,const floatFloat* position)
{
    return(simSetObjectPosition_internal(objectHandle,relativeToObjectHandle,position));
}
SIM_DLLEXPORT int simGetObjectOrientation(int objectHandle,int relativeToObjectHandle,floatFloat* eulerAngles)
{
    return(simGetObjectOrientation_internal(objectHandle,relativeToObjectHandle,eulerAngles));
}
SIM_DLLEXPORT int simGetObjectQuaternion(int objectHandle,int relativeToObjectHandle,floatFloat* quaternion)
{
    return(simGetObjectQuaternion_internal(objectHandle,relativeToObjectHandle,quaternion));
}
SIM_DLLEXPORT int simSetObjectQuaternion(int objectHandle,int relativeToObjectHandle,const floatFloat* quaternion)
{
    return(simSetObjectQuaternion_internal(objectHandle,relativeToObjectHandle,quaternion));
}
SIM_DLLEXPORT int simSetObjectOrientation(int objectHandle,int relativeToObjectHandle,const floatFloat* eulerAngles)
{
    return(simSetObjectOrientation_internal(objectHandle,relativeToObjectHandle,eulerAngles));
}
SIM_DLLEXPORT int simGetJointPosition(int objectHandle,floatFloat* position)
{
    return(simGetJointPosition_internal(objectHandle,position));
}
SIM_DLLEXPORT int simSetJointPosition(int objectHandle,floatFloat position)
{
    return(simSetJointPosition_internal(objectHandle,position));
}
SIM_DLLEXPORT int simSetJointTargetPosition(int objectHandle,floatFloat targetPosition)
{
    return(simSetJointTargetPosition_internal(objectHandle,targetPosition));
}
SIM_DLLEXPORT int simGetJointTargetPosition(int objectHandle,floatFloat* targetPosition)
{
    return(simGetJointTargetPosition_internal(objectHandle,targetPosition));
}
SIM_DLLEXPORT int simGetObjectChildPose(int objectHandle,floatFloat* pose)
{
    return(simGetObjectChildPose_internal(objectHandle,pose));
}
SIM_DLLEXPORT int simSetObjectChildPose(int objectHandle,const floatFloat* pose)
{
    return(simSetObjectChildPose_internal(objectHandle,pose));
}
SIM_DLLEXPORT int simGetJointInterval(int objectHandle,bool* cyclic,floatFloat* interval)
{
    return(simGetJointInterval_internal(objectHandle,cyclic,interval));
}
SIM_DLLEXPORT int simSetJointInterval(int objectHandle,bool cyclic,const floatFloat* interval)
{
    return(simSetJointInterval_internal(objectHandle,cyclic,interval));
}
SIM_DLLEXPORT int simBuildIdentityMatrix(floatFloat* matrix)
{
    return(simBuildIdentityMatrix_internal(matrix));
}
SIM_DLLEXPORT int simBuildMatrix(const floatFloat* position,const floatFloat* eulerAngles,floatFloat* matrix)
{
    return(simBuildMatrix_internal(position,eulerAngles,matrix));
}
SIM_DLLEXPORT int simBuildPose(const floatFloat* position,const floatFloat* eulerAngles,floatFloat* pose)
{
    return(simBuildPose_internal(position,eulerAngles,pose));
}
SIM_DLLEXPORT int simGetEulerAnglesFromMatrix(const floatFloat* matrix,floatFloat* eulerAngles)
{
    return(simGetEulerAnglesFromMatrix_internal(matrix,eulerAngles));
}
SIM_DLLEXPORT int simInvertMatrix(floatFloat* matrix)
{
    return(simInvertMatrix_internal(matrix));
}
SIM_DLLEXPORT int simMultiplyMatrices(const floatFloat* matrixIn1,const floatFloat* matrixIn2,floatFloat* matrixOut)
{
    return(simMultiplyMatrices_internal(matrixIn1,matrixIn2,matrixOut));
}
SIM_DLLEXPORT int simMultiplyPoses(const floatFloat* poseIn1,const floatFloat* poseIn2,floatFloat* poseOut)
{
    return(simMultiplyPoses_internal(poseIn1,poseIn2,poseOut));
}
SIM_DLLEXPORT int simInvertPose(floatFloat* pose)
{
    return(simInvertPose_internal(pose));
}
SIM_DLLEXPORT int simInterpolatePoses(const floatFloat* poseIn1,const floatFloat* poseIn2,floatFloat interpolFactor,floatFloat* poseOut)
{
    return(simInterpolatePoses_internal(poseIn1,poseIn2,interpolFactor,poseOut));
}
SIM_DLLEXPORT int simPoseToMatrix(const floatFloat* poseIn,floatFloat* matrixOut)
{
    return(simPoseToMatrix_internal(poseIn,matrixOut));
}
SIM_DLLEXPORT int simMatrixToPose(const floatFloat* matrixIn,floatFloat* poseOut)
{
    return(simMatrixToPose_internal(matrixIn,poseOut));
}
SIM_DLLEXPORT int simInterpolateMatrices(const floatFloat* matrixIn1,const floatFloat* matrixIn2,floatFloat interpolFactor,floatFloat* matrixOut)
{
    return(simInterpolateMatrices_internal(matrixIn1,matrixIn2,interpolFactor,matrixOut));
}
SIM_DLLEXPORT int simTransformVector(const floatFloat* matrix,floatFloat* vect)
{
    return(simTransformVector_internal(matrix,vect));
}
SIM_DLLEXPORT floatFloat simGetSimulationTime()
{
    return(simGetSimulationTime_internal());
}
SIM_DLLEXPORT floatFloat simGetSystemTime()
{
    return(floatFloat(simGetSystemTime_internal()));
}
SIM_DLLEXPORT int simHandleProximitySensor(int sensorHandle,floatFloat* detectedPoint,int* detectedObjectHandle,floatFloat* normalVector)
{
    return(simHandleProximitySensor_internal(sensorHandle,detectedPoint,detectedObjectHandle,normalVector));
}
SIM_DLLEXPORT int simReadProximitySensor(int sensorHandle,floatFloat* detectedPoint,int* detectedObjectHandle,floatFloat* normalVector)
{
    return(simReadProximitySensor_internal(sensorHandle,detectedPoint,detectedObjectHandle,normalVector));
}
SIM_DLLEXPORT int simHandleDynamics(floatFloat deltaTime)
{
    return(simHandleDynamics_internal(deltaTime));
}
SIM_DLLEXPORT int simCheckProximitySensor(int sensorHandle,int entityHandle,floatFloat* detectedPoint)
{
    return(simCheckProximitySensor_internal(sensorHandle,entityHandle,detectedPoint));
}
SIM_DLLEXPORT int simCheckProximitySensorEx(int sensorHandle,int entityHandle,int detectionMode,floatFloat detectionThreshold,floatFloat maxAngle,floatFloat* detectedPoint,int* detectedObjectHandle,floatFloat* normalVector)
{
    return(simCheckProximitySensorEx_internal(sensorHandle,entityHandle,detectionMode,detectionThreshold,maxAngle,detectedPoint,detectedObjectHandle,normalVector));
}
SIM_DLLEXPORT int simCheckProximitySensorEx2(int sensorHandle,floatFloat* vertexPointer,int itemType,int itemCount,int detectionMode,floatFloat detectionThreshold,floatFloat maxAngle,floatFloat* detectedPoint,floatFloat* normalVector)
{
    return(simCheckProximitySensorEx2_internal(sensorHandle,vertexPointer,itemType,itemCount,detectionMode,detectionThreshold,maxAngle,detectedPoint,normalVector));
}
SIM_DLLEXPORT int simCheckCollisionEx(int entity1Handle,int entity2Handle,floatFloat** intersectionSegments)
{
    return(simCheckCollisionEx_internal(entity1Handle,entity2Handle,intersectionSegments));
}
SIM_DLLEXPORT int simCheckDistance(int entity1Handle,int entity2Handle,floatFloat threshold,floatFloat* distanceData)
{
    return(simCheckDistance_internal(entity1Handle,entity2Handle,threshold,distanceData));
}
SIM_DLLEXPORT int simSetSimulationTimeStep(floatFloat timeStep)
{
    return(simSetSimulationTimeStep_internal(timeStep));
}
SIM_DLLEXPORT floatFloat simGetSimulationTimeStep()
{
    return(simGetSimulationTimeStep_internal());
}
SIM_DLLEXPORT int simAdjustRealTimeTimer(int instanceIndex,floatFloat deltaTime)
{
    return(simAdjustRealTimeTimer_internal(instanceIndex,deltaTime));
}
SIM_DLLEXPORT int simFloatingViewAdd(floatFloat posX,floatFloat posY,floatFloat sizeX,floatFloat sizeY,int options)
{
    return(simFloatingViewAdd_internal(posX,posY,sizeX,sizeY,options));
}
SIM_DLLEXPORT int simHandleGraph(int graphHandle,floatFloat simulationTime)
{
    return(simHandleGraph_internal(graphHandle,simulationTime));
}
SIM_DLLEXPORT int simAddGraphStream(int graphHandle,const char* streamName,const char* unitStr,int options,const floatFloat* color,floatFloat cyclicRange)
{
    return(simAddGraphStream_internal(graphHandle,streamName,unitStr,options,color,cyclicRange));
}
SIM_DLLEXPORT int simSetGraphStreamTransformation(int graphHandle,int streamId,int trType,floatFloat mult,floatFloat off,int movingAvgPeriod)
{
    return(simSetGraphStreamTransformation_internal(graphHandle,streamId,trType,mult,off,movingAvgPeriod));
}
SIM_DLLEXPORT int simAddGraphCurve(int graphHandle,const char* curveName,int dim,const int* streamIds,const floatFloat* defaultValues,const char* unitStr,int options,const floatFloat* color,int curveWidth)
{
    return(simAddGraphCurve_internal(graphHandle,curveName,dim,streamIds,defaultValues,unitStr,options,color,curveWidth));
}
SIM_DLLEXPORT int simSetGraphStreamValue(int graphHandle,int streamId,floatFloat value)
{
    return(simSetGraphStreamValue_internal(graphHandle,streamId,value));
}
SIM_DLLEXPORT int simSetJointTargetVelocity(int objectHandle,floatFloat targetVelocity)
{
    return(simSetJointTargetVelocity_internal(objectHandle,targetVelocity));
}
SIM_DLLEXPORT int simGetJointTargetVelocity(int objectHandle,floatFloat* targetVelocity)
{
    return(simGetJointTargetVelocity_internal(objectHandle,targetVelocity));
}
SIM_DLLEXPORT int simScaleObjects(const int* objectHandles,int objectCount,floatFloat scalingFactor,bool scalePositionsToo)
{
    return(simScaleObjects_internal(objectHandles,objectCount,scalingFactor,scalePositionsToo));
}
SIM_DLLEXPORT int simAddDrawingObject(int objectType,floatFloat size,floatFloat duplicateTolerance,int parentObjectHandle,int maxItemCount,const floatFloat* color,const floatFloat* setToNULL,const floatFloat* setToNULL2,const floatFloat* setToNULL3)
{
    return(simAddDrawingObject_internal(objectType,size,duplicateTolerance,parentObjectHandle,maxItemCount,color,setToNULL,setToNULL2,setToNULL3));
}
SIM_DLLEXPORT int simAddDrawingObjectItem(int objectHandle,const floatFloat* itemData)
{
    return(simAddDrawingObjectItem_internal(objectHandle,itemData));
}
SIM_DLLEXPORT floatFloat simGetObjectSizeFactor(int objectHandle)
{
    return(simGetObjectSizeFactor_internal(objectHandle));
}
SIM_DLLEXPORT int simSetFloatSignal(const char* signalName,floatFloat signalValue)
{
    return(simSetFloatSignal_internal(signalName,signalValue));
}
SIM_DLLEXPORT int simGetFloatSignal(const char* signalName,floatFloat* signalValue)
{
    return(simGetFloatSignal_internal(signalName,signalValue));
}
SIM_DLLEXPORT int simReadForceSensor(int objectHandle,floatFloat* forceVector,floatFloat* torqueVector)
{
    return(simReadForceSensor_internal(objectHandle,forceVector,torqueVector));
}
SIM_DLLEXPORT int simSetLightParameters(int objectHandle,int state,const floatFloat* setToNULL,const floatFloat* diffusePart,const floatFloat* specularPart)
{
    return(simSetLightParameters_internal(objectHandle,state,setToNULL,diffusePart,specularPart));
}
SIM_DLLEXPORT int simGetLightParameters(int objectHandle,floatFloat* setToNULL,floatFloat* diffusePart,floatFloat* specularPart)
{
    return(simGetLightParameters_internal(objectHandle,setToNULL,diffusePart,specularPart));
}
SIM_DLLEXPORT int simGetVelocity(int shapeHandle,floatFloat* linearVelocity,floatFloat* angularVelocity)
{
    return(simGetVelocity_internal(shapeHandle,linearVelocity,angularVelocity));
}
SIM_DLLEXPORT int simGetObjectVelocity(int objectHandle,floatFloat* linearVelocity,floatFloat* angularVelocity)
{
    return(simGetObjectVelocity_internal(objectHandle,linearVelocity,angularVelocity));
}
SIM_DLLEXPORT int simGetJointVelocity(int jointHandle,floatFloat* velocity)
{
    return(simGetJointVelocity_internal(jointHandle,velocity));
}
SIM_DLLEXPORT int simAddForceAndTorque(int shapeHandle,const floatFloat* force,const floatFloat* torque)
{
    return(simAddForceAndTorque_internal(shapeHandle,force,torque));
}
SIM_DLLEXPORT int simAddForce(int shapeHandle,const floatFloat* position,const floatFloat* force)
{
    return(simAddForce_internal(shapeHandle,position,force));
}
SIM_DLLEXPORT int simSetObjectColor(int objectHandle,int index,int colorComponent,const floatFloat* rgbData)
{
    return(simSetObjectColor_internal(objectHandle,index,colorComponent,rgbData));
}
SIM_DLLEXPORT int simGetObjectColor(int objectHandle,int index,int colorComponent,floatFloat* rgbData)
{
    return(simGetObjectColor_internal(objectHandle,index,colorComponent,rgbData));
}
SIM_DLLEXPORT int simSetShapeColor(int shapeHandle,const char* colorName,int colorComponent,const floatFloat* rgbData)
{
    return(simSetShapeColor_internal(shapeHandle,colorName,colorComponent,rgbData));
}
SIM_DLLEXPORT int simGetShapeColor(int shapeHandle,const char* colorName,int colorComponent,floatFloat* rgbData)
{
    return(simGetShapeColor_internal(shapeHandle,colorName,colorComponent,rgbData));
}
SIM_DLLEXPORT int simGetContactInfo(int dynamicPass,int objectHandle,int index,int* objectHandles,floatFloat* contactInfo)
{
    return(simGetContactInfo_internal(dynamicPass,objectHandle,index,objectHandles,contactInfo));
}
SIM_DLLEXPORT int simAuxiliaryConsoleOpen(const char* title,int maxLines,int mode,const int* position,const int* size,const floatFloat* textColor,const floatFloat* backgroundColor)
{
    return(simAuxiliaryConsoleOpen_internal(title,maxLines,mode,position,size,textColor,backgroundColor));
}
SIM_DLLEXPORT int simImportShape(int fileformat,const char* pathAndFilename,int options,floatFloat identicalVerticeTolerance,floatFloat scalingFactor)
{
    return(simImportShape_internal(fileformat,pathAndFilename,options,identicalVerticeTolerance,scalingFactor));
}
SIM_DLLEXPORT int simImportMesh(int fileformat,const char* pathAndFilename,int options,floatFloat identicalVerticeTolerance,floatFloat scalingFactor,floatFloat*** vertices,int** verticesSizes,int*** indices,int** indicesSizes,floatFloat*** reserved,char*** names)
{
    return(simImportMesh_internal(fileformat,pathAndFilename,options,identicalVerticeTolerance,scalingFactor,vertices,verticesSizes,indices,indicesSizes,reserved,names));
}
SIM_DLLEXPORT int simExportMesh(int fileformat,const char* pathAndFilename,int options,floatFloat scalingFactor,int elementCount,const floatFloat** vertices,const int* verticesSizes,const int** indices,const int* indicesSizes,floatFloat** reserved,const char** names)
{
    return(simExportMesh_internal(fileformat,pathAndFilename,options,scalingFactor,elementCount,vertices,verticesSizes,indices,indicesSizes,reserved,names));
}
SIM_DLLEXPORT int simCreateMeshShape(int options,floatFloat shadingAngle,const floatFloat* vertices,int verticesSize,const int* indices,int indicesSize,floatFloat* reserved)
{
    return(simCreateMeshShape_internal(options,shadingAngle,vertices,verticesSize,indices,indicesSize,reserved));
}
SIM_DLLEXPORT int simCreatePrimitiveShape(int primitiveType,const floatFloat* sizes,int options)
{
    return(simCreatePrimitiveShape_internal(primitiveType,sizes,options));
}
SIM_DLLEXPORT int simCreateHeightfieldShape(int options,floatFloat shadingAngle,int xPointCount,int yPointCount,floatFloat xSize,const floatFloat* heights)
{
    return(simCreateHeightfieldShape_internal(options,shadingAngle,xPointCount,yPointCount,xSize,heights));
}
SIM_DLLEXPORT int simGetShapeMesh(int shapeHandle,floatFloat** vertices,int* verticesSize,int** indices,int* indicesSize,floatFloat** normals)
{
    return(simGetShapeMesh_internal(shapeHandle,vertices,verticesSize,indices,indicesSize,normals));
}
SIM_DLLEXPORT int simCreateJoint(int jointType,int jointMode,int options,const floatFloat* sizes,const floatFloat* reservedA,const floatFloat* reservedB)
{
    return(simCreateJoint_internal(jointType,jointMode,options,sizes,reservedA,reservedB));
}
SIM_DLLEXPORT int simGetObjectFloatParam(int objectHandle,int ParamID,floatFloat* Param)
{
    return(simGetObjectFloatParam_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT int simSetObjectFloatParam(int objectHandle,int ParamID,floatFloat Param)
{
    return(simSetObjectFloatParam_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT floatFloat* simGetObjectFloatArrayParam(int objectHandle,int ParamID,int* size)
{
    return(simGetObjectFloatArrayParam_internal(objectHandle,ParamID,size));
}
SIM_DLLEXPORT int simSetObjectFloatArrayParam(int objectHandle,int ParamID,const floatFloat* params,int size)
{
    return(simSetObjectFloatArrayParam_internal(objectHandle,ParamID,params,size));
}
SIM_DLLEXPORT int simGetRotationAxis(const floatFloat* matrixStart,const floatFloat* matrixGoal,floatFloat* axis,floatFloat* angle)
{
    return(simGetRotationAxis_internal(matrixStart,matrixGoal,axis,angle));
}
SIM_DLLEXPORT int simRotateAroundAxis(const floatFloat* matrixIn,const floatFloat* axis,const floatFloat* axisPos,floatFloat angle,floatFloat* matrixOut)
{
    return(simRotateAroundAxis_internal(matrixIn,axis,axisPos,angle,matrixOut));
}
SIM_DLLEXPORT int simGetJointForce(int jointHandle,floatFloat* forceOrTorque)
{
    return(simGetJointForce_internal(jointHandle,forceOrTorque));
}
SIM_DLLEXPORT int simGetJointTargetForce(int jointHandle,floatFloat* forceOrTorque)
{
    return(simGetJointTargetForce_internal(jointHandle,forceOrTorque));
}
SIM_DLLEXPORT int simSetJointTargetForce(int objectHandle,floatFloat forceOrTorque,bool signedValue)
{
    return(simSetJointTargetForce_internal(objectHandle,forceOrTorque,signedValue));
}
SIM_DLLEXPORT int simCameraFitToView(int viewHandleOrIndex,int objectCount,const int* objectHandles,int options,floatFloat scaling)
{
    return(simCameraFitToView_internal(viewHandleOrIndex,objectCount,objectHandles,options,scaling));
}
SIM_DLLEXPORT int simHandleVisionSensor(int visionSensorHandle,floatFloat** auxValues,int** auxValuesCount)
{
    return(simHandleVisionSensor_internal(visionSensorHandle,auxValues,auxValuesCount));
}
SIM_DLLEXPORT int simReadVisionSensor(int visionSensorHandle,floatFloat** auxValues,int** auxValuesCount)
{
    return(simReadVisionSensor_internal(visionSensorHandle,auxValues,auxValuesCount));
}
SIM_DLLEXPORT int simCheckVisionSensor(int visionSensorHandle,int entityHandle,floatFloat** auxValues,int** auxValuesCount)
{
    return(simCheckVisionSensor_internal(visionSensorHandle,entityHandle,auxValues,auxValuesCount));
}
SIM_DLLEXPORT floatFloat* simCheckVisionSensorEx(int visionSensorHandle,int entityHandle,bool returnImage)
{
    return(simCheckVisionSensorEx_internal(visionSensorHandle,entityHandle,returnImage));
}
SIM_DLLEXPORT unsigned char* simGetVisionSensorImg(int sensorHandle,int options,floatFloat rgbaCutOff,const int* pos,const int* size,int* resolution)
{
    return(simGetVisionSensorImg_internal(sensorHandle,options,rgbaCutOff,pos,size,resolution));
}
SIM_DLLEXPORT floatFloat* simGetVisionSensorDepth(int sensorHandle,int options,const int* pos,const int* size,int* resolution)
{
    return(simGetVisionSensorDepth_internal(sensorHandle,options,pos,size,resolution));
}
SIM_DLLEXPORT int simCreateDummy(floatFloat size,const floatFloat* reserved)
{
    return(simCreateDummy_internal(size,reserved));
}
SIM_DLLEXPORT int simCreateForceSensor(int options,const int* intParams,const floatFloat* floatParams,const floatFloat* reserved)
{
    return(simCreateForceSensor_internal(options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT int simCreateProximitySensor(int sensorType,int subType,int options,const int* intParams,const floatFloat* floatParams,const floatFloat* reserved)
{
    return(simCreateProximitySensor_internal(sensorType,subType,options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT int simCreateVisionSensor(int options,const int* intParams,const floatFloat* floatParams,const floatFloat* reserved)
{
    return(simCreateVisionSensor_internal(options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT int simConvexDecompose(int shapeHandle,int options,const int* intParams,const floatFloat* floatParams)
{
    return(simConvexDecompose_internal(shapeHandle,options,intParams,floatParams));
}
SIM_DLLEXPORT int simCreateTexture(const char* fileName,int options,const floatFloat* planeSizes,const floatFloat* scalingUV,const floatFloat* xy_g,int fixedResolution,int* textureId,int* resolution,const void* reserved)
{
    return(simCreateTexture_internal(fileName,options,planeSizes,scalingUV,xy_g,fixedResolution,textureId,resolution,reserved));
}
SIM_DLLEXPORT int simWriteTexture(int textureId,int options,const char* data,int posX,int posY,int sizeX,int sizeY,floatFloat interpol)
{
    return(simWriteTexture_internal(textureId,options,data,posX,posY,sizeX,sizeY,interpol));
}
SIM_DLLEXPORT int simGetShapeGeomInfo(int shapeHandle,int* intData,floatFloat* floatData,void* reserved)
{
    return(simGetShapeGeomInfo_internal(shapeHandle,intData,floatData,reserved));
}
SIM_DLLEXPORT int simScaleObject(int objectHandle,floatFloat xScale,floatFloat yScale,floatFloat zScale,int options)
{
    return(simScaleObject_internal(objectHandle,xScale,yScale,zScale,options));
}
SIM_DLLEXPORT int simSetShapeTexture(int shapeHandle,int textureId,int mappingMode,int options,const floatFloat* uvScaling,const floatFloat* position,const floatFloat* orientation)
{
    return(simSetShapeTexture_internal(shapeHandle,textureId,mappingMode,options,uvScaling,position,orientation));
}
SIM_DLLEXPORT int simTransformImage(unsigned char* image,const int* resolution,int options,const floatFloat* floatParams,const int* intParams,void* reserved)
{
    return(simTransformImage_internal(image,resolution,options,floatParams,intParams,reserved));
}
SIM_DLLEXPORT int simGetQHull(const floatFloat* inVertices,int inVerticesL,floatFloat** verticesOut,int* verticesOutL,int** indicesOut,int* indicesOutL,int reserved1,const floatFloat* reserved2)
{
    return(simGetQHull_internal(inVertices,inVerticesL,verticesOut,verticesOutL,indicesOut,indicesOutL,reserved1,reserved2));
}
SIM_DLLEXPORT int simGetDecimatedMesh(const floatFloat* inVertices,int inVerticesL,const int* inIndices,int inIndicesL,floatFloat** verticesOut,int* verticesOutL,int** indicesOut,int* indicesOutL,floatFloat decimationPercent,int reserved1,const floatFloat* reserved2)
{
    return(simGetDecimatedMesh_internal(inVertices,inVerticesL,inIndices,inIndicesL,verticesOut,verticesOutL,indicesOut,indicesOutL,decimationPercent,reserved1,reserved2));
}
SIM_DLLEXPORT int simComputeMassAndInertia(int shapeHandle,floatFloat density)
{
    return(simComputeMassAndInertia_internal(shapeHandle,density));
}
SIM_DLLEXPORT floatFloat simGetEngineFloatParam(int paramId,int objectHandle,const void* object,bool* ok)
{
    return(simGetEngineFloatParam_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT int simSetEngineFloatParam(int paramId,int objectHandle,const void* object,floatFloat val)
{
    return(simSetEngineFloatParam_internal(paramId,objectHandle,object,val));
}
SIM_DLLEXPORT int simCreateOctree(floatFloat voxelSize,int options,floatFloat pointSize,void* reserved)
{
    return(simCreateOctree_internal(voxelSize,options,pointSize,reserved));
}
SIM_DLLEXPORT int simCreatePointCloud(floatFloat maxVoxelSize,int maxPtCntPerVoxel,int options,floatFloat pointSize,void* reserved)
{
    return(simCreatePointCloud_internal(maxVoxelSize,maxPtCntPerVoxel,options,pointSize,reserved));
}
SIM_DLLEXPORT int simSetPointCloudOptions(int pointCloudHandle,floatFloat maxVoxelSize,int maxPtCntPerVoxel,int options,floatFloat pointSize,void* reserved)
{
    return(simSetPointCloudOptions_internal(pointCloudHandle,maxVoxelSize,maxPtCntPerVoxel,options,pointSize,reserved));
}
SIM_DLLEXPORT int simGetPointCloudOptions(int pointCloudHandle,floatFloat* maxVoxelSize,int* maxPtCntPerVoxel,int* options,floatFloat* pointSize,void* reserved)
{
    return(simGetPointCloudOptions_internal(pointCloudHandle,maxVoxelSize,maxPtCntPerVoxel,options,pointSize,reserved));
}
SIM_DLLEXPORT int simInsertVoxelsIntoOctree(int octreeHandle,int options,const floatFloat* pts,int ptCnt,const unsigned char* color,const unsigned int* tag,void* reserved)
{
    return(simInsertVoxelsIntoOctree_internal(octreeHandle,options,pts,ptCnt,color,tag,reserved));
}
SIM_DLLEXPORT int simRemoveVoxelsFromOctree(int octreeHandle,int options,const floatFloat* pts,int ptCnt,void* reserved)
{
    return(simRemoveVoxelsFromOctree_internal(octreeHandle,options,pts,ptCnt,reserved));
}
SIM_DLLEXPORT int simInsertPointsIntoPointCloud(int pointCloudHandle,int options,const floatFloat* pts,int ptCnt,const unsigned char* color,void* optionalValues)
{
    return(simInsertPointsIntoPointCloud_internal(pointCloudHandle,options,pts,ptCnt,color,optionalValues));
}
SIM_DLLEXPORT int simRemovePointsFromPointCloud(int pointCloudHandle,int options,const floatFloat* pts,int ptCnt,floatFloat tolerance,void* reserved)
{
    return(simRemovePointsFromPointCloud_internal(pointCloudHandle,options,pts,ptCnt,tolerance,reserved));
}
SIM_DLLEXPORT int simIntersectPointsWithPointCloud(int pointCloudHandle,int options,const floatFloat* pts,int ptCnt,floatFloat tolerance,void* reserved)
{
    return(simIntersectPointsWithPointCloud_internal(pointCloudHandle,options,pts,ptCnt,tolerance,reserved));
}
SIM_DLLEXPORT const floatFloat* simGetOctreeVoxels(int octreeHandle,int* ptCnt,void* reserved)
{
    return(simGetOctreeVoxels_internal(octreeHandle,ptCnt,reserved));
}
SIM_DLLEXPORT const floatFloat* simGetPointCloudPoints(int pointCloudHandle,int* ptCnt,void* reserved)
{
    return(simGetPointCloudPoints_internal(pointCloudHandle,ptCnt,reserved));
}
SIM_DLLEXPORT int simInsertObjectIntoPointCloud(int pointCloudHandle,int objectHandle,int options,floatFloat gridSize,const unsigned char* color,void* optionalValues)
{
    return(simInsertObjectIntoPointCloud_internal(pointCloudHandle,objectHandle,options,gridSize,color,optionalValues));
}
SIM_DLLEXPORT int simSubtractObjectFromPointCloud(int pointCloudHandle,int objectHandle,int options,floatFloat tolerance,void* reserved)
{
    return(simSubtractObjectFromPointCloud_internal(pointCloudHandle,objectHandle,options,tolerance,reserved));
}
SIM_DLLEXPORT int simCheckOctreePointOccupancy(int octreeHandle,int options,const floatFloat* points,int ptCnt,unsigned int* tag,unsigned long long int* location,void* reserved)
{
    return(simCheckOctreePointOccupancy_internal(octreeHandle,options,points,ptCnt,tag,location,reserved));
}
SIM_DLLEXPORT int simApplyTexture(int shapeHandle,const floatFloat* textureCoordinates,int textCoordSize,const unsigned char* texture,const int* textureResolution,int options)
{
    return(simApplyTexture_internal(shapeHandle,textureCoordinates,textCoordSize,texture,textureResolution,options));
}
SIM_DLLEXPORT int simSetJointDependency(int jointHandle,int masterJointHandle,floatFloat offset,floatFloat multCoeff)
{
    return(simSetJointDependency_internal(jointHandle,masterJointHandle,offset,multCoeff));
}
SIM_DLLEXPORT int simGetJointDependency(int jointHandle,int* masterJointHandle,floatFloat* offset,floatFloat* multCoeff)
{
    return(simGetJointDependency_internal(jointHandle,masterJointHandle,offset,multCoeff));
}
SIM_DLLEXPORT int simGetShapeMass(int shapeHandle,floatFloat* mass)
{
    return(simGetShapeMass_internal(shapeHandle,mass));
}
SIM_DLLEXPORT int simSetShapeMass(int shapeHandle,floatFloat mass)
{
    return(simSetShapeMass_internal(shapeHandle,mass));
}
SIM_DLLEXPORT int simGetShapeInertia(int shapeHandle,floatFloat* inertiaMatrix,floatFloat* transformationMatrix)
{
    return(simGetShapeInertia_internal(shapeHandle,inertiaMatrix,transformationMatrix));
}
SIM_DLLEXPORT int simSetShapeInertia(int shapeHandle,const floatFloat* inertiaMatrix,const floatFloat* transformationMatrix)
{
    return(simSetShapeInertia_internal(shapeHandle,inertiaMatrix,transformationMatrix));
}
SIM_DLLEXPORT int simGenerateShapeFromPath(const floatFloat* path,int pathSize,const floatFloat* section,int sectionSize,int options,const floatFloat* upVector,floatFloat reserved)
{
    return(simGenerateShapeFromPath_internal(path,pathSize,section,sectionSize,options,upVector,reserved));
}
SIM_DLLEXPORT floatFloat simGetClosestPosOnPath(const floatFloat* path,int pathSize,const floatFloat* pathLengths,const floatFloat* absPt)
{
    return(simGetClosestPosOnPath_internal(path,pathSize,pathLengths,absPt));
}
SIM_DLLEXPORT void _simGetObjectLocalTransformation(const void* object,floatFloat* pos,floatFloat* quat,bool excludeFirstJointTransformation)
{
    return(_simGetObjectLocalTransformation_internal(object,pos,quat,excludeFirstJointTransformation));
}
SIM_DLLEXPORT void _simSetObjectLocalTransformation(void* object,const floatFloat* pos,const floatFloat* quat,floatFloat simTime)
{
    return(_simSetObjectLocalTransformation_internal(object,pos,quat,simTime));
}
SIM_DLLEXPORT void _simDynReportObjectCumulativeTransformation(void* object,const floatFloat* pos,const floatFloat* quat,floatFloat simTime)
{
    return(_simDynReportObjectCumulativeTransformation_internal(object,pos,quat,simTime));
}
SIM_DLLEXPORT void _simSetObjectCumulativeTransformation(void* object,const floatFloat* pos,const floatFloat* quat,bool keepChildrenInPlace)
{
    return(_simSetObjectCumulativeTransformation_internal(object,pos,quat,keepChildrenInPlace));
}
SIM_DLLEXPORT void _simGetObjectCumulativeTransformation(const void* object,floatFloat* pos,floatFloat* quat,bool excludeFirstJointTransformation)
{
    return(_simGetObjectCumulativeTransformation_internal(object,pos,quat,excludeFirstJointTransformation));
}
SIM_DLLEXPORT void _simSetJointVelocity(const void* joint,floatFloat vel)
{
    return(_simSetJointVelocity_internal(joint,vel));
}
SIM_DLLEXPORT void _simSetJointPosition(const void* joint,floatFloat pos)
{
    return(_simSetJointPosition_internal(joint,pos));
}
SIM_DLLEXPORT floatFloat _simGetJointPosition(const void* joint)
{
    return(_simGetJointPosition_internal(joint));
}
SIM_DLLEXPORT void _simSetDynamicMotorPositionControlTargetPosition(const void* joint,floatFloat pos)
{
    return(_simSetDynamicMotorPositionControlTargetPosition_internal(joint,pos));
}
SIM_DLLEXPORT void _simGetInitialDynamicVelocity(const void* shape,floatFloat* vel)
{
    return(_simGetInitialDynamicVelocity_internal(shape,vel));
}
SIM_DLLEXPORT void _simSetInitialDynamicVelocity(void* shape,const floatFloat* vel)
{
    return(_simSetInitialDynamicVelocity_internal(shape,vel));
}
SIM_DLLEXPORT void _simGetInitialDynamicAngVelocity(const void* shape,floatFloat* angularVel)
{
    return(_simGetInitialDynamicAngVelocity_internal(shape,angularVel));
}
SIM_DLLEXPORT void _simSetInitialDynamicAngVelocity(void* shape,const floatFloat* angularVel)
{
    return(_simSetInitialDynamicAngVelocity_internal(shape,angularVel));
}
SIM_DLLEXPORT void _simSetShapeDynamicVelocity(void* shape,const floatFloat* linear,const floatFloat* angular,floatFloat simTime)
{
    return(_simSetShapeDynamicVelocity_internal(shape,linear,angular,simTime));
}
SIM_DLLEXPORT void _simGetAdditionalForceAndTorque(const void* shape,floatFloat* force,floatFloat* torque)
{
    return(_simGetAdditionalForceAndTorque_internal(shape,force,torque));
}
SIM_DLLEXPORT bool _simGetJointPositionInterval(const void* joint,floatFloat* minValue,floatFloat* rangeValue)
{
    return(_simGetJointPositionInterval_internal(joint,minValue,rangeValue));
}
SIM_DLLEXPORT floatFloat _simGetDynamicMotorTargetPosition(const void* joint)
{
    return(_simGetDynamicMotorTargetPosition_internal(joint));
}
SIM_DLLEXPORT floatFloat _simGetDynamicMotorTargetVelocity(const void* joint)
{
    return(_simGetDynamicMotorTargetVelocity_internal(joint));
}
SIM_DLLEXPORT floatFloat _simGetDynamicMotorMaxForce(const void* joint)
{
    return(_simGetDynamicMotorMaxForce_internal(joint));
}
SIM_DLLEXPORT floatFloat _simGetDynamicMotorUpperLimitVelocity(const void* joint)
{
    return(_simGetDynamicMotorUpperLimitVelocity_internal(joint));
}
SIM_DLLEXPORT void _simSetDynamicMotorReflectedPositionFromDynamicEngine(void* joint,floatFloat pos,floatFloat simTime)
{
    return(_simSetDynamicMotorReflectedPositionFromDynamicEngine_internal(joint,pos,simTime));
}
SIM_DLLEXPORT void _simSetJointSphericalTransformation(void* joint,const floatFloat* quat,floatFloat simTime)
{
    return(_simSetJointSphericalTransformation_internal(joint,quat,simTime));
}
SIM_DLLEXPORT void _simAddForceSensorCumulativeForcesAndTorques(void* forceSensor,const floatFloat* force,const floatFloat* torque,int totalPassesCount,floatFloat simTime)
{
    return(_simAddForceSensorCumulativeForcesAndTorques_internal(forceSensor,force,torque,totalPassesCount,simTime));
}
SIM_DLLEXPORT void _simAddJointCumulativeForcesOrTorques(void* joint,floatFloat forceOrTorque,int totalPassesCount,floatFloat simTime)
{
    return(_simAddJointCumulativeForcesOrTorques_internal(joint,forceOrTorque,totalPassesCount,simTime));
}
SIM_DLLEXPORT floatFloat _simGetLocalInertiaInfo(const void* object,floatFloat* pos,floatFloat* quat,floatFloat* diagI)
{
    return(_simGetLocalInertiaInfo_internal(object,pos,quat,diagI));
}
SIM_DLLEXPORT floatFloat _simGetMass(const void* geomInfo)
{
    return(_simGetMass_internal(geomInfo));
}
SIM_DLLEXPORT void _simGetPurePrimitiveSizes(const void* geometric,floatFloat* sizes)
{
    return(_simGetPurePrimitiveSizes_internal(geometric,sizes));
}
SIM_DLLEXPORT void _simGetVerticesLocalFrame(const void* geometric,floatFloat* pos,floatFloat* quat)
{
    return(_simGetVerticesLocalFrame_internal(geometric,pos,quat));
}
SIM_DLLEXPORT const floatFloat* _simGetHeightfieldData(const void* geometric,int* xCount,int* yCount,floatFloat* minHeight,floatFloat* maxHeight)
{
    return(_simGetHeightfieldData_internal(geometric,xCount,yCount,minHeight,maxHeight));
}
SIM_DLLEXPORT void _simGetCumulativeMeshes(const void* geomInfo,floatFloat** vertices,int* verticesSize,int** indices,int* indicesSize)
{
    return(_simGetCumulativeMeshes_internal(geomInfo,vertices,verticesSize,indices,indicesSize));
}
SIM_DLLEXPORT void _simGetGravity(floatFloat* gravity)
{
    return(_simGetGravity_internal(gravity));
}
SIM_DLLEXPORT bool _simGetDistanceBetweenEntitiesIfSmaller(int entity1ID,int entity2ID,floatFloat* distance,floatFloat* ray,int* cacheBuffer,bool overrideMeasurableFlagIfNonCollection1,bool overrideMeasurableFlagIfNonCollection2,bool pathPlanningRoutineCalling)
{
    return(_simGetDistanceBetweenEntitiesIfSmaller_internal(entity1ID,entity2ID,distance,ray,cacheBuffer,overrideMeasurableFlagIfNonCollection1,overrideMeasurableFlagIfNonCollection2,pathPlanningRoutineCalling));
}
SIM_DLLEXPORT int _simHandleJointControl(const void* joint,int auxV,const int* inputValuesInt,const floatFloat* inputValuesFloat,floatFloat* outputValues)
{
    return(_simHandleJointControl_internal(joint,auxV,inputValuesInt,inputValuesFloat,outputValues));
}
SIM_DLLEXPORT int _simHandleCustomContact(int objHandle1,int objHandle2,int engine,int* dataInt,floatFloat* dataFloat)
{
    return(_simHandleCustomContact_internal(objHandle1,objHandle2,engine,dataInt,dataFloat));
}
SIM_DLLEXPORT floatFloat _simGetPureHollowScaling(const void* geometric)
{
    return(_simGetPureHollowScaling_internal(geometric));
}
SIM_DLLEXPORT void _simDynCallback(const int* intData,const floatFloat* floatData)
{
    _simDynCallback_internal(intData,floatData);
}
SIM_DLLEXPORT int simExtCallScriptFunction(int scriptHandleOrType, const char* functionNameAtScriptName,
                                               const int* inIntData, int inIntCnt,
                                               const floatFloat* inFloatData, int inFloatCnt,
                                               const char** inStringData, int inStringCnt,
                                               const char* inBufferData, int inBufferCnt,
                                               int** outIntData, int* outIntCnt,
                                               floatFloat** outFloatData, int* outFloatCnt,
                                               char*** outStringData, int* outStringCnt,
                                               char** outBufferData, int* outBufferSize)
{
    return(simExtCallScriptFunction_internal(scriptHandleOrType, functionNameAtScriptName,
                                             inIntData, inIntCnt, inFloatData, inFloatCnt,
                                             inStringData, inStringCnt, inBufferData, inBufferCnt,
                                             outIntData, outIntCnt, outFloatData, outFloatCnt,
                                             outStringData, outStringCnt, outBufferData, outBufferSize));
}
#endif

// Deprecated begin
SIM_DLLEXPORT int simGetMaterialId(const char* materialName)
{
    return(simGetMaterialId_internal(materialName));
}
SIM_DLLEXPORT int simGetShapeMaterial(int shapeHandle)
{
    return(simGetShapeMaterial_internal(shapeHandle));
}
SIM_DLLEXPORT int simHandleVarious()
{
    return(simHandleVarious_internal());
}
SIM_DLLEXPORT int simSerialPortOpen(int portNumber,int baudRate,void* reserved1,void* reserved2)
{
    return(simSerialPortOpen_internal(portNumber,baudRate,reserved1,reserved2));
}
SIM_DLLEXPORT int simSerialPortClose(int portNumber)
{
    return(simSerialPortClose_internal(portNumber));
}
SIM_DLLEXPORT int simSerialPortSend(int portNumber,const char* data,int dataLength)
{
    return(simSerialPortSend_internal(portNumber,data,dataLength));
}
SIM_DLLEXPORT int simSerialPortRead(int portNumber,char* buffer,int dataLengthToRead)
{
    return(simSerialPortRead_internal(portNumber,buffer,dataLengthToRead));
}
SIM_DLLEXPORT int simGetPathPlanningHandle(const char* pathPlanningObjectName)
{
    return(simGetPathPlanningHandle_internal(pathPlanningObjectName));
}
SIM_DLLEXPORT int simGetMotionPlanningHandle(const char* motionPlanningObjectName)
{
    return(simGetMotionPlanningHandle_internal(motionPlanningObjectName));
}
SIM_DLLEXPORT int simRemoveMotionPlanning(int motionPlanningHandle)
{
    return(simRemoveMotionPlanning_internal(motionPlanningHandle));
}
SIM_DLLEXPORT int simPerformPathSearchStep(int temporaryPathSearchObject,bool abortSearch)
{
    return(simPerformPathSearchStep_internal(temporaryPathSearchObject,abortSearch));
}
SIM_DLLEXPORT int simLockInterface(bool locked)
{
    return(simLockInterface_internal(locked));
}
SIM_DLLEXPORT int simCopyPasteSelectedObjects()
{
    return(simCopyPasteSelectedObjects_internal());
}
SIM_DLLEXPORT int simResetPath(int pathHandle)
{
    return(simResetPath_internal(pathHandle));
}
SIM_DLLEXPORT int simResetJoint(int jointHandle)
{
    return(simResetJoint_internal(jointHandle));
}
SIM_DLLEXPORT int simAppendScriptArrayEntry(const char* reservedSetToNull,int scriptHandleOrType,const char* arrayNameAtScriptName,const char* keyName,const char* data,const int* what)
{
    return(simAppendScriptArrayEntry_internal(reservedSetToNull,scriptHandleOrType,arrayNameAtScriptName,keyName,data,what));
}
SIM_DLLEXPORT int simClearScriptVariable(const char* reservedSetToNull,int scriptHandleOrType,const char* variableNameAtScriptName)
{
    return(simClearScriptVariable_internal(reservedSetToNull,scriptHandleOrType,variableNameAtScriptName));
}
SIM_DLLEXPORT bool _simGetBulletStickyContact(const void* geomInfo)
{
    return(_simGetBulletStickyContact_internal(geomInfo));
}
SIM_DLLEXPORT int simAddSceneCustomData(int header,const char* data,int dataLength)
{
    return(simAddSceneCustomData_internal(header,data,dataLength));
}
SIM_DLLEXPORT int simGetSceneCustomDataLength(int header)
{
    return(simGetSceneCustomDataLength_internal(header));
}
SIM_DLLEXPORT int simGetSceneCustomData(int header,char* data)
{
    return(simGetSceneCustomData_internal(header,data));
}
SIM_DLLEXPORT int simAddObjectCustomData(int objectHandle,int header,const char* data,int dataLength)
{
    return(simAddObjectCustomData_internal(objectHandle,header,data,dataLength));
}
SIM_DLLEXPORT int simGetObjectCustomDataLength(int objectHandle,int header)
{
    return(simGetObjectCustomDataLength_internal(objectHandle,header));
}
SIM_DLLEXPORT int simGetObjectCustomData(int objectHandle,int header,char* data)
{
    return(simGetObjectCustomData_internal(objectHandle,header,data));
}
SIM_DLLEXPORT int simCreateUI(const char* uiName,int menuAttributes,const int* clientSize,const int* cellSize,int* buttonHandles)
{
    return(simCreateUI_internal(uiName,menuAttributes,clientSize,cellSize,buttonHandles));
}
SIM_DLLEXPORT int simCreateUIButton(int uiHandle,const int* position,const int* size,int buttonProperty)
{
    return(simCreateUIButton_internal(uiHandle,position,size,buttonProperty));
}
SIM_DLLEXPORT int simGetUIHandle(const char* uiName)
{
    return(simGetUIHandle_internal(uiName));
}
SIM_DLLEXPORT int simGetUIProperty(int uiHandle)
{
    return(simGetUIProperty_internal(uiHandle));
}
SIM_DLLEXPORT int simGetUIEventButton(int uiHandle,int* auxiliaryValues)
{
    return(simGetUIEventButton_internal(uiHandle,auxiliaryValues));
}
SIM_DLLEXPORT int simSetUIProperty(int uiHandle,int elementProperty)
{
    return(simSetUIProperty_internal(uiHandle,elementProperty));
}
SIM_DLLEXPORT int simGetUIButtonProperty(int uiHandle,int buttonHandle)
{
    return(simGetUIButtonProperty_internal(uiHandle,buttonHandle));
}
SIM_DLLEXPORT int simSetUIButtonProperty(int uiHandle,int buttonHandle,int buttonProperty)
{
    return(simSetUIButtonProperty_internal(uiHandle,buttonHandle,buttonProperty));
}
SIM_DLLEXPORT int simGetUIButtonSize(int uiHandle,int buttonHandle,int* size)
{
    return(simGetUIButtonSize_internal(uiHandle,buttonHandle,size));
}
SIM_DLLEXPORT int simSetUIButtonLabel(int uiHandle,int buttonHandle,const char* upStateLabel,const char* downStateLabel)
{
    return(simSetUIButtonLabel_internal(uiHandle,buttonHandle,upStateLabel,downStateLabel));
}
SIM_DLLEXPORT char* simGetUIButtonLabel(int uiHandle,int buttonHandle)
{
    return(simGetUIButtonLabel_internal(uiHandle,buttonHandle));
}
SIM_DLLEXPORT int simSetUISlider(int uiHandle,int buttonHandle,int position)
{
    return(simSetUISlider_internal(uiHandle,buttonHandle,position));
}
SIM_DLLEXPORT int simGetUISlider(int uiHandle,int buttonHandle)
{
    return(simGetUISlider_internal(uiHandle,buttonHandle));
}
SIM_DLLEXPORT int simSetUIButtonTexture(int uiHandle,int buttonHandle,const int* size,const char* textureData)
{
    return(simSetUIButtonTexture_internal(uiHandle,buttonHandle,size,textureData));
}
SIM_DLLEXPORT int simCreateUIButtonArray(int uiHandle,int buttonHandle)
{
    return(simCreateUIButtonArray_internal(uiHandle,buttonHandle));
}
SIM_DLLEXPORT int simDeleteUIButtonArray(int uiHandle,int buttonHandle)
{
    return(simDeleteUIButtonArray_internal(uiHandle,buttonHandle));
}
SIM_DLLEXPORT int simRemoveUI(int uiHandle)
{
    return(simRemoveUI_internal(uiHandle));
}
SIM_DLLEXPORT int simSetUIPosition(int uiHandle,const int* position)
{
    return(simSetUIPosition_internal(uiHandle,position));
}
SIM_DLLEXPORT int simGetUIPosition(int uiHandle,int* position)
{
    return(simGetUIPosition_internal(uiHandle,position));
}
SIM_DLLEXPORT int simLoadUI(const char* filename,int maxCount,int* uiHandles)
{
    return(simLoadUI_internal(filename,maxCount,uiHandles));
}
SIM_DLLEXPORT int simSaveUI(int count,const int* uiHandles,const char* filename)
{
    return(simSaveUI_internal(count,uiHandles,filename));
}
SIM_DLLEXPORT int simHandleGeneralCallbackScript(int callbackId,int callbackTag,void* additionalData)
{
    return(simHandleGeneralCallbackScript_internal(callbackId,callbackTag,additionalData));
}
SIM_DLLEXPORT int simRegisterCustomLuaFunction(const char* funcName,const char* callTips,const int* inputArgumentTypes,void(*callBack)(struct SLuaCallBack* p))
{
    return(simRegisterCustomLuaFunction_internal(funcName,callTips,inputArgumentTypes,callBack));
}
SIM_DLLEXPORT int simRegisterCustomLuaVariable(const char* varName,const char* varValue)
{
    return(simRegisterScriptVariable_internal(varName,varValue,0));
}
SIM_DLLEXPORT int simGetMechanismHandle(const char* mechanismName)
{
    return(simGetMechanismHandle_internal(mechanismName));
}
SIM_DLLEXPORT int simHandleMechanism(int mechanismHandle)
{
    return(simHandleMechanism_internal(mechanismHandle));
}
SIM_DLLEXPORT int simHandleCustomizationScripts(int callType)
{
    return(simHandleCustomizationScripts_internal(callType));
}
SIM_DLLEXPORT int simCallScriptFunction(int scriptHandleOrType,const char* functionNameAtScriptName,SLuaCallBack* data,const char* reservedSetToNull)
{
    return(simCallScriptFunction_internal(scriptHandleOrType,functionNameAtScriptName,data,reservedSetToNull));
}
SIM_DLLEXPORT int _simGetJointDynCtrlMode(const void* joint)
{
    return(_simGetJointDynCtrlMode_internal(joint));
}
SIM_DLLEXPORT char* simGetScriptSimulationParameter(int scriptHandle,const char* parameterName,int* parameterLength)
{
    return(simGetScriptSimulationParameter_internal(scriptHandle,parameterName,parameterLength));
}
SIM_DLLEXPORT int simSetScriptSimulationParameter(int scriptHandle,const char* parameterName,const char* parameterValue,int parameterLength)
{
    return(simSetScriptSimulationParameter_internal(scriptHandle,parameterName,parameterValue,parameterLength));
}
SIM_DLLEXPORT int simResetMill(int millHandle)
{
    return(-1);
}
SIM_DLLEXPORT int simResetMilling(int objectHandle)
{
    return(-1);
}
SIM_DLLEXPORT int simApplyMilling(int objectHandle)
{
    return(-1);
}
SIM_DLLEXPORT bool _simGetParentFollowsDynamic(const void* shape)
{
    return(false);
}
SIM_DLLEXPORT int simGetNameSuffix(const char* name)
{
    return(simGetNameSuffix_internal(name));
}
SIM_DLLEXPORT int simSetNameSuffix(int nameSuffixNumber)
{
    return(simSetNameSuffix_internal(nameSuffixNumber));
}
SIM_DLLEXPORT int simAddStatusbarMessage(const char* message)
{
    return(simAddStatusbarMessage_internal(message));
}
SIM_DLLEXPORT char* simGetScriptRawBuffer(int scriptHandle,int bufferHandle)
{
    return(simGetScriptRawBuffer_internal(scriptHandle,bufferHandle));
}
SIM_DLLEXPORT int simSetScriptRawBuffer(int scriptHandle,const char* buffer,int bufferSize)
{
    return(simSetScriptRawBuffer_internal(scriptHandle,buffer,bufferSize));
}
SIM_DLLEXPORT int simReleaseScriptRawBuffer(int scriptHandle,int bufferHandle)
{
    return(simReleaseScriptRawBuffer_internal(scriptHandle,bufferHandle));
}
SIM_DLLEXPORT int simRemoveIkGroup(int ikGroupHandle)
{
    return(simRemoveIkGroup_internal(ikGroupHandle));
}
SIM_DLLEXPORT int simExportIk(const char* pathAndFilename,int reserved1,void* reserved2)
{
    return(simExportIk_internal(pathAndFilename,reserved1,reserved2));
}
SIM_DLLEXPORT int simComputeJacobian(int ikGroupHandle,int options,void* reserved)
{
    return(simComputeJacobian_internal(ikGroupHandle,options,reserved));
}
SIM_DLLEXPORT int simGetIkGroupHandle(const char* ikGroupName)
{
    return(simGetIkGroupHandle_internal(ikGroupName));
}
SIM_DLLEXPORT int simHandleIkGroup(int ikGroupHandle)
{
    return(simHandleIkGroup_internal(ikGroupHandle));
}
SIM_DLLEXPORT int simSetThreadIsFree(bool freeMode)
{
    return(simSetThreadIsFree_internal(freeMode));
}
SIM_DLLEXPORT int simTubeOpen(int dataHeader,const char* dataName,int readBufferSize,bool notUsedButKeepZero)
{
    return(simTubeOpen_internal(dataHeader,dataName,readBufferSize,notUsedButKeepZero));
}
SIM_DLLEXPORT int simTubeClose(int tubeHandle)
{
    return(simTubeClose_internal(tubeHandle));
}
SIM_DLLEXPORT int simTubeWrite(int tubeHandle,const char* data,int dataLength)
{
    return(simTubeWrite_internal(tubeHandle,data,dataLength));
}
SIM_DLLEXPORT char* simTubeRead(int tubeHandle,int* dataLength)
{
    return(simTubeRead_internal(tubeHandle,dataLength));
}
SIM_DLLEXPORT int simTubeStatus(int tubeHandle,int* readPacketsCount,int* writePacketsCount)
{
    return(simTubeStatus_internal(tubeHandle,readPacketsCount,writePacketsCount));
}
SIM_DLLEXPORT int simInsertPathCtrlPoints(int pathHandle,int options,int startIndex,int ptCnt,const void* ptData)
{
    return(simInsertPathCtrlPoints_internal(pathHandle,options,startIndex,ptCnt,ptData));
}
SIM_DLLEXPORT int simCutPathCtrlPoints(int pathHandle,int startIndex,int ptCnt)
{
    return(simCutPathCtrlPoints_internal(pathHandle,startIndex,ptCnt));
}
SIM_DLLEXPORT int simGetThreadId()
{
    return(simGetThreadId_internal());
}
SIM_DLLEXPORT int simSwitchThread()
{
    return(simSwitchThread_internal());
}
SIM_DLLEXPORT int simLockResources(int lockType,int reserved)
{
    return(simLockResources_internal(lockType,reserved));
}
SIM_DLLEXPORT int simUnlockResources(int lockHandle)
{
    return(simUnlockResources_internal(lockHandle));
}
SIM_DLLEXPORT char* simGetUserParameter(int objectHandle,const char* parameterName,int* parameterLength)
{
    return(simGetUserParameter_internal(objectHandle,parameterName,parameterLength));
}
SIM_DLLEXPORT int simSetUserParameter(int objectHandle,const char* parameterName,const char* parameterValue,int parameterLength)
{
    return(simSetUserParameter_internal(objectHandle,parameterName,parameterValue,parameterLength));
}
SIM_DLLEXPORT int simGetCollectionHandle(const char* collectionName)
{
    return(simGetCollectionHandle_internal(collectionName));
}
SIM_DLLEXPORT int simRemoveCollection(int collectionHandle)
{
    return(simRemoveCollection_internal(collectionHandle));
}
SIM_DLLEXPORT int simEmptyCollection(int collectionHandle)
{
    return(simEmptyCollection_internal(collectionHandle));
}
SIM_DLLEXPORT char* simGetCollectionName(int collectionHandle)
{
    return(simGetCollectionName_internal(collectionHandle));
}
SIM_DLLEXPORT int simSetCollectionName(int collectionHandle,const char* collectionName)
{
    return(simSetCollectionName_internal(collectionHandle,collectionName));
}
SIM_DLLEXPORT int simCreateCollection(const char* collectionName,int options)
{
    return(simCreateCollection_internal(collectionName,options));
}
SIM_DLLEXPORT int simAddObjectToCollection(int collectionHandle,int objectHandle,int what,int options)
{
    return(simAddObjectToCollection_internal(collectionHandle,objectHandle,what,options));
}
SIM_DLLEXPORT int simGetCollisionHandle(const char* collisionObjectName)
{
    return(simGetCollisionHandle_internal(collisionObjectName));
}
SIM_DLLEXPORT int simGetDistanceHandle(const char* distanceObjectName)
{
    return(simGetDistanceHandle_internal(distanceObjectName));
}
SIM_DLLEXPORT int simResetCollision(int collisionObjectHandle)
{
    return(simResetCollision_internal(collisionObjectHandle));
}
SIM_DLLEXPORT int simResetDistance(int distanceObjectHandle)
{
    return(simResetDistance_internal(distanceObjectHandle));
}
SIM_DLLEXPORT int simHandleCollision(int collisionObjectHandle)
{
    return(simHandleCollision_internal(collisionObjectHandle));
}
SIM_DLLEXPORT int simReadCollision(int collisionObjectHandle)
{
    return(simReadCollision_internal(collisionObjectHandle));
}
SIM_DLLEXPORT int simRemoveBanner(int bannerID)
{
    return(simRemoveBanner_internal(bannerID));
}
SIM_DLLEXPORT int simGetObjectIntParameter(int objectHandle,int ParamID,int* Param)
{
    return(simGetObjectInt32Param_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT int simSetObjectIntParameter(int objectHandle,int ParamID,int Param)
{
    return(simSetObjectInt32Param_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT int simGetObjectInt32Parameter(int objectHandle,int ParamID,int* Param)
{
    return(simGetObjectInt32Param_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT int simSetObjectInt32Parameter(int objectHandle,int ParamID,int Param)
{
    return(simSetObjectInt32Param_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT char* simGetObjectStringParameter(int objectHandle,int ParamID,int* ParamLength)
{
    return(simGetObjectStringParam_internal(objectHandle,ParamID,ParamLength));
}
SIM_DLLEXPORT int simSetObjectStringParameter(int objectHandle,int ParamID,const char* Param,int ParamLength)
{
    return(simSetObjectStringParam_internal(objectHandle,ParamID,Param,ParamLength));
}
SIM_DLLEXPORT int simSetBooleanParameter(int parameter,bool boolState)
{
    return(simSetBoolParam_internal(parameter,boolState));
}
SIM_DLLEXPORT int simGetBooleanParameter(int parameter)
{
    return(simGetBoolParam_internal(parameter));
}
SIM_DLLEXPORT int simSetBoolParameter(int parameter,bool boolState)
{
    return(simSetBoolParam_internal(parameter,boolState));
}
SIM_DLLEXPORT int simGetBoolParameter(int parameter)
{
    return(simGetBoolParam_internal(parameter));
}
SIM_DLLEXPORT int simSetIntegerParameter(int parameter,int intState)
{
    return(simSetInt32Param_internal(parameter,intState));
}
SIM_DLLEXPORT int simGetIntegerParameter(int parameter,int* intState)
{
    return(simGetInt32Param_internal(parameter,intState));
}
SIM_DLLEXPORT int simSetInt32Parameter(int parameter,int intState)
{
    return(simSetInt32Param_internal(parameter,intState));
}
SIM_DLLEXPORT int simGetInt32Parameter(int parameter,int* intState)
{
    return(simGetInt32Param_internal(parameter,intState));
}
SIM_DLLEXPORT int simGetUInt64Parameter(int parameter,unsigned long long int* intState)
{
    return(simGetUInt64Param_internal(parameter,intState));
}
SIM_DLLEXPORT int simSetStringParameter(int parameter,const char* str)
{
    return(simSetStringParam_internal(parameter,str));
}
SIM_DLLEXPORT char* simGetStringParameter(int parameter)
{
    return(simGetStringParam_internal(parameter));
}
SIM_DLLEXPORT int simSetArrayParameter(int parameter,const void* arrayOfValues)
{
    return(simSetArrayParam_internal(parameter,arrayOfValues));
}
SIM_DLLEXPORT int simGetArrayParameter(int parameter,void* arrayOfValues)
{
    return(simGetArrayParam_internal(parameter,arrayOfValues));
}
SIM_DLLEXPORT int simGetEngineInt32Parameter(int paramId,int objectHandle,const void* object,bool* ok)
{
    return(simGetEngineInt32Param_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT bool simGetEngineBoolParameter(int paramId,int objectHandle,const void* object,bool* ok)
{
    return(simGetEngineBoolParam_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT int simSetEngineInt32Parameter(int paramId,int objectHandle,const void* object,int val)
{
    return(simSetEngineInt32Param_internal(paramId,objectHandle,object,val));
}
SIM_DLLEXPORT int simSetEngineBoolParameter(int paramId,int objectHandle,const void* object,bool val)
{
    return(simSetEngineBoolParam_internal(paramId,objectHandle,object,val));
}
SIM_DLLEXPORT int simIsHandleValid(int generalObjectHandle,int generalObjectType)
{
    return(simIsHandle_internal(generalObjectHandle,generalObjectType));
}
SIM_DLLEXPORT int simAddModuleMenuEntry(const char* entryLabel,int itemCount,int* itemHandles)
{
    return(simAddModuleMenuEntry_internal(entryLabel,itemCount,itemHandles));
}
SIM_DLLEXPORT int simSetModuleMenuItemState(int itemHandle,int state,const char* label)
{
    return(simSetModuleMenuItemState_internal(itemHandle,state,label));
}
SIM_DLLEXPORT int simSetIntegerSignal(const char* signalName,int signalValue)
{
    return(simSetInt32Signal_internal(signalName,signalValue));
}
SIM_DLLEXPORT int simGetIntegerSignal(const char* signalName,int* signalValue)
{
    return(simGetInt32Signal_internal(signalName,signalValue));
}
SIM_DLLEXPORT int simClearIntegerSignal(const char* signalName)
{
    return(simClearInt32Signal_internal(signalName));
}
SIM_DLLEXPORT char* simGetObjectName(int objectHandle)
{
    return(simGetObjectName_internal(objectHandle));
}
SIM_DLLEXPORT int simSetObjectName(int objectHandle,const char* objectName)
{
    return(simSetObjectName_internal(objectHandle,objectName));
}
SIM_DLLEXPORT char* simGetScriptName(int scriptHandle)
{
    return(simGetScriptName_internal(scriptHandle));
}
SIM_DLLEXPORT int simGetScriptHandle(const char* scriptName)
{
    return(simGetScriptHandle_internal(scriptName));
}
SIM_DLLEXPORT int simSetScriptVariable(int scriptHandleOrType,const char* variableNameAtScriptName,int stackHandle)
{
    return(simSetScriptVariable_internal(scriptHandleOrType,variableNameAtScriptName,stackHandle));
}
SIM_DLLEXPORT int simGetObjectHandle(const char* objectAlias)
{
    return(simGetObjectHandleEx_internal(objectAlias,-1,-1,0));
}
SIM_DLLEXPORT int simGetObjectHandleEx(const char* objectAlias,int index,int proxy,int options)
{
    return(simGetObjectHandleEx_internal(objectAlias,index,proxy,options));
}
SIM_DLLEXPORT int simGetScript(int index)
{
    return(simGetScript_internal(index));
}
SIM_DLLEXPORT int simGetScriptAssociatedWithObject(int objectHandle)
{
    return(simGetScriptAssociatedWithObject_internal(objectHandle));
}
SIM_DLLEXPORT int simGetCustomizationScriptAssociatedWithObject(int objectHandle)
{
    return(simGetCustomizationScriptAssociatedWithObject_internal(objectHandle));
}
SIM_DLLEXPORT int simGetObjectAssociatedWithScript(int scriptHandle)
{
    return(simGetObjectAssociatedWithScript_internal(scriptHandle));
}
SIM_DLLEXPORT char* simGetObjectConfiguration(int objectHandle)
{
    return(simGetObjectConfiguration_internal(objectHandle));
}
SIM_DLLEXPORT int simSetObjectConfiguration(const char* data)
{
    return(simSetObjectConfiguration_internal(data));
}
SIM_DLLEXPORT char* simGetConfigurationTree(int objectHandle)
{
    return(simGetConfigurationTree_internal(objectHandle));
}
SIM_DLLEXPORT int simSetConfigurationTree(const char* data)
{
    return(simSetConfigurationTree_internal(data));
}
SIM_DLLEXPORT int simEnableEventCallback(int eventCallbackType,const char* plugin,int reserved)
{
    return(simEnableEventCallback_internal(eventCallbackType,plugin,reserved));
}
SIM_DLLEXPORT int simRMLPosition(int dofs,double timeStep,int flags,const double* currentPosVelAccel,const double* maxVelAccelJerk,const bool* selection,const double* targetPosVel,double* newPosVelAccel,void* auxData)
{
    return(simRMLPosition_internal(dofs,timeStep,flags,currentPosVelAccel,maxVelAccelJerk,selection,targetPosVel,newPosVelAccel,auxData));
}
SIM_DLLEXPORT int simRMLVelocity(int dofs,double timeStep,int flags,const double* currentPosVelAccel,const double* maxAccelJerk,const bool* selection,const double* targetVel,double* newPosVelAccel,void* auxData)
{
    return(simRMLVelocity_internal(dofs,timeStep,flags,currentPosVelAccel,maxAccelJerk,selection,targetVel,newPosVelAccel,auxData));
}
SIM_DLLEXPORT int simRMLPos(int dofs,double smallestTimeStep,int flags,const double* currentPosVelAccel,const double* maxVelAccelJerk,const bool* selection,const double* targetPosVel,void* auxData)
{
    return(simRMLPos_internal(dofs,smallestTimeStep,flags,currentPosVelAccel,maxVelAccelJerk,selection,targetPosVel,auxData));
}
SIM_DLLEXPORT int simRMLVel(int dofs,double smallestTimeStep,int flags,const double* currentPosVelAccel,const double* maxAccelJerk,const bool* selection,const double* targetVel,void* auxData)
{
    return(simRMLVel_internal(dofs,smallestTimeStep,flags,currentPosVelAccel,maxAccelJerk,selection,targetVel,auxData));
}
SIM_DLLEXPORT int simRMLStep(int handle,double timeStep,double* newPosVelAccel,void* auxData,void* reserved)
{
    return(simRMLStep_internal(handle,timeStep,newPosVelAccel,auxData,reserved));
}
SIM_DLLEXPORT int simRMLRemove(int handle)
{
    return(simRMLRemove_internal(handle));
}
SIM_DLLEXPORT int simGetSystemTimeInMilliseconds()
{
    return(simGetSystemTimeInMs_internal(-2));
}
SIM_DLLEXPORT unsigned int simGetSystemTimeInMs(int previousTime)
{
    return(simGetSystemTimeInMs_internal(previousTime));
}
SIM_DLLEXPORT char* simFileDialog(int mode,const char* title,const char* startPath,const char* initName,const char* extName,const char* ext)
{
    return(simFileDialog_internal(mode,title,startPath,initName,extName,ext));
}
SIM_DLLEXPORT int simMsgBox(int dlgType,int buttons,const char* title,const char* message)
{
    return(simMsgBox_internal(dlgType,buttons,title,message));
}
SIM_DLLEXPORT int simGetDialogResult(int genericDialogHandle)
{
    return(simGetDialogResult_internal(genericDialogHandle));
}
SIM_DLLEXPORT char* simGetDialogInput(int genericDialogHandle)
{
    return(simGetDialogInput_internal(genericDialogHandle));
}
SIM_DLLEXPORT int simEndDialog(int genericDialogHandle)
{
    return(simEndDialog_internal(genericDialogHandle));
}
SIM_DLLEXPORT int simIsObjectInSelection(int objectHandle)
{
    return(simIsObjectInSelection_internal(objectHandle));
}
SIM_DLLEXPORT int simAddObjectToSelection(int what,int objectHandle)
{
    return(simAddObjectToSelection_internal(what,objectHandle));
}
SIM_DLLEXPORT int simRemoveObjectFromSelection(int what,int objectHandle)
{
    return(simRemoveObjectFromSelection_internal(what,objectHandle));
}
SIM_DLLEXPORT int simGetObjectSelectionSize()
{
    return(simGetObjectSelectionSize_internal());
}
SIM_DLLEXPORT int simGetObjectLastSelection()
{
    return(simGetObjectLastSelection_internal());
}
SIM_DLLEXPORT int simGetObjectSelection(int* objectHandles)
{
    return(simGetObjectSelection_internal(objectHandles));
}
SIM_DLLEXPORT int simDeleteSelectedObjects()
{
    return(simDeleteSelectedObjects_internal());
}
SIM_DLLEXPORT int simSetStringNamedParam(const char* paramName,const char* stringParam,int paramLength)
{
    return(simSetNamedStringParam_internal(paramName,stringParam,paramLength));
}
SIM_DLLEXPORT char* simGetStringNamedParam(const char* paramName,int* paramLength)
{
    return(simGetNamedStringParam_internal(paramName,paramLength));
}
SIM_DLLEXPORT int simGetObjectUniqueIdentifier(int objectHandle,int* uniqueIdentifier)
{
    return(simGetObjectUniqueIdentifier_internal(objectHandle,uniqueIdentifier));
}
SIM_DLLEXPORT void _simSetDynamicJointLocalTransformationPart2IsValid(void* joint,bool valid)
{
    return(_simSetDynamicJointLocalTransformationPart2IsValid_internal(joint,valid));
}
SIM_DLLEXPORT void _simSetDynamicForceSensorLocalTransformationPart2IsValid(void* forceSensor,bool valid)
{
    return(_simSetDynamicForceSensorLocalTransformationPart2IsValid_internal(forceSensor,valid));
}
SIM_DLLEXPORT int simBreakForceSensor(int objectHandle)
{
    return(simBreakForceSensor_internal(objectHandle));
}
SIM_DLLEXPORT bool _simIsForceSensorBroken(const void* forceSensor)
{
    return(_simIsForceSensorBroken_internal(forceSensor));
}
SIM_DLLEXPORT int simSetScriptText(int scriptHandle,const char* scriptText)
{
    return(simSetScriptText_internal(scriptHandle,scriptText));
}
SIM_DLLEXPORT const char* simGetScriptText(int scriptHandle)
{
    return(simGetScriptText_internal(scriptHandle));
}
SIM_DLLEXPORT int simGetScriptProperty(int scriptHandle,int* scriptProperty,int* associatedObjectHandle)
{
    return(simGetScriptProperty_internal(scriptHandle,scriptProperty,associatedObjectHandle));
}
SIM_DLLEXPORT void _simSetGeomProxyDynamicsFullRefreshFlag(void* geomData,bool flag)
{
}
SIM_DLLEXPORT bool _simGetGeomProxyDynamicsFullRefreshFlag(const void* geomData)
{
    return(0);
}
SIM_DLLEXPORT int simRemoveObject(int objectHandle)
{
    return(simRemoveObject_internal(objectHandle));
}
SIM_DLLEXPORT void _simSetShapeIsStaticAndNotRespondableButDynamicTag(const void* shape,bool tag)
{
}
SIM_DLLEXPORT bool _simGetShapeIsStaticAndNotRespondableButDynamicTag(const void* shape)
{
    return(0);
}
SIM_DLLEXPORT int simGetVisionSensorResolution(int visionSensorHandle,int* resolution)
{
    return(simGetVisionSensorResolution_internal(visionSensorHandle,resolution));
}
SIM_DLLEXPORT unsigned char* simGetVisionSensorCharImage(int visionSensorHandle,int* resolutionX,int* resolutionY)
{
    return(simGetVisionSensorCharImage_internal(visionSensorHandle,resolutionX,resolutionY));
}
SIM_DLLEXPORT int simSetVisionSensorCharImage(int visionSensorHandle,const unsigned char* image)
{
    return(simSetVisionSensorCharImage_internal(visionSensorHandle,image));
}
SIM_DLLEXPORT floatFloat* simGetVisionSensorDepthBuffer(int visionSensorHandle)
{
    return(simGetVisionSensorDepthBuffer_internal(visionSensorHandle));
}
SIM_DLLEXPORT void* simBroadcastMessage(int* auxiliaryData,void* customData,int* replyData)
{
    return(simBroadcastMessage_internal(auxiliaryData,customData,replyData));
}
SIM_DLLEXPORT void* simSendModuleMessage(int message,int* auxiliaryData,void* customData,int* replyData)
{
    return(simSendModuleMessage_internal(message,auxiliaryData,customData,replyData));
}
SIM_DLLEXPORT bool _simIsDynamicMotorEnabled(const void* joint)
{
    return(0);
}
SIM_DLLEXPORT bool _simIsDynamicMotorPositionCtrlEnabled(const void* joint)
{
    return(0);
}
SIM_DLLEXPORT bool _simIsDynamicMotorTorqueModulationEnabled(const void* joint)
{
    return(0);
}
SIM_DLLEXPORT int _simGetContactCallbackCount()
{
    return(0);
}
SIM_DLLEXPORT const void* _simGetContactCallback(int index)
{
    return(nullptr);
}
SIM_DLLEXPORT int _simGetJointCallbackCallOrder(const void* joint)
{
    return(1); // not needed anymore
}
SIM_DLLEXPORT int simSetDoubleSignal(const char* signalName,double signalValue)
{
    return(simSetDoubleSignalOld_internal(signalName,signalValue));
}
SIM_DLLEXPORT int simGetDoubleSignal(const char* signalName,double* signalValue)
{
    return(simGetDoubleSignalOld_internal(signalName,signalValue));
}
SIM_DLLEXPORT int simClearDoubleSignal(const char* signalName)
{
    return(simClearDoubleSignalOld_internal(signalName));
}
SIM_DLLEXPORT int simRemoveParticleObject(int objectHandle)
{ // no effect anymore
    return(-1);
}

#ifdef switchToDouble
//************************
SIM_DLLEXPORT int simAddParticleObject_D(int objectType,double size,double density,const void* params,double lifeTime,int maxItemCount,const double* color,const double* setToNULL,const double* setToNULL2,const double* setToNULL3)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT int simAddParticleObjectItem_D(int objectHandle,const double* itemData)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT int simCreateMotionPlanning_D(int jointCnt,const int* jointHandles,const int* jointRangeSubdivisions,const double* jointMetricWeights,int options,const int* intParams,const double* floatParams,const void* reserved)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT int simJointGetForce_D(int jointHandle,double* forceOrTorque)
{
    return(simGetJointForce_internal(jointHandle,forceOrTorque));
}
SIM_DLLEXPORT int simGetMpConfigForTipPose_D(int motionPlanningObjectHandle,int options,double closeNodesDistance,int trialCount,const double* tipPose,int maxTimeInMs,double* outputJointPositions,const double* referenceConfigs,int referenceConfigCount,const double* jointWeights,const int* jointBehaviour,int correctionPasses)
{
    return(simGetMpConfigForTipPose_internal(motionPlanningObjectHandle,options,closeNodesDistance,trialCount,tipPose,maxTimeInMs,outputJointPositions,referenceConfigs,referenceConfigCount,jointWeights,jointBehaviour,correctionPasses));
}
SIM_DLLEXPORT double* simFindMpPath_D(int motionPlanningObjectHandle,const double* startConfig,const double* goalConfig,int options,double stepSize,int* outputConfigsCnt,int maxTimeInMs,double* reserved,const int* auxIntParams,const double* auxFloatParams)
{
    return(simFindMpPath_internal(motionPlanningObjectHandle,startConfig,goalConfig,options,stepSize,outputConfigsCnt,maxTimeInMs,reserved,auxIntParams,auxFloatParams));
}
SIM_DLLEXPORT double* simSimplifyMpPath_D(int motionPlanningObjectHandle,const double* pathBuffer,int configCnt,int options,double stepSize,int increment,int* outputConfigsCnt,int maxTimeInMs,double* reserved,const int* auxIntParams,const double* auxFloatParams)
{
    return(simSimplifyMpPath_internal(motionPlanningObjectHandle,pathBuffer,configCnt,options,stepSize,increment,outputConfigsCnt,maxTimeInMs,reserved,auxIntParams,auxFloatParams));
}
SIM_DLLEXPORT double* simFindIkPath_D(int motionPlanningObjectHandle,const double* startConfig,const double* goalPose,int options,double stepSize,int* outputConfigsCnt,double* reserved,const int* auxIntParams,const double* auxFloatParams)
{
    return(simFindIkPath_internal(motionPlanningObjectHandle,startConfig,goalPose,options,stepSize,outputConfigsCnt,reserved,auxIntParams,auxFloatParams));
}
SIM_DLLEXPORT double* simGetMpConfigTransition_D(int motionPlanningObjectHandle,const double* startConfig,const double* goalConfig,int options,const int* select,double calcStepSize,double maxOutStepSize,int wayPointCnt,const double* wayPoints,int* outputConfigsCnt,const int* auxIntParams,const double* auxFloatParams)
{
    return(simGetMpConfigTransition_internal(motionPlanningObjectHandle,startConfig,goalConfig,options,select,calcStepSize,maxOutStepSize,wayPointCnt,wayPoints,outputConfigsCnt,auxIntParams,auxFloatParams));
}
SIM_DLLEXPORT int simSearchPath_D(int pathPlanningObjectHandle,double maximumSearchTime)
{
    return(simSearchPath_internal(pathPlanningObjectHandle,maximumSearchTime));
}
SIM_DLLEXPORT int simInitializePathSearch_D(int pathPlanningObjectHandle,double maximumSearchTime,double searchTimeStep)
{
    return(simInitializePathSearch_internal(pathPlanningObjectHandle,maximumSearchTime,searchTimeStep));
}
SIM_DLLEXPORT int simHandlePath_D(int pathHandle,double deltaTime)
{
    return(simHandlePath_internal(pathHandle,deltaTime));
}
SIM_DLLEXPORT int simHandleJoint_D(int jointHandle,double deltaTime)
{
    return(simHandleJoint_internal(jointHandle,deltaTime));
}
SIM_DLLEXPORT int simSetUIButtonColor_D(int uiHandle,int buttonHandle,const double* upStateColor,const double* downStateColor,const double* labelColor)
{
    return(simSetUIButtonColor_internal(uiHandle,buttonHandle,upStateColor,downStateColor,labelColor));
}
SIM_DLLEXPORT int simSetUIButtonArrayColor_D(int uiHandle,int buttonHandle,const int* position,const double* color)
{
    return(simSetUIButtonArrayColor_internal(uiHandle,buttonHandle,position,color));
}
SIM_DLLEXPORT int simRegisterContactCallback_D(int(*callBack)(int,int,int,int*,double*))
{
    return(simRegisterContactCallback_internal(callBack));
}
SIM_DLLEXPORT int simRegisterJointCtrlCallback_D(int(*callBack)(int,int,int,const int*,const double*,double*))
{
    return(simRegisterJointCtrlCallback_internal(callBack));
}
SIM_DLLEXPORT int simSetJointForce_D(int objectHandle,double forceOrTorque)
{
    return(simSetJointMaxForce_internal(objectHandle,forceOrTorque));
}
SIM_DLLEXPORT int simHandleMill_D(int millHandle,double* removedSurfaceAndVolume)
{
    return(-1);
}
SIM_DLLEXPORT int simSetShapeMassAndInertia_D(int shapeHandle,double mass,const double* inertiaMatrix,const double* centerOfMass,const double* transformation)
{
    return(simSetShapeMassAndInertia_internal(shapeHandle,mass,inertiaMatrix,centerOfMass,transformation));
}
SIM_DLLEXPORT int simGetShapeMassAndInertia_D(int shapeHandle,double* mass,double* inertiaMatrix,double* centerOfMass,const double* transformation)
{
    return(simGetShapeMassAndInertia_internal(shapeHandle,mass,inertiaMatrix,centerOfMass,transformation));
}
SIM_DLLEXPORT int simCheckIkGroup_D(int ikGroupHandle,int jointCnt,const int* jointHandles,double* jointValues,const int* jointOptions)
{
    return(simCheckIkGroup_internal(ikGroupHandle,jointCnt,jointHandles,jointValues,jointOptions));
}
SIM_DLLEXPORT int simCreateIkGroup_D(int options,const int* intParams,const double* floatParams,const void* reserved)
{
    return(simCreateIkGroup_internal(options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT int simCreateIkElement_D(int ikGroupHandle,int options,const int* intParams,const double* floatParams,const void* reserved)
{
    return(simCreateIkElement_internal(ikGroupHandle,options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT int simGetConfigForTipPose_D(int ikGroupHandle,int jointCnt,const int* jointHandles,double thresholdDist,int maxTimeInMs,double* retConfig,const double* metric,int collisionPairCnt,const int* collisionPairs,const int* jointOptions,const double* lowLimits,const double* ranges,void* reserved)
{
    return(simGetConfigForTipPose_internal(ikGroupHandle,jointCnt,jointHandles,thresholdDist,maxTimeInMs,retConfig,metric,collisionPairCnt,collisionPairs,jointOptions,lowLimits,ranges,reserved));
}
SIM_DLLEXPORT double* simGenerateIkPath_D(int ikGroupHandle,int jointCnt,const int* jointHandles,int ptCnt,int collisionPairCnt,const int* collisionPairs,const int* jointOptions,void* reserved)
{
    return(simGenerateIkPath_internal(ikGroupHandle,jointCnt,jointHandles,ptCnt,collisionPairCnt,collisionPairs,jointOptions,reserved));
}
SIM_DLLEXPORT double* simGetIkGroupMatrix_D(int ikGroupHandle,int options,int* matrixSize)
{
    return(simGetIkGroupMatrix_internal(ikGroupHandle,options,matrixSize));
}
SIM_DLLEXPORT int simSetIkGroupProperties_D(int ikGroupHandle,int resolutionMethod,int maxIterations,double damping,void* reserved)
{
    return(simSetIkGroupProperties_internal(ikGroupHandle,resolutionMethod,maxIterations,damping,reserved));
}
SIM_DLLEXPORT int simGetPositionOnPath_D(int pathHandle,double relativeDistance,double* position)
{
    return(simGetPositionOnPath_internal(pathHandle,relativeDistance,position));
}
SIM_DLLEXPORT int simGetOrientationOnPath_D(int pathHandle,double relativeDistance,double* eulerAngles)
{
    return(simGetOrientationOnPath_internal(pathHandle,relativeDistance,eulerAngles));
}
SIM_DLLEXPORT int simGetDataOnPath_D(int pathHandle,double relativeDistance,int dataType,int* intData,double* floatData)
{
    return(simGetDataOnPath_internal(pathHandle,relativeDistance,dataType,intData,floatData));
}
SIM_DLLEXPORT int simGetClosestPositionOnPath_D(int pathHandle,double* absolutePosition,double* pathPosition)
{
    return(simGetClosestPositionOnPath_internal(pathHandle,absolutePosition,pathPosition));
}
SIM_DLLEXPORT int simGetPathPosition_D(int objectHandle,double* position)
{
    return(simGetPathPosition_internal(objectHandle,position));
}
SIM_DLLEXPORT int simSetPathPosition_D(int objectHandle,double position)
{
    return(simSetPathPosition_internal(objectHandle,position));
}
SIM_DLLEXPORT int simGetPathLength_D(int objectHandle,double* length)
{
    return(simGetPathLength_internal(objectHandle,length));
}
SIM_DLLEXPORT int simCreatePath_D(int attributes,const int* intParams,const double* floatParams,const double* color)
{
    return(simCreatePath_internal(attributes,intParams,floatParams,color));
}
SIM_DLLEXPORT int simSetIkElementProperties_D(int ikGroupHandle,int tipDummyHandle,int constraints,const double* precision,const double* weight,void* reserved)
{
    return(simSetIkElementProperties_internal(ikGroupHandle,tipDummyHandle,constraints,precision,weight,reserved));
}
SIM_DLLEXPORT int simSetVisionSensorFilter_D(int visionSensorHandle,int filterIndex,int options,const int* pSizes,const unsigned char* bytes,const int* ints,const double* floats,const unsigned char* custom)
{
    return(simSetVisionSensorFilter_internal(visionSensorHandle,filterIndex,options,pSizes,bytes,ints,floats,custom));
}
SIM_DLLEXPORT int simGetVisionSensorFilter_D(int visionSensorHandle,int filterIndex,int* options,int* pSizes,unsigned char** bytes,int** ints,double** floats,unsigned char** custom)
{
    return(simGetVisionSensorFilter_internal(visionSensorHandle,filterIndex,options,pSizes,bytes,ints,floats,custom));
}
SIM_DLLEXPORT int simSetPathTargetNominalVelocity_D(int objectHandle,double targetNominalVelocity)
{
    return(simSetPathTargetNominalVelocity_internal(objectHandle,targetNominalVelocity));
}
SIM_DLLEXPORT int simSendData_D(int targetID,int dataHeader,const char* dataName,const char* data,int dataLength,int antennaHandle,double actionRadius,double emissionAngle1,double emissionAngle2,double persistence)
{
    return(simSendData_internal(targetID,dataHeader,dataName,data,dataLength,antennaHandle,actionRadius,emissionAngle1,emissionAngle2,persistence));
}
SIM_DLLEXPORT int simHandleDistance_D(int distanceObjectHandle,double* smallestDistance)
{
    return(simHandleDistance_internal(distanceObjectHandle,smallestDistance));
}
SIM_DLLEXPORT int simReadDistance_D(int distanceObjectHandle,double* smallestDistance)
{
    return(simReadDistance_internal(distanceObjectHandle,smallestDistance));
}
SIM_DLLEXPORT int simAddBanner_D(const char* label,double size,int options,const double* positionAndEulerAngles,int parentObjectHandle,const double* labelColors,const double* backgroundColors)
{
    return(simAddBanner_internal(label,size,options,positionAndEulerAngles,parentObjectHandle,labelColors,backgroundColors));
}
SIM_DLLEXPORT int simAddGhost_D(int ghostGroup,int objectHandle,int options,double startTime,double endTime,const double* color)
{
    return(simAddGhost_internal(ghostGroup,objectHandle,options,startTime,endTime,color));
}
SIM_DLLEXPORT int simModifyGhost_D(int ghostGroup,int ghostId,int operation,double floatValue,int options,int optionsMask,const double* colorOrTransformation)
{
    return(simModifyGhost_internal(ghostGroup,ghostId,operation,floatValue,options,optionsMask,colorOrTransformation));
}
SIM_DLLEXPORT int simSetGraphUserData_D(int graphHandle,const char* streamName,double data)
{
    return(simSetGraphUserData_internal(graphHandle,streamName,data));
}
SIM_DLLEXPORT int simAddPointCloud_D(int pageMask,int layerMask,int objectHandle,int options,double pointSize,int ptCnt,const double* pointCoordinates,const char* defaultColors,const char* pointColors,const double* pointNormals)
{
    return(simAddPointCloud_internal(pageMask,layerMask,objectHandle,options,pointSize,ptCnt,pointCoordinates,defaultColors,pointColors,pointNormals));
}
SIM_DLLEXPORT int simModifyPointCloud_D(int pointCloudHandle,int operation,const int* intParam,const double* floatParam)
{
    return(simModifyPointCloud_internal(pointCloudHandle,operation,intParam,floatParam));
}
SIM_DLLEXPORT int simCopyMatrix_D(const double* matrixIn,double* matrixOut)
{
    return(simCopyMatrix_internal(matrixIn,matrixOut));
}
SIM_DLLEXPORT int simGetObjectFloatParameter_D(int objectHandle,int ParamID,double* Param)
{
    return(simGetObjectFloatParam_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT int simSetObjectFloatParameter_D(int objectHandle,int ParamID,double Param)
{
    return(simSetObjectFloatParam_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT int simSetFloatingParameter_D(int parameter,double floatState)
{
    return(simSetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT int simGetFloatingParameter_D(int parameter,double* floatState)
{
    return(simGetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT int simSetFloatParameter_D(int parameter,double floatState)
{
    return(simSetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT int simGetFloatParameter_D(int parameter,double* floatState)
{
    return(simGetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT double simGetEngineFloatParameter_D(int paramId,int objectHandle,const void* object,bool* ok)
{
    return(simGetEngineFloatParam_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT int simSetEngineFloatParameter_D(int paramId,int objectHandle,const void* object,double val)
{
    return(simSetEngineFloatParam_internal(paramId,objectHandle,object,val));
}
SIM_DLLEXPORT int simSetObjectSizeValues_D(int objectHandle,const double* sizeValues)
{
    return(simSetObjectSizeValues_internal(objectHandle,sizeValues));
}
SIM_DLLEXPORT int simGetObjectSizeValues_D(int objectHandle,double* sizeValues)
{
    return(simGetObjectSizeValues_internal(objectHandle,sizeValues));
}
SIM_DLLEXPORT int simDisplayDialog_D(const char* titleText,const char* mainText,int dialogType,const char* initialText,const double* titleColors,const double* dialogColors,int* elementHandle)
{
    return(simDisplayDialog_internal(titleText,mainText,dialogType,initialText,titleColors,dialogColors,elementHandle));
}
SIM_DLLEXPORT int simScaleSelectedObjects_D(double scalingFactor,bool scalePositionsToo)
{
    return(simScaleSelectedObjects_internal(scalingFactor,scalePositionsToo));
}
SIM_DLLEXPORT int simGetJointMatrix_D(int objectHandle,double* matrix)
{
    return(simGetJointMatrix_internal(objectHandle,matrix));
}
SIM_DLLEXPORT int simSetSphericalJointMatrix_D(int objectHandle,const double* matrix)
{
    return(simSetSphericalJointMatrix_internal(objectHandle,matrix));
}
SIM_DLLEXPORT int simSetScriptAttribute_D(int scriptHandle,int attributeID,double floatVal,int intOrBoolVal)
{
    return(simSetScriptAttribute_internal(scriptHandle,attributeID,floatVal,intOrBoolVal));
}
SIM_DLLEXPORT int simGetScriptAttribute_D(int scriptHandle,int attributeID,double* floatVal,int* intOrBoolVal)
{
    return(simGetScriptAttribute_internal(scriptHandle,attributeID,floatVal,intOrBoolVal));
}
SIM_DLLEXPORT int simGetJointMaxForce_D(int jointHandle,double* forceOrTorque)
{
    return(simGetJointMaxForce_internal(jointHandle,forceOrTorque));
}
SIM_DLLEXPORT int simSetJointMaxForce_D(int objectHandle,double forceOrTorque)
{
    return(simSetJointMaxForce_internal(objectHandle,forceOrTorque));
}
SIM_DLLEXPORT double* simGetVisionSensorImage_D(int visionSensorHandle)
{
    return(simGetVisionSensorImage_internal(visionSensorHandle));
}
SIM_DLLEXPORT int simSetVisionSensorImage_D(int visionSensorHandle,const double* image)
{
    return(simSetVisionSensorImage_internal(visionSensorHandle,image));
}
SIM_DLLEXPORT int simSetVisionSensorDepthBuffer_D(int visionSensorHandle,const double* depthBuffer)
{
    return(-1);
}
SIM_DLLEXPORT int simCreatePureShape_D(int primitiveType,int options,const double* sizes,double mass,const int* precision)
{
    return(simCreatePureShape_internal(primitiveType,options,sizes,mass,precision));
}
SIM_DLLEXPORT int simBuildMatrixQ_D(const double* position,const double* quaternion,double* matrix)
{
    return(simBuildMatrixQ_internal(position,quaternion,matrix));
}
SIM_DLLEXPORT int simGetQuaternionFromMatrix_D(const double* matrix,double* quaternion)
{
    return(simGetQuaternionFromMatrix_internal(matrix,quaternion));
}
SIM_DLLEXPORT int simGetShapeVertex_D(int shapeHandle,int groupElementIndex,int vertexIndex,double* relativePosition)
{
    return(simGetShapeVertex_internal(shapeHandle,groupElementIndex,vertexIndex,relativePosition));
}
SIM_DLLEXPORT int simGetShapeTriangle_D(int shapeHandle,int groupElementIndex,int triangleIndex,int* vertexIndices,double* triangleNormals)
{
    return(simGetShapeTriangle_internal(shapeHandle,groupElementIndex,triangleIndex,vertexIndices,triangleNormals));
}




SIM_DLLEXPORT void _simGetJointOdeParameters_D(const void* joint,double* stopERP,double* stopCFM,double* bounce,double* fudge,double* normalCFM)
{
    return(_simGetJointOdeParameters_internal(joint,stopERP,stopCFM,bounce,fudge,normalCFM));
}
SIM_DLLEXPORT void _simGetJointBulletParameters_D(const void* joint,double* stopERP,double* stopCFM,double* normalCFM)
{
    return(_simGetJointBulletParameters_internal(joint,stopERP,stopCFM,normalCFM));
}
SIM_DLLEXPORT void _simGetOdeMaxContactFrictionCFMandERP_D(const void* geomInfo,int* maxContacts,double* friction,double* cfm,double* erp)
{
    return(_simGetOdeMaxContactFrictionCFMandERP_internal(geomInfo,maxContacts,friction,cfm,erp));
}
SIM_DLLEXPORT bool _simGetBulletCollisionMargin_D(const void* geomInfo,double* margin,int* otherProp)
{
    return(_simGetBulletCollisionMargin_internal(geomInfo,margin,otherProp));
}
SIM_DLLEXPORT double _simGetBulletRestitution_D(const void* geomInfo)
{
    return(_simGetBulletRestitution_internal(geomInfo));
}
SIM_DLLEXPORT void _simGetVortexParameters_D(const void* object,int version,double* floatParams,int* intParams)
{
    _simGetVortexParameters_internal(object,version,floatParams,intParams);
}
SIM_DLLEXPORT void _simGetNewtonParameters_D(const void* object,int* version,double* floatParams,int* intParams)
{
    _simGetNewtonParameters_internal(object,version,floatParams,intParams);
}
SIM_DLLEXPORT void _simGetDamping_D(const void* geomInfo,double* linDamping,double* angDamping)
{
    return(_simGetDamping_internal(geomInfo,linDamping,angDamping));
}
SIM_DLLEXPORT double _simGetFriction_D(const void* geomInfo)
{
    return(_simGetFriction_internal(geomInfo));
}
SIM_DLLEXPORT void _simSetDynamicJointLocalTransformationPart2_D(void* joint,const double* pos,const double* quat)
{
    return(_simSetDynamicJointLocalTransformationPart2_internal(joint,pos,quat));
}
SIM_DLLEXPORT void _simSetDynamicForceSensorLocalTransformationPart2_D(void* forceSensor,const double* pos,const double* quat)
{
    return(_simSetDynamicForceSensorLocalTransformationPart2_internal(forceSensor,pos,quat));
}
SIM_DLLEXPORT void _simGetDynamicForceSensorLocalTransformationPart2_D(const void* forceSensor,double* pos,double* quat)
{
    return(_simGetDynamicForceSensorLocalTransformationPart2_internal(forceSensor,pos,quat));
}
SIM_DLLEXPORT void _simGetMotorPid_D(const void* joint,double* pParam,double* iParam,double* dParam)
{
}
SIM_DLLEXPORT void _simGetPrincipalMomentOfInertia_D(const void* geomInfo,double* inertia)
{
    return(_simGetPrincipalMomentOfInertia_internal(geomInfo,inertia));
}
SIM_DLLEXPORT void _simGetLocalInertiaFrame_D(const void* geomInfo,double* pos,double* quat)
{
    return(_simGetLocalInertiaFrame_internal(geomInfo,pos,quat));
}
//************************
SIM_DLLEXPORT int simAddParticleObject(int objectType,floatFloat size,floatFloat density,const void* params,floatFloat lifeTime,int maxItemCount,const floatFloat* color,const floatFloat* setToNULL,const floatFloat* setToNULL2,const floatFloat* setToNULL3)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT int simAddParticleObjectItem(int objectHandle,const floatFloat* itemData)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT int simCreateMotionPlanning(int jointCnt,const int* jointHandles,const int* jointRangeSubdivisions,const floatFloat* jointMetricWeights,int options,const int* intParams,const floatFloat* floatParams,const void* reserved)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT int simJointGetForce(int jointHandle,floatFloat* forceOrTorque)
{
    double f;
    int retVal=simGetJointForce_internal(jointHandle,&f);
    forceOrTorque[0]=(floatFloat)f;
    return(retVal);
}
SIM_DLLEXPORT int simGetMpConfigForTipPose(int motionPlanningObjectHandle,int options,floatFloat closeNodesDistance,int trialCount,const floatFloat* tipPose,int maxTimeInMs,floatFloat* outputJointPositions,const floatFloat* referenceConfigs,int referenceConfigCount,const floatFloat* jointWeights,const int* jointBehaviour,int correctionPasses)
{
    return(-1);
}
SIM_DLLEXPORT floatFloat* simFindMpPath(int motionPlanningObjectHandle,const floatFloat* startConfig,const floatFloat* goalConfig,int options,floatFloat stepSize,int* outputConfigsCnt,int maxTimeInMs,floatFloat* reserved,const int* auxIntParams,const floatFloat* auxFloatParams)
{
    return(nullptr);
}
SIM_DLLEXPORT floatFloat* simSimplifyMpPath(int motionPlanningObjectHandle,const floatFloat* pathBuffer,int configCnt,int options,floatFloat stepSize,int increment,int* outputConfigsCnt,int maxTimeInMs,floatFloat* reserved,const int* auxIntParams,const floatFloat* auxFloatParams)
{
    return(nullptr);
}
SIM_DLLEXPORT floatFloat* simFindIkPath(int motionPlanningObjectHandle,const floatFloat* startConfig,const floatFloat* goalPose,int options,floatFloat stepSize,int* outputConfigsCnt,floatFloat* reserved,const int* auxIntParams,const floatFloat* auxFloatParams)
{
    return(nullptr);
}
SIM_DLLEXPORT floatFloat* simGetMpConfigTransition(int motionPlanningObjectHandle,const floatFloat* startConfig,const floatFloat* goalConfig,int options,const int* select,floatFloat calcStepSize,floatFloat maxOutStepSize,int wayPointCnt,const floatFloat* wayPoints,int* outputConfigsCnt,const int* auxIntParams,const floatFloat* auxFloatParams)
{
    return(nullptr);
}
SIM_DLLEXPORT int simSearchPath(int pathPlanningObjectHandle,floatFloat maximumSearchTime)
{
    return(-1);
}
SIM_DLLEXPORT int simInitializePathSearch(int pathPlanningObjectHandle,floatFloat maximumSearchTime,floatFloat searchTimeStep)
{
    return(-1);
}
SIM_DLLEXPORT int simHandlePath(int pathHandle,floatFloat deltaTime)
{
    return(-1);
}
SIM_DLLEXPORT int simHandleJoint(int jointHandle,floatFloat deltaTime)
{
    return(-1);
}
SIM_DLLEXPORT int simSetUIButtonColor(int uiHandle,int buttonHandle,const floatFloat* upStateColor,const floatFloat* downStateColor,const floatFloat* labelColor)
{
    return(-1);
}
SIM_DLLEXPORT int simSetUIButtonArrayColor(int uiHandle,int buttonHandle,const int* position,const floatFloat* color)
{
    return(-1);
}
SIM_DLLEXPORT int simRegisterContactCallback(int(*callBack)(int,int,int,int*,floatFloat*))
{
    return(-1);
}
SIM_DLLEXPORT int simRegisterJointCtrlCallback(int(*callBack)(int,int,int,const int*,const floatFloat*,floatFloat*))
{
    return(-1);
}
SIM_DLLEXPORT int simSetJointForce(int objectHandle,floatFloat forceOrTorque)
{
    return(simSetJointMaxForce_internal(objectHandle,(double)forceOrTorque));
}
SIM_DLLEXPORT int simHandleMill(int millHandle,floatFloat* removedSurfaceAndVolume)
{
    return(-1);
}
SIM_DLLEXPORT int simSetShapeMassAndInertia(int shapeHandle,floatFloat mass,const floatFloat* inertiaMatrix,const floatFloat* centerOfMass,const floatFloat* transformation)
{
    double ine[9];
    for (size_t i=0;i<9;i++)
        ine[i]=(double)inertiaMatrix[i];
    double c[3];
    for (size_t i=0;i<3;i++)
        c[i]=(double)centerOfMass[i];
    double tr[12];
    for (size_t i=0;i<12;i++)
        tr[i]=(double)transformation[i];
    return(simSetShapeMassAndInertia_internal(shapeHandle,(double)mass,ine,c,tr));
}
SIM_DLLEXPORT int simGetShapeMassAndInertia(int shapeHandle,floatFloat* mass,floatFloat* inertiaMatrix,floatFloat* centerOfMass,const floatFloat* transformation)
{
    double m;
    double ine[9];
    double c[3];
    double tr[12];
    int retVal=simGetShapeMassAndInertia_internal(shapeHandle,&m,ine,c,tr);
    if (mass!=nullptr)
        mass[0]=(floatFloat)m;
    if (inertiaMatrix!=nullptr)
    {
        for (size_t i=0;i<9;i++)
            inertiaMatrix[i]=(floatFloat)ine[i];
    }
    if (centerOfMass!=nullptr)
    {
        for (size_t i=0;i<3;i++)
            centerOfMass[i]=(floatFloat)c[i];
    }
    if (transformation!=nullptr)
    {
        for (size_t i=0;i<12;i++)
            transformation[i]=(floatFloat)tr[i];
    }
    return(retVal);
}
SIM_DLLEXPORT int simCheckIkGroup(int ikGroupHandle,int jointCnt,const int* jointHandles,floatFloat* jointValues,const int* jointOptions)
{
    return(-1);
}
SIM_DLLEXPORT int simCreateIkGroup(int options,const int* intParams,const floatFloat* floatParams,const void* reserved)
{
    return(-1);
}
SIM_DLLEXPORT int simCreateIkElement(int ikGroupHandle,int options,const int* intParams,const floatFloat* floatParams,const void* reserved)
{
    return(-1);
}
SIM_DLLEXPORT int simGetConfigForTipPose(int ikGroupHandle,int jointCnt,const int* jointHandles,floatFloat thresholdDist,int maxTimeInMs,floatFloat* retConfig,const floatFloat* metric,int collisionPairCnt,const int* collisionPairs,const int* jointOptions,const floatFloat* lowLimits,const floatFloat* ranges,void* reserved)
{
    return(-1);
}
SIM_DLLEXPORT floatFloat* simGenerateIkPath(int ikGroupHandle,int jointCnt,const int* jointHandles,int ptCnt,int collisionPairCnt,const int* collisionPairs,const int* jointOptions,void* reserved)
{
    return(-1);
}
SIM_DLLEXPORT floatFloat* simGetIkGroupMatrix(int ikGroupHandle,int options,int* matrixSize)
{
    return(nullptr);
}
SIM_DLLEXPORT int simSetIkGroupProperties(int ikGroupHandle,int resolutionMethod,int maxIterations,floatFloat damping,void* reserved)
{
    return(-1);
}
SIM_DLLEXPORT int simGetPositionOnPath(int pathHandle,floatFloat relativeDistance,floatFloat* position)
{
    double p[3];
    int retVal=simGetPositionOnPath_internal(pathHandle,relativeDistance,p);
    for (size_t i=0;i<3;i++)
        position[i]=(floatFloat)p[i];
    return(retVal);
}
SIM_DLLEXPORT int simGetOrientationOnPath(int pathHandle,floatFloat relativeDistance,floatFloat* eulerAngles)
{
    double p[3];
    int retVal=simGetOrientationOnPath_internal(pathHandle,relativeDistance,p);
    for (size_t i=0;i<3;i++)
        eulerAngles[i]=(floatFloat)p[i];
    return(retVal);
}
SIM_DLLEXPORT int simGetDataOnPath(int pathHandle,floatFloat relativeDistance,int dataType,int* intData,floatFloat* floatData)
{
    double d[4];
    int retVal=simGetDataOnPath_internal(pathHandle,(double)relativeDistance,dataType,intData,d);
    for (size_t i=0;i<4;i++)
        floatData[i]=(floatFloat)d[i];
    return(retVal);
}
SIM_DLLEXPORT int simGetClosestPositionOnPath(int pathHandle,floatFloat* absolutePosition,floatFloat* pathPosition)
{
    double pp[3];
    for (size_t i=0;i<3;i++)
        pp[i]=(double)absolutePosition[i];
    double p;
    int retVal=simGetClosestPositionOnPath_internal(pathHandle,pp,&p);
    position[0]=(floatFloat)p;
    return(retVal);
}
SIM_DLLEXPORT int simGetPathPosition(int objectHandle,floatFloat* position)
{
    double p;
    int retVal=simGetPathPosition_internal(objectHandle,&p);
    position[0]=(floatFloat)p;
    return(retVal);
}
SIM_DLLEXPORT int simSetPathPosition(int objectHandle,floatFloat position)
{
    return(simSetPathPosition_internal(objectHandle,(double)position));
}
SIM_DLLEXPORT int simGetPathLength(int objectHandle,floatFloat* length)
{
    double l;
    int retVal=simGetPathLength_internal(objectHandle,&l);
    length[0]=(floatFloat)l;
    return(retVal);
}
SIM_DLLEXPORT int simCreatePath(int attributes,const int* intParams,const floatFloat* floatParams,const floatFloat* color)
{
    double p[3];
    double* p_=nullptr;
    if (floatParams!=nullptr)
    {
        for (size_t i=0;i<3;i++)
            p[i]=(double)floatParams[i];
        p_=p;
    }
    double c[12];
    double* c_=nullptr;
    if (color!=nullptr)
    {
        for (size_t i=0;i<12;i++)
            c[i]=(double)color[i];
        c_=c;
    }
    return(simCreatePath_internal(attributes,intParams,p_,c_));
}
SIM_DLLEXPORT int simSetIkElementProperties(int ikGroupHandle,int tipDummyHandle,int constraints,const floatFloat* precision,const floatFloat* weight,void* reserved)
{
    return(-1);
}
SIM_DLLEXPORT int simSetVisionSensorFilter(int visionSensorHandle,int filterIndex,int options,const int* pSizes,const unsigned char* bytes,const int* ints,const floatFloat* floats,const unsigned char* custom)
{
    return(-1);
}
SIM_DLLEXPORT int simGetVisionSensorFilter(int visionSensorHandle,int filterIndex,int* options,int* pSizes,unsigned char** bytes,int** ints,floatFloat** floats,unsigned char** custom)
{
    return(-1);
}
SIM_DLLEXPORT int simSetPathTargetNominalVelocity(int objectHandle,floatFloat targetNominalVelocity)
{
    return(simSetPathTargetNominalVelocity_internal(objectHandle,(double)targetNominalVelocity));
}
SIM_DLLEXPORT int simSendData(int targetID,int dataHeader,const char* dataName,const char* data,int dataLength,int antennaHandle,floatFloat actionRadius,floatFloat emissionAngle1,floatFloat emissionAngle2,floatFloat persistence)
{
    return(simSendData_internal(targetID,dataHeader,dataName,data,dataLength,antennaHandle,(double)actionRadius,(double)emissionAngle1,(double)emissionAngle2,(double)persistence));
}
SIM_DLLEXPORT int simHandleDistance(int distanceObjectHandle,floatFloat* smallestDistance)
{
    double d;
    int retVal=simHandleDistance_internal(distanceObjectHandle,&d);
    if ( (retVal>0)&&(smallestDistance!=nullptr) )
        smallestDistance[0]=(floatFloat)d;
    return(retVal);
}
SIM_DLLEXPORT int simReadDistance(int distanceObjectHandle,floatFloat* smallestDistance)
{
    double d;
    int retVal=simReadDistance_internal(distanceObjectHandle,&d);
    if ( (retVal!=-1)&&(smallestDistance!=nullptr) )
        smallestDistance[0]=(floatFloat)d;
    return(retVal);
}
SIM_DLLEXPORT int simAddBanner(const char* label,floatFloat size,int options,const floatFloat* positionAndEulerAngles,int parentObjectHandle,const floatFloat* labelColors,const floatFloat* backgroundColors)
{
    return(-1);
}
SIM_DLLEXPORT int simAddGhost(int ghostGroup,int objectHandle,int options,floatFloat startTime,floatFloat endTime,const floatFloat* color)
{
    return(-1);
}
SIM_DLLEXPORT int simModifyGhost(int ghostGroup,int ghostId,int operation,floatFloat floatValue,int options,int optionsMask,const floatFloat* colorOrTransformation)
{
    return(-1);
}
SIM_DLLEXPORT int simSetGraphUserData(int graphHandle,const char* streamName,floatFloat data)
{
    return(simSetGraphUserData_internal(graphHandle,streamName,(double)data));
}
SIM_DLLEXPORT int simAddPointCloud(int pageMask,int layerMask,int objectHandle,int options,floatFloat pointSize,int ptCnt,const floatFloat* pointCoordinates,const char* defaultColors,const char* pointColors,const floatFloat* pointNormals)
{
    return(-1);
}
SIM_DLLEXPORT int simModifyPointCloud(int pointCloudHandle,int operation,const int* intParam,const floatFloat* floatParam)
{
    return(-1);
}
SIM_DLLEXPORT int simCopyMatrix(const floatFloat* matrixIn,floatFloat* matrixOut)
{
    double m1[12];
    for (size_t i=0;i<12;i++)
        m1[i]=(double)matrixIn[i];
    double m2[12];
    int retVal=simCopyMatrix_internal(m1,m2);
    for (size_t i=0;i<12;i++)
        matrixOut[i]=(floatFloat)m2[i];
    return(retVal);
}
SIM_DLLEXPORT int simGetObjectFloatParameter(int objectHandle,int ParamID,floatFloat* Param)
{
    double d;
    int retVal=simGetObjectFloatParam_internal(objectHandle,ParamID,&d);
    Param[0]=(floatFloat)d;
    return(retVal);
}
SIM_DLLEXPORT int simSetObjectFloatParameter(int objectHandle,int ParamID,floatFloat Param)
{
    return(simSetObjectFloatParam_internal(objectHandle,ParamID,(double)Param));
}
SIM_DLLEXPORT int simSetFloatingParameter(int parameter,floatFloat floatState)
{
    return(simSetFloatParam_internal(parameter,(double)floatState));
}
SIM_DLLEXPORT int simGetFloatingParameter(int parameter,floatFloat* floatState)
{
    double d;
    int retVal=simGetFloatParam_internal(parameter,&d);
    floatState[0]=(floatFloat)d;
    return(retVal);
}
SIM_DLLEXPORT int simSetFloatParameter(int parameter,floatFloat floatState)
{
    return(simSetFloatParam_internal(parameter,(double)floatState));
}
SIM_DLLEXPORT int simGetFloatParameter(int parameter,floatFloat* floatState)
{
    double d;
    int retVal=simGetFloatParam_internal(parameter,&d);
    floatState[0]=(floatFloat)d;
    return(retVal);
}
SIM_DLLEXPORT floatFloat simGetEngineFloatParameter(int paramId,int objectHandle,const void* object,bool* ok)
{
    return((floatFloat)simGetEngineFloatParam_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT int simSetEngineFloatParameter(int paramId,int objectHandle,const void* object,floatFloat val)
{
    return(simSetEngineFloatParam_internal(paramId,objectHandle,object,(double)val));
}
SIM_DLLEXPORT int simSetObjectSizeValues(int objectHandle,const floatFloat* sizeValues)
{
    double v[3];
    for (size_t i=0;i<3;i++)
        v[i]=(double)sizeValues[i];
    return(simSetObjectSizeValues_internal(objectHandle,v));
}
SIM_DLLEXPORT int simGetObjectSizeValues(int objectHandle,floatFloat* sizeValues)
{
    double v[3];
    int retVal=simGetObjectSizeValues_internal(objectHandle,v);
    for (size_t i=0;i<3;i++)
        sizeValues[i]=(floatFloat)v[i];
    return(retVal);
}
SIM_DLLEXPORT int simDisplayDialog(const char* titleText,const char* mainText,int dialogType,const char* initialText,const floatFloat* titleColors,const floatFloat* dialogColors,int* elementHandle)
{
    double c1[6];
    double* c1_=nullptr;
    double c2[6];
    double* c2_=nullptr;
    for (size_t i=0;i<6;i++)
    {
        if (titleColors!=nullptr)
        {
            c1[i]=(double)titleColors[i];
            c1_=c1;
        }
        if (dialogColors!=nullptr)
        {
            c2[i]=(double)dialogColors[i];
            c2_=c2;
        }
    }
    return(simDisplayDialog_internal(titleText,mainText,dialogType,initialText,c1_,c2_,elementHandle));
}
SIM_DLLEXPORT int simScaleSelectedObjects(floatFloat scalingFactor,bool scalePositionsToo)
{
    return(simScaleSelectedObjects_internal((double)scalingFactor,scalePositionsToo));
}
SIM_DLLEXPORT int simGetJointMatrix(int objectHandle,floatFloat* matrix)
{
    double v[12];
    int retVal=simGetJointMatrix_internal(objectHandle,v);
    for (size_t i=0;i<12;i++)
        matrix[i]=(floatFloat)v[i];
    return(retVal);
}
SIM_DLLEXPORT int simSetSphericalJointMatrix(int objectHandle,const floatFloat* matrix)
{
    double v[12];
    for (size_t i=0;i<12;i++)
        v[i]=(double)matrix[i];
    return(simSetSphericalJointMatrix_internal(objectHandle,v));
}
SIM_DLLEXPORT int simSetScriptAttribute(int scriptHandle,int attributeID,floatFloat floatVal,int intOrBoolVal)
{
    return(simSetScriptAttribute_internal(scriptHandle,attributeID,(double)floatVal,intOrBoolVal));
}
SIM_DLLEXPORT int simGetScriptAttribute(int scriptHandle,int attributeID,floatFloat* floatVal,int* intOrBoolVal)
{
    double v;
    int retVal=simGetScriptAttribute_internal(scriptHandle,attributeID,v,intOrBoolVal);
    if (floatVal!=nullptr)
        floatVal[0]=(floatFloat)v;
    return(retVal);
}
SIM_DLLEXPORT int simGetJointMaxForce(int jointHandle,floatFloat* forceOrTorque)
{
    double v;
    int retVal=simGetJointMaxForce_internal(jointHandle,&v);
    forceOrTorque[0]=(floatFloat)v;
    return(retVal);
}
SIM_DLLEXPORT int simSetJointMaxForce(int objectHandle,floatFloat forceOrTorque)
{
    return(simSetJointMaxForce_internal(objectHandle,(double)forceOrTorque));
}
SIM_DLLEXPORT floatFloat* simGetVisionSensorImage(int visionSensorHandle)
{
    floatFloat retVal=nullptr;
    int handleFlags=visionSensorHandle&0xff00000;
    int sensorHandle=visionSensorHandle&0xfffff;
    CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
    if (it!=nullptr)
    {
        int res[2];
        it->getResolution(res);
        double* dta=simGetVisionSensorImage_internal(visionSensorHandle);
        int valPerPixel=3;
        if ((handleFlags&sim_handleflag_greyscale)!=0)
            valPerPixel=1;
        if (dta!=nullptr)
        {
            retVal=simCreateBuffer_internal(res[0]*res[1]*sizeof(floatFloat)*valPerPixel);
            for (int i=0;i<res[0]*res[1]*valPerPixel;i++)
                retVal[i]=(floatFloat)dta[i];
            simReleaseBuffer_internal((char*)dta);
        }
    }
    return(retVal);
}
SIM_DLLEXPORT int simSetVisionSensorImage(int visionSensorHandle,const floatFloat* image)
{
    int retVal=-1;
    int handleFlags=visionSensorHandle&0xff00000;
    int sensorHandle=visionSensorHandle&0xfffff;
    CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
    if (it!=nullptr)
    {
        int res[2];
        it->getResolution(res);
        int valPerPixel=3;
        if ((handleFlags&(sim_handleflag_greyscale+sim_handleflag_depthbuffer))!=0)
            valPerPixel=1;
        std::vector<double> img;
        img.resize(res[0]*res[1]*valPerPixel);
        for (int i=0;i<res[0]*res[1]*valPerPixel;i++)
            img[i]=(double)image[i];
        retVal=simSetVisionSensorImage_internal(visionSensorHandle,&img[0]));
    }
    return(retVal);
}
SIM_DLLEXPORT int simSetVisionSensorDepthBuffer(int visionSensorHandle,const floatFloat* depthBuffer)
{
    int retVal=-1;
    int handleFlags=visionSensorHandle&0xff00000;
    int sensorHandle=visionSensorHandle&0xfffff;
    CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
    if (it!=nullptr)
    {
        int res[2];
        it->getResolution(res);
        std::vector<double> img;
        img.resize(res[0]*res[1]);
        for (int i=0;i<res[0]*res[1];i++)
            img[i]=(double)depthBuffer[i];
        retVal=simSetVisionSensorDepthBuffer_internal(visionSensorHandle,&img[0]));
    }
    return(retVal);
}
SIM_DLLEXPORT int simCreatePureShape(int primitiveType,int options,const floatFloat* sizes,floatFloat mass,const int* precision)
{
    double s[3];
    for (size_t i=0;i<3;i++)
        s[i]=(double)sizes[i];
    return(simCreatePureShape_internal(primitiveType,options,s,(double)mass,precision));
}
SIM_DLLEXPORT int simBuildMatrixQ(const floatFloat* position,const floatFloat* quaternion,floatFloat* matrix)
{
    double p[3];
    for (size_t i=0;i<3;i++)
        p[i]=(double)position[i];
    double q[4];
    for (size_t i=0;i<4;i++)
        q[i]=(double)quaternion[i];
    double m[12];
    int retVal=simBuildMatrixQ_internal(p,q,m);
    for (size_t i=0;i<12;i++)
        matrix[i]=(floatFloat)m[i];
    return(retVal);
}
SIM_DLLEXPORT int simGetQuaternionFromMatrix(const floatFloat* matrix,floatFloat* quaternion)
{
    double m[12];
    for (size_t i=0;i<12;i++)
        m[i]=(double)matrix[i];
    double q[4];
    int retVal=simGetQuaternionFromMatrix_internal(m,q);
    for (size_t i=0;i<4;i++)
        quaternion[i]=(floatFloat)q[i];
    return(retVal);
}
SIM_DLLEXPORT int simGetShapeVertex(int shapeHandle,int groupElementIndex,int vertexIndex,floatFloat* relativePosition)
{
    return(-1);
}
SIM_DLLEXPORT int simGetShapeTriangle(int shapeHandle,int groupElementIndex,int triangleIndex,int* vertexIndices,floatFloat* triangleNormals)
{
    return(-1);
}


SIM_DLLEXPORT void _simGetJointOdeParameters(const void* joint,floatFloat* stopERP,floatFloat* stopCFM,floatFloat* bounce,floatFloat* fudge,floatFloat* normalCFM)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simGetJointBulletParameters(const void* joint,floatFloat* stopERP,floatFloat* stopCFM,floatFloat* normalCFM)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simGetOdeMaxContactFrictionCFMandERP(const void* geomInfo,int* maxContacts,floatFloat* friction,floatFloat* cfm,floatFloat* erp)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT bool _simGetBulletCollisionMargin(const void* geomInfo,floatFloat* margin,int* otherProp)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
    return(0);
}
SIM_DLLEXPORT floatFloat _simGetBulletRestitution(const void* geomInfo)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT void _simGetVortexParameters(const void* object,int version,floatFloat* floatParams,int* intParams)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simGetNewtonParameters(const void* object,int* version,floatFloat* floatParams,int* intParams)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simGetDamping(const void* geomInfo,floatFloat* linDamping,floatFloat* angDamping)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT floatFloat _simGetFriction(const void* geomInfo)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT void _simSetDynamicJointLocalTransformationPart2(void* joint,const floatFloat* pos,const floatFloat* quat)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simSetDynamicForceSensorLocalTransformationPart2(void* forceSensor,const floatFloat* pos,const floatFloat* quat)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simGetDynamicForceSensorLocalTransformationPart2(const void* forceSensor,floatFloat* pos,floatFloat* quat)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simGetMotorPid(const void* joint,floatFloat* pParam,floatFloat* iParam,floatFloat* dParam)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simGetPrincipalMomentOfInertia(const void* geomInfo,floatFloat* inertia)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT void _simGetLocalInertiaFrame(const void* geomInfo,floatFloat* pos,floatFloat* quat)
{ // not supported anymore in single-precision floatFloat mode, once we switch to double-precision floatFloat mode. Only physics plugins relied on those, and they will be rebuilt
}
//************************
#else
SIM_DLLEXPORT int simAddParticleObject(int objectType,floatFloat size,floatFloat density,const void* params,floatFloat lifeTime,int maxItemCount,const floatFloat* color,const floatFloat* setToNULL,const floatFloat* setToNULL2,const floatFloat* setToNULL3)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT int simAddParticleObjectItem(int objectHandle,const floatFloat* itemData)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT int simCreateMotionPlanning(int jointCnt,const int* jointHandles,const int* jointRangeSubdivisions,const floatFloat* jointMetricWeights,int options,const int* intParams,const floatFloat* floatParams,const void* reserved)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT int simJointGetForce(int jointHandle,floatFloat* forceOrTorque)
{
    return(simGetJointForce_internal(jointHandle,forceOrTorque));
}
SIM_DLLEXPORT int simGetMpConfigForTipPose(int motionPlanningObjectHandle,int options,floatFloat closeNodesDistance,int trialCount,const floatFloat* tipPose,int maxTimeInMs,floatFloat* outputJointPositions,const floatFloat* referenceConfigs,int referenceConfigCount,const floatFloat* jointWeights,const int* jointBehaviour,int correctionPasses)
{
    return(simGetMpConfigForTipPose_internal(motionPlanningObjectHandle,options,closeNodesDistance,trialCount,tipPose,maxTimeInMs,outputJointPositions,referenceConfigs,referenceConfigCount,jointWeights,jointBehaviour,correctionPasses));
}
SIM_DLLEXPORT floatFloat* simFindMpPath(int motionPlanningObjectHandle,const floatFloat* startConfig,const floatFloat* goalConfig,int options,floatFloat stepSize,int* outputConfigsCnt,int maxTimeInMs,floatFloat* reserved,const int* auxIntParams,const floatFloat* auxFloatParams)
{
    return(simFindMpPath_internal(motionPlanningObjectHandle,startConfig,goalConfig,options,stepSize,outputConfigsCnt,maxTimeInMs,reserved,auxIntParams,auxFloatParams));
}
SIM_DLLEXPORT floatFloat* simSimplifyMpPath(int motionPlanningObjectHandle,const floatFloat* pathBuffer,int configCnt,int options,floatFloat stepSize,int increment,int* outputConfigsCnt,int maxTimeInMs,floatFloat* reserved,const int* auxIntParams,const floatFloat* auxFloatParams)
{
    return(simSimplifyMpPath_internal(motionPlanningObjectHandle,pathBuffer,configCnt,options,stepSize,increment,outputConfigsCnt,maxTimeInMs,reserved,auxIntParams,auxFloatParams));
}
SIM_DLLEXPORT floatFloat* simFindIkPath(int motionPlanningObjectHandle,const floatFloat* startConfig,const floatFloat* goalPose,int options,floatFloat stepSize,int* outputConfigsCnt,floatFloat* reserved,const int* auxIntParams,const floatFloat* auxFloatParams)
{
    return(simFindIkPath_internal(motionPlanningObjectHandle,startConfig,goalPose,options,stepSize,outputConfigsCnt,reserved,auxIntParams,auxFloatParams));
}
SIM_DLLEXPORT floatFloat* simGetMpConfigTransition(int motionPlanningObjectHandle,const floatFloat* startConfig,const floatFloat* goalConfig,int options,const int* select,floatFloat calcStepSize,floatFloat maxOutStepSize,int wayPointCnt,const floatFloat* wayPoints,int* outputConfigsCnt,const int* auxIntParams,const floatFloat* auxFloatParams)
{
    return(simGetMpConfigTransition_internal(motionPlanningObjectHandle,startConfig,goalConfig,options,select,calcStepSize,maxOutStepSize,wayPointCnt,wayPoints,outputConfigsCnt,auxIntParams,auxFloatParams));
}
SIM_DLLEXPORT int simSearchPath(int pathPlanningObjectHandle,floatFloat maximumSearchTime)
{
    return(simSearchPath_internal(pathPlanningObjectHandle,maximumSearchTime));
}
SIM_DLLEXPORT int simInitializePathSearch(int pathPlanningObjectHandle,floatFloat maximumSearchTime,floatFloat searchTimeStep)
{
    return(simInitializePathSearch_internal(pathPlanningObjectHandle,maximumSearchTime,searchTimeStep));
}
SIM_DLLEXPORT int simHandlePath(int pathHandle,floatFloat deltaTime)
{
    return(simHandlePath_internal(pathHandle,deltaTime));
}
SIM_DLLEXPORT int simHandleJoint(int jointHandle,floatFloat deltaTime)
{
    return(simHandleJoint_internal(jointHandle,deltaTime));
}
SIM_DLLEXPORT int simSetUIButtonColor(int uiHandle,int buttonHandle,const floatFloat* upStateColor,const floatFloat* downStateColor,const floatFloat* labelColor)
{
    return(simSetUIButtonColor_internal(uiHandle,buttonHandle,upStateColor,downStateColor,labelColor));
}
SIM_DLLEXPORT int simSetUIButtonArrayColor(int uiHandle,int buttonHandle,const int* position,const floatFloat* color)
{
    return(simSetUIButtonArrayColor_internal(uiHandle,buttonHandle,position,color));
}
SIM_DLLEXPORT int simRegisterContactCallback(int(*callBack)(int,int,int,int*,floatFloat*))
{
    return(simRegisterContactCallback_internal(callBack));
}
SIM_DLLEXPORT int simRegisterJointCtrlCallback(int(*callBack)(int,int,int,const int*,const floatFloat*,floatFloat*))
{
    return(simRegisterJointCtrlCallback_internal(callBack));
}
SIM_DLLEXPORT int simSetJointForce(int objectHandle,floatFloat forceOrTorque)
{
    return(simSetJointMaxForce_internal(objectHandle,forceOrTorque));
}
SIM_DLLEXPORT int simHandleMill(int millHandle,floatFloat* removedSurfaceAndVolume)
{
    return(-1);
}
SIM_DLLEXPORT int simSetShapeMassAndInertia(int shapeHandle,floatFloat mass,const floatFloat* inertiaMatrix,const floatFloat* centerOfMass,const floatFloat* transformation)
{
    return(simSetShapeMassAndInertia_internal(shapeHandle,mass,inertiaMatrix,centerOfMass,transformation));
}
SIM_DLLEXPORT int simGetShapeMassAndInertia(int shapeHandle,floatFloat* mass,floatFloat* inertiaMatrix,floatFloat* centerOfMass,const floatFloat* transformation)
{
    return(simGetShapeMassAndInertia_internal(shapeHandle,mass,inertiaMatrix,centerOfMass,transformation));
}
SIM_DLLEXPORT int simCheckIkGroup(int ikGroupHandle,int jointCnt,const int* jointHandles,floatFloat* jointValues,const int* jointOptions)
{
    return(simCheckIkGroup_internal(ikGroupHandle,jointCnt,jointHandles,jointValues,jointOptions));
}
SIM_DLLEXPORT int simCreateIkGroup(int options,const int* intParams,const floatFloat* floatParams,const void* reserved)
{
    return(simCreateIkGroup_internal(options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT int simCreateIkElement(int ikGroupHandle,int options,const int* intParams,const floatFloat* floatParams,const void* reserved)
{
    return(simCreateIkElement_internal(ikGroupHandle,options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT int simGetConfigForTipPose(int ikGroupHandle,int jointCnt,const int* jointHandles,floatFloat thresholdDist,int maxTimeInMs,floatFloat* retConfig,const floatFloat* metric,int collisionPairCnt,const int* collisionPairs,const int* jointOptions,const floatFloat* lowLimits,const floatFloat* ranges,void* reserved)
{
    return(simGetConfigForTipPose_internal(ikGroupHandle,jointCnt,jointHandles,thresholdDist,maxTimeInMs,retConfig,metric,collisionPairCnt,collisionPairs,jointOptions,lowLimits,ranges,reserved));
}
SIM_DLLEXPORT floatFloat* simGenerateIkPath(int ikGroupHandle,int jointCnt,const int* jointHandles,int ptCnt,int collisionPairCnt,const int* collisionPairs,const int* jointOptions,void* reserved)
{
    return(simGenerateIkPath_internal(ikGroupHandle,jointCnt,jointHandles,ptCnt,collisionPairCnt,collisionPairs,jointOptions,reserved));
}
SIM_DLLEXPORT floatFloat* simGetIkGroupMatrix(int ikGroupHandle,int options,int* matrixSize)
{
    return(simGetIkGroupMatrix_internal(ikGroupHandle,options,matrixSize));
}
SIM_DLLEXPORT int simSetIkGroupProperties(int ikGroupHandle,int resolutionMethod,int maxIterations,floatFloat damping,void* reserved)
{
    return(simSetIkGroupProperties_internal(ikGroupHandle,resolutionMethod,maxIterations,damping,reserved));
}
SIM_DLLEXPORT int simGetPositionOnPath(int pathHandle,floatFloat relativeDistance,floatFloat* position)
{
    return(simGetPositionOnPath_internal(pathHandle,relativeDistance,position));
}
SIM_DLLEXPORT int simGetOrientationOnPath(int pathHandle,floatFloat relativeDistance,floatFloat* eulerAngles)
{
    return(simGetOrientationOnPath_internal(pathHandle,relativeDistance,eulerAngles));
}
SIM_DLLEXPORT int simGetDataOnPath(int pathHandle,floatFloat relativeDistance,int dataType,int* intData,floatFloat* floatData)
{
    return(simGetDataOnPath_internal(pathHandle,relativeDistance,dataType,intData,floatData));
}
SIM_DLLEXPORT int simGetClosestPositionOnPath(int pathHandle,floatFloat* absolutePosition,floatFloat* pathPosition)
{
    return(simGetClosestPositionOnPath_internal(pathHandle,absolutePosition,pathPosition));
}
SIM_DLLEXPORT int simGetPathPosition(int objectHandle,floatFloat* position)
{
    return(simGetPathPosition_internal(objectHandle,position));
}
SIM_DLLEXPORT int simSetPathPosition(int objectHandle,floatFloat position)
{
    return(simSetPathPosition_internal(objectHandle,position));
}
SIM_DLLEXPORT int simGetPathLength(int objectHandle,floatFloat* length)
{
    return(simGetPathLength_internal(objectHandle,length));
}
SIM_DLLEXPORT int simCreatePath(int attributes,const int* intParams,const floatFloat* floatParams,const floatFloat* color)
{
    return(simCreatePath_internal(attributes,intParams,floatParams,color));
}
SIM_DLLEXPORT int simSetIkElementProperties(int ikGroupHandle,int tipDummyHandle,int constraints,const floatFloat* precision,const floatFloat* weight,void* reserved)
{
    return(simSetIkElementProperties_internal(ikGroupHandle,tipDummyHandle,constraints,precision,weight,reserved));
}
SIM_DLLEXPORT int simSetVisionSensorFilter(int visionSensorHandle,int filterIndex,int options,const int* pSizes,const unsigned char* bytes,const int* ints,const floatFloat* floats,const unsigned char* custom)
{
    return(simSetVisionSensorFilter_internal(visionSensorHandle,filterIndex,options,pSizes,bytes,ints,floats,custom));
}
SIM_DLLEXPORT int simGetVisionSensorFilter(int visionSensorHandle,int filterIndex,int* options,int* pSizes,unsigned char** bytes,int** ints,floatFloat** floats,unsigned char** custom)
{
    return(simGetVisionSensorFilter_internal(visionSensorHandle,filterIndex,options,pSizes,bytes,ints,floats,custom));
}
SIM_DLLEXPORT int simSetPathTargetNominalVelocity(int objectHandle,floatFloat targetNominalVelocity)
{
    return(simSetPathTargetNominalVelocity_internal(objectHandle,targetNominalVelocity));
}
SIM_DLLEXPORT int simSendData(int targetID,int dataHeader,const char* dataName,const char* data,int dataLength,int antennaHandle,floatFloat actionRadius,floatFloat emissionAngle1,floatFloat emissionAngle2,floatFloat persistence)
{
    return(simSendData_internal(targetID,dataHeader,dataName,data,dataLength,antennaHandle,actionRadius,emissionAngle1,emissionAngle2,persistence));
}
SIM_DLLEXPORT int simHandleDistance(int distanceObjectHandle,floatFloat* smallestDistance)
{
    return(simHandleDistance_internal(distanceObjectHandle,smallestDistance));
}
SIM_DLLEXPORT int simReadDistance(int distanceObjectHandle,floatFloat* smallestDistance)
{
    return(simReadDistance_internal(distanceObjectHandle,smallestDistance));
}
SIM_DLLEXPORT int simAddBanner(const char* label,floatFloat size,int options,const floatFloat* positionAndEulerAngles,int parentObjectHandle,const floatFloat* labelColors,const floatFloat* backgroundColors)
{
    return(simAddBanner_internal(label,size,options,positionAndEulerAngles,parentObjectHandle,labelColors,backgroundColors));
}
SIM_DLLEXPORT int simAddGhost(int ghostGroup,int objectHandle,int options,floatFloat startTime,floatFloat endTime,const floatFloat* color)
{
    return(simAddGhost_internal(ghostGroup,objectHandle,options,startTime,endTime,color));
}
SIM_DLLEXPORT int simModifyGhost(int ghostGroup,int ghostId,int operation,floatFloat floatValue,int options,int optionsMask,const floatFloat* colorOrTransformation)
{
    return(simModifyGhost_internal(ghostGroup,ghostId,operation,floatValue,options,optionsMask,colorOrTransformation));
}
SIM_DLLEXPORT int simSetGraphUserData(int graphHandle,const char* streamName,floatFloat data)
{
    return(simSetGraphUserData_internal(graphHandle,streamName,data));
}
SIM_DLLEXPORT int simAddPointCloud(int pageMask,int layerMask,int objectHandle,int options,floatFloat pointSize,int ptCnt,const floatFloat* pointCoordinates,const char* defaultColors,const char* pointColors,const floatFloat* pointNormals)
{
    return(simAddPointCloud_internal(pageMask,layerMask,objectHandle,options,pointSize,ptCnt,pointCoordinates,defaultColors,pointColors,pointNormals));
}
SIM_DLLEXPORT int simModifyPointCloud(int pointCloudHandle,int operation,const int* intParam,const floatFloat* floatParam)
{
    return(simModifyPointCloud_internal(pointCloudHandle,operation,intParam,floatParam));
}
SIM_DLLEXPORT int simCopyMatrix(const floatFloat* matrixIn,floatFloat* matrixOut)
{
    return(simCopyMatrix_internal(matrixIn,matrixOut));
}
SIM_DLLEXPORT int simGetObjectFloatParameter(int objectHandle,int ParamID,floatFloat* Param)
{
    return(simGetObjectFloatParam_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT int simSetObjectFloatParameter(int objectHandle,int ParamID,floatFloat Param)
{
    return(simSetObjectFloatParam_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT int simSetFloatingParameter(int parameter,floatFloat floatState)
{
    return(simSetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT int simGetFloatingParameter(int parameter,floatFloat* floatState)
{
    return(simGetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT int simSetFloatParameter(int parameter,floatFloat floatState)
{
    return(simSetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT int simGetFloatParameter(int parameter,floatFloat* floatState)
{
    return(simGetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT floatFloat simGetEngineFloatParameter(int paramId,int objectHandle,const void* object,bool* ok)
{
    return(simGetEngineFloatParam_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT int simSetEngineFloatParameter(int paramId,int objectHandle,const void* object,floatFloat val)
{
    return(simSetEngineFloatParam_internal(paramId,objectHandle,object,val));
}
SIM_DLLEXPORT int simSetObjectSizeValues(int objectHandle,const floatFloat* sizeValues)
{
    return(simSetObjectSizeValues_internal(objectHandle,sizeValues));
}
SIM_DLLEXPORT int simGetObjectSizeValues(int objectHandle,floatFloat* sizeValues)
{
    return(simGetObjectSizeValues_internal(objectHandle,sizeValues));
}
SIM_DLLEXPORT int simDisplayDialog(const char* titleText,const char* mainText,int dialogType,const char* initialText,const floatFloat* titleColors,const floatFloat* dialogColors,int* elementHandle)
{
    return(simDisplayDialog_internal(titleText,mainText,dialogType,initialText,titleColors,dialogColors,elementHandle));
}
SIM_DLLEXPORT int simScaleSelectedObjects(floatFloat scalingFactor,bool scalePositionsToo)
{
    return(simScaleSelectedObjects_internal(scalingFactor,scalePositionsToo));
}
SIM_DLLEXPORT int simGetJointMatrix(int objectHandle,floatFloat* matrix)
{
    return(simGetJointMatrix_internal(objectHandle,matrix));
}
SIM_DLLEXPORT int simSetSphericalJointMatrix(int objectHandle,const floatFloat* matrix)
{
    return(simSetSphericalJointMatrix_internal(objectHandle,matrix));
}
SIM_DLLEXPORT int simSetScriptAttribute(int scriptHandle,int attributeID,floatFloat floatVal,int intOrBoolVal)
{
    return(simSetScriptAttribute_internal(scriptHandle,attributeID,floatVal,intOrBoolVal));
}
SIM_DLLEXPORT int simGetScriptAttribute(int scriptHandle,int attributeID,floatFloat* floatVal,int* intOrBoolVal)
{
    return(simGetScriptAttribute_internal(scriptHandle,attributeID,floatVal,intOrBoolVal));
}
SIM_DLLEXPORT int simGetJointMaxForce(int jointHandle,floatFloat* forceOrTorque)
{
    return(simGetJointMaxForce_internal(jointHandle,forceOrTorque));
}
SIM_DLLEXPORT int simSetJointMaxForce(int objectHandle,floatFloat forceOrTorque)
{
    return(simSetJointMaxForce_internal(objectHandle,forceOrTorque));
}
SIM_DLLEXPORT floatFloat* simGetVisionSensorImage(int visionSensorHandle)
{
    return(simGetVisionSensorImage_internal(visionSensorHandle));
}
SIM_DLLEXPORT int simSetVisionSensorImage(int visionSensorHandle,const floatFloat* image)
{
    return(simSetVisionSensorImage_internal(visionSensorHandle,image));
}
SIM_DLLEXPORT int simSetVisionSensorDepthBuffer(int visionSensorHandle,const floatFloat* depthBuffer)
{
    return(-1);
}
SIM_DLLEXPORT int simCreatePureShape(int primitiveType,int options,const floatFloat* sizes,floatFloat mass,const int* precision)
{
    return(simCreatePureShape_internal(primitiveType,options,sizes,mass,precision));
}
SIM_DLLEXPORT int simBuildMatrixQ(const floatFloat* position,const floatFloat* quaternion,floatFloat* matrix)
{
    return(simBuildMatrixQ_internal(position,quaternion,matrix));
}
SIM_DLLEXPORT int simGetQuaternionFromMatrix(const floatFloat* matrix,floatFloat* quaternion)
{
    return(simGetQuaternionFromMatrix_internal(matrix,quaternion));
}
SIM_DLLEXPORT int simGetShapeVertex(int shapeHandle,int groupElementIndex,int vertexIndex,floatFloat* relativePosition)
{
    return(simGetShapeVertex_internal(shapeHandle,groupElementIndex,vertexIndex,relativePosition));
}
SIM_DLLEXPORT int simGetShapeTriangle(int shapeHandle,int groupElementIndex,int triangleIndex,int* vertexIndices,floatFloat* triangleNormals)
{
    return(simGetShapeTriangle_internal(shapeHandle,groupElementIndex,triangleIndex,vertexIndices,triangleNormals));
}




SIM_DLLEXPORT void _simGetJointOdeParameters(const void* joint,floatFloat* stopERP,floatFloat* stopCFM,floatFloat* bounce,floatFloat* fudge,floatFloat* normalCFM)
{
    return(_simGetJointOdeParameters_internal(joint,stopERP,stopCFM,bounce,fudge,normalCFM));
}
SIM_DLLEXPORT void _simGetJointBulletParameters(const void* joint,floatFloat* stopERP,floatFloat* stopCFM,floatFloat* normalCFM)
{
    return(_simGetJointBulletParameters_internal(joint,stopERP,stopCFM,normalCFM));
}
SIM_DLLEXPORT void _simGetOdeMaxContactFrictionCFMandERP(const void* geomInfo,int* maxContacts,floatFloat* friction,floatFloat* cfm,floatFloat* erp)
{
    return(_simGetOdeMaxContactFrictionCFMandERP_internal(geomInfo,maxContacts,friction,cfm,erp));
}
SIM_DLLEXPORT bool _simGetBulletCollisionMargin(const void* geomInfo,floatFloat* margin,int* otherProp)
{
    return(_simGetBulletCollisionMargin_internal(geomInfo,margin,otherProp));
}
SIM_DLLEXPORT floatFloat _simGetBulletRestitution(const void* geomInfo)
{
    return(_simGetBulletRestitution_internal(geomInfo));
}
SIM_DLLEXPORT void _simGetVortexParameters(const void* object,int version,floatFloat* floatParams,int* intParams)
{
    _simGetVortexParameters_internal(object,version,floatParams,intParams);
}
SIM_DLLEXPORT void _simGetNewtonParameters(const void* object,int* version,floatFloat* floatParams,int* intParams)
{
    _simGetNewtonParameters_internal(object,version,floatParams,intParams);
}
SIM_DLLEXPORT void _simGetDamping(const void* geomInfo,floatFloat* linDamping,floatFloat* angDamping)
{
    return(_simGetDamping_internal(geomInfo,linDamping,angDamping));
}
SIM_DLLEXPORT floatFloat _simGetFriction(const void* geomInfo)
{
    return(_simGetFriction_internal(geomInfo));
}
SIM_DLLEXPORT void _simSetDynamicJointLocalTransformationPart2(void* joint,const floatFloat* pos,const floatFloat* quat)
{
    return(_simSetDynamicJointLocalTransformationPart2_internal(joint,pos,quat));
}
SIM_DLLEXPORT void _simSetDynamicForceSensorLocalTransformationPart2(void* forceSensor,const floatFloat* pos,const floatFloat* quat)
{
    return(_simSetDynamicForceSensorLocalTransformationPart2_internal(forceSensor,pos,quat));
}
SIM_DLLEXPORT void _simGetDynamicForceSensorLocalTransformationPart2(const void* forceSensor,floatFloat* pos,floatFloat* quat)
{
    return(_simGetDynamicForceSensorLocalTransformationPart2_internal(forceSensor,pos,quat));
}
SIM_DLLEXPORT void _simGetMotorPid(const void* joint,floatFloat* pParam,floatFloat* iParam,floatFloat* dParam)
{
}
SIM_DLLEXPORT void _simGetPrincipalMomentOfInertia(const void* geomInfo,floatFloat* inertia)
{
    return(_simGetPrincipalMomentOfInertia_internal(geomInfo,inertia));
}
SIM_DLLEXPORT void _simGetLocalInertiaFrame(const void* geomInfo,floatFloat* pos,floatFloat* quat)
{
    return(_simGetLocalInertiaFrame_internal(geomInfo,pos,quat));
}
#endif
