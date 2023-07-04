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
char buffer[256];

void otaeSendConfig() {
    doc.clear();
    doc["wifiSsid"] = confDevConf.wifiSsid;
    doc["btMac"] = confDevConf.btMac;
    doc["nodeId"] = confDevConf.nodeId;
    doc["mode"] = confDevConf.mode;
    for(int i=0;i<CONF_DEV_MOT_COUNT;i++) {
        doc["motorHws"][i]["pinStep"] = confDevConf.motorHws.motHwArray[i].pinStep;
        doc["motHw"][i]["pinDir"] = confDevConf.motorHws.motHwArray[i].pinDir;
        doc["motHw"][i]["pinEn"] = confDevConf.motorHws.motHwArray[i].pinEn;
        doc["motHw"][i]["controlMode"] = confDevConf.motorHws.motHwArray[i].controlMode;
        doc["motHw"][i]["negate"] = confDevConf.motorHws.motHwArray[i].negate;
    }
    serializeJson(doc, buffer, sizeof(buffer));
    events.send(buffer,"config",millis());
}

// TODO: handle type
#define OTAE_REGJSON_BODY(name,type) doc[#name] = *regsRegisters[REGLIST_BODY(RegList_##name)].data.pf;
#define OTAE_REGJSON_NECK(name,type) doc[#name] = *regsRegisters[REGLIST_NECK(RegList_##name)].data.pf;

void odaeSendRegs() {
    doc.clear();
    REGLIST_REGS(OTAE_REGJSON_BODY)
    REGLIST_REGS(OTAE_REGJSON_NECK)
    serializeJson(doc, buffer, sizeof(buffer));
    events.send(buffer,"reg_list",millis());
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



