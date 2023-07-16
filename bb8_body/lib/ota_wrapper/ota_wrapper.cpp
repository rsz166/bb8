#include <ota_wrapper.h>
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <configurations.h>
#include <log.h>
#include <registers.h>
#include <register_list.h>
#include <SPIFFS.h>
#include <stepper_wrapper.h>
#include <ArduinoJson.h>
#include <ota_events.h>

AsyncWebServer server(80);
bool apMode = false;

void otaApConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Device connected to AP");
}

bool otaNetworkInitAP(const char* ssid, const char* pass) {
  apMode = true;
  WiFi.disconnect();
  if(strlen(ssid) == 0) {
    Serial.printf("Invalid AP SSID: %s, using default: ESP\n", ssid);
    ssid = "ESP";
    pass = "";
    return false;
  }
  bool ret = WiFi.softAP(ssid, pass);
  IPAddress ip(192, 168, 5, 1);
  IPAddress gateway(192, 168, 5, 1);
  IPAddress subnet(255, 255, 255, 0);
  if(ret) {
    ret = WiFi.softAPConfig(ip, gateway, subnet);
    WiFi.onEvent(otaApConnected, SYSTEM_EVENT_AP_STACONNECTED);
  }
  return ret;
}

bool otaNetworkInitSTA(const char* ssid, const char* pass, byte *ip, byte *gateway, byte *mask) {
  if(strlen(ssid) == 0) {
    Serial.printf("Invalid STA SSID: %s\n", ssid);
    return false;
  }
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("bb8");
  if((ip[0] != 0) || (ip[1] != 0) || (ip[2] != 0) || (ip[3] != 0)) {
    IPAddress ipAddr(ip[0], ip[1], ip[2], ip[3]);
    IPAddress gatewayAddr(gateway[0], gateway[1], gateway[2], gateway[3]);
    IPAddress subnetAddr(mask[0], mask[1], mask[2], mask[3]);
    WiFi.config(ipAddr, gatewayAddr, subnetAddr);
  }
  WiFi.begin(ssid, pass);
  return WiFi.waitForConnectResult() == WL_CONNECTED;
}

String otaCreatePidTable() {
  String response = "<h1>PID tune</h1><form method=\"POST\"><table><thead><tr><td>idx</td><td>p</td><td>i</td><td>d</td><td>sat</td></tr></thead><tbody>";
    for(int i=0; i<CONF_SYS_PID_COUNT; i++) {
      response.concat("<tr><td>" + String(i) + "</td>");
      response.concat("<td><input type=\"text\" name=\"p" + String(i) + "\" value=\"" + String(confSysTuning.pids.pidArray[i].p) + "\"></td>");
      response.concat("<td><input type=\"text\" name=\"i" + String(i) + "\" value=\"" + String(confSysTuning.pids.pidArray[i].i) + "\"></td>");
      response.concat("<td><input type=\"text\" name=\"d" + String(i) + "\" value=\"" + String(confSysTuning.pids.pidArray[i].d) + "\"></td>");
      response.concat("<td><input type=\"text\" name=\"s" + String(i) + "\" value=\"" + String(confSysTuning.pids.pidArray[i].sat) + "\"></td></tr>");
    }
    response.concat("</tbody></table><input type =\"submit\" value =\"Submit\"></form></br><a href=\"/pidraw\">Download JSON</a>");
    return response;
}

