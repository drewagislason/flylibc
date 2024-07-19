/**************************************************************************************************
  FlyList.c - Genereic linked list handling
  Copyright 2024 Drew Gislason
  license: <https://mit-license.org>
**************************************************************************************************/
#include "FlyList.h"

/*!
  @defgroup FlyList generic linked list handling.

  Features:

  * Works with single linked lists, double linked lists (bidirectional)
  * Works with linear and circular (wrap-around) lists
  * Lists of any structure
  * Does not allocate any memory
  * Does not move structures, only updates links
  * Works with FlySort.h, FlySortList()

  Rules:

  1. Links MUST be at the top of the stucture (make sure optimizing does not move them)
  2. Only double lists need pPrev. pNext comes before pPrev if double-linked list
  3. Use the same fIsCircular and fIsDouble parameters for ALL calls to the same list
  4. The same item (ptr) MUST NEVER be in the same list twice, or the behavior is undefined

  @Example  Single Linked List Example

      #include "FlyList.h"

      typedef struct myStruct
      {
       struct myStruct *pNext;
       uint64_t         id;
       char            *szName;
      } myStruct_t;

      int main(void)
      {
        myStruct_t *pList = NULL;
        myStruct_t *pThis;
        myStruct_t  aUsers[] = {
          { .id=102934, .szName="My Name" },
          { .id=992345, .szName="Another Name" },
          { .id=633328, .szName="Who Knows" }
        };
        unsigned  i;

        // add users to list
        for(i = 0; i < NumElements(aUsers); ++i)
          pList = FlyListAppend(pList, &aUsers[i]);

        // print list
        pThis = pList;
        while(pThis)
        {
          printf("%p, user %llu: %s\n", pThis, pThis->id, pThis->szName);
          pThis = pThis->pNext;
        }

        return 0;
      }

  @Example  Double Linked List Example
      #include "FlyList.h"

      #define MAX_STRS 12
      typedef struct myDoubleList
      {
       struct myDoubleList *pNext;
       struct myDoubleList *pPrev;
       char                *sz;
      } myDoubleList_t;

      int main(void)
      {
        myDoubleList_t *pHead       = NULL;
        myDoubleList_t *pThis;
        myDoubleList_t *aUsers      = malloc(MAX_STRS * sizeof(myDoubleList_t));
        const char     *aszStrs[]   = { "Every", "Good", "Boy", "Does", "Fine" };
        bool_t          fIsCircular = TRUE;
        unsigned        i;
        bool_t          fIsCircular = TRUE;
        bool_t          fIsDouble   = TRUE;

        // add strings to list
        for(i = 0; i < NumElements(aszStrs); ++i)
        {
          aUsers[i].sz = aszStrs[i];
          pHead = FlyListAddSortedEx(pHead, &aUsers[i], fIsCircular, fIsDouble, FlyListCmpStrEx);
        }

        // print list
        pThis = pHead;
        for(i = 0; i < FlyListLenEx(pHead, fIsCircular, fIsDouble))
        {
          printf("user %d: %s\n", pThis->id, pThis->szName);
          pThis = pThis->pNext;
        }

        free(aUsers);
        return 0;
      }
*/

static void * ListAddSorted(void *pList, void *pItem, bool_t fIsCircular, bool_t fIsDouble, bool_t fEx, void *pArg, void *pCmpFunc);

/*-------------------------------------------------------------------------------------------------
  Find the node just before the end of the list

  @param  pList     ptr to head or NULL for new list
  @return ptr to last entry, or NULL if list is empty
*///-----------------------------------------------------------------------------------------------
static flyList_t * ListFindLast(const flyList_t *pList)
{
  const flyList_t * pLast = pList;
  while(pLast)
  {
    if(pLast->pNext == NULL || pLast->pNext == pList) 
      break;
    pLast = pLast->pNext;
  }
  return (flyList_t *)pLast;
}

