#pragma once

#include <simLib/simConst.h>
#include <simLib/simTypes.h>
#include <string>
#include <vector>
#include <map>
#include <simMath/3Vector.h>
#include <simMath/4Vector.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <propertiesAndMethods.h>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
OBJECT_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_obj;
// ----------------------------------------------------------------------------------------------

static std::vector<std::pair<int, std::string>> propertyTypes = {
    {sim_propertytype_bool, proptypetag_bool},
    {sim_propertytype_int, proptypetag_int},
    {sim_propertytype_float, proptypetag_float},
    {sim_propertytype_string, proptypetag_string},
    {sim_propertytype_intarray2, proptypetag_intarray2},
    {sim_propertytype_long, proptypetag_long},
    {sim_propertytype_vector2, proptypetag_vector2},
    {sim_propertytype_vector3, proptypetag_vector3},
    {sim_propertytype_quaternion, proptypetag_quaternion},
    {sim_propertytype_pose, proptypetag_pose},
    {sim_propertytype_matrix3x3, proptypetag_matrix3x3},
    {sim_propertytype_matrix4x4, proptypetag_matrix4x4},
    {sim_propertytype_color, proptypetag_color},
    {sim_propertytype_floatarray, proptypetag_floatarray},
    {sim_propertytype_intarray, proptypetag_intarray},
    {sim_propertytype_table, proptypetag_table},
    {sim_propertytype_matrix, proptypetag_matrix},
    {sim_propertytype_null, proptypetag_null},
    {sim_propertytype_array, proptypetag_array},
    {sim_propertytype_map, proptypetag_map},
    {sim_propertytype_handle, proptypetag_handle},
    {sim_propertytype_handlearray, proptypetag_handlearray},
    {sim_propertytype_stringarray, proptypetag_stringarray},

    {sim_propertytype_buffer, proptypetag_buffer}, // keep always at the end
};

static std::map<int, std::string> propertyStrings = {
    {sim_propertytype_bool, proptypetag_bool},
    {sim_propertytype_int, proptypetag_int},
    {sim_propertytype_float, proptypetag_float},
    {sim_propertytype_string, proptypetag_string},
    {sim_propertytype_intarray2, proptypetag_intarray2},
    {sim_propertytype_long, proptypetag_long},
    {sim_propertytype_vector2, proptypetag_vector2},
    {sim_propertytype_vector3, proptypetag_vector3},
    {sim_propertytype_quaternion, proptypetag_quaternion},
    {sim_propertytype_pose, proptypetag_pose},
    {sim_propertytype_matrix3x3, proptypetag_matrix3x3},
    {sim_propertytype_matrix4x4, proptypetag_matrix4x4},
    {sim_propertytype_color, proptypetag_color},
    {sim_propertytype_floatarray, proptypetag_floatarray},
    {sim_propertytype_intarray, proptypetag_intarray},
    {sim_propertytype_table, proptypetag_table},
    {sim_propertytype_matrix, proptypetag_matrix},
    {sim_propertytype_null, proptypetag_null},
    {sim_propertytype_array, proptypetag_array},
    {sim_propertytype_map, proptypetag_map},
    {sim_propertytype_handle, proptypetag_handle},
    {sim_propertytype_handlearray, proptypetag_handlearray},
    {sim_propertytype_stringarray, proptypetag_stringarray},

    {sim_propertytype_buffer, proptypetag_buffer},
};

class Obj
{
  public:
    Obj();
    virtual ~Obj();

    int setBoolProperty(const char* pName, bool pState);
    int getBoolProperty(const char* pName, bool& pState) const;
    int setIntProperty(const char* pName, int pState);
    int getIntProperty(const char* pName, int& pState) const;
    int setLongProperty(const char* pName, long long int pState);
    int getLongProperty(const char* pName, long long int& pState) const;
    int setFloatProperty(const char* pName, double pState);
    int getFloatProperty(const char* pName, double& pState) const;
    int setHandleProperty(const char* pName, long long int pState);
    int getHandleProperty(const char* pName, long long int& pState) const;
    int setStringProperty(const char* pName, const char* pState);
    int getStringProperty(const char* pName, std::string& pState) const;
    int setBufferProperty(const char* pName, const char* buffer, int bufferL);
    int getBufferProperty(const char* pName, std::string& pState) const;
    int setIntArray2Property(const char* pName, const int* pState);
    int getIntArray2Property(const char* pName, int* pState) const;
    int setVector2Property(const char* pName, const double* pState);
    int getVector2Property(const char* pName, double* pState) const;
    int setVector3Property(const char* pName, const C3Vector& pState);
    int getVector3Property(const char* pName, C3Vector& pState) const;
    int setQuaternionProperty(const char* pName, const C4Vector& pState);
    int getQuaternionProperty(const char* pName, C4Vector& pState) const;
    int setPoseProperty(const char* pName, const C7Vector& pState);
    int getPoseProperty(const char* pName, C7Vector& pState) const;
    int setColorProperty(const char* pName, const float* pState);
    int getColorProperty(const char* pName, float* pState) const;
    int setFloatArrayProperty(const char* pName, const double* v, int vL);
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const;
    int setIntArrayProperty(const char* pName, const int* v, int vL);
    int getIntArrayProperty(const char* pName, std::vector<int>& pState) const;
    int setHandleArrayProperty(const char* pName, const long long int* v, int vL); // ALL handle items have to be of the same type
    int getHandleArrayProperty(const char* pName, std::vector<long long int>& pState) const; // ALL handle items have to be of the same type
    int setStringArrayProperty(const char* pName, const std::vector<std::string>& pState);
    int getStringArrayProperty(const char* pName, std::vector<std::string>& pState) const;
    int removeProperty(const char* pName);
    int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const;
    static int getPropertyName_static(int& index, std::string& pName, std::string& appartenance, int excludeFlags);
    static int getPropertyInfo_static(const char* pName, int& info, std::string& infoTxt);
};
