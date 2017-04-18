/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2006; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      APPLICATION CONFIGURATION
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   STM3210B-LK1 Evaluation Board
*
* Filename      : app_cfg.h
* Version       : V1.10
* Programmer(s) : BH3NVN
*********************************************************************************************************
*/

#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__

/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              TASKS NAMES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

#define  APP_TASK_START_PRIO                          5 

#define  APP_TASK_txPump1_PRIO					             10
#define  APP_TASK_serRX1_PRIO                        11

#define  APP_TASK_txPump2_PRIO					             12
#define  APP_TASK_serRX2_PRIO                        14

#define  APP_TASK_txPump3_PRIO					             10
#define  APP_TASK_serRX3_PRIO                        13

#define  MUTEX_PRIO                                  7



/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define  APP_TASK_START_STK_SIZE                         64
#define  APP_TASK_KBD_STK_SIZE                           64
#define  APP_TASK_USART2_STK_SIZE                        512
#define  APP_TASK_txPump1_STK_SIZE                        1024
#define  APP_TASK_serRX1_STK_SIZE                         1024
#define  APP_TASK_txPump2_STK_SIZE                        1024
#define  APP_TASK_serRX2_STK_SIZE                         1024
#define  APP_TASK_txPump3_STK_SIZE                        1024
#define  APP_TASK_serRX3_STK_SIZE                         1024


#define  uC_CFG_OPTIMIZE_ASM_EN                 DEF_ENABLED
#define  LIB_STR_CFG_FP_EN                      DEF_DISABLED

#endif
