/*
 * CPE 329 - Spring 2013
 * Project 2: Function Generator
 *
 * Tyler Saadus and Jonathan Hernandez
 *
 * Outputs square, sawtooth, and sine waves to an external DAC at varying
 * frequencies and duty cycles. Also supports analog input sampling and mirroring
 * through the onboard ADC.
 *
 */

#include "interrupt.h"

void initTimer2() {
   cli();

   TCCR2A = 0;// set entire TCCR2A register to 0
   TCCR2B = 0;// same for TCCR2B
   TCNT2  = 0;//initialize counter value to 0
   // turn on CTC mode
   TCCR2A |= (1 << WGM21);
   // enable timer compare interrupt
   TIMSK2 |= (1 << OCIE2A);

   //Set interrupts every 26 us (38.46 kHz)
   //f = clk/(2*OCR2A*Prescale) = 16 MHz / (2*208*1) = 38.46 kHz
   OCR2A = 208;
   TCCR2B |= (1 << CS20);

   sei();
}
