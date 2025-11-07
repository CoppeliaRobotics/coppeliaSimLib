#include <jointObject.h>
#include <tt.h>
#include <simInternal.h>
#include <linMotionRoutines.h>
#include <simStrings.h>
#include <utils.h>
#include <app.h>
#include <simFlavor.h>
#include <engineProperties.h>
#ifdef SIM_WITH_GUI
#include <jointRendering.h>
#endif

static std::string OBJECT_META_INFO = R"(
{
    "namespaces": {
        "refs": {"newPropertyForcedType": "sim.propertytype_handlearray"},
        "origRefs": {"newPropertyForcedType": "sim.propertytype_handlearray"},
        "customData": {},
        "signal": {}
    },
    "methods": {
    }
}
)";

CJoint::CJoint()
{
    _commonInit();
    computeBoundingBox();
}

CJoint::CJoint(int jointType)
{
    _commonInit();
    _jointType = jointType;
    if (jointType == sim_joint_revolute)
    {
        _objectName_old = IDSOGL_REVOLUTE_JOINT;
        _objectAlias = IDSOGL_REVOLUTE_JOINT;
        _jointMode = sim_jointmode_dynamic;
        _isCyclic = true;
        _posRange = piValT2;
        _posMin = -piValue;
        _maxStepSize_old = 10.0 * degToRad;
        _targetForce = 2.5; // 0.25 m x 1kg x 9.81
        _maxAcceleration_DEPRECATED = 60.0 * degToRad;
        _maxVelAccelJerk[0] = piValT2;
        _maxVelAccelJerk[1] = piValT2;
        _maxVelAccelJerk[2] = piValT2;
    }
    if (jointType == sim_joint_prismatic)
    {
        _objectName_old = IDSOGL_PRISMATIC_JOINT;
        _objectAlias = IDSOGL_PRISMATIC_JOINT;
        _jointMode = sim_jointmode_dynamic;
        _isCyclic = false;
        _posRange = 1.0;
        _posMin = -0.5;
        _maxStepSize_old = 0.1;
        _targetForce = 50.0; // 5kg x 9.81
        _maxAcceleration_DEPRECATED = 0.1;
        _maxVelAccelJerk[0] = 1.0;
        _maxVelAccelJerk[1] = 1.0;
        _maxVelAccelJerk[2] = 1.0;
    }
    if (jointType == sim_joint_spherical)
    {
        _objectName_old = IDSOGL_SPHERICAL_JOINT;
        _objectAlias = IDSOGL_SPHERICAL_JOINT;
        _jointMode = sim_jointmode_dynamic;
        _isCyclic = true;
        _posRange = piValue;
        _posMin = 0.0;
        _maxStepSize_old = 10.0 * degToRad;
        _targetForce = 0.0;
        _maxAcceleration_DEPRECATED = 60.0 * degToRad;
        _maxVelAccelJerk[0] = piValT2;
        _maxVelAccelJerk[1] = piValT2;
        _maxVelAccelJerk[2] = piValT2;
    }
    _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());
    computeBoundingBox();
}

void CJoint::_commonInit()
{
    _objectType = sim_sceneobject_joint;
    _localObjectSpecialProperty = 0;

    _maxVelAccelJerk[0] = piValT2;
    _maxVelAccelJerk[1] = piValT2;
    _maxVelAccelJerk[2] = piValT2;

    _jointType = sim_joint_revolute;
    _screwLead = 0.0;
    _sphericalTransf.setIdentity();
    _pos = 0.0;
    _targetPos = 0.0;
    _targetVel = 0.0;
    _enforceLimits = false;

    _jointMode = sim_jointmode_kinematic;
    _dependencyMasterJointHandle = -1;
    _dependencyJointMult = 1.0;
    _dependencyJointOffset = 0.0;

    _intrinsicTransformationError.setIdentity();

    _dynCtrlMode = sim_jointdynctrl_free;
    _dynPositionCtrlType = 0; // engine velocity mode + Pos PID, by default
    _dynVelocityCtrlType = 0; // engine velocity mode, by default
    _motorLock = false;
    _targetForce = 1000.0; // This value has to be adjusted according to the joint type
    _dynCtrl_kc[0] = 0.1;
    _dynCtrl_kc[1] = 0.0;

    _jointHasHybridFunctionality = false;

    // Bullet parameters
    // ----------------------------------------------------
    _bulletFloatParams.push_back(0.2); // simi_bullet_joint_stoperp
    _bulletFloatParams.push_back(0.0); // simi_bullet_joint_stopcfm
    _bulletFloatParams.push_back(0.0); // simi_bullet_joint_normalcfm
    _bulletFloatParams.push_back(0.0); // free but 0.0 by default
    _bulletFloatParams.push_back(0.0); // free but 0.0 by default
    _bulletFloatParams.push_back(0.1); // simi_bullet_joint_pospid1
    _bulletFloatParams.push_back(0.0); // simi_bullet_joint_pospid2
    _bulletFloatParams.push_back(0.0); // simi_bullet_joint_pospid3

    _bulletIntParams.push_back(0); // Free
    // ----------------------------------------------------

    // ODE parameters
    // ----------------------------------------------------
    _odeFloatParams.push_back(0.6);     // simi_ode_joint_stoperp
    _odeFloatParams.push_back(0.00001); // simi_ode_joint_stopcfm
    _odeFloatParams.push_back(0.0);     // simi_ode_joint_bounce
    _odeFloatParams.push_back(1.0);     // simi_ode_joint_fudgefactor
    _odeFloatParams.push_back(0.00001); // simi_ode_joint_normalcfm
    _odeFloatParams.push_back(0.1);     // simi_ode_joint_pospid1
    _odeFloatParams.push_back(0.0);     // simi_ode_joint_pospid2
    _odeFloatParams.push_back(0.0);     // simi_ode_joint_pospid3

    _odeIntParams.push_back(0); // Free
    // ----------------------------------------------------

    // Vortex parameters:
    // ----------------------------------------------------
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_lowerlimitdamping
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_upperlimitdamping
    _vortexFloatParams.push_back(DBL_MAX); // simi_vortex_joint_lowerlimitstiffness
    _vortexFloatParams.push_back(DBL_MAX); // simi_vortex_joint_upperlimitstiffness
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_lowerlimitrestitution
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_upperlimitrestitution
    _vortexFloatParams.push_back(DBL_MAX); // simi_vortex_joint_lowerlimitmaxforce
    _vortexFloatParams.push_back(DBL_MAX); // simi_vortex_joint_upperlimitmaxforce
    _vortexFloatParams.push_back(0.001);   // simi_vortex_joint_motorconstraintfrictioncoeff
    _vortexFloatParams.push_back(10.0);    // simi_vortex_joint_motorconstraintfrictionmaxforce
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_motorconstraintfrictionloss
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_p0loss
    _vortexFloatParams.push_back(DBL_MAX); // simi_vortex_joint_p0stiffness
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_p0damping
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_p0frictioncoeff
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_p0frictionmaxforce
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_p0frictionloss
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_p1loss
    _vortexFloatParams.push_back(DBL_MAX); // simi_vortex_joint_p1stiffness
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_p1damping
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_p1frictioncoeff
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_p1frictionmaxforce
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_p1frictionloss
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_p2loss
    _vortexFloatParams.push_back(DBL_MAX); // simi_vortex_joint_p2stiffness
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_p2damping
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_p2frictioncoeff
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_p2frictionmaxforce
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_p2frictionloss
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_a0loss
    _vortexFloatParams.push_back(DBL_MAX); // simi_vortex_joint_a0stiffness
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_a0damping
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_a0frictioncoeff
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_a0frictionmaxforce
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_a0frictionloss
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_a1loss
    _vortexFloatParams.push_back(DBL_MAX); // simi_vortex_joint_a1stiffness
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_a1damping
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_a1frictioncoeff
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_a1frictionmaxforce
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_a1frictionloss
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_a2loss
    _vortexFloatParams.push_back(DBL_MAX); // simi_vortex_joint_a2stiffness
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_a2damping
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_a2frictioncoeff
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_a2frictionmaxforce
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_a2frictionloss
    _vortexFloatParams.push_back(0.0);     // deprecated. simi_vortex_joint_dependencyfactor
    _vortexFloatParams.push_back(0.0);     // deprecated. simi_vortex_joint_dependencyoffset
    _vortexFloatParams.push_back(0.0);     // free but 0.0 by default
    _vortexFloatParams.push_back(0.0);     // free but 0.0 by default
    _vortexFloatParams.push_back(0.1);     // simi_vortex_joint_pospid1
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_pospid2
    _vortexFloatParams.push_back(0.0);     // simi_vortex_joint_pospid3

    _vortexIntParams.push_back(simi_vortex_joint_proportionalmotorfriction); // simi_vortex_joint_bitcoded
    _vortexIntParams.push_back(0);                                           // simi_vortex_joint_relaxationenabledbc. 1 bit per dof
    _vortexIntParams.push_back(0);                                           // simi_vortex_joint_frictionenabledbc. 1 bit per dof
    _vortexIntParams.push_back(1 + 2 + 4 + 8 + 16 + 32);                     // simi_vortex_joint_frictionproportionalbc. 1 bit per dof
    _vortexIntParams.push_back(-1);                                          // deprecated. simi_vortex_joint_objectid
    _vortexIntParams.push_back(-1);                                          // deprecated. simi_vortex_joint_dependentobjectid
    _vortexIntParams.push_back(0);                                           // reserved for future ext.
    // ----------------------------------------------------

    // Newton parameters
    // ----------------------------------------------------
    _newtonFloatParams.push_back(0.0); // deprecated. simi_newton_joint_dependencyfactor
    _newtonFloatParams.push_back(0.0); // deprecated. simi_newton_joint_dependencyoffset
    _newtonFloatParams.push_back(0.1); // simi_newton_joint_pospid1
    _newtonFloatParams.push_back(0.0); // simi_newton_joint_pospid2
    _newtonFloatParams.push_back(0.0); // simi_newton_joint_pospid3

    _newtonIntParams.push_back(-1); // deprecated. simi_newton_joint_objectid. The ID is redefined in each session
    _newtonIntParams.push_back(-1); // deprecated. simi_newton_joint_dependentobjectid
    // ----------------------------------------------------

    // Mujoco parameters
    // ----------------------------------------------------
    _mujocoFloatParams.push_back(0.02);  // sim_mujoco_joint_solreflimit1
    _mujocoFloatParams.push_back(1.0);   // sim_mujoco_joint_solreflimit2
    _mujocoFloatParams.push_back(0.9);   // sim_mujoco_joint_solimplimit1
    _mujocoFloatParams.push_back(0.95);  // sim_mujoco_joint_solimplimit2
    _mujocoFloatParams.push_back(0.001); // sim_mujoco_joint_solimplimit3
    _mujocoFloatParams.push_back(0.5);   // sim_mujoco_joint_solimplimit4
    _mujocoFloatParams.push_back(2.0);   // sim_mujoco_joint_solimplimit5
    _mujocoFloatParams.push_back(0.0);   // sim_mujoco_joint_frictionloss
    _mujocoFloatParams.push_back(0.02);  // sim_mujoco_joint_solreffriction1
    _mujocoFloatParams.push_back(1.0);   // sim_mujoco_joint_solreffriction2
    _mujocoFloatParams.push_back(0.9);   // sim_mujoco_joint_solimpfriction1
    _mujocoFloatParams.push_back(0.95);  // sim_mujoco_joint_solimpfriction2
    _mujocoFloatParams.push_back(0.001); // sim_mujoco_joint_solimpfriction3
    _mujocoFloatParams.push_back(0.5);   // sim_mujoco_joint_solimpfriction4
    _mujocoFloatParams.push_back(2.0);   // sim_mujoco_joint_solimpfriction5
    _mujocoFloatParams.push_back(0.0);   // sim_mujoco_joint_stiffness
    _mujocoFloatParams.push_back(0.0);   // sim_mujoco_joint_damping
    _mujocoFloatParams.push_back(0.0);   // sim_mujoco_joint_springref
    _mujocoFloatParams.push_back(0.0);   // sim_mujoco_joint_springdamper1
    _mujocoFloatParams.push_back(0.0);   // sim_mujoco_joint_springdamper2
    _mujocoFloatParams.push_back(0.0);   // sim_mujoco_joint_armature (changed from 2 to 0.0 on 17.02.2025. changed from 0.02 to 2 on 18.11.2024)
    _mujocoFloatParams.push_back(0.0);   // sim_mujoco_joint_margin
    _mujocoFloatParams.push_back(0.0);   // deprecated. sim_mujoco_joint_polycoef1
    _mujocoFloatParams.push_back(0.0);   // deprecated. sim_mujoco_joint_polycoef2
    _mujocoFloatParams.push_back(0.0);   // sim_mujoco_joint_polycoef3
    _mujocoFloatParams.push_back(0.0);   // sim_mujoco_joint_polycoef4
    _mujocoFloatParams.push_back(0.0);   // sim_mujoco_joint_polycoef5
    _mujocoFloatParams.push_back(0.1);   // simi_mujoco_joint_pospid1
    _mujocoFloatParams.push_back(0.0);   // simi_mujoco_joint_pospid2
    _mujocoFloatParams.push_back(0.0);   // simi_mujoco_joint_pospid3

    _mujocoIntParams.push_back(-1); // deprecated. sim_mujoco_joint_objectid. The ID is redefined in each session
    _mujocoIntParams.push_back(-1); // deprecated. sim_mujoco_joint_dependentobjectid
    // ----------------------------------------------------

    _ikWeight_old = 1.0;
    _diameter = 0.02;
    _length = 0.15;

    _isCyclic = true;
    _posRange = piValT2;
    _posMin = -piValue;
    _maxStepSize_old = 10.0 * degToRad;

    _visibilityLayer = JOINT_LAYER;
    _objectAlias = getObjectTypeInfo();
    _objectName_old = getObjectTypeInfo();
    _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());

    _cumulativeForceOrTorqueTmp = 0.0;
    _lastForceOrTorque_dynStep = 0.0;
    _lastForceOrTorqueValid_dynStep = false;
    _filteredForceOrTorque = 0.0;
    _filteredForceOrTorqueValid = false;
    _kinematicMotionType = 0;
    _kinematicMotionInitVel = 0.0;
    _velCalc_vel = 0.0;
    _velCalc_prevPosValid = false;
    _jointPositionForMotionHandling_DEPRECATED = _pos;
    _velocity_DEPRECATED = 0.0;
    _jointCallbackCallOrder_backwardCompatibility = 0;
    _explicitHandling_DEPRECATED = false;
    _unlimitedAcceleration_DEPRECATED = false;
    _invertTargetVelocityAtLimits_DEPRECATED = true;
    _maxAcceleration_DEPRECATED = 60.0 * degToRad;

    _color.setDefaultValues();
    _color.setColor(1.0f, 0.3f, 0.1f, sim_colorcomponent_ambient_diffuse);
    _color_removeSoon.setDefaultValues();
    _color_removeSoon.setColor(0.22f, 0.22f, 0.22f, sim_colorcomponent_ambient_diffuse);
}

CJoint::~CJoint()
{
}

void CJoint::setHybridFunctionality_old(bool h)
{
    if (((_jointType != sim_joint_spherical) && (_jointMode != sim_jointmode_dynamic)) || (!h))
    {
        bool diff = (_jointHasHybridFunctionality != h);
        if (diff)
        {
            _jointHasHybridFunctionality = h;
            if (h)
            {
                setDynCtrlMode(sim_jointdynctrl_positioncb);
                if (_jointType == sim_joint_revolute)
                    setScrewLead(0.0);
            }
        }
    }
}

void CJoint::getDynamicJointErrors(double& linear, double& angular) const
{
    linear = 0.0;
    angular = 0.0;
    if (_jointType == sim_joint_revolute)
    {
        linear = _intrinsicTransformationError.X.getLength();
        angular = C3Vector::unitZVector.getAngle(_intrinsicTransformationError.Q.getMatrix().axis[2]);
    }
    if (_jointType == sim_joint_prismatic)
    {
        linear = _intrinsicTransformationError.X.getLength();
        angular = _intrinsicTransformationError.Q.getAngleBetweenQuaternions(C4Vector::identityRotation);
    }
    if (_jointType == sim_joint_spherical)
        linear = _intrinsicTransformationError.X.getLength();
}

void CJoint::getDynamicJointErrorsFull(C3Vector& linear, C3Vector& angular) const
{
    linear.clear();
    angular.clear();
    if (_jointType == sim_joint_revolute)
    {
        linear = _intrinsicTransformationError.X;
        angular = _intrinsicTransformationError.Q.getEulerAngles();
        angular(2) = 0.0;
    }
    if (_jointType == sim_joint_prismatic)
    {
        linear = _intrinsicTransformationError.X;
        linear(2) = 0.0;
        angular = _intrinsicTransformationError.Q.getEulerAngles();
    }
    if (_jointType == sim_joint_spherical)
        linear = _intrinsicTransformationError.X;
}

bool CJoint::setEngineFloatParam_old(int what, double v)
{
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_float, indexWithArrays);
    if (prop.size() > 0)
    {
        if (setFloatProperty(prop.c_str(), v) > 0)
            return true;
    }
    prop = _enumToProperty(what, sim_propertytype_vector2, indexWithArrays);
    if (prop.size() > 0)
    {
        double w[2];
        if (getVector2Property(prop.c_str(), w) > 0)
        {
            w[indexWithArrays] = v;
            if (setVector2Property(prop.c_str(), w) > 0)
                return true;
        }
    }
    prop = _enumToProperty(what, sim_propertytype_vector3, indexWithArrays);
    if (prop.size() > 0)
    {
        C3Vector w;
        if (getVector3Property(prop.c_str(), w) > 0)
        {
            w(indexWithArrays) = v;
            if (setVector3Property(prop.c_str(), w) > 0)
                return true;
        }
    }
    prop = _enumToProperty(what, sim_propertytype_floatarray, indexWithArrays);
    if (prop.size() > 0)
    {
        std::vector<double> w;
        if (getFloatArrayProperty(prop.c_str(), w) > 0)
        {
            w[indexWithArrays] = v;
            if (setFloatArrayProperty(prop.c_str(), w.data(), indexWithArrays + 1) > 0)
                return true;
        }
    }

    if (what == sim_vortex_joint_dependencyoffset)
    {
        setDependencyParams(v, _dependencyJointMult);
        return (true);
    }
    if (what == sim_vortex_joint_dependencyfactor)
    {
        setDependencyParams(_dependencyJointOffset, v);
        return (true);
    }
    if (what == sim_newton_joint_dependencyoffset)
    {
        setDependencyParams(v, _dependencyJointMult);
        return (true);
    }
    if (what == sim_newton_joint_dependencyfactor)
    {
        setDependencyParams(_dependencyJointOffset, v);
        return (true);
    }
    if (what == sim_mujoco_joint_polycoef1)
    {
        setDependencyParams(v, _dependencyJointMult);
        return (true);
    }
    if (what == sim_mujoco_joint_polycoef2)
    {
        setDependencyParams(_dependencyJointOffset, v);
        return (true);
    }
    return false;
}

bool CJoint::setEngineIntParam_old(int what, int v)
{
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_int, indexWithArrays);
    if (prop.size() > 0)
    {
        if (setIntProperty(prop.c_str(), v) > 0)
            return true;
    }
    return false;
}

bool CJoint::setEngineBoolParam_old(int what, bool v)
{
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_bool, indexWithArrays);
    if (prop.size() > 0)
    {
        if (setBoolProperty(prop.c_str(), v) > 0)
            return true;
    }
    return false;
}

void CJoint::copyEnginePropertiesTo(CJoint* target)
{
    target->_bulletFloatParams.assign(_bulletFloatParams.begin(), _bulletFloatParams.end());
    target->_bulletIntParams.assign(_bulletIntParams.begin(), _bulletIntParams.end());

    target->_odeFloatParams.assign(_odeFloatParams.begin(), _odeFloatParams.end());
    target->_odeIntParams.assign(_odeIntParams.begin(), _odeIntParams.end());

    target->_vortexFloatParams.assign(_vortexFloatParams.begin(), _vortexFloatParams.end());
    target->_vortexIntParams.assign(_vortexIntParams.begin(), _vortexIntParams.end());

    target->_newtonFloatParams.assign(_newtonFloatParams.begin(), _newtonFloatParams.end());
    target->_newtonIntParams.assign(_newtonIntParams.begin(), _newtonIntParams.end());

    target->_mujocoFloatParams.assign(_mujocoFloatParams.begin(), _mujocoFloatParams.end());
    target->_mujocoIntParams.assign(_mujocoIntParams.begin(), _mujocoIntParams.end());
}

void CJoint::setTargetVelocity(double v)
{
    if (_jointType != sim_joint_spherical)
    {
        bool diff = (_targetVel != v);
        if (diff)
        {
            _targetVel = v;
            if (_isInScene && App::worldContainer->getEventsEnabled())
            {
                const char* cmd = propJoint_targetVel.name;
                CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
                ev->appendKeyDouble(cmd, _targetVel);
                App::worldContainer->pushEvent();
            }
            if (_targetVel * _targetForce < 0.0)
                setTargetForce(-_targetForce, true);
        }
    }
}

void CJoint::setTargetForce(double f, bool isSigned)
{
    if (_jointType != sim_joint_spherical)
    {
        if (!isSigned)
        {
            if (f < 0.0)
                f = 0.0;
            if (f * _targetVel < 0.0)
                f = -f;
        }
        bool diff = (_targetForce != f);
        if (diff)
        {
            _targetForce = f;
            if (_isInScene && App::worldContainer->getEventsEnabled())
            {
                const char* cmd = propJoint_targetForce.name;
                CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
                ev->appendKeyDouble(cmd, _targetForce);
                App::worldContainer->pushEvent();
            }
            if (f * _targetVel < 0.0)
                setTargetVelocity(-_targetVel);
        }
    }
}

void CJoint::setPid_old(double p_param, double i_param, double d_param)
{ // old, back-compatibility function
    C3Vector w(p_param, i_param, d_param);
    setVector3Property(propJoint_bulletPosPid.name, w);
    setVector3Property(propJoint_odePosPid.name, w);
    setVector3Property(propJoint_vortexPosPid.name, w);
    setVector3Property(propJoint_newtonPosPid.name, w);
    // Not for Mujoco! Why not?
}

void CJoint::setKc(double k_param, double c_param)
{
    double maxVal = +10000000000.0;
    if (_jointType == sim_joint_revolute)
        maxVal = +100000000000.0;
    k_param = tt::getLimitedFloat(-maxVal, maxVal, k_param);
    c_param = tt::getLimitedFloat(-maxVal, maxVal, c_param);
    bool diff = (_dynCtrl_kc[0] != k_param) || (_dynCtrl_kc[1] != c_param);
    if (diff)
    {
        _dynCtrl_kc[0] = k_param;
        _dynCtrl_kc[1] = c_param;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propJoint_springDamperParams.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDoubleArray(cmd, _dynCtrl_kc, 2);
            App::worldContainer->pushEvent();
        }
    }
}

void CJoint::setTargetPosition(double pos)
{
    if (_jointType != sim_joint_spherical)
    {
        if ((_jointType == sim_joint_revolute) && _isCyclic)
            pos = tt::getNormalizedAngle(pos);
        bool diff = (_targetPos != pos);
        if (diff)
        {
            _targetPos = pos;
            if (_isInScene && App::worldContainer->getEventsEnabled())
            {
                const char* cmd = propJoint_targetPos.name;
                CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
                ev->appendKeyDouble(cmd, _targetPos);
                App::worldContainer->pushEvent();
            }
        }
    }
}

void CJoint::setDynamicMotorReflectedPosition_useOnlyFromDynamicPart(double rfp, double simTime)
{
    { // When the joint is in dynamic mode we disable the joint limits and allow a cyclic behaviour (revolute joints)
        // This is because dynamic joints can over or undershoot limits.
        // So we set the position directly, without checking for limits.
        // Turn count is taken care by the physics plugin.
        setPosition(rfp, nullptr, !_enforceLimits);
    }
    measureJointVelocity(simTime);
}

void CJoint::setDependencyMasterJointHandle(int depJointID)
{
    bool diff = (_dependencyMasterJointHandle != depJointID);
    if (diff)
    {
        _dependencyMasterJointHandle = depJointID;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propJoint_dependencyMaster.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyInt(cmd, _dependencyMasterJointHandle);
            App::worldContainer->pushEvent();
        }
        App::currentWorld->sceneObjects->actualizeObjectInformation();
        _setDependencyJointHandle_sendOldIk(_dependencyMasterJointHandle);

        if (_dependencyMasterJointHandle == -1)
            setPosition(getPosition());
        else
        {
            // Illegal loop check:
            CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(_dependencyMasterJointHandle);
            CJoint* iterat = it;
            while (iterat->getDependencyMasterJointHandle() != -1)
            {
                int joint = iterat->getDependencyMasterJointHandle();
                if (joint == getObjectHandle())
                {
                    iterat->setDependencyMasterJointHandle(-1);
                    break;
                }
                iterat = App::currentWorld->sceneObjects->getJointFromHandle(joint);
            }
            updateSelfAsSlave();
        }
#if SIM_EVENT_PROTOCOL_VERSION == 2
        _sendDependencyChange_old();
#endif
    }
}

void CJoint::_sendDependencyChange_old() const
{
    if (_isInScene && App::worldContainer->getEventsEnabled())
    {
        const char* cmd = "dependency";
        CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
        ev->openKeyMap(cmd);
        long long int mast = -1;
        if (_dependencyMasterJointHandle != -1)
        {
            CSceneObject* master = App::currentWorld->sceneObjects->getJointFromHandle(_dependencyMasterJointHandle);
            if (master != nullptr)
                mast = master->getObjectUid();
        }
        ev->appendKeyInt("masterUid", mast);
        ev->appendKeyDouble("mult", _dependencyJointMult);
        ev->appendKeyDouble("off", _dependencyJointOffset);
        App::worldContainer->pushEvent();
    }
}

void CJoint::_setDependencyJointHandle_sendOldIk(int depJointID) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle != -1)
    {
        int dep = -1;
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(depJointID);
        if (it != nullptr)
            dep = it->getIkPluginCounterpartHandle();
        App::worldContainer->pluginContainer->oldIkPlugin_setJointDependency(
            _ikPluginCounterpartHandle, dep, _dependencyJointOffset, _dependencyJointMult);
    }
}

void CJoint::_setDependencyJointMult_sendOldIk(double coeff) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle != -1)
    {
        int dep = -1;
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(_dependencyMasterJointHandle);
        if (it != nullptr)
            dep = it->getIkPluginCounterpartHandle();
        App::worldContainer->pluginContainer->oldIkPlugin_setJointDependency(_ikPluginCounterpartHandle, dep, _dependencyJointOffset, coeff);
    }
}

void CJoint::setDependencyParams(double off, double mult)
{
    if (_jointType != sim_joint_spherical)
    {
        off = tt::getLimitedFloat(-10000.0, 10000.0, off);
        mult = tt::getLimitedFloat(-10000.0, 10000.0, mult);
        bool diff = ((_dependencyJointOffset != off) || (_dependencyJointMult != mult));
        if (diff)
        {
            _dependencyJointOffset = off;
            _dependencyJointMult = mult;
            if (_isInScene && App::worldContainer->getEventsEnabled())
            {
                const char* cmd = propJoint_dependencyParams.name;
                CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
                double arr[2] = {_dependencyJointOffset, _dependencyJointMult};
                ev->appendKeyDoubleArray(cmd, arr, 2);
                App::worldContainer->pushEvent();
            }
#if SIM_EVENT_PROTOCOL_VERSION == 2
            _sendDependencyChange_old();
#endif
            _setDependencyJointOffset_sendOldIk(off);
            _setDependencyJointMult_sendOldIk(mult);
            updateSelfAsSlave();
        }
    }
}

