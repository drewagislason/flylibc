/**************************************************************************************************
  FlyToml.c - Parse TOML configuration files
  Copyright 2024 Drew Gislason
  License: MIT <https://mit-license.org>
*///***********************************************************************************************
#include "FlyToml.h"
#include "FlyUtf8.h"

/*!
  @defgroup FlyToml - A simple C/C++ API for processing .toml files.

  The goal of this API is to parse .toml files with minimal code and stack, suitable for embedded
  systems.

  See the TOML specification: <https://toml.io/en/>  
  For other implementations, see: <https://github.com/toml-lang/toml/wiki>  

  Features:

  1. Compliant with TOML v1.0.0, see <https://toml.io/en/>
  2. Usable in multi-threaded environments
  3. Embeddable
    - Minimal stack usage
    - No heap memory allocation
    - No file I/O
    - Minimal code size
    - Optional features to reduce code size (FLYTOML_CFG_FLOAT, FLYTOML_CFG_DATE, TOML_CFG_KEY_MAX)
  4. Parse by iteration or search by keypath if tables/keys are known

  Caveats (for now):

  1. Numbers don't support infinity or nan, as there is no portable C way to do this
  2. No support for double bracketed array of tables [[table]]
  3. For reading/parsing TOML only, no functions for creating TOML
  4. To avoid malloc() or large stack usage, keys longer than FLYTOML_CFG_KEY_MAX will be truncated

  A sample .toml file is below:

  ```toml
  # sample toml file

  my_key = "in unnamed table"

  [simple_types]
  bool = true
  string = "my string"
  `string.literal` = 'C:\Work\folder\'
  int = 42
  long.int = 123_456_789
  pi = 3.14159
  date = 1979-05-27T07:32:00Z
  time = 07:32:00

  # arrays aka ordered list, either of same types or multiple types
  [arrays]
  strings  = ['a "quote"', "/u00dcnicode\nstring" ]
  integers = [+99, 42, 0, -17, 123_456, 0xDEADBEEF, 0xdeadbeef, 0xdead_beef, 0o01234567, 0o755, 0b1101_0110]
  bools    = [true,false]
  floats   = [+1.0, 3.1415, -0.01, 5e+22, 1e06, -2E-2, 6.626e-34, 224_617.445_991_228]
  mixed    = [true, 42, "string"]

  # multi-line strings
  [strings]
  mutli-line = """
    roses are red 
    violets are blue"""
  lines  = '''
  The first newline is
  trimmed in raw strings.
     All other whitespace
     is preserved.
  '''

  # inline table (aka key/value dictionaries or hash table)
  [table]
  inline_table = { key0="string", key1=false, key2=99, array=[0,1,2,3] }
  ```

  Error handling is generally just "not found" or ignored. For example key/value `number = 9s35`
  would result in the number 9.

*/

static const char m_szTripleQuote[] = "\"\"\""; // multi-line string
static const char m_szTripleTick[]  = "'''";    // multi-line string literal

/*------------------------------------------------------------------------------------------------
  Return ptr to next non-blank TOML content (ignores comments, empty lines)

  @param  szLine      a line in a toml file
  @return ptr to line that's not blank or ptr to terminating '\0'
*///-----------------------------------------------------------------------------------------------
static const char * TomlSkipWhite(const char *szLine)
{
  // skip any indent
  szLine = FlyStrSkipWhite(szLine);

  // comments are treated like blank lines
  while(*szLine && (FlyStrLineIsBlank(szLine) || *FlyStrSkipWhite(szLine) == '#'))
  {
    // printf("TomlSkipWhite line: %.*s\n", (unsigned)FlyStrLineLen(szLine), szLine);
    szLine = FlyStrLineNext(szLine);
    szLine = FlyStrSkipWhite(szLine);
  }

  // return ptr to first non-blank content
  return szLine;
}

/*-------------------------------------------------------------------------------------------------
  Return size of quotes with mulit-line or single line strings.
  @return   1 (single quote or tick) or 3 (triple quote)
*///-----------------------------------------------------------------------------------------------
static unsigned TomlQuoteLen(bool_t fMultiline)
{
  return fMultiline ? sizeof(m_szTripleQuote) - 1 : 1;
}

/*-------------------------------------------------------------------------------------------------
  Return ptr to quote or tick or triple-quote or triple-tick string
  @param    fBasic        "basic" or 'literal' string
  @param    fMultiline    multi-line or single-line string
  @return   ptr to ", ', """ or ''''
*///-----------------------------------------------------------------------------------------------
static const char *TomlQuoteStr(bool_t fBasic, bool_t fMultiline)
{
  const char *szQuote = fBasic ? m_szTripleQuote : m_szTripleTick;
  return &szQuote[fMultiline ? 0 : 2];
}

