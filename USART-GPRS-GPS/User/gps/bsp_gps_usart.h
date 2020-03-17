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

//GPS�ӿ�����
////////////////////////////////////////////////////////////////////////////////////////////////
#define GPS_DR_Base											(USART2_BASE+0X04) 	//���ڵ����ݼĴ�����ַ
	
#define GPS_DATA_ADDR										GPS_DR_Base   			//GPSʹ�õĴ������ݼĴ�����ַ
#define GPS_RBUFF_SIZE									512									//���ڽ��ջ�������С
#define HALF_GPS_RBUFF_SIZE							(GPS_RBUFF_SIZE/2) 	//���ڽ��ջ�����һ��


#define GPS_GNGGA_SIZE  72+10		//GPS/������λ��Ϣ
#define GPS_GNGSA_SIZE 	65+10		//��ǰ������Ϣ
#define GPS_GPGSV_SIZE	210+10		//�ɼ� GPS ������Ϣ 
#define GPS_BDGSV_SIZE	210+10		//�ɼ�����������Ϣ
#define GPS_GNRMC_SIZE	70+10		//�Ƽ���λ��Ϣ
#define GPS_GNVTG_SIZE	34+10		//�����ٶ���Ϣ
#define GPS_GNGLL_SIZE	200+10		//���������Ϣ
#define GPS_GNZDA_SIZE	200+10		//��ǰʱ��(UTC 1 )��Ϣ 

////////////////////////////////////////////////////////////////////////////////////////////////
//����
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
#define GPS_DMA_IRQn										DMA1_Stream5_IRQn   //GPS�ж�Դ
#define GPS_USART_DMA_CLK								RCC_AHB1Periph_DMA1
#define GPS_USART_DMA_CHANNEL						DMA_Channel_4


//�����־
//����������
#define GPS_DMA_IT_HT										DMA_IT_HTIF5 
//������ȫ��
#define GPS_DMA_IT_TC										DMA_IT_TCIF5

//�жϺ���
#define GPS_DMA_IRQHandler							DMA1_Stream5_IRQHandler //GPSʹ�õ�DMA�жϷ�����









//�жϴ�����
void GPS_ProcessDMAIRQ(void);

void GPS_Config(void);

//1 ��� GNGGA ����
void Clear_GNGGA(void);

//2 ��� GNGSA ����
void Clear_GNGSA(void);

//3 ��� GPGSV ����
void Clear_GPGSV(void);

//4 ��� BDGSV ����
void Clear_BDGSV(void);

//5 ��� GNRMC ����
void Clear_GNRMC(void);

//6 ��� GNVTG ����
void Clear_GNVTG(void);

//7 ��� GNGLL ����
void Clear_GNGLL(void);

//8 ��� GNZDA ����
void Clear_GNZDA(void);


#endif
