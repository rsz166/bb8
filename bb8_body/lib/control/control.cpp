#include <control.h>
#include <pid.h>
#include <registers.h>
#include <register_list.h>

pidCon_t forw, tilt, rota;
bool isInitialized = false;
bool isEnabled = false;

void conHandle() {
    if(isEnabled) {
        pidStep(&forw);
        pidStep(&tilt);
        pidStep(&rota);
        isInitialized = false;
    } else if(!isInitialized) {
        pidInit(&forw);
        pidInit(&tilt);
        pidInit(&rota);
        isInitialized = true;
    }
}

bool conInit() {
    forw.ref = regsRegisters[REGLIST_MY(RegList_ctrlForw_setp)].data.pf;
    forw.feedback = regsRegisters[REGLIST_MY(RegList_ctrlForw_feedback)].data.pf;
    forw.actuation = regsRegisters[REGLIST_MY(RegList_ctrlForw_act)].data.pf;
    forw.params = REGLIST_IS_NECK ? &confSysTuning.pids.pidNamed.neckForward : &confSysTuning.pids.pidNamed.bodyForward;
    
    tilt.ref = regsRegisters[REGLIST_MY(RegList_ctrlTilt_setp)].data.pf;
    tilt.feedback = regsRegisters[REGLIST_MY(RegList_ctrlTilt_feedback)].data.pf;
    tilt.actuation = regsRegisters[REGLIST_MY(RegList_ctrlTilt_act)].data.pf;
    tilt.params = REGLIST_IS_NECK ? &confSysTuning.pids.pidNamed.neckTilt : &confSysTuning.pids.pidNamed.bodyTilt;
    
    rota.ref = regsRegisters[REGLIST_MY(RegList_ctrlRota_setp)].data.pf;
    rota.feedback = regsRegisters[REGLIST_MY(RegList_ctrlRota_feedback)].data.pf;
    rota.actuation = regsRegisters[REGLIST_MY(RegList_ctrlRota_act)].data.pf;
    rota.params = REGLIST_IS_NECK ? &confSysTuning.pids.pidNamed.neckRotate : &confSysTuning.pids.pidNamed.bodyRotate;

    pidInit(&forw);
    pidInit(&tilt);
    pidInit(&rota);
    isInitialized = true;

    return true;
}
