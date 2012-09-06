#include "includes.h"
#include "..\GUIinc\GUI.h"
#include "..\GUIinc\WM.h" 
#include "..\GUIinc\BUTTON.h"		  
#include <string.h>		
#include "dashboard.h"
#include <stdio.h>
#include "..\mp3player\mp3player.h"

#define true TRUE;
#define false FALSE;

extern unsigned char Controller_key;
extern bool Controller_key_event, mute, vol_up, vol_down, prev, next, play_mode, fm_search, fm_memory;
extern float a;
extern volatile unsigned char ch1; 
extern volatile unsigned long FM_FREQ;

extern unsigned char GPS_NMEA[256];
extern unsigned char GPS_NMEA_offset;
bool GPS_NMEA_fixed;
extern bool GPS_NMEA_complete;

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

unsigned char filename[128];

void dashboard_menu(void);
void FM_display(void);
void CAR_display(void);
bool ECU_Request_param(void);

int PutChar(int ch)
{
  USART_SendData(USART1, (unsigned char) ch);
  while (!(USART1->SR & USART_FLAG_TXE));
  return (ch);
}

void PutString(u8 *s)
{
  while (*s != '\0')
  {
    PutChar(*s);
    s ++;
  }
}

void ECU_Send(unsigned char ch)
{
	unsigned char tmp[64];
	USART_SendData(USART2, ch);
  while (!(USART2->SR & USART_FLAG_TXE));
	sprintf(tmp,"0x%x >> ECU\n\r",ch);
	PutString(tmp);
}

int GetKey (void)  {
  while (!(USART1->SR & USART_FLAG_RXNE));
  return ((int)(USART1->DR & 0x1FF));
}

