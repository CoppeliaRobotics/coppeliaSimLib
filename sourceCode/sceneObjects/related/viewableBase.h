#pragma once

#include "sceneObject.h"
#include "ser.h"

class CViewableBase : public CSceneObject
{
public:
    CViewableBase();
    virtual ~CViewableBase();

    // Following functions need to be implemented in each class derived from CViewableBase
    virtual CSceneObject* copyYourself();
    virtual void display(CViewableBase* renderingObject,int displayAttrib);
    virtual void scaleObject(floatDouble scalingFactor);
    virtual void serialize(CSer& ar);
    virtual void announceObjectWillBeErased(const CSceneObject* object,bool copyBuffer);
    virtual void announceCollectionWillBeErased(int groupID,bool copyBuffer);
    virtual void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    virtual void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    virtual void performObjectLoadingMapping(const std::map<int,int>* map);
    virtual void performCollectionLoadingMapping(const std::map<int,int>* map);
    virtual void performCollisionLoadingMapping(const std::map<int,int>* map);
    virtual void performDistanceLoadingMapping(const std::map<int,int>* map);
    virtual void simulationAboutToStart();
    virtual void simulationEnded();

    virtual std::string getObjectTypeInfo() const;
    virtual std::string getObjectTypeInfoExtended() const;
    virtual bool isPotentiallyCollidable() const;
    virtual bool isPotentiallyMeasurable() const;
    virtual bool isPotentiallyDetectable() const;
    virtual bool isPotentiallyRenderable() const;


    void setNearClippingPlane(floatDouble nearPlane);
    floatDouble getNearClippingPlane() const;
    void setFarClippingPlane(floatDouble farPlane);
    floatDouble getFarClippingPlane() const;
    void setViewAngle(floatDouble angle);
    floatDouble getViewAngle() const;
    void setOrthoViewSize(floatDouble theSize);
    floatDouble getOrthoViewSize() const;

    void setShowFogIfAvailable(bool showFog);
    bool getShowFogIfAvailable() const;

    bool isObjectInsideView(const C7Vector& objectM,const C3Vector& maxBB);
    void computeViewFrustumIfNeeded();

    void setFrustumCullingTemporarilyDisabled(bool d);

    void setFogTimer(floatDouble seconds);
    floatDouble getFogStrength();

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
    void getVolumeVectors(C3Vector& n,C3Vector& f) const;

    static bool getFrustumCullingEnabled();
    static void setFrustumCullingEnabled(bool e);

    std::vector<floatDouble> viewFrustum;

    static bool fogWasActivated;

protected:
    // View frustum culling:
    bool _isBoxOutsideVolumeApprox(const C4X4Matrix& tr,const C3Vector& s,std::vector<floatDouble>* planes);
    
    floatDouble _nearClippingPlane;
    floatDouble _farClippingPlane;
    floatDouble _viewAngle;
    floatDouble _orthoViewSize;
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

    floatDouble _initialViewAngle;
    floatDouble _initialOrthoViewSize;

    int _fogTimer;
    floatDouble _fogTimerDuration;

    int _disabledColorComponents; // not serialized

    static bool _frustumCulling;
};
