/**************************************************************************************************
  FlyToml.h
  Copyright 2024 Drew Gislason
  license: MIT <https://mit-license.org>
*///***********************************************************************************************
#ifndef FLY_TOML_H
#define FLY_TOML_H

#include "FlyStr.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  #define EXTERN_C extern "C" {
#endif

#ifndef TOML_CFG_FLOAT
 #define TOML_CFG_FLOAT   0
#endif
#ifndef TOML_CFG_DATE
 #define TOML_CFG_DATE    0
#endif
#ifndef TOML_CFG_KEY_MAX
 #define TOML_CFG_KEY_MAX 64    // cannot exceed UINT_MAX
#endif

#if TOML_CFG_DATE
#include <time.h>
#endif

typedef enum
{
  TOML_UNKNOWN=0,  // must be 0 so memset(&key, 0, sizeof(key)) sets type to unknown
  TOML_FALSE,
  TOML_TRUE,
  TOML_INTEGER,
  TOML_STRING,
  TOML_ARRAY,
  TOML_INLINE_TABLE,
#if TOML_CFG_FLOAT
  TOML_FLOAT,
#endif
#if TOML_CFG_DATE
  TOML_DATE,
#endif
} tomlType_t;   // type of TOML value

typedef struct
{
  const char  *szKey;
  const char  *szValue;
  tomlType_t  type;
} tomlKey_t;    // key/value pair

typedef struct
{
  const char *szValue;
  tomlType_t  type;
} tomlValue_t;

bool_t        FlyTomlArrayIndex       (const char *szTomlArray, unsigned index, tomlValue_t *pValue);
const char   *FlyTomlArrayIter        (const char *szTomlArray, tomlValue_t *pValue);
bool_t        FlyTomlKeyFind          (const char *szTomlTable, const char *szKeyName, tomlKey_t *pKey);
const char   *FlyTomlKeyIter          (const char *szTomlTable, tomlKey_t *pKey);
bool_t        FlyTomlKeyPathFind      (const char *szTomlFile,  const char *szKeyPath, tomlKey_t *pKey);
const char   *FlyTomlTableFind        (const char *szTomlFile,  const char *szTableName);
const char   *FlyTomlTableIter        (const char *szTomlFile,  const char *szPrevTable);
bool_t        FlyTomlTableIsRoot      (const char *szTomlTable);

void          FlyTomlKeyInit          (tomlKey_t   *pKey);
void          FlyTomlValueInit        (tomlValue_t *pValue);

tomlType_t    FlyTomlType             (const char *szTomlValue);
bool_t        FlyTomlValue            (const char *szTomlKey, tomlValue_t *pValue);
unsigned      FlyTomlKeyLen           (const char *szTomlKey);
unsigned      FlyTomlKeyCpy           (char *szDst, const char *szTomlKey, unsigned size);
unsigned      FlyTomlStrLen           (const char *szTomlStr);
unsigned      FlyTomlStrCpy           (char *szDst, const char *szTomlStr, unsigned size);
const char *  FlyTomlPtr              (const char *szTomlStr);

bool_t        FlyTomlAtoBool          (const char *szTomlBool);
long          FlyTomlAtol             (const char *szTomlNumber);
#if TOML_CFG_DATE
bool_t        FlyTomlAtoDate          (const char *szTomlDate, struct tm *pDate);
#endif
#if TOML_CFG_FLOAT
double        FlyTomlAtof             (const char *szTomlFloat);
#endif

#ifdef __cplusplus
}
#endif

#endif // FLY_TOML_H