void CJoint::_setDependencyJointOffset_sendOldIk(double off) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle != -1)
    {
        int dep = -1;
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(_dependencyMasterJointHandle);
        if (it != nullptr)
            dep = it->getIkPluginCounterpartHandle();
        App::worldContainer->pluginContainer->oldIkPlugin_setJointDependency(_ikPluginCounterpartHandle, dep, _dependencyJointOffset, _dependencyJointMult);
    }
}

void CJoint::measureJointVelocity(double simTime)
{
    if (_jointType != sim_joint_spherical)
    {
        double dt = simTime - _velCalc_prevSimTime;
        if (_velCalc_prevPosValid && (dt > 0.00001))
        {
            double v;
            if (_isCyclic)
                v = tt::getAngleMinusAlpha(_pos, _velCalc_prevPos) / dt;
            else
                v = (_pos - _velCalc_prevPos) / dt;
            setVelocity(v);
        }
        _velCalc_prevPos = _pos;
        _velCalc_prevSimTime = simTime;
        _velCalc_prevPosValid = true;
    }
}

void CJoint::setVelocity(double v, const CJoint* masterJoint /*=nullptr*/)
{ // sets the velocity, and overrides next velocity measurement in measureJointVelocity
    double newVel = _velCalc_vel;
    if (masterJoint != nullptr)
    {
        if (_dependencyMasterJointHandle == masterJoint->getObjectHandle())
        {
            newVel = _dependencyJointMult * masterJoint->getMeasuredJointVelocity();
            _velCalc_prevPosValid = false;
        }
    }
    else
    {
        if (_dependencyMasterJointHandle == -1)
        {
            newVel = v;
            _velCalc_prevPosValid = false; // if false, will use _velCalc_vel as current vel in sim.getJointVelocity
        }
    }
    if (newVel != _velCalc_vel)
    {
        _velCalc_vel = newVel;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propJoint_calcVelocity.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _velCalc_vel);
            App::worldContainer->pushEvent();
        }
    }
    // Handle dependent joints:
    for (size_t i = 0; i < _directDependentJoints.size(); i++)
        _directDependentJoints[i]->setVelocity(0.0, this);
}

void CJoint::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    setVelocity(0.0);
    _dynCtrl_previousVelForce[0] = 0.0;
    _dynCtrl_previousVelForce[1] = 0.0;

    _dynPosCtrl_currentVelAccel[0] = 0.0;
    _dynPosCtrl_currentVelAccel[1] = 0.0;
    _dynVelCtrl_currentVelAccel[0] = 0.0;
    _dynVelCtrl_currentVelAccel[1] = 0.0;

    _initialPosition = _pos;
    _initialSphericalJointTransformation = _sphericalTransf;
    setIntrinsicTransformationError(C7Vector::identityTransformation);
    _initialTargetPosition = _targetPos;
    _initialTargetVelocity = _targetVel;

    _initialJointMode = _jointMode;

    _initialDynCtrlMode = _dynCtrlMode;
    _initialDynVelocityCtrlType = _dynVelocityCtrlType;
    _initialDynPositionCtrlType = _dynPositionCtrlType;
    _initialDynCtrl_lockAtVelZero = _motorLock;
    _initialDynCtrl_force = _targetForce;
    _initialDynCtrl_kc[0] = _dynCtrl_kc[0];
    _initialDynCtrl_kc[1] = _dynCtrl_kc[1];

    _initialHybridOperation = _jointHasHybridFunctionality;

    _setFilteredForceOrTorque(false);
    _setForceOrTorque(false);
    _kinematicMotionType = 0;
    _kinematicMotionInitVel = 0.0;
    _cumulativeForceOrTorqueTmp = 0.0;

    _jointPositionForMotionHandling_DEPRECATED = _pos;
    _velocity_DEPRECATED = 0.0;
    _initialVelocity_DEPRECATED = _velocity_DEPRECATED;
    _initialExplicitHandling_DEPRECATED = _explicitHandling_DEPRECATED;

    _initialMaxVelAccelJerk[0] = _maxVelAccelJerk[0];
    _initialMaxVelAccelJerk[1] = _maxVelAccelJerk[1];
    _initialMaxVelAccelJerk[2] = _maxVelAccelJerk[2];

    warningAboutMujocoLoopClosureProblemsIssued = false;
}

void CJoint::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void CJoint::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
    // ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0)
        {
            setPosition(_initialPosition);
            setSphericalTransformation(_initialSphericalJointTransformation);
            setTargetPosition(_initialTargetPosition);
            setTargetVelocity(_initialTargetVelocity);

            setJointMode(_initialJointMode);
            setDynCtrlMode(_initialDynCtrlMode);
            setDynVelCtrlType(_initialDynVelocityCtrlType);
            setDynPosCtrlType(_initialDynPositionCtrlType);

            setMotorLock(_initialDynCtrl_lockAtVelZero);
            setTargetForce(_initialDynCtrl_force, true);

            setKc(_initialDynCtrl_kc[0], _initialDynCtrl_kc[1]);

            setHybridFunctionality_old(_initialHybridOperation);

            _explicitHandling_DEPRECATED = _initialExplicitHandling_DEPRECATED;
            _velocity_DEPRECATED = _initialVelocity_DEPRECATED;

            setMaxVelAccelJerk(_initialMaxVelAccelJerk);
        }
    }

    _setFilteredForceOrTorque(false);
    _setForceOrTorque(false);
    _cumulativeForceOrTorqueTmp = 0.0;
    setIntrinsicTransformationError(C7Vector::identityTransformation);
    CSceneObject::simulationEnded();
}

// FOLLOWING FUNCTIONS ARE DEPRECATED:
//----------------------------------------
void CJoint::resetJoint_DEPRECATED()
{ // DEPRECATED
    if ((_jointMode != sim_jointmode_motion_deprecated) ||
        (!App::currentWorld->mainSettings_old->jointMotionHandlingEnabled_DEPRECATED))
        return;
    if (_initialValuesInitialized)
    {
        setPosition(_initialPosition);
        setVelocity_DEPRECATED(_initialVelocity_DEPRECATED);
    }
}

void CJoint::handleJoint_DEPRECATED(double deltaTime)
{ // DEPRECATED
    if ((_jointMode != sim_jointmode_motion_deprecated) ||
        (!App::currentWorld->mainSettings_old->jointMotionHandlingEnabled_DEPRECATED))
        return;
    if (_unlimitedAcceleration_DEPRECATED)
    {
        _velocity_DEPRECATED = _targetVel;
        if (_velocity_DEPRECATED != 0.0)
        {
            double newPos = _jointPositionForMotionHandling_DEPRECATED;
            if (!_isCyclic)
            {
                if (_invertTargetVelocityAtLimits_DEPRECATED)
                {
                    double cycleTime = 2.0 * _posRange / _velocity_DEPRECATED;
                    deltaTime = robustMod(deltaTime, cycleTime);
                    while (true)
                    {
                        _velocity_DEPRECATED = _targetVel;
                        double absDist = _posMin + _posRange - newPos;
                        if (_velocity_DEPRECATED < 0.0)
                            absDist = newPos - _posMin;
                        if (absDist > fabs(_velocity_DEPRECATED) * deltaTime)
                        {
                            newPos += _velocity_DEPRECATED * deltaTime;
                            break; // We reached the desired deltaTime
                        }
                        if (_velocity_DEPRECATED < 0.0)
                            newPos -= absDist;
                        else
                            newPos += absDist;
                        deltaTime -= absDist / fabs(_velocity_DEPRECATED);
                        _targetVel *= -1.0; // We invert the target velocity
                    }
                }
                else
                {
                    newPos += _velocity_DEPRECATED * deltaTime;
                    double dv = newPos - (_posMin + _posRange);
                    double dl = _posMin - newPos;
                    if ((dl >= 0.0) || (dv >= 0.0))
                        _velocity_DEPRECATED = 0.0;
                }
            }
            else
                newPos += _velocity_DEPRECATED * deltaTime;
            setPosition(newPos);
            _jointPositionForMotionHandling_DEPRECATED = getPosition();
        }
    }
    else
    { // Acceleration is not infinite!
        double newPos = double(_jointPositionForMotionHandling_DEPRECATED);
        double minV = -DBL_MAX;
        double maxV = +DBL_MAX;
        if (!_isCyclic)
        {
            minV = _posMin;
            maxV = _posMin + _posRange;
            // Make sure we are within limits:
            double m = double(CLinMotionRoutines::getMaxVelocityAtPosition(newPos, _maxAcceleration_DEPRECATED, minV,
                                                                           maxV, 0.0, 0.0));
            tt::limitValue(-m, m, _velocity_DEPRECATED);
        }

        double velocityDouble = double(_velocity_DEPRECATED);
        double deltaTimeDouble = double(deltaTime);
        while (CLinMotionRoutines::getNextValues(newPos, velocityDouble, _targetVel, _maxAcceleration_DEPRECATED, minV,
                                                 maxV, 0.0, 0.0, deltaTimeDouble))
        {
            if (_invertTargetVelocityAtLimits_DEPRECATED)
                _targetVel *= -1.0;
            else
            {
                deltaTime = 0.0;
                deltaTimeDouble = 0.0;
                break;
            }
        }
        _velocity_DEPRECATED = double(velocityDouble);

        setPosition(double(newPos));
        _jointPositionForMotionHandling_DEPRECATED = getPosition();
    }
}
void CJoint::setExplicitHandling_DEPRECATED(bool explicitHandl)
{ // DEPRECATED
    if (_jointType == sim_joint_spherical)
        return;
    _explicitHandling_DEPRECATED = explicitHandl;
}

bool CJoint::getExplicitHandling_DEPRECATED()
{ // DEPRECATED
    return (_explicitHandling_DEPRECATED);
}

void CJoint::setUnlimitedAcceleration_DEPRECATED(bool unlimited)
{ // DEPRECATED
    if (_jointType == sim_joint_spherical)
        return;
    _unlimitedAcceleration_DEPRECATED = unlimited;
    setVelocity_DEPRECATED(getVelocity_DEPRECATED()); // To make sure velocity is within allowed range
}

bool CJoint::getUnlimitedAcceleration_DEPRECATED()
{ // DEPRECATED
    return (_unlimitedAcceleration_DEPRECATED);
}

void CJoint::setInvertTargetVelocityAtLimits_DEPRECATED(bool invert)
{ // DEPRECATED
    if (_jointType == sim_joint_spherical)
        return;
    _invertTargetVelocityAtLimits_DEPRECATED = invert;
}

bool CJoint::getInvertTargetVelocityAtLimits_DEPRECATED()
{ // DEPRECATED
    return (_invertTargetVelocityAtLimits_DEPRECATED);
}
void CJoint::setMaxAcceleration_DEPRECATED(double maxAccel)
{ // DEPRECATED
    if (_jointType == sim_joint_spherical)
        return;
    if (_jointType == sim_joint_prismatic)
        tt::limitValue(0.0001, 1000.0, maxAccel);
    else
        tt::limitValue(0.001 * degToRad, 36000.0 * degToRad, maxAccel);
    _maxAcceleration_DEPRECATED = maxAccel;
    setVelocity_DEPRECATED(getVelocity_DEPRECATED()); // To make sure velocity is within allowed range
}

double CJoint::getMaxAcceleration_DEPRECATED()
{ // DEPRECATED
    return (_maxAcceleration_DEPRECATED);
}

void CJoint::setVelocity_DEPRECATED(double vel)
{ // DEPRECATED
    if (_jointType == sim_joint_spherical)
        return;
    if ((vel != 0.0) && ((_jointType == sim_joint_prismatic) || (!_isCyclic)) &&
        (!_unlimitedAcceleration_DEPRECATED))
    { // We check which is the max allowed:
        double m = double(CLinMotionRoutines::getMaxVelocityAtPosition(_pos, _maxAcceleration_DEPRECATED, _posMin,
                                                                       _posMin + _posRange, 0.0, 0.0));
        tt::limitValue(-m, m, vel);
    }
    _velocity_DEPRECATED = vel;
}

double CJoint::getVelocity_DEPRECATED()
{ // DEPRECATED
    return (_velocity_DEPRECATED);
}
//------------------------------------------------

std::string CJoint::getObjectTypeInfo() const
{
    return "joint";
}

std::string CJoint::getObjectTypeInfoExtended() const
{
    std::string retVal(getObjectTypeInfo());
    if (_jointType == sim_joint_revolute)
    {
        if (fabs(_screwLead) < 0.0000001)
            retVal += tt::decorateString(" (", IDSOGL_REVOLUTE, ", p=");
        else
            retVal += tt::decorateString(" (", IDSOGL_SCREW, ", p=");
        retVal += utils::getAngleString(true, _pos);
    }
    if (_jointType == sim_joint_prismatic)
    {
        retVal += tt::decorateString(" (", IDSOGL_PRISMATIC, ", p=");
        retVal += utils::getPosString(true, _pos);
    }
    if (_jointType == sim_joint_spherical)
    {
        retVal += tt::decorateString(" (", IDSOGL_SPHERICAL, ", a=");
        C3Vector euler(getSphericalTransformation().getEulerAngles());
        retVal += utils::getAngleString(true, euler(0)) + ", b=" + utils::getAngleString(true, euler(1)) +
                  ", g=" + utils::getAngleString(true, euler(2));
    }
    double lin, ang;
    getDynamicJointErrors(lin, ang);
    if ((lin != 0.0) || (ang != 0.0))
    {
        retVal += ", dyn. err.: " + utils::getDoubleEString(false, lin);
        retVal += "/" + utils::getDoubleEString(false, ang * radToDeg);
    }
    retVal += ")";

    return (retVal);
}

double CJoint::getMeasuredJointVelocity() const
{
    return (_velCalc_vel);
}

std::string CJoint::getDependencyJointLoadAlias() const
{
    return (_dependencyJointLoadAlias);
}

std::string CJoint::getDependencyJointLoadName_old() const
{
    return (_dependencyJointLoadName_old);
}

int CJoint::getJointCallbackCallOrder_backwardCompatibility() const
{
    return (_jointCallbackCallOrder_backwardCompatibility);
}

void CJoint::setDirectDependentJoints(const std::vector<CJoint*>& joints)
{
    _directDependentJoints.assign(joints.begin(), joints.end());
}

void CJoint::computeBoundingBox()
{
    C3Vector maxV;
    if (_jointType != sim_joint_spherical)
    {
        maxV(0) = maxV(1) = _diameter / 2.0;
        maxV(2) = _length / 2.0;
    }
    else
    {
        maxV(0) = maxV(1) = maxV(2) = _diameter;
    }
    _setBB(C7Vector::identityTransformation, maxV);
}

void CJoint::setIsInScene(bool s)
{
    CSceneObject::setIsInScene(s);
    if (s)
        _color.setEventParams(true, _objectHandle);
    else
        _color.setEventParams(true, -1);
}

bool CJoint::setScrewLead(double lead)
{
    bool retVal = false;
    if (_jointType == sim_joint_revolute)
    {
        if (_jointMode != sim_jointmode_dynamic)
        { // no lead when in torque/force mode
            lead = tt::getLimitedFloat(-10.0 * piValT2, 10.0 * piValT2, lead);
            bool diff = (_screwLead != lead);
            if (diff)
            {
                _screwLead = lead;
                if (_isInScene && App::worldContainer->getEventsEnabled())
                {
                    const char* cmd = propJoint_screwLead.name;
                    CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
                    ev->appendKeyDouble(cmd, _screwLead);
                    double p[7];
                    getIntrinsicTransformation(true).getData(p, true);
                    ev->appendKeyDoubleArray(propJoint_intrinsicPose.name, p, 7);
                    App::worldContainer->pushEvent();
                }
                _setScrewPitch_sendOldIk(lead / piValT2);
                if (lead != 0.0)
                    setHybridFunctionality_old(false);
            }
            retVal = true;
        }
    }
    return (retVal);
}

void CJoint::_setScrewPitch_sendOldIk(double pitch) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle != -1)
        App::worldContainer->pluginContainer->oldIkPlugin_setJointScrewPitch(_ikPluginCounterpartHandle,
                                                                             _screwLead / piValT2);
}

void CJoint::setInterval(double minV, double maxV)
{ // input are min / max values
    bool diff = false;
    if (_jointType != sim_joint_spherical)
    {
        if (_jointType == sim_joint_revolute)
            minV = tt::getLimitedFloat(-100000.0, 100000.0, minV);
        if (_jointType == sim_joint_prismatic)
            minV = tt::getLimitedFloat(-1000.0, 1000.0, minV);
        diff = (_posMin != minV);
    }

    if (maxV < minV)
        maxV = minV;
    double dv = maxV - minV;

    if (_jointType == sim_joint_revolute)
        dv = tt::getLimitedFloat(0.001 * degToRad, 10000000.0 * degToRad, dv);
    if (_jointType == sim_joint_prismatic)
        dv = tt::getLimitedFloat(0.0, 1000.0, dv);
    if (_jointType == sim_joint_spherical)
    {
        if (_jointMode != sim_jointmode_dynamic)
            dv = tt::getLimitedFloat(0.001 * degToRad, 10000000.0 * degToRad, dv);
        else
            dv = piValue;
    }
    diff = diff || (_posRange != dv);

    if (diff)
    {
        _posMin = minV;
        _posRange = dv;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propJoint_interval.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
#if SIM_EVENT_PROTOCOL_VERSION == 2
            ev->appendKeyDouble("min", _posMin);
            ev->appendKeyDouble("range", _posRange);
#endif
            double arr[2] = {_posMin, _posMin + _posRange};
            ev->appendKeyDoubleArray(cmd, arr, 2);
            App::worldContainer->pushEvent();
        }
        _setPositionIntervalMin_sendOldIk(_posMin);
        _setPositionIntervalRange_sendOldIk(_posRange);
        setPosition(getPosition());
        setSphericalTransformation(getSphericalTransformation());
    }
}

void CJoint::getInterval(double& minV, double& maxV) const
{ // returns min / max values
    minV = _posMin;
    maxV = _posMin + _posRange;
}

void CJoint::_setPositionIntervalMin_sendOldIk(double min) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle != -1)
        App::worldContainer->pluginContainer->oldIkPlugin_setJointInterval(_ikPluginCounterpartHandle, _isCyclic,
                                                                           _posMin, _posRange);
}

void CJoint::_setPositionIntervalRange_sendOldIk(double range) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle != -1)
        App::worldContainer->pluginContainer->oldIkPlugin_setJointInterval(_ikPluginCounterpartHandle, _isCyclic,
                                                                           _posMin, _posRange);
}

void CJoint::setSize(double l /*= 0.0*/, double d /*= 0.0*/)
{
    if (l == 0.0)
        l = _length;
    if (d == 0.0)
        d = _diameter;
    tt::limitValue(0.001, 1000.0, l);
    tt::limitValue(0.0001, 100.0, d);
    bool diff = ((_length != l) || (_diameter != d));
    if (diff)
    {
        _length = l;
        _diameter = d;
        computeBoundingBox();
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propJoint_length.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _length);
            ev->appendKeyDouble(propJoint_diameter.name, _diameter);
            App::worldContainer->pushEvent();
        }
    }
}

void CJoint::scaleObject(double scalingFactor)
{
    setSize(_length * scalingFactor, _diameter * scalingFactor);
    setScrewLead(_screwLead * scalingFactor);
    if (_jointType == sim_joint_prismatic)
    {
        setPosition(_pos * scalingFactor);
        _jointPositionForMotionHandling_DEPRECATED *= scalingFactor;
        setInterval(_posMin * scalingFactor, (_posMin + _posRange) * scalingFactor);
        setDependencyParams(_dependencyJointOffset * scalingFactor, _dependencyJointMult);
        setMaxStepSize_old(_maxStepSize_old * scalingFactor);
        setTargetPosition(_targetPos * scalingFactor);
        setTargetVelocity(_targetVel * scalingFactor);

        setKc(_dynCtrl_kc[0] * scalingFactor * scalingFactor, _dynCtrl_kc[1] * scalingFactor * scalingFactor);

        if ((_dynCtrlMode == sim_jointdynctrl_spring) || (_dynCtrlMode == sim_jointdynctrl_springcb))
            setTargetForce(_targetForce * scalingFactor * scalingFactor, false); //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
        if ((_dynCtrlMode == sim_jointdynctrl_position) || (_dynCtrlMode == sim_jointdynctrl_positioncb))
            setTargetForce(_targetForce * scalingFactor * scalingFactor * scalingFactor, false); //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

        _maxAcceleration_DEPRECATED *= scalingFactor;
        _velocity_DEPRECATED *= scalingFactor;

        double mvaj[3] = {_maxVelAccelJerk[0] * scalingFactor, _maxVelAccelJerk[1] * scalingFactor, _maxVelAccelJerk[2] * scalingFactor};
        setMaxVelAccelJerk(mvaj);

        if (_initialValuesInitialized)
        {
            _initialPosition *= scalingFactor;
            _initialTargetPosition *= scalingFactor;
            _initialTargetVelocity *= scalingFactor;

            if ((_dynCtrlMode == sim_jointdynctrl_spring) || (_dynCtrlMode == sim_jointdynctrl_springcb))
                _initialDynCtrl_force *=
                    scalingFactor * scalingFactor; //*scalingFactor; removed one on 2010/02/17 b/c often working against
                                                   // gravity which doesn't change
            if ((_dynCtrlMode == sim_jointdynctrl_position) || (_dynCtrlMode == sim_jointdynctrl_positioncb))
                _initialDynCtrl_force *=
                    scalingFactor * scalingFactor * scalingFactor; //*scalingFactor; removed one on 2010/02/17 b/c often
                                                                   // working against gravity which doesn't change

            _initialDynCtrl_kc[0] *= scalingFactor * scalingFactor;
            _initialDynCtrl_kc[1] *= scalingFactor * scalingFactor;

            _initialVelocity_DEPRECATED *= scalingFactor;

            _initialMaxVelAccelJerk[0] *= scalingFactor;
            _initialMaxVelAccelJerk[1] *= scalingFactor;
            _initialMaxVelAccelJerk[2] *= scalingFactor;
        }
    }

    if (_jointType == sim_joint_revolute)
    {
        setTargetForce(_targetForce * scalingFactor * scalingFactor * scalingFactor * scalingFactor, false); //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change

        setKc(_dynCtrl_kc[0] * scalingFactor * scalingFactor * scalingFactor * scalingFactor,
              _dynCtrl_kc[1] * scalingFactor * scalingFactor * scalingFactor * scalingFactor);

        if (_initialValuesInitialized)
        {
            _initialDynCtrl_force *= scalingFactor * scalingFactor * scalingFactor *
                                     scalingFactor; //*scalingFactor; removed one on 2010/02/17 b/c often working
                                                    // against gravity which doesn't change
            _initialDynCtrl_kc[0] *= scalingFactor * scalingFactor * scalingFactor * scalingFactor;
            _initialDynCtrl_kc[1] *= scalingFactor * scalingFactor * scalingFactor * scalingFactor;
        }
    }

    _dynamicsResetFlag = true;
    _setForceOrTorque(false);
    _setFilteredForceOrTorque(false);

    CSceneObject::scaleObject(scalingFactor);
}

void CJoint::_setForceOrTorque(bool valid, double f /*= 0.0*/)
{
    if (!valid)
        f = 0.0;
    _lastForceOrTorqueValid_dynStep = valid;
    bool diff = (_lastForceOrTorque_dynStep != f);
    if (diff)
        _lastForceOrTorque_dynStep = f;
}

void CJoint::_setFilteredForceOrTorque(bool valid, double f /*= 0.0*/)
{
    if (!valid)
        f = 0.0;
    _filteredForceOrTorqueValid = valid;
    bool diff = (_filteredForceOrTorque != f);
    if (diff)
    {
        _filteredForceOrTorque = f;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propJoint_averageJointForce.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _filteredForceOrTorque);
            App::worldContainer->pushEvent();
        }
    }
}

void CJoint::addCumulativeForceOrTorque(double forceOrTorque, int countForAverage)
{ // The countForAverage mechanism is needed because we need to average all values in a simulation time step (but this
    // is called every dynamic simulation time step!!)
    _setForceOrTorque(true, forceOrTorque);
    _cumulativeForceOrTorqueTmp += forceOrTorque;
    if (countForAverage > 0)
    {
        _setFilteredForceOrTorque(true, _cumulativeForceOrTorqueTmp / double(countForAverage));
        _cumulativeForceOrTorqueTmp = 0.0;
    }
}

void CJoint::setForceOrTorqueNotValid()
{
    _setFilteredForceOrTorque(false);
    _setForceOrTorque(false);
}

bool CJoint::getDynamicForceOrTorque(double& forceOrTorque, bool dynamicStepValue) const
{
    if (dynamicStepValue)
    {
        if (App::currentWorld->dynamicsContainer->getCurrentlyInDynamicsCalculations())
        {
            if (!_lastForceOrTorqueValid_dynStep)
                return (false);
            forceOrTorque = _lastForceOrTorque_dynStep;
            return (true);
        }
        return (false);
    }
    else
    {
        if (!_filteredForceOrTorqueValid) //(!_dynamicSecondPartIsValid)
            return (false);
        forceOrTorque = _filteredForceOrTorque;
        return (true);
    }
}

