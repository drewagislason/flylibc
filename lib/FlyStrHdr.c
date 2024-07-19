/**************************************************************************************************
  FlyStrHdr.c - Things that handle C, Python and other block comments (headers).
  Copyright 2024 Drew Gislason
  License: MIT <https://mit-license.org>
*///***********************************************************************************************
#include <stdarg.h>
#include "FlyStr.h"

// for FlyStrHdrFind()
static const char m_szOpenC[]     = "/*";
static const char m_szCloseC[]    = "*/";
static const char m_szOpenHash[]  = "##";
static const char m_szOpenPyDoc[] = "\"\"\"";
static const char m_szOpenRust[]  = "///";

/*!
  @defgroup FlyStrHdr   Functions useful to parse block comments

  Used by flydoc, a source code documentation system, this allows source code headers from many
  languages to be parsed easily.

  * Get a "clean" copy of the block comment regardless of the style used to make it
  * Or Parse the comment in place using functions to get "clean" lines
  * Distinguish "doc block comments" from normal block comments
  * Works with most source code comment types (C, C++, Rust, Python, Javascript, etc...)
*/

/*
  Internal function to aid FlyStrHdrFind().

  Returns TRUE if this is a block header, and length of the opening string.
*/
static bool_t HdrIsOpen(const char *szLine, unsigned *pLenOpen, const char **ppszOpen)
{
  const char *aszOpenings[] = { m_szOpenC, m_szOpenHash, m_szOpenPyDoc, m_szOpenRust };
  bool_t      fIsHdr        = FALSE;
  const char *szHdr;
  unsigned    len;
  unsigned    i;

  szHdr = FlyStrSkipWhite(szLine);
  for(i = 0; i < NumElements(aszOpenings); ++i)
  {
    len = strlen(aszOpenings[i]);
    if(strncmp(szHdr, aszOpenings[i], len) == 0)
    {
      if(pLenOpen)
        *pLenOpen = len;
      if(ppszOpen)
        *ppszOpen = aszOpenings[i];
      fIsHdr = TRUE;
      break;
    }
  }

  // make sure the block comment doesn't close on same line (that is NOT a block comment)
  // hash comments must have single hash after opening `##` line
  if(fIsHdr)
  {
    if(*ppszOpen == m_szOpenC && FlyStrLineStr(&szHdr[len],m_szCloseC))
      fIsHdr = FALSE;
    else if(*ppszOpen == m_szOpenPyDoc && FlyStrLineStr(&szHdr[len], m_szOpenPyDoc))
      fIsHdr = FALSE;
    else if(*ppszOpen == m_szOpenHash && FlyStrChrCount(FlyStrSkipWhite(FlyStrLineNext(szHdr)), '#') != 1)
      fIsHdr = FALSE;
  }

  return fIsHdr;
}

/*!------------------------------------------------------------------------------------------------
  Return TRUE if this is a doc header. Must be on a header found by FlyStrHdrFind().

  @param  szHdr     ptr to header found by FlyStrHdrFind()
  @return TRUE if doc header
*///-----------------------------------------------------------------------------------------------
static bool_t HdrIsDoc(const char *szHdr)
{
  bool_t        fIsDocHdr   = FALSE;
  unsigned      lenOpen     = 0;
  const char    *pszOpen;

  szHdr = FlyStrSkipWhite(szHdr);
  if(HdrIsOpen(szHdr, &lenOpen, &pszOpen))
  {
    if(pszOpen == m_szOpenRust || szHdr[lenOpen] == '!')
      fIsDocHdr = TRUE;
  }

  return fIsDocHdr;
}

