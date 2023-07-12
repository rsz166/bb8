#include <ps3_wrapper.h>
#include <Arduino.h>
#include <log.h>
#include <Ps3Controller.h>

#define PS3_MAX_YPR  (100)
#define PS3_MAX_JOY  (100) // joy values are -128..+127, but not all joy can reach these values, therefore it's limited to smaller range
#define PS3_MIN_JOY  (20)


bool ps3MotorEnable = false;
// Yaw: use momentum wheel to turn right - derivative of angle
// Pitch: rotate body forward - angle
// Roll: tilt body right - angle
float ps3Ypr_body[3] = {0,0,0};
float ps3Ypr_neck[3] = {0,0,0};
float ps3Battery = 0;
float ps3BodyZeroOffset[3];
float ps3NeckZeroOffset[3];

float ps3Sign(float val) {
  return val >= 0 ? 1 : -1;
}

float ps3Scale(float val) {
  if(val <= PS3_MIN_JOY && val >= -PS3_MIN_JOY) return 0;
  if(val > PS3_MAX_JOY) return PS3_MAX_YPR;
  if(val < -PS3_MAX_JOY) return -PS3_MAX_YPR;
  return (val - (ps3Sign(val) * PS3_MIN_JOY)) / (PS3_MAX_JOY - PS3_MIN_JOY) * PS3_MAX_YPR;
}

// #define PS3_BODY_Y (Ps3.data.analog.stick.lx)
#define PS3_BODY_P (-Ps3.data.analog.stick.ly)
#define PS3_BODY_R (Ps3.data.analog.stick.lx)
// #define PS3_NECK_Y (Ps3.data.analog.stick.rx)
#define PS3_NECK_P (-Ps3.data.analog.stick.ry)
#define PS3_NECK_R (Ps3.data.analog.stick.rx)

// TODO: handle disconnect and timeout
void ps3Notify() {
  // hold R1 or L1 to enable movement
  bool enable = (Ps3.data.button.l1 || Ps3.data.button.r1);
  if(enable != ps3MotorEnable) {
    if(enable) {
      // save zero offset
      #ifdef PS3_BODY_Y
      ps3BodyZeroOffset[0] = PS3_BODY_Y;
      #endif
      #ifdef PS3_BODY_P
      ps3BodyZeroOffset[1] = PS3_BODY_P;
      #endif
      #ifdef PS3_BODY_R
      ps3BodyZeroOffset[2] = PS3_BODY_R;
      #endif
      #ifdef PS3_NECK_Y
      ps3NeckZeroOffset[0] = PS3_NECK_Y;
      #endif
      #ifdef PS3_NECK_P
      ps3NeckZeroOffset[1] = PS3_NECK_P;
      #endif
      #ifdef PS3_NECK_R
      ps3NeckZeroOffset[2] = PS3_NECK_R;
      #endif
    } else {
      // clear output
      for(int i=0; i<3; i++) ps3Ypr_body[i] = 0;
    }
  }
  ps3MotorEnable = enable;
  if(ps3MotorEnable) {
    // adjust yaw/pith/roll with joys
    #ifdef PS3_BODY_Y
    ps3Ypr_body[0] = ps3Scale(PS3_BODY_Y);
    #endif
    #ifdef PS3_BODY_P
    ps3Ypr_body[1] = ps3Scale(PS3_BODY_P);
    #endif
    #ifdef PS3_BODY_R
    ps3Ypr_body[2] = ps3Scale(PS3_BODY_R);
    #endif
    #ifdef PS3_NECK_Y
    ps3Ypr_neck[0] = ps3Scale(PS3_NECK_Y);
    #endif
    #ifdef PS3_NECK_P
    ps3Ypr_neck[1] = ps3Scale(PS3_NECK_P);
    #endif
    #ifdef PS3_NECK_R
    ps3Ypr_neck[2] = ps3Scale(PS3_NECK_R);
    #endif
  }
  if(Ps3.data.status.battery == ps3_status_battery_charging) {
    ps3Battery = -1;
  } else {
    ps3Battery = (Ps3.data.status.battery - 1) * 25;
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
  for(int i=0; i<3; i++) ps3Ypr_body[i] = 0;
  Ps3.attach(ps3Notify);
  Ps3.attachOnConnect(ps3OnConnect);
  Ps3.begin((char*)mac);
}
