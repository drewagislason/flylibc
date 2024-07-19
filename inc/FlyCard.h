/**************************************************************************************************
  FlyCard.h
  Copyright (c) 2024 Drew Gislason
  license: MIT <https://mit-license.org>
*///***********************************************************************************************
#ifndef FLY_CARD_H
#define FLY_CARD_H
#include "Fly.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

typedef unsigned flyCard_t;

typedef enum 
{
  FLY_CARD_CLUBS,
  FLY_CARD_DIAMONDS,
  FLY_CARD_HEARTS,
  FLY_CARD_SPADES,
} flyCardSuit_t;

// cards are ranked 1-13 (ace - king)
#define FLY_CARD_RANK_NONE   0
#define FLY_CARD_RANK_JOKER 14

typedef enum
{
  FLY_CARD_BACK_PLAIN,    // just an outline
  FLY_CARD_BACK_X,        // Xs
  FLY_CARD_BACK_O,        // Xs
  FLY_CARD_BACK_SQUARES,  // square back
  FLY_CARD_FANCY,         // design
} flyCardBack_t;

typedef struct
{
  unsigned   maxCards;
  unsigned   nCards;
  flyCard_t *pCards;
} flyCardDeck_t;

typedef flyCardDeck_t flyCardHand_t;

#define FLY_CARD_BACK         0
#define FLY_CARD_DECK_SIZE   52

flyCard_t       FlyCardMake         (unsigned rank, flyCardSuit_t suit);
flyCard_t       FlyCardMakeEx       (unsigned rank, flyCardSuit_t suit, bool_t fFaceUp, flyCardBack_t back);
unsigned        FlyCardRank         (flyCard_t card);
flyCardSuit_t   FlyCardSuit         (flyCard_t card);
bool_t          FlyCardIsFaceUp     (flyCard_t card);
void            FlyCardPrint        (const flyCard_t *pCards, unsigned nCards);

flyCardDeck_t  *FlyCardDeckNew      (unsigned nDecks, bool_t fUtf8, unsigned jokers);
void            FlyCardDeckSetup    (flyCardDeck_t *pDeck, bool_t fAsciiOnly, flyCardBack_t back);
void            FlyCardDeckClear    (flyCardDeck_t *pDeck);
void            FlyCardDeckBackSet  (flyCardDeck_t *pDeck, const char *szBack);
void            FlyCardDeal         (flyCardDeck_t *pDeck, unsigned nCards, unsigned nPlayers, flyCardHand_t *pHands);
void            FlyCardDeckInit     (flyCardDeck_t *pDeck);
void            FlyCardDeckShuffle  (flyCardDeck_t *pDeck);

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  }
#endif

#endif // FLY_CARD_H
