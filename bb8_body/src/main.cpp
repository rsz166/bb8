#include <Arduino.h>
#include <log.h>
#include <mpu6050_wrapper.h>
#include <ps3_wrapper.h>

// ########### Definitions ############

#pragma region HW map

// #define PIN_LED 2
#define PIN_I2C_SDA 14
#define PIN_I2C_SCL 15
#define PIN_MPU_IT -1

#pragma endregion

#pragma region SCH definitions

#define SCH_CONTROL_PERIOD_US (1000)
#define SCH_BLINK_PERIOD_US (500000)

#pragma endregion

// ########### Variables ############

#pragma region SCH variables

uint32_t schTimeUs = 0;
uint32_t schLastControlExecutionUs = 0;
uint32_t schLastBlinkExecutionUs = 0;

#pragma endregion

#pragma region Task variables

// uint8_t blinkState = 0;

#pragma endregion

// ########### Functions ############

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
  mpuHwInit(PIN_I2C_SDA, PIN_I2C_SCL, PIN_MPU_IT);

  Serial.begin(115200);
  Serial.println("Startup...");

  if(mpuInit())
  {
    mpuLoadCalibration();
    mpuStart();
  }

  // ps3Initialize("00:00:00:00:00:00");
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
