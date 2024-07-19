/**************************************************************************************************
  FlySec.c - Application level end-to-end encryption
  Copyright 2024 Drew Gislason
  license: <https://mit-license.org>
**************************************************************************************************/
#include "Fly.h"
#include <time.h>
#include "FlyAes.h"
#include "FlyMem.h"
#include "FlyStr.h"
#include "FlySec.h"

/*!
  @defgroup FlySec   A secure interface that works with streams.

  It doesn't know about the transport protocol (could be TCP, UDP, something else). It simply
  encodes/decodes. Allows for custom header to facilitate key lookup. Uses random nonce for

  1. Allows for variable, application specific header (0-n bytes)
  2. Header can contain key lookup/hash info for decoding
  3. Uses symmetic AES-256 for all application data
  4. Takes stream input (feed), and provides application packet data output
  5. Detects tampering with any bits

  Preamble for all secure packets is fixed in size:

  ```
   0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
  |1 1 1 1 1 1 1 0|reserved |0 0 1|           crc-16              |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
  |          totalLen             |         hdrlLen               |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
  |          hdr...                                               |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
  |          encrypted padded data...                             |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
  ```

  The encrypted packets are binary. However, if you need to send it over HTML, you can use base64,
  a common format for converting binary to ascii. See FlyBase64.h.
*/
#define SEC_SYNC_BYTE     0xfe

#define FLY_SEC_SANCHK    936
typedef struct
{
  struct AES_ctx  ctx;
  uint8_t        *pStream;
  unsigned        streamSize; // 1 - n
  unsigned        streamLen;  // 0 - (streamSize - 1)
  unsigned        sanchk;
  long            nonce;
} flySec_t;

typedef enum
{
  SEC_ERR_NONE,
  SEC_ERR_INCOMPLETE,
  SEC_ERR_FUZZ
} secErr_t;

static secErr_t SecDecodePreamble   (const flySec_t *pSec, unsigned *pHdrLen, unsigned *pTotalLen);
static unsigned SecCrc              (void const *mem, unsigned len);

/*-------------------------------------------------------------------------------------------------
  A 16-bit CRC, based on the crc16dnp_bit algorithm
-------------------------------------------------------------------------------------------------*/
static unsigned SecCrc(void const *mem, unsigned len)
{
  uint8_t const  *data = mem;
  unsigned        crc = 0x1d0f;

  if(data == NULL)
    crc = 0xffff;
  else
  {
    crc = ~crc;
    crc &= 0xffff;
    while(len--)
    {
      crc ^= *data++;
      for(int i = 0; i < 8; ++i)
        crc = crc & 1 ? (crc >> 1) ^ 0xa6bc : crc >> 1;
    }
    crc ^= 0xffff;
  }

  return crc;
}

/*-------------------------------------------------------------------------------------------------
  Convert a uitn16_6 into a big endian array of bytes
-------------------------------------------------------------------------------------------------*/
static uint8_t * SecBigEndianUint16(uint8_t *pOut, uint16_t u)
{
  *pOut++ = (u >> 8) & 0xff;
  *pOut++ = u & 0xff;
  return pOut;
}

/*-------------------------------------------------------------------------------------------------
  Convert a big endian set of bytes to a uint16_t
-------------------------------------------------------------------------------------------------*/
static uint16_t SecBigEndianUint16Get(const uint8_t *pIn)
{
  return (((uint16_t)pIn[0] << 8) | (pIn[1]));
}

/*!------------------------------------------------------------------------------------------------
  Pads the string to an even 16 bytes. Assumes data can be appended up to 15 bytes.
  See https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS7

  @param    pData   data to pad
  @param    len     length of data
  @returns  new (potentially larger) length, filled with 1-0xf padded bytes
*///-----------------------------------------------------------------------------------------------
unsigned FlySecPad(uint8_t *pData, unsigned len)
{
  uint8_t nPadBytes;

  if(len & 0xf)
  {
    nPadBytes = 16 - (len & 0xf);
    memset(&pData[len], nPadBytes, nPadBytes);
    len += nPadBytes;
  }

  return len;
}

