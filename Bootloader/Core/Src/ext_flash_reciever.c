#include "ext_flash_reciever.h"
#include "flash_editor.h"
#include "conf_helper.h"
#include "crc_helper.h"
#include "logger.h"

/* Rx Buffer */
static uint8_t rx_buffer[ETX_FRAME_PACKET_MAX_SIZE];

/* Response Buffer */
static uint8_t rsp_buffer[ETX_RSPF_PACKET_SIZE];

/* Download Status */
static ETX_DL_STATE_ dl_state;

static uint16_t total_data_size;
static uint16_t total_data_fragments;
static uint16_t received_data_fragments;
static uint32_t expected_crc;
static bool is_data_transfer_complete;
static bool is_flash_write_started;
static uint8_t nack_sent_count = 0;
static const uint8_t max_nack_retries = 3;

/* Hardware CRC handle */
extern CRC_HandleTypeDef hcrc;

// Internal Function prototypes
static ETX_DL_FRAME_EX_ etx_receive_data(uint8_t *buffer);
static ETX_DL_FRAME_EX_ etx_receive_response(uint8_t *rsp);
static ETX_DL_FRAME_EX_ etx_send_data(ETX_DL_FRAME_ *buffer);
static ETX_DL_FRAME_EX_ etx_send_response(ETX_DL_RSP_ rsp);
static HAL_StatusTypeDef etx_tx_data(ETX_DL_FRAME_ *buffer);
static HAL_StatusTypeDef etx_rx_data(uint8_t *buffer);
static HAL_StatusTypeDef etx_tx_rsp(ETX_DL_RSPF_ *buffer);
static HAL_StatusTypeDef etx_rx_rsp(ETX_DL_RSPF_ *buffer);

static HAL_StatusTypeDef flash_application_data(uint32_t address, uint32_t *data, uint32_t length);
static HAL_StatusTypeDef flash_erase_application();

/**
 * @brief  Download the application from UART and flash it.
 * @param  config: Pointer to the ETX_CONFIG_ structure to update status.
 * @retval ETX_DL_EX_: Status of the download operation.
 */
