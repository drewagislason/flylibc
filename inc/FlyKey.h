/**************************************************************************************************
  FlyKey.h
  Copyright 2024 Drew Gislason
  license: MIT <https://mit-license.org>
*///***********************************************************************************************
#ifndef FLY_KEY_H
#define FLY_KEY_H
#include "Fly.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

typedef unsigned char flyKey_t;

// control keys
#define FLY_KEY_CTRL_SPACE        0
#define FLY_KEY_CTRL_A            1
#define FLY_KEY_CTRL_B            2
#define FLY_KEY_CTRL_C            3
#define FLY_KEY_CTRL_D            4
#define FLY_KEY_CTRL_E            5
#define FLY_KEY_CTRL_F            6
#define FLY_KEY_CTRL_G            7
#define FLY_KEY_CTRL_H            8
#define FLY_KEY_CTRL_I            9
#define FLY_KEY_TAB               9  // same as Ctrl-I
#define FLY_KEY_CTRL_J           10
#define FLY_KEY_LF               10
#define FLY_KEY_CTRL_K           11
#define FLY_KEY_CTRL_L           12
#define FLY_KEY_CTRL_M           13
#define FLY_KEY_CR               13  // same as Ctrl-M
#define FLY_KEY_ENTER            13  // same as Ctrl-M
#define FLY_KEY_CTRL_N           14
#define FLY_KEY_CTRL_O           15
#define FLY_KEY_CTRL_P           16
#define FLY_KEY_CTRL_Q           17
#define FLY_KEY_CTRL_R           18
#define FLY_KEY_CTRL_S           19
#define FLY_KEY_CTRL_T           20
#define FLY_KEY_CTRL_U           21
#define FLY_KEY_CTRL_V           22
#define FLY_KEY_CTRL_W           23
#define FLY_KEY_CTRL_X           24
#define FLY_KEY_CTRL_Y           25
#define FLY_KEY_CTRL_Z           26
#define FLY_KEY_ESC              27  // Esc, Ctrl-[
#define FLY_KEY_MENU             27  // Same as Esc
#define FLY_KEY_CTRL_BACKSLASH   28  // Ctrl-backspace
#define FLY_KEY_CTRL_RIGHT_BRACE 29  // Ctrl-]
#define FLY_KEY_CTRL_CAROT       30  // Ctrl-^
#define FLY_KEY_CTRL_MINUS       31  // Ctrl-Minus
#define FLY_KEY_CTRL_MIN         FLY_KEY_CTRL_SPACE
#define FLY_KEY_CTRL_MAX         FLY_KEY_CTRL_MINUS

// ascii keys
#define FLY_KEY_ASCII_MIN        ((flyKey_t)' ')
#define FLY_KEY_ASCII_MAX        ((flyKey_t)'~')

// backspace
#define FLY_KEY_BACKSPACE       127  // (Fn-delete on MAC)

// cursor keys
#define FLY_KEY_UP              128  // 27,91,65
#define FLY_KEY_DOWN            129  // 27,91,66
#define FLY_KEY_LEFT            130  // 27,91,68
#define FLY_KEY_RIGHT           131  // 27,91,67
#define FLY_KEY_HOME            132  // 27,79,72     (pseudo key on Mac OS X)
#define FLY_KEY_END             133  // 27,79,70     (pseudo key on Mac OS X)
#define FLY_KEY_PGUP            134  // 27,91,53,126 (pseudo key on Mac OS X)
#define FLY_KEY_PGDN            135  // 27,91,54,126 (pseudo key on Mac OS X)
#define FLY_KEY_CTRL_UP         136  // 27, 91, 49, 59, 53, 65
#define FLY_KEY_CTRL_DOWN       137  // 27, 91, 49, 59, 53, 66
#define FLY_KEY_CTRL_LEFT       138  // 27, 91, 49, 59, 53, 68
#define FLY_KEY_CTRL_RIGHT      139  // 27, 91, 49, 59, 53, 67
#define FLY_KEY_CTRL_HOME       140  // psuedo key
#define FLY_KEY_CTRL_END        141  // psuedo key
#define FLY_KEY_CTRL_PGUP       142  // psuedo key
#define FLY_KEY_CTRL_PGDN       143  // psuedo key
#define FLY_KEY_ALT_UP          144  // psuedo key
#define FLY_KEY_ALT_DOWN        145  // psuedo key
#define FLY_KEY_ALT_LEFT        146  // 27, 98
#define FLY_KEY_ALT_RIGHT       147  // 27, 102
#define FLY_KEY_ALT_HOME        148  // psuedo key
#define FLY_KEY_ALT_END         149  // psuedo key
#define FLY_KEY_ALT_PGUP        150  // psuedo key
#define FLY_KEY_ALT_PGDN        151  // psuedo key
#define FLY_KEY_SHIFT_LEFT      152  // 27,91,49,59,50,68
#define FLY_KEY_SHIFT_RIGHT     153  // 27,91,49,59,50,67
#define FLY_KEY_CTRL_BACKSPACE  154  // 27,91,51,59,53,126

