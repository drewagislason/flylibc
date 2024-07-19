/**************************************************************************************************
  FlyCli.c - Easily process command-line options and arguments
  Copyright 2024 Drew Gislason
  license: <https://mit-license.org>
**************************************************************************************************/

/*!
  @defgroup FlyCli - A C API for handling the command-line (argc, argv) in a consistent way.

  This API tries to capture the "main" way options and arguments are used for most Linux-style
  command-line programs.

  Features:

  - Options --version and --help can be static strings or dynamic
  - Options have the following types: string, int, bool
  - Options and arguments can be intermixed
  - Arguments are indexed 0-n (just like argc/argv) regardless of how many options are intermixed
  - Supports -- to allow for options and arguments sent to a sub-program
  - Works with any array of string ptrs, not just `int main(int argc, const char *argv[])`
  - Built-in error messages, or higher layer can use error codes to display its own messages
  - Requires no memory allocation (heap) and very little stack space

  Options are `--options` or `-o`, whereas arguments are "strings". The convention is that single
  letter options need only one dash, e.g. `-v`, while longer options need 2 dashs, e.g. "--debug".

  One convention **not** supported is the combining of single letter options. For example, if you
  options `-v` and `-a`, you cannot specifiy `-va` or `-av`. Maybe in a future version.

  For an example of a common command-line program with lots of options, see the command `ls`:  
  <https://www.man7.org/linux/man-pages/man1/ls.1.html>
  
  Some examples of command lines and a program to parse them:

  ```
  $ example_cli --help
  $ example_cli --version
  $ example_cli arg1
  $ example_cli -v arg1 --name "Sam Spade" arg2 arg3
  $ example_cli -n=3 --name Repeat arg1 arg2
  ```

  @example FlyCli with bool, int, string options

  ```c
  #include "FlyCli.h"

  int main(int argc, const char *argv[])
  {
    char         *pszName   = "No Name";
    bool_t        fVerbose  = FALSE;
    int           repeat    = 1;    // number of times to repeat
    const flyCliOpt_t cliOpts[] =
    {
      { "--name", &pszName,  FLYCLI_STRING },
      { "-n",     &repeat,   FLYCLI_INT },
      { "-v",     &fVerbose, FLYCLI_BOOL },
    };
    const flyCli_t cli =
    {
      .pArgc      = &argc,    // Important! Notice the ampersand.
      .argv       = argv,
      .nOpts      = NumElements(cliOpts),
      .pOpts      = cliOpts,
      .szVersion  = "example_cli v1.0",
      .szHelp     = "Usage = example_cli [-n=#] [--name \"Some Name\"] [-v]\n"
    };
    int           i, j;
    int           nArgs;
    flyCliErr_t   err;

    // parse all options
    err = FlyCliParse(&cli, NULL);
    if(err)
      return 1;

    printf("Hello %s%s\n\n", fVerbose ? "(verbose) " : "", pszName);

    nArgs = FlyCliNumArgs(&cli);
    for(i = 1; i < nArgs; ++i)
    {
      printf("%d: ", i);
      for(j = 0; j < repeat; ++j)
        printf("%s ", FlyCliArg(&cli, i));
      printf("\n");
    }

    return 0;
  }
  ```
*/
#include <ctype.h>
#include "FlyCli.h"

static const char m_szDoubleDash[] = "--";
static const char m_optChar        = '-';

/*-------------------------------------------------------------------------------------------------
  Get pointer to after the equal sign, or NULL if no equal sign

  @param    pOpt    pointer to a found option structure, e.g. pOpt->szOpt is "-v"
  @param    pszArg  ptr to found string that starts with option, e.g. "-v=5" or "-v"
  @return   ptr to char after equal sign
*///-----------------------------------------------------------------------------------------------
static const char * GetAfterEqual(const flyCliOpt_t *pOpt, const char *pszArg)
{
  const char *pszEqual = NULL;
  unsigned    len;

  len = strlen(pOpt->szOpt);
  if((strlen(pszArg) > len) && (pszArg[len] == '='))
    pszEqual = &pszArg[len + 1];

  return pszEqual;
}

/*-------------------------------------------------------------------------------------------------
  Is this argv[i] string option found in the pCli structure? If so, return it.

  @param    pCli    pointer a flyCli_t structure with 0 or more options
  @param    pszArg  pointer to string, usually argv[i]
  @return   pointer to option structure from CLI, or NULL
*///-----------------------------------------------------------------------------------------------
static const flyCliOpt_t * GetOpt(const flyCli_t *pCli, const char *pszArg)
{
  const flyCliOpt_t  *pOptFound = NULL;
  const flyCliOpt_t  *pOpt;
  unsigned            i;
  unsigned            len;

  // all options begin with dash
  if(*pszArg == m_optChar)
  {
    pOpt = pCli->pOpts;
    for(i = 0; i < pCli->nOpts; ++i)
    {
      len = strlen(pOpt->szOpt);
      if(len == 0)
        break;

      // start of string matches, that is, -v matches: -v, -v=string, -vvvvv or -v=1
      if(strncmp(pOpt->szOpt, pszArg, len) == 0)
      {
        if(pszArg[len] == '\0' || pszArg[len] == '=')
        {
          pOptFound = pOpt;
          break;
        }
        else if((pOpt->type == FLYCLI_BOOL || pOpt->type == FLYCLI_INT)
                && pszArg[len] == m_optChar && pszArg[len + 1] == '\0')
        {
          pOptFound = pOpt;
          break;
        }
        else if(pOpt->type == FLYCLI_INT && isdigit(pszArg[len]))
        {
          pOptFound = pOpt;
          break;
        }
      }
      ++pOpt;
    }
  }

  return pOptFound;
}

