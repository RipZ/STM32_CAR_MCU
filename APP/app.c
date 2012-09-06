#include "includes.h"

static  OS_STK App_TaskStartStk[APP_TASK_START_STK_SIZE];
static  OS_STK AppTaskUserIFStk[APP_TASK_USER_IF_STK_SIZE];
static  OS_STK AppTaskControllerStk[APP_TASK_CONTROLLER_STK_SIZE];

static  void App_TaskCreate(void);

static  void App_TaskStart(void* p_arg);
static  void  AppTaskUserIF (void *p_arg);

static  void AppTaskController(void* p_arg);
//static  void App_TaskLCD(void* p_arg);
//
//static  void App_TaskJoystick(void* p_arg);
#define LED_LED1_ON()   GPIO_SetBits(GPIOC, GPIO_Pin_6 );  	   //LED1 
#define LED_LED1_OFF()  GPIO_ResetBits(GPIOC, GPIO_Pin_6 ); 	 //LED1
#define LED_LED2_ON()   GPIO_SetBits(GPIOC, GPIO_Pin_7 );  	   //LED2 
#define LED_LED2_OFF()  GPIO_ResetBits(GPIOC, GPIO_Pin_7 ); 	 //LED2
#define LED_LED3_ON()   GPIO_SetBits(GPIOD, GPIO_Pin_13 );  	 //LED3 
#define LED_LED3_OFF()  GPIO_ResetBits(GPIOD, GPIO_Pin_13 ); 	 //LED3
#define LED_LED4_ON()   GPIO_SetBits(GPIOD, GPIO_Pin_6 );  	   //LED4 
#define LED_LED4_OFF()  GPIO_ResetBits(GPIOD, GPIO_Pin_6 ); 	 //LED4

unsigned char Controller_key;
bool Controller_key_event;
bool mute, vol_up, vol_down, prev, next, play_mode, fm_search, fm_memory;

unsigned char GPS_NMEA[256];
unsigned char GPS_NMEA_offset;
bool GPS_NMEA_complete;

unsigned char ECU_data, ECU_param;
bool ECU_req_param, ECU_complete;

extern unsigned char parameter[8];
extern unsigned char parameter_status[8];
extern unsigned char parameter_value[8];
bool request_next_param;

/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Argument : none.
*
* Return   : none.
*********************************************************************************************************
*/

int main(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
   CPU_INT08U os_err;
   //BSP_Init();      
   //BSP_IntDisAll();                                            /* Disable all ints until we are ready to accept them.  */
   CPU_IntDis();
   
   OSInit();                                                   /* Initialize "uC/OS-II, The Real-Time Kernel".         */

   BSP_Init();                                                 /* Initialize BSP functions.  */

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIOC->BSRR = 1<<0;

   os_err = OSTaskCreate((void (*) (void *)) App_TaskStart,
               /* Create the start task.                               */
                          (void *) 0,
               (OS_STK *) &App_TaskStartStk[APP_TASK_START_STK_SIZE - 1],
               (INT8U) APP_TASK_START_PRIO);
   
#if (OS_TASK_NAME_SIZE >= 11)
   OSTaskNameSet(APP_TASK_START_PRIO, (CPU_INT08U *) "Start Task", &os_err);
#endif

//   InfoSem = OSSemCreate(0); 
//   Disp_Box = OSMboxCreate((void *) 0);               
   OSTimeSet(0);
   OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II).  */

   return (0);
}

static  void App_TaskStart(void* p_arg)
{
   (void) p_arg;
   OS_CPU_SysTickInit();                                       /* Initialize the SysTick.       */
#if (OS_TASK_STAT_EN > 0)
   OSStatInit();                                               /* Determine CPU capacity.                              */
#endif
   App_TaskCreate();
   while (1)
   {
//      LED_LED1_ON();
//      OSTimeDlyHMSM(0, 0, 0, 100);
      
//      LED_LED1_OFF();
      OSTimeDlyHMSM(0, 0, 0, 1);
   }
}

