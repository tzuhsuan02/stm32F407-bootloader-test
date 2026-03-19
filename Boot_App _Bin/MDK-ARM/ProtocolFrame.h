#ifndef __PROTOCOLFRAME_H
#define __PROTOCOLFRAME_H
#include "stm32f4xx_hal.h"
#include "SPI_FLASH.h"

#include <stdio.h>
#include <string.h>

#define DATA_SIZE  256
#define CRC_LEN    (2 + 2 + DATA_SIZE + 4) 
#define FlAG_FLASH_ADDR   0x000000  //标志区起始地址
#define DOWNLOAD_FLASH_ADDR   0xFFF000  //固件存放的起始地址
// 协议帧结构
typedef struct {
    uint8_t head[2];      // 包头 2字节
	  uint8_t cmd[2];				// 命令2字节
    uint8_t length[2];    // 数据长度 2字节
    uint8_t  data[256]; 	// 数据
    uint8_t  reserved[4]; // 保留 4字节
    uint8_t crc[2];         // CRC16
} Frame_t;

//标志区
typedef struct {
    uint32_t bin_size;      // 固件大小
    uint8_t upgrade_flag;  // 是否需要升级
    uint8_t active_app;     // 现在运行的是A/B区
		uint8_t valid_app;    //APP 是否有效
	
} Firmware_Info;

uint8_t Parse_Frame(uint8_t *rx_buf);
uint32_t Get_Bin_Size(uint8_t *rx_buf);
void Write_Firmware_Info(uint32_t Addr, Firmware_Info *info);
void Read_Firmware_Info(uint32_t Addr, Firmware_Info *info);
#endif
