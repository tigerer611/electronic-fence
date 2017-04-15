#include "stm32f10x_rcc.h"              // Keil::Device:StdPeriph Drivers:RCC
#include <stdarg.h>
#include <stdio.h>
#include "serial.h"
#include "circ_buf.h"
#include "demo.h"
// DMA buffer
uint8_t Tx_DMA_Buffer[DMA_BUF_SIZE];
uint8_t Rx_DMA_Buffer[DMA_BUF_SIZE];

// circ buffer
struct circ_buf xmit_buf;
struct circ_buf recv_buf;
uint8_t TxBuffer[TX_BUF_SIZE];
uint8_t RxBuffer[RX_BUF_SIZE];

// auto wrap around since (2^8 = 256) == DMA_BUF_SIZE
uint8_t RxCounter = 0;
void USART1_IRQHandler(void)
{
  do {
    circ_put(&recv_buf, Rx_DMA_Buffer[RxCounter++]);
    // There is magic here. The Rx DMA is circular mode, so when the Rx_DMA_Buffer
    // is full, "DMA_BUF_SIZE - DMA_GetCurrDataCounter" will return to 0, i.e. the
    // DMA index wrap around. At this moment, the while condition will be false
    // and the copy loop will be break.
    //
    // But it works actually! This is why I call it magic.
    // Please note this is copied from st AN3109.
  } while (RxCounter < (DMA_BUF_SIZE - DMA_GetCurrDataCounter(DMA1_Channel_Rx)));
}

static void SER_RCC_Config()
{
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  RCC_APB2PeriphClockCmd(USART_Tx_GPIO_CLK | USART_Rx_GPIO_CLK, ENABLE);
  RCC_APB2PeriphClockCmd(USART_Tx_CLK | USART_Rx_CLK, ENABLE);
}

static void SER_GPIO_Config()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = GPIO_RxPin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USART_Rx_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_TxPin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(USART_Tx_GPIO, &GPIO_InitStructure);
}

static void SER_DMA_Config()
{
  DMA_InitTypeDef DMA_InitStructure;
  /* DMA1 Channel (triggered by USART_Tx event) Config */
  DMA_DeInit(DMA1_Channel_Tx);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART_Tx->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Tx_DMA_Buffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = DMA_BUF_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel_Tx, &DMA_InitStructure);

  /* DMA1 Channel (triggered by USART_Rx event) Config */
  DMA_DeInit(DMA1_Channel_Rx);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART_Rx->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Rx_DMA_Buffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = DMA_BUF_SIZE;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_Init(DMA1_Channel_Rx, &DMA_InitStructure);

  /* Enable DMA1_Channel Transfer Complete interrupt */
  DMA_ITConfig(DMA1_Channel_Tx, DMA_IT_TC, ENABLE);
}

static void SER_UART_Config()
{
  USART_InitTypeDef USART_InitStructure;
  // Rx and Tx are same port, so only config Rx
  USART_DeInit(USART_Rx);
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART_Rx, &USART_InitStructure);

  USART_DMACmd(USART_Rx, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
  /* Enable USART_Rx Receive interrupt */
  USART_ITConfig(USART_Rx, USART_IT_RXNE, ENABLE);

  /* Enable DMA1 Channel_Rx */
  DMA_Cmd(DMA1_Channel_Rx, ENABLE);

  /* Enable the USART_Rx */
  USART_Cmd(USART_Rx, ENABLE);
}

// flag to indicate whether xmit DMA is ongoing
volatile int xmiting = 0;

void DMAChannel4_IRQHandler(void)
{
  /* Disable DMA1_Channel4 transfer*/
  DMA_Cmd(DMA1_Channel4, DISABLE);
  /*  Clear DMA1_Channel4 Transfer Complete Flag*/
  DMA_ClearFlag(DMA1_FLAG_TC4);
  xmiting = 0;
	OSSemPost(tid_txPump); 
}

void SER_Config()
{
  SER_RCC_Config();
  SER_GPIO_Config();
  SER_DMA_Config();
  SER_UART_Config();

  circ_init(&recv_buf, RxBuffer, RX_BUF_SIZE);
  circ_init(&xmit_buf, TxBuffer, TX_BUF_SIZE);
}

uint8_t SER_getchar(void)
{
  uint8_t data;
  while (circ_get(&recv_buf, &data) != 0);
  return data;
}

#ifndef SER_CR_LF
#define SER_CR_LF    0       /* SER: add CR for LF */
#endif
static void SER_putchar(uint8_t data)
{
#if (SER_CR_LF != 0)
  if (data == '\n') {
    while (CIRC_SPACE(xmit_buf.head, xmit_buf.tail, xmit_buf.size) == 0)
      __NOP();
    circ_put(&xmit_buf, '\r');
  }
#endif
  while (CIRC_SPACE(xmit_buf.head, xmit_buf.tail, xmit_buf.size) == 0)
    __NOP();
  circ_put(&xmit_buf, data);
}

/*
 * important notes: there is mutex here, so do not call this function from an ISR.
 */
int SER_printf(const char *fmt, ...)
{
  char buf[128];
  int len;
  int i;
	INT8U   err;
  va_list args;
	OSMutexPend(mutex_pr, INFINITE, &err);
  va_start(args, fmt);
  len = vsnprintf(buf, 128, fmt, args);
  va_end(args);
  for (i = 0; i < len; i++)
    SER_putchar(buf[i]);
	OSMutexPost(mutex_pr); 
  return len;
}
