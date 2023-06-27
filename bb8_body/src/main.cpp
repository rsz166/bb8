#include <Arduino.h>
#include <log.h>
#include <mpu6050_wrapper.h>
#include <ota_wrapper.h>
#include <configurations.h>
#include <ps3_wrapper.h>
#include <stepper_wrapper.h>
#include <registers.h>
#include <internal_com.h>
#include <register_list.h>
#include <control.h>

// ########### Definitions ############

// HW map
#define PIN_I2C_SDA 14
#define PIN_I2C_SCL 15
#define PIN_MPU_IT -1

// SCH definitions
#define SCH_CONTROL_PERIOD_US (1000)
#define SCH_BLINK_PERIOD_US (500000)
#define SCH_MODE_CHANGE_PERIOD_US (500000)
#define SCH_UPTIME_PERIOD_US (100000)

#define SCH_BUTTON_HOLD_MS (1500)

// ########### Variables ############

uint32_t schTimeUs = 0;
uint32_t schLastControlExecutionUs = 0;
uint32_t schLastBlinkExecutionUs = 0;
uint32_t schLastModeChangeExecutionUs = 0;
uint32_t schLastUptimeExecutionUs = 0;

uint32_t schUptimeMillisec = 0;
uint32_t schButtonDownTime = 0;

// ########### Functions ############

void taskControl() {
  // conHandle(); // TODO: test and enable
  // otaHandle();
}

void taskModeChange() {
  // check button
  if(!digitalRead(0)) {
    if(schButtonDownTime == 0) {
      schButtonDownTime = schUptimeMillisec;
    }
    else if((schUptimeMillisec - schButtonDownTime) >= SCH_BUTTON_HOLD_MS) {
      if(confDevConf.mode != CONF_MODE_WIFI) {
        confDevConf.mode = CONF_MODE_WIFI;
        confWrite();
        delay(500);
        ESP.restart();
      }
    }
  } else {
    schButtonDownTime = 0;
  }
  // check remote command
  if(*regsRegisters[REGLIST_MY(RegList_requestedMode)].data.pi != 0) {
    int mode = *regsRegisters[REGLIST_MY(RegList_requestedMode)].data.pi;
    if(mode != confDevConf.mode) {
      if(mode == CONF_MODE_BT || mode == CONF_MODE_WIFI) {
        confDevConf.mode = mode;
        confWrite();
        ESP.restart();
      } else if(mode == 0xffffffff) {
        ESP.restart();
      }
    }
  }
}

void taskUptime() {
  schUptimeMillisec += SCH_BLINK_PERIOD_US / 1000;
}

void taskBlink() {
}

void schRun() {
  schTimeUs = micros();
  if((schTimeUs - schLastModeChangeExecutionUs) >= SCH_MODE_CHANGE_PERIOD_US) {
    taskModeChange();
    schLastModeChangeExecutionUs += SCH_MODE_CHANGE_PERIOD_US;
  } else if((schTimeUs - schLastControlExecutionUs) >= SCH_CONTROL_PERIOD_US) {
    taskControl();
    schLastControlExecutionUs += SCH_CONTROL_PERIOD_US;
  } else if((schTimeUs - schLastUptimeExecutionUs) >= SCH_UPTIME_PERIOD_US) {
    taskUptime();
    schLastUptimeExecutionUs += SCH_UPTIME_PERIOD_US;
  } else if((schTimeUs - schLastBlinkExecutionUs) >= SCH_BLINK_PERIOD_US) {
    taskBlink();
    schLastBlinkExecutionUs += SCH_BLINK_PERIOD_US;
  }
}

