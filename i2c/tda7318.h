#include "includes.h"

#define TDA7318_I2C_ADDRESS 0x44
#define TDA_SW1 0x58
#define TDA_SW2 0x59
#define TDA_SW3 0x5A
#define TDA_SW4 0x5B

// TDA7318 audio prototypes
void setAudioVolume(unsigned char volume);
void setAudioBalance(unsigned char balance);
void setAudioBass(unsigned char bass);
void setAudioTreble(unsigned char treble);
void setAudioMute(unsigned char mute);
void setAudioRearLeftVolume(unsigned char rl_volume);
void setAudioRearRightVolume(unsigned char rr_volume);
void setAudioSource(unsigned char source);
