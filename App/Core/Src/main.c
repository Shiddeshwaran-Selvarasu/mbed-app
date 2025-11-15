#include <stdio.h>

#include "main.h"
#include "logger.h"
#include "FreeRTOS.h"
#include "task.h"

/* Application Version Info start */
#define Major_VERSION  1
#define Minor_VERSION  3
#define Patch_VERSION  0

#define __STRINGIFY(x) #x
#define _STRINGIFY(x) __STRINGIFY(x)
#define APP_VERSION   "v"_STRINGIFY(Major_VERSION)"."_STRINGIFY(Minor_VERSION)"."_STRINGIFY(Patch_VERSION)
#define APP_VER_STRING "Application Version " APP_VERSION " stable release"
/* Application Version Info end */

UART_HandleTypeDef huart3;

void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);

static void MX_GPIO_DeInit(void);
static void MX_USART3_UART_DeInit(void);

static void vTaskApplicationMain(void *pvParameters);
static void vTaskGreenBlink(void *pvParameters);
static void vTaskOrangeBlink(void *pvParameters);
static void vTaskRedBlink(void *pvParameters);

static void shutdown( void );

/* Override FreeRTOS weak function to debug SysTick setup */
void vPortSetupTimerInterrupt( void );

/**
  * @brief  The Application entry point.
  * @retval int
  */
int main(void)
{
  /* CRITICAL: Set NVIC priority grouping FIRST, before HAL_Init()
   * FreeRTOS requires NVIC_PRIORITYGROUP_4 (4 bits for preemption priority) */
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* Initialize the HAL Library */
  HAL_Init();

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();

  LOG_INFO("%s\r\n", APP_VER_STRING);

  /* Reset all LEDs to known state */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);

  LOG_INFO("Creating FreeRTOS tasks...\r\n");

  // Create FreeRTOS tasks
  xTaskCreate(vTaskApplicationMain, "Main Task", 256, NULL, 1, NULL);
  xTaskCreate(vTaskGreenBlink, "Green Blink Task", 256, NULL, 2, NULL);
  xTaskCreate(vTaskOrangeBlink, "Orange Blink Task", 256, NULL, 2, NULL);
  xTaskCreate(vTaskRedBlink, "Red Blink Task", 256, NULL, 2, NULL);

  LOG_INFO("Starting FreeRTOS scheduler...\r\n");
  LOG_INFO("SystemCoreClock = %lu Hz\r\n", SystemCoreClock);

  // Start the FreeRTOS scheduler (it will reconfigure SysTick)
  vTaskStartScheduler();

  /* ... Should never reach here ... */
  /* RTOS failure loop */
  uint16_t fallback_timeout = 50; // (50 * 5) seconds
  while (1)
  {
    LOG_INFO("FreeRTOS failure detected...\r\n");
    HAL_Delay(5000);
    if (--fallback_timeout == 0) {
      LOG_INFO("Performing system shutdown and jump to bootloader...\r\n");
      shutdown();
    }
  }
}

/**
  * @brief  Function implementing the Main thread.
  * @param  pvParameters not used
  * @retval None
  */
static void vTaskApplicationMain(void *pvParameters)
{  
  /* Simple infinite loop for main task */
  while (1)
  {
    LOG_INFO("Main task is running...\r\n");
    vTaskDelay(pdMS_TO_TICKS(2000)); // Log every 2 seconds
  }
}

/**
  * @brief  Function implementing the Green LED blink thread.
  * @param  pvParameters not used
  * @retval None
  */
static void vTaskGreenBlink(void *pvParameters)
{
    while (1)
  {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    vTaskDelay(pdMS_TO_TICKS(500)); // Toggle every 0.5 second
  }
}

/**
  * @brief  Function implementing the Orange LED blink thread.
  * @param  pvParameters not used
  * @retval None
  */
static void vTaskOrangeBlink(void *pvParameters)
{
    while (1)
  {
    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
    vTaskDelay(pdMS_TO_TICKS(1000)); // Toggle every 1 second
  }
}

/**
  * @brief  Function implementing the Red LED blink thread.
  * @param  pvParameters not used
  * @retval None
  */static void vTaskRedBlink(void *pvParameters)
{
    while (1)
  {
    HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
    vTaskDelay(pdMS_TO_TICKS(1500)); // Toggle every 1.5 second
  }
}

/**
  * @brief  Perform application shutdown sequence and jump to bootloader
  * @retval None
  */
