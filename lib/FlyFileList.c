/**************************************************************************************************
  FlyFileList.c - print all entries in the file list
  Copyright (c) 2024 Drew Gislason
  license: <https://mit-license.org>
*///***********************************************************************************************
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <strings.h>
#include "FlyStr.h"
#include "FlyMem.h"
#include "FlyFile.h"

/*!
  @defgroup   FlyFileList   A API for building lists of files/folders

  @example FlyFileList Create a list files

  ```
  #include "FlyFile.h"
  ```
*/

#define FLY_FILELIST_SANCHK        21212

typedef struct
{
  unsigned              sanchk;
  unsigned              len;      // size of string array
  const char          **aszPath;  // array of string pointers
} sFlyFileList_t;

/*!------------------------------------------------------------------------------------------------
  Is this a pointer to an ::sFlyFileList_t structure?

  @param      hList         pointer to file list handle
  @return     none
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileListIsList(void *hList)
{
  sFlyFileList_t  *pFileList = hList;
  return (pFileList && (pFileList->sanchk == FLY_FILELIST_SANCHK)) ? TRUE : FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Same as FlyFileListNew() but also converts `folder/` into `folder\*`

  This allows a caller to just specify a folder and get it's contents, rather than just the single
  folder name from glob.

  @param  pszWildPath    a path or folder, e.g. "*.c", or a folder, e.g. "..", "~/" or "../folder/"
  @return Returns handle to file list or NULL if invalid path
*///-----------------------------------------------------------------------------------------------
void * FlyFileListNewEx(const char *pszWildPath)
{
  char     *szNewPath = NULL;
  void     *hList;
  size_t    size;

  // if no wildcard, adds '*' wildcard to pszWildPath, e.g. "../folder/" becomes "../folder/*"
  if((strpbrk(pszWildPath, "*?") == NULL) && FlyFileExistsFolder(pszWildPath))
  {
    size = strlen(pszWildPath) + 3;
    szNewPath = FlyAlloc(size);
    if(szNewPath)
    {
      strcpy(szNewPath, pszWildPath);
      FlyStrPathAppend(szNewPath, "*", size);
      pszWildPath = szNewPath;
    }
  }

  hList = FlyFileListNew(pszWildPath);

  if(szNewPath)
    FlyFree(szNewPath);

  return hList;
}

/*!------------------------------------------------------------------------------------------------
  Create a list of files and folders from an input wilcard path.

  * Uses glob under the hood
  * Does not recurse, this is one level only
  * All found folders have a slash at the end, files do not
  * Works with the home `~/` folder

  Note: in examples below, backslash is used before `*` so that compiler doesn't complain about
  nested comments.

  Example Inputs: "*", "~/git/project\*", "tcdata_*.txt", "..\*.c"

  Example Output of input "..\*":

      ../READ.ME
      ../docs/
      ../src/
      ../x.c

  @param  pszWildPath    a wildcard path or folder, e.g. "*.c", or a folder, e.g. ".." or "~"
  @return Returns handle to file list or NULL if invalid path
*///-----------------------------------------------------------------------------------------------
void * FlyFileListNew(const char *pszWildPath)
{
  glob_t            globbuf;
  sFlyFileList_t   *pFileList = NULL;
  char             *sz;
  char             *szHome;
  unsigned          len       = 0;
  unsigned          offset;
  unsigned          i;
  int               flags = GLOB_MARK;

  memset(&globbuf, 0, sizeof(globbuf));
  if(*pszWildPath == '~' && (isslash(pszWildPath[1]) || (pszWildPath[1] == '\0')))
    flags |= GLOB_TILDE;

  // get a list of files
  if(glob(pszWildPath, flags, NULL, &globbuf) == 0)
  {
    if(globbuf.gl_pathc)
    {
      // determine total size of all strings
      for(i=0; i<globbuf.gl_pathc; ++i)
      {
        len += strlen(globbuf.gl_pathv[i]) + 1;   // room for '\0'
      }

      // all 3 objects together in 1 malloc (structure, table of str ptrs, strings)
      pFileList = FlyAlloc(sizeof(sFlyFileList_t) + (globbuf.gl_pathc * sizeof(const char *)) + len);
      if(pFileList != NULL)
      {
        // fill in structure
        pFileList->sanchk   = FLY_FILELIST_SANCHK;
        pFileList->len      = (unsigned)globbuf.gl_pathc;

        // array of pointers
        pFileList->aszPath  = (void *)(pFileList + 1);

        // point sz to AFTER array of pointers, where we'll start adding strings
        sz = (char *)(&pFileList->aszPath[pFileList->len]);

        // fill in strings and ptrs to strings
        offset = 0;
        if(flags & GLOB_TILDE)
        {
          szHome = getenv("HOME");
          if(szHome)
            offset = strlen(szHome);
        }

        for(i=0; i<pFileList->len; ++i)
        {
          pFileList->aszPath[i] = sz;
          if(offset)
          {
            strcpy(sz, "~");
            strcat(sz, &globbuf.gl_pathv[i][offset]);
          }
          else
            strcpy(sz, globbuf.gl_pathv[i]);
          sz += strlen(sz) + 1;
        }
      }
    }
    globfree(&globbuf);
  }

  return pFileList;
}

