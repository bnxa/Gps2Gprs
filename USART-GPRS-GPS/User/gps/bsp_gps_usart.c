#include "bsp_gps_usart.h" 


//DMA���ջ�����
uint8_t gps_rbuff[GPS_RBUFF_SIZE];

uint8_t gps_rbuff_GNGGA[GPS_GNGGA_SIZE]; //GPS/������λ��Ϣ
uint8_t gps_rbuff_GNGSA[GPS_GNGSA_SIZE]; //��ǰ������Ϣ
uint8_t gps_rbuff_GPGSV[GPS_GPGSV_SIZE];	//�ɼ� GPS ������Ϣ 
uint8_t gps_rbuff_BDGSV[GPS_BDGSV_SIZE];	//�ɼ�����������Ϣ
uint8_t gps_rbuff_GNRMC[GPS_GNRMC_SIZE];	//�Ƽ���λ��Ϣ
uint8_t gps_rbuff_GNVTG[GPS_GNVTG_SIZE];	//�����ٶ���Ϣ
uint8_t gps_rbuff_GNGLL[GPS_GNGLL_SIZE];	//���������Ϣ
uint8_t gps_rbuff_GNZDA[GPS_GNZDA_SIZE];	//��ǰʱ��(UTC 1 )��Ϣ 


//DMA���������־
__IO uint8_t GPS_TransferEnd = 0, GPS_HalfTransferEnd = 0;


//GPS�õ��Ĵ�������
static void GPS_USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	//ʹ��GPIOʱ��
	RCC_AHB1PeriphClockCmd(GPS_USART_RX_GPIO_CLK|GPS_USART_TX_GPIO_CLK,ENABLE);
	
	//ʹ��USARTʱ��
	GPS_RCC_PeriphClockCmd_Fun(GPS_USART_CLK,ENABLE);
	
	//����GPIO���ŵ�USART��
	GPIO_PinAFConfig(GPS_USART_RX_GPIO_PORT,GPS_USART_RX_SOURCE,GPS_USART_RX_AF);
	//����GPIO���ŵ�USARTд
	GPIO_PinAFConfig(GPS_USART_TX_GPIO_PORT,GPS_USART_TX_SOURCE,GPS_USART_TX_AF);
	
	//����txд����Ϊ���ù���
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = GPS_USART_TX_PIN;
	GPIO_Init(GPS_USART_TX_GPIO_PORT,&GPIO_InitStructure);
	
	//����Rx ������Ϊ���ù���
	GPIO_InitStructure.GPIO_Pin = GPS_USART_RX_PIN;
	GPIO_Init(GPS_USART_RX_GPIO_PORT,&GPIO_InitStructure);
	
	//���ô���GPS_USARTģʽ
	USART_InitStructure.USART_BaudRate = GPS_USART_BAUDRATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(GPS_USART,&USART_InitStructure);
	
	//ʹ�ܴ���
	USART_Cmd(GPS_USART,ENABLE);	
}

//����GPSʹ�õ�DMA�ж�
static void GPS_Interrupt_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	//DMA2
	NVIC_InitStructure.NVIC_IRQChannel = GPS_DMA_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//GPS DMA ��������
static void GPS_DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	//����DMAʱ��
	RCC_AHB1PeriphClockCmd(GPS_USART_DMA_CLK,ENABLE);
	
	//��λ��ʼ��DMA������
	DMA_DeInit(GPS_USART_DMA_STREAM);
	
	//ȷ��DMA��������λ���
	while(DMA_GetCmdStatus(GPS_USART_DMA_STREAM)!=DISABLE)
	{
	}
	
	//USART ��ӦDMA,ͨ����������
	DMA_InitStructure.DMA_Channel = GPS_USART_DMA_CHANNEL;
	//����DMAԴ���������ݼĴ�����ַ
	DMA_InitStructure.DMA_PeripheralBaseAddr = GPS_DATA_ADDR;
	//�ڴ��ַ Ҫ����ı�����ָ��
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)gps_rbuff;
	//���� �����赽�ڴ�
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	//�����С
	DMA_InitStructure.DMA_BufferSize = GPS_RBUFF_SIZE;
	//�����ַ����
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	//�ڴ��ַ����
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	//�������ݵ�λ
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	//�ڴ����ݵ�λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	//DMAģʽ������ѭ��
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// DMA_Mode_Circular; //
	//���ȼ� ��
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	//����FIFO
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	//�洢��ͻ��16������
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	//����ͻ��1������
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	
	//����DMA2��������7
	DMA_Init(GPS_USART_DMA_STREAM,&DMA_InitStructure);
	
	//�����ж����ȼ�
	GPS_Interrupt_Config();
	
	//����DMA������ɺ�����ж�
