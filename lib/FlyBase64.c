/**************************************************************************************************
  FlyBase64.c  Convert to/from base64
  Copyright 2024 Drew Gislason
  license: <https://mit-license.org>
**************************************************************************************************/
#include "FlyBase64.h"

/*!
  @defgroup FlyBase64  A C API For Converting To/From Base64

  Base64 is a way to use ASCII text to transmit/store binary data.

  See <https://en.wikipedia.org/wiki/Base64>
*/

static const char m_szEncoding[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char m_padding      = '=';

/*!-----------------------------------------------------------------------------------------------
  Encode the binary data into a base64 ASCIIZ string.

  Note: the pBase64 string will only be '\0' terminated if size is large enough. It's OK to have

  See: <https://en.wikipedia.org/wiki/Base64>

  @example FlyBase64Encode example

  ```c
  #include "FlyBase64.h"

  print_encoded(uint8_t *pBinary, size_t binLen)
  {
    char   *szBase64;
    size_t  size;

    size = Base64Encode(NULL, SIZE_MAX, pBinary, binLen);
    szBase64 = malloc(size);
    if(szBase64)
    {
      Base64Encode(szBase64, size, pBinary, binLen);
      printf("%s\n", szBase64);
    }
  }
  ```

  @param    pBase64     pointer to buffer to receive base64 string or NULL if just getting length
  @param    size        size of pBase64 buffer, or SIZE_MAX if pBase64 is NULL
  @param    pBinary     pointer to binary data
  @param    binLen      length of binary data
  @return   size of string buffer needed to encode binary data
*///-----------------------------------------------------------------------------------------------
size_t FlyBase64Encode(char *szBase64, size_t size, const uint8_t *pBinary, size_t binLen)
{
  size_t      total       = 0;
  uint32_t    bits;
  char        aEncoded[4];
  unsigned    thisLen;
  unsigned    i;

  while(binLen && (total + sizeof(aEncoded) < size))
  {
    // encode max 3 bytes at a time (up to 24 bits)
    if(binLen >= 3)
      thisLen = 3;
    else
      thisLen = (unsigned)binLen;

    // get bits to encode
    bits = 0;
    for(i = 0; i < thisLen; ++i)
      bits = bits | ((uint32_t)pBinary[i] << (16 - (i << 3)));

    // encode
    memset(aEncoded, m_padding, sizeof(aEncoded));
    for(i = 0; i < thisLen + 1; ++i)
      aEncoded[i] = m_szEncoding[(bits >> (18 - (6 * i))) & 0x3f];
    if(szBase64)
      memcpy(szBase64, aEncoded, sizeof(aEncoded));

    // on to next bits
    binLen -= thisLen;
    pBinary += thisLen;
    total  += sizeof(aEncoded);
    if(szBase64)
      szBase64 += sizeof(aEncoded);
  }

  if(szBase64)
    *szBase64 = '\0';
  ++total;

  return total; // total size for encoded base64 string
}

/*!------------------------------------------------------------------------------------------------
  Decode a base64 string into the binary buffer

  Note: the padding character `=` is optional as the length of the string determines how many bytes
  to decode. Length mod 4 is 0, 2, 3. 1 is invalid.

  See <https://en.wikipedia.org/wiki/Base64>

  @param    pBinary     pointer to buffer to receive binary data, or NULL if just getting length
  @param    pBase64     pointer to base64 string
  @param    size        size of binary buffer
  @return   length of binary buffer required to decode, or 0 if invalid or empty string
*///-----------------------------------------------------------------------------------------------
size_t FlyBase64Decode(uint8_t *pBinary, const char *szBase64, size_t size)
{
  const char *psz;
  size_t      len;
  size_t      binLen = 0;
  uint32_t    bits;
  unsigned    thisLen;
  unsigned    i;
  char        aEncoded[5];  // large enough for 4 encoded chars + '\0'

  // ignore any trailing padding '=' characters
  len = strlen(szBase64);
  psz = &szBase64[len - 1];
  while(*psz == m_padding)
  {
    --len;
    if(psz == szBase64)
      break;
    --psz;
  }

  // decode the string. If length mod 4 is 1, then string length is illegal
  while(len && ((len % 4) != 1))
  {
    // decode 4 encoded chars at a time
    thisLen = 4;
    if(thisLen > len)
      thisLen = (unsigned)len;

    // thisLen must be 2, 3 or 4, which means binLen is 1,2 or 3
    binLen += (thisLen - 1);

    // decode the bits into bytes (but don't exceed binary biffer size)
    if(pBinary && (binLen <= size))
    {
      // decode the (up to) 4 encoded chars into bits
      memcpy(aEncoded, szBase64, thisLen);
      aEncoded[thisLen] = '\0';
      bits = 0;
      for(i = 0; i < 4; ++i)
      {
        psz = strchr(m_szEncoding, aEncoded[i]);
        if(!psz)
        {
          binLen = len = 0;
          break;
        }
        bits |= (psz - m_szEncoding) << (18 - (i * 6));
      }

      if(thisLen == 4)
      {
        pBinary[0] = (uint8_t)(bits >> 16);
        pBinary[1] = (uint8_t)(bits >> 8);
        pBinary[2] = (uint8_t)bits;
      }
      else if(thisLen == 3)
      {
        pBinary[0] = (uint8_t)(bits >> 16);
        pBinary[1] = (uint8_t)(bits >> 8);
      }
      else
      {
        pBinary[0] = (uint8_t)(bits >> 16);
      }
      pBinary += (thisLen - 1);
    }

    // on to next encoding
    len -= thisLen;
    szBase64 += thisLen;
  }

  return binLen;
}
