#pragma once

#include <ser.h>
#include <simLib/simConst.h>

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES \
    FUNCX(propCol_colDiffuse,              "diffuseColor",                             sim_propertytype_color,     0) \
    FUNCX(propCol_colSpecular,             "specularColor",                            sim_propertytype_color,     0) \
    FUNCX(propCol_colEmission,             "emissionColor",                            sim_propertytype_color,     0) \
    FUNCX(propCol_transparency,            "transparency",                             sim_propertytype_float,     0) \

#define FUNCX(name, str, v1, v2) const CProperty name = {str, v1, v2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2) name,
const std::vector<CProperty> allProps_col = { DEFINE_PROPERTIES };
#undef FUNCX
#undef DEFINE_PROPERTIES
#undef CONCAT_PROP
// ----------------------------------------------------------------------------------------------

class CColorObject
{
  public:
    CColorObject();
    virtual ~CColorObject();

    void setDefaultValues();
    void setColorsAllBlack();
    bool setColor(const float theColor[3], unsigned char colorMode, int handleForEventGeneration = -1);
    bool setColor(float r, float g, float b, unsigned char colorMode);
#if SIM_EVENT_PROTOCOL_VERSION == 2
    void pushShapeColorChangeEvent(int objectHandle, int colorIndex);
#endif
    static void pushColorChangeEvent(int objectHandle, float col1[9], float col2[9] = nullptr, float col3[9] = nullptr,
                                     float col4[9] = nullptr);
    void getNewColors(float cols[9]) const;
    void copyYourselfInto(CColorObject *it) const;
    void serialize(CSer &ar, int objType); // 0=3d mesh, 1=3d lines, 2=3d points, 3=3d light, 4=2d thing

    void getColor(float col[3], unsigned char colorMode) const;
    void getColors(float col[15]) const;
    const float *getColorsPtr() const;
    float *getColorsPtr();

    float getTransparency() const;
    bool setTransparency(float t, int handleForEventGeneration = -1);
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

    bool setColors(const float col[15]);
    void setTranslucid(bool e);
    void setOpacity(float e);
    void setShininess(int e);
    void setColorName(const char *nm);
    void setExtensionString(const char *nm);

//    void handleSceneObjectColorEvents(int objHandle);
  private:
    bool _isSame(const CColorObject *it) const;
    std::string _getPatternStringFromPatternId_backwardCompatibility_3_2_2016(int id);

    float _colors[15];
    int _shininess;
    float _opacity;
    bool _translucid;
    std::string _colorName;

    float _icolors[15];
    int _ishininess;
    float _iopacity;
    bool _itranslucid;
    std::string _icolorName;

    std::string _extensionString;

    bool _useSimulationTime;
    float _flashFrequency;
    float _flashRatio;
    float _flashPhase;
    bool _flash;

#ifdef SIM_WITH_GUI
  public:
    void makeCurrentColor(bool useAuxiliaryComponent) const;
    void makeCurrentColor2(bool forceNonTransparent, bool useAuxiliaryComponent) const;
#endif
};
