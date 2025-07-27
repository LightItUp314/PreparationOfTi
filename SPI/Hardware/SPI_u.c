#include "SPI_u.h"
#include "hamming.h"
//ssi3clk pq0 ssi3Fss pq1 SSI3XDAT0 PQ2(master的TX)  SSI3XDAT1  PQ3(master的RX)

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

/*
1表示高电平是闲置或停止
0表示开始和传输中
*/
void SPI_W25Q46_CS(CS_ACTION action)
{
	if(action){
//	/* Flush the Receive FIFO    将FIFO清除干净*/
//	uint32_t TrashData;
//	while(MAP_SSIDataGetNonBlocking(SSI3_BASE, &TrashData));
		GPIOPinWrite(GPIO_PORTQ_BASE, GPIO_PIN_1, GPIO_PIN_1);//高电平
	}
	else{
		GPIOPinWrite(GPIO_PORTQ_BASE, GPIO_PIN_1, 0);
	}
}
//例程见**\Ti\simplelink_msp432e4_sdk_4_20_00_12\examples\nortos\MSP_EXP432E401Y\driverlib\spi_master_legacy_interrupt\keil
void SPI_W25Q46_Init(void)
{	
	//GPIO控制cs信号
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOQ));
	GPIOPinTypeGPIOOutput(GPIO_PORTQ_BASE, GPIO_PIN_1);
	SPI_W25Q46_CS(CS_STOP);//默认停止通信
	
	//SII
	MAP_GPIOPinConfigure(GPIO_PQ0_SSI3CLK);
//	MAP_GPIOPinConfigure(GPIO_PQ1_SSI3FSS); //使用软件控件FSS
	MAP_GPIOPinConfigure(GPIO_PQ2_SSI3XDAT0);
	MAP_GPIOPinConfigure(GPIO_PQ3_SSI3XDAT1);
	MAP_GPIOPinTypeSSI(GPIO_PORTQ_BASE, (GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3));
	
	/* Enable the clock to SSI-0 module and configure the SSI Master */
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);
	while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_SSI3)))
	{
	}

	MAP_SSIConfigSetExpClk(SSI3_BASE, ui32SysClock, SSI_FRF_MOTO_MODE_0,
												 SSI_MODE_MASTER, (ui32SysClock/120), 8);
	//对于SSIDataPut,The upper 32 - N bits of \e ui32Data are discarded by the hardware,此处N为8
