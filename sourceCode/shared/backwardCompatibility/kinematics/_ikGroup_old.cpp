#include <_ikGroup_old.h>
#include <simConst.h>

_CIkGroup_old::_CIkGroup_old()
{
    _objectHandle=-1;
    _maxIterations=3;
    _enabled=true;
    _ignoreMaxStepSizes=true;
    _explicitHandling=false;
    _dampingFactor=0.1;
    _calculationMethod=sim_ik_pseudo_inverse_method;
    _restoreIfPositionNotReached=false;
    _restoreIfOrientationNotReached=false;
    _doOnFailOrSuccessOf=-1;
    _doOnFail=true;
    _doOnPerformed=true;
}

_CIkGroup_old::~_CIkGroup_old()
{
    while (_ikElements.size()!=0)
        _removeIkElement(_ikElements[0]->getObjectHandle());
}

CIkElement_old* _CIkGroup_old::getIkElementFromHandle(int ikElementHandle) const
{
    for (size_t i=0;i<_ikElements.size();i++)
    {
        if (_ikElements[i]->getObjectHandle()==ikElementHandle)
            return(_ikElements[i]);
    }
    return(nullptr);
}

CIkElement_old* _CIkGroup_old::getIkElementFromTipHandle(int tipHandle) const
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

CIkElement_old* _CIkGroup_old::getIkElementFromIndex(size_t index) const
{
    CIkElement_old* retVal=nullptr;
    if (index<_ikElements.size())
        retVal=_ikElements[index];
    return(retVal);
}

size_t _CIkGroup_old::getIkElementCount() const
{
    return(_ikElements.size());
}

void _CIkGroup_old::_addIkElement(CIkElement_old* anElement)
{
    _ikElements.push_back(anElement);
}

void _CIkGroup_old::_removeIkElement(int ikElementHandle)
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

bool _CIkGroup_old::setObjectHandle(int newHandle)
{
    bool diff=(_objectHandle!=newHandle);
    if (diff)
        _objectHandle=newHandle;
    return(diff);
}

int _CIkGroup_old::getObjectHandle() const
{
    return(_objectHandle);
}

bool _CIkGroup_old::setExplicitHandling(bool explicitHandling)
{
    bool diff=(_explicitHandling!=explicitHandling);
    if (diff)
    {
        _explicitHandling=explicitHandling;
        if (getObjectCanSync())
            _setExplicitHandling_send(explicitHandling);
    }
    return(diff);
}

void _CIkGroup_old::_setExplicitHandling_send(bool b) const
{
}

bool _CIkGroup_old::getExplicitHandling() const
{
    return(_explicitHandling);
}

bool _CIkGroup_old::setEnabled(bool enable)
{
    bool diff=(_enabled!=enable);
    if (diff)
    {
        _enabled=enable;
        if (getObjectCanSync())
            _setEnabled_send(enable);
    }
    return(diff);
}

void _CIkGroup_old::_setEnabled_send(bool e) const
{
}

bool _CIkGroup_old::getEnabled() const
{
    return(_enabled);
}

bool _CIkGroup_old::setMaxIterations(int maxIter)
{
    bool diff=(_maxIterations!=maxIter);
    if (diff)
    {
        _maxIterations=maxIter;
        if (getObjectCanSync())
            _setMaxIterations_send(maxIter);
    }
    return(diff);
}

void _CIkGroup_old::_setMaxIterations_send(int it) const
{
}

int _CIkGroup_old::getMaxIterations() const
{
    return(_maxIterations);
}

bool _CIkGroup_old::setCalculationMethod(int theMethod)
{
    bool diff=(_calculationMethod!=theMethod);
    if (diff)
    {
        _calculationMethod=theMethod;
        if (getObjectCanSync())
            _setCalculationMethod_send(theMethod);
    }
    return(diff);
}

void _CIkGroup_old::_setCalculationMethod_send(int m) const
{
}

int _CIkGroup_old::getCalculationMethod() const
{
    return(_calculationMethod);
}

