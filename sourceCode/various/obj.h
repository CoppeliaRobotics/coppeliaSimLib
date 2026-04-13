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

    virtual int setBoolProperty(const char* pName, bool pState) { return sim_propertyret_unknownproperty; }
    virtual int getBoolProperty(const char* pName, bool& pState) const { return sim_propertyret_unknownproperty; }
    virtual int setIntProperty(const char* pName, int pState) { return sim_propertyret_unknownproperty; }
    virtual int getIntProperty(const char* pName, int& pState) const { return sim_propertyret_unknownproperty; }
    virtual int setLongProperty(const char* pName, long long int pState);
    virtual int getLongProperty(const char* pName, long long int& pState) const;
    virtual int setFloatProperty(const char* pName, double pState) { return sim_propertyret_unknownproperty; }
    virtual int getFloatProperty(const char* pName, double& pState) const { return sim_propertyret_unknownproperty; }
    virtual int setHandleProperty(const char* pName, long long int pState) { return sim_propertyret_unknownproperty; }
    virtual int getHandleProperty(const char* pName, long long int& pState) const { return sim_propertyret_unknownproperty; }
    virtual int setStringProperty(const char* pName, const char* pState) { return sim_propertyret_unknownproperty; }
    virtual int getStringProperty(const char* pName, std::string& pState) const;
    virtual int setBufferProperty(const char* pName, const char* buffer, int bufferL) { return sim_propertyret_unknownproperty; }
    virtual int getBufferProperty(const char* pName, std::string& pState) const { return sim_propertyret_unknownproperty; }
    virtual int setIntArray2Property(const char* pName, const int* pState) { return sim_propertyret_unknownproperty; }
    virtual int getIntArray2Property(const char* pName, int* pState) const { return sim_propertyret_unknownproperty; }
    virtual int setVector2Property(const char* pName, const double* pState) { return sim_propertyret_unknownproperty; }
    virtual int getVector2Property(const char* pName, double* pState) const { return sim_propertyret_unknownproperty; }
    virtual int setVector3Property(const char* pName, const C3Vector& pState) { return sim_propertyret_unknownproperty; }
    virtual int getVector3Property(const char* pName, C3Vector& pState) const { return sim_propertyret_unknownproperty; }
    virtual int setMatrixProperty(const char* pName, const CMatrix& pState) { return sim_propertyret_unknownproperty; }
    virtual int getMatrixProperty(const char* pName, CMatrix& pState) const { return sim_propertyret_unknownproperty; }
    virtual int setQuaternionProperty(const char* pName, const C4Vector& pState) { return sim_propertyret_unknownproperty; }
    virtual int getQuaternionProperty(const char* pName, C4Vector& pState) const { return sim_propertyret_unknownproperty; }
    virtual int setPoseProperty(const char* pName, const C7Vector& pState) { return sim_propertyret_unknownproperty; }
    virtual int getPoseProperty(const char* pName, C7Vector& pState) const { return sim_propertyret_unknownproperty; }
    virtual int setColorProperty(const char* pName, const float* pState) { return sim_propertyret_unknownproperty; }
    virtual int getColorProperty(const char* pName, float* pState) const { return sim_propertyret_unknownproperty; }
    virtual int setFloatArrayProperty(const char* pName, const double* v, int vL) { return sim_propertyret_unknownproperty; }
    virtual int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const { return sim_propertyret_unknownproperty; }
    virtual int setIntArrayProperty(const char* pName, const int* v, int vL) { return sim_propertyret_unknownproperty; }
    virtual int getIntArrayProperty(const char* pName, std::vector<int>& pState) const { return sim_propertyret_unknownproperty; }
    virtual int setHandleArrayProperty(const char* pName, const long long int* v, int vL) { return sim_propertyret_unknownproperty; }
    virtual int getHandleArrayProperty(const char* pName, std::vector<long long int>& pState) const { return sim_propertyret_unknownproperty; }
    virtual int setStringArrayProperty(const char* pName, const std::vector<std::string>& pState) { return sim_propertyret_unknownproperty; }
    virtual int getStringArrayProperty(const char* pName, std::vector<std::string>& pState) const { return sim_propertyret_unknownproperty; }
    virtual int removeProperty(const char* pName) { return sim_propertyret_unknownproperty; }
    virtual int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    virtual int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const;

  protected:
    long long int _objectHandle;
    std::string _objectTypeStr;
    std::string _objectMetaInfo;
};
