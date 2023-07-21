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
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22

// SCH definitions
#define SCH_CONTROL_PERIOD_US (10000)
#define SCH_BLINK_PERIOD_US (250000)
#define SCH_MODE_CHANGE_PERIOD_US (500000)
#define SCH_UPTIME_PERIOD_US (100000)
#define SCH_DATASTREAM_PERIOD_US (250000)

#define SCH_BUTTON_HOLD_MS (1500)

// ########### Variables ############

uint32_t schTimeUs = 0;
uint32_t schLastControlExecutionUs = 0;
uint32_t schLastBlinkExecutionUs = 0;
uint32_t schLastModeChangeExecutionUs = 0;
uint32_t schLastUptimeExecutionUs = 0;
uint32_t schLastDataStreamExecutionUs = 0;
uint8_t schmBlinkCounter = 0;

uint32_t schUptimeMillisec = 0;
uint32_t schButtonDownTime = 0;
bool schIsInitBt = false;
uint32_t schmStatusFlg = 0;

// ########### Functions ############

void taskControl() {
  mpuHandle();
  conHandle();
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
  int mode = *regsRegisters[REGLIST_MY(RegList_requestedMode)].data.pi;
  if(mode != 0) {
    if(mode != confDevConf.mode) {
      if(mode == CONF_MODE_BT || mode == CONF_MODE_WIFI) {
        confDevConf.mode = mode;
        confWrite();
        ESP.restart();
      } else if(mode == 0xff) {
        ESP.restart();
      }
    }
  }
}

void taskUptime() {
  schUptimeMillisec += SCH_BLINK_PERIOD_US / 1000;
}

void taskBlink() {
  int mode = *regsRegisters[REGLIST_MY(RegList_mode)].data.pi;
  digitalWrite(BUILTIN_LED, (schmBlinkCounter++ & mode) ? HIGH : LOW);
}

void taskDataStream() {
  if(!schIsInitBt) {
    otaHandle();
  }
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
  } else if((schTimeUs - schLastDataStreamExecutionUs) >= SCH_DATASTREAM_PERIOD_US) {
    taskDataStream();
    schLastDataStreamExecutionUs += SCH_DATASTREAM_PERIOD_US;
  }
}

