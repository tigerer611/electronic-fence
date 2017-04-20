#include "stm32f10x_rcc.h"              // Keil::Device:StdPeriph Drivers:RCC
#include <stdarg.h>
#include <stdio.h>
#include "serial.h"
#include "circ_buf.h"
#include "demo.h"
// DMA buffer
uint8_t Usart1_Tx_DMA_Buffer[DMA_BUF_SIZE];
uint8_t Usart1_Rx_DMA_Buffer[DMA_BUF_SIZE];

uint8_t Usart2_Tx_DMA_Buffer[DMA_BUF_SIZE];
uint8_t Usart2_Rx_DMA_Buffer[DMA_BUF_SIZE];

uint8_t Usart3_Tx_DMA_Buffer[DMA_BUF_SIZE];
uint8_t Usart3_Rx_DMA_Buffer[DMA_BUF_SIZE];

// circ buffer
struct circ_buf xmit_buf1;
struct circ_buf recv_buf1;
uint8_t TxBuffer1[TX_BUF_SIZE];
uint8_t RxBuffer1[RX_BUF_SIZE];

struct circ_buf xmit_buf2;
struct circ_buf recv_buf2;
uint8_t TxBuffer2[TX_BUF_SIZE];
uint8_t RxBuffer2[RX_BUF_SIZE];

struct circ_buf xmit_buf3;
struct circ_buf recv_buf3;
uint8_t TxBuffer3[TX_BUF_SIZE];
uint8_t RxBuffer3[RX_BUF_SIZE];

// auto wrap around since (2^8 = 256) == DMA_BUF_SIZE
uint8_t RxCounter1 = 0;
uint8_t RxCounter2 = 0;
uint8_t RxCounter3 = 0;
void USART1_IRQHandler(void)
{
  do {
    circ_put1(&recv_buf1, Usart1_Rx_DMA_Buffer[RxCounter1++]);
    // There is magic here. The Rx DMA is circular mode, so when the Rx_DMA_Buffer
    // is full, "DMA_BUF_SIZE - DMA_GetCurrDataCounter" will return to 0, i.e. the
    // DMA index wrap around. At this moment, the while condition will be false
    // and the copy loop will be break.
    //
    // But it works actually! This is why I call it magic.
    // Please note this is copied from st AN3109.
  } while (RxCounter1 < (DMA_BUF_SIZE - DMA_GetCurrDataCounter(DMA1_Channel_Rx)));
}

void USART2_IRQHandler(void)
{
  do {
    circ_put2(&recv_buf2, Usart2_Rx_DMA_Buffer[RxCounter2++]);
    // There is magic here. The Rx DMA is circular mode, so when the Rx_DMA_Buffer
    // is full, "DMA_BUF_SIZE - DMA_GetCurrDataCounter" will return to 0, i.e. the
    // DMA index wrap around. At this moment, the while condition will be false
    // and the copy loop will be break.
    //
    // But it works actually! This is why I call it magic.
    // Please note this is copied from st AN3109.
  } while (RxCounter2 < (DMA_BUF_SIZE - DMA_GetCurrDataCounter(DMA1_Channel6)));
}

void USART3_IRQHandler(void)
{
  do {
    circ_put1(&recv_buf3, Usart1_Rx_DMA_Buffer[RxCounter3++]);
    // There is magic here. The Rx DMA is circular mode, so when the Rx_DMA_Buffer
    // is full, "DMA_BUF_SIZE - DMA_GetCurrDataCounter" will return to 0, i.e. the
    // DMA index wrap around. At this moment, the while condition will be false
    // and the copy loop will be break.
    //
    // But it works actually! This is why I call it magic.
    // Please note this is copied from st AN3109.
  } while (RxCounter3 < (DMA_BUF_SIZE - DMA_GetCurrDataCounter(DMA1_Channel3)));
}

static void SER_RCC_Config()
{
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 | RCC_APB1Periph_USART3, ENABLE);
}

