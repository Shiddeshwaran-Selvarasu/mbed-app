#include <stdio.h>
#include <string.h>

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

/* Task timing constants */
#define MAIN_TASK_LOG_PERIOD_MS      2000U  /* Log every 2 seconds */
#define GREEN_LED_TOGGLE_PERIOD_MS   500U   /* Toggle every 0.5 second */
#define ORANGE_LED_TOGGLE_PERIOD_MS  1000U  /* Toggle every 1 second */
#define RED_LED_TOGGLE_PERIOD_MS     1500U  /* Toggle every 1.5 second */

/* FreeRTOS failure handling */
#define FREERTOS_FAILURE_TIMEOUT_MS  5000U  /* 5 seconds delay */
#define FREERTOS_FAILURE_MAX_RETRIES 50U    /* Max retries before shutdown */

/* Bootloader jump addresses */
#define BOOTLOADER_BASE_ADDRESS      0x08000000UL

UART_HandleTypeDef huart3;
SD_HandleTypeDef hsd1;

void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_SDMMC1_Init(void);
static void MX_USART3_UART_Init(void);

static void MX_GPIO_DeInit(void);
static void MX_SDMMC1_DeInit(void);
static void MX_USART3_UART_DeInit(void);

static void vTaskApplicationMain(void *pvParameters);
static void vTaskESPHandshake(void *pvParameters);

static void shutdown( void );
static int SDMMC_WaitCmdDone(SDMMC_TypeDef *sdmmc, int is_r4_resp);
static int SDIO_Send_CMD0(void);
static int SDIO_Send_CMD3(uint32_t *resp);
static int SDIO_Send_CMD5(uint32_t arg, uint32_t *response);
static int SDIO_Send_CMD7(uint32_t rca);
static int SDIO_Send_CMD52_Write(uint8_t fn, uint32_t addr, uint8_t data);
static int SDIO_Send_CMD52_Read(uint8_t fn, uint32_t addr, uint8_t *data);
static int SDIO_SetBlockSize(void);

static int SDIO_CMD53_Write(uint8_t fn, uint32_t addr, uint8_t *buffer, uint32_t length);
static int SDIO_CMD53_Write_ByteMode(uint8_t fn, uint32_t addr, uint8_t *buffer, uint32_t length);
static int SDIO_CMD53_Write_BlockMode(uint8_t fn, uint32_t addr, uint8_t *buffer, uint32_t blocks);
static int SDIO_CMD53_Read(uint8_t fn, uint32_t addr, uint8_t *buffer, uint32_t length);

void print_hex_dump(uint8_t *buffer, uint32_t len);

/* Override FreeRTOS weak function to debug SysTick setup */
void vPortSetupTimerInterrupt( void );

int is_interrupt_called = 0;

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
  MX_SDMMC1_Init();

  /* Reset all LEDs to known state */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);

  /* Reset ESP_EN Pin*/
  HAL_GPIO_WritePin(ESP_EN_GPIO_Port, ESP_EN_Pin, GPIO_PIN_RESET);

  LOG_INFO("Creating FreeRTOS tasks...\r\n");

  // Create FreeRTOS tasks
  xTaskCreate(vTaskApplicationMain, "Main Task", 256, NULL, 1, NULL);
  xTaskCreate(vTaskESPHandshake, "ESP Handshake Task", 256, NULL, 3, NULL);

  LOG_INFO("Starting FreeRTOS scheduler...\r\n");
  LOG_INFO("SystemCoreClock = %lu Hz\r\n", SystemCoreClock);

  // Start the FreeRTOS scheduler (it will reconfigure SysTick)
  vTaskStartScheduler();

  /* ... Should never reach here ... */
  /* RTOS failure loop */
  uint16_t fallback_timeout = FREERTOS_FAILURE_MAX_RETRIES;
  while (1)
  {
    LOG_INFO("FreeRTOS failure detected...\r\n");
    HAL_Delay(FREERTOS_FAILURE_TIMEOUT_MS);
    if (--fallback_timeout == 0) {
      LOG_INFO("Performing system shutdown and jump to bootloader...\r\n");
      shutdown();
    }
  }
}

