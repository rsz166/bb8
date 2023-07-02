#include <ota_events.h>
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <configurations.h>
#include <log.h>
#include <registers.h>
#include <register_list.h>
#include <ArduinoJson.h>

AsyncEventSource events("/events");
StaticJsonDocument<1024> doc;

void otaeSendConfig() {
    doc.clear();
    doc["wifiSsid"] = confAuth.wifiSsid;
    doc["btMac"] = confAuth.btMac;
    doc["nodeId"] = confTuning.nodeId;
    doc["mode"] = confTuning.mode;
    for(int i=0;i<CONF_PID_COUNT;i++) {
        doc["pid"][i]["p"] = confTuning.pid.pidArray[i].p;
        doc["pid"][i]["i"] = confTuning.pid.pidArray[i].i;
        doc["pid"][i]["d"] = confTuning.pid.pidArray[i].d;
        doc["pid"][i]["sat"] = confTuning.pid.pidArray[i].sat;
    }
    events.send("","config",millis());
}

void odaeSendRegs() {
    events.send("","reg_list",millis());

}

void otaeInit(AsyncWebServer *server) {
    
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server->addHandler(&events);
}

void otaeHandle() {
    events.send(String(micros() & 0xf).c_str(),"test_data",millis());
    odaeSendRegs();
    otaeSendConfig();
}



