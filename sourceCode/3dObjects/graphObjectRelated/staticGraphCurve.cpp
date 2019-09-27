
#include "vrepMainHeader.h"
#include "staticGraphCurve.h"


CStaticGraphCurve::CStaticGraphCurve()
{
}

CStaticGraphCurve::CStaticGraphCurve(int type,std::vector<float>* xVals,std::vector<float>* yVals,std::vector<float>* zVals)
{   
    _curveType=type;
    _linkPoints=true;
    _label=true;
    ambientColor[0]=1.0f;
    ambientColor[1]=ambientColor[2]=0.0f;
    emissiveColor[0]=emissiveColor[1]=emissiveColor[2]=0.0f;
    _name="StaticCurve";
    _curveWidth=1.0f;
    _relativeToWorld=false;
    for (int i=0;i<int(xVals->size());i++)
    {
        values.push_back((*xVals)[i]);
        values.push_back((*yVals)[i]);
        if (zVals!=nullptr)
            values.push_back((*zVals)[i]);
    }
}

CStaticGraphCurve::~CStaticGraphCurve()
{
}

void CStaticGraphCurve::setLabel(bool l)
{
    _label=l;
}

bool CStaticGraphCurve::getLabel()
{ 
    return(_label);
}

bool CStaticGraphCurve::getLinkPoints()
{
    return(_linkPoints);
}

int CStaticGraphCurve::getCurveType()
{
    return(_curveType);
}

void CStaticGraphCurve::setCurveWidth(float w)
{
    _curveWidth=w;
}

float CStaticGraphCurve::getCurveWidth()
{
    return(_curveWidth);
}

void CStaticGraphCurve::setRelativeToWorld(bool r)
{
    _relativeToWorld=r;
}

bool CStaticGraphCurve::getRelativeToWorld()
{
    return(_relativeToWorld);
}

void CStaticGraphCurve::setLinkPoints(bool l)
{
    _linkPoints=l;
}

void CStaticGraphCurve::setName(std::string theName)
{
    _name=theName;
}

std::string CStaticGraphCurve::getName() 
{ 
    return(_name); 
}

CStaticGraphCurve* CStaticGraphCurve::copyYourself()
{   // We copy everything
    CStaticGraphCurve* newObj=new CStaticGraphCurve();
    newObj->_curveType=_curveType;
    newObj->ambientColor[0]=ambientColor[0];
    newObj->ambientColor[1]=ambientColor[1];
    newObj->ambientColor[2]=ambientColor[2];
    newObj->emissiveColor[0]=emissiveColor[0];
    newObj->emissiveColor[1]=emissiveColor[1];
    newObj->emissiveColor[2]=emissiveColor[2];
    newObj->_linkPoints=_linkPoints;
    newObj->_label=_label;
    newObj->_name=_name;
    newObj->_curveWidth=_curveWidth;
    newObj->_relativeToWorld=_relativeToWorld;
    for (int i=0;i<int(values.size());i++)
        newObj->values.push_back(values[i]);
    return(newObj);
}

void CStaticGraphCurve::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Ilo");
            ar << _curveType << ambientColor[0] << ambientColor[1] << ambientColor[2] << _name << _curveWidth;
            ar.flush();

            ar.storeDataName("Il2"); // 7/10/2014
            ar << emissiveColor[0] << emissiveColor[1] << emissiveColor[2];
            ar.flush();

            ar.storeDataName("Par");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_linkPoints);
            SIM_SET_CLEAR_BIT(nothing,1,_label);
            SIM_SET_CLEAR_BIT(nothing,2,_relativeToWorld);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Val");
            ar << int(values.size());
            for (int i=0;i<int(values.size());i++)
                ar << values[i];
            ar.flush();

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
                    if (theName.compare("Ilo")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _curveType >> ambientColor[0] >> ambientColor[1] >> ambientColor[2] >> _name >> _curveWidth;
                    }
                    if (theName.compare("Il2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> emissiveColor[0] >> emissiveColor[1] >> emissiveColor[2];
                    }
                    if (theName=="Par")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _linkPoints=SIM_IS_BIT_SET(nothing,0);
                        _label=SIM_IS_BIT_SET(nothing,1);
                        _relativeToWorld=SIM_IS_BIT_SET(nothing,2);
                    }
                    if (theName=="Val")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        int count;
                        ar >> count;
                        values.clear();
                        for (int i=0;i<count;i++)
                        {
                            float dummy;
                            ar >> dummy;
                            values.push_back(dummy);
                        }
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}
