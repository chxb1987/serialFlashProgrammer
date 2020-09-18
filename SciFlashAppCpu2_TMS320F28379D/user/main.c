/********************************************************
**　  文件名称：main.c
**　  文件描述：
**　  功能说明：
**　  修改说明：
**　          版本：
**　  修改记录：
********************************************************/
#include "F28x_Project.h"
/******************************************************
函数名称:main
函数描述:主函数
输入参数:
返回值:
******************************************************/
int main(void)
{
    //芯片时钟初始化
    Device_init();
    //禁用中断
    DINT;
    //初始化中断模块
    Interrupt_initModule();
    //初始化中断向量表
    Interrupt_initVectorTable();
    //启用全局中断
    EINT;
    ERTM;
	while(1)
	{
        GPIO_togglePin(DEVICE_GPIO_PIN_LED2);
        DEVICE_DELAY_US(100000);
	}
}