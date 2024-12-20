#pragma once

#include <sceneObject.h>

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES                                                                                                 \
    FUNCX(propLight_size, "lightSize", sim_propertytype_float, 0, "Size", "Light size")                                   \
    FUNCX(propLight_enabled, "enabled", sim_propertytype_bool, 0, "Enabled", "")                                          \
    FUNCX(propLight_lightType, "lightType", sim_propertytype_int, sim_propertyinfo_notwritable, "Type", "Light type")     \
    FUNCX(propLight_spotExponent, "spotExponent", sim_propertytype_int, 0, "Spot exponent", "")                           \
    FUNCX(propLight_spotCutoffAngle, "spotCutoffAngle", sim_propertytype_float, 0, "Cut off angle", "Spot cut off angle") \
    FUNCX(propLight_attenuationFactors, "attenuationFactors", sim_propertytype_floatarray, 0, "Attenuation factor", "")   \
    FUNCX(propLight_povCastShadows, "povray.castShadows", sim_propertytype_bool, 0, "POV-Ray: cast shadows", "Light casts shadows (with the POV-Ray renderer plugin)")

#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
const std::vector<SProperty> allProps_light = {DEFINE_PROPERTIES};
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

class CLight : public CSceneObject
{
  public:
    CLight();
    CLight(int theType);
    virtual ~CLight();

    // Following functions are inherited from CSceneObject
    void addSpecializedObjectEventData(CCbor* ev);
    CSceneObject* copyYourself();
    void removeSceneDependencies();
    void scaleObject(double scalingFactor);
    void serialize(CSer& ar);
    void announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer);
    void announceCollectionWillBeErased(int groupID, bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID, bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID, bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer);
    void performObjectLoadingMapping(const std::map<int, int>* map, bool loadingAmodel);
    void performCollectionLoadingMapping(const std::map<int, int>* map, bool loadingAmodel);
    void performCollisionLoadingMapping(const std::map<int, int>* map, bool loadingAmodel);
    void performDistanceLoadingMapping(const std::map<int, int>* map, bool loadingAmodel);
    void performIkLoadingMapping(const std::map<int, int>* map, bool loadingAmodel);
    void performTextureObjectLoadingMapping(const std::map<int, int>* map);
    void performDynMaterialObjectLoadingMapping(const std::map<int, int>* map);
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void computeBoundingBox();
    void setIsInScene(bool s);
    int setBoolProperty(const char* pName, bool pState);
    int getBoolProperty(const char* pName, bool& pState) const;
    int setIntProperty(const char* pName, int pState);
    int getIntProperty(const char* pName, int& pState) const;
    int setFloatProperty(const char* pName, double pState);
    int getFloatProperty(const char* pName, double& pState) const;
    int setColorProperty(const char* pName, const float* pState);
    int getColorProperty(const char* pName, float* pState) const;
    int setVector3Property(const char* pName, const C3Vector& pState);
    int getVector3Property(const char* pName, C3Vector& pState) const;
    int setFloatArrayProperty(const char* pName, const double* v, int vL);
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance);
    static int getPropertyName_static(int& index, std::string& pName, std::string& appartenance);
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt);
    static int getPropertyInfo_static(const char* pName, int& info, std::string& infoTxt);

    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;

    void setLightActive(bool active);
    bool getLightActive() const;
    void getAttenuationFactors(double fact[3]) const;
    void setAttenuationFactors(const double fact[3]);
    void setLightSize(double size);
    double getLightSize() const;
    void setSpotExponent(int e);
    int getSpotExponent() const;
    void setSpotCutoffAngle(double co);
    double getSpotCutoffAngle() const;
    int getLightType() const;

    void setLightIsLocal(bool l);
    bool getLightIsLocal() const;

    static void setMaxAvailableOglLights(int c);
    static int getMaxAvailableOglLights();

    CColorObject* getColor(bool getLightColor);

  protected:
    void _setDefaultColors();
    void _commonInit();

    // Variables which need to be serialized & copied
    CColorObject objectColor;
    CColorObject lightColor;
    bool lightActive;
    double constantAttenuation;
    double linearAttenuation;
    double quadraticAttenuation;
    int _spotExponent;
    double _spotCutoffAngle;
    double _lightSize;
    int _lightType;
    bool _lightIsLocal;

    // Variables which do not need to be serialized
    bool _initialLightActive;
    static int _maximumOpenGlLights;

#ifdef SIM_WITH_GUI
  public:
    void display(CViewableBase* renderingObject, int displayAttrib);
#endif
};
