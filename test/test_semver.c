/**************************************************************************************************
  test_semver.c
  Copyright 2024 Drew Gislason
  License: MIT <https://mit-license.org>
**************************************************************************************************/
#include "FlyTest.h"
#include "FlySemVer.h"

/*-------------------------------------------------------------------------------------------------
  Test FlySemVerCmp()
-------------------------------------------------------------------------------------------------*/
void TcSemVerCmp(void)
{
  typedef struct
  {
    const char *szVer1;
    int         ret;
    const char *szVer2;
  } testVer_t;

  static const testVer_t aTests[] = { 
    { "2", 1, "1.1.15" },
    { "1.1.14", -1, "1.1.15" },
    { "*", 0, "1.1.15" },
    { "1.1.15", 0, "*" },
    { "1.1", 1, "1" },
    { "1.2.3", 0, "1.2.3" },
  };
  int   i;   
  int   ret;

  FlyTestBegin();

  for(i = 0; i < NumElements(aTests); ++i)
  {
    if(FlyTestVerbose())
    {
      if(i == 0)
        FlyTestPrintf("\n");
      FlyTestPrintf("%d: %s, %d, %s\n", i, aTests[i].szVer1, aTests[i].ret, aTests[i].szVer2);
    }
    ret = FlySemVerCmp(aTests[i].szVer1, aTests[i].szVer2);
    if(ret != aTests[i].ret)
    {
      FlyTestPrintf("Bad result FlySemVerCmp(%s, %s) %d, expected %d\n",
        aTests[i].szVer1, aTests[i].szVer2, ret, aTests[i].ret);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlySemVerMatch()
-------------------------------------------------------------------------------------------------*/
void TcSemVerMatch(void)
{
  typedef struct
  {
    const char *szVerRange;
    bool_t      ret;
    const char *szVer;
  } testMatch_t;

  static const testMatch_t aTests[] = 
  {
    { "1.2.3", TRUE,  "1.2.3" },
    { "1.2.3", TRUE,  "1.99.99" },
    { "1.2.3", FALSE, "2.0" },
    { "1.2.3", FALSE, "0.99.999" },
    { "1.2",   TRUE,  "1.2" },
    { "1.2",   TRUE,  "1.2.3" },
    { "1.2",   FALSE, "1.1.99" },
    { "1",     TRUE,  "1" },
    { "1",     TRUE,  "1.2" },
    { "1",     TRUE,  "1.2.3" },
    { "1",     FALSE, "9.9.999" },
    { "1",     FALSE, "0" },
    { "1",     FALSE, "2" },
    { "0.2.3", TRUE,  "0.2.3" },
    { "0.2.3", FALSE, "0.2.99" },
    { "0.2.3", FALSE, "0.3" },
    { "0.2.3", FALSE, "0.2.2" },
    { "0.2",   TRUE,  "0.2.0" },
    { "0.2",   TRUE,  "0.2.99" },
    { "0.2",   FALSE, "0.3.0" },
    { "0.2",   FALSE, "0.1" },
    { "2.1",   TRUE,  "2.2" },
    { "2.1",   FALSE, "2.0" },
    { "2.1",   FALSE, "3" },
    { "2.1",   FALSE, "1.0.0" },
    { "0.0.3", TRUE,  "0.0.3" },
    { "0.0.3", TRUE,  "0.0.3.alpha" },
    { "0.0.3", FALSE, "0.1" },
    { "0.0.3", FALSE, "0.0.4" },
    { "0.0.3", FALSE, "0.0.2" },
    { "1.2.3", TRUE,  "*" },
    { "2",     TRUE,  "*" },
    { "0.0.1", TRUE,  "*" },
    { "*",     TRUE,  "1.2.3" },
    { "*",     TRUE,  "9.0" },
    { "*",     TRUE,  "0" },
    { "1.2",   TRUE,  "*" },
    { "99999.99999.99999", TRUE,  "99999.99999.99999" },
  };
  int     i;
  bool_t  ret;

  FlyTestBegin();

  for(i = 0; i < NumElements(aTests); ++i)
  {
    if(FlyTestVerbose())
    {
      if(i == 0)
        FlyTestPrintf("\n");
      FlyTestPrintf("%d: FlySemVerMatch(szRange=%s, szVer=%s) should return %s\n", i,
        FlyStrNullOk(aTests[i].szVerRange), FlyStrNullOk(aTests[i].szVer), aTests[i].ret ? "TRUE" : "FALSE");
    }
    ret = FlySemVerMatch(aTests[i].szVerRange, aTests[i].szVer);
    if(ret != aTests[i].ret)
    {
      FlyTestPrintf("Bad result FlySemVerMatch(%s, %s) %d, expected %d\n",
        aTests[i].szVerRange, aTests[i].szVer, ret, aTests[i].ret);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlySemVerIsValid()
-------------------------------------------------------------------------------------------------*/
void TcSemVerIsValid(void)
{
  typedef struct
  {
    const char *szVer;
    bool_t      fIsValid;
  } testIsValid_t;

  static const testIsValid_t aTests[] = 
  {
    { "*", TRUE },
    { "1", TRUE },
    { "1.2", TRUE },
    { "1.2.3", TRUE },
    { "1.2.3.alpha", TRUE },
    { "12345.67890.12345", TRUE },
    { "99999.67890.12345.beta", TRUE },
    { "zeta", FALSE },
    { "2.zeta", FALSE },
  };
  unsigned    i;
  bool_t      fIsValid;

  FlyTestBegin();

  for(i = 0; i < NumElements(aTests); ++i)
  {
    fIsValid = FlySemVerIsValid(aTests[i].szVer);
    if(fIsValid != aTests[i].fIsValid)
    {
      FlyTestPrintf("Bad result FlySemVerIsValid(%s) got %d, expected %d\n",
        aTests[i].szVer, fIsValid, aTests[i].fIsValid);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlySemVerHigh()
-------------------------------------------------------------------------------------------------*/
void TcSemVerHigh(void)
{
  typedef struct
  {
    const char *szVer;
    const char *szHigh;
  } testSemVerHigh_t;

  static const testSemVerHigh_t aTests[] = 
  {
    { "*", "*" },
    { "1", "2.0.0" },
    { "1.2", "2.0.0" },
    { "1.2.3", "2.0.0" },
    { "3.2.1", "4.0.0" },
    { "0.2", "0.3.0" },
    { "0.2.1", "0.2.2" },
    { "0", "1.0.0" },
  };
  unsigned    i;
  char        szHigh[(5 * 3) + 4];   // allow 5 digit ints for major.minor.patch

  FlyTestBegin();

  for(i = 0; i < NumElements(aTests); ++i)
  {
    FlySemVerHigh(szHigh, aTests[i].szVer, sizeof(szHigh));
    if(strcmp(szHigh, aTests[i].szHigh) != 0)
    {
      FlyTestPrintf("Bad result FlySemVerHigh(%s) got %s, expected %s\n",
        aTests[i].szVer, szHigh, aTests[i].szHigh);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlySemVerCpy()
-------------------------------------------------------------------------------------------------*/
void TcSemVerCpy(void)
{
  typedef struct
  {
    const char *sz;
    const char *szExp;
  } testSemVerCpy_t;

  static const testSemVerCpy_t aTests[] = 
  {
    { "*", "*" },
    { "1  abc", "1" },
    { "1.2 version", "1.2" },
    { "123.456 more text", "123.456" },
    { "1.2.3 and more", "1.2.3" },
    { "3.22.99234.alpha is done", "3.22.99234.alpha" },
    { " 1.2.3 ", "" },
    { "v0.2", "" },
    { "1.a", "" },
    { "1..2", "" },
  };
  unsigned    i;
  unsigned    len;
  char        sz[32];

  FlyTestBegin();

  for(i = 0; i < NumElements(aTests); ++i)
  {
    strcpy(sz, "aaa");
    len = FlySemVerCpy(sz, aTests[i].sz, sizeof(sz));
    if(len != strlen(aTests[i].szExp))
    {
      FlyTestPrintf("%u: `%s`, got len %u, exp %u\n", i, aTests[i].sz, len, (unsigned)strlen(aTests[i].szExp));
      FlyTestFailed();
    }
    if(len && strcmp(sz, aTests[i].szExp) != 0)
    {
      FlyTestPrintf("%u: `%s`, got `%s`, exp `%s`\n", i, aTests[i].sz, sz, aTests[i].szExp);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test each function
-------------------------------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
  const char          szName[] = "test_semver";
  const sTestCase_t   aTestCases[] =
  {
    { "TcSemVerCmp",      TcSemVerCmp },
    { "TcSemVerMatch",    TcSemVerMatch },
    { "TcSemVerIsValid",  TcSemVerIsValid },
    { "TcSemVerHigh",     TcSemVerHigh },
    { "TcSemVerCpy",      TcSemVerCpy },
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