/*-------------------------------------------------------------------------------------------------
  What type of TOML string is this?

  @param    szToml        ptr to TOML "string", 'literal' or  """multi-line"""
  @param    pfBasic       return value TRUE if basic, FALSE if literal
  @param    pfMultiline   return value TRUE if multi-line, FALSE if not
  @return   TRUE if string, FALSE if not
*///-----------------------------------------------------------------------------------------------
static bool_t TomlStrType(const char *szToml, bool_t *pfBasic, bool_t *pfMultiline)
{
  bool_t    fValid = FALSE;

  if(*szToml == '"' || *szToml == '\'')
  {
    fValid = TRUE;
    *pfBasic = (*szToml == '"') ? TRUE : FALSE;
    if(strncmp(szToml, TomlQuoteStr(*pfBasic, TRUE), TomlQuoteLen(TRUE)) == 0)
      *pfMultiline = TRUE;
    else
      *pfMultiline = FALSE;
  }

  return fValid;
}

/*-------------------------------------------------------------------------------------------------
  Converts and copies a TOML string to a UTF-8 string. Returns length of resulting string in szDst

  ppszToml is both and input and a return value. Upon entry, *ppszToml must point to a TOML
  "string", 'literal' or  """multi-line string""" or '''multi-line literal'''. Upon return,
   ppszToml points to the byte after the ending quotes.

  See TOML string specification <https://toml.io/en/v1.0.0#string>

  @param    szDst     where to store UTF-8 string or NULL to just get size
  @param    ppszToml  both an input and a return value. Returns ptr to after string.
  @param    size      size of szDst in bytes, can use UINT_MAX when getting length
  @param    pLen      length in byte of UTF8 converted from TOML
  @return   ptr to after "string"
*///-----------------------------------------------------------------------------------------------
static const char * TomlStrCpy(char *szDst, const char *szToml, unsigned size, unsigned *pLen)
{
  bool_t        fBasic;       // "basic strings" are escaped, 'literal' are not
  bool_t        fMultiline;   // multi-line strings are """triple quotes""" or '''ticks'''
  bool_t        fValid;
  unsigned      totalLen = 0;
  unsigned      thisLen;      // length of a single char (1-4 bytes)
  unsigned      count;
  utf8_t        szUtf8Char[UTF8_MAX];

  // determine "basic string" or 'literal string' or not a string at all
  fValid = TomlStrType(szToml, &fBasic, &fMultiline);

  // determine if multi-line
  if(fValid)
  {
    // sz starts out on string contents, not opening quotes. Skips blank lines/spaces on multi-line
    szToml += TomlQuoteLen(fMultiline);
    if(fMultiline && FlyStrLineIsBlank(szToml))
        szToml = FlyStrLineNext(szToml);

    while(*szToml)
    {
      // at end of string?
      if(*szToml == '"' || *szToml == '\'')
      {
        thisLen = TomlQuoteLen(fMultiline);
        if(strncmp(szToml, TomlQuoteStr(fBasic, fMultiline),  thisLen) == 0)
        {
          count = FlyStrChrCount(szToml, *szToml);
          if(count > thisLen)
          {
            if(totalLen + 1 < size)
            {
              if(szDst)
                szDst[totalLen] = *szToml;
              ++totalLen;
            }
          }
          szToml += count;
          break;
        }
      }

      // line continues on multiline
      if(*szToml == '\\' && fMultiline && fBasic && FlyStrLineIsBlank(szToml + 1))
      {
        // skip all blank lines and indents to next text or end of string
        szToml = FlyStrSkipWhiteEx(szToml + 1);
        continue;
      }

      // assume 1 char in length
      thisLen = 1;

      // handle escape sequence
      if(fBasic && *szToml == '\\')
      {
        szToml = FlyUtf8CharEsc(szUtf8Char, szToml, &thisLen);
        if(szDst && totalLen + thisLen < size)
          strncpy(&szDst[totalLen], szUtf8Char, thisLen);
        totalLen += thisLen;
      }

      else
      {
        if(totalLen + 1 < size)
        {
          if(szDst) 
            szDst[totalLen] = *szToml;
          ++totalLen;
        }
        ++szToml;
      }
    }
  }

  if(!fValid)
    totalLen = 0;
  if(szDst)
    szDst[totalLen] = '\0';
  if(pLen)
    *pLen = totalLen;

  return szToml;
}

/*-------------------------------------------------------------------------------------------------
  Is this a bare key character (A-Za-z0-9_-)?

  @param    c     character
  @return   TRUE if yes, FALSE if no
*///-----------------------------------------------------------------------------------------------
static bool_t TomlIsBareChar(char c)
{
  return (isalnum(c) || c == '-' || c == '_') ? TRUE : FALSE;
}

/*-------------------------------------------------------------------------------------------------
  Is the 1st character of this string a key?

  @param    szTomlKey     ptr to TOML string
  @return   TRUE if yes, FALSE if no
*///-----------------------------------------------------------------------------------------------
static bool_t TomlIsKey(const char *szTomlKey)
{
  return (TomlIsBareChar(*szTomlKey) || *szTomlKey == '"' || *szTomlKey == '\'') ? TRUE : FALSE;
}

