/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "SPI_FLASH.h"
#include "ProtocolFrame.h"
#include "FLASH_Transmit.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
static char BOOT_Rx[]="\r\nBOOT_Rx:\r\n";
static char DataTx_BootMode[]="BOOT_MODE";
static char DataTx_Wait[]="\r\nWait  Data\r\n";
static char DataTx_info[]="\r\ninfo is ready\r\n";
static char DataTx_jump[]="\r\njump to app\r\n";
static char DataTx_upgrade[]="\r\n enter upgrade mode\r\n";
static char DataTx_UKnow[]="\r\nUnknow cmd\r\n";

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile 	uint32_t Last_Data;
volatile	uint32_t Page_Num;
volatile uint32_t Addr;
volatile uint32_t Bin_Size;

uint8_t FLASH_Data[256];
uint8_t Frame_Val;
uint8_t ACK=0x06;
uint8_t NACK=0x15;
   //接收命令缓存
		volatile uint8_t RxCount=0;
		 uint8_t RxTemp=0;
		volatile uint8_t Rx_Flag_Cmd=0;
		char DataRx[32]={0};
static volatile  uint8_t key_pressed=0;
		
		Firmware_Info firmware_info;
		
#define APP_ADDRESS  0x08008000U
typedef void (*pFunction)(void);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_0)
    {
        key_pressed = 1;
    }
}


typedef struct
{
 	 uint32_t boot_mode;
	 uint32_t upgrade_mode;
} boot_flag;

	boot_flag BootFlag={0};

void JumpToApplication(void)
{
    uint32_t app_sp    = *(__IO uint32_t*)APP_ADDRESS;
    uint32_t app_reset = *(__IO uint32_t*)(APP_ADDRESS + 4);

    if ((app_sp & 0x2FFE0000U) == 0x20000000U)
    {
        __disable_irq();

        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL  = 0;

        for (uint32_t i = 0; i < 8; i++)
        {
            NVIC->ICER[i] = 0xFFFFFFFF;
            NVIC->ICPR[i] = 0xFFFFFFFF;
        }

        SCB->VTOR = APP_ADDRESS;
        __set_MSP(app_sp);

        pFunction app_entry = (pFunction)app_reset;
        app_entry();
    }
}

		//
		uint8_t Rx_Data_upgrade[268] = {0};
		uint8_t Rx_Data_First[268]={0};
    volatile uint8_t upgrade_block_ready = 0;
		  volatile uint8_t upgrade_rx_busy =0; 
		volatile uint8_t Upgrade_complete=0;
		 	
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{
		
		if(BootFlag.upgrade_mode==1)
	{
//		HAL_UART_Receive_IT(&huart1,Rx_Data_upgrade,16);
		upgrade_block_ready=1;//字节已经收满
	}
		//将接收到的数据全部保存到DataRx中，然后置位Rx_Flag_Cmd=1以及Flag
		//当while检测到Rx_Flag_Cmd=1时，将DataRx数据输出到串口
   else if (Rx_Flag_Cmd == 0)   // 只有上一条还没处理完时，才继续往 DataRx 里装
     {
         if (RxTemp == '\r' || RxTemp == '\n')
         {
             if (RxCount > 0)
             {
                 DataRx[RxCount] = '\0';
                 Rx_Flag_Cmd = 1;
             }
         }
			
		 else if (RxCount < sizeof(DataRx) - 1)
          {
              DataRx[RxCount] = RxTemp;
              RxCount++;
						
						HAL_UART_Receive_IT(&huart1,&RxTemp,1);
          }
				 
	   		
	} 
 return;
}

}		
uint8_t calculate(uint8_t *rx_buff)
{
    Bin_Size = Get_Bin_Size(rx_buff);
    
	      // 写标志区
    Firmware_Info info;
    info.bin_size = Bin_Size;
    info.upgrade_flag = 1;
    info.active_app = 0;
    Write_Firmware_Info(0xFFE000, &info);
	
    uint8_t Status = Sectors_Erase(Addr, Bin_Size);
    if(Status == 0)
    {
        Page_Num = Bin_Size / 256;
        Last_Data = Bin_Size % 256;
        return 0;
    }
    else
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)"Erase Failed", 12, HAL_MAX_DELAY);
        return 1;
    }
}

