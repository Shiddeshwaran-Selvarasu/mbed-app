BUILD_DIR := build

BUILD_TARGETS := App Bootloader HostFlashApp/PcTool

.PHONY: all clean $(BUILD_TARGETS)

all: $(BUILD_TARGETS)

cleanall:
	@echo "Cleaning all build artifacts..."
	for dir in $(BUILD_TARGETS); do \
		$(MAKE) -C $$dir clean; \
	done

$(BUILD_TARGETS):
	@echo "Building $@..."
	@$(MAKE) -C $@ all

clean:
	@echo "Cleaning release artifacts..."
	@rm -rf $(BUILD_DIR)
