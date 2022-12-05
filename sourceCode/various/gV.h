#pragma once

#include <string>
#include "simTypes.h"

//FULLY STATIC CLASS
class gv  
{
public:
    static std::string getAngleStr(bool sign,double angleInRad,int leadingZeros=-1);
    static std::string getSizeStr(bool sign,double sizeInMeter,int leadingZeros=-1);

    static std::string getTimeStr(double timeInSeconds,int additionalDecimals=0);
    static std::string getHourMinuteSecondMilisecondStr(double timeInSeconds);


    static std::string getAngleUnitStr();
    static std::string getSizeUnitStr();
    static std::string getSizeUnitStrShort();
    static std::string getMassUnitStr();
    static std::string getMassUnitStrShort();
    static std::string getSurfaceUnitStr();
    static std::string getVolumeUnitStr();
    static std::string getTimeUnitStr();
    static std::string getTimeUnitStrShort();
    static std::string getIntUnitStr();
    static std::string getFloatUnitStr();
    static std::string getBoolUnitStr();
    static std::string getCountUnitStr();
    static std::string getBoolStateUnitStr();
    static std::string getUserUnitStr();
    static std::string getNullUnitStr();
    static std::string formatUnitStr(const char* txt,const char* s,bool inParenthesis=true,bool startWithSpace=true);

    static double radToUser;
    static double userToRad;
    static double angularVelToUser;
    static double userToAngularVel;
    static double angularAccelToUser;
    static double userToAngularAccel;
};
