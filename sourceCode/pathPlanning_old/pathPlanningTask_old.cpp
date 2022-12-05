// For backward compatibility

#include "simInternal.h"
#include "pathPlanningTask_old.h"
#include "pathPlanningInterface.h"
#include "tt.h"
#include "sceneObjectContainer.h"
#include "global.h"
#include "tt.h"
#include "ikGroupContainer.h"
#include "app.h"
#include "simStrings.h"
#include "vDateTime.h"
#include "pathPlanningTaskRendering_old.h"

CPathPlanningTask::CPathPlanningTask()
{
    setDefaultValues();
}

CPathPlanningTask::CPathPlanningTask(int theID,int thePathPlanningType)
{
    setDefaultValues();
    _startDummyID=theID;
    pathPlanningType=thePathPlanningType;
}

CPathPlanningTask::~CPathPlanningTask()
{
    App::currentWorld->drawingCont->removeObject(_searchTree1Handle);
    App::currentWorld->drawingCont->removeObject(_searchTree2Handle);
    CPathPlanningInterface::destroyPathPlanningObject(_steppedSearchTemp_theTask);
}

void CPathPlanningTask::setDefaultValues()
{
    _originalTask_useWhenCopied=nullptr;
    _steppedSearchTemp_theTask=nullptr;
    _partialPathIsOk=false;
    _gammaAxis=C3Vector::unitZVector;
    _startDummyID=-1;
    _goalDummyID=-1;
    _pathID=-1;
    _robotEntityID=-1;
    _obstacleEntityID=-1;
    _visualizeSearchArea=true;
    holonomicTaskType=sim_holonomicpathplanning_xy;
    stepSize=0.01;
    angularStepSize=5.0*degToRad;
    searchMinValue[0]=-0.5;
    searchMinValue[1]=-0.5;
    searchMinValue[2]=-0.5;
    searchMinValue[3]=-piValue;
    searchRange[0]=1.0;
    searchRange[1]=1.0;
    searchRange[2]=1.0;
    searchRange[3]=piValT2;
    searchDirection[0]=0;
    searchDirection[1]=0;
    searchDirection[2]=0;
    searchDirection[3]=0;
    collisionDetection=true;
    _postProcessingPassCount=1000;
    minTurningCircleDiameter=0.1;
    obstacleClearance=0.01;
    _obstacleMaxDistance=0.05;
    _obstacleMaxDistanceEnabled=false;
    maximumTime=5.0;
    _showSearchTrees=false;
    _searchTree1Handle=-1;
    _searchTree2Handle=-1;
    _objectID=SIM_IDSTART_PATHPLANNINGTASK;
    objectName="PathPlanningTask";
}

void CPathPlanningTask::setOriginalTask(CPathPlanningTask* originalTask)
{
    _originalTask_useWhenCopied=originalTask;
}

CPathPlanningTask* CPathPlanningTask::getOriginalTask()
{
    return(_originalTask_useWhenCopied);
}

void CPathPlanningTask::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
}

void CPathPlanningTask::simulationAboutToStart()
{
    initializeInitialValues(false);
}

void CPathPlanningTask::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    //if (_initialValuesInitialized&&App::currentWorld->simulation->getResetSceneAtSimulationEnd())
    //{
    //}
    //_initialValuesInitialized=false;
}

void CPathPlanningTask::setPostProcessingPassCount(int p)
{
    tt::limitValue(0,100000,p);
    _postProcessingPassCount=p;
}

int CPathPlanningTask::getPostProcessingPassCount()
{
    return(_postProcessingPassCount);
}

void CPathPlanningTask::setPartialPathIsOk(bool ok)
{
    _partialPathIsOk=ok;
}

bool CPathPlanningTask::getPartialPathIsOk()
{
    return(_partialPathIsOk);
}

void CPathPlanningTask::setShowSearchTrees(bool s)
{
    _showSearchTrees=s;
    if (!_showSearchTrees)
    {
        App::currentWorld->drawingCont->removeObject(_searchTree1Handle);
        App::currentWorld->drawingCont->removeObject(_searchTree2Handle);
        _searchTree1Handle=-1;
        _searchTree2Handle=-1;
    }
}

bool CPathPlanningTask::getShowSearchTrees()
{
    return(_showSearchTrees);
}

void CPathPlanningTask::getAndDisconnectSearchTrees(int& tree1Handle,int& tree2Handle)
{ // Needed if we want to keep the search trees displayed but have to destroy this object!
    tree1Handle=_searchTree1Handle;
    tree2Handle=_searchTree2Handle;
    _searchTree1Handle=-1;
    _searchTree2Handle=-1;
}

void CPathPlanningTask::connectExternalSearchTrees(int tree1Handle,int tree2Handle)
{
    App::currentWorld->drawingCont->removeObject(_searchTree1Handle);
    App::currentWorld->drawingCont->removeObject(_searchTree2Handle);
    _searchTree1Handle=tree1Handle;
    _searchTree2Handle=tree2Handle;
}

