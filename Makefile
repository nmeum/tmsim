VERSION = 0.5
PROGS   = tmsim tmsim-export

OBJECTS = scanner.o parser.o turing.o token.o queue.o util.o
HEADERS = $(OBJECTS:.o=.h)

CFLAGS ?= -O3 -g
CFLAGS += -std=c99 -D_POSIX_C_SOURCE=200809L -DVERSION=\"$(VERSION)\" \
	-Wpedantic -Wall -Wextra -Wconversion -Werror

ifeq "$(findstring clang,$(shell $(CC) --version))" "clang"
	CFLAGS += -Wdocumentation
endif

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
