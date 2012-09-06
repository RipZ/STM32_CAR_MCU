/*
 * main.h
 *
 *  Created on: Mar 29, 2011
 *      Author: Witek
 */

#ifndef MP3PLAYER_H_
#define MP3PLAYER_H_

#include "vs1003.h"
//#include "stm32f10x_it.h"
#include "..\FatFs\sdcard.h"
#include "..\FatFs\ff.h"
#include "..\FatFs\diskio.h"
#include "..\FatFs\timestamp.h"

/*
//defnicje portu diod LED
#define LED1_PORT			GPIOB
#define LED23_PORT			GPIOD
#define LED1_PORT_CLK		RCC_APB2Periph_GPIOB
#define LED23_PORT_CLK		RCC_APB2Periph_GPIOD
#define LED1_PIN			GPIO_Pin_5
#define LED2_PIN			GPIO_Pin_6
#define LED3_PIN			GPIO_Pin_3
*/
#define NEXT_TRACK	3
#define SEND_UART	1
#define PLAY_PAUSE	2
#define VOL_UP		6
#define	VOL_DOWN	4
#define BASS_UP		9
#define BASS_DOWN	7
#define TREBLE_UP	12
#define TREBLE_DOWN	10

#define SD_BlockSize           512

typedef enum {
	MP3_Player = 0,
	FM_Radio = 1,
	Main_Screen = 2,
	MP3_Player_1st_Run = 3,
	MP3_Player_Paused = 4
} System_Function;

void SD_Config(void);
void intToStr(char * string,int n);
void SD_Nvic_conf(void);
void clkInit(void);		// konfiguracja zegara
//void pinSetup(void); 	// konfiguracja portow
//void Maintain_Player_Keyboard_Event(FIL* File, DIR* Directory, FILINFO* File_Info);
void Next_Track(FIL* File, DIR* Directory, FILINFO* File_Info);
//UART
void Send_Playback_Time(void);
void Send_Bitrate(void);
void Send_Samplerate(void);
void Send_Volume(void);
uint8_t IsPlayable(char* File_Name);
unsigned char showBit(__IO uint16_t Bytes, unsigned char bitNum);
#endif /* MP3PLAYER_H_ */
