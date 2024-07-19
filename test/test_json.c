/*!************************************************************************************************
  @ingroup flytest
  @file FlyTestJson.c

  Test cases for JSON subsystem

  Copyright 2022 Drew Gislason

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
  associated documentation files (the "Software"), to deal in the Software without restriction,
  including without limitation the rights to use, copy, modify, merge, publish, distribute,
  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or
  substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*///***********************************************************************************************
#include "Fly.h"
#include "FlyTest.h"
#include "FlyJson.h"
#include "FlyMem.h"

typedef struct
{
  long      number;
  bool_t    fBool;
  char     *sz;
} sTjcObj_t;

const char szJson2Simple[] = "{\"one\":1}";
const char szJson2SimplePretty[] = "{\n    \"one\": 1\n}";

const char szJson2Complex[] = {
  "{\"bool\":true,"
  "\"number\":5,"
  "\"string\":\"string\","
  "\"boolArray\":["
  "true,"
  "false,"
  "true"
  "],"
  "\"numberArray\":["
  "-1,"
  "2,"
  "3,"
  "2147483647"
  "],"
  "\"stringArray\":["
  "\"the quick brown fox jumped over the lazy dog's back\","
  "\"every good boy does fine\""
  "],"
  "\"emptyArray\":[],"
  "\"obj\":{"
  "\"one\":1,"
  "\"two\":true,"
  "\"three\":\"three\""
  "},"
  "\"objArray\":["
  "{"
  "\"a\":1,"
  "\"b\":true,"
  "\"c\":\"three\""
  "},"
  "{"
  "\"a\":4,"
  "\"b\":false,"
  "\"c\":\"six\""
  "}"
  "]"
  "}"
};

const char szJson2ComplexPretty[] = {
  "{\n"
  "    \"bool\": true,\n"
  "    \"number\": 5,\n"
  "    \"string\": \"string\",\n"
  "    \"boolArray\": [\n"
  "        true,\n"
  "        false,\n"
  "        true\n"
  "    ],\n"
  "    \"numberArray\": [\n"
  "        -1,\n"
  "        2,\n"
  "        3,\n"
  "        2147483647\n"
  "    ],\n"
  "    \"stringArray\": [\n"
  "        \"the quick brown fox jumped over the lazy dog's back\",\n"
  "        \"every good boy does fine\"\n"
  "    ],\n"
  "    \"emptyArray\": [],\n"
  "    \"obj\": {\n"
  "        \"one\": 1,\n"
  "        \"two\": true,\n"
  "        \"three\": \"three\"\n"
  "    },\n"
  "    \"objArray\": [\n"
  "        {\n"
  "            \"a\": 1,\n"
  "            \"b\": true,\n"
  "            \"c\": \"three\"\n"
  "        },\n"
  "        {\n"
  "            \"a\": 4,\n"
  "            \"b\": false,\n"
  "            \"c\": \"six\"\n"
  "        }\n"
  "    ]\n"
  "}"
};

/*-------------------------------------------------------------------------------------------------
  Helper to TestJsonGetComplex()
-------------------------------------------------------------------------------------------------*/
bool_t TjObjCmp(const char *szObj, const sTjcObj_t *pObj)
{
  bool_t          fWorked = TRUE;
  size_t          count;
  size_t          i;
  flyJsonType_t   type;
  const char     *szKey;
  const char     *szValue;

  count = FlyJsonGetCount(szObj);
  if(count != 3)
    fWorked = FALSE;
  for(i = 0; fWorked && i < count; ++i)
  {
    szKey = FlyJsonGetKey(szObj, i);
    if(!szKey)
      fWorked = FALSE;
    else
    {
      szValue = FlyJsonGetValuePtr(szKey, &type);

      if(FlyJsonStrCmp("a", szKey) == 0)
      {
        if(type != FLYJSON_NUMBER || FlyJsonGetNumber(szValue) != pObj->number)
          fWorked = FALSE;
      }

      else if(FlyJsonStrCmp("b", szKey) == 0)
      {
        if(type != FLYJSON_BOOL || FlyJsonGetBool(szValue) != pObj->fBool)
          fWorked = FALSE;
      }

      else if(FlyJsonStrCmp("c", szKey) == 0)
      {
        if(type != FLYJSON_STRING || FlyJsonStrCmp(pObj->sz, szValue) != 0)
          fWorked = FALSE;
      }

      else
        fWorked = FALSE;
    }
  }

  return fWorked;
}

