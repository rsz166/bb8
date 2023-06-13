#ifndef REGISTERS_H_
#define REGISTERS_H_

typedef union {
    void* pv;
    int* pi;
} RegsRegister_t;

void regsAddRegister(int id, void* data);
bool regsInit();

#endif
