#!/bin/bash -v

# make test suite for flylibc.a
rm -rf out
mkdir out
cc test_aes.c -c -I. -I../inc/ -Wall -Werror -o out/test_aes.o
cc out/test_aes.o ../lib/flylibc.a -o test_aes
cc test_ansi.c -c -I. -I../inc/ -Wall -Werror -o out/test_ansi.o
cc out/test_ansi.o ../lib/flylibc.a -o test_ansi
cc test_base64.c -c -I. -I../inc/ -Wall -Werror -o out/test_base64.o
cc out/test_base64.o ../lib/flylibc.a -o test_base64
cc test_cli.c -c -I. -I../inc/ -Wall -Werror -o out/test_cli.o
cc out/test_cli.o ../lib/flylibc.a -o test_cli
cc test_example.c -c -I. -I../inc/ -Wall -Werror -o out/test_example.o
cc out/test_example.o ../lib/flylibc.a -o test_example
cc test_file.c -c -I. -I../inc/ -Wall -Werror -o out/test_file.o
cc out/test_file.o ../lib/flylibc.a -o test_file
cc test_flist.c -c -I. -I../inc/ -Wall -Werror -o out/test_flist.o
cc out/test_flist.o ../lib/flylibc.a -o test_flist
cc test_json.c -c -I. -I../inc/ -Wall -Werror -o out/test_json.o
cc out/test_json.o ../lib/flylibc.a -o test_json
cc test_key.c -c -I. -I../inc/ -Wall -Werror -o out/test_key.o
cc out/test_key.o ../lib/flylibc.a -o test_key
cc test_list.c -c -I. -I../inc/ -Wall -Werror -o out/test_list.o
cc out/test_list.o ../lib/flylibc.a -o test_list
cc test_log.c -c -I. -I../inc/ -Wall -Werror -o out/test_log.o
cc out/test_log.o ../lib/flylibc.a -o test_log
cc test_sec.c -c -I. -I../inc/ -Wall -Werror -o out/test_sec.o
cc out/test_sec.o ../lib/flylibc.a -o test_sec
cc test_semver.c -c -I. -I../inc/ -Wall -Werror -o out/test_semver.o
cc out/test_semver.o ../lib/flylibc.a -o test_semver
cc test_signal.c -c -I. -I../inc/ -Wall -Werror -o out/test_signal.o
cc out/test_signal.o ../lib/flylibc.a -o test_signal
cc test_smart.c -c -I. -I../inc/ -Wall -Werror -o out/test_smart.o
cc out/test_smart.o ../lib/flylibc.a -o test_smart
cc test_sort.c -c -I. -I../inc/ -Wall -Werror -o out/test_sort.o
cc out/test_sort.o ../lib/flylibc.a -o test_sort
cc test_str.c -c -I. -I../inc/ -Wall -Werror -o out/test_str.o
cc out/test_str.o ../lib/flylibc.a -o test_str
cc test_time.c -c -I. -I../inc/ -Wall -Werror -o out/test_time.o
cc out/test_time.o ../lib/flylibc.a -o test_time
cc test_toml.c -c -I. -I../inc/ -Wall -Werror -o out/test_toml.o
cc out/test_toml.o ../lib/flylibc.a -o test_toml
cc test_utf8.c -c -I. -I../inc/ -Wall -Werror -o out/test_utf8.o
cc out/test_utf8.o ../lib/flylibc.a -o test_utf8
# created tests. Try ./test_str
