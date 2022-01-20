
#pragma once

#include "sceneObject.h"

class CForceSensor : public CSceneObject  
{
public:
    CForceSensor();
    virtual ~CForceSensor();

    // Following functions are inherited from CSceneObject
    void display(CViewableBase* renderingObject,int displayAttrib);
    void addSpecializedObjectEventData(CInterfaceStackTable* data) const;
    CSceneObject* copyYourself();
    void removeSceneDependencies();
    void scaleObject(float scalingFactor);
    void scaleObjectNonIsometrically(float x,float y,float z);
    void serialize(CSer& ar);
    void announceObjectWillBeErased(const CSceneObject* object,bool copyBuffer);
    void announceCollectionWillBeErased(int groupID,bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer);
    void performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performCollectionLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performCollisionLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performDistanceLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performIkLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performTextureObjectLoadingMapping(const std::vector<int>* map);
    void performDynMaterialObjectLoadingMapping(const std::vector<int>* map);
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void computeBoundingBox();
    bool getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const;
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;

    C7Vector getIntrinsicTransformation(bool includeDynErrorComponent) const;

    // Overridden from _CSceneObject_:
    virtual C7Vector getFullLocalTransformation() const;

    void commonInit();

    void setIntrinsicTransformationError(const C7Vector& tr);

    void addCumulativeForcesAndTorques(const C3Vector& f,const C3Vector& t,int countForAverage);
    void setForceAndTorqueNotValid();

    bool getDynamicForces(C3Vector& f,bool dynamicStepValue) const;
    bool getDynamicTorques(C3Vector& t,bool dynamicStepValue) const;

    float getDynamicPositionError() const;
    float getDynamicOrientationError() const;
    void getDynamicErrorsFull(C3Vector& linear,C3Vector& angular) const;

    bool getStillAutomaticallyBreaking();
    void setForceThreshold(float t);
    float getForceThreshold() const;
    void setTorqueThreshold(float t);
    float getTorqueThreshold() const;
    void setEnableForceThreshold(bool e);
    bool getEnableForceThreshold() const;
    void setEnableTorqueThreshold(bool e);
    bool getEnableTorqueThreshold() const;
    void setConsecutiveThresholdViolationsForBreaking(int count);
    int getConsecutiveThresholdViolationsForBreaking() const;

    void setValueCountForFilter(int c);
    int getValueCountForFilter() const;
    void setFilterType(int t);
    int getFilterType() const;

    // Various
    void setForceSensorSize(float s);
    float getForceSensorSize() const;

    CColorObject* getColor(bool part2);

protected:
    void _computeFilteredValues();
    void _handleSensorBreaking();

    float _forceThreshold;
    float _torqueThreshold;
    int _valueCountForFilter;
    int _filterType; //0=average, 1=median
    bool _forceThresholdEnabled;
    bool _torqueThresholdEnabled;
    bool _stillAutomaticallyBreaking;

    int _consecutiveThresholdViolationsForBreaking;
    int _currentThresholdViolationCount;

    C7Vector _intrinsicTransformationError; // from physics engine

    // Variables which need to be serialized & copied
    // Visual attributes:
    float _forceSensorSize;
    CColorObject _color;
    CColorObject _color_removeSoon;

    // Dynamic values:
    std::vector<C3Vector> _cumulatedForces;
    std::vector<C3Vector> _cumulatedTorques;

    C3Vector _cumulativeForcesTmp;
    C3Vector _cumulativeTorquesTmp;

    // Following are forces/torques acquired during a single dyn. calculation step:
    C3Vector _lastForce_dynStep;
    C3Vector _lastTorque_dynStep;
    bool _lastForceAndTorqueValid_dynStep;

    C3Vector _filteredDynamicForces; // do not serialize! (but initialize appropriately)
    C3Vector _filteredDynamicTorques; // do not serialize! (but initialize appropriately)
    bool _filteredValuesAreValid;
};
