#include <ps3_wrapper.h>
#include <Arduino.h>
#include <log.h>
#include <Ps3Controller.h>

#define PS3_MAX_YAW  (100) // TODO
#define PS3_MAX_PITCH  (100) // TODO
#define PS3_MAX_ROLL  (100) // TODO
#define PS3_MAX_JOY  (100) // TODO

bool ps3MotorEnable = false;
// Yaw: use momentum wheel to turn right - derivative of angle
// Pitch: rotate body backwards - angle
// Roll: tilt body right - angle
float ps3Ypr[3];


void ps3Notify() {
  LOG_S("PS3 notification");
  // hold R1 or L1 to enable movement
  ps3MotorEnable = (Ps3.data.button.l1 || Ps3.data.button.r1);
  if(ps3MotorEnable) {
    // adjust yaw/pith/roll with joys
    ps3Ypr[0] = ((float)Ps3.data.analog.stick.lx) / PS3_MAX_JOY * PS3_MAX_YAW;
    ps3Ypr[1] = -((float)Ps3.data.analog.stick.ry) / PS3_MAX_JOY * PS3_MAX_PITCH;
    ps3Ypr[2] = ((float)Ps3.data.analog.stick.rx) / PS3_MAX_JOY * PS3_MAX_ROLL;
  } else {
    for(int i=0; i<3; i++) ps3Ypr[i] = 0;
  }
}

void ps3OnConnect() {
  LOG_S("PS3 connected");
}

void ps3Initialize(const char* mac) {
  for(int i=0; i<3; i++) ps3Ypr[i] = 0;
  Ps3.attach(ps3Notify);
  Ps3.attachOnConnect(ps3OnConnect);
  Ps3.begin(mac); // TODO: change
}

