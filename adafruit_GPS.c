/***********************************
This is our GPS library

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.
BSD license, check license.txt for more information
All text above must be included in any redistribution
****************************************/

#include <Adafruit_GPS.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <msp430x16x.h>
#ifdef _DEBUG
  #include <__cross_studio_io.h>
#endif

// how long are max NMEA lines to parse
#define MAXLINELENGTH 200

/************ DONNES GLOBAL *************/
GPS_Data *old_data = 0, *last_data = 0;
volatile uint8_t nmea_received = 0;

/************ DONNES STATIC *************/
static GPS_Data data_1, data_2;
static uint8_t inStandbyMode;
static volatile char g = 0;
static char trame[200]; //trame
static volatile uint8_t compteur = 0;
/************** PROTOTYPE FONCTIONS PRIVEE *****************/
static uint8_t parseResponse(char *response);
static uint8_t isAlpha(char c);
static uint8_t isDigit(char c);
static float degToRad(float x); 
static float radToDeg(float x);
static void gpsDataInit(GPS_Data *data);

void setGPS(uint8_t on)
{
  if (on)
  {
    P4OUT |= 0x01;                        // P4.0 HIGH
  }
  else
  {
    P4OUT &= 0xFE;                        // P4.2 LOW
  }
}

void gps_init(void)
{
  gpsDataInit(&data_1);
  gpsDataInit(&data_2);
  old_data = &data_1;
  last_data = &data_2;
  inStandbyMode = 0;
}

void gps_read(void)
{
  while (!(IFG1 & URXIFG0));
  g = RXBUF0;

  if (g == '$')
  {
    compteur = 1;
    trame[0] = g;
    while (g != '\n')
    {
      while (!(IFG1 & URXIFG0));
      g = RXBUF0;
      if (compteur < 200)
      {
        trame[compteur] = g;
        compteur++;
      }
    }
    trame[compteur-1] = '\0';
    nmea_received = 1;
  }
}

void gps_debug_trame(void)
{
  debug_printf("\nMATRAME %s\n", trame);
}
void gps_debug_parse(void)
{
  char *type;

  if (last_data->type == GGA)
  {
    type = "gga";
  }
  else if (last_data->type == RMC)
    type = "rmc";
  else
    type = "inc";
    debug_printf("\nParsed: type: %s\n\
                  hours(%d) minutes(%d) seconds(%d) year(%d) month(%d) day(%d)\n\
                  milliseconds(%d)\n\
                  latitude(%f) longitude(%f)\n\
                  latitude_fixed(%d) longitude_fixed(%d)\n\
                  latitudeDegrees(%f) longitudeDegrees(%f)\n\
                  geoidheight(%f) altitude(%f)\n\
                  speed(%f) angle(%f) magvariation(%f) HDOP(%f)\n\
                  lat(%d) lon(%d) mag(%d)\n\
                  fix(%d)\n\
                  fixquality(%d) satellites(%d)\n",
                  type,
                  last_data->hour, last_data->minute, last_data->seconds, last_data->year, last_data->month, last_data->day,
                  last_data->milliseconds,
                  last_data->latitude, last_data->longitude,
                  last_data->latitude_fixed, last_data->longitude_fixed,
                  last_data->latitudeDegrees, last_data->longitudeDegrees,
                  last_data->geoidheight, last_data->altitude,
                  last_data->speed, last_data->angle, last_data->magvariation, last_data->HDOP,
                  last_data->lat, last_data->lon, last_data->mag,
                  last_data->fix,
                  last_data->fixquality, last_data->satellites);
}

uint8_t gps_parse(void) {
  uint16_t sum; // checksum
  uint8_t i; // compteur boucle
  int32_t degree;
  long degreeL;
  long minutes;
  char degreebuff[10];
  char *p;
  float timef; // temps en float
  uint32_t time; // temps en int
  uint32_t fulldate; // date complète

  nmea_received = 0;
  if (old_data == &data_1)
  {
    old_data = &data_2;
    last_data = &data_1;
  }
  else
  {
    old_data = &data_1;
    last_data = &data_2;
  }

  // do checksum check
  // first look if we even have one
  // N*41C
  if (trame[strlen(trame)-4] == '*') {
    #ifdef _DEBUG
      debug_printf("parse: checksum available (got '*')\n");
      debug_printf("parse: checksum %c\n", trame[strlen(trame)-3]);
      debug_printf("parse: checksum %c\n", trame[strlen(trame)-2]);
    #endif
    sum = parseHex(trame[strlen(trame)-3]) * 16;
    sum += parseHex(trame[strlen(trame)-2]);
    
    // check checksum 
    for (i=1; i < (strlen(trame)-4); i++) {
      sum ^= trame[i];
    }
    if (sum != 0) {
      // bad checksum :(
      #ifdef _DEBUG
        debug_printf("parse: bad checksum\n");
      #endif
      return 0;
    }
    #ifdef _DEBUG
      debug_printf("parse: good checksum\n");
    #endif
  }
  // look for a few common sentences
  if (strstr(trame, "$GPGGA")) {
    // found GGA
    /*
     *
     * UTC position (heure?)
     * Latitude (ddmm.mmmm)
     * Nord/Sud indicateur
     * longitude (dddmm.mmmm)
     * Est/West
     * Positin fix (0,1,2,3|pas valide, reste valide)
     * HDOP (Horizontal Dilution of Precision)
     * MLS Altitude en mètre
     * Units
     */
    last_data->type = GGA;
    p = trame;
    // get time
    p = strchr(p, ',')+1;
    timef = atof(p);
    time = timef;
    last_data->hour = time / 10000;
    last_data->minute = (time % 10000) / 100;
    last_data->seconds = (time % 100);

    last_data->milliseconds = fmod(timef, 1.0) * 1000;

    // parse out latitude
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      strncpy(degreebuff, p, 2);
      p += 2;
      degreebuff[2] = '\0';
      degree = atol(degreebuff) * 10000000;
      strncpy(degreebuff, p, 2); // minutes
      p += 3; // skip decimal point
      strncpy(degreebuff + 2, p, 4);
      degreebuff[6] = '\0';
      minutes = 50 * atol(degreebuff) / 3;
      last_data->latitude_fixed = degree + minutes;
      last_data->latitude = degree / 100000 + minutes * 0.000006F;
      last_data->latitudeDegrees = (last_data->latitude-100*((int)last_data->latitude/100))/60.0;
      last_data->latitudeDegrees += (int)last_data->latitude/100;
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      if (p[0] == 'S') last_data->latitudeDegrees *= -1.0;
      if (p[0] == 'N') last_data->lat = 'N';
      else if (p[0] == 'S') last_data->lat = 'S';
      else if (p[0] == ',') last_data->lat = 0;
      else return 0;
    }
    
    // parse out longitude
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      strncpy(degreebuff, p, 3);
      p += 3;
      degreebuff[3] = '\0';
      degree = atol(degreebuff) * 10000000;
      strncpy(degreebuff, p, 2); // minutes
      p += 3; // skip decimal point
      strncpy(degreebuff + 2, p, 4);
      degreebuff[6] = '\0';
      minutes = 50 * atol(degreebuff) / 3;
      last_data->longitude_fixed = degree + minutes;
      last_data->longitude = degree / 100000 + minutes * 0.000006F;
      last_data->longitudeDegrees = (last_data->longitude-100*((int)last_data->longitude/100))/60.0;
      last_data->longitudeDegrees += (int)last_data->longitude/100;
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      if (p[0] == 'W') last_data->longitudeDegrees *= -1.0;
      if (p[0] == 'W') last_data->lon = 'W';
      else if (p[0] == 'E') last_data->lon = 'E';
      else if (p[0] == ',') last_data->lon = 0;
      else return 0;
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      last_data->fixquality = atoi(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      last_data->satellites = atoi(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      last_data->HDOP = atof(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      last_data->altitude = atof(p);
    }
    
    p = strchr(p, ',')+1;
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      last_data->geoidheight = atof(p);
    }
    return 1;
  }
  if (strstr(trame, "$GPRMC")) {
   // found RMC
    last_data->type = RMC;
    p = trame;

    // get time
    p = strchr(p, ',')+1;
    timef = atof(p);
    time = timef;
    last_data->hour = time / 10000;
    last_data->minute = (time % 10000) / 100;
    last_data->seconds = (time % 100);

    last_data->milliseconds = fmod(timef, 1.0) * 1000;

    p = strchr(p, ',')+1;
    // Serial.println(p);
    if (p[0] == 'A') 
      last_data->fix = 1;
    else if (p[0] == 'V')
      last_data->fix = 0;
    else
      return 0;

    // parse out latitude
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      strncpy(degreebuff, p, 2);
      p += 2;
      degreebuff[2] = '\0';
      degreeL = atol(degreebuff) * 10000000;
      strncpy(degreebuff, p, 2); // minutes
      p += 3; // skip decimal point
      strncpy(degreebuff + 2, p, 4);
      degreebuff[6] = '\0';
      minutes = 50 * atol(degreebuff) / 3;
      last_data->latitude_fixed = degreeL + minutes;
      last_data->latitude = degreeL / 100000 + minutes * 0.000006F;
      last_data->latitudeDegrees = (last_data->latitude-100*((int)last_data->latitude/100))/60.0;
      last_data->latitudeDegrees += (int)last_data->latitude/100;
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      if (p[0] == 'S') last_data->latitudeDegrees *= -1.0;
      if (p[0] == 'N') last_data->lat = 'N';
      else if (p[0] == 'S') last_data->lat = 'S';
      else if (p[0] == ',') last_data->lat = 0;
      else return 0;
    }
    
    // parse out longitude
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      strncpy(degreebuff, p, 3);
      p += 3;
      degreebuff[3] = '\0';
      degree = atol(degreebuff) * 10000000;
      strncpy(degreebuff, p, 2); // minutes
      p += 3; // skip decimal point
      strncpy(degreebuff + 2, p, 4);
      degreebuff[6] = '\0';
      minutes = 50 * atol(degreebuff) / 3;
      last_data->longitude_fixed = degree + minutes;
      last_data->longitude = degree / 100000 + minutes * 0.000006F;
      last_data->longitudeDegrees = (last_data->longitude-100*((int)last_data->longitude/100))/60.0;
      last_data->longitudeDegrees += (int)last_data->longitude/100;
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      if (p[0] == 'W') last_data->longitudeDegrees *= -1.0;
      if (p[0] == 'W') last_data->lon = 'W';
      else if (p[0] == 'E') last_data->lon = 'E';
      else if (p[0] == ',') last_data->lon = 0;
      else return 0;
    }
    // speed
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      last_data->speed = atof(p);
    }
    
    // angle
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      last_data->angle = atof(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      fulldate = atof(p);
      last_data->day = fulldate / 10000;
      last_data->month = (fulldate % 10000) / 100;
      last_data->year = (fulldate % 100);
    }
    // we dont parse the remaining, yet!
    return 1;
  }

  return 0;
}

