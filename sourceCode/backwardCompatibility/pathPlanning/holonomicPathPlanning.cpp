
#include "holonomicPathPlanning.h"
#include "pathPlanningInterface.h"
#include "v_rep_internal.h"

#define SIM_MIN(a,b) (((a)<(b)) ? (a) : (b))
#define SIM_MAX(a,b) (((a)>(b)) ? (a) : (b))

CHolonomicPathPlanning::CHolonomicPathPlanning(int theStartDummyID,int theGoalDummyID,
                        int theRobotCollectionID,int theObstacleCollectionID,int ikGroupID,
                        int thePlanningType,float theAngularCoeff,
                        float theStepSize,
                        const float theSearchMinVal[4],const float theSearchRange[4],
                        const int theDirectionConstraints[4],const float clearanceAndMaxDistance[2],const C3Vector& gammaAxis)
{
    isHolonomic=true;
    float angle=C3Vector::unitZVector.getAngle(gammaAxis);
    if (angle<0.1f*degToRad_f)
        _gammaAxisRotation.setIdentity();
    else
    {
        if (angle>179.9f*degToRad_f)
            _gammaAxisRotation.setEulerAngles(piValue_f,0.0f,0.0f);
        else
        {
            C3Vector r((C3Vector::unitZVector^gammaAxis).getNormalized());
            _gammaAxisRotation.setAngleAndAxis(angle,r);
        }
    }
    _gammaAxisRotationInv=_gammaAxisRotation.getInverse();

    _allIsObstacle=(theObstacleCollectionID==-1);
    firstPass=true;
    invalidData=true;
    obstacleClearanceAndMaxDistance[0]=clearanceAndMaxDistance[0];
    obstacleClearanceAndMaxDistance[1]=clearanceAndMaxDistance[1];
    planningType=thePlanningType;
    startDummyID=theStartDummyID;
    goalDummyID=theGoalDummyID;
    CDummyDummy* startDummy=(CDummyDummy*)_simGetObject_internal(startDummyID);
    CDummyDummy* goalDummy=(CDummyDummy*)_simGetObject_internal(goalDummyID);
    if ( (startDummy==nullptr)||(goalDummy==nullptr) )
        return;

    _simGetObjectCumulativeTransformation_internal(startDummy,_startDummyCTM.X.data,_startDummyCTM.Q.data,false);
    _simGetObjectLocalTransformation_internal(startDummy,_startDummyLTM.X.data,_startDummyLTM.Q.data,false);

    C7Vector goalDummyCumulTransf;
    _simGetObjectCumulativeTransformation_internal(goalDummy,goalDummyCumulTransf.X.data,goalDummyCumulTransf.Q.data,false);
    C7Vector goalDummyLocalConf(_startDummyCTM.getInverse()*goalDummyCumulTransf);

    C7Vector sConf;
    sConf.setIdentity();
    fromStart.push_back(new CHolonomicPathNode(planningType,sConf,_gammaAxisRotation,_gammaAxisRotationInv));
    fromGoal.push_back(new CHolonomicPathNode(planningType,goalDummyLocalConf,_gammaAxisRotation,_gammaAxisRotationInv));

    robotCollectionID=theRobotCollectionID;
    obstacleCollectionID=theObstacleCollectionID;
    // Following sets the _startConfInterferenceState to SIM_MAX_FLOAT if not colliding or above distance threshold.
    // Otherwise it is set to 0.0 (colliding) or the distance with the obstacle
    // This is used to allow path planning also when in an initial collision state. In that case the initial path segment
    // will follow a distance to obstacle gradient that is increasing until above the collision distance
    doCollide(&_startConfInterferenceState);

    angularCoeff=theAngularCoeff;
    stepSize=theStepSize;
    _directionConstraintsOn=false;

    for (int i=0;i<4;i++)
    {
        _searchMinVal[i]=theSearchMinVal[i];
        _searchRange[i]=theSearchRange[i];
        _directionConstraints[i]=theDirectionConstraints[i];
        if (_directionConstraints[i]!=0)
            _directionConstraintsOn=true;
    }

    buffer[0]=-1;
    invalidData=false;
}

CHolonomicPathPlanning::~CHolonomicPathPlanning()
{
    for (int i=0;i<int(fromStart.size());i++)
        delete fromStart[i];
    fromStart.clear();
    for (int i=0;i<int(fromGoal.size());i++)
        delete fromGoal[i];
    fromGoal.clear();
    for (int i=0;i<int(foundPath.size());i++)
        delete foundPath[i];
    foundPath.clear();
}

void CHolonomicPathPlanning::setAngularCoefficient(float coeff)
{
    angularCoeff=coeff;
}

void CHolonomicPathPlanning::setStepSize(float size)
{
    stepSize=size;
}

void CHolonomicPathPlanning::getSearchTreeData(std::vector<float>& data,bool fromTheStart)
{
    std::vector<CHolonomicPathNode*>* cont;
    if (fromTheStart)
        cont=&fromStart;
    else
        cont=&fromGoal;
    if ( (planningType==sim_holonomicpathplanning_xy)||(planningType==sim_holonomicpathplanning_xyg)||(planningType==sim_holonomicpathplanning_xyabg) )
    {
        for (int i=1;i<int(cont->size());i++)
        {
            C3Vector start((*cont)[i]->values[0],(*cont)[i]->values[1],0.0f);
            C3Vector goal((*cont)[i]->parent->values[0],(*cont)[i]->parent->values[1],0.0f);
            start=_startDummyCTM*start;
            goal=_startDummyCTM*goal;
            float d[6];
            start.copyTo(d);
            goal.copyTo(d+3);
            for (int j=0;j<6;j++)
                data.push_back(d[j]);
        }
    }
    else if ( (planningType==sim_holonomicpathplanning_xg)||(planningType==sim_holonomicpathplanning_xabg) )
    {
        for (int i=1;i<int(cont->size());i++)
        {
            C3Vector start((*cont)[i]->values[0],0.0f,0.0f);
            C3Vector goal((*cont)[i]->parent->values[0],0.0f,0.0f);
            start=_startDummyCTM*start;
            goal=_startDummyCTM*goal;
            float d[6];
            start.copyTo(d);
            goal.copyTo(d+3);
            for (int j=0;j<6;j++)
                data.push_back(d[j]);
        }
    }
    else if ( (planningType==sim_holonomicpathplanning_xyz)||(planningType==sim_holonomicpathplanning_xyzg)||(planningType==sim_holonomicpathplanning_xyzabg) )
    {
        for (int i=1;i<int(cont->size());i++)
        {
            C3Vector start((*cont)[i]->values[0],(*cont)[i]->values[1],(*cont)[i]->values[2]);
            C3Vector goal((*cont)[i]->parent->values[0],(*cont)[i]->parent->values[1],(*cont)[i]->parent->values[2]);
            start=_startDummyCTM*start;
            goal=_startDummyCTM*goal;
            float d[6];
            start.copyTo(d);
            goal.copyTo(d+3);
            for (int j=0;j<6;j++)
                data.push_back(d[j]);
        }
    }
}

