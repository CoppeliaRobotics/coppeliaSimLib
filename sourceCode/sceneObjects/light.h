#pragma once

#include <sceneObject.h>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
LIGHT_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_light;
// ----------------------------------------------------------------------------------------------

class CLight : public CSceneObject
{
  public:
    CLight();
    CLight(int theType);
    virtual ~CLight();

    // Following functions are inherited from CSceneObject
    void addSpecializedObjectEventData(CCbor* ev) override;
    CSceneObject* copyYourself() override;
    void removeSceneDependencies() override;
    void scaleObject(double scalingFactor) override;
    void serialize(CSer& ar) override;
    void announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer) override;
    void announceCollectionWillBeErased(int groupID, bool copyBuffer) override;
    void announceCollisionWillBeErased(int collisionID, bool copyBuffer) override;
    void announceDistanceWillBeErased(int distanceID, bool copyBuffer) override;
    void announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer) override;
    void performObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performCollectionLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performCollisionLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performDistanceLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performIkLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performDynMaterialObjectLoadingMapping(const std::map<int, int>* map) override;
    void simulationAboutToStart() override;
    void simulationEnded() override;
    void initializeInitialValues(bool simulationAlreadyRunning) override;
    void computeBoundingBox() override;
    void setIsInScene(bool s) override;
    int setBoolProperty(const char* pName, bool pState) override;
    int getBoolProperty(const char* pName, bool& pState) const override;
    int setIntProperty(const char* pName, int pState) override;
    int getIntProperty(const char* pName, int& pState) const override;
    int setFloatProperty(const char* pName, double pState) override;
    int getFloatProperty(const char* pName, double& pState) const override;
    int getStringProperty(const char* pName, std::string& pState) const override;
    int setColorProperty(const char* pName, const float* pState) override;
    int getColorProperty(const char* pName, float* pState) const override;
    int setVector3Property(const char* pName, const C3Vector& pState) override;
    int getVector3Property(const char* pName, C3Vector& pState) const override;
    int setFloatArrayProperty(const char* pName, const double* v, int vL) override;
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const override;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const override;
    static int getPropertyName_static(int& index, std::string& pName, std::string& appartenance, int excludeFlags);
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const override;
    static int getPropertyInfo_static(const char* pName, int& info, std::string& infoTxt);

    std::string getObjectTypeInfo() const override;
    std::string getObjectTypeInfoExtended() const override;
    bool isPotentiallyCollidable() const override;
    bool isPotentiallyMeasurable() const override;
    bool isPotentiallyDetectable() const override;
    bool isPotentiallyRenderable() const override;

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
    void display(CViewableBase* renderingObject, int displayAttrib) override;
#endif
};
