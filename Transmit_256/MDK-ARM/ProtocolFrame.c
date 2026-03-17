#include "ProtocolFrame.h"

	Frame_t Frame={0};
extern UART_HandleTypeDef huart1;
	
unsigned short crc(unsigned char *data,uint16_t len)
{
    unsigned char temp = 0;
    unsigned short buff = 0xffff;  // 初始值
    unsigned char i = 0, j = 0;
    
     for(uint16_t i = 0; i < len; i++)   // 遍历每个字节
    {
        buff ^= data[i];  // 当前字节与CRC寄存器低8位异或
        
        for(j = 0; j < 8; j++)  // 处理每个bit
        {
            temp = buff & 0x0001;  // 取最低位
            
            if(temp){  // 如果最低位为1
                buff >>= 1;      // 右移1位
                buff ^= 0xa001;  // 与多项式异或
            }else{  // 如果最低位为0
                buff >>= 1;      // 只右移1位
            }
        }
    }
    
    buff = ((buff & 0x00FF) << 8) | ((buff & 0xFF00) >> 8);  // 高低8位交换
    
    return buff;  // 返回CRC值
}


//uint8_t Parse_Frame(uint8_t *rx_buf,uint16_t CRC_Value)
//{
//	uint8_t rx_buf_length=sizeof(rx_buf)/sizeof(rx_buf[0]);
//	uint8_t CRC_Buff[rx_buf_length-4];
//	uint16_t CRC_Val;
//	
//    Frame.head[0]=rx_buf[0];
//   	Frame.head[1]=rx_buf[1];	
//	
//	  Frame.cmd[0]=rx_buf[2];
//  	Frame.cmd[1]=rx_buf[3];
//	
//	  Frame.length[0]=rx_buf[4];
//		Frame.length[1]=rx_buf[5];
//	
//		for(uint8_t i=6;i<rx_buf_length-6;i++)
//	{
//		uint8_t j=0;
//	Frame.data[j]=rx_buf[i];
//		j++;
//		
//	}
//	
//	Frame.reserved[0]=rx_buf[rx_buf_length-5];
//	Frame.reserved[1]=rx_buf[rx_buf_length-4];
//	Frame.reserved[2]=rx_buf[rx_buf_length-3];
//	Frame.reserved[3]=rx_buf[rx_buf_length-2];
//	
//	Frame.crc=(rx_buf[rx_buf_length-1]<<8)|rx_buf[rx_buf_length];

//// 1. 检查包头	
//	if(Frame.head[0]!=0xAA)
//	{
//	return 1;
//	}
//	
//	if(Frame.head[1]!=0xBB)
//	{
//	return 2;
//	}
//    
//	for(uint8_t i=0;i<rx_buf_length-4;i++)
//	{
//	CRC_Buff[i]=rx_buf[i+2];
//	}
//    // 2. 计算 CRC（从 cmd 到 reserved）
//    CRC_Val=crc(CRC_Buff,rx_buf_length-4);
//	
//    // 3. 比较 CRC
//    if(CRC_Val==CRC_Value)
//    // 4. 返回结果
//		{
//		return 0;
//		}
//		return 3;
//}

uint8_t Parse_Frame(uint8_t *rx_buf)
{
	Frame_t *frame = (Frame_t *)rx_buf;

	uint16_t CRC_Val;
	uint16_t CRC_Value;//发送端发来的CRC

//// 打印收到的包头
//    char debug1[50];
//    sprintf(debug1, "head=%02X %02X\r\n", frame->head[0], frame->head[1]);
//    HAL_UART_Transmit(&huart1, (uint8_t*)debug1, strlen(debug1), 100);	

	// 1. 检查包头	
	if(frame->head[0] != 0xAA || frame->head[1] != 0xBB)
{
    return 1;
}
 
  CRC_Value=(frame->crc[0] << 8) | frame->crc[1];

    // 2. 计算 CRC（从 cmd 到 reserved）
		
      CRC_Val = crc(frame->cmd, CRC_LEN);
	
    // 3. 比较 CRC
    if(CRC_Val==CRC_Value)
    // 4. 返回结果
		{
		return 0;
		}
		return 2;
}
//uint16_t calc_crc = crc(frame->cmd, CRC_LEN);
//    uint16_t recv_crc = (frame->crc[0] << 8) | frame->crc[1];
//    
//    // 打印调试
//    char debug[50];
//    sprintf(debug, "calc=%04X recv=%04X\r\n", calc_crc, recv_crc);
//    HAL_UART_Transmit(&huart1, (uint8_t*)debug, strlen(debug), 100);
//    
//    if(calc_crc == recv_crc)
//    {
//        return 0;
//    }
//    return 2;
//}
