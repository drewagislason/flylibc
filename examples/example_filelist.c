#include "FlyFile.h"

const char szHelp[] = 
  "usage = example_filelist '../folder/*'\n"
  "\n"
  "make sure to use the 'quotes' so bash doesn't expand the file list for you\n";

int main(int argc, const char *argv[])
{
  void       *hList;
  unsigned    i, j;
  char        szPath[PATH_MAX];
  unsigned    len;

  if(argc >= 2 && strcmp(argv[1], "--help") == 0)
  {
    printf("%s\n", szHelp);
    return 0;
  }

  for(i = 1; i < argc; ++i)
  {
    printf("Filter: '%s'", argv[i]);
    hList = FlyFileListNew(argv[i]);
    if(!hList)
      printf(" not found\n");
    else
    {
      *szPath = '\0';
      len = FlyFileListGetBasePath(hList, szPath, sizeof(szPath));
      printf(" BasePath: len %u, '%s'\n", len, szPath);
      for(j = 0; j < FlyFileListLen(hList); ++j)
        printf("  %s\n", FlyFileListGetName(hList, j));
      FlyFileListFree(hList);
      printf("\n");
    }
  }

  return 0;
}
