/**
 * @file	: comm.c
 * @purpose	: UART using interrupt mode to test the UART driver
 * @version	: 1.0
 * @date	: 18. Mar. 2009 (NXP), July 2010 (mthomas)
 * @author	: HieuNguyen (NXP example), Martin Thomas (adapted as "library")
 *----------------------------------------------------------------------------
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/

#include "lpc17xx.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_nvic.h"
#include "lpc17xx_pinsel.h"

#include "comm.h"
#include "uart.h"

#define UART_PORT 0

#if (UART_PORT == 0)
#define TEST_UART LPC_UART0
#elif (UART_PORT == 1)
#define TEST_UART LPC_UART1
#endif


/************************** PRIVATE VARIABLES *************************/
// UART Ring buffer
static UART_RING_BUFFER_T rb;
// Current Tx Interrupt enable state
static __IO FlagStatus TxIntStat;

/************************** PRIVATE FUNCTIONS *************************/
#if (UART_PORT == 0)
void UART0_IRQHandler(void);
#elif (UART_PORT == 1)
void UART1_IRQHandler(void);
#endif

static void UART_IntTransmit(void);
static void UART_IntReceive(void);
static uint32_t UARTReceive(LPC_UART_TypeDef *UARTPort, uint8_t *rxbuf, uint8_t buflen);
static uint32_t UARTSend(LPC_UART_TypeDef *UARTPort, uint8_t *txbuf, uint8_t buflen);

#if (UART_PORT == 0)
/*********************************************************************//**
 * @brief	UART0 interrupt handler sub-routine reference, just to call the
 * 				standard interrupt handler in uart driver
 * @param	None
 * @return	None
 **********************************************************************/
void UART0_IRQHandler(void)
{
	// Call Standard UART 0 interrupt handler
	UART0_StdIntHandler();
}
#endif

#if (UART_PORT == 1)
/*********************************************************************//**
 * @brief	UART1 interrupt handler sub-routine reference, just to call the
 * 				standard interrupt handler in uart driver
 * @param	None
 * @return	None
 **********************************************************************/
static void UART1_IRQHandler(void)
{
	// Call Standard UART 0 interrupt handler
	UART1_StdIntHandler();
}
#endif

/********************************************************************//**
 * @brief 		UART receive function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
static void UART_IntReceive(void)
{
	uint8_t tmpc;
	uint32_t rLen;

	while (1)
	{
		// Call UART read function in UART driver
		rLen = UART_Receive(TEST_UART, &tmpc, 1, NONE_BLOCKING);
		// If data received
		if (rLen)
		{
			/* Check if buffer is more space
			 * If no more space, remaining character will be trimmed out
			 */
			if (!__BUF_IS_FULL(rb.rx_head,rb.rx_tail))
			{
				rb.rx[rb.rx_head] = tmpc;
				__BUF_INCR(rb.rx_head);
			}
		}
		// no more data
		else
		{
			break;
		}
	}
}

/********************************************************************//**
 * @brief 		UART transmit function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
static void UART_IntTransmit(void)
{
	// Disable THRE interrupt
	UART_IntConfig(TEST_UART, UART_INTCFG_THRE, DISABLE);

	/* Wait for FIFO buffer empty, transfer UART_TX_FIFO_SIZE bytes
	 * of data or break whenever ring buffers are empty */
	/* Wait until THR empty */
	while (UART_CheckBusy(TEST_UART) == SET)
		;

	while (!__BUF_IS_EMPTY(rb.tx_head,rb.tx_tail))
	{
		/* Move a piece of data into the transmit FIFO */
		if (UART_Send(TEST_UART, (uint8_t *) &rb.tx[rb.tx_tail], 1,
				NONE_BLOCKING))
		{
			/* Update transmit ring FIFO tail pointer */
			__BUF_INCR(rb.tx_tail);
		}
		else
		{
			break;
		}
	}

	/* If there is no more data to send, disable the transmit
	 interrupt - else enable it or keep it enabled */
	if (__BUF_IS_EMPTY(rb.tx_head, rb.tx_tail))
	{
		UART_IntConfig(TEST_UART, UART_INTCFG_THRE, DISABLE);
		// Reset Tx Interrupt state
		TxIntStat = RESET;
	}
	else
	{
		// Set Tx Interrupt state
		TxIntStat = SET;
		UART_IntConfig(TEST_UART, UART_INTCFG_THRE, ENABLE);
	}
}

