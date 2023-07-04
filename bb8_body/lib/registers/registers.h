#ifndef REGISTERS_H_
#define REGISTERS_H_

#include <Arduino.h>

#define REGS_REG_CNT    (80)

typedef struct {
    union {
        void* pv;
        uint32_t* pi;
        float* pf;
    } data;
    bool isTx;
} RegsRegister_t;

extern RegsRegister_t regsRegisters[REGS_REG_CNT];

void regsAddRegister(int id, void* data, bool isTx);
bool regsInit();

#endif
