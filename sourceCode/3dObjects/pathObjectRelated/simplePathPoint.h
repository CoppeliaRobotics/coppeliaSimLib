
#pragma once

#include "pathPoint.h"
#include "ser.h"

class CSimplePathPoint : public CPathPoint 
{
public:
    CSimplePathPoint();
    virtual ~CSimplePathPoint(); 

    void commonInit();
    void serialize(CSer& ar);
    CSimplePathPoint* copyYourself();
    void scaleYourself(float scalingFactor);
    void scaleYourselfNonIsometrically(float x,float y,float z);

    void setBezierFactors(float fBefore,float fAfter);
    void getBezierFactors(float& fBefore,float& fAfter);
    void setBezierPointCount(int c);
    int getBezierPointCount();

protected:

    float _bezierFactorBefore;
    float _bezierFactorAfter;
    int _bezierPointCount;
};
