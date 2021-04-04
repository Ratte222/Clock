#ifndef __I2C_H
#define __I2C_H

#ifdef _cplusplus
extern "C" {
#endif

#include "main.h"
//#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct
{
  I2C_HandleTypeDef *hi;
	uint8_t DEV_ADDR;
	uint32_t txTimeout;
	uint32_t rxTimeout;
}I2C_str;

uint8_t I2C_WriteBuffer(I2C_HandleTypeDef *hi, uint8_t DEV_ADDR, uint8_t sizebuf, uint8_t *TxBuffer, uint32_t Timeout);
uint8_t I2C_ReadBuffer(I2C_HandleTypeDef *hi, uint8_t DEV_ADDR, uint8_t sizebuf, uint8_t *RxBuffer, uint32_t Timeout);
uint8_t I2C_WriteBuffer_(I2C_str *val, uint8_t sizebuf, uint8_t *TxBuffer);
uint8_t I2C_ReadBuffer_(I2C_str val, uint8_t sizebuf, uint8_t *RxBuffer);
#ifdef _cplusplus
}
#endif
#endif
