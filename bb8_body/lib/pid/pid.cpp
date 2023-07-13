#include <pid.h>
#include <log.h>

void pidInit(pidCon_t *pid) {
    pid->iStorage = 0;
    pid->dStorage = 0;
}

void pidStep(pidCon_t *pid) {
    float fb = pid->params->isOpenLoop ? 0 : *pid->feedback;
    float e = *pid->ref - fb;
    pid->iStorage += e * pid->params->i;
    float u = e * pid->params->p + pid->iStorage + (e - pid->dStorage) * pid->params->d;
    pid->dStorage = e;
    float sat = pid->params->sat;
    if(u > sat) u = sat;
    else if(u < -sat) u = -sat;
    *pid->actuation = u;
}
