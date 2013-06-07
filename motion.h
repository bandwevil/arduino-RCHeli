/*
 * CPE 329 - Spring 2013
 * Project 3: Motion Sensor
 *
 * Tyler Saadus and Jonathan Hernandez
 *
 * Sense motion via an MPU-6050 connected to an Arduino
 * Results are then transmitted over UART to be displayed on the PC
 *
 * Transmission format: a one byte identifier, then two bytes of data
 */

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
