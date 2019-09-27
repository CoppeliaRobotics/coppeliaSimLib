
#include "vrepMainHeader.h"
#include "motionPlanningTask_old.h"
#include "pathPlanningInterface.h"
#include "global.h"
#include "app.h"
#include "tt.h"
#include "motionPlanningTaskRendering_old.h"

int CMotionPlanningTask::randomSeed=-1;

CMotionPlanningTask::CMotionPlanningTask(const std::vector<int>& jointHandles)
{
    setDefaultValues();

    // Following not really necessary, but will look cleaner and more "logic" when ordered from base to tip:
    std::vector<int> jointH;
    std::vector<int> parentCnt;
    for (int i=0;i<int(jointHandles.size());i++)
    {
        jointH.push_back(jointHandles[i]);
        C3DObject* obj=App::ct->objCont->getObjectFromHandle(jointHandles[i]);
        int cnt=-1;
        while (obj!=nullptr)
        {
            cnt++;
            obj=obj->getParentObject();
        }
        parentCnt.push_back(cnt);
    }
    tt::orderAscending(parentCnt,jointH);

    for (int i=0;i<int(jointH.size());i++)
    {
        _jointHandles.push_back(jointH[i]);
        _jointStepCount.push_back(6); // 7 DoF manipulator with 8 --> 2'097'152 nodes, with 7 --> 823'543, with 6 --> 279'936
        _robotMetric.push_back(1.0f);
    }
    while (calculateResultingNodeCount()==-1)
    { // we have too many nodes!
        for (int i=0;i<int(_jointStepCount.size());i++)
            _jointStepCount[i]--;
    }
}

CMotionPlanningTask::CMotionPlanningTask()
{ // serialization constructor
    _data=nullptr;
    _nodesSerializationData=nullptr;
}

CMotionPlanningTask::~CMotionPlanningTask()
{
    clearDataStructure();
}

void CMotionPlanningTask::setDefaultValues()
{
    _objectID=SIM_IDSTART_MOTIONPLANNINGTASK;
    _objectName="MotionPlanningTask";
    _ikGroupId=-1;
    _showPhase1Nodes=false;
    _data=nullptr;
    _robotEntity=-1;
    _obstacleEntity=-1;
    _robotSelfCollEntity1=-1;
    _robotSelfCollEntity2=-1;
    _tipMetric[0]=1.0f;
    _tipMetric[1]=1.0f;
    _tipMetric[2]=1.0f;
    _tipMetric[3]=90.0f*3.141592653589f/180.0f;
    _nodesSerializationData=nullptr;
    _nodesSerializationDataSize=0;

    _showPhase2FromStartConnections=false;
    _showPhase2FromGoalConnections=false;
    _showPhase2Path=false;


    _distanceThreshold=0.0f; //0.0f means we do collision detection, instead of distance calculation
    _selfCollDistanceThreshold=0.0f; //0.0f means we do collision detection, instead of distance calculation
}

CMotionPlanningTask* CMotionPlanningTask::copyYourself()
{
    CMotionPlanningTask* newTask=new CMotionPlanningTask(_jointHandles);
    newTask->_objectID=_objectID;
    newTask->_objectName=_objectName;
    newTask->_ikGroupId=_ikGroupId;
    newTask->_robotEntity=_robotEntity;
    newTask->_obstacleEntity=_obstacleEntity;
    newTask->_distanceThreshold=_distanceThreshold;
    newTask->_robotSelfCollEntity1=_robotSelfCollEntity1;
    newTask->_robotSelfCollEntity2=_robotSelfCollEntity2;
    newTask->_selfCollDistanceThreshold=_selfCollDistanceThreshold;
    newTask->_showPhase1Nodes=_showPhase1Nodes;
    for (int i=0;i<4;i++)
        newTask->_tipMetric[i]=_tipMetric[i];
    for (int i=0;i<int(_jointStepCount.size());i++)
    {
        newTask->_jointStepCount[i]=_jointStepCount[i];
        newTask->_robotMetric[i]=_robotMetric[i];
    }

    if (_nodesSerializationData!=nullptr)
    {
        newTask->_nodesSerializationData=new char[_nodesSerializationDataSize];
        newTask->_nodesSerializationDataSize=_nodesSerializationDataSize;
        for (int i=0;i<_nodesSerializationDataSize;i++)
            newTask->_nodesSerializationData[i]=_nodesSerializationData[i];
    }
    else
    { // we do not copy _data since that takes time and memory. We'll compute it on-the-fly when needed! But we copy the collision states (i.e. the robot self collision states)
        //if (_data!=nullptr)
        //{
        //  newTask->_data=_data->copyYourself(int(_jointHandles.size()));
        //  newTask->_data->setTempData(_jointHandles,_jointStepCount,_ikGroupId,_baseFrameId,_tipFrameId,_robotEntity,_obstacleEntity,_distanceThreshold,_tipMetric,_robotMetric);
        //}

        if (_data!=nullptr)
        {
            int cnt;
//          char* dat=_data->getSerializationData(&cnt);
            char* dat=CPathPlanningInterface::getMpSerializationData(_data,&cnt);
            newTask->_nodesSerializationData=new char[cnt];
            newTask->_nodesSerializationDataSize=cnt;
            for (int i=0;i<cnt;i++)
                newTask->_nodesSerializationData[i]=dat[i];
            CPathPlanningInterface::releaseBuffer(dat);         
        }
    }

    return(newTask);
}

