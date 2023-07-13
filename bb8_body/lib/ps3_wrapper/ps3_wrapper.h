#ifndef PS3_WRAPPER_H_
#define PS3_WRAPPER_H_

#include <Arduino.h>

void ps3Initialize(const char* mac);
void ps3Handle();
extern float ps3Ypr_body[3], ps3Ypr_neck[3], ps3Battery;
extern bool ps3MotorEnable;

#endif
