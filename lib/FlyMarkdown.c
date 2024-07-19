/**************************************************************************************************
  FlyMarkdown.c - A simple API for converting markdown to HTML
  Copyright 2024 Drew Gislason
  license: <https://mit-license.org>
**************************************************************************************************/
#include "FlyMarkdown.h"
#include "FlyStr.h"
#include "FlyFile.h"

// DEBUGGING
bool_t fFlyMarkdownDebug = 0;
const char *g_szMd;

// if all debugging is off, then no printfs
#define MD_DEBUG_CONTENT  0     // show each major element as HTML is built, 0, 1 or 2
#define MD_DEBUG_ALTLINK  0     // part of reference debugging
#define MD_DEBUG_NCAT     0     // show copying lines
#define MD_DEBUG_REF      0     // debug all things reference (images, ref, quick-link, footnotes, etc...)
#define MD_DEBUG_PARA     0
#define MD_DEBUG_LIST     0
#define MD_DEBUG_CODE     0     // code blocks
#define MD_DEBUG_TABLE    0
#define MD_DEBUG_BLOCK_QUOTE  0

/*!
  @defgroup FlyMarkdown A C API for converting markdown into HTML.

  High-level Features:

  - Converts entire markdown documents to static HTML
  - Uses the W3.CSS modern, responsive, mobile first CSS framework for prettier output
  - Converts individual elements of markdown to static HTML
  - Does not require the heap; does NOT use `malloc()`
  - Works with common browsers (e.g. Chrome, Firefox, Edge, IE, Safari, Opera, Brave)

  Markdown comes in many variations. This one uses <https://www.markdownguide.org/cheat-sheet/>.

  For a discussion on why W3CSS was chosen over Bootstrap, see <https://blog.hubspot.com/website/w3-css-vs-bootstrap>.

  Markdown features currently supported:

  - Supports ASCII and UTF-8 in the markdown file
    - Uses `<meta charset="UTF-8">` in the HTML
    - Any codepoint, including emoji, are represented by native UTF-8
    - Best if using an editor that visually displays and saves in UTF-8 format
  - Heading levels 1-6: #, ##, ###, etc...
    - Alternate headings with `===` or `---` under text for heading levels <h1>, <h2>
  - Rich paragraphs: `code` `**bold**`, `<quickref.com>` `[text](link.com)`, etc...
    - Bold and Italics `*italic*`, `**bold**` or `***both***`
    - Inline code with `single backticks`
    - Image links: `![alt](link "title")`
    - Quick Links: , `[link text](link)`, `<https://url.com/file>` and `<name@email.com>`
    - Footnotes: `[^footnote]` and the linked text `[^footnote]: some paragraph`
    - HTML for image
  - Line breaks (two spaces at end of line)
  - Ordered and unordered nested lists
    - Supports `*`, `-`, `+`, `1.`
    - Supports task lists [x] or [ ]
  - Fenced code blocks with triple backticks or indent by 4 spaces
  - Tables that support left, right, center alignment
  - Escape any char with backlash, e.g. `\<` or `\|`
    - alternately use names, `&lt;` for `<` or `&amp;` for `&`
    - alternately use numbers, e.g. `&#124;` for `|`
  - Emoji with codes only, e.g. Smiley face &#128512;
  - Horizontal Rule with `---`
  - Subscript and Superscript, e.g. H~2~O and X^2^
  - Footnotes where `[^1]` points to local footnote `[^1]: footnote 1`
  - Horizontal Rule with `---`
  - Block quotes

  Markdown features NOT supported from <https://www.markdownguide.org/cheat-sheet/>:

  - Custom heading IDs, e.g. `# Heading 1 {#custom-id}`
  - Definition Lists
  - Deferred reference links, such as: `[hobbit-hole][1]`
    - the 2nd part MUST be parenthesis, e.g. `[hobbit-hole](https://hobbit-hole.com)`
  - Emoji by name, e.g. `:joy:`
  - Alternate code fencing (e.g. waves `~~~`). Always use triple backticks.
  - Alternate bold, italics (e.g. underscores `_italic_`. Always use asterix.
  
  Features Unique to FlyMarkdown.c:

  - Image formatting by overloading the optional "title" field. Examples:
    1. No title: `![Simple](image.png)`
    2. W3CSS: `![Circle Image](image.png "w3-circle")`
    3. HTML: `![More Control](image.png "class=\"w3-round\" style=\"width:80%\"")`
    4. Normal: `![With Title](image.png "just some title")`

  These produce the following HTML:

  ```html
  No title: <img src="image.png" alt="Simple">
  W3CSS: <img src="image.png" alt="Circle Image" class="w3-circle">
  HTML: <img src="image.png" alt="More Control "class="w3-round" style="width:80%">
  Normal: <img src="image.png" alt="With Title" title="just some title">
  ```
*/

typedef enum
{
  FLY_MD_LIST_TYPE_NOT_LIST   = 0,
  FLY_MD_LIST_TYPE_ORDERED,
  FLY_MD_LIST_TYPE_UNORDERED,
} mdListType_t;

#define   FLY_MD_LIST_CHECK_BOX         0x01  // check box unchecked
#define   FLY_MD_LIST_CHECK_BOX_CHECKED 0x02  // check box with checked

typedef struct
{
  flyMdEmType_t type;
  unsigned      len;
  char          c;
  bool_t        afOpen[MD_EM_TYPE_SIZEOF];
} mdEmphasis_t;

// prototypes
static const char  *MdListType    (const char *szLine, mdListType_t *pType, unsigned *pCheckbox);
static size_t       MdListMake    (char *szHtml, size_t size, const char **ppszMdList, unsigned indent, unsigned level);

static const char   m_szQuote[]       = "\"";
static const char   m_szEndBracket[]  = ">";
static const char   m_szTripleTicks[] = "```";
static const char   m_szSpace[]       = " ";

/*!-------------------------------------------------------------------------------------------------
  Similar to strpbrk(), but for a substring and understands `in-line code` and escaped \] chars.

  Example Usage:

  ```c
  const char sz[] = "Ignores escapes \\? `in-line code!` but finds !this".
  char       *sz_found;

  sz_found = FlyMdNPBrk(sz, sz + strlen(sz), "?!");
  if(sz_found && strcnmp(sz_found, "!this", 5) == 0)
  {
    // do something with keyword !this
  }
  ```

  @param  sz        string to search
  @param  szEnd     end of search
  @param  szAccept  characters to break on
  Returns ptr to found szAccept character or NULL if not found
*///-----------------------------------------------------------------------------------------------
char * FlyMdNPBrk(const char *sz, const char *szEnd, const char *szAccept)
{
  const char *szFound     = NULL;
  bool_t      fInLineCode = FALSE;
  bool_t      fCodeOk     = TRUE;

  // searching for backticks, don't worry about inline code
  if(strchr(szAccept, '`'))
    fCodeOk = FALSE;

  while(sz < szEnd && *sz)
  {
    // don't find escaped characters
    if(*sz == '\\' && (sz + 1 < szEnd) && (sz[1] > ' ' && sz[1] <= '~'))
      sz += 2;
    else
    {
      if(fCodeOk && *sz == '`')
        fInLineCode = !fInLineCode;
      if(!fInLineCode && strchr(szAccept, *sz))
      {
        szFound = sz;
        break;
      }
      ++sz;
    }
  }

  return (char *)szFound;
}

/*!------------------------------------------------------------------------------------------------
  Like strpbrk(), but ignores escaped chars

  THe example below finds the 2nd `]` if `FlyStrPBrk(sz, "]\"(")`;

      [some \] string](#reference)
                     ^

  @param  szHaystack    ptr to UTF-8 (or Markdown) string
  @param  szNeedles     pointer to a list of chars to stop on
  @return len of resulting bytes
*///-----------------------------------------------------------------------------------------------
static char * MdLinePBrk(const char *sz, const char *szAccept)
{
  return FlyMdNPBrk(sz, FlyStrLineEnd(sz), szAccept);
}

/*-------------------------------------------------------------------------------------------------
  Like strncat(), but has size, always '\0' terminates and handles escaped chars.

  Escaped characters include things like `\"`, which take up 2 bytes in szSrc, but only 1 byte in
  szDst.

  @param  szDst   Destination string, may be NULL to just get length
  @param  szSrc   Source string
  @param  size    size of szDst buffer
  @param  n       length to look in szSrc
  @return length copied in bytes
*///-----------------------------------------------------------------------------------------------
static size_t MdNCat(char *szDst, const char *szSrc, size_t size, size_t n)
{
#if MD_DEBUG_NCAT
  char    *szDstOrg = szDst;
#endif
  size_t  len       = 0;
  size_t  lenDst;

#if MD_DEBUG_NCAT
  printf("MdNCat(%.*s, size=%zu, n = %zu)\n", (unsigned)n, szSrc, size, n);
#endif

  if(szDst)
  {
    lenDst = strlen(szDst);
    if(lenDst >= size)
      size = 0;
    else
    {
      szDst += lenDst;
      size -= lenDst;
    }
  }

  // copy characters
  while(size > 1 && n && *szSrc)
  {
    // escaped character
    if(*szSrc == '\\' && (szSrc[1] > ' ' && szSrc[1] <= '~'))
    {
      ++szSrc;
      --n;
      if(n == 0)
        break;
    }
    if(szDst)
      *szDst++ = *szSrc;
    ++len;
    ++szSrc;
    --n;
    --size;
  }

  // zero terminate
  if(szDst && size)
    *szDst = '\0';

#if MD_DEBUG_NCAT
  FlyDbgPrintf("  %s, len %zu\n", FlyStrNullOk(szDstOrg), len);
#endif

  return len;
}

/*-------------------------------------------------------------------------------------------------
  Adjust size and szHtml pointer by strlen of szHtml

  @param  szHtml    ptr to html or NULL
  @param  pSize     ptr to size for adjustment
  @return new ptr of szHtml
*///-----------------------------------------------------------------------------------------------
static char * MdAdjust(char *szHtml, size_t *pSize)
{
  size_t n;

  if(szHtml)
  {
    n = strlen(szHtml);
    if(*pSize > n)
      *pSize -= n;
    else
      *pSize = 0;
    szHtml += n;
  }
  return szHtml;
}

/*-------------------------------------------------------------------------------------------------
  What type is this line? Returns both type and ptr to text following type.

  @param  szLine      ptr to a line of markdown
  @param  pType       return value, e.g. FLY_MD_LIST_TYPE_NOT_LIST, FLY_MD_LIST_TYPE_ORDERED
  @param  pCheckbox   return valye, 0 if not a checkbox, 1 or 2 if empty or checked
  @return ptr to text following list type, or szLine if FLY_MD_LIST_TYPE_NOT_LIST
*///-----------------------------------------------------------------------------------------------
static const char *MdListType(const char *szLine, mdListType_t *pType, unsigned *pCheckbox)
{
  const char   *psz  = szLine;
  const char   *psz2;
  mdListType_t  type = FLY_MD_LIST_TYPE_NOT_LIST;
  unsigned      checkbox = 0;

  // FlyDbgPrintf("MdListType(%.*s)\n", (int)FlyStrLineLen(szLine), szLine);

  if(!FlyStrLineIsBlank(szLine))
  {
    psz = FlyStrSkipWhite(szLine);
    if(*psz && isdigit(*psz))
    {
      while(isdigit(*psz))
        ++psz;
      if(*psz == '.' && isblank(psz[1]))
        type = FLY_MD_LIST_TYPE_ORDERED;
      ++psz;
    }
    else if(*psz && strchr("-+*", *psz) && isblank(psz[1]))
    {
      type = FLY_MD_LIST_TYPE_UNORDERED;
      ++psz;
    }
    if(FlyStrLineIsBlank(psz))
      type = FLY_MD_LIST_TYPE_NOT_LIST;

    // get pointer to text of line list
    if(type != FLY_MD_LIST_TYPE_NOT_LIST)
    {
      // might be a checkbox (checked or unchecked)
      psz2 = FlyStrSkipWhite(psz);
      if(*psz2 == '[' && (psz2[1] == ' ' || toupper(psz2[1]) == 'X') && psz2[2] == ']')
      {
        if(psz2[1] == ' ')
          checkbox = FLY_MD_LIST_CHECK_BOX;
        else
          checkbox = FLY_MD_LIST_CHECK_BOX_CHECKED;
        psz = psz2 + 3;
      }
      psz = FlyStrSkipWhite(psz);
    }
  }

  if(pCheckbox)
    *pCheckbox = checkbox;
  *pType = type;
  return (type == FLY_MD_LIST_TYPE_NOT_LIST) ? szLine : psz;
}

