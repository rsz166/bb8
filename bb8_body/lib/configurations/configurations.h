#include <Arduino.h>

typedef struct {
  float p, i, d;
} ConfPIDParam_t;

void confInit();
void confGetWifiSsid(char* s, int maxLen);
void confSetWifiSsid(const char* s);
void confGetWifiPass(char* s, int maxLen);
void confSetWifiPass(const char* s);
void confGetPID(int idx, ConfPIDParam_t* st);
void confSetPID(int idx, ConfPIDParam_t* st);
