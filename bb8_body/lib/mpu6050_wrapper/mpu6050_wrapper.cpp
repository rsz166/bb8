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

#define MPU_AXIS_UP     (mpuAccel[0]) // TODO: map correct axes based on mounting position
#define MPU_AXIS_RIGHT  (mpuAccel[1])
#define MPU_AXIS_FORW   (mpuAccel[2])
#define MPU_TIMEOUT_US  (100000)

// class default I2C address is 0x68
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu;
//MPU6050 mpu(0x69); // <-- use for AD0 high

float mpuAccel[3],mpuGyro[3];           // [x,y,z]
float mpuTilt[2];                       // [forward-backward-vertical plane from vertical (+ is tilt forward), right-left-vertical plane from vertical (+ is tilt right)]
uint32_t mpuLastSuccess;
bool mpuTimeoutFlg;

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
  mpuLastSuccess = micros();
  mpuTimeoutFlg = false;

  return true;
}

void mpuHandle() {
  int16_t ax,ay,az;
  // int16_t gx,gy,gz;
  if(!mpu.testConnection()) {
    // mpu.getMotion6(&ax,&ay,&az,&gx,&gy,&gz);
    mpu.getAcceleration(&ax,&ay,&az);
    // LOG_F("mpu:\t%i\t%i\t%i\n",ax,ay,az);
    // LOG_F("mpu:\t%i\t%i\t%i\t%i\t%i\t%i\n",ax,ay,az,gx,gy,gz);
    mpuAccel[0] = ax;
    mpuAccel[1] = ay;
    mpuAccel[2] = az;
    mpuTilt[0] = atan2(MPU_AXIS_FORW,-MPU_AXIS_UP); // TODO: use tilt offset calibration
    mpuTilt[1] = atan2(MPU_AXIS_RIGHT,-MPU_AXIS_UP);
    mpuLastSuccess = micros();
    mpuTimeoutFlg = false;
  } else {
    if((micros() - mpuLastSuccess) > MPU_TIMEOUT_US) {
      mpuTimeoutFlg = true;
    }
  }
}
