/**************************************************************************************************
  exmp_utf8.c - Example using FlyUtf8 API
  Copyright 2023 Drew Gislason
  License: MIT <https://mit-license.org>
**************************************************************************************************/
#include "FlyUtf8.h"

int main(void)
{ 
  const char     *szAsciiHello      = "Hello World";
  const utf8_t   *szUtf8Hello       = u8"Héllø Wôrld";
  const utf8_t   *szUtf8UpsideDown  = "\xca\x8e\xc7\x9d\xca\x9e";   // upside down "key"
  const utf8_t   *szUtf8Chars       = u8"~\u00a3\u221e\U0001000f";  // 1, 2, 3 and 4 byte unicode chars
  const utf8_t   *aUtf8Strs[] = { szUtf8Hello, szUtf8UpsideDown, szUtf8Chars };
  const utf8_t   *pszUtf8;
  utf8_t          szUtf8Char[UTF8_MAX];
  unsigned        nChars;
  unsigned        i, j;
  uint32_t        codePoint;     // unicode codepoint

  printf("example_utf8 - demonstrates FlyUtf8 API\n");

  // print each ASCII char with index
  nChars = strlen(szAsciiHello);
  printf("\n%u chars (%u bytes) ASCII string `%s`\n", nChars, (unsigned)strlen(szAsciiHello), szAsciiHello);
  for(i = 0; i < nChars; ++i)
    printf("%2u: char %c, codepoint 0x%x\n", i, szAsciiHello[i], (unsigned)szAsciiHello[i]);

  // print each UTF-8 char with index
  for(j = 0; j < NumElements(aUtf8Strs); ++j)
  {
    printf("\n");
    pszUtf8 = aUtf8Strs[j];
    nChars = FlyUtf8StrLen(pszUtf8);
    printf("\n%u chars (%u bytes) UTF-8 string `%s`\n", nChars, (unsigned)strlen(pszUtf8), pszUtf8);
    for(i = 0; i < nChars; ++i)
    {
      codePoint = FlyUtf8CharGet(pszUtf8);
      pszUtf8 = FlyUtf8CharCpy(szUtf8Char, pszUtf8);
      printf("%2u: char %s, codepoint 0x%x (%u bytes)\n", i, szUtf8Char, codePoint, (unsigned)strlen(szUtf8Char));
    }
  }

  return 0;
}
