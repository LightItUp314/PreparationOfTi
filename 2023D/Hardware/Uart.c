#include "Uart.h"
#include "ADC_u.h"
#include "arm_math.h"
#include <stdlib.h> // 包含 atof 函数

extern uint32_t ui32SysClock;//单片机主频率
extern bool basic_flag;
bool getnum_all=false;
uint16_t AM_baoluo[1024];

uint8_t * p=(uint8_t *)AM_baoluo;

uint8_t u_buf[256];//printf_u的缓存区
//以UART0为例子 PA0为RX PA1为TX 配置UART 115200 bps 8-N-1 format ONLY TO SEND
//使用时将UART0换为UARTn ,PA0换为Pyx,PA1换为Pyx

#define UDMA_CHANNEL_UART0RX    8
//可以使用(UART0->DR)
///* The control table used by the uDMA controller.  This table must be aligned
// * to a 1024 byte boundary. */
//#if defined(__ICCARM__)
//#pragma data_alignment=1024
//uint8_t pui8ControlTable[1024];
//#elif defined(__TI_ARM__)
//#pragma DATA_ALIGN(pui8ControlTable, 1024)
//uint8_t pui8ControlTable[1024];
//#else
//uint8_t pui8ControlTable[1024] __attribute__ ((aligned(1024)));
//#endif
extern uint8_t pui8ControlTable[1024];

/*************ATTENTON PLS!!!!!!THIS "UDMA_IRQHandler" ISR IS USED FOR for uDMA interrupts from the memory channel LOL***********/
//IntEnable(INT_UDMA);//Enable interrupts from the uDMA software channel.不是用来硬件传输至memory的

//基础函数 const修饰表示在该函数中pui8Buffer为不可修改，但不代表传入的pui8Buffer一定是不可修改的数组
//const修饰符只对函数内部的变量有约束作用，它保证了在该函数内部不会修改数据。
//其他函数仍然可以自由地访问和修改buffer的内容，只要这些修改不在UARTSend函数的调用期间发生。
void UARTSend(uint32_t ui32Base, const uint8_t *pui8Buffer, uint32_t ui32Count)
{
		while(ui32Count--)
		{
			/*

			单片机波特率 = 115200bps（约 11.5KB/s 理论速度）
			发送 FIFO 深度 = 16 字节
			你连续调用 UARTCharPut 发送 1024 字节
			单片机快速填充发送 FIFO（例如在 1μs 内写入 16 字节）。
			硬件开始以 115200bps 的速度串行输出（约 87μs/字节）。
			FIFO 满时：
			当 FIFO 被填满后，UARTCharPut 会 阻塞等待。
			只有等 FIFO 中至少空出 1 字节时（即硬件已发出 1 字节），才能继续写入。
			*/
			MAP_UARTCharPut(ui32Base, *pui8Buffer++);//put推出，即发送 
			
			//MAP_UARTCharPutNonBlocking(ui32Base, *pui8Buffer++);
			/*
			立即写入或丢弃：

			如果 FIFO 未满，将数据写入 FIFO，并返回成功。
			如果 FIFO 已满，直接丢弃数据，返回失败（不等待）
			会丢数据 damn
			*/
		}
}

