// Dave Yonkers, 2020

#include <EasyTimer.h>
#include <PWMControl.h>
#include <ReadADC.h>
#include <StateCAN.h>
#include <FlexCAN_T4.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <BoardTemp.h>
#include <EepromHelper.h>

/* TODOS:
 *  -
*/

// can bus decleration
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;
static CAN_message_t rxmsg;
// each bus has a total of 64 mailboxes
#define NUM_RX_STD_MAILBOXES 32
#define NUM_RX_EXT_MAILBOXES 2
#define NUM_TX_MAILBOXES 30
// limit the number of messages each bus can read for each loop cycle.
// Typically, only one message is recieved in the time that the loop can run,
// but a buildup can occur, and this limit can poll the bus to read messages
// while not getting stuck in an infinite loop
#define MAX_CAN_FRAME_READ_PER_CYCLE 5


// global variable definition
int GLO_engine_state = 0; // engine state (no need to change this variable)
const int GLO_read_resolution_bits = 12; // bits for Teensy-based read resolution
const int GLO_max_analog_write_pwm = 255; // maximum PWN value

// minimum voltage for the engine to be in "cranking" mode
const int GLO_cranking_starter_volt_threshold = 5;
// max rpm for the engine to still be in "cranking" mode
const int GLO_cranking_rpm_threshold = 2000;
// engine running min rpm threshold
const int GLO_engine_on_rpm_threshold = 900;
// engine cooldown state duration in milliseconds
const int GLO_engine_cooldown_duration = 5000;

const int GLO_brakelight_min_pressure_F = 150; // minimum pressure required to activate the brakelight (PSI)
const int GLO_brakelight_min_pressure_R = 125;
const int GLO_brakelight_teensy_pin = 4;

const int GLO_data_circuit_teensy_pin = 5;

const int GLO_NeoPixel_teensy_pin = 2;
      int GLO_NeoPixel_brightness_percent = 10; // 0 - 100 %

// boolean used in the C50 to determine if we should be logging
bool GLO_data_log_bool = 0;
const unsigned long data_log_timeout_ms = 5000; // time to log after engine and cooldown mode has stopped

Adafruit_NeoPixel GLO_obd_neopixel(1, GLO_NeoPixel_teensy_pin, NEO_GRB + NEO_KHZ800);

//BoardTemp(int pin, int read_bits, int temp_cal, int mv_cal);
BoardTempDiode board_temp(21, GLO_read_resolution_bits, 28.1, 594);
EasyTimer board_temp_sample_timer(50);

// EEPROM
const int eeprom_cs_pin = 9;
EEPROM_25LC128 eeprom(eeprom_cs_pin);

// engine time clock update frequency. (can be quite low, but don't set too low, as we want this to still be accurate)
// the actual time checking happens in the timer function.
EasyTimer engine_time_update_timer(1);

// odemeter update frequency. Should probably keep this pretty low, because if it updates too fast, the calculations
// will be so small that mileage may actually never be incremented because of poor floating-point math
EasyTimer odometer_update_timer(2);

// eeprom-saved signals
#include "EEPROM_sigs.hpp"

// useful sensor sampling definitions can be found here
#include "sensors.hpp"

// this file contains all of the control tables & declarations for fans and water pump
#include "fans.hpp"

// CAN message definitions are inside
#include "CAN/CAN1.hpp"
#include "CAN/CAN2.hpp"
#include "can_send.hpp"

// odds and ends functions
#include "misc_fcns.hpp"

// on-board diagnostics
#include "obd.hpp"

// timer that you can use to print things out for debugging
EasyTimer debug(1);
const bool GLO_debug = false;




void setup() { //high 18 low 26

  analogReadResolution(GLO_read_resolution_bits);

  // begin OBD Neopixel
  GLO_obd_neopixel.begin();
  GLO_obd_neopixel.setBrightness(map(GLO_NeoPixel_brightness_percent, 0, 100, 0, 255));
  GLO_obd_neopixel.setPixelColor(0, 255, 0, 0); // red
  GLO_obd_neopixel.show();

  // Initialize serial communication
  Serial.begin(112500);

  //initialize the CAN Bus and set its baud rate to 1Mb
  can1.begin();
  can1.setBaudRate(1000000);
  can2.begin();
  can2.setBaudRate(1000000);
  set_mailboxes();

  // populate left fan table
  int *fanl_table_ptr = fan_left_table[0]; // create a temp ptr to populate PWM device
  fan_left.fill_table(fanl_table_ptr); // populate the PWMDevice table

  // populate right fan table
  int *fanr_table_ptr = fan_right_table[0];
  fan_right.fill_table(fanr_table_ptr);

  // populate water pump table
  int *wp_table_ptr = wp_table[0];
  water_pump.fill_table(wp_table_ptr);

  // initialize the ADC sensors
  initialize_ADCs();

  // initialize SPI communication
  SPI.begin();

  // initialize the data circuit pin
  pinMode(GLO_data_circuit_teensy_pin, OUTPUT);
  // turn the data circuit on
  digitalWrite(GLO_data_circuit_teensy_pin, HIGH);
  // initialize brakelight pin
  pinMode(GLO_brakelight_teensy_pin, OUTPUT);

  // EEPROM
  eeprom.begin();

  // write the eeprom variables that are not commented out in the write eeprom function in the EEPROM_sigs file
  initialize_eeprom_variables();

  GLO_obd_neopixel.setPixelColor(0, 0, 255, 0); // green
  GLO_obd_neopixel.show();

  // board temp initialization
  board_temp.begin();

  // neat brakelight animation
  brakelight_startup();

}


