/**************************************************************************************************
  FlyTest.c - Unit test your own C code, build test cases and suites
  Copyright 2024 Drew Gislason
  License: MIT <https://mit-license.org>
*///***********************************************************************************************
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include "FlyTest.h"
#include "FlyMem.h"
#include "FlyStr.h"
#include "FlyFile.h"
#include "FlyCli.h"

/*!
  @defgroup FlyTest   A C Test Framework/API for testing C functions and objects

  This is a simple but powerful test framework for C programs. Allows any C module to be tested
  independant of the overall system, thereby reducing bugs in the final code.

  Features:

  1. Easily run manual and automated tests
  2. Run "smoke" tests for rapid verification of new code for CI/CD
  3. Verbosity flags for better context and understanding of issues
  4. Filter which tests are run by keyword
  5. Automatically creates formal test logs for records / searching
  6. Test case, test group, test suite framework
  7. TestPrintf() can aid debugging and is compiled out of production code
  8. Records time consumed by test cases and suites
  9. Logs data to test.log (or chosen log file) for further analysis 

  The test framework generally is built to test modules (that is, a C collection of functions or a
  C "object"). This is comprised of 1 or more test cases.

      Usage = TestSuite [test_suite_args] [-a] [-f filter] [-m] [-v]

  Nomenclature. See also <https://www.istqb.org>

  * test case - the basic unit of test
  * test suite - a set of test cases to test a module or the entire program
  * unit-test - test code verifies private functions not exposed via API
  * smoke test - quick test cases used for CI/CD
  * fuzz tests - throws bad input at functions to verify how they handle invalid inputs
*/

typedef enum
{
  RESULT_SKIPPED = 0,
  RESULT_PASSED,
  RESULT_FAILED
} testResult_t;

typedef struct
{
  const char         *szSuiteName;
  unsigned            numCases;
  unsigned            curCase;
  const sTestCase_t  *pTestCases;
  testResult_t       *aResult;
  unsigned            passCount;
  unsigned            failCount;
  unsigned            skipCount;
  bool_t              fStopped;
} sTestSuite_t;

static void               T_Color  (unsigned iColor);
static void               T_Line   (void);
static const sTestCase_t *T_Case   (void);

static sTestSuite_t  *m_pSuite;
static unsigned       m_verbose;          // -v[#]
static bool_t         m_fAutomatedOnly;   // -a
static bool_t         m_fList;            // -l
static const char    *m_szFilter;         // -f filter
static const char    *m_szTags;           // -t tags
const char            m_szColumn[]  = "%-40s";

// for machine readable parsing
const char m_szMTestSuiteName[] = ":TEST_SUITE_NAME:";
const char m_szMTestStart[]     = ":TEST_START:";
const char m_szMTestEnd[]       = ":TEST_END:";
const char m_szMTestPassed[]    = ":PASSED:";
const char m_szMTestFailed[]    = ":FAILED:";
const char m_szMTestSkipped[]   = ":SKIPPED:";
// const char m_szRetCodeFile[]    = "tmp_retcode.txt";
const char m_szTagManual[]      = TEST_TAG_MANUAL;
const char m_szTagLogOnly[]     = TEST_TAG_LOG_ONLY;

