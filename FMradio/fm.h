#include "includes.h"

#define Tea5767_WriteAddress1    0xc0
#define Tea5767_ReadAddress1     0xc1

unsigned long fm_ch[50];
unsigned char dat[4096];
volatile unsigned long FM_FREQ;
volatile unsigned long FM_PLL;
volatile unsigned char PLL_HIGH,num;
volatile unsigned char PLL_LOW;
volatile unsigned char rec_f,ch,ch1; 
volatile unsigned short keymsg;
//unsigned char SST25_buffer[4096],fac_id,dev_id;

OS_EVENT* _itMBOX;
BUTTON_Handle   _ahButton[3];