ETX_DL_EX_ etx_app_download_and_flash(ETX_CONFIG_ *config) {
  ETX_DL_EX_ ret_val = ETX_DL_EX_ERR;
  ETX_DL_FRAME_EX_ received_status = ETX_DL_FRAME_EX_NO_DATA;

  if (config == NULL) {
    LOG_ERROR("Invalid configuration pointer\r\n");
    return ret_val;
  }

  dl_state = ETX_DL_STATE_IDLE;
  total_data_size = 0;
  total_data_fragments = 0;
  received_data_fragments = 0;
  is_data_transfer_complete = false;
  is_flash_write_started = false;
  expected_crc = 0;

  LOG_INFO("Waiting ETX APP download to start [State: IDLE]...\r\n");
  do {
    if (nack_sent_count >= max_nack_retries) {
      LOG_ERROR("Maximum NACK retries reached. Aborting download...\r\n");
      dl_state = ETX_DL_STATE_FAILED;
    } else if (!is_data_transfer_complete) {
      received_status = etx_receive_data(rx_buffer);

      if (received_status == ETX_DL_FRAME_EX_NO_DATA) {
        continue; // No data received, continue waiting
      } else if (received_status == ETX_DL_FRAME_EX_ERR) {
        LOG_ERROR("Error receiving data\r\n");
        dl_state = ETX_DL_STATE_FAILED;
        etx_send_response(ETX_DL_RSP_NACK);
      }
    }

    ETX_DL_FRAME_ *received_frame = (ETX_DL_FRAME_ *)rx_buffer;

    switch (dl_state)
    {
    case ETX_DL_STATE_IDLE:
      if (received_frame->packet_type == ETX_DL_FRAME_TYPE_CMD &&
          received_frame->payload_len == 1 &&
          received_frame->payload[0] == ETX_DL_CMD_START) {
        LOG_INFO("Received DL start command. Transitioning to HEADER state...\r\n");
        dl_state = ETX_DL_STATE_HEADER;
        etx_send_response(ETX_DL_RSP_ACK);
      } else {
        etx_send_response(ETX_DL_RSP_NACK);
      }
      break;
    
    case ETX_DL_STATE_HEADER:
      if (received_frame->packet_type == ETX_DL_FRAME_TYPE_HEADER &&
          received_frame->payload_len == 8) {
        total_data_size = (received_frame->payload[0] << 8)  |
                          (received_frame->payload[1]);
        expected_crc = (received_frame->payload[4] << 24) |
                       (received_frame->payload[5] << 16) |
                       (received_frame->payload[6] << 8)  |
                       (received_frame->payload[7]);
        
        LOG_INFO("Received header: Total Size = %lu bytes, Expected CRC = 0x%08lX\r\n", total_data_size, expected_crc);

        total_data_fragments = (total_data_size / ETX_FRAME_DATA_MAX_SIZE) + (total_data_size % ETX_FRAME_DATA_MAX_SIZE != 0);
        received_data_fragments = 0;

        etx_send_response(ETX_DL_RSP_ACK);
        LOG_INFO("Transitioning to DATA state...\r\n");
        dl_state = ETX_DL_STATE_DATA;
      } else {
        etx_send_response(ETX_DL_RSP_NACK);
      }
      break;

    case ETX_DL_STATE_DATA:
      if (received_frame->packet_type == ETX_DL_FRAME_TYPE_DATA && received_frame->payload_len > 0) {

        if (!is_flash_write_started) {
          // Erase the application area before starting to flash
          if (flash_erase_application() != HAL_OK) {
            LOG_ERROR("Failed to erase application area\r\n");
            dl_state = ETX_DL_STATE_FAILED;
            break;
          }
          is_flash_write_started = true;
          LOG_INFO("Application area erased. Starting to flash data...\r\n");
        }
        
        HAL_StatusTypeDef status;

        // Flash the received data
        status = flash_application_data((APPLICATION_ADDRESS + (received_data_fragments * ETX_FRAME_DATA_MAX_SIZE)),
                                   (uint32_t *)received_frame->payload,
                                   (uint32_t)received_frame->payload_len);
        if (status != HAL_OK) {
          LOG_ERROR("Failed to flash data at address 0x%08lX\r\n", APPLICATION_ADDRESS + (received_data_fragments * ETX_FRAME_DATA_MAX_SIZE));
          dl_state = ETX_DL_STATE_FAILED;
          break;
        }

        received_data_fragments++;
        LOG_INFO("Received and flashed fragment %u/%u\r\n", received_data_fragments, total_data_fragments);

        if (received_data_fragments >= total_data_fragments) {
          is_data_transfer_complete = true;
          dl_state = ETX_DL_STATE_SUCCESS;
          LOG_INFO("All data fragments received. Transitioning to Success state...\r\n");
        }
        etx_send_response(ETX_DL_RSP_ACK);
      } else if (received_frame->packet_type == ETX_DL_FRAME_TYPE_CMD &&
                  received_frame->payload_len == 1 &&
                  received_frame->payload[0] == ETX_DL_CMD_END) {
        LOG_INFO("Received DL end command. Transitioning to CRC state...\r\n");
        dl_state = ETX_DL_STATE_SUCCESS;
        etx_send_response(ETX_DL_RSP_ACK);
      } else {
        etx_send_response(ETX_DL_RSP_NACK);
      }
      break;

    case ETX_DL_STATE_FAILED:
      if (is_flash_write_started) {
        config->is_app_bootable = false;
        config->is_app_flashed = false;
        config->reboot_reason = ETX_APP_FAILED;
      }

      LOG_INFO("Download failed. Exiting...\r\n");
      return ETX_DL_EX_ERR;

    case ETX_DL_STATE_SUCCESS:
      config->is_app_bootable = false;
      config->is_app_flashed = true;
      config->reboot_reason = ETX_NORMAL_BOOT;
      LOG_INFO("Download successful. Exiting...\r\n");
      return ETX_DL_EX_OK;

    default:
      dl_state = ETX_DL_STATE_FAILED;
      LOG_ERROR("Unknown state encountered. Aborting...\r\n");
      break;
    }

  } while (true);

  return ret_val;
}