/*---------------------------------------------------------------------------
  Process command-line and Display header info into log file
---------------------------------------------------------------------------*/
void FlyTestInit(const char *szNameVer, int argc, const char *argv[])
{
  const char szBanner[] =
  {
    "\n\n"
    "=======  -----  -----   ---  -----  =======\n"
    "_______    |    |__    /       |    _______\n"
    "           |    |      ----    |           \n"
    "=======    |    |____  ___/    |    =======\n"
    "\n\n"
  };
  const flyCliOpt_t   cliOpts[] =
  {
    { "-a",           &m_fAutomatedOnly,    FLYCLI_BOOL },
    { "-f",           &m_szFilter,          FLYCLI_STRING },
    { "-l",           &m_fList,             FLYCLI_BOOL },
    { "-t",           &m_szTags,            FLYCLI_STRING },
    { "-v",           &m_verbose,           FLYCLI_INT },
  };
  flyCli_t cli =
  {
    .pArgc      = &argc,
    .argv       = argv,
    .nOpts      = NumElements(cliOpts),
    .pOpts      = cliOpts,
    .szVersion  = szNameVer,
    .szHelp     = "Usage = [-a] [-f filter] [-l] [-m] [-t tagfilter] [-v] [test suite args]\n"
    "\n"
    "Options:\n"
    "-a             Automated tests only (no manual tests)\n"
    "-f \"filter\"    Filter based on substring test case names\n"
    "-l             List test cases in this suite then exit\n"
    "-m             Machine readable log results, used when a test suite calls on other test suites\n"
    "-t \"tags\"      Filter by tags. For a list of tags for test cases, use option -l (list)\n"
    "-v[=#]         Verbose level: -v- (none: default), -v (some), -v=2 (more)\n"
  };

  int       i;
  char     *psz;
  bool_t    fGotIt;

  m_pSuite            = NULL;
  m_verbose           = 0;
  m_fAutomatedOnly    = FALSE;
  m_szFilter          = NULL;
  m_szTags            = NULL;
  m_fList             = FALSE;
  if(FlyCliParse(&cli) != FLYCLI_ERR_NONE)
    exit(1);

  // set up assert to print to log and screen
  FlyAssertSetExit(FlyTestErrorPrint);

  // banner shows C compiler info, SHA, etc...
  // display banner and basic info
  FlyLogPrintf("%sc version %lu, datetime of test run: %s\nargc %d argv { ", szBanner, __STDC_VERSION__,
               FlyStrDateTimeCur(), argc);
  for(i=0; i<argc; ++i)
    FlyLogPrintf("%s ",argv[i]);
  FlyLogPrintf("}\n");

  // display Git SHA version
  fGotIt = FALSE;
  FlyLogPrintf("\nGit SHA: ");
  if(system("git log -n 1 >test.tmp") == 0)
  {
    psz = FlyFileRead("test.tmp");
    if(psz)
    {
      fGotIt = TRUE;
      FlyLogPrintf("%s\n", psz);
      FlyFree(psz);
    }
  }
  if(!fGotIt)
    FlyLogPrintf("SHA not found! Perhaps Git not installed?\n");
  remove("test.tmp");

  // display gcc version
  fGotIt = FALSE;
  FlyLogPrintf("\nC Version Info: ");
  if(system("cc --version >test.tmp 2>>test.tmp") == 0)
  {
    psz = FlyFileRead("test.tmp");
    if(psz)
    {
      fGotIt = TRUE;
      FlyLogPrintf("%s\n\n", psz);
      FlyFree(psz);
    }
  }
  if(!fGotIt)
    FlyLogPrintf("C Version not found! Perhaps cc not installed?\n\n");
  remove("test.tmp");

  FlyTestPrintf("Options: automated %u, verbose %u, filter: %s, tags: %s\n", m_fAutomatedOnly,
        m_verbose, m_szFilter ? m_szFilter : "(none)", m_szTags ? m_szTags : "(none)");
}

