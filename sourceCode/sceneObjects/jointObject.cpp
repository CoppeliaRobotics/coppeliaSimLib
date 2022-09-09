#include "jointObject.h"
#include "tt.h"
#include "simInternal.h"
#include "gV.h"
#include "linMotionRoutines.h"
#include "simStrings.h"
#include "ttUtil.h"
#include "easyLock.h"
#include "app.h"
#include "jointRendering.h"
#include "pluginContainer.h"
#include "simFlavor.h"

CJoint::CJoint()
{
    _commonInit();
    computeBoundingBox();
}

CJoint::CJoint(int jointType)
{
    _commonInit();
    _jointType=jointType;
    if (jointType==sim_joint_revolute_subtype)
    {
        _objectName_old=IDSOGL_REVOLUTE_JOINT;
        _objectAlias=IDSOGL_REVOLUTE_JOINT;
        _jointMode=sim_jointmode_dynamic;
        _isCyclic=true;
        _posRange=piValTimes2_f;
        _posMin=-piValue_f;
        _maxStepSize_old=10.0f*degToRad_f;
        _targetForce=2.5f; // 0.25 m x 1kg x 9.81
        _maxAcceleration_DEPRECATED=60.0f*degToRad_f;
        _maxVelAccelJerk[0]=piValTimes2;
        _maxVelAccelJerk[1]=piValTimes2;
        _maxVelAccelJerk[2]=piValTimes2;
    }
    if (jointType==sim_joint_prismatic_subtype)
    {
        _objectName_old=IDSOGL_PRISMATIC_JOINT;
        _objectAlias=IDSOGL_PRISMATIC_JOINT;
        _jointMode=sim_jointmode_dynamic;
        _isCyclic=false;
        _posRange=1.0f;
        _posMin=-0.5f;
        _maxStepSize_old=0.1f;
        _targetForce=50.0f; // 5kg x 9.81
        _maxAcceleration_DEPRECATED=0.1f;
        _maxVelAccelJerk[0]=1.0f;
        _maxVelAccelJerk[1]=1.0f;
        _maxVelAccelJerk[2]=1.0f;
    }
    if (jointType==sim_joint_spherical_subtype)
    {
        _objectName_old=IDSOGL_SPHERICAL_JOINT;
        _objectAlias=IDSOGL_SPHERICAL_JOINT;
        _jointMode=sim_jointmode_dynamic;
        _isCyclic=true;
        _posRange=piValue_f;
        _posMin=0.0f;
        _maxStepSize_old=10.0f*degToRad_f;
        _targetForce=0.0f;
        _maxAcceleration_DEPRECATED=60.0f*degToRad_f;
        _maxVelAccelJerk[0]=piValTimes2;
        _maxVelAccelJerk[1]=piValTimes2;
        _maxVelAccelJerk[2]=piValTimes2;
    }
    _objectAltName_old=tt::getObjectAltNameFromObjectName(_objectName_old.c_str());
    computeBoundingBox();
}

void CJoint::_commonInit()
{
    _objectType=sim_object_joint_type;
    _localObjectSpecialProperty=0;

    _maxVelAccelJerk[0]=piValTimes2;
    _maxVelAccelJerk[1]=piValTimes2;
    _maxVelAccelJerk[2]=piValTimes2;

    _jointType=sim_joint_revolute_subtype;
    _screwPitch=0.0f;
    _sphericalTransf.setIdentity();
    _pos=0.0f;
    _targetPos=0.0f;
    _targetVel=0.0f;

    _jointMode=sim_jointmode_kinematic;
    _dependencyMasterJointHandle=-1;
    _dependencyJointMult=1.0f;
    _dependencyJointOffset=0.0f;

    _intrinsicTransformationError.setIdentity();

    _dynCtrlMode=sim_jointdynctrl_free;
    _dynPositionCtrlType=0; // engine velocity mode + Pos PID, by default
    _dynVelocityCtrlType=0; // engine velocity mode, by default
    _motorLock=false;
    _targetForce=1000.0f; // This value has to be adjusted according to the joint type
    _dynCtrl_kc[0]=0.1f;
    _dynCtrl_kc[1]=0.0f;

    _jointHasHybridFunctionality=false;

    // Bullet parameters
    // ----------------------------------------------------
    _bulletFloatParams.push_back(0.2f); // simi_bullet_joint_stoperp
    _bulletFloatParams.push_back(0.0f); // simi_bullet_joint_stopcfm
    _bulletFloatParams.push_back(0.0f); // simi_bullet_joint_normalcfm
    _bulletFloatParams.push_back(0.0f); // free but 0.0 by default
    _bulletFloatParams.push_back(0.0f); // free but 0.0 by default
    _bulletFloatParams.push_back(0.1f); // simi_bullet_joint_pospid1
    _bulletFloatParams.push_back(0.0f); // simi_bullet_joint_pospid2
    _bulletFloatParams.push_back(0.0f); // simi_bullet_joint_pospid3

    _bulletIntParams.push_back(0); // Free
    // ----------------------------------------------------

    // ODE parameters
    // ----------------------------------------------------
    _odeFloatParams.push_back(0.6f); // simi_ode_joint_stoperp
    _odeFloatParams.push_back(0.00001f); // simi_ode_joint_stopcfm
    _odeFloatParams.push_back(0.0f); // simi_ode_joint_bounce
    _odeFloatParams.push_back(1.0f); // simi_ode_joint_fudgefactor
    _odeFloatParams.push_back(0.00001f); // simi_ode_joint_normalcfm
    _odeFloatParams.push_back(0.1f); // simi_ode_joint_pospid1
    _odeFloatParams.push_back(0.0f); // simi_ode_joint_pospid2
    _odeFloatParams.push_back(0.0f); // simi_ode_joint_pospid3

    _odeIntParams.push_back(0); // Free
    // ----------------------------------------------------

    // Vortex parameters:
    // ----------------------------------------------------
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_lowerlimitdamping
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_upperlimitdamping
    _vortexFloatParams.push_back(FLT_MAX); // simi_vortex_joint_lowerlimitstiffness
    _vortexFloatParams.push_back(FLT_MAX); // simi_vortex_joint_upperlimitstiffness
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_lowerlimitrestitution
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_upperlimitrestitution
    _vortexFloatParams.push_back(FLT_MAX); // simi_vortex_joint_lowerlimitmaxforce
    _vortexFloatParams.push_back(FLT_MAX); // simi_vortex_joint_upperlimitmaxforce
    _vortexFloatParams.push_back(0.001f); // simi_vortex_joint_motorconstraintfrictioncoeff
    _vortexFloatParams.push_back(10.0f); // simi_vortex_joint_motorconstraintfrictionmaxforce
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_motorconstraintfrictionloss
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p0loss
    _vortexFloatParams.push_back(FLT_MAX); // simi_vortex_joint_p0stiffness
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p0damping
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p0frictioncoeff
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p0frictionmaxforce
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p0frictionloss
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p1loss
    _vortexFloatParams.push_back(FLT_MAX); // simi_vortex_joint_p1stiffness
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p1damping
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p1frictioncoeff
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p1frictionmaxforce
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p1frictionloss
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p2loss
    _vortexFloatParams.push_back(FLT_MAX); // simi_vortex_joint_p2stiffness
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p2damping
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p2frictioncoeff
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p2frictionmaxforce
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p2frictionloss
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a0loss
    _vortexFloatParams.push_back(FLT_MAX); // simi_vortex_joint_a0stiffness
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a0damping
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a0frictioncoeff
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a0frictionmaxforce
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a0frictionloss
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a1loss
    _vortexFloatParams.push_back(FLT_MAX); // simi_vortex_joint_a1stiffness
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a1damping
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a1frictioncoeff
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a1frictionmaxforce
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a1frictionloss
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a2loss
    _vortexFloatParams.push_back(FLT_MAX); // simi_vortex_joint_a2stiffness
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a2damping
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a2frictioncoeff
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a2frictionmaxforce
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a2frictionloss
    _vortexFloatParams.push_back(1.0f); // simi_vortex_joint_dependencyfactor
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_dependencyoffset
    _vortexFloatParams.push_back(0.0f); // free but 0.0 by default
    _vortexFloatParams.push_back(0.0f); // free but 0.0 by default
    _vortexFloatParams.push_back(0.1f); // simi_vortex_joint_pospid1
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_pospid2
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_pospid3

    _vortexIntParams.push_back(simi_vortex_joint_proportionalmotorfriction); // simi_vortex_joint_bitcoded
    _vortexIntParams.push_back(0); // simi_vortex_joint_relaxationenabledbc. 1 bit per dof
    _vortexIntParams.push_back(0); // simi_vortex_joint_frictionenabledbc. 1 bit per dof
    _vortexIntParams.push_back(1+2+4+8+16+32); // simi_vortex_joint_frictionproportionalbc. 1 bit per dof
    _vortexIntParams.push_back(-1); // simi_vortex_joint_objectid
    _vortexIntParams.push_back(-1); // simi_vortex_joint_dependentobjectid
    _vortexIntParams.push_back(0); // reserved for future ext.
    // ----------------------------------------------------

    // Newton parameters
    // ----------------------------------------------------
    _newtonFloatParams.push_back(1.0f); // simi_newton_joint_dependencyfactor
    _newtonFloatParams.push_back(0.0f); // simi_newton_joint_dependencyoffset
    _newtonFloatParams.push_back(0.1f); // simi_newton_joint_pospid1
    _newtonFloatParams.push_back(0.0f); // simi_newton_joint_pospid2
    _newtonFloatParams.push_back(0.0f); // simi_newton_joint_pospid3

    _newtonIntParams.push_back(-1); // simi_newton_joint_objectid. The ID is redefined in each session
    _newtonIntParams.push_back(-1); // simi_newton_joint_dependentobjectid
    // ----------------------------------------------------

    // Mujoco parameters
    // ----------------------------------------------------
    _mujocoFloatParams.push_back(0.02f); //sim_mujoco_joint_solreflimit1
    _mujocoFloatParams.push_back(1.0); //sim_mujoco_joint_solreflimit2
    _mujocoFloatParams.push_back(0.9f); //sim_mujoco_joint_solimplimit1
    _mujocoFloatParams.push_back(0.95f); //sim_mujoco_joint_solimplimit2
    _mujocoFloatParams.push_back(0.001f); //sim_mujoco_joint_solimplimit3
    _mujocoFloatParams.push_back(0.5f); //sim_mujoco_joint_solimplimit4
    _mujocoFloatParams.push_back(2.0f); //sim_mujoco_joint_solimplimit5
    _mujocoFloatParams.push_back(0.0); // sim_mujoco_joint_frictionloss
    _mujocoFloatParams.push_back(0.02f); //sim_mujoco_joint_solreffriction1
    _mujocoFloatParams.push_back(1.0); //sim_mujoco_joint_solreffriction2
    _mujocoFloatParams.push_back(0.9f); //sim_mujoco_joint_solimpfriction1
    _mujocoFloatParams.push_back(0.95f); //sim_mujoco_joint_solimpfriction2
    _mujocoFloatParams.push_back(0.001f); //sim_mujoco_joint_solimpfriction3
    _mujocoFloatParams.push_back(0.5f); //sim_mujoco_joint_solimpfriction4
    _mujocoFloatParams.push_back(2.0f); //sim_mujoco_joint_solimpfriction5
    _mujocoFloatParams.push_back(0.0); //sim_mujoco_joint_stiffness
    _mujocoFloatParams.push_back(0.0); //sim_mujoco_joint_damping
    _mujocoFloatParams.push_back(0.0); //sim_mujoco_joint_springref
    _mujocoFloatParams.push_back(0.0); //sim_mujoco_joint_springdamper1
    _mujocoFloatParams.push_back(0.0); //sim_mujoco_joint_springdamper2
    _mujocoFloatParams.push_back(0.02f); //sim_mujoco_joint_armature
    _mujocoFloatParams.push_back(0.0); //sim_mujoco_joint_margin
    _mujocoFloatParams.push_back(0.0); //sim_mujoco_joint_polycoef1
    _mujocoFloatParams.push_back(1.0); //sim_mujoco_joint_polycoef2
    _mujocoFloatParams.push_back(0.0); //sim_mujoco_joint_polycoef3
    _mujocoFloatParams.push_back(0.0); //sim_mujoco_joint_polycoef4
    _mujocoFloatParams.push_back(0.0); //sim_mujoco_joint_polycoef5
    _mujocoFloatParams.push_back(0.1f); // simi_mujoco_joint_pospid1
    _mujocoFloatParams.push_back(0.0f); // simi_mujoco_joint_pospid2
    _mujocoFloatParams.push_back(0.0f); // simi_mujoco_joint_pospid3

    _mujocoIntParams.push_back(-1); // sim_mujoco_joint_objectid. The ID is redefined in each session
    _mujocoIntParams.push_back(-1); // sim_mujoco_joint_dependentobjectid
    // ----------------------------------------------------

    _ikWeight_old=1.0f;
    _diameter=0.02f;
    _length=0.15f;

    _isCyclic=true;
    _posRange=piValTimes2_f;
    _posMin=-piValue_f;
    _maxStepSize_old=10.0f*degToRad_f;

    _visibilityLayer=JOINT_LAYER;
    _objectAlias=IDSOGL_JOINT;
    _objectName_old=IDSOGL_JOINT;
    _objectAltName_old=tt::getObjectAltNameFromObjectName(_objectName_old.c_str());

    _cumulatedForceOrTorque=0.0f;
    _cumulativeForceOrTorqueTmp=0.0f;
    _lastForceOrTorque_dynStep=0.0f;
    _lastForceOrTorqueValid_dynStep=false;
    _averageForceOrTorqueValid=false;
    _kinematicMotionType=0;
    _kinematicMotionInitVel=0.0f;
    _velCalc_vel=0.0f;
    _velCalc_prevPosValid=false;
    _jointPositionForMotionHandling_DEPRECATED=_pos;
    _velocity_DEPRECATED=0.0f;
    _jointCallbackCallOrder_backwardCompatibility=0;
    _explicitHandling_DEPRECATED=false;
    _unlimitedAcceleration_DEPRECATED=false;
    _invertTargetVelocityAtLimits_DEPRECATED=true;
    _maxAcceleration_DEPRECATED=60.0f*degToRad_f;

    _color.setDefaultValues();
    _color.setColor(1.0f,0.3f,0.1f,sim_colorcomponent_ambient_diffuse);
    _color_removeSoon.setDefaultValues();
    _color_removeSoon.setColor(0.22f,0.22f,0.22f,sim_colorcomponent_ambient_diffuse);
}

CJoint::~CJoint()
{

}

void CJoint::setHybridFunctionality_old(bool h)
{
    if ( (_jointType!=sim_joint_spherical_subtype)&&(_jointMode!=sim_jointmode_dynamic) )
    {
        bool diff=(_jointHasHybridFunctionality!=h);
        if (diff)
        {
            _jointHasHybridFunctionality=h;
            if (h)
            {
                setDynCtrlMode(sim_jointdynctrl_positioncb);
                if (_jointType==sim_joint_revolute_subtype)
                    setScrewPitch(0.0f);
            }
        }
    }
}

void CJoint::getDynamicJointErrors(float& linear,float& angular) const
{
    linear=0.0f;
    angular=0.0f;
    if (_jointType==sim_joint_revolute_subtype)
    {
        linear=_intrinsicTransformationError.X.getLength();
        angular=C3Vector::unitZVector.getAngle(_intrinsicTransformationError.Q.getMatrix().axis[2]);
    }
    if (_jointType==sim_joint_prismatic_subtype)
    {
        linear=_intrinsicTransformationError.X.getLength();
        angular=_intrinsicTransformationError.Q.getAngleBetweenQuaternions(C4Vector::identityRotation);
    }
    if (_jointType==sim_joint_spherical_subtype)
        linear=_intrinsicTransformationError.X.getLength();
}

void CJoint::getDynamicJointErrorsFull(C3Vector& linear,C3Vector& angular) const
{
    linear.clear();
    angular.clear();
    if (_jointType==sim_joint_revolute_subtype)
    {
        linear=_intrinsicTransformationError.X;
        angular=_intrinsicTransformationError.Q.getEulerAngles();
        angular(2)=0.0f;
    }
    if (_jointType==sim_joint_prismatic_subtype)
    {
        linear=_intrinsicTransformationError.X;
        linear(2)=0.0f;
        angular=_intrinsicTransformationError.Q.getEulerAngles();
    }
    if (_jointType==sim_joint_spherical_subtype)
        linear=_intrinsicTransformationError.X;
}

bool CJoint::setEngineFloatParam(int what,float v)
{
    if ((what>sim_bullet_joint_float_start)&&(what<sim_bullet_joint_float_end))
    {
        int w=what-sim_bullet_joint_stoperp+simi_bullet_joint_stoperp;
        std::vector<float> fp;
        getBulletFloatParams(fp);
        fp[w]=v;
        setBulletFloatParams(fp);
        return(true);
    }
    if ((what>sim_ode_joint_float_start)&&(what<sim_ode_joint_float_end))
    {
        int w=what-sim_ode_joint_stoperp+simi_ode_joint_stoperp;
        std::vector<float> fp;
        getOdeFloatParams(fp);
        fp[w]=v;
        setOdeFloatParams(fp);
        return(true);
    }
    if ((what>sim_vortex_joint_float_start)&&(what<sim_vortex_joint_float_end))
    {
        int w=what-sim_vortex_joint_lowerlimitdamping+simi_vortex_joint_lowerlimitdamping;
        std::vector<float> fp;
        getVortexFloatParams(fp);
        fp[w]=v;
        setVortexFloatParams(fp);
        return(true);
    }
    if ((what>sim_newton_joint_float_start)&&(what<sim_newton_joint_float_end))
    {
        int w=what-sim_newton_joint_dependencyfactor+simi_newton_joint_dependencyfactor;
        std::vector<float> fp;
        getNewtonFloatParams(fp);
        fp[w]=v;
        setNewtonFloatParams(fp);
        return(true);
    }
    if ((what>sim_mujoco_joint_float_start)&&(what<sim_mujoco_joint_float_end))
    {
        int w=what-sim_mujoco_joint_solreflimit1+simi_mujoco_joint_solreflimit1;
        std::vector<float> fp;
        getMujocoFloatParams(fp);
        fp[w]=v;
        setMujocoFloatParams(fp);
        return(true);
    }
    return(false);
}

bool CJoint::setEngineIntParam(int what,int v)
{
    if ((what>sim_bullet_joint_int_start)&&(what<sim_bullet_joint_int_end))
    {
        // no int params for now
        // search for bji11032016
        return(false);
    }
    if ((what>sim_ode_joint_int_start)&&(what<sim_ode_joint_int_end))
    {
        // no int params for now
        // search for oji11032016
        return(false);
    }
    if ((what>sim_vortex_joint_int_start)&&(what<sim_vortex_joint_int_end))
    {
        int w=what-sim_vortex_joint_bitcoded+simi_vortex_joint_bitcoded;
        std::vector<int> ip;
        getVortexIntParams(ip);
        ip[w]=v;
        setVortexIntParams(ip);
        return(true);
    }
    if ((what>sim_newton_joint_int_start)&&(what<sim_newton_joint_int_end))
    {
        int w=what-sim_newton_joint_objectid+simi_newton_joint_objectid;
        std::vector<int> ip;
        getNewtonIntParams(ip);
        ip[w]=v;
        setNewtonIntParams(ip);
        return(true);
    }
    if ((what>sim_mujoco_joint_int_start)&&(what<sim_mujoco_joint_int_end))
    {
        int w=what-sim_mujoco_joint_objectid+simi_mujoco_joint_objectid;
        std::vector<int> ip;
        getMujocoIntParams(ip);
        ip[w]=v;
        setMujocoIntParams(ip);
        return(true);
    }
    return(false);
}

bool CJoint::setEngineBoolParam(int what,bool v)
{
    if ((what>sim_bullet_joint_bool_start)&&(what<sim_bullet_joint_bool_end))
    {
        // no bool params for now
        // search for bjb11032016
        return(false);
    }
    if ((what>sim_ode_joint_bool_start)&&(what<sim_ode_joint_bool_end))
    {
        // no bool params for now
        // search for ojb11032016
        return(false);
    }
    if ((what>sim_vortex_joint_bool_start)&&(what<sim_vortex_joint_bool_end))
    {
        int b=1;
        int w=(what-sim_vortex_joint_motorfrictionenabled);
        while (w>0) {b*=2; w--;}
        _vortexIntParams[simi_vortex_joint_bitcoded]|=b;
        if (!v)
            _vortexIntParams[simi_vortex_joint_bitcoded]-=b;
        return(true);
    }
    if ((what>sim_newton_joint_bool_start)&&(what<sim_newton_joint_bool_end))
    {
        // no bool params for now
        // search for njb11032016
        return(false);
    }
    if ((what>sim_mujoco_joint_bool_start)&&(what<sim_mujoco_joint_bool_end))
    {
        // no bool params for now
        return(false);
    }
    return(false);
}

void CJoint::setBulletFloatParams(const std::vector<float>& pp)
{
    std::vector<float> p(pp);
    tt::limitValue(0.0f,1.0f,p[simi_bullet_joint_stoperp]); // stop ERP
    tt::limitValue(0.0f,100.0f,p[simi_bullet_joint_stopcfm]); // stop CFM
    tt::limitValue(0.0f,100.0f,p[simi_bullet_joint_normalcfm]); // normal CFM
    bool diff=(_bulletFloatParams.size()!=p.size());
    if (!diff)
    {
        for (size_t i=0;i<p.size();i++)
        {
            if (_bulletFloatParams[i]!=p[i])
            {
                diff=true;
                break;
            }
        }
    }
    if (diff)
        _bulletFloatParams.assign(p.begin(),p.end());
}

