#include "ext_flash_receiver.h"
#include "flash_editor.h"
#include "conf_helper.h"
#include "crc_helper.h"
#include "logger.h"

/* Retry configuration */
#define MAX_NACK_RETRIES              3U      /* Maximum NACK retry attempts */

/*******************************************************************************
 * Private Variables
 ******************************************************************************/

/* Ping-pong Rx buffers for DMA double-buffering */
static uint8_t rx_buffer_a[ETX_FRAME_PACKET_MAX_SIZE];
static uint8_t rx_buffer_b[ETX_FRAME_PACKET_MAX_SIZE];
static uint8_t *rx_active;               /* buffer DMA is currently filling */
static uint8_t *rx_ready;                /* buffer ready to process/flash */
static volatile bool rx_frame_ready = false;
static volatile uint16_t rx_received_size = 0;

/* Response Buffer */
static uint8_t rsp_buffer[ETX_RSPF_PACKET_SIZE];

/* Download Status */
static ETX_DL_STATE_ dl_state;

static uint32_t total_data_size;
static uint16_t total_data_fragments;
static uint16_t received_data_fragments;
static uint32_t expected_crc;
static bool is_data_transfer_complete;
static bool is_flash_write_started;
static uint8_t nack_sent_count;

/* Hardware CRC handle */
extern CRC_HandleTypeDef hcrc;

/*******************************************************************************
 * Private Function Prototypes
 ******************************************************************************/

/* Communication functions */
static ETX_DL_FRAME_EX_ etx_send_response(ETX_DL_RSP_ rsp);
static HAL_StatusTypeDef etx_tx_rsp(ETX_DL_RSPF_ *buffer);
static void restart_dma_receive(void);

/* Flash operation functions */
static HAL_StatusTypeDef flash_application_data(uint32_t address, uint32_t *data, uint32_t length);
static HAL_StatusTypeDef flash_erase_application(uint32_t data_size);

/*******************************************************************************
 * Public Functions
 ******************************************************************************/

/**
 * @brief  Download the application from UART and flash it.
 * @param  config: Pointer to the ETX_CONFIG_ structure to update status.
 * @retval ETX_DL_EX_: Status of the download operation.
 */
