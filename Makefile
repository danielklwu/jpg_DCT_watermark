# Compiler settings
CC = gcc
CFLAGS = -I/opt/homebrew/include -I./inc -L/opt/homebrew/lib -ljpeg -lm
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
	$(CC) $(OBJS) -o $(TARGET) $(CFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

run: $(TARGET)
	./$(TARGET) 

# Clean up (also removes all jpeg files not titled "input.jpeg")
clean:
	rm -f $(TARGET) src/*.o 
	find . -type f \( -iname "*.jpeg" -o -iname "*.jpg" \) ! -name "input.jpeg" ! -name "input2.jpeg" -exec rm {} +

.PHONY: all run clean