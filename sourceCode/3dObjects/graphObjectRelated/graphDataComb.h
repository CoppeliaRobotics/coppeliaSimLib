
#pragma once

#include "graphData.h"
#include "visualParam.h"

class CGraphDataComb  
{
public:

    CGraphDataComb();
    virtual ~CGraphDataComb();

    // Various
    void setVisible(bool v);
    bool getVisible();
    void setVisibleOnTopOfEverything(bool v);
    bool getVisibleOnTopOfEverything();
    void set3DCurveWidth(float width);
    float get3DCurveWidth();
    void setLinkPoints(bool l);
    bool getLinkPoints();
    void setIdentifier(int newIdentifier);
    int getIdentifier();
    std::string getName();
    void setName(std::string theName);
    void setLabel(bool l);
    bool getLabel();
    void serialize(CSer& ar);
    CGraphDataComb* copyYourself();

    void setCurveRelativeToWorld(bool relToWorld);
    bool getCurveRelativeToWorld();

    // Variables which need to be serialized & copied 
    int data[3];
    CVisualParam curveColor;

protected:
    // Variables which need to be serialized & copied 
    std::string name;
    int identifier;
    bool visible;
    bool visibleOnTopOfEverything; // only for 3D curves!
    float threeDCurveWidth; // only for 3D curves!
    bool linkPoints;
    bool label;
    bool _curveRelativeToWorld; // only for 3D curves!

private:
    int _lifeID;
};
