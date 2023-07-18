#ifndef MPU6050_WRAPPER_H_
#define MPU6050_WRAPPER_H_

#include <Arduino.h>

void mpuHwInit(int sda, int scl);
bool mpuInit();
void mpuHandle();

#endif
