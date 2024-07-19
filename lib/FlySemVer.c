/**************************************************************************************************
  FlySemVer.c - Easy parsing and comparison of semantic version strings
  Copyright 2024 Drew Gislason  
  license: <https://mit-license.org>
*///***********************************************************************************************
#include "FlySemVer.h"

/*!
  @defgroup FlySemVer - An API for processing Semantic Versioning

  If you are not familiar with Semantic Versioning, see <https://semver.org>.

  See also Rust Lang <https://doc.rust-lang.org/cargo/reference/specifying-dependencies.html>.

  The basics, a Semantic Version is major.minor.patch, e.g. 1.5.19, where:

  1. MAJOR version when you make incompatible API changes
  2. MINOR version when you add functionality in a backward compatible manner
  3. PATCH version when you make backward compatible bug fixes
*/

/*!------------------------------------------------------------------------------------------------
  Compare two Semenatic Versions: szVer1 to szVer2. The version string "*" is equal to anything.

  Examples:

      "2" > "1.1.15"
      "1.1.14" < "1.1.15"
      "*" == "1.1.15"
      "1.1.15" == "*"
      "1.1" > "1"

  @param    szVer1    a version string, e.g. "1", "1.0.15", "*"
  @param    szVer2    a version string, e.g. "1", "1.0.15", "*"
  @return   < 0 (less than), 0 (equal), 1 (greater than)
*///-----------------------------------------------------------------------------------------------
int FlySemVerCmp(const char *szVer1, const char *szVer2)
{
  static const char  *szZero = "0";
  int                 ret = 0;
  int                 ver1;
  int                 ver2;
  int                 i;

  if(*szVer1 != '*' && *szVer2 != '*')
  {
    // major.minor.subver
    for(i = 0; i < 3; ++i)
    {
      ver1 = atoi(szVer1);
      ver2 = atoi(szVer2);

      // ver1 > ver2
      if(ver1 > ver2)
      {
        ret = 1;
        break;
      }

      // ver1 < ver2
      else if(ver1 < ver2)
      {
        ret = -1;
        break;
      }

      // equal, check next minor version
      else
      {
        szVer1 = strchr(szVer1, '.');
        if(szVer1 == NULL)
          szVer1 = szZero;
        else
          ++szVer1;

        szVer2 = strchr(szVer2, '.');
        if(szVer2 == NULL)
          szVer2 = szZero;
        else
          ++szVer2;
      }
    }
  }

  return ret;
}

/*!------------------------------------------------------------------------------------------------
  Is this range a valid range? (that is, begins with a digit and contains only numbers and digits)

  Stops checking at 3rd dot, so "1.2.3.alpha" is valid, but "1.beta" is not.

  @param    szVer     a version string, e.g. "1", "1.0.15", "*"
  @return   TRUE if szVer is valid, FALSE if NOT
*///-----------------------------------------------------------------------------------------------
bool_t  FlySemVerIsValid(const char *szVer)
{
  return (FlySemVerCpy(NULL, szVer, UINT_MAX) == 0) ? FALSE: TRUE;
}

/*!------------------------------------------------------------------------------------------------
  Get a "high" version string from this version range, suitable for comparison.

  Examples:

      1.2.3 returns 2.0.0
      1.2   returns 2.0.0
      1     returns 2.0.0
      0.2.3 returns 0.3.0
      0.2   returns 0.3.0
      0.0.3 returns 0.0.4
      0.0   returns 0.1.0
      0     returns 1.0.0
      *     returns *

  @param    szHigh    returned string, of size - 1
  @param    szRange   a version string, e.g. "1", "1.0.15", "*"
  @param    size      size in bytes of szHigh string buffer
  @return   none
*///-----------------------------------------------------------------------------------------------
void FlySemVerHigh(char *szHigh, const char *szRange, unsigned size)
{
  #define FSVM_NOT_SET  UINT_MAX
  const char  *psz;
  unsigned     i;
  unsigned     aRange[3];

  if(size < 2)
    *szHigh = '\0';

  else if(*szRange == '*')
  {
    szHigh[0] = '*';
    szHigh[1] = '\0';
  }

  else
  {
    // reset ranges
    for(i = 0; i < NumElements(aRange); ++i)
      aRange[i] = FSVM_NOT_SET;

    psz = szRange;
    *szHigh = '\0';
    for(i = 0; i < NumElements(aRange); ++i)
    {
      if(!isdigit(*psz))
        break;

      aRange[i] = atoi(psz) % 100000;  // limit to 5 digits per major.minor.patch
      psz = strchr(psz, '.');
      if(psz == NULL)
        break;
      else
        ++psz;
    }

    // create high range string, e.g. from szRange "1.2.3 create "2.0.0"
    szHigh[size - 1] = '\0';
    --size;
    if(size)
    {
      if(aRange[0] != 0)
        snprintf(szHigh, size, "%d.0.0", aRange[0] + 1);

      // special case for prerelease, e.g. 0, 0.1, 0.1.2, e.g. high range for "0.1" is "0.2"
      else
      {
        if(aRange[1] == FSVM_NOT_SET)
          snprintf(szHigh, size, "1.0.0");
        else if(aRange[2] == FSVM_NOT_SET)
          snprintf(szHigh, size, "0.%d.0", aRange[1] + 1);
        else
          snprintf(szHigh, size, "0.%d.%d", aRange[1], aRange[2] + 1);
      }
    }
  }
}

