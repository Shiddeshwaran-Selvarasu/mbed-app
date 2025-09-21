BUILD_DIR := build
APP_BIN   := App/$(BUILD_DIR)/app_crc.bin
BOOT_BIN  := Bootloader/$(BUILD_DIR)/bootloader.bin
FLASH_IMG := flash_image.bin

.PHONY: all clean $(APP_BIN) $(BOOT_BIN)


all: $(APP_BIN) $(BOOT_BIN)
	@echo "Cleaning existing flash image..."
	@rm -f $(FLASH_IMG)

	@echo "Merging bootloader and app into flash image..."
	@dd if=$(BOOT_BIN) of=$(FLASH_IMG) bs=1 seek=0 conv=notrunc status=none
	@dd if=$(APP_BIN) of=$(FLASH_IMG) bs=1 seek=262144 conv=notrunc status=none

	@echo "Flash image created successfully: $(FLASH_IMG)"

$(APP_BIN):
	@echo "Building application..."
	@$(MAKE) -C App all

$(BOOT_BIN):
	@echo "Building bootloader..."
	@$(MAKE) -C Bootloader all

clean:
	@echo "Cleaning all build artifacts..."
	@$(MAKE) -C App clean
	@$(MAKE) -C Bootloader clean
	@rm -rf $(BUILD_DIR)
	@rm -f $(FLASH_IMG)
