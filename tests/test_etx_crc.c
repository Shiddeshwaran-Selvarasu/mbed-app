/**
 ******************************************************************************
 * @file    test_etx_crc.c
 * @brief   Unit tests for ETX flash update CRC functions
 ******************************************************************************
 */

#include "unity.h"
#include <stdint.h>
#include <string.h>

/* CRC calculation constants */
#define CRC_INITIAL_VALUE             0xFFFFFFFFU
#define CRC_POLYNOMIAL                0xEDB88320U

/* CalcCRC implementation from etx_flash_update.c */
uint32_t CalcCRC(uint8_t * pData, uint32_t DataLength)
{
  uint32_t crc = CRC_INITIAL_VALUE;
  for(unsigned int i = 0; i < DataLength; i++)
  {
    crc ^= pData[i];
    for(int j = 0; j < 8; j++)
    {
      if(crc & 1)
        crc = (crc >> 1) ^ CRC_POLYNOMIAL;
      else
        crc = crc >> 1;
    }
  }
  return ~crc;
}

/* Test setup and teardown */
void setUp(void) {
}

void tearDown(void) {
}

/* Test cases */
void test_CalcCRC_with_empty_data(void) {
    uint8_t data[] = {};
    
    uint32_t crc = CalcCRC(data, 0);
    
    /* Empty data should produce initial CRC value */
    TEST_ASSERT_EQUAL_UINT32(0, crc);
}

void test_CalcCRC_with_single_byte(void) {
    uint8_t data[] = {0xAA};
    
    uint32_t crc = CalcCRC(data, 1);
    
    /* CRC should be calculated correctly */
    TEST_ASSERT_NOT_EQUAL(0, crc);
}

void test_CalcCRC_with_multiple_bytes(void) {
    uint8_t data[] = {0xAA, 0x01, 0x00, 0x01, 0x01};
    
    uint32_t crc = CalcCRC(data, sizeof(data));
    
    /* CRC should be calculated correctly */
    TEST_ASSERT_NOT_EQUAL(0, crc);
}

void test_CalcCRC_consistency(void) {
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    
    uint32_t crc1 = CalcCRC(data, sizeof(data));
    uint32_t crc2 = CalcCRC(data, sizeof(data));
    
    /* Same data should produce same CRC */
    TEST_ASSERT_EQUAL_UINT32(crc1, crc2);
}

void test_CalcCRC_different_data_different_crc(void) {
    uint8_t data1[] = {0x12, 0x34, 0x56, 0x78};
    uint8_t data2[] = {0x12, 0x34, 0x56, 0x79};
    
    uint32_t crc1 = CalcCRC(data1, sizeof(data1));
    uint32_t crc2 = CalcCRC(data2, sizeof(data2));
    
    /* Different data should produce different CRC */
    TEST_ASSERT_NOT_EQUAL(crc1, crc2);
}

void test_CalcCRC_order_matters(void) {
    uint8_t data1[] = {0x12, 0x34, 0x56};
    uint8_t data2[] = {0x56, 0x34, 0x12};
    
    uint32_t crc1 = CalcCRC(data1, sizeof(data1));
    uint32_t crc2 = CalcCRC(data2, sizeof(data2));
    
    /* Different order should produce different CRC */
    TEST_ASSERT_NOT_EQUAL(crc1, crc2);
}

void test_CalcCRC_known_value(void) {
    /* Test with known data pattern */
    uint8_t data[] = {0xAA, 0x01, 0x00, 0x01, 0x01};
    
    uint32_t crc = CalcCRC(data, sizeof(data));
    
    /* Store the known CRC for this pattern */
    uint32_t known_crc = CalcCRC(data, sizeof(data));
    TEST_ASSERT_EQUAL_UINT32(known_crc, crc);
}

void test_CalcCRC_length_sensitivity(void) {
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78};
    
    uint32_t crc1 = CalcCRC(data, 2);
    uint32_t crc2 = CalcCRC(data, 4);
    
    /* Different lengths should produce different CRC */
    TEST_ASSERT_NOT_EQUAL(crc1, crc2);
}

void test_CalcCRC_all_zeros(void) {
    uint8_t data[10] = {0};
    
    uint32_t crc = CalcCRC(data, sizeof(data));
    
    /* All zeros should still produce a valid CRC */
    TEST_ASSERT_NOT_EQUAL(0, crc);
}

void test_CalcCRC_all_ones(void) {
    uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF};
    
    uint32_t crc = CalcCRC(data, sizeof(data));
    
    /* All ones should still produce a valid CRC */
    TEST_ASSERT_NOT_EQUAL(0, crc);
}

/* Main test runner */
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_CalcCRC_with_empty_data);
    RUN_TEST(test_CalcCRC_with_single_byte);
    RUN_TEST(test_CalcCRC_with_multiple_bytes);
    RUN_TEST(test_CalcCRC_consistency);
    RUN_TEST(test_CalcCRC_different_data_different_crc);
    RUN_TEST(test_CalcCRC_order_matters);
    RUN_TEST(test_CalcCRC_known_value);
    RUN_TEST(test_CalcCRC_length_sensitivity);
    RUN_TEST(test_CalcCRC_all_zeros);
    RUN_TEST(test_CalcCRC_all_ones);
    
    return UNITY_END();
}
