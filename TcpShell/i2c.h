#pragma once

#ifdef __CPLUSPLUS
extern "C" {
#endif // __CPLUSPLUS

// I2C -> RTOS functionality

#include <stdint.h>

// Ok I don't know... even though I asked for 7bit I2C the MCU treats the 8 bit value as a left justified 7 bit value.
// This seems counter intuitive to me?
#define SSD1306_I2C_ADDRESS   (0x3C << 1)

typedef enum 
{
	I2CStatusOK      = 0x00U,
	I2CStatusError   = 0x01U,
	I2CStatusBusy    = 0x02U,
	I2CStatusTimeout = 0x03U,
	I2CStatusAbort   = 0x04U
} I2CStatus;

typedef struct i2c_t
{
	I2C_HandleTypeDef* phandle;
	I2CStatus status;
	int timeout;
	uint8_t transfer_direction;
	uint16_t addr_match_code;
} i2c, *pi2c;

extern i2c i2c1;

void i2c_init(void);
void i2c_master_transmit(pi2c pi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
void i2c_master_receive(pi2c pi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
void i2c_slave_transmit(pi2c pi2c, uint8_t *pData, uint16_t Size);
void i2c_slave_receive(pi2c pi2c, uint8_t *pData, uint16_t Size);
void i2c_mem_write(pi2c pi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
void i2c_mem_read(pi2c pi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);

#ifdef __CPLUSPLUS
}
#endif // __CPLUSPLUS

