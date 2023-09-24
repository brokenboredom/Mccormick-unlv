/*
 * cpe301.c
 *
 * Created: 3/19/2023 11:28:24 PM
 * Author : Samuel McCormick
 * Class: CpE 301
 * Assignment: Design Assignment 3
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define LEDA PB1	// Part a
#define LEDB PB3	// Part b
#define LEDC PB2	// Part c

volatile int COMPA_count;	// OVF Counter for part c
volatile int TOV0_count;	// OVF counter for part a

ISR (TIMER1_OVF_vect)
{
	PORTB ^= (1 << LEDB);	// Toggle our LED
	TCNT1 = 49926;			// for 0.999 sec at 16 MHz (65536 - (16 MHz / 1024)*0.999)
}

ISR (TIMER2_COMPA_vect)
{
	COMPA_count++;				// 16 MHz / (1024 + 1+255)*0.666 = 40.65 compare matches
	if (COMPA_count == 40)
	{
		TCNT2 = 166;			// Adjusted final cycle
	}
	if (COMPA_count == 41)
	{
		PORTB ^= (1 << LEDC);	// Toggle our LED
		COMPA_count = 0;		// Reset OVF counter
		TCNT2 = 0;				// Reset timer2 clock
	}
}

int main()
{
	// Configure PORTS
	DDRB |= (0x01 << LEDA);	//Configure the PORTB5 as output
	DDRB |= (0x01 << LEDB);	//Configure the PORTB3 as output
	DDRB |= (0x01 << LEDC);	//Configure the PORTB4 as output
	
	// Configure Timer0
	TCNT0 = 0;										// for 0.333 sec at 16 MHz	
	TCCR0A = 0x00;
	TCCR0B |= (1<<CS02) | (0<<CS01) | (1<<CS00);	// Normal mode with 1024 prescaler 
	TOV0_count = 0;									// OVF count
	
	// Configure Timer1
	TCNT1 = 49926;									// for 0.999 sec at 16 MHz	
	TCCR1A = 0x00;
	TCCR1B |= (1<<CS12) | (0<<CS11) | (1<<CS10);	// Normal mode with 1024 prescaler
	TIMSK1 |= (1<<TOIE1);							// Set timer interrupt
	
	// Configure Timer2
	TCNT2 = 0;
	OCR2A = 255;
	TCCR2A |= (1<<WGM21) | (0<<WGM20);							// CTC mode
	TCCR2B |= (0<<WGM22) | (1<<CS22) | (1<<CS21) | (1<<CS20);	// Prescaler to 1024
	TIMSK2 |= (1<<OCIE2A);										// Set timer interrupt
	COMPA_count = 0;											// Compare match count
	
	sei();		// Enable global interrupts
	
	while(1)
	{			
		while ((TIFR0 & 0x01) == 0)
		{
			// wait here for overflow flag
		}; 
		TCNT0 = 0x00;	// Reset timer 0
		TIFR0 = 0x01;	// clear timer0 overflow flag (write logic 1)	              
		TOV0_count++; 
		if (TOV0_count==20)
		{
			PORTB ^= (0x01 << LEDA);	// toggle our LED		 
			TOV0_count=0;				// Reset count
		}
	}
}



