#pragma once

#include <sceneObject.h>

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
    static void pushFloatToBuffer(double d, std::vector<char>& data);
    static double popFloatFromBuffer(std::vector<char>& data);
    static void pushIntToBuffer(int d, std::vector<char>& data);
    static int popIntFromBuffer(std::vector<char>& data);

    // General 3D objects:
    CPose configuration;
    long long int uniqueID;
    long long int parentUniqueID;
    int objectType;

    // Joints:
    CQuaternion sphericalJointOrientation;

    // Joints and paths:
    double position;
};
