VERSION = 0.5
PROGS   = tmsim tmsim-export

OBJECTS = scanner.o parser.o turing.o token.o queue.o util.o
HEADERS = $(OBJECTS:.o=.h)

CFLAGS ?= -O2 -g -Wpedantic -Wall -Wextra -Werror
CFLAGS += -std=c99 -D_POSIX_C_SOURCE=200809L -DVERSION=\"$(VERSION)\"

CC      ?= gcc
LDFLAGS += -pthread

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(PROGS)
tmsim: $(OBJECTS) tmsim.o
	$(CC) -o $@ $^ $(LDFLAGS)
tmsim-export: $(OBJECTS) export.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(PROGS) $(OBJECTS) export.o tmsim.o

.PHONY: all clean
