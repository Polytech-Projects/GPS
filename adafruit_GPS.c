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

// how long are max NMEA lines to parse?
#define MAXLINELENGTH 120

// we double buffer: read one line in and leave one for the main program
volatile char line1[MAXLINELENGTH];
volatile char line2[MAXLINELENGTH];
// our index into filling the current line
volatile uint8_t lineidx=0;
// pointers to the double buffers
volatile char *currentline;
volatile char *lastline;
volatile uint8_t recvdflag;
volatile uint8_t inStandbyMode;

/************** PROTOTYPE FONCTIONS PRIVEE *****************/
static uint8_t parseResponse(char *response);
static uint8_t isAlpha(char c);
static uint8_t isDigit(char c);

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

uint8_t parse(char *nmea, GPS_Data *data) {
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
  // do checksum check

  // first look if we even have one
  if (nmea[strlen(nmea)-4] == '*') {
    sum = parseHex(nmea[strlen(nmea)-3]) * 16;
    sum += parseHex(nmea[strlen(nmea)-2]);
    
    // check checksum 
    for (i=2; i < (strlen(nmea)-4); i++) {
      sum ^= nmea[i];
    }
    if (sum != 0) {
      // bad checksum :(
      return 0;
    }
  }
  // look for a few common sentences
  if (strstr(nmea, "$GPGGA")) {
    // found GGA
    p = nmea;
    // get time
    p = strchr(p, ',')+1;
    timef = atof(p);
    time = timef;
    data->hour = time / 10000;
    data->minute = (time % 10000) / 100;
    data->seconds = (time % 100);

    data->milliseconds = fmod(timef, 1.0) * 1000;

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
      data->latitude_fixed = degree + minutes;
      data->latitude = degree / 100000 + minutes * 0.000006F;
      data->latitudeDegrees = (data->latitude-100*((int)data->latitude/100))/60.0;
      data->latitudeDegrees += (int)data->latitude/100;
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      if (p[0] == 'S') data->latitudeDegrees *= -1.0;
      if (p[0] == 'N') data->lat = 'N';
      else if (p[0] == 'S') data->lat = 'S';
      else if (p[0] == ',') data->lat = 0;
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
      data->longitude_fixed = degree + minutes;
      data->longitude = degree / 100000 + minutes * 0.000006F;
      data->longitudeDegrees = (data->longitude-100*((int)data->longitude/100))/60.0;
      data->longitudeDegrees += (int)data->longitude/100;
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      if (p[0] == 'W') data->longitudeDegrees *= -1.0;
      if (p[0] == 'W') data->lon = 'W';
      else if (p[0] == 'E') data->lon = 'E';
      else if (p[0] == ',') data->lon = 0;
      else return 0;
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      data->fixquality = atoi(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      data->satellites = atoi(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      data->HDOP = atof(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      data->altitude = atof(p);
    }
    
    p = strchr(p, ',')+1;
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      data->geoidheight = atof(p);
    }
    return 1;
  }
  if (strstr(nmea, "$GPRMC")) {
   // found RMC
    p = nmea;

    // get time
    p = strchr(p, ',')+1;
    timef = atof(p);
    time = timef;
    data->hour = time / 10000;
    data->minute = (time % 10000) / 100;
    data->seconds = (time % 100);

    data->milliseconds = fmod(timef, 1.0) * 1000;

    p = strchr(p, ',')+1;
    // Serial.println(p);
    if (p[0] == 'A') 
      data->fix = 1;
    else if (p[0] == 'V')
      data->fix = 0;
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
      data->latitude_fixed = degreeL + minutes;
      data->latitude = degreeL / 100000 + minutes * 0.000006F;
      data->latitudeDegrees = (data->latitude-100*((int)data->latitude/100))/60.0;
      data->latitudeDegrees += (int)data->latitude/100;
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      if (p[0] == 'S') data->latitudeDegrees *= -1.0;
      if (p[0] == 'N') data->lat = 'N';
      else if (p[0] == 'S') data->lat = 'S';
      else if (p[0] == ',') data->lat = 0;
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
      data->longitude_fixed = degree + minutes;
      data->longitude = degree / 100000 + minutes * 0.000006F;
      data->longitudeDegrees = (data->longitude-100*((int)data->longitude/100))/60.0;
      data->longitudeDegrees += (int)data->longitude/100;
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      if (p[0] == 'W') data->longitudeDegrees *= -1.0;
      if (p[0] == 'W') data->lon = 'W';
      else if (p[0] == 'E') data->lon = 'E';
      else if (p[0] == ',') data->lon = 0;
      else return 0;
    }
    // speed
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      data->speed = atof(p);
    }
    
    // angle
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      data->angle = atof(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      fulldate = atof(p);
      data->day = fulldate / 10000;
      data->month = (fulldate % 10000) / 100;
      data->year = (fulldate % 100);
    }
    // we dont parse the remaining, yet!
    return 1;
  }

  return 0;
}

