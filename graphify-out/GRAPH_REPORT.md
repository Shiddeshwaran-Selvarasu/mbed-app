# Graph Report - mbed-app  (2026-05-03)

## Corpus Check
- 1084 files · ~7,042,064 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 11111 nodes · 19154 edges · 94 communities detected
- Extraction: 85% EXTRACTED · 15% INFERRED · 0% AMBIGUOUS · INFERRED: 2879 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- [[_COMMUNITY_Community 0|Community 0]]
- [[_COMMUNITY_Community 1|Community 1]]
- [[_COMMUNITY_Community 2|Community 2]]
- [[_COMMUNITY_Community 3|Community 3]]
- [[_COMMUNITY_Community 4|Community 4]]
- [[_COMMUNITY_Community 5|Community 5]]
- [[_COMMUNITY_Community 6|Community 6]]
- [[_COMMUNITY_Community 8|Community 8]]
- [[_COMMUNITY_Community 9|Community 9]]
- [[_COMMUNITY_Community 10|Community 10]]
- [[_COMMUNITY_Community 11|Community 11]]
- [[_COMMUNITY_Community 12|Community 12]]
- [[_COMMUNITY_Community 13|Community 13]]
- [[_COMMUNITY_Community 14|Community 14]]
- [[_COMMUNITY_Community 15|Community 15]]
- [[_COMMUNITY_Community 16|Community 16]]
- [[_COMMUNITY_Community 17|Community 17]]
- [[_COMMUNITY_Community 18|Community 18]]
- [[_COMMUNITY_Community 19|Community 19]]
- [[_COMMUNITY_Community 20|Community 20]]
- [[_COMMUNITY_Community 21|Community 21]]
- [[_COMMUNITY_Community 23|Community 23]]
- [[_COMMUNITY_Community 24|Community 24]]
- [[_COMMUNITY_Community 26|Community 26]]
- [[_COMMUNITY_Community 27|Community 27]]
- [[_COMMUNITY_Community 28|Community 28]]
- [[_COMMUNITY_Community 29|Community 29]]
- [[_COMMUNITY_Community 30|Community 30]]
- [[_COMMUNITY_Community 31|Community 31]]
- [[_COMMUNITY_Community 32|Community 32]]
- [[_COMMUNITY_Community 33|Community 33]]
- [[_COMMUNITY_Community 34|Community 34]]
- [[_COMMUNITY_Community 35|Community 35]]
- [[_COMMUNITY_Community 36|Community 36]]
- [[_COMMUNITY_Community 38|Community 38]]
- [[_COMMUNITY_Community 39|Community 39]]
- [[_COMMUNITY_Community 40|Community 40]]
- [[_COMMUNITY_Community 41|Community 41]]
- [[_COMMUNITY_Community 42|Community 42]]
- [[_COMMUNITY_Community 44|Community 44]]
- [[_COMMUNITY_Community 45|Community 45]]
- [[_COMMUNITY_Community 46|Community 46]]
- [[_COMMUNITY_Community 48|Community 48]]
- [[_COMMUNITY_Community 49|Community 49]]
- [[_COMMUNITY_Community 50|Community 50]]
- [[_COMMUNITY_Community 51|Community 51]]
- [[_COMMUNITY_Community 52|Community 52]]
- [[_COMMUNITY_Community 53|Community 53]]
- [[_COMMUNITY_Community 54|Community 54]]
- [[_COMMUNITY_Community 55|Community 55]]
- [[_COMMUNITY_Community 57|Community 57]]
- [[_COMMUNITY_Community 58|Community 58]]
- [[_COMMUNITY_Community 59|Community 59]]
- [[_COMMUNITY_Community 60|Community 60]]
- [[_COMMUNITY_Community 61|Community 61]]
- [[_COMMUNITY_Community 62|Community 62]]
- [[_COMMUNITY_Community 64|Community 64]]
- [[_COMMUNITY_Community 65|Community 65]]
- [[_COMMUNITY_Community 66|Community 66]]
- [[_COMMUNITY_Community 69|Community 69]]
- [[_COMMUNITY_Community 71|Community 71]]
- [[_COMMUNITY_Community 75|Community 75]]
- [[_COMMUNITY_Community 76|Community 76]]
- [[_COMMUNITY_Community 77|Community 77]]
- [[_COMMUNITY_Community 78|Community 78]]
- [[_COMMUNITY_Community 80|Community 80]]
- [[_COMMUNITY_Community 81|Community 81]]
- [[_COMMUNITY_Community 82|Community 82]]
- [[_COMMUNITY_Community 83|Community 83]]
- [[_COMMUNITY_Community 84|Community 84]]
- [[_COMMUNITY_Community 86|Community 86]]
- [[_COMMUNITY_Community 87|Community 87]]
- [[_COMMUNITY_Community 91|Community 91]]
- [[_COMMUNITY_Community 92|Community 92]]
- [[_COMMUNITY_Community 94|Community 94]]
- [[_COMMUNITY_Community 96|Community 96]]
- [[_COMMUNITY_Community 97|Community 97]]
- [[_COMMUNITY_Community 98|Community 98]]
- [[_COMMUNITY_Community 101|Community 101]]
- [[_COMMUNITY_Community 102|Community 102]]
- [[_COMMUNITY_Community 103|Community 103]]
- [[_COMMUNITY_Community 104|Community 104]]
- [[_COMMUNITY_Community 105|Community 105]]
- [[_COMMUNITY_Community 108|Community 108]]
- [[_COMMUNITY_Community 110|Community 110]]
- [[_COMMUNITY_Community 112|Community 112]]
- [[_COMMUNITY_Community 115|Community 115]]
- [[_COMMUNITY_Community 116|Community 116]]
- [[_COMMUNITY_Community 122|Community 122]]
- [[_COMMUNITY_Community 130|Community 130]]
- [[_COMMUNITY_Community 133|Community 133]]
- [[_COMMUNITY_Community 134|Community 134]]
- [[_COMMUNITY_Community 135|Community 135]]
- [[_COMMUNITY_Community 136|Community 136]]

## God Nodes (most connected - your core abstractions)
1. `HAL_GetTick()` - 225 edges
2. `xPortStartScheduler()` - 184 edges
3. `pxPortInitialiseStack()` - 150 edges
4. `vPortEndScheduler()` - 148 edges
5. `vPortEnterCritical()` - 85 edges
6. `xPortIsAuthorizedToAccessKernelObject()` - 84 edges
7. `vPortExitCritical()` - 82 edges
8. `HAL_DMA_Start_IT()` - 73 edges
9. `USBD_CoreGetEPAdd()` - 66 edges
10. `prvSetupTimerInterrupt()` - 65 edges

## Surprising Connections (you probably didn't know these)
- `OTG_FS_IRQHandler()` --calls--> `HAL_PCD_IRQHandler()`  [INFERRED]
  Bootloader/Core/Src/stm32h7xx_it.c → Common/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd.c
