/*!************************************************************************************************
  flyfile2c.c  - converts a text or source file to a .c string  
  Copyright 2023 Drew Gislason  
  License: MIT <>
*///***********************************************************************************************
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>       // for uint32_t, uint8_t, etc..
#include <stdlib.h>
#include <string.h>

typedef unsigned bool_t;
#define TRUE 1
#define FALSE 0

typedef enum
{
  FILE2C_TYPE_TEXT = 0,   // ASCII
  FILE2C_TYPE_UTF8,       // text with UTF-8 in it
  FILE2C_TYPE_BINARY,     // neither ASCII nor UTF-8, some other kind of file
} file2c_type;

// prototypes
uint8_t  *file2c_file_read(const char *szInFile, long *pLen, file2c_type *pType);
FILE     *file2c_file_write_hdr(const char *szInFile, const char *szOutFile);
int       file2c_file_write_text(FILE *fp, const char *szVar, const uint8_t *pFile, long len, file2c_type type);
int       file2c_file_write_bin(FILE *fp, const char *szVar, const uint8_t *pFile, long len);

/*!
  @defgroup flyfile2c Converts a text file (including UTF-8) or a binary file to a C string

  For all text files, automatically adds newlines (\r\n on windows, \n on Linux/MacOS) and
  escapes "quotes" and other characters so C string is correct.

  1. Text and UTF-8 files have escapes and quotes properly converted.
  2. UTF-8 string lines get u8 in front of the string literals
  3. Binary files get converted to bytes with hexdump -C style comment after each 16-byte line

  ```
  Usage = flyfile2c <infile> <outfile> <varname>
  ```
*/
int main(int argc, const char *argv[])
{
  static const char *aszTypeStr[] = { "text", "utf-8", "binary" };

  uint8_t      *pFile;    // input file contents 
  FILE         *fpOut   = NULL;
  file2c_type   type    = FILE2C_TYPE_TEXT;
  long          len     = 0;
  int           ret     = 0;    // return value, will be 0 if worked, 1 if error

  // display help
  if(argc < 4)
  {
    printf("flyfile2c v1.0\n");
    printf("Converts a text file (including UTF-8) or a binary file to a C string.\n");
    printf("\nUsage = flyfile2c <infile> <outfile> <varname>\n");
    ret = 1;
  }

  // read the file into memory, along with length and type
  if(ret == 0)
  {
    pFile = file2c_file_read(argv[1], &len, &type);
    if(!pFile)
      ret = 1;
    else if(len == 0)
    {
      printf("\nErr: input file %s is empty, nothing to do!\n", argv[1]);
      ret = 1;
    }
  }

  // create output file with header
  if(ret == 0)
  {
    fpOut = file2c_file_write_hdr(argv[1], argv[2]);
    if(!fpOut)
      ret = 1;
  }

  // write the apropriate type of file
  if(ret == 0)
  {
    if(type == FILE2C_TYPE_TEXT || type == FILE2C_TYPE_UTF8)
      ret = file2c_file_write_text(fpOut, argv[3], pFile, len, type);
    else
      ret = file2c_file_write_bin(fpOut, argv[3], pFile, len);
    if(ret != 0)
      printf("\nErr: Can't write to file %s\n", argv[2]);
    else
      printf("Created file '%s' of type %s\n", argv[2], aszTypeStr[type]);
  }

  // done with output file
  if(fpOut)
    fclose(fpOut);

  return ret;
}

/*!
  What type of file is this?

  @param  pFile   ptr to in-memorty file
  @param  len     length of in-memory file
  @return type of file: FILE2C_TYPE_BINARY, FILE2C_TYPE_TEXT, FILE2C_TYPE_UTF8
*/
file2c_type file2c_file_type(uint8_t *pFile, long len)
{
  file2c_type type;

  // assume ASACII text unless proven otherwise
  type = FILE2C_TYPE_TEXT;
  while(len)
  {
    // only binary files contain the byte 0x00
    if(*pFile == 0x00)
    {
      type = FILE2C_TYPE_BINARY;
      break;
    }

    // UTF-8 encoding is 1 to 4 bytes:
    //
    // 00000000 -- 0000007F:   0xxxxxxx (7 bits)
    // 00000080 -- 000007FF:   110xxxxx 10xxxxxx (11 bits)
    // 00000800 -- 0000FFFF:   1110xxxx 10xxxxxx 10xxxxxx (16 bits)
    // 00010000 -- 0010FFFF:   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx (21 bits)
    if(*pFile & 0x80)
    {
      // 2 byte char
      if(len >= 2 && ((*pFile & 0xe0) == 0xc0) && ((pFile[1] & 0xc0) == 0x80))
      {
        type = FILE2C_TYPE_UTF8;
        ++pFile;
        --len;
      }
      else if(len >= 3 && ((*pFile & 0xf0) == 0xe0) && ((pFile[1] & 0xc0) == 0x80) && ((pFile[2] & 0xc0) == 0x80))
      {
        type = FILE2C_TYPE_UTF8;
        pFile += 2;
        len -= 2;
      }
      else if(len >= 4 && ((*pFile & 0xf8) == 0xf0) && ((pFile[1] & 0xc0) == 0x80) && ((pFile[2] & 0xc0) == 0x80) && ((pFile[2] & 0xc0) == 0x80))
      {
        type = FILE2C_TYPE_UTF8;
        pFile += 3;
        len -= 3;
      }
      else
      {
        type = FILE2C_TYPE_BINARY;
        break;
      }
    }

    // look at next byte
    ++pFile;
    --len;
  }

  return type;
}