/*-------------------------------------------------------------------------------------------------
  Does the extenion match something in the szExtList?

  @param  pszExt      an extension, e.g. ".c" or ".c++" or NULL, meaning no extension
  @param  szExtList   list of file extensions, separated by dots, e.g. ".c++.cpp.cxx.cc.C"
  @return Returns TRUE if matches
*///-----------------------------------------------------------------------------------------------
static bool_t FflMatchExt(const char *pszExt, const char *szExtList)
{
  const char *psz;
  const char *pszEnd;
  bool_t      fMatches = FALSE;
  unsigned    len;

  if(pszExt && *pszExt == '.')
  {
    psz = szExtList;
    while(*psz)
    {
      pszEnd = strchr(psz + 1, '.');
      if(!pszEnd)
        pszEnd = psz + strlen(psz);
      len = (unsigned)(pszEnd - psz);
      if(len && (strncmp(pszExt, psz, len) == 0) && pszExt[len] == '\0')
        fMatches = TRUE;
      psz = pszEnd;
    }
  }

  // no file extension, e.g. "Makefile"
  else if(pszExt == NULL || *pszExt == '\0')
  {
    if(FlyStrCharLast(szExtList) == '.')
      fMatches = TRUE;
  }

  return fMatches;
}

/*-------------------------------------------------------------------------------------------------
  Append the matching list of files to the existing list

  @param  pList     An existing list of 0 or more files
  @param  szFolder  A folder in a working PATH_MAX buffer, e.g. "folder/", "~/folder", "./"
  @param  szExts    List of file extensions, separated by dots, e.g. ".c++.cpp.cxx.cc.C"
  @return Pointer to fileList (probably changed since reallocated)
*///-----------------------------------------------------------------------------------------------
static sFlyFileList_t * FflAppend(sFlyFileList_t *pFileList, char *szFolder, const char *szExtList)
{
  void       *hList;
  const char *szPath;
  unsigned    i;
  size_t      size;
  unsigned    len;
  unsigned    nMatched;
  unsigned    n;

  // make a wildmask from folder, e.g. "folder/*"
  len = strlen(szFolder);
  FlyStrPathAppend(szFolder, "*", PATH_MAX);

  // look for files that match folder
  hList = FlyFileListNew(szFolder);
  if(hList)
  {
    // count the # of entries that match
    nMatched = 0;
    for(i = 0; i < FlyFileListLen(hList); ++i)
    {
      szPath = FlyFileListGetName(hList, i);
      if(!FlyStrPathIsFolder(szPath))
      {
        if(FflMatchExt(FlyStrPathExt(szPath), szExtList))
          ++nMatched;
      }
    }

    // reallocate memory to allow for more files in the list
    if(nMatched)
    {
      size = sizeof(*pFileList) + (sizeof(char *) * ((size_t)pFileList->len + nMatched));
      pFileList = FlyRealloc(pFileList, size);
      pFileList->aszPath  = (void *)(pFileList + 1);
    }

    // add the new files to list
    if(nMatched && pFileList)
    {
      n = 0;
      for(i = 0; i < FlyFileListLen(hList); ++i)
      {
        szPath = FlyFileListGetName(hList, i);
        if(!FlyStrPathIsFolder(szPath))
        {
          if(FflMatchExt(FlyStrPathExt(szPath), szExtList))
          {
            pFileList->aszPath[pFileList->len + n] = FlyStrAlloc(szPath);
            ++n;
          }
        }
      }
      pFileList->len += nMatched;
    }

    FlyFileListFree(hList);
  }

  // remove '*' that was added to folder path
  szFolder[len] = '\0';

  return pFileList;
}