- `USBD_LL_Delay()` --calls--> `HAL_Delay()`  [INFERRED]
  Bootloader/USB_Device/Target/usbd_conf.c → Common/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c
- `CCID_Response_SendData()` --calls--> `USBD_LL_Transmit()`  [INFERRED]
  Common/Middlewares/ST/STM32_USB_Device_Library/Class/CCID/Src/usbd_ccid_if_template.c → Bootloader/USB_Device/Target/usbd_conf.c
- `test_logger_with_error_level()` --calls--> `logger()`  [INFERRED]
  tests/test_logger.c → Common/Core/Src/logger.c
- `test_logger_with_info_level()` --calls--> `logger()`  [INFERRED]
  tests/test_logger.c → Common/Core/Src/logger.c

## Communities

### Community 0 - "Community 0"
Cohesion: 0.01
Nodes (477): prvInitialiseCoRoutineLists(), vCoRoutineAddToDelayedList(), xCoRoutineCreate(), xCoRoutineRemoveFromEventList(), prvTestWaitCondition(), uxEventGroupGetNumber(), vEventGroupClearBitsCallback(), vEventGroupDelete() (+469 more)

### Community 1 - "Community 1"
Cohesion: 0.01
Nodes (274): HAL_GetCurrentCPUID(), DMA2D_SetConfig(), HAL_DMA2D_Abort(), HAL_DMA2D_BlendingStart(), HAL_DMA2D_BlendingStart_IT(), HAL_DMA2D_CLUTLoading_Abort(), HAL_DMA2D_CLUTLoading_Suspend(), HAL_DMA2D_CLUTLoadingCpltCallback() (+266 more)

### Community 2 - "Community 2"
Cohesion: 0.02
Nodes (257): prvCheckDelayedList(), prvCheckPendingReadyList(), vCoRoutineSchedule(), MPU_xTaskGetCurrentTaskHandle(), MPU_vTaskGetInfoImpl(), MPU_xEventGroupWaitBitsImpl(), MPU_xStreamBufferReceiveImpl(), MPU_xStreamBufferSendImpl() (+249 more)

### Community 3 - "Community 3"
Cohesion: 0.01
Nodes (306): MX_USB_CDC_DeInit(), HAL_PCD_EP_GetRxCount(), setUp(), AUDIO_REQ_GetCurrent(), AUDIO_REQ_SetCurrent(), USBD_AUDIO_DataOut(), USBD_AUDIO_DeInit(), USBD_AUDIO_GetAudioHeaderDesc() (+298 more)

### Community 4 - "Community 4"
Cohesion: 0.01
Nodes (223): __set_MSP(), config_get(), config_load_defaults(), NVIC_DecodePriority(), NVIC_EncodePriority(), SysTick_Config(), SCB_CleanInvalidateDCache(), SCB_DisableDCache() (+215 more)

### Community 5 - "Community 5"
Cohesion: 0.01
Nodes (137): LL_RCC_CalcPLLClockFreq(), LL_RCC_CSI_IsReady(), LL_RCC_DeInit(), LL_RCC_GetADCClockFreq(), LL_RCC_GetADCClockSource(), LL_RCC_GetAHBPrescaler(), LL_RCC_GetAPB1Prescaler(), LL_RCC_GetAPB2Prescaler() (+129 more)

### Community 6 - "Community 6"
Cohesion: 0.01
Nodes (34): LL_ADC_CommonDeInit(), LL_AHB1_GRP1_ForceReset(), LL_AHB1_GRP1_ReleaseReset(), LL_AHB2_GRP1_ForceReset(), LL_AHB2_GRP1_ReleaseReset(), LL_AHB4_GRP1_ForceReset(), LL_AHB4_GRP1_ReleaseReset(), LL_APB1_GRP2_ForceReset() (+26 more)

### Community 8 - "Community 8"
Cohesion: 0.01
Nodes (280): HAL_DMA_Abort_IT(), HAL_DMA_Start_IT(), FMAC_AppendFilterDataUpdateState(), FMAC_ConfigFilterOutputBufferUpdateState(), FMAC_DMAError(), FMAC_DMAFilterConfig(), FMAC_DMAFilterPreload(), FMAC_DMAGetData() (+272 more)

### Community 9 - "Community 9"
Cohesion: 0.01
Nodes (108): ADC_ConfigureBoostMode(), ADC_ConversionStop(), ADC_Disable(), ADC_DMAConvCplt(), ADC_DMAError(), ADC_DMAHalfConvCplt(), ADC_Enable(), HAL_ADCEx_Calibration_GetValue() (+100 more)

### Community 10 - "Community 10"
Cohesion: 0.02
Nodes (185): __CLZ(), __RBIT(), HAL_MMCEx_ReadBlocksDMAMultiBuffer(), HAL_MMCEx_WriteBlocksDMAMultiBuffer(), HAL_MMC_Abort(), HAL_MMC_Abort_IT(), HAL_MMC_AbortCallback(), HAL_MMC_AwakeDevice() (+177 more)

### Community 11 - "Community 11"
Cohesion: 0.01
Nodes (27): __get_PRIMASK(), __set_PRIMASK(), LPTIM_Disable(), LL_APB1_GRP1_ForceReset(), LL_APB1_GRP1_ReleaseReset(), LL_APB2_GRP1_ForceReset(), LL_APB2_GRP1_ReleaseReset(), LL_APB4_GRP1_ForceReset() (+19 more)

### Community 12 - "Community 12"
Cohesion: 0.01
Nodes (44): LL_SYSTICK_IsActiveCounterFlag(), LL_RTC_ALMA_ConfigTime(), LL_RTC_ALMA_DisableWeekday(), LL_RTC_ALMA_EnableWeekday(), LL_RTC_ALMA_GetHour(), LL_RTC_ALMA_GetMinute(), LL_RTC_ALMA_GetSecond(), LL_RTC_ALMA_GetTime() (+36 more)

### Community 13 - "Community 13"
Cohesion: 0.01
Nodes (7): LL_USART_ClockInit(), LL_USART_Init(), LL_USART_IsEnabled(), LL_USART_SetBaudRate(), LL_USART_SetHWFlowCtrl(), LL_USART_SetPrescaler(), LL_USART_SetStopBitsLength()

### Community 14 - "Community 14"
Cohesion: 0.01
Nodes (120): AT91F_ADC_CfgPIO(), AT91F_ADC_CfgPMC(), AT91F_ADC_GetInterruptMaskStatus(), AT91F_ADC_GetStatus(), AT91F_ADC_IsInterruptMasked(), AT91F_ADC_IsStatusSet(), AT91F_AIC_CfgPIO(), AT91F_AIC_CfgPMC() (+112 more)

