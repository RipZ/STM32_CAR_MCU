/* Includes */
#include <stddef.h>
#include <string.h>
#include "stm32f10x.h"
#include "mp3player.h"

extern void PutString(u8 *s);

BYTE buffer[1024]; // file copy buffer
volatile unsigned int count = 0;
uint16_t poprawka = 0;

System_Function State = MP3_Player_1st_Run;

int main_menu(void)
{

	int i;
	FATFS fs;            		// Work area (file system object) for logical drive
    FIL fsrc, fdst;      		// file objects

    FRESULT res;         		// FatFs function common result code
    UINT br, bw;         		// File R/W count
    FILINFO finfo;
    DIR dirs;
    UINT bytes_saved;
    uint8_t key_pressed;

//    uint16_t VOL;

	clkInit();
//	pinSetup();

//	Keyboard_Pin_setup();
//	Keyboard_Extern_Interrupts_Setup();
//	Keyboard_Timer_Setup();

//	USART_GPIO_Conf();
//	USART_NVIC_Conf();
//	USART_config();

	VS1003_GPIO_conf();
	VS1003_SPI_conf();

	VS1003_Start();

	SD_Nvic_conf();
	SD_Config();

//	Keyboard_Set_All_Columns();

	while (1)
	{
		if ( State == MP3_Player )
		{
			if(f_read(&fsrc, buffer, sizeof(buffer), &br) == 0)
			{
			SCI_ChipSelect(RESET);
			SDI_ChipSelect(SET);
			for (i=0;i<br;i++)
				{
					while(GPIO_ReadInputDataBit(DREQ_PORT,DREQ_PIN) == 0);
					SPIPutChar(buffer[i]);
				}

			}
			//End of file
			if(br == 0)
			{
				Next_Track(&fsrc,&dirs,&finfo);
			}
			//Maintain_Player_Keyboard_Event(&fsrc,&dirs,&finfo);

		}
		else if( State == MP3_Player_1st_Run)
		{
			disk_initialize(0);
			res = f_mount(0, &fs);
			res = f_opendir(&dirs, "");
			Next_Track(&fsrc,&dirs,&finfo);
			State = MP3_Player;
		}
		else if( State == MP3_Player_Paused)
		{
			//Maintain_Player_Keyboard_Event(&fsrc,&dirs,&finfo);
		}
	}
}

void Send_Playback_Time(){
    char time_buf[12]="";
	uint8_t sekundy;
	uint8_t minuty;
	uint16_t playback_time = VS1003_GetDecodeTime() - poprawka;
	minuty = playback_time / 60;
	sekundy = playback_time%60;
	intToStr(time_buf,minuty);
	printf("Czas odtwarzania: ");
	printf(time_buf);
	printf(":");
	intToStr(time_buf,sekundy);
	printf(time_buf);
	printf("\r\n");

}

void Send_Bitrate(){
    char bitr_buf[3]="";
    printf("Przeplywnosc:     ");
    intToStr(bitr_buf,VS1003_GetBitrate());
    printf(bitr_buf);
	printf(" kbit/s\r\n");
}

void Send_Samplerate(){
	 char samp_buf[5]="";
	 printf("Probkowanie:      ");
	 intToStr(samp_buf,VS1003_GetSampleRate());
	 printf(samp_buf);
	 printf(" Hz\r\n");
}

void Send_Volume(){
	uint8_t vol = VS1003_GetVolume();
	char vol_buf[3]="";
	printf("Volume:         -");
	intToStr(vol_buf,vol/2);
	printf(vol_buf);
	if (vol%2!=0)
		printf(",5 dB\r\n");
	else
		printf(" dB\r\n");
}
/*
void Maintain_Player_Keyboard_Event(FIL* File, DIR* Directory, FILINFO* File_Info){
	uint8_t Last_Keyboard_Event = Get_Key_Value();
	if(Last_Keyboard_Event != Nothing_PRESSED)
	{
		switch(Last_Keyboard_Event){
		case VOL_UP:
			VS1003_VolumeUp(1);
			break;
		case VOL_DOWN:
			VS1003_VolumeDown(1);
			break;
		case BASS_UP:
			VS1003_BassUp(1);
			break;
		case BASS_DOWN:
			VS1003_BassDown(1);
			break;
		case TREBLE_UP:
			VS1003_TrebleUp(1);
			break;
		case TREBLE_DOWN:
			VS1003_TrebleDown(1);
			break;
		case NEXT_TRACK:
			Next_Track(File,Directory,File_Info);
			break;
		case PLAY_PAUSE:
			if(State == MP3_Player)
				State = MP3_Player_Paused;
			else if(State == MP3_Player_Paused)
				State = MP3_Player;
			break;
		case SEND_UART:
			USART_Clean_Screen();
			Send_Playback_Time();
			Send_Bitrate();
			Send_Samplerate();
			Send_Volume();
		}
	}
}
*/
/*
 * @brief Switching to the next track in current directory.
 * 	If it is last file, jumps to first.
 * 	@param	File: Pointer to file object.
 * 	@param	Directory: Pointer to opened directory object;
 * 	@param	File_Info: Pointer to FILINFO object in which information
 * 		will be stored.
 */

