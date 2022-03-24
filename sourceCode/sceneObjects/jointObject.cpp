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
        _jointMode=sim_jointmode_force;
        _positionIsCyclic=true;
        _jointPositionRange=piValTimes2_f;
        _jointMinPosition=-piValue_f;
        _maxStepSize=10.0f*degToRad_f;
        _dynamicMotorMaximumForce=2.5f; // 0.25 m x 1kg x 9.81
        _dynamicMotorUpperLimitVelocity=360.0f*degToRad_f;
        _maxAcceleration_DEPRECATED=60.0f*degToRad_f;
    }
    if (jointType==sim_joint_prismatic_subtype)
    {
        _objectName_old=IDSOGL_PRISMATIC_JOINT;
        _objectAlias=IDSOGL_PRISMATIC_JOINT;
        _jointMode=sim_jointmode_force;
        _positionIsCyclic=false;
        _jointPositionRange=1.0f;
        _jointMinPosition=-0.5f;
        _maxStepSize=0.1f;
        _dynamicMotorMaximumForce=50.0f; // 5kg x 9.81
        _dynamicMotorUpperLimitVelocity=10.0f;
        _maxAcceleration_DEPRECATED=0.1f;
    }
    if (jointType==sim_joint_spherical_subtype)
    {
        _objectName_old=IDSOGL_SPHERICAL_JOINT;
        _objectAlias=IDSOGL_SPHERICAL_JOINT;
        _jointMode=sim_jointmode_force;
        _positionIsCyclic=true;
        _jointPositionRange=piValue_f;
        _jointMinPosition=0.0f;
        _maxStepSize=10.0f*degToRad_f;
        _dynamicMotorMaximumForce=0.0f;
        _dynamicMotorUpperLimitVelocity=0.0f;
        _maxAcceleration_DEPRECATED=60.0f*degToRad_f;
    }
    _objectAltName_old=tt::getObjectAltNameFromObjectName(_objectName_old.c_str());
    computeBoundingBox();
}

