#include "pathPoint_old.h"
#include "simInternal.h"
#include "tt.h"

CPathPoint_old::CPathPoint_old()
{
}

CPathPoint_old::~CPathPoint_old()
{
}

void CPathPoint_old::setTransformation(const C7Vector& tr,int attributes)
{
    _transformation=tr;
    if ((attributes&sim_pathproperty_flat_path)!=0)
        _transformation.X(2)=0.0f;
}

C7Vector CPathPoint_old::getTransformation()
{
    return(_transformation);
}

void CPathPoint_old::setMaxRelAbsVelocity(float t)
{
    tt::limitValue(0.0f,1.0f,t);
    _maxRelAbsVelocity=t;
}

float CPathPoint_old::getMaxRelAbsVelocity()
{
    return(_maxRelAbsVelocity);
}

void CPathPoint_old::setOnSpotDistance(float d)
{
    tt::limitValue(0.0f,1000.0f,d);
    _onSpotDistance=d;
}

float CPathPoint_old::getOnSpotDistance()
{
    return(_onSpotDistance);
}

void CPathPoint_old::setAuxFlags(unsigned short f)
{
    _auxFlags=f;
}

unsigned short CPathPoint_old::getAuxFlags()
{
    return(_auxFlags);
}

void CPathPoint_old::setAuxChannels(const float c[4])
{
    for (int i=0;i<4;i++)
        _auxChannels[i]=c[i];
}

void CPathPoint_old::getAuxChannels(float c[4])
{
    for (int i=0;i<4;i++)
        c[i]=_auxChannels[i];
}
