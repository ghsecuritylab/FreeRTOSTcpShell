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
	
static int LogonAppRun(pconsole, int, char**);

const app logon_app = { "logon", LogonAppRun, "", "Sets the " ENV_USERNAME " variable." };

int LogonAppRun(pconsole Context, int argc, char** argv)
{
	int rc = 0;
	int iUserName = 0;
	char username[MAX_USERNAME_LEN] = { };
	
	console_setflags(Context, ConsoleFlagsEchoOff);
	console_puts(Context, "Username: ");
	
	if ((rc = console_getline(Context, ConsoleFlagsEchoOff, username, sizeof(username))) < 0)
	{
		dprintf("%d: console_getline(,,username,) failed. rc=%d", Context->connid, rc);
		goto done;
	}
	
	if ((rc = console_setenv(Context, ENV_USERNAME, username)) < 0)
	{
		dprintf("%d: console_setenv " ENV_USERNAME "=%s failed. rc=%d", Context->connid, username, rc);
		goto done;
	}
	
done:
	console_unsetflags(Context, ConsoleFlagsEchoOff);
	if (rc >= 0)
	{
		console_printf(Context, "Hello %s, ", username);
		console_printf(Context, "Welcome to %s!\n", hostname);
		return 0;
	}
	
	return rc;
}
