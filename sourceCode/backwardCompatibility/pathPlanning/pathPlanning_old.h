#pragma once

#include <simTypes.h>
#include <vector>

class CPathPlanning_old
{
public:
    CPathPlanning_old();
    virtual ~CPathPlanning_old();

    virtual int searchPath(int maxTimePerPass);
    virtual bool setPartialPath();
    virtual int smoothFoundPath(int steps,int maxTimePerPass);

    virtual void getPathData(std::vector<double>& data);
    virtual void getSearchTreeData(std::vector<double>& data,bool fromStart);
    char isHolonomic;

protected:  
    virtual bool doCollide(double* dist);

    int robotCollectionID;
    int obstacleCollectionID;
    bool _allIsObstacle;
    bool invalidData;
    bool firstPass;
    double obstacleClearanceAndMaxDistance[2];
    int buffer[4];
};
