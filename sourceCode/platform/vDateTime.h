#pragma once

#include <simLib/simTypes.h>

// FULLY STATIC CLASS
class VDateTime
{
  public:
    static double getTime();
    static int64_t getTimeInMs();

    static unsigned int getOSTimeInMs();
    static int getTimeDiffInMs(int lastTime);
    static int getTimeDiffInMs(int oldTime, int newTime);
    static uint64_t getSecondsSince1970();
    static uint64_t getUnixTimeInMs();
    static void getYearMonthDayHourMinuteSecond(int* year, int* month, int* day, int* hour, int* minute, int* second);
    static int getDaysTo(int year_before, int month_before, int day_before, int year_after, int month_after,
                         int day_after);

  private:
#ifdef WIN_SIM
    static int64_t _getTimeWithStartInMs_viaPerformanceCounter(bool& success);
#endif
};