/*!------------------------------------------------------------------------------------------------
  Internal function to aid FlyStrHdrFind(). Fill out the pHdr structure.

  @param  pHdr          structure to fill out
  @param  szLine        line for opening header
  @paraam szOpen        returned from HdrIsOpen()
  @return none
*///-----------------------------------------------------------------------------------------------
static void HdrFillOut(flyStrHdr_t *pHdr, const char *szLine, const char *szOpen)
{
  const char *psz;
  bool_t      fAllStars;

  pHdr->szRawHdrLine = szLine;
  pHdr->fIsDoc = HdrIsDoc(szLine);

  // C style block comment
  // /*
  //  * starred contents line 1 (indent 3)
  //  * starred contents line 2
  //  */
  //
  // -or-
  // /*
  //   normal contents (indent 2)
  // */
  //
  if(szOpen == m_szOpenC)
  {
    pHdr->type = FLYSTRHDR_TYPE_C;

    // find pHDr->szStartLine and pHDr->szEndLine
    szLine = FlyStrLineNext(szLine);
    pHdr->szStartLine = szLine;
    while(*szLine)
    {
      if(FlyStrLineStr(szLine, m_szCloseC))
      {
        pHdr->szEndLine = szLine;
        pHdr->szRawHdrEnd = FlyStrLineNext(szLine);
        break;
      }
      szLine = FlyStrLineNext(szLine);
    }
    if(pHdr->szEndLine == NULL)
      pHdr->szEndLine = pHdr->szRawHdrEnd = szLine;

    // determine indent
    // indent may need to skip `*` stars at left
    fAllStars = TRUE;
    szLine = pHdr->szStartLine;
    while(*szLine && szLine < pHdr->szEndLine)
    {
      psz = FlyStrSkipWhite(szLine);
      if(*psz != '*')
      {
        fAllStars = FALSE;
        break;
      }
      if(pHdr->indent == 0 && !FlyStrLineIsBlank(psz + 1))
        pHdr->indent = (FlyStrSkipWhite(psz + 1) - szLine);
      szLine = FlyStrLineNext(szLine);
    }

    // determine indent for normal (not starred) comments
    if(!fAllStars)
    {
      szLine = pHdr->szStartLine;
      while(*szLine && szLine < pHdr->szEndLine)
      {
        if(!FlyStrLineIsBlank(szLine))
        {
          pHdr->indent = (FlyStrSkipWhite(szLine) - szLine);
          break;
        }
        szLine = FlyStrLineNext(szLine);
      }
    }
  }

  // Python Doc """! style block comment
  else if(szOpen == m_szOpenPyDoc)
  {
    pHdr->type = FLYSTRHDR_TYPE_PYDOC;

    // find pHDr->szStartLine and pHDr->szEndLine
    szLine = FlyStrLineNext(szLine);
    pHdr->szStartLine = szLine;
    while(*szLine)
    {
      if(strncmp(FlyStrSkipWhite(szLine), m_szOpenPyDoc, strlen(m_szOpenPyDoc)) == 0)
      {
        pHdr->szEndLine = szLine;
        pHdr->szRawHdrEnd  = FlyStrLineNext(szLine);
        break;
      }
      szLine = FlyStrLineNext(szLine);
    }
    if(pHdr->szEndLine == NULL)
      pHdr->szEndLine = pHdr->szRawHdrEnd = szLine;

    // determine indent
    szLine = pHdr->szStartLine;
    while(*szLine && szLine < pHdr->szEndLine)
    {
      if(!FlyStrLineIsBlank(szLine))
      {
        pHdr->indent = (FlyStrSkipWhite(szLine) - szLine);
        break;
      }
      szLine = FlyStrLineNext(szLine);
    }
  }

  // Hash stuyle block comment, starts with `##` on 1st line
  // These comments are both found in Python and in shell scripts
  else if(szOpen == m_szOpenHash)
  {
    pHdr->type = FLYSTRHDR_TYPE_HASH;

    // find pHDr->szStartLine and pHDr->szEndLine
    szLine = FlyStrLineNext(szLine);
    pHdr->szStartLine = szLine;
    while(*szLine && FlyStrChrCount(FlyStrSkipWhite(szLine), '#') == 1)
      szLine = FlyStrLineNext(szLine);
    pHdr->szEndLine = pHdr->szRawHdrEnd = szLine;

    // determine indent
    szLine = pHdr->szStartLine;
    while(*szLine && szLine < pHdr->szEndLine)
    {
      psz = FlyStrLineChr(szLine, '#');
      if(psz && !FlyStrLineIsBlank(psz + 1))
      {
        pHdr->indent = (FlyStrSkipWhite(psz + 1) - szLine);
        break;
      }
      szLine = FlyStrLineNext(szLine);
    }
  }

  // Rust style block comments are completely line oriented
  else
  {
    FlyAssert(szOpen == m_szOpenRust);
    pHdr->type = FLYSTRHDR_TYPE_RUST;

    pHdr->szStartLine = szLine;
    while(TRUE)
    {
      psz = FlyStrSkipWhite(szLine);
      if(strncmp(psz, m_szOpenRust, strlen(m_szOpenRust)) != 0)
        break;

      if(pHdr->indent == 0)
      {
        psz = FlyStrSkipWhite(&psz[strlen(m_szOpenRust)]);
        if(!FlyStrLineIsBlank(psz))
          pHdr->indent = (psz - szLine);
      }
      szLine = FlyStrLineNext(szLine);
    }
    pHdr->szEndLine = pHdr->szRawHdrEnd = szLine;

    // no lines with text, so just use after the ///
    if(pHdr->indent == 0)
      pHdr->indent = strlen(m_szOpenRust);
  }
}

