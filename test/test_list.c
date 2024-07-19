/**************************************************************************************************
  test_list.c
  Copyright 2023 Drew Gislason
  License: MIT <https://mit-license.org>
**************************************************************************************************/
#include "FlyTest.h"
#include "FlyList.h"

typedef struct myList
{
 struct myList   *pNext;
 struct myList   *pPrev;  // 0x16db238a8
 uint64_t         id;
 char            *szName;
} myList_t;

typedef struct
{
  uint64_t         id;
  char            *szName;
} myData_t;

typedef struct
{
  bool_t      fCircular;
  bool_t      fDouble;
} TcAddSortedArg_t;

/*-------------------------------------------------------------------------------------------------
  Print the list using links
-------------------------------------------------------------------------------------------------*/
void TestListPrint(myList_t *pList)
{
  myList_t  *pThis  = pList;
  size_t     nItems = 0;

  while(pThis && nItems < 8)
  {
    printf("%p: pNext %11p", pThis, pThis->pNext);
    printf(", pPrev %11p", pThis->pPrev);
    printf(", %llu:%s\n", pThis->id, pThis->szName);
    ++nItems;

    if(pThis->pNext == pList)
      break;

    pThis = pThis->pNext;
  }

  printf("%zu items\n", nItems);
}

/*-------------------------------------------------------------------------------------------------
  Print the list as an array
-------------------------------------------------------------------------------------------------*/
void TestArrayPrint(myList_t *pList, unsigned nElem)
{
  unsigned  i;

  FlyTestPrintf("\n");
  for(i = 0; i < nElem; ++i)
  {
    FlyTestPrintf("%p: pNext %p, pPrev %p, id %llu: %s\n", &pList[i], pList[i].pNext,
      pList[i].pPrev, pList[i].id, pList[i].szName);
  }
}

/*-------------------------------------------------------------------------------------------------
  Initialize array from data. Starts with all 0s in pList
-------------------------------------------------------------------------------------------------*/
void TestListInitArray(unsigned nItems, myList_t *pList, myData_t *pData)
{
  unsigned    i;

  memset(pList, 0, nItems * sizeof(*pList));
  for(i = 0; i < nItems; ++i)
  {
    pList[i].id     = pData[i].id;
    pList[i].szName = pData[i].szName;
  }
}

/*-------------------------------------------------------------------------------------------------
  Verify links are what they should be.
-------------------------------------------------------------------------------------------------*/
bool_t TestListLinksOK(myList_t *pList, unsigned nItems, bool_t fCircular, bool_t fDouble)
{
  myList_t *pLast;

  if(nItems == 0 && pList != NULL)
  {
    FlyTestPrintf("nItems == 0 && pList != NULL\n");
    return FALSE;
  }

  if(nItems)
  {
    if(FlyListLen(pList) != nItems)
    {
      FlyTestPrintf("FlyListLen(pList) != nItems");
      return FALSE;
    }
    if(!fDouble && pList->pPrev != NULL)
    {
      FlyTestPrintf("!fDouble && pList->pPrev != NULL\n");
      return FALSE;
    }
    if(fDouble && fCircular && pList->pPrev == NULL)
    {
      FlyTestPrintf("fDouble && fCircular && pList->pPrev == NULL\n");
      return FALSE;
    }
    if(fDouble && !fCircular && pList->pPrev != NULL)
    {
      FlyTestPrintf("fDouble && !fCircular && pList->pPrev != NULL\n");
      return FALSE;
    }

    // find last entry in list (which may be 1st)
    pLast = pList;
    while(pLast)
    {
      if(pLast->pNext == NULL || pLast->pNext == pList)
        break;
      pLast = pLast->pNext;
    }
    if(!fDouble && pLast->pPrev != NULL)
    {
      FlyTestPrintf("!fDouble && pLast->pNext != NULL\n");
      return FALSE;
    }
    if(fDouble && fCircular && pLast->pNext == NULL)
    {
      FlyTestPrintf("fDouble && fCircular && pLast->pNext == NULL\n");
      return FALSE;
    }
    if(fDouble && !fCircular && pLast->pNext != NULL)
    {
      FlyTestPrintf("fDouble && !fCircular && pLast->pNext != NULL\n");
      return FALSE;
    }
  }

  return TRUE;
}

/*-------------------------------------------------------------------------------------------------
  Clear the links. Leave data alone
-------------------------------------------------------------------------------------------------*/
void TestListClearLinks(myList_t *pList)
{
  myList_t *pThis = pList;
  myList_t *pNext;

  while(pThis)
  {
    pNext = pThis->pNext;
    pThis->pNext = pThis->pPrev = NULL;
    pThis = pNext;
  }
}

