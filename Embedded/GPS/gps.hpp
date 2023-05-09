#ifndef GLOBAL_POSITIONING_SUGMA
#define GLOBAL_POSITIONING_SUGMA

#include <SoftwareSerial.h>
#include <Adafruit_GPS.h>
#include <FlexCAN_T4.h>
#include <StateCAN.h>

#include <TinyGPS.h>

#include "CAN/CAN1.hpp"

#define PMTK_SET_NMEA_UPDATE_1HZ  "$PMTK220,1000*1F"
//#define PMTK_SET_NMEA_UPDATE_5HZ  "$PMTK220,200*2C"
#define PMTK_SET_NMEA_UPDATE_10HZ "$PMTK220,100*2F"

TinyGPS gps;

// Connect the GPS Power pin to 5V
// Connect the GPS Ground pin to ground
// Connect the GPS TX (transmit) pin to Digital 7
// Connect the GPS RX (receive) pin to Digital 8

// You can change the pin numbers to match your wiring if you dare...
SoftwareSerial ss(7, 8);

static void smartdelay(unsigned long ms){
    unsigned long start = millis();
    do {
        while (ss.available())
        gps.encode(ss.read());
    } while (millis() - start < ms);
}

void parseGPRMC(){
    float flat = 0;
    float flon = 0;
    long unsigned int age = 0;
    long unsigned int date = 0;
    long unsigned int time = 0;
    // int chars = 0;
    int year;
    byte month, day, hour, minute, second, hundredths;
    
    Serial.println("Satellites:");
    Serial.println(gps.satellites()); // grabs satellite fix number (int)
    GPS_satelliteFix = gps.satellites();
    Serial.println("HDOP:");
    Serial.println(gps.hdop()); // whatever this is (int)
    // Serial.println(static_cast<int>(gps.hdop()));
    GPS_HDOP = static_cast<int>(gps.hdop()); // default value is ovf, largest unsigned int
    gps.f_get_position(&flat, &flon, &age);
    Serial.println("Lat:");
    Serial.println(flat*1000000);
    GPS_latitude = flat*1000000;
    Serial.println("Lon:");
    Serial.println(flon*1000000);
    GPS_longitude = flon*1000000;
    Serial.println("Age:");
    Serial.println(age); // (int)
    // Serial.println(static_cast<int>(age));
    GPS_age = static_cast<int>(age); // default value is ovf, largest unsigned int
    Serial.println("Altitude:");
    Serial.println(gps.f_altitude());
    GPS_altitude = gps.f_altitude();
    Serial.println("Course:");
    Serial.println(gps.f_course());
    GPS_course = gps.f_course();
    Serial.println("Speed (mph):");
    // change speed to signed float?? default value is -1.00
    Serial.println(gps.f_speed_mph());
    GPS_speed = gps.f_speed_mph();
    gps.get_datetime(&date, &time, &age);
    gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
    Serial.println("Year:");
    Serial.println(year);
    GPS_year = year;
    Serial.println("Month:");
    Serial.println(month);
    GPS_month = month;
    Serial.println("Day:");
    Serial.println(day);
    GPS_day = day;
    Serial.println("Hour:");
    Serial.println(hour);
    GPS_hour = hour;
    Serial.println("Min:");
    Serial.println(minute);
    GPS_minutes = minute;
    Serial.println("Sec:");
    Serial.println(second);
    GPS_seconds = second;
    Serial.println("Hundredths:");
    Serial.println(hundredths);
    GPS_hundredths = hundredths;
    
    smartdelay(100);

}

#endif