#define GLOBALS	  					   
#include "includes.h"
#include "demo.h"
#include "circ_buf.h"
#include "serial.h"
#include "shell.h"
extern void Delay(__IO uint32_t nCount);
void USART_OUT(USART_TypeDef* USARTx, uint8_t *Data,...);
extern void USART_OUTB(USART_TypeDef* USARTx, uint8_t *Data,uint16_t Len);
char *itoa(int value, char *string, int radix);
/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  OS_STK App_TaskStartStk[APP_TASK_START_STK_SIZE];
static  OS_STK Task_USART2Stk[APP_TASK_USART2_STK_SIZE];
static  OS_STK Task_txPumpStk1[APP_TASK_txPump1_STK_SIZE];
static  OS_STK Task_serRXStk1[APP_TASK_serRX1_STK_SIZE];
static  OS_STK Task_txPumpStk2[APP_TASK_txPump2_STK_SIZE];
static  OS_STK Task_serRXStk2[APP_TASK_serRX2_STK_SIZE];
static  OS_STK Task_txPumpStk3[APP_TASK_txPump3_STK_SIZE];
static  OS_STK Task_serRXStk3[APP_TASK_serRX3_STK_SIZE];

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void App_TaskCreate(void);	  
static  void App_TaskStart(void* p_arg);
static  void Task_USART2(void* p_arg);	 
static  void Task_txPump1(void* p_arg);	
static  void Task_serRX1(void* p_arg);
static  void Task_txPump2(void* p_arg);	
static  void Task_serRX2(void* p_arg);
static  void Task_txPump3(void* p_arg);	
static  void Task_serRX3(void* p_arg);

/****************************************************************************
* 名    称：int main(void)
* 功    能：主函数入口
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
int main(void)
{
   /* 禁止所有中断 */
   CPU_IntDis();
   
   /* ucosII 初始化 */
   OSInit();                                                  

   /* 硬件平台初始化 */
   BSP_Init();                               
   
   //建立主任务， 优先级最高  建立这个任务另外一个用途是为了以后使用统计任务
	 OSTaskCreate((void (*) (void *)) App_TaskStart,               		    //指向任务代码的指针
                          (void *) 0,												//任务开始执行时，传递给任务的参数的指针
               		     (OS_STK *) &App_TaskStartStk[APP_TASK_START_STK_SIZE - 1],	//分配给任务的堆栈的栈顶指针   从顶向下递减
                         (INT8U) APP_TASK_START_PRIO);								//分配给任务的优先级
             
   OSTimeSet(0);			 //ucosII的节拍计数器清0    节拍计数器是0-4294967295  
   OSStart();                //启动ucosII内核   
   return (0);
}

