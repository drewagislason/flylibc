/*!************************************************************************************************
  @file FlyTestLog.c

  @brief  Fly Log Test Cases

  Copyright 2022 Drew Gislason

*///***********************************************************************************************
#include <unistd.h>
#include "Fly.h"
#include "FlyLog.h"
#include "FlyTest.h"
#include "FlyFile.h"
#include "FlyStr.h"

/*!------------------------------------------------------------------------------------------------
  Get an answer from the user

  @ingroup    ned_test
  @returns    none
-------------------------------------------------------------------------------------------------*/
bool_t TestLogOk(const char *szPrompt)
{
  return FlyTestGetYesNo(szPrompt);
}

/*!------------------------------------------------------------------------------------------------
  Test FlyLogDump()

  @ingroup    ned_test
  @returns    none
-------------------------------------------------------------------------------------------------*/
void TcLogDump(void)
{
  char szString[] = "Hello World is the classic phrase. How does this look in the dump world?\n";
  char szStringDbg[] = "Dbg: Hello World is the classic phrase. How does this look in the dump world?\n";

  FlyTestBegin();

  FlyLogHexDump(szString, sizeof(szString), 16, 2);
  if(!TestLogOk("in the log file, is dump 16 characters, indented by 2?"))
    FlyTestFailed();

  FlyLogHexDump(szString, sizeof(szString), 24, 4);
  if(!TestLogOk("is dump 24 characters, indented by 4?"))
    FlyTestFailed();

  FlyLogHexDump(szStringDbg, sizeof(szStringDbg), 8, 0);
  if(!TestLogOk("is Dbg: dump 8 characters, not indented?"))
    FlyTestFailed();

  FlyTestEnd();
}

/*!------------------------------------------------------------------------------------------------
  Test FlyLogPrintf()

  @ingroup    ned_test
  @returns    none
-------------------------------------------------------------------------------------------------*/
void TcLogPrintf(void)
{
  FlyTestBegin();

  if(FlyLogPrintf("Hello %d %s\n", 3, "Worlds") != 15)
    FlyTestFailed();

  if(!TestLogOk("Did it print Hello 3 Worlds?"))
    FlyTestFailed();

  FlyLogPrintf("Date/time: %s\n", FlyStrDateTimeCur());
  if(!TestLogOk("Did it print the current date/time?"))
    FlyTestFailed();

  FlyTestEnd();
}

/*!------------------------------------------------------------------------------------------------
  Test FlyLogPrintfEx()

  @ingroup    ned_test
  @returns    none
-------------------------------------------------------------------------------------------------*/
void TcLogPrintfEx(void)
{
  flyLogMask_t  oldMask;
  flyLogMask_t  newMask;
  #define TESTLOG_CMD     1
  #define TESTLOG_VIEW    2
  #define TESTLOG_WIN     4
  #define TESTLOG_EDITOR  0x8000
  #define TESTLOG_ALL     (TESTLOG_CMD | TESTLOG_VIEW | TESTLOG_WIN | TESTLOG_EDITOR)

  FlyTestBegin();

  oldMask = FlyLogMaskSet(TESTLOG_CMD | TESTLOG_VIEW);
  if(oldMask != 0)
    FlyTestFailed();
  if(FlyLogMaskGet() != (TESTLOG_CMD | TESTLOG_VIEW))
    FlyTestFailed();

  FlyLogPrintfEx(TESTLOG_CMD, "A: should print TESTLOG_CMD\n");
  FlyLogPrintfEx(TESTLOG_WIN, "B: should NOT print TESTLOG_WIN\n");
  FlyLogPrintfEx(TESTLOG_VIEW, "C: should print TESTLOG_VIEW\n");
  FlyLogPrintfEx(TESTLOG_EDITOR | TESTLOG_VIEW, "D: should print TESTLOG_EDITOR | TESTLOG_VIEW\n");
  FlyLogPrintfEx(TESTLOG_ALL, "E: should print %s\n", "TESTLOG_ALL");
  if(!TestLogOk("Did it print A,C,D,E but not B?"))
    FlyTestFailed();

  newMask = FlyLogMaskSet(oldMask);
  if(newMask != (TESTLOG_CMD | TESTLOG_VIEW))
    FlyTestFailed();

  FlyTestEnd();  
}

/*!------------------------------------------------------------------------------------------------
  Test FlyLogSize()

  @ingroup    ned_test
  @returns    none
-------------------------------------------------------------------------------------------------*/
void TcLogSize(void)
{
  const char szHello[] = "Hello";
  const char szWorld[] = "World\n";
  size_t    len1;
  size_t    len2;
  #define   TESTLOG_MSG   1

  FlyTestBegin();

  FlyLogSizeReset();
  FlyLogPrintf("Hello");
  FlyLogMaskSet(TESTLOG_MSG);
  FlyLogPrintfEx(TESTLOG_MSG, "World\n");
  len1 = FlyLogSizeGet();
  len2 = strlen(szHello) + strlen(szWorld);
  FlyLogPrintf("FlyLogSizeGet() %zu, strlen strings %zu\n", len1, len2);
  if(len1 != len2)
    FlyTestFailed();

  FlyTestEnd();  
}

/*-------------------------------------------------------------------------------------------------
  Test each function
-------------------------------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
  const char          szName[] = "test_log";
  const sTestCase_t   aTestCases[] =
  { 
    { "TcLogDump",      TcLogDump,      "M" },
    { "TcLogPrintf",    TcLogPrintf,    "M" },
    { "TcLogPrintfEx",  TcLogPrintfEx,  "M" },
    { "TcLogSize",      TcLogSize }
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