//少量数据传输，且实时性要求不高，不使用中断
//	MAP_SSIIntEnable(SSI3_BASE, SSI_TXEOT);//Transmit FIFO is empty
	MAP_SSIEnable(SSI3_BASE);
	/* Flush the Receive FIFO    将FIFO清除干净*/
	uint32_t TrashData;
	while(MAP_SSIDataGetNonBlocking(SSI3_BASE, &TrashData));
	//好似不能禁用FIFO
	/* Enable the interrupt generation from SSI-0 */
	//  MAP_IntEnable(INT_SSI3);
}
uint8_t SPI_W25Q46_SwapByte(uint8_t ByteSend)
{
	//! This function allows the caller to determine whether all transmitted bytes
	//! have cleared the transmitter hardware.  If \b false is returned, then the
	//! transmit FIFO is empty and all bits of the last transmitted word have left
	//! the hardware shift register.
	//等待fifo空并且移位寄存器完成移位
	while(SSIBusy(SSI3_BASE));
	SSIDataPut(SSI3_BASE,ByteSend);
//	uint32_t temp;
//	SSIDataGet(SSI3_BASE,&temp);//由于第二个形参是	uint32_t* 直接传入uint8_t *类型数据会覆盖相邻的内存空间，导致未定义行为
//	*ByteReceive=(uint8_t)temp;
//	//If there is no data available, this function waits
//	// until data is received before returning.
	
//	while(!(SSI3->SR & SSI_SR_RNE));
//  return (uint8_t)&SSI3->DR;//HWREG(ui32Base + SSI_O_DR);
	uint32_t temp;
	SSIDataGet(SSI3_BASE,&temp);//由于第二个形参是	uint32_t* 直接传入uint8_t *类型数据会覆盖相邻的内存空间，导致未定义行为
	return (uint8_t)temp;
}
void SSI_DMA_Init()
{
	//1.Enable the μDMA clock using the RCGCDMA register
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
	// 等待时钟稳定
  while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA));
		//2. Enable the μDMA controller by setting the MASTEREN bit of the DMA Configuration (DMACFG)register
	MAP_uDMAEnable();
		/*3. Program the location of the channel control table by writing the base address of the table to the DMA
Channel Control Base Pointer (DMACTLBASE) register. The base address must be aligned on a 1024-
byte boundary.*/
	MAP_uDMAControlBaseSet(pui8ControlTable);
	
	MAP_uDMAChannelAssign(UDMA_CH11_SSI1TX);
	//then cig the atrributes
	MAP_uDMAChannelAttributeDisable(UDMA_CH11_SSI1TX,                                     
																	UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
																	UDMA_ATTR_HIGH_PRIORITY |
																	UDMA_ATTR_REQMASK);
	
	MAP_uDMAChannelControlSet(UDMA_CH11_SSI1TX|UDMA_PRI_SELECT,
														UDMA_SIZE_16 | UDMA_SRC_INC_16 | UDMA_DST_INC_NONE | UDMA_ARB_4);
  MAP_uDMAChannelTransferSet(	UDMA_CH11_SSI1TX | UDMA_PRI_SELECT,  // 通道30 + 主控制结构
			UDMA_MODE_AUTO,                  // 自动请求模式（对应XFERMODE=2）
			(void *)hamming_window,                // 源地址 SSI_PutData
			(void *)(&SSI1->DR),                // 目标地址  UART5_BASE 或者用 (void *)(&UART1->DR)
			64                               // 传输项数（8位数据）
	);	
	//MAP_uDMAChannelEnable(UDMA_CH11_SSI0TX);
	//需要dma时再使用
}
void SSI1_Init(void)
{
	/* Enable clocks to GPIO Port A and configure pins as SSI */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)))
    {
    }
		MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)))
    {
    }
    MAP_GPIOPinConfigure(GPIO_PB5_SSI1CLK);
    MAP_GPIOPinConfigure(GPIO_PB4_SSI1FSS);
    MAP_GPIOPinConfigure(GPIO_PE4_SSI1XDAT0);
    MAP_GPIOPinConfigure(GPIO_PE5_SSI1XDAT1);
    MAP_GPIOPinTypeSSI(GPIO_PORTB_BASE, (
                                         GPIO_PIN_4 | GPIO_PIN_5));
		MAP_GPIOPinTypeSSI(GPIO_PORTE_BASE, (
                                         GPIO_PIN_4 | GPIO_PIN_5));
    /* Enable the clock to SSI-0 module and configure the SSI Master */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_SSI1)))
    {
    }

    MAP_SSIConfigSetExpClk(SSI1_BASE, ui32SysClock, SSI_FRF_MOTO_MODE_0,
                           SSI_MODE_MASTER, (ui32SysClock/24), 16);
    MAP_SSIIntEnable(SSI1_BASE, SSI_TXEOT|SSI_DMATX);// Transmit FIFO is empty
    //使能TX dma
		SSI_DMA_Init();
		
		
		
		MAP_SSIEnable(SSI1_BASE);
		uint32_t trash;
    /* Flush the Receive FIFO    将FIFO清除干净*/
    while(MAP_SSIDataGetNonBlocking(SSI1_BASE, &trash));
		
    /* Enable the interrupt generation from SSI-0 */
    MAP_IntEnable(INT_SSI1);
		//MAP_SSIDMAEnable(SSI1_BASE,SSI_DMA_TX);
}
void SSI1_IRQHandler(void)
{
		uint32_t getIntStatus;
		getIntStatus = MAP_SSIIntStatus(SSI1_BASE, true);
		if((getIntStatus&SSI_TXEOT)!=false)
		{
			MAP_SSIIntClear(SSI1_BASE,getIntStatus);
			/*
			
			
			*/
		}
		if((getIntStatus&SSI_DMATX)!=false)
		{
			MAP_SSIDMADisable(SSI1_BASE,SSI_DMA_TX);
			//			MAP_uDMAChannelDisable(UDMA_CH11_SSI1TX);  可以不需要
			MAP_SSIIntClear(SSI1_BASE,getIntStatus);
			uDMAChannelEnable(UDMA_CH11_SSI1TX);
			
			/*
			//disable the dma transimit
			//需要先关闭ssi硬件方面的dma传输使能  MAP_SSIDMADisable(SSI1_BASE,SSI_DMA_TX);
			SSIIntClear(SSI1_BASE,SSI_DMATX);
			*/
		}
}