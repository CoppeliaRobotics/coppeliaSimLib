
#include "mpObject.h"
#include "v_rep_internal.h"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/variate_generator.hpp>
CmpObject::CmpObject()
{
}

CmpObject::~CmpObject()
{
    _clearAllPhase1Nodes();
    _clearAllPhase2Nodes();
}

CmpObject* CmpObject::copyYourself(int jointCount)
{
    CmpObject* newObj=new CmpObject();
    for (int i=0;i<int(_allNodes.size());i++)
        newObj->_allNodes.push_back(_allNodes[i]->copyYourself(jointCount));

    return(newObj);
}

void CmpObject::setData(int jointCnt,const int* jointHandles,const int* jointStepCount,int ikGroupId,int baseFrameId,int tipFrameId,int robotSelfCollEntity1,int robotSelfCollEntity2,int robotEntity,int obstacleEntity,float distanceThreshold,const float* tipMetricAndIkElConstraints,const float* robotMetric)
{
    _jointHandles.resize(jointCnt,0);
    _jointStepCount.resize(jointCnt,0);
    _robotMetric.resize(jointCnt,0);
    _cyclicJointFlags.resize(jointCnt,0);
    _jointMinVals.resize(jointCnt,0);
    _jointRanges.resize(jointCnt,0);
    _jointObjects.resize(jointCnt,0);

    for (int i=0;i<jointCnt;i++)
    {
        _jointHandles[i]=jointHandles[i];
        _jointStepCount[i]=jointStepCount[i];
        _robotMetric[i]=robotMetric[i];

        CDummyJoint* theJ=(CDummyJoint*)_simGetObject_internal(_jointHandles[i]);
        _jointObjects[i]=theJ;

        int jtype=_simGetJointType_internal(theJ);
        float minV,rangeV;
        bool cyclic=!_simGetJointPositionInterval_internal(theJ,&minV,&rangeV);
        if ((jtype==sim_joint_revolute_subtype)&&cyclic)
        {
            _cyclicJointFlags[i]=1;
            _jointMinVals[i]=-3.141592653589f;
            _jointRanges[i]=6.283185307179f;
        }
        else
        {
            _cyclicJointFlags[i]=0;
            _jointMinVals[i]=minV;
            _jointRanges[i]=rangeV;
        }
    }

    _ikGroupId=ikGroupId;
    _baseFrameId=baseFrameId;
    _tipFrameId=tipFrameId;
    _tipObject=(CDummy3DObject*)_simGetObject_internal(_tipFrameId);

    CDummy3DObject* baseObject=(CDummy3DObject*)_simGetObject_internal(baseFrameId);
    _simGetObjectCumulativeTransformation_internal(baseObject,_baseFrameTransformation.X.data,_baseFrameTransformation.Q.data,true);
    _baseFrameTransformationInverse=_baseFrameTransformation.getInverse();

    _robotSelfCollEntity1=robotSelfCollEntity1;
    _robotSelfCollEntity2=robotSelfCollEntity2;
    _robotEntity=robotEntity;
    _obstacleEntity=obstacleEntity;
    _distanceThreshold=distanceThreshold;

    for (int i=0;i<4;i++)
        _tipMetric[i]=tipMetricAndIkElConstraints[i];

    _ikElementConstraints=(int)tipMetricAndIkElConstraints[4];

    _ikGroup=(CDummyIkGroup*)_simGetIkGroupObject_internal(_ikGroupId);

    _tipDummy=(CDummyDummy*)_simGetObject_internal(_tipFrameId);
    if (_tipDummy!=nullptr)
    {
        int targetDummyID;
        _simGetDummyLinkType_internal(_tipDummy,&targetDummyID);
        _targetDummy=(CDummyDummy*)_simGetObject_internal(targetDummyID);
    }
    else
        _targetDummy=nullptr;
}

int CmpObject::getPhase1NodesRenderData(int index,float** pos)
{
    if (index>=int(_allNodes.size()))
        return(-1);
    pos[0]=_allNodes[index]->tipPose.X.data;
    return(1);
}

/* KEEP!!
int CmpObject::getPhase1ConnectionData(int index,int cIndex,float** pos)
{
    if (index>=int(_allNodes.size()))
        return(-1);
    int neighbourInd=_allNodes[index]->neighbourIndices[cIndex];
    if (neighbourInd==-1)
        return(-1);
    pos[0]=_allNodes[neighbourInd]->tipPose.X.data;
    return(_allNodes[neighbourInd]->attributes&1);
}
*/

int CmpObject::getPhase2NodesRenderData(unsigned char what,int index,float** pos1,float** pos2)
{ // what=0 --> from start, what=1 --> from goal, what=2 --> found path
    if (what==0)
    {
        index++;
        if (index>=int(_nodeListFromStart.size()))
            return(-1);
        pos1[0]=_nodeListFromStart[index]->tipTransf.X.data;
        pos2[0]=_nodeListFromStart[index]->parentNode->tipTransf.X.data;
        return(0);
    }
    if (what==1)
    {
        index++;
        if (index>=int(_nodeListFromGoal.size()))
            return(-1);
        pos1[0]=_nodeListFromGoal[index]->tipTransf.X.data;
        pos2[0]=_nodeListFromGoal[index]->parentNode->tipTransf.X.data;
        return(0);
    }
    if (what==2)
    {
        if (index>=int(_nodeListFoundPath.size())-1)
            return(-1);
        pos1[0]=_nodeListFoundPath[index]->tipTransf.X.data;
        pos2[0]=_nodeListFoundPath[index+1]->tipTransf.X.data;
        return(0);
    }
    return(-1);
}

char* CmpObject::getSerializationData(int cnt[1])
{
    cnt[0]=1+(int)_allNodes.size()/8;
    char* dat=new char[cnt[0]];
    for (int i=0;i<1+int(_allNodes.size())/8;i++)
        dat[i]=0;
    /*
    for (int i=0;i<int(_allNodes.size());i++)
    {
        if (_allNodes[i]->attributes&1)
            dat[i>>3]|=(1<<(i&7));
    }
    */
    return(dat);
}

void CmpObject::_clearAllPhase1Nodes()
{
    for (int i=0;i<int(_allNodes.size());i++)
        delete _allNodes[i];
    _allNodes.clear();
    std::vector<CmpPhase1Node*>(_allNodes).swap(_allNodes);
}

int CmpObject::getPhase1NodeCnt(char collidingNodesOnly)
{
    if (collidingNodesOnly==0)
        return(int(_allNodes.size()));
    int cnt=0;
    return(cnt);
}

char CmpObject::calculateNodes(const char* serializationData,int serializationDataSize)
{
    _serializationData=serializationData;
    _serializationDataSize=serializationDataSize;
    CDummy3DObject* baseFrameObj=(CDummy3DObject*)_simGetObject_internal(_baseFrameId);
    CDummy3DObject* tipFrameObj=(CDummy3DObject*)_simGetObject_internal(_tipFrameId);
    if ((baseFrameObj==nullptr)||(tipFrameObj==nullptr))
        return(0);
    std::vector<float> jointPositions;
    std::vector<float> jointRangesSpecial;
    std::vector<int> loopsProgress;
    std::vector<int> loopNeighbourStepSize(_jointHandles.size(),0);
    int cumulStepSizes=1;
    for (int i=0;i<int(_jointHandles.size());i++)
    {
        CDummyJoint* theJ=_jointObjects[i];
        if (theJ==nullptr)
            return(0);
        int jtype=_simGetJointType_internal(theJ);
        if (jtype==sim_joint_spherical_subtype)
            return(0);
        float minV,rangeV;
        bool cyclic=!_simGetJointPositionInterval_internal(theJ,&minV,&rangeV);
        if ((jtype==sim_joint_revolute_subtype)&&cyclic)
        { // cyclic joints
            jointRangesSpecial.push_back((6.283185307179f*float(_jointStepCount[i]-1))/float(_jointStepCount[i])); // here we "cheat" so that it works ok in the "_prepareNodes" function
        }
        else
        { // non-cyclic joints
            jointRangesSpecial.push_back(rangeV);
        }
        jointPositions.push_back(0.0f); // value doesn't matter
        loopsProgress.push_back(0); // value doesn't matter
        loopNeighbourStepSize[int(_jointHandles.size())-i-1]=cumulStepSizes;
        cumulStepSizes*=_jointStepCount[int(_jointHandles.size())-i-1];
    }

    C7Vector baseInverse;
    _simGetObjectCumulativeTransformation_internal(baseFrameObj,baseInverse.X.data,baseInverse.Q.data,1);
    baseInverse.inverse();
    _prepareNodes(0,&loopsProgress[0],&loopNeighbourStepSize[0],&jointPositions[0],&jointRangesSpecial[0],baseInverse,tipFrameObj);

    return(1);
}

