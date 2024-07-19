/**************************************************************************************************
  test_utf8.c
  Copyright 2023 Drew Gislason
  License: MIT <https://mit-license.org>
**************************************************************************************************/
#include "FlyTest.h"
#include "FlyUtf8.h"

typedef struct
{
  uint32_t    codePoint;
  utf8_t      szUtf8Char[UTF8_MAX];
  unsigned    len;
} tcCodePointPair_t;

/*-------------------------------------------------------------------------------------------------
  Test limits of a single character for get/put
-------------------------------------------------------------------------------------------------*/
void TcUtf8Limits(void)
{

  /* limits:
    00000000 -- 0000007F:   0xxxxxxx (7 bits, ASCII)
    00000080 -- 000007FF:   110xxxxx 10xxxxxx (11 bits UTF-8)
    00000800 -- 0000FFFF:   1110xxxx 10xxxxxx 10xxxxxx (16 bits)
    00010000 -- 0010FFFF:   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx (21 bits)
  */
  const tcCodePointPair_t aCodePointPairs[] =
  {
    { 0,        "\x00",             1 },
    { 0x7f,     "\x7f",             1 },
    { 0x80,     "\xc2\x80",         2 },
    { 0x7ff,    "\xdf\xbf",         2 },
    { 0x800,    "\xe0\xa0\x80",     3 },  // 1110 0000 1010 0000 1000 0000    // 1000 0000 0000
    { 0xffff,   "\xef\xbf\xbf",     3 },
    { 0x10000,  "\xf0\x90\x80\x80", 4 },
    { 0x10ffff, "\xf4\x8f\xbf\xbf", 4 },
    { 0x110000, "",                 0 },
  };
  utf8_t    szUtf8Char[UTF8_MAX];
  uint32_t  codePoint;
  unsigned  i;
  unsigned  len;

  FlyTestBegin();

  for(i = 0; i < NumElements(aCodePointPairs); ++i)
  {
    len = FlyUtf8Len(aCodePointPairs[i].codePoint);
    if(len != aCodePointPairs[i].len)
    {
      FlyTestPrintf("%u: 0x%lx, Bad length FlyUtf8Len(), got %u, expected %u\n", i,
        aCodePointPairs[i].codePoint, len, aCodePointPairs[i].len);
      FlyTestFailed();
    }

    len = FlyUtf8CharPut(szUtf8Char, aCodePointPairs[i].codePoint);
    if(len != aCodePointPairs[i].len)
    {
      FlyTestPrintf("%u: 0x%x, Bad length FlyUtf8CharPut(), got %u, expected %u\n", i,
        aCodePointPairs[i].codePoint, len, aCodePointPairs[i].len);
      FlyTestFailed();
    }
    if(memcmp(aCodePointPairs[i].szUtf8Char, szUtf8Char, len + 1) != 0)
    {
      FlyTestPrintf("%u: 0x%x, Bad content\n", i, aCodePointPairs[i].codePoint);
      FlyTestFailed();
    }

    // don't try to get invalid codepoints
    if(aCodePointPairs[i].len)
    {
      // convert valid codepoint from UTF-8 to a codePoint
      codePoint = FlyUtf8CharGet(szUtf8Char);
      if(codePoint != aCodePointPairs[i].codePoint)
      {
        FlyTestPrintf("%u: Cannot convert back to codepoint with FlyUtf8CharGet(), got %x, expected %x\n",
          i, codePoint, aCodePointPairs[i].codePoint);
        FlyTestFailed();
      }
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test various FlyUtf8Charxxx() functions
-------------------------------------------------------------------------------------------------*/
void TcUtf8Char(void)
{
  const utf8_t    szUtf8String[] = "[\u00a3\u221e\U0001000f]";
  uint32_t        aUnicode[]     = { '[', 0xa3, 0x221e, 0x1000f, ']' }; // must match szUtf8String
  uint32_t        u32;
  const utf8_t   *pszUtf8;
  const utf8_t   *pszUtf8Next;
  unsigned        i;
  unsigned        len;
  utf8_t          szUtf8Char[UTF8_MAX + 1];

  FlyTestBegin();

  memset(szUtf8Char, 0x7f, sizeof(szUtf8Char));

  pszUtf8 = szUtf8String;
  for(i = 0; i < NumElements(aUnicode) && *pszUtf8; ++i)
  {
    // verify get works with this UTF-8 character
    u32 = FlyUtf8CharGet(pszUtf8);
    if(u32 != aUnicode[i])
    {
      FlyTestPrintf("%u: got %u, expected %u\n", i, u32, aUnicode[i]);
      FlyTestFailed();
    }

    // verify pointer advances to next character
    pszUtf8Next = FlyUtf8CharCpy(szUtf8Char, pszUtf8);
    if(pszUtf8Next <= pszUtf8)
      FlyTestFailed();

    // verify it got the proper character
    u32 = FlyUtf8CharGet(szUtf8Char);
    if(u32 != aUnicode[i])
    {
      FlyTestPrintf("got %u, expected %u\n", u32, aUnicode[i]);
      FlyTestFailed();
    }
    if(szUtf8Char[sizeof(szUtf8Char) - 1] != 0x7f)
      FlyTestFailed();

    pszUtf8 = pszUtf8Next;
  }
  if(*pszUtf8 != '\0')
    FlyTestFailed();


  pszUtf8 = szUtf8String;
  for(i = 0; i < NumElements(aUnicode); ++i)
  {
    // verity put/get works
    len = FlyUtf8CharPut(szUtf8Char, aUnicode[i]);
    if(FlyUtf8CharGet(szUtf8Char) != aUnicode[i])
      FlyTestFailed();

    // verify length and content of UTF-8
    if(len != FlyUtf8CharLen(szUtf8Char))
      FlyTestFailed();
    if(len != FlyUtf8CharLen(pszUtf8))
      FlyTestFailed();
    if(strncmp(szUtf8Char, pszUtf8, len) != 0)
      FlyTestFailed();
    if(i < 4 && len != i + 1)
      FlyTestFailed();

    // verify indexing works
    if(pszUtf8 != FlyUtf8CharIdx(szUtf8String, i))
      FlyTestFailed();

    pszUtf8 = FlyUtf8CharNext(pszUtf8);
  }

  FlyTestEnd();
}


/*-------------------------------------------------------------------------------------------------
  Test string functions
-------------------------------------------------------------------------------------------------*/
void TcUtf8String(void)
{
  typedef struct
  {
    unsigned  size;
    unsigned  expChars;
    unsigned  expLen;
  } tcUtf8String_t;
  const utf8_t    szUtf8String[] = "[\u00a3\u221e\U0001000f]";
  utf8_t          szUtf8Dst[sizeof(szUtf8String)];
  const tcUtf8String_t  aTestCases[] = {
    { 12, 5, 11 }, { 11, 4, 10 }, { 10, 3, 6 },  { 7, 3, 6 }, { 4, 2, 3 }, { 2, 1, 1}
  };
  unsigned        len;
  unsigned        nChars;
  unsigned        i;

  FlyTestBegin();

  // test desination too small to receive even 1 character
  szUtf8Dst[0] = 'A';
  if(FlyUtf8StrZCpy(szUtf8Dst, szUtf8String, 0) != 0 || szUtf8Dst[0] != 'A')
    FlyTestFailed();
  if(FlyUtf8StrZCpy(szUtf8Dst, szUtf8String, 1) != 0 || szUtf8Dst[0] == 'A')
    FlyTestFailed();

  // test getting whole characters
  for(i = 0; i < NumElements(aTestCases); ++i)
  {
    memset(szUtf8Dst, 0, sizeof(szUtf8Dst));
    nChars = FlyUtf8StrZCpy(szUtf8Dst, szUtf8String, aTestCases[i].size);
    if(nChars != aTestCases[i].expChars)
    {
      FlyTestPrintf("%u: Bad nChars, got %u, expected %u\n", i, nChars, aTestCases[i].expChars);
      FlyTestFailed();
    }
    nChars = FlyUtf8StrLen(szUtf8Dst);
    if(nChars != aTestCases[i].expChars)
    {
      FlyTestPrintf("%u: Bad FlyUtf8StrLen, got %u, expected %u\n", i, nChars, aTestCases[i].expChars);
      FlyTestFailed();
    }
    len = strlen(szUtf8Dst);
    if(len != aTestCases[i].expLen)
    {
      FlyTestPrintf("%u: Bad length, got %u, expected %u\n", i, len, aTestCases[i].expLen);
      FlyTestFailed();
    }
    if(memcmp(szUtf8Dst, szUtf8String, len) != 0)
    {
      FlyTestPrintf("%u: strings didn't compare!\n", i);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test Escape sequences
-------------------------------------------------------------------------------------------------*/
void TcUtf8Esc(void)
{
  typedef struct
  {
    const utf8_t *szSrc;
    unsigned      srcLen;     // expected length of src escape sequence
    const utf8_t *szDst;      // expected resulting bytes
    unsigned      dstLen;     // expected resulting length

  } tcUtf8Esc_t;
  const tcUtf8Esc_t aTestCases[] =
  {
    { "a ", 1, "a", 1},
    { "\\t ", 2, "\x09", 1},
    { "\\076 ", 4, "\x3e", 1},
    { "\\x1f ", 4, "\x1f", 1},
    { "\\ub6 ", 4, "\xc2\xb6", 2},
    { "\\u25d0 ", 6, "\xe2\x97\x90", 3},
    { "\\U0001000f ", 10, "\xf0\x90\x80\x8f", 4},
  };
  const utf8_t *psz;
  utf8_t        szDst[UTF8_MAX];
  unsigned      dstLen;
  unsigned      i;

  FlyTestBegin();

  for(i = 0; i < NumElements(aTestCases); ++i)
  {
    psz = FlyUtf8CharEsc(szDst, aTestCases[i].szSrc, &dstLen);
    if(dstLen != aTestCases[i].dstLen)
    {
      FlyTestPrintf("%u: Bad dstLen, got %u, expected %u\n", i, dstLen, aTestCases[i].dstLen);
      FlyTestFailed();
    }
    if(!psz || (unsigned)(psz - aTestCases[i].szSrc) != aTestCases[i].srcLen)
    {
      FlyTestPrintf("%u: Bad srcLen, got %u, expected %u\n", i,
        (unsigned)(psz - aTestCases[i].szSrc), aTestCases[i].srcLen);
      FlyTestFailed();
    }
    if(memcmp(szDst, aTestCases[i].szDst, dstLen) != 0)
    {
      FlyTestPrintf("%u: invalid dst data\ngot: ", i);
      FlyTestDump(szDst, dstLen);
      FlyTestPrintf("exp: ");
      FlyTestDump(aTestCases[i].szDst, dstLen);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test bad inputs
-------------------------------------------------------------------------------------------------*/
void TcUtf8Fuzz(void)
{
  utf8_t  szUtf8Char[UTF8_MAX];
  const utf8_t *aszUtf8TestCases[] =
  {
    "\xe0\x80",
    "\x80\x80",
    "\xff\x80",
    "\xc0\xc0",
    "\xF0\x9F\x8C\xCE"
  };
  const utf8_t     *pszUtf8;
  unsigned          i;
  unsigned          len;

  FlyTestBegin();

  for(i = 0; i < NumElements(aszUtf8TestCases); ++i)
  {
    if(FlyUtf8CharGet(aszUtf8TestCases[i]) != UTF8_INVALID)
    {
      FlyTestPrintf("%u: expected UTF8_INVALID\n", i);
      FlyTestFailed();
    }
    len = FlyUtf8CharLen(aszUtf8TestCases[i]);
    if(len != 1)
    {
      FlyTestPrintf("%u: bad len, got %u, expected 1\n", i, len);
      FlyTestFailed();
    }
    pszUtf8 = FlyUtf8CharCpy(szUtf8Char, aszUtf8TestCases[i]);
    if(pszUtf8 != &aszUtf8TestCases[i][1])
    {
      FlyTestPrintf("%u: copied more than 1 invalid byte got %p, exp %p\n", i, pszUtf8, &aszUtf8TestCases[i][1]);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyUtf8SlugCpy() function
-------------------------------------------------------------------------------------------------*/
void TcUtf8SlugCpy(void)
{
  typedef struct
  {
    const char  *sz;
    const char  *szSlug;
    unsigned     size;
  } TcStrSlug_t;
  TcStrSlug_t aTests[] =
  {
    { "  I Love   Waffles  ", "I-Love-Waffles" },     // do not move this one entry, see code
    { "my.echo My Shadow & Me", "my.echo-My-Shadow-Me" },
    { "-._~", "-._~" },
    { "a - b . c _ d ~ e", "a-b.c_d~e" },
    { u8"🔥   🐈  😊 木 ", u8"🔥-🐈-😊-木" },
    { u8"🔥🐈", u8"🔥", 8 },
    { "", "" },
    { "       ", "" },
    { "az09AZ-._~", "az09AZ-._~" },
    { "abc", "abc" },
    { "a.2c", "a.2c" },
    { "Who Knows?", "Who-Knows" },
    { " ME,   my.self &\r\n I    ", "ME-my.self" },
    { " 4.8 - @logo link", "4.8-logo-link" },
  };
  char        szSlug[64];   // manually sized, see aTests[].szSlug above
  unsigned    len;
  unsigned    lenExp;
  unsigned    i;
  unsigned    size;

  FlyTestBegin();

  for(i = 0; i < NumElements(aTests); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("\n%i sz '%s', szSlug '%s', size %u\n", i, aTests[i].sz, aTests[i].szSlug, aTests[i].size);

    // test getting slug length only
    size = aTests[i].size;
    if(size == 0)
      size = sizeof(szSlug);
    len = FlyUtf8SlugCpy(NULL, aTests[i].sz, size, 0);
    lenExp = strlen(aTests[i].szSlug);
    if(len != lenExp)
    {
      FlyTestPrintf("%u: %s, got len %u, expected %u\n", i, aTests[i].sz, len, lenExp);
      FlyTestFailed();
    }
  
    FlyStrZFill(szSlug, 'A', sizeof(szSlug), sizeof(szSlug));
    len = FlyUtf8SlugCpy(szSlug, aTests[i].sz, size, 0);
    if((len != lenExp) || (strcmp(szSlug, aTests[i].szSlug) != 0))
    {
      FlyTestPrintf("%u: got %u,'%s', expected %u,'%s'\n", i, len, szSlug, lenExp, aTests[i].szSlug);
      FlyTestFailed();
    }
    if(szSlug[lenExp + 1] != 'A')
      FlyTestFailed();
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test each function
-------------------------------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
  const char          szName[] = "test_utf8";
  const sTestCase_t   aTestCases[] =
  {
    { "TcUtf8Limits",   TcUtf8Limits },
    { "TcUtf8Char",     TcUtf8Char },
    { "TcUtf8String",   TcUtf8String },
    { "TcUtf8Esc",      TcUtf8Esc },
    { "TcUtf8Fuzz",     TcUtf8Fuzz },
    { "TcUtf8SlugCpy",  TcUtf8SlugCpy },
  };
  hTestSuite_t        hSuite;
  int                 ret;

  // set up signal handling and logging
  // NedSigSetExit(argv[0], NULL);
  // FlyTestMaskSet(TESTLOG_BASIC);

  FlyTestInit(szName, argc, argv);
  hSuite = FlyTestNew(szName, NumElements(aTestCases), aTestCases);
  FlyTestRun(hSuite);
  ret = FlyTestSummary(hSuite);
  FlyTestFree(hSuite);

  return ret;
}