/*-------------------------------------------------------------------------------------------------
  Find the node previous to pThat

  @param  pList     ptr to head or NULL for new list
  @return ptr to last entry, or NULL if list is empty
*///-----------------------------------------------------------------------------------------------
static flyList_t * ListFindPrev(const flyList_t *pList, const flyList_t *pThat)
{
  const flyList_t *pPrev = NULL;
  const flyList_t *pThis = pList;

  while(pThis)
  {
    if(pThis->pNext == NULL || pThis->pNext == pList) 
      break;
    if(pThis->pNext == pThat)
    {
      pPrev = pThis;
      break;
    }
    pThis = pThis->pNext;
  }

  return (flyList_t *)pPrev;
} 


/*!------------------------------------------------------------------------------------------------
  Add to the list in sorted order. Doesn't sort the list, just adds this one item in proper place.

  The compare function returns -1 for pItem < pItemInList, 0 if equal, 1 if >. 

  @param  pList     ptr to head or NULL for new list
  @param  pItem     ptr to allocated, static structure with *pNext as first field
  @param  pfnCmp    compare function
  @return ptr to (possibly new) head
*///-----------------------------------------------------------------------------------------------
void * FlyListAddSorted(void *pList, void *pItem, pfnListCmp_t pfnCmp)
{
  return ListAddSorted(pList, pItem, FALSE, FALSE, FALSE, NULL, (void *)pfnCmp);
}

/*!------------------------------------------------------------------------------------------------
  Append the item to the list. Returns the head.

  @param  pList     ptr to head or NULL for new list
  @param  pItem     ptr to allocated, static structure with *pNext as first field
  @return ptr to (possibly new) head
*///-----------------------------------------------------------------------------------------------
void * FlyListAppend(void *pList, void *pItem)
{
  return FlyListAppendEx(pList, pItem, FALSE, FALSE);
}

/*!------------------------------------------------------------------------------------------------
  Insert pItem after pThis. Fixup all ptrs.

  @param  pList     ptr to head or NULL for new list
  @param  pItem     ptr to allocated, static structure with *pNext as first field
  @param  pThis     ptr to item in list, or NULL (append)
  @return ptr to (possibly new) head
*///-----------------------------------------------------------------------------------------------
void * FlyListInsAfter(void *pList, void *pItem, void *pThis)
{
  return FlyListInsAfterEx(pList, pItem, FALSE, FALSE, pThis);
}

/*!------------------------------------------------------------------------------------------------
  Insert the item before pThis. If pThis is NULL, then appends.

  @param  pList         ptr to head or NULL for new list
  @param  pItem         ptr to allocated, static structure with *pNext as first field
  @param  pThis         ptr to item in list, or NULL (prepend)
  @return ptr to (possibly new) head
*///-----------------------------------------------------------------------------------------------
void * FlyListInsBefore(void *pList, void *pItem, void *pThis)
{
  return FlyListInsBeforeEx(pList, pItem, FALSE, FALSE, pThis);
}

/*!------------------------------------------------------------------------------------------------
  Insert the item before pThis. If pThis is not in list, then appends.

  If pThis is NOT in pList, then behavior is undefined.

  @param  pList         ptr to head or NULL for new list
  @param  pItem         ptr to an allocated, local or static structure with *pNext as first field
  @param  fIsCircular   list is circular (wraps)
  @param  fIsDouble     list has both pNext and pPrev (bidirectional)
  @param  pThis         item in list
  @return ptr to (possibly new) head
*///-----------------------------------------------------------------------------------------------
bool_t FlyListIsInList(void *pList, void *pItem)
{
  flyList_t  *pThis   = pList;
  bool_t      fInList = FALSE;

  while(pThis)
  {
    // found item
    if(pThis == pItem)
    {
      fInList = TRUE;
      break;
    }

    // end of wrapped list
    if(pThis->pNext == pList)
      break;

    pThis = pThis->pNext;
  }

  return fInList;
}

/*!------------------------------------------------------------------------------------------------
  Prepend the item to the list. Returns the item as head.

  @param  pList     ptr to head or NULL for new list
  @param  pItem     ptr to an allocated, local or static structure with *pNext as first field
  @return ptr to new head (pItem)
*///-----------------------------------------------------------------------------------------------
void * FlyListPrepend(void *pList, void *pItem)
{
  return FlyListPrependEx(pList, pItem, FALSE, FALSE);
}