/*-------------------------------------------------------------------------------------------------
  Print out failure with diagnostics
-------------------------------------------------------------------------------------------------*/
void TjLogFailed(size_t len, const char *sz, size_t expLen, const char *szExp)
{
  FlyTestPrintf("Failed, got len %zu:\n%s\n, expected len %zu:\n%s\n", len, sz, expLen, szExp);
}

/*-------------------------------------------------------------------------------------------------
  Helper to TestJsonPutComplex()
-------------------------------------------------------------------------------------------------*/
size_t TjPutComplex(char *sz, size_t maxSize, bool_t fPretty)
{
  hFlyJson_t  hJson         = NULL;
  size_t      len           = 0;
  unsigned    i;
  bool_t      fBool         = TRUE;
  long        number        = 5;
  char        szString[]    = "string";
  bool_t      boolArray[]   = { TRUE, FALSE, TRUE };
  long        numberArray[] = { -1, 2, 3, INT32_MAX };
  char       *stringArray[] = {
                "the quick brown fox jumped over the lazy dog's back",
                "every good boy does fine" };
  long        one           = 1;
  bool_t      fTwo          = TRUE;
  char        szThree[]     = "three";
  sTjcObj_t   aObj[]        = {{1, TRUE, "three"}, {4, FALSE, "six"}};

  hJson = FlyJsonNew(sz, maxSize, fPretty);
  if(FlyJsonIsHandle(hJson))
  {
    len += FlyJsonPutBegin(hJson, FLYJSON_OBJ);
    len += FlyJsonPut(hJson, "bool", FLYJSON_BOOL, &fBool);
    len += FlyJsonPut(hJson, "number", FLYJSON_NUMBER, &number);
    len += FlyJsonPut(hJson, "string", FLYJSON_STRING, szString);
    len += FlyJsonPut(hJson, "boolArray", FLYJSON_ARRAY, NULL);
    for(i=0; i < NumElements(boolArray); ++i)
      len += FlyJsonPutScalar(hJson, FLYJSON_BOOL, &boolArray[i]);
    len += FlyJsonPutEnd(hJson, FLYJSON_ARRAY);
    len += FlyJsonPut(hJson, "numberArray", FLYJSON_ARRAY, NULL);
    for(i=0; i < NumElements(numberArray); ++i)
      len += FlyJsonPutScalar(hJson, FLYJSON_NUMBER, &numberArray[i]);
    len += FlyJsonPutEnd(hJson, FLYJSON_ARRAY);
    len += FlyJsonPut(hJson, "stringArray", FLYJSON_ARRAY, NULL);
    for(i=0; i < NumElements(stringArray); ++i)
      len += FlyJsonPutScalar(hJson, FLYJSON_STRING, stringArray[i]);
    len += FlyJsonPutEnd(hJson, FLYJSON_ARRAY);
    len += FlyJsonPut(hJson, "emptyArray", FLYJSON_ARRAY, NULL);
    len += FlyJsonPutEnd(hJson, FLYJSON_ARRAY);
    len += FlyJsonPut(hJson, "obj",   FLYJSON_OBJ,    NULL);
    len += FlyJsonPut(hJson, "one",   FLYJSON_NUMBER, &one);
    len += FlyJsonPut(hJson, "two",   FLYJSON_BOOL,   &fTwo);
    len += FlyJsonPut(hJson, "three", FLYJSON_STRING, szThree);
    len += FlyJsonPutEnd(hJson, FLYJSON_OBJ);
    len += FlyJsonPut(hJson, "objArray", FLYJSON_ARRAY, NULL);
    for(i=0; i < NumElements(aObj); ++i)
    {
      len += FlyJsonPutBegin(hJson, FLYJSON_OBJ);
      len += FlyJsonPut(hJson, "a", FLYJSON_NUMBER, &aObj[i].number);
      len += FlyJsonPut(hJson, "b", FLYJSON_BOOL,   &aObj[i].fBool);
      len += FlyJsonPut(hJson, "c", FLYJSON_STRING,  aObj[i].sz);
      len += FlyJsonPutEnd(hJson, FLYJSON_OBJ);
    }
    len += FlyJsonPutEnd(hJson, FLYJSON_ARRAY);
    len += FlyJsonPutEnd(hJson, FLYJSON_OBJ);

    FlyJsonFree(hJson);
  }

  return len;
}

