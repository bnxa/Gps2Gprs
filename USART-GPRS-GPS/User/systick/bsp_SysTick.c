#include "bsp_SysTick.h"

static __IO u32 TimingDelay;
/*
����ϵͳ��ʱ��SysTick
*/
void SysTick_Init(void)
{
	/*
	SystemFrequency/1000    1ms�ж�һ��
	SystemFrequency/100000  10us�ж�һ��
	SystemFrequency/1000000 1us�ж�һ��
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
