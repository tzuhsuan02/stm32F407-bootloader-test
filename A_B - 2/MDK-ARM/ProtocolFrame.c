#include "ProtocolFrame.h"

extern UART_HandleTypeDef huart1;

	extern void Uart_SendStr(char *str);
extern void Uart_Printf(const char *fmt, ...);

unsigned short crc(unsigned char *data,uint16_t len)
{
    unsigned char temp = 0;
    unsigned short buff = 0xffff;  // 初始值
    unsigned char  j = 0;
    
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

//协议帧
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

//写标志区
void Write_Firmware_Info(uint32_t Addr, Firmware_Info *info)
{
Firmware_Info verify = {0};


    Sectors_Erase(Addr, sizeof(Firmware_Info));
    
    // 把结构体当作字节数组写入
    Page_Program(Addr, (uint8_t *)info, sizeof(Firmware_Info));

  Read_Firmware_Info(Addr, &verify);

	Uart_Printf("WR size:%08lX flag:%d active:%02X valid:%d res:%02X\r\n",
                info->bin_size, info->upgrade_flag, info->active_app,
                info->valid_app, info->reserved);

    Uart_Printf("RD size:%08lX flag:%d active:%02X valid:%d res:%02X\r\n",
                verify.bin_size, verify.upgrade_flag, verify.active_app,
                verify.valid_app, verify.reserved);
}
//读标志区
void Read_Firmware_Info(uint32_t Addr, Firmware_Info *info)
{
    Read_Flash_Data(Addr, (uint8_t *)info, sizeof(Firmware_Info));
}

void Init_Firmware_Info(void)
{
    Firmware_Info Info={0};
		Info.magic=INFO_MAGIC;
    Info.bin_size = 0;
		Info.a_bin_size=0;
		Info.b_bin_size=0;
    Info.upgrade_flag = 0;
    Info.valid_app = 1;
    Info.active_app = 0x0A;
		Info.reserved=0;
    Write_Firmware_Info(FlAG_FLASH_ADDR, &Info);
    
    HAL_UART_Transmit(&huart1, (uint8_t*)"Info Init Done\r\n", 16, 100);
}
