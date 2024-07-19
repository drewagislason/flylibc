/**************************************************************************************************
  FlyJson.h
  Copyright 2024 Drew Gislason
  license: MIT <https://mit-license.org>
**************************************************************************************************/

#ifndef FLY_JSON_H
#define FLY_JSON_H
#include <ctype.h>
#include "Fly.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

// floating point libraries can be large. Optional.
#ifndef FLYJSON_CFG_FLOAT
  #define FLYJSON_CFG_FLOAT    0
#endif

#ifndef FLYJSON_MAX_LEVEL
  #define FLYJSON_MAX_LEVEL   12
#endif

typedef enum
{
  FLYJSON_ARRAY,
  FLYJSON_BOOL,
  FLYJSON_FLOAT,
  FLYJSON_NULL,
  FLYJSON_NUMBER,
  FLYJSON_OBJ,
  FLYJSON_STRING,
  FLYJSON_INVALID
} flyJsonType_t;

typedef void * hFlyJson_t;

// JSON input
bool_t          FlyJsonIsJson       (const char *szJson);
const char     *FlyJsonGet          (const char *szKeyPath, flyJsonType_t *pType);
const char     *FlyJsonGetObj       (const char *sz);
const char     *FlyJsonGetKey       (const char *szObj, size_t index);
const char     *FlyJsonGetValuePtr  (const char *szKey, flyJsonType_t *pType);
size_t          FlyJsonGetCount     (const char *szObj);
const char     *FlyJsonGetScalar    (const char *szArray, size_t index, flyJsonType_t *pType);
bool_t          FlyJsonGetBool      (const char *szBool);
#if FLYJSON_CFG_FLOAT
double          FlyJsonGetFloat     (const char *szFloat);
#endif
long            FlyJsonGetNumber    (const char *szNumber);
size_t          FlyJsonStrLen       (const char *szJsonStr);
int             FlyJsonStrCmp       (const char *sz, const char *szJsonStr);
size_t          FlyJsonStrNCpy      (char *szDst, const char *szJsonStr, size_t n);

// JSON output
hFlyJson_t      FlyJsonNew          (char *szDst, size_t maxSize, bool_t fPretty);
bool_t          FlyJsonIsHandle     (hFlyJson_t hJson);
//void            FlyJsonLogShow      (hFlyJson_t hJson);
void            FlyJsonFree         (hFlyJson_t hJson);
size_t          FlyJsonPut          (hFlyJson_t hJson, const char *szKey, flyJsonType_t type, const void *pValue);
size_t          FlyJsonPutScalar    (hFlyJson_t hJson, flyJsonType_t type, const void *pValue);
size_t          FlyJsonPutBegin     (hFlyJson_t hJson, flyJsonType_t type);
size_t          FlyJsonPutEnd       (hFlyJson_t hJson, flyJsonType_t type);

#ifdef __cplusplus
  }
#endif

#endif // FLY_JSON_H