int CJoint::handleDynJoint(int flags, const int intVals[3], double currentPosVelAccel[3], double effort, double dynStepSize, double errorV, double velAndForce[2])
{ // constant callback for every dynamically enabled joint, except for spherical joints. retVal: bit0 set: motor on,
    // bit1 set: motor locked
    // Called before a dyn step. After the step, setDynamicMotorReflectedPosition_useOnlyFromDynamicPart is called
    // flags: bit0: init (first time called), bit1: currentPosVelAccel[1] is valid, bit2: currentPosVelAccel[2] is valid, bit3: Mujoco world contains loop closures
    int loopCnt = intVals[0];
    int totalLoops = intVals[1];
    int rk4 = intVals[2];
    int retVal = 1;
    bool outputWarningAboutPossibleLoopClosureProblemsWithMujoco = false;
    if (_dynCtrlMode == sim_jointdynctrl_free)
        retVal = 0;
    else if (_dynCtrlMode == sim_jointdynctrl_force)
    {
        velAndForce[0] = 10000.0;
        velAndForce[1] = _targetForce;
        if (_targetForce < 0.0)
            velAndForce[0] = -10000.0; // make sure they have same sign
    }
    else if (_dynCtrlMode == sim_jointdynctrl_velocity)
    {
        outputWarningAboutPossibleLoopClosureProblemsWithMujoco = (flags & 8);
        if (_dynVelocityCtrlType == 0)
        { // engine internal velocity ctrl
            velAndForce[0] = _targetVel;
            velAndForce[1] = _targetForce;
            if (_motorLock && (_targetVel == 0.0))
                retVal |= 2;
        }
        if (_dynVelocityCtrlType == 1)
        { // motion profile
            if (dynStepSize != 0.0)
            {
                double dynVelCtrlCurrentVelAccel[2] = {_dynVelCtrl_currentVelAccel[0], _dynVelCtrl_currentVelAccel[1]};
                if ((_targetVel != 0.0) || (fabs(dynVelCtrlCurrentVelAccel[0]) > 0.00001) ||
                    (fabs(dynVelCtrlCurrentVelAccel[1]) > 0.00001))
                {
                    if ((fabs(dynVelCtrlCurrentVelAccel[0] - double(_targetVel)) > 0.00001) ||
                        (fabs(dynVelCtrlCurrentVelAccel[1]) > 0.00001))
                    { // target velocity has not been reached yet
                        double maxVelAccelJerk[3] = {double(_maxVelAccelJerk[0]), double(_maxVelAccelJerk[1]),
                                                     double(_maxVelAccelJerk[2])};
                        double targetVel = double(_targetVel);
                        double pos = 0.0;
                        bool sel = true;
                        int ruckObj = App::worldContainer->pluginContainer->ruckigPlugin_vel(
                            -1, 1, double(dynStepSize), -1, &pos, dynVelCtrlCurrentVelAccel,
                            dynVelCtrlCurrentVelAccel + 1, maxVelAccelJerk + 1, maxVelAccelJerk + 2, &sel, &targetVel);
                        if (ruckObj >= 0)
                        {
                            int res = App::worldContainer->pluginContainer->ruckigPlugin_step(
                                ruckObj, double(dynStepSize), &pos, dynVelCtrlCurrentVelAccel,
                                dynVelCtrlCurrentVelAccel + 1, &pos);
                            App::worldContainer->pluginContainer->ruckigPlugin_remove(ruckObj);
                            velAndForce[0] = double(dynVelCtrlCurrentVelAccel[0]);
                            velAndForce[1] = _targetForce;
                            if (velAndForce[0] * velAndForce[1] < 0.0)
                                velAndForce[1] = -velAndForce[1]; // make sure they have same sign
                        }
                    }
                }
                else
                {
                    dynVelCtrlCurrentVelAccel[0] = 0.0;
                    dynVelCtrlCurrentVelAccel[1] = 0.0;
                    if (_motorLock)
                        retVal |= 2;
                }

                velAndForce[0] = double(dynVelCtrlCurrentVelAccel[0]);
                velAndForce[1] = _targetForce;
                if (velAndForce[0] * velAndForce[1] < 0.0)
                    velAndForce[1] = -velAndForce[1]; // make sure they have same sign
                if ((rk4 == 0) || (rk4 == 4))
                {
                    _dynVelCtrl_currentVelAccel[0] = dynVelCtrlCurrentVelAccel[0];
                    _dynVelCtrl_currentVelAccel[1] = dynVelCtrlCurrentVelAccel[1];
                }
                _dynCtrl_previousVelForce[0] = velAndForce[0];
                _dynCtrl_previousVelForce[1] = velAndForce[1];
            }
            else
            { // in case of RK4, pass1 (dt=0)
                velAndForce[0] = _dynCtrl_previousVelForce[0];
                velAndForce[1] = _dynCtrl_previousVelForce[1];
            }
        }
    }
    else
    { // position, spring and callback
        if ((_dynCtrlMode == sim_jointdynctrl_position) || (_dynCtrlMode == sim_jointdynctrl_spring) ||
            (_dynCtrlMode == sim_jointdynctrl_positioncb) || (_dynCtrlMode == sim_jointdynctrl_springcb))
        { // we have the built-in control (position or spring-damper KC)
            outputWarningAboutPossibleLoopClosureProblemsWithMujoco = (flags & 8);
            if (dynStepSize != 0.0)
            {
                if ((_dynPositionCtrlType == 1) && (_dynCtrlMode == sim_jointdynctrl_position))
                { // motion profile
                    double dynPosCtrlCurrentVelAccel[2] = {_dynPosCtrl_currentVelAccel[0],
                                                           _dynPosCtrl_currentVelAccel[1]};
                    double cp = double(currentPosVelAccel[0]);
                    double tp = cp + double(errorV);
                    double maxVelAccelJerk[3] = {double(_maxVelAccelJerk[0]), double(_maxVelAccelJerk[1]),
                                                 double(_maxVelAccelJerk[2])};
                    bool sel = true;
                    double dummy = 0.0;
                    int ruckObj = App::worldContainer->pluginContainer->ruckigPlugin_pos(
                        -1, 1, dynStepSize, -1, &cp, dynPosCtrlCurrentVelAccel, dynPosCtrlCurrentVelAccel + 1,
                        maxVelAccelJerk, maxVelAccelJerk + 1, maxVelAccelJerk + 2, &sel, &tp, &dummy);
                    if (ruckObj >= 0)
                    {
                        int res = App::worldContainer->pluginContainer->ruckigPlugin_step(
                            ruckObj, dynStepSize, &dummy, dynPosCtrlCurrentVelAccel, dynPosCtrlCurrentVelAccel + 1,
                            &dummy);
                        App::worldContainer->pluginContainer->ruckigPlugin_remove(ruckObj);
                        velAndForce[0] = double(dynPosCtrlCurrentVelAccel[0]);
                        velAndForce[1] = _targetForce;
                        if (velAndForce[0] * velAndForce[1] < 0.0)
                            velAndForce[1] = -velAndForce[1]; // make sure they have same sign
                    }
                    if ((rk4 == 0) || (rk4 == 4))
                    {
                        _dynPosCtrl_currentVelAccel[0] = dynPosCtrlCurrentVelAccel[0];
                        _dynPosCtrl_currentVelAccel[1] = dynPosCtrlCurrentVelAccel[1];
                    }
                }
                else
                { // pos ctrl or spring
                    double P, I, D;
                    getPid(P, I, D);
                    if ((_dynCtrlMode == sim_jointdynctrl_spring) || (_dynCtrlMode == sim_jointdynctrl_springcb))
                    {
                        P = _dynCtrl_kc[0];
                        I = 0.0;
                        D = _dynCtrl_kc[1];
                    }

                    if ((flags & 1) != 0)
                        _dynCtrl_pid_cumulErr = 0.0;

                    double e = errorV;

                    // Proportional part:
                    double ctrl = e * P;

                    // Integral part:
                    if ((I != 0.0) && (rk4 == 0)) // so that if we turn the integral part on, we don't try to catch up all the past errors!
                        _dynCtrl_pid_cumulErr += e * dynStepSize;
                    else
                        _dynCtrl_pid_cumulErr = 0.0;
                    ctrl += _dynCtrl_pid_cumulErr * I;

                    // Derivative part:
                    double cv = _velCalc_vel;
                    if ((flags & 2) != 0)
                        cv = currentPosVelAccel[1];
                    ctrl -= cv * D;

                    if ((_dynCtrlMode == sim_jointdynctrl_spring) || (_dynCtrlMode == sim_jointdynctrl_springcb))
                    { // "spring" mode, i.e. force modulation mode
                        velAndForce[0] = fabs(_targetVel);
                        if (ctrl < 0.0)
                            velAndForce[0] = -velAndForce[0];
                        velAndForce[1] = fabs(ctrl);
                        if (velAndForce[0] * velAndForce[1] < 0.0)
                            velAndForce[1] = -velAndForce[1]; // make sure they have same sign
                    }
                    else
                    {                              // regular position control
                        double vel = ctrl / 0.005; // was ctrl/dynStepSize, but has to be step size independent
                        double maxVel = _maxVelAccelJerk[0];
                        if (vel > maxVel)
                            vel = maxVel;
                        if (vel < -maxVel)
                            vel = -maxVel;

                        velAndForce[0] = vel;
                        velAndForce[1] = _targetForce;
                        if (velAndForce[0] * velAndForce[1] < 0.0)
                            velAndForce[1] = -velAndForce[1]; // make sure they have same sign
                    }
                }
                _dynCtrl_previousVelForce[0] = velAndForce[0];
                _dynCtrl_previousVelForce[1] = velAndForce[1];
            }
            else
            { // in case of RK4, pass1 (dt=0)
                velAndForce[0] = _dynCtrl_previousVelForce[0];
                velAndForce[1] = _dynCtrl_previousVelForce[1];
            }
        }
        if ((_dynCtrlMode == sim_jointdynctrl_callback) || (_dynCtrlMode == sim_jointdynctrl_positioncb) ||
            (_dynCtrlMode == sim_jointdynctrl_springcb))
        { // joint callback (and hybrid joint pos/spring + callback, old, for backward compatibility)
            bool rev = (_jointType == sim_joint_revolute);
            bool cycl = _isCyclic;
            double lowL = _posMin;
            double highL = _posMin + _posRange;
            if (App::worldContainer->getSysFuncAndHookCnt(sim_syscb_joint) > 0)
            { // a script might want to handle the joint
                // 1. We prepare the in/out stacks:
                CInterfaceStack* inStack = App::worldContainer->interfaceStackContainer->createStack();
                inStack->pushTableOntoStack();

                inStack->pushTextOntoStack("mode");
                inStack->pushInt32OntoStack(sim_jointmode_dynamic);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("first");
                inStack->pushBoolOntoStack((flags & 1) != 0);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("revolute");
                inStack->pushBoolOntoStack(rev);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("cyclic");
                inStack->pushBoolOntoStack(cycl);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("handle");
                inStack->pushInt32OntoStack(getObjectHandle());
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("lowLimit");
                inStack->pushFloatOntoStack(lowL);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("highLimit");
                inStack->pushFloatOntoStack(highL);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("passCnt");
                inStack->pushInt32OntoStack(loopCnt);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("totalPasses");
                inStack->pushInt32OntoStack(totalLoops);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("currentPos"); // deprecated
                inStack->pushFloatOntoStack(currentPosVelAccel[0]);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("pos");
                inStack->pushFloatOntoStack(currentPosVelAccel[0]);
                inStack->insertDataIntoStackTable();
                if (rk4 > 0)
                {
                    inStack->pushTextOntoStack("rk4pass");
                    inStack->pushInt32OntoStack(rk4);
                    inStack->insertDataIntoStackTable();
                }
                double cv = _velCalc_vel;
                if ((flags & 2) != 0)
                    cv = currentPosVelAccel[1];
                inStack->pushTextOntoStack("currentVel"); // deprecated
                inStack->pushFloatOntoStack(cv);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("vel");
                inStack->pushFloatOntoStack(cv);
                inStack->insertDataIntoStackTable();
                if ((flags & 4) != 0)
                {
                    inStack->pushTextOntoStack("accel");
                    inStack->pushFloatOntoStack(currentPosVelAccel[2]);
                    inStack->insertDataIntoStackTable();
                }
                inStack->pushTextOntoStack("targetPos");
                inStack->pushFloatOntoStack(_targetPos);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("errorValue"); // deprecated
                inStack->pushFloatOntoStack(errorV);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("error");
                inStack->pushFloatOntoStack(errorV);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("effort");
                inStack->pushFloatOntoStack(effort);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("dynStepSize"); // deprecated
                inStack->pushFloatOntoStack(dynStepSize);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("dt");
                inStack->pushFloatOntoStack(dynStepSize);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("targetVel");
                inStack->pushFloatOntoStack(_targetVel);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("maxForce"); // deprecated
                inStack->pushFloatOntoStack(fabs(_targetForce));
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("force");
                inStack->pushFloatOntoStack(_targetForce);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("velUpperLimit"); // deprecated
                inStack->pushFloatOntoStack(_maxVelAccelJerk[0]);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("maxVel");
                inStack->pushFloatOntoStack(_maxVelAccelJerk[0]);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("maxAccel");
                inStack->pushFloatOntoStack(_maxVelAccelJerk[1]);
                inStack->insertDataIntoStackTable();
                inStack->pushTextOntoStack("maxJerk");
                inStack->pushFloatOntoStack(_maxVelAccelJerk[2]);
                inStack->insertDataIntoStackTable();
                CInterfaceStack* outStack = App::worldContainer->interfaceStackContainer->createStack();

                // 2. Call the script(s):
                // First, the old callback functions:
                CScriptObject* script = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
                    sim_scripttype_simulation, _objectHandle);
                if ((script != nullptr) && (!script->getScriptIsDisabled()) &&
                    script->hasSystemFunctionOrHook(sim_syscb_jointcallback))
                    script->systemCallScript(sim_syscb_jointcallback, inStack, outStack);
                if (outStack->getStackSize() == 0)
                {
                    script = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
                        sim_scripttype_customization, _objectHandle);
                    if ((script != nullptr) && (!script->getScriptIsDisabled()) &&
                        script->hasSystemFunctionOrHook(sim_syscb_jointcallback))
                        script->systemCallScript(sim_syscb_jointcallback, inStack, outStack);
                }
                // Now the regular callback (with old and new scripts):
                if ((outStack->getStackSize() == 0) && (_dynCtrlMode == sim_jointdynctrl_callback))
                    App::worldContainer->callScripts(sim_syscb_joint, inStack, outStack, this); // this should only work with the callback mode! (and not
                                                                                                // with the old hybrid pos callback mode)

                // 3. Set default values:
                if (_dynCtrlMode == sim_jointdynctrl_callback)
                { // do not overwrite those with the old sim_jointdynctrl_positioncb and sim_jointdynctrl_springcb
                    velAndForce[0] = 0.0;
                    velAndForce[1] = 0.0;
                }
                // 4. Collect the return values:
                if (outStack->getStackSize() > 0)
                {
                    int s = outStack->getStackSize();
                    if (s > 1)
                        outStack->moveStackItemToTop(0);
                    bool hasForce = outStack->getStackMapDoubleValue("force", velAndForce[1]);
                    bool hasVel = outStack->getStackMapDoubleValue("velocity", velAndForce[0]); // deprecated
                    hasVel = hasVel || outStack->getStackMapDoubleValue("vel", velAndForce[0]);
                    if (!hasForce)
                        hasForce = outStack->getStackMapDoubleValue("ctrl", velAndForce[1]); // "force" or "ctrl" can be used interchangebly
                    // force, vel (and ctrl) are optional
                    if (hasForce)
                    {
                        if (!hasVel)
                        {                             // this is pure force control
                            velAndForce[0] = 10000.0; // default velocity
                            if (velAndForce[1] < 0.0)
                                velAndForce[0] *= -1.0;
                        }
                        else
                        { // this is velocity control (in physics engine), limited by force
                            // the sign of velocity takes precedence
                            if (velAndForce[0] * velAndForce[1] < 0.0)
                                velAndForce[1] = -velAndForce[1]; // make sure they have same sign
                        }
                    }
                    else
                    {
                        if (hasVel)
                        {                             // this is velocity control (in physics engine), with unlimited by force
                            velAndForce[1] = 10000.0; // default force/torque
                        }
                    }
                }
                App::worldContainer->interfaceStackContainer->destroyStack(outStack);
                App::worldContainer->interfaceStackContainer->destroyStack(inStack);
            }
        }
    }
    if (outputWarningAboutPossibleLoopClosureProblemsWithMujoco && (!warningAboutMujocoLoopClosureProblemsIssued))
    {
        std::string txt("Joint ");
        txt += getObjectAlias_printPath();
        txt += ": with the MuJoCo engine, and when the hierarchy tree (starting at the given joint) contains at least one loop closure constraint, the joint might not behave as expected when in position, velocity or spring-damper control mode. In that case, it is recommended to instead run a custom controller for that joint.";
        App::logMsg(sim_verbosity_warnings, txt.c_str());
        warningAboutMujocoLoopClosureProblemsIssued = true;
    }
    // Joint position and velocity is updated later, via _simSetJointPosition and _simSetJointVelocity from the physics
    // engine
    return (retVal);
}

void CJoint::handleMotion()
{
    if ((_jointMode == sim_jointmode_kinematic) && (_jointType != sim_joint_spherical) &&
        ((_kinematicMotionType & 3) != 0))
    {
        if (App::worldContainer->getSysFuncAndHookCnt(sim_syscb_joint) > 0)
        { // a script might want to handle the joint
            bool rev = (_jointType == sim_joint_revolute);
            double errorV;
            if (rev && _isCyclic)
                errorV = tt::getAngleMinusAlpha(_targetPos, _pos);
            else
                errorV = _targetPos - _pos;

            // Prepare the in/out stacks:
            CInterfaceStack* inStack = App::worldContainer->interfaceStackContainer->createStack();
            inStack->pushTableOntoStack();
            inStack->pushTextOntoStack("mode");
            inStack->pushInt32OntoStack(sim_jointmode_kinematic);
            inStack->insertDataIntoStackTable();
            inStack->pushTextOntoStack("revolute");
            inStack->pushBoolOntoStack(rev);
            inStack->insertDataIntoStackTable();
            inStack->pushTextOntoStack("cyclic");
            inStack->pushBoolOntoStack(_isCyclic && rev);
            inStack->insertDataIntoStackTable();
            inStack->pushTextOntoStack("handle");
            inStack->pushInt32OntoStack(_objectHandle);
            inStack->insertDataIntoStackTable();
            inStack->pushTextOntoStack("lowLimit");
            inStack->pushFloatOntoStack(_posMin);
            inStack->insertDataIntoStackTable();
            inStack->pushTextOntoStack("highLimit");
            inStack->pushFloatOntoStack(_posMin + _posRange);
            inStack->insertDataIntoStackTable();

            inStack->pushTextOntoStack("currentPos"); // deprecated
            inStack->pushFloatOntoStack(_pos);
            inStack->insertDataIntoStackTable();
            inStack->pushTextOntoStack("pos");
            inStack->pushFloatOntoStack(_pos);
            inStack->insertDataIntoStackTable();

            inStack->pushTextOntoStack("currentVel"); // deprecated
            inStack->pushFloatOntoStack(_velCalc_vel);
            inStack->insertDataIntoStackTable();
            inStack->pushTextOntoStack("vel");
            inStack->pushFloatOntoStack(_velCalc_vel);
            inStack->insertDataIntoStackTable();

            if ((_kinematicMotionType & 3) == 1)
            {
                inStack->pushTextOntoStack("targetPos");
                inStack->pushFloatOntoStack(_targetPos);
                inStack->insertDataIntoStackTable();
            }
            if ((_kinematicMotionType & 3) == 2)
            {
                inStack->pushTextOntoStack("targetVel");
                inStack->pushFloatOntoStack(_targetVel);
                inStack->insertDataIntoStackTable();
            }

            if ((_kinematicMotionType & 16) != 0)
            {
                inStack->pushTextOntoStack("initVel");
                inStack->pushFloatOntoStack(_kinematicMotionInitVel);
                inStack->insertDataIntoStackTable();
            }

            inStack->pushTextOntoStack("errorValue"); // deprecated
            inStack->pushFloatOntoStack(errorV);
            inStack->insertDataIntoStackTable();
            inStack->pushTextOntoStack("error");
            inStack->pushFloatOntoStack(errorV);
            inStack->insertDataIntoStackTable();
            inStack->pushTextOntoStack("maxVel");
            inStack->pushFloatOntoStack(_maxVelAccelJerk[0]);
            inStack->insertDataIntoStackTable();
            inStack->pushTextOntoStack("maxAccel");
            inStack->pushFloatOntoStack(_maxVelAccelJerk[1]);
            inStack->insertDataIntoStackTable();
            inStack->pushTextOntoStack("maxJerk");
            inStack->pushFloatOntoStack(_maxVelAccelJerk[2]);
            inStack->insertDataIntoStackTable();

            CInterfaceStack* outStack = App::worldContainer->interfaceStackContainer->createStack();

            // Call the script(s):
            App::worldContainer->callScripts(sim_syscb_joint, inStack, outStack, this);

            if (outStack->getStackSize() > 0)
            {
                _kinematicMotionType = _kinematicMotionType & 3; // remove reset flag
                int s = outStack->getStackSize();
                if (s > 1)
                    outStack->moveStackItemToTop(0);
                double pos;
                if (outStack->getStackMapDoubleValue("pos", pos) ||
                    outStack->getStackMapDoubleValue("position", pos)) // "position" is deprecated
                    setPosition(pos);

                bool immobile = false;
                double cv, ca;
                if (outStack->getStackMapDoubleValue("vel", cv) ||
                    outStack->getStackMapDoubleValue("velocity", cv)) // "velocity" is deprecated
                {
                    setVelocity(cv);
                    if (outStack->getStackMapDoubleValue("accel", ca) ||
                        outStack->getStackMapDoubleValue("acceleration", ca)) // "acceleration" is deprecated
                        immobile = ((cv == 0.0) && (ca == 0.0));
                }
                outStack->getStackMapBoolValue("immobile", immobile);
                if (immobile)
                    _kinematicMotionType = 16;
            }
            App::worldContainer->interfaceStackContainer->destroyStack(outStack);
            App::worldContainer->interfaceStackContainer->destroyStack(inStack);
        }
    }
}

int CJoint::getKinematicMotionType() const
{
    return _kinematicMotionType;
}

void CJoint::setKinematicMotionType(int t, bool reset, double initVel /*=0.0*/)
{
    _kinematicMotionType = _kinematicMotionType & 16;
    _kinematicMotionType = _kinematicMotionType | t;
    if (reset)
    {
        _kinematicMotionType = _kinematicMotionType | 16;
        _kinematicMotionInitVel = initVel;
    }
}

void CJoint::setIsCyclic(bool isCyclic)
{
    bool diff = (_isCyclic != isCyclic);
    if (diff)
    {
        if (isCyclic)
        {
            if (getJointType() == sim_joint_revolute)
            {
                setScrewLead(0.0);
                setInterval(-piValue, +piValue);
            }
        }
        _isCyclic = isCyclic;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propJoint_cyclic.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, isCyclic);
            App::worldContainer->pushEvent();
        }
        _setPositionIsCyclic_sendOldIk(isCyclic);
        setVelocity_DEPRECATED(getVelocity_DEPRECATED()); // To make sure velocity is within allowed range
    }
}

void CJoint::setEnforceLimits(bool enforceLimits)
{
    bool diff = (_enforceLimits != enforceLimits);
    if (diff)
    {
        _enforceLimits = enforceLimits;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propJoint_enforceLimits.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _enforceLimits);
            App::worldContainer->pushEvent();
        }
        if (_enforceLimits)
            setPosition(_pos);
    }
}

void CJoint::_setPositionIsCyclic_sendOldIk(bool isCyclic) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle != -1)
        App::worldContainer->pluginContainer->oldIkPlugin_setJointInterval(_ikPluginCounterpartHandle, _isCyclic,
                                                                           _posMin, _posRange);
}

void CJoint::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
    setDependencyMasterJointHandle(-1);
}

void CJoint::addSpecializedObjectEventData(CCbor* ev)
{
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->openKeyMap(getObjectTypeInfo().c_str());
    ev->openKeyArray("colors");
    float c[9];
    _color.getColor(c, sim_colorcomponent_ambient_diffuse);
    _color.getColor(c + 3, sim_colorcomponent_specular);
    _color.getColor(c + 6, sim_colorcomponent_emission);
    ev->appendFloatArray(c, 9);
    _color_removeSoon.getColor(c, sim_colorcomponent_ambient_diffuse);
    _color_removeSoon.getColor(c + 3, sim_colorcomponent_specular);
    _color_removeSoon.getColor(c + 6, sim_colorcomponent_emission);
    ev->appendFloatArray(c, 9);
    ev->closeArrayOrMap(); // colors
    ev->appendKeyDouble("length", _length);
    ev->appendKeyDouble("diameter", _diameter);
    ev->appendKeyDouble("min", _posMin);
    ev->appendKeyDouble("range", _posRange);
    ev->openKeyMap("dependency");
    if (_dependencyMasterJointHandle != -1)
    {
        CSceneObject* master = App::currentWorld->sceneObjects->getJointFromHandle(_dependencyMasterJointHandle);
        if (master != nullptr)
        {
            ev->appendKeyInt("masterUid", master->getObjectUid());
            ev->appendKeyDouble("mult", _dependencyJointMult);
            ev->appendKeyDouble("off", _dependencyJointOffset);
        }
    }
    ev->closeArrayOrMap(); // dependency
#else
    _color.addGenesisEventData(ev);
#endif
    /*
    std::string tmp;
    switch (_jointType)
    {
    case sim_joint_revolute:
        tmp = "revolute";
        break;
    case sim_joint_prismatic:
        tmp = "prismatic";
        break;
    case sim_joint_spherical:
        tmp = "spherical";
        break;
    }
    ev->appendKeyText("type", tmp.c_str());
    */
    ev->appendKeyInt(propJoint_jointType.name, _jointType);
    ev->appendKeyInt(propJoint_jointMode.name, _jointMode);
    ev->appendKeyInt(propJoint_dynCtrlMode.name, _dynCtrlMode);
    ev->appendKeyInt(propJoint_dynVelMode.name, _dynVelocityCtrlType);
    ev->appendKeyInt(propJoint_dynPosMode.name, _dynPositionCtrlType);
    ev->appendKeyInt(propJoint_dependencyMaster.name, _dependencyMasterJointHandle);
    double arr[2] = {_dependencyJointOffset, _dependencyJointMult};
    ev->appendKeyDoubleArray(propJoint_dependencyParams.name, arr, 2);
    ev->appendKeyBool(propJoint_cyclic.name, _isCyclic);
    ev->appendKeyBool(propJoint_enforceLimits.name, _enforceLimits);
    ev->appendKeyDouble(propJoint_targetPos.name, _targetPos);
    ev->appendKeyDouble(propJoint_targetVel.name, _targetVel);
    ev->appendKeyDouble(propJoint_targetForce.name, _targetForce);
    ev->appendKeyDouble(propJoint_averageJointForce.name, _filteredForceOrTorque);

    double q[4];
    _sphericalTransf.getData(q, true);
    ev->appendKeyDoubleArray(propJoint_quaternion.name, q, 4);
    ev->appendKeyDouble(propJoint_position.name, _pos);
    ev->appendKeyDouble(propJoint_screwLead.name, _screwLead);
    double p[7];
    _intrinsicTransformationError.getData(p, true);
    ev->appendKeyDoubleArray(propJoint_intrinsicError.name, p, 7);
    getIntrinsicTransformation(true).getData(p, true);
    ev->appendKeyDoubleArray(propJoint_intrinsicPose.name, p, 7);

    ev->appendKeyDoubleArray(propJoint_maxVelAccelJerk.name, _maxVelAccelJerk, 3);
    ev->appendKeyDoubleArray(propJoint_springDamperParams.name, _dynCtrl_kc, 2);
    getInterval(p[0], p[1]);
    ev->appendKeyDoubleArray(propJoint_interval.name, p, 2);
    ev->appendKeyDouble(propJoint_length.name, _length);
    ev->appendKeyDouble(propJoint_diameter.name, _diameter);
    ev->appendKeyDouble(propJoint_calcVelocity.name, _velCalc_vel);

    // Engine properties:
    setBoolProperty(nullptr, false, ev);
    setIntProperty(nullptr, 0, ev);
    setFloatProperty(nullptr, 0.0, ev);
    setIntArray2Property(nullptr, nullptr, ev);
    setVector2Property(nullptr, nullptr, ev);
    C3Vector dummy;
    setVector3Property(nullptr, dummy, ev);
    setFloatArrayProperty(nullptr, nullptr, 0, ev);
    _sendEngineString(ev);

#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->closeArrayOrMap(); // joint
#endif
}