//	DMA_ITConfig(GPS_USART_DMA_STREAM,DMA_IT_HT|DMA_IT_TC,ENABLE);
	DMA_ITConfig(GPS_USART_DMA_STREAM,DMA_IT_TC,ENABLE);
	
	//DMA ʹ��
	DMA_Cmd(GPS_USART_DMA_STREAM,ENABLE);
	
	//�ȴ�DMA��������Ч
	while(DMA_GetCmdStatus(GPS_USART_DMA_STREAM)!= ENABLE)
	{
	}
	
	//���ô��� �� DMA��������
	USART_DMACmd(GPS_USART,USART_DMAReq_Rx,ENABLE);
}

//GPS ��ʼ��
void GPS_Config(void)
{
	GPS_USART_Config();
	GPS_DMA_Config();
}

//GPS DMA�жϷ�����
void GPS_ProcessDMAIRQ(void)
{ 
//	if(DMA_GetITStatus(GPS_USART_DMA_STREAM,GPS_DMA_IT_HT))	//DMA�봫�����
//	{
//		GPS_HalfTransferEnd =1; //���ð봫����ɱ�־λ
//		DMA_ClearITPendingBit(GPS_USART_DMA_STREAM,GPS_DMA_IT_HT);		
//	} 
//	else 
		
	if(DMA_GetITStatus(GPS_USART_DMA_STREAM,GPS_DMA_IT_TC))	//DMA �������
	{
		GPS_TransferEnd =1;
		DMA_ClearITPendingBit(GPS_USART_DMA_STREAM,GPS_DMA_IT_TC);
		GPS_DEBUG("  DMA �������");
		GPS_DEBUG("%s",gps_rbuff);
		nema_decode();
		
	}
}

//1 ��� GNGGA ����
void Clear_GNGGA(void)
{
	
	uint16_t i=GPS_GNGGA_SIZE+1; 
	while(i)
		gps_rbuff_GNGGA[--i]=0;
}

//2 ��� GNGSA ����
void Clear_GNGSA(void)
{
	
	uint16_t i=GPS_GNGSA_SIZE+1; 
	while(i)
		gps_rbuff_GNGSA[--i]=0;
}

//3 ��� GPGSV ����
void Clear_GPGSV(void)
{
	
	uint16_t i=GPS_GPGSV_SIZE+1; 
	while(i)
		gps_rbuff_GPGSV[--i]=0;
}

//4 ��� BDGSV ����
void Clear_BDGSV(void)
{
	
	uint16_t i=GPS_BDGSV_SIZE+1; 
	while(i)
		gps_rbuff_BDGSV[--i]=0;
}

//5 ��� GNRMC ����
void Clear_GNRMC(void)
{
	
	uint16_t i=GPS_GNRMC_SIZE+1; 
	while(i)
		gps_rbuff_GNRMC[--i]=0;
}

//6 ��� GNVTG ����
void Clear_GNVTG(void)
{
	
	uint16_t i=GPS_GNVTG_SIZE+1; 
	while(i)
		gps_rbuff_GNVTG[--i]=0;
}

//7 ��� GNGLL ����
void Clear_GNGLL(void)
{
	
	uint16_t i=GPS_GNGLL_SIZE+1; 
	while(i)
		gps_rbuff_GNGLL[--i]=0;
}

//8 ��� GNZDA ����
void Clear_GNZDA(void)
{
	
	uint16_t i=GPS_GNZDA_SIZE+1; 
	while(i)
		gps_rbuff_GNZDA[--i]=0;
}





