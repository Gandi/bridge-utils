CC=gcc
CFLAGS=-Wall -g 

all:		bidi

clean:
		rcsclean *
		rm -f *~
		rm -f bidi

bidi:		bidi.c
		$(CC) $(CFLAGS) -o $@ $<
