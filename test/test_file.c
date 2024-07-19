/**************************************************************************************************
  test_file.c
  Copyright 2024 Drew Gislason
  License: MIT <https://mit-license.org>
**************************************************************************************************/
#include "FlyTest.h"
#include "FlyFile.h"
#include "FlyStr.h"

/*-------------------------------------------------------------------------------------------------
  Test file info with various things
-------------------------------------------------------------------------------------------------*/
void TcFileInfo(void)
{
  sFlyFileInfo_t  info;

  FlyTestBegin();

  FlyFileInfoInit(&info);
  if(*info.szFullPath != '\0' || info.fIsDir)
    FlyTestFailed();

  if(!FlyFileInfoGet(&info, "tdata"))
    FlyTestFailed();
  if(!info.fIsDir || FlyStrCharLast(info.szFullPath) != '/')
    FlyTestFailed();

  if(!FlyFileInfoGet(&info, "tdata/hello.txt"))
    FlyTestFailed();
  if(info.fIsDir || FlyStrCharLast(info.szFullPath) == '/')
    FlyTestFailed();

  if(FlyFileInfoGet(&info, "tdata/not_there"))
    FlyTestFailed();

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test exists functions
-------------------------------------------------------------------------------------------------*/
void TcFileExists(void)
{
  bool_t    fFolder;
  bool_t    fExists;

  FlyTestBegin();

  // verify folder exists
  fFolder = FALSE;
  fExists = FlyFileExists("tdata_filelist", &fFolder);
  if(!fExists || !fFolder)
    FlyTestFailed();

  // verify file exists
  fFolder = TRUE;
  fExists = FlyFileExists("test_file", &fFolder);
  if(!fExists || fFolder)
    FlyTestFailed();

  // verify for something that does not exist
  fExists = FlyFileExists("nothere", &fFolder);
  if(fExists)
    FlyTestFailed();

  // verify folder exists function
  fExists = FlyFileExistsFolder("../test/tdata_filelist/subdir1/");
  if(!fExists)
    FlyTestFailed();
  fExists = FlyFileExistsFolder("test_file.c");
  if(fExists)
    FlyTestFailed();

  // verify file exists function
  fExists = FlyFileExistsFile("../test/tdata_filelist/subdir1/subfile1.txt");
  if(!fExists)
    FlyTestFailed();
  fExists = FlyFileExistsFile(".");
  if(fExists)
    FlyTestFailed();

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyFileHomeGet(), FlyFileHomeGetLen(), FlyFileHomeExpand(), FlyFileHomeReduce()
-------------------------------------------------------------------------------------------------*/
void TcFileHome(void)
{
  static const char szTilde[] = "~/folder/";
  static const char szTilde2[] = "~";
  char              szPath[PATH_MAX];
  unsigned          homeLen;
  unsigned          size;

  FlyTestBegin();

  // FlyFileHomeGet() should return TRUE and have a non-zero length path
  memset(szPath, 'a', sizeof(szPath));
  *szPath = '\0';
  if(!FlyFileHomeGet(szPath, sizeof(szPath)) || (strlen(szPath) == 0))
    FlyTestFailed();

  // last character of home folder must be slash
  if(!isslash(FlyStrCharLast(szPath)))
  {
    FlyTestPrintf("szPath %s\n", szPath);
    FlyTestFailed();
  }

  // len of $HOME
  if(FlyFileHomeGetLen() == 0)
    FlyTestFailed();

  // should fail as size of buffer is 1 byte too small
  homeLen = strlen(szPath);
  if(FlyFileHomeGet(szPath, homeLen))
    FlyTestFailed();

  // should expand into ($HOME)/folder/
  memset(szPath, 'a', sizeof(szPath));
  strcpy(szPath, szTilde);
  if(!FlyFileHomeExpand(szPath, sizeof(szPath)))
  {
    FlyTestFailed();
  }
  else if((strlen(szPath) <= homeLen) || (strcmp(&szPath[homeLen], &szTilde[2]) != 0))
  {
    FlyTestPrintf("szPath %s\n", szPath);
    FlyTestFailed();
  }

  size = strlen(szPath) + 1;
  memset(&szPath[size], 'a', sizeof(szPath) - size);
  if(!FlyFileHomeReduce(szPath) || (strcmp(szPath, szTilde) != 0))
  {
    FlyTestPrintf("szPath %s\n", szPath);
    FlyTestFailed();    
  }

  // should expand into ($HOME)/folder/
  memset(szPath, 'a', sizeof(szPath));
  strcpy(szPath, szTilde2);
  if(!FlyFileHomeExpand(szPath, sizeof(szPath)))
  {
    FlyTestFailed();
  }
  else if((strlen(szPath) < homeLen) || (szPath[homeLen] != '\0'))
  {
    FlyTestPrintf("szPath %s\n", szPath);
    FlyTestFailed();
  }

  size = strlen(szPath) + 1;
  memset(&szPath[size], 'a', sizeof(szPath) - size);
  if(!FlyFileHomeReduce(szPath) || (szPath[0] != szTilde2[0] != 0))
  {
    FlyTestPrintf("szPath %s\n", szPath);
    FlyTestFailed();    
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test each function
-------------------------------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
  const char          szName[] = "test_file";
  const sTestCase_t   aTestCases[] =
  {
    { "TcFileInfo",     TcFileInfo, "M" },
    { "TcFileExists",   TcFileExists },
    { "TcFileHome",     TcFileHome },
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
