#pragma once

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include "EspSmartWifi.h"
#include "PowerMonitor.h"
#include "Display.h"

#define BUTTON_PIN 12  



class WebServer {
public:
    WebServer(EspSmartWifi& wifi, EasyLed& led, Display& display);
    void begin();
    void handleClient();
    void stop();
    void updateRelay(int index, bool state);
    void handleButtonPress();

private:
    static const int RELAY_PINS[3];
    static const char INDEX_HTML[] PROGMEM;
    
    ESP8266WebServer server;
    EspSmartWifi& wifi;
    EasyLed& led;
    Display& display;
    PowerMonitor powerMonitor;
    
    
    // 定时器相关
    static const int MAX_TIMERS = 10;
    Timer timers[MAX_TIMERS];
    int timerCount = 0;
    unsigned long lastTimeCheck = 0;
    const unsigned long timeCheckInterval = 1000;  // 1秒
    
    // 电流保护相关
    uint16_t currentLimits[3] = {0, 0, 0};  // 电流限制值（毫安）
    bool protectionTriggered[3] = {false, false, false};  // 保护触发状态
    
    // 基本处理函数
    void handleRoot();
    void handleStatus();
    void handleRelay(int relay);
    void handlePower();
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
    // 定时器相关函数
    void handleGetTimerConfig();
    void handleAddTimer();
    void handleTimerConfig();
    void handleDeleteTimer();
    bool addTimer(const Timer& timer);
    bool deleteTimer(uint8_t id);
    bool updateTimer(const Timer& timer);
    Timer* getTimer(uint8_t id);
    bool isWeekday(int weekday);
    bool shouldTriggerTimer(const Timer& timer, const tm* timeinfo);
    void saveTimerConfig();
    void loadTimerConfig();
    void checkTimers();
    void initTime();
    void syncTime();
    
    // 电流保护相关函数
    void handleGetProtection();
    void handleSetProtection();
    void handleProtectionStatus();
    void saveProtectionConfig();
    void loadProtectionConfig();
    void checkCurrentProtection();
}; 