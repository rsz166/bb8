#include <stepper_wrapper.h>
#include <Arduino.h>
#include <ESP_FlexyStepper.h>

// IO pin assignments for steppers
const int MOTOR1_STEP_PIN = 26;
const int MOTOR1_DIRECTION_PIN = 16;

// // Speed settings
// const int DISTANCE_TO_TRAVEL_IN_STEPS = 2000;
// const int SPEED_IN_STEPS_PER_SECOND = 300;
// const int ACCELERATION_IN_STEPS_PER_SECOND = 800;
// const int DECELERATION_IN_STEPS_PER_SECOND = 800;

// create the stepper motor objects
ESP_FlexyStepper stepper;
float stepperMove = 0, stepperSpeed = 10, stepperLastSpeed = stepperSpeed;

// initalize stepper motors
void stepperInit() {
    // connect and configure the stepper motor to its IO pins
    stepper.connectToPins(MOTOR1_STEP_PIN, MOTOR1_DIRECTION_PIN);

    // // set the speed and acceleration rates for the stepper motor
    // stepper.setSpeedInStepsPerSecond(SPEED_IN_STEPS_PER_SECOND);
    // stepper.setAccelerationInStepsPerSecondPerSecond(ACCELERATION_IN_STEPS_PER_SECOND);
    // stepper.setDecelerationInStepsPerSecondPerSecond(DECELERATION_IN_STEPS_PER_SECOND);

    // //register the callback function, to get informed whenever the target postion has been reached
    // stepper.registerTargetPositionReachedCallback(targetPositionReachedCallback);

    // // Note: start the stepper instance as a service in the "background" as a
    // // separate task and the OS of the ESP will take care of invoking the 
    // // processMovement() task regularily so you can do whatever you want in the loop function
    // stepper.startAsService();
}

void stepperDrive() {
    // set the position, speed and acceleration
    // float stepsToMove = 10, speedToReach = 100, fastToBump = 100;
    // stepper.setSpeedInStepsPerSecond(speedToReach);
    // stepper.setAccelerationInStepsPerSecondPerSecond(fastToBump);
    // stepper.moveRelativeInSteps(stepsToMove);
    if(stepperSpeed != stepperLastSpeed) {
        stepperLastSpeed = stepperSpeed;
        stepper.setSpeedInStepsPerSecond(stepperSpeed);
    }
    if(stepperMove != 0) {
        float tmpMove = stepperMove;
        stepperMove = 0;
        stepper.moveRelativeInSteps(tmpMove);
    }
}
