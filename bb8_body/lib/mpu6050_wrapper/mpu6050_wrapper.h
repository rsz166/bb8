#ifndef MPU6050_WRAPPER_H_
#define MPU6050_WRAPPER_H_

#include <Arduino.h>

void mpuDmpDataReady();
void mpuHwInit(int sda, int scl, int interrupt);
bool mpuInit();
void mpuLoadCalibration();
void mpuStart();
bool mpuTryRead();

extern float mpuYpr[3];

#endif
