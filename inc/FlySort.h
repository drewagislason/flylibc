/*!************************************************************************************************
  FlySort.h
  Copyright 2024 Drew Gislason
  license: MIT <https://mit-license.org>
*///***********************************************************************************************
#include "Fly.h"

#ifndef FLY_SORT_H
#define FLY_SORT_H

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

// see also FlySort.h, as it uses this type
typedef int  (*pfnSortCmp_t)    (const void *pThis, const void *pThat);
typedef int  (*pfnSortCmpEx_t)  (void *pArg, const void *pThis, const void *pThat);

// Note: basic compare functions can be found in FlyList
void    FlySortBubble   (void *pArray, unsigned nElem, unsigned elemSize, pfnSortCmp_t pfnCmp);
void    FlySortQSort    (void *pArray, size_t nElem, size_t elemSize, void *pArg, pfnSortCmpEx_t pfnCmp);
void   *FlySortList     (void *pList, bool_t fIsCircular, bool_t fIsDouble, void *pArg, pfnSortCmpEx_t pfnCmp);

#ifndef FLY_FLAG_NO_MATH
int     FlySortCmpDouble    (const void *pThis, const void *pThat);
int     FlySortCmpDoubleEx  (void *pArg, const void *pThis, const void *pThat);
#endif

int     FlySortCmpInt       (const void *pThis, const void *pThat);
int     FlySortCmpStr       (const void *pThis, const void *pThat);
int     FlySortCmpUnsigned  (const void *pThis, const void *pThat);
int     FlySortCmpIntEx     (void *pArg, const void *pThis, const void *pThat);
int     FlySortCmpStrEx     (void *pArg, const void *pThis, const void *pThat);
int     FlySortCmpUnsignedEx(void *pArg, const void *pThis, const void *pThat);

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  }
#endif

#endif // FLY_SORT_H
