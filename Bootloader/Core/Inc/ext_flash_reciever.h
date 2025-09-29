#ifndef __EXT_FLASH_RECEIVER_H
#define __EXT_FLASH_RECEIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define ETX_FRAME_SOF  0xAAU    // Start of Frame
#define ETX_FRAME_EOF  0xBBU    // End of Frame
#define ETX_FRAME_ACK  0x00U    // ACK
#define ETX_FRAME_NACK 0x01U    // NACK

#define ETX_FRAME_DATA_MAX_SIZE ( 1024 )  //Maximum data Size
#define ETX_FRAME_DATA_OVERHEAD (    9 )  //data overhead
#define ETX_FRAME_PACKET_MAX_SIZE ( ETX_FRAME_DATA_MAX_SIZE + ETX_FRAME_DATA_OVERHEAD )

/*
 * Exception codes
 */
typedef enum
{
  ETX_DL_EX_OK       = 0,    // Success
  ETX_DL_EX_ERR      = 1,    // Failure
}ETX_DL_EX_;

/*
 * DL process state
 */
typedef enum
{
  ETX_DL_STATE_IDLE    = 0,
  ETX_DL_STATE_START   = 1,
  ETX_DL_STATE_HEADER  = 2,
  ETX_DL_STATE_DATA    = 3,
  ETX_DL_STATE_FAILED  = 4,
  ETX_DL_STATE_SUCCESS = 5,
}ETX_DL_STATE_;

ETX_DL_EX_ etx_app_download_and_flash(ETX_CONFIG_ *config);

#ifdef __cplusplus
}
#endif

#endif /* __EXT_FLASH_RECEIVER_H */
