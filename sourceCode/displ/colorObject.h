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
    void setColor(const floatDouble theColor[3],unsigned char colorMode);
    void setColor(floatDouble r,floatDouble g,floatDouble b,unsigned char colorMode);
    void pushShapeColorChangeEvent(int objectHandle,int colorIndex);
    static void pushColorChangeEvent(int objectHandle,floatDouble col1[9],floatDouble col2[9]=nullptr,floatDouble col3[9]=nullptr,floatDouble col4[9]=nullptr);
    void getNewColors(floatDouble cols[9]) const;
    void copyYourselfInto(CColorObject* it) const;
    void serialize(CSer& ar,int objType); // 0=3d mesh, 1=3d lines, 2=3d points, 3=3d light, 4=2d thing
    void setConvexColors();

    void getColor(floatDouble col[3],unsigned char colorMode) const;
    void getColors(floatDouble col[15]) const;
    const floatDouble* getColorsPtr() const;
    floatDouble* getColorsPtr();
    bool getTranslucid() const;
    floatDouble getOpacity() const;
    int getShininess() const;
    std::string getColorName() const;
    std::string getExtensionString() const;

    void setFlash(bool flashIsOn);
    bool getFlash() const;
    void setFlashFrequency(floatDouble f);
    floatDouble getFlashFrequency() const;
    void setFlashRatio(floatDouble f);
    floatDouble getFlashRatio() const;
    void setFlashPhase(floatDouble f);
    floatDouble getFlashPhase() const;
    void setUseSimulationTime(bool sim);
    bool getUseSimulationTime() const;

    void setColors(const floatDouble col[15]);
    void setTranslucid(bool e);
    void setOpacity(floatDouble e);
    void setShininess(int e);
    void setColorName(const char* nm);
    void setExtensionString(const char* nm);

private:
    bool _isSame(const CColorObject* it) const;
    std::string _getPatternStringFromPatternId_backwardCompatibility_3_2_2016(int id);

    floatDouble _colors[15];
    int _shininess;
    floatDouble _opacity;
    bool _translucid;
    std::string _colorName;
    std::string _extensionString;

    bool _useSimulationTime;
    floatDouble _flashFrequency;
    floatDouble _flashRatio;
    floatDouble _flashPhase;
    bool _flash;
};