void dashboard(void)
{
//	int key=0;
//	int val;
//	unsigned char buffer[2048];
//	unsigned char FLASH_write_buffer[40];
//	unsigned char FLASH_read_buffer[40];

	SD_CardInfo SDCardInfo;
	SD_Error Status = SD_OK;
	
  FRESULT fsresult;               //return code for file related operations
  FATFS myfs;                     //FAT file system structure, see ff.h for details
  FIL myfile;                     //file object

	unsigned char GPS_speed;
	unsigned char GPS_speed_knots[8];
	float GPS_speed_kmph;
	
	unsigned char debug_tmp[512];
	
	unsigned char nmea_offset, tmp;
	
    FIL fsrc, fdst;      		// file objects
    UINT br, bw;         		// File R/W count
    DIR dirs;
//		char path[50]={""};
	
//	unsigned char req_param;
	
	parameter[0] = 0x07; parameter_status[0] = 0; parameter_value[0] = 0; // cool
	parameter[1] = 0x21; parameter_status[1] = 0; parameter_value[1] = 0; // rpm
	parameter[2] = 0x06; parameter_status[2] = 0; parameter_value[2] = 0; // tima
	parameter[3] = 0x14; parameter_status[3] = 0; parameter_value[3] = 0; // vbatt
	parameter[4] = 0x26; parameter_status[4] = 0; parameter_value[4] = 0; // knck
	parameter[5] = 0x05; parameter_status[5] = 0; parameter_value[5] = 0;
	parameter[6] = 0x06; parameter_status[6] = 0; parameter_value[6] = 0;
	parameter[7] = 0x08; parameter_status[7] = 0; parameter_value[7] = 0;
	request_next_param = false;
	
	PutString("\r\nDashboard starting\r\n");
	GUI_CURSOR_Hide();
	GUI_SetBkColor(GUI_BLACK);
 	GUI_Clear();

	fm_tuned=0; // first run, PLL not turned
	play_mode = true;

	CAR_display();

	if (play_mode) // FM Radio
	{
		GUI_SetColor(GUI_GREEN);
		GUI_SetFont(&GUI_Font32B_ASCII);
		GUI_DispStringAt("FM  ", 1, 182);
		FM_display(); 
	}
	
	while(1) {
		if (Controller_key_event)
			{
				GUI_SetColor(GUI_GREEN);
				GUI_SetFont(&GUI_Font32B_ASCII);
				if (!play_mode) 
				{
					GUI_DispStringAt("FM  ", 1, 182);
					FM_display();
				}
				else GUI_DispStringAt("MP3", 1, 182);

				if (mute) {
					GUI_SetColor(GUI_RED);
//					GUI_SetFont(&GUI_Font32B_ASCII);
					GUI_DispStringAt("M", 260, 10);
				}
				else {
					GUI_SetColor(GUI_BLACK);
//					GUI_SetFont(&GUI_Font32B_ASCII);
					GUI_DispStringAt("   ", 260, 10);
				}
				if (next && prev) 
				{
					FM_display();
				}
			Controller_key_event = false;
	
			WM_ExecIdle();
			}

			if (GPS_NMEA_complete) // Complete NMEA sting in buffer
			{
				nmea_offset = 0;
				if ((GPS_NMEA[nmea_offset] == '$') && (GPS_NMEA[nmea_offset+1] == 'G') && (GPS_NMEA[nmea_offset+2] == 'P') && (GPS_NMEA[nmea_offset+3] == 'R') && (GPS_NMEA[nmea_offset+4] == 'M') && (GPS_NMEA[nmea_offset+5] == 'C') && (GPS_NMEA[nmea_offset+6] == ','))
				{
					nmea_offset = 7;
					tmp = 0;
					while (GPS_NMEA[nmea_offset] != ',') // get time
					{
						nmea_offset++;
					}
					
					nmea_offset++;
					
					while (GPS_NMEA[nmea_offset] != ',') // get position fixed
					{
						if (GPS_NMEA[nmea_offset] == 'A')
							{ GPS_NMEA_fixed = true; } // GPS coordinates valid 'A'
						else
							{ GPS_NMEA_fixed = false; } // GPS coordinates not valid 'V'
						
						nmea_offset++;
					}
					nmea_offset++;
					
					while (GPS_NMEA[nmea_offset] != ',') // get latitude
					{
//						GPS_latitude[tmp] = GPS_NMEA[nmea_offset];
						nmea_offset++; //tmp++;
					}
					nmea_offset++;
					
					while (GPS_NMEA[nmea_offset] != ',') // get N/S indicator
					{
//						GPS_ns = GPS_NMEA[nmea_offset];
						nmea_offset++;
					}
					nmea_offset++;

					tmp = 0;

					while (GPS_NMEA[nmea_offset] != ',') // get longitude
					{
//						GPS_longitude[tmp] = GPS_NMEA[nmea_offset];
						nmea_offset++; //tmp++;
					}
					nmea_offset++;
					
					while (GPS_NMEA[nmea_offset] != ',') // get E/W indicator
					{
//						GPS_ew = GPS_NMEA[nmea_offset];
						nmea_offset++;
					}
					nmea_offset++;

					tmp = 0;

					while (GPS_NMEA[nmea_offset] != ',') // get speed in knots
					{
						GPS_speed_knots[tmp] = GPS_NMEA[nmea_offset];
						nmea_offset++; tmp++;
					}

					if (GPS_NMEA_fixed) // display speed from GPS if coordinates fixed 'A'
					{
						GPS_speed_kmph = atof((char *) GPS_speed_knots) * 1.852;
						GPS_speed = (unsigned char) GPS_speed_kmph;
						GUI_ClearRect(0, 0, 100, 68);
						GUI_SetFont(&GUI_FontD36x48);
//						GUI_SetColor(GUI_BLACK); GUI_DispStringAt("000", 1, 1);
						
						if (GPS_speed == 0) GUI_SetColor(GUI_WHITE);
						if ((GPS_speed > 0) && (GPS_speed <= 60)) GUI_SetColor(GUI_GREEN);
						if ((GPS_speed > 60) && (GPS_speed <= 90)) GUI_SetColor(GUI_YELLOW);
						if ((GPS_speed > 90) && (GPS_speed <= 110)) GUI_SetColor(GUI_MAGENTA);
						if (GPS_speed > 110) GUI_SetColor(GUI_RED);
						
						if (GPS_speed < 10) GUI_DispDecAt(GPS_speed, 40, 1, 1);
						if ((GPS_speed >= 10) && (GPS_speed < 100)) GUI_DispDecAt(GPS_speed, 25, 1, 2);
						if (GPS_speed >= 100) GUI_DispDecAt(GPS_speed, 1, 1, 3);
					}
					if (GPS_NMEA_fixed) // print km/h in WHITE if GPS coordinates fixed, else print in RED (not ready)
					{
						GUI_SetColor(GUI_WHITE);
					}
					else GUI_SetColor(GUI_RED);
					GUI_SetFont(&GUI_Font32_ASCII);
					GUI_DispStringAt("km/h", 110, 23);
				}
				GPS_NMEA_complete = false; 
				GPS_NMEA_offset = 0;
			}
// car ECU requesting parameters
//			ECU_Request_param(); // request parameter from car ECU
	// sdcard fat test
  sprintf(debug_tmp, "\n");
  /*-------------------------- SD Init ----------------------------- */
  Status = SD_Init();

  if (Status == SD_OK)
  {
    sprintf(debug_tmp, "SD Card initialized ok.\n");
   /*----------------- Read CSD/CID MSD registers ------------------*/
    Status = SD_GetCardInfo(&SDCardInfo);
  }
  else
  {
    sprintf(debug_tmp, "SD Card did not initialize, check that a card is inserted. SD_Error code: %d.  See sdcard.h for SD_Error code meaning.\n", Status);
    while(1);  //infinite loop
  }

  if (Status == SD_OK)
  {
    sprintf(debug_tmp, "SD Card information retrieved ok.\n");
    /*----------------- Select Card --------------------------------*/
    Status = SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16));
  }
  else
  {
    sprintf(debug_tmp, "Could not get SD Card information. SD_Error code: %d.  See sdcard.h for SD_Error code meaning.\n", Status);
    while(1);  //infinite loop
  }

  if (Status == SD_OK)
  {
    sprintf(debug_tmp, "SD Card selected ok.\n");
   /*----------------- Enable Wide Bus Operation --------------------------------*/
    Status = SD_EnableWideBusOperation(SDIO_BusWide_4b);
  }
  else
  {
    sprintf(debug_tmp, "SD Card selection failed. SD_Error code: %d.  See sdcard.h for SD_Error code meaning.\n", Status);
    while(1);  //infinite loop
  }

  if (Status == SD_OK)
     sprintf(debug_tmp, "SD Card 4-bit Wide Bus operation successfully enabled.\n");
  else
  {
    sprintf(debug_tmp, "Could not enable SD Card 4-bit Wide Bus operation, will revert to 1-bit operation.\nSD_Error code: %d.  See sdcard.h for SD_Error code meaning.\n", Status);
  }

