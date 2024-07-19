/**************************************************************************************************
  FlyTabComplete.h  
  Copyright 2024 Drew Gislason
  license: MIT <https://mit-license.org>
**************************************************************************************************/
#ifndef FLY_TAB_COMPLETE_H
#define FLY_TAB_COMPLETE_H

#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>

#include "Fly.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

void *        FlyTabCompleteNew     (unsigned maxSize);
bool_t        FlyTabComplete        (void *hTabComplete, char *szPath);
bool_t        FlyTabCompleteRewind  (void *hTabComplete);
void          FlyTabCompleteFree    (void *hTabComplete);

#ifdef __cplusplus
  }
#endif

#endif // FLY_TAB_COMPLETE_H
