void delay(int delay);
char * chartoHex(char * c);
void decToHex(int decimal, char * hexadecimalnum, int taille_tab);
//boucle jusqu'a que le pad soit actionné
// 0 : centre
// 1 : haut
// 2 : bas
// 3 : gauche
// 4 : droite
int WaitPad(void);
