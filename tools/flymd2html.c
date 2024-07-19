/**************************************************************************************************
  flymd2html.c - A tool that converts a single markdown file to a single html file
  Copyright 2024 Drew Gislason
  license: <https://mit-license.org>
**************************************************************************************************/
#include "FlyFile.h"
#include "FlyMarkdown.h"
#include "FlyMem.h"
#include "FlyStr.h"
#include "FlyCli.h"

extern bool_t fFlyMarkdownDebug;

static const char m_szVersion[] = "flymd2html v" FLY_VER;
static const char m_szHelp[] = 
  "Usage = flymd2html [-v] [-o out] in...\n"
  "\n"
  "-o       output file (if 1 input) or folder/ (2+ inputs)\n"
  "-v       verbose\n"
  "in       input file(s)s\n";

/*
  Write the file to the same name, but with .html after it.
*/
bool_t Md2HtmlWriteFile(const char *szInFile, const char *szOutFile)
{
  const char *szMdFile;
  char       *szHtml;
  size_t      htmlLen;
  const char *pszNameOnly;
  char        szPath[PATH_MAX];
  bool_t      fWorked = TRUE;

  szMdFile = FlyFileRead(szInFile);
  if(!szMdFile)
  {
    printf("  Cannot open %s\n", szInFile);
    fWorked = FALSE;
  }
  if(fFlyMarkdownDebug)
    g_szMd = szMdFile;
  htmlLen = FlyMd2HtmlFile(NULL, UINT_MAX, szMdFile, szInFile);
  if(!htmlLen)
  {
    printf("  %s doesn't appear to be markdown\n", szInFile);
    fWorked = FALSE;
  }
  else
  {
    szHtml = malloc(htmlLen + 1);
    if(szHtml)
    {
      memset(szHtml, 0, sizeof(htmlLen + 1));
      FlyMd2HtmlFile(szHtml, htmlLen + 1, szMdFile, szInFile);
      pszNameOnly = FlyStrPathNameOnly(szInFile);
      if(!pszNameOnly)
      {
        printf("  internal problem on file %s\n", szInFile);
        fWorked = FALSE;
      }
      else
      {
        if(szOutFile == NULL)
        {
          FlyStrZCpy(szPath, pszNameOnly, sizeof(szPath));
          FlyStrPathChangeExt(szPath, ".html");
          szOutFile = szPath;
        }
        else if(FlyStrPathIsFolder(szOutFile))
        {
          FlyStrZCpy(szPath, szOutFile, sizeof(szPath));
          FlyStrPathAppend(szPath, pszNameOnly, sizeof(szPath));
          FlyStrPathChangeExt(szPath, ".html");
          szOutFile = szPath;
        }
        if(!FlyFileWrite(szOutFile, szHtml))
        {
          fWorked = FALSE;
          printf("  problem writing to file %s\n", szOutFile);
        }
      }
      free(szHtml);
    }
  }
  return fWorked;
}

int main(int argc, const char *argv[])
{
  // command-line options
  bool_t              fVerbose  = FALSE;
  bool_t              fDebug    = FALSE;
  const char         *szOut     = NULL;   // file or folder
  const flyCliOpt_t   cliOpts[] =
  {
    { "--debug", &fDebug,   FLYCLI_BOOL },
    { "-o",      &szOut,    FLYCLI_STRING },
    { "-v",      &fVerbose, FLYCLI_BOOL },
  };
  const flyCli_t cli =
  {
    .pArgc      = &argc,
    .argv       = argv,
    .nOpts      = NumElements(cliOpts),
    .pOpts      = cliOpts,
    .szVersion  = m_szVersion,
    .szHelp     = m_szHelp
  };

  flyCliErr_t   err;
  bool_t        fFailed  = FALSE;
  int           nArgs;
  int           i;
  const char   *pszName;
  char          szOutPath[PATH_MAX];

  // bad options?
  err = FlyCliParse(&cli);
  if(err)
    exit(1);

  if(fDebug)
  {
    fFlyMarkdownDebug = TRUE;
    fVerbose = TRUE;
  }
  else
    fFlyMarkdownDebug = FALSE;

  // display version if verbose
  if(fVerbose)
    printf("%s\n\n", cli.szVersion);

  nArgs = FlyCliNumArgs(&cli);
  if(nArgs == 0)
  {
    printf("%s", cli.szHelp);
    exit(1);
  }

  if(nArgs < 2)
  {
    printf("no input files\n");
    exit(1);
  }

  if(nArgs > 2)
  {
    if(szOut == NULL)
      *szOutPath = '\0';
    else
    {
      FlyStrZCpy(szOutPath, szOut, sizeof(szOutPath));
      if(!FlyFileExistsFolder(szOutPath))
      {
        if(FlyFileMakeDir(szOutPath) != 0)
        {
          printf("Cannot make folder %s\n", szOutPath);
          exit(1);
        }
      }
      if(!isslash(FlyStrCharLast(szOutPath)))
        FlyStrZCat(szOutPath, "/", sizeof(szOutPath));
    }
    if(fVerbose)
      printf("Storing HTML files in folder %s\n", (*szOutPath == '\0') ? "(current)" : szOutPath);
  }
  if(nArgs == 2)
  {
    if(szOut == 0)
    {
      FlyStrZCpy(szOutPath, FlyStrPathNameOnly(FlyCliArg(&cli, 1)), sizeof(szOutPath));
      if(*szOutPath)
        FlyStrPathChangeExt(szOutPath, ".html");
    }
    else
      FlyStrZCpy(szOutPath, szOut, sizeof(szOutPath));
  }

  // parse the input files
  if(fFlyMarkdownDebug)
    fVerbose = TRUE;
  if(fVerbose)
    printf("Converting files from markdown to HTML...\n");
  for(i = 1; i < nArgs; ++i)
  {
    if(nArgs != 2)
    {
      pszName = FlyStrPathNameOnly(FlyCliArg(&cli, i));
      FlyStrPathOnly(szOutPath);
      FlyStrPathAppend(szOutPath, pszName, sizeof(szOutPath));
      FlyStrPathChangeExt(szOutPath, ".html");
    }
    printf("  %s => %s\n", FlyCliArg(&cli, i), szOutPath);

    if(!Md2HtmlWriteFile(FlyCliArg(&cli, i), szOutPath))
    {
      fFailed = TRUE;
      break;
    }
  }

  return fFailed ? 1 : 0;
}
