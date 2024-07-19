/**************************************************************************************************
  Flyutf8.h
  Copyright 2024 Drew Gislason
  license: MIT <https://mit-license.org>
*///***********************************************************************************************
#ifndef FLY_UTF8_H
#define FLY_UTF8_H

#include "FlyStr.h"
#include <inttypes.h>

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  #define EXTERN_C extern "C" {
#endif

#define UTF8_MAX            5
#define UTF8_INVALID        0xfffe
#define UTF8_REPLACEMENT    0xfffd
#define UTF8_CODEPOINT_MAX  0x10FFFFL
typedef char utf8_t;


// UTF8 handling
const utf8_t   *FlyUtf8CharCpy    (utf8_t *szDst, const utf8_t *szSrc);
const utf8_t   *FlyUtf8CharEsc    (utf8_t *szDst, const utf8_t *szSrc, unsigned *pDstLen);
uint32_t        FlyUtf8CharGet    (const utf8_t *szUtf8);
const utf8_t   *FlyUtf8CharIdx    (const utf8_t *szUtf8, size_t i);
unsigned        FlyUtf8CharLen    (const utf8_t *szUtf8);
const utf8_t   *FlyUtf8CharNext   (const utf8_t *szUtf8);
unsigned        FlyUtf8CharPut    (utf8_t *szUtf8, uint32_t codePoint);
unsigned        FlyUtf8Len        (uint32_t codePoint);
size_t          FlyUtf8StrLen     (const utf8_t *szUtf8);
size_t          FlyUtf8StrZCpy    (utf8_t *szDst, const utf8_t *szSrc, size_t size);
unsigned        FlyUtf8SlugCpy    (utf8_t *szSlug, const char *szSrc, size_t size, size_t srcLen);

#ifdef __cplusplus
}
#endif

#endif // FLY_UTF8_H
