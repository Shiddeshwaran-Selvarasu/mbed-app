#ifndef __CRC_HELPER_H
#define __CRC_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef enum {
    CRC_OK = 0,
    CRC_ERROR_INVALID_ADDRESS = -1,
    CRC_ERROR_INVALID_VALUE = -2,
    CRC_ERROR_MISMATCH = -3
} crc_status_t;

uint32_t compute_crc32(CRC_HandleTypeDef* hcrc, uint8_t* data, size_t length);
crc_status_t verify_crc32(CRC_HandleTypeDef* hcrc, uint8_t* data, size_t length, uint32_t expected_crc);


#ifdef __cplusplus
}
#endif

#endif /* __CRC_HELPER_H */