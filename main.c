
#ifdef _DEBUG
  #include <__cross_studio_io.h>
#endif
#include <msp430x16x.h>
#include "adafruit_GPS.h"
#include "ports.h"
#include "uart.h"

GPS_Data data;
volatile char g = 0;
char trame[200];
volatile int compteur = 0;
volatile int ok = 0;
char *type;

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
    P2IFG = 0;
    for (i = 0xFF; i > 0; i--);           // Time for flag to set
  }
  while ((IFG1 & OFIFG) != 0);          // OSCFault flag still set?  

  BCSCTL2 |= SELM1+SELS+DIVM1;                // MCLK = SMCLK = XT2 (safe)

  initPorts();
  initUart0();
  initUart1();

  P1OUT = 0x00; // Eteint les leds (chiant qd allumé)

  gpsDataInit(&data);
  setCmdSwitch(GPS);
  setGPS(1);
  wakeup();

  sendCommand(PMTK_SET_BAUD_9600);
  sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  sendCommand(PMTK_SET_NMEA_OUTPUT_OFF);
  sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);

  _EINT(); // Interrupt ON

  for (;;)
  {
    if (ok)
    {
      _DINT();
      debug_printf("\nMATRAME %s\n", trame);
      ok = 0;
      if (parse(trame, &data))
      {
        #ifdef _DEBUG
        if (data.type == GGA)
        {
          type = "gga";
        }
        else if (data.type == RMC)
          type = "rmc";
        else
          type = "inc";
          debug_printf("\nParsed: type: %s\n\
                        hours(%d) minutes(%d) seconds(%d) year(%d) month(%d) day(%d)\n\
                        milliseconds(%d)\n\
                        latitude(%f) longitude(%f)\n\
                        latitude_fixed(%d) longitude_fixed(%d)\n\
                        latitudeDegrees(%f) longitudeDegrees(%f)\n\
                        geoidheight(%f) altitude(%f)\n\
                        speed(%f) angle(%f) magvariation(%f) HDOP(%f)\n\
                        lat(%d) lon(%d) mag(%d)\n\
                        fix(%d)\n\
                        fixquality(%d) satellites(%d)\n",
                        type,
                        data.hour, data.minute, data.seconds, data.year, data.month, data.day,
                        data.milliseconds,
                        data.latitude, data.longitude,
                        data.latitude_fixed, data.longitude_fixed,
                        data.latitudeDegrees, data.longitudeDegrees,
                        data.geoidheight, data.altitude,
                        data.speed, data.angle, data.magvariation, data.HDOP,
                        data.lat, data.lon, data.mag,
                        data.fix,
                        data.fixquality, data.satellites);
        #endif
      }
      _EINT();
    }
  }
}

void usart0_rx (void) __interrupt[UART0RX_VECTOR] 
{
  //while ((IFG2 & UTXIFG1) == 0);
  //TXBUF1 = RXBUF0;

  while (!(IFG1 & URXIFG0));
  g = RXBUF0;

  if (g == '$')
  {
    compteur = 1;
    trame[0] = g;
    while (g != '\n')
    {
      while (!(IFG1 & URXIFG0));
      g = RXBUF0;
      if (compteur < 200)
      {
        trame[compteur] = g;
        compteur++;
      }
    }
    trame[compteur-1] = '\0';
    ok = 1;
  }
}

void usart1_rx (void) __interrupt[UART1RX_VECTOR] 
{
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
    if((P2IES & 0x08));
    else;
  }
  if(P2IFG & 0x10){ // Right
    if((P2IES & 0x10));
    else;
  }

  P2IES ^= P2IFG; // inversion des transition
  P2IFG = 0;
}
