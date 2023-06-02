#include <Arduino.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps_V6_12.h"
// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif
#include <Ps3Controller.h>

// ########### Definitions ############

#pragma region HW map

// #define PIN_LED 2
// #define PIN_MPU_INTERRUPT 2
#define PIN_I2C_SDA 14
#define PIN_I2C_SCL 15

#pragma endregion

#pragma region Support definitions

#define LOG_S(s) Serial.println(F(s))
#define LOG_N(s) Serial.println(s)

#pragma endregion

#pragma region SCH definitions

#define SCH_CONTROL_PERIOD_US (1000)
#define SCH_BLINK_PERIOD_US (500000)

#pragma endregion

#pragma region PS3 definitions

#define PS3_MAX_YAW  (100) // TODO
#define PS3_MAX_PITCH  (100) // TODO
#define PS3_MAX_ROLL  (100) // TODO
#define PS3_MAX_JOY  (100) // TODO

#pragma endregion

// ########### Variables ############

#pragma region MPU variables

// class default I2C address is 0x68
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu;
//MPU6050 mpu(0x69); // <-- use for AD0 high

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
bool mpuReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint16_t mpuPacketSize;    // expected DMP packet size (default is 42 bytes)
uint16_t mpuFifoCount;     // count of all bytes currently in FIFO
uint8_t mpuFifoBuffer[64]; // FIFO storage buffer
Quaternion mpuQ;           // [w, x, y, z]         quaternion container
VectorFloat mpuGravity;    // [x, y, z]            gravity vector
float mpuYpr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

#pragma endregion

#pragma region PS3 variables

bool ps3MotorEnable = false;
// Yaw: use momentum wheel to turn right - derivative of angle
// Pitch: rotate body backwards - angle
// Roll: tilt body right - angle
float ps3Ypr[3];

#pragma endregion

#pragma region SCH variables

uint32_t schTimeUs = 0;
uint32_t schLastControlExecutionUs = 0;
uint32_t schLastBlinkExecutionUs = 0;

#pragma endregion

#pragma region Task variables

// uint8_t blinkState = 0;

// #pragma endregion

// ########### Functions ############

#pragma region Support functions

#pragma endregion

#pragma region MPU functions

void mpuDmpDataReady() {
  mpuInterrupt = true;
}

void mpuHwInit() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 400000);
  // Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif
}

bool mpuInit() {
  LOG_S("MPU Init...");
  mpu.initialize();
  // pinMode(PIN_MPU_INTERRUPT, INPUT);
  
  LOG_S("Testing device connections...");
  if(!mpu.testConnection()) {
    LOG_S("MPU6050 connection failed");
    return false;
  }
  LOG_S("MPU6050 connection successful");
  
  LOG_S("Initializing DMP...");
  switch (mpu.dmpInitialize())
  {
  case 0:
    LOG_S("DMP init successful");
    break;
  case 1:
    LOG_S("DMP init failed: initial memory load failed");
    return false;
  case 2:
    LOG_S("DMP init failed: DMP configuration updates failed");
    return false;
  }

  return true;
}

void mpuLoadCalibration() { // TODO: fix this
  LOG_S("Loading MPU calibration...");
  // supply your own gyro offsets here, scaled for min sensitivity
  mpu.setXGyroOffset(51);
  mpu.setYGyroOffset(8);
  mpu.setZGyroOffset(21);
  mpu.setXAccelOffset(1150);
  mpu.setYAccelOffset(-50);
  mpu.setZAccelOffset(1060);
  // make sure it worked (returns 0 if so)
  // Calibration Time: generate offsets and calibrate our MPU6050
  mpu.CalibrateAccel(6);
  mpu.CalibrateGyro(6);
  Serial.println();
  mpu.PrintActiveOffsets();
  // turn on the DMP, now that it's ready
  LOG_S("Enabling DMP...");
  mpu.setDMPEnabled(true);
}

void mpuStart() {
  LOG_S("Starting MPU processing...");
  // attachInterrupt(digitalPinToInterrupt(PIN_MPU_INTERRUPT), mpuDmpDataReady, RISING);
  mpuReady = true;
  mpuPacketSize = mpu.dmpGetFIFOPacketSize();
}

bool mpuTryRead() {
  if(!mpuReady) return false;
  if(!mpu.dmpGetCurrentFIFOPacket(mpuFifoBuffer)) return false;
  mpu.dmpGetQuaternion(&mpuQ, mpuFifoBuffer);
  mpu.dmpGetGravity(&mpuGravity, &mpuQ);
  mpu.dmpGetYawPitchRoll(mpuYpr, &mpuQ, &mpuGravity);
  LOG_S("ypr\t");
  LOG_N(mpuYpr[0] * 180 / M_PI);
  LOG_S("\t");
  LOG_N(mpuYpr[1] * 180 / M_PI);
  LOG_S("\t");
  LOG_N(mpuYpr[2] * 180 / M_PI);
  return true;
}

#pragma endregion

#pragma region PS3 functions

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

void ps3Initialize() {
  for(int i=0; i<3; i++) ps3Ypr[i] = 0;
  Ps3.attach(ps3Notify);
  Ps3.attachOnConnect(ps3OnConnect);
  Ps3.begin("00:00:00:00:00:00"); // TODO: change
}

#pragma endregion

#pragma region Tasks

void taskControl() {

}

void taskBlink() {
  // digitalWrite(PIN_LED, blinkState);
  // blinkState ^= 1;
}

#pragma endregion

#pragma region SCH functions

void schRun() {
  schTimeUs = micros();
  if((schTimeUs - schLastControlExecutionUs) >= SCH_CONTROL_PERIOD_US) {
    taskControl();
    schLastControlExecutionUs += SCH_CONTROL_PERIOD_US;
  }
  else if((schTimeUs - schLastBlinkExecutionUs) >= SCH_BLINK_PERIOD_US) {
    taskBlink();
    schLastBlinkExecutionUs += SCH_BLINK_PERIOD_US;
  }
}

#pragma endregion

// ########### Entry points ############

void setup() {
  mpuHwInit();

  Serial.begin(115200);
  Serial.println("Startup...");

  if(mpuInit())
  {
    mpuLoadCalibration();
    mpuStart();
  }

  // ps3Initialize();
}

uint32_t loopCnt = 0;
void loop() {
  // // run scheduler
  schRun();
  // // perform async actions
  if(mpuTryRead()) {
    // TODO: new data available
  }
}