/*********************************************************************//**
 * @brief		UART Line Status Error callback
 * @param[in]	bLSErrType	UART Line Status Error Type
 * @return		None
 **********************************************************************/
static void UART_IntErr(uint8_t bLSErrType)
{
	uint8_t test;
	// Loop forever
	while (1)
	{
		// For testing purpose
		test = bLSErrType;
	}
}

/*********************************************************************//**
 * @brief		UART transmit function for interrupt mode (using ring buffers)
 * @param[in]	UARTPort	Selected UART peripheral used to send data,
 * 				should be UART0
 * @param[out]	txbuf Pointer to Transmit buffer
 * @param[in]	buflen Length of Transmit buffer
 * @return 		Number of bytes actually sent to the ring buffer
 **********************************************************************/
static uint32_t UARTSend(LPC_UART_TypeDef *UARTPort, uint8_t txbuf[], uint8_t buflen)
{
	uint8_t* data = &txbuf[0];
	uint32_t bytes = 0;

	/* Temporarily lock out UART transmit interrupts during this
	 read so the UART transmit interrupt won't cause problems
	 with the index values */
	UART_IntConfig(UARTPort, UART_INTCFG_THRE, DISABLE);

	/* Loop until transmit run buffer is full or until n_bytes
	 expires */
	while ((buflen > 0) && (!__BUF_IS_FULL(rb.tx_head, rb.tx_tail)))
	{
		/* Write data from buffer into ring buffer */
		rb.tx[rb.tx_head] = *data;
		data++;

		/* Increment head pointer */
		__BUF_INCR(rb.tx_head);

		/* Increment data count and decrement buffer size count */
		bytes++;
		buflen--;
	}

	/*
	 * Check if current Tx interrupt enable is reset,
	 * that means the Tx interrupt must be re-enabled
	 * due to call UART_IntTransmit() function to trigger
	 * this interrupt type
	 */
	if (TxIntStat == RESET)
	{
		UART_IntTransmit();
	}
	/*
	 * Otherwise, re-enables Tx Interrupt
	 */
	else
	{
		UART_IntConfig(UARTPort, UART_INTCFG_THRE, ENABLE);
	}

	return bytes;
}

/*********************************************************************//**
 * @brief		UART read function for interrupt mode (using ring buffers)
 * @param[in]	UARTPort	Selected UART peripheral used to send data,
 * 				should be UART0
 * @param[out]	rxbuf Pointer to Received buffer
 * @param[in]	buflen Length of Received buffer
 * @return 		Number of bytes actually read from the ring buffer
 **********************************************************************/
static uint32_t UARTReceive(LPC_UART_TypeDef *UARTPort, uint8_t *rxbuf, uint8_t buflen)
{
	uint8_t *data = (uint8_t *) rxbuf;
	uint32_t bytes = 0;

	/* Temporarily lock out UART receive interrupts during this
	 read so the UART receive interrupt won't cause problems
	 with the index values */
	UART_IntConfig(UARTPort, UART_INTCFG_RBR, DISABLE);

	/* Loop until receive buffer ring is empty or
	 until max_bytes expires */
	while ((buflen > 0) && (!(__BUF_IS_EMPTY(rb.rx_head, rb.rx_tail))))
	{
		/* Read data from ring buffer into user buffer */
		*data = rb.rx[rb.rx_tail];
		data++;

		/* Update tail pointer */
		__BUF_INCR(rb.rx_tail);

		/* Increment data count and decrement buffer size count */
		bytes++;
		buflen--;
	}

	/* Re-enable UART interrupts */
	UART_IntConfig(UARTPort, UART_INTCFG_RBR, ENABLE);

	return bytes;
}

/*********************************************************************//**
 * @brief	UART init sub-routine
 **********************************************************************/
