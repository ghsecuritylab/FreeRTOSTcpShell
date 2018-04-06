#pragma once

#define RTC_CLOCK_RATE 32767 // MS per clock assuming clock div/16
#define IDLE_TICK_GRANULARITY_MS 1 // The default tick granularity

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

// Count of clock transactions and idle tick granularity in ms
extern volatile int idle_granularity_ms;

// RTOS entry
void rtos_entry(void);
void beep(uint16_t millis);
void* memcpy4(void*, void*, int);

const char* rtos_hal_status(HAL_StatusTypeDef halTD);
