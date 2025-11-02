#include "logger.h"
#include "crc_helper.h"
#include "conf_helper.h"
#include "flash_editor.h"

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

  etx_config->app_crc = 0; // Application CRC set to 0
  etx_config->app_size = 0; // Application Size set to 0

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

  HAL_StatusTypeDef status;

  status = erase_flash(FLASH_BANK_1, FLASH_SECTOR_2, 1);
  if (status != HAL_OK) {
    LOG_ERROR("Failed to erase flash sector\r\n");
    return CFG_SAVE_ERR;
  }

  status = write_flash(CONFIG_FLASH_ADDR, (uint32_t *)etx_config, sizeof(ETX_CONFIG_), FLASH_BANK_1);
  if (status != HAL_OK) {
    LOG_ERROR("Failed to write Config...\r\n");
    return CFG_SAVE_ERR;
  }

  return CFG_SAVE_OK;
}
