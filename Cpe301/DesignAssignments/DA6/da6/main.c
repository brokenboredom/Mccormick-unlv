/*
 * da6.c
 *
 * Created: 4/13/2023 7:54:39 AM
 * Author : samue
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

#define SHIFT_REGISTER DDRB
#define SHIFT_PORT PORTB
#define DATA (1<<PB3)		//MOSI (SI)
#define LATCH (1<<PB2)		//SS   (RCK)
#define CLOCK (1<<PB5)		//SCK  (SCK)

void read_adc(void);
void adc_init(void);
void init_IO(void){
	//Setup IO
	SHIFT_REGISTER |= (DATA | LATCH | CLOCK);	//Set control pins as outputs
	SHIFT_PORT &= ~(DATA | LATCH | CLOCK);		//Set control pins low
}
void init_SPI(void){
	//Setup SPI
	SPCR = (1<<SPE) | (1<<MSTR);	//Start SPI as Master
}
void spi_send(unsigned char byte){
	SPDR = byte;			//Shift in some data
	while(!(SPSR & (1<<SPIF)));	//Wait for SPI process to finish
}

/* Segment byte maps for numbers 0 to 9 */
const uint8_t SEGMENT_MAP[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99,
									0x92, 0x82, 0xF8, 0X80, 0X90};
/* Byte maps to select digit 1 to 4 */
const uint8_t SEGMENT_SELECT[] = {0xF1, 0xF2, 0xF4, 0xF8};
	
volatile unsigned int adc_temp;
volatile unsigned int adc_hundreds;
volatile unsigned int adc_tens;
volatile unsigned int adc_ones;
volatile int j = 4;

int main(void)
{
    //Configure TIMER2
	TCCR0A |=	(1 << COM0B1) |								// Fast PWM mode non-inverting (OC0B )
				(1 << WGM01) | (1 << WGM00);				// Fast PWM, TOP: 0xFF
	TCCR0B |=	(1 << CS02) | (1 << CS01) | (1 << CS00);	// 1024 prescaler

	//Motor test configure
	DDRD |= 0x03;		// PORTD 0 and 1 as Output
	DDRD |= (1<<PD5);	// PD5(OC0B) motor enable
	PORTD = 0x01;		// set motor to run
    adc_init();			// Initialize ADC
	init_IO();			// Setup IO ports
	init_SPI();			// Initialize SPI as master
    ADCSRA |= (1<<ADSC); //trigger
    _delay_ms(125);
    sei();				// set global interrupts

    while (1) {
	    

    }
}



void adc_init(void)
{
	ADMUX |= (0 << REFS1) |  // Voltage reference bits
	(1 << REFS0) |
	(0 << ADLAR) |  // Left adjust
	(0 << MUX3 ) |  // Source selector
	(0 << MUX2) |
	(0 << MUX1) |	// Select ADC0 (PC0) also our POT
	(0 << MUX0);
	ADCSRA |= (1 << ADEN) | //Enable ADC
	(0 << ADSC) |           //Start conversion
	(1 << ADATE) |          //Enable auto trigger
	(0 << ADIF) |           //Interrupt flag
	(1 << ADIE) |           //Enable interrupt
	(1 << ADPS2) |          //Prescalar
	(0 << ADPS1) |
	(1 << ADPS0);
	ADCSRB |= (0 << ADTS2) |  //Auto-trigger source
	(0 << ADTS1) |
	(0 << ADTS0);
}

//ADC conversion complete ISR
ISR(ADC_vect)
{
	adc_temp += ADC; //sum
	j--;
	if (j==1)
	{
		adc_temp /= 16; // 0-1023 / 16 = 0-255 / 4 average
		// This adc value from POT on protoshield should set our DC PWM duty cycle
		// So this reading needs to be translated to OCR1A
		OCR0B = adc_temp;	// Overflow will just start from 0%
		adc_temp = 0;
		j = 4;

		//Pull LATCH low (Important: this is necessary to start the SPI transfer!)
		SHIFT_PORT &= ~LATCH;

		// Extract our ADC digits
		adc_hundreds = adc_temp % 100;
		adc_temp = adc_temp - (adc_hundreds * 100)
		adc_tens = adc_temp % 10;
		adc_temp = adc_temp - (adc_tens * 10)
		adc_ones = adc_temp
		spi_send((unsigned char)SEGMENT_MAP[adc_hundreds]);
		spi_send((unsigned char)0xF2);
		spi_send((unsigned char)SEGMENT_MAP[adc_tens]);
		spi_send((unsigned char)0xF4);
		spi_send((unsigned char)SEGMENT_MAP[adc_ones]);
		spi_send((unsigned char)0xF8);
		//Toggle latch to copy data to the storage register
		SHIFT_PORT |= LATCH;
		SHIFT_PORT &= ~LATCH;
		//wait for a little bit before repeating everything
		_delay_ms(125);
	}
}
