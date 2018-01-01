#pragma once

// Just wrap around printf so we don't use semihosting in release builds.
#ifdef DEBUG
#include <stdio.h>
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...)
#endif

#ifndef __IO
#define __IO volatile
#endif

// LED control to be used during command processing.
void LedInit();
void LedThinkingOn();
void LedThinkingOff();

// The param is the blink count, which is based on one of the error codes defined above.
// The LED will keep blinking until the param is set to 0 or noerrro
typedef enum ErrorCode_t
{
	ErrorCodeNone = 0,
	ErrorCodeEthAndLwipInit = 1,
	ErrorCodeDhcpTimeout = 2,
	ErrorCodeBrokeOutOfOsKernelStart = 3,
	ErrorCodeNetconnAcceptFailure = 4
} ErrorCode;

void LedError(ErrorCode error);

// User buffer control
void UserInit(int maxConns);
_Bool UserSessionLoad(int sessionId);
_Bool UserSessionUnload(int sessionId);
_Bool UserSessionActive(int sessionId);
_Bool UserSessionPutChar(int sessionId, char ch);
char UserSessionGetChar(int sessionId);

// TCP/IP server control

// For telnet
#define SERVER_PORT 23
#define MAX_CONNECTIONS 4

void TcpInit(int port, int maxConns);