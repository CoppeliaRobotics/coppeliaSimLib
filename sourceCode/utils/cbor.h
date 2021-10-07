#pragma once

#include <vector>
#include <string>

class CCbor
{
public:
    CCbor();
    virtual ~CCbor();

    bool isText(const char* v,size_t l);

    void appendInt(long long int v);
    void appendDouble(double v);
    void appendNull();
    void appendBool(bool v);
    void appendBuff(const unsigned char* v,size_t l);
    void appendString(const char* v,size_t l);
    void appendLuaString(const std::string& v);
    void appendArray(size_t cnt);
    void appendMap(size_t cnt);

    std::string getBuff() const;

protected:
    void _appendItemTypeAndLength(unsigned char t,long long int l);

    std::vector<unsigned char> _buff;
};
