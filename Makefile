# Compiler settings
CC = gcc
CFLAGS = -I/opt/homebrew/include -I./inc -L/opt/homebrew/lib -ljpeg
TARGET = main
SRC = src/main.c src/read.c src/write.c src/embed.c

# Build the executable
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS)

run: $(TARGET)
	./$(TARGET) input.jpeg watermarked.jpeg dkwu

test2: $(TARGET)
	./$(TARGET) input2.jpeg watermarked.jpeg test

# Clean up (also removes all jpeg files not titled "input.jpeg")
clean:
	rm -f $(TARGET) src/*.o 
	find . -type f \( -iname "*.jpeg" -o -iname "*.jpg" \) ! -name "input.jpeg" -exec rm {} +

.PHONY: all run clean