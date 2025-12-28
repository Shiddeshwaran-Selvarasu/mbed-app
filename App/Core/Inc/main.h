#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"

void Error_Handler(void);

// user button
#define USR_BTN_Pin GPIO_PIN_13
#define USR_BTN_GPIO_Port GPIOC

// onboard LEDs
#define LED1_Pin GPIO_PIN_0
#define LED1_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_1
#define LED2_GPIO_Port GPIOE
#define LED3_Pin GPIO_PIN_14
#define LED3_GPIO_Port GPIOB

// ESP Communication pins
#define ESP_EN_Pin GPIO_PIN_1
#define ESP_EN_GPIO_Port GPIOA
#define ESP_HSK_Pin GPIO_PIN_0
#define ESP_HSK_GPIO_Port GPIOA

// SDMMC1 data pins
#define SDMMC1_D0_Pin GPIO_PIN_8
#define SDMMC1_D1_Pin GPIO_PIN_9
#define SDMMC1_D2_Pin GPIO_PIN_10
#define SDMMC1_D3_Pin GPIO_PIN_11
#define SDMMC1_Data_GPIO_Port GPIOC

// SDMMC1 clock and command pins
#define SDMMC1_CLK_Pin GPIO_PIN_12
#define SDMMC1_CLK_GPIO_Port GPIOC
#define SDMMC1_CMD_Pin GPIO_PIN_2
#define SDMMC1_CMD_GPIO_Port GPIOD

// USART3 for logging
#define USART3_TX_Pin GPIO_PIN_8
#define USART3_TX_GPIO_Port GPIOD
#define USART3_RX_Pin GPIO_PIN_9
#define USART3_RX_GPIO_Port GPIOD

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */