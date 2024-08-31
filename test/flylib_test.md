# FlyLib Test Suite

This document defines the Firefly C Library test procedure.

## Automated Test Cases

Go into folder test/ and run `make -B`. Then tun test_* programs for all automated test cases.

## Commands Build-Run-Test Test Cases

```bash
$ 
$ build folder/               # error: no rule to build folder/
$ build folder/ --rl          # builds library
$ build folder/ --rs -D=1     # builds program "folder/folder"
$ build folder/ --rt -D=2     # builds tools   "folder/foo" and "folder/bar"
```

## Test Folder Configurations

Project | Files/Folders
------- | -------------
all     | 
simple  | file.c file.h
lib1    | inc/ lib/ 
lib1a   | include/ library/
src1    | inc/ src/ 
src1a   | include/ source/
lib2    | inc/ lib/ lib2/
src2    | inc/ src/ src2/
all     | inc/ lib/ lib2/ src/ src2/ test/ examples/ folder/

## Dependency Test Configurations


## Manual Test Cases