void CPathPlanningTask::setMaxTime(double mTime)
{
    tt::limitValue(0.1,40000.0,mTime);
    maximumTime=mTime;
}

void CPathPlanningTask::setGoalDummyID(int theID)
{
    _goalDummyID=theID;
}
void CPathPlanningTask::setPathID(int theID)
{
    _pathID=theID;
}
void CPathPlanningTask::setRobotEntityID(int theID)
{
    _robotEntityID=theID;
}
void CPathPlanningTask::setObstacleEntityID(int theID)
{
    _obstacleEntityID=theID;
}
void CPathPlanningTask::setHolonomicTaskType(int type)
{
    holonomicTaskType=type;
}

void CPathPlanningTask::setStepSize(double size)
{
    tt::limitValue(0.0001,10.0,size);
    stepSize=size;
    if (sin(44.0*degToRad)*minTurningCircleDiameter<stepSize)
        minTurningCircleDiameter=stepSize/sin(44.0*degToRad);
}
void CPathPlanningTask::setAngularStepSize(double step)
{
    tt::limitValue(0.1*degToRad,90.0*degToRad,step);
    angularStepSize=step;
}
void CPathPlanningTask::setSearchRange(double searchMin[4],double searchR[4])
{
    for (int i=0;i<3;i++)
    {
        double v=searchMin[i];
        tt::limitValue(-1000.0,1000.0,v);
        searchMinValue[i]=v;
        v=searchR[i];
        tt::limitValue(0.0001,2000.0,v);
        searchRange[i]=v;
    }
    double v=tt::getNormalizedAngle(searchMin[3]);
    searchMinValue[3]=v;
    v=searchR[3];
    tt::limitValue(0.1*degToRad,piValT2,v);
    searchRange[3]=v;
}

void CPathPlanningTask::setSearchDirection(int dir[4])
{
    for (int i=0;i<4;i++)
    {
        searchDirection[i]=dir[i];
        if (searchDirection[i]<-1)
            searchDirection[i]=-1;
        if (searchDirection[i]>1)
            searchDirection[i]=1;
    }
}

void CPathPlanningTask::setCollisionDetection(bool c)
{
    collisionDetection=c;
    if (c)
    {
        // We need to check if _robotEntity or _obstacleEntity is a dummy:
        CDummy* it=App::currentWorld->sceneObjects->getDummyFromHandle(_robotEntityID);
        if (it!=nullptr)
            _robotEntityID=-1;
        it=App::currentWorld->sceneObjects->getDummyFromHandle(_obstacleEntityID);
        if (it!=nullptr)
            _obstacleEntityID=-1;
    }
}

void CPathPlanningTask::setObstacleClearance(double c)
{
    tt::limitValue(0.0001,100.0,c);
    double dx=_obstacleMaxDistance-obstacleClearance;
    obstacleClearance=c;
    _obstacleMaxDistance=obstacleClearance+dx;
}
double CPathPlanningTask::getObstacleClearance()
{
    return(obstacleClearance);
}

void CPathPlanningTask::setObstacleMaxDistance(double c)
{
    tt::limitValue(obstacleClearance+0.0001,100.0,c);
    _obstacleMaxDistance=c;
}
double CPathPlanningTask::getObstacleMaxDistance()
{
    return(_obstacleMaxDistance);
}
void CPathPlanningTask::setObstacleMaxDistanceEnabled(bool e)
{
    _obstacleMaxDistanceEnabled=e;
}
bool CPathPlanningTask::getObstacleMaxDistanceEnabled()
{
    return(_obstacleMaxDistanceEnabled);
}

void CPathPlanningTask::setMinTurningCircleDiameter(double d)
{
    tt::limitValue(0.001,10.0,d);
    minTurningCircleDiameter=d;
    if (sin(44.0*degToRad)*minTurningCircleDiameter<stepSize)
        stepSize=minTurningCircleDiameter*sin(44.0*degToRad);
}
double CPathPlanningTask::getMinTurningCircleDiameter()
{
    return(minTurningCircleDiameter);
}

int CPathPlanningTask::getPathPlanningType()
{
    return(pathPlanningType);
}

