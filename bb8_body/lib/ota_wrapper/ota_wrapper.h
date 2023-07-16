#ifndef OTA_WRAPPER_H_
#define OTA_WRAPPER_H_

#include <Arduino.h>

bool otaNetworkInitAP(const char* ssid, const char* pass);
bool otaNetworkInitSTA(const char* ssid, const char* pass, byte ip[4], byte gateway[4], byte mask[4]);
void otaInit();
void otaHandle();

#endif
