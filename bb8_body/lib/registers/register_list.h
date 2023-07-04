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

#define REGLIST_REGS(func) \
    func(mode,int) \
    func(uptime,int) \
    func(status,int) \
    func(errorCode,int) \
    func(batteryVoltage,float) \
    func(requestedMode,int) \
    func(ctrlForw_setp,float) \
    func(ctrlTilt_setp,float) \
    func(ctrlRota_setp,float) \
    func(ctrlForw_feedback,float) \
    func(ctrlTilt_feedback,float) \
    func(ctrlRota_feedback,float) \
    func(ctrlForw_actMode,float) \
    func(ctrlTilt_actMode,float) \
    func(ctrlRota_actMode,float) \
    func(ctrlForw_act,float) \
    func(ctrlTilt_act,float) \
    func(ctrlRota_act,float) \
    func(ctrlForw_p,float) \
    func(ctrlTilt_p,float) \
    func(ctrlRota_p,float) \
    func(ctrlForw_i,float) \
    func(ctrlTilt_i,float) \
    func(ctrlRota_i,float) \
    func(ctrlForw_d,float) \
    func(ctrlTilt_d,float) \
    func(ctrlRota_d,float) \
    func(ctrlForw_sat,float) \
    func(ctrlTilt_sat,float) \
    func(ctrlRota_sat,float) \
    func(mpuAx,float) \
    func(mpuAy,float) \
    func(mpuAz,float) \
    func(mpuGx,float) \
    func(mpuGy,float) \
    func(mpuGz,float)

#define REGLIST_ENUMFUNC(name,type) RegList_##name,

typedef enum {
    REGLIST_REGS(REGLIST_ENUMFUNC)
    RegList_count
} RegList_en;

#endif
