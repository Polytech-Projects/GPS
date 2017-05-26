#include "ecran.h"
#include "adafruit_GPS.h"
#include "pad.h"
#include "tools.h"
#include <string.h>
#include <math.h>
#ifdef _DEBUG
  #include <__cross_studio_io.h>
#endif

void sendCommandScreen(const char *str) {
  int i = 0;
  char c;



  while (str[i] != '\0') {
    while (!(IFG2 & UTXIFG1));
    c = parseHex(str[i])*16;
    c += parseHex(str[i+1]);
    U1TXBUF = c;
    i=i+2;
  }
  while (!(IFG2 & UTXIFG1));
    U1TXBUF = '\r';
  while (!(IFG2 & UTXIFG1));
    U1TXBUF = '\n';
  delay(100);
}

void MainMenu(void){
  int pad;
  pad = -1;

  changeBackground("31A9");

  //bleu bleu ciel au centre
  sendCommandScreen("00040004002C0040005400400040002C004000545B93");
  //rectange gris du haut
  sendCommandScreen("FFCE000000000080000C7BEF");
  //rectange gris du bas
  sendCommandScreen("FFCE00000062008000807BEF");

      //TEXTES

  //texte en blanc, backgtound gris
  sendCommandScreen("FF7E7BEF");
  sendCommandScreen("FF7FFFFF");

  //MENU PRINCIPAL
  sendCommandScreen("00064D454E55205052494E434950414C00");

  //Choisissez le mode
  sendCommandScreen("FFE4000D0000");
  sendCommandScreen("000643686F6973697373657A206C65206D6F646500");
  //avec le pad
  sendCommandScreen("FFE4000E0003");
  sendCommandScreen("000661766563206C652070616400");

  //texte en gris
  sendCommandScreen("FF7E31A9");
  sendCommandScreen("FF7F8C71");
  //ligne 4 colonne 2
  sendCommandScreen("FFE400040002");

  //text "enregistrement"
  //debug_printf("envoi du texte enregistrement...");
  sendCommandScreen("0006456E72656769737472656D656E7400");

  // navigation
  sendCommandScreen("FFE400070000");
  sendCommandScreen("00064E6176692D00");
  sendCommandScreen("FFE400080000");
  sendCommandScreen("0006676174696F6E00");

  //boussole
  sendCommandScreen("FFE40007000D");
  sendCommandScreen("0006426F752D00");
  sendCommandScreen("FFE40008000D");
  sendCommandScreen("000673736F6C6500");

  while(pad != 1 && pad != 3 && pad != 4){
    pad = WaitPad();
  }
  switch(pad){
    //haut
    case 1:{
      enregistrementMenu();
      break;
    }
    //gauche
    case 3:{
      navigationMenu();
      break;
    }
    //droit
    case 4:{
      BoussoleMenu();
      break;
    }
  }

}

void clearScreen(void){
  sendCommandScreen("FFD7");
}

void screenReverse(void){
  sendCommandScreen("FF680001");
}

void addtext(int padding, int ligne, char * color, char * str){
  char c[8];

  //decalage en pixels

  //couleur
  strcpy(c,"FF7F");
  strcat(c,color);
  sendCommandScreen(c);

  //ligne

  //texte


}

//la couleur est exprim� en hexa
void changeBackground(char *color){
  int i;
  char c[8];
  strcpy(c,"FF6E");
  strcat(c,color);
  //sendCommandScreen("FF6E31A9");
  delay(500);
  sendCommandScreen(c);
  clearScreen();
}

void BoussoleMenu(void) {
  int pad;
  pad = -1;

  clearScreen();
  //rectange gris du haut
  sendCommandScreen("FFCE000000000080000C7BEF");
  //texte en blanc, backgtound gris
  sendCommandScreen("FF7E7BEF");
  sendCommandScreen("FF7FFFFF");
  //Boussole
  sendCommandScreen("FFE400000005");
  sendCommandScreen("0006424F5553534F4C4500");
  //texte en blanc
  sendCommandScreen("FF7E31A9");
  sendCommandScreen("FF7FFFFF");

  //dessin boussole
  sendCommandScreen("FFCC004000460037FFDF");
  sendCommandScreen("FFCC00400046000A2104");

  //aiguille
  //                     .x1.-y1-.x2.-y2-
  sendCommandScreen("FFD20040004600400011C000");

  /*--------------------*/
  //fonction dans GPS.c
  /*--------------------*/


  //fonction temporaire pour revenir au menu, elle devra
  //etre remplacée par une intérruption à l'intérieur
  //de la fonction présente dans GPS.c

  changeBoussoleOrientation(180);

  while(pad != 0){
    pad = WaitPad();
  }
  switch(pad){
    // centre
    case 0:{
      MainMenu();
      break;
    }
  }

}

