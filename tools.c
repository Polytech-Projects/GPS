#include "tools.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <msp430x16x.h>
#include "pad.h"

void delay(int t){
  int i, j;
  for(i=0;i<t;++i)
    for(j=0;j<i/2;++j);
}

char * chartoHex(char * c){
  char text[18], result[36];
  int val,i;

  for(i=0;i<18;i++){
    text[i]=c[i];
  }
  text[18]='\0';

  val = (int) strtol(text, NULL, 0); // error checking omitted for brevity
  sprintf(result, "%x", val);
  debug_printf("%s",result);

  return result;
}

char * decToHex(int decimal){
    long quotient, remainder;
    int i, j = 0;
    char hexadecimalnum[100];

    quotient = decimal;

    while (quotient != 0)
    {
        remainder = quotient % 16;
        if (remainder < 10)
            hexadecimalnum[j++] = 48 + remainder;
        else
            hexadecimalnum[j++] = 55 + remainder;
        quotient = quotient / 16;
    }

    return hexadecimalnum;
}

int WaitPad(void){
  int bs=0x1F,lbs=0x1F,pad;

  //setup
  WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer
  P1DIR |= 0x1F;                        // Set P1.0 to output direction
  P2DIR |= 0x00;
  P1OUT=0x00;
  pad = NULL;

  while(pad==NULL){
    bs=P2IN;
    if(bs != lbs){
      //si le bouton n'est pas a la position nulle
      if(bs!=PAD_NULL){
        //allumer toutes les leds en cliquant au centre
        if(bs==PAD_CENTRE){
          pad = 0;
        }
        //allumer la led du haut avec le bouton haut
        if(bs==PAD_HAUT){
          pad = 1;
        }
        //allumer la led de gauche ...
        if(bs==PAD_GAUCHE){
          pad = 3;
        }
        //allumer la led de droite
        if(bs==PAD_DROIT){
          pad = 4;
        }
        //allumer la led du bas
        if(bs==PAD_BAS){
          pad = 2;
        }
      }
      delay(30);
    }
    lbs= bs;

  }
  return pad;
}
