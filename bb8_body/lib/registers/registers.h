#ifndef REGISTERS_H_
#define REGISTERS_H_

#include <Arduino.h>

#define REGS_REG_CNT    (20)

typedef union {
    void* pv;
    uint32_t* pi;
} RegsRegister_t;

extern RegsRegister_t regsRegisters[REGS_REG_CNT];

void regsAddRegister(int id, void* data);
bool regsInit();

#endif
