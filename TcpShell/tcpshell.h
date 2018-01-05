#pragma once

// Just wrap around printf so we don't use semihosting in release builds.
#ifdef DEBUG
#include <stdio.h>
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...)
#endif

#include <queue.h>
#include "lwipopts.h"
#include "FreeRTOSConfig.h"
#include "libtelnet.h"

#ifndef __IO
#define __IO volatile
#endif

// For telnet
#define SERVER_PORT 23
#define MAX_CONNECTIONS 4
#define MAX_HOSTNAME_LEN 64

extern const char hostname[MAX_HOSTNAME_LEN];
extern const uint8_t macaddress[6];

// The param is the blink count, which is based on one of the error codes defined above.
// The LED will keep blinking until the param is set to 0 or noerrro
typedef enum ErrorCode_t
{
	ErrorCodeNone = 0,
	ErrorCodeEthAndLwipInit = 1,
	ErrorCodeDhcpTimeout = 2,
	ErrorCodeBrokeOutOfOsKernelStart = 3,
	ErrorCodeNetconnAcceptFailure = 4,
	ErrorApplicationStackOverflow = 5,
	ErrorApplicationAssertFailure = 6,
	ErrorApplicationOutOfMemory = 7
} ErrorCode;

typedef enum ConsoleFlags
{
	ConsoleFlagsNone    = 0x0,
	ConsoleFlagsEchoOff = 0x1
} ConsoleFlags;

// These typedefs define the user and app context
typedef struct UserContext_t UserContext, *PUserContext;

typedef int(*PAppRun)(PUserContext);

typedef struct UserApp_t
{
	const char* AppName;
	PAppRun Run;
} UserApp, *PUserApp;

typedef struct UserContext_t
{
	struct netconn *conn;
	unsigned int connid;
	struct telnet_t* telnet;
	struct netbuf* buf; // TCP input and buffer
	char* ptr;
	unsigned short remaining;
	int write_err;
	bool newch;
	PUserApp NextApp;
} UserContext, *PUserContext;

// LED control to be used during command processing.
void LedInit();
void LedThinkingOn();
void LedThinkingOff();
void LedError(ErrorCode error);

// TCP/IP server control and I/O convenience functions
void TcpInit(int port, int maxConns);

// Read and write a char of telnet input. Use this instead of telnet_recv as it properly reads data off the port and checks for errors.
int console_getchar(PUserContext context);
int console_putchar(PUserContext context, char ch);
int console_puts(PUserContext context, const char* str);
int console_printf(PUserContext context, const char* fmt, ...);
int console_vprintf(PUserContext context, const char* fmt, va_list args);
int console_setflags(PUserContext context, ConsoleFlags flags);
int console_unsetflags(PUserContext context, ConsoleFlags flags);