void gps_send_command(const char *str) {
  int i = 0;

  while (str[i] != '\0') {
    while (!(IFG1 & UTXIFG0));
    U0TXBUF = str[i];
    i++;
  }
  while (!(IFG1 & UTXIFG0));
    U0TXBUF = '\r';
  while (!(IFG1 & UTXIFG0));
    U0TXBUF = '\n';
  #ifdef _DEBUG
    debug_printf("\nGPS: command sent.\n", RXBUF0);
  #endif
}

// read a Hex value and return the decimal equivalent
uint8_t parseHex(char c) {
    if (c < '0')
      return 0;
    if (c <= '9')
      return c - '0';
    if (c < 'A')
       return 0;
    if (c <= 'F')
       return (c - 'A')+10;
    // if (c > 'F')
    return 0;
}

float calculDistance(float la1, float lo1, float la2, float lo2)
{
  float radius = 6317e3; // moyenne du rayon de la terre en mètre
  float deltaLat, deltaLon, a, c;

  // Calcul des delta avant conversion en radian
  deltaLat = degToRad(la2-la1);
  deltaLon = degToRad(lo2-lo1);

  // Besoin de tout avoir en radian
  la1 = degToRad(la1);
  lo1 = degToRad(lo1);
  la2 = degToRad(la2);
  lo2 = degToRad(lo2);

  // Racine carré de la moitié de la longueur de l'accord entre les 2 points
  a = sin(deltaLat/2)*sin(deltaLat/2) + cos(la1)*cos(la2) * sin(deltaLon/2)*sin(deltaLon/2);
  // Distance angulaire en radian
  c = 2*atan2(sqrt(a), sqrt(1-a));
  return radius*c;
}

uint16_t calculOrientation(float x1, float y1, float x2, float y2)
{
  return atan2((y1-y2), (x1-x2)) * 180 / 3.14159265;
}

