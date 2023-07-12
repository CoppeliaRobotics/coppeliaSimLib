#pragma once

#include <pathPoint_old.h>
#include <ser.h>

class CSimplePathPoint_old : public CPathPoint_old
{
public:
    CSimplePathPoint_old();
    virtual ~CSimplePathPoint_old();

    void commonInit();
    void serialize(CSer& ar);
    CSimplePathPoint_old* copyYourself();
    void scaleYourself(double scalingFactor);
    void scaleYourselfNonIsometrically(double x,double y,double z);

    void setBezierFactors(double fBefore,double fAfter);
    void getBezierFactors(double& fBefore,double& fAfter);
    void setBezierPointCount(int c);
    int getBezierPointCount();

protected:

    double _bezierFactorBefore;
    double _bezierFactorAfter;
    int _bezierPointCount;
};