CPathPlanningTask* CPathPlanningTask::copyYourself()
{
    int theid=_startDummyID;
    CPathPlanningTask* newTask=new CPathPlanningTask(theid,pathPlanningType);
    // Loose body parameters:
    newTask->_goalDummyID=_goalDummyID;
    newTask->_robotEntityID=_robotEntityID;
    newTask->_obstacleEntityID=_obstacleEntityID;
    newTask->_visualizeSearchArea=_visualizeSearchArea;
    newTask->stepSize=stepSize;
    newTask->angularStepSize=angularStepSize;
    for (int i=0;i<4;i++)
    {
        newTask->searchMinValue[i]=searchMinValue[i];
        newTask->searchRange[i]=searchRange[i];
        newTask->searchDirection[i]=searchDirection[i];
    }
    newTask->holonomicTaskType=holonomicTaskType;
    newTask->minTurningCircleDiameter=minTurningCircleDiameter;

    // Common parameters:
    newTask->_pathID=_pathID;
    newTask->maximumTime=maximumTime;
    newTask->_objectID=_objectID;
    newTask->objectName=objectName;
    newTask->collisionDetection=collisionDetection;
    newTask->obstacleClearance=obstacleClearance;
    newTask->_obstacleMaxDistance=_obstacleMaxDistance;
    newTask->_obstacleMaxDistanceEnabled=_obstacleMaxDistanceEnabled;
    newTask->_postProcessingPassCount=_postProcessingPassCount;
    newTask->_partialPathIsOk=_partialPathIsOk;
    newTask->_gammaAxis=_gammaAxis;
    newTask->_showSearchTrees=_showSearchTrees;
    
    return(newTask);
}


void CPathPlanningTask::serialize(CSer& ar)
{
    if (ar.isStoring())
    {       // Storing
        ar.storeDataName("Idx");
        ar << _pathID << _robotEntityID << _obstacleEntityID;
        ar.flush();

        ar.storeDataName("Gid");
        ar << _goalDummyID;
        ar.flush();

        ar.storeDataName("Pod");
        ar << _startDummyID;
        ar.flush();

        ar.storeDataName("Ppt");
        ar << pathPlanningType;
        ar.flush();

        ar.storeDataName("Ttp");
        ar << holonomicTaskType;
        ar.flush();

#ifdef TMPOPERATION
        ar.storeDataName("Pss");
        ar << (float)stepSize << (float)angularStepSize;
        ar.flush();
#endif

        ar.storeDataName("_ss");
        ar << stepSize << angularStepSize;
        ar.flush();


#ifdef TMPOPERATION
        ar.storeDataName("Rn2");
        for (int i=0;i<4;i++)
            ar << (float)searchMinValue[i] << (float)searchRange[i];
        ar.flush();
#endif

        ar.storeDataName("_n2");
        for (int i=0;i<4;i++)
            ar << searchMinValue[i] << searchRange[i];
        ar.flush();


#ifdef TMPOPERATION
        ar.storeDataName("Mtd");
        ar << (float)minTurningCircleDiameter;
        ar.flush();
#endif

        ar.storeDataName("_td");
        ar << minTurningCircleDiameter;
        ar.flush();


        ar.storeDataName("Nme");
        ar << objectName;
        ar.flush();

        ar.storeDataName("Oid");
        ar << _objectID;
        ar.flush();

#ifdef TMPOPERATION
        ar.storeDataName("Mxt");
        ar << (float)maximumTime;
        ar.flush();
#endif

        ar.storeDataName("_xt");
        ar << maximumTime;
        ar.flush();


        ar.storeDataName("Sd2");
        for (int i=0;i<4;i++)
            ar << searchDirection[i];
        ar.flush();

        ar.storeDataName("Var");
        unsigned char dummy=0;
        SIM_SET_CLEAR_BIT(dummy,0,collisionDetection);
        SIM_SET_CLEAR_BIT(dummy,3,_showSearchTrees);
        SIM_SET_CLEAR_BIT(dummy,4,_visualizeSearchArea);
        SIM_SET_CLEAR_BIT(dummy,5,_obstacleMaxDistanceEnabled);
        SIM_SET_CLEAR_BIT(dummy,7,_partialPathIsOk);
        ar << dummy;
        ar.flush();

#ifdef TMPOPERATION
        ar.storeDataName("Ocl");
        ar << (float)obstacleClearance;
        ar.flush();
#endif

        ar.storeDataName("_cl");
        ar << obstacleClearance;
        ar.flush();


#ifdef TMPOPERATION
        ar.storeDataName("Omd");
        ar << (float)_obstacleMaxDistance;
        ar.flush();
#endif

        ar.storeDataName("_md");
        ar << _obstacleMaxDistance;
        ar.flush();


        ar.storeDataName("Pp2");
        ar << _postProcessingPassCount;
        ar.flush();

#ifdef TMPOPERATION
        ar.storeDataName("Gaa");
        ar << (float)_gammaAxis(0) << (float)_gammaAxis(1) << (float)_gammaAxis(2);
        ar.flush();
#endif

        ar.storeDataName("_aa");
        ar << _gammaAxis(0) << _gammaAxis(1) << _gammaAxis(2);
        ar.flush();


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
                if (theName.compare("Idx")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> _pathID >> _robotEntityID >> _obstacleEntityID;
                }
                if (theName.compare("Gid")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> _goalDummyID;
                }
                if (theName.compare("Pod")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> _startDummyID;
                }
                if (theName.compare("Ppt")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> pathPlanningType;
                }
                if (theName.compare("Ttp")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> holonomicTaskType;
                }
                if (theName.compare("Pss")==0)
                { // for backward comp. (flt->dbl)
                    noHit=false;
                    ar >> byteQuantity;
                    float bla,bli;
                    ar >> bla >> bli;
                    stepSize=(double)bla;
                    angularStepSize=(double)bli;
                }

                if (theName.compare("_ss")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> stepSize >> angularStepSize;
                }

                if (theName.compare("Rn2")==0)
                { // for backward comp. (flt->dbl)
                    noHit=false;
                    ar >> byteQuantity;
                    float bla,bli;
                    for (int i=0;i<4;i++)
                    {
                        ar >> bla >> bli;
                        searchMinValue[i]=(double)bla;
                        searchRange[i]=(double)bli;
                    }
                }

                if (theName.compare("_n2")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    for (int i=0;i<4;i++)
                        ar >> searchMinValue[i] >> searchRange[i];
                }

                if (theName.compare("Mtd")==0)
                { // for backward comp. (flt->dbl)
                    noHit=false;
                    ar >> byteQuantity;
                    float bla;
                    ar >> bla;
                    minTurningCircleDiameter=(double)bla;
                }

                if (theName.compare("_td")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> minTurningCircleDiameter;
                }

                if (theName.compare("Nme")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> objectName;
                }
                if (theName.compare("Oid")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> _objectID;
                }
                if (theName.compare("Mxt")==0)
                { // for backward comp. (flt->dbl)
                    noHit=false;
                    ar >> byteQuantity;
                    float bla;
                    ar >> bla;
                    maximumTime=(double)bla;
                }

                if (theName.compare("_xt")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> maximumTime;
                }

                if (theName.compare("Sd2")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    for (int i=0;i<4;i++)
                        ar >> searchDirection[i];
                }
                if (theName.compare("Var")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    unsigned char dummy;
                    ar >> dummy;
                    collisionDetection=SIM_IS_BIT_SET(dummy,0);
                    _showSearchTrees=SIM_IS_BIT_SET(dummy,3);
                    _visualizeSearchArea=SIM_IS_BIT_SET(dummy,4);
                    _obstacleMaxDistanceEnabled=SIM_IS_BIT_SET(dummy,5);
                    _partialPathIsOk=SIM_IS_BIT_SET(dummy,7);
                }
                if (theName.compare("Ocl")==0)
                { // for backward comp. (flt->dbl)
                    noHit=false;
                    ar >> byteQuantity;
                    float bla;
                    ar >> bla;
                    obstacleClearance=(double)bla;
                }

                if (theName.compare("_cl")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> obstacleClearance;
                }

                if (theName.compare("Omd")==0)
                { // for backward comp. (flt->dbl)
                    noHit=false;
                    ar >> byteQuantity;
                    float bla;
                    ar >> bla;
                    _obstacleMaxDistance=(double)bla;
                }

                if (theName.compare("_md")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> _obstacleMaxDistance;
                }

                if (theName.compare("Pp2")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> _postProcessingPassCount;
                }
                if (theName.compare("Gaa")==0)
                { // for backward comp. (flt->dbl)
                    noHit=false;
                    ar >> byteQuantity;
                    float bla;
                    for (size_t i=0;i<3;i++)
                    {
                        ar >> bla;
                        _gammaAxis(i)=(double)bla;
                    }
                }

                if (theName.compare("_aa")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> _gammaAxis(0) >> _gammaAxis(1) >> _gammaAxis(2);
                }

                if (noHit)
                    ar.loadUnknownData();
            }
        }
    }
}