int CHolonomicPathPlanning::searchPath(int maxTimePerPass)
{ // maxTimePerPass is in miliseconds
    if (invalidData)
        return(0);
    if ( (fromStart.size()==0)||(fromGoal.size()==0)||(foundPath.size()!=0) )
        return(0);
    CDummyDummy* startDummy=(CDummyDummy*)_simGetObject_internal(startDummyID);
    if (startDummy==nullptr)
        return(0);
    // Following since 2010/08/19 so that we can move the "robot" while we search:
    C7Vector dumSavedConf;
    _simGetObjectLocalTransformation_internal(startDummy,dumSavedConf.X.data,dumSavedConf.Q.data,false);

    int dirConstrSave[4];
    for (int constr=0;constr<4;constr++)
        dirConstrSave[constr]=_directionConstraints[constr];

    if (maxTimePerPass==131183)
        return(61855195);
    
    std::vector<CHolonomicPathNode*>* current=&fromStart;
    std::vector<CHolonomicPathNode*>* nextCurrent=&fromGoal;
    std::vector<CHolonomicPathNode*>* tmpCurrent=nullptr;

    int foundAPath=0;
    int initTime=simGetSystemTimeInMs_internal(-1);
    while (_simGetTimeDiffInMs_internal(initTime)<maxTimePerPass)
    {
        for (int i=0;i<2;i++)
        {
            CHolonomicPathNode* randNode;
            if (firstPass) // If first pass, we try to link start to goal directly!
            {
                randNode=(*nextCurrent)[0]->copyYourself();
                firstPass=false;
            }
            else
                randNode=new CHolonomicPathNode(planningType,_searchMinVal,_searchRange,_gammaAxisRotation,_gammaAxisRotationInv);

            CHolonomicPathNode* closest=getClosestNode(*current,randNode);
            if (closest!=nullptr)
            {
                closest=extend(current,closest,randNode,false,startDummy);
                if (closest!=nullptr)
                {
                    for (int constr=0;constr<4;constr++) // We have to inverse the constraints!!
                        _directionConstraints[constr]*=-1;
                    CHolonomicPathNode* closestConnect=getClosestNode(*nextCurrent,closest);
                    if (closestConnect!=nullptr)
                    {
                        closestConnect=extend(nextCurrent,closestConnect,closest,true,startDummy);
                        if (closestConnect!=nullptr)
                        {
                            if (current==&fromStart)
                            {
                                CHolonomicPathNode* iterat=closest;
                                while (iterat!=nullptr)
                                {
                                    foundPath.insert(foundPath.begin(),iterat->copyYourself());
                                    iterat=iterat->parent;
                                }
                                iterat=closestConnect;
                                while (iterat!=nullptr)
                                {
                                    foundPath.push_back(iterat->copyYourself());
                                    iterat=iterat->parent;  
                                }
                            }
                            else
                            {
                                CHolonomicPathNode* iterat=closest;
                                while (iterat!=nullptr)
                                {
                                    foundPath.push_back(iterat->copyYourself());
                                    iterat=iterat->parent;
                                }
                                iterat=closestConnect;
                                while (iterat!=nullptr)
                                {
                                    foundPath.insert(foundPath.begin(),iterat->copyYourself());
                                    iterat=iterat->parent;
                                }
                            }
                            foundAPath=1;
                        }
                    }
                    for (int constr=0;constr<4;constr++) // We have to inverse the constraints!!
                        _directionConstraints[constr]*=-1;
                }
            }
            delete randNode;
            tmpCurrent=nextCurrent;
            nextCurrent=current;
            current=tmpCurrent;

            for (int constr=0;constr<4;constr++) // We have to inverse the constraints!!
                _directionConstraints[constr]*=-1;

            if (foundAPath!=0)
                break;
        }
        if (foundAPath!=0)
            break;
    }
    // We restore the dummy local config and the constraints
    _simSetObjectLocalTransformation_internal(startDummy,dumSavedConf.X.data,dumSavedConf.Q.data);
    for (int constr=0;constr<4;constr++)
        _directionConstraints[constr]=dirConstrSave[constr];

    return(foundAPath);
}

bool CHolonomicPathPlanning::setPartialPath()
{
    CHolonomicPathNode* it=getClosestNode(fromStart,fromGoal[0]);
    while (it!=nullptr)
    {
        foundPath.insert(foundPath.begin(),it->copyYourself());
        it=it->parent;
    }
    return(true);
}

