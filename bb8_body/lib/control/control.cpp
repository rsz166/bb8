#include <control.h>
#include <pid.h>
#include <registers.h>
#include <register_list.h>

pidCon_t conForw, conTilt, conRota;
bool conIsInitialized = false;
uint32_t conIsEnabled = 0;

void conHandle() {
    if(conIsEnabled) {
        pidStep(&conForw);
        pidStep(&conTilt);
        pidStep(&conRota);
        conIsInitialized = false;
    } else if(!conIsInitialized) {
        pidInit(&conForw);
        pidInit(&conTilt);
        pidInit(&conRota);
        conIsInitialized = true;
    }
}

bool conInit() {
    conForw.ref = regsRegisters[REGLIST_MY(RegList_ctrlForw_setp)].data.pf; // TODO: move this to main
    conForw.feedback = regsRegisters[REGLIST_MY(RegList_ctrlForw_feedback)].data.pf;
    conForw.actuation = regsRegisters[REGLIST_MY(RegList_ctrlForw_act)].data.pf;
    conForw.params = REGLIST_IS_NECK ? &confSysTuning.pids.pidNamed.neckForward : &confSysTuning.pids.pidNamed.bodyForward;
    
    conTilt.ref = regsRegisters[REGLIST_MY(RegList_ctrlTilt_setp)].data.pf;
    conTilt.feedback = regsRegisters[REGLIST_MY(RegList_ctrlTilt_feedback)].data.pf;
    conTilt.actuation = regsRegisters[REGLIST_MY(RegList_ctrlTilt_act)].data.pf;
    conTilt.params = REGLIST_IS_NECK ? &confSysTuning.pids.pidNamed.neckTilt : &confSysTuning.pids.pidNamed.bodyTilt;
    
    conRota.ref = regsRegisters[REGLIST_MY(RegList_ctrlRota_setp)].data.pf;
    conRota.feedback = regsRegisters[REGLIST_MY(RegList_ctrlRota_feedback)].data.pf;
    conRota.actuation = regsRegisters[REGLIST_MY(RegList_ctrlRota_act)].data.pf;
    conRota.params = REGLIST_IS_NECK ? &confSysTuning.pids.pidNamed.neckRotate : &confSysTuning.pids.pidNamed.bodyRotate;

    pidInit(&conForw);
    pidInit(&conTilt);
    pidInit(&conRota);
    conIsInitialized = true;

    return true;
}
