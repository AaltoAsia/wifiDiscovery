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


#define NOT_FOUND -1
const char tagEnd = '>';

bool reset() {
  Serial.println("SOFTWARE RESTART.");
  delay(1000);
  ESP.restart();
  return false;
}


//WiFiMulti wifimulti;
bool findValue(const String& xml, String& result, unsigned& fromIndex) {
    String valueStartStr(("<value"));
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
      response->printf("<InfoItem name=\"SSID\"><value>%s</value></InfoItem>", WiFi.SSID().c_str());
      response->printf("<InfoItem name=\"psk\"><value>%s</value></InfoItem>", WiFi.psk().c_str());
      response->print("</Object>");
      request->send(response);
  });
  server.begin();
  // TODO better server
}

HTTPClient client;
bool connectWifiProvider(){
  WiFi.begin(APssidHidden, APpassword);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) return false;
  Serial.println("Hidden wifi found!");
  
  client.begin(apUrl);
  int code = client.GET();
  if (code == HTTP_CODE_OK) {
    String payload = client.getString();
    Serial.println("Connected! Answer: "); Serial.println(payload);
    // FIXME change hack to real xml library etc
    String ssid = String();
    String psk = String();
    unsigned index=0;
    if (findValue(payload, ssid, index)) {
      Serial.print("SSID: "); Serial.print(ssid); 
      if (findValue(payload, psk, index)) {
        Serial.print(" Psk:"); Serial.println(psk); Serial.print("Disconnecting... ");

        client.end();
        //WiFi.disconnect();
        //WiFi.mode(WIFI_OFF);
        //WiFi.mode(WIFI_AP_STA);
        delay(100);
        Serial.println("Connecting...");

        WiFi.begin(ssid.c_str(), psk.c_str());
        Serial.println("Waiting...");
        return WiFi.waitForConnectResult(); // Backtrace: 0x401428ba:0x3ffb1e70 0x40137a19:0x3ffb1e90 0x40137b1f:0x3ffb1f60 0x40135f1e:0x3ffb1fb0 0x4008dfe5:0x3ffb1fd0
      }
    }
  }
  Serial.println("");
  Serial.println("Failure");
  return false;
}


void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("BOOT");
  pinMode(CONF_TRIGGER_PIN, INPUT_PULLUP);

  WiFi.mode(WIFI_AP_STA); // explicitly set mode just in case, esp should default to STA+AP
  WiFi.setHostname(hostname); // not working?
  //wm.setHostname(hostname); // other wm version

  // Try to connect three different ways
  bool connectionSuccessful = false;

  if (WiFi.SSID().length() > 0) {  // TODO FIXME: check that it is not O-MI-BackupNet in case a reset have occured during connection
    // 1.
    Serial.println("Try to connect to saved wifi");
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
    wm.autoConnect(APssid, APpassword) || reset(); // password protected ap, blocks
  }

  Serial.print("Connected...yeey :) SSID: "); Serial.println(WiFi.SSID());
  Serial.print("Local IP: "); Serial.println(WiFi.localIP());

  WiFi.setHostname(hostname); // not working?
  setupWifiProvider();
}

void loop() {
  // TODO: some demo code?
  //doWiFiManager();
  if ( digitalRead(CONF_TRIGGER_PIN) == LOW ) {
    Serial.println("Config portal O-MI-Config triggered via pin/button...");
    if (!wm.startConfigPortal("O-MI-Config")) {
      Serial.println("Failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      reset(); delay(5000);
    }
  }
}
