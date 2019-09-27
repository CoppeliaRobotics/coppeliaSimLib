
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "jointObject.h"
#include "tt.h"
#include "v_rep_internal.h"
#include "gV.h"
#include "linMotionRoutines.h"
#include "v_repStrings.h"
#include "ttUtil.h"
#include "easyLock.h"
#include "app.h"
#include "jointRendering.h"

CJoint::CJoint()
{ // use only during serialization!!
    commonInit();
    // Following is not needed, but just in case!
    _jointType=sim_joint_revolute_subtype;
    _positionIsCyclic=true;
    _jointPositionRange=piValTimes2_f;
    _jointMinPosition=-piValue_f;
    _maxStepSize=10.0f*degToRad_f;
    _maxAcceleration_DEPRECATED=60.0f*degToRad_f;
}

CJoint::CJoint(int jointType)
{
    _jointType=jointType;
    commonInit();
    if (jointType==sim_joint_revolute_subtype)
    {
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
        _jointMode=sim_jointmode_force;
        _positionIsCyclic=true;
        _jointPositionRange=piValue_f;
        _jointMinPosition=0.0f; // no meaning here
        _maxStepSize=10.0f*degToRad_f;
        _dynamicMotorMaximumForce=0.0f; // no meaning here!
        _dynamicMotorUpperLimitVelocity=0.0f; // no meaning here!
        _maxAcceleration_DEPRECATED=60.0f*degToRad_f;
    }
}

void CJoint::commonInit()
{ // set the joint type directly after that!
    _objectType=sim_object_joint_type;
    _screwPitch=0.0f;
    _sphericalTransformation.setIdentity();
    _jointPosition=0.0f;
    _initialValuesInitialized=false;
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

    _jointMode=sim_jointmode_ik;
    _dependencyJointID=-1;
    _dependencyJointCoeff=1.0f;
    _dependencyJointOffset=0.0f;

    // Dynamic values:
    _dynamicMotorEnabled=false;
    _dynamicMotorTargetVelocity=0.0f;
    _dynamicLockModeWhenInVelocityControl=false;
    _dynamicMotorMaximumForce=1000.0f; // This value has to be adjusted according to the joint type
    _dynamicSecondPartIsValid=false;
    _dynamicSecondPartLocalTransform.setIdentity();

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
    _vortexIntParams.push_back(_objectHandle); // simi_vortex_joint_objectid
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
    _newtonIntParams.push_back(_objectHandle); // simi_newton_joint_objectid. The ID is redefined in each session
    _newtonIntParams.push_back(-1); // simi_newton_joint_dependentobjectid
    // NEWTON_JOINT_INT_PARAM_CNT_CURRENT=2
    // ----------------------------------------------------

    _ikWeight=1.0f;
    _jointCallbackCallOrder_backwardCompatibility=0;
    _diameter=0.02f;
    _length=0.15f;
    colorPart1.setDefaultValues();
    colorPart1.setColor(1.0f,0.3f,0.1f,sim_colorcomponent_ambient_diffuse);
    colorPart2.setDefaultValues();
    colorPart2.setColor(0.1f,0.1f,0.9f,sim_colorcomponent_ambient_diffuse);
    layer=JOINT_LAYER;
    _localObjectSpecialProperty=0;

    _objectName=IDSOGL_JOINT;
    if (_jointType==sim_joint_revolute_subtype)
        _objectName=IDSOGL_REVOLUTE_JOINT;
    if (_jointType==sim_joint_prismatic_subtype)
        _objectName=IDSOGL_PRISMATIC_JOINT;
    if (_jointType==sim_joint_spherical_subtype)
        _objectName=IDSOGL_SPHERICAL_JOINT;
    _objectAltName=tt::getObjectAltNameFromObjectName(_objectName);

    _explicitHandling_DEPRECATED=false;
    _unlimitedAcceleration_DEPRECATED=false;
    _invertTargetVelocityAtLimits_DEPRECATED=true;
}

CJoint::~CJoint()
{

}

void CJoint::setHybridFunctionality(bool h)
{
    if ( (_jointType==sim_joint_spherical_subtype)||(_jointMode==sim_jointmode_force) )
        return;
    _jointHasHybridFunctionality=h;
    if (h)
    {
        _dynamicMotorEnabled=true;
        _dynamicMotorControlLoopEnabled=true;
        if (_jointType==sim_joint_revolute_subtype)
        {
            _screwPitch=0.0f;
// REMOVED FOLLOWING ON 24/7/2015: causes problem when switching modes. The physics engine plugin will now not set limits if the range>=360
//          _jointPositionRange=tt::getLimitedFloat(0.0f,piValTimes2_f,_jointPositionRange);
//          setPosition(getPosition()); // to make sure it is within range (will also get/set velocity)
        }
    }
}

void CJoint::setDynamicSecondPartIsValid(bool v)
{
    _dynamicSecondPartIsValid=v;
}

bool CJoint::getDynamicSecondPartIsValid() const
{
    return(_dynamicSecondPartIsValid);
}

void CJoint::setDynamicSecondPartLocalTransform(const C7Vector& tr)
{
    _dynamicSecondPartLocalTransform=tr;
}

C7Vector CJoint::getDynamicSecondPartLocalTransform() const
{
    return(_dynamicSecondPartLocalTransform);
}

void CJoint::getDynamicJointErrors(float& linear,float& angular)
{
    linear=0.0f;
    angular=0.0f;
    if (!_dynamicSecondPartIsValid)
        return;
    if (_jointType==sim_joint_revolute_subtype)
    {
        linear=_dynamicSecondPartLocalTransform.X.getLength();
        angular=C3Vector::unitZVector.getAngle(_dynamicSecondPartLocalTransform.Q.getMatrix().axis[2]);
    }
    if (_jointType==sim_joint_prismatic_subtype)
    {
        float l=_dynamicSecondPartLocalTransform.X*C3Vector::unitZVector;
        linear=sqrt(_dynamicSecondPartLocalTransform.X*_dynamicSecondPartLocalTransform.X-l*l);
        C4Vector idQuat;
        idQuat.setIdentity();
        angular=_dynamicSecondPartLocalTransform.Q.getAngleBetweenQuaternions(idQuat);
    }
    if (_jointType==sim_joint_spherical_subtype)
        linear=_dynamicSecondPartLocalTransform.X.getLength();
}

void CJoint::getDynamicJointErrorsFull(C3Vector& linear,C3Vector& angular)
{
    linear.clear();
    angular.clear();
    if (!_dynamicSecondPartIsValid)
        return;
    if (_jointType==sim_joint_revolute_subtype)
    {
        linear=_dynamicSecondPartLocalTransform.X;
        angular=_dynamicSecondPartLocalTransform.Q.getEulerAngles();
        angular(2)=0.0f;
    }
    if (_jointType==sim_joint_prismatic_subtype)
    {
        linear=_dynamicSecondPartLocalTransform.X;
        linear(2)=0.0f;
        angular=_dynamicSecondPartLocalTransform.Q.getEulerAngles();
    }
    if (_jointType==sim_joint_spherical_subtype)
        linear=_dynamicSecondPartLocalTransform.X;
}

float CJoint::getEngineFloatParam(int what,bool* ok)
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
    return(0.0);
}

int CJoint::getEngineIntParam(int what,bool* ok)
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

bool CJoint::getEngineBoolParam(int what,bool* ok)
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

void CJoint::getBulletFloatParams(std::vector<float>& p)
{
    p.assign(_bulletFloatParams.begin(),_bulletFloatParams.end());
}

void CJoint::setBulletFloatParams(const std::vector<float>& p)
{
    _bulletFloatParams.assign(p.begin(),p.end());
    tt::limitValue(0.0f,1.0f,_bulletFloatParams[simi_bullet_joint_stoperp]); // stop ERP
    tt::limitValue(0.0f,100.0f,_bulletFloatParams[simi_bullet_joint_stopcfm]); // stop CFM
    tt::limitValue(0.0f,100.0f,_bulletFloatParams[simi_bullet_joint_normalcfm]); // normal CFM
}

