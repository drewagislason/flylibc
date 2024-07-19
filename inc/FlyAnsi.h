/**************************************************************************************************
  FlyAnsi.h
  Copyright (c) 2024 Drew Gislason
  license: MIT <https://mit-license.org>
*///***********************************************************************************************

#ifndef FLY_ANSI_H
#define FLY_ANSI_H
#include "Fly.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

/*!
  This is a bitmask of one FLYATTR_ with one FLYBACK_ color, and optionally
  FLYATTR_BOLD. Use FLYATTR_RESET by itself (no background or bold).

  Example: AnsiSetAttr(FLYATTR_BOLD | FLYATTR_RED | FLYBACK_PURPLE);
*/
typedef uint8_t fflyAttr_t;
#define FLYATTR_NONE                  0     //!< default terminal color
#define FLYATTR_RESET                 0     //!< default terminal color
#define FLYATTR_BLACK                 0x00  //!< black text
#define FLYATTR_RED                   0x01  //!< red text
#define FLYATTR_GREEN                 0x02  //!< green text
#define FLYATTR_YELLOW                0x03  //!< yellow text
#define FLYATTR_BLUE                  0x04  //!< blue text
#define FLYATTR_PURPLE                0x05  //!< purple (magenta) text
#define FLYATTR_CYAN                  0x06  //!< cyan (blue-green) text
#define FLYATTR_WHITE                 0x07  //!< white text
#define FLYATTR_BOLD                  0x08  //!< make text bolder and brighter
#define FLYBACK_TERMINAL              0x00  //!< background color of terminal
#define FLYBACK_BRICK                 0x10  //!< brick colored background
#define FLYBACK_FOREST                0x20  //!< forest (deep green) background
#define FLYBACK_BLACK                 0x30  //!< black background
#define FLYBACK_NIGHT                 0x40  //!< navy blue background
#define FLYBACK_PURPLE                0x50  //!< purple background
#define FLYBACK_AQUA                  0x60  //!< aqua background
#define FLYBACK_GREY                  0x70  //!< grey background
#define FLYBACK_CHARCOAL              0x80  //!< dark grey background
#define FLYBACK_RED                   0x90  //!< red background
#define FLYBACK_GREEN                 0xA0  //!< green background
#define FLYBACK_YELLOW                0xB0  //!< yello background
#define FLYBACK_BLUE                  0xC0  //!< blue background
#define FLYBACK_VIOLET                0xD0  //!< violet background
#define FLYBACK_CYAN                  0xE0  //!< cyan background
#define FLYBACK_WHITE                 0xF0  //!< white background

// some default colors
#define FLYATTR_EDIT_FRAME            (FLYATTR_WHITE  | FLYATTR_BOLD | FLYBACK_NIGHT)
#define FLYATTR_EDIT_TXT              (FLYATTR_WHITE  | FLYATTR_BOLD | FLYBACK_NIGHT)
#define FLYATTR_ALERT_FRAME           (FLYATTR_YELLOW | FLYATTR_BOLD | FLYBACK_RED)
#define FLYATTR_ALERT_TXT             (FLYATTR_WHITE  | FLYATTR_BOLD | FLYBACK_RED)

void        AnsiGoto           (unsigned row, unsigned col);
void        AnsiClearScreen    (void);
void        AnsiClearEol       (void);
void        AnsiSetAttr        (fflyAttr_t attr);
const char *AnsiGetAttrStr     (fflyAttr_t attr);
void        AnsiGetRowsCols    (unsigned *pRows, unsigned *pCols);
void        AnsiShowAllColors  (void);

size_t      AnsiCharCount      (void);
void        AnsiCharCountReset (void);

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  }
#endif

#endif // FLY_ANSI_H
