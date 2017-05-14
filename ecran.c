#include "ecran.h"
#include "adafruit_GPS.h"
#include "pad.h"
#include "tools.h"
#include <string.h>
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

    debug_printf("\nECRAN: command sent.\n");
}

void MainMenu(void){
  int pad;

  changeBackground("31A9");

  //bleu bleu ciel au centre
  sendCommandScreen("00040004002C004000540040002C0018002C00405B93");


      //TEXTES

  //texte en noir
  sendCommandScreen("FF7E31A9");
  sendCommandScreen("FF7FFFFF");

  //ligne 1 colonne 5
  sendCommandScreen("FFE400010002");
  //text "enregistrement"
  debug_printf("envoi du texte enregistrement...");
  sendCommandScreen("0006456E72656769737472656D656E7400");
  // navigation
  sendCommandScreen("FFE400050000");
  sendCommandScreen("00064E6176692D00");
  sendCommandScreen("FFE400060000");
  sendCommandScreen("0006676174696F6E00");
  //boussole
  sendCommandScreen("FFE40005000D");
  sendCommandScreen("0006426F752D00");
  sendCommandScreen("FFE40006000D");
  sendCommandScreen("000673736F6C6500");

  //Choisissez le mode
  sendCommandScreen("FFE4000D0000");
  sendCommandScreen("000643686F6973697373657A206C65206D6F646500");
  //avec le pad
  sendCommandScreen("FFE4000E0003");
  sendCommandScreen("000661766563206C652070616400");

  while(pad!=4){
    pad = WaitPad();
  }
  BoussoleMenu();

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
  clearScreen();
  sendCommandScreen("FFE400000005");
  sendCommandScreen("0006426F7573736F6C6500");
  //boussolemode();
}
