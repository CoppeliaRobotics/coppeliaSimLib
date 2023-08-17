#include <vDateTime.h>
#include <QDateTime>
#include <QElapsedTimer>
#include <ctime>
#ifdef WIN_SIM
#include <Windows.h>
#else
#include <sys/time.h>
#endif

unsigned int VDateTime::getOSTimeInMs()
{
#ifdef WIN_SIM
    return(timeGetTime());
#else
    struct timeval now;
    gettimeofday(&now,nullptr);
    return(now.tv_sec*1000+now.tv_usec/1000);
#endif
}

double VDateTime::getTime()
{
    return(double(getTimeInMs())/1000.0);
}

long long int VDateTime::getTimeInMs()
{
    long long int retVal=0;
#ifdef WIN_SIM
    bool ok;
    retVal=_getTimeWithStartInMs_viaPerformanceCounter(ok);
    if (!ok)
        retVal=timeGetTime();
#else
    struct timeval now;
    gettimeofday(&now,nullptr);
    static time_t initSec=now.tv_sec;
    static suseconds_t initUSec=now.tv_usec;
    retVal=(now.tv_sec-initSec)*1000+(now.tv_usec-initUSec)/1000;
#endif
    return(retVal);
}

int VDateTime::getTimeDiffInMs(int lastTime)
{
    return(getTimeDiffInMs(lastTime,(int)getTimeInMs()));
}

int VDateTime::getTimeDiffInMs(int oldTime,int newTime)
{
    return(newTime-oldTime);
}

quint64 VDateTime::getSecondsSince1970()
{
    QDateTime now=QDateTime::currentDateTime();
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        return(now.toTime_t());
    #else
        return(now.toSecsSinceEpoch());
    #endif
}

void VDateTime::getYearMonthDayHourMinuteSecond(int* year,int* month,int* day,int* hour,int* minute,int* second)
{
    QDate now=QDate::currentDate();
    QTime nowTime=QTime::currentTime();
    if (year!=nullptr)
        year[0]=now.year();
    if (month!=nullptr)
        month[0]=now.month();
    if (day!=nullptr)
        day[0]=now.day();
    if (hour!=nullptr)
        hour[0]=nowTime.hour();
    if (minute!=nullptr)
        minute[0]=nowTime.minute();
    if (second!=nullptr)
        second[0]=nowTime.second();
}

int VDateTime::getDaysTo(int year_before,int month_before,int day_before,int year_after,int month_after,int day_after)
{
    QDate before(year_before,month_before,day_before);
    return(before.daysTo(QDate(year_after,month_after,day_after)));
}

#ifdef WIN_SIM
long long int VDateTime::_getTimeWithStartInMs_viaPerformanceCounter(bool& success)
{
    static bool works=true;
    static bool first=true;
    static double pcFreq=0.0;
    static long long int cntStart=0;
    LARGE_INTEGER highResFreq;
    if (first&&works)
    {
        first=false;
        works=(QueryPerformanceFrequency(&highResFreq)!=0);
        pcFreq=double(highResFreq.QuadPart)/1000.0;
        QueryPerformanceCounter(&highResFreq);
        cntStart=highResFreq.QuadPart;
    }
    if (!works)
    {
        success=false;
        return(0);
    }
    success=true;
    QueryPerformanceCounter(&highResFreq);
    return((long long int)(double(highResFreq.QuadPart-cntStart)/pcFreq));
}
#endif