void CmpObject::_prepareNodes(int loopIndex,int* loopsProgress,int* loopNeighbourStepSize,float* jointPositions,const float* jointRangesSpecial,const C7Vector& baseInverse,const CDummy3DObject* tipObject)
{
    float v=_jointMinVals[loopIndex];
    float vdx=jointRangesSpecial[loopIndex]/(float(_jointStepCount[loopIndex]-1));
    for (int i=0;i<_jointStepCount[loopIndex];i++)
    {
        loopsProgress[loopIndex]=i;
        jointPositions[loopIndex]=v;
        v+=vdx;
        if (loopIndex==int(_jointHandles.size())-1)
        {
            for (int j=0;j<int(_jointHandles.size());j++)
                _simSetJointPosition_internal(_jointObjects[j],jointPositions[j]);
            C7Vector tip;
            _simGetObjectCumulativeTransformation_internal(tipObject,tip.X.data,tip.Q.data,1);
            tip=baseInverse*tip;
            CmpPhase1Node* newNode=new CmpPhase1Node(int(_jointHandles.size()),jointPositions,tip);
            // set-up the list of neighbours:
            for (int j=0;j<int(_jointHandles.size());j++)
            {
                for (int k=-1;k<=1;k+=2)
                {
                    int off;
                    bool valid=true;
                    if (k<0)
                    { // indexing backbards
                        if ((loopsProgress[j]-1)<0)
                        {
                            off=(_jointStepCount[j]-1)*loopNeighbourStepSize[j];
                            if (_cyclicJointFlags[j]==0)
                                valid=false;
                        }
                        else
                            off=-1*loopNeighbourStepSize[j];
                    }
                    else
                    { // indexing forwards
                        if (loopsProgress[j]>=_jointStepCount[j]-1)
                        {
                            off=-(_jointStepCount[j]-1)*loopNeighbourStepSize[j];
                            if (_cyclicJointFlags[j]==0)
                                valid=false;
                        }
                        else
                            off=+1*loopNeighbourStepSize[j];
                    }
                    /* KEEP!!!!
                    if (valid)
                        newNode->setNeighbours(nInd++,currentIndex+off);
                    else
                        newNode->setNeighbours(nInd++,-1);
                        */
                }
            }
            // add the node:
            _allNodes.push_back(newNode);
        }
        else
            _prepareNodes(loopIndex+1,loopsProgress,loopNeighbourStepSize,jointPositions,jointRangesSpecial,baseInverse,tipObject);
    }
}

int CmpObject::getRobotConfigFromTipPose(const float* tipPos,const float* tipQuat,int options,float* robotJoints,const float* constraints,const float* configConstraints,int trialCount,float tipCloseDistance,const float* referenceConfigs,int configCount,const int* jointBehaviour,int correctionPasses,int maxTimeInMs)
{ // returns 0 if it failed, otherwise the number of trials before success

    // configCount: a list of robot configurations
    // if configCount==0: we just select any config (actually the node that is closest in space to tipPose)
    // if ((options&2)==0): we select configs that are close to the provided config (one provided config only is taken into account)
    // if ((options&2)!=0): we select configs that are away from the provided configs (all the provided configs are taken into account)

    // trialCount: The max number of IK tries before giving up
    // if trialCount==0: when we call this for the first time and we just want to prepare a list of close nodes.
    // if ((options&1)==0) when we call this for the first time (for a given calculation). We will prepare a list of close nodes.
    // if ((options&1)!=0) when we want to take advantage of previous calls and take into account nodes we already tried.

    // configConstraints: for each joint there is a weight, that tells how important the joint is in closest/farthest configuration calculations
    //                    normally one would use 1.0f for every joint. But if for instance we want to find configurations that have their
    //                    first joint close to the specified referenceConfigs, one would use 0.0f for all entries, except for the first one

    // configConstraints and referenceConfigs can be nullptr
    // tipPose needs to be relative to the Ik base!

    // if ((options&4)!=0) we use IK in an earlier stage to determine which config to pick. WIll produce closer matches with a given config, but slower
    // if ((options&8)!=0) we output some activity to the console
    // if ((options&16)!=0) we don't check robot self-collisions
    // if ((options&32)!=0) we don't check robot-environment collisions

    int collisionCheckMask=1+2; // 1=self-collision, 2=robot-environment
    if ((options&16)!=0)
        collisionCheckMask-=1;
    if ((options&32)!=0)
        collisionCheckMask-=2;

    C7Vector tipPose;
    tipPose.X.setInternalData(tipPos);
    tipPose.Q.setInternalData(tipQuat);

    bool takeIntoAccountPreviousCalculations=((options&1)!=0); // take into account previous calls to this MP task!
    bool preciseMode=((options&4)!=0);
    bool outputActivity=((options&8)!=0);
    bool keepAway=((options&2)!=0);

    if (keepAway)
        correctionPasses=0; // works only when we want to be close to a given configuration
    if (referenceConfigs==nullptr)
        configCount=0;
    if (configCount==0)
        correctionPasses=0;
    std::vector<float> jointWeights(_jointHandles.size(),1.0f);
    if (configConstraints!=nullptr)
    {
        for (int i=0;i<int(_jointHandles.size());i++)
            jointWeights[i]=configConstraints[i];
    }

    if (_allNodes.size()==0)
        return(0);
    C4X4Matrix tipMatrix(tipPose.getMatrix());

    // Get some objects and transformations first:
    CDummy3DObject* baseObject=(CDummy3DObject*)_simGetObject_internal(_baseFrameId);
    if ((baseObject==nullptr)||(_tipDummy==nullptr))
        return(0);
    if (_targetDummy==nullptr)
        return(0);
    CDummy3DObject* targetParent=(CDummy3DObject*)_simGetParentObject_internal(_targetDummy);
//  if ((targetParent==nullptr)||(_ikGroup==nullptr))
    if (_ikGroup==nullptr)
        return(0);

    C7Vector targetParentTr;
    targetParentTr.setIdentity();
    if (targetParent!=nullptr)
        _simGetObjectCumulativeTransformation_internal(targetParent,targetParentTr.X.data,targetParentTr.Q.data,0);
    C7Vector baseTr;
    _simGetObjectCumulativeTransformation_internal(baseObject,baseTr.X.data,baseTr.Q.data,1);

    
    bool firstTime=false;

    if (!takeIntoAccountPreviousCalculations)
    { // Clear the "node already tried"-flags:
        for (int i=0;i<int(_allNodes.size());i++)
            _allNodes[i]->attributes&=253; // clear bit 1 (i.e. 2)
        _closeNodesIndices.clear();
        firstTime=true;
    }


    if (firstTime)
    { // we prepare the smaller selection of nodes:
        if (outputActivity)
            printf("Preparing smaller selection of nodes (i.e. 'close nodes' selection)\n");

        // we go through all nodes
        for (int i=0;i<int(_allNodes.size());i++)
        {
            CmpPhase1Node* node=_allNodes[i];
            if ((node->attributes&2)==0)
            { // not yet checked
                float d=_getErrorSquared(tipMatrix,node->tipPose.getMatrix(),constraints);
                if (d<tipCloseDistance*tipCloseDistance)
                    _closeNodesIndices.push_back(i);
            }
        }
        if (outputActivity)
            printf("Selected nodes count: %i\n",int(_closeNodesIndices.size()));
        if (trialCount==0)
            return(0); // special case
    }

    // Search here:
    int retValue=0;
    // first save the current joint positions:
    std::vector<float> savedJointPositions;
    for (int i=0;i<int(_jointHandles.size());i++)
    {
        CDummyJoint* jt=(CDummyJoint*)_simGetObject_internal(_jointHandles[i]);
        savedJointPositions.push_back(_simGetJointPosition_internal(jt));
    }

    std::vector<float> tmpJointPositions(_jointHandles.size(),0.0f);
    C7Vector targetNewLocal(targetParentTr.getInverse()*baseTr*tipPose);

    if (outputActivity)
        printf("Now searching an appropriate configuration...\n");
    int startTime=simGetSystemTimeInMs_internal(-1);
    for (int tcnt=0;tcnt<trialCount;tcnt++)
    { // Search a maximum of 'trialCount' times
        if (outputActivity)
            printf("Trial count: %i\n",tcnt+1);
        // Find the closest node that is self-collision-free and that does not have the 'node already tried' flag:
        float minD=999999999.0f;
        float maxD=0.0f;
        int bestIndex=-1;
        for (int i=0;i<int(_closeNodesIndices.size());i++)
        { // we go only through 'close' nodes
            CmpPhase1Node* node=_allNodes[_closeNodesIndices[i]];
            if ((node->attributes&2)==0)
            { // not yet checked
                if (configCount==0)
                { // we have no initial config to compare with. So we just take the closest in space (6 dim)
                    float d=_getErrorSquared(tipMatrix,node->tipPose.getMatrix(),constraints);
                    if (d<minD)
                    {
                        minD=d;
                        bestIndex=_closeNodesIndices[i];
                    }
                }
                else
                {
                    bool doIt=true;
                    if (preciseMode)
                    { // but slower
                        doIt=_performIkThenCheckForCollision(&node->jointPositions[0],&tmpJointPositions[0],targetNewLocal,referenceConfigs,jointBehaviour,correctionPasses,collisionCheckMask);
                        //printf(".");
                    }
                    else
                    {
                        for (int j=0;j<int(_jointHandles.size());j++)
                            tmpJointPositions[j]=node->jointPositions[j];
                    }

                    if (doIt)
                    {
                        if (keepAway)
                        { // we try to keep furthest away from the provided config(s)
                            minD=9999999999.0f;
                            for (int j=0;j<configCount;j++)
                            {
                                float d=_getConfigErrorSquared(referenceConfigs+j*int(_jointHandles.size()),&tmpJointPositions[0],&jointWeights[0]);
                                if (d<minD)
                                    minD=d;
                            }
                            if (minD>maxD)
                            {
                                maxD=minD;
                                bestIndex=_closeNodesIndices[i];
                            }
                        }
                        else
                        { // we try to keep closest to the provided config
                            float d=_getConfigErrorSquared(referenceConfigs,&tmpJointPositions[0],&jointWeights[0]);
                            if (d<minD)
                            {
                                minD=d;
                                bestIndex=_closeNodesIndices[i];
                            }
                        }
                    }
                }
            }

            if ((maxTimeInMs!=0)&&(_simGetTimeDiffInMs_internal(startTime)>maxTimeInMs))
                break;
        }

        if (bestIndex==-1)
        {
            if (outputActivity)
                printf("Did not find an appropriate configuration!\n");
            return(0); // we didn't find any node satisfying the requirements
        }
        // Ok, we have the closest node now
        _allNodes[bestIndex]->attributes|=2; // flag that node as "tried"

        // Now let's move the robot via IK into the desired position:
        if (_performIkThenCheckForCollision(&_allNodes[bestIndex]->jointPositions[0],robotJoints,targetNewLocal,referenceConfigs,jointBehaviour,correctionPasses,collisionCheckMask))
        {
            retValue=tcnt+1;
            break;
        }
        if ((maxTimeInMs!=0)&&(_simGetTimeDiffInMs_internal(startTime)>maxTimeInMs))
            break;
    }
    // restore the joint positions:
    for (int i=0;i<int(_jointHandles.size());i++)
    {
        CDummyJoint* jt=(CDummyJoint*)_simGetObject_internal(_jointHandles[i]);
        _simSetJointPosition_internal(jt,savedJointPositions[i]);
    }

    if (outputActivity)
    {
        if (retValue>0)
            printf("Found an appropriate configuration.\n");
        else
            printf("Did not find an appropriate configuration!\n");
    }

    return(retValue);
}

