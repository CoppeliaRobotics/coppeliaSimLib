
#pragma once

#include "3DObject.h"

class CMemorizedConf  
{
public:
    CMemorizedConf(C3DObject* theObject);
    CMemorizedConf(); // default constructor for serialization from memory!
    virtual ~CMemorizedConf();
    void restore();
    bool doesStillExist();
    void serializeToMemory(std::vector<char>& data);
    void serializeFromMemory(std::vector<char>& data);
    int getParentCount();

private:
    void _pushFloatToData(float d,std::vector<char>& data);
    float _popFloatFromData(std::vector<char>& data);
    void _pushIntToData(int d,std::vector<char>& data);
    int _popIntFromData(std::vector<char>& data);

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
