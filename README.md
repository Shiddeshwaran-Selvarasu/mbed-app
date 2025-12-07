# mbed-app
Embbeded Application for STM32H7xx boards

## Project Structure

- **App/**: Main application code for STM32H7xx
- **Bootloader/**: Bootloader code for firmware updates
- **HostFlashApp/**: PC tool for flashing firmware via UART
- **Common/**: Shared code and utilities
- **tests/**: Unit tests for the project components

## Building

Build all components:
```bash
make all
```

Build individual components:
```bash
make App
make Bootloader
make HostFlashApp/PcTool
```

Clean all build artifacts:
```bash
make cleanall
```

## Testing

The project includes comprehensive unit tests for key components.

### Running Tests

```bash
cd tests
make test
```

For more information about tests, see [tests/README.md](tests/README.md).

### Test Coverage

- **Logger**: Common logging functionality
- **CRC Helper**: CRC calculation and verification for bootloader
- **Configuration Helper**: Configuration management for bootloader
- **ETX CRC**: CRC calculation for host flash update tool

All tests use the Unity test framework and are designed to run on the host system without requiring target hardware.
