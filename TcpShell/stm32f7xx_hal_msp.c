/**
  ******************************************************************************
  * File Name          : stm32f7xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization 
  *                      and de-Initialization codes.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

extern void _Error_Handler(char *, int);
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
	/* USER CODE BEGIN MspInit 0 */

	/* USER CODE END MspInit 0 */

	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	/* System interrupt init*/
	/* MemoryManagement_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
	/* BusFault_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
	/* UsageFault_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
	/* SVCall_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SVCall_IRQn, 0, 0);
	/* DebugMonitor_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DebugMonitor_IRQn, 0, 0);
	/* PendSV_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(PendSV_IRQn, 0, 0);
	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

	/* USER CODE BEGIN MspInit 1 */

	/* USER CODE END MspInit 1 */
}
	
void HAL_ETH_MspInit(ETH_HandleTypeDef* heth)
{

	GPIO_InitTypeDef GPIO_InitStruct;
	if (heth->Instance == ETH)
	{
		/* USER CODE BEGIN ETH_MspInit 0 */

		/* USER CODE END ETH_MspInit 0 */
		  /* Peripheral clock enable */
		__HAL_RCC_ETH_CLK_ENABLE();
  
		/**ETH GPIO Configuration    
		PC1     ------> ETH_MDC
		PA1     ------> ETH_REF_CLK
		PA2     ------> ETH_MDIO
		PA7     ------> ETH_CRS_DV
		PC4     ------> ETH_RXD0
		PC5     ------> ETH_RXD1
		PB13     ------> ETH_TXD1
		PG11     ------> ETH_TX_EN
		PG13     ------> ETH_TXD0 
		*/
		GPIO_InitStruct.Pin = RMII_MDC_Pin | RMII_RXD0_Pin | RMII_RXD1_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = RMII_REF_CLK_Pin | RMII_MDIO_Pin | RMII_CRS_DV_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = RMII_TXD1_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(RMII_TXD1_GPIO_Port, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = RMII_TX_EN_Pin | RMII_TXD0_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

		/* USER CODE BEGIN ETH_MspInit 1 */
  
		/* Enable the Ethernet global Interrupt */
		// Eth driver uses systick priority so we adjust this to be 1 more than systick priority
		HAL_NVIC_SetPriority(ETH_IRQn, TICK_INT_PRIORITY + 1, 0);
		HAL_NVIC_EnableIRQ(ETH_IRQn);

		/* USER CODE END ETH_MspInit 1 */
	}

}

void HAL_ETH_MspDeInit(ETH_HandleTypeDef* heth)
{

	if (heth->Instance == ETH)
	{
		/* USER CODE BEGIN ETH_MspDeInit 0 */

		/* USER CODE END ETH_MspDeInit 0 */
		  /* Peripheral clock disable */
		__HAL_RCC_ETH_CLK_DISABLE();
  
		/**ETH GPIO Configuration    
		PC1     ------> ETH_MDC
		PA1     ------> ETH_REF_CLK
		PA2     ------> ETH_MDIO
		PA7     ------> ETH_CRS_DV
		PC4     ------> ETH_RXD0
		PC5     ------> ETH_RXD1
		PB13     ------> ETH_TXD1
		PG11     ------> ETH_TX_EN
		PG13     ------> ETH_TXD0 
		*/
		HAL_GPIO_DeInit(GPIOC, RMII_MDC_Pin | RMII_RXD0_Pin | RMII_RXD1_Pin);

		HAL_GPIO_DeInit(GPIOA, RMII_REF_CLK_Pin | RMII_MDIO_Pin | RMII_CRS_DV_Pin);

		HAL_GPIO_DeInit(RMII_TXD1_GPIO_Port, RMII_TXD1_Pin);

		HAL_GPIO_DeInit(GPIOG, RMII_TX_EN_Pin | RMII_TXD0_Pin);

		/* USER CODE BEGIN ETH_MspDeInit 1 */

		/* USER CODE END ETH_MspDeInit 1 */
	}

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{

	GPIO_InitTypeDef GPIO_InitStruct;
	if (hi2c->Instance == I2C1)
	{
		/* USER CODE BEGIN I2C1_MspInit 0 */

		/* USER CODE END I2C1_MspInit 0 */
  
		  /**I2C1 GPIO Configuration    
		  PB6     ------> I2C1_SCL
		  PB9     ------> I2C1_SDA 
		  */
		GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/* Peripheral clock enable */
		__HAL_RCC_I2C1_CLK_ENABLE();
		/* I2C1 interrupt Init */
		HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
		HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
		/* USER CODE BEGIN I2C1_MspInit 1 */

		/* USER CODE END I2C1_MspInit 1 */
	}

}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{

	if (hi2c->Instance == I2C1)
	{
		/* USER CODE BEGIN I2C1_MspDeInit 0 */

		/* USER CODE END I2C1_MspDeInit 0 */
		  /* Peripheral clock disable */
		__HAL_RCC_I2C1_CLK_DISABLE();
  
		/**I2C1 GPIO Configuration    
		PB6     ------> I2C1_SCL
		PB9     ------> I2C1_SDA 
		*/
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_9);

		/* I2C1 interrupt DeInit */
		HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
		HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
		/* USER CODE BEGIN I2C1_MspDeInit 1 */

		/* USER CODE END I2C1_MspDeInit 1 */
	}

}

void HAL_RNG_MspInit(RNG_HandleTypeDef* hrng)
{

	if (hrng->Instance == RNG)
	{
		/* USER CODE BEGIN RNG_MspInit 0 */

		/* USER CODE END RNG_MspInit 0 */
		  /* Peripheral clock enable */
		__HAL_RCC_RNG_CLK_ENABLE();
		/* USER CODE BEGIN RNG_MspInit 1 */

		/* USER CODE END RNG_MspInit 1 */
	}

}

void HAL_RNG_MspDeInit(RNG_HandleTypeDef* hrng)
{

	if (hrng->Instance == RNG)
	{
		/* USER CODE BEGIN RNG_MspDeInit 0 */

		/* USER CODE END RNG_MspDeInit 0 */
		  /* Peripheral clock disable */
		__HAL_RCC_RNG_CLK_DISABLE();
		/* USER CODE BEGIN RNG_MspDeInit 1 */

		/* USER CODE END RNG_MspDeInit 1 */
	}

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{

	GPIO_InitTypeDef GPIO_InitStruct;
	if (hrtc->Instance == RTC)
	{
		/* USER CODE BEGIN RTC_MspInit 0 */

		/* USER CODE END RTC_MspInit 0 */
		  /* Peripheral clock enable */
		__HAL_RCC_RTC_ENABLE();
  
		/**RTC GPIO Configuration    
		PB15     ------> RTC_REFIN 
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_15;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF0_RTC_50Hz;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/* USER CODE BEGIN RTC_MspInit 1 */

		/* USER CODE END RTC_MspInit 1 */
	}

}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc)
{

	if (hrtc->Instance == RTC)
	{
		/* USER CODE BEGIN RTC_MspDeInit 0 */

		/* USER CODE END RTC_MspDeInit 0 */
		  /* Peripheral clock disable */
		__HAL_RCC_RTC_DISABLE();
  
		/**RTC GPIO Configuration    
		PB15     ------> RTC_REFIN 
		*/
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_15);

		/* USER CODE BEGIN RTC_MspDeInit 1 */

		/* USER CODE END RTC_MspDeInit 1 */
	}

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
