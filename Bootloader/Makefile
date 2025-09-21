# =====================
# Directories
# =====================
BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj

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
		   -IDrivers/CMSIS/Device/ST/STM32H7xx/Include

# =====================
# Compiler Flags
# =====================
CFLAGS   = -mcpu=cortex-m7 -mthumb -O2 -g -Wall -ffunction-sections -fdata-sections $(INCLUDES) -DUSE_HAL_DRIVER -DSTM32H755xx -DCORE_CM7

# =====================
# Output Files
# =====================
ELF      = $(BUILD_DIR)/bootloader.elf
BIN      = $(BUILD_DIR)/bootloader.bin

# =====================
# Linker Flags
# =====================
LDFLAGS = -TSTM32H755ZITX.ld

# =====================
# Default Target
# =====================
all: $(BIN)
	$(SIZE) $(ELF)

# =====================
# vpath for source files
# =====================
vpath %.c Core/Src Common/Src Drivers/STM32H7xx_HAL_Driver/Src Drivers/CMSIS/Device/ST/STM32H7xx/Source Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates
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
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@
	@truncate -s 131072 $@

# =====================
# Clean Rule
# =====================
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean