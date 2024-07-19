/**************************************************************************************************
  test_markdown.c
  Copyright 2024 Drew Gislason
  License: <https://mit-license.org>
**************************************************************************************************/
#include "FlyTest.h"
#include "FlyFile.h"
#include "FlyStr.h"
#include "FlySignal.h"
#include "FlyMarkdown.h"

/*-------------------------------------------------------------------------------------------------
  Test converting simple markdown to HTML
-------------------------------------------------------------------------------------------------*/
void TcMarkdownSimple(void)
{
  FlyTestBegin();
  FlyTestSkipped();
  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test converting simple markdown to HTML
-------------------------------------------------------------------------------------------------*/
void TcMarkdownLinks(void)
{
  FlyTestBegin();
  FlyTestSkipped();
  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test converting a full featured markdown string to HTML
-------------------------------------------------------------------------------------------------*/
void TcMarkdownAll(void)
{
  FlyTestBegin();
  FlyTestSkipped();
  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test converting markdown image to HTML image
-------------------------------------------------------------------------------------------------*/
void TcMd2HtmlImage(void)
{
  typedef struct
  {
    const char *szMarkdown;
    const char *szHtml;
  } TcMdHtmlImage_t;

  // these tests should pass
  static const TcMdHtmlImage_t aTests[] =
  {
    { "![alt](link \"title\")", "<img src=\"link\" alt=\"alt\" title=\"title\">" },
    { "![](link)",              "<img src=\"link\" alt=\"\">" },  // empty alt OK on images
    { "![FireFly Logo](fireflylogo.png \"w3-circle\")", 
      "<img src=\"fireflylogo.png\" alt=\"FireFly Logo\" class=\"w3-circle\" style=\"width:150px\">"
    },
    {
      "![Math Icon](math.jpeg \"class=\\\"w3-round\\\" style=\\\"width:80%\\\"\")",
      "<img src=\"math.jpeg\" alt=\"Math Icon\" class=\"w3-round\" style=\"width:80%\">"
    },
    {
      "![ Not There %#$@! ](   nothere.jpg     \"title\"   )",
      "<img src=\"nothere.jpg\" alt=\" Not There %#$@! \" title=\"title\">",
    },
    {
      "![ No title  ](no_title.gif)",
      "<img src=\"no_title.gif\" alt=\" No title  \">",
    },
  };

  // fuzz tests should all fail
  static const char *aFuzz[] =
  {
    "abc",                    // not a reference
    "![]()",                  // empty image ref
    "![alt](link\n)",         // cannot cross lines
    "[](#local-ref)",         // empty text on link
    "![alt](\"title\" link)", // title before link
  };
  const char *szMd;
  char        szHtml[256];
  unsigned    i;
  unsigned    len;
  bool_t      fOldDebug;

  if(FlyTestVerbose())
  {
    fOldDebug = fFlyMarkdownDebug;
    fFlyMarkdownDebug = TRUE;
  }

  FlyTestBegin();

  for(i = 0; i < NumElements(aTests); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("\n%i: %s => %s\n", i, aTests[i].szMarkdown, aTests[i].szHtml);

    FlyStrZFill(szHtml, 'A', sizeof(szHtml), sizeof(szHtml) - 1);
    szHtml[sizeof(szHtml) - 1] = '\0';
    szMd = aTests[i].szMarkdown;
    len = FlyMd2HtmlImage(szHtml, sizeof(szHtml), &szMd);
    if(len != strlen(szHtml) || strcmp(szHtml, aTests[i].szHtml) != 0 || *szMd != '\0')
    {
      FlyTestPrintf("\ni=%u: '%s', len %u \n", i, aTests[i].szMarkdown, len);
      FlyTestPrintf("got '%s'\n", szHtml);
      FlyTestPrintf("exp '%s'\n", aTests[i].szHtml);
      FlyTestPrintf("szMd '%s'\n", szMd);
      FlyTestFailed();
    }

    // test with NULL dst to just get length
    FlyStrZFill(szHtml, 'A', sizeof(szHtml), sizeof(szHtml) - 1);
    strcpy(szHtml, "bubba");
    szMd = aTests[i].szMarkdown;
    len = FlyMd2HtmlImage(NULL, sizeof(szHtml), &szMd);
    if(len != strlen(aTests[i].szHtml) || strcmp(szHtml, "bubba") != 0 || *szMd != '\0')
    {
      FlyTestPrintf("\ni=%u: got len %u, exp len %zu with NULL szHtml\n", i, len, strlen(aTests[i].szHtml));
      FlyTestPrintf("szHtml should be bubba: '%s'\n", szHtml);
      FlyTestPrintf("szMd '%s'\n", szMd);
      FlyTestFailed();
    }
  }

  // check for fuzz
  for(i = 0; i < NumElements(aFuzz); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("\n%i: %s\n", i, aFuzz[i]);
    FlyStrZFill(szHtml, 'A', sizeof(szHtml), sizeof(szHtml) - 1);
    strcpy(szHtml, "bubba");
    szMd = aFuzz[i];
    len = FlyMd2HtmlImage(NULL, sizeof(szHtml), &szMd);
    if(len != 0 || szMd != aFuzz[i])
    {
      FlyTestPrintf("\nFUZZ i=%u: Did not return length 0 for markdown '%s'\n", i, aTests[i].szMarkdown);
      FlyTestFailed();
    }
  }

  FlyTestEnd();

  if(FlyTestVerbose())
    fFlyMarkdownDebug = fOldDebug;
}

/*-------------------------------------------------------------------------------------------------
  Test converting markdown image to HTML image
-------------------------------------------------------------------------------------------------*/
void TcMd2HtmlRef(void)
{
  typedef struct
  {
    const char     *szMd;
    const char     *szHtml;
    flyMdRefType_t  refType;
  } TcMdHtmlImage_t;

  TcMdHtmlImage_t aTests[] =
  {
    { "[text](link)", "<a href=\"link\">text</a>", MD_REF_TYPE_REF },
    { "[text2](#link2)",        "<a href=\"#link2\">text2</a>", MD_REF_TYPE_REF },
    { "[drew \"gislason\"](http://www.drewgislason.com/main2.html)",
      "<a href=\"http://www.drewgislason.com/main2.html\">drew \"gislason\"</a>", MD_REF_TYPE_REF },
    { "[^footnote]", "<a href=\"#footnote\">[^footnote]</a>", MD_REF_TYPE_FOOT_REF },
    { "[^footnote]:", "<p id=\"footnote\">", MD_REF_TYPE_FOOTNOTE },
    { "[]()", "", MD_REF_TYPE_NONE },           // not a link
    { "[ Not a link ]", "", MD_REF_TYPE_NONE }, // not a link
  };
  const char     *szRef;
  char            szHtml[256];
  unsigned        i;
  unsigned        len;
  bool_t          fFailed;
  flyMdRefType_t  refType;

  FlyTestBegin();

  for(i = 0; i < NumElements(aTests); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("\n%i: szMd %s, exp szHtml %s\n", i, aTests[i].szMd, aTests[i].szHtml);

    // verify proper reference type
    refType = FlyMd2HtmlIsRef(aTests[i].szMd);
    if(refType != aTests[i].refType)
    {
      FlyTestPrintf("%i: got refType %u, exp %u\n", i, refType, aTests[i].refType);
      FlyTestFailed();
    }

    memset(szHtml, 'A', sizeof(szHtml));
    szHtml[sizeof(szHtml) - 1] = '\0';
    fFailed = FALSE;
    szRef = aTests[i].szMd;
    len = FlyMd2HtmlRef(szHtml, sizeof(szHtml), &szRef);

    // should point to end of aTests[i].szMarkdown string
    if(len != 0 && *szRef != '\0')
      fFailed = TRUE;

    // should NOT advance if not a reference
    if(len == 0 && szRef != aTests[i].szMd)
      fFailed = TRUE;

    // len should always be same as expected HTML
    if(len != strlen(aTests[i].szHtml))
      fFailed = TRUE;

    // Not empty
    if(len && (len != strlen(szHtml) || (strcmp(szHtml, aTests[i].szHtml) != 0)))
      fFailed = TRUE;

    if(fFailed)
    {
      FlyTestPrintf("\n%u: failed len %u, exp len %u, szRef %p, szMd %p\n", i, len, strlen(aTests[i].szHtml), szRef, aTests[i].szMd);
      FlyTestPrintf("got %s\n", szHtml);
      FlyTestPrintf("exp %s\n", aTests[i].szHtml);
      FlyTestFailed();
    }
  }

  // test with NULL dst to just get length
  strcpy(szHtml, "bubba");
  szRef = aTests[0].szMd;
  len = FlyMd2HtmlRef(NULL, sizeof(szHtml), &szRef);
  if(len != strlen(aTests[0].szHtml))
    FlyTestFailed();
  if(strcmp(szHtml, "bubba") != 0)
    FlyTestFailed();

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test converting markdown image to HTML image
-------------------------------------------------------------------------------------------------*/
void TcMd2HtmlList(void)
{
  typedef struct
  {
    const char *szMd;
    const char *szHtml;
  } TcMd2HtmlList_t;

  const TcMd2HtmlList_t aList[] =
  {
    { "* single item\n", "<ul>\r\n<li>single item</li>\r\n</ul>\r\n" },
    { "- Two items\n-   item 2\n", "<ul>\r\n<li>Two items</li>\r\n<li>item 2</li>\r\n</ul>\r\n" },
    { "+ 3 items\n+ list 2\n+ list 3\n", "<ul>\r\n<li>3 items</li>\r\n<li>list 2</li>\r\n<li>list 3</li>\r\n</ul>\r\n" },
    { "1. Numbered three item list\n"
      "99. Item 2\n"
      "2. Item 3\n",

      "<ol>\r\n"
      "<li>Numbered three item list</li>\r\n"
      "<li>Item 2</li>\r\n"
      "<li>Item 3</li>\r\n"
      "</ol>\r\n"
    },
    { "1. Nested List\n"
      "  1. inside 1\n"
      "    - wow 1\n"
      "    - wow 2\n"
      "  9. inside 2\n"
      "  123. inside 3\n"
      "2. Nested 2\n"
      "List ended\n",

      "<ol>\r\n"
      "<li>Nested List\r\n"
      "  <ol>\r\n"
      "  <li>inside 1\r\n"
      "    <ul>\r\n"
      "    <li>wow 1</li>\r\n"
      "    <li>wow 2</li>\r\n"
      "    </ul>\r\n"
      "  </li>\r\n"
      "  <li>inside 2</li>\r\n"
      "  <li>inside 3</li>\r\n"
      "  </ol>\r\n"
      "</li>\r\n"
      "<li>Nested 2</li>\r\n"
      "</ol>\r\n"
    },
    { "* [ ] checkbox 1\n"
      "* [x] checkbox 2\n"
      "* [ ] checkbox 3\n",

      "<ul>\r\n"
      "<li><input type=\"checkbox\" id=\"checkbox-1\"> checkbox 1</li>\r\n"
      "<li><input type=\"checkbox\" id=\"checkbox-2\" checked=\"true\"> checkbox 2</li>\r\n"
      "<li><input type=\"checkbox\" id=\"checkbox-3\"> checkbox 3</li>\r\n"
      "</ul>\r\n"
    },

    { "1. [ ] checkbox 1\n"
      "2. [x] checkbox 2\n"
      "99. [ ] checkbox 3\n",

      "<ol>\r\n"
      "<li><input type=\"checkbox\" id=\"checkbox-1\"> checkbox 1</li>\r\n"
      "<li><input type=\"checkbox\" id=\"checkbox-2\" checked=\"true\"> checkbox 2</li>\r\n"
      "<li><input type=\"checkbox\" id=\"checkbox-3\"> checkbox 3</li>\r\n"
      "</ol>\r\n"
    },
  };
  const char   *psz;
  unsigned      i;
  size_t        htmlLen;
  char          szHtml[256];

  FlyTestBegin();

  // verify output HTML strings match expected
  for(i = 0; i < NumElements(aList); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("\n---%i:\n%s\n=>\n%s\n---\n", i, aList[i].szMd, aList[i].szHtml);

    FlyStrZFill(szHtml, 'A', sizeof(szHtml), sizeof(szHtml)/2);
    psz = aList[i].szMd;
    htmlLen = FlyMd2HtmlList(szHtml, sizeof(szHtml), &psz);
    if(psz == aList[i].szMd)
    {
      FlyTestPrintf("Didn't move past list\n");
      FlyTestFailed();
    }
    if(htmlLen != strlen(aList[i].szHtml) || strcmp(szHtml, aList[i].szHtml) != 0)
    {
      FlyTestPrintf("%u: -- input --\n%s\n", i, aList[i].szMd);
      FlyTestPrintf("-- got --\n%s\n", szHtml);
      FlyTestPrintf("-- expected --\n%s\n", aList[i].szHtml);
      FlyTestFailed();
    }

    // verify we ended up on the right place
    if(i == 4)
    {
      if(strcmp(psz, "List ended\n") != 0)
      {
        FlyTestFailed();
      }
    }
    else
    {
      if(strlen(psz) != 0)
      {
        FlyTestFailed();
      }
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Tests the various "is" functions like FlyMd2HtmlIsBreak() or FlyMd2HtmlIsHeading()
-------------------------------------------------------------------------------------------------*/
void TcMd2HtmlIs(void)
{
  typedef struct
  {
    const char  *szLine;
    bool_t      fIs;
    unsigned    extra;  // extra parameter
  } TcMd2HtmlIs_t;

  typedef struct
  {
    const char     *szLine;
    flyMdRefType_t  refType;
  } TcMd2HtmlIsRef_t;

  const TcMd2HtmlIs_t aIsBreak[] =
  {
    { "  some line with break  ", TRUE },
    { "  some line with break  \n", TRUE },
    { "  some line with break  \r\n", TRUE },
    { "Another line with break      ", TRUE },
    { "Doesn't need a break this line \n", FALSE },
    { "   Nor this line\n", FALSE },
  };
  
  const TcMd2HtmlIs_t aIsCodeBlk[] = 
  {
    { "```\n"
      "this is a code block\n"
      "```\n",
      TRUE, TRUE },

    { "    codeblock line 1\n"
      "\n"
      "    line 2\n"
      "    line 3\n"
      "\n"
      "done\n",
      TRUE, FALSE },

    { "   Just some normal text\n",
      FALSE, FALSE },
  };

  const TcMd2HtmlIs_t aIsHeading[] =
  {
    { "# Title\n", TRUE, 1 },
    { "### Title 3\n", TRUE, 3 },
    { "###### Title 6\n", TRUE, 6 },
    { "  # Not Title\n", FALSE, 0 },
    { "####### Not Title\n", FALSE, 0 },
    { "Is Title\n===\n", TRUE, 1 },
    { "Is Title\n---\n", TRUE, 2 },
  };

  const TcMd2HtmlIs_t aIsList[] =
  {
    { "  * 1\n", TRUE, FALSE },
    { "+ List 2\n", TRUE, FALSE },
    { "   - list 3", TRUE, FALSE },
    { "99. list 4", TRUE, TRUE },
    { "  Not a list", FALSE, FALSE },
    { "*Not a list either*\n", FALSE, FALSE }
  };

  const TcMd2HtmlIsRef_t aIsRef[] =
  {
    { "![alt text](link \"title\")", MD_REF_TYPE_IMAGE },
    { "[ref text](link)", MD_REF_TYPE_REF },
    { "[^footnote reference]", MD_REF_TYPE_FOOT_REF },
    { "[^footnote]:", MD_REF_TYPE_FOOTNOTE },
    { "[a](l)", MD_REF_TYPE_REF },                        // only need 1 character in text and link fields
    { "[^1]", MD_REF_TYPE_FOOT_REF },
    { "![](link \"title\")", MD_REF_TYPE_IMAGE },         // empty alt text OK on images
    { "![](i)", MD_REF_TYPE_IMAGE },                      // empty alt text, no title OK on images
    { "[ref text](link \"title\")", MD_REF_TYPE_NONE },   // non images can't have "title"
    { "[^]", MD_REF_TYPE_NONE },
    { "[]", MD_REF_TYPE_NONE },
    { "[]()", MD_REF_TYPE_NONE },
    { "[ref text] (link)", MD_REF_TYPE_NONE },
    { "[ref text](link\n)", MD_REF_TYPE_NONE },
  };

  unsigned        i;
  unsigned        extra;
  flyMdRefType_t  refType;

  FlyTestBegin();

  // test FlyMd2HtmlIsBreak()
  if(FlyTestVerbose())
    FlyTestPrintf("\nTesting FlyMd2HtmlIsBreak()\n");
  for(i = 0; i < NumElements(aIsBreak); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("  %i: '%s'\n", i, aIsBreak[i].szLine);

    if(FlyMd2HtmlIsBreak(aIsBreak[i].szLine) != aIsBreak[i].fIs)
    {
      FlyTestPrintf("\n%u: %s\n", i, aIsBreak[i].szLine);
      FlyTestFailed();
    }
  }

  // test FlyMd2HtmlIsCodeBlk()
  if(FlyTestVerbose())
    FlyTestPrintf("\nTesting FlyMd2HtmlIsCodeBlk()\n");
  for(i = 0; i < NumElements(aIsCodeBlk); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("  %i: %s\n", i, aIsCodeBlk[i].szLine);

    extra = 0;
    if(FlyMd2HtmlIsCodeBlk(aIsCodeBlk[i].szLine, &extra) != aIsCodeBlk[i].fIs || extra != aIsCodeBlk[i].extra)
    {
      FlyTestPrintf("\n%u: fIsBackTicks %u, %s\n", i, extra, aIsCodeBlk[i].szLine);
      FlyTestFailed();
    }
  }

  // test FlyMd2HtmlIsHeading()
  if(FlyTestVerbose())
    FlyTestPrintf("\nTesting FlyMd2HtmlIsHeading()\n");
  for(i = 0; i < NumElements(aIsHeading); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("  %i: %s\n", i, aIsHeading[i].szLine);

    extra = 0;
    if(FlyMd2HtmlIsHeading(aIsHeading[i].szLine, &extra) != aIsHeading[i].fIs || extra != aIsHeading[i].extra)
    {
      FlyTestPrintf("\n%u: level %u, %s\n", i, extra, aIsHeading[i].szLine);
      FlyTestFailed();
    }
  }

  // test FlyMd2HtmlIsList()
  if(FlyTestVerbose())
    FlyTestPrintf("\nTesting FlyMd2HtmlIsList()\n");
  for(i = 0; i < NumElements(aIsList); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("  %i: %s\n", i, aIsList[i].szLine);

    extra = 0;
    if(FlyMd2HtmlIsList(aIsList[i].szLine, &extra) != aIsList[i].fIs || extra != aIsList[i].extra)
    {
      FlyTestPrintf("\n%u: fIsNumeric %u, %s\n", i, extra, aIsList[i].szLine);
      FlyTestFailed();
    }
  }

  // test FlyMd2HtmlIsRef()
  if(FlyTestVerbose())
    FlyTestPrintf("\nTesting FlyMd2HtmlIsRef()\n");
  for(i = 0; i < NumElements(aIsRef); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("  %i: %s\n", i, aIsRef[i].szLine);

    refType = FlyMd2HtmlIsRef(aIsRef[i].szLine);
    if(refType != aIsRef[i].refType)
    {
      FlyTestPrintf("\n%u: got refType %u, exp refType %u, %s\n", i, refType, aIsRef[i].refType, aIsRef[i].szLine);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyMd2HtmlBold()
-------------------------------------------------------------------------------------------------*/
void TcMd2HtmlEmphasis(void)
{
  typedef struct
  {
    const char     *szMd;
    const char     *szHtml;
    flyMdEmType_t   type;
  } TcMd2HtmlBold_t;

  const TcMd2HtmlBold_t aTests[] =
  {
    { "*italic text*", "<i>italic text</i>", MD_EM_TYPE_ITALICS },
    { "**bold text**", "<b>bold text</b>", MD_EM_TYPE_BOLD },
    { "***bold italic text***", "<b><i>bold italic text</i></b>", MD_EM_TYPE_BOLD_ITAL },
    { "==highlight==", "<mark>highlight</mark>", MD_EM_TYPE_HIGHLIGHT },
    { "~~strike through~~", "<del>strike through</del>", MD_EM_TYPE_STRIKE_THROUGH },
    { "~subscript~", "<sub>subscript</sub>", MD_EM_TYPE_SUB},
    { "^superscript^", "<sup>superscript</sup>", MD_EM_TYPE_SUPER },
    { "***==~~all emphasis~~==***", "<b><i><mark><del>all emphasis</del></mark></i></b>", MD_EM_TYPE_BOLD_ITAL},
    { "abc", "abc", MD_EM_TYPE_NONE },
    { "~/path", "~/path", MD_EM_TYPE_NONE },
    { "* missing close", "* missing close", MD_EM_TYPE_ITALICS },
    { "more **missing close", "more **missing close", MD_EM_TYPE_NONE },
    { "* missing ** close", "* missing ** close", MD_EM_TYPE_ITALICS },
    { "****not bold****", "****not bold****", MD_EM_TYPE_NONE },
  };
  unsigned      i;
  unsigned      htmlLen;
  const char   *psz;
  char          szHtml[128]; // largest HTML in above array, manually determined
  bool_t        fFailed;
  flyMdEmType_t type;

  FlyTestBegin();

  // test FlyMd2HtmlIsEmphasis()
  for(i = 0; i < NumElements(aTests); ++i)
  {
    type = FlyMd2HtmlIsEmphasis(aTests[i].szMd);
    if(type != aTests[i].type)
    {
      FlyTestPrintf("%i: %s, got type %u, exp %u\n", i, aTests[i].szMd, type, aTests[i].type);
      FlyTestFailed();
    }
  }

  // test FlyMd2HtmlEmphasis()
  for(i = 0; i < NumElements(aTests); ++i)
  {
    FlyStrZFill(szHtml, 'A', sizeof(szHtml), sizeof(szHtml) - 1);
    fFailed = FALSE;
    psz = aTests[i].szMd;
    if(FlyTestVerbose())
      FlyTestPrintf("\n%s\n", aTests[i].szMd);
    htmlLen = FlyMd2HtmlTextLine(szHtml, sizeof(szHtml), &psz, FlyStrLineEnd(psz));
    if(htmlLen != strlen(aTests[i].szHtml))
      fFailed = TRUE;
    if(htmlLen && (strcmp(szHtml, aTests[i].szHtml) != 0 || *psz != '\0'))
      fFailed = TRUE;
    if(fFailed)
    {
      FlyTestPrintf("\n%u: htmlLen %u, expLen %zu, psz %s\n", i, htmlLen, strlen(aTests[i].szHtml), psz);
      FlyTestPrintf("szHtml %s\n", szHtml);
      FlyTestPrintf("exp    %s\n", aTests[i].szHtml);
      FlyTestFailed();
    }

    psz = aTests[i].szMd;
    htmlLen = FlyMd2HtmlTextLine(NULL, sizeof(szHtml), &psz, FlyStrLineEnd(psz));
    if(htmlLen != strlen(aTests[i].szHtml))
    {
      FlyTestPrintf("\n%u: htmlLen %u, exp %u\n", i, htmlLen, strlen(aTests[i].szHtml));
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyMd2HtmlCodeLine()
-------------------------------------------------------------------------------------------------*/
void TcMd2HtmlCodeLine(void)
{
  typedef struct
  {
    const char  *szMd;
    const char  *szHtml;
  } TcMd2HtmlCodeLine_t;

  const TcMd2HtmlCodeLine_t aCodeLine[] =
  {
    // { "",                     "<br>\r\n" },
    // { "simple\n",             "simple<br>\r\n" },
    { " a b c\n",             "&nbsp;a b c<br>\r\n" },
    // { "char    szMyVar[];",   "char&nbsp; &nbsp; szMyVar[];<br>\r\n" },
    // { "@param  mything  ",    "@param&nbsp; mything&nbsp; <br>\r\n" },
    // { "   { indented=3;  }",  "&nbsp; &nbsp;{ indented=3;&nbsp; }<br>\r\n" },
    // { "<div> text </div>\n",  "&lt;div> text &lt;/div><br>\r\n" },
  };

  unsigned    i;
  unsigned    htmlLen;
  const char *psz;
  char        szHtml[128]; // largest HTML in above array, manually determined

  FlyTestBegin();

  // test FlyMd2HtmlBold()
  for(i = 0; i < NumElements(aCodeLine); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("\n%u: %s\n", i, aCodeLine[i].szHtml);

    FlyStrZFill(szHtml, 'A', sizeof(szHtml), sizeof(szHtml));
    if(strlen(aCodeLine[i].szHtml) >= sizeof(szHtml))
    {
      FlyTestPrintf("\n%u: szHtml too small\n", i);
      FlyTestFailed();
    }

    psz = aCodeLine[i].szMd;
    if(FlyTestVerbose())
      FlyTestPrintf("\n%s\n", aCodeLine[i].szHtml);
    htmlLen = FlyMd2HtmlCodeLine(szHtml, sizeof(szHtml), &psz);
    if(htmlLen != strlen(aCodeLine[i].szHtml) || strcmp(szHtml, aCodeLine[i].szHtml) != 0)
    {
      FlyTestPrintf("\n%u: htmlLen %u, psz %p\ngot: %s\nexp: %s\n", i, htmlLen, psz, szHtml, aCodeLine[i].szHtml);
      FlyTestFailed();
    }

    psz = aCodeLine[i].szMd;
    htmlLen = FlyMd2HtmlCodeLine(NULL, sizeof(szHtml), &psz);
    if(htmlLen != strlen(aCodeLine[i].szHtml))
    {
      FlyTestPrintf("\n%u: htmlLen %u, exp %u\n", i, htmlLen, strlen(aCodeLine[i].szHtml));
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyMd2HtmlCodeBlk()
-------------------------------------------------------------------------------------------------*/
void TcMd2HtmlCodeBlk(void)
{
  typedef struct
  {
    const char  *szTitle;
    const char  *szMd;
    const char  *szHtml;
    const char  *szColor;
  } TcMd2HtmlCodeBlk_t;

  const TcMd2HtmlCodeBlk_t aCodeBlk[] =
  {
    {
      NULL,
      // markdown
      "```\n"
      "this is a code block\n"
      "```\n",
      // HTML
      "<div class=\"w3-code w3-light-grey notranslate\">\r\n"
      "  this is a code block<br>\r\n"
      "</div>\r\n"
    },

    {
      "Title",
      "```\n"
      // markdown
      "this is a code block\n"
      "```\n",
      // HTML
      "<div class=\"w3-panel w3-card w3-light-grey\">\r\n"
      "  <h5 id=\"Title\">Title</h5>\r\n"
      "  <div class=\"w3-code notranslate\">\r\n"
      "    this is a code block<br>\r\n"
      "  </div>\r\n"
      "</div>\r\n"
    },

    {
      NULL,
      // markdown
      "    // codeblock line 1\n"
      "\n"
      "    while(i < 3)\n"
      "      ++i;\n"
      "\n"
      "done\n",
      // HTML
      "<div class=\"w3-code w3-light-grey notranslate\">\r\n"
      "  // codeblock line 1<br>\r\n"
      "  <br>\r\n"
      "  while(i &lt; 3)<br>\r\n"
      "  &nbsp; ++i;<br>\r\n"
      "</div>\r\n"
    },
    {
      "Red Title",
      "```\n"
      // markdown
      "this is a code block\n"
      "```\n",
      // HTML
      "<div class=\"w3-panel w3-card w3-red\">\r\n"
      "  <h5 id=\"Red-Title\">Red Title</h5>\r\n"
      "  <div class=\"w3-code notranslate\">\r\n"
      "    this is a code block<br>\r\n"
      "  </div>\r\n"
      "</div>\r\n",
      "w3-red"
    },
};
  unsigned    i;
  unsigned    htmlLen;
  const char *psz;
  char        szHtml[256]; // largest HTML in above array, manually determined
  bool_t      fFailed;

  FlyTestBegin();

  // test FlyMd2HtmlBold()
  for(i = 0; i < NumElements(aCodeBlk); ++i)
  {
    FlyStrZFill(szHtml, 'A', sizeof(szHtml), sizeof(szHtml));
    fFailed = FALSE;
    psz = aCodeBlk[i].szMd;
    if(FlyTestVerbose())
      FlyTestPrintf("\n%s\n", aCodeBlk[i].szHtml);
    htmlLen = FlyMd2HtmlCodeBlk(szHtml, sizeof(szHtml), &psz, aCodeBlk[i].szTitle, aCodeBlk[i].szColor);
    if(htmlLen != strlen(aCodeBlk[i].szHtml))
      fFailed = TRUE;
    if(htmlLen && (strcmp(szHtml, aCodeBlk[i].szHtml) != 0))
      fFailed = TRUE;
    if(i != 2 && *psz != '\0')
      fFailed = TRUE;
    if(fFailed)
    {
      FlyTestPrintf("\n%u: htmlLen %u, psz %p, szHtml\n%s\nexp\n%s\n", i, htmlLen, psz, szHtml, aCodeBlk[i].szHtml);
      FlyTestFailed();
    }

    psz = aCodeBlk[i].szMd;
    htmlLen = FlyMd2HtmlCodeBlk(NULL, sizeof(szHtml), &psz, aCodeBlk[i].szTitle, aCodeBlk[i].szColor);
    if(htmlLen != strlen(aCodeBlk[i].szHtml))
    {
      FlyTestPrintf("\n%u: htmlLen %u, exp %u\n", i, htmlLen, strlen(aCodeBlk[i].szHtml));
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test TcMd2HtmlCodeIn()
-------------------------------------------------------------------------------------------------*/
void TcMd2HtmlCodeIn(void)
{
  typedef struct
  {
    const char  *szMd;
    const char  *szHtml;
    const char  *szMore;
  } TcMd2HtmlCodeIn_t;

  const TcMd2HtmlCodeIn_t aCodeIn[] =
  {
    { "`inline code`", "<code class=\"w3-codespan\">inline code</code>", "" },
    { "`missing end code", "<code class=\"w3-codespan\">missing end code</code>", "" },
    { "`some code` more stuff\n", "<code class=\"w3-codespan\">some code</code>", " more stuff\n" },
  };
  unsigned    i;
  unsigned    htmlLen;
  const char *psz;
  char        szHtml[64]; // largest HTML in above array, manually determined
  bool_t      fFailed;

  FlyTestBegin();
 
  // test FlyMd2HtmlBold()
  for(i = 0; i < NumElements(aCodeIn); ++i)
  {
    memset(szHtml, 'A', sizeof(szHtml));
    fFailed = FALSE;
    psz = aCodeIn[i].szMd;
    if(FlyTestVerbose())
      FlyTestPrintf("\n%s\n", aCodeIn[i].szHtml);
    htmlLen = FlyMd2HtmlCodeIn(szHtml, sizeof(szHtml), &psz);
    if(htmlLen != strlen(aCodeIn[i].szHtml))
      fFailed = TRUE;
    if(htmlLen && (strcmp(szHtml, aCodeIn[i].szHtml) != 0 || strcmp(psz, aCodeIn[i].szMore) != 0))
      fFailed = TRUE;
    if(fFailed)
    {
      FlyTestPrintf("\n%u: htmlLen %u, szHtml %s, exp %s, psz %s\n", i, htmlLen, szHtml, aCodeIn[i].szHtml, psz);
      FlyTestFailed();
    }

    psz = aCodeIn[i].szMd;
    htmlLen = FlyMd2HtmlCodeIn(NULL, sizeof(szHtml), &psz);
    if(htmlLen != strlen(aCodeIn[i].szHtml))
    {
      FlyTestPrintf("\n%u: htmlLen %u, exp %u\n", i, htmlLen, strlen(aCodeIn[i].szHtml));
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyMd2HtmlPara()
-------------------------------------------------------------------------------------------------*/
void TcMd2HtmlHeading(void)
{
  typedef struct
  {
    const char  *szMd;
    const char  *szHtml;
    const char  *szW3Color;
  } TcMd2HtmlHeading_t;

  static const TcMd2HtmlHeading_t aHeading[] =
  {
    { "# Big Heading",       "<h1 id=\"Big-Heading\">Big Heading</h1>\r\n" },
    { "##heading",           "<h2 id=\"heading\">heading</h2>\r\n" },
    { "###### Deep heading", "<h6 id=\"Deep-heading\">Deep heading</h6>\r\n" },
    { "####### Not heading", "" },   // only up to 6 levels, 7 is too many
    { " # Not heading",      "" },   // heading must be flush left (no spaces, tabs)
    { "Alternate Heading 1\n===\n",  "<h1 id=\"Alternate-Heading-1\">Alternate Heading 1</h1>\r\n" },
    { "Alternate Heading 2\n---\n",  "<h2 id=\"Alternate-Heading-2\">Alternate Heading 2</h2>\r\n" },
    { "### Color & Heading",  "<h3 id=\"Color-Heading\" class=\"w3-red\">Color & Heading</h3>\r\n", "w3-red" },
  };
  static const TcMd2HtmlHeading_t hdrPtr = { "# Title\nNext Line", "<h1 id=\"Title\">Title</h1>" };
  unsigned    i;
  unsigned    htmlLen;
  const char *psz;
  char        szHtml[128]; // big enough for largest HTML in above array, manually determined
  bool_t      fFailed;

  FlyTestBegin();
 
  // test FlyMd2HtmlBold()
  for(i = 0; i < NumElements(aHeading); ++i)
  {
    FlyStrZFill(szHtml, 'A', sizeof(szHtml), sizeof(szHtml));
    fFailed = FALSE;
    if(FlyTestVerbose())
      FlyTestPrintf("\n--- %u: %s ---\n%s\n", i, aHeading[i].szMd, aHeading[i].szHtml);

    psz = aHeading[i].szMd;
    htmlLen = FlyMd2HtmlHeading(szHtml, sizeof(szHtml), &psz, aHeading[i].szW3Color);
    if(htmlLen != strlen(aHeading[i].szHtml))
      fFailed = TRUE;
    if(htmlLen && strcmp(szHtml, aHeading[i].szHtml) != 0)
      fFailed = TRUE;
    if(htmlLen && *psz != '\0')
    {
      FlyTestPrintf("psz not end of string");
      fFailed = TRUE;;
    }
    if(fFailed)
    {
      FlyTestPrintf("\n%u: htmlLen %u, expLen %u .szMd %p, psz %p\n", i, htmlLen,
        (unsigned)strlen(aHeading[i].szHtml), aHeading[i].szMd, psz);
      FlyTestPrintf("--- szHtml ---\n%s\n--- exp ---\n%s\n---\n", szHtml, aHeading[i].szHtml);
      FlyTestFailed();
    }

    // verify we can get length without a buffer
    psz = aHeading[i].szMd;
    FlyStrZFill(szHtml, 'A', sizeof(szHtml), sizeof(szHtml));
    htmlLen = FlyMd2HtmlHeading(NULL, sizeof(szHtml), &psz, aHeading[i].szW3Color);
    if(htmlLen != strlen(aHeading[i].szHtml) || *szHtml != 'A')
    {
      FlyTestPrintf("\n%u: htmlLen %u, exp %u\n", i, htmlLen, strlen(aHeading[i].szHtml));
      FlyTestFailed();
    }
  }

  // test that the pointer moved to the right place
  psz = hdrPtr.szMd;
  htmlLen = FlyMd2HtmlHeading(szHtml, sizeof(szHtml), &psz, NULL);
  if(strcmp(psz, "Next Line") != 0)
    FlyTestFailed();

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyMd2HtmTextLine()
-------------------------------------------------------------------------------------------------*/
void TcMd2HtmTextLine(void)
{
  typedef struct
  {
    const char *szMd;
    const char *szHtml;
  } TcMd2HtmTextLine_t;

  const TcMd2HtmTextLine_t aTests[] =
  {
    { "Plain: abc", "Plain: abc" },
    { u8"UTF-8: 🔥 and 💦, 💨 and 🌎.", u8"UTF-8: 🔥 and 💦, 💨 and 🌎." },
    { "Escapes: a\\!b\\]c\\~", "Escapes: a!b]c~" },
    { "Entities: a < b & c d > e", "Entities: a &lt; b &amp; c d > e" },
    { "Effects: *italics*, **bold**, ***both***",
      "Effects: <i>italics</i>, <b>bold</b>, <b><i>both</i></b>"
    },
    { "More Effects: `code`, ==highlight==, ~~strike through~~, H~2~O, x^2^",
      "More Effects: <code class=\"w3-codespan\">code</code>, <mark>highlight</mark>, <del>strike through</del>, H<sub>2</sub>O, x<sup>2</sup>"
    },
    {
      "![image](file.png \"title\")",
      "<img src=\"file.png\" alt=\"image\" title=\"title\">"
    },
    {
      "This is an ![image](link \"title\") and this a [reference](#local_ref)",
      "This is an <img src=\"link\" alt=\"image\" title=\"title\"> and this a <a href=\"#local_ref\">reference</a>"
    },
    {
      "<https://duckduckgo.com>, <#local_ref> and <me@mail.com>",
      "<a href=\"https://duckduckgo.com\">https://duckduckgo.com</a>, <a href=\"#local_ref\">#local_ref</a> and <a href=\"mailto:me@mail.com\">me@mail.com</a>"
    }
  };
  unsigned    i;
  size_t      htmlLen;
  const char *szMd;
  char        szHtml[256];


  FlyTestBegin();

  for(i = 0; i < NumElements(aTests); ++i)
  {
    if(FlyTestVerbose())
      printf("\n%i:\nmd:   %s\nhtml: %s\n", i, aTests[i].szMd, aTests[i].szHtml);

    FlyStrZFill(szHtml, 'A', sizeof(szHtml), sizeof(szHtml) - 1);
    szMd = aTests[i].szMd;
    htmlLen = FlyMd2HtmlTextLine(szHtml, sizeof(szHtml), &szMd, szMd + strlen(szMd));
    if(htmlLen != strlen(aTests[i].szHtml) || strcmp(szHtml, aTests[i].szHtml) != 0)
    {
      FlyTestPrintf("\n%i: got len %zu, expLen %zu\n", i, htmlLen, strlen(aTests[i].szHtml));
      FlyTestPrintf("got: %s\nexp: %s\n", szHtml, aTests[i].szHtml);
      FlyTestFailed();
    }

    FlyStrZFill(szHtml, 'A', sizeof(szHtml), sizeof(szHtml) - 1);
    szMd = aTests[i].szMd;
    htmlLen = FlyMd2HtmlTextLine(NULL, sizeof(szHtml), &szMd, szMd + strlen(szMd));
    if(htmlLen != strlen(aTests[i].szHtml) || *szHtml != 'A')
    {
      FlyTestPrintf("\n%i: got len %zu, expLen %zu\n", i, htmlLen, strlen(aTests[i].szHtml));
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyMd2HtmlPara()
-------------------------------------------------------------------------------------------------*/
void TcMd2HtmlPara(void)
{
  typedef struct
  {
    const char  *szMd;
    const char  *szHtml;
  } TcMd2HtmlPara_t;

  const TcMd2HtmlPara_t aPara[] =
  {
    { "simple   paragraph.",           "<p>simple   paragraph.</p>\r\n" },
    { "para with `highlight`x",       "<p>para with <code class=\"w3-codespan\">highlight</code>x</p>\r\n" },
    { "para with [link](mysite.com)", "<p>para with <a href=\"mysite.com\">link</a></p>\r\n" },
    { "![image](file.png \"title\") is here", "<p><img src=\"file.png\" alt=\"image\" title=\"title\"> is here</p>\r\n" },
    { // markdown
      "Some text on this line\n"
      " Some slightly indented text on this line\n"
      "  an equation: 1 < 3  \n"
      "And more text\n"
      "\n",
      // html
      "<p>Some text on this line\r\n"
      " Some slightly indented text on this line\r\n"
      "  an equation: 1 &lt; 3  <br>\r\n"
      "And more text</p>\r\n"
    },
    { "[^footnote]: paragraph", "<p id=\"footnote\">[^footnote]: paragraph</p>\r\n" },
    { "\nNot a paragraph\n", "" },
  };
  unsigned    i;
  unsigned    htmlLen;
  const char *psz;
  char        szHtml[256]; // largest HTML in above array, manually determined
  bool_t      fFailed;

  FlyTestBegin();
 
  // test FlyMd2HtmlBold()
  for(i = 0; i < NumElements(aPara); ++i)
  {
    FlyStrZFill(szHtml, 'A', sizeof(szHtml), sizeof(szHtml));
    fFailed = FALSE;
    if(FlyTestVerbose())
      FlyTestPrintf("\n--- %u: ---\n%s\n---\n", i, aPara[i].szHtml);
    psz = aPara[i].szMd;
    htmlLen = FlyMd2HtmlPara(szHtml, sizeof(szHtml), &psz);
    if(htmlLen != strlen(aPara[i].szHtml))
      fFailed = TRUE;
    if(htmlLen && strcmp(szHtml, aPara[i].szHtml) != 0)
      fFailed = TRUE;
    if(fFailed)
    {
      FlyTestPrintf("\n%u: htmlLen %u .szMd %p, psz %p\n", i, htmlLen, aPara[i].szMd, psz);
      FlyTestPrintf("--- szHtml ---\n%s\n--- exp ---\n%s\n---\n", szHtml, aPara[i].szHtml);
      FlyTestFailed();
    }

    psz = aPara[i].szMd;
    htmlLen = FlyMd2HtmlPara(NULL, sizeof(szHtml), &psz);
    if(htmlLen != strlen(aPara[i].szHtml))
    {
      FlyTestPrintf("\n%u: htmlLen %u, exp %u\n", i, htmlLen, strlen(aPara[i].szHtml));
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyMd2HtmlIsQLink()
-------------------------------------------------------------------------------------------------*/
void TcMd2HtmlQLink(void)
{
  typedef struct
  {
    const char  *szMd;
    bool_t      fIs;
    const char  *szHtml;
  } TcMd2HtmlIsQLink_t;

  static const TcMd2HtmlIsQLink_t aQLink[] =
  {
    { "<https://mysite.com>", TRUE, "<a href=\"https://mysite.com\">https://mysite.com</a>" },
    { "<me@mysite.com>",      TRUE, "<a href=\"mailto:me@mysite.com\">me@mysite.com</a>" },
    { "<mysite.com>",         TRUE, "<a href=\"mysite.com\">mysite.com</a>" },
    { "<file.html>",          TRUE, "<a href=\"file.html\">file.html</a>" },
    { "<file.html#location>", TRUE, "<a href=\"file.html#location\">file.html#location</a>" },
    { "<#location>",          TRUE, "<a href=\"#location\">#location</a>" },
    { "hello",                FALSE, "" },
    { "<code>",               FALSE, "" },
    { "<code class=\"some.thing\">", FALSE, "" },
    { "</code>",              FALSE, "" },
    { "< 99 && 3 > 1",        FALSE, "" },
    { "<mysite.com",          FALSE, "" },
  };

  const char *psz;
  size_t      htmlLen;
  char        szHtml[256];
  unsigned    i;
  bool_t      fFailed;

  FlyTestBegin();

  for(i = 0; i < NumElements(aQLink); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("\n%i: szMd: %s, szHtml: %s\n", i, aQLink[i].szMd, aQLink[i].szHtml);

    if(FlyMd2HtmlIsQLink(aQLink[i].szMd) != aQLink[i].fIs)
    {
      FlyTestPrintf("  %u: %s, exp %u  ", i, aQLink[i].szMd, aQLink[i].fIs);
      FlyTestFailed();
    }

    FlyStrZFill(szHtml, 'A', sizeof(szHtml), sizeof(szHtml) - 1);
    fFailed = FALSE;
    psz = aQLink[i].szMd;
    htmlLen = FlyMd2HtmlQLink(szHtml, sizeof(szHtml), &psz);
    if(htmlLen != strlen(aQLink[i].szHtml))
      fFailed = TRUE;
    if(htmlLen && strcmp(szHtml, aQLink[i].szHtml) != 0)
      fFailed = TRUE;
    if((htmlLen && *psz != '\0') || (!htmlLen && psz != aQLink[i].szMd))
    {
      FlyTestPrintf("\nBad szMd ptr after");
      fFailed = TRUE;
    }
    if(fFailed)
    {
      FlyTestPrintf("\n%u: htmlLen %u, psz: %s\ngot: %s\nexp: %s\n", i, htmlLen, psz, szHtml, aQLink[i].szHtml);
      FlyTestFailed();
    }

    FlyStrZFill(szHtml, 'A', sizeof(szHtml), sizeof(szHtml) - 1);
    psz = aQLink[i].szMd;
    htmlLen = FlyMd2HtmlQLink(NULL, sizeof(szHtml), &psz);
    if(htmlLen != strlen(aQLink[i].szHtml))
    {
      FlyTestPrintf("\n%u: htmlLen %u, exp %u\n", i, htmlLen, strlen(aQLink[i].szHtml));
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test FlyMd2HtmlTable() and FlyMd2HtmlIsTable()
-------------------------------------------------------------------------------------------------*/
void TcMd2HtmlTable(void)
{
  typedef struct
  {
    const char  *szMd;
    const char  *szHtml;
  } TcMd2HtmlTable_t;

  const TcMd2HtmlTable_t aTable[] =
  {
    { .szMd =     // 1 column
      "1|\n"
      "---\n"
      "a|\n",

      .szHtml =
      "<table class=\"w3-table-all\" style=\"width:auto\">\r\n"
      "<tr>\r\n"
      "  <th>1</th>\r\n"
      "</tr>\r\n"
      "<tr>\r\n"
      "  <td>a</td>\r\n"
      "</tr>\r\n"
      "</table>\r\n"
    },

    { .szMd =       // header only (no cell data)
      "1|2\n"
      "---|---\n",

      .szHtml =
      "<table class=\"w3-table-all\" style=\"width:auto\">\r\n"
      "<tr>\r\n"
      "  <th>1</th>\r\n"
      "  <th>2</th>\r\n"
      "</tr>\r\n"
      "</table>\r\n"
    },

    { .szMd =       // 2 columns
      "a|b\n"
      "---|---\n"
      "c|d\n",

      .szHtml =
      "<table class=\"w3-table-all\" style=\"width:auto\">\r\n"
      "<tr>\r\n"
      "  <th>a</th>\r\n"
      "  <th>b</th>\r\n"
      "</tr>\r\n"
      "<tr>\r\n"
      "  <td>c</td>\r\n"
      "  <td>d</td>\r\n"
      "</tr>\r\n"
      "</table>\r\n"
    },

    { .szMd =   // left, center, right alignment
      "Name           | Description       | Age In Years\n"
      ":------------- | :---------------: | --:\n"
      "Joe JoJo       |                   | [29](https://en.wikipedia.org/wiki/Birthday)\n"
      "Jane N. Tarzan | Swings & yodels   | 19\n"
      "Bob            | `Floats` in water | 65\n", 

      .szHtml =
      "<table class=\"w3-table-all\" style=\"width:auto\">\r\n"
      "<tr>\r\n"
      "  <th>Name</th>\r\n"
      "  <th class=\"w3-center\">Description</th>\r\n"
      "  <th class=\"w3-right-align\">Age In Years</th>\r\n"
      "</tr>\r\n"
      "<tr>\r\n"
      "  <td>Joe JoJo</td>\r\n"
      "  <td class=\"w3-center\"></td>\r\n"
      "  <td class=\"w3-right-align\"><a href=\"https://en.wikipedia.org/wiki/Birthday\">29</a></td>\r\n"
      "</tr>\r\n"
      "<tr>\r\n"
      "  <td>Jane N. Tarzan</td>\r\n"
      "  <td class=\"w3-center\">Swings &amp; yodels</td>\r\n"
      "  <td class=\"w3-right-align\">19</td>\r\n"
      "</tr>\r\n"
      "<tr>\r\n"
      "  <td>Bob</td>\r\n"
      "  <td class=\"w3-center\"><code class=\"w3-codespan\">Floats</code> in water</td>\r\n"
      "  <td class=\"w3-right-align\">65</td>\r\n"
      "</tr>\r\n"
      "</table>\r\n"
    },

    { .szMd =         // 2 cell rows, some blank columns
      "a|b|c|d\n"
      "---|---|---|---\n"
      "one cell|||\n"
      "cell2a|cell2b\n"
      "cell3a|cell3b|cell3c|cell3d\n"
      "not cell line",

      .szHtml =
      "<table class=\"w3-table-all\" style=\"width:auto\">\r\n"
      "<tr>\r\n"
      "  <th>a</th>\r\n"
      "  <th>b</th>\r\n"
      "  <th>c</th>\r\n"
      "  <th>d</th>\r\n"
      "</tr>\r\n"
      "<tr>\r\n"
      "  <td>one cell</td>\r\n"
      "  <td></td>\r\n"
      "  <td></td>\r\n"
      "  <td></td>\r\n"
      "</tr>\r\n"
      "<tr>\r\n"
      "  <td>cell2a</td>\r\n"
      "  <td>cell2b</td>\r\n"
      "  <td></td>\r\n"
      "  <td></td>\r\n"
      "</tr>\r\n"
      "<tr>\r\n"
      "  <td>cell3a</td>\r\n"
      "  <td>cell3b</td>\r\n"
      "  <td>cell3c</td>\r\n"
      "  <td>cell3d</td>\r\n"
      "</tr>\r\n"
      "</table>\r\n"
    },

    { .szMd =
      " Not | A | Table\n"    // np "---" alignment line
      "\n",
      .szHtml = ""
    },
    { .szMd =                 // 3 columns
      "a|b|c\n"
      "---|---|---\n"
      "1|2|3",
      .szHtml = 
      "<table class=\"w3-table-all\" style=\"width:auto\">\r\n"
      "<tr>\r\n"
      "  <th>a</th>\r\n"
      "  <th>b</th>\r\n"
      "  <th>c</th>\r\n"
      "</tr>\r\n"
      "<tr>\r\n"
      "  <td>1</td>\r\n"
      "  <td>2</td>\r\n"
      "  <td>3</td>\r\n"
      "</tr>\r\n"
      "</table>\r\n"
    },
    { .szMd =           // too many alignment cells "---" are OK
      "a|b|c\n"
      "---|---|---|---\n"
      "1|2|3",
      .szHtml = 
      "<table class=\"w3-table-all\" style=\"width:auto\">\r\n"
      "<tr>\r\n"
      "  <th>a</th>\r\n"
      "  <th>b</th>\r\n"
      "  <th>c</th>\r\n"
      "</tr>\r\n"
      "<tr>\r\n"
      "  <td>1</td>\r\n"
      "  <td>2</td>\r\n"
      "  <td>3</td>\r\n"
      "</tr>\r\n"
      "</table>\r\n"
    },
    { .szMd =         // not enough alignment cells "---" is NOT OK, processed as paragraphs
      "a|b|c|d\n"
      "---|---|---\n"
      "1|2|3",
      .szHtml = 
      ""
    },
    { .szMd =         // not enough data for columns is OK, they are just blank
      "a|b|c|d\n"
      "---|---|---|---\n"
      "1|2|3",
      .szHtml =
      "<table class=\"w3-table-all\" style=\"width:auto\">\r\n"
      "<tr>\r\n"
      "  <th>a</th>\r\n"
      "  <th>b</th>\r\n"
      "  <th>c</th>\r\n"
      "  <th>d</th>\r\n"
      "</tr>\r\n"
      "<tr>\r\n"
      "  <td>1</td>\r\n"
      "  <td>2</td>\r\n"
      "  <td>3</td>\r\n"
      "  <td></td>\r\n"
      "</tr>\r\n"
      "</table>\r\n"
    },
    {
      .szMd =         // many columns
      "a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z\n"
      "---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---\n"
      "1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|17|18|19|20|21|22|23|24|25|26",
      .szHtml =
      "<table class=\"w3-table-all\" style=\"width:auto\">\r\n"
      "<tr>\r\n"
      "  <th>a</th>\r\n"
      "  <th>b</th>\r\n"
      "  <th>c</th>\r\n"
      "  <th>d</th>\r\n"
      "  <th>e</th>\r\n"
      "  <th>f</th>\r\n"
      "  <th>g</th>\r\n"
      "  <th>h</th>\r\n"
      "  <th>i</th>\r\n"
      "  <th>j</th>\r\n"
      "  <th>k</th>\r\n"
      "  <th>l</th>\r\n"
      "  <th>m</th>\r\n"
      "  <th>n</th>\r\n"
      "  <th>o</th>\r\n"
      "  <th>p</th>\r\n"
      "  <th>q</th>\r\n"
      "  <th>r</th>\r\n"
      "  <th>s</th>\r\n"
      "  <th>t</th>\r\n"
      "  <th>u</th>\r\n"
      "  <th>v</th>\r\n"
      "  <th>w</th>\r\n"
      "  <th>x</th>\r\n"
      "  <th>y</th>\r\n"
      "  <th>z</th>\r\n"
      "</tr>\r\n"
      "<tr>\r\n"
      "  <td>1</td>\r\n"
      "  <td>2</td>\r\n"
      "  <td>3</td>\r\n"
      "  <td>4</td>\r\n"
      "  <td>5</td>\r\n"
      "  <td>6</td>\r\n"
      "  <td>7</td>\r\n"
      "  <td>8</td>\r\n"
      "  <td>9</td>\r\n"
      "  <td>10</td>\r\n"
      "  <td>11</td>\r\n"
      "  <td>12</td>\r\n"
      "  <td>13</td>\r\n"
      "  <td>14</td>\r\n"
      "  <td>15</td>\r\n"
      "  <td>16</td>\r\n"
      "  <td>17</td>\r\n"
      "  <td>18</td>\r\n"
      "  <td>19</td>\r\n"
      "  <td>20</td>\r\n"
      "  <td>21</td>\r\n"
      "  <td>22</td>\r\n"
      "  <td>23</td>\r\n"
      "  <td>24</td>\r\n"
      "  <td>25</td>\r\n"
      "  <td>26</td>\r\n"
      "</tr>\r\n"
      "</table>\r\n"
    },


  };

  unsigned    i;
  unsigned    htmlLen;
  const char *psz;
  char       *szHtml    = NULL;
  bool_t      fIs;

  FlyTestBegin();
 
  // test FlyMd2HtmlBold()
  for(i = 0; i < NumElements(aTable); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("\n%u: %.*s\n", i, (int)FlyStrLineLen(aTable[i].szMd), aTable[i].szMd);

    fIs = strlen(aTable[i].szHtml) ? TRUE : FALSE;
    if(FlyMd2HtmlIsTable(aTable[i].szMd) != fIs)
    {
      FlyTestPrintf("\n%u: expected fIs %u\n", i, fIs);
      FlyTestFailed();
    }

    psz = aTable[i].szMd;
    htmlLen = FlyMd2HtmlTable(NULL, UINT_MAX, &psz);
    if(htmlLen != strlen(aTable[i].szHtml))
    {
      FlyTestPrintf("\n%u: got htmlLen %u, expected %u\n", i, htmlLen, strlen(aTable[i].szHtml));
      FlyTestFailed();
    }

    if(htmlLen)
    {
      szHtml = FlyAlloc(htmlLen + 1);
      if(!szHtml)
        FlyTestFailed();
    }

    FlyStrZFill(szHtml, 'A', htmlLen + 1, htmlLen);
    psz = aTable[i].szMd;
    htmlLen = FlyMd2HtmlTable(szHtml, htmlLen + 1, &psz);
    if(strcmp(szHtml, aTable[i].szHtml) != 0)
    {
      FlyTestPrintf("\n%u: --- expected ---\n%s\n", i, aTable[i].szHtml);
      FlyTestPrintf("--- got ---\n%s\n", szHtml);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test block quotes
-------------------------------------------------------------------------------------------------*/
void TcMd2HtmlBlockQuote(void)
{
  typedef struct
  {
    const char  *szMd;
    const char  *szHtml;
  } TcMd2HtmlBlockQuote_t;

  const TcMd2HtmlBlockQuote_t aTests[] =
  {
    { .szMd =
      ">",
      .szHtml =
      "<div class=\"w3-panel w3-leftbar\">\r\n"
      "  <p></p>\r\n"
      "</div>\r\n"
    },
    { .szMd =
      ">simple one line\n",
      .szHtml =
      "<div class=\"w3-panel w3-leftbar\">\r\n"
      "  <p>simple one line</p>\r\n"
      "</div>\r\n"
    },
    { .szMd =
      "> block quote with\n"
      ">\n"
      "> with Three paragraphs\n"
      ">\n"
      "> One with *italics*, **bold**, ***bold & italics***, ==highlight==, ~~strike through~~ and\n"
      "> `inline code`. Subscript and superscript look like this: H~2~O and x^2^.\n"
      "\n",
      .szHtml =
      "<div class=\"w3-panel w3-leftbar\">\r\n"
      "  <p>block quote with</p>\r\n"
      "  <p>with Three paragraphs</p>\r\n"
      "  <p>One with <i>italics</i>, <b>bold</b>, <b><i>bold &amp; italics</i></b>, <mark>highlight</mark>, <del>strike through</del> and\r\n"
      "  <code class=\"w3-codespan\">inline code</code>. Subscript and superscript look like this: H<sub>2</sub>O and x<sup>2</sup>.</p>\r\n"
      "</div>\r\n"
    },
    { "not a block quote", "" },
  };

  unsigned    i;
  unsigned    htmlLen;
  const char *psz;
  char        szHtml[2048];
  bool_t      fFailed;
  bool_t      fIs;

  FlyTestBegin();

  // test FlyMd2HtmlBold()
  for(i = 0; i < NumElements(aTests); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("\n%u: %.*s\n", i, (int)FlyStrLineLen(aTests[i].szMd), aTests[i].szMd);

    fIs = strlen(aTests[i].szHtml) ? TRUE : FALSE;
    if(FlyMd2HtmlIsBlockQuote(aTests[i].szMd) != fIs)
    {
      FlyTestPrintf("\n%u: bad return FlyMd2HtmlIsBlockQuote(), expected fIs %u\n", i, fIs);
      FlyTestFailed();
    }

    psz = aTests[i].szMd;
    fFailed = FALSE;
    htmlLen = FlyMd2HtmlBlockQuote(szHtml, UINT_MAX, &psz);
    if(htmlLen != strlen(aTests[i].szHtml))
      fFailed = TRUE;
    if(htmlLen && strcmp(szHtml, aTests[i].szHtml) != 0)
      fFailed = TRUE;
    if(fFailed)
    {
      FlyTestPrintf("\n%u: got htmlLen %zu, expected %zu\n", i, htmlLen, strlen(aTests[i].szHtml));
      FlyTestPrintf("--- got ---\n%s\n", szHtml);
      FlyTestPrintf("--- exp ---\n%s\n--- end ---\n", aTests[i].szHtml);
      FlyTestFailed();
    }
  }

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test TcMd2HtmlContent()
-------------------------------------------------------------------------------------------------*/
void TcMd2HtmlContent(void)
{
  typedef struct
  {
    const char  *szMd;
    const char  *szHtml;
  } TcMd2HtmlContent_t;

  const TcMd2HtmlContent_t aContent[] =
  {
    { .szMd =
      "# header\n",
      .szHtml =
      "<h1 id=\"header\">header</h1>\r\n"
    },
    { .szMd =
      "# header1\n"
      "Some paragraph\n"
      "\n"
      "1. A list\n"
      "99. With 2 items\n"
      "\n"
      "    for(i = 0; i < 3; ++i) {\n"
      "      printf(\"hello \");\n"
      "    }\n"
      "    printf(\"world!\\n\");"
      "\n"
      "Some inline things: `code` ![alt](image.png \"title\") with a break  \n"
      "[reference](mysite.com)\n"
      "part of same paragraph: ref <https://duckduckgo.com> or\n"
      "mail <drewgislason@icloud.com?subject=test>\n",

      .szHtml =
      "<h1 id=\"header1\">header1</h1>\r\n"
      "<p>Some paragraph</p>\r\n"
      "<ol>\r\n"
      "<li>A list</li>\r\n"
      "<li>With 2 items</li>\r\n"
      "</ol>\r\n"
      "<div class=\"w3-code w3-light-grey notranslate\">\r\n"
      "  for(i = 0; i &lt; 3; ++i) {<br>\r\n"
      "  &nbsp; printf(\"hello \");<br>\r\n"
      "  }<br>\r\n"
      "  printf(\"world!\\n\");<br>\r\n"
      "</div>\r\n"
      "<p>Some inline things: <code class=\"w3-codespan\">code</code> <img src=\"image.png\" alt=\"alt\" title=\"title\"> with a break  <br>\r\n"
      "<a href=\"mysite.com\">reference</a>\r\n"
      "part of same paragraph: ref <a href=\"https://duckduckgo.com\">https://duckduckgo.com</a> or\r\n"
      "mail <a href=\"mailto:drewgislason@icloud.com?subject=test\">drewgislason@icloud.com?subject=test</a></p>\r\n"
    },
    {
      "Expands the \"~/\" into the actual home folder (from environment $HOME) in place.\n"
      "\n"
      "If the size of szPath is too small, then it can't expand things, so it returns FALSE. A safe size\n"
      "for all paths is PATH_MAX.\n"
      "\n"
      "Examples:\n"
      "\n"
      "This             | Expands to That\n"
      "---------------- | ---------------\n"
      "~/Work/myfile.c  | /Users/me/Work/myfile.c\n"
      "/Users/me/file.c | /Users/me/file.c         (unchanged)\n"
      "~myfile.c        | ~myfile.c                (unchanged)\n",
      "<p>Expands the \"~/\" into the actual home folder (from environment $HOME) in place.</p>\r\n"
      "<p>If the size of szPath is too small, then it can't expand things, so it returns FALSE. A safe size\r\n"
      "for all paths is PATH_MAX.</p>\r\n"
      "<p>Examples:</p>\r\n"
      "<table class=\"w3-table-all\" style=\"width:auto\">\r\n"
      "<tr>\r\n"
      "  <th>This</th>\r\n"
      "  <th>Expands to That</th>\r\n"
      "</tr>\r\n"
      "<tr>\r\n"
      "  <td>~/Work/myfile.c</td>\r\n"
      "  <td>/Users/me/Work/myfile.c</td>\r\n"
      "</tr>\r\n"
      "<tr>\r\n"
      "  <td>/Users/me/file.c</td>\r\n"
      "  <td>/Users/me/file.c         (unchanged)</td>\r\n"
      "</tr>\r\n"
      "<tr>\r\n"
      "  <td>~myfile.c</td>\r\n"
      "  <td>~myfile.c                (unchanged)</td>\r\n"
      "</tr>\r\n"
      "</table>\r\n"
    },
  };
  char       *szHtml = NULL;
  const char *szMdEnd;
  unsigned    htmlLen;
  unsigned    i;

  FlyTestBegin();

  for(i = 0; i < NumElements(aContent); ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("\n%u: %.*s\n", i, (int)FlyStrLineLen(aContent[i].szMd), aContent[i].szMd);

    szMdEnd = aContent[i].szMd + strlen(aContent[i].szMd);
    htmlLen =  FlyMd2HtmlContent(NULL, UINT_MAX, aContent[i].szMd, szMdEnd);
    if(!htmlLen)
      FlyTestFailed();

    szHtml = FlyAlloc(htmlLen + 1);
    if(!szHtml)
      FlyTestFailed();

    htmlLen = FlyMd2HtmlContent(szHtml, htmlLen + 1, aContent[i].szMd, szMdEnd);

    if(htmlLen != strlen(aContent[i].szHtml) || strcmp(szHtml, aContent[i].szHtml) != 0)
    {
      FlyTestPrintf("\n%u: htmlLen %u, expLen %u\n", i, htmlLen, (unsigned)strlen(aContent[i].szHtml));
      FlyTestPrintf("--- expected ---\n");
      FlyTestDump(aContent[i].szHtml, strlen(aContent[i].szHtml) + 1);
      FlyTestPrintf("--- got ---\n");
      FlyTestDump(szHtml, strlen(szHtml) + 1);

      FlyTestPrintf("\n--- expected ---\n%s\n", aContent[i].szHtml);
      FlyTestPrintf("--- got ---\n%s\n", szHtml);
      FlyTestFailed();
    }

    if(szHtml)
    {
      FlyFree(szHtml);
      szHtml = NULL;
    }
  }

  // test leaving max alone. Should NOT overwrite anything after content
  i = 1;
  szMdEnd = aContent[i].szMd + strlen(aContent[i].szMd);
  htmlLen = FlyMd2HtmlContent(NULL, UINT_MAX, aContent[i].szMd, szMdEnd);
  szHtml = FlyAlloc(htmlLen * 2);
  if(!szHtml)
    FlyTestFailed();
  FlyStrZFill(szHtml, 'Q', htmlLen * 2, htmlLen * 2);
  FlyMd2HtmlContent(szHtml, UINT_MAX, aContent[i].szMd, szMdEnd);
  if(szHtml[htmlLen + 1] != 'Q' || szHtml[(htmlLen * 2) - 2] != 'Q')
  {
    FlyTestPrintf("\nhtmlLen = 0x%0x\n", htmlLen);
    FlyTestDump(szHtml, htmlLen * 2);
    FlyTestFailed();
  }

  FlyTestEnd();

  if(szHtml)
    FlyFree(szHtml);
}

void PrintFile(const char *szFile)
{
  const char *szLine = szFile;
  size_t      len;

  while(*szLine)
  {
    len = FlyStrLineLen(szLine);
    FlyTestPrintf("%.*s\n", (int)len, szLine);
    szLine = FlyStrLineNext(szLine);
  }
}

/*-------------------------------------------------------------------------------------------------
  Test converting a file from markdown to HTML
-------------------------------------------------------------------------------------------------*/
void TcMd2HtmlFile(void)
{
  const char szTmpFile[]   = "tmp.html";
  const char *szFileMd     = NULL;
  const char *szFileHtml   = NULL;
  const char *szLineBeg;
  char       *szHtml;
  size_t      len;
  size_t      len2;
  size_t      allocLen;
  size_t      pos;
  unsigned    line, col;

  FlyTestBegin();

  szFileMd   = FlyFileRead("tdata/md2html.md");
  if(!szFileMd)
    FlyTestFailed();
  szFileHtml = FlyFileRead("tdata/md2html.html");
  if(!szFileHtml)
    FlyTestFailed();

  if(FlyTestVerbose())
  {
    FlyTestPrintf("\n--- Converting markdown file md2html.md to file %s ---\n", szTmpFile);
    PrintFile(szFileMd);
    FlyTestPrintf("--- end markdown, len %zu ---\n", strlen(szFileMd));
  }

  if(FlyTestVerbose())
    FlyTestPrintf("len md2html.html: %zu\n", strlen(szFileHtml));
  len = FlyMd2HtmlFile(NULL, UINT_MAX, szFileMd, "md2html.md");
  if(FlyTestVerbose())
    FlyTestPrintf("len tmp.html:     %zu\n", len);

  // do NOT fail yet if wrong size, as we need to compare the two and see whaat's wrong
  if(len == 0)
    FlyTestFailed();

  allocLen = 32 * 1024;
  if(len + 2 > allocLen)
    allocLen = len + 2;
  szHtml = FlyAlloc(allocLen);
  if(!szHtml)
    FlyTestFailed();

  if(FlyTestVerbose())
    FlyTestPrintf("\nConverting markdown to %s\n", szTmpFile);
  FlyStrZFill(szHtml, 'Q', allocLen, allocLen - 1);
  len2 = FlyMd2HtmlFile(szHtml, len * 2, szFileMd, "md2html.md");

  if(FlyTestVerbose())
    FlyTestPrintf("Writing file %s\n", szTmpFile);
  if(!FlyFileWrite(szTmpFile, szHtml))
  {
    FlyTestPrintf("Cannot write file %s\n", szTmpFile);
    FlyTestFailed();
  }

  if(len2 != len)
  {
    FlyTestPrintf("Length differs from NULL szHtml, exp %zu, got %zu\n", len, len2);
    FlyTestFailed();
  }

  if(szHtml[0] == 'Q' || szHtml[len] != '\0' || szHtml[len + 1] != 'Q')
  {
    FlyTestPrintf("Something wrong with 1st or last byte\n", szHtml);
    FlyTestDump(szHtml, len + 1);
    FlyTestFailed();
  }

  // compare contents
  if(strcmp(szHtml, szFileHtml) != 0)
  {
    FlyTestPrintf("\n--- got szHtml ---\n");
    PrintFile(szHtml);
    FlyTestPrintf("\n--- expected szFileHtml ---\n");
    PrintFile(szFileHtml);
    FlyTestPrintf("\n--- end---\n");
    pos = FlyStrWhereDiff(szHtml, szFileHtml, strlen(szFileHtml) + 1);
    line = FlyStrLinePos(szHtml, szHtml + pos, &col);
    szLineBeg = FlyStrLineBeg(szHtml, szHtml + pos);
    FlyTestPrintf("szHtml:%u:%u: Doesn't match HTML: %.*s\n", line, col, (int)FlyStrLineLen(szLineBeg), szLineBeg);
    FlyTestPrintf("See file %s\n", szTmpFile);
    FlyTestFailed();
  }

  FlyTestEnd();

  remove(szTmpFile);
  if(szFileMd)
    FlyFree((void *)szFileMd);
  if(szFileHtml)
    FlyFree((void *)szFileHtml);
}

/*-------------------------------------------------------------------------------------------------
  Test each function
-------------------------------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
  const char          szName[] = "test_markdown";
  const sTestCase_t   aTestCases[] =
  {
    { "TcMd2HtmlImage",       TcMd2HtmlImage },
    { "TcMd2HtmlIs",          TcMd2HtmlIs },
    { "TcMd2HtmlList",        TcMd2HtmlList },
    { "TcMd2HtmlRef",         TcMd2HtmlRef },
    { "TcMd2HtmlEmphasis",    TcMd2HtmlEmphasis },
    { "TcMd2HtmlCodeLine",    TcMd2HtmlCodeLine },
    { "TcMd2HtmlCodeBlk",     TcMd2HtmlCodeBlk },
    { "TcMd2HtmlCodeIn",      TcMd2HtmlCodeIn },
    { "TcMd2HtmlHeading",     TcMd2HtmlHeading },
    { "TcMd2HtmTextLine",     TcMd2HtmTextLine },
    { "TcMd2HtmlPara",        TcMd2HtmlPara },
    { "TcMd2HtmlQLink",       TcMd2HtmlQLink },
    { "TcMd2HtmlTable",       TcMd2HtmlTable },
    { "TcMd2HtmlBlockQuote",  TcMd2HtmlBlockQuote },
    { "TcMd2HtmlContent",     TcMd2HtmlContent },
    { "TcMd2HtmlFile",        TcMd2HtmlFile },
  };
  hTestSuite_t        hSuite;
  int                 ret;

  FlySigSetExit(argv[0], NULL);

  FlyTestInit(szName, argc, argv);
  hSuite = FlyTestNew(szName, NumElements(aTestCases), aTestCases);
  FlyTestRun(hSuite);
  ret = FlyTestSummary(hSuite);
  FlyTestFree(hSuite);

  return ret;
}