void registerRegisters() {
  regsAddRegister(REGLIST_MY(RegList_mode), &confDevConf.mode, true);
  regsAddRegister(REGLIST_MY(RegList_uptime), &schUptimeMillisec, true);
  // regsAddRegister(REGLIST_MY(RegList_errorCode), &??, true);
  // regsAddRegister(REGLIST_MY(RegList_batteryVoltage), &??, true);
  // regsAddRegister(REGLIST_MY(RegList_requestedMode), &??, false);
  
  regsAddRegister(REGLIST_BODY(RegList_ctrlForw_p),   &confSysTuning.pids.pidNamed.bodyForward.p,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlForw_i),   &confSysTuning.pids.pidNamed.bodyForward.i,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlForw_d),   &confSysTuning.pids.pidNamed.bodyForward.d,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlForw_sat), &confSysTuning.pids.pidNamed.bodyForward.sat, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlTilt_p),   &confSysTuning.pids.pidNamed.bodyTilt.p,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlTilt_i),   &confSysTuning.pids.pidNamed.bodyTilt.i,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlTilt_d),   &confSysTuning.pids.pidNamed.bodyTilt.d,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlTilt_sat), &confSysTuning.pids.pidNamed.bodyTilt.sat, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlRota_p),   &confSysTuning.pids.pidNamed.bodyRotate.p,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlRota_i),   &confSysTuning.pids.pidNamed.bodyRotate.i,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlRota_d),   &confSysTuning.pids.pidNamed.bodyRotate.d,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlRota_sat), &confSysTuning.pids.pidNamed.bodyRotate.sat, REGLIST_HAVE_OTA);

  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_p),   &confSysTuning.pids.pidNamed.neckForward.p,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_i),   &confSysTuning.pids.pidNamed.neckForward.i,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_d),   &confSysTuning.pids.pidNamed.neckForward.d,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_sat), &confSysTuning.pids.pidNamed.neckForward.sat, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlTilt_p),   &confSysTuning.pids.pidNamed.neckTilt.p,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlTilt_i),   &confSysTuning.pids.pidNamed.neckTilt.i,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlTilt_d),   &confSysTuning.pids.pidNamed.neckTilt.d,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlTilt_sat), &confSysTuning.pids.pidNamed.neckTilt.sat, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlRota_p),   &confSysTuning.pids.pidNamed.neckRotate.p,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlRota_i),   &confSysTuning.pids.pidNamed.neckRotate.i,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlRota_d),   &confSysTuning.pids.pidNamed.neckRotate.d,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlRota_sat), &confSysTuning.pids.pidNamed.neckRotate.sat, REGLIST_HAVE_OTA);
  // TODO: debug only
  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_act),   &ps3Ypr[2],  REGLIST_HAVE_REMOTE);
}

// ########### Entry points ############

void setup() {
  mpuHwInit(PIN_I2C_SDA, PIN_I2C_SCL, PIN_MPU_IT);
  regsInit();

  Serial.begin(115200);
  Serial.println("Startup...");

  pinMode(0, INPUT);

  confInit();

  Serial.println("MPU Start...");
  if(mpuInit())
  {
    mpuLoadCalibration();
    mpuStart();
  }

  if(confDevConf.mode != CONF_MODE_BT) {
    Serial.println("OTA Start...");
    if(otaNetworkInitSTA(confDevConf.wifiSsid.c_str(), confDevConf.wifiPass.c_str())) {
      otaInit();
    } else if(otaNetworkInitAP("ESP")) {
      Serial.println("OTA in AP mode");
      otaInit();
    } else {
      Serial.println("OTA cannot initialize network");
    }
  } else {
    Serial.println("PS3 Start...");
    ps3Initialize(confDevConf.btMac.c_str());
  }
  
  Serial.println("Stepper Start...");
  stepInit(); // TODO: wait for sys config to be downloaded

  Serial.println("Regs Start...");
  registerRegisters();

  Serial.println("Internal com Start...");
  intcInit();

  // Serial.println("Control Start..."); 
  // conInit(); // TODO: test and enable

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
  intcHandle();
  stepHandle();
  // stepperSpeed = ps3Ypr[2]; // TODO: debug only
}
