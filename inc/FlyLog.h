/**************************************************************************************************
  FlyLog.h
  Copyright 2024 Drew Gislason
  license: MIT <https://mit-license.org>
*///***********************************************************************************************
#ifndef FLY_LOG_H
#define FLY_LOG_H

#include "Fly.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

#define FLY_LOG_NAME   "test.log"

typedef unsigned flyLogMask_t;    // bit mask contents is up to higher layer

const char *    FlyLogDefaultName(void);
bool_t          FlyLogFileOpen   (const char *szFilePath);
bool_t          FlyLogFileAppend (const char *szFilePath);
void            FlyLogFileClose  (void);
bool_t          FlyLogClear      (void);
int             FlyLogPrintf     (const char *szFormat, ...);
int             FlyLogPrintfEx   (flyLogMask_t mask, const char *szFormat, ...);
size_t          FlyLogHexDump    (const void *pData, unsigned len, unsigned linelen, unsigned indent);
size_t          FlyLogHexDumpEx  (flyLogMask_t mask, const void *pData, unsigned len, unsigned linelen, unsigned indent);
flyLogMask_t    FlyLogMaskSet    (flyLogMask_t mask);
bool_t          FlyLogMaskMatch  (flyLogMask_t mask);
flyLogMask_t    FlyLogMaskGet    (void);
size_t          FlyLogSizeGet    (void);
void            FlyLogSizeReset  (void);

#ifdef __cplusplus
  }
#endif

#endif // FLY_LOG_H
