#pragma once

#include "ser.h"

class CStaticGraphCurve_old
{
public:
    
    CStaticGraphCurve_old();
    CStaticGraphCurve_old(int type,std::vector<float>* xVals,std::vector<float>* yVals,std::vector<float>* zVals);
    virtual ~CStaticGraphCurve_old();

    // Various
    int getCurveType();
    void setLinkPoints(bool l);
    bool getLinkPoints();
    std::string getName();
    void setName(std::string theName);
    void setLabel(bool l);
    bool getLabel();
    void setCurveWidth(float w);
    float getCurveWidth();
    void setRelativeToWorld(bool r);
    bool getRelativeToWorld();

    void serialize(CSer& ar);
    CStaticGraphCurve_old* copyYourself();
    
    // Variables which need to be serialized & copied 
    std::vector <float> values;
    float ambientColor[3];
    float emissiveColor[3];

protected:
    // Variables which need to be serialized & copied 
    int _curveType;
    float _curveWidth; // for 3D curves only!
    bool _linkPoints;
    bool _label;
    bool _relativeToWorld;
    std::string _name;
};
