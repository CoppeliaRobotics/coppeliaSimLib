#pragma once

#include <string>
#include <simMath/MyMath.h>
#include <syncObject.h>

enum {
    sim_syncobj_ikelement_create=0,
    sim_syncobj_ikelement_delete,
    sim_syncobj_ikelement_enabled,
    sim_syncobj_ikelement_basehandle,
    sim_syncobj_ikelement_constrbasehandle,
    sim_syncobj_ikelement_linprecision,
    sim_syncobj_ikelement_angprecision,
    sim_syncobj_ikelement_linweight,
    sim_syncobj_ikelement_angweight,
    sim_syncobj_ikelement_constraints,
};

class _CIkElement_old : public CSyncObject
{
public:
    _CIkElement_old();
    virtual ~_CIkElement_old();

    int getObjectHandle() const;
    bool getEnabled() const;
    int getTipHandle()const ;
    int getBase() const;
    int getAlternativeBaseForConstraints() const;
    double getMinLinearPrecision() const;
    double getMinAngularPrecision() const;
    double getPositionWeight() const;
    double getOrientationWeight() const;
    int getConstraints() const;

    virtual bool setObjectHandle(int newHandle);
    virtual bool setTipHandle(int newTipHandle);

    virtual bool setEnabled(bool isEnabled);
    virtual bool setBase(int newBase);
    virtual bool setAlternativeBaseForConstraints(int b);
    virtual bool setMinLinearPrecision(double prec);
    virtual bool setMinAngularPrecision(double prec);
    virtual bool setPositionWeight(double weight);
    virtual bool setOrientationWeight(double weight);
    virtual bool setConstraints(int constr);

protected:
    virtual void _setEnabled_send(bool e) const;
    virtual void _setBase_send(int h) const;
    virtual void _setAlternativeBaseForConstraints_send(int h) const;
    virtual void _setMinLinearPrecision_send(double f) const;
    virtual void _setMinAngularPrecision_send(double f) const;
    virtual void _setPositionWeight_send(double f) const;
    virtual void _setOrientationWeight_send(double f) const;
    virtual void _setConstraints_send(int c) const;

    // Variables which need to be serialized and copied:
    int _objectHandle;
    int _tipHandle;
    int _baseHandle;
    int _constraintBaseHandle;
    double _minLinearPrecision;
    double _minAngularPrecision;
    bool _enabled;
    int _constraints; // bit-coded: use sim_x_constraint, etc. values!
    double _positionWeight;
    double _orientationWeight;
};