/*-------------------------------------------------------------------------------------------------
  Append the matching list of files to the existing list

  @param  pFileList   An existing list of 0 or more files
  @param  szFolder    A file folder to check
  @param  maxDepth    depth to add 
  @return Returns handle to file list or NULL if invalid folder
*///-----------------------------------------------------------------------------------------------
sFlyFileList_t * FflRecurse(sFlyFileList_t *pFileList, char * szFolder, const char *szExtList, unsigned maxDepth)
{
  void       *hList;
  const char *szPath;
  unsigned    i;
  unsigned    len;

  // printf("FflRecurse(%p, %s, %s, %u)\n", pFileList, szFolder, szExtList, maxDepth); 

  // make a wildmask from folder, e.g. "folder/*"
  len = strlen(szFolder);
  FlyStrPathAppend(szFolder, "*", PATH_MAX);

  // look for files that match folder
  hList = FlyFileListNew(szFolder);
  if(hList)
  {
    // add all folders
    for(i = 0; i < FlyFileListLen(hList); ++i)
    {
      szFolder[len] = '\0';
      szPath = FlyFileListGetName(hList, i);
      if(FlyStrPathIsFolder(szPath))
      {
        strcpy(szFolder, szPath);
        pFileList = FflAppend(pFileList, szFolder, szExtList);
        if(!pFileList)
          break;
        if(maxDepth)
          pFileList = FflRecurse(pFileList, szFolder, szExtList, maxDepth - 1);
      }
    }

    FlyFileListFree(hList);
  }

  szFolder[len] = '\0';
  return pFileList;
}

/*!------------------------------------------------------------------------------------------------
  Create a list of files with the given file extension(s).

  Note: this finds files only. Folders are found indirectly, when recursing into the folders, but
  are not included in the list.

  For example, `FlyFileListNewExt("folder/", ".c++.cpp.cxx.cc.C", 2)` might return:

  ```
  folder/file.cpp
  folder/subfolder/file.cxx
  folder/subfolder/file2.C
  folder/subfolder/sub/file_deep.c++
  ```

  The folder must exist or this will fail (return NULL). Folder can be "" (empty string) or "." for
  current folder. If you wish the dot prepended, use "./".

  The file extensions are listed with a prepended dot. For example, to find files with ".c", ".h"
  and with no file extension, use ".c.h.".

  @param  szFolder  A folder to check for the given file extensions, e.g. "..", or "~/Folder/" 
  @param  szExts    List of file extensions, separated by dots, e.g. ".c++.cpp.cxx.cc.C"
  @param  maxDepth  0 means just the folder, 1 means go depth 1 into subfolders, etc...
  @return Returns handle to file list or NULL if invalid folder or parameters
*///-----------------------------------------------------------------------------------------------
void * FlyFileListNewExts(const char *szFolder, const char *szExtList, unsigned maxDepth)
{
  sFlyFileList_t *pFileList = NULL;
  char           *szPath;

  // cannot proceed without both folder and extension list
  if(!szFolder || !szExtList || *szExtList != '.')
    return NULL;

  pFileList = FlyAllocZ(sizeof(*pFileList));
  szPath = FlyAlloc(PATH_MAX);
  if(szPath == NULL || pFileList == NULL)
  {
    szPath = FlyFreeIf(szPath);
    pFileList = FlyFreeIf(pFileList);
  }
  else if(*szExtList)
  {
    // fill in structure
    pFileList->sanchk   = FLY_FILELIST_SANCHK;
    pFileList->len      = 0;
    pFileList->aszPath  = (void *)(pFileList + 1);

    // special case: if folder is ".", use empty folder "", but not for "./"
    FlyStrZCpy(szPath, szFolder, PATH_MAX);
    if(strcmp(szPath, ".") == 0)
      *szPath = '\0';
    else if(*szPath && !isslash(FlyStrCharLast(szPath)))
      FlyStrZCat(szPath, "/", PATH_MAX);

    // fill in the top level files/folders
    pFileList = FflAppend(pFileList, szPath, szExtList);

    // recurse if requested
    if(pFileList && maxDepth)
      pFileList = FflRecurse(pFileList, szPath, szExtList, maxDepth - 1);
  }

  return pFileList;
}

