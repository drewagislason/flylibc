/**************************************************************************************************
  FlyTestSort.c
  Copyright 2022 Drew Gislason
  License: MIT
  Brief: Test sorting
**************************************************************************************************/
#include "FlyTest.h"
#include "FlySort.h"

/*-------------------------------------------------------------------------------------------------
  Test FlySortBubble(), a bubble sort
-------------------------------------------------------------------------------------------------*/
void TcSortBubble(void)
{
  int         aInts[]         = { 99, 1, -3, 2, 55, -2, INT_MIN, INT_MAX };
  int         aIntsExp[]      = { INT_MIN, -3, -2, 1, 2, 55, 99, INT_MAX };
  unsigned    aUnsigned[]     = { UINT_MAX, 4, 3, 2, 1, 0 };
  unsigned    aUnsignedExp[]  = { 0, 1, 2, 3, 4, UINT_MAX };
#ifndef FLY_FLAG_NO_MATH
  double      aDoubles[]      = { 0.0, 3.141593, 99999.99, 3.14, -123456.789 };
  double      aDoublesExp[]   = { -123456.789, 0.0, 3.14, 3.141593, 99999.99 };
#endif
  char       *aStrs[]         = { "every", "good", "boy", "does", "fine!", "fine" };
  char       *aStrsExp[]      = { "boy", "does", "every", "fine", "fine!", "good" };

  int         aTestInts[NumElements(aIntsExp)];
  unsigned    aTestUnsigned[NumElements(aUnsignedExp)];
#ifndef FLY_FLAG_NO_MATH
  double      aTestDoubles[NumElements(aDoublesExp)];
#endif
  char       *aTestStrs[NumElements(aStrs)];
  unsigned    i;

  FlyTestBegin();

  // verify we can sort ints
  if(sizeof(aInts) != sizeof(aIntsExp) || sizeof(aInts) != sizeof(aTestInts))
    FlyTestFailed();
  memcpy(aTestInts, aInts, sizeof(aTestInts));
  FlySortBubble(aTestInts, NumElements(aTestInts), sizeof(aTestInts[0]), FlySortCmpInt);
  if(memcmp(aTestInts, aIntsExp, sizeof(aIntsExp)) != 0)
  {
    FlyTestDumpCmp(aTestInts, aIntsExp, sizeof(aIntsExp));
    FlyTestFailed();
  }

  // verify we can sort unsigned
  if(sizeof(aUnsigned) != sizeof(aUnsignedExp) || sizeof(aUnsigned) != sizeof(aTestUnsigned))
    FlyTestFailed();
  memcpy(aTestUnsigned, aUnsigned, sizeof(aTestUnsigned));
  FlySortBubble(aTestUnsigned, NumElements(aTestUnsigned), sizeof(aTestUnsigned[0]), FlySortCmpUnsigned);
  if(memcmp(aTestUnsigned, aUnsignedExp, sizeof(aUnsignedExp)) != 0)
  {
    FlyTestDumpCmp(aTestUnsigned, aUnsignedExp, sizeof(aUnsignedExp));
    FlyTestFailed();
  }

#ifndef FLY_FLAG_NO_MATH
  // verify we can sort double
  if(sizeof(aDoubles) != sizeof(aDoublesExp) || sizeof(aDoubles) != sizeof(aTestDoubles))
    FlyTestFailed();
  memcpy(aTestDoubles, aDoubles, sizeof(aTestDoubles));
  FlySortBubble(aTestDoubles, NumElements(aTestDoubles), sizeof(aTestDoubles[0]), FlySortCmpDouble);
  if(memcmp(aTestDoubles, aDoublesExp, sizeof(aDoublesExp)) != 0)
  {
    FlyTestDumpCmp(aTestDoubles, aDoublesExp, sizeof(aDoublesExp));
    for(i = 0; i < NumElements(aDoublesExp); ++i)
      FlyTestPrintf("aTestDoubles[%u] = %f, aDoublesExp[%u] = %f\n", i, aTestDoubles[i], i, aDoublesExp[i]);
    FlyTestFailed();
  }
#endif

  // verify we can sort strings
  if(sizeof(aStrs) != sizeof(aStrsExp) || sizeof(aStrs) != sizeof(aTestStrs))
    FlyTestFailed();
  memcpy(aTestStrs, aStrs, sizeof(aTestStrs));
  FlySortBubble(aTestStrs, NumElements(aTestStrs), sizeof(aTestStrs[0]), FlySortCmpStr);
  for(i = 0; i < NumElements(aStrsExp); ++i)
  {
    if(strcmp(aTestStrs[i], aStrsExp[i]) != 0)
    {
      FlyTestPrintf("Failed at %u\n", i);
      for(i = 0; i < NumElements(aStrsExp); ++i)
        FlyTestPrintf("aTestStrs[%u] = %s, aStrsExp[%u] = %s\n", i, aTestStrs[i], i, aStrsExp[i]);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

#define ARG_DOUBLE    2.22
#define ARG_INT       -53
#define ARG_UNSIGNED  42
#define ARG_STRING    "hello"

#ifndef FLY_FLAG_NO_MATH
/*!------------------------------------------------------------------------------------------------
  Compare two unsigned. Returns -1 if this < that, 0 if same, 1 if this > that.

  @param  pThis   ptr to an unsigned
  @param  pThat   ptr to an unsigned
  @return Returns -1 if this < that, 0 if same, 1 if this > that.
*///-----------------------------------------------------------------------------------------------
static int CmpDouble(void *pArg, const void *pThis, const void *pThat)
{
  if(*(double *)pArg != ARG_DOUBLE)
  {
    FlyTestPrintf("Bad Arg double\n");
    return -1;
  }
  if(*(double *)pThis == *(double *)pThat)
    return 0;
  return (*(double *)pThis < *(double *)pThat) ? -1 : 1;
}
#endif  // FLY_FLAG_NO_MATH

/*!------------------------------------------------------------------------------------------------
  Compare two ints. Returns -1 if this < that, 0 if same, 1 if this > that.

  @param  pThis   ptr to an int
  @param  pThat   ptr to an int
  @return Returns -1 if this < that, 0 if same, 1 if this > that.
*///-----------------------------------------------------------------------------------------------
static int CmpInt(void *pArg, const void *pThis, const void *pThat)
{
  if(*(int *)pArg != ARG_INT)
  {
    FlyTestPrintf("Bad Arg int\n");
    return -1;
  }
  if(*(int *)pThis == *(int *)pThat)
    return 0;
  return (*(int *)pThis < *(int *)pThat) ? -1 : 1;
}

/*!------------------------------------------------------------------------------------------------
  Compare two strings. Returns -1 if this < that, 0 if same, 1 if this > that.

  @param  pThis   ptr to an unsigned
  @param  pThat   ptr to an unsigned
  @return Returns -1 if this < that, 0 if same, 1 if this > that.
*///-----------------------------------------------------------------------------------------------
static int CmpStr(void *pArg, const void *pThis, const void *pThat)
{
  if(strcmp(pArg, ARG_STRING) != 0)
  {
    FlyTestPrintf("Bad Arg string\n");
    return -1;
  }
  const char * const *ppThis = pThis;
  const char * const *ppThat = pThat;
  return strcmp(*ppThis, *ppThat);
}

/*!------------------------------------------------------------------------------------------------
  Compare two unsigned. Returns -1 if this < that, 0 if same, 1 if this > that.

  @param  pThis   ptr to an int
  @param  pThat   ptr to an int
  @return Returns -1 if this < that, 0 if same, 1 if this > that.
*///-----------------------------------------------------------------------------------------------
static int CmpUnsigned(void *pArg, const void *pThis, const void *pThat)
{
  if(*(int *)pArg != ARG_UNSIGNED)
  {
    FlyTestPrintf("Bad Arg unsigned\n");
    return -1;
  }
  if(*(unsigned *)pThis == *(unsigned *)pThat)
    return 0;
  return (*(unsigned *)pThis < *(unsigned *)pThat) ? -1 : 1;
}

/*-------------------------------------------------------------------------------------------------
  Test FlySortQSort(), the built-in qsort_r with arg
-------------------------------------------------------------------------------------------------*/
void TcSortQSort(void)
{
  int         argInt          = ARG_INT;
  int         aInts[]         = { 99, 1, -3, 2, 55, -2, INT_MIN, INT_MAX };
  int         aIntsExp[]      = { INT_MIN, -3, -2, 1, 2, 55, 99, INT_MAX };
  unsigned    argUnsigned     = ARG_UNSIGNED;
  unsigned    aUnsigned[]     = { UINT_MAX, 4, 3, 2, 1, 0 };
  unsigned    aUnsignedExp[]  = { 0, 1, 2, 3, 4, UINT_MAX };
#ifndef FLY_FLAG_NO_MATH
  double      argDouble       = ARG_DOUBLE;
  double      aDoubles[]      = { 0.0, 3.141593, 99999.99, 3.14, -123456.789 };
  double      aDoublesExp[]   = { -123456.789, 0.0, 3.14, 3.141593, 99999.99 };
#endif
  char       *argStr          = ARG_STRING;
  char       *aStrs[]         = { "every", "good", "boy", "does", "fine!", "fine" };
  char       *aStrsExp[]      = { "boy", "does", "every", "fine", "fine!", "good" };

  int         aTestInts[NumElements(aIntsExp)];
  unsigned    aTestUnsigned[NumElements(aUnsignedExp)];
#ifndef FLY_FLAG_NO_MATH
  double      aTestDoubles[NumElements(aDoublesExp)];
#endif
  char       *aTestStrs[NumElements(aStrs)];
  unsigned    i;

  FlyTestBegin();

  // verify we can sort ints
  if(sizeof(aInts) != sizeof(aIntsExp) || sizeof(aInts) != sizeof(aTestInts))
    FlyTestFailed();
  memcpy(aTestInts, aInts, sizeof(aTestInts));
  FlySortQSort(aTestInts, NumElements(aTestInts), sizeof(aTestInts[0]), &argInt, CmpInt);
  if(memcmp(aTestInts, aIntsExp, sizeof(aIntsExp)) != 0)
  {
    FlyTestDumpCmp(aTestInts, aIntsExp, sizeof(aIntsExp));
    FlyTestFailed();
  }

  // verify we can sort unsigned
  if(sizeof(aUnsigned) != sizeof(aUnsignedExp) || sizeof(aUnsigned) != sizeof(aTestUnsigned))
    FlyTestFailed();
  memcpy(aTestUnsigned, aUnsigned, sizeof(aTestUnsigned));
  FlySortQSort(aTestUnsigned, NumElements(aTestUnsigned), sizeof(aTestUnsigned[0]), &argUnsigned, CmpUnsigned);
  if(memcmp(aTestUnsigned, aUnsignedExp, sizeof(aUnsignedExp)) != 0)
  {
    FlyTestDumpCmp(aTestUnsigned, aUnsignedExp, sizeof(aUnsignedExp));
    FlyTestFailed();
  }

#ifndef FLY_FLAG_NO_MATH
  // verify we can sort double
  if(sizeof(aDoubles) != sizeof(aDoublesExp) || sizeof(aDoubles) != sizeof(aTestDoubles))
    FlyTestFailed();
  memcpy(aTestDoubles, aDoubles, sizeof(aTestDoubles));
  FlySortQSort(aTestDoubles, NumElements(aTestDoubles), sizeof(aTestDoubles[0]), &argDouble, CmpDouble);
  if(memcmp(aTestDoubles, aDoublesExp, sizeof(aDoublesExp)) != 0)
  {
    FlyTestDumpCmp(aTestDoubles, aDoublesExp, sizeof(aDoublesExp));
    for(i = 0; i < NumElements(aDoublesExp); ++i)
      FlyTestPrintf("aTestDoubles[%u] = %f, aDoublesExp[%u] = %f\n", i, aTestDoubles[i], i, aDoublesExp[i]);
    FlyTestFailed();
  }
#endif

  // verify we can sort strings
  if(sizeof(aStrs) != sizeof(aStrsExp) || sizeof(aStrs) != sizeof(aTestStrs))
    FlyTestFailed();
  memcpy(aTestStrs, aStrs, sizeof(aTestStrs));
  FlySortQSort(aTestStrs, NumElements(aTestStrs), sizeof(aTestStrs[0]), argStr, CmpStr);
  for(i = 0; i < NumElements(aStrsExp); ++i)
  {
    if(strcmp(aTestStrs[i], aStrsExp[i]) != 0)
    {
      FlyTestPrintf("Failed at %u\n", i);
      for(i = 0; i < NumElements(aStrsExp); ++i)
        FlyTestPrintf("aTestStrs[%u] = %s, aStrsExp[%u] = %s\n", i, aTestStrs[i], i, aStrsExp[i]);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

typedef struct mySingleList
{
  struct mySingleList  *pNext;
  unsigned              data;
} mySingleList_t;

typedef struct myDoubleList
{
  struct myDoubleList  *pNext;
  struct myDoubleList  *pPrev;
  unsigned              data;
} myDoubleList_t;

/*-------------------------------------------------------------------------------------------------
  helper to TcSortList()
-------------------------------------------------------------------------------------------------*/
static int CmpListSingle(void *pArg, const void *pThis, const void *pThat)
{
  mySingleList_t *pItem1 = (mySingleList_t *)pThis;
  mySingleList_t *pItem2 = (mySingleList_t *)pThat;
  int   ret = 0;

  if(pArg != NULL)
    FlyTestPrintf("bad arg SortListCmpSingle()\n");
  else
  {
    if(pItem1->data == pItem2->data)
      ret = 0;
    else
      ret = (pItem1->data < pItem2->data) ? -1 : 1;
  }
  return ret;
}

#define CMP_DOUBLE_ID 42

/*-------------------------------------------------------------------------------------------------
  helper to TcSortList()
-------------------------------------------------------------------------------------------------*/
static int CmpListDouble(void *pArg, const void *pThis, const void *pThat)
{
  myDoubleList_t *pItem1 = (myDoubleList_t *)pThis;
  myDoubleList_t *pItem2 = (myDoubleList_t *)pThat;
  int             ret = 0;

  if(pArg == NULL || *(unsigned *)pArg != CMP_DOUBLE_ID)
    FlyTestPrintf("bad arg SortListCmpDouble()\n");

  if(pItem1->data == pItem2->data)
    ret = 0;
  else
    ret = (pItem1->data < pItem2->data) ? -1 : 1;
  return ret;
}

/*-------------------------------------------------------------------------------------------------
  Helper to TcSortList()
-------------------------------------------------------------------------------------------------*/
static void InitSingleList(mySingleList_t *pList, unsigned nElem, unsigned *pData)
{
  unsigned        i;

  for(i = 0; i < nElem; ++i)
  {
    pList[i].pNext = (i + 1 == nElem) ? NULL : &pList[i + 1];
    pList[i].data  = pData[i];
  }
}

/*-------------------------------------------------------------------------------------------------
  Helper to TcSortList()
-------------------------------------------------------------------------------------------------*/
static void InitDoubleList(myDoubleList_t *pList, unsigned nElem, unsigned *pData)
{
  unsigned        i;

  for(i = 0; i < nElem; ++i)
  {
    pList[i].pNext = (i + 1 == nElem) ? NULL : &pList[i + 1];
    pList[i].pPrev = (i == 0) ? NULL : &pList[i - 1];
    pList[i].data  = pData[i];
  }
}

/*-------------------------------------------------------------------------------------------------
  Helper to TcSortList()
-------------------------------------------------------------------------------------------------*/
static void PrintList(void *pList, bool_t fDouble)
{
  mySingleList_t *pThis1  = pList;
  myDoubleList_t *pThis2  = pList;
  unsigned        nItems = 0;

  FlyTestPrintf("\n");
  while(pThis1)
  {
    FlyTestPrintf("%p: pNext %p, ", pThis1, pThis1->pNext);
    if(fDouble)
      FlyTestPrintf("pPrev %p, ", pThis2->pPrev);
    FlyTestPrintf("data: %u\n", fDouble ? pThis2->data : pThis1->data);
    pThis1 = pThis1->pNext;
    pThis2 = pThis2->pNext;
    ++nItems;
  }
  FlyTestPrintf("%u items\n", nItems);
}

/*-------------------------------------------------------------------------------------------------
  Test FlySortList()
-------------------------------------------------------------------------------------------------*/
void TcSortList(void)
{
  unsigned        aData[]     = { 99, 1, 7, 5, 2 };
  unsigned        aDataExp[]  = { 1, 2, 5, 7, 99 };
  unsigned        id = CMP_DOUBLE_ID;
  mySingleList_t  aSingle[NumElements(aData)];
  mySingleList_t *pHead1;
  mySingleList_t *pThis1;
  myDoubleList_t  aDouble[NumElements(aData)];
  myDoubleList_t *pHead2;
  myDoubleList_t *pThis2;
  myDoubleList_t *pLast2;
  unsigned        i;

  FlyTestBegin();

  // test single linked list
  InitSingleList(aSingle, NumElements(aSingle), aData);
  if(FlyTestVerbose())
    PrintList(aSingle, FALSE);

  pHead1 = FlySortList(aSingle, FALSE, FALSE, NULL, CmpListSingle);
  i = 0;
  pThis1 = pHead1;
  while(pThis1)
  {
    if(pThis1->data != aDataExp[i])
    {
      PrintList(pHead1, FALSE);
      FlyTestPrintf("\nSingle: %u: got %u, exp %u\n", i, pThis1->data, aDataExp[i]);
      FlyTestFailed();
    }
    pThis1 = pThis1->pNext;
    ++i;
  }

  // test double linked list
  InitDoubleList(aDouble, NumElements(aDouble), aData);
  if(FlyTestVerbose())
    PrintList(aDouble, TRUE);
  pHead2 = FlySortList(aDouble, FALSE, TRUE, &id, CmpListDouble);
  i = 0;
  pThis2 = pHead2;
  while(pThis2)
  {
    if(pThis2->data != aDataExp[i])
    {
      PrintList(pHead2, TRUE);
      FlyTestPrintf("\nDouble: %u: got %u, exp %u\n", i, pThis2->data, aDataExp[i]);
      FlyTestFailed();
    }
    if(pThis2->pNext == NULL)
      pLast2 = pThis2;
    pThis2 = pThis2->pNext;
    ++i;
  }

  // verify back links are still sorted
  pThis2 = pLast2;
  while(pThis2)
  {
    if(i == 0)
      FlyTestFailed();
    --i;
    if(pThis2->data != aDataExp[i])
    {
      PrintList(pHead2, TRUE);
      FlyTestPrintf("\nDouble Back: %u: got %u, exp %u\n", i, pThis2->data, aDataExp[i]);
      FlyTestFailed();
    }
    pThis2 = pThis2->pPrev;
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test each function
-------------------------------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
  const char          szName[] = "test_sort";
  const sTestCase_t   aTestCases[] =
  {
    { "TcSortBubble",     TcSortBubble },
    { "TcSortQSort",      TcSortQSort },
    { "TcSortList",       TcSortList },
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
