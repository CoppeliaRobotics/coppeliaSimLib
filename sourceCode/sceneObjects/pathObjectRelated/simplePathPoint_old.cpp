#include "simplePathPoint_old.h"
#include "tt.h"
#include "simInternal.h"

CSimplePathPoint_old::CSimplePathPoint_old()
{
    commonInit();
}

CSimplePathPoint_old::~CSimplePathPoint_old()
{
}

void CSimplePathPoint_old::commonInit()
{
    _transformation.setIdentity();
    _bezierFactorBefore=0.5f;
    _bezierFactorAfter=0.5f;
    _maxRelAbsVelocity=1.0f;
    _bezierPointCount=15;
    _onSpotDistance=0.0f;
    _auxFlags=0;
    for (int i=0;i<4;i++)
        _auxChannels[i]=0.0f;
}

void CSimplePathPoint_old::setBezierFactors(floatDouble fBefore,floatDouble fAfter)
{ // 0.99f is so that we don't have dedoubled bezier points!
    tt::limitValue(0.1f,0.99f,fBefore);
    tt::limitValue(0.1f,0.99f,fAfter);
    _bezierFactorBefore=fBefore;
    _bezierFactorAfter=fAfter;
}

void CSimplePathPoint_old::getBezierFactors(floatDouble& fBefore,floatDouble& fAfter)
{
    fBefore=_bezierFactorBefore;
    fAfter=_bezierFactorAfter;
}

void CSimplePathPoint_old::setBezierPointCount(int c)
{
    c=tt::getLimitedInt(1,100,c);
    if (c<3)
        c=1;
    _bezierPointCount=c;
}

int CSimplePathPoint_old::getBezierPointCount()
{
    return(_bezierPointCount);
}

void CSimplePathPoint_old::scaleYourself(floatDouble scalingFactor)
{
    _transformation.X=_transformation.X*scalingFactor;
}

void CSimplePathPoint_old::scaleYourselfNonIsometrically(floatDouble x,floatDouble y,floatDouble z)
{
    _transformation.X(0)=_transformation.X(0)*x;
    _transformation.X(1)=_transformation.X(1)*y;
    _transformation.X(2)=_transformation.X(2)*z;
}


CSimplePathPoint_old* CSimplePathPoint_old::copyYourself()
{ // Everything is copied.
    CSimplePathPoint_old* newPoint=new CSimplePathPoint_old();
    newPoint->_transformation=_transformation;
    newPoint->_bezierFactorBefore=_bezierFactorBefore;
    newPoint->_bezierFactorAfter=_bezierFactorAfter;
    newPoint->_maxRelAbsVelocity=_maxRelAbsVelocity;
    newPoint->_bezierPointCount=_bezierPointCount;
    newPoint->_onSpotDistance=_onSpotDistance;
    newPoint->_auxFlags=_auxFlags;
    for (int i=0;i<4;i++)
        newPoint->_auxChannels[i]=_auxChannels[i];
    return(newPoint);
}

void CSimplePathPoint_old::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
#ifdef TMPOPERATION
            ar.storeDataName("At2");
            ar.flt() << (floatFloat)_transformation(0) << (floatFloat)_transformation(1) << (floatFloat)_transformation(2) << (floatFloat)_transformation(3);
            ar.flt() << (floatFloat)_transformation(4) << (floatFloat)_transformation(5) << (floatFloat)_transformation(6);
            ar.flt() << (floatFloat)_bezierFactorBefore << (floatFloat)_bezierFactorAfter;
            ar << _bezierPointCount;
            ar.flt() << (floatFloat)_maxRelAbsVelocity;
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_t2");
            ar.dbl() << _transformation(0) << _transformation(1) << _transformation(2) << _transformation(3);
            ar.dbl() << _transformation(4) << _transformation(5) << _transformation(6);
            ar.dbl() << _bezierFactorBefore << _bezierFactorAfter;
            ar << _bezierPointCount;
            ar.dbl() << _maxRelAbsVelocity;
            ar.flush();
#endif

#ifdef TMPOPERATION
            ar.storeDataName("At3");
            ar.flt() << (floatFloat)_onSpotDistance;
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_t3");
            ar.dbl() << _onSpotDistance;
            ar.flush();
#endif

#ifdef TMPOPERATION
            ar.storeDataName("At4");
            ar << _auxFlags;
            ar.flt() << (floatFloat)_auxChannels[0];
            ar.flt() << (floatFloat)_auxChannels[1];
            ar.flt() << (floatFloat)_auxChannels[2];
            ar.flt() << (floatFloat)_auxChannels[3];
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_t4");
            ar << _auxFlags;
            ar.dbl() << _auxChannels[0];
            ar.dbl() << _auxChannels[1];
            ar.dbl() << _auxChannels[2];
            ar.dbl() << _auxChannels[3];
            ar.flush();
