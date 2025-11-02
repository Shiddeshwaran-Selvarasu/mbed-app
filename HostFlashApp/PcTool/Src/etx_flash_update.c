/**
  ******************************************************************************
  * @file    etx_flash_update.c
  * @author  Shiddeshwaran-S
  * @brief   This file provides set of firmware functions to manage etx flash update
  ******************************************************************************/

#include "etx_flash_update.h"

/* Host Flash Version Info start */
#define Major_VERSION  2
#define Minor_VERSION  0
#define Patch_VERSION  5

#define __STRINGIFY(x) #x
#define _STRINGIFY(x) __STRINGIFY(x)
#define HF_VERSION   "v"_STRINGIFY(Major_VERSION)"."_STRINGIFY(Minor_VERSION)"."_STRINGIFY(Patch_VERSION)
#define HF_VER_STRING "Host Flash Version " HF_VERSION " stable release"
/* Host Flash Version Info end */

uint8_t DATA_BUF[ETX_FRAME_PACKET_MAX_SIZE];
uint8_t RSP_BUF[ETX_RSPF_PACKET_SIZE];
uint8_t APP_BIN[ETX_DL_MAX_FW_SIZE];

uint32_t app_bin_size = 0;
uint32_t app_crc = 0;

uint32_t total_data_fragments = 0;
uint32_t sent_data_fragments = 0;

/* ***** Utility Functions - Start ***** */

