
#include "vrepMainHeader.h"
#include "tt.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "MyMath.h"
#ifndef SIM_WITHOUT_QT_AT_ALL
#include <QString>
#else
#include <algorithm>
#endif

struct filestruct_A
{
    bool operator<(const filestruct_A& rhs) const
    {
        return valA < rhs.valA;
    }
    int valA;
    int valB;
};

struct filestruct_B
{
    bool operator<(const filestruct_B& rhs) const
    {
        return valA < rhs.valA;
    }
    float valA;
    int valB;
};

struct filestruct_C
{
    bool operator<(const filestruct_C& rhs) const
    {
        return valA < rhs.valA;
    }
    std::string valA;
    int valB;
};

bool tt::stringToFloat(const char* txt,float& f,bool allowNegativeValue,bool infGivesMinusOne)
{
#ifdef SIM_WITHOUT_QT_AT_ALL
    std::string str(txt);
    if (allowNegativeValue)
    {
        std::string tmpStr(str);
        while ((tmpStr.length()!=0)&&(tmpStr[0]==' '))
            tmpStr.erase(0,1);
        int l=3;
        if ((tmpStr.length()!=0)&&((tmpStr[0]=='+')||(tmpStr[0]=='-')))
            l=4;
        tmpStr.assign(tmpStr,0,l);
        std::transform(tmpStr.begin(),tmpStr.end(),tmpStr.begin(),::tolower);
        if (tmpStr.compare("inf")==0)
        {
            f=FLT_MAX;
            return(true);
        }
        if (tmpStr.compare("+inf")==0)
        {
            f=FLT_MAX;
            return(true);
        }
        if (tmpStr.compare("-inf")==0)
        {
            f=-FLT_MAX;//FLT_MIN;
            return(true);
        }
        return(getValidFloat(str,f));
    }
    else
    {
        std::string tmpStr(str);
        while ((tmpStr.length()!=0)&&(tmpStr[0]==' '))
            tmpStr.erase(0,1);
        int l=3;
        if ((tmpStr.length()!=0)&&((tmpStr[0]=='+')||(tmpStr[0]=='-')))
            l=4;
        tmpStr.assign(tmpStr,0,l);
        std::transform(tmpStr.begin(),tmpStr.end(),tmpStr.begin(),::tolower);
        if (tmpStr.compare("inf")==0)
        {
            if (infGivesMinusOne)
                f=-1.0f;
            else
                f=FLT_MAX;
            return(true);
        }
        if (tmpStr.compare("+inf")==0)
        {
            if (infGivesMinusOne)
                f=-1.0f;
            else
                f=FLT_MAX;
            return(true);
        }
        if (tmpStr.compare("-inf")==0)
        {
            f=0.0f;
            return(true);
        }
        bool ok=getValidFloat(str,f);
        if (ok&&(f<0.0))
            f=0.0;
        return(ok);
    }
#else
    QString str(txt);
    if (allowNegativeValue)
    {
        QString tmpStr(str);
        while ((tmpStr.length()!=0)&&(tmpStr[0]==' '))
            tmpStr.remove(0,1);
        int l=3;
        if ((tmpStr.length()!=0)&&((tmpStr[0]=='+')||(tmpStr[0]=='-')))
            l=4;
        tmpStr=tmpStr.left(l);
        if (tmpStr.compare("inf",Qt::CaseInsensitive)==0)
        {
            f=FLT_MAX;
            return(true);
        }
        if (tmpStr.compare("+inf",Qt::CaseInsensitive)==0)
        {
            f=FLT_MAX;
            return(true);
        }
        if (tmpStr.compare("-inf",Qt::CaseInsensitive)==0)
        {
            f=-FLT_MAX;//FLT_MIN;
            return(true);
        }

        bool ok;
        f=str.toFloat(&ok);
        return(ok);
    }
    else
    {
        QString tmpStr(str);
        while ((tmpStr.length()!=0)&&(tmpStr[0]==' '))
            tmpStr.remove(0,1);
        int l=3;
        if ((tmpStr.length()!=0)&&((tmpStr[0]=='+')||(tmpStr[0]=='-')))
            l=4;
        tmpStr=tmpStr.left(l);
        if (tmpStr.compare("inf",Qt::CaseInsensitive)==0)
        {
            if (infGivesMinusOne)
                f=-1.0f;
            else
                f=FLT_MAX;
            return(true);
        }
        if (tmpStr.compare("+inf",Qt::CaseInsensitive)==0)
        {
            if (infGivesMinusOne)
                f=-1.0f;
            else
                f=FLT_MAX;
            return(true);
        }
        if (tmpStr.compare("-inf",Qt::CaseInsensitive)==0)
        {
            f=0.0f;
            return(true);
        }

        bool ok;
        f=str.toFloat(&ok);
        if (ok&&(f<0.0f))
            f=0.0f;
        return(ok);
    }
#endif
}

