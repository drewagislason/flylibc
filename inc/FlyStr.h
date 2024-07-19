/**************************************************************************************************
  FlyStr.h
  Copyright 2024 Drew Gislason  
  License: MIT <https://mit-license.org>  
*///***********************************************************************************************
#ifndef FLY_STR_H
#define FLY_STR_H

#include <ctype.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <strings.h>
#include "Fly.h"
#include "FlyMem.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

typedef struct
{
  char       *sz;
  size_t      size;
} flyStrSmart_t;

#define FLY_STR_TAB_SIZE  8   // tabs default to 8 spaces

#if DEBUG
 #define FLYSTRSMART_FILL  '@'
#else
 #define FLYSTRSMART_FILL  '\0'
#endif

typedef enum
{
  IS_LOWER_CASE = 0,    // lower
  IS_UPPER_CASE,        // UPPER
  IS_CAMEL_CASE,        // camelCase
  IS_MIXED_CASE,        // MixedCase
  IS_SNAKE_CASE,        // snake_case
  IS_CONSTANT_CASE,     // CONSTANT_CASE
} flyStrCase_t;

#define FLYSTRSMART_RIGHT INT_MAX
flyStrSmart_t    *FlyStrSmartAlloc    (size_t size);
char             *FlyStrSmartCat      (flyStrSmart_t *pStr, const char *sz);
void              FlyStrSmartClear    (flyStrSmart_t *pStr);
flyStrSmart_t    *FlyStrSmartCombine  (flyStrSmart_t *pStr, const flyStrSmart_t *pStr2);
char             *FlyStrSmartCpy      (flyStrSmart_t *pStr, const char *sz);
flyStrSmart_t    *FlyStrSmartDup      (flyStrSmart_t *pStr);
void              FlyStrSmartFit      (flyStrSmart_t *pStr);
void             *FlyStrSmartFree     (flyStrSmart_t *pStr);
void              FlyStrSmartInit     (flyStrSmart_t *pStr);
bool_t            FlyStrSmartInitEx   (flyStrSmart_t *pStr, size_t size);
char             *FlyStrSmartNCat     (flyStrSmart_t *pStr, const char *sz, size_t len);
char             *FlyStrSmartNCpy     (flyStrSmart_t *pStr, const char *sz, size_t len);
bool_t            FlyStrSmartNeed     (flyStrSmart_t *pStr, size_t len);
flyStrSmart_t    *FlyStrSmartNew      (const char *sz);
flyStrSmart_t    *FlyStrSmartNewEx    (const char *sz, size_t size);
bool_t            FlyStrSmartResize   (flyStrSmart_t *pStr, size_t size);
flyStrSmart_t    *FlyStrSmartSlice    (flyStrSmart_t *pStr, int left, int right);
int               FlyStrSmartSprintf  (flyStrSmart_t *pStr, const char *szFmt, ...);
void              FlyStrSmartUnInit   (flyStrSmart_t *pStr);

typedef enum  // note: used as a bitmask. See FlyStrReplace()
{
  FLYSTR_REP_ONCE      = 0, // replace first found in string
  FLYSTR_REP_ALL,           // replace all found in string
  FLYSTR_REP_ONCE_CASE,     // replace once, ignoring case
  FLYSTR_REP_ALL_CASE       // replace all found in string, ignoring case
} flyStrReplaceOpt_t;

bool_t            FlyCharIsCName      (char c);
bool_t            FlyCharIsDozenal    (char c);
bool_t            FlyCharIsEol        (char c);
bool_t            FlyCharIsInSet      (char c, const char *szSet);
char              FlyCharPrev         (const char *sz);
char             *FlyCharEsc          (const char *sz, uint8_t *pByte);
char             *FlyCharOct          (const char *sz, uint8_t *pByte);
char             *FlyCharHex          (const char *sz, uint8_t *pByte);
char              FlyCharHexDigit     (uint8_t nybble);

unsigned          FlyStrChrCount      (const char *sz, char c);
unsigned          FlyStrChrCountRev   (const char *szStart, const char *sz, char c);


// camelCase, MixedCase, snake_case, UPPER, lower
flyStrCase_t      FlyStrIsCase        (const char *sz);
size_t            FlyStrToCase        (char *szDst, const char *szSrc, size_t size, flyStrCase_t strCase);
void              FlyStrToLower       (char *sz);

// argument handling
const char       *FlyStrArgCpy        (char *szDst, const char *szSrc, size_t size);
int               FlyStrArgCmp        (const char *sz1, const char *sz2);
size_t            FlyStrArgLen        (const char *sz);
char             *FlyStrArgNext       (const char *sz);
const char       *FlyStrArgEnd        (const char *sz);
const char       *FlyStrArgBeg        (const char *szStart, const char *sz);
const char       *FlyStrSkipWhite     (const char *sz);
const char       *FlyStrSkipWhiteEx   (const char *sz);
const char       *FlyStrSkipChars     (const char *sz, const char *szChars);
char             *FlyStrSkipNumber    (const char *sz);
const char       *FlyStrSkipString    (const char *sz);

