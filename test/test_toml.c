/**************************************************************************************************
  test_toml.c
  Copyright 2023 Drew Gislason
  License: MIT <https://mit-license.org>
**************************************************************************************************/
#include "FlyTest.h"
#include "FlyToml.h"

const char szTomlFile[] =
  "# sample toml file\n"
  "[package]\n"
  "name = \"moocow\"\n"
  "version = \"0.1.0\"\n"
  "authors = [\"My Name <me@mysite.com>\"]\n"
  "\n"
  "[dependencies]\n"
  "rand = \"0.5.5\"\n"
  "adder = { path=\"../adder\" }\n"
  "flylib = { git=\"https://github.com/drewgislason/flylib\", branch=\"main\", version=\"0.9\" }\n";

// // shared by multiple test cases
// static const char szTomlFile2[] =
//   "[alltypes]\n"
//   "\n"
//   "# comment here\n"
//   "integer = 5\n"
//   "false  = false\n"
//   "\n"
//   "true   = true\n"
//   "    # another comment\n"
//   "string = \"string\"\n"
//   "escaped.string = \"esc: \\t\\r\\n\"\n"
//   "literal.string = 'format string: \\t string'\n"
//   "array = [true, false]\n"
//   "object = { string = \"str\", int = 99, bool = true }\n"
//   "\n"
//   "[table2]\n";

// static const char szTomlFileNumbers[] = 
//   "# integers\n"
//   "int1 = +99\n"
//   "int2 = 42\n"
//   "int3 = 0\n"
//   "int4 = -17\n"
//   "\n"
//   "# hexadecimal with prefix `0x`\n"
//   "hex1 = 0xDEADBEEF\n"
//   "hex2 = 0xdeadbeef\n"
//   "hex3 = 0xdead_beef\n"
//   "\n"
//   "# octal with prefix `0o`\n"
//   "oct1 = 0o01234567\n"
//   "oct2 = 0o755\n"
//   "\n"
//   "# binary with prefix `0b`\n"
//   "bin1 = 0b11010110\n";

// #if TOML_CFG_FLOAT
// static const char szTomlFileFloats[] =
//   "\n"
//   "# fractional\n"
//   "float1 = +1.0\n"
//   "float2 = 3.1415\n"
//   "float3 = -0.01\n"
//   "\n"
//   "# exponent\n"
//   "float4 = 5e+22\n"
//   "float5 = 1e06\n"
//   "float6 = -2E-2\n"
//   "\n"
//   "# both\n"
//   "float7 = 6.626e-34\n"
//   "\n"
//   "# separators\n"
//   "float8 = 224_617.445_991_228\n";
// #endif

// #if TOML_CFG_DATE
// static const char szTomlFileDates[] = 
//   "# offset datetime\n"
//   "odt1 = 1979-05-27T07:32:00Z\n"
//   "odt2 = 1979-05-27T00:32:00-07:00\n"
//   "odt3 = 1979-05-27T00:32:00.999999-07:00\n"
//   "\n"
//   "# local datetime\n"
//   "ldt1 = 1979-05-27T07:32:00\n"
//   "ldt2 = 1979-05-27T00:32:00.999999\n"
//   "\n"
//   "# local date\n"
//   "ld1 = 1979-05-27\n"
//   "\n"
//   "# local time\n"
//   "lt1 = 07:32:00\n"
//   "lt2 = 00:32:00.999999\n";
// #endif

/*-------------------------------------------------------------------------------------------------
  print a key
-------------------------------------------------------------------------------------------------*/
void TomlPrintKey(const tomlKey_t * pKey)
{
  printf("key %p: szKey(%p)=%.4s, szValue(%p)=%.4s, type=%u\n", pKey, pKey->szKey,
    FlyStrNullOk(pKey->szKey), pKey->szValue, FlyStrNullOk(pKey->szValue), pKey->type);
}

/*-------------------------------------------------------------------------------------------------
  Allocate a TOML key
-------------------------------------------------------------------------------------------------*/
char * TomlKeyAlloc(const char *szTomlKey)
{
  char         *szKey;
  unsigned      size;

  size = FlyTomlKeyLen(szTomlKey) + 1;
  szKey = FlyAlloc(size);
  if(szKey)
    FlyTomlKeyCpy(szKey, szTomlKey, size);

  return szKey;
}

/*-------------------------------------------------------------------------------------------------
  Allocate a TOML key
-------------------------------------------------------------------------------------------------*/
char * TomlStrAlloc(const char *szTomlStr)
{
  char         *szStr;
  unsigned      size;

  size = FlyTomlStrLen(szTomlStr) + 1;
  szStr = FlyAlloc(size);
  if(szStr)
    FlyTomlStrCpy(szStr, szTomlStr, size);

  return szStr;
}

/*-------------------------------------------------------------------------------------------------
  Compare a TOML key with an expected key
-------------------------------------------------------------------------------------------------*/
bool_t TomlKeyCmp(const char *szTomlKey, const char *szCmp)
{
  char         *szKey;
  bool_t        fSame   = FALSE;

  szKey = TomlKeyAlloc(szTomlKey);
  if(szKey)
  {
    if(strcmp(szKey, szCmp) == 0)
      fSame = TRUE;
    FlyFree(szKey);
  }

  return fSame;
}

