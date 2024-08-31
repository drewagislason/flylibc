/**************************************************************************************************
  FlyAssert.c - Custom Asserts with or without stack trace
  Copyright (c) 2024 Drew Gislason
  license: <https://mit-license.org>
**************************************************************************************************/
#include "Fly.h"
#include "FlyAssert.h"

/*!
  @mainpage   FireFly C Library and Tools
  @version    1.0
  @logo       ![flylibc](flylibc_logo.png "w3-circle w3-grayscale-max")

  A set of C algorithms and tools to make developing C or C++ programs easier

  This library relies only upon the standard C library. It is written to the C99 standard.
*/

/*!
  @defgroup FlyAssert   An Assert API which allows control of output

  Features:

  1. Set an exit function for "cleanup" or file save
  2. Determine exactly how output looks
  3. Use "simple" assert with file ID and line only to reduce space
  4. Use "full" assert that includes file, function, line, expression
  5. Debug level asserts so only most important are left in production code
*/

/*!------------------------------------------------------------------------------------------------
  Default assert print function.

  @param    szxExpr     assert expression or helpful message
  @param    szFile      string version of the file
  @param    szFunc      string function name
  @param    line        line number
  @return   1 (error code for failed)
*///-----------------------------------------------------------------------------------------------
int FlyErrorPrint(const char *szExpr, const char *szFile, const char *szFunc, unsigned line)
{
  printf("%s:%u:1: assert: %s, func %s()\n", szFile, line, szExpr, szFunc);
  return 1;
}

static pfnFlyAssert_t m_pfnAssert = FlyErrorPrint;

/*!------------------------------------------------------------------------------------------------
  Set the exit function when an assert happens. Note: exit function asserts are ignored.

  @param    pfnAssert   ptr to assert function to use on exit instead of default
  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyAssertSetExit(pfnFlyAssert_t pfnAssert)
{
  m_pfnAssert = pfnAssert;
}

/*!------------------------------------------------------------------------------------------------
  An Assert has occurred. Exit with error code. Prevents exit/shutdown functions from asserting
  recursively.

  @param    szxExpr     assert expression or helpful message
  @param    szFile      string version of the file
  @param    szFunc      string function name
  @param    line        line number
  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyError(const char *szExpr, const char *szFile, const char *szFunc, unsigned line)
{
  static int  m_fAssert = 0;
  int         err       = 1;

  if(!m_fAssert)
  {
    m_fAssert = 1;
    if(m_pfnAssert)
      err = m_pfnAssert(szExpr, szFile, szFunc, line);
    exit(err);
  }
}
