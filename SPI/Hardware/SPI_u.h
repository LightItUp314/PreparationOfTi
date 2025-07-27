#ifndef SPI_U_H
#define SPI_U_H

#include <ti/devices/msp432e4/driverlib/driverlib.h>
/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#define SSI_PUTLENGTH 64
typedef enum{
	CS_START,
	CS_STOP
}CS_ACTION;
extern uint32_t ui32SysClock;
void SPI_W25Q46_CS(CS_ACTION action);
void SPI_W25Q46_Init(void);
uint8_t SPI_W25Q46_SwapByte(uint8_t ByteSend);
void SSI1_Init(void);
	

extern uint16_t SSI_PutData[SSI_PUTLENGTH];
#endif