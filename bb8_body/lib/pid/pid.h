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

#endif
