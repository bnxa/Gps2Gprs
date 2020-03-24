
//#include <stdlib.h>
//#include <stdarg.h>
#include <string.h>
#include "stm32f4xx.h"
#include "bsp_gsm_gprs.h" 
 
uint8_t imei_buff[IMEI_BUFF_SIZE]; 			//存放IMEI号的数组
uint8_t phone_buff[PHONE_BUFF_SIZE]; 		//存放电话号码数组
uint8_t gprs_data_buff[GPRS_DATA_SIZE];	//存放socket要发送的数据的数组


extern uint8_t gps_rbuff[GPS_RBUFF_SIZE];
extern uint8_t gps_rbuff_BDGSV[GPS_BDGSV_SIZE];	//可见北斗卫星信息 
extern uint8_t gps_rbuff_GNGGA[GPS_GNGGA_SIZE]; //GPS/北斗定位信息
extern uint8_t gps_rbuff_GNGSA[GPS_GNGSA_SIZE]; //当前卫星信息
extern uint8_t gps_rbuff_GPGSV[GPS_GPGSV_SIZE];	//可见 GPS 卫星信息 
extern uint8_t gps_rbuff_GNRMC[GPS_GNRMC_SIZE];	//推荐定位信息
extern uint8_t gps_rbuff_GNVTG[GPS_GNVTG_SIZE];	//地面速度信息
extern uint8_t gps_rbuff_GNGLL[GPS_GNGLL_SIZE];	//大地坐标信息
extern uint8_t gps_rbuff_GNZDA[GPS_GNZDA_SIZE];	//当前时间(UTC 1 )信息 

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
	GSM_DEBUG("返回:<---\r\n%s",redata);
//	GSM_DEBUG("返回长度:=%d",len);
//	GSM_DEBUG("匹配字符串:=%s,长度:=%d",reply,strlen(reply));
	
	n = 0;
	off =0;
	while((n+off)<len)
	{ 
//		GSM_DEBUG("redata[%d]:=%c,%X,n:=%d,off:=%d,n+off:=%d",n+off,redata[n+off],redata[n+off],n,off,n+off);
		if(reply[n] == 0)  //数据为空或者比较完毕
		{
			GSM_DEBUG("成功！");
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
		GSM_DEBUG("返回值[%s]最后匹配成功 GSM_TRUE",reply);
		return GSM_TRUE;
	}
		
	GSM_DEBUG("返回错误，应返回[%s]",reply);
	return GSM_FALSE; //跳出循环表示比较完毕后没有相同的数据
	
}

//0 成功
//1 失败
uint8_t gsm_cmd(char *cmd,char *reply,uint32_t waittime)
{
	GSM_DEBUG_FUNC();
	GSM_CLEAN_RX();   //清空接受缓冲区数据
	GSM_TX(cmd);  		//发送命令
	GSM_DEBUG("\r\n发送命令:--->%s",cmd);
	if(reply == 0)    //不需要接收数据
	{
		return GSM_TRUE;
	}
	
	GSM_DELAY(waittime); //延时
	return gsm_cmd_check(reply); //对接收数据进行处理
}

