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

#include "motion.h"

char initializeMPU()
{
   DDRC = 0xFF;   // Port C contains the pins for i2c

   _delay_ms(500);//Wait for power up

   if (read_reg(0x75) != 0x68) {
      return 1; //Some sort of init error, can't read the device ID
   }

   if (write_reg(0x6C, 0x00) != 0) { //Enable all data
      return 1;
   }
   if (write_reg(0x6B, 0x02) != 0) { //Disable sleep, use Y gyro for clocking
      return 1;
   }

   return 0;
}

int readAccelX() {
   unsigned char data[2];
   read_reg_multiple(data, 0x3B, 2); //Read the X accelerometer registers
   return data[0] << 8 & data[1];
}

int readGyroX() {
   unsigned char data[2];
   read_reg_multiple(data, 0x43, 2); //Read the X gyro registers
   return data[0] << 8 & data[1];
}

int readGyroY() {
   unsigned char data[2];
   read_reg_multiple(data, 0x45, 2); //Read the Y gyro registers
   return data[0] << 8 & data[1];
}


/*
 * changePower: toggle between standard and low power interfaces
 * reads the current setting of cycle and writes based on that
 */
void changePower()
{
   if ((read_reg(0x6B) & 0x20) == 0) {
      write_reg(0x6B, 0x24); //Enable cycle, disable temp, use internal clock
      write_reg(0x6C, 0x37); //Wakeup of 40 Hz, disable gyroscopes
   } else {
      write_reg(0x6C, 0x00); //Enable all components
      write_reg(0x6B, 0x02); //Disable cycle, revert to Y gyro clock source
   }
}

/*
 * Run built-in self test on all accelerometers and gyros
 */
void startSelfTest()
{
   write_reg(0x1B, 0xE0);
   write_reg(0x1C, 0xF0);
}

/*
 * Switch the range setting of the gyro or accelerometer
 *
 * parameters:
 * reg - the register to modify, either 0x1B or 0x1C
 *       undefined operation for other values.
 */
void nextRange(char reg)
{
   char in;
   in = (read_reg(reg) & 0x18) >> 3;
   if (in != 3) {
      in++;
   } else {
      in = 0;
   }
   write_reg(reg, in << 3);
}