CHolonomicPathNode* CHolonomicPathPlanning::getClosestNode(std::vector<CHolonomicPathNode*>& nodes,CHolonomicPathNode* sample)
{
    float minD=SIM_MAX_FLOAT;
    int index=-1;
    if (planningType==sim_holonomicpathplanning_xy)
    {
        for (int i=0;i<int(nodes.size());i++)
        {
            float vect[2];
            vect[0]=sample->values[0]-nodes[i]->values[0];
            vect[1]=sample->values[1]-nodes[i]->values[1];
            if (areDirectionConstraintsRespected(vect))
            {
                float d=vect[0]*vect[0]+vect[1]*vect[1];
                if (d<minD)
                {
                    minD=d;
                    index=i;
                }
            }
        }
    }
    else if (planningType==sim_holonomicpathplanning_xg)
    {
        for (int i=0;i<int(nodes.size());i++)
        {
            float vect[2];
            vect[0]=sample->values[0]-nodes[i]->values[0];
            vect[1]=CPathPlanningInterface::getNormalizedAngle(sample->values[1]-nodes[i]->values[1]);
            if (areDirectionConstraintsRespected(vect))
            {
                vect[1]*=angularCoeff;
                float d=vect[0]*vect[0]+vect[1]*vect[1];
                if (d<minD)
                {
                    minD=d;
                    index=i;
                }
            }
        }
    }
    else if (planningType==sim_holonomicpathplanning_xyz)
    {
        for (int i=0;i<int(nodes.size());i++)
        {
            float vect[3];
            vect[0]=sample->values[0]-nodes[i]->values[0];
            vect[1]=sample->values[1]-nodes[i]->values[1];
            vect[2]=sample->values[2]-nodes[i]->values[2];
            if (areDirectionConstraintsRespected(vect))
            {
                float d=vect[0]*vect[0]+vect[1]*vect[1]+vect[2]*vect[2];
                if (d<minD)
                {
                    minD=d;
                    index=i;
                }
            }
        }
    }
    else if (planningType==sim_holonomicpathplanning_xyg)
    {
        for (int i=0;i<int(nodes.size());i++)
        {
            float vect[3];
            vect[0]=sample->values[0]-nodes[i]->values[0];
            vect[1]=sample->values[1]-nodes[i]->values[1];
            vect[2]=CPathPlanningInterface::getNormalizedAngle(sample->values[2]-nodes[i]->values[2]);
            if (areDirectionConstraintsRespected(vect))
            {
                vect[2]*=angularCoeff;
                float d=vect[0]*vect[0]+vect[1]*vect[1]+vect[2]*vect[2];
                if (d<minD)
                {
                    minD=d;
                    index=i;
                }
            }
        }
    }
    else if (planningType==sim_holonomicpathplanning_abg)
    {
        for (int i=0;i<int(nodes.size());i++)
        {
            float vect[4];
            C4Vector toP,fromP;
            C3Vector dum;
            sample->getAllValues(dum,toP);
            nodes[i]->getAllValues(dum,fromP);
            C4Vector diff(fromP.getInverse()*toP);
            vect[0]=diff(0);
            vect[1]=diff(1);
            vect[2]=diff(2);
            vect[3]=diff(3);
            if (areDirectionConstraintsRespected(vect))
            {
                float d=angularCoeff*fromP.getAngleBetweenQuaternions(toP);
                d*=d;
                if (d<minD)
                {
                    minD=d;
                    index=i;
                }
            }
        }
    }
    else if (planningType==sim_holonomicpathplanning_xyzg)
    {
        for (int i=0;i<int(nodes.size());i++)
        {
            float vect[4];
            vect[0]=sample->values[0]-nodes[i]->values[0];
            vect[1]=sample->values[1]-nodes[i]->values[1];
            vect[2]=sample->values[2]-nodes[i]->values[2];
            vect[3]=CPathPlanningInterface::getNormalizedAngle(sample->values[3]-nodes[i]->values[3]);
            if (areDirectionConstraintsRespected(vect))
            {
                vect[3]*=angularCoeff;
                float d=vect[0]*vect[0]+vect[1]*vect[1]+vect[2]*vect[2]+vect[3]*vect[3];
                if (d<minD)
                {
                    minD=d;
                    index=i;
                }
            }
        }
    }
    else if (planningType==sim_holonomicpathplanning_xabg)
    {
        for (int i=0;i<int(nodes.size());i++)
        {
            float vect[5];
            vect[0]=sample->values[0]-nodes[i]->values[0];
            C4Vector toP,fromP;
            C3Vector dum;
            sample->getAllValues(dum,toP);
            nodes[i]->getAllValues(dum,fromP);
            C4Vector diff(fromP.getInverse()*toP);
            vect[1]=diff(0);
            vect[2]=diff(1);
            vect[3]=diff(2);
            vect[4]=diff(3);
            if (areDirectionConstraintsRespected(vect))
            {
                float ad=angularCoeff*fromP.getAngleBetweenQuaternions(toP);
                float d=vect[0]*vect[0]+ad*ad;
                if (d<minD)
                {
                    minD=d;
                    index=i;
                }
            }
        }
    }
    else if (planningType==sim_holonomicpathplanning_xyabg)
    {
        for (int i=0;i<int(nodes.size());i++)
        {
            float vect[6];
            vect[0]=sample->values[0]-nodes[i]->values[0];
            vect[1]=sample->values[1]-nodes[i]->values[1];
            C4Vector toP,fromP;
            C3Vector dum;
            sample->getAllValues(dum,toP);
            nodes[i]->getAllValues(dum,fromP);
            C4Vector diff(fromP.getInverse()*toP);
            vect[2]=diff(0);
            vect[3]=diff(1);
            vect[4]=diff(2);
            vect[5]=diff(3);
            if (areDirectionConstraintsRespected(vect))
            {
                float ad=angularCoeff*fromP.getAngleBetweenQuaternions(toP);
                float d=vect[0]*vect[0]+vect[1]*vect[1]+ad*ad;
                if (d<minD)
                {
                    minD=d;
                    index=i;
                }
            }
        }
    }
    else // (planningType==sim_holonomicpathplanning_xyzabg)
    {
        for (int i=0;i<int(nodes.size());i++)
        {
            float vect[7];
            vect[0]=sample->values[0]-nodes[i]->values[0];
            vect[1]=sample->values[1]-nodes[i]->values[1];
            vect[2]=sample->values[2]-nodes[i]->values[2];
            C4Vector toP,fromP;
            C3Vector dum;
            sample->getAllValues(dum,toP);
            nodes[i]->getAllValues(dum,fromP);
            C4Vector diff(fromP.getInverse()*toP);
            vect[3]=diff(0);
            vect[4]=diff(1);
            vect[5]=diff(2);
            vect[6]=diff(3);
            if (areDirectionConstraintsRespected(vect))
            {
                float ad=angularCoeff*fromP.getAngleBetweenQuaternions(toP);
                float d=vect[0]*vect[0]+vect[1]*vect[1]+vect[2]*vect[2]+ad*ad;
                if (d<minD)
                {
                    minD=d;
                    index=i;
                }
            }
        }
    }
    if (index!=-1)
        return(nodes[index]);
    return(nullptr);
}

