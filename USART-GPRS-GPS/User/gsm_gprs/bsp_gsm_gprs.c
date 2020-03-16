
//#include <stdlib.h>
//#include <stdarg.h>
#include <string.h>
#include "stm32f4xx.h"
#include "bsp_gsm_gprs.h"
#include "bsp_gsm_usart.h"


//0成功
//1失败
uint8_t gsm_cmd_check(char *reply)
{
	uint8_t len;
	uint8_t n;
	uint8_t off;
	char *redata;
	GSM_DEBUG_FUNC();
	
	redata = GSM_RX(len);
	*(redata+len)='\0';
	GSM_DEBUG("返回数据 <---\r\n---------------\r\n%s---------------",redata);
//	GSM_DEBUG("返回长度:=%d",len);
//	GSM_DEBUG("匹配字符串:=%s,长度:=%d",reply,strlen(reply));
	
	n = 0;
	off =0;
	while((n+off)<len)
	{ 
//		GSM_DEBUG("redata[%d]:=%c,%X,n:=%d,off:=%d,n+off:=%d",n+off,redata[n+off],redata[n+off],n,off,n+off);
		if(reply[n] == 0)  //数据为空或者比较完毕
		{
			GSM_DEBUG("返回值[%s]匹配成功  GSM_TRUE",reply);
			return GSM_TRUE;
		}
		
		if(redata[n+off]==reply[n])
		{
			n++;  //移动到下一个接收数据
		}
		else
		{
			off++;  //进行下一轮匹配
			n=0;    //重来
		}
	}
	
	if(reply[n]==0) //刚好匹配完毕
	{
		GSM_DEBUG("返回值最后匹配成功 GSM_TRUE");
		return GSM_TRUE;
	}
		GSM_DEBUG("返回值匹配失败 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX GSM_FALSE");
	return GSM_FALSE; //跳出循环表示比较完毕后没有相同的数据
	
}

//0 成功
//1 失败
uint8_t gsm_cmd(char *cmd,char *reply,uint32_t waittime)
{
	GSM_DEBUG_FUNC();
	GSM_CLEAN_RX();   //清空接受缓冲区数据
	GSM_TX(cmd);  		//发送命令
	GSM_DEBUG("发送命令:--->%s",cmd);
	if(reply == 0)    //不需要接收数据
	{
		return GSM_TRUE;
	}
	
	GSM_DELAY(waittime); //延时
	return gsm_cmd_check(reply); //对接收数据进行处理
}

