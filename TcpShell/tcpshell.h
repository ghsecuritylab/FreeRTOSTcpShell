#pragma once

#ifdef __CPLUSPLUS
extern "C" {
#endif // __CPLUSPLUS

// Just wrap around printf so we don't use semihosting in release builds.
#ifdef DEBUG
#include <stdio.h>
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...)
#endif

#include <FreeRTOS.h>
#include <list.h>
#include <semphr.h>
#include <assert.h>
#include <queue.h>
#include "lwipopts.h"
#include "libtelnet.h"
	
// assert a value is true and branch if it is true
#define assert_if(X) assert(X); if ((X))
#define BREAK_ERROR_HANDLER() __BKPT(0)
#define BREAK_ASSERT_FAILED() __BKPT(1)
#define BREAK_STACK_OVERFLOW() __BKPT(2)
#define BREAK_MALLOC_FAILED() __BKPT(3)

#ifndef __IO
#define __IO volatile
#endif
	
// RTOS entry
#include "rtosmain.h"
	
// I2C functionality
#include "i2c.h"

// LED control to be used during command processing.
#include "led_display.h"

// TCP/IP server control and I/O convenience functions
#include "tcpserver.h"

// Read and write a char of telnet input. Use this instead of telnet_recv as it properly reads data off the port and checks for errors.
#include "console.h"
	
#ifdef __CPLUSPLUS
}
#endif // __CPLUSPLUS