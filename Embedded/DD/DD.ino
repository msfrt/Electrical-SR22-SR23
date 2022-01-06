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
// each bus has a total of 64 mailboxes
#define NUM_RX_STD_MAILBOXES 60
#define NUM_RX_EXT_MAILBOXES 2
#define NUM_TX_MAILBOXES 2
// limit the number of messages each bus can read for each loop cycle. Typically, only one
// message is recieved in the time that the loop can run, but a buildup can occur, and this
// limit can poll the bus to read messages while not getting stuck in an infinite loop
#define MAX_CAN_FRAME_READ_PER_CYCLE 5

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
#include "LedStartup.hpp"


// classes for control
//#include "classes/ScreenController.h"
#include "Screen.hpp"
#include "ScreenInfo.hpp"
#include "ScreensController.hpp"
#include "LightBarRPM.hpp"
#include "LightBarBlink.hpp"
#include "LightBarController.hpp"


CScreensController screensController(display_left, display_right);
CLightBarController lightsController(pixels_left, pixels_top, pixels_right);
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

  // Set the state racing bitmap
  #include "images/StateRacingBitmap.hpp"
  // draw state racing bitmap
  int stateRacingPosY = (DISPLAY_HEIGHT - stateRacingBitmapHeight) / 2;
  int stateRacingPosX = (DISPLAY_WIDTH  - stateRacingBitmapWidth ) / 2;
  display_left.drawBitmap(stateRacingPosX, stateRacingPosY, stateRacingBitmap, stateRacingBitmapWidth, stateRacingBitmapHeight, ILI9341_WHITE);
  display_right.drawBitmap(stateRacingPosX, stateRacingPosY, stateRacingBitmap, stateRacingBitmapWidth, stateRacingBitmapHeight, ILI9341_WHITE);


  // quick led startup animation
  LedStartup(pixels_top, pixels_left, pixels_right, 1);
  

  // initialize buttons
  pinMode(button1_pin, INPUT_PULLUP);
  pinMode(button2_pin, INPUT_PULLUP);
  
  screensController.Initialize();
  lightsController.Initialize();

  previousUpdateTime = millis();

  debug.set_delay_millis(10000);

  cbus1.mailboxStatus();


}




void loop() {

 

  unsigned long elapsed = millis() - previousUpdateTime;
  previousUpdateTime = millis();
  

  // read any incoming CAN messages
  // Serial.println("Reading CAN:");
  read_can();
  // cbus1.events();
  
  // if there is a new message to display to the driver, assemble and display it
  if (CMD_driverMessageChar0.is_updated()){
    char displayStringChars[] = "\0\0\0\0\0\0\0\0\0";
    displayStringChars[0] = CMD_driverMessageChar0.value();
    displayStringChars[1] = CMD_driverMessageChar1.value();
    displayStringChars[2] = CMD_driverMessageChar2.value();
    displayStringChars[3] = CMD_driverMessageChar3.value();
    displayStringChars[4] = CMD_driverMessageChar4.value();
    displayStringChars[5] = CMD_driverMessageChar5.value();
    displayStringChars[6] = CMD_driverMessageChar6.value();
    displayStringChars[7] = CMD_driverMessageChar7.value();
    String displayString(displayStringChars);
    screensController.OnMessage(displayString);
  }

  // notification light message recieved
  if (CMD_driverNotificationLightR.is_updated()){
    int R = CMD_driverNotificationLightR.value();
    int G = CMD_driverNotificationLightG.value();
    int B = CMD_driverNotificationLightB.value();
    lightsController.OnNotificationRecieved(R, G, B);
  }

  // the laptrigger was set
  if (TCGPS_laptrigger.is_updated() && static_cast<int>(TCGPS_laptrigger.value()) == 100){
    screensController.OnNewLap();
  }

  screensController.Update(elapsed);
  lightsController.Update(elapsed);



  if (debug.isup()){
    //screensController.OnButtonPress();
    //lightsController.OnButtonPress();
    //cbus1.mailboxStatus();
  }
  

}


void test_callback1(const CAN_message_t &imsg){
  //Serial.println("Recieved 1");
  decode_CAN1(imsg);
}

void test_callback2(const CAN_message_t &imsg){
  //Serial.println("Recieved 2");
  decode_CAN1(imsg);
}