CSceneObject* CJoint::copyYourself()
{
    CJoint* newJoint = (CJoint*)CSceneObject::copyYourself();
    newJoint->_dependencyMasterJointHandle = _dependencyMasterJointHandle; // important for copy operations connections
    newJoint->_dependencyJointMult = _dependencyJointMult;
    newJoint->_dependencyJointOffset = _dependencyJointOffset;

    newJoint->_jointType = _jointType;
    newJoint->_jointMode = _jointMode;
    newJoint->_screwLead = _screwLead;
    newJoint->_sphericalTransf = _sphericalTransf;
    newJoint->_pos = _pos;
    newJoint->_ikWeight_old = _ikWeight_old;
    newJoint->_diameter = _diameter;
    newJoint->_length = _length;
    newJoint->_isCyclic = _isCyclic;
    newJoint->_enforceLimits = _enforceLimits;
    newJoint->_posRange = _posRange;
    newJoint->_posMin = _posMin;
    newJoint->_maxStepSize_old = _maxStepSize_old;
    newJoint->_targetPos = _targetPos;
    newJoint->_targetVel = _targetVel;

    _color.copyYourselfInto(&newJoint->_color);

    newJoint->_dynCtrlMode = _dynCtrlMode;
    newJoint->_dynPositionCtrlType = _dynPositionCtrlType;
    newJoint->_dynVelocityCtrlType = _dynVelocityCtrlType;
    newJoint->_motorLock = _motorLock;
    newJoint->_targetForce = _targetForce;

    newJoint->_jointHasHybridFunctionality = _jointHasHybridFunctionality;

    newJoint->_dynCtrl_kc[0] = _dynCtrl_kc[0];
    newJoint->_dynCtrl_kc[1] = _dynCtrl_kc[1];

    newJoint->_bulletFloatParams.assign(_bulletFloatParams.begin(), _bulletFloatParams.end());
    newJoint->_bulletIntParams.assign(_bulletIntParams.begin(), _bulletIntParams.end());

    newJoint->_odeFloatParams.assign(_odeFloatParams.begin(), _odeFloatParams.end());
    newJoint->_odeIntParams.assign(_odeIntParams.begin(), _odeIntParams.end());

    newJoint->_vortexFloatParams.assign(_vortexFloatParams.begin(), _vortexFloatParams.end());
    newJoint->_vortexIntParams.assign(_vortexIntParams.begin(), _vortexIntParams.end());

    newJoint->_newtonFloatParams.assign(_newtonFloatParams.begin(), _newtonFloatParams.end());
    newJoint->_newtonIntParams.assign(_newtonIntParams.begin(), _newtonIntParams.end());

    newJoint->_mujocoFloatParams.assign(_mujocoFloatParams.begin(), _mujocoFloatParams.end());
    newJoint->_mujocoIntParams.assign(_mujocoIntParams.begin(), _mujocoIntParams.end());

    newJoint->_initialValuesInitialized = _initialValuesInitialized;
    newJoint->_initialPosition = _initialPosition;
    newJoint->_initialSphericalJointTransformation = _initialSphericalJointTransformation;
    newJoint->_initialTargetPosition = _initialTargetPosition;
    newJoint->_initialTargetVelocity = _initialTargetVelocity;
    newJoint->_initialDynCtrl_lockAtVelZero = _initialDynCtrl_lockAtVelZero;
    newJoint->_initialDynCtrl_force = _initialDynCtrl_force;
    newJoint->_initialDynCtrl_kc[0] = _initialDynCtrl_kc[0];
    newJoint->_initialDynCtrl_kc[1] = _initialDynCtrl_kc[1];
    newJoint->_initialJointMode = _initialJointMode;
    newJoint->_initialHybridOperation = _initialHybridOperation;

    newJoint->_maxAcceleration_DEPRECATED = _maxAcceleration_DEPRECATED;
    newJoint->_explicitHandling_DEPRECATED = _explicitHandling_DEPRECATED;
    newJoint->_unlimitedAcceleration_DEPRECATED = _unlimitedAcceleration_DEPRECATED;
    newJoint->_invertTargetVelocityAtLimits_DEPRECATED = _invertTargetVelocityAtLimits_DEPRECATED;
    newJoint->_jointPositionForMotionHandling_DEPRECATED = _jointPositionForMotionHandling_DEPRECATED;
    newJoint->_velocity_DEPRECATED = _velocity_DEPRECATED;
    newJoint->_initialExplicitHandling_DEPRECATED = _initialExplicitHandling_DEPRECATED;
    newJoint->_initialVelocity_DEPRECATED = _initialVelocity_DEPRECATED;

    for (size_t i = 0; i < 3; i++)
    {
        newJoint->_maxVelAccelJerk[i] = _maxVelAccelJerk[i];
        newJoint->_initialMaxVelAccelJerk[i] = _initialMaxVelAccelJerk[i];
    }

    return (newJoint);
}

void CJoint::announceCollectionWillBeErased(int groupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID, copyBuffer);
}

void CJoint::announceCollisionWillBeErased(int collisionID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID, copyBuffer);
}

void CJoint::announceDistanceWillBeErased(int distanceID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID, copyBuffer);
}

void CJoint::performIkLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performIkLoadingMapping(map, opType);
}

void CJoint::performCollectionLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performCollectionLoadingMapping(map, opType);
}

void CJoint::performCollisionLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performCollisionLoadingMapping(map, opType);
}

void CJoint::performDistanceLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performDistanceLoadingMapping(map, opType);
}

void CJoint::performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performTextureObjectLoadingMapping(map, opType);
}