/*!------------------------------------------------------------------------------------------------
  Get the previous item, or NULL if no more previous items. Always stops at head of list.

  @param  pList     ptr to head
  @param  pItem     ptr to an allocated, local or static structure with *pNext as first field
  @return ptr to previous item in list
*///-----------------------------------------------------------------------------------------------
void * FlyListPrev(void *pList, void *pItem)
{
  return FlyListPrevEx(pList, pItem, FALSE, FALSE);
}

/*!------------------------------------------------------------------------------------------------
  Remove the item from the list. All links are fixed up.

  @param  pList     ptr to head or NULL for new list
  @param  pItem     ptr to an allocated, local or static structure with *pNext as first field
  @param  pfnCmp    compare function
  @return ptr to (possibly new) head
*///-----------------------------------------------------------------------------------------------
void * FlyListRemove(void *pList, void *pItem)
{
  return FlyListRemoveEx(pList, pItem, FALSE, FALSE);
}

/*!------------------------------------------------------------------------------------------------
  Get the # of items in the list.

  @param  pList     ptr to head or NULL for new list
  @return length of list (# of items)
*///-----------------------------------------------------------------------------------------------
size_t FlyListLen(const void *pList)
{
  const flyList_t  *pThis   = pList;
  size_t            nItems  = 0;

  while(pThis)
  {
    ++nItems;
    if(pThis->pNext == NULL || pThis->pNext == pList)
      break;
    pThis = pThis->pNext;
  };

  return nItems;
}

/*!------------------------------------------------------------------------------------------------
  Prepend the item to the list. Returns the item as list head.

  pItem must NOT be in a list.

  @param  pHead         ptr to head or NULL for new list
  @param  pItem         ptr to allocated, static structure with *pNext as first field
  @param  fIsCircular   list is circular (wraps)
  @param  fIsDouble     list has both pNext and pPrev (bidirectional)
  @return ptr to new head (pItem)
*///-----------------------------------------------------------------------------------------------
void * FlyListPrependEx(void *pList, void *pItem, bool_t fIsCircular, bool_t fIsDouble)
{
  flyList_t  *pThis = pItem;

  // list is empty, pItem becomes the list, fix up links
  if(pList == NULL)
  {
    if(fIsCircular)
      pThis->pNext = pThis;
    else
      pThis->pNext = NULL;
    if(fIsDouble)
    {
      if(fIsCircular)
        pThis->pPrev = pThis;
      else
        pThis->pPrev = NULL;
    }
  }

  // list not empty
  else
    pList = FlyListInsBeforeEx(pList, pItem, fIsCircular, fIsDouble, pList);

  return pItem;
}

/*!------------------------------------------------------------------------------------------------
  Get the previous item, or NULL if no more previous items. Always stops at head of list.

  @param  pList     ptr to head
  @param  pItem     ptr to an allocated, local or static structure with *pNext as first field
  @param  fIsCircular   list is circular (wraps)
  @param  fIsDouble     list has both pNext and pPrev (bidirectional)
  @return ptr to previous item in list
*///-----------------------------------------------------------------------------------------------
void * FlyListPrevEx(void *pList, void *pItem, bool_t fIsCircular, bool_t fIsDouble)
{
  flyList_t  *pPrev = NULL;
  flyList_t  *pThis;

  // if no list or no item or item is already at head of list, already know previos is NULL
  if(pList && pItem && pList != pItem)
  {
    // double linked list, just return previous
    if(fIsDouble)
    {
      pThis = pItem;
      pPrev = pThis->pPrev;
    }

    else
    {
      pThis = pList;
      while(pThis)
      {
        // no more items in list
        if(fIsCircular && pThis->pNext == pList)
          break;

        // found previous item, return it
        if(pThis->pNext == pItem)
        {
          pPrev = pThis;
          break;
        }

        pThis = pThis->pNext;
      }
    }
  }

  return pPrev;
}

/*!------------------------------------------------------------------------------------------------
  Append the item to the list. Returns the head.

  @param  pHead         ptr to head or NULL for new list
  @param  pItem         ptr to allocated, static structure with *pNext as first field
  @param  fIsCircular   list is circular (wraps)
  @param  fIsDouble     list has both pNext and pPrev (bidirectional)
  @return ptr to (possibly new) head
*///-----------------------------------------------------------------------------------------------
void * FlyListAppendEx(void *pList, void *pItem, bool_t fIsCircular, bool_t fIsDouble)
{
  // list is empty, add it and fix up links
  if(pList == NULL)
    pList = FlyListPrependEx(pList, pItem, fIsCircular, fIsDouble);

  // add to end of list
  else
    pList = FlyListInsAfterEx(pList, pItem, fIsCircular, fIsDouble, NULL);

  return pList;
}

