
#pragma once

#include "ser.h"

class CVisualParam  
{
public:

    CVisualParam();
    virtual ~CVisualParam();
    void makeCurrentColor(bool useAuxiliaryComponent) const;
    void makeCurrentColor2(bool forceNonTransparent,bool useAuxiliaryComponent) const;
    void setDefaultValues();
    void setColorsAllBlack();
    void setColor(const float theColor[3],unsigned char colorMode);
    void setColor(float r,float g,float b,unsigned char colorMode);
    void copyYourselfInto(CVisualParam* it) const;
    void serialize(CSer& ar,int objType); // 0=3d mesh, 1=3d lines, 2=3d points, 3=3d light, 4=2d thing
    void setFlash(bool flashIsOn);
    bool getFlash() const;
    void setPovPatternType(int patternType);
    int getPovPatternType() const;
    void setConvexColors();

    // Variables which need to be serialized & copied
    float colors[15];
    int shininess;
    float transparencyFactor;
    bool translucid;
    bool useSimulationTime;
    float flashFrequency;
    float flashRatio;
    float flashPhase;
    bool flash;
    std::string colorName;
    std::string extensionString;

protected:
    std::string _getPatternStringFromPatternId_backwardCompatibility_3_2_2016(int id);
};
