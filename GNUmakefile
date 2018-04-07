VERSION = 0.9.9
PROGS   = tmsim tmsim-export

SOURCES = scanner.c parser.c turing.c token.c queue.c util.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = $(SOURCES:.c=.h)

CFLAGS ?= -O3 -g
CFLAGS += -std=c99 -D_POSIX_C_SOURCE=200809L -DVERSION=\"$(VERSION)\" \
	-Wpedantic -Wall -Wextra -Wconversion -Wmissing-prototypes -Werror

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

test: tmsim
	cd tests/ && ./run_tests.sh

format:
	clang-format -style=file -i $(SOURCES) $(HEADERS)

clean:
	$(RM) $(PROGS) $(OBJECTS) export.o tmsim.o

.PHONY: all clean format test