void CPathPlanningTask::performObjectLoadingMapping(const std::map<int,int>* map)
{
    _startDummyID=CWorld::getLoadingMapping(map,_startDummyID);
    _goalDummyID=CWorld::getLoadingMapping(map,_goalDummyID);
    _pathID=CWorld::getLoadingMapping(map,_pathID);
    if (_robotEntityID<SIM_IDSTART_COLLECTION)
        _robotEntityID=CWorld::getLoadingMapping(map,_robotEntityID);
    if (_obstacleEntityID<SIM_IDSTART_COLLECTION)
        _obstacleEntityID=CWorld::getLoadingMapping(map,_obstacleEntityID);
}
void CPathPlanningTask::performCollectionLoadingMapping(const std::map<int,int>* map)
{
    if (_robotEntityID>=SIM_IDSTART_COLLECTION)
        _robotEntityID=CWorld::getLoadingMapping(map,_robotEntityID);
    if (_obstacleEntityID>=SIM_IDSTART_COLLECTION)
        _obstacleEntityID=CWorld::getLoadingMapping(map,_obstacleEntityID);
}
bool CPathPlanningTask::announceObjectWillBeErased(int objID,bool copyBuffer)
{ // Return value true means that this object should be destroyed
    if (_startDummyID==objID)
        _startDummyID=-1;
    if (_goalDummyID==objID)
        _goalDummyID=-1;
    if (_pathID==objID)
        _pathID=-1;
    if (_robotEntityID==objID)
        _robotEntityID=-1;
    if (_obstacleEntityID==objID)
        _obstacleEntityID=-1;
    return((_pathID==-1)||(_startDummyID==-1)||(_goalDummyID==-1)||(_robotEntityID==-1));
}
bool CPathPlanningTask::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{ // Return value true means that this object should be destroyed
    if (_robotEntityID==groupID)
        _robotEntityID=-1;
    if (_obstacleEntityID==groupID)
        _obstacleEntityID=-1;
    return(_robotEntityID==-1);
}
int CPathPlanningTask::getObjectID()
{
    return(_objectID);
}
void CPathPlanningTask::setObjectID(int newID)
{
    _objectID=newID;
}
std::string CPathPlanningTask::getObjectName()
{
    return(objectName);
}
void CPathPlanningTask::setObjectName(std::string newName)
{
    objectName=newName;
}
int CPathPlanningTask::getStartDummyID()
{
    return(_startDummyID);
}
int CPathPlanningTask::getGoalDummyID()
{
    return(_goalDummyID);
}
int CPathPlanningTask::getPathID()
{
    return(_pathID);
}
int CPathPlanningTask::getRobotEntityID()
{
    return(_robotEntityID);
}
int CPathPlanningTask::getObstacleEntityID()
{
    return(_obstacleEntityID);
}