String otaArgProcessor(const String& var){
  if(var == "conf_apSsid") return String(confDevConf.apSsid);
  if(var == "conf_wifiSsid") return String(confDevConf.wifiSsid);
  if(var == "conf_btMac") return String(confDevConf.btMac);
  if(var == "conf_nodeId") return String(confDevConf.nodeId);
  if(var == "conf_mode") return String(confDevConf.mode);

  if(var == "reg_body_mode") return String(*regsRegisters[REGLIST_BODY(RegList_mode)].data.pi);
  if(var == "reg_body_uptime") return String(*regsRegisters[REGLIST_BODY(RegList_uptime)].data.pi);
  if(var == "reg_body_status") return String(*regsRegisters[REGLIST_BODY(RegList_status)].data.pi);
  if(var == "reg_body_errorCode") return String(*regsRegisters[REGLIST_BODY(RegList_errorCode)].data.pi);
  if(var == "reg_body_batteryVoltage") return String(*regsRegisters[REGLIST_BODY(RegList_batteryVoltage)].data.pf);
  if(var == "reg_body_requestedMode") return String(*regsRegisters[REGLIST_BODY(RegList_requestedMode)].data.pi);
  if(var == "reg_body_ctrlForw_setp") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlForw_setp)].data.pf);
  if(var == "reg_body_ctrlTilt_setp") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlTilt_setp)].data.pf);
  if(var == "reg_body_ctrlRota_setp") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlRota_setp)].data.pf);
  if(var == "reg_body_ctrlForw_feedback") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlForw_feedback)].data.pf);
  if(var == "reg_body_ctrlTilt_feedback") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlTilt_feedback)].data.pf);
  if(var == "reg_body_ctrlRota_feedback") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlRota_feedback)].data.pf);
  if(var == "reg_body_ctrlForw_act") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlForw_act)].data.pf);
  if(var == "reg_body_ctrlTilt_act") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlTilt_act)].data.pf);
  if(var == "reg_body_ctrlRota_act") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlRota_act)].data.pf);
  if(var == "reg_body_ctrlForw_p") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlForw_p)].data.pf);
  if(var == "reg_body_ctrlTilt_p") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlTilt_p)].data.pf);
  if(var == "reg_body_ctrlRota_p") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlRota_p)].data.pf);
  if(var == "reg_body_ctrlForw_i") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlForw_i)].data.pf);
  if(var == "reg_body_ctrlTilt_i") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlTilt_i)].data.pf);
  if(var == "reg_body_ctrlRota_i") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlRota_i)].data.pf);
  if(var == "reg_body_ctrlForw_d") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlForw_d)].data.pf);
  if(var == "reg_body_ctrlTilt_d") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlTilt_d)].data.pf);
  if(var == "reg_body_ctrlRota_d") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlRota_d)].data.pf);
  if(var == "reg_body_ctrlForw_sat") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlForw_sat)].data.pf);
  if(var == "reg_body_ctrlTilt_sat") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlTilt_sat)].data.pf);
  if(var == "reg_body_ctrlRota_sat") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlRota_sat)].data.pf);
  if(var == "reg_neck_mode") return String(*regsRegisters[REGLIST_NECK(RegList_mode)].data.pi);
  if(var == "reg_neck_uptime") return String(*regsRegisters[REGLIST_NECK(RegList_uptime)].data.pi);
  if(var == "reg_neck_status") return String(*regsRegisters[REGLIST_NECK(RegList_status)].data.pi);
  if(var == "reg_neck_errorCode") return String(*regsRegisters[REGLIST_NECK(RegList_errorCode)].data.pi);
  if(var == "reg_neck_batteryVoltage") return String(*regsRegisters[REGLIST_NECK(RegList_batteryVoltage)].data.pf);
  if(var == "reg_neck_requestedMode") return String(*regsRegisters[REGLIST_NECK(RegList_requestedMode)].data.pi);
  if(var == "reg_neck_ctrlForw_setp") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlForw_setp)].data.pf);
  if(var == "reg_neck_ctrlTilt_setp") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlTilt_setp)].data.pf);
  if(var == "reg_neck_ctrlRota_setp") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlRota_setp)].data.pf);
  if(var == "reg_neck_ctrlForw_feedback") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlForw_feedback)].data.pf);
  if(var == "reg_neck_ctrlTilt_feedback") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlTilt_feedback)].data.pf);
  if(var == "reg_neck_ctrlRota_feedback") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlRota_feedback)].data.pf);
  if(var == "reg_neck_ctrlForw_act") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlForw_act)].data.pf);
  if(var == "reg_neck_ctrlTilt_act") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlTilt_act)].data.pf);
  if(var == "reg_neck_ctrlRota_act") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlRota_act)].data.pf);
  if(var == "reg_neck_ctrlForw_p") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlForw_p)].data.pf);
  if(var == "reg_neck_ctrlTilt_p") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlTilt_p)].data.pf);
  if(var == "reg_neck_ctrlRota_p") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlRota_p)].data.pf);
  if(var == "reg_neck_ctrlForw_i") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlForw_i)].data.pf);
  if(var == "reg_neck_ctrlTilt_i") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlTilt_i)].data.pf);
  if(var == "reg_neck_ctrlRota_i") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlRota_i)].data.pf);
  if(var == "reg_neck_ctrlForw_d") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlForw_d)].data.pf);
  if(var == "reg_neck_ctrlTilt_d") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlTilt_d)].data.pf);
  if(var == "reg_neck_ctrlRota_d") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlRota_d)].data.pf);
  if(var == "reg_neck_ctrlForw_sat") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlForw_sat)].data.pf);
  if(var == "reg_neck_ctrlTilt_sat") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlTilt_sat)].data.pf);
  if(var == "reg_neck_ctrlRota_sat") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlRota_sat)].data.pf);

  // if(var == "motorSpeed") return String(stepperSpeed);

  return String();
}