/*-------------------------------------------------------------------------------------------------
  Internal function to convert TOML keys, [tables] and "strings" from TOML to UTF-8.

  In addition to converting string from szToml into szDst, also returns length in bytes of
  converted string in `*pLen`, and ptr to the byte after the TOML [table], key or
  "string". If szDst is NULL, the length is still returned in `*pLen`. unless pLen is also NULL.

  For example, the two keys below are identical once converted to UTF-8:

      some.key."\u00b6" = "value"
      "some" .  'key'  .  "\xc2\xb6" = "value"

  See TOML specification <https://toml.io/en/v1.0.0#keys>

  @param    szDst       ptr to buffer to receive UTF-8 string converted from TOML, or NULL
  @param    szToml      ptr to TOML [table] or key = "value"
  @param    size        size of szDst buffer in bytes (1-n)
  @param    pLen        length in bytes of converted string, or NULL if don't need length
  @return   ptr byte after [table], key or "string"
*///-----------------------------------------------------------------------------------------------
static const char * TomlKeyCpy(char *szDst, const char *szToml, unsigned size, unsigned *pLen)
{
  unsigned      len;      // length in bytes (not UTF-8 chars)
  unsigned      thisLen;  // length in bytes
  bool_t        fIsTable;

  // BUGBUG
  // printf("TomlKeyCpy(szDst=%p, szToml=%.*s, size=%u, pLen=%p)\n", szDst, (unsigned)FlyStrLineLen(szToml), szToml, size, pLen);

  // is this a table or a key?
  if(*szToml == '[')
  {
    fIsTable = TRUE;
    ++szToml;
  }
  else
    fIsTable = FALSE;

  len = 0;
  while(*szToml && len + 1 < size)
  {
    // string part
    if(*szToml == '"' || *szToml == '\'')
    {
      szToml = TomlStrCpy(szDst ? &szDst[len] : szDst, szToml, size - len, &thisLen);
      len += thisLen;
    }

    // whitespace
    else if(isblank(*szToml))
      szToml = FlyStrSkipWhite(szToml);

    // dotted key
    else if(*szToml == '.')
    {
      if(len + 1 < size)
      {
        if(szDst)
          szDst[len] = *szToml;
        ++len;
        ++szToml;
      }
    }

    // bare key part
    else if(TomlIsBareChar(*szToml))
    {
      while(TomlIsBareChar(*szToml))
      {
        if(len + 1 < size)
        {
          if(szDst)
            szDst[len] = *szToml;
          ++len;        
        }
        ++szToml;
      }
    }

    // not a valid key character, might be at closing ']'
    // in any case, we're done
    else
    {
      if(fIsTable && *szToml == ']')
        ++szToml;
      break;
    }
  }

  if(szDst)
    szDst[len] = '\0';
  if(pLen)
    *pLen = len;

  // printf("TomlKeyCpy len %u, after `%.*s`\n", len, (unsigned)FlyStrLineLen(szToml), FlyStrNullOk(szToml));

  return szToml;
}

/*-------------------------------------------------------------------------------------------------
  Specifically for key=value pair, will point to the value

  Examples

      key = "value"
      { key_pi = 3.14159, key_int = 42, key_bool = true, key_str = "string" }
      bad_key_value = # comment

  It is up to the caller to deterrmine if the value is valid

  @param  szKey   ptr to a key=value line
  @return ptr to value, which may point to end of line or string
*///-----------------------------------------------------------------------------------------------
static const char * TomlGetValueFromKey(const char *szKey)
{
  const char  *szValue;

  // skip key and equal sign
  szValue = TomlKeyCpy(NULL, szKey, UINT_MAX, NULL);
  szValue = FlyStrSkipWhite(szValue);
  if(*szValue == '=')
    szValue = FlyStrSkipWhite(szValue + 1);

  return szValue;
}

/*-------------------------------------------------------------------------------------------------
  Unknown type, just until next whitespace or comma

  @param  szValue       ptr to receive pValue
  @return ptr to character after value
*///-----------------------------------------------------------------------------------------------
static const char * TomlSkipUnknown(const char *szToml)
{
  while(*szToml && !isspace(*szToml) && *szToml != ',')
    ++szToml;
  return szToml;
}

/*-------------------------------------------------------------------------------------------------
  Internal function to skip the simple value. Unknown will skip all non-

  @param  szValue       ptr to receive pValue
  @param  type          type of value
  @return ptr to character after value
*///-----------------------------------------------------------------------------------------------
static const char * TomlSkipValue(const char *szValue, tomlType_t type)
{
  const char *sz = szValue;

  if(type == TOML_INTEGER)
    sz = FlyStrSkipNumber(szValue);
#if TOML_CFG_FLOAT
  else if(type == TOML_FLOAT)
    sz = FlyStrSkipNumber(szValue);
#endif
#if TOML_CFG_DATE
  else if(type == TOML_DATE)
    sz = TomlDateCpy(szValue, NULL);
#endif
  else if(type == TOML_STRING)
    sz = TomlStrCpy(NULL, szValue, UINT_MAX, NULL);
  else if(type == TOML_UNKNOWN)
    sz = TomlSkipUnknown(szValue);
  else if(type == TOML_TRUE)
    sz = szValue + strlen("true");
  else if(type == TOML_FALSE)
    sz = szValue + strlen("false");
  else if(type == TOML_INLINE_TABLE)
    sz = FlyStrLineNext(szValue); // TODO: allow multi-line and nested inline tables

  return sz;
}

