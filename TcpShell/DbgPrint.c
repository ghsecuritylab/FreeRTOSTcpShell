#include <stm32f7xx_hal.h>
#include <stm32f746xx.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include <stdarg.h>
#include <stdio.h>
#include "tcpshell.h"

#ifdef DEBUG

static char Buffer[1 << 12] = {};

void DbgPrint(char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int len = vsnprintf(Buffer, sizeof(Buffer), fmt, args);
	if (len > 0)
	{
		// SWO debug it
		int i;
		for (i = 0; i < len; ++i)
		{
			char c = Buffer[i];
			asm(
				"mov r0, #0x03\n"   /* SYS_WRITEC */
				"mov r1, %[msg]\n"
				"bkpt #0xAB\n"
				:
				: [msg] "r" (&c)
				: "r0",
				  "r1");
		}
	}
		
	va_end(args);
}

#endif // DEBUG