#include "includes.h"
#include "..\dashboard\dashboard.h"
#include "..\GUIinc\WM.h"

static	OS_STK App_TaskStartStk[APP_TASK_START_STK_SIZE];
static	OS_STK AppTaskUserIFStk[APP_TASK_USER_IF_STK_SIZE];
static	OS_STK AppTaskControllerStk[APP_TASK_CONTROLLER_STK_SIZE];

static	void App_TaskCreate(void);

static	void App_TaskStart(void* p_arg);
static	void	AppTaskUserIF (void *p_arg);

static	void AppTaskController(void* p_arg);
//static	void App_TaskLCD(void* p_arg);
//
//static	void App_TaskJoystick(void* p_arg);
#define LED_LED1_ON()	 GPIO_SetBits(GPIOC, GPIO_Pin_6 );			 //LED1 
#define LED_LED1_OFF()	GPIO_ResetBits(GPIOC, GPIO_Pin_6 ); 	 //LED1
#define LED_LED2_ON()	 GPIO_SetBits(GPIOC, GPIO_Pin_7 );			 //LED2 
#define LED_LED2_OFF()	GPIO_ResetBits(GPIOC, GPIO_Pin_7 ); 	 //LED2
#define LED_LED3_ON()	 GPIO_SetBits(GPIOD, GPIO_Pin_13 );		 //LED3 
#define LED_LED3_OFF()	GPIO_ResetBits(GPIOD, GPIO_Pin_13 ); 	 //LED3
#define LED_LED4_ON()	 GPIO_SetBits(GPIOD, GPIO_Pin_6 );			 //LED4 
#define LED_LED4_OFF()	GPIO_ResetBits(GPIOD, GPIO_Pin_6 ); 	 //LED4

// Android CarPC relative stuff
unsigned char AndroidReceivedChar;
unsigned char AndroidBuffer[8];
unsigned char AndroidBufferIndex;
bool Controller_key_event;
bool GetAndroidCommand;

// Car ECU relative stuff
unsigned char ECU_data, ECU_param;
bool ECU_req_param, ECU_complete;
extern unsigned char parameter[8];
extern unsigned char parameter_status[8];
extern unsigned char parameter_value[8];
bool request_next_param;

// Sony RM-X6S prototypes and stuff
int impulse;
unsigned int period[25];
void SonyIR(void);
bool SonyRead(void);
void Delay_25us(void);
unsigned int buttons[14][26] = {{167, 106, 106, 107, 166, 106, 286, 106, 106, 107, 166, 166, 106, 167, 106, 166, 106, 226, 107, 106, 226, 106, 226, 107, 285, 286}, // PUSH release
																{167, 106, 106, 107, 166, 106, 286, 106, 106, 107, 226, 166, 106, 167, 106, 166, 106, 226, 107, 106, 226, 106, 166, 107, 285, 286}, // PUSH press
																{167, 106, 106, 107, 166, 106, 286, 106, 106, 107, 166, 286, 106, 167, 106, 166, 106, 226, 107, 106, 226, 106, 226, 226, 285, 286}, // OFF press
																{167, 106, 106, 107, 166, 106, 286, 106, 106, 107, 166, 166, 106, 167, 106, 166, 106, 226, 107, 106, 226, 106, 226, 107, 285, 286}, // OFF release
																{167, 106, 106, 107, 166, 106, 286, 106, 106, 107, 106, 166, 106, 167, 106, 166, 106, 226, 107, 106, 226, 106, 286, 106, 285, 286}, // ATT press
																{167, 106, 106, 107, 166, 106, 286, 106, 106, 107, 166, 166, 106, 167, 106, 166, 106, 226, 107, 106, 226, 106, 226, 107, 285, 286}, // ATT release
																{167, 106, 106, 107, 166, 106, 286, 106, 106, 107, 286, 166, 106, 166, 106, 166, 107, 226, 106, 106, 226, 107, 106, 106, 286, 285}, // MODE press
																{167, 106, 106, 107, 166, 106, 286, 106, 106, 107, 166, 166, 106, 167, 106, 166, 106, 226, 107, 106, 226, 106, 226, 107, 285, 286}, // MODE release
																{167, 106, 106, 107, 166, 106, 286, 106, 106, 107, 286, 106, 166, 107, 106, 166, 106, 226, 107, 106, 226, 106, 167, 166, 226, 226}, // VOL+
																{167, 106, 106, 107, 166, 106, 286, 106, 106, 107, 226, 106, 166, 107, 106, 166, 106, 226, 107, 106, 226, 106, 107, 166, 226, 226}, // VOL-
																{167, 106, 106, 107, 166, 106, 286, 106, 106, 107, 286, 226, 166, 106, 106, 167, 106, 226, 106, 107, 226, 107, 106, 286, 226, 226}, // PREV
																{167, 106, 106, 107, 166, 106, 286, 106, 106, 107, 106, 286, 166, 106, 107, 166, 106, 226, 107, 106, 226, 106, 286, 226, 226, 226}, // NEXT
																{167, 106, 106, 107, 166, 106, 286, 106, 106, 107, 106, 106, 286, 107, 106, 166, 106, 226, 107, 106, 226, 106, 286, 166, 107, 226}, // DISC+
																{167, 106, 106, 107, 166, 106, 286, 106, 106, 107, 166, 106, 286, 107, 106, 166, 106, 226, 107, 106, 226, 106, 226, 166, 107, 226}}; // DISC-

