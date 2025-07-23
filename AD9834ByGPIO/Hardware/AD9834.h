#ifndef __AD9834_H 
#define __AD9834_H 
#include "ti/devices/msp432e4/driverlib/driverlib.h"
typedef enum {
    SIN_WAVE= 0,   // Output triangle wave
    SQU_WAVE = 1,   // Output sine wave
    TRI_WAVE = 2    // Output square wave
} WaveMode;
/* AD9834¾§ÕñÆµÂÊ75MHz */ 
#define AD9834_SYSTEM_COLCK     75000000UL 
void AD9834_WaveSeting(double Freq,unsigned int Phase,unsigned int Freq_SFR,unsigned int Freq_SPR);
void AD9834_Start(uint8_t Freq_SFR, uint8_t Freq_SPR, uint8_t WaveMode);
void AD9834_Init_GPIO(void);
void AD9834_AmpSet(unsigned char amp);	
void AD9834_Write(unsigned int TxData);
#endif /* AD9834_H */ 