void registerRegisters() {
  regsAddRegister(REGLIST_MY(RegList_mode), &confDevConf.mode, true);
  regsAddRegister(REGLIST_MY(RegList_uptime), &schUptimeMillisec, true);
  regsAddRegister(REGLIST_MY(RegList_errorCode), nullptr, true);
  // regsAddRegister(REGLIST_MY(RegList_batteryVoltage), &??, true);
  regsAddRegister(REGLIST_MY(RegList_requestedMode), nullptr, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_OTHER(RegList_requestedMode), nullptr, REGLIST_HAVE_OTA);
  if(REGLIST_HAVE_REMOTE) {
    regsAddRegister(REGLIST_MY(RegList_enableMotors), &ps3MotorEnable, true);
    regsAddRegister(REGLIST_OTHER(RegList_enableMotors), &ps3MotorEnable, true);
  }
  regsAddRegister(REGLIST_MY(RegList_status), &schmStatusFlg,  true);
  
  regsAddRegister(REGLIST_BODY(RegList_ctrlForw_p),   &confSysTuning.pids.pidNamed.bodyForward.p,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlForw_i),   &confSysTuning.pids.pidNamed.bodyForward.i,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlForw_d),   &confSysTuning.pids.pidNamed.bodyForward.d,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlForw_sat), &confSysTuning.pids.pidNamed.bodyForward.sat, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlForw_fbgain), &confSysTuning.pids.pidNamed.bodyForward.fbgain, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlForw_isOpenLoop), &confSysTuning.pids.pidNamed.bodyForward.isOpenLoop, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlTilt_p),   &confSysTuning.pids.pidNamed.bodyTilt.p,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlTilt_i),   &confSysTuning.pids.pidNamed.bodyTilt.i,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlTilt_d),   &confSysTuning.pids.pidNamed.bodyTilt.d,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlTilt_sat), &confSysTuning.pids.pidNamed.bodyTilt.sat, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlTilt_fbgain), &confSysTuning.pids.pidNamed.bodyTilt.fbgain, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlTilt_isOpenLoop), &confSysTuning.pids.pidNamed.bodyTilt.isOpenLoop, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlRota_p),   &confSysTuning.pids.pidNamed.bodyRotate.p,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlRota_i),   &confSysTuning.pids.pidNamed.bodyRotate.i,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlRota_d),   &confSysTuning.pids.pidNamed.bodyRotate.d,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlRota_sat), &confSysTuning.pids.pidNamed.bodyRotate.sat, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlRota_fbgain), &confSysTuning.pids.pidNamed.bodyRotate.fbgain, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlRota_isOpenLoop), &confSysTuning.pids.pidNamed.bodyRotate.isOpenLoop, REGLIST_HAVE_OTA);

  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_p),   &confSysTuning.pids.pidNamed.neckForward.p,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_i),   &confSysTuning.pids.pidNamed.neckForward.i,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_d),   &confSysTuning.pids.pidNamed.neckForward.d,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_sat), &confSysTuning.pids.pidNamed.neckForward.sat, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_fbgain), &confSysTuning.pids.pidNamed.neckForward.fbgain, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_isOpenLoop), &confSysTuning.pids.pidNamed.neckForward.isOpenLoop, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlTilt_p),   &confSysTuning.pids.pidNamed.neckTilt.p,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlTilt_i),   &confSysTuning.pids.pidNamed.neckTilt.i,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlTilt_d),   &confSysTuning.pids.pidNamed.neckTilt.d,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlTilt_sat), &confSysTuning.pids.pidNamed.neckTilt.sat, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlTilt_fbgain), &confSysTuning.pids.pidNamed.neckTilt.fbgain, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlTilt_isOpenLoop), &confSysTuning.pids.pidNamed.neckTilt.isOpenLoop, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlRota_p),   &confSysTuning.pids.pidNamed.neckRotate.p,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlRota_i),   &confSysTuning.pids.pidNamed.neckRotate.i,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlRota_d),   &confSysTuning.pids.pidNamed.neckRotate.d,   REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlRota_sat), &confSysTuning.pids.pidNamed.neckRotate.sat, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlRota_fbgain), &confSysTuning.pids.pidNamed.neckRotate.fbgain, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlRota_isOpenLoop), &confSysTuning.pids.pidNamed.neckRotate.isOpenLoop, REGLIST_HAVE_OTA);

  regsAddRegister(REGLIST_MY(RegList_ctrlForw_act),   &stepControls[0].setpoint,  true);
  regsAddRegister(REGLIST_MY(RegList_ctrlTilt_act),   &stepControls[1].setpoint,  true);
  regsAddRegister(REGLIST_MY(RegList_ctrlRota_act),   &stepControls[2].setpoint,  true);
  regsAddRegister(REGLIST_MY(RegList_ctrlForw_feedback),   &mpuTilt[0],  true);
  regsAddRegister(REGLIST_MY(RegList_ctrlTilt_feedback),   &mpuTilt[1],  true);
  // regsAddRegister(REGLIST_MY(RegList_ctrlRota_feedback),   ??,  true); // no feedback, rotation works in open loop
  regsAddRegister(REGLIST_BODY(RegList_ctrlForw_setp),   &ps3Ftr_body[0],  REGLIST_HAVE_REMOTE);
  regsAddRegister(REGLIST_BODY(RegList_ctrlTilt_setp),   &ps3Ftr_body[1],  REGLIST_HAVE_REMOTE);
  regsAddRegister(REGLIST_BODY(RegList_ctrlRota_setp),   &ps3Ftr_body[2],  REGLIST_HAVE_REMOTE);
  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_setp),   &ps3Ftr_neck[0],  REGLIST_HAVE_REMOTE);
  regsAddRegister(REGLIST_NECK(RegList_ctrlTilt_setp),   &ps3Ftr_neck[1],  REGLIST_HAVE_REMOTE);
  regsAddRegister(REGLIST_NECK(RegList_ctrlRota_setp),   &ps3Ftr_neck[2],  REGLIST_HAVE_REMOTE);
  if(REGLIST_HAVE_REMOTE) {
    regsAddRegister(REGLIST_MY(RegList_batteryVoltage), &ps3Battery, true);
  }

  regsAddRegister(REGLIST_BODY(RegList_ctrlForw_speedLimit), &confSysTuning.motors.motNamed.bodyForward.speed, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlTilt_speedLimit), &confSysTuning.motors.motNamed.bodyTilt.speed, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlRota_speedLimit), &confSysTuning.motors.motNamed.bodyRotate.speed, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_speedLimit), &confSysTuning.motors.motNamed.neckForward.speed, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlTilt_speedLimit), &confSysTuning.motors.motNamed.neckTilt.speed, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlRota_speedLimit), &confSysTuning.motors.motNamed.neckRotate.speed, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlForw_accelLimit), &confSysTuning.motors.motNamed.bodyForward.accel, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlTilt_accelLimit), &confSysTuning.motors.motNamed.bodyTilt.accel, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_BODY(RegList_ctrlRota_accelLimit), &confSysTuning.motors.motNamed.bodyRotate.accel, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlForw_accelLimit), &confSysTuning.motors.motNamed.neckForward.accel, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlTilt_accelLimit), &confSysTuning.motors.motNamed.neckTilt.accel, REGLIST_HAVE_OTA);
  regsAddRegister(REGLIST_NECK(RegList_ctrlRota_accelLimit), &confSysTuning.motors.motNamed.neckRotate.accel, REGLIST_HAVE_OTA);
  
  regsAddRegister(REGLIST_MY(RegList_mpuAx),   &mpuAccel[0],  true);
  regsAddRegister(REGLIST_MY(RegList_mpuAy),   &mpuAccel[1],  true);
  regsAddRegister(REGLIST_MY(RegList_mpuAz),   &mpuAccel[2],  true);
}

