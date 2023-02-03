#pragma once

#include <simTypes.h>
#include <ser.h>

class CStaticGraphCurve_old
{
public:
    
    CStaticGraphCurve_old();
    CStaticGraphCurve_old(int type,std::vector<double>* xVals,std::vector<double>* yVals,std::vector<double>* zVals);
    virtual ~CStaticGraphCurve_old();

    // Various
    int getCurveType();
    void setLinkPoints(bool l);
    bool getLinkPoints();
    std::string getName();
    void setName(std::string theName);
    void setLabel(bool l);
    bool getLabel();
    void setCurveWidth(double w);
    double getCurveWidth();
    void setRelativeToWorld(bool r);
    bool getRelativeToWorld();

    void serialize(CSer& ar);
    CStaticGraphCurve_old* copyYourself();
    
    // Variables which need to be serialized & copied 
    std::vector <double> values;
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
