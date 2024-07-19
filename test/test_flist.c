/**************************************************************************************************
  test_flist.c  test cases for FlyFileList.c
  Copyright 2024 Drew Gislason
  License: MIT <https://mit-license.org>
**************************************************************************************************/
#include "FlyTest.h"
#include "FlyFile.h"
#include "FlyStr.h"

/*-------------------------------------------------------------------------------------------------
  Test file info with various things
-------------------------------------------------------------------------------------------------*/
void TcFileListNew(void)
{
  typedef struct
  {
    const char *szPath;
    unsigned    expLen;     // for FlyListNew() 0 means hList is NULL
  } tcFileListNew_t;

  static const tcFileListNew_t aTestsNew[] =
  {
    { "tdata_filelist", 1 },
    { "tdata_filelist/*", 4 },
    { "tdata_filelist/file*", 2 },
    { "nothere", 0 },
  };

  static const tcFileListNew_t aTestsNewEx[] =
  {
    { "tdata_filelist", 4 },
    { "tdata_filelist/", 4 },
    { "tdata_filelist/*", 4 },
    { "tdata_filelist/file*", 2 },
    { "nothere", 0 },
  };

  void       *hList;
  unsigned    len;
  unsigned    i;
  bool_t      fFailed;

  FlyTestBegin();

  for(i = 0; i < NumElements(aTestsNew); ++i)
  {
    fFailed = FALSE;
    len = 0;
    hList = FlyFileListNew(aTestsNew[i].szPath);
    if(hList)
      len = FlyFileListLen(hList);

    if((aTestsNew[i].expLen == 0) && (hList != NULL))
      fFailed = TRUE;
    else if((aTestsNew[i].expLen != 0) && (hList == NULL))
      fFailed = TRUE;
    else if(aTestsNew[i].expLen != len)
      fFailed = TRUE;
    if(fFailed)
    {
      FlyTestPrintf("%u: %s, hList %p, len %u, expLen %u\n", i, aTestsNew[i].szPath, hList, len, aTestsNew[i].expLen);
      if(FlyTestVerbose())
      {
        for(i = 0; i < len; ++i)
          FlyTestPrintf("  %s\n", FlyFileListGetName(hList, i));
      }
      FlyTestFailed();
    }
  }

  for(i = 0; i < NumElements(aTestsNewEx); ++i)
  {
    fFailed = FALSE;
    len = 0;
    hList = FlyFileListNewEx(aTestsNewEx[i].szPath);
    if(hList)
      len = FlyFileListLen(hList);

    if((aTestsNewEx[i].expLen == 0) && (hList != NULL))
      fFailed = TRUE;
    else if((aTestsNewEx[i].expLen != 0) && (hList == NULL))
      fFailed = TRUE;
    else if(aTestsNewEx[i].expLen != len)
      fFailed = TRUE;
    if(fFailed)
    {
      FlyTestPrintf("NewEx %u: %s, hList %p, len %u, expLen %u\n", i, aTestsNewEx[i].szPath, hList, len, aTestsNewEx[i].expLen);
      if(FlyTestVerbose())
      {
        for(i = 0; i < len; ++i)
          FlyTestPrintf("  %s\n", FlyFileListGetName(hList, i));
      }
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test TcFileListNewExts()
-------------------------------------------------------------------------------------------------*/
void TcFileListNewExts(void)
{
  typedef struct
  {
    const char  *szFolder;
    const char  *szExts;
    unsigned     maxDepth;
    unsigned     len;
    const char  **aszExpList;
  } tcFileListNewExts_t;

  const char *aszExpList0[] = { "Makefile", "foo.c", "test.c", "test.c++", "test.cc", "test.dots.verylongfileextension" };
  const char *aszExpList1[] = { "foo.c", "test.c", "test.c++", "test.cc" };
  const char *aszExpList2[] = { "foo.c", "test.c", "test.c++", "test.cc",
                                "subdir1/bar.c", "subdir1/test.c",
                                "subdir2/test.c", "subdir2/test.c++" };
  const char *aszExpList3[] = { "foo.c", "test.c",
                                "subdir1/bar.c", "subdir1/test.c",
                                "subdir1/subdir1a/baz.c", "subdir1/subdir1a/test.c",
                                "subdir1/subdir1a/subsubdir1/qux.c", "subdir1/subdir1a/subsubdir1/test.c",
                                "subdir2/test.c" };
  const tcFileListNewExts_t aTests[] =
  {
    { "tdata_filelistnewexts/",
      ".c.c++.cc.verylongfileextension.",
      0,
      NumElements(aszExpList0),
      aszExpList0
    },
    { "tdata_filelistnewexts/",
      ".c.c++.cc",
      0,
      NumElements(aszExpList1),
      aszExpList1
    },
    { "tdata_filelistnewexts/",
      ".c.c++.cc",
      1,
      NumElements(aszExpList2),
      aszExpList2
    },
    { "tdata_filelistnewexts/",
      ".c",
      5,
      NumElements(aszExpList3),
      aszExpList3
    }
  };

  void       *hList;
  bool_t      fFailed;
  unsigned    i, j;
  unsigned    len;
  char        szPath[256];

  FlyTestBegin();

  for(i = 0; i < NumElements(aTests); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("%u: %s, szExts %s, maxDepth %u\n", i, aTests[i].szFolder, aTests[i].szExts, aTests[i].maxDepth);
    fFailed = FALSE;
    hList = FlyFileListNewExts(aTests[i].szFolder, aTests[i].szExts, aTests[i].maxDepth);
    if(!hList)
      fFailed = TRUE;
    else if(FlyFileListLen(hList) != aTests[i].len)
      fFailed = TRUE;
    else
    {
      len = FlyFileListLen(hList);
      for(j = 0; j < len; ++j)
      {
        FlyStrZCpy(szPath, aTests[i].szFolder, sizeof(szPath));
        FlyStrZCat(szPath, aTests[i].aszExpList[j], sizeof(szPath));
        if(strcmp(FlyFileListGetName(hList, j), szPath) != 0)
        {
          fFailed = TRUE;
          break;
        }
      }
    }

    if(fFailed)
    {
      FlyTestPrintf("%i: hList %p, Got len %u, expected len %u\n", i, hList, FlyFileListLen(hList), aTests[i].len);
      FlyTestPrintf("Got List:\n");
      len = FlyFileListLen(hList);
      for(j = 0; j < len; ++j)
        FlyTestPrintf("%s\n", FlyFileListGetName(hList, j));
      FlyTestPrintf("Exp List:\n");
      for(j = 0; j < aTests[i].len; ++j)
        FlyTestPrintf("%s%s\n", aTests[i].szFolder, aTests[i].aszExpList[j]);
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
  const char          szName[] = "test_flist";
  const sTestCase_t   aTestCases[] =
  {
    { "TcFileListNew",  TcFileListNew },
    { "TcFileListNewExts",  TcFileListNewExts },
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
