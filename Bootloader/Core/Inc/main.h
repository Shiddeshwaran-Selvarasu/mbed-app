#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"
#include <stdbool.h>

void Error_Handler(void);

#define ETX_NO_OF_SLOTS           2            //Number of slots
#define ETX_SLOT_MAX_SIZE        (704 * 1024)  //Each slot size (704KB)

/*
 * Reboot reason
 */
#define ETX_FIRST_TIME_BOOT       ( 0x00000000 )      // First time boot
#define ETX_NORMAL_BOOT           ( 0xBEEFFEED )      // Normal Boot
#define ETX_DL_REQUEST            ( 0xDEADBEEF )      // Download request

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
  uint32_t           reboot_reason;               // Reboot reason
  uint32_t           reserved[10];                // Reserved for future use
  uint32_t           config_valid_marker;         // Configuration valid marker always 0xDEADBEEF
  uint32_t           config_crc;                  // Configuration CRC
}__attribute__((packed)) ETX_CONFIG_;

// user button 
#define OTA_BTN_Pin GPIO_PIN_13
#define OTA_BTN_GPIO_Port GPIOC

// onboard LED
#define LED1_Pin GPIO_PIN_0
#define LED1_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_1
#define LED2_GPIO_Port GPIOE
#define LED3_Pin GPIO_PIN_14
#define LED3_GPIO_Port GPIOB

// USART2 for application OTA upgrade
#define USART2_TX_Pin GPIO_PIN_2
#define USART2_TX_GPIO_Port GPIOA
#define USART2_RX_Pin GPIO_PIN_3
#define USART2_RX_GPIO_Port GPIOA

// USART3 for logging
#define USART3_TX_Pin GPIO_PIN_8
#define USART3_TX_GPIO_Port GPIOD
#define USART3_RX_Pin GPIO_PIN_9
#define USART3_RX_GPIO_Port GPIOD

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */