#include "FLASH_Transmit.h"
#include "ProtocolFrame.h"

extern UART_HandleTypeDef huart1;
void Write_Internal_Flash(void)
{
	Firmware_Info Info;
	 Read_Firmware_Info(FlAG_FLASH_ADDR,&Info);
	
    uint32_t Page_Num = Info.bin_size / 256;
    uint32_t Last_Data = Info.bin_size % 256;
	 uint8_t Buffer[256];
	
    // 定义 Flash 起始地址（0x08008000），该地址为目标 Flash 存储区的起始位置
    uint32_t Inter_Addr = INTERNAL_FLASH_ADDR;
	 
	  //外部FLASH地址
	  uint32_t W25Q_Addr=W25Q_FLASH_Addr;
    
    // 错误标志变量
    uint32_t error = 0;
    
    // 定义擦除配置结构体
    FLASH_EraseInitTypeDef flash_dat;
    
    // 解锁 Flash 控制寄存器，准备执行写入操作
    HAL_FLASH_Unlock();
    
    // 配置擦除参数，指定要擦除的类型和扇区
	  flash_dat.Banks=FLASH_BANK_1;
		flash_dat.TypeErase=FLASH_TYPEERASE_SECTORS;
		flash_dat.Sector=FLASH_SECTOR_2;
		flash_dat.NbSectors=1;
		//flash_dat.VoltageRange

    // 擦除指定的 Flash 扇区
    HAL_FLASHEx_Erase(&flash_dat, &error);
   
	 for(uint16_t i=0;i<Page_Num;i++)
	 {
	 Read_Flash_Data(W25Q_Addr,Buffer,256);
		 
		 for(uint16_t j=0;j<256;j++)
		 {
		 HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,Inter_Addr,Buffer[j]);
			 Inter_Addr++;
		 }
		 	 W25Q_Addr+=256;
	 }
	 
	 if(Last_Data>0)
	 {
		 Read_Flash_Data(W25Q_Addr,Buffer,Last_Data);
		 
			 for(uint8_t j=0;j<Last_Data;j++) 
			 {
			 HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,Inter_Addr,Buffer[j]);
			 Inter_Addr++; 
			 }
	 }
    
    // 完成所有写入操作后，锁定 Flash 控制寄存器，防止进一步修改
    HAL_FLASH_Lock();
	 
	 // 新固件数据写入，升级完成，清除标志
  Info.upgrade_flag = 0;
  Info.active_app = 1;
  Write_Firmware_Info(FlAG_FLASH_ADDR, &Info);
	 
	 HAL_UART_Transmit(&huart1, (uint8_t*)"Internal FLASH Upgrade Done\r\n", 29, 100);
	}
