#pragma once

#include "_jointObject_.h"

class CJoint : public _CJoint_
{
public:
    CJoint(); // default, use only during serialization!
    CJoint(int jointType);
    virtual ~CJoint();

    // Overridden from CSceneObject:
    void buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting);
    void connectSynchronizationObject();

    // Following functions are inherited from CSceneObject
    void display(CViewableBase* renderingObject,int displayAttrib);
    void addSpecializedObjectEventData(CInterfaceStackTable* data) const;
    CSceneObject* copyYourself();
    void removeSceneDependencies();
    void scaleObject(float scalingFactor);
    void scaleObjectNonIsometrically(float x,float y,float z);
    void serialize(CSer& ar);
    void announceCollectionWillBeErased(int groupID,bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    bool announceObjectWillBeErased(int objectHandle,bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer);
    void performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performIkLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performCollectionLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performCollisionLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performDistanceLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performTextureObjectLoadingMapping(const std::vector<int>* map);
    void performDynMaterialObjectLoadingMapping(const std::vector<int>* map);
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void computeBoundingBox();
    bool getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const;

    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;

    float getMeasuredJointVelocity() const;
    std::string getDependencyJointLoadAlias() const;
    std::string getDependencyJointLoadName_old() const;
    int getJointCallbackCallOrder_backwardCompatibility() const;
    void setDirectDependentJoints(const std::vector<CJoint*>& joints);

    // Overridden from _CJoint_:
    bool setPositionIntervalMin(float min);
    bool setPositionIntervalRange(float range);
    bool setPositionIsCyclic(bool isCyclic);
    bool setLength(float l);
    bool setDiameter(float d);
    bool setScrewPitch(float pitch);
    bool setDependencyMasterJointHandle(int depJointID);
    bool setDependencyJointMult(float coeff);
    bool setDependencyJointOffset(float off);
    bool setIkWeight(float newWeight);
    bool setMaxStepSize(float stepS);
    bool setPosition(float pos);
    bool setSphericalTransformation(const C4Vector& tr);
    bool setJointMode(int theMode);
    bool setHybridFunctionality(bool h);
    bool setDynamicMotorTargetVelocity(float v);
    bool setDynamicMotorUpperLimitVelocity(float v);
    bool setDynamicMotorPositionControlTargetPosition(float pos);
    bool setDynamicMotorPositionControlParameters(float p_param,float i_param,float d_param);
    bool setDynamicMotorSpringControlParameters(float k_param,float c_param);
    bool setDynamicMotorMaximumForce(float f);
    bool setBulletFloatParams(const std::vector<float>& p);
    bool setOdeFloatParams(const std::vector<float>& p);
    bool setVortexFloatParams(const std::vector<float>& p);
    bool setNewtonFloatParams(const std::vector<float>& p);

    bool setJointMode_noDynMotorTargetPosCorrection(int theMode);


    void measureJointVelocity(float dt); // should be called just after the main script was called!!!

    void getDynamicJointErrors(float& linear,float& angular) const;
    void getDynamicJointErrorsFull(C3Vector& linear,C3Vector& angular) const;

    void handleDynJointControl(bool init,int loopCnt,int totalLoops,float currentPos,float effort,float dynStepSize,float errorV,float& velocity,float& forceTorque);

    void setDynamicMotorReflectedPosition_useOnlyFromDynamicPart(float rfp);

    void addCumulativeForceOrTorque(float forceOrTorque,int countForAverage);
    bool getDynamicForceOrTorque(float& forceOrTorque,bool dynamicStepValue) const;
    void setForceOrTorqueNotValid();


    bool setEngineFloatParam(int what,float v);
    bool setEngineIntParam(int what,int v);
    bool setEngineBoolParam(int what,bool v);



    void copyEnginePropertiesTo(CJoint* target);


    // DEPRECATED:
    void setExplicitHandling_DEPRECATED(bool explicitHandl);
    bool getExplicitHandling_DEPRECATED();
    void resetJoint_DEPRECATED();
    void handleJoint_DEPRECATED(float deltaTime);
    void setUnlimitedAcceleration_DEPRECATED(bool unlimited);
    bool getUnlimitedAcceleration_DEPRECATED();
    void setInvertTargetVelocityAtLimits_DEPRECATED(bool invert);
    bool getInvertTargetVelocityAtLimits_DEPRECATED();
    void setMaxAcceleration_DEPRECATED(float maxAccel);
    float getMaxAcceleration_DEPRECATED();
    void setVelocity_DEPRECATED(float vel);
    float getVelocity_DEPRECATED();
    void setTargetVelocity_DEPRECATED(float vel);
    float getTargetVelocity_DEPRECATED();

protected:
    void _rectifyDependentJoints();

private:
    void _commonInit();

    // Overridden from _CJoint_:
    void _setPositionIntervalMin_send(float min) const;
    void _setPositionIntervalRange_send(float range) const;
    void _setPositionIsCyclic_send(bool isCyclic) const;
    void _setScrewPitch_send(float pitch) const;
    void _setDependencyJointHandle_send(int depJointID) const;
    void _setDependencyJointMult_send(float coeff) const;
    void _setDependencyJointOffset_send(float off) const;
    void _setIkWeight_send(float newWeight) const;
    void _setMaxStepSize_send(float stepS) const;
    void _setPosition_send(float pos) const;
    void _setSphericalTransformation_send(const C4Vector& tr) const;
    void _setJointMode_send(int theMode) const;

    std::vector<CJoint*> _directDependentJoints;

    float _dynamicMotorPIDCumulativeErrorForIntegralParameter;
    float _dynamicMotorPIDLastErrorForDerivativeParameter;

    std::string _dependencyJointLoadAlias;
    std::string _dependencyJointLoadName_old;

    float _initialPosition;
    C4Vector _initialSphericalJointTransformation;
    float _initialTargetPosition;
    bool _initialDynamicMotorEnabled;
    float _initialDynamicMotorTargetVelocity;
    bool _initialDynamicMotorLockModeWhenInVelocityControl;
    float _initialDynamicMotorUpperLimitVelocity;
    float _initialDynamicMotorMaximumForce;

    bool _initialDynamicMotorControlLoopEnabled;
    float _initialDynamicMotorPositionControl_P;
    float _initialDynamicMotorPositionControl_I;
    float _initialDynamicMotorPositionControl_D;
    float _initialDynamicMotorSpringControl_K;
    float _initialDynamicMotorSpringControl_C;
    int _initialJointMode;
    bool _initialHybridOperation;
    int _initialJointCallbackCallOrder;

    float _measuredJointVelocity_velocityMeasurement;
    float _previousJointPosition_velocityMeasurement;
    bool _previousJointPositionIsValid;

    float _cumulatedForceOrTorque;
    float _cumulativeForceOrTorqueTmp;
    bool _averageForceOrTorqueValid;

    // Following is the force/torque acquired during a single dyn. calculation step:
    float _lastForceOrTorque_dynStep;
    bool _lastForceOrTorqueValid_dynStep;

    // DEPRECATED:
    float _jointPositionForMotionHandling_DEPRECATED;
    float _velocity_DEPRECATED;
    float _targetVelocity_DEPRECATED;
    bool _explicitHandling_DEPRECATED;
    bool _unlimitedAcceleration_DEPRECATED;
    bool _invertTargetVelocityAtLimits_DEPRECATED;
    float _maxAcceleration_DEPRECATED;
    int _jointCallbackCallOrder_backwardCompatibility;
    float _initialVelocity_DEPRECATED;
    float _initialTargetVelocity_DEPRECATED;
    bool _initialExplicitHandling_DEPRECATED;
};
