CC=gcc
CFLAGS=-Wall -Wextra -std=c99
PROG=partfuzz

ifeq ("$(origin DEBUGFLAGS)","command line")
	CFLAGS+=-ggdb -O0
else
	CFLAGS+=-O2
endif

ifeq ("$(origin V)", "command line")
	BUILD_VERBOSE = $(V)
endif
ifndef BUILD_VERBOSE
	BUILD_VERBOSE = 0
endif

ifeq ($(BUILD_VERBOSE), 1)
	Q =
else
	Q = @
endif

all: $(PROG)

OBJS := partfuzz.o

$(PROG): $(OBJS)
	@echo "    [LD]    $@"
	$(Q)$(CC) $< -o $@

$(OBJS): *.c
	@echo "    [CC]    $@"
	$(Q)$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	@echo "    [CLEAN] partfuzz"
	$(Q)rm -f *.o $(PROG)
