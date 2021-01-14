#pragma once

#include <string>
#include "MyMath.h"
#include "syncObject.h"

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

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    int getObjectHandle() const;
    bool getEnabled() const;
    int getTipHandle()const ;
    int getBase() const;
    int getAlternativeBaseForConstraints() const;
    float getMinLinearPrecision() const;
    float getMinAngularPrecision() const;
    float getPositionWeight() const;
    float getOrientationWeight() const;
    int getConstraints() const;

    virtual bool setObjectHandle(int newHandle);
    virtual bool setTipHandle(int newTipHandle);

    virtual bool setEnabled(bool isEnabled);
    virtual bool setBase(int newBase);
    virtual bool setAlternativeBaseForConstraints(int b);
    virtual bool setMinLinearPrecision(float prec);
    virtual bool setMinAngularPrecision(float prec);
    virtual bool setPositionWeight(float weight);
    virtual bool setOrientationWeight(float weight);
    virtual bool setConstraints(int constr);

protected:
    virtual void _setEnabled_send(bool e) const;
    virtual void _setBase_send(int h) const;
    virtual void _setAlternativeBaseForConstraints_send(int h) const;
    virtual void _setMinLinearPrecision_send(float f) const;
    virtual void _setMinAngularPrecision_send(float f) const;
    virtual void _setPositionWeight_send(float f) const;
    virtual void _setOrientationWeight_send(float f) const;
    virtual void _setConstraints_send(int c) const;

    // Variables which need to be serialized and copied:
    int _objectHandle;
    int _tipHandle;
    int _baseHandle;
    int _constraintBaseHandle;
    float _minLinearPrecision;
    float _minAngularPrecision;
    bool _enabled;
    int _constraints; // bit-coded: use sim_x_constraint, etc. values!
    float _positionWeight;
    float _orientationWeight;
};
