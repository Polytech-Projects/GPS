#define LED_HAUT 0x04
#define LED_BAS 0x02
#define LED_GAUCHE 0x10
#define LED_DROITE 0x08
#define LED_CENTRE 0x01
#define TOUTES_LED 0x1f

void led_haut(int etat);
void led_bas(int etat);
void led_gauche(int etat);
void led_droite(int etat);
void led_centre(int etat);
void all_led(int etat);