bool CmpObject::_performIkThenCheckForCollision(const float* initialJointPositions,float* finalJointPositions,const C7Vector& targetNewLocal,const float* desiredJointPositions,const int* jointBehaviour,int correctionPasses,int collisionCheckMask)
{
    if ((jointBehaviour==nullptr)||(desiredJointPositions==nullptr)||(correctionPasses==0))
        return(_performIkThenCheckForCollision__(initialJointPositions,finalJointPositions,targetNewLocal,collisionCheckMask));
    if (_performIkThenCheckForCollision__(initialJointPositions,finalJointPositions,targetNewLocal,0))
    { // ok, we reached the desired tip configuration. Now let's try to come closer to the desired joint configurations:
        // First, lock a few joints (i.e. so that they are ignored during ik calculations):
        for (int i=0;i<int(_jointHandles.size());i++)
        {
            if (jointBehaviour[i]==-1)
            {
                simSetJointMode_internal(_jointHandles[i],sim_jointmode_passive,0);
//              simSetJointPosition_internal(_jointHandles[i],desiredJointPositions[i]);
            }
        }

        // The correction calculation passes:
        bool success=true;
        for (int passes=0;passes<correctionPasses;passes++)
        {
            std::vector<float> initJointPos(_jointHandles.size(),0.0f);
            // move some joints a bit towards their desired position:
            for (int i=0;i<int(_jointHandles.size());i++)
            {
                initJointPos[i]=finalJointPositions[i];
                if (jointBehaviour[i]>0)
                {
                    float error=desiredJointPositions[i]-initialJointPositions[i];
                    if (_cyclicJointFlags[i]!=0)
                    { // cyclic joints.
                        if (error>=0.0f)
                            error=fmod(error+3.141592653589f,6.283185307179f)-3.141592653589f;
                        else
                            error=fmod(error-3.141592653589f,6.283185307179f)+3.141592653589f;
                    }
                    if (fabs(error)>float(jointBehaviour[i]))
                        error=(float(jointBehaviour[i])*error/fabs(error)); // the error is too big to be achieved in a single step
                    initJointPos[i]+=error;
                }
                if (jointBehaviour[i]==-1)
                    initJointPos[i]=desiredJointPositions[i];
            }

            // ...and perform IK:
            success=_performIkThenCheckForCollision__(&initJointPos[0],finalJointPositions,targetNewLocal,collisionCheckMask);
            if (!success)
                break;
        }

        // Now unlock a few joints:
        for (int i=0;i<int(_jointHandles.size());i++)
        {
            if (jointBehaviour[i]==-1)
                simSetJointMode_internal(_jointHandles[i],sim_jointmode_ik,0);
        }

        return(success);
    }
    return(false);
}

bool CmpObject::_performIkThenCheckForCollision__(const float* initialJointPositions,float* finalJointPositions,const C7Vector& targetNewLocal,int collisionCheckMask)
{ // return true means success
    // Let's move the robot via IK into the desired position (initialJointPositions needs to be 'quite close'):

    // 1. Set the robot to the "close" configuration:
    for (int i=0;i<int(_jointHandles.size());i++)
        _simSetJointPosition_internal(_jointObjects[i],initialJointPositions[i]);

    // 2. Position the target correctly:
    _simSetObjectLocalTransformation_internal(_targetDummy,targetNewLocal.X.data,targetNewLocal.Q.data);

    // 3. Execute the IK:
    int res=_simMpHandleIkGroupObject_internal(_ikGroup);
    if (res==sim_ikresult_success)
    { // ok, IK succeeded. Now let's make sure that configuration is collision-free:
        // Check for collision/distance threshold:
        bool ok=true;

        if (ok&&((collisionCheckMask&1)!=0)&&(_robotSelfCollEntity1!=-1))
        { // robot-robot
            int buffer[4]={-1,-1,-1,-1};
            if (_distanceThreshold<=0.0f)
            { // we check for collision:
                ok=(_simDoEntitiesCollide_internal(_robotSelfCollEntity1,_robotSelfCollEntity2,buffer,0,0,1)==0);
            }
            else
            { // we check for a minimum distance:
                float ray[7];
                float theDist=_distanceThreshold;
                ok=(_simGetDistanceBetweenEntitiesIfSmaller_internal(_robotSelfCollEntity1,_robotSelfCollEntity2,&theDist,ray,buffer,0,0,1)==0);
            }
        }

        if (ok&&((collisionCheckMask&2)!=0)&&(_robotEntity!=-1))
        { // robot-environment
            int buffer[4]={-1,-1,-1,-1};
            if (_distanceThreshold<=0.0f)
            { // we check for collision:
                ok=(_simDoEntitiesCollide_internal(_robotEntity,_obstacleEntity,buffer,0,0,1)==0);
            }
            else
            { // we check for a minimum distance:
                float ray[7];
                float theDist=_distanceThreshold;
                ok=(_simGetDistanceBetweenEntitiesIfSmaller_internal(_robotEntity,_obstacleEntity,&theDist,ray,buffer,0,0,1)==0);
            }
        }

        if (ok)
        { // we are not colliding, and are within the tolerance of the IK group
            for (int i=0;i<int(_jointHandles.size());i++)
            {
                CDummyJoint* jt=(CDummyJoint*)_simGetObject_internal(_jointHandles[i]);
                finalJointPositions[i]=_simGetJointPosition_internal(jt);
            }
            return(true);
        }
    }
    return(false);
}

float CmpObject::_getErrorSquared(const C4X4Matrix& m1,const C4X4Matrix& m2,const float constraints[5])
{   //constraints[0]-constraints[2] represent the lin. constraint weights (x,y,z), constraints[3] represents the alpha-beta constraint weight, constraints[4] the gamma constraint. If constraints[4] is 1.0, constraints[3] should also be
    C3Vector p(m1.X-m2.X);
    float err=0.0f;

    // linear error:
    err+=p(0)*p(0)*_tipMetric[0]*constraints[0];
    err+=p(1)*p(1)*_tipMetric[1]*constraints[1];
    err+=p(2)*p(2)*_tipMetric[2]*constraints[2];

    // angular error:
    float x=m1.M.axis[0]*m2.M.axis[0];
    float z=m1.M.axis[2]*m2.M.axis[2];
    // following is a robust fabs(acosf(x))
    float v0;
    if (x>=1.0f)
        v0=0.0f;
    else if (x<=-1.0f)
        v0=piValue_f;
    else
        v0=fabs(acosf(x));
    v0*=constraints[4];

    // following is a robust fabs(acosf(z))
    float v1;
    if (z>=1.0f)
        v1=0.0f;
    else if (z<=-1.0f)
        v1=piValue_f;
    else
        v1=fabs(acosf(z));
    v1*=constraints[3];

    if (v1>v0)
        err+=v1*v1*_tipMetric[3];
    else
        err+=v0*v0*_tipMetric[3];
    return(err);
}

float CmpObject::_getConfigErrorSquared(const float* config1,const float* config2,const float* configConstraints)
{
    float err=0.0f;
    float dx;
    int cnt=(int)_jointHandles.size();
    for (int i=0;i<cnt;i++)
    {
        dx=config1[i]-config2[i]; 
        if (_cyclicJointFlags[i]!=0)
        { // revolute, cyclic joints:
            if (dx>=0.0f)
                dx=fmod(dx+3.141592653589f,6.283185307179f)-3.141592653589f;
            else
                dx=fmod(dx-3.141592653589f,6.283185307179f)+3.141592653589f;
/*
            dx=fmod(dx,6.283185307179f);
            if (dx<0.0f)
                dx+=6.283185307179f;
            if (dx>3.141592653589f)
                dx=6.283185307179f-dx;
            else
                dx=-dx;
                */
        }
        err+=dx*dx*_robotMetric[i]*_robotMetric[i]*configConstraints[i]*configConstraints[i];
    }
    return(err);
}

