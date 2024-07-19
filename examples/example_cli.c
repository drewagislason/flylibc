/**************************************************************************************************
  exmp_cli.c - Example using FlyCli API
  Copyright 2023 Drew Gislason
  License: MIT <https://mit-license.org>

  Command-line parsing example. Try the following command-lines and see what happens:

      $ exmp_cli --help
      $ exmp_cli --version
      $ exmp_cli arg1
      $ exmp_cli -v arg1 --name "Sam Spade" arg2
      $ exmp_cli -v --name "Repeat" -n=3 arg1 arg2
**************************************************************************************************/
#include "FlyCli.h"

int main(int argc, const char *argv[])
{
  char               *pszName   = "No Name";
  bool_t              fVerbose  = FALSE;
  int                 repeat    = 1;
  const flyCliOpt_t   cliOpts[] =
  {
    { "--name", &pszName,  FLYCLI_STRING },
    { "-n",     &repeat,   FLYCLI_INT },
    { "-v",     &fVerbose, FLYCLI_BOOL },
  };
  const flyCli_t      cli =
  {
    .pArgc      = &argc,
    .argv       = argv,
    .nOpts      = NumElements(cliOpts),
    .pOpts      = cliOpts,
    .szVersion  = "example_cli v1.0",
    .szHelp     = "Usage = example_cli [-n=#] [--name \"Some Name\"] [-v] args...\n"
                  "\n"
                  "Options:\n"
                  "-n       repeat each arg n times\n"
                  "--name   name (default: \"No Name\")\n"
                  "-v       verbose\n"
                  "\n"
                  "Try intermixing arguments and options. They can be in any order.\n"
                  "Try ./example_cli --name World *.c\n"
                  "Try ./example_cli -v Yeah! --name World \"Second Arg\" -n=3\n"
  };
  int     i, j;
  int     nArgs;

  if(FlyCliParse(&cli) != FLYCLI_ERR_NONE)
    exit(1);
  nArgs = FlyCliNumArgs(&cli);
  if(nArgs < 2)
  {
    FlyCliHelp(&cli);
    exit(1);
  }

  printf("%sHello %s\n\n", fVerbose ? "(verbose) " : "", pszName);
  if(nArgs > 1)
  {
    for(i = 1; i < nArgs; ++i)
    {
      printf("Arg %d: ", i);
      for(j = 0; j < repeat; ++j)
        printf("%s ", FlyCliArg(&cli, i));
      printf("\n");
    }
  }

  return 0;
}