void CJoint::performDynMaterialObjectLoadingMapping(const std::map<int, int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CJoint::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing. The order in which we are storing is very important!!!!

            ar.storeDataName("Jtt");
            ar << _jointType;
            ar.flush();

            ar.storeDataName("_sp");
            ar << (_screwLead / piValT2);
            ar.flush();

            ar.storeDataName("Sld");
            ar << _screwLead;
            ar.flush();

            ar.storeDataName("_st");
            ar << _sphericalTransf(0) << _sphericalTransf(1);
            ar << _sphericalTransf(2) << _sphericalTransf(3);
            ar.flush();

            ar.storeDataName("Va9");
            unsigned char dummy = 0;
            SIM_SET_CLEAR_BIT(dummy, 0, _isCyclic);
            SIM_SET_CLEAR_BIT(dummy, 1, _explicitHandling_DEPRECATED);
            SIM_SET_CLEAR_BIT(dummy, 2, _unlimitedAcceleration_DEPRECATED);
            SIM_SET_CLEAR_BIT(dummy, 3, _invertTargetVelocityAtLimits_DEPRECATED);
            SIM_SET_CLEAR_BIT(dummy, 4, _dynCtrlMode != sim_jointdynctrl_free);     // for backward comp. with V4.3 and earlier
            SIM_SET_CLEAR_BIT(dummy, 5, _dynCtrlMode >= sim_jointdynctrl_position); // for backward comp. with V4.3 and earlier
            SIM_SET_CLEAR_BIT(dummy, 6, _jointHasHybridFunctionality);
            SIM_SET_CLEAR_BIT(dummy, 7, (_dynCtrlMode == sim_jointdynctrl_spring) || (_dynCtrlMode == sim_jointdynctrl_springcb)); // for backward comp. with V4.3 and earlier
            ar << dummy;
            ar.flush();

            ar.storeDataName("Vaa");
            dummy = 0;
            SIM_SET_CLEAR_BIT(dummy, 1, _motorLock);
            SIM_SET_CLEAR_BIT(dummy, 2, _enforceLimits);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Cl1");
            ar.setCountingMode();
            _color.serialize(ar, 0);
            if (ar.setWritingMode())
                _color.serialize(ar, 0);

            ar.storeDataName("_mr");
            ar << _posMin << _posRange;
            ar.flush();

            ar.storeDataName("_rt");
            ar << _pos;
            ar.flush();

            ar.storeDataName("_ss");
            ar << _maxStepSize_old;
            ar.flush();

            ar.storeDataName("_rg");
            ar << _length << _diameter;
            ar.flush();

            ar.storeDataName("_kw");
            ar << _ikWeight_old;
            ar.flush();

            ar.storeDataName("Jmd");
            ar << _jointMode;
            ar.flush();

            ar.storeDataName("_dt");
            ar << _dependencyMasterJointHandle;
            ar << _dependencyJointMult << _dependencyJointOffset;
            ar.flush();

            ar.storeDataName("_m3");
            ar << _maxAcceleration_DEPRECATED << _velocity_DEPRECATED;
            ar.flush();

            ar.storeDataName("_mp");
            ar << _targetVel << _targetForce;
            ar.flush();

            double P, I, D;
            getPid(P, I, D, sim_physics_bullet);
            ar.storeDataName("_pp");
            ar << _dynCtrl_kc[0] << _dynCtrl_kc[1];
            ar.flush();

            ar.storeDataName("_tp");
            ar << _targetPos;
            ar.flush();

            ar.storeDataName("_tN"); // Bullet params, keep this after "Bt1"
            ar << int(_bulletFloatParams.size()) << int(_bulletIntParams.size());
            for (size_t i = 0; i < _bulletFloatParams.size(); i++)
                ar << _bulletFloatParams[i];
            for (size_t i = 0; i < _bulletIntParams.size(); i++)
                ar << _bulletIntParams[i];
            ar.flush();

            ar.storeDataName("_dN"); // ODE params, keep this after "Od1"
            ar << int(_odeFloatParams.size()) << int(_odeIntParams.size());
            for (size_t i = 0; i < _odeFloatParams.size(); i++)
                ar << _odeFloatParams[i];
            for (size_t i = 0; i < _odeIntParams.size(); i++)
                ar << _odeIntParams[i];
            ar.flush();

            ar.storeDataName("_o2"); // vortex params:
            ar << int(_vortexFloatParams.size()) << int(_vortexIntParams.size());
            for (size_t i = 0; i < _vortexFloatParams.size(); i++)
                ar << _vortexFloatParams[i];
            for (size_t i = 0; i < _vortexIntParams.size(); i++)
                ar << _vortexIntParams[i];
            ar.flush();

            ar.storeDataName("_w1"); // newton params:
            ar << int(_newtonFloatParams.size()) << int(_newtonIntParams.size());
            for (size_t i = 0; i < _newtonFloatParams.size(); i++)
                ar << _newtonFloatParams[i];
            for (size_t i = 0; i < _newtonIntParams.size(); i++)
                ar << _newtonIntParams[i];
            ar.flush();

            ar.storeDataName("_j3"); // mujoco params:
            ar << int(_mujocoFloatParams.size()) << int(_mujocoIntParams.size());
            for (size_t i = 0; i < _mujocoFloatParams.size(); i++)
                ar << _mujocoFloatParams[i];
            for (size_t i = 0; i < _mujocoIntParams.size(); i++)
                ar << _mujocoIntParams[i];
            ar.flush();

            ar.storeDataName("_uc");
            ar << _maxVelAccelJerk[0] << _maxVelAccelJerk[1] << _maxVelAccelJerk[2];
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
        { // Loading
            int byteQuantity;
            std::string theName = "";
            bool kAndCSpringParameterPresent = false; // for backward compatibility (7/5/2014)
            bool usingDynCtrlMode = false;
            bool motorEnabled_old, ctrlEnabled_old, springMode_old;
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("Prt") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float a;
                        ar >> a;
                        _pos = (double)a;
                        ;
                    }

                    if (theName.compare("_rt") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _pos;
                    }

                    if (theName.compare("Jsp") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float a;
                        ar >> a;
                        _screwLead = (double)(a * piValT2);
                    }

                    if (theName.compare("_sp") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        double a;
                        ar >> a;
                        _screwLead = a * piValT2;
                    }

                    if (theName.compare("Sld") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _screwLead;
                    }

                    if (theName.compare("Jst") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float a, b, c, d;
                        ar >> a >> b >> c >> d;
                        _sphericalTransf(0) = (double)a;
                        _sphericalTransf(1) = (double)b;
                        _sphericalTransf(2) = (double)c;
                        _sphericalTransf(3) = (double)d;
                        _sphericalTransf.normalize(); // we read from float. Make sure we are perfectly normalized!
                    }

                    if (theName.compare("_st") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _sphericalTransf(0) >> _sphericalTransf(1);
                        ar >> _sphericalTransf(2) >> _sphericalTransf(3);
                        _sphericalTransf.normalize();
                    }

                    if (theName.compare("Mss") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float a;
                        ar >> a;
                        _maxStepSize_old = (double)a;
                    }

                    if (theName.compare("_ss") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _maxStepSize_old;
                    }

                    if (theName.compare("Ikw") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float a;
                        ar >> a;
                        _ikWeight_old = (double)a;
                    }

                    if (theName.compare("_kw") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _ikWeight_old;
                    }

                    if (theName.compare("Cl1") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        _color.serialize(ar, 0);
                    }
                    if (theName.compare("Arg") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli;
                        ar >> bla >> bli;
                        _length = (double)bla;
                        _diameter = (double)bli;
                    }

                    if (theName.compare("_rg") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _length >> _diameter;
                    }

                    if (theName.compare("Pmr") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli;
                        ar >> bla >> bli;
                        _posMin = (double)bla;
                        _posRange = (double)bli;
                    }

                    if (theName.compare("_mr") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _posMin >> _posRange;
                    }

                    if (theName.compare("Va9") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _isCyclic = SIM_IS_BIT_SET(dummy, 0);
                        _explicitHandling_DEPRECATED = SIM_IS_BIT_SET(dummy, 1);
                        _unlimitedAcceleration_DEPRECATED = SIM_IS_BIT_SET(dummy, 2);
                        _invertTargetVelocityAtLimits_DEPRECATED = SIM_IS_BIT_SET(dummy, 3);
                        motorEnabled_old = SIM_IS_BIT_SET(dummy, 4);
                        ctrlEnabled_old = SIM_IS_BIT_SET(dummy, 5);
                        _jointHasHybridFunctionality = SIM_IS_BIT_SET(dummy, 6);
                        springMode_old = SIM_IS_BIT_SET(dummy, 7);
                        if (_jointHasHybridFunctionality && CSimFlavor::getBoolVal(18))
                            App::logMsg(sim_verbosity_errors, "Joint has hybrid functionality...");
                    }
                    if (theName.compare("Vaa") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        // _dynamicMotorCustomControl_old=SIM_IS_BIT_SET(dummy,0);
                        _motorLock = SIM_IS_BIT_SET(dummy, 1);
                        _enforceLimits = SIM_IS_BIT_SET(dummy, 2);
                    }
                    if (theName.compare("Jmd") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _jointMode;
                        if (_jointMode == sim_jointmode_reserved_previously_ikdependent)
                            _jointMode = sim_jointmode_dependent; // since 4/7/2014 there is no more an ikdependent mode
                                                                  // (ikdependent and dependent are treated as same)
                        if (CSimFlavor::getBoolVal(18))
                        {
                            if ((_jointMode != sim_jointmode_kinematic) && (_jointMode != sim_jointmode_dependent) &&
                                (_jointMode != sim_jointmode_dynamic))
                                App::logMsg(sim_verbosity_errors, "Joint has deprecated mode...");
                        }
                    }
                    if (theName.compare("Jdt") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _dependencyMasterJointHandle;
                        float bla, bli;
                        ar >> bla >> bli;
                        _dependencyJointMult = (double)bla;
                        _dependencyJointOffset = (double)bli;
                    }

                    if (theName.compare("_dt") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _dependencyMasterJointHandle;
                        ar >> _dependencyJointMult >> _dependencyJointOffset;
                    }

                    if (theName.compare("Jtt") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _jointType;
                    }
                    if (theName.compare("Jm2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        float a, b, v;
                        ar >> a >> b >> v;
                        _maxAcceleration_DEPRECATED = (double)a;
                        _velocity_DEPRECATED = (double)b;
                        _targetVel = (double)v;
                    }
                    if (theName.compare("Jm3") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli;
                        ar >> bla >> bli;
                        _maxAcceleration_DEPRECATED = (double)bla;
                        _velocity_DEPRECATED = (double)bli;
                    }

                    if (theName.compare("_m3") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _maxAcceleration_DEPRECATED >> _velocity_DEPRECATED;
                    }

                    if (theName.compare("Dmp") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli;
                        ar >> bla >> bli;
                        _targetVel = (double)bla;
                        _targetForce = (double)bli;
                        if (_targetVel * _targetForce < 0.0)
                            _targetForce = -_targetForce;
                    }

                    if (theName.compare("_mp") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _targetVel >> _targetForce;
                        if (_targetVel * _targetForce < 0.0)
                            _targetForce = -_targetForce;
                    }

                    if (theName.compare("Dpc") == 0)
                    { // keep for backward compatibility (7/5/2014)
                        noHit = false;
                        ar >> byteQuantity;
                        float P, I, D;
                        ar >> P >> I >> D;
                        I /= 0.005f;
                        D *= 0.005f;
                        setPid_old((double)P, (double)I, (double)D);
                    }
                    if (theName.compare("Dp2") == 0)
                    { // keep for backward compatibility (29/8/2022)
                        noHit = false;
                        ar >> byteQuantity;
                        float P, I, D;
                        ar >> P >> I >> D;
                        setPid_old((double)P, (double)I, (double)D);
                    }
                    if (theName.compare("Spp") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli;
                        ar >> bla >> bli;
                        _dynCtrl_kc[0] = (double)bla;
                        _dynCtrl_kc[1] = (double)bli;
                        kAndCSpringParameterPresent = true;
                    }

                    if (theName.compare("_pp") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _dynCtrl_kc[0] >> _dynCtrl_kc[1];
                        kAndCSpringParameterPresent = true;
                    }

                    if (theName.compare("Dtp") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        float a;
                        ar >> a;
                        _targetPos = (double)a;
                    }

                    if (theName.compare("_tp") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _targetPos;
                    }

                    if (theName.compare("Od1") == 0)
                    { // keep for backward compat. (09/03/2016)
                        noHit = false;
                        ar >> byteQuantity;
                        float a, b, c, d, e;
                        ar >> a >> b >> c >> d >> e;
                        _odeFloatParams[simi_ode_joint_stoperp] = (double)a;
                        _odeFloatParams[simi_ode_joint_stopcfm] = (double)b;
                        _odeFloatParams[simi_ode_joint_bounce] = (double)c;
                        _odeFloatParams[simi_ode_joint_fudgefactor] = (double)d;
                        _odeFloatParams[simi_ode_joint_normalcfm] = (double)e;
                    }
                    if (theName.compare("Bt1") == 0)
                    { // keep for backward compat. (09/03/2016)
                        noHit = false;
                        ar >> byteQuantity;
                        float a, b, c;
                        ar >> a >> b >> c;
                        _bulletFloatParams[simi_bullet_joint_stoperp] = (double)a;
                        _bulletFloatParams[simi_bullet_joint_stopcfm] = (double)b;
                        _bulletFloatParams[simi_bullet_joint_normalcfm] = (double)c;
                    }
                    if (theName.compare("BtN") == 0)
                    { // Bullet params, for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_bulletFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_bulletIntParams.size()), cnt2);

                        float vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _bulletFloatParams already!
                            ar >> vf;
                            _bulletFloatParams[i] = (double)vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _bulletIntParams already!
                            ar >> vi;
                            _bulletIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }

                    if (theName.compare("_tN") == 0)
                    { // Bullet params:
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_bulletFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_bulletIntParams.size()), cnt2);

                        double vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _bulletFloatParams already!
                            ar >> vf;
                            _bulletFloatParams[i] = vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _bulletIntParams already!
                            ar >> vi;
                            _bulletIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }

                    if (theName.compare("OdN") == 0)
                    { // ODE params, for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_odeFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_odeIntParams.size()), cnt2);

                        float vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _odeFloatParams already!
                            ar >> vf;
                            _odeFloatParams[i] = (double)vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _odeIntParams already!
                            ar >> vi;
                            _odeIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }

                    if (theName.compare("_dN") == 0)
                    { // ODE params:
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_odeFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_odeIntParams.size()), cnt2);

                        double vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _odeFloatParams already!
                            ar >> vf;
                            _odeFloatParams[i] = vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _odeIntParams already!
                            ar >> vi;
                            _odeIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }

                    if (theName.compare("Vo2") == 0)
                    { // vortex params, for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_vortexFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_vortexIntParams.size()), cnt2);

                        float vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _vortexFloatParams already!
                            ar >> vf;
                            _vortexFloatParams[i] = (double)vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _vortexIntParams already!
                            ar >> vi;
                            _vortexIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }

                    if (theName.compare("_o2") == 0)
                    { // vortex params:
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_vortexFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_vortexIntParams.size()), cnt2);

                        double vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _vortexFloatParams already!
                            ar >> vf;
                            _vortexFloatParams[i] = vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _vortexIntParams already!
                            ar >> vi;
                            _vortexIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }

                    if (theName.compare("Nw1") == 0)
                    { // Newton params, for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_newtonFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_newtonIntParams.size()), cnt2);

                        float vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _newtonFloatParams already!
                            ar >> vf;
                            _newtonFloatParams[i] = (double)vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _newtonIntParams already!
                            ar >> vi;
                            _newtonIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }

                    if (theName.compare("_w1") == 0)
                    { // Newton params:
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_newtonFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_newtonIntParams.size()), cnt2);

                        double vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _newtonFloatParams already!
                            ar >> vf;
                            _newtonFloatParams[i] = vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _newtonIntParams already!
                            ar >> vi;
                            _newtonIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }

                    if (theName.compare("Mj3") == 0)
                    { // Mujoco params, for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_mujocoFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_mujocoIntParams.size()), cnt2);

                        float vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _mujocoFloatParams already!
                            ar >> vf;
                            _mujocoFloatParams[i] = (double)vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _mujocoIntParams already!
                            ar >> vi;
                            _mujocoIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }

                    if (theName.compare("_j3") == 0)
                    { // Mujoco params:
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_mujocoFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_mujocoIntParams.size()), cnt2);

                        double vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _mujocoFloatParams already!
                            ar >> vf;
                            _mujocoFloatParams[i] = vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _mujocoIntParams already!
                            ar >> vi;
                            _mujocoIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }

                    if (theName.compare("Cco") == 0)
                    { // Keep this for backward compatibility (6/8/2014)
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _jointCallbackCallOrder_backwardCompatibility;
                    }
                    if (theName.compare("Ruc") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli, blo;
                        ar >> bla >> bli >> blo;
                        _maxVelAccelJerk[0] = (double)bla;
                        _maxVelAccelJerk[1] = (double)bli;
                        _maxVelAccelJerk[2] = (double)blo;
                    }

                    if (theName.compare("_uc") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _maxVelAccelJerk[0] >> _maxVelAccelJerk[1] >> _maxVelAccelJerk[2];
                    }

                    if (theName.compare("Ulv") == 0)
                    { // Keep for backward compatibility with V4.3 and earlier
                        noHit = false;
                        ar >> byteQuantity;
                        float val;
                        ar >> val;
                        if ((_jointMode == sim_jointmode_dynamic) || _jointHasHybridFunctionality)
                            _maxVelAccelJerk[0] = (double)val;
                    }
                    if (theName.compare("Dcm") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _dynCtrlMode;
                        usingDynCtrlMode = true;
                    }
                    if (theName.compare("Dpm") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _dynPositionCtrlType;
                    }
                    if (theName.compare("Dvm") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _dynVelocityCtrlType;
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }

            if (!usingDynCtrlMode)
            { // for backward comp. with V4.3 and earlier
                _dynCtrlMode = sim_jointdynctrl_free;
                if (motorEnabled_old)
                {
                    _dynCtrlMode = sim_jointdynctrl_velocity;
                    if (ctrlEnabled_old)
                    {
                        _dynCtrlMode = sim_jointdynctrl_positioncb;
                        if (springMode_old)
                            _dynCtrlMode = sim_jointdynctrl_springcb;
                    }
                }

                if (!kAndCSpringParameterPresent)
                { // for backward compatibility (7/5/2014):
                    if (motorEnabled_old && ctrlEnabled_old && springMode_old)
                    { // we have a joint that behaves as a spring. We need to compute the corresponding K and C
                        // parameters, and adjust the max. force/torque (since that was not limited before):
                        double P, I, D;
                        getPid(P, I, D, sim_physics_bullet);
                        _dynCtrl_kc[0] = _targetForce * P;
                        _dynCtrl_kc[1] = _targetForce * D;
                        double maxTolerablePorDParam = 1.0;
                        if (_jointType == sim_joint_revolute)
                            maxTolerablePorDParam = 1.0 / piValT2;
                        double maxPorD = std::max<double>(fabs(P), fabs(D));
                        if (maxPorD > maxTolerablePorDParam)
                        { // we shift the limit up
                            double corr = maxTolerablePorDParam / maxPorD;
                            P *= corr;
                            I *= corr;
                            D *= corr;
                            setPid_old(P, I, D);
                            _targetForce /= corr;
                        }
                    }
                }
            }

            if (ar.getSerializationVersionThatWroteThisFile() < 17)
                utils::scaleColorUp_(_color.getColorsPtr());
            _fixVortexInfVals();
            computeBoundingBox();
        }
    }
    else
    {
        bool exhaustiveXml = ((ar.getFileType() != CSer::filetype_csim_xml_simplescene_file) &&
                              (ar.getFileType() != CSer::filetype_csim_xml_simplemodel_file));
        if (ar.isStoring())
        {
            double mult = 1.0;
            if (_jointType != sim_joint_prismatic)
                mult = 180.0 / piValue;
            ar.xmlAddNode_comment(" 'type' tag: can be 'revolute', 'prismatic' or 'spherical' ", exhaustiveXml);
            ar.xmlAddNode_enum("type", _jointType, sim_joint_revolute, "revolute", sim_joint_prismatic,
                               "prismatic", sim_joint_spherical, "spherical");
            ar.xmlAddNode_comment(" 'mode' tag: can be 'kinematic', 'dependent' or 'dynamic' ", exhaustiveXml);
            ar.xmlAddNode_enum("mode", _jointMode, sim_jointmode_kinematic, "kinematic", sim_jointmode_ik_deprecated,
                               "ik", sim_jointmode_dependent, "dependent", sim_jointmode_dynamic, "dynamic");

            ar.xmlAddNode_float("minPosition", _posMin * mult);
            ar.xmlAddNode_float("range", _posRange * mult);
            ar.xmlAddNode_float("position", _pos * mult);
            ar.xmlAddNode_float("targetPosition", _targetPos * mult);
            ar.xmlAddNode_float("targetVelocity", _targetVel * mult);
            ar.xmlAddNode_3float("maxVelAccelJerk", _maxVelAccelJerk[0] * mult, _maxVelAccelJerk[1] * mult,
                                 _maxVelAccelJerk[2] * mult);
            ar.xmlAddNode_comment(" 'screwPitch' tag: for backward compatibility", exhaustiveXml);
            ar.xmlAddNode_float("screwPitch", _screwLead / piValT2); // deprecated
            ar.xmlAddNode_float("screwLead", _screwLead);

            if (exhaustiveXml)
                ar.xmlAddNode_floats("sphericalQuaternion", _sphericalTransf.data, 4);
            else
            {
                C3Vector euler(_sphericalTransf.getEulerAngles());
                for (size_t l = 0; l < 3; l++)
                    euler(l) *= 180.0 / piValue;
                ar.xmlAddNode_floats("sphericalEuler", euler.data, 3);
            }

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("cyclic", _isCyclic);
            ar.xmlAddNode_bool("enforceLimits", _enforceLimits);
            ar.xmlAddNode_bool("hybridMode", _jointHasHybridFunctionality);
            ar.xmlPopNode();

            ar.xmlPushNewNode("sizes");
            ar.xmlAddNode_float("length", _length);
            ar.xmlAddNode_float("diameter", _diameter);
            ar.xmlPopNode();

            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("objectColor");
                _color.serialize(ar, 0);
                ar.xmlPopNode();
            }
            else
            {
                int rgb[3];
                for (size_t l = 0; l < 3; l++)
                    rgb[l] = int(_color.getColorsPtr()[l] * 255.1);
                ar.xmlAddNode_ints("objectColor", rgb, 3);
            }

            ar.xmlPushNewNode("ik");
            ar.xmlAddNode_float("maxStepSize", _maxStepSize_old * mult);
            ar.xmlAddNode_float("weight", _ikWeight_old);
            ar.xmlPopNode();

            ar.xmlPushNewNode("dependency");
            if (exhaustiveXml)
                ar.xmlAddNode_int("jointHandle", _dependencyMasterJointHandle);
            else
            {
                std::string str;
                CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(_dependencyMasterJointHandle);
                if (it != nullptr)
                    str = it->getObjectName_old();
                ar.xmlAddNode_comment(
                    " 'dependentJoint' tag only provided for backward compatibility, use instead 'dependentJointAlias' tag",
                    exhaustiveXml);
                ar.xmlAddNode_string("dependentJoint", str.c_str());
                if (it != nullptr)
                {
                    str = it->getObjectAlias() + "*";
                    str += std::to_string(it->getObjectHandle());
                }
                ar.xmlAddNode_string("dependentJointAlias", str.c_str());
            }
            ar.xmlAddNode_comment(" 'offset_m_or_rad' tag: has to be specified in meters or radians ", exhaustiveXml);
            ar.xmlAddNode_float("offset_m_or_rad", _dependencyJointOffset);
            ar.xmlAddNode_comment(" 'mult_m_or_rad' tag: has to be specified in meters or radians ", exhaustiveXml);
            ar.xmlAddNode_float("mult_m_or_rad", _dependencyJointMult);
            ar.xmlPopNode();

            ar.xmlPushNewNode("dynamics");
            ar.xmlAddNode_int("ctrlMode", _dynCtrlMode);
            ar.xmlAddNode_comment(" 'posController' tag: can be 'pid' or 'motionProfile' ", exhaustiveXml);
            ar.xmlAddNode_enum("posController", _dynPositionCtrlType, 0, "pid", 1, "motionProfile");
            ar.xmlAddNode_comment(" 'velController' tag: can be 'none' or 'motionProfile' ", exhaustiveXml);
            ar.xmlAddNode_enum("velController", _dynVelocityCtrlType, 0, "none", 1, "motionProfile");
            ar.xmlAddNode_float("maxForce", _targetForce);
            ar.xmlAddNode_comment(" 'upperVelocityLimit' tag only provided for backward compatibility", exhaustiveXml);
            ar.xmlAddNode_float("upperVelocityLimit",
                                _maxVelAccelJerk[0] * mult); // for backward compatibility (V4.3 and earlier)
            double P, I, D;
            getPid(P, I, D, sim_physics_bullet);
            ar.xmlAddNode_comment(" 'pidValues' tag only provided for backward compatibility", exhaustiveXml);
            ar.xmlAddNode_3float("pidValues", P, I, D); // for backward compatibility (V4.3 and earlier)
            ar.xmlAddNode_2float("kcValues", _dynCtrl_kc[0], _dynCtrl_kc[1]);

            ar.xmlAddNode_comment(" 'switches' tag only provided for backward compatibility", exhaustiveXml);
            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("motorEnabled",
                               _dynCtrlMode != sim_jointdynctrl_free); // for backward compatibility (V4.3 and earlier)
            ar.xmlAddNode_bool("controlLoopEnabled",
                               _dynCtrlMode >=
                                   sim_jointdynctrl_position); // for backward compatibility (V4.3 and earlier)
            ar.xmlAddNode_bool(
                "springMode",
                (_dynCtrlMode == sim_jointdynctrl_spring) ||
                    (_dynCtrlMode == sim_jointdynctrl_springcb)); // for backward compatibility (V4.3 and earlier)
            ar.xmlAddNode_bool("lockInVelocityControl", _motorLock);
            ar.xmlPopNode();

            double v[5];
            ar.xmlPushNewNode("engines");
            ar.xmlPushNewNode("bullet");
            ar.xmlAddNode_float("stoperp", _bulletFloatParams[simi_bullet_joint_stoperp]);
            ar.xmlAddNode_float("stopcfm", _bulletFloatParams[simi_bullet_joint_stopcfm]);
            ar.xmlAddNode_float("normalcfm", _bulletFloatParams[simi_bullet_joint_normalcfm]);
            v[0] = _bulletFloatParams[simi_bullet_joint_pospid1];
            v[1] = _bulletFloatParams[simi_bullet_joint_pospid2];
            v[2] = _bulletFloatParams[simi_bullet_joint_pospid3];
            ar.xmlAddNode_floats("posPid", v, 3);
            ar.xmlPopNode();

            ar.xmlPushNewNode("ode");
            ar.xmlAddNode_float("stoperp", _odeFloatParams[simi_ode_joint_stoperp]);
            ar.xmlAddNode_float("stopcfm", _odeFloatParams[simi_ode_joint_stopcfm]);
            ar.xmlAddNode_float("bounce", _odeFloatParams[simi_ode_joint_bounce]);
            ar.xmlAddNode_float("fudgefactor", _odeFloatParams[simi_ode_joint_fudgefactor]);
            ar.xmlAddNode_float("normalcfm", _odeFloatParams[simi_ode_joint_normalcfm]);
            v[0] = _odeFloatParams[simi_ode_joint_pospid1];
            v[1] = _odeFloatParams[simi_ode_joint_pospid2];
            v[2] = _odeFloatParams[simi_ode_joint_pospid3];
            ar.xmlAddNode_floats("posPid", v, 3);
            ar.xmlPopNode();

            ar.xmlPushNewNode("vortex");
            ar.xmlAddNode_float("lowerlimitdamping", _vortexFloatParams[simi_vortex_joint_lowerlimitdamping]);
            ar.xmlAddNode_float("upperlimitdamping", _vortexFloatParams[simi_vortex_joint_upperlimitdamping]);
            ar.xmlAddNode_float("lowerlimitstiffness",
                                _vortexFloatParams[simi_vortex_joint_lowerlimitstiffness]);
            ar.xmlAddNode_float("upperlimitstiffness",
                                _vortexFloatParams[simi_vortex_joint_upperlimitstiffness]);
            ar.xmlAddNode_float("lowerlimitrestitution",
                                _vortexFloatParams[simi_vortex_joint_lowerlimitrestitution]);
            ar.xmlAddNode_float("upperlimitrestitution",
                                _vortexFloatParams[simi_vortex_joint_upperlimitrestitution]);
            ar.xmlAddNode_float("lowerlimitmaxforce",
                                _vortexFloatParams[simi_vortex_joint_lowerlimitmaxforce]);
            ar.xmlAddNode_float("upperlimitmaxforce",
                                _vortexFloatParams[simi_vortex_joint_upperlimitmaxforce]);
            ar.xmlAddNode_float("motorconstraintfrictioncoeff",
                                _vortexFloatParams[simi_vortex_joint_motorconstraintfrictioncoeff]);
            ar.xmlAddNode_float("motorconstraintfrictionmaxforce",
                                _vortexFloatParams[simi_vortex_joint_motorconstraintfrictionmaxforce]);
            ar.xmlAddNode_float("motorconstraintfrictionloss",
                                _vortexFloatParams[simi_vortex_joint_motorconstraintfrictionloss]);
            ar.xmlAddNode_float("p0loss", _vortexFloatParams[simi_vortex_joint_p0loss]);
            ar.xmlAddNode_float("p0stiffness", _vortexFloatParams[simi_vortex_joint_p0stiffness]);
            ar.xmlAddNode_float("p0damping", _vortexFloatParams[simi_vortex_joint_p0damping]);
            ar.xmlAddNode_float("p0frictioncoeff", _vortexFloatParams[simi_vortex_joint_p0frictioncoeff]);
            ar.xmlAddNode_float("p0frictionmaxforce",
                                _vortexFloatParams[simi_vortex_joint_p0frictionmaxforce]);
            ar.xmlAddNode_float("p0frictionloss", _vortexFloatParams[simi_vortex_joint_p0frictionloss]);
            ar.xmlAddNode_float("p1loss", _vortexFloatParams[simi_vortex_joint_p1loss]);
            ar.xmlAddNode_float("p1stiffness", _vortexFloatParams[simi_vortex_joint_p1stiffness]);
            ar.xmlAddNode_float("p1damping", _vortexFloatParams[simi_vortex_joint_p1damping]);
            ar.xmlAddNode_float("p1frictioncoeff", _vortexFloatParams[simi_vortex_joint_p1frictioncoeff]);
            ar.xmlAddNode_float("p1frictionmaxforce",
                                _vortexFloatParams[simi_vortex_joint_p1frictionmaxforce]);
            ar.xmlAddNode_float("p1frictionloss", _vortexFloatParams[simi_vortex_joint_p1frictionloss]);
            ar.xmlAddNode_float("p2loss", _vortexFloatParams[simi_vortex_joint_p2loss]);
            ar.xmlAddNode_float("p2stiffness", _vortexFloatParams[simi_vortex_joint_p2stiffness]);
            ar.xmlAddNode_float("p2damping", _vortexFloatParams[simi_vortex_joint_p2damping]);
            ar.xmlAddNode_float("p2frictioncoeff", _vortexFloatParams[simi_vortex_joint_p2frictioncoeff]);
            ar.xmlAddNode_float("p2frictionmaxforce",
                                _vortexFloatParams[simi_vortex_joint_p2frictionmaxforce]);
            ar.xmlAddNode_float("p2frictionloss", _vortexFloatParams[simi_vortex_joint_p2frictionloss]);
            ar.xmlAddNode_float("a0loss", _vortexFloatParams[simi_vortex_joint_a0loss]);
            ar.xmlAddNode_float("a0stiffness", _vortexFloatParams[simi_vortex_joint_a0stiffness]);
            ar.xmlAddNode_float("a0damping", _vortexFloatParams[simi_vortex_joint_a0damping]);
            ar.xmlAddNode_float("a0frictioncoeff", _vortexFloatParams[simi_vortex_joint_a0frictioncoeff]);
            ar.xmlAddNode_float("a0frictionmaxforce",
                                _vortexFloatParams[simi_vortex_joint_a0frictionmaxforce]);
            ar.xmlAddNode_float("a0frictionloss", _vortexFloatParams[simi_vortex_joint_a0frictionloss]);
            ar.xmlAddNode_float("a1loss", _vortexFloatParams[simi_vortex_joint_a1loss]);
            ar.xmlAddNode_float("a1stiffness", _vortexFloatParams[simi_vortex_joint_a1stiffness]);
            ar.xmlAddNode_float("a1damping", _vortexFloatParams[simi_vortex_joint_a1damping]);
            ar.xmlAddNode_float("a1frictioncoeff", _vortexFloatParams[simi_vortex_joint_a1frictioncoeff]);
            ar.xmlAddNode_float("a1frictionmaxforce",
                                _vortexFloatParams[simi_vortex_joint_a1frictionmaxforce]);
            ar.xmlAddNode_float("a1frictionloss", _vortexFloatParams[simi_vortex_joint_a1frictionloss]);
            ar.xmlAddNode_float("a2loss", _vortexFloatParams[simi_vortex_joint_a2loss]);
            ar.xmlAddNode_float("a2stiffness", _vortexFloatParams[simi_vortex_joint_a2stiffness]);
            ar.xmlAddNode_float("a2damping", _vortexFloatParams[simi_vortex_joint_a2damping]);
            ar.xmlAddNode_float("a2frictioncoeff", _vortexFloatParams[simi_vortex_joint_a2frictioncoeff]);
            ar.xmlAddNode_float("a2frictionmaxforce",
                                _vortexFloatParams[simi_vortex_joint_a2frictionmaxforce]);
            ar.xmlAddNode_float("a2frictionloss", _vortexFloatParams[simi_vortex_joint_a2frictionloss]);
            ar.xmlAddNode_float("dependencyfactor", _vortexFloatParams[simi_vortex_joint_dependencyfactor]);
            ar.xmlAddNode_float("dependencyoffset", _vortexFloatParams[simi_vortex_joint_dependencyoffset]);
            v[0] = _vortexFloatParams[simi_vortex_joint_pospid1];
            v[1] = _vortexFloatParams[simi_vortex_joint_pospid2];
            v[2] = _vortexFloatParams[simi_vortex_joint_pospid3];
            ar.xmlAddNode_floats("posPid", v, 3);

            ar.xmlAddNode_int("relaxationenabledbc", _vortexIntParams[simi_vortex_joint_relaxationenabledbc]);
            ar.xmlAddNode_int("frictionenabledbc", _vortexIntParams[simi_vortex_joint_frictionenabledbc]);
            ar.xmlAddNode_int("frictionproportionalbc",
                              _vortexIntParams[simi_vortex_joint_frictionproportionalbc]);
            ar.xmlAddNode_int("objectid", _vortexIntParams[simi_vortex_joint_objectid]);
            ar.xmlAddNode_int("dependentobjectid", _vortexIntParams[simi_vortex_joint_dependentobjectid]);

            ar.xmlAddNode_bool("motorfrictionenabled", getBoolPropertyValue(propJoint_vortexAxisFrictionEnabled.name));
            ar.xmlAddNode_bool("proportionalmotorfriction", getBoolPropertyValue(propJoint_vortexAxisFrictionProportional.name));
            ar.xmlPopNode();

            ar.xmlPushNewNode("newton");
            ar.xmlAddNode_float("dependencyfactor", _newtonFloatParams[simi_newton_joint_dependencyfactor]);
            ar.xmlAddNode_float("dependencyoffset", _newtonFloatParams[simi_newton_joint_dependencyoffset]);
            v[0] = _newtonFloatParams[simi_newton_joint_pospid1];
            v[1] = _newtonFloatParams[simi_newton_joint_pospid2];
            v[2] = _newtonFloatParams[simi_newton_joint_pospid3];
            ar.xmlAddNode_floats("posPid", v, 3);

            ar.xmlAddNode_int("objectid", _newtonIntParams[simi_newton_joint_objectid]);
            ar.xmlAddNode_int("dependentobjectid", _newtonIntParams[simi_newton_joint_dependentobjectid]);
            ar.xmlPopNode();

            ar.xmlPushNewNode("mujoco");
            v[0] = _mujocoFloatParams[simi_mujoco_joint_solreflimit1];
            v[1] = _mujocoFloatParams[simi_mujoco_joint_solreflimit2];
            ar.xmlAddNode_floats("solreflimit", v, 2);
            v[0] = _mujocoFloatParams[simi_mujoco_joint_solimplimit1];
            v[1] = _mujocoFloatParams[simi_mujoco_joint_solimplimit2];
            v[2] = _mujocoFloatParams[simi_mujoco_joint_solimplimit3];
            v[3] = _mujocoFloatParams[simi_mujoco_joint_solimplimit4];
            v[4] = _mujocoFloatParams[simi_mujoco_joint_solimplimit5];
            ar.xmlAddNode_floats("solimplimit", v, 5);
            v[0] = _mujocoFloatParams[simi_mujoco_joint_solreffriction1];
            v[1] = _mujocoFloatParams[simi_mujoco_joint_solreffriction2];
            ar.xmlAddNode_floats("solreffriction", v, 2);
            v[0] = _mujocoFloatParams[simi_mujoco_joint_solimpfriction1];
            v[1] = _mujocoFloatParams[simi_mujoco_joint_solimpfriction2];
            v[2] = _mujocoFloatParams[simi_mujoco_joint_solimpfriction3];
            v[3] = _mujocoFloatParams[simi_mujoco_joint_solimpfriction4];
            v[4] = _mujocoFloatParams[simi_mujoco_joint_solimpfriction5];
            ar.xmlAddNode_floats("solimpfriction", v, 5);
            ar.xmlAddNode_float("frictionloss", _mujocoFloatParams[simi_mujoco_joint_frictionloss]);
            ar.xmlAddNode_float("stiffness", _mujocoFloatParams[simi_mujoco_joint_stiffness]);
            ar.xmlAddNode_float("damping", _mujocoFloatParams[simi_mujoco_joint_damping]);
            ar.xmlAddNode_float("springref", _mujocoFloatParams[simi_mujoco_joint_springref]);
            v[0] = _mujocoFloatParams[simi_mujoco_joint_springdamper1];
            v[2] = _mujocoFloatParams[simi_mujoco_joint_springdamper2];
            ar.xmlAddNode_floats("springdamper", v, 2);

            ar.xmlAddNode_float("armature", _mujocoFloatParams[simi_mujoco_joint_armature]);
            ar.xmlAddNode_float("margin", _mujocoFloatParams[simi_mujoco_joint_margin]);
            ar.xmlAddNode_int("dependentobjectid", _mujocoIntParams[simi_mujoco_joint_dependentobjectid]);
            for (size_t j = 0; j < 5; j++)
                v[j] = _mujocoFloatParams[simi_mujoco_joint_polycoef1 + int(j)];
            ar.xmlAddNode_floats("polycoef", v, 5);
            v[0] = _mujocoFloatParams[simi_mujoco_joint_pospid1];
            v[1] = _mujocoFloatParams[simi_mujoco_joint_pospid2];
            v[2] = _mujocoFloatParams[simi_mujoco_joint_pospid3];
            ar.xmlAddNode_floats("posPid", v, 3);
            ar.xmlPopNode();

            ar.xmlPopNode();

            ar.xmlPopNode();
        }
        else
        {
            bool usingDynCtrlMode = false;
            bool motorEnabled_old, ctrlEnabled_old, springMode_old;
            double mult = 1.0;
            if (ar.xmlGetNode_enum("type", _jointType, exhaustiveXml, "revolute", sim_joint_revolute,
                                   "prismatic", sim_joint_prismatic, "spherical", sim_joint_spherical))
            {
                if (_jointType == sim_joint_revolute)
                    mult = piValue / 180.0;
            }

            ar.xmlGetNode_enum("mode", _jointMode, exhaustiveXml, "kinematic", sim_jointmode_kinematic, "ik",
                               sim_jointmode_ik_deprecated, "dependent", sim_jointmode_dependent, "dynamic",
                               sim_jointmode_dynamic, "passive", sim_jointmode_kinematic, "force",
                               sim_jointmode_dynamic);

            if (ar.xmlGetNode_float("minPosition", _posMin, exhaustiveXml))
            {
                _posMin *= mult;

                if (ar.xmlGetNode_float("range", _posRange, exhaustiveXml))
                    _posRange *= mult;
            }

            if (ar.xmlGetNode_float("position", _pos, exhaustiveXml))
                _pos *= mult;
            if (ar.xmlGetNode_float("targetPosition", _targetPos, exhaustiveXml))
                _targetPos *= mult;
            if (ar.xmlGetNode_float("targetVelocity", _targetVel, exhaustiveXml))
                _targetVel *= mult;
            if (ar.xmlGetNode_floats("maxVelAccelJerk", _maxVelAccelJerk, 3, exhaustiveXml))
            {
                _maxVelAccelJerk[0] *= mult;
                _maxVelAccelJerk[1] *= mult;
                _maxVelAccelJerk[2] *= mult;
            }

            double a;
            if (ar.xmlGetNode_float("screwPitch", a, exhaustiveXml))
            { // for backward compatibility
                _screwLead = a * piValT2;
            }
            ar.xmlGetNode_float("screwLead", _screwLead, exhaustiveXml);

            if (exhaustiveXml)
                ar.xmlGetNode_floats("sphericalQuaternion", _sphericalTransf.data, 4);
            else
            {
                C3Vector euler;
                if (ar.xmlGetNode_floats("sphericalEuler", euler.data, 3, exhaustiveXml))
                {
                    euler(0) *= piValue / 180.0;
                    euler(1) *= piValue / 180.0;
                    euler(2) *= piValue / 180.0;
                    _sphericalTransf.setEulerAngles(euler);
                }
            }

            if (ar.xmlPushChildNode("switches", exhaustiveXml))
            {
                ar.xmlGetNode_bool("cyclic", _isCyclic, exhaustiveXml);
                ar.xmlGetNode_bool("enforceLimits", _enforceLimits, exhaustiveXml);
                ar.xmlGetNode_bool("hybridMode", _jointHasHybridFunctionality, exhaustiveXml);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("sizes", exhaustiveXml))
            {
                ar.xmlGetNode_float("length", _length, exhaustiveXml);
                ar.xmlGetNode_float("diameter", _diameter, exhaustiveXml);
                ar.xmlPopNode();
            }

            if (exhaustiveXml)
            {
                if (ar.xmlPushChildNode("objectColor", false))
                {
                    _color.serialize(ar, 0);
                    ar.xmlPopNode();
                }
            }
            else
            {
                int rgb[3];
                if (ar.xmlGetNode_ints("objectColor", rgb, 3, false))
                    _color.setColor(float(rgb[0]) / 255.1, float(rgb[1]) / 255.1, float(rgb[2]) / 255.1,
                                    sim_colorcomponent_ambient_diffuse);
            }

            if (ar.xmlPushChildNode("color", false))
            { // for backward compatibility
                if (exhaustiveXml)
                {
                    if (ar.xmlPushChildNode("part1"))
                    {
                        _color.serialize(ar, 0);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("part2"))
                    {
                        _color_removeSoon.serialize(ar, 0);
                        ar.xmlPopNode();
                    }
                }
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("part1", rgb, 3, exhaustiveXml))
                        _color.setColor(float(rgb[0]) / 255.0, float(rgb[1]) / 255.0, float(rgb[2]) / 255.0,
                                        sim_colorcomponent_ambient_diffuse);
                    if (ar.xmlGetNode_ints("part2", rgb, 3, exhaustiveXml))
                        _color_removeSoon.setColor(float(rgb[0]) / 255.0, float(rgb[1]) / 255.0, float(rgb[2]) / 255.0,
                                                   sim_colorcomponent_ambient_diffuse);
                }
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("ik", exhaustiveXml))
            {
                if (ar.xmlGetNode_float("maxStepSize", _maxStepSize_old, exhaustiveXml))
                    _maxStepSize_old *= mult;
                ar.xmlGetNode_float("weight", _ikWeight_old, exhaustiveXml);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("dependency", exhaustiveXml))
            {
                if (exhaustiveXml)
                    ar.xmlGetNode_int("jointHandle", _dependencyMasterJointHandle);
                else
                {
                    ar.xmlGetNode_string("dependentJointAlias", _dependencyJointLoadAlias, exhaustiveXml);
                    ar.xmlGetNode_string("dependentJoint", _dependencyJointLoadName_old, exhaustiveXml);
                }
                ar.xmlGetNode_float("offset_m_or_rad", _dependencyJointOffset, exhaustiveXml);
                ar.xmlGetNode_float("mult_m_or_rad", _dependencyJointMult, exhaustiveXml);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("dynamics", exhaustiveXml))
            {
                if (ar.xmlGetNode_int("ctrlMode", _dynCtrlMode, exhaustiveXml))
                    usingDynCtrlMode = true;

                ar.xmlGetNode_enum("posController", _dynPositionCtrlType, exhaustiveXml, "pid", 0, "motionProfile", 1);
                ar.xmlGetNode_enum("velController", _dynVelocityCtrlType, exhaustiveXml, "none", 0, "motionProfile", 1);

                ar.xmlGetNode_float("maxForce", _targetForce, exhaustiveXml);
                if (_targetVel * _targetForce < 0.0)
                    _targetForce = -_targetForce;
                double val;
                if (ar.xmlGetNode_float("upperVelocityLimit", val,
                                        false)) // for backward compatibility (V4.3 and earlier)
                {
                    if ((_jointMode == sim_jointmode_dynamic) || _jointHasHybridFunctionality)
                        _maxVelAccelJerk[0] = val * mult;
                }
                if (ar.xmlGetNode_float("targetPosition", _targetPos,
                                        false)) // for backward compatibility (V4.3 and earlier)
                    _targetPos *= mult;
                if (ar.xmlGetNode_float("targetVelocity", _targetVel,
                                        false)) // for backward compatibility (V4.3 and earlier)
                    _targetVel *= mult;
                double P, I, D;
                ar.xmlGetNode_3float("pidValues", P, I, D, exhaustiveXml);
                setPid_old(P, I, D);
                ar.xmlGetNode_2float("kcValues", _dynCtrl_kc[0], _dynCtrl_kc[1], exhaustiveXml);

                if (ar.xmlPushChildNode("switches", exhaustiveXml))
                {
                    ar.xmlGetNode_bool("motorEnabled", motorEnabled_old,
                                       false); // for backward compatibility (V4.3 and earlier)
                    ar.xmlGetNode_bool("controlLoopEnabled", ctrlEnabled_old,
                                       false); // for backward compatibility (V4.3 and earlier)
                    ar.xmlGetNode_bool("springMode", springMode_old,
                                       false); // for backward compatibility (V4.3 and earlier)
                    ar.xmlGetNode_bool("lockInVelocityControl", _motorLock, exhaustiveXml);
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("engines", exhaustiveXml))
                {
                    double v;
                    int vi;
                    bool vb;
                    if (ar.xmlPushChildNode("bullet", exhaustiveXml))
                    {
                        double w[3];
                        if (ar.xmlGetNode_float("stoperp", v, exhaustiveXml))
                            _bulletFloatParams[simi_bullet_joint_stoperp] = v;
                        if (ar.xmlGetNode_float("stopcfm", v, exhaustiveXml))
                            _bulletFloatParams[simi_bullet_joint_stopcfm] = v;
                        if (ar.xmlGetNode_float("normalcfm", v, exhaustiveXml))
                            _bulletFloatParams[simi_bullet_joint_normalcfm] = v;
                        if (ar.xmlGetNode_floats("posPid", w, 3, exhaustiveXml))
                        {
                            _bulletFloatParams[simi_bullet_joint_pospid1] = w[0];
                            _bulletFloatParams[simi_bullet_joint_pospid2] = w[1];
                            _bulletFloatParams[simi_bullet_joint_pospid3] = w[2];
                        }
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("ode", exhaustiveXml))
                    {
                        double w[3];
                        if (ar.xmlGetNode_float("stoperp", v, exhaustiveXml))
                            _odeFloatParams[simi_ode_joint_stoperp] = v;
                        if (ar.xmlGetNode_float("stopcfm", v, exhaustiveXml))
                            _odeFloatParams[simi_ode_joint_stopcfm] = v;
                        if (ar.xmlGetNode_float("bounce", v, exhaustiveXml))
                            _odeFloatParams[simi_ode_joint_bounce] = v;
                        if (ar.xmlGetNode_float("fudgefactor", v, exhaustiveXml))
                            _odeFloatParams[simi_ode_joint_fudgefactor] = v;
                        if (ar.xmlGetNode_float("normalcfm", v, exhaustiveXml))
                            _odeFloatParams[simi_ode_joint_normalcfm] = v;
                        if (ar.xmlGetNode_floats("posPid", w, 3, exhaustiveXml))
                        {
                            _odeFloatParams[simi_ode_joint_pospid1] = w[0];
                            _odeFloatParams[simi_ode_joint_pospid2] = w[1];
                            _odeFloatParams[simi_ode_joint_pospid3] = w[2];
                        }
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("vortex", exhaustiveXml))
                    {
                        double w[3];
                        if (ar.xmlGetNode_float("lowerlimitdamping", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_lowerlimitdamping] = v;
                        if (ar.xmlGetNode_float("upperlimitdamping", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_upperlimitdamping] = v;
                        if (ar.xmlGetNode_float("lowerlimitstiffness", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_lowerlimitstiffness] = v;
                        if (ar.xmlGetNode_float("upperlimitstiffness", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_upperlimitstiffness] = v;
                        if (ar.xmlGetNode_float("lowerlimitrestitution", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_lowerlimitrestitution] = v;
                        if (ar.xmlGetNode_float("upperlimitrestitution", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_upperlimitrestitution] = v;
                        if (ar.xmlGetNode_float("lowerlimitmaxforce", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_lowerlimitmaxforce] = v;
                        if (ar.xmlGetNode_float("upperlimitmaxforce", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_upperlimitmaxforce] = v;
                        if (ar.xmlGetNode_float("motorconstraintfrictioncoeff", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_motorconstraintfrictioncoeff] = v;
                        if (ar.xmlGetNode_float("motorconstraintfrictionmaxforce", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_motorconstraintfrictionmaxforce] = v;
                        if (ar.xmlGetNode_float("motorconstraintfrictionloss", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_motorconstraintfrictionloss] = v;
                        if (ar.xmlGetNode_float("p0loss", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p0loss] = v;
                        if (ar.xmlGetNode_float("p0stiffness", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p0stiffness] = v;
                        if (ar.xmlGetNode_float("p0damping", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p0damping] = v;
                        if (ar.xmlGetNode_float("p0frictioncoeff", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p0frictioncoeff] = v;
                        if (ar.xmlGetNode_float("p0frictionmaxforce", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p0frictionmaxforce] = v;
                        if (ar.xmlGetNode_float("p0frictionloss", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p0frictionloss] = v;
                        if (ar.xmlGetNode_float("p1loss", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p1loss] = v;
                        if (ar.xmlGetNode_float("p1stiffness", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p1stiffness] = v;
                        if (ar.xmlGetNode_float("p1damping", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p1damping] = v;
                        if (ar.xmlGetNode_float("p1frictioncoeff", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p1frictioncoeff] = v;
                        if (ar.xmlGetNode_float("p1frictionmaxforce", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p1frictionmaxforce] = v;
                        if (ar.xmlGetNode_float("p1frictionloss", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p1frictionloss] = v;
                        if (ar.xmlGetNode_float("p2loss", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p2loss] = v;
                        if (ar.xmlGetNode_float("p2stiffness", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p2stiffness] = v;
                        if (ar.xmlGetNode_float("p2damping", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p2damping] = v;
                        if (ar.xmlGetNode_float("p2frictioncoeff", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p2frictioncoeff] = v;
                        if (ar.xmlGetNode_float("p2frictionmaxforce", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p2frictionmaxforce] = v;
                        if (ar.xmlGetNode_float("p2frictionloss", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_p2frictionloss] = v;
                        if (ar.xmlGetNode_float("a0loss", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a0loss] = v;
                        if (ar.xmlGetNode_float("a0stiffness", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a0stiffness] = v;
                        if (ar.xmlGetNode_float("a0damping", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a0damping] = v;
                        if (ar.xmlGetNode_float("a0frictioncoeff", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a0frictioncoeff] = v;
                        if (ar.xmlGetNode_float("a0frictionmaxforce", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a0frictionmaxforce] = v;
                        if (ar.xmlGetNode_float("a0frictionloss", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a0frictionloss] = v;
                        if (ar.xmlGetNode_float("a1loss", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a1loss] = v;
                        if (ar.xmlGetNode_float("a1stiffness", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a1stiffness] = v;
                        if (ar.xmlGetNode_float("a1damping", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a1damping] = v;
                        if (ar.xmlGetNode_float("a1frictioncoeff", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a1frictioncoeff] = v;
                        if (ar.xmlGetNode_float("a1frictionmaxforce", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a1frictionmaxforce] = v;
                        if (ar.xmlGetNode_float("a1frictionloss", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a1frictionloss] = v;
                        if (ar.xmlGetNode_float("a2loss", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a2loss] = v;
                        if (ar.xmlGetNode_float("a2stiffness", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a2stiffness] = v;
                        if (ar.xmlGetNode_float("a2damping", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a2damping] = v;
                        if (ar.xmlGetNode_float("a2frictioncoeff", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a2frictioncoeff] = v;
                        if (ar.xmlGetNode_float("a2frictionmaxforce", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a2frictionmaxforce] = v;
                        if (ar.xmlGetNode_float("a2frictionloss", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_a2frictionloss] = v;
                        if (ar.xmlGetNode_float("dependencyfactor", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_dependencyfactor] = v;
                        if (ar.xmlGetNode_float("dependencyoffset", v, exhaustiveXml))
                            _vortexFloatParams[simi_vortex_joint_dependencyoffset] = v;
                        if (ar.xmlGetNode_floats("posPid", w, 3, exhaustiveXml))
                        {
                            _vortexFloatParams[simi_vortex_joint_pospid1] = w[0];
                            _vortexFloatParams[simi_vortex_joint_pospid2] = w[1];
                            _vortexFloatParams[simi_vortex_joint_pospid3] = w[2];
                        }

                        if (ar.xmlGetNode_int("relaxationenabledbc", vi, exhaustiveXml))
                            _vortexIntParams[simi_vortex_joint_relaxationenabledbc] = vi;
                        if (ar.xmlGetNode_int("frictionenabledbc", vi, exhaustiveXml))
                            _vortexIntParams[simi_vortex_joint_frictionenabledbc] = vi;
                        if (ar.xmlGetNode_int("frictionproportionalbc", vi, exhaustiveXml))
                            _vortexIntParams[simi_vortex_joint_frictionproportionalbc] = vi;
                        if (ar.xmlGetNode_int("objectid", vi, exhaustiveXml))
                            _vortexIntParams[simi_vortex_joint_objectid] = vi;
                        if (ar.xmlGetNode_int("dependentobjectid", vi, exhaustiveXml))
                            _vortexIntParams[simi_vortex_joint_dependentobjectid] = vi;
                        if (ar.xmlGetNode_bool("motorfrictionenabled", vb, exhaustiveXml))
                        {
                            _vortexIntParams[simi_vortex_joint_bitcoded] |= simi_vortex_joint_motorfrictionenabled;
                            if (!vb)
                                _vortexIntParams[simi_vortex_joint_bitcoded] -= simi_vortex_joint_motorfrictionenabled;
                        }
                        if (ar.xmlGetNode_bool("proportionalmotorfriction", vb, exhaustiveXml))
                        {
                            _vortexIntParams[simi_vortex_joint_bitcoded] |= simi_vortex_joint_proportionalmotorfriction;
                            if (!vb)
                                _vortexIntParams[simi_vortex_joint_bitcoded] -= simi_vortex_joint_proportionalmotorfriction;
                        }
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("newton", exhaustiveXml))
                    {
                        double w[3];
                        if (ar.xmlGetNode_float("dependencyfactor", v, exhaustiveXml))
                            _newtonFloatParams[simi_newton_joint_dependencyfactor] = v;
                        if (ar.xmlGetNode_float("dependencyoffset", v, exhaustiveXml))
                            _newtonFloatParams[simi_newton_joint_dependencyoffset] = v;

                        if (ar.xmlGetNode_int("objectid", vi, exhaustiveXml))
                            _newtonIntParams[simi_newton_joint_objectid] = vi;
                        if (ar.xmlGetNode_int("dependentobjectid", vi, exhaustiveXml))
                            _newtonIntParams[simi_newton_joint_dependentobjectid] = vi;
                        if (ar.xmlGetNode_floats("posPid", w, 3, exhaustiveXml))
                        {
                            _newtonFloatParams[simi_newton_joint_pospid1] = w[0];
                            _newtonFloatParams[simi_newton_joint_pospid2] = w[1];
                            _newtonFloatParams[simi_newton_joint_pospid3] = w[2];
                        }
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("mujoco", exhaustiveXml))
                    {
                        double w[5];
                        if (ar.xmlGetNode_floats("solreflimit", w, 2, exhaustiveXml))
                        {
                            _mujocoFloatParams[simi_mujoco_joint_solreflimit1] = w[0];
                            _mujocoFloatParams[simi_mujoco_joint_solreflimit2] = w[1];
                        }
                        if (ar.xmlGetNode_floats("solimplimit", w, 5, exhaustiveXml))
                        {
                            for (size_t j = 0; j < 5; j++)
                                _mujocoFloatParams[simi_mujoco_joint_solimplimit1 + int(j)] = w[j];
                        }
                        if (ar.xmlGetNode_floats("solreffriction", w, 2, exhaustiveXml))
                        {
                            _mujocoFloatParams[simi_mujoco_joint_solreffriction1] = w[0];
                            _mujocoFloatParams[simi_mujoco_joint_solreffriction2] = w[1];
                        }
                        if (ar.xmlGetNode_floats("solimpfriction", w, 5, exhaustiveXml))
                        {
                            for (size_t j = 0; j < 5; j++)
                                _mujocoFloatParams[simi_mujoco_joint_solimpfriction1 + int(j)] = w[j];
                        }
                        if (ar.xmlGetNode_float("frictionloss", v, exhaustiveXml))
                            _mujocoFloatParams[simi_mujoco_joint_frictionloss] = v;
                        if (ar.xmlGetNode_float("stiffness", v, exhaustiveXml))
                            _mujocoFloatParams[simi_mujoco_joint_stiffness] = v;
                        if (ar.xmlGetNode_float("damping", v, exhaustiveXml))
                            _mujocoFloatParams[simi_mujoco_joint_damping] = v;
                        if (ar.xmlGetNode_float("springref", v, exhaustiveXml))
                            _mujocoFloatParams[simi_mujoco_joint_springref] = v;
                        if (ar.xmlGetNode_floats("springdamper", w, 2, exhaustiveXml))
                        {
                            _mujocoFloatParams[simi_mujoco_joint_springdamper1] = w[0];
                            _mujocoFloatParams[simi_mujoco_joint_springdamper2] = w[1];
                        }
                        if (ar.xmlGetNode_float("armature", v, exhaustiveXml))
                            _mujocoFloatParams[simi_mujoco_joint_armature] = v;
                        if (ar.xmlGetNode_float("margin", v, exhaustiveXml))
                            _mujocoFloatParams[simi_mujoco_joint_margin] = v;
                        if (ar.xmlGetNode_floats("polycoef", w, 5, exhaustiveXml))
                        {
                            for (size_t j = 0; j < 5; j++)
                                _mujocoFloatParams[simi_mujoco_joint_polycoef1 + int(j)] = w[j];
                        }
                        if (ar.xmlGetNode_int("dependentobjectid", vi, exhaustiveXml))
                            _mujocoIntParams[simi_mujoco_joint_dependentobjectid] = vi;

                        if (ar.xmlGetNode_floats("posPid", w, 3, exhaustiveXml))
                        {
                            _mujocoFloatParams[simi_mujoco_joint_pospid1] = w[0];
                            _mujocoFloatParams[simi_mujoco_joint_pospid2] = w[1];
                            _mujocoFloatParams[simi_mujoco_joint_pospid3] = w[2];
                        }

                        ar.xmlPopNode();
                    }
                    ar.xmlPopNode();
                }

                ar.xmlPopNode();
            }
            if (!usingDynCtrlMode)
            {
                _dynCtrlMode = sim_jointdynctrl_free;
                if (motorEnabled_old)
                {
                    _dynCtrlMode = sim_jointdynctrl_velocity;
                    if (ctrlEnabled_old)
                    {
                        _dynCtrlMode = sim_jointdynctrl_positioncb;
                        if (springMode_old)
                            _dynCtrlMode = sim_jointdynctrl_springcb;
                    }
                }
            }
            _fixVortexInfVals();
            computeBoundingBox();
        }
    }
}

void CJoint::performObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performObjectLoadingMapping(map, opType);
    _dependencyMasterJointHandle = CWorld::getLoadingMapping(map, _dependencyMasterJointHandle);

    // following to support the old way joint dependencies for the dynamic engines were specified:
    _mujocoIntParams[simi_mujoco_joint_objectid] = -1;
    int masterJ = CWorld::getLoadingMapping(map, _mujocoIntParams[simi_mujoco_joint_dependentobjectid]);
    double off = _mujocoFloatParams[simi_mujoco_joint_polycoef1];
    double mult = _mujocoFloatParams[simi_mujoco_joint_polycoef2];
    _mujocoIntParams[simi_mujoco_joint_dependentobjectid] = -1;

    _vortexIntParams[simi_vortex_joint_objectid] = -1;
    if (masterJ == -1)
    {
        masterJ = CWorld::getLoadingMapping(map, _vortexIntParams[simi_vortex_joint_dependentobjectid]);
        off = _vortexFloatParams[simi_vortex_joint_dependencyoffset];
        mult = _vortexFloatParams[simi_vortex_joint_dependencyfactor];
    }
    _vortexIntParams[simi_vortex_joint_dependentobjectid] = -1;
    _vortexFloatParams[simi_vortex_joint_dependencyoffset] = 0.0;
    _vortexFloatParams[simi_vortex_joint_dependencyfactor] = 0.0;

    _newtonIntParams[simi_newton_joint_objectid] = -1;
    if (masterJ == -1)
    {
        masterJ = CWorld::getLoadingMapping(map, _newtonIntParams[simi_newton_joint_dependentobjectid]);
        off = _newtonFloatParams[simi_newton_joint_dependencyoffset];
        mult = _newtonFloatParams[simi_newton_joint_dependencyfactor];
    }
    _newtonIntParams[simi_newton_joint_dependentobjectid] = -1;
    _newtonFloatParams[simi_newton_joint_dependencyoffset] = 0.0;
    _newtonFloatParams[simi_newton_joint_dependencyfactor] = 0.0;

    if ((masterJ != -1) && (_dependencyMasterJointHandle == -1))
    {
        _dependencyMasterJointHandle = masterJ;
        _dependencyJointOffset = off;
        _dependencyJointMult = mult;
        _jointMode = sim_jointmode_dependent;
    }
}

void CJoint::setJointMode(int theMode)
{
    bool retVal = setJointMode_noDynMotorTargetPosCorrection(theMode);
    if (retVal)
        setTargetPosition(getPosition());
}

bool CJoint::setJointMode_noDynMotorTargetPosCorrection(int newMode)
{
    if (_jointType == sim_joint_spherical)
    {
        if ((newMode == sim_jointmode_motion_deprecated) || (newMode == sim_jointmode_dependent) ||
            (newMode == sim_jointmode_reserved_previously_ikdependent))
            newMode = _jointMode; // above modes forbidden with spherical joints
    }

    bool diff = (_jointMode != newMode);
    if (diff)
    {
        _jointMode = newMode;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propJoint_jointMode.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyInt(cmd, _jointMode);
            App::worldContainer->pushEvent();
        }
        if (_jointMode == sim_jointmode_dependent)
            setDynCtrlMode(sim_jointdynctrl_free); // 21.08.2024: e.g. dependent joints need to be free when master joint is dyn. enabled
        _setJointMode_sendOldIk(_jointMode);
        if ((_jointMode != sim_jointmode_dependent) && (_jointMode != sim_jointmode_reserved_previously_ikdependent))
            setDependencyMasterJointHandle(-1);
        if (_jointMode == sim_jointmode_dynamic)
        {
            if ((_dynCtrlMode == sim_jointdynctrl_spring) || (_dynCtrlMode == sim_jointdynctrl_springcb) ||
                (_dynCtrlMode == sim_jointdynctrl_force))
                setTargetVelocity(1000.0); // just a very high value
            setHybridFunctionality_old(false);
            setScrewLead(0.0);
            if (_jointType == sim_joint_spherical)
                setInterval(0.0, piValue);
            _dynVelCtrl_currentVelAccel[0] = double(_velCalc_vel);
            _dynVelCtrl_currentVelAccel[1] = 0.0;
            _dynCtrl_pid_cumulErr = 0.0;
        }
        if (_jointMode == sim_jointmode_motion_deprecated)
        {
            _velocity_DEPRECATED = 0.0;
            _targetVel = 0.0;
            _explicitHandling_DEPRECATED = true;
            _unlimitedAcceleration_DEPRECATED = true;
            _invertTargetVelocityAtLimits_DEPRECATED = false;
        }
        App::currentWorld->sceneObjects->actualizeObjectInformation();

        if ((_jointMode == sim_jointmode_dependent) || (_jointMode == sim_jointmode_reserved_previously_ikdependent))
            updateSelfAsSlave();
        else
            setPosition(getPosition());
    }
    return (diff);
}

void CJoint::updateSelfAsSlave()
{
    if (_dependencyMasterJointHandle != -1)
    {
        CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(_dependencyMasterJointHandle);
        it->updateSelfAsSlave();
    }
    else
        setPosition(getPosition());
}

void CJoint::_setJointMode_sendOldIk(int theMode) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle != -1)
    {
        if ((theMode == sim_jointmode_reserved_previously_ikdependent) || (theMode == sim_jointmode_dependent) ||
            (theMode == sim_jointmode_hybrid_deprecated))
            theMode = 2; // actually ik_jointmode_ik
        App::worldContainer->pluginContainer->oldIkPlugin_setJointMode(_ikPluginCounterpartHandle, theMode);
    }
}

void CJoint::setSphericalTransformation(const C4Vector& tr)
{ // spherical joints don't have a range anymore since 22.10.2022 (didn't really make sense)
    bool diff = (_sphericalTransf != tr);
    if (diff)
    {
        _sphericalTransf = tr;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propJoint_quaternion.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            double q[4];
            _sphericalTransf.getData(q, true);
            ev->appendKeyDoubleArray(cmd, q, 4);
            double p[7];
            getIntrinsicTransformation(true).getData(p, true);
            ev->appendKeyDoubleArray(propJoint_intrinsicPose.name, p, 7);
            App::worldContainer->pushEvent();
        }
        _setSphericalTransformation_sendOldIk(_sphericalTransf);
    }
}

void CJoint::_setSphericalTransformation_sendOldIk(const C4Vector& tr) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if ((_ikPluginCounterpartHandle != -1) && (_jointType == sim_joint_spherical))
        App::worldContainer->pluginContainer->oldIkPlugin_setSphericalJointQuaternion(_ikPluginCounterpartHandle, tr);
}

void CJoint::setMaxStepSize_old(double stepS)
{
    if (_jointType == sim_joint_revolute)
        tt::limitValue(0.01 * degToRad, 100000.0, stepS); // high number for screws!
    if (_jointType == sim_joint_prismatic)
        tt::limitValue(0.0001, 1000.0, stepS);
    if (_jointType == sim_joint_spherical)
        tt::limitValue(0.01 * degToRad, piValue, stepS);
    bool diff = (_maxStepSize_old != stepS);
    if (diff)
    {
        _maxStepSize_old = stepS;
        _setMaxStepSize_sendOldIk(stepS);
    }
}

void CJoint::_setMaxStepSize_sendOldIk(double stepS) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle != -1)
        App::worldContainer->pluginContainer->oldIkPlugin_setJointMaxStepSize(_ikPluginCounterpartHandle,
                                                                              _maxStepSize_old);
}

void CJoint::setIKWeight_old(double newWeight)
{
    newWeight = tt::getLimitedFloat(-1.0, 1.0, newWeight);
    bool diff = (_ikWeight_old != newWeight);
    if (diff)
    {
        _ikWeight_old = newWeight;
        _setIkWeight_sendOldIk(newWeight);
    }
}

void CJoint::_setIkWeight_sendOldIk(double newWeight) const
{ // Overridden from _CJoint_
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle != -1)
        App::worldContainer->pluginContainer->oldIkPlugin_setJointIkWeight(_ikPluginCounterpartHandle, _ikWeight_old);
}

void CJoint::setPosition(double pos, const CJoint* masterJoint /*=nullptr*/, bool setDirect /*=false*/)
{
    if (masterJoint != nullptr)
    {
        if (_dependencyMasterJointHandle == masterJoint->getObjectHandle())
            pos = _dependencyJointOffset + _dependencyJointMult * masterJoint->getPosition();
    }
    else
    {
        if (_dependencyMasterJointHandle == -1)
        {
            if (!setDirect)
            {
                if (_isCyclic)
                    pos = tt::getNormalizedAngle(pos);
                else
                {
                    if (pos > _posMin + _posRange)
                        pos = _posMin + _posRange;
                    if (pos < _posMin)
                        pos = _posMin;
                }
            }
        }
    }

    bool diff = (_pos != pos);
    if (diff)
    {
        _pos = pos;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propJoint_position.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _pos);
            double p[7];
            getIntrinsicTransformation(true).getData(p, true);
            ev->appendKeyDoubleArray(propJoint_intrinsicPose.name, p, 7);
            App::worldContainer->pushEvent();
        }
        _setPosition_sendOldIk(pos);
        setVelocity_DEPRECATED(getVelocity_DEPRECATED());
    }
    // Execute this even if this joint didn't change. Maybe a slave joint change its parameters (off or mult)
    for (size_t i = 0; i < _directDependentJoints.size(); i++)
        _directDependentJoints[i]->setPosition(0.0, this);
}

void CJoint::_setPosition_sendOldIk(double pos) const
{ // Overriden from _CJoint_
    // Synchronize with IK plugin:
    if ((_ikPluginCounterpartHandle != -1) && (_jointType != sim_joint_spherical))
        App::worldContainer->pluginContainer->oldIkPlugin_setJointPosition(_ikPluginCounterpartHandle, pos);
}

void CJoint::announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceObjectWillBeErased(object, copyBuffer);
    if (_dependencyMasterJointHandle == object->getObjectHandle())
        setDependencyMasterJointHandle(-1);
    if (_vortexIntParams[simi_vortex_joint_dependentobjectid] == object->getObjectHandle()) // that's the Vortex dependency joint
        _vortexIntParams[simi_vortex_joint_dependentobjectid] = -1;
    if (_newtonIntParams[simi_newton_joint_dependentobjectid] == object->getObjectHandle()) // that's the Newton dependency joint
        _newtonIntParams[simi_newton_joint_dependentobjectid] = -1;
    if (_mujocoIntParams[simi_mujoco_joint_dependentobjectid] == object->getObjectHandle()) // that's the Mujoco dependency joint
        _mujocoIntParams[simi_mujoco_joint_dependentobjectid] = -1;

    for (size_t i = 0; i < _directDependentJoints.size(); i++)
    {
        if (_directDependentJoints[i] == object)
            _directDependentJoints.erase(_directDependentJoints.begin() + i);
    }
}

void CJoint::announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID, copyBuffer);
}

void CJoint::buildOrUpdate_oldIk()
{
    // Build IK plugin counterpart:
    _ikPluginCounterpartHandle = App::worldContainer->pluginContainer->oldIkPlugin_createJoint(_jointType);

    // Update the remote sceneObject:
    CSceneObject::buildOrUpdate_oldIk();

    // Update the remote joint:
    _setPositionIntervalMin_sendOldIk(_posMin);
    _setPositionIntervalRange_sendOldIk(_posRange);
    _setPositionIsCyclic_sendOldIk(_isCyclic);
    _setScrewPitch_sendOldIk(_screwLead / piValT2);
    _setIkWeight_sendOldIk(_ikWeight_old);
    _setMaxStepSize_sendOldIk(_maxStepSize_old);
    _setPosition_sendOldIk(_pos);
    _setSphericalTransformation_sendOldIk(_sphericalTransf);
    _setJointMode_sendOldIk(_jointMode);
}

void CJoint::connect_oldIk()
{
    CSceneObject::connect_oldIk();

    _setDependencyJointHandle_sendOldIk(_dependencyMasterJointHandle);
    _setDependencyJointMult_sendOldIk(_dependencyJointMult);
    _setDependencyJointOffset_sendOldIk(_dependencyJointOffset);
}

int CJoint::getDynCtrlMode() const
{
    return (_dynCtrlMode);
}

void CJoint::setDynCtrlMode(int mode)
{
    if (mode != _dynCtrlMode)
    {
        _dynCtrlMode = mode;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propJoint_dynCtrlMode.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyInt(cmd, _dynCtrlMode);
            App::worldContainer->pushEvent();
        }
        if ((_dynCtrlMode == sim_jointdynctrl_spring) || (_dynCtrlMode == sim_jointdynctrl_springcb) ||
            (_dynCtrlMode == sim_jointdynctrl_force))
            setTargetVelocity(1000.0); // just a very high value
        if (_dynCtrlMode == sim_jointdynctrl_velocity)
        {
            if (_jointType == sim_joint_prismatic)
                setTargetVelocity(0.1);
            else
                setTargetVelocity(piValD2);
        }
        _dynVelCtrl_currentVelAccel[0] = double(_velCalc_vel);
        _dynVelCtrl_currentVelAccel[1] = 0.0;
        _dynCtrl_pid_cumulErr = 0.0;
    }
}

int CJoint::getDynVelCtrlType() const
{
    return _dynVelocityCtrlType;
}

int CJoint::getDynPosCtrlType() const
{
    return _dynPositionCtrlType;
}

void CJoint::setDynVelCtrlType(int mode)
{
    if (mode != _dynVelocityCtrlType)
    {
        if (_dynCtrlMode == sim_jointdynctrl_velocity)
        {
            _dynVelCtrl_currentVelAccel[0] = double(_velCalc_vel);
            _dynVelCtrl_currentVelAccel[1] = 0.0;
        }
        _dynVelocityCtrlType = mode;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propJoint_dynVelMode.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyInt(cmd, _dynVelocityCtrlType);
            App::worldContainer->pushEvent();
        }
    }
}

void CJoint::setDynPosCtrlType(int mode)
{
    if (mode != _dynPositionCtrlType)
    {
        if (_dynCtrlMode == sim_jointdynctrl_position)
        {
            _dynPosCtrl_currentVelAccel[0] = double(_velCalc_vel);
            _dynPosCtrl_currentVelAccel[1] = 0.0;
            _dynCtrl_pid_cumulErr = 0.0;
        }
        _dynPositionCtrlType = mode;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propJoint_dynPosMode.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyInt(cmd, _dynPositionCtrlType);
            App::worldContainer->pushEvent();
        }
    }
}

double CJoint::getTargetVelocity() const
{
    return (_targetVel);
}

bool CJoint::getMotorLock() const
{ // deprecated. Should not be used anymore
    return (_motorLock);
}

void CJoint::getMaxVelAccelJerk(double maxVelAccelJerk[3]) const
{
    maxVelAccelJerk[0] = _maxVelAccelJerk[0];
    maxVelAccelJerk[1] = _maxVelAccelJerk[1];
    maxVelAccelJerk[2] = _maxVelAccelJerk[2];
}

void CJoint::setMaxVelAccelJerk(const double maxVelAccelJerk[3])
{
    bool diff = ((_maxVelAccelJerk[0] != maxVelAccelJerk[0]) || (_maxVelAccelJerk[1] != maxVelAccelJerk[1]) || (_maxVelAccelJerk[2] != maxVelAccelJerk[2]));
    if (diff)
    {
        _maxVelAccelJerk[0] = maxVelAccelJerk[0];
        _maxVelAccelJerk[1] = maxVelAccelJerk[1];
        _maxVelAccelJerk[2] = maxVelAccelJerk[2];
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propJoint_maxVelAccelJerk.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDoubleArray(cmd, _maxVelAccelJerk, 3);
            App::worldContainer->pushEvent();
        }
    }
}

double CJoint::getTargetForce(bool signedValue) const
{
    double retVal = _targetForce;
    if (!signedValue)
        retVal = fabs(retVal);
    return (retVal);
}

bool CJoint::getHybridFunctionality_old() const
{
    return (_jointHasHybridFunctionality);
}

double CJoint::getEngineFloatParam_old(int what, bool* ok) const
{
    if (ok != nullptr)
        ok[0] = true;
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_float, indexWithArrays);
    if (prop.size() > 0)
    {
        double v;
        if (getFloatProperty(prop.c_str(), v) > 0)
            return v;
    }
    prop = _enumToProperty(what, sim_propertytype_vector2, indexWithArrays);
    if (prop.size() > 0)
    {
        double v[2];
        if (getVector2Property(prop.c_str(), v) > 0)
            return v[indexWithArrays];
    }
    prop = _enumToProperty(what, sim_propertytype_vector3, indexWithArrays);
    if (prop.size() > 0)
    {
        C3Vector v;
        if (getVector3Property(prop.c_str(), v) > 0)
            return v(indexWithArrays);
    }
    prop = _enumToProperty(what, sim_propertytype_floatarray, indexWithArrays);
    if (prop.size() > 0)
    {
        std::vector<double> v;
        if (getFloatArrayProperty(prop.c_str(), v) > 0)
            return v[indexWithArrays];
    }

    if (what == sim_vortex_joint_dependencyoffset)
        return (_dependencyJointOffset);
    if (what == sim_vortex_joint_dependencyfactor)
        return (_dependencyJointMult);
    if (what == sim_newton_joint_dependencyoffset)
        return (_dependencyJointOffset);
    if (what == sim_newton_joint_dependencyfactor)
        return (_dependencyJointMult);
    if (what == sim_mujoco_joint_polycoef1)
        return (_dependencyJointOffset);
    if (what == sim_mujoco_joint_polycoef2)
        return (_dependencyJointMult);

    if (ok != nullptr)
        ok[0] = false;
    return 0.0;
}

int CJoint::getEngineIntParam_old(int what, bool* ok) const
{
    if (ok != nullptr)
        ok[0] = true;
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_int, indexWithArrays);
    if (prop.size() > 0)
    {
        int v;
        if (getIntProperty(prop.c_str(), v) > 0)
            return v;
    }
    if (ok != nullptr)
        ok[0] = false;
    return 0;
}

bool CJoint::getEngineBoolParam_old(int what, bool* ok) const
{
    if (ok != nullptr)
        ok[0] = true;
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_bool, indexWithArrays);
    if (prop.size() > 0)
    {
        bool v;
        if (getBoolProperty(prop.c_str(), v) > 0)
            return v;
    }
    if (ok != nullptr)
        ok[0] = false;
    return false;
}

void CJoint::getVortexFloatParams(std::vector<double>& p) const
{
    p.assign(_vortexFloatParams.begin(), _vortexFloatParams.end());
}

void CJoint::getVortexIntParams(std::vector<int>& p) const
{
    p.assign(_vortexIntParams.begin(), _vortexIntParams.end());
}

void CJoint::getNewtonFloatParams(std::vector<double>& p) const
{
    p.assign(_newtonFloatParams.begin(), _newtonFloatParams.end());
}

void CJoint::getNewtonIntParams(std::vector<int>& p) const
{
    p.assign(_newtonIntParams.begin(), _newtonIntParams.end());
}

int CJoint::getVortexDependentJointId() const
{
    return (_vortexIntParams[5]);
}

int CJoint::getNewtonDependentJointId() const
{
    return (_newtonIntParams[1]);
}
int CJoint::getMujocoDependentJointId() const
{
    return (_mujocoIntParams[1]);
}

double CJoint::getPosition() const
{
    return (_pos);
}

double CJoint::getTargetPosition() const
{
    return (_targetPos);
}

C7Vector CJoint::getIntrinsicTransformation(bool includeDynErrorComponent, bool* available /*=nullptr*/) const
{ // Overridden from CSceneObject
    C7Vector jointTr;
    if (getJointType() == sim_joint_revolute)
    {
        jointTr.Q.setAngleAndAxis(_pos, C3Vector(0.0, 0.0, 1.0));
        jointTr.X(0) = 0.0;
        jointTr.X(1) = 0.0;
        jointTr.X(2) = _pos * getScrewLead() / piValT2;
    }
    if (getJointType() == sim_joint_prismatic)
    {
        jointTr.Q.setIdentity();
        jointTr.X(0) = 0.0;
        jointTr.X(1) = 0.0;
        jointTr.X(2) = _pos;
    }
    if (getJointType() == sim_joint_spherical)
    {
        jointTr.Q = _sphericalTransf;
        jointTr.X.clear();
    }
    if (includeDynErrorComponent)
        jointTr = jointTr * _intrinsicTransformationError;
    if (available != nullptr)
        available[0] = true;
    return (jointTr);
}

C7Vector CJoint::getFullLocalTransformation() const
{ // Overridden from CSceneObject
    return (_localTransformation * getIntrinsicTransformation(true));
}

void CJoint::getPid(double& p_param, double& i_param, double& d_param, int engine /*=-1 --> current engine*/) const
{
    if (engine == -1)
        engine = App::currentWorld->dynamicsContainer->getDynamicEngineType(nullptr);
    if (engine == sim_physics_bullet)
    {
        p_param = _bulletFloatParams[simi_bullet_joint_pospid1];
        i_param = _bulletFloatParams[simi_bullet_joint_pospid2];
        d_param = _bulletFloatParams[simi_bullet_joint_pospid3];
    }
    if (engine == sim_physics_ode)
    {
        p_param = _odeFloatParams[simi_ode_joint_pospid1];
        i_param = _odeFloatParams[simi_ode_joint_pospid2];
        d_param = _odeFloatParams[simi_ode_joint_pospid3];
    }
    if (engine == sim_physics_vortex)
    {
        p_param = _vortexFloatParams[simi_vortex_joint_pospid1];
        i_param = _vortexFloatParams[simi_vortex_joint_pospid2];
        d_param = _vortexFloatParams[simi_vortex_joint_pospid3];
    }
    if (engine == sim_physics_newton)
    {
        p_param = _newtonFloatParams[simi_newton_joint_pospid1];
        i_param = _newtonFloatParams[simi_newton_joint_pospid2];
        d_param = _newtonFloatParams[simi_newton_joint_pospid3];
    }
    if (engine == sim_physics_mujoco)
    {
        p_param = _mujocoFloatParams[simi_mujoco_joint_pospid1];
        i_param = _mujocoFloatParams[simi_mujoco_joint_pospid2];
        d_param = _mujocoFloatParams[simi_mujoco_joint_pospid3];
    }
}

void CJoint::getKc(double& k_param, double& c_param) const
{
    k_param = _dynCtrl_kc[0];
    c_param = _dynCtrl_kc[1];
}

double CJoint::getLength() const
{
    return (_length);
}

double CJoint::getDiameter() const
{
    return (_diameter);
}

int CJoint::getJointType() const
{
    return (_jointType);
}

double CJoint::getScrewLead() const
{
    return (_screwLead);
}

C4Vector CJoint::getSphericalTransformation() const
{
    return (_sphericalTransf);
}

bool CJoint::getIsCyclic() const
{
    if (_jointType == sim_joint_prismatic)
        return (false);
    return (_isCyclic);
}

bool CJoint::getEnforceLimits() const
{
    return (_enforceLimits);
}

double CJoint::getIKWeight_old() const
{
    return (_ikWeight_old);
}

double CJoint::getMaxStepSize_old() const
{
    return (_maxStepSize_old);
}

int CJoint::getJointMode() const
{
    return (_jointMode);
}

int CJoint::getDependencyMasterJointHandle() const
{
    return (_dependencyMasterJointHandle);
}

void CJoint::getDependencyParams(double& off, double& mult) const
{
    off = _dependencyJointOffset;
    mult = _dependencyJointMult;
}

void CJoint::setMotorLock(bool e)
{
    bool diff = (_motorLock != e);
    if (diff)
        _motorLock = e;
}

void CJoint::setIntrinsicTransformationError(const C7Vector& tr)
{
    bool diff = (_intrinsicTransformationError != tr);
    if (diff)
    {
        _intrinsicTransformationError = tr;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propJoint_intrinsicError.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            double p[7];
            _intrinsicTransformationError.getData(p, true);
            ev->appendKeyDoubleArray(cmd, p, 7);
            getIntrinsicTransformation(true).getData(p, true);
            ev->appendKeyDoubleArray(propJoint_intrinsicPose.name, p, 7);
            App::worldContainer->pushEvent();
        }
    }
}

CColorObject* CJoint::getColor(bool part2)
{
    if (part2)
        return (&_color_removeSoon);
    return (&_color);
}

void CJoint::_fixVortexInfVals()
{ // to fix a past complication (i.e. neg. val. of unsigned would be inf)
    for (size_t i = 0; i < 47; i++)
    {
        if (_vortexFloatParams[i] < 0.0)
            _vortexFloatParams[i] = DBL_MAX;
    }
    // values at index 47 and later are signed
}

#ifdef SIM_WITH_GUI
void CJoint::display(CViewableBase* renderingObject, int displayAttrib)
{
    displayJoint(this, renderingObject, displayAttrib);
}
#endif

int CJoint::setBoolProperty(const char* ppName, bool pState, CCbor* eev /* = nullptr*/)
{
    const char* pName = nullptr;
    std::string _pName;
    if (ppName != nullptr)
    {
        _pName = utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "joint.");
        pName = _pName.c_str();
    }

    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ((eev == nullptr) && (pName != nullptr))
    { // regular properties (i.e. non-engine properties)
        retVal = CSceneObject::setBoolProperty(pName, pState);
        if (retVal == -1)
        {
            if (_pName == propJoint_cyclic.name)
            {
                setIsCyclic(pState);
                retVal = 1;
            }
            else if (_pName == propJoint_enforceLimits.name)
            {
                setEnforceLimits(pState);
                retVal = 1;
            }
        }
    }

    if (retVal == -1)
    {
        // Following only for engine properties:
        // -------------------------------------
        auto handleProp = [&](const std::string& propertyName, std::vector<int>& arr, int simiIndexBitCoded, int simiIndex) {
            if ((pName == nullptr) || (propertyName == pName))
            {
                retVal = 1;
                int nv = (arr[simiIndexBitCoded] | simiIndex) - (1 - pState) * simiIndex;
                if ((nv != arr[simiIndexBitCoded]) || (pName == nullptr))
                {
                    if (pName != nullptr)
                        arr[simiIndexBitCoded] = nv;
                    if (_isInScene && App::worldContainer->getEventsEnabled())
                    {
                        if (ev == nullptr)
                            ev = App::worldContainer->createSceneObjectChangedEvent(this, false, propertyName.c_str(), true);
                        ev->appendKeyBool(propertyName.c_str(), arr[simiIndexBitCoded] & simiIndex);
                        if (pName != nullptr)
                            _sendEngineString(ev);
                    }
                }
            }
        };

        handleProp(propJoint_vortexAxisFrictionEnabled.name, _vortexIntParams, simi_vortex_joint_bitcoded, simi_vortex_joint_motorfrictionenabled);
        handleProp(propJoint_vortexAxisFrictionProportional.name, _vortexIntParams, simi_vortex_joint_bitcoded, simi_vortex_joint_proportionalmotorfriction);

        if ((ev != nullptr) && (eev == nullptr))
            App::worldContainer->pushEvent();
        // -------------------------------------
    }

    return retVal;
}

int CJoint::getBoolProperty(const char* ppName, bool& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getBoolProperty(ppName, pState);
    if (retVal == -1)
    {
        // First non-engine properties:
        if (_pName == propJoint_cyclic.name)
        {
            pState = _isCyclic;
            retVal = 1;
        }
        else if (_pName == propJoint_enforceLimits.name)
        {
            pState = _enforceLimits;
            retVal = 1;
        }

        // Engine-only properties:
        // ------------------------
        if (_pName == propJoint_vortexAxisFrictionEnabled.name)
        {
            retVal = 1;
            pState = _vortexIntParams[simi_vortex_joint_bitcoded] & simi_vortex_joint_motorfrictionenabled;
        }
        else if (_pName == propJoint_vortexAxisFrictionProportional.name)
        {
            retVal = 1;
            pState = _vortexIntParams[simi_vortex_joint_bitcoded] & simi_vortex_joint_proportionalmotorfriction;
        }
        // ------------------------
    }
    return retVal;
}

int CJoint::setIntProperty(const char* ppName, int pState, CCbor* eev /* = nullptr*/)
{
    const char* pName = nullptr;
    std::string _pName;
    if (ppName != nullptr)
    {
        _pName = utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "joint.");
        pName = _pName.c_str();
    }

    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ((eev == nullptr) && (pName != nullptr))
    { // regular properties (i.e. non-engine properties)
        retVal = CSceneObject::setIntProperty(pName, pState);
        if (retVal == -1)
        {
            if (_pName == propJoint_jointMode.name)
            {
                setJointMode(pState);
                retVal = 1;
            }
            else if (_pName == propJoint_dynCtrlMode.name)
            {
                setDynCtrlMode(pState);
                retVal = 1;
            }
            else if (_pName == propJoint_dependencyMaster.name)
            {
                setDependencyMasterJointHandle(pState);
                retVal = 1;
            }
            else if (_pName == propJoint_dynVelMode.name)
            {
                setDynVelCtrlType(pState);
                retVal = 1;
            }
            else if (_pName == propJoint_dynPosMode.name)
            {
                setDynPosCtrlType(pState);
                retVal = 1;
            }
        }
    }

    if (retVal == -1)
    {
        // Following only for engine properties:
        // -------------------------------------
        auto handleProp = [&](const std::string& propertyName, std::vector<int>& arr, int simiIndex) {
            if ((pName == nullptr) || (propertyName == pName))
            {
                retVal = 1;
                if ((pState != arr[simiIndex]) || (pName == nullptr))
                {
                    if (pName != nullptr)
                        arr[simiIndex] = pState;
                    if (_isInScene && App::worldContainer->getEventsEnabled())
                    {
                        if (ev == nullptr)
                            ev = App::worldContainer->createSceneObjectChangedEvent(this, false, propertyName.c_str(), true);
                        ev->appendKeyInt(propertyName.c_str(), arr[simiIndex]);
                        if (pName != nullptr)
                            _sendEngineString(ev);
                    }
                }
            }
        };

        handleProp(propJoint_vortexRelaxationEnabledBits.name, _vortexIntParams, simi_vortex_joint_relaxationenabledbc);
        handleProp(propJoint_vortexFrictionEnabledBits.name, _vortexIntParams, simi_vortex_joint_frictionenabledbc);
        handleProp(propJoint_vortexFrictionProportionalBits.name, _vortexIntParams, simi_vortex_joint_frictionproportionalbc);

        if ((ev != nullptr) && (eev == nullptr))
            App::worldContainer->pushEvent();
        // -------------------------------------
    }

    return retVal;
}

int CJoint::getIntProperty(const char* ppName, int& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getIntProperty(ppName, pState);
    if (retVal == -1)
    {
        // First non-engine properties:
        if (_pName == propJoint_jointType.name)
        {
            retVal = 1;
            pState = _jointType;
        }
        else if (_pName == propJoint_jointMode.name)
        {
            retVal = 1;
            pState = _jointMode;
        }
        else if (_pName == propJoint_dynCtrlMode.name)
        {
            retVal = 1;
            pState = _dynCtrlMode;
        }
        else if (_pName == propJoint_dependencyMaster.name)
        {
            retVal = 1;
            pState = _dependencyMasterJointHandle;
        }
        else if (_pName == propJoint_dynVelMode.name)
        {
            pState = _dynVelocityCtrlType;
            retVal = 1;
        }
        else if (_pName == propJoint_dynPosMode.name)
        {
            pState = _dynPositionCtrlType;
            retVal = 1;
        }

        // Engine-only properties:
        // ------------------------
        if (_pName == propJoint_vortexRelaxationEnabledBits.name)
        {
            retVal = 1;
            pState = _vortexIntParams[simi_vortex_joint_relaxationenabledbc];
        }
        else if (_pName == propJoint_vortexFrictionEnabledBits.name)
        {
            retVal = 1;
            pState = _vortexIntParams[simi_vortex_joint_frictionenabledbc];
        }
        else if (_pName == propJoint_vortexFrictionProportionalBits.name)
        {
            retVal = 1;
            pState = _vortexIntParams[simi_vortex_joint_frictionproportionalbc];
        }
        // ------------------------
    }

    return retVal;
}

int CJoint::setFloatProperty(const char* ppName, double pState, CCbor* eev /* = nullptr*/)
{
    const char* pName = nullptr;
    std::string _pName;
    if (ppName != nullptr)
    {
        _pName = utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "joint.");
        pName = _pName.c_str();
    }

    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ((eev == nullptr) && (pName != nullptr))
    { // regular properties (i.e. non-engine properties)
        retVal = CSceneObject::setFloatProperty(pName, pState);
        if (retVal == -1)
            retVal = _color.setFloatProperty(pName, pState);
        if (retVal == -1)
        {
            if (_pName == propJoint_position.name)
            {
                setPosition(pState);
                retVal = 1;
            }
            else if (_pName == propJoint_screwLead.name)
            {
                setScrewLead(pState);
                retVal = 1;
            }
            else if (_pName == propJoint_targetPos.name)
            {
                setTargetPosition(pState);
                retVal = 1;
            }
            else if (_pName == propJoint_targetVel.name)
            {
                setTargetVelocity(pState);
                retVal = 1;
            }
            else if (_pName == propJoint_targetForce.name)
            {
                setTargetForce(pState, true);
                retVal = 1;
            }
            else if (_pName == propJoint_length.name)
            {
                setSize(pState, _diameter);
                retVal = 1;
            }
            else if (_pName == propJoint_diameter.name)
            {
                setSize(_length, pState);
                retVal = 1;
            }
        }
    }

    if (retVal == -1)
    {
        // Following only for engine properties:
        // -------------------------------------
        auto handleProp = [&](const std::string& propertyName, std::vector<double>& arr, int simiIndex) {
            if ((pName == nullptr) || (propertyName == pName))
            {
                retVal = 1;
                if ((pState != arr[simiIndex]) || (pName == nullptr))
                {
                    if (pName != nullptr)
                        arr[simiIndex] = pState;
                    if (_isInScene && App::worldContainer->getEventsEnabled())
                    {
                        if (ev == nullptr)
                            ev = App::worldContainer->createSceneObjectChangedEvent(this, false, propertyName.c_str(), true);
                        ev->appendKeyDouble(propertyName.c_str(), arr[simiIndex]);
                        if (pName != nullptr)
                            _sendEngineString(ev);
                    }
                }
            }
        };

        handleProp(propJoint_bulletStopErp.name, _bulletFloatParams, simi_bullet_joint_stoperp);
        handleProp(propJoint_bulletStopCfm.name, _bulletFloatParams, simi_bullet_joint_stopcfm);
        handleProp(propJoint_bulletNormalCfm.name, _bulletFloatParams, simi_bullet_joint_normalcfm);
        handleProp(propJoint_odeStopErp.name, _odeFloatParams, simi_ode_joint_stoperp);
        handleProp(propJoint_odeStopCfm.name, _odeFloatParams, simi_ode_joint_stopcfm);
        handleProp(propJoint_odeNormalCfm.name, _odeFloatParams, simi_ode_joint_normalcfm);
        handleProp(propJoint_odeBounce.name, _odeFloatParams, simi_ode_joint_bounce);
        handleProp(propJoint_odeFudgeFactor.name, _odeFloatParams, simi_ode_joint_fudgefactor);
        handleProp(propJoint_vortexLowerLimitDamping.name, _vortexFloatParams, simi_vortex_joint_lowerlimitdamping);
        handleProp(propJoint_vortexUpperLimitDamping.name, _vortexFloatParams, simi_vortex_joint_upperlimitdamping);
        handleProp(propJoint_vortexLowerLimitStiffness.name, _vortexFloatParams, simi_vortex_joint_lowerlimitstiffness);
        handleProp(propJoint_vortexUpperLimitStiffness.name, _vortexFloatParams, simi_vortex_joint_upperlimitstiffness);
        handleProp(propJoint_vortexLowerLimitRestitution.name, _vortexFloatParams, simi_vortex_joint_lowerlimitrestitution);
        handleProp(propJoint_vortexUpperLimitRestitution.name, _vortexFloatParams, simi_vortex_joint_upperlimitrestitution);
        handleProp(propJoint_vortexLowerLimitMaxForce.name, _vortexFloatParams, simi_vortex_joint_lowerlimitmaxforce);
        handleProp(propJoint_vortexUpperLimitMaxForce.name, _vortexFloatParams, simi_vortex_joint_upperlimitmaxforce);
        handleProp(propJoint_vortexAxisFrictionCoeff.name, _vortexFloatParams, simi_vortex_joint_motorconstraintfrictioncoeff);
        handleProp(propJoint_vortexAxisFrictionMaxForce.name, _vortexFloatParams, simi_vortex_joint_motorconstraintfrictionmaxforce);
        handleProp(propJoint_vortexAxisFrictionLoss.name, _vortexFloatParams, simi_vortex_joint_motorconstraintfrictionloss);
        handleProp(propJoint_vortexXAxisPosRelaxationStiffness.name, _vortexFloatParams, simi_vortex_joint_p0stiffness);
        handleProp(propJoint_vortexXAxisPosRelaxationDamping.name, _vortexFloatParams, simi_vortex_joint_p0damping);
        handleProp(propJoint_vortexXAxisPosRelaxationLoss.name, _vortexFloatParams, simi_vortex_joint_p0loss);
        handleProp(propJoint_vortexXAxisPosFrictionCoeff.name, _vortexFloatParams, simi_vortex_joint_p0frictioncoeff);
        handleProp(propJoint_vortexXAxisPosFrictionMaxForce.name, _vortexFloatParams, simi_vortex_joint_p0frictionmaxforce);
        handleProp(propJoint_vortexXAxisPosFrictionLoss.name, _vortexFloatParams, simi_vortex_joint_p0frictionloss);
        handleProp(propJoint_vortexYAxisPosRelaxationStiffness.name, _vortexFloatParams, simi_vortex_joint_p1stiffness);
        handleProp(propJoint_vortexYAxisPosRelaxationDamping.name, _vortexFloatParams, simi_vortex_joint_p1damping);
        handleProp(propJoint_vortexYAxisPosRelaxationLoss.name, _vortexFloatParams, simi_vortex_joint_p1loss);
        handleProp(propJoint_vortexYAxisPosFrictionCoeff.name, _vortexFloatParams, simi_vortex_joint_p1frictioncoeff);
        handleProp(propJoint_vortexYAxisPosFrictionMaxForce.name, _vortexFloatParams, simi_vortex_joint_p1frictionmaxforce);
        handleProp(propJoint_vortexYAxisPosFrictionLoss.name, _vortexFloatParams, simi_vortex_joint_p1frictionloss);
        handleProp(propJoint_vortexZAxisPosRelaxationStiffness.name, _vortexFloatParams, simi_vortex_joint_p2stiffness);
        handleProp(propJoint_vortexZAxisPosRelaxationDamping.name, _vortexFloatParams, simi_vortex_joint_p2damping);
        handleProp(propJoint_vortexZAxisPosRelaxationLoss.name, _vortexFloatParams, simi_vortex_joint_p2loss);
        handleProp(propJoint_vortexZAxisPosFrictionCoeff.name, _vortexFloatParams, simi_vortex_joint_p2frictioncoeff);
        handleProp(propJoint_vortexZAxisPosFrictionMaxForce.name, _vortexFloatParams, simi_vortex_joint_p2frictionmaxforce);
        handleProp(propJoint_vortexZAxisPosFrictionLoss.name, _vortexFloatParams, simi_vortex_joint_p2frictionloss);
        handleProp(propJoint_vortexXAxisOrientRelaxStiffness.name, _vortexFloatParams, simi_vortex_joint_a0stiffness);
        handleProp(propJoint_vortexXAxisOrientRelaxDamping.name, _vortexFloatParams, simi_vortex_joint_a0damping);
        handleProp(propJoint_vortexXAxisOrientRelaxLoss.name, _vortexFloatParams, simi_vortex_joint_a0loss);
        handleProp(propJoint_vortexXAxisOrientFrictionCoeff.name, _vortexFloatParams, simi_vortex_joint_a0frictioncoeff);
        handleProp(propJoint_vortexXAxisOrientFrictionMaxTorque.name, _vortexFloatParams, simi_vortex_joint_a0frictionmaxforce);
        handleProp(propJoint_vortexXAxisOrientFrictionLoss.name, _vortexFloatParams, simi_vortex_joint_a0frictionloss);
        handleProp(propJoint_vortexYAxisOrientRelaxStiffness.name, _vortexFloatParams, simi_vortex_joint_a1stiffness);
        handleProp(propJoint_vortexYAxisOrientRelaxDamping.name, _vortexFloatParams, simi_vortex_joint_a1damping);
        handleProp(propJoint_vortexYAxisOrientRelaxLoss.name, _vortexFloatParams, simi_vortex_joint_a1loss);
        handleProp(propJoint_vortexYAxisOrientFrictionCoeff.name, _vortexFloatParams, simi_vortex_joint_a1frictioncoeff);
        handleProp(propJoint_vortexYAxisOrientFrictionMaxTorque.name, _vortexFloatParams, simi_vortex_joint_a1frictionmaxforce);
        handleProp(propJoint_vortexYAxisOrientFrictionLoss.name, _vortexFloatParams, simi_vortex_joint_a1frictionloss);
        handleProp(propJoint_vortexZAxisOrientRelaxStiffness.name, _vortexFloatParams, simi_vortex_joint_a2stiffness);
        handleProp(propJoint_vortexZAxisOrientRelaxDamping.name, _vortexFloatParams, simi_vortex_joint_a2damping);
        handleProp(propJoint_vortexZAxisOrientRelaxLoss.name, _vortexFloatParams, simi_vortex_joint_a2loss);
        handleProp(propJoint_vortexZAxisOrientFrictionCoeff.name, _vortexFloatParams, simi_vortex_joint_a2frictioncoeff);
        handleProp(propJoint_vortexZAxisOrientFrictionMaxTorque.name, _vortexFloatParams, simi_vortex_joint_a2frictionmaxforce);
        handleProp(propJoint_vortexZAxisOrientFrictionLoss.name, _vortexFloatParams, simi_vortex_joint_a2frictionloss);
        handleProp(propJoint_mujocoArmature.name, _mujocoFloatParams, simi_mujoco_joint_armature);
        handleProp(propJoint_mujocoMargin.name, _mujocoFloatParams, simi_mujoco_joint_margin);
        handleProp(propJoint_mujocoFrictionLoss.name, _mujocoFloatParams, simi_mujoco_joint_frictionloss);
        handleProp(propJoint_mujocoSpringStiffness.name, _mujocoFloatParams, simi_mujoco_joint_stiffness);
        handleProp(propJoint_mujocoSpringDamping.name, _mujocoFloatParams, simi_mujoco_joint_damping);
        handleProp(propJoint_mujocoSpringRef.name, _mujocoFloatParams, simi_mujoco_joint_springref);

        if ((ev != nullptr) && (eev == nullptr))
            App::worldContainer->pushEvent();
        // -------------------------------------
    }

    return retVal;
}

int CJoint::getFloatProperty(const char* ppName, double& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getFloatProperty(ppName, pState);
    if (retVal == -1)
        retVal = _color.getFloatProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propJoint_position.name)
        {
            retVal = 1;
            pState = _pos;
        }
        else if (_pName == propJoint_screwLead.name)
        {
            retVal = 1;
            pState = _screwLead;
        }
        else if (_pName == propJoint_calcVelocity.name)
        {
            retVal = 1;
            pState = _velCalc_vel;
        }
        else if (_pName == propJoint_targetPos.name)
        {
            retVal = 1;
            pState = _targetPos;
        }
        else if (_pName == propJoint_targetVel.name)
        {
            retVal = 1;
            pState = _targetVel;
        }
        else if (_pName == propJoint_targetForce.name)
        {
            retVal = 1;
            pState = _targetForce;
        }
        else if (_pName == propJoint_averageJointForce.name)
        {
            retVal = 1;
            pState = _filteredForceOrTorque;
        }
        else if (_pName == propJoint_jointForce.name)
        {
            retVal = 1;
            pState = _lastForceOrTorque_dynStep;
        }
        else if (_pName == propJoint_length.name)
        {
            pState = _length;
            retVal = 1;
        }
        else if (_pName == propJoint_diameter.name)
        {
            pState = _diameter;
            retVal = 1;
        }
    }
    if (retVal == -1)
    {
        // Engine-only properties:
        // ------------------------
        if (_pName == propJoint_bulletStopErp.name)
        {
            retVal = 1;
            pState = _bulletFloatParams[simi_bullet_joint_stoperp];
        }
        else if (_pName == propJoint_bulletStopCfm.name)
        {
            retVal = 1;
            pState = _bulletFloatParams[simi_bullet_joint_stopcfm];
        }
        else if (_pName == propJoint_bulletNormalCfm.name)
        {
            retVal = 1;
            pState = _bulletFloatParams[simi_bullet_joint_normalcfm];
        }
        else if (_pName == propJoint_odeStopErp.name)
        {
            retVal = 1;
            pState = _odeFloatParams[simi_ode_joint_stoperp];
        }
        else if (_pName == propJoint_odeStopCfm.name)
        {
            retVal = 1;
            pState = _odeFloatParams[simi_ode_joint_stopcfm];
        }
        else if (_pName == propJoint_odeNormalCfm.name)
        {
            retVal = 1;
            pState = _odeFloatParams[simi_ode_joint_normalcfm];
        }
        else if (_pName == propJoint_odeBounce.name)
        {
            retVal = 1;
            pState = _odeFloatParams[simi_ode_joint_bounce];
        }
        else if (_pName == propJoint_odeFudgeFactor.name)
        {
            retVal = 1;
            pState = _odeFloatParams[simi_ode_joint_fudgefactor];
        }
        else if (_pName == propJoint_vortexLowerLimitDamping.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_lowerlimitdamping];
        }
        else if (_pName == propJoint_vortexUpperLimitDamping.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_upperlimitdamping];
        }
        else if (_pName == propJoint_vortexLowerLimitStiffness.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_lowerlimitstiffness];
        }
        else if (_pName == propJoint_vortexUpperLimitStiffness.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_upperlimitstiffness];
        }
        else if (_pName == propJoint_vortexLowerLimitRestitution.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_lowerlimitrestitution];
        }
        else if (_pName == propJoint_vortexUpperLimitRestitution.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_upperlimitrestitution];
        }
        else if (_pName == propJoint_vortexLowerLimitMaxForce.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_lowerlimitmaxforce];
        }
        else if (_pName == propJoint_vortexUpperLimitMaxForce.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_upperlimitmaxforce];
        }
        else if (_pName == propJoint_vortexAxisFrictionCoeff.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_motorconstraintfrictioncoeff];
        }
        else if (_pName == propJoint_vortexAxisFrictionMaxForce.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_motorconstraintfrictionmaxforce];
        }
        else if (_pName == propJoint_vortexAxisFrictionLoss.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_motorconstraintfrictionloss];
        }
        else if (_pName == propJoint_vortexXAxisPosRelaxationStiffness.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p0stiffness];
        }
        else if (_pName == propJoint_vortexXAxisPosRelaxationDamping.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p0damping];
        }
        else if (_pName == propJoint_vortexXAxisPosRelaxationLoss.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p0loss];
        }
        else if (_pName == propJoint_vortexXAxisPosFrictionCoeff.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p0frictioncoeff];
        }
        else if (_pName == propJoint_vortexXAxisPosFrictionMaxForce.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p0frictionmaxforce];
        }
        else if (_pName == propJoint_vortexXAxisPosFrictionLoss.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p0frictionloss];
        }
        else if (_pName == propJoint_vortexYAxisPosRelaxationStiffness.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p1stiffness];
        }
        else if (_pName == propJoint_vortexYAxisPosRelaxationDamping.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p1damping];
        }
        else if (_pName == propJoint_vortexYAxisPosRelaxationLoss.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p1loss];
        }
        else if (_pName == propJoint_vortexYAxisPosFrictionCoeff.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p1frictioncoeff];
        }
        else if (_pName == propJoint_vortexYAxisPosFrictionMaxForce.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p1frictionmaxforce];
        }
        else if (_pName == propJoint_vortexYAxisPosFrictionLoss.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p1frictionloss];
        }
        else if (_pName == propJoint_vortexZAxisPosRelaxationStiffness.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p2stiffness];
        }
        else if (_pName == propJoint_vortexZAxisPosRelaxationDamping.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p2damping];
        }
        else if (_pName == propJoint_vortexZAxisPosRelaxationLoss.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p2loss];
        }
        else if (_pName == propJoint_vortexZAxisPosFrictionCoeff.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p2frictioncoeff];
        }
        else if (_pName == propJoint_vortexZAxisPosFrictionMaxForce.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p2frictionmaxforce];
        }
        else if (_pName == propJoint_vortexZAxisPosFrictionLoss.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_p2frictionloss];
        }
        else if (_pName == propJoint_vortexXAxisOrientRelaxStiffness.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a0stiffness];
        }
        else if (_pName == propJoint_vortexXAxisOrientRelaxDamping.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a0damping];
        }
        else if (_pName == propJoint_vortexXAxisOrientRelaxLoss.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a0loss];
        }
        else if (_pName == propJoint_vortexXAxisOrientFrictionCoeff.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a0frictioncoeff];
        }
        else if (_pName == propJoint_vortexXAxisOrientFrictionMaxTorque.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a0frictionmaxforce];
        }
        else if (_pName == propJoint_vortexXAxisOrientFrictionLoss.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a0frictionloss];
        }
        else if (_pName == propJoint_vortexYAxisOrientRelaxStiffness.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a1stiffness];
        }
        else if (_pName == propJoint_vortexYAxisOrientRelaxDamping.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a1damping];
        }
        else if (_pName == propJoint_vortexYAxisOrientRelaxLoss.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a1loss];
        }
        else if (_pName == propJoint_vortexYAxisOrientFrictionCoeff.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a1frictioncoeff];
        }
        else if (_pName == propJoint_vortexYAxisOrientFrictionMaxTorque.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a1frictionmaxforce];
        }
        else if (_pName == propJoint_vortexYAxisOrientFrictionLoss.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a1frictionloss];
        }
        else if (_pName == propJoint_vortexZAxisOrientRelaxStiffness.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a2stiffness];
        }
        else if (_pName == propJoint_vortexZAxisOrientRelaxDamping.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a2damping];
        }
        else if (_pName == propJoint_vortexZAxisOrientRelaxLoss.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a2loss];
        }
        else if (_pName == propJoint_vortexZAxisOrientFrictionCoeff.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a2frictioncoeff];
        }
        else if (_pName == propJoint_vortexZAxisOrientFrictionMaxTorque.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a2frictionmaxforce];
        }
        else if (_pName == propJoint_vortexZAxisOrientFrictionLoss.name)
        {
            retVal = 1;
            pState = _vortexFloatParams[simi_vortex_joint_a2frictionloss];
        }
        else if (_pName == propJoint_mujocoArmature.name)
        {
            retVal = 1;
            pState = _mujocoFloatParams[simi_mujoco_joint_armature];
        }
        else if (_pName == propJoint_mujocoMargin.name)
        {
            retVal = 1;
            pState = _mujocoFloatParams[simi_mujoco_joint_margin];
        }
        else if (_pName == propJoint_mujocoFrictionLoss.name)
        {
            retVal = 1;
            pState = _mujocoFloatParams[simi_mujoco_joint_frictionloss];
        }
        else if (_pName == propJoint_mujocoSpringStiffness.name)
        {
            retVal = 1;
            pState = _mujocoFloatParams[simi_mujoco_joint_stiffness];
        }
        else if (_pName == propJoint_mujocoSpringDamping.name)
        {
            retVal = 1;
            pState = _mujocoFloatParams[simi_mujoco_joint_damping];
        }
        else if (_pName == propJoint_mujocoSpringRef.name)
        {
            retVal = 1;
            pState = _mujocoFloatParams[simi_mujoco_joint_springref];
        }
        // ------------------------
    }

    return retVal;
}

