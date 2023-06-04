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

bool otaNetworkInitAP() {
  // TODO   WiFi.mode(WIFI_AP);
  return false;
}

bool otaNetworkInitSTA(const char* ssid, const char* pass) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  return WiFi.waitForConnectResult() == WL_CONNECTED;
}

void otaRegisterPages() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", "<h1>BB8</h1><a href=\"/update\">Update</a><br/><a href=\"/webserial\">WebSerial</a><br/><a href=\"/pid\">PID tune</a>");
  });
  server.on("/pid", HTTP_GET, [](AsyncWebServerRequest *request) {
    int params = request->params();
    int idx = -1;
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(!p->isPost()){
        if (p->name() == "idx") idx = p->value().toFloat();
      }
    }
    ConfPIDParam_t pid = {.p = 0, .i = 0, .d = 0};
    if(idx >= 0) {
      confGetPID(idx, &pid);
    }
    LOG_F("PID idx:%i p:%f i:%f d:%f\n", idx, pid.p, pid.i, pid.d);
    char s[30];
    confGetWifiSsid(s,30);
    LOG_F("Test: %s\n", s);
    request->send(200, "text/html", 
      String("<h1>PID tune</h1><form method=\"POST\"><label>idx: " + String(idx) +
      "</label><br><label>p</label><input type=\"text\" name=\"p\" value=\"" + String(pid.p) +
      "\"><br><label>i</label><input type=\"text\" name=\"i\" value=\"" + String(pid.i) +
      "\"><br><label>d</label><input type=\"text\" name=\"d\" value=\"" + String(pid.d) +
      "\"><br><input type =\"submit\" value =\"Submit\"></form>"));
  });
  server.on("/pid", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    ConfPIDParam_t pid;
    int idx = -1;
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        if (p->name() == "p") pid.p = p->value().toFloat();
        if (p->name() == "i") pid.i = p->value().toFloat();
        if (p->name() == "d") pid.d = p->value().toFloat();
      } else {
        if (p->name() == "idx") idx = p->value().toFloat();
      }
    }
    if(idx >= 0) {
      confSetPID(idx, &pid);
      LOG_F("Store PID idx:%i p:%f i:%f d:%f\n", idx, pid.p, pid.i, pid.d);
    }
    request->send(200, "text/html", 
      String("<h1>PID tune</h1><form method=\"POST\"><label>idx: " + String(idx) +
      "</label><br><label>p</label><input type=\"text\" name=\"p\" value=\"" + String(pid.p) +
      "\"><br><label>i</label><input type=\"text\" name=\"i\" value=\"" + String(pid.i) +
      "\"><br><label>d</label><input type=\"text\" name=\"d\" value=\"" + String(pid.d) +
      "\"><br><input type =\"submit\" value =\"Submit\"></form>"));
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
}
