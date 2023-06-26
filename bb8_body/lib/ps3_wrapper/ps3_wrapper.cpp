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
float ps3Ypr[3] = {0,0,0};

void ps3Notify() {
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
  LOG_F("t%u\tl %i %i\tr%i %i\tb%i %i\ta%i %i %i\tg%i\n",
    millis(),
    Ps3.data.analog.stick.lx,
    Ps3.data.analog.stick.ly,
    Ps3.data.analog.stick.rx,
    Ps3.data.analog.stick.ry,
    Ps3.data.analog.button.l2,
    Ps3.data.analog.button.r2,
    Ps3.data.sensor.accelerometer.x,
    Ps3.data.sensor.accelerometer.y,
    Ps3.data.sensor.accelerometer.z,
    Ps3.data.sensor.gyroscope.z
    );
    if(Ps3.event.button_down.circle) {
      Ps3.setPlayer(2);
      Ps3.setRumble(50,500);
    }
    if(Ps3.event.button_down.square) {
      Ps3.setPlayer(3);
      Ps3.setRumble(100,200);
    }
    
}

void ps3OnConnect() {
  LOG_S("PS3 connected");
}

void ps3Initialize(const char* mac) {
  for(int i=0; i<3; i++) ps3Ypr[i] = 0;
  Ps3.attach(ps3Notify);
  Ps3.attachOnConnect(ps3OnConnect);
  Ps3.begin((char*)mac);

}