void CMotionPlanningTask::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    //_initialValuesInitialized=simulationIsRunning;
    //if (simulationIsRunning)
    //{
    //}
}

void CMotionPlanningTask::simulationAboutToStart()
{
    initializeInitialValues(true);
}

void CMotionPlanningTask::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    //if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
    //{
    //}
    //_initialValuesInitialized=false;
}

void CMotionPlanningTask::serialize(CSer& ar)
{
    if (ar.isStoring())
    {       // Storing
        ar.storeDataName("Nme");
        ar << _objectName;
        ar.flush();

        ar.storeDataName("Oid");
        ar << _objectID;
        ar.flush();

        ar.storeDataName("Jhn");
        ar << int(_jointHandles.size());
        for (int i=0;i<int(_jointHandles.size());i++)
            ar << _jointHandles[i];
        ar.flush();

        ar.storeDataName("Jsc");
        ar << int(_jointStepCount.size());
        for (int i=0;i<int(_jointStepCount.size());i++)
            ar << _jointStepCount[i];
        ar.flush();

        ar.storeDataName("Ikg");
        ar << _ikGroupId;
        ar.flush();

        ar.storeDataName("Ety");
        ar << _robotEntity << _obstacleEntity << _robotSelfCollEntity1 << _robotSelfCollEntity2;
        ar.flush();

        ar.storeDataName("Cdt");
        ar << _distanceThreshold << _selfCollDistanceThreshold;
        ar.flush();

        ar.storeDataName("Tmc");
        ar << _tipMetric[0] << _tipMetric[1] << _tipMetric[2] << _tipMetric[3];
        ar.flush();

        ar.storeDataName("Jmc");
        ar << int(_robotMetric.size());
        for (int i=0;i<int(_robotMetric.size());i++)
            ar << _robotMetric[i];
        ar.flush();

        ar.storeDataName("Var");
        unsigned char dummy=0;
        SIM_SET_CLEAR_BIT(dummy,0,_showPhase1Nodes);
        //SIM_SET_CLEAR_BIT(dummy,1,xxxx);
        ar << dummy;
        ar.flush();

        if (_data!=nullptr)
        { // we just save the "colliding state" for each node. Saving all the node info would be way too much (we'll recompute that when needed, on-the-fly)
            ar.storeDataName("Dat");
            int cnt;
            char* dat=CPathPlanningInterface::getMpSerializationData(_data,&cnt);
//          char* dat=_data->getSerializationData(&cnt);
            for (int i=0;i<cnt;i++)
                ar << dat[i];
            CPathPlanningInterface::releaseBuffer(dat);
            ar.flush();
        }

        ar.storeDataName(SER_END_OF_OBJECT);
    }
    else
    {       // Loading
        int byteQuantity;
        std::string theName="";
        while (theName.compare(SER_END_OF_OBJECT)!=0)
        {
            theName=ar.readDataName();
            if (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                bool noHit=true;
                if (theName.compare("Nme")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> _objectName;
                }
                if (theName.compare("Oid")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> _objectID;
                }
                if (theName.compare("Jhn")==0)
                {
                    noHit=false;
                    int cnt,v;
                    ar >> byteQuantity;
                    ar >> cnt;
                    _jointHandles.clear();
                    for (int i=0;i<cnt;i++)
                    {
                        ar >> v;
                        _jointHandles.push_back(v);
                    }
                }
                if (theName.compare("Jsc")==0)
                {
                    noHit=false;
                    int cnt,v;
                    ar >> byteQuantity;
                    ar >> cnt;
                    _jointStepCount.clear();
                    for (int i=0;i<cnt;i++)
                    {
                        ar >> v;
                        _jointStepCount.push_back(v);
                    }
                }
                if (theName.compare("Ikg")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> _ikGroupId;
                }
                if (theName.compare("Ety")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> _robotEntity >> _obstacleEntity >> _robotSelfCollEntity1 >> _robotSelfCollEntity2;
                }
                if (theName.compare("Cdt")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> _distanceThreshold >> _selfCollDistanceThreshold;
                }
                if (theName.compare("Tmc")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> _tipMetric[0] >> _tipMetric[1] >> _tipMetric[2] >> _tipMetric[3];
                }
                if (theName.compare("Jmc")==0)
                {
                    noHit=false;
                    int cnt;
                    float v;
                    ar >> byteQuantity;
                    ar >> cnt;
                    _robotMetric.clear();
                    for (int i=0;i<cnt;i++)
                    {
                        ar >> v;
                        _robotMetric.push_back(v);
                    }
                }

                if (theName.compare("Var")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    unsigned char dummy;
                    ar >> dummy;
                    _showPhase1Nodes=SIM_IS_BIT_SET(dummy,0);
                //  xxxxx=SIM_IS_BIT_SET(dummy,1);
                }

                if (theName.compare("Dat")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    _nodesSerializationData=new char[byteQuantity];
                    for (int i=0;i<byteQuantity;i++)
                        ar >> _nodesSerializationData[i];
                    _nodesSerializationDataSize=byteQuantity;
                }
                if (noHit)
                    ar.loadUnknownData();
            }
        }
    }
}

