#include "stm32f4xx.h"
#include "bsp_led.h"
#include "bsp_debug_usart.h"
#include "bsp_SysTick.h"
#include "bsp_gsm_gprs.h"
#include "bsp_gps_usart.h"


#define		LOCALPORT "2000"
#define		SERVERIP	"28d77e1773.zicp.vip"
#define		SERVERPORT	"37164"

const char *TESTBUFF1="\r\n 1. SIM800A GSMģ��TCP�����ϴ����ܲ���";
const char *TESTBUFF2="\r\n 2. SIM800A GSMģ��TCP�����ϴ����ܲ���";

//static void Show_Message(void);
extern uint8_t gps_rbuff[GPS_RBUFF_SIZE];

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
  
	
	printf("\r\n\r\n");
	printf("\r\n\r\n");
	printf("\r\n\r\n");
	printf("\r\n\r\n");
	printf("\r\n********************************************************************\r\n");
	printf("\r\n***********************GSMģ��TCP�շ�ʾ������***********************\r\n");
	printf("\r\n********************************************************************\r\n");
	
	printf("\r\n>%d ���ڵȴ�GSMģ���ʼ��������\r\n",index++);
	while(gsm_init()!= GSM_TRUE)
	{
		printf("\r\nģ����Ӧ���Բ�������!");
		printf("\r\n��ģ����Ӧһֱ������������ģ�������ߺ͵�Դ");
	}
	
	printf("\r\n>%d ���ڼ��绰��������\r\n",index++);
	while(IsInsertCard() != GSM_TRUE)
	{
		if(++testCard>20)
		{
			printf("\r\n��ⲻ���绰������ϵ�����²���绰��\r\n");
		}
		GSM_DELAY(1000);
	}
	GSM_DELAY(1000);
	
	//ȷ�Ϲر�֮ǰ������
	printf("\r\n>%d ȷ�Ϲر�֮ǰ������!",index++);
	gsm_gprs_link_close();
	
	GSM_DELAY(1000);
	
	//ȷ�Ϲر�֮ǰ�ĳ���
	printf("\r\n>%d ȷ�Ϲر�֮ǰ�ĳ���!",index++);
	gsm_gprs_shut_close();
 
	//���³�ʼ��GPRS
	printf("\r\n>%d ���³�ʼ��GPRS!",index++);
	if(gsm_gprs_init()!= GSM_TRUE) //gprs��ʼ������
	{
		printf("\r\n��ʼ��GPRSʧ�ܣ������¸�ģ���ϵ粢��λ������");
		while(1);
	}
	
	printf("\r\n>%d ���Խ���TCP���ӣ������ĵȴ�������",index++);
	
	if(gsm_gprs_tcp_link(LOCALPORT,SERVERIP,SERVERPORT) != GSM_TRUE)
	{
		printf("\r\n TCP����ʧ�ܣ�������ȷ���ø���ģ�� XXXXXXXXXXXXXXX");
		GSM_DELAY(1000);
		printf("\r\n IP���ӶϿ�");
		GSM_DELAY(1000);
		gsm_gprs_link_close();
		
		printf("\r\n �رճ���");
		GSM_DELAY(1000);
		gsm_gprs_shut_close();
		printf("\r\n ********5s���Զ�����**********");
		GSM_DELAY(5000);
		Soft_Reset();
	}
	
	printf("\r\n>%d ���ӳɹ������Է������ݡ�����",index++);
	if(gsm_gprs_send(TESTBUFF1) != GSM_TRUE)
	{
		printf("\r\n TCP��������ʧ�ܣ�������ȷ���ø���ģ�� XXXXXXXXXXXXXXX");
		GSM_DELAY(1000);
		printf("\r\n IP���ӶϿ�");
		GSM_DELAY(1000);
		gsm_gprs_link_close();
		
		printf("\r\n �رճ���");
		GSM_DELAY(1000);
		gsm_gprs_shut_close();
		while(1);
	}
	
	printf("\r\n>%d ���Է��͵ڶ������ݡ�����",index++);
	if(gsm_gprs_send(TESTBUFF2) != GSM_TRUE)
	{
		printf("\r\n>TCP��������ʧ�ܣ�������ȷ���ø���ģ�� XXXXXXXXXXXXXXX");
		GSM_DELAY(1000);
		printf("\r\n IP���ӶϿ�");
		GSM_DELAY(1000);
		gsm_gprs_link_close();
		
		printf("\r\n �رճ���");
		GSM_DELAY(1000);
		gsm_gprs_shut_close();
		while(1);
	}
	printf ("\r\n>%d ���͵ڶ������ݳɹ�",index++);
	
	
	printf ("\r\n>%d ��ʼ���͵���������",index++);
	
	while(1)
	{ 
		GSM_DELAY(2000);
		 
		char arr1[512];
		int i=0;
		for(i=0;i<512;i++)
		{
			arr1[i] ='\0';
		}
		for(i=0;i<511;i++)
		{
			arr1[i]=gps_rbuff[i];
		}
		 
		
		if(gsm_gprs_send2(arr1) != GSM_TRUE)
		{
			printf("\r\n>TCP��������ʧ�ܣ�������ȷ���ø���ģ�� XXXXXXXXXXXXXXX");
			GSM_DELAY(1000);
			printf("\r\n IP���ӶϿ�");
			GSM_DELAY(1000);
			gsm_gprs_link_close();
			
			printf("\r\n �رճ���");
			GSM_DELAY(1000);
			gsm_gprs_shut_close();
			while(1);
		}
		
	}
	
	
	
	printf("\r\n>%d ��׼���ý���Զ�����ݣ���ʹ������������ַ�������",index++);
	GSM_CLEAN_RX();
	
	while(1)
	{
		if((timecount>=50) && (timestop!=0xFF))
		{
			if(PostGPRS()!=GSM_TRUE)
				timestop++;
			else
				timestop=0;
			timecount = 0;			
		}
		if(timestop == 120) //60s
		{
			printf("\r\n��ʱ����ͨѶ�������ر�����");
			printf("\r\nIP���ӶϿ�");
			GSM_DELAY(100);
			gsm_gprs_link_close();
			
			printf("\r\n�رճ���");
			GSM_DELAY(100);
			gsm_gprs_shut_close();
			
			GSM_DELAY(1000);
			timestop = 0xFF;			
		}
		timecount++;
		GSM_DELAY(10);
	}
	
}
