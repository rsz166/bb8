#ifndef OTA_WRAPPER_H_
#define OTA_WRAPPER_H_

#include <Arduino.h>

bool otaNetworkInitAP(const char* ssid);
bool otaNetworkInitSTA(const char* ssid, const char* pass);
void otaInit();

#endif
