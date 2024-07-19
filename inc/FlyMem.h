/**************************************************************************************************
  FlyMem.h
  Copyright (c) 2024 Drew Gislason
  License: MIT <https://mit-license.org>
*///***********************************************************************************************
#ifndef FLY_MEM_H
#define FLY_MEM_H

#include "Fly.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  #define EXTERN_C extern "C" {
#endif

// alloc, free routines, not-defined means use stlib.h, -DFLY_ALLOC=0 use FlyLibAlloc.c, -DFLY_ALLOC=1 = user supplied
#ifndef FLY_ALLOC
 #define  FlyAlloc(nSize)          malloc(nSize)
 #define  FlyCalloc(nElem, nSize)  calloc((nElem), (nSize))
 #define  FlyFree(p)               free(p)
 #define  FlyRealloc(p, nSize)     realloc((p), (nSize))
#else
  void   *FlyAlloc        (size_t size);
  void   *FlyCalloc       (size_t nmemb, size_t size);
  void    FlyFree         (void *ptr);
  void   *FlyRealloc      (void *ptr, size_t size);
#endif

void     *FlyAllocZ       (size_t size);
void     *FlyFreeIf       (void *);

#ifdef __cplusplus
}
#endif

#endif // FLY_MEM_H
