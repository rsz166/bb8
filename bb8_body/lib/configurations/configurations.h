#ifndef CONFIGURATIONS_H_
#define CONFIGURATIONS_H_

#include <Arduino.h>

#define CONF_PID_COUNT 6
#define CONF_MODE_INVALID 0
#define CONF_MODE_BT 1
#define CONF_MODE_WIFI 2

#define CONF_NODEID_INVALID 0
#define CONF_NODEID_BODY 1
#define CONF_NODEID_NECK 2

typedef struct {
  float p, i, d, sat;
} ConfPIDParam_t;

typedef struct {
  int nodeId;
  union {
    ConfPIDParam_t pidArray[CONF_PID_COUNT];
    struct {
    ConfPIDParam_t bodyForward;
    ConfPIDParam_t bodyTilt;
    ConfPIDParam_t bodyRotate;
    ConfPIDParam_t neckForward;
    ConfPIDParam_t neckTilt;
    ConfPIDParam_t neckRotate;
    } pidNamed;
  } pid;
  int mode;
} ConfTuning_t;

typedef struct {
  String wifiSsid;
  String wifiPass;
  String btMac;
} ConfAuth_t;

extern ConfTuning_t confTuning;
extern ConfAuth_t confAuth;

void confInit();
bool confWrite();
String confGetTuningFile();

#endif