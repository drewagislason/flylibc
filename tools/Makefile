#  Ned Makefile for Test Suite
#  Copyright (c) 2021 Drew Gislason, All Rights Reserved.
#

UNAME=$(shell uname -s)

ifdef WINDIR
	HOSTENV=Cygwin
	HOSTFLAGS=
	HOST_LFLAGS=-limagehlp
else
	ifeq ($(UNAME),Darwin)
		HOSTENV=Mac
		HOSTFLAGS=
		HOST_LFLAGS=
	else
		HOSTENV=Linux
		HOSTFLAGS=
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
DEFINES= -DDEBUG=1
CCFLAGS=-Wall -Werror $(HOSTFLAGS) $(INCLUDE) -o
CFLAGS=-c $(DEFINES) $(CCFLAGS)
LFLAGS=$(HOST_LFLAGS) -o

$(OUT)/%.o: %.c $(DEPS)
	$(CC) $< $(CFLAGS) $@

OBJ_FLYCINFO = \
	$(OUT)/FlyStr.o \
	$(OUT)/FlyStrZ.o \
	$(OUT)/flycinfo.o

OBJ_FLYSHA = \
	$(OUT)/FlyFile.o \
	$(OUT)/FlyStr.o \
	$(OUT)/FlyStrZ.o \
	$(OUT)/flysha.o

OBJ_FLYMD2HTML = \
	$(OUT)/flymd2html.o \
	$(OUT)/FlyAssert.o \
	$(OUT)/FlyCli.o \
	$(OUT)/FlyDebug.o \
	$(OUT)/FlyFile.o \
	$(OUT)/FlyMarkdown.o \
	$(OUT)/FlyStr.o \
	$(OUT)/FlyStrZ.o

.PHONY: clean mkout SayAll SayDone

TOOLS = flycinfo flyfile2c flymd2html flysha

all: SayAll mkout $(TOOLS) SayDone

SayAll:
	@echo ------------------------------
	@echo Making FireFly tools...
	@echo ------------------------------

SayDone:
	@echo ------------------------------
	@echo Created FireFly tools: $(TOOLS)
	@echo ------------------------------

flycinfo: mkout $(OBJ_FLYCINFO)
	$(CC) $(LFLAGS) $@ $(OBJ_FLYCINFO)
	@echo Linked $@ ...

flyfile2c: mkout $(OUT)/flyfile2c.o
	$(CC) $(LFLAGS) $@ $(OUT)/flyfile2c.o
	@echo Linked $@ ...

flymd2html: mkout $(OBJ_FLYMD2HTML)
	$(CC) $(LFLAGS) $@ $(OBJ_FLYMD2HTML)
	@echo Linked $@ ...

flysha: mkout $(OBJ_FLYSHA)
	$(CC) $(LFLAGS) $@ $(OBJ_FLYSHA)
	@echo Linked $@ ...


# clean up files that don't need to be checked in to git
# "test_*" are test case executables, "tmp_*" are temporary test case data
clean:
	rm -rf out/
	rm -f *.log
	rm -f $(TOOLS)
	rm -f tmp.*

# make the out folder
mkout:
	-if test ! -d $(OUT); then mkdir $(OUT); fi