/*******************************************************************************
 * FreeRTOS Task Functions
 ******************************************************************************/
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
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    vTaskDelay(pdMS_TO_TICKS(MAIN_TASK_LOG_PERIOD_MS));
  }
}

/**
  * @brief  Function implementing the ESP32 SDIO handshake thread.
  * @param  pvParameters not used
  * @retval None
  */
static void vTaskESPHandshake(void *pvParameters)
{
    uint32_t r4  = 0;
    uint32_t rca = 0;
    uint8_t  io_ready = 0;

    /* FIX: Static + Aligned to prevent Stack Overflow and HardFaults */
    static uint8_t tx_buffer[512] __attribute__((aligned(4)));

    memset(tx_buffer, 0xAA, sizeof(tx_buffer));

    /*----------------------------------------------------------*/
    /* 1. Power up ESP32                                        */
    /*----------------------------------------------------------*/
    /* Assert EN pin to boot ESP32 */
    HAL_GPIO_WritePin(ESP_EN_GPIO_Port, ESP_EN_Pin, GPIO_PIN_SET);
    LOG_INFO("ESP_EN asserted\r\n");

    /* FIX: Reduced delay to catch the "Hello" packet early */
    vTaskDelay(pdMS_TO_TICKS(100));

    /*----------------------------------------------------------*/
    /* 2. CMD0 (Reset)                                          */
    /*----------------------------------------------------------*/
    if (SDIO_Send_CMD0() != 0)
    {
        LOG_ERROR("CMD0 failed\r\n");
        vTaskDelete(NULL);
    }

    /*----------------------------------------------------------*/
    /* 3. CMD5 – IO_SEND_OP_COND                                */
    /*----------------------------------------------------------*/
    LOG_INFO("Polling CMD5...\r\n");

    for (int i = 0; i < 200; i++) // Increased retries for robustness
    {
        if (SDIO_Send_CMD5(0x00FF8000, &r4) == 0)
        {
            if (r4 & (1UL << 31)) // IO_READY bit
            {
                LOG_INFO("ESP32 SDIO READY (R4=0x%08lX)\r\n", r4);
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // Poll faster
        if (i == 199) {
            LOG_ERROR("CMD5 timeout\r\n");
            vTaskDelete(NULL);
        }
    }

    /*----------------------------------------------------------*/
    /* 4. CMD3 – Ask for Relative Card Address (RCA)            */
    /*----------------------------------------------------------*/
    if (SDIO_Send_CMD3(&rca) != 0) {
        LOG_ERROR("CMD3 failed\r\n");
        vTaskDelete(NULL);
    }
    LOG_INFO("CMD3 RCA = 0x%08lX\r\n", rca);

    /*----------------------------------------------------------*/
    /* 5. CMD7 – Select Card                                    */
    /*----------------------------------------------------------*/
    if (SDIO_Send_CMD7(rca) != 0) {
        LOG_ERROR("CMD7 failed\r\n");
        vTaskDelete(NULL);
    }
    LOG_INFO("CMD7: Card selected\r\n");

    /*----------------------------------------------------------*/
    /* 6. Enable Function 1 (IO_ENABLE)                         */
    /*----------------------------------------------------------*/
    /* CCCR 0x02: IO Enable Register. Bit 1 = Function 1 Enable */
    SDIO_Send_CMD52_Write(0, 0x02, 0x02);

    /* Poll IO_READY (CCCR 0x03) until Function 1 is ready */
    io_ready = 0;
    int timeout = 100;
    do {
        SDIO_Send_CMD52_Read(0, 0x03, &io_ready);
        vTaskDelay(pdMS_TO_TICKS(10));
        timeout--;
    } while ((io_ready & 0x02) == 0 && timeout > 0);

    if (timeout == 0) {
        LOG_ERROR("Function 1 Enable Timeout\r\n");
        vTaskDelete(NULL);
    }
    LOG_INFO("FN1 IO_READY confirmed\r\n");

    /*----------------------------------------------------------*/
    /* 7. Set Block Size for Function 1                         */
    /*----------------------------------------------------------*/
    /* Standard requirement for Block Mode transfers */
    SDIO_SetBlockSize(); // Helper function sets 512 bytes

    /*----------------------------------------------------------*/
    /* 8. Enable Interrupts (CRITICAL STEP)                     */
    /*----------------------------------------------------------*/
    /* A. Enable Master Interrupt (IENM) and Fn1 Interrupt (IEN1) in CCCR */
    SDIO_Send_CMD52_Write(0, 0x04, 0x03); 

    /* B. Enable Interrupt Output in Function 1 Space (The Missing Link) */
    /* This tells ESP32 firmware to actually drive the D1 line low */
    SDIO_Send_CMD52_Write(1, 0x04, 0x01); 

    /*----------------------------------------------------------*/
    /* 9. Switch to 4-bit Bus Width                             */
    /*----------------------------------------------------------*/
    /* Tell ESP32 to use 4-bit mode */
    SDIO_Send_CMD52_Write(0, 0x07, 0x02);

    /* Tell STM32 peripheral to use 4-bit mode */
    SDMMC1->CLKCR &= ~SDMMC_CLKCR_WIDBUS;
    SDMMC1->CLKCR |= SDMMC_CLKCR_WIDBUS_0; // 4-bit wide

    /*----------------------------------------------------------*/
    /* 10. CMD53 Write Test (Byte Mode)                         */
    /*----------------------------------------------------------*/
    LOG_INFO("CMD53 Write Test (Byte Mode)...\r\n");
    if (SDIO_CMD53_Write_ByteMode(1, 0x0000, tx_buffer, 8) == 0) {
        LOG_INFO("CMD53 WRITE SUCCESS\r\n");
    } else {
        LOG_ERROR("CMD53 WRITE FAILED\r\n");
    }

    /*----------------------------------------------------------*/
    /* 11. Wait for Data (MAC Address Packet)                   */
    /*----------------------------------------------------------*/
    LOG_INFO("Waiting for ESP32 Data (MAC Address)...\r\n");

    /* Enable Interrupt Mask in STM32 SDMMC Peripheral */
    SDMMC1->MASK |= SDMMC_MASK_SDIOITIE;

    while (1)
    {
        uint8_t int_pending = 0;
        int has_data = 0;

        /* Check 1: Hardware Interrupt Flag (D1 line dropped) */
        if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_SDIOIT))
        {
             __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_SDIOIT);
             has_data = 1;
        }
        
        /* Check 2: Software Poll (CCCR 0x05) - Backup check */
        if (!has_data) {
             SDIO_Send_CMD52_Read(0, 0x05, &int_pending);
             if (int_pending & 0x02) {
                 has_data = 1;
             }
        }

        if (has_data)
        {
            LOG_INFO("Data Available! Reading FIFO...\r\n");
            
            memset(tx_buffer, 0, 512);

            /* Read 512 bytes from FIFO Address 0x1F800 */
            /* Using Block Mode Read */
            if (SDIO_CMD53_Read(1, 0x1F800, tx_buffer, 512) == 0)
            {
                LOG_INFO("Packet Received! Hex Dump (First 64 bytes):\r\n");
                
                /* Print Hex Dump to console */
                for(int i = 0; i < 64; i++) {
                    printf("%02X ", tx_buffer[i]);
                    if ((i+1) % 16 == 0) printf("\r\n");
                }
                printf("\r\n");

                /* Task complete - we found our packet! */
                HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET); // Orange LED On
                break; 
            }
            else
            {
                LOG_ERROR("Read Failed. Retrying...\r\n");
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    LOG_INFO("Handshake Task Complete.\r\n");
    vTaskDelete(NULL);
}
 
/*******************************************************************************
 * Application Helper Function
 ******************************************************************************/

static int SDMMC_WaitCmdDone(SDMMC_TypeDef *sdmmc, int is_r4_resp)
{
    uint32_t timeout = 1000000; // Increase this if needed for slow clocks

    while (timeout--)
    {
        uint32_t sta = sdmmc->STA;

        // 1. Check if the Command Response was received
        if (sta & SDMMC_STA_CMDREND)
        {
            sdmmc->ICR = SDMMC_ICR_CMDRENDC; // Clear flag
            return 0; // SUCCESS!
        }
        
        // 2. Handle Errors
        if (sta & SDMMC_STA_CTIMEOUT)
        {
            sdmmc->ICR = SDMMC_ICR_CTIMEOUTC;
            return -2; // Timeout
        }

        if (sta & SDMMC_STA_CCRCFAIL)
        {
            sdmmc->ICR = SDMMC_ICR_CCRCFAILC;

            // FIX: If this is CMD5 (R4 response), a CRC fail is NORMAL. 
            // We should treat it as success.
            if (is_r4_resp) {
                return 0; // Success (Ignore CRC)
            }
            
            return -3; // Genuine CRC Error
        }
    }
    return -1; // Loop exhausted
}

static int SDIO_Send_CMD0(void)
{
  SDMMC1->ICR = 0xFFFFFFFF;
  SDMMC1->ARG = 0;
  SDMMC1->CMD = (0U) | SDMMC_CMD_CPSMEN; // no WAITRESP

  while (!(SDMMC1->STA & SDMMC_STA_CMDSENT))
  {
    if (SDMMC1->STA & SDMMC_STA_CTIMEOUT)
      return -1;
  }
  return 0;
}

static int SDIO_Send_CMD3(uint32_t *resp)
{
    SDMMC1->ICR = 0xFFFFFFFF;
    SDMMC1->ARG = 0;
    SDMMC1->CMD = (3U) | SDMMC_CMD_WAITRESP_0 | SDMMC_CMD_CPSMEN;

    if (SDMMC_WaitCmdDone(SDMMC1, 0) != 0)
        return -1;

    *resp = SDMMC1->RESP1;
    return 0;
}

static int SDIO_Send_CMD5(uint32_t arg, uint32_t *resp)
{
  SDMMC_TypeDef *sdmmc = SDMMC1;
  sdmmc->ICR = 0xFFFFFFFF;
  sdmmc->ARG = arg;
  sdmmc->CMD = (5U) | SDMMC_CMD_WAITRESP_0 | SDMMC_CMD_CPSMEN;

  // Pass '1' because CMD5 expects an R4 response (No CRC)
  if (SDMMC_WaitCmdDone(sdmmc, 1) != 0) return -1;

  *resp = sdmmc->RESP1;
  return 0;
}

static int SDIO_Send_CMD7(uint32_t rca)
{
  SDMMC1->ICR = 0xFFFFFFFF;
  SDMMC1->ARG = rca;
  SDMMC1->CMD = (7U) | SDMMC_CMD_WAITRESP_0 | SDMMC_CMD_CPSMEN;

  return SDMMC_WaitCmdDone(SDMMC1, 0);
}

static int SDIO_Send_CMD52_Write(uint8_t fn, uint32_t addr, uint8_t data)
{
  uint32_t arg =
      (1UL << 31) |           // Write
      ((fn & 0x7) << 28) |
      ((addr & 0x1FFFF) << 9) |
      (data & 0xFF);

  SDMMC1->ICR = 0xFFFFFFFF;
  SDMMC1->ARG = arg;
  SDMMC1->CMD = (52U) | SDMMC_CMD_WAITRESP_0 | SDMMC_CMD_CPSMEN;

  return SDMMC_WaitCmdDone(SDMMC1, 0);
}

static int SDIO_Send_CMD52_Read(uint8_t fn, uint32_t addr, uint8_t *data)
{
  SDMMC1->ICR = 0xFFFFFFFF;
  // Direction: Read (0), Function: fn, Address: addr
  uint32_t arg = (0UL << 31) | ((fn & 0x7) << 28) | ((addr & 0x1FFFF) << 9);
  
  SDMMC1->ARG = arg;
  SDMMC1->CMD = (52U) | SDMMC_CMD_WAITRESP_0 | SDMMC_CMD_CPSMEN;

  if (SDMMC_WaitCmdDone(SDMMC1, 0) != 0) return -1; // Note: Pass 0 for WaitResp if using my previous fix, or 1 if it expects R4? 
  // CORRECTION: CMD52 expects R5, which has CRC. So use CRC checking!
  // Use SDMMC_WaitCmdDone(SDMMC1, 0) assuming 0 = Expect CRC (Normal).
  
  // R5 Response Format: [31:24] stuff, [23:16] status, [15:8] stuff, [7:0] data
  *data = (uint8_t)(SDMMC1->RESP1 & 0xFF);
  return 0;
}

static int SDIO_SetBlockSize(void)
{
    /* FBR (Function Basic Registers) for Function 1 start at 0x100.
       Block Size Register is at 0x110 (LSB) and 0x111 (MSB).
       We want 512 bytes = 0x0200.
    */
    
    // 1. Write LSB (0x00) to address 0x110
    if (SDIO_Send_CMD52_Write(0, 0x110, 0x00) != 0) return -1;
    
    // 2. Write MSB (0x02) to address 0x111
    if (SDIO_Send_CMD52_Write(0, 0x111, 0x02) != 0) return -1;
    
    LOG_INFO("SDIO Block Size set to 512 bytes\r\n");
    return 0;
}

static int SDIO_CMD53_Write(uint8_t fn, uint32_t addr, uint8_t *buffer, uint32_t length)
{
    SDMMC_DataInitTypeDef config;
    uint32_t timeout;

    /* 1. Clear DATA flags */
    SDMMC1->ICR = SDMMC_ICR_DATAENDC |
                  SDMMC_ICR_DCRCFAILC |
                  SDMMC_ICR_DTIMEOUTC |
                  SDMMC_ICR_TXUNDERRC |
                  SDMMC_ICR_RXOVERRC;

    /* 2. Configure DPSM */
    config.DataTimeOut   = 0xFFFFFFFF;
    config.DataLength    = length;
    config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
    config.TransferDir   = SDMMC_TRANSFER_DIR_TO_CARD;
    config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
    config.DPSM          = SDMMC_DPSM_ENABLE;
    SDMMC_ConfigData(SDMMC1, &config);

    /* 3. CMD53 argument */
    uint32_t block_count = length / 512;
    uint32_t arg =
        (1UL << 31) |               /* Write */
        ((fn & 0x7) << 28) |
        (1UL << 27) |               /* Block mode */
        (1UL << 26) |               /* Increment address */
        ((addr & 0x1FFFF) << 9) |
        (block_count & 0x1FF);

    SDMMC1->ARG = arg;
    SDMMC1->CMD = (53U) | SDMMC_CMD_WAITRESP_0 | SDMMC_CMD_CPSMEN;

    /* 4. Wait for command response */
    if (SDMMC_WaitCmdDone(SDMMC1, 0) != 0)
        return -1;

    /* 5. TX FIFO write loop */
    uint32_t *p = (uint32_t *)buffer;
    uint32_t words = length / 4;

    timeout = 1000000;
    while (words > 0)
    {
        uint32_t sta = SDMMC1->STA;

        if (sta & SDMMC_STA_TXFIFOHE)
        {
            SDMMC1->FIFO = *p++;
            words--;
            timeout = 1000000;
        }
        else if (sta & (SDMMC_STA_DCRCFAIL | SDMMC_STA_DTIMEOUT))
        {
            return -2;
        }

        if (--timeout == 0)
            return -3;
    }

    /* 6. Wait for DATAEND */
    timeout = 1000000;
    while (!(SDMMC1->STA & SDMMC_STA_DATAEND))
    {
        if (SDMMC1->STA & (SDMMC_STA_DCRCFAIL | SDMMC_STA_DTIMEOUT))
            return -4;

        if (--timeout == 0)
            return -5;
    }

    SDMMC1->ICR = SDMMC_ICR_DATAENDC;
    return 0;
}

static int SDIO_CMD53_Write_ByteMode(uint8_t fn,
                                    uint32_t addr,
                                    uint8_t *buffer,
                                    uint32_t length)
{
    SDMMC_DataInitTypeDef data;

    /* 1. Configure data path for BYTE MODE */
    data.DataTimeOut   = SDMMC_DATATIMEOUT;
    data.DataLength    = length;
    data.DataBlockSize = SDMMC_DATABLOCK_SIZE_1B;
    data.TransferDir   = SDMMC_TRANSFER_DIR_TO_CARD;
    data.TransferMode  = SDMMC_TRANSFER_MODE_STREAM; // BYTE MODE
    data.DPSM          = SDMMC_DPSM_ENABLE;
    SDMMC_ConfigData(SDMMC1, &data);

    /* 2. CMD53 argument:
       Write | Fn | ByteMode | IncrementAddr | Address | Count
    */
    uint32_t arg =
        (1UL << 31) |                 // Write
        ((fn & 0x7) << 28) |
        (0UL << 27) |                 // BYTE MODE
        (1UL << 26) |                 // Increment address
        ((addr & 0x1FFFF) << 9) |
        (length & 0x1FF);             // Byte count

    SDMMC1->ICR = 0xFFFFFFFF;
    SDMMC1->ARG = arg;
    SDMMC1->CMD = (53U) | SDMMC_CMD_WAITRESP_0 | SDMMC_CMD_CPSMEN;

    /* 3. Wait for CMD response (R5) */
    if (SDMMC_WaitCmdDone(SDMMC1, 0) != 0) {
        LOG_ERROR("CMD53 R5 failed\r\n");
        return -1;
    }

    /* 4. Push data */
    uint32_t timeout = 1000000;
    uint32_t words = (length + 3) / 4;
    uint32_t *ptr = (uint32_t *)buffer;

    while (words && timeout--) {
        if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_TXFIFOHE)) {
            SDMMC1->FIFO = *ptr++;
            words--;
        }
    }

    if (timeout == 0) {
        LOG_ERROR("CMD53 TX FIFO timeout\r\n");
        return -2;
    }

    /* 5. Wait for data end */
    timeout = 1000000;
    while (!(SDMMC1->STA & SDMMC_STA_DATAEND) && timeout--) {
        if (SDMMC1->STA & SDMMC_STA_DTIMEOUT) {
            LOG_ERROR("CMD53 DTIMEOUT (ESP not driving DAT lines)\r\n");
            return -3;
        }
        if (SDMMC1->STA & SDMMC_STA_DCRCFAIL) {
            LOG_ERROR("CMD53 DCRCFAIL\r\n");
            return -4;
        }
    }

    SDMMC1->ICR = SDMMC_ICR_DATAENDC;
    return 0;
}

