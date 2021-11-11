#pragma once

#include <string>
#include "MyMath.h"
#include "colorObject.h"
#include "sceneObject.h"

enum { /* Bullet joint float params */
    simi_bullet_joint_stoperp=0,
    simi_bullet_joint_stopcfm,
    simi_bullet_joint_normalcfm
};

// enum { /* Bullet joint int params */
//     /* if you add something here, search for bji11032016 */
// };
// enum { /* Bullet joint bit params */
//     /* if you add something here, search for bjb11032016 */
// };

enum { /* Ode joint float params */
    simi_ode_joint_stoperp=0,
    simi_ode_joint_stopcfm,
    simi_ode_joint_bounce,
    simi_ode_joint_fudgefactor,
    simi_ode_joint_normalcfm
};

// enum { /* Ode joint int params */
//     /* if you add something here, search for oji11032016 */
// };
// enum { /* Ode joint bit params */
//     /* if you add something here, search for ojb11032016 */
// };

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
    simi_vortex_joint_dependencyfactor,
    simi_vortex_joint_dependencyoffset
};

enum { /* Vortex joint int params */
    simi_vortex_joint_bitcoded=0,
    simi_vortex_joint_relaxationenabledbc,
    simi_vortex_joint_frictionenabledbc,
    simi_vortex_joint_frictionproportionalbc,
    simi_vortex_joint_objectid,
    simi_vortex_joint_dependentobjectid
};

enum { /* Vortex joint bit params */
    simi_vortex_joint_motorfrictionenabled=1,
    simi_vortex_joint_proportionalmotorfriction=2
};

enum { /* Newton joint float params */
    simi_newton_joint_dependencyfactor=0,
    simi_newton_joint_dependencyoffset
};

enum { /* Newton joint int params */
    simi_newton_joint_objectid=0,
    simi_newton_joint_dependentobjectid
};

// enum { /* Newton joint bit params */
//     /* if you add something here, search for njb11032016 */
// };

enum {
    sim_syncobj_joint_intervalmin=0,
    sim_syncobj_joint_intervalrange,
    sim_syncobj_joint_intervalcyclic,
    sim_syncobj_joint_diameter,
    sim_syncobj_joint_length,
    sim_syncobj_joint_pitch,
    sim_syncobj_joint_dependencyhandle,
    sim_syncobj_joint_dependencymult,
    sim_syncobj_joint_dependencyoff,
    sim_syncobj_joint_stepsize,
    sim_syncobj_joint_ikweight,
    sim_syncobj_joint_position,
    sim_syncobj_joint_mode,
    sim_syncobj_joint_sphericaltransf,
    sim_syncobj_joint_enablemotor,
    sim_syncobj_joint_hybridfunc,
    sim_syncobj_joint_targetvelocity,
    sim_syncobj_joint_uppervelocitylimit,
    sim_syncobj_joint_targetposition,
    sim_syncobj_joint_pid,
    sim_syncobj_joint_kc,
    sim_syncobj_joint_force,
    sim_syncobj_joint_controlenabled,
    sim_syncobj_joint_torquemodulation,
    sim_syncobj_joint_motorlock,
    sim_syncobj_joint_secondpartvalid,
    sim_syncobj_joint_secondparttransf,
    sim_syncobj_joint_bulletfloats,
    sim_syncobj_joint_bulletints,
    sim_syncobj_joint_odefloats,
    sim_syncobj_joint_odeints,
    sim_syncobj_joint_vortexfloats,
    sim_syncobj_joint_vortexints,
    sim_syncobj_joint_newtonfloats,
    sim_syncobj_joint_newtonints,
};

class CJoint;

class _CJoint_ : public CSceneObject
{
public:
    _CJoint_();
    virtual ~_CJoint_();

