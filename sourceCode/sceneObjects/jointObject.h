#pragma once

#include "sceneObject.h"

enum { /* Bullet joint float params */
    simi_bullet_joint_stoperp=0,
    simi_bullet_joint_stopcfm,
    simi_bullet_joint_normalcfm,
    simi_bullet_joint_free1,
    simi_bullet_joint_free2,
    simi_bullet_joint_pospid1,
    simi_bullet_joint_pospid2,
    simi_bullet_joint_pospid3,
};

enum { /* Ode joint float params */
    simi_ode_joint_stoperp=0,
    simi_ode_joint_stopcfm,
    simi_ode_joint_bounce,
    simi_ode_joint_fudgefactor,
    simi_ode_joint_normalcfm,
    simi_ode_joint_pospid1,
    simi_ode_joint_pospid2,
    simi_ode_joint_pospid3,
};

enum { /* Vortex joint float params */
    simi_vortex_joint_lowerlimitdamping=0,
    simi_vortex_joint_upperlimitdamping,
    simi_vortex_joint_lowerlimitstiffness,
    simi_vortex_joint_upperlimitstiffness,
    simi_vortex_joint_lowerlimitrestitution,
    simi_vortex_joint_upperlimitrestitution,
    simi_vortex_joint_lowerlimitmaxforce,
    simi_vortex_joint_upperlimitmaxforce,
    simi_vortex_joint_motorconstraintfrictioncoeff,
    simi_vortex_joint_motorconstraintfrictionmaxforce,
    simi_vortex_joint_motorconstraintfrictionloss,
    simi_vortex_joint_p0loss,
    simi_vortex_joint_p0stiffness,
    simi_vortex_joint_p0damping,
    simi_vortex_joint_p0frictioncoeff,
    simi_vortex_joint_p0frictionmaxforce,
    simi_vortex_joint_p0frictionloss,
    simi_vortex_joint_p1loss,
    simi_vortex_joint_p1stiffness,
    simi_vortex_joint_p1damping,
    simi_vortex_joint_p1frictioncoeff,
    simi_vortex_joint_p1frictionmaxforce,
    simi_vortex_joint_p1frictionloss,
    simi_vortex_joint_p2loss,
    simi_vortex_joint_p2stiffness,
    simi_vortex_joint_p2damping,
    simi_vortex_joint_p2frictioncoeff,
    simi_vortex_joint_p2frictionmaxforce,
    simi_vortex_joint_p2frictionloss,
    simi_vortex_joint_a0loss,
    simi_vortex_joint_a0stiffness,
    simi_vortex_joint_a0damping,
    simi_vortex_joint_a0frictioncoeff,
    simi_vortex_joint_a0frictionmaxforce,
    simi_vortex_joint_a0frictionloss,
    simi_vortex_joint_a1loss,
    simi_vortex_joint_a1stiffness,
    simi_vortex_joint_a1damping,
    simi_vortex_joint_a1frictioncoeff,
    simi_vortex_joint_a1frictionmaxforce,
    simi_vortex_joint_a1frictionloss,
    simi_vortex_joint_a2loss,
    simi_vortex_joint_a2stiffness,
    simi_vortex_joint_a2damping,
    simi_vortex_joint_a2frictioncoeff,
    simi_vortex_joint_a2frictionmaxforce,
    simi_vortex_joint_a2frictionloss,
    simi_vortex_joint_dependencyfactor, // deprecated
    simi_vortex_joint_dependencyoffset, // deprecated
    simi_vortex_joint_free1,
    simi_vortex_joint_free2,
    simi_vortex_joint_pospid1,
    simi_vortex_joint_pospid2,
    simi_vortex_joint_pospid3,
};

enum { /* Vortex joint int params */
    simi_vortex_joint_bitcoded=0,
    simi_vortex_joint_relaxationenabledbc,
    simi_vortex_joint_frictionenabledbc,
    simi_vortex_joint_frictionproportionalbc,
    simi_vortex_joint_objectid, // deprecated
    simi_vortex_joint_dependentobjectid // deprecated
};

enum { /* Vortex joint bit params */
    simi_vortex_joint_motorfrictionenabled=1,
    simi_vortex_joint_proportionalmotorfriction=2
};

enum { /* Newton joint float params */
    simi_newton_joint_dependencyfactor=0, // deprecated
    simi_newton_joint_dependencyoffset, // deprecated
    simi_newton_joint_pospid1,
    simi_newton_joint_pospid2,
    simi_newton_joint_pospid3,
};

enum { /* Newton joint int params */
    simi_newton_joint_objectid=0, // deprecated
    simi_newton_joint_dependentobjectid // deprecated
};

