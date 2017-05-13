#include "ecran.h"

void clearScreen(void){
	sendCharTX1(0xff);
	sendCharTX1(0xd7);
	waitACK_RX_1();
}

void drawRectangle(char x1,char y1,char x2, char y2, char *couleur){

	sendCharTX1(0xff);
	sendCharTX1(0xcf);
	sendCharTX1(0x00);
	sendCharTX1(x1);
	sendCharTX1(0x00);
	sendCharTX1(y1);
	sendCharTX1(0x00);
	sendCharTX1(x2);
	sendCharTX1(0x00);
	sendCharTX1(y2);
	sendCharTableTX1(couleur,2);
	waitACK_RX_1();

}