/*!------------------------------------------------------------------------------------------------
  Free a list of files that was made using FlyFileListNew().

  @param      hList          A list previously allocated with FlyFileListNew()
  @return     NULL
*///-----------------------------------------------------------------------------------------------
void * FlyFileListFree(void *hList)
{
  if(FlyFileListIsList(hList))
    FlyFree(hList);
  return NULL;
}

/*!------------------------------------------------------------------------------------------------
  Return length of file list (that is, # of items in file list)

  @param      List       A list previously allocated with FlyFileListNew()
  @return     length of file list (1-n)
*///-----------------------------------------------------------------------------------------------
unsigned FlyFileListLen(void *hList)
{
  sFlyFileList_t   *pFileList = hList;
  unsigned          len       = 0;

  if(FlyFileListIsList(pFileList))
    len = pFileList->len;

  return len;
}

/*!------------------------------------------------------------------------------------------------
  Returns the base path based on hList created with FlyFileListNew().

  From        | Returns
  ----------- | -------
  "*"         | ""
  "~/ *"      | "/Users/user/"
  "folder/ *" | "folder/"
  "../ *"     | "../"

  @param      List      A list previously allocated with FlyFileListNew()
  @param      szPath    a string to receive path (may be NULL to just get length of base path)
  @param      size      size of the string to receive path
  @return     length of base path, which might be 0
*///-----------------------------------------------------------------------------------------------
unsigned FlyFileListGetBasePath(void *hList, char *szPath, unsigned size)
{
  const char  *pszName;
  const char  *psz;
  unsigned     basePathLen = 0;

  if(FlyFileListIsList(hList) && FlyFileListLen(hList))
  {
    // find length of base path, copy it if szPath is not NULL
    pszName = FlyFileListGetName(hList, 0);
    if(pszName)
    {
      psz = FlyStrLastSlash(pszName);
      if(psz && psz[1] == '\0')
        psz = FlyStrPrevSlash(pszName, psz - 1);
      if(psz == NULL)
      {
        if(szPath)
          *szPath = 0;
      }
      else
      {
        basePathLen = (unsigned)(psz - pszName) + 1;
        if(szPath)
          FlyStrZNCpy(szPath, pszName, size, basePathLen);
      }
    }
  }

  return basePathLen;
}

/*!------------------------------------------------------------------------------------------------
  Get the name at the index (0-n). To check if it's a folder, use FlyStrPathIsFolder().

  @param      List      A list previously allocated with FlyFileListNew()
  @param      i         index (0-n) into list
  @return     ptr to filename, or NULL if at end of list
*///-----------------------------------------------------------------------------------------------
const char * FlyFileListGetName(void *hList, unsigned i)
{
  const char      *szName     = NULL;
  sFlyFileList_t  *pFileList  = hList;

  FlyAssertDbg(FlyFileListIsList(pFileList));

  if(i < FlyFileListLen(hList))
    szName = pFileList->aszPath[i];

  return szName;
}
/*!------------------------------------------------------------------------------------------------
  Find an item in the list

  @param      List        A list previously allocated with FlyFileListNew()
  @param      sz          string to find
  @param      startIndex  starting index (0-n)
  @param      opts        search options (e.g. FLYFILELIST_OPTS_BEG)
  @return     0-n index if found, FLYFILELIST_NOT_FOUND if not found
*///-----------------------------------------------------------------------------------------------
unsigned FlyFileListFind(void *hList, const char *sz, unsigned startIndex, flyFileListOpts_t opts)
{
  const char *szEntry;
  size_t      lenEntry;
  size_t      len = strlen(sz);
  unsigned    i;
  bool_t      fFound = FALSE;

  for(i = startIndex; i < FlyFileListLen(hList); ++i)
  {
    szEntry = FlyFileListGetName(hList, i);
    lenEntry = strlen(szEntry);

    // case insensitive
    if(opts & FLYFILELIST_OPTS_NOCASE)
    {
      if(opts & FLYFILELIST_OPTS_BEG)
      {
        if(strncasecmp(szEntry, sz, len) == 0)
        {
          fFound = TRUE;
          break;
        }
      }
      else if(opts & FLYFILELIST_OPTS_END)
      {
        if(lenEntry >= len && strcasecmp(&szEntry[lenEntry - len], sz) == 0)
        {
          fFound = TRUE;
          break;
        }
      }
      else
      {
        if(strcasecmp(szEntry, sz) == 0)
        {
          fFound = TRUE;
          break;
        }
      }
    }
    else
    {
      if(opts & FLYFILELIST_OPTS_BEG)
      {
        if(strncmp(szEntry, sz, len) == 0)
        {
          fFound = TRUE;
          break;
        }
      }
      else if(opts & FLYFILELIST_OPTS_END)
      {
        if(lenEntry >= len && strcmp(&szEntry[lenEntry - len], sz) == 0)
        {
          fFound = TRUE;
          break;
        }
      }
      else
      {
        if(strcmp(szEntry, sz) == 0)
        {
          fFound = TRUE;
          break;
        }
      }
    }
  }

  return fFound ? i : FLYFILELIST_NOT_FOUND;
}

