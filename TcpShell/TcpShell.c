/**
  ******************************************************************************
  * @file    FreeRTOS/FreeRTOS_ThreadCreation/Src/main.c
  * @author  MCD Application Team
  * @version V1.2.2
  * @date    25-May-2015
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stm32f7xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include <stdbool.h>
#include "tcpshell.h"

extern ETH_HandleTypeDef EthHandle;
extern ETH_DMADescTypeDef* DMARxDscrTab;
extern ETH_DMADescTypeDef* DMATxDscrTab;
SemaphoreHandle_t SystemSemaphore;

static void MPU_Config(void);
static void CPU_CACHE_Enable(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	/* STM32F4xx HAL library initialization:
	     - Configure the Flash prefetch, instruction and Data caches
	     - Configure the Systick to generate an interrupt each 1 msec
	     - Set NVIC Group Priority to 4
	     - Global MSP (MCU Support Package) initialization
	*/
	dprintf("TcpShell: Init code. Port=%u, maxConns=%u\n", SERVER_PORT, MAX_CONNECTIONS);
	
	/* Configure the MPU attributes as Device memory for ETH DMA descriptors */
	MPU_Config();
  
	/* Enable the CPU Cache */
	CPU_CACHE_Enable();
	
	/* HAL and function init code */
	HAL_Init();
	LedInit();
	
	SystemSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(SystemSemaphore);
	TcpInit(SERVER_PORT, MAX_CONNECTIONS);
  
	/* Start scheduler */
	dprintf("TcpShell: about to call osKernelStart()\n");	
	LedThinkingOff();
	osKernelStart();
	
	/* We should never get here as control is now taken by the scheduler */
	dprintf("TcpShell: Broke out of osKernelStart()\n");
	LedError(ErrorCodeBrokeOutOfOsKernelStart);
}

void SysTick_Handler(void)
{
	HAL_IncTick();
	osSystickHandler();
}

void ETH_IRQHandler(void)
{
	HAL_ETH_IRQHandler(&EthHandle);
}

/**
  * @brief  Configure the MPU attributes as Device for  Ethernet Descriptors in the SRAM1.
  * @note   The Base Address is 0x20010000 since this memory interface is the AXI.
  *         The Configured Region Size is 256B (size of Rx and Tx ETH descriptors) 
  *       
  * @param  None
  * @retval None
  */
static void MPU_Config(void)
{
	MPU_Region_InitTypeDef MPU_InitStruct;
  
	/* Disable the MPU */
	HAL_MPU_Disable();
  
	/* Configure the MPU attributes as Device for Ethernet Descriptors in the SRAM */
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress = (uint32_t)&DMARxDscrTab[0];
	MPU_InitStruct.Size = MPU_REGION_SIZE_128B;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);
  
	/* Configure the MPU attributes as Device for Ethernet Descriptors in the SRAM */
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress = (uint32_t)&DMATxDscrTab[0];
	MPU_InitStruct.Size = MPU_REGION_SIZE_128B;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Enable the MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
	/* Enable I-Cache */
	SCB_EnableICache();

	/* Enable D-Cache */
	SCB_EnableDCache();
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
	dprintf("Assert failed: file %s on line %lu\r\n", file, line);
	LedError(ErrorApplicationAssertFailure);
	asm("bkpt 255");
}

#endif

#if(  configCHECK_FOR_STACK_OVERFLOW > 0 )

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	dprintf("Stack overflow in task %s\r\n", pcTaskName);
	LedError(ErrorApplicationStackOverflow);
	asm("bkpt 255");
}

#endif

void vApplicationMallocFailedHook(void)
{
	dprintf("malloc failed\r\n");
	LedError(ErrorApplicationOutOfMemory);
	asm("bkpt 255");
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