/*!------------------------------------------------------------------------------------------------
  Return the raw header line, that is, the start of the block comment, not the content.

  @param  pHdr      ptr to valid header as returned by FlyStrHdrFind()
  @return raw header line
*///-----------------------------------------------------------------------------------------------
const char * FlyStrRawHdrLine(const flyStrHdr_t *pHdr)
{
  return pHdr->szRawHdrLine;
}

/*!------------------------------------------------------------------------------------------------
  Return the line AFTER the header closing

  @param  pHdr      ptr to valid header as returned by FlyStrHdrFind()
  @return line passed end of header
*///-----------------------------------------------------------------------------------------------
const char * FlyStrRawHdrEnd(const flyStrHdr_t *pHdr)
{
  return pHdr->szRawHdrEnd;
}

/*!------------------------------------------------------------------------------------------------
  Return line past end of content

  @param  pHdr      ptr to valid header as returned by FlyStrHdrFind()
  @return line past end of content
*///-----------------------------------------------------------------------------------------------
const char * FlyStrHdrContentEnd(const flyStrHdr_t *pHdr)
{
  return pHdr->szEndLine;
}

/*!------------------------------------------------------------------------------------------------
  Return start line of content

  @param  pHdr      ptr to valid header as returned by FlyStrHdrFind()
  @return start line of content
*///-----------------------------------------------------------------------------------------------
const char * FlyStrHdrContentStart(const flyStrHdr_t *pHdr)
{
  return pHdr->szStartLine;
}

/*!------------------------------------------------------------------------------------------------
  Return indent to 1st line (assume that's the indent for all)

  @param  pHdr      ptr to valid header as returned by FlyStrHdrFind()
  @return indent to 1st line in bytes
*///-----------------------------------------------------------------------------------------------
size_t FlyStrHdrIndent(const flyStrHdr_t *pHdr)
{
  return pHdr->indent;
}

/*!------------------------------------------------------------------------------------------------
  Return type of header comment

  @param  pHdr      ptr to valid header as returned by FlyStrHdrFind()
  @return header type, e.g. FLYSTRHDR_TYPE_PYDOC or FLYSTRHDR_TYPE_C
*///-----------------------------------------------------------------------------------------------
flyStrHdrType_t FlyStrHdrType(const flyStrHdr_t *pHdr)
{
  return pHdr->type;
}


