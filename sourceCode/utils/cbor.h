#pragma once

#include <vector>
#include <string>
#include <simLib/simTypes.h>

class CCbor
{
public:
    CCbor(const std::string* initBuff,int options=0);
    virtual ~CCbor();

    bool isText(const char* v,size_t l);

    void appendInt(long long int v);
    void appendIntArray(const int* v,size_t cnt);
    void appendIntArray(const long long int* v,size_t cnt);
    void appendFloat(float v);
    void appendFloatArray(const float* v,size_t cnt);
    void appendDouble(double v);
    void appendDoubleArray(const double* v,size_t cnt);
    void appendNull();
    void appendBool(bool v);
    void appendBuff(const unsigned char* v,size_t l);
    void appendString(const char* v,int l=-1);

    void appendKeyInt(const char* key,long long int v);
    void appendKeyIntArray(const char* key,const int* v,size_t cnt);
    void appendKeyIntArray(const char* key,const long long int* v,size_t cnt);
    void appendKeyFloat(const char* key,float v);
    void appendKeyFloatArray(const char* key,const float* v,size_t cnt);
    void appendKeyDouble(const char* key,double v);
    void appendKeyDoubleArray(const char* key,const double* v,size_t cnt);
    void appendKeyNull(const char* key);
    void appendKeyBool(const char* key,bool v);
    void appendKeyBuff(const char* key,const unsigned char* v,size_t l);
    void appendKeyString(const char* key,const char* v,int l=-1);

    void openArray();
    void openMap();
    void closeArrayOrMap();

    void appendLuaString(const std::string& v);
    void appendRaw(const unsigned char* v,size_t l);

    void eventBegin(const std::string& eventFootprint,bool mergeable);
    void eventEnd();
    size_t getEventCnt() const;
    void adjustEventSeqs(long long int endSeq);

    void clear();

    std::string getBuff() const;
    const unsigned char* getBuff(size_t& l) const;

protected:
    void _appendItemTypeAndLength(unsigned char t,long long int l);
    void _adjustEventSeq(size_t pos,long long int endSeq);

    std::vector<unsigned char> _buff;
    int _options; // bit0: treat doubles as float

    std::string _lastEventId;
    std::vector<size_t> _eventBeginPtrs;
};