/*!------------------------------------------------------------------------------------------------
  Parse the command-line for all options.

  @param    pCli    pointer to a filled-in flyCli_t structure
  @return   FLYCLI_ERR_NONE(0) if no error, FLYCLI_ERR_something, e.g. FLYCLI_ERR_BAD_OPT
*///-----------------------------------------------------------------------------------------------
flyCliErr_t FlyCliParse(const flyCli_t *pCli)
{
  return FlyCliParseEx(pCli, NULL, NULL);
}

/*!------------------------------------------------------------------------------------------------
  Parse the command-line for all options with return index if error.

  Used with pCli->fNoPrint=TRUE so higher layer can print error messages.

  @param    pCli      pointer to a filled-in flyCli_t structure
  @param    pNumArgs  # of arguments
  @param    pIndex  returned index into pCli->argv[] array if error code
  @return   FLYCLI_ERR_NONE(0) if no error, FLYCLI_ERR_something, e.g. FLYCLI_ERR_BAD_OPT
*///-----------------------------------------------------------------------------------------------
flyCliErr_t FlyCliParseEx(const flyCli_t *pCli, int *pNumArgs, int *pIndex)
{
  const flyCliOpt_t  *pOpt;
  int                 limit;
  const char        **argv      = pCli->argv;
  const char         *sz;
  unsigned            optLen;
  int                 i;
  flyCliErr_t         err       = FLYCLI_ERR_NONE;
  int                 nArgs     = 0;

  limit = FlyCliDoubleDash(pCli);
  if(limit < 0)
    limit = *pCli->pArgc;
  if(limit >= 1)
    nArgs = 1;

  // process options
  for(i = 1; !err && i < limit; ++i)
  {
    // at double dash "--", everything from here on out is an argument to subprogram
    if(strcmp(argv[i], m_szDoubleDash) == 0)
      break;

    // check for built-in options --help and --version, otherwise error
    pOpt = GetOpt(pCli, argv[i]);
    if(pOpt == NULL)
    {
      // --version will display version if the string is present
      if(strcmp(argv[i], "--version") == 0 && pCli->szVersion)
      {
#if FLY_CLI_PRINT
        if(!pCli->fNoPrint)
          printf("%s\n", pCli->szVersion);
#endif
        err = FLYCLI_VERSION;
        break;
      }

      // --help will display help if the string is present
      if(strcmp(argv[i], "--help") == 0 && pCli->szHelp)
      {
#if FLY_CLI_PRINT
        if(!pCli->fNoPrint)
          printf("%s\n\n%s\n", pCli->szVersion ? pCli->szVersion : argv[0], pCli->szHelp);
#endif
        err = FLYCLI_HELP;
        break;
      }

      // unknown option
      else if(argv[i][0] == m_optChar)
      {
#if FLY_CLI_PRINT
        if(!pCli->fNoPrint)
          printf("Invalid option: %s. Try --help\n", argv[i]);
#endif
        err = FLYCLI_ERR_OPT;
        break;
      }
    }

    // set option variable based on type
    if(pOpt)
    {
      // e.g. -v or -v-
      if(pOpt->type == FLYCLI_BOOL)
      {
        if(argv[i][strlen(argv[i]) - 1] == '-')
          *(bool_t *)pOpt->pValue = FALSE;
        else
          *(bool_t *)pOpt->pValue = TRUE;
      }

      // e.g. -n, -n-, -n5 or -n=99
      // note: -n is the same as -n=1, and -n- is the same as -n=0
      else if(pOpt->type == FLYCLI_INT)
      {
        sz = GetAfterEqual(pOpt, argv[i]);
        if(sz == NULL)
        {
          optLen = strlen(pOpt->szOpt);
          if(isdigit(argv[i][optLen]))
            sz = &argv[i][optLen];
          else if(argv[i][optLen] == '-')
            sz = "0";
          else
            sz = "1";
        }
        if(isdigit(*sz) || ((*sz == '-') && isdigit(sz[1])))
          *(int *)pOpt->pValue = atoi(sz);
        else
        {
#if FLY_CLI_PRINT
            if(!pCli->fNoPrint)
              printf("Expected a number for option: %s. Try --help\n", argv[i]);
#endif
            err = FLYCLI_ERR_NO_INT;
            break;
        }
      }

      // e.g. -s="hello world" or -s "hello world"
      else if(pOpt->type == FLYCLI_STRING)
      {
        sz = GetAfterEqual(pOpt, argv[i]);
        if(sz == NULL)
        {
          if((i + 1) >= limit)
          {
#if FLY_CLI_PRINT
            if(!pCli->fNoPrint)
              printf("Missing argument for option: %s. Try --help\n", argv[i]);
#endif
            err = FLYCLI_ERR_MISSING_ARG;
            break;
          }
          ++i;
          sz = argv[i];
        }
        *(const char **)pOpt->pValue = sz;
      }
    }

    else
      ++nArgs;
  }

  // if user wants error index into argv[], return it
  if(pIndex && err)
    *pIndex = i;
  if(pNumArgs)
    *pNumArgs = nArgs;

  return err;
}