### Community 15 - "Community 15"
Cohesion: 0.01
Nodes (162): DAC_DMAConvCpltCh1(), DAC_DMAErrorCh1(), DAC_DMAHalfConvCpltCh1(), DAC_DMAConvCpltCh2(), DAC_DMAErrorCh2(), DAC_DMAHalfConvCpltCh2(), HAL_DACEx_ConvCpltCallbackCh2(), HAL_DACEx_ConvHalfCpltCallbackCh2() (+154 more)

### Community 16 - "Community 16"
Cohesion: 0.01
Nodes (26): IC1Config(), IC2Config(), IC3Config(), IC4Config(), LL_TIM_ENCODER_Init(), LL_TIM_GenerateEvent_UPDATE(), LL_TIM_HALLSENSOR_Init(), LL_TIM_IC_Init() (+18 more)

### Community 17 - "Community 17"
Cohesion: 0.01
Nodes (100): LL_I2S_ClearFlag_FRE(), LL_I2S_ClearFlag_OVR(), LL_I2S_ClearFlag_UDR(), LL_I2S_DisableDMAReq_RX(), LL_I2S_DisableDMAReq_TX(), LL_I2S_DisableGPIOControl(), LL_I2S_DisableIOSwap(), LL_I2S_DisableIT_FRE() (+92 more)

### Community 18 - "Community 18"
Cohesion: 0.02
Nodes (115): HAL_JPEG_ConfigEncoding(), HAL_JPEG_DataReadyCallback(), HAL_JPEG_Decode(), HAL_JPEG_Decode_DMA(), HAL_JPEG_Decode_IT(), HAL_JPEG_DecodeCpltCallback(), HAL_JPEG_DeInit(), HAL_JPEG_Encode() (+107 more)

### Community 19 - "Community 19"
Cohesion: 0.01
Nodes (6): LL_LPUART_Init(), LL_LPUART_IsEnabled(), LL_LPUART_SetBaudRate(), LL_LPUART_SetHWFlowCtrl(), LL_LPUART_SetPrescaler(), LL_LPUART_SetStopBitsLength()

### Community 20 - "Community 20"
Cohesion: 0.02
Nodes (118): HAL_HCD_Connect_Callback(), HAL_HCD_DeInit(), HAL_HCD_Disconnect_Callback(), HAL_HCD_GetCurrentFrame(), HAL_HCD_GetCurrentSpeed(), HAL_HCD_HC_ClearHubInfo(), HAL_HCD_HC_Halt(), HAL_HCD_HC_Init() (+110 more)

### Community 21 - "Community 21"
Cohesion: 0.01
Nodes (8): LL_PWR_GetRegulVoltageScaling(), LL_FLASH_GetLatency(), LL_FLASH_SetLatency(), LL_SYSCFG_GetPackage(), if(), LL_GetPackageType(), LL_Init1msTick(), LL_InitTick()

### Community 23 - "Community 23"
Cohesion: 0.02
Nodes (18): LL_DMA2D_BGND_IsEnabledCLUTLoad(), LL_DMA2D_BGND_SetCLUTMemAddr(), LL_DMA2D_BGND_SetColor(), LL_DMA2D_BGND_SetLineOffset(), LL_DMA2D_BGND_SetMemAddr(), LL_DMA2D_ConfigLayer(), LL_DMA2D_ConfigOutputColor(), LL_DMA2D_ConfigSize() (+10 more)

### Community 24 - "Community 24"
Cohesion: 0.02
Nodes (10): LL_I2C_AcknowledgeNextData(), LL_I2C_ConfigFilters(), LL_I2C_Disable(), LL_I2C_DisableOwnAddress1(), LL_I2C_Enable(), LL_I2C_EnableOwnAddress1(), LL_I2C_Init(), LL_I2C_SetMode() (+2 more)

### Community 26 - "Community 26"
Cohesion: 0.03
Nodes (70): GetTimerIdxFromDMAHandle(), HAL_HRTIM_BurstDMATransfer(), HAL_HRTIM_BurstDMATransferCallback(), HAL_HRTIM_BurstModePeriodCallback(), HAL_HRTIM_Capture1EventCallback(), HAL_HRTIM_Capture2EventCallback(), HAL_HRTIM_Compare1EventCallback(), HAL_HRTIM_Compare2EventCallback() (+62 more)

### Community 27 - "Community 27"
Cohesion: 0.02
Nodes (30): FDCAN_CalcultateRamBlockAddresses(), FDCAN_CopyMessageToRAM(), HAL_FDCAN_AddMessageToTxBuffer(), HAL_FDCAN_AddMessageToTxFifoQ(), HAL_FDCAN_ClockCalibrationCallback(), HAL_FDCAN_DeInit(), HAL_FDCAN_EnterPowerDownMode(), HAL_FDCAN_ErrorCallback() (+22 more)

### Community 28 - "Community 28"
Cohesion: 0.02
Nodes (19): LL_AHB3_GRP1_ForceReset(), LL_AHB3_GRP1_ReleaseReset(), LL_DMA2D_DeInit(), LL_MDMA_ConfigBlkCounters(), LL_MDMA_ConfigBlkRepeatAddrUpdate(), LL_MDMA_ConfigBlkRptAddrUpdateValue(), LL_MDMA_ConfigBusSelection(), LL_MDMA_ConfigTransfer() (+11 more)

### Community 29 - "Community 29"
Cohesion: 0.04
Nodes (116): AT91F_AIC_AcknowledgeIt(), AT91F_AIC_ClearIt(), AT91F_AIC_ConfigureIt(), AT91F_AIC_DisableIt(), AT91F_AIC_EnableIt(), AT91F_AIC_IsActive(), AT91F_AIC_IsPending(), AT91F_AIC_Open() (+108 more)

### Community 30 - "Community 30"
Cohesion: 0.02
Nodes (50): AT91F_AIC_ConfigureIt(), AT91F_AIC_DisableIt(), AT91F_AIC_Open(), AT91F_AIC_SetExceptionVector(), AT91F_CKGR_GetMainClock(), AT91F_DBGU_GetInterruptMaskStatus(), AT91F_DBGU_IsInterruptMasked(), AT91F_PDC_Close() (+42 more)

### Community 31 - "Community 31"
Cohesion: 0.02
Nodes (15): LL_BDMA_ClearFlag_GI0(), LL_BDMA_ClearFlag_GI1(), LL_BDMA_ClearFlag_GI2(), LL_BDMA_ClearFlag_GI3(), LL_BDMA_ClearFlag_GI4(), LL_BDMA_ClearFlag_GI5(), LL_BDMA_ClearFlag_GI6(), LL_BDMA_ClearFlag_GI7() (+7 more)

### Community 32 - "Community 32"
Cohesion: 0.04
Nodes (78): HAL_DTS_AsyncEndCallback(), HAL_DTS_AsyncHighCallback(), HAL_DTS_AsyncLowCallback(), HAL_DTS_DeInit(), HAL_DTS_EndCallback(), HAL_DTS_GetTemperature(), HAL_DTS_HighCallback(), HAL_DTS_Init() (+70 more)

