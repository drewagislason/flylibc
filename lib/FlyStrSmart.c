/**************************************************************************************************
  FlyStr.c - Smart strings, dynamic strings that don't overflow
  Copyright 2024 Drew Gislason
  License: MIT <https://mit-license.org>
*///***********************************************************************************************
#include <stdarg.h>
#include "FlyStr.h"

#define SMART_SIZE_MIN  32  // size to use if not specified

/*!
  @defgroup FlyStrSmart Smart (allocated) C strings that work like Python strings

  Ever work with Python strings? It sure would be nice to manipulate strings like that in C. Well,
  here you go.

  * Concatinate without worrying about overflow: s1 + s2
  * Get a slice of a string
  * String is always '\0' terminated and suitable for printf() or other C functions
  * Clone a string
  * Pass to functions by reference not by copy
  * Uses the heap, that is malloc(), to allocate strings
*/

/*!-------------------------------------------------------------------------------------------------
  Allocate (create) an empty smart string of a given size. Yes, the name is very funny, Smart Alec.

  @param    initialSize    initialSize of string to allocate (may be 0)
  @return ptr to static string with C version in it
*///------------------------------------------------------------------------------------------------
flyStrSmart_t * FlyStrSmartAlloc(size_t initialSize)
{
  flyStrSmart_t * pStr;

  // always need room for NUL
  if(initialSize == 0)
    ++initialSize;

  // contents might be reallocated, but smart string should never be reallocated (moved)
  // so allocate in 2 parts
  pStr = FlyAlloc(sizeof(flyStrSmart_t));
  if(pStr)
  {
    pStr->sz = FlyAlloc(initialSize);
    if(pStr->sz == NULL)
    {
      FlyFree(pStr);
      pStr = NULL;
    }
    else
    {
      memset(pStr->sz, 0, initialSize);
      pStr->size = initialSize;
    }
  }

  return pStr;
}

/*!-------------------------------------------------------------------------------------------------
  Initialize smart string. Don't use if using FlyStrSmartNew() or FlyStrSmartAlloc().

  Essentially, memset() the struct to 0x00s. Useful if making a local variable.

  @example FlyStrSmartInit Example

      flyStrSmart_t   myStr;

      FlyStrSmartInit(&myStr);
      FlyStrSmartCpy(&myStr, "Hello World");

  @param    pStr    ptr to smart string
  @return   none
*///------------------------------------------------------------------------------------------------
void FlyStrSmartInit(flyStrSmart_t *pStr)
{
  memset(pStr, 0, sizeof(*pStr));
}

/*!-------------------------------------------------------------------------------------------------
  Initialize smart string with a given size.

  Essentially, memset() the struct to 0x00s. Useful if making a local variable.

  @example FlyStrSmartInitEx Example

      flyStrSmart_t       myStr;
      static const char   szHello[] = "Hello World";

      if(FlyStrSmartInitEx(&myStr, sizeof(szHello)))
        strcpy(myStr->sz, szHello);

  @param    pStr    ptr to smart string
  @param    size    size to initialize smart string
  @return   TRUE if worked, FALSE if not enough memory
*///------------------------------------------------------------------------------------------------
bool_t FlyStrSmartInitEx(flyStrSmart_t *pStr, size_t size)
{
  FlyStrSmartInit(pStr);
  return FlyStrSmartResize(pStr, size);
}

/*!-------------------------------------------------------------------------------------------------
  Clear the string. Does not affect size of smart str.

  @param    pStr    ptr to smart string
  @return   none
*///------------------------------------------------------------------------------------------------
void FlyStrSmartClear(flyStrSmart_t *pStr)
{
  *pStr->sz = '\0';
}

/*!-------------------------------------------------------------------------------------------------
  Concatenate the string to end of smart string. Resizes mem if needed.

  If memory couldn't be allocated, smart string is left unchanged.

  @param    pStr    ptr to smart string
  @param    sz      string to concatinate to smart string
  @return ptr where sz was copied into smart string or NULL if memory couldn't be allocated
*///------------------------------------------------------------------------------------------------
char * FlyStrSmartCat(flyStrSmart_t *pStr, const char *sz)
{
  return FlyStrSmartNCat(pStr, sz, strlen(sz));
}

