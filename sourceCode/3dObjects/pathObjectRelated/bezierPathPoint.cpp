
#include "vrepMainHeader.h"
#include "bezierPathPoint.h"

CBezierPathPoint::CBezierPathPoint(const C7Vector& transf)
{
    _transformation=transf;

    cumulativeLength=0.0f;
    segmentLength=0.0f;
    virtualSegmentLength=0.0f;
    virtualCumulativeLength=0.0f;
    _maxRelAbsVelocity=1.0f;
    _onSpotDistance=0.0f;
    cumulativeOnSpotDistance=0.0f;
    segmentOnSpotDistance=0.0f;

}

CBezierPathPoint::~CBezierPathPoint()
{
}

CBezierPathPoint* CBezierPathPoint::copyYourself()
{ // Everything is copied.
    CBezierPathPoint* newPoint=new CBezierPathPoint(_transformation);

    newPoint->cumulativeLength=cumulativeLength;
    newPoint->segmentLength=segmentLength;

    newPoint->cumulativeAngle=cumulativeAngle;
    newPoint->segmentAngleVariation=segmentAngleVariation;

    newPoint->cumulativeOnSpotDistance=cumulativeOnSpotDistance;
    newPoint->segmentOnSpotDistance=segmentOnSpotDistance;

    newPoint->virtualCumulativeLength=virtualCumulativeLength;
    newPoint->virtualSegmentLength=virtualSegmentLength;

    newPoint->_maxRelAbsVelocity=_maxRelAbsVelocity;
    newPoint->_onSpotDistance=_onSpotDistance;

    newPoint->_auxFlags=_auxFlags;
    for (int i=0;i<4;i++)
        newPoint->_auxChannels[i]=_auxChannels[i];

    return(newPoint);
}
