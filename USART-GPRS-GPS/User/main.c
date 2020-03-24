#include "stm32f4xx.h"
#include "bsp_led.h"
#include "bsp_debug_usart.h"
#include "bsp_SysTick.h"
#include "bsp_gsm_gprs.h"
#include "bsp_gps_usart.h"

#define  SOCKETTYPE "TCP"
//������Ϣ����
#define DEBUG_ON 1 
#define DEBUG(fmt,arg...) do{\
																	if(DEBUG_ON)\
																		printf("\r\n"fmt,##arg);\
																}while(0)

  
																
																
#define		LOCALPORT "2000"
#define		SERVERIP	"28d77e1773.zicp.vip"
#define		SERVERPORT	"40658"

const char *TESTBUFF1="\r\n 1. SIM800A GSMģ��GPS�����ϴ����ܲ���--";
const char *TESTBUFF2="\r\n 2. SIM800A GSMģ��GPS�����ϴ����ܲ���--";

//static void Show_Message(void);

																
extern uint8_t gprs_data_buff[GPRS_DATA_SIZE];

//ϵͳ�����λ
void Soft_Reset(void)
{
	__set_FAULTMASK(1); //�ر������ж�
	NVIC_SystemReset(); //ϵͳ��λ
}

int main(void)
{ 
	
	uint8_t testCard =0;
	uint8_t index =0;
	
	LED_GPIO_Config();
	LED1_ON;
	LED2_ON;
	
 	/*����USART*/
	Debug_USART_Config(); 
	//��ʼ��ϵͳ��ʱ��
	SysTick_Init();
	
	GPS_Config();   
	
	GSM_USART_Config(); //��ʼ������
	
	GSM_DELAY(1000);
	LED_RGBOFF 
	
	DEBUG();
	DEBUG();
	DEBUG();
	DEBUG();
	DEBUG("\r\n****************************************************************************************************\r\n");
	DEBUG("\r\n***********************GSMģ��%s�շ�ʾ������*******************************************************\r\n",SOCKETTYPE);
	DEBUG("\r\n****************************************************************************************************\r\n");
	
	DEBUG(">%d ���ڵȴ�GSMģ�� �����豸 ������\r\n",index++);
	while(gsm_Reset()!=GSM_TRUE)
	{		
		DEBUG(">%d �����豸ʧ�� �����ڵȴ�GSMģ�� �����豸������\r\n",index++);
		LED2_LIGHT(index);
		GSM_DELAY(2000);
	}
	LED1_LIGHT(index);
	GSM_DELAY(2000); 
	
	DEBUG(">%d ��ѯ�ź�ǿ�ȡ�����",index++);
	while(gsm_sigin() != GSM_TRUE)
	{
		DEBUG(">%d ��ѯ�ź�ǿ��ʧ�� ������\r\n",index++);
		LED2_LIGHT(index);
		GSM_DELAY(2000);
	}
	LED1_LIGHT(index);
	
	
	DEBUG(">%d ��ȡ�豸IMEI�� \r\n",index++);
	while(GetIMEI()!=GSM_TRUE)
	{ 
		DEBUG(">%d ��ȡ�豸IMEI��ʧ�ܣ�",index++);
		LED2_LIGHT(index);
	} 
	LED1_LIGHT(index);
	 
	
	DEBUG(">%d ���ڵȴ�GSMģ���ʼ��������\r\n",index++); 
	while(gsm_init()!= GSM_TRUE)
	{ 
		DEBUG("\r\nģ����Ӧ���Բ�������!");
		DEBUG("\r\n��ģ����Ӧһֱ������������ģ�������ߺ͵�Դ");
		LED2_LIGHT(index);
	} 
	LED1_LIGHT(index);
	
	
	DEBUG(">%d ���ڼ��绰��������\r\n",index++); 
	while(IsInsertCard() != GSM_TRUE)
	{ 
		if(++testCard>20)
		{
			DEBUG("\r\n��ⲻ���绰������ϵ�����²���绰��\r\n");
			LED2_LIGHT(index);
		}
		GSM_DELAY(1000);
	} 
	LED1_LIGHT(index);
	GSM_DELAY(1000);
	   
	
	//ȷ�Ϲر�֮ǰ������
	DEBUG(">%d ȷ�Ϲر�֮ǰ������!",index++);
	gsm_gprs_link_close();
	LED1_LIGHT(index);	
	GSM_DELAY(1000);
	
	
	//ȷ�Ϲر�֮ǰ�ĳ���
	DEBUG(">%d ȷ�Ϲر�֮ǰ�ĳ���!",index++);
	gsm_gprs_shut_close();
	LED1_LIGHT(index);
     
	
	//���³�ʼ��GPRS
	DEBUG(">%d ���³�ʼ��GPRS!",index++);
	while(gsm_gprs_init()!= GSM_TRUE) //gprs��ʼ������
	{ 
		DEBUG("\r\n��ʼ��GPRSʧ�ܣ������¸�ģ���ϵ粢��λ������"); 
		LED2_LIGHT(index);
		 
		while(1); 
		 
	}
	LED1_LIGHT(index);
	
	
	
	DEBUG(">%d ���Խ���%s���ӣ������ĵȴ�������",index++,SOCKETTYPE);	
	if(gsm_gprs_tcp_link(LOCALPORT,SERVERIP,SERVERPORT) != GSM_TRUE)
	{
		LED2_LIGHT(index);
		
		LED2_ON;
		DEBUG("\r\n %s����ʧ�ܣ�������ȷ���ø���ģ�� XXXXXXXXXXXXXXX",SOCKETTYPE);
		GSM_DELAY(1000);
		DEBUG("\r\n IP���ӶϿ�");
		GSM_DELAY(1000);
		gsm_gprs_link_close();
		
		DEBUG("\r\n �رճ���");
		GSM_DELAY(1000);
		gsm_gprs_shut_close();
		DEBUG("\r\n ********5s���Զ�����**********");
		GSM_DELAY(5000);
		Soft_Reset();
	}
	LED1_LIGHT(index);
	
	 
	DEBUG(">%d ���ӳɹ������Է������ݡ�����",index++);
	if(gsm_gprs_send(TESTBUFF1) != GSM_TRUE)
	{ 
		DEBUG("\r\n TCP��������ʧ�ܣ�������ȷ���ø���ģ�� XXXXXXXXXXXXXXX");
		GSM_DELAY(1000);
		DEBUG("\r\n IP���ӶϿ�");
		GSM_DELAY(1000);
		gsm_gprs_link_close();
		
		DEBUG("\r\n �رճ���");
		GSM_DELAY(1000);
		gsm_gprs_shut_close();
		while(1);
	} 
	LED1_LIGHT(index);
	
	DEBUG(">%d ���Է��͵ڶ������ݡ�����",index++);
	if(gsm_gprs_send(TESTBUFF2) != GSM_TRUE)
	{ 
		DEBUG("\r\n>TCP��������ʧ�ܣ�������ȷ���ø���ģ�� XXXXXXXXXXXXXXX");
		GSM_DELAY(1000);
		DEBUG("\r\n IP���ӶϿ�");
		GSM_DELAY(1000);
		gsm_gprs_link_close();
		
		DEBUG("\r\n �رճ���");
		GSM_DELAY(1000);
		gsm_gprs_shut_close();
		while(1);
	}
	LED1_LIGHT(index);
	DEBUG(">%d ���͵ڶ������ݳɹ�",index++);
	
	
	DEBUG(">%d ��ʼ���͵���������",index++);
	 
		
	GSM_DELAY(2000);
		 
//��ȡGPRS����
	while(1)
	{  
		while( get_gprs_data() == GSM_FALSE) //��ȡGPRS����
		{ 
			GSM_DELAY(1000);
		}
		if(gsm_gprs_send_GpsCmd(gprs_data_buff) != GSM_TRUE)
		{ 
			DEBUG("\r\n>TCP��������ʧ�ܣ�������ȷ���ø���ģ�� XXXXXXXXXXXXXXX");
			GSM_DELAY(1000);
			DEBUG("\r\n IP���ӶϿ�");
			GSM_DELAY(1000);
			gsm_gprs_link_close();
			
			DEBUG("\r\n �رճ���");
			GSM_DELAY(1000);
			gsm_gprs_shut_close();
//			while(1);
			
			DEBUG("\r\n ********5s���Զ�����**********");
			GSM_DELAY(5000);
			Soft_Reset();
		}
		LED1_TOGGLE;

		GSM_DELAY(1000);
	}
	DEBUG("\r\n�����ر�����");
	DEBUG("\r\nIP���ӶϿ�");
	GSM_DELAY(100);
	gsm_gprs_link_close();
	
	DEBUG("\r\n�رճ���");
	GSM_DELAY(100);
	gsm_gprs_shut_close();
 
	
}
