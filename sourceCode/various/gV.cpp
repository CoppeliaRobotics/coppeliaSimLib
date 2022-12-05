
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

std::string gv::formatUnitStr(const char* txt,const char* s,bool inParenthesis,bool startWithSpace)
{
    std::string str(txt);
    if (startWithSpace)
        str+=" ";
    if (inParenthesis)
        str+="[";
    std::string ss(s);
    for (size_t i=0;i<ss.length();i++)
    {
        if (ss[i]=='m')
            str+="m";
        else if (ss[i]=='s')
            str+="s";
        else if (ss[i]=='k')
            str+="kg";
        else if (ss[i]=='r')
            str+="deg";
        else
            str+=ss[i];
    }
    if (inParenthesis)
        str+="]";
    return(str);
}

std::string gv::getTimeStr(double timeInSeconds,int additionalDecimals)
{
    return(tt::FNb(0,timeInSeconds,3+additionalDecimals,false));
}

std::string gv::getHourMinuteSecondMilisecondStr(double timeInSeconds)
{
    std::string tmp(tt::FNb(2,int(timeInSeconds/3600.0),false));
    tmp+=":";
    tmp+=tt::FNb(2,int(CMath::robustMod(timeInSeconds/60.0,60.0)),false)+":";
    tmp+=tt::FNb(2,int(CMath::robustMod(timeInSeconds,60.0)),false)+".";
    tmp+=tt::FNb(2,int(CMath::robustMod(timeInSeconds*100.0,100.0)),false);
    return(tmp);
}


std::string gv::getAngleStr(bool sign,double angleInRad,int leadingZeros)
{ // if leadingZeros==-1, the default leading zeros are used
    if (leadingZeros==-1)
        leadingZeros=3;
    return(tt::FNb(leadingZeros,angleInRad*radToUser,2,sign));
}

std::string gv::getSizeStr(bool sign,double sizeInMeter,int leadingZeros)
{ // if leadingZeros==-1, the default leading zeros are used
    if (leadingZeros==-1)
        leadingZeros=1;
    return(tt::FNb(leadingZeros,sizeInMeter,4,sign));
}

double gv::radToUser=radToDeg;
double gv::userToRad=degToRad;
double gv::angularVelToUser=radToDeg;
double gv::userToAngularVel=degToRad;
double gv::angularAccelToUser=radToDeg;
double gv::userToAngularAccel=degToRad;