void CPathPlanningTask::setVisualizeSearchArea(bool v)
{
    _visualizeSearchArea=v;
}

bool CPathPlanningTask::getVisualizeSearchArea()
{
    return(_visualizeSearchArea);
}

int CPathPlanningTask::getHolonomicTaskType()
{
    return(holonomicTaskType);
}
double CPathPlanningTask::getStepSize()
{
    return(stepSize);
}
double CPathPlanningTask::getAngularStepSize()
{
    return(angularStepSize);
}

void CPathPlanningTask::setGammaAxis(const C3Vector& axis)
{
    if (axis.getLength()>0.001)
        _gammaAxis=axis; // We don't normalize, since we want the user to be able to enter (2;0;3) for example
}

C3Vector CPathPlanningTask::getGammaAxis()
{
    return(_gammaAxis);
}

void CPathPlanningTask::getSearchRange(double searchMin[4],double searchR[4])
{
    for (int i=0;i<4;i++)
    {
        searchMin[i]=searchMinValue[i];
        searchR[i]=searchRange[i];
    }
}
void CPathPlanningTask::getSearchDirection(int dir[4])
{
    for (int i=0;i<4;i++)
        dir[i]=searchDirection[i];
}
bool CPathPlanningTask::getCollisionDetection()
{
    return(collisionDetection);
}
double CPathPlanningTask::getMaxTime()
{
    return(maximumTime);
}
bool CPathPlanningTask::isTaskValid()
{
    if (App::currentWorld->sceneObjects->getPathFromHandle(_pathID)==nullptr)
        return(false); // No path!
    if (pathPlanningType==sim_pathplanning_holonomic)
    { // Holonomic path planning
        if (App::currentWorld->sceneObjects->getDummyFromHandle(_startDummyID)==nullptr)
            return(false);
        if (App::currentWorld->sceneObjects->getDummyFromHandle(_goalDummyID)==nullptr)
            return(false);
        if (_startDummyID==_goalDummyID)
            return(false);
        return(true);
    }
    if (pathPlanningType==sim_pathplanning_nonholonomic)
    { // Holonomic path planning
        if (App::currentWorld->sceneObjects->getDummyFromHandle(_startDummyID)==nullptr)
            return(false);
        if (App::currentWorld->sceneObjects->getDummyFromHandle(_goalDummyID)==nullptr)
            return(false);
        if (_startDummyID==_goalDummyID)
            return(false);
        return(true);
    }
    return(false);
}
bool CPathPlanningTask::performSearch(bool showProgressDlg,double maxTime)
{ // if maxTime is 0.0 or negative, the default maxTime is used

    if (initiateSteppedSearch(showProgressDlg,maxTime,0.1))
    {
        int res=-2;
        while (res==-2)
        {
            res=performSteppedSearch();
        }
        if (res>0)
            return(true); // we found a path or a partial path
    }
    return(false); // we didn't find a path or error

}

