#include <msp430x16x.h>
void clearScreen(void);
void changeBackground(char *color);
void sendCommandScreen(const char *str);
void MainMenu(void);
void screenReverse(void);
void delay(int i);
void BoussoleMenu(void);
void enregistrementMenu(void);
void navigationMenu(void);
void changeBoussoleOrientation(int degres);

static char clear[] = {0xFF,0xD7};
static char landscapeR[] = {0xFF,0x68, 0x00,0x01};
static char fondBleu[] = {0xFF,0xCE, 0x00,0x00, 0x00,0x00, 0x00,0x80, 0x00,0x80, 0x00,0x4D};
static char rondBleuClair[] = {0xFF,0xCC, 0x00,0x40, 0x00,0x40, 0x00,0x32, 0x04,0xB9};
static char txtHeight[] = {0xFF,0x7B, 0x00,0x02};
static char txtWidth[] = {0xFF,0x7C, 0x00,0x02};
static char putStr[] = {0x00,0x06,'G','P','S',0x00};

static char blanc[]={0xff,0xff};
static char noir[]={0x00,0x00};
static char blert[]={0x0d,0x4b};
static char vert[]={0x05,0xe4};
static char bleuclair[]={0x1c,0xbd};
static char rouge[]={0xf8,0x00};

static char fondviolet[]={0xb1,0x38};
static char tourviolet[]={0xe8,0x13};
static char fondbleu[]={0x19,0x71};
static char tourbleu[]={0x1e,0xde};
static char fondorange[]={0xf3,0xa0};
static char tourorange[]={0xf7,0xa0};
static char fondgris[]={0x6b,0x6d};
static char tourgris[]={0xc6,0x18};

void clearScreen(void);
void drawRectangle(char x1,char y1,char x2, char y2, char *couleur);
void screen_ack(void);
