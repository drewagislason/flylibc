/**************************************************************************************************
  FlyUTF8.c - UTF-8 string handling
  Copyright 2024 Drew Gislason  
  License: MIT <https://mit-license.org>  
*///***********************************************************************************************
#include "FlyUtf8.h"

/*!
  @defgroup FlyUtf8   A set of UTF-8 string utilities

  FlyUtf8 is a C API for handling UTF-8 strings in a minimal way.

  UTF-8 strings are strings of bytes that are '\0' terminated like most C strings. You can use
  standard strcpy() or strncpy() to copy unicode strings. However, since each character in UTF-8
  is variable length, finding the nth character or copying n characters is more tricky.
`
  UTF-8 encoding is 1 to 4 bytes: See also: <https://en.wikipedia.org/wiki/UTF-8>

      00000000 -- 0000007F:   0xxxxxxx (7 bits)
      00000080 -- 000007FF:   110xxxxx 10xxxxxx (11 bits)
      00000800 -- 0000FFFF:   1110xxxx 10xxxxxx 10xxxxxx (16 bits)
      00010000 -- 0010FFFF:   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx (21 bits)

  Every Unicode code point can be represented in UTF-8. See [Wikipedia: List of Unicode Characters](https://en.wikipedia.org/wiki/List_of_Unicode_characters).
  
  @example Print each character with index and value for Ascii and UTF-8 strings

  ```
  #include "FlyUtf8.h"

  int main(void)
  { 
    const char     *szAsciiHello = "Hello World\n";
    const utf8_t   *szUtf8Hello  = u8"Héllø Wôrld\n";
    const utf8_t   *pszUtf8;
    const utf8_t    szUtf8Char[UTF8_MAX];
    unsigned        nChars;
    unsigned        i;
    uint32_t        c32;

    // print each ASCII char with index
    nChars = strlen(szAsciiHello);
    for(i = 0; i < nChars; ++i)
      printf("char %u: %c, value 0x%x\n", i, szAscii[i], (unsigned)szAscii[i]);

    // print each UTF-8 char with index
    nChars = FlyUtf8StrLen(szUtf8Hello);
    pszUtf8 = szUtf8Hello;
    for(i = 0; i < nChars; ++i)
    {
      c32 = FlyUtf8CharGet(pszUtf8);
      pszUtf8 = FlyUtf8CharCpy(szUtf8Char, pszUtf8);
      FlyUtf8CharGet(szUtf8Char, pszUtf8
      printf("char %u: %s, value 0x%" PRIu32 "x\n", i, c32);
    }

    return 0;
  }
  ```
*/

/*!------------------------------------------------------------------------------------------------
  Copy a single (perhaps multi-byte) UTF-8 character from one string to another.

  szDst must either be NULL (no copy happens) or hold up to 5 bytes (4 characters plus a '\0').

  @param    szDst   ptr to buffer UTF8_MAX in size or NULL
  @param    szSrc   ptr to a UTF-8 string
  @return   next UTF-8 character, ptr to szSrc + len of UTF-8 char (1-4 bytes)
*///-----------------------------------------------------------------------------------------------
const utf8_t * FlyUtf8CharCpy(utf8_t *szDst, const utf8_t *szSrc)
{
  unsigned len = FlyUtf8CharLen(szSrc);
  if(szDst)
  {
    memcpy(szDst, szSrc, len);
    szDst[len] = '\0';
  }
  return szSrc + len;
}

