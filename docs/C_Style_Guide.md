# Sensible C/C++ Coding and Style Guide

by Drew Gislason
April 14, 2024

There are many C/C++ coding and style guides, starting with the classic book on C, "The C
Programming Lanugage" by Brian Kernighan and Dennis Ritchie.

This one borrows from modern languages, like Rust, and attempts to keep things as simple as
possible. Someday, a style checker/converter may be built to this specificiation.

Feel free to use this for your projects. It is licensed under the MIT license. See: 

<https://mit-license.org>

## 1 - Overview

The C/C++ programming and style guide used by FireFly C Library (flylibc) and Tools.

## 1.1 - C/C++ Coding Style Quick Reference

This is the quick reference. See the next chapter for a more in-depth discussion with examples.

1. Line lengths *mostly* 100 characters or less (readable on a laptop, side-by-side compare on desktop)
2. Indents are 2 spaces. Indent for continued lines *after* the opening parenthesis of function call
3. Functions are`MixedCase()`, variables are `camelCase`, constants are `UPPER_SNAKE_CASE`
4. New types use `_t`,e .g. `myType_t`. Prefer typedefs rather than just `struct name` or `enum name`
5. Use parentheses for `if()` when mixing comparisions, e.g. `if((a && b) || (c && d))`
6. Curly braces (both open and close) are at the same indent as the if, for, while or switch
7. Switch statement `break` lines up with `case`. Content is indented one level from case/break
8. Document every function (mini man page). Those functions/methods that are public use doc strings
9. Use static functions and const variables wherever possible
10. Assume restrict unless otherwise specified
11. Line up elements for easy reading at a glance
12. Header files `.h` only have types, prototypes and constants, no code or data
13. C Header files use `#ifndef` to prevent processing twice
14. Prefer ASCII for functions, variables, and documenting code. Use UTF-8 if emoji, localization needed
15. Prefer C99 features only, unless newer features are required by project
16. It's OK that frameworks, libraries, tested code have different coding style

## 1.2 - Inspiration

* <https://www.joelonsoftware.com/2005/05/11/making-wrong-code-look-wrong/>
* <https://github.com/flipperdevices/flipperzero-firmware/blob/dev/CODING_STYLE.md>
* <https://doc.rust-lang.org/beta/style-guide/index.html>
* <https://google.github.io/styleguide/go/>
* <https://www.kernel.org/doc/html/v4.10/process/coding-style.html>
* <https://docs.unrealengine.com/en-US/Programming/Development/CodingStandard>
* <https://webkit.org/code-style-guidelines>

"Code is read much more often than it is written." - anonymous

"Always code as if the guy who ends up maintaining your code will be a violent psychopath who knows
 where you live." - John Woods

"Any fool can write code that a computer can understand. Good programmers write code that humans
 can understand." ― Martin Fowler

"That's the thing about people who think they hate computers. What they really hate is lousy
 programmers." ― Larry Niven

"Walking on water and developing software from a specification are easy if both are frozen." 
― Edward V. Berard

"The most important property of a program is whether it accomplishes the intention of its user."
- C.A.R. Hoare

"Heisenbug: (from Heisenberg's Uncertainty Principle in quantum physics) A bug that disappears or
alters its behavior when one attempts to probe or isolate it." - anonymous

"Everything must be made as simple as possible. But not simpler." - Albert Einstein

"Code for clarity." - Drew Gislason

## 1.3 - Coding Practice

Code is read/modified/fixed much more often than it is constructed. Hence write obvious code.

* Understand the problem you are trying to solve before coding
* Design/code/test/deploy iteratively, as it's unlikely the problem is well understood at the start
* Take time to name things well
* Prefer existing, tested functions (e.g. standard C library, your own tested code)
* Develop libraries of useful tested functions/methods/classes over time
* Version control everthing, code, planning, etc... Prefer git

The biggest mistake is solving the wrong problem. Truly understand what is needed. Try mock-ups.
Meet with the users of the code, customers, early and often. Keep discussing until there is
clarity on at least some points, then code those.

Naming takes thought. Take the time to discuss nomenclature for your project, using words that make
sense in the given problem space. Functions and methods are generally verbs: that is, they act on
variables and data. Classes and variables and CONSTANTS are generatlly nounds. The names should
reflect that.

Write code in a way that makes wrong code look wrong.

