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

float mpuAccel[3],mpuGyro[3];           // [x,y,z]
float mpuGravity[3];
float mpuTilt[3];

void mpuHwInit(int sda, int scl) {
    // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin(sda, scl, 400000);
  // Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif
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

  return true;
}

void mpuHandle() {
  int16_t ax,ay,az,gx,gy,gz;
  // mpu.getMotion6(&ax,&ay,&az,&gx,&gy,&gz);
  // LOG_F("mpu:\t%i\t%i\t%i\t%i\t%i\t%i\n",ax,ay,az,gx,gy,gz);
}