void CMotionPlanningTask::performObjectLoadingMapping(std::vector<int>* map)
{
    for (int i=0;i<int(_jointHandles.size());i++)
        _jointHandles[i]=App::ct->objCont->getLoadingMapping(map,_jointHandles[i]);
    if (_robotEntity<SIM_IDSTART_COLLECTION)
        _robotEntity=App::ct->objCont->getLoadingMapping(map,_robotEntity);
    if (_obstacleEntity<SIM_IDSTART_COLLECTION)
        _obstacleEntity=App::ct->objCont->getLoadingMapping(map,_obstacleEntity);
    if (_robotSelfCollEntity1<SIM_IDSTART_COLLECTION)
        _robotSelfCollEntity1=App::ct->objCont->getLoadingMapping(map,_robotSelfCollEntity1);
    if (_robotSelfCollEntity2<SIM_IDSTART_COLLECTION)
        _robotSelfCollEntity2=App::ct->objCont->getLoadingMapping(map,_robotSelfCollEntity2);
}
void CMotionPlanningTask::performCollectionLoadingMapping(std::vector<int>* map)
{
    if (_robotEntity>=SIM_IDSTART_COLLECTION)
        _robotEntity=App::ct->objCont->getLoadingMapping(map,_robotEntity);
    if (_obstacleEntity>=SIM_IDSTART_COLLECTION)
        _obstacleEntity=App::ct->objCont->getLoadingMapping(map,_obstacleEntity);
    if (_robotSelfCollEntity1>=SIM_IDSTART_COLLECTION)
        _robotSelfCollEntity1=App::ct->objCont->getLoadingMapping(map,_robotSelfCollEntity1);
    if (_robotSelfCollEntity2>=SIM_IDSTART_COLLECTION)
        _robotSelfCollEntity2=App::ct->objCont->getLoadingMapping(map,_robotSelfCollEntity2);
}
void CMotionPlanningTask::performIkGroupLoadingMapping(std::vector<int>* map)
{
    _ikGroupId=App::ct->objCont->getLoadingMapping(map,_ikGroupId);
}

bool CMotionPlanningTask::announceObjectWillBeErased(int objID,bool copyBuffer)
{ // Return value true means that this object should be destroyed
    for (int i=0;i<int(_jointHandles.size());i++)
    {
        if (_jointHandles[i]==objID)
            return(true);
    }
    if (_robotEntity==objID)
        _robotEntity=-1;
    if (_obstacleEntity==objID)
        _obstacleEntity=-1;
    if (_robotSelfCollEntity1==objID)
    {
        _robotSelfCollEntity1=-1;
        clearDataStructure(); // needs to be recomputed
    }
    if (_robotSelfCollEntity2==objID)
    {
        _robotSelfCollEntity2=-1;
        clearDataStructure(); // needs to be recomputed
    }
    return(false);
}
bool CMotionPlanningTask::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{ // Return value true means that this object should be destroyed
    if (_robotEntity==groupID)
        _robotEntity=-1;
    if (_obstacleEntity==groupID)
        _obstacleEntity=-1;
    if (_robotSelfCollEntity1==groupID)
    {
        _robotSelfCollEntity1=-1;
        clearDataStructure(); // needs to be recomputed
    }
    if (_robotSelfCollEntity2==groupID)
    {
        _robotSelfCollEntity2=-1;
        clearDataStructure(); // needs to be recomputed
    }
    return(false);
}
bool CMotionPlanningTask::announceIkGroupWillBeErased(int ikGroupID,bool copyBuffer)
{ // Return value true means that this object should be destroyed
    if (_ikGroupId==ikGroupID)
        return(true);
    return(false);
}

int CMotionPlanningTask::getObjectID()
{
    return(_objectID);
}
void CMotionPlanningTask::setObjectID(int newID)
{
    _objectID=newID;
}
std::string CMotionPlanningTask::getObjectName()
{
    return(_objectName);
}
void CMotionPlanningTask::setObjectName(std::string newName)
{
    _objectName=newName;
}

void CMotionPlanningTask::getJointHandles(std::vector<int>& handles)
{
    handles.assign(_jointHandles.begin(),_jointHandles.end());
}

int CMotionPlanningTask::getJointCount()
{
    return(int(_jointHandles.size()));
}

int CMotionPlanningTask::getIkGroup()
{
    return(_ikGroupId);
}

void CMotionPlanningTask::setIkGroup(int ikGroupId)
{
    if (_ikGroupId!=ikGroupId)
        clearDataStructure(); // needs to be recomputed
    _ikGroupId=ikGroupId;
}

int CMotionPlanningTask::getJointStepCount(int jointHandle)
{
    for (int i=0;i<int(_jointHandles.size());i++)
    {
        if (_jointHandles[i]==jointHandle)
            return(_jointStepCount[i]);
    }
    return(0);
}

void CMotionPlanningTask::setJointStepCount(int jointHandle,int stepCount)
{
    bool modified=false;
    for (int i=0;i<int(_jointHandles.size());i++)
    {
        if (_jointHandles[i]==jointHandle)
        {
            int saved=_jointStepCount[i];
            _jointStepCount[i]=tt::getLimitedInt(3,50,stepCount);
            if (calculateResultingNodeCount()==-1)
                _jointStepCount[i]=saved; // we have too many nodes!
            modified=(saved!=_jointStepCount[i]);
            break;
        }
    }
    if (modified)
        clearDataStructure(); // needs to be recomputed
}