/*!------------------------------------------------------------------------------------------------
  Reduce length based on padding. If not valid padding, leaves length unchanged.

  @param    pData   padded data
  @param    len     length of padded data
  @returns  reduce length to ignore padding
*///-----------------------------------------------------------------------------------------------
unsigned FlySecPadRemove(uint8_t *pData, unsigned len)
{
  uint8_t   nPadBytes;
  uint8_t   i;

  if((len > 0) && ((len & 0xf) == 0))
  {
    i = nPadBytes = pData[len - 1];
    if((nPadBytes >= 1) && (nPadBytes <= 0xf))
    {
      for(i = 1; (i <= nPadBytes) && (pData[len - 1] == nPadBytes); ++i)
        --len;
    }
  }

  return len;
}

/*!------------------------------------------------------------------------------------------------
  Create a new security object

  @param    maxPacketSize   maximum size of user header + data
  @returns  handle to security object
*///-----------------------------------------------------------------------------------------------
hSec_t FlySecNew(unsigned maxPacketSize)
{
  static bool_t   fSeeded = FALSE;
  flySec_t       *pSec    = NULL;
  unsigned        streamSize;
  char            szPwd[12];  // 4294967295

  // seed with current time for random #
  if(!fSeeded)
  {
    srandom(time(NULL));
    fSeeded = TRUE;
  }

  if(maxPacketSize > 0)
  {
    // allow room for padding
    if(maxPacketSize & 0xf)
      maxPacketSize += (16 - (maxPacketSize & 0xf));
    streamSize = maxPacketSize;
    pSec = FlyAlloc(sizeof(flySec_t) + streamSize);
    if(pSec)
    {
      memset(pSec, 0, sizeof(flySec_t) + streamSize);
      pSec->sanchk = FLY_SEC_SANCHK;
      pSec->streamSize = streamSize;
      pSec->pStream = (uint8_t *)(pSec + 1);

      // random nonce and key
      FlySecPwdRandom(szPwd, sizeof(szPwd));
      FlySecKeySet(pSec, szPwd, sizeof(szPwd));
      FlySecNonceNew(pSec);
    }
  }
  return (hSec_t)pSec;
}

