# =====================
# Directories
# =====================
BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj
# Repo root (two levels up) and its build directory for versioned artifacts
REPO_ROOT = $(abspath $(CURDIR)/../..)
ROOT_BUILD_DIR = $(REPO_ROOT)/build

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
	@echo "Extracting host flash tool version..."
	@VERSION_STR=$$(strings $@ | grep -m1 "Host Flash Version" || true); \
	if [ -z "$$VERSION_STR" ]; then \
	  echo "Warning: Version string not found. Using 'unknown'."; \
	  VERSION=unknown; \
	else \
	  VERSION=$$(echo $$VERSION_STR | sed -E 's/.*Version (v[0-9]+\.[0-9]+\.[0-9]+).*/\1/'); \
	  if ! echo $$VERSION | grep -Eq '^v[0-9]+\.[0-9]+\.[0-9]+'; then \
	    echo "Warning: Could not parse semantic version. Using 'unknown'."; \
	    VERSION=unknown; \
	  fi; \
	fi; \
	echo "Detected version: $$VERSION"; \
	mkdir -p $(ROOT_BUILD_DIR); \
	cp $@ $(ROOT_BUILD_DIR)/HostFlashApp_$$VERSION; \
	echo "Copied versioned binary to $(ROOT_BUILD_DIR)/HostFlashApp_$$VERSION"

# =====================
# Clean Rule
# =====================
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean