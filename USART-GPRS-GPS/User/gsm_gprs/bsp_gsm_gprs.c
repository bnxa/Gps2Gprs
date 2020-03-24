
//#include <stdlib.h>
//#include <stdarg.h>
#include <string.h>
#include "stm32f4xx.h"
#include "bsp_gsm_gprs.h" 
 
uint8_t imei_buff[IMEI_BUFF_SIZE]; 			//���IMEI�ŵ�����
uint8_t phone_buff[PHONE_BUFF_SIZE]; 		//��ŵ绰��������
uint8_t gprs_data_buff[GPRS_DATA_SIZE];	//���socketҪ���͵����ݵ�����


extern uint8_t gps_rbuff[GPS_RBUFF_SIZE];
extern uint8_t gps_rbuff_BDGSV[GPS_BDGSV_SIZE];	//�ɼ�����������Ϣ 
extern uint8_t gps_rbuff_GNGGA[GPS_GNGGA_SIZE]; //GPS/������λ��Ϣ
extern uint8_t gps_rbuff_GNGSA[GPS_GNGSA_SIZE]; //��ǰ������Ϣ
extern uint8_t gps_rbuff_GPGSV[GPS_GPGSV_SIZE];	//�ɼ� GPS ������Ϣ 
extern uint8_t gps_rbuff_GNRMC[GPS_GNRMC_SIZE];	//�Ƽ���λ��Ϣ
extern uint8_t gps_rbuff_GNVTG[GPS_GNVTG_SIZE];	//�����ٶ���Ϣ
extern uint8_t gps_rbuff_GNGLL[GPS_GNGLL_SIZE];	//���������Ϣ
extern uint8_t gps_rbuff_GNZDA[GPS_GNZDA_SIZE];	//��ǰʱ��(UTC 1 )��Ϣ 

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
	GSM_DEBUG("����:<---\r\n%s",redata);
//	GSM_DEBUG("���س���:=%d",len);
//	GSM_DEBUG("ƥ���ַ���:=%s,����:=%d",reply,strlen(reply));
	
	n = 0;
	off =0;
	while((n+off)<len)
	{ 
//		GSM_DEBUG("redata[%d]:=%c,%X,n:=%d,off:=%d,n+off:=%d",n+off,redata[n+off],redata[n+off],n,off,n+off);
		if(reply[n] == 0)  //����Ϊ�ջ��߱Ƚ����
		{
			GSM_DEBUG("�ɹ���");
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
		GSM_DEBUG("����ֵ[%s]���ƥ��ɹ� GSM_TRUE",reply);
		return GSM_TRUE;
	}
		
	GSM_DEBUG("���ش���Ӧ����[%s]",reply);
	return GSM_FALSE; //����ѭ����ʾ�Ƚ���Ϻ�û����ͬ������
	
}

//0 �ɹ�
//1 ʧ��
uint8_t gsm_cmd(char *cmd,char *reply,uint32_t waittime)
{
	GSM_DEBUG_FUNC();
	GSM_CLEAN_RX();   //��ս��ܻ���������
	GSM_TX(cmd);  		//��������
	GSM_DEBUG("\r\n��������:--->%s",cmd);
	if(reply == 0)    //����Ҫ��������
	{
		return GSM_TRUE;
	}
	
	GSM_DELAY(waittime); //��ʱ
	return gsm_cmd_check(reply); //�Խ������ݽ��д���
}

//�����豸
uint8_t gsm_Reset(void)
{ 
	GSM_CLEAN_RX(); //��ս��ջ��������� 
	 
	if(gsm_cmd("ATE0&W\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE; 
	else
		return GSM_TRUE;
}

//��ѯ�ź�ǿ��
uint8_t gsm_sigin(void)
{
	GSM_CLEAN_RX(); //��ս��ջ���������  
	 
	if(gsm_cmd("AT+CSQ\r","OK",2000) != GSM_TRUE)
		return GSM_FALSE; 
	else
		return GSM_TRUE;
}
 

//��ѯGPRS����
uint8_t gsm_has_gprs(void)
{
	GSM_CLEAN_RX(); //��ս��ջ���������  
	 
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


//��ʼ�������ģ��
//0 �ɹ�
//1 ʧ��
uint8_t gsm_init(void)
{
	char *redata;
	uint8_t len;
	
	GSM_DEBUG_FUNC();
	
	GSM_CLEAN_RX(); //��ս��ջ���������  
	
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

//��ȡIMEI��
uint8_t GetIMEI(void)
{
	char *redata;
	uint8_t len;
	memset(imei_buff,0,IMEI_BUFF_SIZE);
	
	GSM_CLEAN_RX(); //��ս��ջ���������  
	
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
				
				GSM_DEBUG(" >> ��ȡIMEI�ɹ��� %s ���ȣ�%d",imei_buff,strlen((char*)imei_buff));
				return GSM_TRUE;
			} 
		}
	}
	
	return GSM_FALSE;
}

//����Ƿ��п� 
// 0 �ɹ�
// 1 ʧ�� 
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
				GSM_DEBUG(" >> ��ȡ�ֻ��ųɹ���%s ���ȣ�%d",phone_buff,strlen((char*)phone_buff));
				return GSM_TRUE;
			}
		} 
	}
	return GSM_TRUE;
}


//�Ͽ���������
//ʧ�� GSM_FALSE
//�ɹ� GSM_TRUE
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

//�رճ���
//ʧ�� GSM_FALSE
//�ɹ� GSM_TRUE
uint8_t gsm_gprs_shut_close(void)
{
	uint8_t check_time=0;
	GSM_DEBUG_FUNC();
	
	GSM_CLEAN_RX();
	gsm_cmd("AT+CIPSHUT\r",0,0);
	
	GSM_DELAY(200); //��ʱ
	
	while(gsm_cmd_check("SHUT OK") != GSM_TRUE)
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
	int fIndex = 1;
	
	GSM_DEBUG_FUNC();
	
	printf("\r\n >>1 ֻGPRS��������");
	GSM_CLEAN_RX();	
	if(gsm_cmd("AT+CGCLASS=\"B\"\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE;
	LED1_Light_Fast(fIndex++);
	
	printf("\r\n >>2 ����PDP�ƶ�����");
	GSM_CLEAN_RX();
	if(gsm_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE;
	
	LED1_Light_Fast(fIndex++);
	
	if(gsm_has_gprs() == GSM_FALSE)
	{
		GSM_CLEAN_RX();
		printf("\r\n >>3 ����GPRS���� ��ȡIP");
		if(gsm_cmd("AT+CGATT=1\r","OK",2000) != GSM_TRUE)
			return GSM_FALSE;
		
		
		LED1_Light_Fast(fIndex++);
	}
	else
	{
		LED1_Light_Fast(fIndex++);
	}
	
	
	printf("\r\n >>%d ����ģ�����ӷ�ʽΪGPRS����,�����Ϊ��CMNET��",fIndex);
	GSM_CLEAN_RX();
	if(gsm_cmd("AT+CIPCSGP=1,\"CMNET\"\r","OK",1000) != GSM_TRUE)
		return GSM_FALSE; 
	LED1_Light_Fast(fIndex++);
	
	
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
		if(++testConnect>10) //��ȴ�20��
		{
			return GSM_FALSE;
		}
		GSM_DELAY(2000);
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
		if(++testConnect>10)//��ȴ�20��
		{
			return GSM_FALSE;
		}
		GSM_DELAY(2000); 
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
uint8_t gsm_gprs_send_GpsCmd(uint8_t *str)
{
	char end = 0x1A;
	uint8_t testSend=0;
	
	GSM_CLEAN_RX();
	
	if(gsm_cmd("AT+CIPSEND\r",">",200) == GSM_TRUE)
	{
		GSM_USART_printf("%s",str);
		GSM_DEBUG("�����ַ���:=��%s��",str);
		
		GSM_CLEAN_RX();
		gsm_cmd(&end,0,100);
		
		printf("\r\n >> ����Ƿ������");
		while(gsm_cmd_check("SEND OK") != GSM_TRUE)
		{
			if(++testSend > 20)//��ȴ�20��
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

//��ȡGPRS����
uint8_t get_gprs_data(void)
{
	uint16_t len =0;
	uint16_t len3 =0;
	//��ʼ���������ݻ�����
	memset(gprs_data_buff,0,GPRS_DATA_SIZE);
	
	//ƴ��IMEI��
	strcat((char*)gprs_data_buff,"$");
	strcat((char*)gprs_data_buff,(char*)imei_buff);
	len = strlen((char*) gprs_data_buff);
	GSM_DEBUG("gprs_data_buff:=%s , len=%d",gprs_data_buff,len); 
	
	//ƴ���ֻ���
	strcat((char*)gprs_data_buff,"$");
	strcat((char*)gprs_data_buff,(char*)phone_buff);
	len = strlen((char*) gprs_data_buff);
	GSM_DEBUG("gprs_data_buff:=%s , len=%d",gprs_data_buff,len); 
	
	//ƴ��GPS����
	strcat((char*)gprs_data_buff,(char*)gps_rbuff_GNRMC);
	len3 = strlen((char*) gprs_data_buff);
	GSM_DEBUG("gprs_data_buff:=%s , len=%d",gprs_data_buff,len3);
	if(len3 > len)
		return GSM_TRUE;
	else
		return GSM_FALSE; 
}
