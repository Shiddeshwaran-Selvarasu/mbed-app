#include "main.h"
#include "logger.h"
#include "stdio.h"

/* Bootloader Version Info start */
#define Major_VERSION  1
#define Minor_VERSION  0
#define Patch_VERSION  0

#define __STRINGIFY(x) #x
#define _STRINGIFY(x) __STRINGIFY(x)
#define BL_VERSION   "v"_STRINGIFY(Major_VERSION)"."_STRINGIFY(Minor_VERSION)"."_STRINGIFY(Patch_VERSION)
#define BL_VER_STRING "Bootloader Version " BL_VERSION " stable release"
/* Bootloader Version Info end */

#define APPLICATION_ADDRESS     (uint32_t)0x08040000U

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);

static void MX_GPIO_DeInit(void);
static void MX_USART2_UART_DeInit(void);
static void MX_USART3_UART_DeInit(void);

static void goto_application( void );

/**
  * @brief  The Bootloader entry point.
  * @retval int
  */
int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();

  LOG_INFO("%s\r\n", BL_VER_STRING);

  GPIO_PinState ota_pin_state;
  __uint32_t timeout = HAL_GetTick() + 5000; // 5 seconds timeout

  LOG_INFO("Press the USER Button to switch to Download Mode...\r\n");

  do {
    ota_pin_state = HAL_GPIO_ReadPin(OTA_BTN_GPIO_Port, OTA_BTN_Pin);

    if (ota_pin_state == GPIO_PIN_SET || HAL_GetTick() > timeout) {
      break;
    }
  } while (1);

  if (ota_pin_state == GPIO_PIN_SET) {
    LOG_INFO("OTA Button Pressed. Entering OTA Mode...\r\n");
    // TODO: Add OTA update logic here....
  } else {
    LOG_INFO("Loading application...\r\n");
    goto_application();
  }

  while (1)
  {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    HAL_Delay(5000);
    LOG_DEBUG("LED BLINKING....\r\n");
  }
}

static void goto_application( void )
{
  /* Reset the peripherals */
  MX_USART2_UART_DeInit();
  MX_USART3_UART_DeInit();
  MX_GPIO_DeInit();

  typedef void (*pFunction)(void);
  __uint32_t jump_address = *(__IO __uint32_t*) (APPLICATION_ADDRESS + 4U);
  pFunction jump_to_application = (pFunction) jump_address;

  /* Initialize user application's Stack Pointer */
  __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);

  /* Reset the Clock */
  HAL_RCC_DeInit();
  HAL_DeInit();
  SysTick->LOAD = 0;
  SysTick->VAL = 0;

  jump_to_application();
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
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
  /* USART2 Port Clock Enable */
  __HAL_RCC_USART2_CLK_ENABLE();

  huart2.Instance = USART2;
  
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK) Error_Handler();
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK) Error_Handler();
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK) Error_Handler();
}

/**
  * @brief USART2 Deinitialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_DeInit(void)
{
  if (HAL_UART_DeInit(&huart2) != HAL_OK) Error_Handler();
  __HAL_RCC_USART2_CLK_DISABLE();
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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : OTA_BTN_Pin */
  GPIO_InitStruct.Pin = OTA_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTA_BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED1_Pin */
  GPIO_InitStruct.Pin = LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);
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

  /* Deconfigure GPIO pin : OTA_BTN_Pin */
  HAL_GPIO_DeInit(OTA_BTN_GPIO_Port, OTA_BTN_Pin);

  /* Deconfigure GPIO pins : LED1_Pin */
  HAL_GPIO_DeInit(LED1_GPIO_Port, LED1_Pin);
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