
#pragma once

#include "ser.h"
#include "dummyClasses.h"
#include "3Vector.h"

class CPathPlanningTask  
{
public:
    CPathPlanningTask();
    CPathPlanningTask(int theID,int thePathPlanningType);
    virtual ~CPathPlanningTask();

    void renderYour3DStuff();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void simulationAboutToStart();
    void simulationEnded();

    CPathPlanningTask* copyYourself();
    void setDefaultValues();
    void serialize(CSer& ar);
    void performObjectLoadingMapping(const std::vector<int>* map);
    void performCollectionLoadingMapping(const std::vector<int>* map);
    bool announceObjectWillBeErased(int objID,bool copyBuffer);
    bool announceCollectionWillBeErased(int groupID,bool copyBuffer);
    int getObjectID();
    void setObjectID(int newID);
    std::string getObjectName();
    void setObjectName(std::string newName);
    bool isTaskValid();
    bool performSearch(bool showProgressDlg,float maxTime);
    bool initiateSteppedSearch(bool showProgressDlg,float maxTime,float subDt);
    int performSteppedSearch();

    void setShowSearchTrees(bool s);
    bool getShowSearchTrees();
    void getAndDisconnectSearchTrees(int& tree1Handle,int& tree2Handle);
    void connectExternalSearchTrees(int tree1Handle,int tree2Handle);

    void setGoalDummyID(int theID);
    void setPathID(int theID);
    void setHolonomicTaskType(int type);
    void setStepSize(float size);
    void setAngularStepSize(float step);
    void setSearchRange(float searchMin[4],float searchR[4]);
    void setMaxTime(float mTime);
    void setSearchDirection(int dir[4]);
    void setCollisionDetection(bool c);
    void setObstacleClearance(float c);
    void setMinTurningCircleDiameter(float d);

    int getGoalDummyID();
    int getPathID();
    void setRobotEntityID(int theID);
    int getRobotEntityID();
    void setObstacleEntityID(int theID);
    int getObstacleEntityID();

    void setObstacleMaxDistance(float c);
    float getObstacleMaxDistance();
    void setObstacleMaxDistanceEnabled(bool e);
    bool getObstacleMaxDistanceEnabled();


    int getHolonomicTaskType();
    float getStepSize();
    float getAngularStepSize();
    void getSearchRange(float searchMin[4],float searchR[4]);
    float getMaxTime();
    void getSearchDirection(int dir[4]);
    bool getCollisionDetection();
    float getObstacleClearance();
    int getPathPlanningType();
    int getStartDummyID();
    float getMinTurningCircleDiameter();

    void setPostProcessingPassCount(int p);
    int getPostProcessingPassCount();
    void setPartialPathIsOk(bool ok);
    bool getPartialPathIsOk();
    void setGammaAxis(const C3Vector& axis);
    C3Vector getGammaAxis();

    void setVisualizeSearchArea(bool v);
    bool getVisualizeSearchArea();

    void setOriginalTask(CPathPlanningTask* originalTask);
    CPathPlanningTask* getOriginalTask();

protected:
    CPathPlanningTask* _originalTask_useWhenCopied;
    int _steppedSearchTemp_maxTimeInMs;
    int _steppedSearchTemp_initTimeInMs;
    int _steppedSearchTemp_maxSubTimeInMs;
    bool _steppedSearchTemp_showProgressDlg;
    void* _steppedSearchTemp_theTask;
    int _steppedSearchTemp_foundPathStatus; // 0=not yet, 1=partial, 2=full
    int _steppedSearchTemp_currentSmoothingPass;

    bool _partialPathIsOk;
    bool _showSearchTrees;
    int _searchTree1Handle;
    int _searchTree2Handle;
    int _postProcessingPassCount;
    int pathPlanningType;
    int _startDummyID;
    int _goalDummyID;
    int _robotEntityID;
    int _obstacleEntityID;
    int _pathID;
    int holonomicTaskType;
    float stepSize;
    float angularStepSize;
    float searchMinValue[4];
    float searchRange[4];
    int searchDirection[4];
    C3Vector _gammaAxis;

    float maximumTime;
    int _objectID;
    bool collisionDetection;
    bool _visualizeSearchArea;
    float obstacleClearance;
    float _obstacleMaxDistance;
    bool _obstacleMaxDistanceEnabled;
    float minTurningCircleDiameter;
    std::string objectName;
};