void CJoint::getBulletIntParams(std::vector<int>& p)
{
    p.assign(_bulletIntParams.begin(),_bulletIntParams.end());
}

void CJoint::setBulletIntParams(const std::vector<int>& p)
{
    _bulletIntParams.assign(p.begin(),p.end());
}


void CJoint::getOdeFloatParams(std::vector<float>& p)
{
    p.assign(_odeFloatParams.begin(),_odeFloatParams.end());
}

void CJoint::setOdeFloatParams(const std::vector<float>& p)
{
    _odeFloatParams.assign(p.begin(),p.end());
    tt::limitValue(0.0f,1.0f,_odeFloatParams[simi_ode_joint_stoperp]); // stop ERP
    tt::limitValue(0.0f,100.0f,_odeFloatParams[simi_ode_joint_stopcfm]); // stop CFM
    tt::limitValue(0.0f,100.0f,_odeFloatParams[simi_ode_joint_bounce]); // bounce
    tt::limitValue(0.0f,10.0f,_odeFloatParams[simi_ode_joint_fudgefactor]); // fudge factor
    tt::limitValue(0.0f,100.0f,_odeFloatParams[simi_ode_joint_normalcfm]); // normal CFM
}

void CJoint::getOdeIntParams(std::vector<int>& p)
{
    p.assign(_odeIntParams.begin(),_odeIntParams.end());
}

void CJoint::setOdeIntParams(const std::vector<int>& p)
{
    _odeIntParams.assign(p.begin(),p.end());
}


int CJoint::getVortexDependentJointId()
{
    return(_vortexIntParams[5]);
}

void CJoint::getVortexFloatParams(std::vector<float>& p)
{
    p.assign(_vortexFloatParams.begin(),_vortexFloatParams.end());
}

void CJoint::setVortexFloatParams(const std::vector<float>& p)
{
    _vortexFloatParams.assign(p.begin(),p.end());
    // Forbid zero stiffness for relaxation axes:
    if (_vortexFloatParams[simi_vortex_joint_p0stiffness]==0.0f)
        _vortexFloatParams[simi_vortex_joint_p0stiffness]=1e-35f;
    if (_vortexFloatParams[simi_vortex_joint_p1stiffness]==0.0f)
        _vortexFloatParams[simi_vortex_joint_p1stiffness]=1e-35f;
    if (_vortexFloatParams[simi_vortex_joint_p2stiffness]==0.0f)
        _vortexFloatParams[simi_vortex_joint_p2stiffness]=1e-35f;
    if (_vortexFloatParams[simi_vortex_joint_a0stiffness]==0.0f)
        _vortexFloatParams[simi_vortex_joint_a0stiffness]=1e-35f;
    if (_vortexFloatParams[simi_vortex_joint_a1stiffness]==0.0f)
        _vortexFloatParams[simi_vortex_joint_a1stiffness]=1e-35f;
    if (_vortexFloatParams[simi_vortex_joint_a2stiffness]==0.0f)
        _vortexFloatParams[simi_vortex_joint_a2stiffness]=1e-35f;
    // Limit offset and factor for dependency equation:
    if (_vortexFloatParams[simi_vortex_joint_dependencyfactor]>10000.0f)
        _vortexFloatParams[simi_vortex_joint_dependencyfactor]=10000.0f;
    if (_vortexFloatParams[simi_vortex_joint_dependencyfactor]<-10000.0f)
        _vortexFloatParams[simi_vortex_joint_dependencyfactor]=-10000.0f;
    if (_vortexFloatParams[simi_vortex_joint_dependencyoffset]>10000.0f)
        _vortexFloatParams[simi_vortex_joint_dependencyoffset]=10000.0f;
    if (_vortexFloatParams[simi_vortex_joint_dependencyoffset]<-10000.0f)
        _vortexFloatParams[simi_vortex_joint_dependencyoffset]=-10000.0f;
}

void CJoint::getVortexIntParams(std::vector<int>& p)
{
    _vortexIntParams[4]=_objectHandle;
    p.assign(_vortexIntParams.begin(),_vortexIntParams.end());
}

void CJoint::setVortexIntParams(const std::vector<int>& p)
{
    _vortexIntParams.assign(p.begin(),p.end());
}



int CJoint::getNewtonDependentJointId()
{
    return(_newtonIntParams[1]);
}

void CJoint::getNewtonFloatParams(std::vector<float>& p)
{
    p.assign(_newtonFloatParams.begin(),_newtonFloatParams.end());
}

void CJoint::setNewtonFloatParams(const std::vector<float>& p)
{
    _newtonFloatParams.assign(p.begin(),p.end());
    // Limit offset and factor for dependency equation:
    if (_newtonFloatParams[simi_newton_joint_dependencyfactor]>10000.0f)
        _newtonFloatParams[simi_newton_joint_dependencyfactor]=10000.0f;
    if (_newtonFloatParams[simi_newton_joint_dependencyfactor]<-10000.0f)
        _newtonFloatParams[simi_newton_joint_dependencyfactor]=-10000.0f;
    if (_newtonFloatParams[simi_newton_joint_dependencyoffset]>10000.0f)
        _newtonFloatParams[simi_newton_joint_dependencyoffset]=10000.0f;
    if (_newtonFloatParams[simi_newton_joint_dependencyoffset]<-10000.0f)
        _newtonFloatParams[simi_newton_joint_dependencyoffset]=-10000.0f;
}

void CJoint::getNewtonIntParams(std::vector<int>& p)
{
    _newtonIntParams[0]=_objectHandle;
    p.assign(_newtonIntParams.begin(),_newtonIntParams.end());
}

void CJoint::setNewtonIntParams(const std::vector<int>& p)
{
    _newtonIntParams.assign(p.begin(),p.end());
}


void CJoint::copyEnginePropertiesTo(CJoint* target)
{
    // Bullet:
    target->_bulletFloatParams.assign(_bulletFloatParams.begin(),_bulletFloatParams.end());
    target->_bulletIntParams.assign(_bulletIntParams.begin(),_bulletIntParams.end());

    // ODE:
    target->_odeFloatParams.assign(_odeFloatParams.begin(),_odeFloatParams.end());
    target->_odeIntParams.assign(_odeIntParams.begin(),_odeIntParams.end());

    // Vortex:
    target->_vortexFloatParams.assign(_vortexFloatParams.begin(),_vortexFloatParams.end());
    target->_vortexIntParams.assign(_vortexIntParams.begin(),_vortexIntParams.end());

    // Newton:
    target->_newtonFloatParams.assign(_newtonFloatParams.begin(),_newtonFloatParams.end());
    target->_newtonIntParams.assign(_newtonIntParams.begin(),_newtonIntParams.end());
}

void CJoint::setEnableDynamicMotor(bool e)
{
    _dynamicMotorEnabled=e;
}

bool CJoint::getEnableDynamicMotor()
{
    return(_dynamicMotorEnabled);
}

void CJoint::setDynamicMotorTargetVelocity(float v)
{
    if (_jointType==sim_joint_spherical_subtype)
        return;
    if (_jointType==sim_joint_revolute_subtype)
        v=tt::getLimitedFloat(-90000.0f*degToRad_f,+90000.0f*degToRad_f,v); // 250 rot/sec --> 0.25 rot/1ms
    if (_jointType==sim_joint_prismatic_subtype)
        v=tt::getLimitedFloat(-100.0f,+100.0f,v); // 100 m/sec --> 0.1 m/1ms
    _dynamicMotorTargetVelocity=v;
}

void CJoint::setDynamicMotorLockModeWhenInVelocityControl(bool e)
{
    _dynamicLockModeWhenInVelocityControl=e;
}

