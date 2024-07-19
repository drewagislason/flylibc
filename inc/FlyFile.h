/**************************************************************************************************
  FlyFile.h  
  Copyright 2024 Drew Gislason
  license: MIT <https://mit-license.org>
**************************************************************************************************/
#ifndef FLY_FILE_H
#define FLY_FILE_H

#include <time.h>
#include "Fly.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

// used for navigating files and folders
typedef struct
{
  unsigned              sanchk;
  bool_t                fIsDir;
  bool_t                fIsModified;
  bool_t                fRdOnly;
  bool_t                fExists;
  time_t                modTime;
  char                  szFullPath[PATH_MAX];
} sFlyFileInfo_t;

typedef unsigned flyFileListOpts_t;
#define FLYFILELIST_OPTS_BEG      0x01  // start matches
#define FLYFILELIST_OPTS_END      0x02  // end matches
#define FLYFILELIST_OPTS_NOCASE   0x04  // case insensitive

#define FLYFILELIST_NOT_FOUND     UINT_MAX

typedef int (*pfnFlyFileSort_t)     (const void *pThis, const void *pThat);

typedef bool_t (*pfnFlyFileListRecurse_t)(const char *szPath, void *pData);

// FlyFile.c for dealing with files (see also getenv() and getcwd()
char         *FlyFileRead           (const char *szFilename);
uint8_t      *FlyFileReadBin        (const char *szFilename, long *pLen);
bool_t        FlyFileWrite          (const char *szFilename, const char *szContents);
bool_t        FlyFileWriteBin       (const char *szFilename, const uint8_t *szContents, long len);
bool_t        FlyFileWriteEx        (const char *szFilename, const char *szContents, bool_t fCrLf);
bool_t        FlyFileCopy           (const char *szOutFilename, const char *szInFilename);
bool_t        FlyFileFullPath       (char *szFullPath, const char *szPartialPath);
bool_t        FlyFileExists         (const char *szPath, bool_t *fFolder);
bool_t        FlyFileExistsFile     (const char *szPath);
bool_t        FlyFileExistsFolder   (const char *szPath);
bool_t        FlyFileIsSamePath     (const char *szPath1, const char *szPath2);
void          FlyFileInfoInit       (sFlyFileInfo_t *pInfo);
bool_t        FlyFileInfoGet        (sFlyFileInfo_t *pInfo, const char *szPath);
bool_t        FlyFileInfoGetEx      (sFlyFileInfo_t *pInfo, const char *szPath);
bool_t        FlyFileFindInPath     (char *szPath, unsigned size, const char *szBaseName, bool_t cwdFirst);
bool_t        FlyFileFindInFolder   (char *szPath, unsigned size, const char *szBaseName, const char *szBaseFolder);
bool_t        FlyFileHomeGet        (char *szPath, unsigned size);
unsigned      FlyFileHomeGetLen     (void);
bool_t        FlyFileHomeExpand     (char *szPath, unsigned size);
bool_t        FlyFileHomeReduce     (char *szPath);
bool_t        FlyFileGetCwd         (char *szPath, unsigned size);
bool_t        FlyFileChangeDir      (const char *szPath);
int           FlyFileMakeDir        (const char *szPath);

// FlyFilelIst.c: for iterating through a file/folder trees/lists
void         *FlyFileListNew        (const char *szWildPath);
void         *FlyFileListNewEx      (const char *szWildPath);
void         *FlyFileListNewExts    (const char *szFolder, const char *szExtList, unsigned maxDepth);
bool_t        FlyFileListRecurse    (const char *szWildPath, unsigned maxDepth, pfnFlyFileListRecurse_t pfnProcess, void *pData);
void         *FlyFileListFree       (void *hList);
const char   *FlyFileListGetName    (void *hList, unsigned i);
const char   *FlyFileListGetNameEx  (void *hList, unsigned i);
unsigned      FlyFileListGetBasePath(void *hList, char *szPath, unsigned size);
unsigned      FlyFileListFind       (void *hList, const char *sz, unsigned startIndex, flyFileListOpts_t opts);
bool_t        FlyFileListIsList     (void *hList);
unsigned      FlyFileListLen        (void *hList);
unsigned      FlyFileListLenEx      (void *hList);
void          FlyFileListPrint      (void *hList);
void          FlyFileListSort       (void *hList, pfnFlyFileSort_t pfnCmpStr);

#ifdef __cplusplus
  }
#endif

#endif // FLY_FILE_H
