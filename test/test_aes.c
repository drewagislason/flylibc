/**************************************************************************************************
  @file test_aes.c

  @brief  Test Cases for the socket subsystem

  Copyright 2022 Drew Gislason

  @ingroup    fly_test
*///***********************************************************************************************
#include <unistd.h>
#include "Fly.h"
#include "FlyLog.h"
#include "FlyTest.h"
#include "FlyFile.h"
#include "FlyStr.h"
#include "FlyAes.h"

/*!------------------------------------------------------------------------------------------------
  Test TcSockUdpIPv4()

  @ingroup    ned_test
  @returns    none
*///-----------------------------------------------------------------------------------------------
void TcAesCtr(void)
{
  uint8_t         key[32];
  uint8_t         txt[16];
  uint8_t         txt2[16];   // original text
  uint8_t         txt3[16];   // encrypted with nonce 1
  uint8_t         nonce[16];
  struct AES_ctx  ctx;

  FlyTestBegin();

  strcpy((char *)txt, "Every Good Boy!");
  memcpy(txt2, txt, sizeof(txt2));
  memcpy(key, "One Key To Rule Them All And In The Darkness Bind Them", sizeof(key));
  memset(nonce, 0, sizeof(nonce));
  AES_init_ctx(&ctx, key);

  if(FlyTestVerbose())
  {
    FlyTestPrintf("\n");
    FlyTestDump(txt, 16);
  }

  // encrypted must be different from original text
  nonce[15] = 1;
  AES_ctx_set_iv(&ctx, nonce);
  AES_CTR_xcrypt_buffer(&ctx, txt, sizeof(txt));
  if(FlyTestVerbose())
    FlyTestDump(txt, 16);
  if(memcmp(txt, txt2, sizeof(txt2)) == 0)
    FlyTestFailed();

  // keep a copy of encrypted block with nonce 1
  memcpy(txt3, txt, sizeof(txt2));

  // decrypt should be same as original text
  AES_ctx_set_iv(&ctx, nonce);
  AES_CTR_xcrypt_buffer(&ctx, txt, sizeof(txt));
  if(FlyTestVerbose())
    FlyTestDump(txt, 16);
  if(memcmp(txt, txt2, sizeof(txt2)) != 0)
    FlyTestFailed();

  // encrypted must be different from original text and encrypted block with nonce 1
  nonce[15] = 2;
  AES_ctx_set_iv(&ctx, nonce);
  AES_CTR_xcrypt_buffer(&ctx, txt, sizeof(txt));
  if(FlyTestVerbose())
    FlyTestDump(txt, 16);
  if(memcmp(txt, txt2, sizeof(txt2)) == 0)
    FlyTestFailed();
  if(memcmp(txt, txt3, sizeof(txt3)) == 0)
    FlyTestFailed();

  // must be back to original text
  AES_ctx_set_iv(&ctx, nonce);
  AES_CTR_xcrypt_buffer(&ctx, txt, sizeof(txt));
  if(FlyTestVerbose())
    FlyTestDump(txt, 16);
  if(memcmp(txt, txt2, sizeof(txt2)) != 0)
    FlyTestFailed();

  FlyTestPassed();

  FlyTestEnd();  
}

void TcAesMulti(void)
{
  FlyTestBegin();
  FlyTestSkipped();
  FlyTestEnd();  
}

/*-------------------------------------------------------------------------------------------------
  Test each function

  test_socket [-a] [-f filter] [-m] [-v[#]

-------------------------------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
  const char          szName[] = "test_aes";
  const sTestCase_t   aTestCases[] =
  { 
    { "TcAesCtr",  TcAesCtr  },
    { "TcAesMulti",  TcAesMulti  }
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