/*!------------------------------------------------------------------------------------------------
  Return length assuming ".." is first entry, unless at /.

  @param      List      A list previously allocated with FlyFileListNew()
  @return     length of file list (1-n)
*///-----------------------------------------------------------------------------------------------
unsigned FlyFileListLenEx(void *hList)
{
  unsigned  len = FlyFileListLen(hList);
  if(len)
    ++len;
  return len;
}

/*!------------------------------------------------------------------------------------------------
  Get the name at the index, where index 0 is always ".."

  @param      List      A list previously allocated with FlyFileListNew()
  @param      i         index (0-n) into list
  @return     ptr to filename, or NULL if at end of list
*///-----------------------------------------------------------------------------------------------
const char * FlyFileListGetNameEx(void *hList, unsigned i)
{
  static const char szDotDot[] = "..";
  if(i == 0)
    return szDotDot;
  else
    return FlyFileListGetName(hList, i - 1);
}

/*!------------------------------------------------------------------------------------------------
  Print the flylist in ls (column) form.

  @param    hList    List created with FlyFileListNew()
  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyFileListPrint(void *hList)
{
  unsigned  i;
  unsigned  len;
  unsigned  col   = 0;

  if(FlyFileListIsList(hList))
  {
    for(i = 0; i < FlyFileListLen(hList); ++i)
    {
      len = strlen(FlyFileListGetName(hList, i));
      len = (len + 16) & (~0xf);
      col += len;
      printf("%-*s", len, FlyFileListGetName(hList, i));
      if(col >= (6 * 16))
      {
        printf("\n");
        col = 0;
      }
    }
    printf("\n");
  }
}

/*-------------------------------------------------------------------------------------------------
  Compare two strings. Returns -1 if this < that, 0 if same, 1 if this > that.

  @param  pThis   ptr to a string ptr
  @param  pThat   ptr to a string ptr
  @return Returns -1 if this < that, 0 if same, 1 if this > that.
*///-----------------------------------------------------------------------------------------------
static int FflCmpStr(const void *ppThis_, const void *ppThat_)
{
  const char * const *ppThis = ppThis_;
  const char * const *ppThat = ppThat_;
  return strcmp(*ppThis, *ppThat);
}

