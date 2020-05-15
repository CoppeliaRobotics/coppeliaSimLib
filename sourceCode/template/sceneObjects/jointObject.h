#pragma once

#include "_jointObject_.h"

class CJoint : public CSceneObject, public _CJoint_
{
public:
    CJoint();
    CJoint(int jointType);
    virtual ~CJoint();

private:
    static int cnt;
};
