#include "main.h"

/*
 * Main program control and flow.
 * initializes the required sensors, then loops based on input from button and MPU.
 */
int main()
{
   int throttle = 32; //Approximate hover @ max charge

   //Initialize interrupt information
   transmitting = 0;
   count = 0;

   state = DIR_C;

   setControls(0, 0); //Disable helicopter

   initializeMPU();

   DDRD |= LEDOUT;
   DDRB |= (1<<5);

   initTimer2(); //Interrupts at 37 kHz for pulsing IR LED


   while(1) {
      setControls(0, 0);
      state = DIR_C;

      while (buttonPressed(3) == 1) { //Wait for button to be pressed
      }

      setControls(MAXTHROTTLE, STD_TRIM); //Gain a bit of altitude
      _delay_ms(350);

      while (buttonPressed(3) == 0) {
         handleInput(readGyroX(), readGyroY()); //Read in the directional controls, modify state as necessary

         if ((readAccelX() - ONE_G) < MIN_ACCEL_THRESHOLD) { //Accelerating down (minus gravity)
            throttle--;
         } else if ((readAccelX() - ONE_G) > MAX_ACCEL_THRESHOLD) { //Accelerating up
            throttle++;
         }

         setControls(throttle, STD_TRIM);
      }
   }

   return 0;
}

/*
 * Checks if the button input on Pin B3 is pressed or not.
 * args:
 * pin - Which pin on Port D should be checked
 * returns:
 * 0 if the button is pressed, 1 if the button is up
 */
int buttonPressed(int pin) {
   int previous, next, i = 0;

   DDRD &= ~(1<<pin);
   PORTD |= (1<<pin);

   previous = PIND & (1<<pin);

   //We read the input every 1ms, waiting until we read the same value
   //4 times in a row, which means it's stable.
   while (i < 4) {
      _delay_ms(1);
      next = PIND & (1<<pin);
      if (previous == next) {
         i++;
      } else {
         i = 0;
      }
      previous = next;
   }

   if (next >= 1) {
      return 1;
   } else {
      return 0;
   }
}


/*
 * Handle input from the gyro sensors, switching between states as necessary.
 * States correspond to the 8 cardinal directions, plus centered (straight hover)
 */
void handleInput(int gyroX, int gyroY)
{
   switch(state) {
      case DIR_C:
         if (gyroX > MAX_GYRO_THRESHOLD && gyroY > MAX_GYRO_THRESHOLD) {
            state = DIR_FL;
         } else if (gyroX > MAX_GYRO_THRESHOLD && gyroY < MIN_GYRO_THRESHOLD) {
            state = DIR_BL;
         } else if (gyroX < MIN_GYRO_THRESHOLD && gyroY > MAX_GYRO_THRESHOLD) {
            state = DIR_FR;
         } else if (gyroX < MIN_GYRO_THRESHOLD && gyroY < MIN_GYRO_THRESHOLD) {
            state = DIR_BR;
         } else if (gyroX > MAX_GYRO_THRESHOLD) {
            state = DIR_L;
         } else if (gyroX < MIN_GYRO_THRESHOLD) {
            state = DIR_R;
         } else if (gyroY > MAX_GYRO_THRESHOLD) {
            state = DIR_F;
         } else if (gyroY < MIN_GYRO_THRESHOLD) {
            state = DIR_B;
         }
         break;
      case DIR_F:
         if (gyroX > MAX_GYRO_THRESHOLD) {
            state = DIR_FL;
         } else if (gyroX < MIN_GYRO_THRESHOLD) {
            state = DIR_FR;
         } else if (gyroY < MIN_GYRO_THRESHOLD) {
            state = DIR_C;
         }
         break;
      case DIR_B:
         if (gyroX > MAX_GYRO_THRESHOLD) {
            state = DIR_BL;
         } else if (gyroX < MIN_GYRO_THRESHOLD) {
            state = DIR_BR;
         } else if (gyroY > MAX_GYRO_THRESHOLD) {
            state = DIR_C;
         }
         break;
      case DIR_L:
         if (gyroY > MAX_GYRO_THRESHOLD) {
            state = DIR_FL;
         } else if (gyroY < MIN_GYRO_THRESHOLD) {
            state = DIR_BL;
         } else if (gyroX < MIN_GYRO_THRESHOLD) {
            state = DIR_C;
         }
         break;
      case DIR_R:
         if (gyroX > MAX_GYRO_THRESHOLD) {
            state = DIR_C;
         } else if (gyroY > MAX_GYRO_THRESHOLD) {
            state = DIR_FR;
         } else if (gyroY < MIN_GYRO_THRESHOLD) {
            state = DIR_BR;
         }
         break;
      case DIR_FR:
         if (gyroX > MAX_GYRO_THRESHOLD) {
            state = DIR_F;
         } else if (gyroY < MIN_GYRO_THRESHOLD) {
            state = DIR_R;
         }
         break;
      case DIR_FL:
         if (gyroX < MIN_GYRO_THRESHOLD) {
            state = DIR_F;
         } else if (gyroY < MIN_GYRO_THRESHOLD) {
            state = DIR_L;
         }
         break;
      case DIR_BR:
         if (gyroX > MAX_GYRO_THRESHOLD) {
            state = DIR_B;
         } else if (gyroY > MAX_GYRO_THRESHOLD) {
            state = DIR_R;
         }
         break;
      case DIR_BL:
         if (gyroX < MIN_GYRO_THRESHOLD) {
            state = DIR_B;
         } else if (gyroY > MAX_GYRO_THRESHOLD) {
            state = DIR_L;
         }
         break;
      default:
         state = DIR_C;
         break;
   }
}

