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

CJoint::CJoint()
{
    _commonInit();
}

CJoint::CJoint(int jointType)
{
    _commonInit();
    _jointType=jointType;
    if (jointType==sim_joint_revolute_subtype)
    {
        _objectName=IDSOGL_REVOLUTE_JOINT;
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
        _objectName=IDSOGL_PRISMATIC_JOINT;
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
        _objectName=IDSOGL_SPHERICAL_JOINT;
        _jointMode=sim_jointmode_force;
        _positionIsCyclic=true;
        _jointPositionRange=piValue_f;
        _jointMinPosition=0.0f;
        _maxStepSize=10.0f*degToRad_f;
        _dynamicMotorMaximumForce=0.0f;
        _dynamicMotorUpperLimitVelocity=0.0f;
        _maxAcceleration_DEPRECATED=60.0f*degToRad_f;
    }
    _objectAltName=tt::getObjectAltNameFromObjectName(_objectName.c_str());
}

void CJoint::_commonInit()
{
    _visibilityLayer=JOINT_LAYER;
    _objectName=IDSOGL_JOINT;
    _objectAltName=tt::getObjectAltNameFromObjectName(_objectName.c_str());

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
    _jointCallbackCallOrder_backwardCompatibility=0;
    _explicitHandling_DEPRECATED=false;
    _unlimitedAcceleration_DEPRECATED=false;
    _invertTargetVelocityAtLimits_DEPRECATED=true;
    _maxAcceleration_DEPRECATED=60.0f*degToRad_f;

    _colorPart1.setDefaultValues();
    _colorPart1.setColor(1.0f,0.3f,0.1f,sim_colorcomponent_ambient_diffuse);
    _colorPart2.setDefaultValues();
    _colorPart2.setColor(0.1f,0.1f,0.9f,sim_colorcomponent_ambient_diffuse);
}

CJoint::~CJoint()
{

}

bool CJoint::setHybridFunctionality(bool h)
{ // Overridden from _CJoint_
    bool retVal=false;
    if ( (_jointType!=sim_joint_spherical_subtype)&&(_jointMode!=sim_jointmode_force) )
    {
        retVal=_CJoint_::setHybridFunctionality(h);
        if (h)
        {
            _CJoint_::setEnableDynamicMotor(true);
            _CJoint_::setEnableDynamicMotorControlLoop(true);
            if (_jointType==sim_joint_revolute_subtype)
            {
                _CJoint_::setScrewPitch(0.0f);
// REMOVED FOLLOWING ON 24/7/2015: causes problem when switching modes. The physics engine plugin will now not set limits if the range>=360
//          _jointPositionRange=tt::getLimitedFloat(0.0f,piValTimes2_f,_jointPositionRange);
//          setPosition(getPosition()); // to make sure it is within range (will also get/set velocity)
            }
        }
    }
    return(retVal);
}

void CJoint::getDynamicJointErrors(float& linear,float& angular) const
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

void CJoint::getDynamicJointErrorsFull(C3Vector& linear,C3Vector& angular) const
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

bool CJoint::setBulletFloatParams(const std::vector<float>& pp)
{ // Overridden from _CJoint_
    std::vector<float> p(pp);
    tt::limitValue(0.0f,1.0f,p[simi_bullet_joint_stoperp]); // stop ERP
    tt::limitValue(0.0f,100.0f,p[simi_bullet_joint_stopcfm]); // stop CFM
    tt::limitValue(0.0f,100.0f,p[simi_bullet_joint_normalcfm]); // normal CFM
    bool retVal=_CJoint_::setBulletFloatParams(p);
    return(retVal);
}

bool CJoint::setOdeFloatParams(const std::vector<float>& pp)
{ // Overridden from _CJoint_
    std::vector<float> p(pp);
    tt::limitValue(0.0f,1.0f,p[simi_ode_joint_stoperp]); // stop ERP
    tt::limitValue(0.0f,100.0f,p[simi_ode_joint_stopcfm]); // stop CFM
    tt::limitValue(0.0f,100.0f,p[simi_ode_joint_bounce]); // bounce
    tt::limitValue(0.0f,10.0f,p[simi_ode_joint_fudgefactor]); // fudge factor
    tt::limitValue(0.0f,100.0f,p[simi_ode_joint_normalcfm]); // normal CFM
    bool retVal=_CJoint_::setOdeFloatParams(p);
    return(retVal);
}

bool CJoint::setVortexFloatParams(const std::vector<float>& pp)
{ // Overridden from _CJoint_
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
    bool retVal=_CJoint_::setVortexFloatParams(p);
    return(retVal);
}

