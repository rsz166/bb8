#ifndef STEPPER_WRAPPER_H_
#define STEPPER_WRAPPER_H_

#include <Arduino.h>

extern float stepperMove, stepperSpeed;
extern bool stepperStop;

void stepperInit();
void stepperDrive();

#endif