int CMotionPlanningTask::calculateResultingNodeCount()
{
    unsigned int retV=1;
    for (int i=0;i<int(_jointStepCount.size());i++)
    {
        retV*=(unsigned int)_jointStepCount[i];
        if (retV>100000000)
            return(-1); // means: too many!
    }
    return(int(retV));
}

void CMotionPlanningTask::clearDataStructure()
{
    if (_data!=nullptr)
        CPathPlanningInterface::destroyMpObject(_data);
//  delete _data;
    _data=nullptr;
    delete _nodesSerializationData;
    _nodesSerializationData=nullptr;
    _nodesSerializationDataSize=0;
}

float CMotionPlanningTask::getRobotMetric(int jointHandle)
{
    for (int i=0;i<int(_jointHandles.size());i++)
    {
        if (_jointHandles[i]==jointHandle)
            return(_robotMetric[i]);
    }
    return(0.0f);
}

void CMotionPlanningTask::setRobotMetric(int jointHandle,float w)
{ // no need to call clearDataStructure here
    for (int i=0;i<int(_jointHandles.size());i++)
    {
        if (_jointHandles[i]==jointHandle)
        {
            _robotMetric[i]=tt::getLimitedFloat(0.0f,1000.0f,w);
            break;
        }
    }
}

float CMotionPlanningTask::getTipMetric(int index)
{
    return(_tipMetric[index]);
}

void CMotionPlanningTask::setTipMetric(int index,float w)
{ // no need to call clearDataStructure here
    _tipMetric[index]=tt::getLimitedFloat(0.0f,1000.0f,w);
}

C3DObject* CMotionPlanningTask::getBaseObject()
{
    CikGroup* ik=getIkGroupObject();
    if (ik==nullptr)
        return(nullptr);
    if (ik->ikElements.size()!=1)
        return(nullptr);
    int b=ik->ikElements[0]->getAlternativeBaseForConstraints();
    if (b==-1)
        b=ik->ikElements[0]->getBase();
    C3DObject* retVal=App::ct->objCont->getObjectFromHandle(b);
    return(retVal);
}

C3DObject* CMotionPlanningTask::getTipObject()
{
    CikGroup* ik=getIkGroupObject();
    if (ik==nullptr)
        return(nullptr);
    if (ik->ikElements.size()!=1)
        return(nullptr);
    C3DObject* retVal=App::ct->objCont->getObjectFromHandle(ik->ikElements[0]->getTooltip());
    return(retVal);
}

C3DObject* CMotionPlanningTask::getTargetObject()
{
    CikGroup* ik=getIkGroupObject();
    if (ik==nullptr)
        return(nullptr);
    if (ik->ikElements.size()!=1)
        return(nullptr);
    C3DObject* retVal=App::ct->objCont->getObjectFromHandle(ik->ikElements[0]->getTarget());
    return(retVal);
}

CikGroup* CMotionPlanningTask::getIkGroupObject()
{
    CikGroup* retVal=App::ct->ikGroups->getIkGroup(_ikGroupId);
    return(retVal);
}

void CMotionPlanningTask::clearPhase2NodeVisualizationAndPathVisualization()
{
    _showPhase2FromStartConnections=false;
    _showPhase2FromGoalConnections=false;
    _showPhase2Path=false;
}

bool CMotionPlanningTask::getPhase1NodeAreReady()
{
    return(_data!=nullptr);
}

