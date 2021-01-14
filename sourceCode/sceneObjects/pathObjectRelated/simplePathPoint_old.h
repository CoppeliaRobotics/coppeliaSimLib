
#pragma once

#include "pathPoint_old.h"
#include "ser.h"

class CSimplePathPoint_old : public CPathPoint_old
{
public:
    CSimplePathPoint_old();
    virtual ~CSimplePathPoint_old();

    void commonInit();
    void serialize(CSer& ar);
    CSimplePathPoint_old* copyYourself();
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
