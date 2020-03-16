#ifndef _BSP_GPS_USART_H
#define _BSP_GPS_USART_H

#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>

//GPS接口配置
////////////////////////////////////////////////////////////////////////////////////////////////
#define GPS_DR_Base											(USART2_BASE+0X04) 	//串口的数据寄存器地址
	
#define GPS_DATA_ADDR										GPS_DR_Base   			//GPS使用的串口数据寄存器地址
#define GPS_RBUFF_SIZE									512									//串口接收缓冲区大小
#define HALF_GPS_RBUFF_SIZE							(GPS_RBUFF_SIZE/2) 	//串口接收缓冲区一半


////////////////////////////////////////////////////////////////////////////////////////////////
//串口
#define GPS_USART_BAUDRATE							38400
#define GPS_USART												USART2
#define GPS_USART_CLK										RCC_APB1Periph_USART2
#define GPS_RCC_PeriphClockCmd_Fun			RCC_APB1PeriphClockCmd

#define GPS_USART_RX_GPIO_PORT					GPIOA
#define GPS_USART_RX_GPIO_CLK						RCC_AHB1Periph_GPIOA
#define GPS_USART_RX_PIN								GPIO_Pin_3
#define GPS_USART_RX_AF									GPIO_AF_USART2
#define GPS_USART_RX_SOURCE							GPIO_PinSource3

#define GPS_USART_TX_GPIO_PORT					GPIOA
#define GPS_USART_TX_GPIO_CLK						RCC_AHB1Periph_GPIOA
#define GPS_USART_TX_PIN								GPIO_Pin_2
#define GPS_USART_TX_AF									GPIO_AF_USART2
#define GPS_USART_TX_SOURCE							GPIO_PinSource2

////////////////////////////////////////////////////////////////////////////////////////////////
//DMA
#define GPS_USART_DMA_STREAM						DMA1_Stream5
#define GPS_DMA_IRQn										DMA1_Stream5_IRQn   //GPS中断源
#define GPS_USART_DMA_CLK								RCC_AHB1Periph_DMA1
#define GPS_USART_DMA_CHANNEL						DMA_Channel_4


//外设标志
//缓冲区半满
#define GPS_DMA_IT_HT										DMA_IT_HTIF5 
//缓冲区全满
#define GPS_DMA_IT_TC										DMA_IT_TCIF5

//中断函数
#define GPS_DMA_IRQHandler							DMA1_Stream5_IRQHandler //GPS使用的DMA中断服务函数
//中断处理函数
void GPS_ProcessDMAIRQ(void);

void GPS_Config(void);




#endif