/*
 * Takes in the desired controls and generates an array of bits to output on
 * the IR LED.
 * Maximum values for the throttle and trim are defined in the header file.
 */
void setControls(unsigned char throttle, unsigned char trim)
{
   unsigned char difference, checkval, i, dirbits, lr, fb;

   //Make sure maximums are not exceeded, will break message format
   if (throttle > MAXTHROTTLE) {
      throttle = MAXTHROTTLE;
   }
   if (trim > MAXTRIM) {
      trim = MAXTRIM;
   }

   //Calculate the distance throttle and trim are from maximum, which will be used for the CRC
   difference = MAXTHROTTLE - throttle;
   difference += MAXTRIM - trim;

   //For each possible directional input, set the control bits as necessary
   //CRC is calculated based on data measured from the controller, which gives us the CHECK_INIT values
   switch (state) {
      case DIR_C:
      default:
         dirbits = 0x00;
         lr = 1;
         fb = 1;
         checkval = (CENTER_CHECK_INIT - difference) % 64;
         break;
      case DIR_F:
         dirbits = 0x0F;
         lr = 1;
         fb = 0;
         checkval = (FB_CHECK_INIT - difference) % 64;
         break;
      case DIR_B:
         dirbits = 0x0F;
         lr = 1;
         fb = 1;
         checkval = (FB_CHECK_INIT - difference) % 64;
         break;
      case DIR_L:
         dirbits = 0xF0;
         lr = 1;
         fb = 1;
         checkval = (LR_CHECK_INIT - difference) % 64;
         break;
      case DIR_R:
         dirbits = 0xF0;
         lr = 0;
         fb = 1;
         checkval = (LR_CHECK_INIT - difference) % 64;
         break;
      case DIR_FR:
         dirbits = 0xFF;
         lr = 0;
         fb = 0;
         checkval = (DIAG_CHECK_INIT - difference) % 64;
         break;
      case DIR_FL:
         dirbits = 0xFF;
         lr = 1;
         fb = 0;
         checkval = (DIAG_CHECK_INIT - difference) % 64;
         break;
      case DIR_BR:
         dirbits = 0xFF;
         lr = 1;
         fb = 1;
         checkval = (DIAG_CHECK_INIT - difference) % 64;
         break;
      case DIR_BL:
         dirbits = 0xFF;
         lr = 0;
         fb = 1;
         checkval = (DIAG_CHECK_INIT - difference) % 64;
         break;
   }

   //Populate temporary array with the message's bits
   for (i = 0; i < 34; i++) {
      if (i == 0) { //Header
         transmit_temp[i] = 2;
      } else if (i == 1 || i == 25) { //Message format = 0
         transmit_temp[i] = 0;
      } else if (i == 26 || i == 33) { //Message format = 1
         transmit_temp[i] = 1;
      } else if (i >= 2 && i <= 8) { //Throttle bits
         transmit_temp[i] = checkBit(throttle, 8-i);
      } else if (i >= 9 && i <= 16) { // Left/Right and Forward/Back value
         transmit_temp[i] = checkBit(dirbits, 16-i);
      } else if (i == 17) { //1 for left, 0 for right
         transmit_temp[i] = lr;
      } else if (i == 18) { //1 for back, 0 for forward
         transmit_temp[i] = fb;
      } else if (i >= 19 && i <= 24) { //Trim value
         transmit_temp[i] = checkBit(trim, 24-i);
      } else if (i >= 27 && i <= 32) { //CRC Value
         transmit_temp[i] = checkBit(checkval, 32-i);
      }
   }

   while(transmitting != 0) { //Wait until current transmission is finished
   }

   //Copy temp data into the functional array to be transmitted on the next message
   for (i = 0; i < 34; i++) {
      transmit[i] = transmit_temp[i];
   }

}


/*
 * Interrupt for timer 2.
 * Handles the transmission of data out to the IR LED, which the helicopter will read from
 * First pulses the LED for the length defined by switchPoint
 * After it counts to switchPoint, waits for a while longer until it counts to
 * countTo, which increments to the next bit transmission.
 * Once it goes through all bits, goes into idle mode and waits ~50 ms to transmit the next message
 */
ISR(TIMER2_COMPA_vect) {
   count++;
   if (transmitting == 0) { //Currently in between message transmissions
      if (count >= PULSES_BETWEEN_MESSAGES) {  //End of the delay, set up next message
         transmitting = 1;
         count = 0;
         currentBit = 0;
         countTo = FULL_PULSE_HEADER;
         switchPoint = HIGH_PULSE_HEADER;
         PORTB |= (1<<5); //Onboard LED on to signify data transmission
      }
   } else {
      if (count <= switchPoint) { //Transmit the high part of the signal
         PORTD ^= LEDOUT;
      } else if (count <= countTo) { //Finished with high transmit, go low
         PORTD &= ~LEDOUT;
      } else { //Finished with current bit, go to next one
         currentBit++;
         count = 0;
         if (currentBit >= 34) { //Finished with the message
            transmitting = 0;
            PORTB &= ~(1<<5);
         } else if (transmit[currentBit] == 1) { //Next bit is a 1
            countTo = FULL_PULSE_BIT;
            switchPoint = HIGH_PULSE_ONE;
         } else if (transmit[currentBit] == 0) { //Next bit is a 0
            countTo = FULL_PULSE_BIT;
            switchPoint = HIGH_PULSE_ZERO;
         } else { //Next bit is the header (shouldn't happen in normal operation)
            countTo = FULL_PULSE_HEADER;
            switchPoint = HIGH_PULSE_HEADER;
         }
      }
   }
}
