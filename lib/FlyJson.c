/**************************************************************************************************
  FlyJson.c - Parse and write JSON files
  Copyright 2024 Drew Gislason
  license: <https://mit-license.org>
**************************************************************************************************/

/*!
  @defgroup FlyJson   A simple API for converting JSON to C and C to JSON

  Key Features

  * Output either compressed (no spaces) or "pretty" (e.g. python -m json.tool [infile] [outfile])
  * Supports JSON as described at <www.json.org>
  * Self-contained, only Fly.h, FlyJson.h, FlyJson.c and some standard C library calls are used
  * Optionally supports floats (off by default)
  * Supports packed JSON or any amount of whitespace

  Limitations (subset of JSON)

  * Numbers are long int or double. You can typecast them into something smaller
  * JSON Floats are disabled by default, see FLYJSON_CFG_FLOAT (code size reduction)
  * Output "depth" is limited by FLYJSON_MAX_LEVEL (does not affect input depth)
  * Total keypath limited to PATH_MAX, see FlyJsonGet()

  See also: python -m json.tool [-h] [--sort-keys] [--json-lines] [infile] [outfile]

  @example  FlyJsonPut

  ```
  Insert example here...
  ```
  
*/
#include "Fly.h"
#include "FlyLog.h"
#include "FlyMem.h"
#include "FlyJson.h"

static const char m_szNull[]     = "null";
static const char m_szTrue[]     = "true";
static const char m_szFalse[]    = "false";

#define FLYJSON_SANCHK    7502

// object (state) for json output
typedef struct
{
  unsigned    sanchk;
  char       *szDst;
  char       *szTmpLine;
  size_t      len;
  size_t      maxSize;
  bool_t      fPretty;
  unsigned    indent;   // indent for each level
  unsigned    level;    // how many objects in?
  unsigned    count[FLYJSON_MAX_LEVEL];
} flyJson_t;

static const char      *NjMatchBrace    (const char *szObj, size_t *pCount);
static const char      *NjSkipKeyVal    (const char *psz);
static const char      *NjSkipValue     (const char *psz);
static flyJsonType_t    NjGetType       (const char *psz);
static const char      *NjSkipBrace     (const char *psz);

/*-------------------------------------------------------------------------------------------------
  If this is a '[' or '{', returns TRUE
-------------------------------------------------------------------------------------------------*/
bool_t NjIsBrace(char cBrace)
{
  return (cBrace == '[' || cBrace == '{') ? TRUE : FALSE;
}

/*-------------------------------------------------------------------------------------------------
  Returns closing brace
-------------------------------------------------------------------------------------------------*/
char NjClosingBrace(char cBrace)
{
  char  closingBrace = '\0';
  if(NjIsBrace(cBrace))
    closingBrace = (cBrace == '{') ? '}' : ']';
  return closingBrace;
}

/*-------------------------------------------------------------------------------------------------
  Skip whitespace
-------------------------------------------------------------------------------------------------*/
static const char * NjSkipWhite(const char *psz)
{
  while(isspace(*psz))
    ++psz;
  return psz;
}

/*-------------------------------------------------------------------------------------------------
  Skips a string. Ends up on character after "closing quote"
-------------------------------------------------------------------------------------------------*/
static const char * NjSkipString(const char *psz)
{
  psz  = NjSkipWhite(psz);
  if(*psz == '"')
  {
    ++psz;
    while(*psz && *psz != '"')
    {
      // handle esc chars, e.g. \"
      if(*psz == '\\')
        ++psz;
      ++psz;
    }
    if(*psz == '"')
      ++psz;
  }

  return psz;
}

/*-------------------------------------------------------------------------------------------------
  Skips the key. Ends up on the value. "key": "value" would end up on "value"
-------------------------------------------------------------------------------------------------*/
static const char * NjSkipKey(const char *psz)
{
  psz = NjSkipString(psz);
  psz = NjSkipWhite(psz);
  if(*psz == ':')
  {
    ++psz;
    psz = NjSkipWhite(psz);
  }

  return psz;
}