enum { /* Mujoco joint float params */
    simi_mujoco_joint_solreflimit1=0,
    simi_mujoco_joint_solreflimit2,
    simi_mujoco_joint_solimplimit1,
    simi_mujoco_joint_solimplimit2,
    simi_mujoco_joint_solimplimit3,
    simi_mujoco_joint_solimplimit4,
    simi_mujoco_joint_solimplimit5,
    simi_mujoco_joint_frictionloss,
    simi_mujoco_joint_solreffriction1,
    simi_mujoco_joint_solreffriction2,
    simi_mujoco_joint_solimpfriction1,
    simi_mujoco_joint_solimpfriction2,
    simi_mujoco_joint_solimpfriction3,
    simi_mujoco_joint_solimpfriction4,
    simi_mujoco_joint_solimpfriction5,
    simi_mujoco_joint_stiffness,
    simi_mujoco_joint_damping,
    simi_mujoco_joint_springref,
    simi_mujoco_joint_springdamper1,
    simi_mujoco_joint_springdamper2,
    simi_mujoco_joint_armature,
    simi_mujoco_joint_margin,
    simi_mujoco_joint_polycoef1, // deprecated
    simi_mujoco_joint_polycoef2, // deprecated
    simi_mujoco_joint_polycoef3,
    simi_mujoco_joint_polycoef4,
    simi_mujoco_joint_polycoef5,
    simi_mujoco_joint_pospid1,
    simi_mujoco_joint_pospid2,
    simi_mujoco_joint_pospid3,
};

enum { /* Mujoco joint int params */
    simi_mujoco_joint_objectid=0, // deprecated
    simi_mujoco_joint_dependentobjectid // deprecated
};


