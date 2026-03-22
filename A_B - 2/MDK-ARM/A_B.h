#ifndef __A_B_H
#define __A_B_H
#include "stm32f4xx_hal.h"

#define FLASH_A_ADDR  0x100000
#define FLASH_B_ADDR  0x200000

uint8_t DOWNLOAD_To_A(void);
uint8_t DOWNLOAD_To_B(void);
void Read_A_To_Internal(void);
void Read_B_To_Internal(void);
#endif
