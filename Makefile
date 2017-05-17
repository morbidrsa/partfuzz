CC=gcc
CFLAGS=-Wall -Wextra -std=c99
PROG=partfuzz

ifeq ("$(origin DEBUGFLAGS)","command line")
	CFLAGS+=-ggdb -O0
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
