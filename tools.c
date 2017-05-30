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

void decToHex(int decimal, char * hexadecimalnum, int taille_tab){
    long quotient, remainder, complethexa;
    int i, j = 0;
    char tab_tmp[50];

    quotient = decimal;
    complethexa=0;

    while (quotient != 0)
    {
        remainder = quotient % 16;
        if (remainder < 10)
            hexadecimalnum[j++] = 48 + remainder;
        else
            hexadecimalnum[j++] = 55 + remainder;
        quotient = quotient / 16;
    }
    j++;
    debug_printf("%s\n", hexadecimalnum);
    hexadecimalnum[j]='\0';

    if((j-1)%4<4){
        complethexa = 4-((j-1)%4);
    }
    for(i=j-1; i<j+(complethexa-1);i++){
        hexadecimalnum[i]='0';
    }
    j=j+complethexa;
    hexadecimalnum[j]='\0';

    for(i=0; i<taille_tab; i++){
        tab_tmp[i] = hexadecimalnum[i];
    }
    for(i=0; i<j; i++){
        hexadecimalnum[i]=tab_tmp[j-(2+i)];
    }
    debug_printf("%s\n", hexadecimalnum);
}

int WaitPad(void){
  int bs=0x1F,lbs=0x1F,pad;

  //setup
  //WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer
  //P1DIR |= 0x1F;                        // Set P1.0 to output direction
  //P2DIR |= 0x00;
  P1OUT=0x00;
  pad = -1;

  while(pad==-1){
    bs=P2IN;
    if(bs != lbs){
      //si le bouton n'est pas a la position nulle
      if(bs!=PAD_NULL){
        //allumer toutes les leds en cliquant au centre
        if(bs==PAD_CENTRE){
          pad = 0;
        }
        //allumer la led du haut avec le bouton haut
        else if(bs==PAD_HAUT){
          pad = 1;
        }
        //allumer la led du bas
        else if(bs==PAD_BAS){
          pad = 2;
        }
        //allumer la led de gauche ...
        else if(bs==PAD_GAUCHE){
          pad = 3;
        }
        //allumer la led de droite
        else if(bs==PAD_DROIT){
          pad = 4;
        }
      }
      delay(30);
    }
    lbs= bs;

  }
  return pad;
}