void set_mailboxes(){

  // to view mailbox status, you can use the member function mailboxStatus(). Don't put it in a fast loop, though,
  // because you may actually affect how the chips moves things around

  cbus1.setMaxMB(64);  // use all mailboxes of course
  cbus2.setMaxMB(64);

  for (int i=0; i<NUM_RX_STD_MAILBOXES; i++){
    cbus1.setMB((FLEXCAN_MAILBOX)i, RX, STD);
    cbus2.setMB((FLEXCAN_MAILBOX)i, RX, STD);
  }
  for (int i=NUM_RX_STD_MAILBOXES; i<(NUM_RX_STD_MAILBOXES + NUM_RX_EXT_MAILBOXES); i++){
    cbus1.setMB((FLEXCAN_MAILBOX)i, RX, EXT);
    cbus2.setMB((FLEXCAN_MAILBOX)i, RX, EXT);
  }
  for (int i=(NUM_RX_STD_MAILBOXES + NUM_RX_EXT_MAILBOXES); i<(NUM_RX_STD_MAILBOXES + NUM_RX_EXT_MAILBOXES + NUM_TX_MAILBOXES); i++){
    cbus1.setMB((FLEXCAN_MAILBOX)i, TX, STD);
    cbus2.setMB((FLEXCAN_MAILBOX)i, TX, STD);
  }

  // be sure to assign at least one mailbox to each message that you want to read.
  // filtering allows us to avoid using clock cycles to read messages that we have no interest in.
  // it also reserves a slot for messages as they are recieved.
  cbus1.setMBFilter(REJECT_ALL);
  cbus1.setMBFilter(MB0, TCGPS_laptrigger.get_msg_id());
  cbus1.setMBFilter(MB1, C50_tcSet.get_msg_id());
  cbus1.setMBFilter(MB2, M400_groundSpeed.get_msg_id());
  cbus1.setMBFilter(MB3, M400_rpm.get_msg_id());
  cbus1.setMBFilter(MB4, M400_oilPressure.get_msg_id());
  cbus1.setMBFilter(MB5, M400_oilTemp.get_msg_id());
  cbus1.setMBFilter(MB6, CMD_driverMessageChar0.get_msg_id());
  cbus1.setMBFilter(MB7, CMD_driverNotificationLightR.get_msg_id());
  cbus1.setMBFilter(MB8, 0);
  cbus1.setMBFilter(MB9, 0);
  cbus1.setMBFilter(MB10, 0);
  cbus1.setMBFilter(MB11, 0);
  cbus1.setMBFilter(MB12, 0);
  cbus1.setMBFilter(MB13, 0);
  cbus1.setMBFilter(MB14, 0);


  
  cbus2.setMBFilter(REJECT_ALL);
  cbus2.setMBFilter(MB0, PDM_pdmVoltAvg.get_msg_id());
  cbus2.setMBFilter(MB1, ATCCF_brakeBias.get_msg_id());
  cbus2.setMBFilter(MB2, ATCCF_rotorTempFL.get_msg_id()); // includes FR
  cbus2.setMBFilter(MB3, ATCCR_rotorTempRL.get_msg_id());  // includes FR
  cbus2.setMBFilter(MB4, PDM_coolingOverrideActive.get_msg_id());
  cbus2.setMBFilter(MB5, 0);
  cbus2.setMBFilter(MB6, 0);
  cbus2.setMBFilter(MB7, 0);
  cbus2.setMBFilter(MB8, 0);
  cbus2.setMBFilter(MB9, 0);
  cbus2.setMBFilter(MB10, 0);
  cbus2.setMBFilter(MB11, 0);
  cbus2.setMBFilter(MB12, 0);
  cbus2.setMBFilter(MB13, 0);
  cbus2.setMBFilter(MB14, 0);
  

  

}


/**
 *  Reads a CAN message if available and then sends it to thr
 *  proper decoding funciton
 **/
void read_can(){

  int count = 0;
  while (cbus1.read(rxmsg) && count < MAX_CAN_FRAME_READ_PER_CYCLE){
    decode_CAN1(rxmsg);
    count++;
  }

  count = 0;
  while (cbus2.read(rxmsg) && count < MAX_CAN_FRAME_READ_PER_CYCLE){
    decode_CAN2(rxmsg);
    count++;
  }

}


