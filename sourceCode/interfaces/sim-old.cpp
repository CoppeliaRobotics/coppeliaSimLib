#ifdef switchToDouble
// single precision, non-deprecated:
SIM_DLLEXPORT simInt simSetFloatParam(simInt parameter,simFloat floatState)
{
    return(simSetFloatParam_internal(parameter,(double)floatState));
}
SIM_DLLEXPORT simInt simGetFloatParam(simInt parameter,simFloat* floatState)
{
    double v;
    int retVal=simGetFloatParam_internal(parameter,&v);
    floatState[0]=(float)v;
    return(retVal);
}
SIM_DLLEXPORT simInt simGetObjectMatrix(simInt objectHandle,simInt relativeToObjectHandle,simFloat* matrix)
{
    double m[12];
    int retVal=simGetObjectMatrix_internal(objectHandle,relativeToObjectHandle,m);
    for (size_t i=0;i<12;i++)
        matrix[i]=(float)m[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simSetObjectMatrix(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* matrix)
{
    double m[12];
    for (size_t i=0;i<12;i++)
        m[i]=(double)matrix[i];
    return(simSetObjectMatrix_internal(objectHandle,relativeToObjectHandle,m));
}
SIM_DLLEXPORT simInt simGetObjectPose(simInt objectHandle,simInt relativeToObjectHandle,simFloat* pose)
{
    double p[7];
    int retVal=simGetObjectPose_internal(objectHandle,relativeToObjectHandle,p);
    for (size_t i=0;i<7;i++)
        pose[i]=(float)p[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simSetObjectPose(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* pose)
{
    double p[7];
    for (size_t i=0;i<7;i++)
        p[i]=(double)pose[i];
    return(simSetObjectPose_internal(objectHandle,relativeToObjectHandle,p));
}
SIM_DLLEXPORT simInt simGetObjectPosition(simInt objectHandle,simInt relativeToObjectHandle,simFloat* position)
{
    double p[3];
    int retVal=simGetObjectPosition_internal(objectHandle,relativeToObjectHandle,p);
    for (size_t i=0;i<3;i++)
        position[i]=(float)p[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simSetObjectPosition(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* position)
{
    double p[3];
    for (size_t i=0;i<3;i++)
        p[i]=(double)position[i];
    return(simSetObjectPosition_internal(objectHandle,relativeToObjectHandle,p));
}
SIM_DLLEXPORT simInt simGetObjectOrientation(simInt objectHandle,simInt relativeToObjectHandle,simFloat* eulerAngles)
{
    double e[3];
    int retVal=simGetObjectOrientation_internal(objectHandle,relativeToObjectHandle,e);
    for (size_t i=0;i<3;i++)
        eulerAngles[i]=(float)e[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simGetObjectQuaternion(simInt objectHandle,simInt relativeToObjectHandle,simFloat* quaternion)
{
    double q[4];
    int retVal=simGetObjectQuaternion_internal(objectHandle,relativeToObjectHandle,q);
    for (size_t i=0;i<4;i++)
        quaternion[i]=(float)q[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simSetObjectQuaternion(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* quaternion)
{
    double q[4];
    for (size_t i=0;i<4;i++)
        q[i]=(double)quaternion[i];
    return(simSetObjectQuaternion_internal(objectHandle,relativeToObjectHandle,q));
}
SIM_DLLEXPORT simInt simSetObjectOrientation(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* eulerAngles)
{
    double e[3];
    for (size_t i=0;i<3;i++)
        e[i]=(double)eulerAngles[i];
    return(simSetObjectOrientation_internal(objectHandle,relativeToObjectHandle,e));
}
SIM_DLLEXPORT simInt simGetJointPosition(simInt objectHandle,simFloat* position)
{
    double p[3];
    int retVal=simGetJointPosition_internal(objectHandle,p);
    for (size_t i=0;i<3;i++)
        position[i]=(float)p[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simSetJointPosition(simInt objectHandle,simFloat position)
{
    return(simSetJointPosition_internal(objectHandle,(double)position));
}
SIM_DLLEXPORT simInt simSetJointTargetPosition(simInt objectHandle,simFloat targetPosition)
{
    return(simSetJointTargetPosition_internal(objectHandle,(double)targetPosition));
}
SIM_DLLEXPORT simInt simGetJointTargetPosition(simInt objectHandle,simFloat* targetPosition)
{
    double t;
    int retVal=simGetJointTargetPosition_internal(objectHandle,&t);
    targetPosition[0]=(float)t;
    return(retVal);
}
SIM_DLLEXPORT simInt simGetObjectChildPose(simInt objectHandle,simFloat* pose)
{
    double p[7];
    int retVal=simGetObjectChildPose_internal(objectHandle,p);
    for (size_t i=0;i<7;i++)
        pose[i]=(float)p[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simSetObjectChildPose(simInt objectHandle,const simFloat* pose)
{
    double p[7];
    for (size_t i=0;i<7;i++)
        p[i]=(double)pose[i];
    return(simSetObjectChildPose_internal(objectHandle,p));
}
SIM_DLLEXPORT simInt simGetJointInterval(simInt objectHandle,simBool* cyclic,simFloat* interval)
{
    double interv[2];
    int retVal=simGetJointInterval_internal(objectHandle,cyclic,interv);
    for (size_t i=0;i<2;i++)
        interval[i]=(float)interv[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simSetJointInterval(simInt objectHandle,simBool cyclic,const simFloat* interval)
{
    double interv[2];
    for (size_t i=0;i<2;i++)
        interv[i]=(double)interval[i];
    return(simSetJointInterval_internal(objectHandle,cyclic,interv));
}
SIM_DLLEXPORT simInt simBuildIdentityMatrix(simFloat* matrix)
{
    double m[12];
    int retVal=simBuildIdentityMatrix_internal(m);
    for (size_t i=0;i<12;i++)
        matrix[i]=(float)m[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simBuildMatrix(const simFloat* position,const simFloat* eulerAngles,simFloat* matrix)
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
        matrix[i]=(float)m[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simBuildPose(const simFloat* position,const simFloat* eulerAngles,simFloat* pose)
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
        pose[i]=(float)p[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simGetEulerAnglesFromMatrix(const simFloat* matrix,simFloat* eulerAngles)
{
    double m[12];
    for (size_t i=0;i<12;i++)
        m[i]=(double)matrix[i];
    double e[3];
    int retVal=simGetEulerAnglesFromMatrix_internal(m,e);
    for (size_t i=0;i<3;i++)
        eulerAngles[i]=(float)e[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simInvertMatrix(simFloat* matrix)
{
    double m[12];
    for (size_t i=0;i<12;i++)
        m[i]=(double)matrix[i];
    int retVal=simInvertMatrix_internal(m);
    for (size_t i=0;i<12;i++)
        matrix[i]=(float)m[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simMultiplyMatrices(const simFloat* matrixIn1,const simFloat* matrixIn2,simFloat* matrixOut)
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
        matrixOut[i]=(float)m[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simMultiplyPoses(const simFloat* poseIn1,const simFloat* poseIn2,simFloat* poseOut)
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
        poseOut[i]=(float)p[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simInvertPose(simFloat* pose)
{
    double p[7];
    for (size_t i=0;i<7;i++)
        p[i]=(double)pose[i];
    int retVal=simInvertPose_internal(p);
    for (size_t i=0;i<7;i++)
        pose[i]=(float)p[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simInterpolatePoses(const simFloat* poseIn1,const simFloat* poseIn2,simFloat interpolFactor,simFloat* poseOut)
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
        poseOut[i]=(float)p[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simPoseToMatrix(const simFloat* poseIn,simFloat* matrixOut)
{
    double p[7];
    for (size_t i=0;i<7;i++)
        p[i]=(double)poseIn[i];
    double m[12];
    int retVal=simPoseToMatrix_internal(p,m);
    for (size_t i=0;i<12;i++)
        matrixOut[i]=(float)m[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simMatrixToPose(const simFloat* matrixIn,simFloat* poseOut)
{
    double m[12];
    for (size_t i=0;i<12;i++)
        m[i]=(double)matrixIn[i];
    double p[7];
    int retVal=simMatrixToPose_internal(m,p);
    for (size_t i=0;i<7;i++)
        poseOut[i]=(float)p[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simInterpolateMatrices(const simFloat* matrixIn1,const simFloat* matrixIn2,simFloat interpolFactor,simFloat* matrixOut)
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
        matrixOut[i]=(float)m[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simTransformVector(const simFloat* matrix,simFloat* vect)
{
    double m[12];
    for (size_t i=0;i<12;i++)
        m[i]=(double)matrix[i];
    double v[3];
    for (size_t i=0;i<3;i++)
        v[i]=(double)vect[i];
    int retVal=simTransformVector_internal(m,v);
    for (size_t i=0;i<3;i++)
        vect[i]=(float)v[i];
    return(retVal);
}
SIM_DLLEXPORT simFloat simGetSimulationTime()
{
    return((float)simGetSimulationTime_internal());
}
SIM_DLLEXPORT simFloat simGetSystemTime()
{
    return(float(simGetSystemTime_internal()));
}
SIM_DLLEXPORT simInt simHandleProximitySensor(simInt sensorHandle,simFloat* detectedPoint,simInt* detectedObjectHandle,simFloat* normalVector)
{
    double pt[3];
    double n[3];
    int retVal=simHandleProximitySensor_internal(sensorHandle,pt,detectedObjectHandle,n);
    if (retVal>0)
    {
        for (size_t i=0;i<3;i++)
        {
            if (detectedPoint!=nullptr)
                detectedPoint[i]=(float)pt[i];
            if (normalVector!=nullptr)
                normalVector[i]=(float)n[i];
        }
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simReadProximitySensor(simInt sensorHandle,simFloat* detectedPoint,simInt* detectedObjectHandle,simFloat* normalVector)
{
    double pt[3];
    double n[3];
    int retVal=simReadProximitySensor_internal(sensorHandle,pt,detectedObjectHandle,n);
    if (retVal>0)
    {
        for (size_t i=0;i<3;i++)
        {
            if (detectedPoint!=nullptr)
                detectedPoint[i]=(float)pt[i];
            if (normalVector!=nullptr)
                normalVector[i]=(float)n[i];
        }
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simHandleDynamics(simFloat deltaTime)
{
    return(simHandleDynamics_internal((double)deltaTime));
}
SIM_DLLEXPORT simInt simCheckProximitySensor(simInt sensorHandle,simInt entityHandle,simFloat* detectedPoint)
{
    double pt[3];
    int retVal=simCheckProximitySensor_internal(sensorHandle,entityHandle,pt);
    if ( (retVal>0)&&(detectedPoint!=nullptr) )
    {
        for (size_t i=0;i<3;i++)
            detectedPoint[i]=(float)pt[i];
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simCheckProximitySensorEx(simInt sensorHandle,simInt entityHandle,simInt detectionMode,simFloat detectionThreshold,simFloat maxAngle,simFloat* detectedPoint,simInt* detectedObjectHandle,simFloat* normalVector)
{
    double pt[3];
    double n[3];
    int retVal=simCheckProximitySensorEx_internal(sensorHandle,entityHandle,detectionMode,(double)detectionThreshold,(double)maxAngle,pt,detectedObjectHandle,n);
    if (retVal>0)
    {
        for (size_t i=0;i<3;i++)
        {
            if (detectedPoint!=nullptr)
                detectedPoint[i]=(float)pt[i];
            if (normalVector!=nullptr)
                normalVector[i]=(float)n[i];
        }
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simCheckProximitySensorEx2(simInt sensorHandle,simFloat* vertexPointer,simInt itemType,simInt itemCount,simInt detectionMode,simFloat detectionThreshold,simFloat maxAngle,simFloat* detectedPoint,simFloat* normalVector)
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
                detectedPoint[i]=(float)pt[i];
            if (normalVector!=nullptr)
                normalVector[i]=(float)n[i];
        }
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simCheckCollisionEx(simInt entity1Handle,simInt entity2Handle,simFloat** intersectionSegments)
{
    double* s;
    int retVal=simCheckCollisionEx_internal(entity1Handle,entity2Handle,&s);
    if (retVal>0)
    {
        intersectionSegments[0]=(float*)simCreateBuffer_internal(sizeof(float)*retVal*6);
        for (int i=0;i<retVal*6;i++)
            intersectionSegments[0][i]=(float)s[i];
        simReleaseBuffer_internal((simChar*)s);
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simCheckDistance(simInt entity1Handle,simInt entity2Handle,simFloat threshold,simFloat* distanceData)
{
    double d[7];
    int retVal=simCheckDistance_internal(entity1Handle,entity2Handle,(double)threshold,d);
    if (retVal>0)
    {
        for (size_t i=0;i<7;i++)
            distanceData[i]=(float)d[i];
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simSetSimulationTimeStep(simFloat timeStep)
{
    return(simSetSimulationTimeStep_internal((double)timeStep));
}
SIM_DLLEXPORT simFloat simGetSimulationTimeStep()
{
    return((double)simGetSimulationTimeStep_internal());
}
SIM_DLLEXPORT simInt simAdjustRealTimeTimer(simInt instanceIndex,simFloat deltaTime)
{
    return(simAdjustRealTimeTimer_internal(instanceIndex,(double)deltaTime));
}
SIM_DLLEXPORT simInt simFloatingViewAdd(simFloat posX,simFloat posY,simFloat sizeX,simFloat sizeY,simInt options)
{
    return(simFloatingViewAdd_internal((double)posX,(double)posY,(double)sizeX,(double)sizeY,options));
}
SIM_DLLEXPORT simInt simHandleGraph(simInt graphHandle,simFloat simulationTime)
{
    return(simHandleGraph_internal(graphHandle,(double)simulationTime));
}
SIM_DLLEXPORT simInt simAddGraphStream(simInt graphHandle,const simChar* streamName,const simChar* unitStr,simInt options,const simFloat* color,simFloat cyclicRange)
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
SIM_DLLEXPORT simInt simSetGraphStreamTransformation(simInt graphHandle,simInt streamId,simInt trType,simFloat mult,simFloat off,simInt movingAvgPeriod)
{
    return(simSetGraphStreamTransformation_internal(graphHandle,streamId,trType,(double)mult,(double)off,movingAvgPeriod));
}
SIM_DLLEXPORT simInt simAddGraphCurve(simInt graphHandle,const simChar* curveName,simInt dim,const simInt* streamIds,const simFloat* defaultValues,const simChar* unitStr,simInt options,const simFloat* color,simInt curveWidth)
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
SIM_DLLEXPORT simInt simSetGraphStreamValue(simInt graphHandle,simInt streamId,simFloat value)
{
    return(simSetGraphStreamValue_internal(graphHandle,streamId,(double)value));
}
SIM_DLLEXPORT simInt simSetJointTargetVelocity(simInt objectHandle,simFloat targetVelocity)
{
    return(simSetJointTargetVelocity_internal(objectHandle,(double)targetVelocity));
}
SIM_DLLEXPORT simInt simGetJointTargetVelocity(simInt objectHandle,simFloat* targetVelocity)
{
    return(simGetJointTargetVelocity_internal(objectHandle,(double)targetVelocity));
}
SIM_DLLEXPORT simInt simScaleObjects(const simInt* objectHandles,simInt objectCount,simFloat scalingFactor,simBool scalePositionsToo)
{
    return(simScaleObjects_internal(objectHandles,objectCount,(double)scalingFactor,scalePositionsToo));
}
SIM_DLLEXPORT simInt simAddDrawingObject(simInt objectType,simFloat size,simFloat duplicateTolerance,simInt parentObjectHandle,simInt maxItemCount,const simFloat* color,const simFloat* setToNULL,const simFloat* setToNULL2,const simFloat* setToNULL3)
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
SIM_DLLEXPORT simInt simAddDrawingObjectItem(simInt objectHandle,const simFloat* itemData)
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
SIM_DLLEXPORT simFloat simGetObjectSizeFactor(simInt objectHandle)
{
    return((float)simGetObjectSizeFactor_internal(objectHandle));
}
SIM_DLLEXPORT simInt simSetFloatSignal(const simChar* signalName,simFloat signalValue)
{
    return(simSetFloatSignal_internal(signalName,(double)signalValue));
}
SIM_DLLEXPORT simInt simGetFloatSignal(const simChar* signalName,simFloat* signalValue)
{
    double v;
    int retVal=simGetFloatSignal_internal(signalName,&v);
    signalValue[0]=(float)v;
    return(retVal);
}
SIM_DLLEXPORT simInt simReadForceSensor(simInt objectHandle,simFloat* forceVector,simFloat* torqueVector)
{
    double f[3];
    double t[3];
    int retVal=simReadForceSensor_internal(objectHandle,f,t);
    if ((retVal&1)!=0)
    {
        for (size_t i=0;i<3;i++)
        {
            if (forceVector!=nullptr)
                forceVector[i]=(float)f[i];
            if (torqueVector!=nullptr)
                torqueVector[i]=(float)t[i];
        }
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simSetLightParameters(simInt objectHandle,simInt state,const simFloat* setToNULL,const simFloat* diffusePart,const simFloat* specularPart)
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
SIM_DLLEXPORT simInt simGetLightParameters(simInt objectHandle,simFloat* setToNULL,simFloat* diffusePart,simFloat* specularPart)
{
    double b[3];
    double c[3];
    int retVal=simGetLightParameters_internal(objectHandle,nullptr,b,c);
    if (retVal!=-1)
    {
        for (size_t i=0;i<3;i++)
        {
            if (diffusePart!=nullptr)
                diffusePart[i]=(float)b[i];
            if (specularPart!=nullptr)
                specularPart[i]=(float)c[i];
        }
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simGetVelocity(simInt shapeHandle,simFloat* linearVelocity,simFloat* angularVelocity)
{
    double lv[3];
    double av[3];
    int retVal=simGetVelocity_internal(shapeHandle,lv,av);
    if (retVal!=-1)
    {
        for (size_t i=0;i<3;i++)
        {
            if (linearVelocity!=nullptr)
                linearVelocity[i]=(float)lv[i];
            if (angularVelocity!=nullptr)
                angularVelocity[i]=(float)av[i];
        }
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simGetObjectVelocity(simInt objectHandle,simFloat* linearVelocity,simFloat* angularVelocity)
{
    double lv[3];
    double av[3];
    int retVal=simGetObjectVelocity_internal(objectHandle,lv,av);
    if (retVal!=-1)
    {
        for (size_t i=0;i<3;i++)
        {
            if (linearVelocity!=nullptr)
                linearVelocity[i]=(float)lv[i];
            if (angularVelocity!=nullptr)
                angularVelocity[i]=(float)av[i];
        }
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simGetJointVelocity(simInt jointHandle,simFloat* velocity)
{
    double v;
    int retVal=simGetJointVelocity_internal(jointHandle,&v);
    velocity[0]=(float)v;
    return(retVal);
}
SIM_DLLEXPORT simInt simAddForceAndTorque(simInt shapeHandle,const simFloat* force,const simFloat* torque)
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
SIM_DLLEXPORT simInt simAddForce(simInt shapeHandle,const simFloat* position,const simFloat* force)
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
SIM_DLLEXPORT simInt simSetObjectColor(simInt objectHandle,simInt index,simInt colorComponent,const simFloat* rgbData)
{
    double c[3];
    for (size_t i=0;i<3;i++)
        c[i]=(double)rgbData[i];
    return(simSetObjectColor_internal(objectHandle,index,colorComponent,c));
}
SIM_DLLEXPORT simInt simGetObjectColor(simInt objectHandle,simInt index,simInt colorComponent,simFloat* rgbData)
{
    double c[3];
    int retVal=simGetObjectColor_internal(objectHandle,index,colorComponent,c);
    if (retVal>0)
    {
        for (size_t i=0;i<3;i++)
            rgbData[i]=(float)c[i];
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simSetShapeColor(simInt shapeHandle,const simChar* colorName,simInt colorComponent,const simFloat* rgbData)
{
    double c[3];
    size_t cnt=3;
    if (colorComponent==sim_colorcomponent_transparency)
        cnt=1;
    for (size_t i=0;i<cnt;i++)
        c[i]=(double)rgbData[i];
    return(simSetShapeColor_internal(shapeHandle,colorName,colorComponent,c));
}
SIM_DLLEXPORT simInt simGetShapeColor(simInt shapeHandle,const simChar* colorName,simInt colorComponent,simFloat* rgbData)
{
    double c[3];
    int retVal=simGetShapeColor_internal(shapeHandle,colorName,colorComponent,c);
    if (retVal>0)
    {
        size_t cnt=3;
        if (colorComponent==sim_colorcomponent_transparency)
            cnt=1;
        for (size_t i=0;i<cnt;i++)
            rgbData[i]=(float)c[i];
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simGetContactInfo(simInt dynamicPass,simInt objectHandle,simInt index,simInt* objectHandles,simFloat* contactInfo)
{
    double inf[9];
    int retVal=simGetContactInfo_internal(dynamicPass,objectHandle,index,objectHandles,inf);
    if (retVal>0)
    {
        size_t cnt=6;
        if ((index&sim_handleflag_extended)!=0)
            cnt=9;
        for (size_t i=0;i<cnt;i++)
            contactInfo[i]=(float)inf[i];
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simAuxiliaryConsoleOpen(const simChar* title,simInt maxLines,simInt mode,const simInt* position,const simInt* size,const simFloat* textColor,const simFloat* backgroundColor)
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
SIM_DLLEXPORT simInt simImportShape(simInt fileformat,const simChar* pathAndFilename,simInt options,simFloat identicalVerticeTolerance,simFloat scalingFactor)
{
    return(simImportShape_internal(fileformat,pathAndFilename,options,(double)identicalVerticeTolerance,(double)scalingFactor));
}
SIM_DLLEXPORT simInt simImportMesh(simInt fileformat,const simChar* pathAndFilename,simInt options,simFloat identicalVerticeTolerance,simFloat scalingFactor,simFloat*** vertices,simInt** verticesSizes,simInt*** indices,simInt** indicesSizes,simFloat*** reserved,simChar*** names)
{
    double** v;
    int retVal=simImportMesh_internal(fileformat,pathAndFilename,options,(double)identicalVerticeTolerance,(double)scalingFactor,&v,verticesSizes,indices,indicesSizes,nullptr,names);
    if (retVal>0)
    {
        vertices[0]=(float**)simCreateBuffer_internal(retVal*sizeof(float*));
        for (int j=0;j<retVal;j++)
        {
            int s=verticesSizes[j]
            vertices[0][j]=(float*)simCreateBuffer_internal(s*sizeof(float));
            for (int i=0;i<s;i++)
                vertices[0][j][i]=(float)v[j][i];
            simReleaseBuffer_internal((simChar*)v[j]);
        }
        simReleaseBuffer_internal((simChar*)v);
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simExportMesh(simInt fileformat,const simChar* pathAndFilename,simInt options,simFloat scalingFactor,simInt elementCount,const simFloat** vertices,const simInt* verticesSizes,const simInt** indices,const simInt* indicesSizes,simFloat** reserved,const simChar** names)
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
SIM_DLLEXPORT simInt simCreateMeshShape(simInt options,simFloat shadingAngle,const simFloat* vertices,simInt verticesSize,const simInt* indices,simInt indicesSize,simFloat* reserved)
{
    std::vector<double> v;
    for (int i=0;i<verticesSize;i++)
        v.push_back((double)vertices[i]);
    return(simCreateMeshShape_internal(options,(double)shadingAngle,&v[0],verticesSize,indices,indicesSize,nullptr));
}
SIM_DLLEXPORT simInt simCreatePrimitiveShape(simInt primitiveType,const simFloat* sizes,simInt options)
{
    double s[3];
    for (size_t i=0;i<3;i++)
        s[i]=(double)sizes[i];
    return(simCreatePrimitiveShape_internal(primitiveType,s,options));
}
SIM_DLLEXPORT simInt simCreateHeightfieldShape(simInt options,simFloat shadingAngle,simInt xPointCount,simInt yPointCount,simFloat xSize,const simFloat* heights)
{
    std::vector<double> h;
    h.resize(xPointCount*yPointCount);
    for (int i=0;i<xPointCount*yPointCount;i++)
        h[i]=(double)heights[i];
    return(simCreateHeightfieldShape_internal(options,(double)shadingAngle,xPointCount,yPointCount,(double)xSize,&h[0]));
}
SIM_DLLEXPORT simInt simGetShapeMesh(simInt shapeHandle,simFloat** vertices,simInt* verticesSize,simInt** indices,simInt* indicesSize,simFloat** normals)
{
    double* vert;
    double* n;
    int retVal=simGetShapeMesh_internal(shapeHandle,&vert,verticesSize,indices,indicesSize,&n);
    if (retVal!=-1)
    {
        vertices[0]=(float*)simCreateBuffer_internal(verticesSize[0]*sizeof(float*));
        normals[0]=(float*)simCreateBuffer_internal(verticesSize[0]*sizeof(float*)*3);
        for (int i=0;i<verticesSize[0];i++)
            vertices[0][i]=(float)vert[i];
        for (int i=0;i<verticesSize[0]*3;i++)
            normals[0][i]=(float)n[i];
        simReleaseBuffer_internal((simChar*)vert);
        simReleaseBuffer_internal((simChar*)n);
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simCreateJoint(simInt jointType,simInt jointMode,simInt options,const simFloat* sizes,const simFloat* reservedA,const simFloat* reservedB)
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
SIM_DLLEXPORT simInt simGetObjectFloatParam(simInt objectHandle,simInt ParamID,simFloat* Param)
{
    double p;
    int retVal=simGetObjectFloatParam_internal(objectHandle,ParamID,&p);
    if (retVal>0)
        Param[0]=(float)p;
    return(retVal);
}
SIM_DLLEXPORT simInt simSetObjectFloatParam(simInt objectHandle,simInt ParamID,simFloat Param)
{
    return(simSetObjectFloatParam_internal(objectHandle,ParamID,(double)Param));
}
SIM_DLLEXPORT simFloat* simGetObjectFloatArrayParam(simInt objectHandle,simInt ParamID,simInt* size)
{
    float* retVal=nullptr;
    double* p=simGetObjectFloatArrayParam_internal(objectHandle,ParamID,size);
    if (p!=nullptr)
    {
        retVal=(float*)simCreateBuffer_internal(size[0]*sizeof(float));
        for (int i=0;i<size[0];i++)
            retVal[i]=(float)p[i];
        simReleaseBuffer_internal((char*)p);
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simSetObjectFloatArrayParam(simInt objectHandle,simInt ParamID,const simFloat* params,simInt size)
{
    std::vector<double> p;
    p.resize(size);
    for (int i=0;i<size;i++)
        p[i]=(double)params[i];
    return(simSetObjectFloatArrayParam_internal(objectHandle,ParamID,&p[0],size));
}
SIM_DLLEXPORT simInt simGetRotationAxis(const simFloat* matrixStart,const simFloat* matrixGoal,simFloat* axis,simFloat* angle)
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
            axis[i]=(float)a[i];
        angle[0]=(float)ang;
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simRotateAroundAxis(const simFloat* matrixIn,const simFloat* axis,const simFloat* axisPos,simFloat angle,simFloat* matrixOut)
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
            matrixOut[i]=(float)mOut[i];
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simGetJointForce(simInt jointHandle,simFloat* forceOrTorque)
{
    double f;
    int retVal=simGetJointForce_internal(jointHandle,&f);
    if (retVal>0)
        forceOrTorque[0]=(float)f;
    return(retVal);
}
SIM_DLLEXPORT simInt simGetJointTargetForce(simInt jointHandle,simFloat* forceOrTorque)
{
    double f;
    int retVal=simGetJointTargetForce_internal(jointHandle,&f);
    if (retVal>0)
        forceOrTorque[0]=(float)f;
    return(retVal);
}
SIM_DLLEXPORT simInt simSetJointTargetForce(simInt objectHandle,simFloat forceOrTorque,simBool signedValue)
{
    return(simSetJointTargetForce_internal(objectHandle,(double)forceOrTorque,signedValue));
}
SIM_DLLEXPORT simInt simCameraFitToView(simInt viewHandleOrIndex,simInt objectCount,const simInt* objectHandles,simInt options,simFloat scaling)
{
    return(simCameraFitToView_internal(viewHandleOrIndex,objectCount,objectHandles,options,(double)scaling));
}
SIM_DLLEXPORT simInt simHandleVisionSensor(simInt visionSensorHandle,simFloat** auxValues,simInt** auxValuesCount)
{
    double* av;
    int retVal=simHandleVisionSensor_internal(visionSensorHandle,&av,auxValuesCount);
    if (retVal!=-1)
    {
        int c=0;
        for (int i=0;i<auxValuesCount[0][0];i++)
            c+=auxValuesCount[0][i+1];
        auxValues[0]=simCreateBuffer_internal(c*sizeof(float));
        for (int i=0;i<c;i++)
            auxValues[0][i]=(float)av[i];
        simReleaseBuffer_internal((char*)av);
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simReadVisionSensor(simInt visionSensorHandle,simFloat** auxValues,simInt** auxValuesCount)
{
    double* av;
    int retVal=simReadVisionSensor_internal(visionSensorHandle,&av,auxValuesCount);
    if (retVal!=-1)
    {
        int c=0;
        for (int i=0;i<auxValuesCount[0][0];i++)
            c+=auxValuesCount[0][i+1];
        auxValues[0]=simCreateBuffer_internal(c*sizeof(float));
        for (int i=0;i<c;i++)
            auxValues[0][i]=(float)av[i];
        simReleaseBuffer_internal((char*)av);
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simCheckVisionSensor(simInt visionSensorHandle,simInt entityHandle,simFloat** auxValues,simInt** auxValuesCount)
{
    double* av;
    int retVal=simCheckVisionSensor_internal(visionSensorHandle,entityHandle,&av,auxValuesCount);
    if (retVal!=-1)
    {
        int c=0;
        for (int i=0;i<auxValuesCount[0][0];i++)
            c+=auxValuesCount[0][i+1];
        auxValues[0]=simCreateBuffer_internal(c*sizeof(float));
        for (int i=0;i<c;i++)
            auxValues[0][i]=(float)av[i];
        simReleaseBuffer_internal((char*)av);
    }
    return(retVal);
}
SIM_DLLEXPORT simFloat* simCheckVisionSensorEx(simInt visionSensorHandle,simInt entityHandle,simBool returnImage)
{
    float retVal=nullptr;
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
            retVal=simCreateBuffer_internal(r[0]*r[1]*s*sizeof(float));
            for (int i=0;i<r[0]*r[1]*s;i++)
                retVal[i]=(float)dta[i];
            simReleaseBuffer_internal((char*)dta);
        }
    }
    return(retVal);
}
SIM_DLLEXPORT simUChar* simGetVisionSensorImg(simInt sensorHandle,simInt options,simFloat rgbaCutOff,const simInt* pos,const simInt* size,simInt* resolution)
{
    return(simGetVisionSensorImg_internal(sensorHandle,options,(double)rgbaCutOff,pos,size,resolution));
}
SIM_DLLEXPORT simFloat* simGetVisionSensorDepth(simInt sensorHandle,simInt options,const simInt* pos,const simInt* size,simInt* resolution)
{
    float retVal=nullptr;
    double* dta=simGetVisionSensorDepth_internal(sensorHandle,options,pos,size,resolution);
    if (dta!=nullptr)
    {
        retVal=simCreateBuffer_internal(resolution[0]*resolution[1]*sizeof(float));
        for (int i=0;i<resolution[0]*resolution[1];i++)
            retVal[i]=(float)dta[i];
        simReleaseBuffer_internal((char*)dta);
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simCreateDummy(simFloat size,const simFloat* reserved)
{
    return(simCreateDummy_internal((double)size,nullptr));
}
SIM_DLLEXPORT simInt simCreateForceSensor(simInt options,const simInt* intParams,const simFloat* floatParams,const simFloat* reserved)
{
    double v[5];
    for (size_t i=0;i<5;i++)
        v[i]=(double)floatParams[i];
    return(simCreateForceSensor_internal(options,intParams,v,nullptr));
}
SIM_DLLEXPORT simInt simCreateProximitySensor(simInt sensorType,simInt subType,simInt options,const simInt* intParams,const simFloat* floatParams,const simFloat* reserved)
{
    double v[15];
    for (size_t i=0;i<15;i++)
        v[i]=(double)floatParams[i];
    return(simCreateProximitySensor_internal(sensorType,subType,options,intParams,v,nullptr));
}
SIM_DLLEXPORT simInt simCreateVisionSensor(simInt options,const simInt* intParams,const simFloat* floatParams,const simFloat* reserved)
{
    double v[11];
    for (size_t i=0;i<11;i++)
        v[i]=(double)floatParams[i];
    return(simCreateVisionSensor_internal(options,intParams,v,nullptr));
}
SIM_DLLEXPORT simInt simConvexDecompose(simInt shapeHandle,simInt options,const simInt* intParams,const simFloat* floatParams)
{
    double v[10];
    for (size_t i=0;i<10;i++)
        v[i]=(double)floatParams[i];
    return(simConvexDecompose_internal(shapeHandle,options,intParams,v));
}
SIM_DLLEXPORT simInt simCreateTexture(const simChar* fileName,simInt options,const simFloat* planeSizes,const simFloat* scalingUV,const simFloat* xy_g,simInt fixedResolution,simInt* textureId,simInt* resolution,const simVoid* reserved)
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
SIM_DLLEXPORT simInt simWriteTexture(simInt textureId,simInt options,const simChar* data,simInt posX,simInt posY,simInt sizeX,simInt sizeY,simFloat interpol)
{
    return(simWriteTexture_internal(textureId,options,data,posX,posY,sizeX,sizeY,(double)interpol));
}
SIM_DLLEXPORT simInt simGetShapeGeomInfo(simInt shapeHandle,simInt* intData,simFloat* floatData,simVoid* reserved)
{
    double v[5];
    int retVal=simGetShapeGeomInfo_internal(shapeHandle,intData,v,reserved);
    if (retVal!=nullptr)
    {
        for (size_t i=0;i<5;i++)
            floatData[i]=(float)v[i];
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simScaleObject(simInt objectHandle,simFloat xScale,simFloat yScale,simFloat zScale,simInt options)
{
    return(simScaleObject_internal(objectHandle,(double)xScale,(double)yScale,(double)zScale,options));
}
SIM_DLLEXPORT simInt simSetShapeTexture(simInt shapeHandle,simInt textureId,simInt mappingMode,simInt options,const simFloat* uvScaling,const simFloat* position,const simFloat* orientation)
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
SIM_DLLEXPORT simInt simTransformImage(simUChar* image,const simInt* resolution,simInt options,const simFloat* floatParams,const simInt* intParams,simVoid* reserved)
{
    return(simTransformImage_internal(image,resolution,options,nullptr,nullptr,nullptr));
}
SIM_DLLEXPORT simInt simGetQHull(const simFloat* inVertices,simInt inVerticesL,simFloat** verticesOut,simInt* verticesOutL,simInt** indicesOut,simInt* indicesOutL,simInt reserved1,const simFloat* reserved2)
{
    std::vector<double> v;
    v.resize(inVerticesL);
    for (int i=0;i<inVerticesL;i++)
        v[i]=(double)inVertices[i];
    double* vo;
    int retVal=simGetQHull_internal(&v[0],inVerticesL,&v0,verticesOutL,indicesOut,indicesOutL,reserved1,nullptr);
    if (retVal>0)
    {
        verticesOut[0]=simCreateBuffer_internal(verticesOutL[0]*sizeof(float));
        for (int i=0;i<verticesOutL[0];i++)
            verticesOut[0][i]=(float)v0[i];
        simReleaseBuffer_internal((char*)vo);
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simGetDecimatedMesh(const simFloat* inVertices,simInt inVerticesL,const simInt* inIndices,simInt inIndicesL,simFloat** verticesOut,simInt* verticesOutL,simInt** indicesOut,simInt* indicesOutL,simFloat decimationPercent,simInt reserved1,const simFloat* reserved2)
{
    std::vector<double> v;
    v.resize(inVerticesL);
    for (int i=0;i<inVerticesL;i++)
        v[i]=(double)inVertices[i];
    double* vo;
    int retVal=simGetDecimatedMesh_internal(&v[0],inVerticesL,inIndices,inIndicesL,&vo,verticesOutL,indicesOut,indicesOutL,(double)decimationPercent,reserved1,nullptr);
    if (retVal>0)
    {
        verticesOut[0]=simCreateBuffer_internal(verticesOutL[0]*sizeof(float));
        for (int i=0;i<verticesOutL[0];i++)
            verticesOut[0][i]=(float)v0[i];
        simReleaseBuffer_internal((char*)vo);
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simComputeMassAndInertia(simInt shapeHandle,simFloat density)
{
    return(simComputeMassAndInertia_internal(shapeHandle,(double)density));
}
SIM_DLLEXPORT simFloat simGetEngineFloatParam(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{
    return((float)simGetEngineFloatParam_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT simInt simSetEngineFloatParam(simInt paramId,simInt objectHandle,const simVoid* object,simFloat val)
{
    return(simSetEngineFloatParam_internal(paramId,objectHandle,object,(double)val));
}
SIM_DLLEXPORT simInt simCreateOctree(simFloat voxelSize,simInt options,simFloat pointSize,simVoid* reserved)
{
    return(simCreateOctree_internal((double)voxelSize,options,(double)pointSize,reserved));
}
SIM_DLLEXPORT simInt simCreatePointCloud(simFloat maxVoxelSize,simInt maxPtCntPerVoxel,simInt options,simFloat pointSize,simVoid* reserved)
{
    return(simCreatePointCloud_internal((double)maxVoxelSize,maxPtCntPerVoxel,options,(double)pointSize,reserved));
}
SIM_DLLEXPORT simInt simSetPointCloudOptions(simInt pointCloudHandle,simFloat maxVoxelSize,simInt maxPtCntPerVoxel,simInt options,simFloat pointSize,simVoid* reserved)
{
    return(simSetPointCloudOptions_internal(pointCloudHandle,(double)maxVoxelSize,maxPtCntPerVoxel,options,(double)pointSize,reserved));
}
SIM_DLLEXPORT simInt simGetPointCloudOptions(simInt pointCloudHandle,simFloat* maxVoxelSize,simInt* maxPtCntPerVoxel,simInt* options,simFloat* pointSize,simVoid* reserved)
{
    double mvs;
    double ps;
    int retVal=simGetPointCloudOptions_internal(pointCloudHandle,&mvs,maxPtCntPerVoxel,options,&ps,reserved);
    if (retVal>0)
    {
        maxVoxelSize[0]=(float)msv;
        pointSize[0]=(float)ps;
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simInsertVoxelsIntoOctree(simInt octreeHandle,simInt options,const simFloat* pts,simInt ptCnt,const simUChar* color,const simUInt* tag,simVoid* reserved)
{
    std::vector<double> p;
    p.resize(ptCnt*3);
    for (int i=0;i<ptCnt*3;i++)
        p[i]=(double)pts[i];
    return(simInsertVoxelsIntoOctree_internal(octreeHandle,options,&p[0],ptCnt,color,tag,reserved));
}
SIM_DLLEXPORT simInt simRemoveVoxelsFromOctree(simInt octreeHandle,simInt options,const simFloat* pts,simInt ptCnt,simVoid* reserved)
{
    std::vector<double> p;
    p.resize(ptCnt*3);
    for (int i=0;i<ptCnt*3;i++)
        p[i]=(double)pts[i];
    return(simRemoveVoxelsFromOctree_internal(octreeHandle,options,&p[0],ptCnt,reserved));
}
SIM_DLLEXPORT simInt simInsertPointsIntoPointCloud(simInt pointCloudHandle,simInt options,const simFloat* pts,simInt ptCnt,const simUChar* color,simVoid* optionalValues)
{
    std::vector<double> p;
    p.resize(ptCnt*3);
    for (int i=0;i<ptCnt*3;i++)
        p[i]=(double)pts[i];
    return(simInsertPointsIntoPointCloud_internal(pointCloudHandle,options,&p[0],ptCnt,color,optionalValues));
}
SIM_DLLEXPORT simInt simRemovePointsFromPointCloud(simInt pointCloudHandle,simInt options,const simFloat* pts,simInt ptCnt,simFloat tolerance,simVoid* reserved)
{
    std::vector<double> p;
    p.resize(ptCnt*3);
    for (int i=0;i<ptCnt*3;i++)
        p[i]=(double)pts[i];
    return(simRemovePointsFromPointCloud_internal(pointCloudHandle,options,&p[0],ptCnt,(double)tolerance,reserved));
}
SIM_DLLEXPORT simInt simIntersectPointsWithPointCloud(simInt pointCloudHandle,simInt options,const simFloat* pts,simInt ptCnt,simFloat tolerance,simVoid* reserved)
{
    std::vector<double> p;
    p.resize(ptCnt*3);
    for (int i=0;i<ptCnt*3;i++)
        p[i]=(double)pts[i];
    return(simIntersectPointsWithPointCloud_internal(pointCloudHandle,options,&p[0],ptCnt,(double)tolerance,reserved));
}
SIM_DLLEXPORT const simFloat* simGetOctreeVoxels(simInt octreeHandle,simInt* ptCnt,simVoid* reserved)
{
    return(nullptr); // We drop this function in case of single-point precision
}
SIM_DLLEXPORT const simFloat* simGetPointCloudPoints(simInt pointCloudHandle,simInt* ptCnt,simVoid* reserved)
{
    return(nullptr); // We drop this function in case of single-point precision
}
SIM_DLLEXPORT simInt simInsertObjectIntoPointCloud(simInt pointCloudHandle,simInt objectHandle,simInt options,simFloat gridSize,const simUChar* color,simVoid* optionalValues)
{
    return(simInsertObjectIntoPointCloud_internal(pointCloudHandle,objectHandle,options,(double)gridSize,color,optionalValues));
}
SIM_DLLEXPORT simInt simSubtractObjectFromPointCloud(simInt pointCloudHandle,simInt objectHandle,simInt options,simFloat tolerance,simVoid* reserved)
{
    return(simSubtractObjectFromPointCloud_internal(pointCloudHandle,objectHandle,options,(double)tolerance,reserved));
}
SIM_DLLEXPORT simInt simCheckOctreePointOccupancy(simInt octreeHandle,simInt options,const simFloat* points,simInt ptCnt,simUInt* tag,simUInt64* location,simVoid* reserved)
{
    std::vector<double> p;
    p.resize(ptCnt*3);
    for (int i=0;i<ptCnt*3;i++)
        p[i]=(double)points[i];
    return(simCheckOctreePointOccupancy_internal(octreeHandle,options,&p[0],ptCnt,tag,location,reserved));
}
SIM_DLLEXPORT simInt simApplyTexture(simInt shapeHandle,const simFloat* textureCoordinates,simInt textCoordSize,const simUChar* texture,const simInt* textureResolution,simInt options)
{
    std::vector<double> tc;
    tc.resize(textCoordSize);
    for (i=0;i<textCoordSize;i++)
        tc[i]=(double)textureCoordinates[i];
    return(simApplyTexture_internal(shapeHandle,&tc[0],textCoordSize,texture,textureResolution,options));
}
SIM_DLLEXPORT simInt simSetJointDependency(simInt jointHandle,simInt masterJointHandle,simFloat offset,simFloat multCoeff)
{
    return(simSetJointDependency_internal(jointHandle,masterJointHandle,(double)offset,(double)multCoeff));
}
SIM_DLLEXPORT simInt simGetJointDependency(simInt jointHandle,simInt* masterJointHandle,simFloat* offset,simFloat* multCoeff)
{
    double o;
    double m;
    int retVal=simGetJointDependency_internal(jointHandle,masterJointHandle,&o,&m);
    if (retVal!=-1)
    {
        offset[0]=(float)o;
        multCoeff[0]=(float)m;
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simGetShapeMass(simInt shapeHandle,simFloat* mass)
{
    double m;
    int retVal=simGetShapeMass_internal(shapeHandle,&m);
    if (retVal!=-1)
        mass[0]=(float)m;
    return(retVal);
}
SIM_DLLEXPORT simInt simSetShapeMass(simInt shapeHandle,simFloat mass)
{
    return(simSetShapeMass_internal(shapeHandle,(double)mass));
}
SIM_DLLEXPORT simInt simGetShapeInertia(simInt shapeHandle,simFloat* inertiaMatrix,simFloat* transformationMatrix)
{
    double ine[9];
    double matr[12];
    int retVal=simGetShapeInertia_internal(shapeHandle,ine,matr);
    if (retVal!=-1)
    {
        for (size_t i=0;i<9;i++)
            inertiaMatrix[i]=(float)ine[i];
        for (size_t i=0;i<12;i++)
            transformationMatrix[i]=(float)matr[i];
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simSetShapeInertia(simInt shapeHandle,const simFloat* inertiaMatrix,const simFloat* transformationMatrix)
{
    double ine[9];
    double matr[12];
    for (size_t i=0;i<9;i++)
        ine[i]=(double)inertiaMatrix[i];
    for (size_t i=0;i<12;i++)
        matr[i]=(double)transformationMatrix[i];
    return(simSetShapeInertia_internal(shapeHandle,ine,matr));
}
SIM_DLLEXPORT simInt simGenerateShapeFromPath(const simFloat* path,simInt pathSize,const simFloat* section,simInt sectionSize,simInt options,const simFloat* upVector,simFloat reserved)
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
SIM_DLLEXPORT simFloat simGetClosestPosOnPath(const simFloat* path,simInt pathSize,const simFloat* pathLengths,const simFloat* absPt)
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
    return((float)simGetClosestPosOnPath_internal(&p[0],pathSize,&pl[0],&pt[0]));
}
SIM_DLLEXPORT simInt simExtCallScriptFunction(simInt scriptHandleOrType, const simChar* functionNameAtScriptName,
                                               const simInt* inIntData, simInt inIntCnt,
                                               const simFloat* inFloatData, simInt inFloatCnt,
                                               const simChar** inStringData, simInt inStringCnt,
                                               const simChar* inBufferData, simInt inBufferCnt,
                                               simInt** outIntData, simInt* outIntCnt,
                                               simFloat** outFloatData, simInt* outFloatCnt,
                                               simChar*** outStringData, simInt* outStringCnt,
                                               simChar** outBufferData, simInt* outBufferSize)
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
        outFloatData[0]=simCreateBuffer_internal(outFloatCnt[0]*sizeof(float));
        for (int i=0;i<outFloatCnt[0];i++)
            outFloatData[0][i]=(float)outFloatD[i];
        simReleaseBuffer_internal((char*)outFloatD);
    }
    return(retVal);
}

SIM_DLLEXPORT simVoid _simGetObjectLocalTransformation(const simVoid* object,simFloat* pos,simFloat* quat,simBool excludeFirstJointTransformation)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simSetObjectLocalTransformation(simVoid* object,const simFloat* pos,const simFloat* quat,simFloat simTime)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simDynReportObjectCumulativeTransformation(simVoid* object,const simFloat* pos,const simFloat* quat,simFloat simTime)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simSetObjectCumulativeTransformation(simVoid* object,const simFloat* pos,const simFloat* quat,simBool keepChildrenInPlace)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simGetObjectCumulativeTransformation(const simVoid* object,simFloat* pos,simFloat* quat,simBool excludeFirstJointTransformation)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simSetJointVelocity(const simVoid* joint,simFloat vel)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simSetJointPosition(const simVoid* joint,simFloat pos)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simFloat _simGetJointPosition(const simVoid* joint)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT simVoid _simSetDynamicMotorPositionControlTargetPosition(const simVoid* joint,simFloat pos)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simGetInitialDynamicVelocity(const simVoid* shape,simFloat* vel)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simSetInitialDynamicVelocity(simVoid* shape,const simFloat* vel)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simGetInitialDynamicAngVelocity(const simVoid* shape,simFloat* angularVel)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simSetInitialDynamicAngVelocity(simVoid* shape,const simFloat* angularVel)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simSetShapeDynamicVelocity(simVoid* shape,const simFloat* linear,const simFloat* angular,simFloat simTime)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simGetAdditionalForceAndTorque(const simVoid* shape,simFloat* force,simFloat* torque)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simBool _simGetJointPositionInterval(const simVoid* joint,simFloat* minValue,simFloat* rangeValue)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
    return(0);
}
SIM_DLLEXPORT simFloat _simGetDynamicMotorTargetPosition(const simVoid* joint)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT simFloat _simGetDynamicMotorTargetVelocity(const simVoid* joint)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT simFloat _simGetDynamicMotorMaxForce(const simVoid* joint)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT simFloat _simGetDynamicMotorUpperLimitVelocity(const simVoid* joint)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT simVoid _simSetDynamicMotorReflectedPositionFromDynamicEngine(simVoid* joint,simFloat pos,simFloat simTime)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simSetJointSphericalTransformation(simVoid* joint,const simFloat* quat,simFloat simTime)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simAddForceSensorCumulativeForcesAndTorques(simVoid* forceSensor,const simFloat* force,const simFloat* torque,int totalPassesCount,simFloat simTime)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simAddJointCumulativeForcesOrTorques(simVoid* joint,simFloat forceOrTorque,int totalPassesCount,simFloat simTime)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simFloat _simGetLocalInertiaInfo(const simVoid* object,simFloat* pos,simFloat* quat,simFloat* diagI)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT simFloat _simGetMass(const simVoid* geomInfo)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT simVoid _simGetPurePrimitiveSizes(const simVoid* geometric,simFloat* sizes)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simGetVerticesLocalFrame(const simVoid* geometric,simFloat* pos,simFloat* quat)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT const simFloat* _simGetHeightfieldData(const simVoid* geometric,simInt* xCount,simInt* yCount,simFloat* minHeight,simFloat* maxHeight)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
    return(nullptr);
}
SIM_DLLEXPORT simVoid _simGetCumulativeMeshes(const simVoid* geomInfo,simFloat** vertices,simInt* verticesSize,simInt** indices,simInt* indicesSize)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simGetGravity(simFloat* gravity)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simBool _simGetDistanceBetweenEntitiesIfSmaller(simInt entity1ID,simInt entity2ID,simFloat* distance,simFloat* ray,simInt* cacheBuffer,simBool overrideMeasurableFlagIfNonCollection1,simBool overrideMeasurableFlagIfNonCollection2,simBool pathPlanningRoutineCalling)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
    return(0);
}
SIM_DLLEXPORT simInt _simHandleJointControl(const simVoid* joint,simInt auxV,const simInt* inputValuesInt,const simFloat* inputValuesFloat,simFloat* outputValues)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
    return(0);
}
SIM_DLLEXPORT simInt _simHandleCustomContact(simInt objHandle1,simInt objHandle2,simInt engine,simInt* dataInt,simFloat* dataFloat)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
    return(0);
}
SIM_DLLEXPORT simFloat _simGetPureHollowScaling(const simVoid* geometric)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
    return(0);
}
SIM_DLLEXPORT simVoid _simDynCallback(const simInt* intData,const simFloat* floatData)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
#else
SIM_DLLEXPORT simInt simSetFloatParam(simInt parameter,simFloat floatState)
{
    return(simSetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT simInt simGetFloatParam(simInt parameter,simFloat* floatState)
{
    return(simGetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT simInt simGetObjectMatrix(simInt objectHandle,simInt relativeToObjectHandle,simFloat* matrix)
{
    return(simGetObjectMatrix_internal(objectHandle,relativeToObjectHandle,matrix));
}
SIM_DLLEXPORT simInt simSetObjectMatrix(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* matrix)
{
    return(simSetObjectMatrix_internal(objectHandle,relativeToObjectHandle,matrix));
}
SIM_DLLEXPORT simInt simGetObjectPose(simInt objectHandle,simInt relativeToObjectHandle,simFloat* pose)
{
    return(simGetObjectPose_internal(objectHandle,relativeToObjectHandle,pose));
}
SIM_DLLEXPORT simInt simSetObjectPose(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* pose)
{
    return(simSetObjectPose_internal(objectHandle,relativeToObjectHandle,pose));
}
SIM_DLLEXPORT simInt simGetObjectPosition(simInt objectHandle,simInt relativeToObjectHandle,simFloat* position)
{
    return(simGetObjectPosition_internal(objectHandle,relativeToObjectHandle,position));
}
SIM_DLLEXPORT simInt simSetObjectPosition(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* position)
{
    return(simSetObjectPosition_internal(objectHandle,relativeToObjectHandle,position));
}
SIM_DLLEXPORT simInt simGetObjectOrientation(simInt objectHandle,simInt relativeToObjectHandle,simFloat* eulerAngles)
{
    return(simGetObjectOrientation_internal(objectHandle,relativeToObjectHandle,eulerAngles));
}
SIM_DLLEXPORT simInt simGetObjectQuaternion(simInt objectHandle,simInt relativeToObjectHandle,simFloat* quaternion)
{
    return(simGetObjectQuaternion_internal(objectHandle,relativeToObjectHandle,quaternion));
}
SIM_DLLEXPORT simInt simSetObjectQuaternion(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* quaternion)
{
    return(simSetObjectQuaternion_internal(objectHandle,relativeToObjectHandle,quaternion));
}
SIM_DLLEXPORT simInt simSetObjectOrientation(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* eulerAngles)
{
    return(simSetObjectOrientation_internal(objectHandle,relativeToObjectHandle,eulerAngles));
}
SIM_DLLEXPORT simInt simGetJointPosition(simInt objectHandle,simFloat* position)
{
    return(simGetJointPosition_internal(objectHandle,position));
}
SIM_DLLEXPORT simInt simSetJointPosition(simInt objectHandle,simFloat position)
{
    return(simSetJointPosition_internal(objectHandle,position));
}
SIM_DLLEXPORT simInt simSetJointTargetPosition(simInt objectHandle,simFloat targetPosition)
{
    return(simSetJointTargetPosition_internal(objectHandle,targetPosition));
}
SIM_DLLEXPORT simInt simGetJointTargetPosition(simInt objectHandle,simFloat* targetPosition)
{
    return(simGetJointTargetPosition_internal(objectHandle,targetPosition));
}
SIM_DLLEXPORT simInt simGetObjectChildPose(simInt objectHandle,simFloat* pose)
{
    return(simGetObjectChildPose_internal(objectHandle,pose));
}
SIM_DLLEXPORT simInt simSetObjectChildPose(simInt objectHandle,const simFloat* pose)
{
    return(simSetObjectChildPose_internal(objectHandle,pose));
}
SIM_DLLEXPORT simInt simGetJointInterval(simInt objectHandle,simBool* cyclic,simFloat* interval)
{
    return(simGetJointInterval_internal(objectHandle,cyclic,interval));
}
SIM_DLLEXPORT simInt simSetJointInterval(simInt objectHandle,simBool cyclic,const simFloat* interval)
{
    return(simSetJointInterval_internal(objectHandle,cyclic,interval));
}
SIM_DLLEXPORT simInt simBuildIdentityMatrix(simFloat* matrix)
{
    return(simBuildIdentityMatrix_internal(matrix));
}
SIM_DLLEXPORT simInt simBuildMatrix(const simFloat* position,const simFloat* eulerAngles,simFloat* matrix)
{
    return(simBuildMatrix_internal(position,eulerAngles,matrix));
}
SIM_DLLEXPORT simInt simBuildPose(const simFloat* position,const simFloat* eulerAngles,simFloat* pose)
{
    return(simBuildPose_internal(position,eulerAngles,pose));
}
SIM_DLLEXPORT simInt simGetEulerAnglesFromMatrix(const simFloat* matrix,simFloat* eulerAngles)
{
    return(simGetEulerAnglesFromMatrix_internal(matrix,eulerAngles));
}
SIM_DLLEXPORT simInt simInvertMatrix(simFloat* matrix)
{
    return(simInvertMatrix_internal(matrix));
}
SIM_DLLEXPORT simInt simMultiplyMatrices(const simFloat* matrixIn1,const simFloat* matrixIn2,simFloat* matrixOut)
{
    return(simMultiplyMatrices_internal(matrixIn1,matrixIn2,matrixOut));
}
SIM_DLLEXPORT simInt simMultiplyPoses(const simFloat* poseIn1,const simFloat* poseIn2,simFloat* poseOut)
{
    return(simMultiplyPoses_internal(poseIn1,poseIn2,poseOut));
}
SIM_DLLEXPORT simInt simInvertPose(simFloat* pose)
{
    return(simInvertPose_internal(pose));
}
SIM_DLLEXPORT simInt simInterpolatePoses(const simFloat* poseIn1,const simFloat* poseIn2,simFloat interpolFactor,simFloat* poseOut)
{
    return(simInterpolatePoses_internal(poseIn1,poseIn2,interpolFactor,poseOut));
}
SIM_DLLEXPORT simInt simPoseToMatrix(const simFloat* poseIn,simFloat* matrixOut)
{
    return(simPoseToMatrix_internal(poseIn,matrixOut));
}
SIM_DLLEXPORT simInt simMatrixToPose(const simFloat* matrixIn,simFloat* poseOut)
{
    return(simMatrixToPose_internal(matrixIn,poseOut));
}
SIM_DLLEXPORT simInt simInterpolateMatrices(const simFloat* matrixIn1,const simFloat* matrixIn2,simFloat interpolFactor,simFloat* matrixOut)
{
    return(simInterpolateMatrices_internal(matrixIn1,matrixIn2,interpolFactor,matrixOut));
}
SIM_DLLEXPORT simInt simTransformVector(const simFloat* matrix,simFloat* vect)
{
    return(simTransformVector_internal(matrix,vect));
}
SIM_DLLEXPORT simFloat simGetSimulationTime()
{
    return(simGetSimulationTime_internal());
}
SIM_DLLEXPORT simFloat simGetSystemTime()
{
    return(float(simGetSystemTime_internal()));
}
SIM_DLLEXPORT simInt simHandleProximitySensor(simInt sensorHandle,simFloat* detectedPoint,simInt* detectedObjectHandle,simFloat* normalVector)
{
    return(simHandleProximitySensor_internal(sensorHandle,detectedPoint,detectedObjectHandle,normalVector));
}
SIM_DLLEXPORT simInt simReadProximitySensor(simInt sensorHandle,simFloat* detectedPoint,simInt* detectedObjectHandle,simFloat* normalVector)
{
    return(simReadProximitySensor_internal(sensorHandle,detectedPoint,detectedObjectHandle,normalVector));
}
SIM_DLLEXPORT simInt simHandleDynamics(simFloat deltaTime)
{
    return(simHandleDynamics_internal(deltaTime));
}
SIM_DLLEXPORT simInt simCheckProximitySensor(simInt sensorHandle,simInt entityHandle,simFloat* detectedPoint)
{
    return(simCheckProximitySensor_internal(sensorHandle,entityHandle,detectedPoint));
}
SIM_DLLEXPORT simInt simCheckProximitySensorEx(simInt sensorHandle,simInt entityHandle,simInt detectionMode,simFloat detectionThreshold,simFloat maxAngle,simFloat* detectedPoint,simInt* detectedObjectHandle,simFloat* normalVector)
{
    return(simCheckProximitySensorEx_internal(sensorHandle,entityHandle,detectionMode,detectionThreshold,maxAngle,detectedPoint,detectedObjectHandle,normalVector));
}
SIM_DLLEXPORT simInt simCheckProximitySensorEx2(simInt sensorHandle,simFloat* vertexPointer,simInt itemType,simInt itemCount,simInt detectionMode,simFloat detectionThreshold,simFloat maxAngle,simFloat* detectedPoint,simFloat* normalVector)
{
    return(simCheckProximitySensorEx2_internal(sensorHandle,vertexPointer,itemType,itemCount,detectionMode,detectionThreshold,maxAngle,detectedPoint,normalVector));
}
SIM_DLLEXPORT simInt simCheckCollisionEx(simInt entity1Handle,simInt entity2Handle,simFloat** intersectionSegments)
{
    return(simCheckCollisionEx_internal(entity1Handle,entity2Handle,intersectionSegments));
}
SIM_DLLEXPORT simInt simCheckDistance(simInt entity1Handle,simInt entity2Handle,simFloat threshold,simFloat* distanceData)
{
    return(simCheckDistance_internal(entity1Handle,entity2Handle,threshold,distanceData));
}
SIM_DLLEXPORT simInt simSetSimulationTimeStep(simFloat timeStep)
{
    return(simSetSimulationTimeStep_internal(timeStep));
}
SIM_DLLEXPORT simFloat simGetSimulationTimeStep()
{
    return(simGetSimulationTimeStep_internal());
}
SIM_DLLEXPORT simInt simAdjustRealTimeTimer(simInt instanceIndex,simFloat deltaTime)
{
    return(simAdjustRealTimeTimer_internal(instanceIndex,deltaTime));
}
SIM_DLLEXPORT simInt simFloatingViewAdd(simFloat posX,simFloat posY,simFloat sizeX,simFloat sizeY,simInt options)
{
    return(simFloatingViewAdd_internal(posX,posY,sizeX,sizeY,options));
}
SIM_DLLEXPORT simInt simHandleGraph(simInt graphHandle,simFloat simulationTime)
{
    return(simHandleGraph_internal(graphHandle,simulationTime));
}
SIM_DLLEXPORT simInt simAddGraphStream(simInt graphHandle,const simChar* streamName,const simChar* unitStr,simInt options,const simFloat* color,simFloat cyclicRange)
{
    return(simAddGraphStream_internal(graphHandle,streamName,unitStr,options,color,cyclicRange));
}
SIM_DLLEXPORT simInt simSetGraphStreamTransformation(simInt graphHandle,simInt streamId,simInt trType,simFloat mult,simFloat off,simInt movingAvgPeriod)
{
    return(simSetGraphStreamTransformation_internal(graphHandle,streamId,trType,mult,off,movingAvgPeriod));
}
SIM_DLLEXPORT simInt simAddGraphCurve(simInt graphHandle,const simChar* curveName,simInt dim,const simInt* streamIds,const simFloat* defaultValues,const simChar* unitStr,simInt options,const simFloat* color,simInt curveWidth)
{
    return(simAddGraphCurve_internal(graphHandle,curveName,dim,streamIds,defaultValues,unitStr,options,color,curveWidth));
}
SIM_DLLEXPORT simInt simSetGraphStreamValue(simInt graphHandle,simInt streamId,simFloat value)
{
    return(simSetGraphStreamValue_internal(graphHandle,streamId,value));
}
SIM_DLLEXPORT simInt simSetJointTargetVelocity(simInt objectHandle,simFloat targetVelocity)
{
    return(simSetJointTargetVelocity_internal(objectHandle,targetVelocity));
}
SIM_DLLEXPORT simInt simGetJointTargetVelocity(simInt objectHandle,simFloat* targetVelocity)
{
    return(simGetJointTargetVelocity_internal(objectHandle,targetVelocity));
}
SIM_DLLEXPORT simInt simScaleObjects(const simInt* objectHandles,simInt objectCount,simFloat scalingFactor,simBool scalePositionsToo)
{
    return(simScaleObjects_internal(objectHandles,objectCount,scalingFactor,scalePositionsToo));
}
SIM_DLLEXPORT simInt simAddDrawingObject(simInt objectType,simFloat size,simFloat duplicateTolerance,simInt parentObjectHandle,simInt maxItemCount,const simFloat* color,const simFloat* setToNULL,const simFloat* setToNULL2,const simFloat* setToNULL3)
{
    return(simAddDrawingObject_internal(objectType,size,duplicateTolerance,parentObjectHandle,maxItemCount,color,setToNULL,setToNULL2,setToNULL3));
}
SIM_DLLEXPORT simInt simAddDrawingObjectItem(simInt objectHandle,const simFloat* itemData)
{
    return(simAddDrawingObjectItem_internal(objectHandle,itemData));
}
SIM_DLLEXPORT simFloat simGetObjectSizeFactor(simInt objectHandle)
{
    return(simGetObjectSizeFactor_internal(objectHandle));
}
SIM_DLLEXPORT simInt simSetFloatSignal(const simChar* signalName,simFloat signalValue)
{
    return(simSetFloatSignal_internal(signalName,signalValue));
}
SIM_DLLEXPORT simInt simGetFloatSignal(const simChar* signalName,simFloat* signalValue)
{
    return(simGetFloatSignal_internal(signalName,signalValue));
}
SIM_DLLEXPORT simInt simReadForceSensor(simInt objectHandle,simFloat* forceVector,simFloat* torqueVector)
{
    return(simReadForceSensor_internal(objectHandle,forceVector,torqueVector));
}
SIM_DLLEXPORT simInt simSetLightParameters(simInt objectHandle,simInt state,const simFloat* setToNULL,const simFloat* diffusePart,const simFloat* specularPart)
{
    return(simSetLightParameters_internal(objectHandle,state,setToNULL,diffusePart,specularPart));
}
SIM_DLLEXPORT simInt simGetLightParameters(simInt objectHandle,simFloat* setToNULL,simFloat* diffusePart,simFloat* specularPart)
{
    return(simGetLightParameters_internal(objectHandle,setToNULL,diffusePart,specularPart));
}
SIM_DLLEXPORT simInt simGetVelocity(simInt shapeHandle,simFloat* linearVelocity,simFloat* angularVelocity)
{
    return(simGetVelocity_internal(shapeHandle,linearVelocity,angularVelocity));
}
SIM_DLLEXPORT simInt simGetObjectVelocity(simInt objectHandle,simFloat* linearVelocity,simFloat* angularVelocity)
{
    return(simGetObjectVelocity_internal(objectHandle,linearVelocity,angularVelocity));
}
SIM_DLLEXPORT simInt simGetJointVelocity(simInt jointHandle,simFloat* velocity)
{
    return(simGetJointVelocity_internal(jointHandle,velocity));
}
SIM_DLLEXPORT simInt simAddForceAndTorque(simInt shapeHandle,const simFloat* force,const simFloat* torque)
{
    return(simAddForceAndTorque_internal(shapeHandle,force,torque));
}
SIM_DLLEXPORT simInt simAddForce(simInt shapeHandle,const simFloat* position,const simFloat* force)
{
    return(simAddForce_internal(shapeHandle,position,force));
}
SIM_DLLEXPORT simInt simSetObjectColor(simInt objectHandle,simInt index,simInt colorComponent,const simFloat* rgbData)
{
    return(simSetObjectColor_internal(objectHandle,index,colorComponent,rgbData));
}
SIM_DLLEXPORT simInt simGetObjectColor(simInt objectHandle,simInt index,simInt colorComponent,simFloat* rgbData)
{
    return(simGetObjectColor_internal(objectHandle,index,colorComponent,rgbData));
}
SIM_DLLEXPORT simInt simSetShapeColor(simInt shapeHandle,const simChar* colorName,simInt colorComponent,const simFloat* rgbData)
{
    return(simSetShapeColor_internal(shapeHandle,colorName,colorComponent,rgbData));
}
SIM_DLLEXPORT simInt simGetShapeColor(simInt shapeHandle,const simChar* colorName,simInt colorComponent,simFloat* rgbData)
{
    return(simGetShapeColor_internal(shapeHandle,colorName,colorComponent,rgbData));
}
SIM_DLLEXPORT simInt simGetContactInfo(simInt dynamicPass,simInt objectHandle,simInt index,simInt* objectHandles,simFloat* contactInfo)
{
    return(simGetContactInfo_internal(dynamicPass,objectHandle,index,objectHandles,contactInfo));
}
SIM_DLLEXPORT simInt simAuxiliaryConsoleOpen(const simChar* title,simInt maxLines,simInt mode,const simInt* position,const simInt* size,const simFloat* textColor,const simFloat* backgroundColor)
{
    return(simAuxiliaryConsoleOpen_internal(title,maxLines,mode,position,size,textColor,backgroundColor));
}
SIM_DLLEXPORT simInt simImportShape(simInt fileformat,const simChar* pathAndFilename,simInt options,simFloat identicalVerticeTolerance,simFloat scalingFactor)
{
    return(simImportShape_internal(fileformat,pathAndFilename,options,identicalVerticeTolerance,scalingFactor));
}
SIM_DLLEXPORT simInt simImportMesh(simInt fileformat,const simChar* pathAndFilename,simInt options,simFloat identicalVerticeTolerance,simFloat scalingFactor,simFloat*** vertices,simInt** verticesSizes,simInt*** indices,simInt** indicesSizes,simFloat*** reserved,simChar*** names)
{
    return(simImportMesh_internal(fileformat,pathAndFilename,options,identicalVerticeTolerance,scalingFactor,vertices,verticesSizes,indices,indicesSizes,reserved,names));
}
SIM_DLLEXPORT simInt simExportMesh(simInt fileformat,const simChar* pathAndFilename,simInt options,simFloat scalingFactor,simInt elementCount,const simFloat** vertices,const simInt* verticesSizes,const simInt** indices,const simInt* indicesSizes,simFloat** reserved,const simChar** names)
{
    return(simExportMesh_internal(fileformat,pathAndFilename,options,scalingFactor,elementCount,vertices,verticesSizes,indices,indicesSizes,reserved,names));
}
SIM_DLLEXPORT simInt simCreateMeshShape(simInt options,simFloat shadingAngle,const simFloat* vertices,simInt verticesSize,const simInt* indices,simInt indicesSize,simFloat* reserved)
{
    return(simCreateMeshShape_internal(options,shadingAngle,vertices,verticesSize,indices,indicesSize,reserved));
}
SIM_DLLEXPORT simInt simCreatePrimitiveShape(simInt primitiveType,const simFloat* sizes,simInt options)
{
    return(simCreatePrimitiveShape_internal(primitiveType,sizes,options));
}
SIM_DLLEXPORT simInt simCreateHeightfieldShape(simInt options,simFloat shadingAngle,simInt xPointCount,simInt yPointCount,simFloat xSize,const simFloat* heights)
{
    return(simCreateHeightfieldShape_internal(options,shadingAngle,xPointCount,yPointCount,xSize,heights));
}
SIM_DLLEXPORT simInt simGetShapeMesh(simInt shapeHandle,simFloat** vertices,simInt* verticesSize,simInt** indices,simInt* indicesSize,simFloat** normals)
{
    return(simGetShapeMesh_internal(shapeHandle,vertices,verticesSize,indices,indicesSize,normals));
}
SIM_DLLEXPORT simInt simCreateJoint(simInt jointType,simInt jointMode,simInt options,const simFloat* sizes,const simFloat* reservedA,const simFloat* reservedB)
{
    return(simCreateJoint_internal(jointType,jointMode,options,sizes,reservedA,reservedB));
}
SIM_DLLEXPORT simInt simGetObjectFloatParam(simInt objectHandle,simInt ParamID,simFloat* Param)
{
    return(simGetObjectFloatParam_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT simInt simSetObjectFloatParam(simInt objectHandle,simInt ParamID,simFloat Param)
{
    return(simSetObjectFloatParam_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT simFloat* simGetObjectFloatArrayParam(simInt objectHandle,simInt ParamID,simInt* size)
{
    return(simGetObjectFloatArrayParam_internal(objectHandle,ParamID,size));
}
SIM_DLLEXPORT simInt simSetObjectFloatArrayParam(simInt objectHandle,simInt ParamID,const simFloat* params,simInt size)
{
    return(simSetObjectFloatArrayParam_internal(objectHandle,ParamID,params,size));
}
SIM_DLLEXPORT simInt simGetRotationAxis(const simFloat* matrixStart,const simFloat* matrixGoal,simFloat* axis,simFloat* angle)
{
    return(simGetRotationAxis_internal(matrixStart,matrixGoal,axis,angle));
}
SIM_DLLEXPORT simInt simRotateAroundAxis(const simFloat* matrixIn,const simFloat* axis,const simFloat* axisPos,simFloat angle,simFloat* matrixOut)
{
    return(simRotateAroundAxis_internal(matrixIn,axis,axisPos,angle,matrixOut));
}
SIM_DLLEXPORT simInt simGetJointForce(simInt jointHandle,simFloat* forceOrTorque)
{
    return(simGetJointForce_internal(jointHandle,forceOrTorque));
}
SIM_DLLEXPORT simInt simGetJointTargetForce(simInt jointHandle,simFloat* forceOrTorque)
{
    return(simGetJointTargetForce_internal(jointHandle,forceOrTorque));
}
SIM_DLLEXPORT simInt simSetJointTargetForce(simInt objectHandle,simFloat forceOrTorque,simBool signedValue)
{
    return(simSetJointTargetForce_internal(objectHandle,forceOrTorque,signedValue));
}
SIM_DLLEXPORT simInt simCameraFitToView(simInt viewHandleOrIndex,simInt objectCount,const simInt* objectHandles,simInt options,simFloat scaling)
{
    return(simCameraFitToView_internal(viewHandleOrIndex,objectCount,objectHandles,options,scaling));
}
SIM_DLLEXPORT simInt simHandleVisionSensor(simInt visionSensorHandle,simFloat** auxValues,simInt** auxValuesCount)
{
    return(simHandleVisionSensor_internal(visionSensorHandle,auxValues,auxValuesCount));
}
SIM_DLLEXPORT simInt simReadVisionSensor(simInt visionSensorHandle,simFloat** auxValues,simInt** auxValuesCount)
{
    return(simReadVisionSensor_internal(visionSensorHandle,auxValues,auxValuesCount));
}
SIM_DLLEXPORT simInt simCheckVisionSensor(simInt visionSensorHandle,simInt entityHandle,simFloat** auxValues,simInt** auxValuesCount)
{
    return(simCheckVisionSensor_internal(visionSensorHandle,entityHandle,auxValues,auxValuesCount));
}
SIM_DLLEXPORT simFloat* simCheckVisionSensorEx(simInt visionSensorHandle,simInt entityHandle,simBool returnImage)
{
    return(simCheckVisionSensorEx_internal(visionSensorHandle,entityHandle,returnImage));
}
SIM_DLLEXPORT simUChar* simGetVisionSensorImg(simInt sensorHandle,simInt options,simFloat rgbaCutOff,const simInt* pos,const simInt* size,simInt* resolution)
{
    return(simGetVisionSensorImg_internal(sensorHandle,options,rgbaCutOff,pos,size,resolution));
}
SIM_DLLEXPORT simFloat* simGetVisionSensorDepth(simInt sensorHandle,simInt options,const simInt* pos,const simInt* size,simInt* resolution)
{
    return(simGetVisionSensorDepth_internal(sensorHandle,options,pos,size,resolution));
}
SIM_DLLEXPORT simInt simCreateDummy(simFloat size,const simFloat* reserved)
{
    return(simCreateDummy_internal(size,reserved));
}
SIM_DLLEXPORT simInt simCreateForceSensor(simInt options,const simInt* intParams,const simFloat* floatParams,const simFloat* reserved)
{
    return(simCreateForceSensor_internal(options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT simInt simCreateProximitySensor(simInt sensorType,simInt subType,simInt options,const simInt* intParams,const simFloat* floatParams,const simFloat* reserved)
{
    return(simCreateProximitySensor_internal(sensorType,subType,options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT simInt simCreateVisionSensor(simInt options,const simInt* intParams,const simFloat* floatParams,const simFloat* reserved)
{
    return(simCreateVisionSensor_internal(options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT simInt simConvexDecompose(simInt shapeHandle,simInt options,const simInt* intParams,const simFloat* floatParams)
{
    return(simConvexDecompose_internal(shapeHandle,options,intParams,floatParams));
}
SIM_DLLEXPORT simInt simCreateTexture(const simChar* fileName,simInt options,const simFloat* planeSizes,const simFloat* scalingUV,const simFloat* xy_g,simInt fixedResolution,simInt* textureId,simInt* resolution,const simVoid* reserved)
{
    return(simCreateTexture_internal(fileName,options,planeSizes,scalingUV,xy_g,fixedResolution,textureId,resolution,reserved));
}
SIM_DLLEXPORT simInt simWriteTexture(simInt textureId,simInt options,const simChar* data,simInt posX,simInt posY,simInt sizeX,simInt sizeY,simFloat interpol)
{
    return(simWriteTexture_internal(textureId,options,data,posX,posY,sizeX,sizeY,interpol));
}
SIM_DLLEXPORT simInt simGetShapeGeomInfo(simInt shapeHandle,simInt* intData,simFloat* floatData,simVoid* reserved)
{
    return(simGetShapeGeomInfo_internal(shapeHandle,intData,floatData,reserved));
}
SIM_DLLEXPORT simInt simScaleObject(simInt objectHandle,simFloat xScale,simFloat yScale,simFloat zScale,simInt options)
{
    return(simScaleObject_internal(objectHandle,xScale,yScale,zScale,options));
}
SIM_DLLEXPORT simInt simSetShapeTexture(simInt shapeHandle,simInt textureId,simInt mappingMode,simInt options,const simFloat* uvScaling,const simFloat* position,const simFloat* orientation)
{
    return(simSetShapeTexture_internal(shapeHandle,textureId,mappingMode,options,uvScaling,position,orientation));
}
SIM_DLLEXPORT simInt simTransformImage(simUChar* image,const simInt* resolution,simInt options,const simFloat* floatParams,const simInt* intParams,simVoid* reserved)
{
    return(simTransformImage_internal(image,resolution,options,floatParams,intParams,reserved));
}
SIM_DLLEXPORT simInt simGetQHull(const simFloat* inVertices,simInt inVerticesL,simFloat** verticesOut,simInt* verticesOutL,simInt** indicesOut,simInt* indicesOutL,simInt reserved1,const simFloat* reserved2)
{
    return(simGetQHull_internal(inVertices,inVerticesL,verticesOut,verticesOutL,indicesOut,indicesOutL,reserved1,reserved2));
}
SIM_DLLEXPORT simInt simGetDecimatedMesh(const simFloat* inVertices,simInt inVerticesL,const simInt* inIndices,simInt inIndicesL,simFloat** verticesOut,simInt* verticesOutL,simInt** indicesOut,simInt* indicesOutL,simFloat decimationPercent,simInt reserved1,const simFloat* reserved2)
{
    return(simGetDecimatedMesh_internal(inVertices,inVerticesL,inIndices,inIndicesL,verticesOut,verticesOutL,indicesOut,indicesOutL,decimationPercent,reserved1,reserved2));
}
SIM_DLLEXPORT simInt simComputeMassAndInertia(simInt shapeHandle,simFloat density)
{
    return(simComputeMassAndInertia_internal(shapeHandle,density));
}
SIM_DLLEXPORT simFloat simGetEngineFloatParam(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{
    return(simGetEngineFloatParam_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT simInt simSetEngineFloatParam(simInt paramId,simInt objectHandle,const simVoid* object,simFloat val)
{
    return(simSetEngineFloatParam_internal(paramId,objectHandle,object,val));
}
SIM_DLLEXPORT simInt simCreateOctree(simFloat voxelSize,simInt options,simFloat pointSize,simVoid* reserved)
{
    return(simCreateOctree_internal(voxelSize,options,pointSize,reserved));
}
SIM_DLLEXPORT simInt simCreatePointCloud(simFloat maxVoxelSize,simInt maxPtCntPerVoxel,simInt options,simFloat pointSize,simVoid* reserved)
{
    return(simCreatePointCloud_internal(maxVoxelSize,maxPtCntPerVoxel,options,pointSize,reserved));
}
SIM_DLLEXPORT simInt simSetPointCloudOptions(simInt pointCloudHandle,simFloat maxVoxelSize,simInt maxPtCntPerVoxel,simInt options,simFloat pointSize,simVoid* reserved)
{
    return(simSetPointCloudOptions_internal(pointCloudHandle,maxVoxelSize,maxPtCntPerVoxel,options,pointSize,reserved));
}
SIM_DLLEXPORT simInt simGetPointCloudOptions(simInt pointCloudHandle,simFloat* maxVoxelSize,simInt* maxPtCntPerVoxel,simInt* options,simFloat* pointSize,simVoid* reserved)
{
    return(simGetPointCloudOptions_internal(pointCloudHandle,maxVoxelSize,maxPtCntPerVoxel,options,pointSize,reserved));
}
SIM_DLLEXPORT simInt simInsertVoxelsIntoOctree(simInt octreeHandle,simInt options,const simFloat* pts,simInt ptCnt,const simUChar* color,const simUInt* tag,simVoid* reserved)
{
    return(simInsertVoxelsIntoOctree_internal(octreeHandle,options,pts,ptCnt,color,tag,reserved));
}
SIM_DLLEXPORT simInt simRemoveVoxelsFromOctree(simInt octreeHandle,simInt options,const simFloat* pts,simInt ptCnt,simVoid* reserved)
{
    return(simRemoveVoxelsFromOctree_internal(octreeHandle,options,pts,ptCnt,reserved));
}
SIM_DLLEXPORT simInt simInsertPointsIntoPointCloud(simInt pointCloudHandle,simInt options,const simFloat* pts,simInt ptCnt,const simUChar* color,simVoid* optionalValues)
{
    return(simInsertPointsIntoPointCloud_internal(pointCloudHandle,options,pts,ptCnt,color,optionalValues));
}
SIM_DLLEXPORT simInt simRemovePointsFromPointCloud(simInt pointCloudHandle,simInt options,const simFloat* pts,simInt ptCnt,simFloat tolerance,simVoid* reserved)
{
    return(simRemovePointsFromPointCloud_internal(pointCloudHandle,options,pts,ptCnt,tolerance,reserved));
}
SIM_DLLEXPORT simInt simIntersectPointsWithPointCloud(simInt pointCloudHandle,simInt options,const simFloat* pts,simInt ptCnt,simFloat tolerance,simVoid* reserved)
{
    return(simIntersectPointsWithPointCloud_internal(pointCloudHandle,options,pts,ptCnt,tolerance,reserved));
}
SIM_DLLEXPORT const simFloat* simGetOctreeVoxels(simInt octreeHandle,simInt* ptCnt,simVoid* reserved)
{
    return(simGetOctreeVoxels_internal(octreeHandle,ptCnt,reserved));
}
SIM_DLLEXPORT const simFloat* simGetPointCloudPoints(simInt pointCloudHandle,simInt* ptCnt,simVoid* reserved)
{
    return(simGetPointCloudPoints_internal(pointCloudHandle,ptCnt,reserved));
}
SIM_DLLEXPORT simInt simInsertObjectIntoPointCloud(simInt pointCloudHandle,simInt objectHandle,simInt options,simFloat gridSize,const simUChar* color,simVoid* optionalValues)
{
    return(simInsertObjectIntoPointCloud_internal(pointCloudHandle,objectHandle,options,gridSize,color,optionalValues));
}
SIM_DLLEXPORT simInt simSubtractObjectFromPointCloud(simInt pointCloudHandle,simInt objectHandle,simInt options,simFloat tolerance,simVoid* reserved)
{
    return(simSubtractObjectFromPointCloud_internal(pointCloudHandle,objectHandle,options,tolerance,reserved));
}
SIM_DLLEXPORT simInt simCheckOctreePointOccupancy(simInt octreeHandle,simInt options,const simFloat* points,simInt ptCnt,simUInt* tag,simUInt64* location,simVoid* reserved)
{
    return(simCheckOctreePointOccupancy_internal(octreeHandle,options,points,ptCnt,tag,location,reserved));
}
SIM_DLLEXPORT simInt simApplyTexture(simInt shapeHandle,const simFloat* textureCoordinates,simInt textCoordSize,const simUChar* texture,const simInt* textureResolution,simInt options)
{
    return(simApplyTexture_internal(shapeHandle,textureCoordinates,textCoordSize,texture,textureResolution,options));
}
SIM_DLLEXPORT simInt simSetJointDependency(simInt jointHandle,simInt masterJointHandle,simFloat offset,simFloat multCoeff)
{
    return(simSetJointDependency_internal(jointHandle,masterJointHandle,offset,multCoeff));
}
SIM_DLLEXPORT simInt simGetJointDependency(simInt jointHandle,simInt* masterJointHandle,simFloat* offset,simFloat* multCoeff)
{
    return(simGetJointDependency_internal(jointHandle,masterJointHandle,offset,multCoeff));
}
SIM_DLLEXPORT simInt simGetShapeMass(simInt shapeHandle,simFloat* mass)
{
    return(simGetShapeMass_internal(shapeHandle,mass));
}
SIM_DLLEXPORT simInt simSetShapeMass(simInt shapeHandle,simFloat mass)
{
    return(simSetShapeMass_internal(shapeHandle,mass));
}
SIM_DLLEXPORT simInt simGetShapeInertia(simInt shapeHandle,simFloat* inertiaMatrix,simFloat* transformationMatrix)
{
    return(simGetShapeInertia_internal(shapeHandle,inertiaMatrix,transformationMatrix));
}
SIM_DLLEXPORT simInt simSetShapeInertia(simInt shapeHandle,const simFloat* inertiaMatrix,const simFloat* transformationMatrix)
{
    return(simSetShapeInertia_internal(shapeHandle,inertiaMatrix,transformationMatrix));
}
SIM_DLLEXPORT simInt simGenerateShapeFromPath(const simFloat* path,simInt pathSize,const simFloat* section,simInt sectionSize,simInt options,const simFloat* upVector,simFloat reserved)
{
    return(simGenerateShapeFromPath_internal(path,pathSize,section,sectionSize,options,upVector,reserved));
}
SIM_DLLEXPORT simFloat simGetClosestPosOnPath(const simFloat* path,simInt pathSize,const simFloat* pathLengths,const simFloat* absPt)
{
    return(simGetClosestPosOnPath_internal(path,pathSize,pathLengths,absPt));
}
SIM_DLLEXPORT simVoid _simGetObjectLocalTransformation(const simVoid* object,simFloat* pos,simFloat* quat,simBool excludeFirstJointTransformation)
{
    return(_simGetObjectLocalTransformation_internal(object,pos,quat,excludeFirstJointTransformation));
}
SIM_DLLEXPORT simVoid _simSetObjectLocalTransformation(simVoid* object,const simFloat* pos,const simFloat* quat,simFloat simTime)
{
    return(_simSetObjectLocalTransformation_internal(object,pos,quat,simTime));
}
SIM_DLLEXPORT simVoid _simDynReportObjectCumulativeTransformation(simVoid* object,const simFloat* pos,const simFloat* quat,simFloat simTime)
{
    return(_simDynReportObjectCumulativeTransformation_internal(object,pos,quat,simTime));
}
SIM_DLLEXPORT simVoid _simSetObjectCumulativeTransformation(simVoid* object,const simFloat* pos,const simFloat* quat,simBool keepChildrenInPlace)
{
    return(_simSetObjectCumulativeTransformation_internal(object,pos,quat,keepChildrenInPlace));
}
SIM_DLLEXPORT simVoid _simGetObjectCumulativeTransformation(const simVoid* object,simFloat* pos,simFloat* quat,simBool excludeFirstJointTransformation)
{
    return(_simGetObjectCumulativeTransformation_internal(object,pos,quat,excludeFirstJointTransformation));
}
SIM_DLLEXPORT simVoid _simSetJointVelocity(const simVoid* joint,simFloat vel)
{
    return(_simSetJointVelocity_internal(joint,vel));
}
SIM_DLLEXPORT simVoid _simSetJointPosition(const simVoid* joint,simFloat pos)
{
    return(_simSetJointPosition_internal(joint,pos));
}
SIM_DLLEXPORT simFloat _simGetJointPosition(const simVoid* joint)
{
    return(_simGetJointPosition_internal(joint));
}
SIM_DLLEXPORT simVoid _simSetDynamicMotorPositionControlTargetPosition(const simVoid* joint,simFloat pos)
{
    return(_simSetDynamicMotorPositionControlTargetPosition_internal(joint,pos));
}
SIM_DLLEXPORT simVoid _simGetInitialDynamicVelocity(const simVoid* shape,simFloat* vel)
{
    return(_simGetInitialDynamicVelocity_internal(shape,vel));
}
SIM_DLLEXPORT simVoid _simSetInitialDynamicVelocity(simVoid* shape,const simFloat* vel)
{
    return(_simSetInitialDynamicVelocity_internal(shape,vel));
}
SIM_DLLEXPORT simVoid _simGetInitialDynamicAngVelocity(const simVoid* shape,simFloat* angularVel)
{
    return(_simGetInitialDynamicAngVelocity_internal(shape,angularVel));
}
SIM_DLLEXPORT simVoid _simSetInitialDynamicAngVelocity(simVoid* shape,const simFloat* angularVel)
{
    return(_simSetInitialDynamicAngVelocity_internal(shape,angularVel));
}
SIM_DLLEXPORT simVoid _simSetShapeDynamicVelocity(simVoid* shape,const simFloat* linear,const simFloat* angular,simFloat simTime)
{
    return(_simSetShapeDynamicVelocity_internal(shape,linear,angular,simTime));
}
SIM_DLLEXPORT simVoid _simGetAdditionalForceAndTorque(const simVoid* shape,simFloat* force,simFloat* torque)
{
    return(_simGetAdditionalForceAndTorque_internal(shape,force,torque));
}
SIM_DLLEXPORT simBool _simGetJointPositionInterval(const simVoid* joint,simFloat* minValue,simFloat* rangeValue)
{
    return(_simGetJointPositionInterval_internal(joint,minValue,rangeValue));
}
SIM_DLLEXPORT simFloat _simGetDynamicMotorTargetPosition(const simVoid* joint)
{
    return(_simGetDynamicMotorTargetPosition_internal(joint));
}
SIM_DLLEXPORT simFloat _simGetDynamicMotorTargetVelocity(const simVoid* joint)
{
    return(_simGetDynamicMotorTargetVelocity_internal(joint));
}
SIM_DLLEXPORT simFloat _simGetDynamicMotorMaxForce(const simVoid* joint)
{
    return(_simGetDynamicMotorMaxForce_internal(joint));
}
SIM_DLLEXPORT simFloat _simGetDynamicMotorUpperLimitVelocity(const simVoid* joint)
{
    return(_simGetDynamicMotorUpperLimitVelocity_internal(joint));
}
SIM_DLLEXPORT simVoid _simSetDynamicMotorReflectedPositionFromDynamicEngine(simVoid* joint,simFloat pos,simFloat simTime)
{
    return(_simSetDynamicMotorReflectedPositionFromDynamicEngine_internal(joint,pos,simTime));
}
SIM_DLLEXPORT simVoid _simSetJointSphericalTransformation(simVoid* joint,const simFloat* quat,simFloat simTime)
{
    return(_simSetJointSphericalTransformation_internal(joint,quat,simTime));
}
SIM_DLLEXPORT simVoid _simAddForceSensorCumulativeForcesAndTorques(simVoid* forceSensor,const simFloat* force,const simFloat* torque,int totalPassesCount,simFloat simTime)
{
    return(_simAddForceSensorCumulativeForcesAndTorques_internal(forceSensor,force,torque,totalPassesCount,simTime));
}
SIM_DLLEXPORT simVoid _simAddJointCumulativeForcesOrTorques(simVoid* joint,simFloat forceOrTorque,int totalPassesCount,simFloat simTime)
{
    return(_simAddJointCumulativeForcesOrTorques_internal(joint,forceOrTorque,totalPassesCount,simTime));
}
SIM_DLLEXPORT simFloat _simGetLocalInertiaInfo(const simVoid* object,simFloat* pos,simFloat* quat,simFloat* diagI)
{
    return(_simGetLocalInertiaInfo_internal(object,pos,quat,diagI));
}
SIM_DLLEXPORT simFloat _simGetMass(const simVoid* geomInfo)
{
    return(_simGetMass_internal(geomInfo));
}
SIM_DLLEXPORT simVoid _simGetPurePrimitiveSizes(const simVoid* geometric,simFloat* sizes)
{
    return(_simGetPurePrimitiveSizes_internal(geometric,sizes));
}
SIM_DLLEXPORT simVoid _simGetVerticesLocalFrame(const simVoid* geometric,simFloat* pos,simFloat* quat)
{
    return(_simGetVerticesLocalFrame_internal(geometric,pos,quat));
}
SIM_DLLEXPORT const simFloat* _simGetHeightfieldData(const simVoid* geometric,simInt* xCount,simInt* yCount,simFloat* minHeight,simFloat* maxHeight)
{
    return(_simGetHeightfieldData_internal(geometric,xCount,yCount,minHeight,maxHeight));
}
SIM_DLLEXPORT simVoid _simGetCumulativeMeshes(const simVoid* geomInfo,simFloat** vertices,simInt* verticesSize,simInt** indices,simInt* indicesSize)
{
    return(_simGetCumulativeMeshes_internal(geomInfo,vertices,verticesSize,indices,indicesSize));
}
SIM_DLLEXPORT simVoid _simGetGravity(simFloat* gravity)
{
    return(_simGetGravity_internal(gravity));
}
SIM_DLLEXPORT simBool _simGetDistanceBetweenEntitiesIfSmaller(simInt entity1ID,simInt entity2ID,simFloat* distance,simFloat* ray,simInt* cacheBuffer,simBool overrideMeasurableFlagIfNonCollection1,simBool overrideMeasurableFlagIfNonCollection2,simBool pathPlanningRoutineCalling)
{
    return(_simGetDistanceBetweenEntitiesIfSmaller_internal(entity1ID,entity2ID,distance,ray,cacheBuffer,overrideMeasurableFlagIfNonCollection1,overrideMeasurableFlagIfNonCollection2,pathPlanningRoutineCalling));
}
SIM_DLLEXPORT simInt _simHandleJointControl(const simVoid* joint,simInt auxV,const simInt* inputValuesInt,const simFloat* inputValuesFloat,simFloat* outputValues)
{
    return(_simHandleJointControl_internal(joint,auxV,inputValuesInt,inputValuesFloat,outputValues));
}
SIM_DLLEXPORT simInt _simHandleCustomContact(simInt objHandle1,simInt objHandle2,simInt engine,simInt* dataInt,simFloat* dataFloat)
{
    return(_simHandleCustomContact_internal(objHandle1,objHandle2,engine,dataInt,dataFloat));
}
SIM_DLLEXPORT simFloat _simGetPureHollowScaling(const simVoid* geometric)
{
    return(_simGetPureHollowScaling_internal(geometric));
}
SIM_DLLEXPORT simVoid _simDynCallback(const simInt* intData,const simFloat* floatData)
{
    _simDynCallback_internal(intData,floatData);
}
SIM_DLLEXPORT simInt simExtCallScriptFunction(simInt scriptHandleOrType, const simChar* functionNameAtScriptName,
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
#endif

// Deprecated begin
SIM_DLLEXPORT simInt simGetMaterialId(const simChar* materialName)
{
    return(simGetMaterialId_internal(materialName));
}
SIM_DLLEXPORT simInt simGetShapeMaterial(simInt shapeHandle)
{
    return(simGetShapeMaterial_internal(shapeHandle));
}
SIM_DLLEXPORT simInt simHandleVarious()
{
    return(simHandleVarious_internal());
}
SIM_DLLEXPORT simInt simSerialPortOpen(simInt portNumber,simInt baudRate,simVoid* reserved1,simVoid* reserved2)
{
    return(simSerialPortOpen_internal(portNumber,baudRate,reserved1,reserved2));
}
SIM_DLLEXPORT simInt simSerialPortClose(simInt portNumber)
{
    return(simSerialPortClose_internal(portNumber));
}
SIM_DLLEXPORT simInt simSerialPortSend(simInt portNumber,const simChar* data,simInt dataLength)
{
    return(simSerialPortSend_internal(portNumber,data,dataLength));
}
SIM_DLLEXPORT simInt simSerialPortRead(simInt portNumber,simChar* buffer,simInt dataLengthToRead)
{
    return(simSerialPortRead_internal(portNumber,buffer,dataLengthToRead));
}
SIM_DLLEXPORT simInt simGetPathPlanningHandle(const simChar* pathPlanningObjectName)
{
    return(simGetPathPlanningHandle_internal(pathPlanningObjectName));
}
SIM_DLLEXPORT simInt simGetMotionPlanningHandle(const simChar* motionPlanningObjectName)
{
    return(simGetMotionPlanningHandle_internal(motionPlanningObjectName));
}
SIM_DLLEXPORT simInt simRemoveMotionPlanning(simInt motionPlanningHandle)
{
    return(simRemoveMotionPlanning_internal(motionPlanningHandle));
}
SIM_DLLEXPORT simInt simPerformPathSearchStep(simInt temporaryPathSearchObject,simBool abortSearch)
{
    return(simPerformPathSearchStep_internal(temporaryPathSearchObject,abortSearch));
}
SIM_DLLEXPORT simInt simLockInterface(simBool locked)
{
    return(simLockInterface_internal(locked));
}
SIM_DLLEXPORT simInt simCopyPasteSelectedObjects()
{
    return(simCopyPasteSelectedObjects_internal());
}
SIM_DLLEXPORT simInt simResetPath(simInt pathHandle)
{
    return(simResetPath_internal(pathHandle));
}
SIM_DLLEXPORT simInt simResetJoint(simInt jointHandle)
{
    return(simResetJoint_internal(jointHandle));
}
SIM_DLLEXPORT simInt simAppendScriptArrayEntry(const simChar* reservedSetToNull,simInt scriptHandleOrType,const simChar* arrayNameAtScriptName,const simChar* keyName,const simChar* data,const simInt* what)
{
    return(simAppendScriptArrayEntry_internal(reservedSetToNull,scriptHandleOrType,arrayNameAtScriptName,keyName,data,what));
}
SIM_DLLEXPORT simInt simClearScriptVariable(const simChar* reservedSetToNull,simInt scriptHandleOrType,const simChar* variableNameAtScriptName)
{
    return(simClearScriptVariable_internal(reservedSetToNull,scriptHandleOrType,variableNameAtScriptName));
}
SIM_DLLEXPORT simBool _simGetBulletStickyContact(const simVoid* geomInfo)
{
    return(_simGetBulletStickyContact_internal(geomInfo));
}
SIM_DLLEXPORT simInt simAddSceneCustomData(simInt header,const simChar* data,simInt dataLength)
{
    return(simAddSceneCustomData_internal(header,data,dataLength));
}
SIM_DLLEXPORT simInt simGetSceneCustomDataLength(simInt header)
{
    return(simGetSceneCustomDataLength_internal(header));
}
SIM_DLLEXPORT simInt simGetSceneCustomData(simInt header,simChar* data)
{
    return(simGetSceneCustomData_internal(header,data));
}
SIM_DLLEXPORT simInt simAddObjectCustomData(simInt objectHandle,simInt header,const simChar* data,simInt dataLength)
{
    return(simAddObjectCustomData_internal(objectHandle,header,data,dataLength));
}
SIM_DLLEXPORT simInt simGetObjectCustomDataLength(simInt objectHandle,simInt header)
{
    return(simGetObjectCustomDataLength_internal(objectHandle,header));
}
SIM_DLLEXPORT simInt simGetObjectCustomData(simInt objectHandle,simInt header,simChar* data)
{
    return(simGetObjectCustomData_internal(objectHandle,header,data));
}
SIM_DLLEXPORT simInt simCreateUI(const simChar* uiName,simInt menuAttributes,const simInt* clientSize,const simInt* cellSize,simInt* buttonHandles)
{
    return(simCreateUI_internal(uiName,menuAttributes,clientSize,cellSize,buttonHandles));
}
SIM_DLLEXPORT simInt simCreateUIButton(simInt uiHandle,const simInt* position,const simInt* size,simInt buttonProperty)
{
    return(simCreateUIButton_internal(uiHandle,position,size,buttonProperty));
}
SIM_DLLEXPORT simInt simGetUIHandle(const simChar* uiName)
{
    return(simGetUIHandle_internal(uiName));
}
SIM_DLLEXPORT simInt simGetUIProperty(simInt uiHandle)
{
    return(simGetUIProperty_internal(uiHandle));
}
SIM_DLLEXPORT simInt simGetUIEventButton(simInt uiHandle,simInt* auxiliaryValues)
{
    return(simGetUIEventButton_internal(uiHandle,auxiliaryValues));
}
SIM_DLLEXPORT simInt simSetUIProperty(simInt uiHandle,simInt elementProperty)
{
    return(simSetUIProperty_internal(uiHandle,elementProperty));
}
SIM_DLLEXPORT simInt simGetUIButtonProperty(simInt uiHandle,simInt buttonHandle)
{
    return(simGetUIButtonProperty_internal(uiHandle,buttonHandle));
}
SIM_DLLEXPORT simInt simSetUIButtonProperty(simInt uiHandle,simInt buttonHandle,simInt buttonProperty)
{
    return(simSetUIButtonProperty_internal(uiHandle,buttonHandle,buttonProperty));
}
SIM_DLLEXPORT simInt simGetUIButtonSize(simInt uiHandle,simInt buttonHandle,simInt* size)
{
    return(simGetUIButtonSize_internal(uiHandle,buttonHandle,size));
}
SIM_DLLEXPORT simInt simSetUIButtonLabel(simInt uiHandle,simInt buttonHandle,const simChar* upStateLabel,const simChar* downStateLabel)
{
    return(simSetUIButtonLabel_internal(uiHandle,buttonHandle,upStateLabel,downStateLabel));
}
SIM_DLLEXPORT simChar* simGetUIButtonLabel(simInt uiHandle,simInt buttonHandle)
{
    return(simGetUIButtonLabel_internal(uiHandle,buttonHandle));
}
SIM_DLLEXPORT simInt simSetUISlider(simInt uiHandle,simInt buttonHandle,simInt position)
{
    return(simSetUISlider_internal(uiHandle,buttonHandle,position));
}
SIM_DLLEXPORT simInt simGetUISlider(simInt uiHandle,simInt buttonHandle)
{
    return(simGetUISlider_internal(uiHandle,buttonHandle));
}
SIM_DLLEXPORT simInt simSetUIButtonTexture(simInt uiHandle,simInt buttonHandle,const simInt* size,const simChar* textureData)
{
    return(simSetUIButtonTexture_internal(uiHandle,buttonHandle,size,textureData));
}
SIM_DLLEXPORT simInt simCreateUIButtonArray(simInt uiHandle,simInt buttonHandle)
{
    return(simCreateUIButtonArray_internal(uiHandle,buttonHandle));
}
SIM_DLLEXPORT simInt simDeleteUIButtonArray(simInt uiHandle,simInt buttonHandle)
{
    return(simDeleteUIButtonArray_internal(uiHandle,buttonHandle));
}
SIM_DLLEXPORT simInt simRemoveUI(simInt uiHandle)
{
    return(simRemoveUI_internal(uiHandle));
}
SIM_DLLEXPORT simInt simSetUIPosition(simInt uiHandle,const simInt* position)
{
    return(simSetUIPosition_internal(uiHandle,position));
}
SIM_DLLEXPORT simInt simGetUIPosition(simInt uiHandle,simInt* position)
{
    return(simGetUIPosition_internal(uiHandle,position));
}
SIM_DLLEXPORT simInt simLoadUI(const simChar* filename,int maxCount,int* uiHandles)
{
    return(simLoadUI_internal(filename,maxCount,uiHandles));
}
SIM_DLLEXPORT simInt simSaveUI(int count,const int* uiHandles,const simChar* filename)
{
    return(simSaveUI_internal(count,uiHandles,filename));
}
SIM_DLLEXPORT simInt simHandleGeneralCallbackScript(simInt callbackId,simInt callbackTag,simVoid* additionalData)
{
    return(simHandleGeneralCallbackScript_internal(callbackId,callbackTag,additionalData));
}
SIM_DLLEXPORT simInt simRegisterCustomLuaFunction(const simChar* funcName,const simChar* callTips,const simInt* inputArgumentTypes,simVoid(*callBack)(struct SLuaCallBack* p))
{
    return(simRegisterCustomLuaFunction_internal(funcName,callTips,inputArgumentTypes,callBack));
}
SIM_DLLEXPORT simInt simRegisterCustomLuaVariable(const simChar* varName,const simChar* varValue)
{
    return(simRegisterScriptVariable_internal(varName,varValue,0));
}
SIM_DLLEXPORT simInt simGetMechanismHandle(const simChar* mechanismName)
{
    return(simGetMechanismHandle_internal(mechanismName));
}
SIM_DLLEXPORT simInt simHandleMechanism(simInt mechanismHandle)
{
    return(simHandleMechanism_internal(mechanismHandle));
}
SIM_DLLEXPORT simInt simHandleCustomizationScripts(simInt callType)
{
    return(simHandleCustomizationScripts_internal(callType));
}
SIM_DLLEXPORT simInt simCallScriptFunction(simInt scriptHandleOrType,const simChar* functionNameAtScriptName,SLuaCallBack* data,const simChar* reservedSetToNull)
{
    return(simCallScriptFunction_internal(scriptHandleOrType,functionNameAtScriptName,data,reservedSetToNull));
}
SIM_DLLEXPORT simInt _simGetJointDynCtrlMode(const simVoid* joint)
{
    return(_simGetJointDynCtrlMode_internal(joint));
}
SIM_DLLEXPORT simChar* simGetScriptSimulationParameter(simInt scriptHandle,const simChar* parameterName,simInt* parameterLength)
{
    return(simGetScriptSimulationParameter_internal(scriptHandle,parameterName,parameterLength));
}
SIM_DLLEXPORT simInt simSetScriptSimulationParameter(simInt scriptHandle,const simChar* parameterName,const simChar* parameterValue,simInt parameterLength)
{
    return(simSetScriptSimulationParameter_internal(scriptHandle,parameterName,parameterValue,parameterLength));
}
SIM_DLLEXPORT simInt simResetMill(simInt millHandle)
{
    return(-1);
}
SIM_DLLEXPORT simInt simResetMilling(simInt objectHandle)
{
    return(-1);
}
SIM_DLLEXPORT simInt simApplyMilling(simInt objectHandle)
{
    return(-1);
}
SIM_DLLEXPORT simBool _simGetParentFollowsDynamic(const simVoid* shape)
{
    return(false);
}
SIM_DLLEXPORT simInt simGetNameSuffix(const simChar* name)
{
    return(simGetNameSuffix_internal(name));
}
SIM_DLLEXPORT simInt simSetNameSuffix(simInt nameSuffixNumber)
{
    return(simSetNameSuffix_internal(nameSuffixNumber));
}
SIM_DLLEXPORT simInt simAddStatusbarMessage(const simChar* message)
{
    return(simAddStatusbarMessage_internal(message));
}
SIM_DLLEXPORT simChar* simGetScriptRawBuffer(simInt scriptHandle,simInt bufferHandle)
{
    return(simGetScriptRawBuffer_internal(scriptHandle,bufferHandle));
}
SIM_DLLEXPORT simInt simSetScriptRawBuffer(simInt scriptHandle,const simChar* buffer,simInt bufferSize)
{
    return(simSetScriptRawBuffer_internal(scriptHandle,buffer,bufferSize));
}
SIM_DLLEXPORT simInt simReleaseScriptRawBuffer(simInt scriptHandle,simInt bufferHandle)
{
    return(simReleaseScriptRawBuffer_internal(scriptHandle,bufferHandle));
}
SIM_DLLEXPORT simInt simRemoveIkGroup(simInt ikGroupHandle)
{
    return(simRemoveIkGroup_internal(ikGroupHandle));
}
SIM_DLLEXPORT simInt simExportIk(const simChar* pathAndFilename,simInt reserved1,simVoid* reserved2)
{
    return(simExportIk_internal(pathAndFilename,reserved1,reserved2));
}
SIM_DLLEXPORT simInt simComputeJacobian(simInt ikGroupHandle,simInt options,simVoid* reserved)
{
    return(simComputeJacobian_internal(ikGroupHandle,options,reserved));
}
SIM_DLLEXPORT simInt simGetIkGroupHandle(const simChar* ikGroupName)
{
    return(simGetIkGroupHandle_internal(ikGroupName));
}
SIM_DLLEXPORT simInt simHandleIkGroup(simInt ikGroupHandle)
{
    return(simHandleIkGroup_internal(ikGroupHandle));
}
SIM_DLLEXPORT simInt simSetThreadIsFree(simBool freeMode)
{
    return(simSetThreadIsFree_internal(freeMode));
}
SIM_DLLEXPORT simInt simTubeOpen(simInt dataHeader,const simChar* dataName,simInt readBufferSize,simBool notUsedButKeepZero)
{
    return(simTubeOpen_internal(dataHeader,dataName,readBufferSize,notUsedButKeepZero));
}
SIM_DLLEXPORT simInt simTubeClose(simInt tubeHandle)
{
    return(simTubeClose_internal(tubeHandle));
}
SIM_DLLEXPORT simInt simTubeWrite(simInt tubeHandle,const simChar* data,simInt dataLength)
{
    return(simTubeWrite_internal(tubeHandle,data,dataLength));
}
SIM_DLLEXPORT simChar* simTubeRead(simInt tubeHandle,simInt* dataLength)
{
    return(simTubeRead_internal(tubeHandle,dataLength));
}
SIM_DLLEXPORT simInt simTubeStatus(simInt tubeHandle,simInt* readPacketsCount,simInt* writePacketsCount)
{
    return(simTubeStatus_internal(tubeHandle,readPacketsCount,writePacketsCount));
}
SIM_DLLEXPORT simInt simInsertPathCtrlPoints(simInt pathHandle,simInt options,simInt startIndex,simInt ptCnt,const simVoid* ptData)
{
    return(simInsertPathCtrlPoints_internal(pathHandle,options,startIndex,ptCnt,ptData));
}
SIM_DLLEXPORT simInt simCutPathCtrlPoints(simInt pathHandle,simInt startIndex,simInt ptCnt)
{
    return(simCutPathCtrlPoints_internal(pathHandle,startIndex,ptCnt));
}
SIM_DLLEXPORT simInt simGetThreadId()
{
    return(simGetThreadId_internal());
}
SIM_DLLEXPORT simInt simSwitchThread()
{
    return(simSwitchThread_internal());
}
SIM_DLLEXPORT simInt simLockResources(simInt lockType,simInt reserved)
{
    return(simLockResources_internal(lockType,reserved));
}
SIM_DLLEXPORT simInt simUnlockResources(simInt lockHandle)
{
    return(simUnlockResources_internal(lockHandle));
}
SIM_DLLEXPORT simChar* simGetUserParameter(simInt objectHandle,const simChar* parameterName,simInt* parameterLength)
{
    return(simGetUserParameter_internal(objectHandle,parameterName,parameterLength));
}
SIM_DLLEXPORT simInt simSetUserParameter(simInt objectHandle,const simChar* parameterName,const simChar* parameterValue,simInt parameterLength)
{
    return(simSetUserParameter_internal(objectHandle,parameterName,parameterValue,parameterLength));
}
SIM_DLLEXPORT simInt simGetCollectionHandle(const simChar* collectionName)
{
    return(simGetCollectionHandle_internal(collectionName));
}
SIM_DLLEXPORT simInt simRemoveCollection(simInt collectionHandle)
{
    return(simRemoveCollection_internal(collectionHandle));
}
SIM_DLLEXPORT simInt simEmptyCollection(simInt collectionHandle)
{
    return(simEmptyCollection_internal(collectionHandle));
}
SIM_DLLEXPORT simChar* simGetCollectionName(simInt collectionHandle)
{
    return(simGetCollectionName_internal(collectionHandle));
}
SIM_DLLEXPORT simInt simSetCollectionName(simInt collectionHandle,const simChar* collectionName)
{
    return(simSetCollectionName_internal(collectionHandle,collectionName));
}
SIM_DLLEXPORT simInt simCreateCollection(const simChar* collectionName,simInt options)
{
    return(simCreateCollection_internal(collectionName,options));
}
SIM_DLLEXPORT simInt simAddObjectToCollection(simInt collectionHandle,simInt objectHandle,simInt what,simInt options)
{
    return(simAddObjectToCollection_internal(collectionHandle,objectHandle,what,options));
}
SIM_DLLEXPORT simInt simGetCollisionHandle(const simChar* collisionObjectName)
{
    return(simGetCollisionHandle_internal(collisionObjectName));
}
SIM_DLLEXPORT simInt simGetDistanceHandle(const simChar* distanceObjectName)
{
    return(simGetDistanceHandle_internal(distanceObjectName));
}
SIM_DLLEXPORT simInt simResetCollision(simInt collisionObjectHandle)
{
    return(simResetCollision_internal(collisionObjectHandle));
}
SIM_DLLEXPORT simInt simResetDistance(simInt distanceObjectHandle)
{
    return(simResetDistance_internal(distanceObjectHandle));
}
SIM_DLLEXPORT simInt simHandleCollision(simInt collisionObjectHandle)
{
    return(simHandleCollision_internal(collisionObjectHandle));
}
SIM_DLLEXPORT simInt simReadCollision(simInt collisionObjectHandle)
{
    return(simReadCollision_internal(collisionObjectHandle));
}
SIM_DLLEXPORT simInt simRemoveBanner(simInt bannerID)
{
    return(simRemoveBanner_internal(bannerID));
}
SIM_DLLEXPORT simInt simGetObjectIntParameter(simInt objectHandle,simInt ParamID,simInt* Param)
{
    return(simGetObjectInt32Param_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT simInt simSetObjectIntParameter(simInt objectHandle,simInt ParamID,simInt Param)
{
    return(simSetObjectInt32Param_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT simInt simGetObjectInt32Parameter(simInt objectHandle,simInt ParamID,simInt* Param)
{
    return(simGetObjectInt32Param_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT simInt simSetObjectInt32Parameter(simInt objectHandle,simInt ParamID,simInt Param)
{
    return(simSetObjectInt32Param_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT simChar* simGetObjectStringParameter(simInt objectHandle,simInt ParamID,simInt* ParamLength)
{
    return(simGetObjectStringParam_internal(objectHandle,ParamID,ParamLength));
}
SIM_DLLEXPORT simInt simSetObjectStringParameter(simInt objectHandle,simInt ParamID,const simChar* Param,simInt ParamLength)
{
    return(simSetObjectStringParam_internal(objectHandle,ParamID,Param,ParamLength));
}
SIM_DLLEXPORT simInt simSetBooleanParameter(simInt parameter,simBool boolState)
{
    return(simSetBoolParam_internal(parameter,boolState));
}
SIM_DLLEXPORT simInt simGetBooleanParameter(simInt parameter)
{
    return(simGetBoolParam_internal(parameter));
}
SIM_DLLEXPORT simInt simSetBoolParameter(simInt parameter,simBool boolState)
{
    return(simSetBoolParam_internal(parameter,boolState));
}
SIM_DLLEXPORT simInt simGetBoolParameter(simInt parameter)
{
    return(simGetBoolParam_internal(parameter));
}
SIM_DLLEXPORT simInt simSetIntegerParameter(simInt parameter,simInt intState)
{
    return(simSetInt32Param_internal(parameter,intState));
}
SIM_DLLEXPORT simInt simGetIntegerParameter(simInt parameter,simInt* intState)
{
    return(simGetInt32Param_internal(parameter,intState));
}
SIM_DLLEXPORT simInt simSetInt32Parameter(simInt parameter,simInt intState)
{
    return(simSetInt32Param_internal(parameter,intState));
}
SIM_DLLEXPORT simInt simGetInt32Parameter(simInt parameter,simInt* intState)
{
    return(simGetInt32Param_internal(parameter,intState));
}
SIM_DLLEXPORT simInt simGetUInt64Parameter(simInt parameter,simUInt64* intState)
{
    return(simGetUInt64Param_internal(parameter,intState));
}
SIM_DLLEXPORT simInt simSetStringParameter(simInt parameter,const simChar* str)
{
    return(simSetStringParam_internal(parameter,str));
}
SIM_DLLEXPORT simChar* simGetStringParameter(simInt parameter)
{
    return(simGetStringParam_internal(parameter));
}
SIM_DLLEXPORT simInt simSetArrayParameter(simInt parameter,const simVoid* arrayOfValues)
{
    return(simSetArrayParam_internal(parameter,arrayOfValues));
}
SIM_DLLEXPORT simInt simGetArrayParameter(simInt parameter,simVoid* arrayOfValues)
{
    return(simGetArrayParam_internal(parameter,arrayOfValues));
}
SIM_DLLEXPORT simInt simGetEngineInt32Parameter(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{
    return(simGetEngineInt32Param_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT simBool simGetEngineBoolParameter(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{
    return(simGetEngineBoolParam_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT simInt simSetEngineInt32Parameter(simInt paramId,simInt objectHandle,const simVoid* object,simInt val)
{
    return(simSetEngineInt32Param_internal(paramId,objectHandle,object,val));
}
SIM_DLLEXPORT simInt simSetEngineBoolParameter(simInt paramId,simInt objectHandle,const simVoid* object,simBool val)
{
    return(simSetEngineBoolParam_internal(paramId,objectHandle,object,val));
}
SIM_DLLEXPORT simInt simIsHandleValid(simInt generalObjectHandle,simInt generalObjectType)
{
    return(simIsHandle_internal(generalObjectHandle,generalObjectType));
}
SIM_DLLEXPORT simInt simAddModuleMenuEntry(const simChar* entryLabel,simInt itemCount,simInt* itemHandles)
{
    return(simAddModuleMenuEntry_internal(entryLabel,itemCount,itemHandles));
}
SIM_DLLEXPORT simInt simSetModuleMenuItemState(simInt itemHandle,simInt state,const simChar* label)
{
    return(simSetModuleMenuItemState_internal(itemHandle,state,label));
}
SIM_DLLEXPORT simInt simSetIntegerSignal(const simChar* signalName,simInt signalValue)
{
    return(simSetInt32Signal_internal(signalName,signalValue));
}
SIM_DLLEXPORT simInt simGetIntegerSignal(const simChar* signalName,simInt* signalValue)
{
    return(simGetInt32Signal_internal(signalName,signalValue));
}
SIM_DLLEXPORT simInt simClearIntegerSignal(const simChar* signalName)
{
    return(simClearInt32Signal_internal(signalName));
}
SIM_DLLEXPORT simChar* simGetObjectName(simInt objectHandle)
{
    return(simGetObjectName_internal(objectHandle));
}
SIM_DLLEXPORT simInt simSetObjectName(simInt objectHandle,const simChar* objectName)
{
    return(simSetObjectName_internal(objectHandle,objectName));
}
SIM_DLLEXPORT simChar* simGetScriptName(simInt scriptHandle)
{
    return(simGetScriptName_internal(scriptHandle));
}
SIM_DLLEXPORT simInt simGetScriptHandle(const simChar* scriptName)
{
    return(simGetScriptHandle_internal(scriptName));
}
SIM_DLLEXPORT simInt simSetScriptVariable(simInt scriptHandleOrType,const simChar* variableNameAtScriptName,simInt stackHandle)
{
    return(simSetScriptVariable_internal(scriptHandleOrType,variableNameAtScriptName,stackHandle));
}
SIM_DLLEXPORT simInt simGetObjectHandle(const simChar* objectAlias)
{
    return(simGetObjectHandleEx_internal(objectAlias,-1,-1,0));
}
SIM_DLLEXPORT simInt simGetObjectHandleEx(const simChar* objectAlias,simInt index,simInt proxy,simInt options)
{
    return(simGetObjectHandleEx_internal(objectAlias,index,proxy,options));
}
SIM_DLLEXPORT simInt simGetScript(simInt index)
{
    return(simGetScript_internal(index));
}
SIM_DLLEXPORT simInt simGetScriptAssociatedWithObject(simInt objectHandle)
{
    return(simGetScriptAssociatedWithObject_internal(objectHandle));
}
SIM_DLLEXPORT simInt simGetCustomizationScriptAssociatedWithObject(simInt objectHandle)
{
    return(simGetCustomizationScriptAssociatedWithObject_internal(objectHandle));
}
SIM_DLLEXPORT simInt simGetObjectAssociatedWithScript(simInt scriptHandle)
{
    return(simGetObjectAssociatedWithScript_internal(scriptHandle));
}
SIM_DLLEXPORT simChar* simGetObjectConfiguration(simInt objectHandle)
{
    return(simGetObjectConfiguration_internal(objectHandle));
}
SIM_DLLEXPORT simInt simSetObjectConfiguration(const simChar* data)
{
    return(simSetObjectConfiguration_internal(data));
}
SIM_DLLEXPORT simChar* simGetConfigurationTree(simInt objectHandle)
{
    return(simGetConfigurationTree_internal(objectHandle));
}
SIM_DLLEXPORT simInt simSetConfigurationTree(const simChar* data)
{
    return(simSetConfigurationTree_internal(data));
}
SIM_DLLEXPORT simInt simEnableEventCallback(simInt eventCallbackType,const simChar* plugin,simInt reserved)
{
    return(simEnableEventCallback_internal(eventCallbackType,plugin,reserved));
}
SIM_DLLEXPORT simInt simRMLPosition(simInt dofs,simDouble timeStep,simInt flags,const simDouble* currentPosVelAccel,const simDouble* maxVelAccelJerk,const simBool* selection,const simDouble* targetPosVel,simDouble* newPosVelAccel,simVoid* auxData)
{
    return(simRMLPosition_internal(dofs,timeStep,flags,currentPosVelAccel,maxVelAccelJerk,selection,targetPosVel,newPosVelAccel,auxData));
}
SIM_DLLEXPORT simInt simRMLVelocity(simInt dofs,simDouble timeStep,simInt flags,const simDouble* currentPosVelAccel,const simDouble* maxAccelJerk,const simBool* selection,const simDouble* targetVel,simDouble* newPosVelAccel,simVoid* auxData)
{
    return(simRMLVelocity_internal(dofs,timeStep,flags,currentPosVelAccel,maxAccelJerk,selection,targetVel,newPosVelAccel,auxData));
}
SIM_DLLEXPORT simInt simRMLPos(simInt dofs,simDouble smallestTimeStep,simInt flags,const simDouble* currentPosVelAccel,const simDouble* maxVelAccelJerk,const simBool* selection,const simDouble* targetPosVel,simVoid* auxData)
{
    return(simRMLPos_internal(dofs,smallestTimeStep,flags,currentPosVelAccel,maxVelAccelJerk,selection,targetPosVel,auxData));
}
SIM_DLLEXPORT simInt simRMLVel(simInt dofs,simDouble smallestTimeStep,simInt flags,const simDouble* currentPosVelAccel,const simDouble* maxAccelJerk,const simBool* selection,const simDouble* targetVel,simVoid* auxData)
{
    return(simRMLVel_internal(dofs,smallestTimeStep,flags,currentPosVelAccel,maxAccelJerk,selection,targetVel,auxData));
}
SIM_DLLEXPORT simInt simRMLStep(simInt handle,simDouble timeStep,simDouble* newPosVelAccel,simVoid* auxData,simVoid* reserved)
{
    return(simRMLStep_internal(handle,timeStep,newPosVelAccel,auxData,reserved));
}
SIM_DLLEXPORT simInt simRMLRemove(simInt handle)
{
    return(simRMLRemove_internal(handle));
}
SIM_DLLEXPORT simInt simGetSystemTimeInMilliseconds()
{
    return(simGetSystemTimeInMs_internal(-2));
}
SIM_DLLEXPORT simUInt simGetSystemTimeInMs(simInt previousTime)
{
    return(simGetSystemTimeInMs_internal(previousTime));
}
SIM_DLLEXPORT simChar* simFileDialog(simInt mode,const simChar* title,const simChar* startPath,const simChar* initName,const simChar* extName,const simChar* ext)
{
    return(simFileDialog_internal(mode,title,startPath,initName,extName,ext));
}
SIM_DLLEXPORT simInt simMsgBox(simInt dlgType,simInt buttons,const simChar* title,const simChar* message)
{
    return(simMsgBox_internal(dlgType,buttons,title,message));
}
SIM_DLLEXPORT simInt simGetDialogResult(simInt genericDialogHandle)
{
    return(simGetDialogResult_internal(genericDialogHandle));
}
SIM_DLLEXPORT simChar* simGetDialogInput(simInt genericDialogHandle)
{
    return(simGetDialogInput_internal(genericDialogHandle));
}
SIM_DLLEXPORT simInt simEndDialog(simInt genericDialogHandle)
{
    return(simEndDialog_internal(genericDialogHandle));
}
SIM_DLLEXPORT simInt simIsObjectInSelection(simInt objectHandle)
{
    return(simIsObjectInSelection_internal(objectHandle));
}
SIM_DLLEXPORT simInt simAddObjectToSelection(simInt what,simInt objectHandle)
{
    return(simAddObjectToSelection_internal(what,objectHandle));
}
SIM_DLLEXPORT simInt simRemoveObjectFromSelection(simInt what,simInt objectHandle)
{
    return(simRemoveObjectFromSelection_internal(what,objectHandle));
}
SIM_DLLEXPORT simInt simGetObjectSelectionSize()
{
    return(simGetObjectSelectionSize_internal());
}
SIM_DLLEXPORT simInt simGetObjectLastSelection()
{
    return(simGetObjectLastSelection_internal());
}
SIM_DLLEXPORT simInt simGetObjectSelection(simInt* objectHandles)
{
    return(simGetObjectSelection_internal(objectHandles));
}
SIM_DLLEXPORT simInt simDeleteSelectedObjects()
{
    return(simDeleteSelectedObjects_internal());
}
SIM_DLLEXPORT simInt simSetStringNamedParam(const simChar* paramName,const simChar* stringParam,simInt paramLength)
{
    return(simSetNamedStringParam_internal(paramName,stringParam,paramLength));
}
SIM_DLLEXPORT simChar* simGetStringNamedParam(const simChar* paramName,simInt* paramLength)
{
    return(simGetNamedStringParam_internal(paramName,paramLength));
}
SIM_DLLEXPORT simInt simGetObjectUniqueIdentifier(simInt objectHandle,simInt* uniqueIdentifier)
{
    return(simGetObjectUniqueIdentifier_internal(objectHandle,uniqueIdentifier));
}
SIM_DLLEXPORT simVoid _simSetDynamicJointLocalTransformationPart2IsValid(simVoid* joint,simBool valid)
{
    return(_simSetDynamicJointLocalTransformationPart2IsValid_internal(joint,valid));
}
SIM_DLLEXPORT simVoid _simSetDynamicForceSensorLocalTransformationPart2IsValid(simVoid* forceSensor,simBool valid)
{
    return(_simSetDynamicForceSensorLocalTransformationPart2IsValid_internal(forceSensor,valid));
}
SIM_DLLEXPORT simInt simBreakForceSensor(simInt objectHandle)
{
    return(simBreakForceSensor_internal(objectHandle));
}
SIM_DLLEXPORT simBool _simIsForceSensorBroken(const simVoid* forceSensor)
{
    return(_simIsForceSensorBroken_internal(forceSensor));
}
SIM_DLLEXPORT simInt simSetScriptText(simInt scriptHandle,const simChar* scriptText)
{
    return(simSetScriptText_internal(scriptHandle,scriptText));
}
SIM_DLLEXPORT const simChar* simGetScriptText(simInt scriptHandle)
{
    return(simGetScriptText_internal(scriptHandle));
}
SIM_DLLEXPORT simInt simGetScriptProperty(simInt scriptHandle,simInt* scriptProperty,simInt* associatedObjectHandle)
{
    return(simGetScriptProperty_internal(scriptHandle,scriptProperty,associatedObjectHandle));
}
SIM_DLLEXPORT simVoid _simSetGeomProxyDynamicsFullRefreshFlag(simVoid* geomData,simBool flag)
{
}
SIM_DLLEXPORT simBool _simGetGeomProxyDynamicsFullRefreshFlag(const simVoid* geomData)
{
    return(0);
}
SIM_DLLEXPORT simInt simRemoveObject(simInt objectHandle)
{
    return(simRemoveObject_internal(objectHandle));
}
SIM_DLLEXPORT simVoid _simSetShapeIsStaticAndNotRespondableButDynamicTag(const simVoid* shape,simBool tag)
{
}
SIM_DLLEXPORT simBool _simGetShapeIsStaticAndNotRespondableButDynamicTag(const simVoid* shape)
{
    return(0);
}
SIM_DLLEXPORT simInt simGetVisionSensorResolution(simInt visionSensorHandle,simInt* resolution)
{
    return(simGetVisionSensorResolution_internal(visionSensorHandle,resolution));
}
SIM_DLLEXPORT simUChar* simGetVisionSensorCharImage(simInt visionSensorHandle,simInt* resolutionX,simInt* resolutionY)
{
    return(simGetVisionSensorCharImage_internal(visionSensorHandle,resolutionX,resolutionY));
}
SIM_DLLEXPORT simInt simSetVisionSensorCharImage(simInt visionSensorHandle,const simUChar* image)
{
    return(simSetVisionSensorCharImage_internal(visionSensorHandle,image));
}
SIM_DLLEXPORT simFloat* simGetVisionSensorDepthBuffer(simInt visionSensorHandle)
{
    return(simGetVisionSensorDepthBuffer_internal(visionSensorHandle));
}
SIM_DLLEXPORT simVoid* simBroadcastMessage(simInt* auxiliaryData,simVoid* customData,simInt* replyData)
{
    return(simBroadcastMessage_internal(auxiliaryData,customData,replyData));
}
SIM_DLLEXPORT simVoid* simSendModuleMessage(simInt message,simInt* auxiliaryData,simVoid* customData,simInt* replyData)
{
    return(simSendModuleMessage_internal(message,auxiliaryData,customData,replyData));
}
SIM_DLLEXPORT simBool _simIsDynamicMotorEnabled(const simVoid* joint)
{
    return(0);
}
SIM_DLLEXPORT simBool _simIsDynamicMotorPositionCtrlEnabled(const simVoid* joint)
{
    return(0);
}
SIM_DLLEXPORT simBool _simIsDynamicMotorTorqueModulationEnabled(const simVoid* joint)
{
    return(0);
}
SIM_DLLEXPORT simInt _simGetContactCallbackCount()
{
    return(0);
}
SIM_DLLEXPORT const void* _simGetContactCallback(simInt index)
{
    return(nullptr);
}
SIM_DLLEXPORT simInt _simGetJointCallbackCallOrder(const simVoid* joint)
{
    return(1); // not needed anymore
}
SIM_DLLEXPORT simInt simSetDoubleSignal(const simChar* signalName,simDouble signalValue)
{
    return(simSetDoubleSignalOld_internal(signalName,signalValue));
}
SIM_DLLEXPORT simInt simGetDoubleSignal(const simChar* signalName,simDouble* signalValue)
{
    return(simGetDoubleSignalOld_internal(signalName,signalValue));
}
SIM_DLLEXPORT simInt simClearDoubleSignal(const simChar* signalName)
{
    return(simClearDoubleSignalOld_internal(signalName));
}
SIM_DLLEXPORT simInt simRemoveParticleObject(simInt objectHandle)
{ // no effect anymore
    return(-1);
}

#ifdef switchToDouble
//************************
SIM_DLLEXPORT simInt simAddParticleObject_D(simInt objectType,simDouble size,simDouble density,const simVoid* params,simDouble lifeTime,simInt maxItemCount,const simDouble* color,const simDouble* setToNULL,const simDouble* setToNULL2,const simDouble* setToNULL3)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT simInt simAddParticleObjectItem_D(simInt objectHandle,const simDouble* itemData)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT simInt simCreateMotionPlanning_D(simInt jointCnt,const simInt* jointHandles,const simInt* jointRangeSubdivisions,const simDouble* jointMetricWeights,simInt options,const simInt* intParams,const simDouble* floatParams,const simVoid* reserved)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT simInt simJointGetForce_D(simInt jointHandle,simDouble* forceOrTorque)
{
    return(simGetJointForce_internal(jointHandle,forceOrTorque));
}
SIM_DLLEXPORT simInt simGetMpConfigForTipPose_D(simInt motionPlanningObjectHandle,simInt options,simDouble closeNodesDistance,simInt trialCount,const simDouble* tipPose,simInt maxTimeInMs,simDouble* outputJointPositions,const simDouble* referenceConfigs,simInt referenceConfigCount,const simDouble* jointWeights,const simInt* jointBehaviour,simInt correctionPasses)
{
    return(simGetMpConfigForTipPose_internal(motionPlanningObjectHandle,options,closeNodesDistance,trialCount,tipPose,maxTimeInMs,outputJointPositions,referenceConfigs,referenceConfigCount,jointWeights,jointBehaviour,correctionPasses));
}
SIM_DLLEXPORT simDouble* simFindMpPath_D(simInt motionPlanningObjectHandle,const simDouble* startConfig,const simDouble* goalConfig,simInt options,simDouble stepSize,simInt* outputConfigsCnt,simInt maxTimeInMs,simDouble* reserved,const simInt* auxIntParams,const simDouble* auxFloatParams)
{
    return(simFindMpPath_internal(motionPlanningObjectHandle,startConfig,goalConfig,options,stepSize,outputConfigsCnt,maxTimeInMs,reserved,auxIntParams,auxFloatParams));
}
SIM_DLLEXPORT simDouble* simSimplifyMpPath_D(simInt motionPlanningObjectHandle,const simDouble* pathBuffer,simInt configCnt,simInt options,simDouble stepSize,simInt increment,simInt* outputConfigsCnt,simInt maxTimeInMs,simDouble* reserved,const simInt* auxIntParams,const simDouble* auxFloatParams)
{
    return(simSimplifyMpPath_internal(motionPlanningObjectHandle,pathBuffer,configCnt,options,stepSize,increment,outputConfigsCnt,maxTimeInMs,reserved,auxIntParams,auxFloatParams));
}
SIM_DLLEXPORT simDouble* simFindIkPath_D(simInt motionPlanningObjectHandle,const simDouble* startConfig,const simDouble* goalPose,simInt options,simDouble stepSize,simInt* outputConfigsCnt,simDouble* reserved,const simInt* auxIntParams,const simDouble* auxFloatParams)
{
    return(simFindIkPath_internal(motionPlanningObjectHandle,startConfig,goalPose,options,stepSize,outputConfigsCnt,reserved,auxIntParams,auxFloatParams));
}
SIM_DLLEXPORT simDouble* simGetMpConfigTransition_D(simInt motionPlanningObjectHandle,const simDouble* startConfig,const simDouble* goalConfig,simInt options,const simInt* select,simDouble calcStepSize,simDouble maxOutStepSize,simInt wayPointCnt,const simDouble* wayPoints,simInt* outputConfigsCnt,const simInt* auxIntParams,const simDouble* auxFloatParams)
{
    return(simGetMpConfigTransition_internal(motionPlanningObjectHandle,startConfig,goalConfig,options,select,calcStepSize,maxOutStepSize,wayPointCnt,wayPoints,outputConfigsCnt,auxIntParams,auxFloatParams));
}
SIM_DLLEXPORT simInt simSearchPath_D(simInt pathPlanningObjectHandle,simDouble maximumSearchTime)
{
    return(simSearchPath_internal(pathPlanningObjectHandle,maximumSearchTime));
}
SIM_DLLEXPORT simInt simInitializePathSearch_D(simInt pathPlanningObjectHandle,simDouble maximumSearchTime,simDouble searchTimeStep)
{
    return(simInitializePathSearch_internal(pathPlanningObjectHandle,maximumSearchTime,searchTimeStep));
}
SIM_DLLEXPORT simInt simHandlePath_D(simInt pathHandle,simDouble deltaTime)
{
    return(simHandlePath_internal(pathHandle,deltaTime));
}
SIM_DLLEXPORT simInt simHandleJoint_D(simInt jointHandle,simDouble deltaTime)
{
    return(simHandleJoint_internal(jointHandle,deltaTime));
}
SIM_DLLEXPORT simInt simSetUIButtonColor_D(simInt uiHandle,simInt buttonHandle,const simDouble* upStateColor,const simDouble* downStateColor,const simDouble* labelColor)
{
    return(simSetUIButtonColor_internal(uiHandle,buttonHandle,upStateColor,downStateColor,labelColor));
}
SIM_DLLEXPORT simInt simSetUIButtonArrayColor_D(simInt uiHandle,simInt buttonHandle,const simInt* position,const simDouble* color)
{
    return(simSetUIButtonArrayColor_internal(uiHandle,buttonHandle,position,color));
}
SIM_DLLEXPORT simInt simRegisterContactCallback_D(simInt(*callBack)(simInt,simInt,simInt,simInt*,simDouble*))
{
    return(simRegisterContactCallback_internal(callBack));
}
SIM_DLLEXPORT simInt simRegisterJointCtrlCallback_D(simInt(*callBack)(simInt,simInt,simInt,const simInt*,const simDouble*,simDouble*))
{
    return(simRegisterJointCtrlCallback_internal(callBack));
}
SIM_DLLEXPORT simInt simSetJointForce_D(simInt objectHandle,simDouble forceOrTorque)
{
    return(simSetJointMaxForce_internal(objectHandle,forceOrTorque));
}
SIM_DLLEXPORT simInt simHandleMill_D(simInt millHandle,simDouble* removedSurfaceAndVolume)
{
    return(-1);
}
SIM_DLLEXPORT simInt simSetShapeMassAndInertia_D(simInt shapeHandle,simDouble mass,const simDouble* inertiaMatrix,const simDouble* centerOfMass,const simDouble* transformation)
{
    return(simSetShapeMassAndInertia_internal(shapeHandle,mass,inertiaMatrix,centerOfMass,transformation));
}
SIM_DLLEXPORT simInt simGetShapeMassAndInertia_D(simInt shapeHandle,simDouble* mass,simDouble* inertiaMatrix,simDouble* centerOfMass,const simDouble* transformation)
{
    return(simGetShapeMassAndInertia_internal(shapeHandle,mass,inertiaMatrix,centerOfMass,transformation));
}
SIM_DLLEXPORT simInt simCheckIkGroup_D(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simDouble* jointValues,const simInt* jointOptions)
{
    return(simCheckIkGroup_internal(ikGroupHandle,jointCnt,jointHandles,jointValues,jointOptions));
}
SIM_DLLEXPORT simInt simCreateIkGroup_D(simInt options,const simInt* intParams,const simDouble* floatParams,const simVoid* reserved)
{
    return(simCreateIkGroup_internal(options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT simInt simCreateIkElement_D(simInt ikGroupHandle,simInt options,const simInt* intParams,const simDouble* floatParams,const simVoid* reserved)
{
    return(simCreateIkElement_internal(ikGroupHandle,options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT simInt simGetConfigForTipPose_D(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simDouble thresholdDist,simInt maxTimeInMs,simDouble* retConfig,const simDouble* metric,simInt collisionPairCnt,const simInt* collisionPairs,const simInt* jointOptions,const simDouble* lowLimits,const simDouble* ranges,simVoid* reserved)
{
    return(simGetConfigForTipPose_internal(ikGroupHandle,jointCnt,jointHandles,thresholdDist,maxTimeInMs,retConfig,metric,collisionPairCnt,collisionPairs,jointOptions,lowLimits,ranges,reserved));
}
SIM_DLLEXPORT simDouble* simGenerateIkPath_D(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simInt ptCnt,simInt collisionPairCnt,const simInt* collisionPairs,const simInt* jointOptions,simVoid* reserved)
{
    return(simGenerateIkPath_internal(ikGroupHandle,jointCnt,jointHandles,ptCnt,collisionPairCnt,collisionPairs,jointOptions,reserved));
}
SIM_DLLEXPORT simDouble* simGetIkGroupMatrix_D(simInt ikGroupHandle,simInt options,simInt* matrixSize)
{
    return(simGetIkGroupMatrix_internal(ikGroupHandle,options,matrixSize));
}
SIM_DLLEXPORT simInt simSetIkGroupProperties_D(simInt ikGroupHandle,simInt resolutionMethod,simInt maxIterations,simDouble damping,void* reserved)
{
    return(simSetIkGroupProperties_internal(ikGroupHandle,resolutionMethod,maxIterations,damping,reserved));
}
SIM_DLLEXPORT simInt simGetPositionOnPath_D(simInt pathHandle,simDouble relativeDistance,simDouble* position)
{
    return(simGetPositionOnPath_internal(pathHandle,relativeDistance,position));
}
SIM_DLLEXPORT simInt simGetOrientationOnPath_D(simInt pathHandle,simDouble relativeDistance,simDouble* eulerAngles)
{
    return(simGetOrientationOnPath_internal(pathHandle,relativeDistance,eulerAngles));
}
SIM_DLLEXPORT simInt simGetDataOnPath_D(simInt pathHandle,simDouble relativeDistance,simInt dataType,simInt* intData,simDouble* floatData)
{
    return(simGetDataOnPath_internal(pathHandle,relativeDistance,dataType,intData,floatData));
}
SIM_DLLEXPORT simInt simGetClosestPositionOnPath_D(simInt pathHandle,simDouble* absolutePosition,simDouble* pathPosition)
{
    return(simGetClosestPositionOnPath_internal(pathHandle,absolutePosition,pathPosition));
}
SIM_DLLEXPORT simInt simGetPathPosition_D(simInt objectHandle,simDouble* position)
{
    return(simGetPathPosition_internal(objectHandle,position));
}
SIM_DLLEXPORT simInt simSetPathPosition_D(simInt objectHandle,simDouble position)
{
    return(simSetPathPosition_internal(objectHandle,position));
}
SIM_DLLEXPORT simInt simGetPathLength_D(simInt objectHandle,simDouble* length)
{
    return(simGetPathLength_internal(objectHandle,length));
}
SIM_DLLEXPORT simInt simCreatePath_D(simInt attributes,const simInt* intParams,const simDouble* floatParams,const simDouble* color)
{
    return(simCreatePath_internal(attributes,intParams,floatParams,color));
}
SIM_DLLEXPORT simInt simSetIkElementProperties_D(simInt ikGroupHandle,simInt tipDummyHandle,simInt constraints,const simDouble* precision,const simDouble* weight,void* reserved)
{
    return(simSetIkElementProperties_internal(ikGroupHandle,tipDummyHandle,constraints,precision,weight,reserved));
}
SIM_DLLEXPORT simInt simSetVisionSensorFilter_D(simInt visionSensorHandle,simInt filterIndex,simInt options,const simInt* pSizes,const simUChar* bytes,const simInt* ints,const simDouble* floats,const simUChar* custom)
{
    return(simSetVisionSensorFilter_internal(visionSensorHandle,filterIndex,options,pSizes,bytes,ints,floats,custom));
}
SIM_DLLEXPORT simInt simGetVisionSensorFilter_D(simInt visionSensorHandle,simInt filterIndex,simInt* options,simInt* pSizes,simUChar** bytes,simInt** ints,simDouble** floats,simUChar** custom)
{
    return(simGetVisionSensorFilter_internal(visionSensorHandle,filterIndex,options,pSizes,bytes,ints,floats,custom));
}
SIM_DLLEXPORT simInt simSetPathTargetNominalVelocity_D(simInt objectHandle,simDouble targetNominalVelocity)
{
    return(simSetPathTargetNominalVelocity_internal(objectHandle,targetNominalVelocity));
}
SIM_DLLEXPORT simInt simSendData_D(simInt targetID,simInt dataHeader,const simChar* dataName,const simChar* data,simInt dataLength,simInt antennaHandle,simDouble actionRadius,simDouble emissionAngle1,simDouble emissionAngle2,simDouble persistence)
{
    return(simSendData_internal(targetID,dataHeader,dataName,data,dataLength,antennaHandle,actionRadius,emissionAngle1,emissionAngle2,persistence));
}
SIM_DLLEXPORT simInt simHandleDistance_D(simInt distanceObjectHandle,simDouble* smallestDistance)
{
    return(simHandleDistance_internal(distanceObjectHandle,smallestDistance));
}
SIM_DLLEXPORT simInt simReadDistance_D(simInt distanceObjectHandle,simDouble* smallestDistance)
{
    return(simReadDistance_internal(distanceObjectHandle,smallestDistance));
}
SIM_DLLEXPORT simInt simAddBanner_D(const simChar* label,simDouble size,simInt options,const simDouble* positionAndEulerAngles,simInt parentObjectHandle,const simDouble* labelColors,const simDouble* backgroundColors)
{
    return(simAddBanner_internal(label,size,options,positionAndEulerAngles,parentObjectHandle,labelColors,backgroundColors));
}
SIM_DLLEXPORT simInt simAddGhost_D(simInt ghostGroup,simInt objectHandle,simInt options,simDouble startTime,simDouble endTime,const simDouble* color)
{
    return(simAddGhost_internal(ghostGroup,objectHandle,options,startTime,endTime,color));
}
SIM_DLLEXPORT simInt simModifyGhost_D(simInt ghostGroup,simInt ghostId,simInt operation,simDouble floatValue,simInt options,simInt optionsMask,const simDouble* colorOrTransformation)
{
    return(simModifyGhost_internal(ghostGroup,ghostId,operation,floatValue,options,optionsMask,colorOrTransformation));
}
SIM_DLLEXPORT simInt simSetGraphUserData_D(simInt graphHandle,const simChar* streamName,simDouble data)
{
    return(simSetGraphUserData_internal(graphHandle,streamName,data));
}
SIM_DLLEXPORT simInt simAddPointCloud_D(simInt pageMask,simInt layerMask,simInt objectHandle,simInt options,simDouble pointSize,simInt ptCnt,const simDouble* pointCoordinates,const simChar* defaultColors,const simChar* pointColors,const simDouble* pointNormals)
{
    return(simAddPointCloud_internal(pageMask,layerMask,objectHandle,options,pointSize,ptCnt,pointCoordinates,defaultColors,pointColors,pointNormals));
}
SIM_DLLEXPORT simInt simModifyPointCloud_D(simInt pointCloudHandle,simInt operation,const simInt* intParam,const simDouble* floatParam)
{
    return(simModifyPointCloud_internal(pointCloudHandle,operation,intParam,floatParam));
}
SIM_DLLEXPORT simInt simCopyMatrix_D(const simDouble* matrixIn,simDouble* matrixOut)
{
    return(simCopyMatrix_internal(matrixIn,matrixOut));
}
SIM_DLLEXPORT simInt simGetObjectFloatParameter_D(simInt objectHandle,simInt ParamID,simDouble* Param)
{
    return(simGetObjectFloatParam_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT simInt simSetObjectFloatParameter_D(simInt objectHandle,simInt ParamID,simDouble Param)
{
    return(simSetObjectFloatParam_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT simInt simSetFloatingParameter_D(simInt parameter,simDouble floatState)
{
    return(simSetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT simInt simGetFloatingParameter_D(simInt parameter,simDouble* floatState)
{
    return(simGetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT simInt simSetFloatParameter_D(simInt parameter,simDouble floatState)
{
    return(simSetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT simInt simGetFloatParameter_D(simInt parameter,simDouble* floatState)
{
    return(simGetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT simDouble simGetEngineFloatParameter_D(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{
    return(simGetEngineFloatParam_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT simInt simSetEngineFloatParameter_D(simInt paramId,simInt objectHandle,const simVoid* object,simDouble val)
{
    return(simSetEngineFloatParam_internal(paramId,objectHandle,object,val));
}
SIM_DLLEXPORT simInt simSetObjectSizeValues_D(simInt objectHandle,const simDouble* sizeValues)
{
    return(simSetObjectSizeValues_internal(objectHandle,sizeValues));
}
SIM_DLLEXPORT simInt simGetObjectSizeValues_D(simInt objectHandle,simDouble* sizeValues)
{
    return(simGetObjectSizeValues_internal(objectHandle,sizeValues));
}
SIM_DLLEXPORT simInt simDisplayDialog_D(const simChar* titleText,const simChar* mainText,simInt dialogType,const simChar* initialText,const simDouble* titleColors,const simDouble* dialogColors,simInt* elementHandle)
{
    return(simDisplayDialog_internal(titleText,mainText,dialogType,initialText,titleColors,dialogColors,elementHandle));
}
SIM_DLLEXPORT simInt simScaleSelectedObjects_D(simDouble scalingFactor,simBool scalePositionsToo)
{
    return(simScaleSelectedObjects_internal(scalingFactor,scalePositionsToo));
}
SIM_DLLEXPORT simInt simGetJointMatrix_D(simInt objectHandle,simDouble* matrix)
{
    return(simGetJointMatrix_internal(objectHandle,matrix));
}
SIM_DLLEXPORT simInt simSetSphericalJointMatrix_D(simInt objectHandle,const simDouble* matrix)
{
    return(simSetSphericalJointMatrix_internal(objectHandle,matrix));
}
SIM_DLLEXPORT simInt simSetScriptAttribute_D(simInt scriptHandle,simInt attributeID,simDouble floatVal,simInt intOrBoolVal)
{
    return(simSetScriptAttribute_internal(scriptHandle,attributeID,floatVal,intOrBoolVal));
}
SIM_DLLEXPORT simInt simGetScriptAttribute_D(simInt scriptHandle,simInt attributeID,simDouble* floatVal,simInt* intOrBoolVal)
{
    return(simGetScriptAttribute_internal(scriptHandle,attributeID,floatVal,intOrBoolVal));
}
SIM_DLLEXPORT simInt simGetJointMaxForce_D(simInt jointHandle,simDouble* forceOrTorque)
{
    return(simGetJointMaxForce_internal(jointHandle,forceOrTorque));
}
SIM_DLLEXPORT simInt simSetJointMaxForce_D(simInt objectHandle,simDouble forceOrTorque)
{
    return(simSetJointMaxForce_internal(objectHandle,forceOrTorque));
}
SIM_DLLEXPORT simDouble* simGetVisionSensorImage_D(simInt visionSensorHandle)
{
    return(simGetVisionSensorImage_internal(visionSensorHandle));
}
SIM_DLLEXPORT simInt simSetVisionSensorImage_D(simInt visionSensorHandle,const simDouble* image)
{
    return(simSetVisionSensorImage_internal(visionSensorHandle,image));
}
SIM_DLLEXPORT int simSetVisionSensorDepthBuffer_D(simInt visionSensorHandle,const simDouble* depthBuffer)
{
    return(-1);
}
SIM_DLLEXPORT simInt simCreatePureShape_D(simInt primitiveType,simInt options,const simDouble* sizes,simDouble mass,const simInt* precision)
{
    return(simCreatePureShape_internal(primitiveType,options,sizes,mass,precision));
}
SIM_DLLEXPORT simInt simBuildMatrixQ_D(const simDouble* position,const simDouble* quaternion,simDouble* matrix)
{
    return(simBuildMatrixQ_internal(position,quaternion,matrix));
}
SIM_DLLEXPORT simInt simGetQuaternionFromMatrix_D(const simDouble* matrix,simDouble* quaternion)
{
    return(simGetQuaternionFromMatrix_internal(matrix,quaternion));
}
SIM_DLLEXPORT simInt simGetShapeVertex_D(simInt shapeHandle,simInt groupElementIndex,simInt vertexIndex,simDouble* relativePosition)
{
    return(simGetShapeVertex_internal(shapeHandle,groupElementIndex,vertexIndex,relativePosition));
}
SIM_DLLEXPORT simInt simGetShapeTriangle_D(simInt shapeHandle,simInt groupElementIndex,simInt triangleIndex,simInt* vertexIndices,simDouble* triangleNormals)
{
    return(simGetShapeTriangle_internal(shapeHandle,groupElementIndex,triangleIndex,vertexIndices,triangleNormals));
}




SIM_DLLEXPORT simVoid _simGetJointOdeParameters_D(const simVoid* joint,simDouble* stopERP,simDouble* stopCFM,simDouble* bounce,simDouble* fudge,simDouble* normalCFM)
{
    return(_simGetJointOdeParameters_internal(joint,stopERP,stopCFM,bounce,fudge,normalCFM));
}
SIM_DLLEXPORT simVoid _simGetJointBulletParameters_D(const simVoid* joint,simDouble* stopERP,simDouble* stopCFM,simDouble* normalCFM)
{
    return(_simGetJointBulletParameters_internal(joint,stopERP,stopCFM,normalCFM));
}
SIM_DLLEXPORT simVoid _simGetOdeMaxContactFrictionCFMandERP_D(const simVoid* geomInfo,simInt* maxContacts,simDouble* friction,simDouble* cfm,simDouble* erp)
{
    return(_simGetOdeMaxContactFrictionCFMandERP_internal(geomInfo,maxContacts,friction,cfm,erp));
}
SIM_DLLEXPORT simBool _simGetBulletCollisionMargin_D(const simVoid* geomInfo,simDouble* margin,simInt* otherProp)
{
    return(_simGetBulletCollisionMargin_internal(geomInfo,margin,otherProp));
}
SIM_DLLEXPORT simDouble _simGetBulletRestitution_D(const simVoid* geomInfo)
{
    return(_simGetBulletRestitution_internal(geomInfo));
}
SIM_DLLEXPORT simVoid _simGetVortexParameters_D(const simVoid* object,simInt version,simDouble* floatParams,simInt* intParams)
{
    _simGetVortexParameters_internal(object,version,floatParams,intParams);
}
SIM_DLLEXPORT simVoid _simGetNewtonParameters_D(const simVoid* object,simInt* version,simDouble* floatParams,simInt* intParams)
{
    _simGetNewtonParameters_internal(object,version,floatParams,intParams);
}
SIM_DLLEXPORT simVoid _simGetDamping_D(const simVoid* geomInfo,simDouble* linDamping,simDouble* angDamping)
{
    return(_simGetDamping_internal(geomInfo,linDamping,angDamping));
}
SIM_DLLEXPORT simDouble _simGetFriction_D(const simVoid* geomInfo)
{
    return(_simGetFriction_internal(geomInfo));
}
SIM_DLLEXPORT simVoid _simSetDynamicJointLocalTransformationPart2_D(simVoid* joint,const simDouble* pos,const simDouble* quat)
{
    return(_simSetDynamicJointLocalTransformationPart2_internal(joint,pos,quat));
}
SIM_DLLEXPORT simVoid _simSetDynamicForceSensorLocalTransformationPart2_D(simVoid* forceSensor,const simDouble* pos,const simDouble* quat)
{
    return(_simSetDynamicForceSensorLocalTransformationPart2_internal(forceSensor,pos,quat));
}
SIM_DLLEXPORT simVoid _simGetDynamicForceSensorLocalTransformationPart2_D(const simVoid* forceSensor,simDouble* pos,simDouble* quat)
{
    return(_simGetDynamicForceSensorLocalTransformationPart2_internal(forceSensor,pos,quat));
}
SIM_DLLEXPORT simVoid _simGetMotorPid_D(const simVoid* joint,simDouble* pParam,simDouble* iParam,simDouble* dParam)
{
}
SIM_DLLEXPORT simVoid _simGetPrincipalMomentOfInertia_D(const simVoid* geomInfo,simDouble* inertia)
{
    return(_simGetPrincipalMomentOfInertia_internal(geomInfo,inertia));
}
SIM_DLLEXPORT simVoid _simGetLocalInertiaFrame_D(const simVoid* geomInfo,simDouble* pos,simDouble* quat)
{
    return(_simGetLocalInertiaFrame_internal(geomInfo,pos,quat));
}
//************************
SIM_DLLEXPORT simInt simAddParticleObject(simInt objectType,simFloat size,simFloat density,const simVoid* params,simFloat lifeTime,simInt maxItemCount,const simFloat* color,const simFloat* setToNULL,const simFloat* setToNULL2,const simFloat* setToNULL3)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT simInt simAddParticleObjectItem(simInt objectHandle,const simFloat* itemData)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT simInt simCreateMotionPlanning(simInt jointCnt,const simInt* jointHandles,const simInt* jointRangeSubdivisions,const simFloat* jointMetricWeights,simInt options,const simInt* intParams,const simFloat* floatParams,const simVoid* reserved)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT simInt simJointGetForce(simInt jointHandle,simFloat* forceOrTorque)
{
    double f;
    int retVal=simGetJointForce_internal(jointHandle,&f);
    forceOrTorque[0]=(float)f;
    return(retVal);
}
SIM_DLLEXPORT simInt simGetMpConfigForTipPose(simInt motionPlanningObjectHandle,simInt options,simFloat closeNodesDistance,simInt trialCount,const simFloat* tipPose,simInt maxTimeInMs,simFloat* outputJointPositions,const simFloat* referenceConfigs,simInt referenceConfigCount,const simFloat* jointWeights,const simInt* jointBehaviour,simInt correctionPasses)
{
    return(-1);
}
SIM_DLLEXPORT simFloat* simFindMpPath(simInt motionPlanningObjectHandle,const simFloat* startConfig,const simFloat* goalConfig,simInt options,simFloat stepSize,simInt* outputConfigsCnt,simInt maxTimeInMs,simFloat* reserved,const simInt* auxIntParams,const simFloat* auxFloatParams)
{
    return(nullptr);
}
SIM_DLLEXPORT simFloat* simSimplifyMpPath(simInt motionPlanningObjectHandle,const simFloat* pathBuffer,simInt configCnt,simInt options,simFloat stepSize,simInt increment,simInt* outputConfigsCnt,simInt maxTimeInMs,simFloat* reserved,const simInt* auxIntParams,const simFloat* auxFloatParams)
{
    return(nullptr);
}
SIM_DLLEXPORT simFloat* simFindIkPath(simInt motionPlanningObjectHandle,const simFloat* startConfig,const simFloat* goalPose,simInt options,simFloat stepSize,simInt* outputConfigsCnt,simFloat* reserved,const simInt* auxIntParams,const simFloat* auxFloatParams)
{
    return(nullptr);
}
SIM_DLLEXPORT simFloat* simGetMpConfigTransition(simInt motionPlanningObjectHandle,const simFloat* startConfig,const simFloat* goalConfig,simInt options,const simInt* select,simFloat calcStepSize,simFloat maxOutStepSize,simInt wayPointCnt,const simFloat* wayPoints,simInt* outputConfigsCnt,const simInt* auxIntParams,const simFloat* auxFloatParams)
{
    return(nullptr);
}
SIM_DLLEXPORT simInt simSearchPath(simInt pathPlanningObjectHandle,simFloat maximumSearchTime)
{
    return(-1);
}
SIM_DLLEXPORT simInt simInitializePathSearch(simInt pathPlanningObjectHandle,simFloat maximumSearchTime,simFloat searchTimeStep)
{
    return(-1);
}
SIM_DLLEXPORT simInt simHandlePath(simInt pathHandle,simFloat deltaTime)
{
    return(-1);
}
SIM_DLLEXPORT simInt simHandleJoint(simInt jointHandle,simFloat deltaTime)
{
    return(-1);
}
SIM_DLLEXPORT simInt simSetUIButtonColor(simInt uiHandle,simInt buttonHandle,const simFloat* upStateColor,const simFloat* downStateColor,const simFloat* labelColor)
{
    return(-1);
}
SIM_DLLEXPORT simInt simSetUIButtonArrayColor(simInt uiHandle,simInt buttonHandle,const simInt* position,const simFloat* color)
{
    return(-1);
}
SIM_DLLEXPORT simInt simRegisterContactCallback(simInt(*callBack)(simInt,simInt,simInt,simInt*,simFloat*))
{
    return(-1);
}
SIM_DLLEXPORT simInt simRegisterJointCtrlCallback(simInt(*callBack)(simInt,simInt,simInt,const simInt*,const simFloat*,simFloat*))
{
    return(-1);
}
SIM_DLLEXPORT simInt simSetJointForce(simInt objectHandle,simFloat forceOrTorque)
{
    return(simSetJointMaxForce_internal(objectHandle,(double)forceOrTorque));
}
SIM_DLLEXPORT simInt simHandleMill(simInt millHandle,simFloat* removedSurfaceAndVolume)
{
    return(-1);
}
SIM_DLLEXPORT simInt simSetShapeMassAndInertia(simInt shapeHandle,simFloat mass,const simFloat* inertiaMatrix,const simFloat* centerOfMass,const simFloat* transformation)
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
SIM_DLLEXPORT simInt simGetShapeMassAndInertia(simInt shapeHandle,simFloat* mass,simFloat* inertiaMatrix,simFloat* centerOfMass,const simFloat* transformation)
{
    double m;
    double ine[9];
    double c[3];
    double tr[12];
    int retVal=simGetShapeMassAndInertia_internal(shapeHandle,&m,ine,c,tr);
    if (mass!=nullptr)
        mass[0]=(float)m;
    if (inertiaMatrix!=nullptr)
    {
        for (size_t i=0;i<9;i++)
            inertiaMatrix[i]=(float)ine[i];
    }
    if (centerOfMass!=nullptr)
    {
        for (size_t i=0;i<3;i++)
            centerOfMass[i]=(float)c[i];
    }
    if (transformation!=nullptr)
    {
        for (size_t i=0;i<12;i++)
            transformation[i]=(float)tr[i];
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simCheckIkGroup(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simFloat* jointValues,const simInt* jointOptions)
{
    return(-1);
}
SIM_DLLEXPORT simInt simCreateIkGroup(simInt options,const simInt* intParams,const simFloat* floatParams,const simVoid* reserved)
{
    return(-1);
}
SIM_DLLEXPORT simInt simCreateIkElement(simInt ikGroupHandle,simInt options,const simInt* intParams,const simFloat* floatParams,const simVoid* reserved)
{
    return(-1);
}
SIM_DLLEXPORT simInt simGetConfigForTipPose(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simFloat thresholdDist,simInt maxTimeInMs,simFloat* retConfig,const simFloat* metric,simInt collisionPairCnt,const simInt* collisionPairs,const simInt* jointOptions,const simFloat* lowLimits,const simFloat* ranges,simVoid* reserved)
{
    return(-1);
}
SIM_DLLEXPORT simFloat* simGenerateIkPath(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simInt ptCnt,simInt collisionPairCnt,const simInt* collisionPairs,const simInt* jointOptions,simVoid* reserved)
{
    return(-1);
}
SIM_DLLEXPORT simFloat* simGetIkGroupMatrix(simInt ikGroupHandle,simInt options,simInt* matrixSize)
{
    return(nullptr);
}
SIM_DLLEXPORT simInt simSetIkGroupProperties(simInt ikGroupHandle,simInt resolutionMethod,simInt maxIterations,simFloat damping,void* reserved)
{
    return(-1);
}
SIM_DLLEXPORT simInt simGetPositionOnPath(simInt pathHandle,simFloat relativeDistance,simFloat* position)
{
    double p[3];
    int retVal=simGetPositionOnPath_internal(pathHandle,relativeDistance,p);
    for (size_t i=0;i<3;i++)
        position[i]=(float)p[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simGetOrientationOnPath(simInt pathHandle,simFloat relativeDistance,simFloat* eulerAngles)
{
    double p[3];
    int retVal=simGetOrientationOnPath_internal(pathHandle,relativeDistance,p);
    for (size_t i=0;i<3;i++)
        eulerAngles[i]=(float)p[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simGetDataOnPath(simInt pathHandle,simFloat relativeDistance,simInt dataType,simInt* intData,simFloat* floatData)
{
    double d[4];
    int retVal=simGetDataOnPath_internal(pathHandle,(double)relativeDistance,dataType,intData,d);
    for (size_t i=0;i<4;i++)
        floatData[i]=(float)d[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simGetClosestPositionOnPath(simInt pathHandle,simFloat* absolutePosition,simFloat* pathPosition)
{
    double pp[3];
    for (size_t i=0;i<3;i++)
        pp[i]=(double)absolutePosition[i];
    double p;
    int retVal=simGetClosestPositionOnPath_internal(pathHandle,pp,&p);
    position[0]=(float)p;
    return(retVal);
}
SIM_DLLEXPORT simInt simGetPathPosition(simInt objectHandle,simFloat* position)
{
    double p;
    int retVal=simGetPathPosition_internal(objectHandle,&p);
    position[0]=(float)p;
    return(retVal);
}
SIM_DLLEXPORT simInt simSetPathPosition(simInt objectHandle,simFloat position)
{
    return(simSetPathPosition_internal(objectHandle,(double)position));
}
SIM_DLLEXPORT simInt simGetPathLength(simInt objectHandle,simFloat* length)
{
    double l;
    int retVal=simGetPathLength_internal(objectHandle,&l);
    length[0]=(float)l;
    return(retVal);
}
SIM_DLLEXPORT simInt simCreatePath(simInt attributes,const simInt* intParams,const simFloat* floatParams,const simFloat* color)
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
SIM_DLLEXPORT simInt simSetIkElementProperties(simInt ikGroupHandle,simInt tipDummyHandle,simInt constraints,const simFloat* precision,const simFloat* weight,void* reserved)
{
    return(-1);
}
SIM_DLLEXPORT simInt simSetVisionSensorFilter(simInt visionSensorHandle,simInt filterIndex,simInt options,const simInt* pSizes,const simUChar* bytes,const simInt* ints,const simFloat* floats,const simUChar* custom)
{
    return(-1);
}
SIM_DLLEXPORT simInt simGetVisionSensorFilter(simInt visionSensorHandle,simInt filterIndex,simInt* options,simInt* pSizes,simUChar** bytes,simInt** ints,simFloat** floats,simUChar** custom)
{
    return(-1);
}
SIM_DLLEXPORT simInt simSetPathTargetNominalVelocity(simInt objectHandle,simFloat targetNominalVelocity)
{
    return(simSetPathTargetNominalVelocity_internal(objectHandle,(double)targetNominalVelocity));
}
SIM_DLLEXPORT simInt simSendData(simInt targetID,simInt dataHeader,const simChar* dataName,const simChar* data,simInt dataLength,simInt antennaHandle,simFloat actionRadius,simFloat emissionAngle1,simFloat emissionAngle2,simFloat persistence)
{
    return(simSendData_internal(targetID,dataHeader,dataName,data,dataLength,antennaHandle,(double)actionRadius,(double)emissionAngle1,(double)emissionAngle2,(double)persistence));
}
SIM_DLLEXPORT simInt simHandleDistance(simInt distanceObjectHandle,simFloat* smallestDistance)
{
    double d;
    int retVal=simHandleDistance_internal(distanceObjectHandle,&d);
    if ( (retVal>0)&&(smallestDistance!=nullptr) )
        smallestDistance[0]=(float)d;
    return(retVal);
}
SIM_DLLEXPORT simInt simReadDistance(simInt distanceObjectHandle,simFloat* smallestDistance)
{
    double d;
    int retVal=simReadDistance_internal(distanceObjectHandle,&d);
    if ( (retVal!=-1)&&(smallestDistance!=nullptr) )
        smallestDistance[0]=(float)d;
    return(retVal);
}
SIM_DLLEXPORT simInt simAddBanner(const simChar* label,simFloat size,simInt options,const simFloat* positionAndEulerAngles,simInt parentObjectHandle,const simFloat* labelColors,const simFloat* backgroundColors)
{
    return(-1);
}
SIM_DLLEXPORT simInt simAddGhost(simInt ghostGroup,simInt objectHandle,simInt options,simFloat startTime,simFloat endTime,const simFloat* color)
{
    return(-1);
}
SIM_DLLEXPORT simInt simModifyGhost(simInt ghostGroup,simInt ghostId,simInt operation,simFloat floatValue,simInt options,simInt optionsMask,const simFloat* colorOrTransformation)
{
    return(-1);
}
SIM_DLLEXPORT simInt simSetGraphUserData(simInt graphHandle,const simChar* streamName,simFloat data)
{
    return(simSetGraphUserData_internal(graphHandle,streamName,(double)data));
}
SIM_DLLEXPORT simInt simAddPointCloud(simInt pageMask,simInt layerMask,simInt objectHandle,simInt options,simFloat pointSize,simInt ptCnt,const simFloat* pointCoordinates,const simChar* defaultColors,const simChar* pointColors,const simFloat* pointNormals)
{
    return(-1);
}
SIM_DLLEXPORT simInt simModifyPointCloud(simInt pointCloudHandle,simInt operation,const simInt* intParam,const simFloat* floatParam)
{
    return(-1);
}
SIM_DLLEXPORT simInt simCopyMatrix(const simFloat* matrixIn,simFloat* matrixOut)
{
    double m1[12];
    for (size_t i=0;i<12;i++)
        m1[i]=(double)matrixIn[i];
    double m2[12];
    int retVal=simCopyMatrix_internal(m1,m2);
    for (size_t i=0;i<12;i++)
        matrixOut[i]=(float)m2[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simGetObjectFloatParameter(simInt objectHandle,simInt ParamID,simFloat* Param)
{
    double d;
    int retVal=simGetObjectFloatParam_internal(objectHandle,ParamID,&d);
    Param[0]=(float)d;
    return(retVal);
}
SIM_DLLEXPORT simInt simSetObjectFloatParameter(simInt objectHandle,simInt ParamID,simFloat Param)
{
    return(simSetObjectFloatParam_internal(objectHandle,ParamID,(double)Param));
}
SIM_DLLEXPORT simInt simSetFloatingParameter(simInt parameter,simFloat floatState)
{
    return(simSetFloatParam_internal(parameter,(double)floatState));
}
SIM_DLLEXPORT simInt simGetFloatingParameter(simInt parameter,simFloat* floatState)
{
    double d;
    int retVal=simGetFloatParam_internal(parameter,&d);
    floatState[0]=(float)d;
    return(retVal);
}
SIM_DLLEXPORT simInt simSetFloatParameter(simInt parameter,simFloat floatState)
{
    return(simSetFloatParam_internal(parameter,(double)floatState));
}
SIM_DLLEXPORT simInt simGetFloatParameter(simInt parameter,simFloat* floatState)
{
    double d;
    int retVal=simGetFloatParam_internal(parameter,&d);
    floatState[0]=(float)d;
    return(retVal);
}
SIM_DLLEXPORT simFloat simGetEngineFloatParameter(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{
    return((float)simGetEngineFloatParam_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT simInt simSetEngineFloatParameter(simInt paramId,simInt objectHandle,const simVoid* object,simFloat val)
{
    return(simSetEngineFloatParam_internal(paramId,objectHandle,object,(double)val));
}
SIM_DLLEXPORT simInt simSetObjectSizeValues(simInt objectHandle,const simFloat* sizeValues)
{
    double v[3];
    for (size_t i=0;i<3;i++)
        v[i]=(double)sizeValues[i];
    return(simSetObjectSizeValues_internal(objectHandle,v));
}
SIM_DLLEXPORT simInt simGetObjectSizeValues(simInt objectHandle,simFloat* sizeValues)
{
    double v[3];
    int retVal=simGetObjectSizeValues_internal(objectHandle,v);
    for (size_t i=0;i<3;i++)
        sizeValues[i]=(float)v[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simDisplayDialog(const simChar* titleText,const simChar* mainText,simInt dialogType,const simChar* initialText,const simFloat* titleColors,const simFloat* dialogColors,simInt* elementHandle)
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
SIM_DLLEXPORT simInt simScaleSelectedObjects(simFloat scalingFactor,simBool scalePositionsToo)
{
    return(simScaleSelectedObjects_internal((double)scalingFactor,scalePositionsToo));
}
SIM_DLLEXPORT simInt simGetJointMatrix(simInt objectHandle,simFloat* matrix)
{
    double v[12];
    int retVal=simGetJointMatrix_internal(objectHandle,v);
    for (size_t i=0;i<12;i++)
        matrix[i]=(float)v[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simSetSphericalJointMatrix(simInt objectHandle,const simFloat* matrix)
{
    double v[12];
    for (size_t i=0;i<12;i++)
        v[i]=(double)matrix[i];
    return(simSetSphericalJointMatrix_internal(objectHandle,v));
}
SIM_DLLEXPORT simInt simSetScriptAttribute(simInt scriptHandle,simInt attributeID,simFloat floatVal,simInt intOrBoolVal)
{
    return(simSetScriptAttribute_internal(scriptHandle,attributeID,(double)floatVal,intOrBoolVal));
}
SIM_DLLEXPORT simInt simGetScriptAttribute(simInt scriptHandle,simInt attributeID,simFloat* floatVal,simInt* intOrBoolVal)
{
    double v;
    int retVal=simGetScriptAttribute_internal(scriptHandle,attributeID,v,intOrBoolVal);
    if (floatVal!=nullptr)
        floatVal[0]=(float)v;
    return(retVal);
}
SIM_DLLEXPORT simInt simGetJointMaxForce(simInt jointHandle,simFloat* forceOrTorque)
{
    double v;
    int retVal=simGetJointMaxForce_internal(jointHandle,&v);
    forceOrTorque[0]=(float)v;
    return(retVal);
}
SIM_DLLEXPORT simInt simSetJointMaxForce(simInt objectHandle,simFloat forceOrTorque)
{
    return(simSetJointMaxForce_internal(objectHandle,(double)forceOrTorque));
}
SIM_DLLEXPORT simFloat* simGetVisionSensorImage(simInt visionSensorHandle)
{
    float retVal=nullptr;
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
            retVal=simCreateBuffer_internal(res[0]*res[1]*sizeof(float)*valPerPixel);
            for (int i=0;i<res[0]*res[1]*valPerPixel;i++)
                retVal[i]=(float)dta[i];
            simReleaseBuffer_internal((char*)dta);
        }
    }
    return(retVal);
}
SIM_DLLEXPORT simInt simSetVisionSensorImage(simInt visionSensorHandle,const simFloat* image)
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
SIM_DLLEXPORT int simSetVisionSensorDepthBuffer(simInt visionSensorHandle,const simFloat* depthBuffer)
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
SIM_DLLEXPORT simInt simCreatePureShape(simInt primitiveType,simInt options,const simFloat* sizes,simFloat mass,const simInt* precision)
{
    double s[3];
    for (size_t i=0;i<3;i++)
        s[i]=(double)sizes[i];
    return(simCreatePureShape_internal(primitiveType,options,s,(double)mass,precision));
}
SIM_DLLEXPORT simInt simBuildMatrixQ(const simFloat* position,const simFloat* quaternion,simFloat* matrix)
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
        matrix[i]=(float)m[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simGetQuaternionFromMatrix(const simFloat* matrix,simFloat* quaternion)
{
    double m[12];
    for (size_t i=0;i<12;i++)
        m[i]=(double)matrix[i];
    double q[4];
    int retVal=simGetQuaternionFromMatrix_internal(m,q);
    for (size_t i=0;i<4;i++)
        quaternion[i]=(float)q[i];
    return(retVal);
}
SIM_DLLEXPORT simInt simGetShapeVertex(simInt shapeHandle,simInt groupElementIndex,simInt vertexIndex,simFloat* relativePosition)
{
    return(-1);
}
SIM_DLLEXPORT simInt simGetShapeTriangle(simInt shapeHandle,simInt groupElementIndex,simInt triangleIndex,simInt* vertexIndices,simFloat* triangleNormals)
{
    return(-1);
}


SIM_DLLEXPORT simVoid _simGetJointOdeParameters(const simVoid* joint,simFloat* stopERP,simFloat* stopCFM,simFloat* bounce,simFloat* fudge,simFloat* normalCFM)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simGetJointBulletParameters(const simVoid* joint,simFloat* stopERP,simFloat* stopCFM,simFloat* normalCFM)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simGetOdeMaxContactFrictionCFMandERP(const simVoid* geomInfo,simInt* maxContacts,simFloat* friction,simFloat* cfm,simFloat* erp)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simBool _simGetBulletCollisionMargin(const simVoid* geomInfo,simFloat* margin,simInt* otherProp)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
    return(0);
}
SIM_DLLEXPORT simFloat _simGetBulletRestitution(const simVoid* geomInfo)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT simVoid _simGetVortexParameters(const simVoid* object,simInt version,simFloat* floatParams,simInt* intParams)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simGetNewtonParameters(const simVoid* object,simInt* version,simFloat* floatParams,simInt* intParams)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simGetDamping(const simVoid* geomInfo,simFloat* linDamping,simFloat* angDamping)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simFloat _simGetFriction(const simVoid* geomInfo)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
    return(0.0);
}
SIM_DLLEXPORT simVoid _simSetDynamicJointLocalTransformationPart2(simVoid* joint,const simFloat* pos,const simFloat* quat)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simSetDynamicForceSensorLocalTransformationPart2(simVoid* forceSensor,const simFloat* pos,const simFloat* quat)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simGetDynamicForceSensorLocalTransformationPart2(const simVoid* forceSensor,simFloat* pos,simFloat* quat)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simGetMotorPid(const simVoid* joint,simFloat* pParam,simFloat* iParam,simFloat* dParam)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simGetPrincipalMomentOfInertia(const simVoid* geomInfo,simFloat* inertia)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
SIM_DLLEXPORT simVoid _simGetLocalInertiaFrame(const simVoid* geomInfo,simFloat* pos,simFloat* quat)
{ // not supported anymore in single-precision float mode, once we switch to double-precision float mode. Only physics plugins relied on those, and they will be rebuilt
}
//************************
#else
SIM_DLLEXPORT simInt simAddParticleObject(simInt objectType,simFloat size,simFloat density,const simVoid* params,simFloat lifeTime,simInt maxItemCount,const simFloat* color,const simFloat* setToNULL,const simFloat* setToNULL2,const simFloat* setToNULL3)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT simInt simAddParticleObjectItem(simInt objectHandle,const simFloat* itemData)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT simInt simCreateMotionPlanning(simInt jointCnt,const simInt* jointHandles,const simInt* jointRangeSubdivisions,const simFloat* jointMetricWeights,simInt options,const simInt* intParams,const simFloat* floatParams,const simVoid* reserved)
{ // no effect anymore
    return(-1);
}
SIM_DLLEXPORT simInt simJointGetForce(simInt jointHandle,simFloat* forceOrTorque)
{
    return(simGetJointForce_internal(jointHandle,forceOrTorque));
}
SIM_DLLEXPORT simInt simGetMpConfigForTipPose(simInt motionPlanningObjectHandle,simInt options,simFloat closeNodesDistance,simInt trialCount,const simFloat* tipPose,simInt maxTimeInMs,simFloat* outputJointPositions,const simFloat* referenceConfigs,simInt referenceConfigCount,const simFloat* jointWeights,const simInt* jointBehaviour,simInt correctionPasses)
{
    return(simGetMpConfigForTipPose_internal(motionPlanningObjectHandle,options,closeNodesDistance,trialCount,tipPose,maxTimeInMs,outputJointPositions,referenceConfigs,referenceConfigCount,jointWeights,jointBehaviour,correctionPasses));
}
SIM_DLLEXPORT simFloat* simFindMpPath(simInt motionPlanningObjectHandle,const simFloat* startConfig,const simFloat* goalConfig,simInt options,simFloat stepSize,simInt* outputConfigsCnt,simInt maxTimeInMs,simFloat* reserved,const simInt* auxIntParams,const simFloat* auxFloatParams)
{
    return(simFindMpPath_internal(motionPlanningObjectHandle,startConfig,goalConfig,options,stepSize,outputConfigsCnt,maxTimeInMs,reserved,auxIntParams,auxFloatParams));
}
SIM_DLLEXPORT simFloat* simSimplifyMpPath(simInt motionPlanningObjectHandle,const simFloat* pathBuffer,simInt configCnt,simInt options,simFloat stepSize,simInt increment,simInt* outputConfigsCnt,simInt maxTimeInMs,simFloat* reserved,const simInt* auxIntParams,const simFloat* auxFloatParams)
{
    return(simSimplifyMpPath_internal(motionPlanningObjectHandle,pathBuffer,configCnt,options,stepSize,increment,outputConfigsCnt,maxTimeInMs,reserved,auxIntParams,auxFloatParams));
}
SIM_DLLEXPORT simFloat* simFindIkPath(simInt motionPlanningObjectHandle,const simFloat* startConfig,const simFloat* goalPose,simInt options,simFloat stepSize,simInt* outputConfigsCnt,simFloat* reserved,const simInt* auxIntParams,const simFloat* auxFloatParams)
{
    return(simFindIkPath_internal(motionPlanningObjectHandle,startConfig,goalPose,options,stepSize,outputConfigsCnt,reserved,auxIntParams,auxFloatParams));
}
SIM_DLLEXPORT simFloat* simGetMpConfigTransition(simInt motionPlanningObjectHandle,const simFloat* startConfig,const simFloat* goalConfig,simInt options,const simInt* select,simFloat calcStepSize,simFloat maxOutStepSize,simInt wayPointCnt,const simFloat* wayPoints,simInt* outputConfigsCnt,const simInt* auxIntParams,const simFloat* auxFloatParams)
{
    return(simGetMpConfigTransition_internal(motionPlanningObjectHandle,startConfig,goalConfig,options,select,calcStepSize,maxOutStepSize,wayPointCnt,wayPoints,outputConfigsCnt,auxIntParams,auxFloatParams));
}
SIM_DLLEXPORT simInt simSearchPath(simInt pathPlanningObjectHandle,simFloat maximumSearchTime)
{
    return(simSearchPath_internal(pathPlanningObjectHandle,maximumSearchTime));
}
SIM_DLLEXPORT simInt simInitializePathSearch(simInt pathPlanningObjectHandle,simFloat maximumSearchTime,simFloat searchTimeStep)
{
    return(simInitializePathSearch_internal(pathPlanningObjectHandle,maximumSearchTime,searchTimeStep));
}
SIM_DLLEXPORT simInt simHandlePath(simInt pathHandle,simFloat deltaTime)
{
    return(simHandlePath_internal(pathHandle,deltaTime));
}
SIM_DLLEXPORT simInt simHandleJoint(simInt jointHandle,simFloat deltaTime)
{
    return(simHandleJoint_internal(jointHandle,deltaTime));
}
SIM_DLLEXPORT simInt simSetUIButtonColor(simInt uiHandle,simInt buttonHandle,const simFloat* upStateColor,const simFloat* downStateColor,const simFloat* labelColor)
{
    return(simSetUIButtonColor_internal(uiHandle,buttonHandle,upStateColor,downStateColor,labelColor));
}
SIM_DLLEXPORT simInt simSetUIButtonArrayColor(simInt uiHandle,simInt buttonHandle,const simInt* position,const simFloat* color)
{
    return(simSetUIButtonArrayColor_internal(uiHandle,buttonHandle,position,color));
}
SIM_DLLEXPORT simInt simRegisterContactCallback(simInt(*callBack)(simInt,simInt,simInt,simInt*,simFloat*))
{
    return(simRegisterContactCallback_internal(callBack));
}
SIM_DLLEXPORT simInt simRegisterJointCtrlCallback(simInt(*callBack)(simInt,simInt,simInt,const simInt*,const simFloat*,simFloat*))
{
    return(simRegisterJointCtrlCallback_internal(callBack));
}
SIM_DLLEXPORT simInt simSetJointForce(simInt objectHandle,simFloat forceOrTorque)
{
    return(simSetJointMaxForce_internal(objectHandle,forceOrTorque));
}
SIM_DLLEXPORT simInt simHandleMill(simInt millHandle,simFloat* removedSurfaceAndVolume)
{
    return(-1);
}
SIM_DLLEXPORT simInt simSetShapeMassAndInertia(simInt shapeHandle,simFloat mass,const simFloat* inertiaMatrix,const simFloat* centerOfMass,const simFloat* transformation)
{
    return(simSetShapeMassAndInertia_internal(shapeHandle,mass,inertiaMatrix,centerOfMass,transformation));
}
SIM_DLLEXPORT simInt simGetShapeMassAndInertia(simInt shapeHandle,simFloat* mass,simFloat* inertiaMatrix,simFloat* centerOfMass,const simFloat* transformation)
{
    return(simGetShapeMassAndInertia_internal(shapeHandle,mass,inertiaMatrix,centerOfMass,transformation));
}
SIM_DLLEXPORT simInt simCheckIkGroup(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simFloat* jointValues,const simInt* jointOptions)
{
    return(simCheckIkGroup_internal(ikGroupHandle,jointCnt,jointHandles,jointValues,jointOptions));
}
SIM_DLLEXPORT simInt simCreateIkGroup(simInt options,const simInt* intParams,const simFloat* floatParams,const simVoid* reserved)
{
    return(simCreateIkGroup_internal(options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT simInt simCreateIkElement(simInt ikGroupHandle,simInt options,const simInt* intParams,const simFloat* floatParams,const simVoid* reserved)
{
    return(simCreateIkElement_internal(ikGroupHandle,options,intParams,floatParams,reserved));
}
SIM_DLLEXPORT simInt simGetConfigForTipPose(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simFloat thresholdDist,simInt maxTimeInMs,simFloat* retConfig,const simFloat* metric,simInt collisionPairCnt,const simInt* collisionPairs,const simInt* jointOptions,const simFloat* lowLimits,const simFloat* ranges,simVoid* reserved)
{
    return(simGetConfigForTipPose_internal(ikGroupHandle,jointCnt,jointHandles,thresholdDist,maxTimeInMs,retConfig,metric,collisionPairCnt,collisionPairs,jointOptions,lowLimits,ranges,reserved));
}
SIM_DLLEXPORT simFloat* simGenerateIkPath(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simInt ptCnt,simInt collisionPairCnt,const simInt* collisionPairs,const simInt* jointOptions,simVoid* reserved)
{
    return(simGenerateIkPath_internal(ikGroupHandle,jointCnt,jointHandles,ptCnt,collisionPairCnt,collisionPairs,jointOptions,reserved));
}
SIM_DLLEXPORT simFloat* simGetIkGroupMatrix(simInt ikGroupHandle,simInt options,simInt* matrixSize)
{
    return(simGetIkGroupMatrix_internal(ikGroupHandle,options,matrixSize));
}
SIM_DLLEXPORT simInt simSetIkGroupProperties(simInt ikGroupHandle,simInt resolutionMethod,simInt maxIterations,simFloat damping,void* reserved)
{
    return(simSetIkGroupProperties_internal(ikGroupHandle,resolutionMethod,maxIterations,damping,reserved));
}
SIM_DLLEXPORT simInt simGetPositionOnPath(simInt pathHandle,simFloat relativeDistance,simFloat* position)
{
    return(simGetPositionOnPath_internal(pathHandle,relativeDistance,position));
}
SIM_DLLEXPORT simInt simGetOrientationOnPath(simInt pathHandle,simFloat relativeDistance,simFloat* eulerAngles)
{
    return(simGetOrientationOnPath_internal(pathHandle,relativeDistance,eulerAngles));
}
SIM_DLLEXPORT simInt simGetDataOnPath(simInt pathHandle,simFloat relativeDistance,simInt dataType,simInt* intData,simFloat* floatData)
{
    return(simGetDataOnPath_internal(pathHandle,relativeDistance,dataType,intData,floatData));
}
SIM_DLLEXPORT simInt simGetClosestPositionOnPath(simInt pathHandle,simFloat* absolutePosition,simFloat* pathPosition)
{
    return(simGetClosestPositionOnPath_internal(pathHandle,absolutePosition,pathPosition));
}
SIM_DLLEXPORT simInt simGetPathPosition(simInt objectHandle,simFloat* position)
{
    return(simGetPathPosition_internal(objectHandle,position));
}
SIM_DLLEXPORT simInt simSetPathPosition(simInt objectHandle,simFloat position)
{
    return(simSetPathPosition_internal(objectHandle,position));
}
SIM_DLLEXPORT simInt simGetPathLength(simInt objectHandle,simFloat* length)
{
    return(simGetPathLength_internal(objectHandle,length));
}
SIM_DLLEXPORT simInt simCreatePath(simInt attributes,const simInt* intParams,const simFloat* floatParams,const simFloat* color)
{
    return(simCreatePath_internal(attributes,intParams,floatParams,color));
}
SIM_DLLEXPORT simInt simSetIkElementProperties(simInt ikGroupHandle,simInt tipDummyHandle,simInt constraints,const simFloat* precision,const simFloat* weight,void* reserved)
{
    return(simSetIkElementProperties_internal(ikGroupHandle,tipDummyHandle,constraints,precision,weight,reserved));
}
SIM_DLLEXPORT simInt simSetVisionSensorFilter(simInt visionSensorHandle,simInt filterIndex,simInt options,const simInt* pSizes,const simUChar* bytes,const simInt* ints,const simFloat* floats,const simUChar* custom)
{
    return(simSetVisionSensorFilter_internal(visionSensorHandle,filterIndex,options,pSizes,bytes,ints,floats,custom));
}
SIM_DLLEXPORT simInt simGetVisionSensorFilter(simInt visionSensorHandle,simInt filterIndex,simInt* options,simInt* pSizes,simUChar** bytes,simInt** ints,simFloat** floats,simUChar** custom)
{
    return(simGetVisionSensorFilter_internal(visionSensorHandle,filterIndex,options,pSizes,bytes,ints,floats,custom));
}
SIM_DLLEXPORT simInt simSetPathTargetNominalVelocity(simInt objectHandle,simFloat targetNominalVelocity)
{
    return(simSetPathTargetNominalVelocity_internal(objectHandle,targetNominalVelocity));
}
SIM_DLLEXPORT simInt simSendData(simInt targetID,simInt dataHeader,const simChar* dataName,const simChar* data,simInt dataLength,simInt antennaHandle,simFloat actionRadius,simFloat emissionAngle1,simFloat emissionAngle2,simFloat persistence)
{
    return(simSendData_internal(targetID,dataHeader,dataName,data,dataLength,antennaHandle,actionRadius,emissionAngle1,emissionAngle2,persistence));
}
SIM_DLLEXPORT simInt simHandleDistance(simInt distanceObjectHandle,simFloat* smallestDistance)
{
    return(simHandleDistance_internal(distanceObjectHandle,smallestDistance));
}
SIM_DLLEXPORT simInt simReadDistance(simInt distanceObjectHandle,simFloat* smallestDistance)
{
    return(simReadDistance_internal(distanceObjectHandle,smallestDistance));
}
SIM_DLLEXPORT simInt simAddBanner(const simChar* label,simFloat size,simInt options,const simFloat* positionAndEulerAngles,simInt parentObjectHandle,const simFloat* labelColors,const simFloat* backgroundColors)
{
    return(simAddBanner_internal(label,size,options,positionAndEulerAngles,parentObjectHandle,labelColors,backgroundColors));
}
SIM_DLLEXPORT simInt simAddGhost(simInt ghostGroup,simInt objectHandle,simInt options,simFloat startTime,simFloat endTime,const simFloat* color)
{
    return(simAddGhost_internal(ghostGroup,objectHandle,options,startTime,endTime,color));
}
SIM_DLLEXPORT simInt simModifyGhost(simInt ghostGroup,simInt ghostId,simInt operation,simFloat floatValue,simInt options,simInt optionsMask,const simFloat* colorOrTransformation)
{
    return(simModifyGhost_internal(ghostGroup,ghostId,operation,floatValue,options,optionsMask,colorOrTransformation));
}
SIM_DLLEXPORT simInt simSetGraphUserData(simInt graphHandle,const simChar* streamName,simFloat data)
{
    return(simSetGraphUserData_internal(graphHandle,streamName,data));
}
SIM_DLLEXPORT simInt simAddPointCloud(simInt pageMask,simInt layerMask,simInt objectHandle,simInt options,simFloat pointSize,simInt ptCnt,const simFloat* pointCoordinates,const simChar* defaultColors,const simChar* pointColors,const simFloat* pointNormals)
{
    return(simAddPointCloud_internal(pageMask,layerMask,objectHandle,options,pointSize,ptCnt,pointCoordinates,defaultColors,pointColors,pointNormals));
}
SIM_DLLEXPORT simInt simModifyPointCloud(simInt pointCloudHandle,simInt operation,const simInt* intParam,const simFloat* floatParam)
{
    return(simModifyPointCloud_internal(pointCloudHandle,operation,intParam,floatParam));
}
SIM_DLLEXPORT simInt simCopyMatrix(const simFloat* matrixIn,simFloat* matrixOut)
{
    return(simCopyMatrix_internal(matrixIn,matrixOut));
}
SIM_DLLEXPORT simInt simGetObjectFloatParameter(simInt objectHandle,simInt ParamID,simFloat* Param)
{
    return(simGetObjectFloatParam_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT simInt simSetObjectFloatParameter(simInt objectHandle,simInt ParamID,simFloat Param)
{
    return(simSetObjectFloatParam_internal(objectHandle,ParamID,Param));
}
SIM_DLLEXPORT simInt simSetFloatingParameter(simInt parameter,simFloat floatState)
{
    return(simSetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT simInt simGetFloatingParameter(simInt parameter,simFloat* floatState)
{
    return(simGetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT simInt simSetFloatParameter(simInt parameter,simFloat floatState)
{
    return(simSetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT simInt simGetFloatParameter(simInt parameter,simFloat* floatState)
{
    return(simGetFloatParam_internal(parameter,floatState));
}
SIM_DLLEXPORT simFloat simGetEngineFloatParameter(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{
    return(simGetEngineFloatParam_internal(paramId,objectHandle,object,ok));
}
SIM_DLLEXPORT simInt simSetEngineFloatParameter(simInt paramId,simInt objectHandle,const simVoid* object,simFloat val)
{
    return(simSetEngineFloatParam_internal(paramId,objectHandle,object,val));
}
SIM_DLLEXPORT simInt simSetObjectSizeValues(simInt objectHandle,const simFloat* sizeValues)
{
    return(simSetObjectSizeValues_internal(objectHandle,sizeValues));
}
SIM_DLLEXPORT simInt simGetObjectSizeValues(simInt objectHandle,simFloat* sizeValues)
{
    return(simGetObjectSizeValues_internal(objectHandle,sizeValues));
}
SIM_DLLEXPORT simInt simDisplayDialog(const simChar* titleText,const simChar* mainText,simInt dialogType,const simChar* initialText,const simFloat* titleColors,const simFloat* dialogColors,simInt* elementHandle)
{
    return(simDisplayDialog_internal(titleText,mainText,dialogType,initialText,titleColors,dialogColors,elementHandle));
}
SIM_DLLEXPORT simInt simScaleSelectedObjects(simFloat scalingFactor,simBool scalePositionsToo)
{
    return(simScaleSelectedObjects_internal(scalingFactor,scalePositionsToo));
}
SIM_DLLEXPORT simInt simGetJointMatrix(simInt objectHandle,simFloat* matrix)
{
    return(simGetJointMatrix_internal(objectHandle,matrix));
}
SIM_DLLEXPORT simInt simSetSphericalJointMatrix(simInt objectHandle,const simFloat* matrix)
{
    return(simSetSphericalJointMatrix_internal(objectHandle,matrix));
}
SIM_DLLEXPORT simInt simSetScriptAttribute(simInt scriptHandle,simInt attributeID,simFloat floatVal,simInt intOrBoolVal)
{
    return(simSetScriptAttribute_internal(scriptHandle,attributeID,floatVal,intOrBoolVal));
}
SIM_DLLEXPORT simInt simGetScriptAttribute(simInt scriptHandle,simInt attributeID,simFloat* floatVal,simInt* intOrBoolVal)
{
    return(simGetScriptAttribute_internal(scriptHandle,attributeID,floatVal,intOrBoolVal));
}
SIM_DLLEXPORT simInt simGetJointMaxForce(simInt jointHandle,simFloat* forceOrTorque)
{
    return(simGetJointMaxForce_internal(jointHandle,forceOrTorque));
}
SIM_DLLEXPORT simInt simSetJointMaxForce(simInt objectHandle,simFloat forceOrTorque)
{
    return(simSetJointMaxForce_internal(objectHandle,forceOrTorque));
}
SIM_DLLEXPORT simFloat* simGetVisionSensorImage(simInt visionSensorHandle)
{
    return(simGetVisionSensorImage_internal(visionSensorHandle));
}
SIM_DLLEXPORT simInt simSetVisionSensorImage(simInt visionSensorHandle,const simFloat* image)
{
    return(simSetVisionSensorImage_internal(visionSensorHandle,image));
}
SIM_DLLEXPORT int simSetVisionSensorDepthBuffer(simInt visionSensorHandle,const simFloat* depthBuffer)
{
    return(-1);
}
SIM_DLLEXPORT simInt simCreatePureShape(simInt primitiveType,simInt options,const simFloat* sizes,simFloat mass,const simInt* precision)
{
    return(simCreatePureShape_internal(primitiveType,options,sizes,mass,precision));
}
SIM_DLLEXPORT simInt simBuildMatrixQ(const simFloat* position,const simFloat* quaternion,simFloat* matrix)
{
    return(simBuildMatrixQ_internal(position,quaternion,matrix));
}
SIM_DLLEXPORT simInt simGetQuaternionFromMatrix(const simFloat* matrix,simFloat* quaternion)
{
    return(simGetQuaternionFromMatrix_internal(matrix,quaternion));
}
SIM_DLLEXPORT simInt simGetShapeVertex(simInt shapeHandle,simInt groupElementIndex,simInt vertexIndex,simFloat* relativePosition)
{
    return(simGetShapeVertex_internal(shapeHandle,groupElementIndex,vertexIndex,relativePosition));
}
SIM_DLLEXPORT simInt simGetShapeTriangle(simInt shapeHandle,simInt groupElementIndex,simInt triangleIndex,simInt* vertexIndices,simFloat* triangleNormals)
{
    return(simGetShapeTriangle_internal(shapeHandle,groupElementIndex,triangleIndex,vertexIndices,triangleNormals));
}




SIM_DLLEXPORT simVoid _simGetJointOdeParameters(const simVoid* joint,simFloat* stopERP,simFloat* stopCFM,simFloat* bounce,simFloat* fudge,simFloat* normalCFM)
{
    return(_simGetJointOdeParameters_internal(joint,stopERP,stopCFM,bounce,fudge,normalCFM));
}
SIM_DLLEXPORT simVoid _simGetJointBulletParameters(const simVoid* joint,simFloat* stopERP,simFloat* stopCFM,simFloat* normalCFM)
{
    return(_simGetJointBulletParameters_internal(joint,stopERP,stopCFM,normalCFM));
}
SIM_DLLEXPORT simVoid _simGetOdeMaxContactFrictionCFMandERP(const simVoid* geomInfo,simInt* maxContacts,simFloat* friction,simFloat* cfm,simFloat* erp)
{
    return(_simGetOdeMaxContactFrictionCFMandERP_internal(geomInfo,maxContacts,friction,cfm,erp));
}
SIM_DLLEXPORT simBool _simGetBulletCollisionMargin(const simVoid* geomInfo,simFloat* margin,simInt* otherProp)
{
    return(_simGetBulletCollisionMargin_internal(geomInfo,margin,otherProp));
}
SIM_DLLEXPORT simFloat _simGetBulletRestitution(const simVoid* geomInfo)
{
    return(_simGetBulletRestitution_internal(geomInfo));
}
SIM_DLLEXPORT simVoid _simGetVortexParameters(const simVoid* object,simInt version,simFloat* floatParams,simInt* intParams)
{
    _simGetVortexParameters_internal(object,version,floatParams,intParams);
}
SIM_DLLEXPORT simVoid _simGetNewtonParameters(const simVoid* object,simInt* version,simFloat* floatParams,simInt* intParams)
{
    _simGetNewtonParameters_internal(object,version,floatParams,intParams);
}
SIM_DLLEXPORT simVoid _simGetDamping(const simVoid* geomInfo,simFloat* linDamping,simFloat* angDamping)
{
    return(_simGetDamping_internal(geomInfo,linDamping,angDamping));
}
SIM_DLLEXPORT simFloat _simGetFriction(const simVoid* geomInfo)
{
    return(_simGetFriction_internal(geomInfo));
}
SIM_DLLEXPORT simVoid _simSetDynamicJointLocalTransformationPart2(simVoid* joint,const simFloat* pos,const simFloat* quat)
{
    return(_simSetDynamicJointLocalTransformationPart2_internal(joint,pos,quat));
}
SIM_DLLEXPORT simVoid _simSetDynamicForceSensorLocalTransformationPart2(simVoid* forceSensor,const simFloat* pos,const simFloat* quat)
{
    return(_simSetDynamicForceSensorLocalTransformationPart2_internal(forceSensor,pos,quat));
}
SIM_DLLEXPORT simVoid _simGetDynamicForceSensorLocalTransformationPart2(const simVoid* forceSensor,simFloat* pos,simFloat* quat)
{
    return(_simGetDynamicForceSensorLocalTransformationPart2_internal(forceSensor,pos,quat));
}
SIM_DLLEXPORT simVoid _simGetMotorPid(const simVoid* joint,simFloat* pParam,simFloat* iParam,simFloat* dParam)
{
}
SIM_DLLEXPORT simVoid _simGetPrincipalMomentOfInertia(const simVoid* geomInfo,simFloat* inertia)
{
    return(_simGetPrincipalMomentOfInertia_internal(geomInfo,inertia));
}
SIM_DLLEXPORT simVoid _simGetLocalInertiaFrame(const simVoid* geomInfo,simFloat* pos,simFloat* quat)
{
    return(_simGetLocalInertiaFrame_internal(geomInfo,pos,quat));
}
#endif