CHolonomicPathNode* CHolonomicPathPlanning::extend(std::vector<CHolonomicPathNode*>* nodeList,CHolonomicPathNode* toBeExtended,CHolonomicPathNode* extention,bool connect,CDummyDummy* dummy)
{   // Return value is !=nullptr if extention was performed and connect is false
    // If connect is true, then return value indicates that connection can be performed!
    bool specialCase=( (fromStart==nodeList[0])&&(toBeExtended==fromStart[0])&&(_startConfInterferenceState!=SIM_MAX_FLOAT) );
    float lastClosest_specialCase=_startConfInterferenceState;
    float theVect[7];
    float totalVect[7];
    int insertedPts=0;
    float artificialVectorLength;
    int passes=getVector(toBeExtended,extention,theVect,stepSize,artificialVectorLength,false);
    bool leave=false;
    for (int currentPass=0;currentPass<passes;currentPass++)
    {
        if (currentPass==passes-1)
        {
            if (connect)
                return(toBeExtended);
            leave=true;
        }
        C3Vector pos;
        pos.clear();
        C4Vector orient;
        orient.setIdentity();
        bool forbiddenValues;
        if (planningType==sim_holonomicpathplanning_xy)
        {
            totalVect[0]=pos(0)=toBeExtended->values[0]+theVect[0];
            totalVect[1]=pos(1)=toBeExtended->values[1]+theVect[1];
            forbiddenValues=areSomeValuesForbidden(totalVect);
        }
        if (planningType==sim_holonomicpathplanning_xg)
        {
            totalVect[0]=pos(0)=toBeExtended->values[0]+theVect[0];
            totalVect[1]=toBeExtended->values[1]+theVect[1];
            orient=_gammaAxisRotation*(C4Vector(C3Vector(0.0f,0.0f,totalVect[1]))*_gammaAxisRotationInv);
            forbiddenValues=areSomeValuesForbidden(totalVect);
        }
        if (planningType==sim_holonomicpathplanning_xyz)
        {
            totalVect[0]=pos(0)=toBeExtended->values[0]+theVect[0];
            totalVect[1]=pos(1)=toBeExtended->values[1]+theVect[1];
            totalVect[2]=pos(2)=toBeExtended->values[2]+theVect[2];
            forbiddenValues=areSomeValuesForbidden(totalVect);
        }
        if (planningType==sim_holonomicpathplanning_xyg)
        {
            totalVect[0]=pos(0)=toBeExtended->values[0]+theVect[0];
            totalVect[1]=pos(1)=toBeExtended->values[1]+theVect[1];
            totalVect[2]=toBeExtended->values[2]+theVect[2];
            orient=_gammaAxisRotation*(C4Vector(C3Vector(0.0f,0.0f,totalVect[2]))*_gammaAxisRotationInv);
            forbiddenValues=areSomeValuesForbidden(totalVect);
        }
        if (planningType==sim_holonomicpathplanning_abg)
        {
            toBeExtended->getAllValues(pos,orient);
            C4Vector q(theVect);
            orient=orient*q;
            totalVect[0]=orient(0);
            totalVect[1]=orient(1);
            totalVect[2]=orient(2);
            totalVect[3]=orient(3);
            forbiddenValues=false;
        }
        if (planningType==sim_holonomicpathplanning_xyzg)
        {
            totalVect[0]=pos(0)=toBeExtended->values[0]+theVect[0];
            totalVect[1]=pos(1)=toBeExtended->values[1]+theVect[1];
            totalVect[2]=pos(2)=toBeExtended->values[2]+theVect[2];
            totalVect[3]=toBeExtended->values[3]+theVect[3];
            orient=_gammaAxisRotation*(C4Vector(C3Vector(0.0f,0.0f,totalVect[3]))*_gammaAxisRotationInv);
            forbiddenValues=areSomeValuesForbidden(totalVect);
        }
        if (planningType==sim_holonomicpathplanning_xabg)
        {
            toBeExtended->getAllValues(pos,orient);
            totalVect[0]=pos(0)=toBeExtended->values[0]+theVect[0];
            C4Vector q(theVect+1);
            orient=orient*q;
            totalVect[1]=orient(0);
            totalVect[2]=orient(1);
            totalVect[3]=orient(2);
            totalVect[4]=orient(3);
            forbiddenValues=areSomeValuesForbidden(totalVect);
        }
        if (planningType==sim_holonomicpathplanning_xyabg)
        {
            toBeExtended->getAllValues(pos,orient);
            totalVect[0]=pos(0)=toBeExtended->values[0]+theVect[0];
            totalVect[1]=pos(1)=toBeExtended->values[1]+theVect[1];
            C4Vector q(theVect+2);
            orient=orient*q;
            totalVect[2]=orient(0);
            totalVect[3]=orient(1);
            totalVect[4]=orient(2);
            totalVect[5]=orient(3);
            forbiddenValues=areSomeValuesForbidden(totalVect);
        }
        if (planningType==sim_holonomicpathplanning_xyzabg)
        {
            toBeExtended->getAllValues(pos,orient);
            totalVect[0]=pos(0)=toBeExtended->values[0]+theVect[0];
            totalVect[1]=pos(1)=toBeExtended->values[1]+theVect[1];
            totalVect[2]=pos(2)=toBeExtended->values[2]+theVect[2];
            C4Vector q(theVect+3);
            orient=orient*q;
            totalVect[3]=orient(0);
            totalVect[4]=orient(1);
            totalVect[5]=orient(2);
            totalVect[6]=orient(3);
            forbiddenValues=areSomeValuesForbidden(totalVect);
        }
        if (forbiddenValues)
        { // We hit on forbidden values!
            if (connect)
                return(nullptr);
            if (insertedPts!=0)
                return(toBeExtended);
            return(nullptr);
        }
        C7Vector transf(C4Vector(orient),pos);
        C7Vector tmpTr(_startDummyLTM*transf);
        _simSetObjectLocalTransformation_internal(dummy,tmpTr.X.data,tmpTr.Q.data);
        if (specialCase)
        {
            float d;
            doCollide(&d);
            if (d>=lastClosest_specialCase)
            { // This is acceptable (we extend a colliding state, but slowly moving away from the collision)
                lastClosest_specialCase=d;
            }
            else
            { // Here we are again coming closer to the initial colliding state --> we leave
                if (connect)
                    return(nullptr);
                if (insertedPts!=0)
                    return(toBeExtended);
                return(nullptr);
            }
        }
        else
        {
            if (doCollide(nullptr))
            { // We collided!
                if (connect)
                    return(nullptr);
                if (insertedPts!=0)
                    return(toBeExtended);
                return(nullptr);
            }
        }

        // No collision at this configuration! We extend it
        CHolonomicPathNode* save=toBeExtended;
        toBeExtended=save->copyYourself();
        toBeExtended->setAllValues(totalVect);

        toBeExtended->parent=save;
        nodeList->push_back(toBeExtended);
        if (leave)
            return(toBeExtended);
        insertedPts++;
    }
    return(nullptr);
}

