/*
 * uart.c
 *
 *  Created on: May 23, 2024
 *      Author: CAO HIEU
 */

#include "uart.h"
#include <string.h>
#include "stm32f4xx_ll_gpio.h"

USART_TypeDef *uart1 = USART1;
USART_TypeDef *uart2 = USART2;
USART_TypeDef *uart5 = UART5;
USART_TypeDef *uart6 = USART6;

#define TIMEOUT_DEF 500  // 500ms timeout
uint16_t timeout;

#define ATOMIC_BLOCK_START(uart) \
    if (uart == USART1) LL_USART_DisableIT_RXNE(uart1); \
    else if (uart == USART2) LL_USART_DisableIT_RXNE(uart2); \
    else if (uart == UART5) LL_USART_DisableIT_RXNE(uart5); \
    else LL_USART_DisableIT_RXNE(uart6);

#define ATOMIC_BLOCK_END(uart) \
    if (uart == USART1) LL_USART_EnableIT_RXNE(uart1); \
    else if (uart == USART2) LL_USART_EnableIT_RXNE(uart2); \
    else if (uart == UART5) LL_USART_EnableIT_RXNE(uart5); \
    else LL_USART_EnableIT_RXNE(uart6);


/* put the following in the ISR
extern void Uart_isr (USART_TypeDef *uart);
extern uint16_t timeout;
*/
ring_buffer rx_buffer1 = { { 0 }, 0, 0};
ring_buffer tx_buffer1 = { { 0 }, 0, 0};
ring_buffer rx_buffer2 = { { 0 }, 0, 0};
ring_buffer tx_buffer2 = { { 0 }, 0, 0};
ring_buffer rx_buffer5 = { { 0 }, 0, 0};
ring_buffer tx_buffer5 = { { 0 }, 0, 0};
ring_buffer rx_buffer6 = { { 0 }, 0, 0};
ring_buffer tx_buffer6 = { { 0 }, 0, 0};

ring_buffer *_rx_buffer1;
ring_buffer *_tx_buffer1;
ring_buffer *_rx_buffer2;
ring_buffer *_tx_buffer2;
ring_buffer *_rx_buffer5;
ring_buffer *_tx_buffer5;
ring_buffer *_rx_buffer6;
ring_buffer *_tx_buffer6;


void store_char(unsigned char c, ring_buffer *buffer);

void Ringbuf_init(void)
{
  _rx_buffer1 = &rx_buffer1;
  _tx_buffer1 = &tx_buffer1;
  _rx_buffer2 = &rx_buffer2;
  _tx_buffer2 = &tx_buffer2;
  _rx_buffer5 = &rx_buffer5;
  _tx_buffer5 = &tx_buffer5;
  _rx_buffer6 = &rx_buffer6;
  _tx_buffer6 = &tx_buffer6;

  /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  LL_USART_EnableIT_ERROR(uart1);
  LL_USART_EnableIT_ERROR(uart2);
  LL_USART_EnableIT_ERROR(uart5);
  LL_USART_EnableIT_ERROR(uart6);
  /* Enable the UART Data Register not empty Interrupt */
  LL_USART_EnableIT_RXNE(uart1);
  LL_USART_EnableIT_RXNE(uart2);
  LL_USART_EnableIT_RXNE(uart5);
  LL_USART_EnableIT_RXNE(uart6);
}

void store_char(unsigned char c, ring_buffer *buffer)
{
  int i = (unsigned int)(buffer->head + 1) % UART_BUFFER_SIZE;

  if(i != buffer->tail) {
	ATOMIC_BLOCK_START(USART1)
	ATOMIC_BLOCK_START(USART2)
	ATOMIC_BLOCK_START(UART5)
	ATOMIC_BLOCK_START(USART6)
    buffer->buffer[buffer->head] = c;
    buffer->head = i;
    ATOMIC_BLOCK_END(USART1)
    ATOMIC_BLOCK_END(USART2)
    ATOMIC_BLOCK_END(UART5)
    ATOMIC_BLOCK_END(USART6)
  }

}

