CC=gcc
CFLAGS=-Wall -g 

all:		bidi tap

clean:
		rcsclean *
		rm -f *~
		rm -f bidi
		rm -f tap

bidi:		bidi.c
		$(CC) $(CFLAGS) -o $@ $<

tap:		tap.c
		$(CC) $(CFLAGS) -o $@ $<