/*-------------------------------------------------------------------------------------------------
  Returns ptr to TOML key/value and value type in `*pKey`.

  @param    szKey   ptr to TOML key in a key/value pair, e.g. "key = 42"
  @param    pKey    Return key/value/type if valid key/value pair input
  @return   ptr to byte after "key = value" or NULL if invalid key/value
*///-----------------------------------------------------------------------------------------------
static const char * TomlKeyGet(const char *szKey, tomlKey_t *pKey)
{
  const char   *szAfter = NULL;
  const char   *szValue;

  memset(pKey, 0, sizeof(*pKey));
  if(TomlIsKey(szKey))
  {
    // if there is a value, 
    szValue = TomlGetValueFromKey(szKey);
    if(szValue)
    {
      pKey->szKey   = szKey;
      pKey->szValue = szValue;
      pKey->type    = FlyTomlType(szValue);
      szAfter       = TomlSkipValue(szValue, pKey->type);
    }
  }

  return szAfter;
}

/*-------------------------------------------------------------------------------------------------
  Find the key by case sensitive name in this [table] or { inline = "table" }

  Note: if finding a key in the unnamed table at top of TOML file, use ptr to the top of the TOML
  file for the szTomlTable parameter.

  For example, when finding "top_key", use ptr to top of file for szTomlTable.  
  When finding key "hello", use ptr to "[table1]".  
  When finding key "b", use ptr to "{" after inline_table.

      # a TOML file
      top_key = 42

      [table1]
      hello = "world"
      inline_table = { a=1, b=2, c=3 }

  @param  szTomlTable   a TOML [table] or { inline = "table" } or top of TOML file
  @param  szKeyName     key in TOML format (e.g. bare."dotted.key")
  @param  pKey          returned key/value/type if found, zeroed if not found
  @return TRUE if key was found, FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlyTomlKeyFind(const char *szTomlTable, const char *szKeyName, tomlKey_t *pKey)
{
  const char *szToml;
  char        szKey1[TOML_CFG_KEY_MAX];
  char        szKey2[TOML_CFG_KEY_MAX];
  bool_t      fFound = FALSE;

  // convert TOML key to UTF-8
  TomlKeyCpy(szKey1, szKeyName, sizeof(szKey1), NULL);
  szToml = szTomlTable;
  while(TRUE)
  {
    szToml = FlyTomlKeyIter(szToml, pKey);
    if(szToml == NULL)
      break;
    TomlKeyCpy(szKey2, pKey->szKey, sizeof(szKey2), NULL);
    if(strcmp(szKey1, szKey2) == 0)
    {
      fFound = TRUE;
      break;
    }
  }

  if(!fFound)
    memset(pKey, 0, sizeof(*pKey));

  return fFound;
}

/*!------------------------------------------------------------------------------------------------
  Find a value in the TOML file based on a case-sensitive key path. Keys are in TOML form.

  To look for keys before named tables, use ":key" implying the empty table name. That is, there
  MUST be at least 1 colon in the path string or it will not be found.

  Below is an example TOML file and some examples of key paths to find various values.

      # a TOML file
      no_table_key = "chair"
      inline1 = { a = 0x61, b = 0x62 }

      [table1]
      key = 42
      hello = "world"
      array = ["zero", "one", "two", "three"]

      [table2]
      "math" = { pi = 3.14159, e = 2.71828 }

  keypath            | value found
  ------------------ | ---------
  :no_table_key      | "chair" (TOML_STRING)
  :inline1           | { a = 0x61, b = 0x62 } (TOML_INLINE_TABLE)
  :inline1:b         | 0x62 (TOML_INTEGER)
  table1:hello       | "world" (TOML_STRING)
  table1:array       | ["zero", "one", "two", "three"] (TOML_ARRAY)
  table1:array:3     | "three" (TOML_STRING)
  table2:math:pi     | 3.14159 (TOML_FLOAT)
  table2:math:e      | 2.71828 (TOML_FLOAT)

  Note: this function assumes no table names contain a colon ':' character in them.

  @param  szToml      a TOML file ('\0' terminated string of entire TOML file)
  @param  szKeyPath   keypath to TOML value, separated by colons ':'
  @param  pKey        returned key/value/type if found, undefined if not
  @return TRUE if found, FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlyTomlKeyPathFind(const char *szTomlFile, const char *szKeyPath, tomlKey_t *pKey)
{
  const char   *pszTable;
  const char   *pszEnd;
  const char   *pszBeg;
  unsigned      len;
  bool_t        fTopLevel;
  bool_t        fWorked = TRUE;
  tomlKey_t     key;
  char          szTableName[TOML_CFG_KEY_MAX];

  // printf("FlyTomlKeyPathFind(%s)\n", szKeyPath);

  pszBeg = szKeyPath;
  pszEnd = strchr(szKeyPath, ':');
  pszTable = szTomlFile;
  fTopLevel = TRUE;
  while(pszEnd)
  {
    memset(&key, 0, sizeof(key));

    // get table name in path
    len = pszEnd - pszBeg;
    if(len > TOML_CFG_KEY_MAX - 1)
      len = TOML_CFG_KEY_MAX - 1;
    strncpy(szTableName, pszBeg, len);
    szTableName[len] = '\0';
    // printf("  table '%s', fTopLevel %u\n", szTableName, fTopLevel);

    if(fTopLevel)
    {
      pszTable = FlyTomlTableFind(szTomlFile, szTableName);
    }

    else
    {
      fWorked = FlyTomlKeyFind(*pszTable == '\0' ? szTomlFile : pszTable, szTableName, &key);
      if(fWorked && key.type != TOML_INLINE_TABLE)
      {
        // printf("  failed 1\n");
        fWorked = FALSE;
        break;
      }
      pszTable = key.szValue;
    }

    if(!pszTable)
    {
      // printf("  failed 2\n");
      fWorked = FALSE;
      break;
    }

    // check for another table in the path
    pszBeg = pszEnd + 1;
    pszEnd = strchr(pszBeg, ':');
    fTopLevel = FALSE;
  }

  if(fWorked)
  {
    // printf("  here3 key %s, table '%.*s'\n", pszBeg, (unsigned)FlyStrLineLen(pszTable), pszTable);
    fWorked  = FlyTomlKeyFind(*pszTable == '\0' ? szTomlFile : pszTable, pszBeg, pKey);
    // if(!fWorked)
    //   printf("  failed 3\n");
  }

  return fWorked;
}

/*!------------------------------------------------------------------------------------------------
  Find a [table] in the TOML file by case sensitive name.

  Specify table without brackets, that is "table1" not "[table1]".

  TOML table name rules apply, so "table2   .  subtable" and "table2.\"subtable\"" are equivalent.
  See <https://toml.io/en/v1.0.0#table>

  As an example, the following TOML file has 3 tables, root or "", "table1" and "table2.subtable".

      # a TOML file with root table
      root_table_key = "value"

      [table1]
      table1_key = true

      [table2.subtable]
      subtable_key = 42

  @param  szTomlFile    a '\0' terminated TOML file in UTF-8 string form
  @param  szTableName   table to find, use "" for root table
  @return ptr to header or NULL if not found
*///-----------------------------------------------------------------------------------------------
const char *  FlyTomlTableFind(const char *szTomlFile, const char *szTableName)
{
  const char   *pszTableFound         = NULL;
  const char   *pszTable;
  char          szTable1[TOML_CFG_KEY_MAX];
  char          szTable2[TOML_CFG_KEY_MAX];

  TomlKeyCpy(szTable1, szTableName, sizeof(szTable1), NULL);
  pszTable = NULL;
  while(TRUE)
  {
    pszTable = FlyTomlTableIter(szTomlFile, pszTable);
    if(!pszTable)
      break;

    // first table might be unnamed table at top of file
    TomlKeyCpy(szTable2, (*pszTable == '\0') ? szTomlFile : pszTable, sizeof(szTable2), NULL);
    if(strcmp(szTable1, szTable2) == 0)
    {
      pszTableFound = pszTable;
      break;
    }
  }

  return pszTableFound;
}

