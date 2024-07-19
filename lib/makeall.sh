#!/bin/bash -v

# make flylibc.a (static library)
rm -rf out
mkdir out
cc FlyAes.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyAes.o
cc FlyAnsi.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyAnsi.o
cc FlyAssert.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyAssert.o
cc FlyBase64.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyBase64.o
cc FlyCard.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyCard.o
cc FlyCli.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyCli.o
cc FlyFile.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyFile.o
cc FlyFileList.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyFileList.o
cc FlyJson.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyJson.o
cc FlyKey.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyKey.o
cc FlyKeyPrompt.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyKeyPrompt.o
cc FlyList.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyList.o
cc FlyLog.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyLog.o
cc FlyMarkdown.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyMarkdown.o
cc FlyMem.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyMem.o
cc FlySec.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlySec.o
cc FlySemVer.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlySemVer.o
cc FlySignal.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlySignal.o
cc FlySocket.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlySocket.o
cc FlySort.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlySort.o
cc FlyStr.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyStr.o
cc FlyStrHdr.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyStrHdr.o
cc FlyStrSmart.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyStrSmart.o
cc FlyStrZ.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyStrZ.o
cc FlyTabComplete.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyTabComplete.o
cc FlyTest.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyTest.o
cc FlyTime.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyTime.o
cc FlyToml.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyToml.o
cc FlyUtf8.c -c -I. -I../inc/ -Wall -Werror -o ./out/FlyUtf8.o
ar -crs flylibc.a out/*.o
# created library lib/flylibc.a
