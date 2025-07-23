#include "SPI_u.h"
//ssi3clk pq0 ssi3Fss pq1 SSI3XDAT0 PQ2(master的TX)  SSI3XDAT1  PQ3(master的RX)
//PE5 SSI1XDAT1(MASTER:RX) PE4 SSI1XDAT0(MASTER:TX)  PB5  SSI1Clk      PB4   SSI1Fss
uint16_t SSI_PutData[SSI_PUTLENGTH];

/* The control table used by the uDMA controller.  This table must be aligned
 * to a 1024 byte boundary. */
#if defined(__ICCARM__)
#pragma data_alignment=1024
uint8_t pui8ControlTable[1024];
#elif defined(__TI_ARM__)
#pragma DATA_ALIGN(pui8ControlTable, 1024)
uint8_t pui8ControlTable[1024];
#else
uint8_t pui8ControlTable[1024] __attribute__ ((aligned(1024)));
#endif


void SPI_AD9833_FFS(FFS_ACTION action)
{
	if(action){
//	/* Flush the Receive FIFO    将FIFO清除干净*/
//	uint32_t TrashData;
//	while(MAP_SSIDataGetNonBlocking(SSI3_BASE, &TrashData));
		GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4);//高电平
	}
	else{
		GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0);
	}
}
//SSI :AD9833
void AD9833_SPI_Init(void)
{
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)));
  MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)));
	
	MAP_GPIOPinConfigure(GPIO_PB5_SSI1CLK);
//	MAP_GPIOPinConfigure(GPIO_PB4_SSI1FSS);
	MAP_GPIOPinConfigure(GPIO_PE4_SSI1XDAT0);
	MAP_GPIOPinConfigure(GPIO_PE5_SSI1XDAT1);
	MAP_GPIOPinTypeSSI(GPIO_PORTB_BASE, (GPIO_PIN_5));//|GPIO_PIN_4 软件控制ffs
	MAP_GPIOPinTypeSSI(GPIO_PORTE_BASE, (GPIO_PIN_4|GPIO_PIN_5));
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4);//高电平
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);
  while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_SSI1)));
	
	MAP_SSIConfigSetExpClk(SSI1_BASE, ui32SysClock, SSI_FRF_MOTO_MODE_0,
                           SSI_MODE_MASTER, 100e3, 16);
	MAP_SSIEnable(SSI1_BASE);
//	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4);//将FFS设置为空闲状态
}