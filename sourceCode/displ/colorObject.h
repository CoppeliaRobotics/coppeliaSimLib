#pragma once

#include "ser.h"

class CColorObject
{
public:
    CColorObject();
    virtual ~CColorObject();

    void makeCurrentColor(bool useAuxiliaryComponent) const;
    void makeCurrentColor2(bool forceNonTransparent,bool useAuxiliaryComponent) const;
    void setDefaultValues();
    void setColorsAllBlack();
    void setColor(const float theColor[3],unsigned char colorMode);
    void setColor(float r,float g,float b,unsigned char colorMode);
    void pushColorChangeEvent(int objectHandle,int colorIndex,bool isLight=false);
    void copyYourselfInto(CColorObject* it) const;
    void serialize(CSer& ar,int objType); // 0=3d mesh, 1=3d lines, 2=3d points, 3=3d light, 4=2d thing
    void setConvexColors();

    void getColor(float col[3],unsigned char colorMode) const;
    void getColors(float col[15]) const;
    const float* getColorsPtr() const;
    float* getColorsPtr();
    bool getTranslucid() const;
    float getOpacity() const;
    int getShininess() const;
    std::string getColorName() const;
    std::string getExtensionString() const;

    void setFlash(bool flashIsOn);
    bool getFlash() const;
    void setFlashFrequency(float f);
    float getFlashFrequency() const;
    void setFlashRatio(float f);
    float getFlashRatio() const;
    void setFlashPhase(float f);
    float getFlashPhase() const;
    void setUseSimulationTime(bool sim);
    bool getUseSimulationTime() const;

    void setColors(const float col[15]);
    void setTranslucid(bool e);
    void setOpacity(float e);
    void setShininess(int e);
    void setColorName(const char* nm);
    void setExtensionString(const char* nm);

private:
    bool _isSame(const CColorObject* it) const;
    std::string _getPatternStringFromPatternId_backwardCompatibility_3_2_2016(int id);

    float _colors[15];
    int _shininess;
    float _opacity;
    bool _translucid;
    std::string _colorName;
    std::string _extensionString;

    bool _useSimulationTime;
    float _flashFrequency;
    float _flashRatio;
    float _flashPhase;
    bool _flash;
};
