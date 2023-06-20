#include <Arduino.h>
#include <log.h>
#include <mpu6050_wrapper.h>
#include <ota_wrapper.h>
#include <configurations.h>
#include <ps3_wrapper.h>
#include <registers.h>
#include <internal_com.h>
#include <register_list.h>

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
uint32_t schUptimeMillisec = 0;

// ########### Functions ############

void taskControl() {

}

void taskBlink() {
  schUptimeMillisec += SCH_BLINK_PERIOD_US / 1000;
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
  regsAddRegister(REGLIST_MY(RegList_mode), &confTuning.mode, false);
  regsAddRegister(REGLIST_MY(RegList_uptime), &schUptimeMillisec, false);

  regsAddRegister(REGLIST_BODY(RegList_ctrlForw_p),   &confTuning.pid.pidNamed.bodyForward.p,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlForw_i),   &confTuning.pid.pidNamed.bodyForward.i,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlForw_d),   &confTuning.pid.pidNamed.bodyForward.d,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlForw_sat), &confTuning.pid.pidNamed.bodyForward.sat, !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlTilt_p),   &confTuning.pid.pidNamed.bodyTilt.p,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlTilt_i),   &confTuning.pid.pidNamed.bodyTilt.i,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlTilt_d),   &confTuning.pid.pidNamed.bodyTilt.d,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlTilt_sat), &confTuning.pid.pidNamed.bodyTilt.sat, !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlRota_p),   &confTuning.pid.pidNamed.bodyRotate.p,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlRota_i),   &confTuning.pid.pidNamed.bodyRotate.i,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlRota_d),   &confTuning.pid.pidNamed.bodyRotate.d,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlRota_sat), &confTuning.pid.pidNamed.bodyRotate.sat, !REGLIST_HAVE_OTA);

  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_p),   &confTuning.pid.pidNamed.neckForward.p,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_i),   &confTuning.pid.pidNamed.neckForward.i,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_d),   &confTuning.pid.pidNamed.neckForward.d,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_sat), &confTuning.pid.pidNamed.neckForward.sat, !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlTilt_p),   &confTuning.pid.pidNamed.neckTilt.p,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlTilt_i),   &confTuning.pid.pidNamed.neckTilt.i,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlTilt_d),   &confTuning.pid.pidNamed.neckTilt.d,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlTilt_sat), &confTuning.pid.pidNamed.neckTilt.sat, !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlRota_p),   &confTuning.pid.pidNamed.neckRotate.p,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlRota_i),   &confTuning.pid.pidNamed.neckRotate.i,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlRota_d),   &confTuning.pid.pidNamed.neckRotate.d,   !REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlRota_sat), &confTuning.pid.pidNamed.neckRotate.sat, !REGLIST_HAVE_OTA);
  // TODO: add further registers
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

  // Serial.println("Internal com Start...");
  // intcInit(); // TODO: test and enable

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
  // intcHandle(); // TODO: test and enable
}
