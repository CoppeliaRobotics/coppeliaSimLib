
#include "pathPlanningInterface.h"
#include "v_repConst.h"
#include <math.h>
#include "pluginContainer.h"
#include "nonHolonomicPathPlanning.h"
#include "holonomicPathPlanning.h"
#include "mpObject.h"

ptr_createNonHolonomicPathPlanningObject CPathPlanningInterface::_createNonHolonomicPathPlanningObject;
ptr_createHolonomicPathPlanningObject CPathPlanningInterface::_createHolonomicPathPlanningObject;
ptr_destroyPathPlanningObject CPathPlanningInterface::_destroyPathPlanningObject;
ptr_searchPath CPathPlanningInterface::_searchPath;
ptr_getPathNodeCount CPathPlanningInterface::_getPathNodeCount;
ptr_setPartialPath CPathPlanningInterface::_setPartialPath;
ptr_smoothFoundPath CPathPlanningInterface::_smoothFoundPath;
ptr_getFoundPath CPathPlanningInterface::_getFoundPath;
ptr_getSearchTree CPathPlanningInterface::_getSearchTree;
ptr_releaseBuffer CPathPlanningInterface::_releaseBuffer;

ptr_createMpObject CPathPlanningInterface::_createMpObject;
ptr_destroyMpObject CPathPlanningInterface::_destroyMpObject;
ptr_setMpObjectData CPathPlanningInterface::_setMpObjectData;
ptr_calculateMpNodesPhase1 CPathPlanningInterface::_calculateMpNodesPhase1;
ptr_getMpPhase1NodesRenderData CPathPlanningInterface::_getMpPhase1NodesRenderData;
ptr_getMpPhase2NodesRenderData CPathPlanningInterface::_getMpPhase2NodesRenderData;
ptr_getMpSerializationData CPathPlanningInterface::_getMpSerializationData;
ptr_getMpPhase1NodeCnt CPathPlanningInterface::_getMpPhase1NodeCnt;
ptr_getMpRobotConfigFromTipPose CPathPlanningInterface::_getMpRobotConfigFromTipPose;
ptr_mpFindPath CPathPlanningInterface::_mpFindPath;
ptr_mpFindIkPath CPathPlanningInterface::_mpFindIkPath;
ptr_mpSimplifyPath CPathPlanningInterface::_mpSimplifyPath;
ptr_mpGetConfigTransition CPathPlanningInterface::_mpGetConfigTransition;




bool CPathPlanningInterface::_pathPlanningInterfaceInitialized=false;
bool CPathPlanningInterface::_pathPlanningSystemPluginsLoadPhaseOver=false;

void CPathPlanningInterface::setSystemPluginsLoadPhaseOver()
{
    _pathPlanningSystemPluginsLoadPhaseOver=true;
}

bool CPathPlanningInterface::initializeFunctionsIfNeeded()
{
    static bool alreadyTried=false;
    if (_pathPlanningInterfaceInitialized)
        return(true);
    if (alreadyTried&&_pathPlanningSystemPluginsLoadPhaseOver)
        return(false);
    alreadyTried=true;
    CPlugin* plugin=CPluginContainer::getPluginFromName("PathPlanning");
    if (plugin==nullptr)
        return(false); // plugin not there!
    // Get the version first:
    int auxVals[4]={0,0,0,0};
    int retVals[4];
    plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,retVals);
//  int version=retVals[0];
    // Now get all the function pointers:
    int v=1;
    auxVals[0]=v++;
    _createNonHolonomicPathPlanningObject=(ptr_createNonHolonomicPathPlanningObject)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _createHolonomicPathPlanningObject=(ptr_createHolonomicPathPlanningObject)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _destroyPathPlanningObject=(ptr_destroyPathPlanningObject)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _searchPath=(ptr_searchPath)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _getPathNodeCount=(ptr_getPathNodeCount)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _setPartialPath=(ptr_setPartialPath)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _smoothFoundPath=(ptr_smoothFoundPath)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _getFoundPath=(ptr_getFoundPath)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _getSearchTree=(ptr_getSearchTree)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _releaseBuffer=(ptr_releaseBuffer)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;

    _createMpObject=(ptr_createMpObject)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _destroyMpObject=(ptr_destroyMpObject)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _setMpObjectData=(ptr_setMpObjectData)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _calculateMpNodesPhase1=(ptr_calculateMpNodesPhase1)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _getMpPhase1NodesRenderData=(ptr_getMpPhase1NodesRenderData)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _getMpPhase2NodesRenderData=(ptr_getMpPhase2NodesRenderData)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _getMpSerializationData=(ptr_getMpSerializationData)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _getMpPhase1NodeCnt=(ptr_getMpPhase1NodeCnt)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _getMpRobotConfigFromTipPose=(ptr_getMpRobotConfigFromTipPose)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _mpFindPath=(ptr_mpFindPath)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _mpFindIkPath=(ptr_mpFindIkPath)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _mpSimplifyPath=(ptr_mpSimplifyPath)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    _mpGetConfigTransition=(ptr_mpGetConfigTransition)plugin->sendEventCallbackMessage(sim_message_eventcallback_pathplanningplugin,auxVals,nullptr,nullptr);
    auxVals[0]=v++;
    // IF YOU ADD A FUNCTION HERE, DO NOT FORGET TO ADD A CORRESPONDING ENTRY IN v_repExtPathPlanning.cpp !!!

    _pathPlanningInterfaceInitialized=true;
    return(true);
}

