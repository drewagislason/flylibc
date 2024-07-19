/**************************************************************************************************
  FlyList.h
  Copyright 2024 Drew Gislason
  license: MIT <https://mit-license.org>
*///***********************************************************************************************
#include "Fly.h"

#ifndef FLY_LIST_H
#define FLY_LIST_H

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

typedef struct flyList
{
  struct flyList  *pNext;
  struct flyList  *pPrev;
} flyList_t;

// see also FlySort.h, as it uses this type
typedef int  (*pfnListCmp_t)    (const void *pThis, const void *pThat);
typedef int  (*pfnListCmpEx_t)  (void *pArg, const void *pThis, const void *pThat);

// single non-wrapping list
void     *FlyListAddSorted    (void *pList, void *pItem, pfnListCmp_t pfnCmp);
void     *FlyListAppend       (void *pList, void *pItem);
void     *FlyListInsAfter     (void *pList, void *pItem, void *pThis);
void     *FlyListInsBefore    (void *pList, void *pItem, void *pThis);
bool_t    FlyListIsInList     (void *pList, void *pItem);
size_t    FlyListLen          (const void *pList);
void     *FlyListPrepend      (void *pList, void *pItem);
void     *FlyListPrev         (void *pList, void *pItem);
void     *FlyListRemove       (void *pList, void *pItem);

// for double and wrapped lists, and argument for sort
void     *FlyListAddSortedEx  (void *pList, void *pItem, bool_t fIsCircular, bool_t fIsDouble, void *pArg, pfnListCmpEx_t pfnCmp);
void     *FlyListAppendEx     (void *pList, void *pItem, bool_t fIsCircular, bool_t fIsDouble);
void     *FlyListInsAfterEx   (void *pList, void *pItem, bool_t fIsCircular, bool_t fIsDouble, void *pThat);
void     *FlyListInsBeforeEx  (void *pList, void *pItem, bool_t fIsCircular, bool_t fIsDouble, void *pThat);
void     *FlyListPrependEx    (void *pList, void *pItem, bool_t fIsCircular, bool_t fIsDouble);
void     *FlyListPrevEx       (void *pList, void *pItem, bool_t fIsCircular, bool_t fIsDouble);
void     *FlyListRemoveEx     (void *pList, void *pItem, bool_t fIsCircular, bool_t fIsDouble);

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  }
#endif

#endif // FLY_LIST_H
