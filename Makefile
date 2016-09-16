PREFIX   ?= /usr/local
PROGS     = tmsim tmsim-export

OBJECTS = scanner.o parser.o turing.o token.o queue.o util.o
HEADERS = scanner.h parser.h turing.h token.h queue.h util.h

CFLAGS ?= -O0 -g -pedantic -Wall -Werror
CFLAGS += -std=c99 -D_POSIX_C_SOURCE=200809L

CC      ?= gcc
LDFLAGS ?= -pthread

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(PROGS)
tmsim: $(OBJECTS) tmsim.o
	$(CC) -o $@ $^ $(LDFLAGS)
tmsim-export: $(OBJECTS) export.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(BIN_NAME) $(OBJECTS)

.PHONY: all clean
