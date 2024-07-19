/**************************************************************************************************
  FlyKey.c - Full keyboard input, e.g. Alt-Left-Arrow, Ctrl-Space
  Copyright (c) 2024 Drew Gislason
  license: <https://mit-license.org>
**************************************************************************************************/
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "FlyKey.h"

/*!
  @defgroup   FlyKey   Full keyboard input API with idle for other processing

  For terminal applications that need arrow keys, ctrl-keys, alt-keys, and fn keys.

  To get a full list of supported keys, see flylibc/examples/example_key.c

  Internally this uses the C standard library cfmakeraw(), tcsetattr() and read().
  See <https://linux.die.net/man/3/cfmakeraw>

  Features:

  1. Supports ctrl-, alt- and function keys
  2. Converts the key into a single keycode.
  3. Provdes names for keys, e.g. "Enter", "Ctrl-Left", "Fn10", etc...
  4. Allows idle processing for other computing (for example, TCP/IP)

  Tested on Linux and macOS.

  @example FlyKey: show name of each key until Esc exits

  ```c
  #include "FlyKey.h"

  int main(void)
  {
    flyKey_t  key = FLY_KEY_NONE;

    printf("Press any key, like ctrl-c, right-arrow, alt-q, etc... Press esc to exit.\n");
    while(key != FLY_KEY_ESC)
    {
      key = FlyKeyGetKey();
      printf("%s\n", FlyKeyName(key));
    }

    return 0;
  }
  ```

  For an example of an ANSI terminal editor using FlyKey, see <https://github.com/drewagislason/ned>
*/

/*************************************************************************
  Local types and defines
**************************************************************************/

typedef struct
{
  const uint8_t *pKeySeq;
  const char    *szName;
  uint8_t        len;
  flyKey_t       key;
} sFlyKeySeq_t;

typedef struct
{
  const char    *szName;
  flyKey_t       key;
} sFlyKeyNames_t;


static bool_t   FlyKeyInSequence            (flyKey_t *pKey);
static int      FlyKeyFindBytesInSeqArray   (int iLen, uint8_t *pKeySeq);
static int      FlyKeyIsFoundInSeqArray     (flyKey_t iKey);
static int      FlyKeyIsFoundInKeyNameArray (flyKey_t iKey);

