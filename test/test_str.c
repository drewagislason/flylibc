/**************************************************************************************************
  test_str.c - Test cases for FlyStr.c
  Copyright 2024 Drew Gislason
  License: MIT <https://mit-license.org>
**************************************************************************************************/
#include "FlyTest.h"
#include "FlyFile.h"
#include "FlyStr.h"

/*-------------------------------------------------------------------------------------------------
  Test case functions: FlyStrIsCase(), FlyStrToCase()
-------------------------------------------------------------------------------------------------*/
void TcStrCase(void)
{
  const char   *asz[]   = { "lowercase", "UPPERCASE", "camelCase", "MixedCase", "snake_case", "CONSTANT_CASE" };
  flyStrCase_t  aCase[] = { IS_LOWER_CASE, IS_UPPER_CASE, IS_CAMEL_CASE, IS_MIXED_CASE, IS_SNAKE_CASE, IS_CONSTANT_CASE };
  const char   *aszCase1[] = { "testone1", "TESTONE1" };
  flyStrCase_t  aCase1[]   = { IS_LOWER_CASE, IS_UPPER_CASE };
  const char   *aszCase2[] = { "testTwoOk", "TestTwoOk", "test_two_ok",  "TEST_TWO_OK",};
  flyStrCase_t  aCase2[]   = { IS_CAMEL_CASE, IS_MIXED_CASE, IS_SNAKE_CASE, IS_CONSTANT_CASE };
  unsigned      n;
  unsigned      next;
  unsigned      i;
  unsigned      len;
  char          szDst[16];

  FlyTestBegin();

  // verioy case detection
  for(i = 0; i < NumElements(asz); ++i)
  {
    if(FlyStrIsCase(asz[i]) != aCase[i])
    {
      FlyTestPrintf("%u: got case %u, expected case %u\n", i, FlyStrIsCase(asz[i]), aCase[i]);
      FlyTestFailed();
    }
  }

  // convert between lower and UPPPER and back
  n = NumElements(aszCase1);
  for(i = 0; i <= n; ++i)
  {
    next = (i + 1) % n;
    *szDst = '\0';
    len = FlyStrToCase(szDst, aszCase1[i % n], sizeof(szDst), aCase1[next]);
    if(FlyTestVerbose())
      FlyTestPrintf("i = %u, next = %u, szDst %s\n", i, next, szDst);
    if(strcmp(szDst, aszCase1[next]) != 0 || len != strlen(szDst))
      FlyTestFailed();
  }

  // convert between chamelCase, MixedCase, snake_case, CONSTANT_CASE
  n = NumElements(aszCase2);
  for(i = 0; i <= n; ++i)
  {
    next = (i + 1) % n;
    *szDst = '\0';
    len = FlyStrToCase(szDst, aszCase2[i % n], sizeof(szDst), aCase2[next]);
    if(FlyTestVerbose())
      FlyTestPrintf("i = %u, next = %u, from %s to %s\n", i, next, aszCase2[i % n], szDst);
    if(strcmp(szDst, aszCase2[next]) != 0 || len != strlen(szDst))
      FlyTestFailed();
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test Path functions:

  #define FlyStrDumpLineSize(cols) ((sizeof(long)*2) + 8 + ((cols)*4))
  void      FlyStrDump      (const void *pData, size_t len);
  void      FlyStrDumpEx    (const void *pData, size_t len, unsigned cols, long addr);
  unsigned  FlyStrDumpLine  (char *szDst, const uint8_t *pData, unsigned len, unsigned cols, long addr);
-------------------------------------------------------------------------------------------------*/
void TcStrDump(void)
{
  // 0000b530  10 d6 40 f9 00 02 1f d6  25 73 3a 25 75 3a 25 75  |.?@?...?%s:%u:%u|
  // 0000b540  3a 20 25 73 0a 00 4f 62  6a 20 66 6c 79 44 6f 63  |: %s..Obj flyDoc|
  // 0000b550  45 78 61 6d 70 6c 65 5f  74 3a 20 25 70 0a 00 73  |Example_t: %p..s|  
  uint8_t aData[] = 
  {
    0x10,0xd6,0x40,0xf9,0x00,0x02,0x1f,0xd6,0x25,0x73,0x3a,0x25,0x75,0x3a,0x25,0x75,
    0x3a,0x20,0x25,0x73,0x0a,0x00,0x4f,0x62,0x6a,0x20,0x66,0x6c,0x79,0x44,0x6f,0x63,
    0x45,0x78,0x61,0x6d,0x70,0x6c,0x65,0x5f,0x74,0x3a,0x20,0x25,0x70,0x0a,0x00,0x73
  };
  char szLineExpected1[] =
    "0000b530  10 d6 40 f9 00 02 1f d6  25 73 3a 25 75 3a 25 75  |..@.....%s:%u:%u|";
  char szLineExpected2[] =
    "00000099  10 d6 40 f9 00                                    |..@..           |";
  char szLineExpected3[] = 
    "00000000  10 d6 40 f9 00 02 1f d6 25 73 3a 25  75 3a 25 75 3a 20 25 73 0a 00 4f 62  |..@.....%s:%u:%u: %s..Ob|";
#if 0
  char szExpected[] =
    "0000b530  10 d6 40 f9 00 02 1f d6  25 73 3a 25 75 3a 25 75  |..@.....%s:%u:%u|\n"
    "0000b540  3a 20 25 73 0a 00 4f 62  6a 20 66 6c 79 44 6f 63  |: %s..Obj flyDoc|\n"
    "0000b550  45 78 61 6d 70 6c 65 5f  74 3a 20 25 70 0a 00 73  |Example_t: %p..s|\n";
  char szExpected2[] =
    "0000b530  10 d6 40 f9 00 02 1f d6  25 73 3a 25 75 3a 25 75  |..@.....%s:%u:%u|\n"
    "0000b540  3a 20 25 73 0a 00 4f 62  6a 20 66 6c 79 44 6f 63  |: %s..Obj flyDoc|\n"
    "0000b550  45 78 61 6d 70 6c 65                              |Example         |\n";
#endif
  char szLine[FlyStrDumpLineSize(FLYSTR_DUMP_COLS)];
  char szLineLong[FlyStrDumpLineSize(24) + 4];
  unsigned  len;
  // unsigned  line;
  // unsigned  i;

  FlyTestBegin();

  // unsigned FlyStrDumpLine(char *szLine, void *pData, unsigned len, unsigned cols, long addr)
  len = FlyStrDumpLine(szLine, aData, 16, 16, 0xb530);
  if(len != strlen(szLineExpected1) || strcmp(szLine, szLineExpected1) != 0)
  {
    FlyTestPrintf("\ngot len %u `%s`\n", len, szLine);
    FlyTestPrintf("exp len %u `%s`\n", (unsigned)strlen(szLineExpected1), szLineExpected1);
    FlyTestFailed();
  }

  len = FlyStrDumpLine(szLine, aData, 5, 16, 0x99);
  if(len != strlen(szLineExpected2) || strcmp(szLine, szLineExpected2) != 0)
  {
    FlyTestPrintf("\ngot len %u `%s`\n", len, szLine);
    FlyTestPrintf("exp len %u `%s`\n", (unsigned)strlen(szLineExpected2), szLineExpected2);
    FlyTestFailed();
  }

  memset(szLineLong, 'A', sizeof(szLineLong));
  len = FlyStrDumpLine(szLineLong, aData, 24, 24, 0);
  if(len != strlen(szLineExpected3) || strcmp(szLineLong, szLineExpected3) != 0)
  {
    FlyTestPrintf("\ngot len %u `%s`\n", len, szLineLong);
    FlyTestPrintf("exp len %u `%s`\n", (unsigned)strlen(szLineExpected3), szLineExpected3);
    FlyTestFailed();
  }
  // check for buffer overrun
  if(szLineLong[sizeof(szLineLong) - 4] != 'A')
    FlyTestFailed();

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test Path functions:

  bool_t            FlyStrPathAppend    (char *szPath, const char *szName, size_t size);  
  const char       *FlyStrPathExt       (const char *szPath);  
  const char       *FlyStrPathHome      (void);  
  bool_t            FlyStrPathHomeExpand(char *szPath, size_t size);  
  bool_t            FlyStrPathIsFolder  (const char *szPath);  
  bool_t            FlyStrPathParent    (char *szPath);  
  const char       *FlyStrPathOnlyName  (const char *szPath);  
  const char       *FlyStrPathOnlyFolder(const char *szPath);  
-------------------------------------------------------------------------------------------------*/
void TcStrPath(void)
{
  typedef struct
  {
    const char *szPath;
    const char *szExp;
  } tcPathExt_t;
  const  char  *psz;
  char          szPath[PATH_MAX];
  char          szPath2[PATH_MAX];
  char          szShort[8];
  unsigned      i;

  FlyTestBegin();

  // test FlyStrPathHome, FlyStrPathHomeExpand
  psz = FlyStrPathHome();
  if(!psz || strlen(psz) < sizeof(szShort)) // /Users/me
    FlyTestFailed();

  // verify expanding ~/ is same as $HOME
  strcpy(szPath, "~/Work/file.c");
  if(!FlyStrPathHomeExpand(szPath, sizeof(szPath)))
    FlyTestFailed();
  if(strncmp(szPath, psz, strlen(psz)) != 0)
    FlyTestFailed();

  // verify FlyStrPathHomeExpand() failing
  memset(szPath, 'a', PATH_MAX);
  strncpy(szPath, "~/", 2);
  szPath[PATH_MAX - 1] = '\0';
  if(FlyStrPathHomeExpand(szPath, sizeof(szPath)))
    FlyTestFailed();

  // test FlyStrPathAppend()
  strcpy(szPath, "/Users/me/Work/");
  strcpy(szPath2, "/Users/me/Work");
  if(!FlyStrPathAppend(szPath, "file.c", sizeof(szPath)))
    FlyTestFailed();
  if(!FlyStrPathAppend(szPath2, "file.c", sizeof(szPath2)))
    FlyTestFailed();
  if(strcmp(szPath, szPath2) != 0)
    FlyTestFailed();

  // test FlyStrPathExt()
  {
    const tcPathExt_t szPathExtTests[] =
    {
      { "hello.py",          ".py" },
      { "/Users/me/file.c",  ".c" },
      { "~/../main.rs",      ".rs" },
      { "file.what.hpp",     ".hpp" },
      { ".hidden",           "" },
      { ".hidden.txt",       ".txt" },
      { ".c++",              "" },
      { "x.c++",             ".c++" },
      { "Makefile",          "" },
      { "weird .  filename", ".  filename" },
      { ".",                 NULL },
      { "..",                NULL },
      { "...",               "." },
      { "../some/folder/",   NULL },
    };
    for(i = 0; i < NumElements(szPathExtTests); ++i)
    {
      psz = FlyStrPathExt(szPathExtTests[i].szPath);
      if(psz == NULL)
      {
        if(szPathExtTests[i].szExp != NULL)
        {
          FlyTestPrintf("%u: exp %s, got NULL\n", i, FlyStrNullOk(szPathExtTests[i].szExp));
          FlyTestFailed();
        }
      }
      else
      {
        if(szPathExtTests[i].szExp == NULL)
        {
          FlyTestPrintf("%u: exp NULL, got %s\n", i, FlyStrNullOk(psz));
          FlyTestFailed();
        }
        else if(strcmp(psz, szPathExtTests[i].szExp) != 0)
        {
          FlyTestPrintf("%u: exp %s, got %s\n", i, FlyStrNullOk(szPathExtTests[i].szExp), FlyStrNullOk(psz));
          FlyTestFailed();
        }
      }
    }
  }

  // test FlyStrPathOnlyName()
  {
    const char *szInPaths[]  = { "~/Work/myfile.c", "C:\\work\things\\hello.py", "myfile.c", "/dir/only/", ".", ".." };
    const char *szOutPaths[] = { "myfile.c", "hello.py", "myfile.c", "", "", "" };

    for(i = 0; i < NumElements(szInPaths); ++i)
    {
      psz = FlyStrPathNameOnly(szInPaths[i]);
      if(psz == NULL)
      {
        FlyTestFailed();
      }
      else if(strcmp(psz, szOutPaths[i]) != 0)
        FlyTestFailed();
    }
  }

  {
    const char *szInPaths[]  = { "~/Work/myfile.c", "C:\\work\things\\hello.py", "myfile.c", "/dir/only/", "", ".." };
    const char *szOutPaths[] = { "~/Work/", "C:\\work\things\\", "./", "/dir/only/", "./", "../" };
    int         len;

    if(NumElements(szInPaths) != NumElements(szInPaths))
      FlyTestFailed();

    for(i = 0; i < NumElements(szInPaths); ++i)
    {
      len = 0;
      psz = FlyStrPathOnlyLen(szInPaths[i], &len);
      if(psz == NULL)
      {
        FlyTestFailed();
      }
      else if((len != strlen(szOutPaths[i])) || (strncmp(psz, szOutPaths[i], len) != 0))
      {
        FlyTestPrintf("len = %d, exp %d, got `%.*s`, exp `%s`\n", len, (int)strlen(szOutPaths[i]), len, psz, szOutPaths[i]);
        FlyTestFailed();
      }
    }    
  }

  // test FlyStrPathIsFolder
  {
    const char *szInPaths[]  = { "~/Work/", ".",  "..", "file.ext" };
    bool_t      fResults[]   = { TRUE,      TRUE, TRUE, FALSE };
    bool_t    fIsFolder;

    for(i = 0; i < NumElements(szInPaths); ++i)
    {
      fIsFolder = FlyStrPathIsFolder(szInPaths[i]);
      if(fIsFolder != fResults[i])
        FlyTestFailed();
    }
  }


  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test TcStrPathRelative() and FlyStrPathIsRelative()
-------------------------------------------------------------------------------------------------*/
void TcStrPathRelative(void)
{
  typedef struct
  {
    const char *szPath;
    bool_t      fIsRelative;
  } tcIsRelative_t;

  typedef struct
  {
    const char  *szBase;
    const char  *szPath;
    const char  *szExpPath;
  } tcRelative_t;

  static const tcIsRelative_t aIsRelTests[] =
  {
    { "../file",           TRUE  },
    { "folder/",           TRUE  },
    { "/Users/me/folder/", FALSE },
    { "~/file",            FALSE },
  };

  static const tcRelative_t aTests[] = 
  {
    { "/Users/me/",                "/Users/me/",               "" },
    { "/",                         "/Users/me/file.c",         "Users/me/file.c" },
    { "/Users/me/",                "/Users/me/Documents/",     "Documents/" },
    { "/Users/me/",                "/Users/",                  "../" },
    { "/Users/me/",                "/Users/me/foo/bar/",       "foo/bar/" },
    { "/Users/me/foo.txt",         "/Users/me/file.c",         "file.c" },
    { "/Users/me/folder/",         "/Users/me/Documents/",     "../Documents/" },
    { "/Users/me/folder/",         "/Users/me/bin/sub/file.c", "../bin/sub/file.c" },
    { "/Users/me/foo/bar/",        "/Users/me/bin/sub/file.c", "../../bin/sub/file.c" },
    { "/Users/me/foo/bar/baz.txt", "/Users/me/bin/sub/file.c", "../../bin/sub/file.c" },
    { "C:\\Work\\dir name\\",      "C:\\Docs\\sub dir\\a b.c", "..\\..\\Docs\\sub dir\\a b.c" },
    { "C:\\",                      "C:\\Docs\\sub dir\\a b.c", "Docs\\sub dir\\a b.c" },
  };

  unsigned  i;
  unsigned  len;
  bool_t    fIs;
  bool_t    fFailed;
  char      szDst[256];

  FlyTestBegin();

  // test FlyStrPathIsRelative()
  for(i = 0; i < NumElements(aIsRelTests); ++i)
  {
    fIs = FlyStrPathIsRelative(aIsRelTests[i].szPath);
    if(fIs != aIsRelTests[i].fIsRelative)
    {
      FlyTestPrintf("%u: %s, exp fIs %u, got %u\n", i, aIsRelTests[i].szPath, aIsRelTests[i].fIsRelative, fIs);
      FlyTestFailed();
    }
  }

  // test FlyStrPathRelative()
  for(i = 0; i < NumElements(aTests); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("%u: szBase %s, szPath %s, szExpPath %s\n", i, aTests[i].szBase, aTests[i].szPath, aTests[i].szExpPath);
    strcpy(szDst, "bad");
    fFailed = FALSE;
    len = FlyStrPathRelative(NULL, sizeof(szDst), aTests[i].szBase, aTests[i].szPath);
    if(len != strlen(aTests[i].szExpPath))
      fFailed = TRUE;

    len = FlyStrPathRelative(szDst, sizeof(szDst), aTests[i].szBase, aTests[i].szPath);
    if(len != strlen(aTests[i].szExpPath) || strcmp(szDst, aTests[i].szExpPath) != 0)
      fFailed = TRUE;
    if(fFailed)
    {
      FlyTestPrintf("%u: exp '%s', len %u, got '%s', len %u\n", i,
          aTests[i].szExpPath, strlen(aTests[i].szExpPath), szDst, len);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyStrPathParent()
-------------------------------------------------------------------------------------------------*/
void TcStrPathParent(void)
{
  typedef struct
  {
    const char *szPath;
    unsigned    len;
    const char *szExp;
  } tcStrPathParent_t;

  static const tcStrPathParent_t aTests[] =
  { 
    { "/Users/me/work/",  10, "/Users/me/" },
    { "/Users/me/file.c", 10, "/Users/me/" },
    { "./folder/sub/",    9,  "./folder/" },
    { "../folder/",       3,  "../" },
    { "file.c",           2,  "./" },
    { "folder/",          2,  "./" },
    { "folder/.",         2,  "./" },
    { "",                 3,  "../" },
    { ".",                3,  "../" },
    { "./",               3,  "../" },
    { "..",               6,  "../../" },
    { "../",              6,  "../../" },
    { "../..",            9,  "../../../" },
    { "../../",           9,  "../../../" },
    { "~/Work/",          2,  "~/" },
    { "~/",               7,  "/Users/" },
    { "~/Folder/..",      9,  "~/Folder/" },
    { "/file.c",          1,  "/" },
    { "/",                0,  "/" },
  };

  char      szPath[32];
  unsigned  i;
  unsigned  len;
  bool_t    fFailed = TRUE;

  FlyTestBegin();

  if(FlyTestVerbose())
    FlyTestPrintf("\n");
  for(i = 0; i < NumElements(aTests); ++i)
  {
    // verboseness
    if(FlyTestVerbose())
      FlyTestPrintf("%u: szPath %s, len %u, exp %s", i, aTests[i].szPath, aTests[i].len, aTests[i].szExp);

    // test
    strcpy(szPath, aTests[i].szPath);
    fFailed = FALSE;
    len = FlyStrPathParent(szPath, sizeof(szPath));
    if(len != aTests[i].len || (strcmp(szPath, aTests[i].szExp) != 0))
      fFailed = TRUE;
    if(fFailed)
    {
      if(FlyTestVerbose())
        FlyTestPrintf(" ... failed\n");
      FlyTestPrintf("%u: got len %u, exp len %u\n", i, len, aTests[i].len);
      FlyTestPrintf("got: %s\n", szPath);
      FlyTestPrintf("exp: %s\n", aTests[i].szExp);
      FlyTestFailed();
    }
    if(FlyTestVerbose())
      FlyTestPrintf(" ... ok\n");
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test CName functions

  size_t FlyStrFnProtoLen(const char *szLine, const char **ppszCName)
  size_t FlyStrCNameLen(const char *sz)
-------------------------------------------------------------------------------------------------*/
void TcStrCName(void)
{
  const char *aszProto[] =
  {
    "unsigned area (unsigned h, unsigned w)\n"
    "{\n"
    "  return h * w;\n"
    "}\n",

    "def PyArea(h=5, w=4):\n"
    "    \"\"\"!\n"
    "        Calculate the area of a rectangle\n"
    "        @param  h   height\n"
    "        @param  w   width\n"
    "        @return area of rectangle\n"
    "    \"\"\"\n"
    "    return h * w\n",

    "function jsArea(h, w) {\n"
    "  return h * w; \n"
    "}\n",

    "void Car::Print(void)\n"
    "{\n"
    "  cout << this->brand << \" \" << this->model << \" \" << this->year;\n"
    "  if(IsCarNewish(this->year))\n"
    "    cout << \" new(ish)\";\n"
    "  cout << \"\n\";\n"
    "}\n",

    "fn rust_area(h: i32, w: i32) -> i32 {\n"
    "    h * w\n"
    "}\n",

    "func SwiftArea(h: Int, w:Int) -> Int {\n"
    "    return h * w\n"
    "}\n",

    "func GoArea(h int, h int) int {\n"
    "  area := h * w\n"
    "  return area\n",

    "unsigned area(\n"
    "  unsigned h,\n"
    "  unsigned w)\n"
    "{\n"
    "  return h * w;\n"
    "}\n",

    "   void fn(void) {",

    "   x=area(5, 4);",
  };

  const char *aszCName[] = 
  {
    // should pass
    "my_c_func(",   // 9
    "my_c_array[",  // 10
    "_myfn99",      // 7
    "a b",          // 1
    "uint32_t",     // 8
    "Cars::model(", // 11

    // should fail (be 0 len)
    "",             // 0
    "99hello",      // 0
    "::1",          // 0
    "a:b",          // 0
    "fe80::57",     // 0
    "%same",        // 0
    "~",            // 0
    " main(",       // 0
  };
  unsigned    aCNameLen[]      = { 9, 10, 7, 1, 8, 11, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned    aProtoLen[]      = { 38, 21, 21, 21, 35, 36, 29, 42, 13, 0 };
  unsigned    aProtoCNameLen[] = { 4, 6, 6, 10, 9, 9, 6, 4, 2, 0 };
  unsigned    i;
  unsigned    len;
  const char *szCName = NULL;

  FlyTestBegin();

  // test FlyStrCNameLen()
  if(NumElements(aszCName) != NumElements(aCNameLen))
    FlyTestFailed();
  for(i = 0; i < NumElements(aszCName); ++i)
  {
    len = FlyStrCNameLen(aszCName[i]);
    if(len != aCNameLen[i])
    {
      FlyTestPrintf("Failed FlyStrCNameLen, got %u, expected %u\n", len, aCNameLen[i]);
      FlyTestFailed();
    }
  }

  // test FlyStrFnProtoLen()
  if(NumElements(aszProto) != NumElements(aProtoLen))
    FlyTestFailed();
  for(i = 0; i < NumElements(aszProto); ++i)
  {
    len = FlyStrFnProtoLen(aszProto[i], NULL);
    if(len != aProtoLen[i])
    {
      FlyTestPrintf("Failed %u: got %u, expected %u, %.*s\n", i, len, aProtoLen[i],
        aProtoLen[i], aszProto[i]);
      FlyTestFailed();
    }
  }

  // test FlyStrFnProtoLen()
  if(NumElements(aszProto) != NumElements(aProtoLen))
    FlyTestFailed();
  for(i = 0; i < NumElements(aszProto); ++i)
  {
    len = FlyStrFnProtoLen(aszProto[i], &szCName);
    if(len != aProtoLen[i])
    {
      FlyTestPrintf("Failed FlyStrFnProtoLen, got %u, expected %u, %.*s\n", len, aProtoLen[i],
        aProtoLen[i], aszProto[i]);
      FlyTestFailed();
    }
    if(len && FlyStrCNameLen(szCName) != aProtoCNameLen[i])
    {
      FlyTestPrintf("Failed i=%u, FlyStrFnCNameLen, got %u, expected %u, %s\n",
          i, FlyStrCNameLen(szCName), aProtoCNameLen[i], aszProto[i]);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyStrFnProtoLen
-------------------------------------------------------------------------------------------------*/
void TcStrProtoLen(void)
{
  typedef struct
  {
    char *szPrototype;
    char *szCName;
  } expected_t;

  const expected_t aExpected[] =
  {
    { "unsigned area(unsigned h, unsigned w)", "area" }, // C
    { "unsigned CppArea(unsigned h, unsigned w)", "CppArea" }, // C++
    { "public Pet(string petName, string petAnimal, string petBreed, int petAge)", "Pet" }, // C#
    { "public void printme()", "printme" }, // C#
    { "func GoArea(h int, h int) int", "GoArea" }, // Go
    { "public Tree(String myClassification, String myType)", "Tree" }, // Java
    { "public void printme()", "printme" }, // Java
    { "public static void main(String[] args)", "main" }, // Java
    { "function jsArea(h, w)", "jsArea" }, // Javascript
    { "def PyArea(h=5, w=4):", "PyArea" }, // Python
    { "fn rust_area(h: i32, w: i32) -> i32", "rust_area" }, // Rust
    { "fn main()", "main" }, // Rust
    { "func SwiftArea(h: Int, w:Int) -> Int", "SwiftArea" }, // Swift
    { "func main()", "main" }, // Swift
    { "Car(string _brand, string _model, int _year)", "Car" }, // C++ 
    { "int  Year(void)", "Year" }, // C++
    { "void Car::Print(void)", "Car::Print" }, // C++
    { "def __init__(self, name, age):", "__init__" }, // Python
    { "def printme(self):", "printme" }, // Python
    { "def printage(self) :", "printage" }, // Python
    { "fn new<S: Into<String>>(name: S) -> Person", "new" }, // Rust
    { "fn print_me(&self)", "print_me" }, // Rust
    { "fn print_age(&self)", "print_age" }, // Rust
  };

  const char *aszNotFunctions[] = 
  {
    "",
    "   name = (void *)ptr; // {",
    "x = 5 + (2 * 3);\n",
    "void Print(void);\n",
  };
  const char szInit[] = "__init__";

  const char *szFile  = NULL;
  const char *szLine;
  const char *pszCName;
  unsigned    i;
  unsigned    protoLen;
  const char *szHdr;
  const char *szPrototype;
  flyStrHdr_t hdr;

  FlyTestBegin();

  szFile = FlyFileRead("tdata/TcStrProto.txt");
  if(!szFile)
    FlyTestFailed();

  // verify test data is at least the same count
  if(NumElements(aExpected) != FlyStrLineCount(szFile, NULL))
    FlyTestFailed();

  szLine = szFile;
  i = 0;
  while(*szLine)
  {
    pszCName = NULL;
    protoLen = FlyStrFnProtoLen(szLine, &pszCName);
    if(protoLen != strlen(aExpected[i].szPrototype))
    {
      FlyTestPrintf("%u: protoLen %u, expected %u\n", i,
          protoLen, (unsigned)strlen(aExpected[i].szPrototype));
      FlyTestPrintf("%.*s\n", FlyStrLineLen(szLine), szLine);
      FlyTestFailed();
    }

    if(strncmp(szLine, aExpected[i].szPrototype, protoLen) != 0)
    {
      FlyTestPrintf("%u: %.*s\n", i, FlyStrLineLen(szLine), szLine);
      FlyTestFailed();
    }

    if(!pszCName || strncmp(pszCName, aExpected[i].szCName, strlen(aExpected[i].szCName)) != 0)
    {
      FlyTestPrintf("%u: %.*s, expected %s\n", i, pszCName, strlen(aExpected[i].szCName),
        aExpected[i].szCName);
      FlyTestFailed();
    }

    szLine = FlyStrLineNext(szLine + protoLen);
    ++i;
  }

  if(szFile)
    free((void *)szFile);    

  for(i = 0; i < NumElements(aszNotFunctions); ++i)
  {
    if(FlyStrFnProtoLen(aszNotFunctions[i], &pszCName))
    {
      FlyTestPrintf("%u %s\n", i, aszNotFunctions[i]);
      FlyTestFailed();
    }
  }

  // read the Python file
  szFile = FlyFileRead("tdata/testclass.py");
  if(!szFile)
    FlyTestFailed();

  // find the 2nd Python Doc header
  szLine = szFile;
  for(i = 0; i < 2; ++i)
  {
    memset(&hdr, 0, sizeof(hdr));
    szHdr = FlyStrHdrFind(szLine, TRUE, &hdr);
    if(!szHdr || hdr.type != FLYSTRHDR_TYPE_PYDOC)
    {
      FlyTestPrintf("\n%i: failed to find PYDOC header, szHdr %p, hdr.type %u\n", i, szHdr, hdr.type);
      FlyTestFailed();
    }
    szLine = FlyStrRawHdrEnd(&hdr);
  }

  // verify Python class init prototype
  szLine = FlyStrLinePrev(szFile, FlyStrRawHdrLine(&hdr));
  szPrototype = FlyStrSkipWhite(szLine);
  pszCName = NULL;
  protoLen = FlyStrFnProtoLen(szPrototype, &pszCName);
  if(protoLen == 0 || pszCName == NULL)
    FlyTestFailed();
  if(strncmp(pszCName, szInit, strlen(szInit)) != 0)
    FlyTestFailed();

  FlyTestEnd();

  if(szFile)
    free((void *)szFile);
}

/*-------------------------------------------------------------------------------------------------
  Test FlyStrPathHasExt()
-------------------------------------------------------------------------------------------------*/
void TcStrPathHasExt(void)
{
  const char szExts[] = ".c.cppr.cpp.js";
  const char *aszFilenames[] =
  {
    "file.c",
    "file.cpp",
    "file.cppr",
    "file.js",
    "~/.hidden.js",
    "file.no",
    "~/.hidden",
    "Makefile",
    "~/Folder/",
  };
  const char *aszExpected[] = { ".c", ".cpp", ".cppr", ".js", ".js", NULL, NULL, NULL, NULL};
  const char *psz;
  unsigned  i;

  FlyTestBegin();

  if(NumElements(aszFilenames) != NumElements(aszExpected))
    FlyTestFailed();

  for(i = 0; i < NumElements(aszFilenames); ++i)
  {
    psz = FlyStrPathHasExt(aszFilenames[i], szExts);
    if(strcmp(FlyStrNullOk(psz), FlyStrNullOk(aszExpected[i])) != 0)
    {
      FlyTestPrintf("Got %s, expected %s\n", FlyStrNullOk(psz) != FlyStrNullOk(aszExpected[i]));
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test functions:

  const char       *FlyStrHdrFind       (const char *szLine, bool_t fIsDoc, flyStrHdr_t *pHdr);
  const char       *FlyStrHdrText       (flyStrHdr_t *pHdr, const char *szLine);
  bool_t            FlyStrHdrIsDoc      (const char *szHdr);
  const char       *FlyStrHdrEndContent (flyStrHdr_t *pHdr);
-------------------------------------------------------------------------------------------------*/
void TcStrHdrFind(void)
{
  typedef struct
  {
    flyStrHdrType_t type;
    bool_t          fIsDoc;
    size_t          indent;
    unsigned        row;
    unsigned        col;
    const char     *szText;
  } tcStrHdrFind_t;

  // Note: if adjusting, be sure to adjust file tdata/TcStrHdrFind.txt
  tcStrHdrFind_t aTests[] =
  {
    { .type = FLYSTRHDR_TYPE_C, .fIsDoc=FALSE, .indent = 2, .row=2, .col=5, .szText =
      "C header This is a file with headers of many languages\n"
      "Languages represented: C, C++, C#\n"
    },

    { .type = FLYSTRHDR_TYPE_C, .fIsDoc=FALSE, .indent = 6, .row=13, .col=16, .szText =
      "Indented C header\n"
      "Are other lines indented?\n"
    },

    { .type = FLYSTRHDR_TYPE_C, .fIsDoc=TRUE, .indent = 0, .row=18, .col=4, .szText =
      "No indent\n"
    },

    { .type = FLYSTRHDR_TYPE_C, .fIsDoc=FALSE, .indent = 4, .row=23, .col=13, .szText =
      "\n"
      "Comment with stars\n"
      "\n"
      "    Code block here  \n"
      "\n"
      "```\n"
      "Another code block\n"
      "```\n"
      "\n"
      "Does it look good?\n"
    },

    { .type = FLYSTRHDR_TYPE_HASH, .fIsDoc=FALSE, .indent = 1, .row=35, .col=4, .szText =
      "A non Doc header\n"
      "@param   \n"
      " \n"
    },

    { .type = FLYSTRHDR_TYPE_HASH, .fIsDoc=TRUE, .indent = 4, .row=44, .col=15, .szText =
      "Calculate surface area of a cone\n"
      "@ingroup maths\n"
      "@param  r   radius\n"
      "@param  l   lateral\n"
      "@return area of cone\n"
    },

    { .type = FLYSTRHDR_TYPE_C, .fIsDoc=TRUE, .indent = 2, .row=55, .col=10, .szText =
      "@class Car A class for listing automobiles\n"
      "\n"
      "Eventually target market will be car and dealerships and repair shops.\n"
    },

    { .type = FLYSTRHDR_TYPE_C, .fIsDoc=TRUE, .indent = 6, .row=66, .col=19, .szText =
      "Constructor for Car\n"
      "\n"
      "@param    _brand    brand of car (e.g. Honda)  \n"
      "@param    _model    model of car (e.g. Ridgeline)  \n"
      "@param    _year     year of car (e.g. 2022)  \n"
    },

    { .type = FLYSTRHDR_TYPE_RUST, .fIsDoc=TRUE, .indent = 4, .row=80, .col=12, .szText =
      "\n"
      "\n"
      "@class Person A person for a contact database\n"
      "\n"
      "  Eventually will contain other person attributes\n"
    },

    { .type = FLYSTRHDR_TYPE_RUST, .fIsDoc=TRUE, .indent = 10, .row=85, .col=18, .szText =
      "Create a Person object\n"
      "@param  name        string or reference to string\n"
      "@return Person\n"
    },

    { .type = FLYSTRHDR_TYPE_PYDOC, .fIsDoc=TRUE, .indent = 4, .row=94, .col=12, .szText =
      "@class Person A class that defines attributes of a person. Useful in many applications.\n"
      "\n"
      "Default attributes:\n"
      "    name    full name of person  \n"
      "    age     age of person\n"
    },

    { .type = FLYSTRHDR_TYPE_PYDOC, .fIsDoc=TRUE, .indent = 8, .row=103, .col=20, .szText =
      "Initialize a person. Requires name and age.\n"
      "@param  name    person's full name\n"
      "@param  age     person's age in years (0-n)\n"
    },

    { .type = FLYSTRHDR_TYPE_RUST, .fIsDoc=TRUE, .indent = 3, .row=111, .col=4, .szText =
      "\n"
      "\n"
      "\n"
    },

    { .type = FLYSTRHDR_TYPE_C, .fIsDoc=TRUE, .indent = 0, .row=117, .col=1, .szText =
      ""
    },
  };
  unsigned      i;
  size_t        len;
  const char   *szLine;
  // const char   *szTextLine;
  // const char   *szExpLine;
  // char         *szText;
  const char   *szFile;
  const char   *psz;
  const char   *pszPos;
  unsigned      row, col;
  flyStrHdr_t   hdr;
  // bool_t        fFailed;
  char          szDst[1024];

  FlyTestBegin();

  // load test file for headers
  szFile = FlyFileRead("tdata/TcStrHdrFind.txt");
  if(!szFile)
    FlyTestFailed();

  szLine = szFile;
  i = 0;
  while(*szLine)
  {
    // show what we're about to test
    if(FlyTestVerbose())
    {
      FlyTestPrintf("\n%i exp: %.*s, type %u, indent %u, fIsDoc %u\n", i,
        (int)FlyStrLineLen(aTests[i].szText), aTests[i].szText,
        aTests[i].type, aTests[i].indent, aTests[i].fIsDoc);
    }

    // find any and all headers
    psz = FlyStrHdrFind(szLine, FALSE, &hdr);
    if(!psz)
      break;
    else if(FlyTestVerbose())
    {
      FlyTestPrintf("szRawHdrLine %p, szStartLine%p, szEndLine %p, szEndHdr %p\n", hdr.szRawHdrLine,
        hdr.szStartLine, hdr.szEndLine, hdr.szRawHdrEnd);
      FlyTestPrintf("type %u, indent %u, fIsDoc %u\n", hdr.type, hdr.indent, hdr.fIsDoc);
      if(hdr.szRawHdrLine)
        FlyTestPrintf("szRawHdrLine: %.*s\n", (int)FlyStrLineLen(hdr.szRawHdrLine), hdr.szRawHdrLine);
      if(hdr.szStartLine)
        FlyTestPrintf("szStartLine: %.*s\n", (int)FlyStrLineLen(hdr.szStartLine), hdr.szStartLine);
      if(hdr.szEndLine)
        FlyTestPrintf("szEndLine: %.*s\n", (int)FlyStrLineLen(hdr.szEndLine), hdr.szEndLine);
      if(hdr.szRawHdrEnd)
        FlyTestPrintf("szRawHdrEnd: %.*s\n", (int)FlyStrLineLen(hdr.szRawHdrEnd), hdr.szRawHdrEnd);
    }

    if(i >= NumElements(aTests))
    {
      FlyTestPrintf("Found too many headers %u!\n", i);
      FlyTestFailed();
    }

    // check for failures
    if(FlyStrHdrType(&hdr) != aTests[i].type || FlyStrHdrIsDoc(&hdr) != aTests[i].fIsDoc ||
       FlyStrHdrIndent(&hdr) != aTests[i].indent || 
       FlyStrHdrContentEnd(&hdr) == NULL || FlyStrHdrContentStart(&hdr) == NULL || 
       FlyStrRawHdrEnd(&hdr) == NULL)
      {
        FlyTestPrintf("\n%i error: exp type %u, indent %zu fIsDoc %u\n", i, aTests[i].type, aTests[i].indent, aTests[i].fIsDoc);
        FlyTestPrintf("         got type %u, indent %zu, fIsDoc %u, szContentStart %p, szContentEnd %p, szRawHdrEnd %p\n",
        FlyStrHdrType(&hdr), FlyStrHdrIndent(&hdr), FlyStrHdrIsDoc(&hdr),
        FlyStrHdrContentStart(&hdr), FlyStrHdrContentEnd(&hdr), FlyStrRawHdrEnd(&hdr));
        FlyTestFailed();
      }

#if 0
    // get header line-by-line
    if(!fFailed)
    {
      szExpLine = aTests[i].szText;
      szTextLine = FlyStrHdrContentStart(&hdr);
      while(*szExpLine && szTextLine < FlyStrHdrContentEnd(&hdr))
      {
        if(FlyStrHdrText(&hdr, szTextLine) == NULL)
        {
          fFailed = TRUE;
          break;
        }
        else
        {
          len = FlyStrLineLen(szExpLine);
          len2 = FlyStrLineLen(FlyStrHdrText(&hdr, szTextLine))
          if(len != len2 || strncmp(FlyStrLineLen(szExpLine), FlyStrHdrText(&hdr, szTextLine), len) != 0)
          {
            fFailed = TRUE;
            break;
          }
        }

        // on to next line)
        szExpLine = FlyStrLineNext(szExpLine);
        szTextLine = FlyStrLineNext(szTextLine);
      }
      if(fFailed)
      {
        FlyTestPrintf("\n%i:\nexp %.*s, got %.*s\n", i, 
          (int)FlyStrLineLen(szExpLine), szExpLine, (int),
          (int)FlyStrLineLen(FlyStrHdrText(&hdr, szTextLine)), FlyStrHdrText(&hdr, szTextLine));
      }
    }
#endif

    FlyStrZFill(szDst, 'Q', sizeof(szDst), sizeof(szDst));
    len = FlyStrHdrCpy(szDst, &hdr, sizeof(szDst));
    if(len + 2 > sizeof(szDst) || szDst[len] != '\0' || szDst[len + 1] != 'Q')
    {
      FlyTestPrintf("\n%i: got len %zu, exp len %zu\n", i, len, strlen(aTests[i].szText));
      FlyTestDump(szDst, len);
      FlyTestPrintf("%s\n", szDst);
    }
    else if(strcmp(szDst, aTests[i].szText) != 0)
    {
      FlyTestPrintf("\n%i: --- exp len %zu---\n%s\n", i, strlen(aTests[i].szText), aTests[i].szText);
      FlyTestPrintf("--- got len %zu---\n%s\n--- end ---\n", len, szDst);
      FlyTestDump(szDst, len);
      FlyTestFailed();
    }

    // verify position matches
    psz = pszPos = szDst;
    while(*psz)
    {
      if(!FlyStrLineIsBlank(psz))
      {
        pszPos = FlyStrArgNext(FlyStrSkipWhite(psz));
        break;
      }
      psz = FlyStrLineNext(psz);
    }
    psz = FlyStrHdrCpyPos(szDst, &hdr, pszPos);
    row = FlyStrLinePos(szFile, psz, &col);
    if(row != aTests[i].row || col != aTests[i].col)
    {
      FlyTestPrintf("exp: row %u, col %u, got row %u, col %u\n", aTests[i].row, aTests[i].col, row, col);
      FlyTestFailed();
    }

    ++i;
    szLine = FlyStrRawHdrEnd(&hdr);
  }

  if(i != NumElements(aTests))
  {
    FlyTestPrintf("got %u hdrs, expected %u\n", i, NumElements(aTests));
    FlyTestFailed();
  }

  FlyTestEnd();

  if(szFile)
    free((void *)szFile);
}

/*-------------------------------------------------------------------------------------------------
  Test line functions
-------------------------------------------------------------------------------------------------*/
void TcStrLine(void)
{
  const char szFile[] = "One\nTwo\nThree\n";
  const char *psz;

  FlyTestBegin();

  psz = FlyStrLineNext(szFile);
  if(strncmp(psz, "Two", 3) != 0)
    FlyTestFailed();
  psz = FlyStrLineNext(psz);
  if(strncmp(psz, "Three", 5) != 0)
    FlyTestFailed();
  psz = FlyStrLineNext(psz);
  if(*psz != '\0')
    FlyTestFailed();
  psz = FlyStrLinePrev(szFile, psz);
  if(strncmp(psz, "Three", 5) != 0)
    FlyTestFailed();
  psz = FlyStrLinePrev(szFile, psz);
  if(strncmp(psz, "Two", 3) != 0)
    FlyTestFailed();
  psz = FlyStrLinePrev(szFile, psz);
  if(strncmp(psz, "One", 3) != 0)
    FlyTestFailed();
  psz = FlyStrLinePrev(szFile, psz);
  if(strncmp(psz, "One", 3) != 0)
    FlyTestFailed();

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test removing blank lines
-------------------------------------------------------------------------------------------------*/
void TcStrBlankRemove(void)
{
  const char *aszText[] =
  {
    "nothing to do",                  "nothing to do",
    "  \tspaces either side\t   ",    "spaces either side",
    " a\r\nb ",                       "a\r\nb",
    "    ",                           "",
  };
  const char *aszLines[] =
  {
    "nothing\n to\n do",              "nothing\n to\n do",
    "  \n \n\n\n",                    "",
    "\nLine 1\n\n",                   "Line 1\n",
    "\r\n\r\n Abcde \nLine2\n\n\n",  " Abcde \nLine2\n",
  };
  size_t      len;
  size_t      len2;
  char        sz[128];
  unsigned    i;

  FlyTestBegin();

  // test removing blanks from strings
  for(i = 0; i < NumElements(aszText); i += 2)
  {
    memset(sz, 0, sizeof(sz));
    FlyStrZCpy(sz, aszText[i], sizeof(sz));
    FlyStrBlankRemove(sz);
    len = strlen(sz);
    len2 = strlen(aszText[i + 1]);
    if(strcmp(sz, aszText[i + 1]) != 0)
    {
      FlyTestPrintf("\n%u: got len %zu, expected len %zu\n", i, len, len2);
      FlyTestPrintf("got\n");
      FlyTestDump(sz, len);
      FlyTestPrintf("expected\n");
      FlyTestDump(aszText[i + 1], len2);
      FlyTestFailed();
    }
  }

  // try removing blanks from strings
  for(i = 0; i < NumElements(aszLines); i += 2)
  {
    memset(sz, 0, sizeof(sz));
    FlyStrZCpy(sz, aszLines[i], sizeof(sz));
    FlyStrLineBlankRemove(sz);
    len = strlen(sz);
    len2 = strlen(aszLines[i + 1]);
    if(strcmp(sz, aszLines[i + 1]) != 0)
    {
      FlyTestPrintf("\n%u: got len %zu, expected len %zu\n", i, len, len2);
      FlyTestPrintf("got\n");
      FlyTestDump(sz, len);
      FlyTestPrintf("expected\n");
      FlyTestDump(aszLines[i + 1], len2);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  test the PathLang function.
-------------------------------------------------------------------------------------------------*/
void TcStrPathLang(void)
{
  typedef struct
  {
    const char *szPath;
    const char *szLang;
  } tcPathLang_t;

  static const tcPathLang_t aLangs[] =
  {
    { "file.c",     "c" },
    { "x.c++",       "C++" },
    { "x.cc",        "C++" },
    { "x.cpp",       "C++" },
    { "x.cxx",       "C++" },
    { "x.cs",        "C#" },
    { "x.go",        "Go" },
    { "x.java",      "Java" },
    { "x.json",      "JSON" },
    { "x.js",        "Javascript" },
    { "x.py",        "Python" },
    { "x.rb",        "Ruby" },
    { "x.rs",        "Rust" },
    { "x.swift",     "Swift" },
    { "x.ts",        "Typescript" },    
    { "~/folder/",   NULL },
    { "Makefile",    NULL },
  };
  unsigned  i;
  const char *szLang;

  FlyTestBegin();

  for(i = 0; i < NumElements(aLangs); ++i)
  {
    szLang = FlyStrPathLang(aLangs[i].szPath);
    if(FlyStrCmp(szLang, aLangs[i].szLang) != 0)
    {
      FlyTestPrintf("%u: got %s, expected %s\n", i, FlyStrNullOk(szLang), FlyStrNullOk(aLangs[i].szLang));
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyStrZ...() functions
-------------------------------------------------------------------------------------------------*/
void TcStrZ(void)
{
  typedef struct
  {
    const char  *szStr;
    const char  *szExp;
    const char  *szExpPre;
  } TcFlyStrZ_t;
  const TcFlyStrZ_t aList[] =
  {
    { "Hello", "Hello", "Yo!Hell" },
    { "A longer string", "A longe", "Yo!A lo" },
    { "", "", "Yo!" },
    { "C", "C", "Yo!C" },
  };
  const char  szPre[]   = "Yo!";
  const char  szA[]     = "AAAAAAA";
  const char  szAYo[]   = "Yo!AAAA";
  char        szShort[8];
  unsigned    len;
  unsigned    i;

  FlyTestBegin();

  // test FlyStrZFill() and FlyStrZCatFill()
  memset(szShort, 'B', sizeof(szShort));
  len = FlyStrZFill(NULL, 'A', sizeof(szShort), sizeof(szShort));
  if(len != sizeof(szA) - 1)
    FlyTestFailed();
  len = FlyStrZFill(szShort, 'A', sizeof(szShort), sizeof(szShort));
  if(len != sizeof(szShort) - 1 || strcmp(szShort, szA) != 0)
    FlyTestFailed();
  strcpy(szShort, szPre);
  len = FlyStrZCatFill(NULL, 'A', sizeof(szShort), sizeof(szShort));
  if(len != sizeof(szShort) - 1)
    FlyTestFailed();
  len = FlyStrZCatFill(szShort, 'A', sizeof(szShort), sizeof(szShort));
  if(len != (sizeof(szShort) - (strlen(szPre) + 1)) || strcmp(szShort, szAYo) != 0)
  {
    printf("len %u, %s\n", len, szShort);
    FlyTestFailed();
  }

  // test FlyStrZCpy()
  for(i = 0; i < NumElements(aList); ++i)
  {
    memset(szShort, 'B', sizeof(szShort));
    len = FlyStrZCpy(NULL, aList[i].szStr, sizeof(szShort));
    if(len != strlen(aList[i].szExp))
      FlyTestFailed();
    memset(szShort, 'B', sizeof(szShort));
    len = FlyStrZCpy(szShort, aList[i].szStr, sizeof(szShort));
    if(len != strlen(aList[i].szExp) || strcmp(szShort, aList[i].szExp) != 0)
      FlyTestFailed();
  }

  // test FlyStrZCat()
  for(i = 0; i < NumElements(aList); ++i)
  {
    strcpy(szShort, szPre);
    len = FlyStrZCat(szShort, aList[i].szStr, sizeof(szShort));
    if(len != (strlen(aList[i].szExpPre) - strlen(szPre)) || strcmp(szShort, aList[i].szExpPre) != 0)
    {
      FlyTestPrintf("\n%u: len %u, %s\n", i, len, szShort);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test StrSlug function
-------------------------------------------------------------------------------------------------*/
void TcStrSlug(void)
{
  typedef struct
  {
    const char  *sz;
    const char  *szSlug;
  } TcStrSlug_t;
  TcStrSlug_t aTests[] =
  {
    { "  I Love   Waffles  ", "I-Love-Waffles" },     // do not move this one entry, see code
    { "my.echo My Shadow & Me", "my.echo-My-Shadow-Me" },
    { "-._~", "-._~" },
    { "a - b . c _ d ~ e", "a-b.c_d~e" },
    { u8"🔥 🐈 😊 木", "%f0%9f%94%a5-%f0%9f%90%88-%f0%9f%98%8a-%e6%9c%a8" },
    { "", "" },
    { "       ", "" },
    { "az09AZ-._~", "az09AZ-._~" },
    { "abc", "abc" },
    { "a.2c", "a.2c" },
    { "Who Knows?", "Who-Knows" },
    { " ME,   my.self &\r\n I    ", "ME-my.self" },
    { " 4.8 - @logo link", "4.8-logo-link" },
  };
  char        szSlug[64];   // manually sized, see aTests[].szSlug above
  unsigned    len;
  unsigned    lenExp;
  unsigned    i;

  FlyTestBegin();

  for(i = 0; i < NumElements(aTests); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("\n%i sz '%s', szSlug '%s'\n", i, aTests[i].sz, aTests[i].szSlug);

    // test getting slug length only
    len = FlyStrSlug(NULL, aTests[i].sz, sizeof(szSlug), 0);
    lenExp = strlen(aTests[i].szSlug);
    if(len != lenExp)
    {
      FlyTestPrintf("%u: %s, got len %u, expected %u\n", i, aTests[i].sz, len, lenExp);
      FlyTestFailed();
    }
  
    FlyStrZFill(szSlug, 'A', sizeof(szSlug), sizeof(szSlug));
    len = FlyStrSlug(szSlug, aTests[i].sz, sizeof(szSlug), 0);
    if((len != lenExp) || (strcmp(szSlug, aTests[i].szSlug) != 0))
    {
      FlyTestPrintf("%u: got %u,'%s', expected %u,'%s'\n", i, len, szSlug, lenExp, aTests[i].szSlug);
      FlyTestFailed();
    }
    if(szSlug[lenExp + 1] != 'A')
      FlyTestFailed();
  }

  // test small size
  if(FlyTestVerbose())
    FlyTestPrintf("\nSmall 0, 0 sz '%s'\n", aTests[i].sz);
  FlyStrZFill(szSlug, 'A', sizeof(szSlug), sizeof(szSlug));
  len = FlyStrSlug(szSlug, aTests[0].sz, 0, 0);
  lenExp = 0;
  if((len != lenExp) || (strcmp(szSlug, "") != 0))
  {
    FlyTestPrintf("small 0 => got %u,'%s', expected %u,'%s'\n", len, szSlug, lenExp, "");
    FlyTestFailed();
  }

  // test small size
  if(FlyTestVerbose())
    FlyTestPrintf("\nSmall 2, 0 sz '%s'\n", aTests[i].sz);
  FlyStrZFill(szSlug, 'A', sizeof(szSlug), sizeof(szSlug));
  len = FlyStrSlug(szSlug, aTests[0].sz, 2, 0);
  lenExp = 1;
  if((len != lenExp) || (strcmp(szSlug, "I") != 0))
  {
    FlyTestPrintf("small 2 => got %u,'%s', expected %u,'%s'\n", len, szSlug, lenExp, "I");
    FlyTestFailed();
  }

  // test small size
  if(FlyTestVerbose())
    FlyTestPrintf("\nSmall 4, 0 sz '%s'\n", aTests[i].sz);
  FlyStrZFill(szSlug, 'A', sizeof(szSlug), sizeof(szSlug));
  len = FlyStrSlug(szSlug, aTests[0].sz, 4, 0);
  lenExp = 3;
  if((len != lenExp) || (strcmp(szSlug, "I-L") != 0))
  {
    FlyTestPrintf("small 4 => got %u,'%s', expected %u,'%s'\n", i, len, szSlug, lenExp, "I-L");
    FlyTestFailed();
  }

  // test small maxLen
  FlyStrZFill(szSlug, 'A', sizeof(szSlug), sizeof(szSlug));
  if(FlyTestVerbose())
    FlyTestPrintf("\nSmall sizeof(szLug), 6 sz '%s'\n", aTests[i].sz);
  len = FlyStrSlug(szSlug, aTests[0].sz, sizeof(szSlug), 6);
  lenExp = 4;
  if((len != lenExp) || (strcmp(szSlug, "I-Lo") != 0))
  {
    FlyTestPrintf("small maxLen(5) => got %u,'%s', expected %u,'%s'\n", len, szSlug, lenExp, "I-Lo");
    FlyTestFailed();
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test path slash functions
-------------------------------------------------------------------------------------------------*/
void TcStrPathSlash(void)
{
  typedef struct
  {
    const char    *szThis;
    const char    *szThat;
    unsigned      len;
  } TcNameLast_t;
  static const TcNameLast_t aNameLast[] =
  {
    { "name",                 "name"     },
    { "name/",                "name"     },
    { "/path1/path2/folder/", "folder"   },
    { "~/Work/myfile.c",      "myfile.c" }
  };
  const char *psz;
  unsigned    i;
  unsigned    len;

  FlyTestBegin();

  for(i = 0; i < NumElements(aNameLast); ++i)
  {
    len  = 0;
    psz = FlyStrPathNameLast(aNameLast[i].szThis, &len);
    if(len != strlen(aNameLast[i].szThat) || strncmp(psz, aNameLast[i].szThat, len) != 0)
    {
      FlyTestPrintf("\n%u: got %.*s, exp %s\n", len, psz, aNameLast[i].szThat);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyCharEsc(), FlyCharOct() and FlyCharHex()
-------------------------------------------------------------------------------------------------*/
void TcCharEsc(void)
{
  static const char     aszEsc1[]       = "\\001~a0A\\a\\b\\f\\n\\r\\t\\v\\\\\\'\\\"";
  static const char     aszEsc1Exp[]    = "\001~a0A\a\b\f\n\r\t\v\\\'\"";
  static const char     aszEscOct[]     = "\\1\\033\\777";
  static const uint8_t  aszEscOctExp[]  = { 001, 033, 0xff, 0x00 };
  static const char     aszEscHex[]     = "\\x1\\xfE";
  static const uint8_t  aszEscHexExp[]  = { 0x01, 0xfe, 0x00 };
  static char          *psz;
  uint8_t               byte = 0;
  uint8_t               aBuff[32];
  unsigned              len;

  FlyTestBegin();

  // test FlyCharOct()
  psz = FlyCharOct("1.23472", &byte);
  if(*psz != '.' || byte != 1)
    FlyTestFailed();
  ++psz;
  psz = FlyCharOct(psz, &byte);
  if(*psz != '7' || byte != 0234)
    FlyTestFailed();
  psz = FlyCharOct(psz, &byte);
  if(*psz != '\0' || byte != 072)
    FlyTestFailed();

  // test FlyCharHex()
  psz = FlyCharHex("f.3Ed", &byte);
  if(*psz != '.' || byte != 0xf)
    FlyTestFailed();
  ++psz;
  psz = FlyCharHex(psz, &byte);
  if(*psz != 'd' || byte != 0x3e)
    FlyTestFailed();
  psz = FlyCharHex(psz, &byte);
  if(*psz != '\0' || byte != 0xd)
    FlyTestFailed();

  // test FlyCharEsc()
  psz = (char *)aszEsc1;
  // if(FlyTestVerbose())
  // {
  //   FlyTestPrintf("\naszEsc1:\n");
  //   FlyTestDump(aszEsc1, sizeof(aszEsc1));
  //   FlyTestPrintf("aszEsc1Exp:\n");
  //   FlyTestDump(aszEsc1Exp, sizeof(aszEsc1Exp));
  //   FlyTestPrintf("psz %p\n", psz);
  // }
  memset(aBuff, 0, sizeof(aBuff));
  for(len = 0; psz && *psz && len < sizeof(aBuff) - 1; ++len)
  {
    psz = FlyCharEsc(psz, &aBuff[len]);
    // if(FlyTestVerbose())
    //   printf("psz %p, len %u c %c c 0x%02x\n", psz, len, isprint(aBuff[len]) ? aBuff[len] : '.', aBuff[len]);
  }
  if((len != strlen(aszEsc1Exp)) || (memcmp(aBuff, aszEsc1Exp, len) != 0))
  {
    printf("len %u, exp %u\n", len, (unsigned)strlen(aszEsc1Exp));
    FlyTestDumpCmp(aBuff, aszEsc1Exp, len);
    FlyTestFailed();
  }
  psz = (char *)aszEscOct;
  memset(aBuff, 0, sizeof(aBuff));
  for(len = 0; psz && *psz && len < sizeof(aBuff) - 1; ++len)
    psz = FlyCharEsc(psz, &aBuff[len]);
  if((len != strlen((const char *)aszEscOctExp)) || (memcmp(aBuff, aszEscOctExp, len) != 0))
  {
    printf("len %u, exp %u\n", len, (unsigned)strlen((const char *)aszEscOctExp));
    FlyTestDumpCmp(aBuff, aszEscOctExp, len);
    FlyTestFailed();
  }
  psz = (char *)aszEscHex;
  // if(FlyTestVerbose())
  // {
  //   FlyTestPrintf("\naszEscHex %p\n", aszEscHex);
  //   FlyTestDump(psz, sizeof(aszEscHexExp));
  // }
  memset(aBuff, 0, sizeof(aBuff));
  for(len = 0; psz && *psz && len < sizeof(aBuff) - 1; ++len)
  {
    psz = FlyCharEsc(psz, &aBuff[len]);
    // if(FlyTestVerbose())
    //   printf("psz %p, len %u c %c c 0x%02x\n", psz, len, isprint(aBuff[len]) ? aBuff[len] : '.', aBuff[len]);
  }
  if((len != strlen((const char *)aszEscHexExp)) || (memcmp(aBuff, aszEscHexExp, len) != 0))
  {
    printf("len %u, exp %u\n", len, (unsigned)strlen((const char *)aszEscHexExp));
    FlyTestDumpCmp(aBuff, aszEscHexExp, len);
    FlyTestFailed();
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyStrReplace()
-------------------------------------------------------------------------------------------------*/
void TcStrReplace(void)
{
  const char  szQuote[]       = "I {me} what I {me}";
  const char  szQuoteExp[]    = "I am what I am";
  const char  szQuoteExp2[]   = "I am not what I am not";
  const char  szQuoteExp3[]   = "I am what I {me}";
  const char  szQuestion[]    = "be or not to be";
  const char  szQuestionExp[] = "Think or not to Think";
  const char  szMe[]          = "me-me-me-";
  const char  szMeExp[]       = "mememe";
  unsigned    len;
  char        sz[128];

  FlyTestBegin();

  // verify replaces ok with shorter replacement string
  memset(sz, 'a', sizeof(sz));
  sz[sizeof(sz) - 1] = '\0';
  strcpy(sz, szQuote);
  len = FlyStrReplace(sz, sizeof(szQuote), "{me}", "am", FLYSTR_REP_ALL);
  if(len != strlen(szQuoteExp) || strcmp(sz, szQuoteExp) != 0)
  {
    FlyTestPrintf("len %u, sz `%s`\n", len, sz);
    FlyTestFailed();
  }

  // verify string unchanged if string to replace not found
  strcpy(sz, szQuote);
  len = FlyStrReplace(sz, sizeof(szQuote), "{notfound}", "am", FLYSTR_REP_ALL);
  if(len != strlen(szQuote) || strcmp(sz, szQuote) != 0)
  {
    FlyTestPrintf("len %u, sz `%s`\n", len, sz);
    FlyTestFailed();
  }

  // verify doesn't replace (original left unchanged) if size is too small
  memset(sz, 0, sizeof(sz));
  strcpy(sz, szQuote);
  len = FlyStrReplace(sz, sizeof(szQuoteExp) - 1, "{me}", "am", FLYSTR_REP_ALL);
  if(len != strlen(szQuoteExp) || strcmp(sz, szQuote) != 0)
  {
    FlyTestPrintf("len %u, sz `%s`\n", len, sz);
    FlyTestFailed();
  }

  // verify replace with something else, case insensitive
  memset(sz, 0, sizeof(sz));
  strcpy(sz, szQuote);
  len = FlyStrReplace(sz, sizeof(sz), "{ME}", "am not", FLYSTR_REP_ALL_CASE);
  if(len != strlen(szQuoteExp2) || strcmp(sz, szQuoteExp2) != 0)
  {
    FlyTestPrintf("len %u, sz `%s`\n", len, sz);
    FlyTestFailed();
  }

  // verify replace only once
  memset(sz, 0, sizeof(sz));
  strcpy(sz, szQuote);
  len = FlyStrReplace(sz, sizeof(sz), "{me}", "am", FLYSTR_REP_ONCE);
  if(len != strlen(szQuoteExp3) || strcmp(sz, szQuoteExp3) != 0)
  {
    FlyTestPrintf("len %u, sz `%s`\n", len, sz);
    FlyTestFailed();
  }

  // verify we can get needed length, then do replacement
  memset(sz, 0, sizeof(sz));
  strcpy(sz, szQuestion);
  len = FlyStrReplace(sz, 0, "be", "Think", FLYSTR_REP_ALL);
  if(len != strlen(szQuestionExp))
    FlyTestFailed();
  len = FlyStrReplace(sz, len + 1, "be", "Think", FLYSTR_REP_ALL);
  if(len != strlen(szQuestionExp) || strcmp(sz, szQuestionExp) != 0)
  {
    FlyTestPrintf("len %u, sz `%s`\n", len, sz);
    FlyTestFailed();
  }

  // verify replacing with nothing (remove found text)
  memset(sz, 0, sizeof(sz));
  strcpy(sz, szMe);
  len = FlyStrReplace(sz, sizeof(sz), "-", "", FLYSTR_REP_ALL);
  if(len != strlen(szMeExp) || strcmp(sz, szMeExp) != 0)
  {
    FlyTestPrintf("len %u, sz `%s`\n", len, sz);
    FlyTestFailed();
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyStrCount()
-------------------------------------------------------------------------------------------------*/
void TcStrCount(void)
{
  const char  szHaystack1[] = "%s This %s is %s";
  const char  szHaystack2[] = "Only has %u";
  const char  szNeedle[]    = "%s";

  FlyTestBegin();

  if(FlyStrCount(szHaystack1, szNeedle) != 3)
    FlyTestFailed();
  if(FlyStrCount(szHaystack2, szNeedle) != 0)
    FlyTestFailed();

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyStrNToL() and FlyStrLToStr()
-------------------------------------------------------------------------------------------------*/
void TcStrTol(void)
{
  typedef struct
  {
    const char *szNum;
    const char *szPlain;
    long        val;
    int         base;
    unsigned    nDigits;
    const char *szIgnore;
  } tcStrTol_t;

  static const tcStrTol_t aTests[] =
  {
    { "0",         "0",          0L,  0, 1 },   // decimal (10)
    { "5",         "5",          5L, 10, 1 },   // decimal (10)
    { "+5",        "5",          5L, 10, 1 },   // decimal (10)
    { "-53",       "-53",       -53L, 0, 3 },   // decimal (10)
    { "0b10111",   "10111",     23L,  2, 5 },   // binary (2)
    { "0733",      "733",     0733L,  8, 3 },   // octal (8)
    { "0z3x2e9",   "3x2e9",  79917L, 12, 5 },   // dozenal (12)
    { "0xfe29",    "fe29",  0xfe29L, 16, 4 },   // hex (16)
    { "1_0111",    "10111",     23L,  2, 5, "_" },
    { ",fe_2a,",   "fe2a",  0xfe2aL, 16, 4, ",_" },
    { "+9,223,372,036,854,775,807",  "9223372036854775807",  LONG_MAX, 10, 19, "," },
    { "-9_223_372_036_854_775_808", "-9223372036854775808",  LONG_MIN, 10, 20, "_" },
  };

  char      szBuffer[32]; // long enough -9_223_372_036_854_775_808
  char     *szEnd;
  long      val;
  unsigned  nDigits;
  unsigned  i;

  FlyTestBegin();

  for(i = 0; i < NumElements(aTests); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("i = %u\n", i);
    nDigits = 0;
    szEnd = NULL;
    val = FlyStrNToL(aTests[i].szNum, &szEnd, aTests[i].base, UINT_MAX, &nDigits, aTests[i].szIgnore);
    if(szEnd != aTests[i].szNum + strlen(aTests[i].szNum))
    {
      FlyTestPrintf("%i: bad szEnd, got %p, expected %p\n", i, szEnd, aTests[i].szNum + strlen(aTests[i].szNum));
      FlyTestFailed();
    }
    if(val != aTests[i].val)
    {
      FlyTestPrintf("%i: bad val, got %li, expected %li\n", i, val, aTests[i].val);
      FlyTestFailed();
    }
    if(nDigits != aTests[i].nDigits)
    {
      FlyTestPrintf("%i: bad nDigits %u expected %u\n", i, nDigits, aTests[i].nDigits);
      FlyTestFailed();
    }

    *szBuffer = '\0';
    nDigits = FlyStrLToStr(szBuffer, aTests[i].val, sizeof(szBuffer), aTests[i].base);
    FlyStrToLower(szBuffer);
    if(strcmp(szBuffer, aTests[i].szPlain) != 0)
    {
      FlyTestPrintf("%i: bad str, got %s, expected %s\n", i, szBuffer, aTests[i].szPlain);
      FlyTestFailed();
    }
    if(nDigits != aTests[i].nDigits)
    {
      FlyTestPrintf("%i: bad LToStr nDigits, got %u, expected %u\n", i, nDigits, aTests[i].nDigits);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyStrEscEndQuoted()
-------------------------------------------------------------------------------------------------*/
void TcStrEscEndQuoted(void)
{
  typedef struct
  {
    const char *sz;
    const char *szExp;
  } TcStrEscEndQuoted_t;

  static const TcStrEscEndQuoted_t aTests[] =
  {
    { "abc", "abc" },
    { "\"quoted string\" end1", "\" end1" },
    { "\"quoted \\\"escaped\\\" string\" end2", "\" end2" },
  };
  const char  szEscCpy[]    = "A \\\"quoted\\\" string";
  const char  szEscCpyExp[] = "A \"quoted\" string";
  unsigned    i;
  const char *psz;
  size_t      len;
  char        szBuf[sizeof(szEscCpy)];

  FlyTestBegin();

  for(i = 0; i < NumElements(aTests); ++i)
  {
    psz = FlyStrEscEndQuoted(aTests[i].sz);
    if(strcmp(psz, aTests[i].szExp) != 0)
    {
      FlyTestPrintf("%u: `%s`, got '%s', exp '%s'\n", i, aTests[i].sz, psz, aTests[i].szExp);
      FlyTestFailed();
    }
  }

  // test FlyStrEscNCpy()
  len = 0;
  memset(szBuf, 0, sizeof(szBuf));
  psz = FlyStrEscNCpy(szBuf, szEscCpy, strlen(szEscCpy), &len);
  if(len != strlen(szEscCpyExp) || strcmp(szBuf, szEscCpyExp) || *psz != '\0')
  {
    FlyTestPrintf("FlyStrEscNCpy, got len %zu, expected %zu\ngot '%s', expected '%s'\n", len,
                  strlen(szEscCpyExp), szBuf, szEscCpyExp);
    FlyTestFailed();
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyStrNStr() and FlyStrLineStr()
-------------------------------------------------------------------------------------------------*/
void TcStrNStr(void)
{
  typedef struct
  {
    const char *szHaystack;
    const char *szNeedle;
    bool_t      fFound;
  } TcStrNStr_t;

  static const TcStrNStr_t aTests[] =
  {
    { "haystack", "needle", FALSE },
    { "I c have clas not class", "class", TRUE },
    { "class dismissed", "class", TRUE },
  };
  unsigned    i;
  const char *psz;

  FlyTestBegin();

  for(i = 0; i < NumElements(aTests); ++i)
  {
    psz = FlyStrNStr(aTests[i].szHaystack, aTests[i].szNeedle, strlen(aTests[i].szHaystack));
    if(!aTests[i].fFound && psz)
    {
      FlyTestPrintf("%u: %s, %s, found when it should NOT have been\n", aTests[i].szHaystack, aTests[i].szNeedle);
      FlyTestFailed();
    }
    else if(aTests[i].fFound && !psz)
    {
      FlyTestPrintf("%u: %s, %s, NOT found when it should have been\n", aTests[i].szHaystack, aTests[i].szNeedle);
      FlyTestFailed();
    }
    else if(psz && strncmp(psz, aTests[i].szNeedle, strlen(aTests[i].szNeedle)) != 0)
    {
      FlyTestPrintf("%u: %s, %s, Doesn't compare, psz at %s\n", aTests[i].szHaystack, aTests[i].szNeedle, psz);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test arg functions
-------------------------------------------------------------------------------------------------*/
void TcStrArg(void)
{
  typedef struct
  {
    const char  *sz;
    const char  *szNextArg;
    unsigned    len;;
  } tcStrArg_t;

  const tcStrArg_t aTests[] =
  {
    { "arg1", "", 4 },
    { "arg1 arg2", "arg2", 4 },
    { u8"Hello🌎 arg2", "arg2", 9 },
    { "\"quoted arg\" arg2", "arg2", 12 },
    { "\"quoted \\\"escaped\\\" arg\" arg2", "arg2", 24 },
  };

  const char *pszNextArg;
  unsigned    i;
  unsigned    len;

  FlyTestBegin();

  for(i = 0; i < NumElements(aTests); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("\n%i: %s, arg2 %s, exp len %u\n", i, aTests[i].sz, aTests[i].szNextArg, aTests[i].len);

    len = FlyStrArgLen(aTests[i].sz);
    if(len != aTests[i].len)
    {
      FlyTestPrintf("\n%i: %s, exp len %u got %u\n", i, aTests[i].sz, aTests[i].len, len);
      FlyTestFailed();
    }

    pszNextArg = FlyStrArgNext(aTests[i].sz);
    if(strcmp(pszNextArg, aTests[i].szNextArg) != 0)
    {
      FlyTestPrintf("\n%i: %s, exp nextArg %s got %s\n", i, aTests[i].sz, aTests[i].szNextArg, pszNextArg);
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
  const char          szName[] = "test_str";
  const sTestCase_t   aTestCases[] =
  {
    { "TcCharEsc",            TcCharEsc },
    { "TcStrArg",             TcStrArg },
    { "TcStrBlankRemove",     TcStrBlankRemove },
    { "TcStrCase",            TcStrCase },
    { "TcStrCName",           TcStrCName },
    { "TcStrCount",           TcStrCount },
    { "TcStrDump",            TcStrDump },
    { "TcStrHdrFind",         TcStrHdrFind },
    { "TcStrLine",            TcStrLine },
    { "TcStrPath",            TcStrPath },
    { "TcStrPathHasExt",      TcStrPathHasExt },
    { "TcStrPathLang",        TcStrPathLang },
    { "TcStrPathParent",      TcStrPathParent },
    { "TcStrPathRelative",    TcStrPathRelative },
    { "TcStrPathSlash",       TcStrPathSlash },
    { "TcStrProtoLen",        TcStrProtoLen },
    { "TcStrReplace",         TcStrReplace },
    { "TcStrZFill",           TcStrZ },
    { "TcStrSlug",            TcStrSlug },
    { "TcStrEscEndQuoted",    TcStrEscEndQuoted },
    { "TcStrNStr",            TcStrNStr },
    { "TcStrTol",             TcStrTol },
  };
  hTestSuite_t        hSuite;
  int                 ret;

  // set up signal handling and logging
  // NedSigSetExit(argv[0], NULL);
  // FlyTestMaskSet(NEDLOG_JSON);

  FlyTestInit(szName, argc, argv);
  hSuite = FlyTestNew(szName, NumElements(aTestCases), aTestCases);
  FlyTestRun(hSuite);
  ret = FlyTestSummary(hSuite);
  FlyTestFree(hSuite);

  return ret;
}
