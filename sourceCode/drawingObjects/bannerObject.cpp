#include "simInternal.h"
#include "bannerObject.h"
#include "app.h"
#include "tt.h"
#include "bannerRendering.h"

bool CBannerObject::getCreatedFromScript()
{
    return(_createdFromScript);
}

void CBannerObject::setCreatedFromScript(bool c)
{
    _createdFromScript=c;
}

CBannerObject::CBannerObject(const char* label,int options,int sceneObjID,const floatDouble relConfig[6],const floatDouble labelCol[12],const floatDouble backCol[12],floatDouble height)
{
    _visible=true;
    _label=label;
    _height=height;
    _createdFromScript=false;
    color.setColorsAllBlack();
    backColor.setColorsAllBlack();
    floatDouble col[15];
    backColor.getColors(col);
    col[0]=1.0;
    col[1]=1.0;
    col[2]=1.0;
    backColor.setColors(col);
    if (labelCol!=nullptr)
    {
        color.setColor(labelCol+0,sim_colorcomponent_ambient_diffuse);
        color.setColor(labelCol+6,sim_colorcomponent_specular);
        color.setColor(labelCol+9,sim_colorcomponent_emission);
    }
    if (backCol!=nullptr)
    {
        backColor.setColor(backCol+0,sim_colorcomponent_ambient_diffuse);
        backColor.setColor(backCol+6,sim_colorcomponent_specular);
        backColor.setColor(backCol+9,sim_colorcomponent_emission);
    }
    _objectID=0;
    _sceneObjectID=sceneObjID;
    _options=options;
    _relativeConfig.setIdentity();
    if (relConfig!=nullptr)
    {
        _relativeConfig.X.setData(relConfig);
        _relativeConfig.Q.setEulerAngles(relConfig[3],relConfig[4],relConfig[5]);
    }
}

CBannerObject::~CBannerObject()
{
}

int CBannerObject::getSceneObjectID()
{
    return(_sceneObjectID);
}

void CBannerObject::setObjectID(int newID)
{
    _objectID=newID;
}

int CBannerObject::getObjectID()
{
    return(_objectID);
}

bool CBannerObject::isVisible()
{
    return(_visible);
}

bool CBannerObject::toggleVisibility()
{
    _visible=!_visible;
    return(_visible);
}

void CBannerObject::adjustForFrameChange(const C7Vector& preCorrection)
{
    _relativeConfig*=preCorrection;
}

void CBannerObject::adjustForScaling(floatDouble xScale,floatDouble yScale,floatDouble zScale)
{
    floatDouble avgScaling=(xScale+yScale+zScale)/3.0;
    _relativeConfig.X*=avgScaling;
}

int CBannerObject::getOptions()
{
    return(_options);
}

int CBannerObject::getParentObjectHandle()
{
    return(_sceneObjectID);
}

bool CBannerObject::announceObjectWillBeErased(int objID)
{
    return(_sceneObjectID==objID);
}

void CBannerObject::draw3DStuff(bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM,const int windowSize[2],floatDouble verticalViewSizeOrAngle,bool perspective)
{
    if (_visible)
    {
        if (_options&sim_banner_overlay)
        {
            if (!overlay)
                return;
        }
        else
        {
            if (overlay)
                return;
        }

        if (!overlay)
        {
            if (false)//_options&(sim_drawing_50percenttransparency+sim_drawing_25percenttransparency+sim_drawing_12percenttransparency))
            {
                if (!transparentObject)
                    return;
            }
            else
            {
                if (transparentObject)
                    return;
            }
        }


        C7Vector tr;
        tr.setIdentity();
        if (_sceneObjectID>=0)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectID);
            if (it==nullptr)
                _sceneObjectID=-2; // should normally never happen
            else
            {
                tr=it->getCumulativeTransformation();
                if (_options&sim_banner_followparentvisibility)
                {
                    if ( ((App::currentWorld->environment->getActiveLayers()&it->getVisibilityLayer())==0)&&((displayAttrib&sim_displayattribute_ignorelayer)==0) )
                        return; // not visible
                    if (it->isObjectPartOfInvisibleModel())
                        return; // not visible
                }
            }
        }
        if (_sceneObjectID==-2)
            return;

        tr*=_relativeConfig;
        floatDouble* bckColor=nullptr;
        if ((_options&sim_banner_nobackground)==0)
            bckColor=backColor.getColorsPtr();

        displayBanner(_objectID,_options,bckColor,tr,_label.c_str(),color,_height,cameraCTM,windowSize,verticalViewSizeOrAngle,perspective);
    }
}
