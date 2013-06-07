#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/*
 * Initializes timer 2 for interrupts based on the given frequency
 * args:
 * freq - desired frequency in multiples of 100 from 100 to 500
 */
void initTimer2();
