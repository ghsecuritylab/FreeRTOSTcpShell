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
	
static int SetAppRun(pconsole, int, char**);

const app set_app = { "set", SetAppRun, "[name [value]]", "Displays variable(s) or sets an environment variable" };

int SetAppRun(pconsole Context, int argc, char** argv)
{
	int rc = 0;
	int i;
	
	if (argc == 1)
	{
		// 1-arg form. Just dump the whole environment block.
		pconsole_env cur = Context->env;
		while (cur)
		{
			console_printf(Context, "%s=%s\r\n", cur->name, cur->value);
			cur = cur->next;
		}
	}
	else if (argc == 2)
	{
		// 2-arg form. Print a specific variable
		const char* value;
		if ((rc = console_getenv(Context, argv[1], &value) == 0))
		{
			console_puts(Context, value);
			console_puts(Context, "\r\n");
		}
		else
		{
			dprintf("%d: console_getenv failed: %d\n", Context->connid, rc);
		}
	}
	else if (argc == 3)
	{
		// 3-arg form. Set a specific variable
		if((rc = console_setenv(Context, argv[1], argv[2]) < 0))
		{
			dprintf("%d: console_setenv failed: %d\n", Context->connid, rc);
		}
	}
	else
	{
		console_puts(Context, "Invalid argument\r\n");
		rc = -1;
	}
	
done:
	if (rc > 0)
	{
		rc = 0;
	}
	
	return rc;
}
