#include "SPI_FLASH.h"

extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart1;
const uint8_t JEDECID[4]= {0x9F,0xFF,0xFF,0xFF};
const uint8_t ReadSR1=0x05;
const uint8_t Dummy=0xFF;
const uint8_t PageProgram=0x02;
const uint8_t WriteEnable=0x06;
const uint8_t SectorErase=0x20;


void SPI_Read_JEDECID(uint8_t *pData)
{
	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_6,GPIO_PIN_RESET);

	HAL_SPI_TransmitReceive(&hspi1,JEDECID,pData,4,HAL_MAX_DELAY);
	
	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_6,GPIO_PIN_SET);

  HAL_UART_Transmit(&huart1,pData,4,HAL_MAX_DELAY);
}

void SPI_ReadReg(uint8_t *Reg)
{
	uint8_t RegCmd[]={0x05,0xFF};
	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_6,GPIO_PIN_RESET);

	HAL_SPI_TransmitReceive(&hspi1,RegCmd,Reg,2,HAL_MAX_DELAY);

	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_6,GPIO_PIN_SET); 
}

void  Write_Enable()
{
	uint8_t Reg[2];
	
 do {
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_SPI_Transmit(&hspi1, &WriteEnable, 1, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_SET);
        
        SPI_ReadReg(Reg);
    } while(!(Reg[1] & WEL));
	
}
void Wait_BUSY()
{
		uint8_t Reg[2];
		//Wait_BUSY
		SPI_ReadReg(Reg);
	
	while(Reg[1]&BUSY)
	{
		SPI_ReadReg(Reg);
	}
}


uint8_t Erase_Sector(uint32_t Addr)
{	
	uint8_t addr[3];
	addr[0] = (Addr >> 16) & 0xFF;
	addr[1] = (Addr >> 8)  & 0xFF;
	addr[2] = (Addr >> 0)  & 0xFF;
	
	Write_Enable();
	
	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_6,GPIO_PIN_RESET);
	

	HAL_SPI_Transmit(&hspi1, &SectorErase, 1, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi1, addr, 3, HAL_MAX_DELAY);
	
	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_6,GPIO_PIN_SET);
	
  Wait_BUSY();
	
	return 0;
}

//uint8_t Send_FLASH_Buffer(uint32_t Addr,uint32_t Size,uint8_t *Buffer)
//{
//	uint8_t Status;
//  Status=Erase_Sector(Addr);
//	if(Status==0)
//	{
//	Status=Page_Program(Addr,Buffer,Size);
//		if(Status==0)
//			return 0;
//	}
//return 1;
//}
void Read_Flash_Data(uint32_t Addr,uint8_t *Buff,uint16_t Size)
{
	uint8_t ReadData=0x03;
	uint8_t Dummy=0xFF;
	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_6,GPIO_PIN_RESET);
	
	uint8_t addr[3];
	addr[0] = (Addr >> 16) & 0xFF;
	addr[1] = (Addr >> 8)  & 0xFF;
	addr[2] = (Addr >> 0)  & 0xFF;

	 // 1. 发送读命令
    HAL_SPI_Transmit(&hspi1, &ReadData, 1, HAL_MAX_DELAY);

    // 2. 发送24位地址
    HAL_SPI_Transmit(&hspi1, addr, 3, HAL_MAX_DELAY);
	
	// 3. 连续读数据
    for (uint16_t i = 0; i < Size; i++)
    {
        HAL_SPI_TransmitReceive(&hspi1, &Dummy, &Buff[i], 1, HAL_MAX_DELAY);
    }

		
	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_6,GPIO_PIN_SET);
}

uint8_t Page_Program(uint32_t Addr,uint8_t *Buff,uint32_t Size)
{
	uint16_t Page_Remain;//当前页还有多少个空间可以写
	uint16_t Write_Len;//要写入的长度
	

	uint8_t addr[3];	

	while(Size>0)
	{
	Page_Remain=256-(Addr%256);//当前页还能写下的长度
		
		Write_Len=(Size<Page_Remain)?Size:Page_Remain;
		
		addr[0] = (Addr >> 16) & 0xFF;
		addr[1] = (Addr >> 8)  & 0xFF;
		addr[2] = (Addr >> 0)  & 0xFF;
		
			Write_Enable();
	
	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_6,GPIO_PIN_RESET);
	
	HAL_SPI_Transmit(&hspi1, &PageProgram, 1, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi1, addr, 3, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi1, Buff, Write_Len, HAL_MAX_DELAY);
		
	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_6,GPIO_PIN_SET);
		
		Wait_BUSY();
	
	//更新指针
		Buff+=Write_Len;
		Addr+=Write_Len;
		Size-=Write_Len;
		
	}
	
	return 0;
}

uint8_t Sectors_Erase(uint32_t Addr,uint32_t Size)
{
 
	uint32_t Start_Sector=Addr/4096;
	uint32_t End_Sector=(Addr+Size-1)/4096;
	
	for(uint32_t i=Start_Sector;i<=End_Sector;i++)
	{
	Erase_Sector(i*4096);
	}
	return 0;
}
