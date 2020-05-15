
#pragma once

#include "colorObject.h"
#include "4X4Matrix.h"

class CBannerObject  
{
public:
    CBannerObject(const char* label,int options,int sceneObjID,const float relConfig[6],const float labelCol[12],const float backCol[12],float height);
    virtual ~CBannerObject();

    void draw3DStuff(bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM,const int windowSize[2],float verticalViewSizeOrAngle,bool perspective);
    void setObjectID(int newID);
    int getObjectID();

    bool announceObjectWillBeErased(int objID);
    void adjustForFrameChange(const C7Vector& preCorrection);
    void adjustForScaling(float xScale,float yScale,float zScale);

    int getSceneObjectID();
    bool getCreatedFromScript();
    void setCreatedFromScript(bool c);
    int getOptions();
    int getParentObjectHandle();

    bool isVisible();
    bool toggleVisibility();

    CColorObject color;
    CColorObject backColor;

protected:
    std::string _label;
    int _objectID;
    int _sceneObjectID;
    int _options;
    bool _createdFromScript;
    float _height;
    bool _visible;
    C7Vector _relativeConfig;
};
