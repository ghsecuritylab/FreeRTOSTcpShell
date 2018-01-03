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

#ifndef __IO
#define __IO volatile
#endif

// For telnet
#define SERVER_PORT 23
#define MAX_CONNECTIONS (MEMP_NUM_TCP_PCB - 1)

// Max username len
#define USER_NAME_LEN 64

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
	ErrorApplicationAssertFailure = 6
} ErrorCode;

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
	struct netbuf *buf;
	char* ptr;
	unsigned short remaining;
	PUserApp NextApp;
} UserContext, *PUserContext;

// LED control to be used during command processing.
void LedInit();
void LedThinkingOn();
void LedThinkingOff();
void LedError(ErrorCode error);

// TCP/IP server control and I/O convenience functions
void TcpInit(int port, int maxConns);

// These are only meant to be called from the thread of a running app...
int TcpPutchar(char ch);
int TcpGetchar();
