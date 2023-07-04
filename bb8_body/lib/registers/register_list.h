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
    func(mode,int,1) \
    func(uptime,int,1) \
    func(status,int,1) \
    func(errorCode,int,1) \
    func(requestedMode,int,1) \
    func(batteryVoltage,float,2) \
    func(ctrlForw_setp,float,2) \
    func(ctrlTilt_setp,float,2) \
    func(ctrlRota_setp,float,2) \
    func(ctrlForw_feedback,float,2) \
    func(ctrlTilt_feedback,float,2) \
    func(ctrlRota_feedback,float,2) \
    func(ctrlForw_actMode,float,2) \
    func(ctrlTilt_actMode,float,2) \
    func(ctrlRota_actMode,float,2) \
    func(ctrlForw_act,float,2) \
    func(ctrlTilt_act,float,2) \
    func(ctrlRota_act,float,2) \
    func(ctrlForw_p,float,2) \
    func(ctrlTilt_p,float,2) \
    func(ctrlRota_p,float,2) \
    func(ctrlForw_i,float,2) \
    func(ctrlTilt_i,float,2) \
    func(ctrlRota_i,float,2) \
    func(ctrlForw_d,float,2) \
    func(ctrlTilt_d,float,2) \
    func(ctrlRota_d,float,2) \
    func(ctrlForw_sat,float,2) \
    func(ctrlTilt_sat,float,2) \
    func(ctrlRota_sat,float,2) \
    func(mpuAx,float,2) \
    func(mpuAy,float,2) \
    func(mpuAz,float,2) \
    func(mpuGx,float,2) \
    func(mpuGy,float,2) \
    func(mpuGz,float,2)

#define REGLIST_ENUMFUNC(name,type,typeNr) RegList_##name,

typedef enum {
    REGLIST_REGS(REGLIST_ENUMFUNC)
    RegList_count
} RegList_en;

#endif
