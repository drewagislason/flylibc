/**************************************************************************************************
  FlyLog.c - Parsable logging to memory, files or screen
  Copyright 2024 Drew Gislason
  license: <https://mit-license.org>
**************************************************************************************************/
#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "FlyLog.h"

/*!
  @defgroup FlyLog  A logging API useful in both desktop and embedded environments

  Features:

  1. By default, logs to a file and to the screen
  2. The log lines can be directed anywhere (e.g. to TCP/IP)
  3. Enable/disable log flags for selective logging
  4. Supports logging colors if desired
  5. Supports limiting size of log so it never overflows alloted space

  This logger can aid debugging of many systems, or simply record event.
*/
const char szFlyLogDefaultName[] = FLY_LOG_NAME;

static FILE          *m_fpLog;
static flyLogMask_t  m_logMask;
static size_t        m_logSize;

/*!------------------------------------------------------------------------------------------------
  Create a new log file. This doesn't append, creates a new log.

  @param    szFilePath    standard printf format
  @return   length of printed string
*///-----------------------------------------------------------------------------------------------
const char * FlyLogDefaultName(void)
{
  return szFlyLogDefaultName;
}

/*!------------------------------------------------------------------------------------------------
  Create a new log file. This doesn't append, creates a new log.

  @param    szFilePath    standard printf format
  @return   length of printed string
*///-----------------------------------------------------------------------------------------------
bool_t FlyLogFileOpen(const char *szFilePath)
{
  m_logSize = 0;
  FlyLogFileClose();
  m_fpLog = fopen(szFilePath, "w");
  return m_fpLog ? TRUE : FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Create a new log file. This doesn't append, creates a new log.

  @param    szFilePath    standard printf format
  @return   length of printed string
*///-----------------------------------------------------------------------------------------------
bool_t FlyLogFileAppend(const char *szFilePath)
{
  m_fpLog = fopen(szFilePath, "a");
  return m_fpLog ? TRUE : FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Close the log file if open

  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyLogFileClose(void)
{
  if(m_fpLog)
    fclose(m_fpLog);
}

/*!------------------------------------------------------------------------------------------------
  print to the log file. Flushed every call.

  @param    szFormat    standard printf format
  @return   length of printed string
*///-----------------------------------------------------------------------------------------------
int FlyLogPrintf(const char *szFormat, ...)
{
  va_list        arglist;
  int            len = 0;

  // open log file if not already open
  if(m_fpLog == NULL)
    m_fpLog = fopen(szFlyLogDefaultName, "a");

  // print to logfile and flush
  if(m_fpLog)
  {
    va_start(arglist, szFormat);
    len = vfprintf(m_fpLog, szFormat, arglist);
    va_end(arglist);
    fflush(m_fpLog);
  }

  m_logSize += len;
  return len;
}

/*!------------------------------------------------------------------------------------------------
  print to the log file, but only if the mask bit is set. Flushed every call.

  @param    mask        mask for this printf
  @param    szFormat    standard printf format
  @return   length of printed string
*///-----------------------------------------------------------------------------------------------
int FlyLogPrintfEx(flyLogMask_t mask, const char *szFormat, ...)
{
  va_list        arglist;
  int            len = 0;

  if(mask & m_logMask)
  {
    // open log file if not already open
    if(m_fpLog == NULL)
      m_fpLog = fopen("debug.log", "a");

    // print to logfile and flush
    if(m_fpLog)
    {
      va_start(arglist, szFormat);
      len = vfprintf(m_fpLog, szFormat, arglist);
      va_end(arglist);
      fflush(m_fpLog);
    }
  }

  m_logSize += len;
  return len;
}

/*!------------------------------------------------------------------------------------------------
  Dump hex data to the log file.

  41 41 41 41 41 41 41 41  42 42 42 42 42 42 42 42  |AAAAAAAABBBBBBBB|

  @param    pData     data to dump
  @param    len       length of data to dump
  @param    linlen    length of line
  @param    indent    indent
  @return   length of dump text
*///-----------------------------------------------------------------------------------------------
size_t FlyLogHexDump(const void *pDataIn, unsigned len, unsigned linelen, unsigned indent)
{
  const uint8_t  *pData = pDataIn;
  unsigned        i;
  unsigned        offset;
  unsigned        thislen;
  unsigned        spaces;
  uint8_t         c;
  size_t          txtLen = 0;

  // default line length to 16
  if(linelen == 0)
    linelen = 16;

  offset = 0;
  while(offset < len)
  {
    if(indent)
      txtLen += FlyLogPrintf("%*s",indent,"");
    thislen = linelen;
    if(len - offset < linelen)
      thislen = len - offset;
    for(i=0; i<thislen; ++i)
    {
      txtLen += FlyLogPrintf("%02x ", pData[offset+i]);
    }
    spaces = 3 * (linelen - thislen);
    if(spaces)
      txtLen += FlyLogPrintf("%*s",spaces,"");
    txtLen += FlyLogPrintf(" |");
    for(i=0; i<thislen; ++i)
    {
      c = pData[offset+i];
      txtLen += FlyLogPrintf("%c",isprint(c) ? c : '.');
    }
    spaces = (linelen - thislen);
    if(spaces)
      txtLen += FlyLogPrintf("%*s",spaces,"");
    txtLen += FlyLogPrintf("|\n");
    offset += thislen;
  }

  return txtLen;
}

/*!------------------------------------------------------------------------------------------------
  Dump hex data to the log file, but only if the mask matches.

  41 41 41 41 41 41 41 41  42 42 42 42 42 42 42 42  |AAAAAAAABBBBBBBB|

  @param    pData     data to dump
  @param    len       length of data to dump
  @param    linlen    length of line
  @param    indent    indent
  @return   length of dump text
*///-----------------------------------------------------------------------------------------------
size_t FlyLogHexDumpEx(flyLogMask_t mask, const void *pData, unsigned len, unsigned linelen, unsigned indent)
{
  size_t txtLen = 0;
  if(FlyLogMaskGet() & mask)
    txtLen = FlyLogHexDump(pData, len, linelen, indent);
  return txtLen;
}

/*!------------------------------------------------------------------------------------------------
  Set the log mask bits, affects what will print with FlyLogPrintfEx()

  @param    mask      bitmask, affects FlyLogPrintfEx()

  @return   old mask
*///-----------------------------------------------------------------------------------------------
flyLogMask_t FlyLogMaskSet(flyLogMask_t mask)
{
  flyLogMask_t oldMask = m_logMask;
  m_logMask = mask;
  return oldMask;
}

/*!------------------------------------------------------------------------------------------------
  Get the current mask

  @return   current bitmask, affects FlyLogPrintfEx()
*///-----------------------------------------------------------------------------------------------
flyLogMask_t FlyLogMaskGet(void)
{
  return m_logMask;
}

/*!------------------------------------------------------------------------------------------------
  Get the total size of the log

  @return   size of the log in bytes
*///-----------------------------------------------------------------------------------------------
size_t FlyLogSizeGet(void)
{
  return m_logSize;
}

/*!------------------------------------------------------------------------------------------------
  Reset size of log (does not delete the file. Higher layer must do that)

  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyLogSizeReset(void)
{
  m_logSize = 0;
}
