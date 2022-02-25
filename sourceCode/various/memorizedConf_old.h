#pragma once

#include "sceneObject.h"

class CMemorizedConf_old
{
public:
    CMemorizedConf_old(CSceneObject* theObject);
    CMemorizedConf_old(); // default constructor for serialization from memory!
    virtual ~CMemorizedConf_old();
    void restore();
    bool doesStillExist();
    void serializeToMemory(std::vector<char>& data);
    void serializeFromMemory(std::vector<char>& data);
    int getParentCount();

private:
    // General 3D objects:
    C7Vector configuration;
    long long int uniqueID;
    long long int parentUniqueID;
    int objectType;

    // Joints:
    C4Vector sphericalJointOrientation;

    // Joints and paths:
    float position;
};