bool CJoint::setNewtonFloatParams(const std::vector<float>& pp)
{ // Overridden from _CJoint_
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
    bool retVal=_CJoint_::setNewtonFloatParams(p);
    return(retVal);
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

bool CJoint::setDynamicMotorTargetVelocity(float v)
{ // Overridden from _CJoint_
    bool retVal=false;
    if (_jointType!=sim_joint_spherical_subtype)
    {
        if (_jointType==sim_joint_revolute_subtype)
            v=tt::getLimitedFloat(-90000.0f*degToRad_f,+90000.0f*degToRad_f,v); // 250 rot/sec --> 0.25 rot/1ms
        if (_jointType==sim_joint_prismatic_subtype)
            v=tt::getLimitedFloat(-100.0f,+100.0f,v); // 100 m/sec --> 0.1 m/1ms
        retVal=_CJoint_::setDynamicMotorTargetVelocity(v);
    }
    return(retVal);
}

bool CJoint::setDynamicMotorUpperLimitVelocity(float v)
{ // Overridden from _CJoint_
    bool retVal=false;
    if (_jointType!=sim_joint_spherical_subtype)
    {
        v=tt::getLimitedFloat(0.01f,20.0f,v);
        retVal=_CJoint_::setDynamicMotorUpperLimitVelocity(v);
    }
    return(retVal);
}

bool CJoint::setDynamicMotorMaximumForce(float f)
{ // Overridden from _CJoint_
    bool retVal=false;
    if (_jointType!=sim_joint_spherical_subtype)
    {
        if (_jointType==sim_joint_revolute_subtype)
            f=tt::getLimitedFloat(0.0f,+100000000000.0f,f);
        if (_jointType==sim_joint_prismatic_subtype)
            f=tt::getLimitedFloat(0.0f,+10000000000.0f,f);
        retVal=_CJoint_::setDynamicMotorMaximumForce(f);
    }
    return(retVal);
}

bool CJoint::setDynamicMotorPositionControlParameters(float p_param,float i_param,float d_param)
{ // Overridden from _CJoint_
    bool retVal=false;
    p_param=tt::getLimitedFloat(-1000.0f,1000.0f,p_param);
    i_param=tt::getLimitedFloat(-1000.0f,1000.0f,i_param);
    d_param=tt::getLimitedFloat(-1000.0f,1000.0f,d_param);
    retVal=_CJoint_::setDynamicMotorPositionControlParameters(p_param,i_param,d_param);
    return(retVal);
}

bool CJoint::setDynamicMotorSpringControlParameters(float k_param,float c_param)
{ // Overridden from _CJoint_
    bool retVal=false;
    float maxVal=+10000000000.0f;
    if (_jointType==sim_joint_revolute_subtype)
        maxVal=+100000000000.0f;
    k_param=tt::getLimitedFloat(-maxVal,maxVal,k_param);
    c_param=tt::getLimitedFloat(-maxVal,maxVal,c_param);
    retVal=_CJoint_::setDynamicMotorSpringControlParameters(k_param,c_param);
    return(retVal);
}

bool CJoint::setDynamicMotorPositionControlTargetPosition(float pos)
{ // Overridden from _CJoint_
    bool retVal=false;
    if (_jointType!=sim_joint_spherical_subtype)
    {
        if ( (_jointType==sim_joint_revolute_subtype)&&_positionIsCyclic )
            pos=tt::getNormalizedAngle(pos);
        retVal=_CJoint_::setDynamicMotorPositionControlTargetPosition(pos);
    }
    return(retVal);
}

void CJoint::setDynamicMotorReflectedPosition_useOnlyFromDynamicPart(float rfp)
{
    {   // When the joint is in dynamic mode we disable the joint limits and allow a cyclic behaviour (revolute joints)
        // This is because dynamic joints can over or undershoot limits.
        // So we set the position directly, without checking for limits.
        // Turn count is taken care by the physics plugin.
        _CJoint_::setPosition(rfp);
    }
    _rectifyDependentJoints(false);
}

bool CJoint::setDependencyJointHandle(int depJointID)
{ // Overridden from _CJoint_
    bool retVal=false;
    if ( (_jointType!=sim_joint_spherical_subtype)&&(getJointMode()==sim_jointmode_dependent) )
    {
        retVal=_CJoint_::setDependencyJointHandle(depJointID);
        if (depJointID==-1)
            App::currentWorld->sceneObjects->actualizeObjectInformation();
        else
        { // enable it
            // We now check for an illegal loop:
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(depJointID);
            CJoint* iterat=it;
            while (iterat->getDependencyJointHandle()!=-1)
            {
                if (iterat->getJointMode()!=_jointMode)
                    break; // We might have a loop, but it is interupted by another jointMode!! (e.g. IK dependency VS direct dependency)
                int joint=iterat->getDependencyJointHandle();
                if (joint==getObjectHandle())
                { // We have an illegal loop! We disable it:
                    iterat->setDependencyJointHandle(-1);
                    break;
                }
                iterat=App::currentWorld->sceneObjects->getJointFromHandle(joint);
            }
            App::currentWorld->sceneObjects->actualizeObjectInformation();
            setPosition(getPosition());
        }
    }
    return(retVal);
}

void CJoint::_setDependencyJointHandle_send(int depJointID) const
{ // Overridden from _CJoint_
    _CJoint_::_setDependencyJointHandle_send(depJointID);

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

bool CJoint::setDependencyJointMult(float coeff)
{ // Overridden from _CJoint_
    bool retVal=false;
    if (_jointType!=sim_joint_spherical_subtype)
    {
        coeff=tt::getLimitedFloat(-10000.0f,10000.0f,coeff);
        retVal=_CJoint_::setDependencyJointMult(coeff);
        setPosition(getPosition());
    }
    return(retVal);
}

void CJoint::_setDependencyJointMult_send(float coeff) const
{ // Overridden from _CJoint_
    _CJoint_::_setDependencyJointMult_send(coeff);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
    {
        int dep=-1;
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_dependencyJointHandle);
        if (it!=nullptr)
            dep=it->getIkPluginCounterpartHandle();
        CPluginContainer::ikPlugin_setJointDependency(_ikPluginCounterpartHandle,dep,_dependencyJointOffset,coeff);
    }
}

bool CJoint::setDependencyJointOffset(float off)
{ // Overridden from _CJoint_
    bool retVal=false;
    if (_jointType!=sim_joint_spherical_subtype)
    {
        off=tt::getLimitedFloat(-10000.0f,10000.0f,off);
        retVal=_CJoint_::setDependencyJointOffset(off);
        setPosition(getPosition());
    }
    return(retVal);
}

void CJoint::_setDependencyJointOffset_send(float off) const
{ // Overridden from _CJoint_
    _CJoint_::_setDependencyJointOffset_send(off);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
    {
        int dep=-1;
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_dependencyJointHandle);
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

void CJoint::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationIsRunning);
    _initialValuesInitialized=simulationIsRunning;
    _dynamicSecondPartIsValid=false; // do the same as for force sensors here?! (if the joint is copied while apart, paste it apart too!)
    _previousJointPositionIsValid=false;
    _measuredJointVelocity_velocityMeasurement=0.0f;
    _previousJointPosition_velocityMeasurement=0.0f;
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
}

