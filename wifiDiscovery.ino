#define WM_MDNS 1
#include <Arduino.h>
#include <FS.h>
//#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ESPAsyncWiFiManager.h>
#include <yxml.h>

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

void setupWifiProvider() {
  Serial.println("Setup softAP.");

  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(APssidHidden, APpassword, 1, 1) || reset(); // Start hidden access point

  
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

bool connectWifiProvider(){
  
}


void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(CONF_TRIGGER_PIN, INPUT_PULLUP);

  WiFi.mode(WIFI_AP_STA); // explicitly set mode just in case, esp should default to STA+AP
  //WiFi.setHostname(hostname); not working!
  //wm.setHostname(hostname); // other wm version


  Serial.println("Try to connect wifi credentials provider.");
  if (!connectWifiProvider()){

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
    if (!wifiManager.startConfigPortal("O-MI-Config")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset(); delay(5000);
    }
  }
}
