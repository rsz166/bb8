#include <Arduino.h>

void mpuDmpDataReady();
void mpuHwInit(int sda, int scl, int interrupt);
bool mpuInit();
void mpuLoadCalibration();
void mpuStart();
bool mpuTryRead();