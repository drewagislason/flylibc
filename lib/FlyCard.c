/**************************************************************************************************
  FlyCard.c - For card games: generic deck and card handling
  Copyright 2024 Drew Gislason
  license: <https://mit-license.org>
**************************************************************************************************/

/*!
  @class FlyCard   A cardgame API. Create, shuffle and deal cards.

  Features:

  1. Designed to make creating card games easy
  2. API can be used in any coding language that supports interacting with C
  3. Cards can be displayed in the terminal, or used in a GUI
  4. Includes shuffling muliple decks, allow 0-n jokers per deck
  5. Includes sample solitaire game

  The only function that displays cards (to a terminal) is FlyCardPrint(). It allows for both UTF-8
  text graphics for clubs, diamonds, hearts, spades. Otherwise,  ascii `+ < @ ^` kind of look like
  clubs, diamonds, hearts, spades.

      +----+ +----+         +----+ +----+
      |10^ | |K@  |         |10&#9827; | |K&#9829;  |
      |    | |    |   vs.   |    | |    |
      | 10^| |  K@|         | 10&#9827;| |  K&#9829;|
      +----+ +----+         +----+ +----+

  You can create your own card backs with FlyCardDeckBackSet(), or use the defaults:

      Default  Plain     X     Square   Fancy    O
      +----+   +----+  +----+  +----+  +----+  +----+  
      |////|   |    |  |XXXX|  |_|__|  |&  %|  |oooo|
      |////|   |    |  |XXXX|  |__|_|  | <> |  |oooo|
      |////|   |    |  |XXXX|  | |  |  |%  &|  |oooo|
      +----+   +----+  +----+  +----+  +----+  +----+

  Note: decks and hands are interchangeable. The names are there for symantic reasons.
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "FlyCard.h"
#include "FlyMem.h"

// suits MUST be in order clubs, diamonds, hearts, spades
static const char  *m_aszSuit[]       = { "\u2663", "\033[1;91m\u2662\033[0m", "\033[1;91m\u2661\033[0m", "\u2660" };
static const char  *m_aszSuitAscii[]  = { "+", "<", "@", "^" };
static const char  *m_aszRank[]       = { "1","2","3","4","5","6","7","8","9","10","J","Q","K" };

/*!------------------------------------------------------------------------------------------------
  Choose setup options.

  @param  fAsciiOnly    Don't use the UTF-8 chars for clubs/spades, etc.., instead use ascii chars.
  @return none
*///-----------------------------------------------------------------------------------------------
void FlyCardDeckSetup(flyCardDeck_t *pDeck, bool_t fAsciiOnly, flyCardBack_t back)
{
  (void)pDeck;
  (void)fAsciiOnly;
  (void)back;
}

/*!------------------------------------------------------------------------------------------------
  Create card based on rank/suit. Leaves faceup status and card back alone.

  @param  rank    0=none, 1-13= Ace-King, 14=joker
  @param  suit    e.g. FLY_CARD_CLUBS
  @return the card
*///-----------------------------------------------------------------------------------------------
flyCard_t FlyCardMake(unsigned rank, flyCardSuit_t suit)
{
  if(rank < 1 || rank > 13)
    rank = 2;  
  return (flyCard_t)(rank | (suit << 4));
}

/*!------------------------------------------------------------------------------------------------
  Get rank of card

  @param  card
  @return rank 0 = not a card, (1 - 13) of card, 14 = joker
*///-----------------------------------------------------------------------------------------------
unsigned FlyCardRank(flyCard_t card)
{
  return (card & 0xf);
}

/*!------------------------------------------------------------------------------------------------
  Get suit of card, which are ranked with clubs lowest and spades highest, just like in poker.

  @param  card
  @return suit   FLY_CARD_CLUBS, FLY_CARD_DIAMONDS, FLY_CARD_HEARTS, FLY_CARD_SPADES
*///-----------------------------------------------------------------------------------------------
flyCardSuit_t FlyCardSuit(flyCard_t card)
{
  unsigned  suit = ((card >> 4) & 0x3);
  if(suit > FLY_CARD_SPADES)
    suit = FLY_CARD_SPADES;
  return suit;
}

