#ifndef LOG_H_
#define LOG_H_

// #include <WebSerial.h>
// #define LOG_S(s) WebSerial.println(F(s))
// #define LOG_N WebSerial.println
// #define LOG_F WebSerial.printf

#include <Arduino.h>
#define LOG_S(s) Serial.println(F(s))
#define LOG_N Serial.println
#define LOG_F Serial.printf

#endif