void CJoint::simulationAboutToStart()
{
    initializeInitialValues(true);
    CSceneObject::simulationAboutToStart();
}

void CJoint::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::currentWorld->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
    {
        _CJoint_::setPosition(_initialPosition);
        _CJoint_::setSphericalTransformation(_initialSphericalJointTransformation);

        _CJoint_::setEnableDynamicMotor(_initialDynamicMotorEnabled);
        _CJoint_::setDynamicMotorTargetVelocity(_initialDynamicMotorTargetVelocity);
        _CJoint_::setDynamicMotorLockModeWhenInVelocityControl(_initialDynamicMotorLockModeWhenInVelocityControl);
        _CJoint_::setDynamicMotorUpperLimitVelocity(_initialDynamicMotorUpperLimitVelocity);
        _CJoint_::setDynamicMotorMaximumForce(_initialDynamicMotorMaximumForce);

        _CJoint_::setEnableDynamicMotorControlLoop(_initialDynamicMotorControlLoopEnabled);
        _CJoint_::setDynamicMotorPositionControlParameters(_initialDynamicMotorPositionControl_P,_initialDynamicMotorPositionControl_I,_initialDynamicMotorPositionControl_D);
        _CJoint_::setDynamicMotorSpringControlParameters(_initialDynamicMotorSpringControl_K,_initialDynamicMotorSpringControl_C);
        _CJoint_::setDynamicMotorPositionControlTargetPosition(_initialTargetPosition);

        _CJoint_::setJointMode(_initialJointMode);
        _CJoint_::setHybridFunctionality(_initialHybridOperation);

        _explicitHandling_DEPRECATED=_initialExplicitHandling_DEPRECATED;
        _velocity_DEPRECATED=_initialVelocity_DEPRECATED;
        _targetVelocity_DEPRECATED=_initialTargetVelocity_DEPRECATED;
    }
    _CJoint_::setDynamicSecondPartIsValid(false);
    _initialValuesInitialized=false;

    _averageForceOrTorqueValid=false;
    _cumulatedForceOrTorque=0.0f;
    _lastForceOrTorqueValid_dynStep=false;
    _lastForceOrTorque_dynStep=0.0f;
    _cumulativeForceOrTorqueTmp=0.0f;

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
        setPosition(_initialPosition);
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

float CJoint::getPosition_useTempValues() const
{
    return(_jointPosition_tempForIK);
}

float CJoint::getMeasuredJointVelocity() const
{
    return(_measuredJointVelocity_velocityMeasurement);
}

std::string CJoint::getDependencyJointLoadName() const
{
    return(_dependencyJointLoadName);
}

int CJoint::getJointCallbackCallOrder_backwardCompatibility() const
{
    return(_jointCallbackCallOrder_backwardCompatibility);
}