static int SDIO_CMD53_Write_BlockMode(uint8_t fn,
                                     uint32_t addr,
                                     uint8_t *buffer,
                                     uint32_t blocks)
{
    SDMMC_DataInitTypeDef data;

    data.DataTimeOut   = SDMMC_DATATIMEOUT;
    data.DataLength    = blocks * 512;
    data.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
    data.TransferDir   = SDMMC_TRANSFER_DIR_TO_CARD;
    data.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
    data.DPSM          = SDMMC_DPSM_ENABLE;

    SDMMC_ConfigData(SDMMC1, &data);

    uint32_t arg =
        (1UL << 31) |
        ((fn & 0x7) << 28) |
        (1UL << 27) |           // BLOCK MODE
        (1UL << 26) |
        ((addr & 0x1FFFF) << 9) |
        (blocks & 0x1FF);

    SDMMC1->ARG = arg;
    SDMMC1->CMD = (53U) | SDMMC_CMD_WAITRESP_0 | SDMMC_CMD_CPSMEN;

    if (SDMMC_WaitCmdDone(SDMMC1, 0) != 0)
        return -1;

    uint32_t *buf = (uint32_t *)buffer;
    uint32_t words = blocks * 128;

    while (words)
    {
        if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_TXFIFOHE))
        {
            SDMMC1->FIFO = *buf++;
            words--;
        }
    }

    while (!__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_DATAEND))
    {
        if (__SDMMC_GET_FLAG(SDMMC1,
            SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT))
            return -2;
    }

    return 0;
}