std::string tt::getEString(bool sign,float f,int precision)
{
#ifdef SIM_WITHOUT_QT_AT_ALL
    std::stringstream s;
    s.precision(precision);
    s.setf(std::ios::scientific, std::ios::floatfield );
    s << f;
    std::string str(s.str());
    if (sign&&(f>=0.0f))
        str="+"+str;
    // Following very unelegant but temporary (to avoid an exponent of 3 width):
    int l=str.size();
    if ( ((str[l-4]=='-')||(str[l-4]=='+'))&&(str[l-3]=='0') )
        str.erase(str.begin()+l-3);
    return(str);
#else
    QString str(QString("%1").arg(f,0,'e',precision));
    if (sign&&(f>=0.0f))
        str="+"+str;
    return(str.toStdString());
#endif
}

std::string tt::getFString(bool sign,float f,int precision)
{
#ifdef SIM_WITHOUT_QT_AT_ALL
    std::stringstream s;
    s.precision(precision);
    s.setf(std::ios::fixed, std::ios::floatfield );
    s << f;
    std::string str(s.str());
    if (sign&&(f>=0.0f))
        str="+"+str;
    return(str);
#else
    QString str(QString("%1").arg(f,0,'f',precision));
    if (sign&&(f>=0.0f))
        str="+"+str;
    return(str.toStdString());
#endif
}

std::string tt::getDString(bool sign,double f,int precision)
{
#ifdef SIM_WITHOUT_QT_AT_ALL
    std::stringstream s;
    s.precision(precision);
    s.setf(std::ios::fixed, std::ios::floatfield );
    s << f;
    std::string str(s.str());
    if (sign&&(f>=0.0))
        str="+"+str;
    return(str);
#else
    QString str(QString("%1").arg(f,0,'f',precision));
    if (sign&&(f>=0.0))
        str="+"+str;
    return(str.toStdString());
#endif
}

std::string tt::getAngleEString(bool sign,float angleInRad,int precision)
{
    return(getEString(sign,angleInRad*radToDeg_f,precision));
}

std::string tt::getAngleFString(bool sign,float angleInRad,int precision)
{
    return(getFString(sign,angleInRad*radToDeg_f,precision));
}

std::string tt::getIString(bool sign,int v)
{
#ifdef SIM_WITHOUT_QT_AT_ALL
    std::stringstream s;
    s << v;
    std::string str(s.str());
    if (sign&&(v>=0))
        str="+"+str;
    return(str);
#else
    QString str(QString("%1").arg(v));
    if (sign&&v>=0)
        str="+"+str;
    return(str.toStdString());
#endif
}


std::string tt::floatToEInfString(float f,bool canBeNegative)
{
    if ((!canBeNegative)&&(f<0.0f))
        return("Infinity");
    if (f==FLT_MAX)
        return("Infinity");
    if (f==-FLT_MAX) //FLT_MIN)
        return("-Infinity");
#ifdef SIM_WITHOUT_QT_AT_ALL
    return(getEString(false,f,4));
#else
    QString str;
    str=QString("%1").arg(f,0,'e',4);
    return(str.toStdString());
#endif
}

bool tt::stringToInt(const char* txt,int& a)
{
#ifdef SIM_WITHOUT_QT_AT_ALL
    return(getValidInt(txt,a));
#else
    QString str(txt);
    bool ok;
    a=str.toInt(&ok);
    return(ok);
#endif
}

std::string tt::intToString(int a)
{
#ifdef SIM_WITHOUT_QT_AT_ALL
    return(getIString(false,a));
#else
    QString str;
    str=QString("%1").arg(a);
    return(str.toStdString());
#endif
}

float tt::floatToUserFloat(float f,float toUserConversion,bool minusValuesGiveInf)
{
    if (f<0.0f)
    {
        if ((!minusValuesGiveInf)&&(f!=-FLT_MAX)) //FLT_MIN))
            return(f*toUserConversion);
    }
    else
    {
        if (f!=FLT_MAX)
            return(f*toUserConversion);
    }
    return(f);
}

float tt::userFloatToFloat(float userFloat,float fromUserConversion,bool minusValuesGiveInf)
{
    if (userFloat<0.0f)
    {
        if ((!minusValuesGiveInf)&&(userFloat!=-FLT_MAX)) //FLT_MIN))
            return(userFloat*fromUserConversion);
    }
    else
    {
        if (userFloat!=FLT_MAX)
            return(userFloat*fromUserConversion);
    }
    return(userFloat);
}

std::string tt::decorateString(const char* prefix,const std::string mainText,const char* suffix)
{
    std::string retVal(prefix);
    retVal+=mainText;
    retVal+=suffix;
    return(retVal);
}

