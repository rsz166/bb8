#include <ota_wrapper.h>
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <WebSerial.h>
#include <configurations.h>
#include <log.h>
#include <registers.h>
#include <register_list.h>
#include <SPIFFS.h>

AsyncWebServer server(80);
bool apMode = false;

bool otaNetworkInitAP(const char* ssid) {
  apMode = true;
  WiFi.disconnect();
  bool ret = WiFi.softAP(ssid);
  IPAddress ip(192, 168, 5, 1);
  IPAddress gateway(192, 168, 5, 1);
  IPAddress subnet(255, 255, 255, 0);
  if(ret) {
    ret = WiFi.softAPConfig(ip, gateway, subnet);
  }
  return ret;
}

bool otaNetworkInitSTA(const char* ssid, const char* pass) {
  if(strlen(ssid) == 0) {
    Serial.printf("Invalid STA SSID: %s\n", ssid);
    return false;
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  return WiFi.waitForConnectResult() == WL_CONNECTED;
}

String otaCreatePidTable() {
  String response = "<h1>PID tune</h1><form method=\"POST\"><table><thead><tr><td>idx</td><td>p</td><td>i</td><td>d</td><td>sat</td></tr></thead><tbody>";
    for(int i=0; i<CONF_PID_COUNT; i++) {
      response.concat("<tr><td>" + String(i) + "</td>");
      response.concat("<td><input type=\"text\" name=\"p" + String(i) + "\" value=\"" + String(confTuning.pid.pidArray[i].p) + "\"></td>");
      response.concat("<td><input type=\"text\" name=\"i" + String(i) + "\" value=\"" + String(confTuning.pid.pidArray[i].i) + "\"></td>");
      response.concat("<td><input type=\"text\" name=\"d" + String(i) + "\" value=\"" + String(confTuning.pid.pidArray[i].d) + "\"></td>");
      response.concat("<td><input type=\"text\" name=\"s" + String(i) + "\" value=\"" + String(confTuning.pid.pidArray[i].sat) + "\"></td></tr>");
    }
    response.concat("</tbody></table><input type =\"submit\" value =\"Submit\"></form></br><a href=\"/pidraw\">Download JSON</a>");
    return response;
}

String otaCreateRegTable() {
  String response = "<h1>Registers</h1><form method=\"POST\"><table><thead><tr><td>idx</td><td>int</td><td>float</td><td>rx</td><td>write int</td><td>write float</td></tr></thead><tbody>";
    for(int i=0; i<REGS_REG_CNT; i++) {
      response.concat("<tr><td>" + String(i) + "</td>");
      response.concat("<td><input type=\"text\" name=\"i" + String(i) + "\" value=\"" + String(*regsRegisters[i].data.pi) + "\"></td>");
      response.concat("<td><input type=\"text\" name=\"f" + String(i) + "\" value=\"" + String(*regsRegisters[i].data.pf) + "\"></td>");
      response.concat("<td>" + String(regsRegisters[i].isRx ? "RX" : "TX") + "</td>");
      response.concat("<td><input type=\"submit\" name=\"wi" + String(i) + "\" value=\"int\"></td>");
      response.concat("<td><input type=\"submit\" name=\"wf" + String(i) + "\" value=\"float\"></td></tr>");
    }
    response.concat("</tbody></table></form>");
    return response;
}

String otaArgProcessor(const String& var){
  if(var == "conf_wifiSsid") return String(confAuth.wifiSsid);
  if(var == "conf_btMac") return String(confAuth.btMac);
  if(var == "conf_nodeId") return String(confTuning.nodeId);
  if(var == "conf_mode") return String(confTuning.mode);

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
  if(var == "reg_body_ctrlForw_actMode") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlForw_actMode)].data.pf);
  if(var == "reg_body_ctrlTilt_actMode") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlTilt_actMode)].data.pf);
  if(var == "reg_body_ctrlRota_actMode") return String(*regsRegisters[REGLIST_BODY(RegList_ctrlRota_actMode)].data.pf);
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
  if(var == "reg_neck_ctrlForw_actMode") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlForw_actMode)].data.pf);
  if(var == "reg_neck_ctrlTilt_actMode") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlTilt_actMode)].data.pf);
  if(var == "reg_neck_ctrlRota_actMode") return String(*regsRegisters[REGLIST_NECK(RegList_ctrlRota_actMode)].data.pf);
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
  if(name == "body_ctrlForw_setp") *regsRegisters[REGLIST_BODY(RegList_ctrlForw_setp)].data.pf = value.toFloat();
  if(name == "body_ctrlTilt_setp") *regsRegisters[REGLIST_BODY(RegList_ctrlTilt_setp)].data.pf = value.toFloat();
  if(name == "body_ctrlRota_setp") *regsRegisters[REGLIST_BODY(RegList_ctrlRota_setp)].data.pf = value.toFloat();
  if(name == "body_ctrlForw_feedback") *regsRegisters[REGLIST_BODY(RegList_ctrlForw_feedback)].data.pf = value.toFloat();
  if(name == "body_ctrlTilt_feedback") *regsRegisters[REGLIST_BODY(RegList_ctrlTilt_feedback)].data.pf = value.toFloat();
  if(name == "body_ctrlRota_feedback") *regsRegisters[REGLIST_BODY(RegList_ctrlRota_feedback)].data.pf = value.toFloat();
  if(name == "body_ctrlForw_actMode") *regsRegisters[REGLIST_BODY(RegList_ctrlForw_actMode)].data.pf = value.toFloat();
  if(name == "body_ctrlTilt_actMode") *regsRegisters[REGLIST_BODY(RegList_ctrlTilt_actMode)].data.pf = value.toFloat();
  if(name == "body_ctrlRota_actMode") *regsRegisters[REGLIST_BODY(RegList_ctrlRota_actMode)].data.pf = value.toFloat();
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
  if(name == "neck_mode") *regsRegisters[REGLIST_NECK(RegList_mode)].data.pi = value.toInt();
  if(name == "neck_requestedMode") *regsRegisters[REGLIST_NECK(RegList_requestedMode)].data.pi = value.toInt();
  if(name == "neck_ctrlForw_setp") *regsRegisters[REGLIST_NECK(RegList_ctrlForw_setp)].data.pf = value.toFloat();
  if(name == "neck_ctrlTilt_setp") *regsRegisters[REGLIST_NECK(RegList_ctrlTilt_setp)].data.pf = value.toFloat();
  if(name == "neck_ctrlRota_setp") *regsRegisters[REGLIST_NECK(RegList_ctrlRota_setp)].data.pf = value.toFloat();
  if(name == "neck_ctrlForw_feedback") *regsRegisters[REGLIST_NECK(RegList_ctrlForw_feedback)].data.pf = value.toFloat();
  if(name == "neck_ctrlTilt_feedback") *regsRegisters[REGLIST_NECK(RegList_ctrlTilt_feedback)].data.pf = value.toFloat();
  if(name == "neck_ctrlRota_feedback") *regsRegisters[REGLIST_NECK(RegList_ctrlRota_feedback)].data.pf = value.toFloat();
  if(name == "neck_ctrlForw_actMode") *regsRegisters[REGLIST_NECK(RegList_ctrlForw_actMode)].data.pf = value.toFloat();
  if(name == "neck_ctrlTilt_actMode") *regsRegisters[REGLIST_NECK(RegList_ctrlTilt_actMode)].data.pf = value.toFloat();
  if(name == "neck_ctrlRota_actMode") *regsRegisters[REGLIST_NECK(RegList_ctrlRota_actMode)].data.pf = value.toFloat();
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
}

