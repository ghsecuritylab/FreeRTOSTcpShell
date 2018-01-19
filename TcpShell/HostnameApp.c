#include <stm32f7xx_hal.h>
#include <stm32f7xx_hal_flash.h>
#include <stm32f7xx_hal_flash_ex.h>
#include <lwip/api.h>
#include <lwip/opt.h>
#include <lwip/dhcp.h>
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
	
extern struct netif gnetif; /* network interface structure */
static char myhostname[MAX_HOSTNAME_LEN];

static int HostnameAppRun(PUserContext, int, char**);

const UserApp HostnameApp = { "hostname", HostnameAppRun, "[hostname]", "Sets or displays the hostname." };

int HostnameAppRun(PUserContext Context, int argc, char** argv)
{
	int rc = 0;
	
	if (argc == 1)
	{
		if (xSemaphoreTake(SystemSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE)
		{
			// 0-arg form. Just print the hostname.
			console_puts(Context, netif_get_hostname(&gnetif));
			console_puts(Context, "\r\n");
			xSemaphoreGive(SystemSemaphore);
		}
		else
		{
			console_puts(Context, "Timed out\r\n");
			rc = -1;
		}
	}
	else if (argc == 2)
	{
		if (xSemaphoreTake(SystemSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE)
		{
			strncpy(myhostname, argv[1], sizeof(myhostname));
			dprintf("%d: hostname changed to %s\n", Context->connid, myhostname);
			
#if LWIP_NETIF_HOSTNAME == 1
			/* Sets the interface hostname */
			netif_set_hostname(&gnetif, myhostname);
#endif // LWIP_NETIF_HOSTNAME
			
			xSemaphoreGive(SystemSemaphore);
		}
		else
		{
			console_puts(Context, "Timed out\r\n");
			rc = -1;
		}
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
