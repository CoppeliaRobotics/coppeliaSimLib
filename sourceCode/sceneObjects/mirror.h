#pragma once

#include <sceneObject.h>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
MIRROR_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_mirror;
// ----------------------------------------------------------------------------------------------

class CMirror : public CSceneObject
{
  public:
    CMirror();
    virtual ~CMirror();

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
    int setFloatProperty(const char* pName, double pState) override;
    int getFloatProperty(const char* pName, double& pState) const override;
    int getStringProperty(const char* pName, std::string& pState) const override;
    int setColorProperty(const char* pName, const float* pState) override;
    int getColorProperty(const char* pName, float* pState) const override;
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

    void setMirrorWidth(double w);
    double getMirrorWidth();
    void setMirrorHeight(double h);
    double getMirrorHeight();
    void setReflectance(double r);
    double getReflectance();

    void setActive(bool a);
    bool getActive();
    void setIsMirror(bool m);
    bool getIsMirror();

    void setClippingObjectOrCollection(int co);
    int getClippingObjectOrCollection();

    bool getContainsTransparentComponent() const;
    CColorObject* getClipPlaneColor();

    // Variables which need to be serialized & copied
    float mirrorColor[3];
    static int currentMirrorContentBeingRendered;

  protected:
    void _commonInit();

    // Variables which need to be serialized & copied
    CColorObject clipPlaneColor;
    double _mirrorWidth;
    double _mirrorHeight;
    float _mirrorReflectance;
    bool _active;
    bool _isMirror;
    int _clippingObjectOrCollection;

    // Variables which do not need to be serialized
    bool _initialMirrorActive;

#ifdef SIM_WITH_GUI
  public:
    void display(CViewableBase* renderingObject, int displayAttrib) override;
#endif
};