int CJoint::setIntArray2Property(const char* ppName, const int* pState, CCbor* eev /* = nullptr*/)
{
    const char* pName = nullptr;
    std::string _pName;
    if (ppName != nullptr)
    {
        _pName = utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "joint.");
        pName = _pName.c_str();
    }

    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ((eev == nullptr) && (pName != nullptr))
    { // regular properties (i.e. non-engine properties)
        retVal = CSceneObject::setIntArray2Property(pName, pState);
        if (retVal == -1)
        {
        }
    }

    if (retVal == -1)
    {
        // Following only for engine properties:
        // -------------------------------------
        auto handleProp = [&](const std::string& propertyName, std::vector<int>& arr, int simiIndex1) {
            if ((pName == nullptr) || (propertyName == pName))
            {
                retVal = 1;
                bool pa = false;
                if (pState != nullptr)
                {
                    for (size_t i = 0; i < 2; i++)
                        pa = pa || (arr[simiIndex1 + i] != pState[i]);
                }
                if ((pName == nullptr) || pa)
                {
                    if (pName != nullptr)
                    {
                        for (size_t i = 0; i < 2; i++)
                            arr[simiIndex1 + i] = pState[i];
                    }
                    if (_isInScene && App::worldContainer->getEventsEnabled())
                    {
                        if (ev == nullptr)
                            ev = App::worldContainer->createSceneObjectChangedEvent(this, false, propertyName.c_str(), true);
                        ev->appendKeyIntArray(propertyName.c_str(), arr.data() + simiIndex1, 2);
                        if (pName != nullptr)
                            _sendEngineString(ev);
                    }
                }
            }
        };

        // handleProp(propJoint_bulletPosPid.name, _bulletIntParams, simi_bullet_joint_pospid1);

        if ((ev != nullptr) && (eev == nullptr))
            App::worldContainer->pushEvent();
        // -------------------------------------
    }

    return retVal;
}

