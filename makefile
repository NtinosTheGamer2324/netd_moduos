# Toolchain configuration
CC = x86_64-elf-gcc
LD = x86_64-elf-ld

# Directories
SRC_DIR = src
INC_DIR = include
EXAMPLE_DIR = examples
OBJ_DIR = obj
DIST_DIR = dist

# Files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))
EXAMPLE_SOURCES = $(wildcard $(EXAMPLE_DIR)/*.c)
# This creates a list of .sqr files in dist/ for every example .c file
EXAMPLES = $(patsubst $(EXAMPLE_DIR)/%.c, $(DIST_DIR)/%.sqr, $(EXAMPLE_SOURCES))

TARGET = $(DIST_DIR)/netd.sqr

# Compiler/Linker Flags
CFLAGS = -std=c11 -ffreestanding -O2 -Wall -Wextra -I$(INC_DIR) -mno-red-zone -fomit-frame-pointer
LDFLAGS = -T ld_user.ld -nostdlib --hash-style=sysv

# Build rules
all: $(DIST_DIR) $(TARGET) examples

$(DIST_DIR):
	@mkdir -p $(DIST_DIR)

# Main Daemon
$(TARGET): $(OBJECTS)
	@echo Linking daemon...
	$(LD) $(LDFLAGS) -o $@ $^

# Object file rule
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Example rule: Compiles each example to .sqr
examples: $(EXAMPLES)

$(DIST_DIR)/%.sqr: $(EXAMPLE_DIR)/%.c
	@echo Compiling example: $<
	$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$(notdir $(basename $@)).o
	$(LD) $(LDFLAGS) -o $@ $(OBJ_DIR)/$(notdir $(basename $@)).o

clean:
	rm -rf $(OBJ_DIR) $(DIST_DIR)

.PHONY: all clean examples