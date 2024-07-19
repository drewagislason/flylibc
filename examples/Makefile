#  Makefile for FlyLib Examples
#  Copyright (c) 2024 Drew Gislason, All Rights Reserved.
#

UNAME=$(shell uname -s)

ifdef WINDIR
	HOSTENV=Cygwin
	HOSTFLAGS= #-g
	HOST_LFLAGS= #-limagehlp
else
	ifeq ($(UNAME),Darwin)
		HOSTENV=Mac
		HOSTFLAGS= #-g
		HOST_LFLAGS= #-g
	else
		HOSTENV=Linux
		HOSTFLAGS= #-g
		HOST_LFLAGS= 
	endif
endif

# common dependancies
DEPS=../inc/Fly.h

# compile local and lib objs the same way
VPATH = .:../lib

# output folder
OUT=out

# defines

CC=gcc
INCLUDE=-I. -I../inc
DEFINES=
CCFLAGS=-Wall -Werror $(HOSTFLAGS) $(INCLUDE) -o
CFLAGS=-c $(DEFINES) $(CCFLAGS)
LFLAGS=$(HOST_LFLAGS) -o

$(OUT)/%.o: %.c $(DEPS)
	$(CC) $< $(CFLAGS) $@

OBJ_EXAMPLE_BASE64 = \
	$(OUT)/FlyBase64.o \
	$(OUT)/FlyCli.o \
	$(OUT)/FlyFile.o \
	$(OUT)/FlyStr.o \
	$(OUT)/FlyStrZ.o \
	$(OUT)/example_base64.o

OBJ_EXAMPLE_CLI = \
	$(OUT)/FlyCli.o \
	$(OUT)/example_cli.o

OBJ_EXAMPLE_FILELIST = \
	$(OUT)/FlyFile.o \
	$(OUT)/FlyFileList.o \
	$(OUT)/FlyMem.o \
	$(OUT)/FlyStr.o \
	$(OUT)/FlyStrZ.o \
	$(OUT)/example_filelist.o

OBJ_EXAMPLE_FILEINFO = \
	$(OUT)/FlyFile.o \
	$(OUT)/FlyStr.o \
	$(OUT)/FlyStrZ.o \
	$(OUT)/example_fileinfo.o

OBJ_EXAMPLE_KEY = \
	$(OUT)/FlyKey.o \
	$(OUT)/example_key.o

OBJ_EXAMPLE_TOML = \
	$(OUT)/FlyFile.o \
	$(OUT)/FlyStr.o \
	$(OUT)/FlyStrZ.o \
	$(OUT)/FlyToml.o \
	$(OUT)/FlyUtf8.o \
	$(OUT)/example_toml.o

OBJ_EXAMPLE_UTF8 = \
	$(OUT)/FlyStr.o \
	$(OUT)/FlyStrZ.o \
	$(OUT)/FlyUtf8.o \
	$(OUT)/example_utf8.o

EXAMPLES = example_base64 example_cli example_fileinfo example_filelist example_key example_toml example_utf8

.PHONY: clean mkout SayAll SayDone

all: SayAll mkout $(EXAMPLES) SayDone

SayAll:
	@echo ------------------------------
	@echo Making flylib examples...
	@echo ------------------------------

SayDone:
	@echo ------------------------------
	@echo Created flylib examples: $(EXAMPLES)
	@echo ------------------------------

example_base64: mkout $(OBJ_EXAMPLE_BASE64)
	$(CC) $(LFLAGS) $@ $(OBJ_EXAMPLE_BASE64)
	@echo Linked $@ ...

example_cli: mkout $(OBJ_EXAMPLE_CLI)
	$(CC) $(LFLAGS) $@ $(OBJ_EXAMPLE_CLI)
	@echo Linked $@ ...

example_fileinfo: mkout $(OBJ_EXAMPLE_FILEINFO)
	$(CC) $(LFLAGS) $@ $(OBJ_EXAMPLE_FILEINFO)
	@echo Linked $@ ...

example_filelist: mkout $(OBJ_EXAMPLE_FILELIST)
	$(CC) $(LFLAGS) $@ $(OBJ_EXAMPLE_FILELIST)
	@echo Linked $@ ...

example_key: mkout $(OBJ_EXAMPLE_KEY)
	$(CC) $(LFLAGS) $@ $(OBJ_EXAMPLE_KEY)
	@echo Linked $@ ...

example_toml: mkout $(OBJ_EXAMPLE_TOML)
	$(CC) $(LFLAGS) $@ $(OBJ_EXAMPLE_TOML)
	@echo Linked $@ ...

example_utf8: mkout $(OBJ_EXAMPLE_UTF8)
	$(CC) $(LFLAGS) $@ $(OBJ_EXAMPLE_UTF8)
	@echo Linked $@ ...

# clean up files that don't need to be checked in to git
# "test_*" are test case executables, "tmp_*" are temporary test case data
clean:
	rm -rf out/
	rm -f $(EXAMPLES)
	rm -f *.log
	rm -f tmp_*
	rm -f tmp.*

# make the out folder
mkout:
	-if test ! -d $(OUT); then mkdir $(OUT); fi