bool CPathPlanningTask::initiateSteppedSearch(bool showProgressDlg,double maxTime,double subDt)
{
    if (!isTaskValid())
        return(false);
    CPath_old* thePath=App::currentWorld->sceneObjects->getPathFromHandle(_pathID);
    if (thePath==nullptr)
        return(false);
    _steppedSearchTemp_initTimeInMs=(int)VDateTime::getTimeInMs();
    _steppedSearchTemp_maxTimeInMs=int(maximumTime*1000.0);
    if (maxTime>0.0)
        _steppedSearchTemp_maxTimeInMs=int(maxTime*1000.0);
    _steppedSearchTemp_maxSubTimeInMs=int(subDt*1000.0);
    _steppedSearchTemp_showProgressDlg=showProgressDlg;
    _steppedSearchTemp_foundPathStatus=0; // means nothing found yet!
    _steppedSearchTemp_currentSmoothingPass=0;
    thePath->pathContainer->removeAllSimplePathPoints();
    double clearanceAndMaxDistance[2]={obstacleClearance,_obstacleMaxDistance};
    if (collisionDetection)
    {
        clearanceAndMaxDistance[0]=-1.0;
        clearanceAndMaxDistance[1]=-1.0;
    }
    else
    {
        if (!_obstacleMaxDistanceEnabled)
            clearanceAndMaxDistance[1]=-1.0;
    }
    double angularCoeff=stepSize/angularStepSize;
    if (pathPlanningType==sim_pathplanning_nonholonomic)
    {
        double maxSteeringAngle=2.0*CMath::robustAsin(stepSize/minTurningCircleDiameter);
        void* p=CPathPlanningInterface::createNonHolonomicPathPlanningObject(_startDummyID,_goalDummyID,_robotEntityID,
            _obstacleEntityID,angularCoeff,1.0,0.01,maxSteeringAngle,stepSize,searchMinValue,searchRange,searchDirection,clearanceAndMaxDistance);
        _steppedSearchTemp_theTask=p;
    }
    if (pathPlanningType==sim_pathplanning_holonomic)
    {
        void* p=CPathPlanningInterface::createHolonomicPathPlanningObject(_startDummyID,_goalDummyID,_robotEntityID,_obstacleEntityID,holonomicTaskType,
            angularCoeff,stepSize,searchMinValue,searchRange,searchDirection,clearanceAndMaxDistance,_gammaAxis.getNormalized().data);
        _steppedSearchTemp_theTask=p;
    }
    return(_steppedSearchTemp_theTask!=nullptr);
}

