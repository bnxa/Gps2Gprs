#include "bsp_led.h"

// 0 首先要开GPIO时钟
// 1 确定引脚号
// 2 输入还是输出 MODER
// 3 如果是输出 推挽还是开漏
// 4 上拉还是下拉
// 5 输出速度

void LED_GPIO_Config(void)
{
		/*定义一个GPIO_InitTypeDef类型的结构体*/
		GPIO_InitTypeDef GPIO_InitStructure;

		/*开启LED相关的GPIO外设时钟*/
		RCC_AHB1PeriphClockCmd ( LED1_GPIO_CLK|
	                           LED2_GPIO_CLK
//	                           LED3_GPIO_CLK
	, ENABLE); 

		/*选择要控制的GPIO引脚*/															   
		GPIO_InitStructure.GPIO_Pin = LED1_PIN;	

		/*设置引脚模式为输出模式*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
    
    /*设置引脚的输出类型为推挽输出*/
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    
    /*设置引脚为上拉模式*/
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

		/*设置引脚速率为2MHz */   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 

		/*调用库函数，使用上面配置的GPIO_InitStructure初始化GPIO*/
		GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStructure);	
    
    /*选择要控制的GPIO引脚*/															   
		GPIO_InitStructure.GPIO_Pin = LED2_PIN;	
    GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStructure);	
    
    /*选择要控制的GPIO引脚*/															   
//		GPIO_InitStructure.GPIO_Pin = LED3_PIN;	
//    GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStructure);	
		
		/*关闭RGB灯*/
		LED_RGBOFF;		
}

//LED1闪烁
void LED1_LIGHT(uint8_t ccTimes)
{ 
		LED1_OFF;
		LED_DELAY(300);
	for(int i=0;i<ccTimes;i++)
	{
		LED1_ON;
		LED_DELAY(300);
		LED1_OFF;
		LED_DELAY(300);
	}
}
//LED1闪烁
void LED1_Light_Fast(uint8_t ccTimes)
{ 
		LED1_OFF;
		LED_DELAY(150);
	for(int i=0;i<ccTimes;i++)
	{
		LED1_ON;
		LED_DELAY(150);
		LED1_OFF;
		LED_DELAY(150);
	}
}

//LED2闪烁
void LED2_LIGHT(uint8_t ccTimes)
{
		LED2_OFF;
		LED_DELAY(300);
	for(int i=0;i<ccTimes;i++)
	{
		LED2_ON;
		LED_DELAY(300);
		LED2_OFF;
		LED_DELAY(300);
	}
}
