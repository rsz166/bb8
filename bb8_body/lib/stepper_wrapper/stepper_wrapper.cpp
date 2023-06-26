#include <stepper_wrapper.h>
#include <Arduino.h>
// #include <ESP_FlexyStepper.h>
#include <log.h>
#include <FastAccelStepper.h>

// IO pin assignments for steppers
const int MOTOR1_STEP_PIN = 23;
const int MOTOR1_DIRECTION_PIN = 22;

float stepperMove = 0, stepperSpeed = 0, stepperLastSpeed = stepperSpeed;
bool stepperStop = false;

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;

// initalize stepper motors
void stepperInit() {
    // connect and configure the stepper motor to its IO pins
    engine.init();
    stepper = engine.stepperConnectToPin(MOTOR1_STEP_PIN);
    if (stepper) {
        stepper->setDirectionPin(MOTOR1_DIRECTION_PIN);
        stepper->setAcceleration(5000);
    }
}

void stepperDrive() {
    // set the position, speed and acceleration
    if(stepperSpeed != stepperLastSpeed) {
        stepperLastSpeed = stepperSpeed;
        if(stepperSpeed == 0) {
            stepper->stopMove();
        } else {
            stepper->setSpeedInHz(abs(stepperSpeed));
            if(stepperSpeed > 0) {
                stepper->runForward();
            } else {
                stepper->runBackward();
            }
        }
    }
}
