/*
 * uartstdio.h
 *
 *  Created on: Jun 21, 2024
 *      Author: CAO HIEU
 */

#ifndef UARTSTDIO_UARTSTDIO_H_
#define UARTSTDIO_UARTSTDIO_H_


#include <stdarg.h>
#include "main.h"
#include "stdbool.h"

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif
#define UART_BUFFERED
//*****************************************************************************
//
// If built for buffered operation, the following labels define the sizes of
// the transmit and receive buffers respectively.
//
//*****************************************************************************
#ifdef UART_BUFFERED
#ifndef UART_RX_BUFFER_SIZE
#define UART_RX_BUFFER_SIZE     128
#endif
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE     1024
#endif
#endif

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern void UARTStdioConfig(USART_TypeDef *Usart, bool bDisableEcho);
extern void UARTStdioConfigWriteUart(USART_TypeDef *Usart, bool bDisableEcho);
extern int UARTgets(char *pcBuf, uint32_t ui32Len);
extern unsigned char UARTgetc(void);
extern void UARTprintf(const char *pcString, ...);
extern void UARTvprintf(const char *pcString, va_list vaArgP);
extern int UARTwrite(const char *pcBuf, uint32_t ui32Len);
#ifdef UART_BUFFERED
extern int UARTPeek(unsigned char ucChar);
extern void UARTFlushTx(bool bDiscard);
extern void UARTFlushRx(void);
extern int UARTRxBytesAvail(void);
extern int UARTTxBytesFree(void);
extern void UARTEchoSet(bool bEnable);
extern void UARTStdioIntHandler(void);
extern void UART_SendData(USART_TypeDef *USARTx, char *buffer, uint32_t bufferSize);

#endif

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* UARTSTDIO_UARTSTDIO_H_ */