/*!------------------------------------------------------------------------------------------------
  Add to the list in sorterd order. Shared by FlyListAddSorted() and FlyListAddSortedEx()

  @param  pList         ptr to head or NULL for new list
  @param  pItem         ptr to allocated, static structure with *pNext as first field
  @param  fIsCircular   list is circular (wraps)
  @param  fIsDouble     list has both pNext and pPrev (bidirectional)
  @param  fEx           Is this the ex compare?
  @param  pArg          Argument (exta info) to ex compare.
  @param  pfnCmp        compare function
  @return ptr to (possibly new) head
*///-----------------------------------------------------------------------------------------------
static void * ListAddSorted(
  void   *pList,
  void   *pItem,
  bool_t  fIsCircular,
  bool_t  fIsDouble,
  bool_t  fEx,
  void   *pArg,
  void   *pCmpFunc)
{
  pfnListCmp_t    pfnCmp    = (pfnListCmp_t)pCmpFunc;
  pfnListCmpEx_t  pfnCmpEx  = (pfnListCmpEx_t)pCmpFunc;
  flyList_t      *pThat;
  int             ret;

// printf("ListAddSorted(pList=%p, pItem=%p, fIsCircular=%u, fIsDouble=%u. fEx=%u, pArg=%p, pCmdFunc=%p\n",
//   pList, pItem, fIsCircular, fIsDouble, fEx, pArg, pCmpFunc);

  // empty just, just add it
  if(pList == NULL)
    pList = FlyListPrependEx(pList, pItem, fIsCircular, fIsDouble);
  else
  {
    pThat = pList;
    while(pThat)
    {
      if(fEx)
        ret = pfnCmpEx(pArg, pItem, pThat);
      else
        ret = pfnCmp(pItem, pThat);

      // is this the right place, insert it and be done
      if(ret <= 0)
      {
        // printf("...before %p\n", pThat);
        pList = FlyListInsBeforeEx(pList, pItem, fIsCircular, fIsDouble, pThat);
        break;
      }

      // at end of list, insert after
      if(pThat->pNext == NULL || pThat->pNext == pList)
      {
        // printf("...after %p\n", pThat);
        pList = FlyListInsAfterEx(pList, pItem, fIsCircular, fIsDouble, pThat);
        break;
      }

      pThat = pThat->pNext;
    }
  }

  return pList;
}

/*!------------------------------------------------------------------------------------------------
  Add to the list in its proper place to keep list sorted.

  @param  pHead         ptr to head or NULL for new list
  @param  pItem         ptr to allocated, static structure with *pNext as first field
  @param  fIsCircular   list is circular (wraps)
  @param  fIsDouble     list has both pNext and pPrev (bidirectional)
  @param  pArg          argument to compare function or NULL
  @param  pfnCmp        user supplied compare function
  @return ptr to (possibly new) head
*///-----------------------------------------------------------------------------------------------
void * FlyListAddSortedEx(void *pList, void *pItem, bool_t fIsCircular, bool_t fIsDouble, void *pArg, pfnListCmpEx_t pfnCmpEx)
{
  return ListAddSorted(pList, pItem, fIsCircular, fIsDouble, TRUE, pArg, (void *)pfnCmpEx);
}

