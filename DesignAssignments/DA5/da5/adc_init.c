/*
 * adc_init.c
 *
 * Created: 4/27/2022 1:50:22 PM
 *  Author: samue
 */ 
#include "adc_init.h"

void adc_init(void)
{
	ADMUX |= (0 << REFS1) |  //Voltage reference bits
	(1 << REFS0) |
	(0 << ADLAR) |  //Left adjust
	(0 << MUX2) |   //Source selector
	(0 << MUX1) |
	(0 << MUX0);
	ADCSRA |= (1 << ADEN) | //Enable ADC
	(0 << ADSC) |           //Start conversion
	(1 << ADATE) |          //Enable auto trigger
	(0 << ADIF) |           //Interrupt flag
	(0 << ADIE) |           //Enable interrupt
	(1 << ADPS2) |          //Prescalar
	(0 << ADPS1) |
	(1 << ADPS0);
	ADCSRB |= (0 << ADTS2) |  //Auto-trigger source
	(0 << ADTS1) |
	(0 << ADTS0);
}