/*!-------------------------------------------------------------------------------------------------
  Copy the string to start of smart string. Resizes mem if needed.

  If memory couldn't be allocated, smart string is left unchanged.

  @param    pStr    ptr to smart string
  @param    sz      string to copy to smart string
  @return ptr where sz was copied into smart string or NULL if couldn't allocate memory
*///------------------------------------------------------------------------------------------------
char * FlyStrSmartCpy(flyStrSmart_t *pStr, const char *sz)
{
  size_t    len = strlen(sz);

  FlyStrSmartNCpy(pStr, sz, len);
  pStr->sz[len] = '\0';

  return pStr->sz;
}

/*!-------------------------------------------------------------------------------------------------
  Duplicate the start string.

  @param    pStr    ptr to smart string
  @return   new smart string filled with same string
*///------------------------------------------------------------------------------------------------
flyStrSmart_t * FlyStrSmartDup(flyStrSmart_t *pStr)
{
  return  FlyStrSmartNew(pStr->sz);
}

/*!-------------------------------------------------------------------------------------------------
  Shrinks mem if needed. Use FlyStrSmartStr() to access string.

  @param    size    size of string to allocate (may be 0)
  @return ptr to static string with C version in it
*///------------------------------------------------------------------------------------------------
void FlyStrSmartFit(flyStrSmart_t *pStr)
{
  size_t  newSize;
  char   *pszNew;

  newSize = strlen(pStr->sz) + 1;
  if(newSize < pStr->size)
  {
    pszNew = realloc(pStr->sz, pStr->size);
    if(pszNew)
    {
      pStr->sz = pszNew;
      pStr->size = newSize;
    }
  }
}

/*!-------------------------------------------------------------------------------------------------
  Free the smart string

  @param    pStr    ptr to smart string
  @return NULL
*///------------------------------------------------------------------------------------------------
void FlyStrSmartUnInit(flyStrSmart_t *pStr)
{
  if(pStr)
  {
    if(pStr->sz)
    {
      if(pStr->size)
        memset(pStr->sz, FLYSTRSMART_FILL, pStr->size);
      FlyFree(pStr->sz);
    }
  }
}

/*!-------------------------------------------------------------------------------------------------
  Free the smart string allocated with 

  @param    pStr    ptr to smart string
  @return NULL
*///------------------------------------------------------------------------------------------------
void * FlyStrSmartFree(flyStrSmart_t *pStr)
{
  if(pStr)
  {
    FlyStrSmartUnInit(pStr);
    memset(pStr, 0, sizeof(*pStr));
    FlyFree(pStr);
  }

  return  NULL;
}

/*!-------------------------------------------------------------------------------------------------
  Concatinate length of string to smart string. Resizes memory of needed.

  If memory couldn't be allocated, smart string is left unchanged.

  @param    pStr    ptr to smart string
  @param    sz      string to copy to smart string
  @param    len     length of string to concatenate to smart string
  @return ptr where sz was copied into smart string or NULL if couldn't allocate mem
*///------------------------------------------------------------------------------------------------
char * FlyStrSmartNCat(flyStrSmart_t *pStr, const char *sz, size_t len)
{
  char *psz = NULL;

  // printf("FlyStrSmartNCat(pStr->sz %s, sz %s, len %zu)\n", pStr->sz, sz, len);
  if(FlyStrSmartNeed(pStr, len))
  {
    psz = pStr->sz;
    psz += strlen(psz);
    // printf("  pStr->sz %p, psz %p, lenpsz %u\n", pStr->sz, psz, (unsigned)strlen(psz));
    strncpy(psz, sz, len);
    psz[len] = '\0';
  }

  return psz;
}

/*!-------------------------------------------------------------------------------------------------
  Copy length of string to smart string. Resizes memory of needed.

  If memory couldn't be allocated, smart string is left unchanged.

  @param    pStr    ptr to smart string
  @param    sz      string to copy to smart string
  @param    len     length of string to copy to smart string
  @return ptr where sz was copied into smart string
*///------------------------------------------------------------------------------------------------
char * FlyStrSmartNCpy(flyStrSmart_t *pStr, const char *sz, size_t len)
{
  char *psz = NULL;

  if(pStr->sz)
    *pStr->sz = '\0';
  psz = FlyStrSmartNCat(pStr, sz, len);

  return psz;
}

