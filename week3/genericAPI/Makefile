CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 
all: main

main : main.o function.o
	${CC} main.o function.o

main.o: main.c
	${CC} ${CFLAGS} main.c

function.o: function.c
	${CC} ${CFLAGS} function.c

clean:
	rm -f *.o *~