bool CJoint::getDynamicMotorLockModeWhenInVelocityControl()
{
    return(_dynamicLockModeWhenInVelocityControl);
}

float CJoint::getDynamicMotorTargetVelocity()
{
    return(_dynamicMotorTargetVelocity);
}

void CJoint::setDynamicMotorUpperLimitVelocity(float v)
{
    if (_jointType==sim_joint_spherical_subtype)
        return;
    v=tt::getLimitedFloat(0.01f,20.0f,v);
    _dynamicMotorUpperLimitVelocity=v;
}

float CJoint::getDynamicMotorUpperLimitVelocity()
{
    return(_dynamicMotorUpperLimitVelocity);
}

void CJoint::setDynamicMotorMaximumForce(float f)
{
    if (_jointType==sim_joint_spherical_subtype)
        return;
    if (_jointType==sim_joint_revolute_subtype)
        f=tt::getLimitedFloat(0.0f,+100000000000.0f,f);
    if (_jointType==sim_joint_prismatic_subtype)
        f=tt::getLimitedFloat(0.0f,+10000000000.0f,f);
    _dynamicMotorMaximumForce=f;
}

float CJoint::getDynamicMotorMaximumForce()
{
    return(_dynamicMotorMaximumForce);
}

void CJoint::setEnableDynamicMotorControlLoop(bool p)
{
    _dynamicMotorControlLoopEnabled=p;
}

bool CJoint::getEnableDynamicMotorControlLoop()
{
    return(_dynamicMotorControlLoopEnabled);
}

void CJoint::setEnableTorqueModulation(bool p)
{
    _dynamicMotorPositionControl_torqueModulation=p;
}

bool CJoint::getEnableTorqueModulation()
{
    return(_dynamicMotorPositionControl_torqueModulation);
}

void CJoint::setDynamicMotorPositionControlParameters(float p_param,float i_param,float d_param)
{
    _dynamicMotorPositionControl_P=tt::getLimitedFloat(-1000.0f,1000.0f,p_param);
    _dynamicMotorPositionControl_I=tt::getLimitedFloat(-1000.0f,1000.0f,i_param);
    _dynamicMotorPositionControl_D=tt::getLimitedFloat(-1000.0f,1000.0f,d_param);
}

void CJoint::getDynamicMotorPositionControlParameters(float& p_param,float& i_param,float& d_param)
{
    p_param=_dynamicMotorPositionControl_P;
    i_param=_dynamicMotorPositionControl_I;
    d_param=_dynamicMotorPositionControl_D;
}

void CJoint::setDynamicMotorSpringControlParameters(float k_param,float c_param)
{
    float maxVal=+10000000000.0f;
    if (_jointType==sim_joint_revolute_subtype)
        maxVal=+100000000000.0f;

    _dynamicMotorSpringControl_K=tt::getLimitedFloat(-maxVal,maxVal,k_param);
    _dynamicMotorSpringControl_C=tt::getLimitedFloat(-maxVal,maxVal,c_param);
}

void CJoint::getDynamicMotorSpringControlParameters(float& k_param,float& c_param)
{
    k_param=_dynamicMotorSpringControl_K;
    c_param=_dynamicMotorSpringControl_C;
}

void CJoint::setDynamicMotorPositionControlTargetPosition(float pos)
{
    if (_jointType==sim_joint_revolute_subtype)
    {
        if (_positionIsCyclic)
            _dynamicMotorPositionControl_targetPosition=tt::getNormalizedAngle(pos);
        else
            _dynamicMotorPositionControl_targetPosition=pos;
    }
    if (_jointType==sim_joint_prismatic_subtype)
        _dynamicMotorPositionControl_targetPosition=pos;
}

float CJoint::getDynamicMotorPositionControlTargetPosition()
{
    return(_dynamicMotorPositionControl_targetPosition);
}

void CJoint::setDynamicMotorReflectedPosition_useOnlyFromDynamicPart(float rfp)
{
    {   // When the joint is in dynamic mode we disable the joint limits and allow a cyclic behaviour (revolute joints)
        // This is because dynamic joints can over or undershoot limits.
        // So we set the position directly, without checking for limits.
        // Turn count is taken care by the physics plugin.

        _jointPosition=rfp;
    }
    _rectifyDependentJoints(false);
}

bool CJoint::setDependencyJointID(int depJointID)
{
    bool retVal=false;
    if ( (_jointType!=sim_joint_spherical_subtype)&&(getJointMode()==sim_jointmode_dependent) )
    {
        _dependencyJointID=depJointID;
        if (depJointID==-1)
        { // We disable the dependency:
            _dependencyJointID=-1;
            App::ct->objCont->actualizeObjectInformation();
        }
        else
        {
            // We now check for an illegal loop:
            CJoint* it=App::ct->objCont->getJoint(depJointID);
            CJoint* iterat=it;
            while (iterat->getDependencyJointID()!=-1)
            {
                if (iterat->getJointMode()!=_jointMode)
                    break; // We might have a loop, but it is interupted by another jointMode!! (e.g. IK dependency VS direct dependency)
                int joint=iterat->getDependencyJointID();
                if (joint==getObjectHandle())
                { // We have an illegal loop! We disable it:
                    iterat->setDependencyJointID(-1);
                    break;
                }
                iterat=App::ct->objCont->getJoint(joint);
            }
            App::ct->objCont->actualizeObjectInformation();
            setPosition(getPosition()); // To actualize dependencies
        }
        retVal=true;
    }
    return(retVal);
}

void CJoint::setDependencyJointCoeff(float coeff)
{
    if (_jointType==sim_joint_spherical_subtype)
        return;
    coeff=tt::getLimitedFloat(-10000.0f,10000.0f,coeff);
    _dependencyJointCoeff=coeff;
    setPosition(getPosition()); // To actualize dependencies
}

void CJoint::setDependencyJointOffset(float off)
{
    if (_jointType==sim_joint_spherical_subtype)
        return;
    off=tt::getLimitedFloat(-10000.0f,10000.0f,off);
    _dependencyJointOffset=off;
    setPosition(getPosition()); // To actualize dependencies
}

void CJoint::measureJointVelocity(float dt)
{
    if (_jointType==sim_joint_spherical_subtype)
        return;
    if (_previousJointPositionIsValid)
    {
        if (_positionIsCyclic)
            _measuredJointVelocity_velocityMeasurement=tt::getAngleMinusAlpha(_jointPosition,_previousJointPosition_velocityMeasurement)/dt;
        else
            _measuredJointVelocity_velocityMeasurement=(_jointPosition-_previousJointPosition_velocityMeasurement)/dt;
    }
    _previousJointPosition_velocityMeasurement=_jointPosition;
    _previousJointPositionIsValid=true;
}

float CJoint::getMeasuredJointVelocity()
{
    return(_measuredJointVelocity_velocityMeasurement);
}

void CJoint::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    initializeInitialValuesMain(simulationIsRunning);
    _initialValuesInitialized=simulationIsRunning;
    _dynamicSecondPartIsValid=false; // do the same as for force sensors here?! (if the joint is copied while apart, paste it apart too!)
    _measuredJointVelocity_velocityMeasurement=0.0f;
    _previousJointPositionIsValid=false;
//    _previousJointPosition_velocityMeasurement=_jointPosition;
    if (simulationIsRunning)
    {
        _initialPosition=_jointPosition;
        _initialSphericalJointTransformation=_sphericalTransformation;

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

        _cumulatedForceOrTorque=0.0f;
        _cumulativeForceOrTorqueTmp=0.0f;

        _averageForceOrTorqueValid=false;
        _lastForceOrTorqueValid_dynStep=false;

        _jointPositionForMotionHandling_DEPRECATED=_jointPosition;
        _velocity_DEPRECATED=0.0f;
        _initialVelocity_DEPRECATED=_velocity_DEPRECATED;
        _initialTargetVelocity_DEPRECATED=_targetVelocity_DEPRECATED;
        _initialExplicitHandling_DEPRECATED=_explicitHandling_DEPRECATED;
    }
}

