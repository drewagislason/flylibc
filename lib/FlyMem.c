/**************************************************************************************************
  FlyMem.c - Help debug memory usage and errors
  Copyright 2024 Drew Gislason
  license: <https://mit-license.org>
**************************************************************************************************/
#include "FlyMem.h"

/*!
  @defgroup   FlyMem   Functions for customizing or debugging memory allocation

  Features:

  1. Statistics show how much memory has been allocated or freed
  2. Detects buffer overruns
  3. Option to fill on alloc to detect uninitialized fields
  4. Option to fill on free to detect use after free
  5. Option to use private heap for application, separate from C Library heap
*/

/*!------------------------------------------------------------------------------------------------
  Allocate and zero out some memory. Like calloc(), but 1 parameter.

  @param  size    size of memory to allocate (and zero out)
  @return ptr to memory if worked, NULL if failed.
*///-----------------------------------------------------------------------------------------------
void * FlyAllocZ(size_t size)
{
  void *p = FlyAlloc(size);
  if(p)
    memset(p, 0, size);
  return p;
}

/*!------------------------------------------------------------------------------------------------
  Free if the pointer is non-NULL. Always returns NULL.

  @param    p   ptr to allocated memory
  @return   NULL
*///-----------------------------------------------------------------------------------------------
void * FlyFreeIf(void *p)
{
  if(p)
    FlyFree(p);
  return NULL;
}
