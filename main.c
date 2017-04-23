// Texas Instruments FET P140 Demonstration Program
// Copyright (c) 2002, 2003 Rowley Associates Limited
//
// Toggles the LED attached to P1.0 on the P140 under interrupt control.

#include <msp430x16x.h>

void
main(void)
{
  // Stop watchdog.
  WDTCTL = WDTPW + WDTHOLD;

  // Drive off SMCLK (from DCO).
  TACTL = TASSEL1 + TACLR;

  // Enable CCR0 interrupt.
  CCTL0 = CCIE;

  // Set timer value.
  CCR0 = 65000;

  // P1.0 to output.
  P1DIR |= 0x01;

  // Start Timer_A in continuous mode.
  TACTL |= MC1;

  // Enable interrupts.
  _EINT();                              

  // Turn off the CPU, as no ACLK can't use LPM3.
  for (;;)
    _BIS_SR(CPUOFF);
}

// Timer A0 interrupt service routine.
void
timera_isr(void) __interrupt[TIMERA0_VECTOR]
{
  // Toggle LED.
  P1OUT ^= 0x01;

  // Schedule next interrupt.
  CCR0 += 65000;
}