CDummyNonHolonomicPathPlanning* CPathPlanningInterface::createNonHolonomicPathPlanningObject(int theStartDummyID,int theGoalDummyID,
                            int theRobotCollectionID,int theObstacleCollectionID,float theAngularCoeff,
                            float theSteeringAngleCoeff,float theMaxSteeringAngleVariation,float theMaxSteeringAngle,
                            float theStepSize,const float theSearchMinVal[2],const float theSearchRange[2],
                            const int theDirectionConstraints[2],const float clearanceAndMaxDistance[2])
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        CNonHolonomicPathPlanning* p=new CNonHolonomicPathPlanning(theStartDummyID,theGoalDummyID,theRobotCollectionID,
                theObstacleCollectionID,-1,theAngularCoeff,theSteeringAngleCoeff,theMaxSteeringAngleVariation,theMaxSteeringAngle,theStepSize,
                theSearchMinVal,theSearchRange,theDirectionConstraints,clearanceAndMaxDistance);
        return((CDummyNonHolonomicPathPlanning*)p);
    }
    // we call the plugin function:
    return((CDummyNonHolonomicPathPlanning*)_createNonHolonomicPathPlanningObject(theStartDummyID,theGoalDummyID,theRobotCollectionID,
            theObstacleCollectionID,theAngularCoeff,theSteeringAngleCoeff,theMaxSteeringAngleVariation,theMaxSteeringAngle,theStepSize,
            theSearchMinVal,theSearchRange,theDirectionConstraints,clearanceAndMaxDistance));
}

CDummyHolonomicPathPlanning* CPathPlanningInterface::createHolonomicPathPlanningObject(int theStartDummyID,int theGoalDummyID,
                            int theRobotCollectionID,int theObstacleCollectionID,int thePlanningType,
                            float theAngularCoeff,float theStepSize,const float theSearchMinVal[4],const float theSearchRange[4],
                            const int theDirectionConstraints[4],const float clearanceAndMaxDistance[2],const float gammaAxis[3])
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        CHolonomicPathPlanning* p=new CHolonomicPathPlanning(theStartDummyID,theGoalDummyID,theRobotCollectionID,
                theObstacleCollectionID,-1,thePlanningType,theAngularCoeff,theStepSize,
                theSearchMinVal,theSearchRange,theDirectionConstraints,clearanceAndMaxDistance,C3Vector(gammaAxis));
        return((CDummyHolonomicPathPlanning*)p);
    }
    // we call the plugin function:
    return((CDummyHolonomicPathPlanning*)_createHolonomicPathPlanningObject(theStartDummyID,theGoalDummyID,theRobotCollectionID,
            theObstacleCollectionID,thePlanningType,theAngularCoeff,theStepSize,
            theSearchMinVal,theSearchRange,theDirectionConstraints,clearanceAndMaxDistance,gammaAxis));
}


void CPathPlanningInterface::destroyPathPlanningObject(CDummyPathPlanning* obj)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        delete (CPathPlanning*)obj;
        return;
    }
    // we call the plugin function:
    _destroyPathPlanningObject(obj);
}

int CPathPlanningInterface::searchPath(CDummyPathPlanning* obj,int maxTimeInMsPerPass)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        return(((CPathPlanning*)obj)->searchPath(maxTimeInMsPerPass));
    }
    // we call the plugin function:
    return(_searchPath(obj,maxTimeInMsPerPass));
}

int CPathPlanningInterface::getPathNodeCount(CDummyPathPlanning* obj,char fromStart)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        if (((CPathPlanning*)obj)->isHolonomic)
        {
            if (fromStart!=0)
                return(int(((CHolonomicPathPlanning*)obj)->fromStart.size()));
            return(int(((CHolonomicPathPlanning*)obj)->fromGoal.size()));
        }
        else
        {
            if (fromStart!=0)
                return(int(((CNonHolonomicPathPlanning*)obj)->fromStart.size()));
            return(int(((CNonHolonomicPathPlanning*)obj)->fromGoal.size()));
        }
    }
    // we call the plugin function:
    return(_getPathNodeCount(obj,fromStart));
}