/*-----------------------------------------------------------------------------
Beginning of FAT file system related code.  The following code shows steps 
necessary to create, read, and write files.  
See http://elm-chan.org/fsw/ff/00index_e.html for more information.
This code assumes a single logical drive, drive number 0.  It also assumes a single partition.
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Mount the file system on logical drive 0.  Mounting associates the file system 
structure with a logical drive number.  Mounting must be done before any file 
operations.  Mounting does not write anything to the SD card, it simply 
initializes and associates the file structure.  The file system structure 
members are not filled in until the first file operation after f_mount.
-----------------------------------------------------------------------------*/
  fsresult = f_mount(0, &myfs);   
  if (fsresult == FR_OK)
     sprintf(debug_tmp, "FAT file system mounted ok.\n");
  else
    sprintf(debug_tmp, "FAT file system mounting failed. FRESULT Error code: %d.  See FATfs/ff.h for FRESULT code meaning.\n", fsresult);

	while(1);
	
/*
			SD_Nvic_conf();
			SD_Config();
	disk_initialize(0);

	res = f_mount(0, &fs);
	res = f_opendir(&dirs, "");
	res = f_readdir(&dirs,&finfo);
	
	Next_Track(&fsrc,&dirs,&finfo);

	while(1);
*/
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
			sprintf(tmp,"========= RETRY =========\n\r");
			PutString(tmp);
			sprintf(tmp,"param = %d, parameter[param] = %x, parameter_status[param] = %d\n\r", param-1, parameter[param-1], parameter_status[param-1]);
			PutString(tmp);
			sprintf(tmp,"0x%x >> ECU\n\r=========================\n\r",parameter[param-1]);
			PutString(tmp);
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

void FM_display(void)
{
	a = FM_FREQ;
	a = a/1000000;
	GUI_SetFont(&GUI_Font32B_ASCII);
	GUI_SetColor(GUI_RED);
//	GUI_DispStringAt("Ch.", 140, 70);
	GUI_DispDecAt(ch1, 4, 210, 2);

	GUI_GotoXY(70,188);
	GUI_SetColor(GUI_YELLOW);
	GUI_SetFont(&GUI_FontD36x48);
	GUI_DispFloatFix(a, 5, 1);

	GUI_SetFont(&GUI_Font32B_ASCII);
	GUI_SetColor(GUI_YELLOW);
	GUI_DispStringAt("MHz", 260, 210);
}