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
StaticJsonDocument<2048> doc;
char buffer[2048]; // TODO: check regs

void otaeSendConfig() {
    doc.clear();
    doc["wifiSsid"] = confDevConf.wifiSsid;
    doc["btMac"] = confDevConf.btMac;
    doc["nodeId"] = confDevConf.nodeId;
    doc["mode"] = confDevConf.mode;
    for(int i=0;i<CONF_DEV_MOT_COUNT;i++) {
        doc["motorHws"][i]["pinStep"] = confDevConf.motorHws.motHwArray[i].pinStep;
        doc["motorHws"][i]["pinDir"] = confDevConf.motorHws.motHwArray[i].pinDir;
        doc["motorHws"][i]["pinEn"] = confDevConf.motorHws.motHwArray[i].pinEn;
        doc["motorHws"][i]["controlMode"] = confDevConf.motorHws.motHwArray[i].controlMode;
        doc["motorHws"][i]["negate"] = confDevConf.motorHws.motHwArray[i].negate;
    }
    serializeJson(doc, buffer, sizeof(buffer));
    events.send(buffer,"config",millis());
}

void otaeSendTuning() {
    doc.clear();
    for(int i=0; i<CONF_SYS_PID_COUNT; i++) {
      doc["pids"][i]["p"] = confSysTuning.pids.pidArray[i].p;
      doc["pids"][i]["i"] = confSysTuning.pids.pidArray[i].i;
      doc["pids"][i]["d"] = confSysTuning.pids.pidArray[i].d;
      doc["pids"][i]["sat"] = confSysTuning.pids.pidArray[i].sat;
    }
    for(int i=0; i<CONF_SYS_MOT_COUNT; i++) {
      doc["motors"][i]["speed"] = confSysTuning.motors.motArray[i].speed;
      doc["motors"][i]["accel"] = confSysTuning.motors.motArray[i].accel;
    }
    serializeJson(doc, buffer, sizeof(buffer));
    events.send(buffer,"tuning",millis());
}

#define OTAE_REGJSON_BODY(name,type) doc["body_"#name] = (type == REGLIST_TYPE_FLOAT) ? (*regsRegisters[REGLIST_BODY(RegList_##name)].data.pf) : (*regsRegisters[REGLIST_BODY(RegList_##name)].data.pi);
#define OTAE_REGJSON_NECK(name,type) doc["neck_"#name] = (type == REGLIST_TYPE_FLOAT) ? (*regsRegisters[REGLIST_NECK(RegList_##name)].data.pf) : (*regsRegisters[REGLIST_NECK(RegList_##name)].data.pi);

void odaeSendRegs() {
    doc.clear();
    REGLIST_REGS(OTAE_REGJSON_BODY)
    REGLIST_REGS(OTAE_REGJSON_NECK)
    serializeJson(doc, buffer, sizeof(buffer));
    events.send(buffer,"regs",millis());
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
    odaeSendRegs();
    otaeSendConfig();
    otaeSendTuning();
}



