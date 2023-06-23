#include <configurations.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <log.h>

#define CONF_MAX_SIZE (1024)

const String confFileTuning = "/tuning.json";
const String confFileAuth = "/auth.json";

ConfTuning_t confTuning;
ConfAuth_t confAuth;

bool confWriteFile(String filename, JsonDocument &doc){
  LOG_F("writeFile -> Writing file: %s\n", filename.c_str());

  File file = SPIFFS.open(filename, FILE_WRITE);
  if(!file || file.isDirectory()) {
    LOG_S("writeFile -> failed to open file for writing");
    return false;
  }

  serializeJson(doc, file);
  
  file.close();

  return true;
}

bool confReadFile(String filename, JsonDocument &doc){
  LOG_F("readFile -> Reading file: %s\n", filename.c_str());

  File file = SPIFFS.open(filename);
  if(!file || file.isDirectory()) {
    LOG_S("readFile -> failed to open file for reading");
    return false;
  }

  bool ret = false;

  int len = file.size();
  LOG_F("Config file size: %i\n", len);
  if(len > CONF_MAX_SIZE) {
    LOG_S("Config file too large");
  } else {
    doc.clear();
    auto error = deserializeJson(doc, file);
    if ( error ) { 
      LOG_S("Error interpreting config file");
    } else {
      ret = true;
    }
  }

  file.close();

  return ret;
}

bool confRead() {
  StaticJsonDocument<CONF_MAX_SIZE> doc;
  bool ret = true;

  // load tuning
  if(confReadFile(confFileTuning, doc)) {
    JsonArrayConst pidArray = doc["pids"].as<JsonArrayConst>();
    int pidCount = 0;
    for(JsonObjectConst pidObj : pidArray) {
      if(pidCount >= CONF_PID_COUNT) break;
      confTuning.pid.pidArray[pidCount].p = pidObj["p"];
      confTuning.pid.pidArray[pidCount].i = pidObj["i"];
      confTuning.pid.pidArray[pidCount].d = pidObj["d"];
      confTuning.pid.pidArray[pidCount].sat = pidObj["sat"];
      pidCount++;
    }
    confTuning.mode = doc["mode"];
    confTuning.nodeId = doc["nodeId"];
  } else {
    ret = false;
  }

  // load auth
  if(confReadFile(confFileAuth, doc)) {
    confAuth.wifiSsid = doc["wifi_ssid"].as<String>();
    confAuth.wifiPass = doc["wifi_pass"].as<String>();
    confAuth.btMac = doc["bt_mac"].as<String>();
  } else {
    ret = false;
  }

  return ret;
}

bool confWrite() {
  StaticJsonDocument<CONF_MAX_SIZE> doc;
  bool ret = true;

  for(int i=0; i<CONF_PID_COUNT; i++) {
    doc["pids"][i]["p"] = confTuning.pid.pidArray[i].p;
    doc["pids"][i]["i"] = confTuning.pid.pidArray[i].i;
    doc["pids"][i]["d"] = confTuning.pid.pidArray[i].d;
    doc["pids"][i]["sat"] = confTuning.pid.pidArray[i].sat;
  }
  doc["mode"] = confTuning.mode;
  doc["nodeId"] = confTuning.nodeId;
  if(!confWriteFile(confFileTuning, doc)) {
    ret = false;
  }

  doc["wifi_ssid"] = confAuth.wifiSsid;
  doc["wifi_pass"] = confAuth.wifiPass;
  doc["bt_mac"] = confAuth.btMac;
  if(!confWriteFile(confFileAuth, doc)) {
    ret = false;
  }

  return ret;
}

String confGetTuningFile(){
  LOG_F("getFile -> Reading file: %s\n", confFileTuning.c_str());

  File file = SPIFFS.open(confFileTuning);
  if(!file || file.isDirectory()) {
    LOG_S("readFile -> failed to open file for reading");
    return "";
  }

  String fileText = "";
  while(file.available()){
    fileText = file.readString();
  }
  
  file.close();
  return fileText;
}

void confInit() {
  if(!SPIFFS.begin(true)){
    LOG_S("SPIFFS mount failed");
  }
  else{
    LOG_S("SPIFFS mounted successfully");
    if(confRead() == false) {
      LOG_S("Could not read config file, initializing new file");
      if (confWrite()) { // TODO: default values
        LOG_S("Config file saved");
      }
    }
  }
}