// line handling
char             *FlyStrLineBeg       (const char *szFile, const char *sz);
void              FlyStrLineBlankRemove(char *sz);
char             *FlyStrLineChr       (const char *sz, char c);
size_t            FlyStrLineCount     (const char *szFile, const char *sz);
char             *FlyStrLineGoto      (const char *szFile, size_t line);
char             *FlyStrLineEnd       (const char *sz);
const char       *FlyStrLineEnding    (void);
char             *FlyStrLineEof       (const char *sz);
size_t            FlyStrLineIndent    (const char *sz, unsigned tabSize);
bool_t            FlyStrLineIsBlank   (const char *sz);
size_t            FlyStrLineLen       (const char *sz);
size_t            FlyStrLineLenEx     (const char *sz);
char             *FlyStrLineNext      (const char *szLine);
unsigned          FlyStrLinePos       (const char *szFile, const char *sz, unsigned *pCol);
char             *FlyStrLinePrev      (const char *szLine, const char *sz);
char             *FlyStrLineSkipBlank (const char *sz);
char             *FlyStrLineStr       (const char *szHaystack, const char *szNeedle);

// ISO 8601 date/time functions
const char       *FlyStrDateTime      (time_t time);
const char       *FlyStrDateTimeCur   (void);

// string path functions
bool_t            FlyStrPathAppend    (char *szPath, const char *szName, size_t size);
bool_t            FlyStrPathChangeExt (char *szPath, const char *szNewExt);
bool_t            FlyStrPathChangeName(char *szPath, const char *szNewName);
char             *FlyStrPathExt       (const char *szPath);
const char       *FlyStrPathHasExt    (const char *szPath, const char *szExts);
const char       *FlyStrPathHome      (void);
bool_t            FlyStrPathHomeExpand(char *szPath, size_t size);
bool_t            FlyStrPathIsFolder  (const char *szPath);
bool_t            FlyStrPathIsRelative(const char *szPath);
const char       *FlyStrPathLang      (const char *szPath);
char             *FlyStrPathNameLast  (const char *szPath, unsigned *pLen);
char             *FlyStrPathNameOnly  (const char *szPath);
char             *FlyStrPathNameBase  (const char *szPath, unsigned *pLen);
void              FlyStrPathOnly      (char *szPath);
char             *FlyStrPathOnlyLen   (const char *szPath, int *pLen);
bool_t            FlyStrPathParent    (char *szPath);

#define FLYMEM_NO_DIFF  SIZE_MAX

// mem type things
bool_t            FlyMemIsFilled      (const void *s, int c, size_t n);
void             *FlyMemRChr          (const void *s, int c, size_t n);
int               FlyMemICmp          (const void *pThis, const void *pThat, size_t len);
size_t            FlyMemRemoveExtraSpaces(char *pData, size_t len);
size_t            FlyMemFindWrap      (const char *pLine, size_t len, size_t wrapWidth);
void              FlyMemSwap          (void *pThis, void *pThat, size_t size);
size_t            FlyMemDiff          (const void *pThis, const void *pThat, size_t size);

// things that use the heap
char             *FlyStrClone         (const char *sz);
char             *FlyStrAlloc         (const char *sz);
char             *FlyStrAllocN        (const char *sz, size_t len);
char             *FlyStrAllocAppend   (char *szAllocStr, const char *sz, size_t len);
char             *FlyStrFree          (char *sz);

// StrZ functions guarantee asciiz '\0' terminated result, allows NULL szDst to calc length
size_t            FlyStrZCpy          (char *szDst, const char *szSrc, size_t size);
size_t            FlyStrZCat          (char *szDst, const char *szSrc, size_t size);
size_t            FlyStrZNCpy         (char *szDst, const char *szSrc, size_t size, size_t srcLen);
size_t            FlyStrZNCat         (char *szDst, const char *szSrc, size_t size, size_t srcLen);
size_t            FlyStrZFill         (char *szDst, char c, size_t size, size_t fillLen);
size_t            FlyStrZCatFill      (char *szDst, char c, size_t size, size_t fillLen);

// miscellaneous string functions
char              FlyStrCharLast      (const char *sz);
size_t            FlyStrReplace       (char *szHaystack, size_t size, const char *szNeedle, const char *szWith, flyStrReplaceOpt_t opts);
bool_t            FlyCharIsSlug       (char c);
extern const char g_szFlyStrSlugChars [];
unsigned          FlyStrSlug          (char *szSlug, const char *szSrc, size_t size, size_t srcLen);
const char       *FlyStrCVer          (void);
size_t            FlyStrIns           (char *szDst, size_t offset, size_t sizeDst, const char *szSrc);
int               FlyStrCmp           (const char *szThis, const char *szThat);
char             *FlyStrNChr          (const char *sz, size_t len, char c);
char             *FlyStrNChrMatch     (const char *sz, const char *szEnd, const char *szMatch);
int               FlyStrICmp          (const char *szThis, const char *szThat);
const char       *FlyStrNullOk        (const char *sz);
char             *FlyStrBlankOf       (const char *sz);
size_t            FlyStrWhereDiff     (const char *szThis, const char *szThat, size_t n);
void             *FlyStrFreeIf        (void *ptr);
char             *FlyStrLastSlash     (const char *szPath);
char             *FlyStrNextSlash     (const char *szPath);
char             *FlyStrPrevSlash     (const char *szPath, const char *psz);
const char       *FlyStrFit           (char *szDst, size_t width, const char *szSrc);
bool_t            FlyStrIsSlash       (char c);
bool_t            isslash             (char c);
void              FlyStrBlankRemove   (char *sz);
char             *FlyStrAsk           (char *szAnswer, const char *szQuestion, size_t size);
size_t            FlyStrCount         (const char *szHaystack, const char *szNeedle);
const char       *FlyStrTrueFalse     (bool_t fFlag);
unsigned          FlyStrLToStr        (char *szDst, long n, size_t size, int base);
long              FlyStrNToL          (const char *szNum, char **ppszEnd, int base,
                                       unsigned len, unsigned *pDigits, const char *szIgnore);
