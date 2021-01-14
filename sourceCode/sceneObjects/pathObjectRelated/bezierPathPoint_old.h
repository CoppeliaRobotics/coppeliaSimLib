#pragma once

#include "pathPoint_old.h"

class CBezierPathPoint_old : public CPathPoint_old
{
public:
    CBezierPathPoint_old(const C7Vector& transf);
    virtual ~CBezierPathPoint_old();

    CBezierPathPoint_old* copyYourself();

    float cumulativeLength;
    float segmentLength;
    float cumulativeAngle;
    float segmentAngleVariation;
    float cumulativeOnSpotDistance;
    float segmentOnSpotDistance;
    float virtualSegmentLength;
    float virtualCumulativeLength;
};
