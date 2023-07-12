#ifndef PS3_WRAPPER_H_
#define PS3_WRAPPER_H_

#include <Arduino.h>

void ps3Notify();
void ps3OnConnect();
void ps3Initialize(const char* mac);
extern float ps3Ypr_body[3], ps3Ypr_neck[3], ps3Battery;

#endif
