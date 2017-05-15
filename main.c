
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


GPS_Data data;
volatile char g = 0;
char trame[200];
volatile int compteur = 0;
volatile int ok = 0;
char *type; // A VIRER

void main(void)
{
  unsigned int i;
  int j;
  /* Le watchdog doit ?re d?activ?pour ne pas provoquer de red?arrage */
  WDTCTL = WDTPW + WDTHOLD;             // Stop WDT (watch dog timer)
  BCSCTL1 &= ~XT2OFF;                   // XT2on

  do
  {
    IFG1 &= ~OFIFG;                       // Clear OSCFault flag
    IFG2 &= ~OFIFG;                       // Clear OSCFault flag
    for (i = 0xFF; i > 0; i--);           // Time for flag to set
  }
  while ((IFG1 & OFIFG) != 0);          // OSCFault flag still set?

  BCSCTL2 |= SELM1+SELS;                // MCLK = SMCLK = XT2 (safe)

  initPorts();
  initUart0();
  initUart1();

  // Reset de l'�cran
  P4OUT = 0x04; // Reset �ran
  for (i = 0xFF; i > 0; i--);
  P4OUT = 0x06; // Remise �ran

  P1OUT = 0x00; // Eteint les leds (chiant qd allumé)

  gpsDataInit(&data);
  setCmdSwitch(GPS);
  //setCmdSwitch(USB);
  setGPS(1);
  wakeup();

  sendCommand(PMTK_SET_BAUD_9600);
  sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  sendCommand(PMTK_SET_NMEA_OUTPUT_OFF);
  sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);

  _EINT(); // Interrupt ON

  led_centre(1);
  delay(500);
  //reverse screen
  screenReverse();
  // stop scroll
  sendCommandScreen("000C0000");
  MainMenu();

  for(;;);
  /*for (;;)
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
    if (newNMEAreceived())
    {
      _DINT();
      #ifdef _DEBUG
        debug_printf("\nNew NMEA: %s\n", lastNMEA());
      #endif
      //MACHING ICI
      _EINT();
    }
  }*/
}

void usart0_rx (void) __interrupt[UART0RX_VECTOR]
{
  //POUR QUE LECRAN MARCHE EN USB
  //-----------------------------
  //while ((IFG2 & UTXIFG1) == 0);
  //TXBUF1 = RXBUF0;
  //-----------------------------
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
  //POUR QUE LECRAN MARCHE EN USB
  //-----------------------------
  //while ((IFG1 & UTXIFG0) == 0);
  //TXBUF0 = RXBUF1;
}

void test(void){
  int bs=0x1F,lbs=0x1F,debug;

  //setup
  WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer
  P1DIR |= 0x1F;                        // Set P1.0 to output direction
  P2DIR |= 0x00;
  P1OUT=0x00;

  for (;;)
  {
    bs=P2IN;
    if(bs != lbs){
      //si le bouton n'est pas a la position nulle
      if(bs!=PAD_NULL){
        //allumer toutes les leds en cliquant au centre
        if(bs==PAD_CENTRE){
          if(P1OUT != TOUTES_LED){
            all_led(1);
          }
          else all_led(0);
        }
        //allumer la led du haut avec le bouton haut
        if(bs==PAD_HAUT){
          if(P1OUT!=LED_HAUT){
            led_haut(1);
          } else {
            led_haut(0);
          }
        }
        //allumer la led de gauche ...
        if(bs==PAD_GAUCHE){
          if(P1OUT!=LED_GAUCHE){
            led_gauche(1);
          } else {
            led_gauche(0);
          }
        }
        //allumer la led de droite
        if(bs==PAD_DROIT){
          if(P1OUT!=LED_DROITE){
            led_droite(1);
          } else {
            led_droite(0);
          }
        }
        //allumer la led du bas
        if(bs==PAD_BAS){
          if(P1OUT!=LED_BAS){
            led_bas(1);
          } else {
            led_bas(0);
          }
        }
      }
      delay(30);
    }
    lbs= bs;

  }
}