//static int check_for (char *str, char *buffertolookinto)
//{
//	int stringlength = strlen (str);
//	int bufferlength = strlen (buffertolookinto);
//	int so_far = 0;
//	int indx = 0;
//repeat:
//	while (str[so_far] != buffertolookinto[indx])
//		{
//			indx++;
//			if (indx>stringlength) return 0;
//		}
//	if (str[so_far] == buffertolookinto[indx])
//	{
//		while (str[so_far] == buffertolookinto[indx])
//		{
//			so_far++;
//			indx++;
//		}
//	}
//
//	if (so_far == stringlength);
//	else
//	{
//		so_far =0;
//		if (indx >= bufferlength) return -1;
//		goto repeat;
//	}
//
//	if (so_far == stringlength) return 1;
//	else return -1;
//}
//

int Uart_read(USART_TypeDef *uart)
{
  ring_buffer *_rx_buffer;

  switch ((uint32_t)uart) {
    case (uint32_t)USART1:
      _rx_buffer = _rx_buffer1;
      break;
    case (uint32_t)USART2:
      _rx_buffer = _rx_buffer2;
      break;
    case (uint32_t)UART5:
      _rx_buffer = _rx_buffer5;
      break;
    default:
      _rx_buffer = _rx_buffer6;
      break;
  }

  // if the head isn't ahead of the tail, we don't have any characters
  if(_rx_buffer->head == _rx_buffer->tail)
  {
    return -1;
  }
  else
  {
    ATOMIC_BLOCK_START(uart)
    unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
    _rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % UART_BUFFER_SIZE;
    ATOMIC_BLOCK_END(uart)
    return c;
  }
}

void Uart_write(USART_TypeDef *uart, int c)
{
  ring_buffer *_tx_buffer;
  switch ((uint32_t)uart) {
    case (uint32_t)USART1:
      _tx_buffer = _tx_buffer1;
      break;
    case (uint32_t)USART2:
      _tx_buffer = _tx_buffer2;
      break;
    case (uint32_t)UART5:
      _tx_buffer = _tx_buffer5;
      break;
    default:
      _tx_buffer = _tx_buffer6;
      break;
  }

	if (c>=0)
	{
		int i = (_tx_buffer->head + 1) % UART_BUFFER_SIZE;

		ATOMIC_BLOCK_START(uart)
		while (i == _tx_buffer->tail);

		_tx_buffer->buffer[_tx_buffer->head] = (uint8_t)c;
		_tx_buffer->head = i;
		ATOMIC_BLOCK_END(uart)

		  switch ((uint32_t)uart) {
		    case (uint32_t)USART1:
		      LL_USART_EnableIT_TXE(uart1);
		      break;
		    case (uint32_t)USART2:
		      LL_USART_EnableIT_TXE(uart2);
		      break;
		    case (uint32_t)UART5:
		      LL_USART_EnableIT_TXE(uart5);
		      break;
		    default:
		      LL_USART_EnableIT_TXE(uart6);
		      break;
		  }
	}
}

/* checks if the new data is available in the incoming buffer
 */
int IsDataAvailable(USART_TypeDef *uart)
{
	  ring_buffer *_rx_buffer;
	  switch ((uint32_t)uart) {
	    case (uint32_t)USART1:
	      _rx_buffer = _rx_buffer1;
	      break;
	    case (uint32_t)USART2:
	      _rx_buffer = _rx_buffer2;
	      break;
	    case (uint32_t)UART5:
	      _rx_buffer = _rx_buffer5;
	      break;
	    default:
	      _rx_buffer = _rx_buffer6;
	      break;
	  }

  return (uint16_t)(UART_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) % UART_BUFFER_SIZE;
}

/* sends the string to the uart
 */
void Uart_sendstring (USART_TypeDef *uart, const char *s)
{
	while(*s) Uart_write(uart, *s++);
}

