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
	
static int DbAppRun(pconsole, int, char**);

const app db_app = { "db", DbAppRun, "<hex-offset> <0-1024>", "Dumps bytes from the given location to the console." };

int DbAppRun(pconsole Context, int argc, char** argv)
{	
	console_setflags(Context, ConsoleFlagsEchoOff);
	int rc = 0;
	if (argc == 3)
	{
		int src;
		int len;
		sscanf(argv[1], "%x", &src);
		sscanf(argv[2], "%d", &len);	
		
		if (src >= 0 && len >= 0 && len < 1024)
		{
			char* buf = (char*)pvPortMalloc(len);
			if (buf)
			{
				memcpy4(buf, (void*)src, len);
				for (int i = 0; i < len; ++i)
				{
					console_printf(Context, "%02x", buf[i]);
				}
			
				vPortFree(buf);
			}
		}
	}
	else
	{
		console_puts(Context, "Invalid arguments\r\n");
		rc = -1;
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
