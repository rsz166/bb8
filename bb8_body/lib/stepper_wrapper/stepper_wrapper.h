#ifndef STEPPER_WRAPPER_H_
#define STEPPER_WRAPPER_H_

#include <Arduino.h>

#define STEP_COUNT 3
#define STEP_CONTROL_DISABLED 0
#define STEP_CONTROL_POSITION 1
#define STEP_CONTROL_SPEED 2
#define STEP_CONTROL_ACCEL 3

typedef struct {
    uint8_t pinStep, pinDir, pinEn, controlMode;
    bool negate;
    float setpoint, *speedLimit, *accelLimit;
} StepControl_t;

extern StepControl_t stepControls[STEP_COUNT];
extern volatile bool stepEnable;

bool stepInit();
void stepHandle();

#endif
