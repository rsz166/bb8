#ifndef STEPPER_WRAPPER_H_
#define STEPPER_WRAPPER_H_

#include <Arduino.h>

#define STEP_COUNT 3

typedef struct {
    uint8_t pinStep, pinDir, pinEn;
    bool negate;
    float position, speed, accel;
    bool positionControl, speedControl;
} StepControl_t;

bool stepInit();
void stepHandle();

#endif
