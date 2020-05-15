#include "pathPoint.h"
#include "simInternal.h"
#include "tt.h"

CPathPoint::CPathPoint()
{
}

CPathPoint::~CPathPoint()
{
}

void CPathPoint::setTransformation(const C7Vector& tr,int attributes)
{
    _transformation=tr;
    if ((attributes&sim_pathproperty_flat_path)!=0)
        _transformation.X(2)=0.0f;
}

C7Vector CPathPoint::getTransformation()
{
    return(_transformation);
}

void CPathPoint::setMaxRelAbsVelocity(float t)
{
    tt::limitValue(0.0f,1.0f,t);
    _maxRelAbsVelocity=t;
}

float CPathPoint::getMaxRelAbsVelocity()
{
    return(_maxRelAbsVelocity);
}

void CPathPoint::setOnSpotDistance(float d)
{
    tt::limitValue(0.0f,1000.0f,d);
    _onSpotDistance=d;
}

float CPathPoint::getOnSpotDistance()
{
    return(_onSpotDistance);
}

void CPathPoint::setAuxFlags(unsigned short f)
{
    _auxFlags=f;
}

unsigned short CPathPoint::getAuxFlags()
{
    return(_auxFlags);
}

void CPathPoint::setAuxChannels(const float c[4])
{
    for (int i=0;i<4;i++)
        _auxChannels[i]=c[i];
}

void CPathPoint::getAuxChannels(float c[4])
{
    for (int i=0;i<4;i++)
        c[i]=_auxChannels[i];
}