/*-------------------------------------------------------------------------------------------------
  Skips object or array {...} or [...]. Ends up on 1st white-space after closing brace
-------------------------------------------------------------------------------------------------*/
static const char * NjSkipBrace(const char *psz)
{
  char            cClosingBrace;
  bool_t          fObj;

  psz = NjSkipString(psz);
  psz = NjSkipWhite(psz);
  if(NjIsBrace(*psz))
  {
    cClosingBrace = NjClosingBrace(*psz);
    fObj = (cClosingBrace == '}') ? TRUE : FALSE;
    ++psz;
    while(*psz)
    {
      psz = NjSkipWhite(psz);
      if(*psz == cClosingBrace)
      {
        ++psz;
        break;
      }

      // object should be filled with "key":"value" pairs
      else if(fObj)
      {
        // not a key value, cannot continue to process
        if(*psz != '"')
          break;
        psz = NjSkipKeyVal(psz);
      }

      // arrays are filled base types (bool, number, string, array or obj)
      else
      {
        // not a valid type, cannot continue to process
        if(NjGetType(psz) == FLYJSON_INVALID)
          break;
        psz = NjSkipValue(psz);
      }
    }

    psz = NjSkipWhite(psz);
  }

  return psz; 
}

/*-------------------------------------------------------------------------------------------------
  Returns type (e.g. FLYJSON_BOOL or FLYJSON_STRING). Returns FLYJSON_INVALID if not a valid type
-------------------------------------------------------------------------------------------------*/
static flyJsonType_t NjGetType(const char *psz)
{
  flyJsonType_t   type = FLYJSON_INVALID;

  // FlyLogPrintf("psz %s\n", psz);

  if(*psz == '-' || (*psz >= '0' && *psz <= '9'))
    type = FLYJSON_NUMBER;
  else if(*psz == '"')
    type = FLYJSON_STRING;
  else if(*psz == '{')
    type = FLYJSON_OBJ;
  else if(*psz == '[')
    type = FLYJSON_ARRAY;
  else if(strncmp(psz, m_szNull, sizeof(m_szNull) - 1) == 0)
    type = FLYJSON_NULL;
  else if((strncmp(psz, m_szTrue,  sizeof(m_szTrue)  - 1) == 0) || 
          (strncmp(psz, m_szFalse, sizeof(m_szFalse) - 1) == 0))
  {
    type = FLYJSON_BOOL;
  }

  return type;
}

/*-------------------------------------------------------------------------------------------------
  Skip JSON value, trailing comma, and trailing whitespace. Stops on end of array or object
-------------------------------------------------------------------------------------------------*/
static const char * NjSkipValue(const char *szValue)
{
  szValue = NjSkipWhite(szValue);

  if(*szValue == '"')
    szValue = NjSkipString(szValue);
  else if(NjIsBrace(*szValue))
    szValue = NjSkipBrace(szValue);

  // skips scalars and the ending ,
  while(*szValue)
  {
    if(*szValue == ',')
    {
      ++szValue;
      break;
    }

    if(*szValue == ']' || *szValue == '}')
    {
      break;
    }

    ++szValue;
  }

  return NjSkipWhite(szValue);
}

/*-------------------------------------------------------------------------------------------------
  Skip "key":"value", 
-------------------------------------------------------------------------------------------------*/
static const char * NjSkipKeyVal(const char *psz)
{
  psz = NjSkipKey(psz);
  psz = NjSkipValue(psz);
  return psz;
}

