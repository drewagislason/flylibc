#  Makefile for FlyLib Test Suite
#  Copyright (c) 2022 Drew Gislason, All Rights Reserved.
#

UNAME=$(shell uname -s)

ifdef WINDIR
	HOSTENV=Windows
	HOSTFLAGS= #-g
	HOST_LFLAGS= #-limagehlp
else
	ifeq ($(UNAME),Darwin)
		HOSTENV=MacOs
		HOSTFLAGS= -g
		HOST_LFLAGS= -g
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

OBJS_TEST_BASE = \
	$(OUT)/FlyAnsi.o \
	$(OUT)/FlyAssert.o \
	$(OUT)/FlyCli.o \
	$(OUT)/FlyLog.o \
	$(OUT)/FlyFile.o \
	$(OUT)/FlyStr.o \
	$(OUT)/FlyStrZ.o \
	$(OUT)/FlyTest.o

OBJ_TEST_AES = \
	$(OBJS_TEST_BASE) \
	$(OUT)/FlyAes.o \
	$(OUT)/test_aes.o

OBJ_TEST_ANSI = \
	$(OBJS_TEST_BASE) \
	$(OUT)/test_ansi.o

OBJ_TEST_CLI = \
	$(OBJS_TEST_BASE) \
	$(OUT)/test_cli.o

OBJ_TEST_BASE64 = \
	$(OBJS_TEST_BASE) \
	$(OUT)/FlyBase64.o \
	$(OUT)/test_base64.o

OBJ_TEST_EXAMPLE = \
	$(OBJS_TEST_BASE) \
	$(OUT)/test_example.o

OBJ_TEST_FILE = \
	$(OBJS_TEST_BASE) \
	$(OUT)/test_file.o

OBJ_TEST_FILE_LIST = \
	$(OBJS_TEST_BASE) \
	$(OUT)/FlyFileList.o \
	$(OUT)/FlyMem.o \
	$(OUT)/test_flist.o

OBJ_TEST_JSON = \
	$(OBJS_TEST_BASE) \
	$(OUT)/FlyJson.o \
	$(OUT)/test_json.o

OBJ_TEST_KEY = \
	$(OBJS_TEST_BASE) \
	$(OUT)/FlyKey.o \
	$(OUT)/FlyKeyPrompt.o \
	$(OUT)/test_key.o

OBJ_TEST_LIST = \
	$(OBJS_TEST_BASE) \
	$(OUT)/FlyList.o \
	$(OUT)/test_list.o

OBJ_TEST_LOG = \
	$(OBJS_TEST_BASE) \
	$(OUT)/test_log.o

OBJ_TEST_MARKDOWN = \
	$(OBJS_TEST_BASE) \
	$(OUT)/FlySignal.o \
	$(OUT)/FlyMarkdown.o \
	$(OUT)/test_markdown.o

OBJ_TEST_SOCKET = \
	$(OBJS_TEST_BASE) \
	$(OUT)/FlySocket.o \
	$(OUT)/test_socket.o

OBJ_TEST_SEC = \
	$(OBJS_TEST_BASE) \
	$(OUT)/FlyAes.o \
	$(OUT)/FlySec.o \
	$(OUT)/test_sec.o

OBJ_TEST_SEM_VER = \
	$(OBJS_TEST_BASE) \
	$(OUT)/FlySemVer.o \
	$(OUT)/test_semver.o

OBJ_TEST_SIGNAL = \
	$(OBJS_TEST_BASE) \
	$(OUT)/FlySignal.o \
	$(OUT)/test_signal.o

OBJ_TEST_SORT = \
	$(OBJS_TEST_BASE) \
	$(OUT)/FlySort.o \
	$(OUT)/test_sort.o

OBJ_TEST_STR = \
	$(OBJS_TEST_BASE) \
	$(OUT)/FlyStrHdr.o \
	$(OUT)/test_str.o

OBJ_TEST_STRSMART = \
	$(OBJS_TEST_BASE) \
	$(OUT)/FlyStrSmart.o \
	$(OUT)/test_smart.o

OBJ_TEST_TIME = \
	$(OBJS_TEST_BASE) \
	$(OUT)/FlyTime.o \
	$(OUT)/test_time.o

OBJ_TEST_TOML = \
	$(OBJS_TEST_BASE) \
	$(OUT)/FlyToml.o \
	$(OUT)/FlyUtf8.o \
	$(OUT)/test_toml.o

