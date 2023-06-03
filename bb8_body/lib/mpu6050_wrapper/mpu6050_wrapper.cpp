#include <mpu6050_wrapper.h>
#include <Arduino.h>
#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps_V6_12.h>
// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include <Wire.h>
#endif
#include <log.h>

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
int interruptPin = -1;

void mpuDmpDataReady() {
  mpuInterrupt = true;
}

void mpuHwInit(int sda, int scl, int interrupt) {
    // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin(sda, scl, 400000);
  // Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif
  if(interrupt >= 0) {
    pinMode(interrupt, INPUT);
  }
}

bool mpuInit() {
  LOG_S("MPU Init...");
  mpu.initialize();
  
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
  if(interruptPin >= 0) {
    attachInterrupt(digitalPinToInterrupt(interruptPin), mpuDmpDataReady, RISING);
  }
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