/*-------------------------------------------------------------------------------------------------
  Return ptr to matching brace. Also returns count. Recursive

  szObj points to '[' or '{'. If pCount, then count is returned (might be 0).

  Returns NULL if doesn't begin on a brace or matching closing brace not found
-------------------------------------------------------------------------------------------------*/
static const char * NjMatchBrace(const char *szObj, size_t *pCount)
{
  const char     *pszMatch  = NULL;
  size_t          count     = 0;
  const char     *psz;
  const char     *pszLast;
  char            closingBrace;
  bool_t          fObj;

  szObj = NjSkipWhite(szObj);

  if(NjIsBrace(*szObj))
  {
    closingBrace = NjClosingBrace(*szObj);
    // FlyLogPrintf("NjMatchBrace(%s, closingBrace %c\n", szObj, closingBrace);
    fObj = (closingBrace == '}') ? TRUE : FALSE;
    psz = NjSkipWhite(szObj + 1);
    while(*psz && *psz != closingBrace)
    {
      pszLast = psz;

      if(fObj)
        psz = NjSkipKeyVal(psz);
      else
        psz = NjSkipValue(psz);

      // if stuck (didn't move forward), then invalid JSON file
      if(psz == pszLast)
        break;

      ++count;
    }

    // return matching
    if(*psz == closingBrace)
      pszMatch = psz;
  }

  if(pCount)
    *pCount = count;
  return pszMatch;
}

/*!------------------------------------------------------------------------------------------------
  Returns TRUE if JSON (at least 1 object or scalar), FALSE if not

  @param    szBool      points to "true" or "false"
  @return   TRUE if "true", FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlyJsonIsJson(const char *szJson)
{
  bool_t        fIsJson = TRUE;
  const char   *szObj;

  szObj = FlyJsonGetObj(szJson);
  if(!szObj)
    fIsJson = FALSE;
  else if(FlyJsonGetCount(szObj) == 0)
    fIsJson = FALSE;

  return fIsJson;
}

/*!------------------------------------------------------------------------------------------------
  Gets a JSON object from a JSON input string

  @param    szJson      points to the beginning of a JSON file or object
  @return   TRUE if "true", FALSE if not
*///-----------------------------------------------------------------------------------------------
const char *FlyJsonGetObj(const char *szJson)
{
  const char *szObj = NjSkipWhite(szJson);
  return NjIsBrace(*szObj) ? szObj : NULL;
}

/*!------------------------------------------------------------------------------------------------
  Return a pointer to the indexed key, or NULL if out of range or contains scalars

  @param    szObj     points to '[' or '{'
  @param    index     index into array
  @return   pointer indexed to key, or NULL if index out of range
*///-----------------------------------------------------------------------------------------------
const char *FlyJsonGetKey(const char *szObj, size_t index)
{
  const char     *szKey = NULL;
  const char     *psz;

  szObj = NjSkipWhite(szObj);
  if(*szObj == '[' || *szObj == '{')
  {
    psz = szObj + 1;
    while(TRUE)
    {
      psz = NjSkipWhite(psz);
      if(*psz != '"')
        break;

      if(index == 0)
      {
        szKey = psz;
        break;
      }

      psz = NjSkipKeyVal(psz);
      --index;
    }
  }

  return szKey;
}

/*!------------------------------------------------------------------------------------------------
  Assuming a "key":"value", get ptr to the value and its type

  @param    szBool      points to "true" or "false"
  @return   ptr to value, NULL if not a key or invalid value
*///-----------------------------------------------------------------------------------------------
const char * FlyJsonGetValuePtr(const char *szKey, flyJsonType_t *pType)
{
  flyJsonType_t       type = FLYJSON_INVALID;
  const char         *szValue = NULL;

  szKey = NjSkipWhite(szKey);
  if(*szKey == '"')
  {
    szValue = NjSkipKey(szKey);
    type = NjGetType(szValue);
    if(type == FLYJSON_INVALID)
      szValue = NULL;
  }

  if(pType)
    *pType = type;
  return szValue;
}

/*!------------------------------------------------------------------------------------------------
  Count of keys (OBJ) or scalars (ARRAY)

  Exmaples: { "key": 99, "key2": "val" } is count 2, [-1, 123, 55] is count 3

  @param    szObj      points to "{" or "["
  @return   count of elements in object or array
*///-----------------------------------------------------------------------------------------------
size_t FlyJsonGetCount(const char *szObj)
{
  size_t    count = 0;

  szObj = NjSkipWhite(szObj);
  NjMatchBrace(szObj, &count);
  return count;
}

