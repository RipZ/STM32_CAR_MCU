#include "includes.h"
#include "..\GUIinc\GUI.h"
#include "..\GUIinc\WM.h" 
#include "..\GUIinc\BUTTON.h"
#include "fm.h"		  
#include <string.h>		
#include "..\usart\SPI_Flash.h"

//extern void SST25_R_BLOCK(unsigned long addr, unsigned char *readbuff, unsigned int BlockSize);
extern unsigned char fm_tuned;
extern bool I2C_ReadByte(u8* pBuffer,   u8 length,   u8 DeviceAddress);
extern bool I2C_Write(uint8_t* pBuffer, uint8_t WriteAddr, uint8_t NumByteToWrite);
//extern void SST25_W_BLOCK(uint32_t addr, u8 *readbuff, uint16_t BlockSize);
extern void Delay(__IO uint32_t nCount);
uint8_t Tx1_Buffer[] = {0XF0,0X2C,0XD0,0X12,0X40};
uint8_t Rx1_Buffer[] = {0XF0,0X2C,0XD0,0X12,0X40};

float a;
unsigned char sb,c;	

void SetPLL(void)
{	     
   FM_PLL=(unsigned long)((4000*(FM_FREQ/1000+225))/32768); 	         //计算PLL值
   if(rec_f==2) PLL_HIGH=(unsigned char)(((FM_PLL >> 8)&0X3f)|0xc0);	 //PLL高字节值--搜索模式静音
   else PLL_HIGH=(unsigned char)((FM_PLL >> 8)&0X3f);	                 //PLL高字节值
   Tx1_Buffer[0]=PLL_HIGH;		 						                 //I2C第一字节值
   PLL_LOW=(unsigned char)FM_PLL;			      		                 //PLL低字节值
   Tx1_Buffer[1]= PLL_LOW;						                         //I2C第二字节值
   I2C_Write(Tx1_Buffer, Tea5767_WriteAddress1, 5); 					 //写入tea767
}

bool FMradio(void)
{  
  float a;
  unsigned char sb,c;	
 	int xCenter;
 	unsigned char key;
 	unsigned long i,f1,f2,f3,f4;

	if (fm_tuned) goto tuned;

	a,key,i,ch1=0;
	c=1;
	xCenter = LCD_GET_XSIZE() / 2;
	rec_f=0;
	FM_FREQ=105800000;
    PLL_HIGH=0;
    PLL_LOW=0;
    ch=0; 
tuned:
 	I2C_Write(Tx1_Buffer, Tea5767_WriteAddress1, 5);       		//写入TEA5767初始字节
    SetPLL();											   		//写入默认频率
	I2C_ReadByte(Rx1_Buffer,5,Tea5767_ReadAddress1);      //读取TEA5767的状态  

											  
//s  	SST25_R_BLOCK(0,dat,4096);							   		//读出曾经调谐的状态
//		M25P80_buf_ToRam(*dat,0,4096);	
  	ch=dat[200];	
  	if(ch==0||dat[201]!=0x69){FM_FREQ=105800000; ch1=0; ch=0;}	//first run, presets not present  
  	else {				                                        //读出信道和频率
  		ch1=1;
		for(i=0; i<ch; i++){
			f1=dat[i*4];
			f2=dat[i*4+1];
			f3=dat[i*4+2];
			f4=dat[i*4+3];
			fm_ch[i]=f1*0x1000000+f2*0x10000+f3*0x100+f4;		//将保存的有效频率值计算出来
		}
		FM_FREQ=fm_ch[0];
}
}

void FM_prev_channel(void)
{
	if(ch>0){
		num=0;
		if(--ch1==0) ch1=ch;
		FM_FREQ= fm_ch[ch1-1];
		rec_f=0;
		SetPLL();
		c=1;
	}
}

void FM_next_channel(void)
{
	if(ch>0){
		num=0;
		if(++ch1>ch) ch1=1;
		FM_FREQ= fm_ch[ch1-1];
		rec_f=0;
		SetPLL();
		c=1;
	}
}

void FM_scan(void)
{
  unsigned long i=0;
 	unsigned char ch1=0;
				num=0;
				Tx1_Buffer[0] = 0XF0; 
				rec_f=2;												   //搜索模式
				FM_FREQ=87500000;										   //起始频率
				ch=0;
		 		while(1){		    
					fm_pub:;						
					a = FM_FREQ;
					a = a/1000000;
					GUI_SetFont(&GUI_FontD36x48);
					GUI_GotoXY(70,188);
					GUI_SetColor(GUI_YELLOW);
					GUI_DispFloatFix(a, 5, 1);	
					if(FM_FREQ>108000000){FM_FREQ=87500000; break;}  	   //搜索完后回到默认频率上
					SetPLL(); 
					FM_FREQ=FM_FREQ+100000;								   //100K的间隔递增频率
					Delay(0x0dffff);									   //延时一下以便检测TEA5767的频率信息
					I2C_ReadByte(Rx1_Buffer,5,Tea5767_ReadAddress1);	   //获取节目信息					
					if((Rx1_Buffer[0]&0x3f)!=(Tx1_Buffer[0]&0x3f)||(Rx1_Buffer[1]!=Tx1_Buffer[1])||
						(Rx1_Buffer[1]&0x80!=0x80)||Rx1_Buffer[2]<50||Rx1_Buffer[2]>=56||(Rx1_Buffer[3]>>4)<7||
						(Rx1_Buffer[3]>>4)>14);						       //无效频率             		    
				
					else {											
						fm_ch[ch++]= FM_FREQ-100000; 			               	   //有效节目
						GUI_SetFont(&GUI_Font32B_ASCII);
						GUI_SetColor(GUI_BLUE);
						GUI_DispDecAt(ch, 4, 210, 2);								
						goto fm_pub;									   //继续搜索下一个节目
					} 				
				}
				if(ch==0){FM_FREQ=105800000; ch1=0;dat[201]=0xff;}		   //没有搜索到有效节目，使用默认值98.8MHz(此台为西安交通广播）
				else {													   //所搜到有效节目频率，将当前频率改为搜索到的第一个有效节目频率
					ch1=1; 
					FM_FREQ= fm_ch[0];
					dat[201]=0x69;										   //设置保存标志
				}
				dat[200]=ch;											   //保存当前有效频率的数量
				for(i=0; i<ch; i++){									   
					dat[i*4]=(u8)(fm_ch[i]>>24);
					dat[i*4+1]=(u8)(fm_ch[i]>>16);
					dat[i*4+2]=(u8)(fm_ch[i]>>8);
					dat[i*4+3]=(u8)(fm_ch[i]);

				}
//s				SST25_W_BLOCK(0,dat,4096);								   //将搜索到的节目信息保存到SST25VF016B的0页 
//				M25P16_RamTo_buf(*dat, 0, 4096);
				rec_f=0;
				SetPLL();
				c=1;
				fm_tuned=1;
}

/******************* (C) COPYRIGHT 2011 奋斗STM32 *****END OF FILE****/
