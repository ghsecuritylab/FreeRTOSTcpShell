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

static int ExitAppRun(PUserContext, int, char**);

const UserApp ExitApp = { "exit", ExitAppRun, "", "Exit the session" };

int ExitAppRun(PUserContext Context, int argc, char** argv)
{
	dprintf("%d: Asked for exit\n", Context->connid);
	Context->exiting = true;
	return 0;
}