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

static int ShellAppRun(pconsole, int, char**);

const app shell_app = { "shell", ShellAppRun, "", "Application shell" };

int ShellAppRun(pconsole Context, int argc, char** argv)
{
	int rc = 0;
	char command[MAX_ARGUMENTS_LEN] = { };
	int iCommand = 0;
	
	// Ask for commands until we think we get a valid one.
	console_setflags(Context, ConsoleFlagsEchoOff);
	console_puts(Context, "? ");
	if ((rc = console_getline(Context, ConsoleFlagsEchoOff, command, sizeof(command))) < 0)
	{
		dprintf("%d: console_getline(command) failed. rc=%d", Context->connid, rc);
		goto done;
	}
	
	if ((rc = console_tokenize(Context, command)) >= 0)
	{
		if (Context->argc > 0)
		{
			for (int i = 0; i < MAX_USERAPP; ++i)
			{
				// Look up a command.
				if(strcmp(app_list[i]->name, Context->argv[0]) == 0)
				{
					if ((rc = console_exec(Context, app_list[i])) < 0)
					{
						dprintf("%d: console_exec(,AppList[i]) failed. rc=%d", Context->connid, rc);
						break;
					}
					
					goto done;
				}
			}
		}
	}
	else
	{
		dprintf("%d: console_tokenize(...) failed. rc=%d", Context->connid, rc);
	}
	
	console_puts(Context, "Unrecognized command. Type 'help' for help.\r\n");
	
done:
	console_unsetflags(Context, ConsoleFlagsEchoOff);
	if (rc > 0)
	{
		rc = 0;
	}
	
	return rc;
}