int CJoint::getIntArray2Property(const char* ppName, int* pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getIntArray2Property(ppName, pState);
    if (retVal == -1)
    { // First non-engine properties:

        // Engine-only properties:
        // ------------------------
        auto handleProp = [&](const std::vector<int>& arr, int simiIndex1) {
            retVal = 1;
            for (size_t i = 0; i < 2; i++)
                pState[i] = arr[simiIndex1 + i];
        };

        //if (_pName == propJoint_bulletPosPid.name)
        //    handleProp(_bulletIntParams, simi_bullet_joint_pospid1);
        // ------------------------
    }

    return retVal;
}

int CJoint::setVector2Property(const char* ppName, const double* pState, CCbor* eev /* = nullptr*/)
{
    const char* pName = nullptr;
    std::string _pName;
    if (ppName != nullptr)
    {
        _pName = utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "joint.");
        pName = _pName.c_str();
    }

    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ((eev == nullptr) && (pName != nullptr))
    { // regular properties (i.e. non-engine properties)
        retVal = CSceneObject::setVector2Property(pName, pState);
        if (retVal == -1)
        {
        }
    }

    if (retVal == -1)
    {
        // Following only for engine properties:
        // -------------------------------------
        auto handleProp = [&](const std::string& propertyName, std::vector<double>& arr, int simiIndex1) {
            if ((pName == nullptr) || (propertyName == pName))
            {
                retVal = 1;
                bool pa = false;
                if (pState != nullptr)
                {
                    for (size_t i = 0; i < 2; i++)
                        pa = pa || (arr[simiIndex1 + i] != pState[i]);
                }
                if ((pName == nullptr) || pa)
                {
                    if (pName != nullptr)
                    {
                        for (size_t i = 0; i < 2; i++)
                            arr[simiIndex1 + i] = pState[i];
                    }
                    if (_isInScene && App::worldContainer->getEventsEnabled())
                    {
                        if (ev == nullptr)
                            ev = App::worldContainer->createSceneObjectChangedEvent(this, false, propertyName.c_str(), true);
                        ev->appendKeyDoubleArray(propertyName.c_str(), arr.data() + simiIndex1, 2);
                        if (pName != nullptr)
                            _sendEngineString(ev);
                    }
                }
            }
        };

        if ((ev != nullptr) && (eev == nullptr))
            App::worldContainer->pushEvent();
        // -------------------------------------
    }

    return retVal;
}