void Upgrade_Mode(void)
{
	Addr = W25Q_FLASH_Addr;
//   calculate ();
	
	    // 清空串口缓冲区
    __HAL_UART_FLUSH_DRREGISTER(&huart1);
	Frame_t *frame = (Frame_t *)Rx_Data_upgrade;

	//接收第一条指令
	 upgrade_block_ready = 0;
    HAL_UART_Receive_IT(&huart1, Rx_Data_First, 268);  
    
    while(upgrade_block_ready == 0);
    
		if(Parse_Frame(Rx_Data_First) == 0)  // 先验证
	{
			if(calculate(Rx_Data_First) == 0)  // 再处理
			{
					HAL_UART_Transmit(&huart1, (uint8_t*)"Ready\r\n", 7, 100);
			}
	else
    {
     	HAL_UART_Transmit(&huart1, (uint8_t*)"Failed\r\n", 8, 100);
    }
   
	}
    // 接收整页
    for(uint32_t i = 0; i < Page_Num; i++)
    {
        upgrade_block_ready = 0;
			
        HAL_UART_Receive_IT(&huart1, Rx_Data_upgrade, 268);
        
			while(upgrade_block_ready == 0);  // 等待接收完成,为1会跳出循环
        
			Frame_Val=Parse_Frame(Rx_Data_upgrade);

			if(Frame_Val==0)
			{
			Page_Program(Addr, frame->data, 256);
		  Addr += 256;
		
			HAL_UART_Transmit(&huart1,&ACK,1,HAL_MAX_DELAY);
			}
			else
			{
			HAL_UART_Transmit(&huart1,&NACK,1,HAL_MAX_DELAY);
			}
		}
if(Last_Data > 0)
{
    upgrade_block_ready = 0;
    HAL_UART_Receive_IT(&huart1, Rx_Data_upgrade, 268);  // 改成 268
    
    while(upgrade_block_ready == 0);
    
    Frame_Val = Parse_Frame(Rx_Data_upgrade);
    
    if(Frame_Val == 0)
    {
        Page_Program(Addr, frame->data, Last_Data);  // 只写有效数据
        HAL_UART_Transmit(&huart1, &ACK, 1, HAL_MAX_DELAY);
    }
    else
    {
        HAL_UART_Transmit(&huart1, &NACK, 1, HAL_MAX_DELAY);
    }
}
    // 升级完成
    HAL_UART_Transmit(&huart1, (uint8_t*)"Upgrade Done\r\n", 14, 100);
    BootFlag.upgrade_mode = 0;
		Upgrade_complete=1;
}