/*!------------------------------------------------------------------------------------------------
  Get the next value from an array. Returns ptr to next value in array or NULL if last item.

  For exmaple, in the array below, the ptr returned would be on "string". The `*pValue` returned
  be the value 1 of type TOML_INTEGER.

      [1, "string", 42, true, 99]

  @param  szTomlArray   ptr to a TOML array
  @param  pValue        ptr to receive pValue and type (e.g. "value" and TOML_STRING)
  @return ptr to next value or NULL  if no more values
*///-----------------------------------------------------------------------------------------------
const char * FlyTomlArrayIter(const char *szTomlArray, tomlValue_t *pValue)
{
  const char   *pszValue = NULL;

  if(*szTomlArray && (*szTomlArray != ']'))
  {
    if(*szTomlArray == '[')
      ++szTomlArray;
    pszValue = TomlSkipWhite(szTomlArray);

    // found the index caller is looking for? return it
    pValue->szValue = pszValue;
    pValue->type    = FlyTomlType(pszValue);;

    // skip to next item in the array, but stop at unexpected characters
    pszValue = TomlSkipWhite(TomlSkipValue(pszValue, pValue->type));
    while(*pszValue == ',')
      pszValue = TomlSkipWhite(pszValue + 1);
  }

  return pszValue;
}