void CJoint::simulationAboutToStart()
{
    initializeInitialValues(true);
}

void CJoint::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
    {
        _jointPosition=_initialPosition;
        _sphericalTransformation=_initialSphericalJointTransformation;

        _dynamicMotorEnabled=_initialDynamicMotorEnabled;
        _dynamicMotorTargetVelocity=_initialDynamicMotorTargetVelocity;
        _dynamicLockModeWhenInVelocityControl=_initialDynamicMotorLockModeWhenInVelocityControl;
        _dynamicMotorUpperLimitVelocity=_initialDynamicMotorUpperLimitVelocity;
        _dynamicMotorMaximumForce=_initialDynamicMotorMaximumForce;

        _dynamicMotorControlLoopEnabled=_initialDynamicMotorControlLoopEnabled;
        _dynamicMotorPositionControl_P=_initialDynamicMotorPositionControl_P;
        _dynamicMotorPositionControl_I=_initialDynamicMotorPositionControl_I;
        _dynamicMotorPositionControl_D=_initialDynamicMotorPositionControl_D;
        _dynamicMotorSpringControl_K=_initialDynamicMotorSpringControl_K;
        _dynamicMotorSpringControl_C=_initialDynamicMotorSpringControl_C;
        _dynamicMotorPositionControl_targetPosition=_jointPosition;
        _dynamicMotorPositionControl_targetPosition=_initialTargetPosition;

        _jointMode=_initialJointMode;
        _jointHasHybridFunctionality=_initialHybridOperation;

        _explicitHandling_DEPRECATED=_initialExplicitHandling_DEPRECATED;
        _velocity_DEPRECATED=_initialVelocity_DEPRECATED;
        _targetVelocity_DEPRECATED=_initialTargetVelocity_DEPRECATED;
    }
    _dynamicSecondPartIsValid=false;
    _initialValuesInitialized=false;

    _averageForceOrTorqueValid=false;
    _lastForceOrTorqueValid_dynStep=false;

    _cumulatedForceOrTorque=0.0f;
    _cumulativeForceOrTorqueTmp=0.0f;

    simulationEndedMain();
}

// FOLLOWING FUNCTIONS ARE DEPRECATED:
//----------------------------------------
void CJoint::resetJoint_DEPRECATED()
{ // DEPRECATED
    if ( (_jointMode!=sim_jointmode_motion_deprecated)||(!App::ct->mainSettings->jointMotionHandlingEnabled_DEPRECATED) )
        return;
    if (_initialValuesInitialized)
    {
        setPosition(_initialPosition);
        setVelocity_DEPRECATED(_initialVelocity_DEPRECATED);
        setTargetVelocity_DEPRECATED(_initialTargetVelocity_DEPRECATED);
    }
}

void CJoint::handleJoint_DEPRECATED(float deltaTime)
{ // DEPRECATED. handling the motion here. Not elegant at all. In future, try using the Reflexxes RML library!
    if ( (_jointMode!=sim_jointmode_motion_deprecated)||(!App::ct->mainSettings->jointMotionHandlingEnabled_DEPRECATED) )
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
            setPosition(newPos);
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

        setPosition(float(newPos));
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
bool CJoint::isPotentiallyCollidable() const
{
    return(false);
}
bool CJoint::isPotentiallyMeasurable() const
{
    return(false);
}
bool CJoint::isPotentiallyDetectable() const
{
    return(false);
}
bool CJoint::isPotentiallyRenderable() const
{
    return(false);
}
bool CJoint::isPotentiallyCuttable() const
{
    return(false);
}

bool CJoint::getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
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
    return(true);
}

bool CJoint::getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    return(getFullBoundingBox(minV,maxV));
}

bool CJoint::getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const
{
    vertices.clear();
    indices.clear();
    return(false); // for now
}

void CJoint::setScrewPitch(float pitch)
{
    if (_jointType==sim_joint_revolute_subtype)
    {
        if (_jointMode!=sim_jointmode_force)
        { // no pitch when in torque/force mode
            pitch=tt::getLimitedFloat(-10.0f,10.0f,pitch);
            _screwPitch=pitch;
            if (_screwPitch!=0.0f)
                _jointHasHybridFunctionality=false;
        }
    }
}

void CJoint::setPositionIntervalMin(float min)
{
    if (_jointType==sim_joint_spherical_subtype)
        return;
    if (_jointType==sim_joint_revolute_subtype)
    {
        min=tt::getLimitedFloat(-100000.0f,100000.0f,min);
    }
    if (_jointType==sim_joint_prismatic_subtype)
        min=tt::getLimitedFloat(-1000.0f,1000.0f,min);
    _jointMinPosition=min;
    setPosition(getPosition()); // to make sure it is within range (will also get/set velocity)
}

void CJoint::setPositionIntervalRange(float range)
{
    if (_jointType==sim_joint_revolute_subtype)
    {

        // Commented following out on 23/3/2014 because:
        // limiting the range is problematic when doing motion planning for instance
        // imagine the angle has a range of 0, +- 360 degrees, which is often the case.
        // We solve the problem by simply having no dynamic limitations when the joint
        // range is larger than 360 degrees.
//      if ((_jointMode!=sim_jointmode_force)&&(!_jointHasHybridFunctionality))
        {
            range=tt::getLimitedFloat(0.001f*degToRad_f,10000000.0f*degToRad_f,range);
        }
//      else
//          range=tt::getLimitedFloat(0.0f,piValTimes2_f,range);

    }
    if (_jointType==sim_joint_prismatic_subtype)
        range=tt::getLimitedFloat(0.0f,1000.0f,range);
    if (_jointType==sim_joint_spherical_subtype)
    {
        if (_jointMode!=sim_jointmode_force)
            range=tt::getLimitedFloat(0.001f*degToRad_f,10000000.0f*degToRad_f,range);
        else
            range=piValue_f;
    }
    _jointPositionRange=range;
    setPosition(getPosition()); // to make sure it is within range (will also get/set velocity)
    setSphericalTransformation(getSphericalTransformation());
}

void CJoint::setLength(float l)
{
    tt::limitValue(0.001f,1000.0f,l);
    _length=l;
}

void CJoint::setDiameter(float d)
{
    tt::limitValue(0.0001f,100.0f,d);
    _diameter=d;
}

float CJoint::getLength() 
{ 
    return(_length); 
}
float CJoint::getDiameter() 
{ 
    return(_diameter); 
}

void CJoint::scaleObject(float scalingFactor)
{
    _diameter*=scalingFactor;
    _length*=scalingFactor;
    _screwPitch*=scalingFactor;
    if (_jointType==sim_joint_prismatic_subtype)
    {
        _jointPosition*=scalingFactor;
        _jointPositionForMotionHandling_DEPRECATED*=scalingFactor;
        _jointMinPosition*=scalingFactor;
        _jointPositionRange*=scalingFactor;
        _dependencyJointOffset*=scalingFactor;
        _maxStepSize*=scalingFactor;
        _dynamicMotorPositionControl_targetPosition*=scalingFactor;

        // following 2 new since 7/5/2014:
        _dynamicMotorSpringControl_K*=scalingFactor*scalingFactor;
        _dynamicMotorSpringControl_C*=scalingFactor*scalingFactor;

        _dynamicMotorTargetVelocity*=scalingFactor;
        _dynamicMotorUpperLimitVelocity*=scalingFactor;
        if (_dynamicMotorPositionControl_torqueModulation) // this condition and next line added on 04/10/2013 (Alles Gute zu Geburtstag Mama :) )
            _dynamicMotorMaximumForce*=scalingFactor*scalingFactor; //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
        else
            _dynamicMotorMaximumForce*=scalingFactor*scalingFactor*scalingFactor; //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

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
        _dynamicMotorMaximumForce*=scalingFactor*scalingFactor*scalingFactor*scalingFactor;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

        // following 2 new since 7/5/2014:
        _dynamicMotorSpringControl_K*=scalingFactor*scalingFactor*scalingFactor*scalingFactor;
        _dynamicMotorSpringControl_C*=scalingFactor*scalingFactor*scalingFactor*scalingFactor;

        if (_initialValuesInitialized)
        {
            _initialDynamicMotorMaximumForce*=scalingFactor*scalingFactor*scalingFactor*scalingFactor;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
            // following 2 new since 7/5/2014:
            _initialDynamicMotorSpringControl_K*=scalingFactor*scalingFactor*scalingFactor*scalingFactor;
            _initialDynamicMotorSpringControl_C*=scalingFactor*scalingFactor*scalingFactor*scalingFactor;
        }
    }

    scaleObjectMain(scalingFactor);

    // We have to reconstruct a part of the dynamics world:
    _dynamicsFullRefreshFlag=true;

    _averageForceOrTorqueValid=false;
    _lastForceOrTorqueValid_dynStep=false;
}

