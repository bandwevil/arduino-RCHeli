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

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/*
 * Initializes timer 2 for interrupts based on the given frequency
 * args:
 * freq - desired frequency in multiples of 100 from 100 to 500
 */
void initTimer2();
