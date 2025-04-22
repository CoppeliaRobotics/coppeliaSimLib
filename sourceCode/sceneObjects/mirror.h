#pragma once

#include <sceneObject.h>

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES

#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
const std::vector<SProperty> allProps_mirror = {DEFINE_PROPERTIES};
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

class CMirror : public CSceneObject
{
  public:
    CMirror();
    virtual ~CMirror();

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
    void performObjectLoadingMapping(const std::map<int, int>* map, int opType);
    void performCollectionLoadingMapping(const std::map<int, int>* map, int opType);
    void performCollisionLoadingMapping(const std::map<int, int>* map, int opType);
    void performDistanceLoadingMapping(const std::map<int, int>* map, int opType);
    void performIkLoadingMapping(const std::map<int, int>* map, int opType);
    void performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType);
    void performDynMaterialObjectLoadingMapping(const std::map<int, int>* map);
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void computeBoundingBox();
    void setIsInScene(bool s);
    int setFloatProperty(const char* pName, double pState);
    int getFloatProperty(const char* pName, double& pState) const;
    int setColorProperty(const char* pName, const float* pState);
    int getColorProperty(const char* pName, float* pState) const;
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
    void display(CViewableBase* renderingObject, int displayAttrib);
#endif
};