class CJoint : public CSceneObject
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
    void scaleObject(floatDouble scalingFactor);
    void scaleObjectNonIsometrically(floatDouble x,floatDouble y,floatDouble z);
    void serialize(CSer& ar);
    void announceCollectionWillBeErased(int groupID,bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    void announceObjectWillBeErased(const CSceneObject* object,bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer);
    void performObjectLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performIkLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performCollectionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performCollisionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performDistanceLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performTextureObjectLoadingMapping(const std::map<int,int>* map);
    void performDynMaterialObjectLoadingMapping(const std::map<int,int>* map);
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void computeBoundingBox();
    C7Vector getFullLocalTransformation() const;

    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;

    floatDouble getLength() const;
    floatDouble getDiameter() const;
    floatDouble getPosition() const;
    floatDouble getTargetVelocity() const;
    bool getMotorLock() const;
    floatDouble getTargetForce(bool signedValue) const;
    int getDynCtrlMode() const;
    int getDynVelCtrlType() const;
    int getDynPosCtrlType() const;
    floatDouble getEngineFloatParam(int what,bool* ok) const;
    int getEngineIntParam(int what,bool* ok) const;
    bool getEngineBoolParam(int what,bool* ok) const;
    void getBulletFloatParams(std::vector<floatDouble>& p) const;
    void getBulletIntParams(std::vector<int>& p) const;
    void getOdeFloatParams(std::vector<floatDouble>& p) const;
    void getOdeIntParams(std::vector<int>& p) const;
    void getVortexFloatParams(std::vector<floatDouble>& p) const;
    void getVortexIntParams(std::vector<int>& p) const;
    void getNewtonFloatParams(std::vector<floatDouble>& p) const;
    void getNewtonIntParams(std::vector<int>& p) const;
    void getMujocoFloatParams(std::vector<floatDouble>& p) const;
    void getMujocoIntParams(std::vector<int>& p) const;
    int getVortexDependentJointId() const;
    int getNewtonDependentJointId() const;
    int getMujocoDependentJointId() const;
    void getMaxVelAccelJerk(floatDouble maxVelAccelJerk[3]) const;
    floatDouble getScrewPitch() const;
    int getJointType() const;
    C4Vector getSphericalTransformation() const;
    bool getIsCyclic() const;
    floatDouble getPositionMin() const;
    floatDouble getPositionRange() const;
    int getJointMode() const;
    int getDependencyMasterJointHandle() const;
    floatDouble getDependencyJointMult() const;
    floatDouble getDependencyJointOffset() const;
    void getPid(floatDouble& p_param,floatDouble& i_param,floatDouble& d_param,int engine=-1) const;
    void getKc(floatDouble& k_param,floatDouble& c_param) const;
    floatDouble getTargetPosition() const;
    CColorObject* getColor(bool part2);

    C7Vector getIntrinsicTransformation(bool includeDynErrorComponent) const;

    floatDouble getMeasuredJointVelocity() const;
    std::string getDependencyJointLoadAlias() const;
    int getJointCallbackCallOrder_backwardCompatibility() const;
    void setDirectDependentJoints(const std::vector<CJoint*>& joints);

    void setPositionMin(floatDouble min);
    void setPositionRange(floatDouble range);
    void setIsCyclic(bool isCyclic);
    void setLength(floatDouble l);
    void setDiameter(floatDouble d);
    void setMaxVelAccelJerk(const floatDouble maxVelAccelJerk[3]);
    bool setScrewPitch(floatDouble pitch);
    void setDependencyMasterJointHandle(int depJointID);
    void setDependencyJointMult(floatDouble coeff);
    void setDependencyJointOffset(floatDouble off);
    void setVelocity(floatDouble vel,const CJoint* masterJoint=nullptr);
    void setPosition(floatDouble pos,const CJoint* masterJoint=nullptr,bool setDirect=false);
    void setSphericalTransformation(const C4Vector& tr);
    void setJointMode(int theMode);

    void setIntrinsicTransformationError(const C7Vector& tr);

    void setTargetVelocity(floatDouble v);
    void setTargetPosition(floatDouble pos);
    void setKc(floatDouble k_param,floatDouble c_param);
    void setTargetForce(floatDouble f,bool isSigned);
    void setDynCtrlMode(int mode);
    void setDynVelCtrlType(int mode);
    void setDynPosCtrlType(int mode);

    void setBulletFloatParams(const std::vector<floatDouble>& p);
    void setOdeFloatParams(const std::vector<floatDouble>& p);
    void setVortexFloatParams(const std::vector<floatDouble>& p);
    void setNewtonFloatParams(const std::vector<floatDouble>& p);
    void setMujocoFloatParams(const std::vector<floatDouble>& p);
    void setBulletIntParams(const std::vector<int>& p);
    void setOdeIntParams(const std::vector<int>& p);
    void setVortexIntParams(const std::vector<int>& p);
    void setNewtonIntParams(const std::vector<int>& p);
    void setMujocoIntParams(const std::vector<int>& p);

    bool setJointMode_noDynMotorTargetPosCorrection(int newMode);

    void setMotorLock(bool e);


    void measureJointVelocity(floatDouble simTime);

    void getDynamicJointErrors(floatDouble& linear,floatDouble& angular) const;
    void getDynamicJointErrorsFull(C3Vector& linear,C3Vector& angular) const;

    void handleMotion();
    int handleDynJoint(int flags,const int intVals[3],floatDouble currentPosVelAccel[3],floatDouble effort,floatDouble dynStepSize,floatDouble errorV,floatDouble velAndForce[2]);

    void setDynamicMotorReflectedPosition_useOnlyFromDynamicPart(floatDouble rfp,floatDouble simTime);

    void addCumulativeForceOrTorque(floatDouble forceOrTorque,int countForAverage);
    bool getDynamicForceOrTorque(floatDouble& forceOrTorque,bool dynamicStepValue) const;
    void setForceOrTorqueNotValid();

    void setKinematicMotionType(int t,bool reset,floatDouble initVel=0.0f);

    bool setEngineFloatParam(int what,floatDouble v);
    bool setEngineIntParam(int what,int v);
    bool setEngineBoolParam(int what,bool v);



    void copyEnginePropertiesTo(CJoint* target);


    // DEPRECATED:
    bool getHybridFunctionality_old() const;
    floatDouble getIKWeight_old() const;
    floatDouble getMaxStepSize_old() const;
    std::string getDependencyJointLoadName_old() const;
    void setIKWeight_old(floatDouble newWeight);
    void setMaxStepSize_old(floatDouble stepS);
    void setHybridFunctionality_old(bool h);
    void setExplicitHandling_DEPRECATED(bool explicitHandl);
    bool getExplicitHandling_DEPRECATED();
    void resetJoint_DEPRECATED();
    void handleJoint_DEPRECATED(floatDouble deltaTime);
    void setUnlimitedAcceleration_DEPRECATED(bool unlimited);
    bool getUnlimitedAcceleration_DEPRECATED();
    void setInvertTargetVelocityAtLimits_DEPRECATED(bool invert);
    bool getInvertTargetVelocityAtLimits_DEPRECATED();
    void setMaxAcceleration_DEPRECATED(floatDouble maxAccel);
    floatDouble getMaxAcceleration_DEPRECATED();
    void setVelocity_DEPRECATED(floatDouble vel);
    floatDouble getVelocity_DEPRECATED();
    void setPid_old(floatDouble p_param,floatDouble i_param,floatDouble d_param);

protected:
    void updateSelfAsSlave();
    void _fixVortexInfVals();

    void _commonInit();
    void _sendDependencyChange() const;

    void _setPositionIntervalMin_sendOldIk(floatDouble min) const;
    void _setPositionIntervalRange_sendOldIk(floatDouble range) const;
    void _setPositionIsCyclic_sendOldIk(bool isCyclic) const;
    void _setScrewPitch_sendOldIk(floatDouble pitch) const;
    void _setDependencyJointHandle_sendOldIk(int depJointID) const;
    void _setDependencyJointMult_sendOldIk(floatDouble coeff) const;
    void _setDependencyJointOffset_sendOldIk(floatDouble off) const;
    void _setIkWeight_sendOldIk(floatDouble newWeight) const;
    void _setMaxStepSize_sendOldIk(floatDouble stepS) const;
    void _setPosition_sendOldIk(floatDouble pos) const;
    void _setSphericalTransformation_sendOldIk(const C4Vector& tr) const;
    void _setJointMode_sendOldIk(int theMode) const;

    std::vector<CJoint*> _directDependentJoints;


    std::string _dependencyJointLoadAlias;

    floatDouble _initialPosition;
    floatDouble _initialTargetPosition;
    floatDouble _initialTargetVelocity;
    C4Vector _initialSphericalJointTransformation;
    int _initialJointMode;

    int _initialDynCtrlMode;
    int _initialDynVelocityCtrlType;
    int _initialDynPositionCtrlType;
    bool _initialDynCtrl_lockAtVelZero;
    floatDouble _initialDynCtrl_force;
    floatDouble _initialDynCtrl_kc[3];

    bool _initialHybridOperation;

    floatDouble _initialMaxVelAccelJerk[3];

    floatDouble _velCalc_vel;
    floatDouble _velCalc_prevPos;
    floatDouble _velCalc_prevSimTime;
    bool _velCalc_prevPosValid;

    double _dynPosCtrl_currentVelAccel[2];
    double _dynVelCtrl_currentVelAccel[2];
    floatDouble _dynCtrl_previousVelForce[2];

    floatDouble _cumulatedForceOrTorque;
    floatDouble _cumulativeForceOrTorqueTmp;
    bool _averageForceOrTorqueValid;
    int _kinematicMotionType; // 0=none, 1=pos ctrl, 2=vel ctrl, bit4 (16): reset motion
    floatDouble _kinematicMotionInitVel;

    int _jointType;
    floatDouble _length;
    floatDouble _diameter;
    C4Vector _sphericalTransf; // spherical joints don't have a range anymore since 22.10.2022
    bool _isCyclic;
    floatDouble _screwPitch;
    floatDouble _posMin;
    floatDouble _posRange;
    floatDouble _pos;
    floatDouble _targetVel; // signed. Should be same sign as _targetForce (however _targetVel has precedence when conflict)
    floatDouble _targetPos;
    int _jointMode;
    int _dependencyMasterJointHandle;
    floatDouble _dependencyJointMult;
    floatDouble _dependencyJointOffset;

    floatDouble _maxVelAccelJerk[3];

    CColorObject _color;
    CColorObject _color_removeSoon;

    bool _motorLock; // deprecated, should not be used anymore
    floatDouble _targetForce; // signed. Should be same sign as _targetVel (however _targetVel has precedence when conflict)
    floatDouble _dynCtrl_pid_cumulErr;

    floatDouble _dynCtrl_kc[2];
    int _dynCtrlMode;
    int _dynPositionCtrlType; // built-in velocity mode + pos PID (0) or Ruckig (1)
    int _dynVelocityCtrlType; // built-in velocity mode (0) or Ruckig (1)

    bool _jointHasHybridFunctionality;


    std::vector<floatDouble> _bulletFloatParams;
    std::vector<int> _bulletIntParams;

    std::vector<floatDouble> _odeFloatParams;
    std::vector<int> _odeIntParams;

    std::vector<floatDouble> _vortexFloatParams;
    std::vector<int> _vortexIntParams;

    std::vector<floatDouble> _newtonFloatParams;
    std::vector<int> _newtonIntParams;

    std::vector<floatDouble> _mujocoFloatParams;
    std::vector<int> _mujocoIntParams;

    C7Vector _intrinsicTransformationError; // from physics engine

    // Following is the force/torque acquired during a single dyn. calculation step:
    floatDouble _lastForceOrTorque_dynStep;
    bool _lastForceOrTorqueValid_dynStep;

    // DEPRECATED:
    floatDouble _maxStepSize_old;
    floatDouble _ikWeight_old;
    std::string _dependencyJointLoadName_old;
    floatDouble _jointPositionForMotionHandling_DEPRECATED;
    floatDouble _velocity_DEPRECATED;
    bool _explicitHandling_DEPRECATED;
    bool _unlimitedAcceleration_DEPRECATED;
    bool _invertTargetVelocityAtLimits_DEPRECATED;
    floatDouble _maxAcceleration_DEPRECATED;
    int _jointCallbackCallOrder_backwardCompatibility;
    floatDouble _initialVelocity_DEPRECATED;
    bool _initialExplicitHandling_DEPRECATED;
};
