#ifndef CAN_SEND_GEEPEE
#define CAN_SEND_GEEPEE

#include <FlexCAN_T4.h>
#include <EasyTimer.h>
//#include <BoardTemp.h>
#include <StateCAN.h>

#include "CAN/CAN1.hpp"

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> cbus1;
static CAN_message_t msg;

// void send_GPS_10(){
//   // the following line initializes a counter that is specific to PDM_10, but is static, so it's only created once.
//   //static StateCounter ctr;

//   // update message ID and length
//   msg.id = 160;
//   msg.len = 8;

//   // since the last calculation event, this records the sensor data into the actual signals. The calculations are
//   // done automatically. why don't we simply just throw the sensors into the can message buffer, you ask? Well, the
//   // signals are global and are used elsewhere for other important PDM functions.
//   GPS_boardTemp = board_temp.value();
//   GPS_teensyTemp = tempmonGetTemp();


//   // load up the message buffer
//   msg.buf[0] = ctr.value();
//   msg.buf[1] = 0;
//   msg.buf[2] = GPS_boardTemp.can_value();
//   msg.buf[3] = GPS_boardTemp.can_value() >> 8;
//   msg.buf[4] = GPS_teensyTemp.can_value();
//   msg.buf[5] = GPS_teensyTemp.can_value() >> 8;
//   msg.buf[6] = 0;
//   msg.buf[7] = 0;

//   // send the message
//   cbus1.write(msg);
// }

// // lap trigger message, no fancy signals needed.
// void send_GPS_11(){

//   msg.id = 161;
//   msg.len = 8;

//   msg.buf[0] = TCGPS_laptrigger.can_value();
//   msg.buf[1] = 0;
//   msg.buf[2] = 0;
//   msg.buf[3] = 0;
//   msg.buf[4] = 0;
//   msg.buf[5] = 0;
//   msg.buf[6] = 0;
//   msg.buf[7] = 0;

//   // send the message
//   cbus1.write(msg);
// }

void send_GPS_12(){

  msg.id = 162;
  msg.len = 8;

  msg.buf[0] = GPS_latitude.can_value();
  msg.buf[1] = GPS_latitude.can_value() >> 8;
  msg.buf[2] = GPS_latitude.can_value() >> 16;
  msg.buf[3] = GPS_latitude.can_value() >> 24;
  msg.buf[4] = GPS_altitude.can_value();
  msg.buf[5] = GPS_altitude.can_value() >> 8;
  msg.buf[6] = GPS_altitude.can_value() >> 16;
  msg.buf[7] = GPS_altitude.can_value() >> 24;

  // send the message
  cbus1.write(msg);
  //Serial.write(" Lat + Alt ");
}

void send_GPS_13(){

  msg.id = 163;
  msg.len = 8;

  msg.buf[0] = GPS_longitude.can_value();
  msg.buf[1] = GPS_longitude.can_value() >> 8;
  msg.buf[2] = GPS_longitude.can_value() >> 16;
  msg.buf[3] = GPS_longitude.can_value() >> 24;
  msg.buf[4] = GPS_speed.can_value();
  msg.buf[5] = GPS_speed.can_value() >> 8;
  msg.buf[6] = GPS_speed.can_value() >> 16;
  msg.buf[7] = GPS_speed.can_value() >> 24;

  // send the message
  cbus1.write(msg);
  //Serial.write(" Lon + Spd ");
}

void send_GPS_14(){

  // update message ID and length
  msg.id = 164;
  msg.len = 8;

  // load up the message buffer
  msg.buf[0] = GPS_age.can_value();
  msg.buf[1] = GPS_course.can_value();
  msg.buf[2] = GPS_HDOP.can_value();
  msg.buf[3] = GPS_satelliteFix.can_value();
  msg.buf[4] = 0;
  msg.buf[5] = 0;
  msg.buf[6] = 0;
  msg.buf[7] = 0;

  // send the message
  cbus1.write(msg);
  //Serial.write(" Age + Course + HDOP + satFix ");
}

void send_GPS_15(){

  // update message ID and length
  msg.id = 165;
  msg.len = 8;

  // load up the message buffer
  // msg.buf[0] = GPS_posStat.can_value();
  msg.buf[0] = GPS_hour.can_value();
  msg.buf[1] = GPS_minutes.can_value();
  msg.buf[2] = GPS_seconds.can_value();
  msg.buf[3] = GPS_hundredths.can_value();
  msg.buf[4] = GPS_day.can_value();
  msg.buf[5] = GPS_month.can_value();
  msg.buf[6] = GPS_year.can_value();
  msg.buf[7] = 0;

  // send the message
  cbus1.write(msg);
  //Serial.write("pos");
  //Serial.write(GPS_posStat.can_value());
}



// void send_can1(){

//   // sends laptrigger message constantly, but the value should be 0 if a trigger has not happened.
//   static EasyTimer TCGPS_11_timer(10); // 10Hz
//   if (TCGPS_11_timer.isup()){
//     send_GPS_11();
//   }
// }

// void send_can2(){

//   static EasyTimer GPS_10_timer(10); // 10Hz
//   if (GPS_10_timer.isup()){
//     send_GPS_10();
//   }

// }

void send_can(){

  static EasyTimer GPS_12_timer(10); // 10Hz
  if (GPS_12_timer.isup()){
    send_GPS_12();
  }

  static EasyTimer GPS_13_timer(10); // 10Hz
  if (GPS_13_timer.isup()){
    send_GPS_13();
  }

  static EasyTimer GPS_14_timer(10); // 10Hz
  if (GPS_14_timer.isup()){
    send_GPS_14();
  }

  static EasyTimer GPS_15_timer(10); // 10Hz
  if (GPS_15_timer.isup()){
    send_GPS_15();
  }

  //Serial.println("CAN sent!");

}

#endif