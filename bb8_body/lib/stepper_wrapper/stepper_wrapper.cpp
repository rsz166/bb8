#include <stepper_wrapper.h>
#include <Arduino.h>
#include <FastAccelStepper.h>

#define STEP_DIR_COEF(i) (stepControls[i].negate ? -1 : 1)

typedef struct {
    float position, speed, accel;
} StepMemory_t;

FastAccelStepperEngine stepEngine = FastAccelStepperEngine();
FastAccelStepper *stepSteppers[STEP_COUNT] = {NULL};
StepControl_t stepControls[STEP_COUNT] = {0};
StepMemory_t stepMemories[STEP_COUNT] = {0};

// initalize stepper motors
bool stepInit() {
    bool ret = true;
    stepEngine.init();
    for(int i=0; i<STEP_COUNT; i++) {
        stepSteppers[i] = stepEngine.stepperConnectToPin(stepControls[i].pinStep);
        if(stepSteppers[i]) {
            stepSteppers[i]->setDirectionPin(stepControls[i].pinDir);
            if(stepControls[i].pinEn > 0) {
                stepSteppers[i]->setEnablePin(stepControls[i].pinEn);
                stepSteppers[i]->setAutoEnable(true); // TODO: necessary?
            }
            stepMemories[i].accel = stepControls[i].accel;
            stepMemories[i].speed = stepControls[i].speed;
            stepSteppers[i]->setAcceleration((uint32_t)abs(stepMemories[i].accel));
            stepSteppers[i]->setSpeedInHz((uint32_t)abs(stepMemories[i].speed));
        } else {
            ret = false;
        }
    }
    return ret;
}

void stepHandle() {
    for(int i=0; i<STEP_COUNT; i++) {
        if(!stepSteppers[i]) continue;
        if(stepControls[i].positionControl) {
            if((stepControls[i].position != stepMemories[i].position) ||
               (stepControls[i].speed != stepMemories[i].speed) ||
               (stepControls[i].accel != stepMemories[i].accel)) {
                stepMemories[i].position = stepControls[i].position;
                stepMemories[i].speed = stepControls[i].speed;
                stepMemories[i].accel = stepControls[i].accel;
                stepSteppers[i]->setAcceleration((uint32_t)abs(stepMemories[i].accel));
                stepSteppers[i]->setSpeedInHz((uint32_t)abs(stepMemories[i].speed));
                stepSteppers[i]->moveTo((int32_t)(stepMemories[i].position*STEP_DIR_COEF(i)));
            }
        } else if(stepControls[i].speedControl) {
            if((stepControls[i].speed != stepMemories[i].speed) ||
               (stepControls[i].accel != stepMemories[i].accel)) {
                stepMemories[i].speed = stepControls[i].speed;
                stepMemories[i].accel = stepControls[i].accel;
                stepSteppers[i]->setAcceleration((uint32_t)abs(stepMemories[i].accel));
                if(stepMemories[i].speed == 0) {
                    stepSteppers[i]->stopMove();
                } else {
                    stepSteppers[i]->setSpeedInHz((uint32_t)abs(stepMemories[i].speed));
                    if((stepMemories[i].speed > 0) == !!stepControls[i].negate) {
                        stepSteppers[i]->runForward();
                    } else {
                        stepSteppers[i]->runBackward();
                    }
                }
            }
        } else { // acceleration control
            if((stepControls[i].speed != stepMemories[i].speed) ||
               (stepControls[i].accel != stepMemories[i].accel)) {
                stepMemories[i].speed = stepControls[i].speed;
                stepMemories[i].accel = stepControls[i].accel;
                stepSteppers[i]->setSpeedInHz((uint32_t)abs(stepMemories[i].speed));
                stepSteppers[i]->moveByAcceleration((uint32_t)(stepMemories[i].accel*STEP_DIR_COEF(i)));
            }
        }
    }
}
