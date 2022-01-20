#ifndef CAN_READ_HPP
#define CAN_READ_HPP

#include "can_send.hpp"
#include "sigs_inside.hpp"
#include <FlexCAN_T4.h>

static CAN_message_t rxmsg;

// ID 411 on bus 2
void read_ATCCF_11(CAN_message_t &imsg){
  ATCCF_brakePressureF.set_can_value(imsg.buf[2] | imsg.buf[3] << 8);
  ATCCF_brakePressureR.set_can_value(imsg.buf[4] | imsg.buf[5] << 8);
}


// ID 710 on bus 2
//void read_USER_10(CAN_message_t &imsg){
//  USER_fanLeftOverride.set_can_value(imsg.buf[0]);
//  USER_fanRightOverride.set_can_value(imsg.buf[1]);
 // USER_wpOverride.set_can_value(imsg.buf[2]);
 // USER_brakeLightOverride.set_can_value(imsg.buf[3]);
//}


// ID 100 on bus 1 - M400 dataset 1
void read_M400_100(CAN_message_t &imsg){
  // multiplexer first-bit
  switch (imsg.buf[0]) {

    case 1:
      M400_groundSpeed.set_can_value(imsg.buf[2] << 8 | imsg.buf[3]);
      break;

    case 2:
      M400_tcPowerReduction.set_can_value(imsg.buf[6] << 8 | imsg.buf[7]);
      break;

    case 4:
      M400_ignCutLevelTotal.set_can_value(imsg.buf[2] << 8 | imsg.buf[3]);
      M400_rpm.set_can_value(imsg.buf[4] << 8 | imsg.buf[5]);
      break;

    case 5:
      M400_gear.set_can_value(imsg.buf[2] << 8 | imsg.buf[3]);
      break;

  }
}


// ID 101 on bus 1 - M400 dataset 2
void read_M400_101(CAN_message_t &imsg){
  // multiplexer first-bit
  switch (imsg.buf[0]) {

    case 2:
      M400_batteryVoltage.set_can_value(imsg.buf[6] << 8 | imsg.buf[7]);
      break;

    case 3:
      M400_engineTemp.set_can_value(imsg.buf[6] << 8 | imsg.buf[7]);
      break;

    case 10:
      M400_fuelPressure.set_can_value(imsg.buf[2] << 8 | imsg.buf[3]);
      break;

    case 15:
      M400_oilPressure.set_can_value(imsg.buf[6] << 8 | imsg.buf[7]);
      break;
  }
}






// function that reads the msg and then directs that data elsewhere
void read_can1(){
  if (cbus1.read(rxmsg)){

    switch (rxmsg.id) {
      case 100:
        read_M400_100(rxmsg);
        break;
      case 101:
        read_M400_101(rxmsg);
        break;
    } // end switch statement

  }
}


// function that reads the msg and then directs that data elsewhere
void read_can2(){
  if (cbus2.read(rxmsg)){

    switch (rxmsg.id) {
      case 411:
        read_ATCCF_11(rxmsg);
        break;
      //case 710:
        //read_USER_10(rxmsg);
        //break;
    } // end switch statement

  }
}


#endif