void              FlyStrRev           (char *sz);
char             *FlyStrEscNCpy       (char *szDst, const char *szSrc, size_t n, size_t *pNBytes);
char             *FlyStrEscEndQuoted  (const char *sz);
char             *FlyStrNStr          (const char *szHaystack, const char *szNeedle, size_t len);
char             *StrSkipNumber       (const char *sz);

// dump: 00008050  01 02 00 00 00 5f 70 72  69 6e 74 66 00 00 00 00  |....._printf....|
#define FLYSTR_DUMP_COLS      16
#define FlyStrDumpLineSize(cols) ((sizeof(long)*2) + 8 + ((cols)*4))
#define FLYSTR_DUMP_LINE_LEN FlyStrDumpLineSize(FLYSTR_DUMP_COLS)
void              FlyStrDump          (const void *pData, size_t len);
void              FlyStrDumpEx        (const void *pData, size_t len, char *szLine, unsigned cols, long addr);
unsigned          FlyStrDumpLine      (char *szLine, const void *pData, unsigned len, unsigned cols, long addr);

// arrays of string pointers
char             *FlyStrArrayCombine  (int count, const char **asz, const char *szDelim);
char            **FlyStrArrayCopy     (const char **asz);
void              FlyStrArrayFree     (const char **asz);
int               FlyStrArrayFind     (const char **aszHaystack, const char *azNeedle);
int               FlyStrArrayCmp      (const char **aszThis, const char **aszThat);
char            **FlyStrArrayMalloc   (size_t numElem, size_t sizeOf);
char             *FlyStrArrayText     (const char **asz);
size_t            FlyStrArrayNumElem  (const char **asz);
size_t            FlyStrArraySizeOf   (const char **asz);
size_t            FlyStrArrayMaxLen   (const char **asz);
void              FlyStrArrayPrint    (const char **asz);

// for parsing source file block comments (aka headers)
typedef enum
{
  FLYSTRHDR_TYPE_NONE = 0,
  FLYSTRHDR_TYPE_C,         // /*
  FLYSTRHDR_TYPE_PYDOC,     // """ or """!
  FLYSTRHDR_TYPE_HASH,      // # line comment
  FLYSTRHDR_TYPE_RUST,      /// rust type comment
} flyStrHdrType_t;

typedef struct
{
  const char     *szRawHdrLine;     // pointer beginning of raw hdr line
  const char     *szStartLine;      // line AFTER end of contents
  const char     *szEndLine;        // start of contents
  const char     *szRawHdrEnd;      // line AFTER end of block comment/header
  size_t          indent;
  flyStrHdrType_t type;
  bool_t          fIsDoc;
} flyStrHdr_t;

// block comment processing
const char       *FlyStrHdrFind         (const char *szLine, bool_t fIsDocOnly, flyStrHdr_t *pHdr);
size_t            FlyStrHdrCpy          (char *szDst, const flyStrHdr_t *pHdr, size_t size);
const char       *FlyStrHdrCpyPos       (const char *szDst, const flyStrHdr_t *pHdr, const char *pszInDst);

// getters for block comment processing
const char       *FlyStrHdrContentStart (const flyStrHdr_t *pHdr);
const char       *FlyStrHdrContentEnd   (const flyStrHdr_t *pHdr);
const char       *FlyStrRawHdrLine      (const flyStrHdr_t *pHdr);
const char       *FlyStrRawHdrEnd       (const flyStrHdr_t *pHdr);
size_t            FlyStrHdrIndent       (const flyStrHdr_t *pHdr);
bool_t            FlyStrHdrIsDoc        (const flyStrHdr_t *pHdr);
const char       *FlyStrHdrSkip         (const flyStrHdr_t *pHdr);
const char       *FlyStrHdrText         (const flyStrHdr_t *pHdr, const char *szLine);
flyStrHdrType_t   FlyStrHdrType         (const flyStrHdr_t *pHdr);

// for programming language processing
size_t            FlyStrFnProtoLen    (const char *szLine, const char **ppszCName);
size_t            FlyStrCNameLen      (const char *sz);

#ifdef __cplusplus
  }
#endif

#endif // FLY_UTIL_STR_H
