/*!************************************************************************************************
  @file FFLyTestAnsi.c

  @brief  ANSI Unit Test

  Copyright 2022 Drew Gislason

  @ingroup test

*///***********************************************************************************************
#include "FlyAnsi.h"
#include "FlyTest.h"

/*!------------------------------------------------------------------------------------------------
  Test location

  @param    szFilePath    standard printf format
  @return   length of printed string
*///-----------------------------------------------------------------------------------------------
void FlyTestAnsiShowLocation(void)
{
  char      s[40];
  unsigned  len;
  unsigned  rows;
  unsigned  cols;
  unsigned  i;
  unsigned  val;

  FlyTestBegin();

  AnsiSetAttr(FLYBACK_NIGHT | FLYATTR_WHITE | FLYATTR_BOLD);
  AnsiGetRowsCols(&rows, &cols);
  AnsiClearScreen();

  AnsiGoto(0, 0);
  printf("0,0");
  AnsiGoto(1, 0);
  printf("FlyTestAnsiShowLocation");
  AnsiGoto(2, 3);
  printf("2, 3");
  len = snprintf(s, sizeof(s), "%u,%u", rows, cols);
  AnsiGoto(rows-2, (cols-10) - len);
  printf("%s",s);
  AnsiGoto(rows-1, (cols-1) - len);
  printf("%s",s);

  AnsiGoto(rows/2, 0);
  val = 0;
  for(i=0; i<cols; ++i)
  {
    printf("%u",val);
    ++val;
    if(val >= 10)
      val = 0;
  }

  AnsiGoto(rows-1, 0);

  printf("Rows %d, Cols %d. ", rows, cols);
  if(!FlyTestPassFail())
    FlyTestFailed();

  FlyTestEnd();
}

void FlyTestAnsiAllColors(void)
{
  unsigned  rows;
  unsigned  cols;

  FlyTestBegin();

  AnsiSetAttr(FLYATTR_RESET);
  AnsiClearScreen();
  AnsiShowAllColors();
  AnsiGetRowsCols(&rows, &cols);
  AnsiSetAttr(FLYATTR_RESET);

  printf("Rows %d, Cols %d. Total chars %zu\n", rows, cols, AnsiCharCount());
  if(!FlyTestPassFail())
    FlyTestFailed();

  FlyTestEnd();
}

int main(int argc, const char *argv[])
{
  const char          szName[] = "test_ansi";
  const sTestCase_t   aTestCases[] =
  { 
    { "FlyTestAnsiShowLocation",  FlyTestAnsiShowLocation, "M LOG_ONLY" },
    { "FlyTestAnsiAllColors", FlyTestAnsiAllColors, "M LOG_ONLY" }
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

