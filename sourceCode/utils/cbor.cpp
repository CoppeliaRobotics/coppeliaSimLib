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

CCbor::CCbor(const std::string* initBuff,int options)
{
    _options=options;
    if (initBuff!=nullptr)
        _buff.assign(initBuff->begin(),initBuff->end());
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

void CCbor::appendIntArray(const int* v,size_t cnt)
{
    appendArray(cnt);

    unsigned char* w=(unsigned char*)v;
    for (size_t i=0;i<cnt;i++)
    {
        if (v[i]<0)
        {
            int x=-v[i]-1;
            _buff.push_back(26+32);
            unsigned char* y=(unsigned char*)&x;
            _buff.push_back(y[3]);
            _buff.push_back(y[2]);
            _buff.push_back(y[1]);
            _buff.push_back(y[0]);
        }
        else
        {
            _buff.push_back(26);
            _buff.push_back(w[3]);
            _buff.push_back(w[2]);
            _buff.push_back(w[1]);
            _buff.push_back(w[0]);
        }
        w+=4;
    }

    appendBreakIfApplicable();
}

void CCbor::appendIntArray(const long long int* v,size_t cnt)
{
    appendArray(cnt);

    unsigned char* w=(unsigned char*)v;
    for (size_t i=0;i<cnt;i++)
    {
        if (v[i]<0)
        {
            long long int x=-v[i]-1;
            _buff.push_back(27+32);
            unsigned char* y=(unsigned char*)&x;
            _buff.push_back(y[7]);
            _buff.push_back(y[6]);
            _buff.push_back(y[5]);
            _buff.push_back(y[4]);
            _buff.push_back(y[3]);
            _buff.push_back(y[2]);
            _buff.push_back(y[1]);
            _buff.push_back(y[0]);
        }
        else
        {
            _buff.push_back(27);
            _buff.push_back(w[7]);
            _buff.push_back(w[6]);
            _buff.push_back(w[5]);
            _buff.push_back(w[4]);
            _buff.push_back(w[3]);
            _buff.push_back(w[2]);
            _buff.push_back(w[1]);
            _buff.push_back(w[0]);
        }
        w+=8;
    }

    appendBreakIfApplicable();
}

void CCbor::appendFloat(float v)
{
    _buff.push_back(128+64+32+26);
    _buff.push_back(((unsigned char*)&v)[3]);
    _buff.push_back(((unsigned char*)&v)[2]);
    _buff.push_back(((unsigned char*)&v)[1]);
    _buff.push_back(((unsigned char*)&v)[0]);
}

void CCbor::appendFloatArray(const float* v,size_t cnt)
{
    appendArray(cnt);

    const unsigned char* w=(const unsigned char*)v;
    for (size_t i=0;i<cnt;i++)
    {
        _buff.push_back(128+64+32+26);
        _buff.push_back(w[3]);
        _buff.push_back(w[2]);
        _buff.push_back(w[1]);
        _buff.push_back(w[0]);
        w+=4;
    }

    appendBreakIfApplicable();
}

void CCbor::appendDouble(double v)
{
    if ((_options&1)==0)
        appendFloat(float(v)); // treat doubles as floats
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
}

void CCbor::appendDoubleArray(const double* v,size_t cnt)
{
    appendArray(cnt);

    if ((_options&1)==0)
    { // treat doubles as floats
        for (size_t i=0;i<cnt;i++)
        {
            float ww=float(v[i]);
            const unsigned char* w=(const unsigned char*)&ww;
            _buff.push_back(128+64+32+26);
            _buff.push_back(w[3]);
            _buff.push_back(w[2]);
            _buff.push_back(w[1]);
            _buff.push_back(w[0]);
        }
    }
    else
    {
        const unsigned char* w=(const unsigned char*)v;
        for (size_t i=0;i<cnt;i++)
        {
            _buff.push_back(128+64+32+27);
            _buff.push_back(w[7]);
            _buff.push_back(w[6]);
            _buff.push_back(w[5]);
            _buff.push_back(w[4]);
            _buff.push_back(w[3]);
            _buff.push_back(w[2]);
            _buff.push_back(w[1]);
            _buff.push_back(w[0]);
            w+=8;
        }
    }

    appendBreakIfApplicable();
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

void CCbor::appendString(const char* v,int l/*=-1*/)
{
    if (l<0)
        l=strlen(v);
    _appendItemTypeAndLength(64+32,size_t(l));
    for (size_t i=0;i<size_t(l);i++)
        _buff.push_back(v[i]);
}

void CCbor::appendRaw(const unsigned char* v,size_t l)
{
    _buff.insert(_buff.end(),v,v+l);
}

void CCbor::appendLuaString(const std::string& v)
{
    if (v.find("@:txt:",v.size()-6-1)!=std::string::npos)
        appendString(v.c_str(),int(v.size())-6);
    else if (v.find("@:dat:",v.size()-6-1)!=std::string::npos)
        appendBuff((unsigned char*)v.c_str(),v.size()-6);
    else
    {
        if (isText(v.c_str(),int(v.size())))
            appendString(v.c_str(),v.size());
        else
            appendBuff((unsigned char*)v.c_str(),v.size());
    }
}

void CCbor::appendArray(size_t cnt)
{
    if ((_options&2)==0)
        _buff.push_back(128+31); // use a break char
    else
        _appendItemTypeAndLength(128,cnt);
}

void CCbor::appendMap(size_t cnt)
{
    if ((_options&2)==0)
        _buff.push_back(128+32+31); // use a break char
    else
        _appendItemTypeAndLength(128+32,cnt);
}

void CCbor::appendBreakIfApplicable()
{
    if ((_options&2)==0)
        _buff.push_back(255); // break char
}

void CCbor::clear()
{
    _buff.clear();
}

std::string CCbor::getBuff() const
{
    std::string retVal;
    retVal.assign(_buff.begin(),_buff.end());
    return(retVal);
}

const std::vector<unsigned char>* CCbor::getBuffPtr() const
{
    return(&_buff);
}

