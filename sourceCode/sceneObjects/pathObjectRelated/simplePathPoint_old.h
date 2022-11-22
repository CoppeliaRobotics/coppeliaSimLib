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
    void scaleYourself(floatDouble scalingFactor);
    void scaleYourselfNonIsometrically(floatDouble x,floatDouble y,floatDouble z);

    void setBezierFactors(floatDouble fBefore,floatDouble fAfter);
    void getBezierFactors(floatDouble& fBefore,floatDouble& fAfter);
    void setBezierPointCount(int c);
    int getBezierPointCount();

protected:

    floatDouble _bezierFactorBefore;
    floatDouble _bezierFactorAfter;
    int _bezierPointCount;
};
