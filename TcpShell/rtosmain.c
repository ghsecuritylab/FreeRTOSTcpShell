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

// RTOS hooks

/* Includes ------------------------------------------------------------------*/
#include <stm32f7xx_hal.h>
#include <stm32f7xx_hal_pwr.h>
#include <stm32f7xx_hal_rtc.h>
#include <stm32f7xx_hal_i2c.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include <stdbool.h>
#include <ctype.h>
#include <list.h>
#include "tcpshell.h"
#include "i2c.h"

// Idle tick granularity (ms) assuming a 32Khz rtc clock source with a 16 clock divisor
#define IDLE_TICKS_TO_COUNTS(X) (RTC_CLOCK_RATE * X / 1000 / 16)

extern RTC_HandleTypeDef hrtc;
volatile int idle_granularity_ms = IDLE_TICK_GRANULARITY_MS;  // The idle clock granularity in MS
static List_t ProcessList;

static void CPU_CACHE_Enable(void);

/**
  * @brief  RTOS entry
  * @param  None
  * @retval None
  */
void rtos_entry(void)
{
	/* STM32F4xx HAL library initialization:
	     - Configure the Flash prefetch, instruction and Data caches
	     - Configure the Systick to generate an interrupt each 1 msec
	     - Set NVIC Group Priority to 4
	     - Global MSP (MCU Support Package) initialization
	*/
	dprintf("TcpShell: RTOS entry. Port=%u, maxConns=%u\n", SERVER_PORT, MAX_CONNECTIONS);
	
	vListInitialise(&ProcessList);
	
	// System initialization
	CPU_CACHE_Enable();
	HAL_PWR_DisableSleepOnExit();
	
	// User defined API initialziation
	i2c_init();
	led_init();
	tcpserver_init(SERVER_PORT, MAX_CONNECTIONS);
  
	/* Start scheduler */
	dprintf("TcpShell: about to call osKernelStart()\n");	
	led_thinking_off();
	osKernelStart();
	
	/* We should never get here as control is now taken by the scheduler */
	dprintf("TcpShell: Broke out of osKernelStart()\n");
}

void rtos_process_init(process* handle, const char* name)
{
	assert_if(handle && name)
	{
		vListInitialiseItem(&handle->item);
		handle->mutex = xSemaphoreCreateBinary();
		xSemaphoreGive(handle->mutex);
		handle->name = name;
		handle->item.xItemValue = (uint32_t)name;
	}
}

void rtos_process_begin(process* handle)
{
	// Must not be called from an ISR
	assert_if(handle && (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) == 0)
	{
		while (pdTRUE != xSemaphoreTake(handle->mutex, 10000)) ;
		vListInsert(&ProcessList, &handle->item);
	}
}

void rtos_process_end(process* handle)
{
	// Must not be called from an ISR
	assert_if(handle && (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) == 0)
	{
		UBaseType_t countStart = listCURRENT_LIST_LENGTH(&ProcessList);
		UBaseType_t countEnd = uxListRemove(&handle->item);
		assert(countEnd - countStart == 1); // Uh oh. The same process was started multiple times?
		if (countEnd < countStart)
		{
			xSemaphoreGive(handle->mutex);
		}
	}
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

/**
  * @brief  SYSTICK callback.
  * @retval None
  */
void HAL_SYSTICK_Callback(void)
{
	osSystickHandler();
}

/**
  * @brief  idle task hook.
  * @retval None
  */
void vApplicationIdleHook(void)
{
	// Configure the MCU into sleep mode with a wakeup alarm to check for more non-ISR work to do in 10ish ms.
	if (idle_granularity_ms > 0)
	{
		uint32_t counts = IDLE_TICKS_TO_COUNTS(idle_granularity_ms);
//		if (counts > 0)
//		{
//			if (HAL_OK == HAL_RTCEx_SetWakeUpTimer(&hrtc, counts, RTC_WAKEUPCLOCK_RTCCLK_DIV16))
//			{
//				if (conns > 0 || !listLIST_IS_EMPTY(&ProcessList))
//				{
//					HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
//				}
//				else
//				{
//					assert(conns == 0);  // Oops. Don't want to go into STOP mode if the MCU has clients
//					__HAL_PWR_UNDERDRIVE_ENABLE();
//					HAL_PWREx_EnterUnderDriveSTOPMode(PWR_MAINREGULATOR_UNDERDRIVE_ON, PWR_SLEEPENTRY_WFI);
//				}
//			
//				HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
//			}
//		}
	}
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
	vTaskSuspendAll();
	BREAK_ASSERT_FAILED();
	xTaskResumeAll();
}

#endif

#if(  configCHECK_FOR_STACK_OVERFLOW > 0 )

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	vTaskSuspendAll();
	BREAK_STACK_OVERFLOW();
	led_error(ErrorApplicationStackOverflow);
	xTaskResumeAll();
}

#endif

void vApplicationMallocFailedHook(void)
{
	vTaskSuspendAll();
	size_t freeHeapSize = xPortGetFreeHeapSize();
	size_t minimumEverFreeHeapSize = xPortGetMinimumEverFreeHeapSize();
	BREAK_MALLOC_FAILED();
	led_error(ErrorApplicationOutOfMemory);
	xTaskResumeAll();
}

const char* rtos_hal_status(HAL_StatusTypeDef halTD)
{
	switch (halTD)
	{
	case HAL_OK:
		return "HAL_OK";
	case HAL_ERROR:
		return "HAL_ERROR";
	case HAL_BUSY:
		return "HAL_BUSY";
	case HAL_TIMEOUT:
		return "HAL_TIMEOUT";
	}
	
	return "";
}
	
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
