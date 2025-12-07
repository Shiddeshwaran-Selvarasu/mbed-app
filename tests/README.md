# Unit Tests for mbed-app

This directory contains unit tests for the mbed-app project components.

## Test Framework

The tests use the [Unity Test Framework](https://github.com/ThrowTheSwitch/Unity), a lightweight unit testing framework for C.

## Test Coverage

### Common Components
- **test_logger.c**: Tests for the logging functionality (`Common/Core/Src/logger.c`)
  - Log level handling (ERROR, INFO, WARN, DEBUG)
  - Formatted string output
  - Invalid level handling
  - Log macros

### Bootloader Components
- **test_crc_helper.c**: Tests for CRC calculation and verification (`Bootloader/Core/Src/crc_helper.c`)
  - CRC32 computation with valid and invalid inputs
  - CRC32 verification with matching and mismatched CRCs
  - Null pointer and zero-length handling
  - Consistency checks

- **test_conf_helper.c**: Tests for configuration management (`Bootloader/Core/Src/conf_helper.c`)
  - Default configuration initialization
  - Configuration field validation
  - Null pointer handling
  - Structure integrity

### HostFlashApp Components
- **test_etx_crc.c**: Tests for ETX flash update CRC (`HostFlashApp/PcTool/Src/etx_flash_update.c`)
  - CRC calculation with various data patterns
  - Empty data handling
  - Consistency and uniqueness verification
  - Edge cases (all zeros, all ones)

## Building and Running Tests

### Prerequisites
- GCC compiler
- Make

### Build All Tests
```bash
cd tests
make all
```

### Run All Tests
```bash
cd tests
make test
```

### Clean Build Artifacts
```bash
cd tests
make clean
```

## Test Results

All tests should pass with output similar to:
```
===================================
Running Unit Tests
===================================
...
===================================
All tests passed!
===================================
```

## Adding New Tests

1. Create a new test file `test_<module>.c` in the `tests/` directory
2. Include the Unity framework: `#include "unity.h"`
3. Implement setUp() and tearDown() functions
4. Write test functions with prefix `test_`
5. Add test runner in main() using `RUN_TEST()`
6. Update the Makefile to include the new test

Example test structure:
```c
#include "unity.h"
#include "module_to_test.h"

void setUp(void) {
    // Setup code
}

void tearDown(void) {
    // Cleanup code
}

void test_feature(void) {
    TEST_ASSERT_EQUAL(expected, actual);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_feature);
    return UNITY_END();
}
```

## Notes

- Tests are compiled for the host system (x86/x64), not for the target ARM platform
- HAL functions are mocked where necessary to enable testing without hardware
- Tests focus on logic and edge cases rather than hardware-specific functionality
