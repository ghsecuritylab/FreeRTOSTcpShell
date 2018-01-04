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

static int EchoAppRun(PUserContext);

UserApp EchoApp = { "Echo", EchoAppRun };

int EchoAppRun(PUserContext Context)
{
	int ch = 0;
	
	console_setflags(Context, ConsoleFlagsEchoOff);
	console_puts(Context, "Hello ");
	console_printf(Context, "Welcome to %s\n", "shitty-echo-client");
	while ((ch = console_getchar(Context)) > 0)
	{
		if (console_putchar(Context, ch) < 0)
		{
			goto done;
		}
	}
	
done:
	console_unsetflags(Context, ConsoleFlagsEchoOff);
	if (ch > 0)
	{
		ch = 0;
	}
	
	return ch;
}
