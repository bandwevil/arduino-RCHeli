#include "interrupt.h"

#define LEDOUT (1<<2)
#define MAXTHROTTLE 0x77
#define MAXTRIM 0x1D

#define checkBit(x, a) (((x)<<(a)) != 0)?1:0

char transmit[34];
char transmit_temp[34];
unsigned char currentBit;
volatile char transmitting;
int countTo;
int switchPoint;
int count;