/*!------------------------------------------------------------------------------------------------
  Get the indexed element in the array. Returns NULL if not an array or index out of bounds

  Example: [99, false, { "key": "val}, [-5, 2]], index 2 would point to { of type FLYJSON_OBJ

  @param    szArray       points to "["
  @param    index         0-n
  @param    pType         pointer to receive type
  @return   ptr to value: number, bool, string, obj, array; and type (e.g. FLYJSON_BOOL)
*///-----------------------------------------------------------------------------------------------
const char *FlyJsonGetScalar(const char *szArray, size_t index, flyJsonType_t *pType)
{
  const char   *szValue = NULL;
  const char   *psz;

  szArray = NjSkipWhite(szArray);
  if(*szArray == '[')
  {
    psz = NjSkipWhite(szArray + 1);
    while(*psz && *psz != ']')
    {
      if(index == 0)
      {
        szValue = psz;
        break;
      }
      psz = NjSkipValue(psz);
      --index;
    }
  }

  // return the type
  if(pType && szValue)
    *pType = NjGetType(szValue);

  return szValue;
}

/*!------------------------------------------------------------------------------------------------
  Get a bool from a JSON file. Must point to the bool.

  @param    szBool      points to "true" or "false"
  @return   TRUE if "true", FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlyJsonGetBool(const char *szBool)
{
  bool_t  fBool = FALSE;
  if(strncmp(szBool, m_szTrue, strlen(m_szTrue)) == 0)
    fBool = TRUE;
  return fBool;
}

/*!------------------------------------------------------------------------------------------------
  Get a number from a JSON file. Must point to the number.
  For floating point (well, double) numbers, see FlyJsonInGetFloat()

  @param    szNumber      points to a number like "-1" or "9223372036854775807"
  @return   TRUE if "true", FALSE if not
*///-----------------------------------------------------------------------------------------------
long FlyJsonGetNumber(const char *szNumber)
{
  long  number = atol(szNumber);
  return number;
}

/*!------------------------------------------------------------------------------------------------
  Get a copy of the JSON "string", removing quotes.

  @param    szDst       points to a character array to receive the string
  @param    szJsonStr   points to string in a JSON file. "my string"
  @param    n           usually sizeof(szDst)
  @return   TRUE if "true", FALSE if not
*///-----------------------------------------------------------------------------------------------
size_t FlyJsonStrLen(const char *szJsonStr)
{
  size_t  len  = 0;

  if(*szJsonStr == '"')
  {
    ++szJsonStr;
    while(*szJsonStr)
    {
      // done with string
      if(*szJsonStr == '"')
        break;

      // handle escaped chars, so we don't end on \"
      if(*szJsonStr == '\\')
      {
        ++szJsonStr;
        ++len;
      }

      if(*szJsonStr)
        ++szJsonStr;
      ++len;
    }
  }

  return len;
}

/*!------------------------------------------------------------------------------------------------
  Compare a normal string and a quoted JSON string. Otherwise, same as strcmp()

  @param    sz          points to a '\0' terminated string
  @param    szJsonStr   points to string in a JSON file, e.g. "my string"
  @return   TRUE if "true", FALSE if not
*///-----------------------------------------------------------------------------------------------
int FlyJsonStrCmp(const char *sz, const char *szJsonStr)
{
  int   ret = -1;

  szJsonStr = NjSkipWhite(szJsonStr);
  if(*szJsonStr == '"')
  {
    ++szJsonStr;
    while(*sz && *szJsonStr != '"')
    {
      if(*sz < *szJsonStr)
      {
        ret = -1;
        break;
      }
      else if(*sz > *szJsonStr)
      {
        ret = 1;
        break;
      }
      ++sz;
      ++szJsonStr;
    }

    if(*szJsonStr == '"')
    {
      if(*sz)
        ret = 1;
      else
        ret = 0;
    }
  }

  return ret;
}