// TEA5767 prototypes
extern void SetPLL(unsigned long freq);

// TDA7318 audio prototypes
extern void setAudioVolume(unsigned char volume);
extern void setAudioBalance(unsigned char balance);
extern void setAudioBass(unsigned char bass);
extern void setAudioTreble(unsigned char treble);
extern void setAudioMute(unsigned char mute);
extern void setAudioRearLeftVolume(unsigned char rl_volume);
extern void setAudioRearRightVolume(unsigned char rr_volume);
extern void setAudioSource(unsigned char source);

int main(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	 CPU_INT08U os_err;
	 //BSP_Init();			
	 //BSP_IntDisAll();																						/* Disable all ints until we are ready to accept them.	*/
	 CPU_IntDis();
	 
	 OSInit();																									 /* Initialize "uC/OS-II, The Real-Time Kernel".				 */

	 BSP_Init();																								 /* Initialize BSP functions.	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIOC->BSRR = 1<<0;

	 os_err = OSTaskCreate((void (*) (void *)) App_TaskStart,
							 /* Create the start task.															 */
													(void *) 0,
							 (OS_STK *) &App_TaskStartStk[APP_TASK_START_STK_SIZE - 1],
							 (INT8U) APP_TASK_START_PRIO);
	 
#if (OS_TASK_NAME_SIZE >= 11)
	 OSTaskNameSet(APP_TASK_START_PRIO, (CPU_INT08U *) "Start Task", &os_err);
#endif

//	 InfoSem = OSSemCreate(0); 
//	 Disp_Box = OSMboxCreate((void *) 0);							 
	 OSTimeSet(0);
	 OSStart();																									/* Start multitasking (i.e. give control to uC/OS-II).	*/

	 return (0);
}

static	void App_TaskStart(void* p_arg)
{
	 (void) p_arg;
	 OS_CPU_SysTickInit();																			 /* Initialize the SysTick.			 */
#if (OS_TASK_STAT_EN > 0)
	 OSStatInit();																							 /* Determine CPU capacity.															*/
#endif
	 App_TaskCreate();
	 while (1)
	 {
//			LED_LED1_ON();
//			OSTimeDlyHMSM(0, 0, 0, 100);
			
//			LED_LED1_OFF();
			OSTimeDlyHMSM(0, 0, 0, 1);
	 }
}

