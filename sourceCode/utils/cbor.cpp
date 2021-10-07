#include "cbor.h"

bool CCbor::isText(const char* v,size_t l)
{
    for (size_t i=0;i<l;i++)
    {
        if (v[i]>=127)
            return(false);
        if ( (v[i]<=31)&&(v[i]!=9)&&(v[i]!=13) )
            return(false);
    }
   return true;
}

CCbor::CCbor()
{
}

CCbor::~CCbor()
{
}

void CCbor::appendInt(long long int v)
{
    unsigned char add=0;
    if (v<0)
    {
        v=-v;
        v=v-1;
        add=32;
    }
    _appendItemTypeAndLength(add,v);
}

void CCbor::appendDouble(double v)
{
    _buff.push_back(128+64+32+27);
    _buff.push_back(((unsigned char*)&v)[7]);
    _buff.push_back(((unsigned char*)&v)[6]);
    _buff.push_back(((unsigned char*)&v)[5]);
    _buff.push_back(((unsigned char*)&v)[4]);
    _buff.push_back(((unsigned char*)&v)[3]);
    _buff.push_back(((unsigned char*)&v)[2]);
    _buff.push_back(((unsigned char*)&v)[1]);
    _buff.push_back(((unsigned char*)&v)[0]);
/*
    unsigned char x[12]={((unsigned char*)&v)[7],((unsigned char*)&v)[6],((unsigned char*)&v)[5],((unsigned char*)&v)[4],((unsigned char*)&v)[3],((unsigned char*)&v)[2],((unsigned char*)&v)[1],((unsigned char*)&v)[0],0,0,0,0};
    if (((long long unsigned int*)(x+2))[0]==0)
    {
        _buff.push_back(128+64+32+25);
        _buff.push_back(((unsigned char*)&v)[7]);
        _buff.push_back(((unsigned char*)&v)[6]);
    }
    else if (((long long unsigned int*)(x+4))[0]==0)
    {
        float w=(float)v;
        _buff.push_back(128+64+32+26);
        _buff.push_back(((unsigned char*)&w)[3]);
        _buff.push_back(((unsigned char*)&w)[2]);
        _buff.push_back(((unsigned char*)&w)[1]);
        _buff.push_back(((unsigned char*)&w)[0]);
    }
    else
    {
        _buff.push_back(128+64+32+27);
        _buff.push_back(((unsigned char*)&v)[7]);
        _buff.push_back(((unsigned char*)&v)[6]);
        _buff.push_back(((unsigned char*)&v)[5]);
        _buff.push_back(((unsigned char*)&v)[4]);
        _buff.push_back(((unsigned char*)&v)[3]);
        _buff.push_back(((unsigned char*)&v)[2]);
        _buff.push_back(((unsigned char*)&v)[1]);
        _buff.push_back(((unsigned char*)&v)[0]);
    }
    */
}

void CCbor::appendNull()
{
    _buff.push_back(128+64+32+22);
}

void CCbor::appendBool(bool v)
{
    if (v)
        _buff.push_back(128+64+32+21);
    else
        _buff.push_back(128+64+32+20);
}

void CCbor::_appendItemTypeAndLength(unsigned char t,long long int l)
{
    if (l<24)
        _buff.push_back(t+(unsigned char)l);
    else if (l<=0xff)
    {
        _buff.push_back(t+24);
        _buff.push_back((unsigned char)l);
    }
    else if (l<=0xffff)
    {
        _buff.push_back(t+25);
        _buff.push_back(((unsigned char*)&l)[1]);
        _buff.push_back(((unsigned char*)&l)[0]);
    }
    else if (l<=0xffffffff)
    {
        _buff.push_back(t+26);
        _buff.push_back(((unsigned char*)&l)[3]);
        _buff.push_back(((unsigned char*)&l)[2]);
        _buff.push_back(((unsigned char*)&l)[1]);
        _buff.push_back(((unsigned char*)&l)[0]);
    }
    else
    {
        _buff.push_back(t+27);
        _buff.push_back(((unsigned char*)&l)[7]);
        _buff.push_back(((unsigned char*)&l)[6]);
        _buff.push_back(((unsigned char*)&l)[5]);
        _buff.push_back(((unsigned char*)&l)[4]);
        _buff.push_back(((unsigned char*)&l)[3]);
        _buff.push_back(((unsigned char*)&l)[2]);
        _buff.push_back(((unsigned char*)&l)[1]);
        _buff.push_back(((unsigned char*)&l)[0]);
    }
}

void CCbor::appendBuff(const unsigned char* v,size_t l)
{
    _appendItemTypeAndLength(64,l);
    for (size_t i=0;i<l;i++)
        _buff.push_back(v[i]);
}

void CCbor::appendString(const char* v,size_t l)
{
    _appendItemTypeAndLength(64+32,l);
    for (size_t i=0;i<l;i++)
        _buff.push_back(v[i]);
}

void CCbor::appendLuaString(const std::string& v)
{
    if (v.find("@:txt:",v.size()-6-1)!=std::string::npos)
        appendString(v.c_str(),v.size()-6);
    else if (v.find("@:dat:",v.size()-6-1)!=std::string::npos)
        appendBuff((unsigned char*)v.c_str(),v.size()-6);
    else
    {
        if (isText(v.c_str(),v.size()))
            appendString(v.c_str(),v.size());
        else
            appendBuff((unsigned char*)v.c_str(),v.size());
    }
}

void CCbor::appendArray(size_t cnt)
{
    _appendItemTypeAndLength(128,cnt);

}

void CCbor::appendMap(size_t cnt)
{
    _appendItemTypeAndLength(128+32,cnt);

}

std::string CCbor::getBuff() const
{
    std::string retVal;
    retVal.assign(_buff.begin(),_buff.end());
    return(retVal);
}
