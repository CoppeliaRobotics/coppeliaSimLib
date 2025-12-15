#pragma once

#include <sceneObject.h>
#include <ser.h>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
VIEWABLEBASE_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_viewable;
// ----------------------------------------------------------------------------------------------

class CViewableBase : public CSceneObject
{
  public:
    CViewableBase();
    virtual ~CViewableBase();

    // Following functions need to be implemented in each class derived from CViewableBase
    virtual void addSpecializedObjectEventData(CCbor* ev) override;
    virtual CSceneObject* copyYourself() override;
#ifdef SIM_WITH_GUI
    virtual void display(CViewableBase* renderingObject, int displayAttrib) override;
#else
    virtual void display(CViewableBase* renderingObject, int displayAttrib);
#endif
    virtual void scaleObject(double scalingFactor) override;
    virtual void serialize(CSer& ar) override;
    virtual void announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer) override;
    virtual void announceCollectionWillBeErased(int groupID, bool copyBuffer) override;
    virtual void announceCollisionWillBeErased(int collisionID, bool copyBuffer) override;
    virtual void announceDistanceWillBeErased(int distanceID, bool copyBuffer) override;
    virtual void performObjectLoadingMapping(const std::map<int, int>* map);
    virtual void performCollectionLoadingMapping(const std::map<int, int>* map);
    virtual void performCollisionLoadingMapping(const std::map<int, int>* map);
    virtual void performDistanceLoadingMapping(const std::map<int, int>* map);
    virtual void simulationAboutToStart() override;
    virtual void simulationEnded() override;

    virtual std::string getObjectTypeInfo() const override;
    virtual std::string getObjectTypeInfoExtended() const override;
    virtual bool isPotentiallyCollidable() const override;
    virtual bool isPotentiallyMeasurable() const override;
    virtual bool isPotentiallyDetectable() const override;
    virtual bool isPotentiallyRenderable() const override;

    virtual int setBoolProperty(const char* pName, bool pState) override;
    virtual int getBoolProperty(const char* pName, bool& pState) const override;
    virtual int setFloatProperty(const char* pName, double pState) override;
    virtual int getFloatProperty(const char* pName, double& pState) const override;
    virtual int setIntArray2Property(const char* pName, const int* pState) override;
    virtual int getIntArray2Property(const char* pName, int* pState) const override;
    virtual int setVector2Property(const char* pName, const double* pState) override;
    virtual int getVector2Property(const char* pName, double* pState) const override;
    virtual int setVector3Property(const char* pName, const C3Vector& pState) override;
    virtual int getVector3Property(const char* pName, C3Vector& pState) const override;
    virtual int setFloatArrayProperty(const char* pName, const double* v, int vL) override;
    virtual int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const override;
    virtual int setIntArrayProperty(const char* pName, const int* v, int vL) override;
    virtual int getIntArrayProperty(const char* pName, std::vector<int>& pState) const override;
    static int getPropertyName_vstatic(int& index, std::string& pName, int excludeFlags);
    static int getPropertyInfo_vstatic(const char* pName, int& info, std::string& infoTxt);

    void setClippingPlanes(double nearPlane, double farPlane);
    void getClippingPlanes(double& nearPlane, double& farPlane) const;
    void setViewAngle(double angle);
    double getViewAngle() const;
    void setOrthoViewSize(double theSize);
    double getOrthoViewSize() const;

    void setHideFog(bool hideFog);
    bool getHideFog() const;

    bool isObjectInsideView(const C7Vector& objectM, const C3Vector& maxBB);
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
    void getVolumeVectors(C3Vector& n, C3Vector& f) const;

    static bool getFrustumCullingEnabled();
    static void setFrustumCullingEnabled(bool e);

    std::vector<double> viewFrustum;

    static bool fogWasActivated;

  protected:
    // View frustum culling:
    bool _isBoxOutsideVolumeApprox(const C4X4Matrix& tr, const C3Vector& s, std::vector<double>* planes);

    double _nearClippingPlane;
    double _farClippingPlane;
    double _viewAngle;
    double _orthoViewSize;
    int _resolution[2];
    bool _perspective;
    bool _showVolume;
    C3Vector _volumeVectorNear;
    C3Vector _volumeVectorFar;

    bool _hideFog;
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
