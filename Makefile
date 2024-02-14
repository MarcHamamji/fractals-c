CC = gcc -o3
CFLAGS = `pkg-config --cflags gtk4 cairo`
LIBS = -lm `pkg-config --libs gtk4 cairo`

TARGET = main.out
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