void CJoint::scaleObjectNonIsometrically(float x,float y,float z)
{
    float diam=sqrt(x*y);
    _diameter*=diam;
    _length*=z;
    _screwPitch*=z;
    if (_jointType==sim_joint_prismatic_subtype)
    {
        _jointPosition*=z;
        _jointMinPosition*=z;
        _jointPositionRange*=z;
        _dependencyJointOffset*=z;
        _maxStepSize*=z;
        _dynamicMotorPositionControl_targetPosition*=z;

        // following 2 new since 7/5/2014:
        _dynamicMotorSpringControl_K*=diam*diam;
        _dynamicMotorSpringControl_C*=diam*diam;

        _dynamicMotorTargetVelocity*=z;
        _dynamicMotorUpperLimitVelocity*=z;
        if (_dynamicMotorPositionControl_torqueModulation) // this condition and next line added on 04/10/2013 (Alles Gute zu Geburtstag Mama :) )
            _dynamicMotorMaximumForce*=diam*diam; //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
        else
            _dynamicMotorMaximumForce*=diam*diam*diam; //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

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
        _dynamicMotorMaximumForce*=diam*diam*diam*diam;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

        // following 2 new since 7/5/2014:
        _dynamicMotorSpringControl_K*=diam*diam*diam*diam;
        _dynamicMotorSpringControl_C*=diam*diam*diam*diam;

        if (_initialValuesInitialized)
        {
            _initialDynamicMotorMaximumForce*=diam*diam*diam*diam;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
            // following 2 new since 7/5/2014:
            _initialDynamicMotorSpringControl_K*=diam*diam*diam*diam;
            _initialDynamicMotorSpringControl_C*=diam*diam*diam*diam;
        }
    }

    scaleObjectNonIsometricallyMain(diam,diam,z);

    // We have to reconstruct a part of the dynamics world:
    _dynamicsFullRefreshFlag=true;
}

void CJoint::addCumulativeForceOrTorque(float forceOrTorque,int countForAverage)
{ // New routine since 1/6/2011. The countForAverage mechanism is needed because we need to average all values in a simulation time step (but this is called every dynamic simulation time step!!)
    _cumulativeForceOrTorqueTmp+=forceOrTorque;
    _lastForceOrTorque_dynStep=forceOrTorque;
    _lastForceOrTorqueValid_dynStep=true;
    if (countForAverage>0)
    {
        _cumulatedForceOrTorque=_cumulativeForceOrTorqueTmp/float(countForAverage);
        _cumulativeForceOrTorqueTmp=0.0f;
        _averageForceOrTorqueValid=true;
    }
}

void CJoint::setForceOrTorqueNotValid()
{
    _averageForceOrTorqueValid=false;
    _lastForceOrTorqueValid_dynStep=false;
}

bool CJoint::getDynamicForceOrTorque(float& forceOrTorque,bool dynamicStepValue)
{
    if (dynamicStepValue)
    {
        if (App::ct->dynamicsContainer->getCurrentlyInDynamicsCalculations())
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

int CJoint::getJointCallbackCallOrder_backwardCompatibility()
{
    return(_jointCallbackCallOrder_backwardCompatibility);
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
            int res=((jointCtrlCallback)getAllJointCtrlCallbacks()[i])(getObjectHandle(),App::ct->dynamicsContainer->getDynamicEngineType(nullptr),0,intParams,floatParams,retParams);
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
        CLuaScriptObject* script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(getObjectHandle());
        if (script!=nullptr)
        {
            if (!script->getContainsJointCallbackFunction())
                script=nullptr;
        }
        CLuaScriptObject* cScript=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(getObjectHandle());
        if (cScript!=nullptr)
        {
            if (!cScript->getContainsJointCallbackFunction())
                cScript=nullptr;
        }
        if ( (script!=nullptr)||(cScript!=nullptr) )
        { // a child or customization scripts want to handle the joint (new calling method)
            // 1. We prepare the in/out stacks:
            CInterfaceStack inStack;
            inStack.pushTableOntoStack();
            inStack.pushStringOntoStack("first",0);
            inStack.pushBoolOntoStack(init);
            inStack.insertDataIntoStackTable();
            inStack.pushStringOntoStack("revolute",0);
            inStack.pushBoolOntoStack(rev);
            inStack.insertDataIntoStackTable();
            inStack.pushStringOntoStack("cyclic",0);
            inStack.pushBoolOntoStack(cycl);
            inStack.insertDataIntoStackTable();
            inStack.pushStringOntoStack("handle",0);
            inStack.pushNumberOntoStack(getObjectHandle());
            inStack.insertDataIntoStackTable();
            inStack.pushStringOntoStack("lowLimit",0);
            inStack.pushNumberOntoStack(lowL);
            inStack.insertDataIntoStackTable();
            inStack.pushStringOntoStack("highLimit",0);
            inStack.pushNumberOntoStack(highL);
            inStack.insertDataIntoStackTable();
            inStack.pushStringOntoStack("passCnt",0);
            inStack.pushNumberOntoStack(loopCnt);
            inStack.insertDataIntoStackTable();
            inStack.pushStringOntoStack("totalPasses",0);
            inStack.pushNumberOntoStack(totalLoops);
            inStack.insertDataIntoStackTable();
            inStack.pushStringOntoStack("currentPos",0);
            inStack.pushNumberOntoStack(currentPos);
            inStack.insertDataIntoStackTable();
            inStack.pushStringOntoStack("targetPos",0);
            inStack.pushNumberOntoStack(targetPos);
            inStack.insertDataIntoStackTable();
            inStack.pushStringOntoStack("errorValue",0);
            inStack.pushNumberOntoStack(errorV);
            inStack.insertDataIntoStackTable();
            inStack.pushStringOntoStack("effort",0);
            inStack.pushNumberOntoStack(effort);
            inStack.insertDataIntoStackTable();
            inStack.pushStringOntoStack("dynStepSize",0);
            inStack.pushNumberOntoStack(dynStepSize);
            inStack.insertDataIntoStackTable();
            inStack.pushStringOntoStack("targetVel",0);
            inStack.pushNumberOntoStack(targetVel);
            inStack.insertDataIntoStackTable();
            inStack.pushStringOntoStack("maxForce",0);
            inStack.pushNumberOntoStack(maxForce);
            inStack.insertDataIntoStackTable();
            inStack.pushStringOntoStack("velUpperLimit",0);
            inStack.pushNumberOntoStack(upperLimitVel);
            inStack.insertDataIntoStackTable();
            CInterfaceStack outStack;

            // 2. Call the script(s):
            if (script!=nullptr)
                script->runNonThreadedChildScript(sim_syscb_jointcallback,&inStack,&outStack);
            if ( (cScript!=nullptr)&&(outStack.getStackSize()==0) )
                cScript->runCustomizationScript(sim_syscb_jointcallback,&inStack,&outStack);
            // 3. Collect the return values:
            if (outStack.getStackSize()>0)
            {
                int s=outStack.getStackSize();
                if (s>1)
                    outStack.moveStackItemToTop(0);
                outStack.getStackMapFloatValue("force",forceTorque);
                outStack.getStackMapFloatValue("velocity",velocity);
            }
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
    if (!isCyclic)
        _positionIsCyclic=isCyclic;
    else
    {
        if (getJointType()==sim_joint_revolute_subtype)
        {
            _screwPitch=0.0f;
            _jointMinPosition=-piValue_f;
            _jointPositionRange=piValTimes2_f;
            _positionIsCyclic=isCyclic;
        }
    }

    setVelocity_DEPRECATED(getVelocity_DEPRECATED()); // To make sure velocity is within allowed range
}

void CJoint::removeSceneDependencies()
{
    removeSceneDependenciesMain();
    _dependencyJointID=-1;
}

C3DObject* CJoint::copyYourself()
{
    CJoint* newJoint=(CJoint*)copyYourselfMain();

    newJoint->_jointType=_jointType;
    newJoint->_jointMode=_jointMode;
    newJoint->_screwPitch=_screwPitch;
    newJoint->_sphericalTransformation=_sphericalTransformation;
    newJoint->_jointPosition=_jointPosition;
    newJoint->_dependencyJointID=_dependencyJointID;
    newJoint->_dependencyJointCoeff=_dependencyJointCoeff;
    newJoint->_dependencyJointOffset=_dependencyJointOffset;
    newJoint->_ikWeight=_ikWeight;
    newJoint->_diameter=_diameter;
    newJoint->_length=_length;
    newJoint->_positionIsCyclic=_positionIsCyclic;
    newJoint->_jointPositionRange=_jointPositionRange;
    newJoint->_jointMinPosition=_jointMinPosition;
    newJoint->_maxStepSize=_maxStepSize;

    colorPart1.copyYourselfInto(&newJoint->colorPart1);
    colorPart2.copyYourselfInto(&newJoint->colorPart2);

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
    newJoint->_dynamicSecondPartIsValid=_dynamicSecondPartIsValid;
    newJoint->_dynamicSecondPartLocalTransform=_dynamicSecondPartLocalTransform;
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
    announceCollectionWillBeErasedMain(groupID,copyBuffer);
}
void CJoint::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollisionWillBeErasedMain(collisionID,copyBuffer);
}
void CJoint::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceDistanceWillBeErasedMain(distanceID,copyBuffer);
}
void CJoint::announceGcsObjectWillBeErased(int gcsObjectID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceGcsObjectWillBeErasedMain(gcsObjectID,copyBuffer);
}

void CJoint::performIkLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
    performIkLoadingMappingMain(map,loadingAmodel);
}
void CJoint::performCollectionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollectionLoadingMappingMain(map,loadingAmodel);
}
void CJoint::performCollisionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollisionLoadingMappingMain(map,loadingAmodel);
}
void CJoint::performDistanceLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performDistanceLoadingMappingMain(map,loadingAmodel);
}