static  void App_TaskCreate(void)
{
   CPU_INT08U os_err;
   
   OSTaskCreateExt(AppTaskUserIF,(void *)0,(OS_STK *)&AppTaskUserIFStk[APP_TASK_USER_IF_STK_SIZE-1],APP_TASK_USER_IF_PRIO,APP_TASK_USER_IF_PRIO,(OS_STK *)&AppTaskUserIFStk[0],
                    APP_TASK_USER_IF_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
                    
//   OSTaskCreateExt(AppTaskController,(void *)0,(OS_STK *)&AppTaskControllerStk[APP_TASK_CONTROLLER_STK_SIZE-1],APP_TASK_CONTROLLER_PRIO,APP_TASK_CONTROLLER_PRIO,(OS_STK *)&AppTaskControllerStk[0],
//                    APP_TASK_CONTROLLER_STK_SIZE,
//                    (void *)0,
//                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);


//   os_err = OSTaskCreate((void (*) (void *)) App_TaskLCD, (void *) 0,
//               (OS_STK *) &App_TaskLCDStk[APP_TASK_LCD_STK_SIZE - 1],
//               (INT8U) APP_TASK_LCD_PRIO);
//   printf("Creat App_TaskLCD£¡\r\n");
//#if (OS_TASK_NAME_SIZE >= 9)
//   OSTaskNameSet(APP_TASK_LCD_PRIO, "LCD", &os_err);
//#endif

//   os_err = OSTaskCreate((void (*) (void *)) App_TaskKbd, (void *) 0,
//               (OS_STK *) &App_TaskKbdStk[APP_TASK_KBD_STK_SIZE - 1],
//               (INT8U) APP_TASK_KBD_PRIO);
//   printf("Creat App_TaskKbd£¡\r\n");
}

extern void LCD_L0_FillRect(int x0, int y0, int x1, int y1);

static  void  AppTaskUserIF (void *p_arg)
{
 (void)p_arg;
  GUI_Init();
  while(1) 
  {
		dashboard();
  }
}

void USART1_IRQHandler(void) // Interrupt on char recieving from USART1 (controller)
{
	LED_LED1_ON();
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != (u16)RESET)
        {
					USART_ClearITPendingBit(USART1, USART_IT_RXNE);
          Controller_key = USART_ReceiveData(USART1);
					switch (Controller_key) {
							case 'm':
								mute = !mute;
								break;
							case 0x44:
								prev = 1;
								FM_prev_channel();
								break;
							case 0x43:
								next = 1;
								FM_next_channel();
								break;
							case 0x41:
								vol_up = 1;
								break;
							case 0x42:
								vol_down = 1;
								break;
							case 'r':
								fm_memory = 1;
								break;
							case 's':
								FM_scan();
								break;
							case ' ':
								play_mode = !play_mode;
					default:
							break;
					}

					Controller_key_event = 1;
        }
				LED_LED1_OFF();
}

void USART2_IRQHandler(void) // Interrupt on char recieving from USART2 (Car ECU)
{
	unsigned char data, ptr;
	unsigned char tmp[64];
	LED_LED2_ON();
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != (u16)RESET)
  {
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		// code here USART_ReceiveData(USART2);
		data = USART_ReceiveData(USART2);
//		sprintf(tmp, "\n\r\n\rECU >> 0x%x\n\r\n\r", data);
//		PutString(tmp);
		for (ptr = 0; ptr < 8; ptr++)
		{
			if ((parameter_status[ptr] == 2) && (parameter[ptr] == data))
			{
				parameter_status[ptr] = 3;
				goto end;
			}
			if (parameter_status[ptr] == 3)
			{
				parameter_value[ptr] = data;
				parameter_status[ptr] = 1;
				request_next_param = 1;
			}
		}	
	}
end: LED_LED2_OFF();
}

void USART3_IRQHandler(void) // Interrupt on char recieving from USART3 (GPS)
{
	LED_LED3_ON();
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != (u16)RESET)
  {
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    GPS_NMEA[GPS_NMEA_offset] = USART_ReceiveData(USART3);
		if(GPS_NMEA[GPS_NMEA_offset] == 0x0a) 
		{
			GPS_NMEA_complete = 1; // end of gps string
		}
			GPS_NMEA_offset++;
	}
		LED_LED3_OFF();
}

#if (OS_APP_HOOKS_EN > 0)

void App_TaskCreateHook(OS_TCB* ptcb)
{
}

void App_TaskDelHook(OS_TCB* ptcb)
{
   (void) ptcb;
}

#if OS_VERSION >= 251
void App_TaskIdleHook(void)
{
}
#endif

void App_TaskStatHook(void)
{
}

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