void Next_Track(FIL* File, DIR* Directory, FILINFO* File_Info){
	FRESULT res = FR_OK;

	PutString(&File_Info->fname[0]);
	return;
	
	while(res == FR_OK){
		res = f_readdir(Directory,File_Info);
		if (res == FR_OK && File_Info->fname[0])
		{
			if (IsPlayable(File_Info->fname))
			{	if (State != MP3_Player_1st_Run)
				{
					res = f_close(File);
				}
				res = f_open(File, File_Info->fname, FA_READ);
				if (res == FR_OK)
				{
					// 4 zeros between streams, as VS1003 datasheet says
					while(GPIO_ReadInputDataBit(DREQ_PORT,DREQ_PIN) == 0);
					SPIPutChar(0);
					SPIPutChar(0);
					SPIPutChar(0);
					SPIPutChar(0);
					poprawka = VS1003_GetDecodeTime();
					break;
				}
				else
				{
					//routines for file opening error
					while(1);
				}
			}

		}
		else
		{
			//End of directory, jumping to its beginning
			res = f_opendir(Directory, "");
			if (res == FR_OK)
			{
				Next_Track(File,Directory,File_Info);
			}

		}
	}


}

uint8_t IsPlayable(char* File_Name){
	uint8_t result = FALSE;
	if (strstr(File_Name,".MP3") || strstr(File_Name,".WMA") || strstr(File_Name,".MID")
			||strstr(File_Name,".mp3") || strstr(File_Name,".wma") || strstr(File_Name,".mid"))
		result = TRUE;
	return result;
}


void clkInit() {
	/* Konfiguracja zegara */

	/* Reset RCC (na wszelki wypadek) */
	RCC_DeInit();

	/* W³¹czenie HSE */
	RCC_HSEConfig(RCC_HSE_ON);

	/* Oczekiwanie na gotowoœæ HSE */
	while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET) {
	}

	/* W³¹czenie opóŸnieñ dla FLASH */
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	FLASH_SetLatency(FLASH_Latency_2);

	/* HCLK = SYSCLK */
	RCC_HCLKConfig(RCC_SYSCLK_Div1);

	/* PCLK2 = HCLK */
	RCC_PCLK2Config(RCC_HCLK_Div1);

	/* PCLK1 = HCLK/2 */
	RCC_PCLK1Config(RCC_HCLK_Div2);

	/* PLLCLK = 8MHz * 9 = 72 MHz */
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

	/* W³¹czenie PLL */
	RCC_PLLCmd(ENABLE);

	/* Oczekiwanie na gotowoœæ PLL */
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {
	}

	/* Wybór PLL na zegar systemowy */
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

	/* Oczekiwanie na wybór zegara systemowego */
	while (RCC_GetSYSCLKSource() != 0x08) {
	}
}
/*
void pinSetup() {
	GPIO_InitTypeDef GPIO_InitStruct;

	// wlaczenie zegara dla GPIO
	RCC_APB2PeriphClockCmd(LED1_PORT_CLK | LED23_PORT_CLK, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

	// konfiguracja pinów
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_Pin = LED1_PIN;
	GPIO_Init(LED1_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = LED2_PIN | LED3_PIN;
	GPIO_Init(LED23_PORT, &GPIO_InitStruct);

}
*/
void SD_Nvic_conf(){
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure the NVIC Preemption Priority Bits */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;					  //ÅäÖÃSDIOÖÐ¶ÏÔ´
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

void SD_Config(void){
	char tmp[16];
	
	SD_CardInfo SDCardInfo;
	SD_Error Status = SD_OK;

	  Status = SD_Init();

	  if (Status == SD_OK)
	  {
	    /*----------------- Read CSD/CID MSD registers ------------------*/
	    Status = SD_GetCardInfo(&SDCardInfo);
			sprintf(tmp,"%d",Status);
			PutString("\r\nSD_GetCardInfo..... "); PutString(tmp);
			sprintf(tmp,"%d",SDCardInfo.CardType);
			PutString("\r\n\t\t info.."); PutString(tmp);
	  }

	  if (Status == SD_OK)
	  {
	    /*----------------- Select Card --------------------------------*/
	    Status = SD_SelectDeselect((u32) (SDCardInfo.RCA << 16));
	  }

	  if (Status == SD_OK)
	  {
	    Status = SD_EnableWideBusOperation(SDIO_BusWide_4b);
	  }

	  /* Set Device Transfer Mode to DMA */
	  if (Status == SD_OK)
	  {
	    Status = SD_SetDeviceMode(SD_DMA_MODE);//oet
	 //   Status = SD_SetDeviceMode(SD_POLLING_MODE);
	 //   Status = SD_SetDeviceMode(SD_INTERRUPT_MODE);
	  }
}

//Najmniej znaczacy ma numer 0
unsigned char showBit(__IO uint16_t Bytes, unsigned char bitNum){
	return ((Bytes & (1 << bitNum) )>> bitNum);
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	if(count) count--;
}

void intToStr(char * string,int n)
{
     int i = 0;
     int j = 0;
     char tmp[12]="";
     char ret[12]="";
     if(n < 0) {
          ret[0] = '-';
          i++;
          n = -n;
     }
     do {
          tmp[j] = n % 10 + 48;
          n -= n % 10;
          if(n > 9) j++;
     }
     while(n /= 10);
     while(j>=0)
     {
    	 ret[i] = tmp[j];
    	 i++;
    	 j--;
     }
     strcpy(string,ret);
}

/*
 * Minimal __assert_func used by the assert() macro
 *
void __assert_func(const char *file, int line, const char *func, const char *failedexpr)
{
  while(1)
  {}
}

/*
 * Minimal __assert() uses __assert__func()
 *
void __assert(const char *file, int line, const char *failedexpr)
{
   __assert_func (file, line, NULL, failedexpr);
}
*/
#ifdef USE_SEE
#ifndef USE_DEFAULT_TIMEOUT_CALLBACK
/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval sEE_FAIL.
  */
uint32_t sEE_TIMEOUT_UserCallback(void)
{
  /* Return with error code */
  return sEE_FAIL;
}
#endif
#endif /* USE_SEE */

