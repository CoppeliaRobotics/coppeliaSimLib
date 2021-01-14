#include "_ikElement_old.h"
#include "simConst.h"

_CIkElement_old::_CIkElement_old()
{
    _baseHandle=-1;
    _constraintBaseHandle=-1;
    _minAngularPrecision=0.1f*degToRad_f;
    _minLinearPrecision=0.0005f;
    _enabled=true;
    _constraints=sim_ik_x_constraint|sim_ik_y_constraint|sim_ik_z_constraint;
    _positionWeight=1.0f;
    _orientationWeight=1.0f;
}

_CIkElement_old::~_CIkElement_old()
{
}

void _CIkElement_old::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
// notReplicatedAnymoreSinceOldFunc
    /*
    if (msg.msg==sim_syncobj_ikelement_enabled)
    {
        setEnabled(((bool*)msg.data)[0]);
        return;
    }
    if (msg.msg==sim_syncobj_ikelement_basehandle)
    {
        setBase(((int*)msg.data)[0]);
        return;
    }
    if (msg.msg==sim_syncobj_ikelement_constrbasehandle)
    {
        setAlternativeBaseForConstraints(((int*)msg.data)[0]);
        return;
    }
    if (msg.msg==sim_syncobj_ikelement_linprecision)
    {
        setMinLinearPrecision(((float*)msg.data)[0]);
        return;
    }
    if (msg.msg==sim_syncobj_ikelement_angprecision)
    {
        setMinAngularPrecision(((float*)msg.data)[0]);
        return;
    }
    if (msg.msg==sim_syncobj_ikelement_linweight)
    {
        setPositionWeight(((float*)msg.data)[0]);
        return;
    }
    if (msg.msg==sim_syncobj_ikelement_angweight)
    {
        setOrientationWeight(((float*)msg.data)[0]);
        return;
    }
    if (msg.msg==sim_syncobj_ikelement_constraints)
    {
        setConstraints(((int*)msg.data)[0]);
        return;
    }
    */
}

bool _CIkElement_old::setObjectHandle(int newHandle)
{
    bool diff=(_objectHandle!=newHandle);
    _objectHandle=newHandle;
    return(diff);
}

int _CIkElement_old::getObjectHandle() const
{
    return(_objectHandle);
}

bool _CIkElement_old::setEnabled(bool isEnabled)
{
    bool diff=(_enabled!=isEnabled);
    if (diff)
    {
        if (getObjectCanChange())
            _enabled=isEnabled;
        if (getObjectCanSync())
            _setEnabled_send(isEnabled);
    }
    return(diff&&getObjectCanChange());
}

void _CIkElement_old::_setEnabled_send(bool e) const
{
// notReplicatedAnymoreSinceOldFunc    sendBool(e,sim_syncobj_ikelement_enabled);
}

bool _CIkElement_old::getEnabled() const
{
    return(_enabled);
}

bool _CIkElement_old::setTipHandle(int newTipHandle)
{
    bool diff=(_tipHandle!=newTipHandle);
    _tipHandle=newTipHandle;
    return(diff);
}

int _CIkElement_old::getTipHandle() const
{
    return(_tipHandle);
}

bool _CIkElement_old::setBase(int newBase)
{
    bool diff=(_baseHandle!=newBase);
    if (diff)
    {
        if (getObjectCanChange())
            _baseHandle=newBase;
        if (getObjectCanSync())
            _setBase_send(newBase);
    }
    return(diff&&getObjectCanChange());
}

void _CIkElement_old::_setBase_send(int h) const
{
// notReplicatedAnymoreSinceOldFunc   sendInt32(h,sim_syncobj_ikelement_basehandle);
}

int _CIkElement_old::getBase() const
{
    return(_baseHandle);
}

bool _CIkElement_old::setAlternativeBaseForConstraints(int b)
{
    bool diff=(_constraintBaseHandle!=b);
    if (diff)
    {
        if (getObjectCanChange())
            _constraintBaseHandle=b;
        if (getObjectCanSync())
            _setAlternativeBaseForConstraints_send(b);
    }
    return(diff&&getObjectCanChange());
}

void _CIkElement_old::_setAlternativeBaseForConstraints_send(int h) const
{
// notReplicatedAnymoreSinceOldFunc    sendInt32(h,sim_syncobj_ikelement_constrbasehandle);
}

int _CIkElement_old::getAlternativeBaseForConstraints() const
{
    return(_constraintBaseHandle);
}

bool _CIkElement_old::setMinLinearPrecision(float prec)
{
    bool diff=(_minLinearPrecision!=prec);
    if (diff)
    {
        if (getObjectCanChange())
            _minLinearPrecision=prec;
        if (getObjectCanSync())
            _setMinLinearPrecision_send(prec);
    }
    return(diff&&getObjectCanChange());
}

void _CIkElement_old::_setMinLinearPrecision_send(float f) const
{
// notReplicatedAnymoreSinceOldFunc    sendFloat(f,sim_syncobj_ikelement_linprecision);
}

float _CIkElement_old::getMinLinearPrecision() const
{
    return(_minLinearPrecision);
}

bool _CIkElement_old::setMinAngularPrecision(float prec)
{
    bool diff=(_minAngularPrecision!=prec);
    if (diff)
    {
        if (getObjectCanChange())
            _minAngularPrecision=prec;
        if (getObjectCanSync())
            _setMinAngularPrecision_send(prec);
    }
    return(diff&&getObjectCanChange());
}

void _CIkElement_old::_setMinAngularPrecision_send(float f) const
{
// notReplicatedAnymoreSinceOldFunc    sendFloat(f,sim_syncobj_ikelement_angprecision);
}

float _CIkElement_old::getMinAngularPrecision() const
{
    return(_minAngularPrecision);
}

bool _CIkElement_old::setPositionWeight(float weight)
{
    bool diff=(_positionWeight!=weight);
    if (diff)
    {
        if (getObjectCanChange())
            _positionWeight=weight;
        if (getObjectCanSync())
            _setPositionWeight_send(weight);
    }
    return(diff&&getObjectCanChange());
}

void _CIkElement_old::_setPositionWeight_send(float f) const
{
// notReplicatedAnymoreSinceOldFunc    sendFloat(f,sim_syncobj_ikelement_linweight);
}

float _CIkElement_old::getPositionWeight() const
{
    return(_positionWeight);
}

bool _CIkElement_old::setOrientationWeight(float weight)
{
    bool diff=(_orientationWeight!=weight);
    if (diff)
    {
        if (getObjectCanChange())
            _orientationWeight=weight;
        if (getObjectCanSync())
            _setOrientationWeight_send(weight);
    }
    return(diff&&getObjectCanChange());
}

void _CIkElement_old::_setOrientationWeight_send(float f) const
{
// notReplicatedAnymoreSinceOldFunc    sendFloat(f,sim_syncobj_ikelement_angweight);
}

float _CIkElement_old::getOrientationWeight() const
{
    return(_orientationWeight);
}

bool _CIkElement_old::setConstraints(int constr)
{
    bool diff=(_constraints!=constr);
    if (diff)
    {
        if (getObjectCanChange())
            _constraints=constr;
        if (getObjectCanSync())
            _setConstraints_send(constr);
    }
    return(diff&&getObjectCanChange());
}

void _CIkElement_old::_setConstraints_send(int c) const
{
// notReplicatedAnymoreSinceOldFunc    sendInt32(c,sim_syncobj_ikelement_constraints);
}

int _CIkElement_old::getConstraints() const
{
    return(_constraints);
}

