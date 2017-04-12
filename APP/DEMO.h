#include "stm32f10x.h"
#include "..\..\APP\includes.h"

#ifdef GLOBALS 
#define EXT
#else
#define EXT extern 
#endif

#define LED1_ON()	GPIO_SetBits(GPIOB,  GPIO_Pin_5);
#define LED2_ON()	GPIO_SetBits(GPIOD,  GPIO_Pin_6);
#define LED3_ON()	GPIO_SetBits(GPIOD,  GPIO_Pin_3);

#define LED1_OFF()	GPIO_ResetBits(GPIOB,  GPIO_Pin_5);
#define LED2_OFF()	GPIO_ResetBits(GPIOD,  GPIO_Pin_6);
#define LED3_OFF()	GPIO_ResetBits(GPIOD,  GPIO_Pin_3);

EXT volatile unsigned short Tx_Size,Rst_Buf, Rst1_Buf;
EXT unsigned char rx_buf[1500],rx_buf_t[1500],tx_buf[1500];                 
EXT unsigned char rx1_buf[1500],rx1_buf_t[1500];                    
EXT unsigned short RxCount,RxCount1;                   
EXT OS_EVENT* USART1_MBOX;		                      //USART1接收邮箱;
EXT OS_EVENT* USART2_MBOX;		                      //USART2接收邮箱;
