#ifndef CONFIGURATIONS_H_
#define CONFIGURATIONS_H_

#include <Arduino.h>

#define CONF_DEV_MOT_COUNT 3
#define CONF_SYS_MOT_COUNT 6
#define CONF_SYS_PID_COUNT 6
#define CONF_MODE_INVALID 0
#define CONF_MODE_BT 1
#define CONF_MODE_WIFI 2

#define CONF_NODEID_INVALID 0
#define CONF_NODEID_BODY 1
#define CONF_NODEID_NECK 2

typedef struct {
  float p, i, d, sat, fbgain;
  int isOpenLoop;
} ConfPIDParam_t;

typedef enum {
  ConfMotMode_disabled = 0,
  ConfMotMode_controlPosition,
  ConfMotMode_controlSpeed,
  ConfMotMode_controlAcceleration,
} ConfMotMode_t;

typedef struct {
  uint8_t pinStep,pinDir,pinEn,controlMode;
  bool negate;
} ConfMotHw_t;

typedef struct {
  float speed, accel; // default speed if in position mode, max speed if in speed mode
} ConfMotTuning_t;

// system level parameters, shared
typedef struct {
  union {
    ConfPIDParam_t pidArray[CONF_SYS_PID_COUNT];
    struct {
    ConfPIDParam_t bodyForward;
    ConfPIDParam_t bodyTilt;
    ConfPIDParam_t bodyRotate;
    ConfPIDParam_t neckForward;
    ConfPIDParam_t neckTilt;
    ConfPIDParam_t neckRotate;
    } pidNamed;
  } pids;
  union {
    ConfMotTuning_t motArray[CONF_SYS_MOT_COUNT];
    struct {
      ConfMotTuning_t bodyForward;
      ConfMotTuning_t bodyTilt;
      ConfMotTuning_t bodyRotate;
      ConfMotTuning_t neckForward;
      ConfMotTuning_t neckTilt;
      ConfMotTuning_t neckRotate;
    } motNamed;
  } motors;
} ConfSysTuning_t;

// device level parameters, unique per device
typedef struct {
  String apSsid;
  String apPass;
  String wifiSsid;
  String wifiPass;
  byte wifiIp[4];
  byte wifiGateway[4];
  byte wifiMask[4];
  String btMac;
  int nodeId;
  int mode;
  union {
    ConfMotHw_t motHwArray[CONF_DEV_MOT_COUNT];
    struct {
      ConfMotHw_t forward;
      ConfMotHw_t tilt;
      ConfMotHw_t rotate;
    } motHwNamed;
  } motorHws;
  int btMin;
  int btMax;
} ConfDeviceConfig_t;

extern ConfSysTuning_t confSysTuning;
extern ConfDeviceConfig_t confDevConf;

void confInit();
bool confWrite();

#endif