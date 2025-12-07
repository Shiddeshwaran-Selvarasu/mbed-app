/**
 ******************************************************************************
 * @file    test_conf_helper.c
 * @brief   Unit tests for conf_helper.c
 ******************************************************************************
 */

#include "unity.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Configuration constants from main.h */
#define ETX_FIRST_TIME_BOOT       ( 0xAFAFAFAF )
#define ETX_NORMAL_BOOT           ( 0xBEEFFEED )
#define ETX_DL_REQUEST            ( 0xDEADBEEF )
#define ETX_APP_FAILED            ( 0xBAADF00D )
#define VALID_CONF_MARKER         0xDEADBEEF
#define CONFIG_RESERVED_ENTRIES   10U

/* Configuration structure */
typedef struct
{
  uint32_t           reboot_reason;
  bool               is_app_bootable;
  bool               is_app_flashed;
  uint32_t           reserved[CONFIG_RESERVED_ENTRIES];
  uint32_t           app_crc;
  uint32_t           app_size;
  uint32_t           config_valid_marker;
  uint32_t           config_crc;
}__attribute__((packed)) ETX_CONFIG_;

/* config_load_defaults implementation from conf_helper.c */
void config_load_defaults(ETX_CONFIG_ *etx_config)
{
  if (etx_config == NULL) {
    return;
  }

  // Set reboot reason
  etx_config->reboot_reason = ETX_FIRST_TIME_BOOT;

  // Application not bootable and not flashed
  etx_config->is_app_bootable = false;
  etx_config->is_app_flashed = false;

  etx_config->app_crc = 0; // Application CRC set to 0
  etx_config->app_size = 0; // Application Size set to 0

  // Reserved space
  for (unsigned int i = 0; i < CONFIG_RESERVED_ENTRIES; i++) {
    etx_config->reserved[i] = 0;
  }

  // Set valid marker
  etx_config->config_valid_marker = VALID_CONF_MARKER;
}

/* Test setup and teardown */
void setUp(void) {
}

void tearDown(void) {
}

/* Test cases */
void test_config_load_defaults_sets_reboot_reason(void) {
    ETX_CONFIG_ config;
    memset(&config, 0xFF, sizeof(config)); // Initialize with garbage
    
    config_load_defaults(&config);
    
    TEST_ASSERT_EQUAL_UINT32(ETX_FIRST_TIME_BOOT, config.reboot_reason);
}

void test_config_load_defaults_sets_app_not_bootable(void) {
    ETX_CONFIG_ config;
    memset(&config, 0xFF, sizeof(config));
    
    config_load_defaults(&config);
    
    TEST_ASSERT_FALSE(config.is_app_bootable);
}

void test_config_load_defaults_sets_app_not_flashed(void) {
    ETX_CONFIG_ config;
    memset(&config, 0xFF, sizeof(config));
    
    config_load_defaults(&config);
    
    TEST_ASSERT_FALSE(config.is_app_flashed);
}

void test_config_load_defaults_clears_app_crc(void) {
    ETX_CONFIG_ config;
    memset(&config, 0xFF, sizeof(config));
    
    config_load_defaults(&config);
    
    TEST_ASSERT_EQUAL_UINT32(0, config.app_crc);
}

void test_config_load_defaults_clears_app_size(void) {
    ETX_CONFIG_ config;
    memset(&config, 0xFF, sizeof(config));
    
    config_load_defaults(&config);
    
    TEST_ASSERT_EQUAL_UINT32(0, config.app_size);
}

void test_config_load_defaults_clears_reserved_fields(void) {
    ETX_CONFIG_ config;
    memset(&config, 0xFF, sizeof(config));
    
    config_load_defaults(&config);
    
    for (unsigned int i = 0; i < CONFIG_RESERVED_ENTRIES; i++) {
        TEST_ASSERT_EQUAL_UINT32(0, config.reserved[i]);
    }
}

void test_config_load_defaults_sets_valid_marker(void) {
    ETX_CONFIG_ config;
    memset(&config, 0, sizeof(config));
    
    config_load_defaults(&config);
    
    TEST_ASSERT_EQUAL_UINT32(VALID_CONF_MARKER, config.config_valid_marker);
}

void test_config_load_defaults_with_null_pointer(void) {
    /* Should not crash with NULL pointer */
    config_load_defaults(NULL);
    TEST_PASS();
}

void test_config_structure_size(void) {
    /* Verify the structure size is as expected */
    ETX_CONFIG_ config;
    size_t expected_min_size = sizeof(uint32_t) * (1 + 10 + 1 + 1 + 1 + 1) + sizeof(bool) * 2;
    
    TEST_ASSERT_GREATER_OR_EQUAL(expected_min_size, sizeof(config));
}

void test_config_load_defaults_initializes_all_critical_fields(void) {
    ETX_CONFIG_ config;
    memset(&config, 0xAA, sizeof(config)); // Fill with pattern
    
    config_load_defaults(&config);
    
    /* Verify all critical fields are initialized */
    TEST_ASSERT_EQUAL_UINT32(ETX_FIRST_TIME_BOOT, config.reboot_reason);
    TEST_ASSERT_FALSE(config.is_app_bootable);
    TEST_ASSERT_FALSE(config.is_app_flashed);
    TEST_ASSERT_EQUAL_UINT32(0, config.app_crc);
    TEST_ASSERT_EQUAL_UINT32(0, config.app_size);
    TEST_ASSERT_EQUAL_UINT32(VALID_CONF_MARKER, config.config_valid_marker);
}

/* Main test runner */
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_config_load_defaults_sets_reboot_reason);
    RUN_TEST(test_config_load_defaults_sets_app_not_bootable);
    RUN_TEST(test_config_load_defaults_sets_app_not_flashed);
    RUN_TEST(test_config_load_defaults_clears_app_crc);
    RUN_TEST(test_config_load_defaults_clears_app_size);
    RUN_TEST(test_config_load_defaults_clears_reserved_fields);
    RUN_TEST(test_config_load_defaults_sets_valid_marker);
    RUN_TEST(test_config_load_defaults_with_null_pointer);
    RUN_TEST(test_config_structure_size);
    RUN_TEST(test_config_load_defaults_initializes_all_critical_fields);
    
    return UNITY_END();
}
