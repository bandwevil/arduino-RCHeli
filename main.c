
#include "main.h"

int main()
{
   DDRD |= LEDOUT;

   //Initialize interrupt information
   transmitting = 0;
   count = 0;

   setControls(0, 0, 0);

   currentBit = 0;

   initTimer2();

   while(1) {
      setControls(1, MAXTRIM, DIR_C);
      _delay_ms(500);
      setControls(1, MAXTRIM, DIR_FL);
      _delay_ms(500);
      setControls(1, MAXTRIM, DIR_C);
      _delay_ms(500);
      setControls(1, MAXTRIM, DIR_BR);
      _delay_ms(500);
   }

   return 0;
}

void setControls(unsigned char throttle, unsigned char trim,
      unsigned char direction)
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

   switch (direction) {
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

   while(transmitting == 1) { //Wait until current transmission is finished
   }
   if (count > 3846-154) {
      count = 3692; // Delay next message by up to 2 ms to avoid reading in middle
   }

   for (i = 0; i < 34; i++) {
      transmit[i] = transmit_temp[i];
   }

}


ISR(TIMER2_COMPA_vect) {
   count++;
   if (transmitting == 0) { //Currently in between message transmissions
      if (count >= 3846) {  //End of the delay, set up next message
         //3846 * 26us = 50 ms between messages
         transmitting = 1;
         count = 0;
         currentBit = 0;
         countTo = 357;
         switchPoint = 279;
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
         } else if (transmit[currentBit] == 1) {
            countTo = 80;
            switchPoint = 24;
         } else if (transmit[currentBit] == 0) {
            countTo = 80;
            switchPoint = 57;
         } else {
            countTo = 357;
            switchPoint = 279;
         }
      }
   }
}
