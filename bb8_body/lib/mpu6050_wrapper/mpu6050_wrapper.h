#ifndef MPU6050_WRAPPER_H_
#define MPU6050_WRAPPER_H_

#include <Arduino.h>

extern float mpuAccel[3];   // [x,y,z]
extern float mpuTilt[2];    // [forward-backward-vertical plane from vertical (+ is tilt forward), right-left-vertical plane from vertical (+ is tilt right)]

void mpuHwInit(int sda, int scl);
bool mpuInit();
void mpuHandle();

#endif
