#include "VoltageCtl.h"

VoltageCtl::VoltageCtl() : currentVoltage(VOLTAGE_5V) {
    // 初始化引脚
    pinMode(PD_CFG1, OUTPUT);
    pinMode(PD_CFG2, OUTPUT);
    pinMode(PD_CFG3, OUTPUT);
    Serial.println("VoltageCtl initialized with 5V");
}

void VoltageCtl::begin() {
    if (!loadConfig()) {
        // 如果加载配置失败，设置默认电压
        setVoltage(VOLTAGE_5V);
    }
    else{
        setVoltage(currentVoltage);
    }
}

bool VoltageCtl::setVoltage(uint8_t level) {
    Serial.printf("Setting voltage level: %d\n", level);
    
    // 检查电压等级是否有效
    if (level > VOLTAGE_20V) {
        Serial.println("Invalid voltage level");
        return false;
    }

    // 根据电压等级设置PD控制引脚
    switch (level) {
        case VOLTAGE_5V:
            Serial.println("Setting 5V: CFG1=1, CFG2=1, CFG3=1");
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
    delay(1000); // 延时1秒，等待电压稳定
    saveConfig(); // 保存配置
    Serial.print("Voltage set to: ");
    Serial.println(level);
    return true;
}

bool VoltageCtl::saveConfig() {
    if (!SPIFFS.begin()) {
        Serial.println("Failed to mount SPIFFS");
        return false;
    }

    // 创建新的JSON文档
    StaticJsonDocument<64> doc;
    doc["currentVol"] = currentVoltage;

    // 直接写入文件
    File configFile = SPIFFS.open("/vol-config.json", "w");
    if (!configFile) {
        Serial.println("Failed to open vol-config.json for writing");
        return false;
    }

    if (serializeJson(doc, configFile) == 0) {
        Serial.println("Failed to write to vol-config.json");
        return false;
    }

    configFile.close();
    Serial.println("Voltage configuration saved");
    return true;
}

bool VoltageCtl::loadConfig() {
    if (!SPIFFS.begin()) {
        Serial.println("Failed to mount SPIFFS");
        return false;
    }

    File configFile = SPIFFS.open("/vol-config.json", "r");
    if (!configFile) {
        Serial.println("Failed to open vol-config.json");
        return false;
    }

    StaticJsonDocument<64> doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();

    if (error) {
        Serial.println("Failed to parse vol-config.json");
        return false;
    }

    if (doc.containsKey("currentVol")) {
        uint8_t savedVoltage = doc["currentVol"];
        if (savedVoltage <= VOLTAGE_20V) {
            currentVoltage = savedVoltage;
            Serial.printf("Loaded voltage configuration: %d\n", currentVoltage);
            return true;
        }
    }

    Serial.println("No valid voltage configuration found");
    return false;
} 