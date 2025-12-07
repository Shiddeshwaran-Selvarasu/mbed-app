/**
 ******************************************************************************
 * @file    test_crc_helper.c
 * @brief   Unit tests for crc_helper.c
 ******************************************************************************
 */

#include "unity.h"
#include <stdint.h>
#include <stddef.h>

/* Include the header first to get type definitions */
#include "crc_helper.h"

/* Mock HAL_CRC_Calculate function implementation */
uint32_t HAL_CRC_Calculate(CRC_HandleTypeDef* hcrc, uint32_t* data, size_t length) {
    /* Simple mock CRC implementation for testing */
    if (hcrc == NULL || data == NULL || length == 0) {
        return 0;
    }
    
    /* Simple checksum for testing purposes */
    uint32_t crc = 0;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
    }
    return crc;
}

/* Test setup and teardown */
void setUp(void) {
}

void tearDown(void) {
}

/* Test cases */
void test_compute_crc32_with_valid_data(void) {
    CRC_HandleTypeDef hcrc = {0};
    uint32_t data[] = {0x12345678, 0x9ABCDEF0, 0x11111111};
    
    uint32_t crc = compute_crc32(&hcrc, data, sizeof(data));
    
    /* CRC should be non-zero for valid data */
    TEST_ASSERT_NOT_EQUAL(0, crc);
}

void test_compute_crc32_with_null_handle(void) {
    uint32_t data[] = {0x12345678};
    
    uint32_t crc = compute_crc32(NULL, data, sizeof(data));
    
    /* Should return 0 for NULL handle */
    TEST_ASSERT_EQUAL_UINT32(0, crc);
}

void test_compute_crc32_with_null_data(void) {
    CRC_HandleTypeDef hcrc = {0};
    
    uint32_t crc = compute_crc32(&hcrc, NULL, 4);
    
    /* Should return 0 for NULL data */
    TEST_ASSERT_EQUAL_UINT32(0, crc);
}

void test_compute_crc32_with_zero_length(void) {
    CRC_HandleTypeDef hcrc = {0};
    uint32_t data[] = {0x12345678};
    
    uint32_t crc = compute_crc32(&hcrc, data, 0);
    
    /* Should return 0 for zero length */
    TEST_ASSERT_EQUAL_UINT32(0, crc);
}

void test_compute_crc32_consistency(void) {
    CRC_HandleTypeDef hcrc = {0};
    uint32_t data[] = {0x12345678, 0x9ABCDEF0};
    
    uint32_t crc1 = compute_crc32(&hcrc, data, sizeof(data));
    uint32_t crc2 = compute_crc32(&hcrc, data, sizeof(data));
    
    /* Same data should produce same CRC */
    TEST_ASSERT_EQUAL_UINT32(crc1, crc2);
}

void test_verify_crc32_with_matching_crc(void) {
    CRC_HandleTypeDef hcrc = {0};
    uint32_t data[] = {0x12345678, 0x9ABCDEF0};
    
    uint32_t expected_crc = compute_crc32(&hcrc, data, sizeof(data));
    crc_status_t status = verify_crc32(&hcrc, data, sizeof(data), expected_crc);
    
    /* Should return CRC_OK for matching CRC */
    TEST_ASSERT_EQUAL(CRC_OK, status);
}

void test_verify_crc32_with_mismatched_crc(void) {
    CRC_HandleTypeDef hcrc = {0};
    uint32_t data[] = {0x12345678, 0x9ABCDEF0};
    
    uint32_t wrong_crc = 0xDEADBEEF;
    crc_status_t status = verify_crc32(&hcrc, data, sizeof(data), wrong_crc);
    
    /* Should return CRC_ERROR_MISMATCH for wrong CRC */
    TEST_ASSERT_EQUAL(CRC_ERROR_MISMATCH, status);
}

void test_verify_crc32_with_null_handle(void) {
    uint32_t data[] = {0x12345678};
    
    crc_status_t status = verify_crc32(NULL, data, sizeof(data), 0x12345678);
    
    /* Should return error for NULL handle */
    TEST_ASSERT_EQUAL(CRC_ERROR_INVALID_ADDRESS, status);
}

void test_verify_crc32_with_null_data(void) {
    CRC_HandleTypeDef hcrc = {0};
    
    crc_status_t status = verify_crc32(&hcrc, NULL, 4, 0x12345678);
    
    /* Should return error for NULL data */
    TEST_ASSERT_EQUAL(CRC_ERROR_INVALID_ADDRESS, status);
}

void test_verify_crc32_with_zero_length(void) {
    CRC_HandleTypeDef hcrc = {0};
    uint32_t data[] = {0x12345678};
    
    crc_status_t status = verify_crc32(&hcrc, data, 0, 0x12345678);
    
    /* Should return error for zero length */
    TEST_ASSERT_EQUAL(CRC_ERROR_INVALID_ADDRESS, status);
}

/* Main test runner */
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_compute_crc32_with_valid_data);
    RUN_TEST(test_compute_crc32_with_null_handle);
    RUN_TEST(test_compute_crc32_with_null_data);
    RUN_TEST(test_compute_crc32_with_zero_length);
    RUN_TEST(test_compute_crc32_consistency);
    RUN_TEST(test_verify_crc32_with_matching_crc);
    RUN_TEST(test_verify_crc32_with_mismatched_crc);
    RUN_TEST(test_verify_crc32_with_null_handle);
    RUN_TEST(test_verify_crc32_with_null_data);
    RUN_TEST(test_verify_crc32_with_zero_length);
    
    return UNITY_END();
}
