
#pragma once

#include <vector>

class CPathPlanning  
{
public:
    CPathPlanning();
    virtual ~CPathPlanning();

    virtual int searchPath(int maxTimePerPass);
    virtual bool setPartialPath();
    virtual int smoothFoundPath(int steps,int maxTimePerPass);

    virtual void getPathData(std::vector<float>& data);
    virtual void getSearchTreeData(std::vector<float>& data,bool fromStart);
    char isHolonomic;

protected:  
    virtual bool doCollide(float* dist);

    int robotCollectionID;
    int obstacleCollectionID;
    bool _allIsObstacle;
    bool invalidData;
    bool firstPass;
    float obstacleClearanceAndMaxDistance[2];
    int buffer[4];
};