void FPGA_SendType(uint8_t type)
{
	uint8_t i;
	for(i=0;i<20;i++)
	{
		MAP_UARTCharPut(UART0_BASE,type);
	}
}
//This interrupt will occur when a DMA
// transfer is complete using the UART1 uDMA channel. 其他UART的中断也是由这个ISR处理
void UART0_IRQHandler(void) {
		uint32_t ui32Status;
    uint32_t ui32Mode;

    //
    // Read the interrupt status of the UART.
    //
    ui32Status = MAP_UARTIntStatus(UART0_BASE, 1);
		// Clear any pending status
		MAP_UARTIntClear(UART0_BASE, ui32Status);
		//Check the DMA control table to see if transfer is
    //complete.
		ui32Mode = MAP_uDMAChannelModeGet(UDMA_CHANNEL_UART0RX | UDMA_PRI_SELECT);
		
	if(ui32Mode == UDMA_MODE_STOP)
    {
			getnum_all=true;
			/*进行中断完成后的逻辑处理*/
			//若还需要DMA传输需重新设置uDMAChannelTransferSet和再次使能channel pingpong模式不一样，具体见
			//***\Ti\simplelink_msp432e4_sdk_4_20_00_12\examples\nortos\MSP_EXP432E401Y\driverlib\udma_demo\keil
			/*
			MAP_uDMAChannelTransferSet(UDMA_CHANNEL_UART1RX | UDMA_PRI_SELECT,
                                   UDMA_MODE_PINGPONG,
                                   (void *)(&UART1->DR),
                                   g_ui8RxBufA, sizeof(g_ui8RxBufA));
			uDMAChannelEnable(UDMA_CHn_xxxxx);
			*/
//			uDMAChannelDisable(UDMA_CH8_UART0RX);
			MAP_UARTDMADisable(UART0_BASE, UART_DMA_RX);
		}
}
void DMA_UART_RX_Init(void)
{
	// 启用DMA控制器时钟
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
	
	// 启用DMA控制器
	MAP_uDMAEnable();
	
	// 设置DMA控制表
	MAP_uDMAControlBaseSet(pui8ControlTable);
	
	MAP_uDMAChannelAssign(UDMA_CH8_UART0RX);
	
	MAP_uDMAChannelAttributeDisable(UDMA_CH8_UART0RX,                                     
																	UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
																	UDMA_ATTR_HIGH_PRIORITY |
																	UDMA_ATTR_REQMASK);			
	
	MAP_uDMAChannelControlSet(
		UDMA_CH8_UART0RX | UDMA_PRI_SELECT,  // 通道30 + 主控制结构
		UDMA_SIZE_8 | UDMA_SRC_INC_NONE | UDMA_DST_INC_8 | UDMA_ARB_1//一次传输一个数据之后仲裁
);
	
	
	MAP_uDMAChannelTransferSet(
			UDMA_CH8_UART0RX | UDMA_PRI_SELECT,  
			UDMA_MODE_BASIC,                                  //UDMA_MODE_AUTO
			(void *)(UART0->DR),                // 源地址
			AM_baoluo,                // 目标地址
			1024                               
			);
	//UDMA_MODE_AUTO 是一种DMA传输模式，表示DMA控制器会在每次传输完成后自动重新启动传输，直到被显式停止。这种模式通常用于连续数据流的传输，例如从串口接收数据到内存
	//uDMAChannelEnable(UDMA_CH8_UART0RX);//在main中执行
			
	//若使用dma中断  由于disabled UDMA_ATTR_REQMASK 相当于udma方面的中断已经使能了 只关注nvic方面的中断使能即可
	
//	IntPrioritySet(INT_UDMA, 0);  // 可选：设置优先级		
	uDMAChannelEnable(UDMA_CH8_UART0RX);
}

