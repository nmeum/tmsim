PREFIX   ?= /usr/local
BIN_NAME ?= tmsim

OBJECTS = tmsim.o scanner.o parser.o turing.o token.o queue.o util.o
HEADERS = scanner.h parser.h turing.h token.h queue.h util.h

CFLAGS ?= -O0 -g -pedantic -Wall -Werror
CFLAGS += -std=c99 -D_POSIX_C_SOURCE=200809L

CC      ?= gcc
LDFLAGS ?= -pthread

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(BIN_NAME)
$(BIN_NAME): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(BIN_NAME) $(OBJECTS)

.PHONY: all clean