static void SER_GPIO_Config()
{
  GPIO_InitTypeDef GPIO_InitStructure;

	//USART1
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//TX
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;//RX
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//USART2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//TX
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//RX
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//USART3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//TX
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//RX
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void SER_DMA_Config()
{
  DMA_InitTypeDef DMA_InitStructure;
	//USART1 DMA1_CHANNEL4_TX
  /* DMA1 Channel (triggered by USART_Tx event) Config */
  DMA_DeInit(DMA1_Channel4);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Usart1_Tx_DMA_Buffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = DMA_BUF_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel4, &DMA_InitStructure);
	//USART1 DMA1_CHANNEL5_RX
  /* DMA1 Channel (triggered by USART_Rx event) Config */
  DMA_DeInit(DMA1_Channel5);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Usart1_Rx_DMA_Buffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = DMA_BUF_SIZE;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_Init(DMA1_Channel5, &DMA_InitStructure);

  /* Enable DMA1_Channel Transfer Complete interrupt */
  DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
	
	//USART2 DMA1_CHANNEL7_TX
  /* DMA1 Channel (triggered by USART_Tx event) Config */
  DMA_DeInit(DMA1_Channel7);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Usart2_Tx_DMA_Buffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = DMA_BUF_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel7, &DMA_InitStructure);
	//USART2 DMA1_CHANNEL6_RX
  /* DMA1 Channel (triggered by USART_Rx event) Config */
  DMA_DeInit(DMA1_Channel6);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Usart2_Rx_DMA_Buffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = DMA_BUF_SIZE;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_Init(DMA1_Channel6, &DMA_InitStructure);

  /* Enable DMA1_Channel Transfer Complete interrupt */
  DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);

	//USART3 DMA1_CHANNEL2_TX
  /* DMA1 Channel (triggered by USART_Tx event) Config */
  DMA_DeInit(DMA1_Channel2);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART3->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Usart3_Tx_DMA_Buffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = DMA_BUF_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel2, &DMA_InitStructure);
	//USART3 DMA1_CHANNEL3_RX
  /* DMA1 Channel (triggered by USART_Rx event) Config */
  DMA_DeInit(DMA1_Channel3);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART3->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Usart3_Rx_DMA_Buffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = DMA_BUF_SIZE;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_Init(DMA1_Channel3, &DMA_InitStructure);

  /* Enable DMA1_Channel Transfer Complete interrupt */
  DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
}