float* CmpObject::findPath(const float* startConfig,const float* goalConfig,int options,float stepSize,int* outputConfigsCnt,int maxTimeInMs,const int* auxIntParams,const float* auxFloatParams,int randomSeed)
{
    // if ((options&8)!=0) we output some activity to the console
    // if ((options&16)!=0) we don't check robot self-collisions
    // if ((options&32)!=0) we don't check robot-environment collisions
    // if ((options&64)!=0) we do path simplification here
    // if ((options&256)!=0) we return the distance without orientation component

    bool activityToConsole=((options&8)!=0);
    bool simplifyFoundPath=((options&64)!=0);
    bool returnedDistanceHasNoOrientationComponent=((options&256)!=0);

    int collisionCheckMask=1+2; // 1=self-collision, 2=robot-environment
    if ((options&16)!=0)
        collisionCheckMask-=1;
    if ((options&32)!=0)
        collisionCheckMask-=2;

    outputConfigsCnt[0]=0;
    _clearAllPhase2Nodes();

    C7Vector tipStartTransf,tipGoalTransf;
    if (_applyJointPosToRobotAndCheckForCollisions_phase2(startConfig,tipStartTransf,collisionCheckMask)||_applyJointPosToRobotAndCheckForCollisions_phase2(goalConfig,tipGoalTransf,collisionCheckMask))
    {
        if (activityToConsole)
            printf("Start or goal configuration is colliding.\n");
        return(nullptr);
    }

    float* retVal=nullptr;

    CmpPhase2Node* startN=new CmpPhase2Node((int)_cyclicJointFlags.size(),startConfig,tipStartTransf);
    _nodeListFromStart.push_back(startN);
    CmpPhase2Node* goalN=new CmpPhase2Node((int)_cyclicJointFlags.size(),goalConfig,tipGoalTransf);
    _nodeListFromGoal.push_back(goalN);
    bool foundAPath=false;

    std::vector<float> constr(_jointHandles.size(),1.0f);
    if (_getConfigErrorSquared(startN->jointPositions,goalN->jointPositions,&constr[0])>0.00001f)
    { // ok, the start and goal configs are far enough
        std::vector<CmpPhase2Node*>* current=&_nodeListFromStart;
        std::vector<CmpPhase2Node*>* nextCurrent=&_nodeListFromGoal;
        int startTime=simGetSystemTimeInMs_internal(-1);
        bool firstPass=true;
        while ((maxTimeInMs==0)||(_simGetTimeDiffInMs_internal(startTime)<maxTimeInMs))
        {
            for (int altern=0;altern<2;altern++)
            {
                CmpPhase2Node* randNode;
                if (firstPass)
                { // if first pass, we try to link start to goal directly!
                    randNode=new CmpPhase2Node((int)_cyclicJointFlags.size(),goalN->jointPositions,goalN->tipTransf);
                }
                else
                { // we have to create a random node:
                    std::vector<float> pos;
                    static boost::random::mt19937 randomNumbergenerator;
                    if (randomSeed!=-1)
                        randomNumbergenerator.seed(randomSeed);
                    boost::random::uniform_real_distribution<> uniformDistribution(0.0,1.0);
                    boost::variate_generator<boost::random::mt19937&, boost::random::uniform_real_distribution < > > generateRandomNumbers(randomNumbergenerator,uniformDistribution);

//                  for (int i=0;i<int(_cyclicJointFlags.size());i++)
//                      pos.push_back(_jointMinVals[i]+_jointRanges[i]*SIM_RAND_FLOAT);
                    for (int i=0;i<int(_cyclicJointFlags.size());i++)
                        pos.push_back(_jointMinVals[i]+_jointRanges[i]*float(generateRandomNumbers()));
                    randNode=new CmpPhase2Node((int)_cyclicJointFlags.size(),&pos[0],tipStartTransf); // last argument is not used
                }
                if (activityToConsole)
                    printf("*");
                CmpPhase2Node* closest=_getPhase2ClosestNode(*current,randNode);
                closest=_extendNode(*current,closest,randNode,stepSize,false,collisionCheckMask);
                if (closest!=nullptr)
                { // ok, we could extend the node a bit
                    // Let's try to connect the extension to the other list:
                    CmpPhase2Node* closestConnect=_getPhase2ClosestNode(*nextCurrent,closest);
                    closestConnect=_extendNode(*nextCurrent,closestConnect,closest,stepSize,true,collisionCheckMask);
                    if (closestConnect!=nullptr)
                    { // ok, we can perform the connection. We have found a path!
                        if (activityToConsole)
                            printf("Found a path from start to goal.\n");
                        foundAPath=true;
                        if (current==&_nodeListFromStart)
                        { // current list is from start
                            CmpPhase2Node* iterat=closest;
                            while (iterat!=nullptr)
                            {
                                _nodeListFoundPath.insert(_nodeListFoundPath.begin(),iterat->copyYourself((int)_cyclicJointFlags.size()));
                                iterat=iterat->parentNode;
                            }
                            iterat=closestConnect;
                            while (iterat!=nullptr)
                            {
                                _nodeListFoundPath.push_back(iterat->copyYourself((int)_cyclicJointFlags.size()));
                                iterat=iterat->parentNode;
                            }
                        }
                        else
                        { // current list is from goal
                            CmpPhase2Node* iterat=closest;
                            while (iterat!=nullptr)
                            {
                                _nodeListFoundPath.push_back(iterat->copyYourself((int)_cyclicJointFlags.size()));
                                iterat=iterat->parentNode;
                            }
                            iterat=closestConnect;
                            while (iterat!=nullptr)
                            {
                                _nodeListFoundPath.insert(_nodeListFoundPath.begin(),iterat->copyYourself((int)_cyclicJointFlags.size()));
                                iterat=iterat->parentNode;
                            }
                        }
                    }
                }
                delete randNode;
                std::vector<CmpPhase2Node*>* tmpCurrent=nextCurrent;
                nextCurrent=current;
                current=tmpCurrent;
                if (foundAPath)
                    break;
                firstPass=false;
            }
            if (foundAPath)
                break;
        }

        if (foundAPath&&(!firstPass))
        {
            if (simplifyFoundPath)
            {
                if (!_simplifyFoundPath(auxIntParams,auxFloatParams,1,stepSize,activityToConsole,collisionCheckMask,999999))
                { // should never happen
                    foundAPath=false;
                }
            }
        }

    }
    else
    { // the start and goal configs are too close, maybe even coincident.
        // We just add a single configuration point to notify this
        if (activityToConsole)
            printf("Start and goal are coincident. Returning a single configuration.\n");
        foundAPath=true;
        _nodeListFoundPath.push_back(goalN->copyYourself((int)_jointHandles.size()));
    }



    if (foundAPath)
    {
        // prepare the return buffer (joint configs + length values + tip pos + tip orient + tip length values):
        int dofs=(int)_jointHandles.size();
        int configs=(int)_nodeListFoundPath.size();

        outputConfigsCnt[0]=configs;

        float l=0.0f;
        float l2=0.0f;

        retVal=(float*)new char[(configs*dofs + configs + configs*3 + configs*4 + configs)*4];
        for (int i=0;i<configs;i++)
        {
            for (int j=0;j<dofs;j++)
                retVal[i*dofs+j]=_nodeListFoundPath[i]->jointPositions[j];

            if (i!=0)
                l+=sqrtf(_getConfigErrorSquared(_nodeListFoundPath[i-1]->jointPositions,_nodeListFoundPath[i+0]->jointPositions,&_robotMetric[0]));
            retVal[configs*dofs+i]=l;

            C7Vector abs(_baseFrameTransformation*_nodeListFoundPath[i]->tipTransf);
            retVal[configs*dofs+configs+3*i+0]=abs.X(0);
            retVal[configs*dofs+configs+3*i+1]=abs.X(1);
            retVal[configs*dofs+configs+3*i+2]=abs.X(2);

            // V-REP quaternion, internally: w x y z
            // V-REP quaternion, at interfaces: x y z w
            retVal[configs*dofs+configs+3*configs+4*i+0]=abs.Q(1);
            retVal[configs*dofs+configs+3*configs+4*i+1]=abs.Q(2);
            retVal[configs*dofs+configs+3*configs+4*i+2]=abs.Q(3);
            retVal[configs*dofs+configs+3*configs+4*i+3]=abs.Q(0);

            if (i!=0)
            {
                C3Vector w(_nodeListFoundPath[i-1]->tipTransf.X-_nodeListFoundPath[i-0]->tipTransf.X);
                float a=0.0f;
                if (!returnedDistanceHasNoOrientationComponent)
                    a=_nodeListFoundPath[i-1]->tipTransf.Q.getAngleBetweenQuaternions(_nodeListFoundPath[i-0]->tipTransf.Q);
                float dx[4]={w(0)*_tipMetric[0],w(1)*_tipMetric[1],w(2)*_tipMetric[2],a*_tipMetric[3]};
                l2+=sqrtf(dx[0]*dx[0]+dx[1]*dx[1]+dx[2]*dx[2]+dx[3]*dx[3]);
            }
            retVal[configs*dofs+configs+3*configs+4*configs+i]=l2;
        }
    }

    return(retVal);
}




