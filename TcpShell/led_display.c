#include <stm32f7xx_hal.h>
#include <stm32f7xx_hal_i2c.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include <FreeRTOS.h>
#include <assert.h>
#include <stdbool.h>
#include <semphr.h>
#include <queue.h>
#include <string.h>
#include "tcpshell.h"

#include "lcdfont.h"

#define SSD1306 "SSD1306: "
#define SSD1306_ADDRESS  0x78
#define SSD1306_PIXEL_ROWS 32
#define SSD1306_PIXEL_COLS 128
#define SSD1306_TEXT_ROWS (SSD1306_PIXEL_ROWS / FONT_HEIGHT)
#define SSD1306_TEXT_COLS (SSD1306_PIXEL_COLS / FONT_WIDTH)
#define SSD1306_TXLEN 16

#define SSD1306_BASE(C,A,...) { \
	uint8_t data[] = { __VA_ARGS__ }; \
	for(size_t i = 0 ; i < sizeof(data)/sizeof(data[0]) ; ++i) { \
		status = HAL_I2C_Mem_Write(&hi2c1, address, A, 1, &data[i], 1, 100); \
		if(HAL_OK != status) { \
			dprintf(SSD1306 C " DMA failed\n"); \
			goto exit; \
		} \
	} \
}

#define SSD1306_COMMAND(C,...) SSD1306_BASE(C, 0x00, __VA_ARGS__)

#define SSD1306_DATA(C,...) SSD1306_BASE(C, 0x40, __VA_ARGS__)

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22
#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_CHARGEPUMP 0x8D
#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL 0x2F
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A

typedef enum Ssd1306Command_t
{
	Clear,
	Print,
	SetCursor
} Ssd1306Command;

typedef struct Ssd1306XY_t
{
	uint16_t x;
	uint16_t y;
} Ssd1306XY;

typedef union Ssd1306MessageData_t
{
	Ssd1306XY xy;
	char message[SSD1306_TEXT_ROWS * SSD1306_TEXT_COLS];
} Ssd1306MessageData;
	
typedef struct Ssd1306Message_t
{
	Ssd1306Command Command;
	Ssd1306MessageData Data;
} Ssd1306Message;

const int max_status_lines = SSD1306_TEXT_ROWS - 1;
extern I2C_HandleTypeDef hi2c1;

static osThreadId HeartbeatLedHandle = NULL;
static osThreadId BusyLedHandle = NULL;
static osThreadId ErrorLedHandle = NULL;
static osThreadId DisplayHandle = NULL;
static QueueHandle_t DisplayQueue;
static int LcdX = 0;
static int LcdY = 0;
static char ipAddr[16] = "              ";
volatile int BlinkCode = 0;

static void led_display_set_cursor(int x, int y);
static void HeartbeatLedThread(void const *argument);
static void BusyLedThread(void const *argument);
static void ErrorLedThread(void const *argument);
extern void DisplayLedThread(void const *argument);