/*!------------------------------------------------------------------------------------------------
  Insert pItem after pThis. Fixup all ptrs.

  If pThis is NOT in pList, then behavior is undefined.

  @param  pThis         ptr to an item in a list
  @param  pItem         ptr to allocated, static structure with *pNext as first field
  @param  fIsCircular   list is circular (wraps)
  @param  fIsDouble     list has both pNext and pPrev (bidirectional)
  @param  pThat         ptr to item in list, or NULL to append
  @return ptr to (possibly new) head
*///-----------------------------------------------------------------------------------------------
void * FlyListInsAfterEx(void *pList, void *pItem, bool_t fIsCircular, bool_t fIsDouble, void *pThat)
{
  flyList_t  *pNode   = pItem;
  flyList_t  *pPrev   = pThat;

  // list is empty, so create the list
  if(pList == NULL)
    pList = FlyListPrependEx(pList, pItem, fIsCircular, fIsDouble);

  else
  {
    // user wants end of list
    if(pThat == NULL)
      pPrev = ListFindLast(pList);

    // fixup this node
    pNode->pNext = pPrev->pNext;
    if(fIsDouble)
      pNode->pPrev = pPrev;

    // fixup next node
    if(fIsDouble && pNode->pNext)
      pNode->pNext->pPrev = pNode;

    // fixup previous node
    pPrev->pNext = pNode;
  }

  return pList;
}

/*!------------------------------------------------------------------------------------------------
  Insert the item before pThis. If pThis is not in list, then appends.

  If pThis is NOT in pList, then behavior is undefined.

  @param  pList         ptr to head or NULL for new list
  @param  pItem         ptr to allocated, static structure with *pNext as first field
  @param  fIsCircular   list is circular (wraps)
  @param  fIsDouble     list has both pNext and pPrev (bidirectional)
  @param  pThis         item in list, or NULL to prepend
  @return ptr to (possibly new) head
*///-----------------------------------------------------------------------------------------------
void * FlyListInsBeforeEx(void *pList, void *pItem, bool_t fIsCircular, bool_t fIsDouble, void *pThat)
{
  flyList_t *pNode = pItem;
  flyList_t *pNext = pThat;
  flyList_t *pPrev = NULL;
  flyList_t *pLast = NULL;

  // list is empty, create it with the 1 item
  if(pList == NULL)
    pList = FlyListPrependEx(pList, pItem, fIsCircular, fIsDouble);

  else
  {
    if(pNext == NULL)
      pNext = pList;

    if(!fIsDouble && fIsCircular && pNext == pList)
      pLast = ListFindLast(pList);
    else if(!fIsDouble && pNext != pList)
      pPrev = ListFindPrev(pList, pNext);

    // node will now be head of list
    if(pThat == pList)
      pList = pNode;

    // fixup this node
    pNode->pNext = pNext;
    if(fIsDouble)
      pNode->pPrev = pNext->pPrev;

    // fixup next node
    if(fIsDouble)
      pNext->pPrev = pNode;

    // fixup previous node
    if(fIsDouble && pNode->pPrev)
      pNode->pPrev->pNext = pNode;
    if(pPrev)
      pPrev->pNext = pNode;

    // we inserted at head of list, so circular single last node points to new head
    if(pLast)
      pLast->pNext = pNode;
  }

  return pList;
}

/*!------------------------------------------------------------------------------------------------
  Remove the item from the list. All links are fixed up.

  @param  pList         ptr to head or NULL for new list
  @param  pItem         ptr to allocated, static structure with *pNext as first field
  @param  fIsCircular   list is circular (wraps)
  @param  fIsDouble     list has both pNext and pPrev (bidirectional)
  @return ptr to (possibly new) head
*///-----------------------------------------------------------------------------------------------
void * FlyListRemoveEx(void *pList, void *pItem, bool_t fIsCircular, bool_t fIsDouble)
{
  flyList_t *pThis = pItem;
  flyList_t *pPrev = NULL;
  flyList_t *pLast = NULL;

  // removing head on circular single list, so last must point to new head
  if(fIsCircular && pThis == pList)
    pLast = ListFindLast(pList);

  if(pThis != pList)
    pPrev = ListFindPrev(pList, pThis);

  // removing head, so it will change to next node
  if(pThis == pList)
  {
    if(pThis->pNext == pList)
      pList = NULL;
    else
      pList = pThis->pNext;
  }

  // fixup previous node
  if(pPrev)
    pPrev->pNext = pThis->pNext;

  // fixup next node
  if(fIsDouble && pThis->pNext)
    pThis->pNext->pPrev = pThis->pPrev;

  // fixup last node on single circular
  if(pLast)
    pLast->pNext = pList;

  // fixup this node
  pThis->pNext = NULL;
  if(fIsDouble)
    pThis->pPrev = NULL;

  return pList;
}
