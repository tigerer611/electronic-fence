/****************************************************************************
* Copyright (C), 2013 �ܶ�Ƕ��ʽ������ www.ourstm.net
*
* �������� �ܶ���STM32������V5,V3��V2,v2.1��MINI�ϵ���ͨ��           
* QQ: 9191274, ������sun68, Email: sun68@163.com 
* �Ա����̣�ourstm.taobao.com  
*
* �ļ���: app.c
* ���ݼ���:
*       �����̲���ϵͳ����ucos2.86a�汾�� ������5������
			������											 ���ȼ�
			APP_TASK_START_PRIO                               2	        ������
		 	TASK_USART1_PRIO                                  10		USART1���Ľ�������
			TASK_USART2_PRIO                                  11		USART2���Ľ�������
		 ��Ȼ��������ϵͳ����
		    OS_TaskIdle                  ��������-----------------���ȼ����
			OS_TaskStat                  ͳ������ʱ�������-------���ȼ��ε�
*
* �ļ���ʷ:
* �汾��  ����       ����    ˵��
* v0.1    2011-11-22 sun68  �������ļ�
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
* ��    �ƣ�int main(void)
* ��    �ܣ����������
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
int main(void)
{
   CPU_INT08U os_err; 
   /* ��ֹ�����ж� */
   CPU_IntDis();
   
   /* ucosII ��ʼ�� */
   OSInit();                                                  

   /* Ӳ��ƽ̨��ʼ�� */
   BSP_Init();                               
   
   //���������� ���ȼ����  ���������������һ����;��Ϊ���Ժ�ʹ��ͳ������
   os_err = OSTaskCreate((void (*) (void *)) App_TaskStart,               		    //ָ����������ָ��
                          (void *) 0,												//����ʼִ��ʱ�����ݸ�����Ĳ�����ָ��
               		     (OS_STK *) &App_TaskStartStk[APP_TASK_START_STK_SIZE - 1],	//���������Ķ�ջ��ջ��ָ��   �Ӷ����µݼ�
                         (INT8U) APP_TASK_START_PRIO);								//�������������ȼ�
             
   OSTimeSet(0);			 //ucosII�Ľ��ļ�������0    ���ļ�������0-4294967295  
   OSStart();                //����ucosII�ں�   
   return (0);
}

/****************************************************************************
* ��    �ƣ�static  void App_TaskStart(void* p_arg)
* ��    �ܣ���ʼ������
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
static  void App_TaskStart(void* p_arg)
{
 
  (void) p_arg;
   //��ʼ��ucosIIʱ�ӽ���
   OS_CPU_SysTickInit();
                               
   //ʹ��ucos ��ͳ������
#if (OS_TASK_STAT_EN > 0)
   
   OSStatInit();                			//----ͳ�������ʼ������                                 
#endif

   App_TaskCreate();						//��������������

   while (1)
   {  
   	  
	  OSTimeDlyHMSM(0, 0, 0, 1000);
   }
}

/****************************************************************************
* ��    �ƣ�static  void App_TaskCreate(void)
* ��    �ܣ�������������ĺ���
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
static  void App_TaskCreate(void)
{ 
   USART1_MBOX=OSMboxCreate((void *) 0);		     							   //����USART1�����������Ϣ����
   USART2_MBOX=OSMboxCreate((void *) 0);		     							   //����USART2�����������Ϣ����	

					   
   /*   ����USART1 ���Ľ������� */
   OSTaskCreateExt(Task_USART1,
   					(void *)0,
					(OS_STK *)&Task_USART1Stk[APP_TASK_USART1_STK_SIZE-1],
					APP_TASK_USART1_PRIO,
					APP_TASK_USART1_PRIO,
					(OS_STK *)&Task_USART1Stk[0],
                    APP_TASK_USART1_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
	/*   ����USART2 ���Ľ������� */
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
* ��    �ƣ�static  void Task_Usart1(void *p_arg)
* ��    �ܣ�USART1��������
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
static  void Task_USART1(void *p_arg){    
   INT8U err;
   unsigned char * msg; 
   (void)p_arg;	  
   while(1){   
	 msg=(unsigned char *)OSMboxPend(USART1_MBOX,0,&err); //�ȴ�USART1�ɹ�����һ֡��������Ϣ
	 memcpy(rx1_buf, msg, RxCount1+2);	
	 Tx_Size=strlen(rx1_buf);							  //��ò��Ա��ĵĳ���  							
	 /* �򴮿�2����һ֡���� */														   									
	 USART_OUTB(USART2,rx1_buf,Tx_Size);	 
	 RxCount1=0;
  }
}

/****************************************************************************
* ��    �ƣ�static  void Task_Usart2(void *p_arg)
* ��    �ܣ�USART2��������
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
static  void Task_USART2(void *p_arg){    
   INT8U err;
   unsigned char * msg; 
   (void)p_arg;	  
   while(1){   
	 msg=(unsigned char *)OSMboxPend(USART2_MBOX,0,&err); 		  //�ȴ�USART2�ɹ�����һ֡��������Ϣ
	 /* �����յ��ı��ı�������ʾ��������������ʾ */			 
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
 /******************* (C) COPYRIGHT 2011 �ܶ�STM32 *****END OF FILE****/