void UART_Init(void)
{
	
	/* Enable the clock to GPIO port A and UART 0 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	
	//GPIO方面
	/* Configure the GPIO Port A for UART 0 */
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	
	//UART方面
	/* Configure the UART for 115200 bps 8-N-1 format */
    MAP_UARTConfigSetExpClk(UART0_BASE, ui32SysClock, 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
	//以UART0为例子 PA0为RX PA1为TX 配置UART 115200 bps 8-N-1 format 目前ONLY TO SEND
	/*该函数中有使能UART
		//
    // Start the UART.
    //
    UARTEnable(ui32Base);*/
	/***********************************************************************************/
	    
		MAP_UARTFIFODisable(UART0_BASE);//不使用FIFO！！实时性要求高
		MAP_IntEnable(INT_UART0);
		//调用这个函数后，NVIC会允许UART0中断被处理
		//
		// Enable the UART DMA TX/RX interrupts. 
		//
		MAP_UARTIntEnable(UART0_BASE, UART_INT_DMARX);//UART_INT_DMARX 函数的说明里面没有但是，.h文件里有这个宏定义 T^T.....
		//当然也要有总的开启"可屏蔽"中断的	MAP_IntMasterEnable  常在main中while前执行
		MAP_UARTDMAEnable(UART0_BASE, UART_DMA_RX);
		//使用dma接收uart_rx_data
		DMA_UART_RX_Init();
		//在main中执行
}





//使用宏，更加便捷，适用于单个工程要使用多个UART发送数据，见UART.h

//sprintf 函数的返回值是格式化字符串的长度（不包括终止符 \0），类型为 int。在大多数情况下，int 类型的返回值可以安全地赋值给 size_t 类型的变量，因为 size_t 通常是一个无符号整数类型，足够大以表示字符串的长度。

//以UART0的接收为例子

bool UART_RECEIVE_ERROR=false;
int uart_index = -HEAD_TAIL_LENTH;
UART_ReceiveState UART0_ReceiveState=UART_RECEIVE_IDLE;
uint8_t receivedByte;
uint8_t UART0_ReceiveBuf[ReceiveBufLenth];
uint16_t index_p=0;

extern float arr[5];
typedef enum {
    UART7_RECEIVE_IDLE,          
		UART7_RECEIVE_DATA,
} UART7_ReceiveState;
UART7_ReceiveState U7State=UART7_RECEIVE_IDLE;
uint8_t ReceiveData_UART7[BUF_LEN];
uint8_t index_U7;
void ProcessData(uint8_t* buf)
{
	
	if(buf[0]=='0'){
		uint8_t index=10*(buf[1]-'0')+ (buf[2] - '0');	
		parameter_b[index] = atof((char *)&buf[3]);
	}
	else if(buf[0]=='1'){
		uint8_t index=10*(buf[1]-'0')+ (buf[2] - '0');	
		parameter_s[index] = atof((char *)&buf[3]);
	}else if(buf[0]=='2'){
		if(buf[0]-'0'==0)
		{
			basic_flag=false;
		}else{
			basic_flag=true;
		}
	}
}

void UART7_IRQHandler(void)
{
	uint32_t ui32Status;

    //
    // Get the interrrupt status.
    //
    ui32Status = MAP_UARTIntStatus(UART7_BASE, true);

    //
    // Clear the asserted interrupts.
    //
    MAP_UARTIntClear(UART7_BASE, ui32Status);

    //
    // Loop while there are characters in the receive FIFO.
    //
    while(MAP_UARTCharsAvail(UART7_BASE))
    {
			receivedByte = MAP_UARTCharGetNonBlocking(UART7_BASE);
				switch(U7State)
				{
					 
					case  UART7_RECEIVE_IDLE:
						if (receivedByte==HEAD)
						{
							U7State=UART7_RECEIVE_DATA;
						}
					break;
					case UART7_RECEIVE_DATA:
						if(receivedByte!=TAIL){
							ReceiveData_UART7[index_U7++]=receivedByte;
						}
						else{
							index_U7=0;
							
							U7State=UART7_RECEIVE_IDLE;
							ProcessData(ReceiveData_UART7);
						}
						break;
				}
    }
	
}
//_UART7
//PC5:TX   RC4:RX
void VOFA_init(void)
{
		

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);
	
	//GPIO方面
	
    MAP_GPIOPinConfigure(GPIO_PC5_U7TX);
    MAP_GPIOPinConfigure(GPIO_PC4_U7RX);
    MAP_GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);
	
	//UART方面
	/* Configure the UART for 115200 bps 8-N-1 format */
    MAP_UARTConfigSetExpClk(UART7_BASE, ui32SysClock, 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
//		MAP_UARTFIFODisable(UART7_BASE);//不使用FIFO！！实时性要求高
		MAP_IntEnable(INT_UART7);
		//调用这个函数后，NVIC会允许UART中断被处理
		//
		// Enable the UART DMA TX/RX interrupts. 
		//
		MAP_UARTIntEnable(UART7_BASE, UART_INT_RT|UART_INT_RX);
		//当然也要有总的开启"可屏蔽"中断的	MAP_IntMasterEnable  常在main中while前执行
		//在main中执行
}
#define VOFA_SendFloatEND(ui32Base) do { \
    MAP_UARTCharPut(ui32Base, 0x00); \
    MAP_UARTCharPut(ui32Base, 0x00); \
    MAP_UARTCharPut(ui32Base, 0x80); \
    MAP_UARTCharPut(ui32Base, 0x7F); \
} while (0)
void VOFA_SendFloat(uint32_t ui32Base,const float * pfbuffer,uint32_t ui32Count)
{
	while(ui32Count--)
	{
		MAP_UARTCharPut(ui32Base, *(unsigned char *)pfbuffer);//put推出，即发送 
		MAP_UARTCharPut(ui32Base, *(((unsigned char *)pfbuffer)+1));
		MAP_UARTCharPut(ui32Base, *(((unsigned char *)pfbuffer)+2));
		MAP_UARTCharPut(ui32Base, *(((unsigned char *)pfbuffer)+3));
		pfbuffer++;
		VOFA_SendFloatEND(ui32Base);
	}
}
void VOFA_SendADC_Buf(int16_t* adc_buff)
{
	uint16_t i;
	float32_t adc_tmap;
	for(i=0;i<FFT_LEN;i++)
	{
		adc_tmap=(float32_t)adc_buff[i]*3.3/4096;
		MAP_UARTCharPut(UART7_BASE, *(unsigned char *)&adc_tmap);//put推出，即发送 
		MAP_UARTCharPut(UART7_BASE, *(((unsigned char *)&adc_tmap)+1));
		MAP_UARTCharPut(UART7_BASE, *(((unsigned char *)&adc_tmap)+2));
		MAP_UARTCharPut(UART7_BASE, *(((unsigned char *)&adc_tmap)+3));
		
		VOFA_SendFloatEND(UART7_BASE);
	}
}
void VOFA_SendFFTMag(float32_t* FFT_Mag)
{
	uint16_t i;
	
	for(i=0;i<FFT_LEN;i++)
	{
		MAP_UARTCharPut(UART7_BASE, *(unsigned char *)&FFT_Mag[i]);//put推出，即发送 
		MAP_UARTCharPut(UART7_BASE, *(((unsigned char *)&FFT_Mag[i])+1));
		MAP_UARTCharPut(UART7_BASE, *(((unsigned char *)&FFT_Mag[i])+2));
		MAP_UARTCharPut(UART7_BASE, *(((unsigned char *)&FFT_Mag[i])+3));
		VOFA_SendFloatEND(UART7_BASE);
	}
	
}

