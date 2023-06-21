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

String processor(const String& var){
  if(var == "wifiSsid") return String(confAuth.wifiSsid);
  if(var == "btMac") return String(confAuth.btMac);
  return String();
}

// handles uploads
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
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

void otaRegisterPages() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
  });
  server.serveStatic("/", SPIFFS, "/");
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
    request->send(SPIFFS, "/auth.html", "text/html", false, processor);
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
    request->send(SPIFFS, "/auth.html", "text/html", false, processor);
  });
  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", String("Mode switched to bluetooth"));
    confTuning.mode = CONF_MODE_BT;
    confWrite();
    delay(500);
    ESP.restart();
  });
  server.on("/reg", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", otaCreateRegTable());
  });
  server.on("/params", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/params.html", "text/html", false, processor);
  });
  server.on("/upload", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/upload.html", "text/html", false, processor);
  });
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200);
  }, handleUpload);
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