### Community 33 - "Community 33"
Cohesion: 0.03
Nodes (37): LL_C2_EXTI_DisableEvent_0_31(), LL_C2_EXTI_DisableEvent_32_63(), LL_C2_EXTI_DisableEvent_64_95(), LL_C2_EXTI_DisableIT_0_31(), LL_C2_EXTI_DisableIT_32_63(), LL_C2_EXTI_DisableIT_64_95(), LL_C2_EXTI_EnableEvent_0_31(), LL_C2_EXTI_EnableEvent_32_63() (+29 more)

### Community 34 - "Community 34"
Cohesion: 0.04
Nodes (82): HAL_HASHEx_SHA224_Accmlt(), HAL_HASHEx_SHA224_Accmlt_End(), HAL_HASHEx_SHA224_Accmlt_End_IT(), HAL_HASHEx_SHA224_Accmlt_IT(), HAL_HASHEx_SHA224_Finish(), HAL_HASHEx_SHA224_Start(), HAL_HASHEx_SHA224_Start_DMA(), HAL_HASHEx_SHA224_Start_IT() (+74 more)

### Community 35 - "Community 35"
Cohesion: 0.31
Nodes (70): MPU_eTaskGetState(), MPU_pcQueueGetName(), MPU_pcTimerGetName(), MPU_pvTaskGetThreadLocalStoragePointer(), MPU_pvTimerGetTimerID(), MPU_ulTaskGenericNotifyTake(), MPU_ulTaskGenericNotifyValueClear(), MPU_ulTaskGetIdleRunTimeCounter() (+62 more)

### Community 36 - "Community 36"
Cohesion: 0.03
Nodes (4): LL_SWPMI_Init(), LL_SWPMI_IsActivated(), LL_SWPMI_SetBitRatePrescaler(), LL_SWPMI_SetVoltageClass()

### Community 38 - "Community 38"
Cohesion: 0.04
Nodes (53): DFSDM_DMAError(), DFSDM_DMAInjectedConvCplt(), DFSDM_DMAInjectedHalfConvCplt(), DFSDM_DMARegularConvCplt(), DFSDM_DMARegularHalfConvCplt(), DFSDM_GetChannelFromInstance(), DFSDM_GetInjChannelsNbr(), DFSDM_InjConvStart() (+45 more)

### Community 39 - "Community 39"
Cohesion: 0.08
Nodes (63): logger(), tearDown(), test_logger_with_debug_level(), test_logger_with_error_level(), test_logger_with_formatted_string(), test_logger_with_info_level(), test_logger_with_invalid_level_above_range(), test_logger_with_invalid_level_below_range() (+55 more)

### Community 40 - "Community 40"
Cohesion: 0.04
Nodes (37): config_save(), erase_flash(), write_flash(), FLASH_CRC_AddSector(), FLASH_CRC_SelectAddress(), FLASH_Erase_Sector(), FLASH_MassErase(), FLASH_OB_CPUFreq_GetBoost() (+29 more)

### Community 41 - "Community 41"
Cohesion: 0.03
Nodes (3): LL_FMAC_EnableReset(), LL_FMAC_Init(), LL_FMAC_IsEnabledReset()

### Community 42 - "Community 42"
Cohesion: 0.07
Nodes (54): CCID_CheckCommandParams(), CCID_CmdAbort(), CCID_UpdateCommandStatus(), PC_to_RDR_Abort(), PC_to_RDR_Escape(), PC_to_RDR_GetParameters(), PC_to_RDR_GetSlotStatus(), PC_to_RDR_IccClock() (+46 more)

### Community 44 - "Community 44"
Cohesion: 0.06
Nodes (40): HAL_SMARTCARDEx_EnableFifoMode(), HAL_SMARTCARDEx_RxFifoFullCallback(), HAL_SMARTCARDEx_SetRxFifoThreshold(), HAL_SMARTCARDEx_SetTxFifoThreshold(), HAL_SMARTCARDEx_TxFifoEmptyCallback(), SMARTCARDEx_SetNbDataToProcess(), HAL_SMARTCARD_Abort_IT(), HAL_SMARTCARD_AbortCpltCallback() (+32 more)

### Community 45 - "Community 45"
Cohesion: 0.1
Nodes (44): __ROR(), CRYP_AES_Decrypt(), CRYP_AES_Decrypt_DMA(), CRYP_AES_Decrypt_IT(), CRYP_AES_Encrypt(), CRYP_AES_Encrypt_IT(), CRYP_AES_IT(), CRYP_AES_ProcessData() (+36 more)

### Community 46 - "Community 46"
Cohesion: 0.08
Nodes (36): HAL_SAI_Abort(), HAL_SAI_DeInit(), HAL_SAI_DMAStop(), HAL_SAI_ErrorCallback(), HAL_SAI_Init(), HAL_SAI_InitProtocol(), HAL_SAI_IRQHandler(), HAL_SAI_MspDeInit() (+28 more)

### Community 48 - "Community 48"
Cohesion: 0.09
Nodes (39): HAL_LPTIM_AutoReloadMatchCallback(), HAL_LPTIM_AutoReloadWriteCallback(), HAL_LPTIM_CompareMatchCallback(), HAL_LPTIM_CompareWriteCallback(), HAL_LPTIM_Counter_Start(), HAL_LPTIM_Counter_Start_IT(), HAL_LPTIM_Counter_Stop(), HAL_LPTIM_Counter_Stop_IT() (+31 more)

### Community 49 - "Community 49"
Cohesion: 0.07
Nodes (21): HAL_LTDC_ConfigLayer(), HAL_LTDC_ConfigLayer_NoReload(), HAL_LTDC_DeInit(), HAL_LTDC_ErrorCallback(), HAL_LTDC_Init(), HAL_LTDC_IRQHandler(), HAL_LTDC_LineEventCallback(), HAL_LTDC_MspDeInit() (+13 more)

### Community 50 - "Community 50"
Cohesion: 0.1
Nodes (32): HAL_SMBUS_AddrCallback(), HAL_SMBUS_DeInit(), HAL_SMBUS_DisableAlert_IT(), HAL_SMBUS_DisableListen_IT(), HAL_SMBUS_EnableAlert_IT(), HAL_SMBUS_EnableListen_IT(), HAL_SMBUS_ER_IRQHandler(), HAL_SMBUS_ErrorCallback() (+24 more)

### Community 51 - "Community 51"
Cohesion: 0.05
Nodes (2): TZ_NVIC_SetPriority_NS(), TZ_SysTick_Config_NS()

### Community 52 - "Community 52"
Cohesion: 0.05
Nodes (2): TZ_NVIC_SetPriority_NS(), TZ_SysTick_Config_NS()

