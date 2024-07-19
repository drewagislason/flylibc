/**************************************************************************************************
  example_toml.c  
  Copyright 2023 Drew Gislason  
  License: MIT <https://mit-license.org>
**************************************************************************************************/
#include "FlyFile.h"
#include "FlyToml.h"

typedef struct
{
  unsigned    nTables;        // # of header `[tables]`, possibly with unnamed root table
  unsigned    nKeys;          // total # of keys in all tables including inline-tables
  unsigned    nArrays;        // # of array values (TOML_ARRAY)
  unsigned    nInlineTables;  // # of inline-tables (TOML_INLINE_TABLE)
  unsigned    nBools;         // # of booleans (TOML_TRUE and TOML_FALSE)
  unsigned    nIntegers;      // # of integers (TOML_INTEGER)
  unsigned    nStrings;
#if TOML_CFG_FLOAT
  unsigned    nFloats;        // # of floats (TOML_FLOAT)
#endif
#if TOML_CFG_DATE
  unsigned    nDates;         // # of dates/times (TOML_DATE)
#endif
  unsigned    nUnknowns;      // # of unknown value types (TOML_UNKNOWN)
} myTomlStats_t;

static const char szHelp[] = 
  "usage = example_toml file.toml\n"
  "\n"
  "Parse a TOML file and show statistics. See the TOML specification: <https://toml.io/en/>\n";

/*------------------------------------------------------------------------------------------------
  Return static string for the type, or ??? if unknown type
  @return static string
*///-----------------------------------------------------------------------------------------------
const char * my_toml_type_str(tomlType_t type)
{
  const char *szType;

  switch(type)
  {
    case TOML_UNKNOWN: szType = "TOML_UNKNOWN"; break;
    case TOML_FALSE: szType = "TOML_FALSE"; break;
    case TOML_TRUE: szType = "TOML_TRUE"; break;
    case TOML_INTEGER: szType = "TOML_INTEGER"; break;
#if TOML_CFG_FLOAT
    case TOML_FLOAT: szType = "TOML_FLOAT"; break;
#endif
#if TOML_CFG_FLOAT_DATE
    case TOML_DATE: szType = "TOML_DATE"; break;
#endif
    case TOML_STRING: szType = "TOML_STRING"; break;
    case TOML_ARRAY: szType = "TOML_ARRAY"; break;
    case TOML_INLINE_TABLE: szType = "TOML_INLINE_TABLE"; break;
    default: szType = "???"; break;
  }

  return szType;
}

/*------------------------------------------------------------------------------------------------
  Print TOML statistics.

  @param    szFile    pointer to asciiz TOML file
  @paaram   pStats    pointer to status
  @return   none
*///-----------------------------------------------------------------------------------------------
void my_toml_stats_print(const char *szFile, myTomlStats_t *pStats)
{
  printf("Statistics for file: %s\n\n", szFile);
  printf("  %2u nTable(s)\n",       pStats->nTables);
  printf("  %2u nKey(s)\n",         pStats->nKeys);
  printf("  %2u nBool(s)\n",        pStats->nBools);
  printf("  %2u nInteger(s)\n",     pStats->nIntegers);
  printf("  %2u nString(s)\n",      pStats->nStrings);
#if TOML_CFG_FLOAT
  printf("  %2u nFloat(s)\n",       pStats->nFloats);
#endif
#if TOML_CFG_DATE
  printf("  %2u nDate(s)\n",        pStats->nDates);
#endif
  printf("  %2u nUnknown(s)\n",     pStats->nUnknowns);
  printf("  %2u nArray(s)\n",       pStats->nArrays);
  printf("  %2u nInlineTable(s)\n", pStats->nInlineTables);
}

/*------------------------------------------------------------------------------------------------
  Determine statistics (# of all items) in a TOML file.

  @param    szFile    pointer to asciiz TOML file
  @paaram   pStats    pointer to status
  @return   none
*///-----------------------------------------------------------------------------------------------
void my_toml_stats_get(const char *szFile, myTomlStats_t *pStats)
{
  const char     *szTomlFile;
  const char     *pszTable;
  const char     *pszKey;
  tomlKey_t       key;

  // initialize statistics
  memset(pStats, 0, sizeof(*pStats));

  // iterate through TOML file
  szTomlFile = FlyFileRead(szFile);
  if(szTomlFile)
  {
    pszTable = FlyTomlTableIter(szTomlFile, NULL);
    while(pszTable)
    {
      ++pStats->nTables;
      pszKey = FlyTomlKeyIter(FlyTomlTableIsRoot(pszTable) ? szTomlFile : pszTable, &key);
      while(pszKey)
      {
        // BUGBUG
        printf("table: %.*s\n", (unsigned)FlyStrLineLen(pszTable), pszTable);
        printf("key:   %.*s\n", (unsigned)FlyStrLineLen(pszKey), pszKey);

        ++pStats->nKeys;
        switch(key.type)
        {
          case TOML_FALSE:
          case TOML_TRUE:
            ++pStats->nBools;
          break;
          case TOML_INTEGER:
            ++pStats->nIntegers;
          break;
#if TOML_CFG_FLOAT
          case TOML_FLOAT:
            ++pStats->nFloats;
          break;
#endif
#if TOML_CFG_DATE
          case TOML_DATE:
            ++pStats->nDates;
          break;
#endif
          case TOML_STRING:
            ++pStats->nStrings;
          break;
          case TOML_ARRAY:
            ++pStats->nArrays;
          break;
          case TOML_INLINE_TABLE:
            ++pStats->nInlineTables;
          break;
          case TOML_UNKNOWN:
          default:
            ++pStats->nUnknowns;
          break;
        }
        pszKey = FlyTomlKeyIter(pszKey, &key);
      }
      pszTable = FlyTomlTableIter(szTomlFile, pszTable);
    }
  }
}

int main(int argc, const char *argv[])
{
  unsigned        i;
  myTomlStats_t   stats;

  if(argc >= 2 && strcmp(argv[1], "--help") == 0)
  {
    printf("%s\n", szHelp);
    return 0;
  }

  for(i = 1; i < argc; ++i)
  {
    my_toml_stats_get(argv[i], &stats);
    my_toml_stats_print(argv[i], &stats);
  }

  return 0;
}