CmpPhase2Node* CmpObject::_extendNode(std::vector<CmpPhase2Node*>& nodeList,CmpPhase2Node* toBeExtendedNode,CmpPhase2Node* goalNode,float stepSize,bool connect,int collisionCheckMask)
{   // return value is !=nullptr if extension was performed and connect is false
    // If connect is true, then return value indicates that connection can be performed
    std::vector<float> extensionVectorPiece(_cyclicJointFlags.size(),0.0f);
    std::vector<float> jointPos(_cyclicJointFlags.size(),0.0f);
    int passes=_getPhase2Vector(toBeExtendedNode,goalNode,stepSize,&extensionVectorPiece[0]);
    bool leave=false;
    int insertedNodes=0;
    for (int currentPass=0;currentPass<passes;currentPass++)
    {
        if (currentPass==passes-1)
        {
            if (connect)
                return(toBeExtendedNode);
            leave=true;
        }
        for (int i=0;i<int(_cyclicJointFlags.size());i++)
            jointPos[i]=toBeExtendedNode->jointPositions[i]+extensionVectorPiece[i];

        C7Vector tipTransf; // relative to base object
        if (_applyJointPosToRobotAndCheckForCollisions_phase2(&jointPos[0],tipTransf,collisionCheckMask))
        { // we collided
            if (connect)
                return(nullptr);
            if (insertedNodes!=0)
                return(toBeExtendedNode);
            return(nullptr);
        }

        // no collision in this configuration. We extend the node:
        CmpPhase2Node* newNode=new CmpPhase2Node((int)_cyclicJointFlags.size(),&jointPos[0],tipTransf);
        newNode->parentNode=toBeExtendedNode;
        nodeList.push_back(newNode);
        toBeExtendedNode=newNode;
        if (leave)
            return(toBeExtendedNode);
        insertedNodes++;
    }
    return(nullptr);
}

bool CmpObject::_applyJointPosToRobotAndCheckForCollisions_phase2(const float* jointValues,C7Vector& tipTransf,int collisionCheckMask)
{   // return value is the collision state (true--> colliding)
    // tipPos is relative to the base frame

    for (int j=0;j<int(_jointHandles.size());j++)
        _simSetJointPosition_internal(_jointObjects[j],jointValues[j]);

    // Check for collision/distance threshold:


    if ((_robotSelfCollEntity1!=-1)&&((collisionCheckMask&1)!=0))
    { // robot-robot collision
        int buffer[4]={-1,-1,-1,-1};
        if (_distanceThreshold<=0.0f)
        { // we check for collision:
            if (_simDoEntitiesCollide_internal(_robotSelfCollEntity1,_robotSelfCollEntity2,buffer,0,0,1)!=0)
                return(true); // colliding
        }
        else
        { // we check for a minimum distance:
            float ray[7];
            float theDist=_distanceThreshold;
            if (_simGetDistanceBetweenEntitiesIfSmaller_internal(_robotSelfCollEntity1,_robotSelfCollEntity2,&theDist,ray,buffer,0,0,1)!=0)
                return(true); // colliding
        }
    }

    if ((_robotEntity!=-1)&&((collisionCheckMask&2)!=0))
    { // robot-environment collision
        int buffer[4]={-1,-1,-1,-1};
        if (_distanceThreshold<=0.0f)
        { // we check for collision:
            if (_simDoEntitiesCollide_internal(_robotEntity,_obstacleEntity,buffer,0,0,1)!=0)
                return(true); // colliding
        }
        else
        { // we check for a minimum distance:
            float ray[7];
            float theDist=_distanceThreshold;
            if (_simGetDistanceBetweenEntitiesIfSmaller_internal(_robotEntity,_obstacleEntity,&theDist,ray,buffer,0,0,1)!=0)
                return(true); // colliding
        }
    }


    // ok, not colliding. Now get the position of the tip, relative to the base object:
    _simGetObjectCumulativeTransformation_internal(_tipObject,tipTransf.X.data,tipTransf.Q.data,true);
    tipTransf=_baseFrameTransformationInverse*tipTransf;
    return(false); // not colliding
}

int CmpObject::_getPhase2Vector(const CmpPhase2Node* startNode,const CmpPhase2Node* goalNode,float stepSize,float* returnVector)
{ // return value is the number of times we have to add 'returnVector' to 'startNode' in order to reach 'goalNode'
    return(_getPhase2Vector(&startNode->jointPositions[0],&goalNode->jointPositions[0],stepSize,returnVector));
}

int CmpObject::_getPhase2Vector(const float* startConfig,const float* goalConfig,float stepSize,float* returnVector)
{ // return value is the number of times we have to add 'returnVector' to 'startNode' in order to reach 'goalNode'
    float l=0.0f;
    for (int i=0;i<int(_cyclicJointFlags.size());i++)
    {
        float dx=goalConfig[i]-startConfig[i];
        if (_cyclicJointFlags[i]!=0)
        { // joint is cyclic
            if (dx>=0.0f)
                dx=fmod(dx+3.141592653589f,6.283185307179f)-3.141592653589f;
            else
                dx=fmod(dx-3.141592653589f,6.283185307179f)+3.141592653589f;
        }
        returnVector[i]=dx; // NO METRIC SCALING HERE, but below!!!
        l+=dx*_robotMetric[i]*dx*_robotMetric[i];
    }
    l=sqrtf(l);
    int retVal=int(l/stepSize)+1;
    float d=float(retVal);
    for (int i=0;i<int(_cyclicJointFlags.size());i++)
        returnVector[i]/=d;
    return(retVal);
}

CmpPhase2Node* CmpObject::_getPhase2ClosestNode(const std::vector<CmpPhase2Node*>& nodeList,const CmpPhase2Node* aNode)
{ // returns the node in the list 'nodeList' that is closest to 'aNode'
    float smallestSqDist=FLT_MAX;
    CmpPhase2Node* retNode=nullptr;
    for (int ni=0;ni<int(nodeList.size());ni++)
    {
        CmpPhase2Node* anotherNode=nodeList[ni];
        float l=0.0f;
        for (int i=0;i<int(_cyclicJointFlags.size());i++)
        {
            float dx=anotherNode->jointPositions[i]-aNode->jointPositions[i];
            if (_cyclicJointFlags[i]!=0)
            { // joint is cyclic
                if (dx>=0.0f)
                    dx=fmod(dx+3.141592653589f,6.283185307179f)-3.141592653589f;
                else
                    dx=fmod(dx-3.141592653589f,6.283185307179f)+3.141592653589f;
            }
            l+=dx*_robotMetric[i]*dx*_robotMetric[i];
        }
        if (l<smallestSqDist)
        {
            smallestSqDist=l;
            retNode=anotherNode;
        }
    }
    return(retNode);
}

void CmpObject::_clearAllPhase2Nodes()
{
    for (int i=0;i<int(_nodeListFromStart.size());i++)
        delete _nodeListFromStart[i];
    for (int i=0;i<int(_nodeListFromGoal.size());i++)
        delete _nodeListFromGoal[i];
    for (int i=0;i<int(_nodeListFoundPath.size());i++)
        delete _nodeListFoundPath[i];
    _nodeListFromStart.clear();
    _nodeListFromGoal.clear();
    _nodeListFoundPath.clear();
}

bool CmpObject::_simplifyFoundPath(const int* auxIntParams,const float* auxFloatParams,int incrementStep,float stepSize,bool activityToConsole,int collisionCheckMask,int maxTimeInMs)
{
    int startTime=simGetSystemTimeInMs_internal(-1);

    // Simplify from start to goal:
    if (_nodeListFoundPath.size()>2)
    {
        if (activityToConsole)
            printf("Simplifying path from start to goal.\n");
        if (!_simplifyFoundPath_pass(incrementStep,stepSize,activityToConsole,collisionCheckMask,maxTimeInMs))
            return(false);
    }
    // Simplify from goal to start:
    int restTime=maxTimeInMs-_simGetTimeDiffInMs_internal(startTime);
    bool success=(restTime>0);

    if (success)
    {
        if (_nodeListFoundPath.size()>2)
        {
            // inverse the list (revert direction):
            std::vector<CmpPhase2Node*> tmpPath(_nodeListFoundPath);
            for (int i=0;i<int(tmpPath.size());i++)
                _nodeListFoundPath[i]=tmpPath[tmpPath.size()-i-1];

            if (activityToConsole)
                printf("Simplifying path from goal to start.\n");
            _simplifyFoundPath_pass(incrementStep,stepSize,activityToConsole,collisionCheckMask,restTime);

            // inverse the list back to original direction:
            tmpPath.assign(_nodeListFoundPath.begin(),_nodeListFoundPath.end());
            for (int i=0;i<int(tmpPath.size());i++)
                _nodeListFoundPath[i]=tmpPath[tmpPath.size()-i-1];
        }
    }
    return(success);
}

C7Vector CmpObject::_getTipTranformationFromPhase2Node(const CmpPhase2Node& node)
{ // returned transformation is relative to base
    for (int i=0;i<int(_jointObjects.size());i++)
        _simSetJointPosition_internal(_jointObjects[i],node.jointPositions[i]);

    C7Vector tr;
    _simGetObjectCumulativeTransformation_internal(_tipObject,tr.X.data,tr.Q.data,1);
    tr=_baseFrameTransformationInverse*tr;
    return(tr);
}

