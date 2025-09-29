#include "logger.h"
#include "crc_helper.h"
#include "conf_helper.h"

/**
 * @brief  Loads configuration from flash
 * @param  ETX_CONFIG_ *etx_config: Pointer to the configuration structure
 * @retval None
 */
void config_get(ETX_CONFIG_ *etx_config)
{
  // copy flash content to etx_config
  memcpy(etx_config, (void *)CONFIG_FLASH_ADDR, sizeof(ETX_CONFIG_));
}

/**
 * @brief  loads default configuration values in flash
 * @param  ETX_CONFIG_ *etx_config: Pointer to the configuration structure
 * @retval None
 */
void config_load_defaults(ETX_CONFIG_ *etx_config)
{
  LOG_INFO("Populating default configuration...\r\n");

  // Set reboot reason
  etx_config->reboot_reason = ETX_FIRST_TIME_BOOT;

  // Application not bootable and not flashed
  etx_config->is_app_bootable = false;
  etx_config->is_app_flashed = false;

  // Reserved space
  for (int i = 0; i < 10; i++) {
    etx_config->reserved[i] = 0;
  }

  // Set valid marker
  etx_config->config_valid_marker = VALID_CONF_MARKER;
  
  LOG_INFO("Default configuration populated...\r\n");
}

/**
 * @brief Saves given config to FLASH memory
 * @param (ETX_CONFIG_ *)etx_config: Pointer to the configuration structure
 * @retval CFG_SAVE_STATUS_: Status of the save operation
 */
CFG_SAVE_STATUS_ config_save(ETX_CONFIG_ *etx_config)
{
  if (etx_config == NULL) {
    LOG_ERROR("Invalid configuration pointer\r\n");
    return CFG_SAVE_ERR;
  }

  // get the CRC of the configuration data
  uint32_t crc = compute_crc32(&hcrc, (uint32_t *)etx_config, sizeof(ETX_CONFIG_) - 4);
  etx_config->config_crc = crc;

  // Erase the FLASH memory before writing
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t SectorError;

  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Sector = FLASH_SECTOR_2; // Sector containing CONFIG_FLASH_ADDR
  EraseInitStruct.NbSectors = 1;
  EraseInitStruct.Banks = FLASH_BANK_1;

  // Unlock the FLASH memory
  if (HAL_FLASH_Unlock() != HAL_OK)
  {
    LOG_ERROR("Failed to unlock flash memory\r\n");
    return CFG_SAVE_ERR;
  }

  //Check if the FLASH_FLAG_BSY.
  FLASH_WaitForLastOperation(HAL_FLASH_OP_TIMEOUT, FLASH_BANK_1);
  
  // clear all flags before you write it to flash
  __HAL_FLASH_CLEAR_FLAG_BANK1(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR);

  if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
  {
    LOG_ERROR("Failed to erase flash sector\r\n");
    HAL_FLASH_Lock();
    return CFG_SAVE_ERR;
  }

  //Check if the FLASH_FLAG_BSY.
  FLASH_WaitForLastOperation(HAL_FLASH_OP_TIMEOUT, FLASH_BANK_1);

  // Program the configuration data to FLASH memory
  for (uint32_t i = 0; i < sizeof(ETX_CONFIG_); i += 32)
  {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, CONFIG_FLASH_ADDR + i, (uint32_t)(etx_config) + i) != HAL_OK)
    {
      LOG_ERROR("Failed to write to flash memory\r\n");
      HAL_FLASH_Lock();
      return CFG_SAVE_ERR;
    }
  }

  //Check if the FLASH_FLAG_BSY.
  FLASH_WaitForLastOperation(HAL_FLASH_OP_TIMEOUT, FLASH_BANK_1);

  LOG_INFO("Configuration saved to flash memory\r\n");

  // Lock the FLASH memory
  if (HAL_FLASH_Lock() != HAL_OK)
  {
    LOG_ERROR("Failed to lock flash memory\r\n");
    return CFG_SAVE_ERR;
  }

  return CFG_SAVE_OK;
}
