
#pragma once

#include <simMath/7Vector.h>

class CPathPoint_old
{
  public:
    CPathPoint_old();
    virtual ~CPathPoint_old();

    void setTransformation(const CPose& tr, int attributes);
    CPose getTransformation();
    void setMaxRelAbsVelocity(double t);
    double getMaxRelAbsVelocity();
    void setOnSpotDistance(double d);
    double getOnSpotDistance();
    void setAuxFlags(unsigned short f);
    unsigned short getAuxFlags();
    void setAuxChannels(const double c[4]);
    void getAuxChannels(double c[4]);

  protected:
    CPose _transformation;
    double _maxRelAbsVelocity;
    double _onSpotDistance;
    unsigned short _auxFlags;
    double _auxChannels[4];
};
