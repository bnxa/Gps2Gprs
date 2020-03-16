#include "bsp_gps_usart.h" 


//DMA���ջ�����
uint8_t gps_rbuff[GPS_RBUFF_SIZE];

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
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
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
	DMA_ITConfig(GPS_USART_DMA_STREAM,DMA_IT_HT|DMA_IT_TC,ENABLE);
	
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
	if(DMA_GetITStatus(GPS_USART_DMA_STREAM,GPS_DMA_IT_HT))	//DMA�봫�����
	{
		GPS_HalfTransferEnd =1; //���ð봫����ɱ�־λ
		DMA_ClearITPendingBit(GPS_USART_DMA_STREAM,GPS_DMA_IT_HT);		
	} 
	else if(DMA_GetITStatus(GPS_USART_DMA_STREAM,GPS_DMA_IT_TC))	//DMA �������
	{
		GPS_TransferEnd =1;
		DMA_ClearITPendingBit(GPS_USART_DMA_STREAM,GPS_DMA_IT_TC);
//		printf("%s",gps_rbuff);
	}
}