void VOFA_SendData(int16_t* adc_buff,float32_t* FFT_Mag)
{
	uint16_t i;
	float32_t adc_tmap;
	for(i=0;i<FFT_LEN;i++)
	{
		adc_tmap=(float32_t)adc_buff[i]*3.3/4096;
		MAP_UARTCharPut(UART7_BASE, *(unsigned char *)&adc_tmap);//put推出，即发送 
		MAP_UARTCharPut(UART7_BASE, *(((unsigned char *)&adc_tmap)+1));
		MAP_UARTCharPut(UART7_BASE, *(((unsigned char *)&adc_tmap)+2));
		MAP_UARTCharPut(UART7_BASE, *(((unsigned char *)&adc_tmap)+3));
		MAP_UARTCharPut(UART7_BASE, *(unsigned char *)&FFT_Mag[i]);//put推出，即发送 
		MAP_UARTCharPut(UART7_BASE, *(((unsigned char *)&FFT_Mag[i])+1));
		MAP_UARTCharPut(UART7_BASE, *(((unsigned char *)&FFT_Mag[i])+2));
		MAP_UARTCharPut(UART7_BASE, *(((unsigned char *)&FFT_Mag[i])+3));
		VOFA_SendFloatEND(UART7_BASE);
	}
	
	
}


















