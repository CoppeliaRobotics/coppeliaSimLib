#pragma once

#include <pathPoint_old.h>

class CBezierPathPoint_old : public CPathPoint_old
{
  public:
    CBezierPathPoint_old(const CPose& transf);
    virtual ~CBezierPathPoint_old();

    CBezierPathPoint_old* copyYourself();

    double cumulativeLength;
    double segmentLength;
    double cumulativeAngle;
    double segmentAngleVariation;
    double cumulativeOnSpotDistance;
    double segmentOnSpotDistance;
    double virtualSegmentLength;
    double virtualCumulativeLength;
};
