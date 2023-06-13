#include <registers.h>
#include <Arduino.h>

RegsRegister_t regsRegisters[REGS_REG_CNT];

void regsAddRegister(int id, void* data) {
    if(id >= REGS_REG_CNT) return;
    regsRegisters[id].pv = data;
}

bool regsInit() {
    for(int i=0; i<REGS_REG_CNT; i++) {
        regsRegisters[i].pv = nullptr;
    }
    return true;
}
