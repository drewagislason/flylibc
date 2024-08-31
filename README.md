# flylibc

flylibc is a C library that augments the standard C library.

Release 1.0  
Copyright (c) 2024 Drew Gislason  
license: <https://mit-license.org>  

## Overview

**Features**

- Depends only on the standard C99 library, no other dependencies
- Assert; better asserts with custom output() and exit function cleanup
- Configuration files: easy JSON and TOML handling
- CLI: argument/option handling for quickly making useful command-line utilities
- Embeddable: manu modules contain small code, are reentrant, use minimal stack and no heap
- Code examples
- Read/write/maniplate files easily, for example changing line endings between Windows and Linux
- Heap: debug heap errors (overrun, use after free, memory leaks, etc...)
- List: easy, generic, debugged linked list search, sort, add, delete
- Keyboard: Generic raw keyboard input handling, detect idle, add custom keys
- Logging: Generic text logging
- Markdown: Easily convert markdown to HTML
- SemVer: support for semantic version comparisons
- Socket: easy IPv4 and IPv6 socket handling
- Stack Trace: made simple for finding segment faults and more info about the asserts()
- Strings: smart strings, better search, manipulation
- Terminal: minimal cmdline for embedded systems
- Test Framework for flylibc and your own code
- Time/Date handling: simplified time/date handling
- Timers: flexible, simplified timers use a single sytem timer
- UTF-8: easily parse and manipulte UTF-8 strings (superset of ASCII)
- Windowing System: make full-screen terminal apps or use colors or formatting
- Any many more

Some tools are built with flylibc, including:

- flymake - a simplified make/CMake: <https://github.com/drewagislason/flymake>
- flydoc - mobile first code documentation: <https://github.com/drewagislason/flydoc>
- ned - a modern editor for bash/zsh: <https://github.com/drewagislason/ned>

flylibc is named after [Firefly](https://en.wikipedia.org/wiki/Firefly_(TV_series)), an iconic space western.

## Getting Started

If not already installed, install a C compiler and, optionally, make or flymake on your system.
Search the internet and you'll find many resources on how to install a C compiler (XCode for
macOS, gcc for Linux, Visual Studio C with developer prompt for Windows).

Install Git if you don't already have it installed. It is recommended that you set up SSH keys for
git, although you can use HTTP and password if you prefer.

Once C and git are installed, open a terminal window create a folder where you will do C projects.
Then clone flylibc, make the library, and try out some examples:

```
$ git clone git@github.com:drewagislason/flylibc.git
$ cd flylibc/lib
$ ./makeall.sh
$ cd ../examples
$ ./makeall.sh
$ ./example_cli --help
```

If installed, substitute `make` or `flymake` instead of `./makeall.sh`.

## flylibc C Library

flylibc compiles with gcc and clang and should be compatible with any C99 compiler.

Features:

- All modules in flylibc use only the standard C library. No other dependencies
- All header (.h) files have the proper wrapping to be included in C++ programs
- Every function is documented similar to a manpage
- Compile with DEBUG flag to gain more insight into usage errors
- All functions are tested. See the test/ folder

Many of the flylibc modules are embeddable. In flylibc, this means:

- Small footprint in terms of ROM, RAM and stack
- Thread safe
- No heap use, ie. no malloc(), free()
- No printf() or other output or input
- No file I/O, e.g. operate on strings only, not files

The modules are described in brief here. Em means embeddable:

Module         | Em | Description
-------------- | -- | -----------
FlyAes         | y  | A small implementation of the AES algorithm
FlyAnsi        | y  | Ansi terminal codes for color and positioning text
FlyAssert      | y  | Custom Asserts with or without stack trace
FlyCard        |    | For card games: generic deck and card handling
FlyCli         |    | Easily process command-line options and arguments
FlyFile        |    | File creation/deletion/listing/conversion utilities
FlyJson        | y  | Parse and write JSON files
FlyKey         | y  | Full keyboard input, e.g. Alt-Left-Arrow, Ctrl-Space
FlyKeyPrompt   | y  | Command-line style key editing (Ctrl-K, etc...)
FlyList        | y  | Genereic linked list handling
FlyLog         | y  | Parsable logging to memory, files or screen
FlyMarkdown    |    | Simple API for converting markdown to HTML
FlyMem         | y  | Help debug memory usage and errors
FlySec         | y  | Application level end-to-end encryption
FlySemVer      |    | Easy parsing and comparison of semantic version strings
FlySocket      | y  | Easy IPV4/IPv6 TCP and UTP sockets
FlySort        | y  | Sort linked lists and arrays
FlyStr         | y  | String utilities including smart strings, path handling
FlyTabComplete | y  | Allows tab completion from any list of strings
FlyTest        |    | Unit test your own C code, build test cases and suites
FlyTime        | y  | Basic time functions, current, elapsed, etc...
FlyToml        | y  | Parse TOML configuration files
FlyUtf8        | y  | UTF-8 string handling

## Project Layout

```
bin       compiled versions of the tools go, including flylibc.a
docs      API and coding documentation
examples  a few example programs using flylibc
images    .pngs for including in documentation
inc       include files
lib       source to library files
test      test code for flylibc modules
tools     some helpful C tools
```
