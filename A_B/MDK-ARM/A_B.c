#include "A_B.h"
#include "ProtocolFrame.h"
#include "SPI_FLASH.h"
#include "FLASH_Transmit.h"

uint8_t Write_To_A()
{
		Firmware_Info Info;
	 Read_Firmware_Info(FlAG_FLASH_ADDR,&Info);
	 	 
	uint8_t Status;
	 uint32_t Page_Num = Info.bin_size / 256;
   uint32_t Last_Data = Info.bin_size % 256;
	 uint8_t Buffer[256];
	
	uint32_t src_addr = DOWNLOAD_FLASH_ADDR;  // 从下载区读
    uint32_t dst_addr = FLASH_A_ADDR;         // 写到A区
	
	Status=Sectors_Erase(FLASH_A_ADDR,Info.bin_size);
	
   if(Status != 0) return 1;
	
 for(uint32_t i = 0; i < Page_Num; i++)
    {
        Read_Flash_Data(src_addr, Buffer, 256);
        Page_Program(dst_addr, Buffer, 256);
        src_addr += 256;
        dst_addr += 256;
    }
    
    if(Last_Data > 0)
    {
        Read_Flash_Data(src_addr, Buffer, Last_Data);
        Page_Program(dst_addr, Buffer, Last_Data);
    }
    Write_Firmware_Info(FlAG_FLASH_ADDR, &Info);
		
    return 0;
}
	
uint8_t Write_To_B()
{
Firmware_Info Info;
	 Read_Firmware_Info(FlAG_FLASH_ADDR,&Info);
	
  Info.active_app=0x0B;
	 uint8_t Status;
	 uint32_t Page_Num = Info.bin_size / 256;
   uint32_t Last_Data = Info.bin_size % 256;
	 uint8_t Buffer[256];
	
	uint32_t src_addr = DOWNLOAD_FLASH_ADDR;  // 从下载区读
    uint32_t dst_addr = FLASH_B_ADDR;    // 写到B区
	
	Status=Sectors_Erase(FLASH_B_ADDR,Info.bin_size);
	
   if(Status != 0) return 1;
	
 for(uint32_t i = 0; i < Page_Num; i++)
    {
        Read_Flash_Data(src_addr, Buffer, 256);
        Page_Program(dst_addr, Buffer, 256);
        src_addr += 256;
        dst_addr += 256;
    }
    
    if(Last_Data > 0)
    {
        Read_Flash_Data(src_addr, Buffer, Last_Data);
        Page_Program(dst_addr, Buffer, Last_Data);
    }
    
				Write_Firmware_Info(FlAG_FLASH_ADDR, &Info);
    return 0;

}	
	
void Read_A()
{
   Firmware_Info Info;
	 Read_Firmware_Info(FlAG_FLASH_ADDR,&Info);
	
  Info.active_app=0x0A;
	
	uint32_t src_addr = FLASH_A_ADDR;  // 从A区读
	
	//写入内部FLASH
	Write_Internal_Flash(src_addr);
	//写入标志位
	Write_Firmware_Info(FlAG_FLASH_ADDR, &Info);
}

void Read_B()
{
   Firmware_Info Info;
	 Read_Firmware_Info(FlAG_FLASH_ADDR,&Info);
	
  Info.active_app=0x0B;
	
	uint32_t src_addr = FLASH_B_ADDR;  // 从B区读
	
	//写入内部FLASH
	Write_Internal_Flash(src_addr);
	//写入标志位
	Write_Firmware_Info(FlAG_FLASH_ADDR, &Info);

}
