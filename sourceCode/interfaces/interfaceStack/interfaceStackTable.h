#pragma once

#include <interfaceStackObject.h>
#include <vector>

class CInterfaceStackTable : public CInterfaceStackObject
{
  public:
    CInterfaceStackTable();
    virtual ~CInterfaceStackTable();

    CInterfaceStackObject *copyYourself() const;
    void printContent(int spaces, std::string &buffer) const;
    std::string getObjectData(std::string &auxInfos) const;
    void addCborObjectData(CCbor *cborObj) const;
    unsigned int createFromData(const char *data, unsigned char version, std::vector<CInterfaceStackObject*> &allCreatedObjects);
    static bool checkCreateFromData(const char *data, unsigned int &w, unsigned int l, unsigned char version);

    bool isEmpty() const;
    bool isTableArray() const;
    size_t getArraySize() const;
    size_t getMapEntryCount() const;
    bool isCircularRef() const;
    void setCircularRef();

    CInterfaceStackObject *getArrayItemAtIndex(size_t ind) const;
    CInterfaceStackObject *getMapItemAtIndex(size_t ind, std::string &stringKey, double &numberKey,
                                             long long int &integerKey, bool &boolKey, int &keyType) const;
    void removeArrayItemAtIndex(size_t ind);
    void getAllObjectsAndClearTable(std::vector<CInterfaceStackObject *> &allObjs);

    void setUCharArray(const unsigned char *array, size_t l);
    void setInt32Array(const int *array, size_t l);
    void setInt64Array(const long long int *array, size_t l);
    void setFloatArray(const float *array, size_t l);
    void setDoubleArray(const double *array, size_t l);

    void appendArrayObject_null();
    void appendArrayObject_bool(bool value);
    void appendArrayObject_float(float value);
    void appendArrayObject_double(double value);
    void appendArrayObject_int32(int value);
    void appendArrayObject_int64(long long int value);
    void appendArrayObject_buffer(const char *value, size_t l, bool cborCoded = false);
    void appendArrayObject_binaryString(const char *value, size_t l, bool cborCoded = false);
    void appendArrayObject_text(const char *txtStr);
    void appendArrayObject_int32Array(const int *arr, size_t l);
    void appendArrayObject_int64Array(const long long int *arr, size_t l);
    void appendArrayObject_floatArray(const float *arr, size_t l);
    void appendArrayObject_doubleArray(const double *arr, size_t l);
    void appendArrayObject(CInterfaceStackObject *obj);
    void insertArrayObject(CInterfaceStackObject *obj, size_t pos);

    void appendMapObject_null(const char *key);
    void appendMapObject_bool(const char *key, bool value);
    void appendMapObject_float(const char *key, float value);
    void appendMapObject_double(const char *key, double value);
    void appendMapObject_int32(const char *key, int value);
    void appendMapObject_int64(const char *key, long long int value);
    void appendMapObject_buffer(const char *key, const char *value, size_t l, bool cborCoded = false);
    void appendMapObject_binaryString(const char *key, const char *value, size_t l, bool cborCoded = false);
    void appendMapObject_text(const char *key, const char *txtStr);
    void appendMapObject_int32Array(const char *key, const int *arr, size_t l);
    void appendMapObject_int64Array(const char *key, const long long int *arr, size_t l);
    void appendMapObject_floatArray(const char *key, const float *arr, size_t l);
    void appendMapObject_doubleArray(const char *key, const double *arr, size_t l);
    void appendMapObject_object(const char *key, CInterfaceStackObject *obj);
    void appendMapObject_object(long long int key, CInterfaceStackObject *obj);
    void appendMapObject_object(double key, CInterfaceStackObject *obj);
    void appendMapObject_object(bool key, CInterfaceStackObject *obj);
    void appendArrayOrMapObject(CInterfaceStackObject *key, CInterfaceStackObject *obj);

    bool getUCharArray(unsigned char *array, int count) const;
    bool getInt32Array(int *array, int count) const;
    bool getInt64Array(long long int *array, int count) const;
    bool getFloatArray(float *array, int count) const;
    bool getDoubleArray(double *array, int count) const;
    CInterfaceStackObject *getMapObject(const char *fieldName) const;
    bool containsKey(const char *fieldName) const;

    bool removeFromKey(const char *keyToRemove);
    bool removeFromKey(const CInterfaceStackObject *keyToRemove);

    int getTableInfo(int infoType) const;

  protected:
    bool _areAllValueThis(int what, bool integerAndDoubleTolerant) const;

    std::vector<CInterfaceStackObject *> _tableObjects;
    bool _isTableArray;
    bool _isCircularRef;
};