void GetDataFromBuffer (char *startString, char *endString, char *buffertocopyfrom, char *buffertocopyinto)
{
	int startStringLength = strlen (startString);
	int endStringLength   = strlen (endString);
	int so_far = 0;
	int indx = 0;
	int startposition = 0;
	int endposition = 0;

repeat1:
	while (startString[so_far] != buffertocopyfrom[indx]) indx++;
	if (startString[so_far] == buffertocopyfrom[indx])
	{
		while (startString[so_far] == buffertocopyfrom[indx])
		{
			so_far++;
			indx++;
		}
	}

	if (so_far == startStringLength) startposition = indx;
	else
	{
		so_far =0;
		goto repeat1;
	}

	so_far = 0;

repeat2:
	while (endString[so_far] != buffertocopyfrom[indx]) indx++;
	if (endString[so_far] == buffertocopyfrom[indx])
	{
		while (endString[so_far] == buffertocopyfrom[indx])
		{
			so_far++;
			indx++;
		}
	}

	if (so_far == endStringLength) endposition = indx-endStringLength;
	else
	{
		so_far =0;
		goto repeat2;
	}

	so_far = 0;
	indx=0;

	for (int i=startposition; i<endposition; i++)
	{
		buffertocopyinto[indx] = buffertocopyfrom[i];
		indx++;
	}


}

void Uart_flush (USART_TypeDef *uart)
{
	  ring_buffer *_rx_buffer;

	  switch ((uint32_t)uart) {
	    case (uint32_t)USART1:
	      _rx_buffer = _rx_buffer1;
	      break;
	    case (uint32_t)USART2:
	      _rx_buffer = _rx_buffer2;
	      break;
	    case (uint32_t)UART5:
	      _rx_buffer = _rx_buffer5;
	      break;
	    default:
	      _rx_buffer = _rx_buffer6;
	      break;
	  }

	  memset(_rx_buffer->buffer,'\0', UART_BUFFER_SIZE);
	  _rx_buffer->head = 0;
	  _rx_buffer->tail = 0;
}

int Uart_peek(USART_TypeDef *uart)
{
	  ring_buffer *_rx_buffer;
	  switch ((uint32_t)uart) {
	    case (uint32_t)USART1:
	      _rx_buffer = _rx_buffer1;
	      break;
	    case (uint32_t)USART2:
	      _rx_buffer = _rx_buffer2;
	      break;
	    case (uint32_t)UART5:
	      _rx_buffer = _rx_buffer5;
	      break;
	    default:
	      _rx_buffer = _rx_buffer6;
	      break;
	  }

	  if(_rx_buffer->head == _rx_buffer->tail)
	  {
	    return -1;
	  }
	  else
	  {
	    return _rx_buffer->buffer[_rx_buffer->tail];
	  }
}

int Copy_upto (USART_TypeDef *uart, char *string, char *buffertocopyinto)
{
	  ring_buffer *_rx_buffer;
	  switch ((uint32_t)uart) {
	    case (uint32_t)USART1:
	      _rx_buffer = _rx_buffer1;
	      break;
	    case (uint32_t)USART2:
	      _rx_buffer = _rx_buffer2;
	      break;
	    case (uint32_t)UART5:
	      _rx_buffer = _rx_buffer5;
	      break;
	    default:
	      _rx_buffer = _rx_buffer6;
	      break;
	  }


	int so_far =0;
	int len = strlen (string);
	int indx = 0;

again:
	while (Uart_peek(uart) != string[so_far])
		{
			buffertocopyinto[indx] = _rx_buffer->buffer[_rx_buffer->tail];
			_rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % UART_BUFFER_SIZE;
			indx++;
			while (!IsDataAvailable(uart));

		}
	while (Uart_peek(uart) == string [so_far])
	{
		so_far++;
		buffertocopyinto[indx++] = Uart_read(uart);
		if (so_far == len) return 1;
		timeout = TIMEOUT_DEF;
		while ((!IsDataAvailable(uart))&&timeout);
		if (timeout == 0) return 0;
	}

	if (so_far != len)
	{
		so_far = 0;
		goto again;
	}

	if (so_far == len) return 1;
	else return 0;
}

int Get_after (USART_TypeDef *uart, char *string, uint8_t numberofchars, char *buffertosave)
{
	for (int indx=0; indx<numberofchars; indx++)
	{
		timeout = TIMEOUT_DEF;
		while ((!IsDataAvailable(uart))&&timeout);  // wait until some data is available
		if (timeout == 0) return 0;  // if data isn't available within time, then return 0
		buffertosave[indx] = Uart_read(uart);  // save the data into the buffer... increments the tail
	}
	return 1;
}

