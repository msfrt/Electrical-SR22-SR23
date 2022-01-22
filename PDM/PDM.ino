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

//sets overide percentage
const int override_percent = 80;

// eeprom-saved signals
#include "EEPROM_sigs.hpp"

// useful sensor sampling definitions can be found here
#include "sensors.hpp"

// this file contains all of the control tables & declarations for fans and water pump
#include "fans.hpp"

// signal definitions are inside
#include "sigs_inside.hpp"

// CAN message definitions are inside
#include "can_send.hpp"

// CAN read functions are inside
#include "can_read.hpp"

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
  cbus1.begin();
  cbus1.setBaudRate(1000000);
  cbus2.begin();
  cbus2.setBaudRate(1000000);
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

  //sets state of fan right/left
  pinMode(fanr_pin, INPUT);
  pinMode(fanl_pin, INPUT);
}

void loop() {

  // sensor sampling
  sample_ADCs();
  if (board_temp_sample_timer.isup()){
    board_temp.sample();
  }

  // read both can buses
  read_can1();
  read_can2();

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

  //Reads fan left switch and set override accordingly
  digitalRead(fanl_pin);
  if (fanl_pin == 1)
    USER_fanLeftOverride = override_percent;
  else
    USER_fanLeftOverride = 0;

  //Reads fan right switch and set orride accordingly
  digitalRead(fanr_pin);
  if (fanr_pin ==1)
    USER_fanRightOverride = override_percent;
  else
    USER_fanRightOverride = 0;
      
  
  
}



void set_mailboxes(){

  // to view mailbox status, you can use the member function mailboxStatus(). Don't put it in a fast loop, though,
  // because you may actually affect how the chips moves things around

  // CAN 2 - sends a bunch of stuff
  cbus2.setMaxMB(64); // change from default 16 mailboxes to 64 (maximum)
  cbus2.enableFIFO(); // first in-first out prevents overwriting unsent messages depending on the queue
  cbus2.setMB(MB4,RX,STD);  // change the first 12 mailboxes to recieve standard frames. 4 for extended. the first four
  cbus2.setMB(MB5,RX,STD);  // already do by default. the rest of the mailboxes (48) are TX mailboxes by default, which
  cbus2.setMB(MB6,RX,STD);  // is necessary because we send more data than the bus can handle in short periods of time
  cbus2.setMB(MB7,RX,STD);
  cbus2.setMB(MB8,RX,STD);
  cbus2.setMB(MB9,RX,STD);
  cbus2.setMB(MB10,RX,STD);
  cbus2.setMB(MB11,RX,STD);
  cbus2.setMB(MB12,RX,EXT);  // perhaps there is an issue and something is sending extended frames for whatever reason
  cbus2.setMB(MB13,RX,EXT);  // we have a lot of mailboxes anyways, so these four can be set to extended
  cbus2.setMB(MB14,RX,EXT);
  cbus2.setMB(MB15,RX,EXT);
  // cbus2.setMB(MB16,TX);
  // cbus2.setMB(MB17, TX);
  // cbus2.setMB(MB18, TX);
  // cbus2.setMB(MB19, TX);
  // cbus2.setMB(MB20, TX);
  // cbus2.setMB(MB21, TX);
  // cbus2.setMB(MB22, TX);
  // cbus2.setMB(MB23, TX);
  // cbus2.setMB(MB24, TX);
  // cbus2.setMB(MB25, TX);
  // cbus2.setMB(MB26, TX);
  // cbus2.setMB(MB27, TX);
  // cbus2.setMB(MB28, TX);
  // cbus2.setMB(MB29, TX);
  // cbus2.setMB(MB30, TX);
  // cbus2.setMB(MB31, TX);
  // cbus2.setMB(MB32, TX);
  // cbus2.setMB(MB33, TX);
  // cbus2.setMB(MB34, TX);
  // cbus2.setMB(MB35, TX);
  // cbus2.setMB(MB36, TX);
  // cbus2.setMB(MB37, TX);
  // cbus2.setMB(MB38, TX);
  // cbus2.setMB(MB39, TX);
  // cbus2.setMB(MB40, TX);
  // cbus2.setMB(MB41, TX);
  // cbus2.setMB(MB42, TX);
  // cbus2.setMB(MB43, TX);
  // cbus2.setMB(MB44, TX);
  // cbus2.setMB(MB45, TX);
  // cbus2.setMB(MB46, TX);
  // cbus2.setMB(MB47, TX);
  // cbus2.setMB(MB48, TX);
  // cbus2.setMB(MB49, TX);
  // cbus2.setMB(MB50, TX);
  // cbus2.setMB(MB51, TX);
  // cbus2.setMB(MB52, TX);
  // cbus2.setMB(MB53, TX);
  // cbus2.setMB(MB54, TX);
  // cbus2.setMB(MB55, TX);
  // cbus2.setMB(MB56, TX);
  // cbus2.setMB(MB57, TX);
  // cbus2.setMB(MB58, TX);
  // cbus2.setMB(MB59, TX);
  // cbus2.setMB(MB60, TX);
  // cbus2.setMB(MB61, TX);
  // cbus2.setMB(MB62, TX);
  // cbus2.setMB(MB63, TX);


  cbus1.setMaxMB(64);
  cbus1.enableFIFO();
  cbus1.setMB(MB4,RX,STD);  // first 16 mailboxes as rx, 4 rx extended. this is pretty overkill, but hey, here they are
  cbus1.setMB(MB5,RX,STD);
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
  cbus1.setMB(MB16,RX,EXT);
  cbus1.setMB(MB17,RX,EXT);
  cbus1.setMB(MB18,RX,EXT);
  cbus1.setMB(MB19,RX,EXT);
}
