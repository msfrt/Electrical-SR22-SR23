// Dave Yonkers, 2022

#include <Adafruit_NeoPixel.h>
#include <StateCAN.h>
#include <FlexCAN_T4.h>
#include <EasyTimer.h>
#include <BoardTemp.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "ILI9341_t3n.h"
#define SPI0_DISP1

#define READ_RESOLUTION_BITS 12

// can bus decleration
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> cbus1;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> cbus2;
static CAN_message_t rxmsg;

// BoardTemp(int pin, int read_bits, int temp_cal, int mv_cal);
BoardTempDiode board_temp(21, READ_RESOLUTION_BITS, 26.2, 598);
EasyTimer board_temp_sample_timer(10);


// fonts :)
#include "fonts/LiberationMonoBold.h"
#include "fonts/LiberationMonoBoldItalic.h"


// NeoPixel parameters
const int pixels_top_pin = 3; // teensy pin #
const int pixels_left_pin= 2;
const int pixels_right_pin = 4;
const int pixels_top_cnt = 16; // number of LEDs
const int pixels_left_cnt = 4;
const int pixels_right_cnt = 4;
      int pixel_brightness_percent = 5; // 0 - 100; 100 is blinding... 4 is the minimum for all LED bar colors to work
const int pixel_brightness_nighttime = 1;
const int pixel_brightness_daytime = 55;