bool CMotionPlanningTask::calculateDataStructureIfNeeded(int* collidingNodeCnt,int* totalNodeCnt,bool outputActivityToConsole)
{
    if (collidingNodeCnt!=nullptr)
        collidingNodeCnt[0]=0;
    if (totalNodeCnt!=nullptr)
        totalNodeCnt[0]=0;
    bool existing=false;
    if (_data==nullptr)
        _data=CPathPlanningInterface::createMpObject();
    else
        existing=true;
    C3DObject* basef=getBaseObject();
    C3DObject* tipf=getTipObject();
    C3DObject* targetF=getTargetObject();
    CikGroup* ik=getIkGroupObject();
    if ((ik==nullptr)||(basef==nullptr)||(tipf==nullptr)||(targetF==nullptr))
    {
        CPathPlanningInterface::destroyMpObject(_data);
        _data=nullptr;
        if (outputActivityToConsole)
            printf("Ill-defined motion planning task!\n");
        return(false);
    }

    CikEl* el=ik->ikElements[0]; // normally there should be exactly one ik element!
    float tipMetricAndIkElConstraints[5];
    tipMetricAndIkElConstraints[0]=_tipMetric[0];
    tipMetricAndIkElConstraints[1]=_tipMetric[1];
    tipMetricAndIkElConstraints[2]=_tipMetric[2];
    tipMetricAndIkElConstraints[3]=_tipMetric[3];
    tipMetricAndIkElConstraints[4]=el->getConstraints();

    CPathPlanningInterface::setMpObjectData(_data,(int)_jointHandles.size(),&_jointHandles[0],&_jointStepCount[0],_ikGroupId,basef->getObjectHandle(),tipf->getObjectHandle(),_robotSelfCollEntity1,_robotSelfCollEntity2,_robotEntity,_obstacleEntity,_distanceThreshold,tipMetricAndIkElConstraints,&_robotMetric[0]);
    if (existing)
    {
        if (collidingNodeCnt!=nullptr)
            collidingNodeCnt[0]=CPathPlanningInterface::getMpPhase1NodeCnt(_data,1);
        if (totalNodeCnt!=nullptr)
            totalNodeCnt[0]=CPathPlanningInterface::getMpPhase1NodeCnt(_data,0);
        if (outputActivityToConsole)
            printf("Using the already existing motion planning data structure.\n");
        return(true); // data structure was already existing
    }
    // Save joint positions/modes (all of them, just in case):
    std::vector<CJoint*> sceneJoints;
    std::vector<float> initSceneJointValues;
    std::vector<int> initSceneJointModes;
    _saveAllSceneJointPositionsAndModes(sceneJoints,initSceneJointValues,initSceneJointModes);

    // Calculate the nodes and their robot-self-collision status:
    if (outputActivityToConsole)
        printf("Preparing the motion planning data structure...\n");
    bool retVal=(CPathPlanningInterface::calculateMpNodesPhase1(_data,_nodesSerializationData,_nodesSerializationDataSize)!=0);
    delete[] _nodesSerializationData;
    _nodesSerializationData=nullptr;
    _nodesSerializationDataSize=0;

    // Restore joint positions/modes:
    _restoreAllSceneJointPositionsAndModes(sceneJoints,initSceneJointValues,initSceneJointModes);

    if (retVal)
    {
        if (collidingNodeCnt!=nullptr)
            collidingNodeCnt[0]=CPathPlanningInterface::getMpPhase1NodeCnt(_data,1);
        if (totalNodeCnt!=nullptr)
            totalNodeCnt[0]=CPathPlanningInterface::getMpPhase1NodeCnt(_data,0);
        if (outputActivityToConsole)
            printf("Finished preparing the motion planning data structure. Total nodes: %i\n",CPathPlanningInterface::getMpPhase1NodeCnt(_data,0));
        return(true);
    }
    if (outputActivityToConsole)
        printf("Failed at preparing the motion planning data structure!\n");
    clearDataStructure();
    return(false);
}

void CMotionPlanningTask::setRobotEntity(int entityID)
{
    _robotEntity=entityID;
    if (_robotEntity==-1)
        _obstacleEntity=-1;
}

int CMotionPlanningTask::getRobotEntity()
{
    return(_robotEntity);
}

void CMotionPlanningTask::setObstacleEntity(int entityID)
{
    _obstacleEntity=entityID;
}

int CMotionPlanningTask::getObstacleEntity()
{
    return(_obstacleEntity);
}

void CMotionPlanningTask::setRobotSelfCollEntity1(int entityID)
{
    if (_robotSelfCollEntity1!=entityID)
        clearDataStructure(); // needs to be recomputed
    _robotSelfCollEntity1=entityID;
    if (_robotSelfCollEntity1==-1)
        _robotSelfCollEntity2=-1;
}

int CMotionPlanningTask::getRobotSelfCollEntity1()
{
    return(_robotSelfCollEntity1);
}

void CMotionPlanningTask::setRobotSelfCollEntity2(int entityID)
{
    if (_robotSelfCollEntity2!=entityID)
        clearDataStructure(); // needs to be recomputed
    _robotSelfCollEntity2=entityID;
}

int CMotionPlanningTask::getRobotSelfCollEntity2()
{
    return(_robotSelfCollEntity2);
}

void CMotionPlanningTask::setDistanceThreshold(float d)
{
    _distanceThreshold=tt::getLimitedFloat(0.0f,10000000000.0f,d);
}

float CMotionPlanningTask::getDistanceThreshold()
{
    return(_distanceThreshold);
}

void CMotionPlanningTask::setSelfCollDistanceThreshold(float d)
{
    d=tt::getLimitedFloat(0.0f,10000000000.0f,d);
    if (_selfCollDistanceThreshold!=d)
        clearDataStructure(); // needs to be recomputed
    _selfCollDistanceThreshold=d;
}

float CMotionPlanningTask::getSelfCollDistanceThreshold()
{
    return(_selfCollDistanceThreshold);
}

void CMotionPlanningTask::setPhase1NodeVisualization(bool show)
{
    _showPhase1Nodes=show;
}

bool CMotionPlanningTask::getPhase1NodeVisualization()
{
    return(_showPhase1Nodes);
}

void CMotionPlanningTask::getCurrentJointPositions(std::vector<float>& jointPositions)
{
    jointPositions.clear();
    for (int i=0;i<int(_jointHandles.size());i++)
    {
        CJoint* joint=App::ct->objCont->getJoint(_jointHandles[i]);
        if (joint!=nullptr)
            jointPositions.push_back(joint->getPosition());
        else
            jointPositions.push_back(0.0f);
    }
}


