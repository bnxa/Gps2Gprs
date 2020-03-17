#ifndef _BSP_GSM_GPRS_H
#define _BSP_GSM_GPRS_H

#include "stm32f4xx.h"
#include "bsp_gsm_usart.h"
#include "bsp_SysTick.h"

typedef enum{
	GSM_TRUE,
	GSM_FALSE,
	
} gsm_res_e;

typedef enum{
	GSM_NULL				= 0,
	GSM_CMD_SEND		= '\r',
	GSM_DATA_SEND		= 0x1A, //发送数据(ctrl + z)
	GSM_DATA_CANCLE	= 0x1B,	//发送数据（Esc)
	
}gsm_cmd_end_e;

//								指令					正常返回
//本机号码				AT+NUM\r			+CNUM: "","1322222222",129,7，4
//SIM卡运营商			AT+COPS?\r		+COPS: 0,0,"CHN-UNICOM"	OK
//SIM卡状态				AT+CPIN?\r		+CPIN:	READY OK
//SIM卡信号强度		AT+CSQ\r			+CSQ:	8,0	OK


#define GSM_CLEAN_RX()					clean_rebuff()
#define GSM_TX(cmd)							GSM_USART_printf("%s",cmd)
#define GSM_DELAY(time)					Delay_ms(time)  //延时
#define GSM_RX(len)							((char *)get_rebuff(&(len)))
	
//调试用串口
#define GSM_DEBUG_ON					1
#define GSM_DEBUG_ARRAY_ON		1
#define GSM_DEBUG_FUNC_ON			0


//LOG define
#define GSM_INFO(fmt,arg...)	printf("<<-GSM-INFO->>"fmt"\n",##arg)
#define GSM_ERROR(fmt,arg...) printf("<<-GSM-ERROR->>",##arg)
#define GSM_DEBUG(fmt,arg...) do{\
																	if(GSM_DEBUG_ON)\
																		printf("\r\n"fmt,##arg);\
																}while(0)

																

#define GSM_DEBUG_FUNC()			do{\
																	if(GSM_DEBUG_FUNC_ON)\
																		printf("\r\n<<-GSM-FUNC->> Func:%s@Line:%d\n",__func__,__LINE__);\
																}while(0)








uint8_t gsm_cmd (char *cmd,char *reply,uint32_t waittime);

uint8_t gsm_Reset(void);
																
uint8_t gsm_init(void);
																
uint8_t IsInsertCard(void);

uint8_t gsm_gprs_link_close(void);

uint8_t gsm_gprs_shut_close(void);

uint8_t gsm_gprs_init(void);

uint8_t gsm_gprs_tcp_link(char *localport,char *serverip,char *serverport);

uint8_t gsm_gprs_send(const char *str);
uint8_t gsm_gprs_send_GpsCmd(uint8_t *str);

uint8_t PostGPRS(void);
	

	
#endif
