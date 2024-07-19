/**************************************************************************************************
  Fly.h
  Copyright (c) 2024 Drew Gislason
  license: MIT <https://mit-license.org>
*///***********************************************************************************************
#ifndef FLY_H
#define FLY_H

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

// not defined in older compilers
#ifndef __STDC_VERSION__
  #ifdef __cplusplus
    #define __STDC_VERSION__ __cplusplus
  #else
    #define __STDC_VERSION__ 198901L
  #endif
#endif

// determine platform
#define FLY_WINDOWS   0
#define FLY_MACOS     1
#define FLY_LINUX     2
#ifdef _WIN32
  #define FLY_PLATFORM    FLY_WINDOWS
  #define FLY_OS   "Windows"
#else
  #ifdef __APPLE__
    #define FLY_PLATFORM  FLY_MACOS
    #define FLY_OS   "macOs"
  #else
    #define FLY_PLATFORM  FLY_LINUX
    #define FLY_OS   "Linux"
  #endif
#endif

// common includes
#include <stdio.h>        // for printf(), etc..
#include <stdint.h>       // for uint32_t, uint8_t, etc..
#include <limits.h>       // for LONG_MAX, etc...
#include <stdlib.h>       // for malloc(), etc..
#include <string.h>       // for memcpy(), etc..
#include <errno.h>

#define FLY_VER        "0.9.6"
#define FLY_VERNUM     906      // mod 100 for minor version, e.g. 100 is "1.0", 321 is "3.21"

// debug off if not defined on cmdline
#define DEBUG_MIN       1
#define DEBUG_MORE      2
#define DEBUG_MAX       3
#ifndef DEBUG
 #define DEBUG          0
#endif

// boolean
typedef enum
{
  FALSE = 0,
  TRUE = 1
} bool_t;

#define UNUSED(var) (void)var

// number of elements in an array of any kind
#define NumElements(a) (sizeof(a)/sizeof((a)[0]))

// allow user function to be called upon assert failure
typedef int (*pfnFlyAssert_t)(const char *szExpr, const char *szFile, const char *szFunc, unsigned line);
void FlyAssertSetExit(pfnFlyAssert_t pfnAssert);

int FlyDbgPrintf(const char *szFormat, ...);

// assert and errors
void       FlyError(const char *szExpr, const char *szFile, const char *szFunc, unsigned line);
#define    FlyAssert(expr)          ((expr) ? (void)0 : FlyError(#expr, __FILE__, __func__, __LINE__))
#define    FlyAssertFail(msg)       FlyError(#msg, __FILE__, __func__, __LINE__)
#if DEBUG
  #define  FlyAssertDbg(expr)       ((expr) ? (void)0 : FlyError(#expr, __FILE__, __func__, __LINE__))
  #define  FlyAssertDbgFail(msg)    FlyError(#msg, __FILE__, __func__, __LINE__)
#else
  #define  FlyAssertDbg(expr)
  #define  FlyAssertDbgFail(msg)
#endif

#ifdef __cplusplus
  }
#endif

#endif // FLY_H
