
#ifdef _DEBUG
  #include <__cross_studio_io.h>
#endif
#include <msp430x16x.h>
#include "adafruit_GPS.h"
#include "ports.h"
#include "uart.h"
#include "led.h"
#include "pad.h"
#include "tools.h"
#include "ecran.h"

GPS_Data data;

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
    for (i = 0xFF; i > 0; i--);           // Time for flag to set
  }
  while ((IFG1 & OFIFG) != 0);          // OSCFault flag still set?  

  BCSCTL2 |= SELM1+SELS;                // MCLK = SMCLK = XT2 (safe)

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
    if (newNMEAreceived())
    {
      _DINT();
      #ifdef _DEBUG
        debug_printf("\nNew NMEA: %s\n", lastNMEA());
        debug_printf("\nNew NMEA: %s\n", lastNMEA());
      #endif
      if (parse(lastNMEA(), &data))
      {
        #ifdef _DEBUG
          debug_printf("\nParsed: hours(%d) minutes(%d) seconds(%d) year(%d) month(%d) day(%d)\
                        milliseconds(%d)\n\
                        latitude(%f) longitude(%f)\n\
                        latitude_fixed(%d) longitude_fixed(%d)\n\
                        latitudeDegrees(%f) longitudeDegrees(%f)\n\
                        geoidheight(%f) altitude(%f)\n\
                        speed(%f) angle(%f) magvariation(%f) HDOP(%f)\n\
                        lat(%d) lon(%d) mag(%d)\n\
                        fix(%d)\n\
                        fixquality(%d) satellites(%d)\n",
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

void menu(void){
  
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
        /*
        if(P1OUT==0 && bs!=0x1F){
          P1OUT=~bs;
        }else{
          P1OUT=0;
        }*/
      }
      delay(10000);
    }
    lbs= bs;

  }
}
void usart0_rx (void) __interrupt[UART0RX_VECTOR] 
{
  //while ((IFG2 & UTXIFG1) == 0);
  //TXBUF1 = RXBUF0;
  read();
}

void usart1_rx (void) __interrupt[UART1RX_VECTOR] 
{
  //while ((IFG1 & UTXIFG0) == 0);
  //TXBUF0 = RXBUF1;
}
