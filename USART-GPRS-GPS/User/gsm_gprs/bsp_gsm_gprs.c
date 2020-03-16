
//#include <stdlib.h>
//#include <stdarg.h>
#include <string.h>
#include "stm32f4xx.h"
#include "bsp_gsm_gprs.h"
#include "bsp_gsm_usart.h"


//0�ɹ�
//1ʧ��
uint8_t gsm_cmd_check(char *reply)
{
	uint8_t len;
	uint8_t n;
	uint8_t off;
	char *redata;
	GSM_DEBUG_FUNC();
	
	redata = GSM_RX(len);
	*(redata+len)='\0';
	GSM_DEBUG("�������� <---\r\n---------------\r\n%s---------------",redata);
//	GSM_DEBUG("���س���:=%d",len);
//	GSM_DEBUG("ƥ���ַ���:=%s,����:=%d",reply,strlen(reply));
	
	n = 0;
	off =0;
	while((n+off)<len)
	{ 
//		GSM_DEBUG("redata[%d]:=%c,%X,n:=%d,off:=%d,n+off:=%d",n+off,redata[n+off],redata[n+off],n,off,n+off);
		if(reply[n] == 0)  //����Ϊ�ջ��߱Ƚ����
		{
			GSM_DEBUG("����ֵ[%s]ƥ��ɹ�  GSM_TRUE",reply);
			return GSM_TRUE;
		}
		
		if(redata[n+off]==reply[n])
		{
			n++;  //�ƶ�����һ����������
		}
		else
		{
			off++;  //������һ��ƥ��
			n=0;    //����
		}
	}
	
	if(reply[n]==0) //�պ�ƥ�����
	{
		GSM_DEBUG("����ֵ���ƥ��ɹ� GSM_TRUE");
		return GSM_TRUE;
	}
		GSM_DEBUG("����ֵƥ��ʧ�� XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX GSM_FALSE");
	return GSM_FALSE; //����ѭ����ʾ�Ƚ���Ϻ�û����ͬ������
	
}

//0 �ɹ�
//1 ʧ��
uint8_t gsm_cmd(char *cmd,char *reply,uint32_t waittime)
{
	GSM_DEBUG_FUNC();
	GSM_CLEAN_RX();   //��ս��ܻ���������
	GSM_TX(cmd);  		//��������
	GSM_DEBUG("��������:--->%s",cmd);
	if(reply == 0)    //����Ҫ��������
	{
		return GSM_TRUE;
	}
	
	GSM_DELAY(waittime); //��ʱ
	return gsm_cmd_check(reply); //�Խ������ݽ��д���
}