/*!------------------------------------------------------------------------------------------------
  Look for the given tag in the szTags string. Case sensitive.

  @param    szTags    a string with one or more tags (haystack), may be NULL
  @param    szTag     the tag to look for in szTags (needle)
  @returns  TRUE if tag found, FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlyTestTagExists(const char *szTags, const char *szTag)
{
  const char   *psz;
  unsigned      tagLen;
  bool_t        fExists = FALSE;


  if(szTags)
  {
    psz = FlyStrSkipWhite(szTags);
    szTag = FlyStrSkipWhite(szTag);
    tagLen = strlen(szTag);
    while(tagLen && *psz)
    {
      if((FlyStrArgLen(psz) == tagLen) && (strncmp(psz, szTag, tagLen) == 0))
      {
        fExists = TRUE;
        break;
      }
      psz = FlyStrArgNext(psz);
    }
  }

  return fExists;
}

/*!------------------------------------------------------------------------------------------------
  Does the test case have all the tags the user is looking for?

  @param    szTcTags    Test Case Tags (may be NULL)
  @param    szUserTags  Space separated tags user is looking for
  @returns  TRUE if all user tags found in test case, FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlyTestTagExistsAll(const char *szTcTags, const char *szUserTags)
{
  const char *pszArg;
  char        szTag[TEST_TAG_MAX_LEN];
  unsigned    argLen;
  bool_t      fExists = FALSE;

  if(szTcTags)
  {
    pszArg = FlyStrSkipWhite(szUserTags);

    // there must be at least 1 user tag and all user tags must exist, or returns FALSE
    while(*pszArg)
    {
      argLen = FlyStrArgLen(pszArg);
      if(argLen)
      {
        FlyStrZNCpy(szTag, pszArg, sizeof(szTag), argLen);
        if(FlyTestTagExists(szTcTags, szTag))
          fExists = TRUE;
        else
        {
          fExists = FALSE;
          break;
        }
      }
      pszArg = FlyStrArgNext(pszArg);
    }
  }

  return fExists;
}

/*!------------------------------------------------------------------------------------------------
  Create a test suite from test cases

  @param    szSuiteName     ptr to persistent string
  @param    numCases        number of test cases
  @param    pTestCases      pointer to peristent array of test cases

  @returns  length of printed string
*///-----------------------------------------------------------------------------------------------
hTestSuite_t FlyTestNew(const char *szSuiteName, unsigned numCases, const sTestCase_t *pTestCases)
{
  sTestSuite_t   *pSuite;
  unsigned        i;

  pSuite = FlyAlloc(sizeof(sTestSuite_t));
  if(pSuite)
  {
    memset(pSuite, 0, sizeof(sTestSuite_t));
    pSuite->szSuiteName = szSuiteName;
    pSuite->numCases    = numCases;
    pSuite->pTestCases  = pTestCases;
    if(numCases)
      pSuite->aResult = FlyAlloc(numCases * sizeof(testResult_t));

    if(m_fList)
    {
      printf("%-42sTags\n", szSuiteName);
      for(i = 0; i < numCases; ++i)
        printf("  %-40s%s\n", pTestCases[i].szName, pTestCases[i].szTags ? pTestCases[i].szTags : "");
      exit(1);
    }
  }

  return (hTestSuite_t)pSuite;
}

/*!------------------------------------------------------------------------------------------------
  Free the test suite

  @param    hSuite          handle to test suite
  @returns  none
*///-----------------------------------------------------------------------------------------------
void FlyTestFree(hTestSuite_t hSuite)
{
  if(hSuite)
  {
    if(hSuite == m_pSuite)
      m_pSuite = NULL;
    FlyFree(hSuite);
  }
}

/*!------------------------------------------------------------------------------------------------
  Free the test suite

  @param    hSuite          handle to test suite
  @returns  none
*///-----------------------------------------------------------------------------------------------
void FlyTestRun(hTestSuite_t hSuite)
{
  sTestSuite_t   *pSuite = hSuite;
  bool_t          fSkipped;

  FlyAssert(hSuite);

  // start of test suite
  FlyLogPrintf("\n\n%s%s\n", m_szMTestSuiteName, pSuite->szSuiteName);
  FlyLogPrintf("%s %s\n\n", m_szMTestStart, FlyStrDateTimeCur());

  m_pSuite              = pSuite;
  pSuite->passCount     = 0;
  pSuite->failCount     = 0;
  pSuite->skipCount     = 0;
  pSuite->fStopped      = FALSE;
  for(pSuite->curCase = 0; pSuite->curCase < pSuite->numCases; ++pSuite->curCase)
  {
    fSkipped = FALSE;
    if(m_szFilter && (strstr(T_Case()->szName, m_szFilter) == 0))
      fSkipped  = TRUE;
    if(m_fAutomatedOnly && FlyTestTagExists(T_Case()->szTags, m_szTagManual))
      fSkipped  = TRUE;
    if(m_szTags && !FlyTestTagExistsAll(T_Case()->szTags, m_szTags))
      fSkipped  = TRUE;

    // run the case (or indicated skipped)
    if(fSkipped)
    {
      T_Begin();
      T_Skipped();
    }
    else if(T_Case()->pfnTestCase)
    {
      (T_Case()->pfnTestCase)();
    }

    if(pSuite->fStopped)
      break;
  }

  // no longer in test suite
  FlyLogPrintf("%s %s\n\n", m_szMTestEnd), FlyStrDateTimeCur();
  m_pSuite = NULL;
}

