#pragma once

#include <vector>
#include <string>
#include <map>
#include <set>
#include <simLib/simTypes.h>
#include <simMath/3Vector.h>
#include <simMath/4Vector.h>
#include <simMath/7Vector.h>
#include <simMath/mXnMatrix.h>
#include <simMath/3X3Matrix.h>

struct SEventInf
{
    size_t pos;
    std::string eventId;
    int64_t target;
    size_t size;
    std::vector<size_t> fieldPositions;
    std::vector<std::string> fieldNames;
    std::vector<size_t> fieldSizes;
};

class CCbor
{
  public:
    CCbor(const std::string* initBuff = nullptr, int options = 0);
    virtual ~CCbor();

    void swapWithEmptyBuffer(std::vector<unsigned char>* emptyBuff);
    static bool isText(const char* v, size_t l);

    void appendInt64(int64_t v);
    void appendHandle(int64_t h);
    void appendUint8Array(const unsigned char* v, size_t cnt);
    void appendInt32Array(const int* v, size_t cnt);
    void appendUint32Array(const unsigned int* v, size_t cnt);
    void appendInt64Array(const int64_t* v, size_t cnt);
    void appendHandleArray(const int64_t* h, size_t cnt);
    void appendHandleArray(const int* h, size_t cnt);
    void appendFloat(float v);
    void appendFloatArray(const float* v, size_t cnt);
    void appendDouble(double v);
    void appendDoubleArray(const double* v, size_t cnt);
    void appendMatrix(const float* v, size_t rows, size_t cols);
    void appendMatrix(const double* v, size_t rows, size_t cols);
    void appendMatrix(const C3X3Matrix& m);
    void appendMatrix(const C4X4Matrix& m);
    void appendMatrix(const CMatrix& m);
    void appendVector2(const double* v);
    void appendVector3(const double* v);
    void appendVector3(const C3Vector& v);
    void appendQuaternion(const double* v, bool xyzwLayout = false);
    void appendQuaternion(const CQuaternion& q);
    void appendPose(const double* v, bool xyzqxqyqzqwLayout = false);
    void appendPose(const CPose& p);
    void appendColor3(const float c[3]);
    void appendColor(const float c[4]);
    void appendNull();
    void appendBool(bool v);
    void appendBuff(const unsigned char* v, size_t l);
    void appendText(const char* v, int l = -1);
    void appendTextArray(const std::vector<std::string>& txtArr);

    void appendKeyInt64(const char* key, int64_t v);
    void appendKeyHandle(const char* key, int64_t h);
    void appendKeyUint8Array(const char* key, const unsigned char* v, size_t cnt);
    void appendKeyInt32Array(const char* key, const int* v, size_t cnt);
    void appendKeyUint32Array(const char* key, const unsigned int* v, size_t cnt);
    void appendKeyInt64Array(const char* key, const int64_t* v, size_t cnt);
    void appendKeyHandleArray(const char* key, const int64_t* h, size_t cnt);
    void appendKeyHandleArray(const char* key, const int* h, size_t cnt);
    void appendKeyFloat(const char* key, float v);
    void appendKeyFloatArray(const char* key, const float* v, size_t cnt);
    void appendKeyDouble(const char* key, double v);
    void appendKeyDoubleArray(const char* key, const double* v, size_t cnt);
    void appendKeyNull(const char* key);
    void appendKeyBool(const char* key, bool v);
    void appendKeyMatrix(const char* key, const float* v, size_t rows, size_t cols);
    void appendKeyMatrix(const char* key, const double* v, size_t rows, size_t cols);
    void appendKeyMatrix(const char* key, const CMatrix& m);
    void appendKeyVector2(const char* key, const double* v);
    void appendKeyVector3(const char* key, const double* v);
    void appendKeyVector3(const char* key, const C3Vector& v);
    void appendKeyQuaternion(const char* key, const double* v, bool xyzwLayout = false);
    void appendKeyQuaternion(const char* key, const CQuaternion& q);
    void appendKeyPose(const char* key, const double* v, bool xyzqxqyqzqwLayout = false);
    void appendKeyPose(const char* key, const CPose& p);
    void appendKeyColor3(const char* key, const float* c);
    void appendKeyColor(const char* key, const float* c);
    void appendKeyBuff(const char* key, const unsigned char* v, size_t l);
    void appendKeyText(const char* key, const char* v, int l = -1);
    void appendKeyTextArray(const char* key, const std::vector<std::string>& txtArr);

    void openArray(int sizedArray = -1);
    void openKeyArray(const char* key, int sizedArray = -1);
    void openMap(int sizedMap = -1);
    void openKeyMap(const char* key, int sizedMap = -1);
    void closeArrayOrMap();

    void appendLuaString(const std::string& v, bool isBuffer, bool isText);
    void appendRaw(const unsigned char* v, size_t l);

    void createEvent(const char* event, const char* fieldName, const char* objType, int64_t handle, int64_t uid, bool mergeable, bool openDataField = true);
    void pushEvent();
    int64_t finalizeEvents(int64_t nextSeq, bool seqChanges, std::vector<SEventInf>* inf = nullptr);
    size_t getEventCnt() const;
    size_t getEventDepth() const;

    void clear();

    std::string getBuff() const;
    const unsigned char* getBuff(size_t& l) const;

    static std::set<std::string> allEVentFieldNames;

  protected:
    void _handleDataField(const char* key = nullptr);
    void _appendItemTypeAndLength(unsigned char t, int64_t l);
    void _adjustEventSeq(size_t pos, int64_t endSeq);

    std::vector<unsigned char> _buff;
    int _options; // bit0: treat doubles as float

    size_t _eventDepth; // nb of array/map closes needed
    bool _eventOpen;    // true when not yet pushed
    bool _nextIsKeyInData;
    bool _inDataField;
    int _handleDataFieldDisableLevel;
    size_t _discardableEventCnt;
    std::vector<SEventInf> _eventInfos;
    std::map<std::string, size_t> _mergeableEventIds;
    std::vector<bool> _sizedArrayInfo;
};