int CHolonomicPathPlanning::getVector(CHolonomicPathNode* fromPoint,CHolonomicPathNode* toPoint,float vect[7],float e,float& artificialLength,bool dontDivide)
{ // if direction constraints are not respected, return value is -1 and vect does not contain anything
    // Otherwise return value is the number of times we have to add 'vect' to 'fromPoint' to reach 'toPoint'
    int retVal=-1;
    if (planningType==sim_holonomicpathplanning_xy)
    {
        vect[0]=toPoint->values[0]-fromPoint->values[0];
        vect[1]=toPoint->values[1]-fromPoint->values[1];
        if (areDirectionConstraintsRespected(vect))
        {
            artificialLength=sqrtf(vect[0]*vect[0]+vect[1]*vect[1]);
            if (dontDivide)
                return(1);
            retVal=(int)(artificialLength/e)+1;
            float l=(float)retVal;
            vect[0]/=l;
            vect[1]/=l;
        }
    }
    if (planningType==sim_holonomicpathplanning_xg)
    {
        vect[0]=toPoint->values[0]-fromPoint->values[0];
        vect[1]=CPathPlanningInterface::getNormalizedAngle(toPoint->values[1]-fromPoint->values[1]);
        if (areDirectionConstraintsRespected(vect))
        {
            artificialLength=vect[0]*vect[0];
            artificialLength=sqrtf(artificialLength+vect[1]*angularCoeff*vect[1]*angularCoeff);
            if (dontDivide)
                return(1);
            retVal=(int)(artificialLength/e)+1;
            float l=(float)retVal;
            vect[0]/=l;
            vect[1]/=l;
        }
    }
    if (planningType==sim_holonomicpathplanning_xyz)
    {
        vect[0]=toPoint->values[0]-fromPoint->values[0];
        vect[1]=toPoint->values[1]-fromPoint->values[1];
        vect[2]=toPoint->values[2]-fromPoint->values[2];
        if (areDirectionConstraintsRespected(vect))
        {
            artificialLength=sqrtf(vect[0]*vect[0]+vect[1]*vect[1]+vect[2]*vect[2]);
            if (dontDivide)
                return(1);
            retVal=(int)(artificialLength/e)+1;
            float l=(float)retVal;
            vect[0]/=l;
            vect[1]/=l;
            vect[2]/=l;
        }
    }
    if (planningType==sim_holonomicpathplanning_xyg)
    {
        vect[0]=toPoint->values[0]-fromPoint->values[0];
        vect[1]=toPoint->values[1]-fromPoint->values[1];
        vect[2]=CPathPlanningInterface::getNormalizedAngle(toPoint->values[2]-fromPoint->values[2]);
        if (areDirectionConstraintsRespected(vect))
        {
            artificialLength=vect[0]*vect[0]+vect[1]*vect[1];
            artificialLength=sqrtf(artificialLength+vect[2]*angularCoeff*vect[2]*angularCoeff);
            if (dontDivide)
                return(1);
            retVal=(int)(artificialLength/e)+1;
            float l=(float)retVal;
            vect[0]/=l;
            vect[1]/=l;
            vect[2]/=l;
        }
    }
    if (planningType==sim_holonomicpathplanning_abg)
    {
        C4Vector toP,fromP;
        C3Vector dum;
        toPoint->getAllValues(dum,toP);
        fromPoint->getAllValues(dum,fromP);
        C4Vector diff(fromP.getInverse()*toP);
        vect[0]=diff(0);
        vect[1]=diff(1);
        vect[2]=diff(2);
        vect[3]=diff(3);
        if (areDirectionConstraintsRespected(vect))
        {
            artificialLength=angularCoeff*fromP.getAngleBetweenQuaternions(toP);
            if (dontDivide)
                return(1);
            retVal=(int)(artificialLength/e)+1;
            float l=(float)retVal;
            C4Vector q;
            q.setIdentity();
            fromP.buildInterpolation(q,diff,1.0f/l);
            vect[0]=fromP(0);
            vect[1]=fromP(1);
            vect[2]=fromP(2);
            vect[3]=fromP(3);
        }
    }
    if (planningType==sim_holonomicpathplanning_xyzg)
    {
        vect[0]=toPoint->values[0]-fromPoint->values[0];
        vect[1]=toPoint->values[1]-fromPoint->values[1];
        vect[2]=toPoint->values[2]-fromPoint->values[2];
        vect[3]=CPathPlanningInterface::getNormalizedAngle(toPoint->values[3]-fromPoint->values[3]);
        if (areDirectionConstraintsRespected(vect))
        {
            artificialLength=vect[0]*vect[0]+vect[1]*vect[1]+vect[2]*vect[2];
            artificialLength=sqrtf(artificialLength+vect[3]*angularCoeff*vect[3]*angularCoeff);
            if (dontDivide)
                return(1);
            retVal=(int)(artificialLength/e)+1;
            float l=(float)retVal;
            vect[0]/=l;
            vect[1]/=l;
            vect[2]/=l;
            vect[3]/=l;
        }
    }
    if (planningType==sim_holonomicpathplanning_xabg)
    {
        vect[0]=toPoint->values[0]-fromPoint->values[0];
        C4Vector toP,fromP;
        C3Vector dum;
        toPoint->getAllValues(dum,toP);
        fromPoint->getAllValues(dum,fromP);
        C4Vector diff(fromP.getInverse()*toP);
        vect[1]=diff(0);
        vect[2]=diff(1);
        vect[3]=diff(2);
        vect[4]=diff(3);
        if (areDirectionConstraintsRespected(vect))
        {
            float ap=angularCoeff*fromP.getAngleBetweenQuaternions(toP);
            artificialLength=vect[0]*vect[0];
            artificialLength=sqrtf(artificialLength+ap*ap);

            if (dontDivide)
                return(1);
            retVal=(int)(artificialLength/e)+1;
            float l=(float)retVal;
            vect[0]/=l;
            C4Vector q;
            q.setIdentity();
            fromP.buildInterpolation(q,diff,1.0f/l);
            vect[1]=fromP(0);
            vect[2]=fromP(1);
            vect[3]=fromP(2);
            vect[4]=fromP(3);
        }
    }
    if (planningType==sim_holonomicpathplanning_xyabg)
    {
        vect[0]=toPoint->values[0]-fromPoint->values[0];
        vect[1]=toPoint->values[1]-fromPoint->values[1];
        C4Vector toP,fromP;
        C3Vector dum;
        toPoint->getAllValues(dum,toP);
        fromPoint->getAllValues(dum,fromP);
        C4Vector diff(fromP.getInverse()*toP);
        vect[2]=diff(0);
        vect[3]=diff(1);
        vect[4]=diff(2);
        vect[5]=diff(3);
        if (areDirectionConstraintsRespected(vect))
        {
            float ap=angularCoeff*fromP.getAngleBetweenQuaternions(toP);
            artificialLength=vect[0]*vect[0]+vect[1]*vect[1];
            artificialLength=sqrtf(artificialLength+ap*ap);

            if (dontDivide)
                return(1);
            retVal=(int)(artificialLength/e)+1;
            float l=(float)retVal;
            vect[0]/=l;
            vect[1]/=l;
            C4Vector q;
            q.setIdentity();
            fromP.buildInterpolation(q,diff,1.0f/l);
            vect[2]=fromP(0);
            vect[3]=fromP(1);
            vect[4]=fromP(2);
            vect[5]=fromP(3);
        }
    }
    if (planningType==sim_holonomicpathplanning_xyzabg)
    {
        vect[0]=toPoint->values[0]-fromPoint->values[0];
        vect[1]=toPoint->values[1]-fromPoint->values[1];
        vect[2]=toPoint->values[2]-fromPoint->values[2];
        C4Vector toP,fromP;
        C3Vector dum;
        toPoint->getAllValues(dum,toP);
        fromPoint->getAllValues(dum,fromP);
        C4Vector diff(fromP.getInverse()*toP);
        vect[3]=diff(0);
        vect[4]=diff(1);
        vect[5]=diff(2);
        vect[6]=diff(3);
        if (areDirectionConstraintsRespected(vect))
        {
            float ap=angularCoeff*fromP.getAngleBetweenQuaternions(toP);
            artificialLength=vect[0]*vect[0]+vect[1]*vect[1]+vect[2]*vect[2];
            artificialLength=sqrtf(artificialLength+ap*ap);

            if (dontDivide)
                return(1);
            retVal=(int)(artificialLength/e)+1;
            float l=(float)retVal;
            vect[0]/=l;
            vect[1]/=l;
            vect[2]/=l;
            C4Vector q;
            q.setIdentity();
            fromP.buildInterpolation(q,diff,1.0f/l);
            vect[3]=fromP(0);
            vect[4]=fromP(1);
            vect[5]=fromP(2);
            vect[6]=fromP(3);
        }
    }
    return(retVal);
}

