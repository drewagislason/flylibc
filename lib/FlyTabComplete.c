/**************************************************************************************************
  FlyTabComplete.c - Allows tab completion from any list of strings
  Copyright 2024 Drew Gislason
  License: MIT <https://mit-license.org>
*///***********************************************************************************************
#include "FlyStr.h"
#include "FlyFile.h"
#include "FlyTabComplete.h"

/*!
  @defgroup   FlyTabComplete   Tab complete handling for making command-lines

  1. Iterate through lists of files/folders
  2. Can handle paths with tilde, e.g. "~/folder/"

  @example FlyTabComplete  Enter a partial path, then tab through all entries

  ```
  #include "FlyTabComplete.h"

  int main(int argc, char *argv[])
  {
    // TODO TabComplete example
  }
  ```
*/

#define FLY_TABCOMPLETE_SANCHK     51515

typedef struct
{
  unsigned    sanchk;             // sanity check
  void       *hFileList;          // list of matching files/folders
  unsigned    index;              // index into list
  bool_t      fReduceHome;        // convert from "/Users/drewg/" to "~/" in user szPath
  unsigned    maxSize;            // max size of szPath
  char       *szPath;             // ptr to path to copy to user string
} sFlyTabComplete_t;

/*!------------------------------------------------------------------------------------------------
  Creates TabComplete state machine

  @param    maxSize     maximum size of path string for this state machine
  @preturn handle to state machine
-------------------------------------------------------------------------------------------------*/
void * FlyTabCompleteNew(unsigned maxSize)
{
  sFlyTabComplete_t *pTabComplete;

  pTabComplete = FlyAlloc(sizeof(sFlyTabComplete_t));
  if(pTabComplete)
  {
    memset(pTabComplete, 0, sizeof(sFlyTabComplete_t));
    pTabComplete->sanchk  = FLY_TABCOMPLETE_SANCHK;
    pTabComplete->maxSize = maxSize;
    pTabComplete->szPath  = FlyAlloc(maxSize);
    if(!pTabComplete)
    {
      FlyTabCompleteFree(pTabComplete);
      pTabComplete = NULL;
    }
  }

  return pTabComplete;
}

/*!------------------------------------------------------------------------------------------------
  Returns TRUE if this is a TabComplete state machine

  @param    hTabComplete     handle to TabComplete state machine
  @return TRUE if this is a TabComplete state machine
-------------------------------------------------------------------------------------------------*/
bool_t FlyTabCompleteIsTabComplete(void *hTabComplete)
{
  sFlyTabComplete_t  *pTabComplete = hTabComplete;

  return (pTabComplete && (pTabComplete->sanchk == FLY_TABCOMPLETE_SANCHK)) ? TRUE : FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Expands szPath in-place if any files or folders match the final name in string. Doesn't expand
  if wildcards are in input string.

  Examples (assuming current and home folder contains folders "Desktop/", "Documents/",
  "Downloads/", and file "Downtime.txt".

  "~/Doc*" becomes "~/Documents/"
  "/Users/drewg/Doc" becomes "/Users/drewg/Documents/"
  "./Doc" becomes "./Documents/"
  "D" becomes "Desktop/", then "Documents/", then "Downloads/", "Downtime.txt"
  "Nothere" doesn't change. Remains "Nothere"

  @param    szPath          a complete or partial path
  @param    hTabComplete    handle to TabComplete state machine
  @return FALSE if no (more) matches in the file/folder list
-------------------------------------------------------------------------------------------------*/
bool_t FlyTabComplete(void *hTabComplete, char *szPath)
{
  sFlyTabComplete_t  *pTabComplete  = hTabComplete;
  bool_t              fFound        = FALSE;

  // not valid state machine
  if(FlyTabCompleteIsTabComplete(hTabComplete))
  {
    // if we are already in the state machine, get next item
    if((pTabComplete->hFileList != NULL) && strcmp(szPath, pTabComplete->szPath) == 0)
    {
      if(pTabComplete->index < FlyFileListLen(pTabComplete->hFileList))
        fFound = TRUE;
    }

    // potentially start a new state machine with this path
    else
    {
      // done with previous file list (about to make new one)
      if(pTabComplete->hFileList)
      {
        FlyFileListFree(pTabComplete->hFileList);
        pTabComplete->hFileList = NULL;
      }

      // expand home folder if needed (because glob() doesn't understand "~/*"
      FlyStrZNCpy(pTabComplete->szPath, szPath, sizeof(pTabComplete->szPath), pTabComplete->maxSize);
      pTabComplete->fReduceHome = FALSE;
      if(strncmp(pTabComplete->szPath, "~/", 2) == 0)
        pTabComplete->fReduceHome = TRUE;
      FlyFileHomeExpand(pTabComplete->szPath, pTabComplete->maxSize);

      // implicit wildcard, converts "Doc" to "Doc*", unless user already put in wildcard(s)
      if((strchr(pTabComplete->szPath, '*') == NULL) && (strchr(pTabComplete->szPath, '?') == NULL))
        strcat(pTabComplete->szPath, "*");

      pTabComplete->hFileList = FlyFileListNew(pTabComplete->szPath);
      if(FlyFileListLen(pTabComplete->hFileList) > 0)
      {
        pTabComplete->index = 0;
        fFound = TRUE;
      }
    }
  }

  if(fFound)
  {
    strcpy(pTabComplete->szPath, FlyFileListGetName(pTabComplete->hFileList, pTabComplete->index));
    if(pTabComplete->fReduceHome)
      FlyFileHomeReduce(pTabComplete->szPath);
    strcpy(szPath, pTabComplete->szPath);
    ++pTabComplete->index;
  }

  return fFound;
}

/*!------------------------------------------------------------------------------------------------
  Returns TRUE if successfully rewound the tab complete

  @param    maxSize     maximum size of path string for this state machine
  @preturn handle to state machine
-------------------------------------------------------------------------------------------------*/
bool_t FlyTabCompleteRewind(void *hTabComplete)
{
  sFlyTabComplete_t  *pTabComplete = hTabComplete;
  bool_t              fWorked = FALSE;

  if(FlyTabCompleteIsTabComplete(hTabComplete) && (pTabComplete->hFileList != NULL))
  {
    pTabComplete->index = 0;
    fWorked = TRUE;
  }

  return fWorked;
}

/*!------------------------------------------------------------------------------------------------
  Frees a TabComplete state machine

  @param    hTabComplete     handle to TabComplete state machine
  @return   none
-------------------------------------------------------------------------------------------------*/
void FlyTabCompleteFree(void *hTabComplete)
{
  sFlyTabComplete_t *pTabComplete = hTabComplete;

  if(FlyTabCompleteIsTabComplete(hTabComplete))
  {
    if(pTabComplete->hFileList)
    {
      FlyFileListFree(pTabComplete->hFileList);
      pTabComplete->hFileList = NULL;
    }
    if(pTabComplete->szPath)
      FlyFree(pTabComplete->szPath);    
    memset(pTabComplete, 0, sizeof(sFlyTabComplete_t));
    FlyFree(pTabComplete);
  }
}