int CPathPlanningTask::performSteppedSearch()
{ // retVal: -1: error, -2: search not yet finished, 0: no path found, 1: partial path found, 2: full path found
    int retVal=-1;
    if (!isTaskValid())
        return(retVal);
    CPath_old* thePath=App::currentWorld->sceneObjects->getPathFromHandle(_pathID);
    if (thePath==nullptr)
        return(retVal);
    if (pathPlanningType==sim_pathplanning_nonholonomic)
    {
        void* p=(void*)_steppedSearchTemp_theTask;
        int timeDiv=_steppedSearchTemp_maxTimeInMs/_steppedSearchTemp_maxSubTimeInMs;
        if (timeDiv<1)
            timeDiv=1;
        if (_steppedSearchTemp_foundPathStatus==0)
        { // we have to search more
            if (VDateTime::getTimeDiffInMs(_steppedSearchTemp_initTimeInMs)<_steppedSearchTemp_maxTimeInMs)
            {
                retVal=-2; // search not yet finished
                if (CPathPlanningInterface::searchPath(p,_steppedSearchTemp_maxTimeInMs/timeDiv)!=0)
                { // we found a path!
                    if (_steppedSearchTemp_showProgressDlg)
                    {
                        // App::uiThread->setProgressBarText(IDS_PATH_WAS_FOUND_,IDS_NOW_PERFORMING_POST_PROCESSING___,nullptr);
                    }
                    _steppedSearchTemp_foundPathStatus=2; // we found a full path!
                    retVal=-2; // search not completely finished (we still have to smooth the path)
                }
                else
                { // we didn't yet find a path
                    if (_steppedSearchTemp_showProgressDlg)
                    { /*
                        std::string txt(IDS_TIME_SPENT_SEARCHING_);
                        txt+=" ";
                        txt+=tt::FNb(0,double(VDateTime::getTimeDiffInMs(_steppedSearchTemp_initTimeInMs))/1000.0,1,false);
                        std::string txt2=IDS_NB_OF_COLL_FREE_NODES_FOUND_;
                        txt2+=" ";
                        txt2+=tt::FNb(0,CPathPlanningInterface::getPathNodeCount(p,true)+CPathPlanningInterface::getPathNodeCount(p,false),false);
                        App::uiThread->setProgressBarText(txt.c_str(),txt2.c_str(),nullptr); */
                    }
                }
            }
            else
            { // We ran out of time!
                retVal=0; // we didn't find a path
                if (_partialPathIsOk)
                {
                    if (CPathPlanningInterface::setPartialPath(p)!=0)
                    {
                        if (_steppedSearchTemp_showProgressDlg)
                        {
                            // App::uiThread->setProgressBarText(IDS_PARTIAL_PATH_WAS_FOUND_,IDS_NOW_PERFORMING_POST_PROCESSING___,nullptr);
                        }
                        _steppedSearchTemp_foundPathStatus=1; // we found a partial path!
                        retVal=-2; // search not completely finished (we still have to smooth the path)
                    }
                }
            }
        }
        else
        { // we have a partial or full path!
            int res=CPathPlanningInterface::smoothFoundPath(p,_postProcessingPassCount,timeDiv);
            if (res==-1)
                retVal=-2; // we have to continue smoothing
            if (res==0)
                retVal=0; // we didn't find anything (the smoothing actually failed)
            if (res==1)
            {
                CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(_pathID);
                int nodeCount;
                double* pathData=CPathPlanningInterface::getFoundPath(p,&nodeCount);
                if ((it!=nullptr)&&(pathData!=nullptr))
                {
                    it->pathContainer->enableActualization(false);
                    it->pathContainer->removeAllSimplePathPoints();
                    int at=sim_pathproperty_automatic_orientation|sim_pathproperty_closed_path;
                    it->pathContainer->setAttributes((it->pathContainer->getAttributes()|at)-at);
                    it->pathContainer->setSquareSize(stepSize*0.25);
                    C7Vector pathInv(it->getFullCumulativeTransformation().getInverse());
                    for (int i=0;i<nodeCount;i++)
                    {
                        C7Vector conf;
                        conf.X.setData(pathData+0+7*i);
                        conf.Q.setData(pathData+3+7*i);
                        conf=pathInv*conf;
                        CSimplePathPoint_old* nspp=new CSimplePathPoint_old();
                        nspp->setTransformation(conf,it->pathContainer->getAttributes());
                        it->pathContainer->addSimplePathPoint(nspp);
                    }
                    it->pathContainer->enableActualization(true);
                    it->pathContainer->actualizePath();
                    CPathPlanningInterface::releaseBuffer(pathData);
                }
                retVal=_steppedSearchTemp_foundPathStatus; // we found a path (FULL) or partial
            }
        }
    }
    if (pathPlanningType==sim_pathplanning_holonomic)
    {
        void* p=(void*)_steppedSearchTemp_theTask;
        int timeDiv=_steppedSearchTemp_maxTimeInMs/_steppedSearchTemp_maxSubTimeInMs;
        if (timeDiv<1)
            timeDiv=1;
        if (_steppedSearchTemp_foundPathStatus==0)
        { // we have to search more
            if (VDateTime::getTimeDiffInMs(_steppedSearchTemp_initTimeInMs)<_steppedSearchTemp_maxTimeInMs)
            {
                retVal=-2; // search not yet finished
                if (CPathPlanningInterface::searchPath(p,_steppedSearchTemp_maxTimeInMs/timeDiv)!=0)
                { // We found a path!
                    if (_steppedSearchTemp_showProgressDlg)
                    {
                        // App::uiThread->setProgressBarText(IDS_PATH_WAS_FOUND_,IDS_NOW_PERFORMING_POST_PROCESSING___,nullptr);
                    }
                    _steppedSearchTemp_foundPathStatus=2; // we found a full path!
                    retVal=-2; // search not completely finished (we still have to smooth the path)
                }
                else
                { // we didn't yet find a path
                    if (_steppedSearchTemp_showProgressDlg)
                    {/*
                        std::string txt(IDS_TIME_SPENT_SEARCHING_);
                        txt+=" ";
                        txt+=tt::FNb(0,double(VDateTime::getTimeDiffInMs(_steppedSearchTemp_initTimeInMs))/1000.0,1,false);
                        std::string txt2=IDS_NB_OF_COLL_FREE_NODES_FOUND_;
                        txt2+=" ";
                        txt2+=tt::FNb(0,CPathPlanningInterface::getPathNodeCount(p,true)+CPathPlanningInterface::getPathNodeCount(p,false),false);
                        App::uiThread->setProgressBarText(txt.c_str(),txt2.c_str(),nullptr); */
                    }
                }
            }
            else
            { // we ran out of time!
                retVal=0; // we didn't find a path
                if (_partialPathIsOk)
                {
                    if (CPathPlanningInterface::setPartialPath(p)!=0)
                    {
                        if (_steppedSearchTemp_showProgressDlg)
                        {
                            // App::uiThread->setProgressBarText(IDS_PARTIAL_PATH_WAS_FOUND_,IDS_NOW_PERFORMING_POST_PROCESSING___,nullptr);
                        }
                        _steppedSearchTemp_foundPathStatus=1; // we found a partial path!
                        retVal=-2; // search not completely finished (we still have to smooth the path)
                    }
                }
            }
        }
        else
        { // we have a partial or full path!
            int res=CPathPlanningInterface::smoothFoundPath(p,_postProcessingPassCount,timeDiv);
            if (res==-1)
                retVal=-2; // we have to continue smoothing
            if (res==0)
                retVal=0; // we didn't find anything (the smoothing actually failed)
            if (res==1)
            {
                CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(_pathID);
                int nodeCount;
                double* pathData=CPathPlanningInterface::getFoundPath(p,&nodeCount);
                if ((it!=nullptr)&&(pathData!=nullptr))
                {
                    it->pathContainer->enableActualization(false);
                    it->pathContainer->removeAllSimplePathPoints();
                    int at=sim_pathproperty_automatic_orientation|sim_pathproperty_closed_path;
                    it->pathContainer->setAttributes((it->pathContainer->getAttributes()|at)-at);
                    it->pathContainer->setSquareSize(stepSize*0.25);
                    C7Vector pathInv(it->getFullCumulativeTransformation().getInverse());
                    for (int i=0;i<nodeCount;i++)
                    {
                        C7Vector conf;
                        conf.X.setData(pathData+0+7*i);
                        conf.Q.setData(pathData+3+7*i);
                        conf=pathInv*conf;
                        CSimplePathPoint_old* nspp=new CSimplePathPoint_old();
                        nspp->setTransformation(conf,it->pathContainer->getAttributes());
                        it->pathContainer->addSimplePathPoint(nspp);
                    }
                    it->pathContainer->enableActualization(true);
                    it->pathContainer->actualizePath();
                    CPathPlanningInterface::releaseBuffer(pathData);
                }

                retVal=_steppedSearchTemp_foundPathStatus; // we found a path (FULL) or partial
            }
        }
    }
    if (retVal!=-2)
    { // This is the last time in this routine. Display the trees and clean-up!
        if ((retVal>0)&&_showSearchTrees)
        {
            App::currentWorld->drawingCont->removeObject(_searchTree1Handle);
            App::currentWorld->drawingCont->removeObject(_searchTree2Handle);
            int fromStartC;
            double* fromStart=CPathPlanningInterface::getSearchTree(_steppedSearchTemp_theTask,&fromStartC,true);
            int fromGoalC;
            double* fromGoal=CPathPlanningInterface::getSearchTree(_steppedSearchTemp_theTask,&fromGoalC,false);
            if (fromStart!=nullptr)
            {
                CDrawingObject* it=new CDrawingObject(sim_drawing_lines,1.0,0.0,-1,1000000,-1);
                it->color.setColor(1.0,0.0,0.0,sim_colorcomponent_ambient_diffuse);
                for (int i=0;i<fromStartC;i++)
                    it->addItem(fromStart+6*i);
                _searchTree1Handle=App::currentWorld->drawingCont->addObject(it);
            }
            if (fromGoal!=nullptr)
            {
                CDrawingObject* it=new CDrawingObject(sim_drawing_lines,1.0,0.0,-1,1000000,-1);
                it->color.setColor(0.0,0.0,1.0,sim_colorcomponent_ambient_diffuse);
                for (int i=0;i<fromGoalC;i++)
                    it->addItem(fromGoal+6*i);
                _searchTree2Handle=App::currentWorld->drawingCont->addObject(it);
            }
            CPathPlanningInterface::releaseBuffer(fromGoal);
            CPathPlanningInterface::releaseBuffer(fromStart);
        }
        CPathPlanningInterface::destroyPathPlanningObject(_steppedSearchTemp_theTask);
        _steppedSearchTemp_theTask=nullptr;
    }
    return(retVal);
}

