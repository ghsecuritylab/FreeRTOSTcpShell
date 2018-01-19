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
	
static int HelpAppRun(pconsole, int, char**);

const app help_app = { "help", HelpAppRun, "", "Displays command help" };

int HelpAppRun(pconsole Context, int argc, char** argv)
{
	int rc = 0;
	int i;
	
	console_setflags(Context, ConsoleFlagsEchoOff);
	for (i = 0; i < MAX_USERAPP; ++i)
	{
		papp app = app_list[i];
		console_printf(Context, "%8s %25s - %s\r\n", app->name, app->usage, app->description);
	}
	
done:
	console_unsetflags(Context, ConsoleFlagsEchoOff);
	if (rc > 0)
	{
		rc = 0;
	}
	
	return rc;
}