void tt::lightEncodeBuffer(char* buff,int length)
{ // Just use the first end last bytes for encoding key:
    if (length<3)
        return;
    int start=buff[0]/2;
    int end=buff[length-1]*3;
    for (int i=1;i<length-1;i++)
    {
        if (i&1)
            buff[i]-=i+start+end;
        if (i&2)
            buff[i]+=3*i+start;
        if (i&4)
            buff[i]-=i/2+end;
    }
}

void tt::lightDecodeBuffer(char* buff,int length)
{
    if (length<3)
        return;
    int start=buff[0]/2;
    int end=buff[length-1]*3;
    for (int i=1;i<length-1;i++)
    {
        if (i&1)
            buff[i]+=i+start+end;
        if (i&2)
            buff[i]-=3*i+start;
        if (i&4)
            buff[i]+=i/2+end;
    }
}

void tt::removeComments(std::string& line)
{
    for (int i=0;i<int(line.length())-1;i++)
    {
        if ( (line[i]=='/')&&(line[i+1]=='/') )
        {
            line.erase(line.begin()+i,line.end());
            return;
        }
    }
}

bool tt::removeSpacesAtBeginningAndEnd(std::string& line)
{
    while ((line.length()!=0)&&(line[0]==' '))
        line.erase(line.begin());
    while ((line.length()!=0)&&(line[line.length()-1]==' '))
        line.erase(line.begin()+line.length()-1);
    return(line.length()!=0);
}

bool tt::removeSpacesAndEmptyLinesAtBeginningAndEnd(std::string& line)
{
    while ((line.length()!=0)&&((line[0]==' ')||(line[0]==10)))
        line.erase(line.begin());
    while ((line.length()!=0)&&((line[line.length()-1]==' ')||(line[line.length()-1]==10)))
        line.erase(line.begin()+line.length()-1);
    return(line.length()!=0);
}

int tt::getDecimalPos(float number,int maxDec)
{
//  int retVal=0;
    if (number==0.0f)
        return(-1);
    for (int i=0;i<maxDec;i++)
    {
        if (fabs(number)>=1.0f)
        {
            number-=float(int(number));
            number*=10.0f;
            if ((fabs(number)>=1.0f)&&(i<maxDec) )
                return(i+1);
            return(i);
        }
        number*=10.0f;
    }
    return(maxDec);
}

std::string tt::FNb(int leadingZeros,float number,int decimals,bool sign)
{ // sign is true by default

    int dec=getDecimalPos(number,decimals*2);
    if (dec>decimals)
        decimals=dec;

    std::string tmp;
    if (sign)
        tmp=boost::str(boost::format("%%+#0%i.%if") % (leadingZeros+decimals+2) % decimals);
    else
        tmp=boost::str(boost::format("%%#0%i.%if") % (leadingZeros+decimals+2) % decimals);
    std::string tmp2(boost::str(boost::format(tmp) % number));

    return(tmp2);
}

std::string tt::FNb(float number)
{
    return(boost::str(boost::format("%f") % number));
}

std::string tt::FNb(double number)
{
    return(boost::str(boost::format("%f") % number));
}

std::string tt::FNb(int leadingZeros,int number,bool sign)
{ // sign is false by default
    std::string tmp;
    if (sign)
        tmp=boost::str(boost::format("%%+#0%ii") % leadingZeros);
    else
        tmp=boost::str(boost::format("%%#0%ii") % leadingZeros);
    std::string tmp2(boost::str(boost::format(tmp) % number));
    return(tmp2);
}

std::string tt::FNb(int number)
{
    return(boost::str(boost::format("%i") % number));
}

double tt::getLimitedDouble(double minValue,double maxValue,double value)
{
    if (value>maxValue)
        value=maxValue;
    if (value<minValue)
        value=minValue;
    return(value);
}

void tt::limitValue(float minValue,float maxValue,float value[2])
{
    if (value[0]>maxValue) value[0]=maxValue;
    if (value[1]>maxValue) value[1]=maxValue;
    if (value[0]<minValue) value[0]=minValue;
    if (value[1]<minValue) value[1]=minValue;
}

void tt::limitValue(int minValue,int maxValue,int* value)
{
    if ((*value)>maxValue) 
        (*value)=maxValue;
    if ((*value)<minValue) 
        (*value)=minValue;
}

bool tt::getValidFloat(const std::string& text,float& value)
{   // don't forget those cases: 1.06581410364015e-014
    try
    {
        value=boost::lexical_cast<float>(text);
        return(true);
    }
    catch (boost::bad_lexical_cast &)
    {
        return(false);
    }
}

bool tt::getValidInt(const std::string& text,int& value)
{
    try
    {
        value=boost::lexical_cast<int>(text);
        return(true);
    }
    catch (boost::bad_lexical_cast &)
    { // "1.0" and such are caught here too!
        return(false);
    }
}

