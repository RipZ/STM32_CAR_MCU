#include "includes.h"
#include "tea5767.h"		  
#include "..\GUIinc\GUI.h"
#include "..\GUIinc\WM.h" 
#include "..\GUIinc\BUTTON.h"
#include "..\usart\SPI_Flash.h"

volatile unsigned long FM_PLL;
volatile unsigned char PLL_HIGH;
volatile unsigned char PLL_LOW;

extern bool I2C_Write(uint8_t* pBuffer, uint8_t WriteAddr, uint8_t NumByteToWrite);

uint8_t TEA5767_Buffer[] = {0XF0,0X2C,0XD0,0X12,0X40};

void SetPLL(unsigned long freq)
{	     
   FM_PLL=(unsigned long)((4000*(freq/1000+225))/32768);
   PLL_HIGH=(unsigned char)((FM_PLL >> 8)&0X3f);
   TEA5767_Buffer[0]=PLL_HIGH;
   PLL_LOW=(unsigned char)FM_PLL;
   TEA5767_Buffer[1]= PLL_LOW;
   I2C_Write(TEA5767_Buffer, TEA5767_I2C_ADDRESS, 5);
}
