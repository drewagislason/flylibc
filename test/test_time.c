/*!************************************************************************************************
  @file FLyTestTime.c

  @brief  Unit test for time functions
  @copyright 2022 Drew Gislason
  @ingroup test
*///***********************************************************************************************
#include "FlyTest.h"
#include "FlyTime.h"

/*!------------------------------------------------------------------------------------------------
  Test waiting milliseconds
*///-----------------------------------------------------------------------------------------------
void FlyTestTimeWaitMs(void)
{
  long      aWait[] = { 5*1000L, 1*1000L, 500L, 1L };
  unsigned  i;

  FlyTestBegin();

  for(i = 0; i < NumElements(aWait); ++i)
  {
    printf("Sleeping %ld milliseconds...\n", aWait[i]);
    FlyTimeMsSleep(aWait[i]);
    if(!FlyTestGetYesNo("Did it wait properly?"))
    {
      FlyLogPrintf("Failed on wait %ld\n", aWait[i]);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*!------------------------------------------------------------------------------------------------
  Test waiting milliseconds
*///-----------------------------------------------------------------------------------------------
void FlyTestTimeGetMs(void)
{
  long      aWait[] = { 3*1000L, 1*1000L, 500L, 1L };
  unsigned  i;
  flytime_t timeMsOrg;
  flytime_t timeMs;
  flytime_t timeDiffMs;
  char      szDateTime[32];  // 2022-09-16T02:31:09.12345678

  FlyTestBegin();

  timeMsOrg = FlyTimeMsGet();
  FlyTimeEpochStrLocal(timeMsOrg / 1000, szDateTime, sizeof(szDateTime));
  FlyTestPrintf("\nCurrent UTC time st start of test %lu, %s\n", timeMsOrg, szDateTime);

  for(i = 0; i < NumElements(aWait); ++i)
  {

    printf("Sleeping %ld milliseconds...\n", aWait[i]);
    timeMs = FlyTimeMsGet();
    FlyTimeMsSleep(aWait[i]);
    timeDiffMs = FlyTimeMsDiff(timeMs);
    printf("Slept %ld milliseconds...\n", timeDiffMs);
    if(timeDiffMs < aWait[i])
    {
      FlyLogPrintf("Failed to sleep %ld, got %ld\n", aWait[i], timeDiffMs);
      FlyTestFailed();
    }
  }

  timeMs = FlyTimeMsGet();
  timeDiffMs = FlyTimeMsDiff(timeMsOrg);
  FlyTimeEpochStrLocal(timeMs / 1000, szDateTime, sizeof(szDateTime));
  FlyTestPrintf("Current UTC time at end of test %ld, %s\n", timeMs, szDateTime);
  FlyTestPrintf("Diff for entire test: %ld\n", timeDiffMs);
  if(timeDiffMs > 5000L)
      FlyTestFailed();    

  FlyTestEnd();
}

void FlyTestTimeEpoch(void)
{
  char  szTest[FLY_TIME_EPOCH_SIZE];
  const char szEpochStr[]       = "Fri Sep 16 23:20:04 2022";
  const char szEpochStrLocal[]  = "Fri Sep 16 16:20:04 2022";
  const char szEpochStrIso[]    = "2022-09-16T23:20:04";
  flytime_t time = 1663370404;
  flytime_t time2;

  FlyTestBegin();

  // make sure current time is greater than when this test was written
  time2 = FlyTimeEpoch(NULL);
  if(time2 <= time)
    FlyTestFailed();

  FlyTimeEpochStr(time, szTest, sizeof(szTest));
  if(strcmp(szTest, szEpochStr))
  {
    printf("got %s, expected %s\n", szTest, szEpochStr);
    FlyTestFailed();
  }

  FlyTimeEpochStrLocal(time, szTest, sizeof(szTest));
  if(strcmp(szTest, szEpochStrLocal))
  {
    printf("got %s, expected %s\n", szTest, szEpochStrLocal);
    FlyTestFailed();
  }

  FlyTimeEpochStrIso(time, szTest, sizeof(szTest));
  if(strcmp(szTest, szEpochStrIso))
  {
    printf("got %s, expected %s\n", szTest, szEpochStrIso);
    FlyTestFailed();
  }

  FlyTestEnd();
}

int main(int argc, const char *argv[])
{
  const char          szName[] = "test_time";
  const sTestCase_t   aTestCases[] =
  { 
    { "FlyTestTimeEpoch",  FlyTestTimeEpoch, "M" },
    { "FlyTestTimeWaitMs",  FlyTestTimeWaitMs, "M" },
    { "FlyTestTimeGetMs",   FlyTestTimeGetMs }
  };
  hTestSuite_t        hSuite;
  int                 ret;

  FlyTestInit(szName, argc, argv);
  hSuite = FlyTestNew(szName, NumElements(aTestCases), aTestCases);
  FlyTestRun(hSuite);
  ret = FlyTestSummary(hSuite);
  FlyTestFree(hSuite);

  return ret;
}