void CJoint::performGcsLoadingMapping(std::vector<int>* map)
{
    performGcsLoadingMappingMain(map);
}

void CJoint::performTextureObjectLoadingMapping(std::vector<int>* map)
{
    performTextureObjectLoadingMappingMain(map);
}

void CJoint::performDynMaterialObjectLoadingMapping(std::vector<int>* map)
{
    performDynMaterialObjectLoadingMappingMain(map);
}

void CJoint::bufferMainDisplayStateVariables()
{
    bufferMainDisplayStateVariablesMain();
    _jointPosition_buffered=_jointPosition;
    _sphericalTransformation_buffered=_sphericalTransformation;
}

void CJoint::bufferedMainDisplayStateVariablesToDisplay()
{
    bufferedMainDisplayStateVariablesToDisplayMain();
    _jointPosition_forDisplay=_jointPosition_buffered;
    _sphericalTransformation_forDisplay=_sphericalTransformation_buffered;
}

bool CJoint::getHybridFunctionality()
{
    return(_jointHasHybridFunctionality);
}

float CJoint::getPosition_forDisplay(bool guiIsRendering)
{
    if (guiIsRendering)
        return(_jointPosition_forDisplay);
    else
        return(_jointPosition);
}

C4Vector CJoint::getSphericalTransformation_forDisplay(bool guiIsRendering) const
{
    if (guiIsRendering)
        return(_sphericalTransformation_forDisplay);
    else
        return(_sphericalTransformation);
}


void CJoint::serialize(CSer& ar)
{
    serializeMain(ar);
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
            colorPart1.serialize(ar,0);
            if (ar.setWritingMode())
                colorPart1.serialize(ar,0);

            ar.storeDataName("Cl2");
            ar.setCountingMode();
            colorPart2.serialize(ar,0);
            if (ar.setWritingMode())
                colorPart2.serialize(ar,0);

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
            ar << _dependencyJointID << _dependencyJointCoeff << _dependencyJointOffset;
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
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        colorPart1.serialize(ar,0);
                    }
                    if (theName.compare("Cl2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        colorPart2.serialize(ar,0);
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
                    }
                    if (theName.compare("Vaa")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        // _dynamicMotorCustomControl_OLD=SIM_IS_BIT_SET(dummy,0);
                        _dynamicLockModeWhenInVelocityControl=SIM_IS_BIT_SET(dummy,1);
                    }
                    if (theName.compare("Jmd")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _jointMode;
                        if (_jointMode==sim_jointmode_reserved_previously_ikdependent)
                            _jointMode=sim_jointmode_dependent; // since 4/7/2014 there is no more an ikdependent mode (ikdependent and dependent are treated as same)
                    }
                    if (theName.compare("Jdt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dependencyJointID >> _dependencyJointCoeff >> _dependencyJointOffset;
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

                        int cnt1_b=SIM_MIN(int(_bulletFloatParams.size()),cnt1);
                        int cnt2_b=SIM_MIN(int(_bulletIntParams.size()),cnt2);

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

                        int cnt1_b=SIM_MIN(int(_odeFloatParams.size()),cnt1);
                        int cnt2_b=SIM_MIN(int(_odeIntParams.size()),cnt2);

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

                        int cnt1_b=SIM_MIN(int(_vortexFloatParams.size()),cnt1);
                        int cnt2_b=SIM_MIN(int(_vortexIntParams.size()),cnt2);

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

                        int cnt1_b=SIM_MIN(int(_newtonFloatParams.size()),cnt1);
                        int cnt2_b=SIM_MIN(int(_newtonIntParams.size()),cnt2);

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
                    float maxPorD=SIM_MAX(fabs(_dynamicMotorPositionControl_P),fabs(_dynamicMotorPositionControl_D));
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
            { // on 29/08/2013 we corrected all default lights. So we need to correct for that change:
                CTTUtil::scaleColorUp_(colorPart1.colors);
                CTTUtil::scaleColorUp_(colorPart2.colors);
            }
        }
    }
}

void CJoint::serializeWExtIk(CExtIkSer& ar)
{
    serializeWExtIkMain(ar);

    ar.writeInt(_jointType);

    ar.writeFloat(_screwPitch);

    ar.writeFloat(_sphericalTransformation(0));
    ar.writeFloat(_sphericalTransformation(1));
    ar.writeFloat(_sphericalTransformation(2));
    ar.writeFloat(_sphericalTransformation(3));

    unsigned char dummy=0;
    SIM_SET_CLEAR_BIT(dummy,0,_positionIsCyclic);
    ar.writeByte(dummy);

    ar.writeFloat(_jointMinPosition);
    ar.writeFloat(_jointPositionRange);

    ar.writeFloat(_jointPosition);

    ar.writeFloat(_maxStepSize);

    ar.writeFloat(_ikWeight);

    ar.writeInt(_jointMode);

    ar.writeInt(_dependencyJointID);
    ar.writeFloat(_dependencyJointCoeff);
    ar.writeFloat(_dependencyJointOffset);
}

