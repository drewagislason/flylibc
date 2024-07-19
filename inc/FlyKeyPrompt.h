/**************************************************************************************************
  FlyKeyPrompt.h
  Copyright 2024 Drew Gislason
  license: MIT <https://mit-license.org>
*///***********************************************************************************************
#ifndef FLY_KEY_PROMPT_H
#define FLY_KEY_PROMPT_H
#include "FlyKey.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

typedef void * hFlyKeyPrompt_t;

hFlyKeyPrompt_t   FlyKeyPromptNew       (unsigned size, const char *sz);
bool_t            FlyKeyPromptIsPrompt  (hFlyKeyPrompt_t hPrompt);
void              FlyKeyPromptFree      (hFlyKeyPrompt_t hPrompt);
int               FlyKeyPromptFeed      (hFlyKeyPrompt_t hPrompt, flyKey_t c);
char *            FlyKeyPromptGets      (hFlyKeyPrompt_t hPrompt);
unsigned          FlyKeyPromptSize      (hFlyKeyPrompt_t hPrompt);
unsigned          FlyKeyPromptLen       (hFlyKeyPrompt_t hPrompt);
void              FlyKeyPromptClear     (hFlyKeyPrompt_t hPrompt);
void              FlyKeyPromptRedraw    (hFlyKeyPrompt_t hPrompt);

#ifdef __cplusplus
  }
#endif

#endif // FLY_KEY_PROMPT_H
