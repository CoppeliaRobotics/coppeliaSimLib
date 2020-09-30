#include "_ikGroup_.h"
#include "simConst.h"

_CIkGroup_::_CIkGroup_()
{
    _objectHandle=-1;
    _maxIterations=3;
    _enabled=true;
    _ignoreMaxStepSizes=true;
    _explicitHandling=false;
    _dampingFactor=0.1f;
    _calculationMethod=sim_ik_pseudo_inverse_method;
    _restoreIfPositionNotReached=false;
    _restoreIfOrientationNotReached=false;
    _doOnFailOrSuccessOf=-1;
    _doOnFail=true;
    _doOnPerformed=true;
}

_CIkGroup_::~_CIkGroup_()
{
    while (_ikElements.size()!=0)
        _removeIkElement(_ikElements[0]->getObjectHandle());
}

CIkElement* _CIkGroup_::getIkElementFromHandle(int ikElementHandle) const
{
    for (size_t i=0;i<_ikElements.size();i++)
    {
        if (_ikElements[i]->getObjectHandle()==ikElementHandle)
            return(_ikElements[i]);
    }
    return(nullptr);
}

CIkElement* _CIkGroup_::getIkElementFromTipHandle(int tipHandle) const
{
    if (tipHandle!=-1)
    {
        for (size_t i=0;i<_ikElements.size();i++)
        {
            if (_ikElements[i]->getTipHandle()==tipHandle)
                return(_ikElements[i]);
        }
    }
    return(nullptr);
}

CIkElement* _CIkGroup_::getIkElementFromIndex(size_t index) const
{
    CIkElement* retVal=nullptr;
    if (index<_ikElements.size())
        retVal=_ikElements[index];
    return(retVal);
}

size_t _CIkGroup_::getIkElementCount() const
{
    return(_ikElements.size());
}

void _CIkGroup_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
// notReplicatedAnymoreSinceOldFunc
    /*
    if (routing.size()>0)
    {
        if (routing[0].objType==sim_syncobj_ikelement)
        { // Msg is for an IK element
            CIkElement* el=getIkElementFromHandle(routing[0].objHandle);
            if (el==nullptr)
            {
                if ( (msg.msg==sim_syncobj_ikelement_create)&&(routing.size()==1) ) // check also size (some msgs have same ids in different scopes)
                {
                    el=new CIkElement(((int*)msg.data)[0]);
                    el->setObjectHandle(routing[0].objHandle);
                    _addIkElement(el);
                }
            }
            else
            {
                if ( (msg.msg==sim_syncobj_ikelement_delete)&&(routing.size()==1) ) // check also size (some msgs have same ids in different scopes)
                    _removeIkElement(routing[0].objHandle);
                else
                {
                    routing.erase(routing.begin());
                    el->synchronizationMsg(routing,msg);
                }
            }
        }
    }
    else
    { // Msg is for this IK group
        if (msg.msg==sim_syncobj_ikgroup_explicit)
        {
            setExplicitHandling(((bool*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_ikgroup_enabled)
        {
            setEnabled(((bool*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_ikgroup_maxiterations)
        {
            setMaxIterations(((int*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_ikgroup_calcmethod)
        {
            setCalculationMethod(((int*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_ikgroup_damping)
        {
            setDampingFactor(((float*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_ikgroup_ignoremaxstepsize)
        {
            setIgnoreMaxStepSizes(((bool*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_ikgroup_restoreifposnotreached)
        {
            setRestoreIfPositionNotReached(((bool*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_ikgroup_restoreiforientnotreached)
        {
            setRestoreIfOrientationNotReached(((bool*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_ikgroup_doonfailorsuccessof)
        {
            setDoOnFailOrSuccessOf(((int*)msg.data)[0],true);
            return;
        }
        if (msg.msg==sim_syncobj_ikgroup_doonfail)
        {
            setDoOnFail(((bool*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_ikgroup_doonperformed)
        {
            setDoOnPerformed(((bool*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_ikgroup_setobjectname)
        {
            setObjectName(((char*)msg.data),true);
            return;
        }
    }
    */
}

void _CIkGroup_::_addIkElement(CIkElement* anElement)
{
    _ikElements.push_back(anElement);
}

