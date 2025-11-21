#pragma once

#include <ser.h>
#include <simLib/simConst.h>
#include <cbor.h>

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES                                                                   \
    FUNCX(propCol_colDiffuse, "diffuse", sim_propertytype_color, 0, "Diffuse color", "")    \
    FUNCX(propCol_colSpecular, "specular", sim_propertytype_color, 0, "Specular color", "") \
    FUNCX(propCol_colEmission, "emission", sim_propertytype_color, 0, "Emission color", "") \
    FUNCX(propCol_transparency, "transparency", sim_propertytype_float, 0, "Transparency", "")

#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
const std::vector<SProperty> allProps_col = {DEFINE_PROPERTIES};
#undef FUNCX
#undef DEFINE_PROPERTIES

// ----------------------------------------------------------------------------------------------

class CColorObject
{
  public:
    CColorObject();
    virtual ~CColorObject();

    void setDefaultValues();
    void setColorsAllBlack();
    void setEventParams(bool belongsToSceneObject, int eventObjectHandle, int eventFlags = -1, const char* eventPrefix = nullptr);
    bool setColor(const float theColor[3], unsigned char colorMode);
    bool setColor(float r, float g, float b, unsigned char colorMode);
#if SIM_EVENT_PROTOCOL_VERSION == 2
    void pushShapeColorChangeEvent(int objectHandle, int colorIndex);
    static void pushColorChangeEvent(int objectHandle, float col1[9], float col2[9] = nullptr, float col3[9] = nullptr, float col4[9] = nullptr);
#else
    void addGenesisEventData(CCbor* ev) const;
#endif
    void getNewColors(float cols[9]) const;
    void copyYourselfInto(CColorObject* it) const;
    void serialize(CSer& ar, int objType); // 0=3d mesh, 1=3d lines, 2=3d points, 3=3d light, 4=2d thing

    void getColor(float col[3], unsigned char colorMode) const;
    void getColors(float col[15]) const;
    const float* getColorsPtr() const;
    float* getColorsPtr();

    float getTransparency() const;
    bool setTransparency(float t);
    bool getTranslucid() const;
    float getOpacity() const;
    int getShininess() const;
    std::string getColorName() const;
    std::string getExtensionString() const;

    int setFloatProperty(const char* pName, double pState);
    int getFloatProperty(const char* pName, double& pState) const;
    int setColorProperty(const char* pName, const float* pState);
    int getColorProperty(const char* pName, float* pState) const;
    int getPropertyName(int& index, std::string& pName, int excludeFlags) const;
    static int getPropertyName_static(int& index, std::string& pName, int eventFlags, const char* eventPrefix, int excludeFlags);
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const;
    static int getPropertyInfo_static(const char* pName, int& info, std::string& infoTxt, int eventFlags, const char* eventPrefix);

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
    void setColorName(const char* nm);
    void setExtensionString(const char* nm);

    //    void handleSceneObjectColorEvents(int objHandle);
  private:
    bool _isSame(const CColorObject* it) const;
    std::string _getPatternStringFromPatternId_backwardCompatibility_3_2_2016(int id);

    float _colors[15];
    int _shininess;
    float _opacity;
    bool _translucid;
    int _eventFlags;
    int _eventObjectHandle;
    bool _belongsToSceneObject;
    std::string _eventPrefix;
    std::string _colorName;

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