//初始化并检测模块
//0 成功
//1 失败
uint8_t gsm_init(void)
{
	char *redata;
	uint8_t len;
	
	GSM_DEBUG_FUNC();
	
	GSM_CLEAN_RX(); //清空接收缓冲区数据 
	GSM_USART_Config(); //初始化串口
	
	if(gsm_cmd("AT+CGMM\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE;
	
	redata = GSM_RX(len);
	 
	if(len == 0)
		return GSM_FALSE;
	
	//本程序兼容GSM900A,GSM800A,GSM800C
	if(strstr(redata,"SIMCOM_GSM900A") != 0)
		return GSM_TRUE;
	else if(strstr(redata,"SIMCOM_SIM800") != 0)
		return GSM_TRUE;
	else
		return GSM_FALSE;
}

//检测是否有卡 
// 0 成功
// 1 失败 
uint8_t IsInsertCard(void)
{
	GSM_DEBUG_FUNC();
  GSM_CLEAN_RX();
	return gsm_cmd("AT+CNUM\r","OK",1000);
}


//断开网络连接
//失败 GSM_FALSE
//成功 GSM_TRUE
uint8_t gsm_gprs_link_close(void)
{
	GSM_DEBUG_FUNC();
	
	GSM_CLEAN_RX();
	if(gsm_cmd("AT+CIPCLOSE=1\r","OK",200) != GSM_TRUE)
	{
		return GSM_FALSE;
	}
	return GSM_TRUE;	
}

//关闭场景
//失败 GSM_FALSE
//成功 GSM_TRUE
uint8_t gsm_gprs_shut_close(void)
{
	uint8_t check_time=0;
	GSM_DEBUG_FUNC();
	
	GSM_CLEAN_RX();
	gsm_cmd("AT+CIPSHUT\r",0,0);
	while(gsm_cmd_check("OK") != GSM_TRUE)
	{
		if(++check_time>50)
			return GSM_FALSE;
		
		GSM_DELAY(1000);
	}
	return GSM_TRUE;
}

//初始化GPRS网络
//失败 GSM_FALSE
//成功 GSM_TRUE
uint8_t gsm_gprs_init(void)
{
	GSM_DEBUG_FUNC();
	
	printf("\r\n >>1 只GPRS工作类型");
	GSM_CLEAN_RX();
	
	if(gsm_cmd("AT+CGCLASS=\"B\"\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE;
	
	printf("\r\n >>2 定义PDP移动场景");
	GSM_CLEAN_RX();
	if(gsm_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE;
	
	GSM_CLEAN_RX();
	printf("\r\n >>3 激活GPRS功能 获取IP");
	if(gsm_cmd("AT+CGATT=1\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE;
	
	printf("\r\n >>4 设置模块连接方式为GPRS连接,接入点为“CMNET”");
	GSM_CLEAN_RX();
	if(gsm_cmd("AT+CIPCSGP=1,\"CMNET\"\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE;
		
	return GSM_TRUE;		
}


//建立TCP连接，最长等待20秒 
//localport 	本地端口
//serverip  	服务器IP
//serverport	服务器端口
//失败				GSM_FALSE
//成功				GSM_TRUE
uint8_t gsm_gprs_tcp_link(char *localport,char *serverip,char *serverport)
{
	char cmd_buf[100];
	uint8_t testConnect =0;
	
	sprintf(cmd_buf,"AT+CLPORT=\"TCP\",\"%s\"\r",localport);
	
	if(gsm_cmd(cmd_buf,"OK",1000) != GSM_TRUE)
		return GSM_FALSE;
	
	GSM_CLEAN_RX();
	
	sprintf(cmd_buf,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"\r",serverip,serverport);
	gsm_cmd(cmd_buf,0,1000);
	
	printf("\r\n >> 检测是否建立连接");
	while(gsm_cmd_check("CONNECT OK") != GSM_TRUE)
	{
		if(++testConnect>200) //最长等待20秒
		{
			return GSM_FALSE;
		}
		GSM_DELAY(100);
	}
	return GSM_TRUE;
}

//建立UDP连接，最长等待20秒 
//localport 	本地端口
//serverip  	服务器IP
//serverport	服务器端口
//失败				GSM_FALSE
//成功				GSM_TRUE
uint8_t gsm_gprs_udp_link(char *localport,char *serverip,char *serverport)
{
	char cmd_buf[100];
	uint8_t testConnect =0;
	
	sprintf(cmd_buf,"AT+CLPORT=\"UDP\",\"%s\"\r",localport);
	
	if(gsm_cmd(cmd_buf,"OK",1000)!=GSM_TRUE)
		return GSM_FALSE;
	
	GSM_CLEAN_RX();
	
	sprintf(cmd_buf,"AT+CIPSTART=\"UDP\",\"%s\",\"%s\"\r",serverip,serverport);
	
	gsm_cmd(cmd_buf,0,1000);
	
	printf("\r\n >> 检测是否建立连接");
	while(gsm_cmd_check("CONNECT OK") != GSM_TRUE)
	{
		if(++testConnect>200)//最长等待20秒
		{
			return GSM_FALSE;
		}
		GSM_DELAY(100);
	}
	return GSM_TRUE;
}

//使用GPRS发送数据，发送前要先建立TCP/UDP连接
//失败				GSM_FALSE
//成功				GSM_TRUE
uint8_t gsm_gprs_send(const char *str)
{
	char end = 0x1A;
	uint8_t testSend=0;
	
	GSM_CLEAN_RX();
	
	if(gsm_cmd("AT+CIPSEND\r",">",500) == GSM_TRUE)
	{
		GSM_USART_printf("%s",str);
		GSM_DEBUG("发送字符串:=“%s”",str);
		
		GSM_CLEAN_RX();
		gsm_cmd(&end,0,100);
		
		printf("\r\n >> 检测是否发送完成");
		while(gsm_cmd_check("SEND OK") != GSM_TRUE)
		{
			if(++testSend > 200)//最长等待20秒
			{
				goto gprs_send_failure;
			}
			GSM_DELAY(100);
		}
		return GSM_TRUE;		
	}
	{
		gprs_send_failure:
		end = 0x1B;
		gsm_cmd(&end,0,0); //ESC,取消发送
		return GSM_FALSE;
	}
}
//使用GPRS发送数据，发送前要先建立TCP/UDP连接
//失败				GSM_FALSE
//成功				GSM_TRUE
uint8_t gsm_gprs_send2(char *str)
{
	char end = 0x1A;
	uint8_t testSend=0;
	
	GSM_CLEAN_RX();
	
	if(gsm_cmd("AT+CIPSEND\r",">",500) == GSM_TRUE)
	{
		GSM_USART_printf("%s",str);
		GSM_DEBUG("发送字符串:=“%s”",str);
		
		GSM_CLEAN_RX();
		gsm_cmd(&end,0,100);
		
		printf("\r\n >> 检测是否发送完成");
		while(gsm_cmd_check("SEND OK") != GSM_TRUE)
		{
			if(++testSend > 200)//最长等待20秒
			{
				goto gprs_send_failure;
			}
			GSM_DELAY(100);
		}
		return GSM_TRUE;		
	}
	{
		gprs_send_failure:
		end = 0x1B;
		gsm_cmd(&end,0,0); //ESC,取消发送
		return GSM_FALSE;
	}
}
//返回从GSM模块收到的数据
//失败				GSM_FALSE
//成功				GSM_TRUE
uint8_t PostGPRS(void)
{
	char *redata;
	uint8_t len;
	
	redata = GSM_RX(len); //接收数据
	if(len==0)
	{
		return GSM_FALSE;
	}
	
	printf("\r\n >> 收到TCP数据:=%s",redata);
	GSM_CLEAN_RX();

	return GSM_TRUE;	
}