// multi-byte key sequences (Mac OS X)
static const uint8_t mpFlyKeyBackTab[]        = { 27,91,90 };
static const uint8_t mpFlyKeyDelete[]         = { 27,91,51,126 };
static const uint8_t mpFlyKeyUp[]             = { 27,91,65 };
static const uint8_t mpFlyKeyDown[]           = { 27,91,66 };
static const uint8_t mpFlyKeyLeft[]           = { 27,91,68 };
static const uint8_t mpFlyKeyRight[]          = { 27,91,67 };
static const uint8_t mpFlyKeyHome[]           = { 27,79,72 };
static const uint8_t mpFlyKeyEnd[]            = { 27,79,70 };
static const uint8_t mpFlyKeyHome2[]          = { 27,91,49,126 };
static const uint8_t mpFlyKeyEnd2[]           = { 27,91,52,126 };
static const uint8_t mpFlyKeyPgUp[]           = { 27,91,53,126 };       // Esc[5~
static const uint8_t mpFlyKeyPgDn[]           = { 27,91,54,126 };       // Esc[6~
static const uint8_t mpFlyKeyCtrlUp[]         = { 27,91,49,59,53,65 };  // Esc[1;5A
static const uint8_t mpFlyKeyCtrlDown[]       = { 27,91,49,59,53,66 };  // Esc[1;5B
static const uint8_t mpFlyKeyCtrlLeft[]       = { 27,91,49,59,53,68 };
static const uint8_t mpFlyKeyCtrlRight[]      = { 27,91,49,59,53,67 };
static const uint8_t mpFlyKeyAltLeft[]        = { 27,98 };
static const uint8_t mpFlyKeyAltRight[]       = { 27,102 };
static const uint8_t mpFlyKeyShiftLeft[]      = { 27,91,49,59,50,68 }; 
static const uint8_t mpFlyKeyShiftRight[]     = { 27,91,49,59,50,67 };
static const uint8_t mpFlyKeyCtrlBackspace[]  = { 27,91,51,59,53,126 };
static const uint8_t mpFlyKeyFn1[]            = { 27,79,80 };
static const uint8_t mpFlyKeyFn2[]            = { 27,79,81 };
static const uint8_t mpFlyKeyFn3[]            = { 27,79,82 };
static const uint8_t mpFlyKeyFn4[]            = { 27,79,83 };
static const uint8_t mpFlyKeyFn5[]            = { 27,91,49,53,126 };
static const uint8_t mpFlyKeyFn1_Linux[]      = { 27,91,91,65 };
static const uint8_t mpFlyKeyFn2_Linux[]      = { 27,91,91,66 };
static const uint8_t mpFlyKeyFn3_Linux[]      = { 27,91,91,67 };
static const uint8_t mpFlyKeyFn4_Linux[]      = { 27,91,91,68 };
static const uint8_t mpFlyKeyFn5_Linux[]      = { 27,91,91,69 };
static const uint8_t mpFlyKeyFn6[]            = { 27,91,49,55,126 };
static const uint8_t mpFlyKeyFn7[]            = { 27,91,49,56,126 };
static const uint8_t mpFlyKeyFn8[]            = { 27,91,49,57,126 };
static const uint8_t mpFlyKeyFn9[]            = { 27,91,50,48,126 };
static const uint8_t mpFlyKeyFn10[]           = { 27,91,50,49,126 };
static const uint8_t mpFlyKeyFn11[]           = { 27,91,50,51,126 };
static const uint8_t mpFlyKeyFn12[]           = { 27,91,50,52,126 };
static const uint8_t mpFlyKeyEsc[]            = { 27,255 };
static const uint8_t mpFlyKeyEsc2[]           = { 27,27 };
static const uint8_t mpFlyKeyAltA[]           = { 195,165 };
static const uint8_t mpFlyKeyAltB[]           = { 226,136,171 };
static const uint8_t mpFlyKeyAltC[]           = { 195,167 };
static const uint8_t mpFlyKeyAltD[]           = { 226,136,130 };
static const uint8_t mpFlyKeyAltF[]           = { 198,146 };
static const uint8_t mpFlyKeyAltG[]           = { 194,169 };
static const uint8_t mpFlyKeyAltH[]           = { 203,153 };
static const uint8_t mpFlyKeyAltJ[]           = { 226,136,134 };
static const uint8_t mpFlyKeyAltK[]           = { 203,154 };
static const uint8_t mpFlyKeyAltL[]           = { 194,172 };
static const uint8_t mpFlyKeyAltM[]           = { 194,181 };
static const uint8_t mpFlyKeyAltO[]           = { 195,184 };
static const uint8_t mpFlyKeyAltP[]           = { 207,128 };
static const uint8_t mpFlyKeyAltQ[]           = { 197,147 };
static const uint8_t mpFlyKeyAltR[]           = { 194,174 };
static const uint8_t mpFlyKeyAltS[]           = { 195,159 };
static const uint8_t mpFlyKeyAltT[]           = { 226,128,160 };
static const uint8_t mpFlyKeyAltV[]           = { 226,136,154 };
static const uint8_t mpFlyKeyAltW[]           = { 226,136,145 };
static const uint8_t mpFlyKeyAltX[]           = { 226,137,136 };
static const uint8_t mpFlyKeyAltZ[]           = { 206,169 };
static const uint8_t mpFlyKeyAltA_2[]         = { 27,97 };
static const uint8_t mpFlyKeyAltB_2[]         = { 27,98 };
static const uint8_t mpFlyKeyAltC_2[]         = { 27,99 };
static const uint8_t mpFlyKeyAltD_2[]         = { 27,100 };
static const uint8_t mpFlyKeyAltE_2[]         = { 27,101 };
static const uint8_t mpFlyKeyAltF_2[]         = { 27,102 };
static const uint8_t mpFlyKeyAltG_2[]         = { 27,103 };
static const uint8_t mpFlyKeyAltH_2[]         = { 27,104 };
static const uint8_t mpFlyKeyAltI_2[]         = { 27,105 };
static const uint8_t mpFlyKeyAltJ_2[]         = { 27,106 };
static const uint8_t mpFlyKeyAltK_2[]         = { 27,107 };
static const uint8_t mpFlyKeyAltL_2[]         = { 27,108 };
static const uint8_t mpFlyKeyAltM_2[]         = { 27,109 };
static const uint8_t mpFlyKeyAltN_2[]         = { 27,110 };
static const uint8_t mpFlyKeyAltO_2[]         = { 27,111 };
static const uint8_t mpFlyKeyAltP_2[]         = { 27,112 };
static const uint8_t mpFlyKeyAltQ_2[]         = { 27,113 };
static const uint8_t mpFlyKeyAltR_2[]         = { 27,114 };
static const uint8_t mpFlyKeyAltS_2[]         = { 27,115 };
static const uint8_t mpFlyKeyAltT_2[]         = { 27,116 };
static const uint8_t mpFlyKeyAltU_2[]         = { 27,117 };
static const uint8_t mpFlyKeyAltV_2[]         = { 27,118 };
static const uint8_t mpFlyKeyAltW_2[]         = { 27,119 };
static const uint8_t mpFlyKeyAltX_2[]         = { 27,120 };
static const uint8_t mpFlyKeyAltY_2[]         = { 27,121 };
static const uint8_t mpFlyKeyAltZ_2[]         = { 27,122 };
static const uint8_t mpFlyKeyAltDash[]        = { 226,128,147 };
static const uint8_t mpFlyKeyAltEqual[]       = { 226,137,160 };
static const uint8_t mpFlyKeyAltLeftBrace[]   = { 226,128,156 };
static const uint8_t mpFlyKeyAltRightBrace[]  = { 226,128,152 };
static const uint8_t mpFlyKeyAltBackslash[]   = { 194,171 };
static const uint8_t mpFlyKeyAltSpace[]       = { 194, 160 };
static const uint8_t mpFlyKeyAltColon[]       = { 226,128,166 };
static const uint8_t mpFlyKeyAltQuote[]       = { 195,166 };
static const uint8_t mpFlyKeyAltComma[]       = { 226,137,164 };
static const uint8_t mpFlyKeyAltPeriod[]      = { 226,137,165 };
static const uint8_t mpFlyKeyAltSlash[]       = { 195,183 };