/*!------------------------------------------------------------------------------------------------
  Return the arg by index, not counting options.

  For Example: "my_prog arg1 -opt1 arg2", FlyCliArg(&cli, 2) would return "arg2"

  @param    pCli    pointer to a filled-in flyCli_t structure
  @param    index   index (0-n) into arguments
  @return   ptr to argument string or NULL if invalid index
*///-----------------------------------------------------------------------------------------------
const char * FlyCliArg(const flyCli_t *pCli, int index)
{
  const flyCliOpt_t  *pOpt;
  int                 argc    = *pCli->pArgc;
  const char        **argv    = pCli->argv;
  const char         *szArg   = NULL;
  int                 i;
  int                 nArgs;

  // bad index or no arguments
  if(index < 0 || argc <= 0)
    return NULL;

  // special case: index 0 is always argv[0] which may look like an option that begins with "-"
  if(index == 0)
    szArg = argv[0];

  else
  {
    nArgs = 1;
    for(i = 1; i < argc; ++i)
    {
      // stop at double dash
      if(strcmp(argv[i], m_szDoubleDash) == 0)
        break;

      // an argument, ignore it
      if(*argv[i] != '-')
      {
        if(nArgs >= index)
        {
          szArg = argv[i];
          break;
        }
        ++nArgs;
      }

      // option, ignore it
      else
      {
        pOpt = GetOpt(pCli, argv[i]);
        if(pOpt && (pOpt->type == FLYCLI_STRING) && (GetAfterEqual(pOpt, argv[i]) == NULL))
          ++i;
      }
    }
  }

  return szArg;
}

/*!------------------------------------------------------------------------------------------------
  Print help. Can be used if no args for example, or bad arg after some message.

  @param    pCli    pointer to a filled-in flyCli_t structure
  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyCliHelp(const flyCli_t *pCli)
{
#if FLY_CLI_PRINT
  if(!pCli->fNoPrint)
    printf("%s\n\n%s\n", pCli->szVersion ? pCli->szVersion : "", pCli->szHelp ? pCli->szHelp : "");
#endif
}

/*!------------------------------------------------------------------------------------------------
  Return the # of non option arguments

  @param    pCli    pointer to a filled-in flyCli_t structure
  @return   1-n (number of arguments)
*///-----------------------------------------------------------------------------------------------
int FlyCliNumArgs(const flyCli_t *pCli)
{
  const flyCliOpt_t  *pOpt;
  int                 nArgs = 0;
  int                 limit;
  int                 i;

  // stop at the double dash (if any), otherwise, what's in argc
  limit = FlyCliDoubleDash(pCli);
  if(limit < 0)
    limit = *pCli->pArgc;

  // count arguments, not options
  for(i = 0; i < limit; ++i)
  {
    if(i == 0)
    {
      ++nArgs;
      continue;
    }

    // if it's not an option, it's an arg
    if(pCli->argv[i][0] != m_optChar)
      ++nArgs;
    else
    {
      // if it's a string option, e.g. "--str value", skip the value
      pOpt = GetOpt(pCli, pCli->argv[i]);
      if(pOpt && pOpt->type == FLYCLI_STRING && (GetAfterEqual(pOpt, pCli->argv[i]) == NULL))
      {
        ++i;
        continue;
      }
    }
  }

  return nArgs;
}

/*!------------------------------------------------------------------------------------------------
  Return in index into the pCli->argv[] array for the double dash, or -1 if no "--".

  A double dash is the convention used to pass arguments to sub-programs:

      $ flymake run program -B -- arg_to_program --opt_to_program

  @param    pCli    pointer to a filled-in flyCli_t structure
  @return   index to double dash or -1 if no "--"
*///-----------------------------------------------------------------------------------------------
int FlyCliDoubleDash(const flyCli_t *pCli)
{
  int index = -1;
  int i;

  for(i = 1; i < *pCli->pArgc; ++i)
  {
    if(strcmp(pCli->argv[i], m_szDoubleDash) == 0)
    {
      index = i;
      break;
    }
  }

  return index;
}
