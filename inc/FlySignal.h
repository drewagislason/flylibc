/**************************************************************************************************
  FlySignal.c
  Copyright 2024 Drew Gislason  
  license: <https://mit-license.org>
*///***********************************************************************************************
#ifndef FLY_SIGNAL_H
#define FLY_SIGNAL_H

#include "Fly.h"
#include <signal.h>

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  #define EXTERN_C extern "C" {
#endif

#define FLYSIG_MAX_STACK_FRAMES  128

typedef int (*pfnFlySigOnExit_t)(int sig);

void  FlySigSetExit     (const char *szProgName, pfnFlySigOnExit_t pfnSigOnExit);
void  FlySigStackTrace  (void);

#ifdef __cplusplus
}
#endif

#endif // FLY_SIGNAL_H
