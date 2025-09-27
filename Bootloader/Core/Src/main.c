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
#define APPLICATION_MAX_SIZE   (128 * 1024)  // 128 KB
#define APPLICATION_CRC_ADDRESS (APPLICATION_ADDRESS + APPLICATION_MAX_SIZE - 4)

CRC_HandleTypeDef hcrc;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_CRC_Init(void);

static void MX_GPIO_DeInit(void);
static void MX_USART2_UART_DeInit(void);
static void MX_USART3_UART_DeInit(void);
static void MX_CRC_DeInit(void);

static void goto_application( void );
static __uint32_t get_application_crc( void );
static __uint32_t verify_application_crc(__uint32_t crc_value);

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
  MX_CRC_Init();

  LOG_INFO("%s\r\n", BL_VER_STRING);

  GPIO_PinState ota_pin_state;
  __uint32_t timeout = HAL_GetTick() + 5000; // 5 seconds timeout
  /**
   * crc_check_status values: 
   * 1 - Failed
   * 0 - Unknown (not checked yet)
   * 
   * In total 8 bits(rrrrrcoa) (crc status + ota status + app jump status + reserved):
   * c - crc_check_status
   * o - ota_mode_status 
   * a - app_jump_status
   * r - reserved for future use
   */
  __uint8_t application_boot_status = 0;

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
    if (0) {
      // OTA successful
      // TODO: add reboot logic here....
    } else {
      // OTA failed
      application_boot_status |= 0x02; // Set ota_mode_status to 1 (failed)
      LOG_ERROR("OTA Update failed. Staying in Bootloader mode...\r\n");
    }
  } else {
    LOG_INFO("Checking for valid application...\r\n");
    __uint32_t app_crc = get_application_crc();
    if (app_crc < 0) {
      LOG_ERROR("Failed to get application CRC. Error code: %d\r\n", app_crc);
      LOG_INFO("Staying in Bootloader mode...\r\n");
    } else {
      LOG_INFO("Application CRC: 0x%08X\r\n", app_crc);
      LOG_INFO("Verifying application CRC...\r\n");
      int verify_status = verify_application_crc((__uint32_t)app_crc);
      if (verify_status == 0) {
        LOG_INFO("Application CRC verified successfully.\r\n");
        LOG_INFO("Loading application...\r\n");
        goto_application();
        application_boot_status |= 0x01; // Set app_jump_status to 1 (failed)
      } else {
        application_boot_status |= 0x04; // Set crc_check_status to 1 (failed)
        LOG_ERROR("Application CRC verification failed. Error code: %d\r\n", verify_status);
        LOG_INFO("Staying in Bootloader mode...\r\n");
      }
    }
  }

  // Indicate boot status code in logs
  if (application_boot_status != 0) LOG_INFO("Boot Status: 0x%02X\r\n", application_boot_status);

  while (1)
  {
    if (application_boot_status & 0x01) HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin); // Application jump failed
    if (application_boot_status & 0x02) HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); // OTA mode failed
    if (application_boot_status & 0x04) HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin); // CRC check failed

    HAL_Delay(2500);
  }
}

/*************************************************************
 * Application Verification and Jump functions
 *************************************************************/

/**
 * @brief  Jump to user application
 * @param  None
 * @retval None
 */
static void goto_application( void )
{ 
  /* Brief delay to ensure UART transmission completes */
  HAL_Delay(100);
  
  /* Reset the peripherals */
  MX_USART2_UART_DeInit();
  MX_USART3_UART_DeInit();
  MX_CRC_DeInit();
  MX_GPIO_DeInit();

  /* Disable all interrupts */
  __disable_irq();
  
  /* Disable SysTick */
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;

  /* Reset the Clock */
  HAL_RCC_DeInit();
  HAL_DeInit();
  
  /* Disable all interrupt lines */
  for(int i = 0; i < 8; i++) {
    NVIC->ICER[i] = 0xFFFFFFFF;
  }
  
  /* Clear all pending interrupts */
  for(int i = 0; i < 8; i++) {
    NVIC->ICPR[i] = 0xFFFFFFFF;
  }

  typedef void (*pFunction)(void);
  __uint32_t jump_address = *(__IO __uint32_t*) (APPLICATION_ADDRESS + 4U);
  pFunction jump_to_application = (pFunction) jump_address;

  /* Initialize user application's Stack Pointer */
  __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);

  /* Set vector table offset to application */
  SCB->VTOR = APPLICATION_ADDRESS;

  /* Enable interrupts for application */
  __enable_irq();

  jump_to_application();
}

/**
 * @brief  Get the stored CRC value from application data
 * @param  None
 * @retval CRC value (32-bit integer) or negative values on error
 */
static __uint32_t get_application_crc( void )
{
  __uint32_t *app_crc_address = (__uint32_t *)APPLICATION_CRC_ADDRESS;

  if (app_crc_address == NULL) {
    return -1; // Invalid address
  }

  if (*app_crc_address == 0xFFFFFFFF || *app_crc_address == 0x00000000) {
    return -2; // Invalid CRC value
  }

  return *app_crc_address;
}

/**
 * @brief  Verify the application CRC against computed CRC
 * @param  crc_value: The expected CRC value to compare against
 * @retval 0 if CRC matches, negative values on error
 */
static __uint32_t verify_application_crc(__uint32_t crc_value)
{
  if (crc_value == 0xFFFFFFFF || crc_value == 0x00000000) {
    return -2; // Invalid CRC value
  }

  // Calculate CRC over application data using STM32 hardware CRC peripheral
  // Data: 131068 bytes (128KB - 4 bytes for CRC storage) = 32767 words exactly
  uint32_t data_size_bytes = APPLICATION_MAX_SIZE - 4;  // 131068 bytes
  uint32_t data_size_words = data_size_bytes / 4;       // 32767 words
  
  // Use word-based processing with half-word inversion enabled
  __uint32_t computed_crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)APPLICATION_ADDRESS, data_size_bytes);

  if (computed_crc != crc_value) {
    LOG_ERROR("Computed CRC: 0x%08X, Expected CRC: 0x%08X\r\n", computed_crc, crc_value);
    return -3; // CRC mismatch
  }

  return 0; // CRC matches
}

/***********************************************************
 * Peripheral initialization and Deinitialization functions
 ***********************************************************/

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
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{  
  hcrc.Instance = CRC;

  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_BYTE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  
  if (HAL_CRC_Init(&hcrc) != HAL_OK) Error_Handler();
}

/**
  * @brief CRC Deinitialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_DeInit(void)
{
  if (HAL_CRC_DeInit(&hcrc) != HAL_OK) Error_Handler();
  __HAL_RCC_CRC_CLK_DISABLE();
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

  /*Configure GPIO pin : OTA_BTN_Pin */
  GPIO_InitStruct.Pin = OTA_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTA_BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED1_Pin and LED3_Pin */
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

  /* Deconfigure GPIO pin : OTA_BTN_Pin */
  HAL_GPIO_DeInit(OTA_BTN_GPIO_Port, OTA_BTN_Pin);

  /* Deconfigure GPIO pins : LED1_Pin */
  HAL_GPIO_DeInit(LED1_GPIO_Port, LED1_Pin | LED3_Pin);

  /* Deconfigure GPIO pin : LED2_Pin */
  HAL_GPIO_DeInit(LED2_GPIO_Port, LED2_Pin);
}

/************************************************************* 
* Retargets the C library printf function to the USART.
**************************************************************/

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

/***************************************************************
*                   Error Handler
****************************************************************/

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