/*!
  Read the entire file into memory

  @param  szFilename  ptr to a path and filename, e.g. "file.c" or "../folder/myfile.bin"
  @param  pLen        return value, length of file in bytes
  @param  pType       type of file, e.g. FILE2C_TYPE_TEXT or FILE2C_TYPE_BINARY
  Returns ptr to the file, or NLLL if not enough memory or file doesn't exist.
*/
uint8_t * file2c_file_read(const char *szInFile, long *pLen, file2c_type *pType)
{
  FILE         *fp;
  uint8_t      *pFile   = NULL;
  file2c_type   type    = FILE2C_TYPE_BINARY;
  long          len     = 0;

  // read the input file
  fp = fopen(szInFile, "rb");
  if(!fp)
    printf("\nErr: Can't open file '%s'\n", szInFile);

  // determine size of file
  if(fp)
  {
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    rewind(fp);
    pFile = malloc(len + 1);
    if(!pFile)
    {
      printf("\nErr: Can't allocate %ld bytes of memory\n", len);
      fclose(fp);
    }
  }

  if(pFile)
  {
    if(fread(pFile, 1, len, fp) != len)
    {
      printf("\nErr: Can't read %ld bytes in file '%s'\n", len, szInFile);
      free(pFile);
      pFile = NULL;
    }
    else
    {
      // null terminate in case of text file
      pFile[len] = '\0';
    }
    fclose(fp);
  }

  // determine file type
  if(pFile)
  {
    // ASACII text allows bytes 0x01 - 0x7f only
    type = file2c_file_type(pFile, len);
  }

  // return ptr to file and extra values
  if(pLen)
    *pLen = len;
  if(pType)
    *pType = type;
  return pFile;
}

/*!
  Create the output file and Write the autogenerated header

  @param  szFilename    file to create, e.g. "../outfile.c"
  Returns FILE * to open text file
*/
FILE * file2c_file_write_hdr(const char *szInFile, const char *szOutFile)
{
  FILE * fp;

  fp = fopen(szOutFile, "w");
  if(!fp)
    printf("\nErr: Can't create file %s\n", szOutFile);
  else
  {
    if(fprintf(fp, "// AUTOGENERATED, DO NOT CHANGE\n// converted file '%s' to '%s'\n\n",
               szInFile, szOutFile) <= 0)
    {
      printf("\nErr: Can't write to file '%s'\n", szOutFile);
      fclose(fp);
      fp = NULL;
    }
  }

  return fp;
}

