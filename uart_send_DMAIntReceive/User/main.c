/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"
#include "delay.h"
#include "Uart.h"
#include "matlabdata.h"
extern uint8_t u_buf[1024];//printf_u的缓存区
extern uint8_t UART0_ReceiveBuf[ReceiveBufLenth];
extern bool UART_RECEIVE_ERROR;
extern bool getnum_all;
extern uint16_t num;

extern uint8_t pui8ControlTable[1024] __attribute__ ((aligned(1024)));
// 使用16位指针直接指向偏移量 0x088
    uint16_t* pui16Ptr;

    // 读取16位数据
    






/* Main ----------------------------------------------------------------------*/
int main(void)
{

	sys_init();
	MAP_IntMasterEnable();
	//NVIC_SetPriority(UART0_IRQn, 3);//用来设置中断优先级的 UART0_IRQn 参数在msp432e401y.h里面找
	UART_Init();
	
	pui16Ptr = (uint16_t*)&pui8ControlTable[0x088];
	// 提取 [13:4] 位
	uint16_t ui16Value=(*pui16Ptr >> 4) & 0x03FF;
	//字符串用""
//	uint16_t i;
//	for(i=0;i<1024;i++){
//		u_buf[i]=i;
//	}
//	uint16_t num[2]={0x1234,0x2222};
		
		
	UARTSend(UART0_BASE,(uint8_t *)data,4096);
	while(1)
	{
//		printf_u("$$$");
//		printf_u("Hi Harry");
//		printf_u("+++");
//		delay_ms(1000);
//		size_t len;
//		//sprintf返回值是写入指针的字符数，不包括\0
//		printf_u("$$$");
//		len=sprintf((char *)u_buf,"fight!!!");//通过将 uint8_t * 转换为 char *，你实际上是在告诉编译器，你希望将这些字节解释为字符，而不是无符号整数
//		UARTSend_u((uint8_t *)u_buf,len);
//		printf_u("+++");
//		delay_ms(1000);
//		if(UART_RECEIVE_ERROR==false){
//			
//		}
		
		//test to fpga
//		UARTSend_u(u_buf,1024);
//		UARTSend(UART0_BASE,(uint8_t *)num,4);
//		UARTSend_u(u_buf,2);
	UARTSend(UART0_BASE,(uint8_t *)data,4096);
//		if(getnum_all)
//		{
//			delay_ms(1000);
//		}
	}
}