static void shutdown( void )
{
  /* Disable all interrupts */
  __disable_irq();
  
  /* Reset the peripherals */
  MX_USART3_UART_DeInit();
  MX_GPIO_DeInit();
  
  /* Deinitialize HAL */
  HAL_DeInit();
  
  /* Reset SysTick */
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;
  
  /* Set MSP to bootloader's stack pointer */
  __set_MSP(*(__IO uint32_t*)0x08000000);

  /* Set vector table offset to bootloader */
  SCB->VTOR = 0x08000000;

  /* Enable interrupts */
  __enable_irq();
  
  /* Jump to bootloader */
  void (*bootloader)(void) = (void (*)(void))(*((uint32_t*)0x08000004));
  bootloader();
  
  /* Should never reach here */
  while(1);
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{
  /* USART3 Port Clock Enable */
  __HAL_RCC_USART3_CLK_ENABLE();

  huart3.Instance = USART3;

  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

  if (HAL_UART_Init(&huart3) != HAL_OK) Error_Handler();
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK) Error_Handler();
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK) Error_Handler();
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK) Error_Handler();
}

/**
  * @brief USART3 Deinitialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_DeInit(void)
{
  if (HAL_UART_DeInit(&huart3) != HAL_OK) Error_Handler();
  __HAL_RCC_USART3_CLK_DISABLE();
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USR_BTN_Pin */
  GPIO_InitStruct.Pin = USR_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USR_BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED1_Pin, LED3_Pin */
  GPIO_InitStruct.Pin = LED1_Pin | LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED2_Pin */
  GPIO_InitStruct.Pin = LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED2_GPIO_Port, &GPIO_InitStruct);
}

/**
  * @brief GPIO Deinitialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_DeInit(void)
{
  /* GPIO Ports Clock Disable */
  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_GPIOD_CLK_DISABLE();
  __HAL_RCC_GPIOE_CLK_DISABLE();

  /* Deconfigure GPIO pin : USR_BTN_Pin */
  HAL_GPIO_DeInit(USR_BTN_GPIO_Port, USR_BTN_Pin);

  /* Deconfigure GPIO pins : LED1_Pin, LED3_Pin */
  HAL_GPIO_DeInit(LED1_GPIO_Port, LED1_Pin|LED3_Pin);

  /* Deconfigure GPIO pins : LED2_Pin */
  HAL_GPIO_DeInit(LED2_GPIO_Port, LED2_Pin);
}

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */ 
int __io_putchar(int ch)
#else
int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
{
  HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {}
}

/* 
  =============================================
  |                 FreeRTOS                  |
  =============================================
*/

/**
  * @brief  FreeRTOS assertion failed
  */
void vAssertCalled( const char * pcFile, uint32_t ulLine )
{
  /* * An assertion has failed. You can find out which one by
   * checking the pcFile and ulLine parameters.
   * * Loop indefinitely so you can catch this in a debugger.
   */
  LOG_ERROR("FreeRTOS Assert Failed: %s:%lu\r\n", pcFile, ulLine);
  taskDISABLE_INTERRUPTS();
  for( ;; );
}

/**
  * @brief  FreeRTOS stack overflow hook
  */
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    /*
     * Run time stack overflow checking is enabled.
     * If you are here, a task has blown its stack.
     * Loop forever so a debugger can attach and find it.
     */
    (void) xTask;
    LOG_ERROR("FreeRTOS Stack Overflow in task: %s\r\n", pcTaskName);
    taskDISABLE_INTERRUPTS();
    for( ;; );
}

/**
  * @brief  FreeRTOS malloc failed hook
  */
void vApplicationMallocFailedHook( void )
{
    /* Called if a call to pvPortMalloc() fails because there is insufficient
     * free memory available in the FreeRTOS heap. */
    LOG_ERROR("FreeRTOS Malloc Failed - insufficient heap memory!\r\n");
    taskDISABLE_INTERRUPTS();
    for( ;; );
}

/**
  * @brief  FreeRTOS Tick Hook function
  * This is called from inside the FreeRTOS Systick handler (port.c)
  */
volatile uint32_t tickHookCounter = 0;
void vApplicationTickHook( void )
{
    /* This function is called from inside the FreeRTOS_Tick_Handler */
    /* and must be used to call HAL_IncTick() */
    tickHookCounter++;
    HAL_IncTick();
}

/**
  * @brief  Period elapsed callback in non-blocking mode
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Check if the interrupt is from our HAL timebase timer (TIM6) */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
}
