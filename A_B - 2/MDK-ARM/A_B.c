#include "A_B.h"
#include "ProtocolFrame.h"
#include "SPI_FLASH.h"
#include "FLASH_Transmit.h"

uint8_t DOWNLOAD_To_A(void)
{
	
		Firmware_Info Info={0};
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
		
			Info.a_bin_size=Info.bin_size;
		Write_Firmware_Info(FlAG_FLASH_ADDR,&Info);
		
    return 0;
}
	
uint8_t DOWNLOAD_To_B(void)
{
		
Firmware_Info Info={0};
	 Read_Firmware_Info(FlAG_FLASH_ADDR,&Info);
	
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
		
		  Info.b_bin_size=Info.bin_size;
			Write_Firmware_Info(FlAG_FLASH_ADDR,&Info);
    return 0;

}	
	
//A/B区 -> 内部Flash
void Read_A_To_Internal()
{
	Firmware_Info Info={0};
	Read_Firmware_Info(FlAG_FLASH_ADDR,&Info);
	
	//A区数据写入内部FLASH
	Write_Internal_Flash(FLASH_A_ADDR,Info.a_bin_size);
}

void Read_B_To_Internal(void)
{
		Firmware_Info Info={0};
	Read_Firmware_Info(FlAG_FLASH_ADDR,&Info);
		
	
	//B区数据写入内部FLASH
	Write_Internal_Flash(FLASH_B_ADDR,Info.b_bin_size);


}
