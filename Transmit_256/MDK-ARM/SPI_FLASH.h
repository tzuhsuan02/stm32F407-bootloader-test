#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H
#include "stm32f4xx_hal.h"

#define WEL           0x02
#define BUSY          0x01

void SPI_Read_JEDECID(uint8_t *pData);
void SPI_ReadReg(uint8_t *Reg1);
void Write_Enable();
uint8_t Erase_Sector(uint32_t Addr);
void Read_Flash_Data(uint32_t Addr,uint8_t *Buff,uint16_t Size);
uint8_t Page_Program(uint32_t Addr,uint8_t *Buff,uint32_t Size);
uint8_t Sectors_Erase(uint32_t AddrR,uint32_t Size);
//uint8_t Send_FLASH_Buffer(uint32_t Addr,uint32_t Size,uint8_t *Buffer);
#endif