Use the style of the project you are working in, if it has one. If you are starting a new project
or your company does not already have set of C or C++ coding guidelines for new code, consider
this one.

## 2 - C/C++ Code Style Examples

This chapter contains examples of each item in the C/C++ Coding Style Quick Reference.

The following function provides a real-world example, a function from flylibc.

```c
/*!------------------------------------------------------------------------------------------------
  Similar to strstr(), but only searches to end of line.

  @param    szHaystack    string to search
  @param    szNeedle      string to look for
  @return   ptr to the found string or NULL
*///-----------------------------------------------------------------------------------------------
char * FlyStrLineStr(const char *szHaystack, const char *szNeedle)
{
  const char *sz      = szHaystack;
  const char *szFound = NULL;
  size_t      len     = strlen(szNeedle);

  if(len)
  {
    while(*sz && (*sz != '\r') && (*sz != '\n'))
    {
      if((*sz == *szNeedle) && (strncmp(sz, szNeedle, len) == 0))
      {
        szFound = sz;
        break;
      }
      ++sz;
    }
  }

  return (char *)szFound;
}
```

### 3.1 - Line Length 100

Generally, you don't want coding lines to go off the right side of the screen or require scrolling to view.

On today's laptops and desktop screens. 100 columns can be viewed side-by-side for comparison. 

Probably doesn't need to be stricly enforced. There certainly are exceptions.

### 3.2 - Line Indents are 2 spaces

Rust and Python prefer 4 spaces. The linux kernel prefers 8. This specification prefers 2 spaces.

Ideally indents would be tabs and all other whitespace would be spaces, but many editors don't have
that setting (they can do all tabs or all spaces of a certain width but not tab indent and space
everything else).

An article on tabs vs spaces: <https://stackoverflow.com/questions/11492179/tabs-vs-space-indentation>

This specification chose 2 space indents because it is enough for code to be obviosly indented and
doesn't eat up the line length.

Example:

```c
  if(len)
  {
    while(*sz && (*sz != '\r') && (*sz != '\n'))
    {
      if((*sz == *szNeedle) && (strncmp(sz, szNeedle, len) == 0))
      {
        szFound = sz;
        break;
      }
      ++sz;
    }
  }
```

If you have a long line that must be split up it is too long (may cause horizontal scrolling),
indent the subsequent line(s) to make it readable and so it is deeper than any opening braces,
ideally lining up with the function opening parenthesis.

```c
  if(fprintf(pDoc->fpOut, m_szMainColRefLine, szRef, pDocument->section.szTitle,
             pDocument->section.szSubtitle ? pDocument->section.szSubtitle : "") <= 0)
  {
  }
```

### 3.3 - Function, Variable and Constant Names

This is probably the most controversial section in this C/C++ style guide. If you don't like it, use
snake_case for functions and variables and ignore this part of the guide.

The standard C library is lowercase, `strcpy()` or `fprintf()` or snake_case, `clock_gettime()`.

This specification uses `MixedCase()` for classes/methods/functions and `char * szCamelCase` for variables.

Arguments for:

1. MixedCase is shorter: makes coding lines shorter, easier to type
2. szCamelCase allows for critical information such as pointer depth
3. Differentiates the code you maintain vs the C library code

Arguments against:

1. Looks weird (if not used to it)

Variables use **proper hungarian**, that is they help make wrong code look wrong. Go read this
article by Joel Spolsky: <https://www.joelonsoftware.com/2005/05/11/making-wrong-code-look-wrong/>

Functions should be preceeded by the "Module" they belong to. For example, all flylibc String
functions are prefixed with "FlyStr", e.g. `FlyStrPathExt()`.

```c
/*!------------------------------------------------------------------------------------------------
  Return ptr to the beginning of the next line in a string (e.g. an in-memory file).

  Lines can be `\n` (Linux/macOS) only or `\r\n` pairs (Windows).

  szLine can point to any part of the string (not just at the beginning of a line). If no lines,
  the returned result will be the end of the string.

  @param  szLine     pointer to a '\0' terminated string, possibly with lines in it
  @return pointer to next line or terminating '\0'
*///-----------------------------------------------------------------------------------------------
char * FlyStrLineNext(const char *szLine)
{
  const char *psz;

  psz = strchr(szLine, '\n');
  if(psz)
    ++psz;
  else
    psz = szLine + strlen(szLine);

  return (char *)psz;
}
```