static ETX_DL_FRAME_EX_ etx_receive_data(uint8_t *buffer)
{
  if (buffer == NULL) {
    return ETX_DL_FRAME_EX_ERR;
  }

  //clear the buffer
  memset( rx_buffer, 0, ETX_FRAME_PACKET_MAX_SIZE );

  HAL_StatusTypeDef status;

  status = etx_rx_data(buffer);
  if (status != HAL_OK) {
    if (status == HAL_TIMEOUT) {
      LOG_DEBUG("No data received...\r\n");
      return ETX_DL_FRAME_EX_NO_DATA; // No data received
    }
    return ETX_DL_FRAME_EX_ERR;
  }

  ETX_DL_FRAME_ *received_frame = (ETX_DL_FRAME_ *)buffer;
  uint32_t computed_crc = compute_crc32(&hcrc, (uint32_t *)&received_frame->sof, (received_frame->payload_len + 4));

  if (computed_crc != received_frame->crc) {
    LOG_ERROR("CRC mismatch: Computed = 0x%08lX, Received = 0x%08lX\r\n", computed_crc, received_frame->crc);
    return ETX_DL_FRAME_EX_ERR;
  }

  return ETX_DL_FRAME_EX_OK;
}

static ETX_DL_FRAME_EX_ etx_receive_response(uint8_t *buffer)
{
  if (buffer == NULL) {
    return ETX_DL_FRAME_EX_ERR;
  }

  //clear the buffer
  memset( buffer, 0, ETX_RSPF_PACKET_SIZE );
  ETX_DL_RSPF_ *rsp_frame = (ETX_DL_RSPF_ *)buffer;

  uint8_t retry_count = 0;
  const uint8_t max_retries = 3;

  do {
    HAL_StatusTypeDef status;

    status = etx_rx_rsp(rsp_frame);
    if (status != HAL_OK) {
      return status;
    }

    if (rsp_frame->payload == ETX_DL_RSP_ACK) {
      return ETX_DL_FRAME_EX_OK; // Acknowledged
    } else if (rsp_frame->payload == ETX_DL_RSP_NACK) {
      LOG_WARN("Host NACK received, retrying...\r\n");
      continue; // NACK received, retry sending
    }
  } while (retry_count++ < max_retries);

  LOG_ERROR("Max retries reached without ACK\r\n");
  return ETX_DL_FRAME_EX_ERR; // Max retries reached without ACK
}

static ETX_DL_FRAME_EX_ etx_send_data(ETX_DL_FRAME_ *buffer)
{
  if (buffer == NULL) {
    return ETX_DL_FRAME_EX_ERR;
  }

  HAL_StatusTypeDef status;

  status = etx_tx_data(buffer);
  if (status != HAL_OK) {
    return ETX_DL_FRAME_EX_ERR;
  }

  return etx_receive_response(rsp_buffer);
}

static ETX_DL_FRAME_EX_ etx_send_response(ETX_DL_RSP_ rsp)
{
  //clear the buffer
  memset( rsp_buffer, 0, ETX_RSPF_PACKET_SIZE );

  ETX_DL_RSPF_ *response_frame = (ETX_DL_RSPF_ *)rsp_buffer;

  if (rsp == ETX_DL_RSP_NACK) {
    nack_sent_count++;
  } else {
    nack_sent_count = 0; // Reset on ACK
  }

  response_frame->sof = ETX_FRAME_SOF;
  response_frame->eof = ETX_FRAME_EOF;
  response_frame->packet_type = ETX_DL_FRAME_TYPE_RESPONSE;
  response_frame->payload = rsp;

  return etx_tx_rsp(response_frame);
}