/*!------------------------------------------------------------------------------------------------
  Does szRange match szVer using the Rust rules related to Semantic Versioning?

  Examples:

      1.2.3 matches >=1.2.3, <2.0.0
      1.2   matches >=1.2.0, <2.0.0
      1     matches >=1.0.0, <2.0.0
      0.2.3 matches >=0.2.3, <0.3.0
      0.2   matches >=0.2.0, <0.3.0
      0.0.3 matches >=0.0.3, <0.0.4
      0.0   matches >=0.0.0, <0.1.0
      0     matches >=0.0.0, <1.0.0
      *     matches anything

  @param    szRange   a version string, e.g. "1", "1.0.15", "*"
  @param    szVer     a version string, e.g. "1", "1.0.15", "*"
  @return   TRUE if szVer is in range, FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlySemVerMatch(const char *szRange, const char *szVer)
{
  bool_t      fMatch = FALSE;
  char        szRangeHigh[(5 * 3) + 4];   // allow 5 digit ints for major.minor.patch

  if(*szRange == '*' || *szVer == '*')
    fMatch = TRUE;
  else if(!isdigit(*szRange) || !isdigit(*szVer))
    fMatch = FALSE;
  else
  {
    FlySemVerHigh(szRangeHigh, szRange, sizeof(szRangeHigh));
    if(FlySemVerCmp(szVer, szRange) < 0)
      fMatch = FALSE;
    else if(FlySemVerCmp(szVer, szRangeHigh) >= 0)
      fMatch = FALSE;
    else
      fMatch = TRUE;
  }

  return fMatch;
}

/*!------------------------------------------------------------------------------------------------
  Copy the Sem Ver string. 1st character must be a number, or return length is 0.

  Some Examples:

      "*" returns len 1
      "1" returns len 1
      "1.2.3" returns length 5
      "124.456.789.alpha" returns len 17
      "1.2 and more text" returns len 3
      "v1.2" returns len 0

  @param    szDst     destination string buffer or NULL to just get length.
  @param    szSemVer  ptr to 1st character of a semver string
  @param    size      size of szDst buffer in bytes
  @return   length of semver string or 0 if not a valid semver string
*///-----------------------------------------------------------------------------------------------
unsigned FlySemVerCpy(char *szDst, const char *szSemVer, unsigned size)
{
  const char *psz;
  unsigned    len       = 0;
  unsigned    nDots     = 0;
  bool_t      fValid    = TRUE;
  unsigned    cpyLen;

  psz = szSemVer;
  if(*szSemVer == '*')
    len = 1;
  else if(isdigit(*szSemVer))
  {
    psz = szSemVer;
    while(*psz && size > 1)
    {
      if(isspace(*psz))
        break;
      else if(*psz == '.')
      {
        ++nDots;

        // after 3rd dot, can be any string, e.g. "1.2.3.alpha"
        if(nDots == 3)
        {
          while(*psz && !isspace(*psz))
            ++psz;
          break;
        }

        // don't allow 2 dots in a row, e.g. "1..2" is invalid
        if(psz[1] == '.')
        {
          fValid = FALSE;
          break;
        }
      }

      // only digits and dots allowed until after 2
      else if(!isdigit(*psz))
      {
        fValid = FALSE;
        break;
      }
      ++psz;
    }

    if(fValid)
      len = (unsigned)(psz - szSemVer);
  }

  // copy the semver to szDst if not NULL
  if(szDst && size && len)
  {
    cpyLen = len;
    if(cpyLen > size - 1)
      cpyLen = size - 1;
    memcpy(szDst, szSemVer, cpyLen);
    szDst[cpyLen] = '\0';
  }

  return len;  
}