/*!------------------------------------------------------------------------------------------------
  Create spaces that will look correct in HTML by using &nbsp;

  Note: this only concatinates. The szHtml string must already be initialized

  1. A single space is always a single space
  2. Multiple spaces intermix every other space &nbsp;

  @param  szHtml    ptr to where to put the HTML
  @param  nSpaces   Number of spaces
  @param  size      sizeof HTML
  @return length added to szHtml
*///-----------------------------------------------------------------------------------------------
static size_t MdCatSpaces(char *szHtml, unsigned nSpaces, size_t size)
{
  unsigned  i;
  size_t    htmlLen = 0;

  if(nSpaces == 1)
    htmlLen += FlyStrZCat(szHtml, " ", size);
  else
  {
    for(i = 0; i < nSpaces; ++i)
      htmlLen += FlyStrZCat(szHtml, ((i & 1) == 0) ? "&nbsp;" : " ", size);
  }

  return htmlLen;
}

/*!------------------------------------------------------------------------------------------------
  Does this line contain a break (two spaces) at the end of the line?

  @param  szLine    ptr to markdown line
  @return TRUE if this line has a link break at the end
*///-----------------------------------------------------------------------------------------------
bool_t FlyMd2HtmlIsBreak(const char *szLine)
{
  unsigned nSpaces = FlyStrChrCountRev(szLine, FlyStrLineEnd(szLine), ' ');
  return nSpaces >= 2 ? TRUE : FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Does this line open/close a code block?

  @param  szLine    ptr to markdown line
  @return TRUE if this is a code block line
*///-----------------------------------------------------------------------------------------------
bool_t FlyMd2HtmlIsCodeBlk(const char *szLine, bool_t *pIsBackticks)
{
  const char *psz;
  bool_t  fIsBackticks = FALSE;
  bool_t  fIsCodeBlock = FALSE;

  // markdown code block can be indented by 4 spaces or enclosed in triple ticks
  psz = FlyStrSkipWhite(szLine);
  if(strncmp(psz, m_szTripleTicks, strlen(m_szTripleTicks)) == 0)
  {
    fIsCodeBlock = TRUE;
    fIsBackticks = TRUE;
  }
  else if(FlyStrLineIndent(szLine, FLY_STR_TAB_SIZE) >= 4)
  {
    fIsCodeBlock = TRUE;
  }

  if(pIsBackticks)
    *pIsBackticks = fIsBackticks;
  return fIsCodeBlock;
}

/*!------------------------------------------------------------------------------------------------
  Find the end of the code block. Handles both tripletick kind and indented kind.

  @param  szLine    ptr to markdown line
  @return ptr to line after code block.
*///-----------------------------------------------------------------------------------------------
char * FlyMd2HtmlCodeBlkEnd(const char *szLine)
{
  const char   *psz;
  bool_t        fIsBackticks;

  if(FlyMd2HtmlIsCodeBlk(szLine, &fIsBackticks))
  {
    if(fIsBackticks)
    {
      psz = strstr(FlyStrSkipWhite(szLine) + strlen(m_szTripleTicks), m_szTripleTicks);
      if(psz)
        szLine = FlyStrLineNext(psz);
      else
        szLine = FlyStrLineEof(szLine);
    }
    else
    {
      while(*szLine && (FlyStrLineIsBlank(szLine) || FlyStrLineIndent(szLine, FLY_STR_TAB_SIZE) >= 4))
        szLine = FlyStrLineNext(szLine);
    }
  }

  return (char *)szLine;
}

/*!------------------------------------------------------------------------------------------------
  Helper to both FlyMd2HtmlIsHeading() and FlyMd2HtmlHeading().

  Is this line a heading line (does it start with one or more hashes)?

  It also handles equal or dashed lines below the heading to H1 and H2.

      # H1 Heading
      ## H2 Heading
      
      Also H1 Heading
      ===============

      ALso H2 Heading
      ---------------

  @param  szLine    ptr to markdown line
  @param  pLevel    0-6, number of hashes, e.g. `#`, `##`, `###'
  @return NULL if not a heading, ptr to line after heading if it is (1 or 2 lines)
*///-----------------------------------------------------------------------------------------------
const char * MdIsHeading(const char *szLine, unsigned *pLevel)
{
  const char *psz;
  unsigned    level;

  // determine level
  psz = FlyStrLineNext(szLine);
  level = FlyStrChrCount(szLine, '#');

  // heading MUST have some text
  if(FlyStrLineIsBlank(&szLine[level]))
    level = 0;

  // ok, heading does have some text, it is alternate form?
  //
  // Heading Text
  // ===========
  else
  {
    if(level == 0)
    {
      // is this the alternate form with lines underneath the heading?
      if(*psz == '=' && FlyStrLineIsBlank(psz + FlyStrChrCount(psz, '=')))
        level = 1;
      if(*psz == '-' && FlyStrLineIsBlank(psz + FlyStrChrCount(psz, '-')))
        level = 2;

      // end of markdown skips the equal or dashed line
      if(level)
        psz = FlyStrLineNext(psz);
    }
  }

  if(level > 6)
    level = 0;

  if(level == 0)
    psz = NULL;

  // return values
  if(pLevel)
    *pLevel = level;
  return psz;
}

/*!------------------------------------------------------------------------------------------------
  Is this line a heading line (does it start with one or more hashes)?

  @param  szLine    ptr to markdown line
  @param  pLevel    number of hashes, e.g. `#`, `##`, `###'
  @return length of szHtml output
*///-----------------------------------------------------------------------------------------------
bool_t FlyMd2HtmlIsHeading(const char *szLine, unsigned *pLevel)
{
  bool_t    fIsHeading = FALSE;
  if(MdIsHeading(szLine, pLevel))
    fIsHeading = TRUE;
  return fIsHeading;
}

/*!------------------------------------------------------------------------------------------------
  Given a heading line, return ptr to heading text.

  Used by FlyDoc

  @param  szLine    ptr to markdown line
  @return ptr to heading text (next non-whitespace after ##), or NULL if not a heading
*///-----------------------------------------------------------------------------------------------
const char * FlyMd2HtmlHeadingText(const char *szLine)
{
  const char *szHdrText = NULL;

  if(FlyMd2HtmlIsHeading(szLine, NULL))
  {
    szHdrText = szLine;
    if(*szLine == '#')
      szHdrText = FlyStrSkipWhite(&szLine[FlyStrChrCount(szLine, '#')]);
  }

  return szHdrText;
}

/*!------------------------------------------------------------------------------------------------
  Is this a list line? 

  @param  szHtml      ptr to char array or NULL to just get size of resulting HTML
  @param  szLine      Is this line a list?
  @param  pIsNumeric  ptr to flag whether numeric or not
  @return TRUE if this is a list line
*///-----------------------------------------------------------------------------------------------
bool_t FlyMd2HtmlIsList(const char *szLine, bool_t *pIsNumeric)
{
  mdListType_t type = FLY_MD_LIST_TYPE_NOT_LIST;

  MdListType(szLine, &type, NULL);
  if(pIsNumeric)
    *pIsNumeric = (type == FLY_MD_LIST_TYPE_ORDERED) ? TRUE : FALSE;
  return (type == FLY_MD_LIST_TYPE_NOT_LIST) ? FALSE : TRUE;
}

/*!------------------------------------------------------------------------------------------------
  Is this a reference to a link, image or footnote?

      MD_REF_TYPE_IMAGE,          // ![alt text](file.png "title")
      MD_REF_TYPE_REF,            // [ref text](site.com/page)
      MD_REF_TYPE_FOOT_REF,       // [^footnote]
      MD_REF_TYPE_FOOTNOTE        // [^footnote]: paragraph text of footnote

  @param  szMd    ptr to markdown
  @param  pIsImg  TRUE if this is an image reference
  @return reference type (e.g. MD_REF_TYPE_REF) or MD_REF_TYPE_NONE
*///-----------------------------------------------------------------------------------------------
flyMdRefType_t FlyMd2HtmlIsRef(const char *szMd)
{
  flyMdAltLink_t  altLink;
  flyMdRefType_t  refType = MD_REF_TYPE_NONE;

  if(FlyMdAltLink(&altLink, szMd))
    refType = altLink.refType;

  return refType;
}

/*!------------------------------------------------------------------------------------------------
  Is this a markdown image? For example

      ![alt text](file.png "title")

  @param  szMd    ptr to markdown
  @return TRUE if markdown image, FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlyMd2HtmlIsImage(const char *szMd)
{
  flyMdAltLink_t  altLink;
  bool_t          fIsImage = FALSE;
  if(FlyMdAltLink(&altLink, szMd) && altLink.refType == MD_REF_TYPE_IMAGE)
    fIsImage = TRUE;
  return fIsImage;
}

/*!------------------------------------------------------------------------------------------------
  Is this a quick link? e.g. <https://mysite.com> or <me@mysite.com> or <#pagelink>

  Note: this does NOT skip whitespace, szMd must be on the starting '<' of the QLink.

  The trick is to distinguish between HTML tags like <span> from <me@mysite.com>.

  Basic rules are:

  1. Must have an ending brace
  2. Must have one of ".@#" in it.

  @param  szMd    ptr to markdown
  @return TRUE if this is a quick link
*///-----------------------------------------------------------------------------------------------
bool_t FlyMd2HtmlIsQLink(const char *szMd)
{
  const char *pszEnd;
  bool_t fIsQLink = FALSE;

  if(*szMd == '<' && !isblank(szMd[1]))
  {
    fIsQLink = TRUE;
    pszEnd = FlyStrLineChr(szMd, '>');  // 
    if(!pszEnd)
      fIsQLink = FALSE;
    else
    {
      --pszEnd;
      // looks like end of an HTML <tag attr="">
      if(*pszEnd == '"' || isblank(*pszEnd))
        fIsQLink = FALSE;
      else
      {
        ++pszEnd;
        if(FlyStrNChrMatch(szMd, pszEnd, ".@#") == NULL)
          fIsQLink = FALSE;
      }
    }
  }

  return fIsQLink;
}

/*-------------------------------------------------------------------------------------------------
  Similar to strchr(), but ends on the line and ignores escaped chars.

  @param  szMd    ptr to markdown characters
  @param  c       character to search for
  @return ptr to found char or NULL
*///-----------------------------------------------------------------------------------------------
const char *MdLineChr(const char *szMd, char c)
{
  const char *sz      = szMd;
  const char *szFound = NULL;

  while(*sz && *sz != '\r' && *sz != '\n')
  {
    // don't find escaped characters
    if(*sz == '\\')
    {
      ++sz;
      if(*sz)
        ++sz;
      continue;
    }

    if(*sz == c)
    {
      szFound = sz;
      break;
    }
    ++sz;
  }

  return szFound;
}

/*-------------------------------------------------------------------------------------------------
  Determine # of cells in this row, each separated by a |

  Example: "cell | cell|cell" would return 3 cells.

  @param  szMd    ptr to markdown
  @return TRUE if this is a quick link
*///-----------------------------------------------------------------------------------------------
static unsigned MdTableCellCount(const char *szMd)
{
  const char *psz;
  const char *pszLast = NULL;
  const char *szLineEnd;
  unsigned    count = 0;

  szLineEnd = FlyStrLineEnd(szMd);
  psz       = szMd;
  while(psz < szLineEnd)
  {
    psz = MdLineChr(psz, '|');
    if(!psz)
      break;
    if(psz)
    {
      ++count;
      ++psz;
      pszLast = psz;
    }
  }

  // "a|" is 1 column vs "a|b" is 2 columns
  // FlyDbgPrintf("count %u, pszLast %p, szLineEnd %p\n", count, pszLast, szLineEnd);
  if(count && FlyStrSkipWhite(pszLast) < szLineEnd)
    ++count;

  return count;
}

/*-------------------------------------------------------------------------------------------------
  Return the table cell data ptr and len. Skips whitespace at start/end of cell data.

  @param  ppszMd    ptr to table cell data. Points to next cell upon return
  @param  pLen      length of cell data
  @param  ppszNext  ptr to next cell, or NULL if no more cells
  @return ptr to cell data
*///-----------------------------------------------------------------------------------------------
static const char * MdTableCellGet(const char *szMd, size_t *pLen, const char **ppszNext)
{
  const char *szCell;
  const char *pszNext;
  const char *psz;
  size_t      len = 0;

  // get ptr to this cell data
  szCell = FlyStrSkipWhite(szMd);

  // determine length
  psz = pszNext = MdLineChr(szCell, '|');
  if(pszNext != NULL)
    ++pszNext;
  if(psz == NULL)
    psz = FlyStrLineEnd(szCell);
  while(psz > szCell)     // skip trailing spaces for len
  {
    --psz;
    if(!isblank(*psz))
    {
      len = (psz - szCell) + 1;
      break;
    }
  }

#if 0
  if(len == 0)
    FlyDbgPrintf("no cell len %zu:, pszNext %p\n", len, pszNext);
  else
  {
    const char *szNext;
    unsigned    n;
    szNext = pszNext;
    if(szNext == NULL)
      szNext = "(NULL)";
    n = FlyStrLineLen(szNext);
    FlyDbgPrintf("got cell len %zu: %.*s, pszNext %.*s\n", len, (int)len, szCell, n, szNext);
  }
#endif

  // return values
  if(pLen)
    *pLen = len;
  if(ppszNext)
    *ppszNext = pszNext ? pszNext : FlyStrLineEnd(szCell);
  return szCell;
}

typedef enum
{
  MDTABLECOL_TYPE_LEFT,
  MDTABLECOL_TYPE_CENTER,
  MDTABLECOL_TYPE_RIGHT,
} mdTableColType_t;

/*-------------------------------------------------------------------------------------------------
  Parses a table header to determine left, right, center text justification for each column

  Example below has 3 columns, left aligned, center and right aligned:

  left | center | right
  :--- | :----: | ----:

  @param  szMd      ptr to markdown
  @param  maxCols   max cols on input, actual cols on output
  @param  aColType  array for left, right, center, e.g. MDTABLECOL_TYPE_RIGHT
  @return 0 = not a valid table, 1-n = # of columns
*///-----------------------------------------------------------------------------------------------s
static unsigned MdTableGetCols(const char *szLine, unsigned maxCols, mdTableColType_t aColType[])
{
  const char *szCell;
  const char *pszNextCell;
  unsigned    nCols;
  size_t      len;
  unsigned    i, j;

  // determine column count from table header
  nCols = MdTableCellCount(szLine);
#if MD_DEBUG_TABLE
  FlyDbgPrintf(" MdTableGetCols, nCols %u\n", nCols);
#endif
  if(nCols)
  {
    // there MUST a least same # of --- | --- | ---
    // also, determine left, right, center for this column
    szLine = FlyStrLineNext(szLine);
    pszNextCell = szLine;
    for(i = 0; i < nCols; ++i)
    {
      // get the cell, e.g. --- or :------:
      szCell = MdTableCellGet(pszNextCell, &len, &pszNextCell);
      if(len < 3)
      {
        nCols = 0;
        break;
      }

      // verify no bad chars in column alignment cells
      for(j = 0; j < len; ++j)
      {
        if(szCell[j] != ':' && szCell[j] != '-')
        {
#if MD_DEBUG_TABLE
          FlyDbgPrintf("bad column alignment %.*s\n", (int)len, szCell);
#endif
          nCols = 0;
          break;
        }
      }
      if(nCols == 0)
        break;

      // determine left/right/center for this column
      if(aColType)
      {
        if(szCell[0] == ':' && szCell[len - 1] == ':')
          aColType[i] = MDTABLECOL_TYPE_CENTER;
        else if(szCell[len - 1] == ':')
          aColType[i] = MDTABLECOL_TYPE_RIGHT;
        else
          aColType[i] = MDTABLECOL_TYPE_LEFT;
      }
    }
  }

  return nCols;
}

/*!------------------------------------------------------------------------------------------------
  Does this line start a table? Requires at least one " | "

  @param  szMd    ptr to markdown
  @return TRUE if this is a quick link
*///-----------------------------------------------------------------------------------------------
bool_t FlyMd2HtmlIsTable(const char *szMd)
{
  unsigned cols;

  cols = MdTableGetCols(szMd, FLYMD2HTML_TABLE_COL_MAX, NULL);
#if MD_DEBUG_TABLE
  FlyDbgPrintf("  FlyMd2HtmlIsTable(%.*s), cols %u\n", (int)FlyStrLineLen(szMd), szMd, cols);
#endif
  return cols ? TRUE : FALSE;
}

typedef struct
{
  flyMdEmType_t   type;
  unsigned        len;
  char            marker;
  const char     *szHtmlOpen;
  const char     *szHtmlClose;
} mdEmTypeInfo_t;

static const mdEmTypeInfo_t m_aEmMdTypeInfo[] =
{
  { MD_EM_TYPE_ITALICS,        1, '*', "<i>", "</i>" },        // *italics*
  { MD_EM_TYPE_BOLD,           2, '*', "<b>", "</b>" },        // **bold**
  { MD_EM_TYPE_BOLD_ITAL,      3, '*', "<b><i>", "</i></b>"},  // ***bold & italics***
  { MD_EM_TYPE_HIGHLIGHT,      2, '=', "<mark>", "</mark>"},   // ==highlight==
  { MD_EM_TYPE_STRIKE_THROUGH, 2, '~', "<del>", "</del>"},     // ~~strike through~~
  { MD_EM_TYPE_SUB,            1, '~', "<sub>", "</sub>"},     // ~subscript~
  { MD_EM_TYPE_SUPER,          1, '^', "<sup>", "</sup>"},     // ^superscript^
};
static const char m_szMdEmMarkers[] = "*=~^";
static const char m_szMdSpecial[]   = "*=~^&<![`";  // make sure to include all of m_szMdEmMarkers

/*-------------------------------------------------------------------------------------------------
  Finds the mdEmTypeInfo_t for bold, italics, both, highlight, strikethrough, subscript, superscript.

  Examples: "2 * 3 = 6 (not italics)", "** not bold **", "**is bold**"

  flyMdEmType_t             | In Markdown
  ------------------------- | ------------
  MD_EM_TYPE_ITALICS        | *italics*
  MD_EM_TYPE_BOLD           | **bold**
  MD_EM_TYPE_BOLD_ITAL      | ***bold & italics***
  MD_EM_TYPE_HIGHLIGHT      | ==highlight==
  MD_EM_TYPE_STRIKE_THROUGH | ~~strike through~~
  MD_EM_TYPE_SUB            | ~subscript~
  MD_EM_TYPE_SUPER          | ^superscript^

  @param  szMd    ptr to markdown
  @return NULL if szMd is not pointing to one of the em types
*///-----------------------------------------------------------------------------------------------
static const mdEmTypeInfo_t * MdEmTypeInfoGet(const char *szMd)
{
  const mdEmTypeInfo_t   *pTypeInfo = NULL;
  const char       *psz;
  unsigned          nMarkers;
  char              marker;
  unsigned          i;

  psz = strchr(m_szMdEmMarkers, *szMd);
  if(psz)
  {
    marker = *psz;
    nMarkers = FlyStrChrCount(szMd, marker);
    for(i = 0; i < NumElements(m_aEmMdTypeInfo); ++i)
    {
      if(m_aEmMdTypeInfo[i].marker == marker && m_aEmMdTypeInfo[i].len == nMarkers)
      {
        pTypeInfo = &m_aEmMdTypeInfo[i];

        // don't consider subscript if ~/, which is home path
        if(pTypeInfo->type == MD_EM_TYPE_SUB && szMd[1] == '/')
        {
          pTypeInfo = NULL;
          continue;
        }
        break;
      }
    }
  }

  // return
  return pTypeInfo;
}

#if 0
/*-------------------------------------------------------------------------------------------------
  Find the matching (ending) Emphasis marker of same type.

  Example, if sz points to "bold" below, would return:

      ***bold and italics***
                         ^

  @param  szMd      ptr to markdown
  @param  type      type to find
  @param  marker    marker char for type, one of `*~^=`
  @return TRUE if this is a quick link
*///-----------------------------------------------------------------------------------------------
static char * MdEmFind(const char *szMd, flyMdEmType_t type, char marker)
{
  const mdEmTypeInfo_t *pTypeInfo;
  const char           *szFound     = NULL;

  while(szMd)
  {
    szMd = FlyStrLineChr(szMd, marker);
    if(szMd == NULL)
      break;

    pTypeInfo = MdEmTypeInfoGet(szMd);
    if(!pTypeInfo)
      ++szMd;
    else if(pTypeInfo->type == type)
    {
      szFound = szMd;
      break;
    }
    else
      szMd += pTypeInfo->len;
  }

  return (char *)szFound;
}
#endif

/*!------------------------------------------------------------------------------------------------
  Return type of emphasis at the given markdown position, which may be 0 (MD_EM_TYPE_ITALICS).

  Examples: "2 * 3 = 6 (not italics)", "** not bold **", "**is bold**"

  flyMdEmType_t             | In Markdown
  ------------------------- | ------------
  MD_EM_TYPE_ITALICS        | *italics*
  MD_EM_TYPE_BOLD           | **bold**
  MD_EM_TYPE_BOLD_ITAL      | ***bold & italics***
  MD_EM_TYPE_HIGHLIGHT      | ==highlight==
  MD_EM_TYPE_STRIKE_THROUGH | ~~strike through~~
  MD_EM_TYPE_SUB            | ~subscript~
  MD_EM_TYPE_SUPER          | ^superscript^

  @param  szMd    ptr to markdown
  @return type of emphasis, e.g. MD_EM_TYPE_BOLD or MD_EM_TYPE_NONE
*///-----------------------------------------------------------------------------------------------
flyMdEmType_t FlyMd2HtmlIsEmphasis(const char *szMd)
{
  flyMdEmType_t   type        = MD_EM_TYPE_NONE;
  const mdEmTypeInfo_t *pTypeInfo;

  pTypeInfo = MdEmTypeInfoGet(szMd);
  if(pTypeInfo)
    type = pTypeInfo->type;

  return type;
}

/*!------------------------------------------------------------------------------------------------
  Is this markdown line a horizontal rule? That is, three or more of `***` or `---` or `___`.

  @param  szMdLine    ptr to markdown line
  @return TRUE if yes
*///-----------------------------------------------------------------------------------------------
bool_t FlyMd2HtmlIsHorzRule(const char *szMdLine)
{
  const char  szHorzRule[] = "*-_";
  bool_t      fIsHorzRule = FALSE;
  unsigned    n;

  if(strchr(szHorzRule, *szMdLine))
  {
    n = FlyStrChrCount(szMdLine, *szMdLine);
    if(n >= 3 && FlyStrLineIsBlank(&szMdLine[n]))
      fIsHorzRule = TRUE;
  }

  return fIsHorzRule;
}

/*!------------------------------------------------------------------------------------------------
  Convert horizontal rule from markdown to HTML. 

  @param  szHtml    ptr to HTML buffer or NULL if just getting length.
  @return length of HTML
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlHorzRule(char *szHtml, size_t size, const char **ppszMd)
{
  const char szHtmlHorzRule[] = "<p><hr></p>\r\n";
  size_t  htmlLen = 0;

  if(FlyMd2HtmlIsHorzRule(*ppszMd))
  {
    htmlLen = FlyStrZCpy(szHtml, szHtmlHorzRule, size);
    *ppszMd = FlyStrLineNext(*ppszMd);
  }

  return htmlLen;
}

/*!------------------------------------------------------------------------------------------------
  Converts **bold**, *italics*  and **bold italics**, etc... into HTML

  Param ppszMd is both input and output. It's advanced to end of "consumed" markdown.

  Markdown | fClose = 0 | fClose = 1
  ---------| ---------- | ----------
  '*'      | <i>        | </i>
  '**'     | <b>        | </b>
  '***'    | <b><i>     | </i></b>
  '=='     | <mark>     | </mark>
  '~~'     | <del>      | </del>
  '~'      | <sub>      | </sub>
  '^'      | <sup>      | </sup>

  @param  szHtml    ptr to char array or NULL to just get size of resulting HTML
  @param  size      sizeof(szHtml)
  @param  ppszMd    ptr markdown `code words`, updated to after `code words`
  @param  fClose    close the emphasis, otherwise open it
  @return length of szHtml output
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlEmphasis(char *szHtml, size_t size, const char **ppszMd, bool_t fClose)
{
  const mdEmTypeInfo_t *pTypeInfo;
  size_t                htmlLen     = 0;

  if(szHtml)
    *szHtml = '\0';
  pTypeInfo = MdEmTypeInfoGet(*ppszMd);
  if(pTypeInfo)
  {
    htmlLen = FlyStrZCpy(szHtml, fClose ? pTypeInfo->szHtmlClose : pTypeInfo->szHtmlOpen, size);
    *ppszMd += pTypeInfo->len;
  }

  return htmlLen;
}

/*!------------------------------------------------------------------------------------------------
  Is this line a block quote? That is, does it start with 1 - 6 `>` characters?

  Example of block quotes:

      > A block quote
      >>>>>> A Deep block quote
      >>>>>>> Not a block quote

  @param  szMd      ptr to markdown which might be a block quote
  @return length of HTML
*///-----------------------------------------------------------------------------------------------
bool_t FlyMd2HtmlIsBlockQuote(const char *szMd)
{
  unsigned level = FlyStrChrCount(szMd, '>');
  return (level && level <= FLYMD2HTM_BLOCK_QUOTE_MAX) ? TRUE : FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Convert block quotes into appropriate HTML.

  This:

  ```
    > Block quote 1
    >> Indented block quote para 1
    >>
    >> Indented block quote para 2
    > Block quote 2a.  
    > Block quote 2b. 
  ```

  Becomes That:

  ```
  <div class="w3-panel w3-leftbar">
    <p>block quote 1</p>
    <div class="w3-panel w3-leftbar">
      <p>Indented block quote para 1</p>
      <p>Indented block quote para 2</p>
    <p>Block quote 2a. Block quote 2b.</p>
    </div> 
  </div> 
```

  ppszMd is both input and output. It's advanced to end of "consumed" markdown

  @param  szHtml    ptr to char array or NULL to just get size of resulting HTML
  @param  size      sizeof(szHtml)
  @param  ppszMd    ptr markdown block quote
  @return length of szHtml output, 0 if not block quote
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlBlockQuote(char *szHtml, size_t size, const char **ppszMd)
{
  const char  szOpenBlockQuote[]  = "<div class=\"w3-panel w3-leftbar\">\r\n";
  const char  szCloseBlockQuote[] = "</div>\r\n";
  const char  szOpenPara[]        = "<p>";
  const char  szClosePara[]       = "</p>";
  const char  szEndLine[]         = "\r\n";

  char          szIndent[2 * (FLYMD2HTM_BLOCK_QUOTE_MAX + 1)];
  const char   *szLine;
  const char   *szLineNext;
  const char   *szLineEnd;
  const char   *pszMd;
  bool_t        fInPara     = FALSE;
  unsigned      lastLevel   = 0;
  unsigned      level;
  unsigned      nextLevel;
  size_t        htmlLen     = 0;
  bool_t        fIndented;

  // 0 means normal paragraph, 1-n means block quote paragraph
  szLine = *ppszMd;
  if(FlyMd2HtmlIsBlockQuote(szLine))
  {
    if(szHtml)
      *szHtml = '\0';

#if MD_DEBUG_BLOCK_QUOTE
    FlyDbgPrintf("FlyMd2HtmlBlockQuote(%.*s)\n", (int)FlyStrLineLen(szLine), szLine);
#endif

    while(TRUE)
    {
      // get block quote level for this line
      level = FlyStrChrCount(szLine, '>');

      // increasing level by 1 or more
      while(lastLevel < level)
      {
        if(lastLevel)
        {
          FlyStrZFill(szIndent, ' ', sizeof(szIndent), 2 * lastLevel);
          htmlLen += FlyStrZCat(szHtml, szIndent, size);
        }
        htmlLen += FlyStrZCat(szHtml, szOpenBlockQuote, size);
        ++lastLevel;
      }

      // reducing level by 1 or more
      while(lastLevel > level)
      {
        --lastLevel;
        if(lastLevel)
        {
          FlyStrZFill(szIndent, ' ', sizeof(szIndent), 2 * lastLevel);
          htmlLen += FlyStrZCat(szHtml, szIndent, size);
        }
        htmlLen += FlyStrZCat(szHtml, szCloseBlockQuote, size);
      }

      // at this point, lastLevel == level

      // no longer in block quotes
      if(level == 0)
        break;

      // find text (if any)
      pszMd = szLine = FlyStrSkipWhite(&szLine[level]);
      FlyStrZFill(szIndent, ' ', sizeof(szIndent), 2 * level);

      fIndented = FALSE;
      if(!fInPara)
      {
        htmlLen += FlyStrZCat(szHtml, szIndent, size);
        if(FlyMd2HtmlIsRef(szLine) == MD_REF_TYPE_FOOTNOTE)
        {
          szHtml = MdAdjust(szHtml, &size);
          htmlLen += FlyMd2HtmlRef(szHtml, size, &pszMd);
        }
        else
          htmlLen += FlyStrZCat(szHtml, szOpenPara, size);
        fInPara = TRUE;
        fIndented = TRUE;
      }

      szLineEnd = FlyStrLineEnd(szLine);
      if(szLineEnd > szLine)
      {
        if(!fIndented)
          htmlLen += FlyStrZCat(szHtml, szIndent, size);
        szHtml = MdAdjust(szHtml, &size);
        htmlLen += FlyMd2HtmlTextLine(szHtml, size, &pszMd, szLineEnd);

        if(FlyMd2HtmlIsBreak(szLine))
          htmlLen += FlyStrZCat(szHtml, "<br>", size);
      }

      // end the paragraph if:
      // 1. this line is blank
      // 2. next line is at different level
      szLineNext = FlyStrLineNext(szLine);
      nextLevel = FlyStrChrCount(szLineNext, '>');
      if(FlyStrLineIsBlank(szLine) || nextLevel != level)
      {
        htmlLen += FlyStrZCat(szHtml, szClosePara, size);
        fInPara = FALSE;
      }

      // end the line whether still in a paragraph or not
      if(!(nextLevel && FlyStrLineIsBlank(&szLineNext[nextLevel])))
        htmlLen += FlyStrZCat(szHtml, szEndLine, size);
      szLine = szLineNext;
    }
  }

  if(htmlLen)
    *ppszMd = szLine;
  return htmlLen;
}

/*!------------------------------------------------------------------------------------------------
  Converts `code words` into <code class="w3-codespan">code words</code>

  ppszMd is both input and output. It's advanced to end of "consumed" markdown

  @param  szHtml    ptr to char array or NULL to just get size of resulting HTML
  @param  size      sizeof(szHtml)
  @param  ppszMd    if szHtml, ptr markdown `code words`, updated to after `code words`
  @return length of szHtml output
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlCodeIn(char *szHtml, size_t size, const char **ppszMd)
{
  const char   *psz;
  const char   *szMd = *ppszMd;
  const char    szCodeOpen[]  = "<code class=\"w3-codespan\">";
  const char    szCodeClose[] = "</code>";
  unsigned      mdLen   = 0;
  unsigned      htmlLen = 0;

  if(*szMd == '`')
  {
    // end of code will be next backtick or end of line
    psz = MdLinePBrk(szMd + 1, "`");
    if(!psz)
      psz = FlyStrLineEnd(szMd);
    mdLen += (unsigned)(psz - (szMd + 1));

    // if there is content to the code, then there is HTML
    if(mdLen)
    {
      htmlLen += FlyStrZCpy(szHtml, szCodeOpen, size);
      htmlLen += FlyStrZNCat(szHtml, szMd + 1, size, mdLen);
      htmlLen += FlyStrZCat(szHtml, szCodeClose, size);
      ++mdLen;
      if(*psz == '`')
        ++mdLen;
    }
  }

  // return length of `code`, update *ppszMd to after code span
  *ppszMd += mdLen;
  return htmlLen;
}

/*-------------------------------------------------------------------------------------------------
  Internal use. For partial code lines.

  1. Changes `<` to &lt;
  2. Changes multiple spaces to use &nbsp;

  ppszMd is both input and output. It's advanced to end of "consumed" markdown

  @param  szHtml    string to receive HTML
  @param  size      sizeof szHtml
  @param  ppszMd    ptr to markdown line
  @return length of output HTML
*///-----------------------------------------------------------------------------------------------
static size_t MsCodeLineSegment(char *szHtml, size_t size, const char **ppszMd, const char *pszLineEnd)
{
  const char *szLine = *ppszMd;
  const char *psz;
  const char *pszText;
  const char  szBreakEnd[] = "<br>\r\n";
  unsigned    nSpaces;
  size_t      htmlLen = 0;

  if(szHtml)
    *szHtml = '\0';

  psz = szLine;
  if(!FlyStrLineIsBlank(psz))
  {
    pszText = psz;
    while(*psz && psz < pszLineEnd)
    {
      if(*psz == ' ' || *psz == '<')
      {
        htmlLen += FlyStrZNCat(szHtml, pszText, size, (psz - pszText));
        if(*psz == ' ')
        {
          // if line starts with space, use non-breaking space so HTML doesn't ignore it
          // if many spaces, intermix with non-breaking space
          nSpaces = FlyStrChrCount(psz, ' ');
          if(psz == szLine && nSpaces == 1)
            htmlLen += FlyStrZCat(szHtml, "&nbsp;", size);
          else
            htmlLen += MdCatSpaces(szHtml, nSpaces, size);
          psz += nSpaces;
        }
        else
        {
          htmlLen += FlyStrZCat(szHtml, "&lt;", size);
          ++psz;
        }
        pszText = psz;
      }
      else
      {
        ++psz;
        continue;
      }
    }

    htmlLen += FlyStrZNCat(szHtml, pszText, size, (psz - pszText));
  }

  htmlLen += FlyStrZCat(szHtml, szBreakEnd, size);

  // return pointer to after the converted text
  *ppszMd = pszLineEnd;

  return htmlLen;
}

/*-------------------------------------------------------------------------------------------------
  Used for lines inside a code-block. Does only 1 conversions:

  1. Changes `<` to &lt;
  2. Changes multiple spaces to use &nbsp;

  ppszMd is both input and output. It's advanced to end of "consumed" markdown

  @param  szHtml    string to receive HTML
  @param  size      sizeof szHtml
  @param  ppszMd    ptr to markdown line
  @return length of output HTML
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlCodeLine(char *szHtml, size_t size, const char **ppszMd)
{
  const char *pszLineEnd;
  size_t      htmlLen;

  pszLineEnd = FlyStrLineEnd(*ppszMd);
  htmlLen = MsCodeLineSegment(szHtml, size, ppszMd, pszLineEnd);
  *ppszMd = FlyStrLineNext(pszLineEnd);
  return htmlLen;
}

/*!------------------------------------------------------------------------------------------------
  Create a visual HTML code block from a markdown code block. A code block starts with triple
  backticks or is indented 4 spaces or a tab.

  ppszMd is both input and output. It's advanced to end of "consumed" markdown

  Note: uses W3.CSS class "w3-panel w3-card" if szTitle,

  Uses 1st line of code block to determine indent to remove. In this way, the code block is always
  flush left. It's up to any higher layer to add back in some indent if needed.

  @param  szHtml      ptr to char array or NULL to just get length of resulting HTML
  @param  size        sizeof(szHtml)
  @param  ppszMd      ptr line of markdown
  @param  szTitle     optional title "Some Title", NULL for no h5 title
  @param  szW3Color   optional color class, default w3-light-grey if NULL
  @return length HTML, or 0 if not a code block
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlCodeBlk(char *szHtml, size_t size, const char **ppszMd, const char *szTitle, const char *szW3Color)
{
  const char  szDivOpen1[]      = "<div class=\"w3-code ";
  const char  szDivOpen2[]      = " notranslate\">\r\n";
  const char  szDivOpenTitle1[] = "<div class=\"w3-panel w3-card ";
  const char  szDivOpenTitle2[] = "\">\r\n";
  const char  szDivTitle1[]     = "  <h5 id=\"";
  const char  szDivTitle2[]     = "\">";
  const char  szDivTitle3[]     = "</h5>\r\n"
                                  "  <div class=\"w3-code notranslate\">\r\n";
  const char  szIndent[]        = "  ";
  const char  szDivEndTitle[]   = "  </div>\r\n";
  const char  szDivEnd[]        = "</div>\r\n";

  bool_t      fIsBackTicks;
  unsigned    indent;
  const char *szLine;
  const char *szLinePrev;
  const char *pszThisLine;
  const char *pszBlockStart;
  const char *pszBlockEnd;
  const char *pszMdEnd        = *ppszMd;
  unsigned    htmlLen         = 0;
  bool_t      fOneLine        = FALSE;

  if(szW3Color == NULL)
    szW3Color = "w3-light-grey";

  szLine = *ppszMd;
  if(FlyMd2HtmlIsCodeBlk(szLine, &fIsBackTicks))
  {
    indent = FlyStrLineIndent(szLine, 1);
    // FlyDbgPrintf("CodeBlk indent %u\n", indent);

    // find start of code block contents
    pszBlockStart = szLine;

    // code block started with triple ticks, must end with the same, or end of file
    if(fIsBackTicks)
    {
      pszBlockStart = FlyStrSkipChars(szLine, " \t`");
      pszBlockEnd = strstr(pszBlockStart, m_szTripleTicks);
      if(pszBlockEnd == NULL)
        pszBlockEnd = FlyStrLineEof(pszBlockStart);
      if(pszBlockEnd < FlyStrLineEnd(pszBlockStart))
      {
        fOneLine = TRUE;
      }
      else
      {
        pszBlockStart = FlyStrLineNext(pszBlockStart);
        pszBlockEnd = FlyStrLineBeg(pszBlockStart, pszBlockEnd);
      }

      // always ends on the next line after tripple ticks
      pszMdEnd = FlyStrLineNext(pszBlockEnd);
    }

    // indented code block ends on non-blank, non-indented line
    else
    {
      while(*szLine && (FlyStrLineIsBlank(szLine) || FlyStrLineIndent(szLine, FLY_STR_TAB_SIZE) >= 4))
        szLine = FlyStrLineNext(szLine);
      pszMdEnd = pszBlockEnd = szLine;
      szLinePrev = FlyStrLinePrev(*ppszMd, szLine);
      if(FlyStrLineIsBlank(szLinePrev))
        pszBlockEnd = szLinePrev;
    }

    // debugging
#if MD_DEBUG_CODE
    if(fFlyMarkdownDebug && g_szMd)
    {
      unsigned row, col;
      row = FlyStrLinePos(g_szMd, pszBlockStart, &col);
      FlyDbgPrintf("BlockStart %u:%u\n", row, col);
      row = FlyStrLinePos(g_szMd, pszBlockEnd, &col);
      FlyDbgPrintf("BlockEnd   %u:%u\n", row, col);
    }
#endif

    // copy front matter
    if(szTitle)
    {
      // <div id="my-title" class="w3-panel w3-card w3-light-grey"> 
      //   <h5 id="Example-FLyJsonPut">Example: FlyJsonPut</h5>
      //   <div class="w3-code notranslate">
      htmlLen += FlyStrZCpy(szHtml, szDivOpenTitle1, size);
      htmlLen += FlyStrZCat(szHtml, szW3Color, size);
      htmlLen += FlyStrZCat(szHtml, szDivOpenTitle2, size);

      htmlLen += FlyStrZCat(szHtml, szDivTitle1, size);
      szHtml = MdAdjust(szHtml, &size);
      htmlLen += FlyStrSlug(szHtml, szTitle, size, strlen(szTitle));
      htmlLen += FlyStrZCat(szHtml, szDivTitle2, size);
      htmlLen += FlyStrZCat(szHtml, szTitle, size);
      htmlLen += FlyStrZCat(szHtml, szDivTitle3, size);
    }
    else
    {
      // <div class=\"w3-code w3-light-grey notranslate\">
      htmlLen += FlyStrZCpy(szHtml, szDivOpen1, size);
      htmlLen += FlyStrZCat(szHtml, szW3Color, size);
      htmlLen += FlyStrZCat(szHtml, szDivOpen2, size);
    }

    if(fOneLine)
    {
      szHtml = MdAdjust(szHtml, &size);
      htmlLen += MsCodeLineSegment(szHtml, size, &pszBlockStart, pszBlockEnd);
    }

    else
    {
      // copy code lines
      szLine = pszBlockStart;
      while(*szLine && szLine < pszBlockEnd)
      {
        if(FlyStrLineLen(szLine) < indent || FlyStrLineIsBlank(szLine))
          pszThisLine = "\n";
        else
          pszThisLine = &szLine[indent];

        htmlLen += FlyStrZCat(szHtml, szIndent, size);
        if(szTitle)
          htmlLen += FlyStrZCat(szHtml, szIndent, size);
        szHtml = MdAdjust(szHtml, &size);
        htmlLen += FlyMd2HtmlCodeLine(szHtml, size, &pszThisLine);
        szLine = FlyStrLineNext(szLine);
      }
    }

    // final matter
    if(szTitle)
      htmlLen += FlyStrZCat(szHtml, szDivEndTitle, size);
    htmlLen += FlyStrZCat(szHtml, szDivEnd, size);
  }

  if(htmlLen)
    *ppszMd = pszMdEnd;
  return htmlLen;
}

/*-------------------------------------------------------------------------------------------------
  Get the alt and optional link and title from a markdown reference.

  The types of references are shown below:

    MD_REF_TYPE_IMAGE,          // ![alt text](file.png "title")
    MD_REF_TYPE_REF,            // [ref text](site.com/page)
    MD_REF_TYPE_FOOT_REF,       // [^footnote]
    MD_REF_TYPE_FOOTNOTE        // [^footnote]: paragraph text of footnote

  Verifies the reference is valid (no illegal chars), and returns NULL if not valid.

  Returns pointer to the byte after the parsed markdown if valid.

  Example inputs:

    ![alt](link "title")
    [text](link)

  @param  pAltLink    structure to store all the found data
  @param  szMarkdown  pointer to image or reference
  Returns NULL if not a markdown image or reference, other wise pointer to after reference
*///-----------------------------------------------------------------------------------------------
char * FlyMdAltLink(flyMdAltLink_t *pAltLink, const char *szMarkdown)
{
  const char *pszMd;
  const char *pszStart;
  const char *pszEnd;
  const char *pszRefEnd   = NULL;
  bool_t      fOk         = TRUE;
  bool_t      fFootnote   = FALSE;

  // initialize *pAltLink
  memset(pAltLink, 0, sizeof(*pAltLink));

  // find and verify [alt] field
  pszMd = szMarkdown;
  if(*pszMd == '!')
  {
    pAltLink->refType = MD_REF_TYPE_IMAGE;
    ++pszMd;
  }
  if(*pszMd != '[')
    fOk = FALSE;

#if MD_DEBUG_ALTLINK
  FlyDbgPrintf("--- FlyMdAltLink(%.*s, fOk %u, refType %u) ---\n", (unsigned)FlyStrLineLen(szMarkdown), szMarkdown, fOk, pAltLink->refType);
#endif

  // find [alt] text and length
  if(fOk)
  {
    ++pszMd;
    pszEnd = MdLinePBrk(pszMd, "]");
    if(!pszEnd)
      fOk = FALSE;
    else
      pAltLink->altLen  = (unsigned)(pszEnd - pszMd);

    // non-image references MUST have text for the link
    // it's OK for images to have no empty alt text
    if(pAltLink->altLen == 0 && pAltLink->refType != MD_REF_TYPE_IMAGE)
      fOk = FALSE;

    if(fOk)
    {
      pAltLink->szAlt = pszMd;
      pszMd = pszEnd + 1;   // passed ending bracket ]

      // check for footnote references or the footnote itself, e.g. [^footnote] or [^footnote]: paragraph
      if(pAltLink->altLen > 1 && *pAltLink->szAlt == '^')
      {
        fFootnote = TRUE;
        if(*pszMd == ':')
        {
          pAltLink->refType = MD_REF_TYPE_FOOTNOTE;
          ++pszMd;
        }
        else
          pAltLink->refType = MD_REF_TYPE_FOOT_REF;
        pszRefEnd = pszMd;
      }
    }

#if MD_DEBUG_ALTLINK
  FlyDbgPrintf("  fOk %u, pszMd '%.*s', szAlt %p, altLen %u, refType %u, fFootnote %u\n",
          fOk, (unsigned)FlyStrLineLen(pszMd), pszMd, pAltLink->szAlt, pAltLink->altLen, pAltLink->refType, fFootnote);
#endif
  }

  if(!fFootnote)
  {
    // find link (or end of ref)
    if(fOk)
    {
      if(*pszMd != '(')
        fOk = FALSE;
      else
      {
        // ok to have whitespace before link
        pszMd = pszStart = FlyStrSkipWhite(pszMd + 1);

        // must be link before title
        if(*pszMd == '"')
          fOk = FALSE;
      }

      if(fOk)
      {
        pszEnd = MdLinePBrk(pszMd, " \t\")");

        // must have space (before title) or end of reference
        if(!pszEnd || (!isblank(*pszEnd) && *pszEnd != ')'))
          fOk = FALSE;
        else
        {
          pszMd = FlyStrSkipWhite(pszEnd);
          if(*pszMd != '"' && *pszMd != ')')
            fOk = FALSE;
          else
          {
            pAltLink->szLink = pszStart;
            pAltLink->linkLen = pszEnd - pszStart;
            if(pAltLink->linkLen == 0)
              fOk = FALSE;
          }
        }
      }
  #if MD_DEBUG_ALTLINK
    FlyDbgPrintf("  fOk %u, pszMd '%.*s', szLink %p, linkLen %u\n", fOk, (unsigned)FlyStrLineLen(pszMd), pszMd, pAltLink->szLink, pAltLink->linkLen);
  #endif
    }

    // find optional title
    if(fOk && *pszMd == '"')
    {
      // there must be an end quote before end of line
      pszEnd = FlyStrEscEndQuoted(pszMd);
      if(pszEnd == pszMd)
        fOk = FALSE;
      else
      {
        pAltLink->szTitle = pszMd + 1;
        pAltLink->titleLen = pszEnd - pAltLink->szTitle;
        pszMd = FlyStrSkipWhite(pszEnd + 1);
      }

      // only images can have "title" field
      if(fOk && pAltLink->szTitle && pAltLink->refType != MD_REF_TYPE_IMAGE)
        fOk = FALSE;
  #if MD_DEBUG_ALTLINK
    FlyDbgPrintf("  fOk %u, pszMd '%.*s', szTitle %p, titleLen %u\n", fOk, (unsigned)FlyStrLineLen(pszMd), pszMd, pAltLink->szTitle, pAltLink->titleLen);
  #endif
    }

    // find end of reference, next character must be ')'
    if(fOk)
    {
      if(*pszMd == ')')
        pszRefEnd = pszMd + 1;
      else
        fOk = FALSE;
    }
  }

  // if reftype still not set, it must be a normal reference, not link or footnote
  if(fOk && pszRefEnd && pAltLink->refType == MD_REF_TYPE_NONE)
    pAltLink->refType = MD_REF_TYPE_REF;

  else if (!fOk)
    pAltLink->refType = MD_REF_TYPE_NONE;

#if MD_DEBUG_ALTLINK
  FlyDbgPrintf("  fOk %u, szRefEnd %p, refType %u, pszMd '%.*s'\n", 
    fOk, pszRefEnd, pAltLink->refType, (unsigned)FlyStrLineLen(pszMd), pszMd);
  FlyDbgPrintf("  szAlt %.*s, altLen %u, szLink %.*s, linkLen %u, szTitle %.*s, titleLen %u\n",
    pAltLink->altLen, FlyStrNullOk(pAltLink->szAlt), pAltLink->altLen,
    pAltLink->linkLen, FlyStrNullOk(pAltLink->szLink), pAltLink->linkLen,
    pAltLink->titleLen, FlyStrNullOk(pAltLink->szTitle), pAltLink->titleLen);
#endif

  return (char *)pszRefEnd;
}

/*!------------------------------------------------------------------------------------------------
  Converts from markdown img link ![alt](link "title") to HTML

  Example output: `<img src="link" alt="alt" title="title">`

  The optional "title" is overloaded to affect the class and style of the image.

  1. If no title, then no class, style or title
  2. If title begins with "w3_", then attributes class="title" style="width:150px"
  3. If title contains "class" and/or "style", then it's treated as attributes at end of image tag
  4. Otherwise title only, no special attributes

  Note: if either option 2. or 3. is used above, there is no title string.

  Note: how in option 3, the quotes must be escaped.

  Examples:

      Option 1: ![Simple](image.png)  
      Option 2: ![Circle Image](image.png "w3-circle")  
      Option 3: ![More Control](image.png "class=\"w3-round\" style=\"width:80%\"")  
      Option 4: ![With Title](image.png "just some title")  

  ppszMd is both input and output. It is advanced to end of "consumed" markdown

  TODO: think about encoding, use &amp;, &quot; etc...
  see <https://stackoverflow.com/questions/1165635/does-the-img-tags-alt-attribute-require-encoding>

  @param  szHtml    ptr to char array or NULL to just get length of resulting HTML
  @param  size      sizeof(szHtml)
  @param  ppszMd    ptr to markdown ptr, both input and output
  @param  szAttr    NULL or any extra attributes, e.g. style="width:150px"
  @return len of resulting "<img ...>" or 0 if not a valid markdown image link
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlImage(char *szHtml, size_t size, const char **ppszMd)
{
  // HTML
  static const char   szImgOpen[]     = "<img src=\"";
  static const char   szAltOpen[]     = " alt=\"";
  static const char   szStyle150px[]  = "\" style=\"width:150px\"";
  static const char   szClassOpen[]   = " class=\"";
  static const char   szTitleOpen[]   = " title=\"";

  flyMdAltLink_t      altLink;
  const char         *szRefEnd;
  size_t              htmlLen         = 0;

  // string must always end in '\0'
  if(szHtml)
    *szHtml = '\0';
 
  szRefEnd = FlyMdAltLink(&altLink, *ppszMd);
  if(szRefEnd && altLink.refType == MD_REF_TYPE_IMAGE)
  {
    // src=
    htmlLen += FlyStrZCpy(szHtml, szImgOpen, size);
    htmlLen += FlyStrZNCat(szHtml, altLink.szLink, size, altLink.linkLen);
    htmlLen += FlyStrZCat(szHtml, m_szQuote, size);

    // alt=
    htmlLen += FlyStrZCat(szHtml, szAltOpen, size);
    htmlLen += FlyStrZNCat(szHtml, altLink.szAlt, size, altLink.altLen);
    htmlLen += FlyStrZCat(szHtml, m_szQuote, size);

    // 1. If no title, then no class or style
    // e.g. <img src=image.png alt="alt">
    if(altLink.szTitle)
    {
      // 2. If title begins with "w3_", then class= the string and style="width:150px"
      // e.g. <img src=image.png alt="alt" class="w3-circle">
      if(strncmp(altLink.szTitle, "w3-", 3) == 0)
      {
        htmlLen += FlyStrZCat(szHtml, szClassOpen, size);
        htmlLen += FlyStrZNCat(szHtml, altLink.szTitle , size, altLink.titleLen);
        htmlLen += FlyStrZCat(szHtml, szStyle150px, size);
      }

      // 3. If title is anything else, is copied as-is to be used for class and style
      // e.g. <img src=image.png alt="alt" class="w3-circle" style="width:80%">
      else if(FlyStrNStr(altLink.szTitle, "class", altLink.titleLen) || 
              FlyStrNStr(altLink.szTitle, "style", altLink.titleLen))
      {
        htmlLen += FlyStrZCat(szHtml, m_szSpace, size);
        htmlLen += MdNCat(szHtml, altLink.szTitle, size, altLink.titleLen);
      }

      // 4. Otherwise title only, no special attributes
      // e.g. <img src=image.png alt="alt" title="some title">
      else
      {
        htmlLen += FlyStrZCat(szHtml, szTitleOpen, size);
        htmlLen += FlyStrZNCat(szHtml, altLink.szTitle , size, altLink.titleLen);
        htmlLen += FlyStrZCat(szHtml, m_szQuote, size);
      }
    }

    htmlLen += FlyStrZCat(szHtml, m_szEndBracket, size);
    *ppszMd = szRefEnd;
  }

  return htmlLen;
}

/*!------------------------------------------------------------------------------------------------
  Converts one of four types of from markdown references to HTML, or returns 0

      MD_REF_TYPE_IMAGE,          // ![alt text](file.png "title")
      MD_REF_TYPE_REF,            // [ref text](site.com/page)
      MD_REF_TYPE_FOOT_REF,       // [^footnote]
      MD_REF_TYPE_FOOTNOTE        // [^footnote]: paragraph text of footnote

  Example output based on the above 4 types:

      <img src="file.png" alt="alt text" title="title">
      <a href="site.com/page">ref text</a>
      <a href="#footnote">[^footnote]</a>


  See also FlyMd2HtmlIsRef()

  ppszMd is both input and output. It is advanced to end of "consumed" markdown

  @param  szHtml    ptr to char array or NULL to just get size of resulting HTML
  @param  size      sizeof(szHtml)
  @param  ppszMd    ptr to ptr to reference, e.g. [text](link "title")
  @return len of resulting HTML or 0 if not a valid markdown reference
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlRef(char *szHtml, size_t size, const char **ppszMd)
{
  static const char   szRefOpen[]       = "<a href=\"";
  static const char   szRefMiddle[]     = "\">";
  static const char   szRefClose[]      = "</a>";

  static const char   szRefFootMiddle[] = "\">[";   // szRefFootOpen[] is same as szRefOpen[]
  static const char   szRefFootClose[]  = "]</a>";

  static const char   szFootnoteOpen[]  = "<p id=\"";
  static const char   szFootnoteClose[] = "\">";

  flyMdAltLink_t      altLink;
  const char         *szRefEnd;
  const char         *psz;
  size_t              htmlLen     = 0;
#if MD_DEBUG_REF
  char               *szHtmlOrg   = szHtml;
#endif

  szRefEnd = FlyMdAltLink(&altLink, *ppszMd);
  if(szRefEnd && altLink.refType != MD_REF_TYPE_NONE)
  {
    // handle image separatelty
    if(altLink.refType == MD_REF_TYPE_IMAGE)
    {
      psz = *ppszMd;
      htmlLen = FlyMd2HtmlImage(szHtml, size, &psz);
    }

    // [ref text](site.com/page)  =>
    // <a href="site.com/page">ref text</a>
    else if(altLink.refType == MD_REF_TYPE_REF)
    {
      FlyAssert(altLink.szLink && altLink.linkLen);
      htmlLen += FlyStrZCpy(szHtml, szRefOpen, size);
      htmlLen += FlyStrZNCat(szHtml, altLink.szLink, size, altLink.linkLen);
      htmlLen += FlyStrZCat(szHtml, szRefMiddle, size);
      htmlLen += FlyStrZNCat(szHtml, altLink.szAlt, size, altLink.altLen);
      htmlLen += FlyStrZCat(szHtml, szRefClose, size);
    }

    // [^footnote]  =>
    // <a href="#footnote"><[^footnote]</a>
    else if(altLink.refType == MD_REF_TYPE_FOOT_REF)
    {
      FlyAssert(altLink.szAlt && altLink.altLen > 1);
      htmlLen += FlyStrZCpy(szHtml, szRefOpen, size);
      htmlLen += FlyStrZCat(szHtml, "#", size);
      szHtml = MdAdjust(szHtml, &size);
#if MD_DEBUG_REF
      if(szHtmlOrg)
        FlyDbgPrintf("Footnote b4 slug: %s", szHtmlOrg);
#endif
      htmlLen += FlyStrSlug(szHtml, altLink.szAlt, size, altLink.altLen);
#if MD_DEBUG_REF
      if(szHtmlOrg)
        FlyDbgPrintf("Footnote af slug: %s", szHtmlOrg);
#endif
      htmlLen += FlyStrZCat(szHtml, szRefFootMiddle, size);
      htmlLen += FlyStrZNCat(szHtml, altLink.szAlt, size, altLink.altLen);
      htmlLen += FlyStrZCat(szHtml, szRefFootClose, size);
#if MD_DEBUG_REF
      if(szHtmlOrg)
        FlyDbgPrintf("Footnote Ref: %s", szHtmlOrg);
#endif
      }

    // [^footnote]: paragraph text of footnote
    // becomes <p id="footnote">
    // note: paragraph is not closed, just given an id
    else if(altLink.refType == MD_REF_TYPE_FOOTNOTE)
    {
      FlyAssert(altLink.szAlt && altLink.altLen > 1);
      htmlLen += FlyStrZCpy(szHtml, szFootnoteOpen, size);
      szHtml = MdAdjust(szHtml, &size);
      htmlLen += FlyStrSlug(szHtml, altLink.szAlt, size, altLink.altLen);
      htmlLen += FlyStrZCat(szHtml, szFootnoteClose, size);
    }
  }

  if(htmlLen)
    *ppszMd = szRefEnd;
  return htmlLen;
}

/*!------------------------------------------------------------------------------------------------
  Convert markdown heading into an HTML heading.

  ppszMd is both input and output. It is advanced to end of "consumed" markdown.

  Level is limited to 6, that is `###### Heading Title`

  @param  szHtml    ptr to char array or NULL to just get size of resulting HTML
  @param  pSize     both input/output
  @param  ppszMd    ptr to ptr to markdown string (so ptr can be advanced)
  @param  szClass   optional color class, e.g. "w3-red", or NULL for none
  @return length of HTML heading
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlHeading(char *szHtml, size_t size, const char **ppszMd, const char *szW3Color)
{
  const char    szFmtHdrOpen[]   = "<h%u id=\"";
  const char    szFmtHdrClass[]  = "\" class=\"";
  const char    szHdrMiddle[]    = "\">";
  const char    szFmtHdrClose[]  = "</h%u>\r\n";
  const char   *pszEndHdr;
  const char   *pszHdrText;
  char          szTag[sizeof(szFmtHdrOpen) + 2];   // e.g. "<h2 id=\"
  size_t        htmlLen   = 0;
  unsigned      level;
  size_t        len;

  // level is limited to 6
  pszEndHdr = MdIsHeading(*ppszMd, &level);
  if(pszEndHdr)
  {
    // heading open
    sprintf(szTag, szFmtHdrOpen, level);
    htmlLen += FlyStrZCpy(szHtml, szTag, size);

    // heading ID
    pszHdrText = FlyMd2HtmlHeadingText(*ppszMd);
    FlyAssert(pszHdrText);
    szHtml = MdAdjust(szHtml, &size);
    htmlLen += FlyStrSlug(szHtml, pszHdrText, size, FlyStrLineLen(pszHdrText));

    // optional heading color
    if(szW3Color)
    {
      len = strlen(szW3Color);
      FlyStrZCat(szHtml, szFmtHdrClass, size);
      FlyStrZCat(szHtml, szW3Color, size);
      htmlLen += strlen(szFmtHdrClass) + len;
    }

    // end the tag and copy title in text form
    htmlLen += FlyStrZCat(szHtml, szHdrMiddle, size);
    len = FlyStrLineEnd(pszHdrText) - pszHdrText;
    htmlLen += FlyStrZNCat(szHtml, pszHdrText, size, len);

    // end the tag
    sprintf(szTag, szFmtHdrClose, level);
    htmlLen += FlyStrZCat(szHtml, szTag, size);

    // on to next line
    *ppszMd = pszEndHdr;
  }

  return htmlLen;
}

/*-------------------------------------------------------------------------------------------------
  Recursive helper function to FlyMd2HtmlList(). 

  This function creates HTML list for one level from markdown, and recurses to handle deeper levels.

  @param  szHtml        ptr to char array or NULL to just get size of resulting HTML
  @param  size          
  @param  ppszMdList    ptr to a list line in markdown string
  @param  indent        indent level (e.g. 2 or 4 bytes)
  @param  level         0-n
  @return ptr to line after list in szMd
*///-----------------------------------------------------------------------------------------------
static size_t MdListMake(char *szHtml, size_t size, const char **ppszMdList, unsigned indent, unsigned level)
{
  const char   *szMdListLine = *ppszMdList;
  const char   *szLine;
  const char   *szItem;
  unsigned      thisIndent;
  unsigned      checkbox;
  mdListType_t  type;
  mdListType_t  thisType;
  const char    szOrdered[]       = "<ol>\r\n";
  const char    szOrderedEnd[]    = "</ol>\r\n";
  const char    szUnordered[]     = "<ul>\r\n";
  const char    szUnorderedEnd[]  = "</ul>\r\n";
  const char    szListItem[]      = "<li>";
  const char    szListItemEnd[]   = "</li>\r\n";
  const char    szLineEnd[]       = "\r\n";
  const char    szCheckbox[]      = "<input type=\"checkbox\" id=\"";
  const char    szChecked[]       = " checked=\"true\"";
  const char    szCheckBoxEnd[]   = "> ";
  bool_t        fEndListItem;
  size_t        htmlLen = 0;

  // validate parameters
  szMdListLine = *ppszMdList;
  FlyAssert(FlyMd2HtmlIsList(szMdListLine, NULL));

  // open the list
  htmlLen += FlyStrZCatFill(szHtml, ' ', size, level * 2);
  MdListType(szMdListLine, &type, NULL);
  if(type == FLY_MD_LIST_TYPE_ORDERED)
    htmlLen += FlyStrZCat(szHtml, szOrdered, size);
  else
    htmlLen += FlyStrZCat(szHtml, szUnordered, size);

  // fill in the list line items
  szLine = szMdListLine;
  fEndListItem = FALSE;
  while(FlyMd2HtmlIsList(szLine, NULL))
  {
    thisIndent = FlyStrLineIndent(szLine, FLY_STR_TAB_SIZE);
    szItem = MdListType(szLine, &thisType, &checkbox);

    // don't close the line yet, as we may have an inner list
    if(thisIndent > indent)
    {
      htmlLen += FlyStrZCat(szHtml, szLineEnd, size);
      szHtml = MdAdjust(szHtml, &size);
      htmlLen += MdListMake(szHtml, size, &szLine, thisIndent, level + 1);
      htmlLen += FlyStrZCatFill(szHtml, ' ', size, level * 2);
      fEndListItem = TRUE;
    }

    else if(thisIndent == indent)
    {
      if(fEndListItem)
      {
        htmlLen += FlyStrZCat(szHtml, szListItemEnd, size);
        fEndListItem = FALSE;
      }
      htmlLen += FlyStrZCatFill(szHtml, ' ', size, level * 2);
      htmlLen += FlyStrZCat(szHtml, szListItem, size);
      if(checkbox)
      {
        htmlLen += FlyStrZCat(szHtml, szCheckbox, size);
        szHtml = MdAdjust(szHtml, &size);
        htmlLen += FlyStrSlug(szHtml, szItem, size, FlyStrLineLen(szItem));
        htmlLen += FlyStrZCat(szHtml, m_szQuote, size);
        if(checkbox == FLY_MD_LIST_CHECK_BOX_CHECKED)
          htmlLen += FlyStrZCat(szHtml, szChecked, size);
        htmlLen += FlyStrZCat(szHtml, szCheckBoxEnd, size);
      }
      htmlLen += FlyMd2HtmlTextLine(szHtml ? szHtml + strlen(szHtml) : szHtml, size, &szItem, FlyStrLineEnd(szItem));
      fEndListItem = TRUE;
      szLine = FlyStrLineNext(szLine);
    }

    else
    {
      break;
    }
  }

  if(fEndListItem)
    htmlLen += FlyStrZCat(szHtml, szListItemEnd, size);

  // close the ordered/unordered list
  htmlLen += FlyStrZCatFill(szHtml, ' ', size, level * 2);
  if(type == FLY_MD_LIST_TYPE_ORDERED)
    htmlLen += FlyStrZCat(szHtml, szOrderedEnd, size);
  else
    htmlLen += FlyStrZCat(szHtml, szUnorderedEnd, size);

  *ppszMdList = szLine;
  return htmlLen;
}

/*!------------------------------------------------------------------------------------------------
  Convert markdown list into HTML list.

  Handles nested lists, both numeric and not.

  ppszMd is both input and output. It is advanced to end of "consumed" markdown

  @param  szHtml    ptr to char array or NULL to just get size of resulting HTML
  @param  pSize     both input/output
  @param  ppszMd    ptr to ptr to markdown string
  @return ptr to line after list, or szMd if not a list
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlList(char *szHtml, size_t size, const char **ppszMd)
{
  const char   *szMd = *ppszMd;
  unsigned      orgIndent;
  size_t        htmlLen = 0;

  if(FlyMd2HtmlIsList(szMd, NULL) && size > 1)
  {
    orgIndent = FlyStrLineIndent(szMd, FLY_STR_TAB_SIZE);
    if(szHtml)
      *szHtml = '\0';
    htmlLen = MdListMake(szHtml, size, &szMd, orgIndent, 0);
  }

  if(htmlLen)
    *ppszMd = szMd;
  return htmlLen;
}

/*!------------------------------------------------------------------------------------------------
  Find matching 

  Handles nested lists, both numeric and not.

  ppszMd is both input and output. It is advanced to end of "consumed" markdown

  @param  pTypeInfo   A found type info at szMd
  @param  szMd        ptr to markdown that produced pTypeInfo
  @param  szMdEnd     end of markdown to search
  @return NULL if match not found, or ptr to match
*///-----------------------------------------------------------------------------------------------
static char * MdEmMatch(const mdEmTypeInfo_t *pTypeInfo, const char *szMd, const char *szMdEnd)
{
  char        szAccept[2];
  const char *szFound = NULL;
  unsigned    count;

  szAccept[0] = pTypeInfo->marker;
  szAccept[1] = '\0';
  szMd += pTypeInfo->len;

  while(szMd < szMdEnd)
  {
    szFound = FlyMdNPBrk(szMd, szMdEnd, szAccept);
    if(!szFound)
      break;
    count = FlyStrChrCount(szFound, *szAccept);
    if(count == pTypeInfo->len)
      break;
    else
      szMd = szFound + count;
  }

  return (char *)szFound;
}


/*!------------------------------------------------------------------------------------------------
  Convert a line or less of text from markdown into HTML.

  Converts [text](link.com), `code`, **bold**, \` etc...

  This does NOT add the <br> at the end of the line for 2 spaces.

  This is used to convert paragraph lines, list items, table cells.

  Param `ppszMd` is both input and output. It is advanced to end of "consumed" markdown.

  Note: `if(*szMdEnd == ']')` then it won't process references, as reference text can't contain a
  reference.

  @param  szHtml    ptr to char array or NULL to just get length of resulting HTML
  @param  size      size of szHtml
  @param  ppszMd    both input and output, start of markdown
  @param  szMdEnd   ptr to byte AFTER end of text
  @return length of output HTML
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlTextLine(char *szHtml, size_t size, const char **ppszMd, const char *szMdEnd)
{
  char                 *szHtmlOrg = szHtml;
  const char           *psz;
  const char           *psz2;
  size_t                n;
  size_t                htmlLen = 0;
  flyMdRefType_t        refType;
  const mdEmTypeInfo_t *pEmTypeInfo;
  bool_t                afClose[MD_EM_TYPE_SIZEOF];
  unsigned              i;

  // do NOT allow bad markdown ptrs
  FlyAssert(*ppszMd && szMdEnd && *ppszMd < szMdEnd);

  // start with a zero terminated string, as we will be concatinating
  (void)szHtmlOrg;    // debugging
  if(szHtml)
    *szHtml = '\0';  

// if(szHtmlOrg) // BUGBUG
//   FlyDbgPrintf("FlyMd2HtmlTextLine(%.*s), size %zu\n", (unsigned)(szMdEnd - *ppszMd), *ppszMd, size);

  // psz2 = FlyStrLineChrMatch(psz, "`![<"
  memset(afClose, 0, sizeof(afClose));
  psz = psz2 = *ppszMd;
  while(psz < szMdEnd)
  {
    // if(szHtmlOrg && *szHtmlOrg) // BUGBUG
    //   FlyDbgPrintf("  %s, size %zu, md %s, htmlLen %zu\n", szHtmlOrg, size, psz, htmlLen);

    // find potential next special sequence not just plain (or escaped) text
    psz2 = FlyMdNPBrk(psz, szMdEnd, m_szMdSpecial);
    if(psz2 == NULL)
      psz2 = szMdEnd;
    // else
    //   FlyDbgPrintf("special %.*s\n", (int)FlyStrLineLen(psz2), psz2);
    if(psz2 > psz)
    {
      htmlLen += MdNCat(szHtml, psz, size, psz2 - psz);
      szHtml = MdAdjust(szHtml, &size);
    }
    if(psz2 == szMdEnd)
      break;

    // this may span multiple other types of things, so do inline `code` 1st
    // backtick starts `code`
    if(*psz2 == '`')
      htmlLen += FlyMd2HtmlCodeIn(szHtml, size, &psz2);

    // handle quick links, e.g. <me@mysite.com>, <https://www.w3schools.com/css/css_intro.asp> <#local_link>
    else if(FlyMd2HtmlIsQLink(psz2))
      htmlLen += FlyMd2HtmlQLink(szHtml, size, &psz2);

    // image or reference, e.g. [ref text](link.com) or ![image text](file.png "opt title")
    else if(*psz2 == '!' || *psz2 == '[')
    {
      // is this a valid reference?
      refType = FlyMd2HtmlIsRef(psz2);
      // handles image, reference, footnote reference
      if(refType == MD_REF_TYPE_IMAGE)
        htmlLen += FlyMd2HtmlImage(szHtml, size, &psz2);
      else if(refType == MD_REF_TYPE_REF ||  refType == MD_REF_TYPE_FOOT_REF)
        htmlLen += FlyMd2HtmlRef(szHtml, size, &psz2);

      // MD_REF_TYPE_FOOTNOTE already handled at paragraph level
      // however, if user wrote a bad line such as:
      // [^footnote]: some text [^bad footnote, must be on different line]: more text
      else
      {
        refType = MD_REF_TYPE_NONE;
      }

      // not a valid reference, just copy the "![" or "[" sequence, and process after that
      if(refType == MD_REF_TYPE_NONE)
      {
        if(*psz2 == '!')
        {
          htmlLen += FlyStrZCat(szHtml, "!", size);
          ++psz2;
        }
        if(*psz2 == '[')
        {
          htmlLen += FlyStrZCat(szHtml, "[", size);
          ++psz2;
        }
      }
    }

    // handle emphasis characters, e.g. **bold** or ==highlight==
    else if(strchr(m_szMdEmMarkers, *psz2))
    {
      pEmTypeInfo = MdEmTypeInfoGet(psz2);
      if(pEmTypeInfo)
      {
        // if opening, there must be a pair, or it's not emphasis
        if(!afClose[pEmTypeInfo->type] && !MdEmMatch(pEmTypeInfo, psz2, szMdEnd))
          pEmTypeInfo = NULL;
        else
        {
          // open or close the emphasis by type
          htmlLen += FlyMd2HtmlEmphasis(szHtml, size, &psz2, afClose[pEmTypeInfo->type]);
          afClose[pEmTypeInfo->type] = afClose[pEmTypeInfo->type] ? FALSE : TRUE;
        }
      }
      if(!pEmTypeInfo)
      {
        n = FlyStrChrCount(psz2, *psz2);
        htmlLen += FlyStrZFill(szHtml, *psz2, size, n);
        psz2 += n;
      }
    }

    // handle special characters to not confuse HTML
    else if(*psz2 == '&')
    {
      htmlLen += FlyStrZCat(szHtml, "&amp;", size);
      ++psz2;
    }
    else if(*psz2 == '<')
    {
      htmlLen += FlyStrZCat(szHtml, "&lt;", size);
      ++psz2;
    }

    // should never get here, as it should have been handled above
    else
    {
      FlyAssertFail("Missing Special");
    }

    // for things that don't concatenate
    szHtml = MdAdjust(szHtml, &size);

    psz = psz2;
  }

  // but endings on all markers still open on the line
  for(i = 1; i < NumElements(afClose); ++i)
  {
    char szMdType[4];
    if(afClose[i])
    {
      pEmTypeInfo = &m_aEmMdTypeInfo[i - 1];
      FlyStrZFill(szMdType, pEmTypeInfo->marker, sizeof(szMdType), pEmTypeInfo->len);
      psz = szMdType;
      htmlLen += FlyMd2HtmlEmphasis(szHtml, size, &psz, afClose[i]);
      szHtml = MdAdjust(szHtml, &size);
    }
  }

  *ppszMd = szMdEnd;
  return htmlLen;
}

/*!------------------------------------------------------------------------------------------------
  Convert markdown paragraph into HTML. Converts [text](ref) and `code`, and `  ` line break

  On input, *ppszMd must point to a non-blank line.

  Stops on first blank line, or on a triple tick line, or on #Heading lines.

  ppszMd is both input and output. It is advanced to end of "consumed" markdown

  @param  szHtml    ptr to char array or NULL to just get size of resulting HTML
  @param  pSize     both input/output
  @param  ppszMd    ptr to ptr to markdown string, advanced forward
  @return size of HTML, or 0 if not a paragraph (blank line)
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlPara(char *szHtml, size_t size, const char **ppszMd)
{
  const char  *szMd         = *ppszMd;
  const char  *psz;
  const char  *szLine;
  const char  *szNextLine;
  size_t      htmlLen       = 0;
  bool_t      fBackTicks;

  // paragraph must start on non-blank
  szLine = szMd;
  if(szLine && !FlyStrLineIsBlank(szLine))
  {
    if(szHtml)
      *szHtml = '\0';

#if MD_DEBUG_PARA
    FlyDbgPrintf("FlyMd2HtmlPara(%.*s)\n", (int)FlyStrLineLen(szLine), szLine);
#endif

    // open the paragraph, possibly with id
    if(FlyMd2HtmlIsRef(szLine) == MD_REF_TYPE_FOOTNOTE)
    {
#if MD_DEBUG_PARA
    FlyDbgPrintf("  ...is footnote\n");
#endif
      psz = szLine;
      htmlLen += FlyMd2HtmlRef(szHtml, size, &psz);
    }
    else
    {
#if MD_DEBUG_PARA
    FlyDbgPrintf("  ...is normal\n");
#endif
      htmlLen += FlyStrZCpy(szHtml, "<p>", size);
    }

    while(*szLine && !FlyStrLineIsBlank(szLine))
    {
      // don't count indented blank lines as they may be part of the paragraph
      if(FlyMd2HtmlIsCodeBlk(szLine, &fBackTicks) && fBackTicks)
        break;
      if(FlyMd2HtmlIsHeading(szLine, NULL))
        break;
      if(FlyMd2HtmlIsHorzRule(szLine))
        break;

      szNextLine = FlyStrLineNext(szLine);
      psz = szLine;
      htmlLen += FlyMd2HtmlTextLine(szHtml ? szHtml + strlen(szHtml) : szHtml, size, &psz, FlyStrLineEnd(szLine));
      if(FlyMd2HtmlIsBreak(szLine))
        htmlLen += FlyStrZCat(szHtml, "<br>", size);

      // end paragraph on same line as last text
      if(!FlyStrLineIsBlank(szNextLine))
        htmlLen += FlyStrZCat(szHtml, "\r\n", size);

      szLine = szNextLine;

      // if(szHtml)
      //   FlyStrDump(szHtml, strlen(szHtml) + 1);
    }
    htmlLen += FlyStrZCat(szHtml, "</p>\r\n", size);
    *ppszMd = szLine;
  }

  return htmlLen;
}

/*!------------------------------------------------------------------------------------------------
  Convert markdown quick linke into an HTML anchor.

  ppszMd is both input and output. It is advanced to end of "consumed" markdown

  Exmples: <https://duckduckgo.com>, <#local_ref>, <me@mysite.com>

      <a href="https://duckduckgo.com">https://duckduckgo.com</a>
      <a href="#local_ref">#local_ref</a>
      <a href="mailto:me@mysite.com">me@mysite.com</a>

  @param  szHtml    ptr to char array or NULL to just get size of resulting HTML
  @param  pSize     both input/output
  @param  ppszMd    ptr to ptr to markdown string
  @return ptr to line after list, or szMd if not a list
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlQLink(char *szHtml, size_t size, const char **ppszMd)
{
  const char   *szMd = *ppszMd;
  const char   *psz;
  const char    szRefOpen[] = "<a href=\"";
  const char    szMailTo[] = "mailto:";
  const char    szRefClose1[] = "\">";
  const char    szRefClose2[] = "</a>";
  bool_t        fMailTo = FALSE;
  unsigned      mdLen;
  unsigned      htmlLen = 0;

  if(FlyMd2HtmlIsQLink(szMd))
  {
    psz = FlyStrLineChr(szMd, '>');
    if(psz)
    {
      mdLen = (unsigned)(psz - (szMd + 1));

      if(mdLen)
      {
        if(FlyStrNChr(szMd, mdLen, '@'))
          fMailTo = TRUE;

        htmlLen += FlyStrZCpy(szHtml, szRefOpen, size);
        if(fMailTo)
          htmlLen += FlyStrZCat(szHtml, szMailTo, size);
        htmlLen += FlyStrZNCat(szHtml, szMd + 1, size, mdLen);
        htmlLen += FlyStrZCat(szHtml, szRefClose1, size);
        htmlLen += FlyStrZNCat(szHtml, szMd + 1, size, mdLen);
        htmlLen += FlyStrZCat(szHtml, szRefClose2, size);
        mdLen += 2;
      }
    }
  }

  if(htmlLen)
    *ppszMd += mdLen;
  return htmlLen;
}

/*!------------------------------------------------------------------------------------------------
  Convert markdown tabls into an HTML tables using W3.

  ppszMd is both input and output. It is advanced to end of "consumed" markdown

  Name | Occupation | Salary
  :--- | :--------: | -----:
  Bob  | Plumber    | 100K
  Joe  | Salesman   | 120K
  Jane | Programmer | 150K

  a|b|c
  ---|---|---
  d|e|f

  @param  szHtml    ptr to char array or NULL to just get size of resulting HTML
  @param  pSize     both input/output
  @param  ppszMd    ptr to ptr to markdown string
  @return ptr to line after list, or szMd if not a list
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlTable(char *szHtml, size_t size, const char **ppszMd)
{
  static const char szTableOpen[]   = "<table class=\"w3-table-all\" style=\"width:auto\">\r\n";
  static const char szRowOpen[]     = "<tr>\r\n";
  static const char szHdrLeft[]     = "  <th>";
  static const char szHdrRight[]    = "  <th class=\"w3-right-align\">";
  static const char szHdrCenter[]   = "  <th class=\"w3-center\">";
  static const char szHdrClose[]    = "</th>\r\n";
  static const char szCellLeft[]    = "  <td>";
  static const char szCellRight[]   = "  <td class=\"w3-right-align\">";
  static const char szCellCenter[]  = "  <td class=\"w3-center\">";
  static const char szCellClose[]   = "</td>\r\n";
  static const char szRowClose[]    = "</tr>\r\n";
  static const char szTableClose[]  = "</table>\r\n";
  const char       *szLine;
  const char       *szOpen;
  const char       *szCell;
  const char       *szNextCell;
  unsigned          nCols;
  size_t            htmlLen = 0;
  mdTableColType_t  aColType[FLYMD2HTML_TABLE_COL_MAX];
  size_t            cellLen;
  unsigned          i;

  if(szHtml)
    *szHtml = '\0';

  szLine = *ppszMd;
  nCols = MdTableGetCols(szLine, NumElements(aColType), aColType);
  if(nCols)
  {
    // create header
    htmlLen += FlyStrZCpy(szHtml, szTableOpen, size);
    htmlLen += FlyStrZCat(szHtml, szRowOpen, size);
    szNextCell = szLine;
    for(i = 0; i < nCols; ++i)
    {
      cellLen = 0;
      if(szNextCell)
        szCell = MdTableCellGet(szNextCell, &cellLen, &szNextCell);

      if(aColType[i] == MDTABLECOL_TYPE_RIGHT)
        szOpen = szHdrRight;
      else if(aColType[i] == MDTABLECOL_TYPE_CENTER)
        szOpen = szHdrCenter;
      else
        szOpen = szHdrLeft;
      htmlLen += FlyStrZCat(szHtml, szOpen, size);
      if(cellLen)
        htmlLen += FlyStrZNCat(szHtml, szCell, size, cellLen);
      htmlLen += FlyStrZCat(szHtml, szHdrClose, size);
    }
    htmlLen += FlyStrZCat(szHtml, szRowClose, size);

    // skip header lines, e.g.
    // Name | Description
    // ---- | :---------:
    szLine = FlyStrLineNext(FlyStrLineNext(szLine));

    while(MdLineChr(szLine, '|') != NULL)
    {
      // FlyDbgPrintf("A len %zu, szHtml =\n%s\n", strlen(szHtml), szHtml);
      htmlLen += FlyStrZCat(szHtml, szRowOpen, size);
      szNextCell = szLine;
      for(i = 0; i < nCols; ++i)
      {
        cellLen = 0;
        if(szNextCell)
          szCell = MdTableCellGet(szNextCell, &cellLen, &szNextCell);
        // FlyDbgPrintf("B len %zu, szHtml =\n%s\n", strlen(szHtml), szHtml);

        if(aColType[i] == MDTABLECOL_TYPE_RIGHT)
          szOpen = szCellRight;
        else if(aColType[i] == MDTABLECOL_TYPE_CENTER)
          szOpen = szCellCenter;
        else
          szOpen = szCellLeft;
        htmlLen += FlyStrZCat(szHtml, szOpen, size);

        // FlyDbgPrintf("C len %zu, szHtml =\n%s\n", strlen(szHtml), szHtml);
        if(cellLen)
          htmlLen += FlyMd2HtmlTextLine(szHtml ? szHtml + strlen(szHtml) : szHtml, size - htmlLen, &szCell, szCell + cellLen);

        // FlyDbgPrintf("D len %zu, szHtml =\n%s\n", strlen(szHtml), szHtml);
        htmlLen += FlyStrZCat(szHtml, szCellClose, size);
        // FlyDbgPrintf("E len %zu, szHtml =\n%s\n", strlen(szHtml), szHtml);
      }
      htmlLen += FlyStrZCat(szHtml, szRowClose, size);

      szLine = FlyStrLineNext(szLine);
    }

    htmlLen += FlyStrZCat(szHtml, szTableClose, size);
    *ppszMd = szLine;
  }

  // FlyDbgPrintf("Z len %zu, szHtml =\n%s\n", strlen(szHtml), szHtml);
  return htmlLen;
}

/*!------------------------------------------------------------------------------------------------
  Convert markdown string to an HTML string using W3.CSS. Does not contain front/end matter.

  Note: this is expected to put content inside a W3.CSS HTML framework.

  @param  szHtml    ptr to char buffer or NULL to just get size of resulting HTML
  @param  size      sizeof(szHtml) or SIZE_MAX
  @param  szMd      ptr to HTML string
  @return len of resulting HTML (1-n)
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlContent(char *szHtml, size_t size, const char *szMd, const char *szMdEnd)
{
  const char *szLine;
  char       *pszHtml   = szHtml;
  size_t      htmlLen   = 0;
  size_t      sizeLeft  = size;
  size_t      thisLen;

  // for debugging
  g_szMd = szMd;

  // need room for terminating '\0'
  if(size == SIZE_MAX)
    --size;

#if MD_DEBUG_CONTENT > 1
  FlyDbgPrintf("FlyMd2HtmlContent(szHtml=%p, size %zu, szMd=%p, szMdEnd=%p)\n", szHtml, size, szMd, szMdEnd);
#endif

  szLine = szMd;
  while(*szLine && szLine < szMdEnd)
  {
    // skip empty lines to get to something interesting
    szLine = FlyStrLineSkipBlank(szLine);
    if(szLine >= szMdEnd)
      break;

#if MD_DEBUG_CONTENT
    if(fFlyMarkdownDebug)
    {
      unsigned  row, col;
      row = FlyStrLinePos(szMd, szLine, &col);      
      FlyDbgPrintf("\n-- %u:%u: %.*s\n", row, col, (int)FlyStrLineLen(szLine), szLine);
    }
#endif

    // # Headter Title
    if(FlyMd2HtmlIsHeading(szLine, NULL))
    {
#if MD_DEBUG_CONTENT
      if(fFlyMarkdownDebug)
        FlyDbgPrintf("isHeading\n");
#endif
      thisLen = FlyMd2HtmlHeading(pszHtml, sizeLeft, &szLine, NULL);
    }

    // > block quote
    else if(FlyMd2HtmlIsBlockQuote(szLine))
    {
#if MD_DEBUG_CONTENT
      if(fFlyMarkdownDebug)
        FlyDbgPrintf("isBlockQuote\n");
#endif
      thisLen = FlyMd2HtmlBlockQuote(pszHtml, sizeLeft, &szLine);
    }

    // ---
    else if(FlyMd2HtmlIsHorzRule(szLine))
    {
#if MD_DEBUG_CONTENT
      if(fFlyMarkdownDebug)
        FlyDbgPrintf("isHorzRule\n");
#endif
      thisLen = FlyMd2HtmlHorzRule(pszHtml, sizeLeft, &szLine);
    }

    // ```
    // code
    // ```
    else if(FlyMd2HtmlIsCodeBlk(szLine, NULL))
    {
#if MD_DEBUG_CONTENT
      const char *szNextLine;
      if(fFlyMarkdownDebug)
      {
        szNextLine = FlyStrLineNext(szLine);
        FlyDbgPrintf("isCodeBlock %.*s\n", (int)FlyStrLineLen(szNextLine), szNextLine);
      }
#endif
#if MD_DEBUG_CONTENT
      FlyDbgPrintf("szLine before: %p\n", szLine);
#endif
      thisLen = FlyMd2HtmlCodeBlk(pszHtml, sizeLeft, &szLine, NULL, NULL);
#if MD_DEBUG_CONTENT
      FlyDbgPrintf("szLine after : %p\n", szLine);
#endif
    }

    // 1. List item
    else if(FlyMd2HtmlIsList(szLine, NULL))
    {
#if MD_DEBUG_CONTENT
      if(fFlyMarkdownDebug)
        FlyDbgPrintf("isList\n");
#endif
      thisLen = FlyMd2HtmlList(pszHtml, sizeLeft, &szLine);
    }

    // Left | Middle | Right
    // :--- | :----: | ----:
    // a    |   b    |    c
    else if(FlyMd2HtmlIsTable(szLine))
    {
#if MD_DEBUG_CONTENT
      if(fFlyMarkdownDebug)
        FlyDbgPrintf("isTable\n");
#endif
      thisLen = FlyMd2HtmlTable(pszHtml, sizeLeft, &szLine);
    }

    else
    {
#if MD_DEBUG_CONTENT
      if(fFlyMarkdownDebug)
        FlyDbgPrintf("isPara\n");
#endif
      thisLen = FlyMd2HtmlPara(pszHtml, sizeLeft, &szLine);
    }

#if MD_DEBUG_CONTENT > 1
      if(szHtml)
        FlyDbgPrintf("--- szHtml ---\n%s\n--- End szHtml ---\n", szHtml);
      FlyDbgPrintf("-- thisLen %zu, htmlLen %zu, sizeLeft %zu, size %zu, pszHtml %p\n", thisLen, htmlLen, sizeLeft, size, pszHtml);
#endif
    // nothing to do, we're done
    if(thisLen == 0)
      break;

    htmlLen += thisLen;
    if(szHtml)
      pszHtml += thisLen;
    if(thisLen > sizeLeft)
      thisLen = sizeLeft;
    sizeLeft -= thisLen;
  }

  // done
  g_szMd = NULL;

#if MD_DEBUG_CONTENT
  FlyDbgPrintf("End FlyMd2HtmlContent() => htmlLen %zu\n\n", htmlLen);  
#endif

  return htmlLen;
}

/*!------------------------------------------------------------------------------------------------
  Write the head of an HTML file to the szHtml string

  @param  szHtml    ptr to char array or NULL to just get length of resulting HTML
  @param  size      sizeof(szHtml)
  @param  szTitle   optional Title, may be NULL for "No Title"
  @return len of resulting HTML (1-n)
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlHead(char *szHtml, size_t size, const char *szTitle)
{
  static const char szHead1[] =
    "<!DOCTYPE html>\r\n"
    "<html>\r\n"
    "<head>\r\n"
    "<title>";
  static const char szHead2[] =
    "</title>\r\n"
    "<meta charset=\"UTF-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n"
    "<link rel=\"stylesheet\" href=\"https://www.w3schools.com/w3css/4/w3.css\">\r\n"
    "</head>\r\n"
    "<body>\r\n"
    "<div class=\"w3-cell-row\">\r\n"
    "  <div class=\"w3-container w3-cell w3-mobile\">\r\n";
  size_t  htmlLen = 0;

  htmlLen += FlyStrZCpy(szHtml, szHead1, size);
  htmlLen += FlyStrZCat(szHtml, szTitle ? szTitle : "No Title", size);
  htmlLen += FlyStrZCat(szHtml, szHead2, size);

  return htmlLen;
}

/*!------------------------------------------------------------------------------------------------
  Write the end of an HTML file to the szHtml string that was opened with FlyMdHtmlHead()

  @param  szHtml    ptr to char array or NULL to just get size of resulting HTML
  @param  size      sizeof(szHtml)
  @return len of resulting HTML (1-n)
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlEnd(char *szHtml, size_t size)
{
  static const char szEnd[] =
    "  </div>\r\n"
    "</div>\r\n"
    "</body>\r\n"
    "</html>\r\n";

  return FlyStrZCpy(szHtml, szEnd, size);
}

/*!------------------------------------------------------------------------------------------------
  Convert markdown file to an HTML file using W3.CSS 

  This will handle a 

  @param  szHtml    ptr to char array or NULL to just get size of resulting HTML
  @param  sizeHtml  sizeof(szHtml)
  @param  szFile    ptr to file string
  @return len of resulting HTML (1-n)
*///-----------------------------------------------------------------------------------------------
size_t FlyMd2HtmlFile(char *szHtml, size_t size, const char *szMd, const char *szTitle)
{
  size_t  htmlLen = 0;

  htmlLen += FlyMd2HtmlHead(szHtml, size, szTitle);
  htmlLen += FlyMd2HtmlContent(szHtml ? &szHtml[htmlLen] : szHtml, size - htmlLen, szMd, szMd + strlen(szMd));
  htmlLen += FlyMd2HtmlEnd(szHtml ? &szHtml[htmlLen] : szHtml, size - htmlLen);

  return htmlLen;
}
