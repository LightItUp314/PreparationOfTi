#ifndef SPI_U_H
#define SPI_U_H

#include <ti/devices/msp432e4/driverlib/driverlib.h>
/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#define SSI_PUTLENGTH 64
typedef enum{
	FFS_START,
	FFS_STOP
}FFS_ACTION;
extern uint32_t ui32SysClock;
void SPI_AD9833_FFS(FFS_ACTION action);
void AD9833_SPI_Init(void);
#endif