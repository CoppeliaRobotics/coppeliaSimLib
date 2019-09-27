
#pragma once

#include "iKGraphJoint.h"
#include "iKObject.h"
#include "iKDummy.h"

class CIKJoint : public CIKObject  
{
public:
    CIKJoint(CIKGraphJoint* joint,C7Vector& localTransformationPart1,bool isTopSpherical,bool isSphericalUp);
    virtual ~CIKJoint();
    void addAvatar(CIKJoint* avatar);
    void copyStateToAvatarKids();
    float getValueOverLimitation(bool deactivate);

    CIKGraphJoint* graphJoint;
    bool revolute;
    bool spherical;
    bool topSpherical;
    bool sphericalUp;
    bool active;
    float parameter;
    float tempParameter;
    float minValue;
    float range;
    bool cyclic;
    float weight;
    float screwPitch;
    float probableDeltaValue;
    CIKJoint* topJoint; // Needed for ball-joints
    CIKJoint* avatarParent;
    std::vector<CIKJoint*> avatarKids;
};
