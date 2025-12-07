# =====================
# Directories
# =====================
TEST_DIR := .
UNITY_DIR := unity
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj

# =====================
# Toolchain
# =====================
CC := gcc

# =====================
# Unity Framework
# =====================
UNITY_SRC := $(UNITY_DIR)/unity.c

# =====================
# Test Files
# =====================
TEST_SRCS := test_logger.c \
             test_crc_helper.c \
             test_etx_crc.c \
             test_conf_helper.c

# Source files to test
COMMON_SRC := ../Common/Core/Src/logger.c

# =====================
# Include Paths
# =====================
INCLUDES := -I$(UNITY_DIR) \
            -I$(TEST_DIR) \
            -I../Common/Core/Inc \
            -I../Bootloader/Core/Inc

# =====================
# Compiler Flags
# =====================
CFLAGS := -Wall -Wextra -g -O0 $(INCLUDES)

# =====================
# Test Executables
# =====================
TEST_BINS := $(patsubst %.c,$(BUILD_DIR)/%,$(TEST_SRCS))

# =====================
# Default Target
# =====================
.PHONY: all
all: $(BUILD_DIR) $(OBJ_DIR) $(TEST_BINS)

# =====================
# Build Rules
# =====================

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR): | $(BUILD_DIR)
	mkdir -p $(OBJ_DIR)

# Compile Unity framework
$(OBJ_DIR)/unity.o: $(UNITY_SRC) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile common source (logger)
$(OBJ_DIR)/logger.o: $(COMMON_SRC) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build test_logger
$(BUILD_DIR)/test_logger: test_logger.c $(OBJ_DIR)/unity.o $(OBJ_DIR)/logger.o | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Build test_crc_helper
$(BUILD_DIR)/test_crc_helper: test_crc_helper.c $(OBJ_DIR)/unity.o | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Build test_etx_crc
$(BUILD_DIR)/test_etx_crc: test_etx_crc.c $(OBJ_DIR)/unity.o | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Build test_conf_helper
$(BUILD_DIR)/test_conf_helper: test_conf_helper.c $(OBJ_DIR)/unity.o | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# =====================
# Run Tests
# =====================
.PHONY: test
test: all
	@echo "==================================="
	@echo "Running Unit Tests"
	@echo "==================================="
	@for test in $(TEST_BINS); do \
		echo ""; \
		echo "Running $$test..."; \
		echo "-----------------------------------"; \
		./$$test || exit 1; \
	done
	@echo ""
	@echo "==================================="
	@echo "All tests passed!"
	@echo "==================================="

# =====================
# Clean Rule
# =====================
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

# =====================
# Help
# =====================
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all    - Build all test executables (default)"
	@echo "  test   - Build and run all tests"
	@echo "  clean  - Remove build artifacts"
	@echo "  help   - Show this help message"
