#!/bin/bash -v

# make tool suite for flylibc.a
rm -rf out
mkdir out
cc flycinfo.c -c -I. -I../inc/ -Wall -Werror -o out/flycinfo.o
cc out/flycinfo.o ../lib/flylibc.a -o flycinfo
cc flyfile2c.c -c -I. -I../inc/ -Wall -Werror -o out/flyfile2c.o
cc out/flyfile2c.o ../lib/flylibc.a -o flyfile2c
cc flymd2html.c -c -I. -I../inc/ -Wall -Werror -o out/flymd2html.o
cc out/flymd2html.o ../lib/flylibc.a -o flymd2html
cc flysha.c -c -I. -I../inc/ -Wall -Werror -o out/flysha.o
cc out/flysha.o ../lib/flylibc.a -o flysha
# created tools. Try ./flycinfo
