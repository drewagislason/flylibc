/**************************************************************************************************
  FlyFile.c - Generic file utilities for checking if a file or path exists.  
  Copyright (c) 2024 Drew Gislason
  license: <https://mit-license.org>
*///***********************************************************************************************
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <strings.h>
#include "FlyStr.h"
#include "FlyFile.h"

/*!
  @defgroup   FlyFile   Generic file utilities for checking if a file or path exists

  The FlyFile set of modules allows discovery, reading and writing files in a simplified manner
  using only functions built into the C standard library.

  1. Can determine if a path points to a file or folder
  2. Can determine if a file has changed since last checked
  3. Exapands paths to full path
  4. Supports '~' for $HOME folder
  5. Read a whole file into memory with one command
  6. Determine lengths of files
  7. Parse entire folder trees

  @example FlyFile Parse files in folder

  ```
  Include example here...
  ```
*/

#define FILEINFO_SANCHK           22922

/*!------------------------------------------------------------------------------------------------
  Read a text file into memory (does not support binary files).

  Caller must free the memory. If file couldn't be read, returns NULL.

  @seealso FlyFileReadBin().

  @param    szFilename    Filename to read. Can include full or partial path.
  @return   pointer file in memory, or NULL if failed. Caller must free file memory
*///-----------------------------------------------------------------------------------------------
char * FlyFileRead(const char *szFilename)
{
  FILE     *fp;
  char     *pszFile = NULL;
  long      len     = 0;

  fp = fopen(szFilename, "r");
  if(fp)
  {
    fseek(fp, 0L, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    pszFile = FlyAlloc(len + 1);
    if(pszFile)
    {
      len = fread(pszFile, 1, len, fp);
      pszFile[len] = '\0';
    }
    fclose(fp);
  }

  return pszFile;
}

/*!------------------------------------------------------------------------------------------------
  Read file into memory as binary

  @param    szFilename    Filename to read. Can include full or partial path.
  @param    pLen          Length of file
  @return   pointer file in memory, or NULL if failed. Caller must free file memory
*///-----------------------------------------------------------------------------------------------
uint8_t * FlyFileReadBin(const char *szFilename, long *pLen)
{
  FILE     *fp;
  uint8_t  *pFile = NULL;
  long      len     = 0;

  fp = fopen(szFilename, "rb");
  if(fp)
  {
    fseek(fp, 0L, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    pFile = FlyAlloc(len);
    if(pFile)
    {
      len = fread(pFile, 1, len, fp);
      if(pLen)
        *pLen = len;
    }
    fclose(fp);
  }

  return pFile;
}

/*!------------------------------------------------------------------------------------------------
  Write a text file from memory.

  @param    szFilename    Filename to wite. Can include full or partial path.
  @param    szContents    contents of file
  @return   pointer file in memory, or NULL if failed. Caller must free file memory
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileWrite(const char *szFilename, const char *szContents)
{
  FILE     *fp;
  long      len     = 0;
  bool_t    fWorked = TRUE;

  len = strlen(szContents);
  fp = fopen(szFilename, "w");
  if(!fp)
    fWorked = FALSE;
  else if(fwrite(szContents, 1, len, fp) != len)
    fWorked = FALSE;
  if(fp)
    fclose(fp);

  return fWorked;
}

/*!------------------------------------------------------------------------------------------------
  Write file with line ending of choice

  @param    szFilename    Filename to write. Can include full or partial path.
  @param    szContents    contents of file
  @param    fCrLf         line endings are CR/LF, otherwise LF only
  @return   pointer file in memory, or NULL if failed. Caller must free file memory
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileWriteEx(const char *szFilename, const char *szContents, bool_t fCrLf)
{
  FILE       *fp;
  const char *szEnding;
  long        len       = 0;
  long        lenEnding = 0;
  bool_t      fWorked = TRUE;

  fp = fopen(szFilename, "w");
  if(!fp)
    fWorked = FALSE;
  else
  {
    szEnding = fCrLf ? "\r\n" : "\n";
    lenEnding = strlen(szEnding);
    while(*szContents)
    {
      len = FlyStrLineLen(szContents);
      if(fwrite(szContents, 1, len, fp) != len)
      {
        fWorked = FALSE;
        break;
      }
      if(fwrite(szEnding, 1, lenEnding, fp) != lenEnding)
      {
        fWorked = FALSE;
        break;
      }
      szContents = FlyStrLineNext(szContents);
    }
  }
  if(fp)
    fclose(fp);

  return fWorked;
}

/*!------------------------------------------------------------------------------------------------
  Write a binary file from memory to disk/storage.

  @param    szFilename    Filename to write. Can include full or partial path.
  @param    pContents     contents of binary file
  @return   TRUE if worked, FALSE if failed
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileWriteBin(const char *szFilename, const uint8_t *pContents, long len)
{
  FILE     *fp;
  bool_t    fWorked = TRUE;

  fp = fopen(szFilename, "wb");
  if(!fp)
    fWorked = FALSE;
  else if(fwrite(pContents, 1, len, fp) != len)
    fWorked = FALSE;
  if(fp)
    fclose(fp);

  return fWorked;
}

/*!------------------------------------------------------------------------------------------------
  Copy a (smallish) file.

  This loads the full file into memory before writing it out, so not suitable for very large
  files.

  @param    szOutFilename   output file path, e.g. "../folder/fileout.png"
  @param    szInFilename    input file path, e.g. "filein.png"
  @return   pointer file in memory, or NULL if failed. Caller must free file memory
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileCopy(const char *szOutFilename, const char *szInFilename)
{
  uint8_t *pContents;
  long    len;
  bool_t  fWorked = FALSE;

  pContents = FlyFileReadBin(szInFilename, &len);
  if(pContents)
    fWorked = FlyFileWriteBin(szOutFilename, pContents, len);

  return fWorked;
}

/*!------------------------------------------------------------------------------------------------
  Essentially "which" under Linux/MacOS or "where" under Windows. Looks for the file szBaseName
  in the current directory or path.

  /Library/Frameworks/Python.framework/Versions/3.8/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:
  /sbin:/Applications/VMware Fusion.app/Contents/Public

  @param    szPath          destination path string
  @param    maxSize         sizeof(szPath) buffer
  @param    szBaseName      file to look for (could be exec name for example)
  @param    cwdFirst        Look at cwd first, before scanning path for file
  @return   TRUE and path in szPath if found, FALSE if not found.
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileFindInPath(char *szPath, unsigned maxSize, const char *szBaseName, bool_t cwdFirst)
{
  char     *psz;
  char     *pszThisPath;
  unsigned  len;
  unsigned  baseLen;
  bool_t    fWorked = FALSE;

  if(!szPath || !maxSize || !szBaseName)
    return FALSE;

  if(cwdFirst && FlyFileExistsFile(szBaseName))
  {
    if(strlen(szBaseName) < maxSize)
    {
      strcpy(szPath, szBaseName);
      fWorked = TRUE;
    }
  }
  else
  {
    pszThisPath = getenv("PATH");
    baseLen = strlen(szBaseName);
    while(pszThisPath && *pszThisPath)
    {
      psz = strchr(pszThisPath, ':');
      if(psz)
        len = psz - pszThisPath;
      else
        len = strlen(pszThisPath);
      if(len + 1 + baseLen < maxSize)
      {
        strncpy(szPath, pszThisPath, len);
        szPath[len] = '/';
        strcpy(&szPath[len+1], szBaseName);
        if(FlyFileExistsFile(szPath))
        {
          fWorked = TRUE;
          break;
        }
      }
      if(!psz)
        break;
      pszThisPath = psz + 1;   // skip ':' or ';' separator
    }
  }

  return fWorked;
}

/*!------------------------------------------------------------------------------------------------
  Searches for this file, going from szBaseFolder to root, then in user folder. If szBaseFolder is
  NULL, the uses cwd.

  For example, if szBaseFolder is "/Users/drewg/Git/ned/test", and basename is "foo" searches:
  
  "/Users/drewg/Git/ned/test/foo"
  "/Users/drewg/Git/ned/foo"
  "/Users/drewg/Git/foo"
  "/Users/drewg/foo"
  "/Users/foo"
  "/foo"

  @param    szPath          destination path string
  @param    maxSize         sizeof(szPath) buffer
  @param    szBaseName      name of file to look for
  @param    szBaseFolder    starting folder to search (will search parent folders also)
  @return   TRUE and path in szPath if found, FALSE if not found.
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileFindInFolder(char *szPath, unsigned maxSize, const char *szBaseName, const char *szBaseFolder)
{
  char  *psz;

  if(!szBaseName || !szPath || !maxSize)
    return FALSE;

  // find base path
  if(!szBaseFolder)
  {
    if(getcwd(szPath, maxSize) == NULL)
      return FALSE;
  }
  else
  {
    if(strlen(szBaseFolder) >= maxSize || !FlyFileFullPath(szPath, szBaseFolder))
      return FALSE;
  }

  // verify entire path with basename is OK in size
  if(strlen(szPath) + 1 + strlen(szBaseName) >= maxSize)
    return FALSE;

  psz = szPath + strlen(szPath);
  *psz = '/';
  while(psz)
  {
    // found the file?
    strcpy(psz+1, szBaseName);
    if(FlyFileExistsFile(szPath))
      return TRUE;

    // no, try parent folder
    --psz;
    if(psz <= szPath)
      break;
    psz = FlyMemRChr(szPath, '/', (psz - szPath));
  }

  // if not found, look in home folder
  psz = getenv("HOME");
  if(psz && (strlen(psz) + 1 + strlen(szBaseName) < maxSize))
  {
    strcpy(szPath, psz);
    psz = szPath + strlen(szPath);
    *psz = '/';
    strcpy(psz+1, szBaseName);
    if(FlyFileExistsFile(szPath))
      return TRUE;
  }

  return FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Get length of $HOME folder, e.g. "/Users/me/" is length 10. Always ends in slash.

  @return   length of home folder, or 0 if no $HOME folder
*///-----------------------------------------------------------------------------------------------
unsigned FlyFileHomeGetLen(void)
{
  char      *pszHome;
  unsigned   len = 0;

  pszHome = getenv("HOME");
  if(pszHome)
  {
    len = strlen(pszHome);
    if(!isslash(FlyStrCharLast(pszHome)))
      ++len;
  }

  return len;
}

/*!------------------------------------------------------------------------------------------------
  Get home folder in folder/ form, e.g. "/Users/me/"

  If home folder doesn't exist in the environment, szPath will be empty "".

  @param    szPath    pointer to dst string
  @param    size      size of szPath buffer
  @return   returns FALSE if buffer is too small (home folder was truncated)
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileHomeGet(char *szPath, unsigned size)
{
  char       *pszHome;
  unsigned    len;
  bool_t      fWorked = TRUE;

  // get home folder and copy it, up to maxSize-1
  if(size == 0)
    fWorked = FALSE;
  else
  {
    pszHome = getenv("HOME");
    if(!pszHome)
      *szPath = '\0';
    else
    {
      len = strlen(pszHome);
      if(len + 1 >= size)
      {
        fWorked = FALSE;
        *szPath = '\0';
      }
      else
      {
        strncpy(szPath, pszHome, len);
        if(!isslash(FlyStrCharLast(pszHome)))
        {
          szPath[len] = '/';
          ++len;
        }
        szPath[len] = '\0';
      }
    }
  }

  return fWorked;
}

/*!------------------------------------------------------------------------------------------------
  Expands home folder in place in path string, e.g. "~/file" becomes "/Users/me/file".

  Expands both "~" and "~/", but not "~file".

  Can fail if size is too small, no $HOME folder exists or szPath points to "~file".

  @param    szPath      pointer input string. Also output as expand happens in place.
  @param    maxSize     sizeof szPath string
  @return   TRUE if expanded. FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileHomeExpand(char *szPath, unsigned size)
{
  const char   *pszHome;
  unsigned      homeLen;
  unsigned      len;
  unsigned      needSlash = 0;
  bool_t        fExpanded = FALSE;

  if(*szPath == '~' && (isslash(szPath[1]) || (szPath[1] == '\0')))
  {
    pszHome = getenv("HOME");
    homeLen = strlen(pszHome);
    if(pszHome)
    {
      if(!isslash(FlyStrCharLast(pszHome)))
        needSlash = 1;
      if(szPath[1] == '\0')
        len = 0;
      else
        len = strlen(&szPath[2]) + 1;

      if(homeLen + len < size)
      {
        // expand, leaving \0' and end of szPath
        if(len == 0)
          szPath[homeLen + needSlash] = '\0';
        else
          memmove(&szPath[homeLen + needSlash], &szPath[2], len);
        memcpy(szPath, pszHome, homeLen);
        if(needSlash)
          szPath[homeLen] = '/';
        fExpanded = TRUE;
      }
    }
  }

  return fExpanded;
}

/*!------------------------------------------------------------------------------------------------
  If string starts with home folder, reduce string to use tilde. Examples:

  "/Users/drewg" becomes "~"
  "/Users/drewg/Work/X*" becomes "~/Work/X*"
  "myfile.txt" remains unchanged

  @param    szPath      pointer path string
  @return   TRUE if reduced
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileHomeReduce(char *szPath)
{
  const char   *pszHome;
  unsigned      homeLen;
  unsigned      len;
  bool_t        fReduced = FALSE;

  // get home folder and copy it, up to maxSize-1
  pszHome = getenv("HOME");
  if(pszHome)
    homeLen = strlen(pszHome);
  if(szPath && pszHome && (strncmp(szPath, pszHome, homeLen) == 0))
  {
    len = strlen(szPath);
    memmove(&szPath[1], &szPath[homeLen], (len - homeLen) + 1);
    szPath[0] = '~';
    fReduced = TRUE;
  }

  return fReduced;
}

/*!------------------------------------------------------------------------------------------------
  Gets the current working directory (folder) into szPath

  @param    szPath    string to receive cwd
  @param    maxSize   sizeof(szPath) string
  @return   none
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileGetCwd(char *szPath, unsigned size)
{
  bool_t  fWorked = TRUE;
  if(getcwd(szPath, size) == NULL)
    fWorked = FALSE;
  return fWorked;
}

/*!------------------------------------------------------------------------------------------------
  Changes to a new directory/folder, e.g. "..", "~/Work"

  @param    szPath    string to receive cwd
  @return   TRUE if worked, FALSE if failed
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileChangeDir(const char *szPath)
{
  bool_t  fWorked = TRUE;
  if(chdir(szPath) != 0)
    fWorked = FALSE;
  return fWorked;
}

/*!------------------------------------------------------------------------------------------------
  Make a directory with the "standard" flags.

  Use C standard library mkdir() if you want to fiddle with flags.

  @param    szPath    string to receive cwd
  @return   0 on success, -1 and errno if failed
*///-----------------------------------------------------------------------------------------------
int FlyFileMakeDir(const char *szPath)
{
  #define MKDIR_OPTS (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
  return mkdir(szPath, MKDIR_OPTS);
}

/*!------------------------------------------------------------------------------------------------
  Returns the full path from a partial name. It's OK if the file doesn't exists as long as the
  folder does.

  Warning: expects return parameter szPath to be PATH_MAX in size.

  Examples assume cwd is "/Users/me/cwd":

      szPartialPath     | szFullPath[out]
      ----------------- | -------------------------
      ~/folder/file.txt | /Users/me/folder/file.txt
      ~/folder          | /Users/me/folder/
      ~/folder/         | /Users/me/folder/
      nothere.txt       | /User/me/cwd/nothere.txt
      somefolder/       | /User/me/cwd/somefolder/
      /bad/path/        | 

  @param    szPath          ptr to PATH_MAX buffer to receive full path
  @param    szPartialPath   pointer to a partial path like "file.c" or "../file.h" or "~/file.c"
  @return   TRUE if worked, FALSE if bad path
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileFullPath(char *szPath, const char *szPartialPath)
{
  sFlyFileInfo_t  info;
  bool_t          fWorked = TRUE;

  // check if file/folder exists (e.g. ../file.c or /Users/me/file.txt"
  FlyFileInfoInit(&info);
  fWorked = FlyFileInfoGetEx(&info, szPartialPath);
  if(fWorked)
    strcpy(szPath, info.szFullPath);
  else
    *szPath = '\0';

  return fWorked;
}

/*!------------------------------------------------------------------------------------------------
  Initialize an info structure, which contains things like full path.

  pInfo can point to a local, static, global or allocated variable.

  @param    pInfo      pointer to preallocated pInfo structure.
  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyFileInfoInit(sFlyFileInfo_t *pInfo)
{
  memset(pInfo, 0, sizeof(sFlyFileInfo_t));
  pInfo->sanchk = FILEINFO_SANCHK;
}

/*!------------------------------------------------------------------------------------------------
  Get information about this file or folder. File or folder must exists.

  Does NOT expand $HOME folder "~/"

  When using this the first time, use FlyFileInfoInit() on the sFlyFileInfo_t structure

  If the file or folder does not exist, then returned contents of pInfo are undefined.

  To get the modified status (fIsModified) of a file, check the file periodically reusing the same
  sFlyFileInfo_t pInfo structure without reinitializing it. Specifically, it uses the internal
  info.modTime field.

  Returns in pInfo:

      fExists       whether file/folder exists (not just parent)
      szFullPath    full path (e.g. ~/Work becomes /Users/drewg/Work)
      fRdOnly       if file is read-only
      fIsDir        TRUE if this path is a folder, FALSE if file
      fIsModified   TRUE if modified since last call
      modTime       Last modified time

  Under the hood, this uses C functions access() and stat().

  See <https://www.man7.org/linux/man-pages/man2/access.2.html>  
  See <https://www.man7.org/linux/man-pages/man2/stat.2.html>

  @param    pInfo     pointer to preallocated pInfo structure, return value if TRUE
  @param    szPath    pointer to a file or dir path (relative or absolute)
  @return             TRUE if file/folder exists, FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileInfoGet(sFlyFileInfo_t *pInfo, const char *szPath)
{
  struct stat   sStat;
  bool_t        fExists = FALSE;
  int           err;
  const char   *psz;

  // if not initialized, do so now
  if(pInfo->sanchk != FILEINFO_SANCHK)
    FlyFileInfoInit(pInfo);

  // if it exists, make sure it's a file or dir (not a link)
  err = stat(szPath, &sStat);
  if(err == 0 && ((sStat.st_mode & S_IFDIR) || (sStat.st_mode & S_IFREG)) )
  {
    fExists = TRUE;
    pInfo->fExists = TRUE;

    // check if file or folder
    if(sStat.st_mode & S_IFREG)
    {
      pInfo->fIsDir = FALSE;
      if(access(szPath, W_OK) == 0)
        pInfo->fRdOnly = FALSE;
      else
        pInfo->fRdOnly = TRUE;
    }
    else
    {
      pInfo->fIsDir   = TRUE;
      pInfo->fRdOnly  = TRUE;
    }

    // check if modified
    if(pInfo->modTime == 0)
    {
      pInfo->fIsModified  = FALSE;
      pInfo->modTime      = sStat.st_mtime;
    }
    else if(sStat.st_mtime > pInfo->modTime)
    {
      pInfo->fIsModified  = TRUE;
      pInfo->modTime      = sStat.st_mtime;
    }
    else
    {
      pInfo->fIsModified = FALSE;
    }

    if(realpath(szPath, pInfo->szFullPath) == NULL)
      FlyStrZCpy(pInfo->szFullPath, szPath, sizeof(pInfo->szFullPath));
    if(pInfo->fIsDir)
    {
      psz = FlyStrLastSlash(pInfo->szFullPath);
      if(!psz || psz[1] != '\0')
        strcat(pInfo->szFullPath, "/");
    }
  }

  return fExists;
}

/*!------------------------------------------------------------------------------------------------
  Simliar to FlyFileGetInfo(), with the following additions:

  1. Expands $HOME folder (e.g ~/Documents becomes /Users/drewg/Documents)
  2. It's OK if the file/folder does not exist, as long as the parent folder does

  @param    pInfo   pointer to preallocated pInfo structure, return value
  @param    szPath  pointer to a file or dir path (relative or absolute)
  @return           TRUE if path is valid, FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileInfoGetEx(sFlyFileInfo_t *pInfo, const char *szPath)
{
  char        szPathEx[PATH_MAX];
  char       *szNameLast  = NULL;
  bool_t      fExists;
  bool_t      fIsDir      = FlyStrPathIsFolder(szPath);

  // if not initialized, do so now
  if(pInfo->sanchk != FILEINFO_SANCHK)
    FlyFileInfoInit(pInfo);

  // expand $HOME folder, e.g. "~" into "/Users/me/" or "~/path/" into "/Users/me/path/"
  FlyStrZCpy(szPathEx, szPath, sizeof(szPathEx));
  FlyFileHomeExpand(szPathEx, sizeof(szPathEx));

  // get copy of last filename/folder in case of expansion
  szNameLast = FlyStrAlloc(FlyStrPathNameLast(szPathEx, NULL));

  // if already exists, we can be done with FlyFileInfoGet
  fExists = FlyFileInfoGet(pInfo, szPathEx);

  // doesn't exist, does parent exist?
  if(!fExists && szNameLast)
  {
    fIsDir = FlyStrPathIsFolder(szPathEx);
    if(FlyStrPathParent(szPathEx))
    {
      fExists = FlyFileInfoGet(pInfo, szPathEx);
      if(fExists && pInfo->fIsDir)
      {
        pInfo->fIsDir      = fIsDir;
        pInfo->fRdOnly     = FALSE;
        pInfo->fIsModified = FALSE;
        pInfo->fExists     = FALSE;
        pInfo->modTime     = 0;
        FlyStrPathAppend(pInfo->szFullPath, szNameLast, sizeof(pInfo->szFullPath));
        fExists = TRUE;
      }
      else
        fExists = FALSE;
    }
  }

  if(szNameLast)
    FlyFree(szNameLast);

  return fExists;
}

/*!------------------------------------------------------------------------------------------------
  Return TRUE if the file or folder exists

  @param      szPath         pointer to path string
  @param      pfFolder      ptr to bool, TRUE if folder, FALSE if file
  @return     TRUE if exists
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileExists(const char *szPath, bool_t *pfFolder)
{
  bool_t          fExists = FALSE;
  sFlyFileInfo_t  sInfo;

  memset(&sInfo, 0, sizeof(sInfo));
  fExists = FlyFileInfoGet(&sInfo, szPath);
  if(pfFolder)
    *pfFolder = sInfo.fIsDir;

  return fExists;
}

/*!------------------------------------------------------------------------------------------------
  Return TRUE if file exists. Returns FALSE if path is to a folder or doesn't exist.

  @param      szPath         pointer to path string
  @return     TRUE if file exists, FALSE if file path does not exist or is a folder
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileExistsFile(const char *szPath)
{
  bool_t          fIsFolder;
  bool_t          fExists;

  fExists = FlyFileExists(szPath, &fIsFolder);
  if(fExists && fIsFolder)
    fExists = FALSE;

  return fExists;
}

/*!------------------------------------------------------------------------------------------------
  Return TRUE if folder exists. Must point to a folder/

  @param      szPath         pointer to path string
  @return     TRUE 
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileExistsFolder(const char *szPath)
{
  bool_t          fIsFolder;
  bool_t          fExists;

  fExists = FlyFileExists(szPath, &fIsFolder);
  if(fExists && !fIsFolder)
    fExists = FALSE;

  return fExists;
}

/*!------------------------------------------------------------------------------------------------
  Are these two paths the same? e.g. "../../folder" might be same as "~/folder"

  @param      szPath1   path to folder
  @param      szPath2   path to folder
  @return     none
*///-----------------------------------------------------------------------------------------------
bool_t FlyFileIsSamePath(const char *szPath1, const char *szPath2)
{
  sFlyFileInfo_t  info1;
  sFlyFileInfo_t  info2;
  bool_t          fIsSame = FALSE;

  memset(&info1, 0, sizeof(info1));
  memset(&info2, 0, sizeof(info2));
  if(FlyFileInfoGet(&info1, szPath1) && FlyFileInfoGet(&info2, szPath2) &&
      strcmp(info1.szFullPath, info2.szFullPath) == 0)
  {
    fIsSame = TRUE;
  }

  return fIsSame;
}
