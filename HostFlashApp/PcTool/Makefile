# =====================
# Directories
# =====================
BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj

# =====================
# Toolchain
# =====================
CC      = gcc
OBJCOPY = objcopy
SIZE    = size

# =====================
# Source Files
# =====================
C_SRCS = Src/etx_flash_update.c RS232/rs232.c

# =====================
# Object Files
# =====================
OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(C_SRCS:.c=.o)))

# =====================
# Include Paths
# =====================
INCLUDES = -IInc -IRS232

# =====================
# Compiler Flags
# =====================
CFLAGS   = -Wall -Wextra -Wshadow -Wformat-nonliteral -Wformat-security -Wtype-limits -O2 $(INCLUDES)

# =====================
# Output Files
# =====================
BIN      = $(BUILD_DIR)/HostFlashApp

# =====================
# Default Target
# =====================
all: $(BIN)

# =====================
# vpath for source files
# =====================
vpath %.c $(sort $(dir $(C_SRCS)))

# =====================
# Build Rules
# =====================

# Single build rule for all C sources
$(OBJ_DIR)/%.o: %.c | $(BUILD_DIR) $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR): | $(BUILD_DIR)
	mkdir -p $(OBJ_DIR)

# Link objects to bin
$(BIN): $(OBJS) | $(BUILD_DIR)
	$(CC) $^ $(CFLAGS) -o $@

# =====================
# Clean Rule
# =====================
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean