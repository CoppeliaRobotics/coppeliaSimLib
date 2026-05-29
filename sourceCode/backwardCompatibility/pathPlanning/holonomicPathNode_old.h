#pragma once

#include <simMath/3Vector.h>
#include <simMath/4Vector.h>
#include <simMath/7Vector.h>

class CHolonomicPathNode_old
{
  public:
    CHolonomicPathNode_old(const CQuaternion& rotAxisRot, const CQuaternion& rotAxisRotInv);
    CHolonomicPathNode_old(int theType, const CPose& conf, const CQuaternion& rotAxisRot,
                           const CQuaternion& rotAxisRotInv);
    CHolonomicPathNode_old(int theType, double searchMin[4], double searchRange[4], const CQuaternion& rotAxisRot,
                           const CQuaternion& rotAxisRotInv);
    virtual ~CHolonomicPathNode_old();

    CHolonomicPathNode_old* copyYourself();
    int getSize();
    void setAllValues(double* v);
    void setAllValues(const C3Vector& pos, const CQuaternion& orient);
    void getAllValues(C3Vector& pos, CQuaternion& orient);

    CHolonomicPathNode_old* parent;
    double* values;

  protected:
    int _nodeType;
    CQuaternion _rotAxisRot;
    CQuaternion _rotAxisRotInv;
};