/****************************************************************************
* 名    称：static  void App_TaskStart(void* p_arg)
* 功    能：开始任务建立
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static  void App_TaskStart(void* p_arg)
{
  (void) p_arg;
   //初始化ucosII时钟节拍
   OS_CPU_SysTickInit();
                               
   //使能ucos 的统计任务
#if (OS_TASK_STAT_EN > 0)
   
   OSStatInit();                			//----统计任务初始化函数                                 
#endif

   App_TaskCreate();						//建立其他的任务

   while (1)
   {  
	  OSTimeDlyHMSM(0, 0, 0, 1000);
   }
}

/****************************************************************************
* 名    称：static  void App_TaskCreate(void)
* 功    能：建立其余任务的函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static  void App_TaskCreate(void)
{ 
	 INT8U   err;
	 mutex_pr1 = OSMutexCreate(MUTEX_PRIO,&err);
	 mutex_pr2 = OSMutexCreate(MUTEX_PRIO,&err);
	 mutex_pr3 = OSMutexCreate(MUTEX_PRIO,&err);
	 tid_txPump1 = OSSemCreate(1);
	 tid_txPump2 = OSSemCreate(1);
	 tid_txPump3 = OSSemCreate(1);
										
   OSTaskCreateExt(Task_txPump1,
				(void *)0,
					(OS_STK *)&Task_txPumpStk1[APP_TASK_txPump1_STK_SIZE-1],
					APP_TASK_txPump1_PRIO,
					APP_TASK_txPump1_PRIO,
					(OS_STK *)&Task_txPumpStk1[0],
                    APP_TASK_txPump1_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);

   OSTaskCreateExt(Task_serRX1,
				(void *)0,
					(OS_STK *)&Task_serRXStk1[APP_TASK_serRX1_STK_SIZE-1],
					APP_TASK_serRX1_PRIO,
					APP_TASK_serRX1_PRIO,
					(OS_STK *)&Task_serRXStk1[0],
                    APP_TASK_serRX1_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);			

   OSTaskCreateExt(Task_txPump2,
				(void *)0,
					(OS_STK *)&Task_txPumpStk2[APP_TASK_txPump2_STK_SIZE-1],
					APP_TASK_txPump2_PRIO,
					APP_TASK_txPump2_PRIO,
					(OS_STK *)&Task_txPumpStk2[0],
                    APP_TASK_txPump2_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);

   OSTaskCreateExt(Task_serRX2,
				(void *)0,
					(OS_STK *)&Task_serRXStk2[APP_TASK_serRX2_STK_SIZE-1],
					APP_TASK_serRX2_PRIO,
					APP_TASK_serRX2_PRIO,
					(OS_STK *)&Task_serRXStk2[0],
                    APP_TASK_serRX2_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);

//   OSTaskCreateExt(Task_txPump3,
//				(void *)0,
//					(OS_STK *)&Task_txPumpStk3[APP_TASK_txPump3_STK_SIZE-1],
//					APP_TASK_txPump3_PRIO,
//					APP_TASK_txPump3_PRIO,
//					(OS_STK *)&Task_txPumpStk3[0],
//                    APP_TASK_txPump3_STK_SIZE,
//                    (void *)0,
//                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);

//   OSTaskCreateExt(Task_serRX3,
//				(void *)0,
//					(OS_STK *)&Task_serRXStk3[APP_TASK_serRX3_STK_SIZE-1],
//					APP_TASK_serRX3_PRIO,
//					APP_TASK_serRX3_PRIO,
//					(OS_STK *)&Task_serRXStk3[0],
//                    APP_TASK_serRX3_STK_SIZE,
//                    (void *)0,
//                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);												
}

extern volatile int xmiting1;
extern volatile int xmiting2;
extern volatile int xmiting3;

// DMA buffer
extern uint8_t Usart1_Tx_DMA_Buffer[DMA_BUF_SIZE];
extern uint8_t Usart1_Rx_DMA_Buffer[DMA_BUF_SIZE];

extern uint8_t Usart2_Tx_DMA_Buffer[DMA_BUF_SIZE];
extern uint8_t Usart2_Rx_DMA_Buffer[DMA_BUF_SIZE];

extern uint8_t Usart3_Tx_DMA_Buffer[DMA_BUF_SIZE];
extern uint8_t Usart3_Rx_DMA_Buffer[DMA_BUF_SIZE];

extern struct circ_buf xmit_buf1;
extern struct circ_buf xmit_buf2;
extern struct circ_buf xmit_buf3;

void Task_txPump1(void* p_arg)
{
  uint8_t data;
  uint16_t TxCounter;
	INT8U err;
  while(1)
  {
		SER_printf1("Task_txPump1 is running \n");
    if (xmiting1 != 1) {
      TxCounter = 0;
      while(circ_get1(&xmit_buf1, &data) == 0) {
        Usart1_Tx_DMA_Buffer[TxCounter++] = data;
        // take up to DMA_BUF_SIZE bytes from circ buf to DMA buffer
        if (TxCounter == DMA_BUF_SIZE)
          break;
      }
      if (TxCounter > 0) {
        DMA_SetCurrDataCounter(DMA1_Channel_Tx, TxCounter);
        /* Enable DMA1 Channel_Tx */
        DMA_Cmd(DMA1_Channel_Tx, ENABLE);
        xmiting1 = 1;
      } else
				OSSemPend(tid_txPump1,200,&err);
    } else
			OSSemPend(tid_txPump1,200,&err);
  }
}

void Task_txPump2(void* p_arg)
{
  uint8_t data;
  uint16_t TxCounter;
	INT8U err;
  while(1)
  {
		SER_printf1("Task_txPump2 is running \n");
    if (xmiting2 != 1) {
      TxCounter = 0;
      while(circ_get2(&xmit_buf2, &data) == 0) {
        Usart2_Tx_DMA_Buffer[TxCounter++] = data;
        // take up to DMA_BUF_SIZE bytes from circ buf to DMA buffer
        if (TxCounter == DMA_BUF_SIZE)
          break;
      }
      if (TxCounter > 0) {
        DMA_SetCurrDataCounter(DMA1_Channel7, TxCounter);
        /* Enable DMA1 Channel_Tx */
        DMA_Cmd(DMA1_Channel7, ENABLE);
        xmiting2 = 1;
      } else
				OSSemPend(tid_txPump2,200,&err);
    } else
		OSSemPend(tid_txPump2,200,&err);
  }
}