bool tt::extractSpaceSeparatedWord(std::string& line,std::string& word)
{ // Returns true if a word could be extracted
    // 1. We remove leading spaces/tabs
    while ( (line.length()!=0)&&((line[0]==' ')||(line[0]==(char)9)) )
        line.erase(line.begin());
    // 2. We extract a word:
    word="";
    while ( (line.length()!=0)&&(line[0]!=' ')&&(line[0]!=(char)9) )
    {
        word.append(line.begin(),line.begin()+1);
        line.erase(line.begin());
    }
    return(word.length()!=0);
}

bool tt::extractSpaceSeparatedWord2(std::string& line,std::string& word,bool recognizeQuote,bool recognizeDoubleQuote,bool recognizeParenthesis,bool recognizeBrackets,bool recognizeSquareBrackets)
{ // Returns true if a word could be extracted
    // 1. We remove leading spaces/tabs
    while ( (line.length()!=0)&&((line[0]==' ')||(line[0]==(char)9)) )
        line.erase(line.begin());
    // 2. We extract a word:
    word="";
    std::string parse;
    while ( (line.length()!=0)&&(((line[0]!=' ')&&(line[0]!=(char)9))||(parse.size()>0)) )
    {
        if (recognizeQuote)
        {
            if (line[0]=='\'')
            {
                if ( (parse.size()==0)||(parse[parse.size()-1]!='\'') )
                    parse+='\'';
                else
                    parse.erase(parse.end()-1);
            }
        }
        if (recognizeDoubleQuote)
        {
            if (line[0]=='\"')
            {
                if ( (parse.size()==0)||(parse[parse.size()-1]!='\"') )
                    parse+='\"';
                else
                    parse.erase(parse.end()-1);
            }
        }
        if (recognizeParenthesis)
        {
            if (line[0]=='(')
                parse+='(';
            if (line[0]==')')
            {
                if ( (parse.size()!=0)&&(parse[parse.size()-1]=='(') )
                    parse.erase(parse.end()-1);
            }
        }
        if (recognizeBrackets)
        {
            if (line[0]=='{')
                parse+='{';
            if (line[0]=='}')
            {
                if ( (parse.size()!=0)&&(parse[parse.size()-1]=='{') )
                    parse.erase(parse.end()-1);
            }
        }
        if (recognizeSquareBrackets)
        {
            if (line[0]=='[')
                parse+='[';
            if (line[0]==']')
            {
                if ( (parse.size()!=0)&&(parse[parse.size()-1]=='[') )
                    parse.erase(parse.end()-1);
            }
        }
        word.append(line.begin(),line.begin()+1);
        line.erase(line.begin());
    }
    return(word.length()!=0);
}

bool tt::extractCommaSeparatedWord(std::string& line,std::string& word)
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

int tt::getNameSuffixNumber(const char* name,bool dash)
{ // -1 means there is no suffix!!
    std::string n(name);
    // In case the string is empty:
    if (n.length()==0)
        return(-1);
    // We count how many digits at the end:
    char aChar=n[n.length()-1];
    std::string oldNumber("");
    while ( (aChar>='0')&&(aChar<='9') )
    {
        oldNumber.insert(oldNumber.begin(),aChar);
        n.erase(n.end()-1);
        if (n.length()!=0)
            aChar=name[n.length()-1];
        else
            aChar='a';
    }
    if ((aChar!='#')&&dash) // 2009/02/04 (NEW COPY INDICATOR!)
        return(-1);
    if (oldNumber.length()==0)
        return(-1);
    return(atoi(oldNumber.c_str()));
}

std::string tt::getNameWithoutSuffixNumber(const char* name,bool dash)
{
    std::string n(name);
    if (n.length()==0)
        return("");
    // We count how many digits at the end:
    char aChar=n[n.length()-1];
    while ( (aChar>='0')&&(aChar<='9') )
    {
        n.erase(n.end()-1);
        if (n.length()!=0)
            aChar=name[n.length()-1];
        else
            aChar='a';
    }
    if (dash)
    {
        if (aChar!='#')
            return(name);
        n.erase(n.end()-1);
    }
    return(n);
}

std::string tt::generateNewName_dash(const std::string& name)
{
    return(generateNewName_dash(name,1)); // increment by 1
}

std::string tt::generateNewName_dash(const std::string& name,int suffixOffset)
{
    int newNumber=getNameSuffixNumber(name.c_str(),true)+suffixOffset;
    std::string nameWithoutSuffix(getNameWithoutSuffixNumber(name.c_str(),true));
    if (newNumber>=0)
    { // should anyway always pass!!
        std::string newNumberStr=FNb(0,newNumber,false);
        nameWithoutSuffix+="#";
        nameWithoutSuffix.append(newNumberStr);
    }
    return(nameWithoutSuffix);
}

