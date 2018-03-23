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
	
static int BeepAppRun(pconsole, int, char**);

const app beep_app = { "beep", BeepAppRun, "period", "Beeps." };

int BeepAppRun(pconsole Context, int argc, char** argv)
{
	int rc = 0;
	
	if (argc == 2)
	{
		int period = atoi(argv[1]);
		beep(period);
	}
	else
	{
		console_puts(Context, "Invalid arguments\r\n");
		rc = -1;
	}
	
done:
	console_unsetflags(Context, ConsoleFlagsEchoOff);
	if (rc > 0)
	{
		rc = 0;
	}
	
	return rc;
}
