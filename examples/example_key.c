/**************************************************************************************************
  example_key.c - Example using FlyKey API
  Copyright 2024 Drew Gislason
  License: MIT <https://mit-license.org>
**************************************************************************************************/
#include "FlyKey.h"

// if idle for 2 seconds, then return TRUE to indicate idle
bool_t key_idle(void *pData)
{
  #define IDLE_PRINT_TIME   20      // every 2 seconds, aka 20 10ths of a second
  bool_t    idle          = FALSE;
  unsigned *idle_print  = pData;

  ++(*idle_print);
  if(*idle_print >= IDLE_PRINT_TIME)
  {
    *idle_print = 0;
    idle = TRUE;
  }

  return idle;
}

void print_all_keys(void)
{
  #define PRINT_COLS 5
  flyKey_t    key;
  const char *sz_key_name;
  unsigned    col;

  col = 0;
  key = 0;
  while(key < FLY_KEY_NONE)
  {
    sz_key_name = FlyKeyName(key);
    if(sz_key_name && *sz_key_name)
    {
      printf("%4u: %-*s", key, 16, sz_key_name); // Ctrl-Backspace
      ++col;
      if((col % PRINT_COLS) == 0)
        printf("\n");
    }
    ++key;
  }
  printf("\n");
}

int main(int argc, const char *argv[])
{
  flyKey_t  key;
  unsigned  idle_print  = 0;

  if(argc > 1 && strcmp(argv[1], "--help") == 0)
    print_all_keys();

  else
  {
    printf("Press any key, like ctrl-c, right-arrow, alt-q, etc... Press esc to exit.\n");
    FlyKeySetIdle(key_idle, &idle_print);
    while(1)
    {
      key = FlyKeyGetKey();
      if(key == FLY_KEY_IDLE)
        printf(".");
      else
      {
        idle_print = 0;
        printf("%s\n", FlyKeyName(key));
      }
      if(key == FLY_KEY_ESC)
        break;
    }
  }

  return 0;
}
