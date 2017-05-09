#ifndef PORTS_H
#define PORTS_H

typedef enum CMD_SWITCH
{
	USB, GPS
} CMD_SWITCH;

/**
 * Initialisation des ports:
 * - direction
 * - mode usart (sélection)
 */
void initPorts(void)
{
	/* Ports P1.0 à P1.4 en sortie
   * Respectivement, LED_0 à LED_4
   */
  P1DIR |= 0x1F;
  /* Ports P2.0 à P2.4 en entré
   * Respectivement, PUSH, TOP, BOTTOM, LEFT, RIGHT
   */
  P2DIR &= 0xE0;
  /* Ports P4.0, P4.1, P4.2 en sortie
   * Respectivement, ENABLE_GPS, RESET_LCD, CMD_SWITCH
   */
  P4DIR |= 0x07;
/*
  P3SEL |= 0xC0;                        // P3.6,7 = USART1 option select
  P3DIR |= 0x20;                        // P3.6 = output direction
  P3SEL |= 0x30;                        // P3.4,5 = USART0 TXD/RXD
  P3DIR |= 0x10;                        // P3.4 output direction
*/
  P3SEL |= 0xF0;
  P3DIR |= 0x50; // ATTENTION PEUT ETER 0x30 (WUT?)
  P3DIR &= 0x5F;
}

void setCmdSwitch(CMD_SWITCH device)
{
	if (device == USB)
	{
		P4OUT |= 0x04;
	}
	else if (device == GPS)
	{
		P4OUT &= 0xFB;
	}
}

#endif
