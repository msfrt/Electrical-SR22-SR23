#ifndef GLOBAL_POSITIONING_SUGMA
#define GLOBAL_POSITIONING_SUGMA

#include <SoftwareSerial.h>
#include <Adafruit_GPS.h>
#include <FlexCAN_T4.h>
#include <StateCAN.h>

#include "CAN/CAN1.hpp"

void parseGPRMC(String input){
  int comma_count = 0;
  String smol_str = "";

  for (size_t i = 0; i < input.length(); i++){
    char c = input.charAt(i);

    // String GPS_UTC = 0;
    // int GPS_posStat = 0;
    // String GPS_latitude = 0;
    // String GPS_latitudeDir = 0;
    // String GPS_longitude = 0;
    // String GPS_longitudeDir = 0;
    // String GPS_groundSpeed_knots = 0;
    // String GPS_date = 0;

    if (c == ','){
      comma_count++;

      if (comma_count == 2){ // 2nd field: UTC
        GPS_UTC = (smol_str.toFloat()) * 1000;
        //Serial.println("UTC: " + smol_str);
        //Serial.println((smol_str.toFloat())*1000);
      }

      if (comma_count == 3){ // 3th field: pos stat
        GPS_posStat = smol_str.charAt(0);
        //Serial.println(GPS_posStat);
        //Serial.println("POS STAT: " + smol_str); // if V, then no GPS fix
      }

      if (comma_count == 4){ // 4th field: lat
        GPS_latitude = (smol_str.toFloat()) * 10000;
        //Serial.println("LAT: " + smol_str);
      }

      if (comma_count == 5){ // 5th field: lat dir
        GPS_latitudeDir = smol_str.charAt(0);
        //GPS_latitudeDir = static_cast<int>(smol_str.charAt(0));
        //Serial.println("LAT DIR: " + smol_str);
      }

      if (comma_count == 6){ // 6th field: long
        GPS_longitude = (smol_str.toFloat()) * 10000;
        //Serial.println("LONG: " + smol_str);
      }
      
      if (comma_count == 7){ // 7th field: long dir
        GPS_longitudeDir = smol_str.charAt(0);
        //Serial.println("LONG DIR: " + smol_str);
      }

      if (comma_count == 8){ // 8th field: speed (knots)
        GPS_groundSpeed_knots = (smol_str.toFloat()) * 100;
        //Serial.println("SPD: " + smol_str + " knots");
      }

      if (comma_count == 10){ // 10th field: date
        GPS_date = smol_str.toFloat();
        //Serial.println("DATE: " + smol_str);
      }

      smol_str = "";
    
    } else {
      
      smol_str += c;
    
    }
  }

}

#endif