/*!------------------------------------------------------------------------------------------------
  Get a copy of the JSON "string", removing quotes. Always NULL terminates.

  @param    szDst       points to a character array to receive the string
  @param    szJsonStr   points to string in a JSON file. "my string"
  @param    n           usually sizeof(szDst)
  @return   length of string (I know, not the same as strncpy()
*///-----------------------------------------------------------------------------------------------
size_t FlyJsonStrNCpy(char *szDst, const char *szJsonStr, size_t n)
{
  size_t    len = 0;
  char     *szDstOrg = szDst;

  if(*szJsonStr == '"' && n > 1)
  {
    ++szJsonStr;
    while(*szJsonStr && *szJsonStr != '"' && len < (n - 1))
    {
      *szDst++ = *szJsonStr++;
      ++len;
    }
  }

  szDstOrg[len] = '\0';
  return len;
}

/*-------------------------------------------------------------------------------------------------
  Internal function to deal with NULL szDst

  @return   ptr to a safe place to put a symbol or line
-------------------------------------------------------------------------------------------------*/
char * _JsonGetPutPtr(flyJson_t *pJson)
{
  char *psz;

  if(pJson->szDst)
    psz = &pJson->szDst[pJson->len];
  else
    psz = pJson->szTmpLine;

  return psz;
}

/*!------------------------------------------------------------------------------------------------
  Create a new JSON output object. Used for all JSON output functions

  @param    szDst       can be NULL if just getting size output would be
  @param    fPretty     pack the output, or make it pretty (more human readable)
  @param    indent      indent (in spaces) per level. 4 is a good number
  @return   NULL if failed, handle to object otherwise
*///-----------------------------------------------------------------------------------------------
hFlyJson_t FlyJsonNew(char *szDst, size_t maxSize, bool_t fPretty)
{
  flyJson_t   *pJson;
  char         *szTmpLine = NULL;

  pJson     = FlyAlloc(sizeof(*pJson));
  if(szDst == NULL)
    szTmpLine = FlyAlloc(PATH_MAX);

  // everything that needed to be allocated was
  if(pJson && (szDst != NULL || szTmpLine != NULL))
  {
    memset(pJson, 0, sizeof(*pJson));
    pJson->sanchk    = FLYJSON_SANCHK;
    pJson->szDst     = szDst;
    pJson->szTmpLine = szTmpLine;
    if(szTmpLine)
      memset(szTmpLine, 0, PATH_MAX);
    pJson->fPretty   = fPretty;
    pJson->maxSize   = maxSize;
    pJson->indent    = 4;
    if(pJson->szDst)
      *pJson->szDst = '\0';
  }

  // something failed to allocate
  else
  {
    if(pJson)
    {
      FlyFree(pJson);
      pJson = NULL;
    }
    if(szTmpLine)
      FlyFree(szTmpLine);
  }
  return pJson;
}

/*!------------------------------------------------------------------------------------------------
  Free the JSON object

  @param    hJson       a JSON object created by FlyJsonNew()  
  @return   none
*///-----------------------------------------------------------------------------------------------
bool_t FlyJsonIsHandle(hFlyJson_t hJson)
{
  flyJson_t   *pJson = hJson;

  return (pJson && pJson->sanchk == FLYJSON_SANCHK) ? TRUE : FALSE;
}

