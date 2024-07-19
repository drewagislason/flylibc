/**************************************************************************************************
  FlyTestStrSmart.c
  Copyright 2022 Drew Gislason
  License: MIT
**************************************************************************************************/
#include "FlyTest.h"
#include "FlyFile.h"
#include "FlyStr.h"

void TcStrSmartSimple(void)
{
  flyStrSmart_t  *pStr;
  flyStrSmart_t   smartStr;
  const char      szSmart[] = "Smart!";
  const char      szCopy[]  = "Using SmartStrCpy()";

  FlyTestBegin();

  // make sure creating a new string works
  pStr = FlyStrSmartNew("Hello");
  if(!pStr || !pStr->sz || pStr->size != 6)
    FlyTestFailed();
  FlyStrSmartCat(pStr, " World!");
  if(strcmp(pStr->sz, "Hello World!") != 0 || pStr->size != 13)
    FlyTestFailed();
  FlyStrSmartFree(pStr);

  // make sure allocating a string works
  pStr = FlyStrSmartAlloc(29);
  if(!pStr || !pStr->sz || pStr->size != 29)
    FlyTestFailed();
  if(*pStr->sz != '\0')
    FlyTestFailed();
  FlyStrSmartCpy(pStr, "Harley Davidson");
  if(strcmp(pStr->sz, "Harley Davidson") != 0 || pStr->size != 29)
    FlyTestFailed();
  FlyStrSmartFree(pStr);

  // make sure using a zero initialized smart string works
  memset(&smartStr, 0, sizeof(smartStr));
  FlyStrSmartCat(&smartStr, szSmart);
  if(strcmp(smartStr.sz, szSmart) != 0 || smartStr.size != sizeof(szSmart))
  {
    FlyTestPrintf("got `%s` size %u, exp `%s` len %u\n", FlyStrNullOk(smartStr.sz),
      (unsigned)smartStr.size, szSmart, (unsigned)sizeof(szSmart));
    FlyTestFailed();
  }
  if(smartStr.sz)
    free(smartStr.sz);

  // test zero initialized smart string with copy
  memset(&smartStr, 0, sizeof(smartStr));
  FlyStrSmartCpy(&smartStr, szCopy);
  if(strcmp(smartStr.sz, szCopy) != 0 || smartStr.size != sizeof(szCopy))
  {
    FlyTestPrintf("got `%s` size %u, exp `%s` size %u\n", FlyStrNullOk(smartStr.sz),
      (unsigned)smartStr.size, szSmart, (unsigned)sizeof(szCopy));
    FlyTestFailed();
  }
  if(smartStr.sz)
    free(smartStr.sz);

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Tests FlyStrSmartCat(), FlyStrSmartCpy(), FlyStrSmartNCat(), FlyStrSmartNCpy()
-------------------------------------------------------------------------------------------------*/
void TcStrSmartCat(void)
{
  const char    szEvery[]     = "Every";
  const char    szGood[]      = " Good";
  const char    szBoy[]       = " Boy";
  const char    szDoesFine[]  = " Does Fine";
  const char    szAll[]       = "Every Good Boy Does Fine";
  flyStrSmart_t *pStr         = NULL;
  const char    *psz;

  FlyTestBegin();

  // test copying where NO ALLOCATION needs to happen
  pStr = FlyStrSmartAlloc(256);
  if(!pStr || strlen(pStr->sz) || pStr->size != 256)
    FlyTestFailed();
  psz = FlyStrSmartCpy(pStr, szEvery);
  if(strcmp(psz, szEvery) != 0 || strcmp(pStr->sz, szEvery) != 0)
    FlyTestFailed();
  psz = FlyStrSmartCat(pStr, szGood);
  if(strcmp(psz, szGood) != 0 || strncmp(pStr->sz, szAll, 10) != 0)
    FlyTestFailed();
  psz = FlyStrSmartNCat(pStr, szBoy, strlen(szBoy));
  if(strcmp(psz, szBoy) != 0 || strncmp(pStr->sz, szAll, 14) != 0)
    FlyTestFailed();
  psz = FlyStrSmartCat(pStr, szDoesFine);
  if(strcmp(psz, szDoesFine) != 0 || strcmp(pStr->sz, szAll) != 0 || pStr->size != 256)
  {
    FlyTestPrintf("pStr->size %zu, psz \"%s\", pStr->sz \"%s\"\n", pStr->size, psz, pStr->sz);
    FlyTestFailed();
  }
  FlyStrSmartFree(pStr);

  // test copying where ALLOCATION needs to happen
  pStr = FlyStrSmartAlloc(0);
  if(!pStr || pStr->size == 0 || strlen(pStr->sz))
    FlyTestFailed();
  psz = FlyStrSmartCpy(pStr, szEvery);
  if(strcmp(psz, szEvery) != 0 || strcmp(pStr->sz, szEvery) != 0 || pStr->size != strlen(szEvery) + 1)
    FlyTestFailed();
  psz = FlyStrSmartCat(pStr, szGood);
  if(strcmp(psz, szGood) != 0 || strncmp(pStr->sz, szAll, 10) != 0)
    FlyTestFailed();
  psz = FlyStrSmartNCat(pStr, szBoy, strlen(szBoy));
  if(strcmp(psz, szBoy) != 0 || strncmp(pStr->sz, szAll, 14) != 0)
    FlyTestFailed();
  psz = FlyStrSmartCat(pStr, szDoesFine);
  if(strcmp(psz, szDoesFine) != 0 || strcmp(pStr->sz, szAll) != 0)
    FlyTestFailed();
  FlyStrSmartFree(pStr);

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Tests TcStrSmartNew()
-------------------------------------------------------------------------------------------------*/
void TcStrSmartNew(void)
{
  typedef struct
  {
    const char *sz;
    size_t      size;
  } TcStrSmartNew_t;

  TcStrSmartNew_t aTests[] =
  {
    { "Hello World", 12 },
    { "abc", 4 },
    { "A", 2 },
    { "", 1 },
  };
  const char      szSearch[] = "Good";
  flyStrSmart_t  *pStr       = NULL;
  unsigned        i;

  FlyTestBegin();

  pStr = FlyStrSmartNewEx(aTests[0].sz, 6);
  if(!pStr || strcmp(pStr->sz, "Hello") != 0 || pStr->size != 6)
    FlyTestFailed();
  FlyStrSmartFree(pStr);

  pStr = FlyStrSmartNewEx(strstr("Every Good Boy", szSearch), sizeof(szSearch));
  if(!pStr || strcmp(pStr->sz, szSearch) != 0 || pStr->size != strlen(szSearch) + 1)
    FlyTestFailed();
  FlyStrSmartFree(pStr);

  // allocate where the length comes from the string
  for(i = 0; i < NumElements(aTests); ++i)
  {
    pStr = FlyStrSmartNew(aTests[i].sz);
    if(!pStr || strcmp(pStr->sz, aTests[i].sz) != 0 || pStr->size != aTests[i].size)
      FlyTestFailed();
    FlyStrSmartFree(pStr);
  }

  FlyTestEnd();
}


/*-------------------------------------------------------------------------------------------------
  Tests FlyStrSmartSprintf()
-------------------------------------------------------------------------------------------------*/
void TcStrSmartSprintf(void)
{
  flyStrSmart_t  *pStr       = NULL;
  const char      szMessage[] = "Hello World 123 times, 0x600002f7003e";
  void           *pVoid       = (void *)0x600002f7003eUL;
  int             len;

  FlyTestBegin();

  pStr = FlyStrSmartAlloc(0);
  if(!pStr)
    FlyTestFailed();
  len = FlyStrSmartSprintf(pStr, "Hello %s %d times, %p", "World", 123, pVoid);
  if(len != strlen(szMessage) || strcmp(pStr->sz, szMessage) != 0)
  {
    FlyTestPrintf("got len %d, exp %d, pStr->sz %s\n", len, (int)strlen(szMessage), szMessage);
    FlyTestFailed();
  }

  FlyStrSmartFree(pStr);

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Tests FlyStrSmartSlice()
-------------------------------------------------------------------------------------------------*/
void TcStrSmartSlice(void)
{
  typedef struct
  {
    int         left;
    int         right;
    const char *sz;
  } TcStrSmartSlice_t;
  flyStrSmart_t    *pStr      = NULL;
  flyStrSmart_t    *pStr2     = NULL;
  const char        szTest[]  = "Hello World!";
  TcStrSmartSlice_t aSlices[] =
  {
    {  0,       2, "He"           },
    {  2,       5, "llo"          },
    { -6,      -1, "World"        },
    { -6, INT_MAX, "World!"       },
    {  0, INT_MAX, "Hello World!" },
    {  9,       2, ""             },
  };
  unsigned    i;

  FlyTestBegin();

  pStr = FlyStrSmartNew(szTest);
  if(!pStr)
    FlyTestFailed();

  for(i = 0; i < NumElements(aSlices); ++i)
  {
    pStr2 = FlyStrSmartSlice(pStr, aSlices[i].left, aSlices[i].right);
    if(!pStr2)
      FlyTestFailed();
    if(strcmp(pStr2->sz, aSlices[i].sz) != 0)
    {
      FlyTestPrintf("\n%u: got %s, expected %s\n", i, pStr2->sz, aSlices[i].sz);
      FlyTestFailed();
    }

    FlyStrSmartFree(pStr2);
    pStr2 = NULL;
  }

  if(pStr)
    FlyStrSmartFree(pStr);
  if(pStr2)
    FlyStrSmartFree(pStr2);

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test each function
-------------------------------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
  const char          szName[] = "test_smart";
  const sTestCase_t   aTestCases[] =
  {
    { "TcStrSmartSimple",     TcStrSmartSimple },
    { "TcStrSmartCat",        TcStrSmartCat },
    { "TcStrSmartNew",        TcStrSmartNew },
    { "TcStrSmartSprintf",    TcStrSmartSprintf },
    { "TcStrSmartSlice",      TcStrSmartSlice },
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
