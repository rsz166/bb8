#include <registers.h>
#include <Arduino.h>

RegsRegister_t regsRegisters[REGS_REG_CNT];
uint32_t regsDefaultBuffer[REGS_REG_CNT];

void regsAddRegister(int id, void* data, bool isRx) {
    if(id >= REGS_REG_CNT) return;
    regsRegisters[id].data.pv = data;
    regsRegisters[id].isRx = isRx;
}

bool regsInit() {
    for(int i=0; i<REGS_REG_CNT; i++) {
        regsRegisters[i].data.pi = &regsDefaultBuffer[i];
        regsRegisters[i].isRx = true;
    }
    return true;
}
