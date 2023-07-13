#ifndef CONTROL_H_
#define CONTROL_H_

#include <Arduino.h>

extern uint32_t conIsEnabled;

void conHandle();
bool conInit();

#endif
