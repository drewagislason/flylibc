/**************************************************************************************************
  FlyBase64.h
  Copyright (c) 2024 Drew Gislason
  license: MIT <https://mit-license.org>
*///***********************************************************************************************
#ifndef FLY_BASE64_H
#define FLY_BASE64_H
#include "Fly.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

size_t FlyBase64Encode(char *szBase64, size_t size, const uint8_t *pBinary, size_t binLen);
size_t FlyBase64Decode(uint8_t *pBinary, const char *szBase64, size_t size);

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  }
#endif

#endif // FLY_BASE64_H
