/*
 * STM32 Port Configuration for ESP-Hosted
 */

#ifndef __PORT_ESP_HOSTED_HOST_CONFIG_H__
#define __PORT_ESP_HOSTED_HOST_CONFIG_H__

/* STM32 SDK Config - matches sdkconfig.h in this dir */
#include "sdkconfig.h"

/* FreeRTOS Includes */
#include "FreeRTOS.h"
#include "task.h"

/* Helper Macros usually in esp_task.h */
#ifndef H_WEAK_REF
#define H_WEAK_REF __attribute__((weak))
#endif

/* Transports */
#define H_TRANSPORT_NONE 0
#define H_TRANSPORT_SDIO 1
#define H_TRANSPORT_SPI  2
#define H_TRANSPORT_UART 3

/* Select Transport */
#define H_TRANSPORT_IN_USE H_TRANSPORT_SDIO

/* Enable Hosted Host Mode */
#define H_ESP_HOSTED_HOST 1

/* Defaults */
#define H_ESP_HOSTED_DFLT_TASK_STACK 2048
#define H_DFLT_TASK_FROM_SPIRAM 0

#endif
