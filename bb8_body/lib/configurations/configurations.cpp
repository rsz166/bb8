#include <EEPROM.h>
#include <configurations.h>

#define CONF_MAX_SIZE (512) // max 512

#define CONF_MAX_WIFI_LEN (30)
#define CONF_ADDR_WIFI_SSID (0)
#define CONF_ADDR_WIFI_PASS (CONF_ADDR_WIFI_SSID + CONF_MAX_WIFI_LEN)
#define CONF_ADDR_PID_START (CONF_ADDR_WIFI_PASS + CONF_MAX_WIFI_LEN)
#define CONF_ADDR_PID_COUNT (6)
#define CONF_ADDR_PID_END (CONF_ADDR_PID_START + CONF_ADDR_PID_COUNT * 12 - 1)

#if CONF_ADDR_PID_END >= CONF_MAX_SIZE
#error EEPROM definitions exceeds max size
#endif

void confInit() {
  EEPROM.begin(CONF_MAX_SIZE);
}

void confWrite(int address, const byte* data, int size) {
  EEPROM.writeBytes(address, data, size);
}

void confRead(int address, byte* data, int size) {
  EEPROM.readBytes(address, data, size);
}

void confGetWifiSsid(char* s, int len) {
  if(len > CONF_MAX_WIFI_LEN) len = CONF_MAX_WIFI_LEN;
  confRead(CONF_ADDR_WIFI_SSID, (byte*)s, len);
}
void confSetWifiSsid(const char* s) {
  int len = strlen(s);
  if(len > CONF_MAX_WIFI_LEN) len = CONF_MAX_WIFI_LEN;
  confWrite(CONF_ADDR_WIFI_SSID, (byte*)s, len);
}

void confGetWifiPass(char* s, int len) {
  if(len > CONF_MAX_WIFI_LEN) len = CONF_MAX_WIFI_LEN;
  confRead(CONF_ADDR_WIFI_PASS, (byte*)s, len);
}
void confSetWifiPass(const char* s) {
  int len = strlen(s);
  if(len > CONF_MAX_WIFI_LEN) len = CONF_MAX_WIFI_LEN;
  confWrite(CONF_ADDR_WIFI_PASS, (byte*)s, len);
}

void confGetPID(int idx, ConfPIDParam_t* st) {
  confRead(CONF_ADDR_PID_START + idx * sizeof(ConfPIDParam_t), (byte*)st, sizeof(ConfPIDParam_t));
}
void confSetPID(int idx, ConfPIDParam_t* st) {
  confWrite(CONF_ADDR_PID_START + idx * sizeof(ConfPIDParam_t), (byte*)st, sizeof(ConfPIDParam_t));
}
