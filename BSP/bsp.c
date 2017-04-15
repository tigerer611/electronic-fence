#include "includes.h"
#include "stm32f10x.h"	
#include "demo.h"	 
#include "serial.h"
uint8_t TxBuffer1[] = "USART Interrupt Example: This is USART1 DEMO";

void GPIO_Configuration(void);
void RCC_Configuration(void);
void USART1_Config(u32 baud);
void USART2_Config(u32 baud);
void USART_OUT(USART_TypeDef* USARTx, uint8_t *Data,...);
char *itoa(int value, char *string, int radix);
void tp_Config(void);
u16 TPReadX(void);
u16 TPReadY(void);
void NVIC_Configuration(void);
extern void FSMC_LCD_Init(void); 
extern void Delay(__IO uint32_t nCount);
void USART_OUTB(USART_TypeDef* USARTx, uint8_t *Data,uint16_t Len);
/****************************************************************************
* 名    称：void RCC_Configuration(void)
* 功    能：系统时钟配置为72MHZ， 外设时钟配置
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/ 
void RCC_Configuration(void){
  SystemInit();
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);			   //复用功能使能
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
                         RCC_APB2Periph_GPIOE, ENABLE);
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2, ENABLE); 			//使能串口2时钟 
}
/****************************************************************************
* 名    称：void NVIC_Configuration(void)
* 功    能：中断源配置
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void NVIC_Configuration(void)
{ 
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  /* Configure DMA1_Channel_Tx interrupt */
  NVIC_SetPriority(DMA1_Channel_Tx_IRQn, 0x01);
  NVIC_EnableIRQ(DMA1_Channel_Tx_IRQn);

  /* Configure USART1 interrupt */
  NVIC_SetPriority(USART1_IRQn, 0x00);
  NVIC_EnableIRQ(USART1_IRQn);

}

/****************************************************************************
* 名    称：void USART2_Config(u32 baud)
* 功    能：串口2配置
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void USART2_Config(u32 baud){
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	         		 	    //USART2 TX
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;    		        //复用推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);		    		        //A端口 

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	         	 	        //USART2 RX
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   	        //复用开漏输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);	

  USART_InitStructure.USART_BaudRate = baud;						//速率
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//数据位8位
  USART_InitStructure.USART_StopBits = USART_StopBits_1;			//停止位1位
  USART_InitStructure.USART_Parity = USART_Parity_No;				//无校验位
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;   //无硬件流控
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式

  /* Configure USART2 */
  USART_Init(USART2, &USART_InitStructure);							//配置串口参数函数   

  /*配置USART1 */
  USART_Init(USART2,&USART_InitStructure);							//配置串口参数函数   
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);                    //使能接收中断
  USART_ITConfig(USART2, USART_IT_TXE, ENABLE);						//使能发送缓冲空中断   
   /* 使能USART2 */
  USART_Cmd(USART2, ENABLE);	
  
}


/****************************************************************************
* 名    称：void BSP_Init(void)
* 功    能：奋斗板初始化函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/  
void BSP_Init(void)
{ 
  Rst_Buf=0;					   //串口2缓存延时复位
  Rst1_Buf=0;					   //串口1缓存延时复位
  RCC_Configuration();  	       //系统时钟初始化	
  NVIC_Configuration(); 		   //中断源配置
	SER_Config();	
  USART2_Config(38400);            //初始化串口2         
  /* 显示例程Logo */
  USART_OUT(USART1,"**** (C) COPYRIGHT 2013 奋斗嵌入式开发工作室 *******\r\n");    	  //向串口1发送开机字符。
  USART_OUT(USART1,"*                                                 *\r\n");    	
  USART_OUT(USART1,"*  奋斗版STM32开发板zigbee/RS-485/USART2通信实验  *\r\n");  
  USART_OUT(USART1,"*                                                 *\r\n");    	  	
  USART_OUT(USART1,"*     MCU平台:STM32F103VET6                       *\r\n");   	  	
  USART_OUT(USART1,"*     固件库：3.5                                 *\r\n");    	
  USART_OUT(USART1,"*     例程版本: 0.1                               *\r\n");  
  USART_OUT(USART1,"*     奋斗STM32店铺：ourstm.taobao.com            *\r\n");     	   
  USART_OUT(USART1,"*     奋斗STM32论坛：www.ourstm.net  QQ：9191274  *\r\n");   
  USART_OUT(USART1,"*                                                 *\r\n");    	 	
  USART_OUT(USART1,"***************************************************\r\n");  
}



