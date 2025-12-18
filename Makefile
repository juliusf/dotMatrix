CC=clang
CFLAGS=--std=c11 -pedantic -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-parameter -Wno-overlength-strings -g
LDFLAGS=-lraylib -lpthread
SOURCES=src/main.c src/util.c src/cpu.c src/interconnect.c src/video.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=bin/dotMatrix

# Detect OS for platform-specific flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	HOMEBREW_PREFIX := $(shell brew --prefix)
	CFLAGS += -I$(HOMEBREW_PREFIX)/include
	LDFLAGS += -L$(HOMEBREW_PREFIX)/lib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
endif
ifeq ($(UNAME_S),Linux)
	LDFLAGS += -lm -ldl -lrt
endif

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	    $(CC) $(LDFLAGS) $(OBJECTS) -o $@

debug: CFLAGS += -DDEBUG 
debug: all

clean:
	rm -f src/*.o
