#include "staticGraphCurve_old.h"

CStaticGraphCurve_old::CStaticGraphCurve_old()
{
}

CStaticGraphCurve_old::CStaticGraphCurve_old(int type,std::vector<floatDouble>* xVals,std::vector<floatDouble>* yVals,std::vector<floatDouble>* zVals)
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

CStaticGraphCurve_old::~CStaticGraphCurve_old()
{
}

void CStaticGraphCurve_old::setLabel(bool l)
{
    _label=l;
}

bool CStaticGraphCurve_old::getLabel()
{ 
    return(_label);
}

bool CStaticGraphCurve_old::getLinkPoints()
{
    return(_linkPoints);
}

int CStaticGraphCurve_old::getCurveType()
{
    return(_curveType);
}

void CStaticGraphCurve_old::setCurveWidth(floatDouble w)
{
    _curveWidth=w;
}

floatDouble CStaticGraphCurve_old::getCurveWidth()
{
    return(_curveWidth);
}

void CStaticGraphCurve_old::setRelativeToWorld(bool r)
{
    _relativeToWorld=r;
}

bool CStaticGraphCurve_old::getRelativeToWorld()
{
    return(_relativeToWorld);
}

void CStaticGraphCurve_old::setLinkPoints(bool l)
{
    _linkPoints=l;
}

void CStaticGraphCurve_old::setName(std::string theName)
{
    _name=theName;
}

std::string CStaticGraphCurve_old::getName() 
{ 
    return(_name); 
}

CStaticGraphCurve_old* CStaticGraphCurve_old::copyYourself()
{   // We copy everything
    CStaticGraphCurve_old* newObj=new CStaticGraphCurve_old();
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

void CStaticGraphCurve_old::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
#ifdef TMPOPERATION
            ar.storeDataName("Ilo");
            ar << _curveType;
            ar.flt() << (floatFloat)ambientColor[0] << (floatFloat)ambientColor[1] << (floatFloat)ambientColor[2];
            ar << _name;
            ar.flt() << (floatFloat)_curveWidth;
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_lo");
            ar << _curveType;
            ar.dbl() << ambientColor[0] << ambientColor[1] << ambientColor[2];
            ar << _name;
            ar.dbl() << _curveWidth;
            ar.flush();
#endif

#ifdef TMPOPERATION
            ar.storeDataName("Il2"); // 7/10/2014
            ar.flt() << (floatFloat)emissiveColor[0] << (floatFloat)emissiveColor[1] << (floatFloat)emissiveColor[2];
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_l2"); // 7/10/2014
            ar.dbl() << emissiveColor[0] << emissiveColor[1] << emissiveColor[2];
            ar.flush();
#endif

            ar.storeDataName("Par");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_linkPoints);
            SIM_SET_CLEAR_BIT(nothing,1,_label);
            SIM_SET_CLEAR_BIT(nothing,2,_relativeToWorld);
            ar << nothing;
            ar.flush();

#ifdef TMPOPERATION
            ar.storeDataName("Val");
            ar << int(values.size());
            for (int i=0;i<int(values.size());i++)
                ar.flt() << (floatFloat)values[i];
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_al");
            ar << int(values.size());
            for (int i=0;i<int(values.size());i++)
                ar.dbl() << values[i];
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
                    if (theName.compare("Ilo")==0)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _curveType;
                        floatFloat bla,bli,blo;
                        ar.flt() >> bla >> bli >> blo;
                        ambientColor[0]=(floatDouble)bla;
                        ambientColor[1]=(floatDouble)bli;
                        ambientColor[2]=(floatDouble)blo;
                        ar >> _name;
                        ar.flt() >> bla;
                        _curveWidth=(floatDouble)bla;
                    }
                    if (theName.compare("_lo")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _curveType;
                        ar.dbl() >> ambientColor[0] >> ambientColor[1] >> ambientColor[2];
                        ar >> _name;
                        ar.dbl() >> _curveWidth;
                    }
                    if (theName.compare("Il2")==0)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        floatFloat bla,bli,blo;
                        ar.flt() >> bla >> bli >> blo;
                        emissiveColor[0]=(floatDouble)bla;
                        emissiveColor[1]=(floatDouble)bli;
                        emissiveColor[2]=(floatDouble)blo;
                    }
                    if (theName.compare("_l2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar.dbl() >> emissiveColor[0] >> emissiveColor[1] >> emissiveColor[2];
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
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        int count;
                        ar >> count;
                        values.clear();
                        for (int i=0;i<count;i++)
                        {
                            floatFloat dummy;
                            ar.flt() >> dummy;
                            values.push_back((floatDouble)dummy);
                        }
                    }
                    if (theName=="_al")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        int count;
                        ar >> count;
                        values.clear();
                        for (int i=0;i<count;i++)
                        {
                            floatDouble dummy;
                            ar.dbl() >> dummy;
                            values.push_back(dummy);
                        }
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        if (ar.isStoring())
        {
            ar.xmlAddNode_string("name",_name.c_str());

            ar.xmlAddNode_int("type",_curveType);

            ar.xmlAddNode_float("curveSize",_curveWidth);

            ar.xmlAddNode_floats("color",ambientColor,3);

            ar.xmlAddNode_floats("emissiveColor",emissiveColor,3);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("linkPoints",_linkPoints);
            ar.xmlAddNode_bool("showLabel",_label);
            ar.xmlAddNode_bool("relativeToWorld",_relativeToWorld);
            ar.xmlPopNode();

            ar.xmlAddNode_floats("values",values);
        }
        else
        {
            ar.xmlGetNode_string("name",_name);

            ar.xmlGetNode_int("type",_curveType);

            ar.xmlGetNode_float("curveSize",_curveWidth);

            ar.xmlGetNode_floats("color",ambientColor,3);
            ar.xmlGetNode_floats("emissiveColor",emissiveColor,3);

            if (ar.xmlPushChildNode("switches"))
            {
                ar.xmlGetNode_bool("linkPoints",_linkPoints);
                ar.xmlGetNode_bool("showLabel",_label);
                ar.xmlGetNode_bool("relativeToWorld",_relativeToWorld);
                ar.xmlPopNode();
            }

            ar.xmlGetNode_floats("values",values);
        }
    }
}
