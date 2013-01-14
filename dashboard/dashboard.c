#include "includes.h"
#include "..\GUIinc\GUI.h"
#include "..\GUIinc\WM.h" 
#include "..\GUIinc\BUTTON.h"			
#include <string.h>		
#include "dashboard.h"
#include <stdio.h>

#define true TRUE;
#define false FALSE;

#define LED4_ON()	 GPIO_SetBits(GPIOD, GPIO_Pin_6 );			 //LED4 
#define LED4_OFF()	GPIO_ResetBits(GPIOD, GPIO_Pin_6 ); 	 //LED4

extern unsigned char AndroidReceivedChar;
extern bool Controller_key_event, mute, vol_up, vol_down, prev, next, play_mode, fm_search, fm_memory;
extern float a;
extern volatile unsigned char ch1; 
extern volatile unsigned long FM_FREQ;

extern unsigned char ECU_data, ECU_param;
extern bool ECU_req_param, ECU_complete;

unsigned char ECU_rpm;
unsigned int CAR_rpm;
unsigned char CAR_cool_temp;

unsigned char parameter[8];
unsigned char parameter_value[8];
unsigned char parameter_status[8];
extern bool request_next_param;
unsigned char param = 0;

void dashboard_menu(void);
void FM_display(void);
void CAR_display(void);
bool ECU_Request_param(void);

void BT_power_on(void);

int AndroidPutChar(int ch)
{
	USART_SendData(USART1, (unsigned char) ch);
	while (!(USART1->SR & USART_FLAG_TXE));
	return (ch);
}

void AndroidPutString(u8 *s)
{
	while (*s != '\0')
	{
		AndroidPutChar(*s);
		s ++;
	}
}

void ECU_Send(unsigned char ch)
{
	unsigned char tmp[64];
	USART_SendData(USART2, ch);
	while (!(USART2->SR & USART_FLAG_TXE));
//	sprintf(tmp,"0x%x >> ECU\n\r",ch);
//	PutString(tmp);
}

void sendAndroidControl(unsigned int control)
{
	unsigned char tmp[16];
	sprintf(tmp, "IR:%d%c", control, 0x13);
	AndroidPutString(tmp);
}

void dashboard(void)
{
	unsigned char str[64];
//	BT_power_on();

	GUI_CURSOR_Hide();
	GUI_SetBkColor(GUI_BLACK);
	GUI_SetColor(GUI_GREEN);
	GUI_SetFont(&GUI_Font8_ASCII);
 	GUI_Clear();
	sendAndroidControl(PREV_TRACK);

	while (1) 
	{
	}
}

bool ECU_Request_param(void)
{
	unsigned char tmp[64];
	// cold start
	if ((parameter_status[0] == 0) && (parameter_status[1] == 0) && (parameter_status[2] == 0) && (parameter_status[3] == 0) && (parameter_status[4] == 0) && (parameter_status[5] == 0) && (parameter_status[6] == 0) && (parameter_status[7] == 0))
	{
		ECU_Send(parameter[param]);
		parameter_status[param] = 2; // status request parameter, value = 2
		param++; // pointer to next parameter
		request_next_param = false; // reset requesting next parameter flag
//		while(1);
		return true;
	} // end of cold start
	
	if (!request_next_param) 
	{
		if (parameter_status[param-1] == 2) // no data from ECU 
		{
			ECU_Send(parameter[param-1]); // retry request
//			sprintf(tmp,"========= RETRY =========\n\r");
//			PutString(tmp);
//			sprintf(tmp,"param = %d, parameter[param] = %x, parameter_status[param] = %d\n\r", param-1, parameter[param-1], parameter_status[param-1]);
//			PutString(tmp);
//			sprintf(tmp,"0x%x >> ECU\n\r=========================\n\r",parameter[param-1]);
//			PutString(tmp);
		}
		return false; // we don't need queue, one by one request !!!11
	}
	
	if (param > 7) param = 0; // check for the end of parameter list
	
	if (parameter_status[param] == 0) // not requested parameter, status value = 0
	{
			ECU_Send(parameter[param]);
			parameter_status[param] = 2; // status request parameter, value = 2
			request_next_param = false; // reset requesting next parameter flag
	}
	param++; // pointer to next parameter
	return true;
}

void CAR_display(void)
{
	CAR_rpm = 3550;
	CAR_cool_temp = 90;

	GUI_SetColor(GUI_WHITE);
	GUI_DrawLine(180, 0, 180, 180);					
	GUI_DrawLine(0, 180, 319, 180);					

	GUI_SetFont(&GUI_FontD24x32);
	if (CAR_rpm == 0) GUI_SetColor(GUI_WHITE);
	if ((CAR_rpm > 0) && (CAR_rpm <= 3500)) GUI_SetColor(GUI_GREEN);
	if ((CAR_rpm > 3500) && (CAR_rpm <= 4000)) GUI_SetColor(GUI_YELLOW);
	if ((CAR_rpm > 4000) && (CAR_rpm <= 4500)) GUI_SetColor(GUI_MAGENTA);
	if (CAR_rpm > 4500) GUI_SetColor(GUI_RED);
	if (CAR_rpm < 1000) GUI_DispDecAt(CAR_rpm, 20, 70, 3);
	else GUI_DispDecAt(CAR_rpm, 1, 70, 4);
	GUI_SetFont(&GUI_Font32_ASCII);
	GUI_DispStringAt("rpm", 105, 75);

	GUI_SetFont(&GUI_FontD24x32);
	if (CAR_cool_temp == 0) GUI_SetColor(GUI_WHITE);
	if ((CAR_cool_temp > 0) && (CAR_cool_temp <= 90)) GUI_SetColor(GUI_GREEN);
	if ((CAR_cool_temp > 90) && (CAR_cool_temp <= 110)) GUI_SetColor(GUI_YELLOW);
	if (CAR_cool_temp > 110) GUI_SetColor(GUI_RED);
	if (CAR_cool_temp < 100) GUI_DispDecAt(CAR_cool_temp, 40, 125, 2);
	else GUI_DispDecAt(CAR_cool_temp, 30, 125, 3);
	GUI_SetFont(&GUI_Font24B_ASCII);
	GUI_DispStringAt("o", 106, 120);
	GUI_SetFont(&GUI_Font32_ASCII);
	GUI_DispStringAt("C", 120, 130);
}

void BT_power_on(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_1 );
	GUI_Delay(4000);
	GPIO_ResetBits(GPIOA, GPIO_Pin_1 );
}
