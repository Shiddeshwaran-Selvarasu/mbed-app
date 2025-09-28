#ifndef __EXT_FLASH_RECEIVER_H
#define __EXT_FLASH_RECEIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "main.h"

#define ETX_FRAME_SOF  0xAAU    // Start of Frame
#define ETX_FRAME_EOF  0xBBU    // End of Frame
#define ETX_FRAME_ACK  0x00U    // ACK
#define ETX_FRAME_NACK 0x01U    // NACK

#define ETX_APP_SLOT_A_FLASH_ADDR  0x08060000UL   //App slot 0 address
#define ETX_APP_SLOT_B_FLASH_ADDR  0x08110000UL   //App slot 1 address
#define ETX_CONFIG_FLASH_ADDR     0x08020000UL   //Configuration's address

#define ETX_NO_OF_SLOTS           2            //Number of slots
#define ETX_SLOT_MAX_SIZE        (704 * 1024)  //Each slot size (704KB)

#define ETX_FRAME_DATA_MAX_SIZE ( 1024 )  //Maximum data Size
#define ETX_FRAME_DATA_OVERHEAD (    9 )  //data overhead
#define ETX_FRAME_PACKET_MAX_SIZE ( ETX_FRAME_DATA_MAX_SIZE + ETX_FRAME_DATA_OVERHEAD )

/*
 * Reboot reason
 */
#define ETX_FIRST_TIME_BOOT       ( 0x00000000 )      // First time boot
#define ETX_NORMAL_BOOT           ( 0xBEEFFEED )      // Normal Boot
#define ETX_DL_REQUEST            ( 0xDEADBEEF )      // Download request

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


/*
 * Application slots
 */
typedef enum
{
  ETX_SLOT_A = 0,
  ETX_SLOT_B = 1,
}ETX_SLOT_;

/*
 * Application slot information
 */
typedef struct
{
  bool      is_active;       // Is this slot active
  bool      is_occupied;     // Is this slot occupied
  bool      is_bootable;     // is this slot bootable
  bool      reserved;        // Reserved for future use
  uint32_t  slot_addr;       // Slot start address
  uint32_t  slot_size;       // Slot size
  uint32_t  app_size;        // Application size
  uint32_t  app_crc;         // Application CRC
}__attribute__((packed)) ETX_APP_SLOT_INFO_;

/*
 * Configuration information
 */
typedef struct
{
  uint32_t           reboot_reason;               // Reboot reason
  ETX_SLOT_          active_slot;                 // Active slot number
  uint32_t           slot_count;                  // Number of slots
  ETX_APP_SLOT_INFO_ slot_info[ETX_NO_OF_SLOTS];  // Slot information
  uint32_t           reserved[10];                // Reserved for future use
  uint32_t           config_valid_marker;         // Configuration valid marker always 0xDEADBEEF
  uint32_t           config_crc;                  // Configuration CRC
}__attribute__((packed)) ETX_CONFIG_;


ETX_DL_EX_ etx_app_download_and_flash( void );

#ifdef __cplusplus
}
#endif

#endif /* __EXT_FLASH_RECEIVER_H */
