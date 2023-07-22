#include <ps3_wrapper.h>
#include <Arduino.h>
#include <log.h>
#include <Ps3Controller.h>

#define PS3_MAX_OUTPUT  (100)
#define PS3_TIMEOUT_US  (200000)
#define PS3_TIMEOUT_RESET_US  (2000000)
#define PS3_ENABLE_US  (2000000)

#define PS3_BODY_F (-Ps3.data.analog.stick.ly)
#define PS3_BODY_T (Ps3.data.analog.stick.lx)
// #define PS3_BODY_R (Ps3.data.analog.stick.lx)
#define PS3_NECK_F (-Ps3.data.analog.stick.ry)
#define PS3_NECK_T (Ps3.data.analog.stick.rx)
// #define PS3_NECK_R (Ps3.data.analog.stick.rx)

void ps3Notify();
void ps3OnConnect();
float ps3Sign(float val);
float ps3Scale(float val);
void ps3SaveOffset();
void ps3UpdateMotors();
void ps3UpdateTimeout();
void ps3SetEnable(bool enable);

// Forward: rotate body forward - angle
// Tilt: tilt body right - angle
// Rotation: use momentum wheel to turn right - derivative of angle
float ps3Ftr_body[3] = {0,0,0};
float ps3Ftr_neck[3] = {0,0,0};
uint32_t ps3MotorEnable = 0;
float ps3Battery = 0;
float ps3BodyZeroOffset[3] = {0,0,0};
float ps3NeckZeroOffset[3] = {0,0,0};
volatile unsigned long ps3UpdateMicros = 0;
bool ps3Timeout = true, ps3Connected;
char* ps3Mac;
int *ps3Min, *ps3Max;
unsigned long ps3HoldTimerMicros = 0;
bool ps3EnablePending = false;

float ps3Sign(float val) {
  return val >= 0 ? 1 : -1;
}

float ps3Scale(float val) {
  // joy values are -128..+127, but not all joy can reach these values, therefore it's limited to smaller range
  int min = *ps3Min;
  int max = *ps3Max;
  if(val <= min && val >= -min) return 0;
  if(val > max) return PS3_MAX_OUTPUT;
  if(val < -max) return -PS3_MAX_OUTPUT;
  return (val - (ps3Sign(val) * min)) / (max - min) * PS3_MAX_OUTPUT;
}

void ps3SaveOffset() {
  // save zero offset
  #ifdef PS3_BODY_F
  ps3BodyZeroOffset[0] = PS3_BODY_F;
  #endif
  #ifdef PS3_BODY_T
  ps3BodyZeroOffset[1] = PS3_BODY_T;
  #endif
  #ifdef PS3_BODY_R
  ps3BodyZeroOffset[2] = PS3_BODY_R;
  #endif
  #ifdef PS3_NECK_F
  ps3NeckZeroOffset[0] = PS3_NECK_F;
  #endif
  #ifdef PS3_NECK_T
  ps3NeckZeroOffset[1] = PS3_NECK_T;
  #endif
  #ifdef PS3_NECK_R
  ps3NeckZeroOffset[2] = PS3_NECK_R;
  #endif
}

void ps3UpdateMotors() {
    // get yaw/pith/roll from joys
    #ifdef PS3_BODY_F
    ps3Ftr_body[0] = ps3Scale(PS3_BODY_F-ps3BodyZeroOffset[0]);
    #endif
    #ifdef PS3_BODY_T
    ps3Ftr_body[1] = ps3Scale(PS3_BODY_T-ps3BodyZeroOffset[1]);
    #endif
    #ifdef PS3_BODY_R
    ps3Ftr_body[2] = ps3Scale(PS3_BODY_R-ps3BodyZeroOffset[2]);
    #endif
    #ifdef PS3_NECK_F
    ps3Ftr_neck[0] = ps3Scale(PS3_NECK_F-ps3NeckZeroOffset[0]);
    #endif
    #ifdef PS3_NECK_T
    ps3Ftr_neck[1] = ps3Scale(PS3_NECK_T-ps3NeckZeroOffset[1]);
    #endif
    #ifdef PS3_NECK_R
    ps3Ftr_neck[2] = ps3Scale(PS3_NECK_R-ps3NeckZeroOffset[2]);
    #endif
}

void ps3UpdateTimeout() {
  ps3UpdateMicros = micros();
  ps3Timeout = false;
}

void ps3SetEnable(bool enable) {
  if(enable) {
    ps3SaveOffset();
    Ps3.setPlayer(10);
  } else {
    // clear output
    for(int i=0; i<3; i++) ps3Ftr_body[i] = 0;
    for(int i=0; i<3; i++) ps3Ftr_neck[i] = 0;
    Ps3.setPlayer(5);
  }
  ps3MotorEnable = enable;
}

void ps3HandleEnable() {
  if(ps3MotorEnable) {
    if(Ps3.data.button.circle) {
      ps3SetEnable(false);
    }
  } else {
    if(Ps3.data.button.r1 && Ps3.data.button.triangle) {
      if(ps3EnablePending) {
        if((micros() - ps3HoldTimerMicros) >= PS3_ENABLE_US) {
          ps3SetEnable(true);
          ps3EnablePending = false;
        }
      } else {
        ps3EnablePending = true;
        ps3HoldTimerMicros = micros();
      }
    } else {
      ps3EnablePending = false;
    }
  }
}

void ps3Notify() {
  ps3HandleEnable();
  if(ps3MotorEnable) {
    ps3UpdateMotors();
  }
  if(Ps3.data.status.battery == ps3_status_battery_charging) {
    ps3Battery = -1;
  } else {
    ps3Battery = (Ps3.data.status.battery - 1) * 25;
  }
  ps3UpdateTimeout();
  // LOG_F("t%u\tl %i %i\tr%i %i\tb%i %i\ta%i %i %i\tg%i\n",
  //   millis(),
  //   Ps3.data.analog.stick.lx,
  //   Ps3.data.analog.stick.ly,
  //   Ps3.data.analog.stick.rx,
  //   Ps3.data.analog.stick.ry,
  //   Ps3.data.analog.button.l2,
  //   Ps3.data.analog.button.r2,
  //   Ps3.data.sensor.accelerometer.x,
  //   Ps3.data.sensor.accelerometer.y,
  //   Ps3.data.sensor.accelerometer.z,
  //   Ps3.data.sensor.gyroscope.z
  //   );
}

void ps3OnConnect() {
  ps3UpdateMicros = micros();
  ps3Connected = true;
  LOG_S("PS3 connected");
}

void ps3OnTimeout() {
  if(ps3MotorEnable) {
    ps3SetEnable(false);
  }
  Ps3.end();
  ps3Battery = -2;
  Ps3.begin(ps3Mac);
  LOG_S("PS3 disconnected, reset in 2 sec");
}

void ps3Initialize(const char* mac, int* min, int* max) {
  for(int i=0; i<3; i++) ps3Ftr_body[i] = 0;
  Ps3.attach(ps3Notify);
  Ps3.attachOnConnect(ps3OnConnect);
  ps3Mac = (char*)mac;
  ps3Min = min;
  ps3Max = max;
  Ps3.begin(ps3Mac);
}

void ps3Handle() {
  if(ps3Connected) {
    unsigned long timeout = micros() - ps3UpdateMicros;
    if(!ps3Timeout) {
      if(timeout > PS3_TIMEOUT_US) {
        ps3Timeout = true;
        ps3OnTimeout();
      }
    } else {
      if(timeout > PS3_TIMEOUT_RESET_US) {
        ESP.restart();
      }
    }
  }
}
