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
	
static int HelpAppRun(PUserContext, int, char**);

const UserApp HelpApp = { "help", HelpAppRun, "", "Displays command help" };

int HelpAppRun(PUserContext Context, int argc, char** argv)
{
	int rc = 0;
	int i;
	
	console_setflags(Context, ConsoleFlagsEchoOff);
	for (i = 0; i < MAX_USERAPP; ++i)
	{
		PUserApp app = AppList[i];
		console_printf(Context, "%8s %25s - %s\r\n", app->AppName, app->ArgUsage, app->Description);
	}
	
done:
	console_unsetflags(Context, ConsoleFlagsEchoOff);
	if (rc > 0)
	{
		rc = 0;
	}
	
	return rc;
}
