# Compiler settings
CC = gcc
# CFLAGS = -I./inc $(shell pkg-config --cflags MagickWand)
CFLAGS = -I./inc -I/opt/homebrew/include $(shell pkg-config --cflags MagickWand)
LDFLAGS = -L/opt/homebrew/lib -ljpeg -lm $(shell pkg-config --libs MagickWand)
SRC_DIR = src
INC_DIR = inc
OBJ_DIR = obj

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET = watermark

# Create obj directory if it doesn't exist
$(shell mkdir -p $(OBJ_DIR))

# Build the executable
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

test1: $(TARGET)
	./$(TARGET) input1.jpg

test2: $(TARGET)
	./$(TARGET) input2.jpeg

test3: $(TARGET)
	./$(TARGET) input3.png

test4: $(TARGET)
	./$(TARGET) 000000.jpg

test_corrected: $(TARGET)
	./$(TARGET) distorted_inputs/barrel_000000.jpg
	./$(TARGET) corrected_outputs/barrel.jpg
# ./$(TARGET) corrected_outputs/rotation.jpg
# ./$(TARGET) corrected_outputs/barrel.jpg
# ./$(TARGET) corrected_outputs/barrel.jpg


# Clean up (also removes all jpeg files not titled "input.jpeg")
clean:
	rm -f $(TARGET) main.o obj/*.o
#	find . -type f \( -iname "*.jpeg" -o -iname "*.jpg" -o -iname "*.png" \) ! -name "input*" -exec rm {} +

.PHONY: all run clean