uint32_t CalcCRC(uint8_t * pData, uint32_t DataLength)
{
    uint32_t crc = 0xFFFFFFFF;
    for(unsigned int i = 0; i < DataLength; i++)
    {
        crc ^= pData[i];
        for(int j = 0; j < 8; j++)
        {
            if(crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc = crc >> 1;
        }
    }
    return ~crc;
}

void delay(uint32_t us)
{
#if defined(__linux__)
    usleep(us);
#else
    //Sleep(ms);
    __int64 time1 = 0, time2 = 0, freq = 0;

    QueryPerformanceCounter((LARGE_INTEGER *) &time1);
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    do {
        QueryPerformanceCounter((LARGE_INTEGER *) &time2);
    } while((time2-time1) < us);
#endif
}

bool load_application_bin(char *file_path) {
  FILE *fp = fopen(file_path, "rb");
  if (fp == NULL) {
    printf("Failed to open application binary file\r\n");
    return false;
  }

  size_t bytesRead = fread(APP_BIN, 1, ETX_DL_MAX_FW_SIZE, fp);
  fclose(fp);

  if (bytesRead == 0) {
    printf("Failed to read application binary file\r\n");
    return false;
  }

  app_bin_size = bytesRead;
  app_crc = CalcCRC(APP_BIN, app_bin_size);

  printf("Loaded application binary, size: %u bytes, CRC: 0x%08X\r\n", app_bin_size, app_crc);
  
  return true;
}

/* ***** Utility Functions - End ***** */

/* ***** Test Functions - Start ***** */

void test_frame_transmission(int comport_number)
{
  printf("=== TEST SEQUENCE START ===\r\n");
  
  // First, test the CRC calculation
  uint8_t test_data[] = {0xAA, 0x01, 0x00, 0x01, 0x01}; // SOF + Type + Len + Payload
  uint32_t calculated_crc = CalcCRC(test_data, 5);
  printf("CRC for test data: 0x%08X\r\n", calculated_crc);
  
  // Send raw test bytes to see what bootloader receives
  uint8_t test_frame[] = {
    0xAA,  // SOF
    0x01,  // packet_type (CMD)
    0x00,  // payload_len high byte
    0x01,  // payload_len low byte  
    0x01,  // payload[0] (START command)
    (calculated_crc >> 0) & 0xFF,   // CRC byte 0 (LSB)
    (calculated_crc >> 8) & 0xFF,   // CRC byte 1
    (calculated_crc >> 16) & 0xFF,  // CRC byte 2
    (calculated_crc >> 24) & 0xFF,  // CRC byte 3 (MSB)
    0xBB   // EOF
  };
  
  printf("Sending test frame (10 bytes):\r\n");
  for(int i = 0; i < 10; i++) {
    printf("  Byte[%d]: 0x%02X\r\n", i, test_frame[i]);
    
    if( RS232_SendByte(comport_number, test_frame[i]) ) {
      printf("Failed to send test byte %d\r\n", i);
      return;
    }
    delay(10000); // 10ms delay between bytes
  }
  
  printf("=== TEST SEQUENCE COMPLETE ===\r\n");
  printf("Check bootloader output now...\r\n");
}

/* ***** Test Functions - End ***** */

/* ***** IO Functions - Start ***** */

ETX_DL_FRAME_EX_ etx_tx_data(int comport_number, ETX_DL_FRAME_ *buffer)
{
  if (buffer == NULL) {
    return ETX_DL_FRAME_EX_ERR;
  }

  // calculate crc for (SOF + packet_type + payload_len + payload)
  buffer->crc = CalcCRC( (uint8_t *)&buffer->sof, (buffer->payload_len + 4)); 

  printf("Sending packet type: %d, length: %d\r\n", buffer->packet_type, buffer->payload_len);

  // send (SOF + packet_type + payload_len + payload)
  for(int i = 0; i < (buffer->payload_len + 4); i++) {
    if( RS232_SendByte(comport_number, ((uint8_t *)&buffer->sof)[i]) ) {
      //some data missed.
      printf("Send Err: %d\n", buffer->packet_type);
      return ETX_DL_FRAME_EX_ERR;
    }
    delay(1500);
  }

  // send (CRC + EOF)
  for(int i = 0; i < 5; i++) {
    if( RS232_SendByte(comport_number, ((uint8_t *)&buffer->crc)[i]) ) {
      //some data missed.
      printf("Send Err: %d\n", buffer->packet_type);
      return ETX_DL_FRAME_EX_ERR;
    }
    delay(1500);
  }

  return ETX_DL_FRAME_EX_OK;
}

ETX_DL_FRAME_EX_ etx_tx_response(int comport_number, ETX_DL_RSPF_ *response)
{
  if (response == NULL) {
    return ETX_DL_FRAME_EX_ERR;
  }

  // send (SOF + packet_type + payload + EOF)
  for(uint32_t i = 0; i < sizeof(ETX_DL_RSPF_); i++) {
    delay(1);
    if( RS232_SendByte(comport_number, ((uint8_t *)response)[i]) ) {
      //some data missed.
      printf("Send Err: %d\n", response->packet_type);
      return ETX_DL_FRAME_EX_ERR;
    }
  }

  return ETX_DL_FRAME_EX_OK;
}

ETX_DL_FRAME_EX_ etx_rx_data(int comport_number, ETX_DL_FRAME_ *buffer)
{
  if (buffer == NULL) {
    return ETX_DL_FRAME_EX_ERR;
  }

  uint16_t index = 0;
  int status;

  // Receive SOF
  while( (status = RS232_PollComport(comport_number, &((uint8_t *)buffer)[index], 1)) == 0 );
  if (status < 0) {
    return ETX_DL_FRAME_EX_ERR;
  } else if ( ((uint8_t *)buffer)[index] != ETX_FRAME_SOF) {
    return ETX_DL_FRAME_EX_ERR; // Invalid SOF
  }

  // check for payload type and length
  index++;
  while( (status = RS232_PollComport(comport_number, &((uint8_t *)buffer)[index], 3)) == 0 );
  if (status < 0) {
    return ETX_DL_FRAME_EX_ERR;
  }

  // check for valid payload length
  uint16_t payload_len = (((uint8_t *)buffer)[2] << 8) | ((uint8_t *)buffer)[3];
  if (payload_len > ETX_FRAME_DATA_MAX_SIZE) {
    return ETX_DL_FRAME_EX_ERR; // Invalid length
  }

  // receive payload
  index += 3;
  while( (status = RS232_PollComport(comport_number, &((uint8_t *)buffer)[index], payload_len)) == 0 );
  if (status < 0) {
    return ETX_DL_FRAME_EX_ERR;
  }

  // receive CRC + EOF
  index += ETX_FRAME_DATA_MAX_SIZE;
  while( (status = RS232_PollComport(comport_number, &((uint8_t *)buffer)[index], 5)) == 0 );
  if (status < 0) {
    return ETX_DL_FRAME_EX_ERR;
  }

  // verify EOF
  if ( ((uint8_t *)buffer)[index + 4] != ETX_FRAME_EOF) {
    return ETX_DL_FRAME_EX_ERR; // Invalid EOF
  }

  return ETX_DL_FRAME_EX_OK;
}

ETX_DL_FRAME_EX_ etx_rx_response(int comport_number, ETX_DL_RSPF_ *buffer)
{
  if (buffer == NULL) {
    return ETX_DL_FRAME_EX_ERR;
  }

  int status;

  printf("Starting to poll for response...\r\n");

  // Receive (SOF + packet_type + payload + EOF)
  while( (status = RS232_PollComport(comport_number, (uint8_t *)buffer, ETX_RSPF_PACKET_SIZE)) == 0 );
  if (status < 0) {
    return ETX_DL_FRAME_EX_ERR;
  }

  // verify SOF and EOF
  if (buffer->sof != ETX_FRAME_SOF 
    || buffer->eof != ETX_FRAME_EOF 
    || buffer->packet_type != ETX_DL_FRAME_TYPE_RESPONSE
    || (buffer->payload != ETX_DL_RSP_ACK && buffer->payload != ETX_DL_RSP_NACK)
  ) {
    return ETX_DL_FRAME_EX_ERR; // Invalid Response frame
  }

  return ETX_DL_FRAME_EX_OK;
}

/* ***** IO Functions - End ***** */

/* ***** COM Functions - Start ***** */

ETX_DL_FRAME_EX_ etx_send_data(int comport_number, ETX_DL_FRAME_ *data_frame, bool skip_ack_check)
{
  if (data_frame == NULL) {
    return ETX_DL_FRAME_EX_ERR;
  }

  ETX_DL_FRAME_EX_ status;

  status = etx_tx_data(comport_number, data_frame);
  if (status != ETX_DL_FRAME_EX_OK) {
    printf("Failed to send data frame\r\n");
    return status;
  }

  // Wait for ACK/NACK (retransmit if NACK max 3 times)
  ETX_DL_RSPF_ response;
  int nack_received_count = 0;
  const int max_nack_retries = 3;

  if (skip_ack_check) {
    printf("Ack check not required...\r\n");
    return ETX_DL_FRAME_EX_OK;
  }

  printf("Waiting for ACK/NACK...\r\n");

  do {
    status = etx_rx_response(comport_number, &response);
    if (status != ETX_DL_FRAME_EX_OK) {
      printf("Failed to receive response from STM32\r\n");
      return status;
    }

    if (response.payload == ETX_DL_RSP_ACK) {
      return ETX_DL_FRAME_EX_OK; // Acknowledged
    } else if (response.payload == ETX_DL_RSP_NACK) {
      nack_received_count++;
      printf("Host NACK received, retrying... (%d/%d)\r\n", nack_received_count, max_nack_retries);
      status = etx_tx_data(comport_number, data_frame); // Resend data frame
      if (status != ETX_DL_FRAME_EX_OK) {
        printf("Failed to resend data frame\r\n");
        return status;
      }
    }
  } while (nack_received_count < max_nack_retries);

  return ETX_DL_FRAME_EX_OK;
}

ETX_DL_FRAME_EX_ etx_send_response(int comport_number, ETX_DL_RSP_ rsp)
{
  //clear the buffer
  memset( RSP_BUF, 0, ETX_RSPF_PACKET_SIZE );

  ETX_DL_RSPF_ *response_frame = (ETX_DL_RSPF_ *)RSP_BUF;

  response_frame->sof = ETX_FRAME_SOF;
  response_frame->eof = ETX_FRAME_EOF;
  response_frame->packet_type = ETX_DL_FRAME_TYPE_RESPONSE;
  response_frame->payload = rsp;

  return etx_tx_response(comport_number, response_frame);
}

ETX_DL_FRAME_EX_ etx_receive_data(int comport_number, ETX_DL_FRAME_ *data_frame)
{
  if (data_frame == NULL) {
    return ETX_DL_FRAME_EX_ERR;
  }

  memset( data_frame, 0, ETX_FRAME_PACKET_MAX_SIZE );

  ETX_DL_FRAME_EX_ status;

  status = etx_rx_data(comport_number, data_frame);
  if (status != ETX_DL_FRAME_EX_OK) {
    return status;
  }

  // verify CRC
  uint32_t calculated_crc = CalcCRC( (uint8_t *)&data_frame->sof, (data_frame->payload_len + 4));
  if (calculated_crc != data_frame->crc) {
    printf("CRC mismatch: Calculated=0x%08X, Received=0x%08X\r\n", calculated_crc, data_frame->crc);
    return ETX_DL_FRAME_EX_ERR; // CRC mismatch
  }

  return ETX_DL_FRAME_EX_OK;
}

ETX_DL_FRAME_EX_ etx_receive_response(int comport_number, ETX_DL_RSPF_ *rsp_frame)
{
  if (rsp_frame == NULL) {
    return ETX_DL_FRAME_EX_ERR;
  }

  ETX_DL_FRAME_EX_ status;

  status = etx_rx_response(comport_number, rsp_frame);
  if (status != ETX_DL_FRAME_EX_OK) {
    printf("Failed to receive response from STM32\r\n");
    return status;
  }

  return ETX_DL_FRAME_EX_OK;
}

/* ***** COM Functions - End ***** */

/* ***** Payload Functions - Start ***** */

ETX_DL_EX_ etx_send_start_cmd(int comport_number)
{
  //clear the buffer
  memset( DATA_BUF, 0, ETX_FRAME_PACKET_MAX_SIZE );

  ETX_DL_FRAME_ *cmd_frame = (ETX_DL_FRAME_ *)DATA_BUF;

  cmd_frame->sof = ETX_FRAME_SOF;
  cmd_frame->eof = ETX_FRAME_EOF;
  cmd_frame->packet_type = ETX_DL_FRAME_TYPE_CMD;
  cmd_frame->payload[0] = ETX_DL_CMD_START;
  cmd_frame->payload_len = 1;

  if (etx_send_data(comport_number, cmd_frame, false) != ETX_DL_FRAME_EX_OK) {
    printf("Failed to send START command\r\n");
    return ETX_DL_EX_ERR;
  }

  return ETX_DL_EX_OK;
}

ETX_DL_EX_ etx_send_fw_info(int comport_number)
{
  //clear the buffer
  memset( DATA_BUF, 0, ETX_FRAME_PACKET_MAX_SIZE );

  ETX_DL_FRAME_ *cmd_frame = (ETX_DL_FRAME_ *)DATA_BUF;

  cmd_frame->sof = ETX_FRAME_SOF;
  cmd_frame->eof = ETX_FRAME_EOF;
  cmd_frame->packet_type = ETX_DL_FRAME_TYPE_HEADER;
  cmd_frame->payload[0] = (app_bin_size >> 24) & 0xFF;
  cmd_frame->payload[1] = (app_bin_size >> 16) & 0xFF;
  cmd_frame->payload[2] = (app_bin_size >> 8) & 0xFF;
  cmd_frame->payload[3] = (app_bin_size >> 0) & 0xFF;
  cmd_frame->payload[4] = (app_crc >> 24) & 0xFF;
  cmd_frame->payload[5] = (app_crc >> 16) & 0xFF;
  cmd_frame->payload[6] = (app_crc >> 8) & 0xFF;
  cmd_frame->payload[7] = (app_crc >> 0) & 0xFF;
  cmd_frame->payload_len = 8;

  if (etx_send_data(comport_number, cmd_frame, false) != ETX_DL_FRAME_EX_OK) {
    printf("Failed to send FW_INFO command\r\n");
    return ETX_DL_EX_ERR;
  }

  return ETX_DL_EX_OK;
}

ETX_DL_EX_ etx_send_fw_data(int comport_number)
{
  total_data_fragments = (app_bin_size / ETX_FRAME_DATA_MAX_SIZE) + (app_bin_size % ETX_FRAME_DATA_MAX_SIZE != 0);
  sent_data_fragments = 0;

  uint32_t bytes_sent = 0;
  uint16_t chunk_size = 0;
  ETX_DL_FRAME_EX_ status;

  while (sent_data_fragments < total_data_fragments) {
    //clear the buffer
    memset( DATA_BUF, 0, ETX_FRAME_PACKET_MAX_SIZE );

    ETX_DL_FRAME_ *data_frame = (ETX_DL_FRAME_ *)DATA_BUF;

    chunk_size = (app_bin_size - bytes_sent) > ETX_FRAME_DATA_MAX_SIZE ? ETX_FRAME_DATA_MAX_SIZE : (app_bin_size - bytes_sent);

    data_frame->sof = ETX_FRAME_SOF;
    data_frame->eof = ETX_FRAME_EOF;
    data_frame->packet_type = ETX_DL_FRAME_TYPE_DATA;
    memcpy(data_frame->payload, &APP_BIN[bytes_sent], chunk_size);
    data_frame->payload_len = chunk_size;

    status = etx_send_data(comport_number, data_frame, false);
    if (status != ETX_DL_FRAME_EX_OK) {
      printf("Failed to send firmware data chunk\r\n");
      return ETX_DL_EX_ERR;
    }

    bytes_sent += chunk_size;
    sent_data_fragments++;
    printf("Sent %u/%u bytes\r\n", bytes_sent, app_bin_size);
  }

  return ETX_DL_EX_OK;
}

ETX_DL_EX_ etx_send_end_cmd(int comport_number)
{
  //clear the buffer
  memset( DATA_BUF, 0, ETX_FRAME_PACKET_MAX_SIZE );

  ETX_DL_FRAME_ *cmd_frame = (ETX_DL_FRAME_ *)DATA_BUF;

  cmd_frame->sof = ETX_FRAME_SOF;
  cmd_frame->eof = ETX_FRAME_EOF;
  cmd_frame->packet_type = ETX_DL_FRAME_TYPE_CMD;
  cmd_frame->payload[0] = ETX_DL_CMD_END;
  cmd_frame->payload_len = 1;

  if (etx_send_data(comport_number, cmd_frame, true) != ETX_DL_FRAME_EX_OK) {
    printf("Failed to send END command\r\n");
    return ETX_DL_EX_ERR;
  }

  return ETX_DL_EX_OK;
}

/* ***** Payload Functions - End ***** */

/* ***** Main Function ***** */
int main(int argc, char *argv[])
{
  char *comport = NULL;
  static int comport_number = -1;
  char bin_name[1024];

  int bdrate  = 921600;       /* Increased baud rate for faster transfer */
  char mode[] = {'8','N','1',0}; /* *-bits, No parity, 1 stop bit */
  
  int exit_code = 0;
  ETX_DL_STATE_ dl_state;

  do {
    if( argc <= 2 ) {
      #ifdef _WIN32
      printf("Please feed the COM PORT number and the Application Image....!!!\n");
      printf("Example: .\\etx_ota_app.exe COM3 ..\\..\\Application\\Debug\\Blinky.bin");
      #else
      printf("Please feed the TTY PORT number and the Application Image....!!!\n");
      printf("Example: ./etx_ota_app /dev/ttyUSB0 ../../Application/Debug/Blinky.bin");
      #endif
      exit_code = -1;
      break;
    }

    printf("%s\r\n", HF_VER_STRING);

    //get the COM port
    comport = argv[1];
    strcpy(bin_name, argv[2]);

    printf("Opening %s...\n", comport);

    comport_number = RS232_GetPortnr(comport);
    if( comport_number < 0 )
    {
      printf("Can not find comport\n");
      exit_code = -1;
      break;
    }

    if( RS232_OpenComport(comport_number, bdrate, mode, 0) )
    {
      printf("Can not open comport\n");
      exit_code = -1;
      break;
    }

    dl_state = ETX_DL_STATE_IDLE;

    printf("Sending DL Start cmd...\r\n");

    do {
      switch (dl_state)
      {
      case ETX_DL_STATE_IDLE:
        if(etx_send_start_cmd(comport_number) != ETX_DL_EX_OK){
          printf("STM32 did not respond to start cmd...\r\n");
        } else {
          printf("STM32 Acknowledged the start cmd. Sending Header now...\r\n");
          load_application_bin(bin_name);
          dl_state = ETX_DL_STATE_HEADER;
        }
        break;
      
      case ETX_DL_STATE_HEADER:
        if(etx_send_fw_info(comport_number) != ETX_DL_EX_OK){
          printf("STM32 did not respond to header message...\r\n");
          dl_state = ETX_DL_STATE_FAILED;
        } else {
          printf("STM32 Acknowledged the header message. Sending Data now...\r\n");
          dl_state = ETX_DL_STATE_DATA;
        }
        break;

      case ETX_DL_STATE_DATA:
        if(etx_send_fw_data(comport_number) != ETX_DL_EX_OK){
          printf("STM32 did not respond to data message...\r\n");
          dl_state = ETX_DL_STATE_FAILED;
        } else {
          printf("STM32 Acknowledged the data message. Sending END command now...\r\n");
          dl_state = ETX_DL_STATE_DATA_COMPLETE;
        }
        break;
      
      case ETX_DL_STATE_DATA_COMPLETE:
        if(etx_send_end_cmd(comport_number) != ETX_DL_EX_OK){
          printf("STM32 did not respond to end message...\r\n");
          dl_state = ETX_DL_STATE_FAILED;
        } else {
          printf("STM32 Acknowledged the end message...\r\n");
          dl_state = ETX_DL_STATE_SUCCESS;
        }
        break;

      case ETX_DL_STATE_FAILED:
        printf("ETX DL failed...");
        return -1;

      case ETX_DL_STATE_SUCCESS:
        printf("ETX DL Success...\r\n");
        return 0;

      default:
        break;
      }

    } while(true);

  } while (0);

  return exit_code;
}