// handles uploads
void otaHandleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    // open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open("/" + filename, "w");
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
  }

  if (final) {
    // close the file handle as the upload is now done
    request->_tempFile.close();
    request->redirect("/");
  }
}

void otaSaveParameter(const String& name, const String& value) {
  LOG_F("Set %s=%s\n", name.c_str(), value.c_str());
  if(name == "body_mode") *regsRegisters[REGLIST_BODY(RegList_mode)].data.pi = value.toInt();
  if(name == "body_requestedMode") *regsRegisters[REGLIST_BODY(RegList_requestedMode)].data.pi = value.toInt();
  if(name == "body_enableMotors") *regsRegisters[REGLIST_BODY(RegList_enableMotors)].data.pi = value.toInt();
  if(name == "body_ctrlForw_setp") *regsRegisters[REGLIST_BODY(RegList_ctrlForw_setp)].data.pf = value.toFloat();
  if(name == "body_ctrlTilt_setp") *regsRegisters[REGLIST_BODY(RegList_ctrlTilt_setp)].data.pf = value.toFloat();
  if(name == "body_ctrlRota_setp") *regsRegisters[REGLIST_BODY(RegList_ctrlRota_setp)].data.pf = value.toFloat();
  if(name == "body_ctrlForw_feedback") *regsRegisters[REGLIST_BODY(RegList_ctrlForw_feedback)].data.pf = value.toFloat();
  if(name == "body_ctrlTilt_feedback") *regsRegisters[REGLIST_BODY(RegList_ctrlTilt_feedback)].data.pf = value.toFloat();
  if(name == "body_ctrlRota_feedback") *regsRegisters[REGLIST_BODY(RegList_ctrlRota_feedback)].data.pf = value.toFloat();
  if(name == "body_ctrlForw_act") *regsRegisters[REGLIST_BODY(RegList_ctrlForw_act)].data.pf = value.toFloat();
  if(name == "body_ctrlTilt_act") *regsRegisters[REGLIST_BODY(RegList_ctrlTilt_act)].data.pf = value.toFloat();
  if(name == "body_ctrlRota_act") *regsRegisters[REGLIST_BODY(RegList_ctrlRota_act)].data.pf = value.toFloat();
  if(name == "body_ctrlForw_p") *regsRegisters[REGLIST_BODY(RegList_ctrlForw_p)].data.pf = value.toFloat();
  if(name == "body_ctrlTilt_p") *regsRegisters[REGLIST_BODY(RegList_ctrlTilt_p)].data.pf = value.toFloat();
  if(name == "body_ctrlRota_p") *regsRegisters[REGLIST_BODY(RegList_ctrlRota_p)].data.pf = value.toFloat();
  if(name == "body_ctrlForw_i") *regsRegisters[REGLIST_BODY(RegList_ctrlForw_i)].data.pf = value.toFloat();
  if(name == "body_ctrlTilt_i") *regsRegisters[REGLIST_BODY(RegList_ctrlTilt_i)].data.pf = value.toFloat();
  if(name == "body_ctrlRota_i") *regsRegisters[REGLIST_BODY(RegList_ctrlRota_i)].data.pf = value.toFloat();
  if(name == "body_ctrlForw_d") *regsRegisters[REGLIST_BODY(RegList_ctrlForw_d)].data.pf = value.toFloat();
  if(name == "body_ctrlTilt_d") *regsRegisters[REGLIST_BODY(RegList_ctrlTilt_d)].data.pf = value.toFloat();
  if(name == "body_ctrlRota_d") *regsRegisters[REGLIST_BODY(RegList_ctrlRota_d)].data.pf = value.toFloat();
  if(name == "body_ctrlForw_sat") *regsRegisters[REGLIST_BODY(RegList_ctrlForw_sat)].data.pf = value.toFloat();
  if(name == "body_ctrlTilt_sat") *regsRegisters[REGLIST_BODY(RegList_ctrlTilt_sat)].data.pf = value.toFloat();
  if(name == "body_ctrlRota_sat") *regsRegisters[REGLIST_BODY(RegList_ctrlRota_sat)].data.pf = value.toFloat();
  if(name == "body_ctrlForw_isOpenLoop") *regsRegisters[REGLIST_BODY(RegList_ctrlForw_isOpenLoop)].data.pi = value.toInt();
  if(name == "body_ctrlTilt_isOpenLoop") *regsRegisters[REGLIST_BODY(RegList_ctrlTilt_isOpenLoop)].data.pi = value.toInt();
  if(name == "body_ctrlRota_isOpenLoop") *regsRegisters[REGLIST_BODY(RegList_ctrlRota_isOpenLoop)].data.pi = value.toInt();
  if(name == "body_ctrlForw_speedLimit") *regsRegisters[REGLIST_BODY(RegList_ctrlForw_speedLimit)].data.pf = value.toFloat();
  if(name == "body_ctrlTilt_speedLimit") *regsRegisters[REGLIST_BODY(RegList_ctrlTilt_speedLimit)].data.pf = value.toFloat();
  if(name == "body_ctrlRota_speedLimit") *regsRegisters[REGLIST_BODY(RegList_ctrlRota_speedLimit)].data.pf = value.toFloat();
  if(name == "body_ctrlForw_accelLimit") *regsRegisters[REGLIST_BODY(RegList_ctrlForw_accelLimit)].data.pf = value.toFloat();
  if(name == "body_ctrlTilt_accelLimit") *regsRegisters[REGLIST_BODY(RegList_ctrlTilt_accelLimit)].data.pf = value.toFloat();
  if(name == "body_ctrlRota_accelLimit") *regsRegisters[REGLIST_BODY(RegList_ctrlRota_accelLimit)].data.pf = value.toFloat();
  if(name == "neck_mode") *regsRegisters[REGLIST_NECK(RegList_mode)].data.pi = value.toInt();
  if(name == "neck_requestedMode") *regsRegisters[REGLIST_NECK(RegList_requestedMode)].data.pi = value.toInt();
  if(name == "neck_enableMotors") *regsRegisters[REGLIST_NECK(RegList_enableMotors)].data.pi = value.toInt();
  if(name == "neck_ctrlForw_setp") *regsRegisters[REGLIST_NECK(RegList_ctrlForw_setp)].data.pf = value.toFloat();
  if(name == "neck_ctrlTilt_setp") *regsRegisters[REGLIST_NECK(RegList_ctrlTilt_setp)].data.pf = value.toFloat();
  if(name == "neck_ctrlRota_setp") *regsRegisters[REGLIST_NECK(RegList_ctrlRota_setp)].data.pf = value.toFloat();
  if(name == "neck_ctrlForw_feedback") *regsRegisters[REGLIST_NECK(RegList_ctrlForw_feedback)].data.pf = value.toFloat();
  if(name == "neck_ctrlTilt_feedback") *regsRegisters[REGLIST_NECK(RegList_ctrlTilt_feedback)].data.pf = value.toFloat();
  if(name == "neck_ctrlRota_feedback") *regsRegisters[REGLIST_NECK(RegList_ctrlRota_feedback)].data.pf = value.toFloat();
  if(name == "neck_ctrlForw_act") *regsRegisters[REGLIST_NECK(RegList_ctrlForw_act)].data.pf = value.toFloat();
  if(name == "neck_ctrlTilt_act") *regsRegisters[REGLIST_NECK(RegList_ctrlTilt_act)].data.pf = value.toFloat();
  if(name == "neck_ctrlRota_act") *regsRegisters[REGLIST_NECK(RegList_ctrlRota_act)].data.pf = value.toFloat();
  if(name == "neck_ctrlForw_p") *regsRegisters[REGLIST_NECK(RegList_ctrlForw_p)].data.pf = value.toFloat();
  if(name == "neck_ctrlTilt_p") *regsRegisters[REGLIST_NECK(RegList_ctrlTilt_p)].data.pf = value.toFloat();
  if(name == "neck_ctrlRota_p") *regsRegisters[REGLIST_NECK(RegList_ctrlRota_p)].data.pf = value.toFloat();
  if(name == "neck_ctrlForw_i") *regsRegisters[REGLIST_NECK(RegList_ctrlForw_i)].data.pf = value.toFloat();
  if(name == "neck_ctrlTilt_i") *regsRegisters[REGLIST_NECK(RegList_ctrlTilt_i)].data.pf = value.toFloat();
  if(name == "neck_ctrlRota_i") *regsRegisters[REGLIST_NECK(RegList_ctrlRota_i)].data.pf = value.toFloat();
  if(name == "neck_ctrlForw_d") *regsRegisters[REGLIST_NECK(RegList_ctrlForw_d)].data.pf = value.toFloat();
  if(name == "neck_ctrlTilt_d") *regsRegisters[REGLIST_NECK(RegList_ctrlTilt_d)].data.pf = value.toFloat();
  if(name == "neck_ctrlRota_d") *regsRegisters[REGLIST_NECK(RegList_ctrlRota_d)].data.pf = value.toFloat();
  if(name == "neck_ctrlForw_sat") *regsRegisters[REGLIST_NECK(RegList_ctrlForw_sat)].data.pf = value.toFloat();
  if(name == "neck_ctrlTilt_sat") *regsRegisters[REGLIST_NECK(RegList_ctrlTilt_sat)].data.pf = value.toFloat();
  if(name == "neck_ctrlRota_sat") *regsRegisters[REGLIST_NECK(RegList_ctrlRota_sat)].data.pf = value.toFloat();
  if(name == "neck_ctrlForw_isOpenLoop") *regsRegisters[REGLIST_NECK(RegList_ctrlForw_isOpenLoop)].data.pi = value.toInt();
  if(name == "neck_ctrlTilt_isOpenLoop") *regsRegisters[REGLIST_NECK(RegList_ctrlTilt_isOpenLoop)].data.pi = value.toInt();
  if(name == "neck_ctrlRota_isOpenLoop") *regsRegisters[REGLIST_NECK(RegList_ctrlRota_isOpenLoop)].data.pi = value.toInt();
  if(name == "neck_ctrlForw_speedLimit") *regsRegisters[REGLIST_NECK(RegList_ctrlForw_speedLimit)].data.pf = value.toFloat();
  if(name == "neck_ctrlTilt_speedLimit") *regsRegisters[REGLIST_NECK(RegList_ctrlTilt_speedLimit)].data.pf = value.toFloat();
  if(name == "neck_ctrlRota_speedLimit") *regsRegisters[REGLIST_NECK(RegList_ctrlRota_speedLimit)].data.pf = value.toFloat();
  if(name == "neck_ctrlForw_accelLimit") *regsRegisters[REGLIST_NECK(RegList_ctrlForw_accelLimit)].data.pf = value.toFloat();
  if(name == "neck_ctrlTilt_accelLimit") *regsRegisters[REGLIST_NECK(RegList_ctrlTilt_accelLimit)].data.pf = value.toFloat();
  if(name == "neck_ctrlRota_accelLimit") *regsRegisters[REGLIST_NECK(RegList_ctrlRota_accelLimit)].data.pf = value.toFloat();
}