/*!------------------------------------------------------------------------------------------------
  Stop the test run. Used by test cases that know continuing would be bad

  @param    hSuite    valid test suite
  @returns  TRUE if verbose, FALSE if not
*///-----------------------------------------------------------------------------------------------
void FlyTestStop(hTestSuite_t hSuite)
{
  sTestSuite_t   *pSuite = hSuite;
  pSuite->fStopped = TRUE;
}

/*!------------------------------------------------------------------------------------------------
  Summarize the test in both log and on-screen. Assumes any full-screen stuff is done.

  @param    hSuite    valid test suite
  @return   0 if all tests passed (or were skipped), 1 if any tests failed
*///-----------------------------------------------------------------------------------------------
int FlyTestSummary(hTestSuite_t hSuite)
{
  sTestSuite_t   *pSuite = hSuite;
  int             ret;

  m_pSuite = pSuite;

  T_Color(TESTATTR_NORMAL);
  printf("\nSUITE SUMMARY: %s -- opts: %s%sfilter %s\n", pSuite->szSuiteName, m_verbose ? "verbose " : "",
    m_fAutomatedOnly ? "automated only " : "", m_szFilter ? m_szFilter : "(none)");
  FlyLogPrintf("\nSUITE SUMMARY: %s -- opts: %s%sfilter %s\n", pSuite->szSuiteName, m_verbose ? "verbose " : "",
    m_fAutomatedOnly ? "automated only " : "", m_szFilter ? m_szFilter : "(none)");
  T_Line();

  // summarize to screen
  T_Color(TESTATTR_PASSED);
  printf(m_szColumn, "Passed");
  printf("%u\n",pSuite->passCount);
  T_Color(TESTATTR_FAILED);
  printf(m_szColumn, "Failed");
  printf("%u\n", pSuite->failCount);
  T_Color(TESTATTR_SKIPPED);
  printf(m_szColumn, "Skipped");
  printf("%u\n", pSuite->skipCount);
  T_Color(TESTATTR_NORMAL);

  // summarize to log
  FlyLogPrintf(m_szColumn, "Passed");
  FlyLogPrintf("%u\n", pSuite->passCount);
  FlyLogPrintf(m_szColumn, "Failed");
  FlyLogPrintf("%u\n", pSuite->failCount);
  FlyLogPrintf(m_szColumn, "Skipped");
  FlyLogPrintf("%u\n", pSuite->skipCount);

  T_Line();

  // return 0=pass, 1=fail, 2=skipped
  // if any failures, that takes precedence
  // if some skipped and some passed, that is still a pass
  // if all skipped, then skipped
  if(pSuite->failCount > 0)
    ret = TEST_EXIT_FAIL;
  else
    ret = TEST_EXIT_PASS;

  return ret;
}

/*!------------------------------------------------------------------------------------------------
  Small banner for log, shows test case name and result

  @returns  TRUE if verbose, FALSE if not
*///-----------------------------------------------------------------------------------------------
void T_End(unsigned attr, const char *szResult)
{
  static const char     m_szEndLine[] = "----- %s(%s) -----\n";

  // Passed, Failed, Skipped
  if(!FlyTestTagExists(T_Case()->szTags, m_szTagLogOnly))
  {
    T_Color(attr);
    printf("%s\n", szResult);
  }

  if(strcmp(szResult, "Passed") == 0)
    szResult = m_szMTestPassed;
  if(strcmp(szResult, "Skipped") == 0)
    szResult = m_szMTestSkipped;

  FlyLogPrintf(m_szEndLine, T_Case()->szName, szResult);
}