static const sFlyKeySeq_t maFlyKeySequences[] =
{
  { mpFlyKeyUp, (const char *)"Up", sizeof(mpFlyKeyUp), FLY_KEY_UP },
  { mpFlyKeyDown, (const char *)"Down", sizeof(mpFlyKeyDown), FLY_KEY_DOWN },
  { mpFlyKeyLeft, (const char *)"Left", sizeof(mpFlyKeyDown), FLY_KEY_LEFT },
  { mpFlyKeyRight, (const char *)"Right", sizeof(mpFlyKeyDown), FLY_KEY_RIGHT },
  { mpFlyKeyHome, (const char *)"Home", sizeof(mpFlyKeyHome), FLY_KEY_HOME },
  { mpFlyKeyEnd, (const char *)"End", sizeof(mpFlyKeyEnd), FLY_KEY_END },
  { mpFlyKeyHome2, (const char *)"Home", sizeof(mpFlyKeyHome2), FLY_KEY_HOME },
  { mpFlyKeyEnd2, (const char *)"End", sizeof(mpFlyKeyEnd2), FLY_KEY_END },
  { mpFlyKeyPgUp, (const char *)"PgUp", sizeof(mpFlyKeyPgUp), FLY_KEY_PGUP },
  { mpFlyKeyPgDn, (const char *)"PgDn", sizeof(mpFlyKeyPgDn), FLY_KEY_PGDN },
  { mpFlyKeyCtrlUp,(const char *)"Ctrl-Up", sizeof(mpFlyKeyCtrlUp), FLY_KEY_CTRL_UP },
  { mpFlyKeyCtrlDown,(const char *)"Ctrl-Down", sizeof(mpFlyKeyCtrlDown), FLY_KEY_CTRL_DOWN },
  { mpFlyKeyCtrlLeft,(const char *)"Ctrl-Left", sizeof(mpFlyKeyCtrlLeft), FLY_KEY_CTRL_LEFT },
  { mpFlyKeyCtrlRight,(const char *)"Ctrl-Right", sizeof(mpFlyKeyCtrlRight), FLY_KEY_CTRL_RIGHT },
  { mpFlyKeyAltLeft,(const char *)"Alt-Left", sizeof(mpFlyKeyAltLeft), FLY_KEY_ALT_LEFT },
  { mpFlyKeyAltRight,(const char *)"Alt-Right", sizeof(mpFlyKeyAltRight), FLY_KEY_ALT_RIGHT },
  { mpFlyKeyShiftLeft,(const char *)"Shift-Left", sizeof(mpFlyKeyShiftLeft), FLY_KEY_SHIFT_LEFT },
  { mpFlyKeyShiftRight,(const char *)"Shift-Right", sizeof(mpFlyKeyShiftRight), FLY_KEY_SHIFT_RIGHT },
  { mpFlyKeyCtrlBackspace,(const char *)"Ctrl-Backspace", sizeof(mpFlyKeyCtrlBackspace), FLY_KEY_CTRL_BACKSPACE },
  { mpFlyKeyEsc,  (const char *)"Esc",   sizeof(mpFlyKeyEsc), FLY_KEY_ESC },
  { mpFlyKeyEsc2, (const char *)"Esc",   sizeof(mpFlyKeyEsc2), FLY_KEY_ESC },
  { mpFlyKeyBackTab, (const char *)"Backtab", sizeof(mpFlyKeyBackTab), FLY_KEY_BACK_TAB },
  { mpFlyKeyDelete, (const char *)"Delete", sizeof(mpFlyKeyDelete), FLY_KEY_DELETE },
  { mpFlyKeyAltDash, (const char *)"Alt--", sizeof(mpFlyKeyAltDash), FLY_KEY_ALT_DASH },
  { mpFlyKeyAltEqual, (const char *)"Alt-=", sizeof(mpFlyKeyAltEqual), FLY_KEY_ALT_EQUAL },
  { mpFlyKeyAltLeftBrace, (const char *)"Alt-[", sizeof(mpFlyKeyAltLeftBrace), FLY_KEY_ALT_LEFT_BRACE },
  { mpFlyKeyAltRightBrace, (const char *)"Alt-]", sizeof(mpFlyKeyAltLeftBrace), FLY_KEY_ALT_RIGHT_BRACE },
  { mpFlyKeyAltBackslash, (const char *)"Alt-\\", sizeof(mpFlyKeyAltBackslash), FLY_KEY_ALT_BACKSLASH },
  { mpFlyKeyAltSpace, (const char *)"Alt-Space", sizeof(mpFlyKeyAltSpace), FLY_KEY_ALT_SPACE },
  { mpFlyKeyAltColon, (const char *)"Alt-;", sizeof(mpFlyKeyAltColon), FLY_KEY_ALT_COLON },
  { mpFlyKeyAltQuote, (const char *)"Alt-'", sizeof(mpFlyKeyAltQuote), FLY_KEY_ALT_QUOTE },
  { mpFlyKeyAltComma, (const char *)"Alt-,", sizeof(mpFlyKeyAltComma), FLY_KEY_ALT_COMMA },
  { mpFlyKeyAltPeriod, (const char *)"Alt-.", sizeof(mpFlyKeyAltPeriod), FLY_KEY_ALT_PERIOD },
  { mpFlyKeyAltSlash, (const char *)"Alt-/", sizeof(mpFlyKeyAltSlash), FLY_KEY_ALT_SLASH },
  { mpFlyKeyFn1, (const char *)"Fn1", sizeof(mpFlyKeyFn1), FLY_KEY_FN1 },
  { mpFlyKeyFn2, (const char *)"Fn2", sizeof(mpFlyKeyFn2), FLY_KEY_FN2 },
  { mpFlyKeyFn3, (const char *)"Fn3", sizeof(mpFlyKeyFn3), FLY_KEY_FN3 },
  { mpFlyKeyFn4, (const char *)"Fn4", sizeof(mpFlyKeyFn4), FLY_KEY_FN4 },
  { mpFlyKeyFn5, (const char *)"Fn5", sizeof(mpFlyKeyFn5), FLY_KEY_FN5 },
  { mpFlyKeyFn1_Linux, (const char *)"Fn1", sizeof(mpFlyKeyFn1_Linux), FLY_KEY_FN1 },
  { mpFlyKeyFn2_Linux, (const char *)"Fn2", sizeof(mpFlyKeyFn2_Linux), FLY_KEY_FN2 },
  { mpFlyKeyFn3_Linux, (const char *)"Fn3", sizeof(mpFlyKeyFn3_Linux), FLY_KEY_FN3 },
  { mpFlyKeyFn4_Linux, (const char *)"Fn4", sizeof(mpFlyKeyFn4_Linux), FLY_KEY_FN4 },
  { mpFlyKeyFn5_Linux, (const char *)"Fn5", sizeof(mpFlyKeyFn5_Linux), FLY_KEY_FN5 },
  { mpFlyKeyFn6, (const char *)"Fn6", sizeof(mpFlyKeyFn6), FLY_KEY_FN6 },
  { mpFlyKeyFn7, (const char *)"Fn7", sizeof(mpFlyKeyFn7), FLY_KEY_FN7 },
  { mpFlyKeyFn8, (const char *)"Fn8", sizeof(mpFlyKeyFn8), FLY_KEY_FN8 },
  { mpFlyKeyFn9, (const char *)"Fn9", sizeof(mpFlyKeyFn9), FLY_KEY_FN9 },
  { mpFlyKeyFn10, (const char *)"Fn10", sizeof(mpFlyKeyFn10), FLY_KEY_FN10 },
  { mpFlyKeyFn11, (const char *)"Fn11", sizeof(mpFlyKeyFn11), FLY_KEY_FN11 },
  { mpFlyKeyFn12, (const char *)"Fn12", sizeof(mpFlyKeyFn12), FLY_KEY_FN12 },
  { mpFlyKeyAltA, (const char *)"Alt-A", sizeof(mpFlyKeyAltA), FLY_KEY_ALT_A },
  { mpFlyKeyAltB, (const char *)"Alt-B", sizeof(mpFlyKeyAltB), FLY_KEY_ALT_B },
  { mpFlyKeyAltC, (const char *)"Alt-C", sizeof(mpFlyKeyAltC), FLY_KEY_ALT_C },
  { mpFlyKeyAltD, (const char *)"Alt-D", sizeof(mpFlyKeyAltD), FLY_KEY_ALT_D },
  { mpFlyKeyAltF, (const char *)"Alt-F", sizeof(mpFlyKeyAltF), FLY_KEY_ALT_F },
  { mpFlyKeyAltG, (const char *)"Alt-G", sizeof(mpFlyKeyAltG), FLY_KEY_ALT_G },
  { mpFlyKeyAltH, (const char *)"Alt-H", sizeof(mpFlyKeyAltH), FLY_KEY_ALT_H },
  { mpFlyKeyAltJ, (const char *)"Alt-J", sizeof(mpFlyKeyAltJ), FLY_KEY_ALT_J },
  { mpFlyKeyAltK, (const char *)"Alt-K", sizeof(mpFlyKeyAltK), FLY_KEY_ALT_K },
  { mpFlyKeyAltL, (const char *)"Alt-L", sizeof(mpFlyKeyAltL), FLY_KEY_ALT_L },
  { mpFlyKeyAltM, (const char *)"Alt-M", sizeof(mpFlyKeyAltM), FLY_KEY_ALT_M },
  { mpFlyKeyAltO, (const char *)"Alt-O", sizeof(mpFlyKeyAltO), FLY_KEY_ALT_O },
  { mpFlyKeyAltP, (const char *)"Alt-P", sizeof(mpFlyKeyAltP), FLY_KEY_ALT_P },
  { mpFlyKeyAltQ, (const char *)"Alt-Q", sizeof(mpFlyKeyAltQ), FLY_KEY_ALT_Q },
  { mpFlyKeyAltR, (const char *)"Alt-R", sizeof(mpFlyKeyAltR), FLY_KEY_ALT_R },
  { mpFlyKeyAltS, (const char *)"Alt-S", sizeof(mpFlyKeyAltS), FLY_KEY_ALT_S },
  { mpFlyKeyAltT, (const char *)"Alt-T", sizeof(mpFlyKeyAltT), FLY_KEY_ALT_T },
  { mpFlyKeyAltV, (const char *)"Alt-V", sizeof(mpFlyKeyAltV), FLY_KEY_ALT_V },
  { mpFlyKeyAltW, (const char *)"Alt-W", sizeof(mpFlyKeyAltW), FLY_KEY_ALT_W },
  { mpFlyKeyAltX, (const char *)"Alt-X", sizeof(mpFlyKeyAltX), FLY_KEY_ALT_X },
  { mpFlyKeyAltZ, (const char *)"Alt-Z", sizeof(mpFlyKeyAltZ), FLY_KEY_ALT_Z },
  { mpFlyKeyAltA_2, (const char *)"Alt-A", sizeof(mpFlyKeyAltA_2), FLY_KEY_ALT_A },
  { mpFlyKeyAltB_2, (const char *)"Alt-B", sizeof(mpFlyKeyAltB_2), FLY_KEY_ALT_B },
  { mpFlyKeyAltC_2, (const char *)"Alt-C", sizeof(mpFlyKeyAltC_2), FLY_KEY_ALT_C },
  { mpFlyKeyAltD_2, (const char *)"Alt-D", sizeof(mpFlyKeyAltD_2), FLY_KEY_ALT_D },
  { mpFlyKeyAltE_2, (const char *)"Alt-E", sizeof(mpFlyKeyAltE_2), FLY_KEY_ALT_E },
  { mpFlyKeyAltF_2, (const char *)"Alt-F", sizeof(mpFlyKeyAltF_2), FLY_KEY_ALT_F },
  { mpFlyKeyAltG_2, (const char *)"Alt-G", sizeof(mpFlyKeyAltG_2), FLY_KEY_ALT_G },
  { mpFlyKeyAltH_2, (const char *)"Alt-H", sizeof(mpFlyKeyAltH_2), FLY_KEY_ALT_H },
  { mpFlyKeyAltI_2, (const char *)"Alt-I", sizeof(mpFlyKeyAltI_2), FLY_KEY_ALT_I },
  { mpFlyKeyAltJ_2, (const char *)"Alt-J", sizeof(mpFlyKeyAltJ_2), FLY_KEY_ALT_J },
  { mpFlyKeyAltK_2, (const char *)"Alt-K", sizeof(mpFlyKeyAltK_2), FLY_KEY_ALT_K },
  { mpFlyKeyAltL_2, (const char *)"Alt-L", sizeof(mpFlyKeyAltL_2), FLY_KEY_ALT_L },
  { mpFlyKeyAltM_2, (const char *)"Alt-M", sizeof(mpFlyKeyAltM_2), FLY_KEY_ALT_M },
  { mpFlyKeyAltN_2, (const char *)"Alt-N", sizeof(mpFlyKeyAltN_2), FLY_KEY_ALT_N },
  { mpFlyKeyAltO_2, (const char *)"Alt-O", sizeof(mpFlyKeyAltO_2), FLY_KEY_ALT_O },
  { mpFlyKeyAltP_2, (const char *)"Alt-P", sizeof(mpFlyKeyAltP_2), FLY_KEY_ALT_P },
  { mpFlyKeyAltQ_2, (const char *)"Alt-Q", sizeof(mpFlyKeyAltQ_2), FLY_KEY_ALT_Q },
  { mpFlyKeyAltR_2, (const char *)"Alt-R", sizeof(mpFlyKeyAltR_2), FLY_KEY_ALT_R },
  { mpFlyKeyAltS_2, (const char *)"Alt-S", sizeof(mpFlyKeyAltS_2), FLY_KEY_ALT_S },
  { mpFlyKeyAltT_2, (const char *)"Alt-T", sizeof(mpFlyKeyAltT_2), FLY_KEY_ALT_T },
  { mpFlyKeyAltU_2, (const char *)"Alt-U", sizeof(mpFlyKeyAltU_2), FLY_KEY_ALT_U },
  { mpFlyKeyAltV_2, (const char *)"Alt-V", sizeof(mpFlyKeyAltV_2), FLY_KEY_ALT_V },
  { mpFlyKeyAltW_2, (const char *)"Alt-W", sizeof(mpFlyKeyAltW_2), FLY_KEY_ALT_W },
  { mpFlyKeyAltX_2, (const char *)"Alt-X", sizeof(mpFlyKeyAltX_2), FLY_KEY_ALT_X },
  { mpFlyKeyAltY_2, (const char *)"Alt-Y", sizeof(mpFlyKeyAltY_2), FLY_KEY_ALT_Y },
  { mpFlyKeyAltZ_2, (const char *)"Alt-Z", sizeof(mpFlyKeyAltZ_2), FLY_KEY_ALT_Z }
};

