#!/bin/bash -v

# make flylibc.a (static library)
rm -rf out
mkdir out
cc example_base64.c -c -I. -I../inc/ -Wall -Werror -o out/example_base64.o
cc out/example_base64.o ../lib/flylibc.a -o example_base64
cc example_cli.c -c -I. -I../inc/ -Wall -Werror -o out/example_cli.o
cc out/example_cli.o ../lib/flylibc.a -o example_cli
cc example_cli.c -c -I. -I../inc/ -Wall -Werror -o out/example_cli.o
cc out/example_cli.o ../lib/flylibc.a -o example_cli
cc example_fileinfo.c -c -I. -I../inc/ -Wall -Werror -o out/example_fileinfo.o
cc out/example_fileinfo.o ../lib/flylibc.a -o example_fileinfo
cc example_filelist.c -c -I. -I../inc/ -Wall -Werror -o out/example_filelist.o
cc out/example_filelist.o ../lib/flylibc.a -o example_filelist
cc example_key.c -c -I. -I../inc/ -Wall -Werror -o out/example_key.o
cc out/example_key.o ../lib/flylibc.a -o example_toml
cc example_toml.c -c -I. -I../inc/ -Wall -Werror -o out/example_toml.o
cc out/example_toml.o ../lib/flylibc.a -o example_key
cc example_utf8.c -c -I. -I../inc/ -Wall -Werror -o out/example_utf8.o
cc out/example_utf8.o ../lib/flylibc.a -o example_utf8
# created examples. Try ./example_cli --help
