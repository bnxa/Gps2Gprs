#include "bsp_gsm_usart.h"
 
#include <stdarg.h>


//中断缓存串口数据
#define UART_BUFF_SIZE	512
volatile uint16_t uart_p = 0;
uint8_t  uart_buff[UART_BUFF_SIZE];

void bsp_GSM_USART_IRQHandler(void)
{
	if(uart_p < UART_BUFF_SIZE)
	{
		if(USART_GetFlagStatus(GSM_USARTx,USART_IT_RXNE) != RESET)
		{
			uart_buff[uart_p] = USART_ReceiveData(GSM_USARTx);
			uart_p++;
		}
	}
}

//配置USART6中断函数
static void GSM_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	//使能中断
	NVIC_InitStructure.NVIC_IRQChannel = GSM_USART_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void GSM_USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	//配置gsm串口时钟
	RCC_AHB1PeriphClockCmd(GSM_USART_GPIO_CLK,ENABLE);
	GSM_USART_APBxClkCmd(GSM_USART_CLK,ENABLE);
	
	//连接引脚GPIOC-Pin7 到串口usart6 RX 收
	GPIO_PinAFConfig(GSM_RX_GPIO_PORT,GSM_RX_SOURCE,GSM_RX_AF);
	//连接引脚GPIOC-Pin6 到串口usart6 TX 发
	GPIO_PinAFConfig(GSM_TX_GPIO_PORT,GSM_TX_SOURCE,GSM_TX_AF);
	
	//配置Tx引脚为复用功能
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	
	GPIO_InitStructure.GPIO_Pin = GSM_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GSM_TX_GPIO_PORT,&GPIO_InitStructure);
	
	//配置Rx引脚为复用功能
	GPIO_InitStructure.GPIO_Pin = GSM_RX_GPIO_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GSM_RX_GPIO_PORT,&GPIO_InitStructure);
	
	//GSM串口USART6配置
	USART_InitStructure.USART_BaudRate=GSM_USART_BAUDRATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl= USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(GSM_USARTx,&USART_InitStructure);
	
	//配置中断优先级
	GSM_NVIC_Config();
	
	//使能串口6接收中断
	USART_ITConfig(GSM_USARTx,USART_IT_RXNE,ENABLE);
	
	//使能串口6
	USART_Cmd(GSM_USARTx,ENABLE);
	
}



//获取接收到的数据和长度
char *get_rebuff(uint8_t *len)
{
	*len = uart_p;
	return (char *)&uart_buff;
}

//清空接收数据缓冲区
void clean_rebuff(void)
{
	uint16_t i=UART_BUFF_SIZE+1;
	uart_p =0;
	while(i)
		uart_buff[--i]=0;
}


/*将整形数据转换成字符串
 * 函数名：itoa
 * 描述  ：将整形数据转换成字符串
 * 输入  ：-radix =10 表示10进制，其他结果为0
 *         -value 要转换的整形数
 *         -buf 转换后的字符串
 *         -radix = 10
 * 输出  ：无
 * 返回  ：无
 * 调用  ：被GSM_USARTx_printf()调用
 */ 
static char *itoa(int value, char *string, int radix)
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */


/* 描述： 格式化输出，类似于c库中的printf,但这里没有用到C库
 * 
 * 输入： -USARTx 串口通道 串口6 即GSM_USARTx
					-Data 要发送到串口的内容的指针
					-... 其他参数
 * 示例： GSM_USARTx_printf(GSM_USARTx,"\r\n this is a demo \r\n");
 *        GSM_USARTx_printf(GSM_USARTx, "\r\n %d \r\n", i );
*/
void GSM_USART_printf(char *Data,...)
{
	const char *s;
	int d;
	char buf[16];
	
	va_list ap;
	va_start(ap,Data);
	
	while(*Data !=0) //判断是否到达字符串结束符
	{
		if(*Data == 0x5c) // '\'
		{
			switch(*++Data)
			{
				case 'r': //回车
					USART_SendData(GSM_USARTx,0x0d);
					Data++;
					break;
				
				case 'n': //换行
					USART_SendData(GSM_USARTx,0x0a);
					Data++;
					break;
				
				default:
					Data++;
					break;
			}
		}
		else if(*Data == '%')
		{
			switch(*++Data)
			{
				case 's':  //字符串
					s = va_arg(ap,const char *);
					for(;*s;s++)
					{
						USART_SendData(GSM_USARTx,*s);
						while(USART_GetFlagStatus(GSM_USARTx,USART_FLAG_TXE) == RESET);
					}
					Data++;
					break;
					
				case 'd': //十进制
					d = va_arg(ap,int);
					itoa(d,buf,10);
					for(s=buf;*s;s++)
					{
						USART_SendData(GSM_USARTx,*s);
						while(USART_GetFlagStatus(GSM_USARTx,USART_FLAG_TXE)==RESET);
					}
					Data++;
					break;
				
				 defaut:
					Data++;
					break;
			}
		}
		else
			USART_SendData(GSM_USARTx,*Data++);
		while(USART_GetFlagStatus(GSM_USARTx,USART_FLAG_TXE)==RESET);
	}
	
}
