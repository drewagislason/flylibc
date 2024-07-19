/**************************************************************************************************
  test_cli.c - Test cases for FlyCli.c
  Copyright 2023 Drew Gislason
  License: MIT <https://mit-license.org>
**************************************************************************************************/
#include "FlyTest.h"
#include "FlyCli.h"
#include "FlyStr.h"

/*-------------------------------------------------------------------------------------------------
  Test a simple command-line with some options
-------------------------------------------------------------------------------------------------*/
void TcCliSimple(void)
{
  const char         *argv[]    = { "my_prog", "-v", "arg1", "arg2" };
  int                 argc      = NumElements(argv);
  bool_t              fVerbose  = FALSE;
  const flyCliOpt_t   opts[]    =
  {
    { "-v", &fVerbose, FLYCLI_BOOL }
  };
  const flyCli_t      cli       = { &argc, argv, NumElements(opts), opts };
  flyCliErr_t         err;
  int                 nArgs;

  FlyTestBegin();

  err = FlyCliParse(&cli);
  if(err)
  {
    FlyTestFailed();
  }

  if(fVerbose != TRUE)
  {
    FlyTestPrintf("Expected verbose to be TRUE\n");
    FlyTestFailed();
  }

  nArgs = FlyCliNumArgs(&cli);
  if(nArgs != 3)
  {
    FlyTestPrintf("Expected 3 arguments, got %d\n", nArgs);
    FlyTestFailed();
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test a complex command-line with come edge cases
-------------------------------------------------------------------------------------------------*/
void TcCliComplex(void)
{
  const char         *argv[]    = 
    { "my_prog", "-v", "arg1", "-o=-3", "arg2", "--sa", "hello", "-n=99", "--sb=world", "arg3" };
  const char         *args_only[] = { "my_prog", "arg1", "arg2", "arg3" };
  int                 argc      = NumElements(argv);
  bool_t              fVerbose  = FALSE;
  int                 o         = 0;
  int                 n         = 0;
  const char         *szSa      = NULL;
  const char         *szSb      = NULL;
  const flyCliOpt_t   aOpts[]   =
  {
    { "-v",   &fVerbose,  FLYCLI_BOOL },
    { "-o",   &o,         FLYCLI_INT },
    { "-n",   &n,         FLYCLI_INT },
    { "--sa", &szSa,      FLYCLI_STRING },
    { "--sb", &szSb,      FLYCLI_STRING }
  };
  const flyCli_t cli =
  {
    .pArgc = &argc,
    .argv = argv,
    .nOpts = NumElements(aOpts),
    .pOpts = aOpts,
    .szVersion = "version",
    .szHelp = "help"
  };
  flyCliErr_t         err;
  int                 nArgs;
  unsigned            i;
  const char         *sz;

  FlyTestBegin();


  err = FlyCliParse(&cli);
  if(err)
    FlyTestFailed();

  nArgs = FlyCliNumArgs(&cli);
  if(nArgs != NumElements(args_only))
  {
    FlyTestPrintf("Expected %zu arguments, got %d\n", NumElements(args_only), nArgs);
    FlyTestFailed();
  }
  if(fVerbose != TRUE)
  {
    FlyTestPrintf("Expected verbose to be TRUE\n");
    FlyTestFailed();
  }
  if(o != -3)
  {
    FlyTestPrintf("Expected o == -3\n");
    FlyTestFailed();
  }
  if(n != 99)
  {
    FlyTestPrintf("Expected n == -3\n");
    FlyTestFailed();
  }
  if(!szSa || strcmp(szSa, "hello") != 0)
  {
    FlyTestPrintf("Expected 'hello', got '%s'\n", FlyStrNullOk(szSa));
    FlyTestFailed();
  }
  if(!szSb || strcmp(szSb, "world") != 0)
  {
    FlyTestPrintf("Expected 'world', got '%s'\n", FlyStrNullOk(szSb));
    FlyTestFailed();
  }

  for(i = 0; i < nArgs; ++i)
  {
    sz = FlyCliArg(&cli, i);
    if(!sz || strcmp(sz, args_only[i]) != 0)
    {
      FlyTestPrintf("Expected '%s', got '%s'\n", args_only[i], FlyStrNullOk(sz));
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test a command-line with errors
-------------------------------------------------------------------------------------------------*/
void TcCliErrors(void)
{
  // cli_1 error unknown argument
  const char         *argv_1[]        = { "my_prog", "-q" };
  int                 argc_1          = NumElements(argv_1);
  const char         *argv_help[]     = { "my_prog", "--help" };
  int                 argc_help       = NumElements(argv_help);
  const char         *argv_version[]  = { "my_prog", "--version" };
  int                 argc_version    = NumElements(argv_version);
  const char         *argv_sa[]       = { "my_prog", "arg1", "arg2", "--sa" };
  int                 argc_sa         = NumElements(argv_sa);
  const char         *argv_num[]      = { "my_prog", "arg1", "-n=foo", "arg2" };
  int                 argc_num        = NumElements(argv_sa);
  bool_t              fVerbose        = FALSE;
  int                 number          = 9;
  const char         *szSa            = NULL;
  const char         *szArg;
  const flyCliOpt_t   aOpts[] =
  {
    { "-v",   &fVerbose,  FLYCLI_BOOL },
    { "-n",   &number,    FLYCLI_INT },
    { "--sa", &szSa,      FLYCLI_STRING },
  };
  flyCli_t            cli       = { &argc_1, argv_1, NumElements(aOpts), aOpts, "version", "help" };
  flyCliErr_t         err;

  FlyTestBegin();

  // test version
  cli.pArgc = &argc_version;
  cli.argv  = argv_version;
  err = FlyCliParse(&cli);
  if(err != FLYCLI_VERSION)
  {
    FlyTestPrintf("Expected FLYCLI_VERSION, got %d\n", err);
    FlyTestFailed();
  }
  if(!FlyTestGetYesNo("Did you see version?"))
  {
    FlyTestFailed();
  }

  // test help
  cli.pArgc = &argc_help;
  cli.argv  = argv_help;
  err = FlyCliParse(&cli);
  if(err != FLYCLI_HELP)
  {
    FlyTestPrintf("Expected FLYCLI_HELP , got %d\n", err);
    FlyTestFailed();
  }
  if(!FlyTestGetYesNo("Did you see version and help?"))
  {
    FlyTestFailed();
  }

  // test bad option
  cli.pArgc = &argc_1;
  cli.argv  = argv_1;
  err = FlyCliParse(&cli);
  if(err != FLYCLI_ERR_OPT)
  {
    FlyTestPrintf("Expected FLYCLI_ERR_OPT, got %d\n", err);
    FlyTestFailed();
  }
  if(!FlyTestGetYesNo("Did you see Invalid option?"))
  {
    FlyTestFailed();
  }
  
  // test expected number
  cli.pArgc = &argc_num;
  cli.argv  = argv_num;
  err = FlyCliParse(&cli);
  if(err != FLYCLI_ERR_NO_INT)
  {
    FlyTestPrintf("Expected FLYCLI_ERR_NO_INT, got %d\n", err);
    FlyTestFailed();
  }
  if(!FlyTestGetYesNo("Did you see Expected number?"))
  {
    FlyTestFailed();
  }
  
  // test missing argument
  cli.pArgc = &argc_sa;
  cli.argv  = argv_sa;
  err = FlyCliParse(&cli);
  szArg = FlyCliArg(&cli, 0);
  if(szArg == NULL || strcmp(szArg, "my_prog") != 0)
    FlyTestFailed();
  szArg = FlyCliArg(&cli, 2);
  if(szArg == NULL || strcmp(szArg, "arg2") != 0)
    FlyTestFailed();
  szArg = FlyCliArg(&cli, 3);
  if(szArg != NULL)
    FlyTestFailed();
  if(err != FLYCLI_ERR_MISSING_ARG)
  {
    FlyTestPrintf("Expected FLYCLI_ERR_MISSING_ARG, got %d\n", err);
    FlyTestFailed();
  }
  if(!FlyTestGetYesNo("Did you Missing argument?"))
  {
    FlyTestFailed();
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test a command-line with errors, but no printing
-------------------------------------------------------------------------------------------------*/
void TcCliNoPrint(void)
{
  // cli_1 error unknown argument
  const char         *argv_1[]        = { "my_prog", "-q" };
  int                 argc_1          = NumElements(argv_1);
  const char         *argv_help[]     = { "my_prog", "--help" };
  int                 argc_help       = NumElements(argv_help);
  const char         *argv_version[]  = { "my_prog", "--version" };
  int                 argc_version    = NumElements(argv_version);
  const char         *argv_sa[]       = { "my_prog", "arg1", "arg2", "--sa" };
  int                 argc_sa         = NumElements(argv_sa);
  bool_t              fVerbose        = FALSE;
  const char         *szSa            = NULL;
  const char          szNoYes[]       = "ny";
  const flyCliOpt_t opts[] =
  {
    { "-v",   &fVerbose,  FLYCLI_BOOL },
    { "--sa", &szSa,      FLYCLI_STRING },
  };
  flyCli_t            cli = { &argc_1, argv_1, NumElements(opts), opts, "version", "help", TRUE };
  flyCliErr_t         err;

  FlyTestBegin();

  cli.pArgc = &argc_1;
  cli.argv  = argv_1;
  err = FlyCliParse(&cli);

  if(err != FLYCLI_ERR_OPT)
  {
    FlyTestPrintf("Expected FLYCLI_ERR_OPT, got %d\n", err);
    FlyTestFailed();
  }
  else if(FlyTestGetAnswer("Did it print invalid option?", szNoYes))
  {
    FlyTestFailed();
  }

  cli.pArgc = &argc_version;
  cli.argv  = argv_version;
  err = FlyCliParse(&cli);
  if(err != FLYCLI_VERSION)
  {
    FlyTestPrintf("Expected FLYCLI_VERSION, got %d\n", err);
    FlyTestFailed();
  }
  else if(FlyTestGetAnswer("Did it print version?", szNoYes))
  {
    FlyTestFailed();
  }

  cli.pArgc = &argc_help;
  cli.argv  = argv_help;
  err = FlyCliParse(&cli);
  if(err != FLYCLI_HELP)
  {
    FlyTestPrintf("Expected FLYCLI_HELP , got %d\n", err);
    FlyTestFailed();
  }
  else if(FlyTestGetAnswer("Did it print version and help?", szNoYes))
  {
    FlyTestFailed();
  }

  cli.pArgc = &argc_sa;
  cli.argv  = argv_sa;
  err = FlyCliParse(&cli);
  if(err != FLYCLI_ERR_MISSING_ARG)
  {
    FlyTestPrintf("Expected FLYCLI_ERR_MISSING_ARG, got %d\n", err);
    FlyTestFailed();
  }
  else if(FlyTestGetAnswer("Did it print Missing argument?", szNoYes))
  {
    FlyTestFailed();
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test all possible ways bool option can be used
-------------------------------------------------------------------------------------------------*/
void TcCliBool(void)
{
  const char         *argv[] = { "my_prog", "-n", "-v-"};
  int                 argc   = NumElements(argv);
  bool_t              n;
  bool_t              v;
  const flyCliOpt_t   opts[] =
  {
    { "-n", &n, FLYCLI_BOOL },
    { "-v", &v, FLYCLI_BOOL }
  };
  const flyCli_t cli = { &argc, argv, NumElements(opts), opts };
  flyCliErr_t         err;

  FlyTestBegin();

  n = FALSE;
  v = TRUE;

  err = FlyCliParse(&cli);
  if(err || FlyCliNumArgs(&cli) != 1)
    FlyTestFailed();
  if(n != TRUE)
    FlyTestFailed();
  if(v != FALSE)
    FlyTestFailed();

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test all possible ways int can be used
-------------------------------------------------------------------------------------------------*/
void TcCliInt(void)
{
  const char   *argv[] = { "my_prog", "-n", "-p=5", "-v99", "-r-", "-q=-33" };
  int           argc   = NumElements(argv);
  int           n;
  int           p;
  int           q;
  int           r;
  int           v;
  const flyCliOpt_t   opts[] =
  {
    { "-n", &n, FLYCLI_INT },
    { "-p", &p, FLYCLI_INT },
    { "-q", &q, FLYCLI_INT },
    { "-r", &r, FLYCLI_INT },
    { "-v", &v, FLYCLI_INT },
  };
  const flyCli_t cli = { &argc, argv, NumElements(opts), opts };
  flyCliErr_t   err;

  FlyTestBegin();

  // set items so we know if they have been changed
  n = p = q = v = 0;
  r = 5;

  err = FlyCliParse(&cli);
  if(err || FlyCliNumArgs(&cli) != 1)
    FlyTestFailed();
  if(n != 1)
    FlyTestFailed();
  if(p != 5)
    FlyTestFailed();
  if(q != -33)
    FlyTestFailed();
  if(r != 0)
    FlyTestFailed();
  if(v != 99)
    FlyTestFailed();

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test all possible ways int can be used
-------------------------------------------------------------------------------------------------*/
void TcCliString(void)
{
  const char   *argv[]  = { "my_prog", "-s=my string", "-p", "print"};
  int           argc    = NumElements(argv);
  const char   *pszS     = NULL;
  const char   *pszP     = NULL;
  const flyCliOpt_t   opts[] =
  {
    { "-s", &pszS, FLYCLI_STRING },
    { "-p", &pszP, FLYCLI_STRING }
  };
  const flyCli_t cli = { &argc, argv, NumElements(opts), opts };
  flyCliErr_t   err;

  FlyTestBegin();

  err = FlyCliParse(&cli);
  if(err || FlyCliNumArgs(&cli) != 1)
    FlyTestFailed();
  if(!pszS || strcmp(pszS, "my string") != 0)
    FlyTestFailed();
  if(!pszP || strcmp(pszP, "print") != 0)
    FlyTestFailed();

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test double dash for sending options to subprograms
-------------------------------------------------------------------------------------------------*/
void TcCliDoubleDash(void)
{
  const char         *argv[]      = { "my_prog", "arg1", "-v", "arg2", "--", "subarg1", "-x", "subarg2" };
  int                 argc        = NumElements(argv);
  int                 argc_empty  = 0;
  bool_t              fVerbose;
  const flyCliOpt_t   opts[]      =
  {
    { "-v", &fVerbose, FLYCLI_BOOL }
  };
  const flyCli_t      cli       = { &argc, argv, NumElements(opts), opts };
  const flyCli_t      cli_empty = { &argc_empty, argv, NumElements(opts), opts };
  flyCliErr_t         err;
  int                 i;
  int                 nArgs;
  const char         *pszArg;

  FlyTestBegin();

  err = FlyCliParse(&cli_empty);
  nArgs = FlyCliNumArgs(&cli_empty);
  if(err || nArgs != 0)
  {
    FlyTestPrintf("Expected 0 arguments, got %d\n", nArgs);
    FlyTestFailed();
  }

  fVerbose = FALSE;
  err = FlyCliParse(&cli);
  nArgs = FlyCliNumArgs(&cli);
  if(err || nArgs != 3)
  {
    FlyTestPrintf("Expected 3 arguments, got %d\n", nArgs);
    if(FlyTestVerbose())
    {
      i = 0;
      do
      {
        pszArg = FlyCliArg(&cli, i);
        if(pszArg)
        {
          FlyTestPrintf("arg %d: %s\n", i, pszArg);
          ++i;
        }
      } while(pszArg);
    }
    FlyTestFailed();
  }
  if(fVerbose != TRUE)
  {
    FlyTestPrintf("Expected verbose to be TRUE\n");
    FlyTestFailed();
  }

  i = FlyCliDoubleDash(&cli);
  if(i != 4)
  {
    FlyTestPrintf("Expected double dash at position 4, got %d\n", i);
    FlyTestFailed();
  }

  FlyTestEnd();
}



/*-------------------------------------------------------------------------------------------------
  Test each function
-------------------------------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
  const char          szName[] = "test_cli";
  const sTestCase_t   aTestCases[] =
  {
    { "TcCliSimple",      TcCliSimple },
    { "TcCliBool",        TcCliBool, "BAR FOO" },
    { "TcCliInt",         TcCliInt },
    { "TcCliString",      TcCliString, "FOO BAR" },
    { "TcCliComplex",     TcCliComplex, "FOO" },
    { "TcCliErrors",      TcCliErrors,  "M" },
    { "TcCliNoPrint",     TcCliNoPrint, "M" },
    { "TcCliDoubleDash",  TcCliDoubleDash, "BAR" },
  };
  hTestSuite_t        hSuite;
  int                 ret;

  // set up signal handling and logging
  // NedSigSetExit(argv[0], NULL);
  // FlyTestMaskSet(NEDLOG_JSON);

  FlyTestInit(szName, argc, argv);
  hSuite = FlyTestNew(szName, NumElements(aTestCases), aTestCases);
  FlyTestRun(hSuite);
  ret = FlyTestSummary(hSuite);
  FlyTestFree(hSuite);

  return ret;
}