int CMotionPlanningTask::getRobotConfigFromTipPose(const C7Vector& tipPose,int options,float* robotJoints,int trialCount,float tipCloseDistance,const float* configConstraints,const float* referenceConfigs,int refConfigCount,const int* jointBehaviour,int correctionPasses,int maxTimeInMs)
{ // returns 0 if it failed, otherwise the number of trials before success
    // tipPose needs to be in absolute coords!

    bool activityToConsole=((options&8)!=0);

    calculateDataStructureIfNeeded(nullptr,nullptr,activityToConsole);
    if (_data!=nullptr)
    {
        float constraints[5]={0.0f,0.0f,0.0f,0.0f,0.0f};
        CikGroup* ikGroup=App::ct->ikGroups->getIkGroup(_ikGroupId);
        if ((ikGroup!=nullptr)&&ikGroup->ikElements.size()!=0)
        {
            CikEl* el=ikGroup->ikElements[0]; // normally there should be exactly one ik element!
            int constr=el->getConstraints();
            if (constr&sim_ik_x_constraint)
                constraints[0]=1.0f;
            if (constr&sim_ik_y_constraint)
                constraints[1]=1.0f;
            if (constr&sim_ik_z_constraint)
                constraints[2]=1.0f;
            if (constr&sim_ik_alpha_beta_constraint)
                constraints[3]=1.0f;
            if (constr&sim_ik_gamma_constraint)
                constraints[4]=1.0f;
            // Save joint positions/modes (all of them, just in case)
            std::vector<CJoint*> sceneJoints;
            std::vector<float> initSceneJointValues;
            std::vector<int> initSceneJointModes;
            _saveAllSceneJointPositionsAndModes(sceneJoints,initSceneJointValues,initSceneJointModes);
            // Set the correct mode for the joints involved:
            for (int i=0;i<int(_jointHandles.size());i++)
            {
                CJoint* joint=App::ct->objCont->getJoint(_jointHandles[i]);
                if (joint!=nullptr)
                {
                    if (joint->getJointMode()!=sim_jointmode_ik)
                        joint->setJointMode(sim_jointmode_ik,false);
                }
            }
            // do the calculation:
            C7Vector tipPoseRelativeToBase=tipPose;
            C3DObject* base=getBaseObject();
            if (base!=nullptr)
                tipPoseRelativeToBase=base->getCumulativeTransformationPart1().getInverse()*tipPose;

//          int retVal=_data->getRobotConfigFromTipPose(tipPoseRelativeToBase.X.data,tipPoseRelativeToBase.Q.data,options,robotJoints,constraints,configConstraints,trialCount,tipCloseDistance,referenceConfigs,refConfigCount,jointBehaviour,correctionPasses,maxTimeInMs);
            int retVal=CPathPlanningInterface::getMpRobotConfigFromTipPose(_data,tipPoseRelativeToBase.X.data,tipPoseRelativeToBase.Q.data,options,robotJoints,constraints,configConstraints,trialCount,tipCloseDistance,referenceConfigs,refConfigCount,jointBehaviour,correctionPasses,maxTimeInMs);
            // Restore joint positions/modes:
            _restoreAllSceneJointPositionsAndModes(sceneJoints,initSceneJointValues,initSceneJointModes);
            return(retVal);
        }
        else
        {
            if (activityToConsole)
                printf("Motion planning task is ill-defined (ik group)!\n");
        }
    }
    return(0);
}

void CMotionPlanningTask::_saveAllSceneJointPositionsAndModes(std::vector<CJoint*>& joints,std::vector<float>& jointPositions,std::vector<int>& jointModes,std::vector<float>* jointIkResolutionWeights)
{ // jointIkResolutionWeights is nullptr by default
    joints.clear();
    jointPositions.clear();
    jointModes.clear();
    for (int i=0;i<int(App::ct->objCont->jointList.size());i++)
    {
        CJoint* aj=App::ct->objCont->getJoint(App::ct->objCont->jointList[i]);
        joints.push_back(aj);
        jointPositions.push_back(aj->getPosition());
        jointModes.push_back(aj->getJointMode());
        if (jointIkResolutionWeights!=nullptr)
            jointIkResolutionWeights->push_back(aj->getIKWeight());
    }

}

void CMotionPlanningTask::_restoreAllSceneJointPositionsAndModes(const std::vector<CJoint*>& joints,const std::vector<float>& jointPositions,const std::vector<int>& jointModes,const std::vector<float>* jointIkResolutionWeights)
{ // jointIkResolutionWeights is nullptr by default
    for (int i=0;i<int(joints.size());i++)
    {
        if (joints[i]->getPosition()!=jointPositions[i])
            joints[i]->setPosition(jointPositions[i]);
        if (joints[i]->getJointMode()!=jointModes[i])
            joints[i]->setJointMode(jointModes[i],false);
        if (jointIkResolutionWeights!=nullptr)
            joints[i]->setIKWeight(jointIkResolutionWeights->at(i));
    }
}

