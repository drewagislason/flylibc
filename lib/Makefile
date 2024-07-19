#  Makefile for flylibc
#  Copyright (c) 2024 Drew Gislason
#  license: <https://mit-license.org>

UNAME=$(shell uname -s)

ifdef WINDIR
	HOSTENV=Cygwin
	HOSTFLAGS= # -g
	HOST_LFLAGS= # -limagehlp
else
	ifeq ($(UNAME),Darwin)
		HOSTENV=Mac
		HOSTFLAGS= # -g
		HOST_LFLAGS= # -g
	else
		HOSTENV=Linux
		HOSTFLAGS= # -g
		HOST_LFLAGS= 
	endif
endif

# common dependancies
DEPS=../inc/Fly.h

# compile local and lib objs the same way
VPATH = .

# output folder
OUT=out

# defines

CC=gcc
INCLUDE=-I. -I../inc
DEFINES=-DFLY_DBG=1
CCFLAGS=-Wall -Werror $(HOSTFLAGS) $(INCLUDE) -o
CFLAGS=-c $(DEFINES) $(CCFLAGS)
LFLAGS=$(HOST_LFLAGS) -o

$(OUT)/%.o: %.c $(DEPS)
	$(CC) $< $(CFLAGS) $@

SRC = $(wildcard *.c)
OBJS = $(addprefix $(OUT)/,$(SRC:.c=.o))

.PHONY: clean mkout SayAll SayDone

all: SayAll mkout flylibc.a SayDone

echoobj:
	@echo src: $(SRC)
	@echo objs: $(OBJS)

SayAll:
	@echo ------------------------------
	@echo Making FireFly library...
	@echo ------------------------------

SayDone:
	@echo ------------------------------
	@echo Created FireFly library *.a
	@echo ------------------------------

flylibc.a: mkout $(OBJS)
	ar -crs $@ $(OBJS)

# clean up files that don't need to be checked in to git
clean:
	rm -rf out/
	rm -f *.a

# make the out folder
mkout:
	-if test ! -d $(OUT); then mkdir $(OUT); fi
