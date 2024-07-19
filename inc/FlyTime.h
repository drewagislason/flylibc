/**************************************************************************************************
  @file FlyTime.h
  Copyright 2024 Drew Gislason
  license: MIT <https://mit-license.org>
*///***********************************************************************************************
#ifndef FLY_TIME_H
#define FLY_TIME_H

#include "Fly.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  #define EXTERN_C extern "C" {
#endif

#ifndef FLY_TIME_TYPE
typedef size_t flytime_t;
#endif

#define FLY_TIME_EPOCH_SIZE       26

flytime_t       FlyTimeSeedRandom     (void);

flytime_t       FlyTimeMsDiff         (flytime_t timeMs);
flytime_t       FlyTimeMsGet          (void);
void            FlyTimeMsSleep        (flytime_t milliseconds);

flytime_t       FlyTimeUsDiff         (flytime_t timeUs);
flytime_t       FlyTimeUsGet          (void);
void            FlyTimeUsSleep        (flytime_t microseconds);

flytime_t       FlyTimeEpoch          (long *pMicroseconds);
void            FlyTimeEpochStr       (flytime_t epoch, char *szDst, unsigned size);
void            FlyTimeEpochStrLocal  (flytime_t epoch, char *szDst, unsigned size);
void            FlyTimeEpochStrIso    (flytime_t epoch, char *szDst, unsigned size);

#ifdef __cplusplus
  }
#endif

#endif // FLY_TIME_H
