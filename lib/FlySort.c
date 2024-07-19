/**************************************************************************************************
  FlySort.c - Sort linked lists and arrays
  Copyright 2024 Drew Gislason  
  license: <https://mit-license.org>
*///***********************************************************************************************
#include "FlySort.h"

/*!
  @defgroup FlySort Sort linked lists, strings, structures and numbers.

  Features:

  * Sort any data types, both native and struct, unions
  * Bubble sort for smallest code (moves objects/data around)
  * QSort for reasonably fast sort for random data (moves objects/data around)
  * Sort (via merge sort) linked list or array structures in-place. Links change only.
  * Provide comparison function (with arg if needed) to sort forward, backward or any criteria 
  * Prebuilt comparison functions for common built-in C types

  For a good discussion of sorting algorithms, see:  
  <https://www.interviewkickstart.com/learn/merge-sort-vs-quicksort-performance-analysis>

  Some types shared by the sort functions:

      typedef int  (*pfnSortCmp_t)    (const void *pThis, const void *pThat);
      typedef int  (*pfnSortCmpEx_t)  (void *pArg, const void *pThis, const void *pThat);

  The `pArg` is any argument you want passed to the compare functions. It has no meaning to the
  built-in compare functions.

  The return int is the same as strcmp(), `< 0` means this is less than that, 0 is equal, 1 is
  greater than.

  If you want to avoid potentially pulling in the double math libraries, use: -DFLY_FLAG_NO_MATH
  which will #if out anything that uses floats or doubles.
*/

typedef struct flySortList
{
  struct flySortList  *pNext;
  struct flySortList  *pPrev;
  char                 aData[];
} flySortList_t;

/*!------------------------------------------------------------------------------------------------
  Basic buble sort. Swaps items in array. To reverse, just provide a different compare function.

  @param  pArray    ptr to array of items
  @param  nElem     number of elements in the array
  @param  elemSize  size of each element
  @param  pfnCmp    compare function
  @param  pfnSwap   swap function
  @return none
*///-----------------------------------------------------------------------------------------------
void FlySortBubble(void *pArray, unsigned nElem, unsigned elemSize, pfnSortCmp_t pfnCmp)
{
  unsigned  i, j;
  unsigned  size;
  bool_t    fSwapped;
  uint8_t   *pThis;
  uint8_t   *pThat;
  uint8_t   tmp;

  if(nElem > 0 && elemSize > 0)
  {
    for(i = 0; i < nElem - 1; ++i)
    {
      fSwapped = FALSE;
      for(j = 0; j < nElem - 1 - i; ++j)
      {
        pThis = (uint8_t *)pArray + ((size_t)j * elemSize);
        pThat = pThis + elemSize;
        if(pfnCmp(pThis, pThat) > 0)
        {
          // swap
          size = elemSize;
          do
          {
            tmp = *pThis;
            *pThis++ = *pThat;
            *pThat++ = tmp;
          } while(--size > 0);
          fSwapped = TRUE;
        }
      }
      if(!fSwapped)
        break;
    }
  }
}

/*!------------------------------------------------------------------------------------------------
  This is just another name for the librarys qsort_r.

  NOTE: manpages are often wrong on the order of arguments for the compare function. The correct
  order is:

      typedef int  (*pfnSortCmpEx_t)  (void *pArg, const void *pThis, const void *pThat);

  @param  pArray    ptr to array of items
  @param  nElem     number of elements in the array
  @param  elemSize  size of each element
  @param  pfnCmp    compare function
  @param  pArg      argument to compare function. Can be NULL.
  @return none
*///-----------------------------------------------------------------------------------------------
void FlySortQSort(void *pArray, size_t nElem, size_t elemSize, void *pArg, pfnSortCmpEx_t pfnCmp)
{
  qsort_r(pArray, nElem, elemSize, pArg, pfnCmp);
}