void CPathPlanningTask::renderYour3DStuff()
{
    if ((_startDummyID==-1)||(!_visualizeSearchArea)||(!App::currentWorld->simulation->isSimulationStopped()))
        return;
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_startDummyID);
    if (it!=nullptr)
    {
        C4X4Matrix m(it->getCumulativeTransformation().getMatrix());
        C3Vector corners[8];
        for (int i=0;i<2;i++)
        {
            for (int j=0;j<2;j++)
            {
                for (int k=0;k<2;k++)
                {
                    double theX=1.0;
                    double theY=1.0;
                    double theZ=1.0;
                    if (pathPlanningType==sim_pathplanning_holonomic)
                    {
                        if (holonomicTaskType==sim_holonomicpathplanning_abg)
                            theX=0.0;
                        if ((holonomicTaskType==sim_holonomicpathplanning_xg)||
                            (holonomicTaskType==sim_holonomicpathplanning_abg)||
                            (holonomicTaskType==sim_holonomicpathplanning_xabg))
                            theY=0.0;
                        if ((holonomicTaskType==sim_holonomicpathplanning_xy)||
                            (holonomicTaskType==sim_holonomicpathplanning_xg)||
                            (holonomicTaskType==sim_holonomicpathplanning_xyg)||
                            (holonomicTaskType==sim_holonomicpathplanning_abg)||
                            (holonomicTaskType==sim_holonomicpathplanning_xabg)||
                            (holonomicTaskType==sim_holonomicpathplanning_xyabg))
                            theZ=0.0;
                    }
                    if (pathPlanningType==sim_pathplanning_nonholonomic)
                        theZ=0.0;
                    corners[4*i+2*j+k]=m.X+(m.M.axis[0]*searchMinValue[0]+m.M.axis[0]*double(i)*searchRange[0])*theX;
                    corners[4*i+2*j+k]+=(m.M.axis[1]*searchMinValue[1]+m.M.axis[1]*double(j)*searchRange[1])*theY;
                    corners[4*i+2*j+k]+=(m.M.axis[2]*searchMinValue[2]+m.M.axis[2]*double(k)*searchRange[2])*theZ;
                }
            }
        }
        displayPathPlanningTask(corners);
    }
}
