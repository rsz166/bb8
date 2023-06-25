#include <registers.h>
#include <Arduino.h>

RegsRegister_t regsRegisters[REGS_REG_CNT];
uint32_t regsDefaultBuffer[REGS_REG_CNT];

void regsAddRegister(int id, void* data, bool isTx) {
    if(id >= REGS_REG_CNT) return;
    regsRegisters[id].isTx = isTx;
    if(data != nullptr) regsRegisters[id].data.pv = data;
}

bool regsInit() {
    for(int i=0; i<REGS_REG_CNT; i++) {
        regsDefaultBuffer[i] = 0;
        regsRegisters[i].data.pi = &regsDefaultBuffer[i];
        regsRegisters[i].isTx = false;
    }
    return true;
}