void CJoint::setOdeFloatParams(const std::vector<float>& pp)
{
    std::vector<float> p(pp);
    tt::limitValue(0.0f,1.0f,p[simi_ode_joint_stoperp]); // stop ERP
    tt::limitValue(0.0f,100.0f,p[simi_ode_joint_stopcfm]); // stop CFM
    tt::limitValue(0.0f,100.0f,p[simi_ode_joint_bounce]); // bounce
    tt::limitValue(0.0f,10.0f,p[simi_ode_joint_fudgefactor]); // fudge factor
    tt::limitValue(0.0f,100.0f,p[simi_ode_joint_normalcfm]); // normal CFM
    bool diff=(_odeFloatParams.size()!=p.size());
    if (!diff)
    {
        for (size_t i=0;i<p.size();i++)
        {
            if (_odeFloatParams[i]!=p[i])
            {
                diff=true;
                break;
            }
        }
    }
    if (diff)
        _odeFloatParams.assign(p.begin(),p.end());
}

void CJoint::setVortexFloatParams(const std::vector<float>& pp)
{
    std::vector<float> p(pp);
    // Forbid zero stiffness for relaxation axes:
    if (p[simi_vortex_joint_p0stiffness]==0.0f)
        p[simi_vortex_joint_p0stiffness]=1e-35f;
    if (p[simi_vortex_joint_p1stiffness]==0.0f)
        p[simi_vortex_joint_p1stiffness]=1e-35f;
    if (p[simi_vortex_joint_p2stiffness]==0.0f)
        p[simi_vortex_joint_p2stiffness]=1e-35f;
    if (p[simi_vortex_joint_a0stiffness]==0.0f)
        p[simi_vortex_joint_a0stiffness]=1e-35f;
    if (p[simi_vortex_joint_a1stiffness]==0.0f)
        p[simi_vortex_joint_a1stiffness]=1e-35f;
    if (p[simi_vortex_joint_a2stiffness]==0.0f)
        p[simi_vortex_joint_a2stiffness]=1e-35f;
    // Limit offset and factor for dependency equation:
    if (p[simi_vortex_joint_dependencyfactor]>10000.0f)
        p[simi_vortex_joint_dependencyfactor]=10000.0f;
    if (p[simi_vortex_joint_dependencyfactor]<-10000.0f)
        p[simi_vortex_joint_dependencyfactor]=-10000.0f;
    if (p[simi_vortex_joint_dependencyoffset]>10000.0f)
        p[simi_vortex_joint_dependencyoffset]=10000.0f;
    if (p[simi_vortex_joint_dependencyoffset]<-10000.0f)
        p[simi_vortex_joint_dependencyoffset]=-10000.0f;
    bool diff=(_vortexFloatParams.size()!=p.size());
    if (!diff)
    {
        for (size_t i=0;i<p.size();i++)
        {
            if (_vortexFloatParams[i]!=p[i])
            {
                diff=true;
                break;
            }
        }
    }
    if (diff)
        _vortexFloatParams.assign(p.begin(),p.end());
}

void CJoint::setNewtonFloatParams(const std::vector<float>& pp)
{
    std::vector<float> p(pp);
    // Limit offset and factor for dependency equation:
    if (p[simi_newton_joint_dependencyfactor]>10000.0f)
        p[simi_newton_joint_dependencyfactor]=10000.0f;
    if (p[simi_newton_joint_dependencyfactor]<-10000.0f)
        p[simi_newton_joint_dependencyfactor]=-10000.0f;
    if (p[simi_newton_joint_dependencyoffset]>10000.0f)
        p[simi_newton_joint_dependencyoffset]=10000.0f;
    if (p[simi_newton_joint_dependencyoffset]<-10000.0f)
        p[simi_newton_joint_dependencyoffset]=-10000.0f;
    bool diff=(_newtonFloatParams.size()!=p.size());
    if (!diff)
    {
        for (size_t i=0;i<p.size();i++)
        {
            if (_newtonFloatParams[i]!=p[i])
            {
                diff=true;
                break;
            }
        }
    }
    if (diff)
        _newtonFloatParams.assign(p.begin(),p.end());
}

void CJoint::setMujocoFloatParams(const std::vector<float>& pp)
{
    std::vector<float> p(pp);
    bool diff=(_mujocoFloatParams.size()!=p.size());
    if (!diff)
    {
        for (size_t i=0;i<p.size();i++)
        {
            if (_mujocoFloatParams[i]!=p[i])
            {
                diff=true;
                break;
            }
        }
    }
    if (diff)
        _mujocoFloatParams.assign(p.begin(),p.end());
}

void CJoint::copyEnginePropertiesTo(CJoint* target)
{
    std::vector<float> fp;
    std::vector<int> ip;
    // Bullet:
    getBulletFloatParams(fp);
    target->setBulletFloatParams(fp);
    getBulletIntParams(ip);
    target->setBulletIntParams(ip);

    // ODE:
    getOdeFloatParams(fp);
    target->setOdeFloatParams(fp);
    getOdeIntParams(ip);
    target->setOdeIntParams(ip);

    // Vortex:
    getVortexFloatParams(fp);
    target->setVortexFloatParams(fp);
    getVortexIntParams(ip);
    target->setVortexIntParams(ip);

    // Newton:
    getNewtonFloatParams(fp);
    target->setNewtonFloatParams(fp);
    getNewtonIntParams(ip);
    target->setNewtonIntParams(ip);

    // Mujoco:
    getMujocoFloatParams(fp);
    target->setMujocoFloatParams(fp);
    getMujocoIntParams(ip);
    target->setMujocoIntParams(ip);
}

void CJoint::setTargetVelocity(float v)
{
    if (_jointType!=sim_joint_spherical_subtype)
    {
        bool diff=(_targetVel!=v);
        if (diff)
        {
            _targetVel=v;
            if (_targetVel*_targetForce<0.0f)
                setTargetForce(-_targetForce,true);
        }
    }
}

void CJoint::setTargetForce(float f,bool isSigned)
{
    if (_jointType!=sim_joint_spherical_subtype)
    {
        if (!isSigned)
        {
            if (f<0.0f)
                f=0.0f;
            if (f*_targetVel<0.0f)
                f=-f;
        }
        bool diff=(_targetForce!=f);
        if (diff)
        {
            _targetForce=f;
            if (f*_targetVel<0.0f)
                setTargetVelocity(-_targetVel);
        }
    }
}

void CJoint::setPid_old(float p_param,float i_param,float d_param)
{ // old, back-compatibility function
    setEngineFloatParam(sim_bullet_joint_pospid1,p_param);
    setEngineFloatParam(sim_bullet_joint_pospid2,i_param);
    setEngineFloatParam(sim_bullet_joint_pospid3,d_param);
    setEngineFloatParam(sim_ode_joint_pospid1,p_param);
    setEngineFloatParam(sim_ode_joint_pospid2,i_param);
    setEngineFloatParam(sim_ode_joint_pospid3,d_param);
    setEngineFloatParam(sim_vortex_joint_pospid1,p_param);
    setEngineFloatParam(sim_vortex_joint_pospid2,i_param);
    setEngineFloatParam(sim_vortex_joint_pospid3,d_param);
    setEngineFloatParam(sim_newton_joint_pospid1,p_param);
    setEngineFloatParam(sim_newton_joint_pospid2,i_param);
    setEngineFloatParam(sim_newton_joint_pospid3,d_param);
    // Not for Mujoco!
}

void CJoint::setKc(float k_param,float c_param)
{
    float maxVal=+10000000000.0f;
    if (_jointType==sim_joint_revolute_subtype)
        maxVal=+100000000000.0f;
    k_param=tt::getLimitedFloat(-maxVal,maxVal,k_param);
    c_param=tt::getLimitedFloat(-maxVal,maxVal,c_param);
    bool diff=(_dynCtrl_kc[0]!=k_param)||(_dynCtrl_kc[1]!=c_param);
    if (diff)
    {
        _dynCtrl_kc[0]=k_param;
        _dynCtrl_kc[1]=c_param;
    }
}

void CJoint::setTargetPosition(float pos)
{
    if (_jointType!=sim_joint_spherical_subtype)
    {
        if ( (_jointType==sim_joint_revolute_subtype)&&_isCyclic )
            pos=tt::getNormalizedAngle(pos);
        bool diff=(_targetPos!=pos);
        if (diff)
            _targetPos=pos;
    }
}

void CJoint::setDynamicMotorReflectedPosition_useOnlyFromDynamicPart(float rfp,float simTime)
{
    {   // When the joint is in dynamic mode we disable the joint limits and allow a cyclic behaviour (revolute joints)
        // This is because dynamic joints can over or undershoot limits.
        // So we set the position directly, without checking for limits.
        // Turn count is taken care by the physics plugin.
        setPosition(rfp,true);
    }
    _rectifyDependentJoints();
    measureJointVelocity(simTime);
}

void CJoint::setDependencyMasterJointHandle(int depJointID)
{
    if ( (_jointType!=sim_joint_spherical_subtype)&&(getJointMode()==sim_jointmode_dependent) )
    {
        bool diff=(_dependencyMasterJointHandle!=depJointID);
        if (diff)
        {
            _dependencyMasterJointHandle=depJointID;
            if (getObjectCanSync())
                _setDependencyJointHandle_sendOldIk(depJointID);

            if (depJointID==-1)
                App::currentWorld->sceneObjects->actualizeObjectInformation();
            else
            { // enable it
                // We now check for an illegal loop:
                CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(depJointID);
                CJoint* iterat=it;
                while (iterat->getDependencyMasterJointHandle()!=-1)
                {
                    if (iterat->getJointMode()!=_jointMode)
                        break; // We might have a loop, but it is interupted by another jointMode!! (e.g. IK dependency VS direct dependency)
                    int joint=iterat->getDependencyMasterJointHandle();
                    if (joint==getObjectHandle())
                    { // We have an illegal loop! We disable it:
                        iterat->setDependencyMasterJointHandle(-1);
                        break;
                    }
                    iterat=App::currentWorld->sceneObjects->getJointFromHandle(joint);
                }
                App::currentWorld->sceneObjects->actualizeObjectInformation();
                setPosition(getPosition(),false);
            }
        }
    }
}

void CJoint::_setDependencyJointHandle_sendOldIk(int depJointID) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
    {
        int dep=-1;
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(depJointID);
        if (it!=nullptr)
            dep=it->getIkPluginCounterpartHandle();
        CPluginContainer::ikPlugin_setJointDependency(_ikPluginCounterpartHandle,dep,_dependencyJointOffset,_dependencyJointMult);
    }
}

void CJoint::setDependencyJointMult(float coeff)
{
    if (_jointType!=sim_joint_spherical_subtype)
    {
        coeff=tt::getLimitedFloat(-10000.0f,10000.0f,coeff);
        bool diff=(_dependencyJointMult!=coeff);
        if (diff)
        {
            _dependencyJointMult=coeff;
            if (getObjectCanSync())
                _setDependencyJointMult_sendOldIk(coeff);
            setPosition(getPosition(),false);
        }
    }
}

void CJoint::_setDependencyJointMult_sendOldIk(float coeff) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
    {
        int dep=-1;
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_dependencyMasterJointHandle);
        if (it!=nullptr)
            dep=it->getIkPluginCounterpartHandle();
        CPluginContainer::ikPlugin_setJointDependency(_ikPluginCounterpartHandle,dep,_dependencyJointOffset,coeff);
    }
}

void CJoint::setDependencyJointOffset(float off)
{
    if (_jointType!=sim_joint_spherical_subtype)
    {
        off=tt::getLimitedFloat(-10000.0f,10000.0f,off);
        bool diff=(_dependencyJointOffset!=off);
        if (diff)
        {
            _dependencyJointOffset=off;
            if (getObjectCanSync())
                _setDependencyJointOffset_sendOldIk(off);
            setPosition(getPosition(),false);
        }
    }
}

void CJoint::_setDependencyJointOffset_sendOldIk(float off) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
    {
        int dep=-1;
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_dependencyMasterJointHandle);
        if (it!=nullptr)
            dep=it->getIkPluginCounterpartHandle();
        CPluginContainer::ikPlugin_setJointDependency(_ikPluginCounterpartHandle,dep,_dependencyJointOffset,_dependencyJointMult);
    }
}

void CJoint::measureJointVelocity(float simTime)
{
    if (_jointType!=sim_joint_spherical_subtype)
    {
        float dt=simTime-_velCalc_prevSimTime;
        if (_velCalc_prevPosValid&&(dt>0.00001f))
        {
            if (_isCyclic)
                _velCalc_vel=tt::getAngleMinusAlpha(_pos,_velCalc_prevPos)/dt;
            else
                _velCalc_vel=(_pos-_velCalc_prevPos)/dt;
        }
        _velCalc_prevPos=_pos;
        _velCalc_prevSimTime=simTime;
        _velCalc_prevPosValid=true;
    }
}

void CJoint::setVelocity(float v)
{ // sets the velocity, and overrides next velocity measurement in measureJointVelocity
    _velCalc_vel=v;
    _velCalc_prevPosValid=false; // if false, will use _velCalc_vel as current vel in sim.getJointVelocity
}

void CJoint::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    _velCalc_prevPosValid=false;
    _velCalc_vel=0.0f;
    _dynCtrl_previousVelForce[0]=0.0;
    _dynCtrl_previousVelForce[1]=0.0;

    _dynPosCtrl_currentVelAccel[0]=0.0;
    _dynPosCtrl_currentVelAccel[1]=0.0;
    _dynVelCtrl_currentVelAccel[0]=0.0;
    _dynVelCtrl_currentVelAccel[1]=0.0;

    _initialPosition=_pos;
    _initialSphericalJointTransformation=_sphericalTransf;
    setIntrinsicTransformationError(C7Vector::identityTransformation);
    _initialTargetPosition=_targetPos;
    _initialTargetVelocity=_targetVel;

    _initialJointMode=_jointMode;

    _initialDynCtrlMode=_dynCtrlMode;
    _initialDynVelocityCtrlType=_dynVelocityCtrlType;
    _initialDynPositionCtrlType=_dynPositionCtrlType;
    _initialDynCtrl_lockAtVelZero=_motorLock;
    _initialDynCtrl_force=_targetForce;
    _initialDynCtrl_kc[0]=_dynCtrl_kc[0];
    _initialDynCtrl_kc[1]=_dynCtrl_kc[1];

    _initialHybridOperation=_jointHasHybridFunctionality;

    _averageForceOrTorqueValid=false;
    _kinematicMotionType=0;
    _kinematicMotionInitVel=0.0f;
    _cumulatedForceOrTorque=0.0f;
    _lastForceOrTorqueValid_dynStep=false;
    _lastForceOrTorque_dynStep=0.0f;
    _cumulativeForceOrTorqueTmp=0.0f;

    _jointPositionForMotionHandling_DEPRECATED=_pos;
    _velocity_DEPRECATED=0.0f;
    _initialVelocity_DEPRECATED=_velocity_DEPRECATED;
    _initialExplicitHandling_DEPRECATED=_explicitHandling_DEPRECATED;

    _initialMaxVelAccelJerk[0]=_maxVelAccelJerk[0];
    _initialMaxVelAccelJerk[1]=_maxVelAccelJerk[1];
    _initialMaxVelAccelJerk[2]=_maxVelAccelJerk[2];
}

void CJoint::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void CJoint::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if (App::currentWorld->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
        {
            setPosition(_initialPosition,false);
            setSphericalTransformation(_initialSphericalJointTransformation);
            setTargetPosition(_initialTargetPosition);
            setTargetVelocity(_initialTargetVelocity);

            setJointMode(_initialJointMode);
            setDynCtrlMode(_initialDynCtrlMode);
            setDynVelCtrlType(_initialDynVelocityCtrlType);
            setDynPosCtrlType(_initialDynPositionCtrlType);

            setMotorLock(_initialDynCtrl_lockAtVelZero);
            setTargetForce(_initialDynCtrl_force,true);

            setKc(_initialDynCtrl_kc[0],_initialDynCtrl_kc[1]);

            setHybridFunctionality_old(_initialHybridOperation);

            _explicitHandling_DEPRECATED=_initialExplicitHandling_DEPRECATED;
            _velocity_DEPRECATED=_initialVelocity_DEPRECATED;

            setMaxVelAccelJerk(_initialMaxVelAccelJerk);
        }
    }

    _averageForceOrTorqueValid=false;
    _cumulatedForceOrTorque=0.0f;
    _lastForceOrTorqueValid_dynStep=false;
    _lastForceOrTorque_dynStep=0.0f;
    _cumulativeForceOrTorqueTmp=0.0f;
    setIntrinsicTransformationError(C7Vector::identityTransformation);
    CSceneObject::simulationEnded();
}

// FOLLOWING FUNCTIONS ARE DEPRECATED:
//----------------------------------------
void CJoint::resetJoint_DEPRECATED()
{ // DEPRECATED
    if ( (_jointMode!=sim_jointmode_motion_deprecated)||(!App::currentWorld->mainSettings->jointMotionHandlingEnabled_DEPRECATED) )
        return;
    if (_initialValuesInitialized)
    {
        setPosition(_initialPosition,false);
        setVelocity_DEPRECATED(_initialVelocity_DEPRECATED);
    }
}

void CJoint::handleJoint_DEPRECATED(float deltaTime)
{ // DEPRECATED. handling the motion here. Not elegant at all. In future, try using the Reflexxes RML library!
    if ( (_jointMode!=sim_jointmode_motion_deprecated)||(!App::currentWorld->mainSettings->jointMotionHandlingEnabled_DEPRECATED) )
        return;
    if (_unlimitedAcceleration_DEPRECATED)
    {
        _velocity_DEPRECATED=_targetVel;
        if (_velocity_DEPRECATED!=0.0f)
        {
            float newPos=_jointPositionForMotionHandling_DEPRECATED;
            if (!_isCyclic)
            {
                if (_invertTargetVelocityAtLimits_DEPRECATED)
                {
                    float cycleTime=2.0f*_posRange/_velocity_DEPRECATED;
                    deltaTime=CMath::robustFmod(deltaTime,cycleTime);
                    while (true)
                    {
                        _velocity_DEPRECATED=_targetVel;
                        float absDist=_posMin+_posRange-newPos;
                        if (_velocity_DEPRECATED<0.0f)
                            absDist=newPos-_posMin;
                        if (absDist>fabs(_velocity_DEPRECATED)*deltaTime)
                        {
                            newPos+=_velocity_DEPRECATED*deltaTime;
                            break; // We reached the desired deltaTime
                        }
                        if (_velocity_DEPRECATED<0.0f)
                            newPos-=absDist;
                        else
                            newPos+=absDist;
                        deltaTime-=absDist/fabs(_velocity_DEPRECATED);
                        _targetVel*=-1.0f; // We invert the target velocity
                    }
                }
                else
                {
                    newPos+=_velocity_DEPRECATED*deltaTime;
                    float dv=newPos-(_posMin+_posRange);
                    float dl=_posMin-newPos;
                    if ( (dl>=0.0f)||(dv>=0.0f) )
                        _velocity_DEPRECATED=0.0f;
                }
            }
            else
                newPos+=_velocity_DEPRECATED*deltaTime;
            setPosition(newPos,false);
            _jointPositionForMotionHandling_DEPRECATED=getPosition();
        }
    }
    else
    { // Acceleration is not infinite!
        double newPos=double(_jointPositionForMotionHandling_DEPRECATED);
        float minV=-SIM_MAX_FLOAT;
        float maxV=+SIM_MAX_FLOAT;
        if (!_isCyclic)
        {
            minV=_posMin;
            maxV=_posMin+_posRange;
            // Make sure we are within limits:
            float m=float(CLinMotionRoutines::getMaxVelocityAtPosition(newPos,_maxAcceleration_DEPRECATED,minV,maxV,0.0f,0.0f));
            tt::limitValue(-m,m,_velocity_DEPRECATED);
        }

        double velocityDouble=double(_velocity_DEPRECATED);
        double deltaTimeDouble=double(deltaTime);
        while (CLinMotionRoutines::getNextValues(newPos,velocityDouble,_targetVel,_maxAcceleration_DEPRECATED,minV,maxV,0.0f,0.0f,deltaTimeDouble))
        {
            if (_invertTargetVelocityAtLimits_DEPRECATED)
                _targetVel*=-1.0f;
            else
            {
                deltaTime=0.0f;
                deltaTimeDouble=0.0;
                break;
            }
        }
        _velocity_DEPRECATED=float(velocityDouble);

        setPosition(float(newPos),false);
        _jointPositionForMotionHandling_DEPRECATED=getPosition();
    }
}
void CJoint::setExplicitHandling_DEPRECATED(bool explicitHandl)
{ // DEPRECATED
    if (_jointType==sim_joint_spherical_subtype)
        return;
    _explicitHandling_DEPRECATED=explicitHandl;
}

bool CJoint::getExplicitHandling_DEPRECATED()
{ // DEPRECATED
    return(_explicitHandling_DEPRECATED);
}

void CJoint::setUnlimitedAcceleration_DEPRECATED(bool unlimited)
{ // DEPRECATED
    if (_jointType==sim_joint_spherical_subtype)
        return;
    _unlimitedAcceleration_DEPRECATED=unlimited;
    setVelocity_DEPRECATED(getVelocity_DEPRECATED()); // To make sure velocity is within allowed range
}

bool CJoint::getUnlimitedAcceleration_DEPRECATED()
{ // DEPRECATED
    return(_unlimitedAcceleration_DEPRECATED);
}

void CJoint::setInvertTargetVelocityAtLimits_DEPRECATED(bool invert)
{ // DEPRECATED
    if (_jointType==sim_joint_spherical_subtype)
        return;
    _invertTargetVelocityAtLimits_DEPRECATED=invert;
}

bool CJoint::getInvertTargetVelocityAtLimits_DEPRECATED()
{ // DEPRECATED
    return(_invertTargetVelocityAtLimits_DEPRECATED);
}
void CJoint::setMaxAcceleration_DEPRECATED(float maxAccel)
{ // DEPRECATED
    if (_jointType==sim_joint_spherical_subtype)
        return;
    if (_jointType==sim_joint_prismatic_subtype)
        tt::limitValue(0.0001f,1000.0f,maxAccel);
    else
        tt::limitValue(0.001f*degToRad_f,36000.0f*degToRad_f,maxAccel);
    _maxAcceleration_DEPRECATED=maxAccel;
    setVelocity_DEPRECATED(getVelocity_DEPRECATED()); // To make sure velocity is within allowed range
}

float CJoint::getMaxAcceleration_DEPRECATED()
{ // DEPRECATED
    return(_maxAcceleration_DEPRECATED);
}

