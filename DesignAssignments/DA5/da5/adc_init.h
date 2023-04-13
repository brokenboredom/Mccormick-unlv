/*
 * adc_init.h
 *
 * Created: 4/27/2022 1:50:10 PM
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

void adc_init(void);


#ifndef ADC_INIT_H_
#define ADC_INIT_H_





#endif /* ADC_INIT_H_ */