/*!------------------------------------------------------------------------------------------------
  Copies a C-like escape sequence from szSrc to a single UTF-8 char szDst.

  Examples of escape sequences:

  Escape Sequence      | UTF-8 Bytes | dstLen | description
  -------------------- | ----------- | ------ | -----------
  "\\n"                | 0a          | 1      | line feed
  "\\177" (octal)      | 7f          | 1      | `~` tilde
  "\\xfe" (hex)        | fe          | 1      | NOT UTF-8
  "\\ua9"   (unicode)  | c2 a9       | 2      | copyright
  "\\u2190" (unicode)  | e2 86 90    | 3      | left arrow
  "\\U1f600" (unicode) | f0 9f 98 80 | 4      | smiley emoji

  szDst must either be NULL (no copy happens) or hold up to 5 bytes (4 characters plus a '\0').

  @param    szDst   ptr to buffer UTF8_MAX in size or NULL
  @param    szSrc   ptr to escape character sequence
  @param    pDstLen return value, destination length in bytes
  @return   ptr to next character after escape sequence in szSrc
*///-----------------------------------------------------------------------------------------------
const utf8_t * FlyUtf8CharEsc(utf8_t *szDst, const utf8_t *szSrc, unsigned *pDstLen)
{
  unsigned  dstLen;
  uint8_t   byte;
  long      codePoint;

  // not an escape char, simply copy the single UTF-8 char
  if(*szSrc != '\\')
  {
    dstLen = FlyUtf8CharLen(szSrc);
    szSrc = FlyUtf8CharCpy(szDst, szSrc);
  }

  // is an escape char
  else
  {
    if((szSrc[1] == 'u' || szSrc[1] == 'U') && isxdigit(szSrc[2]))
    {
      codePoint = FlyStrNToL(&szSrc[2], NULL, 16, *szSrc == 'u' ? 4 : 8, &dstLen, NULL);
      szSrc += 2 + dstLen;
      if(codePoint <= 0 || codePoint > UTF8_CODEPOINT_MAX)
        codePoint = UTF8_REPLACEMENT;
      dstLen = FlyUtf8CharPut(szDst, codePoint);
    }
    else
    {
      szSrc = FlyCharEsc(szSrc, &byte);
      if(szDst)
      {
        *(uint8_t *)szDst = byte;
        szDst[1] = '\0';
      }
      dstLen = 1;
    }
  }

  if(pDstLen)
    *pDstLen = dstLen;

  return szSrc;
}

/*-------------------------------------------------------------------------------------------------
  Helper function, gets # of 1 bits from highest to lowest bit in in 1st byte of UTF-8 string.

  Examples:

      byte in binary  | return | mask in binary  
      --------------- | ------ | --------------
      11000000 (0xc0) | 2      | 00011111
      11110111 (0xf3) | 4      | 00000111

  @param    byte    the bit to check for high bits
  @param    pMask   return value of bit mask for data
  @return   number of top bits
-------------------------------------------------------------------------------------------------*/
static unsigned Utf8TopBits(uint8_t byte, uint8_t *pMask)
{
  unsigned  nBits   = 0;
  unsigned  maskBits;
  uint8_t   mask    = 0;

  // determine # of 1 bits at top of byte
  while(byte & 0x80)
  {
    ++nBits;
    byte = byte << 1;
  }

  if(nBits < 7)
  {
    maskBits = (8 - nBits) - 1;
    while(maskBits)
    {
      mask = (mask << 1) | 1;
      --maskBits;
    }
  }
  *pMask = mask;

  return nBits;
}

/*!------------------------------------------------------------------------------------------------
  Gets a 1-4 byte UTF-8 character from the string and returns it as a 32-bit Unicode code point.

  UTF-8 Encoding is:

      00000000 -- 0000007F:   0xxxxxxx (7 bits, ASCII)
      00000080 -- 000007FF:   110xxxxx 10xxxxxx (11 bits)
      00000800 -- 0000FFFF:   1110xxxx 10xxxxxx 10xxxxxx (16 bits)
      00010000 -- 0010FFFF:   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx (21 bits)

  Anything above 0x10FFFF is invalid. If first byte is binary 10xxxxxx it is invalid. If subsequent
  bytes are not 10xxxxxx it is invalid.

  @param    szDst   ptr to destination string for UTF-8  character
  @param    c32     Unicode character (U + n)
  @return   UTF8_INVALID or Unicode code point (e.g. 0x2020 for dagger)
*///-----------------------------------------------------------------------------------------------
uint32_t FlyUtf8CharGet(const utf8_t *szUtf8)
{
  const uint8_t  *pByte       = (const uint8_t *)szUtf8;
  uint32_t        codePoint;
  unsigned        nBytes;
  uint8_t         mask;

  // ascii byte
  if((*pByte & 0x80) == 0)
  {
    codePoint = *pByte;
  }

  // UTF-8 (possibly invalid)
  else
  {
    // top 2-bits must be set, but not all of them
    if((*pByte & 0xc0) != 0xc0 || *pByte == 0xff)
      codePoint = UTF8_INVALID;
    else
    {
      nBytes = Utf8TopBits(*pByte, &mask);
      if(nBytes > 4)
        codePoint = UTF8_INVALID;
      else
      {
        // 1st byte is special, all others are 10xxxxxx
        codePoint = *pByte & mask;
        ++pByte;
        --nBytes;
        while(nBytes)
        {
          if((*pByte & 0xc0) != 0x80)
          {
            codePoint = UTF8_INVALID;
            break;
          }
          codePoint = (codePoint << 6) + (*pByte & 0x3f);
          ++pByte;
          --nBytes;
        }
      }
    }
  }

  return codePoint;
}