/*!------------------------------------------------------------------------------------------------
  Get the first/next key-value pair from a TOML `[table]` or `{ inline="table" }`.

  Pass in a pointer (in parameter `szTomlTable`) to a TOML header `[table]` or `{ inline="table" }`
  to get the 1st key in the table. In the case of the unnamed root table, pass in the TOML file.

  The returned value is the "next" key, or NULL if no more keys.

  @example FlyTomlKeyIter with a `[table]`

      void print_keys(const char *pszTomlFile, const char *szTableName)
      {
        const char   *pszTable;
        const char   *pszIter = NULL;
        tomlKey_t     key;
        const char    szKey[32];

        pszTable = FlyTomlTableFind(szTomlFile, szTableName);
        if(pszTable)
          pszIter = FlyTomlKeyIter(pszTable, &key);
        while(pszKey)
        {
          FlyTomlKeyCpy(szKey, key.szKey, sizeof(szKey));
          printf("%.*s\n", szKey);
          pszIter = FlyTomlKeyIter(pszIter, &key);
        }
      }

  @param  szTomlTable   ptr to TOML `[table]`, `{ inline = "table" }` for 
  @param  pKey          ptr to receive key/value pair
  @return ptr to next key or NULL if no more keys
*///-----------------------------------------------------------------------------------------------
const char *FlyTomlKeyIter(const char *szTomlTable, tomlKey_t *pKey)
{
  const char *pszKey    = szTomlTable;

  // may be at top of file, get to 1st key
  pszKey = TomlSkipWhite(pszKey);

  // inline table
  if(*pszKey == '{')
    pszKey = TomlSkipWhite(pszKey + 1);

  // main header [table] or top of TOM file, keys are line oriented
  else
  {
    if(*pszKey == '[')
      pszKey = FlyStrLineNext(pszKey);
    pszKey = TomlSkipWhite(pszKey);
  }

  // printf("pszKey %p, %02x\n", pszKey, pszKey ? *pszKey : 0xaa);

  // we should be on a key
  if(!TomlIsKey(pszKey))
    pszKey = NULL;
  else
  {
    pszKey = TomlKeyGet(pszKey, pKey);
    if(pszKey)
    {
      pszKey = TomlSkipWhite(pszKey);
      if(*pszKey == ',')
        pszKey = FlyStrSkipWhite(pszKey + 1);
    }
  }

  // BUGBUG
  // if(pszKey)
  //   printf("FlyTomlKeyIter after: %.*s\n", (unsigned)FlyStrLineLen(pszKey), pszKey);
  // printf("here99 pszKey %p\n", pszKey); 

  // return ptr to next key or NULL
  return pszKey;  
}

/*!------------------------------------------------------------------------------------------------
  Find the first/next header `[table]` in the TOML file.

  This simply points to a position in the TOML file. No translation of the TOML takes place. This
  table can be used for calls that require a table, for example FlyTomlKeyIter().

  Note: the root (unnamed) table is special. This function, `FlyTomlTableIter()` will return an
  empty string "" for the root table. In this case, use the top of the file for the table when
  parameter in functions like FlyTomlKeyIter().

  The parameter szPrevTable must be NULL the 1st time FlyTomlTableIter() is called on a particular
  TOML file. Subsequent calls must pass the returned value in addition to the TOML file.

  THe TOML file below contains tables, the unnamed root table "", "[table1]" and "[table2]", which
  will be returned in that order.

      # a TOML file with unnamed root table
      root_table_key = "value"

      [table1]
      table1_key = true

      [table2]
      table2_key = 42

  @param  szTomlFile   ptr to top of TOML file or ptr to a [table] in the TOML file
  @param  szPrevTable  returned from previous call to FlyTomlTableIter() or NULL
  @return ptr to next [table] or NULL if no more tables
*///-----------------------------------------------------------------------------------------------
const char * FlyTomlTableIter(const char *szTomlFile, const char *szPrevTable)
{
  static const char   szTableEmpty[]  = "";
  const char         *pszTable        = NULL;
  const char         *szToml;
  bool_t              fFoundKeys;

  if(szPrevTable == NULL || *szPrevTable == '\0')
    szToml = szTomlFile;
  else
  {
    szToml = szPrevTable;
    if(*szPrevTable == '[')
      szToml = TomlSkipWhite(FlyStrLineNext(szToml));
  }
 
  // only valid if at least 1 key/value pair
  fFoundKeys = FALSE;
  while(*szToml)
  {
    // skip blank and comment lines
    szToml = TomlSkipWhite(szToml);

    // at end of file
    if(*szToml == '\0')
      break;

    // at first/next table
    else if(*szToml == '[')
    {
      pszTable = szToml;
      if(szPrevTable == NULL && fFoundKeys)
        pszTable = szTableEmpty;
      break;
    }

    fFoundKeys = TRUE;
    szToml = TomlSkipWhite(FlyStrLineNext(szToml));    
  }

  // file just has keys, so it has an implicit unnamed table
  if(pszTable == NULL && szPrevTable == NULL && fFoundKeys)
    pszTable = szTableEmpty;

  return pszTable;
}

/*-------------------------------------------------------------------------------------------------
  Returns TRUE if this an unnamed root table as returned by `FlyTomlTableIter()` or
  `FlyTomlTableFind()`.

  @param  szTomlTable    ptr to a table
  @return TRUE if a root table, FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlyTomlTableIsRoot(const char *szTomlTable)
{
  return (szTomlTable && *szTomlTable == '\0');
}

/*-------------------------------------------------------------------------------------------------
  Initializes a key so that pKey->szKey is NULL

  @param  pKey    ptr to a TOML key structure
  @return none
*///-----------------------------------------------------------------------------------------------
void FlyTomlKeyInit(tomlKey_t   *pKey)
{
  memset(pKey, 0, sizeof(*pKey));
}

