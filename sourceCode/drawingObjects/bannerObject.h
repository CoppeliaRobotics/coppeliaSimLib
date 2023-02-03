#pragma once

#include <colorObject.h>
#include <simMath/4X4Matrix.h>

class CBannerObject  
{
public:
    CBannerObject(const char* label,int options,int sceneObjID,const double relConfig[6],const float labelCol[12],const float backCol[12],double height);
    virtual ~CBannerObject();

    void draw3DStuff(bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM,const int windowSize[2],double verticalViewSizeOrAngle,bool perspective);
    void setObjectID(int newID);
    int getObjectID();

    bool announceObjectWillBeErased(int objID);
    void adjustForFrameChange(const C7Vector& preCorrection);
    void adjustForScaling(double xScale,double yScale,double zScale);

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
    double _height;
    bool _visible;
    C7Vector _relativeConfig;
};
