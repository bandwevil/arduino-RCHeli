#include "interrupt.h"
#include "motion.h"

#define LEDOUT (1<<2) //IR LED output on D2
#define MAXTHROTTLE 0x77 //Max observed throttle
#define MAXTRIM 0x1D //Max observed trim

//One pulse every 26 microseconds
//Multiplying the values here by 26 us gives the time for each segment
#define PULSES_BETWEEN_MESSAGES 3846
#define TWO_MS_PULSES 154
#define FULL_PULSE_HEADER 357
#define HIGH_PULSE_HEADER 279
#define FULL_PULSE_BIT 80
#define HIGH_PULSE_ONE 24
#define HIGH_PULSE_ZERO 57

//base vaules used for calculating the CRC
//These are based on measured CRC values for certain controller positions
#define CENTER_CHECK_INIT 0x23
#define FB_CHECK_INIT 0x32
#define LR_CHECK_INIT 0x13
#define DIAG_CHECK_INIT 0x22

//Threshold and other input values
//These are for the MPU in default mode (2g, 250deg/s)
#define MIN_GYRO_THRESHOLD -13100
#define MAX_GYRO_THRESHOLD 13100
#define MIN_ACCEL_THRESHOLD -2000
#define MAX_ACCEL_THRESHOLD 2000
#define ONE_G 16384
#define STD_TRIM 20

//Directions for the states of control direction
#define DIR_C 0
#define DIR_F 1
#define DIR_B 2
#define DIR_L 3
#define DIR_R 4
#define DIR_FR 5
#define DIR_FL 6
#define DIR_BL 7
#define DIR_BR 8

//Macro to check if a bit is set or not
#define checkBit(x, a) ((((x)&(1<<(a))) != 0)?1:0)

void setControls(unsigned char throttle, unsigned char trim);
int buttonPressed(int pin);
void handleInput(int gyroX, int gyroY);

//Arrays to hold the message bits
char transmit[34];
char transmit_temp[34];

unsigned char currentBit; //index into transmit for the ISR
volatile char transmitting; //Whether a message is stansmitting currently or not
int countTo; //The number of ticks for the current bit
int switchPoint; //Number of ticks that the output is high for the current bit
int count; //tick count in the current bit
char state; //State of the control position