/*-------------------------------------------------------------------------------------------------
  Initializes a value so that pValue->szValue is NULL

  @param  pValue    ptr to a TOML value structure
  @return none
*///-----------------------------------------------------------------------------------------------
void FlyTomlValueInit(tomlValue_t *pValue)
{
  memset(pValue, 0, sizeof(*pValue));
}

/*-------------------------------------------------------------------------------------------------
  Return the value type.

  Types include:

      TOML_UNKNOWN
      TOML_FALSE
      TOML_TRUE,
      TOML_INTEGER
      TOML_FLOAT
      TOML_DATE
      TOML_STRING
      TOML_ARRAY
      TOML_INLINE_TABLE

  @param  szTomlValue     ptr to value
  @return type of the value (e.g. TOML_INTEGER or TOML_STRING)
*///-----------------------------------------------------------------------------------------------
tomlType_t FlyTomlType(const char *szTomlValue)
{
  tomlType_t  type;

  if(*szTomlValue == '"' || *szTomlValue == '\'')
    type = TOML_STRING;
  else if(isdigit(*szTomlValue) || (((*szTomlValue == '+' || *szTomlValue == '-')) && isdigit(szTomlValue[1])))
  {
#if TOML_CFG_FLOAT    // TODO: TOML_FLOAT type
#endif
    type = TOML_INTEGER;
  }
  else if(strncmp(szTomlValue, "true", 4) == 0)
    type = TOML_TRUE;
  else if(strncmp(szTomlValue, "false", 5) == 0)
    type = TOML_FALSE;
  else if(*szTomlValue == '[')
    type = TOML_ARRAY;
  else if(*szTomlValue == '{')
    type = TOML_INLINE_TABLE;
#if TOML_CFG_DATE
  // TODO: TOML_DATE type, see RFC 3339 and TOML specification
#endif
  else
    type = TOML_UNKNOWN;

  return type;
}

/*-------------------------------------------------------------------------------------------------
  Get the value from a key/value pair, given TOML key = value string

  @param  szTomlKey   ptr to TOML key string, e.g. key = "value"
  @param  pValue      ptr to tomlValue_t struct to receive value and type
  @return TRUE if worked, FALSE if bad key="value" pair
*///-----------------------------------------------------------------------------------------------
bool_t FlyTomlValue(const char *szTomlKey, tomlValue_t *pValue)
{
  tomlKey_t key;
  bool_t    fValid = FALSE;

  if(TomlKeyGet(szTomlKey, &key) != NULL)
  {
    pValue->szValue = key.szValue;
    pValue->type    = key.type;
    fValid = TRUE;
  }

  return fValid;
}

/*-------------------------------------------------------------------------------------------------
  Copy and convert a TOML key (perhaps with escapes) to a '\0' terminated UTF-8 string.

  See TOML key specification <https://toml.io/en/v1.0.0#keys>

  If size is too small, the converted key will be truncated but will still have the '\0'
  terminator.

  Note: this also works with `[table]` strings, as they follow the same rules as keys.

  @param    szDst       destination string to receive TOML string, or NULL to just get length
  @param    szTomlKey   ptr to TOML key (e.g. key."dotted" = "value")
  @param    size        size of szDst buffer in bytes (1 - UINT_MAX)
  @return   length of key converted to UTF-8 in bytes (not chars)
-------------------------------------------------------------------------------------------------*/
unsigned FlyTomlKeyCpy(char *szDst, const char *szTomlKey, unsigned size)
{
  unsigned  len = 0;
  TomlKeyCpy(szDst, szTomlKey, size, &len);
  return len;
}

/*-------------------------------------------------------------------------------------------------
  Get length TOML key as if it were converted UTF-8, in bytes (not chars).

  See TOML key specification <https://toml.io/en/v1.0.0#keys>

  @param    szTomlKey   ptr to TOML key (e.g. key."dotted" = "value")
  @return length of TOML key in bytes (not chars).
-------------------------------------------------------------------------------------------------*/
unsigned FlyTomlKeyLen(const char *szTomlKey)
{
  unsigned  len = 0;
  TomlKeyCpy(NULL, szTomlKey, UINT_MAX, &len);
  return len;
}

/*-------------------------------------------------------------------------------------------------
  Copy and convert a TOML string (perhaps with escapes) to a '\0' terminated UTF-8 string.

  Similar to strncpy(), but returns length of copied string, not a pointer

  See TOML string specification <https://toml.io/en/v1.0.0#string>

  Some examples of strings are:

      "\"Quoted\" string\tSan Jos\u00E9 style\n"
      'No "transformation" \n here'
      """\
      roses are red \
      Violets are blue"""
      '''I [dw]on't need \d{2} apples'''

  If size is too small, the converted string will be truncated but will still have the '\0'
  terminator.

  @param    szDst       destination string to receive TOML string, or NULL to just get length
  @param    szTomlStr   ptr to TOML "string" 
  @param    size        size of szDst buffer in bytes (1 - UINT_MAX)
  @return   length of UTF-8 string in bytes (not chars) resulting from TOML string
-------------------------------------------------------------------------------------------------*/
unsigned FlyTomlStrCpy(char *szDst, const char *szTomlStr, unsigned size)
{
  unsigned  len = 0;
  TomlStrCpy(szDst, szTomlStr, size, &len);
  return len;
}