/*!------------------------------------------------------------------------------------------------
  Tests FlyJsonPut() function simply

  @ingroup    ned_test
  @returns    none
*///-----------------------------------------------------------------------------------------------
void TcJsonPutSimple(void)
{
  hFlyJson_t  hJson   = NULL;
  size_t      len     = 0;
  long        number  = 1;
  char        szJson[sizeof(szJson2SimplePretty)];

  FlyTestBegin();

  // test compact version
  hJson = FlyJsonNew(szJson, sizeof(szJson), FALSE);
  if(!FlyJsonIsHandle(hJson))
    FlyTestFailed();
  len = 0;
  len += FlyJsonPutBegin(hJson, FLYJSON_OBJ);
  len += FlyJsonPut(hJson, "one", FLYJSON_NUMBER, &number);
  len += FlyJsonPutEnd(hJson, FLYJSON_OBJ);
  if(len != strlen(szJson2Simple))
  {
    TjLogFailed(len, szJson, strlen(szJson2Simple), szJson2Simple);
    FlyTestFailed();
  }
  if(strcmp(szJson, szJson2Simple) != 0)
  {
    TjLogFailed(len, szJson, strlen(szJson2Simple), szJson2Simple);
    FlyTestFailed();
  }
  FlyJsonFree(hJson);

  // test pretty version
  hJson = FlyJsonNew(szJson, sizeof(szJson), TRUE);
  if(!FlyJsonIsHandle(hJson))
    FlyTestFailed();
  len = 0;
  len += FlyJsonPutBegin(hJson, FLYJSON_OBJ);
  len += FlyJsonPut(hJson, "one", FLYJSON_NUMBER, &number);
  len += FlyJsonPutEnd(hJson, FLYJSON_OBJ);
  if(len != strlen(szJson2SimplePretty))
  {
    TjLogFailed(len, szJson, strlen(szJson2SimplePretty), szJson2SimplePretty);
    FlyTestFailed();
  }
  if(strcmp(szJson, szJson2SimplePretty) != 0)
  {
    TjLogFailed(len, szJson, strlen(szJson2SimplePretty), szJson2SimplePretty);
    FlyTestFailed();
  }
  FlyJsonFree(hJson);

  FlyTestEnd();
}

/*!------------------------------------------------------------------------------------------------
  Tests FlyJsonPut() function with example of each type and nesting

  @ingroup    ned_test
  @returns    none
*///-----------------------------------------------------------------------------------------------
void TcJsonPutComplex(void)
{
  size_t      len     = 0;
  size_t      len2    = 0;
  char       *szJson  = NULL;

  FlyTestBegin();

  // test compact version
  len = TjPutComplex(NULL, sizeof(szJson), FALSE);
  if(len != strlen(szJson2Complex))
  {
    TjLogFailed(len, szJson, strlen(szJson2Complex), szJson2Complex);
    FlyTestFailed();
  }
  szJson = FlyAlloc(len + 1);
  if(!szJson)
    FlyTestFailed();
  len2 = TjPutComplex(szJson, sizeof(szJson), FALSE);
  if(len2 != len || strcmp(szJson, szJson2Complex) != 0)
  {
    TjLogFailed(len, szJson, strlen(szJson2Complex), szJson2Complex);
    FlyTestFailed();
  }
  FlyFree(szJson);

  // test pretty version
  len = TjPutComplex(NULL, sizeof(szJson), TRUE);
  if(len != strlen(szJson2ComplexPretty))
  {
    TjLogFailed(len, szJson, strlen(szJson2ComplexPretty), szJson2ComplexPretty);
    FlyTestFailed();
  }
  szJson = FlyAlloc(len + 1);
  if(!szJson)
    FlyTestFailed();
  len2 = TjPutComplex(szJson, sizeof(szJson), TRUE);
  if(len2 != len || strcmp(szJson, szJson2ComplexPretty) != 0)
  {
    TjLogFailed(len, szJson, strlen(szJson2ComplexPretty), szJson2ComplexPretty);
    FlyTestFailed();
  }
  FlyFree(szJson);

  FlyTestEnd();
}

