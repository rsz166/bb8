#include <Arduino.h>
#include <log.h>
#include <mpu6050_wrapper.h>
#include <ps3_wrapper.h>
#include <ota_wrapper.h>
#include <configurations.h>

// ########### Definitions ############

// HW map
#define PIN_I2C_SDA 14
#define PIN_I2C_SCL 15
#define PIN_MPU_IT -1

// SCH definitions
#define SCH_CONTROL_PERIOD_US (1000)
#define SCH_BLINK_PERIOD_US (500000)

// WIFI
#define WIFI_SSID "..."
#define WIFI_PASS "..."

// ########### Variables ############

uint32_t schTimeUs = 0;
uint32_t schLastControlExecutionUs = 0;
uint32_t schLastBlinkExecutionUs = 0;

// ########### Functions ############

void taskControl() {

}

void taskBlink() {
  // LOG_N(millis());
}

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

// ########### Entry points ############

void setup() {
  mpuHwInit(PIN_I2C_SDA, PIN_I2C_SCL, PIN_MPU_IT);

  Serial.begin(115200);
  Serial.println("Startup...");

  confInit();

  Serial.println("OTA Start...");
  if(otaNetworkInitSTA(WIFI_SSID, WIFI_PASS)) {
    otaInit();
  }

  Serial.println("MPU Start...");
  if(mpuInit())
  {
    mpuLoadCalibration();
    mpuStart();
  }

  // Serial.println("PS3 Start...");
  // ps3Initialize("00:00:00:00:00:00");
}

uint32_t loopCnt = 0;
void loop() {
  // run scheduler
  schRun();
  // perform async actions
  if(mpuTryRead()) {
    // TODO: new data available
  }
}