void CJoint::setVelocity_DEPRECATED(float vel)
{ // DEPRECATED
    if (_jointType==sim_joint_spherical_subtype)
        return;
    if ( (vel!=0.0f)&&((_jointType==sim_joint_prismatic_subtype)||(!_isCyclic))&&(!_unlimitedAcceleration_DEPRECATED) )
    { // We check which is the max allowed:
        float m=float(CLinMotionRoutines::getMaxVelocityAtPosition(_pos,_maxAcceleration_DEPRECATED,_posMin,_posMin+_posRange,0.0f,0.0f));
        tt::limitValue(-m,m,vel);
    }
    _velocity_DEPRECATED=vel;
}

float CJoint::getVelocity_DEPRECATED()
{ // DEPRECATED
    return(_velocity_DEPRECATED);
}
//------------------------------------------------


std::string CJoint::getObjectTypeInfo() const
{
    return(IDSOGL_JOINT);
}

std::string CJoint::getObjectTypeInfoExtended() const
{
    std::string retVal(IDSOGL_JOINT);
    if (_jointType==sim_joint_revolute_subtype)
    {
        if (fabs(_screwPitch)<0.0000001f)
            retVal+=tt::decorateString(" (",IDSOGL_REVOLUTE,", p=");
        else
            retVal+=tt::decorateString(" (",IDSOGL_SCREW,", p=");
        retVal+=gv::getAngleStr(true,_pos)+")";
    }
    if (_jointType==sim_joint_prismatic_subtype)
    {
        retVal+=tt::decorateString(" (",IDSOGL_PRISMATIC,", p=");
        retVal+=gv::getSizeStr(true,_pos)+")";
    }
    if (_jointType==sim_joint_spherical_subtype)
    {
        retVal+=tt::decorateString(" (",IDSOGL_SPHERICAL,", a=");
        C3Vector euler(getSphericalTransformation().getEulerAngles());
        retVal+=gv::getAngleStr(true,euler(0))+", b="+gv::getAngleStr(true,euler(1))+", g="+gv::getAngleStr(true,euler(2))+")";
    }
    return(retVal);
}

float CJoint::getMeasuredJointVelocity() const
{
    return(_velCalc_vel);
}

std::string CJoint::getDependencyJointLoadAlias() const
{
    return(_dependencyJointLoadAlias);
}

std::string CJoint::getDependencyJointLoadName_old() const
{
    return(_dependencyJointLoadName_old);
}

int CJoint::getJointCallbackCallOrder_backwardCompatibility() const
{
    return(_jointCallbackCallOrder_backwardCompatibility);
}

void CJoint::setDirectDependentJoints(const std::vector<CJoint*>& joints)
{
    _directDependentJoints.assign(joints.begin(),joints.end());
}

void CJoint::computeBoundingBox()
{
    C3Vector minV,maxV;
    if (_jointType!=sim_joint_spherical_subtype)
    {
        maxV(0)=maxV(1)=_diameter/2.0f;
        maxV(2)=_length/2.0f;
        minV(0)=-maxV(0);
        minV(1)=-maxV(1);
        minV(2)=-maxV(2);
    }
    else
    {
        maxV(0)=maxV(1)=maxV(2)=_diameter;
        minV(0)=minV(1)=minV(2)=-maxV(0);
    }
    _setBoundingBox(minV,maxV);
}

bool CJoint::setScrewPitch(float pitch)
{
    bool retVal=false;
    if (_jointType==sim_joint_revolute_subtype)
    {
        if (_jointMode!=sim_jointmode_dynamic)
        { // no pitch when in torque/force mode
            pitch=tt::getLimitedFloat(-10.0f,10.0f,pitch);
            bool diff=(_screwPitch!=pitch);
            if (diff)
            {
                _screwPitch=pitch;
                if (getObjectCanSync())
                    _setScrewPitch_sendOldIk(pitch);
                if (pitch!=0.0f)
                    setHybridFunctionality_old(false);
            }
            retVal=true;
        }
    }
    return(retVal);
}

void CJoint::_setScrewPitch_sendOldIk(float pitch) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setJointScrewPitch(_ikPluginCounterpartHandle,_screwPitch);
}

void CJoint::setPositionMin(float min)
{
    if (_jointType!=sim_joint_spherical_subtype)
    {
        if (_jointType==sim_joint_revolute_subtype)
            min=tt::getLimitedFloat(-100000.0f,100000.0f,min);
        if (_jointType==sim_joint_prismatic_subtype)
            min=tt::getLimitedFloat(-1000.0f,1000.0f,min);
        bool diff=(_posMin!=min);
        if (diff)
        {
            _posMin=min;
            if ( _isInScene&&App::worldContainer->getEventsEnabled() )
            {
                const char* cmd="min";
                auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,false,cmd,true);
                data->appendMapObject_stringFloat(cmd,min);
                App::worldContainer->pushEvent(event);
            }
            if (getObjectCanSync())
                _setPositionIntervalMin_sendOldIk(min);
            setPosition(getPosition(),false);
        }
    }
}

void CJoint::_setPositionIntervalMin_sendOldIk(float min) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setJointInterval(_ikPluginCounterpartHandle,_isCyclic,_posMin,_posRange);
}

void CJoint::setPositionRange(float range)
{
    if (_jointType==sim_joint_revolute_subtype)
        range=tt::getLimitedFloat(0.001f*degToRad_f,10000000.0f*degToRad_f,range);
    if (_jointType==sim_joint_prismatic_subtype)
        range=tt::getLimitedFloat(0.0f,1000.0f,range);
    if (_jointType==sim_joint_spherical_subtype)
    {
        if (_jointMode!=sim_jointmode_dynamic)
            range=tt::getLimitedFloat(0.001f*degToRad_f,10000000.0f*degToRad_f,range);
        else
            range=piValue_f;
    }
    bool diff=(_posRange!=range);
    if (diff)
    {
        _posRange=range;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="range";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,false,cmd,true);
            data->appendMapObject_stringFloat(cmd,range);
            App::worldContainer->pushEvent(event);
        }
        if (getObjectCanSync())
            _setPositionIntervalRange_sendOldIk(range);
        setPosition(getPosition(),false);
        setSphericalTransformation(getSphericalTransformation());
    }
}

void CJoint::_setPositionIntervalRange_sendOldIk(float range) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setJointInterval(_ikPluginCounterpartHandle,_isCyclic,_posMin,_posRange);
}

void CJoint::setLength(float l)
{
    tt::limitValue(0.001f,1000.0f,l);
    bool diff=(_length!=l);
    if (diff)
    {
        _length=l;
        computeBoundingBox();
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="length";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,false,cmd,true);
            data->appendMapObject_stringFloat(cmd,l);
            App::worldContainer->pushEvent(event);
        }
    }
}

void CJoint::setDiameter(float d)
{
    tt::limitValue(0.0001f,100.0f,d);
    bool diff=(_diameter!=d);
    if (diff)
    {
        _diameter=d;
        computeBoundingBox();
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="diameter";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,false,cmd,true);
            data->appendMapObject_stringFloat(cmd,d);
            App::worldContainer->pushEvent(event);
        }
    }
}

void CJoint::scaleObject(float scalingFactor)
{
    setDiameter(_diameter*scalingFactor);
    setLength(_length*scalingFactor);
    setScrewPitch(_screwPitch*scalingFactor);
    if (_jointType==sim_joint_prismatic_subtype)
    {
        setPosition(_pos*scalingFactor,false);
        _jointPositionForMotionHandling_DEPRECATED*=scalingFactor;
        setPositionMin(_posMin*scalingFactor);
        setPositionRange(_posRange*scalingFactor);
        setDependencyJointOffset(_dependencyJointOffset*scalingFactor);
        setMaxStepSize_old(_maxStepSize_old*scalingFactor);
        setTargetPosition(_targetPos*scalingFactor);
        setTargetVelocity(_targetVel*scalingFactor);

        setKc(_dynCtrl_kc[0]*scalingFactor*scalingFactor,_dynCtrl_kc[1]*scalingFactor*scalingFactor);

        if ( (_dynCtrlMode==sim_jointdynctrl_spring)||(_dynCtrlMode==sim_jointdynctrl_springcb) )
            setTargetForce(_targetForce*scalingFactor*scalingFactor,false); //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
        if ( (_dynCtrlMode==sim_jointdynctrl_position)||(_dynCtrlMode==sim_jointdynctrl_positioncb) )
            setTargetForce(_targetForce*scalingFactor*scalingFactor*scalingFactor,false); //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

        _maxAcceleration_DEPRECATED*=scalingFactor;
        _velocity_DEPRECATED*=scalingFactor;

        _maxVelAccelJerk[0]*=scalingFactor;
        _maxVelAccelJerk[1]*=scalingFactor;
        _maxVelAccelJerk[2]*=scalingFactor;
        setMaxVelAccelJerk(_maxVelAccelJerk);

        if (_initialValuesInitialized)
        {
            _initialPosition*=scalingFactor;
            _initialTargetPosition*=scalingFactor;
            _initialTargetVelocity*=scalingFactor;

            if ( (_dynCtrlMode==sim_jointdynctrl_spring)||(_dynCtrlMode==sim_jointdynctrl_springcb) )
                _initialDynCtrl_force*=scalingFactor*scalingFactor;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
            if ( (_dynCtrlMode==sim_jointdynctrl_position)||(_dynCtrlMode==sim_jointdynctrl_positioncb) )
                _initialDynCtrl_force*=scalingFactor*scalingFactor*scalingFactor;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

            _initialDynCtrl_kc[0]*=scalingFactor*scalingFactor;
            _initialDynCtrl_kc[1]*=scalingFactor*scalingFactor;

            _initialVelocity_DEPRECATED*=scalingFactor;

            _initialMaxVelAccelJerk[0]*=scalingFactor;
            _initialMaxVelAccelJerk[1]*=scalingFactor;
            _initialMaxVelAccelJerk[2]*=scalingFactor;
        }
    }

    if (_jointType==sim_joint_revolute_subtype)
    {
        setTargetForce(_targetForce*scalingFactor*scalingFactor*scalingFactor*scalingFactor,false);//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

        setKc(_dynCtrl_kc[0]*scalingFactor*scalingFactor*scalingFactor*scalingFactor,_dynCtrl_kc[1]*scalingFactor*scalingFactor*scalingFactor*scalingFactor);

        if (_initialValuesInitialized)
        {
            _initialDynCtrl_force*=scalingFactor*scalingFactor*scalingFactor*scalingFactor;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
            _initialDynCtrl_kc[0]*=scalingFactor*scalingFactor*scalingFactor*scalingFactor;
            _initialDynCtrl_kc[1]*=scalingFactor*scalingFactor*scalingFactor*scalingFactor;
        }
    }

    CSceneObject::scaleObject(scalingFactor);
    _dynamicsResetFlag=true;

    _lastForceOrTorqueValid_dynStep=false;
    _lastForceOrTorque_dynStep=0.0f;
    _averageForceOrTorqueValid=false;
    _cumulatedForceOrTorque=0.0f;
}

void CJoint::scaleObjectNonIsometrically(float x,float y,float z)
{
    float diam=sqrt(x*y);
    setDiameter(_diameter*diam);
    setLength(_length*z);
    setScrewPitch(_screwPitch*z);
    if (_jointType==sim_joint_prismatic_subtype)
    {
        setPosition(_pos*z,false);
        setPositionMin(_posMin*z);
        setPositionRange(_posRange*z);
        setDependencyJointOffset(_dependencyJointOffset*z);
        setMaxStepSize_old(_maxStepSize_old*z);
        setTargetPosition(_targetPos*z);
        setTargetVelocity(_targetVel*z);

        setKc(_dynCtrl_kc[0]*diam*diam,_dynCtrl_kc[1]*diam*diam);

        if ( (_dynCtrlMode==sim_jointdynctrl_spring)||(_dynCtrlMode==sim_jointdynctrl_springcb) )
            setTargetForce(_targetForce*diam*diam,false); //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
        if ( (_dynCtrlMode==sim_jointdynctrl_position)||(_dynCtrlMode==sim_jointdynctrl_positioncb) )
            setTargetForce(_targetForce*diam*diam*diam,false); //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

        _jointPositionForMotionHandling_DEPRECATED*=z;
        _maxAcceleration_DEPRECATED*=z;
        _velocity_DEPRECATED*=z;

        _maxVelAccelJerk[0]*=z;
        _maxVelAccelJerk[1]*=z;
        _maxVelAccelJerk[2]*=z;
        setMaxVelAccelJerk(_maxVelAccelJerk);

        if (_initialValuesInitialized)
        {
            _initialPosition*=z;
            _initialTargetPosition*=z;
            _initialTargetVelocity*=z;

            if ( (_dynCtrlMode==sim_jointdynctrl_spring)||(_dynCtrlMode==sim_jointdynctrl_springcb) )
                _initialDynCtrl_force*=diam*diam;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
            if ( (_dynCtrlMode==sim_jointdynctrl_position)||(_dynCtrlMode==sim_jointdynctrl_positioncb) )
                _initialDynCtrl_force*=diam*diam*diam;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

            _initialDynCtrl_kc[0]*=diam*diam;
            _initialDynCtrl_kc[1]*=diam*diam;

            _initialVelocity_DEPRECATED*=z;

            _initialMaxVelAccelJerk[0]*=z;
            _initialMaxVelAccelJerk[1]*=z;
            _initialMaxVelAccelJerk[2]*=z;
        }
    }

    if (_jointType==sim_joint_revolute_subtype)
    {
        setTargetForce(_targetForce*diam*diam*diam*diam,false);//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

        setKc(_dynCtrl_kc[0]*diam*diam*diam*diam,_dynCtrl_kc[1]*diam*diam*diam*diam);

        if (_initialValuesInitialized)
        {
            _initialDynCtrl_force*=diam*diam*diam*diam;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
            _initialDynCtrl_kc[0]*=diam*diam*diam*diam;
            _initialDynCtrl_kc[1]*=diam*diam*diam*diam;
        }
    }

    CSceneObject::scaleObjectNonIsometrically(diam,diam,z);
    _dynamicsResetFlag=true;
}

void CJoint::addCumulativeForceOrTorque(float forceOrTorque,int countForAverage)
{ // The countForAverage mechanism is needed because we need to average all values in a simulation time step (but this is called every dynamic simulation time step!!)
    _lastForceOrTorque_dynStep=forceOrTorque;
    _lastForceOrTorqueValid_dynStep=true;
    _cumulativeForceOrTorqueTmp+=forceOrTorque;
    if (countForAverage>0)
    {
        _cumulatedForceOrTorque=_cumulativeForceOrTorqueTmp/float(countForAverage);
        _averageForceOrTorqueValid=true;
        _cumulativeForceOrTorqueTmp=0.0f;
    }
}

void CJoint::setForceOrTorqueNotValid()
{
    _averageForceOrTorqueValid=false;
    _cumulatedForceOrTorque=0.0f;
    _lastForceOrTorqueValid_dynStep=false;
    _lastForceOrTorque_dynStep=0.0f;
}

bool CJoint::getDynamicForceOrTorque(float& forceOrTorque,bool dynamicStepValue) const
{
    if (dynamicStepValue)
    {
        if (App::currentWorld->dynamicsContainer->getCurrentlyInDynamicsCalculations())
        {
            if (!_lastForceOrTorqueValid_dynStep)
                return(false);
            forceOrTorque=_lastForceOrTorque_dynStep;
            return(true);
        }
        return(false);
    }
    else
    {
        if (!_averageForceOrTorqueValid) //(!_dynamicSecondPartIsValid)
            return(false);
        forceOrTorque=_cumulatedForceOrTorque;
        return(true);
    }
}