### Community 53 - "Community 53"
Cohesion: 0.05
Nodes (2): TZ_NVIC_SetPriority_NS(), TZ_SysTick_Config_NS()

### Community 54 - "Community 54"
Cohesion: 0.05
Nodes (2): TZ_NVIC_SetPriority_NS(), TZ_SysTick_Config_NS()

### Community 55 - "Community 55"
Cohesion: 0.09
Nodes (23): CalcCRC(), delay(), etx_receive_data(), etx_receive_response(), etx_rx_data(), etx_rx_response(), etx_send_data(), etx_send_end_cmd() (+15 more)

### Community 57 - "Community 57"
Cohesion: 0.07
Nodes (17): DCMI_DMAError(), DCMI_DMAXferCplt(), HAL_DCMI_DeInit(), HAL_DCMI_ErrorCallback(), HAL_DCMI_FrameEventCallback(), HAL_DCMI_Init(), HAL_DCMI_IRQHandler(), HAL_DCMI_LineEventCallback() (+9 more)

### Community 58 - "Community 58"
Cohesion: 0.06
Nodes (3): LL_COMP_DeInit(), LL_COMP_Init(), LL_COMP_IsLocked()

### Community 59 - "Community 59"
Cohesion: 0.06
Nodes (2): TZ_NVIC_SetPriority_NS(), TZ_SysTick_Config_NS()

### Community 60 - "Community 60"
Cohesion: 0.06
Nodes (2): TZ_NVIC_SetPriority_NS(), TZ_SysTick_Config_NS()

### Community 61 - "Community 61"
Cohesion: 0.08
Nodes (17): esp_startup_start_app_common(), main_task(), esp_startup_start_app(), SysTickIsrHandler(), vPortSetupTimer(), xPortSysTickHandler(), uxPortCompareSetExtram(), vPortAssertIfInISR() (+9 more)

### Community 62 - "Community 62"
Cohesion: 0.09
Nodes (22): HAL_SPDIFRX_CxCpltCallback(), HAL_SPDIFRX_CxHalfCpltCallback(), HAL_SPDIFRX_DeInit(), HAL_SPDIFRX_ErrorCallback(), HAL_SPDIFRX_Init(), HAL_SPDIFRX_IRQHandler(), HAL_SPDIFRX_MspDeInit(), HAL_SPDIFRX_MspInit() (+14 more)

### Community 64 - "Community 64"
Cohesion: 0.12
Nodes (20): USBD_AUDIO_GetEpPcktSze(), USBD_CMPSIT_AddClass(), USBD_CMPSIT_AddConfDesc(), USBD_CMPSIT_AddToConfDesc(), USBD_CMPSIT_AssignEp(), USBD_CMPSIT_AUDIODesc(), USBD_CMPSIT_CCIDDesc(), USBD_CMPSIT_CDC_ECMDesc() (+12 more)

### Community 65 - "Community 65"
Cohesion: 0.27
Nodes (9): SecureContext_LoadContextAsm(), SecureContext_SaveContextAsm(), SecureContext_AllocateContext(), SecureContext_FreeContext(), SecureContext_Init(), SecureContext_LoadContext(), SecureContext_SaveContext(), ulGetSecureContext() (+1 more)

### Community 66 - "Community 66"
Cohesion: 0.44
Nodes (11): PendSV_Handler(), SVC_Handler(), ulSetInterruptMask(), vClearInterruptMask(), vPortAllocateSecureContext(), vPortFreeSecureContext(), vRaisePrivilege(), vResetPrivilege() (+3 more)

### Community 69 - "Community 69"
Cohesion: 0.18
Nodes (6): ucPortCountLeadingZeros(), ulPortRaiseBASEPRI(), vPortClearBASEPRIFromISR(), vPortRaiseBASEPRI(), vPortSetBASEPRI(), xPortIsInsideInterrupt()

### Community 71 - "Community 71"
Cohesion: 0.09
Nodes (1): ulPortCountLeadingZeros()

### Community 75 - "Community 75"
Cohesion: 0.17
Nodes (20): HAL_ETHEx_DisableARPOffload(), HAL_ETHEx_DisableL3L4Filtering(), HAL_ETHEx_DisableVLANProcessing(), HAL_ETHEx_EnableARPOffload(), HAL_ETHEx_EnableL3L4Filtering(), HAL_ETHEx_EnableVLANProcessing(), HAL_ETHEx_EnterLPIMode(), HAL_ETHEx_ExitLPIMode() (+12 more)

### Community 76 - "Community 76"
Cohesion: 0.13
Nodes (12): HAL_MDIOS_DeInit(), HAL_MDIOS_ErrorCallback(), HAL_MDIOS_GetReadRegAddress(), HAL_MDIOS_GetWrittenRegAddress(), HAL_MDIOS_Init(), HAL_MDIOS_IRQHandler(), HAL_MDIOS_MspDeInit(), HAL_MDIOS_MspInit() (+4 more)

### Community 77 - "Community 77"
Cohesion: 0.14
Nodes (15): CORDIC_DMAError(), CORDIC_DMAInCplt(), CORDIC_DMAOutCplt(), CORDIC_ReadOutDataIncrementPtr(), CORDIC_WriteInDataIncrementPtr(), HAL_CORDIC_Calculate(), HAL_CORDIC_Calculate_DMA(), HAL_CORDIC_CalculateCpltCallback() (+7 more)

### Community 78 - "Community 78"
Cohesion: 0.12
Nodes (12): CPUInitialize(), DistributorInit(), DoDistributorInit(), XScuGic_CfgInitialize(), XScuGic_CpuIfInitialize(), XScuGic_Disable(), XScuGic_Enable(), XScuGic_InterruptMaptoCpu() (+4 more)

### Community 80 - "Community 80"
Cohesion: 0.11
Nodes (2): HAL_RAMECC_DetectErrorCallback(), HAL_RAMECC_IRQHandler()

### Community 81 - "Community 81"
Cohesion: 0.19
Nodes (18): _close(), _execve(), _exit(), _fork(), _fstat(), _getpid(), initialise_monitor_handles(), _isatty() (+10 more)

### Community 82 - "Community 82"
Cohesion: 0.13
Nodes (9): HAL_OTFDEC_DeInit(), HAL_OTFDEC_ErrorCallback(), HAL_OTFDEC_Init(), HAL_OTFDEC_IRQHandler(), HAL_OTFDEC_KeyCRCComputation(), HAL_OTFDEC_MspDeInit(), HAL_OTFDEC_MspInit(), HAL_OTFDEC_RegionGetKeyCRC() (+1 more)

### Community 83 - "Community 83"
Cohesion: 0.13
Nodes (8): HAL_CEC_DeInit(), HAL_CEC_ErrorCallback(), HAL_CEC_Init(), HAL_CEC_IRQHandler(), HAL_CEC_MspDeInit(), HAL_CEC_MspInit(), HAL_CEC_RxCpltCallback(), HAL_CEC_TxCpltCallback()

