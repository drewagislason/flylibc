/**************************************************************************************************
  FlyAnsi.c - Ansi terminal codes for color and positioning text
  Copyright (c) 2024 Drew Gislason
  license: <https://mit-license.org>
**************************************************************************************************/
#include <stdio.h>
#include <sys/ioctl.h>
#include "FlyAnsi.h"

/*!
  @defgroup FlyAnsi   ANSI color, cursor position, and screen clearing API

  Features:

  1. Get size of screen
  2. Position cursor anywhere on screen
  3. Set current color for text, including background
  4. Colors are simple combination of foreground/background colors
*/

static size_t m_nChars;

/*!----------------------------------------------------------------------------------
  Return total # of characters printed via this interface

  @return   none
*///---------------------------------------------------------------------------------
size_t AnsiCharCount(void)
{
  return m_nChars;
}

/*!----------------------------------------------------------------------------------
  Reset total character count (useful for optimizing)

  @return   none
*///---------------------------------------------------------------------------------
void AnsiCharCountReset(void)
{
  m_nChars = 0;
}

/*!----------------------------------------------------------------------------------
  Goto row/column (use 0 based row/col, ANSI treats it as 1 based)

  @param    row    terminal screen row 0-n
  @param    col    terminal screen col 0-n
  @return   none
*///---------------------------------------------------------------------------------
void AnsiGoto(unsigned row, unsigned col)
{
  m_nChars += printf("\033[%u;%uH",row+1,col+1);
  fflush(stdout);
}

/*!----------------------------------------------------------------------------------
  Clear the screen to the current attribute.

  @return   none
*///---------------------------------------------------------------------------------
void AnsiClearScreen(void)
{
  AnsiGoto(0,0);
  m_nChars += printf("\033[2J");
  fflush(stdout);
}

/*!----------------------------------------------------------------------------------
  Clear to end of line with the current attribute.

  @return   none
*///---------------------------------------------------------------------------------
void AnsiClearEol(void)
{
  m_nChars += printf("\033[K");
  fflush(stdout);
}

/*!----------------------------------------------------------------------------------
  Set the color to one of 256 values using a mask of FLYATTR_x and NEDBACK_x. See
  also ::nedAttr_t.

  @param    attr        Attribute mask that includes foreground and background color.
  @return   none
*///---------------------------------------------------------------------------------
void AnsiSetAttr(fflyAttr_t attr)
{
  // no color for background
  if((attr & 0xf0) == 0)
    m_nChars += printf("\033[0m");

  // color for char (and potentially background)
  if(attr)
    m_nChars += printf("\033%s", AnsiGetAttrStr(attr));
}

/*!----------------------------------------------------------------------------------
  Get the current # of rows and cols in the terminal screen.

  @param    pRows   pointer to an unsigned to receive # of rows
  @param    pCols   pointer to an unsigned to receive # of cols
  @return   none
*///---------------------------------------------------------------------------------
void AnsiGetRowsCols(unsigned * pRows, unsigned * pCols)
{
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  *pRows = (unsigned)w.ws_row;
  *pCols = (unsigned)w.ws_col;
}

/*!----------------------------------------------------------------------------------
  Display all the colors in a grid. Won't look very good if the terminal is not
  at least 18 rows by 182 columns.

  @return   none
*///---------------------------------------------------------------------------------
void AnsiShowAllColors(void)
{
  unsigned  i;
  char      szNumber[5];  // 0x0f
  unsigned  rows;
  unsigned  cols;
  unsigned  width = 5;

  AnsiSetAttr(FLYATTR_RESET);
  AnsiGetRowsCols(&rows,&cols);
  if(cols > 5+(11*16))
    width = 11;

  m_nChars += printf("%5s", "");
  for(i=0; i<16; ++i)
  {
    snprintf(szNumber, sizeof(szNumber), "0x%02x",i);
    m_nChars += printf("%*s", width, szNumber);
  }
  for(i=0; i<256; ++i)
  {
    if((i%16) == 0)
    {
      AnsiSetAttr(FLYATTR_RESET);
      snprintf(szNumber, sizeof(szNumber), "0x%02x",i);
      m_nChars += printf("\n%5s",szNumber);
    }
    AnsiSetAttr((fflyAttr_t)i);
    if(width == 5)
    {
      snprintf(szNumber, sizeof(szNumber), "%u",i);
      m_nChars += printf("%5s",szNumber);
    }
    else
      m_nChars += printf("%11s",AnsiGetAttrStr((fflyAttr_t)i));
  }
  AnsiSetAttr(FLYATTR_RESET);
  m_nChars += printf("\n");
}

/*!----------------------------------------------------------------------------------
  Get the attribute string from the attribute, not including the preceding \033 (ESC).

  ANSI Foreground colors are 30-37, or 90-97, background colors are 0, 40-47 or 100-107
  Pattern is [1;FORE;BACKm, or [0;FORE;BACKm

  @param    attr        Attribute mask that includes foreground and background color.
  @return   constant string pointing to attribute. Overwritten by next call to AnsiGetAttrStr()
*///---------------------------------------------------------------------------------
const char * AnsiGetAttrStr(fflyAttr_t attr)
{
  static char szAttrStr[12];
  unsigned    fore    = 30;
  unsigned    back    = 40;
  char       *pszBold = "0;";

  // reset to normal screen color
  if(attr == FLYATTR_RESET)
  {
    return "[0m";
  }

  // get foreground color
  if(attr & 0x08)
  {
    pszBold = "1;";
    fore    = 90;
  }
  fore += (attr & 0x7);

  // get background color
  if( (attr & 0xf0) == FLYBACK_TERMINAL )
    back = 0;
  else
  {
    if( (attr & 0xf0) == FLYBACK_BLACK )
      attr = 0;
    if(attr & 0x80)
      back = 100;
    back += ((attr & 0x70) >> 4);
  }

  // return the terminal color
  if( back == 0 )
    sprintf(szAttrStr, "[%s%um",pszBold,fore);
  else  
    sprintf(szAttrStr, "[%s%u;%um",pszBold,fore,back);
  return (const char *)szAttrStr;
}
