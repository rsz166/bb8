#ifndef PID_H_
#define PID_H_

#include <configurations.h>

typedef struct {
    float *ref;
    float *feedback;
    float *actuation;
    float iStorage;
    float dStorage;
    ConfPIDParam_t *params;
} pidCon_t;

void pidInit(pidCon_t *pid);
void pidStep(pidCon_t *pid);
float pidStepExternal(pidCon_t *pid, float sp, float fb);
void pidSimpleInit(float *accI, float *accD);
float pidSimpleStep(float e, float sat, float kp, float ki, float kd, float *accI, float *accD);

#endif
