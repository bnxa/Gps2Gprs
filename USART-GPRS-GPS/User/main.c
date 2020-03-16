#include "stm32f4xx.h"
#include "bsp_led.h"
#include "bsp_debug_usart.h"
#include "bsp_SysTick.h"
#include "bsp_gsm_gprs.h"
#include "bsp_gps_usart.h"


#define		LOCALPORT "2000"
#define		SERVERIP	"28d77e1773.zicp.vip"
#define		SERVERPORT	"37164"

const char *TESTBUFF1="\r\n 1. SIM800A GSM模块TCP数据上传功能测试";
const char *TESTBUFF2="\r\n 2. SIM800A GSM模块TCP数据上传功能测试";

//static void Show_Message(void);
extern uint8_t gps_rbuff[GPS_RBUFF_SIZE];

//系统软件复位
void Soft_Reset(void)
{
	__set_FAULTMASK(1); //关闭所有中断
	NVIC_SystemReset(); //系统复位
}

int main(void)
{
	static uint8_t timecount=1,timestop=0;
	uint8_t testCard =0;
	uint8_t index =0;
	
	
 	/*配置USART*/
	Debug_USART_Config(); 
	//初始化系统定时器
	SysTick_Init();
	
	GPS_Config();
  
	
	printf("\r\n\r\n");
	printf("\r\n\r\n");
	printf("\r\n\r\n");
	printf("\r\n\r\n");
	printf("\r\n********************************************************************\r\n");
	printf("\r\n***********************GSM模块TCP收发示例程序***********************\r\n");
	printf("\r\n********************************************************************\r\n");
	
	printf("\r\n>%d 正在等待GSM模块初始化。。。\r\n",index++);
	while(gsm_init()!= GSM_TRUE)
	{
		printf("\r\n模块响应测试不正常！!");
		printf("\r\n若模块相应一直不正常，请检查模块连接线和电源");
	}
	
	printf("\r\n>%d 正在检测电话卡。。。\r\n",index++);
	while(IsInsertCard() != GSM_TRUE)
	{
		if(++testCard>20)
		{
			printf("\r\n检测不到电话卡，请断电后重新插入电话卡\r\n");
		}
		GSM_DELAY(1000);
	}
	GSM_DELAY(1000);
	
	//确认关闭之前的连接
	printf("\r\n>%d 确认关闭之前的连接!",index++);
	gsm_gprs_link_close();
	
	GSM_DELAY(1000);
	
	//确认关闭之前的场景
	printf("\r\n>%d 确认关闭之前的场景!",index++);
	gsm_gprs_shut_close();
 
	//重新初始化GPRS
	printf("\r\n>%d 重新初始化GPRS!",index++);
	if(gsm_gprs_init()!= GSM_TRUE) //gprs初始化环境
	{
		printf("\r\n初始化GPRS失败，请重新给模块上电并复位开发板");
		while(1);
	}
	
	printf("\r\n>%d 尝试建立TCP链接，请耐心等待。。。",index++);
	
	if(gsm_gprs_tcp_link(LOCALPORT,SERVERIP,SERVERPORT) != GSM_TRUE)
	{
		printf("\r\n TCP链接失败，请检测正确设置各个模块 XXXXXXXXXXXXXXX");
		GSM_DELAY(1000);
		printf("\r\n IP链接断开");
		GSM_DELAY(1000);
		gsm_gprs_link_close();
		
		printf("\r\n 关闭场景");
		GSM_DELAY(1000);
		gsm_gprs_shut_close();
		printf("\r\n ********5s后自动重启**********");
		GSM_DELAY(5000);
		Soft_Reset();
	}
	
	printf("\r\n>%d 连接成功，尝试发送数据。。。",index++);
	if(gsm_gprs_send(TESTBUFF1) != GSM_TRUE)
	{
		printf("\r\n TCP发送数据失败，请检测正确设置各个模块 XXXXXXXXXXXXXXX");
		GSM_DELAY(1000);
		printf("\r\n IP链接断开");
		GSM_DELAY(1000);
		gsm_gprs_link_close();
		
		printf("\r\n 关闭场景");
		GSM_DELAY(1000);
		gsm_gprs_shut_close();
		while(1);
	}
	
	printf("\r\n>%d 尝试发送第二条数据。。。",index++);
	if(gsm_gprs_send(TESTBUFF2) != GSM_TRUE)
	{
		printf("\r\n>TCP发送数据失败，请检测正确设置各个模块 XXXXXXXXXXXXXXX");
		GSM_DELAY(1000);
		printf("\r\n IP链接断开");
		GSM_DELAY(1000);
		gsm_gprs_link_close();
		
		printf("\r\n 关闭场景");
		GSM_DELAY(1000);
		gsm_gprs_shut_close();
		while(1);
	}
	printf ("\r\n>%d 发送第二条数据成功",index++);
	
	
	printf ("\r\n>%d 开始发送第三条数据",index++);
	
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
			printf("\r\n>TCP发送数据失败，请检测正确设置各个模块 XXXXXXXXXXXXXXX");
			GSM_DELAY(1000);
			printf("\r\n IP链接断开");
			GSM_DELAY(1000);
			gsm_gprs_link_close();
			
			printf("\r\n 关闭场景");
			GSM_DELAY(1000);
			gsm_gprs_shut_close();
			while(1);
		}
		
	}
	
	
	
	printf("\r\n>%d 已准备好接受远程数据，可使用网络调试助手发送数据",index++);
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
			printf("\r\n长时间无通讯，即将关闭网络");
			printf("\r\nIP连接断开");
			GSM_DELAY(100);
			gsm_gprs_link_close();
			
			printf("\r\n关闭场景");
			GSM_DELAY(100);
			gsm_gprs_shut_close();
			
			GSM_DELAY(1000);
			timestop = 0xFF;			
		}
		timecount++;
		GSM_DELAY(10);
	}
	
}
