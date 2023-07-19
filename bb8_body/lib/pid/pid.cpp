#include <pid.h>
#include <log.h>

void pidInit(pidCon_t *pid) {
    pidSimpleInit(&pid->iStorage, &pid->dStorage);
}

void pidStep(pidCon_t *pid) {
    float fb = pid->params->isOpenLoop ? 0 : *pid->feedback;
    float e = *pid->ref - fb;
    *pid->actuation = pidSimpleStep(e,pid->params->sat,pid->params->p,pid->params->i,pid->params->d,&pid->iStorage,&pid->dStorage);
}

float pidStepExternal(pidCon_t *pid, float sp, float fb) {
    float e = sp - (pid->params->isOpenLoop ? 0 : fb);
    return pidSimpleStep(e,pid->params->sat,pid->params->p,pid->params->i,pid->params->d,&pid->iStorage,&pid->dStorage);
}

float pidLimit(float x, float limit) {
    if(x > limit) return limit;
    if(x < -limit) return -limit;
    return x;
}

void pidSimpleInit(float *accI, float *accD) {
    *accI = 0;
    *accD = 0;
}

float pidSimpleStep(float e, float sat, float kp, float ki, float kd, float *accI, float *accD) {
    float i = pidLimit(*accI + e * ki, sat);
    *accI = i;
    float u = kp * e + i + (e - *accD) * kd;
    *accD = e;
    return pidLimit(u, sat);
}
