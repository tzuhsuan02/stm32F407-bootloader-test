#include "ProtocolFrame.h"
#include "A_B.h"
#include "FLASH_Transmit.h"

extern void Uart_SendStr(char *str);
extern void Uart_Printf(const char *fmt, ...);

void Save_Firmware_OTA_State(uint8_t active_app, uint8_t valid_app, uint8_t upgrade_flag)
{
    Firmware_Info Info = {0};
    Read_Firmware_Info(FlAG_FLASH_ADDR, &Info);

    Info.active_app   = active_app;
    Info.valid_app    = valid_app;
    Info.upgrade_flag = upgrade_flag;

    Write_Firmware_Info(FlAG_FLASH_ADDR, &Info);
}

/*根据active_app把固件区的升级程序写进A/B区
如果当前是A区运行，就把新固件程序写进B区，再把B区数据写进内部FLASH 
	 升级完成之后运行App，可能会变砖
	重启之后发现上次APP崩溃了，需要回滚*/
void OTA_Process(void)
{
    Firmware_Info Info = {0};

    Uart_SendStr("OTA Start\r\n");
    Read_Firmware_Info(FlAG_FLASH_ADDR, &Info);

    Uart_Printf("flag:%d valid:%d active:%02X size:%lu\r\n",
                Info.upgrade_flag,
                Info.valid_app,
                Info.active_app,
                Info.bin_size);

   //下载区收到新的bin包：可升级
		/* 情况1：接收升级包 */
    if (Info.upgrade_flag == 1)
    {
        Uart_SendStr("---------------Upgrading--------------\r\n");

        if (Info.active_app == 0x0A)//如果现在运行的是A区的就放进B区
        {
            Uart_SendStr("Write to B\r\n");
            DOWNLOAD_To_B();//下载区的代码下进B区中
            Write_Internal_Flash(FLASH_B_ADDR,Info.bin_size);//下载区的代码下进内部FLASH中

            Info.active_app = 0x0B;//现在运行的是B区代码
        }
        else
        {
            Uart_SendStr("Write to A\r\n");
            DOWNLOAD_To_A();
            Write_Internal_Flash(FLASH_A_ADDR,Info.bin_size);

            Info.active_app = 0x0A;
        }

			//已完成更新
        Info.upgrade_flag = 0;
        Info.valid_app = 0;//未验证能否运行，等待boot跳转到app区，置位
				
        Uart_SendStr("Upgrade Done\r\n");
    }
    /* 情况2：试运行失败，执行回滚 */
		//升级完成但app没有置位，代表app有问题要回滚
    else if ((Info.upgrade_flag == 0) && (Info.valid_app == 0))
    {
        Uart_SendStr("Rollback\r\n");

        if (Info.active_app == 0x0A)
        {
            Uart_SendStr("Read B\r\n");
            Read_B_To_Internal();

            // 回滚后当前运行的是 B 
            Info.active_app = 0x0B;
        }
        else
        {
            Uart_SendStr("Read A\r\n");
            Read_A_To_Internal();

            /* 回滚后当前运行的是 A */
            Info.active_app = 0x0A;
        }

        //回滚到旧版本，视为已确认有效 
        Info.upgrade_flag = 0;
        Info.valid_app = 1;

        Uart_SendStr("Rollback Done\r\n");
    }
    else
    {
        Uart_SendStr("No OTA Action\r\n");
    }
Save_Firmware_OTA_State(Info.active_app, Info.valid_app, Info.upgrade_flag);

    Uart_SendStr("OTA End\r\n");
}