//*
bool CmpObject::_simplifyFoundPath_pass(int incrementStep,float stepSize,bool activityToConsole,int collisionCheckMask,int maxTimeInMs)
{
    int startTime=simGetSystemTimeInMs_internal(-1);
    bool success=true;

    std::vector<CmpPhase2Node*> originalPath(_nodeListFoundPath);
    _nodeListFoundPath.clear();

    int bottomIndex=0;
    int topIndex=(int)originalPath.size()-1;
    _nodeListFoundPath.push_back(originalPath[0]->copyYourself((int)_jointHandles.size()));
    std::vector<float> extensionVectorPiece(_jointHandles.size(),0.0f);
    while (bottomIndex<topIndex)
    {
        if (activityToConsole)
            printf("o");
        CmpPhase2Node* bottomNode=_nodeListFoundPath[_nodeListFoundPath.size()-1];
        CmpPhase2Node* topNode=originalPath[topIndex];
        int cnt=_getPhase2Vector(bottomNode,topNode,stepSize,&extensionVectorPiece[0]);
        std::vector<float> jointPos;
        for (int i=0;i<int(_jointHandles.size());i++)
            jointPos.push_back(bottomNode->jointPositions[i]);
        bool failed=false;
        std::vector<CmpPhase2Node*> newNodes;
        for (int segi=0;segi<cnt;segi++)
        {
            for (int i=0;i<int(_jointHandles.size());i++)
                jointPos[i]+=extensionVectorPiece[i];

            C7Vector tipTransf; // relative to base object
            if (_applyJointPosToRobotAndCheckForCollisions_phase2(&jointPos[0],tipTransf,collisionCheckMask))
            { // we collided
                failed=true;
                break;
            }
            else
                newNodes.push_back(new CmpPhase2Node((int)_jointHandles.size(),&jointPos[0],tipTransf));
        }
        if (failed)
        {
            for (int i=0;i<int(newNodes.size());i++)
                delete newNodes[i];
            newNodes.clear();
            topIndex-=incrementStep;
            if (topIndex<=bottomIndex+1)
            {
                int incr=incrementStep;
                if (bottomIndex+incr>int(originalPath.size())-1)
                    incr=(int)originalPath.size()-1-bottomIndex;
                for (int i=0;i<incr;i++)
                    _nodeListFoundPath.push_back(originalPath[bottomIndex+1+i]->copyYourself((int)_jointHandles.size()));
                bottomIndex+=incr;
                topIndex=(int)originalPath.size()-1;
            }
        }
        else
        {
            _nodeListFoundPath.insert(_nodeListFoundPath.end(),newNodes.begin(),newNodes.end());
            newNodes.clear();
            bottomIndex=topIndex;
            topIndex=(int)originalPath.size()-1;
        }

        if (_simGetTimeDiffInMs_internal(startTime)>=maxTimeInMs)
        {
            success=false;
            break;
        }
    }
    for (int i=0;i<int(originalPath.size());i++)
        delete originalPath[i];

    return(success);
}
//*/


float* CmpObject::findIkPath(const float* startConfig,const float* goalPos,const float* goalQuat,int options,float stepSize,int* outputConfigsCnt,const int* auxIntParams,const float* auxFloatParams)
{
    // if ((options&8)!=0) we output some activity to the console
    // if ((options&16)!=0) we don't check robot self-collisions
    // if ((options&32)!=0) we don't check robot-environment collisions
    // if ((options&128)!=0) we return the "projected" distance (distance according to the IK constraint settings)
    // if ((options&256)!=0) we return the distance without orientation component

    bool activityToConsole=((options&8)!=0);
    bool returnProjectedDistance=((options&128)!=0);
    bool returnedDistanceHasNoOrientationComponent=((options&256)!=0);

    int collisionCheckMask=1+2; // 1=self-collision, 2=robot-environment
    if ((options&16)!=0)
        collisionCheckMask-=1;
    if ((options&32)!=0)
        collisionCheckMask-=2;


    float* retVal=nullptr;

    outputConfigsCnt[0]=0;

    _clearAllPhase2Nodes();

    C7Vector tipStartTransf;

    C7Vector goalPose;
    goalPose.X.setInternalData(goalPos);
    goalPose.Q.setInternalData(goalQuat);

    if (_applyJointPosToRobotAndCheckForCollisions_phase2(startConfig,tipStartTransf,collisionCheckMask))
    {
        if (activityToConsole)
            printf("Start configuration is colliding.\n");
        return(nullptr);
    }

    // We add a first node:
    CmpPhase2Node* startN=new CmpPhase2Node((int)_cyclicJointFlags.size(),startConfig,tipStartTransf);
    _nodeListFromStart.push_back(startN);


    C7Vector startPose;
    _simGetObjectCumulativeTransformation_internal(_tipDummy,startPose.X.data,startPose.Q.data,true);

    C7Vector goalPoseRel(_baseFrameTransformationInverse*goalPose);
    C7Vector startPoseRel(_baseFrameTransformationInverse*startPose);

    float virtualDist=_getWorkSpaceDistance(startPoseRel,goalPoseRel,false);
    float virtualDist2=virtualDist;
    if (returnedDistanceHasNoOrientationComponent)
        virtualDist2=_getWorkSpaceDistance(startPoseRel,goalPoseRel,true);
//  printf("virtualDist=%f\n",virtualDist);

    std::vector<float> projectedDistances;
    projectedDistances.push_back(0.0f);

    if (virtualDist>0.00001f)
    { // ok, the start and goal configs are far enough

        bool success;
        for (int direction=0;direction<2;direction++)
        { // We can do the interpolation in 2 different directions (for the orientation part: the short, or the long path)
            success=true;
            // the step count needed from start to goal:
            int stepCount=int(virtualDist/stepSize)+1;

            // calculate the steps:
            C3Vector dist(goalPoseRel.X-startPoseRel.X);
            C7Vector interpol;
            C3Vector dx(dist/float(stepCount));
            C3Vector v;
            v.clear();
            float dt=1.0f/float(stepCount);
            float t=0.0f;
            C7Vector targetParentTr;
            targetParentTr.setIdentity();
            CDummy3DObject* targetParent=(CDummy3DObject*)_simGetParentObject_internal(_targetDummy);
            if (targetParent!=nullptr)
                _simGetObjectCumulativeTransformation_internal(targetParent,targetParentTr.X.data,targetParentTr.Q.data,0);
            C7Vector targetParentTrInv(targetParentTr.getInverse());
            for (int i=0;i<stepCount;i++)
            { // do the steps:
                v+=dx;
                t+=dt;
                projectedDistances.push_back(virtualDist2*t);
                interpol.X=startPoseRel.X+v;
                if (direction==0)
                    interpol.Q.buildInterpolation(startPoseRel.Q,goalPoseRel.Q,t);
                else
                    interpol.Q.buildInterpolation_otherWayRound(startPoseRel.Q,goalPoseRel.Q,t);

                C7Vector targetNewLocal(targetParentTrInv*_baseFrameTransformation*interpol);

                std::vector<float> currentJointPos;
                for (int j=0;j<int(_jointHandles.size());j++)
                    currentJointPos.push_back(_simGetJointPosition_internal(_jointObjects[j]));
                std::vector<float> nextJointPos(_jointHandles.size(),0.0f);
                if (_performIkThenCheckForCollision__(&currentJointPos[0],&nextJointPos[0],targetNewLocal,collisionCheckMask))
                { // ok, that worked!
                    CmpPhase2Node* aNode=new CmpPhase2Node((int)_cyclicJointFlags.size(),&nextJointPos[0],interpol);
                    aNode->parentNode=_nodeListFromStart[_nodeListFromStart.size()-1];
                    _nodeListFromStart.push_back(aNode);
                }
                else
                { // we failed
                    if (activityToConsole)
                        printf("IK failed, or resulted in a colliding configuration.\n");
                    success=false;
                }
                if (!success)
                    break;
            }
            if (success)
                break;
            for (int i=1;i<int(_nodeListFromStart.size());i++)
                delete _nodeListFromStart[i];
            _nodeListFromStart.resize(1);
            projectedDistances.resize(1);
        }
        if (success)
        { // simply dedouble the path data to _nodeListFoundPath:
            for (int i=0;i<int(_nodeListFromStart.size());i++)
                _nodeListFoundPath.push_back(_nodeListFromStart[i]->copyYourself((int)_jointHandles.size()));
        }
    }
    else
    { // the start and goal configs are too close, maybe even coincident.
        // We just add a single configuration point to notify this
        if (activityToConsole)
            printf("Start and goal are coincident. Returning a single configuration.\n");
        _nodeListFoundPath.push_back(_nodeListFromStart[0]->copyYourself((int)_jointHandles.size()));
    }

    if (_nodeListFoundPath.size()!=0)
    {
        // prepare the return buffer (joint configs + length values + tip pos + tip orient + tip length values):
        int dofs=(int)_jointHandles.size();
        int configs=(int)_nodeListFoundPath.size();

        outputConfigsCnt[0]=configs;

        float l=0.0f;
        float l2=0.0f;

        retVal=(float*)new char[(configs*dofs + configs + configs*3 + configs*4 + configs)*4];
        for (int i=0;i<configs;i++)
        {
            for (int j=0;j<dofs;j++)
                retVal[i*dofs+j]=_nodeListFoundPath[i]->jointPositions[j];

            if (i!=0)
                l+=sqrtf(_getConfigErrorSquared(_nodeListFoundPath[i-1]->jointPositions,_nodeListFoundPath[i+0]->jointPositions,&_robotMetric[0]));
            retVal[configs*dofs+i]=l;

            C7Vector abs(_baseFrameTransformation*_nodeListFoundPath[i]->tipTransf);
            retVal[configs*dofs+configs+3*i+0]=abs.X(0);
            retVal[configs*dofs+configs+3*i+1]=abs.X(1);
            retVal[configs*dofs+configs+3*i+2]=abs.X(2);

            // V-REP quaternion, internally: w x y z
            // V-REP quaternion, at interfaces: x y z w
            retVal[configs*dofs+configs+3*configs+4*i+0]=abs.Q(1);
            retVal[configs*dofs+configs+3*configs+4*i+1]=abs.Q(2);
            retVal[configs*dofs+configs+3*configs+4*i+2]=abs.Q(3);
            retVal[configs*dofs+configs+3*configs+4*i+3]=abs.Q(0);

            if (returnProjectedDistance)
                retVal[configs*dofs+configs+3*configs+4*configs+i]=projectedDistances[i];
            else
            {
                if (i!=0)
                {
                    C3Vector w(_nodeListFoundPath[i-1]->tipTransf.X-_nodeListFoundPath[i-0]->tipTransf.X);
                    float a=0.0f;
                    if (!returnedDistanceHasNoOrientationComponent)
                        a=_nodeListFoundPath[i-1]->tipTransf.Q.getAngleBetweenQuaternions(_nodeListFoundPath[i-0]->tipTransf.Q);
                    float dx[4]={w(0)*_tipMetric[0],w(1)*_tipMetric[1],w(2)*_tipMetric[2],a*_tipMetric[3]};
                    l2+=sqrtf(dx[0]*dx[0]+dx[1]*dx[1]+dx[2]*dx[2]+dx[3]*dx[3]);
                }
                retVal[configs*dofs+configs+3*configs+4*configs+i]=l2;
            }
        }
    }

    return(retVal);
}

