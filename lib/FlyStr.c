/**************************************************************************************************
  FlyStr.c - String utilities including smart strings, path handling
  Copyright 2024 Drew Gislason
  License: MIT <https://mit-license.org>
*///***********************************************************************************************
#include "FlyStr.h"
#include "FlyMem.h"

/*!
  @defgroup FlyStr A set of C string utilities to augment string.h and strings.h

  Let's face it: C is a poor language for string manipulation. This string API attempts to make it
  a little better.

  Features:

  * Line parsing: FlyStrLineNext(), FlyStrLinePrev(), FlyStrLineLen()
  * Path handling: FlyStrPathAppend(), FlyStrPathParent(), FlyStrPathExt()
  * Dump text/binary: FlyStrDump(), FlyStrDumpEx()
  * Case conversion: lowercase, UPPERCASE, camelCase, MixedCase, snake_case, CONSTANT_CASE
  * Copy/compare arrays of strings: FlyStrArrayCmp()
  * Ensure strings are always NULL terminated with: FlyStrZNCat(), FlyStrZFill()
  * Prevent memory overrun with: FlyStrSmartNew(), FlyStrSmartCat(), FlyStrSmartCpy()
  * And many other string utility functions

  FlyStrZ...() functions have a few properties:

  1. They always result in an ASCIIZ ('\0' termianted) string
  2. They always fit in the # of bytes specified as size
  3. They can have a NULL szDst, which means just calculate size

  One use of FlyStrZ..() is to determine size of strings so you can allocate them. Set the first
  parameter (szDst) to NULL.

  @example Use FlyStrZCat() to create `["Hello","World","!"]`

      #include "FlyStr.h"

      size_t MyAllocArray(char *szDst, const char *asz[], size_t size)
      {
        const char  **ppsz    = asz;
        size_t        lenDst  = 0;
        bool_t        fFirstTime = TRUE;

        lenDst += FlyStrZCpy(szDst, "[", size);
        while(*ppsz)
        {
          lenDst += FlyStrZCat(szDst, fFirstTime ? "\"" : ",\"", size);
          lenDst += FlyStrZCat(szDst, *ppsz, size);
          lenDst += FlyStrZCat(szDst, "\"", size);
          fFirstTime = FALSE;
          ++ppsz;
        }
        lenDst += FlyStrZCat(szDst, size, "]");

        return lenDst;
      }

      int main(void)
      {
        const char *ArrayOfStrings[] = { "Hello", "World", "!", NULL };
        char    *szDst;
        size_t  len;

        len = MyAllocArray(NULL, asz, SIZE_MAX);
        szDst = malloc(len + 1);
        if(szDst)
        {
          len = MyAllocArray(szDst, len + 1, ArrayOfStrings);
          printf("%s\n", szDst);
        }
      }
*/

/*!------------------------------------------------------------------------------------------------
  Returns "(NULL)" if string is NULL or sz if not. Useful in printfs(). Example:

      printf("bang = %s\n", FlyStrNullOk(strchr("Hello! World", '!')));

  @param   sz     ptr to a string, or NULL
  @return  sz or "(NULL)"
*///-----------------------------------------------------------------------------------------------
const char * FlyStrNullOk(const char *sz)
{
  return (sz == NULL) ? "(NULL)" : sz;
}

/*!------------------------------------------------------------------------------------------------
  Returns "" if string is NULL or blank, or string if sz if not. Useful in printfs().

  Example:

      printf("empty = %s\n", FlyStrBlankOf(strchr("Hello World", '!')));

  @param   sz     ptr to a string, or NULL
  @return  sz or ""
*///-----------------------------------------------------------------------------------------------
char * FlyStrBlankOf(const char *sz)
{
  return (char *)((sz == NULL) ? "" : sz);
}

/*!------------------------------------------------------------------------------------------------
  Similar to strchr(), but only searches to end of line.

  @param    sz    string to search
  @param    c     character to look for
  @return   ptr to c, or NULL if not found in line
*///-----------------------------------------------------------------------------------------------
char *FlyStrLineChr(const char *sz, char c)
{
  const char *szFound = NULL;

  while(*sz && *sz != '\r' && *sz != '\n')
  {
    if(*sz == c)
    {
      szFound = sz;
      break;
    }
    ++sz;
  }

  return (char *)szFound;
}