float* CMotionPlanningTask::getConfigTransition(const float* startConfig,const float* goalConfig,int options,const int* select,float calcStepSize,float maxOutStepSize,int wayPointCnt,const float* wayPoints,int* outputConfigsCnt,const int* auxIntParams,const float* auxFloatParams)
{
//  _showPhase2FromStartConnections=((options&1)!=0);
//  _showPhase2FromGoalConnections=((options&2)!=0);
    _showPhase2Path=((options&4)!=0);
    bool activityToConsole=((options&8)!=0);
    outputConfigsCnt[0]=0;
    calculateDataStructureIfNeeded(nullptr,nullptr,activityToConsole); // we need the data structure, not the node.... we calculate the node here anyway

    float* retVal=nullptr;

    if (_data!=nullptr)
    {
        // selection thing:
        std::vector<int> selectHandles;
        std::vector<int> selectModes;
        if (select!=nullptr)
        {
            for (int i=0;i<select[0];i++)
            {
                selectHandles.push_back(select[1+2*i+0]);
                selectModes.push_back(select[1+2*i+1]);
            }
        }

        // Save joint positions/modes (all of them, just in case)
        std::vector<CJoint*> sceneJoints;
        std::vector<float> initSceneJointValues;
        std::vector<int> initSceneJointModes;
        std::vector<float> initSceneJointIkResolutionWeights;
        _saveAllSceneJointPositionsAndModes(sceneJoints,initSceneJointValues,initSceneJointModes,&initSceneJointIkResolutionWeights);

        // Set the correct mode for the joints involved:
        for (int i=0;i<int(_jointHandles.size());i++)
        {
            CJoint* joint=App::ct->objCont->getJoint(_jointHandles[i]);
            if (joint!=nullptr)
            {
                if (joint->getJointMode()!=sim_jointmode_ik)
                {
                    bool doIt=true;
                    for (int j=0;j<int(selectHandles.size());j++)
                    {
                        if (selectHandles[j]==joint->getObjectHandle())
                        {
                            if (selectModes[j]==0)
                            {
                                doIt=false;
                                joint->setJointMode(sim_jointmode_passive);
                            }
                            if (selectModes[j]==1)
                                joint->setIKWeight(0.5f);
                            if (selectModes[j]==2)
                                joint->setIKWeight(0.25f);
                            if (selectModes[j]==3)
                                joint->setIKWeight(0.1f);
                            if (selectModes[j]==4)
                                joint->setIKWeight(0.05f);
                            if (selectModes[j]==5)
                                joint->setIKWeight(0.025f);
                            if (selectModes[j]==6)
                                joint->setIKWeight(0.01f);
                            if (selectModes[j]==7)
                                joint->setIKWeight(0.001f);
                        }
                    }
                    if (doIt)
                        joint->setJointMode(sim_jointmode_ik);
                }
            }
        }

        // do the calculation:
        float* pathData=CPathPlanningInterface::mpGetConfigTransition(_data,startConfig,goalConfig,options,select,calcStepSize,maxOutStepSize,wayPointCnt,wayPoints,outputConfigsCnt,auxIntParams,auxFloatParams);

        if (pathData!=nullptr)
        {
            int cnt=outputConfigsCnt[0]*(int)_jointHandles.size()+outputConfigsCnt[0]+outputConfigsCnt[0]*3+outputConfigsCnt[0]*4+outputConfigsCnt[0];
            retVal=(float*)new char[cnt*4];
            for (int i=0;i<cnt;i++)
                retVal[i]=pathData[i];
            CPathPlanningInterface::releaseBuffer(pathData);
        }

        // Restore joint positions/modes:
        _restoreAllSceneJointPositionsAndModes(sceneJoints,initSceneJointValues,initSceneJointModes,&initSceneJointIkResolutionWeights);
    }

    return(retVal);
}


float* CMotionPlanningTask::findPath(const float* startConfig,const float* goalConfig,int options,float stepSize,int* outputConfigsCnt,int maxTimeInMs,const int* auxIntParams,const float* auxFloatParams)
{
    _showPhase2FromStartConnections=((options&1)!=0);
    _showPhase2FromGoalConnections=((options&2)!=0);
    _showPhase2Path=((options&4)!=0);
    bool activityToConsole=((options&8)!=0);
    outputConfigsCnt[0]=0;
    calculateDataStructureIfNeeded(nullptr,nullptr,activityToConsole); // we need the data structure, not the node.... we calculate the node here anyway

    float* retVal=nullptr;

    if (_data!=nullptr)
    {
        // Save joint positions/modes (all of them, just in case)
        std::vector<CJoint*> sceneJoints;
        std::vector<float> initSceneJointValues;
        std::vector<int> initSceneJointModes;
        _saveAllSceneJointPositionsAndModes(sceneJoints,initSceneJointValues,initSceneJointModes);

        // Set the correct mode for the joints involved:
        for (int i=0;i<int(_jointHandles.size());i++)
        {
            CJoint* joint=App::ct->objCont->getJoint(_jointHandles[i]);
            if (joint!=nullptr)
            {
                if (joint->getJointMode()!=sim_jointmode_ik)
                    joint->setJointMode(sim_jointmode_ik);
            }
        }

        // do the calculation:
//      float* pathData=_data->findPath(startConfig,goalConfig,options,stepSize,outputConfigsCnt,maxTimeInMs,auxIntParams,auxFloatParams);
        float* pathData=CPathPlanningInterface::mpFindPath(_data,startConfig,goalConfig,options,stepSize,outputConfigsCnt,maxTimeInMs,auxIntParams,auxFloatParams,randomSeed);
        randomSeed=-1;
        if (pathData!=nullptr)
        {
            int cnt=outputConfigsCnt[0]*(int)_jointHandles.size()+outputConfigsCnt[0]+outputConfigsCnt[0]*3+outputConfigsCnt[0]*4+outputConfigsCnt[0];
            retVal=(float*)new char[cnt*4];
            for (int i=0;i<cnt;i++)
                retVal[i]=pathData[i];
            CPathPlanningInterface::releaseBuffer(pathData);
        }

        // Restore joint positions/modes:
        _restoreAllSceneJointPositionsAndModes(sceneJoints,initSceneJointValues,initSceneJointModes);
    }

    return(retVal);
}