void _CIkGroup_::_removeIkElement(int ikElementHandle)
{
    for (size_t i=0;i<_ikElements.size();i++)
    {
        if (_ikElements[i]->getObjectHandle()==ikElementHandle)
        {
            delete _ikElements[i];
            _ikElements.erase(_ikElements.begin()+i);
            break;
        }
    }
}

bool _CIkGroup_::setObjectHandle(int newHandle)
{
    bool diff=(_objectHandle!=newHandle);
    if (diff)
        _objectHandle=newHandle;
    return(diff);
}

int _CIkGroup_::getObjectHandle() const
{
    return(_objectHandle);
}

bool _CIkGroup_::setExplicitHandling(bool explicitHandling)
{
    bool diff=(_explicitHandling!=explicitHandling);
    if (diff)
    {
        if (getObjectCanChange())
            _explicitHandling=explicitHandling;
        if (getObjectCanSync())
            _setExplicitHandling_send(explicitHandling);
    }
    return(diff&&getObjectCanChange());
}

void _CIkGroup_::_setExplicitHandling_send(bool b) const
{
// notReplicatedAnymoreSinceOldFunc    sendBool(b,sim_syncobj_ikgroup_explicit);
}

bool _CIkGroup_::getExplicitHandling() const
{
    return(_explicitHandling);
}

bool _CIkGroup_::setEnabled(bool enable)
{
    bool diff=(_enabled!=enable);
    if (diff)
    {
        if (getObjectCanChange())
            _enabled=enable;
        if (getObjectCanSync())
            _setEnabled_send(enable);
    }
    return(diff&&getObjectCanChange());
}

void _CIkGroup_::_setEnabled_send(bool e) const
{
// notReplicatedAnymoreSinceOldFunc    sendBool(e,sim_syncobj_ikgroup_enabled);
}

bool _CIkGroup_::getEnabled() const
{
    return(_enabled);
}

bool _CIkGroup_::setMaxIterations(int maxIter)
{
    bool diff=(_maxIterations!=maxIter);
    if (diff)
    {
        if (getObjectCanChange())
            _maxIterations=maxIter;
        if (getObjectCanSync())
            _setMaxIterations_send(maxIter);
    }
    return(diff&&getObjectCanChange());
}

void _CIkGroup_::_setMaxIterations_send(int it) const
{
// notReplicatedAnymoreSinceOldFunc    sendInt32(it,sim_syncobj_ikgroup_maxiterations);
}

int _CIkGroup_::getMaxIterations() const
{
    return(_maxIterations);
}

bool _CIkGroup_::setCalculationMethod(int theMethod)
{
    bool diff=(_calculationMethod!=theMethod);
    if (diff)
    {
        if (getObjectCanChange())
            _calculationMethod=theMethod;
        if (getObjectCanSync())
            _setCalculationMethod_send(theMethod);
    }
    return(diff&&getObjectCanChange());
}

void _CIkGroup_::_setCalculationMethod_send(int m) const
{
// notReplicatedAnymoreSinceOldFunc    sendInt32(m,sim_syncobj_ikgroup_calcmethod);
}

int _CIkGroup_::getCalculationMethod() const
{
    return(_calculationMethod);
}

bool _CIkGroup_::setDampingFactor(float theFactor)
{
    bool diff=(_dampingFactor!=theFactor);
    if (diff)
    {
        if (getObjectCanChange())
            _dampingFactor=theFactor;
        if (getObjectCanSync())
            _setDampingFactor_send(theFactor);
    }
    return(diff&&getObjectCanChange());
}

void _CIkGroup_::_setDampingFactor_send(float f) const
{
// notReplicatedAnymoreSinceOldFunc    sendFloat(f,sim_syncobj_ikgroup_damping);
}

float _CIkGroup_::getDampingFactor() const
{
    return(_dampingFactor);
}

bool _CIkGroup_::setIgnoreMaxStepSizes(bool ignore)
{
    bool diff=(_ignoreMaxStepSizes!=ignore);
    if (diff)
    {
        if (getObjectCanChange())
            _ignoreMaxStepSizes=ignore;
        if (getObjectCanSync())
            _setIgnoreMaxStepSizes_send(ignore);
    }
    return(diff&&getObjectCanChange());
}

void _CIkGroup_::_setIgnoreMaxStepSizes_send(bool e) const
{
// notReplicatedAnymoreSinceOldFunc    sendBool(e,sim_syncobj_ikgroup_ignoremaxstepsize);
}

