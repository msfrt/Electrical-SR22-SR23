// Tested and works great with the Adafruit Ultimate GPS module
// using MTK33x9 chipset
//    ------> http://www.adafruit.com/products/746

#include "gps.hpp"
#include "can_send.hpp"

#include <SPI.h>
#include <StateCAN.h>
#include <FlexCAN_T4.h>
#include <EasyTimer.h>
#include <BoardTemp.h>

#define CAN1_BAUDRATE 1000000

#include "CAN/CAN1.hpp"


//SoftwareSerial ss(7, 8);
//SoftwareSerial cbus1(22, 23);

#define PMTK_SET_NMEA_UPDATE_1HZ  "$PMTK220,1000*1F"
//#define PMTK_SET_NMEA_UPDATE_5HZ  "$PMTK220,200*2C"
#define PMTK_SET_NMEA_UPDATE_10HZ "$PMTK220,100*2F"

// turn on only the second sentence (GPRMC)
#define PMTK_SET_NMEA_OUTPUT_RMCONLY "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"
// turn on GPRMC and GGA
//#define PMTK_SET_NMEA_OUTPUT_RMCGGA "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"
// turn on ALL THE DATA
//#define PMTK_SET_NMEA_OUTPUT_ALLDATA "$PMTK314,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28"
// turn off output
//#define PMTK_SET_NMEA_OUTPUT_OFF "$PMTK314,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"

void setup() {
  
  Serial.begin(115200); // The serial port for the Arduino IDE port output
  
  ss.begin(9600); // GPS baud rate
  
  delay(2000); // remove if u wanna

  // initilize CAN busses
  cbus1.begin();
  cbus1.setBaudRate(CAN1_BAUDRATE);
  // cbus2.begin();
  // cbus2.setBaudRate(1000000);

  // analogReadResolution(READ_RESOLUTION_BITS);
  // board_temp.begin();

  // begin NeoPixel
  // pixel.setBrightness(map(pixel_brightness_percent, 0, 100, 0, 255));
  // pixel.begin();
  // pixel.show();

  //Serial.println("Software Serial GPS Test Echo Test");
  
  // you can send various commands to get it started
  //mySerial.println(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  //mySerial.println(PMTK_SET_NMEA_OUTPUT_ALLDATA);

  ss.println(PMTK_SET_NMEA_UPDATE_10HZ);

}

void loop() {
    
  parseGPRMC();
  
  send_can();

}