Use of constants are encouraged to allow flexibility in the code. Say you have a "name" field in a
structure. Do you want to enforce a maximum size to allow for fixed-sized structures or to reduce
memory? Use a constant.

Constants are in `UPPER_SNAKE_CASE`.

```
#define PERSON_NAME_SIZE  32
typedef struct
{
  char      szName[PERSON_NAME_SIZE];
  unsigned  age;
} person_t;
```

### 3.4 - Type Names

Use `_t` suffix on type names, just like the standard library does for `size_t` or `uint32_t`.

```c
typedef struct
{
  char       *sz;
  size_t      size;
} flyStrSmart_t;
```

The same goes for an enum:

```c
typedef enum
{
  IS_LOWER_CASE = 0,    // lower
  IS_UPPER_CASE,        // UPPER
  IS_CAMEL_CASE,        // camelCase
  IS_MIXED_CASE,        // MixedCase, aka PascalCase
  IS_SNAKE_CASE,        // snake_case
  IS_CONSTANT_CASE,     // CONSTANT_CASE
} flyStrCase_t;
```

Use in code:

```c
size_t FlyStrToCase(char *szDst, const char *szSrc, size_t size, flyStrCase_t strCase);
```

### 3.5 - Use Parentheses For Clarity

It is critical to use parentheses when mixing bit and/or with logical and/or, because C got the
order wrong (should have done the math first, then checked logical).

```c
  if((a & b) && (c >= 5))
  {
    // ...
  }
```

But in generally, use parentheses and spaces for clarity.

```c
  while(*szLine && (FlyStrLineIsBlank(szLine) || *FlyStrSkipWhite(szLine) == '#'))
  {
    // ...
  }
```

### 3.6 - Curly Braces

Curly braces (both open and close) are at the same indent as the if, for, while or switch. This
makes it very easy to visually see blocks of code.

```c
char * FlyStrLineBeg(const char *szFile, const char *sz)
{
  const char *psz = sz;

  if(psz < szFile)
    psz = szFile;

  else
  {
    while(psz > szFile)
    {
      --psz;
      if(*psz == '\n')
      {
        ++psz;
        break;
      }
    }
  }

  return (char *)psz;
}
```

Same with do/while:

```c
  do
  {
    // ...
  } while(condition);
```

### 3.7 - Switch Statements

In switch statements, line up `case` and `break` like they were curly braces. Always have a default
statement. Often it means the switch case is out of range.

```c
  switch(caseType)
  {
    case IS_CAMEL_CASE:    // camelCase
      // code is indented...
    break;
    case IS_SNAKE_CASE:    // snake_case
      // code is indented...
    break;
    default:
      AssertFail("Bad caseType");
    break;
  }
```

### 3.8 - Source Code Documentation

Comment as you go. Keep comments up to date as code changes. Nothing is as confusing as comments
that don't match the code. Better to have no comments than misdirecting comments.

Comment each function and method like you were making a manpage. Open a command-line terminal and
type `man strcpy` to see what is meant by this.

```bash
$ man strcpy
```

Make parameters and return values clear. Indicate if out-of-bounds inputs may cause bad behavior.

```c
/*!------------------------------------------------------------------------------------------------
  Return number of characters (not bytes) in string. Use standard strlen() to get # of bytes.

  UTF-8 characters are 1-4 bytes in length each.

  @param    szUtf8   ptr to a UTF-8 string which may contain UTF-8 characters
  @return   number characters (both ASCII and UTF-8 characters) in string
*///-----------------------------------------------------------------------------------------------
size_t FlyUtf8StrLen(const utf8_t *szUtf8)
{
  size_t  nChars = 0;
  while(*szUtf8)
  {
    szUtf8 = FlyUtf8CharNext(szUtf8);
    ++nChars;
  }
  return nChars;
}
```

Likewise, document the code to make it more obvious why something is done:

```c
flyStrSmart_t * FlyStrSmartAlloc(size_t initialSize)
{
  flyStrSmart_t * pStr;

  // always make room for NUL
  if(initialSize == 0)
    ++initialSize;

  // contents might be reallocated, but a smart string should never be reallocated (moved)
  // so allocate in 2 parts
  pStr = FlyAlloc(sizeof(flyStrSmart_t));
  if(pStr)
  {
    pStr->sz = FlyAlloc(initialSize);
    if(pStr->sz == NULL)
    {
      FlyFree(pStr);
      pStr = NULL;
    }
    else
    {
      memset(pStr->sz, 0, initialSize);
      pStr->size = initialSize;
    }
  }

  return pStr;
}
```

