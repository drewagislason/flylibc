/**************************************************************************************************
  FlyTime.c - Basic time functions, current, elapsed, etc...
  Copyright 2024 Drew Gislason
  License: MIT <https://mit-license.org>
*///***********************************************************************************************
#include "FlyTime.h"
#include <time.h>
#include <sys/time.h>

/*!
  @defgroup   FlyTime  A simplified time interface

  This defines a small set of functions for checking time and date, creating timers and other. It
  relies on standard C time/date functions, or the app provide functions to customize them.

  Small embedded systems (with no OS) may not have time functions available. This API can support
  those systems.

  Features:

  1. Embeddable: small footprint, no malloc(), redefine inputs
  2. Understands epoch time
  3. Understands ISO 8601 standard date/time format
  4. Create timers
  5. Check time/date
  6. Convert from UTC to local time
  7. Embeddable: time input functions can be redifined for the embedded system.
  8. Random numbers for doing things at random times

  See also: <https://www.epochconverter.com>
*/

#ifndef FLY_TIME_GET
#define FLY_TIME_GET gettimeofday
#endif

#ifndef FLY_TIME_SLEEP
#define FLY_TIME_SLEEP nanosleep
#endif

#ifndef FLY_TIME_TIME
#define FLY_TIME_TIME time
#endif

#ifndef FLY_TIME_GM_TIME
#define FLY_TIME_GM_TIME gmtime
#endif

#ifndef FLY_TIME_LOCAL_TIME
#define FLY_TIME_LOCAL_TIME localtime
#endif

/*!-----------------------------------------------------------------------------------------------
  Seed random() generator with current time. Also returns the current time.

  @return   # of seconds since Epoch UTC (1970).
*///-----------------------------------------------------------------------------------------------
flytime_t FlyTimeSeedRandom(void)
{
  time_t timeNow = time(NULL);
  srandom(timeNow);
  return (flytime_t)timeNow;
}

/*!-----------------------------------------------------------------------------------------------
  Determine time difference in milliseconds from time now. Expects timeMs to be <= time now or
  the number may be quite large.

  @param    timeMs   time returned by FlyTimeMsGet()
  @return   time difference
*///-----------------------------------------------------------------------------------------------
flytime_t FlyTimeMsDiff(flytime_t timeMs)
{
  return FlyTimeMsGet() - timeMs;
}

/*!-----------------------------------------------------------------------------------------------
  Get the current # of milliseconds.  
  NOTE: this can roll-over if time_t is less than 64-bits

  @param    milliseconds
  @return   current time from epoch in ms
*///-----------------------------------------------------------------------------------------------
flytime_t FlyTimeMsGet(void)
{
  struct timeval  tv;
  long            timeStamp;

  FLY_TIME_GET(&tv, NULL);
  timeStamp = 1000UL * tv.tv_sec + (unsigned long)(tv.tv_usec / 1000);

  return timeStamp;
}

/*!-----------------------------------------------------------------------------------------------
  Wait a certain number of milliseconds

  @param    milliseconds
  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyTimeMsSleep(unsigned long milliseconds)
{
   struct timespec tv = {
       milliseconds / 1000,             // seconds
       (milliseconds % 1000) * 1000000  // microseconds
   };

   FLY_TIME_SLEEP(&tv , NULL);  
}

/*!-----------------------------------------------------------------------------------------------
  Determine time difference in microseconds from time now. Expects timeUs to be <= time now or
  number by be quite large.

  @param    timeMs   time returned by FlyTimeMsGet()
  @return   time difference.
*///-----------------------------------------------------------------------------------------------
flytime_t FlyTimeUsDiff(flytime_t timeUs)
{
  return FlyTimeUsGet() - timeUs;
}

/*!-----------------------------------------------------------------------------------------------
  Get the current # of microseconds.
  NOTE: this can roll-over even if time_t is 64-bits

  @param    milliseconds
  @return   current time from epoch in ms
*///-----------------------------------------------------------------------------------------------
flytime_t FlyTimeUsGet(void)
{
  struct timeval  tv;
  long            timeStamp;

  FLY_TIME_GET(&tv, NULL);
  timeStamp = 1000UL * tv.tv_sec + (unsigned long)(tv.tv_usec / 1000);

  return timeStamp;
}