/****************************************************************************
* 名    称：void  OS_CPU_SysTickInit(void)
* 功    能：ucos 系统节拍时钟初始化  初始设置为10ms一次节拍
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void  OS_CPU_SysTickInit(void)
{
    RCC_ClocksTypeDef  rcc_clocks;
    INT32U         cnts;
    RCC_GetClocksFreq(&rcc_clocks);		                        //获得系统时钟的值	 
    cnts = (INT32U)rcc_clocks.HCLK_Frequency/OS_TICKS_PER_SEC;	//算出时钟节拍的值	
	SysTick_Config(cnts);										//设置时钟节拍	     
}

/****************************************************************************
* 名    称：void USART_OUTB(USART_TypeDef* USARTx, uint8_t *Data,uint16_t Len)
* 功    能：2进制数据串口输出函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/
void USART_OUTB(USART_TypeDef* USARTx, uint8_t *Data,uint16_t Len){ 
	uint16_t i;
	for(i=0; i<Len; i++){
		USART_SendData(USARTx, Data[i]);    
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
	}
}
/****************************************************************************
* 名    称：void USART_OUT(USART_TypeDef* USARTx, uint8_t *Data,...)
* 功    能：格式化串口输出函数
* 入口参数：USARTx:  指定串口
			Data：   发送数组
			...:     不定参数
* 出口参数：无
* 说    明：格式化串口输出函数
        	"\r"	回车符	   USART_OUT(USART1, "abcdefg\r")   
			"\n"	换行符	   USART_OUT(USART1, "abcdefg\r\n")
			"%s"	字符串	   USART_OUT(USART1, "字符串是：%s","abcdefg")
			"%d"	十进制	   USART_OUT(USART1, "a=%d",10)
* 调用方法：无 
****************************************************************************/
void USART_OUT(USART_TypeDef* USARTx, uint8_t *Data,...){ 
	const char *s;
    int d;
    char buf[16];
    va_list ap;
    va_start(ap, Data);

	while(*Data!=0){				                          //判断是否到达字符串结束符
		if(*Data==0x5c){									  //'\'
			switch (*++Data){
				case 'r':							          //回车符
					USART_SendData(USARTx, 0x0d);	   

					Data++;
					break;
				case 'n':							          //换行符
					USART_SendData(USARTx, 0x0a);	
					Data++;
					break;
				
				default:
					Data++;
				    break;
			}
			
			 
		}
		else if(*Data=='%'){									  //
			switch (*++Data){				
				case 's':										  //字符串
                	s = va_arg(ap, const char *);
                	for ( ; *s; s++) {
                    	USART_SendData(USARTx,*s);
						while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
                	}
					Data++;
                	break;
            	case 'd':										  //十进制
                	d = va_arg(ap, int);
                	itoa(d, buf, 10);
                	for (s = buf; *s; s++) {
                    	USART_SendData(USARTx,*s);
						while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
                	}
					Data++;
                	break;
				default:
					Data++;
				    break;
			}		 
		}
		else USART_SendData(USARTx, *Data++);
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
	}
}

/******************************************************
		整形数据转字符串函数
        char *itoa(int value, char *string, int radix)
		radix=10 标示是10进制	非十进制，转换结果为0;  

	    例：d=-379;
		执行	itoa(d, buf, 10); 后
		
		buf="-379"							   			  
**********************************************************/
char *itoa(int value, char *string, int radix)
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
/******************* (C) COPYRIGHT 2011 奋斗STM32 *****END OF FILE****/

