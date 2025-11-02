#include "flash_editor.h"
#include "logger.h"

HAL_StatusTypeDef erase_flash(uint32_t bank, uint32_t sector, uint32_t num_sectors)
{
  HAL_StatusTypeDef status;

  // Erase the FLASH memory before writing
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t SectorError;

  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Sector = sector;
  EraseInitStruct.NbSectors = num_sectors;
  EraseInitStruct.Banks = bank;

  LOG_INFO("Erasing flash sectors: Bank %lu, Sector %lu, Number of Sectors %lu\r\n", bank, sector, num_sectors);

  // Unlock the FLASH memory
  status = HAL_FLASH_Unlock();
  if (status != HAL_OK) {
    LOG_ERROR("Failed to unlock flash memory\r\n");
    return status;
  }

  //Check if the FLASH_FLAG_BSY.
  FLASH_WaitForLastOperation(HAL_FLASH_OP_TIMEOUT, bank);

  // clear all flags before you write it to flash
  if (bank == FLASH_BANK_1) {
    __HAL_FLASH_CLEAR_FLAG_BANK1(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR);
  } else if (bank == FLASH_BANK_2) {
    __HAL_FLASH_CLEAR_FLAG_BANK2(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR);
  } else {
    LOG_ERROR("Invalid flash bank specified\r\n");
    HAL_FLASH_Lock();
    return HAL_ERROR;
  }

  status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
  if (status != HAL_OK)
  {
    LOG_ERROR("Failed to erase flash sector\r\n");
    HAL_FLASH_Lock();
    return status;
  }

  // Lock the FLASH memory
  status = HAL_FLASH_Lock();
  if (status != HAL_OK)
  {
    LOG_ERROR("Failed to lock flash memory\r\n");
    return status;
  }

  LOG_INFO("Flash sector erased\r\n");
  
  return HAL_OK;
}

HAL_StatusTypeDef write_flash(uint32_t address, uint32_t *data, uint32_t length, uint32_t bank)
{
  HAL_StatusTypeDef status;
  
  // Unlock the FLASH memory
  status = HAL_FLASH_Unlock();
  if (status != HAL_OK)
  {
    LOG_ERROR("Failed to unlock flash memory\r\n");
    return status;
  }

  LOG_INFO("Writing data[size: %lu] to flash memory...\r\n", length);

  // Program the configuration data to FLASH memory
  for (uint32_t i = 0; i < length; i += 32)
  {
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address + i, (uint32_t)(data) + i);
    if (status != HAL_OK) {
      LOG_ERROR("Failed to write to flash memory\r\n");
      HAL_FLASH_Lock();
      return status;
    }
  }

  //Check if the FLASH_FLAG_BSY.
  FLASH_WaitForLastOperation(HAL_FLASH_OP_TIMEOUT, bank);

  LOG_INFO("Data written to flash memory\r\n");

  // Lock the FLASH memory
  status = HAL_FLASH_Lock();
  if (status != HAL_OK)
  {
    LOG_ERROR("Failed to lock flash memory\r\n");
    return status;
  }

  return HAL_OK;
}