
#pragma once

#include "vrepMainHeader.h"
#include "mainCont.h"

class CCacheCont : CMainCont  
{
public:
    CCacheCont();
    virtual ~CCacheCont();
    void getCacheDataDist(int entity1,int entity2,int cache[4]);
    void setCacheDataDist(int entity1,int entity2,int cache[4]);
    void clearCache();

protected:
    void _getCacheData(int entity1,int entity2,int cache[4],std::vector<int>& entityPairs,std::vector<int>& cacheDat);
    void _setCacheData(int entity1,int entity2,int cache[4],std::vector<int>& entityPairs,std::vector<int>& cacheDat);
    int _getPosition(int entity1,int entity2,bool& inverted,std::vector<int>& entityPairs,std::vector<int>& cacheDat);
    std::vector<int> entityPairIDs_dist;
    std::vector<int> cacheData_dist;
};
