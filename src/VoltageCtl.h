#pragma once

#include <Arduino.h>

// PD电压档位定义
#define VOLTAGE_9V  0
#define VOLTAGE_12V 1
#define VOLTAGE_15V 2
#define VOLTAGE_20V 3

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

private:
    uint8_t currentVoltage;
}; 