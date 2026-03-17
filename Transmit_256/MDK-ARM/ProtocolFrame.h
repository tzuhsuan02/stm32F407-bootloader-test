#ifndef __PROTOCOLFRAME_H
#define __PROTOCOLFRAME_H
#include "stm32f4xx_hal.h"

#include <stdio.h>
#include <string.h>

#define DATA_SIZE  256
#define CRC_LEN    (2 + 2 + DATA_SIZE + 4) 

// 协议帧结构
typedef struct {
    uint8_t head[2];      // 包头 2字节
	  uint8_t cmd[2];				// 命令2字节
    uint8_t length[2];    // 数据长度 2字节
    uint8_t  data[256]; 	// 数据
    uint8_t  reserved[4]; // 保留 4字节
    uint8_t crc[2];         // CRC16
} Frame_t;

#endif