#endif

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("At2")==0)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        floatFloat bla[7];
                        ar.flt() >> bla[0] >> bla[1] >> bla[2] >> bla[3];
                        ar.flt() >> bla[4] >> bla[5] >> bla[6];
                        for (size_t i=0;i<7;i++)
                            _transformation(i)=(floatDouble)bla[i];
                        ar.flt() >> bla[0] >> bla[1];
                        _bezierFactorBefore=(floatDouble)bla[0];
                        _bezierFactorAfter=(floatDouble)bla[1];
                        ar >> _bezierPointCount;
                        ar.flt() >> bla[0];
                        _maxRelAbsVelocity=(floatDouble)bla[0];
                    }
                    if (theName.compare("_t2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar.dbl() >> _transformation(0) >> _transformation(1) >> _transformation(2) >> _transformation(3);
                        ar.dbl() >> _transformation(4) >> _transformation(5) >> _transformation(6);
                        ar.dbl() >> _bezierFactorBefore >> _bezierFactorAfter;
                        ar >> _bezierPointCount;
                        ar.dbl() >> _maxRelAbsVelocity;
                    }
                    if (theName.compare("At3")==0)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        floatFloat bla;
                        ar.flt() >> bla;
                        _onSpotDistance=(floatDouble)bla;
                    }
                    if (theName.compare("_t3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar.dbl() >> _onSpotDistance;
                    }
                    if (theName.compare("At4")==0)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _auxFlags;
                        floatFloat bla;
                        for (size_t i=0;i<4;i++)
                        {
                            ar.flt() >> bla;
                            _auxChannels[i]=(floatDouble)bla;
                        }
                    }
                    if (theName.compare("_t4")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _auxFlags;
                        for (size_t i=0;i<4;i++)
                            ar.dbl() >> _auxChannels[i];
                    }

                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            ar.xmlPushNewNode("localFrame");
            ar.xmlAddNode_floats("position",_transformation.X.data,3);
            if (exhaustiveXml)
                ar.xmlAddNode_floats("quaternion",_transformation.Q.data,4);
            else
            {
                C3Vector euler(_transformation.Q.getEulerAngles());
                euler*=180.0f/piValue;
                ar.xmlAddNode_floats("euler",euler.data,3);
            }
            ar.xmlPopNode();

            ar.xmlPushNewNode("bezierPoints");
            ar.xmlAddNode_int("count",_bezierPointCount);
            ar.xmlAddNode_2float("factors",_bezierFactorBefore,_bezierFactorAfter);
            ar.xmlPopNode();

            ar.xmlAddNode_float("virtualDistance",_onSpotDistance);

            ar.xmlAddNode_int("auxiliaryFlags",_auxFlags);

            ar.xmlAddNode_floats("auxiliaryChannels",_auxChannels,4);
        }
        else
        {
            if (ar.xmlPushChildNode("localFrame",exhaustiveXml))
            {
                ar.xmlGetNode_floats("position",_transformation.X.data,3,exhaustiveXml);
                if (exhaustiveXml)
                {
                    ar.xmlGetNode_floats("quaternion",_transformation.Q.data,4);
                    _transformation.Q.normalize(); // just in case
                }
                else
                {
                    C3Vector euler;
                    if (ar.xmlGetNode_floats("euler",euler.data,3,exhaustiveXml))
                    {
                        euler(0)*=piValue/180.0f;
                        euler(1)*=piValue/180.0f;
                        euler(2)*=piValue/180.0f;
                        _transformation.Q.setEulerAngles(euler);
                    }
                }
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("bezierPoints",exhaustiveXml))
            {
                ar.xmlGetNode_int("count",_bezierPointCount,exhaustiveXml);
                ar.xmlGetNode_2float("factors",_bezierFactorBefore,_bezierFactorAfter,exhaustiveXml);
                ar.xmlPopNode();
            }

            ar.xmlGetNode_float("virtualDistance",_onSpotDistance,exhaustiveXml);

            int tmp;
            ar.xmlGetNode_int("auxiliaryFlags",tmp,exhaustiveXml);
            _auxFlags=(unsigned short)tmp;

            ar.xmlGetNode_floats("auxiliaryChannels",_auxChannels,4,exhaustiveXml);
        }
    }
}