Adafruit_NeoPixel pixels_top =   Adafruit_NeoPixel(pixels_top_cnt,   pixels_top_pin,   NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_left =  Adafruit_NeoPixel(pixels_left_cnt,  pixels_left_pin,  NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_right = Adafruit_NeoPixel(pixels_right_cnt, pixels_right_pin, NEO_GRB + NEO_KHZ800);

// TFT display paramters
#define TFTL_DC 18
#define TFTL_CS 10
#define TFTL_MOSI 11
#define TFTL_MISO 12
#define TFTL_CLK 13
#define TFTL_RST 19
#define TFTL_BL 6
    int display_left_brightness_percent = 100;


#define TFTR_DC 5
#define TFTR_CS 9
#define TFTR_MOSI 11
#define TFTR_MISO 12
#define TFTR_CLK 13
#define TFTR_RST 17
#define TFTR_BL 7
    int display_right_brightness_percent = 100;



// used for light-sensor dimming
const int display_brightness_percent_nighttime = 75;
const int display_brightness_percent_daytime = 100;


ILI9341_t3n display_left = ILI9341_t3n(TFTL_CS, TFTL_DC, TFTL_RST);
ILI9341_t3n display_right = ILI9341_t3n(TFTR_CS, TFTR_DC, TFTR_RST);


// keep track of the time elapsed between loop iterations
unsigned long previousUpdateTime = 0;


// pins for the steering wheel buttons
const int button1_pin = 14;
const int button2_pin = 15;
unsigned long button1_time = 0;
unsigned long button2_time = 0 ;
const unsigned long button_delay = 300; // @GLOBAL_PARAM - milliseconds - used in check_button to avoid double-presses


// CAN message and read definitions
#include "CAN/CAN1.hpp"
#include "CAN/CAN2.hpp"


#include "light_sensor.hpp"
EasyTimer light_sensor_sample_timer(100);
const int light_sensor_pin = 20;


EasyTimer debug(50); // debugging timer

// used for dynamically changing clock speed :-)))
// #if defined(__IMXRT1062__)
// extern "C" uint32_t set_arm_clock(uint32_t frequency);
// #endif

// include externally-written functions
#include "led_startup.hpp"


// classes for control
//#include "classes/ScreenController.h"
#include "Screen.hpp"
#include "ScreenInfo.hpp"
#include "ScreensController.hpp"

CScreensController screensController(display_left, display_right);
//CScreenInfo testScreen(display_left, DISPLAY_WIDTH, DISPLAY_HEIGHT);



void setup() {

  // dynamically change clock speed
  // #if defined(__IMXRT1062__)
  //   set_arm_clock(45000000);
  //   Serial.print("F_CPU_ACTUAL=");
  //   Serial.println(F_CPU_ACTUAL);
  // #endif

  // set analog read resolution
  analogReadResolution(READ_RESOLUTION_BITS);

  // initilize CAN busses
  cbus1.begin();
  cbus1.setBaudRate(1000000);
  cbus2.begin();
  cbus2.setBaudRate(1000000);
  set_mailboxes();



  // initialze serial coms
  Serial.begin(115200);

  // init top pixels
  pixels_top.setBrightness(map(pixel_brightness_percent, 0, 100, 0, 255));
  pixels_top.begin();
  pixels_top.show();
  // init right pixels
  pixels_right.setBrightness(map(pixel_brightness_percent, 0, 100, 0, 255));
  pixels_right.begin();
  pixels_right.show();
  // init left pixels
  pixels_left.setBrightness(map(pixel_brightness_percent, 0, 100, 0, 255));
  pixels_left.begin();
  pixels_left.show();

  // initialize screens
  display_left.begin();
  display_right.begin();
  display_left.setRotation(3);
  display_right.setRotation(1);
  // clear screen
  display_left.fillScreen(ILI9341_BLACK);
  display_right.fillScreen(ILI9341_BLACK);
  

  // initialize buttons
  pinMode(button1_pin, INPUT_PULLUP);
  pinMode(button2_pin, INPUT_PULLUP);

  // fun LED startup sequence. Last parameter is time multiplier. 0 is fastest, 5 is pretty darn slow.
  // if you set it higher than 5, I have respect for your patience
  //led_startup(pixels_top, pixels_left, pixels_right, 1);

  
  // testScreen.SetSignal(1, &M400_rpm, "RPM:", "%.1f", 1000);
  // testScreen.SetSignal(2, &M400_oilPressure, "OILP:", "%.1f");
  // testScreen.SetSignal(3, &M400_oilTemp, "OILT:", "%.0f");
  // testScreen.SetSignal(4, &M400_engineTemp, "ENG:", "%.1f");
  // testScreen.Initialize();
  
  screensController.Initialize();

  previousUpdateTime = millis();

  debug.set_delay_millis(10000);

}


void loop() {

  unsigned long elapsed = millis() - previousUpdateTime;
  previousUpdateTime = millis();
  

  // read any incoming CAN messages
  read_can();
  
  //testScreen.Update();

  screensController.Update(elapsed);


  if (debug.isup()){
    screensController.OnButtonPress();
  }
  

}





void set_mailboxes(){

  // to view mailbox status, you can use the member function mailboxStatus(). Don't put it in a fast loop, though,
  // because you may actually affect how the chips moves things around

  cbus1.setMaxMB(64);
  cbus1.setMB(MB4,RX,STD);  // first 30 mailboxes as rx, 2 rx extended. this is pretty overkill, but hey, here they are
  cbus1.setMB(MB5,RX,STD);  // this leaves the last 32 mailboxes for tx
  cbus1.setMB(MB6,RX,STD);
  cbus1.setMB(MB7,RX,STD);
  cbus1.setMB(MB8,RX,STD);
  cbus1.setMB(MB9,RX,STD);
  cbus1.setMB(MB10,RX,STD);
  cbus1.setMB(MB11,RX,STD);
  cbus1.setMB(MB12,RX,STD);
  cbus1.setMB(MB13,RX,STD);
  cbus1.setMB(MB14,RX,STD);
  cbus1.setMB(MB15,RX,STD);
  cbus1.setMB(MB16,RX,STD);
  cbus1.setMB(MB17,RX,STD);
  cbus1.setMB(MB18,RX,STD);
  cbus1.setMB(MB19,RX,STD);
  cbus1.setMB(MB20,RX,STD);
  cbus1.setMB(MB21,RX,STD);
  cbus1.setMB(MB22,RX,STD);
  cbus1.setMB(MB23,RX,STD);
  cbus1.setMB(MB24,RX,STD);
  cbus1.setMB(MB25,RX,STD);
  cbus1.setMB(MB26,RX,STD);
  cbus1.setMB(MB27,RX,STD);
  cbus1.setMB(MB28,RX,STD);
  cbus1.setMB(MB29,RX,STD);
  cbus1.setMB(MB30,RX,EXT);
  cbus1.setMB(MB31,RX,EXT);

  cbus2.setMaxMB(64);
  cbus2.setMB(MB4,RX,STD);
  cbus2.setMB(MB5,RX,STD);
  cbus2.setMB(MB6,RX,STD);
  cbus2.setMB(MB7,RX,STD);
  cbus2.setMB(MB8,RX,STD);
  cbus2.setMB(MB9,RX,STD);
  cbus2.setMB(MB10,RX,STD);
  cbus2.setMB(MB11,RX,STD);
  cbus2.setMB(MB12,RX,STD);
  cbus2.setMB(MB13,RX,STD);
  cbus2.setMB(MB14,RX,STD);
  cbus2.setMB(MB15,RX,STD);
  cbus2.setMB(MB16,RX,STD);
  cbus2.setMB(MB17,RX,STD);
  cbus2.setMB(MB18,RX,STD);
  cbus2.setMB(MB19,RX,STD);
  cbus2.setMB(MB20,RX,STD);
  cbus2.setMB(MB21,RX,STD);
  cbus2.setMB(MB22,RX,STD);
  cbus2.setMB(MB23,RX,STD);
  cbus2.setMB(MB24,RX,STD);
  cbus2.setMB(MB25,RX,STD);
  cbus2.setMB(MB26,RX,STD);
  cbus2.setMB(MB27,RX,STD);
  cbus2.setMB(MB28,RX,STD);
  cbus2.setMB(MB29,RX,STD);
  cbus2.setMB(MB30,RX,EXT);
  cbus2.setMB(MB31,RX,EXT);
}


/**
 *  Reads a CAN message if available and then sends it to thr
 *  proper decoding funciton
 **/
void read_can(){

  if (cbus1.read(rxmsg)){
    decode_CAN1(rxmsg);
  }

  if (cbus2.read(rxmsg)){
    decode_CAN2(rxmsg);
  }

}


