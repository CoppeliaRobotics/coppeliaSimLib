#pragma once

#include <vector>
#include <string>

class CCbor
{
public:
    CCbor(const std::string* initBuff,int options);
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
    void appendLuaString(const std::string& v);
    void appendArray(size_t cnt);
    void appendMap(size_t cnt);
    void appendBreakIfApplicable();

    std::string getBuff() const;

protected:
    void _appendItemTypeAndLength(unsigned char t,long long int l);

    std::vector<unsigned char> _buff;
    int _options; // bit0: do not code doubles as float, bit1: do not use indefinite-length arrays and maps
};
