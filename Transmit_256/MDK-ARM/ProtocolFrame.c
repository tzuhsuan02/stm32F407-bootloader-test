#include "ProtocolFrame.h"

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


uint8_t Parse_Frame(uint8_t *rx_buf)
{
	Frame_t *frame = (Frame_t *)rx_buf;

	uint16_t CRC_Val;
	uint16_t CRC_Value;//发送端发来的CRC


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
uint32_t Get_Bin_Size(uint8_t *rx_buf)
{
    Frame_t *frame = (Frame_t *)rx_buf;
	
	  uint16_t Cmd=(frame->cmd[0] << 8) | frame->cmd[1];
	
    uint32_t size = (frame->data[0] << 24) | (frame->data[1] << 16) | 
                    (frame->data[2] << 8) | frame->data[3];
    return size;
}

void Write_Firmware_Info(uint32_t Addr, Firmware_Info *info)
{
    Sectors_Erase(Addr, sizeof(Firmware_Info));
    
    // 把结构体当作字节数组写入
    Page_Program(Addr, (uint8_t *)info, sizeof(Firmware_Info));
}

void Read_Firmware_Info(uint32_t Addr, Firmware_Info *info)
{
    Read_Flash_Data(Addr, (uint8_t *)info, sizeof(Firmware_Info));
}
