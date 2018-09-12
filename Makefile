#
# Compiler flags
#
CC     = g++
CFLAGS = -std=c++17 -Wall -Wextra # -Werror

#
# Debug/Release
#
DEBUG ?= 1
ifeq ($(DEBUG), 1)
	DIR = debug
	CFLAGS += -g2 -O0 -DDEBUG
else
	DIR = release
	CFLAGS += -O2 -s -DNDEBUG
endif

#
# Project files
#
OUT = ivy.out
FILES = $(shell find src/ -name "*.cpp")
NO_SRC = $(FILES:src/%=obj/$(DIR)/%)
OBJS = $(NO_SRC:.cpp=.o)
EXE = bin/$(DIR)/$(OUT)

# Subfolders
VPATH = $(dir $(FILES))

.PHONY: all debug release

#
# Default build
#
all: $(EXE)

#
# Build rules
#
debug: all

release: clean
	$(MAKE) DEBUG=0

$(EXE): $(OBJS)
	@mkdir -p $(dir $(EXE))
	$(CC) $(CFLAGS) -o $(EXE) $^

obj/$(DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

#
# Other rules
#
clean:
	rm -rf bin/release/* bin/debug/* obj/release/* obj/debug/*