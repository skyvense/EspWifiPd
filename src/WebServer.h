#pragma once

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include "EspSmartWifi.h"
#include "PowerMonitor.h"
#include "Display.h"
#include "VoltageCtl.h"

#define BUTTON_PIN 0  



class WebServer {
public:
    WebServer(EspSmartWifi& wifi, EasyLed& led, Display& display);
    void begin();
    void handleClient();
    void stop();
    void handleButtonPress();

private:
    static const int RELAY_PINS[3];
    static const char INDEX_HTML[] PROGMEM;
    
    ESP8266WebServer server;
    EspSmartWifi& wifi;
    EasyLed& led;
    Display& display;
    PowerMonitor powerMonitor;
    VoltageCtl voltageCtl;
    
    
    
    // 基本处理函数
    void handleRoot();
    void handleStatus();
    void handlePower();
    void handleVoltage();
    void handleRestart();
    void handleUpgrade();
    void handleUpdate();
    void handleUpdateUpload();
    void handleNotFound();
    bool initPowerMonitor();
    
    // config pages
    void HandleConfigRoot();
    void HandleConfigSave();
    void handleGetConfigData();

    void initTime();
    void syncTime();

}; 