void Task_txPump3(void* p_arg)
{
  uint8_t data;
  uint16_t TxCounter;
	INT8U err;
  while(1)
  {
    if (xmiting3 != 1) {
      TxCounter = 0;
      while(circ_get3(&xmit_buf3, &data) == 0) {
        Usart3_Tx_DMA_Buffer[TxCounter++] = data;
        // take up to DMA_BUF_SIZE bytes from circ buf to DMA buffer
        if (TxCounter == DMA_BUF_SIZE)
          break;
      }
      if (TxCounter > 0) {
        DMA_SetCurrDataCounter(DMA1_Channel2, TxCounter);
        /* Enable DMA1 Channel_Tx */
        DMA_Cmd(DMA1_Channel2, ENABLE);
        xmiting3 = 1;
      } else
				OSSemPend(tid_txPump3,200,&err);
    } else
		OSSemPend(tid_txPump3,200,&err);
  }
}

extern void parse_and_process(char *buf, pr p);
char ser_buf[1024];

void Task_serRX1(void* p_arg) {
  int dat;
  int col = 0;
	SER_printf1("COMMAND>");
  while (1) {
    dat = SER_getchar1();
    if (dat == '\r' || dat == '#' || dat == '\n') {
      ser_buf[col] = '\0';
      col = 0;
      if (strlen(ser_buf) > 0) parse_and_process(ser_buf, SER_printf1);
				SER_printf1("COMMAND> ");
		} else {
      if (col < 1023)
        ser_buf[col++] = dat;
    }
  }
}

void Task_serRX2(void* p_arg) {
  int dat;
  int col = 0;
  SER_printf2("COMMAND>");
  while (1) {
    dat = SER_getchar2();
    if (dat == '\r' || dat == '#' || dat == '\n') {
      ser_buf[col] = '\0';
      col = 0;
      if (strlen(ser_buf) > 0) parse_and_process(ser_buf, SER_printf2);
			SER_printf2("COMMAND> ");
		} else {
      if (col < 1023)
        ser_buf[col++] = dat;
    }
  }
}

void Task_serRX3(void* p_arg) {
  int dat;
  int col = 0;
  SER_printf3("COMMAND>");
  while (1) {
    dat = SER_getchar3();
    if (dat == '\r' || dat == '#' || dat == '\n') {
      ser_buf[col] = '\0';
      col = 0;
      if (strlen(ser_buf) > 0) parse_and_process(ser_buf, SER_printf3);
			SER_printf3("COMMAND> ");
		} else {
      if (col < 1023)
        ser_buf[col++] = dat;
    }
  }
}

/*
*********************************************************************************************************
*********************************************************************************************************
*                                          uC/OS-II APP HOOKS
*********************************************************************************************************
*********************************************************************************************************
*/

#if (OS_APP_HOOKS_EN > 0)
/*
*********************************************************************************************************
*                                      TASK CREATION HOOK (APPLICATION)
*
* Description : This function is called when a task is created.
*
* Argument : ptcb   is a pointer to the task control block of the task being created.
*
* Note     : (1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void App_TaskCreateHook(OS_TCB* ptcb)
{
}

/*
*********************************************************************************************************
*                                    TASK DELETION HOOK (APPLICATION)
*
* Description : This function is called when a task is deleted.
*
* Argument : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note     : (1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void App_TaskDelHook(OS_TCB* ptcb)
{
   (void) ptcb;
}

/*
*********************************************************************************************************
*                                      IDLE TASK HOOK (APPLICATION)
*
* Description : This function is called by OSTaskIdleHook(), which is called by the idle task.  This hook
*               has been added to allow you to do such things as STOP the CPU to conserve power.
*
* Argument : none.
*
* Note     : (1) Interrupts are enabled during this call.
*********************************************************************************************************
*/

#if OS_VERSION >= 251
void App_TaskIdleHook(void)
{
}
#endif

/*
*********************************************************************************************************
*                                        STATISTIC TASK HOOK (APPLICATION)
*
* Description : This function is called by OSTaskStatHook(), which is called every second by uC/OS-II's
*               statistics task.  This allows your application to add functionality to the statistics task.
*
* Argument : none.
*********************************************************************************************************
*/

void App_TaskStatHook(void)
{
}

/*
*********************************************************************************************************
*                                        TASK SWITCH HOOK (APPLICATION)
*
* Description : This function is called when a task switch is performed.  This allows you to perform other
*               operations during a context switch.
*
* Argument : none.
*
* Note     : 1 Interrupts are disabled during this call.
*
*            2  It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                   will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                  task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/

#if OS_TASK_SW_HOOK_EN > 0
void App_TaskSwHook(void)
{
}
#endif

/*
*********************************************************************************************************
*                                     OS_TCBInit() HOOK (APPLICATION)
*
* Description : This function is called by OSTCBInitHook(), which is called by OS_TCBInit() after setting
*               up most of the TCB.
*
* Argument : ptcb    is a pointer to the TCB of the task being created.
*
* Note     : (1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/

#if OS_VERSION >= 204
void App_TCBInitHook(OS_TCB* ptcb)
{
   (void) ptcb;
}
#endif

#endif
 /******************* (C) COPYRIGHT 2011 奋斗STM32 *****END OF FILE****/
