#define WM_MDNS 1
#include <Arduino.h>
#include <FS.h>
//#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ESPAsyncWiFiManager.h>
#include <yxml.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// select which pin will trigger the configuration portal when set to LOW
#ifndef CONF_TRIGGER_PIN
#define CONF_TRIGGER_PIN 0
#endif

// TODO
    //reset settings - wipe credentials for testing
    //wm.resetSettings();

#ifndef HOSTNAME
#define HOSTNAME "O-MI-generic-device"
#endif

const char* hostname = HOSTNAME;

const char* APssid = "O-MI-NotConnected";
const char* APssidHidden = "O-MI-BackupNet";
#ifndef AP_PASSWORD
#define AP_PASSWORD "notsecure"
#endif
const char* APpassword = AP_PASSWORD;
IPAddress apIP(192,168,8,1);               // The IP address of the access point
const char* apUrl = "http://192.168.8.1/Objects/Device/WiFi/";

AsyncWebServer server(80);
DNSServer dns;
AsyncWiFiManager wm(&server, &dns);
unsigned int  timeout   = 120; // seconds to run for
unsigned int  startTime = millis();
bool portalRunning      = false;
bool startAP            = false; // start AP and webserver if true, else start only webserver

bool reset() {
  Serial.println("SOFTWARE RESTART.");
  delay(1000);
  ESP.restart();
  return false;
}



//void doWiFiManager(){
//  // is auto timeout portal running
//  if(portalRunning){
//    wm.process();
//    if((millis()-startTime) > (timeout*1000)){
//      Serial.println("portaltimeout");
//      portalRunning = false;
//      if(startAP){
//        wm.stopConfigPortal();
//      }  
//      else{
//        wm.stopWebPortal();
//      } 
//    }
//  }
//
//  // is configuration portal requested?
//  if(digitalRead(CONF_TRIGGER_PIN) == LOW && (!portalRunning)) {
//    startAP = WiFi.status() != WL_CONNECTED;
//    if(startAP){
//      Serial.println("Button Pressed, Starting Config Portal");
//      wm.setConfigPortalBlocking(false);
//      wm.startConfigPortal();
//    }  
//    else{
//      Serial.println("Button Pressed, Starting Web Portal");
//      wm.startWebPortal();
//    }  
//    portalRunning = true;
//    startTime = millis();
//  }
//}

#define NOT_FOUND -1
const char tagEnd = '>';

//WiFiMulti wifimulti;
bool findValue(const String& xml, String& result, unsigned& fromIndex) {
    String valueStartStr(("<value "));
    String valueEndStr(("</value>"));

    int start = NOT_FOUND, end = NOT_FOUND;

    Serial.println("[OMI-parser] findValue");

    // find start
    if ((start = xml.indexOf(valueStartStr, fromIndex)) != NOT_FOUND) {
        start += valueStartStr.length();

        fromIndex = start;

        // find open tag end
        if ((start = xml.indexOf(tagEnd, fromIndex)) != NOT_FOUND) {
            start += 1; // '>'

            fromIndex = start;

            // find end
            if ((end = xml.indexOf(valueEndStr, start)) != NOT_FOUND) {
                Serial.print("[OMI-parser] FOUND ");

                result = xml.substring(start, end);
                Serial.println(result);

                fromIndex = end + valueEndStr.length();
                return true;
            }
            else {
                Serial.println("[OMI-parser] Error: closing tag not found");
                return false;
            }
        } else {
            Serial.println("[OMI-parser] Error: starting tag end not found");
            return false;
        }
    }
    else {
        Serial.println("[OMI-parser] Error: starting tag not found");
        return false;
    }

}

void setupWifiProvider() {
  Serial.println("Setup softAP.");

  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(APssidHidden, APpassword, 1, 1) || reset(); // Start hidden access point

  server.on("/Objects/Device/WiFi/", HTTP_GET, [](AsyncWebServerRequest *request){
      AsyncResponseStream *response = request->beginResponseStream("text/xml");
      response->print("<Object><id>WiFi</id>");
      response->printf("<InfoItem name=\"SSID\"><value>%s</value></InfoItem>", WiFi.SSID());
      response->printf("<InfoItem name=\"psk\"><value>%s</value></InfoItem>", WiFi.psk());
      response->print("</Object>");
      request->send(response);
  });
  server.begin();
  // TODO better server
}

bool connectWifiProvider(){
  //wifimulti.addAP(APssidHidden, APpassword); // not possible with hidden AP?
  WiFi.begin(APssidHidden, APpassword);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) return false;
  
  HTTPClient client;
  client.begin(apUrl);
  int code = client.GET();
  if (code == HTTP_CODE_OK) {
    String payload = client.getString();
    // FIXME change hack to real xml library etc
    String ssid = String();
    String psk = String();
    unsigned index=0;
    if (findValue(payload, ssid, index)) {
      if (findValue(payload, psk, index)) {
        WiFi.disconnect();
        WiFi.begin(ssid.c_str(), psk.c_str());
        return WiFi.waitForConnectResult();
      }
    }
  }
  return false;
}


void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(CONF_TRIGGER_PIN, INPUT_PULLUP);

  WiFi.mode(WIFI_AP_STA); // explicitly set mode just in case, esp should default to STA+AP
  //WiFi.setHostname(hostname); not working!
  //wm.setHostname(hostname); // other wm version

  // Try to connect three different ways
  bool connectionSuccessful = false;

  if (WiFi.SSID().length() > 0) { 
    // 1.
    Serial.println("Try to connect to saved wifi");
    //wifimulti.addAP() // do we want to rescan a better channel ?
    WiFi.begin(); // try to connect to saved wifi
    connectionSuccessful = WiFi.waitForConnectResult() == WL_CONNECTED;
  }
  if (!connectionSuccessful) {
    // 2. 
    Serial.println("Try to connect wifi credentials provider.");
    connectionSuccessful = connectWifiProvider();
  }
  if (!connectionSuccessful){
    // 3.
    Serial.println("Start WiFiManager process.");
    // wm.autoConnect(); // auto generated AP name from chipid
    // wm.autoConnect("AutoConnectAP"); // anonymous ap
    wm.autoConnect(APssid, APpassword) || reset(); // password protected ap, blocks
  }

  Serial.println("Connected...yeey :)");
  setupWifiProvider();
}

void loop() {
  // TODO?
  //doWiFiManager();
  if ( digitalRead(CONF_TRIGGER_PIN) == LOW ) {
    if (!wm.startConfigPortal("O-MI-Config")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      reset(); delay(5000);
    }
  }
}
