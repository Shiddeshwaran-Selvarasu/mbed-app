#ifndef __ETX_FLASH_UPDATE_H
#define __ETX_FLASH_UPDATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#if defined(__linux__)
#include <unistd.h>
#else
#include <Windows.h>
#endif

#include "rs232.h"

#define ETX_FRAME_SOF  0xAAU    // Start of Frame
#define ETX_FRAME_EOF  0xBBU    // End of Frame
#define ETX_FRAME_ACK  0x00U    // ACK
#define ETX_FRAME_NACK 0x01U    // NACK

#define ETX_FRAME_DATA_MAX_SIZE ( 1024 )  //Maximum data Size
#define ETX_FRAME_DATA_OVERHEAD (    9 )  //data overhead
#define ETX_FRAME_PACKET_MAX_SIZE sizeof(ETX_DL_FRAME_) // Maximum packet size
#define ETX_RSPF_PACKET_SIZE sizeof(ETX_DL_RSPF_) // Maximum packet size
#define ETX_DL_MAX_FW_SIZE ( 1024 * 1024 ) // 1MB

/*
 * ETX DL exit codes
 */
typedef enum
{
  ETX_DL_EX_OK       = 0,    // Success
  ETX_DL_EX_ERR      = 1,    // Failure
  ETX_DL_EX_ABORT    = 2,    // Aborted
}ETX_DL_EX_;

/*
 * ETX DL FRAME Recieve/Send status codes
 */
typedef enum
{
  ETX_DL_FRAME_EX_OK       = 0,    // Success
  ETX_DL_FRAME_EX_ERR      = 1,    // Failure
  ETX_DL_FRAME_EX_NO_DATA  = 2,    // No Data
}ETX_DL_FRAME_EX_;

/*
 * DL process state
 */
typedef enum
{
  ETX_DL_STATE_IDLE    = 0,
  ETX_DL_STATE_HEADER  = 1,
  ETX_DL_STATE_DATA    = 2,
  ETX_DL_STATE_FAILED  = 3,
  ETX_DL_STATE_SUCCESS = 4,
}ETX_DL_STATE_;

/**
 * Packet types
 */
typedef enum
{
  ETX_DL_FRAME_TYPE_CMD       = 0x01,
  ETX_DL_FRAME_TYPE_HEADER    = 0x02,
  ETX_DL_FRAME_TYPE_DATA      = 0x03,
  ETX_DL_FRAME_TYPE_RESPONSE  = 0x04,
}ETX_DL_FRAME_TYPE_;

/**
 * DL Commands
 */
typedef enum
{
  ETX_DL_CMD_START      = 0x01,
  ETX_DL_CMD_ABORT      = 0x02,
  ETX_DL_CMD_END        = 0x03,
}ETX_DL_CMD_;

/**
 * DL responses
 */
typedef enum
{
  ETX_DL_RSP_ACK       = 0x01,
  ETX_DL_RSP_NACK      = 0x02,
}ETX_DL_RSP_;

/*
 * Frame structure
 * _________________________________________
 * |     | Packet |     |  PAY |     |     |
 * | SOF | Type   | Len | LOAD | CRC | EOF |
 * |_____|________|_____|______|_____|_____|
 *   1B      1B     2B   nBytes   4B    1B
 */
typedef struct
{
  uint8_t   sof;                                      // Start of Frame (ETX_FRAME_SOF)
  uint8_t   packet_type;                              // Packet Type (ETX_DL_FRAME_TYPE_)
  uint16_t  payload_len;                              // Length of the payload
  uint8_t   payload[ETX_FRAME_DATA_MAX_SIZE];         // Payload data
  uint32_t  crc;                                      // CRC32 of the payload
  uint8_t   eof;                                      // End of Frame (ETX_FRAME_EOF)
}__attribute__((packed)) ETX_DL_FRAME_;

/*
 * Rsp Frame structure
 * ______________________________
 * |     | Packet |       |     |
 * | SOF | Type   | N/ACK | EOF |
 * |_____|________|_______|_____|
 *   1B      1B      1B      1B
 */
typedef struct
{
  uint8_t   sof;                      // Start of Frame (ETX_FRAME_SOF)
  uint8_t   packet_type;              // Packet Type (ETX_DL_FRAME_TYPE_)
  uint8_t   payload;                  // ACK/NACK (ETX_DL_RSP_) 
  uint8_t   eof;                      // End of Frame (ETX_FRAME_EOF)
}__attribute__((packed)) ETX_DL_RSPF_;

#ifdef __cplusplus
}
#endif

#endif /* __ETX_FLASH_UPDATE_H */