// special keynames checked first (e.g. "Enter" instead of "Ctrl-M")
static const sFlyKeyNames_t maFlyKeyNames[] =
{
  { (const char *)"Ctrl-Space", FLY_KEY_CTRL_SPACE },
  { (const char *)"Esc",        FLY_KEY_ESC },
  { (const char *)"Tab",        FLY_KEY_TAB },
  { (const char *)"Enter",      FLY_KEY_ENTER },
  { (const char *)"Ctrl-\\",    FLY_KEY_CTRL_BACKSLASH },
  { (const char *)"Ctrl-]",     FLY_KEY_CTRL_RIGHT_BRACE },
  { (const char *)"Ctrl-^",     FLY_KEY_CTRL_CAROT },
  { (const char *)"Ctrl--",     FLY_KEY_CTRL_MINUS },
  { (const char *)"Backspace",  FLY_KEY_BACKSPACE },
  { (const char *)"Space",      (flyKey_t)' ' },
  { (const char *)"Ctrl-Home",  FLY_KEY_CTRL_HOME },
  { (const char *)"Ctrl-End",   FLY_KEY_CTRL_END },
  { (const char *)"Ctrl-PgUp",  FLY_KEY_CTRL_PGUP },
  { (const char *)"Ctrl-PgDn",  FLY_KEY_CTRL_PGDN },
  { (const char *)"Alt-Up",     FLY_KEY_ALT_UP },
  { (const char *)"Alt-Down",   FLY_KEY_ALT_DOWN },
  { (const char *)"Alt-Home",   FLY_KEY_ALT_HOME },
  { (const char *)"Alt-End",    FLY_KEY_ALT_END },
  { (const char *)"Alt-PgUp",   FLY_KEY_ALT_PGUP },
  { (const char *)"Alt-PgDn",   FLY_KEY_ALT_PGDN },
  { (const char *)"Alt-E",      FLY_KEY_ALT_E },
  { (const char *)"Alt-I",      FLY_KEY_ALT_I },
  { (const char *)"Alt-N",      FLY_KEY_ALT_N },
  { (const char *)"Alt-U",      FLY_KEY_ALT_U },
  { (const char *)"Alt-Y",      FLY_KEY_ALT_Y },
  { (const char *)"None",       FLY_KEY_NONE }
};

