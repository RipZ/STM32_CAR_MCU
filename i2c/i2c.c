#include "stm32f10x.h"

#define SCL_H				 GPIOB->BSRR = GPIO_Pin_8 //GPIO_Pin_8				 
#define SCL_L				 GPIOB->BRR	= GPIO_Pin_8 //GPIO_Pin_8 
	 
#define SDA_H				 GPIOB->BSRR = GPIO_Pin_9 //GPIO_Pin_9
#define SDA_L				 GPIOB->BRR	= GPIO_Pin_9 //GPIO_Pin_9

#define SCL_read			GPIOB->IDR	& GPIO_Pin_8
#define SDA_read			GPIOB->IDR	& GPIO_Pin_9

void I2C_delay(void);
bool I2C_Start(void);
void I2C_Stop(void);
void I2C_Ack(void);
void I2C_NoAck(void);
bool I2C_WaitAck(void);
void I2C_SendByte(u8 SendByte);
bool I2C_Write(uint8_t* pBuffer, uint8_t WriteAddr, uint8_t NumByteToWrite);

extern void Delay(__IO uint32_t nCount);

bool I2C_Write(uint8_t* pBuffer, uint8_t WriteAddr, uint8_t NumByteToWrite)
{
		if(!I2C_Start())return FALSE;
		I2C_SendByte(WriteAddr);
		if(!I2C_WaitAck()){I2C_Stop(); return FALSE;}
		while(NumByteToWrite--)
		{
			I2C_SendByte(* pBuffer);
			I2C_WaitAck();
					pBuffer++;
		}
		I2C_Stop();
		return TRUE;
}

void I2C_delay(void)
{	
	 u8 i=100; 
	 while(i) 
	 { 
		 i--; 
	 } 
}

bool I2C_Start(void)
{
	SDA_H;
	SCL_H;
	I2C_delay();
	if(!SDA_read){ 
		return FALSE;
	}
	SDA_L;
	I2C_delay();
	if(SDA_read) {
		return FALSE;
	}
	SDA_L;
	I2C_delay();
	return TRUE;
}

void I2C_Stop(void)
{
	SCL_L;					
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SDA_H;
	I2C_delay();
}

void I2C_Ack(void)
{	
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}

void I2C_NoAck(void)
{	
	SCL_L;
	I2C_delay();
	SDA_H;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}

bool I2C_WaitAck(void)
{
	SCL_L;
	I2C_delay();
	SDA_H;			
	I2C_delay();
	SCL_H;
	I2C_delay();
	if(SDA_read)
	{
			SCL_L;
			return FALSE;
	}
	SCL_L;
	return TRUE;
}

void I2C_SendByte(u8 SendByte)
{
		u8 i=8;
		while(i--)
		{
				SCL_L;
				I2C_delay();
			if(SendByte&0x80)
				SDA_H;	
			else 
				SDA_L;	 
				SendByte<<=1;
				I2C_delay();
				SCL_H;
				I2C_delay();
		}
		SCL_L;
}

u8 I2C_ReceiveByte(void)
{ 
		u8 i=8;
		u8 ReceiveByte=0;
		SDA_H;				
		while(i--)
		{
			ReceiveByte<<=1;			
			SCL_L;
			I2C_delay();
			SCL_H;
			I2C_delay();	
			if(SDA_read)
			{
				ReceiveByte|=0x01;
			}
		}
		SCL_L;
		return ReceiveByte;
}

bool I2C_ReadByte(u8* pBuffer,	 u8 length,	 u8 DeviceAddress)
{		
		if(!I2C_Start())return FALSE;
		I2C_SendByte(DeviceAddress);
		if(!I2C_WaitAck()){I2C_Stop(); return FALSE;}
			
		while(length--)
		{
			*pBuffer = I2C_ReceiveByte();
		 		if(length == 1)I2C_NoAck();
		 		else I2C_Ack(); 
					pBuffer++;
				
		}
		I2C_Stop(); 	
		return TRUE;
}