char CPathPlanningInterface::setPartialPath(CDummyPathPlanning* obj)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        return(((CPathPlanning*)obj)->setPartialPath());
    }
    // we call the plugin function:
    return(_setPartialPath(obj));
}

int CPathPlanningInterface::smoothFoundPath(CDummyPathPlanning* obj,int steps,int maxTimePerPass)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        return(((CPathPlanning*)obj)->smoothFoundPath(steps,maxTimePerPass));
    }
    // we call the plugin function:
    return(_smoothFoundPath(obj,steps,maxTimePerPass));
}

float* CPathPlanningInterface::getFoundPath(CDummyPathPlanning* obj,int* nodeCount)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        float* retVal=nullptr;
        std::vector<float> data;
        ((CPathPlanning*)obj)->getPathData(data);
        nodeCount[0]=(int)data.size()/7;
        if (nodeCount[0]!=0)
        {
            retVal=new float[nodeCount[0]*7];
            for (int i=0;i<nodeCount[0]*7;i++)
                retVal[i]=data[i];
        }
        return(retVal);
    }
    // we call the plugin function:
    return(_getFoundPath(obj,nodeCount));
}

float* CPathPlanningInterface::getSearchTree(CDummyPathPlanning* obj,int* segmentCount,char fromStart)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        float* retVal=nullptr;
        std::vector<float> data;
        ((CPathPlanning*)obj)->getSearchTreeData(data,fromStart!=0);
        segmentCount[0]=(int)data.size()/6;
        if (segmentCount[0]!=0)
        {
            retVal=new float[segmentCount[0]*6];
            for (int i=0;i<segmentCount[0]*6;i++)
                retVal[i]=data[i];
        }
        return(retVal);
    }
    // we call the plugin function:
    return(_getSearchTree(obj,segmentCount,fromStart));
}

void CPathPlanningInterface::releaseBuffer(void* buff)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        delete[] ((char*)buff);
        return;
    }
    // we call the plugin function:
    _releaseBuffer(buff);
}


CDummyMpObject* CPathPlanningInterface::createMpObject()
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        return((CDummyMpObject*)new CmpObject());
    }
    // we call the plugin function:
    return(_createMpObject());
}


void CPathPlanningInterface::destroyMpObject(CDummyMpObject* mpObject)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        delete (CmpObject*)mpObject;
        return;
    }
    // we call the plugin function:
    _destroyMpObject(mpObject);
}

void CPathPlanningInterface::setMpObjectData(CDummyMpObject* mpObject,int jointCnt,const int* jointHandles,const int* jointStepCount,int ikGroupId,int baseFrameId,int tipFrameId,int robotSelfCollEntity1,int robotSelfCollEntity2,int robotEntity,int obstacleEntity,float distanceThreshold,const float* tipMetricAndIkElConstraints,const float* robotMetric)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        ((CmpObject*)mpObject)->setData(jointCnt,jointHandles,jointStepCount,ikGroupId,baseFrameId,tipFrameId,robotSelfCollEntity1,robotSelfCollEntity2,robotEntity,obstacleEntity,distanceThreshold,tipMetricAndIkElConstraints,robotMetric);
        return;
    }
    // we call the plugin function:
    _setMpObjectData(mpObject,jointCnt,jointHandles,jointStepCount,ikGroupId,baseFrameId,tipFrameId,robotSelfCollEntity1,robotSelfCollEntity2,robotEntity,obstacleEntity,distanceThreshold,tipMetricAndIkElConstraints,robotMetric);
}

char CPathPlanningInterface::calculateMpNodesPhase1(CDummyMpObject* mpObject,const char* serializationData,int serializationDataSize)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        return(((CmpObject*)mpObject)->calculateNodes(serializationData,serializationDataSize));
    }
    // we call the plugin function:
    return(_calculateMpNodesPhase1(mpObject,serializationData,serializationDataSize));
}


int CPathPlanningInterface::getMpPhase1NodesRenderData(CDummyMpObject* mpObject,int index,float** pos)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        return(((CmpObject*)mpObject)->getPhase1NodesRenderData(index,pos));
    }
    // we call the plugin function:
    return(_getMpPhase1NodesRenderData(mpObject,index,pos));
}

int CPathPlanningInterface::getMpPhase2NodesRenderData(CDummyMpObject* mpObject,unsigned char what,int index,float** pos1,float** pos2)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        return(((CmpObject*)mpObject)->getPhase2NodesRenderData(what,index,pos1,pos2));
    }
    // we call the plugin function:
    return(_getMpPhase2NodesRenderData(mpObject,what,index,pos1,pos2));
}

char* CPathPlanningInterface::getMpSerializationData(CDummyMpObject* mpObject,int* cnt)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        return(((CmpObject*)mpObject)->getSerializationData(cnt));
    }
    // we call the plugin function:
    return(_getMpSerializationData(mpObject,cnt));
}