/*!------------------------------------------------------------------------------------------------
  Get text for this line in the header

  If anything is invalid, the return value points to the end of szLine.

  @param  pHdr      ptr to valid header as returned by FlyStrHdrFind()
  @param  szLine    a line in the header
  @return text for line
*///-----------------------------------------------------------------------------------------------
const char * FlyStrHdrText(const flyStrHdr_t *pHdr, const char *szLine)
{
  const char *szText;

  if(pHdr && pHdr->type && (szLine >= pHdr->szStartLine) && (szLine < pHdr->szEndLine) && (pHdr->indent <= FlyStrLineLen(szLine)))
    szText = &szLine[pHdr->indent];
  else
    szText = FlyStrLineEnd(szLine);

  return szText;
}

/*!------------------------------------------------------------------------------------------------
  Return the line AFTER the header ends, given a valid hdr as returned by FlyStrHdrFind().

  @param  szHdr     ptr to header found by FlyStrHdrFind()
  @return TRUE if doc header
*///-----------------------------------------------------------------------------------------------
bool_t FlyStrHdrIsDoc(const flyStrHdr_t *pHdr)
{
  return pHdr->fIsDoc;
}

/*!------------------------------------------------------------------------------------------------
  Copy the contents of the header found by FlyStrHdrFind().

  Note: call this with a NULL szDst and SIZE_MAX size to just get the length if allocating a
  buffer. Then call again with the allocated szDst buffer.

  Guaranteed to be '\0' terminated, but contents may be trucated if size is too small.

  @param  szDst     ptr to buffer or NULL to just get length
  @param  szHdr     ptr to header found by FlyStrHdrFind()
  @param  size      size of szDst buffer
  @return length in bytes of header
*///-----------------------------------------------------------------------------------------------
size_t FlyStrHdrCpy(char *szDst, const flyStrHdr_t *pHdr, size_t size)
{
  const char *szLine;
  const char *szText;
  size_t      len       = 0;
  size_t      thisLen;

  if(pHdr->type && pHdr->szStartLine && (pHdr->szEndLine >= pHdr->szStartLine))
  {
    if(szDst && size)
      *szDst = '\0';
    szLine = pHdr->szStartLine;
    while(*szLine && (szLine < pHdr->szEndLine) && (len + 1 < size))
    {
      // copy the line of text
      // note: this will preserve any spaces and blank lines
      szText = FlyStrHdrText(pHdr, szLine);
      thisLen = FlyStrLineLen(szText);
// printf("thisLen %zu, szText '%.*s'\n", thisLen, (int)FlyStrLineLen(szText), szText);
      if(thisLen)
      {
        // truncate if not enough room left in szDst according to size
        // note: size is len + 2 or greater at this point
        if(len + thisLen + 1 >= size)
          thisLen = size - (len + 1);
        if(thisLen)
        {
          FlyStrZNCat(szDst, szText, size, thisLen);
          len += thisLen;
        }
      }
      if(len + 1 < size)
      {
        FlyStrZCat(szDst, "\n", size);
        ++len;
      }

      szLine = FlyStrLineNext(szLine);
    }
  }

  return len;
}