int CJoint::handleDynJoint(int flags,const int intVals[3],float currentPosVelAccel[3],float effort,float dynStepSize,float errorV,float velAndForce[2])
{ // constant callback for every dynamically enabled joint, except for spherical joints. retVal: bit0 set: motor on, bit1 set: motor locked
    // Called before a dyn step. After the step, setDynamicMotorReflectedPosition_useOnlyFromDynamicPart is called
    // flags: bit0: init (first time called), bit1: currentPosVelAccel[1] is valid, bit2: currentPosVelAccel[2] is valid
    int loopCnt=intVals[0];
    int totalLoops=intVals[1];
    int rk4=intVals[2];
    int retVal=1;
    if (_dynCtrlMode==sim_jointdynctrl_free)
        retVal=0;
    else if (_dynCtrlMode==sim_jointdynctrl_force)
    {
        velAndForce[0]=10000.0f;
        velAndForce[1]=_targetForce;
        if (_targetForce<0.0f)
            velAndForce[0]=-10000.0f; // make sure they have same sign
    }
    else if (_dynCtrlMode==sim_jointdynctrl_velocity)
    {
        if (_dynVelocityCtrlType==0)
        { // engine internal velocity ctrl
            velAndForce[0]=_targetVel;
            velAndForce[1]=_targetForce;
            if ( _motorLock&&(_targetVel==0.0f) )
                retVal|=2;
        }
        if (_dynVelocityCtrlType==1)
        { // motion profile
            if (dynStepSize!=0.0)
            {
                double dynVelCtrlCurrentVelAccel[2]={_dynVelCtrl_currentVelAccel[0],_dynVelCtrl_currentVelAccel[1]};
                if ( (_targetVel!=0.0f)||(fabs(dynVelCtrlCurrentVelAccel[0])>0.00001)||(fabs(dynVelCtrlCurrentVelAccel[1])>0.00001) )
                {
                    if ( ( fabs(dynVelCtrlCurrentVelAccel[0]-double(_targetVel))>0.00001 )||(fabs(dynVelCtrlCurrentVelAccel[1])>0.00001) )
                    { // target velocity has not been reached yet
                        double maxVelAccelJerk[3]={double(_maxVelAccelJerk[0]),double(_maxVelAccelJerk[1]),double(_maxVelAccelJerk[2])};
                        double targetVel=double(_targetVel);
                        double pos=0.0;
                        unsigned char sel=1;
                        int ruckObj=CPluginContainer::ruckigPlugin_vel(-1,1,double(dynStepSize),-1,&pos,dynVelCtrlCurrentVelAccel,dynVelCtrlCurrentVelAccel+1,maxVelAccelJerk+1,maxVelAccelJerk+2,&sel,&targetVel);
                        if (ruckObj>=0)
                        {
                            int res=CPluginContainer::ruckigPlugin_step(ruckObj,double(dynStepSize),&pos,dynVelCtrlCurrentVelAccel,dynVelCtrlCurrentVelAccel+1,&pos);
                            CPluginContainer::ruckigPlugin_remove(ruckObj);
                            velAndForce[0]=float(dynVelCtrlCurrentVelAccel[0]);
                            velAndForce[1]=_targetForce;
                            if (velAndForce[0]*velAndForce[1]<0.0f)
                                velAndForce[1]=-velAndForce[1]; // make sure they have same sign
                        }
                    }
                }
                else
                {
                    dynVelCtrlCurrentVelAccel[0]=0.0;
                    dynVelCtrlCurrentVelAccel[1]=0.0;
                    if (_motorLock)
                        retVal|=2;
                }

                velAndForce[0]=float(dynVelCtrlCurrentVelAccel[0]);
                velAndForce[1]=_targetForce;
                if (velAndForce[0]*velAndForce[1]<0.0f)
                    velAndForce[1]=-velAndForce[1]; // make sure they have same sign
                if ( (rk4==0)||(rk4==4) )
                {
                    _dynVelCtrl_currentVelAccel[0]=dynVelCtrlCurrentVelAccel[0];
                    _dynVelCtrl_currentVelAccel[1]=dynVelCtrlCurrentVelAccel[1];
                }
                _dynCtrl_previousVelForce[0]=velAndForce[0];
                _dynCtrl_previousVelForce[1]=velAndForce[1];
            }
            else
            { // in case of RK4, pass1 (dt=0)
                velAndForce[0]=_dynCtrl_previousVelForce[0];
                velAndForce[1]=_dynCtrl_previousVelForce[1];
            }
        }
    }
    else
    { // position, spring and callback
        bool rev=(_jointType==sim_joint_revolute_subtype);
        bool cycl=_isCyclic;
        float lowL=_posMin;
        float highL=_posMin+_posRange;
        bool tryScript=((_dynCtrlMode==sim_jointdynctrl_callback)||(_dynCtrlMode==sim_jointdynctrl_positioncb)||(_dynCtrlMode==sim_jointdynctrl_springcb));
        bool handleHere=( (_dynCtrlMode==sim_jointdynctrl_position)||(_dynCtrlMode==sim_jointdynctrl_spring) );
        if (tryScript)
        {
            CScriptObject* script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,getObjectHandle());
            if ( (script!=nullptr)&&(!script->getScriptIsDisabled()) )

            {
                if (!script->getContainsJointCallbackFunction())
                    script=nullptr;
            }
            CScriptObject* cScript=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,getObjectHandle());
            if ( (cScript!=nullptr)&&(!cScript->getScriptIsDisabled()) )
            {
                if (!cScript->getContainsJointCallbackFunction())
                    cScript=nullptr;
            }
            if ( (script!=nullptr)||(cScript!=nullptr) )
            { // a child or customization scripts want to handle the joint (new calling method)
                // 1. We prepare the in/out stacks:
                CInterfaceStack* inStack=App::worldContainer->interfaceStackContainer->createStack();
                inStack->pushTableOntoStack();

                inStack->pushStringOntoStack("mode",0);
                inStack->pushInt32OntoStack(sim_jointmode_dynamic);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("first",0);
                inStack->pushBoolOntoStack((flags&1)!=0);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("revolute",0);
                inStack->pushBoolOntoStack(rev);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("cyclic",0);
                inStack->pushBoolOntoStack(cycl);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("handle",0);
                inStack->pushInt32OntoStack(getObjectHandle());
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("lowLimit",0);
                inStack->pushFloatOntoStack(lowL);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("highLimit",0);
                inStack->pushFloatOntoStack(highL);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("passCnt",0);
                inStack->pushInt32OntoStack(loopCnt);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("totalPasses",0);
                inStack->pushInt32OntoStack(totalLoops);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("currentPos",0); // deprecated
                inStack->pushFloatOntoStack(currentPosVelAccel[0]);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("pos",0);
                inStack->pushFloatOntoStack(currentPosVelAccel[0]);
                inStack->insertDataIntoStackTable();
                if (rk4>0)
                {
                    inStack->pushStringOntoStack("rk4pass",0);
                    inStack->pushInt32OntoStack(rk4);
                    inStack->insertDataIntoStackTable();
                }
                float cv=_velCalc_vel;
                if ((flags&2)!=0)
                    cv=currentPosVelAccel[1];
                inStack->pushStringOntoStack("currentVel",0); // deprecated
                inStack->pushFloatOntoStack(cv);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("vel",0);
                inStack->pushFloatOntoStack(cv);
                inStack->insertDataIntoStackTable();
                if ((flags&4)!=0)
                {
                    inStack->pushStringOntoStack("accel",0);
                    inStack->pushFloatOntoStack(currentPosVelAccel[2]);
                    inStack->insertDataIntoStackTable();
                }
                inStack->pushStringOntoStack("targetPos",0);
                inStack->pushFloatOntoStack(_targetPos);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("errorValue",0); // deprecated
                inStack->pushFloatOntoStack(errorV);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("error",0);
                inStack->pushFloatOntoStack(errorV);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("effort",0);
                inStack->pushFloatOntoStack(effort);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("dynStepSize",0); // deprecated
                inStack->pushFloatOntoStack(dynStepSize);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("dt",0);
                inStack->pushFloatOntoStack(dynStepSize);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("targetVel",0);
                inStack->pushFloatOntoStack(_targetVel);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("maxForce",0); // deprecated
                inStack->pushFloatOntoStack(fabs(_targetForce));
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("force",0);
                inStack->pushFloatOntoStack(_targetForce);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("velUpperLimit",0); // deprecated
                inStack->pushFloatOntoStack(_maxVelAccelJerk[0]);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("maxVel",0);
                inStack->pushFloatOntoStack(_maxVelAccelJerk[0]);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("maxAccel",0);
                inStack->pushFloatOntoStack(_maxVelAccelJerk[1]);
                inStack->insertDataIntoStackTable();
                inStack->pushStringOntoStack("maxJerk",0);
                inStack->pushFloatOntoStack(_maxVelAccelJerk[2]);
                inStack->insertDataIntoStackTable();
                CInterfaceStack* outStack=App::worldContainer->interfaceStackContainer->createStack();

                // 2. Call the script(s):
                if (script!=nullptr)
                    script->systemCallScript(sim_syscb_jointcallback,inStack,outStack);
                if ( (cScript!=nullptr)&&(outStack->getStackSize()==0) )
                    cScript->systemCallScript(sim_syscb_jointcallback,inStack,outStack);
                // 3. Collect the return values:
                if (outStack->getStackSize()>0)
                {
                    int s=outStack->getStackSize();
                    if (s>1)
                        outStack->moveStackItemToTop(0);
                    outStack->getStackMapFloatValue("force",velAndForce[1]);
                    outStack->getStackMapFloatValue("velocity",velAndForce[0]); // deprecated
                    outStack->getStackMapFloatValue("vel",velAndForce[0]);
                    if (velAndForce[0]*velAndForce[1]<0.0f)
                        velAndForce[1]=-velAndForce[1]; // make sure they have same sign
                }
                App::worldContainer->interfaceStackContainer->destroyStack(outStack);
                App::worldContainer->interfaceStackContainer->destroyStack(inStack);
            }
            else
                handleHere=(_dynCtrlMode!=sim_jointdynctrl_callback);
        }
        if (handleHere)
        { // we have the built-in control (position or spring-damper KC)
            if (dynStepSize!=0.0)
            {
                if ( (_dynPositionCtrlType==1)&&(_dynCtrlMode==sim_jointdynctrl_position) )
                { // motion profile
                    double dynPosCtrlCurrentVelAccel[2]={_dynPosCtrl_currentVelAccel[0],_dynPosCtrl_currentVelAccel[1]};
                    double cp=double(currentPosVelAccel[0]);
                    double tp=cp+double(errorV);
                    double maxVelAccelJerk[3]={double(_maxVelAccelJerk[0]),double(_maxVelAccelJerk[1]),double(_maxVelAccelJerk[2])};
                    unsigned char sel=1;
                    double dummy=0.0;
                    int ruckObj=CPluginContainer::ruckigPlugin_pos(-1,1,dynStepSize,-1,&cp,dynPosCtrlCurrentVelAccel,dynPosCtrlCurrentVelAccel+1,maxVelAccelJerk,maxVelAccelJerk+1,maxVelAccelJerk+2,&sel,&tp,&dummy);
                    if (ruckObj>=0)
                    {
                        int res=CPluginContainer::ruckigPlugin_step(ruckObj,dynStepSize,&dummy,dynPosCtrlCurrentVelAccel,dynPosCtrlCurrentVelAccel+1,&dummy);
                        CPluginContainer::ruckigPlugin_remove(ruckObj);
                        velAndForce[0]=float(dynPosCtrlCurrentVelAccel[0]);
                        velAndForce[1]=_targetForce;
                        if (velAndForce[0]*velAndForce[1]<0.0f)
                            velAndForce[1]=-velAndForce[1]; // make sure they have same sign
                    }
                    if ( (rk4==0)||(rk4==4) )
                    {
                        _dynPosCtrl_currentVelAccel[0]=dynPosCtrlCurrentVelAccel[0];
                        _dynPosCtrl_currentVelAccel[1]=dynPosCtrlCurrentVelAccel[1];
                    }
                }
                else
                { // pos ctrl or spring
                    float P,I,D;
                    getPid(P,I,D);
                    if ( (_dynCtrlMode==sim_jointdynctrl_spring)||(_dynCtrlMode==sim_jointdynctrl_springcb) )
                    {
                        P=_dynCtrl_kc[0];
                        I=0.0f;
                        D=_dynCtrl_kc[1];
                    }

                    if ((flags&1)!=0)
                        _dynCtrl_pid_cumulErr=0.0f;

                    float e=errorV;

                    // Proportional part:
                    float ctrl=e*P;

                    // Integral part:
                    if ( (I!=0.0)&&(rk4==0) ) // so that if we turn the integral part on, we don't try to catch up all the past errors!
                        _dynCtrl_pid_cumulErr+=e*dynStepSize;
                    else
                        _dynCtrl_pid_cumulErr=0.0f;
                    ctrl+=_dynCtrl_pid_cumulErr*I;

                    // Derivative part:
                    float cv=_velCalc_vel;
                    if ((flags&2)!=0)
                        cv=currentPosVelAccel[1];
                    ctrl-=cv*D;

                    if ( (_dynCtrlMode==sim_jointdynctrl_spring)||(_dynCtrlMode==sim_jointdynctrl_springcb) )
                    { // "spring" mode, i.e. force modulation mode
                        velAndForce[0]=fabs(_targetVel);
                        if (ctrl<0.0f)
                            velAndForce[0]=-velAndForce[0];
                        velAndForce[1]=fabs(ctrl);
                        if (velAndForce[0]*velAndForce[1]<0.0f)
                            velAndForce[1]=-velAndForce[1]; // make sure they have same sign
                    }
                    else
                    { // regular position control
                        float vel=ctrl/0.005f; // was ctrl/dynStepSize, but has to be step size independent
                        float maxVel=_maxVelAccelJerk[0];
                        if (vel>maxVel)
                            vel=maxVel;
                        if (vel<-maxVel)
                            vel=-maxVel;

                        velAndForce[0]=vel;
                        velAndForce[1]=_targetForce;
                        if (velAndForce[0]*velAndForce[1]<0.0f)
                            velAndForce[1]=-velAndForce[1]; // make sure they have same sign
                    }
                }
                _dynCtrl_previousVelForce[0]=velAndForce[0];
                _dynCtrl_previousVelForce[1]=velAndForce[1];
            }
            else
            { // in case of RK4, pass1 (dt=0)
                velAndForce[0]=_dynCtrl_previousVelForce[0];
                velAndForce[1]=_dynCtrl_previousVelForce[1];
            }
        }
    }
    // Joint position and velocity is updated later, via _simSetJointPosition and _simSetJointVelocity from the physics engine
    return(retVal);
}

bool CJoint::handleMotion(int scriptType)
{ // retVal true: a joint callback function is present and returned values
    bool retVal=false;
    if ( (_jointMode==sim_jointmode_kinematic)&&(_jointType!=sim_joint_spherical_subtype)&&((_kinematicMotionType&3)!=0) )
    {
        CScriptObject* script;
        if (scriptType==-1)
            script=App::currentWorld->embeddedScriptContainer->getMainScript();
        else
            script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(scriptType,_objectHandle);
        if ( (script!=nullptr)&&(!script->getScriptIsDisabled())&&script->getContainsJointCallbackFunction() )
        {
            bool rev=(_jointType==sim_joint_revolute_subtype);
            float errorV;
            if (rev&&_isCyclic)
                errorV=tt::getAngleMinusAlpha(_targetPos,_pos);
            else
                errorV=_targetPos-_pos;

            // Prepare the in/out stacks:
            CInterfaceStack* inStack=App::worldContainer->interfaceStackContainer->createStack();
            inStack->pushTableOntoStack();
            inStack->pushStringOntoStack("mode",0);
            inStack->pushInt32OntoStack(sim_jointmode_kinematic);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("revolute",0);
            inStack->pushBoolOntoStack(rev);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("cyclic",0);
            inStack->pushBoolOntoStack(_isCyclic&&rev);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("handle",0);
            inStack->pushInt32OntoStack(_objectHandle);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("lowLimit",0);
            inStack->pushFloatOntoStack(_posMin);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("highLimit",0);
            inStack->pushFloatOntoStack(_posMin+_posRange);
            inStack->insertDataIntoStackTable();

            inStack->pushStringOntoStack("currentPos",0); // deprecated
            inStack->pushFloatOntoStack(_pos);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("pos",0);
            inStack->pushFloatOntoStack(_pos);
            inStack->insertDataIntoStackTable();

            inStack->pushStringOntoStack("currentVel",0); // deprecated
            inStack->pushFloatOntoStack(_velCalc_vel);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("vel",0);
            inStack->pushFloatOntoStack(_velCalc_vel);
            inStack->insertDataIntoStackTable();

            if ((_kinematicMotionType&3)==1)
            {
                inStack->pushStringOntoStack("targetPos",0);
                inStack->pushFloatOntoStack(_targetPos);
                inStack->insertDataIntoStackTable();
            }
            if ((_kinematicMotionType&3)==2)
            {
                inStack->pushStringOntoStack("targetVel",0);
                inStack->pushFloatOntoStack(_targetVel);
                inStack->insertDataIntoStackTable();
            }

            if ((_kinematicMotionType&16)!=0)
            {
                inStack->pushStringOntoStack("initVel",0);
                inStack->pushFloatOntoStack(_kinematicMotionInitVel);
                inStack->insertDataIntoStackTable();
            }

            inStack->pushStringOntoStack("errorValue",0); // deprecated
            inStack->pushFloatOntoStack(errorV);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("error",0);
            inStack->pushFloatOntoStack(errorV);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("maxVel",0);
            inStack->pushFloatOntoStack(_maxVelAccelJerk[0]);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("maxAccel",0);
            inStack->pushFloatOntoStack(_maxVelAccelJerk[1]);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("maxJerk",0);
            inStack->pushFloatOntoStack(_maxVelAccelJerk[2]);
            inStack->insertDataIntoStackTable();

            CInterfaceStack* outStack=App::worldContainer->interfaceStackContainer->createStack();
            script->systemCallScript(sim_syscb_jointcallback,inStack,outStack);

            if (outStack->getStackSize()>0)
            {
                _kinematicMotionType=_kinematicMotionType&3; // remove reset flag
                int s=outStack->getStackSize();
                if (s>1)
                    outStack->moveStackItemToTop(0);
                float pos;
                if ( outStack->getStackMapFloatValue("pos",pos)||outStack->getStackMapFloatValue("position",pos) ) // "position" is deprecated
                    setPosition(pos,false);

                bool immobile=false;
                float cv,ca;
                if ( outStack->getStackMapFloatValue("vel",cv)||outStack->getStackMapFloatValue("velocity",cv) ) // "velocity" is deprecated
                {
                    setVelocity(cv);
                    if ( outStack->getStackMapFloatValue("accel",ca)||outStack->getStackMapFloatValue("acceleration",ca) ) // "acceleration" is deprecated
                        immobile=( (cv==0.0f)&&(ca==0.0f) );
                }
                outStack->getStackMapBoolValue("immobile",immobile);
                if (immobile)
                    _kinematicMotionType=16;
                retVal=true;
            }
            App::worldContainer->interfaceStackContainer->destroyStack(outStack);
            App::worldContainer->interfaceStackContainer->destroyStack(inStack);
        }
    }
    return(retVal);
}

void CJoint::setKinematicMotionType(int t,bool reset,float initVel/*=0.0f*/)
{
    _kinematicMotionType=_kinematicMotionType&16;
    _kinematicMotionType=_kinematicMotionType|t;
    if (reset)
    {
        _kinematicMotionType=_kinematicMotionType|16;
        _kinematicMotionInitVel=initVel;
    }
}

void CJoint::setIsCyclic(bool isCyclic)
{
    if (isCyclic)
    {
        if (getJointType()==sim_joint_revolute_subtype)
        {
            setScrewPitch(0.0f);
            setPositionMin(-piValue_f);
            setPositionRange(piValTimes2_f);
        }
    }
    bool diff=(_isCyclic!=isCyclic);
    if (diff)
    {
        _isCyclic=isCyclic;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="cyclic";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,false,cmd,true);
            data->appendMapObject_stringBool(cmd,isCyclic);
            App::worldContainer->pushEvent(event);
        }
        if (getObjectCanSync())
            _setPositionIsCyclic_sendOldIk(isCyclic);
        setVelocity_DEPRECATED(getVelocity_DEPRECATED()); // To make sure velocity is within allowed range
    }
}

void CJoint::_setPositionIsCyclic_sendOldIk(bool isCyclic) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setJointInterval(_ikPluginCounterpartHandle,_isCyclic,_posMin,_posRange);
}

void CJoint::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
    setDependencyMasterJointHandle(-1);
}

void CJoint::addSpecializedObjectEventData(CInterfaceStackTable* data) const
{
    CInterfaceStackTable* subC=new CInterfaceStackTable();
    data->appendMapObject_stringObject("joint",subC);
    data=subC;

    std::string tmp;
    switch(_jointType)
    {
        case sim_joint_revolute_subtype : tmp="revolute";
            break;
        case sim_joint_prismatic_subtype : tmp="prismatic";
            break;
        case sim_joint_spherical_subtype : tmp="spherical";
            break;
    }
    data->appendMapObject_stringString("type",tmp.c_str(),0);
    float q[4]={_sphericalTransf(1),_sphericalTransf(2),_sphericalTransf(3),_sphericalTransf(0)};
    data->appendMapObject_stringFloatArray("quaternion",q,4);
    data->appendMapObject_stringFloat("position",_pos);
    C7Vector tr(getIntrinsicTransformation(true));
    float p[7]={tr.X(0),tr.X(1),tr.X(2),tr.Q(1),tr.Q(2),tr.Q(3),tr.Q(0)};
    data->appendMapObject_stringFloatArray("intrinsicPose",p,7);
    data->appendMapObject_stringFloatArray("maxVelAccelJerk",_maxVelAccelJerk,3);
    data->appendMapObject_stringBool("cyclic",_isCyclic);
    data->appendMapObject_stringFloat("min",_posMin);
    data->appendMapObject_stringFloat("range",_posRange);
    data->appendMapObject_stringFloat("diameter",_diameter);
    data->appendMapObject_stringFloat("length",_length);

    CInterfaceStackTable* colors=new CInterfaceStackTable();
    data->appendMapObject_stringObject("colors",colors);
    float c[9];
    _color.getColor(c,sim_colorcomponent_ambient_diffuse);
    _color.getColor(c+3,sim_colorcomponent_specular);
    _color.getColor(c+6,sim_colorcomponent_emission);
    colors->appendArrayObject_floatArray(c,9);
    _color_removeSoon.getColor(c,sim_colorcomponent_ambient_diffuse);
    _color_removeSoon.getColor(c+3,sim_colorcomponent_specular);
    _color_removeSoon.getColor(c+6,sim_colorcomponent_emission);
    colors->appendArrayObject_floatArray(c,9);
}

CSceneObject* CJoint::copyYourself()
{
    CJoint* newJoint=(CJoint*)CSceneObject::copyYourself();
    newJoint->_dependencyMasterJointHandle=_dependencyMasterJointHandle; // important for copy operations connections
    newJoint->_dependencyJointMult=_dependencyJointMult;
    newJoint->_dependencyJointOffset=_dependencyJointOffset;

    newJoint->_jointType=_jointType;
    newJoint->_jointMode=_jointMode;
    newJoint->_screwPitch=_screwPitch;
    newJoint->_sphericalTransf=_sphericalTransf;
    newJoint->_pos=_pos;
    newJoint->_ikWeight_old=_ikWeight_old;
    newJoint->_diameter=_diameter;
    newJoint->_length=_length;
    newJoint->_isCyclic=_isCyclic;
    newJoint->_posRange=_posRange;
    newJoint->_posMin=_posMin;
    newJoint->_maxStepSize_old=_maxStepSize_old;
    newJoint->_targetPos=_targetPos;
    newJoint->_targetVel=_targetVel;

    _color.copyYourselfInto(&newJoint->_color);

    newJoint->_dynCtrlMode=_dynCtrlMode;
    newJoint->_dynPositionCtrlType=_dynPositionCtrlType;
    newJoint->_dynVelocityCtrlType=_dynVelocityCtrlType;
    newJoint->_motorLock=_motorLock;
    newJoint->_targetForce=_targetForce;

    newJoint->_jointHasHybridFunctionality=_jointHasHybridFunctionality;

    newJoint->_dynCtrl_kc[0]=_dynCtrl_kc[0];
    newJoint->_dynCtrl_kc[1]=_dynCtrl_kc[1];

    newJoint->_bulletFloatParams.assign(_bulletFloatParams.begin(),_bulletFloatParams.end());
    newJoint->_bulletIntParams.assign(_bulletIntParams.begin(),_bulletIntParams.end());

    newJoint->_odeFloatParams.assign(_odeFloatParams.begin(),_odeFloatParams.end());
    newJoint->_odeIntParams.assign(_odeIntParams.begin(),_odeIntParams.end());

    newJoint->_vortexFloatParams.assign(_vortexFloatParams.begin(),_vortexFloatParams.end());
    newJoint->_vortexIntParams.assign(_vortexIntParams.begin(),_vortexIntParams.end());

    newJoint->_newtonFloatParams.assign(_newtonFloatParams.begin(),_newtonFloatParams.end());
    newJoint->_newtonIntParams.assign(_newtonIntParams.begin(),_newtonIntParams.end());

    newJoint->_mujocoFloatParams.assign(_mujocoFloatParams.begin(),_mujocoFloatParams.end());
    newJoint->_mujocoIntParams.assign(_mujocoIntParams.begin(),_mujocoIntParams.end());

    newJoint->_initialValuesInitialized=_initialValuesInitialized;
    newJoint->_initialPosition=_initialPosition;
    newJoint->_initialSphericalJointTransformation=_initialSphericalJointTransformation;
    newJoint->_initialTargetPosition=_initialTargetPosition;
    newJoint->_initialTargetVelocity=_initialTargetVelocity;
    newJoint->_initialDynCtrl_lockAtVelZero=_initialDynCtrl_lockAtVelZero;
    newJoint->_initialDynCtrl_force=_initialDynCtrl_force;
    newJoint->_initialDynCtrl_kc[0]=_initialDynCtrl_kc[0];
    newJoint->_initialDynCtrl_kc[1]=_initialDynCtrl_kc[1];
    newJoint->_initialJointMode=_initialJointMode;
    newJoint->_initialHybridOperation=_initialHybridOperation;

    newJoint->_maxAcceleration_DEPRECATED=_maxAcceleration_DEPRECATED;
    newJoint->_explicitHandling_DEPRECATED=_explicitHandling_DEPRECATED;
    newJoint->_unlimitedAcceleration_DEPRECATED=_unlimitedAcceleration_DEPRECATED;
    newJoint->_invertTargetVelocityAtLimits_DEPRECATED=_invertTargetVelocityAtLimits_DEPRECATED;
    newJoint->_jointPositionForMotionHandling_DEPRECATED=_jointPositionForMotionHandling_DEPRECATED;
    newJoint->_velocity_DEPRECATED=_velocity_DEPRECATED;
    newJoint->_initialExplicitHandling_DEPRECATED=_initialExplicitHandling_DEPRECATED;
    newJoint->_initialVelocity_DEPRECATED=_initialVelocity_DEPRECATED;

    for (size_t i=0;i<3;i++)
    {
        newJoint->_maxVelAccelJerk[i]=_maxVelAccelJerk[i];
        newJoint->_initialMaxVelAccelJerk[i]=_initialMaxVelAccelJerk[i];
    }

    return(newJoint);
}

void CJoint::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID,copyBuffer);
}

void CJoint::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID,copyBuffer);
}

void CJoint::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID,copyBuffer);
}

void CJoint::performIkLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{
    CSceneObject::performIkLoadingMapping(map,loadingAmodel);
}

void CJoint::performCollectionLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performCollectionLoadingMapping(map,loadingAmodel);
}

void CJoint::performCollisionLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performCollisionLoadingMapping(map,loadingAmodel);
}

void CJoint::performDistanceLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performDistanceLoadingMapping(map,loadingAmodel);
}

void CJoint::performTextureObjectLoadingMapping(const std::vector<int>* map)
{
    CSceneObject::performTextureObjectLoadingMapping(map);
}