float CmpObject::_getWorkSpaceDistance(const C7Vector& tr1_relToBase,C7Vector tr2_relToBase,bool ignoreOrientationComponents)
{
    // Take into account the IK constraint settings:
    if ((_ikElementConstraints&sim_ik_x_constraint)==0)
        tr2_relToBase.X(0)=tr1_relToBase.X(0);
    if ((_ikElementConstraints&sim_ik_y_constraint)==0)
        tr2_relToBase.X(1)=tr1_relToBase.X(1);
    if ((_ikElementConstraints&sim_ik_z_constraint)==0)
        tr2_relToBase.X(2)=tr1_relToBase.X(2);
    C3Vector startEulerRel(tr1_relToBase.Q.getEulerAngles());
    C3Vector goalEulerRel(tr2_relToBase.Q.getEulerAngles());
    bool changedSomethingInOrientation=false;
    if (((_ikElementConstraints&sim_ik_gamma_constraint)==0)||ignoreOrientationComponents)
    {
        goalEulerRel(2)=startEulerRel(2);
        changedSomethingInOrientation=true;
    }
    if (((_ikElementConstraints&sim_ik_alpha_beta_constraint)==0)||ignoreOrientationComponents)
    {
        goalEulerRel(0)=startEulerRel(0);
        goalEulerRel(1)=startEulerRel(1);
        changedSomethingInOrientation=true;
    }
    if (changedSomethingInOrientation)
        tr2_relToBase.Q=C4Vector(goalEulerRel(0),goalEulerRel(1),goalEulerRel(2));

    // Calculate the virtual distance between startPoseRel and goalPoseRel:
    C3Vector dist(tr2_relToBase.X-tr1_relToBase.X);
    float angle=tr2_relToBase.Q.getAngleBetweenQuaternions(tr1_relToBase.Q);
    float dc[4]={dist(0)*_tipMetric[0],dist(1)*_tipMetric[1],dist(2)*_tipMetric[2],angle*_tipMetric[3]};
    float virtualDist=sqrt(dc[0]*dc[0]+dc[1]*dc[1]+dc[2]*dc[2]+dc[3]*dc[3]);
    return(virtualDist);
}

float* CmpObject::simplifyPath(const float* pathBuffer,int configCnt,int options,float stepSize,int increment,int* outputConfigsCnt,int maxTimeInMs,const int* auxIntParams,const float* auxFloatParams)
{
    // if ((options&8)!=0) we output some activity to the console
    // if ((options&16)!=0) we don't check robot self-collisions
    // if ((options&32)!=0) we don't check robot-environment collisions
    // if ((options&256)!=0) we return the distance without orientation component

    bool activityToConsole=((options&8)!=0);
    bool returnedDistanceHasNoOrientationComponent=((options&256)!=0);

    int collisionCheckMask=1+2; // 1=self-collision, 2=robot-environment
    if ((options&16)!=0)
        collisionCheckMask-=1;
    if ((options&32)!=0)
        collisionCheckMask-=2;


    float* retVal=nullptr;
    outputConfigsCnt[0]=0;
    _clearAllPhase2Nodes();

    int maxTime=999999;
    if (maxTimeInMs!=0)
        maxTime=maxTimeInMs;
    std::vector<float> config(_jointHandles.size(),0.0f);
    C7Vector tr;
    for (int i=0;i<configCnt;i++)
    {
        for (int j=0;j<int(_jointHandles.size());j++)
            config[j]=pathBuffer[i*_jointHandles.size()+j];
        _applyJointPosToRobotAndCheckForCollisions_phase2(&config[0],tr,0);
        CmpPhase2Node* node=new CmpPhase2Node((int)_jointHandles.size(),&config[0],tr);
        _nodeListFoundPath.push_back(node);
    }

    bool success=_simplifyFoundPath(auxIntParams,auxFloatParams,increment,stepSize,activityToConsole,collisionCheckMask,maxTime);

    if (success)
    {
        // prepare the return buffer (joint configs + length values + tip pos + tip orient + tip length values):
        int dofs=(int)_jointHandles.size();
        int configs=(int)_nodeListFoundPath.size();

        outputConfigsCnt[0]=configs;

        float l=0.0f;
        float l2=0.0f;

        retVal=(float*)new char[(configs*dofs + configs + configs*3 + configs*4 + configs)*4];
        for (int i=0;i<configs;i++)
        {
            for (int j=0;j<dofs;j++)
                retVal[i*dofs+j]=_nodeListFoundPath[i]->jointPositions[j];

            if (i!=0)
                l+=sqrtf(_getConfigErrorSquared(_nodeListFoundPath[i-1]->jointPositions,_nodeListFoundPath[i+0]->jointPositions,&_robotMetric[0]));
            retVal[configs*dofs+i]=l;

            C7Vector abs(_baseFrameTransformation*_nodeListFoundPath[i]->tipTransf);
            retVal[configs*dofs+configs+3*i+0]=abs.X(0);
            retVal[configs*dofs+configs+3*i+1]=abs.X(1);
            retVal[configs*dofs+configs+3*i+2]=abs.X(2);

            // V-REP quaternion, internally: w x y z
            // V-REP quaternion, at interfaces: x y z w
            retVal[configs*dofs+configs+3*configs+4*i+0]=abs.Q(1);
            retVal[configs*dofs+configs+3*configs+4*i+1]=abs.Q(2);
            retVal[configs*dofs+configs+3*configs+4*i+2]=abs.Q(3);
            retVal[configs*dofs+configs+3*configs+4*i+3]=abs.Q(0);

            if (i!=0)
            {
                C3Vector w(_nodeListFoundPath[i-1]->tipTransf.X-_nodeListFoundPath[i-0]->tipTransf.X);
                float a=0.0f;
                if (!returnedDistanceHasNoOrientationComponent)
                    a=_nodeListFoundPath[i-1]->tipTransf.Q.getAngleBetweenQuaternions(_nodeListFoundPath[i-0]->tipTransf.Q);
                float dx[4]={w(0)*_tipMetric[0],w(1)*_tipMetric[1],w(2)*_tipMetric[2],a*_tipMetric[3]};
                l2+=sqrtf(dx[0]*dx[0]+dx[1]*dx[1]+dx[2]*dx[2]+dx[3]*dx[3]);
            }
            retVal[configs*dofs+configs+3*configs+4*configs+i]=l2;
        }
    }

    return(retVal);
}