### Community 84 - "Community 84"
Cohesion: 0.17
Nodes (13): ARM_MPU_ClrRegion(), ARM_MPU_ClrRegion_NS(), ARM_MPU_ClrRegionEx(), ARM_MPU_Load(), ARM_MPU_Load_NS(), ARM_MPU_LoadEx(), ARM_MPU_OrderedMemcpy(), ARM_MPU_SetMemAttr() (+5 more)

### Community 86 - "Community 86"
Cohesion: 0.15
Nodes (8): HAL_COMP_DeInit(), HAL_COMP_Init(), HAL_COMP_IRQHandler(), HAL_COMP_MspDeInit(), HAL_COMP_MspInit(), HAL_COMP_Stop(), HAL_COMP_Stop_IT(), HAL_COMP_TriggerCallback()

### Community 87 - "Community 87"
Cohesion: 0.14
Nodes (6): HAL_GFXMMU_DeInit(), HAL_GFXMMU_ErrorCallback(), HAL_GFXMMU_Init(), HAL_GFXMMU_IRQHandler(), HAL_GFXMMU_MspDeInit(), HAL_GFXMMU_MspInit()

### Community 91 - "Community 91"
Cohesion: 0.22
Nodes (10): config_load_defaults(), test_config_load_defaults_clears_app_crc(), test_config_load_defaults_clears_app_size(), test_config_load_defaults_clears_reserved_fields(), test_config_load_defaults_initializes_all_critical_fields(), test_config_load_defaults_sets_app_not_bootable(), test_config_load_defaults_sets_app_not_flashed(), test_config_load_defaults_sets_reboot_reason() (+2 more)

### Community 92 - "Community 92"
Cohesion: 0.23
Nodes (11): CalcCRC(), test_CalcCRC_all_ones(), test_CalcCRC_all_zeros(), test_CalcCRC_consistency(), test_CalcCRC_different_data_different_crc(), test_CalcCRC_known_value(), test_CalcCRC_length_sensitivity(), test_CalcCRC_order_matters() (+3 more)

### Community 94 - "Community 94"
Cohesion: 0.4
Nodes (8): vNonPreemptiveTick(), vPortDisableInterruptsFromThumb(), vPortEnableInterruptsFromThumb(), vPortEnterCritical(), vPortExitCritical(), vPortISRStartFirstTask(), vPortYieldProcessor(), vPreemptiveTick()

### Community 96 - "Community 96"
Cohesion: 0.18
Nodes (5): HAL_OPAMP_DeInit(), HAL_OPAMP_Init(), HAL_OPAMP_MspDeInit(), HAL_OPAMP_MspInit(), HAL_OPAMP_SelfCalibrate()

### Community 97 - "Community 97"
Cohesion: 0.19
Nodes (4): uxDisableInterrupts(), vAssertIfInIsr(), vRestoreInterrupts(), xPortGetCoreID()

### Community 98 - "Community 98"
Cohesion: 0.28
Nodes (2): SecureInit_DePrioritizeNSExceptions(), SecureInit_EnableNSFPUAccess()

### Community 101 - "Community 101"
Cohesion: 0.2
Nodes (5): __env_lock(), __env_unlock(), __malloc_lock(), __malloc_unlock(), vPortClibInit()

### Community 102 - "Community 102"
Cohesion: 0.35
Nodes (10): add_mpu_entry_to_TCB(), find_addr_in_map(), init_private_mpu_regions(), init_task_mpu(), private_overlap_other_regions(), region_mpu_aligned(), region_overlaps_region(), region_within_region() (+2 more)

### Community 103 - "Community 103"
Cohesion: 0.36
Nodes (8): vAssertionTrap(), vContextManagementTrap(), vInstructionErrorTrap(), vInternalProtectionTrap(), vMMUTrap(), vNonMaskableInterruptTrap(), vSystemBusAndPeripheralsTrap(), vTrapInstallHandlers()

### Community 104 - "Community 104"
Cohesion: 0.29
Nodes (2): ARM_MPU_Load(), ARM_MPU_OrderedMemcpy()

### Community 105 - "Community 105"
Cohesion: 0.32
Nodes (4): HAL_WWDG_EarlyWakeupCallback(), HAL_WWDG_Init(), HAL_WWDG_IRQHandler(), HAL_WWDG_MspInit()

### Community 108 - "Community 108"
Cohesion: 0.29
Nodes (4): Get_64(), Set_64(), spin_try_lock_unsafe(), vPortRecursiveLock()

### Community 110 - "Community 110"
Cohesion: 0.48
Nodes (2): vPortMemoryBarrier(), vPortYield()

### Community 112 - "Community 112"
Cohesion: 0.6
Nodes (3): ExitRun0Mode(), SystemCoreClockUpdate(), SystemInit()

### Community 115 - "Community 115"
Cohesion: 0.6
Nodes (5): CpuUp_A53(), PowerUpIsland(), ReleaseReset_A53(), SetCoreResetVector_A53(), StartAndWaitSecondaryCpuUp()

### Community 116 - "Community 116"
Cohesion: 0.67
Nodes (5): copy_common_files_for_compiler_and_arch(), copy_files(), copy_files_in_dir(), copy_portable_files_for_compiler_and_arch(), main()

### Community 122 - "Community 122"
Cohesion: 0.6
Nodes (3): xt_overlay_init_os(), xt_overlay_lock(), xt_overlay_unlock()

### Community 130 - "Community 130"
Cohesion: 0.5
Nodes (1): PUSH()

### Community 133 - "Community 133"
Cohesion: 0.67
Nodes (1): _sbrk()

### Community 134 - "Community 134"
Cohesion: 0.67
Nodes (1): freertos_exc_init()

### Community 135 - "Community 135"
Cohesion: 0.67
Nodes (1): pxPortInitialiseStack()

### Community 136 - "Community 136"
Cohesion: 0.67
Nodes (1): sbrk()