ETX_DL_EX_ etx_app_download_and_flash(ETX_CONFIG_ *config) {
  ETX_DL_EX_ ret_val = ETX_DL_EX_ERR;
  (void)ret_val;

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
  nack_sent_count = 0;
  rx_frame_ready = false;
  rx_received_size = 0;
  rx_active = rx_buffer_a;
  rx_ready  = rx_buffer_b;

  /* Kick off first DMA receive */
  restart_dma_receive();

  LOG_INFO("Waiting ETX APP download to start [State: IDLE]...\r\n");

  do {
    if (nack_sent_count >= MAX_NACK_RETRIES) {
      LOG_ERROR("Maximum NACK retries reached. Aborting download...\r\n");
      dl_state = ETX_DL_STATE_FAILED;

    } else if (!is_data_transfer_complete) {
      if (!rx_frame_ready) {
        HAL_IWDG_Refresh(&hiwdg);
        continue;
      }
      rx_frame_ready = false;

      /* Verify frame: SOF, bounds, CRC, EOF.
       * CRC covers bytes 0..(4+payload_len-1); CRC word follows at offset 4+payload_len. */
      ETX_DL_FRAME_ *f = (ETX_DL_FRAME_ *)rx_ready;
      uint16_t plen = f->payload_len;

      uint32_t expected_frame_crc;
      memcpy(&expected_frame_crc, rx_ready + 4U + plen, 4U);
      uint8_t received_eof = rx_ready[4U + plen + 4U];

      bool frame_valid = (rx_ready[0] == ETX_FRAME_SOF)
                       && (plen <= ETX_FRAME_DATA_MAX_SIZE)
                       && (rx_received_size >= (uint16_t)(4U + plen + 5U))
                       && (received_eof == ETX_FRAME_EOF)
                       && (compute_crc32(&hcrc, (uint32_t *)rx_ready, 4U + plen) == expected_frame_crc);

      if (!frame_valid) {
        LOG_ERROR("Frame integrity check failed\r\n");
        etx_send_response(ETX_DL_RSP_NACK);
        restart_dma_receive();
        continue;
      }
    }

    ETX_DL_FRAME_ *received_frame = (ETX_DL_FRAME_ *)rx_ready;

    switch (dl_state) {
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
        restart_dma_receive();
        break;

      case ETX_DL_STATE_HEADER:
        if (received_frame->packet_type == ETX_DL_FRAME_TYPE_HEADER &&
            received_frame->payload_len == 8) {
          total_data_size = ((uint32_t)received_frame->payload[0] << 24) |
                            ((uint32_t)received_frame->payload[1] << 16) |
                            ((uint32_t)received_frame->payload[2] <<  8) |
                            ((uint32_t)received_frame->payload[3]);
          expected_crc    = ((uint32_t)received_frame->payload[4] << 24) |
                            ((uint32_t)received_frame->payload[5] << 16) |
                            ((uint32_t)received_frame->payload[6] <<  8) |
                            ((uint32_t)received_frame->payload[7]);

          LOG_INFO("Received header: Total Size = %lu bytes, Expected CRC = 0x%08lX\r\n", total_data_size, expected_crc);

          total_data_fragments = (total_data_size / ETX_FRAME_DATA_MAX_SIZE)
                               + (total_data_size % ETX_FRAME_DATA_MAX_SIZE != 0);
          received_data_fragments = 0;

          etx_send_response(ETX_DL_RSP_ACK);
          LOG_INFO("Transitioning to DATA state...\r\n");
          dl_state = ETX_DL_STATE_DATA;
        } else {
          etx_send_response(ETX_DL_RSP_NACK);
        }
        restart_dma_receive();
        break;

      case ETX_DL_STATE_DATA:
        if (received_frame->packet_type == ETX_DL_FRAME_TYPE_DATA && received_frame->payload_len > 0) {

          if (!is_flash_write_started) {
            if (flash_erase_application(total_data_size) != HAL_OK) {
              LOG_ERROR("Failed to erase application area\r\n");
              dl_state = ETX_DL_STATE_FAILED;
              break;
            }
            is_flash_write_started = true;
            LOG_INFO("Application area erased. Starting to flash data...\r\n");
          }

          /* Snapshot flash parameters from rx_ready, then start DMA on rx_active
           * immediately so next frame is received while we write flash (overlap). */
          uint32_t flash_addr = APPLICATION_ADDRESS + (received_data_fragments * ETX_FRAME_DATA_MAX_SIZE);
          uint32_t flash_len  = received_frame->payload_len;
          uint8_t *flash_src  = rx_ready; /* save pointer before restart_dma may change rx_active */
          restart_dma_receive();           /* DMA fills rx_active; we write flash from flash_src */

          HAL_StatusTypeDef status = flash_application_data(
              flash_addr,
              (uint32_t *)(flash_src + 4U), /* payload starts at offset 4 in raw buffer */
              flash_len);

          if (status != HAL_OK) {
            LOG_ERROR("Failed to flash data at address 0x%08lX\r\n", flash_addr);
            dl_state = ETX_DL_STATE_FAILED;
            break;
          }

          received_data_fragments++;
          LOG_INFO("Received and flashed fragment %u/%u\r\n", received_data_fragments, total_data_fragments);

          if (received_data_fragments >= total_data_fragments) {
            dl_state = ETX_DL_STATE_DATA_COMPLETE;
            LOG_INFO("All data fragments received. Transitioning to Data Complete state...\r\n");
          }
          etx_send_response(ETX_DL_RSP_ACK);
        } else {
          etx_send_response(ETX_DL_RSP_NACK);
          restart_dma_receive();
        }
        break;

      case ETX_DL_STATE_DATA_COMPLETE:
        if (received_frame->packet_type == ETX_DL_FRAME_TYPE_CMD &&
            received_frame->payload_len == 1 &&
            received_frame->payload[0] == ETX_DL_CMD_END) {
          LOG_INFO("Received DL end command. Transitioning to SUCCESS state...\r\n");
          is_data_transfer_complete = true;
          dl_state = ETX_DL_STATE_SUCCESS;
          etx_send_response(ETX_DL_RSP_ACK);
        } else {
          etx_send_response(ETX_DL_RSP_NACK);
          restart_dma_receive();
        }
        break;

      case ETX_DL_STATE_FAILED:
        if (is_flash_write_started) {
          config->is_app_bootable = false;
          config->is_app_flashed = false;
          config->reboot_reason = ETX_APP_FAILED;
        }
        HAL_UART_DMAStop(&huart2);
        LOG_INFO("Download failed. Exiting...\r\n");
        return ETX_DL_EX_ERR;

      case ETX_DL_STATE_SUCCESS:
        HAL_UART_DMAStop(&huart2);
        config->is_app_bootable = false;
        config->is_app_flashed = true;
        config->reboot_reason = ETX_NORMAL_BOOT;
        config->app_crc = expected_crc;
        config->app_size = total_data_size;
        config_save(config);
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

/**
 * @brief UART RxEvent callback — fires on IDLE line (end of variable-length DMA frame).
 *        Swaps ping-pong buffers and signals the download loop.
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if (huart->Instance != USART2) return;

  /* Swap buffers: the freshly filled buffer becomes rx_ready */
  rx_ready = rx_active;
  rx_active = (rx_active == rx_buffer_a) ? rx_buffer_b : rx_buffer_a;
  rx_received_size = Size;
  rx_frame_ready = true;
}

/*******************************************************************************
 * Private Communication Functions
 ******************************************************************************/

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

static HAL_StatusTypeDef etx_tx_rsp(ETX_DL_RSPF_ *buffer)
{
  if (buffer == NULL) {
    return HAL_ERROR;
  }

  return HAL_UART_Transmit(&huart2, (uint8_t *)&buffer->sof, ETX_RSPF_PACKET_SIZE, HAL_DL_UART_RX_TIMEOUT);
}

/*******************************************************************************
 * Private Flash Operation Functions
 ******************************************************************************/

static HAL_StatusTypeDef flash_application_data(uint32_t address, uint32_t *data, uint32_t length)
{
  return write_flash(address, data, length, FLASH_BANK_2);
}

static HAL_StatusTypeDef flash_erase_application(uint32_t data_size)
{
  uint32_t num_sectors = (data_size + FLASH_SECTOR_SIZE - 1U) / FLASH_SECTOR_SIZE;
  if (num_sectors == 0) num_sectors = 1;
  return erase_flash(FLASH_BANK_2, FLASH_SECTOR_0, num_sectors);
}

static void restart_dma_receive(void)
{
  memset(rx_active, 0, ETX_FRAME_PACKET_MAX_SIZE);
  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_active, ETX_FRAME_PACKET_MAX_SIZE);
  __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
}
