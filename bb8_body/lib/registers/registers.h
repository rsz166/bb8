#ifndef REGISTERS_H_
#define REGISTERS_H_

#include <Arduino.h>

#define REGS_REG_CNT    (60)

typedef struct {
    union {
        void* pv;
        uint32_t* pi;
    } data;
    bool isRx;
} RegsRegister_t;

extern RegsRegister_t regsRegisters[REGS_REG_CNT];

void regsAddRegister(int id, void* data, bool isRx);
bool regsInit();

#endif
