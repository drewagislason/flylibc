/**************************************************************************************************
  FltTest.h
  Copyright 2024 Drew Gislason
  license: MIT <https://mit-license.org>
*///***********************************************************************************************
#ifndef FLY_TEST_H
#define FLY_TEST_H
#include "FlyAnsi.h"
#include "FlyLog.h"

// uses stack for FlyTestPrintf() lines
#ifndef FLYTEST_CFG_LINE_MAX
 #define FLYTEST_CFG_LINE_MAX 1024
#endif

#define TESTATTR_PASSED  FLYATTR_GREEN
#define TESTATTR_FAILED  FLYATTR_RED
#define TESTATTR_SKIPPED FLYATTR_YELLOW
#define TESTATTR_NORMAL  FLYATTR_RESET


#define TEST_EXIT_PASS    0
#define TEST_EXIT_FAIL    1
#define TEST_EXIT_SKIP    2

#define TEST_TAG_MANUAL   "M"
#define TEST_TAG_LOG_ONLY "LOG_ONLY"
#define TEST_TAG_MAX_LEN  64

typedef void * hTestSuite_t;
typedef void (*pfnTestCase_t)(void);
typedef struct
{
  const char     *szName;       // used for filtering
  pfnTestCase_t   pfnTestCase;  // order of test cases
  const char     *szTags;       // space delimited tags
} sTestCase_t;

// INTERNAL: DO NOT USE
void T_Failed  (const char *szExpr, const char *szFile, unsigned line);
void T_Skipped (void);
void T_Passed  (void);
void T_Begin   (void);

// API
void          FlyTestInit           (const char *szNameVer, int argc, const char *argv[]);
hTestSuite_t  FlyTestNew            (const char *szSuiteName, unsigned numCases, const sTestCase_t *pTestCases);
void          FlyTestFree           (hTestSuite_t hSuite);
void          FlyTestRun            (hTestSuite_t hSuite);
int           FlyTestSummary        (hTestSuite_t hSuite);
void          FlyTestStop           (hTestSuite_t hSuite);
#define       FlyTestBegin()        bool_t fNt_fPassed = TRUE; T_Begin();
#define       FlyTestEnd()          TestEnd: if(fNt_fPassed) T_Passed();
#define       FlyTestFailed()       { T_Failed("", __FILE__,__LINE__); fNt_fPassed = FALSE; goto TestEnd; }
#define       FlyTestAssert(expr)   if(!(expr)) { T_Failed(#expr, __FILE__, __LINE__); fNt_fPassed = FALSE; goto TestEnd; }
#define       FlyTestSkipped()      { fNt_fPassed = FALSE; T_Skipped(); goto TestEnd; }
#define       FlyTestStubbed()      { T_Begin(); T_Skipped(); }
#define       FlyTestPassed()       { fNt_fPassed = TRUE; goto TestEnd; }
int           FlyTestPrintf         (const char *szFormat, ...);
int           FlyTestErrorPrint     (const char *szExpr, const char *szFile, const char *szFunc, unsigned line);
void          FlyTestDump           (const void *pData, unsigned len);
void          FlyTestDumpCmp        (const void *pThis, const void *pThat, size_t len);
unsigned      FlyTestVerbose        (void);
bool_t        FlyTestAutomated      (void);
void          FlyTestVerboseSet     (unsigned verboseLevel);
bool_t        FlyTestPassFail       (void);
unsigned      FlyTestRandRange      (unsigned low, unsigned high);
unsigned      FlyTestGetAnswer      (const char *szPrompt, const char *szChoices);
bool_t        FlyTestGetYesNo       (const char *szPrompt);
bool_t        FlyTestCalcLogTotals  (unsigned *pPassed, unsigned *pFailed, unsigned *pSkipped);
bool_t        FlyTestTagExists      (const char *szTagList, const char *szTag);

// for machine parsing of log file
extern const char m_szColumn[];
extern const char m_szMTestStart[];     // "$TEST_START"
extern const char m_szMTestPassed[];    // "$PASSED";
extern const char m_szMTestFailed[];    // "$FAILED";
extern const char m_szMTestSkipped[];   // "$SKIPPED";
extern const char m_szRetCodeFile[];    // "tmp_retcode.txt"

#endif // FLY_TEST_H