/*-------------------------------------------------------------------------------------------------
  Get length in bytes of TOML "string" as if it were converted UTF-8. Handles escapes.

  Can be used to allocate a string of the proper size for FlyTomlStrCpy().

  See TOML string specification <https://toml.io/en/v1.0.0#string>  
  See also TOML key specification <https://toml.io/en/v1.0.0#keys>  

  @param    szTomlStr   ptr to TOML "string" 
  @return length in bytes of TOML "string" as if it were converted UTF-8. Handles escapes.
-------------------------------------------------------------------------------------------------*/
unsigned FlyTomlStrLen(const char *szTomlStr)
{
  unsigned  len = 0;
  TomlStrCpy(NULL, szTomlStr, UINT_MAX, &len);
  return len;
}

/*!------------------------------------------------------------------------------------------------
  Returns the start of the TOML string, that is the character after the opening `"` or `'`.

  @param  szTomlBool   ptr to true or false
  @return TRUE or FALSE
*///-----------------------------------------------------------------------------------------------
const char *  FlyTomlPtr(const char *szTomlStr)
{
  if(*szTomlStr == '"' || *szTomlStr == '\'')
    ++szTomlStr;
  return szTomlStr;
}

/*!------------------------------------------------------------------------------------------------
  Converts a TOML boolean string (true or false) to a boolean variable.

  @param  szTomlBool   ptr to true or false
  @return TRUE or FALSE
*///-----------------------------------------------------------------------------------------------
bool_t FlyTomlAtoBool(const char *szTomlBool)
{
  return (strncmp(szTomlBool, "true", 4) == 0) ? TRUE : FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Converts a TOML integer to a long. Stop on first illegal character.

  Examples of integers

      int1 = +99
      int2 = 42
      int3 = 0
      int4 = -17
      int5 = 1_000
      int6 = 5_349_221
      int7 = -9223372036854775808   # LONG_MIN
      int8 = 9223372036854775807    # LONG_MAX
      hex1 = 0xDEADBEEF
      hex2 = 0xdeadbeef
      hex3 = 0xdead_beef
      oct1 = 0o01234567
      oct2 = 0o755
      bin1 = 0b11010110

  @param  szTomlInteger   ptr to TOML value
  @return long integer
*///-----------------------------------------------------------------------------------------------
long FlyTomlAtol(const char *szTomlInteger)
{
  const   char *sz  = szTomlInteger;
  long    val       = 0;
  long    thisVal;
  long    base      = 10;
  bool_t  fNegative = FALSE;

  // determine sign
  if(*sz == '+')
    ++sz;
  else if(*sz == '-')
  {
    ++sz;
    fNegative = TRUE;
  }

  // determine base
  if(*sz == '0')
  {
    if(sz[1] == 'x')
    {
      base = 16;
      sz += 2;
    }
    else if(sz[1] == 'o')
    {
      base = 8;
      sz += 2;
    }
    else if(sz[1] == 'b')
    {
      base = 2;
      sz += 2;
    }
  }

  // determine numberic value
  while(TRUE)
  {
    if(*sz == '_')
    {
      ++sz;
      continue;
    }

    // don't allow illegal digits
    if(base == 16)
    {
      if(!isxdigit(*sz))
        break;
    }
    else if(!isdigit(*sz) || (base == 2 && *sz >= '2') || (base == 8 && *sz >= '8'))
      break;

    // determine digit value
    if(base == 16 && !isdigit(*sz))
      thisVal = 10 + toupper(*sz) - 'A';
    else
      thisVal = *sz - '0';

    // multiply the number
    val = (val * base) + thisVal;
    ++sz;
  }

  if(fNegative)
    val = -1L * val;

  return val;
}

#if TOML_CFG_DATE
/*!------------------------------------------------------------------------------------------------
  Returns a date/time based on a TOML date/time input.

  See TOML specification: <https://toml.io/en/v1.0.0#offset-date-time>

  @param  szTomlBool   ptr to true or false
  @return TRUE or FALSE
*///-----------------------------------------------------------------------------------------------
bool_t FlyTomlAtoDate(const char *szTomlDate, struct tm *pDate)
{
  // STUBSTUB: FlyTomlAtoDate()
  (void)szTomlDate;
  (void)pDate;
  return FALSE;
}
#endif

#if TOML_CFG_FLOAT
/*!------------------------------------------------------------------------------------------------
  Converts a TOML float to a double. Stop on first illegal character.

  Examples of floats:

    flt1 = +1.0
    flt2 = 3.1415
    flt3 = -0.01
    flt4 = 5e+22
    flt5 = 1e06
    flt6 = -2E-2
    flt7 = 6.626e-34

  @param  szTomlFloat   ptr to TOML value
  @return length of key or contents of "string" or 0 if not valid key or "string"
*///-----------------------------------------------------------------------------------------------
double FlyTomlAtof(const char *szTomlFloat)
{
  // STUBSTUB: FlyTomlAtof()
  (void)szTomlFloat;
  return FALSE;
}
#endif // TOML_CFG_FLOAT
