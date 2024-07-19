/**************************************************************************************************
  flysha.c  
  Copyright 2024 Drew Gislason  
  license: MIT <https://mit-license.org>
**************************************************************************************************/
#include "FlyFile.h"
#include "FlyStr.h"

static const char m_szTmpFile[] = "flysha.tmp";
static const char m_szVersion[] = "flysha version 1.0";
static const char m_szHelp[] =
  "\n%s\n"
  "\n"
  "usage = flysha [--sha sha] [outfile]\n"
  "\n"
  "flysha creates string constants FLYSHA_SHA and FLYSHA_COMMIT using git log.\n"
  "\n"
  "Output is suitable to add to C/C++ programs. If no outfile, then prints to screen."
  "If sha is given, then output is for that commit sha.\n"
  "\n";
static const char m_szPattern[] =
    "#define FLYSHA_SHA  \"%.*s\"\n"
    "#define FLYSHA_COMMIT \"%.*s\"\n";

/*!
  @defgroup flysha - flysha creates FLYSHA_SHA and FLYSHA_COMMIT constants using git log.

  usage = flysha [-s sha] [outfile]

  Output is suitable to add to C/C++ programs. If no outfile, then prints to screen. If sha is
  given, then output is for that sha.

  Example output:

      #define FLYSHA_SHA  "d5b4f75"
      #define FLYSHA_COMMIT "(HEAD -> main, origin/main) check-in message"

  This can be useful to autogenerate a version string that includes the git SHA, for example:

      const char szVersion[] = "1.0.0." FLYSHA_SHA;
*/

int main(int argc, const char *argv[])
{
  FILE               *fp          = stdout;
  const char         *szOutFile   = NULL;
  char               *szSha       = NULL;
  char               *szCommit    = NULL;
  int                 ret;
  unsigned            shaLen;
  unsigned            i;
  char                szCmdline[128];

  // process arguments and options
  for(i = 1; i < argc; ++i)
  {
    if(strcmp(argv[i], "--help") == 0)
    {
      printf(m_szHelp, m_szVersion);
      return 1;
    }
    else if(strcmp(argv[i], "--version") == 0)
    {
      printf("%s\n", m_szVersion);
      return 1;
    }
    else if(strcmp(argv[i], "--sha") == 0)
    {
      ++i;
      if(i < argc)
        szSha = (char *)argv[i];
      else
      {
        fprintf(stderr, "mising argument for option --sha\n");
        return 1;
      }
    }
    else if(argv[i][0] == '-')
    {
      fprintf(stderr, "invalid argument %s. try flysha --help\n", argv[i]);
      return 1;
    }
    else
      szOutFile = argv[i];
  }

  // git log provides the SHA and commit message
  if(szSha)
    snprintf(szCmdline, sizeof(szCmdline), "git log %s -1 --oneline >%s", szSha, m_szTmpFile);
  else
    snprintf(szCmdline, sizeof(szCmdline), "git log -1 --oneline >%s", m_szTmpFile);

  ret = system(szCmdline);
  if(ret != 0)
  {
    fprintf(stderr, "Cannot create %s. git log failed or current folder is not writtable.\n", m_szTmpFile);
    snprintf(szCmdline, sizeof(szCmdline), "rm -f %s", m_szTmpFile);
    system(szCmdline);
    return 1;
  }

  // get content into gitSha
  else
  {
    szSha = FlyFileRead(m_szTmpFile);
    if(szSha == NULL)
    {
      fprintf(stderr, "Cannot read %s\n", m_szTmpFile);
      return 1;
    }
  }
  snprintf(szCmdline, sizeof(szCmdline), "rm %s", m_szTmpFile);
  system(szCmdline);

  // create output file
  if(szOutFile != NULL)
  {
    fp = fopen(szOutFile, "w");
    if(!fp)
    {
      fprintf(stderr, "Cannot create file %s\n", szOutFile);
      return 1;
    }
  }

  shaLen = FlyStrArgLen(szSha);
  szCommit = &szSha[shaLen + 1];
  if(fprintf(fp, m_szPattern, shaLen, szSha, (int)FlyStrLineLen(szCommit), szCommit) <= 0)
  {
    fprintf(stderr, "Failed to write file %s\n", szOutFile);
    return 1;
  }
  if(szOutFile != NULL)
    fclose(fp);

  return 0;
}