static int SDIO_CMD53_Read(uint8_t fn, uint32_t addr, uint8_t *buffer, uint32_t length)
{
    SDMMC_DataInitTypeDef config;
    
    config.DataTimeOut   = SDMMC_DATATIMEOUT;
    config.DataLength    = length;
    config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
    
    /* FIX: Use 'TO_SDMMC' for reading data into the STM32 */
    config.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC; 
    
    config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
    config.DPSM          = SDMMC_DPSM_ENABLE;
    SDMMC_ConfigData(SDMMC1, &config);

    uint32_t block_count = length / 512;
    
    /* Argument: Read(0) | Fn(x) | Block Mode(1) | OpCode(1=Inc Addr) | Addr(x) | Count(x) */
    uint32_t arg = (0UL << 31) | ((fn & 0x7) << 28) | (1UL << 27) | (1UL << 26) | 
                   ((addr & 0x1FFFF) << 9) | (block_count & 0x1FF);

    SDMMC1->ARG = arg;
    SDMMC1->CMD = (53U) | SDMMC_CMD_WAITRESP_0 | SDMMC_CMD_CPSMEN;
    
    if (SDMMC_WaitCmdDone(SDMMC1, 0) != 0) return -1;

    /* Read Loop (Polling Mode) */
    uint32_t *temp_buff = (uint32_t *)buffer;
    uint32_t count = (length + 3) / 4;
    
    while (count > 0)
    {
        if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_RXFIFOHF)) { /* RX FIFO Half Full */
            *temp_buff++ = SDMMC1->FIFO;
            count--;
        }
    }
    
    return 0;
}

