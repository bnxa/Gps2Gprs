#ifndef _BSP_GPS_USART_H
#define _BSP_GPS_USART_H

#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>
#include "nema_decode.h"


#define GPS_DEBUG_ON					0
#define GPS_DEBUG(fmt,arg...) do{\
																	if(GPS_DEBUG_ON)\
																		printf("\r\n"fmt,##arg);\
																}while(0)

//GPS接口配置
////////////////////////////////////////////////////////////////////////////////////////////////
#define GPS_DR_Base											(USART2_BASE+0X04) 	//串口的数据寄存器地址
	
#define GPS_DATA_ADDR										GPS_DR_Base   			//GPS使用的串口数据寄存器地址
#define GPS_RBUFF_SIZE									512									//串口接收缓冲区大小
#define HALF_GPS_RBUFF_SIZE							(GPS_RBUFF_SIZE/2) 	//串口接收缓冲区一半


#define GPS_GNGGA_SIZE  72+10		//GPS/北斗定位信息
#define GPS_GNGSA_SIZE 	65+10		//当前卫星信息
#define GPS_GPGSV_SIZE	210+10		//可见 GPS 卫星信息 
#define GPS_BDGSV_SIZE	210+10		//可见北斗卫星信息
#define GPS_GNRMC_SIZE	70+10		//推荐定位信息
#define GPS_GNVTG_SIZE	34+10		//地面速度信息
#define GPS_GNGLL_SIZE	200+10		//大地坐标信息
#define GPS_GNZDA_SIZE	200+10		//当前时间(UTC 1 )信息 

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

//1 清空 GNGGA 数组
void Clear_GNGGA(void);

//2 清空 GNGSA 数组
void Clear_GNGSA(void);

//3 清空 GPGSV 数组
void Clear_GPGSV(void);

//4 清空 BDGSV 数组
void Clear_BDGSV(void);

//5 清空 GNRMC 数组
void Clear_GNRMC(void);

//6 清空 GNVTG 数组
void Clear_GNVTG(void);

//7 清空 GNGLL 数组
void Clear_GNGLL(void);

//8 清空 GNZDA 数组
void Clear_GNZDA(void);


#endif