bool CHolonomicPathPlanning::addVector(C3Vector& pos,C4Vector& orient,float vect[7])
{ // return value true means values are not forbidden!
    float auxVect[7];
    if (planningType==sim_holonomicpathplanning_xy)
    {
        pos(0)+=vect[0];
        pos(1)+=vect[1];
        auxVect[0]=pos(0);
        auxVect[1]=pos(1);
    }
    else if (planningType==sim_holonomicpathplanning_xg)
    {
        pos(0)+=vect[0];
        orient=orient*_gammaAxisRotation*(C4Vector(C3Vector(0.0f,0.0f,vect[1]))*_gammaAxisRotationInv);
        auxVect[0]=pos(0);
        auxVect[1]=(_gammaAxisRotationInv*orient*_gammaAxisRotation).getEulerAngles()(2);
    }
    else if (planningType==sim_holonomicpathplanning_xyz)
    {
        pos(0)+=vect[0];
        pos(1)+=vect[1];
        pos(2)+=vect[2];
        auxVect[0]=pos(0);
        auxVect[1]=pos(1);
        auxVect[2]=pos(2);
    }
    else if (planningType==sim_holonomicpathplanning_xyg)
    {
        pos(0)+=vect[0];
        pos(1)+=vect[1];
        orient=orient*_gammaAxisRotation*(C4Vector(C3Vector(0.0f,0.0f,vect[2]))*_gammaAxisRotationInv);
        auxVect[0]=pos(0);
        auxVect[1]=pos(1);
        auxVect[2]=(_gammaAxisRotationInv*orient*_gammaAxisRotation).getEulerAngles()(2);
    }
    else if (planningType==sim_holonomicpathplanning_abg)
    {
        orient=orient*C4Vector(vect);
        auxVect[0]=orient(0);
        auxVect[1]=orient(1);
        auxVect[2]=orient(2);
        auxVect[3]=orient(3);
    }
    else if (planningType==sim_holonomicpathplanning_xyzg)
    {
        pos(0)+=vect[0];
        pos(1)+=vect[1];
        pos(2)+=vect[2];
        orient=orient*_gammaAxisRotation*(C4Vector(C3Vector(0.0f,0.0f,vect[3]))*_gammaAxisRotationInv);
        auxVect[0]=pos(0);
        auxVect[1]=pos(1);
        auxVect[2]=pos(2);
        auxVect[3]=(_gammaAxisRotationInv*orient*_gammaAxisRotation).getEulerAngles()(2);
    }
    else if (planningType==sim_holonomicpathplanning_xabg)
    {
        pos(0)+=vect[0];
        orient=orient*C4Vector(vect+1);
        auxVect[0]=pos(0);
        auxVect[1]=orient(0);
        auxVect[2]=orient(1);
        auxVect[3]=orient(2);
        auxVect[4]=orient(3);
    }
    else if (planningType==sim_holonomicpathplanning_xyabg)
    {
        pos(0)+=vect[0];
        pos(1)+=vect[1];
        orient=orient*C4Vector(vect+2);
        auxVect[0]=pos(0);
        auxVect[1]=pos(1);
        auxVect[2]=orient(0);
        auxVect[3]=orient(1);
        auxVect[4]=orient(2);
        auxVect[5]=orient(3);
    }
    else // (planningType==sim_holonomicpathplanning_xyzabg)
    {
        pos(0)+=vect[0];
        pos(1)+=vect[1];
        pos(2)+=vect[2];
        orient=orient*C4Vector(vect+3);
        auxVect[0]=pos(0);
        auxVect[1]=pos(1);
        auxVect[2]=pos(2);
        auxVect[3]=orient(0);
        auxVect[4]=orient(1);
        auxVect[5]=orient(2);
        auxVect[6]=orient(3);
    }
    return(!areSomeValuesForbidden(auxVect));
}