static int                miNedInKeySeq;
static uint8_t            maFlyKeySeqFound[6];      // maximum of 6 sequences in key
static pfnFlyKeyIdle_t    m_pfnIdle       = NULL;
static void              *m_pIdleData     = NULL;

static bool_t             m_fMacroRecording;
static unsigned           m_MacroIndex;
static unsigned           m_MacroNumKeys;
static flyKey_t           m_aMacroKeys[FLY_KEY_MACRO_MAX];

static bool_t             m_fRawEnabled;
static struct termios     m_oldTerm;
static struct termios     m_newTerm;

/*!------------------------------------------------------------------------------------------------
  Sets the idle function for when keyboard is idle. Useful for idle processing.

  The parameter `pData` must be persistent, as it's the data that is passed to the idle function.

  The idle function `pfnIdle` must be in the form of:

  ```c
  bool_t IdleFunction(void *pData);
  ```

  @param    pfnIdle   idle function, or NULL to turn off
  @param    pData     any peristent app data associated with idle
  @return   TRUE if the FLY_KEY_IDLE should be returned on idle
*///-----------------------------------------------------------------------------------------------
void FlyKeySetIdle(pfnFlyKeyIdle_t pfnIdle, void *pData)
{
  m_pfnIdle = pfnIdle;
  m_pIdleData = pData;
}

