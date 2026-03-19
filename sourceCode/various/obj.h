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
#include <cbor.h>

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
    Obj(long long int objectHandle, const char* objectTypeStr, const char* objectMetaInfo);
    virtual ~Obj();

    virtual void addObjectEventData(CCbor* ev);

    long long int getObjectHandle() const;
    std::string getObjectTypeStr() const;
    std::string getObjectMetaInfo() const;

    virtual int getLongProperty(const char* pName, long long int& pState) const;
    virtual int getStringProperty(const char* pName, std::string& pState) const;
    virtual int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    virtual int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const;

  protected:
    long long int _objectHandle;
    std::string _objectTypeStr;
    std::string _objectMetaInfo;
};