//重启设备
uint8_t gsm_Reset(void)
{ 
	GSM_CLEAN_RX(); //清空接收缓冲区数据 
	 
	if(gsm_cmd("ATE0&W\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE; 
	else
		return GSM_TRUE;
}

//查询信号强度
uint8_t gsm_sigin(void)
{
	GSM_CLEAN_RX(); //清空接收缓冲区数据  
	 
	if(gsm_cmd("AT+CSQ\r","OK",2000) != GSM_TRUE)
		return GSM_FALSE; 
	else
		return GSM_TRUE;
}
 

//查询GPRS附着
uint8_t gsm_has_gprs(void)
{
	GSM_CLEAN_RX(); //清空接收缓冲区数据  
	 
	if(gsm_cmd("AT+CGATT?\r","OK",200) != GSM_TRUE)
		return GSM_FALSE; 
	else
	{ 
		uint8_t len; 
		char *redata;
		
		redata = GSM_RX(len); 
		if(strstr(redata,"+CGATT: 1") != NULL)
			return GSM_TRUE;
		else
			return GSM_FALSE;
	}
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

//获取IMEI号
uint8_t GetIMEI(void)
{
	char *redata;
	uint8_t len;
	memset(imei_buff,0,IMEI_BUFF_SIZE);
	
	GSM_CLEAN_RX(); //清空接收缓冲区数据  
	
	if(gsm_cmd("AT+GSN\r","OK",500) != GSM_TRUE)
		return GSM_FALSE;
	
	redata = GSM_RX(len);
	
	int fBegin = 0;
	int fEnd = 0;
	for(int i=0;i<len;i++)
	{
		if(redata[i]=='\n')
			fBegin = i++;
		if((redata[i] == '\r') && (fBegin > 0))
		{
			fEnd = i;
			len = fEnd-fBegin-1;
			if(len>=15)
			{
				for(int m=0;m<len;m++)
				{
					imei_buff[m]=redata[m+1+fBegin];
				}
				
				GSM_DEBUG(" >> 获取IMEI成功： %s 长度：%d",imei_buff,strlen((char*)imei_buff));
				return GSM_TRUE;
			} 
		}
	}
	
	return GSM_FALSE;
}

//检测是否有卡 
// 0 成功
// 1 失败 
uint8_t IsInsertCard(void)
{
	char *redata;
	uint8_t len;
	memset(phone_buff,0,PHONE_BUFF_SIZE);
	
	GSM_DEBUG_FUNC();
  GSM_CLEAN_RX();
	if(gsm_cmd("AT+CNUM\r","OK",500) != GSM_TRUE)
			return GSM_FALSE;
	
	redata = GSM_RX(len);
	int fBegin =0; 
	
	for(int i=0;i<len;i++)
	{
		if(redata[i]==',')
		{
			if(redata[++i] == '"')
			{
				fBegin = ++i;
			}
		}
		if((redata[i]=='"') && (fBegin>0))
		{
			if(i-fBegin>=11)
			{
				for(int m=0;m<i-fBegin;m++)
				{
					phone_buff[m]=redata[m+fBegin];
				}
				GSM_DEBUG(" >> 获取手机号成功：%s 长度：%d",phone_buff,strlen((char*)phone_buff));
				return GSM_TRUE;
			}
		} 
	}
	return GSM_TRUE;
}


//断开网络连接
//失败 GSM_FALSE
//成功 GSM_TRUE
uint8_t gsm_gprs_link_close(void)
{
	GSM_DEBUG_FUNC();
	
	GSM_CLEAN_RX();
	if(gsm_cmd("AT+CIPCLOSE=1\r","CLOSE OK",200) != GSM_TRUE)
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
	
	GSM_DELAY(200); //延时
	
	while(gsm_cmd_check("SHUT OK") != GSM_TRUE)
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
	int fIndex = 1;
	
	GSM_DEBUG_FUNC();
	
	printf("\r\n >>1 只GPRS工作类型");
	GSM_CLEAN_RX();	
	if(gsm_cmd("AT+CGCLASS=\"B\"\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE;
	LED1_Light_Fast(fIndex++);
	
	printf("\r\n >>2 定义PDP移动场景");
	GSM_CLEAN_RX();
	if(gsm_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE;
	
	LED1_Light_Fast(fIndex++);
	
	if(gsm_has_gprs() == GSM_FALSE)
	{
		GSM_CLEAN_RX();
		printf("\r\n >>3 激活GPRS功能 获取IP");
		if(gsm_cmd("AT+CGATT=1\r","OK",2000) != GSM_TRUE)
			return GSM_FALSE;
		
		
		LED1_Light_Fast(fIndex++);
	}
	else
	{
		LED1_Light_Fast(fIndex++);
	}
	
	
	printf("\r\n >>%d 设置模块连接方式为GPRS连接,接入点为“CMNET”",fIndex);
	GSM_CLEAN_RX();
	if(gsm_cmd("AT+CIPCSGP=1,\"CMNET\"\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE; 
	LED1_Light_Fast(fIndex++);
	
	
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
		if(++testConnect>10) //最长等待20秒
		{
			return GSM_FALSE;
		}
		GSM_DELAY(2000);
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
		if(++testConnect>10)//最长等待20秒
		{
			return GSM_FALSE;
		}
		GSM_DELAY(2000); 
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
uint8_t gsm_gprs_send_GpsCmd(uint8_t *str)
{
	char end = 0x1A;
	uint8_t testSend=0;
	
	GSM_CLEAN_RX();
	
	if(gsm_cmd("AT+CIPSEND\r",">",200) == GSM_TRUE)
	{
		GSM_USART_printf("%s",str);
		GSM_DEBUG("发送字符串:=“%s”",str);
		
		GSM_CLEAN_RX();
		gsm_cmd(&end,0,100);
		
		printf("\r\n >> 检测是否发送完成");
		while(gsm_cmd_check("SEND OK") != GSM_TRUE)
		{
			if(++testSend > 20)//最长等待20秒
			{
				goto gprs_send_failure;
			}
			GSM_DELAY(1000);
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

//获取GPRS数据
uint8_t get_gprs_data(void)
{
	uint16_t len =0;
	uint16_t len3 =0;
	//初始化发送数据缓冲区
	memset(gprs_data_buff,0,GPRS_DATA_SIZE);
	
	//拼接IMEI号
	strcat((char*)gprs_data_buff,"$");
	strcat((char*)gprs_data_buff,(char*)imei_buff);
	len = strlen((char*) gprs_data_buff);
	GSM_DEBUG("gprs_data_buff:=%s , len=%d",gprs_data_buff,len); 
	
	//拼接手机号
	strcat((char*)gprs_data_buff,"$");
	strcat((char*)gprs_data_buff,(char*)phone_buff);
	len = strlen((char*) gprs_data_buff);
	GSM_DEBUG("gprs_data_buff:=%s , len=%d",gprs_data_buff,len); 
	
	//拼接GPS数据
	strcat((char*)gprs_data_buff,(char*)gps_rbuff_GNRMC);
	len3 = strlen((char*) gprs_data_buff);
	GSM_DEBUG("gprs_data_buff:=%s , len=%d",gprs_data_buff,len3);
	if(len3 > len)
		return GSM_TRUE;
	else
		return GSM_FALSE; 
}
