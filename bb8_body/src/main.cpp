#include <Arduino.h>
#include <log.h>
#include <mpu6050_wrapper.h>
#include <ota_wrapper.h>
#include <configurations.h>
#include <ps3_wrapper.h>
#include <registers.h>

// ########### Definitions ############

// HW map
#define PIN_I2C_SDA 14
#define PIN_I2C_SCL 15
#define PIN_MPU_IT -1

// SCH definitions
#define SCH_CONTROL_PERIOD_US (1000)
#define SCH_BLINK_PERIOD_US (500000)

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

void registerRegisters() {
  regsAddRegister(0, &confTuning.mode);
  for(int i=0; i<CONF_PID_COUNT; i++) {
    regsAddRegister(1+4*CONF_PID_COUNT, &confTuning.pid.pidArray[i].p);
    regsAddRegister(2+4*CONF_PID_COUNT, &confTuning.pid.pidArray[i].i);
    regsAddRegister(3+4*CONF_PID_COUNT, &confTuning.pid.pidArray[i].d);
    regsAddRegister(4+4*CONF_PID_COUNT, &confTuning.pid.pidArray[i].sat);
  }
}

// ########### Entry points ############

void setup() {
  mpuHwInit(PIN_I2C_SDA, PIN_I2C_SCL, PIN_MPU_IT);
  regsInit();

  Serial.begin(115200);
  Serial.println("Startup...");

  confInit();

  Serial.println("MPU Start...");
  if(mpuInit())
  {
    mpuLoadCalibration();
    mpuStart();
  }

  if(confTuning.mode != CONF_MODE_BT) {
    Serial.println("OTA Start...");
    if(otaNetworkInitSTA(confAuth.wifiSsid.c_str(), confAuth.wifiPass.c_str())) {
      otaInit();
    } else if(otaNetworkInitAP("ESP")) {
      Serial.println("OTA in AP mode");
      otaInit();
    } else {
      Serial.println("OTA cannot initialize network");
    }
  } else {
    Serial.println("PS3 Start...");
    ps3Initialize(confAuth.btMac.c_str());
  }
  
  Serial.println("Regs Start...");
  registerRegisters();

  Serial.println("Ready.");
}

// uint32_t loopCnt = 0;
void loop() {
  // run scheduler
  schRun();
  // perform async actions
  if(mpuTryRead()) {
    // TODO: new data available
  }
}
