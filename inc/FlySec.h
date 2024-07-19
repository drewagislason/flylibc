/*!************************************************************************************************
  FlySec.h
  Copyright 2024 Drew Gislason
  license: MIT <https://mit-license.org>
*///***********************************************************************************************
#include "Fly.h"

#ifndef FLY_SEC_H
#define FLY_SEC_H

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

#define FLY_SEC_PREAMBLE_SIZE   8
#define FLY_SEC_NO_DATA         0
#define FLY_SEC_ERR             UINT_MAX
#define FLY_SEC_MAX_SIZE        0xfe00    // stream may not exceed this size

typedef void * hSec_t;
typedef bool_t (*pfnProcessHdr_t)(hSec_t hSec, const void *pHdr, unsigned hdrLen, void *pHdrAuxData);

#define FlySecRoundUp(n) (((n) + 0xf) & 0xffffffff0UL)
#define FLySecSize(hdrSize, dataSize) (FLY_SEC_PREAMBLE_SIZE + (hdrSize) + FlySecRoundUp(dataSize))
void             *FlySecNew           (unsigned maxPacketSize);
void              FlySecFree          (hSec_t hSec);
bool_t            FlySecIsSec         (const hSec_t hSec);
void              FlySecPrint         (const hSec_t hSec);

void              FlySecKeySet        (hSec_t hSec, const void *pKey, unsigned lenKey);
void              FlySecPwdRandom     (char *szPwd, unsigned sizePwd);

long              FlySecNonceGet      (const hSec_t hSec);
void              FlySecNonceNew      (hSec_t hSec);
void              FlySecNonceReset    (hSec_t hSec);
void              FlySecNonceSet      (hSec_t hSec, long randNumber);

unsigned          FlySecPad           (uint8_t *pData, unsigned len);
unsigned          FlySecPadRemove     (uint8_t *pData, unsigned len);

unsigned          FlySecStreamSize    (const hSec_t hSec);
unsigned          FlySecStreamLeft    (const hSec_t hSec);
unsigned          FlySecStreamLen     (const hSec_t hSec);
const uint8_t    *FlySecStreamPtr     (const hSec_t hSec);
void              FlySecStreamFlush   (hSec_t hSec);
bool_t            FlySecStreamFeed    (hSec_t hSec, const void *pIn, unsigned len);

unsigned          FlySecEncode        (hSec_t hSec, void *pOut, const void *pHdr, unsigned hdrLen, const void *pData, unsigned dataLen);
unsigned          FlySecDecode        (hSec_t hSec, void *pOut, pfnProcessHdr_t pfnProcessHdr, void *pHdrAuxData);

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  }
#endif

#endif // FLY_SEC_H
