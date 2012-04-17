#ifndef UART_H
#define UART_H

/************************** PRIVATE MACROS *************************/
// leds for status

/* buffer size definition */
#define UART_RING_BUFSIZE 256

/* Buf mask */
#define __BUF_MASK (UART_RING_BUFSIZE-1)
/* Check buf is full or not */
#define __BUF_IS_FULL(head, tail) ((tail&__BUF_MASK)==((head+1)&__BUF_MASK))
/* Check buf will be full in next receiving or not */
#define __BUF_WILL_FULL(head, tail) ((tail&__BUF_MASK)==((head+2)&__BUF_MASK))
/* Check buf is empty */
#define __BUF_IS_EMPTY(head, tail) ((head&__BUF_MASK)==(tail&__BUF_MASK))
/* Reset buf */
#define __BUF_RESET(bufidx)	(bufidx=0)
#define __BUF_INCR(bufidx)	(bufidx=(bufidx+1)&__BUF_MASK)

/** @brief UART Ring buffer structure */
typedef struct
{
	__IO uint32_t tx_head; /*!< UART Tx ring buffer head index */
	__IO uint32_t tx_tail; /*!< UART Tx ring buffer tail index */
	__IO uint32_t rx_head; /*!< UART Rx ring buffer head index */
	__IO uint32_t rx_tail; /*!< UART Rx ring buffer tail index */
	/*__IO*/ uint8_t tx[UART_RING_BUFSIZE]; /*!< UART Tx data ring buffer */
	__IO uint8_t rx[UART_RING_BUFSIZE]; /*!< UART Rx data ring buffer */
} UART_RING_BUFFER_T;

#endif /* UART_H */
