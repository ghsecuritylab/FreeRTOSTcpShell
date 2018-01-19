#include <stm32f7xx_hal.h>
#include <lwip/api.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <queue.h>
#include <stdarg.h>
#include "tcpshell.h"
#include "User.h"
	
static int ResetAppRun(PUserContext, int, char**);

const UserApp ResetApp = { "reset", ResetAppRun, "", "Resets the MCU." };

int ResetAppRun(PUserContext Context, int argc, char** argv)
{
	if (xSemaphoreTake(SystemSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE)
	{
		dprintf("%d: asked for reset\n", Context->connid);
		Context->exiting = true;
		NVIC_SystemReset();
		uint32_t tickend = HAL_GetTick() + 10000 / portTICK_PERIOD_MS;
		while (HAL_GetTick() < tickend) ;
		xSemaphoreGive(SystemSemaphore);
	}
	
	console_puts(Context, "Timed out\r\n");
	return -1;
}
