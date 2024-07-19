/**************************************************************************************************
  FlyKeyPrompt.c - Command-line style key editing (Ctrl-K, etc...)
  Copyright 2024 Drew Gislason
  license: <https://mit-license.org>
**************************************************************************************************/
#include "FlyKeyPrompt.h"
#include "FlyMem.h"

/*!
  @defgroup   FlyKeyPrompt    A C object that mimics the keyboard input prompt of the terminal

  This API is useful for "terminal" like applications, and is fully embeddable.

  Features:

  1. terminal keys like Ctrl-A (start of line) and Ctrl-E (end of line)
  2. line-string can be edited, cursor keys work
  3. Cursor keys up/down go to prev/next saved line.
  4. Define max # of saved lines at compile-time
  5. Ctrl-R to search for string in history
  6. Optional tab-completion of files/folders/commands
  7. Embeddable (no memory allocation, small footprint)

  @example  FlyKeyPrompt

  ```
  Insert example here...
  ```
*/

#define FLY_KEY_PROMPT_SANCHK 9213

typedef struct
{
  unsigned    sanchk;
  unsigned    size;
  unsigned    len;    // strlen of psz, TRUE: len < size
  unsigned    pos;    // TRUE: pos <= len
  char       *psz;    // TRUE: asciiz
} flyKeyPrompt_t;

/*!------------------------------------------------------------------------------------------------
  Create a new prompt. Allows for non-blocking keyboard input while preserving state. For now,
  input is stdin, output is stdout.

  @param    size 2-n, as asciiz string is always NUL terminated
  @param    sz, a seed string, may be NULL for none
  @return   handle to prompt state machine
*///-----------------------------------------------------------------------------------------------
hFlyKeyPrompt_t FlyKeyPromptNew(unsigned size, const char *sz)
{
  flyKeyPrompt_t  *pPrompt = NULL;
  unsigned        len;

  if(size >= 2)
    pPrompt = FlyAlloc(sizeof(*pPrompt));
  if(pPrompt)
  {
    memset(pPrompt, 0, sizeof(*pPrompt));
    pPrompt->sanchk = FLY_KEY_PROMPT_SANCHK;
    pPrompt->psz    = FlyAlloc(size);
    pPrompt->size   = size;
    if(pPrompt->psz == NULL)
    {
      FlyKeyPromptFree(pPrompt);
      pPrompt = NULL;
    }
    else
    {
      memset(&pPrompt->psz[0], 0, size);
      if(sz)
      {
        len = strlen(sz);
        if(len >= size)
          len = size - 1;
        strncpy(pPrompt->psz, sz, len + 1);
        fputs(sz, stdout);
        pPrompt->pos = pPrompt->len = len;
      }
    }
  }

  return pPrompt;  
}

/*!------------------------------------------------------------------------------------------------
  Is this a prompt handle?
  @param    hPrompt   handle to prompt
  @return   TRUE if this is a prompt handle
*///-----------------------------------------------------------------------------------------------
bool_t FlyKeyPromptIsPrompt(hFlyKeyPrompt_t hPrompt)
{
  flyKeyPrompt_t *pPrompt = hPrompt;
  return (pPrompt && (pPrompt->sanchk == FLY_KEY_PROMPT_SANCHK)) ? TRUE : FALSE;
}

/*!------------------------------------------------------------------------------------------------
  Free this prompt handle
  @param    hPrompt   handle to prompt
  @return   none
*///-----------------------------------------------------------------------------------------------
void FlyKeyPromptFree(hFlyKeyPrompt_t hPrompt)
{
  flyKeyPrompt_t *pPrompt = hPrompt;

  if(FlyKeyPromptIsPrompt(hPrompt))
  {
    if(pPrompt->psz)
    {
      memset(pPrompt->psz, 0, pPrompt->size);
      FlyFree(pPrompt->psz);
    }
    memset(pPrompt, 0, sizeof(*pPrompt));
    FlyFree(pPrompt);
  }
}

