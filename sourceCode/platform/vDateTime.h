#pragma once

// FULLY STATIC CLASS
class VDateTime  
{
public:
    static int getTimeInMs();
    static unsigned int getOSTimeInMs();
    static int getTimeDiffInMs(int lastTime);
    static int getTimeDiffInMs(int oldTime,int newTime);
    static unsigned long long int getSecondsSince1970();
    static void getYearMonthDayHourMinuteSecond(int* year,int* month,int* day,int* hour,int* minute,int* second);
    static int getDaysTo(int year_before,int month_before,int day_before,int year_after,int month_after,int day_after);

private:
    static int _getTimeWithStartInMs_viaQt(bool& success);
#ifdef WIN_SIM
    static int _getTimeWithStartInMs_viaPerformanceCounter(bool& success);
    static int _getTimeWithStartInMs_viaTimeGetTime(bool& success);
#else
    static int _getTimeWithStartInMs_viaGetTimeOfDay(bool& success);
#endif
};
