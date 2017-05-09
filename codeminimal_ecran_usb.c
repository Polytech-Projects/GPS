
#ifdef _DEBUG
  #include <__cross_studio_io.h>
#endif
#include <msp430x16x.h>

void main(void)
{
  unsigned int i;
  /* Le watchdog doit ?re d?activ?pour ne pas provoquer de red?arrage */
  WDTCTL = WDTPW + WDTHOLD;             // Stop WDT (watch dog timer)
  
  BCSCTL1 &= ~XT2OFF;                   // XT2on

  do 
  {
    IFG1 &= ~OFIFG;                       // Clear OSCFault flag
    IFG2 &= ~OFIFG;                       // Clear OSCFault flag
    //IFG2 &= ~URXIFG1;                 // flag reset
    for (i = 0xFF; i > 0; i--);           // Time for flag to set
  }
  while ((IFG1 & OFIFG) != 0);          // OSCFault flag still set?  
  
  //P4SEL |= 0x04;                        // P4.2 CMD_SWITCH
  P4DIR |= 0x06;                        // P4.2 output direction
  P4OUT = 0x04; // Reset écran
  for (i = 0x0F; i > 0; i--);
  P4OUT = 0x06; // Remise écran


  BCSCTL2 |= SELM1+SELS;                // MCLK = SMCLK = XT2 (safe)
  UCTL0 |= SWRST;                      // Initialize USART state machine
  UCTL0 = CHAR;                         // 8-bit character
  UTCTL0 = SSEL1;                       // UCLK = ACLK
  UBR00 = 0x41;                         // BRCLK/BAUD = DECIMALE, mettre en HEXA
  UBR10 = 0x03;                         //
  UMCTL0 = 0x92;                        // no modulation
  ME1 |= UTXE0 + URXE0;                 // Enable USART0 TXD/RXD
  IE1 |= URXIE0;                         // Enable USART0 RX interrupt
  P3SEL |= 0x30;                        // P3.4,5 = USART0 TXD/RXD
  P3DIR |= 0x10;                        // P3.4 output direction
  UCTL0 &= ~SWRST;                      // End initialize USART state machine
  //_EINT();                              // Enable interrupts

  UCTL1 |= SWRST;                      // Initialize USART state machine
  /* Initialisation  de UTXD1 et URXD1 */
  UCTL1 = CHAR;                         // 8-bit character
  UTCTL1 = SSEL1;                       // UCLK = ACLK
  UBR01 = 0x41;                         // BRCLK/BAUD = DECIMALE, mettre en HEXA
  UBR11 = 0x03;                         
  UMCTL1 = 0x92;                        // no modulation
  ME2 |= UTXE1 + URXE1;                 // Enable USART1 TXD/RXD
  IE2 |= URXIE1;                        // Enable USART1 RX interrupt
  P3SEL |= 0xC0;                        // P3.6,7 = USART1 option select
  P3DIR |= 0x20;                        // P3.6 = output direction
  UCTL1 &= ~SWRST;                      // End initialize USART state machine
  _EINT();

  /* Initialisation GPS */ /*
  P4SEL |= 0x01;                        // P4.0 ENABLED GPS
  P4DIR |= 0x01;                        // P4.0 output direction
  P4OUT |= 0x01;                        // P4.0 HIGH

  P4SEL |= 0x04;                        // P4.2 CMD_SWITCH
  P4DIR |= 0x04;                        // P4.2 output direction
  //P4OUT &= 0xFE;                        // P4.2 LOW
  P4OUT |= 0x04; */
  //ECRAN_Envoie(clean_screen);

  for (;;)                             
  {
  }
}

void usart0_rx (void) __interrupt[UART0RX_VECTOR] 
{
  while ((IFG2 & UTXIFG1) == 0);
  TXBUF1 =RXBUF0;
}

void usart1_rx (void) __interrupt[UART1RX_VECTOR] 
{
  while ((IFG1 & UTXIFG0) == 0);
  TXBUF0= RXBUF1;
}