std::string tt::generateNewName_noDash(const std::string& name)
{
    int newNumber=getNameSuffixNumber(name.c_str(),false)+1;
    std::string nameWithoutSuffix(getNameWithoutSuffixNumber(name.c_str(),false));
    std::string newNumberStr=FNb(0,newNumber,false);
    nameWithoutSuffix.append(newNumberStr);
    return(nameWithoutSuffix);
}

std::string tt::generateNewName_noDash(const std::string& name,int suffixOffset)
{
    int newNumber=getNameSuffixNumber(name.c_str(),false)+suffixOffset;
    std::string nameWithoutSuffix(getNameWithoutSuffixNumber(name.c_str(),false));
    if (newNumber>=0)
    { // should anyway always pass!!
        std::string newNumberStr=FNb(0,newNumber,false);
        nameWithoutSuffix.append(newNumberStr);
    }
    return(nameWithoutSuffix);
}

bool tt::removeIllegalCharacters(std::string& text,bool allowOneDashFollowedByNumbers)
{   // Illegal characters are replaced with underscore.
    // Permitted characters are: a-z, A-Z, 0-9, parenthesis and underscore
    // If allowOneDashFollowedByNumbers is true, then one dash (not in first position) followed by a number are ok 
    // Return value is true if something was modified
    bool retVal=false;

    size_t dashPos=std::string::npos;
    if (allowOneDashFollowedByNumbers)
    {
        dashPos=text.find('#');
        if (dashPos!=std::string::npos)
        {
            if ( (dashPos==0)||(dashPos==(text.length()-1)) )
                dashPos=std::string::npos;
            else
            {
                // Now check if there is a valid number after the dash:
                if ((text[dashPos+1]=='0')&&(text.length()>dashPos+2)) // no leading zero allowed after the dash, unless last char
                    dashPos=-1;
                else
                {
                    bool ok=true;
                    for (int i=int(dashPos)+1;i<int(text.length());i++)
                    {
                        if ((text[i]<'0')||(text[i]>'9'))
                        {
                            ok=false;
                            break;
                        }
                    }
                    if (ok)
                        text[dashPos]='_'; // so that we don't generate falsly a 'true' return value
                    else
                        dashPos=-1;
                }
            }
        }
    }

    for (size_t i=0;i<text.length();i++)
    {
        bool ok=false;
        if ((text[i]>='a')&&(text[i]<='z'))
            ok=true;
        if ((text[i]>='A')&&(text[i]<='Z'))
            ok=true;
        if ((text[i]>='0')&&(text[i]<='9'))
            ok=true;
        if ((text[i]=='(')||(text[i]==')')||(text[i]=='_'))
            ok=true;
        if (!ok)
        {
            text[i]='_';
            retVal=true;
        }
    }
    if (dashPos!=std::string::npos)
        text[dashPos]='#';
    return(retVal);
}

bool tt::removeAltNameIllegalCharacters(std::string& text)
{   // Illegal characters are replaced with underscore.
    // Permitted characters are: a-z, A-Z, 0-9, underscore
    // Return value is true if something was modified
    bool retVal=false;

    for (size_t i=0;i<text.length();i++)
    {
        bool ok=false;
        if ((text[i]>='a')&&(text[i]<='z'))
            ok=true;
        if ((text[i]>='A')&&(text[i]<='Z'))
            ok=true;
        if ((text[i]>='0')&&(text[i]<='9'))
            ok=true;
        if (text[i]=='_')
            ok=true;
        if (!ok)
        {
            text[i]='_';
            retVal=true;
        }
    }
    return(retVal);
}

std::string tt::getObjectAltNameFromObjectName(const std::string& text)
{
    std::string retVal(text);
    boost::replace_all(retVal,"#","_HASHMARK_");
    boost::replace_all(retVal,"-","_MINUSMARK_");
    boost::replace_all(retVal,"(","_OPARENTHESISMARK_");
    boost::replace_all(retVal,")","_CPARENTHESISMARK_");
    // just in case:
    removeAltNameIllegalCharacters(retVal);
    return(retVal);
}

void tt::addToFloatArray(std::vector<float>* ar,float x,float y,float z)
{
    ar->push_back(x);
    ar->push_back(y);
    ar->push_back(z);
}
void tt::addToIntArray(std::vector<int>* ar,int x,int y,int z)
{
    ar->push_back(x);
    ar->push_back(y);
    ar->push_back(z);
}

float tt::getNormalizedAngle(float angle)
{ // Returns an angle between -PI and +PI
    double sinAngle=sin(double(angle));
    double cosAngle=cos(double(angle));
    double angle_da=atan2(sinAngle,cosAngle);
    return(float(angle_da));
}

