
#include "vrepMainHeader.h"
#include "gV.h"
#include "global.h"
#include "tt.h"
#include "MyMath.h"
#include <boost/lexical_cast.hpp>

std::string gv::getAngleUnitStr()
{
    return("Degrees");
}

std::string gv::getSizeUnitStr()
{
    return("Meters");
}

std::string gv::getSizeUnitStrShort()
{
    return("m");
}

std::string gv::getMassUnitStr()
{
    return("Kilograms");
}

std::string gv::getMassUnitStrShort()
{
    return("kg");
}

std::string gv::getSurfaceUnitStr()
{
    return("Square meters");
}

std::string gv::getVolumeUnitStr()
{
    return("Cubic meters");
}

std::string gv::getTimeUnitStr()
{
    return("Seconds");
}

std::string gv::getTimeUnitStrShort()
{
    return("s");
}

std::string gv::getIntUnitStr()
{
    return("Integer");
}

std::string gv::getFloatUnitStr()
{
    return("Floating point");
}

std::string gv::getBoolUnitStr()
{
    return("Boolean");
}

std::string gv::getCountUnitStr()
{
    return("Count");
}

std::string gv::getBoolStateUnitStr()
{
    return("Boolean state");
}

std::string gv::getUserUnitStr()
{
    return("User unit");
}

std::string gv::getNullUnitStr()
{
    return("Null");
}

std::string gv::formatUnitStr(const std::string& txt,const std::string& s,bool inParenthesis,bool startWithSpace)
{
    std::string str(txt);
    if (startWithSpace)
        str+=" ";
    if (inParenthesis)
        str+="[";
    for (int i=0;i<int(s.length());i++)
    {
        if (s[i]=='m')
            str+="m";
        else if (s[i]=='s')
            str+="s";
        else if (s[i]=='k')
            str+="kg";
        else if (s[i]=='r')
            str+="deg";
        else
            str+=s[i];
    }
    if (inParenthesis)
        str+="]";
    return(str);
}

std::string gv::getTimeStr(float timeInSeconds,int additionalDecimals)
{
    return(tt::FNb(0,timeInSeconds,3+additionalDecimals,false));
}

std::string gv::getHourMinuteSecondMilisecondStr(float timeInSeconds)
{
    std::string tmp(tt::FNb(2,int(timeInSeconds/3600.0f),false));
    tmp+=":";
    tmp+=tt::FNb(2,int(CMath::robustFmod(timeInSeconds/60.0f,60.0f)),false)+":";
    tmp+=tt::FNb(2,int(CMath::robustFmod(timeInSeconds,60.0f)),false)+".";
    tmp+=tt::FNb(2,int(CMath::robustFmod(timeInSeconds*100.0f,100.0f)),false);
    return(tmp);
}


std::string gv::getAngleStr(bool sign,float angleInRad,int leadingZeros)
{ // if leadingZeros==-1, the default leading zeros are used
    if (leadingZeros==-1)
        leadingZeros=3;
    return(tt::FNb(leadingZeros,angleInRad*radToUser,2,sign));
}

std::string gv::getSizeStr(bool sign,float sizeInMeter,int leadingZeros)
{ // if leadingZeros==-1, the default leading zeros are used
    if (leadingZeros==-1)
        leadingZeros=1;
    return(tt::FNb(leadingZeros,sizeInMeter,4,sign));
}

float gv::radToUser=radToDeg_f;
float gv::userToRad=degToRad_f;
float gv::angularVelToUser=radToDeg_f;
float gv::userToAngularVel=degToRad_f;
float gv::angularAccelToUser=radToDeg_f;
float gv::userToAngularAccel=degToRad_f;


