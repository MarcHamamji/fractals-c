CC = gcc
PKG_CFLAGS := $(shell pkg-config --cflags gtk4 cairo)
PKG_LIBS   := $(shell pkg-config --libs gtk4 cairo)

CFLAGS = -std=c2x $(PKG_CFLAGS) -fopenmp
LDFLAGS =
LIBS = -lm $(PKG_LIBS) -lgomp

BUILD_TYPE ?= release
DEBUGGING_FLAGS = -fsanitize=undefined,address -g3 -Og -Wall -Wpedantic
OPTIMIZATION_FLAGS = -O3

SRC := $(shell find src -name "*.c")
OBJS := $(patsubst src/%.c,build/%.o,$(SRC))
TARGET = main.out

ifeq ($(BUILD_TYPE), dev)
	CFLAGS += $(DEBUGGING_FLAGS)
	LDFLAGS += $(DEBUGGING_FLAGS)
else ifeq ($(BUILD_TYPE), release)
	CFLAGS += $(OPTIMIZATION_FLAGS)
else
	$(error "Invalid build type. Valid options are: dev, release")
endif

.PHONY: all clean run

all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@ $(LIBS)

# Compile each .c into build/%.o
build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build $(TARGET)