int Wait_for (USART_TypeDef *uart, char *string)
{
	  ring_buffer *_rx_buffer;
	  switch ((uint32_t)uart) {
	    case (uint32_t)USART1:
	      _rx_buffer = _rx_buffer1;
	      break;
	    case (uint32_t)USART2:
	      _rx_buffer = _rx_buffer2;
	      break;
	    case (uint32_t)UART5:
	      _rx_buffer = _rx_buffer5;
	      break;
	    default:
	      _rx_buffer = _rx_buffer6;
	      break;
	  }

	int so_far =0;
	int len = strlen (string);

again:
	timeout = TIMEOUT_DEF;
	while ((!IsDataAvailable(uart))&&timeout);  // let's wait for the data to show up
	if (timeout == 0) return 0;
	while (Uart_peek(uart) != string[so_far])  // peek in the rx_buffer to see if we get the string
	{
		if (_rx_buffer->tail != _rx_buffer->head)
		{
			_rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % UART_BUFFER_SIZE;  // increment the tail
		}

		else
		{
			return 0;
		}
	}
	while (Uart_peek(uart) == string [so_far]) // if we got the first letter of the string
	{
		// now we will peek for the other letters too
		so_far++;
		_rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % UART_BUFFER_SIZE;  // increment the tail
		if (so_far == len) return 1;
		timeout = TIMEOUT_DEF;
		while ((!IsDataAvailable(uart))&&timeout);
		if (timeout == 0) return 0;
	}

	if (so_far != len)
	{
		so_far = 0;
		goto again;
	}

	if (so_far == len) return 1;
	else return 0;
}

void Uart_isr (USART_TypeDef *uart)
{
  //  uint32_t isrflags   = LL_USART_ReadReg(uart, SR);
    ring_buffer *_rx_buffer;
    ring_buffer *_tx_buffer;

    switch ((uint32_t)uart) {
      case (uint32_t)USART1:
        _rx_buffer = _rx_buffer1;
        _tx_buffer = _tx_buffer1;
        break;
      case (uint32_t)USART2:
        _rx_buffer = _rx_buffer2;
        _tx_buffer = _tx_buffer2;
        break;
      case (uint32_t)UART5:
        _rx_buffer = _rx_buffer5;
        _tx_buffer = _tx_buffer5;
        break;
      default:
        _rx_buffer = _rx_buffer6;
        _tx_buffer = _tx_buffer6;
        break;
    }

    /* if DR is not empty and the Rx Int is enabled */
    if ((LL_USART_IsActiveFlag_RXNE(uart) != RESET) && (LL_USART_IsEnabledIT_RXNE(uart) != RESET))
    {
//        LL_USART_ReceiveData8(uart);                /* Read status register */
//        unsigned char c = LL_USART_ReceiveData8(uart);    /* Read data register */
//        store_char (c, _rx_buffer);  // store data in buffer
//        return;

        unsigned char data = LL_USART_ReceiveData8(uart);
        if ((LL_USART_IsActiveFlag_ORE(uart) != RESET) ||
            (LL_USART_IsActiveFlag_FE(uart) != RESET) ||
            (LL_USART_IsActiveFlag_NE(uart) != RESET))
        {
          // if error, del flag
          LL_USART_ClearFlag_ORE(uart);
          LL_USART_ClearFlag_FE(uart);
          LL_USART_ClearFlag_NE(uart);
        }
        else
        {
          store_char(data, _rx_buffer); // store data in buffer
        }
        return;


    }
    /*If interrupt is caused due to Transmit Data Register Empty */
    if ((LL_USART_IsActiveFlag_TXE(uart) != RESET) && (LL_USART_IsEnabledIT_TXE(uart) != RESET))
    {
        if(_tx_buffer->head == _tx_buffer->tail)
        {
          // Buffer empty, so disable interrupts
          LL_USART_DisableIT_TXE(uart);
        }
        else
        {
          // There is more data in the output buffer. Send the next byte
          unsigned char c = _tx_buffer->buffer[_tx_buffer->tail];
          _tx_buffer->tail = (_tx_buffer->tail + 1) % UART_BUFFER_SIZE;

          LL_USART_TransmitData8(uart, c);
        }
        return;
    }
}