//void Upgrade_Mode(void)
//{
//    uint32_t start_addr = 0xFFF000;  // 保存起始地址
//    Addr = start_addr;
//    calculate();
//    HAL_UART_Transmit(&huart1, (uint8_t*)"Ready\r\n", 7, 100);
//    
//    uint8_t verify_buf[256];
//    uint8_t verify_ok = 1;
//    
//    // 接收整页
//    for(uint32_t i = 0; i < Page_Num; i++)
//    {
//        upgrade_block_ready = 0;
//        HAL_UART_Receive_IT(&huart1, Rx_Data_upgrade, 256);
//        while(upgrade_block_ready == 0);
//        
//        Page_Program(Addr, Rx_Data_upgrade, 256);
//        
//        // 边写边验证
//        Read_Flash_Data(Addr, verify_buf, 256);
//        for(int j = 0; j < 256; j++)
//        {
//            if(verify_buf[j] != Rx_Data_upgrade[j])
//            {
//                verify_ok = 0;
//                break;
//            }
//        }
//        
//        Addr += 256;
//        HAL_UART_Transmit(&huart1, (uint8_t*)".", 1, 10);
//    }
//    
//    // 接收最后不足256的部分
//    if(Last_Data > 0)
//    {
//        upgrade_block_ready = 0;
//        HAL_UART_Receive_IT(&huart1, Rx_Data_upgrade, Last_Data);
//        while(upgrade_block_ready == 0);
//        
//        Page_Program(Addr, Rx_Data_upgrade, Last_Data);
//        
//        // 验证最后一块
//        Read_Flash_Data(Addr, verify_buf, Last_Data);
//        for(int j = 0; j < Last_Data; j++)
//        {
//            if(verify_buf[j] != Rx_Data_upgrade[j])
//            {
//                verify_ok = 0;
//                break;
//            }
//        }
//    }
//    
//    // 打印验证结果
//    if(verify_ok)
//        HAL_UART_Transmit(&huart1, (uint8_t*)"\r\nVerify OK\r\n", 13, 100);
//    else
//        HAL_UART_Transmit(&huart1, (uint8_t*)"\r\nVerify FAIL\r\n", 15, 100);
//    
//    HAL_UART_Transmit(&huart1, (uint8_t*)"Upgrade Done\r\n", 14, 100);
//    BootFlag.upgrade_mode = 0;
//}
void Boot_ProcessCmd(char *pDataRx)
{
			if(strcmp(pDataRx, "info") == 0)
			{
				HAL_UART_Transmit(&huart1,(uint8_t*)DataTx_info,strlen(DataTx_info),100);
				
			}
			else if(strcmp(pDataRx, "jump") == 0)
			{
			HAL_UART_Transmit(&huart1,(uint8_t*)DataTx_jump,strlen(DataTx_jump),100);
				JumpToApplication();
			}
			else if(strcmp(pDataRx, "upgrade") == 0)
			{
			HAL_UART_Transmit(&huart1,(uint8_t*)DataTx_upgrade,strlen(DataTx_upgrade),100);
				
			Rx_Flag_Cmd=0;
			memset(DataRx,0,sizeof(DataRx));
			RxCount=0;
			BootFlag.upgrade_mode=1;
			
			HAL_UART_AbortReceive(&huart1);
			
			}
			else
			{
			HAL_UART_Transmit(&huart1,(uint8_t*)DataTx_UKnow,strlen(DataTx_UKnow),100);		
			
			}

}


void Boot_Wait(void)
	{
		HAL_UART_Receive_IT(&huart1,&RxTemp,1);
		HAL_UART_Transmit(&huart1,(uint8_t*)DataTx_BootMode,strlen(DataTx_BootMode),100);
		HAL_UART_Transmit(&huart1,(uint8_t*)DataTx_Wait,strlen(DataTx_Wait),100);

	 while(1)
	 {
	 HAL_GPIO_WritePin(GPIOF,GPIO_PIN_8,GPIO_PIN_RESET);
		HAL_Delay(50);
			 HAL_GPIO_WritePin(GPIOF,GPIO_PIN_8,GPIO_PIN_SET);
		HAL_Delay(50);
		 
		 if(BootFlag.upgrade_mode==1)
			{
				Upgrade_Mode();
				
				if(Upgrade_complete==1)
				{
				
				
				}
			}
			
		else if(Rx_Flag_Cmd==1)
		{
		Rx_Flag_Cmd=0;
			
		//	HAL_UART_Transmit(&huart1,(uint8_t*)DataRx,strlen(DataRx),100);
			Boot_ProcessCmd(DataRx);
			
			memset(DataRx,0,sizeof(DataRx));
			RxCount=0;
			
			if(BootFlag.upgrade_mode==0)
			{
			
			HAL_UART_Transmit(&huart1,(uint8_t*)DataTx_Wait,strlen(DataTx_Wait),100);
				
			}

		}	 
		}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	uint32_t CurrentTime;
	uint32_t StarttTime;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
		Firmware_Info info;
   Read_Firmware_Info(FlAG_FLASH_ADDR, &info);

   if(info.upgrade_flag == 1)
   {
    // 有新固件，搬运
    Write_Internal_Flash();
   }

    StarttTime=HAL_GetTick();
		CurrentTime=HAL_GetTick();
		while((CurrentTime-StarttTime)<3000)
		{
			HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_RESET);
			HAL_Delay(100);
			HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_SET);
			HAL_Delay(100);
			
			if(key_pressed==1)
			{
			BootFlag.boot_mode=1;
			break;
			}
			
			CurrentTime=HAL_GetTick();
		}
		
		if(BootFlag.boot_mode==1)
		{
			Boot_Wait();
		}
		else
			JumpToApplication();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
			while (1)
			{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
			}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6|GPIO_PIN_8, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pins : PF6 PF8 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PG6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
