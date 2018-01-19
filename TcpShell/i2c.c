// I2C callbacks
#include <FreeRTOS.h>
#include <stm32f7xx_hal_i2c.h>
#include <assert.h>
#include <stdbool.h>
#include <semphr.h>
#include "i2c.h"
#include "tcpshell.h"

extern I2C_HandleTypeDef hi2c1;
i2c i2c1 = { };

#define HSTOIS(X) (I2CStatus)(X)

static void i2c_init_for_device(pi2c i2c, I2C_HandleTypeDef* hi2c);

void i2c_init(void)
{
	// D and I cache enabled?
	assert((SCB->CCR & SCB_CCR_DC_Msk) && (SCB->CCR & SCB_CCR_IC_Msk));
	
	/* Init the I2C2 for the OLED display so we can show our dang hostname and IP addr. */
	i2c_init_for_device(&i2c1, &hi2c1);
	if (HAL_OK != HAL_I2C_IsDeviceReady(i2c1.phandle, SSD1306_I2C_ADDRESS, 1000, i2c1.timeout))
	{
		i2c1.status = I2CStatusError;
	}
}

void i2c_master_transmit(pi2c pi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size)
{
	assert_if(pi2c && pi2c->status == I2CStatusOK)
	{
		pi2c->status = I2CStatusBusy;
		pi2c->status = HSTOIS(HAL_I2C_Master_Transmit(pi2c->phandle, DevAddress, pData, Size, pi2c->timeout));
	}
}

void i2c_master_receive(pi2c pi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size)
{
	assert_if(pi2c && pi2c->status == I2CStatusOK)
	{
		pi2c->status = I2CStatusBusy;
		pi2c->status = HSTOIS(HAL_I2C_Master_Receive(pi2c->phandle, DevAddress, pData, Size, pi2c->timeout));
	}
}

void i2c_slave_transmit(pi2c pi2c, uint8_t *pData, uint16_t Size)
{
	assert_if(pi2c && pi2c->status == I2CStatusOK)
	{
		pi2c->status = I2CStatusBusy;
		pi2c->status = HSTOIS(HAL_I2C_Slave_Transmit(pi2c->phandle, pData, Size, pi2c->timeout));
	}
}

void i2c_slave_receive(pi2c pi2c, uint8_t *pData, uint16_t Size)
{
	assert_if(pi2c && pi2c->status == I2CStatusOK)
	{
		pi2c->status = I2CStatusBusy;
		pi2c->status = HSTOIS(HAL_I2C_Slave_Receive(pi2c->phandle, pData, Size, pi2c->timeout));
	}
}

void i2c_mem_write(pi2c pi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size)
{
	assert_if(pi2c && pi2c->status == I2CStatusOK)
	{
		pi2c->status = I2CStatusBusy;
		pi2c->status = HSTOIS(HAL_I2C_Mem_Write(pi2c->phandle, DevAddress, MemAddress, MemAddSize, pData, Size, pi2c->timeout));
	}
}

void i2c_mem_read(pi2c pi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size)
{
	assert_if(pi2c && pi2c->status == I2CStatusOK)
	{
		pi2c->status = I2CStatusBusy;
		pi2c->status = HSTOIS(HAL_I2C_Mem_Read(pi2c->phandle, DevAddress, MemAddress, MemAddSize, pData, Size, pi2c->timeout));
	}
}

static void i2c_init_for_device(pi2c i2c, I2C_HandleTypeDef* hi2c)
{
	i2c1.phandle = &hi2c1;
	i2c1.timeout = 10000 / portTICK_PERIOD_MS; 
	
}