## Knowledge Gaps
- **Thin community `Community 51`** (39 nodes): `core_armv81mml.h`, `ITM_CheckChar()`, `ITM_ReceiveChar()`, `ITM_SendChar()`, `__NVIC_ClearPendingIRQ()`, `NVIC_ClearTargetState()`, `NVIC_DecodePriority()`, `__NVIC_DisableIRQ()`, `__NVIC_EnableIRQ()`, `NVIC_EncodePriority()`, `__NVIC_GetActive()`, `__NVIC_GetEnableIRQ()`, `__NVIC_GetPendingIRQ()`, `__NVIC_GetPriority()`, `__NVIC_GetPriorityGrouping()`, `NVIC_GetTargetState()`, `__NVIC_GetVector()`, `__NVIC_SetPendingIRQ()`, `__NVIC_SetPriority()`, `__NVIC_SetPriorityGrouping()`, `NVIC_SetTargetState()`, `__NVIC_SetVector()`, `__NVIC_SystemReset()`, `SCB_GetFPUType()`, `SysTick_Config()`, `TZ_NVIC_ClearPendingIRQ_NS()`, `TZ_NVIC_DisableIRQ_NS()`, `TZ_NVIC_EnableIRQ_NS()`, `TZ_NVIC_GetActive_NS()`, `TZ_NVIC_GetEnableIRQ_NS()`, `TZ_NVIC_GetPendingIRQ_NS()`, `TZ_NVIC_GetPriority_NS()`, `TZ_NVIC_GetPriorityGrouping_NS()`, `TZ_NVIC_SetPendingIRQ_NS()`, `TZ_NVIC_SetPriority_NS()`, `TZ_NVIC_SetPriorityGrouping_NS()`, `TZ_SAU_Disable()`, `TZ_SAU_Enable()`, `TZ_SysTick_Config_NS()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 52`** (39 nodes): `core_armv8mml.h`, `ITM_CheckChar()`, `ITM_ReceiveChar()`, `ITM_SendChar()`, `__NVIC_ClearPendingIRQ()`, `NVIC_ClearTargetState()`, `NVIC_DecodePriority()`, `__NVIC_DisableIRQ()`, `__NVIC_EnableIRQ()`, `NVIC_EncodePriority()`, `__NVIC_GetActive()`, `__NVIC_GetEnableIRQ()`, `__NVIC_GetPendingIRQ()`, `__NVIC_GetPriority()`, `__NVIC_GetPriorityGrouping()`, `NVIC_GetTargetState()`, `__NVIC_GetVector()`, `__NVIC_SetPendingIRQ()`, `__NVIC_SetPriority()`, `__NVIC_SetPriorityGrouping()`, `NVIC_SetTargetState()`, `__NVIC_SetVector()`, `__NVIC_SystemReset()`, `SCB_GetFPUType()`, `SysTick_Config()`, `TZ_NVIC_ClearPendingIRQ_NS()`, `TZ_NVIC_DisableIRQ_NS()`, `TZ_NVIC_EnableIRQ_NS()`, `TZ_NVIC_GetActive_NS()`, `TZ_NVIC_GetEnableIRQ_NS()`, `TZ_NVIC_GetPendingIRQ_NS()`, `TZ_NVIC_GetPriority_NS()`, `TZ_NVIC_GetPriorityGrouping_NS()`, `TZ_NVIC_SetPendingIRQ_NS()`, `TZ_NVIC_SetPriority_NS()`, `TZ_NVIC_SetPriorityGrouping_NS()`, `TZ_SAU_Disable()`, `TZ_SAU_Enable()`, `TZ_SysTick_Config_NS()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 53`** (39 nodes): `core_cm33.h`, `ITM_CheckChar()`, `ITM_ReceiveChar()`, `ITM_SendChar()`, `__NVIC_ClearPendingIRQ()`, `NVIC_ClearTargetState()`, `NVIC_DecodePriority()`, `__NVIC_DisableIRQ()`, `__NVIC_EnableIRQ()`, `NVIC_EncodePriority()`, `__NVIC_GetActive()`, `__NVIC_GetEnableIRQ()`, `__NVIC_GetPendingIRQ()`, `__NVIC_GetPriority()`, `__NVIC_GetPriorityGrouping()`, `NVIC_GetTargetState()`, `__NVIC_GetVector()`, `__NVIC_SetPendingIRQ()`, `__NVIC_SetPriority()`, `__NVIC_SetPriorityGrouping()`, `NVIC_SetTargetState()`, `__NVIC_SetVector()`, `__NVIC_SystemReset()`, `SCB_GetFPUType()`, `SysTick_Config()`, `TZ_NVIC_ClearPendingIRQ_NS()`, `TZ_NVIC_DisableIRQ_NS()`, `TZ_NVIC_EnableIRQ_NS()`, `TZ_NVIC_GetActive_NS()`, `TZ_NVIC_GetEnableIRQ_NS()`, `TZ_NVIC_GetPendingIRQ_NS()`, `TZ_NVIC_GetPriority_NS()`, `TZ_NVIC_GetPriorityGrouping_NS()`, `TZ_NVIC_SetPendingIRQ_NS()`, `TZ_NVIC_SetPriority_NS()`, `TZ_NVIC_SetPriorityGrouping_NS()`, `TZ_SAU_Disable()`, `TZ_SAU_Enable()`, `TZ_SysTick_Config_NS()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 54`** (39 nodes): `core_cm35p.h`, `ITM_CheckChar()`, `ITM_ReceiveChar()`, `ITM_SendChar()`, `__NVIC_ClearPendingIRQ()`, `NVIC_ClearTargetState()`, `NVIC_DecodePriority()`, `__NVIC_DisableIRQ()`, `__NVIC_EnableIRQ()`, `NVIC_EncodePriority()`, `__NVIC_GetActive()`, `__NVIC_GetEnableIRQ()`, `__NVIC_GetPendingIRQ()`, `__NVIC_GetPriority()`, `__NVIC_GetPriorityGrouping()`, `NVIC_GetTargetState()`, `__NVIC_GetVector()`, `__NVIC_SetPendingIRQ()`, `__NVIC_SetPriority()`, `__NVIC_SetPriorityGrouping()`, `NVIC_SetTargetState()`, `__NVIC_SetVector()`, `__NVIC_SystemReset()`, `SCB_GetFPUType()`, `SysTick_Config()`, `TZ_NVIC_ClearPendingIRQ_NS()`, `TZ_NVIC_DisableIRQ_NS()`, `TZ_NVIC_EnableIRQ_NS()`, `TZ_NVIC_GetActive_NS()`, `TZ_NVIC_GetEnableIRQ_NS()`, `TZ_NVIC_GetPendingIRQ_NS()`, `TZ_NVIC_GetPriority_NS()`, `TZ_NVIC_GetPriorityGrouping_NS()`, `TZ_NVIC_SetPendingIRQ_NS()`, `TZ_NVIC_SetPriority_NS()`, `TZ_NVIC_SetPriorityGrouping_NS()`, `TZ_SAU_Disable()`, `TZ_SAU_Enable()`, `TZ_SysTick_Config_NS()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 59`** (32 nodes): `core_armv8mbl.h`, `__NVIC_ClearPendingIRQ()`, `NVIC_ClearTargetState()`, `NVIC_DecodePriority()`, `__NVIC_DisableIRQ()`, `__NVIC_EnableIRQ()`, `NVIC_EncodePriority()`, `__NVIC_GetActive()`, `__NVIC_GetEnableIRQ()`, `__NVIC_GetPendingIRQ()`, `__NVIC_GetPriority()`, `NVIC_GetTargetState()`, `__NVIC_GetVector()`, `__NVIC_SetPendingIRQ()`, `__NVIC_SetPriority()`, `NVIC_SetTargetState()`, `__NVIC_SetVector()`, `__NVIC_SystemReset()`, `SCB_GetFPUType()`, `SysTick_Config()`, `TZ_NVIC_ClearPendingIRQ_NS()`, `TZ_NVIC_DisableIRQ_NS()`, `TZ_NVIC_EnableIRQ_NS()`, `TZ_NVIC_GetActive_NS()`, `TZ_NVIC_GetEnableIRQ_NS()`, `TZ_NVIC_GetPendingIRQ_NS()`, `TZ_NVIC_GetPriority_NS()`, `TZ_NVIC_SetPendingIRQ_NS()`, `TZ_NVIC_SetPriority_NS()`, `TZ_SAU_Disable()`, `TZ_SAU_Enable()`, `TZ_SysTick_Config_NS()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 60`** (32 nodes): `core_cm23.h`, `__NVIC_ClearPendingIRQ()`, `NVIC_ClearTargetState()`, `NVIC_DecodePriority()`, `__NVIC_DisableIRQ()`, `__NVIC_EnableIRQ()`, `NVIC_EncodePriority()`, `__NVIC_GetActive()`, `__NVIC_GetEnableIRQ()`, `__NVIC_GetPendingIRQ()`, `__NVIC_GetPriority()`, `NVIC_GetTargetState()`, `__NVIC_GetVector()`, `__NVIC_SetPendingIRQ()`, `__NVIC_SetPriority()`, `NVIC_SetTargetState()`, `__NVIC_SetVector()`, `__NVIC_SystemReset()`, `SCB_GetFPUType()`, `SysTick_Config()`, `TZ_NVIC_ClearPendingIRQ_NS()`, `TZ_NVIC_DisableIRQ_NS()`, `TZ_NVIC_EnableIRQ_NS()`, `TZ_NVIC_GetActive_NS()`, `TZ_NVIC_GetEnableIRQ_NS()`, `TZ_NVIC_GetPendingIRQ_NS()`, `TZ_NVIC_GetPriority_NS()`, `TZ_NVIC_SetPendingIRQ_NS()`, `TZ_NVIC_SetPriority_NS()`, `TZ_SAU_Disable()`, `TZ_SAU_Enable()`, `TZ_SysTick_Config_NS()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 71`** (23 nodes): `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `portmacrocommon.h`, `ulPortCountLeadingZeros()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 80`** (20 nodes): `stm32h7xx_hal_ramecc.c`, `HAL_RAMECC_DeInit()`, `HAL_RAMECC_DetectErrorCallback()`, `HAL_RAMECC_DisableNotification()`, `HAL_RAMECC_EnableNotification()`, `HAL_RAMECC_GetError()`, `HAL_RAMECC_GetFailingAddress()`, `HAL_RAMECC_GetFailingDataHigh()`, `HAL_RAMECC_GetFailingDataLow()`, `HAL_RAMECC_GetHammingErrorCode()`, `HAL_RAMECC_GetRAMECCError()`, `HAL_RAMECC_GetState()`, `HAL_RAMECC_Init()`, `HAL_RAMECC_IRQHandler()`, `HAL_RAMECC_IsECCDoubleErrorDetected()`, `HAL_RAMECC_IsECCSingleErrorDetected()`, `HAL_RAMECC_RegisterCallback()`, `HAL_RAMECC_StartMonitor()`, `HAL_RAMECC_StopMonitor()`, `HAL_RAMECC_UnRegisterCallback()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 98`** (13 nodes): `secure_init.c`, `secure_init.c`, `secure_init.c`, `secure_init.c`, `secure_init.c`, `secure_init.c`, `secure_init.c`, `secure_init.c`, `secure_init.c`, `secure_init.c`, `secure_init.c`, `SecureInit_DePrioritizeNSExceptions()`, `SecureInit_EnableNSFPUAccess()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 104`** (8 nodes): `mpu_armv7.h`, `ARM_MPU_ClrRegion()`, `ARM_MPU_Disable()`, `ARM_MPU_Enable()`, `ARM_MPU_Load()`, `ARM_MPU_OrderedMemcpy()`, `ARM_MPU_SetRegion()`, `ARM_MPU_SetRegionEx()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 110`** (7 nodes): `portmacro.h`, `portmacro.h`, `portmacro.h`, `portmacro.h`, `portmacro.h`, `vPortMemoryBarrier()`, `vPortYield()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 130`** (4 nodes): `portASM.h`, `portASM.h`, `PUSH()`, `VPOPNE()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 133`** (3 nodes): `sysmem.c`, `sysmem.c`, `_sbrk()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 134`** (3 nodes): `arc_freertos_exceptions.c`, `arc_freertos_exceptions.c`, `freertos_exc_init()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 135`** (3 nodes): `portcomn.c`, `portcomn.c`, `pxPortInitialiseStack()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 136`** (3 nodes): `__STD_LIB_sbrk.c`, `__STD_LIB_sbrk.c`, `sbrk()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `HAL_GetTick()` connect `Community 1` to `Community 4`, `Community 8`, `Community 9`, `Community 10`, `Community 15`, `Community 18`, `Community 20`, `Community 27`, `Community 32`, `Community 34`, `Community 38`, `Community 40`, `Community 44`, `Community 45`, `Community 46`, `Community 49`, `Community 50`, `Community 62`, `Community 77`?**
  _High betweenness centrality (0.269) - this node is a cross-community bridge._
