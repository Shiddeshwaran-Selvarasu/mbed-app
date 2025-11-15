#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* Get your system clock frequency */
/* You MUST ensure this is correct */
#include "stm32h7xx.h" // Or "main.h" if SystemCoreClock is defined there
extern uint32_t SystemCoreClock;

/*-----------------------------------------------------------
 * Application specific definitions.
 *----------------------------------------------------------*/
#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCPU_CLOCK_HZ                      ( SystemCoreClock )
#define configTICK_RATE_HZ                      ( (TickType_t)1000 ) // 1ms tick
#define configMAX_PRIORITIES                    ( 5 )
#define configMINIMAL_STACK_SIZE                ( (uint16_t)128 )
#define configTOTAL_HEAP_SIZE                   ( (size_t)(10 * 1024) ) // 10KB
#define configMAX_TASK_NAME_LEN                 ( 16 )
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( 2 )
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            ( configMINIMAL_STACK_SIZE * 2 )

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         ( 2 )

/* Set the following definitions to 1 to include the API function, or 0
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
  /* __BVIC_PRIO_BITS will be defined when CMSIS is used. */
  #define configPRIO_BITS                       __NVIC_PRIO_BITS
#else
  #define configPRIO_BITS                       4 /* STM32H7 has 4 bits */
#endif

/* The lowest interrupt priority that can be used in a call to a "FromISR"
 * ISR safe FreeRTOS API function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15

/* The highest interrupt priority that can be safely used by interrupt
 * service routines that call FreeRTOS API functions. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

/* Interrupt priorities used by the kernel itself. These *must* be */
/* set to the lowest priority. */
#define configKERNEL_INTERRUPT_PRIORITY \
  ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
  ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* Definitions that map the FreeRTOS port-specific functions to the
 * standard CMSIS interrupt handler names. */
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */