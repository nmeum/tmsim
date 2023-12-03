# This is a POSIX Makefile as per IEEE P1003.1™-202x/D3.
.POSIX:

VERSION = 1.0.0
PROGS   = tmsim tmsim-export

SOURCES = scanner.c parser.c turing.c token.c queue.c util.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = $(SOURCES:.c=.h)

CFLAGS ?= -O3 -g -Werror
CFLAGS += -std=c99 -D_POSIX_C_SOURCE=200809L -DVERSION='"$(VERSION)"' \
	-Wpedantic -Wall -Wextra -Wconversion -Wmissing-prototypes \
	-Wpointer-arith -Wstrict-prototypes -Wshadow -Wcast-align

CC      ?= gcc
LDFLAGS += -pthread

all: $(PROGS)
$(OBJECTS): $(HEADERS)

tmsim: $(OBJECTS) tmsim.o
	$(CC) -o $@ $^ $(LDFLAGS)
tmsim-export: $(OBJECTS) export.o
	$(CC) -o $@ $^ $(LDFLAGS)

test: tmsim
	cd tests/ && ./run_tests.sh

format:
	clang-format -style=file -i $(SOURCES) $(HEADERS)

clean:
	$(RM) $(PROGS) $(OBJECTS) export.o tmsim.o

.PHONY: all clean format test
