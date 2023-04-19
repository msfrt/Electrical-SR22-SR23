// Tested and works great with the Adafruit Ultimate GPS module
// using MTK33x9 chipset
//    ------> http://www.adafruit.com/products/746

#include "gps.hpp"
#include "can_send.hpp"

// #include <SoftwareSerial.h>

// #include <Adafruit_GPS.h>

// #include <Adafruit_NeoPixel.h>
// #include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <StateCAN.h>
#include <FlexCAN_T4.h>
#include <EasyTimer.h>
#include <BoardTemp.h>
// #include <EepromHelper.h>
// #include <Printers.h>
// #include <XBee.h>
// #include <BoardTemp.h>

// #include "rainbow_pixels.hpp"

//can bus decleration
// FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> cbus1;
// static CAN_message_t msg;
#define CAN1_BAUDRATE 1000000

#include "CAN/CAN1.hpp"

#include "gps.hpp"

#include "can_send.hpp"

// board temp setup
// #define READ_RESOLUTION_BITS 12
// const int board_temp_pin = 19;
// BoardTempDiode board_temp(board_temp_pin, READ_RESOLUTION_BITS, 26.2, 580);
// EasyTimer board_temp_sample_timer(50);

// NeoPixel parameters & initialization
// const int pixel_pin = 9;  // teensy pin #
// const int pixel_cnt = 1;  // number of LEDs
// const int pixel_brightness_percent = 100;  // 0 - 100%
// Adafruit_NeoPixel pixel = Adafruit_NeoPixel(pixel_cnt, pixel_pin, NEO_GRB + NEO_KHZ800);

// Connect the GPS Power pin to 5V
// Connect the GPS Ground pin to ground
// Connect the GPS TX (transmit) pin to Digital 7
// Connect the GPS RX (receive) pin to Digital 8

// You can change the pin numbers to match your wiring if you dare...
SoftwareSerial mySerial(7, 8);
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

// #define PMTK_Q_RELEASE "$PMTK605*31"

//std::vector<char> vec;

bool warmup = false;

void setup() {
  
  //while (!Serial); // wait for Serial to be ready -- nah fam

  Serial.begin(115200); // The serial port for the Arduino IDE port output
  
  mySerial.begin(9600); // GPS baud rate
  
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
  mySerial.println(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  //mySerial.println(PMTK_SET_NMEA_OUTPUT_ALLDATA);

  mySerial.println(PMTK_SET_NMEA_UPDATE_10HZ);

}

void loop() {

  if (mySerial.available()) {
    
    while (!warmup){
      for (int i = 0; i < 5; i++){ // currently elimates the doodoo sentences on startup -- test to see if changes
        String input = mySerial.readStringUntil('\n');
      }
      warmup = true;
    }
    
    String input = mySerial.readStringUntil('\n');
    
    parseGPRMC(input);
    
    send_can();
    
    //Serial.println();

    // char c = mySerial.read();  
    
    // vec.push_back(c);

    // Serial.write(c);
    
    // if (c == '\n'){
      
    //   for (char c : vec){
    //     Serial.write(c);
    //   }
      
    //   vec.clear();
    // }

  }

}