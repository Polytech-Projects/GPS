
#ifdef _DEBUG
  #include <__cross_studio_io.h>
#endif
#include <msp430x16x.h>
#include "adafruit_GPS.h"
#include "ports.h"
#include "uart.h"
#include "ecran.h"
#include "pad.h"
#include "led.h"
#include "tools.h"
#include <math.h>

typedef enum Mode {
  MAINMENU, BOUSSOLE
} Mode;

Mode mode = MAINMENU;

void main(void)
{
  unsigned int i;
  int j;
  float degres;
  /* Le watchdog doit ?re d?activ?pour ne pas provoquer de red?arrage */
   WDTCTL = WDTPW + WDTHOLD;             // Stop WDT (watch dog timer)
  BCSCTL1 &= ~XT2OFF;                   // XT2on

  do
  {
    IFG1 &= ~OFIFG;                       // Clear OSCFault flag
    IFG2 &= ~OFIFG;                       // Clear OSCFault flag
    P2IFG = 0;
    for (i = 0xFF; i > 0; i--);           // Time for flag to set
  }
  while ((IFG1 & OFIFG) != 0);          // OSCFault flag still set?

  BCSCTL2 |= SELM1+SELS+DIVM1;                // MCLK = SMCLK = XT2 (safe)

  initPorts();
  initUart0();
  initUart1();

  // Reset de l'�cran
  P4OUT = 0x04; // Reset �ran
  for (i = 0x0F; i > 0; i--);
  P4OUT = 0x06; // Remise �ran

  P1OUT = 0x00; // Eteint les leds (chiant qd allumé)

  gps_init();
  setCmdSwitch(GPS);
  //setCmdSwitch(USB);
  setGPS(1);
  wakeup();

  gps_send_command(PMTK_SET_BAUD_9600);
  gps_send_command(PMTK_SET_NMEA_UPDATE_1HZ);
  gps_send_command(PMTK_SET_NMEA_OUTPUT_OFF);
  gps_send_command(PMTK_SET_NMEA_OUTPUT_RMCGGA);

  degres = cos(180);
  debug_printf("%f\n", degres);
  
  led_centre(1);
  delay(1000);
  //reverse screen
  screenReverse();
  // stop scroll
  sendCommandScreen("000C0000");
  MainMenu();
  _EINT(); // Interrupt ON
  
  for (;;)
  {
    if (nmea_received)
    {
      _DINT();
      #ifdef _DEBUG
        gps_debug_trame();
      #endif
      if (gps_parse())
      {
        #ifdef _DEBUG
          gps_debug_parse();
        #endif
        switch (mode) {
          case MAINMENU:
          break;
          case BOUSSOLE:
          break;
        }
      }
      _EINT();
    }
  }
}

void usart0_rx (void) __interrupt[UART0RX_VECTOR]
{
  //POUR QUE LECRAN MARCHE EN USB
  //-----------------------------
  //while ((IFG2 & UTXIFG1) == 0);
  //TXBUF1 = RXBUF0;
  gps_read();
}

void usart1_rx (void) __interrupt[UART1RX_VECTOR]
{
  //POUR QUE LECRAN MARCHE EN USB
  //-----------------------------
  //while ((IFG1 & UTXIFG0) == 0);
  //TXBUF0 = RXBUF1;
}

void bouton_push (void) __interrupt[PORT2_VECTOR] 
{
  if(P2IFG & 0x01) // Push
  {
    if((P2IES & 0x01)) P1OUT = 0x01; // Pressé
    else; // Relaché
  }
  if(P2IFG & 0x02) // Top
  {
    if((P2IES & 0x02)) P1OUT = 0x02;
    else;
  }
  if(P2IFG & 0x04) // Bottom
  {
    if((P2IES & 0x04)) P1OUT = 0x04;
    else;
  }
  if(P2IFG & 0x08) // Left
  {
    if((P2IES & 0x08))
    {
      standby();
      MainMenu();
      mode = MAINMENU;
    }
    else;
  }
  if(P2IFG & 0x10){ // Right
    if((P2IES & 0x10))
    {
      wakeup();
      BoussoleMenu();
      mode = BOUSSOLE;
    }
    else;
  }
  P2IES ^= P2IFG; // inversion des transition
  P2IFG = 0;
}
