#include "GPS.h"
#ifdef _DEBUG
  #include <__cross_studio_io.h>
#endif
/*#include "adafruit_GPS.h"
#include "ports.h"

GPS_Data data;
volatile char g = 0;
volatile int compteur = 0;
volatile int ok = 0;

void boussolemode(void){
  for (;;)
  {
    if (ok)
    {
      _DINT();
      debug_printf("\nMATRAME %s\n", trame);
      ok = 0;


if (parse(trame, &data))
      {
        #ifdef _DEBUG
        if (data.type == GGA)
        {
          type = "gga";
        }
        else if (data.type == RMC)
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
                        data.hour, data.minute, data.seconds, data.year, data.month, data.day,
                        data.milliseconds,
                        data.latitude, data.longitude,
                        data.latitude_fixed, data.longitude_fixed,
                        data.latitudeDegrees, data.longitudeDegrees,
                        data.geoidheight, data.altitude,
                        data.speed, data.angle, data.magvariation, data.HDOP,
                        data.lat, data.lon, data.mag,
                        data.fix,
                        data.fixquality, data.satellites);
        #endif
      }


_EINT();
    }
    if (newNMEAreceived())
    {
      _DINT();
      #ifdef _DEBUG
        debug_printf("\nNew NMEA: %s\n", lastNMEA());
      #endif
      //MACHING ICI
      _EINT();
    }
  }
}*/
