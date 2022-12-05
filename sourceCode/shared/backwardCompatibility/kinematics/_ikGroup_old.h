#pragma once

#include <string>
#include "MyMath.h"
#include "ikElement_old.h"
#include "syncObject.h"

enum {
    sim_syncobj_ikgroup_create=0,
    sim_syncobj_ikgroup_delete,
    sim_syncobj_ikgroup_explicit,
    sim_syncobj_ikgroup_enabled,
    sim_syncobj_ikgroup_maxiterations,
    sim_syncobj_ikgroup_calcmethod,
    sim_syncobj_ikgroup_damping,
    sim_syncobj_ikgroup_ignoremaxstepsize,
    sim_syncobj_ikgroup_restoreifposnotreached,
    sim_syncobj_ikgroup_restoreiforientnotreached,
    sim_syncobj_ikgroup_doonfailorsuccessof,
    sim_syncobj_ikgroup_doonfail,
    sim_syncobj_ikgroup_doonperformed,
    sim_syncobj_ikgroup_setobjectname,
};

class _CIkGroup_old : public CSyncObject
{
public:
    _CIkGroup_old();
    virtual ~_CIkGroup_old();

    CIkElement_old* getIkElementFromHandle(int ikElementHandle) const;
    CIkElement_old* getIkElementFromTipHandle(int tipHandle) const;
    CIkElement_old* getIkElementFromIndex(size_t index) const;
    size_t getIkElementCount() const;

    int getObjectHandle() const;
    bool getExplicitHandling() const;
    bool getEnabled() const;
    int getMaxIterations() const;
    int getCalculationMethod() const;
    double getDampingFactor() const;
    bool getIgnoreMaxStepSizes() const;
    bool getRestoreIfPositionNotReached() const;
    bool getRestoreIfOrientationNotReached() const;
    int getDoOnFailOrSuccessOf() const;
    bool getDoOnFail() const;
    bool getDoOnPerformed() const;
    std::string getObjectName() const;

    virtual bool setObjectHandle(int newHandle);

    virtual bool setExplicitHandling(bool explicitHandling);
    virtual bool setEnabled(bool enable);
    virtual bool setMaxIterations(int maxIter);
    virtual bool setCalculationMethod(int theMethod);
    virtual bool setDampingFactor(double theFactor);
    virtual bool setIgnoreMaxStepSizes(bool ignore);
    virtual bool setRestoreIfPositionNotReached(bool _enabled);
    virtual bool setRestoreIfOrientationNotReached(bool _enabled);
    virtual bool setDoOnFailOrSuccessOf(int groupID,bool check);
    virtual bool setDoOnFail(bool onFail);
    virtual bool setDoOnPerformed(bool turnOn);
    virtual bool setObjectName(const char* newName,bool check);

protected:
    virtual void _addIkElement(CIkElement_old* anElement);
    virtual void _removeIkElement(int ikElementHandle);

    virtual void _setExplicitHandling_send(bool b) const;
    virtual void _setEnabled_send(bool e) const;
    virtual void _setMaxIterations_send(int it) const;
    virtual void _setCalculationMethod_send(int m) const;
    virtual void _setDampingFactor_send(double f) const;
    virtual void _setIgnoreMaxStepSizes_send(bool e) const;
    virtual void _setRestoreIfPositionNotReached_send(bool e) const;
    virtual void _setRestoreIfOrientationNotReached_send(bool e) const;
    virtual void _setDoOnFailOrSuccessOf_send(int h) const;
    virtual void _setDoOnFail_send(bool e) const;
    virtual void _setDoOnPerformed_send(bool e) const;
    virtual void _setObjectName_send(const char* nm) const;

    int _objectHandle;
    bool _explicitHandling;
    bool _enabled;
    int _maxIterations;
    int _calculationMethod;
    double _dampingFactor;
    bool _ignoreMaxStepSizes;
    bool _restoreIfPositionNotReached;
    bool _restoreIfOrientationNotReached;
    int _doOnFailOrSuccessOf; // group identifier which success/fail will be evaluated
    bool _doOnFail;
    bool _doOnPerformed;
    std::string _objectName;

private:
    std::vector<CIkElement_old*> _ikElements;
};
