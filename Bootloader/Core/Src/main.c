#include "main.h"
#include <stdio.h>
#include <stdlib.h>

#include "logger.h"
#include "crc_helper.h"
#include "conf_helper.h"
#include "ext_flash_reciever.h"

/* Bootloader Version Info start */
#define Major_VERSION  3
#define Minor_VERSION  1
#define Patch_VERSION  7

#define __STRINGIFY(x) #x
#define _STRINGIFY(x) __STRINGIFY(x)
#define BL_VERSION   "v"_STRINGIFY(Major_VERSION)"."_STRINGIFY(Minor_VERSION)"."_STRINGIFY(Patch_VERSION)
#define BL_VER_STRING "Bootloader Version " BL_VERSION " stable release"
/* Bootloader Version Info end */

/* Create a config data in Ram and load data from flash */
ETX_CONFIG_ *etx_config;

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
static uint32_t get_application_crc( void );
static uint32_t verify_application_crc(uint32_t crc_value);
static void validate_config( void );

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

  etx_config = (ETX_CONFIG_ *)malloc(sizeof(ETX_CONFIG_));
  config_get(etx_config);

  validate_config(); // Validate and load configuration

  GPIO_PinState ota_pin_state;
  uint32_t timeout = HAL_GetTick() + 5000; // 5 seconds timeout

  /*
   * crc_check_status values: 
   * 1 - Yes
   * 0 - not set
   * 
   * In total 8 bits:
   * LSB - first time boot
   * 2nd - application failed
   * 3rd - download requested from app
   * 4th - button pressed for Download mode
   * others - not in use
   */
  uint8_t etx_app_download_required = 0;

  /************************** Check if APP DL required - START *************************/

  if (etx_config->reboot_reason == ETX_FIRST_TIME_BOOT) {
    LOG_INFO("First time boot detected...\r\n");
    etx_app_download_required |= 0x01; // Set 1st bit to indicate first time boot
  } else if (etx_config->reboot_reason == ETX_DL_REQUEST) {
    LOG_INFO("Reboot reason code: 0x%08lX\r\n", etx_config->reboot_reason);
    etx_app_download_required |= 0x04; // Set 3rd bit to indicate download requested from app
  } else if (etx_config->reboot_reason == ETX_APP_FAILED) {
    LOG_INFO("Application failure detected...\r\n");
    etx_app_download_required |= 0x02; // Set 2nd bit to indicate application failure
  } else {
    etx_app_download_required = 0;
  }

  if (etx_app_download_required == 0) {
    LOG_INFO("Press the USER Button to switch to Download Mode...\r\n");

    do {
      ota_pin_state = HAL_GPIO_ReadPin(OTA_BTN_GPIO_Port, OTA_BTN_Pin);

      if (ota_pin_state == GPIO_PIN_SET || HAL_GetTick() > timeout) {
        break;
      }
    } while (1);

    if (ota_pin_state == GPIO_PIN_SET) {
      LOG_INFO("USER Button Pressed. Entering Download Mode...\r\n");
      etx_app_download_required |= 0x08; // Set 4th bit to indicate button pressed for Download mode
    }
  }

  /*************************** Check if APP DL required - END **************************/

  /******************** Initiate ETX APP DL through USART2 - START *********************/

  if (etx_app_download_required != 0) {
    ETX_DL_EX_ dl_status = etx_app_download_and_flash(etx_config);
    if (dl_status == ETX_DL_EX_ERR) {
      LOG_ERROR("ETX APP Download failed...\r\n");
      etx_config->is_app_bootable = false;
    } else if (dl_status == ETX_DL_EX_ABORT) {
      LOG_INFO("ETX APP Download aborted before writing to flash...\r\n");
    } else {
      LOG_INFO("ETX APP Download successful...\r\n");
    }
  }

  /********************* Initiate ETX APP DL through USART2 - END **********************/

  /*********************** Initiate Jump to application - START ************************/

  if (etx_config->is_app_flashed) {
    uint32_t app_crc = get_application_crc();
    if (app_crc < 0) {
      LOG_ERROR("Failed to get application CRC. Error code: %d\r\n", app_crc);
      etx_config->is_app_bootable = false;
    } else {
      LOG_INFO("Application CRC: 0x%08lX\r\n", app_crc);
      LOG_INFO("Verifying application CRC...\r\n");
      int verify_status = verify_application_crc((uint32_t)app_crc);
      if (verify_status == 0) {
        etx_config->is_app_bootable = true;
        LOG_INFO("CRC verified successfully...\r\n");
        LOG_INFO("Loading application...\r\n");
        goto_application();
      } else {
        LOG_ERROR("CRC verification failed. Error code: %d\r\n", verify_status);
        etx_config->is_app_bootable = false;
      }
    }
    if (config_save(etx_config) != CFG_SAVE_OK) {
      LOG_ERROR("Failed to save updated configuration\r\n");
    }
  }

  /************************ Initiate Jump to application - END *************************/

  LOG_INFO("Staying in Bootloader mode...\r\n");

  while (1)
  {
    HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin); // Application jump failed
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
  uint32_t jump_address = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4U);
  pFunction jump_to_application = (pFunction) jump_address;

  /* Initialize user application's Stack Pointer */
  __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);

  jump_to_application();
}