/*!------------------------------------------------------------------------------------------------
  In-place sort a single or double linked list using merge sort. The list can be circular or not.

  This doesn't move any of the linked list objects, only changes what the links are pointing to.

  IMPORTANT: make sure your struct starts with the links(pNext before pPrev). Example(s):

      typedef struct mySingleList
      {
        struct mySingleList  *pNext;
        // some data here...
      } mySingleList_t;

      typedef struct myDoubleList
      {
        struct myDoubleList  *pNext;
        struct myDoubleList  *pPrev;
        // some data here...
      } myDoubleList_t;

  @param  pList         Pointer to the head of the list
  @param  fIsCircular   Is the list circular?
  @param  fIsDouble     Is the list single or double linked?
  @param  pArg          any extra data needed by compare, or NULL
  @param  pfnCmp        compare function
  @return head of sorted list
*///-----------------------------------------------------------------------------------------------
void * FlySortList(void *pLinkedList, bool_t fIsCircular, bool_t fIsDouble, void *pArg, pfnSortCmpEx_t pfnCmp)
{
  flySortList_t *pList = pLinkedList;
  flySortList_t *p;
  flySortList_t *q;
  flySortList_t *e;
  flySortList_t *pTail;
  flySortList_t *pOldHead;
  int insize, nmerges, psize, qsize, i;

  if(!pList)
    return NULL;

  insize = 1;

  while(TRUE)
  {
    p = pList;
    pOldHead = pList; // only used for circular linkage
    pList = NULL;
    pTail = NULL;

    nmerges = 0;  // count number of merges we do in this pass

    while(p)
    {
      nmerges++;  // there exists a merge to be done

      // step `insize' places along from p
      q = p;
      psize = 0;
      for(i = 0; i < insize; i++)
      {
        psize++;
        if(fIsCircular)
          q = (q->pNext == pOldHead ? NULL : q->pNext);
        else
          q = q->pNext;
        if(!q)
          break;
      }

      //if q hasn't fallen off end, we have two lists to merge
      qsize = insize;

      /* now we have two lists; merge them */
      while(psize > 0 || (qsize > 0 && q))
      {
        // decide whether next element of merge comes from p or q
        if (psize == 0)
        {
          // p is empty; e must come from q.
          e = q; q = q->pNext; qsize--;
          if(fIsCircular && q == pOldHead)
            q = NULL;
        }
        else if (qsize == 0 || !q)
        {
          // q is empty; e must come from p.
          e = p; p = p->pNext; psize--;
          if(fIsCircular && p == pOldHead)
            p = NULL;
        }
        else if(pfnCmp(pArg, p, q) <= 0)
        {
          // First element of p is lower (or same); e must come from p.
          e = p; p = p->pNext; psize--;
          if(fIsCircular && p == pOldHead)
            p = NULL;
        }
        else
        {
          // First element of q is lower; e must come from q.
          e = q; q = q->pNext; qsize--;
          if(fIsCircular && q == pOldHead)
            q = NULL;
        }

        // add the next element to the merged list
        if(pTail)
          pTail->pNext = e;
        else
          pList = e;
    
        /* Maintain reverse pointers in a doubly linked list. */
        if(fIsDouble)
          e->pPrev = pTail;

        pTail = e;
      }

      // now p has stepped `insize' places along, and q has too
      p = q;
    }
  
    if(fIsCircular)
    {
      pTail->pNext = pList;
      if(fIsDouble)
        pList->pPrev = pTail;
    }
    else
    {
      pTail->pNext = NULL;
    }

    // If we have done only one merge, we're finished.
    if(nmerges <= 1)
      break;

    // Otherwise repeat, merging lists twice the size
    insize *= 2;
  }

  return pList;
}

#ifndef FLY_FLAG_NO_MATH
/*!------------------------------------------------------------------------------------------------
  Compare two unsigned. Returns -1 if this < that, 0 if same, 1 if this > that.

  @param  pThis   ptr to an unsigned
  @param  pThat   ptr to an unsigned
  @return Returns -1 if this < that, 0 if same, 1 if this > that.
*///-----------------------------------------------------------------------------------------------
int FlySortCmpDouble(const void *pThis, const void *pThat)
{
  if(*(double *)pThis == *(double *)pThat)
    return 0;
  return (*(double *)pThis < *(double *)pThat) ? -1 : 1;
}

