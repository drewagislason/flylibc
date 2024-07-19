/**************************************************************************************************
  FlySignal.c - Signal handling made easy
  Copyright 2024 Drew Gislason  
  license: <https://mit-license.org>
*///***********************************************************************************************
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <err.h>
#include <execinfo.h>
#include "FlySignal.h"

/*!
  @defgroup FlySignal - Signal handling made easy

  Allows you to trap or ignore various signals, rather than your program abruptly aborting
*/


static pfnFlySigOnExit_t  m_pfnSigOnExit;
static const char        *m_szProgName = NULL;

#if 0
static void                *m_stackTraces[NEDSIG_MAX_STACK_FRAMES];

/*-------------------------------------------------------------------------------------------------
  Resolve symbol name and source location given the path to the executable and an address

  @return 0 if worked
-------------------------------------------------------------------------------------------------*/
int NsAddr2Line(char const * const szProgName, void const * const addr)
{
  char szAdd2LineCmd[512] = {0};
 
  /* have addr2line map the address to the relent line in the code */
  #ifdef __APPLE__
    /* apple does things differently... */
    snprintf(szAdd2LineCmd, sizeof(szAdd2LineCmd) - 1, "atos -o %.256s %p", szProgName, addr); 
  #else
    snprintf(szAdd2LineCmd, sizeof(szAdd2LineCmd) - 1, "addr2line -f -p -e %.256s %p", szProgName, addr); 
  #endif
  szAdd2LineCmd[sizeof(szAdd2LineCmd) - 1] = '\0';
 
  /* This will print a nicely formatted string specifying the function and source line of the address */
  return system(szAdd2LineCmd);
}
#endif

/*!------------------------------------------------------------------------------------------------
  Display stack trace to stdout. Resolves symbol name and source location given the path to the
  executable and an address. Only works if NedSigSetExit() has been called 1st.

  @return   none
*///-----------------------------------------------------------------------------------------------
void FlySigStackTrace(void)
{
  char  **aszStrs;
  void   *callstack[FLYSIG_MAX_STACK_FRAMES];
  int     i;
  int     frames;

  frames = backtrace(callstack, FLYSIG_MAX_STACK_FRAMES);
  aszStrs = backtrace_symbols(callstack, frames);
  for (i = 0; i < frames; ++i)
  {
    fputs(aszStrs[i], stdout);
    fputs("\n", stdout);
  }

  free(aszStrs);
}

/*-------------------------------------------------------------------------------------------------
  Generic signal handler
-------------------------------------------------------------------------------------------------*/
void FlySignalHandler(int sig)
{
  int exitCode = 1;

  //  fputs("\033[0m \n", stdout);
  switch(sig)
  {
    case SIGABRT:
      fputs("\nCaught SIGABRT: usually caused by an abort() or assert()\n", stdout);
    break;
    case SIGFPE:
      fputs("\nCaught SIGFPE: arithmetic exception, such as divide by zero\n", stdout);
    break;
    case SIGILL:
      fputs("\nCaught SIGILL: illegal instruction\n", stdout);
    break;
    case SIGINT:
      fputs("\nCaught SIGINT: interactive attention signal, probably a ctrl+c\n", stdout);
    break;
    case SIGSEGV:
      fputs("\nCaught SIGSEGV: segfault\n", stdout);
    break;
    case SIGTERM:
    default:
      fputs("\nCaught SIGTERM: a termination request was sent to the program\n", stdout);
    break;
  }

  FlySigStackTrace();

  // user routine
  if(m_pfnSigOnExit)
    exitCode = (*m_pfnSigOnExit)(sig);

  _Exit(exitCode);
}
 
/*!------------------------------------------------------------------------------------------------
  Set Signal Handlers to point to given 

  @return   none
-------------------------------------------------------------------------------------------------*/
void FlySigSetHandlers(void)
{
  signal(SIGABRT, FlySignalHandler);
  signal(SIGFPE,  FlySignalHandler);
  signal(SIGILL,  FlySignalHandler);
  signal(SIGINT,  FlySignalHandler);
  signal(SIGSEGV, FlySignalHandler);
  signal(SIGTERM, FlySignalHandler);
}

/*!------------------------------------------------------------------------------------------------
  This causes signals to be cause and display a stack track on signal exit.

  The optional exit function allows handling of the signal AFTER the stack trace and the reason is
  printed to stdout. To avoid stdout and the stack trace altogether, call C library `signal()`
  AFTER calling `FlySigSetExit()`.

  @param    szProgName    usually argv[0]
  @param    pfnOnExit     function to call on exit, or NULL to use 
  @return   none
*///-----------------------------------------------------------------------------------------------
void FlySigSetExit(const char *szProgName, pfnFlySigOnExit_t pfnOnExit)
{
  m_pfnSigOnExit = pfnOnExit;
  m_szProgName   = szProgName;
  FlySigSetHandlers();
}
