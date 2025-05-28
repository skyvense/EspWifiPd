#pragma once

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include "EspSmartWifi.h"
#include "PowerMonitor.h"
#include "Config.h"
#include "Display.h"

// 继电器引脚定义 - 统一在这里定义
#define RELAY_PIN_1 16  // D1
#define RELAY_PIN_2 14  // D2
#define RELAY_PIN_3 12  // D3
#define BUTTON_PIN 13   // D7
// 定时器重复类型
enum class TimerRepeat {
    ONCE,       // 只执行一次
    DAILY,      // 每天
    WEEKDAY,    // 工作日（周一到周五）
    WEEKEND,    // 周末（周六和周日）
    CUSTOM      // 自定义重复（可以指定具体星期几）
};

// 定时器结构体
struct Timer {
    uint8_t id;             // 定时器ID
    uint8_t relayId;        // 继电器ID (0-2)
    uint8_t hour;           // 小时 (0-23)
    uint8_t minute;         // 分钟 (0-59)
    bool enabled;           // 是否启用
    bool state;             // true = ON, false = OFF
    TimerRepeat repeat;     // 重复类型
    uint8_t weekdays;       // 自定义重复时的星期几 (bit 0-6 代表周日到周六)
    time_t lastTriggered;   // 上次触发时间
};

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