/*!------------------------------------------------------------------------------------------------
  Get ptr to character based on the index i in an array of characters.

  Does not stop on '\0' so it could be used both on normal C strings and arbitrary UTF-8 arrays.

  @param    szUtf8    ptr to a string at least UTF8_MAX in size
  @param    i         index 
  @return   ptr to szSrc + len of UTF-8 char (1-4 bytes)
*///-----------------------------------------------------------------------------------------------
const utf8_t * FlyUtf8CharIdx(const utf8_t *szUtf8, size_t i)
{
  while(i && *szUtf8)
  {
    szUtf8 += FlyUtf8CharLen(szUtf8);
    --i;
  }
  return szUtf8;
}

/*!------------------------------------------------------------------------------------------------
  Determine the length of this potentially UTF-8 character sequence. Always 1-4 bytes.

  If the UTF-8 string is invalid, length is always 1.

  @param    szUtf8   ptr to a UTF-8 string or character
  @return   length of UTF-8 character (1-4 bytes)
*///-----------------------------------------------------------------------------------------------
unsigned FlyUtf8CharLen(const utf8_t *szUtf8)
{
  const uint8_t  *pByte       = (const uint8_t *)szUtf8;
  unsigned        len         = 1;
  unsigned        nBytes;
  uint8_t         mask;

  // might be multi-byte
  if((*pByte & 0xc0) == 0xc0 && *pByte != 0xff)
  {
    // cannot be more than 4 bytes in UTF-8 string (that is, up to three 10xxxxxx bytes)
    nBytes = Utf8TopBits(*pByte, &mask) - 1;
    if(nBytes > 3)
      len = 1;
    else
    {
      ++pByte;
      while(nBytes)
      {
        if((*pByte & 0xc0) != 0x80)
        {
          len = 1;
          break;
        }
        ++len;
        --nBytes;
        ++pByte;
      }
    }
  }

  return len;
}

/*!------------------------------------------------------------------------------------------------
  Return next character (advance 1-4 bytes) in the UTF-8 string

  @param    szUtf8   ptr to a UTF-8 character
  @return   size of UTF-8 character (1-4 bytes)
*///-----------------------------------------------------------------------------------------------
const utf8_t * FlyUtf8CharNext(const utf8_t *szUtf8)
{
  return szUtf8 + FlyUtf8CharLen(szUtf8);
}

/*!------------------------------------------------------------------------------------------------
  Converts 32-bit Unicode code point to a 0-4 byte UTF-8 sequence. Always '\0' terminated.

  If the codePoint is > UTF8_CODEPOINT_MAX (invalid), an empty string is returned, length 0.

  Example: copyright U+169 or 0xA9 is "\xC2\xA9" or binary 110 00010  10 101001

  Encoding is:

      00000000 -- 0000007F:   0xxxxxxx (7 bits)
      00000080 -- 000007FF:   110xxxxx 10xxxxxx (11 bits)
      00000800 -- 0000FFFF:   1110xxxx 10xxxxxx 10xxxxxx (16 bits)
      00010000 -- 0010FFFF:   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx (21 bits)

  @param    szDst       ptr to destination string for wide character
  @param    codePoint   Unicode code point (e.g. U+20AC)
  @return   length of encoded unicode character (1-4 bytes).
*///-----------------------------------------------------------------------------------------------
unsigned FlyUtf8CharPut(utf8_t *szDst, uint32_t codePoint)
{
  unsigned  len = 0;

  // 21 bits max. make sure no higher bits are set in debug library
  // 1111 1111 1110 0000 0000 0000 0000 0000
  FlyAssertDbg(!(codePoint & (uint32_t)0xffe00000L));

  if(codePoint <= 0x7f)
  {
    len = 1;
    if(szDst)
      szDst[0] = (uint8_t)codePoint;
  }
  else if(codePoint <= 0x7ff)
  {
    len = 2;
    if(szDst)
    {
      szDst[0] = (uint8_t)(0xc0 | ((codePoint >> 6) & 0x1f));
      szDst[1] = (uint8_t)(0x80 | (codePoint & 0x3f));
    }
  }
  else if(codePoint <= 0xffff)
  {
    len = 3;
    if(szDst)
    {
      szDst[0] = (uint8_t)(0xe0 | ((codePoint >> 12) & 0x0f));
      szDst[1] = (uint8_t)(0x80 | ((codePoint >> 6)  & 0x3f));
      szDst[2] = (uint8_t)(0x80 | (codePoint & 0x3f));
    }
  }
  else if(codePoint <= UTF8_CODEPOINT_MAX)
  {
    len = 4;
    if(szDst)
    {
      szDst[0] = (uint8_t)(0xf0 | ((codePoint >> 18) & 0x07));
      szDst[1] = (uint8_t)(0x80 | ((codePoint >> 12) & 0x3f));
      szDst[2] = (uint8_t)(0x80 | ((codePoint >> 6)  & 0x3f));
      szDst[3] = (uint8_t)(0x80 | (codePoint & 0x3f));
    }
  }
  szDst[len] = '\0';

  return len;
}