// function keys
#define FLY_KEY_FN1             160  // 27, 79, 80
#define FLY_KEY_FN2             161  // 27, 79, 81
#define FLY_KEY_FN3             162  // 27, 79, 82
#define FLY_KEY_FN4             163  // 27, 79, 83
#define FLY_KEY_FN5             164  // 27, 91, 49, 53, 126
#define FLY_KEY_FN6             165  // 27, 91, 49, 55, 126
#define FLY_KEY_FN7             166  // 27, 91, 49, 56, 126
#define FLY_KEY_FN8             167  // 27, 91, 49, 57, 126
#define FLY_KEY_FN9             168  // 27, 91, 50, 48, 126
#define FLY_KEY_FN10            169  // 27, 91, 50, 49, 126
#define FLY_KEY_FN11            170  // 27, 91, 50, 51, 126
#define FLY_KEY_FN12            171  // 27, 91, 50, 52, 126

// Alt Keys
#define FLY_KEY_ALT_A           180  // 195, 165
#define FLY_KEY_ALT_B           181  // 226, 136, 171
#define FLY_KEY_ALT_C           182  // 195, 167
#define FLY_KEY_ALT_D           183  // 226, 136, 130
#define FLY_KEY_ALT_E           184  // psuedo key
#define FLY_KEY_ALT_F           185  // 198, 146
#define FLY_KEY_ALT_G           186  // 194, 169
#define FLY_KEY_ALT_H           187  // 203, 153
#define FLY_KEY_ALT_I           188  // psuedo key
#define FLY_KEY_ALT_J           189  // 226, 136, 134
#define FLY_KEY_ALT_K           190  // 203, 154
#define FLY_KEY_ALT_L           191  // 194, 172
#define FLY_KEY_ALT_M           192  // 194, 181
#define FLY_KEY_ALT_N           193  // psuedo key
#define FLY_KEY_ALT_O           194  // 195, 184
#define FLY_KEY_ALT_P           195  // 207, 128
#define FLY_KEY_ALT_Q           196  // 197, 147
#define FLY_KEY_ALT_R           197  // 194, 174
#define FLY_KEY_ALT_S           198  // 195, 159
#define FLY_KEY_ALT_T           199  // 226, 128, 160
#define FLY_KEY_ALT_U           200  // psuedo key
#define FLY_KEY_ALT_V           201  // 226, 136, 154
#define FLY_KEY_ALT_W           202  // 226, 136, 145
#define FLY_KEY_ALT_X           203  // 226, 137, 136
#define FLY_KEY_ALT_Y           204  // psuedo key
#define FLY_KEY_ALT_Z           205  // 206, 169

// specialty keys
#define FLY_KEY_BACK_TAB        210  // 27,91,90
#define FLY_KEY_DELETE          211  // 27, 91, 51, 126 (fn+delete on MAC)
#define FLY_KEY_ALT_DASH        212  // 226, 128, 147 
#define FLY_KEY_ALT_EQUAL       213  // 226, 137, 160
#define FLY_KEY_ALT_LEFT_BRACE  214  // 226, 128, 156
#define FLY_KEY_ALT_RIGHT_BRACE 215  // 226, 128, 152
#define FLY_KEY_ALT_BACKSLASH   216  // 194, 171
#define FLY_KEY_ALT_SPACE       217  // 194 168
#define FLY_KEY_ALT_COLON       218  // 226, 128, 166
#define FLY_KEY_ALT_QUOTE       219  // 195, 166
#define FLY_KEY_ALT_COMMA       220  // 226, 137, 164
#define FLY_KEY_ALT_PERIOD      221  // 226, 137, 165
#define FLY_KEY_ALT_SLASH       222  // 226, 137, 166

#define FLY_KEY_IDLE            253  // idle quit waiting for key
#define FLY_KEY_MENU_ENTER      254  // pseudo key
#define FLY_KEY_NONE            255  // causes no action

// for defining hot-keys
#define FlyKeyCtrl(key) (FLY_KEY_CTRL_A + ((key)-'A'))
#define FlyKeyAlt(key)  (FLY_KEY_ALT_A  + ((key)-'A'))
#define FlyKeyFn(key)   (FLY_KEY_FN1    +  (key))

#define FLY_KEY_MACRO_MAX       1024

// callback for idle
typedef bool_t (*pfnFlyKeyIdle_t)(void *pData);


/*************************************************************************
  Prototypes
**************************************************************************/

// key functions
flyKey_t          FlyKeyGetKey          (void);
void              FlyKeySetIdle         (pfnFlyKeyIdle_t pfnIdle, void *pData);
const char *      FlyKeyName            (flyKey_t key);
flyKey_t          FlyKeyFromName        (const char *szName);

// macro functions
void              FlyKeyMacroPlay       (void);
bool_t            FlyKeyInMacro         (void);
bool_t            FlyKeyMacroRecording  (void);
void              FlyKeyMacroRecord     (void);
void              FlyKeyMacroEndRecord  (void);
void              FlyKeyMacroClear      (void);
void              FlyKeyMacroAddKey     (flyKey_t key);
const flyKey_t   *FlyKeyMacroGet        (unsigned *pNumKeys);

// raw key functions
void              FlyKeyRawEnable   (void);
void              FlyKeyRawDisable  (void);
flyKey_t          FlyKeyRawGetKey   (void);

#ifdef __cplusplus
  }
#endif

#endif // FLY_KEY_H