#if 0
/*-------------------------------------------------------------------------------------------------
  Display the JSON object to the log

  @param    hJson       a JSON object created by FlyJsonNew()
  
  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyJsonLogShow(hFlyJson_t hJson)
{
  flyJson_t   *pJson = hJson;
  unsigned      i;

  if(!FlyJsonIsHandle(hJson))
  {
    FlyLogPrintf("!!ERR: Bad hFlyJson_t %p\n", pJson);
  }
  else
  {
    FlyLogPrintf("hJson %p: len %zu, fPretty %u, level %u, count [", pJson, pJson->len,
        pJson->fPretty, pJson->level);
    for(i=0; i <= pJson->level; ++i)
      FlyLogPrintf("%s%u", i ? "," : "", pJson->count[i]);
    if(pJson->szDst)
      FlyLogPrintf("]\nszDst =\n%s\n", pJson->szDst);
    else
      FlyLogPrintf("], szDst = NULL\n");
  }
}
#endif

/*!------------------------------------------------------------------------------------------------
  Free the JSON object

  @param    hJson       a JSON object created by FlyJsonNew()  
  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyJsonFree(hFlyJson_t hJson)
{
  flyJson_t   *pJson = hJson;

  FlyAssertDbg(FlyJsonIsHandle(hJson));
  if(pJson)
  {
    if(pJson->szTmpLine)
      FlyFree(pJson->szTmpLine);
    FlyFree(pJson);
  }
}

/*!------------------------------------------------------------------------------------------------
  Put a JSON key. For FLYJSON_ARRAY and FLYJSON_OBJ, this is the opener of the array or object.
  Close it with FlyJsonPutEnd

  @param    hJson       a JSON object created by FlyJsonNew()
  @param    szKey       key for a key:value pair
  @param    pValue      pointer to value if scalar (ignored on FLYJSON_ARRAY, FLYJSON_OBJ, etc...)
  @return   size of the output
*///-----------------------------------------------------------------------------------------------
size_t FlyJsonPut(hFlyJson_t hJson, const char *szKey, flyJsonType_t type, const void *pValue)
{
  flyJson_t   *pJson   = hJson;
  size_t        len     = 0;
  char         *szSpace;
  char         *psz;

  // if bad handle or level is too deep, do nothing
  if(FlyJsonIsHandle(hJson))
  {
    FlyAssertDbg(pJson->level < FLYJSON_MAX_LEVEL);

    // output key
    psz = _JsonGetPutPtr(pJson);
    szSpace = pJson->fPretty ? " " : "";
    if(pJson->count[pJson->level] != 0)
      len += sprintf(&psz[len], ",");
    if(pJson->fPretty)
      len += sprintf(&psz[len], "\n%*s", pJson->indent * pJson->level, "");
    len += sprintf(&psz[len], "\"%s\":%s", szKey, szSpace);

    // # of items in object or array
    pJson->count[pJson->level] += 1;

    switch(type)
    {
      case FLYJSON_ARRAY:
        len += sprintf(&psz[len], "[");
        FlyAssert(pJson->level + 1 < FLYJSON_MAX_LEVEL);
        ++pJson->level;
        pJson->count[pJson->level] = 0;
      break;

      case FLYJSON_BOOL:
        len += sprintf(&psz[len], "%s", *((bool_t *)pValue) ? m_szTrue : m_szFalse);
      break;

#if FLYJSON_CFG_FLOAT
      case FLYJSON_FLOAT:
        len += sprintf(&psz[len], "%f", *((double *)pValue));
      break;
#endif

      case FLYJSON_NULL:
        len += sprintf(&psz[len], "%s", m_szNull);
      break;

      case FLYJSON_NUMBER:
        len += sprintf(&psz[len], "%ld", *((long *)pValue));
      break;

      case FLYJSON_OBJ:
        len += sprintf(&psz[len], "{");
        FlyAssert(pJson->level + 1 < FLYJSON_MAX_LEVEL);
        ++pJson->level;
        pJson->count[pJson->level] = 0;
      break;

      case FLYJSON_STRING:
        len += sprintf(&psz[len], "\"%s\"", (char *)pValue);
      break;

      default:
        FlyAssertFail();
      break;
    }

    pJson->len += len;

    // FlyLogPrintf("FlyJsonPut(type %zu)\n", type);
    // FlyJsonLogShow(pJson);
  }

  return len;
}