#define OTA_REGSET(reg, type, value) if(type==REGLIST_TYPE_INT) *reg.data.pi = value.toInt(); else *reg.data.pf = value.toFloat();
#define OTA_REGSET_BODY(namex,type) if(name == "regs_body_"#namex) {OTA_REGSET(regsRegisters[REGLIST_BODY(RegList_##namex)], type, value) return true;}
#define OTA_REGSET_NECK(namex,type) if(name == "regs_neck_"#namex) {OTA_REGSET(regsRegisters[REGLIST_NECK(RegList_##namex)], type, value) return true;}

bool otaSetParam(const String &name, const String &value) {
  if(name.startsWith("config_")) {
    if(name == "config_apSsid") { confDevConf.apSsid = value; }
    else if(name == "config_apPass") { confDevConf.apPass = value; }
    else if(name == "config_wifiSsid") { confDevConf.wifiSsid = value; }
    else if(name == "config_wifiPass") { confDevConf.wifiPass = value; }
    else if(name == "config_btMac") { confDevConf.btMac = value; }
    else if(name == "config_mode") { confDevConf.mode = value.toInt(); }
    else if(name == "config_nodeId") { confDevConf.nodeId = value.toInt(); }
    else if(name.startsWith("config_motorHws_")) {
      // config_motorHws_0_pinStep
      int idx = name[16]-'0';
      if(name.endsWith("_pinStep")) confDevConf.motorHws.motHwArray[idx].pinStep = value.toInt();
      else return false;
    }
    else return false;
    confWrite();
    return true;
  } else if(name.startsWith("tuning_")) {
    if(name.startsWith("tuning_motors_")) {
      // tuning_motors_0_pinStep
      int idx = name[14]-'0';
      if(name.endsWith("_accel")) confSysTuning.motors.motArray[idx].accel = value.toFloat();
      else if(name.endsWith("_speed")) confSysTuning.motors.motArray[idx].speed = value.toFloat();
      else return false;
    } else if(name.startsWith("tuning_pids_")) { 
      // tuning_pids_0_p
      int idx = name[12]-'0';
      if(name.endsWith("_p")) confSysTuning.pids.pidArray[idx].p = value.toFloat();
      else if(name.endsWith("_i")) confSysTuning.pids.pidArray[idx].i = value.toFloat();
      else if(name.endsWith("_d")) confSysTuning.pids.pidArray[idx].d = value.toFloat();
      else if(name.endsWith("_sat")) confSysTuning.pids.pidArray[idx].sat = value.toFloat();
      else return false;
    }
    else return false;
    confWrite();
    return true;
  } else if(name.startsWith("regs_")) {
    REGLIST_REGS(OTA_REGSET_BODY)
    REGLIST_REGS(OTA_REGSET_NECK)
  }
  return false;
}