//��ʼ�������ģ��
//0 �ɹ�
//1 ʧ��
uint8_t gsm_init(void)
{
	char *redata;
	uint8_t len;
	
	GSM_DEBUG_FUNC();
	
	GSM_CLEAN_RX(); //��ս��ջ��������� 
	GSM_USART_Config(); //��ʼ������
	
	if(gsm_cmd("AT+CGMM\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE;
	
	redata = GSM_RX(len);
	 
	if(len == 0)
		return GSM_FALSE;
	
	//���������GSM900A,GSM800A,GSM800C
	if(strstr(redata,"SIMCOM_GSM900A") != 0)
		return GSM_TRUE;
	else if(strstr(redata,"SIMCOM_SIM800") != 0)
		return GSM_TRUE;
	else
		return GSM_FALSE;
}

//����Ƿ��п� 
// 0 �ɹ�
// 1 ʧ�� 
uint8_t IsInsertCard(void)
{
	GSM_DEBUG_FUNC();
  GSM_CLEAN_RX();
	return gsm_cmd("AT+CNUM\r","OK",1000);
}


//�Ͽ���������
//ʧ�� GSM_FALSE
//�ɹ� GSM_TRUE
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

//�رճ���
//ʧ�� GSM_FALSE
//�ɹ� GSM_TRUE
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

//��ʼ��GPRS����
//ʧ�� GSM_FALSE
//�ɹ� GSM_TRUE
uint8_t gsm_gprs_init(void)
{
	GSM_DEBUG_FUNC();
	
	printf("\r\n >>1 ֻGPRS��������");
	GSM_CLEAN_RX();
	
	if(gsm_cmd("AT+CGCLASS=\"B\"\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE;
	
	printf("\r\n >>2 ����PDP�ƶ�����");
	GSM_CLEAN_RX();
	if(gsm_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE;
	
	GSM_CLEAN_RX();
	printf("\r\n >>3 ����GPRS���� ��ȡIP");
	if(gsm_cmd("AT+CGATT=1\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE;
	
	printf("\r\n >>4 ����ģ�����ӷ�ʽΪGPRS����,�����Ϊ��CMNET��");
	GSM_CLEAN_RX();
	if(gsm_cmd("AT+CIPCSGP=1,\"CMNET\"\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE;
		
	return GSM_TRUE;		
}


//����TCP���ӣ���ȴ�20�� 
//localport 	���ض˿�
//serverip  	������IP
//serverport	�������˿�
//ʧ��				GSM_FALSE
//�ɹ�				GSM_TRUE
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
	
	printf("\r\n >> ����Ƿ�������");
	while(gsm_cmd_check("CONNECT OK") != GSM_TRUE)
	{
		if(++testConnect>200) //��ȴ�20��
		{
			return GSM_FALSE;
		}
		GSM_DELAY(100);
	}
	return GSM_TRUE;
}

//����UDP���ӣ���ȴ�20�� 
//localport 	���ض˿�
//serverip  	������IP
//serverport	�������˿�
//ʧ��				GSM_FALSE
//�ɹ�				GSM_TRUE
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
	
	printf("\r\n >> ����Ƿ�������");
	while(gsm_cmd_check("CONNECT OK") != GSM_TRUE)
	{
		if(++testConnect>200)//��ȴ�20��
		{
			return GSM_FALSE;
		}
		GSM_DELAY(100);
	}
	return GSM_TRUE;
}

//ʹ��GPRS�������ݣ�����ǰҪ�Ƚ���TCP/UDP����
//ʧ��				GSM_FALSE
//�ɹ�				GSM_TRUE
uint8_t gsm_gprs_send(const char *str)
{
	char end = 0x1A;
	uint8_t testSend=0;
	
	GSM_CLEAN_RX();
	
	if(gsm_cmd("AT+CIPSEND\r",">",500) == GSM_TRUE)
	{
		GSM_USART_printf("%s",str);
		GSM_DEBUG("�����ַ���:=��%s��",str);
		
		GSM_CLEAN_RX();
		gsm_cmd(&end,0,100);
		
		printf("\r\n >> ����Ƿ������");
		while(gsm_cmd_check("SEND OK") != GSM_TRUE)
		{
			if(++testSend > 200)//��ȴ�20��
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
		gsm_cmd(&end,0,0); //ESC,ȡ������
		return GSM_FALSE;
	}
}
//ʹ��GPRS�������ݣ�����ǰҪ�Ƚ���TCP/UDP����
//ʧ��				GSM_FALSE
//�ɹ�				GSM_TRUE
uint8_t gsm_gprs_send2(char *str)
{
	char end = 0x1A;
	uint8_t testSend=0;
	
	GSM_CLEAN_RX();
	
	if(gsm_cmd("AT+CIPSEND\r",">",500) == GSM_TRUE)
	{
		GSM_USART_printf("%s",str);
		GSM_DEBUG("�����ַ���:=��%s��",str);
		
		GSM_CLEAN_RX();
		gsm_cmd(&end,0,100);
		
		printf("\r\n >> ����Ƿ������");
		while(gsm_cmd_check("SEND OK") != GSM_TRUE)
		{
			if(++testSend > 200)//��ȴ�20��
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
		gsm_cmd(&end,0,0); //ESC,ȡ������
		return GSM_FALSE;
	}
}
//���ش�GSMģ���յ�������
//ʧ��				GSM_FALSE
//�ɹ�				GSM_TRUE
uint8_t PostGPRS(void)
{
	char *redata;
	uint8_t len;
	
	redata = GSM_RX(len); //��������
	if(len==0)
	{
		return GSM_FALSE;
	}
	
	printf("\r\n >> �յ�TCP����:=%s",redata);
	GSM_CLEAN_RX();

	return GSM_TRUE;	
}
