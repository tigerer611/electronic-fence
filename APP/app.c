/****************************************************************************
* Copyright (C), 2013 奋斗嵌入式工作室 www.ourstm.net
*
* 本例程在 奋斗版STM32开发板V5,V3，V2,v2.1，MINI上调试通过           
* QQ: 9191274, 旺旺：sun68, Email: sun68@163.com 
* 淘宝店铺：ourstm.taobao.com  
*
* 文件名: app.c
* 内容简述:
*       本例程操作系统采用ucos2.86a版本， 建立了5个任务
			任务名											 优先级
			APP_TASK_START_PRIO                               2	        主任务
		 	TASK_USART1_PRIO                                  10		USART1报文接收任务
			TASK_USART2_PRIO                                  11		USART2报文接收任务
		 当然还包含了系统任务：
		    OS_TaskIdle                  空闲任务-----------------优先级最低
			OS_TaskStat                  统计运行时间的任务-------优先级次低
*
* 文件历史:
* 版本号  日期       作者    说明
* v0.1    2011-11-22 sun68  创建该文件
*
*/

#define GLOBALS	  					   
#include "includes.h"
#include "demo.h"
	
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
static  OS_STK Task_USART1Stk[APP_TASK_USART1_STK_SIZE];

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void App_TaskCreate(void);	  
static  void App_TaskStart(void* p_arg);
static  void Task_USART2(void* p_arg);	 
static  void Task_USART1(void* p_arg);	  

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
   CPU_INT08U os_err; 
   /* 禁止所有中断 */
   CPU_IntDis();
   
   /* ucosII 初始化 */
   OSInit();                                                  

   /* 硬件平台初始化 */
   BSP_Init();                               
   
   //建立主任务， 优先级最高  建立这个任务另外一个用途是为了以后使用统计任务
   os_err = OSTaskCreate((void (*) (void *)) App_TaskStart,               		    //指向任务代码的指针
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
   USART1_MBOX=OSMboxCreate((void *) 0);		     							   //建立USART1接收任务的消息邮箱
   USART2_MBOX=OSMboxCreate((void *) 0);		     							   //建立USART2接收任务的消息邮箱	

					   
   /*   建立USART1 报文接收任务 */
   OSTaskCreateExt(Task_USART1,
   					(void *)0,
					(OS_STK *)&Task_USART1Stk[APP_TASK_USART1_STK_SIZE-1],
					APP_TASK_USART1_PRIO,
					APP_TASK_USART1_PRIO,
					(OS_STK *)&Task_USART1Stk[0],
                    APP_TASK_USART1_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
	/*   建立USART2 报文接收任务 */
   OSTaskCreateExt(Task_USART2,
   					(void *)0,
					(OS_STK *)&Task_USART2Stk[APP_TASK_USART2_STK_SIZE-1],
					APP_TASK_USART2_PRIO,
					APP_TASK_USART2_PRIO,
					(OS_STK *)&Task_USART2Stk[0],
                    APP_TASK_USART2_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
}

/****************************************************************************
* 名    称：static  void Task_Usart1(void *p_arg)
* 功    能：USART1接收任务
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static  void Task_USART1(void *p_arg){    
   INT8U err;
   unsigned char * msg; 
   (void)p_arg;	  
   while(1){   
	 msg=(unsigned char *)OSMboxPend(USART1_MBOX,0,&err); //等待USART1成功接收一帧的邮箱信息
	 memcpy(rx1_buf, msg, RxCount1+2);	
	 Tx_Size=strlen(rx1_buf);							  //获得测试报文的长度  							
	 /* 向串口2发送一帧报文 */														   									
	 USART_OUTB(USART2,rx1_buf,Tx_Size);	 
	 RxCount1=0;
  }
}

/****************************************************************************
* 名    称：static  void Task_Usart2(void *p_arg)
* 功    能：USART2接收任务
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static  void Task_USART2(void *p_arg){    
   INT8U err;
   unsigned char * msg; 
   (void)p_arg;	  
   while(1){   
	 msg=(unsigned char *)OSMboxPend(USART2_MBOX,0,&err); 		  //等待USART2成功接收一帧的邮箱信息
	 /* 将接收到的报文保存在显示缓存区里用于显示 */			 
	 memcpy(rx_buf, msg, RxCount+2);
	 RxCount=0;					 	 
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
