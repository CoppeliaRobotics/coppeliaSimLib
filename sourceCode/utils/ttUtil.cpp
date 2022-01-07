#include "ttUtil.h"
#include <stdarg.h>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include "base64.h"
#include "vDateTime.h"
#include <regex>

void CTTUtil::lightBinaryEncode(char* data,int length)
{ // Very simple!
    for (int i=0;i<length;i++)
        data[i]=(((data[i]&0xf0)>>4)|((data[i]&0x0f)<<4))^(i&0xff);
}

void CTTUtil::lightBinaryDecode(char* data,int length)
{ // very simple!
    for (int i=0;i<length;i++)
    {
        char tmp=data[i]^(i&0xff);
        data[i]=(((tmp&0xf0)>>4)|((tmp&0x0f)<<4));
    }
}

unsigned short CTTUtil::getCRC(char* data,int length)
{
    unsigned short crc=0;
    int p=0;
    for (int i=0;i<length;i++)
    {
        crc=crc^(((unsigned short)data[p])<<8);
        for (int j=0;j<8;j++)
        {
            if (crc&((unsigned short)0x8000))
                crc=(crc<<1)^((unsigned short)0x1021);
            else
                crc<<=1;
        }
        p++;
    }
    return(crc);
}

unsigned short CTTUtil::getCRC(const std::string& data)
{
    if (data.length()==0)
        return(0);
    return(getCRC((char*)(&data[0]),int(data.length())));
}

bool CTTUtil::extractCommaSeparatedWord(std::string& line,std::string& word)
{ // Returns true if a word could be extracted
    word="";
    while ( (line.length()!=0)&&(line[0]!=',') )
    {
        word.append(line.begin(),line.begin()+1);
        line.erase(line.begin());
    }
    if ( (line.length()!=0)&&(line[0]==',') )
        line.erase(line.begin());
    return(word.length()!=0);
}

bool CTTUtil::extractSpaceSeparatedWord(std::string& line,std::string& word)
{ // Returns true if a word could be extracted
    word="";
    while ( (line.length()!=0)&&(line[0]!=' ') )
    {
        word.append(line.begin(),line.begin()+1);
        line.erase(line.begin());
    }
    if ( (line.length()!=0)&&(line[0]==' ') )
        line.erase(line.begin());
    return(word.length()!=0);
}

bool CTTUtil::extractLine(std::string& multiline,std::string& line)
{ // Returns true if a line could be extracted
    size_t n=multiline.find('\n');
    if (n!=std::string::npos)
    {
        line.assign(multiline.begin(),multiline.begin()+n);
        multiline.erase(multiline.begin(),multiline.begin()+n+1);
        line.erase(std::remove(line.begin(),line.end(),'\r'),line.end());
        return(true);
    }
    else
    {
        line=multiline;
        multiline.clear();
        line.erase(std::remove(line.begin(),line.end(),'\r'),line.end());
        return(line.length()!=0);
    }
}

std::string CTTUtil::getLightEncodedString(const char* ss)
{ // ss can contain any char, also 0!
    std::string txt(ss);
    std::string s;
    for (int i=0;i<int(txt.length());i++)
    {
        int n[2];
        n[0]=(unsigned char)(txt[i])+i;
        n[1]=n[0]/16;
        n[0]-=n[1]*16;
        for (int j=0;j<2;j++)
        {
            int t=n[1-j];
            if (t>9)
                s+='A'+(t-10);
            else
                s+='0'+t;
        }
    }
    return(s);
}

std::string CTTUtil::getLightDecodedString(const char* ss)
{ // return string can contain any char, also 0!
    std::string txt(ss);
    std::string s;
    for (int i=0;i<int(txt.length())/2;i++)
    {
        int v=0;
        for (int j=0;j<2;j++)
        {
            int w;
            char a=txt[2*i+j];
            if (a>='A')
                w=10+(a-'A');
            else
                w=a-'0';
            if (j==0)
                v+=w*16;
            else
                v+=w;
        }       
        v-=i;
        s+=char(v);
    }
    return(s);
}

bool CTTUtil::removeSpacesAtBeginningAndEnd(std::string& line)
{
    while ((line.length()!=0)&&(line[0]==' '))
        line.erase(line.begin());
    while ((line.length()!=0)&&(line[line.length()-1]==' '))
        line.erase(line.begin()+line.length()-1);
    return(line.length()!=0);
}

std::string CTTUtil::getFormattedString(const char* a,const char* b,const char* c,const char* d,const char* e,const char* f,const char* g,const char* h)
{
    std::string retVal("");
    if (a==nullptr)
        return(retVal);
    retVal+=a;
    if (b==nullptr)
        return(retVal);
    retVal+=b;
    if (c==nullptr)
        return(retVal);
    retVal+=c;
    if (d==nullptr)
        return(retVal);
    retVal+=d;
    if (e==nullptr)
        return(retVal);
    retVal+=e;
    if (f==nullptr)
        return(retVal);
    retVal+=f;
    if (g==nullptr)
        return(retVal);
    retVal+=g;
    if (h==nullptr)
        return(retVal);
    retVal+=h;
    return(retVal);
}

