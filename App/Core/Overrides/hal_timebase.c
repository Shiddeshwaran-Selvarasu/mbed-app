#include "stm32h7xx_hal.h"

TIM_HandleTypeDef htim6;

/**
  * @brief  Initializes the HAL Tick priority.
  * @param  TickPriority: Tick interrupt priority.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  RCC_ClkInitTypeDef    clkconfig;
  uint32_t              uwTimclock;
  uint32_t              uwPrescalerValue = 0;
  uint32_t              pFLatency;

  /* * IMPORTANT: This interrupt must be set to the LOWEST priority.
   * This is critical for RTOS operation.
   */
  HAL_NVIC_SetPriority(TIM6_DAC_IRQn, TICK_INT_PRIORITY, 0U);
  
  /* Enable the TIM6 clock */
  __HAL_RCC_TIM6_CLK_ENABLE();

  /* Get clock configuration */
  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

  /* * Compute TIM6 clock.
   * On H7, the basic timers (TIM6, TIM7) are clocked from APB1.
   * If APB1 prescaler (D2PPRE1) is > 1, the timer clock is 2 * PCLK1.
   * Otherwise, it's PCLK1.
   */
  if (clkconfig.APB1CLKDivider == RCC_D2CFGR_D2PPRE1_DIV1) 
  {
    uwTimclock = HAL_RCC_GetPCLK1Freq();
  }
  else
  {
    uwTimclock = 2 * HAL_RCC_GetPCLK1Freq();
  }

  /* Compute the prescaler value to have TIM6 counter clock = 1MHz (1us tick) */
  uwPrescalerValue = (uint32_t)((uwTimclock / 1000000U) - 1U);

  /* Initialize TIM6 */
  htim6.Instance = TIM6;
  
  /* * Init timer parameters 
   * Prescaler = (TIM6CLK / 1MHz) - 1
   * Period = (1MHz / 1000Hz) - 1 = 1000 - 1 = 999
   * This gives an update event every 1000 * 1us = 1ms
   */
  htim6.Init.Prescaler = uwPrescalerValue;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = (1000000U / 1000U) - 1U;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    return HAL_ERROR;
  }

  /* Start the timer in interrupt mode */
  if (HAL_TIM_Base_Start_IT(&htim6) != HAL_OK)
  {
    return HAL_ERROR;
  }

  /* Enable the TIM6 interrupt */
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);

  return HAL_OK;
}

/**
  * @brief  Suspend Tick increment.
  * @note   Disable the tick interrupt.
  */
void HAL_SuspendTick(void)
{
  /* Disable TIM6 update interrupt */
  __HAL_TIM_DISABLE_IT(&htim6, TIM_IT_UPDATE);
}

/**
  * @brief  Resume Tick increment.
  * @note   Enable the tick interrupt.
  */
void HAL_ResumeTick(void)
{
  /* Enable TIM6 update interrupt */
  __HAL_TIM_ENABLE_IT(&htim6, TIM_IT_UPDATE);
}