void print_hex_dump(uint8_t *buffer, uint32_t len) {
  LOG_INFO("RX DATA (%lu bytes):\r\n", len);
  for (uint32_t i = 0; i < len; i++) {
    printf("%02X ", buffer[i]);
    if ((i + 1) % 16 == 0) printf("\r\n");
  }
  printf("\r\n");
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
  MX_SDMMC1_DeInit();
  MX_GPIO_DeInit();
  
  /* Deinitialize HAL */
  HAL_DeInit();
  
  /* Reset SysTick */
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;
  
  /* Set MSP to bootloader's stack pointer */
  __set_MSP(*(__IO uint32_t*)BOOTLOADER_BASE_ADDRESS);

  /* Set vector table offset to bootloader */
  SCB->VTOR = BOOTLOADER_BASE_ADDRESS;

  /* Enable interrupts */
  __enable_irq();
  
  /* Jump to bootloader */
  void (*bootloader)(void) = (void (*)(void))(*((uint32_t*)(BOOTLOADER_BASE_ADDRESS + 4)));
  bootloader();
  
  /* Should never reach here */
  while(1);
}

/*******************************************************************************
 * System Configuration Functions
 ******************************************************************************/
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

/*******************************************************************************
 * Peripheral Initialization and Deinitialization Functions
 ******************************************************************************/
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
  
  /* SYSCFG Clock Enable - Required for EXTI interrupts */
  __HAL_RCC_SYSCFG_CLK_ENABLE();

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

  /*Configure GPIO pins : ESP_EN_Pin */
  GPIO_InitStruct.Pin = ESP_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ESP_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ESP_HSK_Pin */
  GPIO_InitStruct.Pin = ESP_HSK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(ESP_HSK_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init for ESP_HSK_Pin */
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
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

  /* Deconfigure GPIO pins : ESP_EN_Pin */
  HAL_GPIO_DeInit(ESP_EN_GPIO_Port, ESP_EN_Pin);

  /* Deconfigure GPIO pins : ESP_HSK_Pin */
  HAL_GPIO_DeInit(ESP_HSK_GPIO_Port, ESP_HSK_Pin);

  /* EXTI interrupt Deinit for ESP_HSK_Pin */
  HAL_NVIC_DisableIRQ(EXTI0_IRQn);
}