/*!
  Write the text file in C character array form.

  Assumes pFile ends with a '\0' byte.

  @param  fp      FILE * to output file, already created with header
  @param  szVar   C variable name to write
  @param  pFile   input binary file
  @param  len     length of binary file
  @param  type    e.g. FILE2C_TYPE_TEXT
  @return         0 if worked, 1 if failed
*/
int file2c_file_write_text(FILE *fpOut, const char *szVar, const uint8_t *pFile, long len, file2c_type type)
{
  const uint8_t  *psz;
  const char     *szPrefix;
  char           *pszOut;
  char           *pszLine;
  long            escCount          = 0;   // count # of \ characters needed
  int             fLastLine         = FALSE;
  int             fLastLineNewline;
  int             ret               = 0;

  // prefix UTF-8 with u8
  if(type == FILE2C_TYPE_UTF8)
    szPrefix = "u8";
  else
    szPrefix = "";

  // count number of escape \ characters needed to add for quotes and other escape characters
  psz = pFile;
  while(*psz)
  {
    if(*psz == '\\')
      ++escCount;
    if(*psz == '"')
      ++escCount;
    ++psz;
  }

  // write variable name
  if(fprintf(fpOut, "const char %s[] =\n", szVar) <= 0)
  {
    ret = 1;
  }

  // allocate space for file
  if(ret == 0)
  {
    pszOut = malloc(len + escCount + 1);
    if(!pszOut)
    {
      printf("\nErr: Can't allocate %ld bytes of memory\n", len + escCount + 1);
      ret = 0;
    }
    else
      memset(pszOut, 0, len + escCount + 1);
  }

  // write out file as lines
  if(ret == 0)
  {
    psz = pFile;
    while(*psz)
    {
      // make a NUL terminated string out the line
      pszLine = pszOut;
      while(*psz && (*psz != '\n'))
      {
        if((*psz == '\\') || (*psz == '"'))
          *pszOut++ = '\\';
        else if(*psz != '\r')
          *pszOut++ = (char)(*psz);
        ++psz;
      }
      *pszOut = '\0';

      // last line, break
      if((*psz == '\0') || ((*psz == '\n') && (psz[1]=='\0')))
      {
        if(*psz == '\0')
          fLastLineNewline = FALSE;
        else
          fLastLineNewline = TRUE;
        fLastLine = TRUE;
      }

      // /printf("fLastLine %i, pszBuf %p, psz %p, %c, pszLine '%s'\n", fLastLine, pszBuf, psz, *psz ? *psz : '~', pszLine);

      // lastline will have a ';' to terminate the string
      if(fLastLine)
        ret = fprintf(fpOut, "  %s\"%s%s\";\n", szPrefix, pszLine, fLastLineNewline ? "\\n" : "");
      else
        ret = fprintf(fpOut, "  %s\"%s\\n\"\n", szPrefix, pszLine);
      szPrefix = "";

      if(ret <= 0)
      {
        ret = 1;
        break;
      }

      if(fLastLine)
      {
        ret = 0;
        break;
      }

      // on to next line
      ++psz;
      ++pszOut;
    }
  }

  return ret;
}

/*!
  Write the binary file in C byte array format

  Example:

      uint8_t aBinary[] = 
      {
        0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, // 00000000  |.PNG....|
        0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, // 00000008  |....IHDR|
        //...
      };

  @param  fp      FILE * to output file, already created with header
  @param  szVar   C variable name to write
  @param  pFile   input binary file
  @param  len     length of binary file
  @return         0 if worked, 1 if failed
*/
int file2c_file_write_bin(FILE *fpOut, const char *szVar, const uint8_t *pFile, long len)
{
  #define LINELEN 8
  long      i;
  long      offset;
  unsigned  j;
  unsigned  thisLen;
  char      c;
  int       ret       = 0;    // 0 = ok, 1 = err

  if(fprintf(fpOut, "#include <stdint.h>\n\nuint8_t %s[] =\n{\n", szVar) < 0)
    ret = 1;

  if(ret == 0)
  {
    i = 0;
    while(ret == 0 && i < len)
    {
      // indent 2 spaces each line
      if(fprintf(fpOut, "  ") <= 0)
        ret = 1;

      // offset for comment at end
      offset = i;
      thisLen = LINELEN;
      if(len - i < LINELEN)
        thisLen = (unsigned)(len - i);
      for(j = 0; ret == 0 && j < thisLen; ++j)
      {
        if(fprintf(fpOut, "0x%02x%s", pFile[i], i + 1 >= len ? "  " : ", ") <= 0)
        {
          ret = 1;
          break;
        }
        ++i;
      }

      // short line at end
      if(ret == 0 && thisLen < LINELEN)
      {
        for(j = thisLen; ret == 0 && j < LINELEN; ++j)
        {
          if(fprintf(fpOut, "      ") <= 0)
            ret = 1;
        }
      }

      // offset comment at end
      if(ret == 0 && fprintf(fpOut, "  // %08lX  |", offset) <= 0)
        ret = 1;
      else if(ret == 0)
      {
        for(j = 0; j < LINELEN; ++j)
        {
          if(j < thisLen)
          {
            c = (char)(pFile[offset + j]);
            if(c < ' ' || c > '~')
              c = '.';
          }
          else
            c = ' ';
          if(fprintf(fpOut, "%c", c) <= 0)
          {
            ret = 1;
            break;
          }
        }
      }
      if(ret == 0 && fprintf(fpOut, "|\n") <= 0)
        ret = 1;
    }
  }

  if(ret == 0)
  {
    if(fprintf(fpOut, "};\nlong %s_size = sizeof(%s);\n",szVar,szVar) < 0)
      ret = 1;
  }

  return ret;
}
