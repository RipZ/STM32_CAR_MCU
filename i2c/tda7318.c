#include "includes.h"
#include <string.h>		
#include "..\usart\SPI_Flash.h"

unsigned char volMap[] = {0x3F,0x3D,0x3B,0x39,0x37,0x35,0x33,0x31,   
                  0x2F,0x2D,0x2B,0x29,0x27,0x25,0x23,0x21,   
                  0x1F,0x1D,0x1B,0x19,0x17,0x15,0x13,0x11,   
                  0x0F,0x0D,0x0B,0x09,0x07,0x05,0x03,0x00};
                          
unsigned char lfAttMap[] = {0x9F,0x9D,0x9B,0x99,0x97,0x95,0x93,0x91,
                   0x8F,0x8D,0x8B,0x89,0x87,0x85,0x83,0x80};                          

unsigned char rfAttMap[] = {0xBF,0xBD,0xBB,0xB9,0xB7,0xB5,0xB3,0xB1,
                   0xAF,0xAD,0xAB,0xA9,0xA7,0xA5,0xA3,0xA0}; 

unsigned char lrAttMap[] = {0xDF,0xDD,0xDB,0xD9,0xD7,0xD5,0xD3,0xD1,
                   0xCF,0xCD,0xCB,0xC9,0xC7,0xC5,0xC3,0xC0}; 

unsigned char rrAttMap[] = {0xFF,0xFD,0xFB,0xF9,0xF7,0xF5,0xF3,0xF1,
                   0xEF,0xED,0xEB,0xE9,0xE7,0xE5,0xE3,0xE0}; 
                            
unsigned char bassMap[] = {0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x6F,
                  0x6E,0x6D,0x6C,0x6B,0x6A,0x69,0x68}; 

unsigned char trebleMap[] = {0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x7F,
                    0x7E,0x7D,0x7C,0x7B,0x7A,0x79,0x78};

extern bool I2C_Write(uint8_t* pBuffer, uint8_t WriteAddr, uint8_t NumByteToWrite);
/*
void initTDA7318() {
  sendAudioMute(1);
  sendAudioOn(1);
  delay(3000);
  sendAmpOn(1);
  sendAudioVolume(16);
  sendAudioLFAttenuator(15);
  sendAudioRFAttenuator(15);
  sendAudioLRAttenuator(15);
  sendAudioRRAttenuator(15);
  sendAudioSwitch(3);  
  sendAudioBass(7);
  sendAudioTreble(7);
  sendAudioMute(0);
}
*/
void setAudioVolume(unsigned char volume)
{
}

void setAudioBalance(unsigned char balance)
{
}

void setAudioBass(unsigned char bass)
{
}

void setAudioTreble(unsigned char treble)
{
}

void setAudioMute(unsigned char mute)
{
}

void setAudioRearLeftVolume(unsigned char rl_volume)
{
}

void setAudioRearRightVolume(unsigned char rr_volume)
{
}

void setAudioSource(unsigned char source)
{
}