float tt::getNormalizedAngle_range(float angle,float range)
{ // Returns an angle between -range/2 and +range/2, or between -PI and +PI if range is 0.0
    if (range==0.0)
        return(getNormalizedAngle(angle));
    double a=6.28318530718*double(angle/range);
    double sinAngle=sin(a);
    double cosAngle=cos(a);
    double angle_da=atan2(sinAngle,cosAngle);
    return(float(angle_da*double(range)/6.28318530718));
}

float tt::getAngleMinusAlpha(float angle,float alpha)
{   // Returns angle-alpha. Angle and alpha are cyclic angles!!
    double sinAngle0=sin(double(angle));
    double sinAngle1=sin(double(alpha));
    double cosAngle0=cos(double(angle));
    double cosAngle1=cos(double(alpha));
    double sin_da=sinAngle0*cosAngle1-cosAngle0*sinAngle1;
    double cos_da=cosAngle0*cosAngle1+sinAngle0*sinAngle1;
    double angle_da=atan2(sin_da,cos_da);
    return(float(angle_da));
}

float tt::getAngleMinusAlpha_range(float angle,float alpha,float range)
{   // Returns angle-alpha. Angle and alpha are cyclic values in range [-range/2;range/2], or in range [-PI;+PI] if range is 0.0
    if (range==0.0)
        return(getAngleMinusAlpha(angle,alpha));
    double angle_=6.28318530718*double(angle/range);
    double alpha_=6.28318530718*double(alpha/range);
    double sinAngle0=sin(angle_);
    double sinAngle1=sin(alpha_);
    double cosAngle0=cos(angle_);
    double cosAngle1=cos(alpha_);
    double sin_da=sinAngle0*cosAngle1-cosAngle0*sinAngle1;
    double cos_da=cosAngle0*cosAngle1+sinAngle0*sinAngle1;
    double angle_da=atan2(sin_da,cos_da);
    return(float(angle_da*double(range)/6.28318530718));
}

void tt::rgbToHsl(float rgb[3],float hsl[3])
{
    float r=rgb[0];
    float g=rgb[1];
    float b=rgb[2];
    float h,s,l,delta;
    float cmax=SIM_MAX(r,SIM_MAX(g,b));
    float cmin=SIM_MIN(r,SIM_MIN(g,b));
    l=(cmax+cmin)/2.0f;
    if (cmax==cmin) 
    {
        s=0.0f;
        h=0.0f;
    } 
    else 
    {
        if(l<0.5f) 
            s=(cmax-cmin)/(cmax+cmin);
        else
            s=(cmax-cmin)/(2.0f-cmax-cmin);
        delta=cmax-cmin;
        if (r==cmax)
            h=(g-b)/delta;
        else 
            if (g==cmax)
                h=2.0f+(b-r)/delta;
            else
                h=4.0f+(r-g)/delta;
        h=h/6.0f;
        if (h<0.0f)
            h=h+1.0f;
    }
    hsl[0]=h;
    hsl[1]=s;
    hsl[2]=l;
}

float tt::_hueToRgb(float m1,float m2,float h)
{
    if (h<0.0f) 
        h=h+1.0f;
    if (h>1.0f) 
        h=h-1.0f;
    if (6.0f*h<1.0f)
        return(m1+(m2-m1)*h*6.0f);
    if (2.0f*h<1.0f)
        return(m2);
    if (3.0f*h<2.0f)
        return(m1+(m2-m1)*((2.0f/3.0f)-h)*6.0f);
    return(m1);
}

void tt::hslToRgb(float hsl[3],float rgb[3])
{
    float h=hsl[0];
    float s=hsl[1];
    float l=hsl[2];
    float m1,m2;

    if (s==0.0f)
    {
        rgb[0]=l;
        rgb[1]=l;
        rgb[2]=l;
    }
    else 
    {
        if (l<=0.5f)
            m2=l*(1.0f+s);
        else
            m2=l+s-l*s;
        m1=2.0f*l-m2;
        rgb[0]=_hueToRgb(m1,m2,h+1.0f/3.0f);
        rgb[1]=_hueToRgb(m1,m2,h);
        rgb[2]=_hueToRgb(m1,m2,h-1.0f/3.0f);
    }
}

void tt::orderAscending(std::vector<int>& toBeOrdered,std::vector<int>& index)
{
    std::vector<filestruct_A> lst;
    for (size_t i=0;i<toBeOrdered.size();i++)
    {
        filestruct_A s;
        s.valA=toBeOrdered[i];
        s.valB=index[i];
        lst.push_back(s);
    }
    std::sort(lst.begin(),lst.end());
    for (size_t i=0;i<lst.size();i++)
    {
        toBeOrdered[i]=lst[i].valA;
        index[i]=lst[i].valB;
    }
}

