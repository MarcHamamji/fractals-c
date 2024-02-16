CC = gcc
CFLAGS = `pkg-config --cflags gtk4 cairo`
LIBS = -lm `pkg-config --libs gtk4 cairo`

BUILD_TYPE = dev
DEBUGGING_FLAGS = -fsanitize=undefined,address -g -Og -Wall -Wextra -Wpedantic
OPTIMIZATION_FLAGS = -o3

TARGET = main.out
SRC = main.c

ifeq ($(BUILD_TYPE), dev)
    CFLAGS += $(DEBUGGING_FLAGS)
else ifeq ($(BUILD_TYPE), release)
    CFLAGS += $(OPTIMIZATION_FLAGS)
else
    $(error "Invalid build type. Valid options are: dev, release")
endif

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
