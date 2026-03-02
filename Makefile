# ============================================================
# VARIABLES
# ============================================================

# Compiler
CC = gcc

# Compiler flags
# -Wall -Wextra : Show all warnings
# -Werror       : Treat warnings as errors
# -std=c17      : Use modern C standard
# -g            : Include debug symbols (for gdb)
CFLAGS = -Wall -Wextra -Werror -std=c17 -g

# Linker flags
# -lm : Link math library
LDFLAGS = -lm

# Target executable name
TARGET = chip8

# Source files
SRCS = src/main.c src/chip8.c src/chip8_log.c

# ============================================================
# DEFAULT TARGET
# ============================================================

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

# ============================================================
# CLEAN TARGET
# ============================================================

.PHONY: clean

clean:
	rm -f $(TARGET)

# ============================================================
# CONVENIENCE: RUN TARGET
# ============================================================

.PHONY: run

run: $(TARGET)
	./$(TARGET)