/*!------------------------------------------------------------------------------------------------
  Return TRUE if playing back the macro

  @return   TRUE if playing back the macro
*///-----------------------------------------------------------------------------------------------
bool_t FlyKeyInMacro(void)
{
  return (m_MacroIndex < m_MacroNumKeys) ? TRUE : FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Currently recording the macro

  @return   TRUE if recording
*///-----------------------------------------------------------------------------------------------
bool_t FlyKeyMacroRecording(void)
{
  return m_fMacroRecording;
}

/*!------------------------------------------------------------------------------------------------
  Play the macro (if any)

  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyKeyMacroPlay(void)
{
  if(FlyKeyMacroRecording())
    FlyKeyMacroEndRecord();
  m_MacroIndex = 0;
}

/*!------------------------------------------------------------------------------------------------
  Brings the macro state machine back to factory condition (nothing recorded, everything stopped

  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyKeyMacroClear(void)
{
  m_MacroIndex = m_MacroNumKeys = 0;
  m_fMacroRecording = FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Record the macro

  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyKeyMacroRecord(void)
{
  FlyKeyMacroClear();
  m_fMacroRecording = TRUE;
}

/*!------------------------------------------------------------------------------------------------
  Record the macro

  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyKeyMacroEndRecord(void)
{
  // whatever ended the record, don't include in the list of keys
  if(m_MacroNumKeys)
    --m_MacroNumKeys;
  m_MacroIndex = m_MacroNumKeys;
  m_fMacroRecording = FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Add a key to the macro sequence. Playback remains stopped.

  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyKeyMacroAddKey(flyKey_t key)
{
  if(m_MacroNumKeys < FLY_KEY_MACRO_MAX)
  {
    m_aMacroKeys[m_MacroNumKeys] = key;
    ++m_MacroNumKeys;
    m_MacroIndex = m_MacroNumKeys;
  }
}

/*!------------------------------------------------------------------------------------------------
  Get the current macro key sequence

  @param    pNumKeys    pointer to returned # of keys
  @return   a pointer to the key sequence and the number of keys
*///-----------------------------------------------------------------------------------------------
const flyKey_t *FlyKeyMacroGet(unsigned *pNumKeys)
{
  *pNumKeys = m_MacroNumKeys;
  return m_aMacroKeys;
}

/*!------------------------------------------------------------------------------------------------
  Gets a key from either the macro system (if playing back), or from the raw terminal.

  @return   key code (0-255). See ::flyKey_t
*///-----------------------------------------------------------------------------------------------
flyKey_t FlyKeyGetKey(void)
{
  flyKey_t  key;
  bool_t    fInSeq = FALSE;

  // play the macro
  if(FlyKeyInMacro())
  {
    key = m_aMacroKeys[m_MacroIndex];
    ++m_MacroIndex;
  }

  else
  {
    // keep waiting until an actual key
    do
    {
      miNedInKeySeq = 0;

      // pull the whole sequence out of the keyboard buffer
      do
      {
        // key may be part of a sequence (e.g. up arrow key is 27,91,65)
        // if no key, returns FLY_KEY_NONE
        key = FlyKeyRawGetKey();

        // none cannot be the 1st part of sequence, but can be last part
        if((key != FLY_KEY_NONE) || miNedInKeySeq)
        {
          // printf(" (key = %d) ", key);
          // fflush(stdout);
          fInSeq = FlyKeyInSequence(&key);
        }

      } while( fInSeq );

      // if user installed an idle function... See FlyKeySetIdle()
      if(m_pfnIdle && key == FLY_KEY_NONE)
      {
        // user returns TRUE to pass the idle key up
        if((*m_pfnIdle)(m_pIdleData))
        {
          key = FLY_KEY_IDLE;
          break;
        }
      }

    } while(key == FLY_KEY_NONE);

    // if recording, add the key
    if((key != FLY_KEY_IDLE) && FlyKeyMacroRecording())
      FlyKeyMacroAddKey(key);
  }

  return key;
}

/*!------------------------------------------------------------------------------------------------
  Gets name of the Ned Key in string form (e.g "Ctrl-A" or "Fn-12").

  Note: the name is built on the fly, so copy the string if it must be persistent or when using
  multithreading.

  If the key code is not valid, returns an empty string "".

  @param    key     The key code
  @return   string name of key code, or "" if not a valid key code
*///-----------------------------------------------------------------------------------------------
const char * FlyKeyName(flyKey_t key)
{
  static char   szKeyName[12];
  const  char  *pszKeyName;
  int           i;

  // assume we must build key name
  pszKeyName    = (const char *)szKeyName;
  szKeyName[0]  = 0;

  // is keyname in the sequence array? use it
  i = FlyKeyIsFoundInSeqArray(key);
  if( i != -1 )
  {
    pszKeyName = maFlyKeySequences[i].szName;
  }
  else
  {
    // is keyname in the keyname array? use it
    i = FlyKeyIsFoundInKeyNameArray(key);
    if( i != -1 )
    {
      pszKeyName = maFlyKeyNames[i].szName;
    }

    // keyname not in either array, build it
    else
    {
      if( (key >= FLY_KEY_CTRL_A) && (key <= FLY_KEY_CTRL_Z) )
      {
        sprintf(szKeyName, "Ctrl-%c", 'A' + (key - FLY_KEY_CTRL_A));
      }
      else if( (key >= ' ') && (key <= '~') )
      {
        sprintf(szKeyName, "%c", key);
      }
    }
  }

  return pszKeyName;
}

/*!----------------------------------------------------------------------------------
  Return the nedKey code that matches the name, or FLY_KEY_NONE if no match.

  @param[in]    szName     The key code.
  @return       The nedKey that matches the name. See ::flyKey_t, or FLY_KEY_NONE
*///---------------------------------------------------------------------------------
flyKey_t FlyKeyFromName(const char *szName)
{
  static char   szKeyName[8];
  flyKey_t      key     = FLY_KEY_NONE;
  bool_t        fFound  = FALSE;
  unsigned      i;

  if(szName != NULL)
  {
    // first check the key sequences
    for(i=0; i<NumElements(maFlyKeySequences); ++i)
    {
      if( strcmp(maFlyKeySequences[i].szName, szName) == 0 )
      {
        key    = maFlyKeySequences[i].key;
        fFound = TRUE;
        break;
      }
    }

    // next check the keyname array
    if(!fFound)
    {
      for(i=0; i<NumElements(maFlyKeyNames); ++i)
      {
        if( strcmp(maFlyKeyNames[i].szName, szName) == 0 )
        {
          key    = maFlyKeyNames[i].key;
          fFound = TRUE;
          break;
        }
      }
    }

    // ctrl keys
    if(!fFound)
    {
      for(i=FLY_KEY_CTRL_A; i<= FLY_KEY_CTRL_Z; ++i)
      {
        sprintf(szKeyName, "Ctrl-%c", (char)('A'+(i-1)));
        if( strcmp(szKeyName, szName) == 0 )
        {
          key    = (flyKey_t)i;
          fFound = TRUE;
          break;
        }
      }
    }

    // printable ascii keys
    if(!fFound)
    {
      for(i=' '; i<= '~'; ++i)
      {
        sprintf(szKeyName, "%c", (char)i);
        if( strcmp(szKeyName, szName) == 0 )
        {
          key    = (flyKey_t)i;
          fFound = TRUE;
          break;
        }
      }
    }
  }

  return key;
}

/*-------------------------------------------------------------------------------------------------
  Is this key found in the sequence array?
-------------------------------------------------------------------------------------------------*/
static bool_t FlyKeyInSequence(flyKey_t *pKey)
{
  int i;

  // look for this key sequence in the array
  maFlyKeySeqFound[miNedInKeySeq++] = *pKey;
  i = FlyKeyFindBytesInSeqArray(miNedInKeySeq, maFlyKeySeqFound);
  if(i != -1 )
  {
    if(maFlyKeySequences[i].len == miNedInKeySeq)
    {
      *pKey = maFlyKeySequences[i].key;
      return FALSE;
    }
    return TRUE;
  }
  return FALSE;
}

/*-------------------------------------------------------------------------
  Is this key found in the sequence array?
-------------------------------------------------------------------------*/
static int FlyKeyFindBytesInSeqArray(int len, uint8_t *pKeySeq)
{
  int i;

  // printf("NedFindBytesInSeqArray(len %d) ", len);
  for(i=0; i<NumElements(maFlyKeySequences); ++i)
  {
    if( (maFlyKeySequences[i].len >= len) && 
        (memcmp(pKeySeq, maFlyKeySequences[i].pKeySeq, len) == 0) )
    {
      // printf("  found %d\r\n", i);
      // fflush(stdout);
      return i; // found a match, return the index
    }
  }
  // printf("  not found\r\n");
  // fflush(stdout);
  return -1;  // no match
}

/*-------------------------------------------------------------------------
  Is this key found in the sequence array?
-------------------------------------------------------------------------*/
static int FlyKeyIsFoundInSeqArray(flyKey_t key)
{
  int i;
  for(i=0; i<NumElements(maFlyKeySequences); ++i)
  {
    if(maFlyKeySequences[i].key == key)
      return i;
  }
  return -1;
}

/*-------------------------------------------------------------------------
  Is this key found in the keyname array?
-------------------------------------------------------------------------*/
static int FlyKeyIsFoundInKeyNameArray(flyKey_t key)
{
  int i;
  for(i=0; i<NumElements(maFlyKeyNames); ++i)
  {
    if(maFlyKeyNames[i].key == key)
      return i;
  }
  return -1;
}

/*!------------------------------------------------------------------------------------------------
  enable raw mode if not already enabled.

  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyKeyRawEnable(void)
{
  if(!m_fRawEnabled)
  {
    m_fRawEnabled = TRUE;

    tcgetattr(STDIN_FILENO, &m_oldTerm);
    m_newTerm = m_oldTerm;
    cfmakeraw(&m_newTerm);
    // new.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
    //                         | INLCR  | IGNCR  | ICRNL | IXON);
    // new.c_oflag &= ~OPOST;
    // new.c_lflag &= ~(ECHO   | ECHONL | ICANON | ISIG  | IEXTEN);
    // new.c_cflag &= ~(CSIZE  | PARENB);
    // new.c_cflag |= CS8;
    m_newTerm.c_cc[VMIN] = 0;
    m_newTerm.c_cc[VTIME] = 1;
    tcsetattr(0, TCSADRAIN, &m_newTerm);

  }
}

/*!------------------------------------------------------------------------------------------------
  Go back to original terminal settings. Do this if using FlyKeyRawGetKey() or FlyKeyRawEnable()
  before exiting the programing.

  As the keyboard is places in raw mode, make sure to call FlyKeyRawDisable() before exiting your

  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyKeyRawDisable(void)
{
  if(m_fRawEnabled)
  {
    m_fRawEnabled = FALSE;
    tcsetattr(STDIN_FILENO, TCSANOW, &m_oldTerm);
  }
}

/*!------------------------------------------------------------------------------------------------
  Returns the raw key code from the read() function, one byte at a time. Waits only 1/10th of a
  second and, if no key is available, returns psudo key FLY_KEY_NONE.

  This places the console in raw mode, make sure to call FlyKeyRawDisable() before exiting your
  program. When raw mode is enabled, you need to use \r\n with printf().

  Some keys are multi-byte, for examples arrows. Since some of the multi-byte sequences begin with
  Esc (27), the only way to tell if it's an Esc or an arrow is if a FLY_KEY_NONE occurs after the
  Esc key. See also ::flyKey_t.

  @return   FLY_KEY_NONE or key value
*///-----------------------------------------------------------------------------------------------
flyKey_t FlyKeyRawGetKey(void)
{
  static uint8_t    aBuf[10];
  static unsigned   i     = 0;
  static unsigned   len   = 0;
  uint8_t           key   = FLY_KEY_NONE;

  // will only enabled not already enabled (set to wait 1/10th second)

  if(i >= len)
  {
    i   = 0;
    fflush(stdout);
    FlyKeyRawEnable();
    len = read(STDIN_FILENO, aBuf, sizeof(aBuf));
    FlyKeyRawDisable();
  }

  // len may be 0 after read, or this may be from a previous read
  if(i < len)
  {
    key = aBuf[i];
    ++i;
  }

  return key;
}