/*!-----------------------------------------------------------------------------------------------
  Wait a certain number of milliseconds

  @param    milliseconds
  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyTimeUsSleep(unsigned long microseconds)
{
   struct timespec tv = {
       microseconds / (time_t)1000000,  // seconds
       (microseconds % (long)1000000)   // microseconds
   };

   FLY_TIME_SLEEP(&tv , NULL);  
}

/*!-----------------------------------------------------------------------------------------------
  Current UTC epoch time.  
  NOTE: if time_t is only 32 bits, this limits the year to 2106, eopch

  @return   UTC epoch time
*///-----------------------------------------------------------------------------------------------
flytime_t FlyTimeEpoch(long *pMicroseconds)
{
  struct timeval  tv;
  FLY_TIME_GET(&tv, NULL);

  if(pMicroseconds)
    *pMicroseconds = tv.tv_usec;
  return tv.tv_sec;
}

/*------------------------------------------------------------------------------------------------
  Helper to FlyTimeEpochStr() and FlyTimeLocalStr()
  format of string: Fri Sep 16 15:31:52 2022
-------------------------------------------------------------------------------------------------*/
static void TimeStr(const struct tm *pTime, char *szDst, unsigned size)
{
  static const char *aszMonth[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  static const char *aszWeekday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  unsigned          month;
  unsigned          weekday;

  // make sure string is NULL terminated
  if(szDst)
    *szDst = '0';
  if(pTime && szDst && size)
  {
    // keep strings in range
    weekday = pTime->tm_wday;
    if(weekday >= NumElements(aszWeekday))
      weekday = 0;
    month = pTime->tm_mon;
    if(month >= NumElements(aszMonth))
      month = 0;

    snprintf(szDst, size, "%s %s %2d %02d:%02d:%02d %d", aszWeekday[weekday], aszMonth[month],
      pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec, 1900 + pTime->tm_year);
    szDst[size - 1] = '\0';
  }
}

/*!-----------------------------------------------------------------------------------------------
  Similar to cmdline "date" format, Return epoch time string form: Fri Sep 16 15:31:52 2022

  @return   UTC epoch time
*///-----------------------------------------------------------------------------------------------
void FlyTimeEpochStr(flytime_t epoch, char *szDst, unsigned size)
{
  time_t            timeEpoch = epoch;
  const struct tm  *pTime = FLY_TIME_GM_TIME(&timeEpoch);
  TimeStr(pTime, szDst, size);
}

/*!-----------------------------------------------------------------------------------------------
  Similar to cmdline "date" format, Return local time string form: Fri Sep 16 15:31:52 2022

  @return   UTC epoch time
*///-----------------------------------------------------------------------------------------------
void FlyTimeEpochStrLocal(flytime_t epoch, char *szDst, unsigned size)
{
  time_t            timeEpoch = epoch;
  const struct tm  *pTime = FLY_TIME_LOCAL_TIME(&timeEpoch);
  TimeStr(pTime, szDst, size);
}

/*!-----------------------------------------------------------------------------------------------
  Return epoch time in the ISO 8601 string format: "2022-09-16T02:31:09"

  @return   string in ISO 8601 format
*///-----------------------------------------------------------------------------------------------
void FlyTimeEpochStrIso(flytime_t epoch, char *szDst, unsigned size)
{
  time_t            timeEpoch = epoch;
  const struct tm  *pTime = FLY_TIME_GM_TIME(&timeEpoch);

  if(szDst)
    *szDst = '\0';
  if(pTime && szDst && size)
  {
    snprintf(szDst, size, "%d-%02d-%02dT%02d:%02d:%02d", 1900 + pTime->tm_year, pTime->tm_mon + 1,
      pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
    szDst[size - 1] = '\0';
  }
}
