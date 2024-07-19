#include "FlyStr.h"

/*-------------------------------------------------------------------------------------------------
  Give info about compiler, such as C99, sizeof(size_t), etc...

  @param    pState    cmdline options, etc...
  @return   TRUE if worked, FALSE if failed
*///-----------------------------------------------------------------------------------------------
int main(void)
{
  typedef enum {
    MY_ENUM0,
    MY_ENUM1,
  } myEnum_t;

  typedef struct {
    char    c;
    int     i;
    char   *psz;
  } myStruct_t;

  const char szLargeNumber[] =
    "max uint64_t = 18,446,744,073,709,551,615\n"
    "\n"
    "eighteen quintillion four hundred forty-six quadrillion seven hundred forty-four trillion\n"
    "seventy-three billion seven hundred nine million five hundred fifty-one thousand six hundred\n"
    "fifteen.\n"
    "\n"
    "64-bits       E   P   T   G   M   K   B\n"
    "INT64_MAX  =  9,223,372,036,854,775,807\n"
    "UINT64_MAX = 18,446,744,073,709,551,615\n";

  int         i;
  size_t      max;
  const char  *szLabels[] = { "bytes", "kilobytes", "megabytes", "gigabytes", "terabytes", "petabytes", "exobytes" };

  printf("\nC Version %s\n\n", FlyStrCVer());
  printf("%s\n", szLargeNumber);

  for(i = 0, max = SIZE_MAX; i < NumElements(szLabels) && max; ++i)
  {
    printf("%21zu %s\n", max, szLabels[i]);
    max = max / 1024;
  }
  printf("\n");

  printf("sizeof(enum)      = %zu\n", sizeof(myEnum_t));
  printf("sizeof(myStruct_t)= %zu { char c; int i; char *psz; }\n", sizeof(myStruct_t));
  printf("sizeof(short int) = %zu, SHRT_MIN = %i, SHRT_MAX= %i\n", sizeof(short int), SHRT_MIN, SHRT_MAX);
  printf("sizeof(int)       = %zu, INT_MIN = %i, INT_MAX= %i\n", sizeof(int), INT_MIN, INT_MAX);
  printf("sizeof(long)      = %zu, LONG_MIN = %li, LONG_MAX= %li\n", sizeof(long), LONG_MIN, LONG_MAX);
  printf("sizeof(long long) = %zu, LLONG_MIN = %lli, LLONG_MAX= %lli\n", sizeof(long long), LLONG_MIN, LLONG_MAX);
  printf("sizeof(int8_t)    = %zu, INT8_MIN = %i, INT8_MAX= %i\n", sizeof(int8_t), INT8_MIN, INT8_MAX);
  printf("sizeof(int16_t)   = %zu, INT16_MIN = %i, INT16_MAX= %i\n", sizeof(int16_t), INT16_MIN, INT16_MAX);
  printf("sizeof(int32_t)   = %zu, INT32_MIN = %i, INT32_MAX= %i\n", sizeof(int32_t), INT32_MIN, INT32_MAX);
#ifdef INT64_MAX
  printf("sizeof(int64_t)   = %zu, INT64_MIN = %lli, INT64_MAX= %lli\n\n", sizeof(int64_t), INT64_MIN, INT64_MAX);
#endif
  printf("sizeof(short)     = %zu, min = %u, USHRT_MAX= %u\n", sizeof(unsigned short), 0, USHRT_MAX);
  printf("sizeof(unsigned)  = %zu, min = %u, UINT_MAX= %u\n", sizeof(unsigned), 0, UINT_MAX);
  printf("sizeof(uint8_t)   = %zu, min = %u, UINT8_MAX= %u\n", sizeof(int8_t), 0, UINT8_MAX);
  printf("sizeof(uint16_t)  = %zu, min = %u, UINT16_MAX= %u\n", sizeof(int16_t), 0, UINT16_MAX);
  printf("sizeof(uint32_t)  = %zu, min = %u, UINT32_MAX= %u\n", sizeof(int32_t), 0, UINT32_MAX);
#ifdef UINT64_MAX
  printf("sizeof(uint64_t)  = %zu, min = %llu, UINT64_MAX= %llu\n", sizeof(int64_t), (uint64_t)0, UINT64_MAX);
#endif
  printf("sizeof(size_t)    = %zu, max = %zu\n\n", sizeof(size_t), SIZE_MAX);

  printf("PATH_MAX          = %u\n", PATH_MAX);
  printf("ARG_MAX           = %u\n", ARG_MAX);

  return 0;
}
