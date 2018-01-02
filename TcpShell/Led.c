#include <stm32f7xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include <assert.h>
#include <stdbool.h>
#include "tcpshell.h"

static osThreadId HeartbeatLedHandle = NULL;
static osThreadId BusyLedHandle = NULL;
static osThreadId ErrorLedHandle = NULL;

static void HeartbeatLedThread(void const *argument);
static void BusyLedThread(void const *argument);
static void ErrorLedThread(void const *argument);

volatile int BlinkCode = 0;

void LedInit()
{
	// Set up the LEDs for our devkit, which will be used to display various information.
	GPIO_InitTypeDef gpioInitStructure;
	__GPIOB_CLK_ENABLE();
	gpioInitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14;
	gpioInitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	gpioInitStructure.Pull = GPIO_PULLUP;
	gpioInitStructure.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB, &gpioInitStructure);

	/* Thread 1 definition */
	osThreadDef(HeartbeatLed, HeartbeatLedThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  
	/*  Thread 2 definition */
	osThreadDef(BusyLed, BusyLedThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	
	/*  Thread 3 definition */
	osThreadDef(ErrorLed, ErrorLedThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  
	/* Start thread 1 */
	HeartbeatLedHandle = osThreadCreate(osThread(HeartbeatLed), NULL);
  
	/* Start thread 2 */
	BusyLedHandle = osThreadCreate(osThread(BusyLed), NULL);
	
	/* Start the error thread */
	ErrorLedHandle = osThreadCreate(osThread(ErrorLed), NULL);
}

/**
  * @brief  Toggle LED1
  * @param  thread not used
  * @retval None
  */
static void HeartbeatLedThread(void const *argument)
{
	(void) argument;
  
	for (;;)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
		osDelay(500);
		
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
		osDelay(750);
	}
}

/**
  * @brief  Toggle LED2 thread
  * @param  argument not used
  * @retval None
  */
static void BusyLedThread(void const *argument)
{
	uint32_t count;
	(void) argument;
  
	for (;;)
	{
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
		osDelay(200);
	}
}

static void ErrorLedThread(void const *argument)
{
	int i;
	
	// Blink code is one second per blink between a 3 second delay
	for (;;)
	{
		int count = BlinkCode;
		if (count > 0)
		{
			
			for (i = 0; i < count; ++i)
			{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
				osDelay(500);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
				osDelay(750);
			}
		}
		
		osDelay(1000);
	}
}

void LedThinkingOn()
{
	osThreadResume(BusyLedHandle);
}

void LedThinkingOff()
{
	osThreadSuspend(BusyLedHandle);
}

void LedError(ErrorCode error)
{
	assert(error >= 0);
	osThreadSuspend(ErrorLedHandle);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
	BlinkCode = (int)error;
	if (error != ErrorCodeNone)
	{
		osThreadResume(ErrorLedHandle);	
	}
}
