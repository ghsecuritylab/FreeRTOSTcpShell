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
	
static int EchoAppRun(PUserContext, int, char**);

const UserApp EchoApp = { "echo", EchoAppRun, "[text1 [text2 [...]]]", "Echos some text to the terminal." };

int EchoAppRun(PUserContext Context, int argc, char** argv)
{
	int rc = 0;
	
	console_setflags(Context, ConsoleFlagsEchoOff);
	for (int i = 1; i < argc; ++i)
	{
		console_puts(Context, argv[i]);
		console_puts(Context, " ");
	}
	
	console_puts(Context, "\r\n");
	
done:
	console_unsetflags(Context, ConsoleFlagsEchoOff);
	if (rc > 0)
	{
		rc = 0;
	}
	
	return rc;
}
