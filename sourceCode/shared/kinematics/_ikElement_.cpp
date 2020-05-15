#include "_ikElement_.h"
#include "simConst.h"

_CIkElement_::_CIkElement_()
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

_CIkElement_::~_CIkElement_()
{
}

void _CIkElement_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
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
}

bool _CIkElement_::setObjectHandle(int newHandle)
{
    bool diff=(_objectHandle!=newHandle);
    _objectHandle=newHandle;
    return(diff);
}

int _CIkElement_::getObjectHandle() const
{
    return(_objectHandle);
}

bool _CIkElement_::setEnabled(bool isEnabled)
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

void _CIkElement_::_setEnabled_send(bool e) const
{
    sendBool(e,sim_syncobj_ikelement_enabled);
}

bool _CIkElement_::getEnabled() const
{
    return(_enabled);
}

bool _CIkElement_::setTipHandle(int newTipHandle)
{
    bool diff=(_tipHandle!=newTipHandle);
    _tipHandle=newTipHandle;
    return(diff);
}

int _CIkElement_::getTipHandle() const
{
    return(_tipHandle);
}

bool _CIkElement_::setBase(int newBase)
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

void _CIkElement_::_setBase_send(int h) const
{
    sendInt32(h,sim_syncobj_ikelement_basehandle);
}

int _CIkElement_::getBase() const
{
    return(_baseHandle);
}

bool _CIkElement_::setAlternativeBaseForConstraints(int b)
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

void _CIkElement_::_setAlternativeBaseForConstraints_send(int h) const
{
    sendInt32(h,sim_syncobj_ikelement_constrbasehandle);
}

int _CIkElement_::getAlternativeBaseForConstraints() const
{
    return(_constraintBaseHandle);
}

bool _CIkElement_::setMinLinearPrecision(float prec)
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

void _CIkElement_::_setMinLinearPrecision_send(float f) const
{
    sendFloat(f,sim_syncobj_ikelement_linprecision);
}

float _CIkElement_::getMinLinearPrecision() const
{
    return(_minLinearPrecision);
}

bool _CIkElement_::setMinAngularPrecision(float prec)
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

void _CIkElement_::_setMinAngularPrecision_send(float f) const
{
    sendFloat(f,sim_syncobj_ikelement_angprecision);
}

float _CIkElement_::getMinAngularPrecision() const
{
    return(_minAngularPrecision);
}

bool _CIkElement_::setPositionWeight(float weight)
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

void _CIkElement_::_setPositionWeight_send(float f) const
{
    sendFloat(f,sim_syncobj_ikelement_linweight);
}

float _CIkElement_::getPositionWeight() const
{
    return(_positionWeight);
}

bool _CIkElement_::setOrientationWeight(float weight)
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

void _CIkElement_::_setOrientationWeight_send(float f) const
{
    sendFloat(f,sim_syncobj_ikelement_angweight);
}

float _CIkElement_::getOrientationWeight() const
{
    return(_orientationWeight);
}

bool _CIkElement_::setConstraints(int constr)
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

void _CIkElement_::_setConstraints_send(int c) const
{
    sendInt32(c,sim_syncobj_ikelement_constraints);
}

int _CIkElement_::getConstraints() const
{
    return(_constraints);
}

