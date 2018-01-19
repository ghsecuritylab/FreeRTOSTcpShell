#include <stm32f7xx_hal.h>
#include <stm32f7xx_hal_flash.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include <stdbool.h>
#include "tcpshell.h"
#include "User.h"

const PUserApp AppList[MAX_USERAPP] = 
{ 
	(const PUserApp)&LogonApp,
	(const PUserApp)&ShellApp,
	(const PUserApp)&EchoApp,
	(const PUserApp)&SetApp,
	(const PUserApp)&HostnameApp,
	(const PUserApp)&MacApp,
	(const PUserApp)&ResetApp,
	(const PUserApp)&ExitApp,
	(const PUserApp)&HelpApp
};
