/**************************************************************************************************
  FlySemVer.h
  Copyright (c) 2024 Drew Gislason
  License: MIT <https://mit-license.org>
*///***********************************************************************************************
#ifndef FLY_SEM_VER_H
#define FLY_SEM_VER_H

#include "FlyStr.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  #define EXTERN_C extern "C" {
#endif

int       FlySemVerCmp      (const char *szVer1, const char *szVer2);
bool_t    FlySemVerMatch    (const char *szRange, const char *szVer);
bool_t    FlySemVerIsValid  (const char *szVer);
void      FlySemVerHigh     (char *szHigh, const char *szRange, unsigned size);
unsigned  FlySemVerCpy      (char *szDst, const char *szSemVer, unsigned size);

#ifdef __cplusplus
}
#endif

#endif // FLY_SEM_VER_H
