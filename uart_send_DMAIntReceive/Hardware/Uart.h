#ifndef UART_H
#define UART_H
#include "ti/devices/msp432e4/driverlib/driverlib.h"//MAP_xx 硬件相关的宏......所需要
#include "uart.h"
#include <stdio.h>
#include "stdint.h"
#include "stdbool.h"
//#include "uartstdio.h"//not in stdlibarary//use UARTStdioConfig function to configure UART but can use MAP_UARTConfigSetExpClk instead



extern uint8_t u_buf[1024];//发送缓存区
//UART0_BASE  UART2_BASE是自定义的enum用来指定所要发送的UART
typedef enum {
    usart2_u = 0,//处理串口屏
    usart0_u = 1,//处理bluetooth
    // 其他标志位定义
}usart_user;

//#define printf_u(...) do { \
//    size_t __size; \
//    __size = sprintf((char *)u_buf, __VA_ARGS__); \
//    if (selected_USART == usart0_u) { \
//        UARTSend(UART0_BASE, (uint8_t *)u_buf, __size); \
//    } else if (selected_USART == usart2_u) { \
//        UARTSend(UART2_BASE, (uint8_t *)u_buf, __size); \
//    } \
//} while (0)
#define HEAD '$'
#define TAIL '+'
#define HEAD_TAIL_LENTH 3
#define ReceiveBufLenth 8
#define printf_u(...) do { \
    size_t __size; \
    __size = sprintf((char *)u_buf, __VA_ARGS__); \
    UARTSend(UART0_BASE, (uint8_t *)u_buf, __size); \
} while (0)
/*
标准 ASCII 编码用一个字节中的 7 位就能存储,为了让第 8 位（最高位）也参与编码，
就形成了扩展 ASCII 编码。扩展 ASCII 主要包含了一些特殊符号、外来语字母和图形符号
也就是说用标准的ascll库不用担心uint8_t与char的问题，因为标准的ascll库里面字符的最高位(第7位)都是0
*/
void UART_Init(void);
void UARTSend(uint32_t ui32Base, const uint8_t *pui8Buffer, uint32_t ui32Count);
void UARTSend_u(const uint8_t *pui8Buffer,uint32_t len);
#endif