/*!------------------------------------------------------------------------------------------------
  Compare two unsigned. Returns -1 if this < that, 0 if same, 1 if this > that.

  @param  pArg    ignored. For compatibility only
  @param  pThis   ptr to an unsigned
  @param  pThat   ptr to an unsigned
  @return Returns -1 if this < that, 0 if same, 1 if this > that.
*///-----------------------------------------------------------------------------------------------
int FlySortCmpDoubleEx(void *pArg, const void *pThis, const void *pThat)
{
  (void)pArg;
  return FlySortCmpDouble(pThis, pThat);
}  

#endif  // FLY_FLAG_NO_MATH

/*!------------------------------------------------------------------------------------------------
  Compare two ints. Returns -1 if this < that, 0 if same, 1 if this > that.

  @param  pThis   ptr to an int
  @param  pThat   ptr to an int
  @return Returns -1 if this < that, 0 if same, 1 if this > that.
*///-----------------------------------------------------------------------------------------------
int FlySortCmpInt(const void *pThis, const void *pThat)
{
  if(*(int *)pThis == *(int *)pThat)
    return 0;
  return (*(int *)pThis < *(int *)pThat) ? -1 : 1;
}

/*!------------------------------------------------------------------------------------------------
  Compare two ints. Returns -1 if this < that, 0 if same, 1 if this > that.

  @param  pArg    ignored. For compatibility only
  @param  pThis   ptr to an int
  @param  pThat   ptr to an int
  @return Returns -1 if this < that, 0 if same, 1 if this > that.
*///-----------------------------------------------------------------------------------------------
int FlySortCmpIntEx(void *pArg, const void *pThis, const void *pThat)
{
  (void)pArg;
  return FlySortCmpInt(pThis, pThat);
}

/*!------------------------------------------------------------------------------------------------
  Compare two strings. Returns -1 if this < that, 0 if same, 1 if this > that.

  @param  pThis   ptr to a string ptr
  @param  pThat   ptr to a string ptr
  @return Returns -1 if this < that, 0 if same, 1 if this > that.
*///-----------------------------------------------------------------------------------------------
int FlySortCmpStr(const void *pThis, const void *pThat)
{
  const char * const *ppThis = pThis;
  const char * const *ppThat = pThat;
  return strcmp(*ppThis, *ppThat);
}

/*!------------------------------------------------------------------------------------------------
  Compare two strings. Returns -1 if this < that, 0 if same, 1 if this > that.

  @param  pArg    ignored. For compatibility only
  @param  pThis   ptr to a string ptr
  @param  pThat   ptr to a string ptr
  @return Returns -1 if this < that, 0 if same, 1 if this > that.
*///-----------------------------------------------------------------------------------------------
int FlySortCmpStrEx(void *pArg, const void *pThis, const void *pThat)
{
  (void)pArg;
  return FlySortCmpStr(pThis, pThat);
}

/*!------------------------------------------------------------------------------------------------
  Compare two unsigned. Returns -1 if this < that, 0 if same, 1 if this > that.

  @param  pThis   ptr to an int
  @param  pThat   ptr to an int
  @return Returns -1 if this < that, 0 if same, 1 if this > that.
*///-----------------------------------------------------------------------------------------------
int FlySortCmpUnsigned(const void *pThis, const void *pThat)
{
  if(*(unsigned *)pThis == *(unsigned *)pThat)
    return 0;
  return (*(unsigned *)pThis < *(unsigned *)pThat) ? -1 : 1;
}

/*!------------------------------------------------------------------------------------------------
  Compare two unsigned. Returns -1 if this < that, 0 if same, 1 if this > that.

  @param  pArg    ignored. For compatibility only
  @param  pThis   ptr to an int
  @param  pThat   ptr to an int
  @return Returns -1 if this < that, 0 if same, 1 if this > that.
*///-----------------------------------------------------------------------------------------------
int FlySortCmpUnsignedEx(void *pArg, const void *pThis, const void *pThat)
{
  (void)pArg;
  return FlySortCmpUnsigned(pThis, pThat);
}