/*!------------------------------------------------------------------------------------------------
  Get a simple JSON file

  @ingroup    ned_test
  @returns    none
*///-----------------------------------------------------------------------------------------------
void TcJsonGetSimple(void)
{
  const char         *szValue;
  const char         *szKey;
  const char         *szObj;
  flyJsonType_t       type      = FLYJSON_INVALID;
  const char          szOne[]   = "one";

  FlyTestBegin();

  // verify we can get the number
  szObj = FlyJsonGetObj(szJson2SimplePretty);
  if(!szObj || FlyJsonGetCount(szObj) != 1)
    FlyTestFailed();
  szKey = FlyJsonGetKey(szObj, 0);
  if(!szKey || FlyJsonStrCmp(szOne, szKey) != 0)
    FlyTestFailed();
  szValue = FlyJsonGetValuePtr(szKey, &type);
  if(FlyTestVerbose())
    FlyTestPrintf("szJson2SimplePretty %p, szValue %p, type %u\n", szJson2SimplePretty, szValue, type);
  if(!szValue)
    FlyTestFailed();
  if(type != FLYJSON_NUMBER)
    FlyTestFailed();
  if(FlyJsonGetNumber(szValue) != 1)
    FlyTestFailed();

  FlyTestEnd();
}

/*!------------------------------------------------------------------------------------------------
  Get a complex JSON file object

  @ingroup    ned_test
  @returns    none
*///-----------------------------------------------------------------------------------------------
void TcJsonGetComplex(void)
{
  bool_t          fBool         = TRUE;
  long            number        = 5;
  char            szString[]    = "string";
  bool_t          boolArray[]   = { TRUE, FALSE, TRUE };
  long            numberArray[] = { -1, 2, 3, INT32_MAX };
  char           *stringArray[] = {
                    "the quick brown fox jumped over the lazy dog's back",
                    "every good boy does fine" };
  long            one           = 1;
  bool_t          fTwo          = TRUE;
  char            szThree[]     = "three";
  sTjcObj_t       aObj[]        = {{1, TRUE, "three"}, {4, FALSE, "six"}};
  size_t          i, j;
  size_t          count;
  size_t          count2;
  const char     *szKey;
  const char     *szObj;
  const char     *szValue;
  const char     *szArray;
  flyJsonType_t   type;
  const char     *aszKeys[] = { "bool", "number", "string", "boolArray", "numberArray",
                                "stringArray", "emptyArray", "obj", "objArray" };
  char            szTmp[20];

  FlyTestBegin();

  szObj = FlyJsonGetObj(szJson2ComplexPretty);
  if(!szObj)
    FlyTestFailed();
  count = FlyJsonGetCount(szObj);
  if(FlyTestVerbose())
    FlyTestPrintf("count = %zu\n", count);
  if(count != NumElements(aszKeys))
    FlyTestFailed();
  for(i = 0; i < count; ++i)
  {
    szKey = FlyJsonGetKey(szObj, i);
    if(!szKey)
      FlyTestFailed();
    szValue = FlyJsonGetValuePtr(szKey, &type);

    // verify key
    if(FlyTestVerbose())
      FlyTestPrintf("  [%zu] key=%.16s value=%.8s, type %u\n", i, szKey, szValue, type);
    if(FlyJsonStrCmp(aszKeys[i], szKey) != 0)
    {
      FlyJsonStrNCpy(szTmp, szKey, sizeof(szTmp) - 1);
      szTmp[sizeof(szTmp) - 1] = '\0';
      FlyTestPrintf("Bad key \"%s\", expected \"%s\"\n", szTmp, aszKeys[i]);
      FlyTestFailed();
    }

    if(FlyJsonStrCmp("bool", szKey) == 0)
    {
      if(!szValue || type != FLYJSON_BOOL)
        FlyTestFailed();
      if(FlyJsonGetBool(szValue) != fBool)
        FlyTestFailed();
    }

    else if(FlyJsonStrCmp("number", szKey) == 0)
    {
      if(!szValue || type != FLYJSON_NUMBER)
        FlyTestFailed();
      if(FlyJsonGetNumber(szValue) != number)
        FlyTestFailed();
    }

    else if(FlyJsonStrCmp("string", szKey) == 0)
    {
      if(!szValue || type != FLYJSON_STRING)
        FlyTestFailed();
      if(FlyJsonStrCmp(szString, szValue) != 0)
        FlyTestFailed();
    }

    else if(FlyJsonStrCmp("boolArray", szKey) == 0)
    {
      if(!szValue || type != FLYJSON_ARRAY)
        FlyTestFailed();
      szArray = szValue;
      count2 = FlyJsonGetCount(szArray);
      for(j = 0; j < count2; ++j)
      {
        szValue = FlyJsonGetScalar(szArray, j, &type);
        if(type != FLYJSON_BOOL || !szValue || FlyJsonGetBool(szValue) != boolArray[j])
        {
          FlyTestPrintf("  [%zu], szValue=%.6s, got %u, expected %u, got type %u, expected %u\n", 
            j, szValue, FlyJsonGetBool(szValue), boolArray[j], type, FLYJSON_BOOL);
          FlyTestFailed();
        }
      }
    }

    else if(FlyJsonStrCmp("numberArray", szKey) == 0)
    {
      if(!szValue || type != FLYJSON_ARRAY)
        FlyTestFailed();
      szArray = szValue;
      count2 = FlyJsonGetCount(szArray);
      for(j = 0; j < count2; ++j)
      {
        szValue = FlyJsonGetScalar(szArray, j, &type);
        if(type != FLYJSON_NUMBER || !szValue || FlyJsonGetNumber(szValue) != numberArray[j])
          FlyTestFailed();
      }
    }

    else if(FlyJsonStrCmp("stringArray", szKey) == 0)
    {
      if(!szValue || type != FLYJSON_ARRAY)
        FlyTestFailed();
      szArray = szValue;
      for(j = 0; j < FlyJsonGetCount(szArray); ++j)
      {
        szValue = FlyJsonGetScalar(szArray, j, &type);
        if(type != FLYJSON_STRING || !szValue || FlyJsonStrCmp(stringArray[j], szValue) != 0)
          FlyTestFailed();
      }
    }

    else if(FlyJsonStrCmp("emptyArray", szKey) == 0)
    {
      szArray = szValue;
      if(FlyJsonGetCount(szArray) != 0)
        FlyTestFailed();
    }

    else if(FlyJsonStrCmp("obj", szKey) == 0)
    {
      if(!szValue || type != FLYJSON_OBJ)
        FlyTestFailed();
      szArray = szValue;
      if(FlyJsonGetCount(szArray) != 3)
        FlyTestFailed();
      szKey = FlyJsonGetKey(szArray, 0);
      szValue = FlyJsonGetValuePtr(szKey, &type);
      if(type != FLYJSON_NUMBER || !szValue || FlyJsonGetNumber(szValue) != one)
        FlyTestFailed();
      szKey = FlyJsonGetKey(szArray, 1);
      szValue = FlyJsonGetValuePtr(szKey, &type);
      if(type != FLYJSON_BOOL || !szValue || FlyJsonGetBool(szValue) != fTwo)
        FlyTestFailed();
      szKey = FlyJsonGetKey(szArray, 2);
      szValue = FlyJsonGetValuePtr(szKey, &type);
      if(type != FLYJSON_STRING || !szValue || FlyJsonStrCmp(szThree, szValue) != 0)
        FlyTestFailed();
    }

    else if(FlyJsonStrCmp("objArray", szKey) == 0)
    {
      if(!szValue || type != FLYJSON_ARRAY)
        FlyTestFailed();
      szArray = szValue;
      count2 = FlyJsonGetCount(szArray);
      if(count2 != 2)
        FlyTestFailed();
      for(j = 0; j < count2; ++j)
      {
        szValue = FlyJsonGetScalar(szArray, j, &type);
        if(type != FLYJSON_OBJ || !szValue)
          FlyTestFailed();
        if(!TjObjCmp(szValue, &aObj[j]))
          FlyTestFailed();
      }
    }

    else
    {
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*!------------------------------------------------------------------------------------------------
  Test function TcJsonIsJson()

  @ingroup    ned_test
  @returns    none
*///-----------------------------------------------------------------------------------------------
void TcJsonIsJson(void)
{
  const char szNotJson1[] = "{}";
  const char szNotJson2[] = " \"key without\" : \"array or obj\" ";
  const char szIsJson[]   = "[ \"key with\" : \"array\" ]";

  FlyTestBegin();

  if(!FlyJsonIsJson(szJson2Simple))
    FlyTestFailed();

  if(!FlyJsonIsJson(szIsJson))
    FlyTestFailed();

  if(!FlyJsonIsJson(szJson2Complex))
    FlyTestFailed();

  if(FlyJsonIsJson(szNotJson1))
    FlyTestFailed();

  if(FlyJsonIsJson(szNotJson2))
    FlyTestFailed();

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test each function
*///-----------------------------------------------------------------------------------------------
int main(int argc, const char *argv[])
{
  const char          szName[] = "test_json";
  const sTestCase_t   aTestCases[] =
  {
    { "TcJsonPutSimple",  TcJsonPutSimple },
    { "TcJsonPutComplex", TcJsonPutComplex },
    { "TcJsonGetSimple",  TcJsonGetSimple },
    { "TcJsonGetComplex", TcJsonGetComplex },
    { "TcJsonIsJson",     TcJsonIsJson },
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