void CJoint::performDynMaterialObjectLoadingMapping(const std::vector<int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CJoint::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {   // Storing. The order in which we are storing is very important!!!!

            ar.storeDataName("Jtt");
            ar << _jointType;
            ar.flush();

            ar.storeDataName("Jsp");
            ar << _screwPitch;
            ar.flush();

            ar.storeDataName("Jst");
            ar << _sphericalTransf(0) << _sphericalTransf(1);
            ar << _sphericalTransf(2) << _sphericalTransf(3);
            ar.flush();

            ar.storeDataName("Va9");
            unsigned char dummy=0;
            SIM_SET_CLEAR_BIT(dummy,0,_isCyclic);
            SIM_SET_CLEAR_BIT(dummy,1,_explicitHandling_DEPRECATED);
            SIM_SET_CLEAR_BIT(dummy,2,_unlimitedAcceleration_DEPRECATED);
            SIM_SET_CLEAR_BIT(dummy,3,_invertTargetVelocityAtLimits_DEPRECATED);
            SIM_SET_CLEAR_BIT(dummy,4,_dynCtrlMode!=sim_jointdynctrl_free); // for backward comp. with V4.3 and earlier
            SIM_SET_CLEAR_BIT(dummy,5,_dynCtrlMode>=sim_jointdynctrl_position);  // for backward comp. with V4.3 and earlier
            SIM_SET_CLEAR_BIT(dummy,6,_jointHasHybridFunctionality);
            SIM_SET_CLEAR_BIT(dummy,7,(_dynCtrlMode==sim_jointdynctrl_spring)||(_dynCtrlMode==sim_jointdynctrl_springcb));  // for backward comp. with V4.3 and earlier
            ar << dummy;
            ar.flush();

            ar.storeDataName("Vaa");
            dummy=0;
            SIM_SET_CLEAR_BIT(dummy,1,_motorLock);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Cl1");
            ar.setCountingMode();
            _color.serialize(ar,0);
            if (ar.setWritingMode())
                _color.serialize(ar,0);

            ar.storeDataName("Pmr");
            ar << _posMin << _posRange;
            ar.flush();

            ar.storeDataName("Prt");
            ar << _pos;
            ar.flush();

            ar.storeDataName("Mss");
            ar << _maxStepSize_old;
            ar.flush();

            ar.storeDataName("Arg");
            ar << _length << _diameter;
            ar.flush();

            ar.storeDataName("Ikw");
            ar << _ikWeight_old;
            ar.flush();

            ar.storeDataName("Jmd");
            ar << _jointMode;
            ar.flush();

            ar.storeDataName("Jdt");
            ar << _dependencyMasterJointHandle << _dependencyJointMult << _dependencyJointOffset;
            ar.flush();

            ar.storeDataName("Jm3");
            ar << _maxAcceleration_DEPRECATED << _velocity_DEPRECATED;
            ar.flush();

            ar.storeDataName("Dmp");
            ar << _targetVel << _targetForce;
            ar.flush();

            float P,I,D;
            getPid(P,I,D,sim_physics_bullet);
            // Following for backward compatibility (7/5/2014):
            // Keep this before "Dp2"
            ar.storeDataName("Dpc");
            ar << P << (I*0.005f) << (D/0.005f);
            ar.flush();
            // Following for backward compatibility (29/8/2022):
            // Keep this before "Mj3"
            ar.storeDataName("Dp2");
            ar << P << I << D;
            ar.flush();

            ar.storeDataName("Spp");
            ar << _dynCtrl_kc[0] << _dynCtrl_kc[1];
            ar.flush();

            ar.storeDataName("Dtp");
            ar << _targetPos;
            ar.flush();

            ar.storeDataName("Od1"); // keep this for file backw. compat. (09/03/2016)
            ar << _odeFloatParams[simi_ode_joint_stoperp] << _odeFloatParams[simi_ode_joint_stopcfm] << _odeFloatParams[simi_ode_joint_bounce] << _odeFloatParams[simi_ode_joint_fudgefactor] << _odeFloatParams[simi_ode_joint_normalcfm];
            ar.flush();

            ar.storeDataName("Bt1"); // keep this for file backw. compat. (09/03/2016)
            ar << _bulletFloatParams[simi_bullet_joint_stoperp] << _bulletFloatParams[simi_bullet_joint_stopcfm] << _bulletFloatParams[simi_bullet_joint_normalcfm];
            ar.flush();

            ar.storeDataName("BtN"); // Bullet params, keep this after "Bt1"
            ar << int(_bulletFloatParams.size()) << int(_bulletIntParams.size());
            for (size_t i=0;i<_bulletFloatParams.size();i++)
                ar << _bulletFloatParams[i];
            for (size_t i=0;i<_bulletIntParams.size();i++)
                ar << _bulletIntParams[i];
            ar.flush();

            ar.storeDataName("OdN"); // ODE params, keep this after "Od1"
            ar << int(_odeFloatParams.size()) << int(_odeIntParams.size());
            for (size_t i=0;i<_odeFloatParams.size();i++)
                ar << _odeFloatParams[i];
            for (size_t i=0;i<_odeIntParams.size();i++)
                ar << _odeIntParams[i];
            ar.flush();

            ar.storeDataName("Vo2"); // vortex params:
            ar << int(_vortexFloatParams.size()) << int(_vortexIntParams.size());
            for (size_t i=0;i<_vortexFloatParams.size();i++)
                ar << _vortexFloatParams[i];
            for (size_t i=0;i<_vortexIntParams.size();i++)
                ar << _vortexIntParams[i];
            ar.flush();

            ar.storeDataName("Nw1"); // newton params:
            ar << int(_newtonFloatParams.size()) << int(_newtonIntParams.size());
            for (size_t i=0;i<_newtonFloatParams.size();i++)
                ar << _newtonFloatParams[i];
            for (size_t i=0;i<_newtonIntParams.size();i++)
                ar << _newtonIntParams[i];
            ar.flush();

            ar.storeDataName("Mj3"); // mujoco params:
            ar << int(_mujocoFloatParams.size()) << int(_mujocoIntParams.size());
            for (size_t i=0;i<_mujocoFloatParams.size();i++)
                ar << _mujocoFloatParams[i];
            for (size_t i=0;i<_mujocoIntParams.size();i++)
                ar << _mujocoIntParams[i];
            ar.flush();

            ar.storeDataName("Ruc");
            ar << _maxVelAccelJerk[0] << _maxVelAccelJerk[1] << _maxVelAccelJerk[2];
            ar.flush();

            ar.storeDataName("Ulv");
            ar << _maxVelAccelJerk[0]; // for backward compatibility. Keep after "Ruc"
            ar.flush();

            ar.storeDataName("Dcm");
            ar << _dynCtrlMode;
            ar.flush();

            ar.storeDataName("Dpm");
            ar << _dynPositionCtrlType;
            ar.flush();

            ar.storeDataName("Dvm");
            ar << _dynVelocityCtrlType;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            bool kAndCSpringParameterPresent=false; // for backward compatibility (7/5/2014)
            bool usingDynCtrlMode=false;
            bool motorEnabled_old,ctrlEnabled_old,springMode_old;
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Prt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _pos;
                    }
                    if (theName.compare("Jsp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _screwPitch;
                    }
                    if (theName.compare("Jst")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _sphericalTransf(0) >> _sphericalTransf(1);
                        ar >> _sphericalTransf(2) >> _sphericalTransf(3);
                        _sphericalTransf.normalize();
                    }
                    if (theName.compare("Mss")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _maxStepSize_old;
                    }
                    if (theName.compare("Ikw")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _ikWeight_old;
                    }
                    if (theName.compare("Cl1")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; 
                        _color.serialize(ar,0);
                    }
                    if (theName.compare("Arg")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _length >> _diameter;
                    }
                    if (theName.compare("Pmr")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _posMin >> _posRange;
                    }
                    if (theName.compare("Va9")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _isCyclic=SIM_IS_BIT_SET(dummy,0);
                        _explicitHandling_DEPRECATED=SIM_IS_BIT_SET(dummy,1);
                        _unlimitedAcceleration_DEPRECATED=SIM_IS_BIT_SET(dummy,2);
                        _invertTargetVelocityAtLimits_DEPRECATED=SIM_IS_BIT_SET(dummy,3);
                        motorEnabled_old=SIM_IS_BIT_SET(dummy,4);
                        ctrlEnabled_old=SIM_IS_BIT_SET(dummy,5);
                        _jointHasHybridFunctionality=SIM_IS_BIT_SET(dummy,6);
                        springMode_old=SIM_IS_BIT_SET(dummy,7);
                        if ( _jointHasHybridFunctionality&&CSimFlavor::getBoolVal(18) )
                            App::logMsg(sim_verbosity_errors,"Joint has hybrid functionality...");
                    }
                    if (theName.compare("Vaa")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        // _dynamicMotorCustomControl_old=SIM_IS_BIT_SET(dummy,0);
                        _motorLock=SIM_IS_BIT_SET(dummy,1);
                    }
                    if (theName.compare("Jmd")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _jointMode;
                        if (_jointMode==sim_jointmode_reserved_previously_ikdependent)
                            _jointMode=sim_jointmode_dependent; // since 4/7/2014 there is no more an ikdependent mode (ikdependent and dependent are treated as same)
                        if (CSimFlavor::getBoolVal(18))
                        {
                            if ( (_jointMode!=sim_jointmode_kinematic)&&(_jointMode!=sim_jointmode_dependent)&&(_jointMode!=sim_jointmode_dynamic) )
                                App::logMsg(sim_verbosity_errors,"Joint has deprecated mode...");
                        }
                    }
                    if (theName.compare("Jdt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dependencyMasterJointHandle >> _dependencyJointMult >> _dependencyJointOffset;
                    }
                    if (theName.compare("Jtt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _jointType;
                    }
                    if (theName.compare("Jm2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        float v;
                        ar >> _maxAcceleration_DEPRECATED >> _velocity_DEPRECATED >> v;
                        _targetVel=v;
                    }
                    if (theName.compare("Jm3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _maxAcceleration_DEPRECATED >> _velocity_DEPRECATED;
                    }
                    if (theName.compare("Dmp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _targetVel >> _targetForce;
                        if (_targetVel*_targetForce<0.0f)
                            _targetForce=-_targetForce;
                    }
                    if (theName.compare("Dpc")==0)
                    { // keep for backward compatibility (7/5/2014)
                        noHit=false;
                        ar >> byteQuantity;
                        float P,I,D;
                        ar >> P >> I >> D;
                        I/=0.005f;
                        D*=0.005f;
                        setPid_old(P,I,D);
                    }
                    if (theName.compare("Dp2")==0)
                    { // keep for backward compatibility (29/8/2022)
                        noHit=false;
                        ar >> byteQuantity;
                        float P,I,D;
                        ar >> P >> I >> D;
                        setPid_old(P,I,D);
                    }
                    if (theName.compare("Spp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynCtrl_kc[0] >> _dynCtrl_kc[1];
                        kAndCSpringParameterPresent=true;
                    }

                    if (theName.compare("Dtp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _targetPos;
                    }
                    if (theName.compare("Od1")==0)
                    { // keep for backward compat. (09/03/2016)
                        noHit=false;
                        ar >> byteQuantity;
                        // ar >> _odeStopERP >> _odeStopCFM >> _odeBounce >> _odeFudgeFactor >> _odeNormalCFM;
                        ar >> _odeFloatParams[simi_ode_joint_stoperp] >> _odeFloatParams[simi_ode_joint_stopcfm] >> _odeFloatParams[simi_ode_joint_bounce] >> _odeFloatParams[simi_ode_joint_fudgefactor] >> _odeFloatParams[simi_ode_joint_normalcfm];
                    }
                    if (theName.compare("Bt1")==0)
                    { // keep for backward compat. (09/03/2016)
                        noHit=false;
                        ar >> byteQuantity;
                        // ar >> _bulletStopERP >> _bulletStopCFM >> _bulletNormalCFM;
                        ar >> _bulletFloatParams[simi_bullet_joint_stoperp] >> _bulletFloatParams[simi_bullet_joint_stopcfm] >> _bulletFloatParams[simi_bullet_joint_normalcfm];
                    }
                    if (theName.compare("BtN")==0)
                    { // Bullet params:
                        noHit=false;
                        ar >> byteQuantity;
                        int cnt1,cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b=std::min<int>(int(_bulletFloatParams.size()),cnt1);
                        int cnt2_b=std::min<int>(int(_bulletIntParams.size()),cnt2);

                        float vf;
                        int vi;
                        for (int i=0;i<cnt1_b;i++)
                        { // new versions will always have same or more items in _bulletFloatParams already!
                            ar >> vf;
                            _bulletFloatParams[i]=vf;
                        }
                        for (int i=0;i<cnt1-cnt1_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i=0;i<cnt2_b;i++)
                        { // new versions will always have same or more items in _bulletIntParams already!
                            ar >> vi;
                            _bulletIntParams[i]=vi;
                        }
                        for (int i=0;i<cnt2-cnt2_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }
                    if (theName.compare("OdN")==0)
                    { // ODE params:
                        noHit=false;
                        ar >> byteQuantity;
                        int cnt1,cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b=std::min<int>(int(_odeFloatParams.size()),cnt1);
                        int cnt2_b=std::min<int>(int(_odeIntParams.size()),cnt2);

                        float vf;
                        int vi;
                        for (int i=0;i<cnt1_b;i++)
                        { // new versions will always have same or more items in _odeFloatParams already!
                            ar >> vf;
                            _odeFloatParams[i]=vf;
                        }
                        for (int i=0;i<cnt1-cnt1_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i=0;i<cnt2_b;i++)
                        { // new versions will always have same or more items in _odeIntParams already!
                            ar >> vi;
                            _odeIntParams[i]=vi;
                        }
                        for (int i=0;i<cnt2-cnt2_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }
                    if (theName.compare("Vo2")==0)
                    { // vortex params:
                        noHit=false;
                        ar >> byteQuantity;
                        int cnt1,cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b=std::min<int>(int(_vortexFloatParams.size()),cnt1);
                        int cnt2_b=std::min<int>(int(_vortexIntParams.size()),cnt2);

                        float vf;
                        int vi;
                        for (int i=0;i<cnt1_b;i++)
                        { // new versions will always have same or more items in _vortexFloatParams already!
                            ar >> vf;
                            _vortexFloatParams[i]=vf;
                        }
                        for (int i=0;i<cnt1-cnt1_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i=0;i<cnt2_b;i++)
                        { // new versions will always have same or more items in _vortexIntParams already!
                            ar >> vi;
                            _vortexIntParams[i]=vi;
                        }
                        for (int i=0;i<cnt2-cnt2_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }
                    if (theName.compare("Nw1")==0)
                    { // Newton params:
                        noHit=false;
                        ar >> byteQuantity;
                        int cnt1,cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b=std::min<int>(int(_newtonFloatParams.size()),cnt1);
                        int cnt2_b=std::min<int>(int(_newtonIntParams.size()),cnt2);

                        float vf;
                        int vi;
                        for (int i=0;i<cnt1_b;i++)
                        { // new versions will always have same or more items in _newtonFloatParams already!
                            ar >> vf;
                            _newtonFloatParams[i]=vf;
                        }
                        for (int i=0;i<cnt1-cnt1_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i=0;i<cnt2_b;i++)
                        { // new versions will always have same or more items in _newtonIntParams already!
                            ar >> vi;
                            _newtonIntParams[i]=vi;
                        }
                        for (int i=0;i<cnt2-cnt2_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }
                    if (theName.compare("Mj3")==0)
                    { // Mujoco params:
                        noHit=false;
                        ar >> byteQuantity;
                        int cnt1,cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b=std::min<int>(int(_mujocoFloatParams.size()),cnt1);
                        int cnt2_b=std::min<int>(int(_mujocoIntParams.size()),cnt2);

                        float vf;
                        int vi;
                        for (int i=0;i<cnt1_b;i++)
                        { // new versions will always have same or more items in _mujocoFloatParams already!
                            ar >> vf;
                            _mujocoFloatParams[i]=vf;
                        }
                        for (int i=0;i<cnt1-cnt1_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i=0;i<cnt2_b;i++)
                        { // new versions will always have same or more items in _mujocoIntParams already!
                            ar >> vi;
                            _mujocoIntParams[i]=vi;
                        }
                        for (int i=0;i<cnt2-cnt2_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }
                    if (theName.compare("Cco")==0)
                    { // Keep this for backward compatibility (6/8/2014)
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _jointCallbackCallOrder_backwardCompatibility;
                    }
                    if (theName.compare("Ruc")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _maxVelAccelJerk[0] >> _maxVelAccelJerk[1] >> _maxVelAccelJerk[2];
                    }
                    if (theName.compare("Ulv")==0)
                    { // Keep for backward compatibility with V4.3 and earlier
                        noHit=false;
                        ar >> byteQuantity;
                        float val;
                        ar >> val;
                        if ( (_jointMode==sim_jointmode_dynamic)||_jointHasHybridFunctionality )
                            _maxVelAccelJerk[0]=val;
                    }
                    if (theName.compare("Dcm")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynCtrlMode;
                        usingDynCtrlMode=true;
                    }
                    if (theName.compare("Dpm")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynPositionCtrlType;
                    }
                    if (theName.compare("Dvm")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynVelocityCtrlType;
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }

            if (!usingDynCtrlMode)
            { // for backward comp. with V4.3 and earlier
                _dynCtrlMode=sim_jointdynctrl_free;
                if (motorEnabled_old)
                {
                    _dynCtrlMode=sim_jointdynctrl_velocity;
                    if (ctrlEnabled_old)
                    {
                        _dynCtrlMode=sim_jointdynctrl_positioncb;
                        if (springMode_old)
                            _dynCtrlMode=sim_jointdynctrl_springcb;
                    }
                }

                if (!kAndCSpringParameterPresent)
                { // for backward compatibility (7/5/2014):
                    if (motorEnabled_old&&ctrlEnabled_old&&springMode_old)
                    { // we have a joint that behaves as a spring. We need to compute the corresponding K and C parameters, and adjust the max. force/torque (since that was not limited before):
                        float P,I,D;
                        getPid(P,I,D,sim_physics_bullet);
                        _dynCtrl_kc[0]=_targetForce*P;
                        _dynCtrl_kc[1]=_targetForce*D;
                        float maxTolerablePorDParam=1.0f;
                        if (_jointType==sim_joint_revolute_subtype)
                            maxTolerablePorDParam=1.0f/piValTimes2_f;
                        float maxPorD=std::max<float>(fabs(P),fabs(D));
                        if (maxPorD>maxTolerablePorDParam)
                        { // we shift the limit up
                            float corr=maxTolerablePorDParam/maxPorD;
                            P*=corr;
                            I*=corr;
                            D*=corr;
                            setPid_old(P,I,D);
                            _targetForce/=corr;
                        }
                    }
                }
            }

            if (ar.getSerializationVersionThatWroteThisFile()<17)
                CTTUtil::scaleColorUp_(_color.getColorsPtr());
            _fixVortexInfVals();
            computeBoundingBox();
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            float mult=1.0f;
            if (_jointType!=sim_joint_prismatic_subtype)
                mult=180.0f/piValue_f;
            ar.xmlAddNode_comment(" 'type' tag: can be 'revolute', 'prismatic' or 'spherical' ",exhaustiveXml);
            ar.xmlAddNode_enum("type",_jointType,sim_joint_revolute_subtype,"revolute",sim_joint_prismatic_subtype,"prismatic",sim_joint_spherical_subtype,"spherical");
            ar.xmlAddNode_comment(" 'mode' tag: can be 'kinematic', 'dependent' or 'dynamic' ",exhaustiveXml);
            ar.xmlAddNode_enum("mode",_jointMode,sim_jointmode_kinematic,"kinematic",sim_jointmode_ik_deprecated,"ik",sim_jointmode_dependent,"dependent",sim_jointmode_dynamic,"dynamic");

            ar.xmlAddNode_float("minPosition",_posMin*mult);
            ar.xmlAddNode_float("range",_posRange*mult);
            ar.xmlAddNode_float("position",_pos*mult);
            ar.xmlAddNode_float("targetPosition",_targetPos*mult);
            ar.xmlAddNode_float("targetVelocity",_targetVel*mult);
            ar.xmlAddNode_3float("maxVelAccelJerk",_maxVelAccelJerk[0]*mult,_maxVelAccelJerk[1]*mult,_maxVelAccelJerk[2]*mult);
            ar.xmlAddNode_float("screwPitch",_screwPitch);

            if (exhaustiveXml)
                ar.xmlAddNode_floats("sphericalQuaternion",_sphericalTransf.data,4);
            else
            {
                C3Vector euler(_sphericalTransf.getEulerAngles());
                for (size_t l=0;l<3;l++)
                    euler(l)*=180.0f/piValue_f;
                ar.xmlAddNode_floats("sphericalEuler",euler.data,3);
            }

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("cyclic",_isCyclic);
            ar.xmlAddNode_bool("hybridMode",_jointHasHybridFunctionality);
            ar.xmlPopNode();

            ar.xmlPushNewNode("sizes");
            ar.xmlAddNode_float("length",_length);
            ar.xmlAddNode_float("diameter",_diameter);
            ar.xmlPopNode();

            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("objectColor");
                _color.serialize(ar,0);
                ar.xmlPopNode();
            }
            else
            {
                int rgb[3];
                for (size_t l=0;l<3;l++)
                    rgb[l]=int(_color.getColorsPtr()[l]*255.1f);
                ar.xmlAddNode_ints("objectColor",rgb,3);
            }

            ar.xmlPushNewNode("ik");
            ar.xmlAddNode_float("maxStepSize",_maxStepSize_old*mult);
            ar.xmlAddNode_float("weight",_ikWeight_old);
            ar.xmlPopNode();

            ar.xmlPushNewNode("dependency");
            if (exhaustiveXml)
                ar.xmlAddNode_int("jointHandle",_dependencyMasterJointHandle);
            else
            {
                std::string str;
                CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(_dependencyMasterJointHandle);
                if (it!=nullptr)
                    str=it->getObjectName_old();
                ar.xmlAddNode_comment(" 'dependentJoint' tag only used for backward compatibility, use instead 'dependentJointAlias' tag",exhaustiveXml);
                ar.xmlAddNode_string("dependentJoint",str.c_str());
                if (it!=nullptr)
                {
                    str=it->getObjectAlias()+"*";
                    str+=std::to_string(it->getObjectHandle());
                }
                ar.xmlAddNode_string("dependentJointAlias",str.c_str());
            }
            ar.xmlAddNode_comment(" 'offset_m_or_rad' tag: has to be specified in meters or radians ",exhaustiveXml);
            ar.xmlAddNode_float("offset_m_or_rad",_dependencyJointOffset);
            ar.xmlAddNode_comment(" 'mult_m_or_rad' tag: has to be specified in meters or radians ",exhaustiveXml);
            ar.xmlAddNode_float("mult_m_or_rad",_dependencyJointMult);
            ar.xmlPopNode();

            ar.xmlPushNewNode("dynamics");
            ar.xmlAddNode_int("ctrlMode",_dynCtrlMode);
            ar.xmlAddNode_comment(" 'posController' tag: can be 'pid' or 'motionProfile' ",exhaustiveXml);
            ar.xmlAddNode_enum("posController",_dynPositionCtrlType,0,"pid",1,"motionProfile");
            ar.xmlAddNode_comment(" 'velController' tag: can be 'none' or 'motionProfile' ",exhaustiveXml);
            ar.xmlAddNode_enum("velController",_dynVelocityCtrlType,0,"none",1,"motionProfile");
            ar.xmlAddNode_float("maxForce",_targetForce);
            ar.xmlAddNode_comment(" 'upperVelocityLimit' tag only used for backward compatibility",exhaustiveXml);
            ar.xmlAddNode_float("upperVelocityLimit",_maxVelAccelJerk[0]*mult); // for backward compatibility (V4.3 and earlier)
            float P,I,D;
            getPid(P,I,D,sim_physics_bullet);
            ar.xmlAddNode_comment(" 'pidValues' tag only used for backward compatibility",exhaustiveXml);
            ar.xmlAddNode_3float("pidValues",P,I,D); // for backward compatibility (V4.3 and earlier)
            ar.xmlAddNode_2float("kcValues",_dynCtrl_kc[0],_dynCtrl_kc[1]);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("motorEnabled",_dynCtrlMode!=sim_jointdynctrl_free); // for backward compatibility (V4.3 and earlier)
            ar.xmlAddNode_bool("controlLoopEnabled",_dynCtrlMode>=sim_jointdynctrl_position); // for backward compatibility (V4.3 and earlier)
            ar.xmlAddNode_bool("springMode",(_dynCtrlMode==sim_jointdynctrl_spring)||(_dynCtrlMode==sim_jointdynctrl_springcb)); // for backward compatibility (V4.3 and earlier)
            ar.xmlAddNode_bool("lockInVelocityControl",_motorLock);
            ar.xmlPopNode();

            float v[5];
            ar.xmlPushNewNode("engines");
            ar.xmlPushNewNode("bullet");
            ar.xmlAddNode_float("stoperp",getEngineFloatParam(sim_bullet_joint_stoperp,nullptr));
            ar.xmlAddNode_float("stopcfm",getEngineFloatParam(sim_bullet_joint_stopcfm,nullptr));
            ar.xmlAddNode_float("normalcfm",getEngineFloatParam(sim_bullet_joint_normalcfm,nullptr));
            v[0]=getEngineFloatParam(sim_bullet_joint_pospid1,nullptr);
            v[1]=getEngineFloatParam(sim_bullet_joint_pospid2,nullptr);
            v[2]=getEngineFloatParam(sim_bullet_joint_pospid3,nullptr);
            ar.xmlAddNode_floats("posPid",v,3);
            ar.xmlPopNode();

            ar.xmlPushNewNode("ode");
            ar.xmlAddNode_float("stoperp",getEngineFloatParam(sim_ode_joint_stoperp,nullptr));
            ar.xmlAddNode_float("stopcfm",getEngineFloatParam(sim_ode_joint_stopcfm,nullptr));
            ar.xmlAddNode_float("bounce",getEngineFloatParam(sim_ode_joint_bounce,nullptr));
            ar.xmlAddNode_float("fudgefactor",getEngineFloatParam(sim_ode_joint_fudgefactor,nullptr));
            ar.xmlAddNode_float("normalcfm",getEngineFloatParam(sim_ode_joint_normalcfm,nullptr));
            v[0]=getEngineFloatParam(sim_ode_joint_pospid1,nullptr);
            v[1]=getEngineFloatParam(sim_ode_joint_pospid2,nullptr);
            v[2]=getEngineFloatParam(sim_ode_joint_pospid3,nullptr);
            ar.xmlAddNode_floats("posPid",v,3);
            ar.xmlPopNode();

            ar.xmlPushNewNode("vortex");
            ar.xmlAddNode_float("lowerlimitdamping",getEngineFloatParam(sim_vortex_joint_lowerlimitdamping,nullptr));
            ar.xmlAddNode_float("upperlimitdamping",getEngineFloatParam(sim_vortex_joint_upperlimitdamping,nullptr));
            ar.xmlAddNode_float("lowerlimitstiffness",getEngineFloatParam(sim_vortex_joint_lowerlimitstiffness,nullptr));
            ar.xmlAddNode_float("upperlimitstiffness",getEngineFloatParam(sim_vortex_joint_upperlimitstiffness,nullptr));
            ar.xmlAddNode_float("lowerlimitrestitution",getEngineFloatParam(sim_vortex_joint_lowerlimitrestitution,nullptr));
            ar.xmlAddNode_float("upperlimitrestitution",getEngineFloatParam(sim_vortex_joint_upperlimitrestitution,nullptr));
            ar.xmlAddNode_float("lowerlimitmaxforce",getEngineFloatParam(sim_vortex_joint_lowerlimitmaxforce,nullptr));
            ar.xmlAddNode_float("upperlimitmaxforce",getEngineFloatParam(sim_vortex_joint_upperlimitmaxforce,nullptr));
            ar.xmlAddNode_float("motorconstraintfrictioncoeff",getEngineFloatParam(sim_vortex_joint_motorconstraintfrictioncoeff,nullptr));
            ar.xmlAddNode_float("motorconstraintfrictionmaxforce",getEngineFloatParam(sim_vortex_joint_motorconstraintfrictionmaxforce,nullptr));
            ar.xmlAddNode_float("motorconstraintfrictionloss",getEngineFloatParam(sim_vortex_joint_motorconstraintfrictionloss,nullptr));
            ar.xmlAddNode_float("p0loss",getEngineFloatParam(sim_vortex_joint_p0loss,nullptr));
            ar.xmlAddNode_float("p0stiffness",getEngineFloatParam(sim_vortex_joint_p0stiffness,nullptr));
            ar.xmlAddNode_float("p0damping",getEngineFloatParam(sim_vortex_joint_p0damping,nullptr));
            ar.xmlAddNode_float("p0frictioncoeff",getEngineFloatParam(sim_vortex_joint_p0frictioncoeff,nullptr));
            ar.xmlAddNode_float("p0frictionmaxforce",getEngineFloatParam(sim_vortex_joint_p0frictionmaxforce,nullptr));
            ar.xmlAddNode_float("p0frictionloss",getEngineFloatParam(sim_vortex_joint_p0frictionloss,nullptr));
            ar.xmlAddNode_float("p1loss",getEngineFloatParam(sim_vortex_joint_p1loss,nullptr));
            ar.xmlAddNode_float("p1stiffness",getEngineFloatParam(sim_vortex_joint_p1stiffness,nullptr));
            ar.xmlAddNode_float("p1damping",getEngineFloatParam(sim_vortex_joint_p1damping,nullptr));
            ar.xmlAddNode_float("p1frictioncoeff",getEngineFloatParam(sim_vortex_joint_p1frictioncoeff,nullptr));
            ar.xmlAddNode_float("p1frictionmaxforce",getEngineFloatParam(sim_vortex_joint_p1frictionmaxforce,nullptr));
            ar.xmlAddNode_float("p1frictionloss",getEngineFloatParam(sim_vortex_joint_p1frictionloss,nullptr));
            ar.xmlAddNode_float("p2loss",getEngineFloatParam(sim_vortex_joint_p2loss,nullptr));
            ar.xmlAddNode_float("p2stiffness",getEngineFloatParam(sim_vortex_joint_p2stiffness,nullptr));
            ar.xmlAddNode_float("p2damping",getEngineFloatParam(sim_vortex_joint_p2damping,nullptr));
            ar.xmlAddNode_float("p2frictioncoeff",getEngineFloatParam(sim_vortex_joint_p2frictioncoeff,nullptr));
            ar.xmlAddNode_float("p2frictionmaxforce",getEngineFloatParam(sim_vortex_joint_p2frictionmaxforce,nullptr));
            ar.xmlAddNode_float("p2frictionloss",getEngineFloatParam(sim_vortex_joint_p2frictionloss,nullptr));
            ar.xmlAddNode_float("a0loss",getEngineFloatParam(sim_vortex_joint_a0loss,nullptr));
            ar.xmlAddNode_float("a0stiffness",getEngineFloatParam(sim_vortex_joint_a0stiffness,nullptr));
            ar.xmlAddNode_float("a0damping",getEngineFloatParam(sim_vortex_joint_a0damping,nullptr));
            ar.xmlAddNode_float("a0frictioncoeff",getEngineFloatParam(sim_vortex_joint_a0frictioncoeff,nullptr));
            ar.xmlAddNode_float("a0frictionmaxforce",getEngineFloatParam(sim_vortex_joint_a0frictionmaxforce,nullptr));
            ar.xmlAddNode_float("a0frictionloss",getEngineFloatParam(sim_vortex_joint_a0frictionloss,nullptr));
            ar.xmlAddNode_float("a1loss",getEngineFloatParam(sim_vortex_joint_a1loss,nullptr));
            ar.xmlAddNode_float("a1stiffness",getEngineFloatParam(sim_vortex_joint_a1stiffness,nullptr));
            ar.xmlAddNode_float("a1damping",getEngineFloatParam(sim_vortex_joint_a1damping,nullptr));
            ar.xmlAddNode_float("a1frictioncoeff",getEngineFloatParam(sim_vortex_joint_a1frictioncoeff,nullptr));
            ar.xmlAddNode_float("a1frictionmaxforce",getEngineFloatParam(sim_vortex_joint_a1frictionmaxforce,nullptr));
            ar.xmlAddNode_float("a1frictionloss",getEngineFloatParam(sim_vortex_joint_a1frictionloss,nullptr));
            ar.xmlAddNode_float("a2loss",getEngineFloatParam(sim_vortex_joint_a2loss,nullptr));
            ar.xmlAddNode_float("a2stiffness",getEngineFloatParam(sim_vortex_joint_a2stiffness,nullptr));
            ar.xmlAddNode_float("a2damping",getEngineFloatParam(sim_vortex_joint_a2damping,nullptr));
            ar.xmlAddNode_float("a2frictioncoeff",getEngineFloatParam(sim_vortex_joint_a2frictioncoeff,nullptr));
            ar.xmlAddNode_float("a2frictionmaxforce",getEngineFloatParam(sim_vortex_joint_a2frictionmaxforce,nullptr));
            ar.xmlAddNode_float("a2frictionloss",getEngineFloatParam(sim_vortex_joint_a2frictionloss,nullptr));
            ar.xmlAddNode_float("dependencyfactor",getEngineFloatParam(sim_vortex_joint_dependencyfactor,nullptr));
            ar.xmlAddNode_float("dependencyoffset",getEngineFloatParam(sim_vortex_joint_dependencyoffset,nullptr));
            v[0]=getEngineFloatParam(sim_vortex_joint_pospid1,nullptr);
            v[1]=getEngineFloatParam(sim_vortex_joint_pospid2,nullptr);
            v[2]=getEngineFloatParam(sim_vortex_joint_pospid3,nullptr);
            ar.xmlAddNode_floats("posPid",v,3);

            ar.xmlAddNode_int("relaxationenabledbc",getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr));
            ar.xmlAddNode_int("frictionenabledbc",getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr));
            ar.xmlAddNode_int("frictionproportionalbc",getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr));
            ar.xmlAddNode_int("objectid",getEngineIntParam(sim_vortex_joint_objectid,nullptr));
            ar.xmlAddNode_int("dependentobjectid",getEngineIntParam(sim_vortex_joint_dependentobjectid,nullptr));

            ar.xmlAddNode_bool("motorfrictionenabled",getEngineBoolParam(sim_vortex_joint_motorfrictionenabled,nullptr));
            ar.xmlAddNode_bool("proportionalmotorfriction",getEngineBoolParam(sim_vortex_joint_proportionalmotorfriction,nullptr));
            ar.xmlPopNode();

            ar.xmlPushNewNode("newton");
            ar.xmlAddNode_float("dependencyfactor",getEngineFloatParam(sim_newton_joint_dependencyfactor,nullptr));
            ar.xmlAddNode_float("dependencyoffset",getEngineFloatParam(sim_newton_joint_dependencyoffset,nullptr));
            v[0]=getEngineFloatParam(sim_newton_joint_pospid1,nullptr);
            v[1]=getEngineFloatParam(sim_newton_joint_pospid2,nullptr);
            v[2]=getEngineFloatParam(sim_newton_joint_pospid3,nullptr);
            ar.xmlAddNode_floats("posPid",v,3);

            ar.xmlAddNode_int("objectid",getEngineIntParam(sim_newton_joint_objectid,nullptr));
            ar.xmlAddNode_int("dependentobjectid",getEngineIntParam(sim_newton_joint_dependentobjectid,nullptr));
            ar.xmlPopNode();

            ar.xmlPushNewNode("mujoco");
            v[0]=getEngineFloatParam(sim_mujoco_joint_solreflimit1,nullptr);
            v[1]=getEngineFloatParam(sim_mujoco_joint_solreflimit2,nullptr);
            ar.xmlAddNode_floats("solreflimit",v,2);
            v[0]=getEngineFloatParam(sim_mujoco_joint_solimplimit1,nullptr);
            v[1]=getEngineFloatParam(sim_mujoco_joint_solimplimit2,nullptr);
            v[2]=getEngineFloatParam(sim_mujoco_joint_solimplimit3,nullptr);
            v[3]=getEngineFloatParam(sim_mujoco_joint_solimplimit4,nullptr);
            v[4]=getEngineFloatParam(sim_mujoco_joint_solimplimit5,nullptr);
            ar.xmlAddNode_floats("solimplimit",v,5);
            v[0]=getEngineFloatParam(sim_mujoco_joint_solreffriction1,nullptr);
            v[1]=getEngineFloatParam(sim_mujoco_joint_solreffriction2,nullptr);
            ar.xmlAddNode_floats("solreffriction",v,2);
            v[0]=getEngineFloatParam(sim_mujoco_joint_solimpfriction1,nullptr);
            v[1]=getEngineFloatParam(sim_mujoco_joint_solimpfriction2,nullptr);
            v[2]=getEngineFloatParam(sim_mujoco_joint_solimpfriction3,nullptr);
            v[3]=getEngineFloatParam(sim_mujoco_joint_solimpfriction4,nullptr);
            v[4]=getEngineFloatParam(sim_mujoco_joint_solimpfriction5,nullptr);
            ar.xmlAddNode_floats("solimpfriction",v,5);
            ar.xmlAddNode_float("frictionloss",getEngineFloatParam(sim_mujoco_joint_frictionloss,nullptr));
            ar.xmlAddNode_float("stiffness",getEngineFloatParam(sim_mujoco_joint_stiffness,nullptr));
            ar.xmlAddNode_float("damping",getEngineFloatParam(sim_mujoco_joint_damping,nullptr));
            ar.xmlAddNode_float("springref",getEngineFloatParam(sim_mujoco_joint_springref,nullptr));
            v[0]=sim_mujoco_joint_springdamper1;
            v[2]=sim_mujoco_joint_springdamper2;
            ar.xmlAddNode_floats("springdamper",v,2);

            ar.xmlAddNode_float("armature",getEngineFloatParam(sim_mujoco_joint_armature,nullptr));
            ar.xmlAddNode_float("margin",getEngineFloatParam(sim_mujoco_joint_margin,nullptr));
            ar.xmlAddNode_int("dependentobjectid",getEngineIntParam(sim_mujoco_joint_dependentobjectid,nullptr));
            for (size_t j=0;j<5;j++)
                v[j]=getEngineFloatParam(sim_mujoco_joint_polycoef1+j,nullptr);
            ar.xmlAddNode_floats("polycoef",v,5);
            v[0]=getEngineFloatParam(sim_mujoco_joint_pospid1,nullptr);
            v[1]=getEngineFloatParam(sim_mujoco_joint_pospid2,nullptr);
            v[2]=getEngineFloatParam(sim_mujoco_joint_pospid3,nullptr);
            ar.xmlAddNode_floats("posPid",v,3);
            ar.xmlPopNode();

            ar.xmlPopNode();

            ar.xmlPopNode();
        }
        else
        {
            bool usingDynCtrlMode=false;
            bool motorEnabled_old,ctrlEnabled_old,springMode_old;
            float mult=1.0f;
            if (ar.xmlGetNode_enum("type",_jointType,exhaustiveXml,"revolute",sim_joint_revolute_subtype,"prismatic",sim_joint_prismatic_subtype,"spherical",sim_joint_spherical_subtype))
            {
                if (_jointType==sim_joint_revolute_subtype)
                    mult=piValue_f/180.0f;
            }

            ar.xmlGetNode_enum("mode",_jointMode,exhaustiveXml,"kinematic",sim_jointmode_kinematic,"ik",sim_jointmode_ik_deprecated,"dependent",sim_jointmode_dependent,"dynamic",sim_jointmode_dynamic,"passive",sim_jointmode_kinematic,"force",sim_jointmode_dynamic);

            if (ar.xmlGetNode_float("minPosition",_posMin,exhaustiveXml))
            {
                _posMin*=mult;

                if (ar.xmlGetNode_float("range",_posRange,exhaustiveXml))
                    _posRange*=mult;
            }

            if (ar.xmlGetNode_float("position",_pos,exhaustiveXml))
                _pos*=mult;
            if (ar.xmlGetNode_float("targetPosition",_targetPos,exhaustiveXml))
                _targetPos*=mult;
            if (ar.xmlGetNode_float("targetVelocity",_targetVel,exhaustiveXml))
                _targetVel*=mult;
            if (ar.xmlGetNode_floats("maxVelAccelJerk",_maxVelAccelJerk,3,exhaustiveXml))
            {
                _maxVelAccelJerk[0]*=mult;
                _maxVelAccelJerk[1]*=mult;
                _maxVelAccelJerk[2]*=mult;
            }

            ar.xmlGetNode_float("screwPitch",_screwPitch,exhaustiveXml);

            if (exhaustiveXml)
                ar.xmlGetNode_floats("sphericalQuaternion",_sphericalTransf.data,4);
            else
            {
                C3Vector euler;
                if (ar.xmlGetNode_floats("sphericalEuler",euler.data,3,exhaustiveXml))
                {
                    euler(0)*=piValue_f/180.0f;
                    euler(1)*=piValue_f/180.0f;
                    euler(2)*=piValue_f/180.0f;
                    _sphericalTransf.setEulerAngles(euler);
                }
            }

            if (ar.xmlPushChildNode("switches",exhaustiveXml))
            {
                ar.xmlGetNode_bool("cyclic",_isCyclic,exhaustiveXml);
                ar.xmlGetNode_bool("hybridMode",_jointHasHybridFunctionality,exhaustiveXml);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("sizes",exhaustiveXml))
            {
                ar.xmlGetNode_float("length",_length,exhaustiveXml);
                ar.xmlGetNode_float("diameter",_diameter,exhaustiveXml);
                ar.xmlPopNode();
            }

            if (exhaustiveXml)
            {
                if (ar.xmlPushChildNode("objectColor",false))
                {
                    _color.serialize(ar,0);
                    ar.xmlPopNode();
                }
            }
            else
            {
                int rgb[3];
                if (ar.xmlGetNode_ints("objectColor",rgb,3,false))
                    _color.setColor(float(rgb[0])/255.1f,float(rgb[1])/255.1f,float(rgb[2])/255.1f,sim_colorcomponent_ambient_diffuse);
            }

            if (ar.xmlPushChildNode("color",false))
            { // for backward compatibility
                if (exhaustiveXml)
                {
                    if (ar.xmlPushChildNode("part1"))
                    {
                        _color.serialize(ar,0);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("part2"))
                    {
                        _color_removeSoon.serialize(ar,0);
                        ar.xmlPopNode();
                    }
                }
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("part1",rgb,3,exhaustiveXml))
                        _color.setColor(float(rgb[0])/255.0f,float(rgb[1])/255.0f,float(rgb[2])/255.0f,sim_colorcomponent_ambient_diffuse);
                    if (ar.xmlGetNode_ints("part2",rgb,3,exhaustiveXml))
                        _color_removeSoon.setColor(float(rgb[0])/255.0f,float(rgb[1])/255.0f,float(rgb[2])/255.0f,sim_colorcomponent_ambient_diffuse);
                }
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("ik",exhaustiveXml))
            {
                if (ar.xmlGetNode_float("maxStepSize",_maxStepSize_old,exhaustiveXml))
                    _maxStepSize_old*=mult;
                ar.xmlGetNode_float("weight",_ikWeight_old,exhaustiveXml);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("dependency",exhaustiveXml))
            {
                if (exhaustiveXml)
                    ar.xmlGetNode_int("jointHandle",_dependencyMasterJointHandle);
                else
                {
                    ar.xmlGetNode_string("dependentJointAlias",_dependencyJointLoadAlias,exhaustiveXml);
                    ar.xmlGetNode_string("dependentJoint",_dependencyJointLoadName_old,exhaustiveXml);
                }
                ar.xmlGetNode_float("offset_m_or_rad",_dependencyJointOffset,exhaustiveXml);
                ar.xmlGetNode_float("mult_m_or_rad",_dependencyJointMult,exhaustiveXml);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("dynamics",exhaustiveXml))
            {
                if (ar.xmlGetNode_int("ctrlMode",_dynCtrlMode,exhaustiveXml))
                    usingDynCtrlMode=true;

                ar.xmlGetNode_enum("posController",_dynPositionCtrlType,exhaustiveXml,"pid",0,"motionProfile",1);
                ar.xmlGetNode_enum("velController",_dynVelocityCtrlType,exhaustiveXml,"none",0,"motionProfile",1);

                ar.xmlGetNode_float("maxForce",_targetForce,exhaustiveXml);
                if (_targetVel*_targetForce<0.0f)
                    _targetForce=-_targetForce;
                float val;
                if (ar.xmlGetNode_float("upperVelocityLimit",val,false)) // for backward compatibility (V4.3 and earlier)
                {
                    if ( (_jointMode==sim_jointmode_dynamic)||_jointHasHybridFunctionality )
                        _maxVelAccelJerk[0]=val*mult;
                }
                if (ar.xmlGetNode_float("targetPosition",_targetPos,false)) // for backward compatibility (V4.3 and earlier)
                    _targetPos*=mult;
                if (ar.xmlGetNode_float("targetVelocity",_targetVel,false)) // for backward compatibility (V4.3 and earlier)
                    _targetVel*=mult;
                float P,I,D;
                ar.xmlGetNode_3float("pidValues",P,I,D,exhaustiveXml);
                setPid_old(P,I,D);
                ar.xmlGetNode_2float("kcValues",_dynCtrl_kc[0],_dynCtrl_kc[1],exhaustiveXml);

                if (ar.xmlPushChildNode("switches",exhaustiveXml))
                {
                    ar.xmlGetNode_bool("motorEnabled",motorEnabled_old,false); // for backward compatibility (V4.3 and earlier)
                    ar.xmlGetNode_bool("controlLoopEnabled",ctrlEnabled_old,false); // for backward compatibility (V4.3 and earlier)
                    ar.xmlGetNode_bool("springMode",springMode_old,false); // for backward compatibility (V4.3 and earlier)
                    ar.xmlGetNode_bool("lockInVelocityControl",_motorLock,exhaustiveXml);
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("engines",exhaustiveXml))
                {
                    float v;
                    int vi;
                    bool vb;
                    if (ar.xmlPushChildNode("bullet",exhaustiveXml))
                    {
                        float w[3];
                        if (ar.xmlGetNode_float("stoperp",v,exhaustiveXml)) setEngineFloatParam(sim_bullet_joint_stoperp,v);
                        if (ar.xmlGetNode_float("stopcfm",v,exhaustiveXml)) setEngineFloatParam(sim_bullet_joint_stopcfm,v);
                        if (ar.xmlGetNode_float("normalcfm",v,exhaustiveXml)) setEngineFloatParam(sim_bullet_joint_normalcfm,v);
                        if (ar.xmlGetNode_floats("posPid",w,3,exhaustiveXml))
                        {
                            setEngineFloatParam(sim_bullet_joint_pospid1,w[0]);
                            setEngineFloatParam(sim_bullet_joint_pospid2,w[1]);
                            setEngineFloatParam(sim_bullet_joint_pospid3,w[2]);
                        }
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("ode",exhaustiveXml))
                    {
                        float w[3];
                        if (ar.xmlGetNode_float("stoperp",v,exhaustiveXml)) setEngineFloatParam(sim_ode_joint_stoperp,v);
                        if (ar.xmlGetNode_float("stopcfm",v,exhaustiveXml)) setEngineFloatParam(sim_ode_joint_stopcfm,v);
                        if (ar.xmlGetNode_float("bounce",v,exhaustiveXml)) setEngineFloatParam(sim_ode_joint_bounce,v);
                        if (ar.xmlGetNode_float("fudgefactor",v,exhaustiveXml)) setEngineFloatParam(sim_ode_joint_fudgefactor,v);
                        if (ar.xmlGetNode_float("normalcfm",v,exhaustiveXml)) setEngineFloatParam(sim_ode_joint_normalcfm,v);
                        if (ar.xmlGetNode_floats("posPid",w,3,exhaustiveXml))
                        {
                            setEngineFloatParam(sim_ode_joint_pospid1,w[0]);
                            setEngineFloatParam(sim_ode_joint_pospid2,w[1]);
                            setEngineFloatParam(sim_ode_joint_pospid3,w[2]);
                        }
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("vortex",exhaustiveXml))
                    {
                        float w[3];
                        if (ar.xmlGetNode_float("lowerlimitdamping",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_lowerlimitdamping,v);
                        if (ar.xmlGetNode_float("upperlimitdamping",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_upperlimitdamping,v);
                        if (ar.xmlGetNode_float("lowerlimitstiffness",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_lowerlimitstiffness,v);
                        if (ar.xmlGetNode_float("upperlimitstiffness",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_upperlimitstiffness,v);
                        if (ar.xmlGetNode_float("lowerlimitrestitution",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_lowerlimitrestitution,v);
                        if (ar.xmlGetNode_float("upperlimitrestitution",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_upperlimitrestitution,v);
                        if (ar.xmlGetNode_float("lowerlimitmaxforce",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_lowerlimitmaxforce,v);
                        if (ar.xmlGetNode_float("upperlimitmaxforce",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_upperlimitmaxforce,v);
                        if (ar.xmlGetNode_float("motorconstraintfrictioncoeff",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_motorconstraintfrictioncoeff,v);
                        if (ar.xmlGetNode_float("motorconstraintfrictionmaxforce",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_motorconstraintfrictionmaxforce,v);
                        if (ar.xmlGetNode_float("motorconstraintfrictionloss",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_motorconstraintfrictionloss,v);
                        if (ar.xmlGetNode_float("p0loss",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p0loss,v);
                        if (ar.xmlGetNode_float("p0stiffness",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p0stiffness,v);
                        if (ar.xmlGetNode_float("p0damping",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p0damping,v);
                        if (ar.xmlGetNode_float("p0frictioncoeff",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p0frictioncoeff,v);
                        if (ar.xmlGetNode_float("p0frictionmaxforce",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p0frictionmaxforce,v);
                        if (ar.xmlGetNode_float("p0frictionloss",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p0frictionloss,v);
                        if (ar.xmlGetNode_float("p1loss",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p1loss,v);
                        if (ar.xmlGetNode_float("p1stiffness",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p1stiffness,v);
                        if (ar.xmlGetNode_float("p1damping",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p1damping,v);
                        if (ar.xmlGetNode_float("p1frictioncoeff",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p1frictioncoeff,v);
                        if (ar.xmlGetNode_float("p1frictionmaxforce",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p1frictionmaxforce,v);
                        if (ar.xmlGetNode_float("p1frictionloss",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p1frictionloss,v);
                        if (ar.xmlGetNode_float("p2loss",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p2loss,v);
                        if (ar.xmlGetNode_float("p2stiffness",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p2stiffness,v);
                        if (ar.xmlGetNode_float("p2damping",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p2damping,v);
                        if (ar.xmlGetNode_float("p2frictioncoeff",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p2frictioncoeff,v);
                        if (ar.xmlGetNode_float("p2frictionmaxforce",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p2frictionmaxforce,v);
                        if (ar.xmlGetNode_float("p2frictionloss",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_p2frictionloss,v);
                        if (ar.xmlGetNode_float("a0loss",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a0loss,v);
                        if (ar.xmlGetNode_float("a0stiffness",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a0stiffness,v);
                        if (ar.xmlGetNode_float("a0damping",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a0damping,v);
                        if (ar.xmlGetNode_float("a0frictioncoeff",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a0frictioncoeff,v);
                        if (ar.xmlGetNode_float("a0frictionmaxforce",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a0frictionmaxforce,v);
                        if (ar.xmlGetNode_float("a0frictionloss",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a0frictionloss,v);
                        if (ar.xmlGetNode_float("a1loss",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a1loss,v);
                        if (ar.xmlGetNode_float("a1stiffness",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a1stiffness,v);
                        if (ar.xmlGetNode_float("a1damping",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a1damping,v);
                        if (ar.xmlGetNode_float("a1frictioncoeff",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a1frictioncoeff,v);
                        if (ar.xmlGetNode_float("a1frictionmaxforce",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a1frictionmaxforce,v);
                        if (ar.xmlGetNode_float("a1frictionloss",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a1frictionloss,v);
                        if (ar.xmlGetNode_float("a2loss",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a2loss,v);
                        if (ar.xmlGetNode_float("a2stiffness",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a2stiffness,v);
                        if (ar.xmlGetNode_float("a2damping",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a2damping,v);
                        if (ar.xmlGetNode_float("a2frictioncoeff",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a2frictioncoeff,v);
                        if (ar.xmlGetNode_float("a2frictionmaxforce",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a2frictionmaxforce,v);
                        if (ar.xmlGetNode_float("a2frictionloss",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_a2frictionloss,v);
                        if (ar.xmlGetNode_float("dependencyfactor",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_dependencyfactor,v);
                        if (ar.xmlGetNode_float("dependencyoffset",v,exhaustiveXml)) setEngineFloatParam(sim_vortex_joint_dependencyoffset,v);
                        if (ar.xmlGetNode_floats("posPid",w,3,exhaustiveXml))
                        {
                            setEngineFloatParam(sim_vortex_joint_pospid1,w[0]);
                            setEngineFloatParam(sim_vortex_joint_pospid2,w[1]);
                            setEngineFloatParam(sim_vortex_joint_pospid3,w[2]);
                        }

                        //if (ar.xmlGetNode_int("bitcoded")) setEngineIntParam(sim_vortex_joint_bitcoded,vi);
                        if (ar.xmlGetNode_int("relaxationenabledbc",vi,exhaustiveXml)) setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vi);
                        if (ar.xmlGetNode_int("frictionenabledbc",vi,exhaustiveXml)) setEngineIntParam(sim_vortex_joint_frictionenabledbc,vi);
                        if (ar.xmlGetNode_int("frictionproportionalbc",vi,exhaustiveXml)) setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vi);
                        if (ar.xmlGetNode_int("objectid",vi,exhaustiveXml)) setEngineIntParam(sim_vortex_joint_objectid,vi);
                        if (ar.xmlGetNode_int("dependentobjectid",vi,exhaustiveXml)) setEngineIntParam(sim_vortex_joint_dependentobjectid,vi);

                        if (ar.xmlGetNode_bool("motorfrictionenabled",vb,exhaustiveXml)) setEngineBoolParam(sim_vortex_joint_motorfrictionenabled,vb);
                        if (ar.xmlGetNode_bool("proportionalmotorfriction",vb,exhaustiveXml)) setEngineBoolParam(sim_vortex_joint_proportionalmotorfriction,vb);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("newton",exhaustiveXml))
                    {
                        float w[3];
                        if (ar.xmlGetNode_float("dependencyfactor",v,exhaustiveXml)) setEngineFloatParam(sim_newton_joint_dependencyfactor,v);
                        if (ar.xmlGetNode_float("dependencyoffset",v,exhaustiveXml)) setEngineFloatParam(sim_newton_joint_dependencyoffset,v);

                        if (ar.xmlGetNode_int("objectid",vi,exhaustiveXml)) setEngineIntParam(sim_newton_joint_objectid,vi);
                        if (ar.xmlGetNode_int("dependentobjectid",vi,exhaustiveXml)) setEngineIntParam(sim_newton_joint_dependentobjectid,vi);
                        if (ar.xmlGetNode_floats("posPid",w,3,exhaustiveXml))
                        {
                            setEngineFloatParam(sim_newton_joint_pospid1,w[0]);
                            setEngineFloatParam(sim_newton_joint_pospid2,w[1]);
                            setEngineFloatParam(sim_newton_joint_pospid3,w[2]);
                        }
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("mujoco",exhaustiveXml))
                    {
                        float w[5];
                        if (ar.xmlGetNode_floats("solreflimit",w,2,exhaustiveXml))
                        {
                            setEngineFloatParam(sim_mujoco_joint_solreflimit1,w[0]);
                            setEngineFloatParam(sim_mujoco_joint_solreflimit2,w[1]);
                        }
                        if (ar.xmlGetNode_floats("solimplimit",w,5,exhaustiveXml))
                        {
                            for (size_t j=0;j<5;j++)
                                setEngineFloatParam(sim_mujoco_joint_solimplimit1+j,w[j]);
                        }
                        if (ar.xmlGetNode_floats("solreffriction",w,2,exhaustiveXml))
                        {
                            setEngineFloatParam(sim_mujoco_joint_solreffriction1,w[0]);
                            setEngineFloatParam(sim_mujoco_joint_solreffriction2,w[1]);
                        }
                        if (ar.xmlGetNode_floats("solimpfriction",w,5,exhaustiveXml))
                        {
                            for (size_t j=0;j<5;j++)
                                setEngineFloatParam(sim_mujoco_joint_solimpfriction1+j,w[j]);
                        }
                        if (ar.xmlGetNode_float("frictionloss",v,exhaustiveXml)) setEngineFloatParam(sim_mujoco_joint_frictionloss,v);
                        if (ar.xmlGetNode_float("stiffness",v,exhaustiveXml)) setEngineFloatParam(sim_mujoco_joint_stiffness,v);
                        if (ar.xmlGetNode_float("damping",v,exhaustiveXml)) setEngineFloatParam(sim_mujoco_joint_damping,v);
                        if (ar.xmlGetNode_float("springref",v,exhaustiveXml)) setEngineFloatParam(sim_mujoco_joint_springref,v);
                        if (ar.xmlGetNode_floats("springdamper",w,2,exhaustiveXml))
                        {
                            setEngineFloatParam(sim_mujoco_joint_springdamper1,w[0]);
                            setEngineFloatParam(sim_mujoco_joint_springdamper2,w[1]);
                        }
                        if (ar.xmlGetNode_float("armature",v,exhaustiveXml)) setEngineFloatParam(sim_mujoco_joint_armature,v);
                        if (ar.xmlGetNode_float("margin",v,exhaustiveXml)) setEngineFloatParam(sim_mujoco_joint_margin,v);
                        if (ar.xmlGetNode_floats("polycoef",w,5,exhaustiveXml))
                        {
                            for (size_t j=0;j<5;j++)
                                setEngineFloatParam(sim_mujoco_joint_polycoef1+j,w[j]);
                        }
                        if (ar.xmlGetNode_int("dependentobjectid",vi,exhaustiveXml)) setEngineIntParam(sim_mujoco_joint_dependentobjectid,vi);

                        if (ar.xmlGetNode_floats("posPid",w,3,exhaustiveXml))
                        {
                            setEngineFloatParam(sim_mujoco_joint_pospid1,w[0]);
                            setEngineFloatParam(sim_mujoco_joint_pospid2,w[1]);
                            setEngineFloatParam(sim_mujoco_joint_pospid3,w[2]);
                        }


                        ar.xmlPopNode();
                    }
                    ar.xmlPopNode();
                }

                ar.xmlPopNode();
            }
            if (!usingDynCtrlMode)
            {
                _dynCtrlMode=sim_jointdynctrl_free;
                if (motorEnabled_old)
                {
                    _dynCtrlMode=sim_jointdynctrl_velocity;
                    if (ctrlEnabled_old)
                    {
                        _dynCtrlMode=sim_jointdynctrl_positioncb;
                        if (springMode_old)
                            _dynCtrlMode=sim_jointdynctrl_springcb;
                    }
                }
            }
            _fixVortexInfVals();
            computeBoundingBox();
        }
    }
}

void CJoint::performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    CSceneObject::performObjectLoadingMapping(map,loadingAmodel);
    _dependencyMasterJointHandle=CWorld::getLoadingMapping(map,_dependencyMasterJointHandle);
    // following few for dyn joint dep, so that the correct object handle is set
    std::vector<int> ip;
    getVortexIntParams(ip);
    ip[4]=_objectHandle;
    ip[5]=CWorld::getLoadingMapping(map,ip[5]); // Vortex dependency joint
    setVortexIntParams(ip);
    getNewtonIntParams(ip);
    ip[0]=_objectHandle;
    ip[1]=CWorld::getLoadingMapping(map,ip[1]); // Newton dependency joint
    setNewtonIntParams(ip);
    getMujocoIntParams(ip);
    ip[0]=_objectHandle;
    ip[1]=CWorld::getLoadingMapping(map,ip[1]); // Mujoco dependency joint
    setMujocoIntParams(ip);
}

void CJoint::setJointMode(int theMode)
{
    bool retVal=setJointMode_noDynMotorTargetPosCorrection(theMode);
    if (retVal)
        setTargetPosition(getPosition());
}

bool CJoint::setJointMode_noDynMotorTargetPosCorrection(int theMode)
{
    int md;
    if (theMode==sim_jointmode_kinematic)
    {
        App::currentWorld->sceneObjects->actualizeObjectInformation();
        md=theMode;
    }
    if (theMode==sim_jointmode_motion_deprecated)
    {
        if (_jointMode!=theMode)
        {
            _velocity_DEPRECATED=0.0f;
            _targetVel=0.0f;
            _explicitHandling_DEPRECATED=true;
            _unlimitedAcceleration_DEPRECATED=true;
            _invertTargetVelocityAtLimits_DEPRECATED=false;
        }
        if (_jointType!=sim_joint_spherical_subtype)
        {
            App::currentWorld->sceneObjects->actualizeObjectInformation();
            md=theMode;
        }
    }
    if ((theMode==sim_jointmode_dependent)||(theMode==sim_jointmode_reserved_previously_ikdependent))
    {
        if (_jointType!=sim_joint_spherical_subtype)
        {
            App::currentWorld->sceneObjects->actualizeObjectInformation();
            md=theMode;
        }
    }
    if (theMode==sim_jointmode_dynamic)
    {
        if ( (_jointMode!=theMode)&&( (_dynCtrlMode==sim_jointdynctrl_spring)||(_dynCtrlMode==sim_jointdynctrl_springcb)||(_dynCtrlMode==sim_jointdynctrl_force) ) )
            setTargetVelocity(1000.0f); // just a very high value
        setHybridFunctionality_old(false);
        setScrewPitch(0.0f);
// REMOVED FOLLOWING ON 24/7/2015: causes problem when switching modes. The physics engine plugin will now not set limits if the range>=360
//      if (_jointType==sim_joint_revolute_subtype)
//          _posRange=tt::getLimitedFloat(0.0f,piValTimes2_f,_posRange);
        if (_jointType==sim_joint_spherical_subtype)
            setPositionRange(piValue_f);
        if (_jointMode!=theMode)
        {
            _dynVelCtrl_currentVelAccel[0]=double(_velCalc_vel);
            _dynVelCtrl_currentVelAccel[1]=0.0;
            _dynCtrl_pid_cumulErr=0.0f;
        }
        App::currentWorld->sceneObjects->actualizeObjectInformation();
        md=theMode;
    }
    if (theMode==sim_jointmode_ik_deprecated)
    {
        App::currentWorld->sceneObjects->actualizeObjectInformation();
        md=theMode;
    }

    bool diff=(_jointMode!=md);
    if (diff)
    {
        _jointMode=md;
        if (getObjectCanSync())
            _setJointMode_sendOldIk(_jointMode);
        setPosition(getPosition(),false);
    }
    return(diff);
}

void CJoint::_setJointMode_sendOldIk(int theMode) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setJointMode(_ikPluginCounterpartHandle,theMode);
}

void CJoint::_rectifyDependentJoints()
{
    for (size_t i=0;i<_directDependentJoints.size();i++)
    {
        if ((_directDependentJoints[i]->getJointMode()==sim_jointmode_dependent)||(_directDependentJoints[i]->getJointMode()==sim_jointmode_reserved_previously_ikdependent)) // second part on 3/7/2014
            _directDependentJoints[i]->setPosition(0.0f,false); // value doesn't matter!
    }
}

void CJoint::setSphericalTransformation(const C4Vector& tr)
{
    C4Vector transf(tr);
    if (_posRange<piValue_f*0.99f)
    {
        C3X3Matrix theTr(transf);
        C3Vector zReset(0.0f,0.0f,1.0f);
        float angle=zReset.getAngle(theTr.axis[2]);
        if (angle>_posRange)
        { // We have to limit the movement:
            C3Vector rotAxis((theTr.axis[2]^zReset).getNormalized());
            C4Vector rot(angle-_posRange,rotAxis);
            transf=rot*transf;
        }
    }
    bool diff=(_sphericalTransf!=transf);
    if (diff)
    {
        _sphericalTransf=transf;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="quaternion";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,false,cmd,true);
            float q[4]={transf(1),transf(2),transf(3),transf(0)};
            data->appendMapObject_stringFloatArray(cmd,q,4);
            C7Vector trr(getIntrinsicTransformation(true));
            float p[7]={trr.X(0),trr.X(1),trr.X(2),trr.Q(1),trr.Q(2),trr.Q(3),trr.Q(0)};
            data->appendMapObject_stringFloatArray("intrinsicPose",p,7);
            App::worldContainer->pushEvent(event);
        }
        if (getObjectCanSync())
            _setSphericalTransformation_sendOldIk(transf);
    }
}

void CJoint::_setSphericalTransformation_sendOldIk(const C4Vector& tr) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if ( (_ikPluginCounterpartHandle!=-1)&&(_jointType==sim_joint_spherical_subtype) )
        CPluginContainer::ikPlugin_setSphericalJointQuaternion(_ikPluginCounterpartHandle,tr);
}

void CJoint::setMaxStepSize_old(float stepS)
{
    if (_jointType==sim_joint_revolute_subtype)
        tt::limitValue(0.01f*degToRad_f,100000.0f,stepS); // high number for screws!
    if (_jointType==sim_joint_prismatic_subtype)
        tt::limitValue(0.0001f,1000.0f,stepS);
    if (_jointType==sim_joint_spherical_subtype)
        tt::limitValue(0.01f*degToRad_f,piValue_f,stepS);
    bool diff=(_maxStepSize_old!=stepS);
    if (diff)
    {
        _maxStepSize_old=stepS;
        if (getObjectCanSync())
            _setMaxStepSize_sendOldIk(stepS);
    }
}

void CJoint::_setMaxStepSize_sendOldIk(float stepS) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setJointMaxStepSize(_ikPluginCounterpartHandle,_maxStepSize_old);
}

void CJoint::setIKWeight_old(float newWeight)
{
    newWeight=tt::getLimitedFloat(-1.0f,1.0f,newWeight);
    bool diff=(_ikWeight_old!=newWeight);
    if (diff)
    {
        _ikWeight_old=newWeight;
        if (getObjectCanSync())
            _setIkWeight_sendOldIk(newWeight);
    }
}

void CJoint::_setIkWeight_sendOldIk(float newWeight) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setJointIkWeight(_ikPluginCounterpartHandle,_ikWeight_old);
}

void CJoint::setPosition(float pos,bool setDirect)
{
    if (!setDirect)
    {
        if (_isCyclic)
            pos=tt::getNormalizedAngle(pos);
        else
        {
            if (pos>(getPositionMin()+getPositionRange()))
                pos=getPositionMin()+getPositionRange();
            if (pos<getPositionMin())
                pos=getPositionMin();
        }

        if ( (_jointMode==sim_jointmode_dependent)||(_jointMode==sim_jointmode_reserved_previously_ikdependent) )
        {
            float linked=0.0f;
            if (_dependencyMasterJointHandle!=-1)
            {
                CJoint* anAct=App::currentWorld->sceneObjects->getJointFromHandle(_dependencyMasterJointHandle);
                if (anAct!=nullptr)
                    linked=_dependencyJointMult*anAct->getPosition();
            }
            pos=linked+_dependencyJointOffset;
        }
    }
    bool diff=(_pos!=pos);
    if (diff)
    {
        _pos=pos;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="position";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,false,cmd,true);
            data->appendMapObject_stringFloat(cmd,_pos);

            C7Vector tr(getIntrinsicTransformation(true));
            float p[7]={tr.X(0),tr.X(1),tr.X(2),tr.Q(1),tr.Q(2),tr.Q(3),tr.Q(0)};
            data->appendMapObject_stringFloatArray("intrinsicPose",p,7);
            App::worldContainer->pushEvent(event);
        }
        if (getObjectCanSync())
            _setPosition_sendOldIk(pos);
        _rectifyDependentJoints();
        setVelocity_DEPRECATED(getVelocity_DEPRECATED());
    }
}

void CJoint::_setPosition_sendOldIk(float pos) const
{ // Overriden from _CJoint_
    // Synchronize with IK plugin:
    if ( (_ikPluginCounterpartHandle!=-1)&&(_jointType!=sim_joint_spherical_subtype) )
        CPluginContainer::ikPlugin_setJointPosition(_ikPluginCounterpartHandle,pos);
}

void CJoint::announceObjectWillBeErased(const CSceneObject* object,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceObjectWillBeErased(object,copyBuffer);
    if (_dependencyMasterJointHandle==object->getObjectHandle())
        setDependencyMasterJointHandle(-1);
    if (_vortexIntParams[5]==object->getObjectHandle()) // that's the Vortex dependency joint
    {
        std::vector<int> ip;
        getVortexIntParams(ip);
        ip[5]=-1;
        setVortexIntParams(ip);
    }
    if (_newtonIntParams[1]==object->getObjectHandle()) // that's the Newton dependency joint
    {
        std::vector<int> ip;
        getNewtonIntParams(ip);
        ip[1]=-1;
        setNewtonIntParams(ip);
    }
    if (_mujocoIntParams[1]==object->getObjectHandle()) // that's the Mujoco dependency joint
    {
        std::vector<int> ip;
        getMujocoIntParams(ip);
        ip[1]=-1;
        setMujocoIntParams(ip);
    }

    // We check if the joint is listed in the _directDependentJoints:
    for (size_t i=0;i<_directDependentJoints.size();i++)
    {
        if (_directDependentJoints[i]==object)
            _directDependentJoints.erase(_directDependentJoints.begin()+i);
    }
}

void CJoint::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID,copyBuffer);
}

void CJoint::display(CViewableBase* renderingObject,int displayAttrib)
{
    EASYLOCK(_objectMutex);
    displayJoint(this,renderingObject,displayAttrib);
}

void CJoint::buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting)
{ // Overridden from CSceneObject
    if (setObjectCanSync(true))
    {
        // Set routing:
        SSyncRoute r;
        r.objHandle=_objectHandle;
        r.objType=sim_syncobj_joint;
        setSyncMsgRouting(parentRouting,r);

        // Build IK plugin counterpart:
        _ikPluginCounterpartHandle=CPluginContainer::ikPlugin_createJoint(_jointType);

        // Update the remote sceneObject:
        CSceneObject::buildUpdateAndPopulateSynchronizationObject(parentRouting);

        // Update the remote joint:
        _setPositionIntervalMin_sendOldIk(_posMin);
        _setPositionIntervalRange_sendOldIk(_posRange);
        _setPositionIsCyclic_sendOldIk(_isCyclic);
        _setScrewPitch_sendOldIk(_screwPitch);
        _setIkWeight_sendOldIk(_ikWeight_old);
        _setMaxStepSize_sendOldIk(_maxStepSize_old);
        _setPosition_sendOldIk(_pos);
        _setSphericalTransformation_sendOldIk(_sphericalTransf);
        _setJointMode_sendOldIk(_jointMode);
    }
}

void CJoint::connectSynchronizationObject()
{ // Overridden from CSceneObject
    if (getObjectCanSync())
    {
        CSceneObject::connectSynchronizationObject();

        _setDependencyJointHandle_sendOldIk(_dependencyMasterJointHandle);
        _setDependencyJointMult_sendOldIk(_dependencyJointMult);
        _setDependencyJointOffset_sendOldIk(_dependencyJointOffset);
    }
}

int CJoint::getDynCtrlMode() const
{
    return(_dynCtrlMode);
}

void CJoint::setDynCtrlMode(int mode)
{
    if (mode!=_dynCtrlMode)
    {
        _dynCtrlMode=mode;
        if ( (_dynCtrlMode==sim_jointdynctrl_spring)||(_dynCtrlMode==sim_jointdynctrl_springcb)||(_dynCtrlMode==sim_jointdynctrl_force) )
            setTargetVelocity(1000.0f); // just a very high value
        if (_dynCtrlMode==sim_jointdynctrl_velocity)
        {
            if (_jointType==sim_joint_prismatic_subtype)
                setTargetVelocity(0.1f);
            else
                setTargetVelocity(piValD2_f);
        }
        _dynVelCtrl_currentVelAccel[0]=double(_velCalc_vel);
        _dynVelCtrl_currentVelAccel[1]=0.0;
        _dynCtrl_pid_cumulErr=0.0f;
    }
}

int CJoint::getDynVelCtrlType() const
{
    return(_dynVelocityCtrlType);
}

int CJoint::getDynPosCtrlType() const
{
    return(_dynPositionCtrlType);
}

void CJoint::setDynVelCtrlType(int mode)
{
    if (mode!=_dynVelocityCtrlType)
    {
        if (_dynCtrlMode==sim_jointdynctrl_velocity)
        {
            _dynVelCtrl_currentVelAccel[0]=double(_velCalc_vel);
            _dynVelCtrl_currentVelAccel[1]=0.0;
        }
        _dynVelocityCtrlType=mode;
    }
}

void CJoint::setDynPosCtrlType(int mode)
{
    if (mode!=_dynPositionCtrlType)
    {
        if (_dynCtrlMode==sim_jointdynctrl_position)
        {
            _dynPosCtrl_currentVelAccel[0]=double(_velCalc_vel);
            _dynPosCtrl_currentVelAccel[1]=0.0;
            _dynCtrl_pid_cumulErr=0.0f;
        }
        _dynPositionCtrlType=mode;
    }
}

float CJoint::getTargetVelocity() const
{
    return(_targetVel);
}

bool CJoint::getMotorLock() const
{ // deprecated. Should not be used anymore
    return(_motorLock);
}

void CJoint::getMaxVelAccelJerk(float maxVelAccelJerk[3]) const
{
    maxVelAccelJerk[0]=_maxVelAccelJerk[0];
    maxVelAccelJerk[1]=_maxVelAccelJerk[1];
    maxVelAccelJerk[2]=_maxVelAccelJerk[2];
}

void CJoint::setMaxVelAccelJerk(const float maxVelAccelJerk[3])
{
    bool diff=( (_maxVelAccelJerk[0]!=maxVelAccelJerk[0])||(_maxVelAccelJerk[1]!=maxVelAccelJerk[1])||(_maxVelAccelJerk[2]!=maxVelAccelJerk[2]) );
    if (diff)
    {
        _maxVelAccelJerk[0]=maxVelAccelJerk[0];
        _maxVelAccelJerk[1]=maxVelAccelJerk[1];
        _maxVelAccelJerk[2]=maxVelAccelJerk[2];
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="maxVelAccelJerk";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,false,cmd,true);
            data->appendMapObject_stringFloatArray(cmd,_maxVelAccelJerk,3);
            App::worldContainer->pushEvent(event);
        }
    }
}

float CJoint::getTargetForce(bool signedValue) const
{
    float retVal=_targetForce;
    if (!signedValue)
        retVal=fabs(retVal);
    return(retVal);
}

bool CJoint::getHybridFunctionality_old() const
{
    return(_jointHasHybridFunctionality);
}

float CJoint::getEngineFloatParam(int what,bool* ok) const
{
    if (ok!=nullptr)
        ok[0]=true;
    if ((what>sim_bullet_joint_float_start)&&(what<sim_bullet_joint_float_end))
    {
        int w=what-sim_bullet_joint_stoperp+simi_bullet_joint_stoperp;
        return(_bulletFloatParams[w]);
    }
    if ((what>sim_ode_joint_float_start)&&(what<sim_ode_joint_float_end))
    {
        int w=what-sim_ode_joint_stoperp+simi_ode_joint_stoperp;
        return(_odeFloatParams[w]);
    }
    if ((what>sim_vortex_joint_float_start)&&(what<sim_vortex_joint_float_end))
    {
        int w=what-sim_vortex_joint_lowerlimitdamping+simi_vortex_joint_lowerlimitdamping;
        return(_vortexFloatParams[w]);
    }
    if ((what>sim_newton_joint_float_start)&&(what<sim_newton_joint_float_end))
    {
        int w=what-sim_newton_joint_dependencyfactor+simi_newton_joint_dependencyfactor;
        return(_newtonFloatParams[w]);
    }
    if ((what>sim_mujoco_joint_float_start)&&(what<sim_mujoco_joint_float_end))
    {
        int w=what-sim_mujoco_joint_solreflimit1+simi_mujoco_joint_solreflimit1;
        return(_mujocoFloatParams[w]);
    }
    if (ok!=nullptr)
        ok[0]=false;
    return(0.0f);
}

int CJoint::getEngineIntParam(int what,bool* ok) const
{
    if (ok!=nullptr)
        ok[0]=true;
    if ((what>sim_bullet_joint_int_start)&&(what<sim_bullet_joint_int_end))
    {
        // no int params for now
        // search for bji11032016
        if (ok!=nullptr)
            ok[0]=false;
        return(0);
    }
    if ((what>sim_ode_joint_int_start)&&(what<sim_ode_joint_int_end))
    {
        // no int params for now
        // search for oji11032016
        if (ok!=nullptr)
            ok[0]=false;
        return(0);
    }
    if ((what>sim_vortex_joint_int_start)&&(what<sim_vortex_joint_int_end))
    {
        int w=what-sim_vortex_joint_bitcoded+simi_vortex_joint_bitcoded;
        return(_vortexIntParams[w]);
    }
    if ((what>sim_newton_joint_int_start)&&(what<sim_newton_joint_int_end))
    {
        int w=what-sim_newton_joint_objectid+simi_newton_joint_objectid;
        return(_newtonIntParams[w]);
    }
    if ((what>sim_mujoco_joint_int_start)&&(what<sim_mujoco_joint_int_end))
    {
        int w=what-sim_mujoco_joint_objectid+simi_mujoco_joint_objectid;
        return(_mujocoIntParams[w]);
    }
    if (ok!=nullptr)
        ok[0]=false;
    return(0);
}

bool CJoint::getEngineBoolParam(int what,bool* ok) const
{
    if (ok!=nullptr)
        ok[0]=true;
    if ((what>sim_bullet_joint_bool_start)&&(what<sim_bullet_joint_bool_end))
    {
        // No bool params for now
        // search for bjb11032016
        if (ok!=nullptr)
            ok[0]=false;
        return(0);
    }
    if ((what>sim_ode_joint_bool_start)&&(what<sim_ode_joint_bool_end))
    {
        // No bool params for now
        // search for ojb11032016
        if (ok!=nullptr)
            ok[0]=false;
        return(0);
    }
    if ((what>sim_vortex_joint_bool_start)&&(what<sim_vortex_joint_bool_end))
    {
        int b=1;
        int w=(what-sim_vortex_joint_motorfrictionenabled);
        while (w>0) {b*=2; w--;}
        return((_vortexIntParams[simi_vortex_joint_bitcoded]&b)!=0);
    }
    if ((what>sim_newton_joint_bool_start)&&(what<sim_newton_joint_bool_end))
    {
        // No bool params for now
        // search for njb11032016
        if (ok!=nullptr)
            ok[0]=false;
        return(0);
    }
    if ((what>sim_mujoco_joint_bool_start)&&(what<sim_mujoco_joint_bool_end))
    {
        // No bool params for now
        if (ok!=nullptr)
            ok[0]=false;
        return(0);
    }
    if (ok!=nullptr)
        ok[0]=false;
    return(0);
}

void CJoint::getBulletFloatParams(std::vector<float>& p) const
{
    p.assign(_bulletFloatParams.begin(),_bulletFloatParams.end());
}

void CJoint::getBulletIntParams(std::vector<int>& p) const
{
    p.assign(_bulletIntParams.begin(),_bulletIntParams.end());
}

void CJoint::getOdeFloatParams(std::vector<float>& p) const
{
    p.assign(_odeFloatParams.begin(),_odeFloatParams.end());
}

void CJoint::getOdeIntParams(std::vector<int>& p) const
{
    p.assign(_odeIntParams.begin(),_odeIntParams.end());
}

void CJoint::getVortexFloatParams(std::vector<float>& p) const
{
    p.assign(_vortexFloatParams.begin(),_vortexFloatParams.end());
}

void CJoint::getVortexIntParams(std::vector<int>& p) const
{
    p.assign(_vortexIntParams.begin(),_vortexIntParams.end());
}

void CJoint::getNewtonFloatParams(std::vector<float>& p) const
{
    p.assign(_newtonFloatParams.begin(),_newtonFloatParams.end());
}

void CJoint::getNewtonIntParams(std::vector<int>& p) const
{
    p.assign(_newtonIntParams.begin(),_newtonIntParams.end());
}

void CJoint::getMujocoFloatParams(std::vector<float>& p) const
{
    p.assign(_mujocoFloatParams.begin(),_mujocoFloatParams.end());
}

void CJoint::getMujocoIntParams(std::vector<int>& p) const
{
    p.assign(_mujocoIntParams.begin(),_mujocoIntParams.end());
}

int CJoint::getVortexDependentJointId() const
{
    return(_vortexIntParams[5]);
}

int CJoint::getNewtonDependentJointId() const
{
    return(_newtonIntParams[1]);
}
int CJoint::getMujocoDependentJointId() const
{
    return(_mujocoIntParams[1]);
}

float CJoint::getPosition() const
{
    return(_pos);
}

float CJoint::getTargetPosition() const
{
    return(_targetPos);
}

C7Vector CJoint::getIntrinsicTransformation(bool includeDynErrorComponent) const
{
    C7Vector jointTr;
    if (getJointType()==sim_joint_revolute_subtype)
    {
        jointTr.Q.setAngleAndAxis(_pos,C3Vector(0.0f,0.0f,1.0f));
        jointTr.X(0)=0.0f;
        jointTr.X(1)=0.0f;
        jointTr.X(2)=_pos*getScrewPitch();
    }
    if (getJointType()==sim_joint_prismatic_subtype)
    {
        jointTr.Q.setIdentity();
        jointTr.X(0)=0.0f;
        jointTr.X(1)=0.0f;
        jointTr.X(2)=_pos;
    }
    if (getJointType()==sim_joint_spherical_subtype)
    {
        jointTr.Q=_sphericalTransf;
        jointTr.X.clear();
    }
    if (includeDynErrorComponent)
        jointTr=jointTr*_intrinsicTransformationError;
    return(jointTr);
}

C7Vector CJoint::getFullLocalTransformation() const
{
    return(_localTransformation*getIntrinsicTransformation(true));
}

void CJoint::getPid(float& p_param,float& i_param,float& d_param,int engine/*=-1 --> current engine*/) const
{
    if (engine==-1)
        engine=App::currentWorld->dynamicsContainer->getDynamicEngineType(nullptr);
    if (engine==sim_physics_bullet)
    {
        p_param=getEngineFloatParam(sim_bullet_joint_pospid1,nullptr);
        i_param=getEngineFloatParam(sim_bullet_joint_pospid2,nullptr);
        d_param=getEngineFloatParam(sim_bullet_joint_pospid3,nullptr);
    }
    if (engine==sim_physics_ode)
    {
        p_param=getEngineFloatParam(sim_ode_joint_pospid1,nullptr);
        i_param=getEngineFloatParam(sim_ode_joint_pospid2,nullptr);
        d_param=getEngineFloatParam(sim_ode_joint_pospid3,nullptr);
    }
    if (engine==sim_physics_vortex)
    {
        p_param=getEngineFloatParam(sim_vortex_joint_pospid1,nullptr);
        i_param=getEngineFloatParam(sim_vortex_joint_pospid2,nullptr);
        d_param=getEngineFloatParam(sim_vortex_joint_pospid3,nullptr);
    }
    if (engine==sim_physics_newton)
    {
        p_param=getEngineFloatParam(sim_newton_joint_pospid1,nullptr);
        i_param=getEngineFloatParam(sim_newton_joint_pospid2,nullptr);
        d_param=getEngineFloatParam(sim_newton_joint_pospid3,nullptr);
    }
    if (engine==sim_physics_mujoco)
    {
        p_param=getEngineFloatParam(sim_mujoco_joint_pospid1,nullptr);
        i_param=getEngineFloatParam(sim_mujoco_joint_pospid2,nullptr);
        d_param=getEngineFloatParam(sim_mujoco_joint_pospid3,nullptr);
    }
}

void CJoint::getKc(float& k_param,float& c_param) const
{
    k_param=_dynCtrl_kc[0];
    c_param=_dynCtrl_kc[1];
}

float CJoint::getLength()  const
{
    return(_length);
}

float CJoint::getDiameter() const
{
    return(_diameter);
}

int CJoint::getJointType() const
{
    return(_jointType);
}

float CJoint::getScrewPitch() const
{
    return(_screwPitch);
}

C4Vector CJoint::getSphericalTransformation() const
{
    return(_sphericalTransf);
}

bool CJoint::getIsCyclic() const
{
    if (_jointType==sim_joint_prismatic_subtype)
        return(false);
    return(_isCyclic);
}

float CJoint::getPositionMin() const
{
    return(_posMin);
}

float CJoint::getPositionRange() const
{
    return(_posRange);
}

float CJoint::getIKWeight_old() const
{
    return(_ikWeight_old);
}

float CJoint::getMaxStepSize_old() const
{
    return(_maxStepSize_old);
}

int CJoint::getJointMode() const
{
    return(_jointMode);
}

int CJoint::getDependencyMasterJointHandle() const
{
    return(_dependencyMasterJointHandle);
}

float CJoint::getDependencyJointMult() const
{
    return(_dependencyJointMult);
}

float CJoint::getDependencyJointOffset() const
{
    return(_dependencyJointOffset);
}

void CJoint::setBulletIntParams(const std::vector<int>& p)
{
    bool diff=(_bulletIntParams.size()!=p.size());
    if (!diff)
    {
        for (size_t i=0;i<p.size();i++)
        {
            if (_bulletIntParams[i]!=p[i])
            {
                diff=true;
                break;
            }
        }
    }
    if (diff)
        _bulletIntParams.assign(p.begin(),p.end());
}

void CJoint::setOdeIntParams(const std::vector<int>& p)
{
    bool diff=(_odeIntParams.size()!=p.size());
    if (!diff)
    {
        for (size_t i=0;i<p.size();i++)
        {
            if (_odeIntParams[i]!=p[i])
            {
                diff=true;
                break;
            }
        }
    }
    if (diff)
        _odeIntParams.assign(p.begin(),p.end());
}

void CJoint::setVortexIntParams(const std::vector<int>& p)
{
    bool diff=(_vortexIntParams.size()!=p.size());
    if (!diff)
    {
        for (size_t i=0;i<p.size();i++)
        {
            if (_vortexIntParams[i]!=p[i])
            {
                diff=true;
                break;
            }
        }
    }
    if (diff)
        _vortexIntParams.assign(p.begin(),p.end());
}

void CJoint::setNewtonIntParams(const std::vector<int>& p)
{
    bool diff=(_newtonIntParams.size()!=p.size());
    if (!diff)
    {
        for (size_t i=0;i<p.size();i++)
        {
            if (_newtonIntParams[i]!=p[i])
            {
                diff=true;
                break;
            }
        }
    }
    if (diff)
        _newtonIntParams.assign(p.begin(),p.end());
}

void CJoint::setMujocoIntParams(const std::vector<int>& p)
{
    bool diff=(_mujocoIntParams.size()!=p.size());
    if (!diff)
    {
        for (size_t i=0;i<p.size();i++)
        {
            if (_mujocoIntParams[i]!=p[i])
            {
                diff=true;
                break;
            }
        }
    }
    if (diff)
        _mujocoIntParams.assign(p.begin(),p.end());
}

void CJoint::setMotorLock(bool e)
{
    bool diff=(_motorLock!=e);
    if (diff)
        _motorLock=e;
}

void CJoint::setIntrinsicTransformationError(const C7Vector& tr)
{
    bool diff=(_intrinsicTransformationError!=tr);
    if (diff)
    {
        _intrinsicTransformationError=tr;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="position";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,false,cmd,true);
            data->appendMapObject_stringFloat(cmd,_pos);
            C7Vector tr2(getIntrinsicTransformation(true));
            float p[7]={tr2.X(0),tr2.X(1),tr2.X(2),tr2.Q(1),tr2.Q(2),tr2.Q(3),tr2.Q(0)};
            data->appendMapObject_stringFloatArray("intrinsicPose",p,7);
            App::worldContainer->pushEvent(event);
        }
    }
}

CColorObject* CJoint::getColor(bool part2)
{
    if (part2)
        return(&_color_removeSoon);
    return(&_color);
}

void CJoint::_fixVortexInfVals()
{ // to fix a past complication (i.e. neg. val. of unsigned would be inf)
    for (size_t i=0;i<47;i++)
    {
        if (_vortexFloatParams[i]<0.0)
            _vortexFloatParams[i]=FLT_MAX;
    }
    // values at index 47 and later are signed
}
