/**************************************************************************************************
  FlyCli.h
  Copyright 2024 Drew Gislason
  license: MIT <https://mit-license.org>
*///***********************************************************************************************
#ifndef FLY_CLI_H
#define FLY_CLI_H
#include "Fly.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

// for embedded systems, remove any printf() code by defining FLY_CLI_PRINT 0
#ifndef FLY_CLI_PRINT
 #define FLY_CLI_PRINT 1
#endif

typedef enum 
{
  FLYCLI_BOOL = 0,          // -v or -v-
  FLYCLI_INT,               // -i, -i-, i=5 or -i=99
  FLYCLI_STRING,            // -f=filename or -f filename.c
} flyCliOptType_t;

typedef enum
{
  FLYCLI_ERR_NONE = 0,
  FLYCLI_HELP,              // not an error, but an indicator that help was displayed
  FLYCLI_VERSION,           // not an error, but an indicator that version was displayed
  FLYCLI_ERR_OPT,           // unknown option
  FLYCLI_ERR_MISSING_ARG,   // missing argument, expected a string
  FLYCLI_ERR_NO_INT,        // expected a number, e.g -n=5 or -n99, but not -n=foo
  FLYCLI_ERR_TYPE           // unknown option type in flyCli_t structure
} flyCliErr_t;

typedef struct
{
  const char       *szOpt;    // e.g. -v or --fancy_arg
  void             *pValue;   // must point to type (bool_t, int, or const char **
  flyCliOptType_t   type;
} flyCliOpt_t;

typedef struct
{
  const int          *pArgc;
  const char        **argv;
  unsigned            nOpts;
  const flyCliOpt_t  *pOpts;
  const char         *szVersion;  // e.g. "my_prog v1.0.3"
  const char         *szHelp;     // e.g. "usage = my_prog [-s] [-v] file\n\n-s  sort\n-v  verbose\n";
  bool_t              fNoPrint;   // don't print errors, higher layer will do it, use TRUE or FALSE
} flyCli_t;

#define FLYCLI_HELP     -32766
#define FLYCLI_VERSION  -32765

flyCliErr_t   FlyCliParse       (const flyCli_t *pCli);
flyCliErr_t   FlyCliParseEx     (const flyCli_t *pCli, int *pNumArgs, int *pIndex);
void          FlyCliHelp        (const flyCli_t *pCli);
const char   *FlyCliArg         (const flyCli_t *pCli, int index);
int           FlyCliNumArgs     (const flyCli_t *pCli);
int           FlyCliDoubleDash  (const flyCli_t *pCli);

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  }
#endif

#endif // FLY_CLI_H
