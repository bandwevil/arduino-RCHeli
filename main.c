#include "main.h"

int main()
{
   //Initialize interrupt information
   transmitting = 0;
   count = 0;

   state = DIR_C;

   setControls(0, 0);

   initializeMPU();

   DDRD |= LEDOUT;
   DDRB |= (1<<5);

   initTimer2();


   while(1) {
      setControls(0, 0);
      state = DIR_C;

      while (buttonPressed(3) == 1) {
      }

      setControls(MAXTHROTTLE, 20); //Gain a bit of altitude
      _delay_ms(350);

      while (buttonPressed(3) == 0) {
         handleInput(readGyroX(), readGyroY());
         setControls(50, 20);
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

void handleInput(int gyroX, int gyroY)
{
   switch(state) {
      case DIR_C:
         if (gyroX > 13100 && gyroY > 13100) {
            state = DIR_FL;
         } else if (gyroX > 13100 && gyroY < -13100) {
            state = DIR_BL;
         } else if (gyroX < -13100 && gyroY > 13100) {
            state = DIR_FR;
         } else if (gyroX < -13100 && gyroY < -13100) {
            state = DIR_BR;
         } else if (gyroX > 13100) {
            state = DIR_L;
         } else if (gyroX < -13100) {
            state = DIR_R;
         } else if (gyroY > 13100) {
            state = DIR_F;
         } else if (gyroY < -13100) {
            state = DIR_B;
         }
         break;
      case DIR_F:
         if (gyroX > 13100) {
            state = DIR_FL;
         } else if (gyroX < -13100) {
            state = DIR_FR;
         } else if (gyroY < -13100) {
            state = DIR_C;
         }
         break;
      case DIR_B:
         if (gyroX > 13100) {
            state = DIR_BL;
         } else if (gyroX < -13100) {
            state = DIR_BR;
         } else if (gyroY > 13100) {
            state = DIR_C;
         }
         break;
      case DIR_L:
         if (gyroY > 13100) {
            state = DIR_FL;
         } else if (gyroY < -13100) {
            state = DIR_BL;
         } else if (gyroX < -13100) {
            state = DIR_C;
         }
         break;
      case DIR_R:
         if (gyroX > 13100) {
            state = DIR_C;
         } else if (gyroY > 13100) {
            state = DIR_FR;
         } else if (gyroY < -13100) {
            state = DIR_BR;
         }
         break;
      case DIR_FR:
         if (gyroX > 13100) {
            state = DIR_F;
         } else if (gyroY < -13100) {
            state = DIR_R;
         }
         break;
      case DIR_FL:
         if (gyroX < -13100) {
            state = DIR_F;
         } else if (gyroY < -13100) {
            state = DIR_L;
         }
         break;
      case DIR_BR:
         if (gyroX > 13100) {
            state = DIR_B;
         } else if (gyroY > 13100) {
            state = DIR_R;
         }
         break;
      case DIR_BL:
         if (gyroX < -13100) {
            state = DIR_B;
         } else if (gyroY > 13100) {
            state = DIR_L;
         }
         break;
      default:
         state = DIR_C;
         break;
   }
}

void setControls(unsigned char throttle, unsigned char trim)
{
   unsigned char difference, checkval, i, dirbits, lr, fb;

   if (throttle > MAXTHROTTLE) {
      throttle = MAXTHROTTLE;
   }
   if (trim > MAXTRIM) {
      trim = MAXTRIM;
   }

   difference = MAXTHROTTLE - throttle;
   difference += MAXTRIM - trim;

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

   for (i = 0; i < 34; i++) {
      transmit[i] = transmit_temp[i];
   }

}


ISR(TIMER2_COMPA_vect) {
   count++;
   if (transmitting == 0) { //Currently in between message transmissions
      if (count >= PULSES_BETWEEN_MESSAGES) {  //End of the delay, set up next message
         transmitting = 1;
         count = 0;
         currentBit = 0;
         countTo = FULL_PULSE_HEADER;
         switchPoint = HIGH_PULSE_HEADER;
         PORTB ^= (1<<5);
      }
   } else {
      if (count <= switchPoint) { //Transmit the high part of the signal
         PORTD ^= LEDOUT;
      } else if (count <= countTo) { //Finished with high transmit, go low
         PORTD &= ~LEDOUT;
      } else { //Finished with current bit, go to next one
         currentBit++;
         count = 0;
         if (currentBit >= 34) {
            transmitting = 0;
            PORTB &= ~(1<<5);
         } else if (transmit[currentBit] == 1) {
            countTo = FULL_PULSE_BIT;
            switchPoint = HIGH_PULSE_ONE;
         } else if (transmit[currentBit] == 0) {
            countTo = FULL_PULSE_BIT;
            switchPoint = HIGH_PULSE_ZERO;
         } else {
            countTo = FULL_PULSE_HEADER;
            switchPoint = HIGH_PULSE_HEADER;
         }
      }
   }
}
