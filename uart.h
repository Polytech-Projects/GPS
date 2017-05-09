#ifndef UART_H
#define UART_H

#define BAUD_RATE_LSB 0x41
#define BAUD_RATE_MSB 0x03
#define MODULATION 0x92

void initUart0(void)
{
	UCTL0 |= SWRST;                      // Initialize USART state machine
  UCTL0 = CHAR;                         // 8-bit character
  UTCTL0 = SSEL1;                       // UCLK = ACLK
  UBR00 = BAUD_RATE_LSB;                         // BRCLK/BAUD = DECIMALE, mettre en HEXA
  UBR10 = BAUD_RATE_MSB;                         //
  UMCTL0 = MODULATION;
  ME1 |= UTXE0 + URXE0;                 // Enable USART0 TXD/RXD
  IE1 |= URXIE0;                         // Enable USART0 RX interrupt
  UCTL0 &= ~SWRST;                      // End initialize USART state machine
}
void initUart1(void)
{
  UCTL1 |= SWRST;                      // Initialize USART state machine
  /* Initialisation  de UTXD1 et URXD1 */
  UCTL1 = CHAR;                         // 8-bit character
  UTCTL1 = SSEL1;                       // UCLK = ACLK
  UBR01 = BAUD_RATE_LSB;                         // BRCLK/BAUD = DECIMALE, mettre en HEXA
  UBR11 = BAUD_RATE_MSB;                         
  UMCTL1 = MODULATION;
  ME2 |= UTXE1 + URXE1;                 // Enable USART1 TXD/RXD
  IE2 |= URXIE1;                        // Enable USART1 RX interrupt
  UCTL1 &= ~SWRST;                      // End initialize USART state machine
}

#endif
