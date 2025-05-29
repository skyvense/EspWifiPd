#pragma once

#include <Arduino.h>
#include <FS.h>
#include <ArduinoJson.h>

// PD电压档位定义
#define VOLTAGE_5V  0
#define VOLTAGE_9V  1
#define VOLTAGE_12V 2
#define VOLTAGE_15V 3
#define VOLTAGE_20V 4

// PD控制器引脚定义
#define PD_CFG1 16  // D4
#define PD_CFG2 14  // D5
#define PD_CFG3 12  // D6

class VoltageCtl {
public:
    VoltageCtl();
    void begin();
    bool setVoltage(uint8_t level);
    uint8_t getCurrentVoltage() const { return currentVoltage; }
    bool saveConfig();
    bool loadConfig();

private:
    uint8_t currentVoltage;
    static const unsigned long VOLTAGE_CHANGE_DELAY = 1000; // 1秒延时
}; 