/*!------------------------------------------------------------------------------------------------
  Similar to strstr(), but only searches to end of line.

  @param    szHaystack    string to search
  @param    szNeedle      string to look for
  @return   ptr to the found string or NULL
*///-----------------------------------------------------------------------------------------------
char * FlyStrLineStr(const char *szHaystack, const char *szNeedle)
{
  const char *sz      = szHaystack;
  const char *szFound = NULL;
  size_t      len     = strlen(szNeedle);

  if(len)
  {
    while(*sz && *sz != '\r' && *sz != '\n')
    {
      if(*sz == *szNeedle && (strncmp(sz, szNeedle, len) == 0))
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
  Return ptr to the beginning of the next line in the string.

  Lines must be terminated by \r\n or \n.

  @param  szLine     pointer to a '\0' terminated string, possibly with lines in it
  @return pointer to next line, which may be on the terminating '\0'
*///-----------------------------------------------------------------------------------------------
char * FlyStrLineNext(const char *szLine)
{
  const char *psz;

  psz = strchr(szLine, '\n');
  if(psz)
    ++psz;
  else
    psz = szLine + strlen(szLine);

  return (char *)psz;
}

/*!------------------------------------------------------------------------------------------------
  Return ptr to the beginning of the previous line in the file.

  Lines must be terminated by \r\n or \n.

  sz can be anywhere on the line. If sz is not in the szFile string, then results are
  unpredictable.

  @param    szFile    start of the '\0' terminated file
  @param    sz        ptr to anywhere in the file
  @return  pointer to previous line
*///-----------------------------------------------------------------------------------------------
char * FlyStrLinePrev(const char *szFile, const char *sz)
{
  if(sz < szFile)
    sz = szFile;

  sz = FlyStrLineBeg(szFile, sz);
  if(sz > szFile)
  {
    --sz;
    sz = FlyStrLineBeg(szFile, sz);
  }

  return (char *)sz;
}

/*!------------------------------------------------------------------------------------------------
  Return the line/position of the pointer psz within szFile. Lines/cols are 1 based.

  @param  szFile    file of lines, '\0' terminated
  @param  sz        ptr into szFile
  @param  pCol      column (1-n)
  @return line (1-n), or 0 if sz not in szFile
*///-----------------------------------------------------------------------------------------------
unsigned FlyStrLinePos(const char *szFile, const char *sz, unsigned *pCol)
{
  const char   *szLineNext;
  const char   *szLine    = szFile;
  unsigned      lineFound = 0;
  unsigned      line      = 1;
  unsigned      col       = 1;

  if(sz == szFile)
    lineFound = 1;
  else
  {
    while(*szLine)
    {
      szLineNext = FlyStrLineNext(szLine);
      if(sz >= szLine && sz < szLineNext)
      {
        lineFound = line;
        col += (unsigned)(sz - szLine);
        break;
      }
      ++line;
      szLine = szLineNext;
    }
  }

  // return line, col
  if(pCol)
    *pCol = col;
  return lineFound;
}

/*!------------------------------------------------------------------------------------------------
  Return ptr to the beginning of line, that is to the '\n'. If no \n, then points to szHaystack.

  @param  szFile    file of lines
  @param  sz        ptr into string
  @return ptr to beginning of line or szHaystack
*///-----------------------------------------------------------------------------------------------
char * FlyStrLineBeg(const char *szFile, const char *sz)
{
  const char *psz = sz;

  if(psz < szFile)
    psz = szFile;

  else
  {
    while(psz > szFile)
    {
      --psz;
      if(*psz == '\n')
      {
        ++psz;
        break;
      }
    }
  }

  return (char *)psz;
}

/*!------------------------------------------------------------------------------------------------
  Return ptr to the end of line or string, that is '\r', '\n' or '\0'.

  @param  sz  ptr to string.
  @return ptr to end of line or string
*///-----------------------------------------------------------------------------------------------
char * FlyStrLineEnd(const char *sz)
{
  const char *pszEnd;

  pszEnd = sz;
  while(*pszEnd && *pszEnd != '\r' && *pszEnd != '\n')
    ++pszEnd;

  return (char *)pszEnd;
}

/*!------------------------------------------------------------------------------------------------
  Return the line ending, that is "\r\n" under Windows, "\n" under Linux or macOS.
  @return line ending
*///-----------------------------------------------------------------------------------------------
const char *FlyStrLineEnding(void)
{
  static const char szLineEnding[] =
  #if FLY_PLATFORM == FLY_WINDOWS
    "\r\n";
  #else
    "\n";
  #endif
  return szLineEnding;
}

/*!------------------------------------------------------------------------------------------------
  Return end of string

  @param  sz  ptr to string
  @return ptr to end of file (string)
*///-----------------------------------------------------------------------------------------------
char * FlyStrLineEof(const char *sz)
{
  return (char *)(sz + strlen(sz));
}


/*!------------------------------------------------------------------------------------------------
  Return # if lines from start of file to ptr sz.

  @param  szFile    ptr to file, '\0' terminated
  @param  sz        ptr to string in file (if NULL then count lines in file)
  @return count of lines
*///-----------------------------------------------------------------------------------------------
size_t FlyStrLineCount(const char *szFile, const char *sz)
{
  size_t  count = 0;
  const char  *szLineNext;
  const char  *szLine;

  // count lines in file
  if(sz == NULL || sz < szFile)
    sz = szFile + strlen(szFile);

  // makes no sense for sz to be less than start of file
  szLine = szLineNext = szFile;
  while(*szLine)
  {
    szLineNext = FlyStrLineNext(szLine);
    if(sz < szLineNext)
      break;
    szLine = szLineNext;
    ++count;
  }

  return count;
}

/*!------------------------------------------------------------------------------------------------
  Go to the given line, 1 based, just like in an editor.

  If the line exceeds the # of lines in the file, the returns the last line

  @param  szFile    ptr to file, '\0' terminated
  @param  line      1-n (line to goto)
  @return ptr to beginning of line
*///-----------------------------------------------------------------------------------------------
char * FlyStrLineGoto(const char *szFile, size_t line)
{
  const char *szLine = szFile;
  const char *szEnd;

  while(line > 1)
  {
    szEnd = FlyStrLineEnd(szLine);

    // at end of file
    if(*szEnd == '\0')
      break;

    szLine = FlyStrLineNext(szEnd);
    --line;
  }

  return (char *)szLine;
}

/*!------------------------------------------------------------------------------------------------
  Skip any blank lines at the start of the string. This is line oriented only.

  Example: "\n\n    \n  Hello\n" => "  Hello\n"

  @param  sz  ptr to asciiz string
  @return ptr to beginning of a non-blank line or end of string
*///-----------------------------------------------------------------------------------------------
char * FlyStrLineSkipBlank(const char *sz)
{
  while(*sz && FlyStrLineIsBlank(sz))
    sz = FlyStrLineNext(sz);
  return (char *)sz;
}

/*!------------------------------------------------------------------------------------------------
  Return indent of this line. Treats tabs as 1 space.

  @param  sz        ptr to string.
  @param  tabSize   usually 2, 4 or 8
  @return 0-n
*///-----------------------------------------------------------------------------------------------
size_t FlyStrLineIndent(const char *sz, unsigned tabSize)
{
  size_t  indent = 0;

  while(*sz == ' ' || *sz == '\t')
  {
    indent += (*sz == '\t') ? tabSize : 1;
    ++sz;
  }

  return indent;
}

/*!------------------------------------------------------------------------------------------------
  Return indent of this line. Treats tabs as 1 space.

  @param  sz        ptr to string.
  @param  tabSize   usually 2,4 or 8
  @return 0-n
*///-----------------------------------------------------------------------------------------------
size_t FlyStrLineIndentTab(const char *sz, unsigned tabSize)
{
  size_t  indent = 0;

  while(*sz == ' ' || *sz == '\t')
  {
    indent += *sz == '\t' ? tabSize : 1;
    ++sz;
  }

  return indent;
}

/*!------------------------------------------------------------------------------------------------
  Find length of a line

  @param  sz  ptr to string.
  @return length of line, may be 0 if line is empty
*///-----------------------------------------------------------------------------------------------
size_t FlyStrLineLen(const char *sz)
{
  return (size_t)(FlyStrLineEnd(sz) - sz);
}

/*!------------------------------------------------------------------------------------------------
  Same as FlyStrLineLen(), but also contains the `\n` or `\r\n`.

  @param  sz  ptr to string
  @return ptr to end of line or string including the `\n` or `\r\n`
*///-----------------------------------------------------------------------------------------------
size_t FlyStrLineLenEx(const char *sz)
{
  return (size_t)(FlyStrLineNext(sz) - sz);
}

/*!------------------------------------------------------------------------------------------------
  Is this line blank? (nothing but whitespace)?

  @param  sz  ptr to string
  @return TRUE if line is blank, FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlyStrLineIsBlank(const char *sz)
{
  sz = FlyStrSkipWhite(sz);
  return (FlyStrLineEnd(sz) == sz) ? TRUE : FALSE;
}

/*!-------------------------------------------------------------------------------------------------
  Removes blank lines from start/end of a multi-line string

  @param    szLines     ptr to lines of text
  @return   none
*///------------------------------------------------------------------------------------------------
void FlyStrLineBlankRemove(char *szLines)
{
  char *szLastBlank;
  char *psz;

  // remove blank lines from start
  psz = szLines;
  while(*psz && FlyStrLineIsBlank(psz))
    psz = (char *)FlyStrLineNext(psz);
  if(psz > szLines)
    memmove(szLines, psz, strlen(psz) + 1);

  // remove blank lines from end
  if(*psz)
  {
    szLastBlank = NULL;  // will be the "last blank" line, if any
    while(*psz)
    {
      if(FlyStrLineIsBlank(psz))
      {
        if(szLastBlank == NULL)
          szLastBlank = psz;
      }
      else
      {
        szLastBlank = NULL;
      }

      psz = (char *)FlyStrLineNext(psz);
    }
    if(szLastBlank)
      *szLastBlank = '\0';
  }
}

/*!------------------------------------------------------------------------------------------------
  Removes blank characters from start/end of string

  @param  sz  ptr to string
  @return none
*///-----------------------------------------------------------------------------------------------
void FlyStrBlankRemove(char *sz)
{
  char    *psz;
  char    *pszLastBlank;

  // remove blank chars from start of string
  psz = sz;
  while(*psz && (*psz == ' ' || *psz == '\t'))
    ++psz;
  if(psz > sz)
    memmove(sz, psz, strlen(psz) + 1);

  // remove blank chars from end of string
  if(*psz)
  {
    pszLastBlank = NULL;
    while(*psz)
    {
      if(*psz == ' ' || *psz == '\t')
      {
        if(pszLastBlank == NULL)
          pszLastBlank = psz;
      }
      else
      {
        pszLastBlank = NULL;
      }
      ++psz;
    }
    if(pszLastBlank)
      *pszLastBlank = '\0';
  }
}

/*!------------------------------------------------------------------------------------------------
  Copies argument from szSrc to szDst. Skips preceding and trailing whitespace on szSrc. Allows
  szDst to be NULL (no copy) to just skip argument.

  Example:  "  1starg  2ndarg", would copy "1starg" and return ptr to "2ndarg"

  @param    szDst     destination for argument (may be NULL to just skip arg)
  @param    szSrc     source string
  @param    size      buffer sizeof szDst, must be at least 1 for NUL
  @return  pointer to next arg in szSrc
*///-----------------------------------------------------------------------------------------------
const char * FlyStrArgCpy(char *szDst, const char *szSrc, size_t size)
{
  if(size)
  {
    --size;

    // skip preceding whitespace
    szSrc = FlyStrSkipWhite(szSrc);

    // copy as much of arg as we can (up to size-1)
    while(*szSrc && !isspace(*szSrc))
    {
      if(size)
      {
        if(szDst)
          *szDst++ = *szSrc;
        --size;
      }
      ++szSrc;
    }

    // skip post whitespace
    szSrc = FlyStrSkipWhite(szSrc);
  }

  if(szDst)
    *szDst = '\0';

  return szSrc;
}

/*!------------------------------------------------------------------------------------------------
  Just like strcmp(), but for args.

  @param    sz1     a string or arg
  @param    sz2     a string or arg
  @return   0 if the same, 1 if s1 > s2, -1 if s1 < s2
*///-----------------------------------------------------------------------------------------------
int FlyStrArgCmp(const char *sz1, const char *sz2)
{
  size_t  len1;
  size_t  len2;
  size_t  lenShort;
  int     ret;

  len1 = FlyStrArgLen(sz1);
  len2 = FlyStrArgLen(sz2);
  if(len1 <= len2)
    lenShort = len1;
  else
    lenShort = len2;

  ret = strncmp(sz1, sz2, lenShort);
  if(ret == 0 && len1 != len2)
    ret = (sz1[lenShort] > sz2[lenShort]) ? 1 : -1;

  return ret;
}

/*!------------------------------------------------------------------------------------------------
  Determines length of argument in bytes. Works with both ASCII and UTF-8.

  Stops on end of string or end of line.

  Exmple:

      1starg  second_arg

  Also handles quotes so spaces can be part of an argument.

      "This is arg1" "This is arg2"

  Also handles escapes:

      "This is \"quoted\" arg1" "this is arg2"

  @param    sz        start of string that contains 0 or more arguments
  @return   0-n, length of argument in bytes
*///-----------------------------------------------------------------------------------------------
size_t FlyStrArgLen(const char *sz)
{
  size_t  len = 0;
  bool_t  fInString = FALSE;

  while(isblank(*sz) || (uint8_t)*sz > ' ')
  {
    // handle escapes
    if(*sz == '\\' && (sz[1] >= ' ' && sz[1] <= '~'))
    {
      sz += 2;
      len += 2;
    }

    // handle quoted args
    if(*sz == '"')
      fInString = !fInString;

    // space or tab will end the argument, unless in a string
    if(isblank(*sz) && !fInString)
      break;

    ++len;
    ++sz;
  }

  return len;
}

/*!------------------------------------------------------------------------------------------------
  Finds next argument in this string. Does NOT cross lines if string has end of line in it.

  @param    sz        ptr to string
  @return   ptr to next argument or end of string/line
*///-----------------------------------------------------------------------------------------------
char * FlyStrArgNext(const char *sz)
{
  if(isblank(*sz))
    sz = FlyStrSkipWhite(sz);
  else
  {
    sz += FlyStrArgLen(sz);
    sz = FlyStrSkipWhite(sz);  
  }

  return (char *)sz;
}

/*!------------------------------------------------------------------------------------------------
  Skip tabs '\t' and spaces ' ' only.

  See also FlyStrSkipWhiteEx() for skipping newlines, carraige returns, etc...

  @param    sz      asciiz string
  @return   ptr to first non-tab or space or at '\0' (end of string)
*///-----------------------------------------------------------------------------------------------
const char * FlyStrSkipWhite(const char *sz)
{
  while(*sz && isblank(*sz))
    ++sz;
  return sz;
}

/*!------------------------------------------------------------------------------------------------
  Skips space ' ', form-feed '\f', newline '\n', carriage return '\r', horizontal tab '\t', and
  vertical tab '\v'.

  @param    sz      asciiz string
  @return   ptr to non-whitespace, may be at '\0'
*///-----------------------------------------------------------------------------------------------
const char * FlyStrSkipWhiteEx(const char *sz)
{
  while(*sz && isspace(*sz))
    ++sz;
  return sz;
}

/*!------------------------------------------------------------------------------------------------
  Skip all characters found in szChars.

  @param    sz        asciiz string
  @param    szChars   chars to skip
  @return   ptr to 1st char not in szChars
*///-----------------------------------------------------------------------------------------------
const char * FlyStrSkipChars(const char *sz, const char *szChars)
{
  while(*sz)
  {
    if(strchr(szChars, (int)(*sz)) == NULL)
      break;
    ++sz;
  }
  return sz;
}

/*!------------------------------------------------------------------------------------------------
  Skip to end of argument or token. If on whitespace does nothing.

  This         | Finds That
  ------------ | ---------
  First second | First second
  ^            |     ^
  First second | First second
          ^    |            ^

  @param    sz    ptr to asciiz string
  @return   ptr to whitepspace after token or '\0'
*///-----------------------------------------------------------------------------------------------
const char * FlyStrArgEnd(const char *sz)
{
  while(*sz && !isspace(*sz))
    ++sz;
  return sz;
}

/*!------------------------------------------------------------------------------------------------
  Search backward from sz to beginning of a token or argument. If on whitespace, does nothing.

  Examples:

  This         | Finds That
  ------------ | ---------
  First second | First second
    ^          | ^
  First second | First second
             ^ |       ^

  @param    szStart   the string to search
  @param    sz        ptr to search from
  @return   ptr to beginning of token
*///-----------------------------------------------------------------------------------------------
const char * FlyStrArgBeg(const char *szStart, const char *sz)
{
  if(!isspace(*sz))
  {
    while(sz > szStart)
    {
      if(isspace(*sz))
        break;
      --sz;
    }
    if(isspace(*sz))
      ++sz;
  }

  return sz;
}

/*-------------------------------------------------------------------------------------------------
  Helpr to FlyStrSkipNumber(). Skip integer part.

  Examples: `42, -999_333, +3, 0xdead_beef, 0b110, 0o777, 6.626e-34 -2E-2`

  @param    sz      pointer to potential number
  @return   ptr 
*///-----------------------------------------------------------------------------------------------
static const char * SkipInteger(const char *sz)
{
  bool_t  fHexDigits = FALSE;

  // skip integer number, possibly in hex, octal, etc..
  if(*sz == '-' || *sz == '+')
    ++sz;
  if(*sz == '0' && (sz[1] == 'x' || sz[1] == 'b' || sz[1] == 'o'))
  {
    if(sz[1] == 'x')
      fHexDigits = TRUE;
    sz += 2;
  }
  while(*sz == '_' || isdigit(*sz) || (fHexDigits && isxdigit(*sz)))
    ++sz;

  return (char *)sz;
}

/*!------------------------------------------------------------------------------------------------
  Skip the number. Works with integers, floats, hex, octal, binary numbers.

  Examples: `42, -999_333, +3, 0xdead_beef, 0b110, 0o777, 6.626e-34 -2E-2`

  @param    sz    asciiz string
  @return   ptr to after number
*///-----------------------------------------------------------------------------------------------
char * FlyStrSkipNumber(const char *sz)
{
  // skip integer part
  sz = SkipInteger(sz);

  // skip fraction on floats, e.g. 3.14159
  if(*sz == '.')
    sz = SkipInteger(sz + 1);

  // skip exponent on floats, e.g. 6.626e-34
  if(*sz == 'e' || *sz == 'E')
    sz = SkipInteger(sz + +1);

  return (char *)sz;
}

/*!------------------------------------------------------------------------------------------------
  Can skip any string with escapes. Handles ", ' and ` for strings.

  @param    sz    asciiz string
  @return   ptr to after quoted string
*///-----------------------------------------------------------------------------------------------
const char * FlyStrSkipString(const char *sz)
{
  char        endChar;

  // look for matching string opening for closing char
  endChar = *sz;
  ++sz;
  while(*sz && !(*sz == endChar || *sz == '\r' || *sz == '\n'))
  {
    if(*sz == '\\' && sz[1])
      ++sz;
    ++sz;
  }
  if(*sz == endChar)
    ++sz;

  return sz;
}


/*!------------------------------------------------------------------------------------------------
  StrICmp (case insensitive) is not reliably in the C library. Posix C libraries have strcasecmp()

  @param    sz1      asciiz string
  @param    sz2      asciiz string
  @return   -1 if sz1 is less that sz2, 0 if same, 1 if sz1 is greater
*///-----------------------------------------------------------------------------------------------
int FlyStrICmp(const char *sz1, const char *sz2)
{
  size_t    len1;
  size_t    len2;
  size_t    n;
  int       ret;

  // only compare until the '\0'
  len1 = strlen(sz1);
  len2 = strlen(sz2);
  n = len1;
  if(n < len2)
    n = len2;

  ret = FlyMemICmp(sz1, sz2, n);
  if(ret == 0)
  {
    if(len1 > len2)
      ret = 1;
    if(len1 < len2)
      ret = -1;
  }

  return ret;
}

/*!------------------------------------------------------------------------------------------------
  Is this a valid CName character

  @param  c   character to checl
  @return TRUE if is a valid CName character
*///-----------------------------------------------------------------------------------------------
bool_t FlyCharIsCName(char c)
{
  return (c == '_' || isalnum(c)) ? TRUE : FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Is this a valid dozenal character? (0-9,X,E)

  @param  c   character to checl
  @return TRUE if is a valid CName character
*///-----------------------------------------------------------------------------------------------
bool_t FlyCharIsDozenal(char c)
{
  return (isdigit(c) || (c == 'x') || (c == 'X') || (c == 'e') || (c == 'E')) ? TRUE : FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Return previous character. Why can't C do this natively? e.g. if(psz[-1] == ' ')

  Assumes sz is NOT at the 1st char of the string.

  @param  sz   ptr to a string where sz is NOT the 1st char of the string
  @return previous character
*///-----------------------------------------------------------------------------------------------
char FlyCharPrev(const char *sz)
{
  --sz;
  return *sz;
}

/*!------------------------------------------------------------------------------------------------
  Is this an end of line character? (that is \r or \n)

  @param  c   character to checl
  @return TRUE if it is end of line
*///-----------------------------------------------------------------------------------------------
bool_t FlyCharIsEol(char c)
{
  return (c == '\r' || c == '\n');
}

/*!------------------------------------------------------------------------------------------------
  Is this character in a set?

  @param  c   character to checl
  @return TRUE if it is end of line
*///-----------------------------------------------------------------------------------------------
bool_t FlyCharIsInSet(char c, const char *szSet)
{
  return strchr(szSet, c) ? TRUE : FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Handles escaped sequences (e.g. `\n` or `\"`, `\333` or `0xfe`) in a character string.

  Does not handle escaped unicode (`\uhhhh` or `\Uhhhhhhhh`). See FlyStrNTol().

  See: <https://en.wikipedia.org/wiki/Escape_sequences_in_C>

  @param  sz      ptr to string with escapes
  @param  pChar   ptr to byte to receive next byte/char
  @return ptr to next character (1-4 bytes)
*///-----------------------------------------------------------------------------------------------
char * FlyCharEsc(const char *sz, uint8_t *pByte)
{
  static const char   aszEsc[]  = "abefnrtv";
  static uint8_t      aEscVal[] = { 0x07, 0x08, 0x1b, 0x0c, 0x0a, 0x0d, 0x09, 0x0b };
  const char         *p;
  uint8_t             c;
  unsigned            len;

  // two arrays must match
  FlyAssert((sizeof(aszEsc) - 1) == sizeof(aEscVal));

  len = 1;
  if(*sz == '\\')
  {
    c = (uint8_t)(sz[1]);
    len = 2;

    // special: 2-byte escaped char
    p = strchr(aszEsc, c);
    if(p)
      *pByte = aEscVal[(unsigned)(p - aszEsc)];

    // single byte octal
    else if(c >= '0' && c <= '7')
    {
      p = FlyCharOct(&sz[1], pByte);
      len = (unsigned)(p - sz);
    }

    // single byte hex
    else if (c == 'x')
    {
      p = FlyCharHex(&sz[2], pByte);
      len = (unsigned)(p - sz);
    }

    // normal 2-byte scaped char (the char that follows backslash)
    else
    {
      // don't go past the terminating '\0'
      *pByte = c;
      if(c == '\0')
        len = 0;
    }
  }

  // normal character, no transformation
  else
  {
    c = (uint8_t)(*sz);

    // don't go past the terminating '\0'
    *pByte = c;
    if(c == '\0')
      len = 0;
  }

  return (char *)(sz + len);
}

/*!------------------------------------------------------------------------------------------------
  Convert from octal string to a byte value 0x00-0x7f.

  Returns ptr to after 0-3 byte octal string (e.g. pts to 3 in "725301").

  @param  sz      ptr to octal string
  @param  pChar   ptr to byte to receive value
  @return ptr to after 0-3 byte octal string
*///-----------------------------------------------------------------------------------------------
char * FlyCharOct(const char *sz, uint8_t *pByte)
{
  uint8_t   c   = 0;
  unsigned  len = 0;

  while(len < 3 && (*sz >= '0' && *sz <= '7'))
  {
    c = (c << 3) + (*sz - '0');
    ++len;
    ++sz;
  }

  *pByte = c;
  return (char *)sz;
}

/*!------------------------------------------------------------------------------------------------
  Convert from hex string to a byte value 0x00-0xff.

  Returns ptr to after 0-2 byte hex string (e.g. pts to 'a' in "F3a2").

  @param  sz      ptr to octal string
  @param  pChar   ptr to byte to receive value
  @return ptr to after 0-2 byte hex string
*///-----------------------------------------------------------------------------------------------
char * FlyCharHex(const char *sz, uint8_t *pByte)
{
  uint8_t   xdigit  = 0;
  uint8_t   c       = 0;
  unsigned  len     = 0;

  while(len < 2 && isxdigit(*sz))
  {
    if(*sz >= '0' && *sz <='9')
      xdigit = *sz - '0';
    else if(*sz >= 'a' && *sz <='f')
      xdigit = 10 + (*sz - 'a');
    else
      xdigit = 10 + (*sz - 'A');
    c = (c << 4) + xdigit;
    ++len;
    ++sz;
  }

  *pByte = c;
  return (char *)sz;
}

/*!------------------------------------------------------------------------------------------------
  Return lowercase ASCII char for this hex digit nybble.

  @param  nybble    value 0 - 0xf
  @return character '0'-'9' or 'a'-'f'
*///-----------------------------------------------------------------------------------------------
char FlyCharHexDigit(uint8_t nybble)
{
  nybble = nybble & 0xf;
  return (nybble >= 0xa) ? 'a' + (nybble - 0xa) : '0' + nybble;
}

/*!------------------------------------------------------------------------------------------------
  Return count of a specific character at sz.

  @param  c   character (1 - 0xff)
  @return # of characters at sz (0 - n)
*///-----------------------------------------------------------------------------------------------
unsigned FlyStrChrCount(const char *sz, char c)
{
  unsigned count = 0;
  while(*sz && *sz == c)
  {
    ++count;
    ++sz;
  }
  return count;
}

/*!------------------------------------------------------------------------------------------------
  Return # of a particular character at end of string.

  Example, counts dots at end of string:

  ```c
      char sz_etc[] = "etc...";
      n = FlyStrChrCountRev(sz_etc, sz_etc + strlen(sz_etc), '.');  // n == 3
  ```

  @param  szStart   start of string to searech
  @param  szEnd     end of search (
  @param  c   character (0 - 0xff)
  @return # of characters at sz (0 - n)
*///-----------------------------------------------------------------------------------------------
unsigned FlyStrChrCountRev(const char *szStart, const char *szEnd, char c)
{
  unsigned count = 0;
  while(szEnd > szStart && count < UINT_MAX)
  {
    --szEnd;
    if(*szEnd == c)
      ++count;
    else
      break;
  }
  return count;
}

/*!------------------------------------------------------------------------------------------------
  Return TRUE if this character is tab or space

  @param  c      character (0x00-0xff)
  @return TRUE if this character is tab or space, FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlyStrIsSpace(char c)
{
  return (c == ' ' || c == '\t') ? TRUE : FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Return case of string: lowercase, UPPERCASE, camelCase, MixedCase, snake_case, CONSTANT_CASE

  @param  sz    pointer to a string, perhaps a CName string
  @return case of string, e.g. IS_CAMEL_CASE, IS_MIXED_CASE
*///-----------------------------------------------------------------------------------------------
flyStrCase_t FlyStrIsCase(const char *sz)
{
  flyStrCase_t strCase     = IS_LOWER_CASE;
  bool_t        fFirstLower = FALSE;    // lowercase, chamelCase
  bool_t        fHasLower   = FALSE;    // lowercase, camelCase, MixedCase, snake_case
  bool_t        fHasUpper   = FALSE;    // UPPERCASE, camelCase, MixedCase, CONSTANT_CASE
  bool_t        fHasSnake   = FALSE;    // snake_case, CONSTANT_CASE

  if(islower(*sz))
    fFirstLower = TRUE;

  while(*sz)
  {
    if(islower(*sz))
      fHasLower = TRUE;
    else if(isupper(*sz))
      fHasUpper = TRUE;
    else if(*sz == '_')
      fHasSnake = TRUE;
    ++sz;
  }

  if(fHasSnake)
  {
    if(!fHasLower)
      strCase = IS_CONSTANT_CASE;
    else
      strCase = IS_SNAKE_CASE;
  }
  else
  {
    if(fHasLower && fHasUpper)
    {
      if(fFirstLower)
        strCase = IS_CAMEL_CASE;
      else
        strCase = IS_MIXED_CASE;
    }
    else if(fHasUpper)
      strCase = IS_UPPER_CASE;
    else
      strCase = IS_LOWER_CASE;
  }

  return strCase;
}

/*!------------------------------------------------------------------------------------------------
  Convert string to the new case:  lower, UPPER, camelCase, MixedCase, snake_case, CONSTANT_CASE

  Note: Some cases don't convert well. For example, lowercase can't convert to camelCase, but can
  to UPPERCASE. Converting from MixedCase to lowercase is a one way street (can't convert back),
  as the string no longer indicates words within the string.

  @param    szNew       pointer to char array to receive new string
  @param    size        sizeof dst string buffer
  @param    szOld       pointer to asciiz string to convert from
  @param    strCase     new case desired (e.g. IS_CAMEL_CASE)
  @return   length of new string
*///-----------------------------------------------------------------------------------------------
size_t FlyStrToCase(char *szNew, const char *szOld, size_t size, flyStrCase_t strCase)
{
  size_t        len         = 0;
  size_t        maxLen;
  bool_t        fFirstChar  = TRUE;  // first alpha character
  flyStrCase_t  oldCase     = FlyStrIsCase(szOld);

  // nothing to do
  if(oldCase == strCase)
    return strlen(szOld);
  if(size == 0)
    return 0;

  // handle lower and UPPER case as they are easy, so is CONSTANT_CASE to CONSTANT_CASE
  maxLen = size - 1;
  if(strCase == IS_LOWER_CASE || strCase == IS_UPPER_CASE)
  {
    while(*szOld && len < maxLen)
    {
      // remove underscores for lower and UPPER cases
      if(*szOld != '_')
      {
        *szNew = (strCase == IS_LOWER_CASE) ? tolower(*szOld) : toupper(*szOld);
        ++szNew;
        ++len;
      }
      ++szOld;
    }
  }

  // must be camelCase, MixedCase, snake_case, CONSTANT_CASE
  else
  {
    while(*szOld && len < maxLen)
    {
      // first alpha char is special on camelCase
      if(fFirstChar && isalpha(*szOld))
      {
        if(strCase == IS_CAMEL_CASE || strCase == IS_SNAKE_CASE)
          *szNew = tolower(*szOld);
        else
          *szNew = toupper(*szOld);
        fFirstChar = FALSE;
      }

      else
      {
        // on snake_case or CONSTANT_CASE word separator
        if(*szOld == '_')
        {
          if(strCase == IS_SNAKE_CASE || strCase == IS_CONSTANT_CASE)
            *szNew = *szOld;

          // remove _ and make 1st char of Word uppercase for cameCase and MixedCase
          else 
          {
            ++szOld;
            *szNew = toupper(*szOld);
          }
        }

        // on a camelCase or MixedCase word separator
        else if(isupper(*szOld) && (oldCase != IS_CONSTANT_CASE))
        {
          if(strCase == IS_SNAKE_CASE || strCase == IS_CONSTANT_CASE)
          {
            *szNew++ = '_';
            ++len;
            if(len < maxLen)
              *szNew = (strCase == IS_SNAKE_CASE) ? tolower(*szOld) : *szOld;
          }
          else
            *szNew = *szOld;
        }

        // normal (not 1st) character in word, always lower unless CONSTANT_CASE
        else
        {
          if(strCase == IS_CONSTANT_CASE)
            *szNew = toupper(*szOld);
          else
            *szNew = tolower(*szOld);
        }
      }
      ++szNew;
      ++szOld;
      ++len;
    }
  }

  // add terminating NUL
  *szNew = '\0';

  return len;
}

/*!------------------------------------------------------------------------------------------------
  Convert string to lowercase. Doesn't affect underscores like FlyStrToCase().

  @param    sz    asciiz string
  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyStrToLower(char *sz)
{
  while(*sz)
  {
    *sz = tolower(*sz);
    ++sz;
  }
}

/*!------------------------------------------------------------------------------------------------
  Returns the constant strings TRUE or FALSE, based on the boolean flag

  @param    fFlag    TRUE or FALSE
  @return   ptr to string constant "TRUE" or "FALSE"
*///-----------------------------------------------------------------------------------------------
const char * FlyStrTrueFalse(bool_t fFlag)
{
  return (const char *)(fFlag ? "TRUE" : "FALSE");
}

/*!------------------------------------------------------------------------------------------------
  From the time parameter, get date/time in ISO 8601 string form. Example: "2019-12-18T14:58:01".
  Local time only. Knows nothing about time zones. Returns separate static string from
  FlyStrDateTimeCur()

  @param    time      current date/time in time_t format (see <time.h>)
  @return   pointer to static date/time string built from time_t
*///-----------------------------------------------------------------------------------------------
const char * FlyStrDateTime(time_t time)
{
  struct tm  *pInfo;
  static char szDateTime[22];

  pInfo = localtime(&time);
  snprintf(szDateTime, sizeof(szDateTime), "%04i-%02i-%02iT%02i:%02i:%02i",
          1900+pInfo->tm_year, pInfo->tm_mon+1, pInfo->tm_mday, 
          pInfo->tm_hour,pInfo->tm_min,pInfo->tm_sec);
  return (const char *)szDateTime;
}

/*!------------------------------------------------------------------------------------------------
  Get the current date/time in ISO 8601 form "2019-10-23T08:15:30". Local time only. Knows
  nothing about time zones. Returns separate static string from FlyStrDateTime()

  @return   pointer to static date/time string
*///-----------------------------------------------------------------------------------------------
const char * FlyStrDateTimeCur(void)
{
  time_t      t;
  struct tm  *pInfo;
  static char szDateTime[22];

  time(&t);
  pInfo = localtime(&t);
  snprintf(szDateTime, sizeof(szDateTime), "%04i-%02i-%02iT%02i:%02i:%02i",
          1900+pInfo->tm_year, pInfo->tm_mon+1, pInfo->tm_mday, 
          pInfo->tm_hour,pInfo->tm_min,pInfo->tm_sec);
  return (const char *)szDateTime;
}

/*!------------------------------------------------------------------------------------------------
  Returns the user home folder name (from environment $HOME). Basically the '~' in filenames.

  @return   Returns ptr to home string (e.g. /Users/me)
*///-----------------------------------------------------------------------------------------------
const char * FlyStrPathHome(void)
{
  return getenv("HOME");
}

/*!------------------------------------------------------------------------------------------------
  Returns TRUE if string is a folder. e.g. ".", "..", or "/User/me/".

  This is a string search only and does NOT check:

  1. if the path exists on disk
  2. for legal OS path characters
  3. that the string is < PATH_MAX

  @param    szPath    pointer to a file or dir path (relative or absolute).
  @return   Returns TRUE if this is a folder
*///-----------------------------------------------------------------------------------------------
bool_t FlyStrPathIsFolder(const char *szPath)
{
  bool_t  fIsFolder = FALSE;

  if((strcmp(szPath, ".") == 0) || (strcmp(szPath, "..") == 0))
    fIsFolder = TRUE;

  else if(isslash(FlyStrCharLast(szPath)))
    fIsFolder = TRUE;

  return fIsFolder;
}

/*!------------------------------------------------------------------------------------------------
  Returns TRUE if path is relative.

  "../file" and "folder/" are both relative.  
  "/Users/me/folder/" and "~/file" are not.

  @param    szPath    pointer to a file or folder path (relative or absolute).
  @return   Returns TRUE if this path is relative, FALSE if absolute.
*///-----------------------------------------------------------------------------------------------
bool_t FlyStrPathIsRelative(const char *szPath)
{
  bool_t  fIsRelative = TRUE;

  if(isslash(*szPath) || (*szPath == '~' && (szPath[1] == '\0' || isslash(szPath[1]))))
    fIsRelative = FALSE;

  return fIsRelative;
}

/*!------------------------------------------------------------------------------------------------
  Finds the last slash in a path, or returns NULL if there are none.

  Works with both Windows and Linux style paths, .e.g "/file/path/name" or "C:\file\path\name"

  @param    szPath    pointer to a file or dir path (relative or absolute).
  @return   Returns last slash or NULL if not found
*///-----------------------------------------------------------------------------------------------
char * FlyStrLastSlash(const char *szPath)
{
  const char *psz;
  const char *szLastSlash = NULL;

  psz = szPath;
  while(*psz)
  {
    if(isslash(*psz))
      szLastSlash = psz;
    ++psz;
  }

  return (char *)szLastSlash;
}

/*!------------------------------------------------------------------------------------------------
  Finds the nerxt slash in a path, or returns NULL if there are none.

  Works with both Windows and Linux style paths, .e.g "/file/path/name" or "C:\file\path\name"

  @param    szPath    pointer to a file or dir path (relative or absolute).
  @return   Returns next slash or NULL if not found
*///-----------------------------------------------------------------------------------------------
char * FlyStrNextSlash(const char *szPath)
{
  return strpbrk(szPath, "/\\");
}

/*!------------------------------------------------------------------------------------------------
  Like strrchr(), but works with both Linux and Windows path separators, slash and backslash.

  @param    szPath    ptr to a path string, e.g. "/Users/me/work/"
  @param    psz       pointer to any part of szPath or NULL for end of string
  @return   Returns previous slash, or NULL if not found
*///-----------------------------------------------------------------------------------------------
char * FlyStrPrevSlash(const char *szPath, const char *psz)
{
  // search from end
  if(psz == NULL)
    psz = szPath + strlen(szPath);

  while(TRUE)
  {
    if(isslash(*psz))
      break;
    if(psz <= szPath)
    {
      psz = NULL;
      break;
    }
    --psz;;
  }

  return (char *)psz;
}

/*!------------------------------------------------------------------------------------------------
  Get the very last character of this string.

  @param    sz     pointer to string
  @return   Return last character in the string or '\0' if string is empty
*///-----------------------------------------------------------------------------------------------
char FlyStrCharLast(const char *sz)
{
  char    c = '\0';
  size_t  len;

  len = strlen(sz);
  if(len > 0)
    c = sz[len - 1];

  return c;
}

/*!------------------------------------------------------------------------------------------------
  Finds the filename part of a path. This is a string search only. Does not check for actual
  folders on disk.

  Examples:

  This                    | Finds That
  ----------------------- | ------------
  ~/Work/myfile.c         | myfile.c
  C:\work\things\hello.py | hello.py
  myfile.c                | myfile.c
  /dir/only/              | "" (empty string)
  .                       | "" (empty string)
  ..                      | "" (empty string)

  @param    szPath    pointer to a file or dir path (relative or absolute).
  @return   Returns ptr to name part, or "" if a folder 
*///-----------------------------------------------------------------------------------------------
char * FlyStrPathNameOnly(const char *szPath)
{
  const char *pszName;
  const char *pszLastSlash;

  // if "." or ".." folders, just return "" for name
  if(strcmp(szPath, ".") == 0 || strcmp(szPath, "..") == 0)
    pszName = &szPath[strlen(szPath)];
  else
  {
    // finds the last slash, if no slash return szPath
    pszLastSlash = FlyStrLastSlash(szPath);
    if(pszLastSlash == NULL)
      pszName = szPath;
    else
      pszName = (pszLastSlash + 1);
  }
  return (char *)pszName;
}

/*!------------------------------------------------------------------------------------------------
  Returns ptr to filename part of path. Returns length of base (without file extension).

  Note: this is a string search only. Does not know if the path or file exists.

  Examples:

  This                    | Finds That
  ----------------------- | ------------
  ~/Work/myfile.c         | myfile, len 6
  C:\work\things\hello.py | hello, len 5
  ../myfile               | myfile, len 6
  ../my.dotted.file.ext   | my.dotted.file, len 14
  myfile.c                | myfile.c, len 6
  /dir/only/              | "", len 0
  .                       | "", len 0
  ..                      | "", len 0
  .hidden                 | "", len 0

  @param    szPath    pointer to a file or dir path (relative or absolute).
  @param    pLen      return length of base name
  @return   Returns ptr to name part, or "" if a folder 
*///-----------------------------------------------------------------------------------------------
char * FlyStrPathNameBase(const char *szPath, unsigned *pLen)
{
  const char   *szName;
  const char   *szLastDot;
  size_t        len;

  szName = FlyStrPathNameOnly(szPath);
  szLastDot = strrchr(szName, '.');
  if(szLastDot)
    len = (unsigned)(szLastDot - szName);
  else
    len = strlen(szName);

  if(pLen)
    *pLen = len;
  return (char *)szName;
}

/*!------------------------------------------------------------------------------------------------
  Returns TRUE if this is a slash, that is '/' or '\\'

  @param    c    character to test
  @return   TRUE if character is a slash
*///-----------------------------------------------------------------------------------------------
bool_t FlyStrIsSlash(char c)
{
  return (c == '/' || c == '\\');
}

/*!------------------------------------------------------------------------------------------------
  Returns TRUE if this is a slash, that is '/' or '\\'

  @param    c    character to test
  @return   TRUE if character is a slash
*///-----------------------------------------------------------------------------------------------
bool_t isslash(char c)
{
  return (c == '/' || c == '\\');
}

/*!------------------------------------------------------------------------------------------------
  Finds the last "name" in the path. Could be a folder name or a file name.

  This                    | Finds That | len
  ----------------------- | ---------- | ----
  name                    | name       | 4
  name/                   | name/      | 4
  /path1/path2/folder/    | folder/    | 6
  ~/Work/myfile.c         | myfile.c   | 8
  ..\*                    | *          | 1
  /                       | ""         | 0
  ~                       | ""         | 0

  @param    szPath    pointer to a file or dir path (relative or absolute).
  @param    pLen      ptr to receive length of last name, or NULL if don't care
  @return   Returns ptr to name part, or "" if a folder 
*///-----------------------------------------------------------------------------------------------
char * FlyStrPathNameLast(const char *szPath, unsigned *pLen)
{
  const char *pszStart;
  const char *pszEnd;

  // skip beginning absolute path portion of szPath
  if(*szPath == '~' && (isslash(szPath[1]) || (szPath[1] == '\0')))
    ++szPath;
  if(*szPath == '/')
    ++szPath;

  // no slash?, e.g. "file.txt" or ""
  pszEnd = FlyStrLastSlash(szPath);
  if(!pszEnd)
  {
    pszStart = szPath;
    pszEnd   = szPath + strlen(szPath);
  }

  // slash at end?, e.g. "folder/"
  else if(pszEnd[1] == '\0')
  {
    pszStart = FlyStrPrevSlash(szPath, pszEnd - 1);
    if(pszStart)
      pszStart = pszStart + 1;
    else
      pszStart = szPath;
  }

  // no slash at end, e.g. "../file.c"
  else
  {
    pszStart = pszEnd + 1;
    pszEnd = pszStart + strlen(pszStart);
  }

  if(pLen)
    *pLen = (unsigned)(pszEnd - pszStart);

  return (char *)pszStart;
}

/*!------------------------------------------------------------------------------------------------
  Does this filename have one of the given file extensions?

  Case sensitive. That is ".c" is different from ".C".

  To find a file without an extenstion (e.g. "Makefile"), include a "." at the end of szExts.

  @param    szPath    pointer to a filename (relative or absolute)
  @param    szExts    a list of "." separated extensions, e.g. ".c.cpp.c++.cc"
  @return   Returns ptr to found extention or NULL if not found
*///-----------------------------------------------------------------------------------------------
const char * FlyStrPathHasExt(const char *szPath, const char *szExts)
{
  const char *szExt;
  const char *szFoundExt = NULL;
  const char *psz;
  size_t      len;

  // printf("FlyStrPathHasExt(%s, szExts=%s\n)", szPath, szExts);
  szExt = FlyStrPathExt(szPath);
  if(szExt)
  {
    len = strlen(szExt);
    if(len == 0)
    {
      len = strlen(szExts);
      --len;
      if(strstr(szExts, "..") || szExts[len] == '.')
        szFoundExt = szExt;
    }
    else
    {
      while(TRUE)
      {
        psz = strstr(szExts, szExt);
        if(psz && (psz[len] == '.' || psz[len] == '\0'))
        {
          szFoundExt = szExt;
          break;
        }
        else if(psz == NULL)
          break;
        szExts = psz + 1;
      }
    }
  }

  // printf("found %s\n", FlyStrNullOk(szFoundExt));

  return szFoundExt;
}

/*!------------------------------------------------------------------------------------------------
  Modifies the path string to be the path part only. Either is empty or ends in a slash.

  WARNING: this requires a string where at least 1 character may be added.

  The resulting string is now suitable for FlyStrPathAppend().

  szPath                  | Becomes
  ----------------------- | ------------
  ~/Work/myfile.c         | ~/Work/
  C:\work\things\hello.py | C:\work\things\
  /dir/only/              | /dir/only/
  ..                      | ../
  .                       | ./
  myfile.c                | "" (empty string)

  @param    szPath    pointer to a path string
  @return   nothing
*///-----------------------------------------------------------------------------------------------
void FlyStrPathOnly(char *szPath)
{
  char *psz;

  psz = (char *)FlyStrLastSlash(szPath);
  if(psz == NULL)
  {
    if(strcmp(szPath, ".") == 0 || strcmp(szPath, "..") == 0)
      strcat(szPath, "/");
    else
      *szPath = '\0';
  }
  else
    psz[1] = '\0';
}

/*!------------------------------------------------------------------------------------------------
  Provides length and ptr to path only. Suitable for strncpy() or snprintf() with "%.*s"

  This                    | Returns That
  ----------------------- | ------------
  ~/Work/myfile.c         | ~/Work/
  C:\work\things\hello.py | C:\work\things\
  /dir/only/              | /dir/only/
  ..                      | ../
  .                       | ./
  myfile.c                | ./

  Usage example:

      int         len;
      const char  szFilePath[] = "../src/myfile.c";
      const char  *psz;

      psz = FlyStrPathOnlyLen(szFilePath, &len);
      printf("enclosing folder of %s = %.*s\n", szFilePath, len, psz);

  @param    szPath    pointer to a path string
  @param    pLen      pointer to receive length of path str
  @return   ptr to path str (may be different than passed-in szPath)
*///-----------------------------------------------------------------------------------------------
char * FlyStrPathOnlyLen(const char *szPath, int *pLen)
{
  static const char szParentFolder[]  = "../";
  static const char szThisFolder[]    = "./";
  unsigned          len;
  const char       *psz;

  if(strcmp(szPath, "..") == 0)
  {
    szPath = szParentFolder;
    len = strlen(szPath);
  }
  else
  {
    psz = (char *)FlyStrLastSlash(szPath);
    if(psz != NULL)
      len = (unsigned)(psz - szPath) + 1;
    else      
    {
      szPath = szThisFolder;
      len = strlen(szPath);
    }
  }

  *pLen = len;
  return (char *)szPath;
}

/*!------------------------------------------------------------------------------------------------
  Modify szPath to the parent folder of the current szPath.

  This is a string manipulation only and does not check if any of the folders in the path exist on
  disk. It does NOT convert ~ to the $HOME folder.

  If it worked, the resulting parent folder always ends in a slash..

  Warning: this may add up to 4 characters.

  szPath is unchanged if there is no parent. Some examples:

  Path             | Returns | Parent
  ---------------- | ------- | -------
  /Users/me/work/  | TRUE    | /Users/me/
  /Users/me/file.c | TRUE    | /Users/me/
  ../folder/       | TRUE    | ../
  .                | TRUE    | ../
  ../..            | TRUE    | ../../../
  ~/Work/          | TRUE    | ~/
  ~/Folder/..      | TRUE    | ~/Folder/
  file.c           | FALSE   | file.c
  folder/          | FALSE   | folder/
  ~/               | FALSE   | ~/ and FALSE
  /                | FALSE   | /
  "" (empty)       | FALSE   | ""

  @param    szPath    path to file or folder/
  @return   TRUE if a parent folder, FALSE if no more parents
*///-----------------------------------------------------------------------------------------------
bool_t FlyStrPathParent(char *szPath)
{
  static const char szDotDot[] = "../";
  char   *psz;
  bool_t  fFoundParent = FALSE;

  if(*szPath)
  {
    // special case: if only dots and slashes, then add ../
    // for example, the parent of "../" is "../../"
    psz = szPath;
    while(*psz && (*psz == '.' || FlyStrIsSlash(*psz)))
      ++psz;
    if(*psz == '\0')
    {
      // special case, "/" does not have a parent
      if(FlyStrIsSlash(*szPath) && szPath[1] == '\0')
        fFoundParent = FALSE;

      // special case: replace "." or "./" with "../"
      else if(*szPath == '.' && ((szPath[1] == '\0') || (FlyStrIsSlash(szPath[1]) && (szPath[2] == '\0'))))
      {
        strcpy(szPath, szDotDot);
        fFoundParent = TRUE;
      }

      else
      {
        if(!FlyStrIsSlash(FlyStrCharLast(szPath)))
          strcat(szPath, "/");
        strcat(szPath, szDotDot);
        fFoundParent = TRUE;
      }
    }

    else if(strlen(szPath) > 1)
    {
      psz = szPath + (strlen(szPath) - 1);
      if(FlyStrIsSlash(*psz))
        --psz;
      psz = (char *)FlyStrPrevSlash(szPath, psz);
      if(psz != NULL)
      {
        psz[1] = '\0';
        fFoundParent = TRUE;
      }
    }
  }

  return fFoundParent;
}

/*!------------------------------------------------------------------------------------------------
  String comparison only. Assumes it's a folder if it ends in a slash or is "." or ".."

  Examples of folders:            ".", "..", "/Users/drewg/Documents/"
  Example that is not a folder:   "/Users/drewg/myfile.txt"

  @param    szPath    pionter to asciiz string which contains a file/folder path
  @return   TRUE if this is a folder
*///-----------------------------------------------------------------------------------------------
bool_t FlyStrIsFolder(const char *szPath)
{
  const char *psz;
  if(strcmp(szPath, ".") == 0 || strcmp(szPath, "..") == 0)
    return TRUE;
  else
  {
    psz = FlyStrLastSlash(szPath);
    if(psz && psz[1] == '\0')
      return TRUE;
  }
  return FALSE;
}


/*!------------------------------------------------------------------------------------------------
  Finds the file extension given a filename. Returns NULL if folder.

  Note: this is a text only search (does not check file system). That is, if a folder is called 
  "myfile.txt" it will find the ".txt" extension and not return NULL like it would for "folder/",
   which ends in a slash.

  Since hidden files begin with a ".", this function would treat ".c++" as a hidden file with no
  extension.

  Examples (* means doesn't affect result):

  This              | Finds That
  ----------------- | ----------
  hello.py          | .py
  /Users/me/file.c  | .c
  ~/../main.rs      | .rs
  file.what.hpp     | .hpp
  .hidden           | "" (empty string)
  .hidden.txt       | .txt
  .c++              | "" (empty string)
  x.c++             | .c++
  Makefile          | "" (empty string)
  weird .  filename | ".  filename"
  .                 | NULL
  ..                | NULL
  ...               | "."
  ../some/folder/   | NULL

  @param    szFilename    pointer to filename string
  @param    fHiddenOk     
  @return   Returns pointer to the file extension, or NULL if folder
*///-----------------------------------------------------------------------------------------------
char * FlyStrPathExt(const char *szPath)
{
  const char *szNameOnly;
  const char *psz;

  if(strcmp(szPath, ".") == 0 || strcmp(szPath, "..") == 0)
    return NULL;
  if(isslash(FlyStrCharLast(szPath)))
    return NULL;

  szNameOnly = FlyStrPathNameOnly(szPath);

  // .hidden files: find the .txt in .hidden.txt 
  if(*szNameOnly == '.')
    ++szNameOnly;

  psz = strrchr(szNameOnly, '.');
  if(psz == NULL)
    psz = szNameOnly + strlen(szNameOnly);

  return (char *)psz;
}

/*!------------------------------------------------------------------------------------------------
  Change the file extention to the new one, e.g. change file.md to file.html

  Assumes szPath can handle PATH_MAX.

  @param    szPath  ptr to file path
  @param    szExt   ptr to new file extension
  @return   TRUE if new extension fits in a PATH_MAX string, FALSE if too large
*///-----------------------------------------------------------------------------------------------
bool_t FlyStrPathChangeExt(char *szPath, const char *szExt)
{
  char * szOldExt = (char *)FlyStrPathExt(szPath);
  bool_t  fWorked = TRUE;

  if(szOldExt)
    *szOldExt = '\0';
  if(strlen(szPath) + strlen(szExt) >= PATH_MAX)
    fWorked = FALSE;
  FlyStrZCat(szPath, szExt, PATH_MAX);
  return fWorked;
}

/*!------------------------------------------------------------------------------------------------
  Returns the programming language name based on filename. The following are supported:

  This      | Returns That
  --------- | ------------
  file.c    | c
  .c        | c
  .c++      | C++
  .cc       | C++
  .cpp      | C++
  .cxx      | C++
  .cs       | C#
  .go       | Go
  .java     | Java
  .json     | JSON
  .js       | Javascript
  .py       | Python
  .rb       | Ruby
  .rs       | Rust
  .swift    | Swift
  .ts       | TypeScript
  ~/folder/ | NULL
  makefile  | NULL

  @param    szPath    pointer to filename string
  @return   Returns pointer language name, or NULL if not a known language
*///-----------------------------------------------------------------------------------------------
const char *FlyStrPathLang(const char *szPath)
{
  typedef struct
  {
    const char    *szExt;
    const char    *szLang;
  } flyStrPathLang_t;

  static const flyStrPathLang_t aLangs[] =
  {
    { ".c",     "c" },
    { ".c++",   "C++" },
    { ".cc",    "C++" },
    { ".cpp",   "C++" },
    { ".cxx",   "C++" },
    { ".cs",    "C#" },
    { ".go",    "Go" },
    { ".java",  "Java" },
    { ".json",  "JSON" },
    { ".js",    "Javascript" },
    { ".py",    "Python" },
    { ".rb",    "Ruby" },
    { ".rs",    "Rust" },
    { ".swift", "Swift" },
    { ".ts",    "Typescript" },
  };
  const char *szLang = NULL;
  const char *szExt;
  unsigned    i;

  szExt = FlyStrPathExt(szPath);
  if(szExt)
  {
    for(i = 0; i < NumElements(aLangs); ++i)
    {
      if(strcmp(szExt, aLangs[i].szExt) == 0)
      {
        szLang = aLangs[i].szLang;
        break;
      }
    }
  }

  return szLang;
}

/*!------------------------------------------------------------------------------------------------
  Expands the "~/" into the actual home folder (from environment $HOME) in place.

  If the size of szPath is too small, then it can't expand things, so it returns FALSE. A safe size
  for all paths is PATH_MAX.

  Examples:

  This             | Expands to That
  ---------------- | ---------------
  ~/Work/myfile.c  | /Users/me/Work/myfile.c
  /Users/me/file.c | /Users/me/file.c         (unchanged)
  ~myfile.c        | ~myfile.c                (unchanged)

  @param    szPath    pointer to string which may be expanded
  @param    size      sizeof(szPath), e.g. PATH_MAX
  @return   TRUE if worked FALSE if size too large
*///-----------------------------------------------------------------------------------------------
bool_t FlyStrPathHomeExpand(char *szPath, size_t size)
{
  const char   *szHome;
  size_t        lenHome;
  size_t        lenPath;
  bool_t        fWorked = TRUE;

  if((*szPath == '~') && (szPath[1] == '/'))
  {
    szHome = FlyStrPathHome();
    lenHome = strlen(szHome);
    lenPath = strlen(&szPath[2]);
    if(lenHome + lenPath >= size)
      fWorked = FALSE;
    else
    {
      memmove(&szPath[lenHome], &szPath[1], lenPath);
      memcpy(szPath, szHome, lenHome);
    }
  }

  return fWorked;
}

/*!------------------------------------------------------------------------------------------------
  Copy a string into a potentially smaller space. Does this by prepending "..." if needed and
  appending the end of szSrc that will fit. Useful to display to fixed sized columns.

  Examples with string "/Users/drewg/folder/file.c:"

  szSrc                   | width | szDst (output)
  ----------------------- | ----- | ----------------
  /Users/me/folder/file.c | 50    | /Users/me/folder/file.c
  /Users/me/folder/file.c | 12    | ...er/file.c
  /Users/me/folder/file.c | 3     | ...

  @param    szDst     pointer to string to contain desired width
  @param    width     max width (size - 1) of szDst
  @param    szSrc     pointer to some string
  @return   ptr to szDst
*///-----------------------------------------------------------------------------------------------
const char * FlyStrFit(char *szDst, size_t width, const char *szSrc)
{
  static const char   szDots[] = "...";
  size_t              len = strlen(szSrc);

  // szSrc fits into szDst
  if(len <= width)
    strcpy(szDst, szSrc);

  // very small string, just copy some dots
  else if(width <= strlen(szDots))
  {
    if(width)
      strncpy(szDst, szDots, width);
    szDst[width] = '\0';
  }

  // combo of szDots and szSrc
  else
  {
    strcpy(szDst, szDots);
    strcpy(szDst, &szSrc[len - (width - strlen(szDots))]);
  }

  return szDst;
}

/*!------------------------------------------------------------------------------------------------
  Append the filename to the path in place. String manipulation only (does not check existing
  folders). If cannot append because size of szPath is too small, then leaves szPath unchanged and
  returns FALSE.

  szPath[in]   | szName     | szPath[out]
  ------------ | ---------- | -------------
  "~"          | "file.c"   | "~/file.c"
  "/Users/me"  | "foo"      | "/Users/me/foo"
  "/Users/me/" | "bar"      | "/Users/me/bar"
  ""           | "baz"      | "baz"
  "/Users/me"  | "longname" | FALSE (szPath unchanged)

  @param    szPath    asciiz pointer to path to be modified
  @param    size      sizeof(szPath)
  @param    szName    asciiz pointer to filename string to be appended
  @return   FALSE if existing path + name too long for size
*///-----------------------------------------------------------------------------------------------
bool_t FlyStrPathAppend(char *szPath, const char *szName, size_t size)
{
  size_t      lenName   = strlen(szName);
  size_t      lenPath   = strlen(szPath);
  unsigned    lenSlash  = 0;
  bool_t      fWorked   = TRUE;

  // no path, just use name
  if(!lenPath)
  {
    // name too long
    if(lenName >= size)
      fWorked = FALSE;
    else
      strcpy(szPath, szName);
  }

  // is path, make sure path + name can fit
  else
  {
    if(FlyStrCharLast(szPath) != '/')
      lenSlash = 1;
    if(lenPath + lenSlash + lenName >= size)
      fWorked = FALSE;
    else
    {
      if(lenSlash)
        strcat(szPath, "/");
      strcat(szPath, szName);
    }
  }

  return fWorked;
}

/*!------------------------------------------------------------------------------------------------
  is the memory filled with the byte?

  @param    s         pointer to array
  @param    c         byte (0-255)
  @param    n         length of array

  @return   TRUE if filled (or n==0), FALSE if not.
*///-----------------------------------------------------------------------------------------------
bool_t memisfilled(const void *s, int c, size_t n)
{
  const uint8_t  *p  = s;
  uint8_t         fc = c;   // fill char

  while(s != NULL && n)
  {
    --n;
    if(*p != fc)
      return FALSE;
    ++p;
  }
  return TRUE;
}

/*!------------------------------------------------------------------------------------------------
  Search backward in array for the character as some C environments don't have memrchr().

  @param    s         pointer to array
  @param    c         byte (0-255)
  @param    n         length of array

  @return   ptr to c if found, NULL if not
*///-----------------------------------------------------------------------------------------------
void * FlyMemRChr(const void *s, int c, size_t n)
{
  const uint8_t  *p;
  uint8_t         byte = (uint8_t)c;
  bool_t          fFound = FALSE;

  // no length, no string, not found
  if((n < 1) || (s==NULL))
    return NULL;

  p = (uint8_t *)s + (n-1);
  while(TRUE)
  {
    if(*p == byte)
    {
      fFound = TRUE;
      break;
    }
    if(p == s)
      break;
    --p;
  }

  return (fFound ? (void *)p : NULL);
}

/*!------------------------------------------------------------------------------------------------
  Compare 2 strings, case insensitive, as some C environments don't have memicmp().

  @param    pThis      pointer to data
  @param    pThat      pointer to data
  @param    len        length of data

  @return   0 if strings compare the same, -1 if pThis < pThat, 1 if pThis > pThat
*///-----------------------------------------------------------------------------------------------
int FlyMemICmp(const void *pThis, const void *pThat, size_t len)
{
  const uint8_t   *pStr1 = pThis;
  const uint8_t   *pStr2 = pThat;

  while(len)
  {
    if(toupper(*pStr1) < toupper(*pStr2))
      return -1;
    else if(toupper(*pStr1) > toupper(*pStr2))
      return 1;
    ++pStr1;
    ++pStr2;
    --len;
  }
  return 0;
}

/*!------------------------------------------------------------------------------------------------
  A swap function for any sized bits of memory

  @param  pThis    ptr to array of items
  @param  pThat     number of elements in the array
  @param  elemSize  size of each element
  @return none
*///-----------------------------------------------------------------------------------------------
void FlyMemSwap(void *pThis, void *pThat, size_t size)
{
  uint8_t *pThisByte = pThis;
  uint8_t *pThatByte = pThat;
  uint8_t tmp;

  do
  {
    tmp = *pThisByte;
    *pThisByte++ = *pThatByte;
    *pThatByte++ = tmp;
  } while(--size > 0);
}

/*!------------------------------------------------------------------------------------------------
  Return offset of difference, or SIZE_MAX if no difference

  @param  pThis    ptr to array of items
  @param  pThat     number of elements in the array
  @param  elemSize  size of each element
  @return offset of difference or FLYMEM_NO_DIFF if no difference
*///-----------------------------------------------------------------------------------------------
size_t FlyMemDiff(const void *pThis, const void *pThat, size_t size)
{
  const uint8_t *pThisByte = pThis;
  const uint8_t *pThatByte = pThat;
  size_t  i;

  for(i = 0; i < size; ++i)
  {
    if(*pThisByte++ != *pThatByte++)
      break;
  }

  return (i == size) ? FLYMEM_NO_DIFF : i;
}

/*!------------------------------------------------------------------------------------------------
  Search string for character, case insensitive.

  @param    pThis      pointer to data
  @param    pThat      pointer to data
  @param    len        length of data

  @return   0 if strings compare the same, -1 if pThis < pThat, 1 if pThis > pThat
*///-----------------------------------------------------------------------------------------------
char * FlyStrIChr(const char *sz, int c)
{
  char * szFound = NULL;
  while(sz && *sz)
  {
    if(toupper(c) == toupper(*sz))
    {
      szFound = (char *)sz;
      break;
    }
    ++sz;
  }
  return szFound;
}

/*-------------------------------------------------------------------------------------------------
  Copy the string, keeping it asciiz and not copying more than lenLeft chars. returns the reduced
  leftLen.

  param   szDst     destination string
  param   szSrc     source string
  param   lenLeft   length left in string
  returns reduced lenLeft
-------------------------------------------------------------------------------------------------*/
unsigned FlyStrCpy(char *szDst, const char *szSrc, unsigned lenLeft)
{
  unsigned  len;

  // copy as much of the source string as we can into dst
  if(lenLeft)
  {
    // adjust len so the '\0' will stay within sizeof(szDst[])
    len = strlen(szSrc);
    if(len > lenLeft - 1)
    {
      len = lenLeft - 1;
      lenLeft = 0;
    }
    else
      lenLeft -= len;

    if(len)
      strncpy(szDst, szSrc, len);
    szDst[len] = '\0';
  }

  return lenLeft;
}

/*-------------------------------------------------------------------------------------------------
  Same as strcmp(), but can handle NULLs

  @param  szThis    compares this
  @param  szThat    to that
  @return -1 if szThis < szThat, 0 if same, 1 otherwise
-------------------------------------------------------------------------------------------------*/
int FlyStrCmp(const char *szThis, const char *szThat)
{
  int ret;

  if(szThis && szThat)
    ret = strcmp(szThis, szThat);
  else
  {
    // both NULL, equal
    if(szThis == szThat)
      ret = 0;
    else if(szThis)
      ret = 1;
    else
      ret = -1;
  }

  return ret;
}

/*-------------------------------------------------------------------------------------------------
  Similar to strstr(), but works allows specifying length of haystack

  @param  szHaystack    
  @param  szNeedle      size of destination string (1-n)
  @param  len           length of szSrc substring
  @return length actually copied (may be 0)
-------------------------------------------------------------------------------------------------*/
char * FlyStrNStr(const char *szHaystack, const char *szNeedle, size_t len)
{
  size_t      lenNeedle = strlen(szNeedle);
  const char *szFound   = NULL;

  while(len >= lenNeedle)
  {
    if(*szHaystack == *szNeedle && memcmp(szHaystack, szNeedle, lenNeedle) == 0)
    {
      szFound = szHaystack;
      break;
    }
    ++szHaystack;
    --len;
  }

  return (char *)szFound;
}

/*-------------------------------------------------------------------------------------------------
  Copy len bytes of szSrc to szDst, making sure result is always '\0' terminated.

  @param  szDst     destination string
  @param  sizeDst   size of destination string (1-n)
  @param  szSrc     source string, may or may not be '\0' terminated
  @param  len       length of szSrc substring
  @return length actually copied (may be 0)
-------------------------------------------------------------------------------------------------*/
size_t FlyStrNCpy(char *szDst, size_t sizeDst, const char *szSrc, size_t len)
{
  if(len >= sizeDst)
    len = sizeDst - 1;
  strncpy(szDst, szSrc, len);
  szDst[len] = '\0';

  return len;
}

/*!-------------------------------------------------------------------------------------------------
  Append the szSrc string to szDst. Will not exceed size of szDst. Always '\0' terminates.

  @param  szDst     destination asciiz string, or NULL to just get size
  @param  size      size of destination string
  @param  szSrc     source string
  @param  srcLen    length of src string
  @return # of bytes copied (or would have been copied if szDst == NULL)
*///------------------------------------------------------------------------------------------------
size_t FlyStrCat(char *szDst, size_t size, const char *szSrc, size_t srcLen)
{
  size_t dstLen;

  if(szDst)
  {
    dstLen = strlen(szDst);
    if(dstLen >= size)
      dstLen = size - 1;
    if(srcLen > (size - 1) - dstLen)
      srcLen = (size - 1) - dstLen;
    if(srcLen)
    {
      strncpy(&szDst[dstLen], szSrc, srcLen);
      szDst[dstLen + srcLen] = '\0';
    }
  }

  return srcLen;
}

/*!-------------------------------------------------------------------------------------------------
  Fill at end of string. Ensures that resulting string is '\0' terminated.

  @param  szDst     destination asciiz string, or NULL to just get size
  @param  size      size of destination string
  @param  c         character to fill
  @param  fillLen   # of characters to fill
  @return # of bytes copied (or would have been copied if szDst == NULL)
*///------------------------------------------------------------------------------------------------
size_t FlyStrCatFill(char *szDst, size_t size, char c, size_t fillLen)
{
  size_t dstLen;

  if(szDst)
  {
    dstLen = strlen(szDst);
    if(dstLen >= size)
      dstLen = size - 1;
    if(fillLen > (size - 1) - dstLen)
      fillLen = (size - 1) - dstLen;
    if(fillLen)
    {
      memset(&szDst[dstLen], c, fillLen);
      szDst[dstLen + fillLen] = '\0';
    }
  }

  return fillLen;
}

/*!------------------------------------------------------------------------------------------------
  Insert a substring. Resulting string is always '\0' terminated. May truncate right size of
  szDst string. May copy only some of szSrc string, if too long.

  Example: szDst="ac", offset=1, sizeDst=5, szSrc="b", reesults in szDst="abc", return 1

  @param    szDst     pointer to destination string (szDst MUST be n+1 in size)
  @param    offset    0-n
  @param    sizeDst   size of destination string
  @param    szSrc     pointer to source string

  @return   length pasted
*///-----------------------------------------------------------------------------------------------
size_t FlyStrIns(char *szDst, size_t offset, size_t sizeDst, const char *szSrc)
{
  size_t  lenDst;
  size_t  lenPaste = 0;
  size_t  lenMove;

  // nothing to do if bad input parameters
  if(sizeDst > 0 && offset < sizeDst)
  {
    // keep lenDst in valid range
    lenDst = strlen(szDst);
    if(lenDst >= sizeDst)
    {
      lenDst = sizeDst - 1;
      szDst[lenDst] = '\0';
    }

    // keep offset in valid range
    if(offset > lenDst)
      offset = lenDst;

    // keep paste len in valid range
    lenPaste = strlen(szSrc);
    if(offset + lenPaste >= sizeDst)
      lenPaste = (sizeDst - offset) - 1;

    if(lenPaste)
    {
      // make room for paste if offset in middle of string
      if(offset + lenPaste < sizeDst - 1)
      {
        lenMove = lenDst - offset;
        if(offset + lenPaste + lenMove >= sizeDst)
          lenMove = sizeDst - (offset + lenPaste);
        memmove(&szDst[offset], &szDst[offset + lenPaste], lenMove + 1);
      }

      // paste is large enough, right-hand side is truncated
      else
        szDst[sizeDst - 1] = '\0';

      memcpy(&szDst[offset], szSrc, lenPaste);
    }
  }

  return lenPaste;
}

/*!------------------------------------------------------------------------------------------------
  Find the character in the string, but only search len characters

  @param    sz    string to search
  @param    len   max length of string to search
  @param    c     character to search for
  @return   ptr to found char or NULL
*///-----------------------------------------------------------------------------------------------
char * FlyStrNChr(const char *sz, size_t len, char c)
{
  const char *szFound = NULL;

  while(*sz && len)
  {
    if(*sz == c)
    {
      szFound = sz;
      break;
    }
    ++sz;
    --len;
  }

  return (char *)szFound;
}

/*!------------------------------------------------------------------------------------------------
  Like strpbrk(), but search substring

  Example: `FlyStrNChrMatch(szLine, NULL, "[{\r\n")` will find the 1st of `{`, `[` or end of line.

  @param    sz        string to search
  @param    szEnd     NULL 
  @param    szMatch   a set of characters to look for
  @return   ptr to c, or NULL if not found in line
*///-----------------------------------------------------------------------------------------------
char *FlyStrNChrMatch(const char *sz, const char *szEnd, const char *szMatch)
{
  char *szFound = NULL;

  while(*sz && sz < szEnd)
  {
    if(strchr(szMatch, *sz) != NULL)
    {
      szFound = (char *)sz;
      break;
    }
    ++sz;
  }

  return szFound;
}

/*!------------------------------------------------------------------------------------------------
  Transforms string segment to single line paragraphs. Removes any extra spaces.

  input:  " a  string\nof things\n\n  lifts   a\n   wing "
  output: "a string of things\nlifts a wing"

  input:  "string here  \n\n\n   "
  output: "string here\n"

  @param    pData   an array of nedChars that will be modified (may be shorter, never longer)
  @param    len     length to string segment

  @return   new length (same or shorter, could be 0)
*///-----------------------------------------------------------------------------------------------
size_t FlyMemRemoveExtraSpaces(char *pData, size_t len)
{
  // nedChar_t  *pDataOrg = pData;
  char        *p;
  size_t      nSpaces;
  size_t      nLfs;             // linendings, \n or \r\n
  size_t      i;
  bool_t      fEdge = TRUE;

  // FlyLogDump(pDataOrg, len, 16, 2);

  i = 0;
  while(i < len)
  {
    // count spaces and linefeeds
    nSpaces = 0;
    nLfs    = 0;
    if(isblank(*pData) || *pData == '\n')
    {
      // count spaces and line feeds (we may remove some or all)
      p = pData;
      while((i + nSpaces < len) && (isblank(*p) || *p == '\n'))
      {
        if(*p == '\n')
          ++nLfs;
        ++nSpaces;    // '\n', '\t' and ' ' are all considered spaces
        ++p;
      }

      // for end of string, same as start of string, delete all spaces
      if(i + nSpaces >= len)
        fEdge = TRUE;

      // FlyLogPrintf("i %zu, len %zu, nSpaces %zu, nLfs %zu, fEdge %u\n", i, len, nSpaces, nLfs, fEdge);
    }

    // transform the string
    if(nSpaces > 0)
    {
      // for multiple '\n', always leave one '\n'
      // at start/end of sting, delete all spaces, but in the middle leave 1
      if(nLfs > 1)
        fEdge = FALSE;
      if(!fEdge)
        --nSpaces;

      // remove all extra spaces and fill in space
      if(nSpaces)
      {
        memmove(pData, pData + nSpaces, (len - i) - nSpaces);
        len -= nSpaces;

      }

      if(!fEdge)
      {
        *pData = (nLfs > 1) ? '\n' : ' ';
        ++pData;
        ++i;
      }

      // FlyLogDump(pDataOrg, len, 16, 2);

    }

    else
    {
      ++i;
      ++pData;
    }

    fEdge = FALSE;
  }

  return len;
}

/*!------------------------------------------------------------------------------------------------
  Determine where to wrap a paragraph into a line

    1. Always at least 1 word
    2. Stops at \n
    3. Stops at space following word
    4. Stops at len

  @param    pLine       pointer paragraph
  @param    len         length of pline
  @param    wrapWidth   length to wrap (1-n)
  @return   position at where to wrap
*///-----------------------------------------------------------------------------------------------
size_t FlyMemFindWrap(const char *pLine, size_t len, size_t wrapWidth)
{
  const char  *p;
  const char  *pLastWord = NULL;
  size_t       i;

  p = pLine;
  for(i=0; i < len; ++i)
  {
    if(isblank(*p) || FlyCharIsEol(*p))
    {
      if(i > wrapWidth)
      {
        if(pLastWord)
          i = (size_t)(pLastWord - pLine);
        else
          i = (size_t)(p - pLine);
        break;
      }
      else if(FlyCharIsEol(*p))
      {
        i = (size_t)(p - pLine);
        break;
      }

      pLastWord = p;
    }
    ++p;
  }

  return i;
}

/*!------------------------------------------------------------------------------------------------
  Find difference in 2 strings. Points out position where they differ

  @param  szThis    pointer to string
  @param  szThat    pointer to 2nd string
  @param  n         length to check
  @return position where there is difference, or n if no difference
*///-----------------------------------------------------------------------------------------------
size_t FlyStrWhereDiff(const char *szThis, const char *szThat, size_t n)
{
  size_t    i;

  for(i = 0; i < n; ++i)
  {
    if(*szThis != *szThat)
      break;
    ++szThis;
    ++szThat;
  }

  return i;
}

/*--------------------------------------------------------------------------------------------------
  Free the variable unless NULL. Always returns NULL so the following pattern can be used:

      szMyString = FlyStrFreeIf(szMyString);

  @return   NULL
*///-----------------------------------------------------------------------------------------------
void * FlyStrFreeIf(void *ptr)
{
  if(ptr)
    FlyFree(ptr);
  return NULL;
}

/*!------------------------------------------------------------------------------------------------
  Dump (print) some data to the screen in classic dump format.

  Slight difference from "hexdump -C": bytes >= 0xa0 are displayed as '.' and does not try to
  display UTF-8 characters.

  Example output:

    00000000  01 02 00 a0 00 5f 70 72  69 6e 74 66 00 00 9e 00  |....._printf....|
    00000010  12                                                |.|

  @param  pData     ptr to data
  @param  len       length of data
  @return none
*///-----------------------------------------------------------------------------------------------
void FlyStrDump(const void *pData, size_t len)
{
  char  szLine[FlyStrDumpLineSize(FLYSTR_DUMP_COLS)];
  FlyStrDumpEx(pData, len, szLine, FLYSTR_DUMP_COLS, 0);
}

/*!------------------------------------------------------------------------------------------------
  Dump (print) data with any line length to screen, as it dumps each line to szLine.

  Note: sizeof(szLine) must be at least FlyStrDumpLineSize() in size.

  Example output:

    00008050  01 02 00 00 00 5f 70 72  69 6e 74 66 00 00 00 00  |....._printf....|
    00008290  12                                                |.|

  @param  pData   ptr to data
  @param  len     length of data
  @param  szLine  caller supplied line of size FlyStrDumpLineSize(cols)
  @param  col     # of columns (1-n)
  @param  addr    address for 1st line, displayed in hex
  @return none
*///-----------------------------------------------------------------------------------------------
void FlyStrDumpEx(const void *pData, size_t len, char *szLine, unsigned cols, long addr)
{
  const uint8_t  *pByte = pData;
  unsigned        thisLen;

  while(len)
  {
    thisLen = (len > cols) ? cols : len;
    FlyStrDumpLine(szLine, pByte, thisLen, cols, addr);
    printf("%s\n", szLine);
    len       -= thisLen;
    pByte     += thisLen;
    addr      += thisLen;
  }
}

/*!------------------------------------------------------------------------------------------------
  Dump a single line to a string.

  Note: sizeof(szLine) must be at least FlyStrDumpLineSize() in size.

  Example output:

    00008050  01 02 00 00 00 5f 70 72  69 6e 74 66 00 00 00 00  |....._printf....|

  @param  szLine    ptr to receive dump string (output)
  @param  pData     pointer to data
  @param  len       length of data in bytes (1 - cols)
  @param  cols      # of columns in each line, determines spacing of |...|
  @param  addr      address to display for that line (0 - n), displayed in hex
  @return len of output string
*///-----------------------------------------------------------------------------------------------
unsigned FlyStrDumpLine(char *szLine, const void *pData, unsigned len, unsigned cols, long addr)
{
  const char     *szSep;
  const uint8_t  *pByte   = pData;
  int             offset  = 0;
  unsigned        i;

  if(len == 0)
  {
    *szLine = '\0';
  }

  else
  {
    // print "00008050" part of line
    offset = sprintf(szLine, "%08lx  ", addr);

    // print "01 02 00 00 00 5f 70 72  69 6e 74 66 00 00 00 00" part of line
    for(i = 0; i < cols; ++i)
    {
      szSep = (i == (cols >> 1)) ? " " : "";
      if(i < len)
        offset += sprintf(&szLine[offset], "%s%02x ", szSep, pByte[i]);
      else
        offset += sprintf(&szLine[offset], "%s   ", szSep);
    }

    // print "|....._printf....|" part of line
    offset += sprintf(&szLine[offset], " |");
    for(i = 0; i < cols; ++i)
    {
      if(i < len)
        szLine[offset++] = isprint(pByte[i]) ? (char)(pByte[i]) : '.';
      else
        szLine[offset++] = ' ';
    }
    offset += sprintf(&szLine[offset], "|");
  }

  return offset;
}

/*!------------------------------------------------------------------------------------------------
  Allocate a copy of the array of strings on the heap. Input may be a hard-coded array of strings,
  local strings, etc.... Note: the array of strings can be "empty" by having the 1st element be
  NULL.

  The array of strings MUST end in a NULL ptr.

  @param  asz       a valid, NULL terminated array of strings
  @return an allocated array of strings, which may have no strings, just the terminating NULL
*///-----------------------------------------------------------------------------------------------
char **FlyStrArrayCopy(const char **asz)
{
  size_t      i;
  size_t      numElem;
  size_t      sizeOf;
  char      **ppChars = NULL;
  char       *psz;

  // calculate memory needed
  numElem = FlyStrArrayNumElem(asz);
  sizeOf  = FlyStrArraySizeOf(asz);

  // allocate the memory of asz was valid
  if(numElem)
    ppChars = FlyStrArrayMalloc(numElem, sizeOf);

  // fill in the newly allocated memory with a copy of text from asz
  if(ppChars)
  {
    psz = FlyStrArrayText((const char **)ppChars);
    for(i = 0; asz[i]; ++i)
    {
      ppChars[i] = psz;
      strcpy(psz, asz[i]);
      psz += strlen(asz[i]) + 1;
    }
  }

  return ppChars;
}

/*!------------------------------------------------------------------------------------------------
  Return the position in a NULL terminated array of strings

  @param  aszHaystack     array of strings to search
  @param  szNeedle        string to search for
  @return index (0-n) or -1 if not found
*///-----------------------------------------------------------------------------------------------
int FlyStrArrayFind(const char **aszHaystack, const char *szNeedle)
{
  unsigned  i;
  unsigned  index = -1;

  for(i = 0; i < INT_MAX && aszHaystack && aszHaystack[i]; ++i)
  {
    if(strcmp(aszHaystack[i], szNeedle) == 0)
    {
      index = i;
      break;
    }
  }

  return index;
}

/*!------------------------------------------------------------------------------------------------
  Free the strings created by FlyStrArrayMalloc()

  Note: FlyStrArrayMalloc() allocates all strings in a single heap allocation, so we can just free
  it.

  @param  asz      Returned value from FlyStrArrayMalloc()
  @return a memory location 
*///-----------------------------------------------------------------------------------------------
void FlyStrArrayFree(const char **asz)
{
  if(asz)
    FlyFree(asz);
}

/*!------------------------------------------------------------------------------------------------
  Compare two string arrays. Doesn't care if they are hard-coded or allocated on heap. Stops of
  1st string that is different.

  @param  aszThis      a valid, NULL terminated array of strings
  @param  aszThat      a valid, NULL terminated array of strings
  @return -1 if this less than that, 0 if equal, 1 if greater than
*///-----------------------------------------------------------------------------------------------
int FlyStrArrayCmp(const char **aszThis, const char **aszThat)
{
  unsigned    i;
  int         ret = 0;

  for(i=0; (ret == 0) && aszThis[i] && aszThat[i]; ++i)
  {
    ret = strcmp(aszThis[i], aszThat[i]);
  }

  if(ret == 0)
  {
    if(aszThis[i] != NULL)
      ret = 1;
    else if(aszThat[i] != NULL)
      ret = -1;
  }
  if(ret > 0)
    ret = 1;
  if(ret < 0)
    ret = -1;

  return ret;
}

/*!------------------------------------------------------------------------------------------------
  Allocate space for an array of strings with enough room for all the text, as specified by sizeOf.
  Example:

  char    **asz;
  char     *psz;
  size_t    i;
  const    *aStrings[] = { "alpha", "beta", "gamma", NULL };

  asz = FlyStrArrayMalloc(FlyStrArrayNumElem(aStrings), FlyStrArraySizeOf(aStrings));
  if(asz)
  {
    psz = FlyStrArrayText(asz);
    for(i = 0; i < NumElements(aStrings) - 1; ++i)
    {
      asz[i] = psz;
      strcpy(asz[i], aStrings[i]);
      psz += strlen(aStrings[i]) + 1;
    }
    FlyStrArrayPrint(asz);
  }

  @param  numElem     number of string pointers that can be set by the higher layer
  @param  sizeOf      total size of text, including '\0', that will be filled in
  @return pointer to a NULL terminated array of strings
*///-----------------------------------------------------------------------------------------------
char **FlyStrArrayMalloc(size_t numElem, size_t sizeOf)
{
  char    **asz         = NULL;
  char     *psz;
  size_t    i;
  size_t    sizePtrs;

  sizePtrs = sizeof(char *) * (numElem + 1);
  asz      = FlyAlloc(sizePtrs + sizeOf);

  // if allocated OK, then initialize all strings to point to an empty string
  if(asz)
  {
    memset(asz, 0, sizePtrs + sizeOf);
    psz = ((char *)asz) + sizePtrs;
    for(i = 0; i < numElem; ++i)
    {
      asz[i] = psz;
    }
    asz[i] = NULL;
  }

  return asz;
}

/*!------------------------------------------------------------------------------------------------
  Combine all the strings in an array into a single allocated string

  Delimeter is placed between each string. It can be "" for no delimeter, " ", ":" or "anything".

  It is OK if some (or all) the strings in the array are NULL. They will be skipped.

  @param  count     count of strings in array
  @param  asz       array of ASCIIZ string pointers
  @param  szDelim   delimiter to separate strings with. If NULL, defaults to space.
  @return ptr to allocated combined string
*///-----------------------------------------------------------------------------------------------
char * FlyStrArrayCombine(int count, const char **asz, const char *szDelim)
{
  size_t  size    = 1;
  size_t  lenDelim;
  int     i;
  char   *sz      = NULL;
  bool_t  fFirst  = TRUE;

  // get delimeter
  if(!szDelim)
    szDelim = " ";
  lenDelim = strlen(szDelim);

  // allocate size of combined string
  if(count >= 0)
  {
    size += lenDelim * count;
    for(i = 0; i < count; ++i)
    {
      if(asz[i])
        size += strlen(asz[i]);
    }
    sz = FlyAlloc(size);
  }

  if(sz)
  {
    for(i = 0; i < count; ++i)
    {
      if(asz[i])
      {
        if(!fFirst)
          strcat(sz, szDelim);
        if(fFirst)
        {
          strcpy(sz, asz[i]);
          fFirst = FALSE;
        }
        else
          strcat(sz, asz[i]);
      }
    }
  }

  return sz;
}

/*!------------------------------------------------------------------------------------------------
  Get the text of the 1st element, aka asz[0]). If FlyStrArrayMalloc() was used, this is the
  area where to start filling in strings. Do not exceed sizeOf.

  @param  asz     a valid, NULL terminated array of strings
  @return ptr to text of 1st item. 
*///-----------------------------------------------------------------------------------------------
char *FlyStrArrayText(const char **asz)
{
  return (asz && *asz) ? (char *)(*asz) : NULL;
}

/*!------------------------------------------------------------------------------------------------
  Determine number of elemens in a NULL terminated string array. For example:

  const char *asz[] = { "alpha", "beta", "gamma", NULL };
  FlyStrArrayNumElem(asz) == 3

  @param  asz     a valid, NULL terminated array of strings
  @return number of strings in the NULL terminated string array.
*///-----------------------------------------------------------------------------------------------
size_t FlyStrArrayNumElem(const char **asz)
{
  size_t    numElem;

  for(numElem = 0; asz && asz[numElem]; ++numElem)
    /* just counting */;

  return numElem;
}

/*!------------------------------------------------------------------------------------------------
  Determine total sizeof(text) of all strings in a NULL terminated string array. For example:

  const char *asz[] = { "alpha", "beta", "gamma", NULL };
  FlyStrArraySizeOf(asz) == 17

  @param  asz     a valid, NULL terminated array of strings
  @return total text size of all strings, including '\0's
*///-----------------------------------------------------------------------------------------------
size_t FlyStrArraySizeOf(const char **asz)
{
  size_t    i;
  size_t    sizeOf = 0;

  for(i = 0; asz && asz[i]; ++i)
  {
    sizeOf += strlen(asz[i]) + 1;
  }

  return sizeOf;
}

/*!------------------------------------------------------------------------------------------------
  Determine length longest string in the array

  @param  asz     a valid, NULL terminated array of strings
  @return length of longest string
*///-----------------------------------------------------------------------------------------------
size_t FlyStrArrayMaxLen(const char **asz)
{
  size_t    i;
  size_t    maxLen = 0;
  size_t    len;

  for(i = 0; asz && asz[i]; ++i)
  {
    len = strlen(asz[i]);
    if(len > maxLen)
      maxLen = len;
  }

  return maxLen;
}

/*!------------------------------------------------------------------------------------------------
  Print the NULL terminated array of strings in the form of:

  0: string1
  1: string2
  2: string3

  @param  asz     a valid, NULL terminated array of strings
  @return none
*///-----------------------------------------------------------------------------------------------
void FlyStrArrayPrint(const char **asz)
{
  size_t    i;

  for(i = 0; asz && asz[i]; ++i)
    printf("%zu: %s\n", i, asz[i]);
}

#if 0
/*!------------------------------------------------------------------------------------------------
  Return TRUE if this is a line comment style header. Must be on a header found by FlyStrHdrFind().

  @param  szHdr     ptr to header found by FlyStrHdrFind()
  @return TRUE if doc header
*///-----------------------------------------------------------------------------------------------
bool_t FlyStrHdrIsLine(const char *szHdr)
{
  bool_t        fIsLineHdr  = FALSE;
  unsigned      lenOpen     = 0;
  const char    *pszOpen;

  if(HdrIsOpen(szHdr, &lenOpen, &pszOpen))
  {
    if(pszOpen == m_szOpenRust || pszOpen == m_szOpenPy)
      fIsLineHdr = TRUE;
  }

  return fIsLineHdr;
}

/*!------------------------------------------------------------------------------------------------
  Return TRUE if this is a PyDoc document comment

  @param  szHdr     ptr to header found by FlyStrHdrFind()
  @return TRUE if doc header
*///-----------------------------------------------------------------------------------------------
bool_t FlyStrHdrIsPyDoc(const char *szHdr)
{
  unsigned len = strlen(m_szOpenPyDoc);
  return (strncmp(szHdr, m_szOpenPyDoc, len) == 0 && szHdr[len] == '!') ? TRUE : FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Determine length block comment (aka header)

  Works with most languages such as C, C++, Javascript, Python, Rust, Swift, etc...

  See also [FlyStrHdrFind()](#FlyStrHdrFind).

  @param  szHdr    ptr to start of block comment
  @return length of header
*///-----------------------------------------------------------------------------------------------
size_t FlyStrHdrLen(const char *szHdr)
{
  const char *  pszEnd        = NULL;
  const char *  psz;
  const char *  psz2;
  const char *  pszOpen;
  const char *  pszClose;
  unsigned      lenOpen;
  size_t        hdrLen        = 0;
  char          c;

  if(HdrIsOpen(szHdr, &lenOpen, &pszOpen))
  {
    // Line comments like Python or Rust
    if(pszOpen == m_szOpenPy || pszOpen == m_szOpenRust)
    {
      pszEnd = szHdr;
      c = (pszOpen == m_szOpenPy) ? '#' : '/';
      do
      {
        pszEnd = FlyStrLineNext(pszEnd);
        psz2 = FlyStrSkipWhite(pszEnd);
      } while(*psz2 == c);
    }

    // C-like languages: C++, C#, Java, Javascript, Objective-C, PHP, Swift, etc...
    // and Python Doc comments
    else
    {
      if(pszOpen == m_szOpenC)
        pszClose = m_szCloseC;
      else
        pszClose = m_szOpenPyDoc;

      psz = strstr(szHdr + lenOpen, pszClose);
      if(psz)
        pszEnd = psz + strlen(pszClose);
    }
  }

  if(pszEnd)
      hdrLen = pszEnd - szHdr;

  return hdrLen;
}

/*!------------------------------------------------------------------------------------------------
  Strips a header down to the content. This does the following:

  1. On block comments, ignores 1st and last line
  2. Determines indent of real content based on 1st non-blank line
  3. Strips consistent with indent, so content is flush left
  4. Empty lines (with nothing but spaces/tabs) are empty (with just \n)
  5. Converts tabs to spaces using indent for column

  @param  szDst         ptr to destination char array. Can be NULL if just getting size
  @param  size          sizeof szDst (or maxSize if szDst == NULL)
  @param  szHdr         ptr to header found by FlyStrHdrFind()
  @param  szLineBeg     ptr to beginning of header line, in case header is indented
  @return length of header contents, or 0 if not a header
*///-----------------------------------------------------------------------------------------------
size_t FlyStrHdrStrip(char *szDst, size_t size, const char *szHdr, const char *szLineBeg)
{
  const char *  pszHdrEnd;
  const char *  psz;
  const char *  psz2;
  size_t        strippedLen = 0;
  size_t        thisLen;
  unsigned      indent;
  bool_t        fIndentFound = FALSE;
  unsigned      lenOpen;
  const char   *pszOpen;

  // only process headers
  if(HdrIsOpen(szHdr, &lenOpen, &pszOpen))
  {

    // determine where to stop processing
    pszHdrEnd = szHdr + FlyStrHdrLen(szHdr);
    if((pszOpen == m_szOpenC) || (pszOpen == m_szOpenPyDoc))
      pszHdrEnd = FlyStrLineBeg(szHdr, pszHdrEnd);

    // printf("\nFlyStrHdrStrip(dst %p, size %zu, szHdr %p, szLineBeg %p, pszHdrEnd %p\n",
    //     szDst, size, szHdr, szLineBeg, pszHdrEnd);
    // printf("%.*s\n", (int)FlyStrLineLen(szLineBeg), szLineBeg);
    // printf("%.*s\n", (int)FlyStrLineLen(pszHdrEnd), pszHdrEnd);

    // for line comments, 1st line may have content
    if(FlyStrHdrIsLine(szHdr))
    {
      // printf("isLine\n");
      psz2 = szLineBeg;
      psz = szHdr + lenOpen;
      if(*psz == '!')
        ++psz;
      if(!FlyStrLineIsBlank(psz))
      {
        psz = FlyStrSkipWhite(psz);
        fIndentFound = TRUE;
      }
    }

    // look for indent on 1st non-blank line
    if(!fIndentFound)
    {
      indent = szHdr - szLineBeg;
      // printf("indent for star %u\n", indent);
      psz = psz2 = FlyStrLineNext(szHdr);

      // skip the '*' in column 1 for block comments
      while(psz < pszHdrEnd)
      {
        // printf("%.*s\n", (int)FlyStrLineLen(psz2), psz2);
        if(FlyStrLineLen(psz2) >= indent + 1 && psz[indent + 1] == '*')
          psz = FlyStrSkipWhite(&psz[indent + 2]);
        else
          psz = FlyStrSkipChars(psz, " \t#/");
        if(!FlyStrLineIsBlank(psz))
          break;
        psz = psz2 = FlyStrLineNext(psz);
      }
    }

    // psz points to 1st content, psz2 to 1st content line
    indent = (psz - psz2);
    // printf("indent %u\n", indent);
    psz = psz2;
    while(psz < pszHdrEnd)
    {
      // printf("%.*s\n", (int)FlyStrLineLen(psz), psz);

      // length of line
      psz2 = FlyStrLineEnd(psz);
      if(psz2 > pszHdrEnd)
        psz2 = pszHdrEnd;

      // copy the data if not short line
      if((psz2 - psz) > indent)
      {
        psz += indent;
        thisLen = (psz2 - psz);
        // printf("thisLen %zu\n", thisLen);
        if(szDst)
          strncpy(&szDst[strippedLen], psz, thisLen);
        strippedLen += thisLen;
      }

      // add in line for data
      if(szDst)
      {
        // keep string NUL terminated
        szDst[strippedLen] = '\n';
        szDst[strippedLen + 1] = '\0';
      }
      ++strippedLen;  // for \n
      // printf("strippedLen %zu\n", strippedLen);

      psz = FlyStrLineNext(psz2);
    }
  }

  // NUL terminate, even if 0 length
  if(szDst != NULL)
    szDst[strippedLen] = '\0';

  return strippedLen;
}
#endif

/*!------------------------------------------------------------------------------------------------
  Allocate a copy of a string on the heap.

  Same as strdup(), but uses FlyAlloc() rather than malloc().

  @param   sz     ptr to a string, which may or may not be NUL terminated
  @return  ptr an asciiz string on the heap
*///-----------------------------------------------------------------------------------------------
char * FlyStrClone(const char *sz)
{
  return FlyStrAllocN(sz, strlen(sz));
}

/*!------------------------------------------------------------------------------------------------
  Allocate a copy of a string on the heap.

  TODO: Remove FlyStrAlloc() as FlyStrClone() is a better name.

  Same as strdup(), but uses FlyAlloc() rather than malloc().

  @param   sz     ptr to a string, which may or may not be NUL terminated
  @return  ptr an asciiz string on the heap
*///-----------------------------------------------------------------------------------------------
char * FlyStrAlloc(const char *sz)
{
  return FlyStrAllocN(sz, strlen(sz));
}

/*!------------------------------------------------------------------------------------------------
  Allocates a substring to the heap. Resulting string is `\0` terminated.

  Same as strbdup(), but uses FlyAlloc() rather than malloc().


  @param   sz     ptr to a string, which may or may not be `\0` terminated
  @param   len    length of substring to copy
  @return  ptr an asciiz string on the heap
*///-----------------------------------------------------------------------------------------------
char * FlyStrAllocN(const char *sz, size_t len)
{
  char *psz;

  if(len == SIZE_MAX)
    --len;

  psz = FlyAlloc(len + 1);
  if(psz)
  {
    strncpy(psz, sz, len);
    psz[len] = '\0';
  }

  return psz;
}

/*!------------------------------------------------------------------------------------------------
  Free the (assumed to be allocated) string. OK if sz is NULL. Returns NULL.

  TODO: Remove FlyStrFree() as FlyFree() is sufficient.

  @param   sz     NULL or allocated string
  @return  NULL
*///-----------------------------------------------------------------------------------------------
char * FlyStrFree(char *sz)
{
  if(sz)
    FlyFree(sz);
  return NULL;
}

/*!------------------------------------------------------------------------------------------------
  Appends a substring to the allocated string. Returned ptr is always different that szAllocStr.

  Note: uses realloc() under the hood.

  @param   szAllocStr   ptr to allocated string
  @param   sz           ptr to a substring, which may or may not be '\0' terminated
  @param   len          length of substring to append
  @return  NULL if szAllocStr not on the heap, or heap is out of room, otherwise new ptr
*///-----------------------------------------------------------------------------------------------
char * FlyStrAllocAppend(char *szAllocStr, const char *sz, size_t len)
{
  char     *psz = szAllocStr;
  size_t    newLen = strlen(szAllocStr) + len + 1;

  if(len)
  {
    psz = realloc(szAllocStr, newLen);
    if(psz)
      strncat(psz, sz, newLen);
  }

  return psz;
}

/*!------------------------------------------------------------------------------------------------
  Determines if this line contains a function definition. Returns prototype length and fn CName.

  Note: the prototype might span multiple lines. Also returns CName of function in ppszCName.

  See also FlyStrCNameLen().

  This follows some very basic rules:

  * C:          type        CName( ... ) {
  * C++:        type        CName::CName( ... ) {
  * C++ class:              CName( ... ) {
  * Go:         func        CName( ... ) type {
  * Java:       public type CName( ...) {
  * Javascript: function    CName( ... ) {
  * Python:     def         CName( ... ):
  * Rust:       fn          CName( ... ) -> type {
  * Rust:       pub fn      CName( ... ) -> type {
  * Swift:      func        CName( ... ) -> type {

  @param    szLine      ptr to beginning of a line or string
  @param    ppszCName   ptr to a string ptr will receive ptr to the C
  @return   Returns length of a prototype string, skipping any preceeding whitespace
*///-----------------------------------------------------------------------------------------------
size_t FlyStrFnProtoLen(const char *szLine, const char **ppszCName)
{
  static const char   szDef[]       = "def";        // python
  static const char   szFn[]        = "fn";         // rust
  static const char   szFnPub[]     = "pub";        // rust   pub fn
  static const char   szFunction[]  = "function";   // javascript
  static const char   szFunc[]      = "func";       // swift, go

  const char         *psz;
  const char         *psz2;
  const char         *pszCName      = NULL;
  const char         *pszLastCName;
  size_t              lenPrototype  = 0;
  size_t              len;
  bool_t              fLookForBrace = FALSE;
  bool_t              fTypeFirst    = FALSE;
  bool_t              fFailed       = FALSE;
  bool_t              fIsPython     = FALSE;

  // any function may be indented
  szLine = psz = FlyStrSkipWhite(szLine);

  // Python
  if(FlyStrArgCmp(szDef, psz) == 0)
  {
    fIsPython = TRUE;
    psz += FlyStrArgLen(psz);
  }

  // Rust fn Func()
  else if(FlyStrArgCmp(szFn, psz) == 0)
  {
    fLookForBrace = TRUE;
    psz += FlyStrArgLen(psz);
  }

  // Rust pub fn Func()
  else if(FlyStrArgCmp(szFnPub, psz) == 0)
  {
    fLookForBrace = TRUE;
    psz = FlyStrArgNext(psz);
    if(FlyStrArgCmp(szFn, psz) != 0)
      fFailed = TRUE;
    else
      psz += FlyStrArgLen(psz);
  }

  // javascript
  else if(FlyStrArgCmp(szFunction, psz) == 0)
  {
    psz += FlyStrArgLen(psz);
  }

  // Swift, Go
  else if(FlyStrArgCmp(szFunc, psz) == 0)
  {
    fLookForBrace = TRUE;
    psz += FlyStrArgLen(psz);
  }

  // C or C++ starts with the return type
  else
    fTypeFirst = TRUE;

  // for most languages, CName follows function keyword
  // Rust example: pub fn new<S: Into<String>>(name: S) -> Person { ... }
  if(!fFailed && !fTypeFirst)
  {
    psz = FlyStrSkipWhite(psz);
    pszCName = psz;
  }

  // typefirst is C/C++/C#/Java, etc... where `CName(` is expected
  if(!fFailed && fTypeFirst)
  {
    pszLastCName = NULL;
    while(*psz)
    {
      if(*psz == '*' || *psz == ' ' || *psz == '\t')
        ++psz;
      else
      {
        len = FlyStrCNameLen(psz);
        if(len == 0)
          break;

        pszLastCName = psz;
        psz += len;
      }
    }

    // we've skipped all CNames. We should now be on the '(' if worked
    if(*psz != '(')
      fFailed = TRUE;
    else
      pszCName = pszLastCName;
  }

  // if all worked, by now we have the CName of the function
  // now, make sure it's actually a function definition
  if(!fFailed)
  {
    // look for opening '(' before '=' or end of line
    while(*psz && *psz != '=' && *psz != '(' && *psz != '\n')
      ++psz;

    if(*psz == '(')
    {
      // using across lines using counting method, e.g. ( ( stuff ( one )))
      unsigned count = 1;
      ++psz;
      while(*psz && count && (*psz != '{' && *psz != ';'))
      {
        if(*psz == '(')
          ++count;
        else if(*psz == ')' && count)
          --count;
        ++psz;
      }
      if(count != 0)      // no closing ')' before ';' or '{'
        fFailed = TRUE; 

      psz2 = psz;

      // include the ':' on python prototype
      if(fIsPython)
      {
        psz2 = FlyStrSkipWhite(psz2);
        if(*psz2 != ':')
          fFailed = TRUE;
        else
          psz = psz2 + 1;
      }

      // found closing ')'. Expect a '{' before a ';'
      else
      {
        while(*psz2 && (*psz2 != ';' && *psz2 != '{'))
          ++psz2;
        if(*psz2 == ';')
          fFailed = TRUE;
      }

      // include everything up to the brace in prototype
      if(fLookForBrace)
      {
        --psz2;
        while(*psz2 == ' ' || *psz2 == '\t')
          --psz2;
        ++psz2;
        psz = psz2;
      }

      if(!fFailed && psz)
        lenPrototype = psz - szLine;
    }
  }

  // return CName
  if(lenPrototype && (ppszCName != NULL))
    *ppszCName = pszCName;

  return lenPrototype;
}

/*!------------------------------------------------------------------------------------------------
  Returns length of CName, or 0 if not a CName

    This         | Returns
    ------------ | -------
    my_c_func(   | 9
    my_c_array[  | 10
    _myfn99      | 7
    uint32_t     | 8
    99hello      | 0
    Cars::model( | 11
    ::1          | 0
    a:b          | 0
    fe80::57     | 0
    %name        | 0

  @param   sz     character to checkl
  @return  Length of CName or 0 if not a CName
*///-----------------------------------------------------------------------------------------------
size_t FlyStrCNameLen(const char *sz)
{
  size_t    lenCName = 0;
  unsigned  nColons  = 0;
  bool_t    fFirst   = TRUE;

  while(*sz == '_' || isalpha(*sz) || isdigit(*sz) || *sz == ':')
  {
    // CName first char must be `_` or alpha
    if(fFirst)
    {
      if(isdigit(*sz) || *sz == ':')
      {
        lenCName = 0;
        break;
      }
      fFirst = FALSE;
    }

    // might be IPv6 address or a C++ class
    if(*sz == ':')
    {
      ++nColons;
      if(nColons > 2)
      {
        lenCName = 0;
        break;
      }
      else if(nColons == 2)
        fFirst = TRUE;
    }

    ++sz;
    ++lenCName;
  }

  if(nColons == 1)
    lenCName = 0;

  return lenCName;
}

/*!-------------------------------------------------------------------------------------------------
  Get the version of C (e.g. C99, C11, C17)
  @return ptr to static string with C version in it
*///------------------------------------------------------------------------------------------------
const char *FlyStrCVer(void)
{
  static char szCVer[4] = "C89";

#ifdef __STDC_VERSION__
  long cver;
  cver = (__STDC_VERSION__ / 100) % 100;
  szCVer[1] = '0' + (cver / 10);
  szCVer[2] = '0' + (cver % 10);
#endif

  return szCVer;
}

/*!-------------------------------------------------------------------------------------------------
  Replace a one (or more) occurrences of a string in a larger string in place.

  Always null terminated. Never allocates memory.

  If size of resulting asciiz string would be too large to fit in szHastack (len >= size), then the
  original string is left unchanged. The returned length can then be used to allocate a larger
  string.

  The example below results in string "I am what I am":

      char    szQuote[] = "I {me} what I {me}."
      FlyStrReplace(szQuote, sizeof(szQuote), "{me}", "am", TRUE, FALSE);

  The example below shows where the resulting string would be too large:

      char   *szQuestion = strdup("To be or not to be");
      size_t  len = FlyStrReplace(szQuestion, strlen(szQuestion) + 1, "be", "think", TRUE, FALSE);

      if(len > strlen(szQuestion))
      {
        size_t size = len + 1;
        szQuesiton = realloc(szQuestion, size);
        if(szQuestion)
          FlyStrReplace(szQuestion, size, "be", "think", TRUE, FALSE);
        // resulting string: "to think or not to think"
      }

  @oaran  szHaystack    larger asciiz string
  @param  size          size of string
  @param  szNeedle      string to find in szHaystack (must have at least 1 char)
  @param  szWith        string to replace it with in szHaystack
  @param  fAll          Replace all or once
  @param  fIgnoreCase   TRUE if search should ignore case
  @return strlen of szHaystack after replacement
*///------------------------------------------------------------------------------------------------
size_t FlyStrReplace(char *szHaystack, size_t size, const char *szNeedle, const char *szWith, flyStrReplaceOpt_t opts)
{
  char     *psz;
  char     *pszPrev;
  size_t    lenNeedle   = strlen(szNeedle);
  size_t    lenWith     = strlen(szWith);
  size_t    len         = 0;
  unsigned  i;
  bool_t    fAll        = opts & 0x01 ? TRUE : FALSE;
  bool_t    fIgnoreCase = opts & 0x02 ? TRUE : FALSE;
  bool_t    fReplace    = FALSE;

  // Go through string twice. Once to determine required len. Once to do the actual replace
  for(i = 0; i < 2; ++i)
  {
    psz = szHaystack;
    len = 0;
    while(psz)
    {
      // find substring, possibly case insensitive
      pszPrev = psz;
      if(fIgnoreCase)
        psz = strcasestr(psz, szNeedle);
      else
        psz = strstr(psz, szNeedle);

      // printf("  i=%u szHaystack `%s`, psz `%s`\n", i, szHaystack, FlyStrNullOk(psz));
      if(psz == NULL)
      {
        len += strlen(pszPrev);

        // printf("  0: len %zu\n", len);

        // not found, nothing to do
        if(pszPrev == szHaystack)
          break;
      }
      else
      {
        // replace substring
        len += (psz - pszPrev);
        if(!fReplace)
          psz += lenNeedle;
        else
        {
          // printf("  1: len %zu, lenNeedle %zu, lenWith %zu, szHaystack `%s`\n", len, lenNeedle, lenWith, szHaystack);
          memmove(&psz[lenWith], &psz[lenNeedle], strlen(&psz[lenNeedle]) + 1);
          if(lenWith)
            memcpy(psz, szWith, lenWith);
          psz += lenWith;
          // printf("  2: len %zu, lenNeedle %zu, lenWith %zu, szHaystack `%s`\n", len, lenNeedle, lenWith, szHaystack);
        }
        len += lenWith;
      }

      // only replacing 1 item
      if(psz && !fAll)
      {
        len += strlen(psz);
        break;
      }
    }


    // too large to fit
    if(len >= size)
      break;

    fReplace = TRUE;
  }

  // printf("done: len %zu, szHaystack `%s`\n", len, szHaystack);
  return len;
}

/*!-------------------------------------------------------------------------------------------------
  Count the number of needles (substrings) in the haystack (string).

  @param    szHaystack  The larger string
  @param    szNeedle    The string to count inside of the haystack
  @return   count of needles in haystack
*///------------------------------------------------------------------------------------------------
size_t FlyStrCount(const char *szHaystack, const char *szNeedle)
{
  size_t      count = 0;
  const char *psz   = szHaystack;

  while(psz)
  {
    psz = strstr(psz, szNeedle);
    if(!psz)
      break;
    ++count;
    psz += strlen(szNeedle);
  }

  return count;
}

const char g_szFlyStrSlugChars[] = "-._~";

/*!-------------------------------------------------------------------------------------------------
  Returns TRUE if this character is a slug character (alnum, utf8 or "-._~").

  @param    c   character
  @return   TRUE or FALSE
*///------------------------------------------------------------------------------------------------
bool_t FlyCharIsSlug(char c)
{
  return (isalnum(c) || ((uint8_t)c > 0x80) || (strchr(g_szFlyStrSlugChars, c) != NULL)) ? TRUE : FALSE;
}

/*!-------------------------------------------------------------------------------------------------
  Similar to strcpy(), but converts UTF-8 string to a case sensitive slug (URI friendly string).

  1. Only alnum and characters "-._~" are coped. All else is considered whitespace.
  2. UTF-8 characters (0x80 and above) are converted to percent nottation e.g. `%f0%9f%94%a5`
  3. Whitespace and non-slug-friendly characters from start and end are removed
  4. Whitespace and non-slug-friendly characters in the middle are converted to a single dash
  4. End of line is treated as end of string (e.g. `\r` or `\n`)

  Examples:

  UTF-8 String             | Becomes Slug
  ------------------------ | ------------
  "  I Love   Waffles  "   | "I-Love-Waffles"
  "my.echo My Shadow & Me" | "my.echo-My-Shadow-Me"
  "-._~"                   | "-._~"
  "a - b . c _ d ~ e"      | "a-b.c_d~e"
  "🔥 🐈 😊 木"           | "%f0%9f%94%a5-%f0%9f%90%88-%f0%9f%98%8a-%e6%9c%a8"

  A hex dump of that final UTF-8 string with fire, cat, smile emoji and Japanese tree is below:

      00000000  f0 9f 94 a5 20 f0 9f 90  88 20 f0 9f 98 8a 20 e6  |.... .... .... .|
      00000010  9c a8                                             |..|
      00000012

  Set szSlug buffer to NULL to just get length of slug (for allocating memory for the slug).

  Resulting string will only contain Unreserved Characters from Section 2.3 of RFC 3986.

  It's a VERY bad idea to have URI's longer than 2000 characters. This function limits the slug
  length to UINT_MAX - 1.

  See: <https://www.rfc-editor.org/rfc/rfc3986>  
  See also: <https://webmasters.stackexchange.com/questions/90339/why-are-urls-case-sensitive>  

  @param    szSlug  destination buffer for slug, may be NULL to just get length
  @param    szSrc   source string to convert to a slug string
  @param    size    1-n, size of destination buffer szSlug
  @param    srcLen  maximum length of szSrc to process, can be 0 to just use strlen()
  @return   length of slug in bytes
*///------------------------------------------------------------------------------------------------
unsigned FlyStrSlug(char *szSlug, const char *szSrc, size_t size, size_t srcLen)
{
  // char       *szSlugOrg  = szSlug;
  unsigned    len        = 0;
  bool_t      fExit      = FALSE;
  char        whitespace = 0;
  uint8_t     utf8;

// printf("FlyStrSlug(szSrc=%s, srcLen %zu\n", szSrc, srcLen);

  if(szSrc && srcLen == 0)
    srcLen = FlyStrLineLen(szSrc);

  // skip preceding non-slug characters (whitespace). UTF-8 (0x80+) is not whitespace
  while(srcLen && *szSrc && !FlyCharIsEol(*szSrc) && !FlyCharIsSlug(*szSrc))
  {
    --srcLen;
    ++szSrc;
  }

  // limit to size of unsigned with room for '\0'
  if(size >= UINT_MAX)
    size = UINT_MAX - 1;

  // copy slug string from ASCIIZ string
  while(srcLen && *szSrc && !FlyCharIsEol(*szSrc) && size > 1)
  {
    // copy all slug characters
    while(srcLen && *szSrc && FlyCharIsSlug(*szSrc))
    {
      // handle UTF characters
      utf8 = (uint8_t)(*szSrc);
      if(utf8 >= 0x80)
      {
        if(size <= 4)
        {
          fExit = TRUE;
          break;
        }
        if(szSlug)
        {
          szSlug[0] = '%';
          szSlug[1] = FlyCharHexDigit(utf8 >> 4);
          szSlug[2] = FlyCharHexDigit(utf8 & 0xf);
          szSlug += 3;
        }
        len += 3;
        size -= 3;
        ++szSrc;
        --srcLen;
      }

      // handle ASCII characters
      else
      {
        if(szSlug)
          *szSlug++ = *szSrc;
        ++szSrc;
        --srcLen;
        ++len;
        --size;
      }
      if(size <= 1)
      {
        fExit = TRUE;
        break;
      }
    }
    if(fExit)
      break;

    // skip trailing non-slug characters (whitespace)
    // special case " - " is considered whitespace, allows for sections like:
    // 4.8 - Some Section in Document
    whitespace = 0;
    if(srcLen >= 3 && isblank(*szSrc) && strchr(g_szFlyStrSlugChars, szSrc[1]) && isblank(szSrc[2]))
    {
      whitespace = szSrc[1];
      szSrc += 3;
      srcLen -= 3;
    }
    while(srcLen && *szSrc && !FlyCharIsEol(*szSrc) && !FlyCharIsSlug(*szSrc))
    {
      whitespace = '-';
      ++szSrc;
      --srcLen;
    }

    // put a single dash to replace any number of in-between whitespace and non-slug chars
    if(whitespace && *szSrc && !FlyCharIsEol(*szSrc))
    {
      if(szSlug)
        *szSlug++ = whitespace;
      ++len;
      --size;
      if(size <= 1)
        break;
    }
  }

  if(szSlug)
    *szSlug = '\0';

// if(szSlugOrg)
//   printf("  szSlug  %s\n", szSlugOrg);

  return len;
}

/*!-------------------------------------------------------------------------------------------------
  Ask a question, get an answer. Uses fgets() and printf() with stdin and stdout.

  @param    szAnswer      ptr to string for answer
  @param    szQuestion    question to ask
  @param    size          sizeof(szAnswer) 2-n
  @return   ptr to szAnswer
*///------------------------------------------------------------------------------------------------
char * FlyStrAsk(char *szAnswer, const char *szQuestion, size_t size)
{
  char *p;

  printf("%s ", szQuestion);
  fflush(stdout);
  fgets(szAnswer, size, stdin);
  szAnswer[size - 1] = '\0';
  p = strchr(szAnswer, '\r');
  if(p)
    *p = '\0';
  p = strchr(szAnswer, '\n');
  if(p)
    *p = '\0';

  return szAnswer;
}

/*!------------------------------------------------------------------------------------------------
  Find the end of a quoted string with possible C escapes.

  Stops at end of line or string.

  For example, will find the closing quote before xyz if given the opening quote after abc:

      "abc \"hello \\\"world!\\\"\" xyz"

  @param    sz     Pointer to the opening quote of a string
  @return   Return ptr to closing quote, or end of line/string
*///-----------------------------------------------------------------------------------------------
char * FlyStrEscEndQuoted(const char *sz)
{
  uint8_t byte;

  if(*sz == '"')
  {
    ++sz;
    while(*sz && !FlyCharIsEol(*sz))
    {
      if(*sz == '"')
        break;
      sz = FlyCharEsc(sz, &byte);
    }
  }

  return (char *)sz;
}

/*!-------------------------------------------------------------------------------------------------
  Same as strncpy(), but will convert escaped characters to their appropriate bytes.

  Esc | Byte | Description
  --- | ---- | -----------
  \\  | 0x5c | backslash
  \"  | 0x22 | quote
  \a  | 0x07 | audible bell
  \b  | 0x08 | backspace
  \f  | 0x0c | form feed
  \n  | 0x0a | line feed
  \r  | 0x0d | carriage return
  \t  | 0x08 | horizontal tab
  \v  | 0x0b | vertical tab
  \x  | \xnn | hexidecimal sequence, e.g. /xf3
  \0  | ooo  | octal sequence, where /0 is nul byte 0x00 and /377 is 0xff

  Warning: if there is no terminating `\0` byte in the 1st n characters of szSrc, then szDst will
  not be `\0` terminated.

  Note: return value is up to 1-n (total bytes copied), not a string pointer.

  Optional return valuye pNBytes returns number of bytes actually copied, not including possible
  terminating '\0'.

  @param    szDst     a buffer to hold escaped string or NULL to just get length
  @param    szSrc     a string with potential escapes
  @param    n         number of bytes to copy
  @param    pNBytes   returned # of bytes actually copied to szDst, may be NULL if don't care
  @return   number of bytes actually copied (1-n)
*///------------------------------------------------------------------------------------------------
char * FlyStrEscNCpy(char *szDst, const char *szSrc, size_t n, size_t *pNBytes)
{
  size_t    nCopied = 0;
  bool_t    fGotEsc;
  uint8_t   c;

  while(n)
  {
    fGotEsc = FALSE;
    if(n > 1 && *szSrc == '\\')
    {
      // handles \007, \xfe, \", etc...
      szSrc = FlyCharEsc(szSrc, &c);
      fGotEsc = TRUE;
    }
    else
      c = *szSrc;

    // will copy '\0' if within the 1st n bytes of szSrc
    if(szDst)
      *(uint8_t *)szDst++ = c;
    if(c == 0)
      break;

    --n;
    ++nCopied;
    if(!fGotEsc)
      ++szSrc;
  }

  // return values
  if(pNBytes)
    *pNBytes = nCopied;
  return (char *)szSrc;
}

/*!-------------------------------------------------------------------------------------------------
  Reverse characters in a string.

  @param    sz    an ASCIIZ string
  @return   none
*///------------------------------------------------------------------------------------------------
void FlyStrRev(char *sz)
{
  char   *szEnd;
  size_t  len;
  char    c;

  len = strlen(sz);
  if(len > 1)
  {
    szEnd = sz + (len - 1);
    while(szEnd > sz)
    {
      c = *sz;
      *sz = *szEnd;
      *szEnd = c;
      --szEnd;
      ++sz;
    }
  }
}


// used by FlyStrLToStr(), FlyStrNTol()
static const char m_aszDigits[]    = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char m_aszDozDigits[] = "XE";  // alternative dozenal digits

/*!-------------------------------------------------------------------------------------------------
  Convert from a long into a string of digits.

  The base is limited to 36 because the higher digits use alphabetic characters A-Z. However, for
  dozenal (base 12), the digits X,E (Dec, El) will be used. Base of 0 is assumed to be 10.

  @param    szDst     pointer buffer or NULL if just getting # of digits
  @param    n         long number to convert
  @param    size      size of szDst buffer, or UINT_MAX
  @param    base      2-36
  @return   number of digits (1-n)
*///------------------------------------------------------------------------------------------------
unsigned FlyStrLToStr(char *szDst, long n, size_t size, int base)
{
  char     *szNum   = szDst;
  unsigned  nDigits = 0;
  unsigned  digit;

  // deal with invalid base
  if(base < 2 || base > 36)
    base = 10;

  // handle negative numbers
  if(n < 0)
  {
    ++nDigits;
    if(size > 1 && szDst)
    {
      *szDst++ = '-';
      --size;
    }

    // special case for LONG_MIN because we can't represent it in a positive number
    if(n == LONG_MIN)
    {
      ++nDigits;
      if(size > 1 && szDst)
      {
        digit = (unsigned)(-1L * (n % base));
        if(base == 12 && digit >= 10)
          *szDst++ = m_aszDozDigits[digit - 10];
        else
          *szDst++ = m_aszDigits[digit];
        --size;
      }
      n = n / base;
    }
    n = -1L * n;
  }

  // handle 0
  if(n == 0)
  {
    ++nDigits;
    if(size > 1 && szDst)
    {
      *szDst++ = '0';
      --size;
    }
  }

  // convert long to string
  while(n)
  {
    ++nDigits;
    if(size > 1 && szDst)
    {
      digit = (unsigned)(n % base);
      if(base == 12 && digit >= 10)
        *szDst++ = m_aszDozDigits[digit - 10];
      else
        *szDst++ = m_aszDigits[digit];
      --size;
    }
    n = n / base;
  }

  if(size && szDst)
  {
    *szDst = '\0';
    if(*szNum == '-')
      ++szNum;
    FlyStrRev(szNum);
  }

  return nDigits;
}

/*!-------------------------------------------------------------------------------------------------
  Convert a numeric string, of some base, to a long integer.

  Similar to strtol() but contains more parameters and more features:

  1. Ignore certain characters, allows "1,234" or "1_234" vs "1234"
  2. Lenth allows termination after a specific # of digits
  3. Returns number of digits found AND character after number
  4. Only base 0 autodetects base 2, 8, 12, 16
  5. With dozenal (base 12), use dek, el (X, E) or (a, b) for extra digits
  6. Does NOT affect errno

  The base is limited to 36 because the higher digits use alphabetic characters A-Z. For dozenal
  (base 12), the digits A,B or X,E (Dek, El) can be used.

  If number begins with `0b`, `0#`, `0z` or `0x`, then it will autodetect the base using the 1st
  character(s):

  Number        | Base
  ------------- | -------- 
  0b10111       | binary (2)
  0733          | octal (8)
  0z3x2e9       | dozenal (12)
  0xfe29        | hex (16)
  anything else | decimal (10)

  Case of digits is ignored: that is, the hex number e2f is same as E2F.

  szIgnore, if not NULL, will cause those characters to be ignored. Example, szIgnore = ",", then
  the number "1,234,567" will be processed, otherwise it will stop at the "1".

  Note: `(unsigned)(szEnd - szNum)` and `*pDigits` (returned # of digits found) may be different if
  szIgnore is not NULL.

  @param    szNum     Pointer to a string with 0 or more "numbers"
  @param    szEnd     If not NULL, returns ptr character after the last processed
  @param    base      2-36
  @param    len       maximum # of bytes to parse
  @param    pDigits   return value, # of digits found (0-len)
  @param    szIgnore  ignore 
  @return   TRUE if c is a slash
*///------------------------------------------------------------------------------------------------
long FlyStrNToL(const char *szNum, char **ppszEnd, int base,
                unsigned len, unsigned *pDigits, const char *szIgnore)
{
  const char   *pszDigit;
  long          val           = 0L;
  unsigned      nDigits       = 0;
  bool_t        fNegative     = FALSE;
  bool_t        fDigitFound;
  unsigned      digit;
  char          c;

  // deal with preceeding + or -
  if(len && *szNum == '-')
  {
    fNegative = TRUE;
    ++nDigits;
    ++szNum;
    --len;
  }
  else if(len && *szNum == '+')
  {
    ++szNum;
    --len;
  }

  // autodetect base if requested
  if((len >= 2) && (*szNum == '0'))
  {
    if((base == 0 || base == 8) && isdigit(szNum[1]) && ((szNum[1] - '0') <= 7))
    {
      base = 8;
      ++szNum;
      --len;
    }
    else
    {
      switch(szNum[1])
      {
        case 'x': base = 16; szNum += 2; len -= 2; break;
        case 'z': base = 12; szNum += 2; len -= 2; break;
        case 'b': base = 2;  szNum += 2; len -= 2; break;
        default: break;
      }
    }
  }
  if(base < 2 || base >= sizeof(m_aszDigits))
    base = 10;

  while(*szNum && len)
  {
    if((szIgnore == NULL) || strchr(szIgnore, *szNum) == NULL)
    {
      fDigitFound = FALSE;
      c = toupper(*szNum);
      if(base == 12 && (c == m_aszDozDigits[0] || c == m_aszDozDigits[1]))
      {
        val = (val * base) + ((c == m_aszDozDigits[0]) ? 10 : 11);
        fDigitFound = TRUE;
      }
      if(!fDigitFound)
      {
        pszDigit = strchr(m_aszDigits, c);
        if(pszDigit != NULL)
        {
          digit = (unsigned)(pszDigit - m_aszDigits);
          if(digit >= base)
          {
            printf("digit %u, c %c, m_aszDigits %p, pszDigit %p\n", digit, c, m_aszDigits, pszDigit);
          }
          if(digit < base)
          {
            val = (val * base) + digit;
            fDigitFound = TRUE;
          }
        }
      }
      if(!fDigitFound)
        break;
      else
        ++nDigits;
    }
    ++szNum;
    --len;
  }

  // return values
  if(fNegative)
    val = -val;
  if(pDigits)
    *pDigits = nDigits;
  if(ppszEnd)
    *ppszEnd = (char *)szNum;

  return val;
}
