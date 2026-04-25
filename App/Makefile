# =====================
# Directories
# =====================
BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj
# Repo root (one level up) and its build directory for versioned artifacts
REPO_ROOT = $(abspath $(CURDIR)/..)
ROOT_BUILD_DIR = $(REPO_ROOT)/build

# =====================
# Toolchain
# =====================
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

# =====================
# Source Files
# =====================
C_SRCS = $(shell cat sources.list)
ASM_SRCS = $(shell cat asm_sources.list)

# =====================
# Object Files
# =====================
OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(C_SRCS:.c=.o))) \
	$(addprefix $(OBJ_DIR)/, $(notdir $(ASM_SRCS:.s=.o)))

# =====================
# Include Paths
# =====================
INCLUDES = -ICore/Inc \
		   -ICommon/Inc \
		   -IDrivers/STM32H7xx_HAL_Driver/Inc \
		   -IDrivers/CMSIS/Include \
		   -IDrivers/CMSIS/Device/ST \
		   -IDrivers/CMSIS/Device/ST/STM32H7xx/Include \
		   -IFreeRTOS/portable/GCC/ARM_CM7/r0p1 \
		   -IFreeRTOS/portable/MemMang \
		   -IFreeRTOS/include \
		   -IESP-Hosted \
		   -IESP-Hosted/api/include \
		   -IESP-Hosted/api/priv \
		   -IESP-Hosted/drivers/rpc/core \
		   -IESP-Hosted/drivers/rpc/slaveif \
		   -IESP-Hosted/drivers/rpc/wrap \
		   -IESP-Hosted/drivers/serial \
		   -IESP-Hosted/drivers/mempool \
		   -IESP-Hosted/drivers/transport \
		   -IESP-Hosted/drivers/transport/sdio \
		   -IESP-Hosted/drivers/virtual_serial_if \
		   -IESP-Hosted/port/stm32 \
		   -Iesp-idf/components/esp_common/include \
		   -Iesp-idf/components/esp_hw_support/include \
		   -Iesp-idf/components/esp_wifi/include \
		   -Iesp-idf/components/esp_netif/include \
		   -Iesp-idf/components/esp_event/include \
		   -Iesp-idf/components/esp_system/include \
		   -Iesp-idf/components/esp_timer/include \
		   -Iesp-idf/components/log/include \
		   -Iesp-idf/components/wpa_supplicant/include \
		   -Iesp-idf/components/esp_rom/include \
		   -Iesp-idf/components/soc/esp32/include \
		   -Iesp-idf/components/freertos/FreeRTOS-Kernel-SMP/include/freertos \
		   -Iesp-idf/components/esp_coex/include 

# =====================
# Compiler Flags
# =====================
CFLAGS   = -mcpu=cortex-m7 -mthumb -O2 -g -Wall -ffunction-sections -fdata-sections $(INCLUDES) -DUSE_HAL_DRIVER -DSTM32H755xx -DCORE_CM7 -mfloat-abi=hard -mfpu=fpv5-d16

# =====================
# Output Files
# =====================
ELF      = $(BUILD_DIR)/app.elf
BIN      = $(BUILD_DIR)/app.bin

# =====================
# Size Limits
# =====================
MAX_SIZE = 1048576  # 1 MB in bytes

# =====================
# Linker Flags
# =====================
LDFLAGS = -TSTM32H755ZITX.ld

# =====================
# Default Target
# =====================
all: $(BIN)
	@echo "Checking binary size..."
	@BIN_SIZE=$$(stat -c%s $(BIN)); \
	if [ $$BIN_SIZE -gt $(MAX_SIZE) ]; then \
		echo "ERROR: Binary size ($$BIN_SIZE bytes) exceeds maximum allowed size ($(MAX_SIZE) bytes)"; \
		echo "Binary is $$(($$BIN_SIZE - $(MAX_SIZE))) bytes too large"; \
		exit 1; \
	else \
		echo "Binary size check passed: $$BIN_SIZE bytes ($$(($(MAX_SIZE) - $$BIN_SIZE)) bytes remaining)"; \
	fi
# =====================
# vpath for source files
# =====================
vpath %.c Core/Src Common/Src Core/Overrides Drivers/STM32H7xx_HAL_Driver/Src Drivers/CMSIS/Device/ST/STM32H7xx/Source Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates FreeRTOS FreeRTOS/portable/GCC/ARM_CM7/r0p1 FreeRTOS/portable/MemMang ESP-Hosted/utils ESP-Hosted/port/esp/freertos/src ESP-Hosted/drivers/transport ESP-Hosted/drivers/transport/spi_hd ESP-Hosted/drivers/transport/sdio ESP-Hosted/drivers/transport/spi ESP-Hosted/drivers/transport/uart ESP-Hosted/drivers/power_save ESP-Hosted/drivers/virtual_serial_if ESP-Hosted/drivers/serial ESP-Hosted/drivers/bt ESP-Hosted/drivers/mempool ESP-Hosted/drivers/rpc/wrap ESP-Hosted/drivers/rpc/core ESP-Hosted/drivers/rpc/slaveif ESP-Hosted/api/src
vpath %.s Core/Startup

# =====================
# Build Rules
# =====================

# Single build rule for all C sources
$(OBJ_DIR)/%.o: %.c | $(BUILD_DIR) $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Single build rule for all ASM sources
$(OBJ_DIR)/%.o: %.s | $(BUILD_DIR) $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR): | $(BUILD_DIR)
	mkdir -p $(OBJ_DIR)

# Link objects to ELF
$(ELF): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

# Convert ELF to BIN
$(BIN): $(ELF) | $(BUILD_DIR)
	$(OBJCOPY) -O binary $< $@
	@echo "Extracting application version..."
	@VERSION_STR=$$(strings $@ | grep -m1 "Application Version" || true); \
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
	cp $@ $(ROOT_BUILD_DIR)/App_$$VERSION.bin; \
	echo "Copied versioned binary to $(ROOT_BUILD_DIR)/App_$$VERSION.bin"

# =====================
# Clean Rule
# =====================
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean