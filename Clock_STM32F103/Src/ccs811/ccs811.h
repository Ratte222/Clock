#ifndef __CCS811_H
#define __CCS811_H

#ifdef _cplusplus
extern "C" {
#endif

#include "..\i2c\i2c.h"

#define I2C_Address_When_ADDR_LOW 0x5A//default
#define I2C_Address_When_ADDR_HIGH 0x5B
typedef struct
{
  I2C_str i2c;
	uint16_t temp;
	uint16_t humin;
}CCS811_srt;


#ifdef _cplusplus
}
#endif
#endif