float* CMotionPlanningTask::findIkPath(const float* startConfig,const C7Vector& goalPose,int options,float stepSize,int* outputConfigsCnt,const int* auxIntParams,const float* auxFloatParams)
{
    _showPhase2FromStartConnections=((options&1)!=0);
    _showPhase2FromGoalConnections=((options&2)!=0);
    _showPhase2Path=((options&4)!=0);
    bool activityToConsole=((options&8)!=0);
    outputConfigsCnt[0]=0;
    calculateDataStructureIfNeeded(nullptr,nullptr,activityToConsole); // we need the data structure, not the node.... we calculate the node here anyway

    float* retVal=nullptr;

    if (_data!=nullptr)
    {
        // Save joint positions/modes (all of them, just in case)
        std::vector<CJoint*> sceneJoints;
        std::vector<float> initSceneJointValues;
        std::vector<int> initSceneJointModes;
        _saveAllSceneJointPositionsAndModes(sceneJoints,initSceneJointValues,initSceneJointModes);

        // Set the correct mode for the joints involved:
        for (int i=0;i<int(_jointHandles.size());i++)
        {
            CJoint* joint=App::ct->objCont->getJoint(_jointHandles[i]);
            if (joint!=nullptr)
            {
                if (joint->getJointMode()!=sim_jointmode_ik)
                    joint->setJointMode(sim_jointmode_ik);
            }
        }

        // do the calculation:
        float* pathData=CPathPlanningInterface::mpFindIkPath(_data,startConfig,goalPose.X.data,goalPose.Q.data,options,stepSize,outputConfigsCnt,auxIntParams,auxFloatParams);

        if (pathData!=nullptr)
        {
            int cnt=outputConfigsCnt[0]*(int)_jointHandles.size()+outputConfigsCnt[0]+outputConfigsCnt[0]*3+outputConfigsCnt[0]*4+outputConfigsCnt[0];
            retVal=(float*)new char[cnt*4];
            for (int i=0;i<cnt;i++)
                retVal[i]=pathData[i];
            CPathPlanningInterface::releaseBuffer(pathData);
        }

        // Restore joint positions/modes:
        _restoreAllSceneJointPositionsAndModes(sceneJoints,initSceneJointValues,initSceneJointModes);
    }

    return(retVal);
}

float* CMotionPlanningTask::simplifyPath(const float* pathBuffer,int configCnt,int options,float stepSize,int increment,int* outputConfigsCnt,int maxTimeInMs,const int* auxIntParams,const float* auxFloatParams)
{
    _showPhase2FromStartConnections=((options&1)!=0);
    _showPhase2FromGoalConnections=((options&2)!=0);
    _showPhase2Path=((options&4)!=0);
    bool activityToConsole=((options&8)!=0);
    outputConfigsCnt[0]=0;
    calculateDataStructureIfNeeded(nullptr,nullptr,activityToConsole); // we need the data structure, not the node.... we calculate the node here anyway

    float* retVal=nullptr;

    if (_data!=nullptr)
    {
        // Save joint positions/modes (all of them, just in case)
        std::vector<CJoint*> sceneJoints;
        std::vector<float> initSceneJointValues;
        std::vector<int> initSceneJointModes;
        _saveAllSceneJointPositionsAndModes(sceneJoints,initSceneJointValues,initSceneJointModes);

        // Set the correct mode for the joints involved:
        for (int i=0;i<int(_jointHandles.size());i++)
        {
            CJoint* joint=App::ct->objCont->getJoint(_jointHandles[i]);
            if (joint!=nullptr)
            {
                if (joint->getJointMode()!=sim_jointmode_ik)
                    joint->setJointMode(sim_jointmode_ik);
            }
        }

        // do the calculation:
        float* pathData=CPathPlanningInterface::mpSimplifyPath(_data,pathBuffer,configCnt,options,stepSize,increment,outputConfigsCnt,maxTimeInMs,auxIntParams,auxFloatParams);

        if (pathData!=nullptr)
        {
            int cnt=outputConfigsCnt[0]*(int)_jointHandles.size()+outputConfigsCnt[0]+outputConfigsCnt[0]*3+outputConfigsCnt[0]*4+outputConfigsCnt[0];
            retVal=(float*)new char[cnt*4];
            for (int i=0;i<cnt;i++)
                retVal[i]=pathData[i];
            CPathPlanningInterface::releaseBuffer(pathData);
        }

        // Restore joint positions/modes:
        _restoreAllSceneJointPositionsAndModes(sceneJoints,initSceneJointValues,initSceneJointModes);
    }

    return(retVal);
}

void CMotionPlanningTask::renderYour3DStuff()
{ // for debugging
    displayMotionPlanningTask(this);
}
