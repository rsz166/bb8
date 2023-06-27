#ifndef REGISTER_LIST_H_
#define REGISTER_LIST_H_

#include <configurations.h>

#define REGLIST_BODY_OFFSET (0)
#define REGLIST_NECK_OFFSET (RegList_count)

#define REGLIST_IS_NECK (confDevConf.nodeId == CONF_NODEID_NECK)
#define REGLIST_IS_BODY (confDevConf.nodeId == CONF_NODEID_BODY)
#define REGLIST_IS_VALID (REGLIST_IS_NECK || REGLIST_IS_BODY)
#define REGLIST_MY_OFFSET (REGLIST_IS_NECK ? (REGLIST_NECK_OFFSET) : (REGLIST_BODY_OFFSET))
#define REGLIST_OTHER_OFFSET (REGLIST_IS_NECK ? (REGLIST_BODY_OFFSET) : (REGLIST_NECK_OFFSET))
#define REGLIST_MY(x) (REGLIST_IS_VALID ? (REGLIST_MY_OFFSET + (x)) : 0xff)
#define REGLIST_OTHER(x) (REGLIST_IS_VALID ? (REGLIST_OTHER_OFFSET + (x)) : 0xff)
#define REGLIST_BODY(x) (REGLIST_BODY_OFFSET + (x))
#define REGLIST_NECK(x) (REGLIST_NECK_OFFSET + (x))

// variant selector macros
#define REGLIST_HAVE_OTA (REGLIST_IS_NECK) // mainly handles OTA parametrization
#define REGLIST_HAVE_REMOTE (REGLIST_IS_BODY) // mainly handles remote controller

typedef enum {
    RegList_mode = 0,
    RegList_uptime,
    RegList_status,
    RegList_errorCode,
    RegList_batteryVoltage,
    RegList_requestedMode,
    RegList_ctrlForw_setp,
    RegList_ctrlTilt_setp,
    RegList_ctrlRota_setp,
    RegList_ctrlForw_feedback,
    RegList_ctrlTilt_feedback,
    RegList_ctrlRota_feedback,
    RegList_ctrlForw_actMode,
    RegList_ctrlTilt_actMode,
    RegList_ctrlRota_actMode,
    RegList_ctrlForw_act,
    RegList_ctrlTilt_act,
    RegList_ctrlRota_act,
    RegList_ctrlForw_p,
    RegList_ctrlTilt_p,
    RegList_ctrlRota_p,
    RegList_ctrlForw_i,
    RegList_ctrlTilt_i,
    RegList_ctrlRota_i,
    RegList_ctrlForw_d,
    RegList_ctrlTilt_d,
    RegList_ctrlRota_d,
    RegList_ctrlForw_sat,
    RegList_ctrlTilt_sat,
    RegList_ctrlRota_sat,
    RegList_count
} RegList_en;

#endif