static HAL_StatusTypeDef etx_tx_data(ETX_DL_FRAME_ *buffer)
{
  if (buffer == NULL) {
    return HAL_ERROR;
  }

  HAL_StatusTypeDef status;

  // calculate crc for (SOF + packet_type + payload_len + payload)
  buffer->crc = compute_crc32(&hcrc, (uint32_t *)&buffer, (buffer->payload_len + 4)); 

  // send (SOF + packet_type + payload_len + payload)
  status = HAL_UART_Transmit(&huart2, (uint8_t *)&buffer->sof, (buffer->payload_len + 4), HAL_DL_UART_RX_TIMEOUT);
  if (status != HAL_OK) return status;

  // send (CRC + EOF)
  status = HAL_UART_Transmit(&huart2, (uint8_t *)&buffer->crc, 5, HAL_DL_UART_RX_TIMEOUT);
  if (status != HAL_OK) return status;

  return HAL_OK;
}

static HAL_StatusTypeDef etx_tx_rsp(ETX_DL_RSPF_ *buffer)
{
  if (buffer == NULL) {
    return HAL_ERROR;
  }

  return HAL_UART_Transmit(&huart2, (uint8_t *)&buffer->sof, ETX_RSPF_PACKET_SIZE, HAL_DL_UART_RX_TIMEOUT);
}

static HAL_StatusTypeDef etx_rx_data(uint8_t *buffer)
{
  if (buffer == NULL) {
    return HAL_ERROR;
  }

  uint16_t index = 0;
  HAL_StatusTypeDef status;

  // Receive SOF
  status = HAL_UART_Receive(&huart2, &buffer[index], 1, HAL_DL_UART_RX_TIMEOUT);
  if (status != HAL_OK) {
    return status;
  } else if (buffer[index] != ETX_FRAME_SOF) {
    return HAL_ERROR; // Invalid SOF
  }

  // check for payload type and length
  index++;
  status = HAL_UART_Receive(&huart2, &buffer[index], 3, HAL_DL_UART_RX_TIMEOUT);
  if (status != HAL_OK) {
    return status;
  }

  uint16_t payload_len = (buffer[index + 1] << 8) | buffer[index + 2];
  if (payload_len > ETX_FRAME_DATA_MAX_SIZE) {
    return HAL_ERROR; // Payload length exceeds maximum
  }

  // Receive payload
  index += 3;
  status = HAL_UART_Receive(&huart2, &buffer[index], payload_len, HAL_DL_UART_RX_TIMEOUT);
  if (status != HAL_OK) {
    return status;
  }

  // Receive CRC and EOF
  status = HAL_UART_Receive(&huart2, &buffer[index], 5, HAL_DL_UART_RX_TIMEOUT);
  if (status != HAL_OK) {
    return status;
  } else if (buffer[index + 4] != ETX_FRAME_EOF) {
    return HAL_ERROR; // Invalid EOF
  }

  return HAL_OK;
}

static HAL_StatusTypeDef etx_rx_rsp(ETX_DL_RSPF_ *buffer)
{
  if (buffer == NULL) {
    return HAL_ERROR;
  }

  HAL_StatusTypeDef status;

  status = HAL_UART_Receive(&huart2, (uint8_t *)&buffer->sof, ETX_RSPF_PACKET_SIZE, HAL_DL_UART_RX_TIMEOUT);
  if (status != HAL_OK) {
    return status;
  } else {
    if (buffer->sof != ETX_FRAME_SOF 
      || buffer->eof != ETX_FRAME_EOF
      || buffer->packet_type != ETX_DL_FRAME_TYPE_RESPONSE
      || buffer->payload < ETX_DL_RSP_ACK
      || buffer->payload > ETX_DL_RSP_NACK) {
      return HAL_ERROR;
    }
  }

  return HAL_OK;
}

static HAL_StatusTypeDef flash_application_data(uint32_t address, uint32_t *data, uint32_t length)
{
  return write_flash(address, data, length, FLASH_BANK_2);
}

static HAL_StatusTypeDef flash_erase_application()
{
  return erase_flash(FLASH_BANK_2, FLASH_SECTOR_0, FLASH_SECTOR_TOTAL);
}