/*!------------------------------------------------------------------------------------------------
  Print a hand of cards to terminal. This prints all cards on the same line.

  ```
  +----+ +----+
  |10+ | |K@  |
  |    | |    |
  | 10+| |  K@|
  +----+ +----+
  ```

*///-----------------------------------------------------------------------------------------------
void FlyCardPrint(const flyCard_t *pCards, unsigned nCards)
{
  static const char   szTop[]     = "+----+";
  static const char   szMid[]     = "|    |";
  static const char   szBack[]    = "|////|";
  const char        **ppSuit;
  unsigned            lines       = 5;
  unsigned            rank;
  unsigned            suit;
  unsigned            line;
  unsigned            col;

  // ascii only means no color, no unicode

  for(line = 0; line < lines; ++line)
  {
    for(col = 0; col < nCards; ++ col)
    {
      rank = FlyCardRank(pCards[col]);
      suit = FlyCardSuit(pCards[col]);
      if(pCards[col] & 0x8000)
        ppSuit = m_aszSuit;
      else
        ppSuit = m_aszSuitAscii;

      if((line == 0) || ((line + 1) == lines))
        printf("%s ", szTop);
      else if(pCards[col] == 0)
        printf("%s ", szBack);
      else if(line == 1)
        printf("|%s%s%*s| ", m_aszRank[rank-1], ppSuit[suit], (rank == 10) ? 1 : 2, "");
      else if((line + 2) == lines)
        printf("|%*s%s%s| ", (rank == 10) ? 1 : 2, "", m_aszRank[rank-1], ppSuit[suit]);
      else
        printf("%s ", szMid);
    }
    printf("\n");
  }
}

/*!------------------------------------------------------------------------------------------------
  Create a new deck or cards (or set of decks).

  A black-jack game may utilize 6 shuffled decks, whereas crazy eights uses a single deck.

  Cards will be in in order like they come out of a fresh pack of cards. Make sure to shuffle them
  if needed.

  @example  Create and deal a game of crazy eights

  ```
  #define NPLAYERS          4
  #define NCARDS_AT_DEAL    8

  flyCardDeck_t * Crazy8Deal(unsigned nPlayers, flyCardHand_t *pHands)
  {
    flyCardDeck_t *pDeck = FlyCardDeckNew(1, TRUE, 0);
    if(pDeck)
    {
      FlyCardDeckShuffle(pDeck);
      FlyCardDeckDeal(pDeck, NCARDS_AT_DEAL, nPlayers, pHands);
    }
  }

  int main(void)
  {
    flyCardHand_t *pHands = FlyCardDeckNew(NPLAYERS, TRUE, 0);
  }
  ```

  @param  nDecks      # of decks
  @param  fAsciiOnly  use Ascii rather than UTF-8 for card display
  @param  jokers      jokers per deck
  @return none
*///-----------------------------------------------------------------------------------------------
flyCardDeck_t *FlyCardDeckNew(unsigned nDecks, bool_t fAsciiOnly, unsigned nJokers)
{
  flyCardDeck_t *pDeck = NULL;

  (void)fAsciiOnly;
  (void)nJokers;
  pDeck = FlyAlloc(nDecks * FLY_CARD_DECK_SIZE);
  if(pDeck)
  {
    memset(pDeck, 0, sizeof(*pDeck));
    FlyCardDeckInit(pDeck);
  }

  return pDeck;
}

/*!------------------------------------------------------------------------------------------------
  Initialize a deck of cards, (or only some cards). Initializes with spades, hearts, diamonds,
  clubs. Can even initialize multiple decks. Examples:

  @param    pCardDeck   pointer to array of cards
  @param    nCards      number of cards in array
  @returns  none
*///-----------------------------------------------------------------------------------------------
void FlyCardDeckInit(flyCardDeck_t *pDeck)
{
  flyCard_t      *pCard;
  flyCardSuit_t   suit;
  unsigned        rank;
  unsigned        i;
  unsigned        nCards = pDeck->nCards;

  pCard = pDeck->pCards;
  for(i = 0; i < pDeck->nCards; ++i)
  {
    for(suit = FLY_CARD_CLUBS; nCards && suit <= FLY_CARD_SPADES; ++suit)
    {
      for(rank = 1; nCards && rank <= 13; ++rank)
      {
        *pCard = FlyCardMake(rank, suit);
        --nCards;
        ++pCard;
      }
    }
  }
}

/*!------------------------------------------------------------------------------------------------
  Shuffle cards. Simply pass the deck.

  @param    pCards   pointer to array of cards
  @param    nCards   number of cards in array
  @returns  none
*///-----------------------------------------------------------------------------------------------
void FlyCardDeckShuffle(flyCardDeck_t *pDeck)
{
  (void)pDeck;
#if 0
  static bool_t   fInitialized = FALSE;
  unsigned       *p;

  if(!fInitialized)
  {
    fInitialized = TRUE;
    srandom(time(NULL));
  }

  p = malloc(nCards * sizeof(long));
  if(p)
  {
  }
#endif
}