void checkTimeouts() {
  int mode = *regsRegisters[REGLIST_MY(RegList_requestedMode)].data.pi;
  bool mpuOk = (mode == 4) || !mpuTimeoutFlg;
  schmStatusFlg = !intcTimeoutFlg && mpuOk;
  bool enable = schmStatusFlg && *regsRegisters[REGLIST_OTHER(RegList_status)].data.pi && *regsRegisters[REGLIST_MY(RegList_enableMotors)].data.pi;
  conIsEnabled = enable;
  stepEnable = enable;
}

// ########### Entry points ############

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  mpuHwInit(PIN_I2C_SDA, PIN_I2C_SCL);
  regsInit();

  Serial.begin(115200);
  Serial.println("Startup...");

  pinMode(0, INPUT);

  confInit();

  Serial.println("MPU Start...");
  if(!mpuInit())
  {
    Serial.println("MPU init failed");
  }

  if(confDevConf.mode != CONF_MODE_BT) {
    Serial.println("OTA Start...");
    if(otaNetworkInitSTA(confDevConf.wifiSsid.c_str(), confDevConf.wifiPass.c_str(), confDevConf.wifiIp, confDevConf.wifiGateway, confDevConf.wifiMask)) {
      otaInit();
    } else if(otaNetworkInitAP(confDevConf.apSsid.c_str(), confDevConf.apPass.c_str())) {
      Serial.println("OTA in AP mode");
      otaInit();
    } else {
      Serial.println("OTA cannot initialize network");
    }
  } else {
    Serial.println("PS3 Start...");
    ps3Initialize(confDevConf.btMac.c_str(), &confDevConf.btMin, &confDevConf.btMax);
    schIsInitBt = true;
  }
  
  Serial.println("Stepper Start...");
  for(int i=0;i<STEP_COUNT;i++) {
    stepControls[i].pinDir = confDevConf.motorHws.motHwArray[i].pinDir;
    stepControls[i].pinEn = confDevConf.motorHws.motHwArray[i].pinEn;
    stepControls[i].pinStep = confDevConf.motorHws.motHwArray[i].pinStep;
    stepControls[i].negate = confDevConf.motorHws.motHwArray[i].negate;
    stepControls[i].controlMode = confDevConf.motorHws.motHwArray[i].controlMode;
  }
  stepInit();

  Serial.println("Regs Start...");
  registerRegisters();
  
  for(int i=0;i<STEP_COUNT;i++) {
    stepControls[i].accelLimit = &confSysTuning.motors.motArray[i + (REGLIST_IS_NECK ? 3 : 0)].accel;
    stepControls[i].speedLimit = &confSysTuning.motors.motArray[i + (REGLIST_IS_NECK ? 3 : 0)].speed;
  }

  Serial.println("Internal com Start...");
  intcInit();

  Serial.println("Control Start..."); 
  conInit();

  Serial.println("Ready.");
}

void loop() {
  // run scheduler
  schRun();
  // perform async actions
  intcHandle();
  stepHandle();
  if(schIsInitBt) {
    ps3Handle();
  }
  checkTimeouts();
}
