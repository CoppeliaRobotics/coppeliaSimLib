
#pragma once

#include "7Vector.h"

class CPathPoint  
{
public:
    CPathPoint();
    virtual ~CPathPoint(); 

    void setTransformation(const C7Vector& tr,int attributes);
    C7Vector getTransformation();
    void setMaxRelAbsVelocity(float t);
    float getMaxRelAbsVelocity();
    void setOnSpotDistance(float d);
    float getOnSpotDistance();
    void setAuxFlags(unsigned short f);
    unsigned short getAuxFlags();
    void setAuxChannels(const float c[4]);
    void getAuxChannels(float c[4]);

protected:
    C7Vector _transformation;
    float _maxRelAbsVelocity;
    float _onSpotDistance;
    unsigned short _auxFlags;
    float _auxChannels[4];
};
