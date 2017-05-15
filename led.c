#include "led.h"
#include  <msp430x16x.h>

void led_haut(int etat){
  if(etat) P1OUT = P1OUT | LED_HAUT;
  else P1OUT &= ~LED_HAUT;
}
void led_bas(int etat){
  if(etat) P1OUT = P1OUT | LED_BAS;
  else P1OUT &= ~LED_BAS;
}
void led_gauche(int etat){
  if(etat) P1OUT = P1OUT | LED_GAUCHE;
  else P1OUT &= ~LED_GAUCHE;
}
void led_droite(int etat){
  if(etat) P1OUT = P1OUT | LED_DROITE;
  else P1OUT &= ~LED_DROITE;
}
void led_centre(int etat){
  if(etat) P1OUT = P1OUT | LED_CENTRE;
  else P1OUT &= ~LED_CENTRE;
}
void all_led(int etat){
  if(etat){
    led_haut(1); led_gauche(1); led_droite(1); led_bas(1); led_centre(1);
  } else {
    led_haut(0); led_gauche(0); led_droite(0); led_bas(0); led_centre(0);
  }
}
