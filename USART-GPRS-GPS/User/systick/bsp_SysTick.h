#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f4xx.h"

#define Delay_ms(x) Delay_us(100*x); //µ•Œª∫¡√Îms

void TimingDelay_Decrement(void);
void SysTick_Init(void);
void Delay_us(__IO u32 nTime); 



#endif