/*-------------------------------------------------------------------------------------------------
  Compare a TOML string with an expected string
-------------------------------------------------------------------------------------------------*/
bool_t TomlStrCmp(const char *szTomlStr, const char *szCmp)
{
  char         *szStr;
  bool_t        fSame   = FALSE;

  szStr = TomlStrAlloc(szTomlStr);
  if(szStr)
  {
    if(strcmp(szStr, szCmp) == 0)
      fSame = TRUE;
    FlyFree(szStr);
  }

  return fSame;
}

/*-------------------------------------------------------------------------------------------------
  Test FlyTomlAtoBool(), FlyTomlAtol(), FlyTomlAtoDate() and FlyTomlAtof().

  See <https://toml.io/en/v1.0.0#boolean>  
  See <https://toml.io/en/v1.0.0#integer>  
  See <https://toml.io/en/v1.0.0#offset-date-time>  
  See <https://toml.io/en/v1.0.0#float>  
-------------------------------------------------------------------------------------------------*/
void TcTomlAtox(void)
{
  const char *aszTomlInts[] =
  {
    "+99",
    "42",
    "0",
    "-17",
    "1_000",
    "5_349_221",
    "-9223372036854775808   # LONG_MIN",
    "9223372036854775807    # LONG_MAX",
    "0xDEADBEEF",
    "0xdeadbeef",
    "0xdead_beef",
    "0o01234567",
    "0o755",
    "0b11010110"
  };
  long  aExpInts[] =
  {
    99,
    42,
    0,
    -17,
    1000,
    5349221,
    LONG_MIN,
    LONG_MAX,
    0xDEADBEEFL,
    0xdeadbeefL,
    0xdeadbeefL,
    01234567,
    0755,
    0b11010110L
  };
  unsigned    i;
  long        val;

  FlyTestBegin();

  // verify test data is OK
  FlyTestAssert(NumElements(aszTomlInts) == NumElements(aExpInts));

  // check for correctness
  for(i = 0; i < NumElements(aszTomlInts); ++i)
  {
    val = FlyTomlAtol(aszTomlInts[i]);
    if(val != aExpInts[i])
    {
      FlyTestPrintf("%u: Mismatch, got %ld, exp %ld\n", i, val, aExpInts[i]);
      FlyTestFailed();
    }
  }
 
  FlyTestEnd();
}



