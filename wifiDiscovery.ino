#define WM_MDNS 1
#include <Arduino.h>
#include <FS.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

// TODO
    //reset settings - wipe credentials for testing
    //wm.resetSettings();

#ifndef HOSTNAME
#define HOSTNAME "O-MI-generic-device"
#endif

const char* hostname = HOSTNAME;

const char* APssid = "O-MI-NotConnected";
const char* APssidHidden = "O-MI-BackupNet";
const char* APpassword = "notsecure";
IPAddress apIP(192,168,8,1);               // The IP address of the access point

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

void setup() {
  Serial.begin(115200);
  delay(100);

  WiFi.setHostname(hostname);
  WiFi.mode(WIFI_AP_STA); // explicitly set mode, esp defaults to STA+AP

  Serial.println("Start WiFiManager process.");
  WiFiManager wm;
  // wm.autoConnect(); // auto generated AP name from chipid
  // wm.autoConnect("AutoConnectAP"); // anonymous ap
  wm.autoConnect(APssid, APpassword) || reset(); // password protected ap, blocks

  Serial.println("Connected...yeey :)");
  setupWifiProvider();
}

void loop() {
  // TODO?
}
