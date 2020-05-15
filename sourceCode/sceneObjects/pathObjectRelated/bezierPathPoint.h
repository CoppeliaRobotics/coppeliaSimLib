
#pragma once

#include "pathPoint.h"

class CBezierPathPoint : public CPathPoint
{
public:
    CBezierPathPoint(const C7Vector& transf);
    virtual ~CBezierPathPoint();

    CBezierPathPoint* copyYourself();

    float cumulativeLength;
    float segmentLength;
    float cumulativeAngle;
    float segmentAngleVariation;
    float cumulativeOnSpotDistance;
    float segmentOnSpotDistance;
    float virtualSegmentLength;
    float virtualCumulativeLength;
};
