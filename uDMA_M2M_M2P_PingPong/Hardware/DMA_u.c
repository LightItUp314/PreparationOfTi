#include "DMA_u.h"
#define DMASWREQ_OFFSET 0x14
//DMA相关函数放在udma.c/.h文件中


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

/***********memory to memory***********/
// 缓冲区定义（256个32位字）
uint32_t srcBuffer1[256];
uint32_t dstBuffer1[256];
//*****************************************************************************
//
// The interrupt handler for uDMA interrupts from the memory channel.  This
// interrupt will increment a counter, and then restart another memory
// transfer.
//
//*****************************************************************************
void
UDMA_IRQHandler(void)
{
    uint32_t ui32Mode;

    //
    // Check for the primary control structure to indicate complete.
    //
    ui32Mode = MAP_uDMAChannelModeGet(UDMA_CH30_SW);
    if(ui32Mode == UDMA_MODE_STOP)
    {
        /*
				M2M传输完成后的逻辑实现
				*/
				//若需再次是用需要重新设置uDMAChannelTransferSet和重新使能与发起请求
//			//
//			// Configure it for another transfer.
//			//
//			MAP_uDMAChannelTransferSet(UDMA_CHANNEL_SW, UDMA_MODE_AUTO,
//																	 g_ui32SrcBuf, g_ui32DstBuf,
//																	 MEM_BUFFER_SIZE);

//			//
//			// Initiate another transfer.
//			//
//			MAP_uDMAChannelEnable(UDMA_CHANNEL_SW);
//			MAP_uDMAChannelRequest(UDMA_CHANNEL_SW);
	}

}

void DMA_M2M_Init(void)
{
	//Module Init 不管何种模式dma都需要的Init的部分
	
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
	
	//M2M
	//channel 30是专门用来M2M的（其他channel都可以，只有channel30只可以M2M）
	
	//Cfg channel attributes: Priority    primary channel/alternate control structrue    单次与burst模式的使用    清除标志位(为中断做准备)
	//首先将UDMA_CH30_SW映射至channel 30  Assigns a peripheral mapping for a uDMA channel.
	MAP_uDMAChannelAssign(UDMA_CH30_SW);
	//设置attributes  虽然是disable但是UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |UDMA_ATTR_HIGH_PRIORITY |UDMA_ATTR_REQMASK只用一位就可以表示，禁用是另一种形式的启用
	/* Put the attributes in a known state for the uDMA ADC0 Sequencer 2
     * channel. These should already be disabled by default. */
	MAP_uDMAChannelAttributeDisable(UDMA_CH30_SW,                                     
																	UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
																	UDMA_ATTR_HIGH_PRIORITY |
																	UDMA_ATTR_REQMASK);			
	//UDMA_ATTR_REQMASK：默认启用，Disable屏蔽中断,相当于使能了dma的中断。
/*Clear the useburst bit   			
Clear the alternate control select bit				
Clear the high priority bit for this channel		  			
Clear the request mask bit for this channel	 				
*/
//现在已经处于known状态   若要调整则用MAP_uDMAChannelAttributeEnable

// 1. 设置控制参数：32位数据，地址递增，仲裁大小=8（ARBSIZE=3）
	MAP_uDMAChannelControlSet(
			UDMA_CH30_SW | UDMA_PRI_SELECT,  // 通道30 + 主控制结构
			UDMA_SIZE_32 | UDMA_SRC_INC_32 | UDMA_DST_INC_32 | UDMA_ARB_8
	);

	// 2. 设置传输参数：自动请求模式，传输256项
	MAP_uDMAChannelTransferSet(
			UDMA_CH30_SW | UDMA_PRI_SELECT,  // 通道30 + 主控制结构
			UDMA_MODE_AUTO,                  // 自动请求模式（对应XFERMODE=2）
			(void *)srcBuffer1,                // 源地址
			(void *)dstBuffer1,                // 目标地址
			256                               // 传输项数（32位数据）
	);

	//使用中断  全局中断使能默认在main中使用
	MAP_IntEnable(INT_UDMA);
	////UDMA_ATTR_REQMASK：默认启用，Disable屏蔽中断,相当于使能了dma的中断。

	// Now the software channel is primed to start a transfer.  The channel
	// must be enabled.  For software based transfers, a request must be
	// issued.  After this, the uDMA memory transfer begins.
	MAP_uDMAChannelEnable(UDMA_CH30_SW);
	MAP_uDMAChannelRequest(UDMA_CH30_SW);
}
	




