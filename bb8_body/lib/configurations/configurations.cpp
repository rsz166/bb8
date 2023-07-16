#include <configurations.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <log.h>

#define CONF_MAX_SIZE (1024)

const String confFileSysTuning = "/sysTuning.json";
const String confFileDevConf = "/devConf.json";

ConfSysTuning_t confSysTuning;
ConfDeviceConfig_t confDevConf;

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

  // load sys tuning
  if(confReadFile(confFileSysTuning, doc)) {
    JsonArrayConst objArray = doc["pids"].as<JsonArrayConst>();
    int count = 0;
    for(JsonObjectConst obj : objArray) {
      if(count >= CONF_SYS_PID_COUNT) break;
      confSysTuning.pids.pidArray[count].p = obj["p"];
      confSysTuning.pids.pidArray[count].i = obj["i"];
      confSysTuning.pids.pidArray[count].d = obj["d"];
      confSysTuning.pids.pidArray[count].sat = obj["sat"];
      confSysTuning.pids.pidArray[count].isOpenLoop = obj["isOpenLoop"];
      count++;
    }
    objArray = doc["motors"].as<JsonArrayConst>();
    count = 0;
    for(JsonObjectConst obj : objArray) {
      if(count >= CONF_SYS_MOT_COUNT) break;
      confSysTuning.motors.motArray[count].speed = obj["speed"];
      confSysTuning.motors.motArray[count].accel = obj["accel"];
      count++;
    }
  } else {
    ret = false;
  }

  // load dev config
  if(confReadFile(confFileDevConf, doc)) {
    confDevConf.mode = doc["mode"];
    confDevConf.nodeId = doc["nodeId"];
    confDevConf.apSsid = doc["ap_ssid"].as<String>();
    confDevConf.apPass = doc["ap_pass"].as<String>();
    confDevConf.wifiSsid = doc["wifi_ssid"].as<String>();
    confDevConf.wifiPass = doc["wifi_pass"].as<String>();
    confDevConf.btMac = doc["bt_mac"].as<String>();

    for(int i=0; i<4; i++) {
      confDevConf.wifiIp[i] = doc["wifi_ip"][i];
    }
    for(int i=0; i<4; i++) {
      confDevConf.wifiGateway[i] = doc["wifi_gateway"][i];
    }
    for(int i=0; i<4; i++) {
      confDevConf.wifiMask[i] = doc["wifi_mask"][i];
    }
    
    JsonArrayConst objArray = doc["motorHws"].as<JsonArrayConst>();
    int count = 0;
    for(JsonObjectConst obj : objArray) {
      if(count >= CONF_DEV_MOT_COUNT) break;
      confDevConf.motorHws.motHwArray[count].pinStep = obj["pinStep"];
      confDevConf.motorHws.motHwArray[count].pinDir = obj["pinDir"];
      confDevConf.motorHws.motHwArray[count].pinEn = obj["pinEn"];
      confDevConf.motorHws.motHwArray[count].controlMode = obj["controlMode"];
      confDevConf.motorHws.motHwArray[count].negate = obj["negate"];
      count++;
    }
  } else {
    ret = false;
  }

  return ret;
}

bool confWrite() {
  StaticJsonDocument<CONF_MAX_SIZE> doc;
  bool ret = true;

  for(int i=0; i<CONF_SYS_PID_COUNT; i++) {
    doc["pids"][i]["p"] = confSysTuning.pids.pidArray[i].p;
    doc["pids"][i]["i"] = confSysTuning.pids.pidArray[i].i;
    doc["pids"][i]["d"] = confSysTuning.pids.pidArray[i].d;
    doc["pids"][i]["sat"] = confSysTuning.pids.pidArray[i].sat;
    doc["pids"][i]["isOpenLoop"] = confSysTuning.pids.pidArray[i].isOpenLoop;
  }
  for(int i=0; i<CONF_SYS_MOT_COUNT; i++) {
    doc["motors"][i]["speed"] = confSysTuning.motors.motArray[i].speed;
    doc["motors"][i]["accel"] = confSysTuning.motors.motArray[i].accel;
  }
  if(!confWriteFile(confFileSysTuning, doc)) {
    ret = false;
  }

  doc.clear();

  doc["mode"] = confDevConf.mode;
  doc["nodeId"] = confDevConf.nodeId;
  doc["ap_ssid"] = confDevConf.apSsid;
  doc["ap_pass"] = confDevConf.apPass;
  doc["wifi_ssid"] = confDevConf.wifiSsid;
  doc["wifi_pass"] = confDevConf.wifiPass;
  doc["bt_mac"] = confDevConf.btMac;
  
  for(int i=0; i<4; i++) {
    doc["wifi_ip"][i] = confDevConf.wifiIp[i];
  }
  for(int i=0; i<4; i++) {
    doc["wifi_gateway"][i] = confDevConf.wifiGateway[i];
  }
  for(int i=0; i<4; i++) {
    doc["wifi_mask"][i] = confDevConf.wifiMask[i];
  }

  for(int i=0; i<CONF_SYS_PID_COUNT; i++) {
    doc["motorHws"][i]["pinStep"] = confDevConf.motorHws.motHwArray[i].pinStep;
    doc["motorHws"][i]["pinDir"] = confDevConf.motorHws.motHwArray[i].pinDir;
    doc["motorHws"][i]["pinEn"] = confDevConf.motorHws.motHwArray[i].pinEn;
    doc["motorHws"][i]["controlMode"] = confDevConf.motorHws.motHwArray[i].controlMode;
    doc["motorHws"][i]["negate"] = confDevConf.motorHws.motHwArray[i].negate;
  }
  if(!confWriteFile(confFileDevConf, doc)) {
    ret = false;
  }

  return ret;
}

String confGetTuningFile(){
  LOG_F("getFile -> Reading file: %s\n", confFileDevConf.c_str());

  File file = SPIFFS.open(confFileDevConf);
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