static	void App_TaskCreate(void)
{
	 CPU_INT08U os_err;
	 
	 OSTaskCreateExt(AppTaskUserIF,(void *)0,(OS_STK *)&AppTaskUserIFStk[APP_TASK_USER_IF_STK_SIZE-1],APP_TASK_USER_IF_PRIO,APP_TASK_USER_IF_PRIO,(OS_STK *)&AppTaskUserIFStk[0],
										APP_TASK_USER_IF_STK_SIZE,
										(void *)0,
										OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
										
//	 OSTaskCreateExt(AppTaskController,(void *)0,(OS_STK *)&AppTaskControllerStk[APP_TASK_CONTROLLER_STK_SIZE-1],APP_TASK_CONTROLLER_PRIO,APP_TASK_CONTROLLER_PRIO,(OS_STK *)&AppTaskControllerStk[0],
//										APP_TASK_CONTROLLER_STK_SIZE,
//										(void *)0,
//										OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);


//	 os_err = OSTaskCreate((void (*) (void *)) App_TaskLCD, (void *) 0,
//							 (OS_STK *) &App_TaskLCDStk[APP_TASK_LCD_STK_SIZE - 1],
//							 (INT8U) APP_TASK_LCD_PRIO);
//	 printf("Creat App_TaskLCD£¡\r\n");
//#if (OS_TASK_NAME_SIZE >= 9)
//	 OSTaskNameSet(APP_TASK_LCD_PRIO, "LCD", &os_err);
//#endif

//	 os_err = OSTaskCreate((void (*) (void *)) App_TaskKbd, (void *) 0,
//							 (OS_STK *) &App_TaskKbdStk[APP_TASK_KBD_STK_SIZE - 1],
//							 (INT8U) APP_TASK_KBD_PRIO);
//	 printf("Creat App_TaskKbd£¡\r\n");
}

extern void LCD_L0_FillRect(int x0, int y0, int x1, int y1);

static	void	AppTaskUserIF (void *p_arg)
{
 (void)p_arg;
	GUI_Init();
	while(1) 
	{
		dashboard();
	}
}

void EXTI0_IRQHandler(void)
{
	unsigned char i, j, k, key;
	SonyIR();
	for(i=0; i<13; i++)
	{
		k=1;
		for(j=0; j<25; j++)
		{
			if((buttons[i][j] < period[j]-2) || (buttons[i][j] > period[j]+2)) // ïðîâåðêà âûõîäà çà óñòàíîâëåííûé ïðåäåë (+-2)
			{ // not equal
				k=0;
				break;
			}
		}
		if(k) { key = i; break; }
		else key = 255;
	}
	switch (key) {
		case 255: // not recoginazing
			break;
		case 0:
			// key = PUSH release
			break;
		case 1:
			// key = PUSH press
			break;
		case 2:
			// key = OFF press
			break;
		case 3:
			// key = OFF release
			break;
		case 4:
			// key = ATT press
			sendAndroidControl(MUTE);
			break;
		case 5:
			// key = ATT release
			break;
		case 6:
			// key = MODE press
			sendAndroidControl(MODE);
			break;
		case 7:
			// key = MODE release
			break;
		case 8:
			// key = VOL+
			sendAndroidControl(VOLUME_UP);
			break;
		case 9:
			// key = VOL-
			sendAndroidControl(VOLUME_DOWN);
			break;
		case 10:
			// key = PREV
			sendAndroidControl(PREV_TRACK);
			break;
		case 11:
			// key = NEXT
			sendAndroidControl(NEXT_TRACK);
			break;
		case 12:
			// key = DISC+
			sendAndroidControl(PRESET_UP);
			break;
		case 13:
			// key = DISC-
			sendAndroidControl(PRESET_DOWN);
			break;
		default:
		break;
	}
/*

	GUI_DispDecAt(period[0], 10, 10, 3);
	GUI_DispDecAt(period[1], 10, 20, 3);
	GUI_DispDecAt(period[2], 10, 30, 3);
	GUI_DispDecAt(period[3], 10, 40, 3);
	GUI_DispDecAt(period[4], 10, 50, 3);
	GUI_DispDecAt(period[5], 10, 60, 3);
	GUI_DispDecAt(period[6], 10, 70, 3);
	GUI_DispDecAt(period[7], 10, 80, 3);
	GUI_DispDecAt(period[8], 10, 90, 3);
	GUI_DispDecAt(period[9], 10, 100, 3);
	GUI_DispDecAt(period[10], 10, 110, 3);
	GUI_DispDecAt(period[11], 10, 120, 3);
	GUI_DispDecAt(period[12], 10, 130, 3);
	GUI_DispDecAt(period[13], 10, 140, 3);
	GUI_DispDecAt(period[14], 10, 150, 3);
	GUI_DispDecAt(period[15], 10, 160, 3);
	GUI_DispDecAt(period[16], 10, 170, 3);
	GUI_DispDecAt(period[17], 10, 180, 3);
	GUI_DispDecAt(period[18], 10, 190, 3);
	GUI_DispDecAt(period[19], 10, 200, 3);
	GUI_DispDecAt(period[20], 10, 210, 3);
	GUI_DispDecAt(period[21], 60, 10, 3);
	GUI_DispDecAt(period[22], 60, 20, 3);
	GUI_DispDecAt(period[23], 60, 30, 3);
	GUI_DispDecAt(period[24], 60, 40, 3);
	GUI_DispDecAt(period[25], 60, 50, 3);
*/	
	EXTI->PR |= (1<<1);
}

void Delay_25us(void)
{
	int dummy;
	for (dummy=0;dummy<192;dummy++);
}

void SonyIR(void)
{
	int delay;
	impulse = 0;
	while(impulse<26)
	{
		Delay_25us();
		LED_LED4_ON();
		delay = 0;
		while(!SonyRead())
		{
			delay++;
		}
		LED_LED4_OFF();
		period[impulse] = delay;
		impulse++;
	}
}

bool SonyRead(void)
{
	return GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_0);
}