void tt::orderAscending(std::vector<float>& toBeOrdered,std::vector<int>& index)
{
    std::vector<filestruct_B> lst;
    for (size_t i=0;i<toBeOrdered.size();i++)
    {
        filestruct_B s;
        s.valA=toBeOrdered[i];
        s.valB=index[i];
        lst.push_back(s);
    }
    std::sort(lst.begin(),lst.end());
    for (size_t i=0;i<lst.size();i++)
    {
        toBeOrdered[i]=lst[i].valA;
        index[i]=lst[i].valB;
    }
}

void tt::orderStrings(std::vector<std::string>& toBeOrdered,std::vector<int>& index)
{
    std::vector<filestruct_C> lst;
    for (size_t i=0;i<toBeOrdered.size();i++)
    {
        filestruct_C s;
        s.valA=toBeOrdered[i];
        s.valB=index[i];
        lst.push_back(s);
    }
    std::sort(lst.begin(),lst.end());
    for (size_t i=0;i<lst.size();i++)
    {
        toBeOrdered[i]=lst[i].valA;
        index[i]=lst[i].valB;
    }
}

std::string tt::getLowerUpperCaseString(std::string str,bool upper)
{
    if (upper)
    {
        for (size_t i=0;i<str.length();i++)
            str[i]=toupper(str[i]);
    }
    else
    {
        for (size_t i=0;i<str.length();i++)
            str[i]=tolower(str[i]);
    }
    return(str);
}

void tt::getColorFromIntensity(float intensity,int colorTable,float col[3])
{
    if (intensity>1.0f)
        intensity=1.0f;
    if (intensity<0.0f)
        intensity=0.0f;
    const float c[12]={0.0f,0.0f,0.0f,0.0f,0.0f,1.0f,1.0f,0.0f,0.0f,1.0f,1.0f,0.0f};
    int d=int(intensity*3);
    if (d>2)
        d=2;
    float r=(intensity-float(d)/3.0f)*3.0f;
    col[0]=c[3*d+0]*(1.0f-r)+c[3*(d+1)+0]*r;
    col[1]=c[3*d+1]*(1.0f-r)+c[3*(d+1)+1]*r;
    col[2]=c[3*d+2]*(1.0f-r)+c[3*(d+1)+2]*r;
}


int tt::getLimitedInt(int minValue,int maxValue,int value)
{
    if (value>maxValue)
        value=maxValue;
    if (value<minValue)
        value=minValue;
    return(value);
}

void tt::limitValue(int minValue,int maxValue,int &value)
{
    if (value>maxValue) value=maxValue;
    if (value<minValue) value=minValue;
}

float tt::getLimitedFloat(float minValue,float maxValue,float value)
{
    if (value>maxValue)
        value=maxValue;
    if (value<minValue)
        value=minValue;
    return(value);
}

void tt::limitValue(float minValue,float maxValue,float &value)
{
    if (value>maxValue) value=maxValue;
    if (value<minValue) value=minValue;
}

bool tt::getValueOfKey(const char* key,const char* txt,std::string& value)
{
    std::vector<std::string> keys;
    int keyCnt=separateWords(key,'@',keys);
    if (keyCnt==0)
        return(false);
    if (strlen(txt)==0)
        return(false);
    std::string currentString(txt);
    for (int keyC=int(keys.size()-1);keyC>=0;keyC--)
    {
        std::vector<std::string> allKeys;
        std::vector<std::string> allValues;
        getAllKeyValuePairs(currentString.c_str(),allKeys,allValues);
        currentString.clear();
        for (size_t i=0;i<allKeys.size();i++)
        {
            if (allKeys[i].compare(keys[keyC])==0)
            {
                currentString=allValues[i];
                break;
            }
        }
        if (currentString.size()==0)
            return(false);
    }
    value=currentString;
    return(true);
}

void tt::removeKeyAndValue(const char* key,std::string& txt)
{
    _removeKeyAndValue(key,txt);
    _removeKeysWithEmptyValues(txt);
}

void tt::_removeKeyAndValue(const char* key,std::string& txt)
{
    if (txt.size()==0)
        return;
    std::string theMainKey(key);
    std::string theAuxKey;
    size_t p=theMainKey.find('@');
    while (p!=std::string::npos)
    {
        std::string subF(theMainKey.begin(),theMainKey.begin()+p);
        if (theAuxKey.size()!=0)
            theAuxKey+='@';
        theAuxKey+=subF;
        theMainKey.assign(theMainKey.begin()+p+1,theMainKey.end());
        p=theMainKey.find('@');
    }
    std::string newString;
    std::vector<std::string> allKeys;
    std::vector<std::string> allValues;
    getAllKeyValuePairs(txt.c_str(),allKeys,allValues);
    txt.clear();
    for (size_t i=0;i<allKeys.size();i++)
    {
        if (allKeys[i].compare(theMainKey)!=0)
            appendKeyValuePair(txt,allKeys[i],allValues[i]);
        else
            newString=allValues[i];
    }
    if (theAuxKey.size()!=0)
    {
        _removeKeyAndValue(theAuxKey.c_str(),newString);
//      if (newString.size()!=0)
            appendKeyValuePair(txt,theMainKey,newString);
    }
}