std::string CTTUtil::getLowerCaseString(const char* str)
{
    std::string retValue(str);
    for (int i=0;i<int(retValue.length());i++)
    {
        if ((retValue[i]>=65)&&(retValue[i]<=90))
            retValue[i]+=32;
    }
    return(retValue);
}

void CTTUtil::scaleLightDown_(float& r,float& g,float& b)
{ // this is for backward compatibility (29/8/2013). Default lights are now less intense, and more similar to other applications
    r*=0.675f;
    g*=0.675f;
    b*=0.675f;
}

void CTTUtil::scaleColorUp_(float& r,float& g,float& b)
{ // this is for backward compatibility (29/8/2013). Default lights are now less intense, and more similar to other applications
    float f=1.5f;
    float m=std::max<float>(std::max<float>(r,g),b);
    float d=1.0f/m;
    if (d<f)
        f=d;
    r*=f;
    g*=f;
    b*=f;
}

void CTTUtil::scaleLightDown_(float* rgb)
{
    scaleLightDown_(rgb[0],rgb[1],rgb[2]);
}

void CTTUtil::scaleColorUp_(float* rgb)
{
    scaleColorUp_(rgb[0],rgb[1],rgb[2]);
}

std::string CTTUtil::decode64(const std::string &data)
{
    return(base64_decode(data));
}

std::string CTTUtil::encode64(const std::string &data)
{
    return(base64_encode((const unsigned char*)data.c_str(),(unsigned int)data.size()));
}

std::string CTTUtil::generateUniqueString()
{
    std::string s("1234567890123456");
    char a[17];
    for (size_t i=0;i<16;i++)
        a[i]=(unsigned char)(((static_cast<float>(rand())/static_cast<float>(RAND_MAX)))*255.1f);
    char b[17];
    sprintf(b,"%i",VDateTime::getTimeInMs());
    char c[17];
    sprintf(c,"%i",int(VDateTime::getSecondsSince1970()));
    for (size_t i=0;i<16;i++)
        s[i]+=a[i]+b[i]+c[i];
    return(s);
}

std::string CTTUtil::generateUniqueReadableString()
{
    static bool seeded=false;
    if (!seeded)
    {
        srand((unsigned int)VDateTime::getTimeInMs()+(unsigned int)VDateTime::getSecondsSince1970());
        seeded=true;
    }
    std::string str;
    char num[4];
    for (size_t i=0;i<8;i++)
    {
        unsigned char nb=(unsigned char)(((static_cast<float>(rand())/static_cast<float>(RAND_MAX)))*255.1f);
        snprintf(num,3,"%x",nb);
        if (strlen(num)==1)
            str+=std::string("0")+num;
        else
            str+=num;
    }
    std::transform(str.begin(),str.end(),str.begin(),::toupper);
    return(str);
}

void CTTUtil::replaceSubstring(std::string& str,const char* subStr,const char* replacementSubStr)
{
    size_t index=0;
    size_t str1L=strlen(subStr);
    size_t str2L=strlen(replacementSubStr);
    while (true)
    {
        index=str.find(subStr,index);
        if (index==std::string::npos)
            break;
        str.replace(index,str1L,replacementSubStr);
        index+=str2L;
    }
}

void CTTUtil::regexReplace(std::string& str,const char* regexStr,const char* regexReplacementSubStr)
{
    str=std::regex_replace(str,std::regex(regexStr),regexReplacementSubStr);
}

void CTTUtil::pushFloatToBuffer(float d,std::vector<char>& data)
{
    for (size_t i=0;i<sizeof(float);i++)
        data.push_back(((char*)&d)[i]);
}

float CTTUtil::popFloatFromBuffer(std::vector<char>& data)
{
    float d;
    for (size_t i=0;i<sizeof(float);i++)
    {
        ((char*)&d)[sizeof(float)-1-i]=data[data.size()-1];
        data.pop_back();
    }
    return(d);
}

void CTTUtil::pushIntToBuffer(int d,std::vector<char>& data)
{
    for (size_t i=0;i<sizeof(int);i++)
        data.push_back(((char*)&d)[i]);
}

int CTTUtil::popIntFromBuffer(std::vector<char>& data)
{
    int d;
    for (size_t i=0;i<sizeof(int);i++)
    {
        ((char*)&d)[sizeof(int)-1-i]=data[data.size()-1];
        data.pop_back();
    }
    return(d);
}

bool CTTUtil::doStringMatch_wildcard(const char* wildcardStr,const char* otherStr)
{
    if ( (wildcardStr[0]=='\0')&&(otherStr[0]=='\0') )
        return(true);
    if ( (wildcardStr[0]=='*')&&((wildcardStr+1)[0]!='\0')&&(otherStr[0]=='\0') )
        return(false);
    if ( (wildcardStr[0]=='?')||(wildcardStr[0]==otherStr[0]) )
        return(doStringMatch_wildcard(wildcardStr+1,otherStr+1));
    if (wildcardStr[0]=='*')
        return(doStringMatch_wildcard(wildcardStr+1,otherStr)||doStringMatch_wildcard(wildcardStr,otherStr+1));
    return(false);
}
