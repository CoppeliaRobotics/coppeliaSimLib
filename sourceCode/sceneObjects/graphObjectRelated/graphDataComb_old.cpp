#include <simInternal.h>
#include <graphDataComb_old.h>
#include <global.h>
#include <graph.h>
#include <tt.h>
#include <app.h>
#ifdef SIM_WITH_OPENGL
#include <oGL.h>
#endif


CGraphDataComb_old::CGraphDataComb_old()
{
    name="Curve";
    visible=true;
    visibleOnTopOfEverything=false;
    threeDCurveWidth=1.0;
    data[0]=-1;
    data[1]=-1;
    data[2]=-1;
    curveColor.setColorsAllBlack();
    curveColor.getColorsPtr()[0]=1.0;
    curveColor.getColorsPtr()[2]=1.0;
    linkPoints=true;
    label=true;
    _curveRelativeToWorld=false;
}

CGraphDataComb_old::~CGraphDataComb_old()
{
}

void CGraphDataComb_old::setLinkPoints(bool l)
{
    linkPoints=l;
}
void CGraphDataComb_old::setLabel(bool l)
{
    label=l;
}
bool CGraphDataComb_old::getLabel()
{
    return(label); 
}

bool CGraphDataComb_old::getLinkPoints()
{
    return(linkPoints); 
}
int CGraphDataComb_old::getIdentifier()
{
    return(identifier); 
}
bool CGraphDataComb_old::getVisible()
{
    return(visible); 
}
void CGraphDataComb_old::setVisible(bool v)
{
    visible=v;
}

void CGraphDataComb_old::setCurveRelativeToWorld(bool relToWorld)
{
    _curveRelativeToWorld=relToWorld;
}

bool CGraphDataComb_old::getCurveRelativeToWorld()
{
    return(_curveRelativeToWorld);
}

void CGraphDataComb_old::setVisibleOnTopOfEverything(bool v)
{
    visibleOnTopOfEverything=v;
}

bool CGraphDataComb_old::getVisibleOnTopOfEverything()
{
    return(visibleOnTopOfEverything);
}

void CGraphDataComb_old::set3DCurveWidth(double width)
{
    tt::limitValue(1.0,6.0,width);
    threeDCurveWidth=width;
}

double CGraphDataComb_old::get3DCurveWidth()
{
    return(threeDCurveWidth);
}

void CGraphDataComb_old::setIdentifier(int newIdentifier)
{
    identifier=newIdentifier;
}
void CGraphDataComb_old::setName(std::string theName)
{
    name=theName;
}
std::string CGraphDataComb_old::getName()
{ 
    return(name); 
}
CGraphDataComb_old* CGraphDataComb_old::copyYourself()
{   // Everything is copied, even the identifier and name
    CGraphDataComb_old* newObj=new CGraphDataComb_old();
    newObj->setName(getName());
    newObj->setIdentifier(getIdentifier());
    newObj->setVisible(getVisible());
    newObj->setVisibleOnTopOfEverything(getVisibleOnTopOfEverything());
    newObj->set3DCurveWidth(get3DCurveWidth());
    newObj->setLinkPoints(getLinkPoints());
    newObj->setLabel(getLabel());
    newObj->setCurveRelativeToWorld(getCurveRelativeToWorld());
    curveColor.copyYourselfInto(&newObj->curveColor);
    for (int i=0;i<3;i++)
        newObj->data[i]=data[i];
    return(newObj);
}

void CGraphDataComb_old::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Nme");
            ar << name;
            ar.flush();

            ar.storeDataName("Oid");
            ar << identifier;
            ar.flush();

            ar.storeDataName("Cl2");
            ar.setCountingMode();
            curveColor.serialize(ar,0);
            if (ar.setWritingMode())
                curveColor.serialize(ar,0);

            ar.storeDataName("Did");
            ar << data[0] << data[1] << data[2];
            ar.flush();

#ifdef TMPOPERATION
            ar.storeDataName("3dw");
            ar << (float)threeDCurveWidth;
            ar.flush();
#endif

            ar.storeDataName("_dw");
            ar << threeDCurveWidth;
            ar.flush();



            ar.storeDataName("Par");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,visible);
            SIM_SET_CLEAR_BIT(nothing,1,linkPoints);
            SIM_SET_CLEAR_BIT(nothing,2,label);
            SIM_SET_CLEAR_BIT(nothing,3,visibleOnTopOfEverything);
            SIM_SET_CLEAR_BIT(nothing,4,_curveRelativeToWorld);
            ar << nothing;
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
                    if (theName.compare("Nme")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> name;
                    }
                    if (theName.compare("Oid")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> identifier;
                    }
                    if (theName.compare("Col")==0)
                    { // for backward compatibility 11/06/2016
                        noHit=false;
                        ar >> byteQuantity;
                        float bla[3];
                        ar >> bla[0] >> bla[1] >> bla[2];
                        curveColor.getColorsPtr()[0]=(double)bla[0];
                        curveColor.getColorsPtr()[1]=(double)bla[1];
                        curveColor.getColorsPtr()[2]=(double)bla[2];
                        ar >> bla[0] >> bla[1] >> bla[2];
                        curveColor.getColorsPtr()[9]=(double)bla[0];
                        curveColor.getColorsPtr()[10]=(double)bla[1];
                        curveColor.getColorsPtr()[11]=(double)bla[2];
                    }
                    if (theName.compare("Cl2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; 
                        curveColor.serialize(ar,0);
                    }
                    if (theName.compare("Did")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> data[0] >> data[1] >> data[2];
                    }
                    if (theName=="3dw")
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        threeDCurveWidth=(double)bla;
                    }

                    if (theName=="_dw")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> threeDCurveWidth;
                    }

                    if (theName=="Par")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        visible=SIM_IS_BIT_SET(nothing,0);
                        linkPoints=SIM_IS_BIT_SET(nothing,1);
                        label=SIM_IS_BIT_SET(nothing,2);
                        visibleOnTopOfEverything=SIM_IS_BIT_SET(nothing,3);
                        _curveRelativeToWorld=SIM_IS_BIT_SET(nothing,4);
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
            ar.xmlAddNode_string("name",name.c_str());

            ar.xmlAddNode_int("id",identifier);

            ar.xmlAddNode_float("curveSize",threeDCurveWidth);

            ar.xmlAddNode_3int("dataStreams",data[0],data[1],data[2]);

            ar.xmlPushNewNode("color");
            curveColor.serialize(ar,0);
            ar.xmlPopNode();

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("visible",visible);
            ar.xmlAddNode_bool("linkPoints",linkPoints);
            ar.xmlAddNode_bool("showLabel",label);
            ar.xmlAddNode_bool("onTop",visibleOnTopOfEverything);
            ar.xmlAddNode_bool("relativeToWorld",_curveRelativeToWorld);
            ar.xmlPopNode();
        }
        else
        {
            ar.xmlGetNode_string("name",name);
            ar.xmlGetNode_int("id",identifier);

            ar.xmlGetNode_float("curveSize",threeDCurveWidth);

            ar.xmlGetNode_3int("dataStreams",data[0],data[1],data[2]);

            if (ar.xmlPushChildNode("color"))
            {
                curveColor.serialize(ar,0);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("switches"))
            {
                ar.xmlGetNode_bool("visible",visible);
                ar.xmlGetNode_bool("linkPoints",linkPoints);
                ar.xmlGetNode_bool("showLabel",label);
                ar.xmlGetNode_bool("onTop",visibleOnTopOfEverything);
                ar.xmlGetNode_bool("relativeToWorld",_curveRelativeToWorld);
                ar.xmlPopNode();
            }
        }
    }
}