/*-------------------------------------------------------------------------------------------------
  Internal function to indicate failed with small end banner
-------------------------------------------------------------------------------------------------*/
void T_Failed(const char *szExpr, const char *szFile, unsigned line)
{
  static const char     m_szEndLine[] = "----- %s(%s) -----\n";

  ++m_pSuite->failCount;
  m_pSuite->aResult[m_pSuite->curCase] = RESULT_FAILED;
  FlyLogPrintf("%s:%u:1: failed %s: %s\n", szFile, line, T_Case()->szName, szExpr);

  // display to screen in color
  if(!FlyTestTagExists(T_Case()->szTags, m_szTagLogOnly))
  {
    T_Color(TESTATTR_FAILED);
    printf("%s:%u:1: %s\n", szFile, line, "Failed");
  }

  FlyLogPrintf(m_szEndLine, T_Case()->szName, m_szMTestFailed);
}

/*-------------------------------------------------------------------------------------------------
  Internal function to indicate skipped with small end banner
-------------------------------------------------------------------------------------------------*/
void T_Skipped(void)
{
  ++m_pSuite->skipCount;
  m_pSuite->aResult[m_pSuite->curCase] = RESULT_SKIPPED;
  T_End(TESTATTR_SKIPPED, "Skipped");
}

/*-------------------------------------------------------------------------------------------------
  Internal function to indicate passed with small end banner
-------------------------------------------------------------------------------------------------*/
void T_Passed(void)
{
  ++m_pSuite->passCount;
  m_pSuite->aResult[m_pSuite->curCase] = RESULT_PASSED;
  T_End(TESTATTR_PASSED, "Passed");
}

/*!------------------------------------------------------------------------------------------------
  Indicates the name of the test with small banner

  @returns  TRUE if verbose, FALSE if not
*///-----------------------------------------------------------------------------------------------
void T_Begin(void)
{
  // to screen
  if(!FlyTestTagExists(T_Case()->szTags, m_szTagLogOnly))
  {
    T_Color(TESTATTR_NORMAL);
    printf(m_szColumn, T_Case()->szName);
  }

  // log to debug.log
  FlyLogPrintf("----- :TEST_CASE:%s -----\n", T_Case()->szName);
}

/*!------------------------------------------------------------------------------------------------
  Print to screen and the log. Doesn't print to screen if TC_SCREEN flag is set for this test

  @param    szFormat    constant format string
  @param    ...         optional variable number of parameters
  @returns  length of printed string
*///-----------------------------------------------------------------------------------------------
int FlyTestPrintf(const char *szFormat, ...)
{
  char        szLine[FLYTEST_CFG_LINE_MAX];
  va_list     arglist;
  int         len;

  // print, and clear to end of line
  va_start(arglist, szFormat);
  len = vsnprintf(szLine, sizeof(szLine) - 1, szFormat, arglist);
  szLine[sizeof(szLine) - 1] = '\0';
  va_end(arglist);

  FlyLogPrintf("%s", szLine);
  if((m_pSuite == NULL) || !FlyTestTagExists(T_Case()->szTags, m_szTagLogOnly))
    printf("%s", szLine);

  return len;
}

/*!------------------------------------------------------------------------------------------------
  Dump to screen and log

  @param    pData   data to dump
  @param    len     length of data
  @returns  none
*///-----------------------------------------------------------------------------------------------
void FlyTestDump(const void *pData, unsigned len)
{
  char            szLine[FlyStrDumpLineSize(FLYSTR_DUMP_COLS)];
  unsigned        thisLen;
  long            addr = 0;
  const uint8_t  *pByte = pData;

  while(len > 0)
  {
    if(len > FLYSTR_DUMP_COLS)
      thisLen = FLYSTR_DUMP_COLS;
    else
      thisLen = len;
    FlyStrDumpLine(szLine, pByte, thisLen, FLYSTR_DUMP_COLS, addr);
    FlyTestPrintf("%s\n", szLine);
    pByte += thisLen;
    len   -= thisLen;
    addr  += thisLen;
  }
}