### 3.9 - Information Hiding

Good coding practice hides under-the-hood informaiton that might change. For example, in your car,
you can count on gas pedal causing the car to move. It doesn't matter if under the hood the engine
is gas or electric.

Any function, type or resource that is not a public API function or method should be hidden with
`static`. Likewise, if a variable will not be modified it should be indicated that it is constant
with `const`.

```c
static bool_t TomlStrType(const char *szToml, bool_t *pfBasic, bool_t *pfMultiline)
{
  // ...
}
```

### 3.10 - Assume Restrict Unless Otherwise Specified

Newer specifications of C added a keyword "restrict". This is to indicate that pointers and their
associated buffers cannot overlap. For example strcpy() does NOT allow the source and destination
strings to overlap, but memmove() does.

```c
char *strcpy(char *restrict dst, const char *restrict src);
void *memmove(void *dst, const void *src, size_t n);
```

Most functions cannot overlap if copying from one place to another. So, without having to type this
on every function prototype, just assume restrict, unless the function documentation says
otherwise.

Besides, restrict is a hint to the compiler but cannot enforce the rule.

### 3.11 - Line Up Elements

It is less typing to just add a some or no whitespace between elements like arrays of strings or
structs or types and variables. However, it is much easier to read at a glance if you line them up
as shown below.

```c
void TcStrPathExt(void)
{
  typedef struct
  {
    const char *szPath;
    const char *szExp;
  } tcPathExt_t;

  const tcPathExt_t szPathExtTests[] =
  {
    { "hello.py",          ".py" },
    { "/Users/me/file.c",  ".c" },
    { "~/../main.rs",      ".rs" },
    { "../some/folder/",   NULL },
  };

  const char   *szErrMsg;
  int           i;
}
```

### 3.12 - Header Files Are For Types And Prototypes

Aside from (potentially conditional) macros, header files should only contain prototypes and
defines. No code or data is placed in header files, even for classes. The class methods can be
instantiated in .c++ files.

```c
#ifndef DEBUG
#define DEBUG 0
#endif

void       FlyError(const char *szExpr, const char *szFile, const char *szFunc, unsigned line);

#if DEBUG
  #define  FlyAssertDbg(expr)       ((expr) ? (void)0 : FlyError(#expr, __FILE__, __func__, __LINE__))
#else
  #define  FlyAssertDbg(expr)
#endif
```

### 3.13 - Prevent Header Files From Being Processed Twice

To prevent header files from being processed multiple times, and to ensure C prototypes work with
C++. use the following pattern for each .h file. For example, for FlyCli.h, use:

```c
#ifndef FLY_CLI_H
#define FLY_CLI_H
#include "Fly.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

// DEFINES, types, prototypes...

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  }
#endif

#endif // FLY_CLI_H
```

### 3.14 - Prefer ASCII

Most modern editors can support unicode in some form. This specification prefers UTF-8 if foreign
language or emoji are required in either coding variable/function names or in data.

```c
const char szHelloWorld[] = u8"こんにちは 🌎";  // hello world in Japanese with Emoji
```

However, sticking with english and ASCII makes your code accessible to a wider audience and will
work with a wider variety of compilers (especially older or embedded compilers).

### 3.15 - Prefer C99

There are some modern features, however, if you keep most (all?) of your code to the C99
specification, you will support a wider variety of compilers (especially older or embedded
compilers).

For a list features C version, see: <https://en.wikipedia.org/wiki/C_(programming_language)#History>

### 3.16 - Don't Change All Code To Match This Style Guide

Whatever coding style guide you use for internal projects, do you best to stick with that. Code
should look like it was written by one developer, not the patchwork of many developers.

However, you may wish to use one or many other frameworks to complete your project, perhaps even an
older one your company wrote with a different style guide. If so, don't adapt them to your style
guide, instead, use them as-is. Yes, your code will look a little unusual in places due to the mix
function/variable names in source files that use multiple libraries, but that greatly offsets any
editing/debugging that would otherwise occur.
