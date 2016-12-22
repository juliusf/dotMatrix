CC=clang
CFLAGS=--std=c11 -pedantic -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-parameter -Wno-overlength-strings -g
LDFLAGS=
SOURCES=src/main.c src/util.c src/cpu.c src/interconnect.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=bin/dotMatrix

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	    $(CC) $(LDFLAGS) $(OBJECTS) -o $@

debug: CFLAGS += -DDEBUG 
debug: all

clean:
	rm -f src/*.o
