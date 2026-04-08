#pragma once

#include <obj.h>

class CCustomProperties
{
  public:
    CCustomProperties();
    virtual ~CCustomProperties();

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
    int setMatrixProperty(const char* pName, const CMatrix& pState);
    int getMatrixProperty(const char* pName, CMatrix& pState) const;
    int setMatrix3x3Property(const char* pName, const CMatrix& pState);
    int getMatrix3x3Property(const char* pName, CMatrix& pState) const;
    int setMatrix4x4Property(const char* pName, const CMatrix& pState);
    int getMatrix4x4Property(const char* pName, CMatrix& pState) const;
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
    int setHandleArrayProperty(const char* pName, const long long int* v, int vL);
    int getHandleArrayProperty(const char* pName, std::vector<long long int>& pState) const;
    int setStringArrayProperty(const char* pName, const std::vector<std::string>& pState);
    int getStringArrayProperty(const char* pName, std::vector<std::string>& pState) const;

    int removeProperty(const char* pName);
    int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const;

    bool saveToFile(const char* filename) const;
    bool loadFromFile(const char* filename);

    std::string serialize() const;
    bool deserialize(const std::string& data);

    void clear();
    size_t getPropertyCount() const;

  private:
    // Layout of value buffer:
    //   [0..3]   int32: property type
    //   [4..7]   int32: property info/flags
    //   [8..11]  int32: length of info text (N)
    //   [12..12+N-1]: info text bytes
    //   [12+N..]: property data bytes

    std::map<std::string, std::string> _properties;

    static std::string _packHeader(int propType, int propInfo, const std::string& infoTxt);
    static bool _unpackHeader(const std::string& buffer, int& propType, int& propInfo, std::string& infoTxt, size_t& dataOffset);

    static void _writeInt32(std::string& buf, size_t offset, int32_t val);
    static int32_t _readInt32(const std::string& buf, size_t offset);

    static std::string _packProperty(int propType, int propInfo, const std::string& infoTxt, const char* data, size_t dataLen);

    bool _findProperty(const char* pName, int& propType, int& propInfo, std::string& infoTxt, const char*& dataPtr, size_t& dataLen) const;
    bool _hasProperty(const char* pName) const;

    void _setPropertyRaw(const char* pName, int propType, int propInfo, const std::string& infoTxt, const char* data, size_t dataLen);
    void _updatePropertyData(const char* pName, const char* data, size_t dataLen);
};
