
#pragma once

#include "7Vector.h"

class CPathPoint_old
{
public:
    CPathPoint_old();
    virtual ~CPathPoint_old();

    void setTransformation(const C7Vector& tr,int attributes);
    C7Vector getTransformation();
    void setMaxRelAbsVelocity(floatDouble t);
    floatDouble getMaxRelAbsVelocity();
    void setOnSpotDistance(floatDouble d);
    floatDouble getOnSpotDistance();
    void setAuxFlags(unsigned short f);
    unsigned short getAuxFlags();
    void setAuxChannels(const floatDouble c[4]);
    void getAuxChannels(floatDouble c[4]);

protected:
    C7Vector _transformation;
    floatDouble _maxRelAbsVelocity;
    floatDouble _onSpotDistance;
    unsigned short _auxFlags;
    floatDouble _auxChannels[4];
};