static int uart_init_intern(void)
{
	// UART Configuration structure variable
	UART_CFG_Type UARTConfigStruct;
	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	// Pin configuration for UART0
	PINSEL_CFG_Type PinCfg;

#if (UART_PORT == 0)
	/*
	 * Initialize UART0 pin connect
	 */
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 2;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 3;
	PINSEL_ConfigPin(&PinCfg);
#endif

#if (UART_PORT == 1)
	/*
	 * Initialize UART1 pin connect
	 */
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 0;
	PinCfg.Portnum = 2;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 1;
	PINSEL_ConfigPin(&PinCfg);
#endif

	/* Initialize UART Configuration parameter structure to default state:
	 * Baudrate = 9600bps
	 * 8 data bit
	 * 1 Stop bit
	 * None parity
	 */
	UART_ConfigStructInit(&UARTConfigStruct);
	// Re-configure baud-rate to 115200bps
	UARTConfigStruct.Baud_rate = 115200;

	// Initialize UART0 peripheral with given to corresponding parameter
	UART_Init(TEST_UART, &UARTConfigStruct);

	/* Initialize FIFOConfigStruct to default state:
	 * 				- FIFO_DMAMode = DISABLE
	 * 				- FIFO_Level = UART_FIFO_TRGLEV0
	 * 				- FIFO_ResetRxBuf = ENABLE
	 * 				- FIFO_ResetTxBuf = ENABLE
	 * 				- FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Initialize FIFO for UART0 peripheral
	UART_FIFOConfig(TEST_UART, &UARTFIFOConfigStruct);

	// Setup callback ---------------
	// Receive callback
	UART_SetupCbs(TEST_UART, 0, (void *) UART_IntReceive);
	// Transmit callback
	UART_SetupCbs(TEST_UART, 1, (void *) UART_IntTransmit);
	// Line Status Error callback
	UART_SetupCbs(TEST_UART, 3, (void *) UART_IntErr);

	// Enable UART Transmit
	UART_TxCmd(TEST_UART, ENABLE);

	/* Enable UART Rx interrupt */
	UART_IntConfig(TEST_UART, UART_INTCFG_RBR, ENABLE);
	/* Enable UART line status interrupt */
	UART_IntConfig(TEST_UART, UART_INTCFG_RLS, ENABLE);
	/*
	 * Do not enable transmit interrupt here, since it is handled by
	 * UART_Send() function, just to reset Tx Interrupt state for the
	 * first time
	 */
	TxIntStat = RESET;

	// Reset ring buf head and tail idx
	__BUF_RESET(rb.rx_head);
	__BUF_RESET(rb.rx_tail);
	__BUF_RESET(rb.tx_head);
	__BUF_RESET(rb.tx_tail);

#if (UART_PORT == 0)
	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(UART0_IRQn, ((0x01 << 3) | 0x01));
	/* Enable Interrupt for UART0 channel */
	NVIC_EnableIRQ(UART0_IRQn);
#endif

#if (UART_PORT == 1)
	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(UART1_IRQn, ((0x01<<3)|0x01));
	/* Enable Interrupt for UART0 channel */
	NVIC_EnableIRQ(UART1_IRQn);
#endif

	return 0;
}

/************************** term_io - Interface  *************************/

int comm_test(void)
{
	return ( __BUF_IS_EMPTY(rb.rx_head, rb.rx_tail) ) ? 0 : 1;
}

char comm_get(void)
{
	uint8_t buf[] = {'\0', '\0' };
	while ( UARTReceive(TEST_UART, buf, 1) < 1 ) { ; }
	return (char)buf[0];
}

void comm_put(char d)
{
	uint8_t buf[2];

	buf[0] = (uint8_t)d;
	while ( UARTSend(TEST_UART, buf, 1) < 1 ) { ; }
}

void comm_puts(const char* s)
{
	char c;
	while ( ( c = *s++ ) != '\0' ) {
		comm_put(c);
	}
}

void comm_init(void)
{
	uart_init_intern();
}

int comm_txbusy(void)
{
	return ( UART_CheckBusy(TEST_UART) == SET ) ? 1 : 0;
}

void xcomm_put(unsigned char c)
{
	comm_put((char)c);
}

unsigned char xcomm_get(void)
{
	return (unsigned char) comm_get();
}
