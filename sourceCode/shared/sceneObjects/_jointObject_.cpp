#include "_jointObject_.h"
#include "simConst.h"
#include "app.h"

_CJoint_::_CJoint_()
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
}


_CJoint_::~_CJoint_()
{

}

void _CJoint_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSceneObject_
    if (routing.size()>0)
    {
        if (routing[0].objType==sim_syncobj_color)
        { // this message is for the color objects
            CColorObject* obj=&_colorPart1;
            if (routing[0].objHandle==1)
                obj=&_colorPart2;
            routing.erase(routing.begin());
            obj->synchronizationMsg(routing,msg);
        }
    }
    else
    {
        if (msg.msg>=sim_syncobj_sceneobject_cmdstart)
        { // message is for this sceneObject
            _CSceneObject_::synchronizationMsg(routing,msg);
        }
        else
        { // message is for this joint
            if (msg.msg==sim_syncobj_joint_intervalmin)
            {
                setPositionIntervalMin(((float*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_intervalrange)
            {
                setPositionIntervalRange(((float*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_diameter)
            {
                setDiameter(((float*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_length)
            {
                setLength(((float*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_pitch)
            {
                setScrewPitch(((float*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_dependencymult)
            {
                setDependencyJointMult(((float*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_dependencyoff)
            {
                setDependencyJointOffset(((float*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_stepsize)
            {
                setMaxStepSize(((float*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_ikweight)
            {
                setIkWeight(((float*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_position)
            {
                setPosition(((float*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_targetvelocity)
            {
                setDynamicMotorTargetVelocity(((float*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_uppervelocitylimit)
            {
                setDynamicMotorUpperLimitVelocity(((float*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_targetposition)
            {
                setDynamicMotorPositionControlTargetPosition(((float*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_force)
            {
                setDynamicMotorMaximumForce(((float*)msg.data)[0]);
                return;
            }

            if (msg.msg==sim_syncobj_joint_dependencyhandle)
            {
                setDependencyMasterJointHandle(((int*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_mode)
            {
                setJointMode(((int*)msg.data)[0]);
                return;
            }

            if (msg.msg==sim_syncobj_joint_intervalcyclic)
            {
                setPositionIsCyclic(((bool*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_enablemotor)
            {
                setEnableDynamicMotor(((bool*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_hybridfunc)
            {
                setHybridFunctionality(((bool*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_controlenabled)
            {
                setEnableDynamicMotorControlLoop(((bool*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_torquemodulation)
            {
                setEnableTorqueModulation(((bool*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_motorlock)
            {
                setDynamicMotorLockModeWhenInVelocityControl(((bool*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_secondpartvalid)
            {
                setDynamicSecondPartIsValid(((bool*)msg.data)[0]);
                return;
            }


            if (msg.msg==sim_syncobj_joint_pid)
            {
                setDynamicMotorPositionControlParameters(((float*)msg.data)[0],((float*)msg.data)[1],((float*)msg.data)[2]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_kc)
            {
                setDynamicMotorSpringControlParameters(((float*)msg.data)[0],((float*)msg.data)[1]);
                return;
            }
            if (msg.msg==sim_syncobj_joint_sphericaltransf)
            {
                C4Vector tr;
                tr.setInternalData((float*)msg.data);
                setSphericalTransformation(tr);
                return;
            }
            if (msg.msg==sim_syncobj_joint_secondparttransf)
            {
                C7Vector tr;
                tr.setInternalData((float*)msg.data);
                setDynamicSecondPartLocalTransform(tr);
                return;
            }
            if (msg.msg==sim_syncobj_joint_bulletfloats)
            {
                std::vector<float> p((float*)msg.data,(float*)msg.data+msg.dataSize);
                setBulletFloatParams(p);
                return;
            }
            if (msg.msg==sim_syncobj_joint_odefloats)
            {
                std::vector<float> p((float*)msg.data,(float*)msg.data+msg.dataSize);
                setOdeFloatParams(p);
                return;
            }
            if (msg.msg==sim_syncobj_joint_vortexfloats)
            {
                std::vector<float> p((float*)msg.data,(float*)msg.data+msg.dataSize);
                setVortexFloatParams(p);
                return;
            }
            if (msg.msg==sim_syncobj_joint_newtonfloats)
            {
                std::vector<float> p((float*)msg.data,(float*)msg.data+msg.dataSize);
                setNewtonFloatParams(p);
                return;
            }
            if (msg.msg==sim_syncobj_joint_bulletints)
            {
                std::vector<int> p((int*)msg.data,(int*)msg.data+msg.dataSize);
                setBulletIntParams(p);
                return;
            }
            if (msg.msg==sim_syncobj_joint_odeints)
            {
                std::vector<int> p((int*)msg.data,(int*)msg.data+msg.dataSize);
                setOdeIntParams(p);
                return;
            }
            if (msg.msg==sim_syncobj_joint_vortexints)
            {
                std::vector<int> p((int*)msg.data,(int*)msg.data+msg.dataSize);
                setVortexIntParams(p);
                return;
            }
            if (msg.msg==sim_syncobj_joint_newtonints)
            {
                std::vector<int> p((int*)msg.data,(int*)msg.data+msg.dataSize);
                setNewtonIntParams(p);
                return;
            }
        }
    }
}

bool _CJoint_::setPositionIntervalMin(float min)
{
    bool diff=(_jointMinPosition!=min);
    if (diff)
    {
        if (getObjectCanChange())
            _jointMinPosition=min;
        if (getObjectCanSync())
            _setPositionIntervalMin_send(min);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setPositionIntervalMin_send(float min) const
{
    sendFloat(min,sim_syncobj_joint_intervalmin);
}

bool _CJoint_::setPositionIntervalRange(float range)
{
    bool diff=(_jointPositionRange!=range);
    if (diff)
    {
        if (getObjectCanChange())
            _jointPositionRange=range;
        if (getObjectCanSync())
            _setPositionIntervalRange_send(range);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setPositionIntervalRange_send(float range) const
{
    sendFloat(range,sim_syncobj_joint_intervalrange);
}

bool _CJoint_::setScrewPitch(float pitch)
{
    bool diff=(_screwPitch!=pitch);
    if (diff)
    {
        if (getObjectCanChange())
            _screwPitch=pitch;
        if (getObjectCanSync())
            _setScrewPitch_send(pitch);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setScrewPitch_send(float pitch) const
{
    sendFloat(pitch,sim_syncobj_joint_pitch);
}

bool _CJoint_::setPositionIsCyclic(bool isCyclic)
{
    bool diff=(_positionIsCyclic!=isCyclic);
    if (diff)
    {
        if (getObjectCanChange())
            _positionIsCyclic=isCyclic;
        if (getObjectCanSync())
            _setPositionIsCyclic_send(isCyclic);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setPositionIsCyclic_send(bool isCyclic) const
{
    sendBool(isCyclic,sim_syncobj_joint_intervalcyclic);
}

bool _CJoint_::setDiameter(float d)
{
    bool diff=(_diameter!=d);
    if (diff)
    {
        if (getObjectCanChange())
            _diameter=d;
        if (getObjectCanSync())
            _setDiameter_send(d);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setDiameter_send(float d) const
{
    sendFloat(d,sim_syncobj_joint_diameter);
}

bool _CJoint_::setLength(float l)
{
    bool diff=(_length!=l);
    if (diff)
    {
        if (getObjectCanChange())
            _length=l;
        if (getObjectCanSync())
            _setLength_send(l);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setLength_send(float l) const
{
    sendFloat(l,sim_syncobj_joint_length);
}

bool _CJoint_::setDependencyMasterJointHandle(int depJointID)
{
    bool diff=(_dependencyMasterJointHandle!=depJointID);
    if (diff)
    {
        if (getObjectCanChange())
            _dependencyMasterJointHandle=depJointID;
        if (getObjectCanSync())
            _setDependencyJointHandle_send(depJointID);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setDependencyJointHandle_send(int depJointID) const
{
    sendInt32(depJointID,sim_syncobj_joint_dependencyhandle);
}

bool _CJoint_::setDependencyJointMult(float coeff)
{
    bool diff=(_dependencyJointMult!=coeff);
    if (diff)
    {
        if (getObjectCanChange())
            _dependencyJointMult=coeff;
        if (getObjectCanSync())
            _setDependencyJointMult_send(coeff);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setDependencyJointMult_send(float coeff) const
{
    sendFloat(coeff,sim_syncobj_joint_dependencymult);
}

bool _CJoint_::setDependencyJointOffset(float off)
{
    bool diff=(_dependencyJointOffset!=off);
    if (diff)
    {
        if (getObjectCanChange())
            _dependencyJointOffset=off;
        if (getObjectCanSync())
            _setDependencyJointOffset_send(off);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setDependencyJointOffset_send(float off) const
{
    sendFloat(off,sim_syncobj_joint_dependencyoff);
}

bool _CJoint_::setPosition(float pos)
{
    bool diff=(_jointPosition!=pos);
    if (diff)
    {
        if (_isInScene)
        {
            const char* cmd="jointPosition";
            CInterfaceStackTable* event=App::worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED,cmd,this);
            event->appendMapObject_stringFloat(cmd,pos);
            App::worldContainer->pushEvent();
        }
        if (getObjectCanChange())
            _jointPosition=pos;
        if (getObjectCanSync())
            _setPosition_send(pos);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setPosition_send(float pos) const
{
    sendFloat(pos,sim_syncobj_joint_position);
}

bool _CJoint_::setSphericalTransformation(const C4Vector& tr)
{
    bool diff=(_sphericalTransformation!=tr);
    if (diff)
    {
        if (_isInScene)
        {
            const char* cmd="jointQuaternion";
            CInterfaceStackTable* event=App::worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED,cmd,this);
            float p[4]={tr(1),tr(2),tr(3),tr(0)};
            event->appendMapObject_stringFloatArray(cmd,p,4);
            App::worldContainer->pushEvent();
        }
        if (getObjectCanChange())
            _sphericalTransformation=tr;
        if (getObjectCanSync())
            _setSphericalTransformation_send(tr);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setSphericalTransformation_send(const C4Vector& tr) const
{
    sendQuaternion(&tr,sim_syncobj_joint_sphericaltransf);
}

bool _CJoint_::setIkWeight(float newWeight)
{
    bool diff=(_ikWeight!=newWeight);
    if (diff)
    {
        if (getObjectCanChange())
            _ikWeight=newWeight;
        if (getObjectCanSync())
            _setIkWeight_send(newWeight);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setIkWeight_send(float newWeight) const
{
    sendFloat(newWeight,sim_syncobj_joint_ikweight);
}

bool _CJoint_::setMaxStepSize(float stepS)
{
    bool diff=(_maxStepSize!=stepS);
    if (diff)
    {
        if (getObjectCanChange())
            _maxStepSize=stepS;
        if (getObjectCanSync())
            _setMaxStepSize_send(stepS);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setMaxStepSize_send(float stepS) const
{
    sendFloat(stepS,sim_syncobj_joint_stepsize);
}

bool _CJoint_::setJointMode(int theMode)
{
    bool diff=(_jointMode!=theMode);
    if (diff)
    {
        if (getObjectCanChange())
            _jointMode=theMode;
        if (getObjectCanSync())
            _setJointMode_send(theMode);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setJointMode_send(int theMode) const
{
    sendInt32(theMode,sim_syncobj_joint_mode);
}

bool _CJoint_::setEnableDynamicMotor(bool e)
{
    bool diff=(_dynamicMotorEnabled!=e);
    if (diff)
    {
        if (getObjectCanChange())
            _dynamicMotorEnabled=e;
        if (getObjectCanSync())
            _setEnableDynamicMotor_send(e);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setEnableDynamicMotor_send(bool e) const
{
    sendBool(e,sim_syncobj_joint_enablemotor);
}

bool _CJoint_::setHybridFunctionality(bool h)
{
    bool diff=(_jointHasHybridFunctionality!=h);
    if (diff)
    {
        if (getObjectCanChange())
            _jointHasHybridFunctionality=h;
        if (getObjectCanSync())
            _setHybridFunctionality_send(h);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setHybridFunctionality_send(bool e) const
{
    sendBool(e,sim_syncobj_joint_hybridfunc);
}

bool _CJoint_::setDynamicMotorPositionControlTargetPosition(float p)
{
    bool diff=(_dynamicMotorPositionControl_targetPosition!=p);
    if (diff)
    {
        if (getObjectCanChange())
            _dynamicMotorPositionControl_targetPosition=p;
        if (getObjectCanSync())
            _setDynamicMotorPositionControlTargetPosition_send(p);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setDynamicMotorPositionControlTargetPosition_send(float p) const
{
    sendFloat(p,sim_syncobj_joint_targetposition);
}

bool _CJoint_::getEnableDynamicMotor() const
{
    return(_dynamicMotorEnabled);
}

float _CJoint_::getDynamicMotorTargetVelocity() const
{
    return(_dynamicMotorTargetVelocity);
}

bool _CJoint_::getDynamicMotorLockModeWhenInVelocityControl() const
{
    return(_dynamicLockModeWhenInVelocityControl);
}

float _CJoint_::getDynamicMotorUpperLimitVelocity() const
{
    return(_dynamicMotorUpperLimitVelocity);
}

float _CJoint_::getDynamicMotorMaximumForce() const
{
    return(_dynamicMotorMaximumForce);
}

bool _CJoint_::getDynamicSecondPartIsValid() const
{
    return(_dynamicSecondPartIsValid);
}

C7Vector _CJoint_::getDynamicSecondPartLocalTransform() const
{
    return(_dynamicSecondPartLocalTransform);
}

bool _CJoint_::getEnableDynamicMotorControlLoop() const
{
    return(_dynamicMotorControlLoopEnabled);
}

bool _CJoint_::getEnableTorqueModulation() const
{
    return(_dynamicMotorPositionControl_torqueModulation);
}

bool _CJoint_::setDynamicMotorTargetVelocity(float v)
{
    bool diff=(_dynamicMotorTargetVelocity!=v);
    if (diff)
    {
        if (getObjectCanChange())
            _dynamicMotorTargetVelocity=v;
        if (getObjectCanSync())
            _setDynamicMotorTargetVelocity_send(v);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setDynamicMotorTargetVelocity_send(float v) const
{
    sendFloat(v,sim_syncobj_joint_targetvelocity);
}

bool _CJoint_::setDynamicMotorLockModeWhenInVelocityControl(bool e)
{
    bool diff=(_dynamicLockModeWhenInVelocityControl!=e);
    if (diff)
    {
        if (getObjectCanChange())
            _dynamicLockModeWhenInVelocityControl=e;
        if (getObjectCanSync())
            _setDynamicMotorLockModeWhenInVelocityControl_send(e);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setDynamicMotorLockModeWhenInVelocityControl_send(bool e) const
{
    sendBool(e,sim_syncobj_joint_motorlock);
}

bool _CJoint_::setDynamicMotorUpperLimitVelocity(float v)
{
    bool diff=(_dynamicMotorUpperLimitVelocity!=v);
    if (diff)
    {
        if (getObjectCanChange())
            _dynamicMotorUpperLimitVelocity=v;
        if (getObjectCanSync())
            _setDynamicMotorUpperLimitVelocity_send(v);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setDynamicMotorUpperLimitVelocity_send(float v) const
{
    sendFloat(v,sim_syncobj_joint_uppervelocitylimit);
}

bool _CJoint_::setDynamicMotorMaximumForce(float f)
{
    bool diff=(_dynamicMotorMaximumForce!=f);
    if (diff)
    {
        if (getObjectCanChange())
            _dynamicMotorMaximumForce=f;
        if (getObjectCanSync())
            _setDynamicMotorMaximumForce_send(f);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setDynamicMotorMaximumForce_send(float v) const
{
    sendFloat(v,sim_syncobj_joint_force);
}

bool _CJoint_::setDynamicSecondPartIsValid(bool v)
{
    bool diff=(_dynamicSecondPartIsValid!=v);
    if (diff)
    {
        if (getObjectCanChange())
            _dynamicSecondPartIsValid=v;
        if (getObjectCanSync())
            _setDynamicSecondPartIsValid_send(v);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setDynamicSecondPartIsValid_send(bool v) const
{
    sendBool(v,sim_syncobj_joint_secondpartvalid);
}

bool _CJoint_::setDynamicSecondPartLocalTransform(const C7Vector& tr)
{
    bool diff=(_dynamicSecondPartLocalTransform!=tr);
    if (diff)
    {
        if (getObjectCanChange())
            _dynamicSecondPartLocalTransform=tr;
        if (getObjectCanSync())
            _setDynamicSecondPartLocalTransform_send(tr);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setDynamicSecondPartLocalTransform_send(const C7Vector& tr) const
{
    sendTransformation(&tr,sim_syncobj_joint_secondparttransf);
}

bool _CJoint_::setEnableDynamicMotorControlLoop(bool p)
{
    bool diff=(_dynamicMotorControlLoopEnabled!=p);
    if (diff)
    {
        if (getObjectCanChange())
            _dynamicMotorControlLoopEnabled=p;
        if (getObjectCanSync())
            _setEnableDynamicMotorControlLoop_send(p);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setEnableDynamicMotorControlLoop_send(bool v) const
{
    sendBool(v,sim_syncobj_joint_controlenabled);
}

bool _CJoint_::setEnableTorqueModulation(bool p)
{
    bool diff=(_dynamicMotorPositionControl_torqueModulation!=p);
    if (diff)
    {
        if (getObjectCanChange())
            _dynamicMotorPositionControl_torqueModulation=p;
        if (getObjectCanSync())
            _setEnableTorqueModulation_send(p);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setEnableTorqueModulation_send(bool v) const
{
    sendBool(v,sim_syncobj_joint_torquemodulation);
}

bool _CJoint_::getHybridFunctionality() const
{
    return(_jointHasHybridFunctionality);
}

float _CJoint_::getEngineFloatParam(int what,bool* ok) const
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

int _CJoint_::getEngineIntParam(int what,bool* ok) const
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

bool _CJoint_::getEngineBoolParam(int what,bool* ok) const
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

void _CJoint_::getBulletFloatParams(std::vector<float>& p) const
{
    p.assign(_bulletFloatParams.begin(),_bulletFloatParams.end());
}

void _CJoint_::getBulletIntParams(std::vector<int>& p) const
{
    p.assign(_bulletIntParams.begin(),_bulletIntParams.end());
}

void _CJoint_::getOdeFloatParams(std::vector<float>& p) const
{
    p.assign(_odeFloatParams.begin(),_odeFloatParams.end());
}

void _CJoint_::getOdeIntParams(std::vector<int>& p) const
{
    p.assign(_odeIntParams.begin(),_odeIntParams.end());
}

void _CJoint_::getVortexFloatParams(std::vector<float>& p) const
{
    p.assign(_vortexFloatParams.begin(),_vortexFloatParams.end());
}

void _CJoint_::getVortexIntParams(std::vector<int>& p) const
{
    p.assign(_vortexIntParams.begin(),_vortexIntParams.end());
}

void _CJoint_::getNewtonFloatParams(std::vector<float>& p) const
{
    p.assign(_newtonFloatParams.begin(),_newtonFloatParams.end());
}

void _CJoint_::getNewtonIntParams(std::vector<int>& p) const
{
    p.assign(_newtonIntParams.begin(),_newtonIntParams.end());
}

int _CJoint_::getVortexDependentJointId() const
{
    return(_vortexIntParams[5]);
}

int _CJoint_::getNewtonDependentJointId() const
{
    return(_newtonIntParams[1]);
}

float _CJoint_::getPosition() const
{
    return(_jointPosition);
}

bool _CJoint_::setBulletFloatParams(const std::vector<float>& p)
{
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
    {
        if (getObjectCanChange())
            _bulletFloatParams.assign(p.begin(),p.end());
        if (getObjectCanSync())
            _setBulletFloatParams_send(p);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setBulletFloatParams_send(const std::vector<float>& p) const
{
    sendFloatArray(&p[0],p.size(),sim_syncobj_joint_bulletfloats);
}

bool _CJoint_::setBulletIntParams(const std::vector<int>& p)
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
    {
        if (getObjectCanChange())
            _bulletIntParams.assign(p.begin(),p.end());
        if (getObjectCanSync())
            _setBulletIntParams_send(p);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setBulletIntParams_send(const std::vector<int>& p) const
{
    sendInt32Array(&p[0],p.size(),sim_syncobj_joint_bulletints);
}

bool _CJoint_::setOdeFloatParams(const std::vector<float>& p)
{
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
    {
        if (getObjectCanChange())
            _odeFloatParams.assign(p.begin(),p.end());
        if (getObjectCanSync())
            _setOdeFloatParams_send(p);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setOdeFloatParams_send(const std::vector<float>& p) const
{
    sendFloatArray(&p[0],p.size(),sim_syncobj_joint_odefloats);
}

bool _CJoint_::setOdeIntParams(const std::vector<int>& p)
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
    {
        if (getObjectCanChange())
            _odeIntParams.assign(p.begin(),p.end());
        if (getObjectCanSync())
            _setOdeIntParams_send(p);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setOdeIntParams_send(const std::vector<int>& p) const
{
    sendInt32Array(&p[0],p.size(),sim_syncobj_joint_odeints);
}

bool _CJoint_::setVortexFloatParams(const std::vector<float>& p)
{
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
    {
        if (getObjectCanChange())
            _vortexFloatParams.assign(p.begin(),p.end());
        if (getObjectCanSync())
            _setVortexFloatParams_send(p);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setVortexFloatParams_send(const std::vector<float>& p) const
{
    sendFloatArray(&p[0],p.size(),sim_syncobj_joint_vortexfloats);
}

bool _CJoint_::setVortexIntParams(const std::vector<int>& p)
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
    {
        if (getObjectCanChange())
            _vortexIntParams.assign(p.begin(),p.end());
        if (getObjectCanSync())
            _setVortexIntParams_send(p);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setVortexIntParams_send(const std::vector<int>& p) const
{
    sendInt32Array(&p[0],p.size(),sim_syncobj_joint_vortexints);
}

bool _CJoint_::setNewtonFloatParams(const std::vector<float>& p)
{
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
    {
        if (getObjectCanChange())
            _newtonFloatParams.assign(p.begin(),p.end());
        if (getObjectCanSync())
            _setNewtonFloatParams_send(p);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setNewtonFloatParams_send(const std::vector<float>& p) const
{
    sendFloatArray(&p[0],p.size(),sim_syncobj_joint_newtonfloats);
}

bool _CJoint_::setNewtonIntParams(const std::vector<int>& p)
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
    {
        if (getObjectCanChange())
            _newtonIntParams.assign(p.begin(),p.end());
        if (getObjectCanSync())
            _setNewtonIntParams_send(p);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setNewtonIntParams_send(const std::vector<int>& p) const
{
    sendInt32Array(&p[0],p.size(),sim_syncobj_joint_newtonints);
}

void _CJoint_::getDynamicMotorPositionControlParameters(float& p_param,float& i_param,float& d_param) const
{
    p_param=_dynamicMotorPositionControl_P;
    i_param=_dynamicMotorPositionControl_I;
    d_param=_dynamicMotorPositionControl_D;
}

void _CJoint_::getDynamicMotorSpringControlParameters(float& k_param,float& c_param) const
{
    k_param=_dynamicMotorSpringControl_K;
    c_param=_dynamicMotorSpringControl_C;
}

float _CJoint_::getDynamicMotorPositionControlTargetPosition() const
{
    return(_dynamicMotorPositionControl_targetPosition);
}

C7Vector _CJoint_::getFullLocalTransformation() const
{ // Overridden from _CSceneObject_
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
    return(_localTransformation*jointTr);
}

bool _CJoint_::setDynamicMotorPositionControlParameters(float p_param,float i_param,float d_param)
{
    bool diff=(_dynamicMotorPositionControl_P!=p_param)||(_dynamicMotorPositionControl_I!=i_param)||(_dynamicMotorPositionControl_D!=d_param);
    if (diff)
    {
        if (getObjectCanChange())
        {
            _dynamicMotorPositionControl_P=p_param;
            _dynamicMotorPositionControl_I=i_param;
            _dynamicMotorPositionControl_D=d_param;
        }
        if (getObjectCanSync())
            _setDynamicMotorPositionControlParameters_send(p_param,i_param,d_param);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setDynamicMotorPositionControlParameters_send(float p_param,float i_param,float d_param) const
{
    float a[3]={p_param,i_param,d_param};
    sendFloatArray(a,3,sim_syncobj_joint_pid);
}

bool _CJoint_::setDynamicMotorSpringControlParameters(float k_param,float c_param)
{
    bool diff=(_dynamicMotorSpringControl_K!=k_param)||(_dynamicMotorSpringControl_C!=c_param);
    if (diff)
    {
        if (getObjectCanChange())
        {
            _dynamicMotorSpringControl_K=k_param;
            _dynamicMotorSpringControl_C=c_param;
        }
        if (getObjectCanSync())
            _setDynamicMotorSpringControlParameters_send(k_param,c_param);
    }
    return(diff&&getObjectCanChange());
}

void _CJoint_::_setDynamicMotorSpringControlParameters_send(float k_param,float c_param) const
{
    float a[2]={k_param,c_param};
    sendFloatArray(a,2,sim_syncobj_joint_kc);
}

float _CJoint_::getLength()  const
{
    return(_length);
}

float _CJoint_::getDiameter() const
{
    return(_diameter);
}

int _CJoint_::getJointType() const
{
    return(_jointType);
}

float _CJoint_::getScrewPitch() const
{
    return(_screwPitch);
}

C4Vector _CJoint_::getSphericalTransformation() const
{
    return(_sphericalTransformation);
}

bool _CJoint_::getPositionIsCyclic() const
{
    if (_jointType==sim_joint_prismatic_subtype)
        return(false);
    return(_positionIsCyclic);
}

float _CJoint_::getPositionIntervalMin() const
{
    return(_jointMinPosition);
}

float _CJoint_::getPositionIntervalRange() const
{
    return(_jointPositionRange);
}

float _CJoint_::getIKWeight() const
{
    return(_ikWeight);
}

float _CJoint_::getMaxStepSize() const
{
    return(_maxStepSize);
}

int _CJoint_::getJointMode() const
{
    return(_jointMode);
}

int _CJoint_::getDependencyMasterJointHandle() const
{
    return(_dependencyMasterJointHandle);
}

float _CJoint_::getDependencyJointMult() const
{
    return(_dependencyJointMult);
}

float _CJoint_::getDependencyJointOffset() const
{
    return(_dependencyJointOffset);
}

CColorObject* _CJoint_::getJointColor1()
{
    return(&_colorPart1);
}

CColorObject* _CJoint_::getJointColor2()
{
    return(&_colorPart2);
}
