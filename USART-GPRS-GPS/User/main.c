#include "stm32f4xx.h"
#include "bsp_led.h"
#include "bsp_debug_usart.h"
#include "bsp_SysTick.h"
#include "bsp_gsm_gprs.h"
#include "bsp_gps_usart.h"


//调试信息开关
#define DEBUG_ON 1 
#define DEBUG(fmt,arg...) do{\
																	if(DEBUG_ON)\
																		printf("\r\n"fmt,##arg);\
																}while(0)


																
																
#define		LOCALPORT "2000"
#define		SERVERIP	"28d77e1773.zicp.vip"
#define		SERVERPORT	"37164"

const char *TESTBUFF1="\r\n 1. SIM800A GSM模块TCP数据上传功能测试";
const char *TESTBUFF2="\r\n 2. SIM800A GSM模块TCP数据上传功能测试";

//static void Show_Message(void);
extern uint8_t gps_rbuff[GPS_RBUFF_SIZE];
extern uint8_t gps_rbuff_BDGSV[GPS_BDGSV_SIZE];	//可见北斗卫星信息 
extern uint8_t gps_rbuff_GNGGA[GPS_GNGGA_SIZE]; //GPS/北斗定位信息
extern uint8_t gps_rbuff_GNGSA[GPS_GNGSA_SIZE]; //当前卫星信息
extern uint8_t gps_rbuff_GPGSV[GPS_GPGSV_SIZE];	//可见 GPS 卫星信息 
extern uint8_t gps_rbuff_GNRMC[GPS_GNRMC_SIZE];	//推荐定位信息
extern uint8_t gps_rbuff_GNVTG[GPS_GNVTG_SIZE];	//地面速度信息
extern uint8_t gps_rbuff_GNGLL[GPS_GNGLL_SIZE];	//大地坐标信息
extern uint8_t gps_rbuff_GNZDA[GPS_GNZDA_SIZE];	//当前时间(UTC 1 )信息 



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
	 
	
	DEBUG();
	DEBUG();
	DEBUG();
	DEBUG();
	DEBUG("\r\n********************************************************************\r\n");
	DEBUG("\r\n***********************GSM模块TCP收发示例程序***********************\r\n");
	DEBUG("\r\n********************************************************************\r\n");
	
	DEBUG(">%d 正在等待GSM模块 重启设备 。。。\r\n",index++);
	while(gsm_Reset()!=GSM_TRUE)
	{ 
		DEBUG(">%d 重启设备失败 ，正在等待GSM模块 重启设备。。。\r\n",index++);
	}
	
	DEBUG(">%d 正在等待GSM模块初始化。。。\r\n",index++);
	while(gsm_init()!= GSM_TRUE)
	{
		DEBUG("\r\n模块响应测试不正常！!");
		DEBUG("\r\n若模块相应一直不正常，请检查模块连接线和电源");
	}
	
	DEBUG(">%d 正在检测电话卡。。。\r\n",index++);
	while(IsInsertCard() != GSM_TRUE)
	{
		if(++testCard>20)
		{
			DEBUG("\r\n检测不到电话卡，请断电后重新插入电话卡\r\n");
		}
		GSM_DELAY(1000);
	}
	GSM_DELAY(1000);
	
	//确认关闭之前的连接
	DEBUG(">%d 确认关闭之前的连接!",index++);
	gsm_gprs_link_close();
	
	GSM_DELAY(1000);
	
	//确认关闭之前的场景
	DEBUG(">%d 确认关闭之前的场景!",index++);
	gsm_gprs_shut_close();
 
	//重新初始化GPRS
	DEBUG(">%d 重新初始化GPRS!",index++);
	if(gsm_gprs_init()!= GSM_TRUE) //gprs初始化环境
	{
		DEBUG("\r\n初始化GPRS失败，请重新给模块上电并复位开发板");
		while(1);
	}
	
	DEBUG(">%d 尝试建立TCP链接，请耐心等待。。。",index++);
	
	if(gsm_gprs_tcp_link(LOCALPORT,SERVERIP,SERVERPORT) != GSM_TRUE)
	{
		DEBUG("\r\n TCP链接失败，请检测正确设置各个模块 XXXXXXXXXXXXXXX");
		GSM_DELAY(1000);
		DEBUG("\r\n IP链接断开");
		GSM_DELAY(1000);
		gsm_gprs_link_close();
		
		DEBUG("\r\n 关闭场景");
		GSM_DELAY(1000);
		gsm_gprs_shut_close();
		DEBUG("\r\n ********5s后自动重启**********");
		GSM_DELAY(5000);
		Soft_Reset();
	}
	
	DEBUG(">%d 连接成功，尝试发送数据。。。",index++);
	if(gsm_gprs_send(TESTBUFF1) != GSM_TRUE)
	{
		DEBUG("\r\n TCP发送数据失败，请检测正确设置各个模块 XXXXXXXXXXXXXXX");
		GSM_DELAY(1000);
		DEBUG("\r\n IP链接断开");
		GSM_DELAY(1000);
		gsm_gprs_link_close();
		
		DEBUG("\r\n 关闭场景");
		GSM_DELAY(1000);
		gsm_gprs_shut_close();
		while(1);
	}
	
	DEBUG(">%d 尝试发送第二条数据。。。",index++);
	if(gsm_gprs_send(TESTBUFF2) != GSM_TRUE)
	{
		DEBUG("\r\n>TCP发送数据失败，请检测正确设置各个模块 XXXXXXXXXXXXXXX");
		GSM_DELAY(1000);
		DEBUG("\r\n IP链接断开");
		GSM_DELAY(1000);
		gsm_gprs_link_close();
		
		DEBUG("\r\n 关闭场景");
		GSM_DELAY(1000);
		gsm_gprs_shut_close();
		while(1);
	}
	DEBUG(">%d 发送第二条数据成功",index++);
	
	
	DEBUG(">%d 开始发送第三条数据",index++);
	 
		
	GSM_DELAY(2000);
		 
	DEBUG("\r\n %s",gps_rbuff_BDGSV);
	if(gsm_gprs_send_GpsCmd(gps_rbuff_BDGSV) != GSM_TRUE)
	{
		DEBUG("\r\n>TCP发送数据失败，请检测正确设置各个模块 XXXXXXXXXXXXXXX");
		GSM_DELAY(1000);
		DEBUG("\r\n IP链接断开");
		GSM_DELAY(1000);
		gsm_gprs_link_close();
		
		DEBUG("\r\n 关闭场景");
		GSM_DELAY(1000);
		gsm_gprs_shut_close();
		while(1);
	}		 

	GSM_DELAY(10000);
 
	DEBUG("\r\n即将关闭网络");
	DEBUG("\r\nIP连接断开");
	GSM_DELAY(100);
	gsm_gprs_link_close();
	
	DEBUG("\r\n关闭场景");
	GSM_DELAY(100);
	gsm_gprs_shut_close();
 
	
}
