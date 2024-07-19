/*!************************************************************************************************
  @file FLyTestKey.c

  @brief  Unit test for key functions
  @copyright 2022 Drew Gislason
  @ingroup test
*///***********************************************************************************************
#include "FlyTest.h"
#include "FlyKeyPrompt.h"

/*!------------------------------------------------------------------------------------------------
  Test getting keys and presenting valuye and name
*///-----------------------------------------------------------------------------------------------
void FlyTestKey(void)
{
  flyKey_t  key;

  FlyTestBegin();

  FlyTestPrintf("Press any key. Press ENTER to quit (pass). Press ESC to fail.\n");
  while(1)
  {
    key = FlyKeyGetKey();
    FlyTestPrintf("key %u = %s\n", key, FlyKeyName(key));
    if(key == FLY_KEY_ENTER)
      break;
    if(key == FLY_KEY_ESC)
      FlyTestFailed();
  }

  FlyTestPassed();
  FlyTestEnd();
}

bool_t KeyIdle(void *pData)
{
  static  unsigned   col    = 0;
  size_t            *pCount = pData;

  if(pCount)
    *pCount += 1;

  FlyTestPrintf(".");
  fflush(stdout);

  ++col;
  if(col >= 80)
  {
    FlyTestPrintf("\n");
    col = 0;
  }

  return FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Test casse: verify idle function is called
*///-----------------------------------------------------------------------------------------------
void FlyTestKeyIdle(void)
{
  flyKey_t  key;
  size_t    idleCount;

  FlyTestBegin();

  FlyTestPrintf("Idle should print '.'. Press ENTER to quit (pass). Press ESC to fail.\n");
  idleCount = 0;
  FlyKeySetIdle(KeyIdle, &idleCount);

  while(1)
  {
    key = FlyKeyGetKey();
    FlyTestPrintf("key %u = %s\n", key, FlyKeyName(key));
    if(key == FLY_KEY_ENTER)
      break;
    if(key == FLY_KEY_ESC)
      FlyTestFailed();
  }

  FlyTestPrintf("idleCount %zu\n", idleCount);
  if(idleCount == 0)
    FlyTestFailed();

  FlyTestPassed();
  FlyTestEnd();

  FlyKeySetIdle(NULL, NULL);
}

/*!------------------------------------------------------------------------------------------------
  Test prompt automated
*///-----------------------------------------------------------------------------------------------
void FlyTestKeyPrompt(void)
{
  hFlyKeyPrompt_t   hPrompt = NULL;
  char             *psz;
  int               len;
  #define SZ_SIZE   8

  FlyTestBegin();

  hPrompt = FlyKeyPromptNew(SZ_SIZE, NULL);

  if(!FlyKeyPromptIsPrompt(hPrompt))
  {
    FlyTestFailed();
  }
  if(FlyKeyPromptSize(hPrompt) != SZ_SIZE)
  {
    FlyTestFailed();
  }

  psz = FlyKeyPromptGets(hPrompt);
  if(psz == NULL || strlen(psz) != 0)
  {
    FlyTestFailed();
  }

  // verify we can feed a key
  if(FlyKeyPromptFeed(hPrompt, 'a') != -1 || *psz != 'a' || strlen(psz) != 1)
  {
    printf("psz %s, len %zu\n", psz, strlen(psz));
    FlyTestFailed();
  }

  // feed two more
  FlyKeyPromptFeed(hPrompt, 'b');
  len = FlyKeyPromptFeed(hPrompt, 'c');
  if(len != -1 || strcmp(psz, "abc") != 0)
  {
    FlyTestFailed();
  }

  // test feed of enter
  len = FlyKeyPromptFeed(hPrompt, FLY_KEY_ENTER);
  if(len != 3 || strcmp(FlyKeyPromptGets(hPrompt), "abc") != 0)
  {
    FlyTestFailed();
  }

  // test clear
  FlyKeyPromptClear(hPrompt);
  if(strlen(FlyKeyPromptGets(hPrompt)) != 0)
  {
    FlyTestFailed();
  }

  FlyTestEnd();

  // cleanup
  if(hPrompt)
    FlyKeyPromptFree(hPrompt);
}

/*!------------------------------------------------------------------------------------------------
  Test prompt manual
*///-----------------------------------------------------------------------------------------------
void FlyTestKeyPrompt2(void)
{
  hFlyKeyPrompt_t   hPrompt = NULL;
  char             *psz;
  flyKey_t          c;
  int               ret;
  #define SZ_SIZE   8

  FlyTestBegin();


  hPrompt = FlyKeyPromptNew(SZ_SIZE, "dude!");
  if(!FlyKeyPromptIsPrompt(hPrompt))
  {
    FlyTestFailed();
  }

  psz = FlyKeyPromptGets(hPrompt);
  if(psz == NULL || strcmp(psz, "dude!") != 0)
  {
    FlyTestFailed();
  }

  FlyTestPrintf("\nPress Esc\n\n> ");
  FlyKeyPromptRedraw(hPrompt);
  while(1)
  {
    c = FlyKeyGetKey();
    ret = FlyKeyPromptFeed(hPrompt, c);
    if(ret == -2)
      break;
    else
    {
      FlyTestPrintf("\n");
      FlyTestFailed();
    }
  }

  FlyTestPrintf("\nTry Ctrl-A, Ctrl-B, Ctrl-F, Ctrl-K, Ctrl-E. Press Enter to 'pass', Esc to 'fail'\n\n> ");
  FlyKeyPromptRedraw(hPrompt);
  while(1)
  {
    c = FlyKeyGetKey();
    ret = FlyKeyPromptFeed(hPrompt, c);
    if(ret == -2)
    {
      FlyTestPrintf("\n");
      FlyTestFailed();
    }
    else if(ret >= 0)
    {
      FlyTestPrintf(", len %d\n", ret);
      FlyTestPassed();
    }
  }

  FlyTestEnd();

  // cleanup
  if(hPrompt)
    FlyKeyPromptFree(hPrompt);
}

int main(int argc, const char *argv[])
{
  const char          szName[] = "test_key";
  const sTestCase_t   aTestCases[] =
  { 
    { "FlyTestKey",         FlyTestKey,       "M"   },
    { "FlyTestKeyIdle",     FlyTestKeyIdle,   "M"   },
    { "FlyTestKeyPrompt",   FlyTestKeyPrompt },
    { "FlyTestKeyPrompt2",  FlyTestKeyPrompt2, "M"  },
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
