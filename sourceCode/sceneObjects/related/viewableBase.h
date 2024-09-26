#pragma once

#include <sceneObject.h>
#include <ser.h>

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES \
    FUNCX(propViewable_viewAngle,               "viewAngle",                                sim_propertytype_float,     0, "View angle", "View angle (in perspective projection mode)") \
    FUNCX(propViewable_viewSize,                "viewSize",                                 sim_propertytype_float,     0, "View size", "View size (in orthogonal projection mode)") \
    FUNCX(propViewable_clippingPlanes,          "clippingPlanes",                           sim_propertytype_vector,    0, "Clipping planes", "Near and far clipping planes") \
    FUNCX(propViewable_perspective,             "perspective",                              sim_propertytype_bool,      sim_propertyinfo_notwritable, "Perspective", "Perspective projection mode, otherwise orthogonal projection mode") \
    FUNCX(propViewable_showFrustum,             "showFrustum",                              sim_propertytype_bool,      0, "Show view frustum", "") \
    FUNCX(propViewable_frustumCornerNear,       "frustumCornerNear",                        sim_propertytype_vector3,   sim_propertyinfo_notwritable, "Near corner of View frustum", "") \
    FUNCX(propViewable_frustumCornerFar,        "frustumCornerFar",                         sim_propertytype_vector3,   sim_propertyinfo_notwritable, "Far corner of view frustum", "") \
    FUNCX(propViewable_resolution,              "resolution",                               sim_propertytype_intvector, 0, "Resolution", "Resolution (relevant only with vision sensors)") \

#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
const std::vector<SProperty> allProps_viewable = { DEFINE_PROPERTIES };
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

class CViewableBase : public CSceneObject
{
  public:
    CViewableBase();
    virtual ~CViewableBase();

    // Following functions need to be implemented in each class derived from CViewableBase
    virtual void addSpecializedObjectEventData(CCbor *ev);
    virtual CSceneObject *copyYourself();
    virtual void display(CViewableBase *renderingObject, int displayAttrib);
    virtual void scaleObject(double scalingFactor);
    virtual void serialize(CSer &ar);
    virtual void announceObjectWillBeErased(const CSceneObject *object, bool copyBuffer);
    virtual void announceCollectionWillBeErased(int groupID, bool copyBuffer);
    virtual void announceCollisionWillBeErased(int collisionID, bool copyBuffer);
    virtual void announceDistanceWillBeErased(int distanceID, bool copyBuffer);
    virtual void performObjectLoadingMapping(const std::map<int, int> *map);
    virtual void performCollectionLoadingMapping(const std::map<int, int> *map);
    virtual void performCollisionLoadingMapping(const std::map<int, int> *map);
    virtual void performDistanceLoadingMapping(const std::map<int, int> *map);
    virtual void simulationAboutToStart();
    virtual void simulationEnded();

    virtual std::string getObjectTypeInfo() const;
    virtual std::string getObjectTypeInfoExtended() const;
    virtual bool isPotentiallyCollidable() const;
    virtual bool isPotentiallyMeasurable() const;
    virtual bool isPotentiallyDetectable() const;
    virtual bool isPotentiallyRenderable() const;

    virtual int setBoolProperty(const char* pName, bool pState);
    virtual int getBoolProperty(const char* pName, bool& pState) const;
    virtual int setFloatProperty(const char* pName, double pState);
    virtual int getFloatProperty(const char* pName, double& pState) const;
    virtual int setVector3Property(const char* pName, const C3Vector& pState);
    virtual int getVector3Property(const char* pName, C3Vector& pState) const;
    virtual int setVectorProperty(const char* pName, const double* v, int vL);
    virtual int getVectorProperty(const char* pName, std::vector<double>& pState) const;
    virtual int setIntVectorProperty(const char* pName, const int* v, int vL);
    virtual int getIntVectorProperty(const char* pName, std::vector<int>& pState) const;
    static int getPropertyName_vstatic(int& index, std::string& pName);
    static int getPropertyInfo_vstatic(const char* pName, int& info, std::string& infoTxt);

    void setClippingPlanes(double nearPlane, double farPlane);
    void getClippingPlanes(double& nearPlane, double& farPlane) const;
    void setViewAngle(double angle);
    double getViewAngle() const;
    void setOrthoViewSize(double theSize);
    double getOrthoViewSize() const;

    void setShowFogIfAvailable(bool showFog);
    bool getShowFogIfAvailable() const;

    bool isObjectInsideView(const C7Vector &objectM, const C3Vector &maxBB);
    void computeViewFrustumIfNeeded();

    void setFrustumCullingTemporarilyDisabled(bool d);

    void setFogTimer(double seconds);
    double getFogStrength();

    void setUseLocalLights(bool l);
    bool getuseLocalLights();

    int getDisabledColorComponents();
    void setDisabledColorComponents(int c);

    virtual void setResolution(const int r[2]);
    void getResolution(int r[2]) const;

    void setPerspective(bool p);
    bool getPerspective() const;
    void setShowVolume(bool s);
    bool getShowVolume() const;
    void computeVolumeVectors();
    void getVolumeVectors(C3Vector &n, C3Vector &f) const;

    static bool getFrustumCullingEnabled();
    static void setFrustumCullingEnabled(bool e);

    std::vector<double> viewFrustum;

    static bool fogWasActivated;

  protected:
    // View frustum culling:
    bool _isBoxOutsideVolumeApprox(const C4X4Matrix &tr, const C3Vector &s, std::vector<double> *planes);

    double _nearClippingPlane;
    double _farClippingPlane;
    double _viewAngle;
    double _orthoViewSize;
    int _resolution[2];
    bool _perspective;
    bool _showVolume;
    C3Vector _volumeVectorNear;
    C3Vector _volumeVectorFar;

    bool _showFogIfAvailable;
    bool _useLocalLights;

    bool _planesCalculated;
    int _currentViewSize[2];
    bool _currentPerspective;
    bool _frustumCullingTemporarilyDisabled;

    double _initialViewAngle;
    double _initialOrthoViewSize;

    int _fogTimer;
    double _fogTimerDuration;

    int _disabledColorComponents; // not serialized

    static bool _frustumCulling;
};
