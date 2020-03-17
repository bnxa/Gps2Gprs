#include "stm32f4xx.h"
#include "bsp_led.h"
#include "bsp_debug_usart.h"
#include "bsp_SysTick.h"
#include "bsp_gsm_gprs.h"
#include "bsp_gps_usart.h"


//������Ϣ����
#define DEBUG_ON 1 
#define DEBUG(fmt,arg...) do{\
																	if(DEBUG_ON)\
																		printf("\r\n"fmt,##arg);\
																}while(0)


																
																
#define		LOCALPORT "2000"
#define		SERVERIP	"28d77e1773.zicp.vip"
#define		SERVERPORT	"37164"

const char *TESTBUFF1="\r\n 1. SIM800A GSMģ��TCP�����ϴ����ܲ���";
const char *TESTBUFF2="\r\n 2. SIM800A GSMģ��TCP�����ϴ����ܲ���";

//static void Show_Message(void);
extern uint8_t gps_rbuff[GPS_RBUFF_SIZE];
extern uint8_t gps_rbuff_BDGSV[GPS_BDGSV_SIZE];	//�ɼ�����������Ϣ 
extern uint8_t gps_rbuff_GNGGA[GPS_GNGGA_SIZE]; //GPS/������λ��Ϣ
extern uint8_t gps_rbuff_GNGSA[GPS_GNGSA_SIZE]; //��ǰ������Ϣ
extern uint8_t gps_rbuff_GPGSV[GPS_GPGSV_SIZE];	//�ɼ� GPS ������Ϣ 
extern uint8_t gps_rbuff_GNRMC[GPS_GNRMC_SIZE];	//�Ƽ���λ��Ϣ
extern uint8_t gps_rbuff_GNVTG[GPS_GNVTG_SIZE];	//�����ٶ���Ϣ
extern uint8_t gps_rbuff_GNGLL[GPS_GNGLL_SIZE];	//���������Ϣ
extern uint8_t gps_rbuff_GNZDA[GPS_GNZDA_SIZE];	//��ǰʱ��(UTC 1 )��Ϣ 



//ϵͳ�����λ
void Soft_Reset(void)
{
	__set_FAULTMASK(1); //�ر������ж�
	NVIC_SystemReset(); //ϵͳ��λ
}

int main(void)
{
	static uint8_t timecount=1,timestop=0;
	uint8_t testCard =0;
	uint8_t index =0;
	
	
 	/*����USART*/
	Debug_USART_Config(); 
	//��ʼ��ϵͳ��ʱ��
	SysTick_Init();
	
	GPS_Config(); 
	 
	
	DEBUG();
	DEBUG();
	DEBUG();
	DEBUG();
	DEBUG("\r\n********************************************************************\r\n");
	DEBUG("\r\n***********************GSMģ��TCP�շ�ʾ������***********************\r\n");
	DEBUG("\r\n********************************************************************\r\n");
	
	DEBUG(">%d ���ڵȴ�GSMģ�� �����豸 ������\r\n",index++);
	while(gsm_Reset()!=GSM_TRUE)
	{ 
		DEBUG(">%d �����豸ʧ�� �����ڵȴ�GSMģ�� �����豸������\r\n",index++);
	}
	
	DEBUG(">%d ���ڵȴ�GSMģ���ʼ��������\r\n",index++);
	while(gsm_init()!= GSM_TRUE)
	{
		DEBUG("\r\nģ����Ӧ���Բ�������!");
		DEBUG("\r\n��ģ����Ӧһֱ������������ģ�������ߺ͵�Դ");
	}
	
	DEBUG(">%d ���ڼ��绰��������\r\n",index++);
	while(IsInsertCard() != GSM_TRUE)
	{
		if(++testCard>20)
		{
			DEBUG("\r\n��ⲻ���绰������ϵ�����²���绰��\r\n");
		}
		GSM_DELAY(1000);
	}
	GSM_DELAY(1000);
	
	//ȷ�Ϲر�֮ǰ������
	DEBUG(">%d ȷ�Ϲر�֮ǰ������!",index++);
	gsm_gprs_link_close();
	
	GSM_DELAY(1000);
	
	//ȷ�Ϲر�֮ǰ�ĳ���
	DEBUG(">%d ȷ�Ϲر�֮ǰ�ĳ���!",index++);
	gsm_gprs_shut_close();
 
	//���³�ʼ��GPRS
	DEBUG(">%d ���³�ʼ��GPRS!",index++);
	if(gsm_gprs_init()!= GSM_TRUE) //gprs��ʼ������
	{
		DEBUG("\r\n��ʼ��GPRSʧ�ܣ������¸�ģ���ϵ粢��λ������");
		while(1);
	}
	
	DEBUG(">%d ���Խ���TCP���ӣ������ĵȴ�������",index++);
	
	if(gsm_gprs_tcp_link(LOCALPORT,SERVERIP,SERVERPORT) != GSM_TRUE)
	{
		DEBUG("\r\n TCP����ʧ�ܣ�������ȷ���ø���ģ�� XXXXXXXXXXXXXXX");
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
	DEBUG(">%d ���͵ڶ������ݳɹ�",index++);
	
	
	DEBUG(">%d ��ʼ���͵���������",index++);
	 
		
	GSM_DELAY(2000);
		 
	DEBUG("\r\n %s",gps_rbuff_BDGSV);
	if(gsm_gprs_send_GpsCmd(gps_rbuff_BDGSV) != GSM_TRUE)
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

	GSM_DELAY(10000);
 
	DEBUG("\r\n�����ر�����");
	DEBUG("\r\nIP���ӶϿ�");
	GSM_DELAY(100);
	gsm_gprs_link_close();
	
	DEBUG("\r\n�رճ���");
	GSM_DELAY(100);
	gsm_gprs_shut_close();
 
	
}