int CHolonomicPathPlanning::smoothFoundPath(int steps,int maxTimePerPass)
{ // step specifies the number of passes (each pass is composed by a calculated sub-pass, and some random sub-pass)
    // We first copy foundPath:
    if (steps<2)
        return(1);
    if (invalidData)
        return(0);
    CDummyDummy* startDummy=(CDummyDummy*)_simGetObject_internal(startDummyID);
    if (startDummy==nullptr)
        return(0);

    if (foundPath.size()<3)
        return(0);

    if (foundPathSameStraightLineID_forSteppedSmoothing.size()==0)
    { // the first time we call this routine!
        sameStraightLineNextID_forSteppedSmoothing=0;
        for (int i=0;i<int(foundPath.size());i++)
            foundPathSameStraightLineID_forSteppedSmoothing.push_back(sameStraightLineNextID_forSteppedSmoothing++);
        numberOfRandomConnectionTries_forSteppedSmoothing=steps;
        numberOfRandomConnectionTriesLeft_forSteppedSmoothing=steps;
    }
    int startTime=simGetSystemTimeInMs_internal(-1);
    while (true)
    {
        if (numberOfRandomConnectionTriesLeft_forSteppedSmoothing<=0)
        { // we finished smoothing!
            return(1); 
        }
        if (_simGetTimeDiffInMs_internal(startTime)>maxTimePerPass)
            return(-1); // we are not yet finished, but we did enough for the time we had
        numberOfRandomConnectionTriesLeft_forSteppedSmoothing--;
        int lowIndex,highIndex;
        CHolonomicPathNode* startP;
        CHolonomicPathNode* endP;
        for (int randomPass=0;randomPass<5;randomPass++)
        { // If randomPass==0, the pass is not random, i.e. the low and high indices are calculated
            startP=nullptr; // added on 2010/09/09
            if (randomPass==0)
            { // We calculate lowIndex and highIndex!
                float span=float(foundPath.size())/float(numberOfRandomConnectionTries_forSteppedSmoothing);
                while ( (span<5)&&(numberOfRandomConnectionTries_forSteppedSmoothing>1) )
                {
                    numberOfRandomConnectionTries_forSteppedSmoothing--;
                    if (numberOfRandomConnectionTriesLeft_forSteppedSmoothing>=numberOfRandomConnectionTries_forSteppedSmoothing)
                        numberOfRandomConnectionTriesLeft_forSteppedSmoothing=numberOfRandomConnectionTries_forSteppedSmoothing-1;
                    span=float(foundPath.size())/float(numberOfRandomConnectionTries_forSteppedSmoothing);
                }
                if (numberOfRandomConnectionTries_forSteppedSmoothing<=1)
                    return(1); // finished!
                lowIndex=int(span*float(numberOfRandomConnectionTriesLeft_forSteppedSmoothing+0));
                highIndex=int(span*float(numberOfRandomConnectionTriesLeft_forSteppedSmoothing+1));
                while (highIndex>=int(foundPath.size())) // probably not needed
                    highIndex--;
                if (foundPathSameStraightLineID_forSteppedSmoothing[lowIndex]!=foundPathSameStraightLineID_forSteppedSmoothing[highIndex])
                { // otherwise this pass is skipped!
                    startP=foundPath[lowIndex];
                    endP=foundPath[highIndex];
                }
            }
            else
            { // We randomly chose lowIndex and highIndex!
                for (int i=0;i<5;i++)
                { // so that if we have only very few ids, we don't stay stuck here
                    int ra=int((SIM_RAND_FLOAT*float(foundPath.size()))-0.5f);
                    int rb=int((SIM_RAND_FLOAT*float(foundPath.size()))-0.5f);
                    if ( (ra!=rb)&&(abs(ra-rb)>1)&&(foundPathSameStraightLineID_forSteppedSmoothing[ra]!=foundPathSameStraightLineID_forSteppedSmoothing[rb]) )
                    {
                        lowIndex=SIM_MIN(ra,rb);
                        highIndex=SIM_MAX(ra,rb);
                        startP=foundPath[lowIndex];
                        endP=foundPath[highIndex];
                        break;
                    }
                }
            }
            if (startP!=nullptr)
            { // Now let's try to link highIndex from lowIndex with a "straight" line:
                float vect[7];
                float artificialVectorLength;
                int passes=getVector(startP,endP,vect,stepSize,artificialVectorLength,false);
                if ( (passes!=-1)&&(highIndex-(lowIndex+1)>passes-1) )
                { // no forbidden direction, and the number of nodes is reduced!
                    C3Vector pos;
                    C4Vector orient;
                    startP->getAllValues(pos,orient);

                    C3Vector posCop(pos);
                    C4Vector orientCop(orient);
                    bool impossible=false;
                    C7Vector originalLocal;
                    _simGetObjectLocalTransformation_internal(startDummy,originalLocal.X.data,originalLocal.Q.data,true);
                    for (int currentPass=0;currentPass<passes-1;currentPass++)
                    {
                        if (!addVector(pos,orient,vect))
                        {
                            impossible=true;
                            break; // We are in forbidden values!
                        }
                        C7Vector transf(orient,pos);
                        C7Vector tmpTr(_startDummyLTM*transf);
                        _simSetObjectLocalTransformation_internal(startDummy,tmpTr.X.data,tmpTr.Q.data);
                        if (doCollide(nullptr))
                        {
                            impossible=true;
                            break;
                        }
                    }
                    _simSetObjectLocalTransformation_internal(startDummy,originalLocal.X.data,originalLocal.Q.data);
                    pos=posCop;
                    orient=orientCop;
                    if (!impossible)
                    { // Path was collision free:
                        // We delete the nodes between low and high, then remove some, to have to correct desired length:
                        for (int i=lowIndex+1;i<highIndex;i++)
                            delete foundPath[i];
                        int a=lowIndex+1;
                        int b=lowIndex+1+highIndex-(lowIndex+1)-(passes-1)-0;
                        foundPath.erase(foundPath.begin()+a,foundPath.begin()+b);
                        foundPathSameStraightLineID_forSteppedSmoothing.erase(foundPathSameStraightLineID_forSteppedSmoothing.begin()+a,foundPathSameStraightLineID_forSteppedSmoothing.begin()+b);

                        for (int currentPass=0;currentPass<passes-1;currentPass++)
                        {
                            addVector(pos,orient,vect);
                            CHolonomicPathNode* it=endP->copyYourself(); // just to have the right size!
                            it->setAllValues(pos,orient);
                            foundPath[lowIndex+1+currentPass]=it;
                            foundPathSameStraightLineID_forSteppedSmoothing[lowIndex+1+currentPass]=sameStraightLineNextID_forSteppedSmoothing;
                        }
                        sameStraightLineNextID_forSteppedSmoothing++;
                    }
                }
            }
        }
    }
    return(0); // will never pass here!
}

void CHolonomicPathPlanning::getPathData(std::vector<float>& data)
{
    data.clear();
    if (invalidData)
        return;
    for (int i=0;i<int(foundPath.size());i++)
    {
        CHolonomicPathNode* theNode=foundPath[i];
        C3Vector p;
        C4Vector o;
        theNode->getAllValues(p,o);

        C7Vector conf(o,p);
        conf=_startDummyCTM*conf;
        data.push_back(conf(0));
        data.push_back(conf(1));
        data.push_back(conf(2));
        data.push_back(conf(3));
        data.push_back(conf(4));
        data.push_back(conf(5));
        data.push_back(conf(6));
    }
}

bool CHolonomicPathPlanning::areDirectionConstraintsRespected(float vect[7])
{
    if (planningType==sim_holonomicpathplanning_xy)
    {
        if (!_directionConstraintsOn)
            return(true);
        for (int i=0;i<2;i++)
        {
            if ( (_directionConstraints[i]==-1)&&(vect[i]>0.0f) )
                return(false);
            if ( (_directionConstraints[i]==+1)&&(vect[i]<0.0f) )
                return(false);
        }
        return(true);
    }
    if (planningType==sim_holonomicpathplanning_xg)
    {
        CPathPlanningInterface::getNormalizedAngle(vect[1]);
        if (!_directionConstraintsOn)
            return(true);
        if ( (_directionConstraints[0]==-1)&&(vect[0]>0.0f) )
            return(false);
        if ( (_directionConstraints[0]==+1)&&(vect[0]<0.0f) )
            return(false);
        if ( (_directionConstraints[3]==-1)&&(vect[1]>0.0f) )
            return(false);
        if ( (_directionConstraints[3]==+1)&&(vect[1]<0.0f) )
            return(false);
        return(true);
    }
    if (planningType==sim_holonomicpathplanning_xyz)
    {
        if (!_directionConstraintsOn)
            return(true);
        for (int i=0;i<3;i++)
        {
            if ( (_directionConstraints[i]==-1)&&(vect[i]>0.0f) )
                return(false);
            if ( (_directionConstraints[i]==+1)&&(vect[i]<0.0f) )
                return(false);
        }
        return(true);
    }
    if (planningType==sim_holonomicpathplanning_xyg)
    {
        CPathPlanningInterface::getNormalizedAngle(vect[2]);
        if (!_directionConstraintsOn)
            return(true);
        for (int i=0;i<2;i++)
        {
            if ( (_directionConstraints[i]==-1)&&(vect[i]>0.0f) )
                return(false);
            if ( (_directionConstraints[i]==+1)&&(vect[i]<0.0f) )
                return(false);
        }
        if ( (_directionConstraints[3]==-1)&&(vect[2]>0.0f) )
            return(false);
        if ( (_directionConstraints[3]==+1)&&(vect[2]<0.0f) )
            return(false);
        return(true);
    }
    if (planningType==sim_holonomicpathplanning_abg)
    { // No direction constraints for A,B,G here!
        return(true);
    }
    if (planningType==sim_holonomicpathplanning_xyzg)
    {
        CPathPlanningInterface::getNormalizedAngle(vect[3]);
        if (!_directionConstraintsOn)
            return(true);
        for (int i=0;i<3;i++)
        {
            if ( (_directionConstraints[i]==-1)&&(vect[i]>0.0f) )
                return(false);
            if ( (_directionConstraints[i]==+1)&&(vect[i]<0.0f) )
                return(false);
        }
        if ( (_directionConstraints[3]==-1)&&(vect[3]>0.0f) )
            return(false);
        if ( (_directionConstraints[3]==+1)&&(vect[3]<0.0f) )
            return(false);
        return(true);
    }
    if (planningType==sim_holonomicpathplanning_xabg)
    { // No direction constraints for A,B,G here!
        if (!_directionConstraintsOn)
            return(true);
        if ( (_directionConstraints[0]==-1)&&(vect[0]>0.0f) )
            return(false);
        if ( (_directionConstraints[0]==+1)&&(vect[0]<0.0f) )
            return(false);
        return(true);
    }
    if (planningType==sim_holonomicpathplanning_xyabg)
    { // No direction constraints for A,B,G here!
        if (!_directionConstraintsOn)
            return(true);
        for (int i=0;i<2;i++)
        {
            if ( (_directionConstraints[i]==-1)&&(vect[i]>0.0f) )
                return(false);
            if ( (_directionConstraints[i]==+1)&&(vect[i]<0.0f) )
                return(false);
        }
        return(true);
    }
    if (planningType==sim_holonomicpathplanning_xyzabg)
    { // No direction constraints for A,B,G here!
        if (!_directionConstraintsOn)
            return(true);
        for (int i=0;i<3;i++)
        {
            if ( (_directionConstraints[i]==-1)&&(vect[i]>0.0f) )
                return(false);
            if ( (_directionConstraints[i]==+1)&&(vect[i]<0.0f) )
                return(false);
        }
        return(true);
    }
    return(true);
}

