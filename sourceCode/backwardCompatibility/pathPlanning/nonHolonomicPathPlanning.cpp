
#include "nonHolonomicPathPlanning.h"
#include "pathPlanningInterface.h"
#include "v_rep_internal.h"

#define SIM_MIN(a,b) (((a)<(b)) ? (a) : (b))
#define SIM_MAX(a,b) (((a)>(b)) ? (a) : (b))

CNonHolonomicPathPlanning::CNonHolonomicPathPlanning(int theStartDummyID,int theGoalDummyID,
                        int theRobotCollectionID,int theObstacleCollectionID,int ikGroupID,float theAngularCoeff,
                        float theSteeringAngleCoeff,float theMaxSteeringAngleVariation,float theMaxSteeringAngle,
                        float theStepSize,const float theSearchMinVal[2],const float theSearchRange[2],
                        const int theDirectionConstraints[2],const float clearanceAndMaxDistance[2])
{
    isHolonomic=false;
    _allIsObstacle=(theObstacleCollectionID==-1);
    firstPass=true;
    invalidData=true;
    obstacleClearanceAndMaxDistance[0]=clearanceAndMaxDistance[0];
    obstacleClearanceAndMaxDistance[1]=clearanceAndMaxDistance[1];
    fromStart.reserve(300000);
    fromGoal.reserve(300000);
    stepSize=theStepSize;
    angularCoeff=theAngularCoeff;
    steeringAngleCoeff=theSteeringAngleCoeff;
    maxSteeringAngleVariation=theMaxSteeringAngleVariation;
    maxSteeringAngle=theMaxSteeringAngle;
    minTurningRadius=stepSize/(2.0f*sin(maxSteeringAngle/2.0f));
    _startDummyID=theStartDummyID;
    CDummyDummy* startDummy=(CDummyDummy*)_simGetObject_internal(_startDummyID);
    CDummyDummy* goalDummy=(CDummyDummy*)_simGetObject_internal(theGoalDummyID);
    if ( (startDummy==nullptr)||(goalDummy==nullptr) )
        return;
    _simGetObjectCumulativeTransformation_internal(startDummy,_startDummyCTM.X.data,_startDummyCTM.Q.data,false);
    _simGetObjectLocalTransformation_internal(startDummy,_startDummyLTM.X.data,_startDummyLTM.Q.data,false);

    C7Vector startDummyCTMInv(_startDummyCTM.getInverse());
    C7Vector tmpTr;
    _simGetObjectCumulativeTransformation_internal(goalDummy,tmpTr.X.data,tmpTr.Q.data,false);
    C7Vector gd(startDummyCTMInv*tmpTr);
    C3Vector gdEuler(gd.Q.getEulerAngles());
    gdEuler(0)=0.0f;
    gdEuler(1)=0.0f;
    gd.X(2)=0.0f;//startDummyLocal.X(2);
    fromStart.push_back(new CNonHolonomicPathNode(0.0f,0.0f,0.0f));//startDummyLocal.X(0),startDummyLocal.X(1),startDummyLocalEuler(2)));
    fromGoal.push_back(new CNonHolonomicPathNode(gd.X(0),gd.X(1),gdEuler(2)));
    robotCollectionID=theRobotCollectionID;
    obstacleCollectionID=theObstacleCollectionID;

    // Following sets the _startConfInterferenceState to SIM_MAX_FLOAT if not colliding or above distance threshold.
    // Otherwise it is set to 0.0 (colliding) or the distance with the obstacle
    // This is used to allow path planning also when in an initial collision state. In that case the initial path segment
    // will follow a distance to obstacle gradient that is increasing until above the collision distance
    doCollide(&_startConfInterferenceState);

    for (int i=0;i<2;i++)
    {
        searchMinVal[i]=theSearchMinVal[i];
        searchRange[i]=theSearchRange[i];
        directionConstraints[i]=theDirectionConstraints[i];
    }
    buffer[0]=-1;
    invalidData=false;
}

CNonHolonomicPathPlanning::~CNonHolonomicPathPlanning()
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

void CNonHolonomicPathPlanning::setStepSize(float size)
{
    stepSize=size;
}

