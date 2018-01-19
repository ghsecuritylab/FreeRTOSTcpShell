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
	
static int MacAppRun(PUserContext, int, char**);

const UserApp MacApp = { "macaddr", MacAppRun, "", "Displays the mac address." };

int MacAppRun(PUserContext Context, int argc, char** argv)
{
	int rc = 0;
	
	if (argc == 1)
	{
		// 0-arg form. Just print the mac address.
		console_printf(Context, "%02x:%02x:%02x:%02x:%02x:%02x\r\n", macaddress[0], macaddress[1], macaddress[2], macaddress[3], macaddress[4], macaddress[5]);
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