char read(GPS_Data *data) {
  char c = 0;
  
  if (data->paused) return c;

  /* LIRE LE BUFFER ? (READY?), not ready, return c;
     c = BUFFER;
  */

  //Serial.print(c);

//  if (c == '$') {         //please don't eat the dollar sign - rdl 9/15/14
//    currentline[lineidx] = 0;
//    lineidx = 0;
//  }
  if (c == '\n') {
    currentline[lineidx] = 0;

    if (currentline == line1) {
      currentline = line2;
      lastline = line1;
    } else {
      currentline = line1;
      lastline = line2;
    }

    //Serial.println("----");
    //Serial.println((char *)lastline);
    //Serial.println("----");
    lineidx = 0;
    recvdflag = 1;
  }

  currentline[lineidx++] = c;
  if (lineidx >= MAXLINELENGTH)
    lineidx = MAXLINELENGTH-1;

  return c;
}

// Initialization code used by all constructor types
void gpsDataInit(GPS_Data *data) {
  recvdflag   = 0;
  data->paused      = 0;
  lineidx     = 0;
  currentline = line1;
  lastline    = line2;

  data->hour = data->minute = data->seconds = data->year = data->month = data->day =
    data->fixquality = data->satellites = 0; // uint8_t
  data->lat = data->lon = data->mag = 0; // char
  data->fix = 0; // uint8_t
  data->milliseconds = 0; // uint16_t
  data->latitude = data->longitude = data->geoidheight = data->altitude =
    data->speed = data->angle = data->magvariation = data->HDOP = 0.0; // float
}

void sendCommand(const char *str) {
  int i = 0;

  while (str[i] != '\0') {
    while (!(IFG1 & UTXIFG1));
    U1TXBUF = str[i];
    i++;
  }
}

uint8_t newNMEAreceived(void) {
  return recvdflag;
}

void pause(uint8_t p, GPS_Data *data) {
  data->paused = p;
}

char *lastNMEA(void) {
  recvdflag = 0;
  return (char *)lastline;
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

uint8_t waitForSentence(const char *wait4me, uint8_t max) {
  char str[20];
  char *nmea;

  uint8_t i=0;
  while (i < max) {
    if (newNMEAreceived()) { 
      nmea = lastNMEA();
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
  sendCommand(PMTK_LOCUS_STARTLOG);
  recvdflag = 0;
  return waitForSentence(PMTK_LOCUS_STARTSTOPACK, MAXWAITSENTENCE);
}

uint8_t LOCUS_StopLogger(void) {
  sendCommand(PMTK_LOCUS_STOPLOG);
  recvdflag = 0;
  return waitForSentence(PMTK_LOCUS_STARTSTOPACK, MAXWAITSENTENCE);
}

uint8_t LOCUS_ReadStatus(LOCUS_Data *locus_data) {
  char *response;
  uint16_t parsed[10];
  uint8_t i;
  char c;
  sendCommand(PMTK_LOCUS_QUERY_STATUS);
  
  if (! waitForSentence("$PMTKLOG", MAXWAITSENTENCE))
    return 0;

  response = lastNMEA();
  
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
    sendCommand(PMTK_STANDBY);
    //return waitForSentence(PMTK_STANDBY_SUCCESS, MAXWAITSENTENCE);  // don't seem to be fast enough to catch the message, or something else just is not working
    return 1;
  }
}

uint8_t wakeup(void) {
  if (inStandbyMode) {
   inStandbyMode = 0;
    sendCommand("");  // send byte to wake it up
    return waitForSentence(PMTK_AWAKE, MAXWAITSENTENCE);
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