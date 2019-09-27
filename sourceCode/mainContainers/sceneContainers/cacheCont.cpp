
#include "vrepMainHeader.h"
#include "cacheCont.h"

CCacheCont::CCacheCont()
{
}

CCacheCont::~CCacheCont()
{
}

int CCacheCont::_getPosition(int entity1,int entity2,bool& inverted,std::vector<int>& entityPairs,std::vector<int>& cacheDat)
{
    inverted=false;
    int position=-1;
    // 1. We search for an already existing entry:
    for (int i=0;i<int(entityPairs.size())/2;i++)
    {
        if ( (entityPairs[2*i+0]==entity1)&&(entityPairs[2*i+1]==entity2) )
        {
            position=i;
            break;
        }
        else if ( (entityPairs[2*i+0]==entity2)&&(entityPairs[2*i+1]==entity1) )
        {
            position=i;
            inverted=true;
            break;
        }
    }
    // 2. we add an entry if necessary:
    if (position==-1)
    {
        entityPairs.push_back(entity1);
        entityPairs.push_back(entity2);
        cacheDat.push_back(-1);
        cacheDat.push_back(-1);
        cacheDat.push_back(-1);
        cacheDat.push_back(-1);
        position=int(entityPairs.size()/2)-1;
    }
    return(position);
}

void CCacheCont::_getCacheData(int entity1,int entity2,int cache[4],std::vector<int>& entityPairs,std::vector<int>& cacheDat)
{
    bool inverted=false;
    int pos=_getPosition(entity1,entity2,inverted,entityPairs,cacheDat);
    if (!inverted)
    {
        cache[0]=cacheDat[4*pos+0];
        cache[1]=cacheDat[4*pos+1];
        cache[2]=cacheDat[4*pos+2];
        cache[3]=cacheDat[4*pos+3];
    }
    else
    {
        cache[2]=cacheDat[4*pos+0];
        cache[3]=cacheDat[4*pos+1];
        cache[0]=cacheDat[4*pos+2];
        cache[1]=cacheDat[4*pos+3];
    }
}

void CCacheCont::_setCacheData(int entity1,int entity2,int cache[4],std::vector<int>& entityPairs,std::vector<int>& cacheDat)
{
    bool inverted=false;
    int pos=_getPosition(entity1,entity2,inverted,entityPairs,cacheDat);
    if (!inverted)
    {
        cacheDat[4*pos+0]=cache[0];
        cacheDat[4*pos+1]=cache[1];
        cacheDat[4*pos+2]=cache[2];
        cacheDat[4*pos+3]=cache[3];
    }
    else
    {
        cacheDat[4*pos+0]=cache[2];
        cacheDat[4*pos+1]=cache[3];
        cacheDat[4*pos+2]=cache[0];
        cacheDat[4*pos+3]=cache[1];
    }
}

void CCacheCont::getCacheDataDist(int entity1,int entity2,int cache[4])
{
    _getCacheData(entity1,entity2,cache,entityPairIDs_dist,cacheData_dist);
}
void CCacheCont::setCacheDataDist(int entity1,int entity2,int cache[4])
{
    _setCacheData(entity1,entity2,cache,entityPairIDs_dist,cacheData_dist);
}

void CCacheCont::clearCache()
{
    entityPairIDs_dist.clear();
    cacheData_dist.clear();
}
