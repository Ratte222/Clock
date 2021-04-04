#include "i2c.h"

//��������������

uint8_t I2C_WriteBuffer(I2C_HandleTypeDef *hi, uint8_t DEV_ADDR, uint8_t sizebuf, uint8_t *TxBuffer, uint32_t Timeout)
{
	uint8_t attempts = 0;
	while(HAL_I2C_Master_Transmit(hi, (uint16_t)DEV_ADDR, TxBuffer, (uint16_t)sizebuf, Timeout)!= HAL_OK)
	{
		if ((HAL_I2C_GetError(hi) != HAL_I2C_ERROR_AF)||(attempts++ > 30))
		{
			return 0;
		}
	}
	return 1;
}

//��������������

uint8_t I2C_ReadBuffer(I2C_HandleTypeDef *hi, uint8_t DEV_ADDR, uint8_t sizebuf, uint8_t *TxBuffer, uint32_t Timeout)
{
	uint8_t attempts = 0;
	while(HAL_I2C_Master_Receive(hi, (uint16_t)DEV_ADDR, TxBuffer, (uint16_t)sizebuf, Timeout)!= HAL_OK)
	{
		if ((HAL_I2C_GetError(hi) != HAL_I2C_ERROR_AF)||(attempts++ > 30))
		{
			return 0;
		}		
	}
return 1;
}

uint8_t I2C_WriteBuffer_(I2C_str *val, uint8_t sizebuf, uint8_t *TxBuffer)
{
	uint8_t attempts = 0;
	while(HAL_I2C_Master_Transmit(val->hi, (uint16_t)val->DEV_ADDR, TxBuffer, (uint16_t)sizebuf, val->txTimeout)!= HAL_OK)
	{
		if ((HAL_I2C_GetError(val->hi) != HAL_I2C_ERROR_AF)||(attempts++ > 30))
		{
			return 0;
		}
	}
	return 1;
}

uint8_t I2C_ReadBuffer_(I2C_str val, uint8_t sizebuf, uint8_t *RxBuffer)
{
	// uint8_t attempts = 0;
	// while(HAL_I2C_Master_Receive(&val.hi, (uint16_t)val.DEV_ADDR, RxBuffer, (uint16_t)sizebuf, val.rxTimeout)!= HAL_OK)
	// {
	// 	if ((HAL_I2C_GetError(&val.hi) != HAL_I2C_ERROR_AF)||(attempts++ > 30))
	// 	{
	// 		return 0;
	// 	}		
	// }
return 1;
}