void loop() {

  // sensor sampling
  sample_ADCs();
  if (board_temp_sample_timer.isup()){
    board_temp.sample();
  }

  // read both can buses
  read_CAN();

  // run the brakelight
  brakelight_run();
  

  // determine engine state and control PWM outputs
  determine_engine_state(GLO_engine_state);
  fan_left.set_pwm(GLO_engine_state);
  fan_right.set_pwm(GLO_engine_state);
  water_pump.set_pwm(GLO_engine_state);

  // determine if we need to log right now
  GLO_data_log_bool = determine_logging_state(GLO_engine_state, GLO_engine_cooldown_duration);

  // continously run OBD (individual timers are included)
  obd_main();

  // engine timer update
  if (engine_time_update_timer.isup()){
    engine_timer(eeprom_engine_hours, eeprom_engine_minutes);
  }

  // odometer update
  if (odometer_update_timer.isup())
    odometer(M400_groundSpeed, eeprom_mileage);

  // send all of the things
  send_can1();
  send_can2();

  if (GLO_debug && debug.isup()){
    Serial.println();
    Serial.println(eeprom_engine_hours.value());
    Serial.println(eeprom_engine_minutes.value());
  }
}



void set_mailboxes() {
  // to view mailbox status, you can use the member function mailboxStatus().
  // Don't put it in a fast loop, though, because you may actually affect how
  // the chips moves things around

  can1.setMaxMB(64);  // use all mailboxes of course
  can2.setMaxMB(64);

  for (int i = 0; i < NUM_RX_STD_MAILBOXES; i++) {
    can1.setMB((FLEXCAN_MAILBOX)i, RX, STD);
    can2.setMB((FLEXCAN_MAILBOX)i, RX, STD);
  }
  for (int i = NUM_RX_STD_MAILBOXES;
       i < (NUM_RX_STD_MAILBOXES + NUM_RX_EXT_MAILBOXES); i++) {
    can1.setMB((FLEXCAN_MAILBOX)i, RX, EXT);
    can2.setMB((FLEXCAN_MAILBOX)i, RX, EXT);
  }
  for (int i = (NUM_RX_STD_MAILBOXES + NUM_RX_EXT_MAILBOXES);
       i < (NUM_RX_STD_MAILBOXES + NUM_RX_EXT_MAILBOXES + NUM_TX_MAILBOXES);
       i++) {
    can1.setMB((FLEXCAN_MAILBOX)i, TX, STD);
    can2.setMB((FLEXCAN_MAILBOX)i, TX, STD);
  }

  // be sure to assign at least one mailbox to each message that you want to
  // read. filtering allows us to avoid using clock cycles to read messages that
  // we have no interest in. it also reserves a slot for messages as they are
  // recieved.
  // can1.setMBFilter(REJECT_ALL);
  // can1.setMBFilter(MB0, REJECT_ALL);
  // can1.setMBFilter(MB0, M400_engineTemp.get_msg_id());
  // can1.setMBFilter(MB1, 0);
  // can1.setMBFilter(MB2, 0);
  // can1.setMBFilter(MB3, 0);
  // can1.setMBFilter(MB4, 0);
  // can1.setMBFilter(MB5, 0);
  // can1.setMBFilter(MB6, 0);
  // can1.setMBFilter(MB7, 0);
  // can1.setMBFilter(MB8, 0);
  // can1.setMBFilter(MB9, 0);
  // can1.setMBFilter(MB10, 0);
  // can1.setMBFilter(MB11, 0);
  // can1.setMBFilter(MB12, 0);
  // can1.setMBFilter(MB13, 0);
  // can1.setMBFilter(MB14, 0);

  // can2.setMBFilter(REJECT_ALL);
  // can2.setMBFilter(MB0, REJECT_ALL);
  // can2.setMBFilter(MB0, ATCCF_brakePressureF.get_msg_id());
  // can2.setMBFilter(MB1, REJECT_ALL);
  // can2.setMBFilter(MB1, ATCCF_brakePressureR.get_msg_id());
  // can2.setMBFilter(MB2, ATCCF_rotorTempFL.get_msg_id());  // includes FR
  // can2.setMBFilter(MB3, ATCCR_rotorTempRL.get_msg_id());  // includes FR
  // can2.setMBFilter(MB4, PDM_coolingOverrideActive.get_msg_id());
  // can2.setMBFilter(MB5, PDM_fanLeftDutyCycle.get_msg_id());
  // can2.setMBFilter(MB6, 0);
  // can2.setMBFilter(MB7, 0);
  // can2.setMBFilter(MB8, 0);
  // can2.setMBFilter(MB9, 0);
  // can2.setMBFilter(MB10, 0);
  // can2.setMBFilter(MB11, 0);
  // can2.setMBFilter(MB12, 0);
  // can2.setMBFilter(MB13, 0);
  // can2.setMBFilter(MB14, 0);
}

/**
 *  Reads a CAN message if available and then sends it to thr
 *  proper decoding funciton
 **/
void read_CAN() {
  int count = 0;
  while (can1.read(rxmsg) && count < MAX_CAN_FRAME_READ_PER_CYCLE) {
    decode_CAN1(rxmsg);
    count++;
  }

  count = 0;
  while (can2.read(rxmsg) && count < MAX_CAN_FRAME_READ_PER_CYCLE) {
    decode_CAN2(rxmsg);
    count++;
  }
}