/*!-------------------------------------------------------------------------------------------------
  Make sure we can concatinate at least len bytes to smart string.

  @param    pStr    ptr to smart string
  @param    sz      string to copy to smart string
  @param    len     length of string to copy to smart string
  @return ptr to start of concatenated string.
*///------------------------------------------------------------------------------------------------
bool_t FlyStrSmartNeed(flyStrSmart_t *pStr, size_t len)
{
  bool_t  fWorked = TRUE;
  size_t  lenSz;
  size_t  newSize;

  lenSz = pStr->sz ? strlen(pStr->sz) : 0;
  // printf("FlyStrSmartNeed(pStr->size=%zu, len=%zu), lenSz %zu\n", pStr->size, len, lenSz);
  if((lenSz + len + 1) > pStr->size)
  {
    // how much space to we need?
    newSize = (lenSz + len + 1);

    //  minimize allocs by allocating double the space
    if(newSize < pStr->size * 2)
      newSize = pStr->size * 2;
    // printf("  newSize %zu\n", newSize);

    // do not change string, if we couldn't allocate space
    fWorked = FlyStrSmartResize(pStr, newSize);
  }

  return fWorked;
}

/*!-------------------------------------------------------------------------------------------------
  Create a new smart string from a normal asciiz C string.

  @param    sz      string to copy to smart stringm or NULL
  @return ptr to new smart string
*///------------------------------------------------------------------------------------------------
flyStrSmart_t * FlyStrSmartNew(const char *sz)
{
  return FlyStrSmartNewEx(sz, sz ? strlen(sz) + 1 : 0);
}

/*!-------------------------------------------------------------------------------------------------
  Create a new smart string from a normal asciiz C string with size.

  Example that makes a smart string out of a substring:

      flyStrSmart_t  *pSmart;
      const char      szSearch[] = "Good";

      pSmart = FlyStrSmartNewEx(strstr("Every Good Boy", szSearch), sizeof(szSearch));
      printf("found %s\n", pSmart->sz);

  @param    sz      string to copy to smart string, or NULL
  @param    size    size of smart string or 0 for default
  @return ptr to new smart string
*///------------------------------------------------------------------------------------------------
flyStrSmart_t * FlyStrSmartNewEx(const char *sz, size_t size)
{
  flyStrSmart_t  *pStr;
  size_t          len = 0;

  // 
  if(size == 0)
  {
    size = SMART_SIZE_MIN;
    len = strlen(sz);
    if(len + 1 > SMART_SIZE_MIN)
      size = len + 1;
  }

  pStr = FlyStrSmartAlloc(size);
  if(pStr)
  {
    if(sz)
    {
      strncpy(pStr->sz, sz, size - 1);
      pStr->sz[size - 1] = '\0';
    }
    else
      pStr->sz[0] = '\0';
  }

  return pStr;
}

/*--------------------------------------------------------------------------------------------------
  Return position based on string length and an integer that might be negative.
  Example: -2 on string "Hello World!" would point to the 'd'
*///------------------------------------------------------------------------------------------------
int SmartSlicePos(int pos, int szLen)
{
  if(pos < 0)
  {
    pos = -1 * pos;
    if(pos >= szLen)
      pos = 0;
    else
      pos = szLen - pos;
  }
  else if(pos > szLen)
    pos = szLen;
  return pos;
}