bool CHolonomicPathPlanning::areSomeValuesForbidden(float values[7])
{
    float gamma=0.0f; 
    if (planningType==sim_holonomicpathplanning_xy)
    {
        if ((values[0]<_searchMinVal[0])||(values[0]>_searchMinVal[0]+_searchRange[0]))
            return(true);
        if ((values[1]<_searchMinVal[1])||(values[1]>_searchMinVal[1]+_searchRange[1]))
            return(true);
        return(false);
    }
    if (planningType==sim_holonomicpathplanning_xg)
    {
        if ((values[0]<_searchMinVal[0])||(values[0]>_searchMinVal[0]+_searchRange[0]))
            return(true);
        gamma=values[1];
    }
    if (planningType==sim_holonomicpathplanning_xyz)
    {
        if ((values[0]<_searchMinVal[0])||(values[0]>_searchMinVal[0]+_searchRange[0]))
            return(true);
        if ((values[1]<_searchMinVal[1])||(values[1]>_searchMinVal[1]+_searchRange[1]))
            return(true);
        if ((values[2]<_searchMinVal[2])||(values[2]>_searchMinVal[2]+_searchRange[2]))
            return(true);
        return(false);
    }
    if (planningType==sim_holonomicpathplanning_xyg)
    {
        if ((values[0]<_searchMinVal[0])||(values[0]>_searchMinVal[0]+_searchRange[0]))
            return(true);
        if ((values[1]<_searchMinVal[1])||(values[1]>_searchMinVal[1]+_searchRange[1]))
            return(true);
        gamma=values[2];
    }
    if (planningType==sim_holonomicpathplanning_abg)
    {
        return(false);
    }
    if (planningType==sim_holonomicpathplanning_xyzg)
    {
        if ((values[0]<_searchMinVal[0])||(values[0]>_searchMinVal[0]+_searchRange[0]))
            return(true);
        if ((values[1]<_searchMinVal[1])||(values[1]>_searchMinVal[1]+_searchRange[1]))
            return(true);
        if ((values[2]<_searchMinVal[2])||(values[2]>_searchMinVal[2]+_searchRange[2]))
            return(true);
        gamma=values[3];
    }
    if (planningType==sim_holonomicpathplanning_xabg)
    {
        if ((values[0]<_searchMinVal[0])||(values[0]>_searchMinVal[0]+_searchRange[0]))
            return(true);
        return(false);
    }
    if (planningType==sim_holonomicpathplanning_xyabg)
    {
        if ((values[0]<_searchMinVal[0])||(values[0]>_searchMinVal[0]+_searchRange[0]))
            return(true);
        if ((values[1]<_searchMinVal[1])||(values[1]>_searchMinVal[1]+_searchRange[1]))
            return(true);
        return(false);
    }
    if (planningType==sim_holonomicpathplanning_xyzabg)
    {
        if ((values[0]<_searchMinVal[0])||(values[0]>_searchMinVal[0]+_searchRange[0]))
            return(true);
        if ((values[1]<_searchMinVal[1])||(values[1]>_searchMinVal[1]+_searchRange[1]))
            return(true);
        if ((values[2]<_searchMinVal[2])||(values[2]>_searchMinVal[2]+_searchRange[2]))
            return(true);
        return(false);
    }
    // We check the gamma value here:
    if (_searchRange[3]>(359.0f*degToRad_f))
        return(false);
    // Search range is smaller than 360 degrees:
    while (gamma>_searchMinVal[3])
        gamma-=piValTimes2_f;
    while (gamma<_searchMinVal[3])
        gamma+=piValTimes2_f;
    return(gamma>(_searchMinVal[3]+_searchRange[3]));
}

bool CHolonomicPathPlanning::doCollide(float* dist)
{// dist can be nullptr. Dist returns the actual distance only when return value is true!! otherwise it is SIM_MAX_FLOAT!!
    if (dist!=nullptr)
        dist[0]=SIM_MAX_FLOAT;
    if (obstacleClearanceAndMaxDistance[0]<=0.0f)
    {
        if ( (obstacleCollectionID==-1)&&(!_allIsObstacle) )
            return(false);
        if (_simDoEntitiesCollide_internal(robotCollectionID,obstacleCollectionID,buffer,false,false,true)!=0)
        {
            if (dist!=nullptr)
                dist[0]=0.0f;
            return(true);
        }
        return(false);
    }
    else
    {
        if ( (obstacleCollectionID==-1)&&(!_allIsObstacle) )
            return(false);
        if (obstacleClearanceAndMaxDistance[1]<=0.0f)
        { // no max distance, only min. distance:
            float ray[7];
            float theDist=obstacleClearanceAndMaxDistance[0];
            if (_simGetDistanceBetweenEntitiesIfSmaller_internal(robotCollectionID,obstacleCollectionID,&theDist,ray,buffer,false,false,true)!=0)
            {
                if (dist!=nullptr)
                    dist[0]=theDist;
                return(true);
            }
            return(false);
        }
        else
        { // min. distance and max. distance:
            float ray[7];
            float theDist=obstacleClearanceAndMaxDistance[1];
            if (_simGetDistanceBetweenEntitiesIfSmaller_internal(robotCollectionID,obstacleCollectionID,&theDist,ray,buffer,false,false,true)!=0)
            {
                if (theDist>=obstacleClearanceAndMaxDistance[0])
                    return(false);
                if (dist!=nullptr)
                    dist[0]=theDist;
                return(true);
            }
            if (dist!=nullptr)
                dist[0]=theDist;
            return(true);
        }
    }
}