void CJoint::setDirectDependentJoints(const std::vector<CJoint*>& joints)
{
    _directDependentJoints.assign(joints.begin(),joints.end());
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

bool CJoint::setScrewPitch(float pitch)
{ // Overridden from _CJoint_
    bool retVal=false;
    if (_jointType==sim_joint_revolute_subtype)
    {
        if (_jointMode!=sim_jointmode_force)
        { // no pitch when in torque/force mode
            pitch=tt::getLimitedFloat(-10.0f,10.0f,pitch);
            if (pitch!=0.0f)
                _CJoint_::setHybridFunctionality(false);
            retVal=_CJoint_::setScrewPitch(pitch);
        }
    }
    return(retVal);
}

void CJoint::_setScrewPitch_send(float pitch) const
{ // Overridden from _CJoint_
    _CJoint_::_setScrewPitch_send(pitch);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setJointScrewPitch(_ikPluginCounterpartHandle,_screwPitch);
}

bool CJoint::setPositionIntervalMin(float min)
{ // Overridden from _CJoint_
    bool retVal=false;
    if (_jointType!=sim_joint_spherical_subtype)
    {
        if (_jointType==sim_joint_revolute_subtype)
            min=tt::getLimitedFloat(-100000.0f,100000.0f,min);
        if (_jointType==sim_joint_prismatic_subtype)
            min=tt::getLimitedFloat(-1000.0f,1000.0f,min);
        retVal=_CJoint_::setPositionIntervalMin(min);

        setPosition(getPosition());
    }
    return(retVal);
}

void CJoint::_setPositionIntervalMin_send(float min) const
{ // Overridden from _CJoint_
    _CJoint_::_setPositionIntervalMin_send(min);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setJointInterval(_ikPluginCounterpartHandle,_positionIsCyclic,_jointMinPosition,_jointPositionRange);
}

bool CJoint::setPositionIntervalRange(float range)
{ // Overridden from _CJoint_
    bool retVal=false;
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
    retVal=_CJoint_::setPositionIntervalRange(range);
    setPosition(getPosition());
    setSphericalTransformation(getSphericalTransformation());
    return(retVal);
}

void CJoint::_setPositionIntervalRange_send(float range) const
{ // Overridden from _CJoint_
    _CJoint_::_setPositionIntervalRange_send(range);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setJointInterval(_ikPluginCounterpartHandle,_positionIsCyclic,_jointMinPosition,_jointPositionRange);
}

bool CJoint::setLength(float l)
{ // Overridden from _CJoint_
    tt::limitValue(0.001f,1000.0f,l);
    bool retVal=_CJoint_::setLength(l);
    return(retVal);
}

bool CJoint::setDiameter(float d)
{ // Overridden from _CJoint_
    tt::limitValue(0.0001f,100.0f,d);
    bool retVal=_CJoint_::setDiameter(d);
    return(retVal);
}

void CJoint::scaleObject(float scalingFactor)
{
    _CJoint_::setDiameter(_diameter*scalingFactor);
    _CJoint_::setLength(_length*scalingFactor);
    _CJoint_::setScrewPitch(_screwPitch*scalingFactor);
    if (_jointType==sim_joint_prismatic_subtype)
    {
        _CJoint_::setPosition(_jointPosition*scalingFactor);
        _jointPositionForMotionHandling_DEPRECATED*=scalingFactor;
        _CJoint_::setPositionIntervalMin(_jointMinPosition*scalingFactor);
        _CJoint_::setPositionIntervalRange(_jointPositionRange*scalingFactor);
        _CJoint_::setDependencyJointOffset(_dependencyJointOffset*scalingFactor);
        _CJoint_::setMaxStepSize(_maxStepSize*scalingFactor);
        _CJoint_::setDynamicMotorPositionControlTargetPosition(_dynamicMotorPositionControl_targetPosition*scalingFactor);

        _CJoint_::setDynamicMotorSpringControlParameters(_dynamicMotorSpringControl_K*scalingFactor*scalingFactor,_dynamicMotorSpringControl_C*scalingFactor*scalingFactor);

        _CJoint_::setDynamicMotorTargetVelocity(_dynamicMotorTargetVelocity*scalingFactor);
        _CJoint_::setDynamicMotorUpperLimitVelocity(_dynamicMotorUpperLimitVelocity*scalingFactor);
        if (_dynamicMotorPositionControl_torqueModulation) // this condition and next line added on 04/10/2013 (Alles Gute zu Geburtstag Mama :) )
            _CJoint_::setDynamicMotorMaximumForce(_dynamicMotorMaximumForce*scalingFactor*scalingFactor); //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
        else
            _CJoint_::setDynamicMotorMaximumForce(_dynamicMotorMaximumForce*scalingFactor*scalingFactor*scalingFactor); //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

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
        _CJoint_::setDynamicMotorMaximumForce(_dynamicMotorMaximumForce*scalingFactor*scalingFactor*scalingFactor*scalingFactor);//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

        _CJoint_::setDynamicMotorSpringControlParameters(_dynamicMotorSpringControl_K*scalingFactor*scalingFactor*scalingFactor*scalingFactor,_dynamicMotorSpringControl_C*scalingFactor*scalingFactor*scalingFactor*scalingFactor);

        if (_initialValuesInitialized)
        {
            _initialDynamicMotorMaximumForce*=scalingFactor*scalingFactor*scalingFactor*scalingFactor;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
            // following 2 new since 7/5/2014:
            _initialDynamicMotorSpringControl_K*=scalingFactor*scalingFactor*scalingFactor*scalingFactor;
            _initialDynamicMotorSpringControl_C*=scalingFactor*scalingFactor*scalingFactor*scalingFactor;
        }
    }

    CSceneObject::scaleObject(scalingFactor);

    // We have to reconstruct a part of the dynamics world:
    _dynamicsFullRefreshFlag=true;

    _lastForceOrTorqueValid_dynStep=false;
    _lastForceOrTorque_dynStep=0.0f;
    _averageForceOrTorqueValid=false;
    _cumulatedForceOrTorque=0.0f;
}

void CJoint::scaleObjectNonIsometrically(float x,float y,float z)
{
    float diam=sqrt(x*y);
    _CJoint_::setDiameter(_diameter*diam);
    _CJoint_::setLength(_length*z);
    _CJoint_::setScrewPitch(_screwPitch*z);
    if (_jointType==sim_joint_prismatic_subtype)
    {
        _CJoint_::setPosition(_jointPosition*z);
        _CJoint_::setPositionIntervalMin(_jointMinPosition*z);
        _CJoint_::setPositionIntervalRange(_jointPositionRange*z);
        _CJoint_::setDependencyJointOffset(_dependencyJointOffset*z);
        _CJoint_::setMaxStepSize(_maxStepSize*z);
        _CJoint_::setDynamicMotorPositionControlTargetPosition(_dynamicMotorPositionControl_targetPosition*z);

        _CJoint_::setDynamicMotorSpringControlParameters(_dynamicMotorSpringControl_K*diam*diam,_dynamicMotorSpringControl_C*diam*diam);

        _CJoint_::setDynamicMotorTargetVelocity(_dynamicMotorTargetVelocity*z);
        _CJoint_::setDynamicMotorUpperLimitVelocity(_dynamicMotorUpperLimitVelocity*z);
        if (_dynamicMotorPositionControl_torqueModulation) // this condition and next line added on 04/10/2013 (Alles Gute zu Geburtstag Mama :) )
            _CJoint_::setDynamicMotorMaximumForce(_dynamicMotorMaximumForce*diam*diam); //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
        else
            _CJoint_::setDynamicMotorMaximumForce(_dynamicMotorMaximumForce*diam*diam*diam); //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

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
        _CJoint_::setDynamicMotorMaximumForce(_dynamicMotorMaximumForce*diam*diam*diam*diam);//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

        _CJoint_::setDynamicMotorSpringControlParameters(_dynamicMotorSpringControl_K*diam*diam*diam*diam,_dynamicMotorSpringControl_C*diam*diam*diam*diam);

        if (_initialValuesInitialized)
        {
            _initialDynamicMotorMaximumForce*=diam*diam*diam*diam;//*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
            // following 2 new since 7/5/2014:
            _initialDynamicMotorSpringControl_K*=diam*diam*diam*diam;
            _initialDynamicMotorSpringControl_C*=diam*diam*diam*diam;
        }
    }

    CSceneObject::scaleObjectNonIsometrically(diam,diam,z);

    // We have to reconstruct a part of the dynamics world:
    _dynamicsFullRefreshFlag=true;
}

void CJoint::addCumulativeForceOrTorque(float forceOrTorque,int countForAverage)
{ // New routine since 1/6/2011. The countForAverage mechanism is needed because we need to average all values in a simulation time step (but this is called every dynamic simulation time step!!)
    _cumulativeForceOrTorqueTmp+=forceOrTorque;
    _lastForceOrTorqueValid_dynStep=true;
    _lastForceOrTorque_dynStep=forceOrTorque;
    if (countForAverage>0)
    {
        _averageForceOrTorqueValid=true;
        _cumulatedForceOrTorque=_cumulativeForceOrTorqueTmp/float(countForAverage);
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
        CLuaScriptObject* script=App::currentWorld->luaScriptContainer->getScriptFromObjectAttachedTo_child(getObjectHandle());
        if (script!=nullptr)
        {
            if (!script->getContainsJointCallbackFunction())
                script=nullptr;
        }
        CLuaScriptObject* cScript=App::currentWorld->luaScriptContainer->getScriptFromObjectAttachedTo_customization(getObjectHandle());
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

bool CJoint::setPositionIsCyclic(bool isCyclic)
{ // Overridden from _CJoint_
    bool retVal=false;
    if (isCyclic)
    {
        if (getJointType()==sim_joint_revolute_subtype)
        {
            _CJoint_::setScrewPitch(0.0f);
            _CJoint_::setPositionIntervalMin(-piValue_f);
            _CJoint_::setPositionIntervalRange(piValTimes2_f);
        }
    }
    retVal=_CJoint_::setPositionIsCyclic(isCyclic);
    setVelocity_DEPRECATED(getVelocity_DEPRECATED()); // To make sure velocity is within allowed range
    return(retVal);
}

void CJoint::_setPositionIsCyclic_send(bool isCyclic) const
{ // Overridden from _CJoint_
    _CJoint_::_setPositionIsCyclic_send(isCyclic);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setJointInterval(_ikPluginCounterpartHandle,_positionIsCyclic,_jointMinPosition,_jointPositionRange);
}

void CJoint::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
    _CJoint_::setDependencyJointHandle(-1);
}

CSceneObject* CJoint::copyYourself()
{
    CJoint* newJoint=(CJoint*)CSceneObject::copyYourself();
    newJoint->_dependencyJointHandle=_dependencyJointHandle; // important for copy operations connections
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

    _colorPart1.copyYourselfInto(&newJoint->_colorPart1);
    _colorPart2.copyYourselfInto(&newJoint->_colorPart2);

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
            _colorPart1.serialize(ar,0);
            if (ar.setWritingMode())
                _colorPart1.serialize(ar,0);

            ar.storeDataName("Cl2");
            ar.setCountingMode();
            _colorPart2.serialize(ar,0);
            if (ar.setWritingMode())
                _colorPart2.serialize(ar,0);

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
            ar << _dependencyJointHandle << _dependencyJointMult << _dependencyJointOffset;
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
                        _colorPart1.serialize(ar,0);
                    }
                    if (theName.compare("Cl2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        _colorPart2.serialize(ar,0);
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
                        if ( _jointHasHybridFunctionality&&(App::userSettings->xrTest==123456789) )
                            App::logMsg(sim_verbosity_errors,"Joint has hybrid functionality...");
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
                        if (App::userSettings->xrTest==123456789)
                        {
                            if ( (_jointMode!=sim_jointmode_passive)&&(_jointMode!=sim_jointmode_dependent)&&(_jointMode!=sim_jointmode_force) )
                                App::logMsg(sim_verbosity_errors,"Joint has deprecated mode...");
                        }
                    }
                    if (theName.compare("Jdt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dependencyJointHandle >> _dependencyJointMult >> _dependencyJointOffset;
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
            { // on 29/08/2013 we corrected all default lights. So we need to correct for that change:
                CTTUtil::scaleColorUp_(_colorPart1.getColorsPtr());
                CTTUtil::scaleColorUp_(_colorPart2.getColorsPtr());
            }
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

            ar.xmlPushNewNode("color");
            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("part1");
                _colorPart1.serialize(ar,0);
                ar.xmlPopNode();
                ar.xmlPushNewNode("part2");
                _colorPart2.serialize(ar,0);
                ar.xmlPopNode();
            }
            else
            {
                int rgb[3];
                for (size_t l=0;l<3;l++)
                    rgb[l]=int(_colorPart1.getColorsPtr()[l]*255.1f);
                ar.xmlAddNode_ints("part1",rgb,3);
                for (size_t l=0;l<3;l++)
                    rgb[l]=int(_colorPart2.getColorsPtr()[l]*255.1f);
                ar.xmlAddNode_ints("part2",rgb,3);
            }
            ar.xmlPopNode();

            ar.xmlPushNewNode("ik");
            ar.xmlAddNode_float("maxStepSize",_maxStepSize*mult);
            ar.xmlAddNode_float("weight",_ikWeight);
            ar.xmlPopNode();

            ar.xmlPushNewNode("dependency");
            if (exhaustiveXml)
                ar.xmlAddNode_int("jointHandle",_dependencyJointHandle);
            else
            {
                std::string str;
                CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(_dependencyJointHandle);
                if (it!=nullptr)
                    str=it->getObjectName();
                ar.xmlAddNode_string("dependentJoint",str.c_str());
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

            if (ar.xmlPushChildNode("color",exhaustiveXml))
            {
                if (exhaustiveXml)
                {
                    if (ar.xmlPushChildNode("part1"))
                    {
                        _colorPart1.serialize(ar,0);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("part2"))
                    {
                        _colorPart2.serialize(ar,0);
                        ar.xmlPopNode();
                    }
                }
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("part1",rgb,3,exhaustiveXml))
                        _colorPart1.setColor(float(rgb[0])/255.0f,float(rgb[1])/255.0f,float(rgb[2])/255.0f,sim_colorcomponent_ambient_diffuse);
                    if (ar.xmlGetNode_ints("part2",rgb,3,exhaustiveXml))
                        _colorPart2.setColor(float(rgb[0])/255.0f,float(rgb[1])/255.0f,float(rgb[2])/255.0f,sim_colorcomponent_ambient_diffuse);
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
                    ar.xmlGetNode_int("jointHandle",_dependencyJointHandle);
                else
                    ar.xmlGetNode_string("dependentJoint",_dependencyJointLoadName,exhaustiveXml);
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
        }
    }
}

void CJoint::serializeWExtIk(CExtIkSer& ar)
{
    CSceneObject::serializeWExtIk(ar);

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

    ar.writeInt(_dependencyJointHandle);
    ar.writeFloat(_dependencyJointMult);
    ar.writeFloat(_dependencyJointOffset);
}

void CJoint::performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    CSceneObject::performObjectLoadingMapping(map,loadingAmodel);
    _dependencyJointHandle=CWorld::getLoadingMapping(map,_dependencyJointHandle);
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

bool CJoint::setJointMode(int theMode)
{ // Overriden from _CJoint_
    bool retVal=setJointMode_noDynMotorTargetPosCorrection(theMode);
    if ( retVal&&(theMode==sim_jointmode_force)&&_dynamicMotorControlLoopEnabled )
    { // Make sure the target position is the same here (otherwise big jump)
        setDynamicMotorPositionControlTargetPosition(getPosition());
    }
    return(retVal);
}

bool CJoint::setJointMode_noDynMotorTargetPosCorrection(int theMode)
{
    bool retVal=false;
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
        _CJoint_::setHybridFunctionality(false);
        _CJoint_::setScrewPitch(0.0f);
// REMOVED FOLLOWING ON 24/7/2015: causes problem when switching modes. The physics engine plugin will now not set limits if the range>=360
//      if (_jointType==sim_joint_revolute_subtype)
//          _jointPositionRange=tt::getLimitedFloat(0.0f,piValTimes2_f,_jointPositionRange); // new since 18/11/2012 (was forgotten)
        if (_jointType==sim_joint_spherical_subtype)
            _CJoint_::setPositionIntervalRange(piValue_f);

        App::currentWorld->sceneObjects->actualizeObjectInformation();
        md=theMode;
    }
    if (theMode==sim_jointmode_ik_deprecated)
    {
        App::currentWorld->sceneObjects->actualizeObjectInformation();
        md=theMode;
    }
    retVal=_CJoint_::setJointMode(md);
    setPosition(getPosition());
    return(retVal);
}

void CJoint::_setJointMode_send(int theMode) const
{ // Overridden from _CJoint_
    _CJoint_::_setJointMode_send(theMode);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setJointMode(_ikPluginCounterpartHandle,theMode);
}

void CJoint::_rectifyDependentJoints(bool useTempValues)
{   // We rectify now all joints linked to that one (rewritten on 2009-01-27):
    for (size_t i=0;i<_directDependentJoints.size();i++)
    {
        if ((_directDependentJoints[i]->getJointMode()==sim_jointmode_dependent)||(_directDependentJoints[i]->getJointMode()==sim_jointmode_reserved_previously_ikdependent)) // second part on 3/7/2014
        {
            if (useTempValues)
                _directDependentJoints[i]->setPosition_useTempValues(0.0f); // value doesn't matter!
            else
                _directDependentJoints[i]->setPosition(0.0f); // value doesn't matter!
        }
    }
}

bool CJoint::setSphericalTransformation(const C4Vector& tr)
{ // Overridden from _CJoint_
    bool retVal=false;
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
    retVal=_CJoint_::setSphericalTransformation(transf);
    return(retVal);
}

void CJoint::_setSphericalTransformation_send(const C4Vector& tr) const
{ // Overridden from _CJoint_
    _CJoint_::_setSphericalTransformation_send(tr);

    // Synchronize with IK plugin:
    if ( (_ikPluginCounterpartHandle!=-1)&&(_jointType==sim_joint_spherical_subtype) )
        CPluginContainer::ikPlugin_setSphericalJointQuaternion(_ikPluginCounterpartHandle,tr);
}

bool CJoint::setMaxStepSize(float stepS)
{ // Overridden from _CJoint_
    bool retVal=false;
    if (_jointType==sim_joint_revolute_subtype)
        tt::limitValue(0.01f*degToRad_f,100000.0f,stepS); // high number for screws!
    if (_jointType==sim_joint_prismatic_subtype)
        tt::limitValue(0.0001f,1000.0f,stepS);
    if (_jointType==sim_joint_spherical_subtype)
        tt::limitValue(0.01f*degToRad_f,piValue_f,stepS);
    retVal=_CJoint_::setMaxStepSize(stepS);
    return(retVal);
}

void CJoint::_setMaxStepSize_send(float stepS) const
{ // Overridden from _CJoint_
    _CJoint_::_setMaxStepSize_send(stepS);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setJointMaxStepSize(_ikPluginCounterpartHandle,_maxStepSize);
}

bool CJoint::setIkWeight(float newWeight)
{ // Overridden from _CJoint_
    bool retVal=false;
    newWeight=tt::getLimitedFloat(-1.0f,1.0f,newWeight);
    retVal=_CJoint_::setIkWeight(newWeight);
    return(retVal);
}

void CJoint::_setIkWeight_send(float newWeight) const
{ // Overridden from _CJoint_
    _CJoint_::_setIkWeight_send(newWeight);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setJointIkWeight(_ikPluginCounterpartHandle,_ikWeight);
}

bool CJoint::setPosition(float pos)
{ // Overriden from _CJoint_
    bool retVal=false;
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
        if (_dependencyJointHandle!=-1)
        {
            CJoint* anAct=App::currentWorld->sceneObjects->getJointFromHandle(_dependencyJointHandle);
            if (anAct!=nullptr)
                linked=_dependencyJointMult*anAct->getPosition();
        }
        pos=linked+_dependencyJointOffset;
    }
    retVal=_CJoint_::setPosition(pos);

    _rectifyDependentJoints(false);
    setVelocity_DEPRECATED(getVelocity_DEPRECATED());
    return(retVal);
}

