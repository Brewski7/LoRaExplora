/*
 * Some code modified from https://youtu.be/q2llnFjRSxk?si=uqncJ97KgL861P5E&t=225 , Time Library and TinyGPS++ Library Examples
 */
#define GPS_DEBUG 0

//#include <TimeLib.h>
#include <TinyGPS++.h>
TinyGPSPlus gps;

byte Hour, Minute;

void getGPS(){
  
  while (gpsSerial.available())
  {
    if (gps.encode(gpsSerial.read()))
    {
      locValid = gps.location.isValid();
      
      if(locValid){
        getCoordinates();
      }
      updateLCD_GPS();
      #if(GPS_DEBUG)
        displayLoc();
      #endif

      if(!timeSynced){
        timeValid = gps.time.isValid();
        if(timeValid){
          getTime();
        }
        updateLCD_Time();
        #if(GPS_DEBUG)
          displayTime();
        #endif
      }
    }
  }
}

void getCoordinates(){
  GPSlat = (gps.location.lat());
  GPSlong = (gps.location.lng());
}

void getTime(){
   Hour = gps.time.hour() + offset;
   if(Hour >= 24) Hour = 0;
   Minute = gps.time.minute();
   
   // set the Time to the latest GPS reading
   setTime(Hour, Minute, 0, 14, 8, 2000);
   
   if (timeStatus()!= timeNotSet) {
    if (now() != gpsTime) {
      gpsTime = now();
      timeSynced = 1;
    }
   }
}

void displayLoc(){
  if(locValid){
    comSerial.print ("lattitude: ");
    comSerial.println (GPSlat, 4);
    comSerial.print ("longitude: ");
    comSerial.println (GPSlong, 4);
  }else{
    comSerial.println("INVALID");
  }
}

void displayTime(){
  if(timeValid){
    comSerial.print("time: ");
    if (Hour < 10) comSerial.print(F("0"));
    comSerial.print(Hour);
    comSerial.print(F(":"));
    if (Minute < 10) comSerial.print(F("0"));
    comSerial.println(Minute); 
  }else{
     comSerial.println("INVALID");
  }
}
