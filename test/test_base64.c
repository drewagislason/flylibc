/**************************************************************************************************
  test_base64.c
  Copyright 2024 Drew Gislason
  License: MIT <https://mit-license.org>
**************************************************************************************************/
#include "FlyTest.h"
#include "FlyMem.h"
#include "FlyStr.h"
#include "FlyBase64.h"

typedef struct
{
  const uint8_t  *pBinary;
  size_t          binLen;
  const char     *szBase64;
  size_t          size;
} tcBase64_t;

/*-------------------------------------------------------------------------------------------------
  Test FlyBase64Encode() and FlyBase64Decode()
-------------------------------------------------------------------------------------------------*/
void TcBase64Simple(void)
{
  tcBase64_t aTests[] =
  {
    { (uint8_t *)"Man", 3, "TWFu", 5 },
    { (uint8_t *)"Ma", 2, "TWE=", 5 },
    { (uint8_t *)"M", 1, "TQ==", 5 },
    { (uint8_t *)"Many hands make light work.", 27, "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu", 37 },
    { (uint8_t *)"\x00\x01Hello World\x00\xfe\xff", 16, "AAFIZWxsbyBXb3JsZAD+/w==", 25 },
  };
  char       *szBase64 = NULL;
  size_t      size;
  size_t      binLen;
  unsigned    i;

  FlyTestBegin();

  for(i = 0; i < NumElements(aTests); ++i)
  {
    // allocate szBase64 string buffer
    size = FlyBase64Encode(NULL, SIZE_MAX, aTests[i].pBinary, aTests[i].binLen);
    if(size != aTests[i].size)
    {
      FlyTestPrintf("%u: bad size, got %zu, expected %zu\n", i, size, aTests[i].size);
      FlyTestFailed();
    }
    szBase64 = FlyAlloc(size + 2);
    if(!szBase64)
    {
      FlyTestFailed();
    }
    memset(szBase64, '~', size + 1);
    szBase64[size + 1] = '\0';

    // encode
    size = FlyBase64Encode(szBase64, size, aTests[i].pBinary, aTests[i].binLen);
    if(size != aTests[i].size || strcmp(szBase64, aTests[i].szBase64) != 0)
    {
      FlyTestPrintf("%u: bad encode size or data, got %zu, expected %zu\n", i, size, aTests[i].size);
      FlyTestPrintf("got: %s\nexp: %s\n", szBase64, aTests[i].szBase64);
      FlyTestFailed();
    }

    // verify decode length
    binLen = FlyBase64Decode(NULL, aTests[i].szBase64, aTests[i].binLen);
    if(binLen != aTests[i].binLen || binLen > size)
    {
      FlyTestPrintf("%u: bad binLen, got %zu, expected %zu\n", i, binLen, aTests[i].binLen);
      FlyTestFailed();
    }

    // verify decode
    memset(szBase64, '~', size + 1);
    szBase64[size + 1] = '\0';
    binLen = FlyBase64Decode((uint8_t *)szBase64, aTests[i].szBase64, size);
    if(binLen != aTests[i].binLen || memcmp(szBase64, aTests[i].pBinary, binLen) != 0)
    {
      FlyTestPrintf("%u: bad deocde len or data, got %zu, expected %zu\n", i, binLen, aTests[i].binLen);
      FlyTestPrintf("got:\n");
      FlyStrDump(szBase64, binLen);
      FlyTestPrintf("exp:\n");
      FlyStrDump(aTests[i].pBinary, aTests[i].binLen);
      FlyTestFailed();
    }

    // free the memory
    FlyFree(szBase64);
    szBase64 = NULL;
  }

  FlyTestEnd();

  if(szBase64)
    FlyFree(szBase64);
}

/*-------------------------------------------------------------------------------------------------
  Test 
-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------
  Test each function
-------------------------------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
  const char          szName[] = "test_base64";
  const sTestCase_t   aTestCases[] =
  {
    { "TcBase64Simple",      TcBase64Simple },
  };
  hTestSuite_t        hSuite;
  int                 ret;

  FlyTestInit(szName, argc, argv);
  hSuite = FlyTestNew(szName, NumElements(aTestCases), aTestCases);
  FlyTestRun(hSuite);
  ret = FlyTestSummary(hSuite);
  FlyTestFree(hSuite);

  return ret;
}