void CJoint::_setPosition_send(float pos) const
{ // Overriden from _CJoint_
    _CJoint_::_setPosition_send(pos);
    // Synchronize with IK plugin:
    if ( (_ikPluginCounterpartHandle!=-1)&&(_jointType!=sim_joint_spherical_subtype) )
        CPluginContainer::ikPlugin_setJointPosition(_ikPluginCounterpartHandle,pos);
}

void CJoint::setPosition_useTempValues(float pos)
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
        if (_dependencyJointHandle!=-1)
        {
            CJoint* anAct=App::currentWorld->sceneObjects->getJointFromHandle(_dependencyJointHandle);
            if (anAct!=nullptr)
                linked=_dependencyJointMult*anAct->getPosition_useTempValues();
        }
        pos=linked+_dependencyJointOffset;
    }
    _jointPosition_tempForIK=pos;

    _rectifyDependentJoints(true);
}

void CJoint::initializeParametersForIK()
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
            if (_jointPositionRange/2.0f<angle)
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

int CJoint::getDoFs() const
{
    int retVal=1;
    if (_jointType==sim_joint_spherical_subtype)
        retVal=3;;
    return(retVal);
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
            C7Vector tr2(getFullLocalTransformation());
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
            mTr=getLocalTransformation();
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
        C7Vector tr1(getLocalTransformation());
        C7Vector tr2(getFullLocalTransformation_ikOld());
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
    bool retVal=CSceneObject::announceObjectWillBeErased(objectHandle,copyBuffer);
    if (_dependencyJointHandle==objectHandle)
        _CJoint_::setDependencyJointHandle(-1);
    if (_vortexIntParams[5]==objectHandle) // that's the Vortex dependency joint
    {
        std::vector<int> ip;
        getVortexIntParams(ip);
        ip[5]=-1;
        _CJoint_::setVortexIntParams(ip);
    }
    if (_newtonIntParams[1]==objectHandle) // that's the Vortex dependency joint
    {
        std::vector<int> ip;
        getNewtonIntParams(ip);
        ip[1]=-1;
        _CJoint_::setNewtonIntParams(ip);
    }

    // We check if the joint is listed in the _directDependentJoints:
    for (size_t i=0;i<_directDependentJoints.size();i++)
    {
        if (_directDependentJoints[i]->getObjectHandle()==objectHandle)
            _directDependentJoints.erase(_directDependentJoints.begin()+i);
    }
    return(retVal);
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

        // Build remote joint:
        sendInt32(_jointType,sim_syncobj_sceneobject_create);

        // Update the remote sceneObject:
        CSceneObject::buildUpdateAndPopulateSynchronizationObject(parentRouting);

        // Update the remote joint:
        _setPositionIntervalMin_send(_jointMinPosition);
        _setPositionIntervalRange_send(_jointPositionRange);
        _setPositionIsCyclic_send(_positionIsCyclic);
        _setDiameter_send(_diameter);
        _setLength_send(_length);
        _setScrewPitch_send(_screwPitch);
        // _setDependencyJointHandle_send(_dependencyJointHandle);
        // _setDependencyJointMult_send(_dependencyJointMult);
        // _setDependencyJointOffset_send(_dependencyJointOffset);
        _setIkWeight_send(_ikWeight);
        _setMaxStepSize_send(_maxStepSize);
        _setPosition_send(_jointPosition);
        _setSphericalTransformation_send(_sphericalTransformation);
        _setJointMode_send(_jointMode);
        _setEnableDynamicMotor_send(_dynamicMotorEnabled);
        _setHybridFunctionality_send(_jointHasHybridFunctionality);
        _setDynamicMotorTargetVelocity_send(_dynamicMotorTargetVelocity);
        _setDynamicMotorUpperLimitVelocity_send(_dynamicMotorUpperLimitVelocity);
        _setDynamicMotorPositionControlTargetPosition_send(_dynamicMotorPositionControl_targetPosition);
        _setDynamicMotorPositionControlParameters_send(_dynamicMotorPositionControl_P,_dynamicMotorPositionControl_I,_dynamicMotorPositionControl_D);
        _setDynamicMotorSpringControlParameters_send(_dynamicMotorSpringControl_K,_dynamicMotorSpringControl_C);
        _setDynamicMotorMaximumForce_send(_dynamicMotorMaximumForce);
        _setEnableDynamicMotorControlLoop_send(_dynamicMotorControlLoopEnabled);
        _setEnableTorqueModulation_send(_dynamicMotorPositionControl_torqueModulation);
        _setDynamicMotorLockModeWhenInVelocityControl_send(_dynamicLockModeWhenInVelocityControl);
        _setDynamicSecondPartIsValid_send(_dynamicSecondPartIsValid);
        _setDynamicSecondPartLocalTransform_send(_dynamicSecondPartLocalTransform);
        _setBulletFloatParams_send(_bulletFloatParams);
        _setBulletIntParams_send(_bulletIntParams);
        _setOdeFloatParams_send(_odeFloatParams);
        _setOdeIntParams_send(_odeIntParams);
        _setVortexFloatParams_send(_vortexFloatParams);
        _setVortexIntParams_send(_vortexIntParams);
        _setNewtonFloatParams_send(_newtonFloatParams);
        _setNewtonIntParams_send(_newtonIntParams);

        _colorPart1.buildUpdateAndPopulateSynchronizationObject(getSyncMsgRouting());
        _colorPart2.buildUpdateAndPopulateSynchronizationObject(getSyncMsgRouting());

        // Update other objects:
    }
}

void CJoint::connectSynchronizationObject()
{ // Overridden from CSceneObject
    if (getObjectCanSync())
    {
        CSceneObject::connectSynchronizationObject();

        _setDependencyJointHandle_send(_dependencyJointHandle);
        _setDependencyJointMult_send(_dependencyJointMult);
        _setDependencyJointOffset_send(_dependencyJointOffset);
    }
}