/***********Configuring a Peripheral for Simple Transmit***********/
// 缓冲区定义（64个8位字）
uint8_t srcBuffer2[64];
//This example configures the μDMA controller to transmit a buffer of data to a peripheral.use channel7 for instance
void DMA_M2P_Init(void) 
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
	
	//指定channel
	MAP_uDMAChannelAssign(UDMA_CH7_UART5TX);
	//then cig the atrributes
	MAP_uDMAChannelAttributeDisable(UDMA_CH7_UART5TX,                                     
																	UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
																	UDMA_ATTR_HIGH_PRIORITY |
																	UDMA_ATTR_REQMASK);
	//Attribute in a known situation
	//UDMA_ATTR_REQMASK：默认启用，Disable屏蔽中断,相当于使能了dma的中断。
	//隐式中断触发：当 DMA 传输完成（Primary/Alternate 缓冲区）时，硬件会自动触发中断，无需额外使能通道中断。

	
	//with interrupt //todo
	
	
	MAP_uDMAChannelControlSet(UDMA_CH7_UART5TX|UDMA_PRI_SELECT,
														UDMA_SIZE_8 | UDMA_SRC_INC_8 | UDMA_DST_INC_NONE | UDMA_ARB_4);
  MAP_uDMAChannelTransferSet(	UDMA_CH7_UART5TX | UDMA_PRI_SELECT,  // 通道30 + 主控制结构
			UDMA_MODE_BASIC,                  // 自动请求模式（对应XFERMODE=2）
			(void *)srcBuffer2,                // 源地址
			(void *)(UART5_BASE+0x00),                // 目标地址  UART5_BASE 或者用 (void *)(&UART1->DR)
			64                               // 传输项数（8位数据）
	);							
	
	MAP_uDMAChannelEnable(UDMA_CH7_UART5TX);
	//硬件方面也需要使能时钟，和enable相关硬件 此处省略
			/*
			MAP_UARTConfigSetExpClk(UART1_BASE, g_ui32SysClock, 115200,
                            UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                            UART_CONFIG_PAR_NONE);		
			//使用dma传输完成中断  irq函数是UART5_IRQHandler   dma相关的中断和uart自己的中断公用trq
			MAP_UARTIntEnable(UART5_BASE, UART_INT_DMATX);
			MAP_IntEnable(INT_UART1);
			//全局中断使能默认在main中执行
			
			//使用dma
			MAP_UARTDMAEnable(UART5_BASE, UART_DMA_RX | UART_DMA_TX);
			
			*/
}




//详情见***\Ti\simplelink_msp432e4_sdk_4_20_00_12\examples\nortos\MSP_EXP432E401Y\driverlib\udma_demo\keil
/***********Configuring a Peripheral for Ping-Pong Service***********/
uint8_t srcBuffer3_PRI[64];
uint8_t srcBuffer3_ALT[64];
void DMA_PingPong_Init()
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
	
	MAP_uDMAChannelAssign(UDMA_CH8_UART0RX);
	
	MAP_uDMAChannelAttributeDisable(UDMA_CH8_UART0RX,
																	UDMA_ATTR_USEBURST|
																	UDMA_ATTR_ALTSELECT|
																	UDMA_ATTR_HIGH_PRIORITY|
																	UDMA_ATTR_REQMASK);
//UDMA_ATTR_REQMASK：默认启用，Disable屏蔽中断,相当于使能了dma的中断。
//隐式中断触发：当 DMA 传输完成（Primary/Alternate 缓冲区）时，硬件会自动触发中断，无需额外使能通道中断。
	MAP_uDMAChannelControlSet(UDMA_CH8_UART0RX|UDMA_PRI_SELECT,
															UDMA_SIZE_8 | UDMA_SRC_INC_NONE | UDMA_DST_INC_8 | UDMA_ARB_8);
	MAP_uDMAChannelControlSet(UDMA_CH8_UART0RX|UDMA_ALT_SELECT,
															UDMA_SIZE_8 | UDMA_SRC_INC_NONE | UDMA_DST_INC_8 | UDMA_ARB_8);
	MAP_uDMAChannelTransferSet(UDMA_CH8_UART0RX|UDMA_PRI_SELECT,
														UDMA_MODE_PINGPONG,                  // 自动请求模式（对应XFERMODE=2）
														(void *)(UART0_BASE+0x00),                // 源地址
														(void *)srcBuffer3_PRI,                // 目标地址  UART5_BASE
														64                               // 传输项数（8位数据）
													);	
	MAP_uDMAChannelTransferSet(UDMA_CH8_UART0RX|UDMA_PRI_SELECT,
														UDMA_MODE_PINGPONG,                  // 自动请求模式（对应XFERMODE=2）
														(void *)(UART0_BASE+0x00),                // 源地址
														(void *)srcBuffer3_ALT,                // 目标地址  UART5_BASE
														64                               // 传输项数（8位数据）
													);	
	
	//配置中断源
	//UART配置省略  包括但不限于ARTDMAEnable(UART0_BASE,UART_DMA_RX);
	
	// 在NVIC中启用uDMA全局中断
    IntEnable(INT_UDMA);//Enables an interrupt.
    IntPrioritySet(INT_UDMA, 0);  // 可选：设置优先级
    IntMasterEnable();            // 全局中断使能

    // 8. 启动DMA传输
    uDMAChannelEnable(UDMA_CH8_UART0RX);
}



/*
总结：先初始化dma模板，然后指定设置的channel与硬件，之后进行channel的control，transfer的set
有需要还要使能中断(dma中断在disable UDMA_ATTR_REQMASK中使能、dma依靠的外设的中断使能和nvic方面的中断使能)，如对应硬件（如uart）的dma使能（这个不需要NVIC），
要用ISR处理dma中断的话需要NVIC使能INT_UDMA或者硬件的相关中断（与硬件相关的dma传输完成中断依靠硬件的），并编写ISR，最后开启全局中断。最后使能channel


注意dma的UDMA_ARB_n表示一次dma请求会转移n个item 如果传输的数据实时性要求高，（考虑关闭FIFO）请设置UDMA_ARB_1 意思是，每一个dma请求只传输一个item，然后仲裁。
一般这样就是硬件发出一次请求你就传输一次。  一半不要让高等级的dma传输的UDMA_ARB_n设置过高否则可能dma一直传输高仲裁优先级的dma，低等级的无法传输

*/