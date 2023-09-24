/*
 * da5.c
 *
 * Created: 4/12/2023 5:50:19 PM
 * Author : Samuel McCormick
 */ 
//-------------------------------------------------------------------------
#define F_CPU 16000000UL

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>

#define BAUDRATE 9600
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)
//-------------------------------------------------------------------------------

#define  Trigger_pin	PB5			// Trigger pin

int TimerOverflow = 0;				// counts overflows

volatile uint16_t tempCnt = 250;	// holds TCNT1 for US sensor
volatile uint16_t distance = 0;		// holds distance from echo calculation
volatile int servoCnt = 97;			// holds OCR1A for servo
volatile uint16_t degree = 0;		// holds approx. angle

void USART_init(void){
	
	UBRR0H = (uint8_t)(BAUD_PRESCALLER>>8);
	UBRR0L = (uint8_t)(BAUD_PRESCALLER);
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	UCSR0C = (3<<UCSZ00);
}

void USART_send( unsigned char data){
	
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
	
}

void USART_putstring(char* StringPtr){
	
	while(*StringPtr != 0x00){
		USART_send(*StringPtr);
	StringPtr++;}
	
}

ISR(TIMER1_OVF_vect)
{
	TimerOverflow++;		/* Increment Timer Overflow count */
}

int main(void)
{
	char string[10];
	char angle[10];
	long count;
	double distance;

	DDRB|=(1<<PB1);				// PWM Pins as Out	
	DDRB |= (1<<Trigger_pin);	// Make trigger pin as output, PB0 is echo
	
	USART_init();
	
	sei();						/* Enable global interrupt */


	//--------------------------------------------
	
	while(1)
	{
		
		// -- DO Servo operation
		//--------------------------------------------
		//Servo motor operations
	
		//Configure TIMER1
		TCCR1A|=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);			// NON Inverted PWM
		TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS11)|(1<<CS10);	// PRESCALER=64 MODE 14(FAST PWM)
		TIMSK1 &= ~(1 << TOIE1);							// Disable Timer1 overflow interrupts

		ICR1=4999;											// 16MHz/64/4999 = fPWM=50Hz (Period = 20ms Standard).
		//--------------------------------------------
		// 535 = 180 degrees
		if(servoCnt > 535) {
			servoCnt = 97; // 0 degrees
			OCR1A = servoCnt;
			degree = 0;
			_delay_ms(100);
		}
		else {
			OCR1A = servoCnt;
			/*******************************************************/
			degree = degree + 2;
			servoCnt += 5;			// 535/97 = 438. 438/(180/2) ~6. 438/(180/2.5) ~= 4. We'll use 5 for ~87 segments
			_delay_ms(500);
		}
		
		// -- DO Sonic operation
		TIMSK1 = (1 << TOIE1);			// Enable Timer1 overflow interrupts
		TCCR1A = 0;						// Set all bit to zero Normal operation

		PORTB |= (1 << Trigger_pin);	// Give 10us trigger pulse on trig. pin to HC-SR04
		_delay_us(10);
		PORTB &= (~(1 << Trigger_pin));
		
		TCNT1 = 0;			// Clear Timer counter
		TCCR1B = 0x41;		// Setting for capture rising edge, No pre-scaler
		TIFR1 = 1<<ICF1;	// Clear ICP flag (Input Capture flag)
		TIFR1 = 1<<TOV1;	// Clear Timer Overflow flag

		// Calculate width of Echo by Input Capture (ICP) on PortD PD6
		
		while ((TIFR1 & (1 << ICF1)) == 0);		// Wait for rising edge
		TCNT1 = 0;								// Clear Timer counter
		TCCR1B = 0x01;							// Setting for capture falling edge, No pre-scaler
		TIFR1 = 1<<ICF1;						// Clear ICP flag (Input Capture flag)
		TIFR1 = 1<<TOV1;						// Clear Timer Overflow flag
		TimerOverflow = 0;						// Clear Timer overflow count

		while ((TIFR1 & (1 << ICF1)) == 0);		// Wait for falling edge
		count = ICR1 + (65535 * TimerOverflow);	// Take value of capture register
		
		// 8MHz Timer freq, sound speed =343 m/s, calculation mentioned in doc.
		distance = (double)count / (58*16);
		// Output should look like: "angle,distance."
		itoa(degree, angle, 10);			// String to Int angle for Processor graph
		USART_putstring(angle);				
		USART_putstring(",");				// add the comma
		itoa(distance, string, 10);			// String to int distance
		USART_putstring(string);
		USART_putstring(".");				// add ending period
		_delay_ms(10);
		
	}
}
