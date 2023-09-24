/*
 * usart_init.h
 *
 * Created: 4/20/2022 7:01:03 PM
 *  Author: samue
 */ 
//-------------------------------------------------------------------------
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#define BAUDRATE 9600
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)
//-------------------------------------------------------------------------------

void USART_init(unsigned int ubrr);
void USART_tx_string(char *data);

#ifndef usart_INIT_H_
#define usart_INIT_H_





#endif /* usart_INIT_H_ */