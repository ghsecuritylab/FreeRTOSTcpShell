#include <stm32f7xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include <FreeRTOS.h>
#include <assert.h>
#include <stdbool.h>
#include <semphr.h>
#include "tcpshell.h"

static osThreadId HeartbeatLedHandle = NULL;
static osThreadId BusyLedHandle = NULL;
static osThreadId ErrorLedHandle = NULL;
volatile int BlinkCode = 0;

static void HeartbeatLedThread(void const *argument);
static void BusyLedThread(void const *argument);
static void ErrorLedThread(void const *argument);
extern void DisplayLedThread(void const *argument);

void led_init()
{
	osThreadDef(HeartbeatLed, HeartbeatLedThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadDef(BusyLed, BusyLedThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadDef(ErrorLed, ErrorLedThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadDef(DisplayLed, DisplayLedThread, osPriorityNormal, 0, 2 * configMINIMAL_STACK_SIZE);
		
	HeartbeatLedHandle = osThreadCreate(osThread(HeartbeatLed), NULL);
	BusyLedHandle = osThreadCreate(osThread(BusyLed), NULL);
	ErrorLedHandle = osThreadCreate(osThread(ErrorLed), NULL);
}

void led_thinking_on()
{
	osThreadResume(BusyLedHandle);
}

void led_thinking_off()
{
	osThreadSuspend(BusyLedHandle);
}

void led_error(ErrorCode error)
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
	for(;  ;)
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
