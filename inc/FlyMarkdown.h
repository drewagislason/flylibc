/**************************************************************************************************
  FlyMarkdown.h
  Copyright 2024 Drew Gislason
  license: <https://mit-license.org>
**************************************************************************************************/

#ifndef FLY_MARKDOWN_H
#define FLY_MARKDOWN_H
#include "Fly.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

// max table columns
#ifndef FLYMD2HTML_TABLE_COL_MAX
#define FLYMD2HTML_TABLE_COL_MAX  26
#endif

// max depth for block quotes
#ifndef FLYMD2HTM_BLOCK_QUOTE_MAX
#define FLYMD2HTM_BLOCK_QUOTE_MAX 6
#endif

// debugging
extern bool_t   fFlyMarkdownDebug;
extern const char *g_szMd;

// functions that produce HTML code from markdown
size_t  FlyMd2HtmlBlockQuote  (char *szHtml, size_t size, const char **ppszMd);
size_t  FlyMd2HtmlCodeBlk     (char *szHtml, size_t size, const char **ppszMd, const char *szTitle, const char *szW3Color);
size_t  FlyMd2HtmlCodeIn      (char *szHtml, size_t size, const char **ppszMd);
size_t  FlyMd2HtmlCodeLine    (char *szHtml, size_t size, const char **ppszMd);
size_t  FlyMd2HtmlEmphasis    (char *szHtml, size_t size, const char **ppszMd, bool_t fClose);
size_t  FlyMd2HtmlHeading     (char *szHtml, size_t size, const char **ppszMd, const char *szW3Color);
size_t  FlyMd2HtmlHorzRule    (char *szHtml, size_t size, const char **ppszMd);
size_t  FlyMd2HtmlImage       (char *szHtml, size_t size, const char **ppszMd);
size_t  FlyMd2HtmlList        (char *szHtml, size_t size, const char **ppszMd);
size_t  FlyMd2HtmlPara        (char *szHtml, size_t size, const char **ppszMd);
size_t  FlyMd2HtmlTextLine    (char *szHtml, size_t size, const char **ppszMd, const char *szMdEnd);
size_t  FlyMd2HtmlQLink       (char *szHtml, size_t size, const char **ppszMd);
size_t  FlyMd2HtmlRef         (char *szHtml, size_t size, const char **ppszMd);
size_t  FlyMd2HtmlTable       (char *szHtml, size_t size, const char **ppszMd);

size_t  FlyMd2HtmlContent     (char *szHtml, size_t size, const char *szMd, const char *szMdEnd);
size_t  FlyMd2HtmlFileHead    (char *szHtml, size_t size, const char *szTitle);
size_t  FlyMd2HtmlFileEnd     (char *szHtml, size_t size);
size_t  FlyMd2HtmlFile        (char *szHtml, size_t size, const char *szMd, const char *szTitle);

typedef enum
{
  MD_EM_TYPE_NONE = 0,
  MD_EM_TYPE_ITALICS,         // *italics*
  MD_EM_TYPE_BOLD,            // **bold**
  MD_EM_TYPE_BOLD_ITAL,       // ***bold & italics***
  MD_EM_TYPE_HIGHLIGHT,       // ==highlight==
  MD_EM_TYPE_STRIKE_THROUGH,  // ~~strike through~~
  MD_EM_TYPE_SUB,             // ~subscript~
  MD_EM_TYPE_SUPER,           // ^superscript^
  MD_EM_TYPE_SIZEOF
} flyMdEmType_t;

typedef enum
{
  MD_REF_TYPE_NONE = 0,
  MD_REF_TYPE_IMAGE,          // ![alt text](file.png "title")
  MD_REF_TYPE_REF,            // [ref text](site.com/page)
  MD_REF_TYPE_FOOT_REF,       // [^footnote]
  MD_REF_TYPE_FOOTNOTE        // [^footnote]: paragraph text of footnote
} flyMdRefType_t;

typedef struct
{
  const char     *szAlt;
  const char     *szLink;
  const char     *szTitle;
  unsigned        altLen;
  unsigned        linkLen;
  unsigned        titleLen;
  flyMdRefType_t  refType;
} flyMdAltLink_t;

// helper functions do NOT produce HTML code
bool_t          FlyMd2HtmlIsBlockQuote  (const char *szMd);
bool_t          FlyMd2HtmlIsBreak       (const char *szMd);
bool_t          FlyMd2HtmlIsCodeBlk     (const char *szMd, bool_t *pIsBackticks);
flyMdEmType_t   FlyMd2HtmlIsEmphasis    (const char *szMd);
bool_t          FlyMd2HtmlIsHeading     (const char *szMdLine, unsigned *pLevel);
bool_t          FlyMd2HtmlIsHorzRule    (const char *szMdLine);
bool_t          FlyMd2HtmlIsImage       (const char *szMd);
bool_t          FlyMd2HtmlIsList        (const char *szMdLine, bool_t *pIsNumeric);
bool_t          FlyMd2HtmlIsQLink       (const char *szMd);
flyMdRefType_t  FlyMd2HtmlIsRef         (const char *szMd);
bool_t          FlyMd2HtmlIsTable       (const char *szMdLine);

char           *FlyMd2HtmlCodeBlkEnd    (const char *szMd);
const char     *FlyMd2HtmlHeadingText   (const char *szLine);
char           *FlyMdAltLink            (flyMdAltLink_t *pAltLink, const char *szMd);
char           *FlyMdNPBrk              (const char *sz, const char *szEnd, const char *szAccept);

#ifdef __cplusplus
  }
#endif

#endif // FLY_MARKDOWN_H
