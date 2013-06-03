#include "interrupt.h"

#define LEDOUT (1<<2)
#define MAXTHROTTLE 0x77
#define MAXTRIM 0x1D

//3846 * 26us = 50 ms between messages
#define PULSES_BETWEEN_MESSAGES 3846
#define FULL_PULSE_HEADER 357
#define HIGH_PULSE_HEADER 279
#define FULL_PULSE_BIT 80
#define HIGH_PULSE_ONE 24
#define HIGH_PULSE_ZERO 57

#define CENTER_CHECK_INIT 0x23
#define FB_CHECK_INIT 0x32
#define LR_CHECK_INIT 0x13
#define DIAG_CHECK_INIT 0x22

#define DIR_C 0
#define DIR_F 1
#define DIR_B 2
#define DIR_L 3
#define DIR_R 4
#define DIR_FR 5
#define DIR_FL 6
#define DIR_BL 7
#define DIR_BR 8

#define checkBit(x, a) ((((x)&(1<<(a))) != 0)?1:0)

void setControls(unsigned char throttle, unsigned char trim,
      unsigned char direction);

char transmit[34];
char transmit_temp[34];
unsigned char currentBit;
volatile char transmitting;
int countTo;
int switchPoint;
int count;
