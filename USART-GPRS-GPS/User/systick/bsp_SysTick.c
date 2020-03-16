#include "bsp_SysTick.h"

static __IO u32 TimingDelay;
/*
启动系统定时器SysTick
*/
void SysTick_Init(void)
{
	/*
	SystemFrequency/1000    1ms中断一次
	SystemFrequency/100000  10us中断一次
	SystemFrequency/1000000 1us中断一次
	*/
	if(SysTick_Config(SystemCoreClock / 100000))
	{
		while(1);
	}
}

void Delay_us(__IO u32 nTime)
{
	TimingDelay = nTime;
	while(TimingDelay != 0);
}

void TimingDelay_Decrement(void)
{
	if(TimingDelay != 0x00)
		TimingDelay--;
}
