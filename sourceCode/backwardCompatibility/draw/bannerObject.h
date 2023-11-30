#pragma once

#include <colorObject.h>
#include <simMath/4X4Matrix.h>

class CBannerObject
{
  public:
    CBannerObject(const char *label, int options, int sceneObjID, const double relConfig[6], const float labelCol[12],
                  const float backCol[12], double height);
    virtual ~CBannerObject();

    void setObjectID(int newID);
    int getObjectID();

    bool announceObjectWillBeErased(int objID);

    int getSceneObjectID();
    bool getCreatedFromScript();
    void setCreatedFromScript(bool c);
    int getOptions();
    int getParentObjectHandle();

    bool isVisible();
    bool toggleVisibility();

#ifdef SIM_WITH_GUI
    void draw3DStuff(bool overlay, bool transparentObject, int displayAttrib, const C4X4Matrix &cameraCTM,
                     const int windowSize[2], double verticalViewSizeOrAngle, bool perspective);
#endif

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