void changeBoussoleOrientation(int degres){
  float x,y,xc,yc,x2,y2;
  float radian;
  char xchar[5], ychar[5], command[25];
  // (x,y)  : point que l'on veut obtenir
  // (x',y'): centre de la boussole

  radian = (degres * 3.14159265359)/180.0;

  x = 64.0;
  y = 17.0;
  xc = 64.0;
  yc = 70.0;

  x2 = cosf(radian)*(x-xc) - sinf(radian)*(y-yc) + xc;
  y2 = sinf(radian)*(x-xc) - cosf(radian)*(y-yc) + yc;

  //dessin boussole
  sendCommandScreen("FFCC004000460037FFDF");
  sendCommandScreen("FFCC00400046000A2104");

  //aiguille
  debug_printf("x: %d  y: %d\n", x2,y2);
  decToHex(x, xchar, 5);
  decToHex(y, ychar, 5);


  strcpy(command,"FFD2");
  debug_printf("command: %s\n", command);
  strcat(command,xchar);
  debug_printf("command: %s\n", command);
  strcat(command,ychar);
  debug_printf("command: %s\n", command);
  strcat(command, "C000");
  debug_printf("%s\n", command);
  sendCommandScreen(command);

}

void navigationMenu(void){
  int pad;
  pad = -1;
  clearScreen();
  //rectange gris du haut
  sendCommandScreen("FFCE000000000080000C7BEF");
  //texte en blanc, backgtound gris
  sendCommandScreen("FF7E7BEF");
  sendCommandScreen("FF7FFFFF");
  //Boussole
  sendCommandScreen("FFE400000005");
  sendCommandScreen("00064E415649474154494F4E00");
  //texte en blanc
  sendCommandScreen("FF7E31A9");
  sendCommandScreen("FF7FFFFF");

  //mode indisponible
  sendCommandScreen("FFE400070000");
  sendCommandScreen("00064D4F444520494E444953504F4E49424C4500");

  //cliquez sur le pad
  sendCommandScreen("FFE4000A0000");
  sendCommandScreen("0006636C697175657A20737572206C652070616400");

  //pour quitter
  sendCommandScreen("FFE4000B0000");
  sendCommandScreen("0006706F7572207175697474657200");

  while(pad != 0){
    pad = WaitPad();
  }
  switch(pad){
    // centre
    case 0:{
      MainMenu();
      break;
    }
  }

}

void enregistrementMenu(void){
  int pad;
  pad = -1;

  clearScreen();
  //rectange gris du haut
  sendCommandScreen("FFCE000000000080000C7BEF");
  //texte en blanc, backgtound gris
  sendCommandScreen("FF7E7BEF");
  sendCommandScreen("FF7FFFFF");
  //Boussole
  sendCommandScreen("FFE400000001");
  sendCommandScreen("0006454E52454749535452454D454E5400");
  //texte en blanc
  sendCommandScreen("FF7E31A9");
  sendCommandScreen("FF7FFFFF");

  //mode indisponible
  sendCommandScreen("FFE400070000");
  sendCommandScreen("00064D4F444520494E444953504F4E49424C4500");

  //cliquez sur le pad
  sendCommandScreen("FFE4000A0000");
  sendCommandScreen("0006636C697175657A20737572206C652070616400");

  //pour quitter
  sendCommandScreen("FFE4000B0000");
  sendCommandScreen("0006706F7572207175697474657200");

  while(pad != 0){
    pad = WaitPad();
  }
  switch(pad){
    // centre
    case 0:{
      MainMenu();
      break;
    }
  }

}
