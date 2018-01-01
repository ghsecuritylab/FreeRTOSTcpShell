#include <stm32f7xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <queue.h>
#include <stdarg.h>
#include "tcpshell.h"
#include "User.h"

static bool LogonAppRun(PUserContext UserContext);
static void LogonAppTerminate(PUserContext UserContext);
static PAppContext LogonAppCreateContext(PUserContext UserContext);
static void LogonAppDeleteContext(PAppContext UserContext);

UserApp LogonApp = { "Logon", LogonAppRun, LogonAppTerminate, LogonAppCreateContext, LogonAppDeleteContext };
	
bool LogonAppRun(PUserContext Context)
{
	char UserName[USER_NAME_LEN] = { };
	UserContextPutString(Context, "Username: ");
	Context->EchoOn = true;
	int read = UserContextGetChars(Context, UserName, USER_NAME_LEN, '\n');
	if (UserName[read - 1] == '\n')
	{
		UserName[read - 1] = 0;
		Context->EchoOn = false;
		return UserContextSetEnv(Context, ENV_USERNAME, UserName, strlen(UserName));	
	}
	
	// Session terminal
	return false;
}

void LogonAppTerminate(PUserContext UserContext)
{
}

PAppContext LogonAppCreateContext(PUserContext UserContext)
{
	return NULL;
}

void LogonAppDeleteContext(PAppContext UserContext)
{
}