/**
 * @brief  Get the stored CRC value from configuration data
 * @param  None
 * @retval CRC value (32-bit integer) or negative values on error
 */
static uint32_t get_application_crc( void )
{
  uint32_t *app_crc = (uint32_t *)&etx_config->app_crc;

  if (app_crc == NULL) {
    return -1; // Invalid address
  }

  if (*app_crc == 0xFFFFFFFF || *app_crc == 0x00000000) {
    return -2; // Invalid CRC value
  }

  return *app_crc;
}

/**
 * @brief  Verify the application CRC against computed CRC
 * @param  crc_value: The expected CRC value to compare against
 * @retval 0 if CRC matches, negative values on error
 */
static uint32_t verify_application_crc(uint32_t crc_value)
{
  if (crc_value == 0xFFFFFFFF || crc_value == 0x00000000) {
    return -2; // Invalid CRC value
  }

  // Calculate CRC over application data using STM32 hardware CRC peripheral
  uint32_t data_size_bytes = etx_config->app_size; 
  
  uint32_t computed_crc = compute_crc32(&hcrc, (uint32_t *)APPLICATION_ADDRESS, data_size_bytes);

  if (computed_crc != crc_value) {
    LOG_ERROR("Computed CRC: 0x%08lX, Expected CRC: 0x%08lX\r\n", computed_crc, crc_value);
    return -3; // CRC mismatch
  }

  return 0; // CRC matches
}

/**
 * @brief  valiade the configuration data in flash
 * @param  None
 * @retval None
 */
void validate_config( void )
{
  if (etx_config->config_valid_marker != VALID_CONF_MARKER) {
    LOG_ERROR("Invalid configuration marker: 0x%08lX\r\n", etx_config->config_valid_marker);
    config_load_defaults(etx_config);
    if (config_save(etx_config) != CFG_SAVE_OK) {
      LOG_ERROR("Failed to save default configuration\r\n");
    }
    return;
  }

  if(verify_crc32(&hcrc, (uint32_t *)etx_config, sizeof(ETX_CONFIG_) - 4, etx_config->config_crc) != CRC_OK) {
    LOG_ERROR("Configuration CRC mismatch. Expected: 0x%08lX\r\n", etx_config->config_crc);
    config_load_defaults(etx_config);
    if (config_save(etx_config) != CFG_SAVE_OK) {
      LOG_ERROR("Failed to save default configuration\r\n");
    }
    return;
  }
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
  huart2.Instance = USART2;
  
  huart2.Init.BaudRate = 921600;  // Increased from default for faster transfer
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_8;  // Reduce oversampling for higher speeds
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
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

  /* Deinitialize GPIO pin : OTA_BTN_Pin */
  HAL_GPIO_DeInit(OTA_BTN_GPIO_Port, OTA_BTN_Pin);

  /* Deinitialize GPIO pins : LED1_Pin */
  HAL_GPIO_DeInit(LED1_GPIO_Port, LED1_Pin | LED3_Pin);

  /* Deinitialize GPIO pin : LED2_Pin */
  HAL_GPIO_DeInit(LED2_GPIO_Port, LED2_Pin);
}

/************************************************************* 
* Redirect the C library printf function to the USART.
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
