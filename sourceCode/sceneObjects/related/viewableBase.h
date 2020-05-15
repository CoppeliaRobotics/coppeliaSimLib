
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
    virtual void scaleObject(float scalingFactor);
    virtual void serialize(CSer& ar);
    virtual bool announceObjectWillBeErased(int objID,bool copyBuffer);
    virtual void announceCollectionWillBeErased(int groupID,bool copyBuffer);
    virtual void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    virtual void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    virtual void performObjectLoadingMapping(const std::vector<int>* map);
    virtual void performCollectionLoadingMapping(const std::vector<int>* map);
    virtual void performCollisionLoadingMapping(const std::vector<int>* map);
    virtual void performDistanceLoadingMapping(const std::vector<int>* map);
    virtual void simulationAboutToStart();
    virtual void simulationEnded();

    virtual std::string getObjectTypeInfo() const;
    virtual std::string getObjectTypeInfoExtended() const;
    virtual bool isPotentiallyCollidable() const;
    virtual bool isPotentiallyMeasurable() const;
    virtual bool isPotentiallyDetectable() const;
    virtual bool isPotentiallyRenderable() const;


    void setNearClippingPlane(float nearPlane);
    float getNearClippingPlane();
    void setFarClippingPlane(float farPlane);
    float getFarClippingPlane();
    void setViewAngle(float angle);
    float getViewAngle();
    void setOrthoViewSize(float theSize);
    float getOrthoViewSize();

    void setShowFogIfAvailable(bool showFog);
    bool getShowFogIfAvailable();

    bool isObjectInsideView(const C7Vector& objectM,const C3Vector& maxBB);
    void computeViewFrustumIfNeeded();

    void setFrustumCullingTemporarilyDisabled(bool d);

    void setFogTimer(float seconds);
    float getFogStrength();

    void setUseLocalLights(bool l);
    bool getuseLocalLights();

    int getDisabledColorComponents();
    void setDisabledColorComponents(int c);

    static bool getFrustumCullingEnabled();
    static void setFrustumCullingEnabled(bool e);

    std::vector<float> viewFrustum;

    static bool fogWasActivated;

protected:
    // View frustum culling:
    bool _isBoxOutsideVolumeApprox(const C4X4Matrix& tr,const C3Vector& s,std::vector<float>* planes);
    
    float _nearClippingPlane;
    float _farClippingPlane;
    float _viewAngle;
    float _orthoViewSize;
    bool _showFogIfAvailable;
    bool _useLocalLights;

    bool _planesCalculated;
    int _currentViewSize[2];
    bool _currentPerspective;
    bool _frustumCullingTemporarilyDisabled;

    float _initialViewAngle;
    float _initialOrthoViewSize;

    int _fogTimer;
    float _fogTimerDuration;

    int _disabledColorComponents; // not serialized

    static bool _frustumCulling;
};