/*-------------------------------------------------------------------------------------------------
  FlyTomlStrlenLen() and FlyTomlStrCpy()
-------------------------------------------------------------------------------------------------*/
void TcTomlStrLen(void)
{
  typedef struct
  {
    const char *sz;
    const char *szExp;
    unsigned  len;
  } TcTomlStr_t;

  const TcTomlStr_t   aStr[] =
  {
    { "string",                       "string",               6  },
    { "\"string\"",                   "string",               6  },
    { "name = \"moocow\"\n",          "name",                 4  },
    { "version=\"0.1.0\"\n",          "version",              7  },
    { "\"quote \\\" in middle\"",     "quote \" in middle",   17 },
    { "\"escaped\\nstring\"",         "escaped\nstring",      14 },
    { "'not escaped\\nstring'",       "not escaped\\nstring", 19 },
    { "'literal\\nstring',",          "literal\\nstring",     15 },
    { "literal.key\\t=\"hello\"",     "literal.key\\t",       13 },
    { "'literal.key\\t'  =\"hello\"", "literal.key\\t",       13 },
    { "[table]",                      "table",                5  },
    { "[\"table.2\"]",                "table.2",              7  },
    { "['table [3]']",                "table [3]",            9  },
    { "git@gitlab.com:drew.gislason/foo.git", "git@gitlab.com:drew.gislason/foo.git", 36 },
  };
  char      sz[64];
  unsigned  len;
  unsigned  i;

  FlyTestBegin();

  // test FlyTomlStrLen()
  for(i = 0; i < NumElements(aStr); ++i)
  {
    if(FlyTomlStrLen(aStr[i].sz) != aStr[i].len)
    {
      FlyTestPrintf("\n%u: got len %u, %s\n", i, FlyTomlStrLen(aStr[i].sz), aStr[i].sz);
      FlyTestFailed();
    }
  }

  // test FlyTomlStrLen()
  for(i = 0; i < NumElements(aStr); ++i)
  {
    memset(sz, 0, sizeof(sz));
    len = FlyTomlStrCpy(sz, aStr[i].sz, sizeof(sz));
    if(len != aStr[i].len || len != strlen(sz))
    {
      FlyTestPrintf("\ncpy %u: len %u, %s, got %s\n", i, len, aStr[i].sz, sz);
      FlyTestFailed();
    }
    if(strncmp(sz, aStr[i].szExp, len) != 0)
    {
      FlyTestPrintf("\ncpy %u: len %u, got %s, exp %s\n", i, len, sz, aStr[i].szExp);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyTomlArrayIter()
-------------------------------------------------------------------------------------------------*/
void TcTomlArrayIter(void)
{
  static const char szTomlArray[] = "[true, 42, \"string\", -12_345]";
  const char       *pszValue;
  tomlValue_t       value;

  FlyTestBegin();

  memset(&value, 0, sizeof(value));
  pszValue = FlyTomlArrayIter(szTomlArray, &value);
  if(!pszValue || value.type != TOML_TRUE)
  {
    FlyTestPrintf("pszValue %p, type %u, szValue %.*s\n", pszValue, value.type,
      (unsigned)FlyStrLineLen(value.szValue), value.szValue);
    FlyTestFailed();
  }

  memset(&value, 0, sizeof(value));
  pszValue = FlyTomlArrayIter(pszValue, &value);
  if(!pszValue || value.type != TOML_INTEGER || atoi(value.szValue) != 42)
  {
    FlyTestPrintf("pszValue %p, type %u, szValue %.*s\n", pszValue, value.type,
      (unsigned)FlyStrLineLen(value.szValue), value.szValue);
    FlyTestFailed();
  }

  memset(&value, 0, sizeof(value));
  pszValue = FlyTomlArrayIter(pszValue, &value);
  if(!pszValue || value.type != TOML_STRING || *value.szValue != '"')
  {
    FlyTestPrintf("pszValue %p, type %u, szValue %.*s\n", pszValue, value.type,
      (unsigned)FlyStrLineLen(value.szValue), value.szValue);
    FlyTestFailed();
  }

  memset(&value, 0, sizeof(value));
  pszValue = FlyTomlArrayIter(pszValue, &value);
  if(!pszValue || value.type != TOML_INTEGER || FlyTomlAtol(value.szValue) != -12345)
  {
    FlyTestPrintf("pszValue %p, type %u, szValue %.*s\n", pszValue, value.type,
      (unsigned)FlyStrLineLen(value.szValue), value.szValue);
    FlyTestFailed();
  }

  // should be at end of array
  pszValue = FlyTomlArrayIter(pszValue, &value);
  if(pszValue)
    FlyTestFailed();

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyTomlKeyIter()
-------------------------------------------------------------------------------------------------*/
void TcTomlKeyIter(void)
{
  typedef struct
  {
    const char       *szTomlTable;
    const tomlKey_t  *aTableExp;
    unsigned          nTableExp;
  } tcTomlKeyIter_t;

  static const char  szTomlTable[] = 
    "[header]\n"
    "key1  =    true\n"
    "key2=42\n"
    "# a comment line\n"
    "key3 = \"string\"\n"
    "\n"
    "[header2]\n";
  static const tomlKey_t  aTableExp[] =
  {
     { "key1", "true",    TOML_TRUE },
     { "key2", "42",      TOML_INTEGER },
     { "key3", "string",  TOML_STRING },
  };

  static const char szTomlTable2[] =  
  "[dependencies]\n"
  "dep1 = { path=\"../dep1/\" }\n"
  "dep2 = { path=\"../dep2/lib/dep2.a\", inc=\"../dep2/inc/\" }\n"
  "flylib = { git=\"https://github.com/drewagislason/flylib\", version=\"*\" }\n"
  "\n"
  "[compiler]\n"
  "# .c = { cc=\"cc {in} -c {incs} -Wall -Werror -o {out}\", ll=\"cc {in} -o {out}\", cc_dbg=\"-g -DDEBUG=1\", ll_dbg=\"-g\" }\n"
  "# .cc.cpp.c++ = { cc=\"c++ {in} -c {incs} -Wall -Werror -o {out}\", ll=\"c++ {in} -o {out}\", ar=\"ar {out} -rcs {in}\" }\n";

  static const tomlKey_t  aTableExp2[] =
  {
     { "dep1", "{", TOML_INLINE_TABLE },
     { "dep2", "{", TOML_INLINE_TABLE },
     { "flylib", "{", TOML_INLINE_TABLE },
  };

  static const char  szTomlInlineTable[] = "{ k1=99, 'k2'=\"string\", \"k3\" = 'three', k4=-1 }\n";
  static const tomlKey_t  aInlineTableExp[] =
  {
     { "k1", "99",      TOML_INTEGER },
     { "k2", "string",  TOML_STRING },
     { "k3", "three",   TOML_STRING },
     { "k4", "-1",      TOML_INTEGER },
  };

  static const char  szTomlNoHeader[] =
    u8"# a no-header TOML file\n"
    "key1.\"\u203c\"=\"bang bang\" # bang bang\n"
    "    'key2'.\"\u21d0\" = \"left double arrow\"  # end of line comment\n"
    "\n"
    "     # comment\n"
    "\n"
    "\"key3\"  .  \"\u21d2\"    =    \"right double arrow\"\n"
    "\n";
  static const tomlKey_t  aNoHeaderTableExp[] =
  {
     { u8"key1.\u203c", "bang bang", TOML_STRING },
     { u8"key2.\u21d0", "left double arrow", TOML_STRING },
     { u8"key3.\u21d2", "right double arrow", TOML_STRING },
  };


  tomlKey_t       key;
  unsigned        i, j;
  const char     *pszKey;
  char            szTmp[TOML_CFG_KEY_MAX];  // large enough for any key/value in above data
  const tcTomlKeyIter_t aTables[] =
  {
    { szTomlTable,        aTableExp,          NumElements(aTableExp) },
    { szTomlTable2,       aTableExp2,         NumElements(aTableExp2) },
    { szTomlInlineTable,  aInlineTableExp,    NumElements(aInlineTableExp) },
    { szTomlNoHeader,     aNoHeaderTableExp,  NumElements(aNoHeaderTableExp) },
  };

  FlyTestBegin();

  // test indexing into a table header
  for(i = 0; i < NumElements(aTables); ++i)
  {
    pszKey = aTables[i].szTomlTable;
    for(j = 0; j < aTables[i].nTableExp; ++j)
    {
      if(FlyTestVerbose())
        FlyTestPrintf("\n%u:%u %.*s\n", i, j, (unsigned)FlyStrLineLen(pszKey), FlyStrNullOk(pszKey));

      memset(&key, 0, sizeof(key));
      pszKey = FlyTomlKeyIter(pszKey, &key);
      if(!pszKey || !key.szKey)
      {
        FlyTestPrintf("%u:%u No key\n", i, j);
        FlyTestFailed();
      }

      // verify value type is correct and that both key and value point to something
      if(key.type != aTables[i].aTableExp[j].type || !key.szKey || !key.szValue)
      {
        TomlPrintKey(&key);
        FlyTestPrintf("%u:%u Bad type, got %u, exp %u\n", i, j, key.type,
          aTables[i].aTableExp[j].type);
        FlyTestFailed();
      }

      // verfy key is correct
      FlyTomlKeyCpy(szTmp, key.szKey, sizeof(szTmp));
      if(strcmp(szTmp, aTables[i].aTableExp[j].szKey) != 0)
      {
        FlyTestPrintf("%u:%u Bad key, got %s\n", i, j, szTmp);
        FlyTestFailed();
      }

      // verify value is correct (cheats: assumes all values can be copied by FlyTomlKeyCpy)
      if(key.type == TOML_INLINE_TABLE)
      {
        if(*key.szValue != '{')
        {
          FlyTestPrintf("%u:%u Bad value %.*s\n", i, j, (unsigned)FlyStrLineLen(key.szValue), key.szValue);
          FlyTestFailed();
        }
      }
      else
      {
        FlyTomlKeyCpy(szTmp, key.szValue, sizeof(szTmp));
        if(strcmp(szTmp, aTables[i].aTableExp[j].szValue) != 0)
        {
          FlyTestPrintf("%u:%u Bad value %s\n", i, j, szTmp);
          FlyTestFailed();
        }
      }
    }

    // verify no more keys
    pszKey = FlyTomlKeyIter(pszKey, &key);
    if(pszKey != NULL)
    {
      FlyTestPrintf("%u: Too many keys\n");
      FlyTestFailed();      
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test TcTomlTableIter()
-------------------------------------------------------------------------------------------------*/
void TcTomlTableIter(void)
{
  static const char   szTomlFileSimple[] =
    "# a TOML file\n"
    "[simple]\n"
    "key = \"value\"\n";
  static const char szSimple[] = "[simple]";

  static const char   szTomlFileNoHeader[] =
    "# a TOML file with no table headers\n"
    "key1 = \"value\"\n"
    "key2 = true\n";

  static const char   szTomlFileThree[] =
    "# a TOML file\n"
    "table_no_name_key = \"value\"\n"
    "\n"
    "[table1]\n"
    "table1_key = true\n"
    "\n"
    "[table2]\n"
    "table2_key = 42\n";
  static const char  *aTomlTablesThree[] = { "", "[table1]", "[table2]" };
  const char         *pszTable;
  unsigned            i;

  FlyTestBegin();

  // test finding the header in a simple TOML file
  pszTable = FlyTomlTableIter(szTomlFileSimple, NULL);
  if(!pszTable || strncmp(pszTable, szSimple, strlen(szSimple)) != 0)
  {
    FlyTestPrintf("pszTable %p, %s\n", pszTable, FlyStrNullOk(pszTable));
    FlyTestFailed();
  }
  pszTable = FlyTomlTableIter(szTomlFileSimple, pszTable);
  if(pszTable)
    FlyTestFailed();

  // test finding the header in a no [table] TOML file
  pszTable = FlyTomlTableIter(szTomlFileNoHeader, NULL);
  if(!FlyTomlTableIsRoot(pszTable))
  {
    FlyTestPrintf("pszTable %p, %s\n", pszTable, FlyStrNullOk(pszTable));
    FlyTestFailed();
  }

  // test finding the headers in a TOML file
  pszTable = NULL;
  for(i = 0; i < NumElements(aTomlTablesThree); ++i)
  {
    pszTable = FlyTomlTableIter(szTomlFileThree, pszTable);
    if(i == 0)
    {
      if(!FlyTomlTableIsRoot(pszTable))
        FlyTestFailed();
    }
    else if(!pszTable || strncmp(pszTable, aTomlTablesThree[i], strlen(aTomlTablesThree[i])) != 0)
    {
      const char *sz;
      sz = FlyStrNullOk(pszTable);
      FlyTestPrintf("Got %.*s, expected %s\n", (unsigned)FlyStrLineLen(sz), sz, aTomlTablesThree[i]);
      FlyTestFailed();
    }
  }
  if(FlyTomlTableIter(szTomlFileSimple, pszTable))
    FlyTestFailed()

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Helper to test TcTomlKeyFind()
-------------------------------------------------------------------------------------------------*/
bool_t TkfCmpKeyValue(unsigned i, const char *szInlineTable, const char *szKeyName, const char *szExp)
{
  tomlKey_t   key;
  bool_t      fSame = FALSE;

  if(FlyTomlKeyFind(szInlineTable, szKeyName, &key))
  {
    if(szExp == NULL)
      FlyTestPrintf("%u: got key for %s, expected none\n", i, szKeyName);
    else if(key.szValue == NULL)
      FlyTestPrintf("%u: expected value for key %s\n", i, szKeyName);
    else if(!TomlStrCmp(key.szValue, szExp))
    {
      FlyTestPrintf("%u: got %.*s, exp %s\n", i, (unsigned)FlyTomlStrLen(key.szValue),
          FlyTomlPtr(key.szValue), szExp);
    }
    else
      fSame = TRUE;
  }
  else
  {
    if(szExp == NULL)
      fSame = TRUE;
    else
      FlyTestPrintf("%u: didn't find key %s, expected value %s\n", i, szKeyName, szExp);
  }

  return fSame;
}

/*-------------------------------------------------------------------------------------------------
  Test FlyTomlKeyFind()
-------------------------------------------------------------------------------------------------*/
void TcTomlKeyFind(void)
{
  typedef struct
  {
    const char *szKey;
    const char *szPath;
    const char *szInc;
    const char *szGit;
    const char *szVer;
  }
  tcTomlKeyFind_t;

  const char szTomlFile[] = 
  "[package]\n"
  "name = \"x\"\n"
  "version = \"0.1.0\"\n"
  "authors = [\"me@mysite.com\"]\n"
  "std = \"*\"\n"
  "\n"
  "[dependencies]\n"
  "foo = { path=\"../foo/lib/foo.a\", inc=\"../foo/inc/\" }\n"
  "bar = { git = \"https://github.com/drewagislason/flylib\", version=\"1.23.9\" }\n"
  "baz = { path=\"../baz/baz/\" }\n"
  "baz2 = { }\n";

  const char *szTable;
  const char *szInlineTable;
  const char *szIter;
  char       *szKey;
  tomlKey_t   key;
  const tcTomlKeyFind_t aTests[] = 
  {
    { "foo", "../foo/lib/foo.a", "../foo/inc/", NULL, NULL },
    { "bar", NULL, NULL, "https://github.com/drewagislason/flylib", "1.23.9" },
    { "baz", "../baz/baz/", NULL, NULL, NULL },
    { "baz2", NULL, NULL, NULL, NULL },
  };
  unsigned  i;

  FlyTestBegin();

  szTable = FlyTomlTableFind(szTomlFile, "dependencies");
  szIter = FlyTomlKeyIter(szTable, &key);
  if(!szIter)
  {
    FlyTestPrintf("Expected %u keys in [dependecies]\n", NumElements(aTests));
    FlyTestFailed();
  }

  i = 0;
  while(szIter)
  {
    if(FlyTestVerbose())
    {
      szKey = TomlKeyAlloc(key.szKey);
      FlyTestPrintf("\n%u: key %s .szKey %p, .szValue %p\n", i, FlyStrNullOk(szKey), key.szKey,
          key.szValue);
    }

    // verify this is the expected key
    if(!TomlKeyCmp(key.szKey, aTests[i].szKey))
    {
      FlyTestPrintf("%u: expKey %s\n", i, aTests[i].szKey);
      FlyTestFailed();
    }

    // every dependency must be a TOML inline table, e.g. foo = { "path" = "../foo/" }
    if(key.type != TOML_INLINE_TABLE)
    {
      FlyTestPrintf("%u: exp inline table, got type %u\n", i, key.type);
      FlyTestFailed();
    }

    // look for inline table keys we recognize in this dependency
    szInlineTable = key.szValue;
    if(!TkfCmpKeyValue(i, szInlineTable, "path", aTests[i].szPath))
      FlyTestFailed();
    if(!TkfCmpKeyValue(i, szInlineTable, "inc", aTests[i].szInc))
      FlyTestFailed();
    if(!TkfCmpKeyValue(i, szInlineTable, "git", aTests[i].szGit))
      FlyTestFailed();
    if(!TkfCmpKeyValue(i, szInlineTable, "version", aTests[i].szVer))
      FlyTestFailed();

    // on to next key in table
    szIter = FlyTomlKeyIter(szIter, &key);
    ++i;
    if(i > NumElements(aTests))
      FlyTestFailed();
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test TcTomlTableFind()
-------------------------------------------------------------------------------------------------*/
void TcTomlTableFind(void)
{
  typedef struct
  {
    const char  *szTomlFile;
    const char  *szTableName;
    const char  *szExp;
  } tcTomlTableFind_t;

  const char *pszTable;
  char        szTmp[100];
  const tcTomlTableFind_t aTables[] =
  {
    { szTomlFile, "dependencies", "dependencies" },
    { szTomlFile, "package", "package" },
    { szTomlFile, "nothere", NULL },
  };
  unsigned  i;

  FlyTestBegin();

  // find dependencies
  for(i = 0; i < NumElements(aTables); ++i)
  {
    pszTable = FlyTomlTableFind(aTables[i].szTomlFile, aTables[i].szTableName);
    if((aTables[i].szExp != NULL && pszTable == NULL) ||
       (aTables[i].szExp == NULL && pszTable != NULL))
    {
      FlyTestPrintf("%u: NULL mismatch got %p, exp %p\n", i, pszTable, aTables[i].szExp);
      FlyTestFailed();
    }

    // for those tables found, make sure name is right 
    if(pszTable)
    {
      memset(szTmp, 0, sizeof(szTmp));
      FlyTomlKeyCpy(szTmp, pszTable, sizeof(szTmp));
      if(strcmp(szTmp, aTables[i].szExp) != 0)
      {
        FlyTestPrintf("%u: Table didn't match, got %s, expected %s\n", i, szTmp, aTables[i].szExp);
        FlyTestFailed();
      }
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test TcTomlKeyPathFind()
-------------------------------------------------------------------------------------------------*/
void TcTomlKeyPathFind(void)
{
  typedef union
  {
    const char *sz;
    int         i;
  } tcTomlKeyPathVal_t;
  typedef struct
  {
    const char         *szKeyPath;
    tomlType_t          type;
    tcTomlKeyPathVal_t  val;
    bool_t              fExpFailRet;
  } TcTomlKeyPath_t;

  static const char szTomlFile[] = 
    "# a TOML file\n"
    "no_table_key = \"chair\"\n"
    "inline1 = { a = 0x61, b = 0x62 }\n"
    "\n"
    "[table1]\n"
    "key = 42\n"
    "hello = \"world\"\n"
    "array = [\"zero\", \"one\", \"two\", \"three\"]\n"
    "\n"
    "[table2]\n"
    "\"boolean\" = { yes = true, no = false }\n";

  TcTomlKeyPath_t aTests[] =
  {
    { ":no_table_key",      TOML_STRING, {.sz = "chair"} },
    { ":inline1",           TOML_INLINE_TABLE },
    { ":inline1:b",         TOML_INTEGER, {.i = 0x62} },
    { "table1:hello",       TOML_STRING, {.sz = "world"} },
    { "table1:array",       TOML_ARRAY },
    { "table2:boolean:yes", TOML_TRUE },
    { "table2:boolean:no",  TOML_FALSE },
    { ":nothere",           TOML_UNKNOWN, { .i=0 }, TRUE },
    { "table2:nothere",     TOML_UNKNOWN, { .i=0 }, TRUE },
  };

  tomlKey_t     key;
  unsigned      i;
  unsigned      len;
  bool_t        fFailRet;

  FlyTestBegin();

  for(i = 0; i < NumElements(aTests); ++i)
  {
    fFailRet = FALSE;
    if(!FlyTomlKeyPathFind(szTomlFile, aTests[i].szKeyPath, &key))
      fFailRet = TRUE;

    // make sure pass/fail agrees
    if(fFailRet != aTests[i].fExpFailRet)
    {
      FlyTestPrintf("%u: %s, bad ret, exp %u, got %u\n", i, aTests[i].szKeyPath, aTests[i].fExpFailRet, fFailRet);
      FlyTestFailed();
    }

    // make sure type agrees
    if(!aTests[i].fExpFailRet)
    {
      if(key.type != aTests[i].type)
      {
        FlyTestPrintf("%u: %s, bad type, exp %u, got %u\n", i, aTests[i].szKeyPath,
          aTests[i].type, key.type);
        FlyTestFailed();
      }

      if(key.type == TOML_STRING)
      {
        len = strlen(aTests[i].val.sz);
        if(strncmp(&key.szValue[1], aTests[i].val.sz, len) != 0)
        {
          FlyTestPrintf("%u: %s, bad str, exp %s, got %.*s\n", i, aTests[i].szKeyPath,
            aTests[i].val.sz, len, &key.szValue[1]);
          FlyTestFailed();
        }
      }

      if(key.type == TOML_INTEGER && FlyTomlAtol(key.szValue) != aTests[i].val.i)
      {
        FlyTestPrintf("%u: %s, bad int, exp %i, got %li\n", i, aTests[i].szKeyPath,
          aTests[i].val.i, FlyTomlAtol(key.szValue));
        FlyTestFailed();        
      }
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Tests All built-in types defined at <https://toml.io/en/v1.0.0#integer>

  * String
  * Integer
  * Float
  * Boolean
  * Offset Date-Time
  * Local Date-Time
  * Local Date
  * Local Time
  * Array
  * Inline Table
-------------------------------------------------------------------------------------------------*/
void TcTomlTypes(void)
{
  tomlValue_t aValueTypes[] =
  {
    { "\"string\"",                     TOML_STRING },  // String
    { "'string literal'",               TOML_STRING },  // String
    { "\"\"\"multi-line string\"\"\"",  TOML_STRING },  // String
    { "'''multi-line literal'''",       TOML_STRING },  // String
    { "-12345",                         TOML_INTEGER }, // Integer
#if TOML_CFG_FLOAT
    { "3.14159",                        TOML_FLOAT },   // Float
#endif
    { "true",                           TOML_TRUE },    // Boolean
    { "false",                          TOML_FALSE },   // Boolean
#if TOML_CFG_DATE
      // Offset Date-Time
      // Local Date-Time
      // Local Date
      // Local Time
#endif
    { "[1, true, \"string\" ]",         TOML_ARRAY },   // Array
    { "{ inline=\"table\" }",           TOML_INLINE_TABLE }, // Inline Table
    { u8"/u00D9nknown",                 TOML_UNKNOWN }, // Unknown
  };

  unsigned    i;
  tomlType_t  type;

  FlyTestBegin();

  for(i = 0; i < NumElements(aValueTypes); ++i)
  {
    type = FlyTomlType(aValueTypes[i].szValue);
    if(type != aValueTypes[i].type)
    {
      FlyTestPrintf("%u: failed %s, got type %u, expected %u\n", i, aValueTypes[i].szValue, type, aValueTypes[i].type);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test TOML strings in all their forms using FlyTomlStrLen() and FlyTomlStrCPy()

  See: <https://toml.io/en/v1.0.0#string>  
-------------------------------------------------------------------------------------------------*/
void TcTomlStrings(void)
{
  typedef struct
  {
    const char *szToml;
    const char *szExp;
  } tcTomlString_t;

  // All valid examples in TOML string spec
  // See <https://toml.io/en/v1.0.0#string>
  const tcTomlString_t aStrings[] =
  {
    { "\"\\\"escape codes\\\": \\x01 \\002 \\\\ \\a \\b \\e \\f \\n \\r \\t \\v \\'\"",
      "\"escape codes\": \x01 \002 \\ \a \b \e \f \n \r \t \v \'"
    },
    { "'\\\"no escapes\\\": \\x01 \\002 \\\\ \\a \\b \\e \\f \\n \\r \\t \\v'",
      "\\\"no escapes\\\": \\x01 \\002 \\\\ \\a \\b \\e \\f \\n \\r \\t \\v"
    },
    { "\"I'm a string. \\\"You can quote me\\\". Name\\tJos\\u00E9\\nLocation\\tSF.\"", 
      u8"I'm a string. \"You can quote me\". Name\tJos\u00E9\nLocation\tSF."
    },
    {
      "\"\"\"\nRoses are red\nViolets are blue\"\"\"\n",
      "Roses are red\nViolets are blue"
    },
    {
      "\"The quick brown fox jumps over the lazy dog.\"",
      "The quick brown fox jumps over the lazy dog."
    },
    {
      "\"\"\"\n"
      "The quick brown \\\n"
      "  fox jumps over \\\n"
      "    the lazy dog.\"\"\"\n",
      "The quick brown fox jumps over the lazy dog."
    },
    {
      "\"\"\"\\\n"
      "      The quick brown \\\n"
      "      fox jumps over \\\n"
      "      the lazy dog.\\\n"
      "      \"\"\"\n",
      "The quick brown fox jumps over the lazy dog."
    },
    {
      "\"\"\"Here are two quotation marks: \"\". Simple enough.\"\"\"",
      "Here are two quotation marks: \"\". Simple enough."
      // Here are two quotation marks: "". Simple enough.
    },
    {
      "\"\"\"Here are three quotation marks: \"\"\\\".\"\"\"",
      "Here are three quotation marks: \"\"\"."
      // Here are three quotation marks: """.
    },
    {
      "\"\"\"Here are fifteen quotation marks: \"\"\\\"\"\"\\\"\"\"\\\"\"\"\\\"\"\"\\\".\"\"\" extra",
      "Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\"."
      // Here are fifteen quotation marks: """"""""""""""".
    },
    {
      "\"\"\"\"This,\" she said, \"is just a pointless statement.\"\"\"\"",
      "\"This,\" she said, \"is just a pointless statement.\""
      // "This," she said, "is just a pointless statement."
    },
    {
      "'C:\\Users\\nodejs\\templates'",
      "C:\\Users\\nodejs\\templates"
      // C:\Users\nodejs\templates
    },
    {
      "'\\\\ServerX\\admin$\\system32\\'",
      "\\\\ServerX\\admin$\\system32\\"
      // \\ServerX\admin$\system32~
    },
    {
      "'Tom \"Dubs\" Preston-Werner'",
      "Tom \"Dubs\" Preston-Werner"
      // Tom "Dubs" Preston-Werner
    },
    {
      "'<\\i\\c*\\s*>'",
      "<\\i\\c*\\s*>"
      // <\i\c*\s*>
    },
    {
      "'''I [dw]on't need \\d{2} apples'''",
      "I [dw]on't need \\d{2} apples",
      // I [dw]on't need \d{2} apples
    },
    {
      "'''\n"
      "The first newline is\n"
      "trimmed in raw strings.\n"
      "   All other whitespace\n"
      "   is preserved.\n"
      "'''\n",
      "The first newline is\n"
      "trimmed in raw strings.\n"
      "   All other whitespace\n"
      "   is preserved.\n"
      // The first newline is~trimmed in raw strings.~   All other whitespace~   is preserved.~
    },
    {
      "'''Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\"'''",
      "Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\""
      // Here are fifteen quotation marks: """""""""""""""
    },
    {
      "\"Here are fifteen apostrophes: '''''''''''''''\"",
      "Here are fifteen apostrophes: '''''''''''''''"
      // Here are fifteen apostrophes: '''''''''''''''
    },
    {
      "''''That,' she said, 'is still pointless.''''",
      "'That,' she said, 'is still pointless.'"
      // 'That,' she said, 'is still pointless.'
    },
  };

  unsigned    i;
  unsigned    len;
  char        szExp[128]; // note: make sure this is large enough for any aStrings[i].szExp

  FlyTestBegin();

  for(i = 0; i < NumElements(aStrings); ++i)
  {
    if(FlyTestVerbose())
    {
      if(i == 0)
        FlyTestPrintf("\n");
      FlyTestPrintf("%u: %s\n", i, aStrings[i].szToml);
    }

    // verify FlyTomlStrLen()
    len = FlyTomlStrLen(aStrings[i].szToml);
    if(len != strlen(aStrings[i].szExp))
    {
      FlyTestPrintf("%u: Bad length on %s, got %u, expected %u\n", i, aStrings[i].szToml, len,
        (unsigned)strlen(aStrings[i].szExp));
      FlyTomlStrCpy(szExp, aStrings[i].szToml, sizeof(szExp));
      FlyTestPrintf(" got str: %s\n", szExp);
      FlyTestFailed();
    }

    // verify escapes convert correctly with FlyTomlStrCPy
    memset(szExp, 0, sizeof(szExp));
    len = FlyTomlStrCpy(szExp, aStrings[i].szToml, sizeof(szExp));
    if(len != strlen(aStrings[i].szExp))
    {
      FlyTestPrintf("%u: Bad len, expected %u, got %u\n", i, 
        (unsigned)strlen(aStrings[i].szExp), len);
      FlyTestFailed();
    }
    if(strcmp(szExp, aStrings[i].szExp) != 0)
    {
      FlyTestPrintf("%u: Bad string converstion. Expected:\n", i);
      FlyTestDump(aStrings[i].szExp, strlen(aStrings[i].szExp));
      FlyTestPrintf("Got:\n");
      FlyTestDump(szExp, strlen(szExp));
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test TOML keys in all their forms using FlyTomlStrLen() and FlyTomlStrCPy()

  See <https://toml.io/en/v1.0.0#keys>
-------------------------------------------------------------------------------------------------*/
void TcTomlKeys(void)
{

  typedef struct
  {
    const char *szToml;
    const char *szExp;
  } tcTomlKeys_t;

  // examples from the TOML keys specification
  // See <https://toml.io/en/v1.0.0#keys>
tcTomlKeys_t aKeys[] =
{
  {
    "key = \"value\"",
    "key"
  },
  {
    "bare_key = \"value\"",
    "bare_key"
  },
  {
    "bare-key = \"value\"\n",
    "bare-key"
  },
  {
    "1234 = \"value\"",
    "1234"
  },
  {
    "\"127.0.0.1\" = \"value\"",
    "127.0.0.1"
  },
  {
    "\"character encoding\" = \"value\"",
    "character encoding"
  },
  {
    u8"\"ʎǝʞ\" = \"value\"",
    u8"ʎǝʞ"
  },
  {
    "'key2' = \"value\"",
    "key2"
  },
  {
    "'quoted \"value\"' = \"value\"",
    "quoted \"value\""
  },
  {
    "\"\" = \"blank\"",
    ""
  },
  {
    "physical.color = \"orange\"",
    "physical.color"
  },
  {
    "physical.shape = \"round\"",
    "physical.shape"
  },
  {
    "site.\"google.com\" = true",
    "site.google.com"
  },
  {
    "fruit.name = \"banana\"     # this is best practice",
    "fruit.name"
  },
  {
    "fruit. color = \"yellow\"    # same as fruit.color",
    "fruit.color"
  },
  {
    "fruit . flavor = \"banana\"   # same as fruit.flavor",
    "fruit.flavor"
  },
  {
    "fruit.apple.smooth = true",
    "fruit.apple.smooth"
  },
  {
    "fruit.orange = 2",
    "fruit.orange",
  },
  {
    "3.14159 = \"pi\"",
    "3.14159"
  },
};

  unsigned    i;
  unsigned    len;
  char        szExp[128]; // note: make sure this is large enough for any aStrings[i].szExp

  FlyTestBegin();

  for(i = 0; i < NumElements(aKeys); ++i)
  {
    if(FlyTestVerbose())
    {
      if(i == 0)
        FlyTestPrintf("\n");
      FlyTestPrintf("%u: %s\n", i, aKeys[i].szToml);
    }

    // verify FlyTomlStrLen()
    len = FlyTomlKeyLen(aKeys[i].szToml);
    if(len != strlen(aKeys[i].szExp))
    {
      FlyTestPrintf("%u: Bad length on %s, got %u, expected %u\n", i, aKeys[i].szToml, len,
        (unsigned)strlen(aKeys[i].szExp));
      FlyTomlKeyCpy(szExp, aKeys[i].szToml, sizeof(szExp));
      FlyTestPrintf(" got str: %s\n", szExp);
      FlyTestFailed();
    }

    // verify escapes convert correctly with FlyTomlStrCPy
    memset(szExp, 0, sizeof(szExp));
    len = FlyTomlKeyCpy(szExp, aKeys[i].szToml, sizeof(szExp));
    if(len != strlen(aKeys[i].szExp))
    {
      FlyTestPrintf("%u: Bad len, expected %u, got %u\n", i, 
        (unsigned)strlen(aKeys[i].szExp), len);
      FlyTestFailed();
    }
    if(strcmp(szExp, aKeys[i].szExp) != 0)
    {
      FlyTestPrintf("%u: Bad string converstion. Expected:\n", i);
      FlyTestDump(aKeys[i].szExp, strlen(aKeys[i].szExp));
      FlyTestPrintf("Got:\n");
      FlyTestDump(szExp, strlen(szExp));
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test bad input
-------------------------------------------------------------------------------------------------*/
void TcTomlFuzz(void)
{
  FlyTestStubbed();
}

/*-------------------------------------------------------------------------------------------------
  Test each function
-------------------------------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
  const char          szName[] = "test_toml";
  const sTestCase_t   aTestCases[] =
  {
    { "TcTomlTypes",            TcTomlTypes },
    { "TcTomlAtox",             TcTomlAtox },
    { "TcTomlStrings",          TcTomlStrings },
    { "TcTomlKeys",             TcTomlKeys },
    { "TcTomlArrayIter",        TcTomlArrayIter },
    { "TcTomlKeyIter",          TcTomlKeyIter },
    { "TcTomlTableIter",        TcTomlTableIter },
    { "TcTomlKeyFind",          TcTomlKeyFind },
    { "TcTomlKeyPathFind",      TcTomlKeyPathFind },
    { "TcTomlTableFind",        TcTomlTableFind },
    { "TcTomlFuzz",             TcTomlFuzz },
  };
  hTestSuite_t        hSuite;
  int                 ret;

  // set up signal handling and logging
  // NedSigSetExit(argv[0], NULL);
  // FlyTestMaskSet(TESTLOG_BASIC);

  FlyTestInit(szName, argc, argv);
  hSuite = FlyTestNew(szName, NumElements(aTestCases), aTestCases);
  FlyTestRun(hSuite);
  ret = FlyTestSummary(hSuite);
  FlyTestFree(hSuite);

  return ret;
}