OBJ_TEST_UTF8 = \
	$(OBJS_TEST_BASE) \
	$(OUT)/FlyUtf8.o \
	$(OUT)/test_utf8.o

OBJ_FLY_CLIENT = \
	$(OUT)/FlySocket.o \
	$(OUT)/test_client.o

OBJ_FLY_SERVER = \
	$(OUT)/FlySocket.o \
	$(OUT)/test_server.o

TEST_CASES = test_aes test_ansi test_base64 test_cli test_example test_file test_flist test_json test_key \
  test_list test_log test_markdown test_sec test_semver test_signal test_smart test_sort test_str \
  test_time test_toml test_utf8

.PHONY: clean mkout SayAll SayDone

all: SayAll mkout $(TEST_CASES) SayDone

SayAll:
	@echo ------------------------------
	@echo Making FireFly test suite...
	@echo ------------------------------

SayDone:
	@echo ------------------------------
	@echo Created FireFly suite: $(TEST_CASES)
	@echo ------------------------------

test_aes: mkout $(OBJ_TEST_AES)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_AES)
	@echo Linked $@ ...

test_ansi: mkout $(OBJ_TEST_ANSI)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_ANSI)
	@echo Linked $@ ...

test_base64: mkout $(OBJ_TEST_BASE64)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_BASE64)
	@echo Linked $@ ...

test_cli: mkout $(OBJ_TEST_CLI)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_CLI)
	@echo Linked $@ ...

test_example: mkout $(OBJ_TEST_EXAMPLE)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_EXAMPLE)
	@echo Linked $@ ...

test_file: mkout $(OBJ_TEST_FILE)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_FILE)
	@echo Linked $@ ...

test_flist: mkout $(OBJ_TEST_FILE_LIST)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_FILE_LIST)
	@echo Linked $@ ...

test_json: mkout $(OBJ_TEST_JSON)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_JSON)
	@echo Linked $@ ...

test_key: mkout $(OBJ_TEST_KEY)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_KEY)
	@echo Linked $@ ...

test_list: mkout $(OBJ_TEST_LIST)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_LIST)
	@echo Linked $@ ...

test_log: mkout $(OBJ_TEST_LOG)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_LOG)
	@echo Linked $@ ...

test_markdown: mkout $(OBJ_TEST_MARKDOWN)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_MARKDOWN)
	@echo Linked $@ ...

test_semver: mkout $(OBJ_TEST_SEM_VER)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_SEM_VER)
	@echo Linked $@ ...

test_sec: mkout $(OBJ_TEST_SEC)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_SEC)
	@echo Linked $@ ...

test_signal: mkout $(OBJ_TEST_SIGNAL)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_SIGNAL)
	@echo Linked $@ ...

test_socket: mkout $(OBJ_TEST_SOCKET)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_SOCKET)
	@echo Linked $@ ...

test_sort: mkout $(OBJ_TEST_SORT)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_SORT)
	@echo Linked $@ ...

test_str: mkout $(OBJ_TEST_STR)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_STR)
	@echo Linked $@ ...

test_smart: mkout $(OBJ_TEST_STRSMART)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_STRSMART)
	@echo Linked $@ ...

test_time: mkout $(OBJ_TEST_TIME)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_TIME)
	@echo Linked $@ ...

test_toml: mkout $(OBJ_TEST_TOML)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_TOML)
	@echo Linked $@ ...

test_utf8: mkout $(OBJ_TEST_UTF8)
	$(CC) $(LFLAGS) $@ $(OBJ_TEST_UTF8)
	@echo Linked $@ ...

flyclient: mkout $(OBJ_FLY_CLIENT)
	$(CC) $(LFLAGS) $@ $(OBJ_FLY_CLIENT)
	@echo Linked $@ ...

flyserver: mkout $(OBJ_FLY_SERVER)
	$(CC) $(LFLAGS) $@ $(OBJ_FLY_SERVER)
	@echo Linked $@ ...

# clean up files that don't need to be checked in to git
# "test_*" are test case executables, "tmp_*" are temporary test case data
clean:
	rm -rf out/
	rm -f $(TEST_CASES)
	rm -f *.log
	rm -f tmp_*
	rm -f tmp.*

# make the out folder
mkout:
	-if test ! -d $(OUT); then mkdir $(OUT); fi
