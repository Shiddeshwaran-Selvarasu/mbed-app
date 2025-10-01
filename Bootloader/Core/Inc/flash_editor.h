#ifndef __FLASH_EDITOR_H
#define __FLASH_EDITOR_H
#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define HAL_FLASH_OP_TIMEOUT 1000U /* 1 s */

HAL_StatusTypeDef erase_flash(uint32_t bank, uint32_t sector, uint32_t num_sectors);
HAL_StatusTypeDef write_flash(uint32_t address, uint32_t *data, uint32_t length, uint32_t bank);

#ifdef __cplusplus
}
#endif
#endif /* __FLASH_EDITOR_H */