    // Overridden from _CSceneObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    float getLength() const;
    float getDiameter() const;
    float getPosition() const;
    bool getEnableDynamicMotor() const;
    float getDynamicMotorTargetVelocity() const;
    bool getDynamicMotorLockModeWhenInVelocityControl() const;
    float getDynamicMotorUpperLimitVelocity() const;
    float getDynamicMotorMaximumForce() const;
    bool getEnableDynamicMotorControlLoop() const;
    bool getEnableTorqueModulation() const;
    bool getHybridFunctionality() const;
    float getEngineFloatParam(int what,bool* ok) const;
    int getEngineIntParam(int what,bool* ok) const;
    bool getEngineBoolParam(int what,bool* ok) const;
    void getBulletFloatParams(std::vector<float>& p) const;
    void getBulletIntParams(std::vector<int>& p) const;
    void getOdeFloatParams(std::vector<float>& p) const;
    void getOdeIntParams(std::vector<int>& p) const;
    void getVortexFloatParams(std::vector<float>& p) const;
    void getVortexIntParams(std::vector<int>& p) const;
    void getNewtonFloatParams(std::vector<float>& p) const;
    void getNewtonIntParams(std::vector<int>& p) const;
    int getVortexDependentJointId() const;
    int getNewtonDependentJointId() const;
    float getScrewPitch() const;
    int getJointType() const;
    C4Vector getSphericalTransformation() const;
    bool getPositionIsCyclic() const;
    float getPositionIntervalMin() const;
    float getPositionIntervalRange() const;
    float getIKWeight() const;
    float getMaxStepSize() const;
    int getJointMode() const;
    int getDependencyMasterJointHandle() const;
    float getDependencyJointMult() const;
    float getDependencyJointOffset() const;
    void getDynamicMotorPositionControlParameters(float& p_param,float& i_param,float& d_param) const;
    void getDynamicMotorSpringControlParameters(float& k_param,float& c_param) const;
    float getDynamicMotorPositionControlTargetPosition() const;

    C7Vector getIntrinsicTransformation(bool includeDynErrorComponent) const;

    // Overridden from _CSceneObject_:
    virtual C7Vector getFullLocalTransformation() const;

    virtual bool setPositionIntervalMin(float min);
    virtual bool setPositionIntervalRange(float range);
    virtual bool setPositionIsCyclic(bool isCyclic);
    virtual bool setDiameter(float d);
    virtual bool setLength(float l);
    virtual bool setScrewPitch(float pitch);
    virtual bool setDependencyMasterJointHandle(int depJointID);
    virtual bool setDependencyJointMult(float coeff);
    virtual bool setDependencyJointOffset(float off);
    virtual bool setIkWeight(float newWeight);
    virtual bool setMaxStepSize(float stepS);
    virtual bool setPosition(float pos);
    virtual bool setSphericalTransformation(const C4Vector& tr);
    virtual bool setJointMode(int theMode);
    virtual bool setEnableDynamicMotor(bool e);
    virtual bool setHybridFunctionality(bool h);
    virtual bool setDynamicMotorTargetVelocity(float v);
    virtual bool setDynamicMotorUpperLimitVelocity(float v);
    virtual bool setDynamicMotorPositionControlTargetPosition(float p);
    virtual bool setDynamicMotorPositionControlParameters(float p_param,float i_param,float d_param);
    virtual bool setDynamicMotorSpringControlParameters(float k_param,float c_param);
    virtual bool setDynamicMotorMaximumForce(float f);
    virtual bool setEnableDynamicMotorControlLoop(bool p);
    virtual bool setEnableTorqueModulation(bool p);
    virtual bool setDynamicMotorLockModeWhenInVelocityControl(bool e);
    virtual bool setBulletFloatParams(const std::vector<float>& p);
    virtual bool setBulletIntParams(const std::vector<int>& p);
    virtual bool setOdeFloatParams(const std::vector<float>& p);
    virtual bool setOdeIntParams(const std::vector<int>& p);
    virtual bool setVortexFloatParams(const std::vector<float>& p);
    virtual bool setVortexIntParams(const std::vector<int>& p);
    virtual bool setNewtonFloatParams(const std::vector<float>& p);
    virtual bool setNewtonIntParams(const std::vector<int>& p);

    void setIntrinsicTransformationError(const C7Vector& tr);