/**
 * @brief SDMMC1 Initialization for ESP32 SDIO (ESP-Hosted)
 * @note This is NOT for SD cards - it's for ESP32 WiFi co-processor
 * @retval None
 */
static void MX_SDMMC1_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  
  LOG_INFO("Initializing SDMMC1 for ESP32 SDIO...\r\n");

  /* Configure SDMMC1 peripheral clock source */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC;
  PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Enable SDMMC1 peripheral clock */
  __HAL_RCC_SDMMC1_CLK_ENABLE();

  /* GPIO clocks already enabled in MX_GPIO_Init() */

  /* Configure SDMMC1 GPIO pins for ESP32 SDIO communication */
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;

  /* Configure D0-D3 and CLK pins (Port C) */
  GPIO_InitStruct.Pin = SDMMC1_D0_Pin | SDMMC1_D1_Pin | SDMMC1_D2_Pin | 
                        SDMMC1_D3_Pin | SDMMC1_CLK_Pin;
  HAL_GPIO_Init(SDMMC1_Data_GPIO_Port, &GPIO_InitStruct);

  /* Configure CMD pin (Port D) */
  GPIO_InitStruct.Pin = SDMMC1_CMD_Pin;
  HAL_GPIO_Init(SDMMC1_CMD_GPIO_Port, &GPIO_InitStruct);

  /* Power ON the SDMMC peripheral */
  SDMMC1->POWER = SDMMC_POWER_PWRCTRL;  // 0x03 = power on

  /* Small delay for power to stabilize */
  for (volatile int i = 0; i < 10000; i++);

  /* Clear all interrupt flags */
  SDMMC1->ICR = 0xFFFFFFFFU;

  /*
   * Configure CLKCR register:
   * - CLKDIV = 120 (~1-2 MHz for initial handshake)
   * - WIDBUS = 0 (1-bit mode initially)
   * - No power save, no hardware flow control
   * 
   * Note: STM32H7 SDMMC doesn't have CLKEN bit - clock is auto-enabled when powered
   */
  SDMMC1->CLKCR = (120U << SDMMC_CLKCR_CLKDIV_Pos);  // Just set clock divider

  LOG_INFO("SDMMC1 basic init done (1-bit, ~1.6 MHz clock)\r\n");
}