/*!------------------------------------------------------------------------------------------------
  Feed the prompt with a key, Cases display to update if key will do so.

  Supports the following keys:
  Ctrl-A, Ctrl-E, Ctrl-B, Ctr-F, Ctrl-K Enter, BS, Del, Left Arrow, Right Arrow
  
  @param    hPrompt   handle to prompt
  @return   -1 if string not yet complete. len of string if Enter is pressed.
*///-----------------------------------------------------------------------------------------------
int FlyKeyPromptFeed(hFlyKeyPrompt_t hPrompt, flyKey_t c)
{
  flyKeyPrompt_t *pPrompt = hPrompt;
  int             ret     = -1;
  unsigned        i;
  unsigned        n;
  unsigned        len;
  unsigned        pos;

  if(FlyKeyPromptIsPrompt(hPrompt))
  {
    // useful shortcut vars
    len = pPrompt->len;
    pos = pPrompt->pos;
    n   = len - pos;

    // insert char
    if(c >= ' ' && c <= '~')
    {
      // printf("1: c %c, pos %u, len %u\n", c, len, pos);
      if(len + 1 < pPrompt->size)
      {
        // update state machine
        memmove(&pPrompt->psz[pos + 1], &pPrompt->psz[pos], n + 1);
        pPrompt->psz[pos] = c;
        ++pPrompt->pos;
        ++pPrompt->len;

        // display to screen
        fputs(&pPrompt->psz[pos], stdout);
        for(i = 0; i < n; ++i)
          fputc('\010', stdout);
      }
      else
      {
        fputc('\007', stdout);
      }
    }

    // left arrow
    else if((c == FLY_KEY_CTRL_B) || (c == FLY_KEY_LEFT))
    {
      if(pos > 0)
      {
        fputc('\010', stdout);
        --pPrompt->pos;
      }
    }

    // right arrow
    else if((c == FLY_KEY_CTRL_F) || (c == FLY_KEY_RIGHT))
    {
      if(pos < len)
      {
        fputc(pPrompt->psz[pos], stdout);
        ++pPrompt->pos;
      }
    }

    // home
    else if((c == FLY_KEY_CTRL_A) || (c == FLY_KEY_HOME))
    {
      if(pos > 0)
      {
        for(i = 0; i < pos; ++i)
          fputc('\010', stdout);
        pPrompt->pos = 0;
      }
    }

    // end
    else if((c == FLY_KEY_CTRL_E) || (c == FLY_KEY_END))
    {
      if(pos < len)
      {
        for(i = pos; i < len; ++i)
          fputc(pPrompt->psz[i], stdout);
        pPrompt->pos = len;
      }
    }

    // Kill to end of line
    else if(c == FLY_KEY_CTRL_K)
    {
      pPrompt->len = pos;
      memset(&pPrompt->psz[pos], 0, n + 1);
      for(i = 0; i < n; ++i)
        fputc(' ', stdout);
      for(i = 0; i < n; ++i)
        fputc('\010', stdout);
    }

    // BS
    else if(c == FLY_KEY_BACKSPACE)
    {
      if(pos > 0)
      {
        --pos;
        memmove(&pPrompt->psz[pos], &pPrompt->psz[pos + 1], n + 1);

        fputc('\010', stdout);
        fputs(&pPrompt->psz[pos], stdout);
        fputc(' ', stdout);
        for(i = 0; i < n + 1; ++i)
          fputc('\010', stdout);

        --pPrompt->pos;
        --pPrompt->len;
      }
    }

    // Del
    else if(c == FLY_KEY_DELETE)
    {
      if(pos < len)
      {
        memmove(&pPrompt->psz[pos], &pPrompt->psz[pos + 1], n + 1);

        fputs(&pPrompt->psz[pos], stdout);
        fputc(' ', stdout);
        for(i = 0; i < n; ++i)
          fputc('\010', stdout);

        --pPrompt->len;
      }
    }

    // Enter
    else if(c == FLY_KEY_ENTER)
    {
      fputs(&pPrompt->psz[pos], stdout);
      ret = len;
    }

    // Esc
    else if(c == FLY_KEY_ESC)
    {
      fputs(&pPrompt->psz[pos], stdout);
      ret = -2;
    }

    // BUGBUG
    else if(c == FLY_KEY_CTRL_Y)
    {
      printf("\npos %u, len %u, size %u, psz '%s'\n> ", pPrompt->pos, pPrompt->len, pPrompt->size, pPrompt->psz);
      FlyKeyPromptRedraw(hPrompt);
    }

    fflush(stdout);
  }

  return ret;
}

/*!------------------------------------------------------------------------------------------------
  Return the string allocated in the prompt state machine

  @param    hPrompt   handle to prompt
  @return   ptr to asciiz string
*///-----------------------------------------------------------------------------------------------
char * FlyKeyPromptGets(hFlyKeyPrompt_t hPrompt)
{
  flyKeyPrompt_t *pPrompt = hPrompt;
  char           *psz     = NULL;

  if(FlyKeyPromptIsPrompt(hPrompt))
    psz = &pPrompt->psz[0];
  return psz;
}

/*!------------------------------------------------------------------------------------------------
  Assumes cursor is at start of string. Redraw it, and back up cursor to position.

  @param    hPrompt   handle to prompt
  @return   ptr to asciiz string
*///-----------------------------------------------------------------------------------------------
unsigned FlyKeyPromptSize(hFlyKeyPrompt_t hPrompt)
{
  flyKeyPrompt_t *pPrompt = hPrompt;
  unsigned        size    = 0;

  if(FlyKeyPromptIsPrompt(hPrompt))
    size = pPrompt->size;

  return size;
}

/*!------------------------------------------------------------------------------------------------
  Assumes cursor is at start of string. Redraw it, and back up cursor to position.

  @param    hPrompt   handle to prompt
  @return   ptr to asciiz string
*///-----------------------------------------------------------------------------------------------
unsigned FlyKeyPromptLen(hFlyKeyPrompt_t hPrompt)
{
  flyKeyPrompt_t *pPrompt = hPrompt;
  unsigned        len    = 0;

  if(FlyKeyPromptIsPrompt(hPrompt))
    len = pPrompt->len;

  return len;
}

/*!------------------------------------------------------------------------------------------------
  Clears the prompt. Prints nothing to screen

  @param    hPrompt   handle to prompt
  @return   ptr to asciiz string
*///-----------------------------------------------------------------------------------------------
void FlyKeyPromptClear(hFlyKeyPrompt_t hPrompt)
{
  flyKeyPrompt_t *pPrompt = hPrompt;

  if(FlyKeyPromptIsPrompt(hPrompt))
  {
    memset(&pPrompt->psz[0], 0, pPrompt->size);
    pPrompt->len = pPrompt->pos = 0;
  }  
}

/*!------------------------------------------------------------------------------------------------
  Assumes cursor is at start of string. Redraw it, and back up cursor to position.

  @param    hPrompt   handle to prompt
  @return   ptr to asciiz string
*///-----------------------------------------------------------------------------------------------
void FlyKeyPromptRedraw(hFlyKeyPrompt_t hPrompt)
{
  flyKeyPrompt_t *pPrompt = hPrompt;

  if(FlyKeyPromptIsPrompt(hPrompt))
  {
    // display the string
    fputs(pPrompt->psz, stdout);

    // backup to position if in middle of string
    for(unsigned i = pPrompt->pos; i < pPrompt->len; ++i)
      fputc('\010', stdout);

    fflush(stdout);
  }
}
