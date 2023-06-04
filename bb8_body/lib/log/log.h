#include <Arduino.h>
#include <WebSerial.h>

#define LOG_S(s) WebSerial.println(F(s))
#define LOG_N WebSerial.println
#define LOG_F WebSerial.printf
