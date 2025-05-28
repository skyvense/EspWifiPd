#include "VoltageCtl.h"

VoltageCtl::VoltageCtl() : currentVoltage(VOLTAGE_9V) {
    // 初始化引脚
    pinMode(PD_CFG1, OUTPUT);
    pinMode(PD_CFG2, OUTPUT);
    pinMode(PD_CFG3, OUTPUT);
}

void VoltageCtl::begin() {
    // 设置默认电压为9V
    setVoltage(VOLTAGE_9V);
}

bool VoltageCtl::setVoltage(uint8_t level) {
    if (level > VOLTAGE_20V) {
        return false;  // 无效的电压等级
    }

    // 根据电压等级设置PD控制引脚
    switch (level) {
        case VOLTAGE_9V:
            digitalWrite(PD_CFG1, LOW);
            digitalWrite(PD_CFG2, LOW);
            digitalWrite(PD_CFG3, LOW);
            break;
        case VOLTAGE_12V:
            digitalWrite(PD_CFG1, HIGH);
            digitalWrite(PD_CFG2, LOW);
            digitalWrite(PD_CFG3, LOW);
            break;
        case VOLTAGE_15V:
            digitalWrite(PD_CFG1, LOW);
            digitalWrite(PD_CFG2, HIGH);
            digitalWrite(PD_CFG3, LOW);
            break;
        case VOLTAGE_20V:
            digitalWrite(PD_CFG1, HIGH);
            digitalWrite(PD_CFG2, HIGH);
            digitalWrite(PD_CFG3, HIGH);
            break;
    }

    currentVoltage = level;
    return true;
} 