void otaRegisterPages() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html", false, otaArgProcessor);
  });
  server.on("/pidraw", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/json", confGetTuningFile());
  });
  server.on("/pid", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", otaCreatePidTable());
  });
  server.on("/pid", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost() && p->name().length() >= 2){
        int idx = p->name().substring(1).toInt();
        switch(p->name()[0]) {
          case 'p': confTuning.pid.pidArray[idx].p = p->value().toFloat(); break;
          case 'i': confTuning.pid.pidArray[idx].i = p->value().toFloat(); break;
          case 'd': confTuning.pid.pidArray[idx].d = p->value().toFloat(); break;
          case 's': confTuning.pid.pidArray[idx].sat = p->value().toFloat(); break;
        }
      }
    }
    confWrite();
    request->send(200, "text/html", otaCreatePidTable());
  });
  server.on("/auth", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/auth.html", "text/html", false, otaArgProcessor);
  });
  server.on("/auth", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        if (p->name() == "ssid" && p->value() != "") confAuth.wifiSsid = p->value();
        if (p->name() == "pass" && p->value() != "" && p->value() != "***") confAuth.wifiPass = p->value();
        if (p->name() == "btmac" && p->value() != "") confAuth.btMac = p->value();
      }
    }
    confWrite();
    request->send(SPIFFS, "/auth.html", "text/html", false, otaArgProcessor);
  });
  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", String("Mode switched to bluetooth"));
    confTuning.mode = CONF_MODE_BT;
    confWrite();
    delay(500);
    ESP.restart();
  });
  server.on("/reg", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/reg.html", "text/html", false, otaArgProcessor);
  });
  server.on("/reg", HTTP_POST, [](AsyncWebServerRequest *request) {
    AsyncWebParameter* submit = request->getParam("submit", true);
    if(submit != nullptr) {
      AsyncWebParameter* p = request->getParam(submit->value(), true);
      if(p != nullptr) {
        otaSaveParameter(p->name(), p->value());
      }
    }
    request->send(SPIFFS, "/reg.html", "text/html", false, otaArgProcessor);
  });
  server.on("/params", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/params.html", "text/html", false, otaArgProcessor);
  });
  server.on("/params", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        if (p->name() == "nodeId" && p->value() != "") confTuning.nodeId = p->value().toInt();
        if (p->name() == "mode" && p->value() != "") confTuning.mode = p->value().toInt();
      }
    }
    confWrite();
    request->send(SPIFFS, "/params.html", "text/html", false, otaArgProcessor);
  });
  server.on("/upload", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/upload.html", "text/html", false, otaArgProcessor);
  });
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200);
  }, otaHandleUpload);
}

void otaInit() {
  otaRegisterPages();

  AsyncElegantOTA.begin(&server);
  // WebSerial is accessible at "<IP Address>/webserial" in browser
  WebSerial.begin(&server);
  server.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  if(apMode) {
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
  }
}