/*!------------------------------------------------------------------------------------------------
  Dump to screen and log

  @param    pData   data to dump
  @param    len     length of data
  @returns  none
*///-----------------------------------------------------------------------------------------------
void FlyTestDumpCmp(const void *pThis, const void *pThat, size_t len)
{
  size_t  offset;
  FlyTestPrintf("\n--- this ---\n");
  FlyTestDump(pThis, len);
  FlyTestPrintf("\n--- that ---\n");
  FlyTestDump(pThat, len);
  offset = FlyMemDiff(pThis, pThat, len);
  if(offset == FLYMEM_NO_DIFF)
    FlyTestPrintf("\n--- this == that ---\n");
  else
    FlyTestPrintf("\n--- diff at offset %zx ---\n", offset);
}

/*!------------------------------------------------------------------------------------------------
  Run tests in automated mode only?

  @returns  TRUE if automated tests only, FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlyTestAutomated(void)
{
  return m_fAutomatedOnly;
}

/*!------------------------------------------------------------------------------------------------
  Current verbose level.

  @returns  verbose level (0-n)
*///-----------------------------------------------------------------------------------------------
unsigned FlyTestVerbose(void)
{
  return m_verbose;
}

/*!------------------------------------------------------------------------------------------------
  Turn verbose on or off

  @param    verboseLevel
  @returns  none
*///-----------------------------------------------------------------------------------------------
void FlyTestVerboseSet(unsigned verboseLevel)
{
  m_verbose = verboseLevel;
}

/*-------------------------------------------------------------------------------------------------
  Gets keyboard input using getchar() with 'p' for pass, 'f' for fail, Returns TRUE if passed,
  FALSE if not.

  @returns  TRUE if passed, FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlyTestPassFail(void)
{
  bool_t    fPassed = TRUE;
  char      sz[5];

  printf("Press <enter> for pass, 'f'<enter> for fail: ");
  *sz = '\0';
  fgets(sz, sizeof(sz)-1, stdin);
  if(*sz == 'f' || *sz == 'F')
    fPassed = FALSE;

  return fPassed;
}

/*!------------------------------------------------------------------------------------------------
  Return a random number.

  Seeds randomness with current time.

  @param      low    0 - UINT_MAX
  @param      high   low - UINT_MAX
  @returns    random number between low and high inclusive
*///-----------------------------------------------------------------------------------------------
unsigned FlyTestRandRange(unsigned low, unsigned high)
{
  static bool_t fInit = TRUE;

  if(fInit)
  {
    srand(time(0));
    fInit = FALSE;
  }
  return low + (unsigned)(random()%(high-low));
}

/*---------------------------------------------------------------------------
  Print a line
---------------------------------------------------------------------------*/
static void T_Line(void)
{
  char      szBuffer[51];

  memset(szBuffer, '-', sizeof(szBuffer));
  szBuffer[sizeof(szBuffer)-1] = '\0';

  printf("%s\n", szBuffer);
  FlyLogPrintf("%s\n", szBuffer);
}

/*---------------------------------------------------------------------------
  Set the color
---------------------------------------------------------------------------*/
static void T_Color(unsigned color)
{
  AnsiSetAttr((fflyAttr_t)color);
}

/*-------------------------------------------------------------------------------------------------
  Internal function: returns ptr to current suite suite case
-------------------------------------------------------------------------------------------------*/
static const sTestCase_t * T_Case(void)
{
  const sTestCase_t *pTestCase = NULL;

  FlyAssert(m_pSuite);
  pTestCase = &m_pSuite->pTestCases[m_pSuite->curCase];

  return pTestCase;
}


/*--------------------------------------------------------------------------------------------------
  Return count of substring needles found in large string haystack

  pszHaystack   large string to search
  pszNeedle     substring to count within haystack
  returns       count of needles in haystack (0-n)
--------------------------------------------------------------------------------------------------*/
unsigned Nt2Count(const char *pszHaystack, const char *pszNeedle)
{
  const char   *psz;
  unsigned      count = 0;

  psz = pszHaystack;
  while(psz)
  {
    psz = strstr(psz, pszNeedle);
    if(psz)
    {
      ++count;
      psz += strlen(pszNeedle);
    }
  }

  return count;
}