    CColorObject* getJointColor1();
    CColorObject* getJointColor2();

protected:
    virtual void _setPositionIntervalMin_send(float min) const;
    virtual void _setPositionIntervalRange_send(float range) const;
    virtual void _setPositionIsCyclic_send(bool isCyclic) const;
    virtual void _setScrewPitch_send(float pitch) const;
    virtual void _setDependencyJointHandle_send(int depJointID) const;
    virtual void _setDependencyJointMult_send(float coeff) const;
    virtual void _setDependencyJointOffset_send(float off) const;
    virtual void _setIkWeight_send(float newWeight) const;
    virtual void _setMaxStepSize_send(float stepS) const;
    virtual void _setPosition_send(float pos) const;
    virtual void _setSphericalTransformation_send(const C4Vector& tr) const;
    virtual void _setJointMode_send(int theMode) const;
    virtual void _setEnableDynamicMotor_send(bool e) const;
    virtual void _setHybridFunctionality_send(bool h) const;
    virtual void _setDynamicMotorTargetVelocity_send(float v) const;
    virtual void _setDynamicMotorUpperLimitVelocity_send(float v) const;
    virtual void _setDynamicMotorPositionControlTargetPosition_send(float p) const;
    virtual void _setDynamicMotorPositionControlParameters_send(float p_param,float i_param,float d_param) const;
    virtual void _setDynamicMotorSpringControlParameters_send(float k_param,float c_param) const;
    virtual void _setDynamicMotorMaximumForce_send(float f) const;
    virtual void _setEnableDynamicMotorControlLoop_send(bool p) const;
    virtual void _setEnableTorqueModulation_send(bool p) const;
    virtual void _setDynamicMotorLockModeWhenInVelocityControl_send(bool e) const;
    virtual void _setBulletFloatParams_send(const std::vector<float>& p) const;
    virtual void _setBulletIntParams_send(const std::vector<int>& p) const;
    virtual void _setOdeFloatParams_send(const std::vector<float>& p) const;
    virtual void _setOdeIntParams_send(const std::vector<int>& p) const;
    virtual void _setVortexFloatParams_send(const std::vector<float>& p) const;
    virtual void _setVortexIntParams_send(const std::vector<int>& p) const;
    virtual void _setNewtonFloatParams_send(const std::vector<float>& p) const;
    virtual void _setNewtonIntParams_send(const std::vector<int>& p) const;

    int _jointType;
    float _length;
    float _diameter;
    C4Vector _sphericalTransformation;
    bool _positionIsCyclic;
    float _screwPitch;
    float _jointMinPosition;
    float _jointPositionRange;
    float _jointPosition;
    float _maxStepSize;
    float _ikWeight;
    int _jointMode;
    int _dependencyMasterJointHandle;
    float _dependencyJointMult;
    float _dependencyJointOffset;

    CColorObject _colorPart1;
    CColorObject _colorPart2;

    bool _dynamicMotorEnabled;
    float _dynamicMotorTargetVelocity;
    bool _dynamicLockModeWhenInVelocityControl;
    float _dynamicMotorUpperLimitVelocity;
    float _dynamicMotorMaximumForce;
    bool _dynamicMotorControlLoopEnabled;
    bool _dynamicMotorPositionControl_torqueModulation;
    float _dynamicMotorPositionControl_targetPosition;
    float _dynamicMotorPositionControl_P;
    float _dynamicMotorPositionControl_I;
    float _dynamicMotorPositionControl_D;
    float _dynamicMotorSpringControl_K;
    float _dynamicMotorSpringControl_C;
    bool _jointHasHybridFunctionality;

    std::vector<float> _bulletFloatParams;
    std::vector<int> _bulletIntParams;

    std::vector<float> _odeFloatParams;
    std::vector<int> _odeIntParams;

    std::vector<float> _vortexFloatParams;
    std::vector<int> _vortexIntParams;

    std::vector<float> _newtonFloatParams;
    std::vector<int> _newtonIntParams;

    C7Vector _intrinsicTransformationError; // from physics engine
};