/*!------------------------------------------------------------------------------------------------
  Get position of pszInDst, but in terms of the original file parsed in pHdr.

  That is, if you've used FlyStrHdrCpy(), which makes a clean copy of the block comment/header
  contents, then this will help you find the actual position in the file copied from.

  @param  szDst     copy of header contents from FlyStrHdrCpy()
  @param  szHdr     ptr to header found by FlyStrHdrFind()
  @param  pszInDst  some pointer within the clean header contents contain in szDst
  @return position in original file
*///-----------------------------------------------------------------------------------------------
const char * FlyStrHdrCpyPos(const char *szDst, const flyStrHdr_t *pHdr, const char *pszInDst)
{
  unsigned  line;
  unsigned  col;
  char      *szPos;

  // find line/col within the clean copy of block comment/header
  line = FlyStrLinePos(szDst, pszInDst, &col);
  // printf("pszInDst %p in szDst %p, line %u: col %u\n", pszInDst, szDst, line, col);
  if(pHdr->type != FLYSTRHDR_TYPE_RUST)
    ++line;

  // convert relative to pHdr->szHdrLine
  szPos = FlyStrLineGoto(pHdr->szRawHdrLine, line);
  if(pHdr->indent + (col - 1) > FlyStrLineLen(szPos))
    szPos = FlyStrLineEnd(szPos);
  else
    szPos += pHdr->indent + (col - 1);

  return szPos;
}

/*!------------------------------------------------------------------------------------------------
  Find pointer to start of next block comment (file or function header). Returns NULL if not found.

  Works with most languages such as C, C++, Javascript, Python, Rust, Swift, bash, etc...

  Doc headers are meant for flydoc or Doxygen. They generally include `!` after the  normal block
  comment opening. The exception is `///` which is used by RustDoc, but could also be used in
  C/C++ programs for document headers.

  Python Doc Strings `"""` are ignored unless followed by `!`, that is `"""!`. This is because
  Python Doc strings can be used for long strings, not just class or function headers.

  For hash `#` line comments to be headers, use two or more hashes `##` for the opening line. Any
  lines that start with `#` are considered part of the same header.

      ##
      # 1st line of header
      # 2nd line of header

  If a header is found (return of non-NULL), The pHdr structure contains all the information needed
  to process the header contents.

  ```c
      typedef enum
      {
        FLYSTRHDR_TYPE_NONE = 0,
        FLYSTRHDR_TYPE_C,         // /-*
        FLYSTRHDR_TYPE_PYDOC,     // """ or """!
        FLYSTRHDR_TYPE_HASH,      // # line comment
        FLYSTRHDR_TYPE_RUST,      /// rust type comment
      } flyStrHdrType_t;

      typedef struct
      {
        const char     *szStartLine; // start of contents
        const char     *szEndLine;   // line AFTER end of contents
        const char     *szEndHdr;     // line AFTER end of block comment/header
        size_t          indent;      // indent on each line of contents
        flyStrHdrType_t type;
        bool_t          fIsDoc;
      } flyStrHdr_t;
  ```

  @param  szLine    Ptr to start of line that might contain the opening of a block comment
  @param  fIsDoc    Only find document headers (they have a ! after comment opening)
  @param  pHdr      return value, handle to use in other functions
  @return ptr to header line or NULL if not found
*///-----------------------------------------------------------------------------------------------
const char * FlyStrHdrFind(const char *szLine, bool_t fIsDoc, flyStrHdr_t *pHdr)
{
  const char   *pszHdr = NULL;
  const char   *psz;
  const char   *pszOpen;
  unsigned      openLen;
  bool_t        fIsHdr;

  memset(pHdr, 0, sizeof(*pHdr));

  // look for block headers at start of lines
  psz = szLine;
  while(*psz && (pszHdr == NULL))
  {
    psz   = FlyStrSkipWhite(szLine);
    pszOpen = NULL;
    if(HdrIsOpen(psz, &openLen, &pszOpen))
    {
      fIsHdr = TRUE;

      // looking only for doc headers
      if(fIsDoc)
      {
        if(!HdrIsDoc(psz))
          fIsHdr = FALSE;
      }

      // only allow """!, as Python DocStrings aren't always headers
      else if(pszOpen == m_szOpenPyDoc && psz[openLen] != '!')
        fIsHdr = FALSE;

      // get header type
      if(fIsHdr)
      {
        HdrFillOut(pHdr, szLine, pszOpen);
        pszHdr = szLine;
        break;
      }
    }
    szLine = FlyStrLineNext(szLine);
  }

  return pszHdr;
}