#define OTA_REGISTER_HTML(name,method) server.on("/" name, method, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/" name ".html", "text/html", false, otaArgProcessor); });

void otaRegisterPages() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Index page opened");
    request->send(SPIFFS, "/index.html", "text/html", false, otaArgProcessor);
  });
  OTA_REGISTER_HTML("test", HTTP_GET);
  OTA_REGISTER_HTML("auth", HTTP_GET);
  OTA_REGISTER_HTML("reg", HTTP_GET);
  OTA_REGISTER_HTML("params", HTTP_GET);
  OTA_REGISTER_HTML("motor", HTTP_GET);
  OTA_REGISTER_HTML("upload", HTTP_GET);
  OTA_REGISTER_HTML("mpu", HTTP_GET);

  server.on("/pid", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", otaCreatePidTable());
  });
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    ESP.restart();
  });
  server.on("/pid", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost() && p->name().length() >= 2){
        int idx = p->name().substring(1).toInt();
        switch(p->name()[0]) {
          case 'p': confSysTuning.pids.pidArray[idx].p = p->value().toFloat(); break;
          case 'i': confSysTuning.pids.pidArray[idx].i = p->value().toFloat(); break;
          case 'd': confSysTuning.pids.pidArray[idx].d = p->value().toFloat(); break;
          case 's': confSysTuning.pids.pidArray[idx].sat = p->value().toFloat(); break;
        }
      }
    }
    confWrite();
    request->send(200, "text/html", otaCreatePidTable());
  });
  server.on("/auth", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        if (p->name() == "ssid") confDevConf.wifiSsid = p->value();
        if (p->name() == "pass" && p->value() != "***") confDevConf.wifiPass = p->value();
        if (p->name() == "btmac" && p->value() != "") confDevConf.btMac = p->value();
      }
    }
    confWrite();
    request->send(SPIFFS, "/auth.html", "text/html", false, otaArgProcessor);
  });
  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", String("Mode switched to bluetooth"));
    confDevConf.mode = CONF_MODE_BT;
    confWrite();
    delay(500);
    ESP.restart();
  });
  server.on("/motor", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      // if(p->isPost()){ // TODO
      //   if (p->name() == "speed" && p->value() != "") stepperSpeed = p->value().toFloat();
      //   if (p->name() == "move" && p->value() != "") {
      //     if(p->value() == "Stop") stepperStop = true;
      //     else stepperMove = p->value().toFloat();
      //   }
      // }
    }
    request->send(SPIFFS, "/motor.html", "text/html", false, otaArgProcessor);
  });
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200);
  }, otaHandleUpload);
  server.on("/set", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    bool ret = true;
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(!otaSetParam(p->name(), p->value())) ret = false;
    }
    request->send(ret ? 200 : 400, "text/html", ret ? "Ok" : "Error");
  });
  server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request) {
    int params = request->params();
    bool ret = true;
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(!otaSetParam(p->name(), p->value())) ret = false;
    }
    request->send(ret ? 200 : 400, "text/html", ret ? "Ok" : "Error");
  });
}

void otaInit() {
  otaRegisterPages();

  AsyncElegantOTA.begin(&server);

  server.serveStatic("/", SPIFFS, "/");

  otaeInit(&server);

  server.begin();

  Serial.println("Ready");
  if(apMode) {
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void otaHandle() {
  otaeHandle();
}
