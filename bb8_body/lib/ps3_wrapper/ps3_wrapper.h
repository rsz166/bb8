#ifndef PS3_WRAPPER_H_
#define PS3_WRAPPER_H_

#include <Arduino.h>

void ps3Initialize(const char* mac);
void ps3Handle();
extern float ps3Ftr_body[3], ps3Ftr_neck[3], ps3Battery;
extern uint32_t ps3MotorEnable;

#endif
