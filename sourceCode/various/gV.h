#pragma once

#include <string>

//FULLY STATIC CLASS
class gv  
{
public:
    static std::string getAngleStr(bool sign,float angleInRad,int leadingZeros=-1);
    static std::string getSizeStr(bool sign,float sizeInMeter,int leadingZeros=-1);

    static std::string getTimeStr(float timeInSeconds,int additionalDecimals=0);
    static std::string getHourMinuteSecondMilisecondStr(float timeInSeconds);


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

    static float radToUser;
    static float userToRad;
    static float angularVelToUser;
    static float userToAngularVel;
    static float angularAccelToUser;
    static float userToAngularAccel;
};
