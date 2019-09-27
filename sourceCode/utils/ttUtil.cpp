
#include "vrepMainHeader.h"
#include "ttUtil.h"
#include <stdarg.h>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include "base64.h"
#include "mathDefines.h"
#include "vDateTime.h"

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
    line="";
    while ( (multiline.length()!=0)&&((multiline[0]=='\n')||(multiline[0]=='\r')) )
        multiline.erase(multiline.begin());
    while ( (multiline.length()!=0)&&((multiline[multiline.length()-1]=='\n')||(multiline[multiline.length()-1]=='\r')) )
        multiline.erase(multiline.end()-1);
    size_t p1=multiline.find('\n');
    size_t p2=multiline.find('\r');
    size_t p3=std::string::npos;
    if (p1!=std::string::npos)
        p3=p1;
    if (p2!=std::string::npos)
    {
        if (p3!=std::string::npos)
        {
            if (p2<p3)
                p3=p2;
        }
        else
            p3=p2;
    }
    if (p3!=std::string::npos)
    {
        line.assign(multiline.begin(),multiline.begin()+p3);
        multiline.erase(multiline.begin(),multiline.begin()+p3);
        while ( (multiline.length()!=0)&&((multiline[0]=='\n')||(multiline[0]=='\r')) )
            multiline.erase(multiline.begin());
    }
    else
    {
        line=multiline;
        multiline="";
    }
    return(line.length()!=0);
}

std::string CTTUtil::intToString(int intVal)
{
    return(boost::lexical_cast<std::string>(intVal));
}

std::string CTTUtil::dwordToString(unsigned int dwordVal)
{
    return(boost::lexical_cast<std::string>(dwordVal));
}

std::string CTTUtil::dataToString(char* data,int startPos,int length)
{
    std::string retVal("");
    for (int i=0;i<length;i++)
        retVal+=data[startPos+i];
    return(retVal);
}

void CTTUtil::stringToData(const std::string& str,char* data,int startPos)
{
    for (int i=0;i<int(str.length());i++)
        data[startPos+i]=str[i];
}

std::string CTTUtil::getAdjustedString(const std::string& str,int length)
{
    std::string retString(str);
    while (int(retString.length())<length)
        retString+=' ';
    while (int(retString.length())>length)
        retString.erase(retString.end()-1);
    return(retString);
}

std::string CTTUtil::getLightEncodedString(const std::string& ss)
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

std::string CTTUtil::getLightDecodedString(const std::string& ss)
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

int CTTUtil::replaceWordInLine(std::string& line,const std::string& oldText,const std::string& newText)
{
    int retVal=0;
    size_t p=0;
    p=line.find(oldText,p);
    while (p!=std::string::npos)
    {
        line.erase(line.begin()+p,line.begin()+p+oldText.length());
        line.insert(line.begin()+p,newText.begin(),newText.end());
        p=line.find(oldText,p+1);
        retVal++;
    }
    return(retVal);
}

std::string CTTUtil::formatString(const std::string fmt, ...)
{ // taken from http://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
    // Courtesy of Erik Aronesty
    int size = 512;
    std::string str;
    va_list ap;
    while (1)
    {
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.c_str(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {
            str.resize(n);
            return str;
        }
        if (n > -1)
            size = n + 1;
        else
            size *= 2;
    }
    return str;
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
    float m=SIM_MAX(SIM_MAX(r,g),b);
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
        a[i]=(unsigned char)(SIM_RAND_FLOAT*255.1f);
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
        unsigned char nb=(unsigned char)(SIM_RAND_FLOAT*255.1f);
        snprintf(num,3,"%x",nb);
        if (strlen(num)==1)
            str+=std::string("0")+num;
        else
            str+=num;
    }
    std::transform(str.begin(),str.end(),str.begin(),::toupper);
    return(str);
}