static void SER_UART_Config()
{
  USART_InitTypeDef USART_InitStructure;
  // Rx and Tx are same port, so only config Rx
  USART_DeInit(USART1);
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);

	// Rx and Tx are same port, so only config Rx
  USART_DeInit(USART2);
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART2, &USART_InitStructure);

	// Rx and Tx are same port, so only config Rx
  USART_DeInit(USART3);
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART3, &USART_InitStructure);

  USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
  /* Enable USART_Rx Receive interrupt */
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

  USART_DMACmd(USART2, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
  /* Enable USART_Rx Receive interrupt */
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	USART_DMACmd(USART3, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
  /* Enable USART_Rx Receive interrupt */
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

  /* Enable DMA1 Channel5 */
  DMA_Cmd(DMA1_Channel5, ENABLE);

  /* Enable DMA1 Channel6 */
  DMA_Cmd(DMA1_Channel6, ENABLE);

  /* Enable DMA1 Channel3 */
  DMA_Cmd(DMA1_Channel3, ENABLE);

  /* Enable the USART1 */
  USART_Cmd(USART1, ENABLE);

  /* Enable the USART2 */
  USART_Cmd(USART2, ENABLE);

  /* Enable the USART3 */
  USART_Cmd(USART3, ENABLE);
}


// flag to indicate whether xmit DMA is ongoing
volatile int xmiting1 = 0;

void DMAChannel4_IRQHandler(void)
{
  /* Disable DMA1_Channel4 transfer*/
  DMA_Cmd(DMA1_Channel4, DISABLE);
  /*  Clear DMA1_Channel4 Transfer Complete Flag*/
  DMA_ClearFlag(DMA1_FLAG_TC4);
  xmiting1 = 0;
	OSSemPost(tid_txPump1); 
}

// flag to indicate whether xmit DMA is ongoing
volatile int xmiting2 = 0;

void DMAChannel7_IRQHandler(void)
{
  /* Disable DMA1_Channel7 transfer*/
  DMA_Cmd(DMA1_Channel7, DISABLE);
  /*  Clear DMA1_Channel7 Transfer Complete Flag*/
  DMA_ClearFlag(DMA1_FLAG_TC7);
  xmiting2 = 0;
	OSSemPost(tid_txPump2); 
}

// flag to indicate whether xmit DMA is ongoing
volatile int xmiting3 = 0;

void DMAChannel2_IRQHandler(void)
{
  /* Disable DMA1_Channel2 transfer*/
  DMA_Cmd(DMA1_Channel2, DISABLE);
  /*  Clear DMA1_Channel2 Transfer Complete Flag*/
  DMA_ClearFlag(DMA1_FLAG_TC2);
  xmiting3 = 0;
	OSSemPost(tid_txPump3); 
}

void SER_Config()
{
  SER_RCC_Config();
  SER_GPIO_Config();
  SER_DMA_Config();
  SER_UART_Config();

  circ_init1(&recv_buf1, RxBuffer1, RX_BUF_SIZE);
  circ_init1(&xmit_buf1, TxBuffer1, TX_BUF_SIZE);

  //usart2 circle buff
  circ_init2(&recv_buf2, RxBuffer2, RX_BUF_SIZE);
  circ_init2(&xmit_buf2, TxBuffer2, TX_BUF_SIZE);

  //usart3 circle buff
	circ_init3(&recv_buf3, RxBuffer3, RX_BUF_SIZE);
	circ_init3(&xmit_buf3, TxBuffer3, TX_BUF_SIZE);
}

uint8_t SER_getchar1(void)
{
  uint8_t data;
  while (circ_get1(&recv_buf1, &data) != 0);
  return data;
}

uint8_t SER_getchar2(void)
{
  uint8_t data;
//  while (circ_get2(&recv_buf2, &data) != 0);
	if (circ_get2(&recv_buf2, &data) != 0)
		return data;
	else
		return -1;
}

uint8_t SER_getchar3(void)
{
  uint8_t data;
//  while (circ_get3(&recv_buf3, &data) != 0);
	if (circ_get3(&recv_buf3, &data) != 0)
		return data;
	else
		return -1;
  return data;
}

#ifndef SER_CR_LF
#define SER_CR_LF    0       /* SER: add CR for LF */
#endif
static void SER_putchar1(uint8_t data)
{
#if (SER_CR_LF != 0)
  if (data == '\n') {
    while (CIRC_SPACE1(xmit_buf1.head, xmit_buf1.tail, xmit_buf1.size) == 0)
      __NOP();
    circ_put1(&xmit_buf1, '\r');
  }
#endif
  while (CIRC_SPACE1(xmit_buf1.head, xmit_buf1.tail, xmit_buf1.size) == 0)
    __NOP();
  circ_put1(&xmit_buf1, data);
}

static void SER_putchar2(uint8_t data)
{
#if (SER_CR_LF != 0)
  if (data == '\n') {
    while (CIRC_SPACE2(xmit_buf2.head, xmit_buf2.tail, xmit_buf2.size) == 0)
      __NOP();
    circ_put2(&xmit_buf2, '\r');
  }
#endif
  while (CIRC_SPACE2(xmit_buf2.head, xmit_buf2.tail, xmit_buf2.size) == 0)
    __NOP();
  circ_put2(&xmit_buf2, data);
}

static void SER_putchar3(uint8_t data)
{
#if (SER_CR_LF != 0)
  if (data == '\n') {
    while (CIRC_SPACE3(xmit_buf3.head, xmit_buf3.tail, xmit_buf3.size) == 0)
      __NOP();
    circ_put3(&xmit_buf3, '\r');
  }
#endif
  while (CIRC_SPACE3(xmit_buf3.head, xmit_buf3.tail, xmit_buf3.size) == 0)
    __NOP();
  circ_put3(&xmit_buf3, data);
}

/*
 * important notes: there is mutex here, so do not call this function from an ISR.
 */
int SER_printf1(const char *fmt, ...)
{
  char buf[128];
  int len;
  int i;
	INT8U   err;
  va_list args;
	OSMutexPend(mutex_pr1, INFINITE, &err);
  va_start(args, fmt);
  len = vsnprintf(buf, 128, fmt, args);
  va_end(args);
  for (i = 0; i < len; i++)
    SER_putchar1(buf[i]);
	OSMutexPost(mutex_pr1); 
  return len;
}

int SER_printf2(const char *fmt, ...)
{
  char buf[128];
  int len;
  int i;
	INT8U   err;
  va_list args;
	OSMutexPend(mutex_pr2, INFINITE, &err);
  va_start(args, fmt);
  len = vsnprintf(buf, 128, fmt, args);
  va_end(args);
  for (i = 0; i < len; i++)
    SER_putchar2(buf[i]);
	OSMutexPost(mutex_pr2); 
  return len;
}

int SER_printf3(const char *fmt, ...)
{
  char buf[128];
  int len;
  int i;
	INT8U   err;
  va_list args;
	OSMutexPend(mutex_pr3, INFINITE, &err);
  va_start(args, fmt);
  len = vsnprintf(buf, 128, fmt, args);
  va_end(args);
  for (i = 0; i < len; i++)
    SER_putchar3(buf[i]);
	OSMutexPost(mutex_pr3); 
  return len;
}