void CJoint::performObjectLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    performObjectLoadingMappingMain(map,loadingAmodel);
    _dependencyJointID=App::ct->objCont->getLoadingMapping(map,_dependencyJointID);
    _vortexIntParams[5]=App::ct->objCont->getLoadingMapping(map,_vortexIntParams[5]); // Vortex dependency joint
    _newtonIntParams[1]=App::ct->objCont->getLoadingMapping(map,_newtonIntParams[1]); // Newton dependency joint
}

void CJoint::setJointMode(int theMode,bool correctDynMotorTargetPositions/*=true*/)
{
    if (theMode==sim_jointmode_passive)
    {
        //_dependencyJointID=-1;
        App::ct->objCont->actualizeObjectInformation(); // to actualize dependencies
        _jointMode=theMode;
    }
    if (theMode==sim_jointmode_motion_deprecated)
    {
        if (_jointMode!=theMode)
        { // we switched back to the motion mode, which is deprecated. So we turn it off
            _velocity_DEPRECATED=0.0f;
            _targetVelocity_DEPRECATED=0.0f;
            _explicitHandling_DEPRECATED=true;
            _unlimitedAcceleration_DEPRECATED=true;
            _invertTargetVelocityAtLimits_DEPRECATED=false;
        }
        if (_jointType!=sim_joint_spherical_subtype)
        {
            //_dependencyJointID=-1;
            App::ct->objCont->actualizeObjectInformation(); // to actualize dependencies
            _jointMode=theMode;
        }
    }
    if ((theMode==sim_jointmode_dependent)||(theMode==sim_jointmode_reserved_previously_ikdependent))
    {
        if (_jointType!=sim_joint_spherical_subtype)
        {
            App::ct->objCont->actualizeObjectInformation(); // to actualize dependencies
            _jointMode=theMode;
        }
    }
    if (theMode==sim_jointmode_force)
    {
        _jointHasHybridFunctionality=false;
        _screwPitch=0.0f;
// REMOVED FOLLOWING ON 24/7/2015: causes problem when switching modes. The physics engine plugin will now not set limits if the range>=360
//      if (_jointType==sim_joint_revolute_subtype)
//          _jointPositionRange=tt::getLimitedFloat(0.0f,piValTimes2_f,_jointPositionRange); // new since 18/11/2012 (was forgotten)
        if (_jointType==sim_joint_spherical_subtype)
            _jointPositionRange=piValue_f;

        App::ct->objCont->actualizeObjectInformation(); // to actualize dependencies
        _jointMode=theMode;
    }
    if (theMode==sim_jointmode_ik)
    {
        //_dependencyJointID=-1;
        App::ct->objCont->actualizeObjectInformation(); // to actualize dependencies
        _jointMode=theMode;
    }
    setPosition(getPosition()); // needed here for ex. for dependent joints! // not sure it is needed here, but better than not!
    if ((theMode==sim_jointmode_force)&&_dynamicMotorControlLoopEnabled&&correctDynMotorTargetPositions)
    { // Make sure the target position is the same here (otherwise big jump)
        setDynamicMotorPositionControlTargetPosition(getPosition());
    }
}

void CJoint::_rectifyDependentJoints(bool useTempValues)
{   // We rectify now all joints linked to that one (rewritten on 2009-01-27):
    for (int i=0;i<int(directDependentJoints.size());i++)
    {
        if ((directDependentJoints[i]->getJointMode()==sim_jointmode_dependent)||(directDependentJoints[i]->getJointMode()==sim_jointmode_reserved_previously_ikdependent)) // second part on 3/7/2014
            directDependentJoints[i]->setPosition(0.0f,useTempValues); // value doesn't matter!
    }
}

int CJoint::getJointMode()
{
    return(_jointMode);
}

int CJoint::getDependencyJointID()
{
    return(_dependencyJointID);
}

float CJoint::getDependencyJointCoeff()
{
    return(_dependencyJointCoeff);
}

float CJoint::getDependencyJointFact()
{
    return(_dependencyJointOffset);
}

CVisualParam* CJoint::getColor(bool getPart2)
{
    if (getPart2)
        return(&colorPart2);
    return(&colorPart1);
}

int CJoint::getJointType()
{
    return(_jointType);
}

float CJoint::getScrewPitch() const
{
    return(_screwPitch);
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
    _sphericalTransformation=transf;
}

C4Vector CJoint::getSphericalTransformation() const
{
    return(_sphericalTransformation);
}

void CJoint::setMaxStepSize(float stepS)
{
    if (_jointType==sim_joint_revolute_subtype)
        tt::limitValue(0.01f*degToRad_f,100000.0f,stepS); // high number for screws!
    if (_jointType==sim_joint_prismatic_subtype)
        tt::limitValue(0.0001f,1000.0f,stepS);
    if (_jointType==sim_joint_spherical_subtype)
        tt::limitValue(0.01f*degToRad_f,piValue_f,stepS);
    _maxStepSize=stepS;
}

float CJoint::getMaxStepSize()
{
    return(_maxStepSize);
}

float CJoint::getPosition(bool useTempValues) 
{  // useTempValues is false by default
    if (useTempValues)
        return(_jointPosition_tempForIK); 
    return(_jointPosition); 
}

float CJoint::getIKWeight()
{
    return(_ikWeight);
}

void CJoint::setIKWeight(float newWeight)
{
    newWeight=tt::getLimitedFloat(-1.0f,1.0f,newWeight);
    _ikWeight=newWeight;
}

float CJoint::getPosition_ratio()
{
    return((getPosition()-_jointMinPosition)/_jointPositionRange);
}

void CJoint::setPosition_ratio(float posRatio)
{
    setPosition(_jointMinPosition+posRatio*_jointPositionRange);
}

void CJoint::setPosition(float parameter,bool useTempValues)
{ // useTempValues is false by default
    if (_positionIsCyclic)
        parameter=tt::getNormalizedAngle(parameter);
    else
    {
        if (parameter>(getPositionIntervalMin()+getPositionIntervalRange()))
            parameter=getPositionIntervalMin()+getPositionIntervalRange();
        if (parameter<getPositionIntervalMin())
            parameter=getPositionIntervalMin();
    }
    if (useTempValues)  
        _jointPosition_tempForIK=parameter;
    else
        _jointPosition=parameter;

    if ((_jointMode==sim_jointmode_dependent)||(_jointMode==sim_jointmode_reserved_previously_ikdependent))  // second part on 3/7/2014
    { // If this joint is constrained
        float linked=0.0f;
        if (_dependencyJointID!=-1)
        {
            CJoint* anAct=App::ct->objCont->getJoint(_dependencyJointID);
            if (anAct!=nullptr)
                linked=_dependencyJointCoeff*anAct->getPosition(useTempValues);
        }
        if (useTempValues)  
            _jointPosition_tempForIK=linked+_dependencyJointOffset;
        else
            _jointPosition=linked+_dependencyJointOffset;
    }
    _rectifyDependentJoints(useTempValues);
    setVelocity_DEPRECATED(getVelocity_DEPRECATED()); // To make sure velocity is within allowed range
}

float CJoint::getPositionIntervalMin() 
{ 
    return(_jointMinPosition); 
}

float CJoint::getPositionIntervalRange() 
{ 
    return(_jointPositionRange); 
}

bool CJoint::getPositionIsCyclic()
{
    if (_jointType==sim_joint_prismatic_subtype)
        return(false);
    return(_positionIsCyclic);
}