/**
 * @brief SDMMC1 Deinitialization for ESP32 SDIO (ESP-Hosted)
 * @note This powers down SDIO peripheral, NOT SD card
 * @param None
 * @retval None
 */
static void MX_SDMMC1_DeInit(void)
{
  /* Disable all SDMMC interrupts */
  SDMMC1->MASK = 0;
  
  /* Clear all flags */
  SDMMC1->ICR = 0xFFFFFFFFU;
  
  /* Power OFF the SDMMC peripheral */
  SDMMC1->POWER = 0x00;  // Power off
  
  /* De-initialize GPIO pins */
  HAL_GPIO_DeInit(SDMMC1_Data_GPIO_Port, SDMMC1_D0_Pin | SDMMC1_D1_Pin | 
                  SDMMC1_D2_Pin | SDMMC1_D3_Pin | SDMMC1_CLK_Pin);
  HAL_GPIO_DeInit(SDMMC1_CMD_GPIO_Port, SDMMC1_CMD_Pin);
  
  /* Disable SDMMC1 peripheral clock */
  __HAL_RCC_SDMMC1_CLK_DISABLE();
  
  LOG_INFO("SDIO peripheral deinitialized\r\n");
}

/*******************************************************************************
 * C Library printf Redirection
 ******************************************************************************/

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

/*******************************************************************************
 * Error Handler
 ******************************************************************************/
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

/*******************************************************************************
 * FreeRTOS Callbacks and Hooks
 ******************************************************************************/

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

/*******************************************************************************
 * ESP Interrupt Callbacks and Hooks
 ******************************************************************************/

/**
 * @brief  EXTI line detection callbacks
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
 void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == ESP_HSK_Pin)
  {
    is_interrupt_called = 1;
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
  }
}