void tt::_removeKeysWithEmptyValues(std::string& txt)
{
    if (txt.size()==0)
        return;
    std::vector<std::string> allKeys;
    std::vector<std::string> allValues;
    if (getAllKeyValuePairs(txt.c_str(),allKeys,allValues)>0)
    {
        txt.clear();
        for (size_t i=0;i<allKeys.size();i++)
        {
            _removeKeysWithEmptyValues(allValues[i]);
            if (allValues[i].size()>0)
                appendKeyValuePair(txt,allKeys[i],allValues[i]);
        }
    }
}

void tt::insertKeyAndValue(const char* key,const char* value,std::string& txt)
{
    tt::removeKeyAndValue(key,txt);
    tt::_insertKeyAndValue(key,value,txt);
}

void tt::_insertKeyAndValue(const char* key,const char* value,std::string& txt)
{
    std::string theMainKey(key);
    std::string theAuxKey;
    size_t p=theMainKey.find('@');
    while (p!=std::string::npos)
    {
        std::string subF(theMainKey.begin(),theMainKey.begin()+p);
        if (theAuxKey.size()!=0)
            theAuxKey+='@';
        theAuxKey+=subF;
        theMainKey.assign(theMainKey.begin()+p+1,theMainKey.end());
        p=theMainKey.find('@');
    }
    std::string newString;
    std::vector<std::string> allKeys;
    std::vector<std::string> allValues;
    getAllKeyValuePairs(txt.c_str(),allKeys,allValues);
    txt.clear();
    for (size_t i=0;i<allKeys.size();i++)
    {
        if (allKeys[i].compare(theMainKey)!=0)
            appendKeyValuePair(txt,allKeys[i],allValues[i]);
        else
            newString=allValues[i];
    }

    if (theAuxKey.size()==0)
        appendKeyValuePair(txt,key,value);
    else
    {
        _insertKeyAndValue(theAuxKey.c_str(),value,newString);
        appendKeyValuePair(txt,theMainKey,newString);
    }
}

int tt::getAllKeyValuePairs(const char* txt,std::vector<std::string>& allKeys,std::vector<std::string>& allValues)
{
    allKeys.clear();
    allValues.clear();
    std::string f(txt);
    while (true)
    {
        size_t p1=f.find('{');
        if (p1==std::string::npos)
            break;
        int opCnt=1;
        size_t p2=p1+1;
        bool ok=false;
        while (f.size()>p2)
        {
            if (f[p2]=='{')
                opCnt++;
            if (f[p2]=='}')
                opCnt--;
            if (opCnt==0)
            {
                ok=true;
                break;
            }
            p2++;
        }
        if (!ok)
            break;

        std::string key(f.begin(),f.begin()+p1);
        removeSpacesAtBeginningAndEnd(key);
        std::string value(f.begin()+p1+1,f.begin()+p2);
        removeSpacesAtBeginningAndEnd(value);
        // insert only if that key is not yet present:
        bool found=false;
        for (size_t i=0;i<allKeys.size();i++)
        {
            if (allKeys[i].compare(key)==0)
                found=true;
        }
        if (!found)
        {
            allKeys.push_back(key);
            allValues.push_back(value);
        }
        f.assign(f.begin()+p2+1,f.end());
    }
    return((int)allKeys.size());
}

std::string tt::generateKeyValuePairs(const std::vector<std::string>& allKeys,const std::vector<std::string>& allValues)
{
    std::string retV;
    for (size_t i=0;i<allKeys.size();i++)
        appendKeyValuePair(retV,allKeys[i],allValues[i]);
    return(retV);
}

void tt::appendKeyValuePair(std::string& txt,const std::string& key,const std::string& value)
{
    if (txt.size()>0)
        txt+=" ";
    txt+=key;
    txt+=" {";
    txt+=value;
    txt+="}";
}

int tt::separateWords(const char* txt,char separationChar,std::vector<std::string>& words)
{
    words.clear();
    if (strlen(txt)==0)
        return(0);
    std::string f(txt);
    size_t p=f.find(separationChar);
    while (p!=std::string::npos)
    {
        std::string subF(f.begin(),f.begin()+p);
        words.push_back(subF);
        f.assign(f.begin()+p+1,f.end());
        p=f.find(separationChar);
    }
    words.push_back(f);
    return((int)words.size());
}
