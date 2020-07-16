
#pragma once

#include "sceneObject.h"

class CMemorizedConf  
{
public:
    CMemorizedConf(CSceneObject* theObject);
    CMemorizedConf(); // default constructor for serialization from memory!
    virtual ~CMemorizedConf();
    void restore();
    bool doesStillExist();
    void serializeToMemory(std::vector<char>& data);
    void serializeFromMemory(std::vector<char>& data);
    int getParentCount();

private:
    // General 3D objects:
    C7Vector configuration;
    int uniqueID;
    int parentUniqueID;
    int objectType;
    int memorizedConfigurationValidCounter;

    // Joints:
    C4Vector sphericalJointOrientation;

    // Joints and paths:
    float position;
};
