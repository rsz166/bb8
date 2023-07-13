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
#define REGLIST_TYPE_INT    (1)
#define REGLIST_TYPE_FLOAT    (2)

#define REGLIST_REGS(func) \
    func(mode,REGLIST_TYPE_INT) \
    func(uptime,REGLIST_TYPE_INT) \
    func(status,REGLIST_TYPE_INT) \
    func(errorCode,REGLIST_TYPE_INT) \
    func(requestedMode,REGLIST_TYPE_INT) \
    func(enableMotors,REGLIST_TYPE_INT) \
    func(batteryVoltage,REGLIST_TYPE_FLOAT) \
    func(ctrlForw_setp,REGLIST_TYPE_FLOAT) \
    func(ctrlTilt_setp,REGLIST_TYPE_FLOAT) \
    func(ctrlRota_setp,REGLIST_TYPE_FLOAT) \
    func(ctrlForw_feedback,REGLIST_TYPE_FLOAT) \
    func(ctrlTilt_feedback,REGLIST_TYPE_FLOAT) \
    func(ctrlRota_feedback,REGLIST_TYPE_FLOAT) \
    func(ctrlForw_act,REGLIST_TYPE_FLOAT) \
    func(ctrlTilt_act,REGLIST_TYPE_FLOAT) \
    func(ctrlRota_act,REGLIST_TYPE_FLOAT) \
    func(ctrlForw_p,REGLIST_TYPE_FLOAT) \
    func(ctrlTilt_p,REGLIST_TYPE_FLOAT) \
    func(ctrlRota_p,REGLIST_TYPE_FLOAT) \
    func(ctrlForw_i,REGLIST_TYPE_FLOAT) \
    func(ctrlTilt_i,REGLIST_TYPE_FLOAT) \
    func(ctrlRota_i,REGLIST_TYPE_FLOAT) \
    func(ctrlForw_d,REGLIST_TYPE_FLOAT) \
    func(ctrlTilt_d,REGLIST_TYPE_FLOAT) \
    func(ctrlRota_d,REGLIST_TYPE_FLOAT) \
    func(ctrlForw_sat,REGLIST_TYPE_FLOAT) \
    func(ctrlTilt_sat,REGLIST_TYPE_FLOAT) \
    func(ctrlRota_sat,REGLIST_TYPE_FLOAT) \
    func(ctrlForw_isOpenLoop,REGLIST_TYPE_FLOAT) \
    func(ctrlTilt_isOpenLoop,REGLIST_TYPE_FLOAT) \
    func(ctrlRota_isOpenLoop,REGLIST_TYPE_FLOAT) \
    func(mpuAx,REGLIST_TYPE_FLOAT) \
    func(mpuAy,REGLIST_TYPE_FLOAT) \
    func(mpuAz,REGLIST_TYPE_FLOAT) \
    func(mpuGx,REGLIST_TYPE_FLOAT) \
    func(mpuGy,REGLIST_TYPE_FLOAT) \
    func(mpuGz,REGLIST_TYPE_FLOAT)

#define REGLIST_ENUMFUNC(name,type) RegList_##name,

typedef enum {
    REGLIST_REGS(REGLIST_ENUMFUNC)
    RegList_count
} RegList_en;

#endif
