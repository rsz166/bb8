#include <ota_wrapper.h>
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <WebSerial.h>
#include <configurations.h>
#include <log.h>

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
    response.concat("</tbody></table><input type =\"submit\" value =\"Submit\"></form>");
    return response;
}

void otaRegisterPages() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", "<h1>BB8</h1><a href=\"/update\">Update</a><br/><a href=\"/webserial\">WebSerial</a><br/><a href=\"/pid\">PID tune</a><br/><a href=\"/auth\">Authentication</a>");
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
    request->send(200, "text/html", 
      String("<h1>Authentication</h1><form method=\"POST\"><label>ssid</label><input type=\"text\" name=\"ssid\" value=\""+confAuth.wifiSsid+
      "\"/><br/><label>pass</label><input type=\"password\" name=\"pass\" value=\"***\"/>"+
      "<br/><label>Bluetooth MAC</label><input type=\"text\" name=\"btmac\" value=\""+confAuth.btMac+"\"/><br/><input type =\"submit\" value =\"Submit\"></form>"));
  });
  server.on("/auth", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        if (p->name() == "ssid" && p->value() != "") confAuth.wifiSsid = p->value();
        if (p->name() == "pass" && p->value() != "") confAuth.wifiPass = p->value();
        if (p->name() == "btmac" && p->value() != "") confAuth.btMac = p->value();
      }
    }
    confWrite();
    request->send(200, "text/html", 
      String("<h1>Authentication</h1><form method=\"POST\"><label>ssid</label><input type=\"text\" name=\"ssid\" value=\""+confAuth.wifiSsid+
      "\"/><br/><label>pass</label><input type=\"password\" name=\"pass\" value=\"***\"/>"+
      "<br/><label>Bluetooth MAC</label><input type=\"text\" name=\"btmac\" value=\""+confAuth.btMac+"\"/><br/><input type =\"submit\" value =\"Submit\"></form>"));
  });
  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", String("Mode switched to bluetooth"));
    confTuning.mode = CONF_MODE_BT;
    confWrite();
    delay(500);
    ESP.restart();
  });
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
