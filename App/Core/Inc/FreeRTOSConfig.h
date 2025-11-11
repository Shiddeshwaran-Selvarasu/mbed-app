#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 *----------------------------------------------------------*/

/* * Ensure stdint.h is available. This is required by FreeRTOS.
 * You might also need to include your main stm32h7xx.h file 
 * if SystemCoreClock is not defined elsewhere.
 */
#include "system_stm32h7xx.h"
#include <stdint.h>
extern uint32_t SystemCoreClock;

/* ====================================================================== */
/* === Mandatory Kernel and Hardware Settings =========================== */
/* ====================================================================== */

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     1 /* Enable the tick hook */
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 ) /* 1ms tick */
#define configCPU_CLOCK_HZ                      ( SystemCoreClock )
#define configMAX_PRIORITIES                    ( 5 ) /* Keep this low, 5-7 is typical */
#define configMINIMAL_STACK_SIZE                ( ( uint16_t ) 128 )  /* In WORDS (128 * 4 = 512 bytes) */
#define configMAX_TASK_NAME_LEN                 ( 16 )
#define configUSE_16_BIT_TICKS                  0 /* 0 for 32-bit tick, required for H7 */

/* ====================================================================== */
/* === Memory Allocation Settings ======================================= */
/* ====================================================================== */

/* This is the total heap size available to pvPortMalloc() */
/* (e.g., for tasks, queues, semaphores...) */
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 64 * 1024 ) ) /* 64K */
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0 /* Set to 1 if you want to create tasks statically */

/* ====================================================================== */
/* === Cortex-M7 Core & Interrupt Settings (CRITICAL!) ================== */
/* ====================================================================== */

/* * The STM32H7 has 4 bits for interrupt priority, giving 16 levels (0-15).
 * A lower number is a HIGHER priority.
 * * FreeRTOS requires you to set ALL priority bits as "pre-emption priority"
 * bits. You MUST call this in your main() before starting the scheduler:
 *
 * NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
 */
#define configPRIO_BITS                         4

/* * The lowest possible interrupt priority.
 * With 4 bits, this is 15.
 */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY   15

/* * The highest interrupt priority (lowest numerical value) from which
 * you can safely call an interrupt-safe FreeRTOS API function
 * (e.g., xQueueSendFromISR).
 *
 * *** DO NOT set this to 0! ***
 * * Any interrupt set to a priority *numerically lower* (i.e., higher logical 
 * priority) than this value (e.g., 0-4) CANNOT use FreeRTOS API calls.
 * Any interrupt set to this value or *numerically higher* (e.g., 5-15) CAN.
 */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

/*
 * These are the raw hardware priority values used by the kernel itself.
 * Do not change these.
 */
#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* ====================================================================== */
/* === Handler Renaming (Required by the port) ========================== */
/* ====================================================================== */
/* This maps the FreeRTOS handlers to the names in your startup .s file */

#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

/* Disable handler installation check since we're using renamed handlers */
#define configCHECK_HANDLER_INSTALLATION        0

/* ====================================================================== */
/* === Optional Feature Includes (Set to 1 to enable) =================== */
/* ====================================================================== */
/* These control which API functions are included in the build. */

#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1 /* Useful for debugging stack overflows */

/* Optional features you copied: */
#define configUSE_TIMERS                        1 /* For software timers */
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            ( configMINIMAL_STACK_SIZE * 2 )

#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_MALLOC_FAILED_HOOK            1 /* Set to 1 to add vApplicationMallocFailedHook() */
#define configCHECK_FOR_STACK_OVERFLOW          2 /* Set to 2 for full stack checking */

/* ====================================================================== */
/* === Debugging and Asserts ============================================ */
/* ====================================================================== */

/* * configASSERT() is a macro that behaves like a standard assert().
 * If the expression passes, nothing happens. If it fails, it calls 
 * the function you define. This is invaluable for catching errors,
 * especially interrupt priority misconfigurations.
 */
extern void vAssertCalled( const char * pcFile, uint32_t ulLine );
#define configASSERT( x ) if( ( x ) == 0 ) vAssertCalled( __FILE__, __LINE__ )

#endif /* FREERTOS_CONFIG_H */