void CNonHolonomicPathPlanning::getSearchTreeData(std::vector<float>& data,bool fromTheStart)
{
    std::vector<CNonHolonomicPathNode*>* cont;
    if (fromTheStart)
        cont=&fromStart;
    else
        cont=&fromGoal;
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

int CNonHolonomicPathPlanning::searchPath(int maxTimePerPass)
{ // maxTimePerPass is in miliseconds
    if (invalidData)
        return(0);
    if ( (fromStart.size()==0)||(fromGoal.size()==0)||(foundPath.size()!=0) )
        return(0);

    // Following since 2010/08/19 so that we can move the "robot" while we search:
    CDummyDummy* startDummy=(CDummyDummy*)_simGetObject_internal(_startDummyID);
    if (startDummy==nullptr)
        return(0);
    C7Vector dumSavedConf;
    _simGetObjectLocalTransformation_internal(startDummy,dumSavedConf.X.data,dumSavedConf.Q.data,false);

    std::vector<CNonHolonomicPathNode*>* current=&fromStart;
    std::vector<CNonHolonomicPathNode*>* nextCurrent=&fromGoal;
    std::vector<CNonHolonomicPathNode*>* tmpCurrent=nullptr;
    int initTime=simGetSystemTimeInMs_internal(-1);
    int pathWasFound=0;
    int dirConstraintsSave[2]={directionConstraints[0],directionConstraints[1]};
    while (_simGetTimeDiffInMs_internal(initTime)<maxTimePerPass)
    {
        CNonHolonomicPathNode* savedRandNode=nullptr;
        for (int i=0;i<2;i++)
        {
            CNonHolonomicPathNode* randNode;
            if (savedRandNode!=nullptr)
                randNode=savedRandNode;
            else
            {
                randNode=new CNonHolonomicPathNode(searchMinVal[0]+searchRange[0]*SIM_RAND_FLOAT,
                                                searchMinVal[1]+searchRange[1]*SIM_RAND_FLOAT,
                                                -piValue_f+piValTimes2_f*SIM_RAND_FLOAT);
                savedRandNode=randNode;
            }
            CNonHolonomicPathNode* closest=nullptr;
            if (!firstPass)
                closest=getClosestNode(*current,randNode,i==0,false);
            if ( (closest!=nullptr)||firstPass )
            {
                if (!firstPass)
                    closest=extend(current,closest,randNode,i==0,startDummy);
                if ( (closest!=nullptr)||firstPass )
                {
                    for (int constr=0;constr<2;constr++) // We have to inverse the constraints!!
                        directionConstraints[constr]*=-1;
                    CNonHolonomicPathNode* closestConnect=nullptr;
                    if (!firstPass)
                        closestConnect=getClosestNode(*nextCurrent,closest,i==1,true);
                    if ( (closestConnect!=nullptr)||firstPass )
                    {
                        if (firstPass)
                        {
                            closestConnect=(*nextCurrent)[0];
                            closest=(*current)[0];
                            firstPass=false;
                        }
                        closestConnect=connect(nextCurrent,current,closestConnect,closest,i==1,true,false,startDummy);
                        if (closestConnect!=nullptr)
                        {
                            if (current==&fromStart)
                            {
                                CNonHolonomicPathNode* iterat=closest;
                                while (iterat!=nullptr)
                                {
                                    foundPath.insert(foundPath.begin(),new CNonHolonomicPathNode(iterat));
                                    iterat=iterat->parent;
                                }
                                iterat=closestConnect;
                                while (iterat!=nullptr)
                                {
                                    foundPath.push_back(new CNonHolonomicPathNode(iterat));
                                    iterat=iterat->parent;  
                                }
                            }
                            else
                            {
                                CNonHolonomicPathNode* iterat=closest;
                                while (iterat!=nullptr)
                                {
                                    foundPath.push_back(new CNonHolonomicPathNode(iterat));
                                    iterat=iterat->parent;
                                }
                                iterat=closestConnect;
                                while (iterat!=nullptr)
                                {
                                    foundPath.insert(foundPath.begin(),new CNonHolonomicPathNode(iterat));
                                    iterat=iterat->parent;
                                }
                            }
                            pathWasFound=1;
                        }
                    }
                    for (int constr=0;constr<2;constr++) // We have to inverse the constraints!!
                        directionConstraints[constr]*=-1;
                }
            }
            if (pathWasFound!=0)
                break;
            tmpCurrent=nextCurrent;
            nextCurrent=current;
            current=tmpCurrent;
            for (int constr=0;constr<2;constr++) // We have to inverse the constraints!!
                directionConstraints[constr]*=-1;
        }
        delete savedRandNode;
        if (pathWasFound!=0)
            break;
    }
    directionConstraints[0]=dirConstraintsSave[0];
    directionConstraints[1]=dirConstraintsSave[1];
    if (maxTimePerPass==61855195)
        return(131183);

    _simSetObjectLocalTransformation_internal(startDummy,dumSavedConf.X.data,dumSavedConf.Q.data);

    return(pathWasFound);
}

bool CNonHolonomicPathPlanning::setPartialPath()
{
    CNonHolonomicPathNode* it=getClosestNode(fromStart,fromGoal[0],true,false);
    while (it!=nullptr)
    {
        foundPath.insert(foundPath.begin(),new CNonHolonomicPathNode(it));
        it=it->parent;
    }
    if (foundPath.size()>7)
    { // This is so that we don't end facing a wall without any chance to escape
        for (int i=0;i<5;i++)
        {
            delete foundPath[foundPath.size()-1];
            foundPath.pop_back();
        }
        return(true);
    }
    return(false);
}


CNonHolonomicPathNode* CNonHolonomicPathPlanning::getClosestNode(std::vector<CNonHolonomicPathNode*>& nodes,CNonHolonomicPathNode* sample,bool forward,bool forConnection)
{
    float minD=SIM_MAX_FLOAT;
    int index=-1;
    float dPart2=2.0f*minTurningRadius;
    if (forConnection)
        dPart2=6.0f*minTurningRadius;
    for (int i=0;i<int(nodes.size());i++)
    {
        float vect[3];
        vect[0]=sample->values[0]-nodes[i]->values[0];
        vect[1]=sample->values[1]-nodes[i]->values[1];
        vect[2]=sample->values[2]-nodes[i]->values[2];
        float dPart1=vect[0]*vect[0]+vect[1]*vect[1];
        if ( (dPart1>dPart2))
        {
            float d=dPart1;//+fabs(vect[2])*0.01f;
            if (d<minD)
            {
                minD=d;
                index=i;
            }
        }
    }
    if (index!=-1)
        return(nodes[index]);
    return(nullptr);
}

CNonHolonomicPathNode* CNonHolonomicPathPlanning::extend(std::vector<CNonHolonomicPathNode*>* currentList,CNonHolonomicPathNode* toBeExtended,CNonHolonomicPathNode* extention,bool forward,CDummyDummy* startDummy)
{   // Return value is !=nullptr if extention was performed to some extent
    bool specialCase=( (fromStart==currentList[0])&&(toBeExtended==fromStart[0])&&(_startConfInterferenceState!=SIM_MAX_FLOAT) );
    float lastClosest_specialCase=_startConfInterferenceState;

    float dir=1.0f;
    if (!forward)
        dir=-1.0f;
    int directionCounter=0;
    bool awayDirection=false; 
    int i;
    for (i=0;i<1000;i++)
    {
        float theta=toBeExtended->values[2];
        float dy=extention->values[1]-toBeExtended->values[1];
        float dx=extention->values[0]-toBeExtended->values[0];
        float sqDist=dx*dx+dy*dy;
        float phi=atan2(dy,dx);
        float diff=phi-theta;
        float comp=0.0f;
        if (dir<0.0f)
        {
            comp=piValue_f;
            diff=phi-theta+piValue_f;
        }
        diff=CPathPlanningInterface::getNormalizedAngle(diff);
        if (fabs(diff)>maxSteeringAngle)
        {
            if (diff<0.0f)
                diff=-maxSteeringAngle;
            else
                diff=maxSteeringAngle;
        }
        float x=toBeExtended->values[0]+stepSize*cos(theta+diff+comp);
        float y=toBeExtended->values[1]+stepSize*sin(theta+diff+comp);
        float t=toBeExtended->values[2]+diff;
        // 1. Valid values?
        if ( (x>searchMinVal[0])&&(x<searchMinVal[0]+searchRange[0])&&
                (y>searchMinVal[1])&&(y<searchMinVal[1]+searchRange[1]) )
        { // new values are valid
            C3Vector trEuler(0.0f,0.0f,t);
            C7Vector tr(C4Vector(trEuler),C3Vector(0.0f,0.0f,0.0f));
            tr.X(0)=x;
            tr.X(1)=y;
            C7Vector tmpTr(_startDummyLTM*tr);
            _simSetObjectLocalTransformation_internal(startDummy,tmpTr.X.data,tmpTr.Q.data);
            if (!specialCase)
            {
                if (!doCollide(nullptr))
                {
                    CNonHolonomicPathNode* newNode=new CNonHolonomicPathNode(x,y,t);
                    newNode->parent=toBeExtended;
                    toBeExtended=newNode;
                    currentList->push_back(toBeExtended);
                    // close enough?
                    dy=extention->values[1]-y;
                    dx=extention->values[0]-x;
                    float sqDist2=dx*dx+dy*dy;
                    bool awayDirection2=(sqDist2>sqDist);
                    if (i==0)
                        awayDirection=awayDirection2;
                    else
                    {
                        if (awayDirection2!=awayDirection)
                            directionCounter++;
                        if (directionCounter>1)
                            break;
                    }
                    if ( dy*dy+dx*dx<stepSize*stepSize )
                        return(toBeExtended); // Point was reached!
                }
                else
                    break;
            }
            else
            { // we have a special case here!
                float d;
                doCollide(&d);
                if (d>=lastClosest_specialCase)
                { // This is acceptable (we extend a colliding state, but slowly moving away from the collision)
                    lastClosest_specialCase=d;
                    CNonHolonomicPathNode* newNode=new CNonHolonomicPathNode(x,y,t);
                    newNode->parent=toBeExtended;
                    toBeExtended=newNode;
                    currentList->push_back(toBeExtended);
                    // close enough?
                    dy=extention->values[1]-y;
                    dx=extention->values[0]-x;
                    float sqDist2=dx*dx+dy*dy;
                    bool awayDirection2=(sqDist2>sqDist);
                    if (i==0)
                        awayDirection=awayDirection2;
                    else
                    {
                        if (awayDirection2!=awayDirection)
                            directionCounter++;
                        if (directionCounter>1)
                            break;
                    }
                    if ( dy*dy+dx*dx<stepSize*stepSize )
                        return(toBeExtended); // Point was reached!
                }
                else
                { // Here we are again coming closer to the initial colliding state --> we leave
                    break;
                }
            }
        }
        else
            break;
    }
    if (i<=1)
        return(nullptr);
    return(toBeExtended);
}

CNonHolonomicPathNode* CNonHolonomicPathPlanning::connect(std::vector<CNonHolonomicPathNode*>* currentList,std::vector<CNonHolonomicPathNode*>* nextList,CNonHolonomicPathNode* toBeExtended,CNonHolonomicPathNode* extention,bool forward,bool connect,bool test,CDummyDummy* startDummy)
{   // if connect is true the return value indicates that connection can be performed!
    // other wise return value different from null means that toBeExtended could be extended to some extent!
    // if test is true, nothing is changed in the lists, we just check if connection could be made!
    std::vector<CNonHolonomicPathNode*> fromStartP;
    std::vector<CNonHolonomicPathNode*> fromGoalP;
    CNonHolonomicPathNode* fromStartHandle=toBeExtended;
    CNonHolonomicPathNode* fromGoalHandle=extention;
    std::vector<CNonHolonomicPathNode*>* currentL=&fromStartP;
    std::vector<CNonHolonomicPathNode*>* nextL=&fromGoalP;
    std::vector<CNonHolonomicPathNode*>* tmpL=nullptr;
    CNonHolonomicPathNode* currentHandle=fromStartHandle;
    CNonHolonomicPathNode* nextHandle=fromGoalHandle;
    CNonHolonomicPathNode* tmpHandle=nullptr;
    CNonHolonomicPathNode* lastHandleFromStart=nullptr;
    
    CNonHolonomicPathNode st(toBeExtended); // Needed when test is true
    CNonHolonomicPathNode gl(extention);
    if (test)
    {
        currentHandle=&st;
        nextHandle=&gl;
    }


    float dir=1.0f;
    if (!forward)
        dir=-1.0f;
    bool couldReach=false;
    int directionCounter=0;
    bool awayDirection=false; // 'false' not needed but for warning supression
    for (int i=0;i<1000;i++)
    {
        float theta=currentHandle->values[2];
        float deltaTheta=nextHandle->values[2]-theta;
        float dy=nextHandle->values[1]-currentHandle->values[1];
        float dx=nextHandle->values[0]-currentHandle->values[0];
        float sqDist=dx*dx+dy*dy;
        float phi=0.0f;
        if ( (fabs(dx)>0.0001f)||(fabs(dy)>0.0001f) )
            phi=atan2(dy,dx);
        float diff=phi-theta;
        float comp=0.0f;
        if (dir<0.0f)
        {
            comp=piValue_f;
            diff=phi-theta+piValue_f;
        }
        diff=CPathPlanningInterface::getNormalizedAngle(diff);
        if (fabs(diff)>maxSteeringAngle)
        {
            if (diff<0.0f)
                diff=-maxSteeringAngle;
            else
                diff=maxSteeringAngle;
        }
        deltaTheta=CPathPlanningInterface::getNormalizedAngle(deltaTheta);
        float nSa=diff;
        float x=currentHandle->values[0]+stepSize*cos(theta+nSa+comp);
        float y=currentHandle->values[1]+stepSize*sin(theta+nSa+comp);
        float t=currentHandle->values[2]+nSa;
        // 1. Valid values?
        if ( (x>searchMinVal[0])&&(x<searchMinVal[0]+searchRange[0])&&
                (y>searchMinVal[1])&&(y<searchMinVal[1]+searchRange[1]) )
        { // new values are valid
            C3Vector trEuler(0.0f,0.0f,t);
            C7Vector tr(C4Vector(trEuler),C3Vector(0.0f,0.0f,0.0f));
            tr.X(0)=x;
            tr.X(1)=y;
            C7Vector tmpTr(_startDummyLTM*tr);
            _simSetObjectLocalTransformation_internal(startDummy,tmpTr.X.data,tmpTr.Q.data);
            if (!doCollide(nullptr))
            {
                if (test)
                {
                    currentHandle->values[0]=x;
                    currentHandle->values[1]=y;
                    currentHandle->values[2]=t;
                }
                else
                {
                    CNonHolonomicPathNode* newNode=new CNonHolonomicPathNode(x,y,t);
                    newNode->parent=currentHandle;
                    currentHandle=newNode;
                    currentL->push_back(newNode);
                    if (currentL==&fromStartP)
                        lastHandleFromStart=newNode;
                }
                // close enough?
                float dTheta=nextHandle->values[2]-t;
                dTheta=CPathPlanningInterface::getNormalizedAngle(dTheta);
                dy=nextHandle->values[1]-y;
                dx=nextHandle->values[0]-x;

                float sqDist2=dx*dx+dy*dy;
                bool awayDirection2=(sqDist2>sqDist);
                if (i==0)
                    awayDirection=awayDirection2;
                else
                {
                    if (awayDirection2!=awayDirection)
                        directionCounter++;
                    if (directionCounter>1)
                        break;
                }
                if ( (dy*dy+dx*dx)<(1.2f*stepSize*stepSize) ) // 1.2f is important!!
                {
                    if (fabs(dTheta)<0.06f*degToRad_f) // 0.06 degrees
                    {
                        couldReach=true;
                        break;
                    }
                    else
                        break;
                }
            }
            else
                break;
        }
        else
            break;
//      break; // REMOVED 2008.09.03 BECAUSE IT CAN'T FIND A PATH
        tmpL=currentL;
        currentL=nextL;
        nextL=tmpL;
        tmpHandle=currentHandle;
        currentHandle=nextHandle;
        nextHandle=tmpHandle;
        dir*=-1.0f;
    }
    if (test)
    {
        if (couldReach)
            return((CNonHolonomicPathNode*)1); // Just anything different from nullptr!!
        else
            return(nullptr);
    }
    for (int i=0;i<int(fromStartP.size());i++)
        currentList->push_back(fromStartP[i]);
    if (connect)
    {
        if (couldReach)
        {
            for (int i=0;i<int(fromGoalP.size());i++)
            {
                CNonHolonomicPathNode* it=fromGoalP[fromGoalP.size()-i-1];
                it->parent=lastHandleFromStart;
                currentList->push_back(it);
                lastHandleFromStart=it;
            }
            return(lastHandleFromStart);
        }
        else
        {
            for (int i=0;i<int(fromGoalP.size());i++)
                nextList->push_back(fromGoalP[i]);
            return(nullptr);
        }
    }
    else
    {
        if (couldReach)
        {
            for (int i=0;i<int(fromGoalP.size());i++)
            {
                CNonHolonomicPathNode* it=fromGoalP[fromGoalP.size()-i-1];
                it->parent=lastHandleFromStart;
                currentList->push_back(it);
                lastHandleFromStart=it;
            }
            return(lastHandleFromStart);
        }
        else
        {
            for (int i=0;i<int(fromGoalP.size());i++)
                delete fromGoalP[i];
            return(lastHandleFromStart);
        }
    }
}


int CNonHolonomicPathPlanning::smoothFoundPath(int steps,int maxTimePerPass)
{ // step specifies the number of passes (each pass is composed by a calculated sub-pass, and some random sub-pass)
    // We first copy foundPath:
    if (steps<2)
        return(1);
    if (invalidData)
        return(0);
    CDummyDummy* startDummy=(CDummyDummy*)_simGetObject_internal(_startDummyID);
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
        CNonHolonomicPathNode* startP;
        CNonHolonomicPathNode* endP;
        for (int randomPass=0;randomPass<5;randomPass++)
        { // If randomPass==0, the pass is not random, i.e. the low and high indices are calculated
            startP=nullptr; // since 2010/09/09
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
                if (highIndex>=int(foundPath.size())) // probably not needed
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
                std::vector<CNonHolonomicPathNode*> newPathElementsBetweenAndIncludingLowAndHigh;
                newPathElementsBetweenAndIncludingLowAndHigh.push_back(new CNonHolonomicPathNode(startP));
                C7Vector startDummyOriginalLocalTr;
                _simGetObjectLocalTransformation_internal(startDummy,startDummyOriginalLocalTr.X.data,startDummyOriginalLocalTr.Q.data,true); // save the local transformation ("connect" modifies it)
                if (connect(&newPathElementsBetweenAndIncludingLowAndHigh,nullptr,startP,endP,true,true,true,startDummy)!=nullptr)
                { // The path can be performed!
                    connect(&newPathElementsBetweenAndIncludingLowAndHigh,nullptr,startP,endP,true,true,false,startDummy);
                    int elementsBefore=highIndex-lowIndex+1;
                    int elementsAfter=(int)newPathElementsBetweenAndIncludingLowAndHigh.size();
                    if (elementsAfter<elementsBefore)
                    { // The new path section is shorter. We replace current path section with this one:
                        for (int i=lowIndex;i<=highIndex;i++)
                            delete foundPath[i];
                        foundPath.erase(foundPath.begin()+lowIndex,foundPath.begin()+highIndex+1);
                        foundPathSameStraightLineID_forSteppedSmoothing.erase(foundPathSameStraightLineID_forSteppedSmoothing.begin()+lowIndex,foundPathSameStraightLineID_forSteppedSmoothing.begin()+highIndex+1);

                        foundPath.insert(foundPath.begin()+lowIndex,newPathElementsBetweenAndIncludingLowAndHigh.begin(),newPathElementsBetweenAndIncludingLowAndHigh.end());
                        foundPathSameStraightLineID_forSteppedSmoothing.insert(foundPathSameStraightLineID_forSteppedSmoothing.begin()+lowIndex,elementsAfter,sameStraightLineNextID_forSteppedSmoothing);
                        sameStraightLineNextID_forSteppedSmoothing++;
                    }
                    else
                    { // The new path section is longer or same! We don't do anything
                        for (int i=0;i<int(newPathElementsBetweenAndIncludingLowAndHigh.size());i++)
                            delete newPathElementsBetweenAndIncludingLowAndHigh[i];
                    }
                }
                else
                    delete newPathElementsBetweenAndIncludingLowAndHigh[0]; // Impossible to connect, we have to erase the only element in the list
                _simSetObjectLocalTransformation_internal(startDummy,startDummyOriginalLocalTr.X.data,startDummyOriginalLocalTr.Q.data); // Restore the original transformation
            }
        }
    }
    return(0); // will never pass here!
}


void CNonHolonomicPathPlanning::getPathData(std::vector<float>& data)
{
    data.clear();
    if (invalidData)
        return;
    for (int i=0;i<int(foundPath.size());i++)
    {
        CNonHolonomicPathNode* theNode=foundPath[i];
        C3Vector p,euler;
        p.clear();
        euler.clear();
        p(0)=theNode->values[0];
        p(1)=theNode->values[1];
        euler(2)=theNode->values[2];
        C7Vector conf(C4Vector(euler),p);
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

bool CNonHolonomicPathPlanning::doCollide(float* dist)
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