/*!------------------------------------------------------------------------------------------------
  Get one of multiple choice from console user and return index into the choices (0-n).
  Repeats prommpt if user gives invalid anser. Enter = choice index 0

  @param      szPrompt    prompt to display
  @param      szChoices   choices to display
  @returns    index into choices 0-n. Enter = choice index 0
-------------------------------------------------------------------------------------------------*/
unsigned FlyTestGetAnswer(const char *szPrompt, const char *szChoices)
{
  char        sz[64];
  unsigned    i;
  unsigned    numChoices = strlen(szChoices);
  unsigned    ret = 0;
  bool_t      fFound;
  char       *szSep;

  while(numChoices)
  {
    // print prompt and choices
    printf("\n%s (", szPrompt);
    szSep = "";
    for(i = 0; i  < numChoices; ++i)
    {
      printf("%s%c", szSep, szChoices[i]);
      szSep = "/";
    }
    printf(") ");
    fflush(stdout);

    memset(sz, '\0', sizeof(sz));
    fgets(sz, sizeof(sz)-1, stdin);

    // return only is choosing choice index 0
    if(*sz == '\0' || *sz == '\n' || *sz == '\r')
      break;

    else
    {
      fFound = FALSE;
      for(i = 0; i < numChoices; ++i)
      {
        if(toupper(*sz) == toupper(szChoices[i]))
        {
          ret = i;
          fFound = TRUE;
          break;
        }
      }
      if(fFound)
        break;
    }
  }

  return ret;
}

/*!------------------------------------------------------------------------------------------------
  Get yes/no choice with prompt

  @param      szPrompt    prompt to display
  @returns    TRUE or FALSE
-------------------------------------------------------------------------------------------------*/
bool_t FlyTestGetYesNo(const char *szPrompt)
{
  return (FlyTestGetAnswer(szPrompt, "yn") == 0) ? TRUE : FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Calculate totals from log file

  @param    pPassed    pointer to receive passed count
  @param    pFailed    pointer to receive failed count
  @param    pSkipped   pointer to receive skipped count
  @returns  TRUE if log file contains test results, FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlyTestCalcLogTotals(unsigned *pPassed, unsigned *pFailed, unsigned *pSkipped)
{
  const char       *pszFile;
  const char       *pszStart;
  const char       *psz;
  unsigned          passed  = 0;
  unsigned          failed  = 0;
  unsigned          skipped = 0;
  bool_t            fWorked = TRUE;

  // read the log file
  FlyLogFileClose();
  pszFile = FlyFileRead(FlyLogDefaultName());
  if(!pszFile)
    fWorked = FALSE;

  if(fWorked)
  {
    // find last test suite run (last $TEST_START)
    psz = pszFile;
    pszStart = NULL;
    while(psz)
    {
      psz = strstr(psz, m_szMTestStart);
      if(psz != NULL)
      {
        pszStart = psz;
        psz += strlen(m_szMTestStart);
      }
    }

    if(pszStart == NULL)
      fWorked = FALSE;
    else
    {
      passed  = Nt2Count(pszStart, m_szMTestPassed);
      failed  = Nt2Count(pszStart, m_szMTestFailed);
      skipped = Nt2Count(pszStart, m_szMTestSkipped);
    }

    FlyFree((void *)pszFile);
  }

  *pPassed  = passed;
  *pFailed  = failed;
  *pSkipped = skipped;
  return fWorked;
}

/*!------------------------------------------------------------------------------------------------
  Default assert print function.

  @param    szxExpr     assert expression or helpful message
  @param    szFile      string version of the file
  @param    szFunc      string function name
  @param    line        line number
  @return   1 (exit code for failed)
*///-----------------------------------------------------------------------------------------------
int FlyTestErrorPrint(const char *szExpr, const char *szFile, const char *szFunc, unsigned line)
{
  FlyTestPrintf("Assert: (%s), file: %s, func: %s(), line: %u\n", szExpr, szFile, szFunc, line);
  return 1;
}
