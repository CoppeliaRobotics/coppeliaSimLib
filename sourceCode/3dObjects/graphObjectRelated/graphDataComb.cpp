
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "graphDataComb.h"
#include "global.h"
#include "graph.h"
#include "tt.h"
#include "app.h"
#ifdef SIM_WITH_OPENGL
#include "oGL.h"
#endif


CGraphDataComb::CGraphDataComb()
{
    name="Curve";
    visible=true;
    visibleOnTopOfEverything=false;
    threeDCurveWidth=1.0f;
    data[0]=-1;
    data[1]=-1;
    data[2]=-1;
    curveColor.setColorsAllBlack();
    curveColor.colors[0]=1.0;
    curveColor.colors[2]=1.0;
    linkPoints=true;
    label=true;
    _curveRelativeToWorld=false;
}

CGraphDataComb::~CGraphDataComb()
{
}

void CGraphDataComb::setLinkPoints(bool l)
{
    linkPoints=l;
}
void CGraphDataComb::setLabel(bool l)
{
    label=l;
}
bool CGraphDataComb::getLabel() 
{
    return(label); 
}

bool CGraphDataComb::getLinkPoints() 
{
    return(linkPoints); 
}
int CGraphDataComb::getIdentifier() 
{
    return(identifier); 
}
bool CGraphDataComb::getVisible() 
{
    return(visible); 
}
void CGraphDataComb::setVisible(bool v)
{
    visible=v;
}

void CGraphDataComb::setCurveRelativeToWorld(bool relToWorld)
{
    _curveRelativeToWorld=relToWorld;
}

bool CGraphDataComb::getCurveRelativeToWorld()
{
    return(_curveRelativeToWorld);
}

void CGraphDataComb::setVisibleOnTopOfEverything(bool v)
{
    visibleOnTopOfEverything=v;
}

bool CGraphDataComb::getVisibleOnTopOfEverything()
{
    return(visibleOnTopOfEverything);
}

void CGraphDataComb::set3DCurveWidth(float width)
{
    tt::limitValue(1.0f,6.0f,width);
    threeDCurveWidth=width;
}

float CGraphDataComb::get3DCurveWidth()
{
    return(threeDCurveWidth);
}

void CGraphDataComb::setIdentifier(int newIdentifier)
{
    identifier=newIdentifier;
}
void CGraphDataComb::setName(std::string theName)
{
    name=theName;
}
std::string CGraphDataComb::getName() 
{ 
    return(name); 
}
CGraphDataComb* CGraphDataComb::copyYourself()
{   // Everything is copied, even the identifier and name
    CGraphDataComb* newObj=new CGraphDataComb();
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

void CGraphDataComb::serialize(CSer& ar)
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

            ar.storeDataName("3dw");
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
                        ar >> curveColor.colors[0] >> curveColor.colors[1] >> curveColor.colors[2];
                        ar >> curveColor.colors[9] >> curveColor.colors[10] >> curveColor.colors[11];
                    }
                    if (theName.compare("Cl2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        curveColor.serialize(ar,0);
                    }
                    if (theName.compare("Did")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> data[0] >> data[1] >> data[2];
                    }
                    if (theName=="3dw")
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
}
