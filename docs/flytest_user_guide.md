# FlyTest User Guide

FlyTest is small, simple command-line code test framework written in C.

Automated testing is critical to any product. This test framework may aid in that effort.

FlyTest is used to test [flylibc](https://github.com/drewagislason/flylibc), a C library that
augments the standard lib. However, FlyTest can be used to test anything.

**Features**

- Encourages Test Driven Design
- Parsable markdown log-file produced for test run records or finding failure context
- Filter which tests are run by test case name or tags
- Run only automated tests (test cases that require no user input)
- List all test cases in each test suite
- Easily stub tests for code not yet written
- Verbose options to produce minimal more or max output to screen or log
- Runnable as part of Continuous Integration
- Prevent buggy code from being checked into source control (e.g. git)

See <https://en.wikipedia.org/wiki/Test-driven_development>  
See <https://en.wikipedia.org/wiki/Continuous_integration>  
See <https://en.wikipedia.org/wiki/Continuous_delivery>  
See <https://en.wikipedia.org/wiki/Continuous_deployment>  

In short, "know" your program works. Don't just "hope" it does.

## Overview

Terminology:

- Test Case - the smallest unit of testing
- Test Suite - a collection of test cases relating to a module, class, framework or program
- Module - A collection of one or more source files that fullfill a purpose
- Class - similar to a module, but for coding languages like C++ that have inheritance
- CI/CD - Continuous integration / continuous deployment
- Integration tests - tests that span multiple classes or modules

Any given test case has one of three results (a stubbed test is Skipped):

- Passed
- Failed
- Skipped

Running a test suite provides a summary of passed, failed and skipped (in colors green, red,
yellow). For example:

```
$ ./test_cli -a
TcCliSimple                             Passed
TcCliBool                               Passed
TcCliInt                                Passed
TcCliString                             Passed
TcCliComplex                            Passed
TcCliErrors                             Skipped
TcCliNoPrint                            Skipped
TcCliDoubleDash                         Passed

SUITE SUMMARY: test_cli -- opts: automated only filter)
--------------------------------------------------
Passed                                  6
Failed                                  0
Skipped                                 2
--------------------------------------------------
$ cat test.log
```

If all tests passed (or were skipped), then the test suite program returns 0, otherwise 1 if there
was one or more failures.

The screen and log can have independent information. The log file is a text file contains keywords
easily parsed for further analysis.

Run all test suites with summary using the provided bash script `testall.sh`.

## FlyTest Options

Every test suite program created has the same options, as


```
$ test_cli --help

test_cli

Usage = [-a] [-f filter] [-l] [-t tagfilter] [-v] [test suite args]

Options:
-a             Automated tests only (no manual tests)
-f "filter"    Filter based on substring test case names
-l             List test cases in this suite then exit
-t "tags"      Filter by tags. For a list of tags for test cases, use option -l (list)
-v[=#]         Verbose level: -v- (none: default), -v (some), -v=2 (more)
```

The `-a` automated is a filter which runs tests that do NOT have the "M" (manual) tag.

By default, all test cases are run in any given test suite. You can, however, filter by name using
the `-f` option. For example, say you had a suite of tests some of which test `Foo` and others
that test `Bar`. Simply use `-f Bar` to test only the `Bar` test cases.

You can also filter based on arbitrary tags using the `-t` option. Tags are explained in another
section of this document.

To list all the test casess in a test suite, use `-l` (lowercase L for list).

The verbose option `-v` sets the verbose level (0-n). Default verbose is 0. 

## FlyTest Example

Below is a fully functional example test suite program with 3 test cases.

```c
#include "FlyTest.h"

void TcExampleSkip(void)
{
  FlyTestStubbed();
}

void TcExamplePass(void)
{
  FlyTestBegin();
  if(FlyTestVerbose() >= 1)
    FlyTestPrintf("\nThis Test Always Passes\n");
  FlyTestPassed();
  FlyTestEnd();
}

void TcExampleFail(void)
{
  FlyTestBegin();
  if(FlyTestGetYesNo("Do you want this test to fail?"))
  {
    FlyTestFailed();
  }
  FlyTestEnd();
}

int main(int argc, const char *argv[])
{
  const char          szName[] = "test_example";
  const sTestCase_t   aTestCases[] =
  {
    { "TcExamplePass",    TcExamplePass },
    { "TcExampleFail",    TcExampleFail, "M" },
    { "TcExampleSkip",    TcExampleSkip },
  };
  hTestSuite_t        hSuite;
  int                 ret;

  FlyTestInit(szName, argc, argv);
  hSuite = FlyTestNew(szName, NumElements(aTestCases), aTestCases);
  FlyTestRun(hSuite);
  ret = FlyTestSummary(hSuite);
  FlyTestFree(hSuite);

  return ret;
}
```

Running this test suite produces the following results:

```
$ test_example

Options: automated 0, verbose 0, machine 0, filter: (none), tags: (none)
TcExamplePass                           Passed
TcExampleFail                           test_example.c:18:1: Failed
TcExampleSkip                           Skipped

SUITE SUMMARY: test_example -- opts: filter (none)
--------------------------------------------------
Passed                                  1
Failed                                  1
Skipped                                 1
--------------------------------------------------
```

Notice how the failure points out the exact line of code that failed to aid in debugging the
problem.

## Test Case Tags

Tags are used to filter which test cases are run. They are case sensitive.

Each test case may optionally have 1 or more space separated tags. Order doesn't matter.

Two default tags are "M" and "LOG_ONLY". The "M" tag indicates a test is manual (requires user
input). The "LOG_ONLY" tag indicates FlyTest functions do NOT display to the screen for that test
case in when your program needs to control what is printed to the terminal window.

Below are some example test cases which use the tags "FOO" and "BAR":

```c
  const sTestCase_t   aTestCases[] =
  {
    { "TcCliSimple",      TcCliSimple },
    { "TcCliBool",        TcCliBool,        "BAR FOO" },
    { "TcCliInt",         TcCliInt },
    { "TcCliString",      TcCliString,      "FOO BAR" },
    { "TcCliComplex",     TcCliComplex,     "FOO" },
    { "TcCliErrors",      TcCliErrors,      "M FOO BAR" },
    { "TcCliNoPrint",     TcCliNoPrint,     "M" },
    { "TcCliDoubleDash",  TcCliDoubleDash,  "BAR" },
  };
```

## The Log File

Each test run is appended to the log file "test.log" in the current working directory (folder). To
start a new log file, simply delete the file "test.log".

The log is a text file and contains the following data:

* A large banner to make finding test runs each
* C compiler version
* Git SHA (if in a repository and git is installed)
* Command-line options to the test_suite program.
* A set of keywords to aid in 

A sample log file is shown below.

```
=======  -----  -----   ---  -----  =======
_______    |    |__    /       |    _______
           |    |      ----    |           
=======    |    |____  ___/    |    =======


c version 201710, datetime of test run: 2024-07-15T10:38:43
argc 2 argv { ./test_example -a }

Git SHA: commit 32dafe0e76f06f523063da16aac26a6d1de2fa51
Author: Drew Gislason <drewgislason@icloud.com>
Date:   Mon Jul 15 09:20:10 2024 -0700

    Added testall.sh to run all tests, fixed some test that didn't remove files


C Version Info: Apple clang version 15.0.0 (clang-1500.3.9.4)
Target: arm64-apple-darwin23.5.0
Thread model: posix
InstalledDir: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin


Options: automated 1, verbose 0, filter: (none), tags: (none)


:TEST_SUITE_NAME:test_example
:TEST_START: 2024-07-15T10:38:43

----- :TEST_CASE:TcExamplePass -----
----- TcExamplePass(:PASSED:) -----
----- :TEST_CASE:TcExampleFail -----
----- TcExampleFail(:SKIPPED:) -----
----- :TEST_CASE:TcExampleSkip -----
----- TcExampleSkip(:SKIPPED:) -----
:TEST_END: 2024-07-15T10:38:43


SUITE SUMMARY: test_example -- opts: automated only filter (none)
--------------------------------------------------
Passed                                  1
Failed                                  0
Skipped                                 2
--------------------------------------------------
```
