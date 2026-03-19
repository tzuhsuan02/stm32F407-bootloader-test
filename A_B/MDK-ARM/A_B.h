#ifndef __A_B_H
#define __A_B_H
#include "stm32f4xx_hal.h"

#define FLASH_A_ADDR  0x100000
#define FLASH_B_ADDR  0x200000

uint8_t Write_To_A();
uint8_t Write_To_B();
void Read_A();
void Read_B();
#endif