/*!------------------------------------------------------------------------------------------------
  Return length of this codepoint (Unicode character) in bytes if converted to UTF-8

  @param    codePoint   U + # (e.g. 0x2020 is dagger)
  @return   length of codepoint (1-4 bytes), or 0 > UTF8_CODEPOINT_MAX (invalid)
*///-----------------------------------------------------------------------------------------------
unsigned FlyUtf8Len(uint32_t codePoint)
{
  unsigned len = 0;
  if(codePoint <= 0x7f)
    len = 1;
  else if(codePoint <= 0x7ff)
    len = 2;
  else if(codePoint <= 0xffff)
    len = 3;
  else if(codePoint <= UTF8_CODEPOINT_MAX)
    len = 4;
  return len;
}

/*!------------------------------------------------------------------------------------------------
  Return number of characters (not bytes) in string. Use standard strlen() to get # of bytes.

  UTF-8 characters are 1-4 bytes in length each.

  @param    szUtf8   ptr to a UTF-8 string which may contain UTF-8 characters
  @return   number characters (both ASCII and UTF-8 characters) in string
*///-----------------------------------------------------------------------------------------------
size_t FlyUtf8StrLen(const utf8_t *szUtf8)
{
  size_t  nChars = 0;
  while(*szUtf8)
  {
    szUtf8 = FlyUtf8CharNext(szUtf8);
    ++nChars;
  }
  return nChars;
}

/*!------------------------------------------------------------------------------------------------
  Copy a UTF-8 string from one memory location to another non-overlapping memory location.

  Similar to strncpy(), but always '\0' terminates the szDst string, unless size is 0, which does
  nothing. Also returns number of UTF-8 characters copied, rather than pointer to szDst.

  Input size is in bytes, not variable length UTF-8 characters.

  Only whole UTF-8 characters are copied. For example, if size is 3, the 4-byte earth character
  (F0 9F 8C 8E) would NOT be copied, but copyright (C2 A9) would be.

  @param    szDst   ptr to a UTF-8 destination at least size
  @param    szSrc   ptr to a UTF-8 character
  @param    size    size of szDst string in bytes (not UTF-8 chars)
  @return   number of variable length UTF-8 characters copied
*///-----------------------------------------------------------------------------------------------
size_t FlyUtf8StrZCpy(utf8_t *szDst, const utf8_t *szSrc, size_t size)
{
  size_t      nChars = 0;
  unsigned    len;

  while(*szSrc && size > 1)
  {
    len = FlyUtf8CharLen(szSrc);
    if(len >= size)
      break;

    // copy the character
    memcpy(szDst, szSrc, len);
    szDst += len;
    szSrc += len;
    size  -= len;
    ++nChars;
  }

  if(size)
    *szDst = '\0';

  return nChars;
}

