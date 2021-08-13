all: play

play: assembler.o conversion.o validation.o parser.o creator.o
	gcc -ansi -Wall -pedantic -o assembler assembler.o conversion.o validation.o parser.o creator.o

assembler.o: assembler.c
	gcc -ansi -Wall -pedantic -c assembler.c assembler.h

conversion.o: conversion.c
	gcc -ansi -Wall -pedantic -c conversion.c conversion.h

validation.o: validation.c
	gcc -ansi -Wall -pedantic -c validation.c validation.h

parser.o: parser.c
	gcc -ansi -Wall -pedantic -c parser.c parser.h

creator.o: creator.c
	gcc -ansi -Wall -pedantic -c creator.c creator.h

clean:
	rm *.o assembler

misc_clean:
	rm *.gch *.ent *.ob *.ext

clean_all:
	rm *.o assembler *.gch *.ent *.ob *.ext
