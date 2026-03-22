#ifndef __FLASH_TRANSMIT
#define __FLASH_TRANSMIT
#include "stm32f4xx_hal.h"

#define INTERNAL_FLASH_ADDR 0x08008000;//内部FLASH读取数据存放的地址
void Write_Internal_Flash(uint32_t Addr,uint32_t Bin_Size);

#endif