/*!------------------------------------------------------------------------------------------------
  Put a JSON scalar value.

  @param    hJson       a JSON object created by FlyJsonNew()
  @param    type        must be scalar type (FLYJSON_BOOL, FLYJSON_NUMBER, etc...)
  @param    pValue      pointer to value
  @return   size of the output
*///-----------------------------------------------------------------------------------------------
size_t FlyJsonPutScalar(hFlyJson_t hJson, flyJsonType_t type, const void *pValue)
{
  flyJson_t   *pJson   = hJson;
  size_t        len     = 0;
  char         *psz;

  // if bad handle or level is too deep, do nothing
  if(FlyJsonIsHandle(hJson))
  {
    FlyAssertDbg(pJson->level < FLYJSON_MAX_LEVEL);

    // output key
    psz = _JsonGetPutPtr(pJson);
    if(pJson->count[pJson->level] != 0)
      len += sprintf(&psz[len], ",");
    if(pJson->fPretty)
      len += sprintf(&psz[len], "\n%*s", pJson->indent * pJson->level, "");

    // # of items in object or array
    pJson->count[pJson->level] += 1;

    switch(type)
    {
      case FLYJSON_BOOL:
        len += sprintf(&psz[len], "%s", *((bool_t *)pValue) ? m_szTrue : m_szFalse);
      break;

#if FLYJSON_CFG_FLOAT
      case FLYJSON_FLOAT:
        len += sprintf(&psz[len], "%f", *((double *)pValue));
      break;
#endif

      case FLYJSON_NULL:
        len += sprintf(&psz[len], "%s", m_szNull);
      break;

      case FLYJSON_NUMBER:
        len += sprintf(&psz[len], "%ld", *((long *)pValue));
      break;

      case FLYJSON_STRING:
        len += sprintf(&psz[len], "\"%s\"", (char *)pValue);
      break;

      default:
        FlyAssertFail();
      break;
    }

    pJson->len += len;

    // FlyLogPrintf("FlyJsonPutScalar(type %zu)\n", type);
    // FlyJsonLogShow(pJson);
  }

  return len;
}


/*!------------------------------------------------------------------------------------------------
  Used at start of JSON file which is always '{' or '['

  @param    hJson       a JSON object created by FlyJsonNew()
  @param    type        either FLYJSON_ARRAY or FLYJSON_OBJ  
  @return   TRUE if this is an obj
*///-----------------------------------------------------------------------------------------------
size_t FlyJsonPutBegin(hFlyJson_t hJson, flyJsonType_t type)
{
  flyJson_t   *pJson = hJson;
  size_t        len   = 0;
  char         *psz;

  if(FlyJsonIsHandle(hJson))
  {
    FlyAssert(pJson->level + 1 < FLYJSON_MAX_LEVEL);

    psz = _JsonGetPutPtr(pJson);
    switch(type)
    {
      case FLYJSON_ARRAY:
        len += sprintf(&psz[len], "[");
      break;

      case FLYJSON_OBJ:
        if(pJson->level > 0)
        {
          if(pJson->count[pJson->level] != 0)
            len += sprintf(&psz[len], ",");
          if(pJson->fPretty)
            len += sprintf(&psz[len], "\n%*s", pJson->indent * pJson->level, "");
        }
        len += sprintf(&psz[len], "{");
        pJson->count[pJson->level] += 1;
      break;

      default:
        FlyAssertFail();
      break;
    }

    // for both 
    ++pJson->level;
    pJson->count[pJson->level] = 0;
    pJson->len += len;

    // FlyLogPrintf("FlyJsonPutBegin(type %zu)\n", type);
    // FlyJsonLogShow(pJson);
  }

  return len;
}

/*!------------------------------------------------------------------------------------------------
  End the array or object

  @param    hJson       a JSON object created by FlyJsonNew()
  @param    type        either FLYJSON_ARRAY or FLYJSON_OBJ  
  @return   TRUE if this is an obj
*///-----------------------------------------------------------------------------------------------
size_t FlyJsonPutEnd(hFlyJson_t hJson, flyJsonType_t type)
{
  flyJson_t   *pJson = hJson;
  size_t        len   = 0;
  char         *psz;
  unsigned      count;

  if(FlyJsonIsHandle(hJson))
  {
    psz = _JsonGetPutPtr(pJson);
    count = pJson->count[pJson->level];

    if(pJson->level)
      --pJson->level;

    if(pJson->fPretty && count)
    {
      len += sprintf(&psz[len], "\n%*s", pJson->indent * pJson->level, "");
    }

    switch(type)
    {
      case FLYJSON_ARRAY:
        len += sprintf(&psz[len], "]");
      break;
      case FLYJSON_OBJ:
        len += sprintf(&psz[len], "}");
      break;
      default:
      FlyAssertFail();
      break;
    }
    pJson->len += len;

    // FlyLogPrintf("FlyJsonPutEnd(type %zu)\n", type);
    // FlyJsonLogShow(pJson);
  }

  return len;
}