void CJoint::_commonInit()
{
    _objectType=sim_object_joint_type;
    _localObjectSpecialProperty=0;

    _jointType=sim_joint_revolute_subtype;
    _screwPitch=0.0f;
    _sphericalTransformation.setIdentity();
    _jointPosition=0.0f;

    _jointMode=sim_jointmode_passive;
    _dependencyMasterJointHandle=-1;
    _dependencyJointMult=1.0f;
    _dependencyJointOffset=0.0f;

    // Dynamic values:
    _dynamicMotorEnabled=false;
    _dynamicMotorTargetVelocity=0.0f;
    _dynamicLockModeWhenInVelocityControl=false;
    _dynamicMotorMaximumForce=1000.0f; // This value has to be adjusted according to the joint type
    _intrinsicTransformationError.setIdentity();

    _dynamicMotorControlLoopEnabled=false;
    _dynamicMotorPositionControl_P=0.1f;
    _dynamicMotorPositionControl_I=0.0f;
    _dynamicMotorPositionControl_D=0.0f;
    _dynamicMotorSpringControl_K=0.1f;
    _dynamicMotorSpringControl_C=0.0f;
    _dynamicMotorPositionControl_targetPosition=_jointPosition;
    _dynamicMotorPositionControl_torqueModulation=false;
    _dynamicMotorPositionControl_targetPosition=0.0f;
    _jointHasHybridFunctionality=false;

    // Bullet parameters
    // ----------------------------------------------------
    // next is index 0:
    _bulletFloatParams.push_back(0.2f); // simi_bullet_joint_stoperp
    _bulletFloatParams.push_back(0.0f); // simi_bullet_joint_stopcfm
    _bulletFloatParams.push_back(0.0f); // simi_bullet_joint_normalcfm
    _bulletFloatParams.push_back(0.0f); // Free
    _bulletFloatParams.push_back(0.0f); // Free
    // BULLET_JOINT_FLOAT_PARAM_CNT_CURRENT=5

    // next is index 0:
    _bulletIntParams.push_back(0); // Free
    // BULLET_JOINT_INT_PARAM_CNT_CURRENT=1
    // ----------------------------------------------------

    // ODE parameters
    // ----------------------------------------------------
    // next is index 0:
    _odeFloatParams.push_back(0.6f); // simi_ode_joint_stoperp
    _odeFloatParams.push_back(0.00001f); // simi_ode_joint_stopcfm
    _odeFloatParams.push_back(0.0f); // simi_ode_joint_bounce
    _odeFloatParams.push_back(1.0f); // simi_ode_joint_fudgefactor
    _odeFloatParams.push_back(0.00001f); // simi_ode_joint_normalcfm
    // ODE_JOINT_FLOAT_PARAM_CNT_CURRENT=5

    // next is index 0:
    _odeIntParams.push_back(0); // Free
    // ODE_JOINT_INT_PARAM_CNT_CURRENT=1
    // ----------------------------------------------------

    // Vortex parameters (neg. stiffness --> INF):
    // ----------------------------------------------------
    // next is index 0:
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_lowerlimitdamping
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_upperlimitdamping
    _vortexFloatParams.push_back(-1.0f); // simi_vortex_joint_lowerlimitstiffness
    _vortexFloatParams.push_back(-1.0f); // simi_vortex_joint_upperlimitstiffness
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_lowerlimitrestitution
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_upperlimitrestitution
    _vortexFloatParams.push_back(-1.0f); // simi_vortex_joint_lowerlimitmaxforce
    _vortexFloatParams.push_back(-1.0f); // simi_vortex_joint_upperlimitmaxforce
    _vortexFloatParams.push_back(0.001f); // simi_vortex_joint_motorconstraintfrictioncoeff
    _vortexFloatParams.push_back(10.0f); // simi_vortex_joint_motorconstraintfrictionmaxforce
    // next is index 10:
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_motorconstraintfrictionloss
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p0loss
    _vortexFloatParams.push_back(-1.0f); // simi_vortex_joint_p0stiffness
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p0damping
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p0frictioncoeff
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p0frictionmaxforce
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p0frictionloss
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p1loss
    _vortexFloatParams.push_back(-1.0f); // simi_vortex_joint_p1stiffness
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p1damping
    // next is index 20:
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p1frictioncoeff
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p1frictionmaxforce
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p1frictionloss
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p2loss
    _vortexFloatParams.push_back(-1.0f); // simi_vortex_joint_p2stiffness
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p2damping
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p2frictioncoeff
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p2frictionmaxforce
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_p2frictionloss
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a0loss
    // next is index 30:
    _vortexFloatParams.push_back(-1.0f); // simi_vortex_joint_a0stiffness
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a0damping
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a0frictioncoeff
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a0frictionmaxforce
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a0frictionloss
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a1loss
    _vortexFloatParams.push_back(-1.0f); // simi_vortex_joint_a1stiffness
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a1damping
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a1frictioncoeff
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a1frictionmaxforce
    // next is index 40:
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a1frictionloss
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a2loss
    _vortexFloatParams.push_back(-1.0f); // simi_vortex_joint_a2stiffness
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a2damping
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a2frictioncoeff
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a2frictionmaxforce
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_a2frictionloss
    // next is index 47:
    _vortexFloatParams.push_back(1.0f); // simi_vortex_joint_dependencyfactor
    _vortexFloatParams.push_back(0.0f); // simi_vortex_joint_dependencyoffset
    _vortexFloatParams.push_back(0.0f); // reserved for future ext.
    _vortexFloatParams.push_back(0.0f); // reserved for future ext.
    // VORTEX_JOINT_FLOAT_PARAM_CNT_CURRENT=51

    // next is index 0:
    _vortexIntParams.push_back(simi_vortex_joint_proportionalmotorfriction); // simi_vortex_joint_bitcoded
    _vortexIntParams.push_back(0); // simi_vortex_joint_relaxationenabledbc. 1 bit per dof
    _vortexIntParams.push_back(0); // simi_vortex_joint_frictionenabledbc. 1 bit per dof
    _vortexIntParams.push_back(1+2+4+8+16+32); // simi_vortex_joint_frictionproportionalbc. 1 bit per dof
    _vortexIntParams.push_back(-1); // simi_vortex_joint_objectid
    _vortexIntParams.push_back(-1); // simi_vortex_joint_dependentobjectid
    _vortexIntParams.push_back(0); // reserved for future ext.
    // VORTEX_JOINT_INT_PARAM_CNT_CURRENT=7
    // ----------------------------------------------------

    // Newton parameters (neg. stiffness --> INF):
    // ----------------------------------------------------
    // next is index 0:
    _newtonFloatParams.push_back(1.0f); // simi_newton_joint_dependencyfactor
    _newtonFloatParams.push_back(0.0f); // simi_newton_joint_dependencyoffset
    // NEWTON_JOINT_FLOAT_PARAM_CNT_CURRENT=2

    // next is index 0:
    _newtonIntParams.push_back(-1); // simi_newton_joint_objectid. The ID is redefined in each session
    _newtonIntParams.push_back(-1); // simi_newton_joint_dependentobjectid
    // NEWTON_JOINT_INT_PARAM_CNT_CURRENT=2
    // ----------------------------------------------------

    _ikWeight=1.0f;
    _diameter=0.02f;
    _length=0.15f;

    _positionIsCyclic=true;
    _jointPositionRange=piValTimes2_f;
    _jointMinPosition=-piValue_f;
    _maxStepSize=10.0f*degToRad_f;

    _visibilityLayer=JOINT_LAYER;
    _objectAlias=IDSOGL_JOINT;
    _objectName_old=IDSOGL_JOINT;
    _objectAltName_old=tt::getObjectAltNameFromObjectName(_objectName_old.c_str());

    _cumulatedForceOrTorque=0.0f;
    _cumulativeForceOrTorqueTmp=0.0f;
    _lastForceOrTorque_dynStep=0.0f;
    _lastForceOrTorqueValid_dynStep=false;
    _averageForceOrTorqueValid=false;
    _measuredJointVelocity_velocityMeasurement=0.0f;
    _previousJointPositionIsValid=false;
    _jointPositionForMotionHandling_DEPRECATED=_jointPosition;
    _velocity_DEPRECATED=0.0f;
    _targetVelocity_DEPRECATED=0.0f;
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

void CJoint::setHybridFunctionality(bool h)
{
    if ( (_jointType!=sim_joint_spherical_subtype)&&(_jointMode!=sim_jointmode_force) )
    {
        bool diff=(_jointHasHybridFunctionality!=h);
        if (diff)
        {
            _jointHasHybridFunctionality=h;
            if (h)
            {
                setEnableDynamicMotor(true);
                setEnableDynamicMotorControlLoop(true);
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
}

void CJoint::setDynamicMotorTargetVelocity(float v)
{
    if (_jointType!=sim_joint_spherical_subtype)
    {
        if (_jointType==sim_joint_revolute_subtype)
            v=tt::getLimitedFloat(-90000.0f*degToRad_f,+90000.0f*degToRad_f,v); // 250 rot/sec --> 0.25 rot/1ms
        if (_jointType==sim_joint_prismatic_subtype)
            v=tt::getLimitedFloat(-100.0f,+100.0f,v); // 100 m/sec --> 0.1 m/1ms
        bool diff=(_dynamicMotorTargetVelocity!=v);
        if (diff)
            _dynamicMotorTargetVelocity=v;
    }
}

void CJoint::setDynamicMotorUpperLimitVelocity(float v)
{
    if (_jointType!=sim_joint_spherical_subtype)
    {
        v=tt::getLimitedFloat(0.01f,20.0f,v);
        bool diff=(_dynamicMotorUpperLimitVelocity!=v);
        if (diff)
            _dynamicMotorUpperLimitVelocity=v;
    }
}

void CJoint::setDynamicMotorMaximumForce(float f,bool isSigned)
{
    if (_jointType!=sim_joint_spherical_subtype)
    {
        if (isSigned)
        {
            if (_dynamicMotorTargetVelocity*f<0.0f)
                setDynamicMotorTargetVelocity(-_dynamicMotorTargetVelocity);
            f=fabs(f);
        }
        if (_jointType==sim_joint_revolute_subtype)
            f=tt::getLimitedFloat(0.0f,+100000000000.0f,f);
        if (_jointType==sim_joint_prismatic_subtype)
            f=tt::getLimitedFloat(0.0f,+10000000000.0f,f);
        bool diff=(_dynamicMotorMaximumForce!=f);
        if (diff)
            _dynamicMotorMaximumForce=f;
    }
}

void CJoint::setDynamicMotorPositionControlParameters(float p_param,float i_param,float d_param)
{
    p_param=tt::getLimitedFloat(-1000.0f,1000.0f,p_param);
    i_param=tt::getLimitedFloat(-1000.0f,1000.0f,i_param);
    d_param=tt::getLimitedFloat(-1000.0f,1000.0f,d_param);
    bool diff=(_dynamicMotorPositionControl_P!=p_param)||(_dynamicMotorPositionControl_I!=i_param)||(_dynamicMotorPositionControl_D!=d_param);
    if (diff)
    {
        _dynamicMotorPositionControl_P=p_param;
        _dynamicMotorPositionControl_I=i_param;
        _dynamicMotorPositionControl_D=d_param;
    }
}

void CJoint::setDynamicMotorSpringControlParameters(float k_param,float c_param)
{
    float maxVal=+10000000000.0f;
    if (_jointType==sim_joint_revolute_subtype)
        maxVal=+100000000000.0f;
    k_param=tt::getLimitedFloat(-maxVal,maxVal,k_param);
    c_param=tt::getLimitedFloat(-maxVal,maxVal,c_param);
    bool diff=(_dynamicMotorSpringControl_K!=k_param)||(_dynamicMotorSpringControl_C!=c_param);
    if (diff)
    {
        _dynamicMotorSpringControl_K=k_param;
        _dynamicMotorSpringControl_C=c_param;
    }
}

void CJoint::setDynamicMotorPositionControlTargetPosition(float pos)
{
    if (_jointType!=sim_joint_spherical_subtype)
    {
        if ( (_jointType==sim_joint_revolute_subtype)&&_positionIsCyclic )
            pos=tt::getNormalizedAngle(pos);
        bool diff=(_dynamicMotorPositionControl_targetPosition!=pos);
        if (diff)
            _dynamicMotorPositionControl_targetPosition=pos;
    }
}

void CJoint::setDynamicMotorReflectedPosition_useOnlyFromDynamicPart(float rfp)
{
    {   // When the joint is in dynamic mode we disable the joint limits and allow a cyclic behaviour (revolute joints)
        // This is because dynamic joints can over or undershoot limits.
        // So we set the position directly, without checking for limits.
        // Turn count is taken care by the physics plugin.
        setPosition(rfp,true);
    }
    _rectifyDependentJoints();
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

void CJoint::measureJointVelocity(float dt)
{
    if (_jointType!=sim_joint_spherical_subtype)
    {
        float vel=_measuredJointVelocity_velocityMeasurement;
        if (_previousJointPositionIsValid)
        {
            if (_positionIsCyclic)
                vel=tt::getAngleMinusAlpha(_jointPosition,_previousJointPosition_velocityMeasurement)/dt;
            else
                vel=(_jointPosition-_previousJointPosition_velocityMeasurement)/dt;
        }
        _previousJointPositionIsValid=true;
        _previousJointPosition_velocityMeasurement=_jointPosition;
        _measuredJointVelocity_velocityMeasurement=vel;
    }
}

void CJoint::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    _previousJointPositionIsValid=false;
    _measuredJointVelocity_velocityMeasurement=0.0f;
    _previousJointPosition_velocityMeasurement=0.0f;
    _initialPosition=_jointPosition;
    _initialSphericalJointTransformation=_sphericalTransformation;
    setIntrinsicTransformationError(C7Vector::identityTransformation);

    _initialDynamicMotorEnabled=_dynamicMotorEnabled;
    _initialDynamicMotorTargetVelocity=_dynamicMotorTargetVelocity;
    _initialDynamicMotorLockModeWhenInVelocityControl=_dynamicLockModeWhenInVelocityControl;
    _initialDynamicMotorUpperLimitVelocity=_dynamicMotorUpperLimitVelocity;
    _initialDynamicMotorMaximumForce=_dynamicMotorMaximumForce;

    _initialDynamicMotorControlLoopEnabled=_dynamicMotorControlLoopEnabled;
    _initialDynamicMotorPositionControl_P=_dynamicMotorPositionControl_P;
    _initialDynamicMotorPositionControl_I=_dynamicMotorPositionControl_I;
    _initialDynamicMotorPositionControl_D=_dynamicMotorPositionControl_D;
    _initialDynamicMotorSpringControl_K=_dynamicMotorSpringControl_K;
    _initialDynamicMotorSpringControl_C=_dynamicMotorSpringControl_C;
    _initialTargetPosition=_dynamicMotorPositionControl_targetPosition;

    _initialJointMode=_jointMode;
    _initialHybridOperation=_jointHasHybridFunctionality;

    _averageForceOrTorqueValid=false;
    _cumulatedForceOrTorque=0.0f;
    _lastForceOrTorqueValid_dynStep=false;
    _lastForceOrTorque_dynStep=0.0f;
    _cumulativeForceOrTorqueTmp=0.0f;

    _jointPositionForMotionHandling_DEPRECATED=_jointPosition;
    _velocity_DEPRECATED=0.0f;
    _initialVelocity_DEPRECATED=_velocity_DEPRECATED;
    _initialTargetVelocity_DEPRECATED=_targetVelocity_DEPRECATED;
    _initialExplicitHandling_DEPRECATED=_explicitHandling_DEPRECATED;
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

            setEnableDynamicMotor(_initialDynamicMotorEnabled);
            setDynamicMotorTargetVelocity(_initialDynamicMotorTargetVelocity);
            setDynamicMotorLockModeWhenInVelocityControl(_initialDynamicMotorLockModeWhenInVelocityControl);
            setDynamicMotorUpperLimitVelocity(_initialDynamicMotorUpperLimitVelocity);
            setDynamicMotorMaximumForce(_initialDynamicMotorMaximumForce,false);

            setEnableDynamicMotorControlLoop(_initialDynamicMotorControlLoopEnabled);
            setDynamicMotorPositionControlParameters(_initialDynamicMotorPositionControl_P,_initialDynamicMotorPositionControl_I,_initialDynamicMotorPositionControl_D);
            setDynamicMotorSpringControlParameters(_initialDynamicMotorSpringControl_K,_initialDynamicMotorSpringControl_C);
            setDynamicMotorPositionControlTargetPosition(_initialTargetPosition);

            setJointMode(_initialJointMode);
            setHybridFunctionality(_initialHybridOperation);

            _explicitHandling_DEPRECATED=_initialExplicitHandling_DEPRECATED;
            _velocity_DEPRECATED=_initialVelocity_DEPRECATED;
            _targetVelocity_DEPRECATED=_initialTargetVelocity_DEPRECATED;
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
        setTargetVelocity_DEPRECATED(_initialTargetVelocity_DEPRECATED);
    }
}

void CJoint::handleJoint_DEPRECATED(float deltaTime)
{ // DEPRECATED. handling the motion here. Not elegant at all. In future, try using the Reflexxes RML library!
    if ( (_jointMode!=sim_jointmode_motion_deprecated)||(!App::currentWorld->mainSettings->jointMotionHandlingEnabled_DEPRECATED) )
        return;
    if (_unlimitedAcceleration_DEPRECATED)
    {
        _velocity_DEPRECATED=_targetVelocity_DEPRECATED;
        if (_velocity_DEPRECATED!=0.0f)
        {
            float newPos=_jointPositionForMotionHandling_DEPRECATED;
            if (!_positionIsCyclic)
            {
                if (_invertTargetVelocityAtLimits_DEPRECATED)
                {
                    float cycleTime=2.0f*_jointPositionRange/_velocity_DEPRECATED;
                    deltaTime=CMath::robustFmod(deltaTime,cycleTime);
                    while (true)
                    {
                        _velocity_DEPRECATED=_targetVelocity_DEPRECATED;
                        float absDist=_jointMinPosition+_jointPositionRange-newPos;
                        if (_velocity_DEPRECATED<0.0f)
                            absDist=newPos-_jointMinPosition;
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
                        _targetVelocity_DEPRECATED*=-1.0f; // We invert the target velocity
                    }
                }
                else
                {
                    newPos+=_velocity_DEPRECATED*deltaTime;
                    float dv=newPos-(_jointMinPosition+_jointPositionRange);
                    float dl=_jointMinPosition-newPos;
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
        if (!_positionIsCyclic)
        {
            minV=_jointMinPosition;
            maxV=_jointMinPosition+_jointPositionRange;
            // Make sure we are within limits:
            float m=float(CLinMotionRoutines::getMaxVelocityAtPosition(newPos,_maxAcceleration_DEPRECATED,minV,maxV,0.0f,0.0f));
            tt::limitValue(-m,m,_velocity_DEPRECATED);
        }

        double velocityDouble=double(_velocity_DEPRECATED);
        double deltaTimeDouble=double(deltaTime);
        while (CLinMotionRoutines::getNextValues(newPos,velocityDouble,_targetVelocity_DEPRECATED,_maxAcceleration_DEPRECATED,minV,maxV,0.0f,0.0f,deltaTimeDouble))
        {
            if (_invertTargetVelocityAtLimits_DEPRECATED)
                _targetVelocity_DEPRECATED*=-1.0f;
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
    if ( (vel!=0.0f)&&((_jointType==sim_joint_prismatic_subtype)||(!_positionIsCyclic))&&(!_unlimitedAcceleration_DEPRECATED) )
    { // We check which is the max allowed:
        float m=float(CLinMotionRoutines::getMaxVelocityAtPosition(_jointPosition,_maxAcceleration_DEPRECATED,_jointMinPosition,_jointMinPosition+_jointPositionRange,0.0f,0.0f));
        tt::limitValue(-m,m,vel);
    }
    _velocity_DEPRECATED=vel;
}

float CJoint::getVelocity_DEPRECATED()
{ // DEPRECATED
    return(_velocity_DEPRECATED);
}

void CJoint::setTargetVelocity_DEPRECATED(float vel)
{ // DEPRECATED
    if (_jointType==sim_joint_spherical_subtype)
        return;
    if (_jointType==sim_joint_prismatic_subtype)
        tt::limitValue(-1000.0f,1000.0f,vel);
    else
        tt::limitValue(-36000.0f*degToRad_f,36000.0f*degToRad_f,vel);
    _targetVelocity_DEPRECATED=vel;
}

float CJoint::getTargetVelocity_DEPRECATED()
{ // DEPRECATED
    return(_targetVelocity_DEPRECATED);
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
        retVal+=gv::getAngleStr(true,_jointPosition)+")";
    }
    if (_jointType==sim_joint_prismatic_subtype)
    {
        retVal+=tt::decorateString(" (",IDSOGL_PRISMATIC,", p=");
        retVal+=gv::getSizeStr(true,_jointPosition)+")";
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
    return(_measuredJointVelocity_velocityMeasurement);
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
        if (_jointMode!=sim_jointmode_force)
        { // no pitch when in torque/force mode
            pitch=tt::getLimitedFloat(-10.0f,10.0f,pitch);
            bool diff=(_screwPitch!=pitch);
            if (diff)
            {
                _screwPitch=pitch;
                if (getObjectCanSync())
                    _setScrewPitch_sendOldIk(pitch);
                if (pitch!=0.0f)
                    setHybridFunctionality(false);
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

void CJoint::setPositionIntervalMin(float min)
{
    if (_jointType!=sim_joint_spherical_subtype)
    {
        if (_jointType==sim_joint_revolute_subtype)
            min=tt::getLimitedFloat(-100000.0f,100000.0f,min);
        if (_jointType==sim_joint_prismatic_subtype)
            min=tt::getLimitedFloat(-1000.0f,1000.0f,min);
        bool diff=(_jointMinPosition!=min);
        if (diff)
        {
            _jointMinPosition=min;
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
        CPluginContainer::ikPlugin_setJointInterval(_ikPluginCounterpartHandle,_positionIsCyclic,_jointMinPosition,_jointPositionRange);
}

void CJoint::setPositionIntervalRange(float range)
{
    if (_jointType==sim_joint_revolute_subtype)
        range=tt::getLimitedFloat(0.001f*degToRad_f,10000000.0f*degToRad_f,range);
    if (_jointType==sim_joint_prismatic_subtype)
        range=tt::getLimitedFloat(0.0f,1000.0f,range);
    if (_jointType==sim_joint_spherical_subtype)
    {
        if (_jointMode!=sim_jointmode_force)
            range=tt::getLimitedFloat(0.001f*degToRad_f,10000000.0f*degToRad_f,range);
        else
            range=piValue_f;
    }
    bool diff=(_jointPositionRange!=range);
    if (diff)
    {
        _jointPositionRange=range;
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
        CPluginContainer::ikPlugin_setJointInterval(_ikPluginCounterpartHandle,_positionIsCyclic,_jointMinPosition,_jointPositionRange);
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
        setPosition(_jointPosition*scalingFactor,false);
        _jointPositionForMotionHandling_DEPRECATED*=scalingFactor;
        setPositionIntervalMin(_jointMinPosition*scalingFactor);
        setPositionIntervalRange(_jointPositionRange*scalingFactor);
        setDependencyJointOffset(_dependencyJointOffset*scalingFactor);
        setMaxStepSize(_maxStepSize*scalingFactor);
        setDynamicMotorPositionControlTargetPosition(_dynamicMotorPositionControl_targetPosition*scalingFactor);

        setDynamicMotorSpringControlParameters(_dynamicMotorSpringControl_K*scalingFactor*scalingFactor,_dynamicMotorSpringControl_C*scalingFactor*scalingFactor);

        setDynamicMotorTargetVelocity(_dynamicMotorTargetVelocity*scalingFactor);
        setDynamicMotorUpperLimitVelocity(_dynamicMotorUpperLimitVelocity*scalingFactor);
        if (_dynamicMotorPositionControl_torqueModulation) // this condition and next line added on 04/10/2013 (Alles Gute zu Geburtstag Mama :) )
            setDynamicMotorMaximumForce(_dynamicMotorMaximumForce*scalingFactor*scalingFactor,false); //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
        else
            setDynamicMotorMaximumForce(_dynamicMotorMaximumForce*scalingFactor*scalingFactor*scalingFactor,false); //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

        // Following removed on 04/10/2013. Why did we have this?!
        //_dynamicMotorPositionControl_targetPosition=_jointPosition;

        _maxAcceleration_DEPRECATED*=scalingFactor;
        _velocity_DEPRECATED*=scalingFactor;
        _targetVelocity_DEPRECATED*=scalingFactor;

        if (_initialValuesInitialized)
        {
            _initialPosition*=scalingFactor;
            _initialTargetPosition*=scalingFactor;

            _initialDynamicMotorTargetVelocity*=scalingFactor;
            _initialDynamicMotorUpperLimitVelocity*=scalingFactor;
            if (_dynamicMotorPositionControl_torqueModulation) // this condition and next line added on 04/10/2013 (Alles Gute zu Geburtstag Mama :) )
                _initialDynamicMotorMaximumForce*=scalingFactor*scalingFactor;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
            else
                _initialDynamicMotorMaximumForce*=scalingFactor*scalingFactor*scalingFactor;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

            // following 2 new since 7/5/2014:
            _initialDynamicMotorSpringControl_K*=scalingFactor*scalingFactor;
            _initialDynamicMotorSpringControl_C*=scalingFactor*scalingFactor;

            _initialVelocity_DEPRECATED*=scalingFactor;
            _initialTargetVelocity_DEPRECATED*=scalingFactor;
        }
    }

    if (_jointType==sim_joint_revolute_subtype)
    {
        setDynamicMotorMaximumForce(_dynamicMotorMaximumForce*scalingFactor*scalingFactor*scalingFactor*scalingFactor,false);//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

        setDynamicMotorSpringControlParameters(_dynamicMotorSpringControl_K*scalingFactor*scalingFactor*scalingFactor*scalingFactor,_dynamicMotorSpringControl_C*scalingFactor*scalingFactor*scalingFactor*scalingFactor);

        if (_initialValuesInitialized)
        {
            _initialDynamicMotorMaximumForce*=scalingFactor*scalingFactor*scalingFactor*scalingFactor;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
            // following 2 new since 7/5/2014:
            _initialDynamicMotorSpringControl_K*=scalingFactor*scalingFactor*scalingFactor*scalingFactor;
            _initialDynamicMotorSpringControl_C*=scalingFactor*scalingFactor*scalingFactor*scalingFactor;
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
        setPosition(_jointPosition*z,false);
        setPositionIntervalMin(_jointMinPosition*z);
        setPositionIntervalRange(_jointPositionRange*z);
        setDependencyJointOffset(_dependencyJointOffset*z);
        setMaxStepSize(_maxStepSize*z);
        setDynamicMotorPositionControlTargetPosition(_dynamicMotorPositionControl_targetPosition*z);

        setDynamicMotorSpringControlParameters(_dynamicMotorSpringControl_K*diam*diam,_dynamicMotorSpringControl_C*diam*diam);

        setDynamicMotorTargetVelocity(_dynamicMotorTargetVelocity*z);
        setDynamicMotorUpperLimitVelocity(_dynamicMotorUpperLimitVelocity*z);
        if (_dynamicMotorPositionControl_torqueModulation) // this condition and next line added on 04/10/2013 (Alles Gute zu Geburtstag Mama :) )
            setDynamicMotorMaximumForce(_dynamicMotorMaximumForce*diam*diam,false); //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
        else
            setDynamicMotorMaximumForce(_dynamicMotorMaximumForce*diam*diam*diam,false); //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

        // Following removed on 04/10/2013. Why did we have this?!
        //_dynamicMotorPositionControl_targetPosition=_jointPosition;

        _jointPositionForMotionHandling_DEPRECATED*=z;
        _maxAcceleration_DEPRECATED*=z;
        _velocity_DEPRECATED*=z;
        _targetVelocity_DEPRECATED*=z;

        if (_initialValuesInitialized)
        {
            _initialPosition*=z;
            _initialTargetPosition*=z;

            _initialDynamicMotorTargetVelocity*=z;
            _initialDynamicMotorUpperLimitVelocity*=z;
            if (_dynamicMotorPositionControl_torqueModulation) // this condition and next line added on 04/10/2013 (Alles Gute zu Geburtstag Mama :) )
                _initialDynamicMotorMaximumForce*=diam*diam;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
            else
                _initialDynamicMotorMaximumForce*=diam*diam*diam;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

            // following 2 new since 7/5/2014:
            _initialDynamicMotorSpringControl_K*=diam*diam;
            _initialDynamicMotorSpringControl_C*=diam*diam;

            _initialVelocity_DEPRECATED*=z;
            _initialTargetVelocity_DEPRECATED*=z;
        }
    }

    if (_jointType==sim_joint_revolute_subtype)
    {
        setDynamicMotorMaximumForce(_dynamicMotorMaximumForce*diam*diam*diam*diam,false);//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

        setDynamicMotorSpringControlParameters(_dynamicMotorSpringControl_K*diam*diam*diam*diam,_dynamicMotorSpringControl_C*diam*diam*diam*diam);

        if (_initialValuesInitialized)
        {
            _initialDynamicMotorMaximumForce*=diam*diam*diam*diam;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
            // following 2 new since 7/5/2014:
            _initialDynamicMotorSpringControl_K*=diam*diam*diam*diam;
            _initialDynamicMotorSpringControl_C*=diam*diam*diam*diam;
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

void CJoint::handleDynJointControl(bool init,int loopCnt,int totalLoops,float currentPos,float effort,float dynStepSize,float errorV,float& velocity,float& forceTorque)
{
    bool spring=_dynamicMotorPositionControl_torqueModulation;
    bool rev=(_jointType==sim_joint_revolute_subtype);
    bool cycl=_positionIsCyclic;
    float lowL=_jointMinPosition;
    float highL=_jointMinPosition+_jointPositionRange;
    float targetPos=_dynamicMotorPositionControl_targetPosition;
    float targetVel=_dynamicMotorTargetVelocity;
    float maxForce=_dynamicMotorMaximumForce;
    float upperLimitVel=_dynamicMotorUpperLimitVelocity;

    // We check if a plugin wants to handle the joint controller:
    bool handleJointHere=true;
    int callbackCount=(int)getAllJointCtrlCallbacks().size();
    if (callbackCount!=0)
    {
        int intParams[3];
        float floatParams[10];
        float retParams[2];
        intParams[0]=0;
        if (init)
            intParams[0]|=1;
        if (rev)
            intParams[0]|=2;
        if (cycl)
            intParams[0]|=4;
        intParams[1]=loopCnt;
        intParams[2]=totalLoops;
        floatParams[0]=currentPos;
        floatParams[1]=targetPos;
        floatParams[2]=errorV;
        floatParams[3]=effort;
        floatParams[4]=dynStepSize;
        floatParams[5]=lowL;
        floatParams[6]=highL;
        floatParams[7]=targetVel;
        floatParams[8]=maxForce;
        floatParams[9]=upperLimitVel;
        for (int i=0;i<callbackCount;i++)
        {
            int res=((jointCtrlCallback)getAllJointCtrlCallbacks()[i])(getObjectHandle(),App::currentWorld->dynamicsContainer->getDynamicEngineType(nullptr),0,intParams,floatParams,retParams);
            if (res==0)
            { // override... we don't want any control on this joint (free joint)
                forceTorque=0.0f;
                velocity=0.0f;
                handleJointHere=false;
                break;
            }
            if (res>0)
            { // override... we use control values provided by the callback
                forceTorque=retParams[0];
                velocity=retParams[1];

                handleJointHere=false;
                break;
            }
        }
    }

    if (handleJointHere)
    { // The plugins didn't want to handle that joint
        CScriptObject* script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo_child(getObjectHandle());
        if (script!=nullptr)
        {
            if (!script->getContainsJointCallbackFunction())
                script=nullptr;
        }
        CScriptObject* cScript=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo_customization(getObjectHandle());
        if (cScript!=nullptr)
        {
            if (!cScript->getContainsJointCallbackFunction())
                cScript=nullptr;
        }
        if ( (script!=nullptr)||(cScript!=nullptr) )
        { // a child or customization scripts want to handle the joint (new calling method)
            // 1. We prepare the in/out stacks:
            CInterfaceStack* inStack=App::worldContainer->interfaceStackContainer->createStack();
            inStack->pushTableOntoStack();
            inStack->pushStringOntoStack("first",0);
            inStack->pushBoolOntoStack(init);
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
            inStack->pushStringOntoStack("currentPos",0);
            inStack->pushFloatOntoStack(currentPos);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("targetPos",0);
            inStack->pushFloatOntoStack(targetPos);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("errorValue",0);
            inStack->pushFloatOntoStack(errorV);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("effort",0);
            inStack->pushFloatOntoStack(effort);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("dynStepSize",0);
            inStack->pushFloatOntoStack(dynStepSize);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("targetVel",0);
            inStack->pushFloatOntoStack(targetVel);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("maxForce",0);
            inStack->pushFloatOntoStack(maxForce);
            inStack->insertDataIntoStackTable();
            inStack->pushStringOntoStack("velUpperLimit",0);
            inStack->pushFloatOntoStack(upperLimitVel);
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
                outStack->getStackMapFloatValue("force",forceTorque);
                outStack->getStackMapFloatValue("velocity",velocity);
            }
            App::worldContainer->interfaceStackContainer->destroyStack(outStack);
            App::worldContainer->interfaceStackContainer->destroyStack(inStack);
        }
        else
        { // there doesn't seem to be any appropriate function for joint handling in the attached child or customization scripts
            // we have the built-in control (position PID or spring-damper KC)
            // Following 9 new since 7/5/2014:
            float P=_dynamicMotorPositionControl_P;
            float I=_dynamicMotorPositionControl_I;
            float D=_dynamicMotorPositionControl_D;
            if (spring)
            {
                P=_dynamicMotorSpringControl_K/maxForce;
                I=0.0f;
                D=_dynamicMotorSpringControl_C/maxForce;
            }

            if (init)
                _dynamicMotorPIDCumulativeErrorForIntegralParameter=0.0f;

            float e=errorV;

            // Proportional part:
            float ctrl=e*P;

            // Integral part:
            if (I!=0.0f) // so that if we turn the integral part on, we don't try to catch up all the past errors!
                _dynamicMotorPIDCumulativeErrorForIntegralParameter+=e*dynStepSize; // '*dynStepSize'  was forgotten and added on 7/5/2014. The I term is corrected during load operation.
            else
                _dynamicMotorPIDCumulativeErrorForIntegralParameter=0.0f; // added on 2009/11/29
            ctrl+=_dynamicMotorPIDCumulativeErrorForIntegralParameter*I;

            // Derivative part:
            if (!init) // this condition was forgotten. Added on 7/5/2014
                ctrl+=(e-_dynamicMotorPIDLastErrorForDerivativeParameter)*D/dynStepSize; // '/dynStepSize' was forgotten and added on 7/5/2014. The D term is corrected during load operation.
            _dynamicMotorPIDLastErrorForDerivativeParameter=e;

            if (spring)
            { // "spring" mode, i.e. force modulation mode
                float vel=fabs(targetVel);
                if (ctrl<0.0f)
                    vel=-vel;

                forceTorque=fabs(ctrl)*maxForce;

                // Following 2 lines new since 7/5/2014:
                if (forceTorque>maxForce)
                    forceTorque=maxForce;

                velocity=vel;
            }
            else
            { // regular position control (i.e. built-in PID)
                // We calculate the velocity needed to reach the position in one time step:
                float vel=ctrl/dynStepSize;
                float maxVel=upperLimitVel;
                if (vel>maxVel)
                    vel=maxVel;
                if (vel<-maxVel)
                    vel=-maxVel;

                forceTorque=maxForce;
                velocity=vel;
            }
        }
    }
}

void CJoint::setPositionIsCyclic(bool isCyclic)
{
    if (isCyclic)
    {
        if (getJointType()==sim_joint_revolute_subtype)
        {
            setScrewPitch(0.0f);
            setPositionIntervalMin(-piValue_f);
            setPositionIntervalRange(piValTimes2_f);
        }
    }
    bool diff=(_positionIsCyclic!=isCyclic);
    if (diff)
    {
        _positionIsCyclic=isCyclic;
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
        CPluginContainer::ikPlugin_setJointInterval(_ikPluginCounterpartHandle,_positionIsCyclic,_jointMinPosition,_jointPositionRange);
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
    float q[4]={_sphericalTransformation(1),_sphericalTransformation(2),_sphericalTransformation(3),_sphericalTransformation(0)};
    data->appendMapObject_stringFloatArray("quaternion",q,4);
    data->appendMapObject_stringFloat("position",_jointPosition);
    C7Vector tr(getIntrinsicTransformation(true));
    float p[7]={tr.X(0),tr.X(1),tr.X(2),tr.Q(1),tr.Q(2),tr.Q(3),tr.Q(0)};
    data->appendMapObject_stringFloatArray("intrinsicPose",p,7);
    data->appendMapObject_stringBool("cyclic",_positionIsCyclic);
    data->appendMapObject_stringFloat("min",_jointMinPosition);
    data->appendMapObject_stringFloat("range",_jointPositionRange);
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
    newJoint->_sphericalTransformation=_sphericalTransformation;
    newJoint->_jointPosition=_jointPosition;
    newJoint->_ikWeight=_ikWeight;
    newJoint->_diameter=_diameter;
    newJoint->_length=_length;
    newJoint->_positionIsCyclic=_positionIsCyclic;
    newJoint->_jointPositionRange=_jointPositionRange;
    newJoint->_jointMinPosition=_jointMinPosition;
    newJoint->_maxStepSize=_maxStepSize;

    _color.copyYourselfInto(&newJoint->_color);

    newJoint->_dynamicMotorEnabled=_dynamicMotorEnabled;
    newJoint->_dynamicMotorTargetVelocity=_dynamicMotorTargetVelocity;
    newJoint->_dynamicLockModeWhenInVelocityControl=_dynamicLockModeWhenInVelocityControl;
    newJoint->_dynamicMotorUpperLimitVelocity=_dynamicMotorUpperLimitVelocity;
    newJoint->_dynamicMotorMaximumForce=_dynamicMotorMaximumForce;

    newJoint->_dynamicMotorControlLoopEnabled=_dynamicMotorControlLoopEnabled;
    newJoint->_dynamicMotorPositionControl_P=_dynamicMotorPositionControl_P;
    newJoint->_dynamicMotorPositionControl_I=_dynamicMotorPositionControl_I;
    newJoint->_dynamicMotorPositionControl_D=_dynamicMotorPositionControl_D;

    newJoint->_dynamicMotorSpringControl_K=_dynamicMotorSpringControl_K;
    newJoint->_dynamicMotorSpringControl_C=_dynamicMotorSpringControl_C;

    newJoint->_dynamicMotorPositionControl_targetPosition=_dynamicMotorPositionControl_targetPosition;
    newJoint->_dynamicMotorPositionControl_torqueModulation=_dynamicMotorPositionControl_torqueModulation;
    newJoint->_jointHasHybridFunctionality=_jointHasHybridFunctionality;
    newJoint->_dynamicMotorPositionControl_targetPosition=_dynamicMotorPositionControl_targetPosition;

    newJoint->_bulletFloatParams.assign(_bulletFloatParams.begin(),_bulletFloatParams.end());
    newJoint->_bulletIntParams.assign(_bulletIntParams.begin(),_bulletIntParams.end());

    newJoint->_odeFloatParams.assign(_odeFloatParams.begin(),_odeFloatParams.end());
    newJoint->_odeIntParams.assign(_odeIntParams.begin(),_odeIntParams.end());

    newJoint->_vortexFloatParams.assign(_vortexFloatParams.begin(),_vortexFloatParams.end());
    newJoint->_vortexIntParams.assign(_vortexIntParams.begin(),_vortexIntParams.end());

    newJoint->_newtonFloatParams.assign(_newtonFloatParams.begin(),_newtonFloatParams.end());
    newJoint->_newtonIntParams.assign(_newtonIntParams.begin(),_newtonIntParams.end());

    newJoint->_initialValuesInitialized=_initialValuesInitialized;
    newJoint->_initialPosition=_initialPosition;
    newJoint->_initialSphericalJointTransformation=_initialSphericalJointTransformation;
    newJoint->_initialTargetPosition=_initialTargetPosition;
    newJoint->_initialDynamicMotorEnabled=_initialDynamicMotorEnabled;
    newJoint->_initialDynamicMotorTargetVelocity=_initialDynamicMotorTargetVelocity;
    newJoint->_initialDynamicMotorLockModeWhenInVelocityControl=_initialDynamicMotorLockModeWhenInVelocityControl;
    newJoint->_initialDynamicMotorUpperLimitVelocity=_initialDynamicMotorUpperLimitVelocity;
    newJoint->_initialDynamicMotorMaximumForce=_initialDynamicMotorMaximumForce;
    newJoint->_initialDynamicMotorControlLoopEnabled=_initialDynamicMotorControlLoopEnabled;
    newJoint->_initialDynamicMotorPositionControl_P=_initialDynamicMotorPositionControl_P;
    newJoint->_initialDynamicMotorPositionControl_I=_initialDynamicMotorPositionControl_I;
    newJoint->_initialDynamicMotorPositionControl_D=_initialDynamicMotorPositionControl_D;
    newJoint->_initialDynamicMotorSpringControl_K=_initialDynamicMotorSpringControl_K;
    newJoint->_initialDynamicMotorSpringControl_C=_initialDynamicMotorSpringControl_C;
    newJoint->_initialJointMode=_initialJointMode;
    newJoint->_initialHybridOperation=_initialHybridOperation;

//    newJoint->_measuredJointVelocity_velocityMeasurement=_measuredJointVelocity_velocityMeasurement;
//    newJoint->_previousJointPosition_velocityMeasurement=_previousJointPosition_velocityMeasurement;

    newJoint->_maxAcceleration_DEPRECATED=_maxAcceleration_DEPRECATED;
    newJoint->_explicitHandling_DEPRECATED=_explicitHandling_DEPRECATED;
    newJoint->_unlimitedAcceleration_DEPRECATED=_unlimitedAcceleration_DEPRECATED;
    newJoint->_invertTargetVelocityAtLimits_DEPRECATED=_invertTargetVelocityAtLimits_DEPRECATED;
    newJoint->_jointPositionForMotionHandling_DEPRECATED=_jointPositionForMotionHandling_DEPRECATED;
    newJoint->_velocity_DEPRECATED=_velocity_DEPRECATED;
    newJoint->_targetVelocity_DEPRECATED=_targetVelocity_DEPRECATED;
    newJoint->_initialExplicitHandling_DEPRECATED=_initialExplicitHandling_DEPRECATED;
    newJoint->_initialVelocity_DEPRECATED=_initialVelocity_DEPRECATED;
    newJoint->_initialTargetVelocity_DEPRECATED=_initialTargetVelocity_DEPRECATED;

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
            ar << _sphericalTransformation(0) << _sphericalTransformation(1);
            ar << _sphericalTransformation(2) << _sphericalTransformation(3);
            ar.flush();

            ar.storeDataName("Va9");
            unsigned char dummy=0;
            SIM_SET_CLEAR_BIT(dummy,0,_positionIsCyclic);
            SIM_SET_CLEAR_BIT(dummy,1,_explicitHandling_DEPRECATED);
            SIM_SET_CLEAR_BIT(dummy,2,_unlimitedAcceleration_DEPRECATED);
            SIM_SET_CLEAR_BIT(dummy,3,_invertTargetVelocityAtLimits_DEPRECATED);
            SIM_SET_CLEAR_BIT(dummy,4,_dynamicMotorEnabled);
            SIM_SET_CLEAR_BIT(dummy,5,_dynamicMotorControlLoopEnabled);
            SIM_SET_CLEAR_BIT(dummy,6,_jointHasHybridFunctionality);
            SIM_SET_CLEAR_BIT(dummy,7,_dynamicMotorPositionControl_torqueModulation);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Vaa");
            dummy=0;
            SIM_SET_CLEAR_BIT(dummy,1,_dynamicLockModeWhenInVelocityControl);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Cl1");
            ar.setCountingMode();
            _color.serialize(ar,0);
            if (ar.setWritingMode())
                _color.serialize(ar,0);

            ar.storeDataName("Pmr");
            ar << _jointMinPosition << _jointPositionRange;
            ar.flush();

            ar.storeDataName("Prt");
            ar << _jointPosition;
            ar.flush();

            ar.storeDataName("Mss");
            ar << _maxStepSize;
            ar.flush();

            ar.storeDataName("Arg");
            ar << _length << _diameter;
            ar.flush();

            ar.storeDataName("Ikw");
            ar << _ikWeight;
            ar.flush();

            ar.storeDataName("Jmd");
            ar << _jointMode;
            ar.flush();

            ar.storeDataName("Jdt");
            ar << _dependencyMasterJointHandle << _dependencyJointMult << _dependencyJointOffset;
            ar.flush();

            ar.storeDataName("Jm2");
            ar << _maxAcceleration_DEPRECATED << _velocity_DEPRECATED << _targetVelocity_DEPRECATED;
            ar.flush();

            ar.storeDataName("Dmp");
            ar << _dynamicMotorTargetVelocity << _dynamicMotorMaximumForce;
            ar.flush();

            // Following for backward compatibility (7/5/2014):
            // Keep this before "Dp2"!
            ar.storeDataName("Dpc");
            ar << _dynamicMotorPositionControl_P << (_dynamicMotorPositionControl_I*0.005f) << (_dynamicMotorPositionControl_D/0.005f);
            ar.flush();

            ar.storeDataName("Dp2");
            ar << _dynamicMotorPositionControl_P << _dynamicMotorPositionControl_I << _dynamicMotorPositionControl_D;
            ar.flush();

            ar.storeDataName("Spp");
            ar << _dynamicMotorSpringControl_K << _dynamicMotorSpringControl_C;
            ar.flush();

            ar.storeDataName("Dtp");
            ar << _dynamicMotorPositionControl_targetPosition;
            ar.flush();

            ar.storeDataName("Od1"); // keep this for file backw. compat. (09/03/2016)
            // ar << _odeStopERP << _odeStopCFM << _odeBounce << _odeFudgeFactor << _odeNormalCFM;
            ar << _odeFloatParams[simi_ode_joint_stoperp] << _odeFloatParams[simi_ode_joint_stopcfm] << _odeFloatParams[simi_ode_joint_bounce] << _odeFloatParams[simi_ode_joint_fudgefactor] << _odeFloatParams[simi_ode_joint_normalcfm];
            ar.flush();

            ar.storeDataName("Bt1"); // keep this for file backw. compat. (09/03/2016)
            // ar << _bulletStopERP << _bulletStopCFM << _bulletNormalCFM;
            ar << _bulletFloatParams[simi_bullet_joint_stoperp] << _bulletFloatParams[simi_bullet_joint_stopcfm] << _bulletFloatParams[simi_bullet_joint_normalcfm];
            ar.flush();

            ar.storeDataName("BtN"); // Bullet params, keep this after "Bt1"
            ar << int(_bulletFloatParams.size()) << int(_bulletIntParams.size());
            for (int i=0;i<int(_bulletFloatParams.size());i++)
                ar << _bulletFloatParams[i];
            for (int i=0;i<int(_bulletIntParams.size());i++)
                ar << _bulletIntParams[i];
            ar.flush();

            ar.storeDataName("OdN"); // ODE params, keep this after "Od1"
            ar << int(_odeFloatParams.size()) << int(_odeIntParams.size());
            for (int i=0;i<int(_odeFloatParams.size());i++)
                ar << _odeFloatParams[i];
            for (int i=0;i<int(_odeIntParams.size());i++)
                ar << _odeIntParams[i];
            ar.flush();

            ar.storeDataName("Vo2"); // vortex params:
            ar << int(_vortexFloatParams.size()) << int(_vortexIntParams.size());
            for (int i=0;i<int(_vortexFloatParams.size());i++)
                ar << _vortexFloatParams[i];
            for (int i=0;i<int(_vortexIntParams.size());i++)
                ar << _vortexIntParams[i];
            ar.flush();

            ar.storeDataName("Nw1"); // newton params:
            ar << int(_newtonFloatParams.size()) << int(_newtonIntParams.size());
            for (int i=0;i<int(_newtonFloatParams.size());i++)
                ar << _newtonFloatParams[i];
            for (int i=0;i<int(_newtonIntParams.size());i++)
                ar << _newtonIntParams[i];
            ar.flush();

            ar.storeDataName("Ulv");
            ar << _dynamicMotorUpperLimitVelocity;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            bool dynamicUpperVelocityLimitPresent=false; // for backward compatibility (2010/11/13)
            bool kAndCSpringParameterPresent=false; // for backward compatibility (7/5/2014)
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
                        ar >> _jointPosition;
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
                        ar >> _sphericalTransformation(0) >> _sphericalTransformation(1);
                        ar >> _sphericalTransformation(2) >> _sphericalTransformation(3);
                        _sphericalTransformation.normalize();
                    }
                    if (theName.compare("Mss")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _maxStepSize;
                    }
                    if (theName.compare("Ikw")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _ikWeight;
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
                        ar >> _jointMinPosition >> _jointPositionRange;
                    }
                    if (theName.compare("Va9")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _positionIsCyclic=SIM_IS_BIT_SET(dummy,0);
                        _explicitHandling_DEPRECATED=SIM_IS_BIT_SET(dummy,1);
                        _unlimitedAcceleration_DEPRECATED=SIM_IS_BIT_SET(dummy,2);
                        _invertTargetVelocityAtLimits_DEPRECATED=SIM_IS_BIT_SET(dummy,3);
                        _dynamicMotorEnabled=SIM_IS_BIT_SET(dummy,4);
                        _dynamicMotorControlLoopEnabled=SIM_IS_BIT_SET(dummy,5);
                        _jointHasHybridFunctionality=SIM_IS_BIT_SET(dummy,6);
                        _dynamicMotorPositionControl_torqueModulation=SIM_IS_BIT_SET(dummy,7);
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
                        _dynamicLockModeWhenInVelocityControl=SIM_IS_BIT_SET(dummy,1);
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
                            if ( (_jointMode!=sim_jointmode_passive)&&(_jointMode!=sim_jointmode_dependent)&&(_jointMode!=sim_jointmode_force) )
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
                        ar >> _maxAcceleration_DEPRECATED >> _velocity_DEPRECATED >> _targetVelocity_DEPRECATED;
                    }
                    if (theName.compare("Dmp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynamicMotorTargetVelocity >> _dynamicMotorMaximumForce;
                    }
                    if (theName.compare("Dpc")==0)
                    { // keep for backward compatibility (7/5/2014)
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynamicMotorPositionControl_P >> _dynamicMotorPositionControl_I >> _dynamicMotorPositionControl_D;
                        _dynamicMotorPositionControl_I/=0.005f;
                        _dynamicMotorPositionControl_D*=0.005f;
                    }
                    if (theName.compare("Dp2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynamicMotorPositionControl_P >> _dynamicMotorPositionControl_I >> _dynamicMotorPositionControl_D;
                    }

                    if (theName.compare("Spp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynamicMotorSpringControl_K >> _dynamicMotorSpringControl_C;
                        kAndCSpringParameterPresent=true;
                    }

                    if (theName.compare("Dtp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynamicMotorPositionControl_targetPosition;
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
                    if (theName.compare("Ulv")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynamicMotorUpperLimitVelocity;
                        dynamicUpperVelocityLimitPresent=true;
                    }
                    if (theName.compare("Cco")==0)
                    { // Keep this for backward compatibility (6/8/2014)
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _jointCallbackCallOrder_backwardCompatibility;
                    }

                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            if (!dynamicUpperVelocityLimitPresent)
            { // for backward compatibility (2010/11/13):
                if (_jointType==sim_joint_revolute_subtype)
                    _dynamicMotorUpperLimitVelocity=360.0f*degToRad_f;
                if (_jointType==sim_joint_prismatic_subtype)
                    _dynamicMotorUpperLimitVelocity=10.0f;
                if (_jointType==sim_joint_spherical_subtype)
                    _dynamicMotorUpperLimitVelocity=0.0; // no meaning here
            }

            if (!kAndCSpringParameterPresent)
            { // for backward compatibility (7/5/2014):
                if (_dynamicMotorEnabled&&_dynamicMotorControlLoopEnabled&&_dynamicMotorPositionControl_torqueModulation)
                { // we have a joint that behaves as a spring. We need to compute the corresponding K and C parameters, and adjust the max. force/torque (since that was not limited before):
                    _dynamicMotorSpringControl_K=_dynamicMotorMaximumForce*_dynamicMotorPositionControl_P;
                    _dynamicMotorSpringControl_C=_dynamicMotorMaximumForce*_dynamicMotorPositionControl_D;
                    float maxTolerablePorDParam=1.0f;
                    if (_jointType==sim_joint_revolute_subtype)
                        maxTolerablePorDParam=1.0f/piValTimes2_f;
                    float maxPorD=std::max<float>(fabs(_dynamicMotorPositionControl_P),fabs(_dynamicMotorPositionControl_D));
                    if (maxPorD>maxTolerablePorDParam)
                    { // we shift the limit up
                        float corr=maxTolerablePorDParam/maxPorD;
                        _dynamicMotorPositionControl_P*=corr;
                        _dynamicMotorPositionControl_I*=corr;
                        _dynamicMotorPositionControl_D*=corr;
                        _dynamicMotorMaximumForce/=corr;
                    }
                }
            }

            if (ar.getSerializationVersionThatWroteThisFile()<17)
                CTTUtil::scaleColorUp_(_color.getColorsPtr());
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
            ar.xmlAddNode_comment(" 'mode' tag: can be 'passive', 'dependent' or 'force' ",exhaustiveXml);
            ar.xmlAddNode_enum("mode",_jointMode,sim_jointmode_passive,"passive",sim_jointmode_ik_deprecated,"ik",sim_jointmode_dependent,"dependent",sim_jointmode_force,"force");

            ar.xmlAddNode_float("minPosition",_jointMinPosition*mult);
            ar.xmlAddNode_float("range",_jointPositionRange*mult);
            ar.xmlAddNode_float("position",_jointPosition*mult);

            ar.xmlAddNode_float("screwPitch",_screwPitch);

            if (exhaustiveXml)
                ar.xmlAddNode_floats("sphericalQuaternion",_sphericalTransformation.data,4);
            else
            {
                C3Vector euler(_sphericalTransformation.getEulerAngles());
                for (size_t l=0;l<3;l++)
                    euler(l)*=180.0f/piValue_f;
                ar.xmlAddNode_floats("sphericalEuler",euler.data,3);
            }

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("cyclic",_positionIsCyclic);
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
            ar.xmlAddNode_float("maxStepSize",_maxStepSize*mult);
            ar.xmlAddNode_float("weight",_ikWeight);
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
            ar.xmlAddNode_float("maxForce",_dynamicMotorMaximumForce);
            ar.xmlAddNode_float("upperVelocityLimit",_dynamicMotorUpperLimitVelocity*mult);
            ar.xmlAddNode_float("targetPosition",_dynamicMotorPositionControl_targetPosition*mult);
            ar.xmlAddNode_float("targetVelocity",_dynamicMotorTargetVelocity*mult);
            ar.xmlAddNode_3float("pidValues",_dynamicMotorPositionControl_P,_dynamicMotorPositionControl_I,_dynamicMotorPositionControl_D);
            ar.xmlAddNode_2float("kcValues",_dynamicMotorSpringControl_K,_dynamicMotorSpringControl_C);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("motorEnabled",_dynamicMotorEnabled);
            ar.xmlAddNode_bool("controlLoopEnabled",_dynamicMotorControlLoopEnabled);
            ar.xmlAddNode_bool("springMode",_dynamicMotorPositionControl_torqueModulation);
            ar.xmlAddNode_bool("lockInVelocityControl",_dynamicLockModeWhenInVelocityControl);
            ar.xmlPopNode();

            ar.xmlPushNewNode("engines");
            ar.xmlPushNewNode("bullet");
            ar.xmlAddNode_float("stoperp",getEngineFloatParam(sim_bullet_joint_stoperp,nullptr));
            ar.xmlAddNode_float("stopcfm",getEngineFloatParam(sim_bullet_joint_stopcfm,nullptr));
            ar.xmlAddNode_float("normalcfm",getEngineFloatParam(sim_bullet_joint_normalcfm,nullptr));
            ar.xmlPopNode();

            ar.xmlPushNewNode("ode");
            ar.xmlAddNode_float("stoperp",getEngineFloatParam(sim_ode_joint_stoperp,nullptr));
            ar.xmlAddNode_float("stopcfm",getEngineFloatParam(sim_ode_joint_stopcfm,nullptr));
            ar.xmlAddNode_float("bounce",getEngineFloatParam(sim_ode_joint_bounce,nullptr));
            ar.xmlAddNode_float("fudgefactor",getEngineFloatParam(sim_ode_joint_fudgefactor,nullptr));
            ar.xmlAddNode_float("normalcfm",getEngineFloatParam(sim_ode_joint_normalcfm,nullptr));
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

            //ar.xmlAddNode_int("bitcoded",getEngineIntParam(sim_vortex_joint_bitcoded,nullptr));
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

            ar.xmlAddNode_int("objectid",getEngineIntParam(sim_newton_joint_objectid,nullptr));
            ar.xmlAddNode_int("dependentobjectid",getEngineIntParam(sim_newton_joint_dependentobjectid,nullptr));
            ar.xmlPopNode();

            ar.xmlPopNode();

            ar.xmlPopNode();
        }
        else
        {
            float mult=1.0f;
            if (ar.xmlGetNode_enum("type",_jointType,exhaustiveXml,"revolute",sim_joint_revolute_subtype,"prismatic",sim_joint_prismatic_subtype,"spherical",sim_joint_spherical_subtype))
            {
                if (_jointType==sim_joint_revolute_subtype)
                    mult=piValue_f/180.0f;
            }

            ar.xmlGetNode_enum("mode",_jointMode,exhaustiveXml,"passive",sim_jointmode_passive,"ik",sim_jointmode_ik_deprecated,"dependent",sim_jointmode_dependent,"force",sim_jointmode_force);

            if (ar.xmlGetNode_float("minPosition",_jointMinPosition,exhaustiveXml))
            {
                _jointMinPosition*=mult;

                if (ar.xmlGetNode_float("range",_jointPositionRange,exhaustiveXml))
                    _jointPositionRange*=mult;
            }

            if (ar.xmlGetNode_float("position",_jointPosition,exhaustiveXml))
                _jointPosition*=mult;

            ar.xmlGetNode_float("screwPitch",_screwPitch,exhaustiveXml);

            if (exhaustiveXml)
                ar.xmlGetNode_floats("sphericalQuaternion",_sphericalTransformation.data,4);
            else
            {
                C3Vector euler;
                if (ar.xmlGetNode_floats("sphericalEuler",euler.data,3,exhaustiveXml))
                {
                    euler(0)*=piValue_f/180.0f;
                    euler(1)*=piValue_f/180.0f;
                    euler(2)*=piValue_f/180.0f;
                    _sphericalTransformation.setEulerAngles(euler);
                }
            }

            if (ar.xmlPushChildNode("switches",exhaustiveXml))
            {
                ar.xmlGetNode_bool("cyclic",_positionIsCyclic,exhaustiveXml);
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
                if (ar.xmlGetNode_float("maxStepSize",_maxStepSize,exhaustiveXml))
                    _maxStepSize*=mult;
                ar.xmlGetNode_float("weight",_ikWeight,exhaustiveXml);
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
                ar.xmlGetNode_float("maxForce",_dynamicMotorMaximumForce,exhaustiveXml);
                if (ar.xmlGetNode_float("upperVelocityLimit",_dynamicMotorUpperLimitVelocity,exhaustiveXml))
                    _dynamicMotorUpperLimitVelocity*=mult;
                if (ar.xmlGetNode_float("targetPosition",_dynamicMotorPositionControl_targetPosition,exhaustiveXml))
                    _dynamicMotorPositionControl_targetPosition*=mult;
                if (ar.xmlGetNode_float("targetVelocity",_dynamicMotorTargetVelocity,exhaustiveXml))
                    _dynamicMotorTargetVelocity*=mult;
                ar.xmlGetNode_3float("pidValues",_dynamicMotorPositionControl_P,_dynamicMotorPositionControl_I,_dynamicMotorPositionControl_D,exhaustiveXml);
                ar.xmlGetNode_2float("kcValues",_dynamicMotorSpringControl_K,_dynamicMotorSpringControl_C,exhaustiveXml);

                if (ar.xmlPushChildNode("switches",exhaustiveXml))
                {
                    ar.xmlGetNode_bool("motorEnabled",_dynamicMotorEnabled,exhaustiveXml);
                    ar.xmlGetNode_bool("controlLoopEnabled",_dynamicMotorControlLoopEnabled,exhaustiveXml);
                    ar.xmlGetNode_bool("springMode",_dynamicMotorPositionControl_torqueModulation,exhaustiveXml);
                    ar.xmlGetNode_bool("lockInVelocityControl",_dynamicLockModeWhenInVelocityControl,exhaustiveXml);
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("engines",exhaustiveXml))
                {
                    float v;
                    int vi;
                    bool vb;
                    if (ar.xmlPushChildNode("bullet",exhaustiveXml))
                    {
                        if (ar.xmlGetNode_float("stoperp",v,exhaustiveXml)) setEngineFloatParam(sim_bullet_joint_stoperp,v);
                        if (ar.xmlGetNode_float("stopcfm",v,exhaustiveXml)) setEngineFloatParam(sim_bullet_joint_stopcfm,v);
                        if (ar.xmlGetNode_float("normalcfm",v,exhaustiveXml)) setEngineFloatParam(sim_bullet_joint_normalcfm,v);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("ode",exhaustiveXml))
                    {
                        if (ar.xmlGetNode_float("stoperp",v,exhaustiveXml)) setEngineFloatParam(sim_ode_joint_stoperp,v);
                        if (ar.xmlGetNode_float("stopcfm",v,exhaustiveXml)) setEngineFloatParam(sim_ode_joint_stopcfm,v);
                        if (ar.xmlGetNode_float("bounce",v,exhaustiveXml)) setEngineFloatParam(sim_ode_joint_bounce,v);
                        if (ar.xmlGetNode_float("fudgefactor",v,exhaustiveXml)) setEngineFloatParam(sim_ode_joint_fudgefactor,v);
                        if (ar.xmlGetNode_float("normalcfm",v,exhaustiveXml)) setEngineFloatParam(sim_ode_joint_normalcfm,v);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("vortex",exhaustiveXml))
                    {
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
                        if (ar.xmlGetNode_float("dependencyfactor",v,exhaustiveXml)) setEngineFloatParam(sim_newton_joint_dependencyfactor,v);
                        if (ar.xmlGetNode_float("dependencyoffset",v,exhaustiveXml)) setEngineFloatParam(sim_newton_joint_dependencyoffset,v);

                        if (ar.xmlGetNode_int("objectid",vi,exhaustiveXml)) setEngineIntParam(sim_newton_joint_objectid,vi);
                        if (ar.xmlGetNode_int("dependentobjectid",vi,exhaustiveXml)) setEngineIntParam(sim_newton_joint_dependentobjectid,vi);
                        ar.xmlPopNode();
                    }
                    ar.xmlPopNode();
                }

                ar.xmlPopNode();
            }
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
}

void CJoint::setJointMode(int theMode)
{
    bool retVal=setJointMode_noDynMotorTargetPosCorrection(theMode);
    if ( retVal&&(theMode==sim_jointmode_force)&&_dynamicMotorControlLoopEnabled )
    { // Make sure the target position is the same here (otherwise big jump)
        setDynamicMotorPositionControlTargetPosition(getPosition());
    }
}

bool CJoint::setJointMode_noDynMotorTargetPosCorrection(int theMode)
{
    int md;
    if (theMode==sim_jointmode_passive)
    {
        App::currentWorld->sceneObjects->actualizeObjectInformation();
        md=theMode;
    }
    if (theMode==sim_jointmode_motion_deprecated)
    {
        if (_jointMode!=theMode)
        {
            _velocity_DEPRECATED=0.0f;
            _targetVelocity_DEPRECATED=0.0f;
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
    if (theMode==sim_jointmode_force)
    {
        setHybridFunctionality(false);
        setScrewPitch(0.0f);
// REMOVED FOLLOWING ON 24/7/2015: causes problem when switching modes. The physics engine plugin will now not set limits if the range>=360
//      if (_jointType==sim_joint_revolute_subtype)
//          _jointPositionRange=tt::getLimitedFloat(0.0f,piValTimes2_f,_jointPositionRange); // new since 18/11/2012 (was forgotten)
        if (_jointType==sim_joint_spherical_subtype)
            setPositionIntervalRange(piValue_f);

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
    if (_jointPositionRange<piValue_f*0.99f)
    {
        C3X3Matrix theTr(transf);
        C3Vector zReset(0.0f,0.0f,1.0f);
        float angle=zReset.getAngle(theTr.axis[2]);
        if (angle>_jointPositionRange)
        { // We have to limit the movement:
            C3Vector rotAxis((theTr.axis[2]^zReset).getNormalized());
            C4Vector rot(angle-_jointPositionRange,rotAxis);
            transf=rot*transf;
        }
    }
    bool diff=(_sphericalTransformation!=transf);
    if (diff)
    {
        _sphericalTransformation=transf;
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

void CJoint::setMaxStepSize(float stepS)
{
    if (_jointType==sim_joint_revolute_subtype)
        tt::limitValue(0.01f*degToRad_f,100000.0f,stepS); // high number for screws!
    if (_jointType==sim_joint_prismatic_subtype)
        tt::limitValue(0.0001f,1000.0f,stepS);
    if (_jointType==sim_joint_spherical_subtype)
        tt::limitValue(0.01f*degToRad_f,piValue_f,stepS);
    bool diff=(_maxStepSize!=stepS);
    if (diff)
    {
        _maxStepSize=stepS;
        if (getObjectCanSync())
            _setMaxStepSize_sendOldIk(stepS);
    }
}

void CJoint::_setMaxStepSize_sendOldIk(float stepS) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setJointMaxStepSize(_ikPluginCounterpartHandle,_maxStepSize);
}

void CJoint::setIkWeight(float newWeight)
{
    newWeight=tt::getLimitedFloat(-1.0f,1.0f,newWeight);
    bool diff=(_ikWeight!=newWeight);
    if (diff)
    {
        _ikWeight=newWeight;
        if (getObjectCanSync())
            _setIkWeight_sendOldIk(newWeight);
    }
}

void CJoint::_setIkWeight_sendOldIk(float newWeight) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setJointIkWeight(_ikPluginCounterpartHandle,_ikWeight);
}

void CJoint::setPosition(float pos,bool setDirect)
{
    if (!setDirect)
    {
        if (_positionIsCyclic)
            pos=tt::getNormalizedAngle(pos);
        else
        {
            if (pos>(getPositionIntervalMin()+getPositionIntervalRange()))
                pos=getPositionIntervalMin()+getPositionIntervalRange();
            if (pos<getPositionIntervalMin())
                pos=getPositionIntervalMin();
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
    bool diff=(_jointPosition!=pos);
    if (diff)
    {
        _jointPosition=pos;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="position";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,false,cmd,true);
            data->appendMapObject_stringFloat(cmd,_jointPosition);

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
    if (_newtonIntParams[1]==object->getObjectHandle()) // that's the Vortex dependency joint
    {
        std::vector<int> ip;
        getNewtonIntParams(ip);
        ip[1]=-1;
        setNewtonIntParams(ip);
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
        _setPositionIntervalMin_sendOldIk(_jointMinPosition);
        _setPositionIntervalRange_sendOldIk(_jointPositionRange);
        _setPositionIsCyclic_sendOldIk(_positionIsCyclic);
        _setScrewPitch_sendOldIk(_screwPitch);
        _setIkWeight_sendOldIk(_ikWeight);
        _setMaxStepSize_sendOldIk(_maxStepSize);
        _setPosition_sendOldIk(_jointPosition);
        _setSphericalTransformation_sendOldIk(_sphericalTransformation);
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

bool CJoint::getEnableDynamicMotor() const
{
    return(_dynamicMotorEnabled);
}

float CJoint::getDynamicMotorTargetVelocity() const
{
    return(_dynamicMotorTargetVelocity);
}

bool CJoint::getDynamicMotorLockModeWhenInVelocityControl() const
{
    return(_dynamicLockModeWhenInVelocityControl);
}

float CJoint::getDynamicMotorUpperLimitVelocity() const
{
    return(_dynamicMotorUpperLimitVelocity);
}

float CJoint::getDynamicMotorMaximumForce(bool signedValue) const
{
    float retVal=_dynamicMotorMaximumForce;
    if (signedValue)
    {
        if (_dynamicMotorTargetVelocity<0.0f)
            retVal=-retVal;
    }
    return(retVal);
}

bool CJoint::getEnableDynamicMotorControlLoop() const
{
    return(_dynamicMotorControlLoopEnabled);
}

bool CJoint::getEnableTorqueModulation() const
{
    return(_dynamicMotorPositionControl_torqueModulation);
}

bool CJoint::getHybridFunctionality() const
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

int CJoint::getVortexDependentJointId() const
{
    return(_vortexIntParams[5]);
}

int CJoint::getNewtonDependentJointId() const
{
    return(_newtonIntParams[1]);
}

float CJoint::getPosition() const
{
    return(_jointPosition);
}

float CJoint::getDynamicMotorPositionControlTargetPosition() const
{
    return(_dynamicMotorPositionControl_targetPosition);
}

C7Vector CJoint::getIntrinsicTransformation(bool includeDynErrorComponent) const
{
    C7Vector jointTr;
    if (getJointType()==sim_joint_revolute_subtype)
    {
        jointTr.Q.setAngleAndAxis(_jointPosition,C3Vector(0.0f,0.0f,1.0f));
        jointTr.X(0)=0.0f;
        jointTr.X(1)=0.0f;
        jointTr.X(2)=_jointPosition*getScrewPitch();
    }
    if (getJointType()==sim_joint_prismatic_subtype)
    {
        jointTr.Q.setIdentity();
        jointTr.X(0)=0.0f;
        jointTr.X(1)=0.0f;
        jointTr.X(2)=_jointPosition;
    }
    if (getJointType()==sim_joint_spherical_subtype)
    {
        jointTr.Q=_sphericalTransformation;
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

void CJoint::getDynamicMotorPositionControlParameters(float& p_param,float& i_param,float& d_param) const
{
    p_param=_dynamicMotorPositionControl_P;
    i_param=_dynamicMotorPositionControl_I;
    d_param=_dynamicMotorPositionControl_D;
}

void CJoint::getDynamicMotorSpringControlParameters(float& k_param,float& c_param) const
{
    k_param=_dynamicMotorSpringControl_K;
    c_param=_dynamicMotorSpringControl_C;
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
    return(_sphericalTransformation);
}

bool CJoint::getPositionIsCyclic() const
{
    if (_jointType==sim_joint_prismatic_subtype)
        return(false);
    return(_positionIsCyclic);
}

float CJoint::getPositionIntervalMin() const
{
    return(_jointMinPosition);
}

float CJoint::getPositionIntervalRange() const
{
    return(_jointPositionRange);
}

float CJoint::getIKWeight() const
{
    return(_ikWeight);
}

float CJoint::getMaxStepSize() const
{
    return(_maxStepSize);
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

void CJoint::setEnableDynamicMotor(bool e)
{
    bool diff=(_dynamicMotorEnabled!=e);
    if (diff)
        _dynamicMotorEnabled=e;
}

void CJoint::setEnableDynamicMotorControlLoop(bool p)
{
    bool diff=(_dynamicMotorControlLoopEnabled!=p);
    if (diff)
        _dynamicMotorControlLoopEnabled=p;
}

void CJoint::setEnableTorqueModulation(bool p)
{
    bool diff=(_dynamicMotorPositionControl_torqueModulation!=p);
    if (diff)
        _dynamicMotorPositionControl_torqueModulation=p;
}

void CJoint::setDynamicMotorLockModeWhenInVelocityControl(bool e)
{
    bool diff=(_dynamicLockModeWhenInVelocityControl!=e);
    if (diff)
        _dynamicLockModeWhenInVelocityControl=e;
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
            data->appendMapObject_stringFloat(cmd,_jointPosition);
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