bool _CIkGroup_::getIgnoreMaxStepSizes() const
{
    return(_ignoreMaxStepSizes);
}

bool _CIkGroup_::setRestoreIfPositionNotReached(bool active)
{
    bool diff=(_restoreIfPositionNotReached!=active);
    if (diff)
    {
        if (getObjectCanChange())
            _restoreIfPositionNotReached=active;
        if (getObjectCanSync())
            _setRestoreIfPositionNotReached_send(active);
    }
    return(diff&&getObjectCanChange());
}

void _CIkGroup_::_setRestoreIfPositionNotReached_send(bool e) const
{
// notReplicatedAnymoreSinceOldFunc    sendBool(e,sim_syncobj_ikgroup_restoreifposnotreached);
}

bool _CIkGroup_::getRestoreIfPositionNotReached() const
{
    return(_restoreIfPositionNotReached);
}

bool _CIkGroup_::setRestoreIfOrientationNotReached(bool active)
{
    bool diff=(_restoreIfOrientationNotReached!=active);
    if (diff)
    {
        if (getObjectCanChange())
            _restoreIfOrientationNotReached=active;
        if (getObjectCanSync())
            _setRestoreIfOrientationNotReached_send(active);
    }
    return(diff&&getObjectCanChange());
}

void _CIkGroup_::_setRestoreIfOrientationNotReached_send(bool e) const
{
// notReplicatedAnymoreSinceOldFunc    sendBool(e,sim_syncobj_ikgroup_restoreiforientnotreached);
}

bool _CIkGroup_::getRestoreIfOrientationNotReached() const
{
    return(_restoreIfOrientationNotReached);
}

bool _CIkGroup_::setDoOnFailOrSuccessOf(int groupID,bool check)
{
    bool diff=(_doOnFailOrSuccessOf!=groupID);
    if (diff)
    {
        if (getObjectCanChange())
            _doOnFailOrSuccessOf=groupID;
        if (getObjectCanSync())
            _setDoOnFailOrSuccessOf_send(groupID);
    }
    return(diff&&getObjectCanChange());
}

void _CIkGroup_::_setDoOnFailOrSuccessOf_send(int h) const
{
// notReplicatedAnymoreSinceOldFunc    sendInt32(h,sim_syncobj_ikgroup_doonfailorsuccessof);
}

int _CIkGroup_::getDoOnFailOrSuccessOf() const
{
    return(_doOnFailOrSuccessOf);
}

bool _CIkGroup_::setDoOnFail(bool onFail)
{
    bool diff=(_doOnFail!=onFail);
    if (diff)
    {
        if (getObjectCanChange())
            _doOnFail=onFail;
        if (getObjectCanSync())
            _setDoOnFail_send(onFail);
    }
    return(diff&&getObjectCanChange());
}

void _CIkGroup_::_setDoOnFail_send(bool e) const
{
// notReplicatedAnymoreSinceOldFunc    sendBool(e,sim_syncobj_ikgroup_doonfail);
}

bool _CIkGroup_::getDoOnFail() const
{
    return(_doOnFail);
}

bool _CIkGroup_::setDoOnPerformed(bool turnOn)
{
    bool diff=(_doOnPerformed!=turnOn);
    if (diff)
    {
        if (getObjectCanChange())
            _doOnPerformed=turnOn;
        if (getObjectCanSync())
            _setDoOnPerformed_send(turnOn);
    }
    return(diff&&getObjectCanChange());
}

void _CIkGroup_::_setDoOnPerformed_send(bool e) const
{
// notReplicatedAnymoreSinceOldFunc    sendBool(e,sim_syncobj_ikgroup_doonperformed);
}

bool _CIkGroup_::getDoOnPerformed() const
{
    return(_doOnPerformed);
}

bool _CIkGroup_::setObjectName(const char* newName,bool check)
{
    bool diff=(_objectName!=newName);
    if (diff)
    {
        if (getObjectCanChange())
            _objectName=newName;
        if (getObjectCanSync())
            _setObjectName_send(newName);
    }
    return(diff&&getObjectCanChange());
}

void _CIkGroup_::_setObjectName_send(const char* nm) const
{
// notReplicatedAnymoreSinceOldFunc    sendString(nm,sim_syncobj_ikgroup_setobjectname);
}

std::string _CIkGroup_::getObjectName() const
{
    return(_objectName);
}

