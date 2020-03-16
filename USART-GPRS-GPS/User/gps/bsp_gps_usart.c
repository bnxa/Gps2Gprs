#include "bsp_gps_usart.h" 


//DMA接收缓冲区
uint8_t gps_rbuff[GPS_RBUFF_SIZE];

//DMA传输结束标志
__IO uint8_t GPS_TransferEnd = 0, GPS_HalfTransferEnd = 0;


//GPS用到的串口配置
static void GPS_USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	//使能GPIO时钟
	RCC_AHB1PeriphClockCmd(GPS_USART_RX_GPIO_CLK|GPS_USART_TX_GPIO_CLK,ENABLE);
	
	//使能USART时钟
	GPS_RCC_PeriphClockCmd_Fun(GPS_USART_CLK,ENABLE);
	
	//连接GPIO引脚到USART读
	GPIO_PinAFConfig(GPS_USART_RX_GPIO_PORT,GPS_USART_RX_SOURCE,GPS_USART_RX_AF);
	//连接GPIO引脚到USART写
	GPIO_PinAFConfig(GPS_USART_TX_GPIO_PORT,GPS_USART_TX_SOURCE,GPS_USART_TX_AF);
	
	//配置tx写引脚为复用功能
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = GPS_USART_TX_PIN;
	GPIO_Init(GPS_USART_TX_GPIO_PORT,&GPIO_InitStructure);
	
	//配置Rx 读引脚为复用功能
	GPIO_InitStructure.GPIO_Pin = GPS_USART_RX_PIN;
	GPIO_Init(GPS_USART_RX_GPIO_PORT,&GPIO_InitStructure);
	
	//配置串口GPS_USART模式
	USART_InitStructure.USART_BaudRate = GPS_USART_BAUDRATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(GPS_USART,&USART_InitStructure);
	
	//使能串口
	USART_Cmd(GPS_USART,ENABLE);	
}

//配置GPS使用的DMA中断
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

//GPS DMA 接收配置
static void GPS_DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	//开启DMA时钟
	RCC_AHB1PeriphClockCmd(GPS_USART_DMA_CLK,ENABLE);
	
	//复位初始化DMA数据流
	DMA_DeInit(GPS_USART_DMA_STREAM);
	
	//确保DMA数据流复位完成
	while(DMA_GetCmdStatus(GPS_USART_DMA_STREAM)!=DISABLE)
	{
	}
	
	//USART 对应DMA,通道，数据流
	DMA_InitStructure.DMA_Channel = GPS_USART_DMA_CHANNEL;
	//设置DMA源：串口数据寄存器地址
	DMA_InitStructure.DMA_PeripheralBaseAddr = GPS_DATA_ADDR;
	//内存地址 要传输的变量的指针
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)gps_rbuff;
	//方向： 从外设到内存
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	//传输大小
	DMA_InitStructure.DMA_BufferSize = GPS_RBUFF_SIZE;
	//外设地址不增
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	//内存地址自增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	//外设数据单位
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	//内存数据单位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	//DMA模式：不断循环
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	//优先级 中
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	//禁用FIFO
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	//存储器突发16个节拍
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	//外设突发1个节拍
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	
	//配置DMA2的数据流7
	DMA_Init(GPS_USART_DMA_STREAM,&DMA_InitStructure);
	
	//配置中断优先级
	GPS_Interrupt_Config();
	
	//配置DMA发送完成后产生中断
	DMA_ITConfig(GPS_USART_DMA_STREAM,DMA_IT_HT|DMA_IT_TC,ENABLE);
	
	//DMA 使能
	DMA_Cmd(GPS_USART_DMA_STREAM,ENABLE);
	
	//等待DMA数据流有效
	while(DMA_GetCmdStatus(GPS_USART_DMA_STREAM)!= ENABLE)
	{
	}
	
	//配置串口 向 DMA发出请求
	USART_DMACmd(GPS_USART,USART_DMAReq_Rx,ENABLE);
}

//GPS 初始化
void GPS_Config(void)
{
	GPS_USART_Config();
	GPS_DMA_Config();
}

//GPS DMA中断服务函数
void GPS_ProcessDMAIRQ(void)
{ 
	if(DMA_GetITStatus(GPS_USART_DMA_STREAM,GPS_DMA_IT_HT))	//DMA半传输完成
	{
		GPS_HalfTransferEnd =1; //设置半传输完成标志位
		DMA_ClearITPendingBit(GPS_USART_DMA_STREAM,GPS_DMA_IT_HT);		
	} 
	else if(DMA_GetITStatus(GPS_USART_DMA_STREAM,GPS_DMA_IT_TC))	//DMA 传输完成
	{
		GPS_TransferEnd =1;
		DMA_ClearITPendingBit(GPS_USART_DMA_STREAM,GPS_DMA_IT_TC);
//		printf("%s",gps_rbuff);
	}
}
