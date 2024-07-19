/**************************************************************************************************
  example_base64.c  An example of using FlyBase64, converts binary files or text to base64
  Copyright 2024 Drew Gislason
  license: <https://mit-license.org>
**************************************************************************************************/
#include "FlyBase64.h"
#include "FlyCli.h"
#include "FlyFile.h"
#include "FlyStr.h"

void base64_print(const char *sz_base64, unsigned width);
char * combine_cmdline(const flyCli_t *p_cli, size_t *p_len);

int main(int argc, const char *argv[])
{
  char           *sz_base64 = NULL;
  uint8_t        *bin_data  = 0;
  size_t          bin_len;
  size_t          size;
  char           *sz_file   = NULL;
  bool_t          verbose   = TRUE;

  const flyCliOpt_t cliOpts[] =
  {
    { "-f", &sz_file, FLYCLI_STRING },
    { "-v", &verbose, FLYCLI_BOOL }
  };
  const flyCli_t cli =
  {
    .pArgc      = &argc,
    .argv       = argv,
    .nOpts      = NumElements(cliOpts),
    .pOpts      = cliOpts,
    .szVersion  = "base64 v1.0",
    .szHelp     = "Usage = base64 [-v] [-f binary_file] text...\n"
                  "\n"
                  "Options:\n"
                  "-f      specify a file to convert to base64\n"
                  "-v-     turn off verbose (silent). Verbose is on by default\n"
  };

  if(FlyCliParse(&cli) != FLYCLI_ERR_NONE)
    exit(1);
  if(sz_file == NULL && FlyCliNumArgs(&cli) < 2)
  {
    printf("nothing to do. Try base64 --help\n");
    exit(1);
  }

  // either use specified file or default binary data
  if(verbose)
    printf("base64 v1.0\n\n");
  if(sz_file != NULL)
  {
    long len;
    bin_data = FlyFileReadBin(sz_file, &len);
    bin_len = len;
  }
  else
    bin_data = (uint8_t *)combine_cmdline(&cli, &bin_len);

  // invalid file
  if(!bin_data || bin_len == 0)
  {
    printf("invalid file %s\n", sz_file);
    exit(1);
  }

  if(verbose)
  {
    printf("Encoding %s%s, len %zu...\n", sz_file ? "file " : "",
            sz_file ? sz_file : (char *)bin_data, bin_len);
    if(sz_file)
      FlyStrDump(bin_data, bin_len);
    printf("\n");
  }

  size = FlyBase64Encode(NULL, SIZE_MAX, bin_data, bin_len);
  if(size)
    sz_base64 = FlyAlloc(size);
  if(sz_base64 == NULL)
  {
    printf("Internal error: memory\n");
    exit(1);
  }
  else
    FlyBase64Encode(sz_base64, size, bin_data, bin_len);

  base64_print(sz_base64, 80);

  return 0;
}

/*
  Print base64 with line breaks at 80 columns to make it readable

  @param  sz_base64   Base64 encoded data
  @rturn  none
*/
void base64_print(const char *sz_base64, unsigned width)
{
  size_t    len;
  size_t    i;
  unsigned  this_width;

  len = strlen(sz_base64);
  for(i = 0; i < len; i += width)
  {
    this_width = width;
    if(len - i < width)
      this_width = (unsigned)(len - i);
    printf("%.*s\n", width, sz_base64);
    sz_base64 += this_width;
  }
}

/*
  Combine command line into a single string. Return size and string.

  @param  p_cli     pointer to command-line structure
  @param  p_size    returned pointer to size including terminating '\0'
  @rturn  pointer to combined ASCIIZ string
*/
char * combine_cmdline(const flyCli_t *p_cli, size_t *p_size)
{
  static const char sz_empty[]  = "";
  size_t  size                  = 0;
  int     i;
  char   *sz;

  // allocate string
  for(i = 1; i < FlyCliNumArgs(p_cli); ++i)
    size += strlen(FlyCliArg(p_cli, i)) + 1;
  sz = FlyAlloc(size);
  if(sz == NULL)
  {
    sz = (char *)sz_empty;
    *p_size = sizeof(sz_empty);
  }
  else
  {
    for(i = 1; i < FlyCliNumArgs(p_cli); ++i)
    {
      if(i == 1)
        strcpy(sz, FlyCliArg(p_cli, i));
      else
      {
        strcat(sz, " ");
        strcat(sz, FlyCliArg(p_cli, i));
      }
    }
    *p_size = size;
  }

  return sz;
}
