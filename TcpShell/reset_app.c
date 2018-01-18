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
	
static int ResetAppRun(pconsole, int, char**);

const app reset_app = { "reset", ResetAppRun, "", "Resets the MCU." };

int ResetAppRun(pconsole Context, int argc, char** argv)
{
	dprintf("%d: asked for reset\n", Context->connid);
	Context->exiting = true;
	NVIC_SystemReset();
	uint32_t tickend = HAL_GetTick() + 10000 / portTICK_PERIOD_MS;
	while (HAL_GetTick() < tickend) ;
	
	console_puts(Context, "Timed out\r\n");
	return -1;
}
