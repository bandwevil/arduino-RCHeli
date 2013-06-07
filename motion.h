#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <stdlib.h> // Standard C library
#include "twi.h"

int readAccelX();
int readGyroX();
int readGyroY();
char initializeMPU();
void nextRange(char reg);
void startSelfTest();
void changePower();
