#include <stepper_wrapper.h>
#include <Arduino.h>
#include <FastAccelStepper.h>
#include <log.h>

#define STEP_DIR_COEF(i) (stepControls[i].negate ? -1 : 1)

typedef struct {
    float setpoint, speedLimit, accelLimit;
} StepMemory_t;

FastAccelStepperEngine stepEngine = FastAccelStepperEngine();
FastAccelStepper *stepSteppers[STEP_COUNT] = {NULL};
StepControl_t stepControls[STEP_COUNT] = {0};
StepMemory_t stepMemories[STEP_COUNT] = {0};
volatile bool stepEnable = false;

// initalize stepper motors
bool stepInit() {
    bool ret = true;
    stepEngine.init();
    for(int i=0; i<STEP_COUNT; i++) {
        if((stepControls[i].controlMode != STEP_CONTROL_DISABLED) && (stepControls[i].pinStep != 0)) {
            stepSteppers[i] = stepEngine.stepperConnectToPin(stepControls[i].pinStep);
            if(stepSteppers[i]) {
                if(stepControls[i].pinDir > 0) {
                    stepSteppers[i]->setDirectionPin(stepControls[i].pinDir);
                }
                if(stepControls[i].pinEn > 0) {
                    stepSteppers[i]->setEnablePin(stepControls[i].pinEn);
                    stepSteppers[i]->setAutoEnable(true); // TODO: necessary?
                }
                stepMemories[i].accelLimit = 0;
                stepMemories[i].speedLimit = 0;
            } else {
                ret = false;
            }
        }
    }
    return ret;
}

void stepHandle() {
    for(int i=0; i<STEP_COUNT; i++) {
        if(!stepSteppers[i]) continue;
        if(stepEnable) {
            if(stepControls[i].controlMode == STEP_CONTROL_POSITION) {
                if((stepControls[i].setpoint != stepMemories[i].setpoint) ||
                (*stepControls[i].speedLimit != stepMemories[i].speedLimit) ||
                (*stepControls[i].accelLimit != stepMemories[i].accelLimit)) {
                    stepMemories[i].setpoint = stepControls[i].setpoint;
                    stepMemories[i].speedLimit = *stepControls[i].speedLimit;
                    stepMemories[i].accelLimit = *stepControls[i].accelLimit;
                    stepSteppers[i]->setAcceleration((uint32_t)abs(stepMemories[i].accelLimit));
                    stepSteppers[i]->setSpeedInHz((uint32_t)abs(stepMemories[i].speedLimit));
                    stepSteppers[i]->moveTo((int32_t)(stepMemories[i].setpoint*STEP_DIR_COEF(i)));
                }
            } else if(stepControls[i].controlMode == STEP_CONTROL_SPEED) {
                if((stepControls[i].setpoint != stepMemories[i].setpoint) ||
                (*stepControls[i].speedLimit != stepMemories[i].speedLimit) ||
                (*stepControls[i].accelLimit != stepMemories[i].accelLimit)) {
                    stepMemories[i].setpoint = stepControls[i].setpoint;
                    stepMemories[i].speedLimit = *stepControls[i].speedLimit;
                    stepMemories[i].accelLimit = *stepControls[i].accelLimit;
                    stepSteppers[i]->setAcceleration((uint32_t)abs(stepMemories[i].accelLimit));
                    if(stepMemories[i].setpoint > stepMemories[i].speedLimit) stepMemories[i].setpoint = stepMemories[i].speedLimit;
                    else if(stepMemories[i].setpoint < -stepMemories[i].speedLimit) stepMemories[i].setpoint = -stepMemories[i].speedLimit;
                    if(stepMemories[i].setpoint == 0) {
                        stepSteppers[i]->stopMove();
                    } else {
                        stepSteppers[i]->setSpeedInHz((uint32_t)abs(stepMemories[i].setpoint));
                        if((stepMemories[i].setpoint > 0) == !stepControls[i].negate) {
                            stepSteppers[i]->runForward();
                        } else {
                            stepSteppers[i]->runBackward();
                        }
                    }
                }
            } else if(stepControls[i].controlMode == STEP_CONTROL_ACCEL) { // acceleration control
                if((*stepControls[i].speedLimit != stepMemories[i].speedLimit) ||
                (*stepControls[i].accelLimit != stepMemories[i].accelLimit)) {
                    stepMemories[i].speedLimit = *stepControls[i].speedLimit;
                    stepMemories[i].accelLimit = *stepControls[i].accelLimit;
                    stepSteppers[i]->setSpeedInHz((uint32_t)abs(stepMemories[i].speedLimit));
                    stepSteppers[i]->moveByAcceleration((uint32_t)(stepMemories[i].setpoint*STEP_DIR_COEF(i)));
                }
            }
        } else {
            stepMemories[i].setpoint = 0;
            stepMemories[i].speedLimit = 0;
            stepMemories[i].accelLimit = 0;
            stepSteppers[i]->stopMove();
        }
    }
}
