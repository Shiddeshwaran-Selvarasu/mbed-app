#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"
#include <string.h>
#include <stdbool.h>

void Error_Handler(void);

#define HAL_DL_UART_RX_TIMEOUT 10000U /* 10 s */

/* Flash memory addresses */
#define APPLICATION_ADDRESS    0x08100000UL
#define APPLICATION_MAX_SIZE    (1024 * 1024)  // 1024 KB
#define APPLICATION_CRC_ADDRESS (APPLICATION_ADDRESS + APPLICATION_MAX_SIZE - 4)
#define CONFIG_FLASH_ADDR      0x08040000UL
#define CONFIG_SIZE            (0x20000) // 128KB: 0x08040000 - 0x0805FFFF

/*
 * Reboot reason
 */
#define ETX_FIRST_TIME_BOOT       ( 0xAFAFAFAF )      // First time boot go directly to Download mode
#define ETX_NORMAL_BOOT           ( 0xBEEFFEED )      // Normal Boot go to application
#define ETX_DL_REQUEST            ( 0xDEADBEEF )      // Download request go to Download mode
#define ETX_APP_FAILED            ( 0xBAADF00D )      // Application failed switch slot if available and boot or go to Download mode

/*
 * Configuration information
 */
typedef struct
{
  uint32_t           reboot_reason;               // Reboot reason
  bool               is_app_bootable;             // Is application bootable
  bool               is_app_flashed;              // Is application flashed
  uint32_t           reserved[10];                // Reserved for future use
  uint32_t           app_crc;                     // Application CRC
  uint32_t           config_valid_marker;         // Configuration valid marker always 0xDEADBEEF
  uint32_t           config_crc;                  // Configuration CRC
}__attribute__((packed)) ETX_CONFIG_;

extern CRC_HandleTypeDef hcrc;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

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