- **Why does `LL_LPTIM_Disable()` connect `Community 11` to `Community 5`?**
  _High betweenness centrality (0.170) - this node is a cross-community bridge._
- **Why does `main()` connect `Community 4` to `Community 0`, `Community 1`, `Community 40`?**
  _High betweenness centrality (0.100) - this node is a cross-community bridge._
- **Are the 222 inferred relationships involving `HAL_GetTick()` (e.g. with `CRYP_AESGCM_Process()` and `CRYP_AESCCM_Process()`) actually correct?**
  _`HAL_GetTick()` has 222 INFERRED edges - model-reasoned connections that need verification._
- **Are the 15 inferred relationships involving `xPortStartScheduler()` (e.g. with `vTaskStartScheduler()` and `vTaskSwitchContext()`) actually correct?**
  _`xPortStartScheduler()` has 15 INFERRED edges - model-reasoned connections that need verification._
- **Are the 3 inferred relationships involving `pxPortInitialiseStack()` (e.g. with `prvInitialiseNewTask()` and `ulPortGetCP0Cause()`) actually correct?**
  _`pxPortInitialiseStack()` has 3 INFERRED edges - model-reasoned connections that need verification._
- **Are the 4 inferred relationships involving `vPortEndScheduler()` (e.g. with `vTaskEndScheduler()` and `xTaskGetCurrentTaskHandle()`) actually correct?**
  _`vPortEndScheduler()` has 4 INFERRED edges - model-reasoned connections that need verification._