float* CmpObject::getConfigTransition(const float* startConfig,const float* goalConfig,int options,const int* select,float calcStepSize,float maxOutStepSize,int wayPointCnt,const float* wayPoints,int* outputConfigsCnt,const int* auxIntParams,const float* auxFloatParams)
{
    // if ((options&8)!=0) we output some activity to the console
    // if ((options&16)!=0) we don't check robot self-collisions
    // if ((options&32)!=0) we don't check robot-environment collisions
    // if ((options&256)!=0) we return the distance without orientation component
    // if ((options&512)!=0) we use IK to keep on a linear trajectory in the Cartesian space

    bool activityToConsole=((options&8)!=0);
    bool returnedDistanceHasNoOrientationComponent=((options&256)!=0);
    bool useIk=((options&512)!=0);

    int collisionCheckMask=1+2; // 1=self-collision, 2=robot-environment
    if ((options&16)!=0)
        collisionCheckMask-=1;
    if ((options&32)!=0)
        collisionCheckMask-=2;

    float* retVal=nullptr;

    outputConfigsCnt[0]=0;
    _clearAllPhase2Nodes();

    C7Vector tipStartTransf,tipGoalTransf;
    if (_applyJointPosToRobotAndCheckForCollisions_phase2(startConfig,tipStartTransf,collisionCheckMask)||_applyJointPosToRobotAndCheckForCollisions_phase2(goalConfig,tipGoalTransf,collisionCheckMask))
    {
        if (activityToConsole)
            printf("Start or goal configuration is colliding.\n");
        return(nullptr);
    }

    std::vector<C7Vector> waypts;
    std::vector<float> wayptsdist;

    float wayPointsTotalDistance=0.0f;
    if ((wayPointCnt<3)||(wayPoints==nullptr))
    { // now way points (i.e. just a straight line)
        if (activityToConsole)
            printf("Not using any way points.\n");
        waypts.push_back(tipStartTransf);
        wayptsdist.push_back(0.0f);
        waypts.push_back(tipGoalTransf);
        wayptsdist.push_back(1.0f);
    }
    else
    { // we have way points
        if (activityToConsole)
            printf("Using %i way points.\n",wayPointCnt);
        waypts.push_back(tipStartTransf);
        for (int i=1;i<(wayPointCnt-1);i++)
        {
            C7Vector w(C4Vector(wayPoints[7*i+6],wayPoints[7*i+3],wayPoints[7*i+4],wayPoints[7*i+5]),C3Vector(wayPoints+7*i+0));
            waypts.push_back(_baseFrameTransformationInverse*w);
        }
        waypts.push_back(tipGoalTransf);
        for (int i=0;i<int(waypts.size()-1);i++)
        {
            C7Vector w0(waypts[i]);
            C7Vector w1(waypts[i+1]);
            float d=_getWorkSpaceDistance(w0,w1,false);
            wayPointsTotalDistance+=d;
        }
        wayptsdist.push_back(0.0f);
        float currentD=0.0f;
        for (int i=0;i<int(waypts.size()-2);i++)
        {
            C7Vector w0(waypts[i]);
            C7Vector w1(waypts[i+1]);
            float d=_getWorkSpaceDistance(w0,w1,false);
            currentD+=d;
            wayptsdist.push_back(currentD/wayPointsTotalDistance);
        }
        wayptsdist.push_back(1.0f);

    }


    CmpPhase2Node* startN=new CmpPhase2Node((int)_cyclicJointFlags.size(),startConfig,tipStartTransf);
    _nodeListFromStart.push_back(startN);

    std::vector<float> constr(_jointHandles.size(),1.0f);
    if ((_getConfigErrorSquared(startConfig,goalConfig,&constr[0])>0.00001f)||(wayPointsTotalDistance>0.00001f))
    { // ok, the start and goal configs are far enough
        std::vector<float> dx(_jointHandles.size(),0.0f);
        int passes=_getPhase2Vector(startConfig,goalConfig,calcStepSize,&dx[0]);

        for (int rotDir=0;rotDir<2;rotDir++)
        { // this might only be executed twice with IK. Without ik, just once
            std::vector<float> currentJointPos;
            std::vector<float> previousJointPos;
            std::vector<float> nextJointPos(_jointHandles.size(),0.0f);
            currentJointPos.assign(startConfig,startConfig+_jointHandles.size());
            previousJointPos.assign(startConfig,startConfig+_jointHandles.size());
            float dt=1.0f/float(passes);
            float t=0.0f;

            C7Vector targetParentTr;
            targetParentTr.setIdentity();
            CDummy3DObject* targetParent=(CDummy3DObject*)_simGetParentObject_internal(_targetDummy);
            if (targetParent!=nullptr)
                _simGetObjectCumulativeTransformation_internal(targetParent,targetParentTr.X.data,targetParentTr.Q.data,0);
            C7Vector targetParentTrInv(targetParentTr.getInverse());

            std::vector<float> constr(_jointHandles.size(),1.0f);

            bool success=true;
            for (int currentPass=0;currentPass<passes;currentPass++)
            {
                // 1. Set the joints into the interpolated state:
                for (int i=0;i<int(dx.size());i++)
                    currentJointPos[i]+=dx[i];

                // 2. Check for collisions (and use IK if so desired):
                t+=dt;
                C7Vector newRelTipPose;
                if (useIk)
                {
                    C7Vector interpol(_getInterpolatedTransformationOnCurve(waypts,wayptsdist,t));
//                  interpol.buildInterpolation(tipStartTransf,tipGoalTransf,t);
                    if (rotDir==1)
                    { // we want to rotate the other way round. THis only happens when we don't have any way points:
                        interpol.Q.buildInterpolation_otherWayRound(tipStartTransf.Q,tipGoalTransf.Q,t);
                    }

                    C7Vector targetNewLocal(targetParentTrInv*_baseFrameTransformation*interpol);
                    if (_performIkThenCheckForCollision__(&currentJointPos[0],&nextJointPos[0],targetNewLocal,collisionCheckMask))
                    { // ok, that worked!
                        newRelTipPose=interpol;
                    }
                    else
                    { // we failed
                        if (activityToConsole)
                            printf("IK failed, or resulted in a colliding configuration.\n");
                        success=false;
                    }
                }
                else
                { // do not use IK. But test for collisions:
                    nextJointPos.assign(currentJointPos.begin(),currentJointPos.end());
                    if (_applyJointPosToRobotAndCheckForCollisions_phase2(&nextJointPos[0],newRelTipPose,collisionCheckMask))
                    { // we failed
                        if (activityToConsole)
                            printf("Interpolated configuration is colliding.\n");
                        success=false;
                    }
                }

                // 3. Add the node if the movement wasn't too large:
                if (success)
                {
                    if (_getConfigErrorSquared(&previousJointPos[0],&nextJointPos[0],&constr[0])<(maxOutStepSize*maxOutStepSize))
                    { // ok, the steps are not too large
                        CmpPhase2Node* aNode=new CmpPhase2Node((int)_cyclicJointFlags.size(),&nextJointPos[0],newRelTipPose);
                        aNode->parentNode=_nodeListFromStart[_nodeListFromStart.size()-1];
                        _nodeListFromStart.push_back(aNode);
                    }
                    else
                    {
                        if (activityToConsole)
                            printf("'maxOutStepSize' overshot. Select a smaller 'calcStepSize', or larger 'maxOutStepSize'.\n");
                        success=false;
                    }
                }
                previousJointPos.assign(nextJointPos.begin(),nextJointPos.end());
                if (!success)
                    break;
            }

            if (success)
            { // simply dedouble the path data to _nodeListFoundPath:
                for (int i=0;i<int(_nodeListFromStart.size());i++)
                    _nodeListFoundPath.push_back(_nodeListFromStart[i]->copyYourself((int)_jointHandles.size()));
                break;
            }

            if ((!useIk)||(waypts.size()<=2))
                break; // with IK (and no way points), we will try one time more, but the other way round rotation interpol
        }
    }
    else
    { // the start and goal configs are too close, maybe even coincident.
        // We just add a single configuration point to notify this
        if (activityToConsole)
            printf("Start and goal are coincident. Returning a single configuration.\n");
        _nodeListFoundPath.push_back(_nodeListFromStart[0]->copyYourself((int)_jointHandles.size()));
    }


    if (_nodeListFoundPath.size()!=0)
    {
        // prepare the return buffer (joint configs + length values + tip pos + tip orient + tip length values):
        int dofs=(int)_jointHandles.size();
        int configs=(int)_nodeListFoundPath.size();

        outputConfigsCnt[0]=configs;

        float l=0.0f;
        float l2=0.0f;

        retVal=(float*)new char[(configs*dofs + configs + configs*3 + configs*4 + configs)*4];
        for (int i=0;i<configs;i++)
        {
            for (int j=0;j<dofs;j++)
                retVal[i*dofs+j]=_nodeListFoundPath[i]->jointPositions[j];

            if (i!=0)
                l+=sqrtf(_getConfigErrorSquared(_nodeListFoundPath[i-1]->jointPositions,_nodeListFoundPath[i+0]->jointPositions,&_robotMetric[0]));
            retVal[configs*dofs+i]=l;

            C7Vector abs(_baseFrameTransformation*_nodeListFoundPath[i]->tipTransf);
            retVal[configs*dofs+configs+3*i+0]=abs.X(0);
            retVal[configs*dofs+configs+3*i+1]=abs.X(1);
            retVal[configs*dofs+configs+3*i+2]=abs.X(2);

            // V-REP quaternion, internally: w x y z
            // V-REP quaternion, at interfaces: x y z w
            retVal[configs*dofs+configs+3*configs+4*i+0]=abs.Q(1);
            retVal[configs*dofs+configs+3*configs+4*i+1]=abs.Q(2);
            retVal[configs*dofs+configs+3*configs+4*i+2]=abs.Q(3);
            retVal[configs*dofs+configs+3*configs+4*i+3]=abs.Q(0);

            if (i!=0)
            {
                C3Vector w(_nodeListFoundPath[i-1]->tipTransf.X-_nodeListFoundPath[i-0]->tipTransf.X);
                float a=0.0f;
                if (!returnedDistanceHasNoOrientationComponent)
                    a=_nodeListFoundPath[i-1]->tipTransf.Q.getAngleBetweenQuaternions(_nodeListFoundPath[i-0]->tipTransf.Q);
                float dx[4]={w(0)*_tipMetric[0],w(1)*_tipMetric[1],w(2)*_tipMetric[2],a*_tipMetric[3]};
                l2+=sqrtf(dx[0]*dx[0]+dx[1]*dx[1]+dx[2]*dx[2]+dx[3]*dx[3]);
            }
            retVal[configs*dofs+configs+3*configs+4*configs+i]=l2;
        }
    }

    return(retVal);
}

C7Vector CmpObject::_getInterpolatedTransformationOnCurve(const std::vector<C7Vector>& wayPts,const std::vector<float>& wayPtsRelDists,float t)
{ // t should vary between 0 and 1!
    if (t<=0.0f)
        return(wayPts[0]);
    if (t>=1.0f)
        return(wayPts[wayPts.size()-1]);
    for (int i=0;i<int(wayPts.size()-1);i++)
    {
        if (t<=wayPtsRelDists[i+1])
        {
            float t0=wayPtsRelDists[i];
            C7Vector w0(wayPts[i]);
            float t1=wayPtsRelDists[i+1];
            C7Vector w1(wayPts[i+1]);
            float s=t-t0;
            float sw=t1-t0;
            C7Vector interpol;
            interpol.buildInterpolation(w0,w1,s/sw);
            return(interpol);
        }
    }
    return(C7Vector::identityTransformation); // should never happen!
}
