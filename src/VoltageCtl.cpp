#include "VoltageCtl.h"

VoltageCtl::VoltageCtl() : currentVoltage(VOLTAGE_5V) {
    // 初始化引脚
    pinMode(PD_CFG1, OUTPUT);
    pinMode(PD_CFG2, OUTPUT);
    pinMode(PD_CFG3, OUTPUT);
    Serial.println("VoltageCtl initialized with default 5V");
}

void VoltageCtl::begin() {
    // 设置默认电压为5V
    setVoltage(VOLTAGE_5V);
    Serial.println("VoltageCtl begin with 5V");
}

bool VoltageCtl::setVoltage(uint8_t level) {
    Serial.print("Setting voltage level: ");
    Serial.println(level);
    
    // 检查电压等级是否有效
    if (level > VOLTAGE_20V) {
        Serial.println("Invalid voltage level");
        return false;
    }

    // 根据电压等级设置PD控制引脚
    switch (level) {
        case VOLTAGE_5V:
            Serial.println("Setting 5V: CFG1=1, CFG2=0, CFG3=0");
            digitalWrite(PD_CFG1, HIGH);  // 1
            digitalWrite(PD_CFG2, HIGH);   // -
            digitalWrite(PD_CFG3, HIGH);   // -
            break;
        case VOLTAGE_9V:
            Serial.println("Setting 9V: CFG1=0, CFG2=0, CFG3=0");
            digitalWrite(PD_CFG1, LOW);   // 0
            digitalWrite(PD_CFG2, LOW);   // 0
            digitalWrite(PD_CFG3, LOW);   // 0
            break;
        case VOLTAGE_12V:
            Serial.println("Setting 12V: CFG1=0, CFG2=0, CFG3=1");
            digitalWrite(PD_CFG1, LOW);   // 0
            digitalWrite(PD_CFG2, LOW);   // 0
            digitalWrite(PD_CFG3, HIGH);  // 1
            break;
        case VOLTAGE_15V:
            Serial.println("Setting 15V: CFG1=0, CFG2=1, CFG3=1");
            digitalWrite(PD_CFG1, LOW);   // 0
            digitalWrite(PD_CFG2, HIGH);  // 1
            digitalWrite(PD_CFG3, HIGH);  // 1
            break;
        case VOLTAGE_20V:
            Serial.println("Setting 20V: CFG1=0, CFG2=1, CFG3=0");
            digitalWrite(PD_CFG1, LOW);   // 0
            digitalWrite(PD_CFG2, HIGH);  // 1
            digitalWrite(PD_CFG3, LOW);   // 0
            break;
    }

    currentVoltage = level;
    Serial.print("Voltage set to: ");
    Serial.println(level);
    return true;
} 