/*
 * usart_init.c
 *
 * Created: 4/20/2022 7:02:03 PM
 *  Author: samue
 */ 

#include "usart_init.h"

/* Initializes the USART (RS232 interface) */
void USART_init(unsigned int ubrr) {
	UBRR0H = (ubrr >> 8);
	UBRR0L = (ubrr);
	UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0); // Enable receiver, transmitter & RX interrupt
	//asynchronous 8 N 1 // mega328
	UCSR0C = (0 << UMSEL01) |
	(0 << UMSEL00) | // 00 async operation, 01 synch operation
	(0 << UPM01) | // Parity - 0 Disabled, 0 Reserved, 1 Enabled Even, 1 Enabled Odd
	(0 << UPM00) | // Parity - 0 Disabled, 1 Reserved, 0 Enabled Even, 1 Enabled Odd
	(0 << USBS0) | // stop Bits - 0 = 1bit 1 = 2bit
	(1 << UCSZ01) | // 8 Data bits
	(1 << UCSZ00) | //
	(0 << UCPOL0); // for Synch Mode only - clock polarity

}

/* Send some data to the serial port */
void USART_tx_string( char *data ) {
	while ((*data != '\0')) {
		while (!(UCSR0A & (1 <<UDRE0)));
		UDR0 = *data;
		data++;
	}
}