bool _CIkGroup_old::setDampingFactor(double theFactor)
{
    bool diff=(_dampingFactor!=theFactor);
    if (diff)
    {
        _dampingFactor=theFactor;
        if (getObjectCanSync())
            _setDampingFactor_send(theFactor);
    }
    return(diff);
}

void _CIkGroup_old::_setDampingFactor_send(double f) const
{
}

double _CIkGroup_old::getDampingFactor() const
{
    return(_dampingFactor);
}

bool _CIkGroup_old::setIgnoreMaxStepSizes(bool ignore)
{
    bool diff=(_ignoreMaxStepSizes!=ignore);
    if (diff)
    {
        _ignoreMaxStepSizes=ignore;
        if (getObjectCanSync())
            _setIgnoreMaxStepSizes_send(ignore);
    }
    return(diff);
}

void _CIkGroup_old::_setIgnoreMaxStepSizes_send(bool e) const
{
}

bool _CIkGroup_old::getIgnoreMaxStepSizes() const
{
    return(_ignoreMaxStepSizes);
}

bool _CIkGroup_old::setRestoreIfPositionNotReached(bool active)
{
    bool diff=(_restoreIfPositionNotReached!=active);
    if (diff)
    {
        _restoreIfPositionNotReached=active;
        if (getObjectCanSync())
            _setRestoreIfPositionNotReached_send(active);
    }
    return(diff);
}

void _CIkGroup_old::_setRestoreIfPositionNotReached_send(bool e) const
{
}

bool _CIkGroup_old::getRestoreIfPositionNotReached() const
{
    return(_restoreIfPositionNotReached);
}

bool _CIkGroup_old::setRestoreIfOrientationNotReached(bool active)
{
    bool diff=(_restoreIfOrientationNotReached!=active);
    if (diff)
    {
        _restoreIfOrientationNotReached=active;
        if (getObjectCanSync())
            _setRestoreIfOrientationNotReached_send(active);
    }
    return(diff);
}

void _CIkGroup_old::_setRestoreIfOrientationNotReached_send(bool e) const
{
}

bool _CIkGroup_old::getRestoreIfOrientationNotReached() const
{
    return(_restoreIfOrientationNotReached);
}

bool _CIkGroup_old::setDoOnFailOrSuccessOf(int groupID,bool check)
{
    bool diff=(_doOnFailOrSuccessOf!=groupID);
    if (diff)
    {
        _doOnFailOrSuccessOf=groupID;
        if (getObjectCanSync())
            _setDoOnFailOrSuccessOf_send(groupID);
    }
    return(diff);
}

void _CIkGroup_old::_setDoOnFailOrSuccessOf_send(int h) const
{
}

int _CIkGroup_old::getDoOnFailOrSuccessOf() const
{
    return(_doOnFailOrSuccessOf);
}

bool _CIkGroup_old::setDoOnFail(bool onFail)
{
    bool diff=(_doOnFail!=onFail);
    if (diff)
    {
        _doOnFail=onFail;
        if (getObjectCanSync())
            _setDoOnFail_send(onFail);
    }
    return(diff);
}

void _CIkGroup_old::_setDoOnFail_send(bool e) const
{
}

bool _CIkGroup_old::getDoOnFail() const
{
    return(_doOnFail);
}

bool _CIkGroup_old::setDoOnPerformed(bool turnOn)
{
    bool diff=(_doOnPerformed!=turnOn);
    if (diff)
    {
        _doOnPerformed=turnOn;
        if (getObjectCanSync())
            _setDoOnPerformed_send(turnOn);
    }
    return(diff);
}

void _CIkGroup_old::_setDoOnPerformed_send(bool e) const
{
}

bool _CIkGroup_old::getDoOnPerformed() const
{
    return(_doOnPerformed);
}

bool _CIkGroup_old::setObjectName(const char* newName,bool check)
{
    bool diff=(_objectName!=newName);
    if (diff)
    {
        _objectName=newName;
        if (getObjectCanSync())
            _setObjectName_send(newName);
    }
    return(diff);
}

void _CIkGroup_old::_setObjectName_send(const char* nm) const
{
}

std::string _CIkGroup_old::getObjectName() const
{
    return(_objectName);
}