void CJoint::initializeParametersForIK(float angularJointLimitationThreshold)
{
    if (_jointType!=sim_joint_spherical_subtype)
        _jointPosition_tempForIK=_jointPosition;
    else
    {
        // 1. Do we need to prepare the thing for the joint limitation?
        _sphericalTransformation_eulerLockTempForIK=0;
        C3X3Matrix m(_sphericalTransformation);
        float angle=C3Vector::unitZVector.getAngle(m.axis[2]);
        if ( (_jointPositionRange<179.9f*degToRad_f)&&(angle>1.0f*degToRad_f) )
        {
            if ((_jointPositionRange-angularJointLimitationThreshold)/2.0f<angle)
            { // We have to activate the second type of spherical joint (with joint limitation (IK pass dependent))
                _sphericalTransformation_eulerLockTempForIK=2;
                C3Vector n(m.axis[2]);
                n(2)=0.0f;
                n.normalize();
                C3Vector y((C3Vector::unitZVector^n).getNormalized());
                float angle2=C3Vector::unitXVector.getAngle(y);
                C3Vector zz(C3Vector::unitXVector^y);
                if (zz(2)<0.0f)
                    angle2=-angle2;
                _jointPosition_tempForIK=0.0f; // Not really needed!
                _sphericalTransformation_euler1TempForIK=angle2;
                _sphericalTransformation_euler2TempForIK=angle;
                float angle3=m.axis[0].getAngle(y);
                C3Vector nz(y^m.axis[0]);
                if (nz*m.axis[2]<0.0f)
                    angle3=-angle3;
                _sphericalTransformation_euler3TempForIK=angle3;
            }
        }
        if (_sphericalTransformation_eulerLockTempForIK==0)
        { // No joint limitations for the IK (in this IK pass)
            _jointPosition_tempForIK=0.0f; // Not really needed!
            _sphericalTransformation_euler1TempForIK=0.0f;
            _sphericalTransformation_euler2TempForIK=0.0f;
            _sphericalTransformation_euler3TempForIK=0.0f;
        }
    }
}

int CJoint::getDoFs()
{
    if (_jointType!=sim_joint_spherical_subtype)
        return(1);
    return(3);
}

int CJoint::getTempSphericalJointLimitations()
{
    return(_sphericalTransformation_eulerLockTempForIK);
}

void CJoint::getLocalTransformationExPart1(C7Vector& mTr,int index,bool useTempValues)
{ // Used for Jacobian calculation with spherical joints
    if (_sphericalTransformation_eulerLockTempForIK==0)
    { // Spherical joint limitations are not activated in the IK algorithm (but if we come close to the limit, it might get activated in next pass!)
        if (index==0)
        { 
            mTr.setIdentity();
            mTr.Q.setEulerAngles(0.0f,piValD2_f,0.0f);
            C7Vector tr2(getLocalTransformation());
            mTr=tr2*mTr;
        }
        if (index==1)
        {
            mTr.setIdentity();
            mTr.Q.setEulerAngles(-piValD2_f,0.0f,-piValD2_f);
        }
        if (index==2)
        {
            mTr.setIdentity();
            mTr.Q.setEulerAngles(piValD2_f,0.0f,0.0f);
        }
    }
    else
    {
        if (index==0)
        {
            mTr=getLocalTransformationPart1();
        }
        if (index==1)
        {
            mTr.setIdentity();
            mTr.Q.setEulerAngles(0.0f,piValD2_f,0.0f);
        }
        if (index==2)
        {
            mTr.setIdentity();
            mTr.Q.setEulerAngles(0.0f,-piValD2_f,0.0f);
        }
    }
}

float CJoint::getTempParameterEx(int index)
{
    if (index==0)
        return(_sphericalTransformation_euler1TempForIK);
    if (index==1)
        return(_sphericalTransformation_euler2TempForIK);
    if (index==2)
        return(_sphericalTransformation_euler3TempForIK);
    return(0.0f);
}

void CJoint::setTempParameterEx(float parameter,int index)
{
    if (index==0)
        _sphericalTransformation_euler1TempForIK=parameter;
    if (index==1)
        _sphericalTransformation_euler2TempForIK=parameter;
    if (index==2)
        _sphericalTransformation_euler3TempForIK=parameter;

    if (_sphericalTransformation_eulerLockTempForIK==0)
    { // Spherical joint limitations are not activated in the IK algorithm (but if we come close to the limit, it might get activated in next pass!)
        C4Vector saved(_sphericalTransformation);
        applyTempParametersEx();
        C4Vector tr(saved.getInverse()*_sphericalTransformation);
        C3Vector euler(tr.getEulerAngles());
        _sphericalTransformation_euler1TempForIK=euler(0);
        _sphericalTransformation_euler2TempForIK=euler(1);
        _sphericalTransformation_euler3TempForIK=euler(2);
        _sphericalTransformation=saved;
    }
    else
    { // Spherical joint limitations are activated in the IK algorithm
        C4Vector saved(_sphericalTransformation);
        applyTempParametersEx();

        C3X3Matrix m(_sphericalTransformation);

        float angle=C3Vector::unitZVector.getAngle(m.axis[2]);
        if (angle>0.01f*degToRad_f)
        {
            C3Vector n(m.axis[2]);
            n(2)=0.0f;
            n.normalize();
            C3Vector y((C3Vector::unitZVector^n).getNormalized());
            float angle2=C3Vector::unitXVector.getAngle(y);
            C3Vector zz(C3Vector::unitXVector^y);
            if (zz(2)<0.0f)
                angle2=-angle2;
            _sphericalTransformation_euler1TempForIK=angle2;
            _sphericalTransformation_euler2TempForIK=angle;
            float angle3=m.axis[0].getAngle(y);
            C3Vector nz(y^m.axis[0]);
            if (nz*m.axis[2]<0.0f)
                angle3=-angle3;
            _sphericalTransformation_euler3TempForIK=angle3;
        }
        else
        { // This is a rare case and should never happen if the spherical joint limitation is not too small!
            float angle=C3Vector::unitXVector.getAngle(m.axis[0]);
            if ((C3Vector::unitXVector^m.axis[0])(2)<0.0f)
                angle=-angle;
            _sphericalTransformation_euler1TempForIK=angle;
            _sphericalTransformation_euler2TempForIK=0.0f;
            _sphericalTransformation_euler3TempForIK=0.0f;
        }
        _sphericalTransformation=saved;
    }
}

void CJoint::applyTempParametersEx()
{
    if (_jointType==sim_joint_spherical_subtype)
    {
        C7Vector tr1(getLocalTransformationPart1(true));
        C7Vector tr2(getLocalTransformation(true));
        setSphericalTransformation(tr1.Q.getInverse()*tr2.Q);
    }
}

bool CJoint::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(objectHandle)'-call or similar
    // Return value true means 'this' has to be erased too!
    bool retVal=announceObjectWillBeErasedMain(objectHandle,copyBuffer);
    if (_dependencyJointID==objectHandle)
        _dependencyJointID=-1;
    if (_vortexIntParams[5]==objectHandle) // that's the Vortex dependency joint
        _vortexIntParams[5]=-1;
    if (_newtonIntParams[1]==objectHandle) // that's the Vortex dependency joint
        _newtonIntParams[1]=-1;

    // We check if the joint is listed in the directDependentJoints:
    for (int i=0;i<int(directDependentJoints.size());i++)
    {
        if (directDependentJoints[i]->getObjectHandle()==objectHandle)
            directDependentJoints.erase(directDependentJoints.begin()+i);
    }
    return(retVal);
}

void CJoint::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceIkObjectWillBeErasedMain(ikGroupID,copyBuffer);
}

void CJoint::display(CViewableBase* renderingObject,int displayAttrib)
{
    FUNCTION_INSIDE_DEBUG("CJoint::display");
    EASYLOCK(_objectMutex);
    displayJoint(this,renderingObject,displayAttrib);
}
