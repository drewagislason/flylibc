/**************************************************************************************************
  FlyStrZ.c - Ensure strings always and in zero and don't overflow
  Copyright 2024 Drew Gislason
  License: MIT <https://mit-license.org>
*///***********************************************************************************************
#include "FlyStr.h"

/*!-------------------------------------------------------------------------------------------------
  Copy the szSrc string to szDst. Will not exceed size of szDst. Always '\0' terminates.

  @ingroup FlyStr
  @param  szDst     destination asciiz string, or NULL to just get size
  @param  szSrc     source string
  @param  size      sizeof szDst (1-n)
  @return # of bytes copied (or would have been copied if szDst == NULL)
*///------------------------------------------------------------------------------------------------
size_t FlyStrZCpy(char *szDst, const char *szSrc, size_t size)
{
  return FlyStrZNCpy(szDst, szSrc, size, strlen(szSrc));
}

/*!-------------------------------------------------------------------------------------------------
  Copy the szSrc string to szDst. Will not exceed size of szDst. Always '\0' terminates.

  @param  szDst     destination asciiz string, or NULL to just get size
  @param  szSrc     source string
  @param  size      sizeof szDst (1-n)
  @param  srcLen    length of src string
  @return # of bytes copied (or would have been copied if szDst == NULL)
*///------------------------------------------------------------------------------------------------
size_t FlyStrZNCpy(char *szDst, const char *szSrc, size_t size, size_t srcLen)
{
  // nothing to do
  if(size == 0)
    srcLen = 0;

  else
  {
    if(srcLen > (size - 1))
      srcLen = (size - 1);
    if(srcLen > strlen(szSrc))
      srcLen = strlen(szSrc);

    if(szDst)
    {
      if(srcLen)
        strncpy(szDst, szSrc, srcLen);
      szDst[srcLen] = '\0';
    }
  }

  // printf("FlyStrZNCpy => srcLen %zu\n", srcLen);
  return srcLen;
}

/*!-------------------------------------------------------------------------------------------------
  Append the szSrc string to szDst. Will not exceed size of szDst. Always '\0' terminates.

  @param  szDst     destination asciiz string, or NULL to just get size
  @param  szSrc     source string
  @param  size      size of destination string (1-n)
  @return # of bytes copied (or would have been copied if szDst == NULL)
*///------------------------------------------------------------------------------------------------
size_t FlyStrZCat(char *szDst, const char *szSrc, size_t size)
{
  return FlyStrZNCat(szDst, szSrc, size, strlen(szSrc));
}

/*!-------------------------------------------------------------------------------------------------
  Append the szSrc string to szDst. Will not exceed size of szDst. Always '\0' terminates.

  @param  szDst     destination asciiz string, or NULL to just get size
  @param  szSrc     source string
  @param  size      sizeof szDst (1-n)
  @param  srcLen    length of src string
  @return # of bytes copied (or would have been copied if szDst == NULL)
*///------------------------------------------------------------------------------------------------
size_t FlyStrZNCat(char *szDst, const char *szSrc, size_t size, size_t srcLen)
{
  size_t dstLen;
  size_t n;

  // buffer to small
  if(size == 0)
    srcLen = 0;

  else
  {
    // from here on out, size becomes maxLen
    --size;

    // determine size to copy based on both size -1 and length of szSrc
    n = strlen(szSrc);
    if(srcLen > n)
      srcLen = n;
    if(srcLen > size)
      srcLen = size;

    if(szDst)
    {
      dstLen = strlen(szDst);
      if(dstLen > size)
        dstLen = size;
      if(srcLen > size - dstLen)
        srcLen = size - dstLen;
      if(srcLen)
        strncpy(&szDst[dstLen], szSrc, srcLen);
      szDst[dstLen + srcLen] = '\0';
    }
  }

  return srcLen;
}

/*!-------------------------------------------------------------------------------------------------
  Fill a string with a character, but still '\0' terminate the string. Will not exceed size.

  @param  szDst     destination asciiz string, or NULL to just get fill length
  @param  c         character to fill
  @param  size      sizeof szDst (1-n)
  @param  fillLen   length to fill with character
  @return # of characters filled 
*///------------------------------------------------------------------------------------------------
size_t FlyStrZFill(char *szDst, char c, size_t size, size_t fillLen)
{
  if(size < 1)
    fillLen = 0;

  else
  {
    if(fillLen > size - 1)
      fillLen = size - 1;
    if(szDst)
    {
      if(fillLen)
        memset(szDst, c, fillLen);
      szDst[fillLen] = '\0';
    }
  }

  return fillLen;
}

/*!-------------------------------------------------------------------------------------------------
  Fill end of string with character, but still '\0' terminate the string. Will not exceed size.

  @param  szDst     destination asciiz string
  @param  c         character to fill
  @param  size      sizeof szDst (1-n)
  @param  fillLen   length to fill with character
  @return # of characters filled
*///------------------------------------------------------------------------------------------------
size_t FlyStrZCatFill(char *szDst, char c, size_t size, size_t fillLen)
{
  size_t  len;

  if(size < 1)
    fillLen = 0;
  else
  {
    if(fillLen > size - 1)
      fillLen = size - 1;

    if(szDst)
    {
      // no room to fill anything
      len = strlen(szDst);
      if(len >= size - 1)
        fillLen = 0;

      // reduce fill len as there isn't room
      else if(fillLen >= size - len)
        fillLen = (size - len) - 1;

      if(fillLen)
      {
        memset(&szDst[len], c, fillLen);
        szDst[len + fillLen] = '\0';
      }
    }
  }

  return fillLen;
}

