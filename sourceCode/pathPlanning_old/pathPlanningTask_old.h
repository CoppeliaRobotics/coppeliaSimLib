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
    void performObjectLoadingMapping(const std::map<int,int>* map);
    void performCollectionLoadingMapping(const std::map<int,int>* map);
    bool announceObjectWillBeErased(int objID,bool copyBuffer);
    bool announceCollectionWillBeErased(int groupID,bool copyBuffer);
    int getObjectID();
    void setObjectID(int newID);
    std::string getObjectName();
    void setObjectName(std::string newName);
    bool isTaskValid();
    bool performSearch(bool showProgressDlg,double maxTime);
    bool initiateSteppedSearch(bool showProgressDlg,double maxTime,double subDt);
    int performSteppedSearch();

    void setShowSearchTrees(bool s);
    bool getShowSearchTrees();
    void getAndDisconnectSearchTrees(int& tree1Handle,int& tree2Handle);
    void connectExternalSearchTrees(int tree1Handle,int tree2Handle);

    void setGoalDummyID(int theID);
    void setPathID(int theID);
    void setHolonomicTaskType(int type);
    void setStepSize(double size);
    void setAngularStepSize(double step);
    void setSearchRange(double searchMin[4],double searchR[4]);
    void setMaxTime(double mTime);
    void setSearchDirection(int dir[4]);
    void setCollisionDetection(bool c);
    void setObstacleClearance(double c);
    void setMinTurningCircleDiameter(double d);

    int getGoalDummyID();
    int getPathID();
    void setRobotEntityID(int theID);
    int getRobotEntityID();
    void setObstacleEntityID(int theID);
    int getObstacleEntityID();

    void setObstacleMaxDistance(double c);
    double getObstacleMaxDistance();
    void setObstacleMaxDistanceEnabled(bool e);
    bool getObstacleMaxDistanceEnabled();


    int getHolonomicTaskType();
    double getStepSize();
    double getAngularStepSize();
    void getSearchRange(double searchMin[4],double searchR[4]);
    double getMaxTime();
    void getSearchDirection(int dir[4]);
    bool getCollisionDetection();
    double getObstacleClearance();
    int getPathPlanningType();
    int getStartDummyID();
    double getMinTurningCircleDiameter();

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
    double stepSize;
    double angularStepSize;
    double searchMinValue[4];
    double searchRange[4];
    int searchDirection[4];
    C3Vector _gammaAxis;

    double maximumTime;
    int _objectID;
    bool collisionDetection;
    bool _visualizeSearchArea;
    double obstacleClearance;
    double _obstacleMaxDistance;
    bool _obstacleMaxDistanceEnabled;
    double minTurningCircleDiameter;
    std::string objectName;
};