/*-------------------------------------------------------------------------------------------------
  Test FlyListAppend()
-------------------------------------------------------------------------------------------------*/
void TcListAppend(void)
{
  myList_t *pList;
  myData_t  aData[] = {
    { .id=102934, .szName="Abby" },
    { .id=992345, .szName="Bobby" },
    { .id=633328, .szName="Charlie" }
  };
  myList_t  aUsers[NumElements(aData)];
  bool_t    fCircular;
  bool_t    fDouble;
  unsigned  i;

  FlyTestBegin();

  // add users to list
  for(fCircular = 0; fCircular < 2; ++fCircular)
  {
    for(fDouble = 0; fDouble < 2; ++fDouble)
    {
      pList = NULL;
      TestListInitArray(NumElements(aData), aUsers, aData);
      if(FlyTestVerbose())
        FlyTestPrintf("\nfCircular %u, fDouble %u\n", fCircular, fDouble);
      for(i = 0; i < NumElements(aUsers); ++i)
        pList = FlyListAppendEx(pList, &aUsers[i], fCircular, fDouble);
      if(FlyTestVerbose())
        TestListPrint(pList);
      if(!TestListLinksOK(pList, NumElements(aUsers), fCircular, fDouble))
        FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyListAppend()
-------------------------------------------------------------------------------------------------*/
void TcListPrepend(void)
{
  myList_t *pList;
  myData_t  aData[] = {
    { .id=102934, .szName="Abby" },
    { .id=992345, .szName="Bobby" },
    { .id=633328, .szName="Charlie" }
  };
  myList_t  aUsers[NumElements(aData)];
  bool_t    fCircular;
  bool_t    fDouble;
  unsigned  i;

  FlyTestBegin();

  // add users to list
  for(fCircular = 0; fCircular < 2; ++fCircular)
  {
    for(fDouble = 0; fDouble < 2; ++fDouble)
    {
      pList = NULL;
      TestListInitArray(NumElements(aData), aUsers, aData);
      if(FlyTestVerbose())
        FlyTestPrintf("\nfCircular %u, fDouble %u\n", fCircular, fDouble);
      for(i = 0; i < NumElements(aUsers); ++i)
      {
        pList = FlyListPrependEx(pList, &aUsers[i], fCircular, fDouble);
        if(pList != &aUsers[i])
          FlyTestFailed();
      }
      if(FlyTestVerbose())
        TestListPrint(pList);
      if(!TestListLinksOK(pList, NumElements(aUsers), fCircular, fDouble))
        FlyTestFailed();
    }
  }

  // charlie should be first
  if(strcmp(pList->szName, "Charlie") != 0)
    FlyTestFailed();

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Helper to TcListAddSorted()
-------------------------------------------------------------------------------------------------*/
int ListCmpEx(void *pArg, const void *pThis, const void *pThat)
{
  const myList_t   *pItem1 = pThis;
  const myList_t   *pItem2 = pThat;
  TcAddSortedArg_t *pA     = pArg;
  int               ret;

  ret = strcmp(pItem1->szName, pItem2->szName);
  if(FlyTestVerbose())
  {
    FlyTestPrintf("ListCmpEx(%s, %s) == %d, fCircular %u, fDouble %u\n", pItem1->szName,
      pItem2->szName, ret, pArg ? pA->fCircular : 0, pArg ? pA->fDouble : 0);
  }

  return ret;
}

/*-------------------------------------------------------------------------------------------------
  Helper to TcListAddSorted()
-------------------------------------------------------------------------------------------------*/
int ListCmp(const void *pThis, const void *pThat)
{
  const myList_t   *pItem1 = pThis;
  const myList_t   *pItem2 = pThat;
  int               ret;

  ret = strcmp(pItem1->szName, pItem2->szName);
  if(FlyTestVerbose())
    FlyTestPrintf("ListCmpEx(%s, %s) == %d\n", pItem1->szName, pItem2->szName, ret);

  return ret;
}

/*-------------------------------------------------------------------------------------------------
  Test FlyListAddSorted()
-------------------------------------------------------------------------------------------------*/
void TcListAddSorted(void)
{
  myList_t *pList;
  myData_t  aData[] = {
    { .id=633328, .szName="Charlie" },
    { .id=222222, .szName="Don" },
    { .id=102934, .szName="Abby" },
    { .id=765432, .szName="Eve" },
    { .id=992345, .szName="Bobby" },
  };
  myList_t    aUsers[NumElements(aData)];
  myList_t   *pThis;


  const char       *aszExpName[] = { "Abby", "Bobby", "Charlie", "Don", "Eve" };
  bool_t            fCircular;
  bool_t            fDouble;
  TcAddSortedArg_t  arg;
  unsigned          i;

  FlyTestBegin();

  // add users to list
  for(fCircular = 0; fCircular < 2; ++fCircular)
  {
    for(fDouble = 0; fDouble < 2; ++fDouble)
    {
      arg.fCircular = fCircular;
      arg.fDouble = fDouble;
      pList = NULL;
      TestListInitArray(NumElements(aData), aUsers, aData);
      if(FlyTestVerbose())
        FlyTestPrintf("\nfCircular %u, fDouble %u\n", fCircular, fDouble);
      for(i = 0; i < NumElements(aUsers); ++i)
      {
        pList = FlyListAddSortedEx(pList, &aUsers[i], fCircular, fDouble, &arg, ListCmpEx);
        // if(FlyTestVerbose())
        //   TestListPrint(pList);
      }

      if(FlyTestVerbose())
        TestListPrint(pList);
      if(!TestListLinksOK(pList, NumElements(aUsers), fCircular, fDouble))
        FlyTestFailed();

      // verify list is sorted properly
      i = 0;
      pThis = pList;
      while(pThis)
      {
        if(i >= NumElements(aData))
          FlyTestFailed();
        if(strcmp(pThis->szName, aszExpName[i]) != 0)
        {
          FlyTestPrintf("\n%u: got %s, exp %s\n", i, pThis->szName, aszExpName[i]);
          FlyTestFailed();
        }
        ++i;
        if(pThis->pNext == pList)
          break;
        pThis = pThis->pNext;
      }
    }
  }

  pList = NULL;
  TestListInitArray(NumElements(aData), aUsers, aData);
  for(i = 0; i < NumElements(aUsers); ++i)
  {
    pList = FlyListAddSorted(pList, &aUsers[i], ListCmp);
    if(FlyTestVerbose())
      TestListPrint(pList);
  }

  // if(FlyTestVerbose())
  //   TestListPrint(pList);
  if(!TestListLinksOK(pList, NumElements(aUsers), FALSE, FALSE))
    FlyTestFailed();

  // verify list is sorted properly
  i = 0;
  pThis = pList;
  while(pThis)
  {
    if(i >= NumElements(aData))
      FlyTestFailed();
    if(strcmp(pThis->szName, aszExpName[i]) != 0)
    {
      FlyTestPrintf("\n%u: got %s, exp %s\n", i, pThis->szName, aszExpName[i]);
      FlyTestFailed();
    }
    ++i;
    if(pThis->pNext == pList)
      break;
    pThis = pThis->pNext;
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Helper to TcListRemoveEx()
-------------------------------------------------------------------------------------------------*/
static myList_t * FindNode(const myList_t *pList, const char *szName)
{
  const myList_t *pFound  = NULL;
  const myList_t *pThis   = pList;

  while(pThis)
  {
    if(strcmp(pThis->szName, szName) == 0)
    {
      pFound = pThis;
      break;
    }
    if(pThis->pNext == pList)
      break;
    pThis = pThis->pNext;
  }

  return (myList_t *)pFound;
}

/*-------------------------------------------------------------------------------------------------
  Helper to TcListRemoveEx()
-------------------------------------------------------------------------------------------------*/
static myList_t * GetNthNode(const myList_t *pList, unsigned nth)
{
  const myList_t *pThis   = pList;

  while(pThis && nth)
  {
    pThis = pThis->pNext;
    --nth;
  }
  return (myList_t *)pThis;
}

/*-------------------------------------------------------------------------------------------------
  Test FlyListRemoveEx()
-------------------------------------------------------------------------------------------------*/
void TcListRemove(void)
{
  myList_t *pList;
  myData_t  aData[] = {
    { .id=102934, .szName="Abby" },
    { .id=992345, .szName="Bobby" },
    { .id=633328, .szName="Charlie" },
    { .id=222222, .szName="Don" },
    { .id=765432, .szName="Eve" },
  };
  myList_t    aUsers[NumElements(aData)];
  myList_t   *pThis;

  bool_t            fCircular;
  bool_t            fDouble;
  TcAddSortedArg_t  arg;
  unsigned          i;

  FlyTestBegin();

  // add users to list
  for(fCircular = 0; fCircular < 2; ++fCircular)
  {
    for(fDouble = 0; fDouble < 2; ++fDouble)
    {
      // initialize list structures
      arg.fCircular = fCircular;
      arg.fDouble = fDouble;
      TestListInitArray(NumElements(aData), aUsers, aData);
      if(FlyTestVerbose())
        FlyTestPrintf("\nfCircular %u, fDouble %u\n", fCircular, fDouble);

      // create the list we'll delete from
      pList = NULL;
      for(i = 0; i < NumElements(aUsers); ++i)
        pList = FlyListAppendEx(pList, &aUsers[i], fCircular, fDouble);

      // verify list was created properly
      if(FlyTestVerbose())
        TestListPrint(pList);
      if(!TestListLinksOK(pList, NumElements(aUsers), fCircular, fDouble))
        FlyTestFailed();

      if(FlyListLen(pList) != NumElements(aData))
        FlyTestFailed();

      // remove Charlie (middle node)
      pThis = GetNthNode(pList, 2);
      if(!pThis)
        FlyTestFailed();
      if(FlyTestVerbose())
        FlyTestPrintf("Removing Charlie...\n");
      pList = FlyListRemoveEx(pList, pThis, fCircular, fDouble);
      if(FlyTestVerbose())
        TestListPrint(pList);
      if(!pList || strcmp(pList->szName, "Abby") != 0 || FindNode(pList, "Charlie"))
        FlyTestFailed();

      // remove Abby (1st node)
      if(FlyTestVerbose())
        FlyTestPrintf("Removing Abby...\n");
      pList = FlyListRemoveEx(pList, pList, fCircular, fDouble);
      if(FlyTestVerbose())
        TestListPrint(pList);
      if(!pList || strcmp(pList->szName, "Bobby") != 0 || FindNode(pList, "Abby"))
        FlyTestFailed();

      // remove Eve (node at end)
      pThis = GetNthNode(pList, 2);
      if(!pThis || strcmp(pThis->szName, "Eve") != 0)
        FlyTestFailed();
      if(FlyTestVerbose())
        FlyTestPrintf("Removing Eve...\n");
      pList = FlyListRemoveEx(pList, pThis, fCircular, fDouble);
      if(FlyTestVerbose())
        TestListPrint(pList);
      if(!pList || strcmp(pList->szName, "Bobby") != 0 || FindNode(pList, "Eve"))
        FlyTestFailed();

      // all nodes
      if(FlyTestVerbose())
        FlyTestPrintf("Removing all the rest...\n");
      pList = FlyListRemoveEx(pList, pList, fCircular, fDouble);
      if(FlyTestVerbose())
        TestListPrint(pList);
      pList = FlyListRemoveEx(pList, pList, fCircular, fDouble);
      if(FlyTestVerbose())
        TestListPrint(pList);
      if(pList)
        FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyListPrev()
-------------------------------------------------------------------------------------------------*/
void TcListPrev(void)
{
  myList_t *pList;
  myData_t  aData[] = {
    { .id=102934, .szName="Abby" },
    { .id=992345, .szName="Bobby" },
    { .id=633328, .szName="Charlie" },
    { .id=222222, .szName="Don" },
    { .id=765432, .szName="Eve" },
  };
  myList_t    aUsers[NumElements(aData)];
  myList_t   *pThis;
  const char       *aszExpName[] = { "Abby", "Bobby", "Charlie", "Don", "Eve" };
  bool_t            fCircular;
  bool_t            fDouble;
  TcAddSortedArg_t  arg;
  unsigned          i;
  unsigned          len;

  FlyTestBegin();

  // add users to list
  for(fCircular = 0; fCircular < 2; ++fCircular)
  {
    for(fDouble = 0; fDouble < 2; ++fDouble)
    {
      arg.fCircular = fCircular;
      arg.fDouble = fDouble;
      pList = NULL;
      TestListInitArray(NumElements(aData), aUsers, aData);
      if(FlyTestVerbose())
        FlyTestPrintf("\nfCircular %u, fDouble %u\n", fCircular, fDouble);
      for(i = 0; i < NumElements(aUsers); ++i)
        pList = FlyListAddSortedEx(pList, &aUsers[i], fCircular, fDouble, &arg, ListCmpEx);
      len = FlyListLen(pList);
      if(!pList || len != NumElements(aData))
      {
        FlyTestPrintf("pList %p, len %u\n", pList, len);
        FlyTestFailed();
      }

      if(FlyTestVerbose())
        TestListPrint(pList);
      if(!TestListLinksOK(pList, NumElements(aUsers), fCircular, fDouble))
        FlyTestFailed();

      // get last element
      pThis = pList;
      for(i = 0; i < len - 1; ++i)
      {
        pThis = pThis->pNext;
        if(!pThis)
          FlyTestFailed();
      }

      // make sure previous gives us the proper element
      for(i = len; i > 0; --i)
      {
        if(pThis == NULL || pThis->szName == NULL)
          FlyTestFailed();
        if(strcmp(pThis->szName, aszExpName[i - 1]) != 0)
        {
          FlyTestPrintf("i %u, pThis->szName %s, exp %s\n", i, pThis->szName, aszExpName[i - 1]);
          FlyTestFailed();
        }
        pThis = FlyListPrevEx(pList, pThis, fCircular, fDouble);
      }

      if(pThis != NULL)
        FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test each function
-------------------------------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
  const char          szName[] = "test_list";
  const sTestCase_t   aTestCases[] =
  {
    { "TcListAppend",     TcListAppend },
    { "TcListPrepend",    TcListPrepend },
    { "TcListAddSorted",  TcListAddSorted },
    { "TcListRemove",     TcListRemove },
    { "TcListPrev",       TcListPrev },
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
