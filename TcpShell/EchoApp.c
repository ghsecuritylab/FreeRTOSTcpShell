#include <stm32f7xx_hal.h>
#include <lwip/api.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <queue.h>
#include <stdarg.h>
#include "tcpshell.h"
#include "User.h"

static int EchoAppRun(PUserContext);

UserApp EchoApp = { "Echo", EchoAppRun };

int EchoAppRun(PUserContext Context)
{
	int ch = 0;
	while ((ch = TcpGetchar(Context)) > 0)
	{
		if (TcpPutchar(Context, ch) != ERR_OK)
		{
			goto done;
		}
	}
	
done:
	if (ch > 0)
	{
		ch = 0;
	}
	
	return ch;
}
