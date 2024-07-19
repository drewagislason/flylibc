#include "FlyFile.h"

const char szHelp[] = 
  "usage = example_fileinfo '../folder/'\n"
  "\n"
  "make sure to use the 'quotes' so bash doesn't expand anything for you\n";

int main(int argc, const char *argv[])
{
  bool_t          fExists;
  unsigned        i;
  sFlyFileInfo_t  info;

  if(argc >= 2 && strcmp(argv[1], "--help") == 0)
  {
    printf("%s\n", szHelp);
    return 0;
  }

  for(i = 1; i < argc; ++i)
  {
    FlyFileInfoInit(&info);
    fExists = FlyFileInfoGet(&info, argv[i]);
    if(!fExists)
      printf("%s doesn't exist\n", argv[i]);
    else
    {
      printf("%i: %s full path is %s\nflags: fIsDir %u, fRdOnly %u\n",
        i, argv[i], info.szFullPath, info.fIsDir, info.fRdOnly);
    }
  }

  return 0;
}
