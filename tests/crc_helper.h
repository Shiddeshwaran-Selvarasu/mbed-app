/**
 ******************************************************************************
 * @file    crc_helper.h (test version)
 * @brief   CRC helper functions for testing
 ******************************************************************************
 */

#ifndef __CRC_HELPER_H
#define __CRC_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/* CRC status enumeration - must match the original */
typedef enum {
    CRC_OK = 0,
    CRC_ERROR_INVALID_ADDRESS = -1,
    CRC_ERROR_INVALID_VALUE = -2,
    CRC_ERROR_MISMATCH = -3
} crc_status_t;

/* Mock CRC handle type - defined in test file */
typedef struct {
    uint32_t dummy;
} CRC_HandleTypeDef;

/* Mock HAL function - defined in test file */
extern uint32_t HAL_CRC_Calculate(CRC_HandleTypeDef* hcrc, uint32_t* data, size_t length);

/**
 * @brief  Compute CRC32 over given data using STM32 hardware CRC peripheral
 * @param  hcrc: Pointer to CRC handle
 * @param  data: Pointer to data buffer
 * @param  length: Length of data in bytes
 * @retval Computed CRC32 value
 */
static inline uint32_t compute_crc32(CRC_HandleTypeDef* hcrc, uint32_t* data, size_t length) {
    if (hcrc == NULL || data == NULL || length == 0) {
        return 0; // Invalid parameters
    }

    uint32_t crc = HAL_CRC_Calculate(hcrc, data, length);

    return ~crc;
}

/**
 * @brief  Verify CRC32 over given data against expected value
 * @param  hcrc: Pointer to CRC handle
 * @param  data: Pointer to data buffer
 * @param  length: Length of data in bytes
 * @param  expected_crc: Expected CRC32 value to compare against
 * @retval CRC_OK if match, error code otherwise
 */
static inline crc_status_t verify_crc32(CRC_HandleTypeDef* hcrc, uint32_t* data, size_t length, uint32_t expected_crc) {
    if (hcrc == NULL || data == NULL || length == 0) {
        return CRC_ERROR_INVALID_ADDRESS; // Invalid parameters
    }

    uint32_t computed_crc = compute_crc32(hcrc, data, length);
    if (computed_crc != expected_crc) {
        return CRC_ERROR_MISMATCH; // CRC mismatch
    }

    return CRC_OK;      // CRC matches
}

#ifdef __cplusplus
}
#endif

#endif /* __CRC_HELPER_H */