int CPathPlanningInterface::getMpPhase1NodeCnt(CDummyMpObject* mpObject,char collidingNodesOnly)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        return(((CmpObject*)mpObject)->getPhase1NodeCnt(collidingNodesOnly));
    }
    // we call the plugin function:
    return(_getMpPhase1NodeCnt(mpObject,collidingNodesOnly));
}


int CPathPlanningInterface::getMpRobotConfigFromTipPose(CDummyMpObject* mpObject,const float* tipPos,const float* tipQuat,int options,float* robotJoints,const float* constraints,const float* configConstraints,int trialCount,float tipCloseDistance,const float* referenceConfigs,int configCount,const int* jointBehaviour,int correctionPasses,int maxTimeInMs)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        return(((CmpObject*)mpObject)->getRobotConfigFromTipPose(tipPos,tipQuat,options,robotJoints,constraints,configConstraints,trialCount,tipCloseDistance,referenceConfigs,configCount,jointBehaviour,correctionPasses,maxTimeInMs));
    }
    // we call the plugin function:
    return(_getMpRobotConfigFromTipPose(mpObject,tipPos,tipQuat,options,robotJoints,constraints,configConstraints,trialCount,tipCloseDistance,referenceConfigs,configCount,jointBehaviour,correctionPasses,maxTimeInMs));
}

float* CPathPlanningInterface::mpFindPath(CDummyMpObject* mpObject,const float* startConfig,const float* goalConfig,int options,float stepSize,int* outputConfigsCnt,int maxTimeInMs,const int* auxIntParams,const float* auxFloatParams,int randomSeed)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        return(((CmpObject*)mpObject)->findPath(startConfig,goalConfig,options,stepSize,outputConfigsCnt,maxTimeInMs,auxIntParams,auxFloatParams,randomSeed));
    }
    // we call the plugin function:
    return(_mpFindPath(mpObject,startConfig,goalConfig,options,stepSize,outputConfigsCnt,maxTimeInMs,auxIntParams,auxFloatParams,randomSeed));
}


float* CPathPlanningInterface::mpFindIkPath(CDummyMpObject* mpObject,const float* startConfig,const float* goalPos,const float* goalQuat,int options,float stepSize,int* outputConfigsCnt,const int* auxIntParams,const float* auxFloatParams)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        return(((CmpObject*)mpObject)->findIkPath(startConfig,goalPos,goalQuat,options,stepSize,outputConfigsCnt,auxIntParams,auxFloatParams));
    }
    // we call the plugin function:
    return(_mpFindIkPath(mpObject,startConfig,goalPos,goalQuat,options,stepSize,outputConfigsCnt,auxIntParams,auxFloatParams));
}

float* CPathPlanningInterface::mpSimplifyPath(CDummyMpObject* mpObject,const float* pathBuffer,int configCnt,int options,float stepSize,int increment,int* outputConfigsCnt,int maxTimeInMs,const int* auxIntParams,const float* auxFloatParams)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        return(((CmpObject*)mpObject)->simplifyPath(pathBuffer,configCnt,options,stepSize,increment,outputConfigsCnt,maxTimeInMs,auxIntParams,auxFloatParams));
    }
    // we call the plugin function:
    return(_mpSimplifyPath(mpObject,pathBuffer,configCnt,options,stepSize,increment,outputConfigsCnt,maxTimeInMs,auxIntParams,auxFloatParams));
}

float* CPathPlanningInterface::mpGetConfigTransition(CDummyMpObject* mpObject,const float* startConfig,const float* goalConfig,int options,const int* select,float calcStepSize,float maxOutStepSize,int wayPointCnt,const float* wayPoints,int* outputConfigsCnt,const int* auxIntParams,const float* auxFloatParams)
{
    if (!initializeFunctionsIfNeeded())
    { // we use the internal functions
        return(((CmpObject*)mpObject)->getConfigTransition(startConfig,goalConfig,options,select,calcStepSize,maxOutStepSize,wayPointCnt,wayPoints,outputConfigsCnt,auxIntParams,auxFloatParams));
    }
    // we call the plugin function:
    return(_mpGetConfigTransition(mpObject,startConfig,goalConfig,options,select,calcStepSize,maxOutStepSize,wayPointCnt,wayPoints,outputConfigsCnt,auxIntParams,auxFloatParams));
}


float CPathPlanningInterface::getNormalizedAngle(float angle)
{ // Returns an angle between -PI and +PI
    angle=fmod(angle,6.28318531f);
    if (angle<-3.14159266f)
        angle+=6.28318531f;
    else if (angle>+3.14159266f)
        angle-=6.28318531f;
    return(angle);
}