/*!------------------------------------------------------------------------------------------------
  Is this a valid pSec strucure?
  @param    hSec      Pointer to object returned by FlySecNew()
  @returns  TRUE if this is a valid pSec
*///-----------------------------------------------------------------------------------------------
bool_t FlySecIsSec(const hSec_t hSec)
{
  const flySec_t   *pSec = hSec;
  return (pSec && (pSec->sanchk == FLY_SEC_SANCHK)) ? TRUE : FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Free the secure structure
  @param    hSec      Pointer to object returned by FlySecNew()
  @returns  none
*///-----------------------------------------------------------------------------------------------
void FlySecFree(hSec_t hSec)
{
  flySec_t   *pSec = hSec;

  if(FlySecIsSec(hSec))
  {
    memset(pSec, 0, sizeof(flySec_t) + pSec->streamSize);
    FlyFree(pSec);
  }
}

/*!------------------------------------------------------------------------------------------------
  Print the security object
  @param    hSec      Pointer to object returned by FlySecNew()
  @returns  none
*///-----------------------------------------------------------------------------------------------
void FlySecPrint(const hSec_t hSec)
{
  const flySec_t   *pSec = hSec;
  if(FlySecIsSec(hSec))
    printf("pSec %p, nonce %ld, streamSize %u, streamLen %u, ctx\n", pSec, pSec->nonce, pSec->streamSize, pSec->streamLen);
  FlyStrDump(&pSec->ctx, sizeof(pSec->ctx));
}

/*!------------------------------------------------------------------------------------------------
  Hash into the key
  @param    hSec      Pointer to object returned by FlySecNew()
  @param    pKey      some key data, e.g. password (if NULL, key will be random)
  @param    lenKey    length of key data
  @returns  none
*///-----------------------------------------------------------------------------------------------
void FlySecKeySet(hSec_t hSec, const void *pKey, unsigned lenKey)
{
  flySec_t   *pSec = hSec;
  uint8_t     aKey[AES_KEYLEN];

  if(FlySecIsSec(hSec))
  {
    memset(aKey, 0, sizeof(aKey));
    if(lenKey > sizeof(aKey))
      lenKey = sizeof(aKey);
    if(lenKey)
      memcpy(aKey, pKey, lenKey);
    AES_init_ctx(&pSec->ctx, aKey);
  }
}

/*!------------------------------------------------------------------------------------------------
  Create a random password of a given size. The password is all ascii.

  @param    hSec     to object returned by FlySecNew()
  @returns  current nonce
*///-----------------------------------------------------------------------------------------------
void FlySecPwdRandom(char *szPwd, unsigned sizePwd)
{
  static const char szChars[] = "0123456789ABCDEF";
  int               i;

  memset(szPwd, 0, sizePwd);
  for(i = 1; i < sizePwd; ++i)
    *szPwd++ = szChars[random() % (sizeof(szChars) - 1)];
}

/*!------------------------------------------------------------------------------------------------
  Return the current nonce

  @param    hSec     to object returned by FlySecNew()
  @returns  current nonce
*///-----------------------------------------------------------------------------------------------
long FlySecNonceGet(const hSec_t hSec)
{
  flySec_t   *pSec  = hSec;
  long        nonce = 0;

  if(FlySecIsSec(hSec))
    nonce = pSec->nonce;

  return nonce;
}

/*!------------------------------------------------------------------------------------------------
  Create a new random nonce

  @param    hSec     to object returned by FlySecNew()
  @returns  none
*///-----------------------------------------------------------------------------------------------
void FlySecNonceNew(hSec_t hSec)
{
  flySec_t     *pSec = hSec;

  if(FlySecIsSec(hSec))
  {
    pSec->nonce = random();
    FlySecNonceReset(hSec);
  }  
}

/*!------------------------------------------------------------------------------------------------
  Reset to current nonce. Higher layer must manage counters if desired

  @param    hSec     to object returned by FlySecNew()
  @returns  current nonce
*///-----------------------------------------------------------------------------------------------
void FlySecNonceReset(hSec_t hSec)
{
  flySec_t     *pSec = hSec;
  uint8_t       aNonce[AES_BLOCKLEN];

  if(FlySecIsSec(pSec))
  { 
    memset(aNonce, 0, sizeof(aNonce));
    snprintf((char *)aNonce, sizeof(aNonce), "%ld", pSec->nonce);
    AES_ctx_set_iv(&pSec->ctx, aNonce);

    // printf("Nonce %ld\n", pSec->nonce);
    // FlyStrDump(aNonce, sizeof(aNonce));
  }
}

/*!------------------------------------------------------------------------------------------------
  The nonce is an additional random component to encryption

  @param    hSec          Pointer to object returned by FlySecNew()
  @param    randomNumber  a number to feed Nonce
  @returns  none
*///-----------------------------------------------------------------------------------------------
void FlySecNonceSet(hSec_t hSec, long randNumber)
{
  flySec_t     *pSec = hSec;

  if(FlySecIsSec(hSec))
  {
    pSec->nonce = randNumber;
    FlySecNonceReset(hSec);
  }  
}

/*!------------------------------------------------------------------------------------------------
  Encode the packet. The encryption header is fixed in size. The user header may be anything, but
  the user header + data + pad bytes must all fit in the maxPacketSize set upon FlySecNew().

  The packet will look like:

  `[pre][crc][totalLen][hdrLen][hdr...][encrypted padded data...]`

  @param    hSec      object returned by FlySecNew()
  @param    pBuf      outbuf, assumed to be FlySecStreamSize(hSec) in size
  @param    pHdr      ptr to user header
  @param    hdrLen    length of user header
  @param    pData     ptr to application data
  @param    dataLen   length of application data
  @returns  length of encoded packet, or 0 if invalid parameters or data too large
*///-----------------------------------------------------------------------------------------------
unsigned FlySecEncode(hSec_t hSec, void *pBuf, const void *pHdr, unsigned hdrLen, const void *pData, unsigned dataLen)
{
  flySec_t     *pSec = hSec;
  uint8_t      *pOut = pBuf;
  uint8_t       aBlock[AES_BLOCKLEN];
  unsigned      totalLen = 0;
  unsigned      i, n;
  unsigned      crc;

  // make sure length will fit
  if(FlySecIsSec(hSec))
  {
    // calculate total length. data may be padded
    totalLen = dataLen;
    if(dataLen & 0xf)
      totalLen += (16 - (dataLen & 0xf));
    totalLen += FLY_SEC_PREAMBLE_SIZE + hdrLen;

    // too long, can't encode/decode
    if(totalLen > pSec->streamSize)
      totalLen = 0;
  }

  // OK, total length looks OK, now build packet
  if(totalLen && pOut)
  {
    // make sure AES CTR is what user want's it to be
    FlySecNonceReset(hSec);

    // copy in the header
    if(pHdr && hdrLen)
      memcpy(&pOut[FLY_SEC_PREAMBLE_SIZE], pHdr, hdrLen);

    // copy, pad and encrypt the data
    for(i = 0; i < dataLen; i += sizeof(aBlock))
    {
      // last block might be short, pad it
      if(dataLen - i < sizeof(aBlock))
        n = dataLen - i;
      else
        n = sizeof(aBlock);
      memcpy(aBlock, &pData[i], n);
      n = FlySecPad(aBlock, n);
      AES_CTR_xcrypt_buffer(&pSec->ctx, aBlock, AES_BLOCKLEN);
      memcpy(&pOut[FLY_SEC_PREAMBLE_SIZE + hdrLen + i], aBlock, AES_BLOCKLEN);
    }

    // output the preamble
    pOut[0] = SEC_SYNC_BYTE;
    pOut[1] = 1;      // version 1
    crc = SecCrc(&pOut[FLY_SEC_PREAMBLE_SIZE], hdrLen + i);
    SecBigEndianUint16(&pOut[2], crc);
    SecBigEndianUint16(&pOut[4], totalLen);
    SecBigEndianUint16(&pOut[6], hdrLen);
  }

  return totalLen;
}

/*!------------------------------------------------------------------------------------------------
  Checks for valid packets. This does NOT affect the stream at all, only examines what is in it.
  Returns 1 of 3 states:

  SEC_ERR_READY         packet is ready in the stream
  SEC_ERR_INCOMPLETE    0 or more bytes of packet received, but not complete
  SEC_ERR_FUZZ          bad data is in the stream

  NOTE: if the CRC for the packet is wrong, even if everything else is OK, the entire packet is
  considered fuzz.

  NOTE: if there is more than 1 packet in the stream, that's OK. This only examines the 1st
  packet.

  @param    hSec        to object returned by FlySecNew()
  @param    hdrLen      returned value, size of header. OK if it's NULL if don't need the value
  @param    totalLen    returned value, size of packet or fuzz (1st bad byte), OK if it's NULL
  @returns  TRUE if packet looks OK so far 
*///-----------------------------------------------------------------------------------------------
static secErr_t SecDecodePreamble(const flySec_t *pSec, unsigned *pHdrLen, unsigned *pTotalLen)
{
  const uint8_t  *pStream = pSec->pStream;
  unsigned        crc;
  unsigned        hdrLen;
  unsigned        totalLen = 0;
  secErr_t        err = SEC_ERR_INCOMPLETE;

  // if preamble marker and version aren't there, it's fuzz (bad data)
  if(pSec->streamLen)
  {
    if(pStream[0] != SEC_SYNC_BYTE)
      err = SEC_ERR_FUZZ;
    else if((pSec->streamLen >= 2) && (pStream[1] != 0x01))
      err = SEC_ERR_FUZZ;      
  }

  // check for minimim packet length. if too short, packet is just incomplete
  if(pSec->streamLen >= (FLY_SEC_PREAMBLE_SIZE + AES_BLOCKLEN))
  {
    crc       = SecBigEndianUint16Get(&pStream[2]);
    totalLen  = SecBigEndianUint16Get(&pStream[4]);
    hdrLen    = SecBigEndianUint16Get(&pStream[6]);

    // hdrLen is always smaller than totalLen
    // totalLen cannot exceed max
    if(totalLen <= hdrLen || totalLen > pSec->streamSize || totalLen > FLY_SEC_MAX_SIZE)
      err = SEC_ERR_FUZZ;

    // encrypted data must always be at least 1 AES block
    else if(totalLen < (FLY_SEC_PREAMBLE_SIZE + hdrLen + AES_BLOCKLEN))
      err = SEC_ERR_FUZZ;

    // if all of packet is here, we can move on to CRC check
    else if(pSec->streamLen >= totalLen)
    {
      // bad crc, not a valid packet
      if(crc != SecCrc(&pStream[FLY_SEC_PREAMBLE_SIZE], totalLen - FLY_SEC_PREAMBLE_SIZE))
        err = SEC_ERR_FUZZ;

      // packet looks OK, return the size of header and 
      else
      {
        err = SEC_ERR_NONE;
        if(pHdrLen)
          *pHdrLen = hdrLen;
        if(pTotalLen)
          *pTotalLen = totalLen;
      }
    }
  }

  return err;
}

/*-------------------------------------------------------------------------------------------------
  Remove fuzz or data that hasn't been processed in a long time
-------------------------------------------------------------------------------------------------*/
static void SecFuzzRemove(flySec_t *pSec)
{
  unsigned    n;
  uint8_t     *p;

// printf("SecFuzzRemove, streamLen %u\n", pSec->streamLen);

  // keep removing bad data (fuzz) until we get to a potentially good preamble, or the stream is empty
  while(pSec->streamLen)
  {
// printf("byte %02x, streamLen %u\n", *pSec->pStream, pSec->streamLen);

    // if good (but possibly incomplete) preamble, we're done removing fuzz
    if(SecDecodePreamble(pSec, NULL, NULL) != SEC_ERR_FUZZ)
      break;

    // single fuzz byte, remove it
    else if(pSec->streamLen == 1)
      pSec->streamLen = 0;

    // look for another preamble. Perhaps this next one is OK
    else
    {
      // if no preamble in stream, throw out entire stream
      p = memchr(pSec->pStream + 1, SEC_SYNC_BYTE, pSec->streamLen - 1);
      if(!p)
      {
        pSec->streamLen = 0;
      }

      // remove fuzz, and leave potential preamble as 1st in stream
      else
      {
        n = (p - pSec->pStream);
        memmove(pSec->pStream, &pSec->pStream[n], pSec->streamLen - n);
        pSec->streamLen -= n;
      }
    }
  }
}

/*!------------------------------------------------------------------------------------------------
  Return the maximum packet size set on FlySecNew()
  @param    hSec      Pointer to object returned by FlySecNew()
  @returns  size of the stream for this security object
*///-----------------------------------------------------------------------------------------------
unsigned FlySecStreamSize(const hSec_t hSec)
{
  const flySec_t   *pSec = hSec;
  unsigned          streamSize = 0;
  if(FlySecIsSec(hSec))
    streamSize = pSec->streamSize;
  return streamSize;
}

/*!------------------------------------------------------------------------------------------------
  How much room is left in the incoming packet stream?

  @param    hSec      to object returned by FlySecNew()
  @returns  0-n bytes left in the stream
*///-----------------------------------------------------------------------------------------------
unsigned FlySecStreamLeft(const hSec_t hSec)
{
  const flySec_t   *pSec    = hSec;
  unsigned          streamLeft = 0;
  if(FlySecIsSec(hSec))
    streamLeft = pSec->streamSize - pSec->streamLen;
  return streamLeft;
}

/*!------------------------------------------------------------------------------------------------
  How much data is currently in the stream?

  @param    hSec      to object returned by FlySecNew()
  @returns  0-n bytes in the stream
*///-----------------------------------------------------------------------------------------------
unsigned FlySecStreamLen(const hSec_t hSec)
{
  const flySec_t   *pSec    = hSec;
  unsigned          streamLen = 0;
  if(FlySecIsSec(hSec))
    streamLen = pSec->streamLen;
  return streamLen;
}

const uint8_t * FlySecStreamPtr(const hSec_t hSec)
{
  const flySec_t   *pSec    = hSec;
  const uint8_t    *pStream = NULL;
  if(FlySecIsSec(hSec))
    pStream = pSec->pStream;
  return pStream;
}

/*!------------------------------------------------------------------------------------------------
  Flush (throw out) the input stream of all data

  @param    hSec     to object returned by FlySecNew()
  @returns  none
*///-----------------------------------------------------------------------------------------------
void FlySecStreamFlush(hSec_t hSec)
{
  flySec_t     *pSec = hSec;
  if(FlySecIsSec(hSec))
    pSec->streamLen = 0;
}

/*!------------------------------------------------------------------------------------------------
  Feed data into the stream. This is an all or nothing operation. If "len" amount of data won't fit
  in the stream, it's rejected. See also FlySecStreamLeft().

  Note: This will also remove fuzz (bad data in the stream)

  @param    hSec      to object returned by FlySecNew()
  @param    pIn       ptr to data to feed into stream
  @param    len       length of data to feed into stream
  @returns  TRUE if this data was added to the stream, FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlySecStreamFeed(hSec_t hSec, const void *pIn, unsigned len)
{
  flySec_t     *pSec    = hSec;
  bool_t        fAdded  = FALSE;

  // don't add if too long to fit in stream
  if(FlySecIsSec(hSec) && len <= FlySecStreamLeft(hSec))
  {
    if(len > 0)
    {
      memcpy(&pSec->pStream[pSec->streamLen], pIn, len);
      pSec->streamLen += len;
    }

    // may have fuzz after adding data to stream
    SecFuzzRemove(pSec);
    fAdded = TRUE;
  }

  return fAdded;
}

/*!------------------------------------------------------------------------------------------------
  This decodes and decryptes a packet in the stream. Copies the user data pBuf.

  FLY_SEC_NO_DATA   if no or incomplete packet
  FLY_SEC_ERR       if bad header (rejected by higher layer)

  @param    hSec            to object returned by FlySecNew()
  @param    pBuf            ptr to buffer to receive application data
  @param    pfnProcessHdr   NULL or ptr to function to check header
  @param    pHdrAuxData     data to pass to ProcessHdr (in addition pHdr/hdrLen)
  @returns  1-n or FLY_SEC_NO_DATA
*///-----------------------------------------------------------------------------------------------
unsigned FlySecDecode(hSec_t hSec, void *pOut, pfnProcessHdr_t pfnProcessHdr, void *pHdrAuxData)
{
  flySec_t     *pSec    = hSec;
  uint8_t       aBlock[AES_BLOCKLEN];
  unsigned      len       = FLY_SEC_NO_DATA;
  unsigned      totalLen  = 0;
  unsigned      hdrLen    = 0;
  unsigned      i;
  bool_t        fHdrValid;
  secErr_t      err;

  if(!FlySecIsSec(hSec))
    return FLY_SEC_ERR;

  // check if a packet is ready in the stream
  err = SecDecodePreamble(pSec, &hdrLen, &totalLen);
  // printf("FlySecDecode, err %u, streamLen %u\n", err, pSec->streamLen);
  // FlyStrDump(pSec->pStream, pSec->streamLen);
  if(err == SEC_ERR_FUZZ)
  {
    SecFuzzRemove(hSec);
    // printf("fuzz removed, streamLen %u\n", pSec->streamLen);
    // FlyStrDump(pSec->pStream, pSec->streamLen);
    err = SecDecodePreamble(pSec, &hdrLen, &totalLen);
  }

  // see if preamble looks OK, including CRC. If not, this will remove any fuzz from stream
  if(err == SEC_ERR_NONE)
  {
    // data must be multiple of block size
    // if higher layer wants to, it can valid the header (and perhaps use it to set security materal)
    len = totalLen - (FLY_SEC_PREAMBLE_SIZE + hdrLen);
    if(len % AES_BLOCKLEN)
      fHdrValid = FALSE;
    else if(pfnProcessHdr == NULL)
      fHdrValid = TRUE;
    else
      fHdrValid = (*pfnProcessHdr)(hSec, &pSec->pStream[FLY_SEC_PREAMBLE_SIZE], hdrLen, pHdrAuxData);

    if(!fHdrValid)
      len = FLY_SEC_NO_DATA;
    else
    {
      FlySecNonceReset(hSec);

      // decrypt the data and store in pBuf
      for(i = 0; i < len; i+= sizeof(aBlock))
      {
        memcpy(aBlock, &pSec->pStream[FLY_SEC_PREAMBLE_SIZE + hdrLen + i], AES_BLOCKLEN);
        AES_CTR_xcrypt_buffer(&pSec->ctx, aBlock, AES_BLOCKLEN);
        memcpy(&pOut[i], aBlock, AES_BLOCKLEN);
      }
      len = FlySecPadRemove(pOut, len);
    }

    // remove packet from stream (which may be all of stream)
    if(totalLen < pSec->streamLen)
      memmove(&pSec->pStream[0], &pSec->pStream[totalLen], pSec->streamLen - totalLen);
    if(totalLen > pSec->streamLen)
      pSec->streamLen = 0;
    else
      pSec->streamLen -= totalLen;
  }

  return len;   // 1-n or FLY_SEC_NO_DATA
}
