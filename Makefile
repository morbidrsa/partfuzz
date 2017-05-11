CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -pedantic
PROG=partfuzz

ifeq ("$(origin DEBUGFLAGS)","command line")
	CFLAGS+=-ggdb
else
	CFLAGS+=-O2
endif

all: $(PROG)

OBJS := partfuzz.o

$(PROG): $(OBJS)
	$(CC) $< -o $@

*.o: *.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -f *.o $(PROG)
