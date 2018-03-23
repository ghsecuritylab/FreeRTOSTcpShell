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
	
static int MessageAppRun(pconsole, int, char**);
static const int MaxChars = 104;

const app message_app = { "message", MessageAppRun, "[text1 [text2 [...]]]", "Shows a message on the LCD." };

int MessageAppRun(pconsole Context, int argc, char** argv)
{
	int rc = 0;
	char buffer[MaxChars];
	
	memset(buffer, 0, MaxChars);
	for (int i = 1; i < argc; ++i)
	{
		strncat(buffer, argv[i], MaxChars);
		strncat(buffer, " ", MaxChars);
	}
	
	led_display_set_message(buffer);
	
done:
	if (rc > 0)
	{
		rc = 0;
	}
	
	return rc;
}