void USART1_IRQHandler(void) // Interrupt on char recieving from USART1 (Android board)
{
	unsigned char param[16];
	LED_LED1_ON();
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != (u16)RESET)
				{
					USART_ClearITPendingBit(USART1, USART_IT_RXNE);
					AndroidReceivedChar = USART_ReceiveData(USART1);
					AndroidBuffer[AndroidBufferIndex] = AndroidReceivedChar;
					AndroidBufferIndex++;
					if (AndroidReceivedChar == 19) // fucking programmer
					{
						AndroidBuffer[AndroidBufferIndex-1] = 0; //EOL
						AndroidBufferIndex = 0;

						strcpy(param, AndroidBuffer+1);

						switch (AndroidBuffer[0]) {
								case 'R':
									if (!strcmp(param, "0")) {
										GUI_DispString("Radio Mute\n");
									// TODO: Radio mute
										break;
									}
									GUI_DispString("Freq = "); GUI_DispDec(atol(param)*100000, 9); GUI_DispString("Hz\n");
									SetPLL(atol(param)*100000);
									break;
								case 'V':
									setAudioVolume(atoi(param));
									GUI_DispString("Volume = "); GUI_DispDec(atoi(param), 2); GUI_DispString("\n");
									break;
								case 'B':
									setAudioBalance(atoi(param));
									GUI_DispString("Balance = "); GUI_DispDec(atoi(param), 2); GUI_DispString("\n");
									break;
								case 'J':
									setAudioBass(atoi(param));
									GUI_DispString("Bass = "); GUI_DispDec(atoi(param), 2); GUI_DispString("\n");
									break;
								case 'K':
									setAudioTreble(atoi(param));
									GUI_DispString("Treble = "); GUI_DispDec(atoi(param), 2); GUI_DispString("\n");
									break;
								case 'L':
									setAudioRearLeftVolume(atoi(param));
									GUI_DispString("Rear Left volume = "); GUI_DispDec(atoi(param), 2); GUI_DispString("\n");
									break;
								case 'X':
									setAudioRearRightVolume(atoi(param));
									GUI_DispString("Rear Right volume = "); GUI_DispDec(atoi(param), 2); GUI_DispString("\n");
									break;
								case 'M':
									setAudioMute(atoi(param));
									GUI_DispString("Audio mute = "); GUI_DispDec(atoi(param), 2); GUI_DispString("\n");
									break;
								case 'S':
									setAudioSource(atoi(param));
									GUI_DispString("Audio source = "); GUI_DispDec(atoi(param), 2); GUI_DispString("\n");
									break;
								default:
									break;
						}
					}
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
*																		 OS_TCBInit() HOOK (APPLICATION)
*
* Description : This function is called by OSTCBInitHook(), which is called by OS_TCBInit() after setting
*							 up most of the TCB.
*
* Argument : ptcb		is a pointer to the TCB of the task being created.
*
* Note		 : (1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/

#if OS_VERSION >= 204
void App_TCBInitHook(OS_TCB* ptcb)
{
	 (void) ptcb;
}
#endif

#endif