void led_init()
{
	// D and I cache enabled?
	assert((SCB->CCR & SCB_CCR_DC_Msk) && (SCB->CCR & SCB_CCR_IC_Msk));
	
	osThreadDef(HeartbeatLed, HeartbeatLedThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadDef(BusyLed, BusyLedThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadDef(ErrorLed, ErrorLedThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadDef(Display, DisplayLedThread, osPriorityNormal, 0, 2 * configMINIMAL_STACK_SIZE);
		
	HeartbeatLedHandle = osThreadCreate(osThread(HeartbeatLed), NULL);
	BusyLedHandle = osThreadCreate(osThread(BusyLed), NULL);
	ErrorLedHandle = osThreadCreate(osThread(ErrorLed), NULL);
	
	DisplayQueue = xQueueCreate(1, sizeof(Ssd1306Message));
	DisplayHandle = osThreadCreate(osThread(Display), NULL);
}

void led_thinking_on()
{
	osThreadResume(BusyLedHandle);
}

void led_thinking_off()
{
	osThreadSuspend(BusyLedHandle);
}

void led_error(ErrorCode error)
{
	assert(error >= 0);
	osThreadSuspend(ErrorLedHandle);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
	BlinkCode = (int)error;
	if (error != ErrorCodeNone)
	{
		osThreadResume(ErrorLedHandle);	
	}
}

void led_display_clear()
{
	Ssd1306Message message = { Clear };
	if (pdTRUE != xQueueSend(DisplayQueue, &message, portMAX_DELAY))
	{
		dprintf(SSD1306 "clear failed\n");
	}
	
	led_display_set_ip(ipAddr);
}

void led_display_set_cursor(int x, int y)
{
	Ssd1306Message message = { SetCursor, x, y };
	if (pdTRUE != xQueueSend(DisplayQueue, &message, portMAX_DELAY))
	{
		dprintf(SSD1306 "set cursor failed\n");
	}
}

void led_display_set_ip(const char* text)
{
	led_display_set_cursor(0, 0);
	Ssd1306Message message;
	message.Command = Print;
	strncpy(message.Data.message, text, sizeof(message.Data.message));
	if (pdTRUE != xQueueSend(DisplayQueue, &message, portMAX_DELAY))
	{
		dprintf(SSD1306 "print failed\n");
	}
	
	strncpy(ipAddr, text, sizeof(ipAddr));
}

void led_display_set_message(const char* text)
{
	led_display_clear();
	led_display_set_cursor(0, 1);
	
	Ssd1306Message message;
	message.Command = Print;
	strncpy(message.Data.message, text, sizeof(message.Data.message));
	if (pdTRUE != xQueueSend(DisplayQueue, &message, portMAX_DELAY))
	{
		dprintf(SSD1306 "print failed\n");
	}
}

/**
  * @brief  Toggle LED1
  * @param  thread not used
  * @retval None
  */
static void HeartbeatLedThread(void const *argument)
{
	(void) argument;
  
	for (;;)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
		osDelay(500);
		
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
		osDelay(750);
	}
}

/**
  * @brief  Toggle LED2 thread
  * @param  argument not used
  * @retval None
  */
static void BusyLedThread(void const *argument)
{
	uint32_t count;
	(void) argument;
  
	for (;;)
	{
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
		osDelay(200);
	}
}

static void ErrorLedThread(void const *argument)
{
	int i;
	
	// Blink code is one second per blink between a 3 second delay
	for(;  ;)
	{
		int count = BlinkCode;
		if (count > 0)
		{
			
			for (i = 0; i < count; ++i)
			{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
				osDelay(500);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
				osDelay(750);
			}
		}
		
		osDelay(1000);
	}
}

void DisplayLedThread(void const *argument)
{
	const int address = SSD1306_ADDRESS;
	const int PxWidth = SSD1306_PIXEL_COLS;
	const int PxHeight = SSD1306_PIXEL_ROWS;
	const int TextWidth = SSD1306_TEXT_COLS;
	const int TextHeight = SSD1306_TEXT_ROWS;
	HAL_StatusTypeDef status;
	
	status = HAL_I2C_IsDeviceReady(&hi2c1, address, 1000, 10000);
	if (HAL_OK != status)
	{
		dprintf(SSD1306 "display not ready\n");
		goto exit;
	}
	
	SSD1306_COMMAND("display off", SSD1306_DISPLAYOFF);                     // 0xAE
	SSD1306_COMMAND("set clock div", SSD1306_SETDISPLAYCLOCKDIV, 0x80);     // 0xD5
	SSD1306_COMMAND("set multiplex", SSD1306_SETMULTIPLEX, 0x1f);                   // 0xA8
	SSD1306_COMMAND("set display offset", SSD1306_SETDISPLAYOFFSET, 0x0);               // 0xD3
	SSD1306_COMMAND("set start line", SSD1306_SETSTARTLINE);             // line #0
	SSD1306_COMMAND("set memory mode", SSD1306_MEMORYMODE, 0x00);                     // 0x20
	SSD1306_COMMAND("set remap", SSD1306_SEGREMAP | 0x1);

	// Set scan direction
	SSD1306_COMMAND("set scan direction", SSD1306_COMSCANDEC);
	SSD1306_COMMAND("set pins", SSD1306_SETCOMPINS, 0x02);                      // 0xDA
	SSD1306_COMMAND("set contrast", SSD1306_SETCONTRAST, 0x8f);                     // 0x81
	SSD1306_COMMAND("set precharge", SSD1306_SETPRECHARGE, 0xF1);                    // 0xd9
	SSD1306_COMMAND("set vcomdetect", SSD1306_SETVCOMDETECT, 0x40);                   // 0xDB
	SSD1306_COMMAND("normal display", SSD1306_NORMALDISPLAY);                   // 0xA6

	SSD1306_COMMAND("charge pump", SSD1306_CHARGEPUMP, 0x14);                      // 0x8D
	SSD1306_COMMAND("display on", SSD1306_DISPLAYON);
	
	dprintf(SSD1306 "waiting on display input for a %dx%d [%dx%d characters] display.\n", PxWidth, PxHeight, TextWidth, TextHeight);
	for (;;)
	{
		// Wait for input messages
		Ssd1306Message message;
		if (pdTRUE == xQueueReceive(DisplayQueue, &message, portMAX_DELAY))
		{
			switch (message.Command)
			{
			case Clear:
				{
					uint16_t i;
					SSD1306_COMMAND("set memory mode", SSD1306_MEMORYMODE, 0);
					SSD1306_COMMAND("set column start/end addr", SSD1306_COLUMNADDR, 0, PxWidth - 1);

					// Set page start and end address
					SSD1306_COMMAND("set page start/end addr", SSD1306_PAGEADDR, 0, PxHeight - 1);

					for (i = 0; i < PxWidth * (TextHeight - 1); i++)
					{
						SSD1306_DATA("clear", 0);
					} 
					
					LcdX = LcdY = 0;
				}
				break;
				
			case SetCursor:
				{
					LcdX = message.Data.xy.x;
					LcdY = message.Data.xy.y;
				}
				break;
				
			case Print:
				{
					size_t len = strlen(message.Data.message);
					
					for (size_t i = 0; i < len; ++i)
					{
						uint8_t *font_bitmap = (uint8_t *)&lcdfont[(int)(message.Data.message[i] * FONT_HEIGHT)];
						uint8_t i;
						uint8_t start = LcdX * FONT_WIDTH;

						// Set vertical addressing mode
						SSD1306_COMMAND("set memory mode", SSD1306_MEMORYMODE, 1);

						// Set column start and end address
						SSD1306_COMMAND("set column start and end", SSD1306_COLUMNADDR, start, start + FONT_HEIGHT);

						// Set page start and end address
						SSD1306_COMMAND("set page start and end", SSD1306_PAGEADDR, LcdY, LcdY);

						for (i = 0 ; i < FONT_HEIGHT ; i++)
						{
							SSD1306_DATA("print", font_bitmap[i]);
						}

						++LcdX;
						if (LcdX >= TextWidth)
						{
							++LcdY;
							LcdX = 0;
						}

						if (LcdY >= TextHeight)
						{
							LcdY = 0; 	
						}
					}
				}
				break;
			}
		}
	}
	
exit:
	dprintf(SSD1306 "exiting DisplayLedThread. HAL status: %s [%x]\n", rtos_hal_status(status), status);
	
	for (;;)
	{
		osThreadTerminate(NULL);
	}
}