/*!-------------------------------------------------------------------------------------------------
  Similar to strcpy(), but converts UTF-8 string to a case sensitive slug (URI friendly string).

  1. Only alnum, characters "-._~" and UTF-8 (0x80+) are copied. All else is considered whitespace.
  2. Removes whitespace and non-slug-friendly characters from start and end
  3. Middle Whitespace and non-slug-friendly characters are converted to a single dash
  4. End of line is treated as end of string (e.g. `\r` or `\n`)

  Examples:

  UTF-8 String             | Becomes Slug
  ------------------------ | ------------
  "  I Love   Waffles  "   | "I-Love-Waffles"
  "my.echo My Shadow & Me" | "my.echo-My-Shadow-Me"
  "-._~"                   | "-._~"
  "a - b . c _ d ~ e"      | "a-b.c_d~e"
  "🔥 🐈 😊 木"           | "🔥-🐈-😊-木"

  A hex dump of that final UTF-8 string with fire, cat, smile emoji and Japanese tree is below:

      00000000  f0 9f 94 a5 20 f0 9f 90  88 20 f0 9f 98 8a 20 e6  |.... .... .... .|
      00000010  9c a8                                             |..|
      00000012

  Set szSlug buffer to NULL to just get length of slug (for allocating memory for the slug).

  Resulting string will only contain Unreserved Characters from Section 2.3 of RFC 3986.

  It's a VERY bad idea to have URI's longer than 2000 characters. This function limits the slug
  length to UINT_MAX - 1.

  See: <https://www.rfc-editor.org/rfc/rfc3986>  
  See also: <https://webmasters.stackexchange.com/questions/90339/why-are-urls-case-sensitive>  

  @param    szSlug  destination buffer for slug, may be NULL to just get length
  @param    szSrc   source string to convert to a slug string
  @param    size    1-n, size of destination buffer szSlug
  @param    srcLen  maximum length in bytes of szSrc to process, can be 0 to just use strlen()
  @return   length of slug in bytes
*///------------------------------------------------------------------------------------------------
unsigned FlyUtf8SlugCpy(utf8_t *szSlug, const char *szSrc, size_t size, size_t srcLen)
{
  unsigned    len        = 0;
  unsigned    thisLen;
  bool_t      fExit      = FALSE;
  char        whitespace = 0;

  if(srcLen == 0)
    srcLen = FlyStrLineLen(szSrc);

  // skip preceding non-slug characters (whitespace). UTF-8 (0x80+) is not whitespace
  while(srcLen && *szSrc && !FlyCharIsEol(*szSrc) && !FlyCharIsSlug(*szSrc))
  {
    --srcLen;
    ++szSrc;
  }

  // limit to size of unsigned with room for '\0'
  if(size >= UINT_MAX)
    size = UINT_MAX - 1;

  // copy slug string from ASCIIZ string
  while(srcLen && *szSrc && !FlyCharIsEol(*szSrc) && size > 1)
  {
    // copy all slug characters
    while(srcLen && *szSrc && FlyCharIsSlug(*szSrc))
    {
      // if not enough room to copy full char, we're done
      thisLen = FlyUtf8CharLen(szSrc);
      if(thisLen >= size)
      {
        szSrc = FlyStrLineEnd(szSrc);
        fExit = TRUE;
        break;
      }
      szSrc = FlyUtf8CharCpy(szSlug, szSrc);
      if(szSlug)
        szSlug += thisLen;
      len += thisLen;
      size -= thisLen;
    }
    if(fExit)
      break;

    // skip trailing non-slug characters (whitespace)
    // special case " - " is considered whitespace, allows for sections like:
    // 4.8 - Some Section in Document
    whitespace = 0;
    if(srcLen >= 3 && isblank(*szSrc) && strchr(g_szFlyStrSlugChars, szSrc[1]) && isblank(szSrc[2]))
    {
      whitespace = szSrc[1];
      szSrc += 3;
      srcLen -= 3;
    }
    while(srcLen && *szSrc && !FlyCharIsEol(*szSrc) && !FlyCharIsSlug(*szSrc))
    {
      whitespace = '-';
      ++szSrc;
      --srcLen;
    }

    // put a single dash to replace any number of in-between whitespace and non-slug chars
    if(whitespace && *szSrc && !FlyCharIsEol(*szSrc))
    {
      if(szSlug)
        *szSlug++ = whitespace;
      ++len;
      --size;
      if(size <= 1)
        break;
    }
  }

  if(szSlug)
    *szSlug = '\0';

  return len;
}