int CJoint::getVector2Property(const char* ppName, double* pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getVector2Property(ppName, pState);
    if (retVal == -1)
    { // First non-engine properties:

        // Engine-only properties:
        // ------------------------
        auto handleProp = [&](const std::vector<double>& arr, int simiIndex1) {
            retVal = 1;
            for (size_t i = 0; i < 2; i++)
                pState[i] = arr[simiIndex1 + i];
        };

        // ------------------------
    }

    return retVal;
}

int CJoint::setVector3Property(const char* ppName, const C3Vector& pState, CCbor* eev /* = nullptr*/)
{
    const char* pName = nullptr;
    std::string _pName;
    if (ppName != nullptr)
    {
        _pName = utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "joint.");
        pName = _pName.c_str();
    }

    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ((eev == nullptr) && (pName != nullptr))
    { // regular properties (i.e. non-engine properties)
        retVal = CSceneObject::setVector3Property(pName, pState);
        if (retVal == -1)
        {
        }
    }

    if (retVal == -1)
    {
        // Following only for engine properties:
        // -------------------------------------
        auto handleProp = [&](const std::string& propertyName, std::vector<double>& arr, int simiIndex1) {
            if ((pName == nullptr) || (propertyName == pName))
            {
                retVal = 1;
                bool pa = false;
                for (size_t i = 0; i < 3; i++)
                    pa = pa || (arr[simiIndex1 + i] != pState(i));
                if ((pName == nullptr) || pa)
                {
                    if (pName != nullptr)
                    {
                        for (size_t i = 0; i < 3; i++)
                            arr[simiIndex1 + i] = pState(i);
                    }
                    if (_isInScene && App::worldContainer->getEventsEnabled())
                    {
                        if (ev == nullptr)
                            ev = App::worldContainer->createSceneObjectChangedEvent(this, false, propertyName.c_str(), true);
                        ev->appendKeyDoubleArray(propertyName.c_str(), arr.data() + simiIndex1, 3);
                        if (pName != nullptr)
                            _sendEngineString(ev);
                    }
                }
            }
        };

        if ((ev != nullptr) && (eev == nullptr))
            App::worldContainer->pushEvent();
        // -------------------------------------
    }

    return retVal;
}

int CJoint::getVector3Property(const char* ppName, C3Vector& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getVector3Property(ppName, pState);
    if (retVal == -1)
    { // First non-engine properties:

        // Engine-only properties:
        // ------------------------
        auto handleProp = [&](const std::vector<double>& arr, int simiIndex1) {
            retVal = 1;
            pState.setData(arr.data() + simiIndex1);
        };

        // ------------------------
    }

    return retVal;
}

int CJoint::setStringProperty(const char* ppName, const char* pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setStringProperty(ppName, pState);
    if (retVal == -1)
    {
        if (strcmp(ppName, propJoint_engineProperties.name) == 0)
        {
            retVal = 0;
            CEngineProperties prop;
            std::string current(prop.getObjectProperties(_objectHandle));
            if (prop.setObjectProperties(_objectHandle, pState))
            {
                retVal = 1;
                std::string current2(prop.getObjectProperties(_objectHandle));
                if (current != current2)
                    _sendEngineString();
            }
        }
    }
    return retVal;
}

int CJoint::getStringProperty(const char* ppName, std::string& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getStringProperty(ppName, pState);
    if (retVal == -1)
    {
        if (strcmp(ppName, propJoint_engineProperties.name) == 0)
        {
            retVal = 1;
            CEngineProperties prop;
            pState = prop.getObjectProperties(_objectHandle);
        }
        else if (strcmp(ppName, propJoint_objectMetaInfo.name) == 0)
        {
            retVal = 1;
            pState = OBJECT_META_INFO;
        }
    }

    return retVal;
}

int CJoint::setQuaternionProperty(const char* ppName, const C4Vector& pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setQuaternionProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propJoint_quaternion.name)
        {
            retVal = 1;
            setSphericalTransformation(pState);
        }
    }
    return retVal;
}

int CJoint::getQuaternionProperty(const char* ppName, C4Vector& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getQuaternionProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propJoint_quaternion.name)
        {
            retVal = 1;
            pState = _sphericalTransf;
        }
    }

    return retVal;
}

int CJoint::getPoseProperty(const char* ppName, C7Vector& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getPoseProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propJoint_intrinsicError.name)
        {
            retVal = 1;
            pState = _intrinsicTransformationError;
        }
        else if (_pName == propJoint_intrinsicPose.name)
        {
            retVal = 1;
            pState = getIntrinsicTransformation(true);
        }
    }

    return retVal;
}

int CJoint::setColorProperty(const char* ppName, const float* pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setColorProperty(ppName, pState);
    if (retVal == -1)
        retVal = _color.setColorProperty(ppName, pState);
    if (retVal != -1)
    {
    }
    return retVal;
}

int CJoint::getColorProperty(const char* ppName, float* pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getColorProperty(ppName, pState);
    if (retVal == -1)
        retVal = _color.getColorProperty(ppName, pState);
    if (retVal != -1)
    {
    }
    return retVal;
}

int CJoint::setFloatArrayProperty(const char* ppName, const double* v, int vL, CCbor* eev /* = nullptr*/)
{
    const char* pName = nullptr;
    std::string _pName;
    if (ppName != nullptr)
    {
        _pName = utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "joint.");
        pName = _pName.c_str();
    }

    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ((eev == nullptr) && (pName != nullptr))
    { // regular properties (i.e. non-engine properties)
        retVal = CSceneObject::setFloatArrayProperty(pName, v, vL);
        if (retVal == -1)
        {
            if (_pName == propJoint_maxVelAccelJerk.name)
            {
                if (vL >= 2)
                {
                    setMaxVelAccelJerk(v);
                    retVal = 1;
                }
                else
                    retVal = 0;
            }
            else if (_pName == propJoint_interval.name)
            {
                if (vL >= 2)
                {
                    setInterval(v[0], v[1]);
                    retVal = 1;
                }
                else
                    retVal = 0;
            }
            else if (_pName == propJoint_dependencyParams.name)
            {
                if (vL >= 2)
                {
                    setDependencyParams(v[0], v[1]);
                    retVal = 1;
                }
                else
                    retVal = 0;
            }
            else if (_pName == propJoint_springDamperParams.name)
            {
                if (vL >= 2)
                {
                    setKc(v[0], v[1]);
                    retVal = 1;
                }
                else
                    retVal = 0;
            }
        }
    }

    if (retVal == -1)
    {
        // Following only for engine properties:
        // -------------------------------------
        auto handleProp = [&](const std::string& propertyName, std::vector<double>& arr, int simiIndex1, size_t n) {
            if ((pName == nullptr) || (propertyName == pName))
            {
                retVal = 1;
                bool pa = false;
                for (size_t i = 0; i < n; i++)
                    pa = pa || ((vL > i) && (arr[simiIndex1 + i] != v[i]));
                if ((pName == nullptr) || pa)
                {
                    if (pName != nullptr)
                    {
                        for (size_t i = 0; i < n; i++)
                        {
                            if (vL > i)
                                arr[simiIndex1 + i] = v[i];
                        }
                    }
                    if (_isInScene && App::worldContainer->getEventsEnabled())
                    {
                        if (ev == nullptr)
                            ev = App::worldContainer->createSceneObjectChangedEvent(this, false, propertyName.c_str(), true);
                        ev->appendKeyDoubleArray(propertyName.c_str(), arr.data() + simiIndex1, n);
                        if (pName != nullptr)
                            _sendEngineString(ev);
                    }
                }
            }
        };

        handleProp(propJoint_mujocoLimitsSolRef.name, _mujocoFloatParams, simi_mujoco_joint_solreflimit1, 2);
        handleProp(propJoint_mujocoFrictionSolRef.name, _mujocoFloatParams, simi_mujoco_joint_solreffriction1, 2);
        handleProp(propJoint_mujocoSpringDamper.name, _mujocoFloatParams, simi_mujoco_joint_springdamper1, 2);
        handleProp(propJoint_mujocoLimitsSolImp.name, _mujocoFloatParams, simi_mujoco_joint_solimplimit1, 5);
        handleProp(propJoint_mujocoFrictionSolImp.name, _mujocoFloatParams, simi_mujoco_joint_solimpfriction1, 5);
        handleProp(propJoint_mujocoDependencyPolyCoef.name, _mujocoFloatParams, simi_mujoco_joint_polycoef1, 5);
        handleProp(propJoint_bulletPosPid.name, _bulletFloatParams, simi_bullet_joint_pospid1, 3);
        handleProp(propJoint_odePosPid.name, _odeFloatParams, simi_ode_joint_pospid1, 3);
        handleProp(propJoint_vortexPosPid.name, _vortexFloatParams, simi_vortex_joint_pospid1, 3);
        handleProp(propJoint_newtonPosPid.name, _newtonFloatParams, simi_newton_joint_pospid1, 3);
        handleProp(propJoint_mujocoPosPid.name, _mujocoFloatParams, simi_mujoco_joint_pospid1, 3);

        if ((ev != nullptr) && (eev == nullptr))
            App::worldContainer->pushEvent();
        // -------------------------------------
    }

    return retVal;
}

int CJoint::getFloatArrayProperty(const char* ppName, std::vector<double>& pState) const
{
    std::string _pName(ppName);
    pState.clear();
    int retVal = CSceneObject::getFloatArrayProperty(ppName, pState);
    if (retVal == -1)
    { // First non-engine properties:
        if (_pName == propJoint_maxVelAccelJerk.name)
        {
            pState.push_back(_maxVelAccelJerk[0]);
            pState.push_back(_maxVelAccelJerk[1]);
            pState.push_back(_maxVelAccelJerk[2]);
            retVal = 1;
        }
        else if (_pName == propJoint_interval.name)
        {
            double minV, maxV;
            getInterval(minV, maxV);
            pState.push_back(minV);
            pState.push_back(maxV);
            retVal = 1;
        }
        else if (_pName == propJoint_dependencyParams.name)
        {
            pState.push_back(_dependencyJointOffset);
            pState.push_back(_dependencyJointMult);
            retVal = 1;
        }
        else if (_pName == propJoint_springDamperParams.name)
        {
            pState.push_back(_dynCtrl_kc[0]);
            pState.push_back(_dynCtrl_kc[1]);
            retVal = 1;
        }

        // Engine-only properties:
        // ------------------------
        auto handleProp = [&](const std::vector<double>& arr, int simiIndex1, size_t n) {
            retVal = 1;
            for (size_t i = 0; i < n; i++)
                pState.push_back(arr[simiIndex1 + i]);
        };

        if (_pName == propJoint_mujocoLimitsSolRef.name)
            handleProp(_mujocoFloatParams, simi_mujoco_joint_solreflimit1, 2);
        else if (_pName == propJoint_mujocoFrictionSolRef.name)
            handleProp(_mujocoFloatParams, simi_mujoco_joint_solreffriction1, 2);
        else if (_pName == propJoint_mujocoSpringDamper.name)
            handleProp(_mujocoFloatParams, simi_mujoco_joint_springdamper1, 2);
        else if (_pName == propJoint_mujocoLimitsSolImp.name)
            handleProp(_mujocoFloatParams, simi_mujoco_joint_solimplimit1, 5);
        else if (_pName == propJoint_mujocoFrictionSolImp.name)
            handleProp(_mujocoFloatParams, simi_mujoco_joint_solimpfriction1, 5);
        else if (_pName == propJoint_mujocoDependencyPolyCoef.name)
            handleProp(_mujocoFloatParams, simi_mujoco_joint_polycoef1, 5);
        else if (_pName == propJoint_bulletPosPid.name)
            handleProp(_bulletFloatParams, simi_bullet_joint_pospid1, 3);
        else if (_pName == propJoint_odePosPid.name)
            handleProp(_odeFloatParams, simi_ode_joint_pospid1, 3);
        else if (_pName == propJoint_vortexPosPid.name)
            handleProp(_vortexFloatParams, simi_vortex_joint_pospid1, 3);
        else if (_pName == propJoint_newtonPosPid.name)
            handleProp(_newtonFloatParams, simi_newton_joint_pospid1, 3);
        else if (_pName == propJoint_mujocoPosPid.name)
            handleProp(_mujocoFloatParams, simi_mujoco_joint_pospid1, 3);
        // ------------------------
    }

    return retVal;
}

int CJoint::getPropertyName(int& index, std::string& pName, std::string& appartenance) const
{
    int retVal = CSceneObject::getPropertyName(index, pName, appartenance);
    if (retVal == -1)
    {
        appartenance += ".joint";
        retVal = _color.getPropertyName(index, pName);
    }
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_joint.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_joint[i].name, pName.c_str()))
            {
                if ((allProps_joint[i].flags & sim_propertyinfo_deprecated) == 0)
                {
                    index--;
                    if (index == -1)
                    {
                        pName = allProps_joint[i].name;
                        retVal = 1;
                        break;
                    }
                }
            }
        }
    }
    return retVal;
}

int CJoint::getPropertyName_static(int& index, std::string& pName, std::string& appartenance)
{
    int retVal = CSceneObject::getPropertyName_bstatic(index, pName, appartenance);
    if (retVal == -1)
    {
        appartenance += ".joint";
        retVal = CColorObject::getPropertyName_static(index, pName, 1 + 4 + 8, "");
    }
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_joint.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_joint[i].name, pName.c_str()))
            {
                if ((allProps_joint[i].flags & sim_propertyinfo_deprecated) == 0)
                {
                    index--;
                    if (index == -1)
                    {
                        pName = allProps_joint[i].name;
                        retVal = 1;
                        break;
                    }
                }
            }
        }
    }
    return retVal;
}

int CJoint::getPropertyInfo(const char* ppName, int& info, std::string& infoTxt) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getPropertyInfo(ppName, info, infoTxt);
    if (retVal == -1)
        retVal = _color.getPropertyInfo(ppName, info, infoTxt);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_joint.size(); i++)
        {
            if (strcmp(allProps_joint[i].name, ppName) == 0)
            {
                retVal = allProps_joint[i].type;
                info = allProps_joint[i].flags;
                if ((infoTxt == "") && (strcmp(allProps_joint[i].infoTxt, "") != 0))
                    infoTxt = allProps_joint[i].infoTxt;
                else
                    infoTxt = allProps_joint[i].shortInfoTxt;
                break;
            }
        }
    }
    return retVal;
}

int CJoint::getPropertyInfo_static(const char* ppName, int& info, std::string& infoTxt)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getPropertyInfo_bstatic(ppName, info, infoTxt);
    if (retVal == -1)
        retVal = CColorObject::getPropertyInfo_static(ppName, info, infoTxt, 1 + 4 + 8, "");
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_joint.size(); i++)
        {
            if (strcmp(allProps_joint[i].name, ppName) == 0)
            {
                retVal = allProps_joint[i].type;
                info = allProps_joint[i].flags;
                if ((infoTxt == "") && (strcmp(allProps_joint[i].infoTxt, "") != 0))
                    infoTxt = allProps_joint[i].infoTxt;
                else
                    infoTxt = allProps_joint[i].shortInfoTxt;
                break;
            }
        }
    }
    return retVal;
}

void CJoint::_sendEngineString(CCbor* eev /*= nullptr*/)
{
    if (_isInScene && App::worldContainer->getEventsEnabled())
    {
        CCbor* ev = nullptr;
        if (eev != nullptr)
            ev = eev;
        CEngineProperties prop;
        std::string current(prop.getObjectProperties(_objectHandle));
        if (ev == nullptr)
            ev = App::worldContainer->createSceneObjectChangedEvent(this, false, propJoint_engineProperties.name, true);
        ev->appendKeyText(propJoint_engineProperties.name, current.c_str());
        if ((ev != nullptr) && (eev == nullptr))
            App::worldContainer->pushEvent();
    }
}

std::string CJoint::_enumToProperty(int oldEnum, int type, int& indexWithArrays) const
{
    std::string retVal;
    for (size_t i = 0; i < allProps_joint.size(); i++)
    {
        for (size_t j = 0; j < 5; j++)
        {
            int en = allProps_joint[i].oldEnums[j];
            if (en == -1)
                break;
            else if (en == oldEnum)
            {
                if (type == allProps_joint[i].type)
                {
                    if ((j > 0) || (allProps_joint[i].oldEnums[j + 1] != -1))
                        indexWithArrays = int(j);
                    else
                        indexWithArrays = -1;
                    retVal = allProps_joint[i].name;
                }
                break;
            }
        }
        if (retVal.size() > 0)
            break;
    }
    return retVal;
}

// Some helpers:
bool CJoint::getBoolPropertyValue(const char* pName) const
{
    bool retVal = false;
    getBoolProperty(pName, retVal);
    return retVal;
}

int CJoint::getIntPropertyValue(const char* pName) const
{
    int retVal = 0;
    getIntProperty(pName, retVal);
    return retVal;
}

double CJoint::getFloatPropertyValue(const char* pName) const
{
    double retVal = 0.0;
    getFloatProperty(pName, retVal);
    return retVal;
}