/*!------------------------------------------------------------------------------------------------
  Sort the list of files in the list (in case OS doesn't).

  The default order is the order the OS provides the files/folders. This allows the list to be
  sorted in a different way. Simply provide a function

  @param    hList     List created with FlyFileListNew() or FlyFileListNewExts()
  @param    pfnSort   Sort function or NULL to use default, strcmp()
  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyFileListSort(void *hList, pfnFlyFileSort_t pfnCmpStr)
{
  sFlyFileList_t *pList = hList;
  if(FlyFileListIsList(hList))
    qsort(pList->aszPath, pList->len, sizeof(char *), pfnCmpStr ? pfnCmpStr : FflCmpStr);
}

/*!------------------------------------------------------------------------------------------------
  Recursively process files/folders to a maximum depth.

  The szPath is flexible. If there is a wildcard mask, it will be used in all subfolders. If no
  wildcard, then all files/folders will be processed. Examples of szPath:

      .
      ../folder/
      *.c
      ~/folder/Foo*.c
      a?cd.e?

  The caller supplied pfnProcess function can abort the processing by returning FALSE. The pData
  parameter is data that is passed through from the caller: perhaps to build a linked list of
  files that match, or count the files that match. The paramter pfnProcess has the following
  prototype. 

  ```c
  typedef bool_t (*pfnFlyFileListRecurse_t)(const char *szPath, void *pData);
  ```

  @example  FlyFileListRecurse() Usage

  ```c
  #include "FlyFile.h"

  bool_t process(const char *szPath, void *pData)
  {
    unsigned  *pCount = pData;

    printf("got %s\n", szPath);
    (*pCount)++;
    return TRUE;
  }

  int main(int argc, char *argv[])
  {
    unsigned  count = 0;
    unsigned  maxdepth;
    int       i;

    if(argc < 3)
      printf("filelist maxdepth [path...]\n");
    else
    {
      maxdepth = atoi(argv[1]);
      for(i = 2; i < argc; ++i)
        FlyFileListRecurse(argv[i], maxdepth, process, &count);
      printf("found %u file(s) and folderr(s)\n", count);
    }

    return 0;
  }
  ```

  @param    szPath      a path to a folder or a file, possibly with wildcard mask
  @param    maxDepth    maximum depths to recurse, Use 0 for given folder only.
  @param    pfnProcess  function that will process the file/folder name
  @param    pData       persistent data that is passed to the process function
  @return   TRUE if path is valid, FALSE if not (or invalid permissions) or was aborted.
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileListRecurse(const char *szPath, unsigned maxDepth, pfnFlyFileListRecurse_t pfnProcess, void *pData)
{
  void       *hList;
  char       *szThisPath;
  const char *szFolder;
  const char *szWildMask  = NULL;
  bool_t      fIsFolder   = FALSE;
  bool_t      fWorked     = TRUE;
  unsigned    size;
  unsigned    len;
  unsigned    i;

  // process files/folders first, based on input path (which may have wildcard like path/*.c)
  hList = FlyFileListNewEx(szPath);
  if(hList)
  {
    len = FlyFileListLen(hList);

    for(i = 0; i < len; ++i)
    {
      if(!(*pfnProcess)(FlyFileListGetName(hList, i), pData))
      {
        fWorked = FALSE;
        break;
      }
    }
  }

  // reecurse into subfolders if not already deep enough
  if(hList && maxDepth)
  {
    // e.g. "../folder/*.c" points to "*.c"
    if(strpbrk(FlyStrPathNameOnly(szPath), "*?"))
      szWildMask = FlyStrPathNameOnly(szPath);
    else
      fIsFolder = FlyFileExistsFolder(szPath);

    // we need to look for path/* so we can find all subfolders
    if(szWildMask && strcmp(szWildMask, "*") != 0)
    {
      // we used to be looking for something like *.c, now look for *
      FlyFileListFree(hList);
      size = strlen(szPath) + 4;
      szThisPath = FlyAlloc(size);
      if(!szThisPath)
        fWorked = FALSE;
      else
      {
        strcpy(szThisPath, szPath);
        FlyStrPathOnly(szThisPath);
        FlyStrPathAppend(szThisPath, "*", size);
        hList = FlyFileListNewEx(szThisPath);
        if(hList)
          len = FlyFileListLen(hList);
        FlyFree(szThisPath);
      }
    }

    for(i = 0; hList && fWorked && i < len; ++i)
    {
      szFolder = FlyFileListGetName(hList, i);
      if(FlyStrPathIsFolder(szFolder))
      {
        size = strlen(szPath) + strlen(szFolder) + 4;
        szThisPath = FlyAlloc(size);
        if(!szThisPath)
          fWorked = FALSE;
        else
        {
          strcpy(szThisPath, szPath);
          if(!fIsFolder)
            FlyStrPathOnly(szThisPath);
          FlyStrPathAppend(szThisPath, FlyStrPathNameLast(szFolder, NULL), size);
          FlyStrPathAppend(szThisPath, szWildMask ? szWildMask : "*", size);
          fWorked = FlyFileListRecurse(szThisPath, maxDepth - 1, pfnProcess, pData);
          FlyFree(szThisPath);
        }
      }
    }
  }

  if(hList)
    FlyFileListFree(hList);

  return fWorked;
}
