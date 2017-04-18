#ifndef __SERIAL_H
#define __SERIAL_H
#include <stdint.h>
// USART1
// PA9  Tx
// PA10 Rx
#define DMA_BUF_SIZE 256
#define RX_BUF_SIZE 1024  // must be 2^n
#define TX_BUF_SIZE 1024  // must be 2^n

#define  USART_Tx                 USART1
#define  USART_Tx_GPIO            GPIOA
#define  USART_Tx_CLK             RCC_APB2Periph_USART1
#define  USART_Tx_GPIO_CLK        RCC_APB2Periph_GPIOA
#define  GPIO_TxPin               GPIO_Pin_9

#define  USART_Rx                 USART1
#define  USART_Rx_GPIO            GPIOA
#define  USART_Rx_CLK             RCC_APB2Periph_USART1
#define  USART_Rx_GPIO_CLK        RCC_APB2Periph_GPIOA
#define  GPIO_RxPin               GPIO_Pin_10

#define  DMA1_Channel_Tx          DMA1_Channel4
#define  DMA1_Channel_Tx_IRQn     DMA1_Channel4_IRQn

#define  DMA1_Channel_Rx          DMA1_Channel5

void SER_Config(void);
uint8_t SER_getchar1(void);
uint8_t SER_getchar2(void);
uint8_t SER_getchar3(void);
int SER_printf1(const char *fmt, ...);
int SER_printf2(const char *fmt, ...);
int SER_printf3(const char *fmt, ...);

#endif