uint8_t waitForSentence(const char *wait4me, uint8_t max) {
  char str[20];
  char *nmea;

  uint8_t i=0;
  while (i < max) {
    if (1) { // TODO l'adapter, plus de newNMEAreceived
      //nmea = lastNMEA();
      strncpy(str, nmea, 20);
      str[19] = 0;
      i++;

      if (strstr(str, wait4me))
	      return 1;
    }
  }

  return 0;
}

uint8_t LOCUS_StartLogger(void) {
  gps_send_command(PMTK_LOCUS_STARTLOG);
  return waitForSentence(PMTK_LOCUS_STARTSTOPACK, MAXWAITSENTENCE);
}

uint8_t LOCUS_StopLogger(void) {
  gps_send_command(PMTK_LOCUS_STOPLOG);
  return waitForSentence(PMTK_LOCUS_STARTSTOPACK, MAXWAITSENTENCE);
}

uint8_t LOCUS_ReadStatus(LOCUS_Data *locus_data) {
  char *response;
  uint16_t parsed[10];
  uint8_t i;
  char c;
  gps_send_command(PMTK_LOCUS_QUERY_STATUS);
  
  if (! waitForSentence("$PMTKLOG", MAXWAITSENTENCE))
    return 0;

  //response = lastNMEA();
  
  for (i=0; i<10; i++) parsed[i] = -1;
  
  response = strchr(response, ',');
  for (i=0; i<10; i++) {
    if (!response || (response[0] == 0) || (response[0] == '*')) 
      break;
    response++;
    parsed[i]=0;
    while ((response[0] != ',') && 
	   (response[0] != '*') && (response[0] != 0)) {
      parsed[i] *= 10;
      c = response[0];
      if (isDigit(c))
        parsed[i] += c - '0';
      else
        parsed[i] = c;
      response++;
    }
  }
  locus_data->LOCUS_serial = parsed[0];
  locus_data->LOCUS_type = parsed[1];
  if (isAlpha(parsed[2])) {
    parsed[2] = parsed[2] - 'a' + 10; 
  }
  locus_data->LOCUS_mode = parsed[2];
  locus_data->LOCUS_config = parsed[3];
  locus_data->LOCUS_interval = parsed[4];
  locus_data->LOCUS_distance = parsed[5];
  locus_data->LOCUS_speed = parsed[6];
  locus_data->LOCUS_status = !parsed[7];
  locus_data->LOCUS_records = parsed[8];
  locus_data->LOCUS_percent = parsed[9];

  return 1;
}

// Standby Mode Switches
uint8_t standby(void) {
  if (inStandbyMode) {
    return 0;  // Returns 0 if already in standby mode, so that you do not wake it up by sending commands to GPS
  }
  else {
    inStandbyMode = 1;
    gps_send_command(PMTK_STANDBY);
    //return waitForSentence(PMTK_STANDBY_SUCCESS, MAXWAITSENTENCE);  // don't seem to be fast enough to catch the message, or something else just is not working
    return 1;
  }
}

uint8_t wakeup(void) {
  if (inStandbyMode) {
   inStandbyMode = 0;
    gps_send_command("");  // send byte to wake it up
    return 1; //waitForSentence(PMTK_AWAKE, MAXWAITSENTENCE);
  }
  else {
      return 0;  // Returns 0 if not in standby mode, nothing to wakeup
  }
}

static uint8_t isAlpha(char c)
{
  return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') ? 1 : 0;
}
static uint8_t isDigit(char c)
{
  return (c >= '0' && c <= '9') ? 1 : 0;
}

static float degToRad(float x)
{
    return x / 180 * 3.14159265;
}
 
static float radToDeg(float x)
{
    return x / 3.14159265 * 180;
}

// Initialization code used by all constructor types
static void gpsDataInit(GPS_Data *data) {
  data->type = INCONNU;
  data->hour = data->minute = data->seconds = data->year = data->month = data->day =
    data->fixquality = data->satellites = 0; // uint8_t
  data->lat = data->lon = data->mag = 0; // char
  data->fix = 0; // uint8_t
  data->milliseconds = 0; // uint16_t
  data->latitude = data->longitude = data->geoidheight = data->altitude =
    data->speed = data->angle = data->magvariation = data->HDOP = 0.0; // float
}