/*!-------------------------------------------------------------------------------------------------
  Creates a new smart string from a slice of another smart string.

  Position may be negative to mean backwards from end of string. If parameters are invalid, e.g.
  calculated position of right < left or left > strlen string, will return an empty slice. String
  length must be < INT_MAX.

  Examples with string "Hello World?":

  left  | right   | new slice
  ----: | ------: | ----------
      0 | INT_MAX | "Hello World?"
      0 | -1      | "Hello World"
      0 | 2       | "He"
      2 | 5       | "llo"
     -6 | -1      | "World"
      6 | 1       | "World"

  @example FlyStrSmartSlice()

  ```c
  #include "FlyStr.h"

  // get a slice for the left and right and combine them
  flyStrSmart_t * get_start_end(const flyStrSmart_t *pStr, int width)
  {
    flyStrSmart_t  *pLeft  = FlyStrSmartSlice(pStr, 0, width);;
    flyStrSmart_t  *pRight = FlyStrSmartSlice(pStr, -1 * width, FLYSTRSMART_RIGHT);

    pLeft  = FlyStrSmartCombine(pLeft, pRight);
    FlyStrSmartFree(pRight);

    return pLeft;
  }
  ```

  @param    pStr    ptr to smart string
  @param    left    left position of slice
  @param    right   left position of slice
  @return ptr to new smart string or NULL if out of memory
*///------------------------------------------------------------------------------------------------
flyStrSmart_t * FlyStrSmartSlice(flyStrSmart_t *pStr, int left, int right)
{
  flyStrSmart_t  *pStr2 = NULL;
  size_t          szLen;
  const char     *sz;
  static const char szEmpty[] = "";

  // printf("FlyStrSmartSlice(left %d, right %d, sz %s)\n", left, right, pStr->sz);
  szLen = (int)strlen(pStr->sz);
  if(szLen < INT_MAX)
  {
    // determine slice positions
    left = SmartSlicePos(left, (int)szLen);
    right =  SmartSlicePos(right, (int)szLen);
    // printf("  posLeft %d, posRight %d\n", left, right);
    if(left < right)
    {
      sz = &pStr->sz[left];
    }
    else
    {
      sz = szEmpty;
      right = left;
    }

    // make the new string
    pStr2 = FlyStrSmartNewEx(sz, (right - left) + 1);
  }

  return pStr2;
}

/*!-------------------------------------------------------------------------------------------------
  Like snprintf(), but allocates memory if needed to fit.

  @param    pStr    ptr to smart string
  @param    szFmt   format, e.g. "hello %3d %*s"
  @return strlen of formatted string, or 0 if failed (no memory).
*///------------------------------------------------------------------------------------------------
int FlyStrSmartSprintf(flyStrSmart_t *pStr, const char *szFmt, ...)
{
  va_list     arglist;
  int         len = 0;
  size_t      newSize;
  unsigned    i;
  #define MAX_ITER  31    // prevent infinite loops

  // print, and clear to end of line
  *pStr->sz = '\0';
  for(i = 0; i < MAX_ITER; ++i)
  {
    // does string fit?
    va_start(arglist, szFmt);
    len = vsnprintf(pStr->sz, pStr->size, szFmt, arglist);
    va_end(arglist);

    if(len < pStr->size)
      break;

    // string too long to fit in an int
    if(pStr->size >= INT_MAX)
    {
      len = 0;
      break;
    }

    // didn't fit, try larger string buffer size
    if(pStr->size > INT_MAX / 2)
      newSize = INT_MAX;
    else
      newSize = pStr->size * 2;
    if(!FlyStrSmartResize(pStr, newSize))
    {
      len = 0;  // NOMEM
      break;
    }
  }

  if(len)
    len = strlen(pStr->sz);
  return len;
}

/*!-------------------------------------------------------------------------------------------------
  Ensure smart string is at least "size" bytes. Never shrinks memory held by smart string.

  Grows memory if needed. If memory couldn't be allocated, smart string is left unchanged and return
  is FALSE.

  Like C realloc(), will free the string if size is 0, But why do that? Use FlyStrSmartFree()
  instead.

  @param    pStr    ptr to smart string
  @param    size    new size, may be 0 to free string
  @return ptr to reallocated string, or NULL if it couldn't be allocated
*///------------------------------------------------------------------------------------------------
bool_t FlyStrSmartResize(flyStrSmart_t *pStr, size_t size)
{
  bool_t  fWorked = TRUE;
  char   *pszNew = NULL;

  // printf("FlyStrSmartResize(pStr->size %zu, size %zu)\n", pStr->size, size);

  // never allow 0 length smart strings, as they must be always be '\0' terminated
  if(size == 0)
    ++size;

  // no string allocated, do so now, should never happen
  if(pStr->sz == NULL)
  {
    pStr->sz = FlyAlloc(size);
    if(pStr->sz == NULL)
    {
      fWorked = FALSE;
      pStr->size = 0;
    }
    else
    {
      memset(pStr->sz, 0, size);
      pStr->size = size;
    }
  }

  // only grow if needed
  else if(size > pStr->size)
  {
    pszNew = FlyRealloc(pStr->sz, size);
    if(pszNew == NULL)
      fWorked = FALSE;
    else
    {
      if(size > pStr->size)
        memset(&pszNew[pStr->size], 0, size - pStr->size);
      pStr->sz = pszNew;
      pStr->size = size;
    }
  }

  return fWorked;
}
